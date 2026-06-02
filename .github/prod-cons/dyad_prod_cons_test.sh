#!/bin/bash

if test "$#" -ne 1; then
    echo "Invalid number of arguments to $0"
    exit 1
fi

mode="$1"
valid_modes=("c" "cpp" "python")

mode_is_valid=0
for vm in "${valid_modes[@]}"; do
    if [[ "$mode" == "$vm" ]]; then
        mode_is_valid=1
        break
    fi
done

if [[ $mode_is_valid -eq 0 ]]; then
    echo "Invalid mode: $mode (expected one of: ${valid_modes[*]})"
    exit 2
fi

echo "Creating KVS namespace ${DYAD_KVS_NAMESPACE}"
flux kvs namespace create ${DYAD_KVS_NAMESPACE}

if [ "${DYAD_PATH}" == "" ]; then
    DYAD_PATH="dir"
fi
export DYAD_PATH_CONSUMER=${DYAD_PATH}_consumer
export DYAD_PATH_PRODUCER=${DYAD_PATH}_producer

echo "Starting DYAD service"
echo "  -- producer managed dir:" ${DYAD_PATH_PRODUCER}
dyad start -p ${DYAD_PATH_PRODUCER}
flux resource list

echo "Submitting consumer job"
flux submit --nodes 1 --exclusive -t 10 \
    --env=DYAD_KVS_NAMESPACE=${DYAD_KVS_NAMESPACE} \
    --env=DYAD_DTL_MODE=${DYAD_DTL_MODE} \
    --env=DYAD_MARGO_PROTO=${DYAD_MARGO_PROTO} \
    --output=out-cons.txt \
    --error=err-cons.txt \
    --unbuffered \
    ${GITHUB_WORKSPACE}/.github/prod-cons/dyad_consumer.sh $mode
CONS_JOB=$(flux job last)

echo "Submitting producer job"
flux submit --nodes 1 --exclusive -t 10 \
    --env=DYAD_KVS_NAMESPACE=${DYAD_KVS_NAMESPACE} \
    --env=DYAD_DTL_MODE=${DYAD_DTL_MODE} \
    --env=DYAD_MARGO_PROTO=${DYAD_MARGO_PROTO} \
    --output=out-prod.txt \
    --error=err-prod.txt \
    --unbuffered \
    ${GITHUB_WORKSPACE}/.github/prod-cons/dyad_producer.sh $mode
PROD_JOB=$(flux job last)

flux jobs -a
flux job attach $PROD_JOB
flux job attach $CONS_JOB

dyad stop
flux kvs namespace remove ${DYAD_KVS_NAMESPACE}
flux exec -r all rm -rf ${DYAD_PATH_CONSUMER} ${DYAD_PATH_PRODUCER}
