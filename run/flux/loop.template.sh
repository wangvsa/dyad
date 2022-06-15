flux resource list

@JOB_NODES@
num_of_jobs=10
num_events_per_file=12

export DYAD_NUM_CPA_PTS=`echo "${num_events_per_file} + @MAX_ITERS_PER_NODE@ * @NUM_FILES_PER_ITER@ * ${num_events_per_file}" | bc`
echo "DYAD_NUM_CPA_PTS=${DYAD_NUM_CPA_PTS}"


# set variables
check_interval=10

spec=separate_steps.${nodes}.sh

rm -rf 0 1 2 3 content.sqlite

sleep ${check_interval}
# first run
bash ${spec} 1

sleep ${check_interval}

# more runs
if [ ${num_of_jobs} -gt 1 ] ; then
    for i in `seq 2 ${num_of_jobs} `
    do
        rm -rf 0 1 2 3 content.sqlite
        bash ${spec} $i

        sleep ${check_interval}

        # wait until the run completes
        sleep ${check_interval}
    done
fi
