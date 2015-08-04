// ======================================================================
//
// ConfigClientBugReporting.h
// Copyright 2004, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ConfigClientBugReporting_H
#define INCLUDED_ConfigClientBugReporting_H

// ======================================================================

class ConfigClientBugReporting
{
public:

	static void install();

	static bool getDisableToolBugReports();
	static char const * getToolBugReportEmailAddress();
	static char const * getToolBugReportFromEmailAddress();
	static char const * getToolBugReportSmtpServer();
	static char const * getToolBugReportSmtpPort();
	
	static char const * getCrashReporterExeName();
};

// ======================================================================

#endif
