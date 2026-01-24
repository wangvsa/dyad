***************
Getting Started
***************

Prerequisites
#############

DYAD has the following minimum requirements to build and install:

* A C11-compliant C compiler
* A C++17-compliant C++ compiler
* CNake 3.12
* pkg-config
* `flux-core <https://github.com/flux-framework/flux-core.git>`_
* `jansson 2.10 or newer <https://github.com/akheron/jansson.git>`_
* flux-python

Optionally, DYAD leverages:
* `mochi-margo <https://github.com/mochi-hpc/mochi-margo.git>`_ to enable libfabric-based data transport layer (DTL).
* `ucx <https://github.com/openucx/ucx.git>`_ to enable ucx-based DTL.
* `dftracer <https://github.com/llnl/dftracer.git>`_ for performance tracing of Python-based applications.
* cpp-logger for logging operational details to aid debugging.
* `perflow-aspect <https://perfflowaspect.readthedocs.io/en/latest/>`_ for visualizing function-level events to aid workflow performance diagnosis.
* `caliper <https://github.com/llnl/Caliper.git>`_ for collecting performance profiling.

Installation
############

Manual Installation
*******************

You can get DYAD from its `GitHub repository <https://github.com/flux-framework/dyad>`_ using
these commands:

.. code-block:: shell

   $ git clone https://github.com/flux-framework/dyad.git
   $ cd dyad

DYAD relies on cmake for both building and installation.

.. code-block:: shell

   $ mkdir build; cd build
   $ cmake -DDYAD_ENABLE_MARGO_DATA=ON \
           -DDYAD_LIBDIR_AS_LIB=ON \
           -DCMAKE_INSTALL_PREFIX=${DYAD_INSTALL_PREFIX} \
           ..
   $ make -j install

.. note::

   The cmake command above is provided as an example. Refer to the options below to customize configuration as needed.
   Set the env variable `DYAD_INSTALL_PREFIX` to the desired installation directory.


To enable the DYAD Python binding,

.. code-block:: shell

   $ python3 -m venv .venv
   $ source .venv/bin/activate
   $ pip install flux-python=0,80.0
   $ cd pydyad
   $ pip install .

.. note::

   When installing *flux-python*, ensure that the version matches *flux-core*.



There are several custom CMake options available to configure a DYAD build:

.. This is how the list-table below would render ideally
   +--------------------------+----------------------------+---------------------------------------------+
   | Flag                     | Values (**default**)       | Description                                 |
   +==========================+============================+=============================================+
   | DYAD_ENABLE_MARGO_DATA   | ON, **OFF**                | Allow dynamic selection of Margo-based DTL  |
   | DYAD_ENABLE_UCX_DATA     | ON, **OFF**                | Allow dynamic selection of UCX-based DTL    |
   | DYAD_ENABLE_UCX_DATA_RMA | ON, **OFF**                | Allow dynamic selection of UCX-based RMA DTL|
   +--------------------------+----------------------------+---------------------------------------------+
   | DYAD_LOGGER              | FLUX, CPP_LOGGER, **NONE** | Choose the method to log stdout/stderr      |
   | DYAD_LOGGER_LEVEL        | DEBUG, INFO, WARN,         | Choose the level of logging                 |
   |                          | ERROR, **NONE**            |                                             |
   +--------------------------+----------------------------+---------------------------------------------+
   | DYAD_PROFILER            | PERFFLOW_ASPECT, CALIPER,  | Choose the performance profiler             |
   |                          | DFTRACER, **NONE**         |                                             |
   +--------------------------+----------------------------+---------------------------------------------+
   | DYAD_ENABLE_TESTS        | ON, **OFF**                | Build unit tests                            |
   | DYAD_LIBDIR_AS_LIB       | ON, **OFF**                | Force lib as library install dir (no lib64) |
   | DYAD_USE_CLANG_LIBCXX    | ON, **OFF**                | Use clang's native runtime instead of gnu   |
   | DYAD_WARNINGS_AS_ERRORS  | ON, **OFF**                | Turn compiler warning into error            |
   +--------------------------+----------------------------+---------------------------------------------+

.. list-table::
   :header-rows: 1
   :widths: 25 25 50

   * - Flag
     - Values (**default**)
     - Description
   * - **Data Transfer Options**
     -
     -
   * - DYAD_ENABLE_MARGO_DATA
     - ON, **OFF**
     - Allow dynamic selection of Margo-based DTL
   * - DYAD_ENABLE_UCX_DATA
     - ON, **OFF**
     - Allow dynamic selection of UCX-based DTL
   * - DYAD_ENABLE_UCX_DATA_RMA
     - ON, **OFF**
     - Allow dynamic selection of UCX-based RMA DTL
   * - **Logging and Profiling**
     -
     -
   * - DYAD_LOGGER
     - FLUX, CPP_LOGGER, **NONE**
     - Choose the method to log stdout/stderr
   * - DYAD_LOGGER_LEVEL
     - DEBUG, INFO, WARN, ERROR, **NONE**
     - Choose the level of logging
   * - DYAD_PROFILER
     - PERFFLOW_ASPECT, CALIPER, DFTRACER, **NONE**
     - Choose the performance profiler
   * - **Compiling/Linking Customization**
     - 
     - 
   * - DYAD_ENABLE_TESTS
     - ON, **OFF**
     - Build unit tests
   * - DYAD_LIBDIR_AS_LIB
     - ON, **OFF**
     - Force lib as library install dir (no lib64)
   * - DYAD_USE_CLANG_LIBCXX
     - ON, **OFF**
     - Use clang's native runtime instead of GNU
   * - DYAD_WARNINGS_AS_ERRORS
     - ON, **OFF**
     - Turn compiler warnings into errors


