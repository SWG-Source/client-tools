#ifndef INC_NXN_API_H
#define INC_NXN_API_H

/* \file		NxNApi.h
 *
 *  \brief		This file provides the concerning define which api is installed
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-29-Aug-2000 file created.
 *	\endmod
 */

#include "NxNLocal.h"

#ifdef NXN_SQUADRON
#	define NXN_MEDIALIZER_API
#else
#	define NXN_ALIENBRAIN_API
#endif

#if defined(DEBUG) || defined(_DEBUG)
#   define NXN_DEBUG
#endif


#endif // INC_NXN_API_H
