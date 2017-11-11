// ======================================================================
//
// GodClientPerforce.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "GodClientPerforce.h"

#include "GodClientPerforceUser.h"
#include "Mainframe.h"
#include "StringFilesystemTree.h"
#include "UnicodeUtils.h"
#include "sharedDebug/PerformanceTimer.h"

// ======================================================================

static const int SUBMIT_NO_FILE_ERR = 17;	// need to add file before submitting

//----------------------------------------------------------------------

const char * const GodClientPerforce::Messages::COMMAND_MESSAGE = "GodClientPerforce::Messages::COMMAND_MESSAGE";

//-----------------------------------------------------------------

GodClientPerforce::CommandMessage::CommandMessage (const std::string & msg)
: MessageDispatch::MessageBase (Messages::COMMAND_MESSAGE),
  m_msg (msg)
{
}

//----------------------------------------------------------------------

const std::string & GodClientPerforce::CommandMessage::getMessage () const
{
	return m_msg;
}

//-----------------------------------------------------------------

GodClientPerforce::GodClientPerforce () :
Singleton<GodClientPerforce> ()
{

}
//-----------------------------------------------------------------

/**
* Contatenate the subPath onto the root, stripping any extraneous slashes as necessary
*/

const std::string GodClientPerforce::concatenateSubpath (const std::string & root, const std::string & subPath)
{
	std::string result = root;

	while (!result.empty () && (result [result.size () - 1] == '/' || result [result.size () - 1] == '\\'))
		result = result.substr (0, result.size () - 1);

	if (!subPath.empty ())
	{
		result += std::string ("/") + subPath;
	}

	return result;
}
//-----------------------------------------------------------------

/**
* Check whether the local user has an open Perforce session.
*
* @return true if successful
*/
bool GodClientPerforce::isAuthenticated(std::string& result, bool attemptLogin) const
{
	GodClientPerforceUser ui;
	bool ret = ui.runCommand ("login", "-s");

	if (!ret)
	{
		if (attemptLogin)
		{
			ui.runCommand("login", 0);

			ret = isAuthenticated(result, false);
		}
		else
		{
			result = "User not logged in to Perforce, or the login session expired. Please log in and try again.\n" + ui.getLastErrorText();
		}
	}
	else
	{
		result.clear();
	}

	return ret;
}

//-----------------------------------------------------------------

/**
* Edit and lock a file in perforce.
*
* @param filename is a filename in either depot,clientspec,or local notation
* @return usually true, should be false for a failed add but is not
*/
bool GodClientPerforce::editFilesAndLock (const StringVector & filenames, std::string& result) const
{
	if (!isAuthenticated(result))
	{
		return false;
	}

	HCURSOR oldCursor = SetCursor( LoadCursor( 0, MAKEINTRESOURCE( IDC_WAIT ) ) );

	GodClientPerforceUser ui;

	bool ret = ui.runCommand ("edit", filenames);

	if (!ret)
	{
		result = "editFiles failed trying to open the file for edit.\n" + ui.getLastErrorText();
	}
	else
	{
		ret = ui.runCommand( "lock", filenames );

		if ( !ret )
		{
			result = "editFiles failed trying to lock the file.\n" + ui.getLastErrorText();
		}
	}


	SetCursor( oldCursor );

	return ret;
}

//-----------------------------------------------------------------

//-----------------------------------------------------------------

/**
* Edit a file in perforce.
*
* @param filename is a filename in either depot,clientspec,or local notation
* @return usually true, should be false for a failed add but is not
*/
bool GodClientPerforce::editFiles (const StringVector & filenames, std::string& result) const
{
	if (!isAuthenticated(result))
	{
		return false;
	}

	GodClientPerforceUser ui;
	bool ret = ui.runCommand ("edit", filenames);

	if (!ret)
	{
		result = "editFiles failed.\n" + ui.getLastErrorText();
	}

	return ret;
}

//-----------------------------------------------------------------

/**
* Revert a file in perforce.
*
* @param filename is a filename in either depot,clientspec,or local notation
* @return usually true, should be false for a failed add but is not
*/

