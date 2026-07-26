# Driver for the assembly-to-HLSL conversion: walks the reachable programs, emits converted
# modules once each and converted programs that include them, wraps pixel programs back into
# the IFF container the engine's loader expects, and compiles every output with fxc.

import io
import os
import re
import shutil
import struct
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8", errors="replace")

import asm2hlsl as T

CORPUS = T.CORPUS
OUT = os.path.join(HERE, "converted")
FXC = r"C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\fxc.exe"

CONSTANTS_INC = "vertex_program/include/asm_constants.inc"

# ======================================================================
# The converted registers include: symbolic names onto a flat constant register file.

def build_constants_include():
    original = T.read("vertex_program/modules/registers.inc")
    if original is None:
        raise SystemExit("registers.inc missing from the corpus")

    lines = []
    lines.append("// Generated from vertex_program/modules/registers.inc by asm2hlsl.")
    lines.append("//")
    lines.append("// The symbolic names are unchanged so the converted modules read like the assembly")
    lines.append("// they came from. Constants resolve into a flat float4 c[96] at register(c0): fxc")
    lines.append("// places that at $Globals offset 0, size 1536, so element i is at byte 16i, which is")
    lines.append("// byte-identical to the register file the engine already uploads.")
    lines.append("")
    lines.append("float4 c[96] : register(c0);")
    lines.append("")
    lines.append("#define VERTEX_SHADER_VERSION 20")
    lines.append("")
    lines.append("// A texture coordinate output is a whole register in assembly, but the input side is")
    lines.append("// whatever width the runtime's DECLARE_textureCoordinateSets gave it: float2 for an")
    lines.append("// ordinary set and float4 for a DOT3 one, decided per vertex buffer. These overloads")
    lines.append("// let the generated code assign one to the other without knowing which it got.")
    lines.append("float4 asmTexcoord(float2 v) { return float4(v, 0.0f, 0.0f); }")
    lines.append("float4 asmTexcoord(float3 v) { return float4(v, 0.0f); }")
    lines.append("float4 asmTexcoord(float4 v) { return v; }")
    lines.append("")

    vertexInputs = {}
    T.CONSTANT_REGISTERS.clear()
    # The extended light data block: three of these are the symbols ILM's diffuse.inc uses
    # and never defines, mapped onto HemisphericLightData's field order at c60..c63.
    T.CONSTANT_REGISTERS["cExtLtData_parallelSpec_0_backColor"] = 60
    T.CONSTANT_REGISTERS["cExtLtData_parallelSpec_0_tangentColor"] = 61
    T.CONSTANT_REGISTERS["cExtLtData_parallelSpec_0_tangentMinusBack"] = 62
    T.CONSTANT_REGISTERS["cExtLtData_parallelSpec_0_tangentMinusDiffuse"] = 63

    for m in T.DEFINE_RE.finditer(original):
        name, value = m.group(1), m.group(2).strip()
        vm = re.match(r"^v(\d+)$", value)
        if vm:
            vertexInputs[name] = int(vm.group(1))
            continue

        cm = re.match(r"^c(\d+)(?:\.([xyzw]))?$", value)
        if not cm:
            continue
        index = int(cm.group(1))
        channel = cm.group(2)

        if index == 95:
            # Only assembly reads c95, preloaded by a D3DX #pragma def the D3D11 path has no
            # equivalent for. Emit the literals instead of depending on an upload.
            literal = {"x": "0.0f", "y": "0.5f", "z": "1.0f", "w": "1.4426950408889634f"}[channel]
            lines.append("#define %-46s %s" % (name, literal))
            continue

        if not channel:
            # A matrix operand names only its first row, so translate_matrix has to be able
            # to resolve the symbol to a number and step forward from it.
            T.CONSTANT_REGISTERS[name] = index

        target = "c[%d]" % index
        if channel:
            target += "." + channel
        lines.append("#define %-46s %s" % (name, target))

    lines.append("")
    lines.append("// The three symbols ILM_visuals.tre's diffuse.inc uses and never defines. They map")
    lines.append("// onto the engine's extendedLightData block at c60..c63, whose HemisphericLightData")
    lines.append("// fields are backColor, tangentColor, tangentMinusBackColor and")
    lines.append("// tangentMinusDiffuseColor in that order (Direct3d9_LightManager.h).")
    lines.append("#define %-46s %s" % ("cExtLtData_parallelSpec_0_backColor", "c[60]"))
    lines.append("#define %-46s %s" % ("cExtLtData_parallelSpec_0_tangentColor", "c[61]"))
    lines.append("#define %-46s %s" % ("cExtLtData_parallelSpec_0_tangentMinusBack", "c[62]"))
    lines.append("#define %-46s %s" % ("cExtLtData_parallelSpec_0_tangentMinusDiffuse", "c[63]"))
    lines.append("")

    return "\n".join(lines) + "\n", vertexInputs


