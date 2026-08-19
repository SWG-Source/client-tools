// ======================================================================
//
// Direct3d11_EmbeddedShaderCorpus.h
// copyright (c) 2026 Galaxies Reborn
//
// The asm2hlsl corpus, embedded in this DLL as a last-resort substitution
// table. The stock retail dataset ships 224 shader programs as Direct3D 9
// assembly, which D3DCompile cannot build; the converted HLSL normally
// arrives via client-assets on the search path. When it is not mounted,
// this lookup supplies the same translations (corpus includes inlined, so
// each text is self-contained), and a client built from this repository
// alone still renders against a stock dataset. The lookup is a fallback by
// contract: callers consult it only after the mounted data has already
// arrived as untranslatable assembly, so a mounted corpus always wins.
//
// The table itself lives in Direct3d11_EmbeddedShaderCorpus.cpp, which is
// GENERATED from the corpus drop -- regenerate it rather than hand-edit.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_EmbeddedShaderCorpus_H
#define INCLUDED_Direct3d11_EmbeddedShaderCorpus_H

// ======================================================================

class Direct3d11_EmbeddedShaderCorpus
{
public:

	// name is the TreeFile path, e.g. "pixel_program/ui.psh" or
	// "vertex_program/tfcl.vsh". Returns the converted HLSL text
	// (NUL-terminated, static storage, corpus includes already inlined),
	// or NULL if the corpus has no translation under that name.
	static char const *lookupProgram(char const *name);
};

// ======================================================================

#endif
