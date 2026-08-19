// ======================================================================
//
// Direct3d9_VertexShaderData.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstDirect3d9.h"
#include "Direct3d9_VertexShaderData.h"

#ifdef VSPS

#include "ConfigDirect3d9.h"
#include "Direct3d9.h"
#include "Direct3d9_PixelShaderConstantRegisters.h"
#include "Direct3d9_VertexShaderConstantRegisters.h"
#include "Direct3d9_VertexShaderVertexRegisters.h"
#include "Direct3d9_HlslRewrite.h"   // Fix B (Phase 32): shared HLSL textual rewrite (Rules A/B/C) for the D3DCompile path
#include "Direct3d9_ShaderCache.h"   // CONSULT-45 fix #1: disk-persisted VS bytecode cache (L2; survives launches)
#include "clientGraphics/ShaderCapability.h"
#include "fileInterface/AbstractFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedFoundation/PersistentCrcString.h"

#include <map>
#include <string>    // Phase 32 (2026-06-16): process-global bytecode-cache key string
#include <vector>
#include <cstdint>   // CONSULT-45: uint64_t disk shader-cache content hash
#include <cstdio>    // Phase 32 (2026-06-16): _snprintf for buildByteCodeCacheKey
#include <float.h>   // Phase 19: _clearfp / _fpreset for the SEH-guarded compile (FP-fault fix)
#include <d3dcompiler.h>   // Fix B (Phase 32): D3DCompile replaces D3DXCompileShader on the HLSL compile path

// ======================================================================

namespace Direct3d9_VertexShaderDataNamespace
{
	struct Include
	{
	public:

		Include(CrcString const &fileName, AbstractFile *file);
		~Include();

	public:

		PersistentCrcString  m_fileName;
		int                  m_length;
		byte *               m_data;

	private:

		Include(Include const &);
		Include & operator =(Include const &);
	};

