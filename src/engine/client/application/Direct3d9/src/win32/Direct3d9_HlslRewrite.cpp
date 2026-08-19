// ======================================================================
//
// Direct3d9_HlslRewrite.cpp
// Phase 11 D3D11 renderer plugin -- shared HLSL textual rewrite utility.
//
// Plan 11-07 Iteration 7 -- see Direct3d9_HlslRewrite.h preamble for the
// full design rationale (two-call-site lifetime semantics, the three
// rewrite rules A/B/C, semantic-loss caveat, cache-invalidation strategy).
//
// Implementation summary (Phase 32 D3D9 fork):
//   Rule D (gl11's structural cbuffer wrap of c-register globals; Iter-13)
//   is REMOVED from this D3D9 copy. Rule D is the SM4+/level9 X4016 $Globals
//   fix; on D3D9 vs_2_0 flat `: register(cN)` is the NATIVE constant model
//   and the engine pushes by index (Direct3d9_StateCache.cpp:237) -- applying
//   Rule D here would silently remap c0/c16 to the wrong matrices/lighting
//   (D-02 / review fix #3). BOTH the include-handler and main-source paths
//   feed the source straight into the single-pass A/B/C/E scanner.
//
//   Single-pass scanner walks the source byte-by-byte. At each position
//   it tries (in order):
//     1. Rule A: SM4+ reserved-keyword whole-word match (Iter-4 origin).
//        On match -- emit the keyword followed by the `_id` suffix; output
//        grows by 3 bytes per match. Left + right boundary checks ensure
//        `endpoint` / `pointSize` / `linear` / etc. don't trigger.
//     2. Rule B: `:\s*[bcstuv]\d+\b` -- legacy struct-member register-
//        binding shortcut (Iter-7 origin; Iter-8 expanded `c` -> `[bcstu]`;
//        Iter-9 further expanded to `[bcstuv]` after the THROWAWAY dump
//        revealed `2d.vsh:8` uses `v0`; Iter-11 made context-aware --
//        only strips when a previous `:` appears on the SAME logical
//        line, i.e. the stacked-semantic struct-member shape).
//        On match -- emit the same number of spaces; output size unchanged
//        from this rule.
//     3. Rule C: `:\s*register\s*\(\s*[bcstuv]\d+\s*\)` -- explicit
//        register() form (Iter-7 origin; Iter-8 / Iter-9 / Iter-11 all
//        evolved the same way as Rule B).
//        On match -- emit the same number of spaces; output size unchanged
//        from this rule.
//     4. Rule E: line-start `#pragma def` directive (Iter-14 origin --
//        CODEX phone-a-friend identified `#pragma def(vs, c95, ...)` as
//        the actual X4016 'c0' trigger via controlled FXC repro). Strips
//        the entire line (up to but not including the trailing `\n`),
//        emitting match-length spaces -- the `\n` itself flows through
//        the byte-copy fallback so line-count parity is preserved.
//        D3D9-era explicit float-register pre-fill; the SM4+ level9 path
//        emits its own c#-register mappings from the cbuffer layout and
//        the legacy pragma collides with that allocator. Safe to strip
//        for compile/load; Plan 11-08's reflection-driven cbuffer push
//        catches the four floats (`0.0, 0.5, 1.0, 1/ln(2)`) if any path
//        turns out to need them.
//   No match -- emit the single byte unchanged + advance one position.
//
// Iter-11 context-aware-rule state (line-scoped):
//   A single bool `sawColonThisLine` is threaded through both passes of
//   the scanner. It is FALSE at start-of-buffer; set to TRUE the FIRST
//   time the scanner encounters a `:` on the current logical line; reset
//   to FALSE on every `\n`. Rules B and C only fire when this flag is
//   already TRUE at the matching `:` position -- i.e. the rule attempts
//   to strip a SECOND `:` on the same line, which is the SM4+-illegal
//   stacked-semantic-on-struct-member shape (`: POSITION0 : register(v0)`).
//   On the FIRST `:` of a line (e.g. a global `float4 foo : register(c0);`
//   in vertex_shader_constants.inc), the flag is still FALSE, the rule
//   returns 0, and the binding is preserved verbatim -- which is exactly
//   what SM4+ requires for global register bindings.
//
//   This context check was added in Iter-11 after Iter-10's diagnostic
//   dumps revealed that Iter-7..Iter-10's rules were OVER-STRIPPING:
//   they stripped `: register(cN)` regardless of whether the `:` was
//   the first colon (global -- binding is LEGAL and required) or a
//   subsequent colon (struct-member stacked semantic -- binding is
//   ILLEGAL under SM4+). The over-strip removed all bindings from
//   vertex_shader_constants.inc's ~15 global declarations including
//   the multi-register `LightData lightData : register(c16)` struct,
//   forcing D3DCompile's auto-allocator to invent non-overlapping
//   placements -- which it could not, emitting X4016.
//
// Order matters: Rule C is tried BEFORE Rule B because Rule C is a
// superset / more-specific match starting at the same `:` position.
// (Both rules start at `:`; Rule C's pattern is longer when it matches,
// so we try the longer one first to avoid Rule B greedily eating the
// `:` of a Rule-C-eligible site.)
//
// Two-pass strategy:
//   Pass 1 (computeRewrittenLength) -- walks the buffer once and returns
//     the total output size in bytes. Required because the include-handler
//     entry point needs to size the destination heap allocation BEFORE
//     emitting any bytes; the main-source entry point uses vector::resize
//     against the same size for symmetry + same emit code path.
//   Pass 2 (emitRewritten) -- walks the same buffer a second time and
//     emits the rewritten bytes into the caller's destination. Identical
//     control-flow shape to Pass 1; the difference is "+= 1 to output
//     size" vs. "write 1 byte to dst[outPos++]".
//   The two passes are guaranteed to agree on every match's location and
//   length because they read the same input and use identical match
//   predicates -- if they ever diverge, DEBUG_FATAL guards in emitRewritten
//   catch the corruption.
//
// Column preservation: Rules B/C replace match-length bytes with the same
// number of spaces. This preserves the source's column offsets so any
// subsequent D3DCompile error message that prints `(line,col)` still
// points at the same logical location in the original source. Line
// numbers are preserved automatically because the rewrite never adds or
// removes newlines.
//
// Memory: the include-handler entry point delete[]'s the original buffer
// when it allocates a new one; the main-source entry point only reads
// the input (caller owns it). Both entry points produce a `std::size_t`
// output-size value the caller passes to D3DCompile as the
// SrcDataSize / Bytes parameter.
//
// ======================================================================

