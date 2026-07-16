#!/bin/bash
# FLUX: -N 2
# FLUX: --output=pydyad_spsc_range_test.out
# FLUX: --error=pydyad_spsc_range_test.err

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
    DYAD_KVS_NAMESPACE="pydyad_range_test"
fi
if [ -z "${FILE_SIZE}" ]; then
    FILE_SIZE=1048576  # 1 MiB, big enough for several distinct ranges
fi
if [ -z "${NUM_RANGES}" ]; then
    NUM_RANGES=20
fi

export LD_LIBRARY_PATH="${DYAD_INSTALL_LIBDIR}:${LD_LIBRARY_PATH}"

flux kvs namespace create ${DYAD_KVS_NAMESPACE}

#### NOTE: consume_range() is only implemented for DYAD_DTL_FLUX_RPC and
#### DYAD_DTL_MARGO (not UCX) -- see dyad_consume_range() in dyad_client.c.

cmd_cons="(rm -rf ${DYAD_PATH_CONSUMER}; \
           mkdir -m 755 -p ${DYAD_PATH_CONSUMER}; \
           python3 consumer.py ${DYAD_PATH_CONSUMER} ${FILE_SIZE} ${NUM_RANGES})"

flux submit -N 1 --tasks-per-node=1 --exclusive \
    --output="pydyad_range_cons.out" --error="pydyad_range_cons.err" \
    --env=DYAD_PATH_CONSUMER=${DYAD_PATH_CONSUMER} \
    --env=DYAD_DTL_MODE=${DYAD_DTL_MODE} \
    --env=DYAD_KVS_NAMESPACE=${DYAD_KVS_NAMESPACE} \
    --flags=waitable \
    bash -c "${cmd_cons}"

cmd_prod="(rm -rf ${DYAD_PATH_PRODUCER}; \
           mkdir -m 755 -p ${DYAD_PATH_PRODUCER}; \
           flux module load ${DYAD_INSTALL_LIBDIR}/dyad.so --mode=${DYAD_DTL_MODE} ${DYAD_PATH_PRODUCER}; \
           flux getattr rank > prod_rank.txt; \
           python3 producer.py ${DYAD_PATH_PRODUCER} ${FILE_SIZE})"

flux submit -N 1 --tasks-per-node=1 --exclusive \
    --output="pydyad_range_prod.out" --error="pydyad_range_prod.err" \
    --env=DYAD_PATH_PRODUCER=${DYAD_PATH_PRODUCER} \
    --env=DYAD_DTL_MODE=${DYAD_DTL_MODE} \
    --env=DYAD_KVS_NAMESPACE=${DYAD_KVS_NAMESPACE} \
    --flags=waitable \
    bash -c "${cmd_prod}"

flux job wait --all

if [ -f prod_rank.txt ] ; then
    flux exec -r `cat prod_rank.txt` flux module remove dyad
    rm prod_rank.txt
else
    flux exec -r all flux module remove dyad
fi
flux kvs namespace remove ${DYAD_KVS_NAMESPACE}

if [ $? -ne 0 ]; then
    echo "ERROR: a job crashed with an error"
    exit 1
fi
