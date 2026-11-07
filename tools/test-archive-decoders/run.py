#!/usr/bin/env python3
"""Build this checkout's real Archive/Unicode decoders and Windows mutex; run bounded fixtures."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
EXPECTED = 'PASS: 34 bounded decoder checks'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--bits', type=int, choices=(32, 64), required=True)
    parser.add_argument('--wine-arch', choices=('win32', 'win64', 'wow64'))
    parser.add_argument('--artifacts', type=Path, required=True)
    args = parser.parse_args()
    args.artifacts.mkdir(parents=True, exist_ok=True)
    archive = ROOT / 'src/external/ours/library/archive'
    sources = [HERE / 'fixtures.cpp', archive / 'src/shared/ByteStream.cpp',
               archive / 'src/win32/ArchiveMutex.cpp',
               ROOT / 'src/external/ours/library/unicodeArchive/src/shared/UnicodeArchive.cpp']
    headers = sorted((archive / 'src/shared').glob('*.h')) + [
        archive / 'src/win32/ArchiveMutex.h', archive / 'include/Archive/ArchiveMutex.h']
    headers += [ROOT / 'src/external/ours/library/unicode/src/shared/Unicode.h',
                ROOT / 'src/external/ours/library/unicodeArchive/src/shared/UnicodeArchive.h',
                ROOT / 'src/external/ours/library/unicodeArchive/src/shared/FirstUnicodeArchive.h']
    compiler = ('i686' if args.bits == 32 else 'x86_64') + '-w64-mingw32-g++'
    manifest = {
        'bits': args.bits,
        'sources': {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
                    for p in sources + headers},
        'compiler': subprocess.check_output([compiler, '--version'], text=True),
        'wine': subprocess.check_output(['wine', '--version'], text=True),
    }
    manifest_path = args.artifacts / 'manifest.json'
    manifest_path.write_text(json.dumps(manifest, indent=2))
    with tempfile.TemporaryDirectory(prefix='swg-archive-decoders-') as tmp:
        exe = args.artifacts.resolve() / ('archive-decoders-' + str(args.bits) + '.exe')
        # A failed rebuild must never run a stale executable.
        exe.unlink(missing_ok=True)
        command = [compiler, '-std=c++11', '-DWIN32=1', '-include', 'cstring',
                   '-I' + str(archive / 'src/shared'), '-I' + str(archive / 'include'),
                   '-I' + str(ROOT / 'src/external/ours/library/unicodeArchive/include/public'),
                   '-I' + str(ROOT / 'src/external/ours/library/unicode/include'),
                   '-static', *map(str, sources), '-o', str(exe)]
        manifest['command'] = command
        manifest_path.write_text(json.dumps(manifest, indent=2))
        result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                text=True, timeout=120)
        (args.artifacts / 'build.log').write_text(result.stdout)
        if result.returncode:
            print(result.stdout)
            return result.returncode
        data = exe.read_bytes()
        pe = struct.unpack_from('<I', data, 0x3c)[0]
        machine = struct.unpack_from('<H', data, pe + 4)[0]
        manifest['machine'] = machine
        manifest['executable_sha256'] = hashlib.sha256(data).hexdigest()
        manifest_path.write_text(json.dumps(manifest, indent=2))
        if machine != (0x14c if args.bits == 32 else 0x8664):
            raise RuntimeError('wrong executable architecture')
        env = dict(os.environ, WINEDEBUG='-all',
                   WINEARCH=args.wine_arch or ('win32' if args.bits == 32 else 'win64'),
                   WINEPREFIX=os.environ.get('WINEPREFIX' + str(args.bits), str(Path(tmp) / 'wine')))
        result = subprocess.run(['wine', str(exe)], env=env, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE, text=True, timeout=120)
        (args.artifacts / 'run.log').write_text(result.stdout)
        (args.artifacts / 'wine.log').write_text(result.stderr)
        print(result.stdout, end='')
        # No skipped cases, unexpected output, duplicate verdicts, or failing exit accepted.
        passed = result.returncode == 0 and result.stdout.splitlines() == [EXPECTED]
        manifest['exit'] = result.returncode
        manifest['passed'] = passed
        manifest_path.write_text(json.dumps(manifest, indent=2))
        return 0 if passed else 1


if __name__ == '__main__':
    raise SystemExit(main())
