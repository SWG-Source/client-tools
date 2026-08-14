// ======================================================================
//
// Direct3d11_ShaderCompiler.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"

#include <algorithm>
#include "Direct3d11_ShaderCompiler.h"

#include "Direct3d11_Device.h"
#include "Direct3d11_Metrics.h"
#include "Direct3d11_ShaderCache.h"
#include "Direct3d11_ShaderPrograms.h"
#include "Direct3d11_ShaderSource.h"

#include "fileInterface/AbstractFile.h"
#include "sharedFile/TreeFile.h"

#include <d3dcompiler.h>
#include <windows.h>
#include <map>
#include <string>

// ======================================================================

namespace Direct3d11_ShaderCompilerNamespace
{
	char const *const cms_vertexShaderTarget = "vs_4_0";
	char const *const cms_pixelShaderTarget = "ps_4_0";

	// Backwards compatibility is mandatory; level 3 optimisation matches what the
	// shipped DX9 pixel programs were built with. PACK_MATRIX_ROW_MAJOR is
	// deliberately absent -- see the header.
	UINT const cms_compileFlags = D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY | D3DCOMPILE_OPTIMIZATION_LEVEL3;

	struct CachedInclude
	{
		char *data;
		int length;
	};

	typedef std::map<std::string, CachedInclude> IncludeCache;
	IncludeCache ms_includeCache;
	bool ms_installed;

	// ID3DInclude, not ID3DXInclude: same shape, different interface, and the D3DX
	// one belongs to a DirectX SDK this backend does not link.
	class IncludeHandler : public ID3DInclude
	{
	public:
		virtual HRESULT STDMETHODCALLTYPE Open(D3D_INCLUDE_TYPE includeType, LPCSTR fileName, LPCVOID parentData, LPCVOID *data, UINT *bytes);
		virtual HRESULT STDMETHODCALLTYPE Close(LPCVOID data);
	};

	IncludeHandler ms_includeHandler;

	ID3DBlob *compile(char const *source, int sourceLength, char const *name, D3D_SHADER_MACRO const *macros, char const *target);
	bool refuseIfNotHlsl(char const *source, int sourceLength, char const *name);
	ID3DBlob *compilePatched(char const *source, int sourceLength, char const *name, D3D_SHADER_MACRO const *macros, char const *target, bool isVertexProgram);
	void reportCompilerVersion();
} // namespace Direct3d11_ShaderCompilerNamespace
using namespace Direct3d11_ShaderCompilerNamespace;

// ======================================================================
/**
 * Resolve an #include out of the TRE set.
 *
 * The leading "../../" strip is not cosmetic. Shader sources carry paths written
 * for a command-line compiler invoked from a directory two levels below the
 * asset root, and TreeFile paths are root-relative, so the prefix has to come off
 * or nothing resolves. DX9 does the same thing at
 * Direct3d9_VertexShaderData.cpp:207-209.
 *
 * Includes are cached because the corpus shares a handful of .inc files across
 * hundreds of programs, and re-reading them out of the TRE stack for each one is
 * pure waste. DX9 caches only when the engine owns the window -- that is, only in
 * the game and not in the tools -- which is a reasonable way to stop a tool
 * serving stale content while someone edits an include. Here the cache is always
 * on and flushIncludeCache() exists for that case instead, so the game path is
 * not paying for the tool's problem.
 */