	class IncludeHandler : public ID3DInclude
	{
	public:
		virtual HRESULT STDMETHODCALLTYPE Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes);
		virtual HRESULT STDMETHODCALLTYPE Close(LPCVOID pData);
	};

	typedef std::vector<D3D_SHADER_MACRO>  Defines;
	typedef std::map<CrcString const *, Include *, LessPointerComparator> IncludeCache;

	// ------------------------------------------------------------------
	// Phase 33 Plan 02 (2026-06-17 / SHADER-01 / D-04 / D-05): the //asm .vsh
	// compile path now assembles through D3DAssemble (d3dcompiler_47), NOT
	// D3DXAssembleShader -- taking the LAST D3DX compile dependency off the D3D9
	// path (the //hlsl path already migrated to D3DCompile in Phase 32). d3dx9 has
	// no x64 static lib, so this is a precondition for the x64 link (Plan 05).
	//
	// D3DAssemble is NOT in the public d3dcompiler.h header, so it is resolved via
	// GetProcAddress on the UNDECORATED export (the .lib symbol is C++-mangled;
	// GetProcAddress sidesteps the mismatch). The HMODULE + fn pointer are cached
	// in a STATIC-LOCAL resolve-once at the call site (validated against the
	// Phase 32-01 probe shape) -- never LoadLibrary/GetProcAddress per compile.
	//
	// x64-export confirmed (reviews fix #7a, 2026-06-17): dumpbin /exports on the
	// x64 C:\Windows\System32\d3dcompiler_47.dll (machine 8664) lists D3DAssemble,
	// so s_d3dAssemble is non-null on x64 -- it will NOT FATAL on every asm shader.
	//
	// ABI: D3DXMACRO == D3D_SHADER_MACRO, ID3DXInclude == ID3DInclude, ID3DXBuffer
	// == ID3DBlob (binary-layout-identical), so we reinterpret-cast only at the
	// single call boundary -- the same technique the 32-02 HLSL path uses (:178-189).
	typedef HRESULT (WINAPI *PFN_D3DAssemble)(
		LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName,
		const D3D_SHADER_MACRO *pDefines, ID3DInclude *pInclude, UINT Flags,
		ID3DBlob **ppCode, ID3DBlob **ppErrorMsgs);

	void getToken(char const *& s, char * d);
	void skipRestOfTheLine(char const *& s);

	const Tag TAG_DOT3 = TAG(D,O,T,3);

	// these exist to avoid disk access and memory allocations creating shaders
	Defines        ms_defines;
	char           ms_scratchBuffer[2 * 1024];
	IncludeCache * ms_includeCache;

	// ------------------------------------------------------------------
	// Phase 32 (2026-06-16): PROCESS-GLOBAL compiled-bytecode cache for the
	// D3DCompile (//hlsl) path. RUNTIME-CONFIRMED leak fix.
	//
	// The compiled-shader caches (m_container / m_nonPatchedVertexShader) live
	// INSIDE Direct3d9_VertexShaderData, which is refcounted and `delete`d when a
	// cantina cell unloads (ShaderImplementation.cpp:2301 release() -> delete this
	// at refcount 0 -> ~Direct3d9_VertexShaderData destroys those caches). Re-entry
	// -> fresh object -> empty cache -> D3DCompile RECOMPILES every cell cycle.
	// Each recompile pays a large per-compile transient (the HlslRewrite + the
	// D3DCompile working set) that fragments the 32-bit heap -> linear private-byte
	// growth (+144 MB / 2 min in the cantina A/B). The old D3DXCompileShader path
	// was FLAT because a single compile is harmless.
	//
	// This cache stores BYTECODE (the post-compile token stream) at FILE scope, so
	// it SURVIVES Direct3d9_VertexShaderData destruction (created in install(),
	// freed in remove() -- plugin shutdown only). A cache HIT needs neither the
	// rewrite NOR D3DCompile: we CreateVertexShader directly from the cached bytes.
	// Compile-once-per-session, reuse across all cell cycles. This mirrors the
	// D3D11 Direct3d11_ShaderCache (hashSource/tryLoad/store) -- adapted to an
	// in-memory map of owned byte vectors (no on-disk .cso; the D3D9 transient cost
	// is process-heap fragmentation, which an in-memory cache fully eliminates).
	//
	// KEY = (.vsh identity, textureCoordinateSetKey, target profile). These are the
	// EXACT inputs that determine the emitted bytecode AND are knowable BEFORE the
	// rewrite, so a hit skips both the rewrite and D3DCompile. The generated defines
	// (texcoord-set remap macros + DECLARE_textureCoordinateSets) are a deterministic
	// function of (m_textureCoordinateSetTags, textureCoordinateSetKey), and the tag
	// list is itself fixed by the .vsh -- so keying on (file, key, target) is both
	// sound and complete. The non-patched site calls createVertexShader(0) with
	// m_textureCoordinateSetTags==NULL, so it naturally keys on texcoordKey=0 with no
	// special-casing (the patched site supplies the real per-key value).
	//
	// THREAD-SAFETY: shader compilation runs on the render/main thread, same as the
	// surrounding createVertexShader / ms_defines / ms_scratchBuffer (all unguarded
	// process-wide statics). We MATCH that (no locking added) -- ASSUMPTION: single
	// compile thread, identical to the pre-existing code's contract.
	//
	// MEMORY: bounded by the unique (.vsh x texcoordKey) set for the session (a few
	// hundred entries x a few KB bytecode each = trivial); freed in remove().
	typedef std::vector<unsigned char>             VsByteCode;
	typedef std::map<std::string, VsByteCode>      VsByteCodeCache;
	VsByteCodeCache * ms_byteCodeCache;

	// CONSULT-45 fix #1: rewrite-version baked into the DISK shader-cache content hash.
	// BUMP THIS when Direct3d9_HlslRewrite rules/gates change OR the texcoord macro
	// generation below changes -- it invalidates stale on-disk .cso bytecode (the source
	// text + defines are also hashed, but the rewrite transforms the source AFTER hashing
	// the raw text, and rewritten #includes never appear in m_compileText, so this version
	// is the catch-all for rewrite/include changes). See Direct3d9_ShaderCache::hashSource.
	const uint32 cs_shaderCacheRewriteVersion = 1;

	// Build the bytecode-cache key. Mirrors Direct3d11_ShaderCache::hashSource's
	// "inputs that determine the bytecode" contract, but as a readable composite
	// string (the D3D9 set is small; a string map is simpler than FNV and avoids
	// any collision concern). '\x1f' (unit separator) delimits the three fields so
	// no field-boundary ambiguity is possible.
	std::string buildByteCodeCacheKey(char const *fileName, uint32 textureCoordinateSetKey, char const *target)
	{
		char keyBuf[32];
		std::string key(fileName ? fileName : "");
		key += '\x1f';
		_snprintf(keyBuf, sizeof(keyBuf), "%08x", static_cast<unsigned>(textureCoordinateSetKey));
		keyBuf[sizeof(keyBuf) - 1] = '\0';
		key += keyBuf;
		key += '\x1f';
		key += (target ? target : "");
		return key;
	}

	// ------------------------------------------------------------------
	// Phase 32 Fix B (2026-06-16): the HLSL `//hlsl` .vsh compile path now runs through
	// D3DCompile (d3dcompiler_47), NOT D3DXCompileShader. D3DX was statically compiled FROM
	// SOURCE into the D3D9 plugin (gl05/gl07); built with the modern MSVC toolchain its shader
	// compiler unmasked FP exceptions and faulted (0xC0000090) during its post-compile FP
	// cleanup on certain bump/dot3 vertex shaders -- the retail VS2003-built D3DX did not.
	// D3DCompile does not have that bug (the D3D11 path has used it since Phase 11). The
	// textual modernization needed by the strict modern compiler (SM4+ reserved keywords,
	// stacked struct-member register bindings) is handled by Direct3d9_HlslRewrite (Rules
	// A/B/C; Rule D disabled for D3D9; Rule E gated OFF) applied to the main source and every
	// #include before this call.
	//
	// ABI: D3DXMACRO / ID3DXInclude / ID3DXBuffer are binary-layout-identical to the
	// d3dcompiler types (D3D_SHADER_MACRO / ID3DInclude / ID3DBlob), so the surrounding
	// D3DX-typed code AND the separate D3DXAssembleShader asm path stay untouched; we
	// reinterpret-cast only at this single call boundary (RESEARCH Pattern 1).
	//
	// Fix-A SEH guard RETAINED -- FINAL DISPOSITION (Phase 33 Plan 02, D-05 / reviews fix #8):
	// BOTH compile paths are now off D3DX (the //hlsl path -> D3DCompile in Phase 32-02, and the
	// //asm path -> D3DAssemble in this plan's Task 1). Per D-05 the default disposition is
	// RETAIN-with-comment, and that is the decision taken here. Rationale (reviews fix #8 / Opus,
	// MEDIUM): Fix-A's __try/__except guards a REAL observed crash -- the D3DX FP fault 0xC0000090.
	// D3DCompile / D3DAssemble are believed immune to that fault, but the guard is cheap insurance
	// against the FP-crash class resurfacing on an unexercised .vsh; retiring it on a code path the
	// Phase 32-01 6/96-//asm sample never covered carries render risk that the build link-grep does
	// NOT retire. So the guard STAYS as belt-and-suspenders -- the no-render-risk path. (Note: the
	// guard wraps only the HLSL D3DCompile call; the asm D3DAssemble call is, and always was, a
	// direct FATAL-on-failure -- it does not need the SEH wrapper because the D3DX FP fault was
	// specific to the D3DX HLSL compiler's post-compile FP cleanup, which D3DAssemble does not run.)
	// __try/__except cannot live in a function needing C++ object unwinding (C2712), hence the
	// helper. One guard covers all VSPS plugins (gl05/07).
	inline int direct3d9CompileFpExceptionFilter(unsigned long code)
	{
		// STATUS_FLOAT_* SEH codes occupy 0xC000008D..0xC0000093 (denormal..underflow).
		return (code >= 0xC000008DUL && code <= 0xC0000093UL) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH;
	}

	HRESULT compileVertexShaderFpGuarded(
		LPCSTR src, UINT srcLen, D3D_SHADER_MACRO const *defines, ID3DInclude *includeHandler,
		LPCSTR functionName, LPCSTR profile, DWORD flags,
		ID3DBlob **outShader, ID3DBlob **outErrors)
	{
		HRESULT hr = E_FAIL;
		__try
		{
			// Fix B: D3DCompile on the now-native D3D_SHADER_MACRO/ID3DInclude/ID3DBlob
			// types (Phase 33-03 swapped the residual D3DX typedefs to their ABI-identical
			// native counterparts so d3dx9.h could be dropped).
			// `flags` carries D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY ONLY -- explicitly NOT
			// D3DCOMPILE_PACK_MATRIX_ROW_MAJOR (review fix #4): the D3D9 path uses explicit
			// transposed constant uploads, not a row-major cbuffer contract; copying gl11's
			// row-major flag would silently change matrix packing -> wrong transforms.
			hr = D3DCompile(
				src, srcLen, NULL,
				defines,
				includeHandler,
				functionName, profile, flags, 0,
				outShader,
				outErrors);
			_clearfp();
		}
		__except (direct3d9CompileFpExceptionFilter(GetExceptionCode()))
		{
			// Belt-and-suspenders: D3DCompile is immune to the D3DX FP fault, but keep the
			// recovery path until Fix-A is formally retired in Wave 2. If a fault somehow
			// occurs, the bytecode is fully produced before any post-compile FP cleanup;
			// reset the FPU and report success when we have bytecode.
			_fpreset();
			_clearfp();
			hr = (outShader && *outShader) ? S_OK : E_FAIL;
		}
		return hr;
	}
}
using namespace Direct3d9_VertexShaderDataNamespace;

