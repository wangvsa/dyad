/************************************************************\
 * Copyright 2014 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#ifndef DYAD_UTILS_READ_ALL_H
#define DYAD_UTILS_READ_ALL_H

#if defined(DYAD_HAS_CONFIG)
#include <dyad/dyad_config.hpp>
#else
#error "no config"
#endif

#include <sys/types.h>

#if defined(__cplusplus)
extern "C" {
#endif  // defined(__cplusplus)

/**
 * @brief Writes all bytes from a buffer to an open file descriptor.
 *
 * @details
 * Writes exactly @p len bytes from @p buf to @p fd, retrying with the
 * remaining data if @c write() returns fewer bytes than requested. This
 * handles the case where a single @c write() call does not transfer all
 * data, which can occur for large writes or certain file descriptor types
 * such as sockets or pipes.
 *
 * @param[in] fd   Open file descriptor to write to. Must not be negative.
 * @param[in] buf  Buffer containing the data to write. May be @c NULL only
 *                 if @p len is 0.
 * @param[in] len  Number of bytes to write from @p buf.
 *
 * @return ssize_t
 * @retval >0  The total number of bytes written, equal to @p len.
 * @retval  0  @p len is 0; nothing was written.
 * @retval -1  @p fd is negative, @p buf is @c NULL with non-zero @p len
 *             (@c errno set to @c EINVAL), or @c write() failed (@c errno
 *             set by @c write()).
 */
ssize_t write_all (int fd, const void *buf, size_t len);

/**
 * @brief Reads the entire contents of an open file into a newly allocated buffer.
 *
 * @details
 * Determines the file size via @c get_file_size(), allocates a buffer of that
 * size, and reads the entire file contents into it in a single @c read() call.
 * The allocated buffer is returned via @p bufp and the caller is responsible
 * for freeing it.
 *
 * @param[in]  fd    Open file descriptor to read from. The file position must
 *                   be at the beginning; the caller is responsible for seeking
 *                   to the start if needed.
 * @param[out] bufp  Address of a pointer to receive the allocated buffer
 *                   containing the file contents. Must not be @c NULL. On any
 *                   error return, @p *bufp may be @c NULL or point to a
 *                   partially filled buffer; the caller should free it if
 *                   non-@c NULL.
 *
 * @return ssize_t
 * @retval >0  The number of bytes read, equal to the file size. @p *bufp
 *             points to the complete file contents.
 * @retval  0  The file is empty or @c get_file_size() failed (@c errno set
 *             to @c EINVAL). @p *bufp is not modified.
 * @retval -1  Memory allocation failed (@c errno set to @c EINVAL).
 * @retval <file_size  @c read() returned fewer bytes than expected (@c errno
 *                     set to @c EINVAL). @p *bufp points to the partially
 *                     read buffer.
 *
 * @note A single @c read() call is used to read the entire file. On some
 *       systems or for large files, @c read() may return fewer bytes than
 *       requested even without an error. Callers that require guaranteed
 *       complete reads should check the return value against the expected
 *       file size.
 */
#if DYAD_PERFFLOW
__attribute__ ((annotate ("@critical_path()")))
#endif
ssize_t read_all (int fd, void **bufp);

#if defined(__cplusplus)
}
#endif  // defined(__cplusplus)

#endif /* DYAD_UTILS_READ_ALL_H */
