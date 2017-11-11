// ======================================================================
//
// VisitAnimationCommand.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef REEXPORT_COMMAND_H
#define REEXPORT_COMMAND_H

// ======================================================================

class Messenger;

#include "maya/MPxCommand.h"

// ======================================================================

/**
 * This class is used to represent the "reexport" command available from the mayaExporter.
 * Its functionality is to non-interactively export the currently open Maya file by referencing
 * the log files found in the relative path ..\log\*.log.  It opens each log file to determine
 * what command line to run to start the export process, then it calls that command, which
 * assumedly another command that the mayaExporter handles.
 */
class ReexportCommand: public MPxCommand
{
public:

	static void install(Messenger *newMessenger);
	static void remove();

	static void *creator();

public:

	MStatus doIt(const MArgList &argList);

private:
	static bool processArguments(const MArgList &args, bool &partOfOtherExport, bool &noRevertOnFail, bool &commitToSourceControl, bool &commitToCurrent, std::string &reexportBranch);
	static void exportFromLogFile(const std::string& logFilename, const std::string& reexportBranch, bool commitToAlienbrain);
};

// ======================================================================

#endif