.. note::

   Mochi-Margo enables seamless adoption of various DTL types. Currently, DYAD
   relies on it only for **libfabric**, but we plan to fully leverage the diverse
   options it provides in the near future.

   To enable a specific DTL type, DYAD requires the environment variable
   ``DYAD_DTL_MODE`` to be set accordingly. At present, three values are
   supported: ``MARGO``, ``UCX``, and ``FLUX_RPC``.

   - When ``DYAD_DTL_MODE`` is set to ``UCX`` and DYAD has been built with the
     CMake option ``DYAD_ENABLE_UCX_DATA_RMA=ON``, data transfer is performed
     asynchronously via **remote memory access (RMA)** to reduce communication
     costs.
   - When built with ``DYAD_ENABLE_UCX_DATA=ON``, data transfer is synchronous
     using UCX. In other words, the choice between synchronous and RMA-based UCX
     is mutually exclusive at compile time.

   However, the selection between ``MARGO``, ``UCX``, and ``FLUX_RPC`` can be
   made dynamically at launch time. Ensure that ``DYAD_DTL_MODE`` is set
   consistently in both the service and client environments.

   If none of the three DTL-related CMake options are set, DYAD defaults to
   using **FLUX RPC** for data transfer. While DYAD currently supports four
   different data transfer methods, the client relies only on FLUX RPC to send
   transfer requests to the service. In the future, we plan to offer alternative,
   portable RPC methods.


Using DYAD's APIs
#################

Currently, DYAD provides APIs for the following programming languages:

* C
* C++
* Python

This section describes the basics of integrating them into an application.

C API
*****

DYAD's C API leverages the `LD_PRELOAD trick <https://www.admin-magazine.com/HPC/Articles/Preload-Trick>`_
to integrate into user applications. As a result, users can utilize DYAD's C API by
simply adding the following before the shell command that launches their application:

.. code-block:: shell

   $ LD_PRELOAD=path/to/dyad_wrapper.so

Once preloaded, DYAD's C API will intercept the :code:`open` and :code:`fopen` functions when consuming
files and the :code:`close` and :code:`fclose` functions when producing files. As a result,
if their code already uses thse functions, users do not need to change their code.

C++ API
*******

DYAD's C++ API is implemented as a small library that wraps C++'s Standard Library file streams.
To use DYAD's C++ API, first, add the following to your code:

.. code-block:: cpp

   #include <dyad/stream/dyad_stream_api.hpp>

This header defines thin wrappers around the file streams provided by the C++ Standard Library.
More specifically, it provides the following classes:

* :code:`dyad::basic_ifstream_dyad`
* :code:`dyad::ifstream_dyad`
* :code:`dyad::wifstream_dyad`
* :code:`dyad::basic_ofstream_dyad`
* :code:`dyad::ofstream_dyad`
* :code:`dyad::wofstream_dyad`
* :code:`dyad::basic_fstream_dyad`
* :code:`dyad::fstream_dyad`
* :code:`dyad::wfstream_dyad`

When using DYAD, these file streams should be used in place of the file streams from the C++
Standard Library. The DYAD file streams should be directly used to do the following:

