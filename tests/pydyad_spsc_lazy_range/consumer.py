from pydyad import Dyad

import argparse
import os
from pathlib import Path


def make_pattern(size):
    return bytes([i % 256 for i in range(size)])


def compute_ranges(file_size, num_ranges):
    # Deterministic (offset, length) pairs spread across the file, varying
    # in size (including a couple of edge cases: offset 0, and a range
    # ending exactly at file_size) so the test exercises more than just one
    # fixed-size middle chunk, and typically spans multiple
    # DYAD_RANGE_CACHE_BLOCK_SIZE-aligned blocks.
    step = max(file_size // (num_ranges + 1), 1)
    ranges = []
    for i in range(num_ranges):
        offset = min(i * step, max(file_size - 1, 0))
        length = min(step + (i * 37) % 101 + 1, file_size - offset)
        if length > 0:
            ranges.append((offset, length))
    return ranges


def fetch_and_check(dyad_io, fname, expected, ranges, label):
    for offset, length in ranges:
        print("[{}] Trying to consume range (offset={}, length={})".format(label, offset, length),
              flush=True)
        got = dyad_io.consume_range(str(fname), offset, length)
        want = expected[offset:offset + length]
        if got != want:
            raise RuntimeError(
                "[{}] Range mismatch at offset={}, length={}: got {} bytes, expected {} bytes".format(
                    label, offset, length, len(got) if got is not None else -1, len(want)
                )
            )
        print("[{}] Correctly consumed range (offset={}, length={})".format(label, offset, length),
              flush=True)


def main():
    parser = argparse.ArgumentParser("Consumer side of pydyad lazy origin-backed range cache test")
    parser.add_argument("cons_managed_dir", type=Path,
                        help="DYAD's consumer managed path")
    parser.add_argument("origin_dir", type=Path,
                        help="Shared-FS directory standing in for the PFS origin")
    parser.add_argument("file_size", type=int,
                        help="Size (bytes) of the file the producer wrote")
    parser.add_argument("num_ranges", type=int,
                        help="Number of byte ranges to fetch and verify")
    args = parser.parse_args()
    cons_dir = args.cons_managed_dir.expanduser().resolve()
    origin_dir = args.origin_dir.expanduser().resolve()
    fname = cons_dir / "range_test_data.bin"
    origin_path = origin_dir / "range_test_data.bin"

    dyad_io = Dyad()
    dyad_io.init_env()

    expected = make_pattern(args.file_size)
    ranges = compute_ranges(args.file_size, args.num_ranges)

    # Pass 1: origin still exists. Each range is a miss on the producer's
    # lazy range cache the first time it's touched, fetched from origin_path
    # and write-through cached into the producer's managed directory by
    # dyad_range_cache_ensure() (invoked from dyad_fetch_range_request_cb()
    # since this file is owned by a different rank than the consumer).
    fetch_and_check(dyad_io, fname, expected, ranges, "pass1-cold")

    # Remove the origin entirely. If pass 1 had left any requested byte
    # uncached, this pass would now fail outright (dyad_range_cache_ensure()
    # would have no origin to fall back to for a real miss) -- so success
    # here proves every previously-requested span was truly cached
    # write-through, not silently re-read from origin on every request.
    if origin_path.exists():
        os.remove(origin_path)
    print("Removed origin file {} -- repeating fetches from cache only".format(origin_path),
          flush=True)

    fetch_and_check(dyad_io, fname, expected, ranges, "pass2-cached")

    dyad_io.finalize()


if __name__ == "__main__":
    main()