bool GodClientPerforce::revertFiles (const StringVector & filenames, bool unchanged, std::string& result) const
{
	if (!isAuthenticated(result))
	{
		return false;
	}

	GodClientPerforceUser ui;
	bool ret = false;

	if (unchanged)
	{
		StringVector extraArgs;
		extraArgs.reserve (filenames.size () + 1);
		extraArgs.push_back ("-a");

		for (StringVector::const_iterator it = filenames.begin (); it != filenames.end (); ++it)
			extraArgs.push_back (*it);

		ret = ui.runCommand ("revert", extraArgs);
	}
	else
	{
		ret = ui.runCommand ("revert", filenames);
	}


	if (!ret)
	{
		result = "revertFiles failed.\n" + ui.getLastErrorText();
	}

	return ret;

}

//-----------------------------------------------------------------

/**
* Add some files to perforce.
*
* @param filenames are in either depot,clientspec,or local notation
* @return usually true, should be false for a failed add but is not
*/
bool GodClientPerforce::addFiles (const StringVector & filenames, std::string& result) const
{
	if (!isAuthenticated(result))
	{
		return false;
	}

	GodClientPerforceUser ui;
	bool ret = ui.runCommand ("add", filenames);

	if (!ret)
	{
		result = "addFiles failed.\n" + ui.getLastErrorText();
	}

	return ret;
}

//-----------------------------------------------------------------

/**
* Submit some files to perforce.
*
* @param filenames are in either depot,clientspec,or local notation
* @return usually true, should be false for a failed add but is not
*/

bool GodClientPerforce::submitFiles (const StringVector & filenames, std::string& result) const
{
	if (!isAuthenticated(result))
	{
		return false;
	}

	GodClientPerforceUser ui;
	bool ret = ui.runCommand ("submit", filenames);

	if (!ret)
	{
		result = "submit failed.\n" + ui.getLastErrorText();
	}

	return ret;
}

//-----------------------------------------------------------------

/**
* Perform a 'p4 where' on the selected file
* Return values are the depot path, the clientspec path, and the local filesystem path
*
* @param depot the path in the depot.  starts with //depot/
* @param clientPath the path relative to the clientspec. starts with //<clientspec>/
* @param local the path on the local filesystem
* @return true if all 3 paths are valid, false otherwise
*/
bool GodClientPerforce::getFileMapping (const std::string & path, std::string & depot, std::string & clientPath, std::string & local, std::string& result) const
{
	if (!isAuthenticated(result))
	{
		return false;
	}

	const std::string fullPath = GodClientPerforce::concatenateSubpath (path, "");
	
	std::string tmpResult;

	{
		GodClientPerforceUser ui;
		bool ret = ui.runCommand ("where", fullPath.c_str ());

		if (!ret)
		{
			result = "getFileMapping failed.\n" + ui.getLastErrorText();
			return false;
		}
		
		tmpResult = ui.getReturnValue ();
	}

	size_t start = 0;
	size_t endpos = 0;

	bool found = false;
	while(!found)
	{
		if (!Unicode::getFirstToken (tmpResult, start, endpos, depot) || endpos == tmpResult.npos) //lint !e737 implicit promotion, bug in STLPort with size_t and std::string::npos being of different signage
			break;
		//files can be mapped and/or unmapped multiple times, so make sure we aren't tracking an "unmap"
		if(depot.find("-//depot") == std::string::npos) //lint !e737 !e650 implicit promotion, bug in STLPort with size_t and std::string::npos being of different signage
			found = true;

		start = endpos + 1;
		if (!Unicode::getFirstToken (tmpResult, start, endpos, clientPath) || endpos == tmpResult.npos) //lint !e737 implicit promotion, bug in STLPort with size_t and std::string::npos being of different signage
			break;

		start = endpos + 1;
		if (!Unicode::getFirstToken (tmpResult, start, endpos, local))
			break;

		//if we have a multiple mapping, we'll have another listing begin *immediately*, without any whitespace (sigh, perforce).
		//Check for and handle this case (meaning, truncate the local string correctly, and set ourselves up for the next pass)
		size_t pos = local.find("//depot");
		if(pos != std::string::npos) //lint !e737 !e650 implicit promotion, bug in STLPort with size_t and std::string::npos being of different signage
		{
			local = local.substr(0, pos);
			//move our global start to the actual end of the local string
			start += pos;
 			endpos = 0;
		}
		else if(!found)
		{
			//in this case, we haven't found what we want, and there isn't another mapping coming up
			break;
		}
	}

	if (!found)
	{
		if (tmpResult.empty())
		{
			tmpResult = "NONE (GodClient may be using the wrong client spec)";
		}
		result = "getFileMapping: no mapping found!\nLocal Path = [" + path + "]\nPerforce Mapping = [" + tmpResult + "]";
		return false;
	}

	return true;
}
//-----------------------------------------------------------------------

