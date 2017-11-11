#ifndef __DPVSIMPMESHMODEL_HPP
#define __DPVSIMPMESHMODEL_HPP

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
 * Description: 	MeshModel implementation class
 *
 * $Archive: /dpvs/implementation/include/dpvsImpMeshModel.hpp $
 * $Author: wili $ 
 * $Revision: #2 $
 * $Modtime: 10.05.01 14:31 $
 * $Date: 2003/06/19 $
 * 
 ******************************************************************************/

#if !defined (__DPVSIMPMODEL_HPP)
#	include "dpvsImpModel.hpp"
#endif
#if !defined (__DPVSSILHOUETTECACHE_HPP)
#	include "dpvsSilhouetteCache.hpp"
#endif

namespace DPVS
{
/******************************************************************************
 *
 * Class:			DPVS::ImpMeshModel
 *
 * Description:		Implementation of DPVS::MeshModel
 *
 *****************************************************************************/

class ImpMeshModel : public ImpModel, public SilhouetteCache::Client
{
public:
						ImpMeshModel				(const Vector3* l,const Vector3i* i,int vn,int tn,bool clockwise);
	virtual				~ImpMeshModel				(void);	
	virtual bool		backFaceCull				(const Vector4&) const;
	virtual	bool		getExactRectangle			(FloatRectangle&);
	virtual float		getGeometryCost				(void) const;
	virtual const Mesh*	getTestMesh					(void) const;
	virtual	void		getTestMesh					(VertexArray& v) const;
	virtual bool		getWriteSilhouette			(EdgeSilhouette &s, Range<float>&, const Matrix4x3& ObjectToCamera, ImpCamera *c);
protected:
	virtual void		computeExactOBB				(void);
private:								
						ImpMeshModel				(const ImpMeshModel &);	// not permitted
	ImpMeshModel&		operator=					(const ImpMeshModel &);	// not permitted

	bool				frontClipWriteSilhouette	(EdgeSilhouette& s, const Matrix4x3& objectToCamera, const SilhouetteCache::EdgeSilhouetteInfo& inf, ImpCamera *cam);
	void				splatWriteSilhouette		(EdgeSilhouette& s, const Matrix4x3& objectToCamera, const SilhouetteCache::EdgeSilhouetteInfo& inf, ImpCamera *cam);
	bool				testShaftIntersection		(const Matrix4x3& objectToCamera, ImpCamera* cam);
	static Vector4		getCameraInverseTranslation	(const Matrix4x3& objectToCamera);
};

} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSIMPMESHMODEL_HPP
