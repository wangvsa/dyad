#!/bin/bash
# Flux commands manual
# https://flux-framework.readthedocs.io/projects/flux-core/en/latest/man1
# This script submits jobs for ${n_tasks} pairs of producers and consumers.
n_tasks=1

script_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd)

# Check if lib64 is under the installation. If not, try lib.
if [ -d ${DYAD_INSTALL_PREFIX}/lib64 ] ; then
    export DYAD_INSTALL_LIBDIR=${DYAD_INSTALL_PREFIX}/lib64
elif [ -d ${DYAD_INSTALL_PREFIX}/lib ] ; then
    export DYAD_INSTALL_LIBDIR=${DYAD_INSTALL_PREFIX}/lib
else
    echo "Cannot find DYAD LIB DIR"
    exit 1
fi


# Parse the argument that specifies the application language mode (c|cpp|python)
source ${script_dir}/select_language.sh


if [ "${DYAD_PATH_CONSUMER}" == "" ] || [ "${DYAD_PATH_PRODUCER}" == "" ]  ; then
    echo Undefined environment variables: DYAD_PATH_PRODUCER and DYAD_PATH_CONSUMER
    exit 1
fi

# Clean up any potentially existing data from previous runs
flux exec -r all rm -rf ${DYAD_PATH_CONSUMER} ${DYAD_PATH_PRODUCER}
# Prepare directories on local storages
flux exec -r all mkdir -p ${DYAD_PATH_CONSUMER} ${DYAD_PATH_PRODUCER}


# Start DYAD service
echo "Creating a KVS namespace for DYAD: ${DYAD_KVS_NAMESPACE}"
flux kvs namespace create ${DYAD_KVS_NAMESPACE}

echo "Loading DYAD service module ${DYAD_INSTALL_LIBDIR}/dyad.so"
flux exec -r all flux module load ${DYAD_INSTALL_LIBDIR}/dyad.so #--mode="${DYAD_DTL_MODE}" ${DYAD_PATH_PRODUCER}


# Here, we submit the consumer jobs first for demonstration purposes.
# In practice, producer jobs are submitted first.

for i_task in `seq 1 $n_tasks`
do
    echo "Submitting Consumer job"
    flux submit --nodes 1 --exclusive -t 1 \
                --env=DYAD_INSTALL_LIBDIR=${DYAD_INSTALL_LIBDIR} \
                --output=out-cons-${i_task}.txt \
                --error=err-cons-${i_task}.txt \
                ${script_dir}/task_cons.sh ${i_task} ${mode}
    CONS_IDs="${CONS_IDs} $(flux job last)"

    echo "Submitting Producer job"
    flux submit --nodes 1 --exclusive -t 1 \
                --env=DYAD_INSTALL_LIBDIR=${DYAD_INSTALL_LIBDIR} \
                --output=out-prod-${i_task}.txt \
                --error=err-prod-${i_task}.txt \
                ${script_dir}/task_prod.sh ${i_task} ${mode}
    PROD_IDs="${PROD_IDs} $(flux job last)"
done


# Inspect job's stdout/stderr by interactively attaching to the job while it is running or after it has completed
flux jobs -a
for id_prod in ${PROD_IDs}
do
    flux job attach ${id_prod}
done

for id_cons in ${CONS_IDs}
do
    flux job attach ${id_cons}
done

# Block until all queues become empty
flux queue drain


# Clean up
# Make sure the consumers are complete. In our case, `flux queue drain` facilitates that.
echo "Cleaning up"
flux exec -r all flux module remove dyad 2> /dev/null
flux kvs namespace remove ${DYAD_KVS_NAMESPACE}
flux exec -r all rm -rf ${DYAD_PATH_CONSUMER} ${DYAD_PATH_PRODUCER}