* Open files (with the file stream's :code:`open` method)
* Close files (with the file stream's :code:`close` method or destructor)
* Access the underlying C++ Standard Library file stream using the DYAD stream's :code:`get_stream` method

All reading from and writing to files should be done using the underlying C++ Standard Library file stream.
A simple example of using DYAD's C++ API in a producer application is shown below:

.. code-block:: cpp

   #include <dyad_stream_api.hpp>

   void produce_file(std::string& full_path, int32_t* data, std::size_t data_size)
   {
       dyad::ofstream_dyad ofs_dyad;
       ofs_dyad.open(full_path, std::ofstream::out | std::ios::binary);
       std::ofstream& ofs = ofs_dyad.get_stream();
       ofs.write((char*) data, data_size);
       ofs_dyad.close();
   }

After replacing C++ Standard Library file streams with their DYAD equivalents,
there is one final requirement to using the C++ API. When compiling your code,
you must link the associated library (i.e., :code:`libdyad_stream.so` or
:code:`libdyad_stream.a`). This library can be found in the :code:`lib`
subdirectory of the install prefix.

Running DYAD
############

There are three steps to running DYAD-enabled applications:

1. :ref:`Create a Flux key-value store (KVS) namespace <Create a Flux KVS Namespace>`
2. :ref:`Determine the managed directories for each application <Determine the Managed Directories for Each Application>`
3. :ref:`Load DYAD's Flux module <Load DYAD's Flux Module>`
4. :ref:`Configure and run the DYAD-enabled applications <Configure and Run the DYAD-Enabled Applications>`

Create a Flux KVS Namespace
***************************

DYAD uses its own namespace in Flux's hierarchical key-value store (KVS) to isolate
itself from the KVS entries from other Flux services. Thus, the first step in running DYAD
is to create a KVS namespace. This namespace is used by DYAD to exchange
file information (e.g., the Flux broker that "owns" a file) needed to synchronize
the consumer application and transfer the file from producer to
consumer. To create this namespace, run the following:

.. code-block:: shell

   $ flux kvs namespace create <DYAD_KVS_NAMESPACE>

The namespace can be whatever string value you want.

Determine the Managed Directories for Each Application
******************************************************

To determine when to perform synchronization and data transfer, DYAD tracks two directories for
each application: a **producer-managed directory** and a **consumer-managed directory**. At least
one of these directories must be specified for DYAD to do anything. If neither are provided, the application
will still run, but DYAD will not do anything.

When a producer-managed directory is provided, DYAD will store information about any file
stored in that directory (or its subdirectories) into a namespace within the Flux key-value
store (KVS). This information is later used by DYAD to transfer files from producer to consumer.

When a consumer-managed directory is provided, DYAD will block the application whenever a
file inside that directory (or subdirectory) is opened. This blocking will last until DYAD sees
information about the file in the Flux KVS namespace. If the information retrieved from the KVS
indicates that the file is actually located elsewhere, DYAD will use Flux's
remote procedure call (RPC) system to ask DYAD's Flux module to transfer the file.
If a transfer occurs, the file's contents will be stored at the file path passed to the
original file opening function (e.g., :code:`open`, :code:`fopen`).

Before running the following steps, determine the producer- and/or consumer-managed directories
for each application. These directories will need to be provided to the commands in the next steps.

.. note::

   When opening or closing a file not in the producer- or consumer-managed directories, DYAD
   will simply open or close the file. DYAD changes the behavior of opening or closing only the
   files in the managed directories.

Load DYAD's Flux Module
***********************

The next step in running DYAD is to load DYAD's Flux module. The module is the component of DYAD
responsible for sending files from producer to consumer. Once loaded, this module will run whenever
its associated Flux broker receives a relevant remote procedure call from a DYAD-enabled consumer. To load the module,
first, determine where :code:`dyad.so` is located. This should normally be :code:`<PREFIX>/lib/dyad.so`. Once you
have found the path to :code:`dyad.so`, you can load the module on the current Flux broker using:

.. code-block:: shell

   $ flux module load path/to/dyad.so <DYAD_PATH_PRODUCER>

The :code:`dyad.so` module takes a single command-line argument: the producer-managed directory. The producer
uses this directory as the root from which the module will look for files to transfer.

Note that the command above will only load the module on the Flux broker on which the command is run.
This can be an issue if you are submitting jobs because you will not know on which broker your jobs will be run.
As a result, it is **highly** recommended that you launch the DYAD module on all brokers in your Flux instance. You can
do this by running:

.. code-block:: shell

   $ flux exec -r all flux module load path/to/dyad.so <DYAD_PATH_PRODUCER>

Configure and Run the DYAD-Enabled Applications
***********************************************

In addition to the two essential variables discussed above—``DYAD_KVS_NAMESPACE``
and ``DYAD_DTL_MODE``—two more variables, ``DYAD_PATH_PRODUCER`` and ``DYAD_PATH_CONSUMER``,
are required to run DYAD-enabled applications.”
The table below list the essential ones. For the full list of variables, refer to
:doc:`runtime_configuration`.


+--------------------------------+-----------------+--------------+----------+-----------------------------------------------------------------+
| Name                           | Type            | Required?    | Default  | Description                                                     |
+================================+=================+==============+==========+=================================================================+
| :code:`DYAD_KVS_NAMESPACE`     | String          | Yes          | N/A      | The Flux KVS namespace that DYAD will use to record or look     |
|                                |                 |              |          |                                                                 |
|                                |                 |              |          | for file information                                            |
+--------------------------------+-----------------+--------------+----------+-----------------------------------------------------------------+
| :code:`DYAD_PATH_PRODUCER`     | Directory Path  | Yes [#one]_  | N/A      | The producer-managed path of the application                    |
+--------------------------------+                 +              +          +-----------------------------------------------------------------+
| :code:`DYAD_PATH_CONSUMER`     |                 |              |          | The consumer-managed path of the application                    |
+--------------------------------+-----------------+--------------+----------+-----------------------------------------------------------------+
| :code:`DYAD_DTL_MODEE`         | String          | No           | FLUX_RPC | Choose data transfer method among MARGO, UCX, FLUX_RPC          |
+--------------------------------+-----------------+--------------+----------+-----------------------------------------------------------------+

.. [#one] For DYAD to do anything, at least one of :code:`DYAD_PATH_PRODUCER` or :code:`DYAD_PATH_CONSUMER` must be provided.
   Applications will still work if neither are provided, but DYAD will not do anything.

