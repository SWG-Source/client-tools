#!/usr/bin/env python3
"""Build current DPVS and every probe from source with native VS2013 (v120)."""
import argparse
import collections
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
import xml.etree.ElementTree as ET

HERE = Path(__file__).resolve().parent
SUBTREE = Path('src/external/3rd/library/dpvs')
NS = {'m': 'http://schemas.microsoft.com/developer/msbuild/2003'}


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def execute(args, cwd, log, env=None):
    with log.open('wb') as output:
        result = subprocess.run(args, cwd=cwd, env=env, stdout=output,
                                stderr=subprocess.STDOUT, timeout=1200)
    log.with_suffix('.exit').write_text(str(result.returncode) + '\n')
    require(result.returncode == 0, '{} failed ({}): {}'.format(args[0], result.returncode, log))
    return log.read_text(encoding='utf-8', errors='replace')


def compiler_environment(vcvars, arch):
    command = 'call "{}" {} >nul && set'.format(vcvars, arch)
    result = subprocess.run('cmd.exe /d /s /c "' + command + '"',
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True)
    env = os.environ.copy()
    for line in result.stdout.decode('mbcs').splitlines():
        key, sep, value = line.partition('=')
        if sep and key:
            env[key] = value
    # Never inherit implicit cl.exe arguments from a developer environment.
    for key in list(env):
        if key.upper() in ('CL', '_CL_', 'LINK', '_LINK_'):
            del env[key]
    return env


def check_runtime(stress, occlusion):
    require(len(re.findall(r'^QUERY ', stress, re.M)) == 192, 'stress: extra/missing records')
    require(len(re.findall(r'^mode=', occlusion, re.M)) == 64, 'occlusion: extra/missing records')
    queries = re.findall(r'^QUERY q=(\d+) sentinel=1 begin=1 end=1 invalid=0 visible=(.*)$', stress, re.M)
    require([int(q) for q, _ in queries] == list(range(192)), 'stress: missing/invalid query')
    for _, visible in queries:
        ids = [int(x) for x in visible.split(',') if x]
        require(0 in ids and len(ids) == len(set(ids)) and all(0 <= x < 257 for x in ids),
                'stress: invalid visible object records')
    require(stress.count('RESULT failures=0') == 1, 'stress: failed or missing verdict')
    frames = re.findall(r'^mode=1 frame=(\d+) visible=319 begin=1 end=1 writes=1$', occlusion, re.M)
    require([int(f) for f in frames] == list(range(64)), 'occlusion: missing/invalid frame')
    require(occlusion.count('SUMMARY firstHidden=0 reappeared=0 totalWrites=64 failures=0') == 1,
            'occlusion: failed or missing verdict')
    return {'stress_queries': len(queries), 'occlusion_frames': len(frames)}


def number(bits):
    return struct.unpack('!f', bytes.fromhex(bits))[0]


