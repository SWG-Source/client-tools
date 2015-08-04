// ======================================================================
//
// ShaderTemplateList.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/ShaderTemplateList.h"

#include "clientGraphics/StaticShader.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CrashReportInformation.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedSynchronization/RecursiveMutex.h"

#include <map>
#include <vector>

// ======================================================================

namespace ShaderTemplateListNamespace
{
	typedef std::map<Tag, ShaderTemplateList::CreationFunction>                   CreationFunctionMap;
	typedef std::map<const CrcString*, ShaderTemplate *, LessPointerComparator>   ShaderTemplateMap;

	bool                 ms_installed;
	RecursiveMutex       ms_criticalSection;
	CreationFunctionMap  ms_creationFunctionMap;
	ShaderTemplateMap    ms_shaderTemplateMap;

	const StaticShader  *ms_2dVertexColorStaticShader;
	const StaticShader  *ms_2dVertexColorAStaticShader;
	const StaticShader  *ms_3dVertexColorStaticShader;
	const StaticShader  *ms_3dVertexColorAStaticShader;
	const StaticShader  *ms_3dVertexColorACStaticShader;
	const StaticShader  *ms_3dVertexColorAZStaticShader;
	const StaticShader  *ms_3dVertexColorZStaticShader;
	const StaticShader  *ms_3dVertexColorLZStaticShader;
	const StaticShader  *ms_badVertexShaderStaticShader;

	static const char   *ms_defaultShaderName = "shader/defaultshader.sht";

	char s_crashReportInfoLoadFromIff[MAX_PATH];
}
using namespace ShaderTemplateListNamespace;

// ======================================================================

void ShaderTemplateList::install()
{
	DEBUG_FATAL(ms_installed, ("ShaderTemplateList already installed"));

	if (TreeFile::isLoggingFiles())
		delete TreeFile::open(ms_defaultShaderName, AbstractFile::PriorityData, true);

	s_crashReportInfoLoadFromIff[0] = '\0';
	CrashReportInformation::addDynamicText(s_crashReportInfoLoadFromIff);

	ms_installed = true;
	ExitChain::add(ShaderTemplateList::remove, "ShaderTemplateList");
}

// ----------------------------------------------------------------------
/**
 * Register the load function for a ShaderTemplate-derived class.
 *
 * The implementer of a new ShaderTemplate-derived class must
 * announce the new class to the ShaderTemplateList before any
 * instances of the new class can be loaded.  This function allows
 * the caller to register a creation function for the new
 * ShaderTemplate-derived class.
 *
 * The caller should ensure ShaderTemplateList::deregisterShaderTemplateType()
 * when the derived class should no longer be supported (e.g. when the derived
 * class has its ::remove() called).
 *
 * @param tag         the first tag in the new ShaderTemplate-derived class' iff
 *                    data.  This tag must be unique among all supported ShaderTemplate-
 *                    derived classes.
 * @creationFunction  the creation function used to create an instance of the
 *                    ShaderTemplate-derived class.
 *
 * @see ShaderTemplateList::CreationFunction
 * @see ShaderTemplateList::deregisterShaderTemplateType()
 */

void ShaderTemplateList::registerShaderTemplateType(Tag tag, CreationFunction creationFunction)
{
	DEBUG_FATAL(!ms_installed, ("ShaderTemplateList not installed"));

	ms_criticalSection.enter();

		//-- check if the creation function has been registered already
		const CreationFunctionMap::iterator lowerBoundResult = ms_creationFunctionMap.lower_bound(tag);
		if ((lowerBoundResult != ms_creationFunctionMap.end()) && !ms_creationFunctionMap.key_comp()(tag, lowerBoundResult->first))
		{
			// creation function already registered for given tag
#ifdef _DEBUG
			char buffer[5];

			ConvertTagToString(tag, buffer);
			DEBUG_FATAL(true, ("attempted to register creation function for ShaderTemplate type [%s] multiple times", buffer));
#endif
		}
		else
		{
			//-- enter the creation function into the map
			IGNORE_RETURN(ms_creationFunctionMap.insert(lowerBoundResult, CreationFunctionMap::value_type(tag, creationFunction)));
		}

	ms_criticalSection.leave();
}

