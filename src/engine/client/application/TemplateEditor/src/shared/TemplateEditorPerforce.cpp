// ============================================================================
//
// TemplateEditorPerforce.cpp
// Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TemplateEditorPerforce.h"
#include "TemplateEditorPerforceUser.h"
#include "UnicodeUtils.h"
#include "sharedDebug/PerformanceTimer.h"

static const int SUBMIT_NO_FILE_ERR = 17;	// need to add file before submitting

// ============================================================================
//
// TemplateEditorPerforce::CommandMessage
//
// ============================================================================

//-----------------------------------------------------------------------------
TemplateEditorPerforce::CommandMessage::CommandMessage(const std::string & msg)
: MessageDispatch::MessageBase (Messages::COMMAND_MESSAGE),
  m_msg (msg)
{
}

//-----------------------------------------------------------------------------
const std::string & TemplateEditorPerforce::CommandMessage::getMessage() const
{
	return m_msg;
}

// ============================================================================
//
// TemplateEditorPerforce
//
// ============================================================================

const char * const TemplateEditorPerforce::Messages::COMMAND_MESSAGE = "TemplateEditorPerforce::Messages::COMMAND_MESSAGE";

//-----------------------------------------------------------------------------
TemplateEditorPerforce::TemplateEditorPerforce()
 :Singleton<TemplateEditorPerforce>()
{
}

/**
* Edit a file in perforce.
*
* @param filename is a filename in either depot,clientspec,or local notation
* @return usually true, should be false for a failed add but is not
*/
//-----------------------------------------------------------------------------
bool TemplateEditorPerforce::editFiles(const StringVector & filenames) const
{
	TemplateEditorPerforceUser ui;
	return ui.runCommand ("edit", filenames);
}

/**
* Add some files to perforce.
*
* @param filenames are in either depot,clientspec,or local notation
* @return usually true, should be false for a failed add but is not
*/
//-----------------------------------------------------------------------------
bool TemplateEditorPerforce::addFiles(const StringVector & filenames) const
{
	TemplateEditorPerforceUser ui;
	return ui.runCommand("add", filenames);
}

//-----------------------------------------------------------------------
void TemplateEditorPerforce::getOpenedFiles(std::map<std::string, enum FileState> & target) const
{
	TemplateEditorPerforceUser ui;
	ui.addFilteredError(SUBMIT_NO_FILE_ERR);
	if(ui.runCommand("opened", "//..."))
	{
		std::string tmpResult = ui.getReturnValue();
		size_t start = 0;
		for(;start != tmpResult.npos && start < tmpResult.size();) //lint !e737 implicit promotion, bug in STLPort with size_t and std::string::npos being of different signage
		{
			FileState fs = FS_not;

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
					fs = FS_add;
				}
				else if (token == "edit")
				{
					fs = FS_edit;
				}
				else if (token == "delete")
				{
					fs = FS_delete;
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
bool TemplateEditorPerforce::getFileMapping (const std::string & path, std::string & depot, std::string & clientPath, std::string & local) const
{
	const std::string fullPath = TemplateEditorPerforce::concatenateSubpath (path, "");

	TemplateEditorPerforceUser ui;
	if (!ui.runCommand ("where", fullPath.c_str ()))
		return false;

	const std::string tmpResult = ui.getReturnValue ();

	size_t start = 0;
	size_t endpos = 0;

	bool found = false;
	while(!found)
	{
		if (!Unicode::getFirstToken (tmpResult, start, endpos, depot) || endpos == tmpResult.npos) //lint !e737 implicit promotion, bug in STLPort with size_t and std::string::npos being of different signage
			return false;
		//files can be mapped and/or unmapped multiple times, so make sure we aren't tracking an "unmap"
		if(depot.find("-//depot") == std::string::npos) //lint !e737 !e650 implicit promotion, bug in STLPort with size_t and std::string::npos being of different signage
			found = true;

		start = endpos + 1;
		if (!Unicode::getFirstToken (tmpResult, start, endpos, clientPath) || endpos == tmpResult.npos) //lint !e737 implicit promotion, bug in STLPort with size_t and std::string::npos being of different signage
			return false;

		start = endpos + 1;
		if (!Unicode::getFirstToken (tmpResult, start, endpos, local))
			return false;

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
			return false;
		}
	}
	return true;
}

/**
* Contatenate the subPath onto the root, stripping any extraneous slashes as necessary
*/

const std::string TemplateEditorPerforce::concatenateSubpath (const std::string & root, const std::string & subPath)
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

// ============================================================================