PIXEL_CONSTANTS_INC = "pixel_program/include/asm_constants.inc"

# The six symbolic names the assembly pixel programs use, bound to WHOLE registers of the
# engine's PSCR layout. The engine-layout include cannot be reused here because it exposes
# these as partial swizzles.
PIXEL_CONSTANTS = """// Generated by asm2hlsl for the converted assembly pixel programs.
//
// Assembly reads a whole constant register. The engine-layout
// pixel_program/include/pixel_shader_constants.inc defines these same names as partial
// swizzles of packed registers, which is right for the HLSL programs written against it and
// wrong for assembly: `mul r1, r1, c[dot3LightSpecularColor]` multiplied all four channels.
// So the names are bound here to whole registers of the engine's PSCR layout instead.

float4 psC[8] : register(c0);
float4 psUserConstants[17] : register(c8);

#define dot3LightDirection      psC[0]
#define dot3LightDiffuseColor   psC[1]
#define dot3LightSpecularColor  psC[2]
#define alphaFadeOpacity        psC[2]
#define textureFactor           psC[5]
#define textureFactor2          psC[6]
#define materialSpecularColor   psC[7]
"""

VERTEX_INPUT_FIELD = {
    0:  ("float4", "position",  "POSITION0"),
    3:  ("float3", "normal",    "NORMAL0"),
    4:  ("float",  "pointSize", "PSIZE0"),
    5:  ("float4", "color0",    "COLOR0"),
    6:  ("float4", "color1",    "COLOR1"),
}


# ======================================================================

