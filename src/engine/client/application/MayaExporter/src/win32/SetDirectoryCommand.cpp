// ======================================================================
//
// SetDirectoryCommand.cpp
// copyright 1999, Bootprint Entertainment
// copyright 2001, Sony Online Entertainment
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "SetDirectoryCommand.h"

#include <algorithm>
#include "maya/MArgList.h"
#include "Messenger.h"
#include <string>
#include <utility>
#include <vector>


// ======================================================================

namespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<std::string> StringVector;
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool          ms_installed;
	Messenger    *messenger;

	StringVector *ms_directoryTitles;
	StringVector *ms_directoryValues;

}

// ======================================================================

void SetDirectoryCommand::install(Messenger *newMessenger)
{
	DEBUG_FATAL(ms_installed, ("SetDirectoryCommand already installed\n"));

	ms_directoryTitles = new StringVector;
	ms_directoryValues = new StringVector;

	messenger    = newMessenger;
	ms_installed = true;
}

// ----------------------------------------------------------------------

void SetDirectoryCommand::remove()
{
	DEBUG_FATAL(!ms_installed, ("SetDirectoryCommand not installed\n"));

	delete ms_directoryTitles;
	delete ms_directoryValues;

	messenger    = 0;
	ms_installed = false;
}

// ----------------------------------------------------------------------

int SetDirectoryCommand::registerDirectory(const char *title)
{
	DEBUG_FATAL(!ms_installed, ("SetDirectoryCommand not installed\n"));

	//-- check if directory title already registered
	std::string  searchTitle(title);

	StringVector::iterator it = std::find(ms_directoryTitles->begin(), ms_directoryTitles->end(), searchTitle);
	if (it != ms_directoryTitles->end())
	{
		//-- directory title already registered.  return index for it.
		return static_cast<int>(std::distance(it, ms_directoryTitles->begin()));
	}
	else
	{
		const int newIndex = static_cast<int>(ms_directoryTitles->size());

		//-- new directory title, create it
		ms_directoryTitles->push_back(searchTitle);
		ms_directoryValues->push_back("");

		return newIndex;
	}
}

// ----------------------------------------------------------------------

int SetDirectoryCommand::getDirectoryId(const char *title)
{
	DEBUG_FATAL(!ms_installed, ("SetDirectoryCommand not installed\n"));

	std::string  searchTitle(title);
	StringVector::iterator it = std::find(ms_directoryTitles->begin(), ms_directoryTitles->end(), searchTitle);
	if (it == ms_directoryTitles->end())
		return -1;
	return static_cast<int>(std::distance(it, ms_directoryTitles->begin()));
}

// ----------------------------------------------------------------------

const char *SetDirectoryCommand::getDirectoryString(int directoryId)
{
	DEBUG_FATAL(!ms_installed, ("SetDirectoryCommand not installed\n"));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, directoryId, static_cast<int>(ms_directoryTitles->size()));

	return (*ms_directoryValues)[static_cast<size_t>(directoryId)].c_str();
}

// ----------------------------------------------------------------------

void SetDirectoryCommand::setDirectoryString(int directoryId, const char *newDirString)
{
	DEBUG_FATAL(!ms_installed, ("SetDirectoryCommand not installed\n"));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, directoryId, static_cast<int>(ms_directoryTitles->size()));

	std::string &directoryValue = (*ms_directoryValues)[static_cast<size_t>(directoryId)];
	directoryValue = newDirString;
}

// ----------------------------------------------------------------------

MStatus SetDirectoryCommand::doIt(const MArgList &argList)
{
#if 1

	UNREF(argList);

	MESSENGER_LOG_ERROR(("command not supported\n"));
	return MStatus(MStatus::kFailure);

#else

	if (argList.length() != 1)
	{
		MESSENGER_LOG_ERROR(("Command expecting one argument, user specified %u\n", argList.length()));
		return MS::kFailure;
	}

	// get new dir string
	MStatus  status;
	MString newDir = argList.asString(0, &status);
	if (!status)
	{
		MESSENGER_LOG_ERROR(("failed to interpret argument as a string\n"));
		MESSENGER_LOG_ERROR(("error = \"%s\"\n", status.errorString ().asChar()));
		return status;
	}

	// check dir string length
	const unsigned int dirLength = newDir.length();
	if (dirLength >= BufferSize)
	{
		MESSENGER_LOG_ERROR(("directory name length too long (%u), must be under %u\n", dirLength, BufferSize-1));
		return MS::kFailure;
	}

	MESSENGER_LOG(("old directory: \"%s\"\n", dirString));

	// copy the string
	strcpy (dirString, newDir.asChar ());
	if (dirString[dirLength-1] != '\\')
	{
		dirString[dirLength] = '\\';
		dirString[dirLength+1] = 0;
	}

	MESSENGER_LOG (("new directory: \"%s\"\n", dirString));

	// return success
	return MS::kSuccess;

#endif
}

// ======================================================================
