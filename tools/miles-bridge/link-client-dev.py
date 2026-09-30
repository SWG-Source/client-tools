#!/usr/bin/env python3
"""Relink the existing Debug-x64 game with development Miles objects on Windows.

This reuses baseline engine libraries. It is neither a fresh whole-source build
nor runtime qualification. No provider substitutions, /FORCE, or stub inputs.
"""
import argparse
from collections import Counter
from datetime import datetime, timezone
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys

_build_spec = importlib.util.spec_from_file_location('miles_bridge_build', Path(__file__).with_name('build.py'))
bridge_build = importlib.util.module_from_spec(_build_spec)
_build_spec.loader.exec_module(bridge_build)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def tokens(text):
    if text.count('"') % 2:
        raise ValueError('Unbalanced response-file quotes')
    return [s.replace('"', '') for s in re.findall(r'(?:[^\s"]|"[^"]*")+', text)]


def extract(log):
    lines = log.splitlines()
    matches = [i for i, line in enumerate(lines)
               if re.search(r'link\.exe\s', line, re.I)
               and re.search(r'/OUT:"?[^\r\n]*?SwgClient_d\.exe', line, re.I)]
    if len(matches) != 1:
        raise ValueError('Require exactly one actual SwgClient_d final link command')
    index = matches[0]
    line = re.sub(r'^\s*(?:\d+>)?\s*', '', lines[index])
    tool, args = re.split(r'link\.exe\s+', line, maxsplit=1, flags=re.I)
    tool = tool.strip('"') + 'link.exe'
    chunks = [args]
    for line in lines[index + 1:]:
        line = re.sub(r'^\s*(?:\d+>)?\s*', '', line)
        if not re.fullmatch(r'"[^"\r\n]+\.(?:obj|res)"\s*', line, re.I):
            break
        chunks.append(line)
    return tool, '\n'.join(chunks), index + 1


def machine(data):
    if len(data) < 20:
        raise ValueError('Truncated COFF member')
    return struct.unpack_from('<H', data, 6 if data[:4] == b'\0\0\xff\xff' else 0)[0]


def verify_type(path, archive):
    data = path.read_bytes()
    if not archive:
        if data.startswith(b'!<arch>\n') or machine(data) != 0x8664:
            raise ValueError('Expected AMD64 COFF object: %s' % path)
        return 1
    if not data.startswith(b'!<arch>\n'):
        raise ValueError('Expected COFF archive: %s' % path)
    offset, count = 8, 0
    while offset < len(data):
        header = data[offset:offset + 60]
        if len(header) != 60 or header[58:60] != b'`\n':
            raise ValueError('Malformed archive header')
        size = int(header[48:58].strip())
        start = offset + 60
        if start + size > len(data):
            raise ValueError('Truncated archive member')
        name = header[:16].strip()
        if name not in (b'/', b'//', b'/SYM64/'):
            if machine(data[start:start + size]) != 0x8664:
                raise ValueError('Non-AMD64 archive member: %s' % path)
            count += 1
        offset = start + size + (size & 1)
    if not count or offset != len(data):
        raise ValueError('Empty or malformed archive')
    return count