#include "FirstDirect3d9.h"
#include "Direct3d9_HlslRewrite.h"

#include "sharedDebug/DebugFlags.h"

#include <cstddef>
#include <cstring>

// ======================================================================

namespace
{
	// ------------------------------------------------------------------
	// Rule E enable/disable gate (Phase 32, D-02 / review fix Cursor C6).
	// ------------------------------------------------------------------
	//
	// gl11 runs Rule E (`#pragma def(vs, c95, ...)` line strip)
	// UNCONDITIONALLY because under the SM4+ vs_4_0_level_9_3 profile FXC
	// emits its own legacy c#-register mappings and the pragma collides
	// with that allocator (X4016 'c0'). On the D3D9 vs_2_0 path that gl05/
	// gl07 compile, `def` is a NATIVE D3D9 instruction and the pragma may
	// be legitimate -- stripping it could drop the runtime c95 pre-fill
	// (0.0, 0.5, 1.0, 1/ln(2)) that registers.inc aliases as
	// c0_0/c0_5/c1_0/cLog2e (c95.x/y/z/w). #pragma def IS present in
	// vertex_shader_constants.inc:114 and IS referenced -- so Rule E is
	// NOT inert on this corpus.
	//
	// Phase-32 Task-1 decision table (probe-then-act, default-first):
	//   | Probe result                                   | Action |
	//   | #pragma def absent                             | INERT  |
	//   | present AND D3DCompile@vs_2_0 ACCEPTS it       | OFF    |
	//   | present AND D3DCompile@vs_2_0 REJECTS it (X#)  | ON     |
	//
	// RESOLVED (Phase 32 Task-3 recovery, 2026-06-16) -> BOTTOM ROW = ON.
	// The non-probed shader vertex_program/2d_texture.vsh FATAL'd in-game
	// at character-select with `error X4016: overlapping register semantics
	// not yet implemented 'c0'` (Graphics.cpp:1200, ui_planet_sel). Ground-
	// truth fxc_47 (d3dcompiler_47) repro at vs_2_0 /Gec against the FULLY-
	// REWRITTEN include tree (vertex_shader_constants.inc:122 carries
	// `#pragma def(vs, c95, 0.0, 0.5f, 1.0f, 1.4426950408889634f)`):
	//   Rule E OFF -> EXACT X4016 'c0' (FXC reports its first-register slot,
	//                 NOT the literal c95) + the same benign X3206 noise.
	//   Rule E ON  -> compiles CLEAN (vs_2_0 code emitted).
	// So at vs_2_0 d3dcompiler_47 REJECTS the legacy `def` (it collides with
	// its own constant auto-allocator) -- the bottom-row condition holds.
	//
	// Runtime-safety confirmed (bottom-row REQUIREMENT): the c95 prefill is
	// NOT lost by stripping the pragma. Direct3d9_StateCache::setConstants()
	// (Direct3d9_StateCache.cpp:235-237, #ifdef VSPS) uploads
	//   c95 = { 0.0, 0.5, 1.0, 1.0/log(2.0) }  via setVertexShaderConstants(95,..)
	// UNCONDITIONALLY every frame -- the same quadruple the pragma encoded
	// (registers.inc aliases it as c0_0/c0_5/c1_0/cLog2e). The pragma was a
	// redundant compile-time literal; the runtime index-push covers it.
	// (For 2d_texture itself fxc folds the 0.5/1.0 literals into an inline
	// `def c0` anyway -- it does not even read c95.) Recorded in 32-02-SUMMARY.
	constexpr bool kRuleEStripPragmaDef = true;

