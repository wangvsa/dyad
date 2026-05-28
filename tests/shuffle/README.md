Demonstration of input file shuffling in deep learning.

Deep learning training often requires randomizing the order of input samples at
each epoch. In distributed or parallel training, where each worker processes a
subset of the samples, the set of files assigned to each worker changes at every
epoch due to this randomization.

In this demo, we assume that each sample is stored in a unique file, such as an
image file. We also assume that the dataset is too large to fit entirely within
the local storage of a single worker. As a result, the input data resides on
shared storage.

With DYAD, workers can avoid repeatedly loading files from shared storage at
every epoch. Instead, workers can retrieve files from the local storage of other
workers when needed. Initially, the dataset is partitioned across workers, and
each worker loads the files in its partition into local storage.

In this demo, instead of staging each worker’s partition from shared storage
into local storage, we mimic a partitioned dataset by having each worker create
the files belonging to its partition directly in its local storage under a
DYAD-managed directory.

TODO: As local storage reaches its capacity limit, some files will need to be
randomly evicted to make room for files that are not locally available and must
otherwise be fetched from another worker’s local storage or from shared storage.
TODO: staging in from shared storage to locally managed dir.

# Program Usage

```
./shuffle --work-dir <path> [options]
```
## Required Argument
`--work_dir` is the directory where files are made ready for processing.
It can reside on either local or shared storage, controlled by `--is-local`.
When local, each worker has its own work-dir and owns an exclusive partition
of the file list — files are either generated or staged into it before the
epoch loop begins. When shared, all workers reference the same work_dir —
files are either generated into it or assumed to already exist there.

## Options

| Option | Short | Argument | Description |
|---|---|---|---|
| `--count` | `-c` | `N` | Number of files to auto-generate |
| `--list` | `-l` | `<file>` | File containing list of filenames, one per line, without `work-dir` prefix |
| `--work-dir` | `-d` | `<path>` | working data directory |
| `--is-local` | `-i` | `<0\|1>` | `1` if `work-dir` is local, `0` if shared |
| `--generate` | `-g` | — | Generate files (default: off) |
| `--shared-dir` | `-S` | `<path>` | Shared storage path |
| `--size` | `-z` | `<n>[K\|M\|G]` | Size of each generated file (default: `4096`); suffix optional |
| `--epochs` | `-e` | `<n>` | Number of epochs (default: `1`) |
| `--seed` | `-s` | `<n>` | Random seed (default: random) |
| `--validate` | `-v` | — | Validate file contents after reading (default: off); requires `--generate` |
| `--help` | `-h` | — | Print usage message |

Either `--count` or `--list` is required, but not both.
`--is-local` is always required.
`--size` is valid only when files are being generated (via `--count` or `--list --generate`); it is an error otherwise.

TODO: --shared-dir, --generate

## Option Combinations

| Mode | `--is-local` | `--generate` | `--shared-dir` | `--size` | Behavior |
|---|---|---|---|---|---|
| `--count N` | either | implicit; explicit allowed | not given | optional | Generate `N` files with auto-generated names into `work-dir` |
| `--count N` | `1` (local) | implicit | given | optional | Generate `N` files into `shared_dir`, then stage into local `work-dir` |
| `--count N` | `0` (shared) | implicit | given | — | **Error** |
| `--list f` | `1` (local) | given | not given | optional | Generate files listed in `f` into local `work-dir` |
| `--list f` | `1` (local) | not given | given | — | Stage files from `shared_dir` into local `work-dir` |
| `--list f` | `1` (local) | not given | not given | — | **Error**: one of `--generate` or `--shared-dir` must be given |
| `--list f` | `0` (shared) | given | not given | optional | Generate files listed in `f` into shared `work-dir` |
| `--list f` | `0` (shared) | not given (default) | not given | — | Files assumed to already exist in shared `work-dir` |
| `--list f` | `0` (shared) | either | given | — | **Error** |

## Program Behavior Summary

### Startup

1. Every worker parses and validates command-line options independently.
2. If `--list` is given, the first worker loads the file list and broadcasts it to all others. If `--count` is given, every worker populates the file list locally and independently in the same deterministic way — no communication needed.

### Sample file preparation
Before the epoch loop begins, each worker prepares its own mutually exclusive partition of the file list on its local work-dir.
What "preparation" means depends on the options specified — files may be freshly generated, staged (copied) from a shared storage location, or assumed to already exist.
The full matrix of valid option combinations and their corresponding actions is described in the [Option Combinations](#option-combinations) table above.

### Epoch Loop

For each epoch (repeated `--epochs` times):

1. The file list is shuffled randomly at the beginning of each epoch.
2. Every worker performs the shuffle independently using an RNG initialized with the same seed, so all workers see the identical shuffled order without any communication.
3. Each worker processes (reads) its assigned subset of files from the shuffled list.
4. All workers synchronize at the end of each epoch.

## Auto-Generated File Naming

When `--count N` is used, files are named sequentially as:

```
file_0.dat
file_1.dat
file_2.dat
...
file_N-1.dat
```

## File Size

`--size` sets the size of each generated file and defaults to `4096` bytes. An optional suffix can be used:

| Suffix | Multiplier | Example |
|---|---|---|
| none | 1 | `--size 4096` → 4096 bytes |
| `K` | 1024 | `--size 4K` → 4096 bytes |
| `M` | 1024²  | `--size 2M` → 2097152 bytes |
| `G` | 1024³  | `--size 1G` → 1073741824 bytes |

## Examples

Use mpi launcher as `mpirun`, `srun` or `flux run`. e.g., `mpirun -N 2 -n 64 ./shuffle ...`

Generate 500 files into a local working directory:
```bash
./shuffle --count 500 --work-dir /local/work-dir --is-local 1
```

Generate 500 files of 2MB each into shared storage, then stage into local work-dir directory:
```bash
./shuffle --count 500 --work-dir /local/work-dir --is-local 1 --shared-dir /shared/storage --size 2M
```

Use a file list, generating files of 1GB each into a local work-dir directory:
```bash
./shuffle --list filelist.txt --work-dir /local/work-dir --is-local 1 --generate --size 1G
```

Use a file list, staging files from shared storage into local work-dir directory:
```bash
./shuffle --list filelist.txt --work-dir /local/work-dir --is-local 1 --shared-dir /shared/storage
```

Use a file list, generating files into shared work-dir directory:
```bash
./shuffle --list filelist.txt --work-dir /shared/work-dir --is-local 0 --generate --size 512K
```

Use a file list, files already exist in shared work-dir directory:
```bash
./shuffle --list filelist.txt --work-dir /shared/work-dir --is-local 0
```

With optional epochs and seed:
```bash
./shuffle --count 100 --work-dir /local/work-dir --is-local 1 --epochs 10 --seed 42
```