def receipt_artifact(path, target):
    receipt = json.loads(path.read_text(encoding='utf-8'))
    if receipt.get('target') != target or receipt.get('arch') != 'x64' or receipt.get('exit_code') != 0:
        raise ValueError('Require successful %s x64 receipt: %s' % (target, path))
    steps = receipt.get('steps', [])
    if not steps or any(s.get('exit_code') != 0 for s in steps):
        raise ValueError('Receipt has incomplete/failed steps')
    compiles = [s['command'] for s in steps if isinstance(s.get('command'), list)
                and Path(s['command'][0]).name.lower() == 'cl.exe']
    if not compiles or any('/mtd' not in [v.lower() for v in cmd] for cmd in compiles):
        raise ValueError('All compile steps must use Debug /MTd')
    if target in ('audio-dev', 'engine-worker') and receipt.get('engine_configuration') != 'Debug|x64':
        raise ValueError('Require actual engine Debug|x64 configuration')
    if target == 'audio-dev' and not any('/DCLIENT_MILES_DEV_FACADE' in cmd for cmd in compiles):
        raise ValueError('Audio object lacks explicit development selection')
    outputs = receipt['artifacts'] if target == 'audio-dev' else [receipt['artifact']]
    if len(outputs) != (2 if target == 'audio-dev' else 1):
        raise ValueError('Unexpected number of built artifacts')
    artifacts, details = [], []
    for output in outputs:
        artifact = Path(output['path']).resolve()
        if artifact.suffix.lower() != ('.obj' if target == 'audio-dev' else '.lib'):
            raise ValueError('Wrong artifact extension')
        if digest(artifact) != output['sha256']:
            raise ValueError('Artifact SHA256 mismatch: %s' % artifact)
        count = verify_type(artifact, target != 'audio-dev')
        artifacts.append(artifact)
        details.append({'path': str(artifact), 'sha256': digest(artifact), 'amd64_coff_members': count})
    return artifacts, {'receipt': str(path), 'receipt_sha256': digest(path),
                       'artifacts': details, 'target': target}


