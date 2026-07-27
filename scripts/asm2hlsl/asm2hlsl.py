# Direct3D 9 shader assembly -> HLSL, for the SWG shader corpus.
#
# D3DCompile has no assembler and D3D9 shader assembly is not a D3D11 shader model, so the
# assembly half of the corpus has to become HLSL for a D3D11 backend to render it at all.
# 97 programs are reachable at shader capability 2.0, and they draw on 15 shared modules.
#
# The translation is deliberately structural rather than clever:
#
#   * Vertex programs keep their modular shape. A converted module is a block of HLSL
#     statements that the converted program #includes inside main(), exactly as the assembly
#     did, so the output diffs against the input and the shared lighting code is translated
#     once. It also keeps the inherited c_ambient.inc patch a runtime include-handler
#     concern rather than baking it into 23 generated files.
#
#   * Constants become a flat float4 c[96] at register(c0). fxc places that at $Globals
#     offset 0 with size 1536, so element i sits at byte 16i -- byte-identical to the
#     register file the engine already uploads. registers.inc becomes #defines onto c[N],
#     so module bodies keep their symbolic names.
#
#   * Texture coordinate sets keep the runtime's indirection. The engine rewrites the
#     tag-to-set mapping per vertex buffer, so converted programs use the HLSL-form
#     textureCoordinateSetTAG names and DECLARE_textureCoordinateSets, and the header
#     #define lines are preserved so the runtime can still parse the tag list.
#
#   * c95 holds four literals that only assembly reads, preloaded by a D3DX
#     `#pragma def` the D3D11 path has no equivalent for. They are emitted as literals.
#
#   * cExtLtData_parallelSpec_0_* are the three symbols ILM_visuals.tre's diffuse.inc uses
#     and never defines. They map onto the engine's extendedLightData block at c60..c63,
#     whose HemisphericLightData fields are backColor, tangentColor, tangentMinusBackColor
#     and tangentMinusDiffuseColor in that order.

import collections
import io
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
CORPUS = os.path.join(HERE, "corpus")
OUT = os.path.join(HERE, "converted")

INCLUDE_RE = re.compile(r'^\s*#include\s+"([^"]+)"')
DEFINE_RE = re.compile(r'^\s*#define\s+(\S+)\s+(.*?)\s*$', re.M)


# ======================================================================
# Reading

def psrc(data):
    if data[:4] != b"FORM":
        return data
    i = data.find(b"PSRC")
    if i < 0:
        return None
    length = int.from_bytes(data[i + 4:i + 8], "big")
    return data[i + 8:i + 8 + length]


def read(rel):
    path = os.path.join(CORPUS, *rel.split("/"))
    if not os.path.exists(path):
        return None
    payload = psrc(open(path, "rb").read())
    if payload is None:
        return None
    # PSRC chunks carry a trailing NUL.
    return payload.decode("latin1", "replace").replace("\x00", "")


# ======================================================================
# Operand translation

SWIZZLE_RE = re.compile(r"^([a-zA-Z_][a-zA-Z0-9_]*)(?:\[([^\]]+)\])?(?:\.([a-zA-Z]+))?$")

# Source modifier suffixes, applied to the value.
SOURCE_MODIFIERS = {
    "_bx2":  lambda v: "((%s) * 2.0f - 1.0f)",
    "_bias": lambda v: "((%s) - 0.5f)",
    "_x2":   lambda v: "((%s) * 2.0f)",
}


