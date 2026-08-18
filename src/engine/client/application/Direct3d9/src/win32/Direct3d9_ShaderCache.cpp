// ======================================================================
//
// Direct3d9_ShaderCache.cpp
// Disk-persisted vertex-shader bytecode cache for the D3D9 plugins.
// Ported from gl11 Direct3d11_ShaderCache (CONSULT-45 fix #1).
//
// Cache path = <cacheDir>/<hash:016x>.cso. The hash is a CONTENT hash over the
// full compile inputs (see Direct3d9_ShaderCache::hashSource). The .cso blob is
// opaque & self-versioning; no format game -- hash + dump + load.
//
// store() failures are NON-FATAL (cache is opportunistic; next launch recompiles).
// install() creates the directory lazily. Cache files survive across launches.
//
// ======================================================================

#include "FirstDirect3d9.h"
#include "Direct3d9_ShaderCache.h"

#include "ConfigDirect3d9.h"
#include "sharedDebug/DebugFlags.h"

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <filesystem>

// ======================================================================

namespace Direct3d9_ShaderCacheNamespace
{
	bool        ms_installed     = false;
	std::string ms_cacheDir;
	int         ms_hits          = 0;
	int         ms_misses        = 0;
	int         ms_stores        = 0;
	int         ms_storeFailures = 0;

	// CONSULT-68 sibling ([Direct3d9] shaderCachePreload, default true). Ported
	// from gl11 Direct3d11_ShaderCache: the stall stack sampler convicted the
	// per-hit fopen/fread below as a first-draw stall class there; gl05's cold
	// single-item creates pay the same per-hit disk read. The cache dir is small
	// (a few KB per VS), so a background thread slurps every .cso into ms_blobs
	// at install and tryLoad becomes a memory lookup. Until the preload finishes,
	// a map miss falls back to the per-hit disk read; after it finishes, a map
	// miss is authoritative (no disk I/O at all in the draw path).
	std::mutex        ms_blobMutex;
	std::unordered_map<uint64_t, std::vector<unsigned char> > ms_blobs;
	std::atomic<bool> ms_preloadDone(false);
	std::thread       ms_preloadThread;
	bool              ms_preloadEnabled = false;
	int               ms_preloadedFiles = 0;   // written by the preload thread before ms_preloadDone
	int               ms_ramHits        = 0;

	// FNV-1a 64-bit constants.
	const uint64_t kFnv1aOffset = 14695981039346656037ULL;
	const uint64_t kFnv1aPrime  = 1099511628211ULL;

	inline uint64_t fnv1aStep(uint64_t hash, unsigned char b)
	{
		hash ^= static_cast<uint64_t>(b);
		hash *= kFnv1aPrime;
		return hash;
	}

	inline uint64_t fnv1aString(uint64_t hash, char const *s)
	{
		if (!s)
			return hash;
		while (*s)
		{
			hash = fnv1aStep(hash, static_cast<unsigned char>(*s));
			++s;
		}
		return hash;
	}

	// <cacheDir>/<hash:016x>.cso. Defensive hex-only scan (our own hex output,
	// defense-in-depth against path traversal -- cacheDir is engine-configured).
	std::string buildCachePath(uint64_t hash)
	{
		char hexBuf[17];
		_snprintf(hexBuf, sizeof(hexBuf), "%016llx", static_cast<unsigned long long>(hash));
		hexBuf[sizeof(hexBuf) - 1] = '\0';
		for (int i = 0; i < 16; ++i)
		{
			char const c = hexBuf[i];
			DEBUG_FATAL(!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')),
				("Direct3d9_ShaderCache: non-hex char '%c' in cache filename (hash=%016llx)",
				c, static_cast<unsigned long long>(hash)));
		}

		std::string path = ms_cacheDir;
		if (!path.empty() && path.back() != '/' && path.back() != '\\')
			path += '/';
		path += hexBuf;
		path += ".cso";
		return path;
	}

	// Read one cache file into bytes. Same sanity gates as the per-hit path
	// (empty / >64MB treated as unusable). Returns false on any failure.
	bool readCacheFile(char const *path, std::vector<unsigned char> &outBytes)
	{
		FILE *f = std::fopen(path, "rb");
		if (!f)
			return false;
		if (std::fseek(f, 0, SEEK_END) != 0)
		{
			std::fclose(f);
			return false;
		}
		long const sizeLong = std::ftell(f);
		if (sizeLong <= 0 || sizeLong > (64 * 1024 * 1024))
		{
			std::fclose(f);
			return false;
		}
		std::fseek(f, 0, SEEK_SET);
		outBytes.resize(static_cast<std::size_t>(sizeLong));
		std::size_t const got = std::fread(&outBytes.front(), 1, outBytes.size(), f);
		std::fclose(f);
		return got == outBytes.size();
	}

