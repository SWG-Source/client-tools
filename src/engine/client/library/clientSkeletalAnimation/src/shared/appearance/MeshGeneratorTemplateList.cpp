// ======================================================================
//
// MeshGeneratorTemplateList.cpp
// Copyright 2001-2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/MeshGeneratorTemplateList.h"

#include "clientSkeletalAnimation/MeshGeneratorTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedSynchronization/Mutex.h"

#include <algorithm>
#include <map>

// ======================================================================

namespace MeshGeneratorTemplateListNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class BindData
	{
	public:

		BindData(MeshGeneratorTemplateList::CreationFunction creationFunction, bool newSupportsAsynchronousLoading, CrcString const &fileExtension);

		MeshGeneratorTemplateList::CreationFunction  getCreationFunction() const;
		bool                                         supportsAsynchronousLoading() const;
		CrcString const                             &getFileExtension() const;

	private:

		// Disabled.
		BindData();
		BindData(BindData const & rhs);
		BindData & operator=(BindData const & rhs);

	private:

		MeshGeneratorTemplateList::CreationFunction m_creationFunction;
		bool                                        m_supportsAsynchronousLoading;
		PersistentCrcString                         m_fileExtension;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::map<CrcString const*, MeshGeneratorTemplate*, LessPointerComparator>  MeshGeneratorTemplateMap;
	typedef std::map<Tag, BindData*>                                                   TagBindDataMap;
	typedef std::map<CrcString const*, BindData const*, LessPointerComparator>         ExtensionBindDataMap;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void		remove();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool                      s_installed;
	MeshGeneratorTemplateMap  s_templates;
	TagBindDataMap            s_bindDataMap;
	ExtensionBindDataMap      s_fileExtensionMap;

	Mutex                     s_criticalSection;

	Iff                      *s_iff;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

}

using namespace MeshGeneratorTemplateListNamespace;

// ======================================================================
// namespace MeshGeneratorTemplateListNamespace
// ======================================================================

MeshGeneratorTemplateListNamespace::BindData::BindData(MeshGeneratorTemplateList::CreationFunction creationFunction, bool newSupportsAsynchronousLoading, CrcString const &fileExtension) :
	m_creationFunction(creationFunction),
	m_supportsAsynchronousLoading(newSupportsAsynchronousLoading),
	m_fileExtension(fileExtension)
{
}

// ----------------------------------------------------------------------

inline MeshGeneratorTemplateList::CreationFunction MeshGeneratorTemplateListNamespace::BindData::getCreationFunction() const
{
	return m_creationFunction;
}

// ----------------------------------------------------------------------

inline bool MeshGeneratorTemplateListNamespace::BindData::supportsAsynchronousLoading() const
{
	return m_supportsAsynchronousLoading;
}

// ----------------------------------------------------------------------

inline CrcString const &MeshGeneratorTemplateListNamespace::BindData::getFileExtension() const
{
	return m_fileExtension;
}

// ======================================================================

void MeshGeneratorTemplateListNamespace::remove()
{
	//-- check if we're not installed
	DEBUG_FATAL(!s_installed, ("MeshGeneratorTemplateList not installed"));
	s_installed = false;

	//-- handle unreleased named mesh generators
	if (!s_templates.empty())
	{
		DEBUG_WARNING(true, ("MeshGeneratorTemplateList leaks: %d named mesh generators leaked", s_templates.size()));
		
		const MeshGeneratorTemplateMap::iterator endIt = s_templates.end();
		for (MeshGeneratorTemplateMap::iterator it = s_templates.begin();it != endIt; ++it)
		{
			// print name of leaked MeshGenerator
#ifdef _DEBUG
			CrcString const *const name = it->first;
			DEBUG_WARNING(true, ("MeshGeneratorTemplateList leaks: -- leaked \"%s\"", name ? name->getString() : "<null name>"));
#endif

			// note: we can't delete these items because they're already blown away by this point in the exit chain.
			// delete meshGeneratorTemplate;
			(*it).second = 0;
		}

		DEBUG_WARNING(true, ("MeshGeneratorTemplateList leaks: end named leaks"));
	}

	//-- Release bind data.
	std::for_each(s_bindDataMap.begin(), s_bindDataMap.end(), PointerDeleterPairSecond());

	//-- Shrink down containers.
	TagBindDataMap().swap(s_bindDataMap);
	ExtensionBindDataMap().swap(s_fileExtensionMap);
	MeshGeneratorTemplateMap().swap(s_templates);
}

