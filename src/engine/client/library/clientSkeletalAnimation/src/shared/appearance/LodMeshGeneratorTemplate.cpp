// ======================================================================
//
// LodMeshGeneratorTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/LodMeshGeneratorTemplate.h"

#include "clientSkeletalAnimation/BasicMeshGeneratorTemplate.h"
#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/MeshGeneratorTemplateList.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedFoundation/VoidMemberFunction.h"

#include <algorithm>

// ======================================================================

const Tag TAG_MLOD = TAG(M,L,O,D);

// ======================================================================

bool LodMeshGeneratorTemplate::ms_installed;
bool LodMeshGeneratorTemplate::ms_skipL0;

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(LodMeshGeneratorTemplate, true, 0, 0, 0);

// ======================================================================
// class LodMeshGeneratorTemplate: public static member functions
// ======================================================================

void LodMeshGeneratorTemplate::install(bool allowLod0Skipping)
{
	DEBUG_FATAL(ms_installed, ("LodMeshGeneratorTemplate already installed."));

	installMemoryBlockManager();

	if (allowLod0Skipping)
		ms_skipL0 = ConfigFile::getKeyBool ("ClientSkeletalAnimation", "skipL0", false);

	REPORT_LOG (ms_skipL0, ("LodMeshGeneratorTemplate: skipping L0\n"));

	const bool result = MeshGeneratorTemplateList::registerMeshGeneratorTemplate(TAG_MLOD, create, false, TemporaryCrcString("lmg", false));
	DEBUG_FATAL(!result, ("failed to register LodMeshGeneratorTemplate with List."));
	UNREF(result);

	ms_installed = true;
	ExitChain::add(remove, "LodMeshGeneratorTemplate");
}

// ======================================================================
// class LodMeshGeneratorTemplate: public member functions
// ======================================================================

int LodMeshGeneratorTemplate::getDetailCount() const
{
	return std::min(4, static_cast<int>(m_pathNames.size()));
}

// ----------------------------------------------------------------------

const BasicMeshGeneratorTemplate *LodMeshGeneratorTemplate::fetchBasicMeshGeneratorTemplate(int detailIndex) const
{
	//-- Validate preconditions.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, detailIndex, getDetailCount());

	if (!m_meshGeneratorTemplates[static_cast<BasicMeshGeneratorTemplateVector::size_type>(detailIndex)])
	{
		//-- Load the mesh generator template now.
		CrcLowerString const *const pathName = m_pathNames[static_cast<CrcStringVector::size_type>(detailIndex)];
		NOT_NULL(pathName);

		const MeshGeneratorTemplate *const meshGeneratorTemplate = MeshGeneratorTemplateList::fetch(*pathName);
		NOT_NULL(meshGeneratorTemplate);

		const BasicMeshGeneratorTemplate *const basicMeshGeneratorTemplate = safe_cast<const BasicMeshGeneratorTemplate*>(meshGeneratorTemplate);
		DEBUG_FATAL(!basicMeshGeneratorTemplate, ("LOD Mesh [%s]'s detail skeleton at index [%d] is not a BasicMeshGeneratorTemplate, bad data.", getName().getString(), detailIndex));

		m_meshGeneratorTemplates[static_cast<BasicMeshGeneratorTemplateVector::size_type>(detailIndex)] = basicMeshGeneratorTemplate;
	}

	//-- Retrieve the pointer to the specified LOD's BasicMeshGeneratorTemplate.
	const BasicMeshGeneratorTemplate *const skeletonTemplate = m_meshGeneratorTemplates[static_cast<BasicMeshGeneratorTemplateVector::size_type>(detailIndex)];
	NOT_NULL(skeletonTemplate);

	//-- Fetch a reference for the caller.
	skeletonTemplate->fetch();

	//-- Return it.
	return skeletonTemplate;
}

// ----------------------------------------------------------------------

void LodMeshGeneratorTemplate::preloadAssets() const
{
	//-- Fetch each mesh generator template.
	int const detailCount = getDetailCount();
	for (int i = 0; i < detailCount; ++i)
	{
		// This fetch will cause this LOD template to cache a copy of the template.
		BasicMeshGeneratorTemplate const *const bmgTemplate = fetchBasicMeshGeneratorTemplate(i);

		// Release the returned reference.  The cached copy still remains.
		if (bmgTemplate)
		{
			bmgTemplate->preloadAssets();
			bmgTemplate->release();
		}
	}
}

