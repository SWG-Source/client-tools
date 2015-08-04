// ======================================================================
//
// ToolBugReporting.cpp
// copyright 2002, Sony Online Entertainment
//
// ======================================================================

#include "clientBugReporting/FirstClientBugReporting.h"
#include "clientBugReporting/ToolBugReporting.h"


#include "clientBugReporting/ConfigClientBugReporting.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"

#include "blat.h"

#include <string>
#include <fstream>
#include <sstream>
#include <vector>

// ======================================================================

namespace ToolBugReportingNamespace
{

    bool ms_installed        = false;

	DWORD ms_crashReporterId = 0;

	//=========================================================

	void getFileList( std::vector<std::string>& fileNameList, const char *shortProgramName )
	{
		//-- crash reports typically generate a mdmp file and a txt file. We look
		//-- for the presence of the mdmp file andsend it as an attachment. If we
		//-- have a text file, it forms the body of the message.

		WIN32_FIND_DATA findData;
		HANDLE hFind;

		std::string fileNameMask(shortProgramName);
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
					fileNameList.push_back (fileName);
				}
			} while (FindNextFile(hFind, &findData));

			IGNORE_RETURN(FindClose(hFind));
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
				<< "unknown: FATAL 00000000: minidump from ToolBugReporting." << std::endl
                << "Backtrace for " << baseName << " not found." << std::endl;
		}


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
					IGNORE_RETURN(file.getline(buf, bufSize));

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
		int const numAttachments = static_cast<int>(attachments.size());

		const int static_args = 13;

		int argc = static_args + (2 * numAttachments);
		char** argv = new char*[static_cast<unsigned int>(argc)];
		int argv_value = 0;
		argv[++argv_value] = const_cast<char*>("-to");
		argv[++argv_value] = const_cast<char*>(ConfigClientBugReporting::getToolBugReportEmailAddress());
		argv[++argv_value] = const_cast<char*>("-subject");
		argv[++argv_value] = const_cast<char*>(subject.c_str());
		argv[++argv_value] = const_cast<char*>("-smtphost");
		argv[++argv_value] = const_cast<char*>(ConfigClientBugReporting::getToolBugReportSmtpServer());
		argv[++argv_value] = const_cast<char*>("-port");
		argv[++argv_value] = const_cast<char*>(ConfigClientBugReporting::getToolBugReportSmtpPort());
		argv[++argv_value] = const_cast<char*>("-f");
		argv[++argv_value] = const_cast<char*>(ConfigClientBugReporting::getToolBugReportFromEmailAddress());
		argv[++argv_value] = const_cast<char*>("-body");
		argv[++argv_value] = const_cast<char*>(body.c_str());

		//iterate through all attachments, building the argv entries for them
		for(std::vector<std::string>::const_iterator i = attachments.begin(); i != attachments.end(); ++i)
		{
			argv[++argv_value] = const_cast<char*>("-attach");
			argv[++argv_value] = const_cast<char*>(i->c_str());
		}

	    // Blat doesn't use this parameter, but we want to send initialized memory anyway
        //  in case that changes.
	    char envp[256];
	    memset(envp, 0, sizeof(envp));

	    //call into blat to send the mail
        char *ptmp = envp;
	    IGNORE_RETURN(callBlat(argc, argv, &ptmp));

		//clean up allocated memory
		delete[] argv;
	}
}

using namespace ToolBugReportingNamespace;

// ======================================================================

void ToolBugReporting::install()
{
	ExitChain::add(ToolBugReporting::remove, "ToolBugReporting::remove", 0, true);

	ms_installed = true;
}

// ======================================================================

void ToolBugReporting::remove()
{
	ms_installed = false;
}

// ======================================================================

bool ToolBugReporting::sendCrashReport (const char *shortProgramName, const char *bodyText)
{
	bool ret = false;

	if (!ConfigClientBugReporting::getDisableToolBugReports())
	{
		//-- crash reports typically generate a mdmp file and a txt file. We look
		//-- for the presence of the mdmp file andsend it as an attachment. If we
		//-- have a text file, it forms the body of the message.

		std::vector<std::string> fileNameList;

		getFileList( fileNameList, shortProgramName );

		if (!fileNameList.empty ())
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
			for (iter = fileNameList.begin(); iter != fileNameList.end(); ++iter)
			{
				std::vector<std::string> attachments;
				
				std::string const dumpName = getMinidump ( *iter );
				if (!dumpName.empty())
				{
					attachments.push_back (dumpName);
				}

				std::string const backtraceName = getBacktrace( *iter );

				std::string subject = getSubject(*iter);
				std::string body = getBody(*iter, bodyText);

				sendMail (subject, body, attachments);

				if (!logfile.fail())
				{
					//-- write the file name and the exception or fatal string
					logfile << backtraceName << ' ' << getCode (*iter) << std::endl;
				}

				//-- delete the files
				IGNORE_RETURN(::remove (backtraceName.c_str()));
				IGNORE_RETURN(::remove (dumpName.c_str()));
			}

			logfile << std::endl;

			ret = true;
		}
	}

	return ret;
}

