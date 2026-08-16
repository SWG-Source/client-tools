// ======================================================================
//
// Direct3d11_ShaderCache.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_ShaderCache.h"

#include "ConfigDirect3d11.h"
#include "Direct3d11_Metrics.h"
#include "Direct3d11_ShaderCompiler.h"

#include "sharedFile/TreeFile.h"

#include <d3dcompiler.h>

#include <map>
#include <stdio.h>
#include <string>
#include <vector>

// ======================================================================

namespace Direct3d11_ShaderCacheNamespace
{
	// Bumped when the manifest's meaning changes, not when its contents do. A run reading a
	// manifest it does not understand disables the cache rather than guessing at the fields.
	int const cms_manifestVersion = 1;

	char const *const cms_directory = "compiled_shader";
	char const *const cms_manifestName = "compiled_shader/manifest.txt";

	bool ms_installed;
	bool ms_enabled;
	bool ms_baking;

	// Blobs baked this run, so a program compiled twice is written once. The engine can build the
	// same program's graphics data more than once across a shader template reload.
	std::map<Direct3d11ShaderHash, std::string> ms_baked;

	// Include lines carried forward from a seeded manifest (see the bake branch of install):
	// includes the previous bake served that this run may not serve itself.
	std::vector<std::pair<std::string, unsigned long long> > ms_carriedIncludes;

	// ------------------------------------------------------------------

	void formatKey(Direct3d11ShaderHash key, char *destination, int destinationSize)
	{
		// Sixteen lower-case hex digits, always. Fixed width so the file names sort and so a
		// truncated one is obvious rather than merely unlucky.
		IGNORE_RETURN(snprintf(destination, static_cast<size_t>(destinationSize), "%016llx", key));
	}

	void formatBlobPath(Direct3d11ShaderHash key, char *destination, int destinationSize)
	{
		char hex[32];
		formatKey(key, hex, isizeof(hex));
		IGNORE_RETURN(snprintf(destination, static_cast<size_t>(destinationSize), "%s/%s.cso", cms_directory, hex));
	}

	// ------------------------------------------------------------------
	// Seed the baker from the existing manifest, so a re-bake UNIONS with prior coverage
	// instead of replacing it. Without this, a bake session that visits less content than
	// the last one silently SHRINKS the cache -- the manifest lists only what that session
	// compiled, so filling a small gap (the first field gap was the sky and cloud programs)
	// would have required a session that also re-covered everything already baked. Seeding
	// applies the same validation a using run applies -- version, flags, and every recorded
	// include hash against the live data -- and a manifest that fails ANY check is not
	// carried forward: a stale entry must never be re-listed as current. Seeded programs'
	// blobs already exist on disk, and store() skips keys already in ms_baked, so this run
	// only writes what is genuinely new.

	void seedBakerFromExistingManifest()
	{
		AbstractFile *const file = TreeFile::open(cms_manifestName, AbstractFile::PriorityData, true);
		if (!file)
			return;                       // nothing to carry forward

		int const length = file->length();
		byte *const contents = file->readEntireFileAndClose();
		delete file;

		if (!contents || length <= 0)
		{
			delete[] contents;
			return;
		}

		std::string text(reinterpret_cast<char const *>(contents), static_cast<size_t>(length));
		delete[] contents;

		bool versionSeen = false;
		bool flagsSeen = false;
		std::map<Direct3d11ShaderHash, std::string> programs;
		std::vector<std::pair<std::string, unsigned long long> > includes;

		size_t at = 0;
		while (at < text.size())
		{
			size_t const eol = text.find('\n', at);
			std::string line = text.substr(at, (eol == std::string::npos) ? std::string::npos : eol - at);
			at = (eol == std::string::npos) ? text.size() : eol + 1;

			while (!line.empty() && (line[line.size() - 1] == '\r' || line[line.size() - 1] == ' '))
				line.erase(line.size() - 1);

			if (line.empty() || line[0] == '#')
				continue;

			char keyword[64];
			char first[512];

			if (sscanf(line.c_str(), "%63s", keyword) != 1)
				continue;

			if (strcmp(keyword, "version") == 0)
			{
				int version = 0;
				if (sscanf(line.c_str(), "%63s %d", keyword, &version) != 2 || version != cms_manifestVersion)
				{
					WARNING(true, ("Direct3d11: the existing manifest is version %d and this build understands %d; baking from scratch.", version, cms_manifestVersion));
					return;
				}
				versionSeen = true;
			}
			else if (strcmp(keyword, "flags") == 0)
			{
				unsigned int flags = 0;
				if (sscanf(line.c_str(), "%63s 0x%x", keyword, &flags) != 2 || flags != Direct3d11_ShaderCompiler::getCompileFlags())
				{
					WARNING(true, ("Direct3d11: the existing manifest was baked with compile flags 0x%08x and this build uses 0x%08x; baking from scratch.", flags, Direct3d11_ShaderCompiler::getCompileFlags()));
					return;
				}
				flagsSeen = true;
			}
			else if (strcmp(keyword, "include") == 0)
			{
				unsigned long long recorded = 0;
				if (sscanf(line.c_str(), "%63s %511s %llx", keyword, first, &recorded) != 3)
					continue;

				Direct3d11ShaderHash live = 0;
				if (!Direct3d11_ShaderCompiler::hashIncludeAsServed(first, live) || live != recorded)
				{
					WARNING(true, ("Direct3d11: include '%s' no longer matches the existing manifest; baking from scratch.", first));
					return;
				}
				includes.push_back(std::make_pair(std::string(first), recorded));
			}
			else if (strcmp(keyword, "program") == 0)
			{
				unsigned long long key = 0;
				if (sscanf(line.c_str(), "%63s %llx %511s", keyword, &key, first) == 3)
					IGNORE_RETURN(programs.insert(std::make_pair(static_cast<Direct3d11ShaderHash>(key), std::string(first))));
			}
		}

		if (!versionSeen || !flagsSeen)
			return;

		ms_baked.insert(programs.begin(), programs.end());
		ms_carriedIncludes.swap(includes);

		REPORT_LOG_PRINT(true, ("Direct3d11: baker seeded with %d program(s) and %d include hash(es) from the existing manifest; this run's compiles union in.\n",
			static_cast<int>(ms_baked.size()), static_cast<int>(ms_carriedIncludes.size())));
	}
} // namespace Direct3d11_ShaderCacheNamespace
using namespace Direct3d11_ShaderCacheNamespace;

