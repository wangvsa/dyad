#ifndef DYAD_CACHE_DYAD_CACHE_API_H
#define DYAD_CACHE_DYAD_CACHE_API_H

#if defined(DYAD_HAS_CONFIG)
#include <dyad/dyad_config.hpp>
#else
#error "no config"
#endif

// clang-format off
#include <sys/stat.h>
#include <dyad/common/dyad_cache.h>
#include <dyad/common/dyad_rc.h>
#include <dyad/common/dyad_structures_int.h>
// clang-format on

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Internal state for the LRU cache-eviction policy.
 * @see dyad_cache_private_t
 */
struct dyad_cache_lru;

/**
 * @brief Internal state for the FIFO cache-eviction policy.
 * @see dyad_cache_private_t
 */
struct dyad_cache_fifo;

/**
 * @brief Union holding a pointer to the internal state of the active
 *        cache-eviction policy.
 *
 * @details
 * Only one member is valid at a time, selected by the @c mode field of
 * the owning @c dyad_cache_policy struct. LRU and FIFO carry no state
 * beyond their mode (both are derived purely from filesystem metadata
 * at scan time), so these structs are currently empty placeholders —
 * the union exists so a future, richer policy (e.g. one that needs a
 * persisted access-count for LFU) can add real state without another
 * interface change.
 */
union dyad_cache_private {
    struct dyad_cache_lru *lru_handle;
    struct dyad_cache_fifo *fifo_handle;
} __attribute__ ((aligned (16)));
typedef union dyad_cache_private dyad_cache_private_t;

/**
 * @brief Cache-eviction policy handle.
 *
 * @details
 * Provides a uniform interface for all eviction policies through a
 * single function pointer. The active policy is selected at
 * initialization time by @c dyad_cache_policy_init() and stored in
 * @c private_cache. Mirrors the @c dyad_dtl struct-of-function-pointers
 * pattern used for the Data Transport Layer (@c dyad_dtl_api.h), scoped
 * down to what LRU/FIFO actually need.
 */
struct dyad_cache_policy {
    dyad_cache_private_t private_cache;  ///< Opaque pointer to the active policy's state.
    dyad_cache_policy_mode_t mode;       ///< Active policy. @see dyad_cache_policy_mode_t.

    /**
     * @brief Computes the recency key used to rank an eviction candidate.
     *
     * @details
     * Smaller values are evicted first. LRU returns @c st->st_atime;
     * FIFO returns @c st->st_mtime.
     *
     * @param[in]  ctx DYAD context.
     * @param[in]  st  @c stat() result for the candidate file.
     * @param[out] out Recency key; smaller is evicted first.
     * @return @c DYAD_RC_OK on success, or an error code on failure.
     */
    dyad_rc_t (*get_recency_key) (const dyad_ctx_t *ctx, const struct stat *st, int64_t *out);

} __attribute__ ((aligned (256)));
typedef struct dyad_cache_policy dyad_cache_policy_t;

/**
 * @brief Initializes the cache-eviction policy for a DYAD context.
 *
 * @details
 * Allocates the @c cache_policy handle inside @p ctx and wires its
 * @c get_recency_key function pointer based on @p mode. Unlike the DTL
 * backends, LRU and FIFO have no external dependencies and thus no
 * compile-time gating — both are always available.
 *
 * @param[in,out] ctx  DYAD context. On success, @c ctx->cache_policy is
 *                     allocated and initialized.
 * @param[in]     mode Eviction policy to use. @c DYAD_CACHE_NONE is a
 *                     valid mode: initialization still succeeds, but
 *                     @c dyad_cache_maybe_evict() will no-op.
 *
 * @return @c dyad_rc_t return code:
 * @retval DYAD_RC_OK      Initialization succeeded.
 * @retval DYAD_RC_SYSFAIL Failed to allocate the @c dyad_cache_policy handle.
 */
dyad_rc_t dyad_cache_policy_init (dyad_ctx_t *ctx, dyad_cache_policy_mode_t mode);

/**
 * @brief Finalizes and frees the cache-eviction policy for a DYAD context.
 *
 * @details
 * Frees @c ctx->cache_policy and sets it to @c NULL. If already @c NULL,
 * this is a no-op that returns @c DYAD_RC_OK.
 *
 * @param[in,out] ctx DYAD context. On return, @c ctx->cache_policy is @c NULL.
 * @return @c DYAD_RC_OK always (finalization errors are non-recoverable
 *         and should not block teardown).
 */
dyad_rc_t dyad_cache_policy_finalize (dyad_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* DYAD_CACHE_DYAD_CACHE_API_H */
