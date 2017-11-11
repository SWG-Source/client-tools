// ======================================================================
//
// TemporaryAttachedObjectAction.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/TemporaryAttachedObjectAction.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientGame/TemporaryAttachedObjectActionTemplate.h"
#include "clientGraphics/RenderWorld.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedObject/ObjectTemplateList.h"

#include <string>

// ======================================================================

namespace TemporaryAttachedObjectActionNamespace
{
	const Tag TAG_AWPN = TAG(A,W,P,N);

	const ConstCharCrcString cs_holdLeftHardpointName("hold_l");
	const ConstCharCrcString cs_holdRightHardpointName("hold_r");

	std::string  s_temporaryString;
}

using namespace TemporaryAttachedObjectActionNamespace;

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(TemporaryAttachedObjectAction, true, 0, 0, 0);

// ======================================================================

inline const TemporaryAttachedObjectActionTemplate &TemporaryAttachedObjectAction::getOurTemplate() const
{
	return *NON_NULL(safe_cast<const TemporaryAttachedObjectActionTemplate*>(getPlaybackActionTemplate()));
}

// ======================================================================

TemporaryAttachedObjectAction::TemporaryAttachedObjectAction(const TemporaryAttachedObjectActionTemplate &actionTemplate) :
	PlaybackAction(&actionTemplate),
	m_temporaryObject()
{
}

// ----------------------------------------------------------------------

bool TemporaryAttachedObjectAction::update(float deltaTime, PlaybackScript &script)
{
	UNREF(deltaTime);

	const TemporaryAttachedObjectActionTemplate &actionTemplate = getOurTemplate();
	if (actionTemplate.isCreateCommand())
	{
		// Process the create command.

		//-- Get the actor.
		Object *const actorObject = script.getActor(actionTemplate.getActorIndex());
		if (!actorObject)
		{
			DEBUG_REPORT_LOG(true, ("TAOA: create failed because the specified actor [%d] is NULL.\n", actionTemplate.getActorIndex()));
			return false;
		}

		//-- Get the skeletal appearance for the actor.
		SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(actorObject->getAppearance());
		if (!appearance)
		{
			DEBUG_REPORT_LOG(true, ("TAOA: create failed because the actor is not have a skeletal appearance.\n"));
			return false;
		}

		//-- Retrieve the hardpoint name.
		const bool hasHardpointName = script.getStringVariable(actionTemplate.getHardpointNameVariable(), s_temporaryString);
		if (!hasHardpointName)
		{
#ifdef _DEBUG
			char variableName[5];
			ConvertTagToString(actionTemplate.getHardpointNameVariable(), variableName);
			DEBUG_REPORT_LOG(true, ("TAOA: create failed because the hardpoint variable [%s] does not exist for the PST.\n", variableName));
#endif
			return false;
		}

		// @todo fix up CRC-ness.
		const CrcLowerString  hardpointName(s_temporaryString.c_str());

		//-- Ensure nothing exists at the hardpoint.  If a script needs to hide
		//   an object existing in a slot, such as in this case, use the 
		//   ShowAttachedObject(Template), hide mode, to hide any object existing 
		//   in a specified hardpoint.  See SHOW_ATTACHED_OBJECT/HIDE_ATTACHED_OBJECT
		//   in playback.def for .pst miff compilation.

#ifdef _DEBUG
		{
			Object *const attachedObject = appearance->findAttachedObject(hardpointName);
			if (attachedObject)
			{
				DEBUG_WARNING(true, ("TAOA: create failed because an object already exists at the specified location.  PST logic error."));
				return false;
			}
		}
#endif

		//-- Create the temporary object from the given object template.
		// Get the object template name.
		const bool hasObjectTemplateName = script.getStringVariable(actionTemplate.getObjectTemplateNameVariable(), s_temporaryString);
		if (!hasObjectTemplateName)
		{
#ifdef _DEBUG
			char variableName[5];
			ConvertTagToString(actionTemplate.getObjectTemplateNameVariable(), variableName);
			DEBUG_REPORT_LOG(true, ("TAOA: create failed because the object template variable [%s] does not exist for the PST.\n", variableName));
#endif
			return false;
		}

		// Create an object from the object template.
 		Object *const attachedObject = ObjectTemplateList::createObject(TemporaryCrcString(s_temporaryString.c_str(), true));
		if (!attachedObject)
		{
			DEBUG_REPORT_LOG(true, ("TAOA: create failed because creating an object from [%s] failed.\n", s_temporaryString.c_str()));
			return false;			
		}

		// Save a watcher to the object.
		m_temporaryObject = attachedObject;

		// Enable rendering.
		RenderWorld::addObjectNotifications(*attachedObject);

		//-- Put the object in the specified hard point.
		appearance->attach(attachedObject, hardpointName);

		//-- If placing temp object in right or left hand, specify this object as the weapon for the attack.
		if ((hardpointName == cs_holdLeftHardpointName) || (hardpointName == cs_holdRightHardpointName))
			script.setObjectVariable(TAG_AWPN, attachedObject);
	}
	else
	{
		// Process the destroy command.

		//-- Get the PlaybackAction for the corresponding show command that should already have executed.
		const int threadIndex = actionTemplate.getCreateCommandThreadIndex();
		const int actionIndex = actionTemplate.getCreateCommandActionIndex();

		TemporaryAttachedObjectAction *const createCommand = safe_cast<TemporaryAttachedObjectAction*>(script.getAction(threadIndex, actionIndex));
		NOT_NULL(createCommand);

		createCommand->destroyObject(script);
	}

	//-- Nothing more to do at this time.
	return false;
}

// ----------------------------------------------------------------------

void TemporaryAttachedObjectAction::cleanup(PlaybackScript &playbackScript)
{
	if (m_temporaryObject != 0)
	{
		// This must be a hide command with a target attached object that is still
		// hidden.  Show it now.
		destroyObject(playbackScript);
	}
}

// ----------------------------------------------------------------------

void TemporaryAttachedObjectAction::destroyObject(PlaybackScript &script)
{
	const TemporaryAttachedObjectActionTemplate &actionTemplate = getOurTemplate();
	DEBUG_FATAL(!actionTemplate.isCreateCommand(), ("destroyObject() may only be called on create actions.  PST layout error."));
	
	//-- Check if there was an object created during the create process.
	if (!m_temporaryObject)
	{
		DEBUG_REPORT_LOG(true, ("TAOA: destroyObject() skipping because no object was created during the create process.\n"));
		return;
	}

	//-- Get the actor.
	Object *const actorObject = script.getActor(actionTemplate.getActorIndex());
	if (!actorObject)
	{
		DEBUG_REPORT_LOG(true, ("TAOA: destroyObject() failed because the specified actor [%d] is NULL.\n", actionTemplate.getActorIndex()));
		return;
	}

	//-- Get the skeletal appearance for the actor.
	SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(actorObject->getAppearance());
	if (!appearance)
	{
		DEBUG_REPORT_LOG(true, ("TAOA: destroyObject() failed because the actor is not have a skeletal appearance.\n"));
		return;
	}

	//-- Remove the object.
	appearance->detach(m_temporaryObject.getPointer());

	//-- Destroy the temporary object.
	m_temporaryObject->kill();
	m_temporaryObject = 0;
}

// ======================================================================
