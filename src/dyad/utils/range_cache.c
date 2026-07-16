#if defined(DYAD_HAS_CONFIG)
#include <dyad/dyad_config.hpp>
#else
#error "no config"
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif  // _GNU_SOURCE

// clang-format off
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
// clang-format on

#if defined(__cplusplus)
#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#else
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <dyad/utils/range_cache.h>

#include <dyad/common/dyad_logging.h>
#include <dyad/common/dyad_profiler.h>
#include <dyad/utils/utils.h>

#define DYAD_RANGE_CACHE_SUFFIX ".dyad_cached"

static dyad_rc_t range_cache_bitmap_path (const char *local_path, char *out, size_t out_capacity)
{
    if (snprintf (out, out_capacity, "%s%s", local_path, DYAD_RANGE_CACHE_SUFFIX)
        >= (int)out_capacity) {
        return DYAD_RC_BADBUF;
    }
    return DYAD_RC_OK;
}

static inline size_t range_cache_num_blocks (size_t file_size)
{
    return (file_size + DYAD_RANGE_CACHE_BLOCK_SIZE - 1ul) / DYAD_RANGE_CACHE_BLOCK_SIZE;
}

static inline size_t range_cache_bitmap_bytes (size_t num_blocks)
{
    return (num_blocks + 7ul) / 8ul;
}

static bool range_cache_blocks_set (const uint8_t *bitmap, size_t block_start, size_t block_end)
{
    size_t b = 0ul;
    for (b = block_start; b <= block_end; b++) {
        if ((bitmap[b / 8ul] & (uint8_t)(1u << (b % 8ul))) == 0) {
            return false;
        }
    }
    return true;
}

static void range_cache_set_blocks (uint8_t *bitmap, size_t block_start, size_t block_end)
{
    size_t b = 0ul;
    for (b = block_start; b <= block_end; b++) {
        bitmap[b / 8ul] |= (uint8_t)(1u << (b % 8ul));
    }
}

// Reads the block-aligned span [block_start, block_end] from origin_path and
// writes it into local_path at the same offset. Called with no bitmap lock
// held -- see the "no lock held" comment in dyad_range_cache_ensure() for
// why concurrent callers racing on an overlapping span is safe.
static dyad_rc_t range_cache_fetch_span (const dyad_ctx_t *ctx,
                                         const char *local_path,
                                         const char *origin_path,
                                         size_t block_start,
                                         size_t block_end,
                                         size_t origin_size)
{
    dyad_rc_t rc = DYAD_RC_OK;
    int origin_fd = -1;
    int local_fd = -1;
    void *span_buf = NULL;
    size_t span_off = block_start * DYAD_RANGE_CACHE_BLOCK_SIZE;
    size_t span_end = (block_end + 1ul) * DYAD_RANGE_CACHE_BLOCK_SIZE;
    size_t span_len = 0ul;

    if (span_end > origin_size) {
        span_end = origin_size;
    }
    if (span_end <= span_off) {
        return DYAD_RC_BADFIO;
    }
    span_len = span_end - span_off;

    span_buf = malloc (span_len);
    if (span_buf == NULL) {
        return DYAD_RC_SYSFAIL;
    }

    origin_fd = open (origin_path, O_RDONLY);
    if (origin_fd == -1
        || pread (origin_fd, span_buf, span_len, (off_t)span_off) != (ssize_t)span_len) {
        DYAD_LOG_ERROR (ctx,
                        "DYAD RANGE_CACHE: cannot read span [%zu, %zu) from origin '%s'",
                        span_off,
                        span_end,
                        origin_path);
        rc = DYAD_RC_BADFIO;
        goto fetch_span_done;
    }

    local_fd = open (local_path, O_WRONLY);
    if (local_fd == -1
        || pwrite (local_fd, span_buf, span_len, (off_t)span_off) != (ssize_t)span_len) {
        DYAD_LOG_ERROR (ctx,
                        "DYAD RANGE_CACHE: cannot write span [%zu, %zu) into local '%s'",
                        span_off,
                        span_end,
                        local_path);
        rc = DYAD_RC_BADFIO;
        goto fetch_span_done;
    }
    rc = DYAD_RC_OK;

fetch_span_done:;
    if (origin_fd != -1) {
        close (origin_fd);
    }
    if (local_fd != -1) {
        close (local_fd);
    }
    free (span_buf);
    return rc;
}

