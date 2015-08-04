// ======================================================================
//
// main.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstRemoteDebugTool.h"
#include "RemoteDebugTool.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "sharedThread/SetupSharedThread.h"

#include <ctime>

// ======================================================================

/**This file is the first function run.  It installs the engine and
  *then starts up the application.  Once finished, it cleans up.
  */
int main(int argc, char ** argv)
{
	UNREF(argc);
	UNREF(argv);

	SetupSharedThread::install();
	SetupSharedDebug::install(1024);

	//-- setup foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);

	#if defined(PLATFORM_WIN32)
	setupFoundationData.hInstance                 = NULL;
	setupFoundationData.configFile                = "tools.cfg";
	setupFoundationData.createWindow              = false;
	setupFoundationData.runInBackground           = true;
	#endif

	SetupSharedFoundation::install (setupFoundationData);

	SetupSharedCompression::install();

	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultClientSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);

	//-- run game
	SetupSharedFoundation::callbackWithExceptionHandling(remoteDebugTool::run);

	ConfigSharedNetwork::remove();
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	return 0;
}

// ======================================================================

//allocate 64megs *max*
#if defined(PLATFORM_WIN32)
#pragma warning(disable: 4074)
#pragma init_seg(compiler)
static MemoryManager memoryManager(64*1024*1024, false);
#elif defined(PLATFORM_LINUX)
static MemoryManager memoryManager(64*1024*1024, false) __attribute__ ((init_priority(101)));
#else
#error unsupported platform
#endif

// ======================================================================

