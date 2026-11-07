#!/usr/bin/env python3
"""Add bounded Debug/Release x64 configurations; retain Win32 text verbatim."""
import argparse
import os
from pathlib import Path
import re
import subprocess
import xml.etree.ElementTree as ET

NS = {'m': 'http://schemas.microsoft.com/developer/msbuild/2003'}
ROOT = Path(__file__).resolve().parents[2]
SLN = ROOT / 'src/build/win32/swg.sln'
MARKER = '<!-- configure-client-x64: generated -->'


def resolve_case(path):
    parts = path.parts
    result = Path(parts[0])
    for part in parts[1:]:
        if part == '..':
            result = result.parent
        elif part != '.':
            matches = [p for p in result.iterdir() if p.name.casefold() == part.casefold()]
            if len(matches) != 1:
                raise ValueError('Missing or ambiguous Windows path: ' + str(path))
            result = matches[0]
    return result


def closure(text):
    projects = {}
    for match in re.finditer(r'Project\("[^\"]+"\) = "([^\"]+)", "([^\"]+\.vcxproj)", "([^\"]+)"(.*?)EndProject', text, re.S):
        name, relative, guid, body = match.groups()
        deps = re.findall(r'^\s*({[^}]+}) = {[^}]+}', body, re.M)
        projects[guid.upper()] = (name, resolve_case(SLN.parent / relative.replace('\\', '/')), deps)
    todo = [g for g, row in projects.items() if row[0] == 'SwgClient']
    if len(todo) != 1:
        raise ValueError('Expected one SwgClient project')
    selected = {}
    while todo:
        guid = todo.pop().upper()
        if guid not in selected:
            selected[guid] = projects[guid]
            todo.extend(selected[guid][2])
    return selected


def transform_project(raw, path):
    text = raw.decode('utf-8-sig')
    newline = '\r\n' if '\r\n' in text else '\n'
    text = text.replace('\r\n', '\n')
    if MARKER in text:
        raise ValueError("--base must precede generated client x64 configurations")
    existing = set(re.findall(r'<ProjectConfiguration Include="([^"]+)"', text))
    if {'Debug|x64', 'Release|x64'} <= existing:
        return raw  # DPVS already has reviewed independent x64 configurations.
    if any(c.endswith('|x64') for c in existing):
        raise ValueError('Partial x64 configuration: ' + str(path))
    configs = re.findall(r'    <ProjectConfiguration Include="(?:Debug|Release)\|Win32">.*?</ProjectConfiguration>', text, re.S)
    if len(configs) != 2:
        raise ValueError('Expected Debug and Release: ' + str(path))
    text = text.replace('  </ItemGroup>', '\n'.join(c.replace('|Win32', '|x64').replace('<Platform>Win32</Platform>', '<Platform>x64</Platform>') for c in configs) + '\n  </ItemGroup>', 1)
    pattern = re.compile(r'<(?P<tag>\w+)\b(?P<attrs>[^>]*\bCondition="[^\"]*\$\(Configuration\)\|\$\(Platform\)[^\"]*(?:Debug|Release)\|Win32[^\"]*"[^>]*)(?:/>|>.*?</(?P=tag)>)', re.S)
    def clone(match):
        original = match[0]
        added = original.replace('|Win32', '|x64').replace('compile\\win32\\', 'compile\\x64\\')
        added = re.sub(r'_USE_32BIT_TIME_T(?:=[^;<]*)?;?', '', added)
        added = added.replace('MachineX86', 'MachineX64')
        def deploy(command):
            destination = command[1].replace('win32', 'x64')
            directory = destination.rsplit('\\', 1)[0].rstrip('\\')
            return ('<Command>if not exist "' + directory + '\\" mkdir "' + directory + '"\n'
                    + 'if not exist "' + directory + '\\" exit /b 1\n'
                    + 'copy /Y "$(TargetPath)" "' + destination + '"</Command>')
        added = re.sub(r'<Command>copy \$\(TargetPath\) ([^<]*?dev\\+win32\\+[^<]+)</Command>', deploy, added)
        return original + '\n  ' + added
    text = pattern.sub(clone, text)
    relative = os.path.relpath(ROOT / 'tools/configure-client-x64/client-x64.props', path.parent).replace('/', '\\')
    target = '  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets" />'
    if text.count(target) != 1:
        raise ValueError('Expected Cpp.targets import: ' + str(path))
    text = text.replace(target, '  ' + MARKER + '\n  <Import Project="' + relative + '" Condition="\'$(Platform)\'==\'x64\'" />\n' + target)
    output = text.replace('\n', newline).encode('utf-8')
    return (b'\xef\xbb\xbf' if raw.startswith(b'\xef\xbb\xbf') else b'') + output


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--base', required=True, help='pre-generation Git commit; project configuration source of truth')
    parser.add_argument('--check', action='store_true', help='fail if generation would change files')
    args = parser.parse_args()
    base = subprocess.check_output(['git', 'rev-parse', args.base], cwd=ROOT, text=True).strip()
    def source(path):
        return subprocess.check_output(['git', 'show', base + ':' + path.relative_to(ROOT).as_posix()], cwd=ROOT)
    raw = source(SLN)
    text = raw.decode('utf-8-sig').replace('\r\n', '\n')
    projects = closure(text)
    edits = {}
    for _, path, _ in projects.values():
        before = path.read_bytes()
        after = transform_project(source(path), path)
        if before != after:
            edits[path] = after
    configs = {}
    for guid, (_, path, _) in projects.items():
        tree = ET.fromstring(edits.get(path, path.read_bytes()))
        configs[guid] = {e.attrib['Include'] for e in tree.findall('.//m:ProjectConfiguration', NS)}
    lines = []
    for line in text.splitlines():
        lines.append(line)
        match = re.match(r'\s*({[^}]+})\.(Debug|Release)\|Win32\.(ActiveCfg|Build\.0) = (.*)\|Win32$', line)
        if match and match[1].upper() in projects:
            desired = match[4] + '|x64'
            if desired not in configs[match[1].upper()]:
                raise ValueError('Missing project mapping: ' + line)
            new = line.replace('|Win32', '|x64')
            if new not in text.splitlines():
                lines.append(new)
        elif line.strip() in ('Debug|Win32 = Debug|Win32', 'Release|Win32 = Release|Win32'):
            new = line.replace('Win32', 'x64')
            if new not in text.splitlines():
                lines.append(new)
    newline = '\r\n' if b'\r\n' in raw else '\n'
    encoded = (newline.join(lines) + newline).encode('utf-8')
    if raw.startswith(b'\xef\xbb\xbf'):
        encoded = b'\xef\xbb\xbf' + encoded
    if encoded != SLN.read_bytes():
        edits[SLN] = encoded
    for path, content in edits.items():
        print(path.relative_to(ROOT))
        if not args.check:
            path.write_bytes(content)
    print(f'{len(projects)} closure projects; {len(edits)} files requiring generation')
    return bool(edits) if args.check else 0


if __name__ == '__main__':
    raise SystemExit(main())
