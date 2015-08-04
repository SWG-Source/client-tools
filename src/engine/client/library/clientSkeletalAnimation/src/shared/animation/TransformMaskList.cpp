// ======================================================================
//
// TransformMaskList.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/TransformMaskList.h"

#include "clientSkeletalAnimation/TransformMask.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"

#include <map>

// ======================================================================

bool                                 TransformMaskList::ms_installed;

TransformMaskList::NamedTemplateMap *TransformMaskList::ms_templates;
TransformMask                       *TransformMaskList::ms_defaultTemplate;

// ======================================================================
// class TransformMaskList: public static member functions
// ======================================================================

void TransformMaskList::install()
{
	DEBUG_FATAL(ms_installed, ("TransformMaskList already installed."));

	ms_templates       = new NamedTemplateMap();

	ms_defaultTemplate = new TransformMask();
	ms_defaultTemplate->fetch();

	ms_installed = true;
	ExitChain::add(remove, "TransformMaskList");
}

// ----------------------------------------------------------------------

const TransformMask *TransformMaskList::fetch(const CrcLowerString &pathName)
{
	DEBUG_FATAL(!ms_installed, ("TransformMaskList not installed."));

	TransformMask *transformMask = 0;

	//-- Searched loaded named templates for match.
	const NamedTemplateMap::iterator lowerBoundResult = ms_templates->lower_bound(&pathName);
	if ((lowerBoundResult != ms_templates->end()) && !ms_templates->key_comp()(&pathName, lowerBoundResult->first))
	{
		//-- Found it, the TransformMask is already loaded.
		transformMask = lowerBoundResult->second;
	}
	else
	{
		//-- Need to load the TransformMask.
		if (TreeFile::exists(pathName.getString()))
		{
			// File exists, load it.
			Iff iff(pathName.getString());
			transformMask = new TransformMask(pathName.getString(), iff);

			//-- Add transform mask to the list.
			IGNORE_RETURN(ms_templates->insert(lowerBoundResult, NamedTemplateMap::value_type(&(transformMask->getName()), transformMask)));
		}
		else
		{
			// File doesn't exist, return the default.
			DEBUG_WARNING(true, ("requested TransformMask file [%s] not found, using default.", pathName.getString()));
			transformMask = ms_defaultTemplate;
		}
	}

	//-- Fetch a reference to the TransformMask instance for the caller.
	NOT_NULL(transformMask);
	transformMask->fetch();

	return transformMask;
}

// ======================================================================
// class TransformMaskList: private static member functions
// ======================================================================

void TransformMaskList::remove()
{
	DEBUG_FATAL(!ms_installed, ("TransformMaskList not installed."));

	//-- Release class resources.
	ms_defaultTemplate->release();
	ms_defaultTemplate = 0;

	//-- Report leaked instances.
#ifdef _DEBUG
	const int leakCount = static_cast<int>(ms_templates->size());
	if (leakCount)
	{
		DEBUG_WARNING(true, ("TransformMaskList: [%d] named TransformMask instances leaked:", leakCount));

		const NamedTemplateMap::iterator endIt = ms_templates->end();
		for (NamedTemplateMap::iterator it = ms_templates->begin(); it != endIt; ++it)
		{
			DEBUG_REPORT_LOG(true, ("-| TransformMask [%s]: %d references.\n", it->first->getString(), it->second->getReferenceCount()));
		}
	}
#endif

	delete ms_templates;

	ms_installed = false;
}

// ----------------------------------------------------------------------

void TransformMaskList::stopTracking(const TransformMask &mask)
{
	DEBUG_FATAL(!ms_installed, ("TransformMaskList not installed."));

	const CrcLowerString *targetName = &(mask.getName());

	//-- Check for anonymous instances; they're not tracked.
	if (*(targetName->getString()) == 0)
		return;

	//-- Find the TransformMask entry associated with the given mask's name.
	const NamedTemplateMap::iterator lowerBoundResult = ms_templates->lower_bound(targetName);
	if ((lowerBoundResult != ms_templates->end()) && !ms_templates->key_comp()(targetName, lowerBoundResult->first))
	{
		//-- Found it, remove the entry.
		ms_templates->erase(lowerBoundResult);
	}
	else
	{
		DEBUG_WARNING(true, ("TransformMaskList::stopTracking() called for [%s] but not tracked in the first place.", mask.getName().getString()));
	}
}

// ======================================================================
