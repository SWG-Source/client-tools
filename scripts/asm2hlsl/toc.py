"""
Reader for the SWG ".toc" (Table Of Contents) archive-index format.

Specification taken directly from the engine source:
  src/engine/shared/library/sharedFile/src/shared/TreeFile_SearchNode.h
      TreeFile::SearchTOC::Header
      TreeFile::SearchTOC::TableOfContentsEntry
  src/engine/shared/library/sharedFile/src/shared/TreeFile_SearchNode.cpp
      SearchTOC::validate / SearchTOC::SearchTOC / localExists / getPathName / open

KEY FACT: a .toc carries NO file payload.  Each entry's (offset, length,
compressedLength) is a byte range inside m_treeFiles[treeFileIndex] -- i.e.
inside one of the .tre archives named in the tree-file-name block.  SearchTOC::open
reads from m_treeFiles[entry.treeFileIndex]; it never reads payload from m_TOCFile.
The .toc is purely an index: name -> (which .tre, where in it, how big, how compressed).

ON-DISK LAYOUT
  offset  size  field
  ------  ----  ----------------------------------------------------------------
   0       4    token                    little-endian uint32 Tag; the bytes on
                                         disk read " COT" (== 'TOC ' reversed)
                                         because Tags are 32-bit ints
   4       4    version                  disk bytes "1000" == Tag '0001'
   8       1    tocCompressor            CT_none=0, CT_deprecated=1, CT_zlib=2
   9       1    fileNameBlockCompressor  ditto
  10       1    unusedOne
  11       1    unusedTwo
  12       4    numberOfFiles
  16       4    sizeOfTOC                on-disk size of the entry block
  20       4    sizeOfNameBlock          on-disk size of the file-name block
  24       4    uncompSizeOfNameBlock
  28       4    numberOfTreeFiles
  32       4    sizeOfTreeFileNameBlock
  ---- header is exactly 36 bytes (isizeof(Header)); no padding ----
  36    sizeOfTreeFileNameBlock   tree-file-name block: numberOfTreeFiles
                                  NUL-terminated ASCII names back to back.
                                  ALWAYS UNCOMPRESSED -- no compressor field
                                  exists for it and the ctor reads it raw.
                                  Position in this block == treeFileIndex.
  ...   sizeOfTOC                 entry block; numberOfFiles * 24 bytes once
                                  expanded; zlib-deflated iff tocCompressor != 0
  ...   sizeOfNameBlock           file-name block; uncompSizeOfNameBlock bytes
                                  once expanded; zlib iff
                                  fileNameBlockCompressor != 0

TableOfContentsEntry, 24 bytes, little-endian:
    0   1  compressor         0 = stored, 2 = zlib
    1   1  unused             (present only to force 32-bit alignment)
    2   2  treeFileIndex      index into the tree-file-name block
    4   4  crc                Crc::calculate(fileName) -- case SENSITIVE
    8   4  fileNameOffset     *** ON DISK THIS FIELD HOLDS THE NAME LENGTH ***
   12   4  offset             byte offset INSIDE the .tre
   16   4  length             uncompressed size
   20   4  compressedLength   on-disk size inside the .tre

The ctor rewrites that 8th field in place after loading the block:
    currentFileNameLength = m_tableOfContents[i].fileNameOffset;
    m_tableOfContents[i].fileNameOffset = currentFileNameOffset;
    currentFileNameOffset += (currentFileNameLength + 1);
so names sit in the name block in exactly entry order, each followed by a NUL.
Entries are sorted ascending by (crc, name): localExists binary-searches on crc
and breaks ties with _stricmp on the name.

localExists() treats an entry as NO MATCH when length == 0 or offset == 0.
SearchTOC::exists always reports deleted = false, so such an entry does not halt
the search -- it merely fails to match, and lower-priority nodes still get a look.
"""

import os
import struct
import sys
import zlib

HEADER_SIZE = 36
ENTRY_SIZE = 24
ENTRY_FMT = '<BBHIIIII'

TOKEN_ON_DISK = b' COT'          # Tag 'TOC '
VERSION_0001_ON_DISK = b'1000'   # Tag '0001'
# TreeFile_SearchNode.cpp also accepts the raw uint32 0x30303032 through the same
# code path: disk bytes "2000", i.e. Tag '0002' (a Restoration-era variant).
VERSION_0002_ON_DISK = b'2000'

