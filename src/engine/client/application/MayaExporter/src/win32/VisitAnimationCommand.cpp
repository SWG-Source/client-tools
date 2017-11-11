// ======================================================================
//
// VisitAnimationCommand.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstMayaExporter.h"
#include "VisitAnimationCommand.h"

#include "maya/MAnimControl.h"
#include "maya/MArgList.h"
#include "maya/MDagPath.h"
#include "maya/MFnDagNode.h"
#include "maya/MGlobal.h"
#include "maya/MSelectionList.h"
#include "maya/MTime.h"

#include "MayaUtility.h"
#include "Messenger.h"
#include "PluginMain.h"
#include "Resource.h"

#include <map>
#include <utility>
#include <vector>

// ======================================================================

MayaAnimationList*                        VisitAnimationCommand::m_animationList;
VisitAnimationCommand::SelectionContainer VisitAnimationCommand::m_allAnimations;
VisitAnimationCommand::SelectionContainer VisitAnimationCommand::m_selectedAnimations;

// ======================================================================

namespace
{
	Messenger *messenger;
}

// ======================================================================
// class VisitAnimationCommand
// ======================================================================

void VisitAnimationCommand::install(Messenger *newMessenger)
{
	messenger = newMessenger;
}

// ----------------------------------------------------------------------

void VisitAnimationCommand::remove()
{
	messenger = 0;
}

// ----------------------------------------------------------------------

void *VisitAnimationCommand::creator()
{
	return new VisitAnimationCommand();
}

// ======================================================================

MStatus VisitAnimationCommand::doIt(const MArgList &argList)
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

bool VisitAnimationCommand::showGUI()
{
	MStatus status;

	//-- get name of animation from user
	bool                   done;
	do
	{
		int result = DialogBox(GetPluginInstanceHandle(), MAKEINTRESOURCE (IDD_SELECT_ANIMATIONS), NULL, visitAnimationDialogProc); //lint !e1924 C-style cast (this conveinent macro uses one "internally")
		if(result != IDOK)
		{
			//-- user cancelled operation
			return false;
		}

		const SelectionContainer &selectionContainer = m_selectedAnimations;
		if (selectionContainer.empty())
		{
			// user did not select any animations, break (no error)
			return true;
		}
		else if (selectionContainer.size() > 1)
		{
			const int mbResult = MESSENGER_MESSAGE_BOX(NULL, "You may only visit a single animtion at a time.  Try again?", "Multiple Animations Selected", MB_ICONSTOP | MB_YESNO);

			// quit only if user chose not to re-enter
			done = (mbResult != IDYES);
		}
		else
		{
			// user made a single selection.
			//-- set the animation range to that of the specified animation
			SelectionContainer::const_iterator iteratorIt = selectionContainer.begin();
			DEBUG_FATAL(iteratorIt == selectionContainer.end(), ("logic error: iterator == end of container"));

			MayaAnimationList::AnimationInfoMap::iterator animationInfoIt = *iteratorIt;
			MayaAnimationList::AnimationInfo &animationInfo = *NON_NULL(animationInfoIt->second);

			//-- set range start time
			MTime    firstTime;

			status = firstTime.setValue(static_cast<double>(animationInfo.m_firstFrame));
			MESSENGER_REJECT(!status, ("MTime.setValue(%.2f) failed\n", static_cast<double>(animationInfo.m_firstFrame)));

			status = MAnimControl::setMinTime(firstTime);
			MESSENGER_REJECT(!status, ("MAnimControl::setMinTime() failed\n"));

			//-- set range start time
			MTime    lastTime;

			status = lastTime.setValue(static_cast<double>(animationInfo.m_lastFrame));
			MESSENGER_REJECT(!status, ("MTime.setValue(%.2f) failed\n", static_cast<double>(animationInfo.m_lastFrame)));

			status = MAnimControl::setMaxTime(lastTime);
			MESSENGER_REJECT(!status, ("MAnimControl::setMinTime() failed\n"));
			done = true;
		}
	} while (!done);

	delete m_animationList;
	m_animationList = 0;

	// success
	return true;
}

