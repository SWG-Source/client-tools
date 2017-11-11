// ======================================================================
//
// ClientBugReporting.h
// copyright 2002, Sony Online Entertainment
//
// ======================================================================

#ifndef CLIENT_BUG_REPORTING_H
#define CLIENT_BUG_REPORTING_H

// ======================================================================

class RegistryKey;

// ======================================================================

#include <vector>

// ======================================================================

class ClientBugReporting
{
public:
	static void install();
	static void remove(void);

	static bool failedShutdown();

	static bool sendMail(const std::string& to, const std::string& from, const std::string& subject, const std::string& body, const std::vector<std::string>& attachments, bool sendMiniDump);

private:
	static bool ms_installed;
	static const std::string ClientBugReporting::ms_smtpServer;
	static const std::string ClientBugReporting::ms_smtpPort;
	static DWORD ms_processId;
};

// ======================================================================

#endif
