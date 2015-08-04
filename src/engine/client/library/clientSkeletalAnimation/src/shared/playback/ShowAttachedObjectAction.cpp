// ======================================================================
//
// ShowAttachedObjectAction.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/ShowAttachedObjectAction.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientSkeletalAnimation/ShowAttachedObjectActionTemplate.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

#include <string>

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(ShowAttachedObjectAction, true, 0, 0, 0);

// ======================================================================

inline const ShowAttachedObjectActionTemplate &ShowAttachedObjectAction::getOurTemplate() const
{
	return *NON_NULL(safe_cast<const ShowAttachedObjectActionTemplate*>(getPlaybackActionTemplate()));
}

// ======================================================================

ShowAttachedObjectAction::ShowAttachedObjectAction(const ShowAttachedObjectActionTemplate &actionTemplate) :
	PlaybackAction(&actionTemplate),
	m_objectIsHidden(false)
{
}

// ----------------------------------------------------------------------

bool ShowAttachedObjectAction::update(float deltaTime, PlaybackScript &script)
{
	UNREF(deltaTime);

	const ShowAttachedObjectActionTemplate &actionTemplate = getOurTemplate();
	if (!actionTemplate.isShowCommand())
	{
		// Process the hide command.

		//-- Get the actor.
		Object *const actorObject = script.getActor(actionTemplate.getActorIndex());
		if (!actorObject)
		{
			DEBUG_REPORT_LOG(true, ("SAOA: hide failed because the specified actor [%d] is NULL.\n", actionTemplate.getActorIndex()));
			return false;
		}

		//-- Get the skeletal appearance for the actor.
		SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(actorObject->getAppearance());
		if (!appearance)
		{
			DEBUG_REPORT_LOG(true, ("SAOA: hide failed because the actor is not have a skeletal appearance.\n"));
			return false;
		}

		//-- Retrieve the hardpoint name.
		std::string  hardpointName;

		const bool hasHardpointName = script.getStringVariable(actionTemplate.getHardpointNameVariable(), hardpointName);
		if (!hasHardpointName)
		{
#ifdef _DEBUG
			char variableName[5];
			ConvertTagToString(actionTemplate.getHardpointNameVariable(), variableName);
			DEBUG_REPORT_LOG(true, ("SAOA: hide failed because the hardpoint variable [%s] does not exist for the PST.\n", variableName));
#endif
			return false;
		}

		//-- Hide the attachment.
		int const attachmentIndex = appearance->findAttachmentIndexByHardpoint(TemporaryCrcString(hardpointName.c_str(), false));
		if (attachmentIndex >= 0)
		{
			appearance->hideAttachment(attachmentIndex, true);
			m_objectIsHidden = true;
		}
		else
		{
			DEBUG_REPORT_LOG(true, ("SAOA: hide failed because the actor does not have an attachment in hardpoint [%s].\n", hardpointName.c_str()));
			return false;
		}
	}
	else
	{
		// Process the show command.

		//-- Get the PlaybackAction for the corresponding hide command that should already have executed.
		const int threadIndex = actionTemplate.getHideCommandThreadIndex();
		const int actionIndex = actionTemplate.getHideCommandActionIndex();

		ShowAttachedObjectAction *const hideCommand = safe_cast<ShowAttachedObjectAction*>(script.getAction(threadIndex, actionIndex));
		NOT_NULL(hideCommand);

		hideCommand->showObject(script);
	}

	//-- Nothing more to do at this time.
	return false;
}

// ----------------------------------------------------------------------

void ShowAttachedObjectAction::cleanup(PlaybackScript &playbackScript)
{
	if (m_objectIsHidden)
	{
		// This must be a hide command with an attachmentobject that is still
		// hidden.  Show it now.
		showObject(playbackScript);
	}
}

// ----------------------------------------------------------------------

void ShowAttachedObjectAction::showObject(PlaybackScript &script)
{
	const ShowAttachedObjectActionTemplate &actionTemplate = getOurTemplate();
	DEBUG_FATAL(actionTemplate.isShowCommand(), ("showObject() may only be called on hide actions.  PST layout error."));
	
	//-- Check if there was an object removed from the specified hardpoint.
	if (!m_objectIsHidden)
	{
		DEBUG_REPORT_LOG(true, ("SAOA: showObject() skipping because no attachment was hidden.\n"));
		return;
	}

	//-- Get the actor.
	Object *const actorObject = script.getActor(actionTemplate.getActorIndex());
	if (!actorObject)
	{
		DEBUG_REPORT_LOG(true, ("SAOA: showObject() failed because the specified actor [%d] is NULL.\n", actionTemplate.getActorIndex()));
		return;
	}

	//-- Get the skeletal appearance for the actor.
	SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(actorObject->getAppearance());
	if (!appearance)
	{
		DEBUG_REPORT_LOG(true, ("SAOA: showObject() failed because the actor is not have a skeletal appearance.\n"));
		return;
	}

	//-- Get the hardpoint name.
	std::string  hardpointName;

	const bool hasHardpointName = script.getStringVariable(actionTemplate.getHardpointNameVariable(), hardpointName);
	if (!hasHardpointName)
	{
#ifdef _DEBUG
		char variableName[5];
		ConvertTagToString(actionTemplate.getHardpointNameVariable(), variableName);
		DEBUG_REPORT_LOG(true, ("SAOA: showObject() failed because the hardpoint variable [%s] does not exist for the PST.\n", variableName));
#endif
		return;
	}

	//-- Hide the attachment.
	int const attachmentIndex = appearance->findAttachmentIndexByHardpoint(TemporaryCrcString(hardpointName.c_str(), false));
	if (attachmentIndex >= 0)
	{
		appearance->hideAttachment(attachmentIndex, false);
		m_objectIsHidden = false;
	}
	else
	{
		DEBUG_REPORT_LOG(true, ("SAOA: showObject() failed because the actor does not have an attachment in hardpoint [%s].\n", hardpointName.c_str()));
		return;
	}
}

// ======================================================================
