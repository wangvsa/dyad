#!/bin/bash

script_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd)

if [ $# -lt 1 ] ; then
  echo "Need a task id"
  exit 1
fi

task=$1
mkdir -p ${DYAD_PATH_PRODUCER}/${task}
shift

source ${script_dir}/select_language.sh


# Choose among C, C++, Python
if [[ "$mode" == "${valid_modes[0]}" ]]; then # C
    echo `hostname` \$ LD_PRELOAD=${DYAD_INSTALL_LIBDIR}/libdyad_wrapper.so ${script_dir}/../c_prod 10 ${DYAD_PATH_PRODUCER}/${task}
    LD_PRELOAD=${DYAD_INSTALL_LIBDIR}/libdyad_wrapper.so ${script_dir}/../c_prod 10 ${DYAD_PATH_PRODUCER}/${task}
elif [[ "$mode" == "${valid_modes[1]}" ]]; then # C++
    echo ${script_dir}/../cpp_prod 10 ${DYAD_PATH_PRODUCER}/${task}
    ${script_dir}/../cpp_prod 10 ${DYAD_PATH_PRODUCER}/${task}
elif [[ "$mode" == "${valid_modes[2]}" ]]; then # Python
    echo python3 ../../../../tests/pydyad_spsc/producer.py ${DYAD_PATH_PRODUCER}/${task} 10 50
    python3 ../../../../tests/pydyad_spsc/producer.py ${DYAD_PATH_PRODUCER}/${task} 10 50
else
    echo "Invalid language mode for producer ${task}: $mode"
    exit 1
fi
