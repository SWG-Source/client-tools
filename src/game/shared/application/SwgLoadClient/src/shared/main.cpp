// main.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstSwgLoadClient.h"
#include "ConfigSwgLoadClient.h"
#include "SwgLoadClient.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedFoundation/Os.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedThread/SetupSharedThread.h"

//-----------------------------------------------------------------------

int main( int argc, char ** argv )
{
	SetupSharedThread::install();
	SetupSharedDebug::install(1024);

	//-- setup foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);

	//setupFoundationData.hInstance               = hInstance;
	setupFoundationData.argc                      = argc;
	setupFoundationData.argv                      = argv;
	#if defined(PLATFORM_WIN32)
	setupFoundationData.createWindow              = false;
	#endif

	//setupFoundationData.runInBackground           = true;
	
	SetupSharedFoundation::install (setupFoundationData);

	SetupSharedFile::install(false);

	SetupSharedRandom::install(Os::getRealSystemTime());

	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultClientSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);

	//-- setup game server
	ConfigSwgLoadClient::install ();

	//-- run game
	SetupSharedFoundation::callbackWithExceptionHandling(SwgLoadClient::run);

	SetupSharedFoundation::remove();

	ConfigSwgLoadClient::remove();
	SetupSharedThread::remove();

	return 0;
}

// ======================================================================
