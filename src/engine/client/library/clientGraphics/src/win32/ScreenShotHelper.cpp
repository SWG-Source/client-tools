// ======================================================================
//
// ScreenShotHelper.cpp
// Copyright 2003, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/ScreenShotHelper.h"

#include "clientGraphics/Graphics.h"
#include "sharedFoundation/Os.h"
#include "sharedUtility/LocalMachineOptionManager.h"

#include <stdio.h>
#include <string>
// ======================================================================

namespace ScreenShotHelperNamespace
{
	int ms_screenShotNumber = -1;
}
using namespace ScreenShotHelperNamespace;

// ======================================================================

void ScreenShotHelper::install()
{
	LocalMachineOptionManager::registerOption(ms_screenShotNumber, "ClientGraphics", "screenShotNumber");
}

// ----------------------------------------------------------------------

bool ScreenShotHelper::screenShot()
{
	char buffer[Os::MAX_PATH_LENGTH];
	
	// Run through and find the next available file name.
	FILE* fileCheck = NULL;

	Os::createDirectories("screenshots/");

	std::string format;

	switch(Graphics::getScreenShotFormat())
	{
	case GSSF_tga:
		format = ".tga";
			break;
	case GSSF_jpg:
		format = ".jpg";
			break;
	case GSSF_bmp:
		format = ".bmp";
		break;
	}

	do 
	{
		if(fileCheck)
			fclose(fileCheck);

		sprintf(buffer, "./screenshots/screenShot%04d", ++ms_screenShotNumber);
		std::string const fileName = buffer + format;
		fileCheck = fopen(fileName.c_str(), "r");
	}
	while(fileCheck);

	if(fileCheck)
		fclose(fileCheck);


	return Graphics::screenShot(buffer);
}

// ======================================================================
