#ifndef INC_NXN_VERSION_H
#define INC_NXN_VERSION_H

/*! \file		NxNVersion.h
 *
 *  \brief		Contains all SDK version related stuff.
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-05-03-2000 file created.
 *	\endmod
 */
//---------------------------------------------------------------------------
//	current sdk version defines (DO NOT ALTER THESE CONSTANTS!)
//---------------------------------------------------------------------------
#define VERSION_MAJOR				(1)
#define VERSION_MINOR				(2)
#define VERSION_BUILDNUMBER			(2)

#define NXN_ALIENBRAIN_API

// alienbrain lib, dll
#ifdef NXN_ALIENBRAIN_API 
#	if defined(NXN_DEBUG)
#		define NXN_XDK_DLL_NAME			"NxN_alienbrain_XDK_122_D.dll"
#		define NXN_XDK_LIB_NAME			"NxN_alienbrain_XDK_122_D.lib"
#		define NXN_XDK_DLL_NAME_WIDE	L"NxN_alienbrain_XDK_122_D.dll"
#	else
#		define NXN_XDK_DLL_NAME			"NxN_alienbrain_XDK_122.dll"
#		define NXN_XDK_LIB_NAME			"NxN_alienbrain_XDK_122.lib"
#		define NXN_XDK_DLL_NAME_WIDE	L"NxN_alienbrain_XDK_122.dll"
#	endif
#else 
// medializer lib, dll
#	ifdef NXN_MEDIALIZER_API
#		if defined(NXN_DEBUG)
#			define NXN_XDK_DLL_NAME			"NxN_medializer_XDK_122_D.dll"
#			define NXN_XDK_LIB_NAME			"NxN_medializer_XDK_122_D.lib"
#			define NXN_XDK_DLL_NAME_WIDE	L"NxN_medializer_XDK_122_D.dll"
#		else
#			define NXN_XDK_DLL_NAME			"NxN_medializer_XDK_122.dll"
#			define NXN_XDK_LIB_NAME			"NxN_medializer_XDK_122.lib"
#			define NXN_XDK_DLL_NAME_WIDE    L"NxN_medializer_XDK_122.dll"
#		endif
// wrong api
#	else 
#		error Wrong or no correct API defined. Define either NXN_ALIENBRAIN_API or NXN_MEDIALIZER_API!
#	endif // NXN_MEDIALIZER_API
#endif // NXN_ALIENBRAIN_API



#endif // INC_NXN_VERSION_H
