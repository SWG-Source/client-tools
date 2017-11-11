// ======================================================================
//
// SetAuthorCommand.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef SET_AUTHOR_COMMAND_H
#define SET_AUTHOR_COMMAND_H

// ======================================================================

class Messenger;

#include "maya/MPxCommand.h"

// ======================================================================

class SetAuthorCommand: public MPxCommand
{
public:

	static void install(Messenger *newMessenger);
	static void remove();

	static void *creator();

public:

	MStatus doIt(const MArgList &argList);

};

// ======================================================================

#endif
