#!/bin/bash

this_script_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd)

source $this_script_dir/prod_cons_argparse.sh

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
    ${GITHUB_WORKSPACE}/.github/prod-cons/dyad_consumer.sh $mode
CONS_JOB=$(flux job last)

echo "Submitting producer job"
flux submit --nodes 1 --exclusive -t 10 \
    --env=DYAD_KVS_NAMESPACE=${DYAD_KVS_NAMESPACE} \
    --env=DYAD_DTL_MODE=${DYAD_DTL_MODE} \
    ${GITHUB_WORKSPACE}/.github/prod-cons/dyad_producer.sh $mode
PROD_JOB=$(flux job last)

flux jobs -a
flux job attach $PROD_JOB
flux job attach $CONS_JOB

dyad stop
flux kvs namespace remove ${DYAD_KVS_NAMESPACE}
flux exec -r all rm -rf ${DYAD_PATH_CONSUMER} ${DYAD_PATH_PRODUCER}