// ======================================================================
// class MeshGeneratorTemplateList: PUBLIC STATIC
// ======================================================================

void MeshGeneratorTemplateList::install()
{
	// check if we're already installed
	DEBUG_FATAL(s_installed, ("MeshGeneratorTemplateList already installed"));
	if (s_installed)
		return;

	s_installed = true;
	ExitChain::add(remove, "MeshGeneratorTemplateList");
}

// ----------------------------------------------------------------------

bool MeshGeneratorTemplateList::registerMeshGeneratorTemplate(Tag meshGeneratorTag, CreationFunction creationFunction, bool supportsAsynchronousLoading, CrcString const &fileExtension)
{
	DEBUG_FATAL(!s_installed, ("MeshGeneratorTemplateList not installed"));
	NOT_NULL(creationFunction);

	//-- Check if tag already registered.
	TagBindDataMap::const_iterator searchIt = s_bindDataMap.find(meshGeneratorTag);
	if (searchIt != s_bindDataMap.end())
	{
		// Failed to register --- item with tag is already registered.
#ifdef _DEBUG
		char name[5];
		ConvertTagToString(meshGeneratorTag, name);
		DEBUG_FATAL(true, ("MeshGeneratorTemplate: tried to register multiple classes with tag [%s].", name));
#endif
		return false; //lint !e527 // Unreachable // Reachable in release.
	}

	//-- Register the creation function.

	// Add all bindData to the s_bindDataMap map.  This list has all bind entries.
	BindData *const bindData = new BindData(creationFunction, supportsAsynchronousLoading, fileExtension);
	IGNORE_RETURN(s_bindDataMap.insert(TagBindDataMap::value_type(meshGeneratorTag, bindData)));

	// Only add asynchronous load support values to the file extension map.  This map does not own the bind data.
	if (supportsAsynchronousLoading && fileExtension.getString() && strlen(fileExtension.getString()))
		IGNORE_RETURN(s_fileExtensionMap.insert(ExtensionBindDataMap::value_type(&bindData->getFileExtension(), bindData)));

	return true;
}

// ----------------------------------------------------------------------

bool MeshGeneratorTemplateList::deregisterMeshGeneratorTemplate(Tag meshGeneratorTag)
{
	DEBUG_FATAL(!s_installed, ("MeshGeneratorTemplateList not installed"));

	//-- Check if tag has been registered.
	TagBindDataMap::iterator searchIt = s_bindDataMap.find(meshGeneratorTag);
	if (searchIt == s_bindDataMap.end())
	{
		// failed to find creation functioned tagged with given mesh generator
#ifdef _DEBUG
		char name[5];
		ConvertTagToString(meshGeneratorTag, name);
		DEBUG_FATAL(true, ("MeshGeneratorTemplate: tried to deregister non-registered class with tag [%s].", name));
#endif
		return false; //lint !e527 // Unreachable // Reachable in release.
	}
	
	//-- Get the bind data.
	BindData *const bindData = searchIt->second;
	NOT_NULL(bindData);

	//-- Find the map entry in the filename extension map.
	ExtensionBindDataMap::iterator const extIt = s_fileExtensionMap.find(&bindData->getFileExtension());
	if (extIt != s_fileExtensionMap.end())
		s_fileExtensionMap.erase(extIt);

	//-- Remove from tag map.
	s_bindDataMap.erase(searchIt);

	//-- Delete it.
	delete bindData;

	return true;
}

// ----------------------------------------------------------------------

const MeshGeneratorTemplate *MeshGeneratorTemplateList::fetch(const CrcString &filename)
{
	return fetch(filename, true);
}

// ----------------------------------------------------------------------

const MeshGeneratorTemplate *MeshGeneratorTemplateList::fetch(Iff &iff)
{
	DEBUG_FATAL(!s_installed, ("MeshGeneratorTemplateList not installed"));

	//-- First check for a named chunk (a chunk called NAME that contains a filename for the real data).
	const Tag meshGeneratorTag = iff.getCurrentName();
	if (meshGeneratorTag == TAG_NAME)
	{
		char pathName[1024];

		iff.enterChunk(TAG_NAME);
			iff.read_string(pathName, sizeof(pathName) - 1);
		iff.exitChunk(TAG_NAME);

		return fetch(TemporaryCrcString(pathName, true));
	}
	
	//-- Lookup mesh generator creation function
	TagBindDataMap::iterator it = s_bindDataMap.find(meshGeneratorTag);

#ifdef _DEBUG
	if (it == s_bindDataMap.end())
	{
		char tagName[5];
		ConvertTagToString(meshGeneratorTag, tagName);
		DEBUG_FATAL(true, ("failed to find creation function for mesh generator [%s]\n", tagName));
	}
#endif

	BindData const *const bindData = it->second;
	NOT_NULL(bindData);

	CreationFunction creationFunction = bindData->getCreationFunction();

	//-- create the mesh generator
	const MeshGeneratorTemplate *const meshGeneratorTemplate = (*creationFunction)(&iff, TemporaryCrcString("", false));
	NOT_NULL(meshGeneratorTemplate);

	//-- increment ref count and add to anonymous mesh generator list
	meshGeneratorTemplate->fetch();

	return meshGeneratorTemplate;
}