def compare_numeric(left, right, caller):
    prefixes = ('R ', 'C ') if caller else ('F ', 'R ', 'M ', 'D ')
    a = [line.split() for line in left.splitlines() if line.startswith(prefixes)]
    b = [line.split() for line in right.splitlines() if line.startswith(prefixes)]
    counts = collections.Counter(row[0] for row in a)
    require(len(a) == len(b) == (11978 if caller else 38880), 'numerical row count')
    require(len({tuple(row[:(4 if row[0] == 'R' else 3)]) for row in a}) == len(a) if caller else
            len({tuple(row[:(5 if row[0] == 'R' else 4)]) for row in a}) == len(a), 'duplicate numerical fixture keys')
    if not caller:
        require(counts == {'F': 6240, 'R': 24960, 'M': 1440, 'D': 6240}, 'broad row distribution')
    if caller:
        require(counts == {'C': 3584, 'R': 8394}, 'caller row distribution')
    require('RAW_X87 cw=037f' in left and 'DISPATCH assembly=1' in left,
            'Win32 oracle is not PC64 assembly')
    require('DISPATCH assembly=0' in right, 'x64 did not use scalar path')
    expected = 'RESULT rasterExactFailures=0' if caller else 'RESULT rows=38880 failures=0'
    require(left.count(expected) == right.count(expected) == 1 and
            len(re.findall(r'^RESULT ', left, re.M)) == len(re.findall(r'^RESULT ', right, re.M)) == 1, 'numerical verdict')
    require('STATE_FAIL' not in left + right, 'floating-point state changed')
    differences = []
    for index, (x, y) in enumerate(zip(a, b)):
        if caller and x[0] == 'C':
            # Compare all input plane/camera bits as well as the fixture indices.
            require(x[:3] == y[:3] and x[5:] == y[5:], 'caller inputs differ')
            require(x[4] == y[4], 'dot sign/zero classification changed')
            if x[3] != y[3]:
                require(number(x[3]) < 0 and number(y[3]) < 0 and
                        abs(int(x[3], 16) - int(y[3], 16)) <= 1,
                        'dot exceeds the documented negative one-ULP limit')
        elif not caller and x[0] == 'M':
            require(x[:4] == y[:4] and all(number(u) == number(v) for u, v in zip(x[4:], y[4:])),
                    'min/max differs beyond signed zero')
        else:
            require(x == y, 'numerical mismatch at row {}: {} != {}'.format(index, x, y))
        if x != y:
            differences.append({'row': index, 'win32': x, 'x64': y})
    require(len(differences) <= (1 if caller else 15), 'additional numerical differences')
    return {'rows': len(a), 'counts': dict(counts), 'differences': differences}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=HERE.parents[1])
    parser.add_argument('--source-archive', type=Path, help='git archive tar containing DPVS and this harness')
    parser.add_argument('--baseline', type=Path, help='optional stock checkout for Win32 DLL comparison')
    parser.add_argument('--out', type=Path, required=True, help='new, nonexistent output directory')
    parser.add_argument('--vcvars', type=Path, default=Path(os.environ.get('ProgramFiles(x86)', 'C:/Program Files (x86)')) / 'Microsoft Visual Studio 12.0/VC/vcvarsall.bat')
    parser.add_argument('--msbuild', type=Path, default=Path(os.environ.get('ProgramFiles(x86)', 'C:/Program Files (x86)')) / 'MSBuild/12.0/Bin/MSBuild.exe')
    args = parser.parse_args()
    require(os.name == 'nt', 'Run on native Windows with VS2013/v120 and Python 3')
    require(args.vcvars.is_file() and args.msbuild.is_file(), 'VS2013/v120 or MSBuild12 missing')
    source, out = args.source.resolve(), args.out.resolve()
    require(not out.exists(), 'Output must not exist; stale results are never reused')
    require(source / SUBTREE not in out.parents, 'Output cannot be inside the source subtree')
    if args.baseline:
        require(out not in args.baseline.resolve().parents, 'Baseline cannot be inside generated output')
    out.mkdir(parents=True)
    provenance = {'source_revision': None, 'source_archive_sha256': None}
    if args.source_archive:
        source = out / 'archive'
        with tarfile.open(args.source_archive) as archive:
            revision = archive.pax_headers.get('comment', '')
            require(re.fullmatch(r'[0-9a-f]{40}', revision), 'git archive commit metadata missing')
            for member in archive.getmembers():
                name = Path(member.name)
                require(not name.is_absolute() and '..' not in name.parts and
                        not member.issym() and not member.islnk() and
                        (member.isfile() or member.isdir()), 'unsafe archive member')
                if member.isdir():
                    (source / name).mkdir(parents=True, exist_ok=True)
                else:
                    (source / name).parent.mkdir(parents=True, exist_ok=True)
                    with archive.extractfile(member) as stream:
                        (source / name).write_bytes(stream.read())
        for p in list(HERE.glob('*.py')) + list(HERE.glob('*.cpp')):
            require(sha(p) == sha(source / 'tools/test-dpvs' / p.name), 'runner differs from archived commit: ' + p.name)
        provenance = {'source_revision': revision, 'source_archive_sha256': sha(args.source_archive)}
    require((source / SUBTREE).is_dir(), 'DPVS source not found')
    (out / 'provenance.json').write_text(json.dumps(provenance, indent=2) + '\n')
    snapshot = out / 'source'
    shutil.copytree(source / SUBTREE, snapshot / SUBTREE,
                    ignore=shutil.ignore_patterns('*.obj', '*.dll', '*.exe', '*.lib', '*.pch', '*.pdb', '*.ilk', '*.res', '*.tlog'))
    # Fixtures come from the runner revision, production code from --source.
    fixtures = out / 'fixtures'
    fixtures.mkdir()
    for name in ('stress', 'occlusion', 'numerical', 'caller'):
        shutil.copy2(HERE / (name + '.cpp'), fixtures / (name + '.cpp'))
    dpvs = snapshot / SUBTREE
    project = dpvs / 'implementation/msvc8/dpvs.vcxproj'
    manifest = {str(p.relative_to(snapshot)): sha(p) for p in snapshot.rglob('*') if p.is_file()}
    manifest.update({'fixtures/' + p.name: sha(p) for p in fixtures.iterdir()})
    manifest.update({'harness/' + p.name: sha(p) for p in HERE.glob('*.py')})
    (out / 'source-sha256.json').write_text(json.dumps(manifest, indent=2) + '\n')
    include = [dpvs / 'interface', dpvs / 'implementation/include', dpvs / 'implementation/sources']
    sources = [Path(node.attrib['Include'].replace('\\', '/')).stem
               for node in ET.parse(project).findall('.//m:ClCompile[@Include]', NS)]
    require(len(sources) == len(set(sources)) and 'dpvsMath' in sources, 'unexpected project source list')
    results, numerics = [], {}
    for platform, arch in [('Win32', 'x86'), ('x64', 'amd64')]:
        env = compiler_environment(args.vcvars, arch)
        compiler = shutil.which('cl.exe', path=env.get('Path', env.get('PATH')))
        require(compiler is not None, 'cl.exe missing')
        # cl without a source exits nonzero; a tiny compile captures version and proves usable toolchain.
        version_source = fixtures / 'version.cpp'
        version_source.write_text('#if _MSC_VER != 1800\n#error VS2013 required\n#endif\nint x;\n')
        execute([compiler, '/c', str(version_source), '/Fo' + str(out / (arch + '-version.obj'))],
                out, out / (arch + '-compiler.log'), env)
        for config in ('Release', 'Debug'):
            tag = platform + '-' + config
            logs = out / tag
            logs.mkdir()
            print('Building ' + tag, flush=True)
            execute([str(args.msbuild), str(project), '/t:Rebuild', '/m:2', '/v:normal',
                     '/p:Configuration=' + config, '/p:Platform=' + platform,
                     '/p:UserRootDir=' + str(out / 'empty-props') + '\\'], project.parent,
                    logs / 'build.log', env)
            product = (project.parent / '../../../../../../compile' / platform / 'dpvs' / config).resolve()
            dll, library = product / 'dpvs.dll', product / 'dpvs.lib'
            require(dll.is_file() and library.is_file(), 'project output missing')
            shutil.copy2(dll, logs / dll.name)
            runtime = {}
            for probe in ('stress', 'occlusion'):
                exe = logs / (probe + '.exe')
                execute([compiler, '/nologo', '/EHsc', '/MD', '/DDPVS_DLL',
                         '/I' + str(include[0]), str(fixtures / (probe + '.cpp')),
                         str(library), '/Fo' + str(logs / (probe + '.obj')), '/Fe' + str(exe)],
                        logs, logs / (probe + '-build.log'), env)
                runtime[probe] = execute([str(exe)], logs, logs / (probe + '.log'), env)
            result = {'platform': platform, 'configuration': config, 'dll_sha256': sha(dll),
                      'build_exit': 0, 'probe_exits': [0, 0]}
            result.update(check_runtime(runtime['stress'], runtime['occlusion']))
            results.append(result)
            if config == 'Release':
                object_dir = project.parent / 'release' if platform == 'Win32' else product
                objects = [object_dir / (stem + '.obj') for stem in sources if stem != 'dpvsMath']
                require(all(obj.is_file() for obj in objects), 'fresh project objects missing')
                rsp = logs / 'objects.rsp'
                rsp.write_text('\n'.join('"{}"'.format(obj) for obj in objects))
                for probe in ('numerical', 'caller'):
                    obj, exe = logs / (probe + '.obj'), logs / (probe + '.exe')
                    execute([compiler, '/nologo', '/c', '/O2', '/Ob1', '/fp:precise', '/EHsc', '/MT',
                             '/DWIN32', '/DNDEBUG', '/DDPVS_DLL', '/DDPVS_BUILD_LIBRARY'] +
                            ['/I' + str(p) for p in include] + [str(fixtures / (probe + '.cpp')), '/Fo' + str(obj)],
                            logs, logs / (probe + '-build.log'), env)
                    execute([str(Path(compiler).with_name('link.exe')), '/nologo', '/OUT:' + str(exe), str(obj), '@' + str(rsp),
                             'kernel32.lib', 'user32.lib', 'advapi32.lib'], logs, logs / (probe + '-link.log'), env)
                    numerics[(platform, probe)] = execute([str(exe)], logs, logs / (probe + '.log'), env)
    report = {'provenance': provenance, 'runtime': results, 'numerical': {probe: compare_numeric(numerics[('Win32', probe)],
              numerics[('x64', probe)], probe == 'caller') for probe in ('numerical', 'caller')}}
    if args.baseline:
        baseline = args.baseline.resolve() / SUBTREE
        require(baseline.is_dir(), 'baseline DPVS source missing')
        # Same build path is necessary: changing it changes embedded PDB/debug paths.
        shutil.rmtree(dpvs)
        compile_dir = snapshot / 'src/compile'
        if compile_dir.exists():
            shutil.rmtree(compile_dir)
        shutil.copytree(baseline, dpvs, ignore=shutil.ignore_patterns('*.obj', '*.dll', '*.exe', '*.lib', '*.pch', '*.pdb', '*.ilk', '*.res', '*.tlog'))
        (out / 'baseline-sha256.json').write_text(json.dumps({str(p.relative_to(snapshot)): sha(p) for p in dpvs.rglob('*') if p.is_file()}, indent=2) + '\n')
        env = compiler_environment(args.vcvars, 'x86')
        logs = out / 'baseline'
        logs.mkdir()
        execute([str(args.msbuild), str(project), '/t:Rebuild', '/m:2', '/v:normal', '/p:Configuration=Release', '/p:Platform=Win32', '/p:UserRootDir=' + str(out / 'empty-props') + '\\'], project.parent, logs / 'build.log', env)
        dll = snapshot / 'src/compile/win32/dpvs/Release/dpvs.dll'
        shutil.copy2(dll, logs / 'dpvs.dll')
        sys.path.insert(0, str(HERE))
        from compare_dll import compare
        report['baseline_comparison'] = compare(logs / 'dpvs.dll', out / 'Win32-Release/dpvs.dll')
        (out / 'dll-comparison.json').write_text(json.dumps(report['baseline_comparison'], indent=2) + '\n')
        require(report['baseline_comparison']['normalized_equal'], 'baseline DLL changed beyond enumerated metadata')
    (out / 'results.json').write_text(json.dumps(report, indent=2) + '\n')
    print('PASS: four fresh DLLs; 192 stress queries and 64 occlusion frames each; PC64 comparisons verified')


if __name__ == '__main__':
    try:
        main()
    except (RuntimeError, OSError, subprocess.SubprocessError) as error:
        print('FAIL: ' + str(error), file=sys.stderr)
        sys.exit(1)
