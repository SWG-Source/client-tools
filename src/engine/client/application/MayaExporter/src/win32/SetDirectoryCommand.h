// ======================================================================
//
// SetDirectoryCommand.h
// copyright 1999, Bootprint Entertainment
// copyright 2001, Sony Online Entertainment
//
// ======================================================================

#ifndef SET_DIRECTORY_H
#define SET_DIRECTORY_H

// ======================================================================

class MArgList;
class Messenger;
class MStatus;

#include "maya/MPxCommand.h"

// ======================================================================

class SetDirectoryCommand: public MPxCommand
{
public:

	static void        install(Messenger *newMessenger);
	static void        remove();

	static int         registerDirectory(const char *title);
	static int         getDirectoryId(const char *title);

	static const char *getDirectoryString(int directoryId);
	static void        setDirectoryString(int directoryId, const char *newDirString);

	static void       *creator(void);

public:

	MStatus doIt(const MArgList &argList);

};

// ======================================================================

#endif