// ======================================================================

#define SCRATCH_STRING(a, b)	strcpy(scratchBuffer, a); DEBUG_FATAL(b != strlen(a), ("wrong string length (was: %d should: %d", b, strlen(a))); scratchBuffer += b
#define SCRATCH_TAG(a)	      ConvertTagToString(a, scratchBuffer); scratchBuffer += 4
#define SCRATCH_INT(a)	      _itoa(a, scratchBuffer, 10); scratchBuffer += strlen(scratchBuffer)
#define SCRATCH_DONE()	      *scratchBuffer = '\0'; scratchBuffer += 1

// ======================================================================

Direct3d9_VertexShaderDataNamespace::Include::Include(CrcString const &fileName, AbstractFile * file)
:
	m_fileName(fileName),
	m_length(0),
	m_data(0)
{
	// Fix B (Phase 32) -- include-cache rewrite-BEFORE-insert (review fix Cursor C2):
	// Unlike gl11 (which rewrites on EVERY Open and never caches), the D3D9 IncludeHandler
	// caches the include bytes. Apply Direct3d9_HlslRewrite to the resolved .inc text HERE,
	// in the Include ctor, so the REWRITTEN bytes are what get stored in ms_includeCache
	// (the ctor runs before the cache insert in IncludeHandler::Open). This guarantees the
	// 2nd #include of the same module (e.g. vertex_shader_constants.inc) returns rewritten
	// text on the cache-hit path too -- never the raw stacked `: register()` D3DCompile rejects.
	// applyToIncludeBuffer takes ownership of the readEntireFileAndClose() buffer (delete[]s it
	// on rewrite) and returns a buffer we own + free in ~Include via delete[]. Capture the raw
	// length BEFORE the read closes the file.
	int const rawLength = file->length();
	std::size_t outLength = 0;
	m_data = reinterpret_cast<byte *>(Direct3d9_HlslRewrite::applyToIncludeBuffer(
		reinterpret_cast<unsigned char *>(file->readEntireFileAndClose()),
		static_cast<std::size_t>(rawLength), outLength, m_fileName.getString()));
	m_length = static_cast<int>(outLength);
}

// ----------------------------------------------------------------------

Direct3d9_VertexShaderDataNamespace::Include::~Include()
{
	delete [] m_data;
}

// ======================================================================

