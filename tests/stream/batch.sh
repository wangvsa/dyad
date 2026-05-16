#************************************************************
# Copyright 2021 Lawrence Livermore National Security, LLC
# (c.f. AUTHORS, NOTICE.LLNS, COPYING)
#
# This file is part of the Flux resource manager framework.
# For details, see https://github.com/flux-framework.
#
# SPDX-License-Identifier: LGPL-3.0
#************************************************************

#!/bin/bash

DYAD_PATH=/tmp/$USER/dyad

# prepares the dyad environemnt (e.g., dyad module and flux kvs)
./startup.sh

flux resource list

flux submit -N 1 -n 1 --output=flux-{{id}}.out bash -c "./test_stream ${DYAD_PATH} ${DYAD_PATH}/test.txt 0 0"
flux submit -N 1 -n 1 --output=flux-{{id}}.out bash -c "./test_stream ${DYAD_PATH} ${DYAD_PATH}/test.txt 1 0"

flux jobs -a
flux queue drain

flux kvs namespace remove "test"
flux module remove dyad
