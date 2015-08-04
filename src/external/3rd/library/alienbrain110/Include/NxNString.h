
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
 *		[ap]-05-02-2000 file created.
 *	\endmod
 */

//disable warnings on 255 char debug symbols
#pragma warning (disable : 4786)

//disable warnings on extern before template instantiation
#pragma warning (disable : 4231)

#include "macros.h"
#include "NxNBSTR.h"

#define _STR(strString)		CNxNString(strString)


#endif // INC_NXN_STRING_H
