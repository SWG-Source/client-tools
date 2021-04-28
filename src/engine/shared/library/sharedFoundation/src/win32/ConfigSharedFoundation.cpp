// ======================================================================
//
// ConfigSharedFoundation.cpp
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/ConfigSharedFoundation.h"

#include "sharedFoundation/ConfigFile.h"

// ======================================================================

#define KEY_INT(a,b)     (ms_ ## a = ConfigFile::getKeyInt("SharedFoundation", #a, b))
#define KEY_BOOL(a,b)    (ms_ ## a = ConfigFile::getKeyBool("SharedFoundation", #a, b))
#define KEY_FLOAT(a,b)   (ms_ ## a = ConfigFile::getKeyFloat("SharedFoundation", #a, b))
#define KEY_STRING(a,b)  (ms_ ## a = ConfigFile::getKeyString("SharedFoundation", #a, b))

// ======================================================================

const int c_defaultFatalCallStackDepth   = 32;
const int c_defaultWarningCallStackDepth = -1;

// ======================================================================

namespace ConfigSharedFoundationNamespace
{
	bool        ms_noExceptionHandling;

	bool        ms_fpuExceptionPrecision;
	bool        ms_fpuExceptionUnderflow;
	bool        ms_fpuExceptionOverflow;
	bool        ms_fpuExceptionZeroDivide;
	bool        ms_fpuExceptionDenormal;
	bool        ms_fpuExceptionInvalid;

	bool        ms_demoMode;

	real        ms_frameRateLimit;
	real        ms_minFrameRate;

	bool        ms_useRemoteDebug;
	int         ms_defaultRemoteDebugPort;

	bool        ms_profilerExpandAllBranches;

	bool        ms_memoryManagerReportAllocations;
	bool        ms_memoryManagerReportOnOutOfMemory;

	bool        ms_useMemoryBlockManager;
	bool        ms_memoryBlockManagerDebugDumpOnRemove;

	int         ms_fatalCallStackDepth;
	int         ms_warningCallStackDepth;
	bool        ms_lookUpCallStackNames;

	int         ms_processPriority;

	bool        ms_verboseHardwareLogging;
	bool        ms_verboseWarnings;

	bool        ms_causeAccessViolation;

	float       ms_debugReportLongFrameTime;

	bool		ms_developmentMode;
}

using namespace ConfigSharedFoundationNamespace;

// ======================================================================
// Determine the Platform-specific configuration information
//
// Remarks:
//
//   This routine inspects the ConfigFile class to set some variables for rapid access
//   by the rest of the engine.

void ConfigSharedFoundation::install (const Defaults &defaults)
{
	KEY_BOOL(noExceptionHandling,             false);

	KEY_BOOL(fpuExceptionPrecision,           false);
	KEY_BOOL(fpuExceptionUnderflow,           false);
	KEY_BOOL(fpuExceptionOverflow,            false);
	KEY_BOOL(fpuExceptionZeroDivide,          false);
	KEY_BOOL(fpuExceptionDenormal,            false);
	KEY_BOOL(fpuExceptionInvalid,             false);

	KEY_BOOL(demoMode,                        defaults.demoMode);

	KEY_FLOAT(frameRateLimit,				  defaults.frameRateLimit);
	KEY_FLOAT(minFrameRate,					  defaults.minFrameRate);

	KEY_BOOL(useRemoteDebug,                  false);
	KEY_INT(defaultRemoteDebugPort,           4445);

	KEY_BOOL(profilerExpandAllBranches,       false);
	KEY_BOOL(memoryManagerReportAllocations, true);
	KEY_BOOL(memoryManagerReportOnOutOfMemory, true);
	KEY_BOOL(useMemoryBlockManager, true);
	KEY_BOOL(memoryBlockManagerDebugDumpOnRemove, false);

	KEY_INT(fatalCallStackDepth,              c_defaultFatalCallStackDepth);
	KEY_INT(warningCallStackDepth,            c_defaultWarningCallStackDepth);
	KEY_BOOL(lookUpCallStackNames,            true);

	KEY_INT(processPriority,                  0);

	KEY_BOOL(verboseHardwareLogging,          false);
	KEY_BOOL(verboseWarnings,                 defaults.verboseWarnings);

	KEY_BOOL(causeAccessViolation,            false);

	KEY_FLOAT(debugReportLongFrameTime,       0.25f);

	KEY_BOOL(developmentMode, true);
}

