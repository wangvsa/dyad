******************************
Runtime Configuration
******************************

At launch time, DYAD allows users to customize its behavior and make better use
of the environment through various environment variables. The list of these 
variables is shown below.

+--------------------------------+-----------------+--------------+----------+-----------------------------------------------------------------+
| Name                           | Type            | Required?    | Default  | Description                                                     |
+================================+=================+==============+==========+=================================================================+
| :code:`DYAD_KVS_NAMESPACE`     | String          | Yes          | N/A      | The Flux KVS namespace that DYAD will use to record or look     |
|                                |                 |              |          |                                                                 |
|                                |                 |              |          | for file information                                            |
+--------------------------------+-----------------+--------------+----------+-----------------------------------------------------------------+
| :code:`DYAD_PATH_PRODUCER`     | Directory Path  | Yes [#two]_  | N/A      | The producer-managed path of the application                    |
+--------------------------------+                 +              +          +-----------------------------------------------------------------+
| :code:`DYAD_PATH_CONSUMER`     |                 |              |          | The consumer-managed path of the application                    |
+--------------------------------+-----------------+--------------+----------+-----------------------------------------------------------------+
| :code:`DYAD_DTL_MODE`          | String          | No           | FLUX_RPC | Choose data transfer method among MARGO, UCX, FLUX_RPC          |
+--------------------------------+-----------------+--------------+----------+-----------------------------------------------------------------+
| :code:`DYAD_MARGO_PROTO`       | String [#thr]_  | No           | ofi\+tcp | Specify network protocol when :code:`DYAD_DTL_MODE=MARGO`       |
+--------------------------------+-----------------+--------------+----------+-----------------------------------------------------------------+
| :code:`DYAD_PATH_RELATIVE`     | 0 or 1          | No           | 0        | The presence of this variable in the environment indicates that |
|                                |                 |              |          |                                                                 |
|                                |                 |              |          | DYAD treats relative paths as relative to the managed directory |
+--------------------------------+-----------------+--------------+----------+-----------------------------------------------------------------+
| :code:`DYAD_SHARED_STORAGE`    | 0 or 1          | No           | 0        | 1: only per-file access synchronization for consumer            |
|                                |                 |              |          |                                                                 |
|                                |                 |              |          | but no transfer or the overhead associated with it              |
+--------------------------------+-----------------+--------------+----------+-----------------------------------------------------------------+
| :code:`DYAD_ASYNC_PUBLISH`     | 0 or 1          | No           | 0        | Enable asynchronous metadata publishing by producers.           |
+--------------------------------+-----------------+--------------+----------+-----------------------------------------------------------------+
| :code:`DYAD_SERVICE_MUX`       | integer >= 1    | No           | 1        | Number of Flux brokers sharing node-local storage.              |
+--------------------------------+-----------------+--------------+----------+-----------------------------------------------------------------+
| :code:`DYAD_KEY_DEPTH` [#for]_ | Integer         | No           | 3        | The number of levels in Flux's hierarchical KVS to use          |
|                                |                 |              |          |                                                                 |
|                                |                 |              |          | within DYAD's namespace                                         |
+--------------------------------+-----------------+--------------+----------+-----------------------------------------------------------------+
| :code:`DYAD_KEY_BINS` [#for]_  | Integer         | No           | 1024     | The maximum number of unique hash values per level in Flux’s    |
|                                |                 |              |          |                                                                 |
|                                |                 |              |          | hierarchical KVS within DYAD’s namespace.                       |
+--------------------------------+-----------------+--------------+----------+-----------------------------------------------------------------+

.. [#two] For DYAD to do anything, at least one of :code:`DYAD_PATH_PRODUCER` or :code:`DYAD_PATH_CONSUMER` must be provided.
   Applications will still work if neither are provided, but DYAD will not do anything.

.. [#thr] When built with the cmake option :code:`DYAD_ENABLE_MARGO_DATA=ON` and run with the env variable :code:`DYAD_DTL_MODE=MARGO`, :code:`DYAD_MARGO_PROTO` specifies the network protocol.
   Example values include:

   .. code-block:: none
      ofi+tcp     – portable TCP/IP via libfabric (default)
      ofi+verbs   – InfiniBand via libfabric
      ofi+cxi     – HPE Slingshot (Cray EX) via libfabric
      sm          – shared memory (single-node testing)
      na+sm       – shared memory (newer Margo)
      ucx         - auto (all);              let UCX pick, safe default for ucx
      ucx+tcp     - TCP/IP;                  portable, works everywhere
      ucx+rc_v    - InfiniBand RC (verbs);   low-latency IB
      ucx+rc_mlx5 - InfiniBand RC (mlx5 optimized); Mellanox HCAs
      ucx+ud_v    - InfiniBand UD (verbs);   scalable IB, less reliability overhead
      ucx+dc_mlx5 - InfiniBand DC (mlx5);    large-scale IB (Frontier, Sierra)
      ucx+cma     - Cross-Memory Attach;     intra-node shared memory (Linux)
      ucx+sysv    - SysV shared memory;      intra-node only

.. [#for] The Flux KVS supports organized categorization of data through hierarchical `key structuring <https://flux-framework.readthedocs.io/projects/flux-core/en/latest/man1/flux-kvs.html>`_. DYAD leverages this capability to optimize hash lookup performance by increasing the likelihood of early search termination when no matching entry exists in the store. The search key is hashed across multiple levels using different seeds. A match is confirmed only if corresponding entries are found at all levels, with an exact key-string match at the final level.
