// ======================================================================
//
// Direct3d11_ShaderPrograms.h
// copyright (c) 2026 Galaxies Reborn
//
// Turning a shipped program's bytes into the exact input D3DCompile is given.
//
// This exists so that there is ONE implementation of that, compiled into both the backend DLL
// and the offline baker in tools/dxbcbake. The baker's whole job is to produce, ahead of time,
// bytecode identical to what the client would produce at first use -- which is only true if it
// prepares the source identically. A second implementation that agreed on the day it was
// written is a second implementation that stops agreeing later, and the failure mode is
// bytecode that compiles from the wrong text and renders wrong.
//
// Nothing here touches the engine. No ShaderImplementation, no TreeFile, no Tag. The caller
// supplies bytes and gets bytes back, because the baker cannot link the engine and should not
// have to. Tag values are passed as the four-character codes they already are.
//
// ----------------------------------------------------------------------
// What the leading #define block is for, and why it is thrown away
//
// A vertex program opens with a language marker and a block of #defines naming the texture
// coordinate sets it uses BY TAG:
//
//     //hlsl vs_2_0
//     #define textureCoordinateSetMAIN textureCoordinateSet0
//     #define DECLARE_textureCoordinateSets float2 textureCoordinateSet0 : TEXCOORD0;
//
// The order of those tag defines is the program's canonical tag order: tag i becomes
// TEXCOORD i. The file's own definitions are then discarded along with the block, and the
// runtime supplies its own -- which is what lets one compiled variant serve every material,
// with the input layout routing each material's chosen vertex buffer set to the matching
// semantic. Leaving the block in place makes the two sets of definitions collide and seven
// programs fail X4532.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_ShaderPrograms_H
#define INCLUDED_Direct3d11_ShaderPrograms_H

// ======================================================================

#include <d3dcommon.h>

// unsigned long long rather than the engine's uint64: this header is compiled into a tool that
// does not include sharedFoundation, and the type has to mean the same thing in both.
typedef unsigned long long Direct3d11ShaderHash;

// ======================================================================

class Direct3d11_ShaderPrograms
{
public:
	// Eight is what the engine supports, and a program declaring more is reported rather
	// than truncated silently.
	static constexpr int MAX_TEXTURE_COORDINATE_SETS = 8;

	// Room for the tag pairs, the DECLARE_ macro, the point rename and the terminator.
	static constexpr int MAX_MACROS = (MAX_TEXTURE_COORDINATE_SETS * 2) + 8;

	enum Language
	{
		L_unknown,
		L_hlsl,
		L_assembly
	};

	// What the leading block said, and where the compilable source begins.
	class Header
	{
	public:
		Header();

		Language language;

		// Offset into the text the caller passed, not a pointer: the baker and the runtime hold
		// that text in different places and an offset survives being passed between them.
		int sourceOffset;
		int sourceLength;

		// The four-character tag codes, in declaration order. Not NUL terminated individually;
		// each is exactly four characters.
		char tags[MAX_TEXTURE_COORDINATE_SETS][4];
		int tagCount;

		// A program declaring more sets than the engine supports. The first
		// MAX_TEXTURE_COORDINATE_SETS are kept so the caller can still report which program.
		bool tooManyTags;
	};

	// Builds and owns the macro array, so the pointers it hands to D3DCompile stay valid for as
	// long as it does. Copying is disabled for the same reason: the D3D_SHADER_MACRO entries
	// point into this object's own storage.
	class Macros
	{
	public:
		Macros();

		// Tag i becomes textureCoordinateSet<i>, and DECLARE_textureCoordinateSets becomes the
		// struct members. The dimension follows the TAG, not whichever numbered set a material
		// points it at -- DOT3 is four components, everything else two -- which is what makes the
		// compiled program independent of the material.
		void buildForVertexProgram(Header const &header);
		void buildForPixelProgram();

		D3D_SHADER_MACRO const *get() const;
		int getCount() const;

	private:
		Macros(Macros const &);
		Macros &operator=(Macros const &);

		void add(char const *name, char const *definition);
		void terminate();

		// Fixed storage rather than std::string: this crosses into a tool that does not link the
		// engine's allocator, and the sizes are bounded by the eight-set limit.
		static constexpr int TEXT_BYTES = 32;
		static constexpr int DECLARATION_BYTES = 8 + (MAX_TEXTURE_COORDINATE_SETS * 64);

		D3D_SHADER_MACRO m_macros[MAX_MACROS];
		char m_names[MAX_MACROS][TEXT_BYTES];
		char m_definitions[MAX_MACROS][TEXT_BYTES];
		char m_declaration[DECLARATION_BYTES];
		int m_count;
	};

public:
	// Reads the marker and the leading #define block. Returns false when the text is empty; the
	// caller reports that, because only the caller knows the program's name.
	static bool parseHeader(char const *text, int length, Header &header);

	// FNV-1a over everything D3DCompile is given except the includes: the prepared source, the
	// macro name/definition pairs in order, the target profile and the flag word.
	//
	// The includes are deliberately not in here, and cannot be: they are served during the
	// compile, so their contents are not known until after the point where a cache would need to
	// have decided. They are covered separately and once, by validating the manifest's recorded
	// include hashes at install -- see Direct3d11_ShaderCache.h. That split is the whole reason
	// this key is safe to use as a file name.
	static Direct3d11ShaderHash hashCompilerInput(char const *preparedSource, int preparedLength, D3D_SHADER_MACRO const *macros, char const *target, unsigned int flags);

	// FNV-1a over a buffer, for the include manifest. Exposed because the baker records these and
	// the runtime re-computes them, and both have to agree to the bit.
	static Direct3d11ShaderHash hashBytes(void const *data, int length);

private:
	Direct3d11_ShaderPrograms();
	Direct3d11_ShaderPrograms(Direct3d11_ShaderPrograms const &);
	Direct3d11_ShaderPrograms &operator=(Direct3d11_ShaderPrograms const &);
};

// ======================================================================

#endif
