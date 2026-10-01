#!/usr/bin/env python3
"""Build the Miles x86 host or x64 component archives with Windows VS2013."""
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
import xml.etree.ElementTree as ET


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


def engine_worker_flags(engine_root, receipt, configuration):
    """Keep the legacy STLport engine TU separate from the modern pipe TUs."""
    if engine_root is None:
        raise ValueError('engine-worker requires --engine-root (actual engine checkout)')
    engine_root = engine_root.resolve()
    project = engine_root / 'src/engine/client/library/clientAudio/build/win32/clientAudio.vcxproj'
    namespace = {'ms': 'http://schemas.microsoft.com/developer/msbuild/2003'}
    tree = ET.parse(str(project))
    compile_settings = None
    for group in tree.findall('ms:ItemDefinitionGroup', namespace):
        if group.get('Condition', '').replace(' ', '') == "'$(Configuration)|$(Platform)'=='%s|x64'" % configuration:
            compile_settings = group.find('ms:ClCompile', namespace)
            break
    if compile_settings is None:
        raise ValueError('Missing clientAudio %s|x64 compiler settings' % configuration)
    # These ABI/runtime settings match the selected clientAudio configuration. PCH/minimal rebuild
    # are disabled because this is one independent, freshly compiled engine TU.
    flags = ['/nologo', '/c', '/EHsc', '/Y-', '/Gm-', '/Zc:wchar_t-',
             '/Zc:forScope', '/GR', '/Gy', '/fp:precise', '/W4', '/Zi',
             '/FC', '/showIncludes', '/WX']
    expected_runtime = 'MultiThreadedDebug' if configuration == 'Debug' else 'MultiThreaded'
    if compile_settings.findtext('ms:RuntimeLibrary', '', namespace) != expected_runtime:
        raise ValueError('Unsupported engine CRT selection')
    flags += (['/MTd', '/Od', '/Ob1', '/RTC1'] if configuration == 'Debug'
              else ['/MT', '/O2', '/Ob1', '/Oi', '/Ot', '/Oy', '/GF'])
    for setting, prefix in (('PreprocessorDefinitions', '/D'),
                            ('AdditionalIncludeDirectories', '/I')):
        values = compile_settings.findtext('ms:' + setting, '', namespace)
        for value in values.split(';'):
            value = value.strip()
            if not value or value == '%(' + setting + ')':
                continue
            if '$(' in value or '%(' in value:
                raise ValueError('Unresolved project setting: %s' % value)
            if prefix == '/I':
                value = str((project.parent / value.replace('\\', '/')).resolve())
            flags.append(prefix + value)
    stlport = engine_root / 'src/external/3rd/library/stlport453/stlport'
    if not (stlport / 'string').is_file() or '/I' + str(stlport) not in flags:
        raise ValueError('Actual engine STLport headers must be in the project include path')
    receipt['engine_project'] = str(project)
    receipt['engine_configuration'] = configuration + '|x64'
    return flags


