// ======================================================================
//
// SetAuthorCommand.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstMayaExporter.h"
#include "SetAuthorCommand.h"

#include "maya/MArgList.h"
#include "Messenger.h"
#include "PluginMain.h"
#include "SetDirectoryCommand.h"

// ======================================================================

namespace
{
	Messenger *messenger;
}

// ======================================================================

void SetAuthorCommand::install(Messenger *newMessenger)
{
	messenger = newMessenger;
}

// ----------------------------------------------------------------------

void SetAuthorCommand::remove()
{
	messenger = 0;
}

// ----------------------------------------------------------------------

void *SetAuthorCommand::creator()
{
	return new SetAuthorCommand();
}

// ======================================================================

MStatus SetAuthorCommand::doIt(const MArgList &argList)
{
	MStatus status;

	const unsigned argCount = argList.length(&status);
	MESSENGER_REJECT_STATUS(!status, ("failed to get number of arguments\n"));
	MESSENGER_REJECT_STATUS(argCount != 1, ("command requires one string argument\n"));

	MString mayaArg = argList.asString(0, &status);
	MESSENGER_REJECT_STATUS(!status, ("failed to get first argument as string\n"));

	MESSENGER_LOG(("old author: \"%s\"\n", SetDirectoryCommand::getDirectoryString(AUTHOR_INDEX)));
	SetDirectoryCommand::setDirectoryString(AUTHOR_INDEX, mayaArg.asChar());
	MESSENGER_LOG(("new author: \"%s\"\n", SetDirectoryCommand::getDirectoryString(AUTHOR_INDEX)));

	return MStatus(MStatus::kSuccess);
}

// ======================================================================