class Operand(object):
    def __init__(self, text):
        self.raw = text.strip()
        self.negate = False
        self.complement = False
        self.modifier = None
        self.mask = None
        self.name = None
        self.index = None
        self.parse()

    def parse(self):
        t = self.raw
        if t.startswith("1-"):
            self.complement = True
            t = t[2:].strip()
        if t.startswith("-"):
            self.negate = True
            t = t[1:].strip()

        for suffix in ("_bx2", "_bias", "_x2"):
            # The modifier sits before any swizzle: t0_bx2, t0_bx2.a
            m = re.match(r"^([a-zA-Z_][a-zA-Z0-9_]*)" + suffix + r"(\..*)?$", t)
            if m:
                self.modifier = suffix
                t = m.group(1) + (m.group(2) or "")
                break

        m = SWIZZLE_RE.match(t)
        if not m:
            raise ValueError("cannot parse operand %r" % self.raw)
        self.name = m.group(1)
        self.index = m.group(2)
        self.mask = m.group(3)

    def value(self, mapper):
        base = mapper(self.name, self.index)
        if self.mask:
            base = "%s.%s" % (base, self.mask)
        if self.modifier:
            base = SOURCE_MODIFIERS[self.modifier](None) % base
        if self.negate:
            base = "-(%s)" % base
        if self.complement:
            base = "(1.0f - (%s))" % base
        return base

    def destination(self, mapper):
        base = mapper(self.name, self.index)
        return base, (self.mask or None)


# ======================================================================
# Emission helpers

# Whether the value being assigned is a genuine four-component vector. Set per instruction by the
# translators below.
#
# D3D9 replicates a single-component source swizzle across all four channels, so
# `mul r0.w, r11.w, cFog.w` multiplies two scalars and the result is uniform. Translated to HLSL
# that is a scalar expression, and the only subscript a scalar accepts is .x -- asking for .w is
# error X3018. But when a source is a whole register the result really is a four-vector whose
# components differ, and then the destination mask has to select the MATCHING component:
# `mov r0.a, c2` means r0.a = c2.a, not c2.x.
#
# Both cases used to be emitted as the leading components, which is right only for a mask starting
# at x. That is how every `mov r0.a, c[alphaFadeOpacity]` in the corpus came to read the dot3
# light's specular red instead: disassembling the shipped PEXE for a_simple.psh shows the MOV's
# source is 0xA0FF0002, c2 with swizzle .wwww.
SOURCE_IS_WIDE = [False]

# Symbolic constants whose registers.inc definition already names a single component, so an operand
# that carries no swizzle of its own is STILL a scalar. cMaterial_specularPower is c15.x and cLog2e
# is a bare literal; `mov r0.w, cMaterial_specularPower` reads as unswizzled but cannot take a .w.
# Filled by the driver when it generates asm_constants.inc, which is the only place that knows.
SCALAR_CONSTANTS = set()


def note_source_width(sources):
    """A four-vector result needs a component-matched swizzle; a scalar result cannot take one.

    ANY four-vector source makes the whole expression a four-vector, because HLSL promotes the
    scalar: `t0.a * psC[2]` is a float4 whose components differ, so writing it into r0.a has to take
    .a. Requiring every source to be wide would get that one wrong -- it was tried, and it left 29
    programs reading psC[2].x where the assembly means psC[2].w.

    A source is narrow when it carries its own single-component swizzle, or when it is a symbolic
    constant whose definition already names one component (SCALAR_CONSTANTS). With every source
    narrow the result is a scalar, and .x is then both correct and the only legal subscript --
    `mul r0.w, cLog2e, r0.w` cannot take .w.
    """
    wide = False
    for operand in sources:
        narrow = (operand.mask is not None and len(operand.mask) == 1) or operand.name in SCALAR_CONSTANTS
        if not narrow:
            wide = True
            break
    SOURCE_IS_WIDE[0] = wide


