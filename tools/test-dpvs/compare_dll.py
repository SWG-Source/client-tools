"""Compare Win32 DLLs, normalizing only enumerated build metadata."""
from pathlib import Path
import argparse
import hashlib
import json
import re
import struct


def normalize(data):
    b = bytearray(data)
    pe = struct.unpack_from('<I', b, 60)[0]
    if b[pe:pe+4] != b'PE\0\0' or struct.unpack_from('<H', b, pe+24)[0] != 0x10b:
        raise ValueError('Expected PE32 DLL')
    op = pe + 24
    dd = op + 96
    sh = op + struct.unpack_from('<H', b, pe+20)[0]
    sections, hashes = [], {}
    for i in range(struct.unpack_from('<H', b, pe+6)[0]):
        o = sh + 40*i
        name = b[o:o+8].rstrip(b'\0').decode()
        vs, va, rs, rp = struct.unpack_from('<IIII', b, o+8)
        sections.append((va, max(vs, rs), rp))
        hashes[name] = hashlib.sha256(b[rp:rp+rs]).hexdigest()
    def pos(rva):
        for va, size, rp in sections:
            if va <= rva < va+size:
                return rp+rva-va
        raise ValueError('Invalid RVA')
    fields = [(pe+8, 4, 'COFF timestamp')]
    exp = struct.unpack_from('<I', b, dd)[0]
    if exp:
        fields.append((pos(exp)+4, 4, 'export timestamp'))
    rv, size = struct.unpack_from('<II', b, dd+6*8)
    for i in range(size//28):
        o = pos(rv)+28*i
        fields.append((o+4, 4, 'debug timestamp'))
        typ, ptr = struct.unpack_from('<I', b, o+12)[0], struct.unpack_from('<I', b, o+24)[0]
        if typ == 2 and b[ptr:ptr+4] == b'RSDS':
            fields.append((ptr+4, 20, 'PDB GUID and age'))
            end = b.index(0, ptr+24)
            path = bytes(b[ptr+24:end])
            changed = path.replace(b'\\Win32\\', b'\\win32\\')
            if path != changed:
                fields.append((ptr+24, len(path), 'PDB Win32 path capitalization (case only)'))
            b[ptr+24:end] = changed
    pattern = rb'(?:Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) [ 0-9][0-9] [0-9]{4} [0-9]{2}:[0-9]{2}:[0-9]{2}\x00'
    dates = list(re.finditer(pattern, b))
    if len(dates) != 1:
        raise ValueError('Expected exactly one DPVS_BUILD_TIME string')
    fields.append((dates[0].start(), len(dates[0].group())-1, 'DPVS_BUILD_TIME'))
    for offset, length, label in fields:
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
