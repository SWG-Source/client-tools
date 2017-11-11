// ======================================================================
//
// LodSkeletonTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/LodSkeletonTemplate.h"

#include "clientSkeletalAnimation/BasicSkeletonTemplate.h"
#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletonTemplateList.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/VoidMemberFunction.h"

#include <algorithm>

// ======================================================================

const Tag TAG_SLOD = TAG(S,L,O,D);

// ======================================================================

bool LodSkeletonTemplate::ms_installed;
bool LodSkeletonTemplate::ms_skipL0;

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(LodSkeletonTemplate, true, 0, 0, 0);

// ======================================================================
// class LodSkeletonTemplate: public static member functions
// ======================================================================

void LodSkeletonTemplate::install(bool allowLod0Skipping)
{
	DEBUG_FATAL(ms_installed, ("LodSkeletonTemplate already installed."));

	installMemoryBlockManager();
	SkeletonTemplateList::registerCreateFunction(TAG_SLOD, create);

	if (allowLod0Skipping)
		ms_skipL0 = ConfigFile::getKeyBool ("ClientSkeletalAnimation", "skipL0", false);

   	REPORT_LOG (ms_skipL0, ("LodSkeletonTemplate: skipping L0\n"));

	ms_installed = true;
	ExitChain::add(remove, "LodSkeletonTemplate");
}

// ======================================================================
// class LodSkeletonTemplate: public member functions
// ======================================================================

int LodSkeletonTemplate::getDetailCount() const
{
	return std::min(4, static_cast<int>(m_skeletonTemplates.size()));
}

// ----------------------------------------------------------------------

const BasicSkeletonTemplate *LodSkeletonTemplate::fetchBasicSkeletonTemplate(int detailIndex) const
{
	//-- Validate preconditions.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, detailIndex, getDetailCount());

	//-- Retrieve the pointer to the specified LOD's BasicSkeletonTemplate.
	const BasicSkeletonTemplate *const skeletonTemplate = m_skeletonTemplates[static_cast<BasicSkeletonTemplateVector::size_type>(detailIndex)];
	NOT_NULL(skeletonTemplate);

	//-- Fetch a reference for the caller.
	skeletonTemplate->fetch();

	//-- Return it.
	return skeletonTemplate;
}

// ======================================================================
// class LodSkeletonTemplate: private static member functions
// ======================================================================

void LodSkeletonTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("LodSkeletonTemplate not installed."));

	SkeletonTemplateList::deregisterCreateFunction(TAG_SLOD);
	removeMemoryBlockManager();

	ms_installed = false;
}

// ----------------------------------------------------------------------

SkeletonTemplate *LodSkeletonTemplate::create(Iff &iff, CrcString const &filename)
{
	return new LodSkeletonTemplate(iff, filename);
}

// ======================================================================
// class LodSkeletonTemplate: private member functions
// ======================================================================

LodSkeletonTemplate::LodSkeletonTemplate(Iff &iff, CrcString const &filename) :
	SkeletonTemplate(filename),
	m_skeletonTemplates()
{
#ifdef _DEBUG
	if (DataLint::isEnabled())
		DataLint::pushAsset(filename.getString());
#endif

	iff.enterForm(TAG_SLOD);

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
					DEBUG_FATAL(true, ("unsupported LodSkeletonTemplate version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_SLOD);

#ifdef _DEBUG
	if (DataLint::isEnabled())
		DataLint::popAsset();
#endif
}

// ----------------------------------------------------------------------

LodSkeletonTemplate::~LodSkeletonTemplate()
{
	std::for_each(m_skeletonTemplates.begin(), m_skeletonTemplates.end(), VoidMemberFunction(&SkeletonTemplate::release));
}

// ----------------------------------------------------------------------

void LodSkeletonTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_INFO);

			//-- Retrieve # detail levels.
			int lodCount = static_cast<int>(iff.read_int16());
			DEBUG_WARNING(ConfigClientSkeletalAnimation::getWarningTooManyLods()  && (lodCount < 1 || lodCount > 4), 
				("LOD skeleton template [%s] has bad # detail levels [%d].", iff.getFileName(), lodCount));

			if (lodCount < 0)
				lodCount = 0;

		iff.exitChunk(TAG_INFO);

		//-- Allocate storage.
		m_skeletonTemplates.reserve(static_cast<BasicSkeletonTemplateVector::size_type>(lodCount));

		//-- Load the LODs.
		for (int i = 0; i < lodCount; ++i)
		{
			if ((i == 0) && (lodCount > 1) && ms_skipL0)
			{
				// Effectively ignore the L0 data.
				IGNORE_RETURN(iff.goForward());
			}
			else
			{
				const SkeletonTemplate *const skeletonTemplate = SkeletonTemplateList::fetch(iff);
				NOT_NULL(skeletonTemplate);

				const BasicSkeletonTemplate *const basicSkeletonTemplate = safe_cast<const BasicSkeletonTemplate*>(skeletonTemplate);
				DEBUG_FATAL(!basicSkeletonTemplate, ("LOD Skeleton [%s]'s detail skeleton at index [%d] is not a BasicSkeletonTemplate, bad data.", getName().getString(), i));
				m_skeletonTemplates.push_back(basicSkeletonTemplate);
			}
		}

	iff.exitForm(TAG_0000);
}

// ======================================================================
