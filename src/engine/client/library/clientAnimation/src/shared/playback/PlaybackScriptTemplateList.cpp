// ======================================================================
//
// PlaybackScriptTemplateList.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientAnimation/FirstClientAnimation.h"
#include "clientAnimation/PlaybackScriptTemplateList.h"

#include "clientAnimation/PlaybackScriptTemplate.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"

#include <map>
#include <string>

// ======================================================================

bool                                     PlaybackScriptTemplateList::ms_installed;
PlaybackScriptTemplateList::TemplateMap *PlaybackScriptTemplateList::ms_templates;

// ======================================================================
// public static member functions
// ======================================================================

void PlaybackScriptTemplateList::install()
{
	InstallTimer const installTimer("PlaybackScriptTemplateList::install");

	DEBUG_FATAL(ms_installed, ("PlaybackScriptTemplateList already installed"));

	ms_templates = new TemplateMap();

	ms_installed = true;
	ExitChain::add(remove, "PlaybackScriptTemplateList");
}

// ----------------------------------------------------------------------

const PlaybackScriptTemplate *PlaybackScriptTemplateList::fetch(const CrcLowerString &filename)
{
	DEBUG_FATAL(!ms_installed, ("PlaybackScriptTemplateList not installed"));

	PlaybackScriptTemplate *scriptTemplate = 0;

	//-- check if we've got this one cached
	TemplateMap::iterator lowerBoundResult = ms_templates->lower_bound(&filename);
	if ((lowerBoundResult != ms_templates->end()) && !ms_templates->key_comp()(&filename, lowerBoundResult->first))
	{
		// it's cached, return it
		scriptTemplate = lowerBoundResult->second;
	}
	else
	{
		// it's not available

		//-- ensure sure file exists
		if (!TreeFile::exists(filename.getString()))
		{
			WARNING(true, ("specified PlaybackScriptTemplate file [%s] does not exist\n", filename.getString()));
			return 0;
		}

		//-- load it
		Iff iff(filename.getString());
		scriptTemplate = new PlaybackScriptTemplate(iff, filename);

		//-- add to managed container
		IGNORE_RETURN(ms_templates->insert(lowerBoundResult, TemplateMap::value_type(&scriptTemplate->getName(), scriptTemplate)));
	}

	// we should have a valid slot scriptTemplate here
	NOT_NULL(scriptTemplate);

	//-- bump up reference count
	scriptTemplate->fetch();

	return scriptTemplate;
}

// ----------------------------------------------------------------------

PlaybackScript *PlaybackScriptTemplateList::createPlaybackScript(const CrcLowerString &templateName, int priority, const ObjectVector &actors)
{
	//-- fetch the PlaybackScriptTemplate
	const PlaybackScriptTemplate *const scriptTemplate = fetch(templateName);
	if (!scriptTemplate)
		return NULL;

	//-- create the PlaybackScript
	PlaybackScript *const playbackScript = scriptTemplate->createPlaybackScript(priority, actors);

	//-- release the PlaybackScriptTemplate reference
	scriptTemplate->release();

	return playbackScript;
}

// ======================================================================
// private static member functions
// ======================================================================

void PlaybackScriptTemplateList::remove()
{
	DEBUG_FATAL(!ms_installed, ("PlaybackScriptTemplateList not installed"));

	//-- handle leaks
	DEBUG_WARNING(!ms_templates->empty(), ("PlaybackScriptTemplateList: %u named PlaybackScriptTemplate leaks, not safe to delete now so leaking.\n", ms_templates->size()));

	delete ms_templates;
	ms_templates = 0;

	ms_installed = false;
}

// ----------------------------------------------------------------------

void PlaybackScriptTemplateList::stopTracking(const PlaybackScriptTemplate *playbackScriptTemplate)
{
	DEBUG_FATAL(!ms_installed, ("PlaybackScriptTemplateList not installed"));

	const CrcLowerString &filename = playbackScriptTemplate->getName();

	TemplateMap::iterator lowerBoundResult = ms_templates->lower_bound(&filename);
	if ((lowerBoundResult == ms_templates->end()) || ms_templates->key_comp()(&filename, lowerBoundResult->first))
	{
		// we don't manage this playbackScriptTemplate
		return;
	}

	//-- remove the SlotDescriptor from the map
	ms_templates->erase(lowerBoundResult);
}

// ======================================================================
