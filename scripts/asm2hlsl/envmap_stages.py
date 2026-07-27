# Which pixel program sampler stages are fed a cube map.
#
# D3D9 assembly before ps_2_0 declared no sampler dimension: `tex t1` sampled whatever kind of
# texture happened to be bound. HLSL has to commit, and D3D11 rejects the read when the bound
# view disagrees with the declaration -- the draw does not render and the debug layer names a
# slot number with no program attached.
#
# The dimension is recoverable from the assets rather than guessed. An effect's pixel shader
# FORM carries one PTXM entry per sampler stage, holding the stage index and the texture tag
# that stage draws from. One of those tags is special: ENVM is not supplied by the material at
# all. ShaderPrimitiveSorter pushes it as a GLOBAL texture per cell --
#
#     Graphics::setGlobalTexture(TAG_ENVM, *ms_environmentTextures.back());
#
# -- defaulting to ms_defaultEnvironmentTexture, which is a cube map. So a stage tagged ENVM is
# always a cube map, everywhere, and every other stage is a 2D texture.
#
# Tags are stored byte-reversed on disk: ENVM appears as "MVNE", MAIN as "NIAM".
#
# Writes envmap-stages.tsv: one line per (pixel program, stage) that needs a cube declaration.

import io
import os
import sys
import collections

HERE = os.path.dirname(os.path.abspath(__file__))
CORPUS = os.path.join(HERE, "corpus")
OUT = os.path.join(HERE, "envmap-stages.tsv")

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8", errors="replace")

TAG_ENVM = b"MVNE"   # "ENVM" reversed


def walk(raw, start, end, onChunk, path):
    """Walk an IFF range, calling onChunk(name, version, data, path) for each leaf."""
    i = start
    while i + 8 <= end:
        name = raw[i:i + 4]
        length = int.from_bytes(raw[i + 4:i + 8], "big")
        body = i + 8
        stop = min(body + length, end)

        if name == b"FORM":
            if body + 4 <= stop:
                formType = raw[body:body + 4]
                walk(raw, body + 4, stop, onChunk, path + [formType])
        else:
            onChunk(name, raw[body:stop], path)

        i = body + length
        if length == 0:
            break


def scan(path):
    """Return {pixelProgramName: {stage: tag}} for one effect file."""
    raw = open(path, "rb").read()
    found = {}

    # A PTXM entry belongs to the PPSH subtree it sits in, so the walk keeps the enclosing
    # form types and the program name is whichever DATA came last inside this PPSH.
    state = {"program": None, "depthOfProgram": None}

    def onChunk(name, data, formPath):
        inPixelShader = b"PPSH" in formPath

        if name == b"DATA" and inPixelShader and data:
            # The payload is a one-byte texture-stage count followed by the program name, so the
            # name does not start at offset zero. Taking the whole payload picks up the count as
            # a leading control character and every later comparison silently misses.
            body = data[1:] if data[0] < 32 else data
            text = body.split(b"\0")[0].decode("latin1").replace("\\", "/")
            if text.endswith(".psh"):
                state["program"] = text
                found.setdefault(text, {})

        elif name == b"0002" and b"PTXM" in formPath and len(data) >= 5:
            program = state["program"]
            if program is None:
                return
            stage = data[0]
            tag = data[1:5]
            found.setdefault(program, {})[stage] = tag

    walk(raw, 0, len(raw), onChunk, [])
    return found


def main():
    effects = os.path.join(CORPUS, "effect")
    if not os.path.isdir(effects):
        raise SystemExit("no corpus/effect -- run shadercorpus.py first")

    # program -> stage -> set of tags seen, and which effects contributed
    stages = collections.defaultdict(lambda: collections.defaultdict(set))
    origins = collections.defaultdict(set)

    for f in sorted(os.listdir(effects)):
        if not f.endswith(".eft"):
            continue
        for program, byStage in scan(os.path.join(effects, f)).items():
            for stage, tag in byStage.items():
                stages[program][stage].add(tag)
                origins[(program, stage)].add(f)

    cubeStages = []
    conflicts = []

    for program in sorted(stages):
        for stage in sorted(stages[program]):
            tags = stages[program][stage]
            if TAG_ENVM not in tags:
                continue

            cubeStages.append((program, stage, origins[(program, stage)]))

            if len(tags) > 1:
                # The same stage is ENVM under one effect and something else under another, so one
                # declaration cannot be right for both and a fully correct answer needs a compiled
                # variant per dimension.
                #
                # Resolved as a cube anyway, and recorded here rather than dropped. ENVM is the
                # only one of these tags the engine supplies itself -- ShaderPrimitiveSorter
                # pushes it globally per cell and its default is a cube map -- so it is the case
                # that is certainly a cube and certainly common. The other tag is material
                # supplied and its dimension is not knowable from the effects alone. Dropping the
                # stage instead would declare it 2D and lose every environment-mapped draw, which
                # is the worse of the two guesses.
                #
                # The backend checks the bound view against the declaration on every material
                # apply and names the program, so if a material ever binds a 2D texture here the
                # log says so in the first frame it happens.
                conflicts.append((program, stage, tags, origins[(program, stage)]))

    with open(OUT, "w", encoding="utf-8", newline="\n") as out:
        out.write("# pixel program\tsampler stage\teffects\n")
        for program, stage, effectNames in cubeStages:
            out.write("%s\t%d\t%s\n" % (program, stage, ",".join(sorted(effectNames))))

    print("effects scanned      : %d" % len([f for f in os.listdir(effects) if f.endswith(".eft")]))
    print("pixel programs seen  : %d" % len(stages))
    print("cube (ENVM) stages   : %d" % len(cubeStages))
    for program, stage, effectNames in cubeStages:
        print("   %-44s stage %d   from %s" % (program, stage, ",".join(sorted(effectNames))[:70]))

    if conflicts:
        print()
        print("AMBIGUOUS -- a stage is ENVM under one effect and another tag under another.")
        print("Resolved as a cube map, because ENVM is the engine-supplied global and is certainly")
        print("one; the other tag is material supplied and not knowable from the effects. The")
        print("backend checks the bound view against the declaration and names the program, so a")
        print("2D texture arriving here is reported in the frame it happens.")
        for program, stage, tags, effectNames in conflicts:
            print("   %-44s stage %d   tags %s   from %s" % (
                program, stage,
                ",".join(sorted(t.decode("latin1")[::-1] for t in tags)),
                ",".join(sorted(effectNames))[:60]))

    return 0


if __name__ == "__main__":
    sys.exit(main())
