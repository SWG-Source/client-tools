// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the NXNINTEGRATORSDK_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// NXNINTEGRATORSDK_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

#ifndef NXNINTEGRATORSDK_API

	#ifdef BUILD_NXN_INTEGRATOR_SDK_DLL
		#define NXNINTEGRATORSDK_API __declspec(dllexport)
		#define NXNINTEGRATORSDK_EXPIMP_TEMPLATE
		#pragma message("     Exporting methods to NxNIntegratorSDK DLL")
	#else
		#ifdef USE_NXN_INTEGRATOR_SDK_DLL
			#define NXNINTEGRATORSDK_API __declspec(dllimport)
			#define NXNINTEGRATORSDK_EXPIMP_TEMPLATE extern
			#pragma message("     Importing methods from NxNIntegratorSDK DLL")
		#else
//			#define NXN_INTEGRATORSDL_API
//			#pragma message("     NxNIntegratorSDK DLL is not in use")
			#define NXNINTEGRATORSDK_API __declspec(dllimport)
			#define NXNINTEGRATORSDK_EXPIMP_TEMPLATE extern
			#pragma message("     Importing methods from NxNIntegratorSDK DLL")
		#endif
	#endif

	// export an explicit calling convention for static methods
	#define NXNINTEGRATORSDK_API_CALL	__cdecl
#endif
