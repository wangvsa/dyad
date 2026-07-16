#!/bin/bash
# FLUX: -N 2
# FLUX: --output=pydyad_spsc_lazy_range_test.out
# FLUX: --error=pydyad_spsc_lazy_range_test.err

if [ -z "${DYAD_INSTALL_LIBDIR}" ]; then
    echo "DYAD_INSTALL_LIBDIR must be defined"
    exit 1
fi
if [ ! -d "${DYAD_INSTALL_LIBDIR}" ]; then
    echo "DYAD_INSTALL_LIBDIR ($DYAD_INSTALL_LIBDIR) does not exist"
    exit 1
fi
if [ ! -f "${DYAD_INSTALL_LIBDIR}/dyad.so" ]; then
    echo "Invalid contents in DYAD_INSTALL_LIBDIR ($DYAD_INSTALL_LIBDIR)"
    exit 1
fi
if [ -z "${DYAD_PATH_CONSUMER}" ]; then
    if [ -z "${DYAD_PATH}" ]; then
        echo "Either DYAD_PATH_CONSUMER or DYAD_PATH must be defined"
        exit 1
    else
        DYAD_PATH_CONSUMER="${DYAD_PATH}"
    fi
fi
if [ -z "${DYAD_PATH_PRODUCER}" ]; then
    if [ -z "${DYAD_PATH}" ]; then
        echo "Either DYAD_PATH_PRODUCER or DYAD_PATH must be defined"
        exit 1
    else
        DYAD_PATH_PRODUCER="${DYAD_PATH}"
    fi
fi
if [ -z "${DYAD_DTL_MODE}" ]; then
    DYAD_DTL_MODE="FLUX_RPC"
fi
if [ -z "${DYAD_KVS_NAMESPACE}" ]; then
    DYAD_KVS_NAMESPACE="pydyad_lazy_range_test"
fi
if [ -z "${FILE_SIZE}" ]; then
    FILE_SIZE=1048576  # 1 MiB, big enough for several distinct ranges/blocks
fi
if [ -z "${NUM_RANGES}" ]; then
    NUM_RANGES=20
fi
if [ -z "${ORIGIN_DIR}" ]; then
    # Must be on a filesystem shared by both the producer and consumer nodes
    # (unlike DYAD_PATH_PRODUCER/_CONSUMER, which are typically node-local
    # storage) -- this stands in for the parallel file system.
    ORIGIN_DIR="$(pwd)/lazy_range_origin"
fi

export LD_LIBRARY_PATH="${DYAD_INSTALL_LIBDIR}:${LD_LIBRARY_PATH}"

flux kvs namespace create ${DYAD_KVS_NAMESPACE}

rm -rf ${ORIGIN_DIR}
mkdir -m 755 -p ${ORIGIN_DIR}

#### NOTE: consume_range() (and thus dyad_range_cache_ensure()'s lazy origin
#### fallback) is only implemented for DYAD_DTL_FLUX_RPC and DYAD_DTL_MARGO
#### (not UCX) -- see dyad_consume_range() in dyad_client.c.

cmd_cons="(rm -rf ${DYAD_PATH_CONSUMER}; \
           mkdir -m 755 -p ${DYAD_PATH_CONSUMER}; \
           python3 consumer.py ${DYAD_PATH_CONSUMER} ${ORIGIN_DIR} ${FILE_SIZE} ${NUM_RANGES})"

flux submit -N 1 --tasks-per-node=1 --exclusive \
    --output="pydyad_lazy_range_cons.out" --error="pydyad_lazy_range_cons.err" \
    --env=DYAD_PATH_CONSUMER=${DYAD_PATH_CONSUMER} \
    --env=DYAD_DTL_MODE=${DYAD_DTL_MODE} \
    --env=DYAD_KVS_NAMESPACE=${DYAD_KVS_NAMESPACE} \
    --flags=waitable \
    bash -c "${cmd_cons}"

# Note: --origin_path is only set on the producer's flux module -- the
# consumer never touches DYAD_PATH_ORIGIN since this file is always remote
# to it (owned by a different rank), so only the module's
# dyad_fetch_range_request_cb() ever needs the origin fallback here.
cmd_prod="(rm -rf ${DYAD_PATH_PRODUCER}; \
           mkdir -m 755 -p ${DYAD_PATH_PRODUCER}; \
           flux module load ${DYAD_INSTALL_LIBDIR}/dyad.so --mode=${DYAD_DTL_MODE} \
               --origin_path=${ORIGIN_DIR} ${DYAD_PATH_PRODUCER}; \
           flux getattr rank > prod_rank.txt; \
           python3 producer.py ${DYAD_PATH_PRODUCER} ${ORIGIN_DIR} ${FILE_SIZE})"

flux submit -N 1 --tasks-per-node=1 --exclusive \
    --output="pydyad_lazy_range_prod.out" --error="pydyad_lazy_range_prod.err" \
    --env=DYAD_PATH_PRODUCER=${DYAD_PATH_PRODUCER} \
    --env=DYAD_DTL_MODE=${DYAD_DTL_MODE} \
    --env=DYAD_KVS_NAMESPACE=${DYAD_KVS_NAMESPACE} \
    --flags=waitable \
    bash -c "${cmd_prod}"

flux job wait --all
job_wait_rc=$?

if [ -f prod_rank.txt ] ; then
    prod_rank=$(cat prod_rank.txt)
    # Verify the local cache file and its bitmap were actually populated
    # lazily by dyad_range_cache_ensure() while serving the consumer's
    # fetches -- i.e. confirm laziness itself happened, not just that the
    # bytes returned to the consumer were correct.
    echo "Checking that the local cache file/bitmap were populated lazily on producer rank ${prod_rank}..."
    if ! flux exec -r ${prod_rank} bash -c \
        "test -s ${DYAD_PATH_PRODUCER}/range_test_data.bin && \
         test -s ${DYAD_PATH_PRODUCER}/range_test_data.bin.dyad_cached"; then
        echo "ERROR: local cache file/bitmap missing or empty after lazy fetch"
        job_wait_rc=1
    fi
    flux exec -r ${prod_rank} flux module remove dyad
    rm prod_rank.txt
else
    flux exec -r all flux module remove dyad
fi
flux kvs namespace remove ${DYAD_KVS_NAMESPACE}
rm -rf ${ORIGIN_DIR}

if [ ${job_wait_rc} -ne 0 ]; then
    echo "ERROR: a job crashed with an error, or the post-run laziness check failed"
    exit 1
fi
