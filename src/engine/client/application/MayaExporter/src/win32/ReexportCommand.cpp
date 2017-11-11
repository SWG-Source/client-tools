// ======================================================================
//
// ReexportCommand.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstMayaExporter.h"
#include "ReexportCommand.h"

#include "maya/MArgList.h"
#include "maya/MFileIO.h"
#include "maya/MGlobal.h"

#include "AlienbrainImporter.h"
#include "ExportArgs.h"
#include "ExporterLog.h"
#include "Messenger.h"
#include "PerforceImporter.h"

static Messenger *messenger;

// ======================================================================
// class ReexportCommand
// ======================================================================


void ReexportCommand::install(Messenger *newMessenger)
{
	messenger = newMessenger;
}

// ----------------------------------------------------------------------

void ReexportCommand::remove()
{
	messenger = 0;
}

// ----------------------------------------------------------------------

void *ReexportCommand::creator()
{
	return new ReexportCommand();
}

// ----------------------------------------------------------------------

/**
 * This function takes a log file, determines how to export the currently open file, and exports it.
 * The log file holds all the information needed to export the file (maya command, any parameters, etc.).
 * Since a single maya file may export many items (skeletons, meshs, anims, etc.), each has its own log
 * file that knows how to recreate that exported item.
 */
void ReexportCommand::exportFromLogFile(const std::string& logFilename, const std::string& reexportBranch, bool commitToAlienbrain)
{
	FILE* file = fopen(logFilename.c_str(), "r");
	if(!file)
	{
//@TODO this pops up often in reexportalls, but the log files in the errors appear to 1: be spurrious, and 2: be a 
//valid log file with a \n on the end (or some other bizarre ascii character)
//		MESSENGER_LOG(("%s is not a log file in ExporterLog::exportFromLogFile()\n", logFilename.c_str()));
		return;
	}
	char buffer[50000];
	memset(buffer, 0, 50000);
	IGNORE_RETURN(fread(buffer, sizeof(char), 50000, file));
	std::string bufferString = buffer;

	//now parse the buffer for the info we care about (export command and reexport options)

	const std::string sourceFilename = MFileIO::currentFile().asChar();

	//find and store the maya command
	std::string filename;
	std::string::size_type  pos = 0;
	std::string::size_type endPos = 0;
	const std::string mayaFilenameConst = "maya_filename = ";
	pos = bufferString.find(mayaFilenameConst);
	if(pos != std::string::npos) //lint !e737 !e650 std::string::npos isn't same signage as std::string's find(), sigh
	{
		pos += mayaFilenameConst.size();
		endPos = bufferString.find('\n', pos);
		if(endPos != std::string::npos) //lint !e737 !e650 std::string::npos isn't same signage as std::string's find(), sigh
		{
			filename = bufferString.substr(pos, endPos-pos);
		}
	}

	//see if the log file points to a non-z: path
	bool hadToFixUpPath = false;
	pos = filename.find("/SWG/");
	std::string prepend;
	if(pos != std::string::npos) //lint !e737 !e650 std::string::npos isn't same signage as std::string's find(), sigh
	{
		prepend = filename.substr(0, pos);
		if(_stricmp(prepend.c_str(), "z:") != 0)
		{
			IGNORE_RETURN(filename.erase(0, pos));
			filename = "z:" + filename;
			hadToFixUpPath = true;
		}
	}

	//do not try to export from a log file that did not come from this file
	//need a case-insensitive compare since the filename given might not be capitalized the same way as ours
	if(_stricmp(sourceFilename.c_str(), filename.c_str()) != 0)
	{
		fclose(file);
		return;
	}

	//now that we know we have the right log file, complain if we had to go to extra lengths to figure it out
	if(hadToFixUpPath)
		MESSENGER_LOG_WARNING(("WARNING, log file doesn't point to z:, but to %s instead, export will succeed but please fix\n", prepend.c_str()));

	pos = 0;
	endPos = 0;
	std::string mayaCommand;
	const std::string mayaCommandConst = "maya_command = ";
	pos = bufferString.find(mayaCommandConst);
	if(pos != std::string::npos) //lint !e737 !e650 std::string::npos isn't same signage as std::string's find(), sigh
	{	
		pos += mayaCommandConst.size();
		endPos = bufferString.find('\n', pos);
		if(endPos != std::string::npos) //lint !e737 !e650 std::string::npos isn't same signage as std::string's find(), sigh
		{
			mayaCommand = bufferString.substr(pos, endPos-pos);
		}
	}

//refuse to export skeletal meshes now (process is obsolete)
#if 0
	if (mayaCommand == "exportSkeletalMeshGenerator")
	{
		MESSENGER_LOG_ERROR(("Logfile %s references obsolete command exportSkeletalMeshGenerator\n"));
		return;
	}
#endif

	std::string reexportOptions;
	pos = 0;
	endPos = 0;
	const std::string mayaExportOptionsConst = "maya_export_options = ";
	pos = bufferString.find(mayaExportOptionsConst);
	if(pos != std::string::npos) //lint !e737 !e650 std::string::npos isn't same signage as std::string's find(), sigh
	{
		pos += mayaExportOptionsConst.size();
		endPos = bufferString.find('\n', pos);
		if(endPos != std::string::npos) //lint !e737 !e650 std::string::npos isn't same signage as std::string's find(), sigh
		{
			reexportOptions = bufferString.substr(pos, endPos-pos);
		}
	}

	// TPERRY - added ability to specify/override branch for reexport
	if (reexportBranch.size())
	{
		std::string tempOptions = reexportOptions;
		const std::string branchArg = "-branch ";
		const std::string nextArg = " -";
		pos = tempOptions.find(branchArg);
		if (pos != std::string::npos)
		{
			// replace branch arg from log w/reexport branch
			reexportOptions = tempOptions.substr(0, pos-1);
			reexportOptions += " " + branchArg + reexportBranch;
	
			pos = tempOptions.find(nextArg);
			if (pos != std::string::npos)
			{
				endPos = tempOptions.find('\n', pos);
				reexportOptions += tempOptions.substr(pos, endPos);
			}
		}
		else
		{
			// add reexport branch
			reexportOptions += " " + branchArg + reexportBranch;
		}
	}

	std::string melString = mayaCommand + " ";
	melString += reexportOptions;
	if(commitToAlienbrain)
	{
		melString += ExportArgs::cs_submitArgName.asChar();
	}

	//close the log file before we do the export
	fclose(file);

	IGNORE_RETURN(MGlobal::executeCommand(melString.c_str()));
	//now call the regular export process
}