def candidate(original, out, additions):
    args = tokens(original)
    upper = [arg.upper() for arg in args]
    if '/MACHINE:X64' not in upper or '/DEBUG' not in upper:
        raise ValueError('Require baseline Debug AMD64 link')
    for arg in args:
        if arg.startswith('@') or re.match(r'[-/]FORCE(?:\b|:)', arg, re.I):
            raise ValueError('Nested response files and /FORCE are forbidden')
        if re.search(r'(?:^|[\\/])[^\\/]*stub[^\\/]*\.(?:obj|lib)$', arg, re.I):
            raise ValueError('Stub input forbidden: %s' % arg)
        if arg.lower().endswith(('audio.obj', '-audio.obj')):
            raise ValueError('Baseline already has direct Audio object')
    outputs = {'OUT': 'SwgClient-dev.exe', 'PDB': 'SwgClient-dev.pdb',
               'IMPLIB': 'SwgClient-dev.lib', 'MAP': 'SwgClient-dev.map',
               'ILK': 'SwgClient-dev.ilk', 'MANIFESTFILE': 'SwgClient-dev.manifest',
               'PGD': 'SwgClient-dev.pgd'}
    seen = Counter()
    result = [str(path) for path in additions]
    for arg in args:
        key = arg[1:].split(':', 1)[0].upper() if arg.startswith('/') else ''
        if key in outputs:
            seen[key] += 1
            result.append('/%s:%s' % (key, out / outputs[key]))
        else:
            result.append(arg)
    if any(seen[k] != 1 for k in ('OUT', 'PDB', 'IMPLIB')) or any(n > 1 for n in seen.values()):
        raise ValueError('Missing/duplicate baseline output switches')
    # Other default output names follow /OUT; VS2013 has no /ILK switch.
    for key in ('MAP',):
        if not seen[key]:
            result.append('/%s:%s' % (key, out / outputs[key]))
    return result, {'baseline_arguments': len(args), 'candidate_arguments': len(result),
                    'baseline_explicit_libraries': sum(not a.startswith('/') and a.lower().endswith('.lib') for a in args),
                    'added_direct_objects': 2, 'added_archives': 2,
                    'removed_provider_inputs': 0}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('engine-root', 'baseline-log', 'audio-receipt', 'pipe-receipt', 'worker-receipt', 'out'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--cwd', type=Path, help='Actual SwgClient project directory; supports existing drive mappings')
    parser.add_argument('--vcvars', type=Path, default=Path(os.environ.get('ProgramFiles(x86)', r'C:\Program Files (x86)')) / 'Microsoft Visual Studio 12.0/VC/vcvarsall.bat')
    args = parser.parse_args()
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=False)
    record = {'qualification': 'development relink only; existing engine archives; no runtime qualification',
              'started_utc': datetime.now(timezone.utc).isoformat(), 'exit_code': 1, 'steps': []}
    try:
        if os.name != 'nt':
            raise ValueError('Run on native Windows with VS2013')
        root = args.engine_root.resolve()
        cwd = (args.cwd or root / 'src/game/client/application/SwgClient/build/win32').resolve()
        if not root.is_dir() or not (cwd / 'SwgClient.vcxproj').is_file():
            raise ValueError('Missing actual SwgClient project cwd')
        expected_cwd = root / 'src/game/client/application/SwgClient/build/win32'
        if not expected_cwd.samefile(cwd):
            raise ValueError('--cwd must identify this engine root’s SwgClient project')
        record.update(engine_root=str(root), cwd=str(cwd), baseline_log=str(args.baseline_log.resolve()), baseline_sha256=digest(args.baseline_log))
        additions, records = [], []
        for path, target in ((args.audio_receipt, 'audio-dev'), (args.pipe_receipt, 'pipe'), (args.worker_receipt, 'engine-worker')):
            artifact, details = receipt_artifact(path.resolve(), target)
            additions.extend(artifact)
            records.append(details)
        record['inputs'] = records
        linker, original, line = extract(args.baseline_log.read_text(encoding='utf-8-sig', errors='replace'))
        if not Path(linker).is_file():
            raise ValueError('Original linker not found: %s' % linker)
        (out / 'original.rsp').write_text(original + '\n', encoding='utf-8')
        arguments, counts = candidate(original, out, additions)
        record.update(counts=counts, baseline_command_line=line, linker=linker, link_arguments=arguments)
        # Response files avoid shell interpretation; quote every argument, rejecting
        # embedded quotes/newlines instead of attempting ambiguous escaping.
        if any(any(c in a for c in '\r\n"') for a in arguments):
            raise ValueError('Unsupported character in linker argument')
        rsp = out / 'candidate.rsp'
        rsp.write_text('\n'.join('"' + a + '"' for a in arguments) + '\n', encoding='utf-8')
        env = bridge_build.compiler_environment(args.vcvars.resolve(), 'x64', out, record)
        record['baseline_linker'] = linker
        linker = shutil.which('link.exe', path=env.get('PATH'))
        if not linker:
            raise ValueError('Missing linker in the matching VS2013 AMD64 environment')
        record['linker'] = linker
        command = [linker, '@' + str(rsp)]
        record['command'] = command
        with (out / 'link.log').open('wb') as log:
            result = subprocess.run(command, cwd=str(cwd), env=env, stdout=log, stderr=subprocess.STDOUT)
        text = (out / 'link.log').read_text(errors='replace')
        diagnostics = Counter(re.findall(r'\bLNK\d{4}\b', text))
        record.update(link_exit_code=result.returncode, diagnostic_counts=dict(diagnostics),
                      error_lines=[line for line in text.splitlines() if re.search(r':\s+(?:fatal\s+)?error\s+LNK\d{4}\b|\bLNK4088\b', line, re.I)])
        product = out / 'SwgClient-dev.exe'
        if result.returncode or record['error_lines'] or not product.is_file():
            raise RuntimeError('Development link failed; inspect link.log and diagnostic_counts')
        record['artifact'] = {'path': str(product), 'sha256': digest(product)}
        record['exit_code'] = 0
    except (Exception, KeyboardInterrupt) as error:
        record['error'] = str(error)
        print(str(error), file=sys.stderr)
    finally:
        record['finished_utc'] = datetime.now(timezone.utc).isoformat()
        (out / 'receipt.json').write_text(json.dumps(record, indent=2) + '\n', encoding='utf-8')
        print('Development relink receipt: %s' % (out / 'receipt.json'))
    return record['exit_code']


if __name__ == '__main__':
    sys.exit(main())
