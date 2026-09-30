#!/usr/bin/env python3
"""Offline native v120 builds of the client's original PCRE 4.1 and libxml2 2.6.7."""
import argparse
import json
import importlib.util
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tarfile
import tempfile
import time
_spec = importlib.util.spec_from_file_location("swg_native_dependencies", Path(__file__).with_name("build.py"))
_shared = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_shared)
digest, ensure_owner = _shared.digest, _shared.ensure_owner

ROOT = Path(__file__).resolve().parents[2]
SOURCES = {
    'pcre': ('https://downloads.sourceforge.net/project/pcre/pcre/4.1/pcre-4.1.tar.gz',
             '9ac01a6c5763120732c560ac26890c79c6ec0f8df4f5d42c2a6f0cae50c25575'),
    'libxml': ('https://download.gnome.org/sources/libxml2/2.6/libxml2-2.6.7.tar.gz',
               '785dec9ef48babf65f06c2dd98d6ef2ab2da09259af8f41c913c014046d5a39b')}


def extract(archive, work):
    with tarfile.open(archive) as source:
        for member in source.getmembers():
            if work not in (work / member.name).resolve().parents or not (member.isfile() or member.isdir()):
                raise RuntimeError('Unsafe source archive member: ' + member.name)
        source.extractall(work)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--vcvars', required=True, type=Path)
    parser.add_argument('--platform', required=True, choices=['Win32', 'x64'])
    parser.add_argument('--configuration', required=True, choices=['Debug', 'Release'])
    parser.add_argument('--pcre-archive', required=True, type=Path)
    parser.add_argument('--libxml-archive', required=True, type=Path)
    args = parser.parse_args()
    if os.name != 'nt':
        raise RuntimeError('Native Windows VS2013 is required')
    for name in SOURCES:
        path = getattr(args, name + '_archive')
        if not path.is_file() or digest(path) != SOURCES[name][1]:
            raise RuntimeError('Missing or mismatched offline %s source; obtain %s' % (name, SOURCES[name][0]))
    if not args.vcvars.is_file():
        raise RuntimeError('Missing VS2013 vcvarsall.bat')
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
                raise RuntimeError('Parser build lock timed out; inspect its owner before removal')
            time.sleep(0.5)
    try:
        ensure_owner(output, dict(checkout=str(ROOT.resolve()), platform=args.platform,
                                  configuration=args.configuration, provider='legacy-parsers'))
        build(args, output)
    finally:
        lock.rmdir()


