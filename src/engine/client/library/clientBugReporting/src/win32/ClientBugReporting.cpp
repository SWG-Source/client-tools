// ======================================================================
//
// ClientBugReporting.cpp
// copyright 2002, Sony Online Entertainment
//
// ======================================================================

#include "clientBugReporting/FirstClientBugReporting.h"
#include "clientBugReporting/ClientBugReporting.h"

#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedFoundation/RegistryKey.h"
#include "sharedFoundation/ExitChain.h"

#include <string>

// ======================================================================

namespace
{
	namespace MyOs
	{
		int listFilesGeneric(const char*dirname, char**& children, size_t bits, size_t exclude)
		{
			std::vector<char*> result;
			
			WIN32_FIND_DATA data;
			
			HANDLE handle = FindFirstFile(dirname,&data);
			
			if(handle != INVALID_HANDLE_VALUE)
			{	
				do
				{
					if((data.dwFileAttributes& exclude) == 0u &&
						(data.dwFileAttributes& bits) != 0u &&
						data.cFileName [0] != '.')
						result.push_back(DuplicateString(data.cFileName));
				}
				while(FindNextFile(handle,&data));
				
				IGNORE_RETURN(FindClose(handle));
				
				if(!result.empty())
				{
					children = new char* [result.size()];
					IGNORE_RETURN(std::copy(result.begin(), result.end(), children));
				}
			}
			
			return static_cast<int>(result.size());
		}

		inline int listDirectories(const char*dirname, char**& children)
		{
			return listFilesGeneric(dirname, children, FILE_ATTRIBUTE_DIRECTORY, 0);
		}

		inline int listFiles(const char*dirname, char**& children)
		{
			return listFilesGeneric(dirname, children, 0xffffffff, FILE_ATTRIBUTE_DIRECTORY);
		}
	}
}

// ======================================================================

bool ClientBugReporting::ms_installed      = false;
const std::string ClientBugReporting::ms_smtpServer = "mail.station.sony.com";
const std::string ClientBugReporting::ms_smtpPort   = "2525";
DWORD ClientBugReporting::ms_processId;

// ======================================================================

void ClientBugReporting::install()
{
	ExitChain::add(ClientBugReporting::remove, "ClientBugReporting::remove", 0, true);

	ms_installed = true;
}

// ======================================================================

void ClientBugReporting::remove()
{
	ms_installed = false;
}

//@todo hack fix this (create a blat.h)
int callBlat(int argc, char **argv, char **envp);

// ======================================================================

bool ClientBugReporting::sendMail(const std::string& to, const std::string& from, const std::string& subject, const std::string& body, const std::vector<std::string>& attachments, bool sendMiniDump)
{
	int numAttachments = attachments.size();
	if(sendMiniDump)
		++numAttachments;

	const int static_args = 13;

	int argc = static_args + (2 * numAttachments);
	char** argv = new char*[argc];
	int argv_value = 1;
	argv[argv_value++] = "-to";
	argv[argv_value++] = const_cast<char*>(to.c_str());
	argv[argv_value++] = "-subject";
	argv[argv_value++] = const_cast<char*>(subject.c_str());
	argv[argv_value++] = "-smtphost";
	argv[argv_value++] = const_cast<char*>(ms_smtpServer.c_str());
	argv[argv_value++] = "-port";
	argv[argv_value++] = const_cast<char*>(ms_smtpPort.c_str());
	argv[argv_value++] = "-f";
	argv[argv_value++] =  const_cast<char*>(from.c_str());
	argv[argv_value++] = "-body";
	argv[argv_value++] = const_cast<char*>(body.c_str());

	//iterate through all attachments, building the argv entries for them
	for(std::vector<std::string>::const_iterator i = attachments.begin(); i != attachments.end(); ++i)
	{
		argv[argv_value++] = "-attach";
		argv[argv_value++] = const_cast<char*>(i->c_str());
	}

	int numFiles = 0;
	char** files = NULL;
	//now attach the minidump, if needed
	if(sendMiniDump)
	{
		std::string miniDumpFilename;

		//build the substring we need to find within the file list "_<pid>.mdmp"
		char processIdString[256];
		_itoa(ms_processId, processIdString, 10);
		std::string substring = "_";
		substring += processIdString;
		substring += ".mdmp";

		char* minidumpfilename = NULL;
		//the minidump must exist in the current working directory
		numFiles = MyOs::listFiles(".\\SwgClient_*.mdmp", files);
		
		for(int i = 0; i < numFiles; ++i)
		{
			char* f = files[i];
			UNREF(f);
			if(strstr(f, substring.c_str()) != 0)
				minidumpfilename = f;
		}

		if(minidumpfilename)
		{
			argv[argv_value++] = "-attach";
			argv[argv_value++] = minidumpfilename;
		}
		else
			//despite ourselves, we couldn't find a valid minidump file to send, update the argc to reflect that
			argc -= 2;
	}

	//this doesn't get used, but make sure to send initialized memory anyway
	char* envp = new char[256];
	memset(envp, 0, 256);

	//call into blat to send the mail
	int result = callBlat(argc, argv, &envp);
	//clean up allocated memory
	delete[] argv;
	delete[] envp;
	if(numFiles)
		delete[] files;

	if(result != 0)
		return false;
	else
		return true;
}

// ======================================================================
