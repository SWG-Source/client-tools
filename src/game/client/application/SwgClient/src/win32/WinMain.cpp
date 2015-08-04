// ======================================================================
//
// WinMain.cpp
//
// ======================================================================

#include "FirstSwgClient.h"

#include "ClientMain.h"

#include "LocalizedString.h"
#include "StringId.h"

#include "clientGame/Game.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Production.h"
#include "../../../../../../engine/shared/library/sharedGame/include/public/sharedGame/PlatformFeatureBits.h"

#include <shellapi.h>

extern void externalCommandHandler(const char*);

// ======================================================================

static bool SetUserSelectedMemoryManagerTarget()
{
	char buffer[32];
	DWORD result = GetEnvironmentVariable("SWGCLIENT_MEMORY_SIZE_MB", buffer, sizeof(buffer));

	// make sure the environment variable was set
	if (result <= 0 || result >= sizeof(buffer))
		return false;

	// inline atoi() because the crt hasn't been initialized yet
	int megabytes = 0;
	for (char const * b = buffer; *b; ++b)
	{
		// handle bad characters in the environment variable by ignoring the whole thing
		if (*b < '0' || *b > '9')
			return false;

		megabytes = (megabytes * 10) + (*b - '0');
	}

	MemoryManager::setLimit(megabytes, false, false);
	return true;
}

// ----------------------------------------------------------------------

static void SetDefaultMemoryManagerTargetSize()
{
	int megabytes = 0;
	MEMORYSTATUS memoryStatus;
	GlobalMemoryStatus (&memoryStatus);
	megabytes = ((memoryStatus.dwTotalPhys / 4) * 3) / (1024 * 1024);

	// clamp it between 250 and 2000 MB in production, 750 in dev environments
	if (megabytes < 250)
		megabytes = 250;
#if PRODUCTION == 0
	if (megabytes > 750)
		megabytes = 750;
#else
	if (megabytes > 2000)
		megabytes = 2000;
#endif


	MemoryManager::setLimit(megabytes, false, false);
}


void externalCommandHandler(const char* command)
{
	if (strcmp(command, "npe_continue") == 0)
	{
		const StringId trialNagId("client", "npe_nag_url_trial");
		const StringId rentalNagId("client", "npe_nag_url_rental");

		Unicode::String url;

		/*
		if ((Game::getSubscriptionFeatureBits() & ClientSubscriptionFeature::NPENagForRental) != 0)
		{
			url = rentalNagId.localize();
		}
		else 
		*/
		if ((Game::getSubscriptionFeatureBits() & ClientSubscriptionFeature::NPENagForTrial) != 0)
		{
			url = trialNagId.localize();
		}

		if (!url.empty())
		{
			Unicode::NarrowString url8 = Unicode::wideToNarrow( url );

			HINSTANCE result = ShellExecute(NULL, "open", url8.c_str(), NULL, NULL, SW_SHOWNORMAL);

			if (reinterpret_cast<int>(result) < 32) //Pulled straight from MSDN -ARH
			{
				WARNING(true, ("could not launch external application (%d)", reinterpret_cast<int>(result)));
			}
			else
			{
				Game::quit();
			}
		}
	}
}

// ======================================================================
// Entry point for the application
//
// Return Value:
//
//   Result code to return to the operating system
//
// Remarks:
//
//   This routine should set up the engine, invoke the main game loop,
//   and then tear down the engine.

int WINAPI WinMain(
	HINSTANCE hInstance,      // handle to current instance
	HINSTANCE hPrevInstance,  // handle to previous instance
	LPSTR     lpCmdLine,      // pointer to command line
	int       nCmdShow        // show state of window
	)
{
	if (!SetUserSelectedMemoryManagerTarget())
		SetDefaultMemoryManagerTargetSize();

	return ClientMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

// ======================================================================
