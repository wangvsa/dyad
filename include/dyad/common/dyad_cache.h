/**
 * @file dyad_cache.h
 * @brief Cache-eviction policy interface definitions for DYAD.
 *
 * @details
 * Defines the enumeration describing the available node-local cache
 * eviction policies for DYAD's producer/consumer managed directories
 * (DMDs). The eviction subsystem abstracts the choice of *which* cached
 * file to remove when a managed directory exceeds its configured
 * capacity, so the rest of DYAD does not need to know which policy is
 * active. This mirrors the Data Transport Layer (DTL) abstraction in
 * @c dyad_dtl.h.
 */

#ifndef DYAD_COMMON_DYAD_CACHE_H
#define DYAD_COMMON_DYAD_CACHE_H

#if defined(DYAD_HAS_CONFIG)
#include <dyad/dyad_config.hpp>
#else
#error "no config"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Available cache-eviction policies for DYAD-managed directories.
 *
 * @details
 * Selects how DYAD picks which cached file(s) to remove when a managed
 * directory exceeds @c DYAD_CACHE_CAPACITY. Each policy ranks candidate
 * files by a "recency key" derived from filesystem metadata (no
 * separate persisted index is kept):
 *
 * - @c DYAD_CACHE_NONE disables eviction entirely (the default). DYAD's
 *   managed directories grow unboundedly, matching all prior behavior.
 * - @c DYAD_CACHE_LRU evicts the least-recently-*accessed* file first,
 *   ranked by @c st_atime.
 * - @c DYAD_CACHE_FIFO evicts the oldest file first, ranked by
 *   @c st_mtime (files under DYAD's managed directories are effectively
 *   write-once, so mtime approximates insertion order).
 * - @c DYAD_CACHE_DEFAULT aliases @c DYAD_CACHE_LRU — the default
 *   *policy* once eviction is enabled (i.e. once
 *   @c DYAD_CACHE_CAPACITY is set to a nonzero value).
 * - @c DYAD_CACHE_END is a sentinel marking the end of valid values.
 *   It is used to size @c dyad_cache_policy_name and for bounds checking.
 */
enum dyad_cache_policy_mode {
    DYAD_CACHE_NONE = 0,     ///< Eviction disabled (default).
    DYAD_CACHE_LRU = 1,      ///< Evict least-recently-accessed file first (by st_atime).
    DYAD_CACHE_FIFO = 2,     ///< Evict oldest file first (by st_mtime).
    DYAD_CACHE_DEFAULT = 1,  ///< Default policy once enabled (alias for DYAD_CACHE_LRU).
    DYAD_CACHE_END = 3       ///< Sentinel — number of valid cache policy modes.
};
typedef enum dyad_cache_policy_mode dyad_cache_policy_mode_t;

/**
 * @brief Human-readable names for each cache-eviction policy mode.
 *
 * @details
 * Indexed by @c dyad_cache_policy_mode_t. The extra entry at index
 * @c DYAD_CACHE_END holds @c "CACHE_UNKNOWN" for out-of-range values.
 * Marked @c unused to suppress warnings when included in translation
 * units that do not reference it directly.
 */
static const char *dyad_cache_policy_name[DYAD_CACHE_END + 1]
    __attribute__ ((unused)) = {"NONE", "LRU", "FIFO", "CACHE_UNKNOWN"};

/**
 * @brief Opaque cache-eviction policy handle.
 *
 * @details
 * Forward declaration of the policy implementation struct. The full
 * definition is provided by each policy (LRU, FIFO) in its own source
 * file. Callers interact with the policy exclusively through the
 * function pointer embedded in the struct, allowing the policy to be
 * swapped at initialization time without changing call sites.
 *
 * @see dyad_cache_policy_mode_t for available policies.
 * @see dyad_ctx_t for the context that owns the cache policy handle.
 */
struct dyad_cache_policy;

#ifdef __cplusplus
}
#endif

#endif /* DYAD_COMMON_DYAD_CACHE_H */
