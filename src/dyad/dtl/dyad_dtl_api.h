#ifndef DYAD_DTL_DYAD_DTL_IMPL_H
#define DYAD_DTL_DYAD_DTL_IMPL_H

#if defined(DYAD_HAS_CONFIG)
#include <dyad/dyad_config.hpp>
#else
#error "no config"
#endif

// clang-format off
#include <jansson.h>
#include <sys/types.h>
#include <dyad/common/dyad_structures_int.h>
#include <dyad/common/dyad_dtl.h>
#include <dyad/common/dyad_rc.h>
#include <flux/core.h>
// clang-format on

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#include <dyad/common/dyad_dtl.h>
#include <dyad/common/dyad_rc.h>
#include <dyad/common/dyad_structures.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Internal state for the UCX DTL backend.
 * @see dyad_dtl_private_t
 */
struct dyad_dtl_ucx;

/**
 * @brief Internal state for the Flux RPC DTL backend.
 * @see dyad_dtl_private_t
 */
struct dyad_dtl_flux;

/**
 * @brief Union holding a pointer to the internal state of the active
 *        DTL backend.
 *
 * @details
 * Only one member is valid at a time, selected by the @c mode field of
 * the owning @c dyad_dtl struct. Each member points to a struct that
 * holds all state needed by that transport backend — for example, UCX
 * endpoint handles and memory registrations, or Flux RPC futures and
 * buffers. The union is aligned to 16 bytes to satisfy the alignment
 * requirements of the underlying backend structs.
 */
union dyad_dtl_private {
    struct dyad_dtl_ucx *ucx_dtl_handle;
    struct dyad_dtl_flux *flux_dtl_handle;
    struct dyad_dtl_margo *margo_dtl_handle;
} __attribute__ ((aligned (16)));
typedef union dyad_dtl_private dyad_dtl_private_t;

/**
 * @brief Data Transport Layer handle.
 *
 * @details
 * Provides a uniform interface for all DTL backends through a table of
 * function pointers. The active backend is selected at initialization
 * time by @c dyad_dtl_init() and stored in @c private_dtl. All function
 * pointers are set by the backend-specific init function and must not be
 * called before initialization or after finalization.
 *
 * The struct is aligned to 256 bytes to avoid false sharing across cache
 * lines in multi-threaded access patterns.
 *
 * The function pointer protocol for a consumer fetch is:
 * @code
 * rpc_pack()           // consumer: pack the fetch request
 * rpc_recv_response()  // consumer: wait for initial RPC acknowledgement
 * establish_connection() // consumer: set up DTL data channel
 * get_buffer()         // consumer: allocate receive buffer
 * recv()               // consumer: receive file data
 * return_buffer()      // consumer: release receive buffer
 * close_connection()   // consumer: tear down DTL data channel
 * @endcode
 *
 * And for the producer service side:
 * @code
 * rpc_unpack()         // service: unpack the incoming fetch request
 * rpc_respond()        // service: send initial RPC acknowledgement
 * get_buffer()         // service: allocate send buffer
 * establish_connection() // service: set up DTL data channel
 * send()               // service: send file data
 * return_buffer()      // service: release send buffer
 * close_connection()   // service: tear down DTL data channel
 * @endcode
 */
struct dyad_dtl {
    dyad_dtl_private_t private_dtl;  ///< Opaque pointer to the active backend context.
    dyad_dtl_mode_t mode;            ///< Active DTL backend. @see dyad_dtl_mode_t.

    /**
     * @brief Packs a file fetch request into a JSON object for an RPC call.
     *
     * @param[in]  ctx           DYAD context.
     * @param[in]  upath         Relative path of the file to fetch.
     * @param[in]  producer_rank Flux rank of the producer broker.
     * @param[out] packed_obj    JSON object containing the packed request.
     * @return @c DYAD_RC_OK on success, or an error code on failure.
     */
    dyad_rc_t (*rpc_pack) (const dyad_ctx_t *ctx,
                           const char *upath,
                           uint32_t producer_rank,
                           json_t **packed_obj);