HRESULT Direct3d9_VertexShaderDataNamespace::IncludeHandler::Open(D3D_INCLUDE_TYPE, LPCSTR pFileName, LPCVOID, LPCVOID *ppData, UINT *pBytes)
{
	// hack to support relative path includes when using the command line compiler
	if (strncmp(pFileName, "../../", 6) == 0)
		pFileName += 6;

	if (Direct3d9::engineOwnsWindow())
	{
		TemporaryCrcString findFileName(pFileName, true);
		IncludeCache::iterator i = ms_includeCache->find(&findFileName);
		if (i == ms_includeCache->end())
		{
			AbstractFile *file = TreeFile::open(pFileName, AbstractFile::PriorityData, true);
			if (!file)
			{
				DEBUG_FATAL(true, ("could not open include %s", pFileName));
				return STG_E_FILENOTFOUND;
			}

			Include *include = new Include(findFileName, file);
			delete file;

			i = ms_includeCache->insert(IncludeCache::value_type(&include->m_fileName, include)).first;
		}

		*ppData = reinterpret_cast<void*>(i->second->m_data);
		*pBytes = i->second->m_length;
	}
	else
	{
		AbstractFile *file = TreeFile::open(pFileName, AbstractFile::PriorityData, true);
		if (!file)
		{
			DEBUG_FATAL(true, ("could not open include %s", pFileName));
			return STG_E_FILENOTFOUND;
		}

		// Fix B (Phase 32): non-engine-owns (viewer/tool) path -- rewrite the resolved include
		// too, so D3DCompile sees the same modernized text as the cached engine path. The matching
		// Close() delete[]s whatever pointer we return; applyToIncludeBuffer owns the read buffer.
		int const rawLength = file->length();
		unsigned char *raw = reinterpret_cast<unsigned char *>(file->readEntireFileAndClose());
		delete file;
		std::size_t outLength = 0;
		*ppData = Direct3d9_HlslRewrite::applyToIncludeBuffer(raw, static_cast<std::size_t>(rawLength), outLength, pFileName);
		*pBytes = static_cast<UINT>(outLength);
	}

	return S_OK;
}

// ----------------------------------------------------------------------

HRESULT Direct3d9_VertexShaderDataNamespace::IncludeHandler::Close(LPCVOID data)
{
	// this is a hack to unload include files for the viewer so that it can be used for interactive shader editing
	if (!Direct3d9::engineOwnsWindow())
		delete [] (const_cast<byte *>(reinterpret_cast<byte const *>(data)));

	return 0;
}

// ----------------------------------------------------------------------

void Direct3d9_VertexShaderDataNamespace::getToken(const char *& s, char *d)
{
	// handle end of the buffer
	if (s == 0 || *s == '\0')
	{
		*d = '\0';
		return;
	}

	// skip leading whitespace
	while (*s && (*s == ' ' || *s == '\r' || *s == '\n' || *s == '\t'))
		++s;

	// copy characters until a NUL or whitespace
	for ( ; *s && *s != ' ' && *s != '\n' && *s != '\r' && *s != '\t'; ++s, ++d)
		*d = *s;

	// terminate the destination buffer
	*d = '\0';
}

// ----------------------------------------------------------------------

void Direct3d9_VertexShaderDataNamespace::skipRestOfTheLine(const char *& s)
{
	// handle end of the buffer
	if (s == 0 || *s == '\0')
		return;

	// skip characters until a NUL or whitespace
	for ( ; *s && *s != '\n' && *s != '\r' ; )
		if (*s == '\\')
		{
			// handle a quoted newline
			++s;
			if (*s == '\r')
				++s;
			if (*s == '\n')
				++s;
		}
		else
		{
			++s;
		}
}

// ======================================================================

void Direct3d9_VertexShaderData::install()
{
	ms_defines.reserve(32);
	ms_includeCache = new IncludeCache;
	// Phase 32 (2026-06-16): process-global compiled-bytecode cache (survives
	// per-object destruction; freed in remove() at plugin shutdown only).
	ms_byteCodeCache = new VsByteCodeCache;

	// CONSULT-45 fix #1: install the DISK bytecode cache (L2 -- survives launches so the
	// FIRST-ever zone-in is a hit after one warm run). The client's CWD at runtime IS the
	// staging dir (stage/ for Win32, stage-x64/ for x64 -- verified: gl11's "stage/shader-cache/"
	// landed double-nested at stage/stage/shader-cache/). So use a BARE relative dir: it lands
	// cleanly per-platform at stage/shader-cache-d3d9/ resp. stage-x64/shader-cache-d3d9/.
	// Gitignored under stage/* . VS bytecode is GPU-arch-independent, so the two are interchangeable.
	Direct3d9_ShaderCache::install("shader-cache-d3d9/");
}

// ----------------------------------------------------------------------

void Direct3d9_VertexShaderData::remove()
{
	if (ms_includeCache)
	{
		while (!ms_includeCache->empty())
		{
			IncludeCache::iterator i = ms_includeCache->begin();
			Include *include = i->second;
			ms_includeCache->erase(i);
			delete include;
		}
		delete ms_includeCache;
		ms_includeCache = NULL;
	}

	// Phase 32 (2026-06-16): free the process-global bytecode cache. The owned
	// std::vector<unsigned char> entries free their bytes via the map dtor.
	delete ms_byteCodeCache;
	ms_byteCodeCache = NULL;

	// CONSULT-45 fix #1: tear down the disk cache (logs hit/miss/store stats; leaves
	// the .cso files on disk for the next launch).
	Direct3d9_ShaderCache::remove();
}

// ----------------------------------------------------------------------

