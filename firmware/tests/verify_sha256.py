#!/usr/bin/env python3
"""Verify Instinct golden SHA256SUMS.txt."""

from __future__ import annotations

import hashlib
import sys
from pathlib import Path


def main() -> int:
    sums_path = Path(sys.argv[1] if len(sys.argv) > 1 else "design/instinct-firmware-goldens-pr1/SHA256SUMS.txt")
    root = sums_path.parent
    fail = 0
    for line in sums_path.read_text().splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        expected, name = line.split()
        path = root / name
        if not path.exists():
            print(f"FAIL missing {path}")
            fail += 1
            continue
        got = hashlib.sha256(path.read_bytes()).hexdigest()
        if got != expected:
            print(f"FAIL {name}\n  expected {expected}\n  got      {got}")
            fail += 1
            continue
        print(f"ok  {name}")
    if fail:
        print(f"{fail} hash mismatches")
        return 1
    print("SHA256SUMS ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