CT_none = 0
CT_deprecated = 1
CT_zlib = 2


def is_compressed(compressor):
    """TreeFile::SearchTOC::isCompressed -- anything but CT_none."""
    return compressor != CT_none


def _tag(disk_bytes):
    """Render 4 on-disk bytes as the Tag string the engine sees."""
    return disk_bytes[::-1].decode('latin-1')


def _build_crc_table():
    table = []
    for i in range(256):
        c = (i << 24) & 0xFFFFFFFF
        for _ in range(8):
            if c & 0x80000000:
                c = ((c << 1) ^ 0x04C11DB7) & 0xFFFFFFFF
            else:
                c = (c << 1) & 0xFFFFFFFF
        table.append(c)
    return table


_CRCTABLE = _build_crc_table()


def crc(name):
    """Crc::calculate from sharedFoundation/Crc.cpp: poly 0x04C11DB7, MSB-first,
    init 0xFFFFFFFF, final xor 0xFFFFFFFF.  Case sensitive -- the engine does not
    lower-case before hashing."""
    c = 0xFFFFFFFF
    for ch in name.encode('latin-1'):
        c = (_CRCTABLE[((c >> 24) ^ ch) & 0xFF] ^ ((c << 8) & 0xFFFFFFFF)) & 0xFFFFFFFF
    return c ^ 0xFFFFFFFF


class TocHeader(object):
    __slots__ = ('token', 'version', 'tocCompressor', 'fileNameBlockCompressor',
                 'unusedOne', 'unusedTwo', 'numberOfFiles', 'sizeOfTOC',
                 'sizeOfNameBlock', 'uncompSizeOfNameBlock', 'numberOfTreeFiles',
                 'sizeOfTreeFileNameBlock')

    def __repr__(self):
        return ('TocHeader(token=%r version=%r tocCompressor=%d '
                'fileNameBlockCompressor=%d unused=(%d,%d) numberOfFiles=%d '
                'sizeOfTOC=%d sizeOfNameBlock=%d uncompSizeOfNameBlock=%d '
                'numberOfTreeFiles=%d sizeOfTreeFileNameBlock=%d)' % (
                    self.token, self.version, self.tocCompressor,
                    self.fileNameBlockCompressor, self.unusedOne, self.unusedTwo,
                    self.numberOfFiles, self.sizeOfTOC, self.sizeOfNameBlock,
                    self.uncompSizeOfNameBlock, self.numberOfTreeFiles,
                    self.sizeOfTreeFileNameBlock))


class TocEntry(object):
    __slots__ = ('name', 'treeFileIndex', 'crc', 'offset', 'length',
                 'compressor', 'compressedLength', 'nameOffset')

    def __init__(self, name, treeFileIndex, crc_, offset, length, compressor,
                 compressedLength, nameOffset):
        self.name = name
        self.treeFileIndex = treeFileIndex
        self.crc = crc_
        self.offset = offset
        self.length = length
        self.compressor = compressor
        self.compressedLength = compressedLength
        self.nameOffset = nameOffset

    @property
    def usable(self):
        """localExists(): length==0 or offset==0 means the lookup fails."""
        return self.length != 0 and self.offset != 0

    def __repr__(self):
        return ('TocEntry(%r tree=%d off=%d len=%d comp=%d clen=%d)'
                % (self.name, self.treeFileIndex, self.offset, self.length,
                   self.compressor, self.compressedLength))


