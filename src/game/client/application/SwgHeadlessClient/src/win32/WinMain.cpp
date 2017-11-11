// ======================================================================
//
// WinMain.cpp
//
// ======================================================================

#include "FirstSwgHeadlessClient.h"

#include "ClientMain.h"
#include "sharedFoundation/Production.h"

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

	// clamp it between 250 and 750 MB
	if (megabytes < 250)
		megabytes = 250;

	if (megabytes > 750)
		megabytes = 750;

	MemoryManager::setLimit(megabytes, false, false);
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

#ifdef HEADLESS

#include <string>

int main( int /*argc*/, char *argv[] )
{
	if (!SetUserSelectedMemoryManagerTarget())
		SetDefaultMemoryManagerTargetSize();


	std::string cmdline;

	for ( char **arg = argv; *arg; ++arg )
	{
		cmdline += std::string( *arg ) + " " ;
	}

	return ClientMain( 0, 0, const_cast< char * >( cmdline.c_str() ), 0 );
}

#endif // HEADLESS
// ======================================================================
