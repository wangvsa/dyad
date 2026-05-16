How to compile and run tests/stream

  1. Build and install DYAD first

  Make sure `DYAD_INSTALL` is set to the dyad install location
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

  3. Set up Flux and start DYAD service

  The test requires a running Flux instance with the DYAD Flux module loaded.

  # run inside a flux session:
  ```bash
  export DYAD_PATH=/tmp/${USER}/dyad
  flux start --test-size=2
  dyad start -p ${DYAD_PATH}
  ```

  4. Run the test

  test_stream takes 4 arguments: <dyad_path> <file> <write=1|read=0> <use_open_close=1|0>

  # Producer (writer)
  flux run -n 1 ./test_stream ${DYAD_PATH} ${DYAD_PATH}/test.txt 1 0

  # Consumer (reader) — run after producer
  flux run -n 1 ./test_stream ${DYAD_PATH} ${DYAD_PATH}/test.txt 0 0

  5. Stop the DYAD service

  ```bash
  dyad stop
  exit # exit the Flux environment
  ```

  ---
  Note: The stream tests are not integrated into CMake/ctest. They are manual integration tests that require a live Flux
  environment with the DYAD module loaded. The combined_test.cpp and io_test.cpp files support test logic (not standalone
  executables), while test_stream.cpp is the main entry point.
