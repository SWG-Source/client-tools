// ======================================================================
//
// Direct3d9_ShaderCache.h
// Disk-persisted vertex-shader bytecode cache for the D3D9 (gl05/06/07) plugins.
//
// Ported from the gl11 Direct3d11_ShaderCache (Phase 11). PURPOSE: the //hlsl
// vertex-shader path compiles at RUNTIME via D3DCompile on the main thread during
// zone-in (~44 compiles first cantina load) -> a visible load hitch the retail
// client never paid (it ships precompiled bytecode). The existing process-global
// IN-MEMORY cache (Direct3d9_VertexShaderData ms_byteCodeCache) only kills RE-ENTRY
// recompiles within a session. THIS disk layer persists compiled bytecode across
// launches, so the FIRST-ever zone-in becomes a cache HIT after one warm run --
// matching what retail/SWGEmu effectively ship (CONSULT-45).
//
// CORRECTNESS (CONSULT-45 / Cursor): the disk key is a CONTENT hash over the full
// compile inputs (source bytes + expanded D3D_SHADER_MACRO defines + target profile
// + a rewrite-version + a compiler-fingerprint tag) -- NOT the in-memory path-key
// (filename,texcoordKey,target), which would silently load STALE bytecode after a
// .vsh / HlslRewrite edit. VS bytecode is GPU-arch-independent, so Win32 and x64
// share one cache dir safely.
//
// The .cso blob is opaque & self-versioning; we just hash + dump + load (no format
// game). store() failure is NON-FATAL (cache is opportunistic; next launch just
// recompiles). install() creates the directory lazily.
//
// CONSULT-68 sibling: install() also starts a background thread that slurps the
// whole cache dir into RAM ([Direct3d9] shaderCachePreload, default true), so
// tryLoad in the draw path is a memory lookup, never a per-hit disk read.
//
// ======================================================================

#ifndef INCLUDED_Direct3d9_ShaderCache_H
#define INCLUDED_Direct3d9_ShaderCache_H

// ======================================================================

#include <cstddef>
#include <cstdint>
#include <vector>
#include <d3dcompiler.h>   // D3D_SHADER_MACRO

// ======================================================================

class Direct3d9_ShaderCache
{
public:

	static void install(char const *cacheDir);   // creates cacheDir if missing
	static void remove();

	// Cache HIT -> fills outBytes with the compiled bytecode and returns true.
	// MISS -> returns false; caller compiles and passes the result to store().
	static bool tryLoad(uint64_t sourceHash, std::vector<unsigned char> &outBytes);

	// Persist bytecode for next-launch warm start. Failure (disk full / perms)
	// logs a warning -- NOT FATAL.
	static void store(uint64_t sourceHash, void const *data, std::size_t size);

	// Stable 64-bit content hash (FNV-1a). Includes the source body, EVERY define
	// (Name\0Definition\0), the target profile, a rewriteVersion (bump when the
	// HlslRewrite rules or texcoord macro generation change), and a fixed compiler
	// fingerprint -- so any input that changes the emitted bytecode changes the hash.
	static uint64_t hashSource(char const *text, std::size_t length,
		D3D_SHADER_MACRO const *defines, char const *target, uint32_t rewriteVersion);
};

// ======================================================================

#endif