HRESULT STDMETHODCALLTYPE Direct3d11_ShaderCompilerNamespace::IncludeHandler::Open(D3D_INCLUDE_TYPE, LPCSTR fileName, LPCVOID, LPCVOID *data, UINT *bytes)
{
	NOT_NULL(fileName);
	NOT_NULL(data);
	NOT_NULL(bytes);

	char const *path = fileName;
	if (strncmp(path, "../../", 6) == 0)
		path += 6;

	IncludeCache::const_iterator const i = ms_includeCache.find(path);
	if (i != ms_includeCache.end())
	{
		*data = i->second.data;
		*bytes = static_cast<UINT>(i->second.length);
		return S_OK;
	}

	// Two includes are served from this DLL rather than from the TRE set, because the
	// shipped copies are incompatible with a shader compiled from source -- one puts
	// textureFactor on a register the engine fills with a negative value, the other binds
	// a register to an aggregate, which d3dcompiler_47 refuses outright. See
	// Direct3d11_ShaderSource.h for the evidence on both.
	{
		int overrideLength = 0;
		char const *const overrideText = Direct3d11_ShaderSource::getIncludeOverride(path, overrideLength);
		if (overrideText)
		{
			CachedInclude cached;
			cached.length = overrideLength;
			cached.data = new char[overrideLength];
			memcpy(cached.data, overrideText, static_cast<size_t>(overrideLength));

			IGNORE_RETURN(ms_includeCache.insert(std::make_pair(std::string(path), cached)));

			// Once per distinct include, because a shader compile error is reported against a
			// FILE and a LINE, and neither means anything unless it is known which copy of that
			// file the compiler was given. The corpus has up to eight copies of some includes
			// across the TRE stack, plus the ones this DLL substitutes and any loose override on
			// the search path, so "functions.inc line 193" is ambiguous without this.
			WARNING(true, ("Direct3d11: include '%s' served from this DLL, %d bytes, %d lines.",
						   path, overrideLength, 1 + static_cast<int>(std::count(cached.data, cached.data + overrideLength, '\n'))));

			*data = cached.data;
			*bytes = static_cast<UINT>(overrideLength);
			return S_OK;
		}
	}

	AbstractFile *const file = TreeFile::open(path, AbstractFile::PriorityData, true);
	if (!file)
	{
		// Not a DEBUG_FATAL: a missing include means the shader will fail to
		// compile, and the compile failure needs to be able to name this file as
		// the reason in a Release build too.
		WARNING(true, ("Direct3d11: shader include '%s' could not be opened from the TRE set.", path));
		return E_FAIL;
	}

	int const length = file->length();
	byte *const contents = file->readEntireFileAndClose();
	delete file;

	if (!contents || length <= 0)
	{
		WARNING(true, ("Direct3d11: shader include '%s' opened but read empty.", path));
		delete[] contents;
		return E_FAIL;
	}

	CachedInclude cached;

	// c_ambient.inc is rewritten on the way in, inherited from the DX9 x64 build.
	int patchedLength = 0;
	char *const patched = Direct3d11_ShaderSource::patchIncludeContents(path, reinterpret_cast<char const *>(contents), length, patchedLength);
	if (patched)
	{
		cached.length = patchedLength;
		cached.data = patched;
	}
	else
	{
		cached.length = length;
		cached.data = new char[length];
		memcpy(cached.data, contents, length);
	}

	delete[] contents;

	IGNORE_RETURN(ms_includeCache.insert(std::make_pair(std::string(path), cached)));

	// See the note at the override branch: a compile error names a file and a line, and the TRE
	// stack holds several copies of most of these, so the size and line count are what identify
	// WHICH copy TreeFile resolved.
	WARNING(true, ("Direct3d11: include '%s' served from the search path, %d bytes, %d lines%s.",
				   path, cached.length,
				   1 + static_cast<int>(std::count(cached.data, cached.data + cached.length, '\n')),
				   patched ? " (patched by this backend)" : ""));

	// Writing the served text out to logs/ as well was how the hemispheric-lighting failure was
	// finally pinned down -- the byte count alone matched no copy in the 209 TREs and no loose
	// file, so only the bytes settled it. That dump is not left in: a shipping client should not
	// write a file per shader include every run. If another asset-resolution question comes up,
	// re-adding it is a dozen lines right here, and the line above is what tells you that you
	// need to.

	*data = cached.data;

	// cached.length, not length: a patched include is longer than the file it came from,
	// and reporting the file's length truncates the last of the patch.
	*bytes = static_cast<UINT>(cached.length);
	return S_OK;
}