	// ------------------------------------------------------------------
	// Rule A: SM4+ reserved keywords renamed to `<keyword>_id`.
	// ------------------------------------------------------------------

	struct ReservedKeyword
	{
		char const *word;
		std::size_t length;
	};

	// SM4+ HLSL lexer reserves these identifiers as keywords for
	// geometry-shader primitive-type-binding syntax. SWG's D3D9-era
	// HLSL uses them as ordinary identifiers; the lexer rejects them
	// with X3000 "syntax error: unexpected token '<keyword>'" before
	// any profile-specific feature gating runs.
	constexpr ReservedKeyword kReservedKeywords[] = {
		{ "point",       5 },  // GS primitive type + sampler filter literal
		{ "line",        4 },  // GS primitive type
		{ "triangle",    8 },  // GS primitive type
		{ "lineadj",     7 },  // GS primitive type (line w/ adjacency)
		{ "triangleadj", 11 }, // GS primitive type (triangle w/ adjacency)
	};

	constexpr std::size_t kReservedKeywordCount =
		sizeof(kReservedKeywords) / sizeof(kReservedKeywords[0]);

	// Replacement suffix appended after each Rule-A match.
	constexpr char const  kReplacementSuffix[]     = "_id";
	constexpr std::size_t kReplacementSuffixLength = 3;

	// HLSL identifier character class (same as C): [A-Za-z0-9_]. A
	// whole-word match for Rule A requires the previous char (if any) to
	// be OUT of this class AND the following char (if any) to be OUT of
	// this class.
	inline bool isIdentChar(unsigned char c)
	{
		return (c >= 'a' && c <= 'z')
			|| (c >= 'A' && c <= 'Z')
			|| (c >= '0' && c <= '9')
			|| (c == '_');
	}

	inline bool isAsciiDigit(unsigned char c)
	{
		return (c >= '0' && c <= '9');
	}

	// Whitespace per HLSL's lexer (used to scan `\s*` between tokens in
	// Rules B/C). HLSL treats space, tab, and the line-end characters
	// equivalently between tokens; we follow the same set.
	inline bool isHlslSpace(unsigned char c)
	{
		return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n');
	}

	// D3D HLSL register-type letter (used by Rules B/C). The full
	// canonical set per the MSDN HLSL reference + the `register()`
	// keyword grammar:
	//   `b#` -- constant buffer (cbuffer) register
	//   `c#` -- single constant register (D3D9 legacy + D3D11 cbuffer-
	//           member packoffset offset)
	//   `s#` -- sampler register
	//   `t#` -- texture / SRV register
	//   `u#` -- UAV (unordered access view) register (D3D10+)
	//   `v#` -- vertex input stream register (D3D9-era vertex
	//           declaration syntax; struct-member `: register(vN)`
	//           bindings are common in SWG's `.vsh` corpus)
	//
	// Iter-7 originally hardcoded just `c` here because the X3202 error
	// site visible at the time (`2d.vsh(8,44-45)`) was speculatively
	// thought to be a `: c<N>` binding. Iter-7's smoke produced the
	// identical X3202 at the identical site, meaning the actual letter
	// at line 8 col 44 was NOT `c`. Iter-8 expanded the set to
	// `[bcstu]`, but Iter-8's THROWAWAY diagnostic dump (reverted in
	// Iter-9's first commit) revealed the ground truth: 2d.vsh line 8
	// is literally `float4 position : POSITION0 : register(v0);`. The
	// letter is `v` -- vertex input stream register binding, which
	// neither Iter-7's single-`c` rule nor Iter-8's `[bcstu]` expansion
	// covered. Iter-9 extends the set to `[bcstuv]` -- all 6 canonical
	// D3D HLSL register-type letters.
	//
	// Semantic-loss note specific to `v#`: vertex input stream register
	// bindings are normally honored by D3D9's vertex-declaration stage;
	// under D3D11 the IA stage handles input-layout binding via
	// Direct3d11_VertexDeclarationMap (Plan 11-06). Stripping `: register(vN)`
	// from a struct member drops a binding hint that D3D11 doesn't
	// consume at the shader level -- the actual binding happens at the
	// input-layout level via D3D11_INPUT_ELEMENT_DESC. The semantic-loss
	// caveat in the file preamble still applies generally, but the
	// `v#` case in particular should be safe.
	//
	// HLSL is case-sensitive; these letters are lowercase by HLSL
	// convention -- uppercase user semantics like `: COLOR0` /
	// `: SV_POSITION` are not affected.
	inline bool isHlslRegisterTypeLetter(unsigned char c)
	{
		return (c == 'b') || (c == 'c') || (c == 's')
			|| (c == 't') || (c == 'u') || (c == 'v');
	}

