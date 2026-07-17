#if defined(DYAD_HAS_CONFIG)
#include <dyad/dyad_config.hpp>
#else
#error "no config"
#endif

#include <stdlib.h>

#include <dyad/cache/lru_cache.h>
#include <dyad/common/dyad_logging.h>
#include <dyad/common/dyad_profiler.h>

static dyad_rc_t dyad_cache_lru_get_recency_key (const dyad_ctx_t *ctx,
                                                 const struct stat *st,
                                                 int64_t *out)
{
    (void)ctx;
    if (st == NULL || out == NULL) {
        return DYAD_RC_BADBUF;
    }
    *out = (int64_t)st->st_atime;
    return DYAD_RC_OK;
}

dyad_rc_t dyad_cache_lru_init (dyad_ctx_t *ctx)
{
    DYAD_C_FUNCTION_START ();
    dyad_rc_t rc = DYAD_RC_OK;
    ctx->cache_policy->private_cache.lru_handle = malloc (sizeof (struct dyad_cache_lru));
    if (ctx->cache_policy->private_cache.lru_handle == NULL) {
        DYAD_LOG_ERROR (ctx, "Cannot allocate memory for LRU cache policy");
        rc = DYAD_RC_SYSFAIL;
        goto lru_init_done;
    }
    ctx->cache_policy->get_recency_key = dyad_cache_lru_get_recency_key;
lru_init_done:;
    DYAD_C_FUNCTION_END ();
    return rc;
}

dyad_rc_t dyad_cache_lru_finalize (dyad_ctx_t *ctx)
{
    DYAD_C_FUNCTION_START ();
    if (ctx->cache_policy != NULL) {
        free (ctx->cache_policy->private_cache.lru_handle);
        ctx->cache_policy->private_cache.lru_handle = NULL;
    }
    DYAD_C_FUNCTION_END ();
    return DYAD_RC_OK;
}
