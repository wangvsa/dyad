#ifndef DYAD_CACHE_MARGO_CLIENT_H
#define DYAD_CACHE_MARGO_CLIENT_H

#include <margo.h>
#include <dyad/service/cache/cache_common.h>

typedef struct cache_margo_client* cache_margo_client_t;
#define CACHE_MARGO_CLIENT_NULL ((cache_margo_client_t)NULL)

typedef struct cache_provider_handle *cache_provider_handle_t;
#define CACHE_PROVIDER_HANDLE_NULL ((cache_provider_handle_t)NULL)

/**
 * @brief Creates a CACHE client.
 *
 * @param[in] mid Margo instance
 * @param[out] client CACHE client
 *
 * @return CACHE_SUCCESS or error code defined in cache-common.h
 */
int cache_margo_client_init(margo_instance_id mid, cache_margo_client_t* client);

/**
 * @brief Finalizes a CACHE client.
 *
 * @param[in] client CACHE client to finalize
 *
 * @return CACHE_SUCCESS or error code defined in cache-common.h
 */
int cache_margo_client_finalize(cache_margo_client_t client);

/**
 * @brief Makes the target CACHE provider compute the sum of the
 * two numbers and return the result.
 *
 * @param[in] handle provide handle.
 * @param[in] x first number.
 * @param[in] y second number.
 * @param[out] result resulting value.
 *
 * @return CACHE_SUCCESS or error code defined in cache-common.h
 */
int cache_compute_sum(
        cache_provider_handle_t handle,
        int32_t x,
        int32_t y,
        int32_t* result);

/**
 * @brief Creates a CACHE provider handle.
 *
 * @param[in] client CACHE client responsible for the provider handle
 * @param[in] addr Mercury address of the provider
 * @param[in] provider_id id of the provider
 * @param[in] handle provider handle
 *
 * @return CACHE_SUCCESS or error code defined in cache-common.h
 */
int cache_provider_handle_create(
        cache_margo_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        cache_provider_handle_t* handle);

/**
 * @brief Increments the reference counter of a provider handle.
 *
 * @param handle provider handle
 *
 * @return CACHE_SUCCESS or error code defined in cache-common.h
 */
int cache_provider_handle_ref_incr(
        cache_provider_handle_t handle);

/**
 * @brief Releases the provider handle. This will decrement the
 * reference counter, and free the provider handle if the reference
 * counter reaches 0.
 *
 * @param[in] handle provider handle to release.
 *
 * @return CACHE_SUCCESS or error code defined in cache-common.h
 */
int cache_provider_handle_release(cache_provider_handle_t handle);

#endif
