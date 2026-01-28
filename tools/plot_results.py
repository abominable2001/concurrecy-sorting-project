#!/usr/bin/env python3
"""
Compatibility wrapper.

Primary script lives at: figures/tools/plot_results.py
This wrapper lets you run:
  python3 tools/plot_results.py results.csv 1000000
"""

import runpy
from pathlib import Path
import os
import sys


def main() -> None:
    # If the user runs with system Python (no matplotlib), transparently
    # re-exec with the local venv interpreter if it exists.
    if os.environ.get("VIRTUAL_ENV") is None:
        venv_bin = Path(__file__).resolve().parents[1] / "venv" / "bin"
        for name in ("python3", "python", "python3.14"):
            venv_py = venv_bin / name
            if venv_py.exists():
                os.execv(str(venv_py), [str(venv_py), *sys.argv])

    target = Path(__file__).resolve().parents[1] / "figures" / "tools" / "plot_results.py"
    runpy.run_path(str(target), run_name="__main__")


if __name__ == "__main__":
    main()

