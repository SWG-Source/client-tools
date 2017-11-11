// ======================================================================
//
// SkeletonTemplateList.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletonTemplateList.h"

#include "clientSkeletalAnimation/SkeletonTemplate.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"

#include <algorithm>
#include <map>
#include <vector>

// ======================================================================

bool                                     SkeletonTemplateList::ms_installed;

SkeletonTemplateList::CreateFunctionMap *SkeletonTemplateList::ms_createFunctionMap;
SkeletonTemplateList::TemplateMap       *SkeletonTemplateList::ms_templateMap;

// ======================================================================

void SkeletonTemplateList::install()
{
	DEBUG_FATAL(ms_installed, ("SkeletonTemplateList already installed"));

	ms_createFunctionMap = new CreateFunctionMap();
	ms_templateMap       = new TemplateMap();

	ms_installed = true;
	ExitChain::add(SkeletonTemplateList::remove, "SkeletonTemplateList");
}

// ----------------------------------------------------------------------

void SkeletonTemplateList::registerCreateFunction(Tag tag, CreateFunction function)
{
	DEBUG_FATAL(!ms_installed, ("SkeletonTemplateList not installed"));

	//-- check if create function has been registered previously
	CreateFunctionMap::iterator lowerBoundResult = ms_createFunctionMap->lower_bound(tag);
	if ((lowerBoundResult != ms_createFunctionMap->end()) && !ms_createFunctionMap->key_comp()(tag, lowerBoundResult->first))
	{
		//-- report error, already registered
		char buffer[5];

		ConvertTagToString(tag, buffer);
		DEBUG_FATAL(true, ("attempting to register skeleton template [%s] multiple times.\n", buffer));
		return; //lint !e527 // Unreachable // Reachable in release.
	}

	//-- register create function
	IGNORE_RETURN(ms_createFunctionMap->insert(lowerBoundResult, CreateFunctionMap::value_type(tag, function)));
}

// ----------------------------------------------------------------------

void SkeletonTemplateList::deregisterCreateFunction(Tag tag)
{
	DEBUG_FATAL(!ms_installed, ("SkeletonTemplateList not installed"));

	//-- find create function map entry
	CreateFunctionMap::iterator findIt = ms_createFunctionMap->find(tag);
	if (findIt == ms_createFunctionMap->end())
	{
		char buffer[5];

		ConvertTagToString(tag, buffer);
		DEBUG_FATAL(true, ("attempting to deregister skeleton template [%s] but isn't registered.\n", buffer));
		return; //lint !e527 // Unreachable // Reachable in release.
	}

	ms_createFunctionMap->erase(findIt);
}

// ----------------------------------------------------------------------

const SkeletonTemplate *SkeletonTemplateList::fetch(CrcString const &pathName)
{
	DEBUG_FATAL(!ms_installed, ("SkeletonTemplateList not installed"));

	// check if skeletonTemplate has already been created
	SkeletonTemplate *skeletonTemplate;

	TemplateMap::iterator lowerBoundResult = ms_templateMap->lower_bound(&pathName);
	if ((lowerBoundResult != ms_templateMap->end()) && !ms_templateMap->key_comp()(&pathName, lowerBoundResult->first))
	{
		//-- found it
		skeletonTemplate = NON_NULL(lowerBoundResult->second);
	}
	else
	{
		//-- load up the file as an iff
		Iff iff(pathName.getString());

		//-- find create function to use based on first tag name
		const Tag currentName              = iff.getCurrentName();
		CreateFunctionMap::iterator findIt = ms_createFunctionMap->find(currentName);
		if (findIt == ms_createFunctionMap->end())
		{
			char buffer[5];

			ConvertTagToString(currentName, buffer);
			DEBUG_FATAL(true, ("no SkeletonTemplate registered for handling [%s].\n", buffer));
			return 0; //lint !e527 // Unreachable // Reachable in release.
		}

		//-- create the skeleton template from the iff
		CreateFunction createFunction = findIt->second;
		NOT_NULL(createFunction);

		skeletonTemplate = (*createFunction)(iff, pathName);
		NOT_NULL(skeletonTemplate);

		// add it to the named template list
		IGNORE_RETURN(ms_templateMap->insert(lowerBoundResult, TemplateMap::value_type(&skeletonTemplate->getName(), skeletonTemplate)));
	}

	// bump up the ref count
	skeletonTemplate->fetch();

	return skeletonTemplate;
}

