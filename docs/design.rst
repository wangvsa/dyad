******************************
DYAD System Design
******************************

High-level Overview
###################

Comparison of a producer and consumer pair sharing a file with and without DYAD
-------------------------------------------------------------------------------

.. figure:: _static/figs/dyad_without_vs_with.svg
   :alt: Without DYAD vs With DYAD

   Without DYAD, producer and consumer share data via a shared file system
   (e.g., Lustre) with explicit synchronization through dependent jobs, APIs,
   or polling. With DYAD, they use node-local storage with the abstraction of
   shared visibility. DYAD transparently intercepts I/O and transfers data
   across local storages while coordinating accesses through KVS and
   RDMA-based data movement, eliminating both the shared file system
   bottleneck and the need for explicit synchronization.


Local file access coordination via locking
##########################################

To allow seamless coordination between producers and consumers of files,
DYAD supports both intercepting POSIX I/O using GOTCHA and wrapper classes
for C++ streams. The two methods currently operate in a mutually exclusive
fashion. To assure the soundness of access on files that are visible to
multiple workers including producers and consumers, we rely on file locking
mechanisms.

.. figure:: _static/figs/dyad_locking_summary.svg
   :alt: DYAD locking protocol summary

   This diagram shows the flow of each of the two independent methods.
   The C GOTCHA wrappers intercept POSIX-level calls while the C++ stream
   wrappers operate independently. In both paths the producer holds an
   exclusive lock for the duration of the write to protect consumers with
   direct file visibility from reading premature files.
   The consumer acquires an exclusive lock inside ``dyad_consume()`` to
   coordinate with local producers as well as to serialize the fetch
   among concurrent consumers.

.. figure:: _static/figs/dyad_consumer_race_fetch.svg
   :alt: Consumer race management — file fetched from remote producer

   The exclusive lock serializes consumers so that only one performs the
   expensive KVS wait and data fetch while avoiding to overwrite the file.
   When the second consumer eventually acquires the lock, it finds the file
   already present (size > 0) and skips the fetch entirely.
   The full sequence for consumer A is: fetch metadata ->
   data transfer (``dyad_get_data()``) -> write to disk (``dyad_cons_store()``)
   -> release lock, with consumer B blocked throughout all three steps.

.. figure:: _static/figs/dyad_consumer_local_file.svg
   :alt: Consumer race management — file already local

   If the file is already present locally, there is no need for fetching
   metadata or the file itself. In that case, the consumer releases the lock
   immediately and continues to perform read-only access on the file.


Fallback behavior when filesystem locking is unavailable
--------------------------------------------------------

Filesystem locking in the C++ stream wrapper path relies on the ability to
extract the underlying file descriptor from a ``std::basic_filebuf`` object
via the GCC/libstdc++ internal ``_M_file.fd()`` member. This capability is
detected automatically at configure time by CMake and exposed as the
``DYAD_HAS_STD_FSTREAM_FD`` compile-time flag.

When ``DYAD_HAS_STD_FSTREAM_FD`` is not defined, filesystem locking is
unavailable in the C++ stream path and ``ctx->use_fs_locks`` is set to
``false``. In this case, ``dyad_consume()`` cannot rely on file size alone
to determine whether a co-located producer has finished writing — since the
producer cannot lock the file it is still writing, a non-zero file size does
not guarantee the file is complete. DYAD therefore falls back to always
consulting the Flux KVS for synchronization regardless of file size, at the
cost of additional network overhead compared to the local locking path.

The C GOTCHA wrapper path is unaffected by this fallback. Since it always
has direct access to file descriptors, ``use_fs_locks`` is explicitly set to
``true`` in ``dyad_wrapper_init()`` during library initialization.

``dyad_consume()`` is invoked on the consumer side in both interception
paths: in the C GOTCHA wrapper it is called inside the ``open()`` wrapper,
and in the C++ stream path it is called via ``open_sync()`` from
``basic_ifstream_dyad`` and ``basic_fstream_dyad`` constructors and their
``open()`` methods.

Similarly, ``dyad_produce()`` is invoked on the producer side in both paths:
in the C GOTCHA wrapper it is called inside the ``close()`` wrapper, and in
the C++ stream path it is called via ``close_sync()`` from
``basic_ofstream_dyad`` and ``basic_fstream_dyad`` destructors and their
``close()`` methods.

Data durability in the C++ stream path
--------------------------------------

Unlike the C GOTCHA wrapper, the C++ stream destructor and ``close()``
perform an additional step of flushing kernel-buffered stream data to stable
storage by calling ``fsync()`` on the file descriptor extracted from the
stream. This ensures all written data is durable before ``dyad_produce()``
notifies consumers, providing a stronger durability guarantee than the C
GOTCHA wrapper path which does not perform an explicit ``fsync()``.


DYAD entry points and initalization
###################################


.. figure:: _static/figs/dyad_initialization_points.svg
   :alt: DYAD initialization points

   DYAD has multiple initialization entry points depending on the interception
   method in use. The C GOTCHA wrapper path initializes via
   ``dyad_wrapper_init()``, which calls ``dyad_ctx_init()``, which in turn
   calls ``dyad_init_env()`` to read configuration from environment variables
   before delegating to ``dyad_init()``. The C++ stream wrapper path
   initializes via ``dyad_stream_core::init()``, which either calls
   ``dyad_init()`` directly when given explicit parameters via
   ``dyad_params``, or follows the same ``dyad_ctx_init()`` →
   ``dyad_init_env()`` chain when reading from environment variables.
   The Python bindings (pydyad) may call either ``dyad_init()`` directly
   or ``dyad_init_env()``, depending on whether configuration is provided
   programmatically or via the environment. All paths ultimately converge
   on ``dyad_init()``, which allocates and configures the DYAD context.
   The Flux module initialization path is shown separately in the diagram
   below.


.. figure:: _static/figs/dyad_flux_module_init.svg
   :alt: DYAD Flux module initialization

   The DYAD Flux module follows the same ``dyad_ctx_init()`` →
   ``dyad_init_env()`` → ``dyad_init()`` chain as the C GOTCHA wrapper
   (shown above), but with an additional step — ``mod_main()`` first calls
   ``dyad_module_ctx_init()``, which applies any command-line argument
   overrides to environment variables before delegating to
   ``dyad_ctx_init()``. The module also differs in two other key ways: it
   initializes with ``DYAD_COMM_SEND`` (producer mode) rather than
   ``DYAD_COMM_RECV`` (consumer mode), and it adopts the Flux handle
   provided by the broker rather than opening its own.
