// ======================================================================
//
// DeleteAnimationCommand.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef DELETE_ANIMATION_COMMAND_H
#define DELETE_ANIMATION_COMMAND_H

// ======================================================================

class Messenger;

#include "maya/MPxCommand.h"

#include "MayaAnimationList.h"

#include <map>

// ======================================================================

class DeleteAnimationCommand: public MPxCommand
{
public:

	static void install(Messenger *newMessenger);
	static void remove();

	static void *creator();

public:

	MStatus doIt(const MArgList &argList);

private:
	static bool showGUI();
};

// ======================================================================

#endif
