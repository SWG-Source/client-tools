// ======================================================================
//
// Direct3d9_HlslRewrite.h
// Phase 11 D3D11 renderer plugin -- shared HLSL textual rewrite utility.
//
// Plan 11-07 Iteration 7 -- extracted from Direct3d11_CompileIncludeHandler
// (Iter-4 origin) so the same rewrite logic can run at two distinct call
// sites with different buffer-lifetime semantics:
//
//   1. Include-handler path (Iter-2/4): D3DCompile resolves a `#include`
//      directive through Direct3d11_CompileIncludeHandler::Open(), which
//      reads the included file via TreeFile and returns a heap-allocated
//      buffer to D3DCompile. The handler OWNS that heap buffer (it was
//      allocated by AbstractFile::readEntireFileAndClose() as
//      `new unsigned char[]`); D3DCompile's matching Close() callback
//      delete[]'s it. So the include-handler entry point either passes
//      the original buffer through unchanged (no rewrites needed --
//      fast path) or swaps in a freshly-allocated heap buffer and frees
//      the original. Close() releases whichever pointer was returned.
//
//   2. Main-source path (Iter-7 NEW): Direct3d11_VertexShaderData::
//      compileOrLoad receives a `(char const *, size_t)` source pair
//      from the engine -- the buffer is NOT owned by the plugin
//      (engine retains ownership via ShaderImplementationPassVertexShader::
//      m_text). The plugin must NOT free this buffer. The Iter-7 entry
//      point therefore emits the rewritten bytes into a caller-supplied
//      std::vector<char> with stack scope; D3DCompile is invoked
//      synchronously against the vector's storage, the vector goes out
//      of scope cleanly afterwards.
//
// Rewrite rules (case-sensitive throughout -- HLSL is case-sensitive;
// engine .vsh sources use ALL-CAPS for system semantics like POSITION,
// COLOR, SV_POSITION):
//
//   Rule A (Iter-4 origin -- SM4+ reserved-keyword whole-word rename):
//     Whole-word match of any of `point`, `line`, `triangle`, `lineadj`,
//     `triangleadj` becomes `<keyword>_id`. These are SM4+ geometry-
//     shader primitive-type lexer keywords; SWG's D3D9-era HLSL uses
//     them as ordinary identifier names. The `_id` suffix preserves
//     uniqueness without colliding with any existing identifier in the
//     engine HLSL corpus.
//
//   Rule B (Iter-7 origin; Iter-8 expanded `c` -> `[bcstu]`;
//   Iter-9 added `v` -> `[bcstuv]`; Iter-11 made context-aware):
//     Match `:\s*[bcstuv]\d+\b` (e.g., `: c4`, `:c12`, `: s0`, `: t3`,
//     `: b0`, `: u1`, `: v0`) becomes the same number of whitespace
//     characters (so the column offset on subsequent tokens is
//     preserved -- error messages with `(line,col)` still point at
//     the right place). Iter-11 context check: only fires when a
//     PREVIOUS `:` has already appeared on the SAME logical line
//     (the stacked-semantic struct-member shape). On the FIRST `:`
//     of a line (a legal global-declaration register binding), the
//     rule no-ops and the binding survives verbatim.
//
//   Rule C (Iter-7 origin; Iter-8 expanded `c` -> `[bcstu]`;
//   Iter-9 added `v` -> `[bcstuv]`; Iter-11 made context-aware):
//     Match `:\s*register\s*\(\s*[bcstuv]\d+\s*\)` (e.g.,
//     `: register(c4)`, `:register( s0 )`, `: register(t3)`,
//     `: register(v0)`) becomes the same number of whitespace
//     characters. Same column-preservation rationale as Rule B.
//     Same Iter-11 context check as Rule B.
//
//   Register-type letters covered: all 6 canonical D3D HLSL register
//   types per the MSDN HLSL `register()` grammar:
//     `b#` -- cbuffer / constant-buffer slot
//     `c#` -- single constant register (D3D9 legacy + D3D11 cbuffer-
//             member packoffset offset)
//     `s#` -- sampler slot
//     `t#` -- texture / SRV slot
//     `u#` -- UAV (unordered access view) slot (D3D10+)
//     `v#` -- vertex input stream register (D3D9-era vertex
//             declaration syntax; appears on struct members in
//             SWG's `.vsh` corpus, e.g. 2d.vsh line 8:
//             `float4 position : POSITION0 : register(v0);`)
//   Struct members may not carry any of these as semantics under SM4+
//   (only globals, cbuffer-members-with-packoffset, and sampler/
//   texture declarations can). Iteration history:
//     * Iter-7 hardcoded just `c` based on a speculative read.
//     * Iter-7 smoke produced X3202 unchanged -> letter is NOT `c`.
//     * Iter-8 expanded to `[bcstu]` + landed a THROWAWAY diagnostic
//       dump capturing the first main-source bytes.
//     * Iter-8 smoke produced X3202 unchanged -> letter is NOT in
//       `[bcstu]` either. The THROWAWAY dump revealed the actual
//       syntax: `: register(v0)` -- letter is `v`.
//     * Iter-9 reverted the dump (first commit) and extended the
//       set to `[bcstuv]`. The dump's purpose was served and the
//       set is exhaustive per the HLSL grammar.
//     * Iter-9 smoke cleared X3202 but surfaced X4016 "overlapping
//       register semantics not yet implemented 'c0'" with no
//       line/col info -- a globals-level binding collision.
//     * Iter-10 was a PURE DIAGNOSTIC THROWAWAY iteration adding I/O
//       dumps at both rewrite entry points (orchestrator extended
//       the include dump to 5 includes late in Iter-10 / early in
//       Iter-11).
//     * Iter-10/11 dumps revealed the OVER-STRIP root cause: Rules
//       B/C were stripping `: register(cN)` on GLOBAL declarations
//       in vertex_shader_constants.inc (e.g.
//       `float4x4 objectWorldCameraProjectionMatrix : register(c0)`)
//       not just on the SM4+-illegal struct-member position. The
//       resulting unannotated globals (including the multi-register
//       `LightData lightData : register(c16)` struct) defeated
//       D3DCompile's auto-allocator -> X4016.
//     * Iter-11 makes Rules B/C context-aware (this commit): only
//       strip when a previous `:` has appeared on the same line
//       (struct-member stacked-semantic position). On global
//       declarations (single-colon shape), the binding is preserved
//       verbatim -- which is exactly what SM4+ requires.
//
//   Case sensitivity for Rules B/C: the register-type letter and the
//   literal `register` keyword must be lowercase. `: COLOR` /
//   `: COLOR0` / `: COLOR1` / `: SV_TARGET` are user/system semantics
//   that legitimately appear on struct members and MUST NOT be
//   touched. HLSL is case-sensitive so the lowercase regex letters
//   distinguish them naturally. The Iter-9 expansion does NOT widen
//   case sensitivity -- uppercase `B0` / `S0` / `V0` etc. would not
//   match.
//
// Semantic-loss caveat (Iter-7 forward-looking note):
//   Rules B and C DROP the register-binding metadata entirely. When
//   D3DCompile then compiles the rewritten source, its automatic
//   register-allocator picks registers from the available pool. If the
//   engine assumes a specific register layout at the C++ level (e.g.,
//   SetVertexShaderConstantF(4, ...) expecting the shader to read
//   constant c4), runtime rendering could break in subtle ways --
//   shader compiles + draws don't FATAL, but produce wrong colors / no
//   geometry / NaN math. This is a known forward-looking risk recorded
//   in the Plan 11-07 Iter-7 iteration log; if a Plan 11-07 Task 3
//   smoke shows visual issues, the rewrite's semantic-info loss is on
//   the suspect list alongside the BACKWARDS_COMPATIBILITY flag and
//   the X3206 implicit-truncation warnings in functions.inc.
//
// Cache invalidation:
//   The rewrite happens AFTER hashSource() computes the FNV-1a cache
//   key (the hash sees the ORIGINAL bytes). To force a mass cache miss
//   when this rewrite logic changes, the VS/PS compile sites inject a
//   `D3D11_REWRITE_VERSION` macro into the defines list (which DOES
//   participate in hashSource). Bump that macro's value when changing
//   the rewrite rules. See Direct3d11_VertexShaderData.cpp's defines
//   list around the D3D11_REWRITE_VERSION push.
//
// Per CONTEXT D-13: no D3DPOOL_MANAGED / OnLostDevice / OnResetDevice
// (this utility owns no GPU resources at all).
// Per Plan 11-04 D-05: D3D9 plugin source untouched.
//
// ======================================================================