def assign(dst, mask, expression, saturate=False, scale=None):
    if scale is not None:
        expression = "(%s) * %s" % (expression, scale)
    if saturate:
        expression = "saturate(%s)" % expression

    # A texture coordinate output is a full register in assembly, but the input side is
    # whatever width DECLARE_textureCoordinateSets gave it -- float2 for an ordinary set,
    # float4 for a DOT3 one -- and which it is depends on the runtime's key. asmTexcoord is
    # an overload set that widens 2 and 3 component sources and passes 4 through, so the
    # generated code does not have to know.
    if dst.startswith("vsOutput.texcoord") and not mask:
        expression = "asmTexcoord(%s)" % expression
    if mask:
        # Assigning into a masked destination: HLSL requires the right-hand side to have the same
        # component count, so the source is selected down with a swizzle.
        #
        # The swizzle is the DESTINATION MASK'S OWN COMPONENTS, not the leading ones. D3D9 writes
        # component-wise: destination .w takes source .w, destination .y takes source .y. This used
        # to emit "xyzw"[:n], the first n components, which is right only when the mask happens to
        # start at x -- so .rgb and .r were correct and every other narrow mask was wrong.
        #
        # `mov r0.a, c[alphaFadeOpacity]` is the case that showed it. alphaFadeOpacity is
        # packedRegister2.a, so the assembly means r0.a = c2.a; the emitter produced r0.a = c2.x,
        # which is the dot3 light's specular RED. Disassembling the shipped PEXE for a_simple.psh
        # settles the intent: the MOV's source is 0xA0FF0002, c2 with swizzle 0xFF = .wwww. 59 of
        # the 127 converted pixel programs carry that line.
        #
        # It is alpha-only in practice, because the composite discards scene alpha, but it is wrong
        # and it does reach the epilogue's alpha-test clip.
        n = len(mask)
        if n == 4:
            return "\t%s = %s;" % (dst, expression)

        # A uniform result is the same in every channel, so the leading component is both correct
        # and the only legal subscript.
        selector = mask if SOURCE_IS_WIDE[0] else "xyzw"[:n]
        return "\t%s.%s = (%s).%s;" % (dst, mask, expression, selector)
    return "\t%s = %s;" % (dst, expression)


def broadcast(expression):
    """A dp3/dp4 result is a scalar that D3D9 replicates across the destination."""
    return "(%s).xxxx" % expression


# ======================================================================
# Vertex translation

VERTEX_OUTPUTS = {
    "opos": "vsOutput.position",
    "od0":  "vsOutput.color0",
    "od1":  "vsOutput.color1",
    "ofog": "vsOutput.fog",
    "opts": "vsOutput.pointSize",
}


def vertex_mapper(name, index):
    low = name.lower()
    if low in VERTEX_OUTPUTS:
        return VERTEX_OUTPUTS[low]
    m = re.match(r"^ot(\d)$", low)
    if m:
        return "vsOutput.texcoord%s" % m.group(1)
    if low == "ot":
        return "vsOutput.texcoord0"
    if name.startswith("textureCoordinateSet"):
        # DECLARE_textureCoordinateSets declares these as MEMBERS of the input struct, so a
        # bare macro expansion cannot reach them -- the HLSL programs write
        # inputVertex.textureCoordinateSetMAIN and let the runtime's macro turn the tag into a
        # set index. Same indirection here.
        return "vsInput." + name
    if re.match(r"^r\d+$", low):
        return low
    if re.match(r"^c\d+$", low):
        return "c[%d]" % int(low[1:])
    if low == "c" and index is not None:
        return "c[%s]" % index
    if re.match(r"^v\d+$", low):
        # Vertex inputs are reached through their symbolic names, which the converted
        # registers.inc defines. A bare vN here would mean registers.inc was bypassed.
        raise ValueError("raw vertex input register %s" % name)
    return name


