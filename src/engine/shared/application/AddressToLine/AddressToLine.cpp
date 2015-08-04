// AddressToLine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdio.h>
#include <assert.h>
#include <atlbase.h>

#include "dbghelp.h"

// Print a fatal error message, with optional translation of system error code.
// Terminate execution (do not return).
void Fatal( const char *msg, bool printLastError )
{
	printf( "Fatal: %s\n", msg );

	if (printLastError)
	{
		LPVOID lpMsgBuf;
		
		if (FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL ))
		{
			printf( "%s\n", (LPCTSTR)lpMsgBuf );

			// Free the buffer.
			LocalFree( lpMsgBuf );
		}
	}

    exit( -1 );
}

// Print command-line syntax and then exit.
void usage( int /*argc*/, char* argv[] )
{
	printf( "usage: %s <exe-filename> [addresses...]\n", argv[0] );
	printf( "  <exe-filename> - the name of the executable image file.\n");
	printf( "  [addresses...] - zero or more addresses to look up. If no addresses are\n");
	printf( "                   specified, addresses will be read from stdin.\n");
	exit( -1 );
}


void Lookup(HANDLE hProcess, const char *pAddress)
{
	DWORD address = strtol(pAddress, 0, 0);

	DWORD displacement = 0;
	IMAGEHLP_LINE64 line;
	line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

	if (SymGetLineFromAddr64(hProcess, address, &displacement, &line))
	{
		__int64 unsigned displacement = 0;

		char buffer[8 * 1024];
		memset(buffer, 0, sizeof(buffer));

		PIMAGEHLP_SYMBOL64 symbol = reinterpret_cast<PIMAGEHLP_SYMBOL64>(buffer);
		symbol->SizeOfStruct = sizeof(symbol);
		symbol->MaxNameLength = sizeof(buffer) - sizeof(symbol);

		if (SymGetSymFromAddr64(hProcess, address, &displacement, symbol))
		{
			printf("%s(%u) %s\n", line.FileName, line.LineNumber, symbol->Name);
		}
		else
		{
			printf("%s(%u) unknown\n", line.FileName, line.LineNumber);
		}
	}
	else
	{
		printf("UNKNOWN(%s)\n", pAddress);
	}
}


int main(int argc, char* argv[])
{
	static const int s_numRequiredArgs = 2;

    if ( argc < s_numRequiredArgs ) 
	{
        usage(argc, argv); // does not return
    }


	SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS | SYMOPT_EXACT_SYMBOLS);

	HANDLE hProcess = GetCurrentProcess();

	// set up for processing from the input file
	if (!SymInitialize(hProcess, 0, FALSE))
	{
		Fatal("SymInitialize", true);	// does not return
	}


	DWORD64 baseAddr = SymLoadModule64(hProcess, 0, argv[1], 0, 0, 0);
	if (baseAddr == 0)
	{
		Fatal("SymLoadModule64", true); // does not return
	}


	if (argc > s_numRequiredArgs)
	{
		// process addresses from the command line
		int i;
		for (i = s_numRequiredArgs; i < argc; ++i)
		{
			Lookup(hProcess, argv[i]);
		}
	}
	else
	{
		// process addresses from stdin
		char buf[256];
		char delim[] = " \r\n\t,;";
		while (fgets(buf, sizeof(buf), stdin))
		{
			char *pToken = strtok(buf, delim);

			while (pToken != 0)
			{
				Lookup(hProcess, pToken);
				pToken = strtok(0, delim);
			}
		}
	}


	if (!SymUnloadModule64(hProcess, baseAddr))
	{
		Fatal("SymUnloadModule64", true);  // does not return
	}

	if (!SymCleanup(hProcess))
	{
		Fatal("SymCleanup", true);  // does not return
	}
	
	return 0;
}
