// ======================================================================
//
// SetBaseDirectory.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef SET_BASE_DIRECTORY_H
#define SET_BASE_DIRECTORY_H

// ======================================================================

class Messenger;

#include "maya/MPxCommand.h"

// ======================================================================

class SetBaseDirectory: public MPxCommand
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
