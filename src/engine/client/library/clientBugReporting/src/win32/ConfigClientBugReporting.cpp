// ======================================================================
//
// ConfigClientBugReporting.cpp
// copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "ClientBugReporting/FirstClientBugReporting.h"
#include "ClientBugReporting/ConfigClientBugReporting.h"

#include "sharedFoundation/ConfigFile.h"

#include <string>
#include <lmcons.h>


// ======================================================================

#define KEY_BOOL(a,b)    (ms_ ## a = ConfigFile::getKeyBool("ClientBugReporting", #a,   (b)))
#define KEY_STRING(a,b)    (ms_ ## a = ConfigFile::getKeyString("ClientBugReporting", #a,   (b)))

// ======================================================================

namespace ConfigClientBugReportingNamespace
{
	bool ms_disableToolBugReports;
	char const * ms_toolBugReportEmailAddress;
	char const * ms_toolBugReportFromEmailAddress;
	char const * ms_toolBugReportSmtpServer;
	char const * ms_toolBugReportSmtpPort;
	char const * ms_crashReporterExeName;

    char const ms_domainName[] = "@soe.sony.com";
    char ms_defaultFromEmailAddress[UNLEN + sizeof(ms_domainName) + 1];

}

using namespace ConfigClientBugReportingNamespace;

// ======================================================================

void ConfigClientBugReporting::install()
{
	DWORD size = sizeof(ms_defaultFromEmailAddress);
    IGNORE_RETURN(GetUserName( ms_defaultFromEmailAddress, &size));

    strncat(ms_defaultFromEmailAddress, ms_domainName, sizeof(ms_defaultFromEmailAddress));


	KEY_BOOL(disableToolBugReports, false);
	KEY_STRING(toolBugReportEmailAddress, "swgtoolbugreports@soe.sony.com");
	KEY_STRING(toolBugReportFromEmailAddress, ms_defaultFromEmailAddress);
	KEY_STRING(toolBugReportSmtpServer, "monitor.station.sony.com");
	KEY_STRING(toolBugReportSmtpPort, "25");
	KEY_STRING(crashReporterExeName, "CrashReporter_r.exe");
}

// ----------------------------------------------------------------------

bool ConfigClientBugReporting::getDisableToolBugReports()
{
	return ms_disableToolBugReports;
}

// ----------------------------------------------------------------------

char const * ConfigClientBugReporting::getToolBugReportEmailAddress()
{
	return ms_toolBugReportEmailAddress;
}

// ----------------------------------------------------------------------

char const * ConfigClientBugReporting::getToolBugReportFromEmailAddress()
{
	return ms_toolBugReportFromEmailAddress;
}

// ----------------------------------------------------------------------

char const * ConfigClientBugReporting::getToolBugReportSmtpServer()
{
	return ms_toolBugReportSmtpServer;
}

// ----------------------------------------------------------------------

char const * ConfigClientBugReporting::getToolBugReportSmtpPort()
{
	return ms_toolBugReportSmtpPort;
}

// ----------------------------------------------------------------------

char const * ConfigClientBugReporting::getCrashReporterExeName()
{
	return ms_crashReporterExeName;
}


// ======================================================================


