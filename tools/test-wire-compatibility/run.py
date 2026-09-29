#!/usr/bin/env python3
"""Legacy 32-bit wire fixtures for the client, compiled as real Win32 and Win64 binaries.

Port of SWG-Source/src tools/test-wire-compatibility (src#35) to client-tools. Builds this
checkout's own Archive, AutoDelta, NetworkId, PlayerQuestData and MissionListResponse
serializers with clang in MSVC-compatibility mode against MinGW-w64 headers, links with
MinGW-w64 and runs the result under Wine. See README.md for scope and limits.
"""
import argparse, os, pathlib, re, shutil, subprocess, sys, tempfile

HERE = pathlib.Path(__file__).resolve().parent
EXPECTED_RUNTIME_PASSES = 40  # check() calls in fixtures.cpp; a run must report exactly these
TRIPLE = {32: 'i686-w64-mingw32', 64: 'x86_64-w64-mingw32'}

p = argparse.ArgumentParser()
p.add_argument('--bits', type=int, choices=[32, 64], required=True)
p.add_argument('--root', type=pathlib.Path, default=HERE.parents[1],
               help='client-tools checkout to test (default: this checkout)')
p.add_argument('--no-32bit-time', action='store_true',
               help='omit _USE_32BIT_TIME_T on Win32 (for checkouts whose projects no longer define it)')
p.add_argument('--types-only', action='store_true', help='run only the compile-time wire-width assertions')
p.add_argument('--keep', action='store_true', help='keep the temporary tree and print its path')
a = p.parse_args()

SOURCES = [
    'external/ours/library/archive/src/shared/ByteStream.cpp',
    'external/ours/library/archive/src/shared/AutoByteStream.cpp',
    'external/ours/library/archive/src/shared/AutoDeltaByteStream.cpp',
    'external/ours/library/archive/src/shared/AutoDeltaPackedMap.cpp',
    'external/ours/library/archive/src/win32/ArchiveMutex.cpp',
    'engine/shared/library/sharedFoundation/src/shared/NetworkId.cpp',
    'engine/shared/library/sharedFoundation/src/shared/NetworkIdArchive.cpp',
    'engine/shared/library/sharedGame/src/shared/quest/PlayerQuestData.cpp',
    'engine/shared/library/sharedNetworkMessages/src/shared/clientGameServer/MessageQueueMissionListResponseArchive.cpp',
    'engine/shared/library/sharedNetworkMessages/src/shared/clientGameServer/MessageQueueMissionListResponseData.cpp',
    'engine/shared/library/sharedNetworkMessages/src/shared/clientGameServer/MessageQueueMissionListResponseDataArchive.cpp',
    'external/ours/library/localizationArchive/src/shared/StringIdArchive.cpp',
    'external/ours/library/localization/src/shared/StringId.cpp',
    'external/ours/library/unicodeArchive/src/shared/UnicodeArchive.cpp',
    'external/ours/library/unicodeArchive/src/shared/UnicodeAutoDeltaPackedMap.cpp',
    'engine/shared/library/sharedUtility/src/shared/NetworkIdAutoDeltaPackedMap.cpp',
    'engine/shared/library/sharedNetworkMessages/src/shared/chat/ChatOnRequestLog.cpp',
    'engine/shared/library/sharedNetworkMessages/src/shared/clientGameServer/ImageDesignChangeMessage.cpp',
    'engine/shared/library/sharedNetworkMessages/src/shared/clientGameServer/BuffBuilderChangeMessage.cpp',
    'external/ours/library/unicode/src/shared/UnicodeUtils.cpp',
    'external/ours/library/unicode/src/shared/utf8.cpp',
]
# Serializers some checkouts keep in a separate file (e.g. SWG-Source/client-tools#21).
OPTIONAL_SOURCES = [
    'engine/shared/library/sharedNetworkMessages/src/shared/chat/ChatLogEntryArchive.cpp',
    'engine/shared/library/sharedNetworkMessages/src/shared/chat/ChatLogEntry.cpp',
]
LIBS = ['engine/shared/library', 'external/ours/library', 'game/shared/library']


def edit(path, pattern, repl):
    if path.exists():
        path.write_text(re.sub(pattern, repl, path.read_text(), flags=re.M))