// ----------------------------------------------------------------------
/**
 * Deregister a ShaderTemplate-derived class previously registered.
 *
 * The caller should ensure ShaderTemplateList::deregisterShaderTemplateType()
 * when the derived class should no longer be supported (e.g. when the derived
 * class has its ::remove() called).
 *
 * @param tag  the first tag in the new ShaderTemplate-derived class' iff
 *             data.  This tag must be unique among all supported ShaderTemplate-
 *             derived classes.
 *
 * @see ShaderTemplateList::CreationFunction
 * @see ShaderTemplateList::registerShaderTemplateType()
 */

void ShaderTemplateList::deregisterShaderTemplateType(Tag tag)
{
	DEBUG_FATAL(!ms_installed, ("ShaderTemplateList not installed"));

	ms_criticalSection.enter();

		//-- check if the creation function has been registered
		CreationFunctionMap::iterator lowerBoundResult = ms_creationFunctionMap.lower_bound(tag);
		if ((lowerBoundResult != ms_creationFunctionMap.end()) && !ms_creationFunctionMap.key_comp()(tag, lowerBoundResult->first))
		{
			//-- remove creation function mapping
			ms_creationFunctionMap.erase(lowerBoundResult);
		}
		else
		{
			//-- mapping doesn't exist
#ifdef _DEBUG
			char buffer[5];

			ConvertTagToString(tag, buffer);
			DEBUG_FATAL(true, ("attempted to deregister creation function for ShaderTemplate type [%s], but not registered", buffer));
#endif
		}


	ms_criticalSection.leave();
}

// ----------------------------------------------------------------------

void ShaderTemplateList::assignAsynchronousLoaderFunctions()
{
	if (AsynchronousLoader::isInstalled())
		AsynchronousLoader::bindFetchReleaseFunctions("sht", &asynchronousLoaderFetchNoCreate, &asynchronousLoaderRelease);
}

// ----------------------------------------------------------------------

const void *ShaderTemplateList::asynchronousLoaderFetchNoCreate(const char *fileName)
{
	TemporaryCrcString crcName(fileName, true);
	bool error = false;
	return fetch(crcName, false, error);
}

// ----------------------------------------------------------------------

void ShaderTemplateList::asynchronousLoaderRelease(const void *shaderTemplate)
{
	static_cast<const ShaderTemplate *>(shaderTemplate)->release();
}

// ----------------------------------------------------------------------

const ShaderTemplate *ShaderTemplateList::fetch(const CrcString &name)
{
	bool error = false;
	return fetch(name, true, error);
}

//----------------------------------------------------------------------

const ShaderTemplate *ShaderTemplateList::fetch(const CrcString &name, bool & error)
{
	return fetch(name, true, error);
}

// ----------------------------------------------------------------------

const ShaderTemplate *ShaderTemplateList::fetch(const CrcString &name, bool create, bool & error)
{
	error = false;

	const ShaderTemplate *result = NULL;
	ms_criticalSection.enter();

		ShaderTemplateMap::iterator i = ms_shaderTemplateMap.find(&name);
		if (i != ms_shaderTemplateMap.end())
		{
			result = i->second;
			result->fetch();
		}
		else
			if (create)
			{
				Iff iff;
				if (!iff.open(name.getString(), true))
				{
					error = true;
					WARNING(true, ("could not open shader template %s", name.getString()));
					IGNORE_RETURN(iff.open(ms_defaultShaderName));
				}

				result = fetch(name, iff, error);
			}

	ms_criticalSection.leave();
	return result;
}

// ----------------------------------------------------------------------

const ShaderTemplate *ShaderTemplateList::fetch(const char *name)
{
	TemporaryCrcString crcName(name, true);
	return fetch(crcName);
}

// ----------------------------------------------------------------------

const ShaderTemplate *ShaderTemplateList::fetch(Iff &iff, bool & error)
{
	PersistentCrcString crcString;
	ms_criticalSection.enter();
		const ShaderTemplate *result = fetch(crcString, iff, error);
	ms_criticalSection.leave();

	return result;
}

//----------------------------------------------------------------------

const ShaderTemplate *ShaderTemplateList::fetch(Iff &iff)
{
	bool error = false;
	return ShaderTemplateList::fetch(iff, error);
}

// ----------------------------------------------------------------------

