.. _dyad_dev_api_service:

DYAD Service API
################

The DYAD service runs on the producer side, serving file data to consumers via
RPC for coordination and a communication channel optimizated for data transport.

.. _dyad_dev_api_service_flux_module:

Flux module
===========

DYAD Flux module runs as a broker plugin of flux-core.
It is loaded using
``flux module load dyad.so [options] [producer_path]``.

.. doxygenfile:: dyad.c
   :project: dyad
