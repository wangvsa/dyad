#include <stdio.h>
#include <stdlib.h>
#include <margo.h>
#include <dyad/service/cache/cache_client.h>
#include <dyad/service/cache/cache_margo_client.h>

int main(int argc, char** argv)
{
    if(argc != 2) {
        fprintf(stderr,"Usage: %s <server address>\n", argv[0]);
        exit(0);
    }

    const char* svr_addr_str = argv[1];

    margo_instance_id mid = margo_init("na+sm", MARGO_CLIENT_MODE, 0, 0);
    margo_set_log_level(mid, MARGO_LOG_INFO);

    hg_addr_t svr_addr;
    margo_addr_lookup(mid, svr_addr_str, &svr_addr);

    cache_margo_client_t cache_clt;
    cache_provider_handle_t cache_ph;

    cache_margo_client_init(mid, &cache_clt);

    cache_provider_handle_create(cache_clt, svr_addr, CACHE_MARGO_PROVIDER_ID, &cache_ph);

    int32_t result;
    cache_compute_sum(cache_ph, 45, 23, &result);

    cache_provider_handle_release(cache_ph);

    cache_margo_client_finalize(cache_clt);

    margo_addr_free(mid, svr_addr);

    margo_finalize(mid);

    return 0;
}