def read_toc(path, want_entries=True):
    """Parse a .toc.  Returns (header, treeFileNames, entries).

    treeFileNames is the ordered list of .tre names; a name's index in this list
    is the treeFileIndex the entries refer to.  entries is a list of TocEntry in
    on-disk order (== crc ascending), or [] when want_entries is False.
    """
    with open(path, 'rb') as f:
        raw = f.read(HEADER_SIZE)
        if len(raw) != HEADER_SIZE:
            raise ValueError('%s: short header' % path)

        h = TocHeader()
        h.token = _tag(raw[0:4])
        h.version = _tag(raw[4:8])
        (h.tocCompressor, h.fileNameBlockCompressor, h.unusedOne,
         h.unusedTwo) = struct.unpack_from('<4B', raw, 8)
        (h.numberOfFiles, h.sizeOfTOC, h.sizeOfNameBlock,
         h.uncompSizeOfNameBlock, h.numberOfTreeFiles,
         h.sizeOfTreeFileNameBlock) = struct.unpack_from('<6I', raw, 12)

        if raw[0:4] != TOKEN_ON_DISK:
            raise ValueError('%s: not a TOC file (token %r)' % (path, h.token))
        if raw[4:8] not in (VERSION_0001_ON_DISK, VERSION_0002_ON_DISK):
            raise ValueError('%s: unsupported version %r' % (path, h.version))

        # --- tree file name block: always uncompressed, immediately after header
        blk = f.read(h.sizeOfTreeFileNameBlock)
        if len(blk) != h.sizeOfTreeFileNameBlock:
            raise ValueError('%s: short tree name block' % path)
        treeFileNames = []
        pos = 0
        for i in range(h.numberOfTreeFiles):
            end = blk.find(b'\0', pos)
            if end < 0:
                raise ValueError('%s: tree name %d unterminated' % (path, i))
            treeFileNames.append(blk[pos:end].decode('latin-1'))
            pos = end + 1
        if pos != h.sizeOfTreeFileNameBlock:
            raise ValueError('%s: tree name block has %d trailing bytes %r'
                             % (path, h.sizeOfTreeFileNameBlock - pos, blk[pos:]))

        if not want_entries:
            return h, treeFileNames, []

        # --- table-of-contents entry block
        expanded = h.numberOfFiles * ENTRY_SIZE
        if is_compressed(h.tocCompressor):
            buf = zlib.decompress(f.read(h.sizeOfTOC))
        else:
            buf = f.read(expanded)
        if len(buf) != expanded:
            raise ValueError('%s: entry block is %d bytes, expected %d'
                             % (path, len(buf), expanded))

        # --- file name block
        if is_compressed(h.fileNameBlockCompressor):
            names = zlib.decompress(f.read(h.sizeOfNameBlock))
        else:
            names = f.read(h.sizeOfNameBlock)
        if len(names) != h.uncompSizeOfNameBlock:
            raise ValueError('%s: name block is %d bytes, expected %d'
                             % (path, len(names), h.uncompSizeOfNameBlock))
        tail = f.tell()

    entries = []
    nameOffset = 0
    for i in range(h.numberOfFiles):
        (compressor, _unused, treeFileIndex, crc_, nameLength, offset, length,
         compressedLength) = struct.unpack_from(ENTRY_FMT, buf, i * ENTRY_SIZE)
        name = names[nameOffset:nameOffset + nameLength].decode('latin-1')
        if names[nameOffset + nameLength] != 0:
            raise ValueError('%s: entry %d name not NUL terminated' % (path, i))
        if treeFileIndex >= len(treeFileNames):
            raise ValueError('%s: entry %d treeFileIndex %d out of range'
                             % (path, i, treeFileIndex))
        entries.append(TocEntry(name, treeFileIndex, crc_, offset, length,
                                compressor, compressedLength, nameOffset))
        nameOffset += nameLength + 1

    if nameOffset != h.uncompSizeOfNameBlock:
        raise ValueError('%s: consumed %d of %d name bytes'
                         % (path, nameOffset, h.uncompSizeOfNameBlock))
    h_bytes_used = tail
    if h_bytes_used != os.path.getsize(path):
        # not fatal, but worth knowing about
        sys.stderr.write('NOTE %s: %d bytes parsed, file is %d bytes\n'
                         % (path, h_bytes_used, os.path.getsize(path)))
    return h, treeFileNames, entries


def tre_list(path):
    """The ordered .tre archive list this searchTOC contributes."""
    _h, treeFileNames, _e = read_toc(path, want_entries=False)
    return treeFileNames


