#ifndef __DPVSDEFS_HPP
#define __DPVSDEFS_HPP
/*****************************************************************************
 *
 * dPVS
 * -----------------------------------------
 *
 * (C) 1999-2004 Hybrid Graphics, Ltd.
 * All Rights Reserved.
 *
 * Dynamic PVS and dPVS are trademarks of Criterion Software Ltd.
 *
 * This file consists of unpublished, proprietary source code of
 * Hybrid Graphics, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Hybrid Graphics and legal action against the party in breach.
 *
 * Description:     Common type definitions for all classes of DPVS
 *
 * $Id: //depot/products/dpvs/interface/dpvsDefs.hpp#4 $
 * $Date: 2004/11/12 $
 * $Author: wili $
 * 
 ******************************************************************************/

#ifndef __cplusplus
#   error "C++ compiler required"
#endif

#include <stddef.h>                                             // required for size_t on some platforms

//------------------------------------------------------------------------
// Define some internal macros affecting how the library is compiled
//------------------------------------------------------------------------

#if defined (DPVS_DLL)							// Dynamic Link Library
#   if defined (DPVS_BUILD_LIBRARY)
#       define DPVSDEC __declspec(dllexport)
#   else
#       define DPVSDEC __declspec(dllimport)
#   endif
#else
#   define DPVSDEC								// Static Library
#endif

namespace DPVS
{

//------------------------------------------------------------------------
// Portable definition of a 32-bit integer. Most of the DPVS API
// uses 'ints' that can be 32 or 64 bits depending on the target
// platform. However, some large structures must always use 32-bit
// integers. Note that these definitions are inside the DPVS namespace
// so they should not conflict with the user's code.
//------------------------------------------------------------------------

typedef int             INT32;                  // 32-bit signed integer
typedef unsigned int    UINT32;                 // 32-bit unsigned integer

//------------------------------------------------------------------------
// Frustum class used for passing view-frustum information to several
// API functions.
//------------------------------------------------------------------------

struct Frustum                                          
{
    DPVSDEC Frustum();

	enum Type
	{
		PERSPECTIVE		= 0,		// the frustum describes a perspective projection
		ORTHOGRAPHIC	= 1			// the frustum describes an orthographic projection
	};

    float left;        // frustum left value
    float right;       // frustum right value
    float top;         // frustum top value
    float bottom;      // frustum bottom value
    float zNear;       // frustum near value in range ]0,zFar[
    float zFar;        // frustum far value in range ]zNear,infinity]
	Type  type;		   // PERSPECTIVE (default) or ORTHOGRAPHIC
};

//------------------------------------------------------------------------
// If DPVS_OVERRIDE_VECTOR_TYPES macro has been defined, the vector and
// matrix classes are not defined here - the user must provide his own 
// implementation of the classes below. The data layout of the vectors and
// matrices must be bit-exact with the following definitions.
//------------------------------------------------------------------------

#if !defined (DPVS_OVERRIDE_VECTOR_TYPES) 

class Vector2    { public: float v[2];			};
class Vector3    { public: float v[3];			};
class Vector3d   { public: double v[3];			};
class Vector3i   { public: INT32 i,j,k;			};
class Vector4    { public: float v[4];			};
class Vector4d   { public: double v[4];			};
class Matrix4x4  { public: float m[4][4];		};
class Matrix4x4d { public: double m[4][4];		};

#endif // !DPVS_OVERRIDE_VECTOR_TYPES

} //DPVS

//------------------------------------------------------------------------
#endif // __DPVSDEFS_HPP