Direct3d9_VertexShaderData::Direct3d9_VertexShaderData(ShaderImplementation::Pass::VertexShader const & vertexShader)
: ShaderImplementationPassVertexShaderGraphicsData(),
	m_vertexShader(&vertexShader),
	m_hlsl(false),
	m_compileText(NULL),
	m_compileTextLength(0),
	m_textureCoordinateSetTags(NULL),
	m_container(NULL),
	m_nonPatchedVertexShader(NULL),
	m_lastRequestedKey(0xFFFFFFFF),
	m_lastReturnedValue(NULL)
{
	char const * text = m_vertexShader->m_text;

	// here's two samples of the data we're parsing
	/*

	//hlsl vs_1_1
	#define textureCoordinateSetMAIN        textureCoordinateSet0
	#define DECLARE_textureCoordinateSet0   float2 textureCoordinateSet0 : TEXCOORD0 : register(v7);

	//asm vs_1_1
	#define maxTextureCoordinate      2
	#define vTextureCoordinateSetDTLA vTextureCoordinateSet0
	#define vTextureCoordinateSetDTLB vTextureCoordinateSet1
	#define vTextureCoordinateSetMASK vTextureCoordinateSet2

	*/

	bool assembly = false;
	for (bool done = false; !done; )
	{
		// remember where we were before the first bad token
		m_compileText = text;

		char token[128];
		getToken(text, token);

		if (strcmp(token, "//hlsl") == 0)
		{
			m_hlsl = true;
			skipRestOfTheLine(text);
		}
		else if (strcmp(token, "//asm") == 0)
		{
			assembly = true;
			skipRestOfTheLine(text);
		}
		else if (strcmp(token, "#define") == 0)
		{
			getToken(text, token);

			char * tag = 0;
			if (m_hlsl)
			{
				if (strncmp(token, "textureCoordinateSet", 20) == 0)
					tag = token + 20;
			}
			if (assembly)
			{
				if (strncmp(token, "vTextureCoordinateSet", 21) == 0)
					tag = token + 21;
			}

			if (tag)
			{
				if (!m_textureCoordinateSetTags)
					m_textureCoordinateSetTags = new TextureCoordinateSetTags();
				m_textureCoordinateSetTags->push_back(ConvertStringToTag(tag));
			}

			skipRestOfTheLine(text);
		}
		else
		{
			done = true;
		}
	}

	DEBUG_FATAL(!assembly && !m_hlsl, ("Could not determine shader language for %s",  m_vertexShader->getFilename()));
	m_compileTextLength = strlen(m_compileText);
}

// ----------------------------------------------------------------------

Direct3d9_VertexShaderData::~Direct3d9_VertexShaderData()
{
	m_vertexShader = NULL;

	if (m_nonPatchedVertexShader)
	{
		m_nonPatchedVertexShader->Release();
		m_nonPatchedVertexShader = NULL;
	}

	if (m_container)
	{
		while (!m_container->empty())
		{
			m_container->begin()->second->Release();
			m_container->erase(m_container->begin());
		}
		delete m_container;
	}

	delete m_textureCoordinateSetTags;
	m_compileText = 0;
}

// ----------------------------------------------------------------------

