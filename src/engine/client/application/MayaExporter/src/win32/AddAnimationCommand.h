// ======================================================================
//
// AddAnimationCommand.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef ADD_ANIMATION_COMMAND_H
#define ADD_ANIMATION_COMMAND_H

// ======================================================================

#include "maya/MPxCommand.h"
#include <string>

// ======================================================================

class Messenger;

// ======================================================================

class AddAnimationCommand: public MPxCommand
{
public:

	static void install(Messenger *newMessenger);
	static void remove();
	static void *creator();

public:
	static std::string getAnimationName();

public:
	MStatus doIt(const MArgList &argList);

private:
	static bool showGUI();
	static BOOL CALLBACK addAnimationDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

private:
	static std::string m_animationName;
};

// ======================================================================

inline std::string AddAnimationCommand::getAnimationName()
{
	return m_animationName;
}

// ======================================================================

#endif
