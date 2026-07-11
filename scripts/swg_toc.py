#!/usr/bin/env python3
"""Find or extract one asset through an SWG external .toc index."""

from __future__ import annotations

import argparse
import os
import struct
import sys
import zlib
from dataclasses import dataclass
from pathlib import Path


HEADER = struct.Struct("<4s4s4B6I")
ENTRY = struct.Struct("<BBHIIIII")
TOC_TOKEN = b" COT"
SUPPORTED_VERSIONS = {b"1000", b"2000"}


class TocError(Exception):
    pass


@dataclass(frozen=True)
class TocEntry:
    name: str
    tree_file: str
    compressor: int
    offset: int
    length: int
    compressed_length: int
    crc: int


def _decompress(data: bytes, expected_size: int, label: str) -> bytes:
    try:
        result = zlib.decompress(data)
    except zlib.error as exc:
        raise TocError(f"failed to decompress {label}: {exc}") from exc
    if len(result) != expected_size:
        raise TocError(
            f"{label} decompressed to {len(result)} bytes; expected {expected_size}"
        )
    return result


def _split_cstrings(data: bytes, count: int, label: str) -> list[str]:
    result: list[str] = []
    offset = 0
    for _ in range(count):
        try:
            end = data.index(b"\0", offset)
        except ValueError as exc:
            raise TocError(f"unterminated string in {label}") from exc
        result.append(data[offset:end].decode("latin-1"))
        offset = end + 1
    return result


def read_toc(path: Path) -> list[TocEntry]:
    with path.open("rb") as stream:
        raw_header = stream.read(HEADER.size)
        if len(raw_header) != HEADER.size:
            raise TocError(f"{path}: truncated header")

        (
            token,
            version,
            toc_compressor,
            name_compressor,
            _unused_one,
            _unused_two,
            number_of_files,
            toc_size,
            name_size,
            uncompressed_name_size,
            number_of_tree_files,
            tree_name_size,
        ) = HEADER.unpack(raw_header)

        if token != TOC_TOKEN:
            raise TocError(f"{path}: unexpected token {token!r}")
        if version not in SUPPORTED_VERSIONS:
            raise TocError(f"{path}: unsupported version {version!r}")

        tree_names_raw = stream.read(tree_name_size)
        if len(tree_names_raw) != tree_name_size:
            raise TocError(f"{path}: truncated tree-file name block")
        tree_names = _split_cstrings(
            tree_names_raw, number_of_tree_files, "tree-file name block"
        )

        toc_raw = stream.read(toc_size)
        if len(toc_raw) != toc_size:
            raise TocError(f"{path}: truncated table-of-contents block")
        expected_toc_size = number_of_files * ENTRY.size
        if toc_compressor:
            toc_raw = _decompress(toc_raw, expected_toc_size, "TOC block")
        elif len(toc_raw) != expected_toc_size:
            raise TocError(
                f"{path}: TOC is {len(toc_raw)} bytes; expected {expected_toc_size}"
            )

        names_raw = stream.read(name_size)
        if len(names_raw) != name_size:
            raise TocError(f"{path}: truncated asset name block")
        if name_compressor:
            names_raw = _decompress(
                names_raw, uncompressed_name_size, "asset name block"
            )
        elif len(names_raw) != uncompressed_name_size:
            raise TocError(
                f"{path}: name block is {len(names_raw)} bytes; "
                f"expected {uncompressed_name_size}"
            )

    entries: list[TocEntry] = []
    name_offset = 0
    for index in range(number_of_files):
        (
            compressor,
            _unused,
            tree_file_index,
            crc,
            name_length,
            offset,
            length,
            compressed_length,
        ) = ENTRY.unpack_from(toc_raw, index * ENTRY.size)

        if tree_file_index >= len(tree_names):
            raise TocError(
                f"{path}: entry {index} references tree index {tree_file_index}, "
                f"but only {len(tree_names)} trees exist"
            )
        name_end = name_offset + name_length
        if name_end >= len(names_raw):
            raise TocError(f"{path}: entry {index} has an invalid name length")
        name = names_raw[name_offset:name_end].decode("latin-1").replace("\\", "/")
        if names_raw[name_end] != 0:
            raise TocError(f"{path}: entry {index} name is not NUL terminated")
        name_offset = name_end + 1

        entries.append(
            TocEntry(
                name=name,
                tree_file=tree_names[tree_file_index],
                compressor=compressor,
                offset=offset,
                length=length,
                compressed_length=compressed_length,
                crc=crc,
            )
        )

    return entries


def extract_entry(toc_path: Path, entry: TocEntry, output_path: Path) -> None:
    tree_path = toc_path.parent / entry.tree_file
    with tree_path.open("rb") as stream:
        stream.seek(entry.offset)
        read_size = entry.length if entry.compressor == 0 else entry.compressed_length
        data = stream.read(read_size)
    if len(data) != read_size:
        raise TocError(
            f"{tree_path}: read {len(data)} bytes at {entry.offset}; expected {read_size}"
        )
    if entry.compressor:
        data = _decompress(data, entry.length, entry.name)
    if len(data) != entry.length:
        raise TocError(f"{entry.name}: extracted size does not match its TOC entry")

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_bytes(data)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Find or extract an exact asset path through SWG .toc files."
    )
    parser.add_argument("--toc", required=True, action="append", type=Path)
    parser.add_argument("--find", required=True, help="Exact internal asset path")
    parser.add_argument("--extract", type=Path, help="Output file for the winning match")
    args = parser.parse_args()

    wanted = args.find.lower().replace("\\", "/")
    matches: list[tuple[Path, TocEntry]] = []
    try:
        for toc_path in args.toc:
            for entry in read_toc(toc_path):
                if entry.name.lower() == wanted:
                    matches.append((toc_path, entry))
                    print(
                        f"{toc_path.name}: {entry.name} -> {entry.tree_file} "
                        f"offset={entry.offset} length={entry.length} "
                        f"compressed={entry.compressed_length} codec={entry.compressor} "
                        f"crc={entry.crc:08x}"
                    )
    except (OSError, TocError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2

    if not matches:
        print(f"not found: {args.find}", file=sys.stderr)
        return 1
    if args.extract:
        # SearchTOC nodes are added in priority order; the last supplied TOC wins.
        toc_path, entry = matches[-1]
        try:
            extract_entry(toc_path, entry, args.extract)
        except (OSError, TocError) as exc:
            print(f"error: {exc}", file=sys.stderr)
            return 2
        print(f"extracted: {args.extract} ({entry.length} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
