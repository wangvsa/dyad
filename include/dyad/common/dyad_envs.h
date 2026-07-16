/**
 * @file dyad_envs.h
 * @brief Environment variable name constants for DYAD configuration.
 *
 * @details
 * Lists all environment variable names used to configure DYAD
 * at runtime. These are read by @c dyad_init_env() during context
 * initialization and some may be overridden by command-line arguments
 * in the Flux module via @c dyad_module_ctx_init().
 */

#ifndef DYAD_COMMON_DYAD_ENVS_H
#define DYAD_COMMON_DYAD_ENVS_H

#if defined(DYAD_HAS_CONFIG)
#include <dyad/dyad_config.hpp>
#else
#error "no config"
#endif

/**
 * @brief Path to the producer-managed directory (DMD) on the local node.
 */
#define DYAD_PATH_PRODUCER_ENV "DYAD_PATH_PRODUCER"

/**
 * @brief Path to the consumer-managed directory (DMD) on the local node.
 */
#define DYAD_PATH_CONSUMER_ENV "DYAD_PATH_CONSUMER"

/**
 * @brief If set, file paths are interpreted as relative to the managed path.
 */
#define DYAD_PATH_RELATIVE_ENV "DYAD_PATH_RELATIVE"

/**
 * @brief Flux KVS namespace used to scope DYAD metadata entries.
 */
#define DYAD_KVS_NAMESPACE_ENV "DYAD_KVS_NAMESPACE"

/**
 * @brief Data Transport Layer mode. Valid values: @c UCX, @c MARGO, @c FLUX_RPC.
 * @see dyad_dtl_mode_t
 */
#define DYAD_DTL_MODE_ENV "DYAD_DTL_MODE"

/**
 * @brief If set, the managed path is on shared storage visible to all nodes,
 *        removing the need for inter-node data transfer.
 */
#define DYAD_SHARED_STORAGE_ENV "DYAD_SHARED_STORAGE"

/**
 * @brief If set, KVS metadata publication by the producer is performed
 *        asynchronously, allowing the producer to continue without blocking.
 */
#define DYAD_ASYNC_PUBLISH_ENV "DYAD_ASYNC_PUBLISH"

/**
 * @brief If set, the producer calls @c fsync() on the file descriptor before
 *        publishing metadata to the KVS, ensuring data is durable on disk.
 */
#define DYAD_FSYNC_WRITE_ENV "DYAD_FSYNC_WRITE"

/**
 * @brief Depth of the hierarchical KVS key tree used to shard metadata entries.
 *
 * @details
 * Higher values reduce hot-spot contention at the cost of deeper lookups.
 * @see gen_path_key()
 */
#define DYAD_KEY_DEPTH_ENV "DYAD_KEY_DEPTH"

/**
 * @brief Number of bins (buckets) per level of the KVS key hierarchy.
 *
 * @details
 * Combined with @c DYAD_KEY_DEPTH, controls key distribution.
 * @see gen_path_key()
 */
#define DYAD_KEY_BINS_ENV "DYAD_KEY_BINS"

/**
 * @brief If set, DYAD records a marker file after a successful consumer fetch
 *        to indicate that the file has been synchronized. Used for health checks.
 */
#define DYAD_CHECK_ENV "DYAD_SYNC_HEALTH"

/**
 * @brief If set, enables synchronization health checking in the consumer path.
 */
#define DYAD_SYNC_CHECK_ENV "DYAD_SYNC_CHECK"

/**
 * @brief If set, enables verbose debug logging for synchronization operations.
 */
#define DYAD_SYNC_DEBUG_ENV "DYAD_SYNC_DEBUG"

/**
 * @brief Number of Flux broker ranks sharing a single DYAD service instance
 *        on a node.
 *
 * @details
 * Used to compute the node index from a rank: @c node_idx = rank / service_mux.
 */
#define DYAD_SERVICE_MUX_ENV "DYAD_SERVICE_MUX"

/**
 * @brief If set, forces reinitialization of the DYAD context even if it has
 *        already been initialized.
 *
 * @details
 * Used by @c dyad_stream_core::init(bool reinit).
 */
#define DYAD_REINIT_ENV "DYAD_REINIT"

/**
 * @brief Priority value for the GOTCHA interception hooks.
 *
 * @details
 * Controls the order in which DYAD's wrappers are called relative to
 * other GOTCHA users.
 */
#define DYAD_GOTCHA_PRIORITY_ENV "DYAD_GOTCHA_PRIORITY"

/**
 * @brief Mercury/Margo protocol string used when the Margo DTL backend
 *        is selected.
 *
 * @details
 * Example values: @c "ofi+tcp", @c "ofi+verbs", @c "ucx+tcp://", @c "ucx+dc://".
 */
#define DYAD_MARGO_PROTO_ENV "DYAD_MARGO_PROTO"

/**
 * @brief Maximum bytes DYAD may use in a managed directory before evicting
 *        cached files.
 *
 * @details
 * @c 0 (the default) disables eviction entirely, preserving prior
 * behavior for deployments that don't opt in.
 */
#define DYAD_CACHE_CAPACITY_ENV "DYAD_CACHE_CAPACITY"

/**
 * @brief Cache-eviction policy to use once @c DYAD_CACHE_CAPACITY is set.
 *
 * @details
 * Valid values: @c LRU (default), @c FIFO, @c NONE. Ignored if
 * @c DYAD_CACHE_CAPACITY is @c 0.
 */
#define DYAD_CACHE_POLICY_ENV "DYAD_CACHE_POLICY"

/**
 * @brief Fraction (0..1) of @c DYAD_CACHE_CAPACITY to evict down to once
 *        eviction triggers.
 *
 * @details
 * Evicting to a low-water mark rather than the exact capacity limit
 * avoids evicting on every single produce/consume call while usage
 * hovers at the boundary. Default: @c 0.8.
 */
#define DYAD_CACHE_LOW_WATERMARK_ENV "DYAD_CACHE_LOW_WATERMARK"

/**
 * @brief Skip eviction candidates accessed more recently than this many
 *        seconds.
 *
 * @details
 * Guards against evicting a file that may still be mid-access by another
 * process. Default: @c 5.
 */
#define DYAD_CACHE_GRACE_PERIOD_ENV "DYAD_CACHE_GRACE_PERIOD"

/**
 * @brief Fallback source path (e.g. on the parallel file system) used by
 *        @c dyad_range_cache_ensure() to lazily fill missing spans of a
 *        DYAD-managed file on demand.
 *
 * @details
 * Unset (the default) preserves prior behavior: managed files must already
 * be fully present locally (e.g. pre-staged), and byte-range fetch never
 * consults an origin.
 */
#define DYAD_PATH_ORIGIN_ENV "DYAD_PATH_ORIGIN"

#endif  // DYAD_COMMON_DYAD_ENVS_H
