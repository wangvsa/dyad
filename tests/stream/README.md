How to compile and run tests/stream

  1. Build and install DYAD first

  Make sure `DYAD_INSTALL` is set to the dyad install locatoin
  ```bash
  export DYAD_INSTALL=/path/to/dyad/install/
  ```

  2. Compile test_stream manually

  The test includes dyad_stream_api.hpp and links against the installed DYAD libraries:

  ```bash
  cd tests/stream
  g++ -std=c++14 -g -O0 \
      -DDYAD_HAS_CONFIG \
      -I${DYAD_INSTALL}/include \
      test_stream.cpp \
      -L${DYAD_INSTALL}/lib \
      -ldyad_fstream \
      -o test_stream
  ```

  3. Set up Flux and load the DYAD module

  The test requires a running Flux instance with the DYAD Flux module loaded. startup.sh does this:

  # then run inside a flux session:
  ```bash
  flux start --test-size=2
  ```
  startup.sh creates `/tmp/$USER/dyad` as the managed directory, and creates a test KVS namespace.

  4. Run the test

  test_stream takes 4 arguments: <dyad_path> <file> <write=1|read=0> <use_open_close=1|0>

  ```bash
  export DYAD_PATH=/tmp/${USER}/dyad
  bash startup.sh
  ```
  # Producer (writer)
  flux run -n 1 ./test_stream ${DYAD_PATH} ${DYAD_PATH}/test.txt 1 0

  # Consumer (reader) — run after producer
  flux run -n 1 ./test_stream ${DYAD_PATH} ${DYAD_PATH}/test.txt 0 0

  batch.sh shows the full Flux-based workflow — it submits producer and consumer as separate Flux jobs and lets DYAD handle
  synchronization:

  # Inside a flux session with the module loaded:
  bash batch.sh

  ---
  Note: The stream tests are not integrated into CMake/ctest. They are manual integration tests that require a live Flux
  environment with the DYAD module loaded. The combined_test.cpp and io_test.cpp files support test logic (not standalone
  executables), while test_stream.cpp is the main entry point.
