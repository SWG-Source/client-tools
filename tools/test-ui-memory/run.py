#!/usr/bin/env python3
"""Native VS2013 UI pool test; requires genuine prebuilt project dependencies."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=Path(__file__).resolve().parent.parent.parent)
    parser.add_argument('--vcvars', type=Path, required=True)
    parser.add_argument('--inputs', type=Path, required=True)
    parser.add_argument('--configuration', choices=['Debug', 'Release'], required=True)
    parser.add_argument('--platform', choices=['Win32', 'x64'], required=True)
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    source, out = args.source.resolve(), args.out.resolve()
    out.mkdir(parents=True, exist_ok=False)
    inputs = json.loads(args.inputs.read_text())
    ui = source / 'src/external/3rd/library/ui'
    sources = [ui / 'src/shared/core/UiMemoryBlockManager.cpp',
               ui / 'src/shared/core/UiReport.cpp',
               Path(__file__).resolve().with_name('fixtures.cpp'),
               ui / 'src/shared/core/UILowerString.cpp']
    # Input paths may be relative to the input manifest, never the process cwd.
    def resolve(value):
        path = Path(value)
        return path.resolve() if path.is_absolute() else (args.inputs.resolve().parent / path).resolve()
    libraries = [resolve(p) for p in inputs['link_inputs']]
    includes = [resolve(p) for p in inputs['include_dirs']]
    for path in sources + libraries + includes + [args.vcvars.resolve()]:
        if not path.exists():
            raise RuntimeError('Missing real build input: ' + str(path))
    if not any('stlport' in p.name.lower() for p in libraries):
        raise RuntimeError('Explicit matching bundled STLport library required')
    def quote(value):
        value = str(value).replace('\\', '/')
        if any(c in value for c in ['"', '\n', '\r', '%']):
            raise ValueError('Unsupported command character in input')
        return '"' + value + '"'
    debug = args.configuration == 'Debug'
    flags = ['/nologo', '/EHsc', '/Y-', '/Zc:wchar_t-', '/Gy',
             '/MTd' if debug else '/MT', '/Od' if debug else '/O2']
    flags += ['/D' + quote(d) for d in inputs['defines']]
    flags += ['/I' + quote(p) for p in includes]
    commands = []
    for i, path in enumerate(sources):
        rsp = out / ('compile-%d.rsp' % i)
        rsp.write_text(' '.join(flags + ['/c', quote(path), '/Fo' + quote(out / ('%d.obj' % i))]))
        commands += ['cl @' + quote(rsp), 'if errorlevel 1 exit /b %errorlevel%']
    link = ['/nologo', '/OPT:REF', '/NODEFAULTLIB:stlport_vc71_static.lib',
            '/NODEFAULTLIB:stlport_vc71_stldebug_static.lib']
    link += [quote(out / ('%d.obj' % i)) for i in range(len(sources))]
    link += [quote(p) for p in libraries]
    link += ['kernel32.lib', 'user32.lib', 'gdi32.lib', 'advapi32.lib', 'winmm.lib', 'shell32.lib',
             '/OUT:' + quote(out / 'probe.exe'), '/MAP:' + quote(out / 'probe.map')]
    (out / 'link.rsp').write_text(' '.join(link))
    commands += ['link @' + quote(out / 'link.rsp'), 'exit /b %errorlevel%']
    build = out / 'build.cmd'
    build.write_text('@echo off\ncall ' + quote(args.vcvars.resolve()) +
                     (' x86' if args.platform == 'Win32' else ' amd64') +
                     '\nif errorlevel 1 exit /b %errorlevel%\n' + '\n'.join(commands))
    result = {'configuration': args.configuration, 'platform': args.platform,
              'sources': {str(p): digest(p) for p in sources},
              'link_inputs': {str(p): digest(p) for p in libraries}, 'inputs': inputs,
              'vcvars_sha256': digest(args.vcvars.resolve()), 'passed': False}
    try:
        process = subprocess.run(['cmd', '/c', str(build)], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        (out / 'build.log').write_bytes(process.stdout)
        result['build_exit'] = process.returncode
        if process.returncode:
            return 1
        data = (out / 'probe.exe').read_bytes()
        machine = struct.unpack_from('<H', data, struct.unpack_from('<I', data, 0x3c)[0] + 4)[0]
        result['machine'] = machine
        run = subprocess.run([str(out / 'probe.exe')], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=30)
        (out / 'run.log').write_bytes(run.stdout)
        lines = run.stdout.decode(errors='replace').splitlines()
        expected = 'PASS: 3129 UI alignment checks'
        extras = [line for line in lines if line != expected]
        mapping = (out / 'probe.map').read_text(errors='replace')
        binding = any('?allocMem@UiMemoryBlockManager@@' in line and '0.obj' in line for line in mapping.splitlines())
        result.update(run_exit=run.returncode, production_map_binding=binding, diagnostic_lines=extras)
        result['passed'] = (machine == (0x14c if args.platform == 'Win32' else 0x8664) and
                            run.returncode == 0 and lines.count(expected) == 1 and binding and
                            (not extras or (debug and all(re.fullmatch(r'MM::remove \d+/\d+=bytes \d+/\d+=allocs', line) for line in extras))))
        return 0 if result['passed'] else 1
    finally:
        (out / 'results.json').write_text(json.dumps(result, indent=2))
        print('PASS' if result['passed'] else 'FAIL', args.configuration, args.platform)


if __name__ == '__main__':
    raise SystemExit(main())
