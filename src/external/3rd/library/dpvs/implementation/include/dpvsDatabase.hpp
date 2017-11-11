#ifndef __DPVSDATABASE_HPP
#define __DPVSDATABASE_HPP
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
 *
 * Desc:	Spatial database interface
 *
 * $Archive: /dpvs/implementation/include/dpvsDatabase.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 2.10.02 13:16 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif

/******************************************************************************
 *
 * Class:			DPVS::IDatabase
 *
 * Description:		Spatial database external interface
 *
 ******************************************************************************/

namespace DPVS
{ 
	class ImpObject;
	class AABB;

	class IDatabase
	{
	public:

		static int		getInstanceCount		(void);					// return total number of instances in the world
		static int		getInstanceMemoryUsed	(void);
		static int		getNodeCount			(void);					// return total number of nodes in the world
		static int		getNodeMemoryUsed		(void);					// return total number of nodes in the world

		enum Optimization
		{
			OPTIMIZE_MEMORY_USAGE,										// optimize memory usage of the database
			OPTIMIZE_HIDDEN_AREAS										// optimize hidden areas of the database
		};
						IDatabase			(void);
						~IDatabase			(void);
		void			addObject			(ImpObject*);				
		void			checkConsistency	(void) const;
		int				getObjectDeltaTimeStamp (ImpObject*) const;
		void			optimize			(Optimization o);
		void			removeObject		(ImpObject*);
		void			test				(void) const;
		bool			traverse			();
		void			updateObjectBounds	(ImpObject*);			
	private:
						IDatabase			(const IDatabase&);			// no access
		IDatabase&		operator=			(const IDatabase&);			// no access
		
		class Database*	m_database;										// opaque pointer to hide implementation details
	};
} // namespace DPVS

//------------------------------------------------------------------------
#endif //__DPVSDATABASE_HPP