// ======================================================================

bool ToolBugReporting::detectCrashReport (const char *shortProgramName)
{
	bool ret = false;

	if (!ConfigClientBugReporting::getDisableToolBugReports())
	{
		std::vector<std::string> fileNameList;

		getFileList( fileNameList, shortProgramName );

		ret = fileNameList.size() > 0;
	}

	return ret;
}

// ======================================================================

void ToolBugReporting::clearCrashReport (const char *shortProgramName)
{
	std::vector<std::string> fileNameList;

	getFileList( fileNameList, shortProgramName );

	if (!fileNameList.empty ())
	{
		//-- send one email per dump found
		//-- log the files, and then remove them

		std::ofstream logfile("minidump.log", std::ios_base::out | std::ios_base::app);

		if (!logfile.fail())
		{
			//-- write the header
			time_t timeNow = time(0);
			std::string const header = asctime(localtime(&timeNow));
			
			logfile << header << std::endl;
		}


		std::vector<std::string>::const_iterator iter;
		for (iter = fileNameList.begin(); iter != fileNameList.end(); ++iter)
		{
			std::string const dumpName = getMinidump ( *iter );
			std::string const backtraceName = getBacktrace( *iter );

			if (!logfile.fail())
			{
				logfile << "[cleared]  ";
				//-- write the file name and the exception or fatal string
				logfile << backtraceName << ' ' << getCode (*iter) << std::endl;
			}

			//-- delete the files
			IGNORE_RETURN(::remove (backtraceName.c_str()));
			IGNORE_RETURN(::remove (dumpName.c_str()));
		}

		logfile << std::endl;
	}
}

// ======================================================================

bool ToolBugReporting::startCrashReporter(void)
{
	bool ret = false;

	FATAL( ms_crashReporterId != 0, ("CrashReporter already started"));

	char eventName[MAX_PATH];
    IGNORE_RETURN(snprintf(eventName, sizeof(eventName), "%s_%d", Os::getShortProgramName(), GetCurrentProcessId()));
	HANDLE event = CreateEvent(0, TRUE, FALSE, eventName );

	if (event != NULL)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );

		char commandLine[MAX_PATH];
		IGNORE_RETURN(snprintf(commandLine, sizeof(commandLine), "%s -e %s -i %d -v %s -s %s -p %s -t %s -f %s", 
            ConfigClientBugReporting::getCrashReporterExeName(), 
            Os::getShortProgramName(), 
            GetCurrentProcessId(),
            eventName,
            ConfigClientBugReporting::getToolBugReportSmtpServer(),
            ConfigClientBugReporting::getToolBugReportSmtpPort(),
            ConfigClientBugReporting::getToolBugReportEmailAddress(),
            ConfigClientBugReporting::getToolBugReportFromEmailAddress()
            ));

		if (CreateProcess(NULL, commandLine, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
		{
			if (WaitForSingleObject( event, 10000 ) == WAIT_OBJECT_0)
            {
			    ms_crashReporterId = pi.dwProcessId;

			    IGNORE_RETURN(CloseHandle(pi.hThread));
			    IGNORE_RETURN(CloseHandle(pi.hProcess));

			    // this prevents the OS from sending our crash reports to Microsoft
			    IGNORE_RETURN(SetErrorMode( SEM_NOGPFAULTERRORBOX ));

			    ret = true;
            }
            else
            {
                WARNING(true, ("startCrashReporter: exe did not start properly"));
            }
		}
		else
		{
			WARNING(true, ("startCrashReporter: exe not found [%s]", ConfigClientBugReporting::getCrashReporterExeName()));
		}

		IGNORE_RETURN(CloseHandle(event));
	}
	else
	{
		FATAL(true, ("startCrashReporter CreateEvent failed"));
	}

	return ret;
}

// ======================================================================

void ToolBugReporting::stopCrashReporter(unsigned int exitCode)
{
	if (ms_crashReporterId != 0)
	{
		HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, ms_crashReporterId);

		if (process != NULL)
		{
			IGNORE_RETURN(TerminateProcess(process, exitCode));
			IGNORE_RETURN(CloseHandle( process ));
		}

		ms_crashReporterId = 0;

		IGNORE_RETURN(SetErrorMode( 0 ));
	}
}

