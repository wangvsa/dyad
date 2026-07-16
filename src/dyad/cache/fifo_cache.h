#ifndef DYAD_CACHE_FIFO_CACHE_H
#define DYAD_CACHE_FIFO_CACHE_H

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
 * @brief Internal state for the FIFO cache-eviction policy.
 *
 * @details
 * Carries no fields: FIFO's recency key (@c st_mtime) is read directly
 * from each candidate's @c stat() result at scan time, so there is
 * nothing to persist between calls. The struct exists only to give
 * @c dyad_cache_private_t a distinct, named member per policy.
 */
struct dyad_cache_fifo {
    int _unused;
};

/**
 * @brief Initializes the FIFO cache-eviction policy.
 *
 * @details
 * Allocates @c ctx->cache_policy->private_cache.fifo_handle and wires
 * @c ctx->cache_policy->get_recency_key to @c dyad_cache_fifo_get_recency_key().
 *
 * @param[in,out] ctx DYAD context. @c ctx->cache_policy must already be
 *                    allocated by @c dyad_cache_policy_init().
 * @return @c DYAD_RC_OK on success, or @c DYAD_RC_SYSFAIL on allocation failure.
 */
dyad_rc_t dyad_cache_fifo_init (dyad_ctx_t *ctx);

/**
 * @brief Finalizes the FIFO cache-eviction policy.
 * @param[in,out] ctx DYAD context.
 * @return @c DYAD_RC_OK always.
 */
dyad_rc_t dyad_cache_fifo_finalize (dyad_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* DYAD_CACHE_FIFO_CACHE_H */
