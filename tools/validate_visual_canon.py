#!/usr/bin/env python3
"""Validate UI source colors against the DAK Sovereign Canon."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

ALLOWED = {
    # Shared 2026 semantic light/dark token extension.
    "#F4F8FD",
    "#0F2A45",
    "#EAF1FA",
    "#173A5E",
    "#122C45",
    "#E8EDF2",
    "#314B66",
    "#D0DCE8",
    "#4D6177",
    "#B3C4D6",
    "#004E98",
    "#70A5FF",
    "#1A5FDB",
    "#8AB6FF",
    "#071A2D",
    "#B5122D",
    "#FF7B86",
    "#9C1026",
    "#FF9AA2",
    "#CAD8E8",
    "#31516F",
    "#D9E8FF",
    "#244F82",
    "#6B7C90",
    "#9BAFC4",
    "#FFFFFF",
    "#F4F6FA",
    "#0039A6",
    "#0A1E46",
    "#D52B1E",
    "#B51A10",
}
HEX = re.compile(r"#[0-9A-Fa-f]{6}\b")
RGB = re.compile(r"\bRGB\(\s*(\d{1,3})\s*,\s*(\d{1,3})\s*,\s*(\d{1,3})\s*\)", re.IGNORECASE)

AAA_TEXT_PAIRS = (
    ("#0A1E46", "#F4F6FA"),
    ("#0039A6", "#F4F6FA"),
    ("#FFFFFF", "#0039A6"),
    ("#FFFFFF", "#0A1E46"),
    ("#122C45", "#F4F8FD"),
    ("#E8EDF2", "#071A2D"),
    ("#FFFFFF", "#004E98"),
    ("#071A2D", "#70A5FF"),
)


def rgb_hex(red: str, green: str, blue: str) -> str:
    values = tuple(int(item) for item in (red, green, blue))
    if any(item < 0 or item > 255 for item in values):
        raise ValueError(f"RGB component outside 0..255: {values}")
    return "#" + "".join(f"{item:02X}" for item in values)


def luminance(color: str) -> float:
    channels = [int(color[index:index + 2], 16) / 255 for index in (1, 3, 5)]
    linear = [
        channel / 12.92 if channel <= 0.04045
        else ((channel + 0.055) / 1.055) ** 2.4
        for channel in channels
    ]
    return 0.2126 * linear[0] + 0.7152 * linear[1] + 0.0722 * linear[2]


def contrast_ratio(foreground: str, background: str) -> float:
    first, second = sorted((luminance(foreground), luminance(background)), reverse=True)
    return (first + 0.05) / (second + 0.05)


def inspect(path: Path) -> list[str]:
    source = path.read_text(encoding="utf-8")
    found = [value.upper() for value in HEX.findall(source)]
    for match in RGB.finditer(source):
        found.append(rgb_hex(*match.groups()))
    if not found:
        raise ValueError(f"{path}: no explicit UI color declarations found")
    invalid = sorted(set(found) - ALLOWED)
    if invalid:
        raise ValueError(f"{path}: non-canonical UI colors: {', '.join(invalid)}")
    return found


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("files", nargs="+", type=Path, help="UI theme source files to inspect")
    args = parser.parse_args()

    try:
        for path in args.files:
            colors = inspect(path)
            print(f"OK {path}: {len(colors)} canonical color declarations")
        for foreground, background in AAA_TEXT_PAIRS:
            ratio = contrast_ratio(foreground, background)
            if ratio < 7.0:
                raise ValueError(
                    f"required AAA text pair failed: {foreground} on {background} = {ratio:.2f}:1"
                )
        print("OK canonical ordinary-text pairs satisfy WCAG AAA (7:1)")
    except (OSError, UnicodeError, ValueError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
