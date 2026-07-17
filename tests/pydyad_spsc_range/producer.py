from pydyad import dyad_open

import argparse
from pathlib import Path


PROD_DIR = None


def make_pattern(size):
    return bytes([i % 256 for i in range(size)])


def main():
    parser = argparse.ArgumentParser("Generates data for pydyad byte-range test")
    parser.add_argument("prod_managed_dir", type=Path,
                        help="DYAD's producer managed path")
    parser.add_argument("file_size", type=int,
                        help="Size (bytes) of the file to produce")
    args = parser.parse_args()
    global PROD_DIR
    PROD_DIR = args.prod_managed_dir.expanduser().resolve()
    fname = PROD_DIR / "range_test_data.bin"

    with dyad_open(fname, "wb") as f:
        f.write(make_pattern(args.file_size))
    print("Successfully produced {} bytes to {}".format(args.file_size, fname), flush=True)


if __name__ == "__main__":
    main()