def copy_tree(src, dst):
    """Copy the shared libraries and apply syntax-only edits a conforming compiler needs.
    MSVC 2013 accepted these; none changes a type, value or serialized byte."""
    for lib in LIBS:
        shutil.copytree(src / lib, dst / lib, symlinks=True)
    boost = src / 'external/3rd/library/boost'
    if boost.is_dir():
        (dst / 'external/3rd/library').mkdir(parents=True, exist_ok=True)
        os.symlink(boost, dst / 'external/3rd/library/boost')
    arc = dst / 'external/ours/library/archive/src/shared'
    # explicit specializations need template<>
    edit(arc / 'AutoDeltaPackedMap.h',
         r'^(\s*)inline void (AutoDeltaPackedMap<(?:int|unsigned long)[^>]*>::)', r'\1template<> inline void \2')
    edit(dst / 'engine/shared/library/sharedGame/src/shared/quest/PlayerQuestData.cpp',
         r'^(\s*)void (Archive::AutoDeltaPackedMap<uint32,\s*PlayerQuestData>::)', r'\1template<> void \2')
    for f in ['engine/shared/library/sharedFoundation/src/shared/AutoDeltaNetworkIdPackedMap.h',
              'engine/shared/library/sharedUtility/src/shared/NetworkIdAutoDeltaPackedMap.cpp',
              'external/ours/library/unicodeArchive/src/shared/UnicodeAutoDeltaPackedMap.cpp']:
        edit(dst / f, r'^(\s*)(?!template)((?:inline )?void AutoDeltaPackedMap<[^>]*>::(?:un)?pack\()', r'\1template<> \2')
    # dependent names need typename
    edit(arc / 'AutoDeltaSet.h', r'^(\s*)SetType::const_iterator i\(', r'\1typename SetType::const_iterator i(')
    # MSVC looked up friend parameter types in the befriended namespace
    edit(dst / 'engine/shared/library/sharedGame/src/shared/quest/PlayerQuestData.h',
         r'friend void Archive::(get|put)\((ReadIterator|ByteStream) &', r'friend void Archive::\1(Archive::\2 &')
    # the C runtime already declares finite()
    edit(dst / 'engine/shared/library/sharedFoundation/src/win32/PlatformGlue.h',
         r'^int\s+finite\(double value\);', '// test harness: finite() comes from the C runtime')
    # type shims (see shim/): every path to these headers resolves to the shim
    found = dst / 'engine/shared/library/sharedFoundation/src/shared'
    shutil.copy(HERE / 'shim/sharedFoundation/FirstSharedFoundation.h', found / 'FirstSharedFoundation.h')
    shutil.copy(HERE / 'shim/StlForwardDeclaration.h', found / 'StlForwardDeclaration.h')
    # Windows include spellings are case-insensitive
    pub = dst / 'external/ours/library/unicodeArchive/include/public'
    if (pub / 'unicodeArchive').is_dir() and not (pub / 'UnicodeArchive').exists():
        os.symlink('unicodeArchive', pub / 'UnicodeArchive')


def flags(tree, bits):
    inc = ['-I' + str(HERE / 'shim')]
    for lib in LIBS:
        for d in sorted((tree / lib).iterdir()):
            for suffix in ('include/public', 'include', 'src/shared', 'src/win32'):
                if (d / suffix).is_dir():
                    inc.append('-I' + str(d / suffix))
    inc.append('-idirafter' + str(tree / 'external/3rd/library/boost'))
    time32 = ['-D_USE_32BIT_TIME_T=1'] if bits == 32 and not a.no_32bit_time else []
    return ['clang++', '--target=' + TRIPLE[bits], '-std=gnu++14',
            '-fms-compatibility', '-fms-compatibility-version=19.40', '-fms-extensions',
            '-fdelayed-template-parsing', '-Wno-everything',
            '-D__int64=long long', '-D__GNUC__=13', '-D__GNUC_MINOR__=2',
            # MinGW i686 silently defines _USE_32BIT_TIME_T; MSVC (VS2005+) does not
            '-D__MINGW_USE_VC2005_COMPAT=1',
            '-D__GCC_ATOMIC_TEST_AND_SET_TRUEVAL=1',
            '-include', 'cstring', '-include', 'unordered_map', '-include', str(HERE / 'shim/hashmap_shim.h'),
            '-DWIN32=1', '-D_WIN32=1', '-D_USING_STL=1', '-DDEBUG_LEVEL=0', '-DPRODUCTION=1',
            '-DWIRE_TEST_MISSIONS=1', *time32, '-O1', *inc]