	// ------------------------------------------------------------------
	// Match helpers for Rules A/B/C. Each returns 0 on no-match, else
	// the number of input bytes the match consumes (i.e., how far to
	// advance the scanner). On match, callers know how many output bytes
	// to produce based on the rule:
	//   Rule A -- emit `keyword + "_id"` (out = match length + 3).
	//   Rule B -- emit match-length spaces (out = match length).
	//   Rule C -- emit match-length spaces (out = match length).
	// ------------------------------------------------------------------

	// Rule A: returns (keyword index, match length) on match else (0,0).
	// Matched keyword index is stored in `outKwIndex` for emitRewritten
	// to know which keyword string to copy.
	std::size_t tryMatchRuleA(
		unsigned char const *src,
		std::size_t          length,
		std::size_t          pos,
		std::size_t         &outKwIndex)
	{
		// Left boundary: start-of-buffer OR preceding char is non-ident.
		bool const leftBoundaryOk =
			(pos == 0) || !isIdentChar(src[pos - 1]);
		if (!leftBoundaryOk)
			return 0;

		for (std::size_t k = 0; k < kReservedKeywordCount; ++k)
		{
			ReservedKeyword const &kw = kReservedKeywords[k];
			if (pos + kw.length > length)
				continue;
			if (std::memcmp(src + pos, kw.word, kw.length) != 0)
				continue;

			// Right boundary: end-of-buffer OR following char is
			// non-ident.
			std::size_t const tail = pos + kw.length;
			bool const rightBoundaryOk =
				(tail == length) || !isIdentChar(src[tail]);
			if (!rightBoundaryOk)
				continue;

			outKwIndex = k;
			return kw.length;
		}
		return 0;
	}

	// Rule C: `:\s*register\s*\(\s*[bcstuv]\d+\s*\)`
	// Tried BEFORE Rule B (both start at `:`; Rule C is the more-specific
	// longer match). Returns total match length (from `:` through the
	// closing `)`) on match else 0.
	//
	// Implementation walks the input strictly forward, never backtracking,
	// because the pattern is purely sequential: `:`, then optional
	// whitespace, then the literal `register` token, etc.
	//
	// Iter-9: the register-type letter set is `[bcstuv]` (all 6 canonical
	// D3D HLSL register types: b/cbuffer, c/constant, s/sampler,
	// t/texture, u/UAV, v/vertex-input-stream). Iter-7 started with `c`
	// only; Iter-8 expanded to `[bcstu]`; Iter-9 added `v` after the
	// THROWAWAY dump (reverted) revealed `2d.vsh:8` uses `register(v0)`.
	// See isHlslRegisterTypeLetter() above for the full rationale.
	//
	// Iter-11: context-aware -- the `sawColonThisLine` parameter must be
	// TRUE for the match to fire. When FALSE, this is the first `:` on
	// the line (a legal global-declaration register binding) and we MUST
	// NOT strip it -- doing so produced X4016 in Iter-10 by removing
	// vertex_shader_constants.inc's globals-level bindings. When TRUE,
	// this is a second-or-later `:` on the line (the SM4+-illegal
	// stacked-semantic struct-member shape `: POSITION0 : register(v0)`)
	// and we strip as before.
	std::size_t tryMatchRuleC(
		unsigned char const *src,
		std::size_t          length,
		std::size_t          pos,
		bool                 sawColonThisLine)
	{
		if (pos >= length || src[pos] != ':')
			return 0;

		// Iter-11 context check: only strip when this is NOT the first
		// `:` of the line -- i.e. we're at a stacked-semantic struct-
		// member position, not at a legal global-declaration binding.
		if (!sawColonThisLine)
			return 0;

		std::size_t i = pos + 1;

		// `\s*`
		while (i < length && isHlslSpace(src[i]))
			++i;

		// Literal `register` (8 chars, lowercase, whole-word).
		constexpr char const kRegister[] = "register";
		constexpr std::size_t kRegisterLen = 8;
		if (i + kRegisterLen > length)
			return 0;
		if (std::memcmp(src + i, kRegister, kRegisterLen) != 0)
			return 0;
		// Right boundary on the literal `register` keyword: next char
		// must be non-ident (else this is some identifier like
		// `register_42` we should NOT match).
		std::size_t const afterRegister = i + kRegisterLen;
		if (afterRegister < length && isIdentChar(src[afterRegister]))
			return 0;
		i = afterRegister;

		// `\s*`
		while (i < length && isHlslSpace(src[i]))
			++i;

		// Literal `(`
		if (i >= length || src[i] != '(')
			return 0;
		++i;

		// `\s*`
		while (i < length && isHlslSpace(src[i]))
			++i;

		// `[bcstuv]` (single register-type letter, lowercase). Iter-9
		// set: b/c/s/t/u/v -- see isHlslRegisterTypeLetter() for the
		// canonical-6-letter rationale.
		if (i >= length || !isHlslRegisterTypeLetter(src[i]))
			return 0;
		++i;

		// `\d+` (one or more digits)
		std::size_t const digitStart = i;
		while (i < length && isAsciiDigit(src[i]))
			++i;
		if (i == digitStart)
			return 0;

		// `\s*`
		while (i < length && isHlslSpace(src[i]))
			++i;

		// Literal `)`
		if (i >= length || src[i] != ')')
			return 0;
		++i;

		return i - pos;
	}

