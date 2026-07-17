from pydyad import Dyad

import argparse
from pathlib import Path


def make_pattern(size):
    return bytes([i % 256 for i in range(size)])


def main():
    parser = argparse.ArgumentParser("Producer side of pydyad lazy origin-backed range cache test")
    parser.add_argument("prod_managed_dir", type=Path,
                        help="DYAD's producer managed path")
    parser.add_argument("origin_dir", type=Path,
                        help="Shared-FS directory standing in for the PFS origin")
    parser.add_argument("file_size", type=int,
                        help="Size (bytes) of the file to produce")
    args = parser.parse_args()
    prod_dir = args.prod_managed_dir.expanduser().resolve()
    origin_dir = args.origin_dir.expanduser().resolve()
    origin_dir.mkdir(parents=True, exist_ok=True)

    # Write the "real" data directly to the origin (PFS-like) location --
    # NOT through DYAD, and NOT into prod_managed_dir. dyad_range_cache_ensure()
    # is responsible for lazily copying pieces of this into prod_managed_dir
    # on demand; this test intentionally never stages it upfront (unlike
    # tests/pydyad_spsc_range, which writes directly into the managed dir).
    origin_path = origin_dir / "range_test_data.bin"
    with open(origin_path, "wb") as f:
        f.write(make_pattern(args.file_size))
    print("Wrote {} bytes directly to origin {}".format(args.file_size, origin_path), flush=True)

    local_path = prod_dir / "range_test_data.bin"
    if local_path.exists():
        raise RuntimeError(
            "{} already exists -- test requires the local copy to not be pre-staged".format(
                local_path
            )
        )

    dyad_io = Dyad()
    dyad_io.init_env()
    # Announce ownership of the not-yet-materialized local path -- no data
    # movement (mirrors pecan/datacopy_flat_dyad.py's simplified announce-only
    # flow once upfront `cp` staging is dropped in favor of lazy caching).
    dyad_io.produce(str(local_path))
    print("Announced ownership of {} (not yet materialized locally)".format(local_path), flush=True)


if __name__ == "__main__":
    main()
