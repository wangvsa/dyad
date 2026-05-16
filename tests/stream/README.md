# How to compile and run tests/stream

## 1. Build and install DYAD

Set `DYAD_INSTALL` to the DYAD install location and add the `dyad` binary to your PATH:

```bash
export DYAD_INSTALL=/path/to/dyad/install
export PATH=${DYAD_INSTALL}/bin:${PATH}
```

## 2. Compile test_stream

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

## 3. Start a Flux session and the DYAD service

```bash
export DYAD_PATH=/tmp/${USER}/dyad
flux start --test-size=2
```

Inside the Flux session:

```bash
dyad start -p ${DYAD_PATH}
```

## 4. Run the test

`test_stream` takes 4 arguments: `<dyad_path> <file> <write=1|read=0> <use_open_close=1|0>`

```bash
# Producer (writer)
flux run -n 1 ./test_stream ${DYAD_PATH} ${DYAD_PATH}/test.txt 1 0

# Consumer (reader) — run after producer
flux run -n 1 ./test_stream ${DYAD_PATH} ${DYAD_PATH}/test.txt 0 0
```

## 5. Stop the DYAD service and exit

```bash
dyad stop
exit
```

---

**Note:** The stream tests are not integrated into CMake/ctest. They are manual
integration tests that require a live Flux environment with the DYAD module loaded.
`combined_test.cpp` and `io_test.cpp` support test logic (not standalone executables),
while `test_stream.cpp` is the main entry point.
