// ======================================================================
//
// PerforceImporter.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

//precompiled header include
#include "FirstMayaExporter.h"

//module include
#include "PerforceImporter.h"

//engine shared includes
#include "sharedFoundation/Os.h"
#include "sharedFile/Treefile.h"

//local mayaExporter includes
#include "AlienbrainImporter.h"

#pragma warning (push)
#pragma warning (disable:4100) // unreferenced formal parameters abound in the perforce clientAPI
#include "ClientApi.h"
#pragma warning (pop)
#include "errornum.h"

#include "ExportArgs.h"
#include "ExportManager.h"
#include "ExporterLog.h"
#include "Messenger.h"
#include "PluginMain.h"
#include "VersionFile.h"

//system / STL includes
#include <map>
#include <set>
#include "windows.h"
#include "resource.h"

const int s_perforceErrorNotLoggedIn      = 7189;    // Perforce password (P4PASSWD) invalid or unset
const int s_perforceErrorBadPassword      = 7206;    // password invalid
const int s_perforceErrorSessionExpired   = 7480;    // Your session has expired, please login again
const int s_perforceErrorSessionLoggedOut = 7486;    // Your session was logged out, please login again

const char* s_changelistTemplate = "Change: new \nClient: %s \nUser:   %s \nStatus: new \nDescription: \n\t[exported %s] \nFiles:\n";


//////////////////////////////////////////////////////////////////////////////////

namespace PerforceImporterNamespace
{
	//flip this flag to do EVERYTHING but the actual submission.  Good for testing
	bool s_fakeSubmit            = false;
	bool s_revertAfterFakeSubmit = true;
	
	const std::string   cs_defaultComment               = "Export from mayaExporter";


    // subclass of the PerforceAPI StrBuf class, to workaround a bug
    //  in the destructor. We can't fix the bug because it's an external library
    class StrBufFixed : public StrBuf
    {
    public:
        ~StrBufFixed()
        {
            delete buffer;
            StringInit();
        }

    };
}

using namespace PerforceImporterNamespace;

//////////////////////////////////////////////////////////////////////////////////

/**
 * This class is derived from a P4API-provided class.  By overriding it I can
 * provide functionality to store return values and put values into the input stream.
 * Only the OutputInfo and InputData functions are overrides from the ClientUser class
 * the other functions are utilites functions to get/set the data it requires.
 *
 */
class PerforceImporter::ClientUserWithReturnValues : public ClientUser
{
	public:
        ClientUserWithReturnValues(void);

		void SetInputBuffer(const std::string& buffer);
        void SetMessenger(Messenger *m);
		void Reset();

		bool ErrorOccurred() const;
		int GetLastError() const;
		const std::string& GetLastErrorText() const;
		const std::string& GetReturnValue() const;

		bool GetUserCancelled() const;

	private:
        void HandleError(Error *err);
		void OutputInfo(char level, const_char *data);
		void InputData(StrBuf *buf, Error *e);
		void Prompt(const StrPtr& msg, StrBuf& rsp, int noEcho, Error* e);


	private:
		std::string m_return_value;
		std::string m_input_buffer;

		int m_lastError;
		std::string m_lastErrorText;

		bool m_userCancelled;

        Messenger* messenger;
}; //lint !e1510, base class 'ClientUser' has no destructor, we have no control over that code

//////////////////////////////////////////////////////////////////////////////////

Messenger*                                      PerforceImporter::messenger;
ClientApi*                                      PerforceImporter::ms_perforceClient = 0;
PerforceImporter::ClientUserWithReturnValues    PerforceImporter::ms_perforceUser;
bool                                            PerforceImporter::ms_installed;
bool                                            PerforceImporter::ms_connected;
std::string                                     PerforceImporter::ms_changelistNumber = "";
std::string                                     PerforceImporter::ms_comment;
bool                                            PerforceImporter::ms_revertOnFail;
std::map<std::string, std::string>              PerforceImporter::ms_fileToPerforceFileMap;
bool                                            PerforceImporter::ms_interactive;
std::set<std::string>                           PerforceImporter::ms_revertedFiles;
std::string                                     PerforceImporter::ms_branch;

//////////////////////////////////////////////////////////////////////////////////

/**
 * Constructor
 */
PerforceImporter::ClientUserWithReturnValues::ClientUserWithReturnValues(void) 
        : messenger(0)
		, m_lastError(0)
		, m_userCancelled(false)
{
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Set the messenger object to report errors to
 */
void PerforceImporter::ClientUserWithReturnValues::SetMessenger(Messenger *m)
{
    messenger = m;
}


//////////////////////////////////////////////////////////////////////////////////

/**
 * Override the ClientUser::OutputInfo function with one that stores the return value.
 */
void PerforceImporter::ClientUserWithReturnValues::OutputInfo(char level, const_char *data) 
{
	switch(level)
	{
		default:
		case '0': 
			break;

		case '1':
			m_return_value += ("... ");
			break;

		case '2':
			m_return_value += ("... ");
			break;
	}
	m_return_value += data;
} //lint !e818 data could be const, but don't change signature on overidden function


//////////////////////////////////////////////////////////////////////////////////

/**
 * Dialog proc for the prompt dialog
 */

struct PromptDialog
{
	std::string m_promptText;
	std::string m_responseText;

	bool m_noEcho;

	bool RunDialog();

	enum
	{
		OK = 2,
		CANCEL = 3
	};

};

int __stdcall DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch( uMsg )
	{

	case WM_INITDIALOG:
		{
			SetWindowLong( hwndDlg, DWL_USER, lParam );

			PromptDialog *dlg = (PromptDialog*)lParam;
			SetWindowText( GetDlgItem( hwndDlg, IDC_PROMPT ), dlg->m_promptText.c_str() );

			HWND hwndInput = GetDlgItem( hwndDlg, IDC_RESPONSE );

			if (dlg->m_noEcho)
			{
				SendMessage(hwndInput, EM_SETPASSWORDCHAR, '*', 0);
			}

			SetFocus( hwndInput );

			return FALSE;
		}
		break;

	case WM_COMMAND:
		{
			if ( LOWORD(wParam) == IDOK )
			{
				HWND hwndInput = GetDlgItem( hwndDlg, IDC_RESPONSE );
				int len = GetWindowTextLength( hwndInput );

				if (len > 0)
				{
					char* temp = new char[len + 1];
					GetWindowText( hwndInput, temp, len+1 );

					PromptDialog* dlg = (PromptDialog*) GetWindowLong( hwndDlg, DWL_USER );

					dlg->m_responseText = temp;

					delete [] temp;
				}
				EndDialog( hwndDlg, PromptDialog::OK );
				return TRUE;
			}
			else if (LOWORD(wParam) == IDCANCEL)
			{
				EndDialog( hwndDlg, PromptDialog::CANCEL );
				return FALSE;
			}
		}
		break;

	case WM_CLOSE:
		{
			EndDialog( hwndDlg, PromptDialog::CANCEL );
			return TRUE;
		}
		break;
	}

	return FALSE;
}


