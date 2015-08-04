#ifndef __DPVSREMOTEDEBUGGER_HPP
#define __DPVSREMOTEDEBUGGER_HPP
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
 * Description: 	RemoteDebugger API
 *
 * $Archive: /dpvs/implementation/include/dpvsRemoteDebugger.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 4/26/02 14:24 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif

//#define DPVS_USE_REMOTE_DEBUGGER

namespace DPVS
{
	
/*****************************************************************************
 *
 * Class:			RemoteDebuggerWrapper
 *
 * Description:		
 *
 * Notes:			
 *
 *****************************************************************************/

class RemoteDebuggerWrapper
{
public:
							RemoteDebuggerWrapper	(void);
							~RemoteDebuggerWrapper	(void);

	static void				init					(void);
	static void				shutdown				(void);
	static void				update					(void);
	static void				drawLine				(const Vector2& a, const Vector2& b, const Vector4& color);
	static void				setScreenSize			(int width, int height);

private:
							RemoteDebuggerWrapper	(const RemoteDebuggerWrapper&);		// not allowed!
	RemoteDebuggerWrapper&	operator=				(const RemoteDebuggerWrapper&);		// not allowed!

	//--------------------------------------------------------------------
	// member variables
	//--------------------------------------------------------------------
};


} // DPVS

//------------------------------------------------------------------------

#endif // __DPVSREMOTEDEBUGGER_HPP