	// Rule B: `:\s*[bcstuv]\d+\b`
	// Tried AFTER Rule C at the same `:` site so the longer, more-specific
	// register() form wins when present. Returns total match length on
	// match else 0.
	//
	// Iter-9: the register-type letter set is `[bcstuv]` (all 6 canonical
	// D3D HLSL register types). Iter-7 started with `c` only; Iter-8
	// expanded to `[bcstu]`; Iter-9 added `v` after the THROWAWAY dump
	// (reverted) revealed `2d.vsh:8` uses `register(v0)`. See
	// isHlslRegisterTypeLetter() above for the full rationale.
	//
	// Iter-11: context-aware -- the `sawColonThisLine` parameter must be
	// TRUE for the match to fire. Same rationale as Rule C above; see
	// the comment block there for the over-strip / X4016 narrative.
	std::size_t tryMatchRuleB(
		unsigned char const *src,
		std::size_t          length,
		std::size_t          pos,
		bool                 sawColonThisLine)
	{
		if (pos >= length || src[pos] != ':')
			return 0;

		// Iter-11 context check: only strip on a second-or-later `:`
		// on the same line. Mirrors Rule C's guard above.
		if (!sawColonThisLine)
			return 0;

		std::size_t i = pos + 1;

		// `\s*`
		while (i < length && isHlslSpace(src[i]))
			++i;

		// `[bcstuv]` (single register-type letter, lowercase). Iter-9
		// set: b/c/s/t/u/v -- see isHlslRegisterTypeLetter() for the
		// canonical-6-letter rationale.
		if (i >= length || !isHlslRegisterTypeLetter(src[i]))
			return 0;
		++i;

		// `\d+` -- one or more digits.
		std::size_t const digitStart = i;
		while (i < length && isAsciiDigit(src[i]))
			++i;
		if (i == digitStart)
			return 0;

		// `\b` -- right boundary: end-of-buffer OR next char is non-ident.
		// This stops `: c4foo` from matching (the `c4` is a substring of
		// the identifier `c4foo`, not a register binding) and stops
		// `: c12_thing` from matching for the same reason.
		if (i < length && isIdentChar(src[i]))
			return 0;

		return i - pos;
	}

	// Rule E: line-start `#pragma def` directive (Iter-14 origin).
	// Matches the shape `^\s*#\s*pragma\s+def\b.*` where `^` is the start
	// of a logical line (start-of-buffer OR preceded by `\n`). On match,
	// consumes through the rest of the line up to (but not including) the
	// trailing `\n`; returns the total consumed length. Returns 0 on
	// no-match. The trailing `\n` is intentionally NOT consumed so the
	// scanner's line-state machinery (sawColonThisLine) resets normally
	// when it emits the `\n` via the byte-copy fallback on the next loop
	// iteration. Match-length spaces are emitted in place, preserving line
	// counts AND column offsets for downstream FXC error messages.
	//
	// Rationale (CODEX phone-a-friend, 2026-05-19): controlled FXC repro
	// against `vertex_shader_constants.inc` identified
	// `#pragma def(vs, c95, 0.0, 0.5f, 1.0f, 1.4426950408889634f)` as the
	// X4016 'c0' trigger -- struct-typed packoffset was REFUTED. The
	// pragma is a D3D9-era explicit float-register pre-fill; under
	// vs_4_0_level_9_3 FXC emits its own legacy c#-register mappings from
	// the cbuffer layout and the pragma collides with that allocator.
	// 'c0' in the error message is FXC's first-register report, not the
	// literal c95 slot. Stripping the pragma is safe for compile/load;
	// Plan 11-08's reflection-driven cbuffer push handles the four floats
	// if any active shader path semantically requires them.
	//
	// Pattern is line-anchored deliberately -- a `#pragma def` substring
	// that ever appears mid-line (e.g. inside a string literal, though
	// HLSL has none at file scope) would NOT match. Line-anchoring is
	// stateless: pos == 0 OR src[pos - 1] == '\n'.
	std::size_t tryMatchRuleE(
		unsigned char const *src,
		std::size_t          length,
		std::size_t          pos)
	{
		if (pos >= length)
			return 0;

		// Line-start check: start-of-buffer OR immediately after `\n`.
		if (pos != 0 && src[pos - 1] != '\n')
			return 0;

		std::size_t i = pos;

		// `\s*` before `#` (spaces/tabs only -- a `\n` here would mean
		// we're past the start of the line we tried to match, so it
		// belongs to the NEXT line; treat as no-match).
		while (i < length && (src[i] == ' ' || src[i] == '\t'))
			++i;

		// Literal `#`
		if (i >= length || src[i] != '#')
			return 0;
		++i;

		// `\s*` between `#` and `pragma` (HLSL preprocessor allows it).
		while (i < length && (src[i] == ' ' || src[i] == '\t'))
			++i;

		// Literal `pragma` (whole-word).
		constexpr char const kPragma[]    = "pragma";
		constexpr std::size_t kPragmaLen  = 6;
		if (i + kPragmaLen > length)
			return 0;
		if (std::memcmp(src + i, kPragma, kPragmaLen) != 0)
			return 0;
		std::size_t const afterPragma = i + kPragmaLen;
		if (afterPragma < length && isIdentChar(src[afterPragma]))
			return 0;
		i = afterPragma;

		// `\s+` (REQUIRED at least one space/tab before `def`).
		if (i >= length || (src[i] != ' ' && src[i] != '\t'))
			return 0;
		while (i < length && (src[i] == ' ' || src[i] == '\t'))
			++i;

		// Literal `def` (whole-word; right boundary check rejects e.g.
		// `#pragma define` or `#pragma default` if such things ever existed).
		constexpr char const kDef[]    = "def";
		constexpr std::size_t kDefLen  = 3;
		if (i + kDefLen > length)
			return 0;
		if (std::memcmp(src + i, kDef, kDefLen) != 0)
			return 0;
		std::size_t const afterDef = i + kDefLen;
		if (afterDef < length && isIdentChar(src[afterDef]))
			return 0;
		i = afterDef;

		// Match confirmed -- consume through end-of-line (exclusive of `\n`).
		while (i < length && src[i] != '\n')
			++i;

		return i - pos;
	}

