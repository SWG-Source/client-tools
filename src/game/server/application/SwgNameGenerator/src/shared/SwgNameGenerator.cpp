#include "FirstSwgNameGenerator.h"
#include "sharedUtility/NameGenerator.h"
#include "sharedRandom/SetupSharedRandom.h"

#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedUtility/SetupSharedUtility.h"
#include "UnicodeUtils.h"

#include <stdio.h>
#include <time.h>

int main(int argc, char ** argv)
{
	if (argc <= 1)
	{
		fprintf(stderr, "Usage: %0 language (count)\n", argv[0]);
		exit(1);
	}

	SetupSharedThread::install();
	SetupSharedDebug::install(4096);

	SetupSharedFoundation::Data SetupSharedFoundationData (SetupSharedFoundation::Data::D_console);
	SetupSharedFoundationData.useWindowHandle  = false;
	SetupSharedFoundationData.argc = 0;
	SetupSharedFoundationData.argv = 0;
	SetupSharedFoundationData.configFile = "servercommon.cfg";
	SetupSharedFoundation::install (SetupSharedFoundationData);

	SetupSharedFile::install(false);
	
	SetupSharedRandom::install(time(0));
	
	SetupSharedUtility::Data setupUtilityData;
	SetupSharedUtility::setupGameData (setupUtilityData);
	SetupSharedUtility::install (setupUtilityData);

	char * race = argv[1];
	int count = 10;
	if (argc >= 3)
		count = atoi(argv[2]);

	NameGenerator * generator = new NameGenerator(race);
	while (count--)
	{
		printf("%s\n", Unicode::wideToNarrow(generator->generateRandomName()).c_str());
	}

	delete generator;

	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	return 0;
}