#ifndef INCLUDED_Direct3d9_HlslRewrite_H
#define INCLUDED_Direct3d9_HlslRewrite_H

// ======================================================================

#include <cstddef>
#include <vector>

// ======================================================================

namespace Direct3d9_HlslRewrite
{
	// Entry point for the include-handler call site (heap-owning buffer
	// semantics).
	//
	// Reads `inBuffer` (length `inLength`); applies Rules A/B/C; returns
	// either the original buffer untouched (fast path -- no occurrences
	// of any rule matched) or a freshly-allocated `new unsigned char[]`
	// buffer with the rewrites applied.
	//
	// Ownership contract: the caller is presumed to OWN `inBuffer`
	// (allocated via `new unsigned char[]` -- e.g., from
	// AbstractFile::readEntireFileAndClose). On rewrite, this function
	// `delete[]`s the original `inBuffer` and returns a fresh heap
	// allocation. The caller's eventual `delete[]` on the returned
	// pointer releases whichever buffer was returned.
	//
	// `outLength` receives the rewritten buffer's size in bytes.
	// `diagName` is used only for DEBUG_REPORT_LOG_PRINT diagnostic
	// output; pass the filename for traceability.
	unsigned char *applyToIncludeBuffer(
		unsigned char *inBuffer,
		std::size_t    inLength,
		std::size_t   &outLength,
		char const    *diagName);

	// Entry point for the main-source call site (non-owning view
	// semantics).
	//
	// Reads `src` / `length` as a (char const *, size_t) view -- the
	// CALLER OWNS the underlying storage and this function does not
	// touch it. Output is written into `outVec` (cleared and resized
	// internally). After the call, `outVec.data()` / `outVec.size()`
	// are valid for passing to D3DCompile as the `pSrcData` / `SrcDataSize`
	// arguments. The vector goes out of scope after the synchronous
	// D3DCompile returns.
	//
	// When no rewrites apply, `outVec` is populated with a verbatim copy
	// of the original bytes (one allocation; simpler ownership story
	// than threading a "use original buffer" sentinel up to compileOrLoad).
	// The fast path matters more for `#include` content (where the
	// allocation is already deferred) than for the main shader source
	// (which is loaded once per compile-or-load call).
	//
	// `diagName` is used only for diagnostic output.
	void applyToMainSource(
		char const           *src,
		std::size_t           length,
		std::vector<char>    &outVec,
		char const           *diagName);
}

// ======================================================================

#endif