// ----------------------------------------------------------------------

void LodMeshGeneratorTemplate::garbageCollect() const
{
	//-- Clear out the cache of basic mesh generator templates.
	BasicMeshGeneratorTemplateVector::iterator const endIt = m_meshGeneratorTemplates.end();
	for (BasicMeshGeneratorTemplateVector::iterator it = m_meshGeneratorTemplates.begin(); it != endIt; ++it)
	{
		if (*it)
		{
			(*it)->release();
			(*it) = NULL;
		}
	}
}

// ======================================================================
// class LodMeshGeneratorTemplate: private static member functions
// ======================================================================

void LodMeshGeneratorTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("LodMeshGeneratorTemplate not installed."));
	ms_installed = false;

	const bool result = MeshGeneratorTemplateList::deregisterMeshGeneratorTemplate(TAG_MLOD);
	DEBUG_WARNING(!result, ("failed to deregister LodMeshGeneratorTemplate."));
	UNREF(result);

	removeMemoryBlockManager();
}

// ----------------------------------------------------------------------

MeshGeneratorTemplate *LodMeshGeneratorTemplate::create(Iff *iff, CrcString const &filename)
{
	NOT_NULL(iff);
	return new LodMeshGeneratorTemplate(*iff, filename);
}

// ======================================================================
// class LodMeshGeneratorTemplate: private member functions
// ======================================================================

LodMeshGeneratorTemplate::LodMeshGeneratorTemplate(Iff &iff, CrcString const &filename) :
	MeshGeneratorTemplate(filename),
	m_pathNames(),
	m_meshGeneratorTemplates()
{
	iff.enterForm(TAG_MLOD);

		const Tag version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
				load_0000(iff);
				break;
			
			default:
				{
					char buffer[5];

					ConvertTagToString(version, buffer);
					DEBUG_FATAL(true, ("unsupported LodMeshGeneratorTemplate version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_MLOD);
}

// ----------------------------------------------------------------------

LodMeshGeneratorTemplate::~LodMeshGeneratorTemplate()
{
	std::for_each(m_pathNames.begin(), m_pathNames.end(), PointerDeleter());

	BasicMeshGeneratorTemplateVector::iterator const endIt = m_meshGeneratorTemplates.end();
	for (BasicMeshGeneratorTemplateVector::iterator it = m_meshGeneratorTemplates.begin(); it != endIt; ++it)
	{
		if (*it)
			(*it)->release();
	}
}

// ----------------------------------------------------------------------

void LodMeshGeneratorTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_INFO);

			//-- Retrieve # detail levels.
			int lodCount = static_cast<int>(iff.read_int16());
			DEBUG_WARNING(ConfigClientSkeletalAnimation::getWarningTooManyLods() && (lodCount < 1 || lodCount > 4), 
				("LOD mesh generator template [%s] has bad # detail levels [%d].", iff.getFileName(), lodCount));

			if (lodCount < 0)
				lodCount = 0;

		iff.exitChunk(TAG_INFO);

		//-- Allocate storage.
		m_pathNames.reserve(static_cast<CrcStringVector::size_type>(lodCount));

		//-- Set all loaded meshes to NULL.
		m_meshGeneratorTemplates.resize(static_cast<BasicMeshGeneratorTemplateVector::size_type>(lodCount));
		for (int j = 0; j < lodCount; ++j)
			m_meshGeneratorTemplates[static_cast<BasicMeshGeneratorTemplateVector::size_type>(j)] = 0;

		//-- Load the LOD names.
		for (int i = 0; i < lodCount; ++i)
		{
			if ((i == 0) && (lodCount > 1) && ms_skipL0)
			{
				// Effectively ignore the L0 data.
				IGNORE_RETURN(iff.goForward());
			}
			else
			{
				char pathName[MAX_PATH * 2];

				iff.enterChunk(TAG_NAME);
					iff.read_string(pathName, sizeof(pathName) - 1);
					m_pathNames.push_back(new CrcLowerString(pathName, true));
				iff.exitChunk(TAG_NAME);
			}
		}

	iff.exitForm(TAG_0000);
}

// ======================================================================