// ----------------------------------------------------------------------
/**
 * Return whether to run with exception handling enabled.
 *
 * @return True to run without exception handling
 */

bool ConfigSharedFoundation::getNoExceptionHandling()
{
	return ms_noExceptionHandling;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getFpuExceptionPrecision()
{
	return ms_fpuExceptionPrecision;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getFpuExceptionUnderflow()
{
	return ms_fpuExceptionUnderflow;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getFpuExceptionOverflow()
{
	return ms_fpuExceptionOverflow;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getFpuExceptionZeroDivide()
{
	return ms_fpuExceptionZeroDivide;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getFpuExceptionDenormal()
{
	return ms_fpuExceptionDenormal;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getFpuExceptionInvalid()
{
	return ms_fpuExceptionInvalid;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getDemoMode()
{
  return ms_demoMode;
}

// ----------------------------------------------------------------------
/**
 * Return the frame rate limit value for the game.
 *
 * @return The initial frame rate limiter value
 */

real ConfigSharedFoundation::getFrameRateLimit()
{
	return ms_frameRateLimit;
}

// ----------------------------------------------------------------------
/**
 * Return the minimum frame rate value for the game.  Frames that take longer
 * will log a warning and be hard set to the given value.
 *
 * @return The initial min frame rate value
 */

real ConfigSharedFoundation::getMinFrameRate()
{
	return ms_minFrameRate;
}

// ----------------------------------------------------------------------

int ConfigSharedFoundation::getDefaultRemoteDebugPort()
{
	return ms_defaultRemoteDebugPort;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getUseRemoteDebug()
{
	return ms_useRemoteDebug;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getProfilerExpandAllBranches()
{
	return ms_profilerExpandAllBranches;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getMemoryManagerReportAllocations()
{
	return ms_memoryManagerReportAllocations;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getMemoryManagerReportOnOutOfMemory()
{
	return ms_memoryManagerReportOnOutOfMemory;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getUseMemoryBlockManager()
{
	return ms_useMemoryBlockManager;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getMemoryBlockManagerDebugDumpOnRemove ()
{
	return ms_memoryBlockManagerDebugDumpOnRemove;
}

// ----------------------------------------------------------------------

int ConfigSharedFoundation::getFatalCallStackDepth()
{
	if(getDevelopmentMode())
	{
		return c_defaultFatalCallStackDepth;
	}
	return ms_fatalCallStackDepth;
}

// ----------------------------------------------------------------------

int ConfigSharedFoundation::getWarningCallStackDepth()
{
	if(getDevelopmentMode())
	{
		return 12;
	}
	return ms_warningCallStackDepth;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getLookUpCallStackNames()
{
	if(getDevelopmentMode())
	{
		return true;
	}
	return ms_lookUpCallStackNames;
}

// ----------------------------------------------------------------------

int ConfigSharedFoundation::getProcessPriority()
{
	return ms_processPriority;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getVerboseHardwareLogging()
{
	return ms_verboseHardwareLogging;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getVerboseWarnings()
{
	return ms_verboseWarnings;
}

// ----------------------------------------------------------------------

void ConfigSharedFoundation::setVerboseWarnings(bool const verboseWarnings)
{
	ms_verboseWarnings = verboseWarnings;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getCauseAccessViolation()
{
	return ms_causeAccessViolation;
}


// ----------------------------------------------------------------------

float ConfigSharedFoundation::getDebugReportLongFrameTime()
{
	return ms_debugReportLongFrameTime;
}

// ----------------------------------------------------------------------

// This is an SWG Source addition we're using for the purposes of defaulting
// more verbose logging and development tools to ON but we're using a config
// option so players wanting a more live-like client to share don't have to
// recompile based on the PRODUCTION constant and can instead just set this
// [SharedFoundation] developmentMode=false if they want

// ***** WARNING ******
// This is a config toggle which anyone can change so obviously nothing
// related to access should be wrapped in this (read: things that should
// require god mode still need to use PlayerObject::isAdmin() not this)

bool ConfigSharedFoundation::getDevelopmentMode()
{
	return ms_developmentMode;
}

// ======================================================================