// ======================================================================

bool ReexportCommand::processArguments(const MArgList &args, bool &partOfOtherExport, bool &noRevertOnFail, bool &commitToSourceControl, bool &createNewChangelist, std::string &reexportBranch)
{
	MESSENGER_INDENT;
	MStatus  status;

	const unsigned argCount = args.length(&status);
	MESSENGER_REJECT(!status, ("failed to get args length\n"));

	// always non-silent unless the silent arg is present
	messenger->endSilentExport();

	//-- handle each argument
	partOfOtherExport  = false;
	noRevertOnFail     = false;
	createNewChangelist = false;

	for (unsigned argIndex = 0; argIndex < argCount; ++argIndex)
	{
		MString argName = args.asString(argIndex, &status);
		MESSENGER_REJECT(!status, ("failed to get arg [%u] as string\n", argIndex));

		IGNORE_RETURN( argName.toLowerCase() );

		if (argName == ExportArgs::cs_partOfOtherExportArgName)
		{
			partOfOtherExport     = true;
		}
		else if (argName == ExportArgs::cs_noRevertOnFailArgName)
		{
			noRevertOnFail     = true;
		}
		else if (argName == ExportArgs::cs_submitArgName)
		{
			commitToSourceControl = true;
		}
		else if (argName == ExportArgs::cs_branchArgName)
		{
			// TPERRY - added to allow branch specification
			reexportBranch = args.asString(argIndex + 1, &status).asChar();
			// fixup argIndex
			++argIndex;
		}
		else if (argName == ExportArgs::cs_createNewChangelistArgName)
		{
			createNewChangelist = true;
		}
		else if (argName == ExportArgs::cs_showViewerAfterExport)
		{
			// don't show viewer on re-exports - ignore this argument
		}
		else
		{
			MESSENGER_LOG_ERROR(("unknown argument [%s]\n", argName.asChar()));
			return false;
		}
	}

	return true;
}

// ======================================================================

