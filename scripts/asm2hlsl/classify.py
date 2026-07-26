# Classifies the extracted corpus. .vsh files are raw text with the language marker on
# line one; .psh files are IFF-wrapped, with the text inside a PSRC chunk. Reports the
# language and declared profile split, which is what decides how much of the corpus a
# D3D11 backend can compile at all.

import collections
import io
import os
import re
import sys

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8", errors="replace")

HERE = os.path.dirname(os.path.abspath(__file__))
CORPUS = os.path.join(HERE, "corpus")


def psrc_text(data):
    """The PSRC chunk payload out of an IFF pixel program."""
    i = data.find(b"PSRC")
    if i < 0:
        return None
    # IFF chunk: 4-byte tag then big-endian 4-byte length, then payload.
    length = int.from_bytes(data[i + 4:i + 8], "big")
    return data[i + 8:i + 8 + length]


def first_line(text):
    return text.split(b"\n", 1)[0].decode("latin1", "replace").strip()


def main():
    marker = collections.Counter()
    profile = collections.Counter()
    asm_files = collections.defaultdict(list)

    for kind, subdir in ((".vsh", "vertex_program"), (".psh", "pixel_program")):
        root = os.path.join(CORPUS, subdir)
        for dirpath, _dirs, files in os.walk(root):
            for name in files:
                if not name.lower().endswith(kind):
                    continue
                path = os.path.join(dirpath, name)
                raw = open(path, "rb").read()

                text = raw
                if raw[:4] == b"FORM":
                    payload = psrc_text(raw)
                    if payload is None:
                        marker[(kind, "no-PSRC")] += 1
                        continue
                    text = payload

                line = first_line(text)
                tok = line.split()
                lang = tok[0] if tok else ""
                prof = tok[1] if len(tok) > 1 else "?"

                if lang == "//hlsl":
                    marker[(kind, "hlsl")] += 1
                    profile[(kind, "hlsl", prof)] += 1
                elif lang == "//asm":
                    marker[(kind, "asm")] += 1
                    profile[(kind, "asm", prof)] += 1
                    asm_files[kind].append(os.path.relpath(path, CORPUS))
                elif lang.startswith(("ps.", "vs.")):
                    # Bare D3D9 shader assembly: no //asm marker, just the version
                    # directive that opens an asm program. This is still assembly and
                    # D3DCompile cannot build it -- DX9 reaches these through
                    # D3DXAssembleShader or their precompiled PEXE blob.
                    marker[(kind, "asm")] += 1
                    profile[(kind, "asm", lang)] += 1
                    asm_files[kind].append(os.path.relpath(path, CORPUS))
                else:
                    marker[(kind, "unrecognised")] += 1
                    profile[(kind, "unrecognised", line[:30])] += 1

    print("language marker:")
    for key in sorted(marker):
        print("   %-5s %-13s %d" % (key[0], key[1], marker[key]))

    print("\ndeclared profile:")
    for key in sorted(profile):
        print("   %-5s %-13s %-10s %d" % (key[0], key[1], key[2], profile[key]))

    total_asm = sum(len(v) for v in asm_files.values())
    print("\nassembly programs that D3DCompile cannot build: %d" % total_asm)
    for kind in sorted(asm_files):
        print("  %s (%d), first 12:" % (kind, len(asm_files[kind])))
        for f in sorted(asm_files[kind])[:12]:
            print("     ", f)

    with open(os.path.join(HERE, "asm-programs.txt"), "w", encoding="utf-8") as out:
        for kind in sorted(asm_files):
            for f in sorted(asm_files[kind]):
                out.write(f + "\n")
    print("\nfull list written to asm-programs.txt")


if __name__ == "__main__":
    main()