    /**
     * @brief Unpacks a file fetch request from an incoming Flux RPC message.
     *
     * @param[in]  ctx        DYAD context.
     * @param[in]  packed_obj Incoming Flux RPC message.
     * @param[out] upath      Relative path of the requested file.
     * @return @c DYAD_RC_OK on success, or an error code on failure.
     */
    dyad_rc_t (*rpc_unpack) (const dyad_ctx_t *ctx, const flux_msg_t *packed_obj, char **upath);

    /**
     * @brief Sends the initial RPC acknowledgement from the service to the consumer.
     *
     * @param[in] ctx      DYAD context.
     * @param[in] orig_msg The incoming Flux RPC message to respond to.
     * @return @c DYAD_RC_OK on success, or an error code on failure.
     */
    dyad_rc_t (*rpc_respond) (const dyad_ctx_t *ctx, const flux_msg_t *orig_msg);

    /**
     * @brief Waits for and receives the initial RPC response from the service.
     *
     * @param[in] ctx DYAD context.
     * @param[in] f   Flux future representing the pending RPC response.
     * @return @c DYAD_RC_OK on success, or an error code on failure.
     */
    dyad_rc_t (*rpc_recv_response) (const dyad_ctx_t *ctx, flux_future_t *f);

    /**
     * @brief Allocates a backend-managed buffer for data transfer.
     *
     * @details
     * For UCX this may be a pinned RDMA-registered buffer. For Flux RPC
     * it is a plain heap allocation. The buffer must be released via
     * @c return_buffer() when no longer needed.
     *
     * @param[in]  ctx       DYAD context.
     * @param[in]  data_size Required buffer size in bytes.
     * @param[out] data_buf  Pointer set to the allocated buffer.
     * @return @c DYAD_RC_OK on success, or an error code on failure.
     */
    dyad_rc_t (*get_buffer) (const dyad_ctx_t *ctx, size_t data_size, void **data_buf);

    /**
     * @brief Releases a buffer previously allocated by @c get_buffer().
     *
     * @param[in]     ctx      DYAD context.
     * @param[in,out] data_buf Pointer to the buffer to release. Set to
     *                         @c NULL on return.
     * @return @c DYAD_RC_OK on success, or an error code on failure.
     */
    dyad_rc_t (*return_buffer) (const dyad_ctx_t *ctx, void **data_buf);

    /**
     * @brief Establishes the DTL data channel between producer and consumer.
     *
     * @details
     * For UCX this creates a @c ucp_ep_h endpoint. For Flux RPC no
     * additional connection setup is required beyond the RPC itself.
     *
     * @param[in] ctx DYAD context.
     * @return @c DYAD_RC_OK on success, or an error code on failure.
     */
    dyad_rc_t (*establish_connection) (const dyad_ctx_t *ctx);

    /**
     * @brief Sends file data to the consumer over the DTL data channel.
     *
     * @param[in] ctx    DYAD context.
     * @param[in] buf    Buffer containing the data to send.
     * @param[in] buflen Number of bytes to send.
     * @return @c DYAD_RC_OK on success, or an error code on failure.
     */
    dyad_rc_t (*send) (const dyad_ctx_t *ctx, void *buf, size_t buflen);

    /**
     * @brief Receives file data from the producer over the DTL data channel.
     *
     * @param[in]  ctx    DYAD context.
     * @param[out] buf    Set to the buffer containing the received data.
     * @param[out] buflen Set to the number of bytes received.
     * @return @c DYAD_RC_OK on success, or an error code on failure.
     */
    dyad_rc_t (*recv) (const dyad_ctx_t *ctx, void **buf, size_t *buflen);

    /**
     * @brief Tears down the DTL data channel.
     *
     * @param[in] ctx DYAD context.
     * @return @c DYAD_RC_OK on success, or an error code on failure.
     */
    dyad_rc_t (*close_connection) (const dyad_ctx_t *ctx);

} __attribute__ ((aligned (256)));
typedef struct dyad_dtl dyad_dtl_t;

