from pydyad import Dyad

import argparse
from pathlib import Path


CONS_DIR = None


def make_pattern(size):
    return bytes([i % 256 for i in range(size)])


def main():
    parser = argparse.ArgumentParser("Consumes byte ranges for pydyad byte-range test")
    parser.add_argument("cons_managed_dir", type=Path,
                        help="DYAD's consumer managed path")
    parser.add_argument("file_size", type=int,
                        help="Size (bytes) of the file the producer wrote")
    parser.add_argument("num_ranges", type=int,
                        help="Number of byte ranges to fetch and verify")
    args = parser.parse_args()
    global CONS_DIR
    CONS_DIR = args.cons_managed_dir.expanduser().resolve()
    fname = CONS_DIR / "range_test_data.bin"

    dyad_io = Dyad()
    dyad_io.init_env()

    # Deterministic (offset, length) pairs spread across the file, varying
    # in size (including a couple of edge cases: offset 0, and a range
    # ending exactly at file_size) so the test exercises more than just one
    # fixed-size middle chunk.
    expected = make_pattern(args.file_size)
    step = max(args.file_size // (args.num_ranges + 1), 1)
    for i in range(args.num_ranges):
        offset = min(i * step, max(args.file_size - 1, 0))
        length = min(step + (i * 37) % 101 + 1, args.file_size - offset)
        if length <= 0:
            continue
        print("Trying to consume range (offset={}, length={})".format(offset, length), flush=True)
        got = dyad_io.consume_range(str(fname), offset, length)
        want = expected[offset:offset + length]
        if got != want:
            raise RuntimeError(
                "Range mismatch at offset={}, length={}: got {} bytes, expected {} bytes".format(
                    offset, length, len(got) if got is not None else -1, len(want)
                )
            )
        print("Correctly consumed range (offset={}, length={})".format(offset, length), flush=True)

    dyad_io.finalize()


if __name__ == "__main__":
    main()