// ----------------------------------------------------------------------

void MeshGeneratorTemplateList::assignAsynchronousLoaderFunctions()
{
	if (AsynchronousLoader::isInstalled())
		AsynchronousLoader::bindFetchReleaseFunctions("mgn", &asynchronousLoaderFetchNoCreate, &asynchronousLoaderRelease);
}

// ----------------------------------------------------------------------

void MeshGeneratorTemplateList::debugDump()
{
#ifdef _DEBUG
	s_criticalSection.enter();

		DEBUG_REPORT_LOG(true, ("====\nMeshGeneratorTemplates currently loaded [count=%d]:\n", static_cast<int>(s_templates.size())));

		MeshGeneratorTemplateMap::iterator const endIt = s_templates.end();
		for (MeshGeneratorTemplateMap::iterator it = s_templates.begin(); it != endIt; ++it)
		{
			char const *const                  name           = (it->first ? it->first->getString() : "<NULL name>");
			MeshGeneratorTemplate const *const mgTemplate     = it->second;
			int const                          referenceCount = (mgTemplate ? mgTemplate->getReferenceCount() : 0);

			DEBUG_REPORT_LOG(true, ("  name=[%s],refCount=[%d]\n", name, referenceCount));
		}

		DEBUG_REPORT_LOG(true, ("====\n"));

	s_criticalSection.leave();
#endif
}

// ======================================================================

void MeshGeneratorTemplateList::stopTracking(const MeshGeneratorTemplate &meshGeneratorTemplate)
{
	DEBUG_FATAL(!s_installed, ("MeshGeneratorTemplateList not installed"));

	s_criticalSection.enter();

	//-- get the MeshGenerator name
	const CrcString &name = meshGeneratorTemplate.getName();
	if (!name.getString() || !*name.getString())
	{
		s_criticalSection.leave();
		return;
	}

	//-- check if we have it in the list
	std::pair<MeshGeneratorTemplateMap::iterator, MeshGeneratorTemplateMap::iterator> rangePair = s_templates.equal_range(&name);
	if (rangePair.first != rangePair.second)
	{
		//-- we've found it
		s_templates.erase(rangePair.first);
	}
	else
	{
		DEBUG_WARNING(true, ("stopTracking() failed to find named MeshGeneratorTemplate [%s]", name.getString()));
#if 1
		debugDump();
#endif
	}

	s_criticalSection.leave();
}

// ----------------------------------------------------------------------

const void *MeshGeneratorTemplateList::asynchronousLoaderFetchNoCreate(char const *fileName)
{
	TemporaryCrcString cfn(fileName, true);
	return MeshGeneratorTemplateList::fetch(cfn, false);
}

// ----------------------------------------------------------------------

void MeshGeneratorTemplateList::asynchronousLoaderRelease(void const *meshGeneratorTemplate)
{
	static_cast<MeshGeneratorTemplate const *>(meshGeneratorTemplate)->release();
}

// ----------------------------------------------------------------------