def main():
    tmp = pathlib.Path(tempfile.mkdtemp(prefix='swg-wire-'))
    try:
        tree = tmp / 'src'
        copy_tree(a.root.resolve() / 'src', tree)
        cxx = flags(tree, a.bits)
        r = subprocess.run(cxx + ['-fsyntax-only', str(HERE / 'wire_types.cpp')], capture_output=True, text=True)
        errors = [l.split('error: ', 1)[1] for l in r.stderr.splitlines() if 'error: ' in l]
        if r.returncode and not errors:
            errors = ['wire_types.cpp did not compile: ' + (r.stderr.strip().splitlines() or ['no diagnostics'])[-1]]
        types_ok = r.returncode == 0 and not errors
        print('PASS: time fields are 4 bytes on the wire' if types_ok else 'FAIL: ' + '\nFAIL: '.join(errors))
        if a.types_only:
            return 0 if types_ok else 1
        objs = []
        for i, src in enumerate([HERE / 'fixtures.cpp', HERE / 'fatal.cpp', HERE / 'mission_glue.cpp'] + [tree / s for s in SOURCES] + [tree / s for s in OPTIONAL_SOURCES if (tree / s).exists()]):
            obj = tmp / f'{i}.o'
            r = subprocess.run(cxx + ['-c', str(src), '-o', str(obj)], capture_output=True, text=True)
            if r.returncode:
                sys.stderr.write(r.stderr[-3000:])
                return 2
            objs.append(str(obj))
        exe = tmp / 'fixtures.exe'
        r = subprocess.run([TRIPLE[a.bits] + '-g++', '-static', *objs, '-o', str(exe)], capture_output=True, text=True)
        if r.returncode:
            sys.stderr.write(r.stderr[-3000:])
            return 2
        env = dict(os.environ, WINEDEBUG='-all', WINEARCH='win32' if a.bits == 32 else 'win64',
                   WINEPREFIX=os.environ.get(f'WINEPREFIX{a.bits}', str(pathlib.Path.home() / f'.wine-swg{a.bits}')))
        run = subprocess.run(['wine', str(exe)], env=env, capture_output=True, text=True)
        sys.stdout.write(run.stdout)
        sys.stderr.write(run.stderr)
        lines = run.stdout.splitlines()
        passes = sum(l.startswith('PASS: ') for l in lines)
        problems = [l for l in lines if l.startswith(('FAIL: ', 'NOT RUN'))]
        # Success needs a clean exit AND every expected check reported as passing: an exit
        # code alone cannot tell "all passed" from "the fixtures never ran".
        # Out-of-range time_t must be rejected by toWireTime (FATAL aborts), one process per field.
        probes_expected = probes_passed = 0
        for field in ('image', 'buff', 'chat'):
            pr = subprocess.run(['wine', str(exe), '--probe', field], env=env, capture_output=True, text=True)
            if 'SKIP:' in pr.stdout:
                print(f'SKIP: {field} out-of-range time probe (time_t is 32-bit)')
                continue
            probes_expected += 1
            rejected = pr.returncode != 0 and 'does not fit the legacy signed 32-bit time field' in pr.stderr
            probes_passed += rejected
            print(('PASS: ' if rejected else 'FAIL: ') + f'{field} time 2147483648 is rejected, not truncated')
        passes += probes_passed
        expected = EXPECTED_RUNTIME_PASSES + probes_expected
        if probes_passed != probes_expected:
            problems.append('probe')
        if run.returncode == 0 and not problems and passes == expected and types_ok:
            print(f'OK: {passes + 1}/{expected + 1} checks passed')
            return 0
        print(f'NOT OK: exit={run.returncode} runtime passes={passes}/{expected} '
              f'problems={len(problems)} types={"ok" if types_ok else "failed"}')
        return run.returncode or 1
    finally:
        if a.keep:
            print('kept', tmp)
        else:
            shutil.rmtree(tmp, ignore_errors=True)


sys.exit(main())
