#!/usr/bin/env python3
"""
Convert a Liberty (.lib) file to a JSON file using the existing libertyParser.

Usage:
    python liberty_to_json.py <input.lib> [output.json]

If output path is omitted, the file is written as 'lib_data.json'
in the current directory.
"""

import argparse
import json
import collections
from libertyParser import libertyParser


def to_serializable(obj):
    """Fallback converter for json.dump (handles OrderedDict)."""
    if isinstance(obj, collections.OrderedDict):
        return dict(obj)
    raise TypeError(f"Object of type {type(obj)} is not JSON serializable")


def main():
    parser = argparse.ArgumentParser(description="Export Liberty file to JSON")
    parser.add_argument("liberty_file", help="Path to .lib file")
    parser.add_argument(
        "json_file",
        nargs="?",
        default="lib_data.json",
        help="Output JSON path (default: lib_data.json)",
    )
    args = parser.parse_args()

    lp = libertyParser(args.liberty_file)        # unmodified parser
    with open(args.json_file, "w") as fp:
        json.dump(lp.libDic, fp, indent=2, default=to_serializable)

    print(f"✅  Exported '{args.liberty_file}' → '{args.json_file}'")


if __name__ == "__main__":
    main()