	// CONSULT-68 sibling preload thread: slurp every <16-hex>.cso in the cache
	// dir into ms_blobs. Runs once, concurrently with early rendering; tryLoad
	// falls back to per-hit disk reads until ms_preloadDone.
	void preloadProc()
	{
		int loaded = 0;
		std::error_code ec;
		for (std::filesystem::directory_iterator it(ms_cacheDir, ec), end; !ec && it != end; it.increment(ec))
		{
			std::filesystem::directory_entry const &entry = *it;
			std::error_code fileEc;
			if (!entry.is_regular_file(fileEc) || fileEc)
				continue;
			if (entry.path().extension() != ".cso")
				continue;
			std::string const stem = entry.path().stem().string();
			if (stem.size() != 16)
				continue;
			char *endPtr = NULL;
			uint64_t const hash = std::strtoull(stem.c_str(), &endPtr, 16);
			if (!endPtr || *endPtr != '\0')
				continue;

			std::vector<unsigned char> bytes;
			if (!readCacheFile(entry.path().string().c_str(), bytes))
				continue;

			{
				std::lock_guard<std::mutex> lock(ms_blobMutex);
				ms_blobs.emplace(hash, std::move(bytes));
			}
			++loaded;
		}

		ms_preloadedFiles = loaded;
		ms_preloadDone.store(true, std::memory_order_release);
		REPORT_LOG_PRINT(true, ("Direct3d9_ShaderCache: preload complete (%d cached shaders in RAM)\n", loaded));
	}
}
using namespace Direct3d9_ShaderCacheNamespace;

// ======================================================================

void Direct3d9_ShaderCache::install(char const *cacheDir)
{
	NOT_NULL(cacheDir);

	ms_cacheDir      = cacheDir;
	ms_installed     = true;
	ms_hits          = 0;
	ms_misses        = 0;
	ms_stores        = 0;
	ms_storeFailures = 0;

	// Lazy directory creation. Failure is non-fatal: store() then quietly skips,
	// tryLoad() always misses, compile-every-time still works.
	std::error_code ec;
	std::filesystem::create_directories(ms_cacheDir, ec);
	if (ec)
	{
		DEBUG_REPORT_LOG_PRINT(true,
			("Direct3d9_ShaderCache: could not create cache dir '%s' (%s); cache disabled this run\n",
			ms_cacheDir.c_str(), ec.message().c_str()));
	}
	else
	{
		DEBUG_REPORT_LOG_PRINT(true,
			("Direct3d9_ShaderCache: install at '%s'\n", ms_cacheDir.c_str()));

		// CONSULT-68 sibling: slurp the cache into RAM off-thread (see namespace note).
		ms_preloadEnabled = ConfigDirect3d9::getShaderCachePreload();
		if (ms_preloadEnabled)
			ms_preloadThread = std::thread(preloadProc);
	}
}

// ----------------------------------------------------------------------

void Direct3d9_ShaderCache::remove()
{
	if (ms_preloadThread.joinable())
		ms_preloadThread.join();

	DEBUG_REPORT_LOG_PRINT(true,
		("Direct3d9_ShaderCache: remove -- hits=%d (%d from RAM) misses=%d stores=%d storeFailures=%d preloaded=%d\n",
		ms_hits, ms_ramHits, ms_misses, ms_stores, ms_storeFailures, ms_preloadedFiles));

	ms_cacheDir.clear();
	ms_installed = false;
	ms_preloadEnabled = false;
	ms_preloadDone.store(false);
	{
		std::lock_guard<std::mutex> lock(ms_blobMutex);
		ms_blobs.clear();
	}
	// Disk files intentionally NOT deleted (cache survives across launches).
}

// ----------------------------------------------------------------------