const Shader *ShaderTemplateList::fetchShader(const char *name)
{
	const ShaderTemplate *shaderTemplate = fetch(name);
		const Shader *shader = shaderTemplate->fetchShader();
	shaderTemplate->release();
	return shader;
}

// ----------------------------------------------------------------------

const Shader *ShaderTemplateList::fetchShader(const CrcString &name)
{
	const ShaderTemplate *shaderTemplate = fetch(name);
	const Shader *shader = shaderTemplate->fetchShader();
	shaderTemplate->release();
	return shader;
}

// ----------------------------------------------------------------------

Shader *ShaderTemplateList::fetchModifiableShader(const char *name)
{
	const ShaderTemplate *shaderTemplate = fetch(name);
	Shader *shader = shaderTemplate->fetchModifiableShader();
	shaderTemplate->release();
	return shader;
}

// ----------------------------------------------------------------------

Shader *ShaderTemplateList::fetchModifiableShader(const CrcString &name)
{
	const ShaderTemplate *shaderTemplate = fetch(name);
	Shader *shader = shaderTemplate->fetchModifiableShader();
	shaderTemplate->release();
	return shader;
}

// ----------------------------------------------------------------------

const Shader *ShaderTemplateList::fetchShader(Iff &iff)
{
	const ShaderTemplate *shaderTemplate = fetch(iff);
	const Shader *shader = shaderTemplate->fetchShader();
	shaderTemplate->release();
	return shader;
}

// ----------------------------------------------------------------------

Shader *ShaderTemplateList::fetchModifiableShader(Iff &iff)
{
	const ShaderTemplate *shaderTemplate = fetch(iff);
	Shader *shader = shaderTemplate->fetchModifiableShader();
	shaderTemplate->release();
	return shader;
}

// ----------------------------------------------------------------------

void ShaderTemplateList::preloadVertexColorShaderTemplates()
{
	ms_2dVertexColorStaticShader     = safe_cast<const StaticShader*>(fetchShader("shader/2d_vertexcolor.sht"));
	ms_2dVertexColorAStaticShader    = safe_cast<const StaticShader*>(fetchShader("shader/2d_vertexcolora.sht"));
	ms_3dVertexColorStaticShader     = safe_cast<const StaticShader*>(fetchShader("shader/3d_vertexcolor.sht"));
	ms_3dVertexColorAStaticShader    = safe_cast<const StaticShader*>(fetchShader("shader/3d_vertexcolora.sht"));
	ms_3dVertexColorACStaticShader   = safe_cast<const StaticShader*>(fetchShader("shader/3d_vertexcolorac.sht"));
	ms_3dVertexColorAZStaticShader   = safe_cast<const StaticShader*>(fetchShader("shader/3d_vertexcoloraz.sht"));
	ms_3dVertexColorZStaticShader    = safe_cast<const StaticShader*>(fetchShader("shader/3d_vertexcolorz.sht"));
	ms_3dVertexColorLZStaticShader   = safe_cast<const StaticShader*>(fetchShader("shader/3d_vertexcolorlz.sht"));
	ms_badVertexShaderStaticShader   = safe_cast<const StaticShader*>(fetchShader("shader/bad_vertex_shader.sht"));
}

// ----------------------------------------------------------------------

void ShaderTemplateList::releaseVertexColorShaderTemplates()
{
	if (ms_3dVertexColorStaticShader)
	{
		ms_2dVertexColorStaticShader->release();
		ms_2dVertexColorAStaticShader->release();
		ms_3dVertexColorStaticShader->release();
		ms_3dVertexColorAStaticShader->release();
		ms_3dVertexColorACStaticShader->release();
		ms_3dVertexColorAZStaticShader->release();
		ms_3dVertexColorZStaticShader->release();
		ms_3dVertexColorLZStaticShader->release();
		ms_badVertexShaderStaticShader->release();
		ms_2dVertexColorStaticShader = NULL;
		ms_2dVertexColorAStaticShader = NULL;
		ms_3dVertexColorStaticShader = NULL;
		ms_3dVertexColorAStaticShader = NULL;
		ms_3dVertexColorACStaticShader = NULL;
		ms_3dVertexColorAZStaticShader = NULL;
		ms_3dVertexColorZStaticShader = NULL;
		ms_3dVertexColorLZStaticShader = NULL;
		ms_badVertexShaderStaticShader = NULL;
	}
}

