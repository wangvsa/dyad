.. _dyad_cache_eviction:

Node-Local Cache Eviction
#########################

By default, DYAD's producer- and consumer-managed directories (DMDs) grow
without bound: every file produced or consumed accumulates on node-local
storage forever. For workloads that repeatedly touch a shifting working set
larger than any single node's capacity — for example the shuffled-sample-file
training pattern described above, where a `DistributedSampler` shuffles
across all shard files every epoch — this means a node's DMD eventually
approaches the size of the entire dataset rather than just the fraction it
actually needs at any one time.

DYAD addresses this with an optional, pluggable cache-eviction subsystem,
disabled by default (``DYAD_CACHE_CAPACITY`` unset or ``0``), so existing
deployments see no change in behavior unless they opt in.

Pluggable policy architecture
==============================

Cache eviction follows the same struct-of-function-pointers pattern as the
:ref:`Data Transport Layer <dyad_dtl>`: a policy handle selected by a mode
enum (:code:`DYAD_CACHE_NONE`, :code:`DYAD_CACHE_LRU`, :code:`DYAD_CACHE_FIFO`),
with the active policy's implementation reached through a single function
pointer, ``get_recency_key()``, rather than DTL's larger transport-specific
interface. Unlike the UCX/Margo DTL backends, LRU and FIFO have no external
dependencies, so both are always compiled in — no CMake feature flag is
needed to enable them.

Recency tracking without a persisted index
=============================================

Rather than maintaining a separate database of access times (which would
itself need cross-process synchronization to stay consistent), each policy
derives its ranking directly from filesystem metadata already maintained by
the kernel:

- **LRU** ranks by ``st_atime`` — updated on every read, with no DYAD code
  changes needed on the hot read path.
- **FIFO** ranks by ``st_mtime`` — files under a DYAD-managed directory are
  effectively write-once, so modification time approximates insertion order.

This works correctly across independent processes sharing one managed
directory (for example, PyTorch ``DataLoader`` workers with ``num_workers >
0``, which run as separate OS processes, each with its own DYAD context) with
no shared memory or IPC: ``stat()`` is a kernel-level, cross-process-consistent
syscall, so every process observes the same recency signal for a given file.

Locking and eviction procedure
=================================

The only cross-process hazard is the *scan-and-evict decision* itself — two
processes could otherwise race to decide a file needs evicting and both act
on it. This is serialized with a dedicated per-directory advisory lock file
(``.dyad_cache.lock``), reusing the same ``dyad_excl_flock()``/
``dyad_release_flock()`` primitives already described in
:ref:`Local file access coordination via locking <dyad_local-file-access>` for
per-file coordination — just applied to a directory-level sentinel instead of
a data file.

``dyad_produce()`` and ``dyad_consume()``/``dyad_consume_w_metadata()`` each
call ``dyad_cache_maybe_evict()`` once they otherwise complete successfully.
That function:

1. Returns immediately if eviction is disabled (``DYAD_CACHE_CAPACITY`` is
   ``0``, the default) — a single branch, no filesystem access at all.
2. Scans the managed directory's current on-disk usage. If already at or
   under capacity, returns without taking any lock.
3. Otherwise acquires the directory lock, re-checks usage (another process
   may have already evicted enough while this one waited), and if still over
   capacity, ranks every candidate file by the active policy's recency key.
4. Walks candidates oldest-first, skipping any file accessed within
   ``DYAD_CACHE_GRACE_PERIOD`` seconds or currently locked by another
   in-flight ``dyad_produce()``/``dyad_consume()`` call (checked via a
   non-blocking ``dyad_try_excl_flock()``, so the evictor never blocks
   waiting on a file that's mid-transfer), removing files until usage drops
   to ``DYAD_CACHE_LOW_WATERMARK`` (a fraction of capacity, default 0.8, so a
   single eviction pass doesn't need to run again on the very next call).
5. Releases the directory lock.

Eviction failures on individual files are logged and skipped; this mechanism
never turns an otherwise-successful ``dyad_produce()``/``dyad_consume()``
call into a reported error.

Known limitation: stale metadata on producer-side eviction
===============================================================

Evicting a file from the local DMD does not retract its published Flux KVS
metadata. :ref:`Case 2 <dyad_metadata_lookup>` of the Hierarchical File
Locator already tolerates a *consumer-side* cache miss by falling back to the
global HFL hierarchy — but if the *producer's* origin copy is evicted, a
later consumer resolving that file's metadata could still be pointed at a
producer that no longer has it locally, causing the subsequent fetch to fail.
Addressing this — for example with a metadata retraction/unpublish primitive,
or a staleness check at fetch time — is left as future work.
