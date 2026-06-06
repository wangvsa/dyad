.. _DYAD_debugging_tips:

***********************************
Common Tips for Debugging with DYAD
***********************************

Debugging distributed operations of mutiple jobs coordination under batch system is quite challending. Here are several tips.


Build DYAD for Debugging
========================

To facilitate debugging, DYAD provides several CMake options that can be enabled
at build time.

- **For users:** Enable DYAD logging support:

  ::

     -DDYAD_LOGGER=FLUX|CPP_LOGGER -DDYAD_LOGGER_LEVEL=DEBUG

- **For developers:** Treat all compiler warnings as errors:

  ::

     -DDYAD_WARNINGS_AS_ERRORS=ON

- **For developers:** Use Clang with AddressSanitizer as needed:

  ::

     -DCMAKE_C_COMPILER=clang
     -DCMAKE_CXX_COMPILER=clang++
     -DCMAKE_BUILD_TYPE=Debug


Runtime Logging
===============

- Enable Flux logging when starting an instance to capture DYAD logs:

::

   flux start -v -o,-S,log-filename=out.txt

- Enable stdout and stderr forwarding with allocation (see `flux logging <https://flux-framework.readthedocs.io/projects/flux-core/en/stable/man7/flux-broker-attributes.html#logging>`_):

::

   flux alloc -N 2 --broker-opts=--setattr=log-filename="$PWD/flux-${USER}.log" --broker-opts=--setattr=log-level=7 --broker-opts=--setattr=log-forward-level=7 --broker-opts=--setattr=log-critical-level=7 --broker-opts=--setattr=log-stderr-level=7 --broker-opts=--setattr=log-syslog-enable=1 --broker-opts=--setattr=log-stderr-mode=leader


Controlling Job Standard I/O
============================

Flux job-related options can be used to control standard I/O behavior (see
`flux-run <https://flux-framework.readthedocs.io/projects/flux-core/en/latest/man1/flux-run.html>`_):

- Disable output buffering:

  ::

     -u, --unbuffered

- Label output by rank:

  ::

     -l, --label-io

- Redirect job output streams:

  ::

     --output=, --error=, --log=, --log-stderr=

- Use
  `mustache templates <https://flux-framework.readthedocs.io/projects/flux-core/en/latest/man1/flux-submit.html#mustache-templates>`_
  for fine-controlling output.


Simulated Multi-Node Debugging
==============================

Use a single node with a simulated multi-node setup via
``flux start --test-size=N``. In this configuration, DYAD should use different
managed paths to mimic operations on distinct nodes.


Common Debugging Steps
======================

When isolating errors in DYAD-enabled applications, the following steps are
recommended:

- Verify environment variable propagation by running a script that prints all
  DYAD-related environment variables in place of a DYAD job.
- Ensure environment variables are set consistently between producers and consumers.
- Confirm that ``DYAD_KVS_NAMESPACE`` is set and that the namespace exists in the KVS. ``flux kvs namespace list``
- Clear any namespaces or files left over from previous runs.
- Inspect logging output to identify where a DYAD consumer may be hanging or where
  a DYAD job may have crashed.
- Inspect `KVS <https://flux-framework.readthedocs.io/projects/flux-core/en/latest/man1/flux-kvs.html>`_  entries at both the producer and consumer as needed. ``flux kvs dir -N ${DYAD_KVS_NAMESPACE} [key]``