// ----------------------------------------------------------------------

const StaticShader &ShaderTemplateList::get2dVertexColorStaticShader()
{
	NOT_NULL(ms_2dVertexColorStaticShader);
	return *ms_2dVertexColorStaticShader;
}

// ----------------------------------------------------------------------

const StaticShader &ShaderTemplateList::get2dVertexColorAStaticShader()
{
	NOT_NULL(ms_2dVertexColorAStaticShader);
	return *ms_2dVertexColorAStaticShader;
}

// ----------------------------------------------------------------------

const StaticShader &ShaderTemplateList::get3dVertexColorStaticShader()
{
	NOT_NULL(ms_3dVertexColorStaticShader);
	return *ms_3dVertexColorStaticShader;
}

// ----------------------------------------------------------------------

const StaticShader &ShaderTemplateList::get3dVertexColorAStaticShader()
{
	NOT_NULL(ms_3dVertexColorAStaticShader);
	return *ms_3dVertexColorAStaticShader;
}

// ----------------------------------------------------------------------

const StaticShader &ShaderTemplateList::get3dVertexColorACStaticShader()
{
	NOT_NULL(ms_3dVertexColorACStaticShader);
	return *ms_3dVertexColorACStaticShader;
}

// ----------------------------------------------------------------------

const StaticShader &ShaderTemplateList::get3dVertexColorAZStaticShader()
{
	NOT_NULL(ms_3dVertexColorAZStaticShader);
	return *ms_3dVertexColorAZStaticShader;
}

// ----------------------------------------------------------------------

const StaticShader &ShaderTemplateList::get3dVertexColorZStaticShader()
{
	NOT_NULL(ms_3dVertexColorZStaticShader);
	return *ms_3dVertexColorZStaticShader;
}

// ----------------------------------------------------------------------

const StaticShader &ShaderTemplateList::get3dVertexColorLZStaticShader()
{
	NOT_NULL(ms_3dVertexColorLZStaticShader);
	return *ms_3dVertexColorLZStaticShader;
}

// ----------------------------------------------------------------------

const StaticShader *ShaderTemplateList::getBadVertexShaderStaticShader()
{
	return ms_badVertexShaderStaticShader;
}

// ======================================================================

void ShaderTemplateList::remove()
{
	DEBUG_FATAL(!ms_installed, ("ShaderTemplateList not installed"));

	ms_installed = false;

#ifdef _DEBUG
	//-- report shader leaks
	if (!ms_shaderTemplateMap.empty())
	{
		DEBUG_REPORT_LOG(true, ("ShaderTemplateList: [%u] ShaderTemplate instances still allocated:\n", ms_shaderTemplateMap.size()));

		ShaderTemplateMap::const_iterator end = ms_shaderTemplateMap.end();
		for (ShaderTemplateMap::const_iterator i = ms_shaderTemplateMap.begin(); i != end; ++i)
		{
			const ShaderTemplate *shaderTemplate = i->second;
			const int users = shaderTemplate->m_users;
			const char *name = shaderTemplate->getName().getString();
			DEBUG_REPORT_LOG(true, (" %3d %s\n", users, name));
			UNREF (users);
			UNREF (name);
		}
	}
#endif

	//-- clear the ShaderTemplate map	
	ms_shaderTemplateMap.clear();

#ifdef _DEBUG
	//-- report creation function registration leaks (these aren't really resource leaks, but could cause problems on multiple install/remove cycles)
	if (!ms_creationFunctionMap.empty())
	{
		char name[5];

		DEBUG_REPORT_LOG(true, ("ShaderTemplateList: [%u] registered types were not deregistered:\n", ms_creationFunctionMap.size()));

		const CreationFunctionMap::iterator endIt = ms_creationFunctionMap.end();
		for (CreationFunctionMap::iterator it = ms_creationFunctionMap.begin(); it != endIt; ++it)
		{
			ConvertTagToString(it->first, name);
			DEBUG_REPORT_LOG(true, ("|- for [%s]\n", name));
		}
	}
#endif

	//-- clear the CreationFunction map
	ms_creationFunctionMap.clear();

	CrashReportInformation::removeDynamicText(s_crashReportInfoLoadFromIff);
}

// ----------------------------------------------------------------------

