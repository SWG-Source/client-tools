#!/usr/bin/env python3
"""Recheck generated logs, exit codes and optional DLLs without running Windows."""
import argparse
import importlib.util
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent


def load(name):
    spec = importlib.util.spec_from_file_location(name, HERE / (name + '.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def verify(root):
    checks = load('run')
    def success(log):
        checks.require(log.with_suffix('.exit').read_text().strip() == '0', str(log) + ': exit not zero')
        return log.read_text(encoding='utf-8', errors='strict')
    report = {'provenance': json.loads((root / 'provenance.json').read_text()), 'runtime': [], 'numerical': {}}
    for arch in ('x86', 'amd64'):
        success(root / (arch + '-compiler.log'))
    for platform in ('Win32', 'x64'):
        for config in ('Release', 'Debug'):
            directory = root / (platform + '-' + config)
            success(directory / 'build.log')
            logs = {}
            for probe in ('stress', 'occlusion'):
                success(directory / (probe + '-build.log'))
                logs[probe] = success(directory / (probe + '.log'))
            item = {'platform': platform, 'configuration': config, 'build_exit': 0,
                    'probe_exits': [0, 0], 'dll_sha256': checks.sha(directory / 'dpvs.dll')}
            item.update(checks.check_runtime(logs['stress'], logs['occlusion']))
            report['runtime'].append(item)
    for probe in ('numerical', 'caller'):
        logs = []
        for platform in ('Win32', 'x64'):
            directory = root / (platform + '-Release')
            success(directory / (probe + '-build.log'))
            success(directory / (probe + '-link.log'))
            logs.append(success(directory / (probe + '.log')))
        report['numerical'][probe] = checks.compare_numeric(*logs, caller=probe == 'caller')
    if (root / 'baseline').exists():
        success(root / 'baseline/build.log')
        comparison = load('compare_dll').compare(root / 'baseline/dpvs.dll', root / 'Win32-Release/dpvs.dll')
        report['baseline_comparison'] = comparison
        checks.require(comparison['normalized_equal'], 'baseline DLL mismatch')
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('results', type=Path)
    args = parser.parse_args()
    try:
        print(json.dumps(verify(args.results), indent=2))
    except (RuntimeError, OSError, ValueError) as error:
        parser.exit(1, 'FAIL: {}\n'.format(error))
