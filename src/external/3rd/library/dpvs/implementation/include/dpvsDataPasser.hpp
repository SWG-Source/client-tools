#ifndef __DPVSDATAPASSER_HPP
#define __DPVSDATAPASSER_HPP
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
 * Description: 	A class for sharing some "global" data
 *
 * $Archive: /dpvs/implementation/include/dpvsDataPasser.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 14.06.01 12:48 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif

namespace DPVS
{

class Commander;
class ImpCommander;
class ImpCamera;

/*****************************************************************************
 *
 * Class:			DPVS::DataPasser
 *
 * Description:		Class for storing some "global data" needed by many
 *					different classes during a visibility query. Only
 *					a single instance of DataPasser exists.
 *
 *****************************************************************************/

class DataPasser
{
public:
	static Commander*			getCommander		(void)							{ return s_commander; }
	static int					getFrameCounter		(void)							{ return s_frameCounter; }
	static ImpCommander*		getImpCommander		(void);
	static float				getObjectImportance	(void)							{ return s_objectImportance; }
	static float				getQueryTime		(void)							{ return s_queryTime; }
	static inline ImpCamera*	getViewerCamera		(void)							{ return s_viewerCamera; }

	static void					setCommander		(class Commander* c)			{ s_commander = c; }
	static void					setFrameCounter		(int n)							{ s_frameCounter = n; }
	static void					setObjectImportance	(float d)						{ s_objectImportance = d; }
	static void					setQueryTime		(float t)						{ s_queryTime = t; }
	static inline void			setViewerCamera		(ImpCamera* c)					{ s_viewerCamera=c; }
	
private:
	DataPasser(); // Cannot make instances
	static class Commander*		s_commander;			// current commander (or null)
	static int					s_frameCounter;			// global frame counter
	static ImpCamera*			s_viewerCamera;			// pointer to current camera
	static float				s_queryTime;			// time at beginning of query (in seconds)
	static float				s_objectImportance;		// current object importance
};

} // DPVS

//------------------------------------------------------------------------
#endif //__DPVSDATAPASSER_HPP