	// ------------------------------------------------------------------
	// Two-pass core: compute output size, then emit bytes.
	// ------------------------------------------------------------------

	std::size_t computeRewrittenLength(
		unsigned char const *src,
		std::size_t          length,
		std::size_t         &outRewriteCount)
	{
		outRewriteCount = 0;
		if (!src || length == 0)
			return 0;

		// Iter-11 line-scoped state: see file preamble. FALSE at start-
		// of-buffer; set to TRUE the first time we see a `:` on the
		// current line; reset to FALSE on every `\n`. Rules B/C only
		// fire when TRUE -- i.e. on a SECOND or later `:` on the line,
		// which is the stacked-semantic struct-member shape that SM4+
		// rejects.
		bool sawColonThisLine = false;

		std::size_t out = 0;
		std::size_t i   = 0;
		while (i < length)
		{
			// Rule A -- reserved-keyword rename (output = match + 3).
			// Rule-A keyword bytes are pure-alpha (point/line/triangle/
			// lineadj/triangleadj); they cannot contain `:` or `\n`, so
			// stepping past them does not perturb sawColonThisLine.
			std::size_t kwIndex = 0;
			std::size_t matchA = tryMatchRuleA(src, length, i, kwIndex);
			if (matchA != 0)
			{
				out += matchA + kReplacementSuffixLength;
				i   += matchA;
				++outRewriteCount;
				continue;
			}

			// Rule C -- explicit register() form (output = match, all
			// spaces). Tried before Rule B because both start at `:`
			// and Rule C is the more-specific longer match.
			//
			// Iter-11: context-aware. Rule C internally guards on
			// `sawColonThisLine` -- returns 0 (no match) on a global-
			// declaration first-colon; matches only on stacked-semantic
			// struct-member second-colon shape.
			std::size_t matchC = tryMatchRuleC(src, length, i, sawColonThisLine);
			if (matchC != 0)
			{
				out += matchC;
				i   += matchC;
				++outRewriteCount;
				// The `:` that opened the match counts as "saw colon
				// this line" for any subsequent `:` later on the same
				// line. The remaining bytes consumed by Rule C
				// (whitespace + `register` + `(` + letter + digits +
				// `)` ) contain no `:` or `\n` by construction (Rule C's
				// matcher rejects them via its strict character classes).
				sawColonThisLine = true;
				continue;
			}

			// Rule B -- shortcut `:\s*[bcstuv]\d+\b` (output = match,
			// all spaces). Iter-11: context-aware -- same
			// sawColonThisLine guard as Rule C.
			std::size_t matchB = tryMatchRuleB(src, length, i, sawColonThisLine);
			if (matchB != 0)
			{
				out += matchB;
				i   += matchB;
				++outRewriteCount;
				// Same rationale as above -- Rule B's consumed bytes
				// after the `:` are whitespace + letter + digits, none
				// of which can be `:` or `\n`.
				sawColonThisLine = true;
				continue;
			}

			// Rule E -- line-start `#pragma def` directive (Iter-14;
			// output = match, all spaces). Line-anchored via pos==0 or
			// src[pos-1]=='\n'. Consumes up to (but not including) the
			// trailing `\n`, so the next loop iteration emits the `\n`
			// via the byte-copy fallback and the existing
			// sawColonThisLine reset logic stays intact.
			std::size_t matchE = kRuleEStripPragmaDef
				? tryMatchRuleE(src, length, i)
				: 0;
			if (matchE != 0)
			{
				out += matchE;
				i   += matchE;
				++outRewriteCount;
				// No sawColonThisLine update needed -- Rule E only fires
				// at line-start where the flag is already FALSE, and the
				// trailing `\n` flows through the fallback path below to
				// keep the reset behavior canonical.
				continue;
			}

			// No rule matched -- copy single byte through. Update the
			// line-scoped state based on the byte we just passed.
			unsigned char const ch = src[i];
			if (ch == '\n')
				sawColonThisLine = false;
			else if (ch == ':')
				sawColonThisLine = true;
			out += 1;
			i   += 1;
		}

		return out;
	}

