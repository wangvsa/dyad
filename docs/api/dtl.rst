.. _dyad_dev_api_dtl:

DYAD Data Transport API
#######################

Once consumer identifies the owner of the file it tries to access via metadata,
and it finds out the owner is at a remote location, i.e., the file is not
locally visible, it request the owner to transfer the file.
DYAD offers multiple backend implementations to transfer data from producer to
consumer. The streaming RPC that is native mechanism of Flux, UCX with RDMA,
and the Margo+Mercury framework.

.. doxygenfile:: dyad_dtl_api.c
   :project: dyad


.. _dyad_dev_api_dtl_flux_rpc:

Flux Streaming RPC
==================

The Flux RPC DTL backend uses the built-in RPC mechanism provided by
the `Flux framework <https://flux-framework.readthedocs.io>`_, requiring
no additional networking dependencies beyond Flux itself.

**Flux** is a next-generation resource manager and job scheduler for
HPC systems. Beyond job and resource management, Flux provides a
distributed messaging infrastructure based on ZeroMQ that allows
processes running under the same Flux instance to communicate via
lightweight RPCs. Each Flux broker runs on a node and handles message
routing between processes across the job allocation.

**Flux streaming RPC** extends the basic request-response RPC model to
allow the service to send multiple responses to a single request, with
``ENODATA`` signalling end-of-stream. DYAD uses this mechanism on the
producer side — the DYAD service responds to a consumer fetch request
by sending the file data as one or more raw payloads via
``flux_respond_raw()``, and signals completion by responding with
``ENODATA`` via ``flux_respond_error()``. The consumer reads successive
responses from the same ``flux_future_t`` via ``flux_rpc_get_raw()``
until it receives ``ENODATA``.

.. doxygenfile:: flux_dtl.c
   :project: dyad


.. _dyad_dev_api_dtl_margo:

Margo, Mercury and Mochi
========================

Margo, Mercury, and libfabric form a layered communication stack
used by DYAD's Margo DTL backend.

**Margo** is a higher-level RPC library built on top of Mercury and
Argobots. It integrates Mercury's progress loop with Argobots user-level
threads (ULTs) and execution streams (ESs), allowing RPC handlers to be
written as straightforward blocking functions rather than callback chains.

**Mercury** is an RPC framework built on top of a pluggable Network
Abstraction (NA) layer. It provides serialization of RPC arguments and
RDMA bulk transfers via ``hg_bulk_t`` handles, supporting both push
(``HG_BULK_PUSH``) and pull (``HG_BULK_PULL``) transfer modes. Mercury
supports multiple NA plugins including libfabric and UCX, allowing it to
target a wide range of high-performance network fabrics. In DYAD's Margo
DTL backend, the pull model is adopted — the producer registers its buffer
and notifies the consumer, which pulls the data directly from the producer's
memory via ``HG_BULK_PULL``.

Together these libraries are part of the `Mochi project
<https://www.mcs.anl.gov/research/projects/mochi/>`_, an HPC ecosystem
of composable microservices for data management and storage, developed
under the DOE Exascale Computing Project.

**libfabric (OFI)** is a low-level network abstraction library that
provides portable access to high-performance network fabrics such as
InfiniBand (via ``ofi+verbs``) and TCP (via ``ofi+tcp``).
It is maintained by the `OpenFabrics Alliance
<https://ofiwg.github.io/libfabric/>`_ and used by Mercury as its
high-performance network transport layer.

.. doxygenfile:: margo_dtl.c
   :project: dyad


.. _dyad_dev_api_ucx:

UCX
===

The UCX (Unified Communication X) DTL backend provides high-performance
inter-node data movement using one-sided RDMA. UCX is an open-source
communication library that abstracts over multiple high-speed network
fabrics including InfiniBand, RoCE, and shared memory, exposing a
unified API for remote memory access operations.

In DYAD's UCX backend, the producer pushes file data directly into the
consumer's pre-registered memory buffer using ``ucp_put_nbx()``, a
non-blocking one-sided RDMA put operation. Unlike the Margo backend
which uses a pull model (``HG_BULK_PULL``), the UCX backend uses a push
model — the producer writes directly to the consumer's memory without
the consumer needing to initiate the transfer.

Before any data transfer can occur, the consumer pre-allocates and
registers an RDMA buffer with UCX via ``ucp_mem_map()`` during
initialization. The consumer's buffer address and the associated remote
key (``ucp_rkey_t``) are packed into the Flux RPC request payload
(base64-encoded) and sent to the producer. The producer decodes these
fields, unpacks the remote key via ``ucp_ep_rkey_unpack()``, and uses
them to perform the RDMA put directly into the consumer's buffer without
any intermediate copy.

To avoid the cost of repeated endpoint creation, the UCX backend
maintains a per-producer endpoint cache (``ucx_ep_cache_h``) that maps
consumer connection keys to ``ucp_ep_h`` endpoints. An endpoint is
created on the first transfer to a given consumer and reused for all
subsequent transfers to the same consumer within the same job, amortizing
the connection establishment overhead across multiple file fetches.

The consumer detects the arrival of data by polling the first
``sizeof(ssize_t)`` bytes of its RDMA buffer, which the producer
prepends with the file size before initiating the put. This sentinel
value is initialized to zero at the start of each transfer and becomes
non-zero once the producer begins writing, allowing the consumer to busy-
wait without a UCX request handle since one-sided RDMA puts do not
notify the target.

.. doxygenfile:: ucx_dtl.c
   :project: dyad

.. doxygenfile:: ucx_ep_cache.cpp
   :project: dyad

