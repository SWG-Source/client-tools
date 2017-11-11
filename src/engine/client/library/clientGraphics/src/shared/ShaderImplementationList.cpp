// ======================================================================
//
// ShaderImplementationList.cpp
//
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/ShaderImplementationList.h"

#include "clientGraphics/ShaderImplementation.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/StringCompare.h"
#include "sharedSynchronization/RecursiveMutex.h"

#include <map>

// ======================================================================

RecursiveMutex ShaderImplementationList::ms_criticalSection;
ShaderImplementationList::ShaderImplementationMap *ShaderImplementationList::ms_shaderImplementationMap;
static const Tag                                   TAG_IMPL = TAG (I,M,P,L);

// ======================================================================

void ShaderImplementationList::install()
{
	ExitChain::add(ShaderImplementationList::remove, "ShaderImplementationList");
	ms_shaderImplementationMap = new ShaderImplementationMap;
}

// ----------------------------------------------------------------------

void ShaderImplementationList::remove()
{
	DEBUG_FATAL(!ms_shaderImplementationMap->empty(), ("ShaderImplementations still allocated"));
	delete ms_shaderImplementationMap;
	ms_shaderImplementationMap = NULL;
}
	
// ----------------------------------------------------------------------

const ShaderImplementation *ShaderImplementationList::fetch(const char *name)
{
	const ShaderImplementation *result = NULL;

	ms_criticalSection.enter();

		// search for the file already being loaded
		ShaderImplementationMap::iterator i = ms_shaderImplementationMap->find(name);
		if (i != ms_shaderImplementationMap->end())
		{
			// file was already loaded, so return it
			result = i->second;
			result->fetch();
		}
		else
		{
			// load the file
			Iff iff(name);
			result = fetch(name, iff);
		}

	ms_criticalSection.leave();
	return result;
}

// ----------------------------------------------------------------------

const ShaderImplementation *ShaderImplementationList::fetch(Iff &iff)
{
	ms_criticalSection.enter();
		const ShaderImplementation *result = fetch(NULL, iff);
	ms_criticalSection.leave();
	return result;
}

// ----------------------------------------------------------------------

const ShaderImplementation *ShaderImplementationList::fetch(const char *name, Iff &iff)
{
	switch (iff.getCurrentName())
	{
		case TAG_NAME:
			{
				iff.enterChunk(TAG_NAME);
					char buffer[256];
					iff.read_string(buffer, sizeof(buffer));
				iff.exitChunk(TAG_NAME);

				// TODO: what do I want to do about implementations with two valid names?
				return fetch(buffer);
			}

		case TAG_IMPL	:
			{
				ShaderImplementation *shaderImplementation = new ShaderImplementation(name, iff);
				if (!shaderImplementation->isSupported())
				{
					delete shaderImplementation;
					return NULL;
				}

				if (name)
				{
					ShaderImplementationMap::value_type entry(shaderImplementation->getName(), shaderImplementation);
					std::pair<ShaderImplementationMap::iterator, bool> result = ms_shaderImplementationMap->insert(entry);
					DEBUG_FATAL(!result.second, ("insert failed"));
				}

				shaderImplementation->fetch();
				return shaderImplementation;
			}

		default:
			{
#ifdef _DEBUG
				char buffer[256];
				iff.formatLocation(buffer, sizeof(buffer));
				DEBUG_FATAL(true, ("Unknown implementation tag in %s", buffer));
#endif
			}
			return NULL;  //lint !e527 // Unreachable
	}
}

// ----------------------------------------------------------------------

void ShaderImplementationList::remove(const ShaderImplementation *shaderImplementation)
{
	const char *name = shaderImplementation->getName();
	if (name)
	{
		ShaderImplementationMap::iterator it = ms_shaderImplementationMap->find(name);
		DEBUG_FATAL(it == ms_shaderImplementationMap->end(), ("Could not find named shader implementation %s", name));
		ms_shaderImplementationMap->erase(it);
	}
}

// ======================================================================