void GodClientPerforce::getOpenedFiles(std::map<std::string, enum FileState> & target, std::string& result) const
{
	if (!isAuthenticated(result))
	{
		return;
	}

	GodClientPerforceUser ui;
	ui.addFilteredError(SUBMIT_NO_FILE_ERR);

	bool ret = ui.runCommand("opened", "//...");

	if (!ret)
	{
		result = "getOpenedFiles failed.\n" + ui.getLastErrorText();
		return;
	}

	std::string tmpResult = ui.getReturnValue();
	size_t start = 0;
	for(;start != tmpResult.npos && start < tmpResult.size();) //lint !e737 implicit promotion, bug in STLPort with size_t and std::string::npos being of different signage
	{
		FileState fs = FileState_not;

		const size_t hashPos = tmpResult.find_first_of("#", start);
		if(hashPos != tmpResult.npos)                               //lint !e737 implicit promotion, bug in STLPort with size_t and std::string::npos being of different signage
		{
			const std::string filePath = tmpResult.substr(start, hashPos - start);
			start = hashPos + 1;
			//-- see if it is 'add'ed or 'edit'ed.
			//-- the open type 'add' 'edit', etc.. is the 3rd token after the hash
			//-- e.g. "//depot/filename#43 - edit changelist etc..."
			size_t pendingEnd;
			std::string token;

			if (!Unicode::getNthToken (tmpResult, 2, start, pendingEnd, token) || tmpResult.npos == pendingEnd) //lint !e737 implicit promotion, bug in STLPort with size_t and std::string::npos being of different signage
			{
				return;
			}

			if (token == "add")
			{
				fs = FileState_add;
			}
			else if (token == "edit")
			{
				fs = FileState_edit;
			}
			else if (token == "delete")
			{
				fs = FileState_delete;
			}

			size_t lastSlash = filePath.rfind("/");
			std::string shortName = filePath.substr(lastSlash + 1);

			IGNORE_RETURN(target.insert(std::pair<std::string, enum FileState>(shortName, fs)));

			start = tmpResult.find     ("//", start);
			if (start == tmpResult.npos) //lint !e737 implicit promotion, bug in STLPort with size_t and std::string::npos being of different signage
			{
				break;
			}
		}
	}
}

//-----------------------------------------------------------------

/**
* Obtain a tree listing of all the files below the specified path in the depot.
* Does a p4 files or p4 opened
* 
* @param path a depot relative path.  must start with //depot/
* @param extension the filetype to search for, by extension.  may be null.
* @param pending search the client's open changelists for new (added) files
*
*/

