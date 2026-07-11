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
	MEMORYSTATUSEX memoryStatus = { sizeof memoryStatus };
	GlobalMemoryStatusEx(&memoryStatus);
	int const ramMB = static_cast<int>(memoryStatus.ullTotalPhys / 1048576);

	// x64 client: the original code capped the target at 1536MB whenever RAM>=2048.
	// That was a 32-bit no-PAE ~2GB address-space workaround ("SWG crashes if we
	// give it all the RAM") and is OBSOLETE on this 64-bit build. The cap caused a
	// perf cliff once tracked allocations crossed 1536MB (the soft MemoryManager
	// target shown in the DebugInfo overlay as /1536MB). Use 75% of physical RAM as
	// the soft target (hardLimit=false), clamped to a 12288MB ceiling so very large
	// machines don't get a pathological target. Low-RAM boxes keep the old 75%.
	// Override at runtime with env var SWGCLIENT_MEMORY_SIZE_MB (uncapped).
	int targetMB = static_cast<int>(ramMB * 0.75);
	if (targetMB > 12288)
		targetMB = 12288;

	MemoryManager::setLimit(targetMB, false, false);
}

void externalCommandHandler(const char* command)
{
	const StringId trialNagId("client", "npe_nag_url_trial");
	const StringId rentalNagId("client", "npe_nag_url_rental");

	Unicode::String url;

	if ((Game::getSubscriptionFeatureBits() & ClientSubscriptionFeature::NPENagForTrial) != 0)
	{
		url = trialNagId.localize();
	}

	if (!url.empty())
	{
		Unicode::NarrowString url8 = Unicode::wideToNarrow( url );

		HINSTANCE result = ShellExecute(NULL, "open", url8.c_str(), NULL, NULL, SW_SHOWNORMAL);

		// ShellExecute returns HINSTANCE-typed errors: value <= 32 means failure.
		// On x64 HINSTANCE is 8 bytes; casting to int truncates the upper 32 bits
		// and produces nonsense comparisons. Use intptr_t for the cast.
		if (reinterpret_cast<intptr_t>(result) <= 32) //Pulled straight from MSDN -ARH
		{
			WARNING(true, ("could not launch external application (%p)", result));
		}
		else
		{
			Game::quit();
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