// ======================================================================
/**
 * Read the manifest and decide whether the cache can be trusted.
 *
 * Every reason to refuse is reported, because a silently disabled cache looks exactly like a
 * cache that is working and simply never hits -- and the symptom of that is the stall this class
 * exists to remove, back again with nothing to explain it.
 */

void Direct3d11_ShaderCache::install()
{
	DEBUG_FATAL(ms_installed, ("Direct3d11_ShaderCache::install called twice"));
	ms_installed = true;

	ms_baking = ConfigDirect3d11::getBakeCompiledShaders();

	if (ms_baking)
	{
		// Baking and using at the same time would bake only what missed, which is how a partial
		// cache gets mistaken for a complete one.
		WARNING(true, ("Direct3d11: baking compiled shaders to '%s'. The cache is not used this run, so every program is compiled and written.", cms_directory));
		IGNORE_RETURN(CreateDirectoryA(cms_directory, NULL));
		seedBakerFromExistingManifest();
		return;
	}

	if (!ConfigDirect3d11::getUseCompiledShaders())
	{
		REPORT_LOG_PRINT(true, ("Direct3d11: the compiled shader cache is switched off by configuration.\n"));
		return;
	}

	AbstractFile *const file = TreeFile::open(cms_manifestName, AbstractFile::PriorityData, true);
	if (!file)
	{
		// Not a warning. Shipping without a baked cache is a supported configuration -- every
		// program compiles at first use, exactly as it did before this class existed.
		REPORT_LOG_PRINT(true, ("Direct3d11: no compiled shader manifest at '%s'; every program will be compiled at first use.\n", cms_manifestName));
		return;
	}

	int const length = file->length();
	byte *const contents = file->readEntireFileAndClose();
	delete file;

	if (!contents || length <= 0)
	{
		WARNING(true, ("Direct3d11: the compiled shader manifest '%s' opened but read empty, so the cache is disabled.", cms_manifestName));
		delete[] contents;
		return;
	}

	std::string text(reinterpret_cast<char const *>(contents), static_cast<size_t>(length));
	delete[] contents;

	bool versionSeen = false;
	bool flagsSeen = false;
	int includesChecked = 0;
	int programsListed = 0;

	size_t at = 0;
	while (at < text.size())
	{
		size_t const eol = text.find('\n', at);
		std::string line = text.substr(at, (eol == std::string::npos) ? std::string::npos : eol - at);
		at = (eol == std::string::npos) ? text.size() : eol + 1;

		while (!line.empty() && (line[line.size() - 1] == '\r' || line[line.size() - 1] == ' '))
			line.erase(line.size() - 1);

		if (line.empty() || line[0] == '#')
			continue;

		char keyword[64];
		char first[512];
		char second[64];

		if (sscanf(line.c_str(), "%63s", keyword) != 1)
			continue;

		if (strcmp(keyword, "version") == 0)
		{
			int version = 0;
			if (sscanf(line.c_str(), "%63s %d", keyword, &version) == 2 && version == cms_manifestVersion)
			{
				versionSeen = true;
			}
			else
			{
				WARNING(true, ("Direct3d11: the compiled shader manifest is version %d and this build understands %d, so the cache is disabled.", version, cms_manifestVersion));
				return;
			}
		}
		else if (strcmp(keyword, "flags") == 0)
		{
			unsigned int flags = 0;
			if (sscanf(line.c_str(), "%63s 0x%x", keyword, &flags) == 2)
			{
				if (flags != Direct3d11_ShaderCompiler::getCompileFlags())
				{
					// The flags are in every program's key as well, so a mismatch would simply miss
					// on every lookup. Saying so is the difference between "the cache is useless"
					// and "the cache was baked with different flags".
					WARNING(true, ("Direct3d11: the compiled shader cache was baked with compile flags 0x%08x and this build uses 0x%08x, so nothing in it can match. The cache is disabled.", flags, Direct3d11_ShaderCompiler::getCompileFlags()));
					return;
				}
				flagsSeen = true;
			}
		}
		else if (strcmp(keyword, "include") == 0)
		{
			unsigned long long recorded = 0;
			if (sscanf(line.c_str(), "%63s %511s %llx", keyword, first, &recorded) != 3)
				continue;

			// Re-read it through the compiler's own include handler, so this compares what the
			// compiler would be given rather than what is on disk. The two differ: two includes are
			// substituted from this DLL and one is rewritten on the way in.
			Direct3d11ShaderHash live = 0;
			if (!Direct3d11_ShaderCompiler::hashIncludeAsServed(first, live))
			{
				WARNING(true, ("Direct3d11: the compiled shader cache records include '%s', which cannot be read now. The cache is disabled because every program that included it would compile from different text than was baked.", first));
				return;
			}

			if (live != recorded)
			{
				WARNING(true, ("Direct3d11: include '%s' hashes %016llx now and %016llx when the cache was baked. The cache is disabled -- regenerate it with tools/dxbcbake.", first, live, recorded));
				return;
			}

			++includesChecked;
		}
		else if (strcmp(keyword, "program") == 0)
		{
			// Informational: which key belongs to which program. Coverage is worth reporting and
			// a hash on its own is not a name anybody can act on.
			if (sscanf(line.c_str(), "%63s %63s %511s", keyword, second, first) == 3)
				++programsListed;
		}
	}

	if (!versionSeen || !flagsSeen)
	{
		WARNING(true, ("Direct3d11: the compiled shader manifest is missing its version or flags line, so the cache is disabled."));
		return;
	}

	ms_enabled = true;

	REPORT_LOG_PRINT(true, ("Direct3d11: compiled shader cache enabled -- %d program(s) baked, %d include(s) verified unchanged.\n", programsListed, includesChecked));
}

