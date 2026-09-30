#!/usr/bin/env python3
"""Compare generated project semantics with an explicit pre-generation Git ref."""
import argparse
import subprocess
import xml.etree.ElementTree as ET
import generate


def canonical(element):
    return (element.tag, tuple(sorted(element.attrib.items())), (element.text or '').strip(), tuple(canonical(c) for c in element))


def remove_x64(element):
    for child in list(element):
        condition = child.attrib.get('Condition', '')
        config = child.attrib.get('Include', '')
        if 'x64' in condition or (child.tag.endswith('ProjectConfiguration') and config.endswith('|x64')):
            element.remove(child)
        else:
            remove_x64(child)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--base', required=True, help='immutable pre-generation commit')
    args = parser.parse_args()
    base = subprocess.check_output(['git', 'rev-parse', args.base], cwd=generate.ROOT, text=True).strip()
    text = generate.SLN.read_text(encoding='utf-8-sig')
    projects = generate.closure(text)
    for name, path, _ in projects.values():
        relative = path.relative_to(generate.ROOT).as_posix()
        original = ET.fromstring(subprocess.check_output(['git', 'show', base + ':' + relative], cwd=generate.ROOT))
        current = ET.parse(path).getroot()
        for group in current:
            if '|x64' not in group.attrib.get('Condition', ''):
                continue
            for node in group.iter():
                if node.tag.split('}')[-1] in ('Command', 'Outputs', 'AdditionalInputs') and 'win32' in (node.text or '').lower():
                    raise ValueError('x64 build event refers to Win32: ' + name)
        remove_x64(original)
        remove_x64(current)
        if canonical(original) != canonical(current):
            raise ValueError('Win32 XML semantics changed: ' + name)
    original = subprocess.check_output(['git', 'show', base + ':src/build/win32/swg.sln'], cwd=generate.ROOT).decode('utf-8-sig')
    if [l for l in text.splitlines() if '|x64' not in l] != [l for l in original.splitlines() if '|x64' not in l]:
        raise ValueError('Existing solution mapping changed')
    if 'Optimized|x64' in text or 'IntelCPP|x64' in text:
        raise ValueError('Unsupported x64 solution configuration')
    print(f'PASS: {len(projects)} Win32 project XML trees and original solution mappings unchanged against {base}')


if __name__ == '__main__':
    main()
