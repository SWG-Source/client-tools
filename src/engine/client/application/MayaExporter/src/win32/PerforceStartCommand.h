// ======================================================================
//
// PerforceStartCommand.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef PERFORCESTART_COMMAND_H
#define PERFORCESTART_COMMAND_H

// ======================================================================

class Messenger;

#include "maya/MPxCommand.h"

// ======================================================================

/**
 * This class wraps the "perforceStartMultiExport" command.  That command only has
 * one optional flag, "-interactive", which makes the function show GUI dialogs
 * for success and failure states.  By default it does not display a GUI, to make
 * auto-export functions easier to implement.
 *
 * This function opens a changelist and tells the perforce importer system to not
 * submit any changelist until the user explicitly calls "perforceEndMultiExport".
 */
class PerforceStartCommand : public MPxCommand
{
public:

	static void install(Messenger *newMessenger);
	static void remove();

	static bool processArguments(const MArgList &args, bool &interactive, bool &commitToCurrent);
	static void *creator();

public:

	MStatus doIt(const MArgList &argList);
};

// ======================================================================

#endif
