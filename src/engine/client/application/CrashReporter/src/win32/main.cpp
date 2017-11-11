// main.cpp : Defines the entry point for the console application.
//

#include "FirstCrashReporter.h"

#include "blat.h"

#include "resource.h"

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <time.h>

#include <lmcons.h>


//=========================================================

const char *s_processId = 0;
const char *s_exeName = 0;
const char *s_eventName = 0;
const char *s_smtpServer = 0;
const char *s_smtpPort = 0;
const char *s_emailFrom = 0;
const char *s_emailTo = 0;

std::vector<std::string> s_fileNameList;

char s_promptText[1024];

char s_userName[UNLEN + 1];
char s_computerName[MAX_COMPUTERNAME_LENGTH + 1];

char s_inputText[8192];


//=========================================================

void fillFileList( );
std::string getMinidump (std::string const & fileName);
std::string getLog (std::string const & fileName);
std::string getBacktrace (std::string const & fileName);
std::string getSubject (std::string const & baseName);
std::string getBody (std::string const & baseName, const char *extraText);
std::string getCode (std::string const & baseName);
void sendMail (std::string const & subject, std::string const & body, std::vector<std::string> const & attachments);
bool sendCrashReport ( const char *bodyText );
bool detectCrashReport ( );
void clearCrashReport ();
BOOL CALLBACK DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM /*lParam*/);
void doCheck(UINT stringId);
void usage(const char *msg);
bool processCommandLine(int argc, char* argv[]);

//=========================================================

int main(int argc, char* argv[])
{
	int ret = -1;

	DWORD processId = 0;
	HANDLE process = NULL;

	if (processCommandLine(argc, argv))
    {
	    processId = atoi(s_processId);
	    process = OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, processId );

	    if (process != NULL)
	    {
		    DWORD size;
		    size = sizeof(s_userName);
		    GetUserName(s_userName, &size);

		    size = sizeof(s_computerName);
		    GetComputerName(s_computerName, &size);

		    // Check for previously-existing core files. There probably aren't any, unless the
		    // program crashed at some point before the watcher was running.
		    doCheck(IDS_PREVIOUS_RUN);

		    HANDLE event = OpenEvent(EVENT_ALL_ACCESS, false, s_eventName );

		    // Tell the parent process we're good to go, then
		    // wait for the parent process to exit. We will return from this on successful exit
		    // as well as on crash, even if the process were stopped in the debugger. The
		    // only time we don't return is if we are killed by someone.
		    if (event != NULL)
		    {
			    SignalObjectAndWait(event, process, INFINITE, false);
			    CloseHandle(event);

			    doCheck(IDS_CURRENT_RUN);

			    CloseHandle(process);
			    process = NULL;

			    ret = 0;
		    }
            else
            {
                usage("Event not found");
            }
	    }
        else
        {
            usage("Process not found");
        }
    }

	return ret;
}






//=========================================================