// ----------------------------------------------------------------------

HRESULT STDMETHODCALLTYPE Direct3d11_ShaderCompilerNamespace::IncludeHandler::Close(LPCVOID)
{
	// Nothing to do: every buffer handed out is owned by the cache and lives until
	// flushIncludeCache or remove.
	return S_OK;
}

// ======================================================================

/**
 * Report which compiler this process actually loaded.
 *
 * Not diagnostics for their own sake. d3dcompiler_47.dll is resolved from the system
 * directory unless a copy sits beside the executable, and the system copy tracks the
 * Windows build -- 10.0.26100.8875 on the machine this was written on, against
 * 10.0.26100.7705 in the Windows SDK that the offline shader sweep used. Two builds of
 * the same compiler need not emit identical bytecode.
 *
 * For a port whose acceptance test is a pixel comparison against gl05, that makes the
 * shader compiler an uncontrolled variable, and an image difference traced to it would
 * otherwise be indistinguishable from a port defect. Logging the path and version costs
 * one line at install and makes the variable attributable; pinning a known copy next to
 * the executable would remove it, which is a shipping decision rather than a code one.
 */

void Direct3d11_ShaderCompilerNamespace::reportCompilerVersion()
{
	// The module is already loaded: this DLL imports it, so by the time install runs it is
	// mapped and GetModuleHandle cannot fail for a reason worth handling.
	HMODULE const module = GetModuleHandleA("d3dcompiler_47.dll");
	if (!module)
	{
		WARNING(true, ("Direct3d11: d3dcompiler_47.dll is not loaded under that name, so its version cannot be reported."));
		return;
	}

	char path[MAX_PATH];
	path[0] = 0;
	if (!GetModuleFileNameA(module, path, sizeof(path)))
	{
		WARNING(true, ("Direct3d11: the path of the loaded d3dcompiler could not be read."));
		return;
	}

	DWORD ignored = 0;
	DWORD const infoSize = GetFileVersionInfoSizeA(path, &ignored);
	if (!infoSize)
	{
		REPORT_LOG_PRINT(true, ("Direct3d11: shader compiler is %s, version unavailable.\n", path));
		return;
	}

	uint8 *const info = new uint8[infoSize];
	VS_FIXEDFILEINFO *fixed = NULL;
	UINT fixedSize = 0;

	if (GetFileVersionInfoA(path, 0, infoSize, info) && VerQueryValueA(info, "\\", reinterpret_cast<void **>(&fixed), &fixedSize) && fixed)
	{
		REPORT_LOG_PRINT(true, ("Direct3d11: shader compiler is %s, version %d.%d.%d.%d\n",
								path,
								static_cast<int>(HIWORD(fixed->dwFileVersionMS)),
								static_cast<int>(LOWORD(fixed->dwFileVersionMS)),
								static_cast<int>(HIWORD(fixed->dwFileVersionLS)),
								static_cast<int>(LOWORD(fixed->dwFileVersionLS))));
	}
	else
	{
		REPORT_LOG_PRINT(true, ("Direct3d11: shader compiler is %s, version could not be parsed.\n", path));
	}

	delete[] info;
}

// ----------------------------------------------------------------------

void Direct3d11_ShaderCompiler::install()
{
	DEBUG_FATAL(ms_installed, ("Direct3d11_ShaderCompiler::install called twice"));
	ms_installed = true;

	reportCompilerVersion();
}

// ----------------------------------------------------------------------

void Direct3d11_ShaderCompiler::remove()
{
	flushIncludeCache();
	ms_installed = false;
}

// ----------------------------------------------------------------------

void Direct3d11_ShaderCompiler::flushIncludeCache()
{
	for (IncludeCache::iterator i = ms_includeCache.begin(); i != ms_includeCache.end(); ++i)
		delete[] i->second.data;

	ms_includeCache.clear();
}

