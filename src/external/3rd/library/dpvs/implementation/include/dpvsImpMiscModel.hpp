#ifndef __DPVSIMPMISCMODEL_HPP
#define __DPVSIMPMISCMODEL_HPP
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
 * Description: 	Miscellaneous Model classes..
 *
 * $Archive: /dpvs/implementation/include/dpvsImpMiscModel.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 12:06 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSIMPMODEL_HPP)
#	include "dpvsImpModel.hpp"
#endif

namespace DPVS
{
/******************************************************************************
 *
 * Class:			DPVS::ImpOBBModel
 *
 * Description:		OBB model
 *
 *****************************************************************************/

class ImpOBBModel : public ImpModel
{
public:
						ImpOBBModel			(const Matrix4x3& obb);
						ImpOBBModel			(const Vector3* vertices, int numVertices);
	virtual				~ImpOBBModel		(void);

private:
						ImpOBBModel			(const ImpOBBModel &);	// not permitted
	ImpOBBModel&		operator=			(const ImpOBBModel &);	// not permitted
};

/******************************************************************************
 *
 * Class:			DPVS::ImpSphereModel
 *
 * Description:		Sphere model
 *
 *****************************************************************************/

class ImpSphereModel : public ImpModel
{
public:
						ImpSphereModel		(const Vector3& center,float radius);
	virtual				~ImpSphereModel		(void);

private:
						ImpSphereModel		(const ImpSphereModel &);	// not permitted
	ImpSphereModel&		operator=			(const ImpSphereModel &);	// not permitted
};

} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSIMPMISCMODEL_HPP
