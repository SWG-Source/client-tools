// ======================================================================
//
// AlignToHardpointAction.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/AlignToHardpointAction.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientGame/AlignToHardpointActionTemplate.h"
#include "clientGame/RemoteCreatureController.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedMath/Transform.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/Object.h"

#include <string>

// ======================================================================

namespace
{
	std::string  s_alignmentHardpointName;
	Transform    s_hardpointTransform;
	Transform    s_newTransform_o2p;
}

// ======================================================================
// inlines
// ======================================================================

inline const AlignToHardpointActionTemplate &AlignToHardpointAction::getOurTemplate() const
{
	return *NON_NULL(safe_cast<const AlignToHardpointActionTemplate*>(getPlaybackActionTemplate()));
}

// ======================================================================
// class AlignToHardpointAction: public member functions
// ======================================================================

AlignToHardpointAction::AlignToHardpointAction(const AlignToHardpointActionTemplate &actionTemplate) :
	PlaybackAction(&actionTemplate)
{
}

// ----------------------------------------------------------------------

bool AlignToHardpointAction::update(float deltaTime, PlaybackScript &script)
{
	UNREF(deltaTime);

	const AlignToHardpointActionTemplate &actionTemplate = getOurTemplate();

	//-- Get movement actor.  If target isn't available, abort the action.
	Object *const moveObject = script.getActor(actionTemplate.getMoveActorIndex());
	if (!moveObject)
	{
		DEBUG_WARNING(true, ("move object for actor index [%d] is NULL.", actionTemplate.getMoveActorIndex()));
		return false;
	}

	//-- Get the alignment actor.
	Object *const alignmentObject = script.getActor(actionTemplate.getAlignmentActorIndex());
	if (!alignmentObject)
	{
		DEBUG_WARNING(true, ("alignment object for actor index [%d] is NULL.", actionTemplate.getAlignmentActorIndex()));
		return false;
	}

	//-- Get the alignemnt actor's appearance.
	const Appearance *const alignmentAppearance = alignmentObject->getAppearance();
	if (!alignmentAppearance)
	{
		DEBUG_WARNING(true, ("alignment object doesn't have an appearance."));
		return false;
	}

	//-- Get the alignment hardpoint name.
	const bool gotHardpointName = script.getStringVariable(actionTemplate.getAlignmentHardpointNameVariable(), s_alignmentHardpointName);
	if (!gotHardpointName)
	{
		// Abort.  Specified hardpoint doesn't exist.
		char buffer[5];

		ConvertTagToString(actionTemplate.getAlignmentHardpointNameVariable(), buffer);
		DEBUG_WARNING(true, ("hardpoint variable [%s] not available for script.", buffer));
		return false;
	}

	//-- Get the alignment object's appearance-relative hardpoint.
	const bool hasHardpoint = alignmentAppearance->findHardpoint(CrcLowerString(s_alignmentHardpointName.c_str()), s_hardpointTransform);
	if (!hasHardpoint)
	{
		DEBUG_WARNING(true, ("alignment hardpoint [%s] doesn't exist on alignment object.", s_alignmentHardpointName.c_str()));
		return false;
	}

	//-- Handle moving the object to the specified spot.
	// Construct the new transform for the move object in the parent cell space.
	// @todo this assumes the alignment object is not parented to another (non-cell) object.
	// @todo this does not take into account a non-skeletal alignment object's scale.
	s_newTransform_o2p.multiply(alignmentObject->getTransform_o2c(), s_hardpointTransform);

	RemoteCreatureController *const creatureController = dynamic_cast<RemoteCreatureController*>(moveObject->getController());
	if (creatureController)
		creatureController->updateDeadReckoningModel(alignmentObject->getParentCell(), s_newTransform_o2p, 0.0f);

	//-- Move the move object into the same parent cell as the target object.  This likely already
	//   is the case already, but make sure.
	moveObject->setParentCell(alignmentObject->getParentCell());

	//-- Snap move object to the specified hardpoint.
	moveObject->setTransform_o2p(s_newTransform_o2p);

	//-- Done.
	return false;
}

// ======================================================================