// ======================================================================
/**
 * Compile one program.
 *
 * A failure is fatal in a developer build. That is a deliberate choice about
 * where a missing shader should stop: a warning here produces a null shader,
 * which produces a draw that either renders nothing or renders with whatever was
 * bound last, and the resulting image is wrong somewhere far from the cause. In
 * PRODUCTION it degrades to a warning and a null return, because shipping a
 * client that refuses to start over one bad program is worse than shipping one
 * with a visible defect.
 *
 * Warnings on a SUCCESSFUL compile are logged rather than discarded. Implicit
 * truncation and register-packing warnings are exactly the diagnostics a parity
 * investigation needs, and the prior attempt released that buffer without ever
 * reading it.
 */

ID3DBlob *Direct3d11_ShaderCompilerNamespace::compile(char const *source, int sourceLength, char const *name, D3D_SHADER_MACRO const *macros, char const *target)
{
	NOT_NULL(source);
	NOT_NULL(name);

	// The key is the hash of exactly what would be passed below, so a hit is bytecode compiled
	// from this text with these macros, this target and these flags -- or nothing.
	Direct3d11ShaderHash const key = Direct3d11_ShaderPrograms::hashCompilerInput(source, sourceLength, macros, target, cms_compileFlags);

	{
		ID3DBlob *const cached = Direct3d11_ShaderCache::fetch(key, name);
		if (cached)
			return cached;
	}

	// Named, because a program that compiles at runtime is a program the cache does not cover, and
	// the cost of that is the stall the cache exists to remove. Three lines in a good run and a
	// list worth acting on in a bad one; there are only a few hundred programs in the corpus.
	WARNING(true, ("Direct3d11: '%s' (%s, key %016llx) is being compiled because the precompiled cache %s.",
				   name, target, key,
				   Direct3d11_ShaderCache::isEnabled() ? "does not contain it" : "is not in use"));

	ID3DBlob *bytecode = NULL;
	ID3DBlob *errors = NULL;

	// QueryPerformanceCounter rather than the engine's Clock: this runs from whichever thread
	// loaded the effect, and the engine's frame clock is not that thread's to read.
	LARGE_INTEGER start;
	start.QuadPart = 0;
	IGNORE_RETURN(QueryPerformanceCounter(&start));

	HRESULT const hresult = D3DCompile(
		source,
		static_cast<SIZE_T>(sourceLength),
		name,
		macros,
		&ms_includeHandler,
		"main",
		target,
		cms_compileFlags,
		0,
		&bytecode,
		&errors);

	LARGE_INTEGER end;
	end.QuadPart = 0;
	IGNORE_RETURN(QueryPerformanceCounter(&end));

	LARGE_INTEGER frequency;
	frequency.QuadPart = 0;
	IGNORE_RETURN(QueryPerformanceFrequency(&frequency));

	if (frequency.QuadPart > 0)
		Direct3d11_Metrics::shaderCompileMicroseconds += static_cast<int>(((end.QuadPart - start.QuadPart) * 1000000) / frequency.QuadPart);

	++Direct3d11_Metrics::shaderCompiles;

	Direct3d11_ShaderCache::store(key, name, bytecode);

	if (FAILED(hresult) || !bytecode)
	{
		char const *const detail = errors ? static_cast<char const *>(errors->GetBufferPointer()) : "no detail available";

		// The exact text that failed, written beside the log.
		//
		// The error names a line in a file, and by this point the file is not any file on disk: it
		// is the asset after every source transform, with a generated signature wrapper appended.
		// Reasoning about a compile error from the asset instead of from this is how an afternoon
		// gets spent on the wrong copy of a shader -- the corpus resolves differently offline than
		// it does in the client, so "it compiles in the harness" and "it compiles here" are
		// genuinely different claims.
		{
			char dumpPath[512];
			strcpy(dumpPath, "logs/failed_");

			int j = static_cast<int>(strlen(dumpPath));
			for (int i = 0; name[i] && j < isizeof(dumpPath) - 8; ++i, ++j)
				dumpPath[j] = (name[i] == '/' || name[i] == '\\' || name[i] == ':') ? '_' : name[i];
			strcpy(dumpPath + j, ".hlsl");

			FILE *const dump = fopen(dumpPath, "wb");
			if (dump)
			{
				IGNORE_RETURN(fwrite(source, 1, static_cast<size_t>(sourceLength), dump));
				fclose(dump);
				WARNING(true, ("Direct3d11: the source that failed is in %s.", dumpPath));
			}
		}

#if PRODUCTION == 0
		FATAL(true, ("Direct3d11: '%s' failed to compile as %s (%s):\n%s", name, target, Direct3d11_Device::describeHresult(hresult), detail));
#else
		WARNING(true, ("Direct3d11: '%s' failed to compile as %s (%s): %s", name, target, Direct3d11_Device::describeHresult(hresult), detail));
#endif

		if (errors)
			errors->Release();
		if (bytecode)
		{
			bytecode->Release();
			bytecode = NULL;
		}

		return NULL;
	}

	if (errors)
	{
		// Succeeded, but said something. Worth reading.
		WARNING(true, ("Direct3d11: '%s' compiled as %s with warnings:\n%s", name, target, static_cast<char const *>(errors->GetBufferPointer())));
		errors->Release();
	}

	return bytecode;
}