// ----------------------------------------------------------------------

void Direct3d11_ShaderCache::remove()
{
	if (ms_baking)
		writeManifest();

	ms_baked.clear();
	ms_carriedIncludes.clear();
	ms_enabled = false;
	ms_baking = false;
	ms_installed = false;
}

// ----------------------------------------------------------------------

bool Direct3d11_ShaderCache::isEnabled()
{
	return ms_enabled;
}

// ======================================================================

ID3DBlob *Direct3d11_ShaderCache::fetch(Direct3d11ShaderHash key, char const *name)
{
	if (!ms_enabled)
		return NULL;

	char path[256];
	formatBlobPath(key, path, isizeof(path));

	AbstractFile *const file = TreeFile::open(path, AbstractFile::PriorityData, true);
	if (!file)
	{
		++Direct3d11_Metrics::shaderCacheMisses;
		return NULL;
	}

	int const length = file->length();
	byte *const contents = file->readEntireFileAndClose();
	delete file;

	if (!contents || length <= 0)
	{
		WARNING(true, ("Direct3d11: compiled shader '%s' for '%s' opened but read empty, so it will be compiled instead.", path, name));
		delete[] contents;
		++Direct3d11_Metrics::shaderCacheMisses;
		return NULL;
	}

	// A DXBC container opens with 'DXBC' and carries its own total size at offset 24. Checked
	// because a truncated or wrong-typed file handed to CreateVertexShader is a driver-level
	// failure with no useful message, and this costs two comparisons.
	bool valid = (length >= 32) && (memcmp(contents, "DXBC", 4) == 0);
	if (valid)
	{
		unsigned int declaredSize = 0;
		memcpy(&declaredSize, contents + 24, sizeof(declaredSize));
		valid = (declaredSize == static_cast<unsigned int>(length));
	}

	if (!valid)
	{
		WARNING(true, ("Direct3d11: compiled shader '%s' for '%s' is not a %d-byte DXBC container, so it will be compiled instead.", path, name, length));
		delete[] contents;
		++Direct3d11_Metrics::shaderCacheMisses;
		return NULL;
	}

	ID3DBlob *blob = NULL;
	if (FAILED(D3DCreateBlob(static_cast<SIZE_T>(length), &blob)) || !blob)
	{
		delete[] contents;
		++Direct3d11_Metrics::shaderCacheMisses;
		return NULL;
	}

	memcpy(blob->GetBufferPointer(), contents, static_cast<size_t>(length));
	delete[] contents;

	++Direct3d11_Metrics::shaderCacheHits;
	return blob;
}

