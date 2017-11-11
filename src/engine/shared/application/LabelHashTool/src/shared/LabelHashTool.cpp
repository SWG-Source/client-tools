// ======================================================================
//
// DataTableTool.cpp
//
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "FirstLabelHashTool.h"
#include "LabelHashTool.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/SetupSharedFoundation.h"

#include <stdio.h>

// ======================================================================

static int globalArgc;
static char **globalArgv;

// ======================================================================

static void labelHash()
{
	for (int i = 1; i < globalArgc; ++i)
		printf("0x%08x %s\n", Crc::calculate(globalArgv[i]), globalArgv[i]);
}

// ======================================================================

int main(int argc, char **argv)
{
	//-- thread
	SetupSharedThread::install();

	//-- debug
	SetupSharedDebug::install(4096);

	{
		SetupSharedFoundation::Data data(SetupSharedFoundation::Data::D_console);
		SetupSharedFoundation::install(data);
	}

	globalArgc = argc;
	globalArgv = argv;

	SetupSharedFoundation::callbackWithExceptionHandling(labelHash);
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	return 0;
}


// ======================================================================
