#!/bin/bash

if [ ${DYAD_ARGS_PARSER_PATH} == "" ] ; then
    echo "The environment variable DYAD_ARGS_PARSER_PATH is not set."
    exit 1
fi

if [ `basename ${DYAD_ARGS_PARSER_PATH}` != "args.sh" ] || \
   [ ! -f ${DYAD_ARGS_PARSER_PATH} ]
then
    echo "Cannot find argument parser." >&2
    exit 1
fi

source ${DYAD_ARGS_PARSER_PATH}

if [ "${DYAD_KVS_NS}" != "" ] ; then
    if [ "`flux kvs namespace list | grep ${DYAD_KVS_NS}`" != "" ] ; then
        # flush out any existing key-value entries
        flux kvs namespace remove ${DYAD_KVS_NS}
    fi
fi

if [ "${SYNC}" == "0" ] ; then
    flux exec -r all rm -rf $DYAD_PATH
    flux exec -r all mkdir -p $DYAD_PATH
    flux exec -r all chmod 775 $DYAD_PATH

    if [ -z ${RESET_KVS+x} ] || [ "${RESET_KVS}" == "" ] || [ "${RESET_KVS}" == "0" ] ; then
        kvs_exists=`flux exec -r all flux module list | grep kvs`
        if [ "${kvs_exists}" != "" ] ; then
            flux exec -r all flux module remove kvs
        fi
        flux exec -r all flux module load kvs
    fi

    dyad_exists=`flux exec -r all flux module list | grep dyad`
    if [ "${dyad_exists}" != "" ] ; then
        flux exec -r all flux module remove dyad
    fi
else
    pushd `dirname $DYAD_PATH` > /dev/null
    rm -rf $DYAD_PATH 2> /dev/null
    mkdir -p $DYAD_PATH 2> /dev/null
    popd > /dev/null
fi
