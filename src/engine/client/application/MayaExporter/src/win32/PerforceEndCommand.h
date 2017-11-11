// ======================================================================
//
// PerforceEndCommand.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef PERFORCEEND_COMMAND_H
#define PERFORCEEND_COMMAND_H

// ======================================================================

class Messenger;

#include "maya/MPxCommand.h"

// ======================================================================

/**
 * This class wraps the "perforceEndMultiExport" command.  That command only has
 * one optional flag, "-interactive", which makes the function show GUI dialogs
 * for success and failure states.  By default it does not display a GUI, to make
 * auto-export functions easier to implement.
 *
 * This function submits the currently open changelist.  If no changelist is open
 * then this function simply returns.
 */
class PerforceEndCommand : public MPxCommand
{
public:

	static void install(Messenger *newMessenger);
	static void remove();

	static bool processArguments(const MArgList &args, bool *interactive);
	static void *creator();

public:

	MStatus doIt(const MArgList &argList);
};

// ======================================================================

#endif
