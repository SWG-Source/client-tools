# Reachability over the WHOLE shader set, not just the effects.
#
# The first pass scanned effect/*.eft only, and undercounted: some .sht files carry their
# shader implementation inline instead of naming an .eft. shader/2d_bloom.sht is one --
# 725 bytes, no .eft reference anywhere in it -- so its pixel program looked unreachable
# when it is in fact what Bloom::install fetches by name.
#
# So this resolves every shader/*.sht and every effect/*.eft through search-tree
# precedence, finds every implementation FORM carrying a SCAP chunk in either, and
# collects the vertex and pixel programs referenced by the ones live at capability 2.0.

import collections
import io
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8", errors="replace")
import tre as tremod

ROOT = r"E:\SWG\64bit-server\_client"

TARGET = (2 << 8) | 0
REMAP_OLD = {0x0100: 0x0002, 0x0105: 0x0003, 0x0200: 0x0101, 0x0205: 0x0104, 0x0300: 0x0200}
REMAP_RECENT = {0x0002: 0x0002, 0x0003: 0x0003, 0x0100: 0x0101, 0x0104: 0x0104, 0x0200: 0x0200}
VERSION_TAGS = {b"%04d" % i for i in range(10)}

NAME_RE = re.compile(rb"[ -~]{4,}")
PROGRAM_RE = re.compile(r"[a-z0-9_/\.\-]+\.(?:vsh|psh)")


def search_order():
    entries = []
    for cfg in ("client.cfg", "live.cfg", "preload.cfg", "options.cfg", "user.cfg", "login.cfg"):
        path = os.path.join(ROOT, cfg)
        if not os.path.exists(path):
            continue
        for line in open(path, encoding="latin-1", errors="replace"):
            m = re.match(r"\s*searchTree_(\d+)_(\d+)\s*=\s*(\S+)", line)
            if m:
                entries.append((int(m.group(2)), m.group(3)))
    byPriority = {}
    for priority, name in entries:
        byPriority.setdefault(priority, []).append(name)
    order, seen = [], set()
    for priority in sorted(byPriority, reverse=True):
        for name in reversed(byPriority[priority]):
            if name not in seen:
                seen.add(name)
                order.append(name)
    return order


def forms(data):
    out = []

    def walk(start, end):
        pos = start
        while pos + 8 <= end:
            tag = data[pos:pos + 4]
            length = struct.unpack_from(">I", data, pos + 4)[0]
            body = pos + 8
            stop = min(body + length, end)
            if tag == b"FORM" and body + 4 <= stop:
                out.append((data[body:body + 4], body + 4, stop))
                walk(body + 4, stop)
            pos = body + length
            if length == 0:
                break

    walk(0, len(data))
    return out


def scap(data, start, end):
    pos = start
    while pos + 8 <= end:
        tag = data[pos:pos + 4]
        length = struct.unpack_from(">I", data, pos + 4)[0]
        body = pos + 8
        if tag == b"SCAP":
            return list(struct.unpack_from("<%di" % (length // 4), data, body))
        pos = body + length
        if length == 0:
            break
    return None


def programs(data, start, end):
    found = set()
    for m in NAME_RE.finditer(data[start:end]):
        s = m.group().decode("latin1").lower().replace("\\", "/")
        for piece in PROGRAM_RE.findall(s):
            found.add(piece.lstrip("*"))
    return found


def main():
    order = search_order()

    resolved = {}
    for tre_name in order:
        path = os.path.join(ROOT, tre_name)
        if not os.path.exists(path):
            continue
        opened = tremod.read_tre(path)
        if not opened:
            continue
        f, _ver, entries = opened
        for e in entries:
            nm = e[0].lower().replace("\\", "/")
            if not (nm.startswith("shader/") and nm.endswith(".sht")) and not (nm.startswith("effect/") and nm.endswith(".eft")):
                continue
            if nm in resolved:
                continue
            resolved[nm] = tremod.extract(f, e, None)
        f.close()

    shts = sum(1 for k in resolved if k.endswith(".sht"))
    efts = sum(1 for k in resolved if k.endswith(".eft"))
    print("resolved %d shaders and %d effects" % (shts, efts))

    live = set()
    dead = set()
    inline_impl_shts = 0
    impls = 0
    impls_live = 0

    for nm, data in resolved.items():
        hasImpl = False
        for formType, body, stop in forms(data):
            if formType not in VERSION_TAGS:
                continue
            values = scap(data, body, stop)
            if values is None:
                continue
            hasImpl = True
            impls += 1
            version = int(formType)
            if version in (4, 5):
                mapped = [REMAP_OLD.get(v, 0) for v in values]
            elif version == 6:
                mapped = [REMAP_RECENT.get(v, 0) for v in values]
            else:
                mapped = list(values)
            isLive = TARGET in mapped
            if isLive:
                impls_live += 1

            for p in programs(data, body, stop):
                (live if isLive else dead).add(p)
        if hasImpl and nm.endswith(".sht"):
            inline_impl_shts += 1

    print("shaders carrying an inline implementation: %d" % inline_impl_shts)
    print("implementations seen: %d, live at 2.0: %d" % (impls, impls_live))
    print()
    print("programs reachable at 2.0:            %d" % len(live))
    print("programs only in dead implementations: %d" % len(dead - live))

    asm = set()
    p = os.path.join(HERE, "asm-programs.txt")
    for line in open(p, encoding="utf-8"):
        s = line.strip().replace("\\", "/").lower()
        if s:
            asm.add(s)

    asm_live = sorted(x for x in asm if x in live)
    print()
    print("assembly programs in corpus:              %d" % len(asm))
    print("assembly programs REACHABLE at 2.0:       %d" % len(asm_live))
    print("assembly programs never reached:          %d" % (len(asm) - len(asm_live)))

    vsh = [x for x in asm_live if x.endswith(".vsh")]
    psh = [x for x in asm_live if x.endswith(".psh")]
    print("   vertex: %d   pixel: %d" % (len(vsh), len(psh)))

    with open(os.path.join(HERE, "asm-reachable.txt"), "w", encoding="utf-8") as out:
        for x in asm_live:
            out.write(x + "\n")
    with open(os.path.join(HERE, "reachable-programs.txt"), "w", encoding="utf-8") as out:
        for x in sorted(live):
            out.write(x + "\n")

    # Did the eight fxc failures matter after all?
    failures = os.path.join(HERE, "fxc-failures.txt")
    if os.path.exists(failures):
        print("\nthe fxc failures, re-checked against the full reachable set:")
        for line in open(failures, encoding="utf-8"):
            rel = line.split("\t")[0].strip()
            print("   %-52s %s" % (rel, "REACHABLE" if rel in live else "not reached"))


if __name__ == "__main__":
    main()