class Converter(object):
    def __init__(self):
        self.constantsInclude, self.vertexInputs = build_constants_include()
        self.modulesDone = {}
        self.errors = []

    # ------------------------------------------------------------------

    def convert_module(self, rel):
        """A converted module is a block of HLSL statements, textually included inside main()
        exactly as the assembly was."""
        if rel in self.modulesDone:
            return self.modulesDone[rel]

        text = T.read(rel)
        if text is None:
            self.errors.append((rel, "module missing from corpus"))
            self.modulesDone[rel] = None
            return None

        # Inherited from the DX9 x64 build: c_ambient.inc adds the global ambient, because
        # skinned meshes have no baked vColor0 and rendered black without it.
        if rel.endswith("c_ambient.inc"):
            text = text.replace("mov r7, vColor0", "add r7, vColor0, cLightData_ambient_ambientColor")

        out = ["// Generated from %s by asm2hlsl. Included inside main()." % rel, ""]
        nested = []
        ok = self.translate_body(rel, text, out, nested, vertex=True)
        self.modulesDone[rel] = (out, nested) if ok else None
        return self.modulesDone[rel]

    # ------------------------------------------------------------------

    def translate_body(self, origin, text, out, nested, vertex, state=None):
        ok = True
        for raw in text.splitlines():
            stripped = raw.strip()
            code = stripped.split("//")[0].split(";")[0].strip()

            m = T.INCLUDE_RE.match(raw)
            if m:
                inc = m.group(1).replace("\\", "/")
                if inc.startswith("../../"):
                    inc = inc[6:]
                # registers.inc was nothing but #defines; asm_constants.inc replaces it at
                # file scope, so including it inside main() would be an empty file.
                if inc.endswith("registers.inc"):
                    continue
                # texture_coordinates.inc was nothing but dcl_texcoordN declarations, which
                # DECLARE_textureCoordinateSets now covers in the input struct.
                if inc.endswith("texture_coordinates.inc"):
                    continue
                if inc not in nested:
                    nested.append(inc)
                out.append('#include "%s"' % self.converted_name(inc))
                continue

            if not code:
                if stripped.startswith("//"):
                    out.append(stripped)
                continue

            if code.startswith("#"):
                # Preprocessor lines pass through; HLSL has the same preprocessor.
                if T.DEFINE_RE.match(code):
                    continue
                out.append(code)
                continue

            if code == "+":
                # Co-issue marker: the paired instruction follows and writes a disjoint mask,
                # so the pairing carries no semantics worth preserving.
                continue

            low = code.lower()
            if low.startswith("dcl") or low in ("target",) or re.match(r"^(ps|vs)[\.\d]", low):
                continue
            if low.startswith("def "):
                continue

            # Assembly static branching, as the base diffuse.inc uses.
            bm = re.match(r"^if\s+(\S+)$", code)
            if bm:
                out.append("\tif (%s) {" % bm.group(1))
                continue
            if low == "endif":
                out.append("\t}")
                continue

            if code.startswith("+"):
                # A co-issue marker glued to the front of its instruction rather than sitting
                # on its own line. The pairing carries no semantics worth preserving: the two
                # instructions write disjoint masks.
                code = code[1:].strip()

            # Texture coordinate sets keep the runtime's indirection, but under the HLSL
            # spelling: the engine synthesises textureCoordinateSetTAG macros for an HLSL
            # program and vTextureCoordinateSetTAG for an assembly one.
            code = code.replace("vTextureCoordinateSet", "textureCoordinateSet")

            parts = code.split(None, 1)
            op = parts[0].lower()
            operandText = parts[1] if len(parts) > 1 else ""
            try:
                operands = [T.Operand(o) for o in operandText.split(",") if o.strip()]
                if vertex:
                    out.append(T.translate_vertex_instruction(op, operands, origin))
                else:
                    out.append(T.translate_pixel_instruction(op, operands, origin, state))
            except Exception as e:
                self.errors.append((origin, "%s   [%s]" % (e, code)))
                ok = False
        return ok

    # ------------------------------------------------------------------

    def converted_name(self, rel):
        return rel

    # ------------------------------------------------------------------

    def convert_vertex_program(self, rel):
        text = T.read(rel)
        if text is None:
            self.errors.append((rel, "program missing"))
            return None

        tags = []
        bodyLines = []
        inHeader = True
        declared = set()

        lines = text.splitlines(True)
        i = 0
        while i < len(lines):
            line = lines[i]
            token = line.strip().split()
            if inHeader:
                if not token:
                    i += 1
                    continue
                if token[0] in ("//asm", "//hlsl"):
                    i += 1
                    continue
                if token[0] == "#define" and len(token) > 1:
                    name = token[1]
                    if name.startswith("vTextureCoordinateSet") and not re.match(r"^vTextureCoordinateSet\d$", name):
                        tags.append(name[len("vTextureCoordinateSet"):])
                    while i < len(lines) and lines[i].rstrip("\r\n").endswith("\\"):
                        i += 1
                    i += 1
                    continue
                inHeader = False
            bodyLines.append(line)
            i += 1

        body = "".join(bodyLines)

        for m in re.finditer(r"^\s*dcl_(\w+)", body, re.M):
            declared.add(m.group(1).lower())

        out = []
        nested = []
        if not self.translate_body(rel, body, out, nested, vertex=True):
            return None

        for module in nested:
            self.convert_module(module)

        # Which outputs does the program write? Modules write outputs too -- fog.inc writes
        # oFog and transform.inc writes oPos -- so the scan has to cover the program's own
        # lines AND every module it pulls in, transitively. Scanning only the program leaves
        # those members out of the output struct, which then fails to compile on the
        # member reference rather than anywhere near the cause.
        scan = list(out)
        pending = list(nested)
        seenModules = set(nested)
        while pending:
            module = pending.pop()
            result = self.modulesDone.get(module)
            if not result:
                continue
            moduleLines, moduleNested = result
            scan.extend(moduleLines)
            for deeper in moduleNested:
                if deeper not in seenModules:
                    seenModules.add(deeper)
                    pending.append(deeper)

        written = set()
        for line in scan:
            for name, field in (("position", "position"), ("color0", "color0"), ("color1", "color1"),
                                ("fog", "fog"), ("pointSize", "pointSize")):
                if "vsOutput." + field in line:
                    written.add(field)
            for n in range(8):
                if "vsOutput.texcoord%d" % n in line:
                    written.add("texcoord%d" % n)

        text_out = []
        text_out.append("//hlsl vs_2_0")
        for index, tag in enumerate(tags):
            text_out.append("#define textureCoordinateSet%-8s textureCoordinateSet%d" % (tag, index))

        if tags:
            # The macro the input struct uses below, defined HERE rather than assumed to come
            # from somewhere.
            #
            # This is a convention of the hand-written HLSL programs: each one defines
            # DECLARE_textureCoordinateSets in its own source and then names it inside its input
            # struct. 166 of the corpus programs do exactly that. The original ASSEMBLY has no
            # such macro -- it declares inputs with dcl_texcoordN -- so a converted program that
            # names the macro without defining it does not compile, which is what 28 of these
            # did.
            #
            # float4 per set, not float2. Assembly treats a vertex input as a whole register, and
            # D3D11 fills components the input layout does not supply with 0 and w with 1, which
            # is what D3D9 did for an unwritten register component. No ": register(vN)" clause:
            # the backend strips those from vertex inputs anyway, and the set a tag resolves to
            # is decided by the input layout from the tag mapping, not by a register number.
            text_out.append("#define DECLARE_textureCoordinateSets\t\\")
            for index in range(len(tags)):
                terminator = "" if index == len(tags) - 1 else "\\"
                text_out.append("\tfloat4 textureCoordinateSet%d : TEXCOORD%d;%s" % (index, index, terminator))

        text_out.append("")
        text_out.append("// Converted from Direct3D 9 vertex assembly by asm2hlsl. The module structure of the")
        text_out.append("// original is preserved: each #include below is a block of statements inside main(),")
        text_out.append("// exactly as the assembly had it.")
        text_out.append("")
        text_out.append('#include "%s"' % CONSTANTS_INC)
        text_out.append("")

        text_out.append("struct VsInput")
        text_out.append("{")
        # Assembly declares its inputs with dcl_*; map those to the engine's semantics.
        wantPosition = any(d.startswith("position") for d in declared) or True
        if wantPosition:
            text_out.append("\tfloat4 position : POSITION0;")
        if any(d.startswith("normal") for d in declared):
            # float4, not float3. Assembly treats a vertex input as a whole register and some
            # programs swizzle all four components of the normal (mov r0, vNormal.yzxw). D3D11
            # fills components the input layout does not supply with 0, and w with 1, which is
            # the same thing D3D9 did for an unwritten register component.
            text_out.append("\tfloat4 normal : NORMAL0;")
        if any(d.startswith("color0") for d in declared) or "vColor0" in body:
            text_out.append("\tfloat4 color0 : COLOR0;")
        if any(d.startswith("color1") for d in declared) or "vColor1" in body:
            text_out.append("\tfloat4 color1 : COLOR1;")
        if tags:
            text_out.append("\tDECLARE_textureCoordinateSets")
        else:
            # A program with no tag defines addresses its texture coordinate sets by number
            # instead, through dcl_texcoordN -- cloudlayer.vsh is the one that does. There is
            # no tag indirection to preserve, so the members are declared directly.
            for m in sorted({int(x) for x in re.findall(r"dcl_texcoord(\d)", body)}):
                text_out.append("\tfloat4 textureCoordinateSet%d : TEXCOORD%d;" % (m, m))
        text_out.append("};")
        text_out.append("")

        text_out.append("struct VsOutput")
        text_out.append("{")
        text_out.append("\tfloat4 position : POSITION0;")
        if "color0" in written:
            text_out.append("\tfloat4 color0 : COLOR0;")
        if "color1" in written:
            text_out.append("\tfloat4 color1 : COLOR1;")
        if "fog" in written:
            text_out.append("\tfloat fog : FOG;")
        for n in range(8):
            if "texcoord%d" % n in written:
                text_out.append("\tfloat4 texcoord%d : TEXCOORD%d;" % (n, n))
        text_out.append("};")
        text_out.append("")

        text_out.append("// The assembly's symbolic input names.")
        text_out.append("#define vPosition vsInput.position")
        if any(d.startswith("normal") for d in declared):
            text_out.append("#define vNormal vsInput.normal")
        if any(d.startswith("color0") for d in declared) or "vColor0" in body:
            text_out.append("#define vColor0 vsInput.color0")
        if any(d.startswith("color1") for d in declared) or "vColor1" in body:
            text_out.append("#define vColor1 vsInput.color1")
        text_out.append("")

        text_out.append("VsOutput main(VsInput vsInput)")
        text_out.append("{")
        text_out.append("\tVsOutput vsOutput = (VsOutput)0;")
        text_out.append("\tfloat4 r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11;")
        text_out.append("\tr0 = r1 = r2 = r3 = r4 = r5 = r6 = r7 = r8 = r9 = r10 = r11 = 0.0f;")
        text_out.append("")
        text_out.extend(out)
        text_out.append("")
        text_out.append("\treturn vsOutput;")
        text_out.append("}")

        for module in nested:
            self.convert_module(module)

        return "\n".join(text_out) + "\n", nested

    # ------------------------------------------------------------------

    def convert_pixel_program(self, rel):
        text = T.read(rel)
        if text is None:
            self.errors.append((rel, "program missing"))
            return None

        state = {"samplers": set(), "texcoords": set(), "m3x2_u": False}
        out = []
        nested = []
        if not self.translate_body(rel, text, out, nested, vertex=False, state=state):
            return None

        usesColor0 = any("psInput.color0" in l for l in out)
        usesColor1 = any("psInput.color1" in l for l in out)
        temporaries = sorted({int(m.group(1)) for l in out for m in re.finditer(r"\br(\d+)\b", l)})
        textures = sorted({int(m.group(1)) for l in out for m in re.finditer(r"\bt(\d+)\b", l)})

        head = []
        head.append("//hlsl ps_2_0")
        head.append("")
        head.append("// Converted from Direct3D 9 pixel assembly by asm2hlsl. Constants keep the symbolic")
        head.append("// names the assembly used; the engine-layout pixel_shader_constants.inc defines every")
        head.append("// one of them, so they translate to themselves.")
        head.append("")
        head.append('#include "pixel_program/include/asm_constants.inc"')
        head.append("")
        for n in sorted(state["samplers"]):
            head.append("sampler pixelSampler%d : register(s%d);" % (n, n))
        head.append("")
        head.append("struct PsInput")
        head.append("{")
        if usesColor0:
            head.append("\tfloat4 color0 : COLOR0;")
        if usesColor1:
            head.append("\tfloat4 color1 : COLOR1;")
        for n in sorted(state["texcoords"]):
            head.append("\tfloat4 texcoord%d : TEXCOORD%d;" % (n, n))
        head.append("};")
        head.append("")
        head.append("float4 main(PsInput psInput) : COLOR")
        head.append("{")
        if temporaries:
            head.append("\tfloat4 %s;" % ", ".join("r%d" % n for n in temporaries))
            head.append("\t%s;" % " = ".join("r%d" % n for n in temporaries) + " = 0.0f" if False else
                        "\t" + " = ".join("r%d" % n for n in temporaries) + " = 0.0f;")
        if textures:
            head.append("\tfloat4 %s;" % ", ".join("t%d" % n for n in textures))
            head.append("\t" + " = ".join("t%d" % n for n in textures) + " = 0.0f;")
        if state["m3x2_u"]:
            head.append("\tfloat texm3x2u = 0.0f, texm3x2v = 0.0f;")
        head.append("")

        tail = ["", "\treturn r0;", "}"]
        return "\n".join(head + out + tail) + "\n", []


