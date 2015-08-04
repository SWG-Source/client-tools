// \addtodoc

#ifndef INC_NXN_MACROS_H
#define INC_NXN_MACROS_H

/* \file		NxNMacros.h
 *
 *  \brief		Collection of all NxNIntegratorSDK specific macros.
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-03-May-2000 file created.
 *	\endmod
 */
//---------------------------------------------------------------------------
//	error codes
//---------------------------------------------------------------------------
#include "NxNErrors.h"

//---------------------------------------------------------------------------
//	export/import macros
//---------------------------------------------------------------------------
#include "NxNExport.h"

//---------------------------------------------------------------------------
//	nxn internal macros
//---------------------------------------------------------------------------
#include "macros.h"

//---------------------------------------------------------------------------
//	class forward declarations
//---------------------------------------------------------------------------
#include "NxNFwdDecl.h"

#include "NxNClassList.h"

//---------------------------------------------------------------------------
//	macros
//---------------------------------------------------------------------------
// generation/new macros
#define SAFE_NEW(v, t)	{ (v) = new (t); ZeroMemory((v), sizeof(t)); }

// cleanup macros
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }

// bit macros
#define	TESTBIT(a,b)	((a&b)?(1):(0))					// tests if a bit is true, or false
#define	SETBIT(a,b)		(a|b)							// sets a bit
#define	SWITCHBIT(a,b)	(a^b)							// toggles a bit
#define	CLEARBIT(a,b)	(a&(0xffffffff^b))				// clears a bit

#define PRINTTREE()		{ if (g_pIntegrator) g_pIntegrator->PrintTree(); }

#endif // INC_NXN_MACROS_H