	void emitRewritten(
		unsigned char const *src,
		std::size_t          length,
		unsigned char       *dst,
		std::size_t          dstCapacity)
	{
		// Iter-11 line-scoped state: MUST mirror computeRewrittenLength
		// exactly. Both passes are guaranteed to evolve sawColonThisLine
		// identically because they read the same input bytes and use
		// identical match predicates -- if they ever diverge, the
		// DEBUG_FATAL at function end catches the length mismatch.
		bool sawColonThisLine = false;

		std::size_t outPos = 0;
		std::size_t i      = 0;
		while (i < length)
		{
			// Rule A -- emit keyword + "_id" suffix. (Rule-A keyword
			// bytes are pure-alpha; cannot contain `:` or `\n`.)
			std::size_t kwIndex = 0;
			std::size_t matchA = tryMatchRuleA(src, length, i, kwIndex);
			if (matchA != 0)
			{
				ReservedKeyword const &kw = kReservedKeywords[kwIndex];
				DEBUG_FATAL(outPos + kw.length > dstCapacity,
					("Direct3d9_HlslRewrite: emit overflow Rule A keyword"
					 " '%s' at src[%zu], outPos=%zu/%zu",
					 kw.word, i, outPos, dstCapacity));
				std::memcpy(dst + outPos, kw.word, kw.length);
				outPos += kw.length;

				DEBUG_FATAL(outPos + kReplacementSuffixLength > dstCapacity,
					("Direct3d9_HlslRewrite: emit overflow Rule A suffix"
					 " '%s' at src[%zu], outPos=%zu/%zu",
					 kw.word, i, outPos, dstCapacity));
				std::memcpy(dst + outPos, kReplacementSuffix, kReplacementSuffixLength);
				outPos += kReplacementSuffixLength;

				i += matchA;
				continue;
			}

			// Rule C -- emit match-length spaces. Iter-11: context-
			// aware via sawColonThisLine -- skipped on global-
			// declaration first-colon.
			std::size_t matchC = tryMatchRuleC(src, length, i, sawColonThisLine);
			if (matchC != 0)
			{
				DEBUG_FATAL(outPos + matchC > dstCapacity,
					("Direct3d9_HlslRewrite: emit overflow Rule C at"
					 " src[%zu] (len=%zu), outPos=%zu/%zu",
					 i, matchC, outPos, dstCapacity));
				std::memset(dst + outPos, ' ', matchC);
				outPos += matchC;
				i      += matchC;
				sawColonThisLine = true;
				continue;
			}

			// Rule B -- emit match-length spaces. Iter-11: same
			// sawColonThisLine guard.
			std::size_t matchB = tryMatchRuleB(src, length, i, sawColonThisLine);
			if (matchB != 0)
			{
				DEBUG_FATAL(outPos + matchB > dstCapacity,
					("Direct3d9_HlslRewrite: emit overflow Rule B at"
					 " src[%zu] (len=%zu), outPos=%zu/%zu",
					 i, matchB, outPos, dstCapacity));
				std::memset(dst + outPos, ' ', matchB);
				outPos += matchB;
				i      += matchB;
				sawColonThisLine = true;
				continue;
			}

			// Rule E -- emit match-length spaces. Mirrors the Pass 1
			// dispatch in computeRewrittenLength; identical match
			// predicate guarantees the same length is consumed here.
			std::size_t matchE = kRuleEStripPragmaDef
				? tryMatchRuleE(src, length, i)
				: 0;
			if (matchE != 0)
			{
				DEBUG_FATAL(outPos + matchE > dstCapacity,
					("Direct3d9_HlslRewrite: emit overflow Rule E at"
					 " src[%zu] (len=%zu), outPos=%zu/%zu",
					 i, matchE, outPos, dstCapacity));
				std::memset(dst + outPos, ' ', matchE);
				outPos += matchE;
				i      += matchE;
				continue;
			}

			// No rule matched -- pass byte through. Update line-scoped
			// state based on the byte we're about to emit.
			DEBUG_FATAL(outPos + 1 > dstCapacity,
				("Direct3d9_HlslRewrite: emit overflow passthrough byte"
				 " src[%zu]=0x%02x, outPos=%zu/%zu",
				 i, src[i], outPos, dstCapacity));
			unsigned char const ch = src[i];
			if (ch == '\n')
				sawColonThisLine = false;
			else if (ch == ':')
				sawColonThisLine = true;
			dst[outPos] = ch;
			++outPos;
			++i;
		}

		DEBUG_FATAL(outPos != dstCapacity,
			("Direct3d9_HlslRewrite: emit length mismatch outPos=%zu"
			 " expected=%zu (Pass1/Pass2 divergence)",
			 outPos, dstCapacity));
	}

}  // anonymous namespace (Rule-A/B/C/E matchers + two-pass core)

