// ======================================================================
//
// AddAnimationCommand.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstMayaExporter.h"
#include "AddAnimationCommand.h"

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

#include <map>
#include <utility>

// ======================================================================

std::string AddAnimationCommand::m_animationName;

namespace
{
	Messenger *messenger;
}

// ======================================================================
// class AddAnimationCommand
// ======================================================================

void AddAnimationCommand::install(Messenger *newMessenger)
{
	messenger = newMessenger;
}

// ----------------------------------------------------------------------

void AddAnimationCommand::remove()
{
	messenger = 0;
}

// ----------------------------------------------------------------------

void *AddAnimationCommand::creator()
{
	return new AddAnimationCommand();
}

// ======================================================================

MStatus AddAnimationCommand::doIt(const MArgList &argList)
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

bool AddAnimationCommand::showGUI()
{
	typedef MayaAnimationList::AnimationInfoMap  AIMap;
	typedef AIMap::iterator                      AIMapIterator;

	MStatus status;

	//-- get the skeleton root node (might be the node selected, might be an ancestor)
	MDagPath  skeletonRootDagPath;
	const bool gsrResult = MayaUtility::getSkeletonRootDagPath(&skeletonRootDagPath);
	MESSENGER_REJECT(!gsrResult, ("failed to get skeleton root\n"));

	//-- retrieve animation list associated with root node
	MayaAnimationList animationList(skeletonRootDagPath, &status);
	MESSENGER_REJECT(!status, ("failed to create animation list with storage at skeleton root dag path [%s]\n", skeletonRootDagPath.partialPathName().asChar()));

	AIMap &animationInfoMap = animationList.getAnimationInfoMap();

	//-- get name of animation from user
	bool done;
	do
	{
		int result = DialogBox(GetPluginInstanceHandle(), MAKEINTRESOURCE (IDD_ADD_ANIMATION), NULL, addAnimationDialogProc); //lint !e1924 C-style cast (this conveinent macro uses one "internally")
		if (result != IDOK)
		{
			//-- user cancelled operation
			done = true;
		}
		else
		{
			//-- user requested we add a new animation
			CrcLowerString  crcNewName(getAnimationName().c_str());
			bool            writeData = false;

			AIMapIterator it = animationInfoMap.find(&crcNewName);
			if (it == animationInfoMap.end())
			{
				writeData = true;

				MayaAnimationList::AnimationInfo *const newAnimationInfo = new MayaAnimationList::AnimationInfo(crcNewName.getString());
				std::pair<AIMapIterator, bool> insertResult = animationInfoMap.insert(AIMap::value_type(&newAnimationInfo->m_animationName, newAnimationInfo));
				if (!insertResult.second)
				{
					delete newAnimationInfo;
					MESSENGER_LOG(("both find and insert failed for animation [%s]\n", crcNewName.getString()));
					return false;
				}

				it = insertResult.first;
			} //lint !e429 // custodial pointer not freed or returned
			else
			{
				// this animation name already exists in the list!
				char message[512];

				IGNORE_RETURN( _snprintf(message, sizeof(message), "Animation [%s] already exists.  Press Yes to redefine range and export node for animation, or press No to enter a new name.", crcNewName.getString()) );
				const int mbResult = MESSENGER_MESSAGE_BOX(NULL, message, "Animation Name Conflict", MB_ICONSTOP | MB_YESNO | MB_DEFBUTTON2);

				// only write the data if the user selected "Yes" to redefine animation info
				writeData = (mbResult == IDYES);
			}

			if (!writeData)
				done = false;
			else
			{
				//-- write the data
				MESSENGER_REJECT(!it->second, ("error: AnimationInfo is null\n"));
				MayaAnimationList::AnimationInfo &animationInfo = *(it->second);

				// grab selected node name
				MSelectionList selectionList;

				status = MGlobal::getActiveSelectionList(selectionList);
				MESSENGER_REJECT(!status, ("failed to get the active selection list\n"));
				MESSENGER_REJECT(selectionList.length() != 1, ("exactly one item should be selected.  currently %u items are selected.\n", selectionList.length()));

				MDagPath selectedDagPath;
				status = selectionList.getDagPath(0, selectedDagPath);
				MESSENGER_REJECT(!status, ("failed to get dag path for selected item --- is it a DAG node?\n"));

				MFnDagNode selectedDagNode(selectedDagPath, &status);
				MESSENGER_REJECT(!status, ("failed to set MFnDagNode for selected dag node\n"));

				const MString mayaNodeName = selectedDagNode.name(&status);
				MESSENGER_REJECT(!status, ("failed to get name of dag node\n"));

				animationInfo.m_exportNodeName = mayaNodeName.asChar();

				// grab first and last frame from animation control playback slider bar
				const MTime minTime = MAnimControl::minTime();
				animationInfo.m_firstFrame = static_cast<int>(minTime.value());

				const MTime maxTime = MAnimControl::maxTime();
				animationInfo.m_lastFrame = static_cast<int>(maxTime.value());

				// write the new data back to the skeleton root node
				const bool saveResult = animationList.saveMapToStorage();
				MESSENGER_REJECT(!saveResult, ("failed to save animation list to skeleton root node\n"));

				//-- we're done, get out of the loop
				done = true;
			}
		}
	} while (!done);

	// success
	return true;
}

// ======================================================================

BOOL CALLBACK AddAnimationCommand::addAnimationDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	UNREF(lParam); 

	HWND animationNameEdit                = GetDlgItem(hDlg, IDC_EDIT_ANIMATION_NAME);
	UNREF(animationNameEdit);

	switch(iMsg)
	{
		case WM_INITDIALOG:
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_EDIT_ANIMATION_NAME:
					switch(HIWORD(wParam)) //lint !e1924 C-style cast, needed by silly Win32API code
					{
						//if the edit box has been updated, store the new value
						case EN_UPDATE:
						{
							int length = GetWindowTextLength(animationNameEdit);
							char* buffer = new char[static_cast<unsigned int>(length) + 1];
							IGNORE_RETURN(GetWindowText(animationNameEdit, buffer, length+1));
							m_animationName = buffer;
							delete[] buffer;
							break;
						}
						default:
							break;
					}
					break;

				case IDOK:
				{
					IGNORE_RETURN(EndDialog(hDlg, IDOK));
					return TRUE;
				}

				case IDCANCEL:
					IGNORE_RETURN(EndDialog(hDlg, IDCANCEL));
					return TRUE;

				default:
					return FALSE;
			}

		default:         //lint !e616 !e825 control flow falls though, otherwise we get unreachable code according to MSVC (sigh)
			return FALSE;
	}
}

// ======================================================================
