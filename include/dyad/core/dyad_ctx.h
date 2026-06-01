#ifndef DYAD_CORE_DYAD_CTX_H
#define DYAD_CORE_DYAD_CTX_H

#if defined(DYAD_HAS_CONFIG)
#include <dyad/dyad_config.hpp>
#else
#error "no config"
#endif

#include <dyad/common/dyad_dtl.h>
#include <dyad/common/dyad_rc.h>
#include <dyad/common/dyad_structures.h>

#ifdef __cplusplus
extern "C" {
#endif

DYAD_DLL_EXPORTED extern const struct dyad_ctx dyad_ctx_default;

/** Returns a pointer to the thread-local DYAD context, or @c NULL if not
 *  initialized. The pointer is stored as a static thread-local variable to
 *  prevent multiple initializations and ensure each thread maintains an
 *  independent pointer to its own context instance. */
DYAD_DLL_EXPORTED dyad_ctx_t *dyad_ctx_get (void);

/**
 * @brief Initializes the DYAD context from environment variables.
 *
 * @details
 * Delegates to @c dyad_init_env() to initialize the DYAD context from
 * environment variables. This is the top-level initialization entry point
 * in the initialization chain:
 *
 * @code
 * dyad_ctx_init()
 *     -> dyad_init_env()   [reads environment variables]
 *         -> dyad_init()   [allocates and configures the context]
 * @endcode
 *
 * If initialization fails, logs the error to @c stderr and sets
 * @c ctx->initialized and @c ctx->reenter to @c false to leave the
 * context in a safe, inert state. Called by @c dyad_wrapper_init() at
 * library load time.
 *
 * @param[in] dtl_comm_mode  Communication mode for the data transport layer.
 * @param[in] flux_handle    Optional existing Flux handle. If @c NULL, DYAD
 *                           will open its own handle via @c flux_open().
 */
DYAD_DLL_EXPORTED void dyad_ctx_init (dyad_dtl_comm_mode_t dtl_comm_mode, void *flux_handle);

/**
 * @brief Tears down the DYAD context at the wrapper or library level.
 *
 * @details
 * Calls @c dyad_finalize() to release all DYAD resources if the context
 * is non-NULL. If the context is already @c NULL, returns immediately
 * without taking any action.
 *
 * This is the top-level finalization entry point in the teardown chain:
 * @code
 * dyad_ctx_fini()
 *     -> dyad_finalize()     [frees the context struct]
 *         -> dyad_clear()    [frees all context resources]
 *             -> dyad_dtl_finalize() [finalizes the DTL handle]
 * @endcode
 *
 * Called by @c dyad_wrapper_fini() at library unload time.
 *
 * @note The @c DYAD_PROFILER == 3 branch is obsolete and will be removed
 *       in a future pull request.
 */
DYAD_DLL_EXPORTED void dyad_ctx_fini (void);

/**
 * @brief Initializes the DYAD context with explicit parameters.
 *
 * @details
 * Allocates and initializes the thread-local DYAD context with the provided
 * configuration. If the context is already initialized and @p reinit is
 * @c false, returns @c DYAD_RC_OK immediately without reinitializing.
 * If @p reinit is @c true, calls @c dyad_finalize() to tear down the
 * existing context before reinitializing.
 *
 * The initialization sequence is:
 *  1. Allocate the context struct and set it to @c dyad_ctx_default.
 *  2. Open or adopt a Flux handle and retrieve the broker rank.
 *  3. Compute @c node_idx from @c rank / @c service_mux.
 *  4. Copy the KVS namespace string.
 *  5. Initialize the DTL via @c dyad_set_and_init_dtl_mode().
 *  6. Set the producer-managed path via @c dyad_set_prod_path().
 *  7. Set the consumer-managed path via @c dyad_set_cons_path().
 *  8. Redirect log output to per-process log files under @c logs/.
 *
 * On any failure after allocation, @c dyad_clear() is called to release
 * partially initialized resources and the context is reset to
 * @c dyad_ctx_default.
 *
 * If neither @p prod_managed_path nor @p cons_managed_path is provided,
 * DYAD will not perform any synchronization or data transfer. A warning
 * is printed to @c stderr and @c DYAD_RC_OK is returned.
 *
 * @param[in] debug                     Enable debug logging.
 * @param[in] check                     Enable operation checking.
 * @param[in] shared_storage            Enable shared storage mode.
 * @param[in] reinit                    Force re-initialization if already
 *                                      initialized.
 * @param[in] async_publish             Enable asynchronous KVS publishing.
 * @param[in] fsync_write               Enable @c fsync() on write.
 * @param[in] key_depth                 KVS key hierarchy depth.
 * @param[in] key_bins                  KVS key bins per level.
 * @param[in] service_mux               Number of Flux broker ranks per node.
 *                                      Clamped to a minimum of 1.
 * @param[in] kvs_namespace             Flux KVS namespace. Must not be
 *                                      @c NULL.
 * @param[in] prod_managed_path         Producer-managed directory path.
 *                                      May be @c NULL.
 * @param[in] cons_managed_path         Consumer-managed directory path.
 *                                      May be @c NULL.
 * @param[in] relative_to_managed_path  If @c true, file paths are interpreted
 *                                      as relative to the managed directory.
 * @param[in] dtl_mode_str              Name of the DTL mode to use.
 * @param[in] dtl_comm_mode             Communication mode for the DTL.
 * @param[in] flux_handle               Optional existing Flux handle. If
 *                                      @c NULL, a new handle is opened via
 *                                      @c flux_open().
 *
 * @return @c dyad_rc_t return code indicating the outcome:
 * @retval DYAD_RC_OK       Initialization succeeded, or was already
 *                          initialized and @p reinit is @c false.
 * @retval DYAD_RC_NOCTX    Context allocation failed, or
 *                          @c DYAD_PATH_DELIM is invalid or undefined.
 * @retval DYAD_RC_FLUXFAIL The Flux handle could not be opened or the
 *                          broker rank could not be retrieved.
 * @retval DYAD_RC_*        Any error propagated from
 *                          @c dyad_set_and_init_dtl_mode(),
 *                          @c dyad_set_prod_path(), or
 *                          @c dyad_set_cons_path().
 *
 * @note If @c DYAD_PROFILER_DFTRACER is defined, initializes the DFTracer
 *       profiler at the start of this function.
 * @note Log output is redirected to per-process files under @c logs/ unless
 *       @c DYAD_LOGGER_NO_LOG is defined.
 */
DYAD_DLL_EXPORTED dyad_rc_t dyad_init (bool debug,
                                       bool check,
                                       bool shared_storage,
                                       bool reinit,
                                       bool async_publish,
                                       bool fsync_write,
                                       unsigned int key_depth,
                                       unsigned int key_bins,
                                       unsigned int service_mux,
                                       const char *kvs_namespace,
                                       const char *prod_managed_path,
                                       const char *cons_managed_path,
                                       bool relative_to_managed_path,
                                       const char *dtl_mode_str,
                                       const dyad_dtl_comm_mode_t dtl_comm_mode,
                                       void *flux_handle);

/**
 * @brief Initializes DYAD by reading configuration from environment variables.
 *
 * @details
 * Reads the following environment variables to configure DYAD, then delegates
 * to @c dyad_init() to set up the full DYAD context:
 *
 *  | Environment Variable       | Default          | Description                          |
 *  |----------------------------|------------------|--------------------------------------|
 *  | @c DYAD_SYNC_DEBUG         | @c false         | Enable debug logging                 |
 *  | @c DYAD_SYNC_CHECK         | @c false         | Enable operation checking            |
 *  | @c DYAD_SHARED_STORAGE     | @c false         | Enable shared storage mode           |
 *  | @c DYAD_REINIT             | @c false         | Force re-initialization              |
 *  | @c DYAD_ASYNC_PUBLISH      | @c false         | Enable asynchronous KVS publishing   |
 *  | @c DYAD_FSYNC_WRITE        | @c false         | Enable @c fsync() on write           |
 *  | @c DYAD_KEY_DEPTH          | @c 3             | KVS key hierarchy depth              |
 *  | @c DYAD_KEY_BINS           | @c 1024          | KVS key bins per level               |
 *  | @c DYAD_SERVICE_MUX        | @c 1             | Flux broker ranks per node           |
 *  | @c DYAD_KVS_NAMESPACE      | @c NULL          | Flux KVS namespace                   |
 *  | @c DYAD_PATH_CONSUMER      | @c NULL          | Consumer-managed directory path      |
 *  | @c DYAD_PATH_PRODUCER      | @c NULL          | Producer-managed directory path      |
 *  | @c DYAD_PATH_RELATIVE      | @c false         | Paths are relative to managed dirs   |
 *  | @c DYAD_DTL_MODE           | @c DYAD_DTL_DEFAULT | Data transport layer mode         |
 *
 * If @c DYAD_DTL_MODE is not set, defaults to @c DYAD_DTL_DEFAULT and logs
 * a warning to @c stderr.
 *
 * @param[in] dtl_comm_mode  Communication mode for the data transport layer.
 * @param[in] flux_handle    Optional existing Flux handle. If @c NULL, DYAD
 *                           will create its own handle.
 *
 * @return @c dyad_rc_t return code propagated from @c dyad_init().
 */
DYAD_DLL_EXPORTED dyad_rc_t dyad_init_env (const dyad_dtl_comm_mode_t dtl_comm_mode,
                                           void *flux_handle);

/**
 * @brief Switches the DYAD Data Transport Layer (DTL) to a new mode.
 *
 * @details
 * Resolves @p dtl_mode_name to a @c dyad_dtl_mode_t value, finalizes the current
 * DTL handle via @c dyad_dtl_finalize(), and reinitializes it with the new
 * mode via @c dyad_dtl_init(). This allows the DTL to be changed at runtime
 * without reinitializing the full DYAD context.
 *
 * Supported mode names are those in @c dyad_dtl_mode_name[]:
 * @c DYAD_DTL_DEFAULT, @c DYAD_DTL_UCX, @c DYAD_DTL_MARGO, and
 * @c DYAD_DTL_FLUX_RPC.
 *
 * @param[in] dtl_mode_name  Name of the DTL mode to switch to. Must not be
 *                           @c NULL and must match one of the supported mode
 *                           name strings.
 * @param[in] dtl_comm_mode  Communication mode for the new DTL instance.
 *
 * @return @c dyad_rc_t return code indicating the outcome:
 * @retval DYAD_RC_OK          The DTL was successfully switched and initialized.
 * @retval DYAD_RC_BADDTLMODE  @p dtl_mode_name is @c NULL or does not match any
 *                             supported DTL mode name.
 * @retval DYAD_RC_*           Any error code propagated from
 *                             @c dyad_dtl_finalize() or @c dyad_dtl_init().
 *
 * @note If the current DTL handle is @c NULL, @c dyad_dtl_finalize() returns
 *       success without taking any action.
 */
DYAD_DLL_EXPORTED dyad_rc_t dyad_set_and_init_dtl_mode (const char *dtl_mode_name,
                                                        dyad_dtl_comm_mode_t dtl_comm_mode);

/**
 * @brief Sets the producer-managed directory path in the DYAD context.
 *
 * @details
 * Updates @c ctx->prod_managed_path and its associated canonical form,
 * lengths, and hashes. Any previously set producer path is freed before
 * the new one is stored.
 *
 * If @p prod_managed_path is @c NULL, the producer path fields in the
 * context are cleared, disabling producer-side operations.
 *
 * If @p prod_managed_path is non-NULL, the function also resolves its
 * canonical form via @c realpath(). If the canonical form differs from
 * the provided path, it is stored separately in @c ctx->prod_real_path
 * for use in path prefix matching. If @c realpath() fails (e.g. because
 * the directory does not yet exist) or the canonical form is identical
 * to the provided path, @c ctx->prod_real_path is set to @c NULL to
 * avoid redundant matching.
 *
 * The re-entrancy guard (@c ctx->reenter) is disabled for the duration
 * of this call and restored before returning.
 *
 * @param[in] path               Path to the producer-managed directory.
 *                               May be @c NULL to clear the producer path.
 *
 * @return @c dyad_rc_t return code indicating the outcome:
 * @retval DYAD_RC_OK              The producer path was successfully set or
 *                                 cleared.
 * @retval DYAD_RC_NOCTX           The DYAD context is @c NULL.
 * @retval DYAD_RC_BADMANAGEDPATH  @p prod_managed_path is an empty string,
 *                                 or hashing the path returned 0.
 * @retval DYAD_RC_SYSFAIL         Memory allocation or @c memcpy() failed.
 *
 * @note @c realpath() failure is not treated as an error. This can occur
 *       when the producer-managed directory has not yet been created, which
 *       is valid in configurations where both producer and consumer managed
 *       paths are set but only one side creates its directory.
 */
DYAD_DLL_EXPORTED dyad_rc_t dyad_set_prod_path (const char *path);

/**
 * @brief Sets the consumer-managed directory path in the DYAD context.
 *
 * @details
 * Functionally equivalent to @c dyad_set_prod_path() but sets the
 * consumer-managed directory path (@c ctx->cons_managed_path).
 *
 * @param[in] path               Path to the consumer-managed directory.
 *                               May be @c NULL to clear the consumer path.
 *
 * @return @c dyad_rc_t return code indicating the outcome. See
 *         @c dyad_set_prod_path() for the full list of return codes.
 *
 * @see dyad_set_prod_path()
 */
DYAD_DLL_EXPORTED dyad_rc_t dyad_set_cons_path (const char *path);

/**
 * @brief Resets the DYAD context to its default state, releasing all
 *        internal resources without freeing the context struct itself.
 *
 * @details
 * Releases all resources held by the global DYAD context in the following
 * order:
 *  1. Finalizes the DTL handle via @c dyad_dtl_finalize().
 *  2. Closes the Flux handle via @c flux_close().
 *  3. Frees the KVS namespace string.
 *  4. Frees the producer-managed path and its canonical form.
 *  5. Frees the consumer-managed path and its canonical form.
 *
 * Unlike @c dyad_finalize(), this function does not free the context struct
 * itself. The context pointer remains valid after this call, allowing the
 * GOTCHA wrapper layer to continue using it for the lifetime of the process.
 * This is important because the wrapper requires a valid context pointer even
 * during error recovery.
 *
 * If the context is already @c NULL, returns @c DYAD_RC_OK immediately
 * without taking any action.
 *
 * This function is called by @c dyad_finalize() as part of normal teardown,
 * and also directly by @c dyad_init() on initialization failure to release
 * partially initialized resources.
 *
 * @return @c dyad_rc_t return code indicating the outcome:
 * @retval DYAD_RC_OK  All resources were successfully released, or the
 *                     context was already @c NULL.
 *
 * @note If @c DYAD_PROFILER_DFTRACER is defined, finalizes the DFTracer
 *       profiler after the context is cleared.
 */
DYAD_DLL_EXPORTED dyad_rc_t dyad_clear (void);

/**
 * @brief Finalizes and deallocates the DYAD context.
 *
 * @details
 * Calls @c dyad_clear() to release all resources held by the context,
 * then frees the context struct itself and sets the global context pointer
 * to @c NULL. If the context is already @c NULL, returns @c DYAD_RC_OK
 * immediately without taking any action.
 *
 * This is the top-level teardown function in the finalization chain:
 * @code
 * dyad_finalize()
 *     -> dyad_clear()        [frees all context resources]
 *         -> dyad_dtl_finalize() [finalizes the DTL handle]
 * @endcode
 *
 * @return @c dyad_rc_t return code indicating the outcome:
 * @retval DYAD_RC_OK  The context was successfully finalized, or was already
 *                     @c NULL.
 *
 * @note If @c DYAD_PROFILER_DFTRACER is defined, finalizes the DFTracer
 *       profiler after the context is freed.
 */
DYAD_DLL_EXPORTED dyad_rc_t dyad_finalize (void);

#ifdef __cplusplus
}
#endif

#endif /* DYAD_CORE_DYAD_CTX_H */