// ----------------------------------------------------------------------

const SkeletonTemplate *SkeletonTemplateList::fetch(Iff &iff)
{
	DEBUG_FATAL(!ms_installed, ("SkeletonTemplateList not installed"));

	//-- First check for a named chunk (a chunk called NAME that contains a filename for the real data).
	const Tag currentName = iff.getCurrentName();
	if (currentName == TAG_NAME)
	{
		char pathName[1024];

		iff.enterChunk(TAG_NAME);
			iff.read_string(pathName, sizeof(pathName) - 1);
		iff.exitChunk(TAG_NAME);

		return fetch(CrcLowerString(pathName));
	}

	//-- Find create function to use based on first tag name.
	CreateFunctionMap::iterator findIt = ms_createFunctionMap->find(currentName);
	if (findIt == ms_createFunctionMap->end())
	{
		char buffer[5];

		ConvertTagToString(currentName, buffer);
		DEBUG_FATAL(true, ("no SkeletonTemplate registered for handling [%s].\n", buffer));
		return 0; //lint !e527 // Unreachable // Reachable in release.
	}

	//-- create the skeleton template from the iff
	CreateFunction createFunction = findIt->second;
	NOT_NULL(createFunction);

	SkeletonTemplate *const skeletonTemplate = (*createFunction)(iff, CrcLowerString::empty);
	NOT_NULL(skeletonTemplate);

	//-- bump up ref count
	skeletonTemplate->fetch();

	return skeletonTemplate;
}

// ======================================================================

void SkeletonTemplateList::remove()
{
	DEBUG_FATAL(!ms_installed, ("SkeletonTemplateList not installed"));

	// cleanup named templates
	{
		DEBUG_WARNING(!ms_templateMap->empty(), ("SkeletonTemplateList: %u named SkeletonTemplate objects not released", ms_templateMap->size()));

		const TemplateMap::iterator endIt = ms_templateMap->end();
		for (TemplateMap::iterator it = ms_templateMap->begin(); it != endIt; ++it)
		{
			CrcString const *name = NON_NULL(it->first);

			DEBUG_WARNING(true, ("-- SkeletonTemplate [%s] not released", name->getString()));
			UNREF(name);

			//-- should not delete the SkeletonTemplate here because it likely is uninstalled at this time.
		}

		delete ms_templateMap;
		ms_templateMap = 0;
	}

	delete ms_createFunctionMap;
	ms_createFunctionMap = 0;

	ms_installed = false;
}

// ----------------------------------------------------------------------

void SkeletonTemplateList::stopTracking(const SkeletonTemplate &skeletonTemplate)
{
	DEBUG_FATAL(!ms_installed, ("SkeletonTemplateList not installed"));

	CrcString const   &name  = skeletonTemplate.getName();
	const char *const  cName = name.getString();

	if (!cName || !*cName)
	{
		// This is not a named SkeletonTemplate.  Unnamed instances are not tracked.
		return;
	}

	//-- find skeleton template in one of the template lists, remove it
	TemplateMap::iterator findIt = ms_templateMap->find(&name);
	if (findIt != ms_templateMap->end())
	{
		// erase the entry
		ms_templateMap->erase(findIt);
		return;
	}

	// failed to find named SkeletonTemplate
	DEBUG_WARNING(true, ("stopTracking() failed to find named SkeletonTemplate [%s][0x%08x]", cName, &skeletonTemplate));
}

// ======================================================================