//this function always submits to alienbrain (since this function's purpose is to
//automatically update art assets when data formats change, without artists needing to
//re-export everything)
MStatus ReexportCommand::doIt(const MArgList &argList)
{
	//store all errors and warnings until the VERY end, then output them
	messenger->startMultiExport();

	bool partofotherexport     = false;
	bool noRevertOnFail        = false;
	bool commitToSourceControl = false;
	bool createNewChangelist   = false;

	std::string reexportBranch;

	IGNORE_RETURN(processArguments(argList, partofotherexport, noRevertOnFail, commitToSourceControl, createNewChangelist, reexportBranch));

	if(noRevertOnFail)
		PerforceImporter::setRevertOnFail(false);

// JU_TODO: alienbrain def out
#if 0
	AlienbrainImporter::startMultiExport();
#endif
// JU_TODO: end alienbrain def out

	//the log file(s) should be in ..\log, so find them
	const std::string sourceFilename = MFileIO::currentFile().asChar();
	if(sourceFilename == "./untitled")
	{
		MESSENGER_LOG_ERROR(("no file opened for reexport\n"));
		return MStatus(MStatus::kSuccess);
	}

	MESSENGER_LOG(("Reexporting file %s\n", sourceFilename.c_str()));

	//build the directory of the log file, so we can find and tack on all the log file for reexport
	std::string logDirectory = sourceFilename;
	std::string::size_type pos = logDirectory.find_last_of('\\');
	if (pos == std::string::npos) //lint !e737 !e650 std::string::npos isn't same signage as std::string's find_last_of(), sigh
	{
		//forward slashes, build rest of path using them
		pos = logDirectory.find_last_of('/');
		if (pos == std::string::npos) //lint !e737 !e650 std::string::npos isn't same signage as std::string's find_last_of(), sigh
		{
			//bad, can't find a '/' or a '\\'
			MESSENGER_LOG_ERROR(("malformed filename in reexport\n"));
			return MStatus(MStatus::kSuccess);
		}
		//erase the filename
		IGNORE_RETURN(logDirectory.erase(pos));
		pos = logDirectory.find_last_of('/');
		if (pos == std::string::npos)//lint !e737 !e650 std::string::npos isn't same signage as std::string's find_last_of(), sigh
		{
			//bad, there isn't another directory level
			MESSENGER_LOG_ERROR(("malformed filename in reexport\n"));
			return MStatus(MStatus::kSuccess);
		}
		//delete the \scenes directory
		IGNORE_RETURN(logDirectory.erase(pos));
		//add path to log file
		logDirectory += "/log/";
	}
	else
	{
		//success with back slashes, so build rest of path
		//erase to end
		IGNORE_RETURN(logDirectory.erase(pos));
		pos = logDirectory.find_last_of('\\');
		if (pos == std::string::npos) //lint !e737 !e650 std::string::npos isn't same signage as std::string's find_last_of(), sigh
		{
			//bad, there isn't another directory level
			MESSENGER_LOG(("malformed directory structure in reexport\n"));
			return MStatus(MStatus::kSuccess);
		}
		//delete the \scenes directory
		IGNORE_RETURN(logDirectory.erase(pos));
		//add path to log file
		logDirectory += "\\log\\";
	}

	//we'll be deleting it before we're done, so keep it on the local disk
	std::string listOfLogsFilename = "c:/listOfLogs.txt";
	std::string theCommand = "dir /b \\\"";
	theCommand += logDirectory + "\\\" > \\\"";
	theCommand += listOfLogsFilename + "\\\"";

	//put a list of all the logs in a file that we can read
	std::string melString = "system(\"";
	melString += theCommand + "\")";
	IGNORE_RETURN(MGlobal::executeCommand(melString.c_str()));

	//now open and read from that file
	FILE* logFileListHandle = fopen(listOfLogsFilename.c_str(), "r");
	if (!logFileListHandle)
		return MStatus(MStatus::kSuccess);

	char buffer[50000];
	memset(buffer, 0, 50000);
	IGNORE_RETURN(fread(buffer, sizeof(char), 50000, logFileListHandle));
	std::string bufferString = buffer;
	std::string::size_type current_pos = 0;
	std::string::size_type starting_pos = 0;
	//for each file
	while(current_pos != std::string::npos && current_pos < bufferString.max_size()) //lint !e737 !e650 std::string::npos isn't same signage as std::string's find(), sigh
	{
		current_pos = bufferString.find("\n", current_pos);
		if(current_pos != std::string::npos && current_pos < bufferString.max_size())  //lint !e737 !e650 std::string::npos isn't same signage as std::string's find(), sigh
		{
			FATAL(current_pos < starting_pos, ("negative value being used for substr"));
			std::string filename = bufferString.substr(starting_pos, current_pos-starting_pos);
			//move past newline, and save new value
			starting_pos = ++current_pos;

			//only work with log files
			if(filename.find(".log") != std::string::npos) //lint !e737 !e650 std::string::npos isn't same signage as std::string's find(), sigh
			{
				std::string logFile = logDirectory + filename;
				exportFromLogFile(logFile.c_str(), reexportBranch, commitToSourceControl);
			}
		}
	}

	//now close and delete that file
	fclose(logFileListHandle);
	melString = "system(\"del \\\"";
	melString += listOfLogsFilename + "\\\"\")";
	IGNORE_RETURN(MGlobal::executeCommand(melString.c_str()));

// JU_TODO: alienbrain def out
#if 0
	if(commitToSourceControl && !partofotherexport)
	{
		AlienbrainImporter::endMultiExport();
	}
#endif
// JU_TODO: end alienbrain def out

	if(!partofotherexport)
	{
		messenger->endMultiExport();
	}

	//print out the errors and warnings from the exports we own
	messenger->printWarningsAndErrors();
	messenger->clearWarningsAndErrors();

	return MStatus(MStatus::kSuccess);
}

// ======================================================================