def build(args, output):
    # Deliberately rebuild: no cache claim about external SDK/CRT inputs.
    arch = 'amd64' if args.platform == 'x64' else 'x86'
    debug = args.configuration == 'Debug'
    with tempfile.TemporaryDirectory(prefix='parsers-', dir=output) as tmp:
        work = Path(tmp)
        setup = work / 'environment.cmd'
        setup.write_text('@echo off\ncall "%s" %s >nul\nif errorlevel 1 exit /b %%errorlevel%%\nset\n' % (args.vcvars.resolve(), arch))
        result = subprocess.run(['cmd', '/d', '/c', str(setup)], capture_output=True, timeout=60)
        if result.returncode:
            raise RuntimeError('vcvarsall failed')
        env = {key.upper(): value for key, value in os.environ.items()}
        for line in result.stdout.decode(errors='replace').splitlines():
            if '=' in line and not line.startswith('='):
                key, value = line.split('=', 1)
                env[key.upper()] = value
        for key in ['CL', '_CL_', 'LINK']:
            if env.get(key, '').strip():
                raise RuntimeError('Unset ambient ' + key + ' options')
        tools = {name: shutil.which(name, path=env['PATH']) for name in ['cl.exe', 'lib.exe', 'nmake.exe', 'cscript.exe']}
        if not all(tools.values()):
            raise RuntimeError('Native compiler, librarian, nmake and cscript are required')
        logs = output / 'logs'
        logs.mkdir(exist_ok=True)
        commands = []

        def run(command, cwd, name):
            commands.append(command)
            result = subprocess.run(command, cwd=cwd, env=env, stdout=subprocess.PIPE,
                                    stderr=subprocess.STDOUT, timeout=240)
            (logs / (name + '.log')).write_bytes(result.stdout)
            if result.returncode:
                raise RuntimeError('Parser build failed: ' + name + '; see ' + str(logs))
            return result.stdout

        for name in SOURCES:
            extract(getattr(args, name + '_archive'), work)
        guard = work / 'v120.h'
        guard.write_text('#if !defined(_MSC_VER) || _MSC_VER != 1800\n#error Requires VS2013 v120\n#endif\n')
        pcre = work / 'pcre-4.1'
        header = (pcre / 'pcre.in').read_text().replace('@PCRE_MAJOR@', '4').replace('@PCRE_MINOR@', '1').replace('@PCRE_DATE@', '12-Mar-2003')
        original = ROOT / 'src/external/3rd/library/pcre/4.1/win32/include/pcre/pcre.h'
        if header != original.read_text():
            raise RuntimeError('PCRE public header differs from original client ABI')
        (pcre / 'pcre.h').write_text(header)
        (pcre / 'config.h').write_text((pcre / 'config.in').read_text().replace('HAVE_STRERROR 0', 'HAVE_STRERROR 1').replace('HAVE_MEMMOVE  0', 'HAVE_MEMMOVE  1'))
        flags = ['/nologo', '/DPCRE_STATIC', '/DSUPPORT_UTF8', '/DHAVE_CONFIG_H', '/DPOSIX_MALLOC_THRESHOLD=10',
                 '/MTd' if debug else '/MT', '/Od' if debug else '/O2', '/D_DEBUG' if debug else '/DNDEBUG', '/FI' + str(guard)]
        run([tools['cl.exe']] + flags + ['dftables.c', '/Fedftables.exe'], pcre, 'pcre-tables-build')
        (pcre / 'chartables.c').write_bytes(run([str(pcre / 'dftables.exe')], pcre, 'pcre-tables-run'))
        names = ['maketables', 'get', 'study', 'pcre', 'pcreposix']
        run([tools['cl.exe']] + flags + ['/c'] + [name + '.c' for name in names], pcre, 'pcre-compile')
        for name in names:
            if struct.unpack('<H', (pcre / (name + '.obj')).read_bytes()[:2])[0] != (0x8664 if arch == 'amd64' else 0x14c):
                raise RuntimeError('PCRE object architecture mismatch')
        run([tools['lib.exe'], '/nologo', '/OUT:pcre.lib'] + [name + '.obj' for name in names], pcre, 'pcre-archive')
        xml = work / 'libxml2-2.6.7'
        options = ['ftp=no', 'http=no', 'html=yes', 'c14n=no', 'docb=no', 'iconv=no', 'sax1=yes', 'legacy=no',
                   'xml_debug=' + ('yes' if debug else 'no'), 'mem_debug=' + ('yes' if debug else 'no'),
                   'cruntime=' + ('/MTd' if debug else '/MT'), 'debug=' + ('yes' if debug else 'no')]
        make = xml / 'win32/Makefile.msvc'
        text = make.read_text()
        if text.count('/OPT:NOWIN98') != 1:
            raise RuntimeError('Unexpected libxml linker-option source')
        # Removed option only controls obsolete Windows 98 image layout; v120 rejects it.
        text = text.replace('/OPT:NOWIN98', '').replace('CFLAGS = /nologo ', 'CFLAGS = /FI"' + str(guard) + '" /nologo ', 1)
        make.write_text(text)
        run([tools['cscript.exe'], '//nologo', 'configure.js'] + options, xml / 'win32', 'xml-configure')
        run([tools['nmake.exe'], '/f', 'Makefile.msvc', 'libxml'], xml / 'win32', 'xml-build')
        binary = xml / 'win32/bin.msvc/libxml2.dll'
        data = binary.read_bytes()
        pe = struct.unpack_from('<I', data, 0x3c)[0]
        if struct.unpack_from('<H', data, pe + 4)[0] != (0x8664 if arch == 'amd64' else 0x14c):
            raise RuntimeError('XML DLL architecture mismatch')
        # Execute genuine APIs before publishing; validates UTF-8 and imported XML data.
        probe = ROOT / 'tools/build-client-deps/tests/parser-smoke.c'
        run([tools['cl.exe']] + flags + ['/I' + str(pcre), '/I' + str(xml / 'include'),
            str(probe), str(pcre / 'pcre.lib'), str(binary.with_suffix('.lib')),
            '/Feparser-smoke.exe'], binary.parent, 'parser-smoke-build')
        run([str(binary.parent / 'parser-smoke.exe')], binary.parent, 'parser-smoke-run')
        products = {'pcre.lib': pcre / 'pcre.lib', 'libxml2.lib': binary.with_suffix('.lib'), 'libxml2.dll': binary}
        # Do not publish any product until both providers build successfully.
        for name, path in products.items():
            os.replace(path, output / name)
        shutil.copyfile(pcre / 'LICENCE', output / 'PCRE-LICENCE.txt')
        shutil.copyfile(xml / 'Copyright', output / 'LIBXML-COPYRIGHT.txt')
        (output / 'commands.json').write_text(json.dumps(commands, indent=2))
        manifest = dict(platform=args.platform, configuration=args.configuration, builder=digest(Path(__file__)),
                        sources={name: value[1] for name, value in SOURCES.items()},
                        outputs={name: digest(output / name) for name in products})
        pending = output / 'manifest.pending'
        pending.write_text(json.dumps(manifest, indent=2))
        os.replace(pending, output / 'manifest.json')
        print('Built genuine legacy parsers: ' + str(output))


if __name__ == '__main__':
    try:
        main()
    except (OSError, RuntimeError, ValueError, subprocess.SubprocessError) as error:
        sys.exit('build-client-parsers: ' + str(error))