/**
 * @brief Initializes the Data Transport Layer for a DYAD context.
 *
 * @details
 * Allocates the @c dyad_dtl handle inside @p ctx and delegates to the
 * backend-specific initialization function based on @p mode:
 *
 * - @c DYAD_DTL_UCX      → @c dyad_dtl_ucx_init()
 *   (only if built with @c DYAD_ENABLE_UCX_DTL)
 * - @c DYAD_DTL_MARGO    → @c dyad_dtl_margo_init()
 *   (only if built with @c DYAD_ENABLE_MARGO_DTL)
 * - @c DYAD_DTL_FLUX_RPC → @c dyad_dtl_flux_init()
 *   (always available)
 *
 * If @p mode does not match any enabled backend, returns
 * @c DYAD_RC_BADDTLMODE without initializing the handle.
 *
 * @param[in,out] ctx       DYAD context. On success, @c ctx->dtl_handle
 *                          is allocated and initialized.
 * @param[in]     mode      DTL backend to use. @see dyad_dtl_mode_t.
 * @param[in]     comm_mode Communication direction (@c DYAD_COMM_SEND
 *                          for producer, @c DYAD_COMM_RECV for consumer).
 *                          @see dyad_dtl_comm_mode_t.
 * @param[in]     debug     If @c true, enables verbose debug logging
 *                          in the DTL backend.
 *
 * @return @c dyad_rc_t return code:
 * @retval DYAD_RC_OK        Initialization succeeded.
 * @retval DYAD_RC_SYSFAIL   Failed to allocate the @c dyad_dtl handle.
 * @retval DYAD_RC_BADDTLMODE @p mode does not match any enabled backend.
 * @retval other             Any error code returned by the backend-specific
 *                           init function.
 */
dyad_rc_t dyad_dtl_init (dyad_ctx_t *ctx,
                         dyad_dtl_mode_t mode,
                         dyad_dtl_comm_mode_t comm_mode,
                         bool debug);

/**
 * @brief Finalizes and frees the Data Transport Layer for a DYAD context.
 *
 * @details
 * Delegates to the backend-specific finalization function based on
 * @c ctx->dtl_handle->mode, then frees the @c dyad_dtl handle and sets
 * @c ctx->dtl_handle to @c NULL.
 *
 * If @c ctx->dtl_handle is already @c NULL, the function is a no-op and
 * returns @c DYAD_RC_OK. This allows @c dyad_dtl_finalize() to be called
 * safely on an already-finalized context without error.
 *
 * Backend dispatch:
 * - @c DYAD_DTL_UCX      → @c dyad_dtl_ucx_finalize()
 *   (only if built with @c DYAD_ENABLE_UCX_DTL and the UCX handle is
 *   non-@c NULL)
 * - @c DYAD_DTL_MARGO    → @c dyad_dtl_margo_finalize()
 *   (only if built with @c DYAD_ENABLE_MARGO_DTL)
 * - @c DYAD_DTL_FLUX_RPC → @c dyad_dtl_flux_finalize()
 *   (only if the Flux handle is non-@c NULL)
 *
 * @note The @c dyad_dtl handle is always freed and set to @c NULL
 *       regardless of whether the backend finalization succeeds or fails.
 *       Any error code returned by the backend is overwritten with
 *       @c DYAD_RC_OK before returning — this is intentional, as
 *       finalization errors are non-recoverable and should not prevent
 *       the caller from continuing teardown.
 *
 * @param[in,out] ctx  DYAD context. On return, @c ctx->dtl_handle is
 *                     @c NULL.
 *
 * @return @c dyad_rc_t return code:
 * @retval DYAD_RC_OK         Finalization succeeded, or the handle was
 *                            already @c NULL.
 * @retval DYAD_RC_BADDTLMODE The mode stored in the handle does not match
 *                            any enabled backend. The handle is still freed.
 */
dyad_rc_t dyad_dtl_finalize (dyad_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* DYAD_DTL_DYAD_DTL_IMPL_H */