def open_entry(root, treeFileNames, entry):
    """Return the bytes for a TocEntry.  Mirrors SearchTOC::open exactly: the
    payload is read out of the .tre named by entry.treeFileIndex, at the offset
    the TOC gives, NOT via the .tre's own internal directory.

    This matters: 137 of the 209 .tre files in a live client are version 0006
    and carry a fully zeroed header (numberOfFiles == 0, tocOffset == 0), so they
    have no internal directory at all.  Their contents are reachable ONLY through
    a .toc.  `root` is the directory holding both the .toc and the .tre files.
    """
    path = os.path.join(root, treeFileNames[entry.treeFileIndex])
    with open(path, 'rb') as f:
        f.seek(entry.offset)
        if is_compressed(entry.compressor):
            data = zlib.decompress(f.read(entry.compressedLength))
        else:
            data = f.read(entry.length)
    if len(data) != entry.length:
        raise ValueError('%s[%s]: got %d bytes, entry says %d'
                         % (path, entry.name, len(data), entry.length))
    return data


def lookup(entries, fileName):
    """Reproduce localExists(): binary search on crc, tie-break with _stricmp.
    Returns the TocEntry, or None if absent / marked unusable."""
    target = crc(fileName)
    low = fileName.lower()
    lo, hi = 0, len(entries) - 1
    while lo <= hi:
        mid = (lo + hi) // 2
        e = entries[mid]
        if e.crc < target:
            lo = mid + 1
        elif e.crc > target:
            hi = mid - 1
        else:
            m = e.name.lower()
            if m < low:
                lo = mid + 1
            elif m > low:
                hi = mid - 1
            else:
                return e if e.usable else None
    return None


def _main(argv):
    if len(argv) < 3:
        sys.stdout.write(__doc__)
        print('usage: toc.py header <file.toc>...')
        print('       toc.py trees  <file.toc>...')
        print('       toc.py names  <file.toc>            tab-separated dump')
        print('       toc.py find   <file.toc> <innerpath>...')
        print('       toc.py cat    <file.toc> <innerpath> <outfile>')
        return 2
    mode = argv[1]
    if mode == 'header':
        for p in argv[2:]:
            h, trees, entries = read_toc(p)
            print('=== %s (%d bytes on disk)' % (p, os.path.getsize(p)))
            print('  ' + repr(h))
            print('  treeFiles=%d  entries=%d  unusable(len==0 or off==0)=%d'
                  % (len(trees), len(entries),
                     sum(1 for e in entries if not e.usable)))
            print('  entries sorted by crc ascending: %s'
                  % all(entries[i].crc <= entries[i + 1].crc
                        for i in range(len(entries) - 1)))
            used = sorted(set(e.treeFileIndex for e in entries))
            comp = sorted(set(e.compressor for e in entries))
            print('  distinct treeFileIndex referenced: %d of %d (min %s max %s)'
                  % (len(used), len(trees), used[0] if used else '-',
                     used[-1] if used else '-'))
            print('  entry compressor values seen: %s' % comp)
    elif mode == 'trees':
        for p in argv[2:]:
            print('=== %s' % p)
            for i, n in enumerate(tre_list(p)):
                print('  %3d  %s' % (i, n))
    elif mode == 'names':
        _h, trees, entries = read_toc(argv[2])
        out = []
        for e in entries:
            out.append('%s\t%s\t%d\t%d\t%d\t%d\t%08x'
                       % (trees[e.treeFileIndex], e.name, e.length, e.compressor,
                          e.compressedLength, e.offset, e.crc))
        sys.stdout.write('\n'.join(out) + '\n')
    elif mode == 'find':
        _h, trees, entries = read_toc(argv[2])
        for want in argv[3:]:
            e = lookup(entries, want)
            if e is None:
                print('%s -> NOT FOUND (absent, or entry marked unusable)' % want)
            else:
                print('%s -> %s  off=%d len=%d comp=%d clen=%d crc=%08x'
                      % (e.name, trees[e.treeFileIndex], e.offset, e.length,
                         e.compressor, e.compressedLength, e.crc))
    elif mode == 'cat':
        p = argv[2]
        _h, trees, entries = read_toc(p)
        e = lookup(entries, argv[3])
        if e is None:
            print('%s -> NOT FOUND' % argv[3])
            return 1
        data = open_entry(os.path.dirname(os.path.abspath(p)), trees, e)
        with open(argv[4], 'wb') as out:
            out.write(data)
        print('%s from %s: %d bytes -> %s'
              % (e.name, trees[e.treeFileIndex], len(data), argv[4]))
    else:
        print('unknown mode %r' % mode)
        return 2
    return 0


if __name__ == '__main__':
    sys.exit(_main(sys.argv))
