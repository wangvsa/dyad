.. _dyad_entry_and_init:

DYAD entry points and initalization
###################################


.. _dyad_entry_and_init_client:

Client
======

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


.. _dyad_entry_and_init_service:

Service
=======

.. figure:: _static/figs/dyad_flux_module_init.svg
   :alt: DYAD Flux module initialization

   The DYAD Flux module follows the same ``dyad_ctx_init()`` →
   ``dyad_init_env()`` -> ``dyad_init()`` chain as the C GOTCHA wrapper
   (shown above), but with an additional step — ``mod_main()`` first calls
   ``dyad_module_ctx_init()``, which applies any command-line argument
   overrides to environment variables before delegating to
   ``dyad_ctx_init()``. The module also differs in two other key ways: it
   initializes with ``DYAD_COMM_SEND`` (producer mode) rather than
   ``DYAD_COMM_RECV`` (consumer mode), and it adopts the Flux handle
   provided by the broker rather than opening its own.


