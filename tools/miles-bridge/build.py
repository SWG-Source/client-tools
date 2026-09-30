#!/usr/bin/env python3
"""Build the Miles x86 host or x64 pipe/native archive with Windows VS2013."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parent


def run(command, work, env, receipt, label):
    """Retain complete tool output, including failed invocations."""
    log = work / (label + '.log')
    entry = {'command': command, 'cwd': str(work), 'exit_code': None,
             'log': str(log)}
    receipt['steps'].append(entry)
    with log.open('wb') as stream:
        try:
            result = subprocess.run(command, cwd=str(work), env=env,
                                    stdout=stream, stderr=subprocess.STDOUT)
        except OSError as error:
            stream.write(str(error).encode('utf-8'))
            raise
    entry['exit_code'] = result.returncode
    if result.returncode:
        raise RuntimeError('%s failed (%s); see %s' %
                           (label, result.returncode, log))


def compiler_environment(vcvars, arch, work, receipt):
    # Capture SET privately; only vcvars diagnostics go into the retained log.
    log = work / 'vcvars.log'
    setup = work / 'environment.cmd'
    for path in (vcvars, log):
        if any(character in str(path) for character in '%!\r\n"'):
            raise ValueError('Unsupported cmd metacharacter in path: %s' % path)
    setup.write_text('@echo off\ncall "%s" %s > "%s" 2>&1\n'
                     'if errorlevel 1 exit /b %%errorlevel%%\nset\n' %
                     (vcvars, 'x86' if arch == 'x86' else 'amd64', log))
    # Pass cmd's quoting literally; list2cmdline escapes quotes for C runtimes,
    # which is not the quoting grammar used by cmd.exe.
    command = 'cmd.exe /d /s /c ""%s""' % setup
    entry = {'command': command, 'cwd': str(work), 'exit_code': None,
             'log': str(log)}
    receipt['steps'].append(entry)
    result = subprocess.run(command, cwd=str(work), stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    entry['exit_code'] = result.returncode
    if result.returncode:
        raise RuntimeError('vcvarsall failed; see %s' % log)
    env = {key.upper(): value for key, value in os.environ.items()}
    for line in result.stdout.decode('mbcs', errors='replace').splitlines():
        if '=' in line and not line.startswith('='):
            key, value = line.split('=', 1)
            env[key.upper()] = value
    if env.get('VISUALSTUDIOVERSION') != '12.0':
        raise RuntimeError('VS2013 (v120) environment required')
    for key in ('CL', '_CL_', 'LINK', '_LINK_'):
        if env.get(key, '').strip():
            raise RuntimeError('Unset ambient %s build options before building' % key)
    return env


def build(args, work, receipt):
    if os.name != 'nt':
        raise RuntimeError('Run this build with Python 3 on native Windows')
    vcvars = args.vcvars.resolve()
    sdk = args.sdk.resolve()
    if not vcvars.is_file():
        raise ValueError('Missing VS2013 vcvarsall.bat: %s' % vcvars)
    if not (sdk / 'Mss.h').is_file():
        raise ValueError('--sdk must be the SDK include directory containing Mss.h')
    sdk_lib = args.sdk_lib.resolve() if args.sdk_lib else None
    if args.target == 'host' and (sdk_lib is None or not sdk_lib.is_file()
                                  or sdk_lib.name.lower() != 'mss32.lib'):
        raise ValueError('host requires --sdk-lib pointing to the real x86 Mss32.lib')
    manifest = json.loads((ROOT / 'sources.json').read_text(encoding='utf-8'))
    relative_sources = manifest[args.target]
    if not isinstance(relative_sources, list) or not relative_sources:
        raise ValueError('sources.json target must contain a nonempty source list')
    source_root = ROOT / 'src'
    sources = []
    for relative in relative_sources:
        path = (source_root / relative).resolve()
        path.relative_to(source_root.resolve())
        if not path.is_file() or path.suffix.lower() != '.cpp':
            raise ValueError('Missing or invalid source: %s' % relative)
        sources.append(path)
    if len(set(sources)) != len(sources):
        raise ValueError('Duplicate sources in sources.json')
    receipt['sources'] = relative_sources
    env = compiler_environment(vcvars, receipt['arch'], work, receipt)
    tools = {}
    for name in ('cl.exe', 'link.exe' if args.target == 'host' else 'lib.exe'):
        tools[name] = shutil.which(name, path=env.get('PATH'))
        if not tools[name]:
            raise RuntimeError('Tool missing from VS2013 environment: %s' % name)
    objects = []
    flags = ['/nologo', '/c', '/W4', '/WX', '/EHsc', '/MTd', '/Od', '/Ob0', '/Zi',
             '/DWIN32', '/D_WIN32_WINNT=0x0601', '/DNOMINMAX',
             '/I' + str(sdk), '/I' + str(source_root)]
    for index, source in enumerate(sources):
        stem = '%02d-%s' % (index, source.stem)
        obj = work / (stem + '.obj')
        run([tools['cl.exe']] + flags + ['/Fo' + str(obj),
            '/Fd' + str(work / (stem + '.pdb')), str(source)],
            work, env, receipt, stem)
        objects.append(str(obj))
    artifact = work / ('miles-host.exe' if args.target == 'host'
                       else 'miles-%s.lib' % args.target)
    options = ['/NOLOGO', '/WX', '/OUT:' + str(artifact)]
    if args.target == 'host':
        linker = tools['link.exe']
        options += ['/MACHINE:X86', '/DEBUG', '/INCREMENTAL:NO',
                    '/PDB:' + str(work / 'miles-host.pdb')]
        options += objects + [str(sdk_lib), 'user32.lib', 'kernel32.lib',
                              'advapi32.lib']
    else:
        linker = tools['lib.exe']
        options += ['/MACHINE:X64'] + objects
    response = work / 'link.rsp'
    response.write_text(subprocess.list2cmdline(options), encoding='utf-8')
    receipt['link_arguments'] = options
    run([linker, '@' + str(response)], work, env, receipt, 'link')
    receipt['artifact'] = {'path': str(artifact),
                           'sha256': hashlib.sha256(artifact.read_bytes()).hexdigest()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target', choices=('host', 'pipe', 'native'), required=True)
    parser.add_argument('--sdk', type=Path, required=True,
                        help='Miles SDK include directory containing Mss.h')
    parser.add_argument('--sdk-lib', type=Path, help='real x86 Mss32.lib (host only)')
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--vcvars', type=Path, default=Path(os.environ.get(
        'ProgramFiles(x86)', r'C:\Program Files (x86)')) /
        'Microsoft Visual Studio 12.0/VC/vcvarsall.bat')
    args = parser.parse_args()
    arch = 'x86' if args.target == 'host' else 'x64'
    output = args.out.resolve() / args.target / arch
    output.mkdir(parents=True, exist_ok=True)
    # A fresh directory preserves previous evidence and prevents stale products.
    work = Path(tempfile.mkdtemp(prefix='build-', dir=str(output)))
    receipt = {'target': args.target, 'arch': arch, 'command': sys.argv,
               'started_utc': datetime.now(timezone.utc).isoformat(),
               'steps': [], 'artifact': None, 'exit_code': 1}
    try:
        build(args, work, receipt)
        receipt['exit_code'] = 0
    except (Exception, KeyboardInterrupt) as error:
        receipt['error'] = str(error) or type(error).__name__
        print(receipt['error'], file=sys.stderr)
    finally:
        receipt['finished_utc'] = datetime.now(timezone.utc).isoformat()
        path = work / 'receipt.json'
        path.write_text(json.dumps(receipt, indent=2) + '\n', encoding='utf-8')
        print('Build receipt: %s' % path)
    return receipt['exit_code']


if __name__ == '__main__':
    sys.exit(main())