def build(args, work, receipt):
    if os.name != 'nt':
        raise RuntimeError('Run this build with Python 3 on native Windows')
    vcvars = args.vcvars.resolve()
    sdk = args.sdk.resolve() if args.sdk else None
    if not vcvars.is_file():
        raise ValueError('Missing VS2013 vcvarsall.bat: %s' % vcvars)
    if args.target != 'engine-worker' and (sdk is None or not (sdk / 'Mss.h').is_file()):
        raise ValueError('--sdk must be the SDK include directory containing Mss.h')
    sdk_lib = args.sdk_lib.resolve() if args.sdk_lib else None
    if args.target == 'host' and (sdk_lib is None or not sdk_lib.is_file()
                                  or sdk_lib.name.lower() != 'mss32.lib'):
        raise ValueError('host requires --sdk-lib pointing to the real x86 Mss32.lib')
    manifest = json.loads((ROOT / 'sources.json').read_text(encoding='utf-8'))
    relative_sources = ([] if args.target == 'audio-dev' else manifest['pipe'] + manifest['engine-worker']
                        if args.target == 'pipe-probe' else manifest[args.target])
    if not isinstance(relative_sources, list) or (not relative_sources and args.target != 'audio-dev'):
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
    if args.target == 'audio-dev':
        audio_root = ROOT.parents[1] / 'src/engine/client/library/clientAudio/src/win32'
        sources += [audio_root / name for name in ('Audio.cpp', 'SoundObject3d.cpp', 'SetupClientAudio.cpp')]
        receipt['sources'] = [str(source) for source in sources]
    if args.target == 'pipe-probe':
        sources.append(ROOT / 'tests/pipe_lock_probe.cpp')
        sources.append(ROOT / 'tests/engine_worker_context.cpp')
        receipt['sources'] = relative_sources + ['../tests/pipe_lock_probe.cpp',
                                                 '../tests/engine_worker_context.cpp']
    env = compiler_environment(vcvars, receipt['arch'], work, receipt)
    tools = {}
    for name in ('cl.exe', 'link.exe' if args.target in ('host', 'pipe-probe') else 'lib.exe'):
        tools[name] = shutil.which(name, path=env.get('PATH'))
        if not tools[name]:
            raise RuntimeError('Tool missing from VS2013 environment: %s' % name)
    objects = []
    if args.target in ('engine-worker', 'audio-dev'):
        flags = engine_worker_flags(args.engine_root, receipt, args.configuration)
        if args.target == 'audio-dev':
            flags += ['/DCLIENT_MILES_DEV_FACADE', '/I' + str(source_root),
                      '/I' + str(args.engine_root.resolve() /
                                 'src/engine/client/library/clientAudio/src/win32')]
    else:
        flags = ['/nologo', '/c', '/W4', '/WX', '/EHsc', '/Zi',
                 '/DWIN32', '/D_WIN32_WINNT=0x0601', '/DNOMINMAX',
                 '/I' + str(sdk), '/I' + str(source_root)]
        flags += (['/MTd', '/Od', '/Ob0'] if args.configuration == 'Debug'
                  else ['/MT', '/O2', '/Ob2', '/DNDEBUG'])
    if args.target == 'host':
        if args.bink_sdk is None or not (args.bink_sdk / 'bink.h').is_file():
            raise ValueError('--bink-sdk must contain genuine Bink 1.9c bink.h')
        flags += ['/I' + str(args.bink_sdk.resolve())]
    for index, source in enumerate(sources):
        stem = '%02d-%s' % (index, source.stem)
        obj = work / (stem + '.obj')
        unit_flags = (engine_worker_flags(args.engine_root, receipt, args.configuration)
                      if args.target == 'pipe-probe' and source.name in
                         ('EngineFileWorker.cpp', 'engine_worker_context.cpp')
                      else flags)
        run([tools['cl.exe']] + unit_flags + ['/Fo' + str(obj),
            '/Fd' + str(work / (stem + '.pdb')), str(source)],
            work, env, receipt, stem)
        objects.append(str(obj))
    if args.target == 'audio-dev':
        receipt['artifacts'] = [{'path': obj,
                                'sha256': hashlib.sha256(Path(obj).read_bytes()).hexdigest()}
                               for obj in objects]
        return
    artifact = work / ('miles-pipe-probe.exe' if args.target == 'pipe-probe'
                       else 'miles-host.exe' if args.target == 'host'
                       else 'miles-%s.lib' % args.target)
    options = ['/NOLOGO', '/WX', '/OUT:' + str(artifact)]
    if args.target == 'host':
        linker = tools['link.exe']
        options += ['/MACHINE:X86', '/DEBUG', '/INCREMENTAL:NO',
                    '/PDB:' + str(work / 'miles-host.pdb')]
        options += objects + [str(sdk_lib), 'user32.lib', 'kernel32.lib',
                              'advapi32.lib']
    elif args.target == 'pipe-probe':
        linker = tools['link.exe']
        engine_root = args.engine_root.resolve()
        names = ('sharedThread', 'sharedSynchronization', 'sharedFoundation',
                 'sharedMemoryManager', 'sharedDebug', 'sharedMath', 'sharedRandom',
                 'unicode', 'sharedFile', 'sharedCompression', 'fileInterface',
                 'archive', 'zlib')
        libraries = [engine_root / 'src/compile/x64' / name / args.configuration / (name + '.lib')
                     for name in names]
        libraries.append(engine_root / 'src/compile/deps/v120/x64' / args.configuration / 'stlport.lib')
        for library in libraries:
            if not library.is_file():
                raise ValueError('Missing genuine selected-configuration x64 library: %s' % library)
        receipt['engine_libraries'] = [str(path) for path in libraries]
        # The maintained engine archives store compiler PDBs under obj/;
        # LINK searches beside the library and in its working directory.
        compiler_pdbs = []
        for library in libraries:
            for pdb in sorted((library.parent / 'obj').glob('*.pdb')):
                if (work / pdb.name).exists():
                    raise ValueError('Conflicting compiler PDB basename: %s' % pdb.name)
                shutil.copyfile(str(pdb), str(work / pdb.name))
                compiler_pdbs.append(str(pdb))
        receipt['engine_compiler_pdbs'] = compiler_pdbs
        options += ['/MACHINE:X64', '/DEBUG', '/INCREMENTAL:NO', '/OPT:REF',
                    '/MAP:' + str(work / 'miles-pipe-probe.map'),
                    '/PDB:' + str(work / 'miles-pipe-probe.pdb'),
                    '/NODEFAULTLIB:stlport_vc71_static.lib',
                    '/NODEFAULTLIB:stlport_vc71_stldebug_static.lib']
        options += objects + [str(path) for path in libraries]
        options += ['kernel32.lib', 'user32.lib', 'advapi32.lib', 'winmm.lib',
                    'gdi32.lib', 'shell32.lib', 'ws2_32.lib', 'dbghelp.lib']
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
    parser.add_argument('--target', choices=('host', 'pipe', 'native', 'engine-worker', 'pipe-probe', 'audio-dev'), required=True)
    parser.add_argument('--configuration', choices=('Debug', 'Release'), default='Debug')
    parser.add_argument('--sdk', type=Path,
                        help='Miles SDK include directory containing Mss.h (required except engine-worker)')
    parser.add_argument('--engine-root', type=Path,
                        help='actual engine checkout, required for engine-worker x64 archive')
    parser.add_argument('--bink-sdk', type=Path,
                        default=ROOT.parents[1] / 'src/external/3rd/library/bink/include',
                        help='genuine Bink 1.9c include directory (host; defaults to repository SDK)')
    parser.add_argument('--sdk-lib', type=Path, help='real x86 Mss32.lib (host only)')
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--vcvars', type=Path, default=Path(os.environ.get(
        'ProgramFiles(x86)', r'C:\Program Files (x86)')) /
        'Microsoft Visual Studio 12.0/VC/vcvarsall.bat')
    args = parser.parse_args()
    arch = 'x86' if args.target == 'host' else 'x64'
    output = args.out.resolve() / args.target / arch
    if args.configuration != 'Debug':
        output /= args.configuration
    output.mkdir(parents=True, exist_ok=True)
    # A fresh directory preserves previous evidence and prevents stale products.
    work = Path(tempfile.mkdtemp(prefix='build-', dir=str(output)))
    receipt = {'target': args.target, 'arch': arch, 'configuration': args.configuration, 'command': sys.argv,
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