def translate_vertex_instruction(op, operands, origin):
    note_source_width(operands[1:])
    saturate = op.endswith("_sat")
    if saturate:
        op = op[:-4]
    scale = None
    for suffix, factor in (("_x2", "2.0f"), ("_x4", "4.0f"), ("_d2", "0.5f")):
        if op.endswith(suffix):
            scale = factor
            op = op[:-len(suffix)]
            break

    dst, mask = operands[0].destination(vertex_mapper)
    src = [o.value(vertex_mapper) for o in operands[1:]]

    if op == "mov":
        return assign(dst, mask, src[0], saturate, scale)
    if op == "add":
        return assign(dst, mask, "%s + %s" % (src[0], src[1]), saturate, scale)
    if op == "sub":
        return assign(dst, mask, "%s - %s" % (src[0], src[1]), saturate, scale)
    if op == "mul":
        return assign(dst, mask, "%s * %s" % (src[0], src[1]), saturate, scale)
    if op == "mad":
        return assign(dst, mask, "%s * %s + %s" % (src[0], src[1], src[2]), saturate, scale)
    if op == "min":
        return assign(dst, mask, "min(%s, %s)" % (src[0], src[1]), saturate, scale)
    if op == "max":
        return assign(dst, mask, "max(%s, %s)" % (src[0], src[1]), saturate, scale)
    if op in ("dp3",):
        return assign(dst, mask, broadcast("dot((%s).xyz, (%s).xyz)" % (src[0], src[1])), saturate, scale)
    if op in ("dp4",):
        return assign(dst, mask, broadcast("dot(%s, %s)" % (src[0], src[1])), saturate, scale)
    if op == "rsq":
        # D3D9: 1/sqrt(abs(scalar)), replicated.
        return assign(dst, mask, broadcast("rsqrt(abs(%s))" % scalar_of(operands[1], vertex_mapper)), saturate, scale)
    if op == "rcp":
        return assign(dst, mask, broadcast("(1.0f / (%s))" % scalar_of(operands[1], vertex_mapper)), saturate, scale)
    if op == "exp":
        # D3D9 exp is 2^x, full precision, replicated.
        return assign(dst, mask, broadcast("exp2(%s)" % scalar_of(operands[1], vertex_mapper)), saturate, scale)
    if op == "log":
        return assign(dst, mask, broadcast("log2(max(abs(%s), 1e-30f))" % scalar_of(operands[1], vertex_mapper)), saturate, scale)
    if op == "lit":
        return assign(dst, mask, "lit((%s).x, (%s).y, (%s).w)" % (src[0], src[0], src[0]), saturate, scale)
    if op == "dst":
        # dst.x = 1, dst.y = src0.y * src1.y, dst.z = src0.z, dst.w = src1.w
        return assign(dst, mask, "float4(1.0f, (%s).y * (%s).y, (%s).z, (%s).w)" % (src[0], src[1], src[0], src[1]), saturate, scale)
    if op in ("m4x4", "m4x3", "m3x3"):
        return translate_matrix(op, operands, dst, mask, saturate, scale)

    raise ValueError("unhandled vertex opcode %r in %s" % (op, origin))


def scalar_of(operand, mapper):
    """A source used as a scalar. D3D9 replicate-scalar sources always name a channel; when
    they do not, x is the channel the assembler uses."""
    value = operand.value(mapper)
    if operand.mask and len(operand.mask) == 1:
        return value
    return "(%s).x" % value


# Symbolic constant name -> register number, populated by the driver from registers.inc. A
# matrix operand names only its FIRST row, so the rows after it have to be addressed
# numerically, which means the symbol has to be resolved here rather than left to the
# generated #defines.
CONSTANT_REGISTERS = {}


def translate_matrix(op, operands, dst, mask, saturate, scale):
    rows = {"m4x4": 4, "m4x3": 3, "m3x3": 3}[op]
    width = 3 if op == "m3x3" else 4

    base = operands[2]
    name = base.name

    rowExpressions = []
    if name in CONSTANT_REGISTERS:
        first = CONSTANT_REGISTERS[name]
        rowExpressions = ["c[%d]" % (first + i) for i in range(rows)]
    else:
        resolved = vertex_mapper(name, base.index)
        m = re.match(r"^c\[(\d+)\]$", resolved)
        if m:
            first = int(m.group(1))
            rowExpressions = ["c[%d]" % (first + i) for i in range(rows)]
        else:
            m = re.match(r"^r(\d+)$", resolved)
            if not m:
                raise ValueError("matrix source is neither a constant nor a temporary: %s" % base.raw)
            # A matrix built in temporaries, as dot3_3x2spec.inc does: the named register is
            # the first row and the rows after it follow consecutively.
            first = int(m.group(1))
            rowExpressions = ["r%d" % (first + i) for i in range(rows)]

    source = operands[1].value(vertex_mapper)
    parts = []
    for row in rowExpressions:
        if width == 3:
            parts.append("dot((%s).xyz, (%s).xyz)" % (source, row))
        else:
            parts.append("dot(%s, %s)" % (source, row))

    expression = "float%d(%s)" % (rows, ", ".join(parts))
    return assign(dst, mask, expression, saturate, scale)


