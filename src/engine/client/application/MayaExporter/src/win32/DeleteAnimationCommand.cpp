// ======================================================================
//
// DeleteAnimationCommand.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstMayaExporter.h"
#include "DeleteAnimationCommand.h"

#include <map>
#include "maya/MAnimControl.h"
#include "maya/MArgList.h"
#include "maya/MDagPath.h"
#include "maya/MFnDagNode.h"
#include "maya/MGlobal.h"
#include "maya/MSelectionList.h"
#include "maya/MTime.h"

#include "MayaAnimationList.h"
#include "MayaUtility.h"
#include "Messenger.h"
#include "PluginMain.h"
#include "Resource.h"
#include "VisitAnimationCommand.h"

#include <utility>
#include <vector>

// ======================================================================

namespace
{
	Messenger *messenger;
}

// ======================================================================
// class DeleteAnimationCommand
// ======================================================================

void DeleteAnimationCommand::install(Messenger *newMessenger)
{
	messenger = newMessenger;
}

// ----------------------------------------------------------------------

void DeleteAnimationCommand::remove()
{
	messenger = 0;
}

// ----------------------------------------------------------------------

void *DeleteAnimationCommand::creator()
{
	return new DeleteAnimationCommand();
}

// ======================================================================

MStatus DeleteAnimationCommand::doIt(const MArgList &argList)
{
	MStatus status;

	//-- command does not expect any args
	const unsigned argCount = argList.length(&status);
	MESSENGER_REJECT_STATUS(!status, ("failed to get number of arguments\n"));
	MESSENGER_REJECT_STATUS(argCount > 0, ("command does not expect any arguments\n"));

	if(showGUI())
		return MStatus(MStatus::kSuccess);
	else
		return MStatus(MStatus::kFailure);
}

// ======================================================================

bool DeleteAnimationCommand::showGUI()
{
	MStatus status;

	int result = DialogBox(GetPluginInstanceHandle(), MAKEINTRESOURCE (IDD_SELECT_ANIMATIONS), NULL, VisitAnimationCommand::visitAnimationDialogProc); //lint !e1924 C-style cast (this conveinent macro uses one "internally")
	if(result != IDOK)
	{
		//-- user cancelled operation
		return false;
	}

	// we cannot remove the animations by the given iterators since removal of one iterator will invalidate the rest.
	//-- save the names we want to remove instead, them remove using those names.
	std::vector<const CrcLowerString*> animationsToRemove;

	const VisitAnimationCommand::SelectionContainer &selectionContainer = VisitAnimationCommand::m_selectedAnimations;
	animationsToRemove.reserve(selectionContainer.size());

	{
		const VisitAnimationCommand::SelectionContainer::const_iterator itEnd = selectionContainer.end();
		for (VisitAnimationCommand::SelectionContainer::const_iterator it = selectionContainer.begin(); it != itEnd; ++it)
		{
			const CrcLowerString* s = (*it)->first;
			animationsToRemove.push_back(s);
		}
	}

	//-- now remove the animations from the list
	{
		MayaAnimationList::AnimationInfoMap &animationInfoMap = VisitAnimationCommand::m_animationList->getAnimationInfoMap();

		const std::vector<const CrcLowerString*>::const_iterator itEnd = animationsToRemove.end();
		for (std::vector<const CrcLowerString*>::const_iterator it = animationsToRemove.begin(); it != itEnd; ++it)
		{
			IGNORE_RETURN(animationInfoMap.erase(*it));
		}
	}

	//-- commit the list back to the node
	IGNORE_RETURN(VisitAnimationCommand::m_animationList->saveMapToStorage());

	delete VisitAnimationCommand::m_animationList;
	VisitAnimationCommand::m_animationList = 0;

	// success
	return true;
}

// ======================================================================

