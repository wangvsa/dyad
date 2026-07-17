/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#ifndef DYAD_STREAM_DYAD_PARAMS_HPP
#define DYAD_STREAM_DYAD_PARAMS_HPP

#if defined(DYAD_HAS_CONFIG)
#include <dyad/dyad_config.hpp>
#else
#error "no config"
#endif

#include <dyad/common/dyad_cache.h>
#include <dyad/common/dyad_dtl.h>
#include <cstdint>
#include <string>

namespace dyad
{
/**
 * @brief Configuration parameters for explicit DYAD context initialization.
 *
 * @details
 * Aggregates all configuration options for DYAD into a single object that
 * can be passed to @c dyad_stream_core::init(const dyad_params&) to
 * initialize the DYAD context programmatically, bypassing environment
 * variable-based configuration.
 *
 * This is the preferred initialization method when configuration must be
 * set explicitly in code rather than through environment variables, for
 * example in applications that manage their own configuration systems or
 * in testing scenarios where environment variables are not practical.
 *
 * Default values are set by the default constructor and reflect a minimal,
 * no-op configuration — no managed paths, no debug output, default DTL,
 * and conservative key depth and bin counts. Override only the fields
 * relevant to your use case.
 *
 * @note @c dyad_stream_core does not initialize the DYAD context
 *       automatically in its constructor, nor does it tear it down in its
 *       destructor. Initialization and teardown are costly operations, and
 *       C++ objects may trigger many implicit copy constructions and
 *       destructions. The caller is responsible for explicitly calling
 *       @c dyad_stream_core::init() once and managing the context lifetime.
 *
 * @see dyad_stream_core::init(const dyad_params&) for explicit initialization.
 */
struct dyad_params {
    bool m_debug;
    /** Indicate if the storage associated with the managed path is shared
     * (i.e. visible to all ranks) */
    bool m_shared_storage;
    /// Indicate if reinitialization is required even if already initinialized
    bool m_reinit;
    /// Enable asynchronous publish by producer
    bool m_async_publish;
    /// Apply fsync after write by producer
    bool m_fsync_write;
    /// The depth of the key hierarchy for path
    unsigned int m_key_depth;
    /// The number of bins used in key hashing
    unsigned int m_key_bins;
    /// The number of brokers sharing node-local storage
    unsigned int m_service_mux;
    /// The DTL to use to move data from producer to consumer
    /// Valid values can be found in dyad_dtl_defs.h from core
    int m_dtl_mode;

    /// The KVS namespace of the sharing context
    std::string m_kvs_namespace;

    /// The path managed by DYAD for consumer
    std::string m_cons_managed_path;
    /// The path managed by DYAD for producer
    std::string m_prod_managed_path;
    /// A relative path is relative to a managed path
    bool m_relative_to_managed_path;

    /// Maximum bytes DYAD may use in a managed directory before evicting
    /// cached files. 0 (default) disables eviction.
    uint64_t m_cache_capacity_bytes;
    /// Cache-eviction policy name ("LRU", "FIFO", or "NONE"). Ignored if
    /// m_cache_capacity_bytes is 0.
    std::string m_cache_policy;
    /// Fraction (0..1) of m_cache_capacity_bytes to evict down to once
    /// eviction triggers.
    double m_cache_low_watermark;
    /// Skip eviction candidates accessed more recently than this many seconds.
    unsigned int m_cache_grace_period_sec;

    /// Optional fallback source path (e.g. on the parallel file system) used
    /// by dyad_range_cache_ensure() to lazily fill missing spans of a
    /// managed file on demand. Empty (default) disables lazy origin-backed
    /// caching.
    std::string m_origin_path;

    /**
     * @brief Constructs a @c dyad_params object with safe default values.
     *
     * @details
     * All boolean flags default to @c false, managed paths default to empty
     * strings (disabling producer and consumer operations), key depth and
     * bins default to conservative values (2 and 256), service multiplexer
     * defaults to 1 (one broker per node), and the DTL defaults to
     * @c DYAD_DTL_DEFAULT.
     */
    dyad_params ()
        : m_debug (false),
          m_shared_storage (false),
          m_reinit (false),
          m_async_publish (false),
          m_fsync_write (false),
          m_key_depth (2u),
          m_key_bins (256u),
          m_service_mux (1u),
          m_dtl_mode (DYAD_DTL_DEFAULT),
          m_kvs_namespace (""),
          m_cons_managed_path (""),
          m_prod_managed_path (""),
          m_relative_to_managed_path (false),
          m_cache_capacity_bytes (0ull),
          m_cache_policy (dyad_cache_policy_name[DYAD_CACHE_NONE]),
          m_cache_low_watermark (0.8),
          m_cache_grace_period_sec (5u),
          m_origin_path ("")
    {
    }
};

}  // end of namespace dyad
#endif  // DYAD_STREAM_DYAD_PARAMS_HPP
