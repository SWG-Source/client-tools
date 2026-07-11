#!/usr/bin/env python3
"""Decode a compiled SWG DataTable IFF into a tab-separated text file."""

from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path


class IffError(Exception):
    pass


def _read_chunk(data: bytes, offset: int) -> tuple[bytes, int, int, int]:
    if offset + 8 > len(data):
        raise IffError(f"truncated chunk header at {offset}")
    tag = data[offset : offset + 4]
    size = struct.unpack_from(">I", data, offset + 4)[0]
    payload = offset + 8
    next_offset = payload + size
    if next_offset > len(data):
        raise IffError(f"chunk {tag!r} at {offset} overruns the file")
    return tag, size, payload, next_offset


def _enter_form(data: bytes, offset: int) -> tuple[bytes, int, int]:
    tag, _size, payload, next_offset = _read_chunk(data, offset)
    if tag != b"FORM":
        raise IffError(f"expected FORM at {offset}, found {tag!r}")
    return data[payload : payload + 4], payload + 4, next_offset


class Cursor:
    def __init__(self, data: bytes, offset: int, end: int):
        self.data = data
        self.offset = offset
        self.end = end

    def int32(self) -> int:
        if self.offset + 4 > self.end:
            raise IffError("integer cell overruns its chunk")
        value = struct.unpack_from("<i", self.data, self.offset)[0]
        self.offset += 4
        return value

    def float32(self) -> float:
        if self.offset + 4 > self.end:
            raise IffError("float cell overruns its chunk")
        value = struct.unpack_from("<f", self.data, self.offset)[0]
        self.offset += 4
        return value

    def string(self) -> str:
        try:
            end = self.data.index(b"\0", self.offset, self.end)
        except ValueError as exc:
            raise IffError("unterminated string cell") from exc
        value = self.data[self.offset:end].decode("latin-1")
        self.offset = end + 1
        return value


def _basic_type(specification: str) -> str:
    code = specification.strip().lower()[:1]
    if code == "f":
        return "float"
    if code in {"i", "h", "b", "e", "v"}:
        return "int"
    return "string"


def read_datatable(path: Path) -> tuple[str, list[str], list[str], list[list[str]]]:
    data = path.read_bytes()
    form_type, child_start, _child_end = _enter_form(data, 0)
    if form_type != b"DTII":
        raise IffError(f"{path}: top-level form is {form_type!r}, not DTII")
    version_tag, version_start, version_end = _enter_form(data, child_start)
    version = version_tag.decode("latin-1")

    columns: list[str] | None = None
    types: list[str] | None = None
    rows_chunk: tuple[int, int] | None = None
    offset = version_start
    while offset < version_end:
        tag, _size, payload, next_offset = _read_chunk(data, offset)
        cursor = Cursor(data, payload, next_offset)
        if tag == b"COLS":
            columns = [cursor.string() for _ in range(cursor.int32())]
        elif tag == b"TYPE":
            if columns is None:
                raise IffError("TYPE appears before COLS")
            if version == "0001":
                types = [cursor.string() for _ in columns]
            else:
                type_names = {0: "i", 1: "f", 2: "s"}
                types = [type_names.get(cursor.int32(), "s") for _ in columns]
        elif tag == b"ROWS":
            rows_chunk = (payload, next_offset)
        offset = next_offset

    if columns is None or types is None or rows_chunk is None:
        raise IffError("datatable is missing COLS, TYPE, or ROWS")

    cursor = Cursor(data, *rows_chunk)
    number_of_rows = cursor.int32()
    rows: list[list[str]] = []
    for _ in range(number_of_rows):
        row: list[str] = []
        for specification in types:
            cell_type = _basic_type(specification)
            if cell_type == "int":
                row.append(str(cursor.int32()))
            elif cell_type == "float":
                row.append(f"{cursor.float32():g}")
            else:
                row.append(cursor.string())
        rows.append(row)
    if cursor.offset != cursor.end:
        raise IffError(
            f"ROWS decoder stopped at {cursor.offset}, chunk ends at {cursor.end}"
        )
    return version, columns, types, rows


def _quote(value: str) -> str:
    if any(character in value for character in ('\t', '\n', '"')):
        return '"' + value.replace('"', '""') + '"'
    return value


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("iff", type=Path)
    parser.add_argument("--out", type=Path)
    parser.add_argument("--head", type=int, default=0)
    args = parser.parse_args()
    try:
        version, columns, types, rows = read_datatable(args.iff)
    except (OSError, IffError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2

    lines = ["\t".join(map(_quote, columns)), "\t".join(map(_quote, types))]
    lines.extend("\t".join(map(_quote, row)) for row in rows)
    output = "\n".join(lines) + "\n"
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(output, encoding="latin-1", newline="\n")
    if args.head:
        print("\t".join(columns))
        for row in rows[: args.head]:
            print("\t".join(row))
    print(
        f"version={version} columns={len(columns)} rows={len(rows)}",
        file=sys.stderr,
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
