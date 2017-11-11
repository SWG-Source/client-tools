
#ifndef INC_NXN_STRING_H
#define INC_NXN_STRING_H

/*	\class		CNxNString
 *
 *	\file		NxNString.h
 *
 *  \brief		---
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-02-May-2000 file created.
 *	\endmod
 */

//
// This file was changed during internationalization on 12/18/2000 by Jens.

//disable warnings on 255 char debug symbols
#pragma warning (disable : 4786)

//disable warnings on extern before template instantiation
#pragma warning (disable : 4231)

#include "NxNBSTR.h"

//---------------------------------------------------------------------------
//	string related helper macros
//---------------------------------------------------------------------------
#define _STR(strString)		CNxNString(strString)

//---------------------------------------------------------------------------
//	typedef for a string list
//---------------------------------------------------------------------------
typedef CNxNArray<CNxNString>           TNxNStringList;

#endif // INC_NXN_STRING_H
