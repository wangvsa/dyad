#ifndef DYAD_CACHE_MARGO_SERVER_H
#define DYAD_CACHE_MARGO_SERVER_H

#include <margo.h>
#include <dyad/service/cache/cache_common.h>

#define CACHE_ABT_POOL_DEFAULT ABT_POOL_NULL

typedef struct cache_provider* cache_provider_t;
#define CACHE_PROVIDER_NULL ((cache_provider_t)NULL)
#define CACHE_PROVIDER_IGNORE ((cache_provider_t*)NULL)

/**
 * @brief Creates a new CACHE provider. If CACHE_PROVIDER_IGNORE
 * is passed as last argument, the provider will be automatically
 * destroyed when calling :code:`margo_finalize`.
 *
 * @param[in] mid Margo instance
 * @param[in] pool Argobots pool
 * @param[out] provider provider handle
 *
 * @return CACHE_SUCCESS or error code defined in cache_common.h
 */
int cache_provider_register(
        margo_instance_id mid,
        ABT_pool pool,
        cache_provider_t* provider);

/**
 * @brief Destroys the Cache provider and deregisters its RPC.
 *
 * @param[in] provider Cache provider
 *
 * @return CACHE_SUCCESS or error code defined in cache_common.h
 */
int cache_provider_destroy(
        cache_provider_t provider);

#endif
