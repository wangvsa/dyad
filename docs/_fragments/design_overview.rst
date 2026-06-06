Overview of Synchronization and File Transfer
################################################

.. figure:: _static/figs/dyad_operation_overview.svg
   :alt: DYAD high-level operation

   DYAD data flow between producer and consumer nodes. Numbers indicate
   the order of steps and correspond to the labeled arrows in the diagram.

DYAD consists of two components: a :ref:`**wrapper**<dyad_user_api>` library that is injected
into the application process via ``LD_PRELOAD`` or C++ stream wrapper
classes to transparently intercept I/O calls, and a :ref:`**service**<dyad_user_service>` that
runs as a plugin module to the Flux broker on each node. Together they
coordinate file access and transfer between nodes.

The wrapper intercepts I/O only for files in the directory it manages,
or :ref:`DYAD managed directory (DMD)<dyad_managed_dir>`. If the file shared between producer and
consumer under DMD resides on the producer's local storage that is not
visible to the consumer, DYAD both synchronizes accesses and transfers
files to the DMD on the consumer's local storage. If it is visible to
both parties (e.g., residing on a shared storage), DYAD only synchronizes
accesses.

Producer path
=============

When a file is written into the DMD or any of its subdirectories, the
following steps occur in order (see **p.1–p.2** in the diagram):

1. **p.1** ``write(managed_dir/filename)`` — the application writes the
   file to local storage. The wrapper intercepts this call on file close
   to perform necessary steps.
2. **p.2** ``publish(<filename, producer_rank>)`` — the wrapper registers
   the metadata in the DYAD :ref:`hierarchical file locator (HFL)<dyad_metadata_lookup>` that relies on the
   global key-value store (KVS) managed by Flux.
   The metadata is a ``(filename, rank)`` pair, where ``rank`` is the
   rank of the Flux broker on the node where the file owner has local
   visibility to the file. In rare cases, there can be multiple brokers
   running on each node and :ref:`local coordination <dyad_local-file-access>`
   protects accesses.

Consumer path
=============

When a consumer application opens a file in the DMD, the following steps
occur in order (see **c.1–c.3** in the diagram):

1. **c.1** ``query(filename) → producer_rank`` — the wrapper queries
   the :ref:`HFL<dyad_metadata_lookup>` to obtain the rank of the file owner (producer). If the entry
   is not yet present, the consumer blocks until it receives a metadata that
   matches the key it queried. The HFL service responds to the posted
   query when it sees the matching entry, which propagates from the
   producer's local HFL through the hierarchy.
2. **c.2** ``rpc_get(producer_rank, filename)`` — the wrapper sends an
   RPC to the producer's :ref:`DYAD service<dyad_user_service>` requesting the file. The service
   transfers the file over the selected :ref:`DTL backend<dyad_dtl>` (Flux RPC, Margo, or
   UCX) and stores it on the consumer's local storage.
3. **c.3** ``read(managed_dir/filename)`` — the application reads the
   file from local storage as if it had always been there, with no
   knowledge of the transfer or synchronization that took place.