// ======================================================================

unsigned char *Direct3d9_HlslRewrite::applyToIncludeBuffer(
	unsigned char *inBuffer,
	std::size_t    inLength,
	std::size_t   &outLength,
	char const    *diagName)
{
	if (!inBuffer || inLength == 0)
	{
		outLength = inLength;
		return inBuffer;
	}

	// Rule D (gl11's cbuffer-wrap of c-register globals) is DISABLED for the D3D9 path and
	// removed from this copy. On D3D9 vs_2_0, flat ": register(cN)" globals are NATIVE and the
	// engine pushes constants BY INDEX (Direct3d9_StateCache.cpp:237, setVertexShaderConstants(95,...));
	// gl11's Rule D is the SM4+ X4016 $Globals fix and would silently remap c0/c16 to the wrong
	// matrices/lighting on D3D9 with NO compile error (Phase 32, D-02 / review fix #3). The include
	// buffer is rewritten by Rules A/B/C/E only (identical to the main-source path).


	std::size_t rewriteCount = 0;
	std::size_t const rewrittenLen = computeRewrittenLength(
		inBuffer, inLength, rewriteCount);

	if (rewriteCount == 0)
	{
		// Fast path -- no Rule A/B/C/E occurrences. Pass the buffer
		// through unchanged; no further allocation.
		DEBUG_FATAL(rewrittenLen != inLength,
			("Direct3d9_HlslRewrite: include-handler fast-path size"
			 " mismatch rewrittenLen=%zu inLength=%zu",
			 rewrittenLen, inLength));

		outLength = inLength;
		return inBuffer;
	}

	// Allocate a fresh buffer + emit.
	unsigned char *outBuffer = new unsigned char[rewrittenLen];
	emitRewritten(inBuffer, inLength, outBuffer, rewrittenLen);

	// Discard the source buffer (we return a freshly-emitted one).
	delete[] inBuffer;

	DEBUG_REPORT_LOG_PRINT(true,
		("Direct3d9_HlslRewrite: include '%s' Rules A/B/C/E rewritten"
		 " (%zu rewrite%s; input=%zu bytes -> output=%zu bytes; delta=%+zd)\n",
		 diagName ? diagName : "<unnamed>",
		 rewriteCount, (rewriteCount == 1) ? "" : "s",
		 inLength, rewrittenLen,
		 static_cast<ptrdiff_t>(rewrittenLen) - static_cast<ptrdiff_t>(inLength)));

	outLength = rewrittenLen;
	return outBuffer;
}

// ----------------------------------------------------------------------

void Direct3d9_HlslRewrite::applyToMainSource(
	char const        *src,
	std::size_t        length,
	std::vector<char> &outVec,
	char const        *diagName)
{
	outVec.clear();

	if (!src || length == 0)
		return;

	unsigned char const *srcU =
		reinterpret_cast<unsigned char const *>(src);

	std::size_t rewriteCount = 0;
	std::size_t const rewrittenLen = computeRewrittenLength(
		srcU, length, rewriteCount);

	outVec.resize(rewrittenLen);
	if (rewrittenLen == 0)
		return;

	emitRewritten(
		srcU,
		length,
		reinterpret_cast<unsigned char *>(outVec.data()),
		rewrittenLen);

	if (rewriteCount != 0)
	{
		DEBUG_REPORT_LOG_PRINT(true,
			("Direct3d9_HlslRewrite: main-source '%s' rewritten"
			 " (%zu rewrite%s; input=%zu bytes -> output=%zu bytes; delta=%+zd)\n",
			 diagName ? diagName : "<unnamed>",
			 rewriteCount, (rewriteCount == 1) ? "" : "s",
			 length, rewrittenLen,
			 static_cast<ptrdiff_t>(rewrittenLen) - static_cast<ptrdiff_t>(length)));
	}
}

// ======================================================================