IDirect3DVertexShader9 * Direct3d9_VertexShaderData::createVertexShader(uint32 textureCoordinateSetKey) const
{
	ID3DBlob *compiledShader = NULL;

	ms_defines.clear();
	char const * target = NULL;
	if (Direct3d9::getShaderCapability() >= ShaderCapability(2,0))
	{
		target = "vs_2_0";
	}
	else
	{
		// Fix B (Phase 32 / review fix #5 / D-06): D3DCompile (d3dcompiler_47) DROPPED vs_1_1
		// support. The legacy `target = "vs_1_1"` here would hand D3DCompile a profile it rejects
		// -> a compile FATAL or (worse) a silently-nulled VS / skipped draw on a <SM2.0 machine.
		// Promote unconditionally to vs_2_0 so the path can never compile-FATAL on the profile and
		// never silently null a VS. A genuine <2.0 GPU is unsupported on this modern toolchain; the
		// DEBUG_FATAL makes that assumption loud in the Debug build (the HLSL path only ever runs
		// on shader-capable hardware; getShaderCapability() < 2.0 is not a configuration we ship).
		DEBUG_FATAL(true, ("Direct3d9_VertexShaderData: getShaderCapability() < 2.0 -- D3DCompile dropped vs_1_1; promoting to vs_2_0 (unsupported config)"));
		target = "vs_2_0";
	}

	// ------------------------------------------------------------------
	// Phase 32 (2026-06-16): PROCESS-GLOBAL bytecode-cache lookup. ONLY the
	// //hlsl (D3DCompile) path leaks -- it recompiles on every cell cycle because
	// the per-object caches die with the object. The //asm (D3DXAssembleShader)
	// path is unaffected (flat in the A/B), so it stays out of this cache.
	//
	// HIT: CreateVertexShader directly from the cached bytecode and RETURN early --
	// skipping applyToMainSource AND D3DCompile entirely (the whole point: no
	// rewrite, no compile, no per-compile heap transient on re-entry). The cache key
	// is computed from (.vsh identity, texcoordKey, target) -- all known here,
	// BEFORE the rewrite -- so the hit short-circuits the expensive work.
	std::string byteCodeCacheKey;
	if (m_hlsl && ms_byteCodeCache)
	{
		byteCodeCacheKey = buildByteCodeCacheKey(
			m_vertexShader->m_fileName.getString(), textureCoordinateSetKey, target);

		VsByteCodeCache::const_iterator hit = ms_byteCodeCache->find(byteCodeCacheKey);
		if (hit != ms_byteCodeCache->end())
		{
			VsByteCode const & bytes = hit->second;
			IDirect3DVertexShader9 *cachedVertexShader = NULL;
			HRESULT const cachedHr = Direct3d9::getDevice()->CreateVertexShader(
				reinterpret_cast<DWORD const *>(bytes.empty() ? NULL : &bytes.front()), &cachedVertexShader);
			FATAL_DX_HR("CreateVertexShader (bytecode-cache hit) failed %s", cachedHr);
			NOT_NULL(cachedVertexShader);
			return cachedVertexShader;
		}
	}

	const int numberOfTextureCoordinateSets = 8;
	if (m_hlsl)
	{
		if (m_textureCoordinateSetTags)
		{
			int textureCoordinateSetDimension[numberOfTextureCoordinateSets] = { 0, 0, 0, 0, 0, 0, 0, 0 };

			char *scratchBuffer = ms_scratchBuffer;
			{
				int const size = m_textureCoordinateSetTags->size();
				for (int i = 0; i < size; ++i)
				{
					int const textureCoordinateSet = (textureCoordinateSetKey >> (i * 3)) & 7;
					int const dimension = ((*m_textureCoordinateSetTags)[i] == TAG_DOT3) ? 4 : 2;
					DEBUG_FATAL(textureCoordinateSetDimension[textureCoordinateSet] != 0 && textureCoordinateSetDimension[textureCoordinateSet] != dimension, ("Competing dimensions (existing %d, new %d) for texture coordinate %d", textureCoordinateSetDimension[textureCoordinateSet], dimension, textureCoordinateSet));
					textureCoordinateSetDimension[textureCoordinateSet] = dimension;

					D3D_SHADER_MACRO macro;

					// here's an example of what we are defining:
					// #define textureCoordinateSetMAIN textureCoordinateSet0

					macro.Name = scratchBuffer;
					SCRATCH_STRING("textureCoordinateSet", 20);
					SCRATCH_TAG((*m_textureCoordinateSetTags)[i]);
					SCRATCH_DONE();

					macro.Definition = scratchBuffer;
					SCRATCH_STRING("textureCoordinateSet", 20);
					SCRATCH_INT(textureCoordinateSet);
					SCRATCH_DONE();

					ms_defines.push_back(macro);
				}
			}

			{
				// here's an example of what we are defining:
				// #define DECLARE_textureCoordinateSets  float2 textureCoordinateSet0 : TEXCOORD0 : register(v7);

				D3D_SHADER_MACRO macro;

				macro.Name = scratchBuffer;
				SCRATCH_STRING("DECLARE_textureCoordinateSets", 29);
				SCRATCH_DONE();

				macro.Definition = scratchBuffer;

				for (int i = 0; i < numberOfTextureCoordinateSets; ++i)
					if (textureCoordinateSetDimension[i])
					{

						SCRATCH_STRING("float", 5);
						SCRATCH_INT(textureCoordinateSetDimension[i]);
						SCRATCH_STRING(" textureCoordinateSet", 21);
						SCRATCH_INT(i);
						SCRATCH_STRING(" : TEXCOORD", 11);
						SCRATCH_INT(i);
						// Fix B (Phase 32 / review fix #7 / Pitfall 3): OMIT the D3D9-era
						// ": register(vN)" on this struct member. Modern fxc (d3dcompiler_47)
						// rejects a register binding on a struct member (X3202) even at vs_2_0.
						// This macro expands INSIDE D3DCompile AFTER applyToMainSource ran on the
						// raw source -- the pre-preprocessor Rules B/C never see this post-expansion
						// text, so the omission must be made HERE. The TEXCOORD<i> semantic alone
						// drives input binding; the gapped VSVR v# slot (v7..v14) is honored by the
						// D3DVERTEXELEMENT9 stream decl, NOT by this inline register hint (Task-2(G)
						// input-signature audit proves the dcl signature still matches VSVR).
						SCRATCH_STRING(";", 1);
					}

				SCRATCH_DONE();

				ms_defines.push_back(macro);
			}

			DEBUG_FATAL(ms_scratchBuffer + sizeof(ms_scratchBuffer) < scratchBuffer, ("Scratch buffer overflow"));
		}

		{
			D3D_SHADER_MACRO empty = { NULL, NULL };
			ms_defines.push_back(empty);
		}

		// CONSULT-45 fix #1: DISK bytecode cache (L2) lookup. Content-hashed over the FULL
		// compile inputs (source bytes + the just-built defines + target + rewrite version +
		// compiler tag), so a .vsh / HlslRewrite edit can never load stale bytecode. The
		// in-memory L1 (above) already returned on a session re-hit; reaching here means L1
		// missed, so a disk HIT skips the rewrite AND D3DCompile -- this makes the FIRST-ever
		// zone-in fast after one warm run (retail ships precompiled bytecode).
		uint64_t const diskCacheHash = Direct3d9_ShaderCache::hashSource(
			m_compileText, static_cast<std::size_t>(m_compileTextLength),
			ms_defines.empty() ? NULL : &ms_defines.front(), target, cs_shaderCacheRewriteVersion);
		{
			std::vector<unsigned char> diskBytes;
			if (Direct3d9_ShaderCache::tryLoad(diskCacheHash, diskBytes) && !diskBytes.empty())
			{
				IDirect3DVertexShader9 *diskVertexShader = NULL;
				HRESULT const diskHr = Direct3d9::getDevice()->CreateVertexShader(
					reinterpret_cast<DWORD const *>(&diskBytes.front()), &diskVertexShader);
				FATAL_DX_HR("CreateVertexShader (disk shader-cache hit) failed %s", diskHr);
				NOT_NULL(diskVertexShader);
				// Populate the in-memory L1 so re-entry this session skips disk I/O.
				if (ms_byteCodeCache)
					(*ms_byteCodeCache)[byteCodeCacheKey].assign(diskBytes.begin(), diskBytes.end());
				return diskVertexShader;
			}
		}

		IncludeHandler includeHandler;
		ID3DBlob *error = NULL;
		// Fix B (Phase 32): apply Rule A (and the conditional rules) to the MAIN shader source
		// before compiling -- the #includes are rewritten in IncludeHandler::Open / the Include
		// ctor. Then compile via D3DCompile (inside the retained Fix-A SEH guard --
		// compileVertexShaderFpGuarded now wraps D3DCompile). Flags =
		// D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY ONLY (NO PACK_MATRIX_ROW_MAJOR -- review fix #4).
		std::vector<char> rewrittenSource;
		Direct3d9_HlslRewrite::applyToMainSource(m_compileText, static_cast<std::size_t>(m_compileTextLength), rewrittenSource, m_vertexShader->m_fileName.getString());
		HRESULT result = compileVertexShaderFpGuarded(rewrittenSource.data(), static_cast<UINT>(rewrittenSource.size()), &(ms_defines.front()), &includeHandler, "main", target, D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY, &compiledShader, &error);

		//-----------------------------------------------------------------------------------
		// DBE - I was getting strange Float Invalid Operation Exceptions (0xC0000090) in the
		// Debug Build on floating point instructions which I eventually traced back to the FPU
		// being left in some bad state after a call to D3DXCompileShader (on a certain .vsh).
		// I also found that calling '_clearfp()' cleared up the problem.
		// (Retained belt-and-suspenders; the SEH guard above is the actual fix for the fault
		//  that occurs INSIDE the call, which this post-call _clearfp cannot reach.)
		_clearfp();
		//-----------------------------------------------------------------------------------

		FATAL(FAILED(result), ("Could not compile shader %s %d (%s)", m_vertexShader->m_fileName.getString(), HRESULT_CODE(result), error ? error->GetBufferPointer() : "none"));

		if (error)
		{
			DEBUG_REPORT_LOG_PRINT(true, ("%s", error->GetBufferPointer()));
			error->Release();
		}

		// Phase 32 (2026-06-16): cache MISS just compiled -- copy the bytecode bytes
		// into the process-global cache so the NEXT cell cycle (a fresh
		// Direct3d9_VertexShaderData with an empty per-object cache) hits and skips
		// both the rewrite and D3DCompile. We copy into an OWNED std::vector because
		// compiledShader->Release() (below, shared with the asm path) frees the blob
		// right after. The engine blob releases (error->Release above,
		// compiledShader->Release below) are correct and left untouched.
		if (ms_byteCodeCache && compiledShader)
		{
			SIZE_T const blobSize = compiledShader->GetBufferSize();
			if (blobSize > 0)
			{
				unsigned char const * const blobBytes =
					static_cast<unsigned char const *>(compiledShader->GetBufferPointer());
				(*ms_byteCodeCache)[byteCodeCacheKey].assign(blobBytes, blobBytes + blobSize);
			}
		}

		// CONSULT-45 fix #1: persist the freshly compiled bytecode to the DISK cache so the
		// NEXT launch's first zone-in is a hit (store is non-fatal / opportunistic).
		if (compiledShader)
		{
			SIZE_T const diskBlobSize = compiledShader->GetBufferSize();
			if (diskBlobSize > 0)
				Direct3d9_ShaderCache::store(diskCacheHash, compiledShader->GetBufferPointer(),
					static_cast<std::size_t>(diskBlobSize));
		}
	}
	else
	{
		if (m_textureCoordinateSetTags)
		{
#ifdef _DEBUG
			int textureCoordinateSetDimension[numberOfTextureCoordinateSets] = { 0, 0, 0, 0, 0, 0, 0, 0 };
#endif

			int maxTextureCoordinateSet = -1;

			char *scratchBuffer = ms_scratchBuffer;
			int const size = m_textureCoordinateSetTags->size();
			for (int i = 0; i < size; ++i)
			{
				int const textureCoordinateSet = (textureCoordinateSetKey >> (i * 3)) & 7;

#ifdef _DEBUG
				int const dimension = ((*m_textureCoordinateSetTags)[i] == TAG_DOT3) ? 4 : 2;
				DEBUG_FATAL(textureCoordinateSetDimension[textureCoordinateSet] != 0 && textureCoordinateSetDimension[textureCoordinateSet] != dimension, ("Competing dimensions (existing %d, new %d) for texture coordinate %d", textureCoordinateSetDimension[textureCoordinateSet], dimension, textureCoordinateSet));
				textureCoordinateSetDimension[textureCoordinateSet] = dimension;
#endif

				if (textureCoordinateSet > maxTextureCoordinateSet)
					maxTextureCoordinateSet = textureCoordinateSet;

				D3D_SHADER_MACRO macro;

				// here's an example of what we are defining:
				// #define vTextureCoordinateSetDTLA vTextureCoordinateSet0

				macro.Name = scratchBuffer;
				SCRATCH_STRING("vTextureCoordinateSet", 21);
				SCRATCH_TAG((*m_textureCoordinateSetTags)[i]);
				SCRATCH_DONE();

				macro.Definition = scratchBuffer;
				SCRATCH_STRING("vTextureCoordinateSet", 21);
				SCRATCH_INT(textureCoordinateSet);
				SCRATCH_DONE();

				ms_defines.push_back(macro);
			}

			{
				// here's an example of what we are defining:
				// #define maxTextureCoordinate 2
				D3D_SHADER_MACRO macro;

				macro.Name = "maxTextureCoordinate";

				macro.Definition = scratchBuffer;
				SCRATCH_INT(maxTextureCoordinateSet);
				SCRATCH_DONE();

				ms_defines.push_back(macro);
			}

			DEBUG_FATAL(ms_scratchBuffer + sizeof(ms_scratchBuffer) < scratchBuffer, ("Scratch buffer overflow"));
		}

		{
			D3D_SHADER_MACRO d = { "TARGET", target };
			ms_defines.push_back(d);
		}

		{
			D3D_SHADER_MACRO empty = { NULL, NULL };
			ms_defines.push_back(empty);
		}

		IncludeHandler includeHandler;

		// Phase 33 Plan 02 (D-04/D-05): assemble through D3DAssemble (d3dcompiler_47)
		// instead of D3DXAssembleShader -- the last D3DX compile dependency off the D3D9
		// path. Resolve-once via a static-local-cached GetProcAddress on the undecorated
		// export (D3DAssemble is not in the public d3dcompiler.h). Do NOT LoadLibrary /
		// GetProcAddress per compile (refcount/reload leak).
		static const HMODULE          s_d3dCompilerModule = LoadLibraryA("d3dcompiler_47.dll");
		static const PFN_D3DAssemble  s_d3dAssemble       =
			s_d3dCompilerModule ? reinterpret_cast<PFN_D3DAssemble>(GetProcAddress(s_d3dCompilerModule, "D3DAssemble")) : NULL;

		// D-06: a missing fn pointer (d3dcompiler_47.dll absent or no D3DAssemble export)
		// is LOUD (FATAL), never a silently-nulled VS / skipped draw. gl11's
		// [P19VSFALLBACK] does NOT transfer -- D3D9 natively executes the assembled bytecode.
		FATAL(!s_d3dAssemble, ("Direct3d9_VertexShaderData: D3DAssemble unavailable (d3dcompiler_47.dll missing or no D3DAssemble export) while assembling //asm shader %s", m_vertexShader->m_fileName.getString()));

		// ABI-identical reinterpret-cast at the call boundary (D3DXMACRO -> D3D_SHADER_MACRO,
		// ID3DXInclude -> ID3DInclude, ID3DXBuffer -> ID3DBlob). Same defines array, same
		// TreeFile-routed IncludeHandler, Flags=0 -- the exact dialect the 32-01 gate validated.
		ID3DBlob *assembleErrors = NULL;
		HRESULT result = s_d3dAssemble(
			m_compileText, static_cast<SIZE_T>(m_compileTextLength), NULL,
			reinterpret_cast<const D3D_SHADER_MACRO *>(&(ms_defines.front())),
			reinterpret_cast<ID3DInclude *>(&includeHandler), 0,
			reinterpret_cast<ID3DBlob **>(&compiledShader),
			reinterpret_cast<ID3DBlob **>(&assembleErrors));
		// Loud failure path: surface the assembler error-blob text (D-06).
		FATAL(FAILED(result), ("Could not assemble shader %s %d (%s)", m_vertexShader->m_fileName.getString(), HRESULT_CODE(result), assembleErrors ? static_cast<char const *>(assembleErrors->GetBufferPointer()) : "none"));
		if (assembleErrors)
		{
			DEBUG_REPORT_LOG_PRINT(true, ("%s", assembleErrors->GetBufferPointer()));
			assembleErrors->Release();
		}
	}

	// create the vertex shader	from the binary token stream
	IDirect3DVertexShader9 *vertexShader = NULL;
	HRESULT const hresult = Direct3d9::getDevice()->CreateVertexShader(reinterpret_cast<DWORD const *>(compiledShader->GetBufferPointer()), &vertexShader);
	FATAL_DX_HR("CreateVertexShader failed %s", hresult);
	NOT_NULL(vertexShader);
	compiledShader->Release();

	return vertexShader;
}

// ----------------------------------------------------------------------

IDirect3DVertexShader9 * Direct3d9_VertexShaderData::getVertexShader(uint32 textureCoordinateSetKey) const
{
	if(m_lastRequestedKey == textureCoordinateSetKey)
		return m_lastReturnedValue;
#ifdef _DEBUG
	if (!ConfigDirect3d9::getCreateShaders())
		return 0;
#endif

	// non-patched vertex shaders can always return the same value
	if (!m_textureCoordinateSetTags)
	{
		if (!m_nonPatchedVertexShader)
			m_nonPatchedVertexShader = createVertexShader(0);

		return m_nonPatchedVertexShader;
	}

	if (!m_container)
		m_container = new Container();

	// see if we have a vertex shader already created for this set of texture coordinate sets
	Container::const_iterator i = m_container->find(textureCoordinateSetKey);
	if (i == m_container->end())
	{
		// create it and put it into the map
		i = m_container->insert(Container::value_type(textureCoordinateSetKey, createVertexShader(textureCoordinateSetKey))).first;
	}

	m_lastRequestedKey = textureCoordinateSetKey;
	m_lastReturnedValue = i->second;
	return i->second;
}

// ======================================================================

#endif
