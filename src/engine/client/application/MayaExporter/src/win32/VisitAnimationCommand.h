// ======================================================================
//
// VisitAnimationCommand.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef VISIT_ANIMATION_COMMAND_H
#define VISIT_ANIMATION_COMMAND_H

// ======================================================================

class Messenger;

#include "maya/MPxCommand.h"

#include "MayaAnimationList.h"

#include <map>

// ======================================================================

class MayaAnimationList;

// ======================================================================

class VisitAnimationCommand: public MPxCommand
{
public:
	static void install(Messenger *newMessenger);
	static void remove();
	static void *creator();

public:
	MStatus doIt(const MArgList &argList);

public:
	static BOOL CALLBACK visitAnimationDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

	typedef MayaAnimationList::AnimationInfoMap     AnimationInfoMap;
	typedef AnimationInfoMap::iterator              AnimationInfoMapIterator;
	typedef stdvector<AnimationInfoMap::iterator>::fwd SelectionContainer;

	static SelectionContainer m_selectedAnimations;
	static MayaAnimationList* m_animationList;
	static SelectionContainer m_allAnimations;

private:
	static bool showGUI();
};

// ======================================================================

#endif
