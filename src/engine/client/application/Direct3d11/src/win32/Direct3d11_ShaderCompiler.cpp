// ======================================================================
//
// Direct3d11_ShaderCompiler.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_ShaderCompiler.h"

#include "Direct3d11_Device.h"
#include "Direct3d11_Metrics.h"

#include "fileInterface/AbstractFile.h"
#include "sharedFile/TreeFile.h"

#include <d3dcompiler.h>
#include <map>
#include <string>

// ======================================================================

namespace Direct3d11_ShaderCompilerNamespace
{
	char const * const cms_vertexShaderTarget = "vs_4_0";
	char const * const cms_pixelShaderTarget  = "ps_4_0";

	// Backwards compatibility is mandatory; level 3 optimisation matches what the
	// shipped DX9 pixel programs were built with. PACK_MATRIX_ROW_MAJOR is
	// deliberately absent -- see the header.
	UINT const cms_compileFlags = D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY | D3DCOMPILE_OPTIMIZATION_LEVEL3;

	struct CachedInclude
	{
		char  *data;
		int    length;
	};

	typedef std::map<std::string, CachedInclude> IncludeCache;
	IncludeCache  ms_includeCache;
	bool          ms_installed;

	// ID3DInclude, not ID3DXInclude: same shape, different interface, and the D3DX
	// one belongs to a DirectX SDK this backend does not link.
	class IncludeHandler : public ID3DInclude
	{
	public:

		virtual HRESULT STDMETHODCALLTYPE Open(D3D_INCLUDE_TYPE includeType, LPCSTR fileName, LPCVOID parentData, LPCVOID *data, UINT *bytes);
		virtual HRESULT STDMETHODCALLTYPE Close(LPCVOID data);
	};

	IncludeHandler  ms_includeHandler;

	ID3DBlob       *compile(char const *source, int sourceLength, char const *name, D3D_SHADER_MACRO const *macros, char const *target);
}
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
		*data  = i->second.data;
		*bytes = static_cast<UINT>(i->second.length);
		return S_OK;
	}

	AbstractFile * const file = TreeFile::open(path, AbstractFile::PriorityData, true);
	if (!file)
	{
		// Not a DEBUG_FATAL: a missing include means the shader will fail to
		// compile, and the compile failure needs to be able to name this file as
		// the reason in a Release build too.
		WARNING(true, ("Direct3d11: shader include '%s' could not be opened from the TRE set.", path));
		return E_FAIL;
	}

	int const length = file->length();
	byte * const contents = file->readEntireFileAndClose();
	delete file;

	if (!contents || length <= 0)
	{
		WARNING(true, ("Direct3d11: shader include '%s' opened but read empty.", path));
		delete [] contents;
		return E_FAIL;
	}

	CachedInclude cached;
	cached.length = length;
	cached.data = new char[length];
	memcpy(cached.data, contents, length);
	delete [] contents;

	IGNORE_RETURN(ms_includeCache.insert(std::make_pair(std::string(path), cached)));

	*data  = cached.data;
	*bytes = static_cast<UINT>(length);
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

void Direct3d11_ShaderCompiler::install()
{
	DEBUG_FATAL(ms_installed, ("Direct3d11_ShaderCompiler::install called twice"));
	ms_installed = true;
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
		delete [] i->second.data;

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

	ID3DBlob *bytecode = NULL;
	ID3DBlob *errors = NULL;

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

	++Direct3d11_Metrics::shaderCompiles;

	if (FAILED(hresult) || !bytecode)
	{
		char const * const detail = errors ? static_cast<char const *>(errors->GetBufferPointer()) : "no detail available";

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

ID3DBlob *Direct3d11_ShaderCompiler::compileVertexShader(char const *source, int sourceLength, char const *name, D3D_SHADER_MACRO const *macros)
{
	return compile(source, sourceLength, name, macros, cms_vertexShaderTarget);
}

// ----------------------------------------------------------------------

ID3DBlob *Direct3d11_ShaderCompiler::compilePixelShader(char const *source, int sourceLength, char const *name, D3D_SHADER_MACRO const *macros)
{
	return compile(source, sourceLength, name, macros, cms_pixelShaderTarget);
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