dyad_rc_t dyad_range_cache_ensure (const dyad_ctx_t *ctx,
                                   const char *local_path,
                                   const char *origin_path,
                                   size_t offset,
                                   size_t length)
{
    dyad_rc_t rc = DYAD_RC_OK;
    char bitmap_path[PATH_MAX + 1] = {'\0'};
    int bitmap_fd = -1;
    int size_fd = -1;
    struct flock lock;
    struct stat origin_st;
    uint8_t *bitmap = NULL;
    size_t bitmap_bytes = 0ul;
    size_t num_blocks = 0ul;
    size_t block_start = 0ul;
    size_t block_end = 0ul;
    size_t origin_size = 0ul;
    bool already_cached = false;

    if (origin_path == NULL || length == 0ul) {
        return DYAD_RC_OK;
    }

    rc = range_cache_bitmap_path (local_path, bitmap_path, sizeof (bitmap_path));
    if (DYAD_IS_ERROR (rc)) {
        DYAD_LOG_ERROR (ctx, "DYAD RANGE_CACHE: local path '%s' is too long", local_path);
        return rc;
    }

    block_start = offset / DYAD_RANGE_CACHE_BLOCK_SIZE;
    block_end = (offset + length - 1ul) / DYAD_RANGE_CACHE_BLOCK_SIZE;

    bitmap_fd = open (bitmap_path, O_RDWR | O_CREAT, 0644);
    if (bitmap_fd == -1) {
        DYAD_LOG_ERROR (ctx, "DYAD RANGE_CACHE: cannot open bitmap file '%s'", bitmap_path);
        return DYAD_RC_BADFIO;
    }

    // Fast path: under a shared lock, check whether the requested span is
    // already fully cached.
    rc = dyad_shared_flock (ctx, bitmap_fd, &lock);
    if (DYAD_IS_ERROR (rc)) {
        close (bitmap_fd);
        return rc;
    }
    bitmap_bytes = (size_t)get_file_size (bitmap_fd);
    if (bitmap_bytes > block_end / 8ul) {
        bitmap = (uint8_t *)malloc (bitmap_bytes);
        if (bitmap != NULL && pread (bitmap_fd, bitmap, bitmap_bytes, 0) == (ssize_t)bitmap_bytes
            && range_cache_blocks_set (bitmap, block_start, block_end)) {
            free (bitmap);
            dyad_release_flock (ctx, bitmap_fd, &lock);
            close (bitmap_fd);
            return DYAD_RC_OK;
        }
        free (bitmap);
        bitmap = NULL;
    }
    dyad_release_flock (ctx, bitmap_fd, &lock);

    // Miss (or bitmap not yet sized): escalate to an exclusive lock just
    // long enough to do the cheap metadata work (stat/ftruncate/re-check),
    // then release it *before* doing the slow origin fetch + local write --
    // holding it across that would serialize every concurrent miss on this
    // shard (even for disjoint spans) behind whatever process got there
    // first, which is exactly what turned occasional slow PFS reads into
    // multi-second-to-tens-of-seconds stalls for every other consumer of
    // the same shard queued behind the lock.
    rc = dyad_excl_flock (ctx, bitmap_fd, &lock);
    if (DYAD_IS_ERROR (rc)) {
        close (bitmap_fd);
        return rc;
    }

    if (stat (origin_path, &origin_st) != 0 || origin_st.st_size <= 0) {
        DYAD_LOG_ERROR (ctx, "DYAD RANGE_CACHE: cannot stat origin file '%s'", origin_path);
        dyad_release_flock (ctx, bitmap_fd, &lock);
        close (bitmap_fd);
        return DYAD_RC_BADFIO;
    }
    origin_size = (size_t)origin_st.st_size;
    num_blocks = range_cache_num_blocks (origin_size);
    bitmap_bytes = range_cache_bitmap_bytes (num_blocks);

    if ((size_t)get_file_size (bitmap_fd) < bitmap_bytes) {
        // First touch for this local_path: size both the bitmap and the
        // (possibly not-yet-existing) local data file once, up front.
        size_fd = open (local_path, O_WRONLY | O_CREAT, 0644);
        if (size_fd == -1 || ftruncate (size_fd, (off_t)origin_size) != 0
            || ftruncate (bitmap_fd, (off_t)bitmap_bytes) != 0) {
            DYAD_LOG_ERROR (ctx,
                            "DYAD RANGE_CACHE: cannot size local file/bitmap for '%s'",
                            local_path);
            if (size_fd != -1) {
                close (size_fd);
            }
            dyad_release_flock (ctx, bitmap_fd, &lock);
            close (bitmap_fd);
            return DYAD_RC_BADFIO;
        }
        close (size_fd);
    }

    bitmap = (uint8_t *)calloc (1ul, bitmap_bytes);
    if (bitmap == NULL) {
        dyad_release_flock (ctx, bitmap_fd, &lock);
        close (bitmap_fd);
        return DYAD_RC_SYSFAIL;
    }
    // A short/zero read here just means no blocks are set yet (e.g. right
    // after the ftruncate() above), which range_cache_blocks_set() below
    // will correctly treat as a miss since bitmap was calloc'd to all-zero.
    pread (bitmap_fd, bitmap, bitmap_bytes, 0);
    already_cached = range_cache_blocks_set (bitmap, block_start, block_end);
    free (bitmap);
    bitmap = NULL;
    dyad_release_flock (ctx, bitmap_fd, &lock);

    if (already_cached) {
        // Another process filled this span while we waited.
        close (bitmap_fd);
        return DYAD_RC_OK;
    }

    // Fetch from origin_path into local_path with *no* lock held. If
    // another process is concurrently missing on an overlapping span (same
    // shard), both will redundantly pread the same origin bytes and pwrite
    // them to the same local_path offset -- since both write identical data
    // to the same range, this races safely (worst case is duplicated I/O,
    // never corruption), and is a better trade than serializing unrelated
    // misses behind a single slow PFS read.
    rc = range_cache_fetch_span (ctx, local_path, origin_path, block_start, block_end, origin_size);
    if (DYAD_IS_ERROR (rc)) {
        close (bitmap_fd);
        return rc;
    }

    // Re-acquire the lock just long enough to record that this span is now
    // cached, merging with whatever bits any concurrent racers have set.
    rc = dyad_excl_flock (ctx, bitmap_fd, &lock);
    if (DYAD_IS_ERROR (rc)) {
        close (bitmap_fd);
        return rc;
    }
    bitmap = (uint8_t *)calloc (1ul, bitmap_bytes);
    if (bitmap == NULL) {
        dyad_release_flock (ctx, bitmap_fd, &lock);
        close (bitmap_fd);
        return DYAD_RC_SYSFAIL;
    }
    pread (bitmap_fd, bitmap, bitmap_bytes, 0);
    range_cache_set_blocks (bitmap, block_start, block_end);
    if (pwrite (bitmap_fd, bitmap, bitmap_bytes, 0) != (ssize_t)bitmap_bytes) {
        DYAD_LOG_ERROR (ctx, "DYAD RANGE_CACHE: cannot update bitmap file '%s'", bitmap_path);
        rc = DYAD_RC_BADFIO;
    }
    free (bitmap);
    dyad_release_flock (ctx, bitmap_fd, &lock);
    close (bitmap_fd);
    return rc;
}