bool PromptDialog::RunDialog()
{
	INT_PTR ret = DialogBoxParam( GetPluginInstanceHandle(), MAKEINTRESOURCE( IDD_PERFORCE_PROMPT ), NULL, &DialogProc, (LPARAM) this );

	return ret == OK;
}


//////////////////////////////////////////////////////////////////////////////////

/**
 * Ask the user for something
 */

void PerforceImporter::ClientUserWithReturnValues::Prompt(const StrPtr& msg, StrBuf& rsp, int noEcho, Error* /*e*/)
{
	PromptDialog dlg;

	dlg.m_promptText = msg.Text();
	dlg.m_noEcho = noEcho != 0;

	if (dlg.RunDialog())
	{
		rsp.Set( dlg.m_responseText.c_str() );
		m_userCancelled = false;
	}
	else
	{
		m_userCancelled = true;
	}
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Override function to read data from "stdin", use our own buffer instead.
 */
void PerforceImporter::ClientUserWithReturnValues::InputData(StrBuf *buf, Error *e)
{
	UNREF(e);
	buf->Set(m_input_buffer.c_str());
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Clear the current return value from the internal buffer
 */
void PerforceImporter::ClientUserWithReturnValues::Reset()
{
	m_return_value.clear();
	m_lastErrorText.clear();
	m_lastError = 0;
	m_userCancelled = false;
}
//////////////////////////////////////////////////////////////////////////////////

/**
 * Set the input buffer with the passed in data.  This data will be read as if from stdin
 * when the command is run (so commands such as "p4 change -i" will expect data here)
 */
void PerforceImporter::ClientUserWithReturnValues::SetInputBuffer(const std::string& buffer)
{
	m_input_buffer = buffer;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Get the return value.
 */
const std::string& PerforceImporter::ClientUserWithReturnValues::GetReturnValue() const
{
	return m_return_value;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Did an error occur?
 */
bool PerforceImporter::ClientUserWithReturnValues::ErrorOccurred() const
{
	return m_lastError != 0;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Get the error code.
 */
int PerforceImporter::ClientUserWithReturnValues::GetLastError() const
{
	return m_lastError;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Get the return value.
 */
const std::string& PerforceImporter::ClientUserWithReturnValues::GetLastErrorText() const
{
	return m_lastErrorText;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Check the flag that says whether the user cancelled out of a prompt dialog
 */

bool PerforceImporter::ClientUserWithReturnValues::GetUserCancelled() const
{
	return m_userCancelled;
}


//////////////////////////////////////////////////////////////////////////////////

/**
 * Handle errors coming out of the perforce server.
 */
void PerforceImporter::ClientUserWithReturnValues::HandleError(Error *err)
{
    // report errors out of perforce if they are actual faults.
    // EV_EMPTY is documented in errornum.h as "action returned empty results"
    //  and "No fault at all"
    //
    if (err->GetGeneric() != EV_EMPTY)
    {
        StrBufFixed buf;

        err->Fmt( &buf );

		int errorCode = err->GetId(0)->UniqueCode();

        switch(err->GetSeverity())
        {
        case E_INFO:
            MESSENGER_LOG( ("%s (%d)", buf.Text(), errorCode) );
            break;

        case E_WARN:
            MESSENGER_LOG_WARNING( ("%s (%d)", buf.Text(), errorCode) );
            break;

        case E_FAILED:
        case E_FATAL:
			if(errorCode != s_perforceErrorNotLoggedIn 
				&& errorCode != s_perforceErrorBadPassword
				&& errorCode != s_perforceErrorSessionLoggedOut
				&& errorCode != s_perforceErrorSessionExpired) // these get logged outside handleerror in the perforce login
			{
				MESSENGER_LOG_ERROR( ("%s (%d)", buf.Text(), errorCode) );
			}
            break;

        default:
            MESSENGER_LOG_WARNING( ("HandleError() called with unexpected error level (%d).\n\t%s", err->GetSeverity(), buf.Text()) );
            break;

        }

		m_lastError = errorCode;
		m_lastErrorText = buf.Text();
    }

}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Install the module.
 */
void PerforceImporter::install(Messenger* newMessenger)
{
	messenger                      = newMessenger;
	ms_installed                   = true;
	ms_connected                   = false;
	ms_interactive                 = false;
	ms_comment                     = cs_defaultComment;
	ms_branch                      = ExportArgs::cs_currentBranch;
	ms_revertOnFail                = true;

    ms_perforceUser.SetMessenger(messenger);
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Remove the module, closing down any data structures.
 */
void PerforceImporter::remove()
{
	//disconnect if need be (shouldn't ever occur, all functions that connect should also disconnect)
	if(ms_connected)
	{
		IGNORE_RETURN(disconnectFromDepot());
	}
	ms_interactive = false;
	ms_installed = false;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Clean all data structures.
 */
void PerforceImporter::reset()
{
	FATAL(!ms_installed, ("PerforceImporter not installed\n"));
	ms_fileToPerforceFileMap.clear();
	ms_comment = cs_defaultComment;
	ms_branch  = ExportArgs::cs_currentBranch;
}

//////////////////////////////////////////////////////////////////////////////////

void PerforceImporter::setComment(const std::string& comment)
{
	FATAL(!ms_installed, ("PerforceImporter not installed\n"));
	ms_comment = comment;
	if(ms_comment.empty())
	{
		ms_comment = cs_defaultComment;
	}
}

//////////////////////////////////////////////////////////////////////////////////

bool PerforceImporter::setBranch(const std::string& branch)
{
	FATAL(!ms_installed, ("PerforceImporter not installed\n"));
	MString localBranch(branch.c_str());
	localBranch = localBranch.toLowerCase();
	//check branch name before accepting
	if(ExportManager::isValidBranch(branch))
	{
		ms_branch = localBranch.asChar();
		return true;
	}
	else
		return false;
}

//////////////////////////////////////////////////////////////////////////////////

void PerforceImporter::setRevertOnFail(bool revertOnFail)
{
	ms_revertOnFail = revertOnFail;
}

// ======================================================================

class AddFileException {};

bool PerforceImporter::addFilesToChangelist( const std::string & baseDir, const std::set<std::string> & destFiles, const std::string& subDir, bool locking, bool unlocking)
{
	std::string sourceDirectory;
	std::string filename;

	for(std::set<std::string>::iterator it = destFiles.begin(); it != destFiles.end(); ++it)
	{
		std::string const & destName = (*it);

		//get the source directory and filename

		splitDirectoryFromFilename(destName, sourceDirectory, filename);

		std::string finalPerforceLocationDir = findFinalPerforceDir(baseDir, sourceDirectory,subDir);

		if(finalPerforceLocationDir == "")
		{
			throw AddFileException();
		}

		ms_fileToPerforceFileMap[destName] = finalPerforceLocationDir;

		//the perforce \exported area won't accept files that have any uppercase letters in them, check for this case
		char buf[256];
		strcpy(buf, filename.c_str());
		IGNORE_RETURN(_strlwr(buf));
		std::string lowerFilename = buf;

		if(lowerFilename != filename)
		{
			MESSENGER_LOG_WARNING(("WARNING, file %s is not lower case, perforce will reject it\n", filename.c_str()));
			if(ms_interactive)
			{
				std::string errorMsg = "File ";
				errorMsg += filename.c_str();
				errorMsg += " is not lowercase, perforce will reject it";
				MESSENGER_MESSAGE_BOX(NULL, errorMsg.c_str(), "Error", MB_OK);
			}
		}

		bool success = false;
		if(unlocking)
		{
			success = reopenFile(finalPerforceLocationDir, sourceDirectory, filename);
		}
		else
		{
			success = editFile(finalPerforceLocationDir, sourceDirectory, filename, !locking);
		}
		if(!success)
		{
			if(ms_interactive)
			{
				std::string errorMsg = "could not add file ";
				errorMsg += destName;
				errorMsg += " to changelist";
				MESSENGER_MESSAGE_BOX(NULL, errorMsg.c_str(), "Error", MB_OK);
			}
		
			throw AddFileException();
		}
	}

	return true;
}

// ======================================================================
/**
 * Import all the destination files into perforce
 */
bool PerforceImporter::importCommon(bool interactive, bool createNewChangelist, bool lock, bool unlock)
{
	UNREF(interactive);
	FATAL(!ms_installed, ("PerforceImporter not installed\n"));

	// ----------

	MESSENGER_LOG(("====== BEGINNING IMPORT PROCESS TO PERFORCE ======\n"));

// JU_TODO: alienbrain def out
#if 0
	std::string clientDir = AlienbrainImporter::getPerforceClientPath();
	std::string sharedDir = AlienbrainImporter::getPerforceSharedPath();
	std::string serverDir = AlienbrainImporter::getPerforceServerPath();
#else
	// JU_TODO: implement
	std::string sourceFilename = ExporterLog::getSourceFilename();

	std::string clientDir = ExportManager::getPerforceClientDir(sourceFilename);
	std::string sharedDir = ExportManager::getPerforceSharedDir(sourceFilename); 
	std::string serverDir = ExportManager::getPerforceServerDir(sourceFilename);

#endif
// JU_TODO: end alienbrain def out

	//check branch name before using
	if(!ExportManager::isValidBranch(ms_branch))
	{
		DEBUG_FATAL(true, ("Bad branch name: %s found when trying to import files", ms_branch.c_str()));
		return false;
	}

	//only muck with branches if necessary
	if(ms_branch != ExportArgs::cs_currentBranch)
	{
		std::string::size_type posClient = clientDir.find(ExportArgs::cs_currentBranch);
		std::string::size_type posShared = sharedDir.find(ExportArgs::cs_currentBranch);
		std::string::size_type posServer = serverDir.find(ExportArgs::cs_currentBranch);

		//if the found cs_currentBranch in all the strings, replace them all
		if(posClient != clientDir.npos && posShared != sharedDir.npos && posServer != serverDir.npos)
		{
			clientDir.replace(posClient, ExportArgs::cs_currentBranch.size(), ms_branch);
			sharedDir.replace(posShared, ExportArgs::cs_currentBranch.size(), ms_branch);
			serverDir.replace(posServer, ExportArgs::cs_currentBranch.size(), ms_branch);
		}
		else
		{
			FATAL(true, ("Could not find [%s] to replace in one of the following strings, [%s] [%s] [%s]", ExportArgs::cs_currentBranch.c_str(), clientDir.c_str(), sharedDir.c_str(), serverDir.c_str()));
		}
	}

	if(clientDir.empty())
	{
		MESSENGER_LOG_ERROR(("Could not get the Perforce client directory from Alienbrain\n"));
		MESSENGER_LOG(("====== PERFORCE SUBMISSION FAILED ======\n"));
		return false;
	}

	if(sharedDir.empty())
	{
		MESSENGER_LOG_ERROR(("Could not get the Perforce shared directory from Alienbrain\n"));
		MESSENGER_LOG(("====== PERFORCE SUBMISSION FAILED ======\n"));
		return false;
	}

	if(serverDir.empty())
	{
		MESSENGER_LOG_ERROR(("Could not get the Perforce server directory from Alienbrain\n"));
		MESSENGER_LOG(("====== PERFORCE SUBMISSION FAILED ======\n"));
		return false;
	}

	// ----------

	if (createNewChangelist || !validateCurrentChangelist())
	{
		createChangelist();
	}

	try
	{
		//lock files, if that's the request
		if(lock)
		{
			//check out ALL files for the asset
			IGNORE_RETURN(addFilesToChangelist(getClientBase(clientDir),     ExporterLog::getClientDestinationFiles(), "",         true, false));
			IGNORE_RETURN(addFilesToChangelist(getClientBase(sharedDir),     ExporterLog::getSharedDestinationFiles(), "",         true, false));
			IGNORE_RETURN(addFilesToChangelist(getClientBase(serverDir),     ExporterLog::getServerDestinationFiles(), "server\\", true, false));

			lockOpenFiles();
		}
		else if(unlock)
		{
			//check out ALL files for the asset
			IGNORE_RETURN(addFilesToChangelist(getClientBase(clientDir),     ExporterLog::getClientDestinationFiles(), "",         false, true));
			IGNORE_RETURN(addFilesToChangelist(getClientBase(sharedDir),     ExporterLog::getSharedDestinationFiles(), "",         false, true));
			IGNORE_RETURN(addFilesToChangelist(getClientBase(serverDir),     ExporterLog::getServerDestinationFiles(), "server\\", false, true));

			unlockOpenFiles();
			revertAll();
		}
		else
		{
			IGNORE_RETURN(addFilesToChangelist(getClientBase(clientDir), ExporterLog::getClientDestinationFiles(), "",           false, false));
			IGNORE_RETURN(addFilesToChangelist(getClientBase(sharedDir), ExporterLog::getSharedDestinationFiles(), "",           false, false));
			IGNORE_RETURN(addFilesToChangelist(getClientBase(serverDir), ExporterLog::getServerDestinationFiles(), "server\\",   false, false));
		}
	}
	catch(...)
	{
		MESSENGER_LOG_ERROR(("EXCEPTION THROWN\n"));
		if(ms_revertOnFail)
			IGNORE_RETURN(revertAll());
	
		IGNORE_RETURN(disconnectFromDepot());
		MESSENGER_LOG_ERROR(("Could not add files to changelist\n"));
		MESSENGER_LOG(("====== PERFORCE SUBMISSION FAILED ======\n"));

		return false;
	}

	//if(lock || unlock)
	//{
	//	ms_changelistNumber = "";
	//}

	MESSENGER_LOG(("====== IMPORT PROCESS TO PERFORCE SUCCEEDED ======\n"));
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

void PerforceImporter::lockOpenFiles()
{
	const char *commands[2];
	commands[0] = "-c";
	commands[1] = ms_changelistNumber.c_str();
	
	runP4Command(2, commands, "lock", "Waiting on perforce to lock open files...");
}

//////////////////////////////////////////////////////////////////////////////////

void PerforceImporter::unlockOpenFiles()
{
	const char *commands[2];
	commands[0] = "-c";
	commands[1] = ms_changelistNumber.c_str();

	runP4Command(2, commands, "unlock", "Waiting on perforce to unlock open files...");
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Connect to the perforce depot.
 */
bool PerforceImporter::connectToDepot()
{
	FATAL(!ms_installed, ("PerforceImporter not installed\n"));

	DEBUG_FATAL(ms_connected, ("PerforceImporter already connected\n"));

	//don't connect if we're already connected
	if(ms_connected)
		return true;

	Error e;
	ms_perforceClient = new ClientApi;
	ms_perforceClient->Init(&e);
	ms_perforceClient->SetProg("Maya Exporter");
	MESSENGER_REJECT(e.Test(), ("Problem while trying to start Perforce connection\n"));
	ms_connected = true;
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Disconnect from the perforce depot.
 */
bool PerforceImporter::disconnectFromDepot()
{
	FATAL(!ms_installed, ("PerforceImporter not installed\n"));

	DEBUG_FATAL(!ms_connected, ("PerforceImporter not connected\n"));

	//don't disconnect if we're not connected
	if(!ms_connected)
		return true;

	Error e;
	IGNORE_RETURN(ms_perforceClient->Final(&e));
	delete ms_perforceClient;
	ms_perforceClient = 0;
	ms_connected = false;
	MESSENGER_REJECT(e.Test(), ("Problem trying to finalize Perforce connection\n"));
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Given a perforce location (i.e. //depot/swg/current/data), determine the local path,
 * and return it (i.e. c:\projects\swg\current\data).  This functions works both for
 * directories and files.
 *
 * This function uses the "p4 where" command to find the perforce location of the root location
 * of the perforce depot where we want to submit (which was pulled from a property in Alienbrain)
 *
 *
 */
std::string PerforceImporter::getClientBase(const std::string& perforceDir)
{
	FATAL(!ms_installed, ("PerforceImporter not installed\n"));

	//call "p4 where"
	const char * commands[1];
	commands[0] = perforceDir.c_str();
	
	runP4Command(1, commands, "where", "");

	//the function returns three strings, seperated by spaces, we want the third one
	std::string return_value = ms_perforceUser.GetReturnValue();
	ms_perforceUser.Reset();
	//find the first space
	std::string::size_type pos = return_value.find(" ");

	//find the second space
	pos = return_value.find(" ", pos+1);

	if(pos == static_cast<std::string::size_type>(std::string::npos))
		FATAL(true, ("Could not get the p4 client base, is the destination area of the depot [%s] mapped on your client?", perforceDir.c_str()));

	//grab from past second space to the end
	std::string clientBase = return_value.substr(pos+1); //copy to end implicit
	return clientBase;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 */
bool PerforceImporter::isChangelistEmpty()
{
	FATAL(!ms_installed, ("PerforceImporter not installed\n"));
	FATAL(ms_changelistNumber == "", ("no changelist currently open"));

	//call "p4 where"
	const char * commands[2];
	commands[0] = "-c";
	commands[1] = ms_changelistNumber.c_str();
	
	runP4Command(2, commands, "opened", "");
	std::string result = ms_perforceUser.GetReturnValue();
	if(result == "File(s) not opened on this client.")
		return true;
	else
		return false;
}

//////////////////////////////////////////////////////////////////////////////////

void PerforceImporter::runP4Command(int argc, const char* argvCommands[], const std::string& commandName, const std::string& waitingMsg)
{
	IGNORE_RETURN(connectToDepot());

	if(!waitingMsg.empty())
	{
		MESSENGER_LOG(("%s", waitingMsg.c_str()));
	}

	bool retryOp;
	do
	{
		retryOp = false;

		ms_perforceUser.Reset();
		ms_perforceClient->SetArgv(argc, const_cast<char **>(&argvCommands[0]));
		ms_perforceClient->Run(commandName.c_str(), &ms_perforceUser);

		if (ms_perforceUser.ErrorOccurred())
		{
			bool tryLogin = ms_perforceUser.GetLastError() == s_perforceErrorNotLoggedIn
						 || ms_perforceUser.GetLastError() == s_perforceErrorSessionLoggedOut
						 || ms_perforceUser.GetLastError() == s_perforceErrorSessionExpired;

			while (tryLogin)
			{
				ms_perforceUser.Reset();
				ms_perforceClient->SetArgv(0, 0);
				ms_perforceClient->Run("login", &ms_perforceUser);

				if ( !ms_perforceUser.ErrorOccurred() )
				{
					retryOp = true;
					tryLogin = false;
				}

				else if (ms_perforceUser.GetUserCancelled())
				{
					tryLogin = false;
				}

				else if (ms_perforceUser.GetLastError() != s_perforceErrorBadPassword)
				{
					tryLogin = false;
				}

				// else, try login again because they mistyped the password.
			}

			if ( ms_perforceUser.ErrorOccurred() )
			{
				char buf[256];
				snprintf(buf, 256, "%s (%d)", ms_perforceUser.GetLastErrorText().c_str(), ms_perforceUser.GetLastError());
				MESSENGER_MESSAGE_BOX(NULL, buf, "Perforce Error", MB_OK);

				if (ms_perforceUser.GetLastError() == s_perforceErrorNotLoggedIn
					|| ms_perforceUser.GetLastError() == s_perforceErrorSessionLoggedOut
					|| ms_perforceUser.GetLastError() == s_perforceErrorSessionExpired)
				{
					MESSENGER_LOG_ERROR( (buf) );
				}
			}
		}
	} while (retryOp);

	if (!waitingMsg.empty())
	{
		MESSENGER_LOG(("done.\n"));
	}

	IGNORE_RETURN(disconnectFromDepot());
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Given a client base location in the perforce depot (i.e. c:\projects\swg\current\data\exported\creature\appearance\skt),
 * and a file that we want to submit there (i.e. c:\mayaexported\appearance\skeleton\bantha.skt), first try
 * to edit the file in that location (which will fail quietly if the file doesn't exist 
 * in the depot yet).  Then copy the file to the correct Perforce location.  Then try 
 * to add the file to the depot (which will fail quietly if the file *does* exist in 
 * the depot yet).  Exactly one of the add/edit will succeed.
 *
 * @pre changelist open for add/edit/submission via createChangelist() or startMultiExport()
 */
bool PerforceImporter::editFile(const std::string& directoryInPerforceDepot, const std::string& sourceDir, const std::string& filename, bool addIfNecessary)
{
	FATAL(!ms_installed, ("PerforceImporter not installed\n"));
	FATAL(ms_changelistNumber == "", ("no changelist currently open"));

	//ensure the directory exists
	bool success =  Os::createDirectories(directoryInPerforceDepot.c_str());
	FATAL(!success, ("could not create depot directory for perforce import"));

	//build the source and dest filenames from the input params
	std::string sourceFilename = sourceDir + "\\";
	sourceFilename += filename;
	std::string destFilename = directoryInPerforceDepot + "\\";
	destFilename += filename;

	const char* argvCommands[3];

//1: sync the file

	std::string waitingStr = "Waiting on perforce to sync ";
	waitingStr += destFilename.c_str();
	waitingStr += "...";
	argvCommands[0] = "-f";
    argvCommands[1] = destFilename.c_str();
	runP4Command(2, argvCommands, "sync", waitingStr);

//2: try to edit the file
	//build the array for edit, argv[0] is the -c option, argv[1] is the changelist number, argv[3] is the filename
	argvCommands[0] = "-c";
	argvCommands[1] = ms_changelistNumber.c_str();
	argvCommands[2] = destFilename.c_str();
	waitingStr = "Waiting on perforce to edit ";
	waitingStr += destFilename.c_str();
	waitingStr += "...";
	runP4Command(3, argvCommands, "edit", waitingStr);

	//copy the file to the correct location for perforce to handle
	int returnVal = CopyFile(sourceFilename.c_str(), destFilename.c_str(), false);

	if(!returnVal)
	{
		std::string errorStr = "\"";
		errorStr += sourceFilename + "\" could not be copied to \"";
		errorStr += destFilename + "\", is the destinantion file read-only?\nAnother possibility is that you do not have permission to edit this file.";
		if(ms_interactive)
		{
			MESSENGER_MESSAGE_BOX(NULL, errorStr.c_str(), "Error", MB_OK);
		}
		MESSENGER_LOG_ERROR(("%s\n", errorStr.c_str()));
		return false;
	}

//4: optionally try to add the file
	if(addIfNecessary)
	{
		//build the array for add, argv[0] is the -c option, argv[1] is the changelist number, argv[3] is the filename
		argvCommands[0] = "-c";
		argvCommands[1] = ms_changelistNumber.c_str();
		argvCommands[2] = destFilename.c_str();

		runP4Command(3, argvCommands, "add", "");
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

bool PerforceImporter::reopenFile(const std::string& directoryInPerforceDepot, const std::string& sourceDir, const std::string& filename)
{
	FATAL(!ms_installed, ("PerforceImporter not installed\n"));
	FATAL(ms_changelistNumber == "", ("no changelist currently open"));

	//ensure the directory exists
	bool success =  Os::createDirectories(directoryInPerforceDepot.c_str());
	FATAL(!success, ("could not create depot directory for perforce import"));

	//build the source and dest filenames from the input params
	std::string sourceFilename = sourceDir + "\\";
	sourceFilename += filename;
	std::string destFilename = directoryInPerforceDepot + "\\";
	destFilename += filename;

	const char* argvCommands[3];

	//build the array for edit, argv[0] is the -c option, argv[1] is the changelist number, argv[3] is the filename
	argvCommands[0] = "-c";
	argvCommands[1] = ms_changelistNumber.c_str();
	argvCommands[2] = destFilename.c_str();
	std::string waitingStr = "Waiting on perforce to reopen ";
	waitingStr += destFilename.c_str();
	waitingStr += "...";
	runP4Command(3, argvCommands, "reopen", waitingStr);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * It reverts unchanged files.
 *
 * @pre changelist open for add/edit/submission via createChangelist() or startMultiExport()
 */
void PerforceImporter::revertMostUnchangedFiles()
{
	FATAL(!ms_installed, ("PerforceImporter not installed\n"));
	FATAL(ms_changelistNumber == "", ("no changelist currently open"));

	//get the changelist spec so we can examine the filelist and revert the correct files
	const char * argvCommands1[2];
	argvCommands1[0] = "-o";
	argvCommands1[1] = ms_changelistNumber.c_str();
	runP4Command(2, argvCommands1, "change", "");
	std::string changeListSpec = ms_perforceUser.GetReturnValue();

	std::string::size_type start_pos = 0;
	std::string::size_type pos       = changeListSpec.find("\n", start_pos);
	std::string line;
	std::string revertList;
	std::string extension;

	//examine each line looking for a depot add or edit, revert unchanged on it unless it is in the special list (see func comment)
	while(pos != changeListSpec.npos)
	{
		line = changeListSpec.substr(start_pos, pos-start_pos);
		start_pos = pos;

		//is this line a depot add or edit?
		if(line.find("//depot") != line.npos && (line.find("edit") != line.npos || line.find("add") != line.npos))
		{
			//revert the file
			std::string::size_type pos3 = line.find("\t");
			while(pos3 != line.npos)
			{
				line.erase(pos3, 1);
				pos3 = line.find("\t");
			}

			//remove the non-file part on the end
			std::string::size_type p = line.find("#");
			line = line.substr(0, p);
			revertList += line.substr(0, line.size());

			//to revert a file, it must be ONE file with no whitespace around it (unlike the command line which accepts multiple files, whitespace, etc. sigh)
			const char * commands[4];
			commands[0] = "-a";
			commands[1] = "-c";
			commands[2] = ms_changelistNumber.c_str();
			commands[3] = line.c_str();
			runP4Command(4, commands, "revert", "");
		}
		pos  = changeListSpec.find("\n", ++start_pos);
	}
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Given a fully-pathed filename, split it into the filename and its fully-pathed directory.
 * The filename and directory are returned via references.
 */
void PerforceImporter::splitDirectoryFromFilename(const std::string& directoryAndFilename, std::string& directory, std::string& filename)
{
	FATAL(!ms_installed, ("PerforceImporter not installed\n"));
	std::string::size_type pos = directoryAndFilename.find_last_of("\\/");
	if(pos == static_cast<std::string::size_type>(std::string::npos))
		return;

	filename = directoryAndFilename.substr(pos+1); //copy to end implicit
	directory = directoryAndFilename.substr(0, pos);
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Using the fully-pathed filename, determine the final perforce path to it
 * This code places files in the perforce depot in the same relative location
 * 
 * @return the final perforce location (i.e. //depot/swg/current/data/<etc>/exported/appearance/mesh/human_male.mgn), or "" on failure
 */
std::string PerforceImporter::findFinalPerforceDir(const std::string& clientBase, const std::string& sourceDir, const std::string& subDir)
{
	FATAL(!ms_installed, ("PerforceImporter not installed\n"));

	std::string finalPerforcePath = clientBase + "\\";

	std::string baseDir = ExporterLog::getBaseDir();

	if(!subDir.empty())
	{
		baseDir += subDir;
	}

	std::string::size_type pos = sourceDir.find(baseDir);
	if(pos == static_cast<std::string::size_type>(std::string::npos))
	{
		if(ms_interactive)
		{
			std::string errorStr = baseDir + " was not found in ";
			errorStr += sourceDir + " reasons could be:\n 1: The file is not in the Alienbrain database\n 2: The file does not have a mapping into Perforce";
			MESSENGER_MESSAGE_BOX(NULL, errorStr.c_str(), "Error", MB_OK);
		}
		return "";
	}
	finalPerforcePath += sourceDir.substr(pos+baseDir.size());
	return finalPerforcePath;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Do the actual submission.  Since we have our own changelist number, get that changelist spec
 * (via "p4 change -o [number]"), and then call "p4 submit -i" with that spec in the input buffer
 *
 * @pre changelist open for add/edit/submission via createChangelist() or startMultiExport()
 * @post currently open changelist has either been submitted or reverted (if all files were unchanged)
 */
bool PerforceImporter::doSubmission()
{
	FATAL(!ms_installed, ("PerforceImporter not installed\n"));
	FATAL(ms_changelistNumber == "", ("no changelist currently open"));

	getRevertedFileList();
	revertMostUnchangedFiles();
	addRevertedFileListToComment();

	//get the changelist spec
	const char * argvCommands1[2];
	argvCommands1[0] = "-o";
	argvCommands1[1] = ms_changelistNumber.c_str();

	runP4Command(2, argvCommands1, "change", "");

	std::string changeListSpec = ms_perforceUser.GetReturnValue();
	ms_perforceUser.Reset();

	//set the return value as the input for the next call (which will be a submit -i)
	ms_perforceUser.SetInputBuffer(changeListSpec);

	if(!s_fakeSubmit)
	{
		//now do the submission, with the changelist spec coming from the inputbuffer
		const char *argvCommands2[1];
		argvCommands2[0] = "-i";
		ms_perforceUser.Reset();

		runP4Command(1, argvCommands2, "submit", "Waiting on perforce to submit the changelist...");

		std::string returnValue = ms_perforceUser.GetReturnValue();

		//the only other non-error return values are "Changlist <changelist> submitted." and "Submitting change <changelist>."
		if(returnValue.find("submitted") == std::string::npos
		&& returnValue.find("Submitting change") == std::string::npos) //lint !e650 !e737
		{
			std::string errorMsg = "Error submitting to P4, [";
			errorMsg += returnValue;
			errorMsg += "]\n";
			if (ms_interactive)
			{
				MESSENGER_MESSAGE_BOX(NULL, errorMsg.c_str(), "Error", MB_OK);
			}
			MESSENGER_LOG_ERROR(("%s\n", errorMsg.c_str()));
		}
	}
	//else we faked the submit, should we also revert the files back
	else
	{	
		MESSENGER_LOG(("FAKING SUBMISSION!!!! (not actually submitting files to perforce)...\n"));
		if(s_revertAfterFakeSubmit)
		{
			MESSENGER_LOG(("Reverting files after fake submissions...\n"));
			revertAll();
		}
	}

	//now try to delete empty changelist, if we don't have any files to submit
	const char *argvCommands3[2];
	argvCommands3[0] = "-d";
	argvCommands3[1] = ms_changelistNumber.c_str();

	runP4Command(2, argvCommands3, "change", "");

	//this changelist has been completed (either submitted or reverted), so remove referenes to it
	ms_changelistNumber = "";
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Determines whether the current changelist is still open.
 * The current changelist number is reset if the changelist is no longer valid.
 *
 */
bool PerforceImporter::validateCurrentChangelist()
{
	FATAL(!ms_installed, ("PerforceImporter not installed\n"));

	bool returnValue = false;

	if (ms_changelistNumber != "")
	{
		//call "p4 desribe"
		const char * args[1];
		args[0] = ms_changelistNumber.c_str();
		
		runP4Command(1, args, "describe", "");
		std::string result = ms_perforceUser.GetReturnValue();

		if(result.empty() || result.find("no such changelist") != std::string::npos)
		{
			// changelist is no longer open
			MESSENGER_LOG(("Changelist %s no longer open. Creating new changelist.\n", ms_changelistNumber.c_str()));
			ms_changelistNumber = "";
			returnValue = false;
		}
		else
		{
			// changelist is still open
			returnValue = true;
		}
	}

	return returnValue;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Create a new perforce changelist, the description will be the same as the comment for the
 * AlienbrainImporter submissions.  The changelist number is stored for future use.
 *
 * @post a changelist has been created and is ready for files to be edited/added and submitted
 */
void PerforceImporter::createChangelist()
{
	FATAL(!ms_installed, ("PerforceImporter not installed\n"));

	char changelistSpec[500];

	connectToDepot();
	sprintf(changelistSpec, s_changelistTemplate, ms_perforceClient->GetClient().Text(), ms_perforceClient->GetUser().Text(), MAYA_EXPORTER_VERSION );
	disconnectFromDepot();

	//submit to perforce to generate a new changelist
	const char *argvCommands2[1];
	argvCommands2[0] = "-i";

	ms_perforceUser.SetInputBuffer(changelistSpec);
	runP4Command(1, argvCommands2, "change", "Waiting on perforce to create the changelist...");

	//the function returns a new changelist number in the form "Change 9902 created", get the number
	std::string changeListNumberReturnValue = ms_perforceUser.GetReturnValue();
	ms_perforceUser.Reset();
	int posStart = changeListNumberReturnValue.find(" ");
	int posEnd = changeListNumberReturnValue.find(" ", posStart+1);
	ms_changelistNumber = changeListNumberReturnValue.substr(posStart+1, (posEnd-posStart)-1);
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * This function reverts all files in the currently open changelist, and then deletes that changelist.
 * This function is really only used when an error occurs and we don't want to submit the files, 
 * but we also don't want to leave them checked out.
 *
 * @pre changelist open for add/edit/submission via createChangelist() or startMultiExport()
 * @post currently open changelist and all its files have been reverted
 */
bool PerforceImporter::revertAll()
{
	FATAL(!ms_installed, ("PerforceImporter not installed\n"));
	FATAL(ms_changelistNumber == "", ("no changelist currently open"));

	const char *commands[3];
	commands[0] = "-c";
	commands[1] = ms_changelistNumber.c_str();
	commands[2] = "//...";

	runP4Command(3, commands, "revert", "Waiting on perforce to revert all files...");

	//now delete empty changelist
	const char *commands2[2];
	commands2[0] = "-d";
	commands2[1] = ms_changelistNumber.c_str();

	runP4Command(2, commands2, "change", "Waiting on perforce to delete the changelist...");

	ms_changelistNumber = "";
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * For a given perforce filename (i.e. "//depot/swg/current/.../something.pob"), get the local file.
 */
std::string PerforceImporter::findFileOnDisk(const std::string& perforcePath)
{
	const char *commands[2];

	// call "p4 files"
	commands[0] = perforcePath.c_str();
	runP4Command(1, commands, "files", "Waiting on perforce to return a \"files\" command...");

	std::string return_value = ms_perforceUser.GetReturnValue();
	ms_perforceUser.Reset();

	std::string::size_type pos = return_value.find("#");

	FATAL(pos == static_cast<std::string::size_type>(std::string::npos),
		("Could not get the filename, is the destination area of the depot [%s] mapped on your client?", perforcePath.c_str())
		);

	std::string depotLoc = return_value.substr(0, pos);

	// call "p4 where"
	commands[0] = depotLoc.c_str();
	runP4Command(1, commands, "where", "Waiting on perforce to return a \"where\" command...");

	// the function returns three strings, separated by spaces, we want the third one
	return_value = ms_perforceUser.GetReturnValue();
	ms_perforceUser.Reset();

	// find the first space
	pos = return_value.find(" ");

	// next space
	pos = return_value.find(" ", pos + 1);

	FATAL(pos == static_cast<std::string::size_type>(std::string::npos),
		("Could not get the filename, is the destination area of the depot [%s] mapped on your client?", perforcePath.c_str())
		);

	std::string filename = return_value.substr(pos + 1);

	return filename;
}

//////////////////////////////////////////////////////////////////////////////////

/**
* For a given filename (i.e. "c:\work\swg\current\data\sku.0\sys.client\exported\appearance\something.pob"), revert it if it's checked out.
*/
bool PerforceImporter::revertFile(const std::string& filename)
{
	const char *commands[2];

	// call "p4 revert"
	commands[0] = filename.c_str();
	runP4Command(1, commands, "revert", "Waiting on perforce to execute a \"revert\" command...");

	std::string return_value = ms_perforceUser.GetReturnValue();
	ms_perforceUser.Reset();

	return return_value.find(" reverted") != std::string::npos;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * For a given filename (i.e. "c:\projects\swg\...\file.sht"), get the perforce-pathed file
 * (i.e. //depot/swg/.../file.sht")
 */
std::string PerforceImporter::getPerforcePath(const std::string& filename)
{
	std::string fileLoc = ms_fileToPerforceFileMap[filename];

	//call "p4 where"
	const char *commands[2];
	commands[0] = fileLoc.c_str();

	runP4Command(1, commands, "where", "Waiting on perforce to return a \"where\" command...");

	//the function returns three strings, seperated by spaces, we want the third one
	std::string return_value = ms_perforceUser.GetReturnValue();
	ms_perforceUser.Reset();
	//find the first space
	std::string::size_type pos = return_value.find(" ");

	std::string depotLoc = return_value.substr(0, pos);
	return depotLoc;
}

//////////////////////////////////////////////////////////////////////////////////

/** Determine what files would be revert-unchanged
 */
void PerforceImporter::getRevertedFileList()
{
	//build a command to preview what file would be revert-unchanged
	const char * commands[5];
	commands[0] = "-a";
	commands[1] = "-n";
	commands[2] = "-c";
	commands[3] = ms_changelistNumber.c_str();
	commands[4] = "//...";
	runP4Command(4, commands, "revert", "");

	std::string returnValue = ms_perforceUser.GetReturnValue();

	ms_revertedFiles.clear();
	size_t pos = returnValue.find("//depot");
	size_t endPos = 0;
	std::string file;
	//add each file to the static vector
	while(pos != returnValue.npos)
	{
		endPos = returnValue.find(" ", pos);
		file = returnValue.substr(pos, endPos-pos);
		ms_revertedFiles.insert(file);
		pos = returnValue.find("//depot", pos+1);
	}
}

//////////////////////////////////////////////////////////////////////////////////

/** Edit the changelist spec to include the list of all the files that have been revert-unchanged
 *  @pre ms_revertedFiles has been prepopulated with the list of reverted files
 *  @pre there is a currently open changelist
 *  @post the changelist spec has been updated with the reverted files injected into the comment
 */
void PerforceImporter::addRevertedFileListToComment()
{
	if(ms_revertedFiles.empty())
		return;
	
	//get the current changelist spec
	const char * argvCommands[2];
	argvCommands[0] = "-o";
	argvCommands[1] = ms_changelistNumber.c_str();
	runP4Command(2, argvCommands, "change", "Getting changelist spec to add reverted file information...");

	std::string changeListSpec = ms_perforceUser.GetReturnValue();
	ms_perforceUser.Reset();

	//change the comment in the changelist spec
	std::string::size_type posStart = changeListSpec.find("Files:");

	std::string specStart = changeListSpec.substr(0, posStart);

	//for each file, add a line to the comment of the form "~ <filename>"
	specStart += "\n\t";
	for(std::set<std::string>::iterator i = ms_revertedFiles.begin(); i != ms_revertedFiles.end(); ++i)
	{
		specStart += "~ ";
		specStart += *i + "\n\t";
	}

	//add the remainder of the clientspec back
	std::string specEnd = changeListSpec.substr(posStart);
	specStart += specEnd;

	//submit the new changelist spec
	const char *argvCommands2[1];
	argvCommands2[0] = "-i";
	ms_perforceUser.SetInputBuffer(specStart);
	runP4Command(1, argvCommands2, "change", "Adding reverted file information to the clientspec...");
}

//////////////////////////////////////////////////////////////////////////////////
