// ======================================================================
//
// Direct3d11_ShaderCache.h
// copyright (c) 2026 Galaxies Reborn
//
// Precompiled DXBC, fetched instead of compiled.
//
// The problem it solves is measured, not assumed. With every cache in this backend working --
// zero state objects, zero input layouts and zero constant buffers created in the worst frame
// of an eleven thousand frame run -- the one remaining source of in-frame work was first-use
// HLSL compilation, and it cost 310.9 ms in a single frame at zone-in, 1213.4 ms over the run
// for 181 programs. That is a third of a second of stall, and it is a regression against DX9,
// which only had to ASSEMBLE vs_1_1 and ps_1_1: a parse and an encode, not a compiler.
//
// ----------------------------------------------------------------------
// Keyed by content, never by name
//
// A blob is stored under the hash of the exact input D3DCompile was given: the prepared source,
// the macro pairs in order, the target profile and the flag word. Not under the program's name.
//
// That choice is the whole safety argument. A name-keyed cache has to answer "is this blob still
// current?", and every answer to that question is a guess that can be wrong -- a timestamp, a
// version number someone has to remember to bump, an asset revision that does not cover loose
// overrides. A content-keyed cache cannot be stale: different input hashes differently, finds no
// file, and compiles. The worst a wrong or missing blob can do is cost what the port costs today.
//
// ----------------------------------------------------------------------
// The one thing the key cannot cover, and what covers it instead
//
// #include contents are part of the compiler input, and they are not in the key. They cannot be:
// includes are served BY the compile, through the include handler, so their contents are unknown
// at the moment a cache has to decide whether to compile at all. Hashing them would mean
// reimplementing enough of a preprocessor to find them, which is a second implementation of the
// thing most likely to disagree.
//
// So they are covered once, at install, instead. The baker records the hash of every include it
// served, patched exactly as the runtime patches them, and install() re-reads each one through
// the same path and compares. A single mismatch disables the whole cache and names the file. That
// is sound because the corpus shares a handful of .inc files across hundreds of programs: the
// check is a few file reads, and an include that changed invalidates everything that could have
// included it, which is the correct blast radius.
//
// An include the baker never saw is not a hole. A program that includes a file the baker did not
// serve is a program whose source the baker did not compile, so its key is absent and it compiles.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_ShaderCache_H
#define INCLUDED_Direct3d11_ShaderCache_H

// ======================================================================

#include "Direct3d11_ShaderPrograms.h"

#include <d3d11_1.h>

// ======================================================================

class Direct3d11_ShaderCache
{
public:
	// Reads and validates the manifest. Everything after this is a no-op when validation failed,
	// so no caller has to check whether the cache is on.
	static void install();
	static void remove();

	static bool isEnabled();

	// The blob for this key, or null. The caller owns what it gets back, exactly as it owns what
	// D3DCompile returns, so the two paths are interchangeable at the call site.
	//
	// The name is for diagnostics only. It is deliberately NOT part of the key: two programs whose
	// prepared source is byte-identical are the same program as far as the compiler is concerned,
	// and the corpus has such pairs.
	static ID3DBlob *fetch(Direct3d11ShaderHash key, char const *name);

	// Write a freshly compiled blob out, when baking is on. A no-op otherwise, so the compile path
	// can call it unconditionally.
	static void store(Direct3d11ShaderHash key, char const *name, ID3DBlob *bytecode);

	// Written at shutdown when baking is on: the manifest the next run validates against. The
	// include list comes from Direct3d11_ShaderCompiler's own include cache rather than being
	// recorded separately here -- that cache already holds every include exactly as it was served,
	// and a second record of the same thing is a second thing that can disagree.
	static void writeManifest();

private:
	Direct3d11_ShaderCache();
	Direct3d11_ShaderCache(Direct3d11_ShaderCache const &);
	Direct3d11_ShaderCache &operator=(Direct3d11_ShaderCache const &);
};

// ======================================================================

#endif