const ShaderTemplate *ShaderTemplateList::fetch(const CrcString &name, Iff &iff, bool & error)
{
	//-- handle named ShaderTemplate chunks
	if (iff.getCurrentName() == TAG_NAME)
	{
		// handle fetching a named ShaderTemplate
		iff.enterChunk(TAG_NAME);

			char filename[1024];
			iff.read_string(filename, sizeof(filename) - 1);

		iff.exitChunk(TAG_NAME);

		return fetch(TemporaryCrcString(filename, true), error);
	}

#ifdef _DEBUG
	DataLint::pushAsset(iff.getFileName());
#endif // _DEBUG

	ShaderTemplate *const shaderTemplate = create(name, iff);
	NOT_NULL(shaderTemplate);

#ifdef _DEBUG
	DataLint::popAsset();
#endif // _DEBUG

	//-- fetch reference to ShaderTemplate for caller
	shaderTemplate->fetch();

	//-- keep track of named ShaderTemplate instances in the map
	const char *const cStringName = shaderTemplate->getName().getString();
	if (cStringName && *cStringName)
	{
		ShaderTemplateMap::value_type entry(&(shaderTemplate->getName()), shaderTemplate);
		std::pair<ShaderTemplateMap::iterator, bool> result = ms_shaderTemplateMap.insert(entry);
		DEBUG_FATAL(!result.second, ("insert failed"));
	}

	return shaderTemplate;
}

// ----------------------------------------------------------------------

ShaderTemplate *ShaderTemplateList::create(const CrcString &name, Iff &iff)
{
	//-- Track name of most recently loading shader template name to give us more 
	//   data to work with when we receive crash info.
	char const * const filename = iff.getFileName();
	IGNORE_RETURN(snprintf(s_crashReportInfoLoadFromIff, sizeof(s_crashReportInfoLoadFromIff) - 1, "ShaderTemplate_Iff: %s\n", (filename && *filename) ? filename : "<NULL>"));
	s_crashReportInfoLoadFromIff[sizeof(s_crashReportInfoLoadFromIff) - 1] = '\0';

	//-- look up the creation function based on the tag of the ShaderTemplate
	const Tag targetTag = iff.getCurrentName();

	CreationFunctionMap::iterator findIt = ms_creationFunctionMap.find(targetTag);

	if (findIt == ms_creationFunctionMap.end())
	{
		// unknown ShaderTemplate type
		char buffer[5];

		// extra debugging code to find frequent client crasher
		char mappings[256];
		char *m;
		m = mappings;
		for (CreationFunctionMap::iterator i = ms_creationFunctionMap.begin(); i != ms_creationFunctionMap.end(); ++i)
		{
			ConvertTagToString(i->first, buffer);
			sprintf(m, "%s:%p\n", buffer, i->second);
			m += strlen(m);
		}

		ConvertTagToString(targetTag, buffer);
		FATAL(true, ("Unknown shader template tag %s for %s\n%s", buffer, name.getString(), mappings));

		return NULL;
	}

	//-- create the ShaderTemplate instance via the creation function
	CreationFunction creationFunction = findIt->second;
	NOT_NULL(creationFunction);

	ShaderTemplate *result = (*creationFunction)(name, iff);
	if (!result)
	{
		char buffer[5];
		ConvertTagToString(targetTag, buffer);
		FATAL(true, ("creationFunction %p for shader template tag %s returned NULL for %s", creationFunction, buffer, name.getString()));
	}
	return result;
}

// ----------------------------------------------------------------------

void ShaderTemplateList::stopTracking(const ShaderTemplate *shaderTemplate)
{
	ms_criticalSection.enter();

	const char *name = shaderTemplate->getName().getString();
	if (name && *name)
	{
		ShaderTemplateMap::iterator it = ms_shaderTemplateMap.find(&shaderTemplate->getName());
		DEBUG_FATAL(it == ms_shaderTemplateMap.end(), ("Could not find named shader template %s", name));
		ms_shaderTemplateMap.erase(it);
	}

	ms_criticalSection.leave();
}

// ----------------------------------------------------------------------

void ShaderTemplateList::enterCriticalSection()
{
	ms_criticalSection.enter();
}

// ----------------------------------------------------------------------

void ShaderTemplateList::leaveCriticalSection()
{
	ms_criticalSection.leave();
}

// ======================================================================
