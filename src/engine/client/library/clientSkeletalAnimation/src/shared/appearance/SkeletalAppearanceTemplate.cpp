// ======================================================================
//
// SkeletalAppearanceTemplate.cpp
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAppearanceTemplate.h"

#include "clientGraphics/LodDistanceTable.h"
#include "clientSkeletalAnimation/BasicSkeletonTemplate.h"
#include "clientSkeletalAnimation/LogicalAnimationTableTemplate.h"
#include "clientSkeletalAnimation/LogicalAnimationTableTemplateList.h"
#include "clientSkeletalAnimation/MeshGeneratorTemplate.h"
#include "clientSkeletalAnimation/MeshGeneratorTemplateList.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "clientSkeletalAnimation/SkeletonTemplate.h"
#include "clientSkeletalAnimation/SkeletonTemplateList.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/Tag.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Object.h"

#include <algorithm>
#include <map>
#include <string>

// ======================================================================
// module constants
// ======================================================================

const Tag TAG_APAG = TAG(A,P,A,G);
const Tag TAG_LATX = TAG(L,A,T,X);
const Tag TAG_LDTB = TAG(L,D,T,B);
const Tag TAG_MSGN = TAG(M,S,G,N);
const Tag TAG_SFSK = TAG(S,F,S,K);
const Tag TAG_SKTI = TAG(S,K,T,I);
const Tag TAG_SMAT = TAG(S,M,A,T);

// ======================================================================
// static storage
// ======================================================================

bool SkeletalAppearanceTemplate::ms_installed;

// ======================================================================
// embedded entity declarations
// ======================================================================

struct SkeletalAppearanceTemplate::SkeletonTemplateInfo
{
public:

	SkeletonTemplateInfo(const char *skeletonTemplateName, const char *attachmentTransformName);

public:

	CrcLowerString  m_skeletonTemplateName;
	CrcLowerString  m_attachmentTransformName;

private:
	// disabled
	SkeletonTemplateInfo();
};

// ======================================================================
// struct SkeletalAppearanceTemplate::SkeletonTemplateInfo
// ======================================================================

SkeletalAppearanceTemplate::SkeletonTemplateInfo::SkeletonTemplateInfo(const char *skeletonTemplateName, const char *attachmentTransformName) :
	m_skeletonTemplateName(skeletonTemplateName),
	m_attachmentTransformName(attachmentTransformName)
{
}

// ======================================================================
// class SkeletalAppearanceTemplate: public static member functions
// ======================================================================

void SkeletalAppearanceTemplate::install()
{
	DEBUG_FATAL(ms_installed, ("SkeletalAppearanceTemplate already installed"));

	ExitChain::add(remove, "SkeletalAppearanceTemplate");
	AppearanceTemplateList::assignBinding(TAG_SMAT, load);

	ms_installed = true;
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("SkeletalAppearanceTemplate not installed"));

	AppearanceTemplateList::removeBinding(TAG_SMAT);

	ms_installed = false;
}

// ----------------------------------------------------------------------

AppearanceTemplate *SkeletalAppearanceTemplate::load(const char *name, Iff *iff)
{
	NOT_NULL(iff);
	return new SkeletalAppearanceTemplate(name, *iff);
}

// ======================================================================
// class SkeletalAppearanceTemplate: public member functions
// ======================================================================

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// engine run-time interface
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SkeletalAppearanceTemplate::~SkeletalAppearanceTemplate()
{
	{
		MeshGeneratorTemplateVector::iterator const end = m_meshGenerators.end();
		for (MeshGeneratorTemplateVector::iterator iter = m_meshGenerators.begin(); iter != end; ++iter)
			if (*iter)
				(*iter)->release();
	}

	{
		SkeletonTemplateVector::iterator const end = m_skeletonTemplates.end();
		for (SkeletonTemplateVector::iterator iter = m_skeletonTemplates.begin(); iter != end; ++iter)
			if (*iter)
				(*iter)->release();
	}

	{
		LatVector::iterator const endIt = m_latVector.end();
		for (LatVector::iterator it = m_latVector.begin(); it != endIt; ++it)
		{
			if (*it)
				(*it)->release();
		}
	}

	delete m_lodDistanceTable;
	delete m_sktToLatMap;
	delete m_skeletonTemplateInfo;
	delete m_meshGeneratorNames;
	delete m_animationStateGraphTemplateName;
}

// ----------------------------------------------------------------------

int SkeletalAppearanceTemplate::getMeshGeneratorCount() const
{
	NOT_NULL(m_meshGeneratorNames);
	return static_cast<int>(m_meshGeneratorNames->size());
}

// ----------------------------------------------------------------------