# ======================================================================
# Pixel translation

# Literal constants a pixel program declares with `def`, as {register number: (x, y, z, w)}.
# Module scope for the same reason CONSTANT_REGISTERS is: pixel_mapper is called from the
# instruction translator, which has no route to per-program state. Cleared per program by the
# driver.
PIXEL_DEFS = {}


def pixel_mapper(name, index):
    low = name.lower()
    if re.match(r"^r\d+$", low):
        return low
    if re.match(r"^t\d+$", low):
        return low
    if re.match(r"^v0$", low):
        return "psInput.color0"
    if re.match(r"^v1$", low):
        return "psInput.color1"
    if low == "c" and index is not None:
        # c[symbolicName]. The name is NOT taken from the engine-layout
        # pixel_shader_constants.inc: that file defines these as partial swizzles
        # (packedRegister2.rgb) because it packs several meanings into one register, whereas
        # the assembly reads the WHOLE register -- `mul r1, r1, c[dot3LightSpecularColor]`
        # multiplies all four channels, alpha included. The generated asm_constants.inc binds
        # each name to its whole register instead, which is what the assembly meant.
        return index.strip()
    if re.match(r"^c\d+$", low):
        number = int(low[1:])
        # A `def cN, x, y, z, w` in the assembly declares an INLINE LITERAL that shadows the
        # engine's register N for this program only. Reading psC[N] instead reads whatever the
        # engine happens to have uploaded, which is a different number every frame and has
        # nothing to do with what the shader meant. bad_vertex_shader.psh defines c0 as magenta
        # and multiplies the vertex colour by it; without this it multiplied by a live engine
        # constant and came out an arbitrary flat colour.
        if number in PIXEL_DEFS:
            return "psDef%d" % number
        return "psC[%d]" % number
    return name


PIXEL_MASK_MAP = {"rgb": "rgb", "a": "a", "r": "r", "g": "g", "b": "b", "w": "a"}