// ----------------------------------------------------------------------

/**
 * Refuse a program whose source is not HLSL, naming what it is instead.
 *
 * Must be given the program's ORIGINAL text -- the marker it tests for is the first line, and
 * anything that has already stripped a header will fail this for the wrong reason.
 *
 * D3DCompile has no assembler and D3D9 shader assembly is not a D3D11 shader model, so an
 * assembly program cannot be built at all; the asset pipeline translates the reachable ones
 * offline and client-assets carries the results.
 */

bool Direct3d11_ShaderCompilerNamespace::refuseIfNotHlsl(char const *source, int sourceLength, char const *name)
{
	Direct3d11_ShaderSource::Language const language = Direct3d11_ShaderSource::getLanguage(source, sourceLength);
	if (language == Direct3d11_ShaderSource::L_hlsl)
		return false;

	WARNING(true, ("Direct3d11: '%s' is %s, which D3DCompile cannot build. It needs translating to HLSL.",
				   name, (language == Direct3d11_ShaderSource::L_assembly) ? "Direct3D 9 shader assembly" : "not recognisable as HLSL or assembly"));
	return true;
}

// ----------------------------------------------------------------------

ID3DBlob *Direct3d11_ShaderCompilerNamespace::compilePatched(char const *source, int sourceLength, char const *name, D3D_SHADER_MACRO const *macros, char const *target, bool isVertexProgram)
{
	NOT_NULL(source);

	// No language check here, deliberately. It used to be at the top of this function and it
	// rejected EVERY VERTEX PROGRAM IN THE GAME.
	//
	// The language marker is the first line of the file, and a vertex program never reaches this
	// function with its first line intact: Direct3d11_VertexShaderData::parseHeader consumes the
	// marker and the leading #define block on purpose -- the block redefines the texture
	// coordinate set macros the runtime supplies, and leaving it makes seven programs fail
	// X4532. So by the time the text arrives here the "//hlsl" it would be tested for is gone,
	// getLanguage returned L_unknown, and the program was refused as "not recognisable".
	//
	// The first run of the client showed exactly that: 23 refusals, one per vertex program the
	// login screen reached, and "a draw was reached with no vertex shader bound" alongside them.
	// Only 23 because only 23 were reached -- nothing 3D would ever have drawn.
	//
	// The check belongs where the original text is, so it now lives in the two callers:
	// parseHeader validates the vertex side before stripping anything, and compilePixelShader
	// validates below on the untouched PSRC chunk.

	int patchedLength = 0;
	char *const patched = Direct3d11_ShaderSource::patchProgramSource(name, source, sourceLength, isVertexProgram, patchedLength);

	ID3DBlob *const bytecode = patched
								   ? compile(patched, patchedLength, name, macros, target)
								   : compile(source, sourceLength, name, macros, target);

	delete[] patched;
	return bytecode;
}

