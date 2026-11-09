#!/usr/bin/env python3
"""Package the Release x64 client as an add-on for an SWG Source 3.0 client folder.

The add-on unzips into the player's existing client folder:

    SwgClient_x64.bat           launcher (working directory = client folder)
    x64/SwgClient_r.exe         Release x64 game
    x64/gl05_r.dll gl06_r.dll gl07_r.dll dpvs.dll libxml2.dll
    x64/x64.cfg                 selects the media helper (loaded after client.cfg)
    x64/setup.bat               copies the player's own Mss32/Bink/miles beside the helper
    x64/README-x64.txt          player guide
    x64/MANIFEST.sha256         hashes of every packaged file
    x64/helper-x86/miles-host.exe

No vendor or Microsoft redistributable is packaged: the helper uses the
player's own Miles and Bink files, copied by setup.bat, and the renderers use
the DirectX End-User Runtime.

Usage:
    python package.py --game-dir <dir> --host <miles-host.exe> --out <zip> [--label <text>]

--game-dir holds the Release x64 build outputs named above. --host is the
Release x86 miles-host.exe built by tools/miles-bridge/build.py --target host
--configuration Release.
"""
import argparse
import hashlib
import os
import sys
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
TEMPLATES = os.path.join(HERE, 'templates')

GAME_FILES = ['SwgClient_r.exe', 'gl05_r.dll', 'gl06_r.dll', 'gl07_r.dll', 'dpvs.dll', 'libxml2.dll']
# DllExport.dll is a delay-load pseudo-module resolved to the game executable;
# a separate DllExport.dll must never be shipped beside the x64 client.
FORBIDDEN = ['DllExport.dll', 'Mss32.dll', 'binkw32.dll']
TEXT_FILES = {
    'SwgClient_x64.bat': 'SwgClient_x64.bat',
    'setup.bat': 'x64/setup.bat',
    'x64.cfg': 'x64/x64.cfg',
    'README-x64.txt': 'x64/README-x64.txt',
}
MACHINE_X64 = 0x8664
MACHINE_X86 = 0x014C
FIXED_TIME = (2026, 1, 1, 0, 0, 0)


def pe_machine(path):
    with open(path, 'rb') as f:
        head = f.read(4096)
    if head[:2] != b'MZ':
        raise SystemExit('%s is not a Windows executable' % path)
    pe = int.from_bytes(head[0x3C:0x40], 'little')
    if head[pe:pe + 4] != b'PE\0\0':
        raise SystemExit('%s has no PE header' % path)
    return int.from_bytes(head[pe + 4:pe + 6], 'little')


def crlf(text):
    return text.replace('\r\n', '\n').replace('\n', '\r\n').encode('ascii')


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('--game-dir', required=True)
    p.add_argument('--host', required=True)
    p.add_argument('--out', required=True)
    p.add_argument('--label', default='', help='version text written to x64/VERSION.txt')
    a = p.parse_args()

    entries = []  # (archive path, bytes)
    for name in GAME_FILES:
        src = os.path.join(a.game_dir, name)
        if not os.path.isfile(src):
            raise SystemExit('missing build output: %s' % src)
        if pe_machine(src) != MACHINE_X64:
            raise SystemExit('%s is not x64' % src)
        with open(src, 'rb') as f:
            entries.append(('x64/' + name, f.read()))
    for name in FORBIDDEN:
        if os.path.exists(os.path.join(a.game_dir, name)):
            print('note: ignoring %s in --game-dir; it is not shipped' % name)

    if pe_machine(a.host) != MACHINE_X86:
        raise SystemExit('%s is not an x86 executable' % a.host)
    with open(a.host, 'rb') as f:
        entries.append(('x64/helper-x86/miles-host.exe', f.read()))

    for template, target in TEXT_FILES.items():
        with open(os.path.join(TEMPLATES, template), encoding='ascii') as f:
            entries.append((target, crlf(f.read())))
    if a.label:
        entries.append(('x64/VERSION.txt', crlf(a.label + '\n')))

    manifest = ''.join('%s *%s\n' % (hashlib.sha256(data).hexdigest(), path)
                       for path, data in sorted(entries))
    entries.append(('x64/MANIFEST.sha256', crlf(manifest)))

    os.makedirs(os.path.dirname(os.path.abspath(a.out)), exist_ok=True)
    tmp = a.out + '.tmp'
    with zipfile.ZipFile(tmp, 'w', zipfile.ZIP_DEFLATED) as z:
        for path, data in sorted(entries):
            info = zipfile.ZipInfo(path, FIXED_TIME)
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = 0o644 << 16
            z.writestr(info, data)
    os.replace(tmp, a.out)
    print(manifest, end='')
    print('wrote %s' % a.out)


if __name__ == '__main__':
    sys.exit(main())