def translate_pixel_instruction(op, operands, origin, state):
    note_source_width(operands[1:])
    saturate = op.endswith("_sat")
    if saturate:
        op = op[:-4]
    scale = None
    for suffix, factor in (("_x2", "2.0f"), ("_x4", "4.0f"), ("_d2", "0.5f")):
        if op.endswith(suffix):
            scale = factor
            op = op[:-len(suffix)]
            break

    if op == "tex":
        target = operands[0]
        n = int(re.sub(r"\D", "", target.name))
        state["samplers"].add(n)
        state["texcoords"].add(n)
        if n in state.get("cubeStages", ()):
            # A cube read takes a direction, not a pair. The assembly said nothing about which
            # this was -- ps_1_x had no sampler dimension and D3D9 took it from whatever texture
            # was bound -- so the kind comes from the effect's texture tag. See envmap_stages.py.
            return "\t%s = texCUBE(pixelSampler%d, psInput.texcoord%d.xyz);" % (pixel_mapper(target.name, None), n, n)
        return "\t%s = tex2D(pixelSampler%d, psInput.texcoord%d);" % (pixel_mapper(target.name, None), n, n)

    if op == "texcoord":
        target = operands[0]
        n = int(re.sub(r"\D", "", target.name))
        state["texcoords"].add(n)
        return "\t%s = float4(psInput.texcoord%d.xy, 0.0f, 1.0f);" % (pixel_mapper(target.name, None), n)

    if op == "texld":
        target = operands[0]
        n = int(re.sub(r"\D", "", target.name))
        state["samplers"].add(n)
        coord = operands[1].value(pixel_mapper)
        if n in state.get("cubeStages", ()):
            return "\t%s = texCUBE(pixelSampler%d, (%s).xyz);" % (pixel_mapper(target.name, None), n, coord)
        return "\t%s = tex2D(pixelSampler%d, (%s).xy);" % (pixel_mapper(target.name, None), n, coord)

    if op == "texm3x2pad":
        # u = dot(texcoord[N], src); held for the following texm3x2tex.
        target = operands[0]
        n = int(re.sub(r"\D", "", target.name))
        state["texcoords"].add(n)
        state["m3x2_u"] = True
        return "\ttexm3x2u = dot(psInput.texcoord%d.xyz, (%s).xyz);" % (n, operands[1].value(pixel_mapper))

    if op == "texm3x2tex":
        # v = dot(texcoord[M], src); then sample sampler M at (u, v).
        target = operands[0]
        n = int(re.sub(r"\D", "", target.name))
        state["samplers"].add(n)
        state["texcoords"].add(n)
        # A texm3x2 pair produces a 2D coordinate by construction, so this one cannot be a
        # cube read. Asserted rather than assumed: a cube-tagged stage reaching here means the
        # tag table and the assembly disagree about what the stage is, which is worth stopping
        # for rather than silently sampling the wrong thing.
        assert n not in state.get("cubeStages", ()), (
            "%s: stage %d is tagged as a cube map but is read through texm3x2tex, "
            "which produces a two-component coordinate" % (origin, n))
        return ("\ttexm3x2v = dot(psInput.texcoord%d.xyz, (%s).xyz);\n"
                "\t%s = tex2D(pixelSampler%d, float2(texm3x2u, texm3x2v));"
                % (n, operands[1].value(pixel_mapper), pixel_mapper(target.name, None), n))

    dst, mask = operands[0].destination(pixel_mapper)
    if mask:
        mask = PIXEL_MASK_MAP.get(mask, mask)
    src = [o.value(pixel_mapper) for o in operands[1:]]

    if op == "mov":
        return assign(dst, mask, src[0], saturate, scale)
    if op == "add":
        return assign(dst, mask, "%s + %s" % (src[0], src[1]), saturate, scale)
    if op == "sub":
        return assign(dst, mask, "%s - %s" % (src[0], src[1]), saturate, scale)
    if op == "mul":
        return assign(dst, mask, "%s * %s" % (src[0], src[1]), saturate, scale)
    if op == "mad":
        return assign(dst, mask, "%s * %s + %s" % (src[0], src[1], src[2]), saturate, scale)
    if op == "lrp":
        # D3D9: dst = src2 + src0 * (src1 - src2)
        return assign(dst, mask, "lerp(%s, %s, %s)" % (src[2], src[1], src[0]), saturate, scale)
    if op == "dp3":
        return assign(dst, mask, broadcast("dot((%s).xyz, (%s).xyz)" % (src[0], src[1])), saturate, scale)
    if op == "min":
        return assign(dst, mask, "min(%s, %s)" % (src[0], src[1]), saturate, scale)
    if op == "max":
        return assign(dst, mask, "max(%s, %s)" % (src[0], src[1]), saturate, scale)
    if op == "cnd":
        return assign(dst, mask, "((%s).a > 0.5f ? %s : %s)" % (src[0], src[1], src[2]), saturate, scale)

    raise ValueError("unhandled pixel opcode %r in %s" % (op, origin))


# ======================================================================

def masked_assign_fix(line):
    """HLSL will not take a float4 expression into a .rgb destination; the emitter already
    swizzles, this is just where that is documented."""
    return line
