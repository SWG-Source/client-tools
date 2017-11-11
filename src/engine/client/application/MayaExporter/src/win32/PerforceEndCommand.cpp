// ======================================================================
//
// PerforceEndCommand.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstMayaExporter.h"
#include "PerforceEndCommand.h"

#include "AlienbrainImporter.h"
#include "ExportArgs.h"
#include "PerforceImporter.h"
#include "maya/MArgList.h"
#include "Messenger.h"

static Messenger *messenger;

// ======================================================================
// class PerforceEndCommand
// ======================================================================


void PerforceEndCommand::install(Messenger *newMessenger)
{
	messenger = newMessenger;
}

// ----------------------------------------------------------------------

void PerforceEndCommand::remove()
{
	messenger = 0;
}

// ----------------------------------------------------------------------

void *PerforceEndCommand::creator()
{
	return new PerforceEndCommand();
}

// ----------------------------------------------------------------------

bool PerforceEndCommand::processArguments(const MArgList &args, bool *interactive)
{
	MESSENGER_INDENT;
	MStatus  status;

	const unsigned argCount = args.length(&status);
	MESSENGER_REJECT(!status, ("failed to get args length\n"));

	//-- handle each argument
	*interactive             = false;

	// NOTE: interactive or autoexport argument needs to come first

	for (unsigned argIndex = 0; argIndex < argCount; ++argIndex)
	{
		MString argName = args.asString(argIndex, &status);
		MESSENGER_REJECT(!status, ("failed to get arg [%u] as string\n", argIndex));

		IGNORE_RETURN( argName.toLowerCase() );

		if (argName == ExportArgs::cs_interactiveArgName)
		{
			*interactive   = true;
		}
		else
		{
			MESSENGER_LOG_ERROR(("unknown argument [%s]\n", argName.asChar()));
			return false;
		}
	}
	return true;
}

// ----------------------------------------------------------------------

MStatus PerforceEndCommand::doIt(const MArgList &argList)
{
	bool interactive = false;
	IGNORE_RETURN(processArguments(argList, &interactive));

	IGNORE_RETURN(AlienbrainImporter::endMultiExport());
	PerforceImporter::endMultiExport(interactive);

	return MStatus(MStatus::kSuccess);
}

// ======================================================================
