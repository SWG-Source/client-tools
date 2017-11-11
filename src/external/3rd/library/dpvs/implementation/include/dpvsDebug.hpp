#ifndef __DPVSDEBUG_HPP
#define __DPVSDEBUG_HPP
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
 * Description: 	Debugging code
 *
 * $Archive: /dpvs/implementation/include/dpvsDebug.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 14.06.01 12:51 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif

#if !defined (__DPVSLIBRARY_HPP)
#	include "dpvsLibrary.hpp"
#endif

#if !defined (__DPVSWRAPPER_HPP)
#	include "dpvsWrapper.hpp"
#endif

namespace DPVS
{
class Surface;

/******************************************************************************
 *
 * Class:           DPVS::Debug
 *
 * Description:     Debugging functionality
 *
 *****************************************************************************/

class Debug
{
public:
	static void						clearBufferFlags	(UINT32 flags)					{ s_bufferFlags &= ~flags; }
	static void						clearLineDrawFlags	(UINT32 flags)					{ s_lineDrawFlags &= ~flags; }
	static void						drawLine			(Surface* surf, float x0, float y0, float x1, float y1, UINT32 color);
	static void						drawLine			(Library::LineType, const Vector2& a, const Vector2& b, const Vector4& color);
	static void						drawLine			(Library::LineType, const Vector3& a, const Vector3& b, const Vector4& color);
	static void						exit				(void);
	static UINT32					getBufferFlags		(void)							{ return s_bufferFlags; }
	static Surface&					getDefaultSurface	(void);
	static UINT32					getLineDrawFlags	(void)							{ return s_lineDrawFlags; }
	static void						init				(Library::Services* s);
	static void						print				(const char* format, ... );
	static void						setBufferFlags		(UINT32 flags)					{ s_bufferFlags |= flags;					}
	static void						setLineDrawFlags	(UINT32 flags)					{ s_lineDrawFlags |= flags;					}
	static Library::Services*		getServices			(void)							{ DPVS_ASSERT_INIT(); return s_services;	}
	static void						setServices			(Library::Services* s)			{ s_services = s ? s : &s_defaultServices;	}
private:
	static UINT32					s_lineDrawFlags;	// current line drawing flags
	static UINT32					s_bufferFlags;		// current buffer visualization flags
	static Surface*					s_defaultSurface;	// pointer to "default surface"
	static Library::Services*		s_services;			// pointer to current services
	static Library::Services		s_defaultServices;	// default copy of services
};

} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSDEBUG_HPP


