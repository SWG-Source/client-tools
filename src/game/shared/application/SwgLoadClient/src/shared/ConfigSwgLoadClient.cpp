// ConfigSwgLoadClient.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstSwgLoadClient.h"
#include "ConfigSwgLoadClient.h"
#include "sharedFoundation/ConfigFile.h"

//-----------------------------------------------------------------------

ConfigSwgLoadClient::Data *	ConfigSwgLoadClient::data = 0;

#define KEY_INT(a,b)    (data->a = ConfigFile::getKeyInt("SwgLoadClient", #a, b))
#define KEY_BOOL(a,b)   (data->a = ConfigFile::getKeyBool("SwgLoadClient", #a, b)) 
#define KEY_REAL(a,b)   (data->a = ConfigFile::getKeyFloat("SwgLoadClient", #a, b))
#define KEY_STRING(a,b) (data->a = ConfigFile::getKeyString("SwgLoadClient", #a, b))

//-----------------------------------------------------------------------

void ConfigSwgLoadClient::install(void)
{
	data = new ConfigSwgLoadClient::Data;

	KEY_REAL   (chatEventTimerMax, 120);
	KEY_REAL   (chatEventTimerMin, 40);
	KEY_STRING (loginServerAddress, "swo-dev5.station.sony.com");
	KEY_INT    (loginServerPort, 44453);
	KEY_REAL   (clientCreateDelay, 5.0f);
	KEY_STRING (clusterName, "devcluster");
	KEY_STRING (creationLocation, "beta1");
	KEY_STRING (namePrefix, "dev");
	KEY_INT    (loadCount, 100);
	KEY_BOOL   (resetStart, false);
	KEY_REAL   (socialEventTimerMax, 120);
	KEY_REAL   (socialEventTimerMin, 30);
	KEY_REAL   (startX, 3409.0f);
	KEY_REAL   (startZ, 4634);
	KEY_REAL   (shipSpeed, 50.0f);
	KEY_REAL   (shipLoiterRadius, 512.0f);
	KEY_BOOL   (shipLoiterInCube, false);
	KEY_REAL   (shipLoiterCenterX, 0.f);
	KEY_REAL   (shipLoiterCenterY, 0.f);
	KEY_REAL   (shipLoiterCenterZ, 0.f);
	KEY_STRING (scriptSetupText, "loadClientSetup");
}

//-----------------------------------------------------------------------

void ConfigSwgLoadClient::remove(void)
{
	delete data;
	data = 0;
}

//-----------------------------------------------------------------------