const CrcLowerString &SkeletalAppearanceTemplate::getMeshGeneratorName(int index) const
{
	NOT_NULL(m_meshGeneratorNames);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_meshGeneratorNames->size()));

	return (*m_meshGeneratorNames)[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

int SkeletalAppearanceTemplate::getSkeletonTemplateCount() const
{
	NOT_NULL(m_skeletonTemplateInfo);
	return static_cast<int>(m_skeletonTemplateInfo->size());
}

// ----------------------------------------------------------------------

const SkeletalAppearanceTemplate::SkeletonTemplateInfo &SkeletalAppearanceTemplate::getSkeletonTemplateInfo(int index) const
{
	NOT_NULL(m_skeletonTemplateInfo);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_skeletonTemplateInfo->size()));

	return (*m_skeletonTemplateInfo)[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

const CrcLowerString &SkeletalAppearanceTemplate::getSkeletonTemplateName(const SkeletonTemplateInfo &skeletonTemplateInfo) const
{
	return skeletonTemplateInfo.m_skeletonTemplateName;
}

// ----------------------------------------------------------------------

bool SkeletalAppearanceTemplate::skeletonTemplateHasAttachmentTransform(const SkeletonTemplateInfo &skeletonTemplateInfo) const
{
	return (strlen(skeletonTemplateInfo.m_skeletonTemplateName.getString()) > 0);
}

// ----------------------------------------------------------------------

const CrcLowerString &SkeletalAppearanceTemplate::getSkeletonTemplateAttachmentTransformName(const SkeletonTemplateInfo &skeletonTemplateInfo) const
{
	return skeletonTemplateInfo.m_attachmentTransformName;
}

// ----------------------------------------------------------------------

const CrcLowerString &SkeletalAppearanceTemplate::getAnimationStateGraphTemplateName() const
{
	if (m_animationStateGraphTemplateName)
		return *m_animationStateGraphTemplateName;
	else
		return CrcLowerString::empty;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the number of detail levels in the first skeleton template.
 *
 * This is useful if the caller needs to know the count of LODs and if the
 * caller assumes the root skeleton will dictate the count of LODs for
 * an object.
 *
 * Calling this function will fetch and release the root SkeletonTemplate.
 * This may cause the SkeletonTemplate to get loaded and immediately unloaded.
 * This could lead to poor performance if this function is called prior to
 * creating a Skeleton with the base SkeletonTemplate --- the SkeletonTemplate
 * may get loaded, unloaded, the loaded again in short succession.
 * 
 * @return  the number of detail levels in the first skeleton template.
 */

int SkeletalAppearanceTemplate::getRootSkeletonTemplateDetailCount() const
{
	if (m_skeletonTemplateInfo->empty())
		return 0;
	else
	{
		//-- fetch the SkeletonTemplate
		const SkeletonTemplate *const skeletonTemplate = SkeletonTemplateList::fetch(m_skeletonTemplateInfo->front().m_skeletonTemplateName);
		NOT_NULL(skeletonTemplate);

		//-- get detail count
		const int detailCount = skeletonTemplate->getDetailCount();

		//-- release SkeletonTemplate
		skeletonTemplate->release();

		return detailCount;
	}
}

// ----------------------------------------------------------------------
/**
 * Create all Skeleton LOD instances specified by this SkeletalAppearanceTemplate
 * instance.
 *
 * Create all Skeleton instances, one per LOD, according to the
 * SkeletonTemplate instances and attachment rules specified in this
 * SkeletalAppearanceTemplate instance.
 *
 * The order of Skeleton instances returned is with highest LOD at vector index 0,
 * and each lower detail level following in order.
 *
 * @param skeletons  this vector will be cleared, resized, and filled with
 *                   the created Skeleton instances upon return.
 */

void SkeletalAppearanceTemplate::createSkeletonLods(SkeletonVector &skeletons, TransformAnimationResolver &animationResolver) const
{
	//-- trivially reject routine if there's not skeleton templates
	if (m_skeletonTemplateInfo->empty())
	{
		skeletons.resize(0);
		return;
	}

	//-- retrieve all the SkeletonTemplate instances

	//   Implementation note: I don't store skeleton templates directly as pointers
	//   internally because the creation of SkeletalAppearanceTemplate instances uses
	//   a filename interface when adding and removing items.  Consider reworking this
	//   to simply this function.
	const size_t skeletonTemplateCount = m_skeletonTemplateInfo->size();
	std::vector<const SkeletonTemplate*>  skeletonTemplates(skeletonTemplateCount);

	for (size_t i = 0; i < skeletonTemplateCount; ++i)
	{
		skeletonTemplates[i] = SkeletonTemplateList::fetch((*m_skeletonTemplateInfo)[i].m_skeletonTemplateName);
		NOT_NULL(skeletonTemplates[i]);
	}

	//-- prepare the return array: size to # of LODs in first SkeletonTemplate.
	const int skeletonDetailCount = skeletonTemplates[0]->getDetailCount();
	DEBUG_FATAL(skeletonDetailCount < 1, ("skeleton template has not detail levels.\n"));

	skeletons.resize(static_cast<size_t>(skeletonDetailCount));

	//-- create each skeleton LOD
	for (int detailIndex = 0; detailIndex < skeletonDetailCount; ++detailIndex)
	{
		//-- build Skeleton instance for this LOD by creating the base skeleton and attaching all required attachments.
		for (size_t templateIndex = 0; templateIndex < skeletonTemplateCount; ++templateIndex)
		{
			//-- fetch the basic skeleton template associated with this LOD level
			const SkeletonTemplate *skeletonTemplate = skeletonTemplates[templateIndex];
			NOT_NULL(skeletonTemplate);

			// determine which LOD to use (I'll allow a constituent skeleton template to have fewer LODs than the root skeleton).
			const int detailIndexUsed = std::min(detailIndex, skeletonTemplate->getDetailCount() - 1);

			// fetch it
			const BasicSkeletonTemplate *const basicSkeletonTemplate = skeletonTemplate->fetchBasicSkeletonTemplate(detailIndexUsed);
			NOT_NULL(basicSkeletonTemplate);

			if (templateIndex == 0)
			{
				// create the skeleton with the root skeleton template
				skeletons[static_cast<size_t>(detailIndex)] = basicSkeletonTemplate->createSkeleton(animationResolver);
				NOT_NULL(skeletons[static_cast<size_t>(detailIndex)]);

				skeletons[static_cast<size_t>(detailIndex)]->beginSkeletonModification();
			}
			else
			{
				//-- attach skeleton template to the skeleton
				NOT_NULL(skeletons[static_cast<size_t>(detailIndex)]);
				IGNORE_RETURN(skeletons[static_cast<size_t>(detailIndex)]->attachSkeletonSegment(*basicSkeletonTemplate, (*m_skeletonTemplateInfo)[templateIndex].m_attachmentTransformName));
			}

			//-- release local reference to basicSkeletonTemplate
			basicSkeletonTemplate->release();
		}

		//-- finish up the skeleton modification
		NOT_NULL(skeletons[static_cast<size_t>(detailIndex)]);
		skeletons[static_cast<size_t>(detailIndex)]->endSkeletonModification();
	}

	//-- release the skeleton template instances
	std::for_each(skeletonTemplates.begin(), skeletonTemplates.end(), VoidMemberFunction(&SkeletonTemplate::release));
}

// ----------------------------------------------------------------------

const CrcLowerString &SkeletalAppearanceTemplate::lookupLatForSkeleton(CrcString const &skeletonPathName) const
{
	//-- Check if we have a local name map.
	if (m_sktToLatMap)
	{
		//-- Lookup skeleton file in local name map.
		const NameMap::const_iterator findIt = m_sktToLatMap->find(PersistentCrcString(skeletonPathName));
		if (findIt != m_sktToLatMap->end())
		{
			//-- Found a map in our local entry.
			return findIt->second;
		}
	}

	// @todo: lookup in global table.  For now, only handle local lookup table.
	return CrcLowerString::empty;
}

// ----------------------------------------------------------------------

bool SkeletalAppearanceTemplate::shouldCreateAnimationController() const
{
	return m_createAnimationController;
}

// ----------------------------------------------------------------------

TransformAnimationController *SkeletalAppearanceTemplate::createAnimationController(CrcString const &skeletonTemplateName, const TransformNameMap *layoutTransformNameMap, AnimationEnvironment &animationEnvironment, Appearance *ownerAppearance, int channel, AnimationStatePath const &initialPath) const
{
	DEBUG_WARNING(!shouldCreateAnimationController(), ("logic error: shouldCreateAnimationController() returned false, why is a controller being created?"));

	TransformAnimationController *controller = 0;

	//-- Handle StateGraphAnimationControllers.
	if (m_animationStateGraphTemplateName)
	{
		// Do nothing: StateGraphAnimationControllers are no longer supported.
		DEBUG_WARNING(true, ("SkeletalAppearanceTemplate: file [%s] wants a state graph animation controller, these are not supported.", getName()));
	}
	else
	{
		//-- Handle creation of StateHierarchyAnimationController associated with specified skt.
		const CrcLowerString &crcLatReferenceName = lookupLatForSkeleton(skeletonTemplateName);
		const char *const     cLatReferenceName = crcLatReferenceName.getString();
		if (cLatReferenceName && *cLatReferenceName)
		{
			// Found a Lat file to use.
			const LogicalAnimationTableTemplate *const latTemplate = LogicalAnimationTableTemplateList::fetch(crcLatReferenceName);
			if (!latTemplate)
				WARNING(true, ("failed to load .LAT file [%s] specified by .SAT file [%s] for skeleton [%s], no animations will play.", cLatReferenceName, getName(), skeletonTemplateName.getString()));
			else
			{
				// Create the controller from the lat file.
				controller = latTemplate->createAnimationController(animationEnvironment, *NON_NULL(layoutTransformNameMap), ownerAppearance, channel, initialPath);
				WARNING(!controller, (".LAT file [%s] failed to create controller.", cLatReferenceName));

				// Release local reference.
				latTemplate->release();
			}
		}
	}

	return controller;
}

// ----------------------------------------------------------------------

Appearance *SkeletalAppearanceTemplate::createAppearance() const
{
	if (!m_missingAssets)
	{
		//-- Create the appearance.
		return new SkeletalAppearance2(this);
	}
	else
	{
		//-- Create the default appearance.
		return AppearanceTemplateList::createAppearance(AppearanceTemplateList::getDefaultAppearanceTemplateName());
	}
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplate::preloadAssets() const
{
	preloadAssetsLight();

	//-- Preload mesh generators.  This will cause LMGs to load their MGNs.
	{
		MeshGeneratorTemplateVector::const_iterator endIt = m_meshGenerators.end();
		for (MeshGeneratorTemplateVector::const_iterator it = m_meshGenerators.begin(); it != endIt; ++it)
		{
			MeshGeneratorTemplate const *const mgTemplate = *it;
			if (mgTemplate)
				mgTemplate->preloadAssets();
		}
	}
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplate::preloadAssetsLight() const
{
	//-- Preload mesh generators, but only do it once.
	if (m_meshGenerators.empty() && !m_meshGeneratorNames->empty())
	{
		CrcLowerStringVector::const_iterator const endIt = m_meshGeneratorNames->end();
		for (CrcLowerStringVector::const_iterator it = m_meshGeneratorNames->begin(); it != endIt; ++it)
			m_meshGenerators.push_back(MeshGeneratorTemplateList::fetch(*it));
	}

	//-- Preload skeleton templates, but only do it once.
	if (m_skeletonTemplates.empty())
	{
		int const skeletonTemplateCount = getSkeletonTemplateCount();
		for (int i = 0; i < skeletonTemplateCount; ++i)
		{
			CrcString const &skeletonTemplateName = getSkeletonTemplateName(getSkeletonTemplateInfo(i));
			SkeletonTemplate const *skeletonTemplate = SkeletonTemplateList::fetch(skeletonTemplateName);
			m_skeletonTemplates.push_back(skeletonTemplate);
		}
	}

	//-- Preload .LAT files.
	if ((m_sktToLatMap != NULL) && m_latVector.empty())
	{
		NameMap::const_iterator const endIt = m_sktToLatMap->end();
		for (NameMap::const_iterator it = m_sktToLatMap->begin(); it != endIt; ++it)
			m_latVector.push_back(LogicalAnimationTableTemplateList::fetch(it->second));
	}
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplate::garbageCollect() const
{
	//-- Release preloaded mesh generator templates.
	{
		MeshGeneratorTemplateVector::iterator const endIt = m_meshGenerators.end();
		for (MeshGeneratorTemplateVector::iterator it = m_meshGenerators.begin(); it != endIt; ++it)
		{
			if (*it)
			{
				//-- Tell LMG to throw out MGNs but don't throw the LMG out.  They're really light weight
				//   and are not worth hitting the disk for again later.
				(*it)->garbageCollect();
			}
		}
	}

	//-- Release preloaded skeleton templates.
	{
		SkeletonTemplateVector::iterator const endIt = m_skeletonTemplates.end();
		for (SkeletonTemplateVector::iterator it = m_skeletonTemplates.begin(); it != endIt; ++it)
		{
			if (*it)
				(*it)->release();
		}

		m_skeletonTemplates.clear();
	}

	//-- Release preloaded LATs.
	{
		LatVector::iterator const endIt = m_latVector.end();
		for (LatVector::iterator it = m_latVector.begin(); it != endIt; ++it)
		{
			if (*it)
				(*it)->release();
		}

		m_latVector.clear();
	}

}

// ----------------------------------------------------------------------

bool SkeletalAppearanceTemplate::hasAsgController() const
{
	//-- Appearance will have an asg animation controller if there's a name specified for it.
	return (m_animationStateGraphTemplateName && *m_animationStateGraphTemplateName->getString());
}

// ----------------------------------------------------------------------

bool SkeletalAppearanceTemplate::hasAshController() const
{
	//-- Appearance will have an ash controller if there isn't an animation state graph template name
	//   and if there are any .skt -> .lat entries.
	return (!m_animationStateGraphTemplateName && (m_sktToLatMap && !m_sktToLatMap->empty()));
}

// ----------------------------------------------------------------------

bool SkeletalAppearanceTemplate::mustUseSoftSkinning() const
{
	return m_mustUseSoftSkinning;
}

// ----------------------------------------------------------------------

bool SkeletalAppearanceTemplate::getAlwaysPlayActionGeneratorAnimations() const
{
	return m_alwaysPlayActionGeneratorAnimations;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// tools construction interface
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SkeletalAppearanceTemplate::SkeletalAppearanceTemplate() :
	AppearanceTemplate(""),
	m_meshGeneratorNames(new CrcLowerStringVector()),
	m_skeletonTemplateInfo(new SkeletonTemplateInfoVector()),
	m_createAnimationController(false),
	m_animationStateGraphTemplateName(0),
	m_sktToLatMap(0),
	m_lodDistanceTable(0),
	m_meshGenerators(),
	m_skeletonTemplates(),
	m_latVector(),
	m_missingAssets(false),
	m_mustUseSoftSkinning(false),
	m_alwaysPlayActionGeneratorAnimations(false)
{
}

// ----------------------------------------------------------------------

int SkeletalAppearanceTemplate::addMeshGenerator(const char *meshGeneratorName)
{
	NOT_NULL(m_meshGeneratorNames);

	//-- Add MeshGeneratorTemplate name to list.
	const int newIndex = static_cast<int>(m_meshGeneratorNames->size());
	m_meshGeneratorNames->push_back(CrcLowerString(meshGeneratorName));

	//-- Fetch a reference to it if I can find the file.
	if (TreeFile::exists(meshGeneratorName))
	{
		const MeshGeneratorTemplate *mgTemplate = MeshGeneratorTemplateList::fetch(CrcLowerString(meshGeneratorName));
		if (mgTemplate)
			m_meshGenerators.push_back(mgTemplate);
	}

	return newIndex;
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplate::removeMeshGenerator(int index)
{
	NOT_NULL(m_meshGeneratorNames);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_meshGeneratorNames->size()));

	CrcLowerStringVector::iterator it = m_meshGeneratorNames->begin();
	it += static_cast<size_t>(index);

	IGNORE_RETURN(m_meshGeneratorNames->erase(it));

	// @todo remove the fetched MeshGeneratorTemplate from m_meshGenerators.
}

// ----------------------------------------------------------------------

int SkeletalAppearanceTemplate::addSkeletonTemplate(const char *skeletonTemplateName, const char *attachmentTransformName)
{
	NOT_NULL(m_skeletonTemplateInfo);

	//-- Add SkeletonTemplate name to list.
	const int newIndex = static_cast<int>(m_skeletonTemplateInfo->size());
	m_skeletonTemplateInfo->push_back(SkeletonTemplateInfo(skeletonTemplateName, attachmentTransformName));

	//-- Fetch a reference to it if I can find the file.
	if (TreeFile::exists(skeletonTemplateName))
	{
		const SkeletonTemplate *sTemplate = SkeletonTemplateList::fetch(CrcLowerString(skeletonTemplateName));
		if (sTemplate)
			m_skeletonTemplates.push_back(sTemplate);
	}

	return newIndex;
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplate::removeSkeletonTemplate(int index)
{
	NOT_NULL(m_skeletonTemplateInfo);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_skeletonTemplateInfo->size()));

	SkeletonTemplateInfoVector::iterator it = m_skeletonTemplateInfo->begin();
	it += static_cast<size_t>(index);

	IGNORE_RETURN(m_skeletonTemplateInfo->erase(it));

	// @todo remove the fetched SkeletonTemplate from m_skeletonTemplates.
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplate::setCreateAnimationController(bool createController)
{
	m_createAnimationController = createController;
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplate::setSktToLatMapping(const CrcLowerString &sktReferenceName, const CrcLowerString &latReferenceName)
{
	//-- Create map as necessary.
	if (!m_sktToLatMap)
		m_sktToLatMap = new NameMap;

	//-- Find entry if already exists.
	const NameMap::iterator lowerBoundResult = m_sktToLatMap->lower_bound(sktReferenceName);
	if ((lowerBoundResult != m_sktToLatMap->end()) && !m_sktToLatMap->key_comp()(sktReferenceName, lowerBoundResult->first))
	{
		// Entry already exists, replace value.
		if (*latReferenceName.getString())
		{
			// Caller specified a real mapping entry.
			lowerBoundResult->second = latReferenceName;
		}
		else
		{
			// Caller specified no mapping entry (zero-length string), delete entry.
			m_sktToLatMap->erase(lowerBoundResult);
		}
	}
	else
	{
		// Entry doesn't exist.  Add one as necessary.
		if (*latReferenceName.getString())
		{
			// Caller specified a real entry, add it.
			IGNORE_RETURN(m_sktToLatMap->insert(lowerBoundResult, NameMap::value_type(sktReferenceName, latReferenceName)));
		}
	}

	//-- When the caller specifies mappings, it is only logical that an animation controller is expected, so force this.
	setCreateAnimationController(true);
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplate::setAnimationStateGraphTemplateName(const CrcLowerString &pathName)
{
	if (m_animationStateGraphTemplateName)
		*m_animationStateGraphTemplateName = pathName;
	else
		m_animationStateGraphTemplateName = new CrcLowerString(pathName);

	//-- When the caller specifies an animation controller, clearly you want an animation controller, so force this.
	setCreateAnimationController(true);
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplate::write(Iff &iff) const
{
	if (m_animationStateGraphTemplateName && *m_animationStateGraphTemplateName->getString())
	{
		DEBUG_FATAL(m_sktToLatMap, ("programmer error: nothing should be trying to write both .ash data and .skt/.lat mappings --- .asgs are going away."));

		// @todo remove this code when MayaExporter is updated.

		//-- This is a version-2 .sat file, written by tool that doesn't yet know about
		//   the switch to AnimationStateHierarchyTemplate.
		write_0002(iff);
	}
	else
		write_0003(iff);
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

bool SkeletalAppearanceTemplate::renameMgnToLmg()
{
	bool valueChanged = false;
	char fixupName[2 * MAX_PATH];

	NOT_NULL(m_meshGeneratorNames);
	CrcLowerStringVector::iterator const endIt = m_meshGeneratorNames->end();
	for (CrcLowerStringVector::iterator it = m_meshGeneratorNames->begin(); it != endIt; ++it)
	{
		char const *const originalName = it->getString();
		if (originalName)
		{
			strcpy(fixupName, originalName);
			char *const extensionStart = strrchr(fixupName, '.');
			if ((extensionStart) && (_stricmp(extensionStart, ".mgn") == 0))
			{
				strcpy(extensionStart + 1, "lmg");
				if (TreeFile::exists(fixupName) || !TreeFile::exists(originalName))
				{
					valueChanged = true;
					*it = CrcLowerString(fixupName);
				}
				else
				{
					DEBUG_REPORT_LOG(true, ("SAT [%s]: no fixup needed, LMG [%s] doesn't exist.\n", getName(), fixupName));
				}
			}
		}
	}

	return valueChanged;
}

#endif

// ======================================================================

SkeletalAppearanceTemplate::SkeletalAppearanceTemplate(const char *name, Iff &iff) :
	AppearanceTemplate(name),
	m_meshGeneratorNames(new CrcLowerStringVector()),
	m_skeletonTemplateInfo(new SkeletonTemplateInfoVector()),
	m_createAnimationController(false),
	m_animationStateGraphTemplateName(0),
	m_sktToLatMap(0),
	m_lodDistanceTable(0),
	m_meshGenerators(),
	m_skeletonTemplates(),
	m_latVector(),
	m_missingAssets(false),
	m_mustUseSoftSkinning(false),
	m_alwaysPlayActionGeneratorAnimations(false)
{
	iff.enterForm(TAG_SMAT);
	{
		switch (iff.getCurrentName())
		{
			case TAG_0001:
				load_0001(iff);

				// too many sat files for clothing exist where an animation controller never will be specified and so don't need an update.  ignore this warning for now.
#if 0
				WARNING(true, ("** old-version SAT file [%s]: no animation state graph -- expect animation issues", name ? name : "<no name available>"));
#endif
				break;

			case TAG_0002:
				load_0002(iff);
				break;

			case TAG_0003:
				load_0003(iff);
				break;

			default:
				{
					char tagName[5];

					ConvertTagToString(iff.getCurrentName(), tagName);
					FATAL(true, ("unsupported SkeletalAppearanceTemplate version [%s]", tagName));
				}
		}
	}
	iff.exitForm(TAG_SMAT);

	//-- Perform missing asset check.
	m_missingAssets = hasMissingAssets();
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplate::load_0001(Iff &iff)
{
	int meshGeneratorCount;
	int skeletonTemplateCount;

	m_createAnimationController = false;

	iff.enterForm(TAG_0001);
	{
		//-- load general info
		iff.enterChunk(TAG_INFO);
		{
			// get counts
			meshGeneratorCount    = iff.read_int32();
			skeletonTemplateCount = iff.read_int32();

			// load skeletal appearance instance filename (ignore it, no longer needed)
			char filename[MAX_PATH];
			iff.read_string(filename, sizeof(filename) - 1);

			// validate counts
			DEBUG_FATAL(meshGeneratorCount < 0, ("invalid meshGeneratorCount [%d]", meshGeneratorCount));
			DEBUG_FATAL(skeletonTemplateCount < 0, ("invalid skeletonTemplateCount [%d]", skeletonTemplateCount));
		}
		iff.exitChunk(TAG_INFO);

		//-- load mesh generator data
		iff.enterChunk(TAG_MSGN);
		{
			m_meshGeneratorNames->reserve(static_cast<size_t>(meshGeneratorCount));

			for (int i = 0; i < meshGeneratorCount; ++i)
			{
				char name[MAX_PATH];

				iff.read_string(name, sizeof(name)-1);
				m_meshGeneratorNames->push_back(CrcLowerString(name));
			}
		}
		iff.exitChunk(TAG_MSGN);

		//-- load skeleton template info data
		iff.enterChunk(TAG_SKTI);
		{
			m_skeletonTemplateInfo->reserve(static_cast<size_t>(skeletonTemplateCount));

			for (int i = 0; i < skeletonTemplateCount; ++i)
			{
				char skeletonTemplateName[MAX_PATH];
				char attachmentTransformName[MAX_PATH];

				iff.read_string(skeletonTemplateName, sizeof(skeletonTemplateName)-1);
				iff.read_string(attachmentTransformName, sizeof(attachmentTransformName)-1);

				m_skeletonTemplateInfo->push_back(SkeletonTemplateInfo(skeletonTemplateName, attachmentTransformName));
			}
		}
		iff.exitChunk(TAG_SKTI);
	}
	iff.exitForm(TAG_0001);
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplate::load_0002(Iff &iff)
{
	int meshGeneratorCount;
	int skeletonTemplateCount;

	iff.enterForm(TAG_0002);
	{
		//-- load general info
		iff.enterChunk(TAG_INFO);
		{
			// get counts
			meshGeneratorCount    = iff.read_int32();
			skeletonTemplateCount = iff.read_int32();

			// load AnimationStateGraphTemplate pathname
			char pathname[MAX_PATH];

			iff.read_string(pathname, sizeof(pathname) - 1);
			if (pathname[0])
			{
				if (!m_animationStateGraphTemplateName)
					m_animationStateGraphTemplateName = new CrcLowerString(pathname);
				else
					m_animationStateGraphTemplateName->setString(pathname);

				// Indicate this template should create animation controllers for the associated .skts.
				m_createAnimationController = true;
			}
			
			// validate counts
			DEBUG_FATAL(meshGeneratorCount < 0, ("invalid meshGeneratorCount [%d]", meshGeneratorCount));
			DEBUG_FATAL(skeletonTemplateCount < 0, ("invalid skeletonTemplateCount [%d]", skeletonTemplateCount));
		}
		iff.exitChunk(TAG_INFO);

		//-- load mesh generator data
		iff.enterChunk(TAG_MSGN);
		{
			m_meshGeneratorNames->reserve(static_cast<size_t>(meshGeneratorCount));
			m_meshGenerators.reserve(static_cast<MeshGeneratorTemplateVector::size_type>(meshGeneratorCount));

			for (int i = 0; i < meshGeneratorCount; ++i)
			{
				char name[MAX_PATH];

				iff.read_string(name, sizeof(name)-1);

				//-- Track MeshGeneratorName.
				CrcLowerString pathName(name);
				m_meshGeneratorNames->push_back(pathName);

				//-- Fetch reference to MeshGeneratorTemplate.
				if (TreeFile::exists(name))
				{
					const MeshGeneratorTemplate *const mgTemplate = MeshGeneratorTemplateList::fetch(pathName);
					if (mgTemplate)
						m_meshGenerators.push_back(mgTemplate);
				}
			}
		}
		iff.exitChunk(TAG_MSGN);

		//-- load skeleton template info data
		iff.enterChunk(TAG_SKTI);
		{
			m_skeletonTemplateInfo->reserve(static_cast<size_t>(skeletonTemplateCount));
			m_skeletonTemplates.reserve(static_cast<SkeletonTemplateVector::size_type>(skeletonTemplateCount));

			for (int i = 0; i < skeletonTemplateCount; ++i)
			{
				char skeletonTemplateName[MAX_PATH];
				char attachmentTransformName[MAX_PATH];

				iff.read_string(skeletonTemplateName, sizeof(skeletonTemplateName)-1);
				iff.read_string(attachmentTransformName, sizeof(attachmentTransformName)-1);

				//-- Track SkeletonTemplate info.
				m_skeletonTemplateInfo->push_back(SkeletonTemplateInfo(skeletonTemplateName, attachmentTransformName));

				//-- Fetch reference to SkeletonTemplate.
				if (TreeFile::exists(skeletonTemplateName))
				{
					const SkeletonTemplate *const sTemplate = SkeletonTemplateList::fetch(CrcLowerString(skeletonTemplateName));
					if (sTemplate)
						m_skeletonTemplates.push_back(sTemplate);
				}
			}
		}
		iff.exitChunk(TAG_SKTI);
	}
	iff.exitForm(TAG_0002);
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplate::load_0003(Iff &iff)
{
	int meshGeneratorCount;
	int skeletonTemplateCount;

	iff.enterForm(TAG_0003);
	{
		//-- load general info
		iff.enterChunk(TAG_INFO);
		{
			meshGeneratorCount    = iff.read_int32();
			DEBUG_FATAL(meshGeneratorCount < 0, ("invalid meshGeneratorCount [%d]", meshGeneratorCount));

			skeletonTemplateCount = iff.read_int32();
			DEBUG_FATAL(skeletonTemplateCount < 0, ("invalid skeletonTemplateCount [%d]", skeletonTemplateCount));

			m_createAnimationController = (iff.read_int8() != 0);
		}
		iff.exitChunk(TAG_INFO);

		//-- load mesh generator data
		iff.enterChunk(TAG_MSGN);
		{			
			m_meshGeneratorNames->reserve(static_cast<size_t>(meshGeneratorCount));
			m_meshGenerators.reserve(static_cast<MeshGeneratorTemplateVector::size_type>(meshGeneratorCount));

			for (int i = 0; i < meshGeneratorCount; ++i)
			{
				char name[MAX_PATH];
				iff.read_string(name, sizeof(name)-1);

				// Convert all forward slashes to back slashes.
				const int nameLength = static_cast<int>(strlen(name));
				for (int j = 0; j < nameLength; ++j)
				{
					if (name[j] == '/')
						name[j] = '\\';
				}

				//-- Track MeshGeneratorName.
				CrcLowerString pathName(name);
				m_meshGeneratorNames->push_back(pathName);

				//-- Fetch reference to MeshGeneratorTemplate.
				if (TreeFile::exists(name))
				{
					const MeshGeneratorTemplate *const mgTemplate = MeshGeneratorTemplateList::fetch(pathName);
					if (mgTemplate)
						m_meshGenerators.push_back(mgTemplate);
				}
			}
		}
		iff.exitChunk(TAG_MSGN);

		//-- load skeleton template info data
		iff.enterChunk(TAG_SKTI);
		{
			m_skeletonTemplateInfo->reserve(static_cast<size_t>(skeletonTemplateCount));
			m_skeletonTemplates.reserve(static_cast<SkeletonTemplateVector::size_type>(skeletonTemplateCount));

			for (int i = 0; i < skeletonTemplateCount; ++i)
			{
				char skeletonTemplateName[MAX_PATH];
				char attachmentTransformName[MAX_PATH];

				iff.read_string(skeletonTemplateName, sizeof(skeletonTemplateName)-1);
				iff.read_string(attachmentTransformName, sizeof(attachmentTransformName)-1);

				// Convert all forward slashes to back slashes.
				const int nameLength = static_cast<int>(strlen(skeletonTemplateName));
				for (int j = 0; j < nameLength; ++j)
				{
					if (skeletonTemplateName[j] == '/')
						skeletonTemplateName[j] = '\\';
				}

				//-- Track SkeletonTemplate info.
				m_skeletonTemplateInfo->push_back(SkeletonTemplateInfo(skeletonTemplateName, attachmentTransformName));

				//-- Fetch reference to SkeletonTemplate.
				if (TreeFile::exists(skeletonTemplateName))
				{
					const SkeletonTemplate *const sTemplate = SkeletonTemplateList::fetch(CrcLowerString(skeletonTemplateName));
					if (sTemplate)
						m_skeletonTemplates.push_back(sTemplate);
				}
			}
		}
		iff.exitChunk(TAG_SKTI);

		if (iff.enterChunk(TAG_LATX, true))
		{
			// Get # entries.
			const int count = static_cast<int>(iff.read_int16());

			DEBUG_FATAL(count < 0, ("bad lat -> skt entry count [%d].", count));
			DEBUG_WARNING(count == 0, ("LATX chunk written with zero entries."));

			// Load entries.
			if (m_sktToLatMap)
				m_sktToLatMap->clear();
			else
				m_sktToLatMap = new NameMap;

			char key[1024];
			char value[1024];

			for (int i = 0; i < count; ++i)
			{
				iff.read_string(key, sizeof(key) - 1);
				iff.read_string(value, sizeof(value) - 1);

				// Convert all forward slashes to back slashes.
				{
					const int nameLength = static_cast<int>(strlen(key));
					for (int j = 0; j < nameLength; ++j)
					{
						if (key[j] == '/')
							key[j] = '\\';
					}
				}

				{
					const int nameLength = static_cast<int>(strlen(value));
					for (int j = 0; j < nameLength; ++j)
					{
						if (value[j] == '/')
							value[j] = '\\';
					}
				}

				IGNORE_RETURN(m_sktToLatMap->insert(NameMap::value_type(PersistentCrcString(key, true), CrcLowerString(value))));
			}

			iff.exitChunk(TAG_LATX);
		}

		//-- Handle LOD Distance Table.
		if (!iff.atEndOfForm() && (iff.getCurrentName() == TAG_LDTB))
			m_lodDistanceTable = new LodDistanceTable(iff);

		if (iff.enterChunk(TAG_SFSK, true))
		{
			m_mustUseSoftSkinning = (iff.read_uint8() != 0);
			iff.exitChunk(TAG_SFSK);
		}

		if (iff.enterChunk(TAG_APAG, true))
		{
			m_alwaysPlayActionGeneratorAnimations = (iff.read_uint8() != 0);
			iff.exitChunk(TAG_APAG);
		}
	}
	iff.exitForm(TAG_0003);
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplate::write_0002(Iff &iff) const
{
	NOT_NULL(m_meshGeneratorNames);
	NOT_NULL(m_skeletonTemplateInfo);

	iff.insertForm(TAG_SMAT);
		iff.insertForm(TAG_0002);

			iff.insertChunk(TAG_INFO);
			{
				iff.insertChunkData(static_cast<int32>(m_meshGeneratorNames->size()));
				iff.insertChunkData(static_cast<int32>(m_skeletonTemplateInfo->size()));
				iff.insertChunkString(getAnimationStateGraphTemplateName().getString());
			}
			iff.exitChunk(TAG_INFO);

			iff.insertChunk(TAG_MSGN);
			{
				for (size_t i = 0; i < m_meshGeneratorNames->size(); ++i)
				{
					const CrcLowerString &meshGeneratorName = (*m_meshGeneratorNames)[i];
					iff.insertChunkString(meshGeneratorName.getString());
				}
			}
			iff.exitChunk(TAG_MSGN);

			iff.insertChunk(TAG_SKTI);
			{
				for (size_t i = 0; i < m_skeletonTemplateInfo->size(); ++i)
				{
					const SkeletonTemplateInfo &skeletonTemplateInfo = (*m_skeletonTemplateInfo)[i];
					iff.insertChunkString(skeletonTemplateInfo.m_skeletonTemplateName.getString());
					iff.insertChunkString(skeletonTemplateInfo.m_attachmentTransformName.getString());
				}
			}
			iff.exitChunk(TAG_SKTI);

		iff.exitForm(TAG_0002);
	iff.exitForm(TAG_SMAT);
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplate::write_0003(Iff &iff) const
{
	NOT_NULL(m_meshGeneratorNames);
	NOT_NULL(m_skeletonTemplateInfo);

	iff.insertForm(TAG_SMAT);
		iff.insertForm(TAG_0003);

			iff.insertChunk(TAG_INFO);
			{
				iff.insertChunkData(static_cast<int32>(m_meshGeneratorNames->size()));
				iff.insertChunkData(static_cast<int32>(m_skeletonTemplateInfo->size()));
				iff.insertChunkData(static_cast<int8>(m_createAnimationController ? 1 : 0));
			}
			iff.exitChunk(TAG_INFO);

			iff.insertChunk(TAG_MSGN);
			{
				for (size_t i = 0; i < m_meshGeneratorNames->size(); ++i)
				{
					const CrcLowerString &meshGeneratorName = (*m_meshGeneratorNames)[i];
					iff.insertChunkString(meshGeneratorName.getString());
				}
			}
			iff.exitChunk(TAG_MSGN);

			iff.insertChunk(TAG_SKTI);
			{
				for (size_t i = 0; i < m_skeletonTemplateInfo->size(); ++i)
				{
					const SkeletonTemplateInfo &skeletonTemplateInfo = (*m_skeletonTemplateInfo)[i];
					iff.insertChunkString(skeletonTemplateInfo.m_skeletonTemplateName.getString());
					iff.insertChunkString(skeletonTemplateInfo.m_attachmentTransformName.getString());
				}
			}
			iff.exitChunk(TAG_SKTI);

			if (m_sktToLatMap && !m_sktToLatMap->empty())
			{
				iff.insertChunk(TAG_LATX);

					iff.insertChunkData(static_cast<int16>(m_sktToLatMap->size()));

					const NameMap::const_iterator endIt = m_sktToLatMap->end();
					for (NameMap::const_iterator it = m_sktToLatMap->begin(); it != endIt; ++it)
					{
						iff.insertChunkString(it->first.getString());
						iff.insertChunkString(it->second.getString());
					}

				iff.exitChunk(TAG_LATX);
			}

			//-- Handle writing LodDistanceTable.
			if (m_lodDistanceTable)
				m_lodDistanceTable->write(iff);

		iff.exitForm(TAG_0003);
	iff.exitForm(TAG_SMAT);
}

// ----------------------------------------------------------------------
/**
 * Indicate whether any assets (skeletons or mesh generators) are
 * missing from disk.
 *
 * @return  true if any .skt or .msh files are not on disk; false otherwise.
 */

bool SkeletalAppearanceTemplate::hasMissingAssets() const	
{
	//-- Check skeleton templates.
	const SkeletonTemplateInfoVector::const_iterator skelEndIt = m_skeletonTemplateInfo->end();
	for (SkeletonTemplateInfoVector::const_iterator skelIt = m_skeletonTemplateInfo->begin(); skelIt != skelEndIt; ++skelIt)
	{
		//-- Check if skeleton template exists.
		const char *const pathName = skelIt->m_skeletonTemplateName.getString();

		if (!TreeFile::exists(pathName))
		{
			WARNING(true, ("SkeletalAppearanceTemplate [%s] has non-existing skeleton [%s], will generate default skeletal appearance.", getName(), pathName));
			return true;
		}
	}

	//-- Check mesh generator templates.
	const CrcLowerStringVector::const_iterator meshEndIt = m_meshGeneratorNames->end();
	for (CrcLowerStringVector::const_iterator meshIt = m_meshGeneratorNames->begin(); meshIt != meshEndIt; ++meshIt)
	{
		//-- Check if skeleton template exists.
		const char *const pathName = meshIt->getString();

		if (!TreeFile::exists(pathName))
		{
			WARNING(true, ("SkeletalAppearanceTemplate [%s] has non-existing mesh [%s], will generate default skeletal appearance.", getName(), pathName));
			return true;
		}
	}

	//-- No missing assets.
	return false;
}

// ======================================================================