// ======================================================================

BOOL CALLBACK VisitAnimationCommand::visitAnimationDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	MStatus  status;

	UNREF(lParam); 

	HWND animationsListBox                = GetDlgItem(hDlg, IDC_LIST_ANIMATIONS);
	HWND selectAllButton                  = GetDlgItem(hDlg, IDC_BUTTON_SELECT_ALL);
	UNREF(selectAllButton);

	switch(iMsg)
	{
		case WM_INITDIALOG:
		{
			delete m_animationList;
			m_animationList = 0;

			//-- get the skeleton root node (might be the node selected, might be an ancestor)
			MDagPath  skeletonRootDagPath;
			const bool gsrResult = MayaUtility::getSkeletonRootDagPath(&skeletonRootDagPath);
			MESSENGER_REJECT(!gsrResult, ("failed to get skeleton root\n"));

			m_animationList = new MayaAnimationList(skeletonRootDagPath, &status);
			MESSENGER_REJECT(!status, ("failed to create animation list with storage at skeleton root dag path [%s]\n", skeletonRootDagPath.partialPathName().asChar()));

			m_allAnimations.clear();
			m_selectedAnimations.clear();

			//put all the animations in the container
			AnimationInfoMap& aiMap = m_animationList->getAnimationInfoMap();
			m_allAnimations.reserve(aiMap.size());
			for (AnimationInfoMapIterator i = aiMap.begin(); i != aiMap.end(); ++i)
				m_allAnimations.push_back(i);

			//now put all the animations in the list box
			const SelectionContainer::iterator  itEnd = m_allAnimations.end();
			SelectionContainer::iterator        it    = m_allAnimations.begin();
			for (; it != itEnd; ++it)
			{
				//add item to the list box
				const MayaAnimationList::AnimationInfo &animationInfo = *NON_NULL((*it)->second);
				int index = SendMessage(animationsListBox, LB_ADDSTRING, 0, (LPARAM) animationInfo.m_animationName.getString()); //lint !e1924 C-style cast, needed by silly Win32API code

				//add data to the item
				IGNORE_RETURN(SendMessage(animationsListBox, LB_SETITEMDATA, index, (LPARAM) &(*it))); //lint !e1924 C-style cast, needed by silly Win32API code
			}
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_BUTTON_SELECT_ALL:
					switch(HIWORD(wParam)) //lint !e1924 C-style cast, needed by silly Win32API code
					{
						case BN_CLICKED:
						{
							int count = SendMessage(animationsListBox, LB_GETCOUNT, 0, 0);
							for(int i = 0; i < count; ++i)
								IGNORE_RETURN(SendMessage(animationsListBox, LB_SETSEL, true, i));
							break;
						}
						default:
							break;
					}
					break;

				case IDOK:
				{
					//fill in selected animations
					unsigned int count = static_cast<unsigned int>(SendMessage(animationsListBox, LB_GETCOUNT, 0, 0));
					int* selectedItems = new int[count];
					int numSelected = SendMessage(animationsListBox, LB_GETSELITEMS, count, (LPARAM) &selectedItems[0]); //lint !e1924 C-style cast, needed by silly Win32API code
					m_selectedAnimations.clear();
					m_selectedAnimations.reserve(count);
					for (int i = 0; i < numSelected; ++i)
					{
						SelectionContainer::iterator it = reinterpret_cast<SelectionContainer::iterator>(SendMessage(animationsListBox, LB_GETITEMDATA, selectedItems[static_cast<unsigned int>(i)], 0));
						m_selectedAnimations.push_back(*it);
					}
					delete[] selectedItems;
					IGNORE_RETURN(EndDialog(hDlg, IDOK));
					return TRUE;
				}

				case IDCANCEL:

					IGNORE_RETURN(EndDialog(hDlg, IDCANCEL));
					return TRUE;

				default:
					return FALSE;
			}

		default:        //lint !e616 !e825 control flow falls though, otherwise we get unreachable code according to MSVC (sigh)
			return FALSE;
	}
}

// ======================================================================

