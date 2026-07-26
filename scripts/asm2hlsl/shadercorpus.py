# Extracts every vertex_program/*.vsh, pixel_program/*.psh and their include set out of
# the TRE stack, respecting search-tree precedence, and classifies each program by the
# language marker on its first line.
#
# Precedence matters: the same name appears in several TREs (543 .vsh records for 286
# unique names), and the client resolves through the searchTree order in live.cfg. Getting
# it backwards means classifying a superseded version of a program.

import os
import re
import sys
import collections

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = r"E:\SWG\64bit-server\_client"
sys.path.insert(0, HERE)
import tre as tremod

OUT = os.path.join(HERE, "corpus")

WANT = ("vertex_program/", "pixel_program/", "shared_program/", "effect/")


def search_order():
    """The engine's resolution order for searchTree_<sku>_<priority> entries.

    TreeFile keeps its node list sorted by priority DESCENDING and inserts each new node
    with std::lower_bound against a `a->priority > b->priority` comparator. lower_bound
    returns the first position whose priority is not greater than the new node's, so an
    equal-priority node is inserted BEFORE the ones already there -- the last tree added
    at a given priority is searched first. (The doc comment above addSearchNode says
    "inserted after the last priority match", which is the opposite of what the code
    does. The code is what runs, and last-added-wins is also the only rule under which a
    patch tree can patch anything.)

    The cfg loader adds entries priority 0 upward, each priority's entries in listed
    order. So the resolution order is: priority descending, and within a priority, the
    reverse of the listed order.
    """
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

    order = []
    seen = set()
    for priority in sorted(byPriority, reverse=True):
        for name in reversed(byPriority[priority]):
            if name not in seen:
                seen.add(name)
                order.append(name)
    return order


def main():
    order = search_order()
    print("search order (%d trees), first 8:" % len(order))
    for n in order[:8]:
        print("   ", n)

    resolved = {}          # name -> (tre, bytes)
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
            if not nm.startswith(WANT):
                continue
            if nm in resolved:
                continue          # an earlier tree already won
            resolved[nm] = (tre_name, tremod.extract(f, e, None))
        f.close()

    os.makedirs(OUT, exist_ok=True)
    for nm, (_tre, data) in resolved.items():
        target = os.path.join(OUT, nm.replace("/", os.sep))
        os.makedirs(os.path.dirname(target), exist_ok=True)
        open(target, "wb").write(data)

    kinds = collections.Counter()
    for nm in resolved:
        kinds[os.path.splitext(nm)[1]] += 1
    print("\nresolved unique files:", dict(kinds))

    # Classify programs by their first-line marker.
    marker = collections.Counter()
    unmarked = []
    targets = collections.Counter()

    for nm, (_tre, data) in sorted(resolved.items()):
        if not nm.endswith((".vsh", ".psh")):
            continue
        head = data[:200].decode("latin1", "replace")
        first = head.split("\n", 1)[0].strip()
        tok = first.split()
        kind = os.path.splitext(nm)[1]
        if tok and tok[0] == "//hlsl":
            marker[(kind, "hlsl")] += 1
            targets[(kind, "hlsl", tok[1] if len(tok) > 1 else "?")] += 1
        elif tok and tok[0] == "//asm":
            marker[(kind, "asm")] += 1
            targets[(kind, "asm", tok[1] if len(tok) > 1 else "?")] += 1
        else:
            marker[(kind, "unmarked")] += 1
            unmarked.append((nm, first[:70]))

    print("\nlanguage marker by program type:")
    for key in sorted(marker):
        print("   %-6s %-9s %d" % (key[0], key[1], marker[key]))

    print("\ndeclared profiles:")
    for key in sorted(targets):
        print("   %-6s %-5s %-8s %d" % (key[0], key[1], key[2], targets[key]))

    if unmarked:
        print("\nunmarked (%d), first 10:" % len(unmarked))
        for nm, first in unmarked[:10]:
            print("   %-46s %s" % (nm, first))

    print("\nextracted to", OUT)


if __name__ == "__main__":
    main()
