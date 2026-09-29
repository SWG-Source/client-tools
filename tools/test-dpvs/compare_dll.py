"""Compare Win32 DLLs, normalizing only enumerated build metadata."""
from pathlib import Path
import argparse
import hashlib
import json
import re
import struct


def normalize(data):
    b = bytearray(data)
    def bounded(offset, size):
        if offset < 0 or size < 0 or offset + size > len(b):
            raise ValueError('PE span outside file')
        return offset
    bounded(0, 64)
    if b[:2] != b'MZ':
        raise ValueError('Expected DOS header')
    pe = struct.unpack_from('<I', b, 60)[0]
    bounded(pe, 24)
    op = pe + 24
    optional_size = struct.unpack_from('<H', b, pe+20)[0]
    bounded(op, optional_size)
    if b[pe:pe+4] != b'PE\0\0' or optional_size < 152 or struct.unpack_from('<H', b, op)[0] != 0x10b:
        raise ValueError('Expected PE32 DLL')
    dd = op + 96
    if struct.unpack_from('<I', b, op+92)[0] < 7:
        raise ValueError('Missing PE data directories')
    sh = op + optional_size
    count = struct.unpack_from('<H', b, pe+6)[0]
    bounded(sh, count*40)
    sections, hashes = [], {}
    for i in range(count):
        o = sh + 40*i
        name = b[o:o+8].rstrip(b'\0').decode()
        vs, va, rs, rp = struct.unpack_from('<IIII', b, o+8)
        flags = struct.unpack_from('<I', b, o+36)[0]
        bounded(rp, rs)
        if name in hashes or (rs and rp < sh+count*40):
            raise ValueError('Invalid/duplicate section')
        for old in sections:
            if rs and old[2] and max(rp, old[3]) < min(rp+rs, old[3]+old[2]):
                raise ValueError('Overlapping raw sections')
            if max(va, old[1]) < min(va+max(vs,rs), old[1]+old[5]):
                raise ValueError('Overlapping virtual sections')
        sections.append((name, va, rs, rp, flags, max(vs,rs)))
        hashes[name] = hashlib.sha256(b[rp:rp+rs]).hexdigest()
    def readonly_span(offset, size):
        bounded(offset, size)
        found = [s for s in sections if s[3] <= offset and offset+size <= s[3]+s[2]]
        if len(found) != 1 or found[0][0] != '.rdata' or found[0][4] & (0x20000000 | 0x80000000):
            raise ValueError('Metadata outside nonexecuting read-only .rdata')
        return offset
    def pos(rva, size):
        found = [s for s in sections if s[1] <= rva and rva+size <= s[1]+s[2]]
        if len(found) != 1:
            raise ValueError('RVA is not fully backed by raw bytes')
        return readonly_span(found[0][3]+rva-found[0][1], size)
    fields = [(pe+8, 4, 'COFF timestamp')]
    exp, export_size = struct.unpack_from('<II', b, dd)
    if exp:
        if export_size < 40:
            raise ValueError('Truncated export directory')
        fields.append((pos(exp, export_size)+4, 4, 'export timestamp'))
    rv, size = struct.unpack_from('<II', b, dd+6*8)
    if not rv or not size or size % 28:
        raise ValueError('Invalid debug directory')
    debug = pos(rv, size)
    for i in range(size//28):
        o = debug+28*i
        fields.append((o+4, 4, 'debug timestamp'))
        typ, payload_size, payload_rva, ptr = struct.unpack_from('<IIII', b, o+12)
        readonly_span(ptr, payload_size)
        if pos(payload_rva, payload_size) != ptr:
            raise ValueError('Debug RVA/file pointer disagree')
        if typ == 2:
            if payload_size < 25 or b[ptr:ptr+4] != b'RSDS':
                raise ValueError('Invalid CodeView record')
            fields.append((ptr+4, 20, 'PDB GUID and age'))
            end = b.index(0, ptr+24, ptr+payload_size)
            path = bytes(b[ptr+24:end])
            changed = path.replace(b'\\Win32\\', b'\\win32\\')
            if path != changed:
                fields.append((ptr+24, len(path), 'PDB Win32 path capitalization (case only)'))
            b[ptr+24:end] = changed
    pattern = rb'(?:Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) [ 0-9][0-9] [0-9]{4} [0-9]{2}:[0-9]{2}:[0-9]{2}\x00'
    dates = list(re.finditer(pattern, b))
    if len(dates) != 1:
        raise ValueError('Expected exactly one DPVS_BUILD_TIME string')
    readonly_span(dates[0].start(), len(dates[0].group()))
    fields.append((dates[0].start(), len(dates[0].group())-1, 'DPVS_BUILD_TIME'))
    for offset, length, label in fields:
        bounded(offset, length)
        if 'capitalization' not in label:
            b[offset:offset+length] = b'\0'*length
    return b, fields, hashes


def compare(base, candidate):
    a, af, ah = normalize(Path(base).read_bytes())
    b, bf, bh = normalize(Path(candidate).read_bytes())
    offsets = [i for i, (x, y) in enumerate(zip(a, b)) if x != y]
    return {'normalized_equal': a == b, 'size_base': len(a), 'size_candidate': len(b),
            'differing_bytes': len(offsets)+abs(len(a)-len(b)), 'first_differences': offsets[:30],
            'base_normalized_fields': af, 'candidate_normalized_fields': bf,
            'raw_sections_equal': {s: ah[s] == bh.get(s) for s in ah}}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('base', type=Path)
    parser.add_argument('candidate', type=Path)
    args = parser.parse_args()
    result = compare(args.base, args.candidate)
    print(json.dumps(result, indent=2))
    raise SystemExit(0 if result['normalized_equal'] else 1)