uint64_t Direct3d9_ShaderCache::hashSource(char const *text, std::size_t length,
	D3D_SHADER_MACRO const *defines, char const *target, uint32_t rewriteVersion)
{
	uint64_t hash = kFnv1aOffset;

	// Source body, byte-wise.
	if (text)
	{
		for (std::size_t i = 0; i < length; ++i)
			hash = fnv1aStep(hash, static_cast<unsigned char>(text[i]));
	}

	// Defines: Name\0Definition\0 each (trailing nulls disambiguate boundaries).
	if (defines)
	{
		for (D3D_SHADER_MACRO const *p = defines; p->Name != NULL; ++p)
		{
			hash = fnv1aString(hash, p->Name);
			hash = fnv1aStep(hash, 0);
			hash = fnv1aString(hash, p->Definition);
			hash = fnv1aStep(hash, 0);
		}
	}

	// Target profile (e.g. vs_2_0, including the vs_1_1->vs_2_0 promotion).
	hash = fnv1aString(hash, target);
	hash = fnv1aStep(hash, 0);

	// Rewrite version (4 bytes) -- bump in the caller when HlslRewrite rules or the
	// texcoord macro generation change, to invalidate stale on-disk bytecode.
	for (int i = 0; i < 4; ++i)
	{
		hash = fnv1aStep(hash, static_cast<unsigned char>(rewriteVersion & 0xffu));
		rewriteVersion >>= 8;
	}

	// Compiler fingerprint -- bump the literal if the d3dcompiler redist changes
	// (FXC output can differ across builds).
	hash = fnv1aString(hash, "d3dcompiler_47");

	return hash;
}

// ----------------------------------------------------------------------

bool Direct3d9_ShaderCache::tryLoad(uint64_t sourceHash, std::vector<unsigned char> &outBytes)
{
	if (!ms_installed || ms_cacheDir.empty())
	{
		++ms_misses;
		return false;
	}

	// CONSULT-68 sibling: serve from the RAM preload. After the preload completes
	// a map miss is authoritative (the draw path never touches the disk); while
	// it is still running, fall through to the per-hit disk read.
	if (ms_preloadEnabled)
	{
		{
			std::lock_guard<std::mutex> lock(ms_blobMutex);
			std::unordered_map<uint64_t, std::vector<unsigned char> >::const_iterator const it = ms_blobs.find(sourceHash);
			if (it != ms_blobs.end())
			{
				outBytes = it->second;
				++ms_hits;
				++ms_ramHits;
				return true;
			}
		}
		if (ms_preloadDone.load(std::memory_order_acquire))
		{
			++ms_misses;
			return false;
		}
	}

	std::string const path = buildCachePath(sourceHash);

	FILE *f = std::fopen(path.c_str(), "rb");
	if (!f)
	{
		++ms_misses;
		return false;
	}

	if (std::fseek(f, 0, SEEK_END) != 0)
	{
		std::fclose(f);
		++ms_misses;
		return false;
	}
	long const sizeLong = std::ftell(f);
	if (sizeLong <= 0 || sizeLong > (64 * 1024 * 1024))
	{
		// Empty / oversized files are misses; store() will overwrite. 64 MB is a
		// sanity ceiling; a SWG-era VS compiles to a few KB.
		std::fclose(f);
		++ms_misses;
		return false;
	}
	std::fseek(f, 0, SEEK_SET);

	outBytes.resize(static_cast<std::size_t>(sizeLong));
	std::size_t const got = std::fread(&outBytes.front(), 1, static_cast<std::size_t>(sizeLong), f);
	std::fclose(f);
	if (got != static_cast<std::size_t>(sizeLong))
	{
		outBytes.clear();
		++ms_misses;
		return false;
	}

	++ms_hits;
	return true;
}

// ----------------------------------------------------------------------

void Direct3d9_ShaderCache::store(uint64_t sourceHash, void const *data, std::size_t size)
{
	if (!ms_installed || ms_cacheDir.empty() || !data || size == 0)
		return;

	// CONSULT-68 sibling: keep the RAM map coherent (a freshly-compiled variant
	// should hit RAM if re-queried this session, even if the disk write fails).
	if (ms_preloadEnabled)
	{
		unsigned char const * const bytes = static_cast<unsigned char const *>(data);
		std::lock_guard<std::mutex> lock(ms_blobMutex);
		ms_blobs[sourceHash].assign(bytes, bytes + size);
	}

	std::string const path = buildCachePath(sourceHash);

	FILE *f = std::fopen(path.c_str(), "wb");
	if (!f)
	{
		++ms_storeFailures;
		DEBUG_REPORT_LOG_PRINT(true,
			("Direct3d9_ShaderCache: store FAILED to open '%s' (errno=%d) -- recompile next launch (non-fatal)\n",
			path.c_str(), errno));
		return;
	}

	std::size_t const wrote = std::fwrite(data, 1, size, f);
	std::fclose(f);

	if (wrote != size)
	{
		++ms_storeFailures;
		DEBUG_REPORT_LOG_PRINT(true,
			("Direct3d9_ShaderCache: store SHORT WRITE '%s' wanted=%zu got=%zu (non-fatal)\n",
			path.c_str(), size, wrote));
		// Remove the truncated file so it can't poison future tryLoad attempts.
		std::error_code ec;
		std::filesystem::remove(path, ec);
		return;
	}

	++ms_stores;
}

// ======================================================================
