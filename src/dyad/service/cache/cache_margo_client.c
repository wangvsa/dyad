#include <dyad/service/cache/cache_margo_client.h>
#include <stdlib.h>

struct cache_margo_client {
   margo_instance_id mid;
   hg_id_t           sum_id;
   uint64_t          num_prov_hdl;
};

struct cache_provider_handle {
    cache_margo_client_t client;
    hg_addr_t      addr;
    uint16_t       provider_id;
    uint64_t       refcount;
};

int cache_margo_client_init(margo_instance_id mid, cache_margo_client_t* client)
{
    int ret = CACHE_SUCCESS;

    cache_margo_client_t c = (cache_margo_client_t)calloc(1, sizeof(*c));
    if(!c) return CACHE_FAILURE;

    c->mid = mid;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "cache_sum", &id, &flag);

    if(flag == HG_TRUE) {
        margo_registered_name(mid, "cache_sum", &c->sum_id, &flag);
    } else {
        c->sum_id = MARGO_REGISTER(mid, "cache_sum", sum_in_t, sum_out_t, NULL);
    }

    *client = c;
    return CACHE_SUCCESS;
}

int cache_margo_client_finalize(cache_margo_client_t client)
{
    if(client->num_prov_hdl != 0) {
        margo_warning(client->mid,
            "%d provider handles not released when cache_margo_client_finalize was called",
            client->num_prov_hdl);
    }
    free(client);
    return CACHE_SUCCESS;
}

int cache_provider_handle_create(
        cache_margo_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        cache_provider_handle_t* handle)
{
    if(client == CACHE_MARGO_CLIENT_NULL)
        return CACHE_FAILURE;

    cache_provider_handle_t ph =
        (cache_provider_handle_t)calloc(1, sizeof(*ph));

    if(!ph) return CACHE_FAILURE;

    hg_return_t ret = margo_addr_dup(client->mid, addr, &(ph->addr));
    if(ret != HG_SUCCESS) {
        free(ph);
        return CACHE_FAILURE;
    }

    ph->client      = client;
    ph->provider_id = provider_id;
    ph->refcount    = 1;

    client->num_prov_hdl += 1;

    *handle = ph;
    return CACHE_SUCCESS;
}

int cache_provider_handle_ref_incr(
        cache_provider_handle_t handle)
{
    if(handle == CACHE_PROVIDER_HANDLE_NULL)
        return CACHE_FAILURE;
    handle->refcount += 1;
    return CACHE_SUCCESS;
}

int cache_provider_handle_release(cache_provider_handle_t handle)
{
    if(handle == CACHE_PROVIDER_HANDLE_NULL)
        return CACHE_FAILURE;
    handle->refcount -= 1;
    if(handle->refcount == 0) {
        margo_addr_free(handle->client->mid, handle->addr);
        handle->client->num_prov_hdl -= 1;
        free(handle);
    }
    return CACHE_SUCCESS;
}

int cache_compute_sum(
        cache_provider_handle_t handle,
        int32_t x,
        int32_t y,
        int32_t* result)
{
    hg_handle_t   h;
    sum_in_t     in;
    sum_out_t   out;
    hg_return_t ret;

    in.x = x;
    in.y = y;

    ret = margo_create(handle->client->mid, handle->addr, handle->client->sum_id, &h);
    if(ret != HG_SUCCESS)
        return CACHE_FAILURE;

    ret = margo_provider_forward(handle->provider_id, h, &in);
    if(ret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHE_FAILURE;
    }

    ret = margo_get_output(h, &out);
    if(ret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHE_FAILURE;
    }

    *result = out.ret;

    margo_free_output(h, &out);
    margo_destroy(h);
    return CACHE_SUCCESS;
}
