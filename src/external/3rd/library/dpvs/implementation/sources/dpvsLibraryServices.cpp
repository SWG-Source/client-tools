/*****************************************************************************
 *
 * dPVS
 * -----------------------------------------
 *
 * (C) 1999-2001 Hybrid Holding, Ltd.
 * All Rights Reserved.
 *
 * This file consists of unpublished, proprietary source code of
 * Hybrid Holding, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Hybrid Holding and legal action against the party in breach.
 *
 * Description: 	Library Services - default implementation
 *
 * $Archive: /dpvs/implementation/sources/dpvsLibraryServices.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 8.10.02 14:34 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsPrivateDefs.hpp"
#include "dpvsLibrary.hpp"
#include "dpvsDebug.hpp"

//#undef NDEBUG								// we need to get the real assertion routines...

#include <cassert>
#include <cstdio>
#include <ctime>

#if defined (DPVS_OS_GAMECUBE)
//NGC-BEGIN
#	include <dolphin/os.h>
//NGC-END
#endif

#include <cstdlib>

namespace DPVS
{
/*****************************************************************************
 *
 * Function:		DPVS::assertFail()
 *
 * Description:		Assertion failure function -- calls either user-defined
 *					error handler or native assert() function.
 *
 * Parameters:		expr	= expression string (non-null)
 *					file	= string containing name of file where assertion happened (non-null)
 *					line	= line number where the assertion happened (starts from 1)
 *					message	= additional message (may be null)
 *
 * Notes:			This function will _not_ return.
 *
 *****************************************************************************/

void assertFail (const char* expr, const char* file, int line, const char* message)
{
	static bool g_failedOnce = false;

	//--------------------------------------------------------------------
	// The idea here is that if the error handler lets asserts go through, we don't
	// display subsequent asserts
	//--------------------------------------------------------------------

	if (!g_failedOnce)
	{
		g_failedOnce = true;

		Library::Services* handler = Debug::getServices();

		// we don't use error messages that are longer than 1023 characters.
		if (handler) // THERE MUST BE ONE!
		{
			char tmp[1024];
			sprintf (tmp,"%s in file %s, line %d\n.%s\n",const_cast<char*>(expr),const_cast<char*>(file),line,message?const_cast<char*>(message):"");
			handler->error (tmp);
		}
	}
}

LibraryDefs::Services::Services (void)
{
	// NADA
}

LibraryDefs::Services::~Services (void)
{
	// NADA
}

void LibraryDefs::Services::error (const char* s)
{
	printf ("%s",const_cast<char*>(s));
	assert(false);
}

void* LibraryDefs::Services::allocateMemory (size_t bytes)
{
	return ::malloc(bytes);
}

void LibraryDefs::Services::releaseMemory (void* p)
{
	::free(p);
}

float LibraryDefs::Services::getTime (void)
{
#if defined (DPVS_PS2) 
//PS2-BEGIN
	return 0.0f;
//PS2-END
#else
	static const double ooHz = 1.0 / (double)(CLOCKS_PER_SEC);
	double cl = clock();
	return (float)(cl * ooHz);
#endif 
}

/*****************************************************************************
 *
 * Function:		DPVS::LibraryDefs::Services::enterMutex()
 *
 * Description:		Enter mutex
 *
 *****************************************************************************/

void LibraryDefs::Services::enterMutex (void)
{
	// nada in basic implementation
}

/*****************************************************************************
 *
 * Function:		DPVS::LibraryDefs::Services::leaveMutex()
 *
 * Description:		Leave mutex
 *
 *****************************************************************************/

void LibraryDefs::Services::leaveMutex (void)
{
	// nada in basic implementation
}

} // namespace DPVS

//------------------------------------------------------------------------