void fillFileList( )
{
	//-- crash reports typically generate a mdmp file and a txt file. We look
	//-- for the presence of the mdmp file andsend it as an attachment. If we
	//-- have a text file, it forms the body of the message.
    s_fileNameList.clear();

	WIN32_FIND_DATA findData;
	HANDLE hFind;

	std::string fileNameMask(s_exeName);
	fileNameMask += "-*.mdmp";
	hFind = FindFirstFile( fileNameMask.c_str(), &findData );

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if ( !(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				&& (findData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
				)
			{
				std::string fileName = findData.cFileName;
				std::string::size_type const index = fileName.find_last_of('.');
				fileName = fileName.substr(0, index);
				s_fileNameList.push_back (fileName);
			}
		} while (FindNextFile(hFind, &findData));

		FindClose(hFind);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::string getMinidump (std::string const & fileName)
{
	std::string result(fileName + ".mdmp");

	std::ifstream file(result.c_str());
	if (file.fail())
	{
		result = "";
	}

	return result;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::string getLog (std::string const & fileName)
{
	std::string result(fileName + ".log");

	std::ifstream file(result.c_str());
	if (file.fail())
	{
		result = "";
	}

	return result;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::string getBacktrace (std::string const & fileName)
{
	std::string result(fileName + ".txt");

	std::ifstream file(result.c_str());
	if (file.fail())
	{
		result = "";
	}

	return result;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::string getSubject (std::string const & baseName)
{
	return "automated crash dump " + baseName;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::string getBody (std::string const & baseName, const char *extraText)
{
	std::stringstream body;

	std::string fileName = getBacktrace( baseName );

	if (!fileName.empty())
	{
		std::ifstream file(fileName.c_str());
		if (!file.fail())
		{
			file >> body.rdbuf();
		}
	}
	else
	{
		body << "automated crash dump" << std::endl << std::endl 
			<< "unknown: FATAL 00000000: minidump sent via CrashReporter" << std::endl
            << "Backtrace for " << baseName << " not found." << std::endl;
	}


    body << std::endl << "------------------------------------" << std::endl 
        << "UserName: " << s_userName << std::endl
        << "ComputerName: " << s_computerName << std::endl;



	if (extraText != 0)
	{
		body << std::endl << "------------------------------------" << std::endl << extraText << std::endl;
	}

	
	return body.str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::string getCode (std::string const & baseName)
{
	std::string body;

	std::string fileName = getBacktrace( baseName );

	if (!fileName.empty())
	{
		std::ifstream file(fileName.c_str());
		if (!file.fail())
		{
			const int bufSize = 1024;
			char buf[bufSize];

			while (!file.eof())
			{
				file.getline(buf, bufSize);

				if (strstr(buf, "Exception") || strstr(buf, "FATAL"))
				{
					body = buf;
					break;
				}
			}
		}
	}

	if (body.empty())
	{
		body = "unknown";
	}

	return body;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void sendMail (std::string const & subject, std::string const & body, std::vector<std::string> const & attachments)
{
	int const numAttachments = attachments.size();

	const int static_args = 13;

	int argc = static_args + (2 * numAttachments);
	char** argv = new char*[argc];
	int argv_value = 0;
	argv[++argv_value] = "-to";
	argv[++argv_value] = const_cast<char*>(s_emailTo);
	argv[++argv_value] = "-subject";
	argv[++argv_value] = const_cast<char*>(subject.c_str());
	argv[++argv_value] = "-smtphost";
	argv[++argv_value] = const_cast<char*>(s_smtpServer);
	argv[++argv_value] = "-port";
	argv[++argv_value] = const_cast<char*>(s_smtpPort);
	argv[++argv_value] = "-f";
	argv[++argv_value] = const_cast<char*>(s_emailFrom);
	argv[++argv_value] = "-body";
	argv[++argv_value] = const_cast<char*>(body.c_str());

	//iterate through all attachments, building the argv entries for them
	for(std::vector<std::string>::const_iterator i = attachments.begin(); i != attachments.end(); ++i)
	{
		argv[++argv_value] = "-attach";
		argv[++argv_value] = const_cast<char*>(i->c_str());
	}

	//this doesn't get used, but make sure to send initialized memory anyway
	char envp[256];
	memset(envp, 0, sizeof(envp));

	//call into blat to send the mail
    char *ptmp = envp;
	callBlat(argc, argv, &ptmp);

	//clean up allocated memory
	delete[] argv;
}


// ======================================================================

bool sendCrashReport ( const char *bodyText )
{
	bool ret = false;

	//-- crash reports typically generate a mdmp file and a txt file. We look
	//-- for the presence of the mdmp file andsend it as an attachment. If we
	//-- have a text file, it forms the body of the message.

	if (!s_fileNameList.empty ())
	{
		//-- send one email per dump found
		//-- log the files and then remove them

		std::ofstream logfile("minidump.log", std::ios_base::out | std::ios_base::app);

		if (!logfile.fail())
		{
			//-- write the header
			time_t timeNow = time(0);
			std::string const header = asctime(localtime(&timeNow));
			
			logfile << header << std::endl;
		}


		std::vector<std::string>::const_iterator iter;
		for (iter = s_fileNameList.begin(); iter != s_fileNameList.end(); ++iter)
		{
			std::vector<std::string> attachments;
			
			std::string const dumpName = getMinidump ( *iter );
			if (!dumpName.empty())
			{
				attachments.push_back (dumpName);
			}

			std::string const logName = getLog ( *iter );
			if (!logName.empty())
			{
				attachments.push_back (logName);
			}
			std::string const backtraceName = getBacktrace( *iter );

			std::string subject = getSubject(*iter);
			std::string body = getBody(*iter, bodyText);

			sendMail (subject, body, attachments);

			if (!logfile.fail())
			{
				logfile << "sent    ";
				//-- write the file name and the exception or fatal string
				logfile << backtraceName << ' ' << getCode (*iter) << std::endl;
			}

			//-- delete the files
			::remove (backtraceName.c_str());
			::remove (dumpName.c_str());
		}

		logfile << std::endl;

		ret = true;
	}

	return ret;
}

// ======================================================================

bool detectCrashReport ( )
{
	bool ret = false;

	fillFileList( );

	ret = s_fileNameList.size() > 0;

	return ret;
}

// ======================================================================

void clearCrashReport ()
{
	if (!s_fileNameList.empty ())
	{
		std::ofstream logfile("minidump.log", std::ios_base::out | std::ios_base::app);

		if (!logfile.fail())
		{
			//-- write the header
			time_t timeNow = time(0);
			std::string const header = asctime(localtime(&timeNow));
			
			logfile << header << std::endl;
		}


		std::vector<std::string>::const_iterator iter;
		for (iter = s_fileNameList.begin(); iter != s_fileNameList.end(); ++iter)
		{
			std::string const dumpName = getMinidump ( *iter );
			std::string const logName = getLog ( *iter );
			std::string const backtraceName = getBacktrace( *iter );

			if (!logfile.fail())
			{
				logfile << "cleared ";
				//-- write the file name and the exception or fatal string
				logfile << backtraceName << ' ' << getCode (*iter) << std::endl;
			}

			//-- delete the files
			::remove (backtraceName.c_str());
			::remove (logName.c_str());
			::remove (dumpName.c_str());
		}

		logfile << std::endl;
	}
}

// ======================================================================

BOOL CALLBACK DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM /*lParam*/) 
{ 

    switch (message) 
    { 
	case WM_INITDIALOG:
		{
			HWND item = GetDlgItem(hwndDlg, IDC_PROMPT);
			SetWindowText(item, s_promptText);
			
			item = GetDlgItem(hwndDlg, IDC_USERNAME);
			SetWindowText(item, s_userName);

			item = GetDlgItem(hwndDlg, IDC_COMPUTERNAME);
			SetWindowText(item, s_computerName);
		}
		return TRUE;	// make sure the focus gets set.
		break;
	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) 
			{ 
				case IDOK: 
					{
						HWND item = GetDlgItem( hwndDlg, IDC_BUG_REPORT );
						GetWindowText(item, s_inputText, sizeof(s_inputText)) ;
					}
					// Fall through. 

				case IDCANCEL: 
					EndDialog(hwndDlg, wParam); 
					return TRUE; 
			} 
		}
    } 

    return FALSE; 
}

// ======================================================================

void doCheck(UINT stringId)
{
	if ( detectCrashReport( ) )
	{
		char tmpString[1024];
		LoadString(GetModuleHandle(NULL), stringId, tmpString, sizeof(tmpString));

		_snprintf(s_promptText, sizeof(s_promptText), tmpString, s_exeName);

		if (DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_REPORT_DIALOG), NULL, (DLGPROC)DlgProc) == IDOK)
		{
			sendCrashReport( s_inputText );
		}
		else
		{
			clearCrashReport( );
		}
	}
}

// ======================================================================

void usage(const char *msg)
{
	char usageMsg[4096];

	_snprintf(usageMsg, sizeof(usageMsg), "%s\n\n"
                                          "CrashReporter <parameters> \n"
                                          "  parameters:\n"
                                          "    -h Print this message and exit\n\n"
                                          "    -p <processId>    \n"
                                          "    -e <exeName>      \n"
                                          "    -v <eventName>    \n"
                                          "    -s <smtpServer>   \n"
                                          "    -p <smtpPort>     \n"
                                          "    -f <emailFrom>    \n"
                                          "    -t <emailTo>      \n"
                                          "\n"
                                          "CrashReporter is not intended to be run manually.\n"
                                          "It should be launched programatically by the process being monitored.\n",
                                          msg);

	MessageBox (NULL, 
		usageMsg,
		"CrashReporter", 
		MB_ICONERROR | MB_OK);
}

// ======================================================================

bool processCommandLine(int argc, char* argv[])
{
    bool ret = true;
	int i;
	for (i = 1; (i < argc) && ret; ++i)
	{
		if (argv[i][0] == '-')
		{
			switch(argv[i][1])
			{
			case 'h':
				usage("Printing command line info");
                ret = false;
				break;
			case 'i':
				s_processId = argv[++i];
				break;
			case 'e':
				s_exeName = argv[++i];
				break;
			case 'v':
				s_eventName = argv[++i];
				break;
			case 's':
				s_smtpServer = argv[++i];
				break;
			case 'p':
				s_smtpPort = argv[++i];
				break;
			case 'f':
				s_emailFrom = argv[++i];
				break;
			case 't':
				s_emailTo = argv[++i];
				break;
			}
		}
		else
		{
            char msg[256];
            _snprintf(msg, sizeof(msg), "invalid parameter %s", argv[i]);
            usage(msg);
            ret = false;
		}
	}

    return ret;
}

// ======================================================================
