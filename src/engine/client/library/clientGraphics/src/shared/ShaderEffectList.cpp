// ======================================================================
//
// ShaderEffectList.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/ShaderEffectList.h"

#include "clientGraphics/ShaderEffect.h"
#include "sharedDebug/DataLint.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedSynchronization/RecursiveMutex.h"

#include <map>
#include <cstdio>

// ======================================================================

const Tag TAG_EFCT = TAG(E,F,C,T);

// ======================================================================

namespace ShaderEffectListNamespace
{
	typedef std::map<CrcString const *, ShaderEffect *, LessPointerComparator> ShaderEffectMap;

	RecursiveMutex    ms_criticalSection;
	ShaderEffectMap * ms_shaderEffectMap;
}
using namespace ShaderEffectListNamespace;


// ======================================================================

void ShaderEffectList::install()
{
	ExitChain::add(ShaderEffectList::remove, "ShaderEffectList");
	ms_shaderEffectMap = new ShaderEffectMap();
}

// ----------------------------------------------------------------------

void ShaderEffectList::remove()
{
#ifdef _DEBUG
	DEBUG_WARNING(!ms_shaderEffectMap->empty(), ("%u ShaderEffects still allocated", ms_shaderEffectMap->size()));

	const ShaderEffectMap::iterator endIt = ms_shaderEffectMap->end();
	for (ShaderEffectMap::iterator it = ms_shaderEffectMap->begin(); it != endIt; ++it)
		DEBUG_REPORT_LOG(true, ("--| leaked [%s]\n", it->first->getString()));
#endif _DEBUG

	delete ms_shaderEffectMap;
	ms_shaderEffectMap = NULL;
}

// ----------------------------------------------------------------------

void ShaderEffectList::assignAsynchronousLoaderFunctions()
{
	if (AsynchronousLoader::isInstalled())
		AsynchronousLoader::bindFetchReleaseFunctions("eft", &asynchronousLoaderFetchNoCreate, &asynchronousLoaderRelease);
}

// ----------------------------------------------------------------------

const void *ShaderEffectList::asynchronousLoaderFetchNoCreate(const char *fileName)
{
	TemporaryCrcString crcString(fileName, true);
	return fetch(crcString, false);
}

// ----------------------------------------------------------------------

void ShaderEffectList::asynchronousLoaderRelease(const void *shaderEffect)
{
	static_cast<const ShaderEffect *>(shaderEffect)->release();
}

// ----------------------------------------------------------------------

const ShaderEffect *ShaderEffectList::fetch(CrcString const &crcString)
{
	return fetch(crcString, true);
}

// ----------------------------------------------------------------------

const ShaderEffect *ShaderEffectList::fetch(const char *name)
{
	TemporaryCrcString crcString(name, true);
	return fetch(crcString);
}

// ----------------------------------------------------------------------

const ShaderEffect *ShaderEffectList::fetch(CrcString const &name, bool create)
{
	const ShaderEffect *result = NULL;
	ms_criticalSection.enter();

		// search for the file already being loaded
		ShaderEffectMap::iterator i = ms_shaderEffectMap->find(&name);
		if (i != ms_shaderEffectMap->end())
		{
			// file was already loaded, so return it
			result = i->second;
			result->fetch();
		}
		else
			if (create)
			{
				// load the file
				Iff iff;
				if (!iff.open (name.getString(), true))
				{
					WARNING(true, ("could not open effect %s", name.getString()));
					IGNORE_RETURN(iff.open("effect/defaulteffect.eft"));
				}

				result = fetch(name, iff);
			}

	ms_criticalSection.leave();

	return result;
}

// ----------------------------------------------------------------------

const ShaderEffect *ShaderEffectList::fetch(Iff &iff)
{
	PersistentCrcString crcString;
	ms_criticalSection.enter();
		const ShaderEffect *result = fetch(crcString, iff);
	ms_criticalSection.leave();
	return result;
}

// ----------------------------------------------------------------------

namespace E3Hacks
{
	const char* stripPath (const char* const filename)
	{
		NOT_NULL (filename);

		int         length  = strlen (filename);
		const char* current = filename + length;

		//-- start from end of string and search backward for '\' or '/'
		while (current != filename && !(*current == '\\' || *current == '/'))
			--current;

		if (*current == '\\' || *current == '/')
			++current;

		return current;
	}
};

// ----------------------------------------------------------------------

const ShaderEffect *ShaderEffectList::fetch(CrcString const &name, Iff &iff)
{
	switch (iff.getCurrentName())
	{
		case TAG_NAME:
			{
				iff.enterChunk(TAG_NAME);
					char buffer[256];
					iff.read_string(buffer, sizeof(buffer));
				iff.exitChunk(TAG_NAME);

#if 1
				//-- sorry jeff, we have too many bad assets... :)  see if the effect exists, and
				//   if it doesn't, strip the path and extension and see if it exists again. this
				//   is to fix the artists not setting up the paths correctly in the shader builder
				if (!TreeFile::exists (buffer))
				{
					char newName [256];
					WARNING (true, ("%s: full path effect name (%s) detected", iff.getFileName (), buffer));
					sprintf (newName, "effect\\%s", E3Hacks::stripPath (buffer));
					strcpy (buffer, newName);
				}
#endif

				// TODO: what do I want to do about effects with two valid names?
				return fetch(buffer);
			}

		case TAG_EFCT:
			{
#ifdef _DEBUG
				DataLint::pushAsset(iff.getFileName());
#endif // _DEBUG

				ShaderEffect *shaderEffect = new ShaderEffect(name, iff);
				if (!name.isEmpty())
				{
					bool inserted = ms_shaderEffectMap->insert(ShaderEffectMap::value_type(&shaderEffect->getCrcString(), shaderEffect)).second;
					UNREF(inserted);
					DEBUG_FATAL(!inserted, ("insert failed"));
				}

				shaderEffect->fetch();

#ifdef _DEBUG
				DataLint::popAsset();
#endif // _DEBUG

				return shaderEffect;
			}

		default:
			{
#ifdef _DEBUG
				char buffer[256];
				iff.formatLocation(buffer, sizeof(buffer));
				DEBUG_FATAL(true, ("Unknown effect tag in %s", buffer));
#endif
			}
			return NULL; //lint !e527 // Unreachable
	}
}

// ----------------------------------------------------------------------

void ShaderEffectList::remove(const ShaderEffect *shaderEffect)
{
	if (!shaderEffect->getCrcString().isEmpty())
	{
		ShaderEffectMap::iterator it = ms_shaderEffectMap->find(&shaderEffect->getCrcString());
		DEBUG_FATAL(it == ms_shaderEffectMap->end(), ("Could not find named shader effect %s", shaderEffect->getName()));
		ms_shaderEffectMap->erase(it);
	}
}

// ----------------------------------------------------------------------

void ShaderEffectList::enterCriticalSection()
{
	ms_criticalSection.enter();
}

// ----------------------------------------------------------------------

void ShaderEffectList::leaveCriticalSection()
{
	ms_criticalSection.leave();
}

// ======================================================================
