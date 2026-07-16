#if defined(DYAD_HAS_CONFIG)
#include <dyad/dyad_config.hpp>
#else
#error "no config"
#endif

#include <stdlib.h>

#include <dyad/cache/dyad_cache_api.h>
#include <dyad/cache/fifo_cache.h>
#include <dyad/cache/lru_cache.h>
#include <dyad/common/dyad_logging.h>
#include <dyad/common/dyad_profiler.h>

dyad_rc_t dyad_cache_policy_init (dyad_ctx_t *ctx, dyad_cache_policy_mode_t mode)
{
    DYAD_C_FUNCTION_START ();
    dyad_rc_t rc = DYAD_RC_OK;
    ctx->cache_policy = malloc (sizeof (struct dyad_cache_policy));
    if (ctx->cache_policy == NULL) {
        rc = DYAD_RC_SYSFAIL;
        DYAD_LOG_ERROR (ctx, "Cannot allocate memory for cache policy handle");
        goto cache_policy_init_done;
    }
    ctx->cache_policy->mode = mode;
    ctx->cache_policy->get_recency_key = NULL;
    // No compile-time gating here (unlike the DTL backends): LRU and FIFO
    // have no external dependencies and are always available.
    if (mode == DYAD_CACHE_LRU) {
        rc = dyad_cache_lru_init (ctx);
    } else if (mode == DYAD_CACHE_FIFO) {
        rc = dyad_cache_fifo_init (ctx);
    } else if (mode == DYAD_CACHE_NONE) {
        rc = DYAD_RC_OK;
    } else {
        rc = DYAD_RC_BADCACHEMODE;
    }

cache_policy_init_done:;
    DYAD_C_FUNCTION_END ();
    return rc;
}

dyad_rc_t dyad_cache_policy_finalize (dyad_ctx_t *ctx)
{
    DYAD_C_FUNCTION_START ();
    dyad_rc_t rc = DYAD_RC_OK;
    if (ctx->cache_policy == NULL) {
        rc = DYAD_RC_OK;
        goto cache_policy_finalize_done;
    }
    if (ctx->cache_policy->mode == DYAD_CACHE_LRU) {
        dyad_cache_lru_finalize (ctx);
    } else if (ctx->cache_policy->mode == DYAD_CACHE_FIFO) {
        dyad_cache_fifo_finalize (ctx);
    }
    rc = DYAD_RC_OK;

cache_policy_finalize_done:;
    free (ctx->cache_policy);
    ctx->cache_policy = NULL;
    DYAD_C_FUNCTION_END ();
    return rc;
}
