/**
 * @file dyad_dtl.h
 * @brief Data Transport Layer (DTL) interface definitions for DYAD.
 *
 * @details
 * Defines the enumerations, type aliases, and constants that describe
 * the available data transport backends and their communication modes.
 * The DTL abstracts the underlying data movement mechanism so that the
 * rest of DYAD can transfer file data between producer and consumer
 * without being coupled to a specific transport library.
 */

#ifndef DYAD_COMMON_DYAD_DTL_H
#define DYAD_COMMON_DYAD_DTL_H

#if defined(DYAD_HAS_CONFIG)
#include <dyad/dyad_config.hpp>
#else
#error "no config"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Available data transport backends.
 *
 * @details
 * Selects the underlying library used to move file data from the
 * producer's broker to the consumer. Each backend offers different
 * performance and portability trade-offs:
 *
 * - @c DYAD_DTL_UCX uses UCX (Unified Communication X) with RDMA
 *   for high-performance inter-node data movement. Requires UCX to
 *   be available at build time.
 * - @c DYAD_DTL_MARGO uses the Margo/Mercury RPC framework for
 *   data transport.
 * - @c DYAD_DTL_FLUX_RPC uses Flux's built-in streaming RPC
 *   mechanism. This is the default and requires no additional
 *   dependencies beyond Flux itself.
 * - @c DYAD_DTL_DEFAULT aliases @c DYAD_DTL_FLUX_RPC.
 * - @c DYAD_DTL_END is a sentinel marking the end of valid values.
 *   It is used to size @c dyad_dtl_mode_name and for bounds checking.
 */
enum dyad_dtl_mode {
    DYAD_DTL_UCX = 0,       ///< UCX RDMA-capable transport backend.
    DYAD_DTL_MARGO = 1,     ///< Margo/Mercury RPC transport backend.
    DYAD_DTL_FLUX_RPC = 2,  ///< Flux streaming RPC transport backend.
    DYAD_DTL_DEFAULT = 2,   ///< Default transport (alias for @c DYAD_DTL_FLUX_RPC).
    DYAD_DTL_END = 3        ///< Sentinel — number of valid DTL modes.
};
typedef enum dyad_dtl_mode dyad_dtl_mode_t;

/**
 * @brief Human-readable names for each DTL mode.
 *
 * @details
 * Indexed by @c dyad_dtl_mode_t. The extra entry at index
 * @c DYAD_DTL_END holds @c "DTL_UNKNOWN" for out-of-range values.
 * Marked @c unused to suppress warnings when included in translation
 * units that do not reference it directly.
 */
static const char* dyad_dtl_mode_name[DYAD_DTL_END + 1]
    __attribute__ ((unused)) = {"UCX", "MARGO", "FLUX_RPC", "DTL_UNKNOWN"};

/**
 * @brief Communication direction for a DTL connection.
 *
 * @details
 * Controls whether the DTL handle is configured to send or receive
 * data. A single DTL handle is unidirectional — producers initialize
 * with @c DYAD_COMM_SEND and consumers with @c DYAD_COMM_RECV.
 * @c DYAD_COMM_NONE is a sanity-check value used to detect
 * uninitialized or torn-down connections.
 */
enum dyad_dtl_comm_mode {
    DYAD_COMM_NONE = 0,  ///< No connection established (uninitialized).
    DYAD_COMM_RECV = 1,  ///< Connection to receive data (consumer).
    DYAD_COMM_SEND = 2,  ///< Connection to send data (producer / service).
    DYAD_COMM_END = 3    ///< Sentinel — number of valid comm modes.
};
typedef enum dyad_dtl_comm_mode dyad_dtl_comm_mode_t;

/**
 * @brief Flux RPC topic name for DYAD file fetch requests.
 *
 * @details
 * The consumer sends a Flux RPC to this topic to request a file from
 * the producer's broker. The producer's DYAD service registers a
 * handler for this topic via @c flux_msg_handler_addvec(). Both sides
 * must use the same string for requests to be dispatched correctly.
 */
#define DYAD_DTL_RPC_NAME "dyad.fetch"

/**
 * @brief Flux RPC topic name for DYAD byte-range fetch requests.
 *
 * @details
 * Like @c DYAD_DTL_RPC_NAME, but for @c dyad_consume_range() requests
 * (FLUX_RPC and MARGO DTL modes only). Registered as a separate topic so
 * the existing whole-file @c DYAD_DTL_RPC_NAME handler is untouched.
 */
#define DYAD_DTL_RPC_RANGE_NAME "dyad.fetch_range"

/**
 * @brief Opaque DTL handle.
 *
 * @details
 * Forward declaration of the DTL implementation struct. The full
 * definition is provided by each backend (UCX, Margo, Flux RPC) in
 * its own source file. Callers interact with the DTL exclusively
 * through the function pointers embedded in the struct, allowing
 * the backend to be swapped at initialization time without changing
 * call sites.
 *
 * @see dyad_dtl_mode_t for available backends.
 * @see dyad_ctx_t for the context that owns the DTL handle.
 */
struct dyad_dtl;

#ifdef __cplusplus
}
#endif

#endif /* DYAD_COMMON_DYAD_DTL_H */