// ----------------------------------------------------------------------

ID3DBlob *Direct3d11_ShaderCompiler::compileVertexShader(char const *source, int sourceLength, char const *name, D3D_SHADER_MACRO const *macros)
{
	// No language check: the text has already had its marker stripped by parseHeader, which is
	// also what validated it. Testing here would reject every vertex program. See compilePatched.
	return compilePatched(source, sourceLength, name, macros, cms_vertexShaderTarget, true);
}

// ----------------------------------------------------------------------

ID3DBlob *Direct3d11_ShaderCompiler::compileGeometryShader(char const *source, int sourceLength, char const *name)
{
	// gs_4_0 rather than gs_5_0: this backend targets feature level 10 hardware and up, and
	// nothing here needs a 5.0 construct.
	return Direct3d11_ShaderCompilerNamespace::compile(source, sourceLength, name, NULL, "gs_4_0");
}

// ----------------------------------------------------------------------

ID3DBlob *Direct3d11_ShaderCompiler::compilePixelShader(char const *source, int sourceLength, char const *name, D3D_SHADER_MACRO const *macros)
{
	// The pixel side receives the PSRC chunk untouched, so its marker is still the first line and
	// this is the right place to test it.
	if (refuseIfNotHlsl(source, sourceLength, name))
		return NULL;

	return compilePatched(source, sourceLength, name, macros, cms_pixelShaderTarget, false);
}

// ----------------------------------------------------------------------

unsigned int Direct3d11_ShaderCompiler::getCompileFlags()
{
	return cms_compileFlags;
}

// ----------------------------------------------------------------------

bool Direct3d11_ShaderCompiler::hashIncludeAsServed(char const *path, Direct3d11ShaderHash &hash)
{
	if (!path)
		return false;

	// Deliberately goes through the include handler rather than reading the file. Two of these are
	// substituted from this DLL and one is rewritten on the way in, so what is on disk is not what
	// the compiler is given -- and it is what the compiler is given that the baked bytecode
	// depends on. Calling Open() is not "the same as" that path; it IS that path.
	LPCVOID data = NULL;
	UINT bytes = 0;
	if (FAILED(ms_includeHandler.Open(D3D_INCLUDE_LOCAL, path, NULL, &data, &bytes)) || !data)
		return false;

	hash = Direct3d11_ShaderPrograms::hashBytes(data, static_cast<int>(bytes));
	return true;
}

// ----------------------------------------------------------------------

int Direct3d11_ShaderCompiler::getServedIncludeCount()
{
	return static_cast<int>(ms_includeCache.size());
}

// ----------------------------------------------------------------------

bool Direct3d11_ShaderCompiler::getServedInclude(int index, char const *&path, Direct3d11ShaderHash &hash)
{
	if (index < 0 || index >= static_cast<int>(ms_includeCache.size()))
		return false;

	IncludeCache::const_iterator i = ms_includeCache.begin();
	std::advance(i, index);

	path = i->first.c_str();
	hash = Direct3d11_ShaderPrograms::hashBytes(i->second.data, i->second.length);
	return true;
}

// ----------------------------------------------------------------------

char const *Direct3d11_ShaderCompiler::getVertexShaderTarget()
{
	return cms_vertexShaderTarget;
}

// ----------------------------------------------------------------------

char const *Direct3d11_ShaderCompiler::getPixelShaderTarget()
{
	return cms_pixelShaderTarget;
}

// ======================================================================