const MeshGeneratorTemplate *MeshGeneratorTemplateList::fetch(const CrcString &filename, bool create)
{
	DEBUG_FATAL(!s_installed, ("MeshGeneratorTemplateList not installed"));
	DEBUG_FATAL(!filename.getString() || !*filename.getString(), ("bad filename arg"));

	MeshGeneratorTemplate *meshGeneratorTemplate = 0;

	s_criticalSection.enter();

#ifdef _DEBUG
		DataLint::pushAsset(filename.getString());
#endif

		//-- Check if we've already fetched this mesh generator.
		MeshGeneratorTemplateMap::iterator lowerBoundIt = s_templates.lower_bound(&filename);
		if ((lowerBoundIt != s_templates.end()) && !s_templates.key_comp()(&filename, lowerBoundIt->first))
		{
			// We've got it: return the cached mesh generator.
			meshGeneratorTemplate = lowerBoundIt->second;
		}
		else
		{
			// We don't have the mesh generator cached.  Load it.

			// Return immediately if caller doesn't want to load.
			if (!create)
			{
				s_criticalSection.leave();
				return 0;
			}

			BindData const *asyncBindData = 0;

			//-- Check if we should asychronously load based on extension.
			char const *const constCharFilename = filename.getString();
			char const *      extension         = strrchr(constCharFilename, '.');
			if (extension)
				++extension;

			if (extension)
			{
				TemporaryCrcString  crcExtension(extension, false);
				ExtensionBindDataMap::const_iterator const asyncFindIt = s_fileExtensionMap.find(&crcExtension);
				if (asyncFindIt != s_fileExtensionMap.end())
					asyncBindData = asyncFindIt->second;
			}

			if (AsynchronousLoader::isEnabled() && asyncBindData && asyncBindData->supportsAsynchronousLoading())
			{
				// Handle async loading.
				CreationFunction creationFunction = asyncBindData->getCreationFunction();

				//-- Asyncronouly create the mesh generator.  The NULL Iff parameter indicates
				//   we are asynchronously loading.
				meshGeneratorTemplate = (*creationFunction)(NULL, filename);
				NOT_NULL(meshGeneratorTemplate);
			}
			else
			{
				// Handle synchronous loading.

				//-- Open the iff.
				FATAL(s_iff != NULL, ("MeshGeneratorTemplateList: static Iff pointer is not NULL but should be, invalid state."));

				s_iff = new Iff();
				FATAL(s_iff == NULL, ("MeshGeneratorTemplateList: new Iff() returned NULL."));

				const bool loadSuccess = s_iff->open(filename.getString(), true);
				if (!loadSuccess)
				{
					WARNING(true, ("Failed to load MeshGeneratorTemplate file [%s].", filename.getString()));
					delete s_iff;
					s_iff = NULL;
					s_criticalSection.leave();
#ifdef _DEBUG
					DataLint::popAsset();
#endif // _DEBUG
					return 0;
				}

				//-- get form name, should be name of mesh generator
				Tag meshGeneratorTag = s_iff->getCurrentName();
				
				//-- lookup mesh generator creation function
				TagBindDataMap::iterator it = s_bindDataMap.find(meshGeneratorTag);

#ifdef _DEBUG
				if (it == s_bindDataMap.end())
				{
					char tagName[5];
					ConvertTagToString(meshGeneratorTag, tagName);
					DEBUG_FATAL(true, ("failed to find creation function for mesh generator tag type [%s]\n", tagName));
				}
#endif

				BindData const *const bindData = it->second;
				if (!bindData)
				{
#ifdef _DEBUG
					char tagName[5];
					ConvertTagToString(meshGeneratorTag, tagName);
					DEBUG_FATAL(true, ("MeshGeneratorTemplateList: NULL bindData mapping for mesh generator tag type [%s].", tagName));
#endif
					delete s_iff;
					s_iff = NULL;
					s_criticalSection.leave();
#ifdef _DEBUG
					DataLint::popAsset();
#endif // _DEBUG
					return NULL; //lint !e527 // unreachable // reachable in release.
				}

				CreationFunction creationFunction = bindData->getCreationFunction();
				if (!creationFunction)
				{
#ifdef _DEBUG
					char tagName[5];
					ConvertTagToString(meshGeneratorTag, tagName);
					DEBUG_FATAL(true, ("MeshGeneratorTemplateList: NULL bindData->getCreationFunction() for mesh generator tag type [%s].", tagName));
#endif
					delete s_iff;
					s_iff = NULL;
					s_criticalSection.leave();
#ifdef _DEBUG
					DataLint::popAsset();
#endif // _DEBUG
					return NULL; //lint !e527 // unreachable // reachable in release.
				}

				//-- create the mesh generator
				meshGeneratorTemplate = (*creationFunction)(s_iff, filename);
				NOT_NULL(meshGeneratorTemplate);

				delete s_iff;
				s_iff = NULL;
			}

			//-- Add to named generator list.
			IGNORE_RETURN(s_templates.insert(lowerBoundIt, MeshGeneratorTemplateMap::value_type(&meshGeneratorTemplate->getName(), meshGeneratorTemplate)));
		}

		//-- increment ref count
		meshGeneratorTemplate->fetch();

#ifdef _DEBUG
		DataLint::popAsset();
#endif // _DEBUG

	s_criticalSection.leave();

	return meshGeneratorTemplate;
}

// ==================================================================
