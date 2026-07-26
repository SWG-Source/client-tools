import os, struct, sys, zlib, glob, json

def read_tre(path):
    f = open(path, 'rb')
    hdr = f.read(36)
    tok, ver = hdr[0:4], hdr[4:8]
    if tok != b'EERT':
        f.close(); return None
    (nfiles, tocOffset, tocComp, sizeOfTOC, blockComp, sizeOfNameBlock,
     uncompNameBlock) = struct.unpack_from('<7I', hdr, 8)
    verstr = ver[::-1].decode('ascii', 'replace')
    entrySize = 32 if ver == b'6000' else 24
    f.seek(tocOffset)
    if tocComp != 0:
        raw = f.read(sizeOfTOC)
        buf = zlib.decompress(raw)
        pos = tocOffset + sizeOfTOC
    else:
        buf = f.read(entrySize * nfiles)
        pos = tocOffset + len(buf)
    f.seek(pos)
    if blockComp != 0:
        nb = zlib.decompress(f.read(sizeOfNameBlock))
    else:
        nb = f.read(uncompNameBlock)
    entries = []
    for i in range(nfiles):
        s = struct.unpack_from('<8I' if entrySize == 32 else '<6I', buf, i*entrySize)
        if entrySize == 32:
            crc, length, offset, _u1, _u2, nameOff, comp, clen = s
        else:
            crc, length, offset, comp, clen, nameOff = s
        end = nb.index(b'\0', nameOff)
        name = nb[nameOff:end].decode('latin-1')
        entries.append((name, length, offset, comp, clen))
    return f, verstr, entries

def extract(f, ent, out):
    name, length, offset, comp, clen = ent
    f.seek(offset)
    if comp == 0:
        data = f.read(length)
    else:
        data = zlib.decompress(f.read(clen))
    return data

if __name__ == '__main__':
    mode = sys.argv[1]
    root = sys.argv[2]
    if mode == 'list':
        out = []
        for p in sorted(glob.glob(os.path.join(root, '*.tre'))):
            r = read_tre(p)
            if r is None:
                print('SKIP', p, file=sys.stderr); continue
            f, ver, entries = r
            base = os.path.basename(p)
            print(f'#TRE {base} ver={ver} files={len(entries)}', file=sys.stderr)
            for name, length, offset, comp, clen in entries:
                out.append(f'{base}\t{name}\t{length}\t{comp}\t{clen}')
            f.close()
        sys.stdout.write('\n'.join(out))
    elif mode == 'get':
        # tre.py get <root> <innerpath> <outfile>
        inner = sys.argv[3].lower().replace('\\','/')
        outfile = sys.argv[4]
        found = False
        for p in sorted(glob.glob(os.path.join(root, '*.tre')), reverse=True):
            r = read_tre(p)
            if r is None: continue
            f, ver, entries = r
            for e in entries:
                if e[0].lower().replace('\\','/') == inner:
                    data = extract(f, e, None)
                    open(outfile,'wb').write(data)
                    print(f'from {os.path.basename(p)}: {len(data)} bytes')
                    found = True
                    break
            f.close()
            if found: break
        if not found:
            print('NOT FOUND', inner); sys.exit(1)
