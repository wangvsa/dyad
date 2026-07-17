#ifndef DYAD_CACHE_LRU_CACHE_H
#define DYAD_CACHE_LRU_CACHE_H

#if defined(DYAD_HAS_CONFIG)
#include <dyad/dyad_config.hpp>
#else
#error "no config"
#endif

#include <dyad/cache/dyad_cache_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Internal state for the LRU cache-eviction policy.
 *
 * @details
 * Carries no fields: LRU's recency key (@c st_atime) is read directly
 * from each candidate's @c stat() result at scan time, so there is
 * nothing to persist between calls. The struct exists only to give
 * @c dyad_cache_private_t a distinct, named member per policy.
 */
struct dyad_cache_lru {
    int _unused;
};

/**
 * @brief Initializes the LRU cache-eviction policy.
 *
 * @details
 * Allocates @c ctx->cache_policy->private_cache.lru_handle and wires
 * @c ctx->cache_policy->get_recency_key to @c dyad_cache_lru_get_recency_key().
 *
 * @param[in,out] ctx DYAD context. @c ctx->cache_policy must already be
 *                    allocated by @c dyad_cache_policy_init().
 * @return @c DYAD_RC_OK on success, or @c DYAD_RC_SYSFAIL on allocation failure.
 */
dyad_rc_t dyad_cache_lru_init (dyad_ctx_t *ctx);

/**
 * @brief Finalizes the LRU cache-eviction policy.
 * @param[in,out] ctx DYAD context.
 * @return @c DYAD_RC_OK always.
 */
dyad_rc_t dyad_cache_lru_finalize (dyad_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* DYAD_CACHE_LRU_CACHE_H */
