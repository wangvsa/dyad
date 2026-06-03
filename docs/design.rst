******************************
DYAD System Design
******************************

High-level Overview
###################

Comparison of a producer and consumer pair sharing a file with and without DYAD
===============================================================================

.. figure:: _static/figs/dyad_without_vs_with.svg
   :alt: Without DYAD vs With DYAD

   Without DYAD, producer and consumer share data via a shared file system
   (e.g., Lustre) with explicit synchronization through dependent jobs, APIs,
   or polling. With DYAD, they use node-local storage with the abstraction of
   shared visibility. DYAD transparently intercepts I/O and transfers data
   across local storages while coordinating accesses through KVS and
   RDMA-based data movement, eliminating both the shared file system
   bottleneck and the need for explicit synchronization.



.. include:: _fragments/design_dtl.rst

.. include:: _fragments/design_lookup.rst

.. include:: _fragments/design_local_access.rst

.. include:: _fragments/design_init.rst

