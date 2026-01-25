module load flux-core mochi-margo

export DYAD_INSTALL_PREFIX=/home/${USER}/venv
export DYAD_KVS_NAMESPACE=dyad
export DYAD_DTL_MODE=MARGO
export DYAD_PATH_PRODUCER=/mnt/ssd/${USER}/dyad
export DYAD_PATH_CONSUMER=/mnt/ssd/${USER}/dya
source ${DYAD_INSTALL_PREFIX}/bin/activate
