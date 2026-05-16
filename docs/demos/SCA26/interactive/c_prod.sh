mkdir -m 775 -p ${DYAD_PATH_PRODUCER}/prod-cons
echo LD_PRELOAD=${DYAD_INSTALL_PREFIX}/lib/libdyad_wrapper.so ../c_prod 10 ${DYAD_PATH_PRODUCER}/prod-cons
LD_PRELOAD=${DYAD_INSTALL_PREFIX}/lib/libdyad_wrapper.so ../c_prod 10 ${DYAD_PATH_PRODUCER}/prod-cons