AbstractFilesystemTree * GodClientPerforce::getFileTree (const std::string & path, const char * extension, std::string & result, FileState state) const
{

	if (!isAuthenticated(result))
	{
		return 0;
	}

	PerformanceTimer timer;
	timer.start();
	std::string rootPath = path;
	
	if (path.empty () || path [path.size () - 1] != '/')
		rootPath += "/";
	
	const std::string fullPath = rootPath + "..." + (extension ? (std::string (".") + extension) : "");

	GodClientPerforceUser ui;
	ui.addFilteredError(SUBMIT_NO_FILE_ERR);

	if (!ui.runCommand (state == FileState_depot ? "files" : "opened", fullPath.c_str ()))
	{
		result = "Error running perforce command on " + fullPath + "\n"
				+ ui.getLastErrorText();
		return 0;
	}

	std::string tmpResult = ui.getReturnValue ();

	StringFilesystemTree * sfst = new StringFilesystemTree;
	size_t start = 0;

	bool error = false;

	// rootpath may be relative, so we need to adjust it accordingly.
	// munch off any '../ components'
	while (rootPath.substr(0, 3) == "../")
		rootPath.erase(0, 3);
	rootPath = tmpResult.substr(0, tmpResult.find(rootPath)) + rootPath;
	
	for (;start != tmpResult.npos && start < tmpResult.size ();) //lint !e737 implicit promotion, bug in STLPort with size_t and std::string::npos being of different signage
	{
		if (tmpResult.compare(start, rootPath.size (), rootPath) != 0)
		{
			DEBUG_FATAL (true, ("Not good.\n"));
			error = true;
			break;
		}

		start += rootPath.size ();
		const size_t hashPos = tmpResult.find_first_of ("#", start);

		if (hashPos == tmpResult.npos) //lint !e737 implicit promotion, bug in STLPort with size_t and std::string::npos being of different signage
		{
			DEBUG_FATAL (true, ("Not good.\n"));
			error = true;
			break;
		}

		const std::string filePath = tmpResult.substr (start, hashPos - start);
		start = hashPos + 1;

		//-- the open type 'add' 'edit' 'delete', etc.. is the 3rd token after the hash
		//-- e.g. "//depot/filename#43 - edit changelist etc..."
		size_t pendingEnd;
		std::string token;

		if (!Unicode::getNthToken (tmpResult, 2, start, pendingEnd, token) || tmpResult.npos == pendingEnd) //lint !e737 implicit promotion, bug in STLPort with size_t and std::string::npos being of different signage
		{
			error = true;
			break;
		}
		start = pendingEnd + 1;

		if (error)
		{
			result = "Error parsing open file info for: " + filePath;
			break;
		}

		//if we are getting the opened files, specificially add the added and edit files
		if(state != FileState_depot)
		{
			if (state == FileState_add && token == "add")
				sfst->addFile (filePath);
			else if (state == FileState_edit && token == "edit")
				sfst->addFile (filePath);
		}
		else
		{
			//otherwise, we're getting the whole depot. But do *not* add the deleted files (which come back in a "p4 files")
			if(token != "delete")
				sfst->addFile (filePath);
		}

		start = tmpResult.find     ("//", start);
		if (start == tmpResult.npos) //lint !e737 implicit promotion, bug in STLPort with size_t and std::string::npos being of different signage
		{
			break;
		}
	}

	if (error)
	{
		delete sfst;
		sfst = 0;
		if (result.empty ())
			result = "Generic Error parsing p4 output results.";

	}

	timer.stop();
	QString qstr;
	IGNORE_RETURN(qstr.setNum(timer.getElapsedTime()));
	std::string s = std::string(qstr.latin1()) + std::string(" seconds elapsed getting the file tree");
	MainFrame::getInstance().textToConsole(s.c_str());
	return sfst;
}

//-----------------------------------------------------------------
/** @returns a list of other clients that have the given file checked out.
 *  This function requires a synchronous call to p4
*/
bool GodClientPerforce::fileAlsoOpenedBy (std::string const & path, GodClientPerforce::StringVector & alsoOpenedBy, std::string & result)
{
	result.clear();
	alsoOpenedBy.clear();

	if (!isAuthenticated(result))
	{
		return false;
	}

	std::string tmpResult;

	{
		GodClientPerforceUser ui;
		bool ret = ui.runCommand ("fstat", path.c_str ());

		if (!ret)
		{
			result = "fstat failed.\n" + ui.getLastErrorText();
			return false;
		}
		
		tmpResult = ui.getReturnValue ();
	}

	int otherOpenIndex = 0;
	char buffer[64];
	_itoa(otherOpenIndex, buffer, 10);

	static Unicode::String c_otherOpen(Unicode::narrowToWide("otherOpen"));

	Unicode::String stringToSearchFor = c_otherOpen + Unicode::narrowToWide(buffer);

	Unicode::UnicodeStringVector resultVector;
	Unicode::tokenize(Unicode::narrowToWide(tmpResult), resultVector);
	for(Unicode::UnicodeStringVector::const_iterator i = resultVector.begin(); i != resultVector.end(); ++i)
	{
		Unicode::String s = *i;
		
		if(s == stringToSearchFor)
		{
			++i;
			if(i != resultVector.end())
			{
				std::string openedBy(Unicode::wideToNarrow(*i));
				alsoOpenedBy.push_back(openedBy);
			}

			_itoa(++otherOpenIndex, buffer, 10);
			stringToSearchFor = c_otherOpen + Unicode::narrowToWide(buffer);
		}
	}

	return true;
}

// ======================================================================