// ----------------------------------------------------------------------

void Direct3d11_ShaderCache::store(Direct3d11ShaderHash key, char const *name, ID3DBlob *bytecode)
{
	if (!ms_baking || !bytecode)
		return;

	if (ms_baked.find(key) != ms_baked.end())
		return;

	char path[256];
	formatBlobPath(key, path, isizeof(path));

	FILE *const out = fopen(path, "wb");
	if (!out)
	{
		WARNING(true, ("Direct3d11: could not write compiled shader '%s' for '%s'.", path, name));
		return;
	}

	IGNORE_RETURN(fwrite(bytecode->GetBufferPointer(), 1, bytecode->GetBufferSize(), out));
	IGNORE_RETURN(fclose(out));

	IGNORE_RETURN(ms_baked.insert(std::make_pair(key, std::string(name ? name : "<unnamed>"))));
}

// ----------------------------------------------------------------------

void Direct3d11_ShaderCache::writeManifest()
{
	if (!ms_baking)
		return;

	char path[256];
	IGNORE_RETURN(snprintf(path, sizeof(path), "%s/manifest.txt", cms_directory));

	FILE *const out = fopen(path, "wb");
	if (!out)
	{
		WARNING(true, ("Direct3d11: could not write the compiled shader manifest '%s'.", path));
		return;
	}

	IGNORE_RETURN(fprintf(out, "# Direct3d11 compiled shader manifest. Generated by a run with bakeCompiledShaders set.\n"));
	IGNORE_RETURN(fprintf(out, "#\n"));
	IGNORE_RETURN(fprintf(out, "# Each program's blob is named after the hash of the exact input D3DCompile was given, so a\n"));
	IGNORE_RETURN(fprintf(out, "# blob can never be stale: a changed program hashes differently, finds no file and compiles.\n"));
	IGNORE_RETURN(fprintf(out, "# The include hashes below are the part the key cannot cover -- includes are served during the\n"));
	IGNORE_RETURN(fprintf(out, "# compile, so they are verified once at install instead. Any mismatch disables the cache.\n"));
	IGNORE_RETURN(fprintf(out, "version %d\n", cms_manifestVersion));
	IGNORE_RETURN(fprintf(out, "flags 0x%08x\n", Direct3d11_ShaderCompiler::getCompileFlags()));
	IGNORE_RETURN(fprintf(out, "vertexTarget %s\n", Direct3d11_ShaderCompiler::getVertexShaderTarget()));
	IGNORE_RETURN(fprintf(out, "pixelTarget %s\n", Direct3d11_ShaderCompiler::getPixelShaderTarget()));

	// Includes: everything served this run, plus lines carried from the seeded manifest
	// that this run did not serve itself -- their hashes were validated against the live
	// data at seed time, so they are equally current. Served entries win on a path both
	// lists carry (the values are identical when the seed validated).
	std::map<std::string, unsigned long long> includeLines;
	{
		int const servedCount = Direct3d11_ShaderCompiler::getServedIncludeCount();
		for (int i = 0; i < servedCount; ++i)
		{
			char const *includePath = NULL;
			Direct3d11ShaderHash hash = 0;
			if (Direct3d11_ShaderCompiler::getServedInclude(i, includePath, hash) && includePath)
				includeLines[includePath] = hash;
		}
		for (size_t i = 0; i < ms_carriedIncludes.size(); ++i)
			IGNORE_RETURN(includeLines.insert(ms_carriedIncludes[i]));
	}

	for (std::map<std::string, unsigned long long>::const_iterator i = includeLines.begin(); i != includeLines.end(); ++i)
		IGNORE_RETURN(fprintf(out, "include %s %016llx\n", i->first.c_str(), i->second));

	for (std::map<Direct3d11ShaderHash, std::string>::const_iterator i = ms_baked.begin(); i != ms_baked.end(); ++i)
		IGNORE_RETURN(fprintf(out, "program %016llx %s\n", i->first, i->second.c_str()));

	IGNORE_RETURN(fclose(out));

	WARNING(true, ("Direct3d11: baked %d compiled shader(s) and %d include hash(es) to '%s'.",
				   static_cast<int>(ms_baked.size()), static_cast<int>(includeLines.size()), cms_directory));
}

// ======================================================================
