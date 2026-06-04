.. _dyad_dev_api_core:

C/C++ API Reference
===================

Core API
========

Initialization and finalization
--------------------------------

.. doxygenfunction:: dyad_init
   :project: dyad

.. doxygenfunction:: dyad_init_env
   :project: dyad

.. doxygenfunction:: dyad_ctx_init
   :project: dyad

.. doxygenfunction:: dyad_ctx_get
   :project: dyad

.. doxygenfunction:: dyad_finalize
   :project: dyad

.. doxygenfunction:: dyad_ctx_fini
   :project: dyad

.. doxygenfunction:: dyad_clear
   :project: dyad

Path management
---------------

.. doxygenfunction:: dyad_set_prod_path
   :project: dyad

.. doxygenfunction:: dyad_set_cons_path
   :project: dyad

Return codes
------------

.. doxygenenum:: dyad_core_return_codes
   :project: dyad

.. doxygendefine:: DYAD_IS_ERROR
   :project: dyad


Internal implementation
-----------------------

The following documents all internal functions from the core client
implementation that are not exposed in the public headers.

.. doxygenfile:: dyad_client.c
   :project: dyad
