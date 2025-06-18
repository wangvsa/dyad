#include <stdio.h>
#include <stdlib.h>
#include <margo.h>
#include <assert.h>
#include <dyad/service/cache/cache_client.h>
#include <dyad/service/cache/cache_margo_client.h>

struct cache_client {
    margo_instance_id       mid;
    hg_addr_t               svr_addr;
    cache_margo_client_t    margo_clt;
    cache_provider_handle_t cache_ph;
};
typedef struct cache_client* cache_client_t;
static cache_client_t g_cache_client = NULL;

void cache_init(char* server_addr_str) {

    margo_instance_id       mid;
    hg_addr_t               svr_addr;
    cache_provider_handle_t cache_ph;
    cache_margo_client_t    margo_clt;

    mid = margo_init("na+sm", MARGO_CLIENT_MODE, 0, 0);
    margo_set_log_level(mid, MARGO_LOG_INFO);

    margo_addr_lookup(mid, server_addr_str, &svr_addr);

    cache_margo_client_init(mid, &margo_clt);

    cache_provider_handle_create(margo_clt, svr_addr, CACHE_MARGO_PROVIDER_ID, &cache_ph);

    g_cache_client = (cache_client_t) malloc(sizeof(struct cache_client));
    g_cache_client->mid       = mid;
    g_cache_client->svr_addr  = svr_addr;
    g_cache_client->margo_clt = margo_clt;
    g_cache_client->cache_ph  = cache_ph;
}

void cache_fini() {
    if (g_cache_client == NULL) {
        return;
    }

    cache_provider_handle_release(g_cache_client->cache_ph);
    cache_margo_client_finalize(g_cache_client->margo_clt);
    margo_addr_free(g_cache_client->mid, g_cache_client->svr_addr);
    margo_finalize(g_cache_client->mid);
    free(g_cache_client);
    g_cache_client = NULL;
}

int cache_add_file(const char* fname, size_t len) {
    assert (g_cache_client != NULL);

    int32_t result;
    cache_compute_sum(g_cache_client->cache_ph, 45, 23, &result);
    printf("cache add file: %s, %ld\n", fname, len);

    return 0;
}
int cache_evict_file(const char* fname) {
    assert (g_cache_client != NULL);

    printf("cache evict file: %s\n", fname);
    return 0;
}
int cache_access_file(const char* fname) {
    assert (g_cache_client != NULL);

    printf("cache access file: %s\n", fname);
    return 0;
}

