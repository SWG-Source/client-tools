#ifndef __DPVSINSTANCECOUNT_HPP
#define __DPVSINSTANCECOUNT_HPP
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
 * Description: 	Class reference-count checking code
 *
 * $Archive: /dpvs/implementation/include/dpvsInstanceCount.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 12:06 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif


namespace DPVS
{

/******************************************************************************
 *
 * Class:           DPVS::InstanceCount
 *
 * Description:     
 *
 *****************************************************************************/

class InstanceCount
{
public:
	enum Class
	{
		CELL					= 0,
		CAMERA					= 1,
		VIEWER					= 2,
		VIRTUALPORTAL			= 3,
		PHYSICALPORTAL			= 4,
		REGIONOFINFLUENCE		= 5,
		OBJECT					= 6,
		MESHMODEL				= 7,
		MODEL					= 8,
		CLASS_MAX
	};

	static void		init				(void);
	static void		exit				(void);

	static void		incInstanceCount	(Class c)						{ ++s_references[c]; }
	static void		decInstanceCount	(Class c)						{ --s_references[c]; DPVS_ASSERT(s_references[c]>=0); }
	static int		getInstanceCount	(Class c)						{ return s_references[c]; }

private:
	InstanceCount();

	static int		s_references[CLASS_MAX];							// # of existing instances
};

} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSINSTANCECOUNT_HPP