# ======================================================================
# The IFF container a converted pixel program has to be wrapped in.

def iff_chunk(tag, payload):
    return tag + struct.pack(">I", len(payload)) + payload


def write_pixel_program(path, hlsl):
    source = hlsl.encode("latin-1", "replace") + b"\x00"

    # PEXE is not optional in the loader, and getVersion() reads m_exe[0], so it carries the
    # ps_2_0 version token rather than being left empty.
    pexe = struct.pack("<I", 0xFFFF0200)

    inner = iff_chunk(b"PSRC", source) + iff_chunk(b"PEXE", pexe)
    form0000 = iff_chunk(b"FORM", b"0000" + inner)
    formPshp = iff_chunk(b"FORM", b"PSHP" + form0000)

    os.makedirs(os.path.dirname(path), exist_ok=True)
    open(path, "wb").write(formPshp)


# ======================================================================

def main():
    reachable = [l.strip().replace("\\", "/") for l in open(os.path.join(HERE, "asm-reachable.txt"), encoding="utf-8") if l.strip()]

    if os.path.exists(OUT):
        shutil.rmtree(OUT)
    os.makedirs(OUT)

    converter = Converter()

    # The generated constants include.
    p = os.path.join(OUT, *CONSTANTS_INC.split("/"))
    os.makedirs(os.path.dirname(p), exist_ok=True)
    open(p, "w", encoding="latin-1").write(converter.constantsInclude)

    p = os.path.join(OUT, *PIXEL_CONSTANTS_INC.split("/"))
    os.makedirs(os.path.dirname(p), exist_ok=True)
    open(p, "w", encoding="latin-1").write(PIXEL_CONSTANTS)

    vertexDone = 0
    pixelDone = 0

    for rel in sorted(reachable):
        if rel.endswith(".vsh"):
            result = converter.convert_vertex_program(rel)
            if result is None:
                continue
            hlsl, _nested = result
            target = os.path.join(OUT, *rel.split("/"))
            os.makedirs(os.path.dirname(target), exist_ok=True)
            open(target, "w", encoding="latin-1").write(hlsl)
            vertexDone += 1
        elif rel.endswith(".psh"):
            result = converter.convert_pixel_program(rel)
            if result is None:
                continue
            hlsl, _nested = result
            write_pixel_program(os.path.join(OUT, *rel.split("/")), hlsl)
            pixelDone += 1

    # Converted modules.
    moduleCount = 0
    for rel, result in converter.modulesDone.items():
        if result is None:
            continue
        out, _nested = result
        target = os.path.join(OUT, *rel.split("/"))
        os.makedirs(os.path.dirname(target), exist_ok=True)
        open(target, "w", encoding="latin-1").write("\n".join(out) + "\n")
        moduleCount += 1

    print("converted: %d vertex programs, %d pixel programs, %d modules" % (vertexDone, pixelDone, moduleCount))

    if converter.errors:
        print()
        print("translation errors: %d" % len(converter.errors))
        seen = {}
        for origin, message in converter.errors:
            seen.setdefault(message.split("[")[0].strip(), []).append(origin)
        for message, origins in sorted(seen.items(), key=lambda kv: -len(kv[1]))[:20]:
            print("   %-70s x%d  e.g. %s" % (message[:70], len(origins), origins[0]))


if __name__ == "__main__":
    main()
