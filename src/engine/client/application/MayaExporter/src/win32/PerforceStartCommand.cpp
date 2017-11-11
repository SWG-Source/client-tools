// ======================================================================
//
// PerforceStartCommand.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstMayaExporter.h"
#include "PerforceStartCommand.h"

#include "AlienbrainImporter.h"
#include "ExportArgs.h"
#include "PerforceImporter.h"
#include "maya/MArgList.h"
#include "Messenger.h"

static Messenger *messenger;

// ======================================================================
// class PerforceStartCommand
// ======================================================================


void PerforceStartCommand::install(Messenger *newMessenger)
{
	messenger = newMessenger;
}

// ----------------------------------------------------------------------

void PerforceStartCommand::remove()
{
	messenger = 0;
}

// ----------------------------------------------------------------------

void *PerforceStartCommand::creator()
{
	return new PerforceStartCommand();
}

// ----------------------------------------------------------------------

bool PerforceStartCommand::processArguments(const MArgList &args, bool &interactive, bool &commitToCurrent)
{
	MESSENGER_INDENT;
	MStatus  status;

	const unsigned argCount = args.length(&status);
	MESSENGER_REJECT(!status, ("failed to get args length\n"));

	//-- handle each argument
	interactive     = false;
	commitToCurrent = true;

	// NOTE: interactive or autoexport argument needs to come first

	for (unsigned argIndex = 0; argIndex < argCount; ++argIndex)
	{
		MString argName = args.asString(argIndex, &status);
		MESSENGER_REJECT(!status, ("failed to get arg [%u] as string\n", argIndex));

		IGNORE_RETURN( argName.toLowerCase() );

		if (argName == ExportArgs::cs_interactiveArgName)
		{
			interactive   = true;
		}
		else if (argName == ExportArgs::cs_commitToCurrentArgName)
		{
			MESSENGER_REJECT(true, ("%s is not a valid argument", ExportArgs::cs_commitToGoldArgName));
		}
		else if (argName == ExportArgs::cs_commitToGoldArgName)
		{
			MESSENGER_REJECT(true, ("%s is not a valid argument", ExportArgs::cs_commitToGoldArgName));
		}
		else
		{
			MESSENGER_LOG_ERROR(("unknown argument [%s]\n", argName.asChar()));
			return false;
		}
	}
/*
	if(!commitTypeSet)
	{
		MESSENGER_LOG_ERROR(("either -committocurrent or -committogold must be declared for the command\n"));
		if (interactive)
		{
			MessageBox(NULL, "Either -committocurrent or -committogold must be declared for the command", "Error", MB_OK);
		}
		return false;
	}
*/
	return true;
}

// ----------------------------------------------------------------------

MStatus PerforceStartCommand::doIt(const MArgList &argList)
{
	bool interactive     = false;
	bool commitToCurrent = true;
	bool success = processArguments(argList, interactive, commitToCurrent);
	if(success)
	{
		PerforceImporter::startMultiExport(interactive, commitToCurrent);
		IGNORE_RETURN(AlienbrainImporter::startMultiExport());
	}
	return MStatus(MStatus::kSuccess);
}

// ======================================================================
