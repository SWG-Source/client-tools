// ======================================================================
//
// ClientBugReporting.h
// copyright 2002, Sony Online Entertainment
//
// ======================================================================

#ifndef TOOL_BUG_REPORTING_H
#define TOOL_BUG_REPORTING_H

// ======================================================================

class ToolBugReporting
{
public:
	static void install();
	static void remove(void);

    // helper functions to start and stop the CrashReporter process
	static bool startCrashReporter(void);
	static void stopCrashReporter(unsigned int exitCode = 0);

    // Helper functions to put the CrashReporter's functionality inline to an application.
    // Note that the CrashReporter implements this functionality separately from the 
    //   clientBugReporting library, and does not depend on clientBugReporting.
	static bool detectCrashReport(const char *shortProgramName);
	static bool sendCrashReport(const char *shortProgramName, const char *reportText);
	static void clearCrashReport(const char *shortProgramName);

};

// ======================================================================

#endif
