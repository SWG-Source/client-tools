#!/usr/bin/env python3
"""Build the original client JPEG6b and bundled STLport with native VS2013."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys
import tarfile
import tempfile
import time
import urllib.request

JPEG_URL = 'https://www.ijg.org/files/jpegsrc.v6b.tar.gz'
JPEG_SHA256 = '75c3ec241e9996504fe02a9ed4d12f16b74ade713972f3db9e65ce95cd27e35d'
ROOT = Path(__file__).resolve().parents[2]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def ensure_owner(output, owner):
    """Validate or establish ownership while the caller holds .build-lock."""
    owner_file = output / 'owner.json'
    try:
        saved = json.loads(owner_file.read_text())
    except FileNotFoundError:
        if any(path.name != '.build-lock' for path in output.iterdir()):
            raise RuntimeError('Output contains unowned files; choose a new empty output directory; existing contents were preserved')
    else:
        if saved != owner:
            raise RuntimeError('Output belongs to another checkout/platform/configuration; choose a private output directory')
        return
    # A failed write/publication leaves unowned staging data and blocks adoption.
    pending = output / 'owner.pending'
    with pending.open('x') as stream:
        stream.write(json.dumps(owner, indent=2))
        stream.flush()
        os.fsync(stream.fileno())
    os.replace(pending, owner_file)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--vcvars', required=True, type=Path)
    parser.add_argument('--platform', choices=['Win32', 'x64'], required=True)
    parser.add_argument('--configuration', choices=['Debug', 'Release'], required=True)
    parser.add_argument('--jpeg-archive', required=True, type=Path)
    parser.add_argument('--download', action='store_true', help='explicitly download the pinned official archive if absent')
    args = parser.parse_args()
    if os.name != 'nt':
        raise RuntimeError('Native Windows VS2013 is required; this is not a cross compiler')
    archive = args.jpeg_archive.resolve()
    if not archive.exists() and args.download:
        archive.parent.mkdir(parents=True, exist_ok=True)
        with urllib.request.urlopen(JPEG_URL, timeout=60) as response:
            content = response.read()
        if hashlib.sha256(content).hexdigest() != JPEG_SHA256:
            raise RuntimeError('Downloaded JPEG archive does not match pinned SHA256')
        archive.write_bytes(content)
    if not archive.is_file():
        raise RuntimeError('Missing offline JPEG archive: %s; obtain %s or explicitly use --download' % (archive, JPEG_URL))
    if digest(archive) != JPEG_SHA256:
        raise RuntimeError('JPEG archive SHA256 mismatch; refusing to extract or compile')
    if not args.vcvars.is_file():
        raise RuntimeError('VS2013 vcvarsall.bat is missing: ' + str(args.vcvars))
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    lock = output / '.build-lock'
    deadline = time.monotonic() + 600
    while True:
        try:
            lock.mkdir()
            break
        except FileExistsError:
            if time.monotonic() >= deadline:
                raise RuntimeError('Dependency lock timed out: %s; inspect the owning build before removal' % lock)
            time.sleep(0.5)
    try:
        owner = dict(checkout=str(ROOT.resolve()), platform=args.platform, configuration=args.configuration)
        ensure_owner(output, owner)
        build(args, archive, output)
    finally:
        lock.rmdir()


def build(args, archive, output):
    arch = 'amd64' if args.platform == 'x64' else 'x86'
    stl = ROOT / 'src/external/3rd/library/stlport453'
    headers = ROOT / 'src/external/3rd/library/libjpeg/include'
    # Header-only changes must invalidate archives too. Exclude prebuilt artifacts.
    inputs = {str(p.relative_to(ROOT)): digest(p)
              for directory in [stl / 'src', stl / 'stlport', headers]
              for p in sorted(directory.rglob('*')) if p.is_file()}
    inputs['builder'] = digest(Path(__file__))
    with tempfile.TemporaryDirectory(prefix='deps-', dir=output) as temporary:
        work = Path(temporary)
        setup = work / 'environment.cmd'
        setup.write_text('@echo off\ncall "%s" %s >nul\nif errorlevel 1 exit /b %%errorlevel%%\nset\n' % (args.vcvars.resolve(), arch))
        response = subprocess.run(['cmd', '/d', '/c', str(setup)], stdout=subprocess.PIPE,
                                  stderr=subprocess.STDOUT, timeout=60)
        if response.returncode:
            raise RuntimeError('vcvarsall failed; verify toolchain installation')
        # Never persist the environment, which can contain unrelated credentials.
        env = {k.upper(): v for k, v in os.environ.items()}
        for line in response.stdout.decode(errors='replace').splitlines():
            if '=' in line and not line.startswith('='):
                key, value = line.split('=', 1)
                env[key.upper()] = value
        for key in ['CL', '_CL_', 'LINK']:
            if env.get(key, '').strip():
                raise RuntimeError('Unset ambient %s compiler/linker options for reproducible dependency builds' % key)
        tools = {}
        for name in ['cl.exe', 'lib.exe']:
            path = shutil.which(name, path=env['PATH'])
            if not path:
                raise RuntimeError('Missing native compiler tool: ' + name)
            tools[name] = path
        # Compiler frontends and external headers can change in place without
        # changing INCLUDE or cl.exe. Hash content, not timestamps, for cache reuse.
        external = {}
        directories = [Path(value) for value in env.get('INCLUDE', '').split(';') if value]
        for directory in directories:
            if not directory.is_dir():
                raise RuntimeError('Missing compiler include directory: ' + str(directory))
            entries = [(str(path.relative_to(directory)), digest(path))
                       for path in sorted(directory.rglob('*')) if path.is_file()]
            external[str(directory)] = hashlib.sha256(json.dumps(entries).encode()).hexdigest()
        for directory in {Path(path).parent for path in tools.values()}:
            for path in sorted(directory.iterdir()):
                if path.is_file() and path.suffix.lower() in ['.dll', '.exe']:
                    external[str(path)] = digest(path)
        identity = dict(platform=args.platform, configuration=args.configuration,
                        jpeg_sha256=JPEG_SHA256, inputs=inputs, external=external, include_path=env.get('INCLUDE', ''),
                        compiler={path: digest(Path(path)) for path in tools.values()})
        manifest = output / 'manifest.json'
        if manifest.exists():
            saved = json.loads(manifest.read_text())
            if saved.get('identity') == identity and all(
                    (output / name).is_file() and digest(output / name) == sha
                    for name, sha in saved.get('outputs', {}).items()) and len(saved.get('outputs', {})) == 2:
                print('Verified dependency cache: ' + str(output))
                return
        with tarfile.open(archive) as source:
            for member in source.getmembers():
                destination = (work / member.name).resolve()
                if work not in destination.parents or not (member.isfile() or member.isdir()):
                    raise RuntimeError('Unsafe JPEG archive member: ' + member.name)
            source.extractall(work)
        jpeg = work / 'jpeg-6b'
        if (jpeg / 'jconfig.vc').read_bytes() != (headers / 'jconfig.h').read_bytes():
            raise RuntimeError('Repository JPEG configuration differs from verified official jconfig.vc')
        for name in ['jconfig.h', 'jmorecfg.h', 'jpeglib.h']:
            shutil.copyfile(headers / name, jpeg / name)
        version = work / 'v120.h'
        version.write_text('#if !defined(_MSC_VER) || _MSC_VER != 1800\n#error Requires VS2013 v120\n#endif\n')
        debug = args.configuration == 'Debug'
        common = ['/nologo', '/c', '/MTd' if debug else '/MT', '/Od' if debug else '/O2',
                  '/D_DEBUG' if debug else '/DNDEBUG', '/DWIN32', '/FI' + str(version)]
        commands = []
        logs = output / 'logs'
        logs.mkdir(exist_ok=True)

        def run(command, name):
            commands.append(command)
            result = subprocess.run(command, cwd=work, env=env, stdout=subprocess.PIPE,
                                    stderr=subprocess.STDOUT, timeout=240)
            (logs / (name + '.log')).write_bytes(result.stdout)
            if result.returncode:
                raise RuntimeError('Native build failed: %s; see %s' % (name, logs / (name + '.log')))

        def library(name, sources, flags):
            objects = work / name
            objects.mkdir()
            built = []
            for source in sources:
                obj = objects / (source.stem + '.obj')
                run([tools['cl.exe']] + common + flags + [str(source), '/Fo' + str(obj)], name + '-' + source.stem)
                if struct.unpack('<H', obj.read_bytes()[:2])[0] != (0x8664 if arch == 'amd64' else 0x14c):
                    raise RuntimeError('Wrong object architecture: ' + str(obj))
                built.append(str(obj))
            run([tools['lib.exe'], '/nologo', '/OUT:' + str(work / (name + '.lib')),
                 '/MACHINE:' + ('X64' if arch == 'amd64' else 'X86')] + built, name + '-archive')

        block = (stl / 'src/common_macros.mak').read_text().split('RELEASE_OBJECTS_static=', 1)[1].split('\n\n', 1)[0]
        names = re.findall(r'\$\(PATH_SEP\)(\w+)\.\$\(OBJEXT\)', block)
        if len(names) != 33 or len(set(names)) != 33:
            raise RuntimeError('Unexpected bundled STLport source inventory')
        sources = []
        for name in names:
            matches = [p for p in [stl / 'src' / (name + '.cpp'), stl / 'src' / (name + '.c')] if p.is_file()]
            if len(matches) != 1:
                raise RuntimeError('Ambiguous STLport source: ' + name)
            sources.extend(matches)
        library('stlport', sources, ['/W3', '/GR', '/EHsc', '/Zc:wchar_t-', '/D_WINDOWS', '/D_MBCS',
                '/D_STLP_NO_FORCE_INSTANTIATE', '/FI' + str(stl / 'src/vc_warning_disable.h'), '/I' + str(stl / 'stlport')])
        block = re.search(r'LIBSOURCES=(.*?)\n# memmgr', (jpeg / 'makefile.vc').read_text(), re.S).group(1)
        names = block.replace('\\', '').split() + ['jmemnobs.c']
        if len(names) != 46 or len(set(names)) != 46:
            raise RuntimeError('Unexpected official JPEG source inventory')
        library('jpeg', [jpeg / name for name in names], ['/TC', '/D_CRT_SECURE_NO_WARNINGS', '/FIwindows.h', '/I' + str(jpeg)])
        # Publish only after both libraries succeeded; manifest is the completion marker.
        for name in ['jpeg.lib', 'stlport.lib']:
            os.replace(work / name, output / name)
        shutil.copyfile(jpeg / 'README', output / 'JPEG-README.txt')
        (output / 'commands.json').write_text(json.dumps(commands, indent=2))
        data = dict(identity=identity, outputs={name: digest(output / name) for name in ['jpeg.lib', 'stlport.lib']})
        pending = output / 'manifest.pending'
        pending.write_text(json.dumps(data, indent=2))
        os.replace(pending, manifest)
        print('Built real client dependencies: ' + str(output))


if __name__ == '__main__':
    try:
        main()
    except (OSError, RuntimeError, subprocess.SubprocessError, ValueError) as error:
        sys.exit('build-client-deps: ' + str(error))
