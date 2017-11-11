#ifndef __DPVSIMPMODEL_HPP
#define __DPVSIMPMODEL_HPP

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
 * Description: 	Model implementation class
 *
 * $Archive: /dpvs/implementation/include/dpvsImpModel.hpp $
 * $Author: wili $ 
 * $Revision: #2 $
 * $Modtime: 7.05.01 12:06 $
 * $Date: 2003/06/19 $
 * 
 ******************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif
#if !defined (__DPVSMODEL_HPP)
#	include "dpvsModel.hpp"
#endif
#if !defined (__DPVSSPHERE_HPP)
#	include "dpvsSphere.hpp"
#endif
#if !defined (__DPVSOBB_HPP)
#	include "dpvsOBB.hpp"
#endif
#if !defined (__DPVSSILHOUETTEMATH_HPP)
#	include "dpvsSilhouetteMath.hpp"
#endif

namespace DPVS
{
class Mesh;
class EdgeSilhouette;
class ImpCamera;
class FloatRectangle;

/******************************************************************************
 *
 * Class:			DPVS::VertexArray
 *
 * Description:		Internal structure for passing a vertex array
 *
 * See Also:		DPVS::ImpModel::getConvexHull()
 *
 *****************************************************************************/

struct VertexArray
{
	const Vector3*	m_vertexPositions;			// vertex positions
	int				m_vertexCount;				// vertex count
};

/******************************************************************************
 *
 * Class:			DPVS::ImpModel
 *
 * Description:		Implementation of Model base class
 *
 *****************************************************************************/

class ImpModel
{
public:
						ImpModel					(void);
	virtual				~ImpModel					(void);

	virtual bool		backFaceCull				(const Vector4&) const;

	void				getDepthRange				(Range<float>& range, const Matrix4x3& objectToCamera, ImpCamera* c) const		{ SilhouetteMath::computeOBBDepthRange(range,objectToCamera,c,m_OBB); }
	const OBB&			getExactOBB					(void);
	virtual	bool		getExactRectangle			(FloatRectangle&);
	virtual float		getGeometryCost				(void) const;
	const OBB&			getOBB						(void);
	bool				getOBBRectangle				(FloatRectangle&);
	bool				getRectangle_IM				(FloatRectangle& r, Range<float>& range, const Matrix4x3& objectToCamera, ImpCamera *c)	{ return SilhouetteMath::getRectangle(r,range,objectToCamera,c,m_OBB); }
	const Sphere&		getSphere					(void) const				{ return m_sphere; }
	virtual	void		getTestMesh					(VertexArray& v) const;
	virtual const Mesh*	getTestMesh					(void) const;
	virtual bool		getTestSilhouette			(EdgeSilhouette &);
	Model*				getUserModel				(void) const				{ return ((Model*)(const_cast<ImpModel*>(this)))-1; /*DPVS_ASSERT(m_userModel && m_userModel->getImplementation()==this); return m_userModel; */}
	virtual bool		getWriteSilhouette			(EdgeSilhouette &, Range<float>&, const Matrix4x3&, ImpCamera*);
	
	bool				isBackFaceCullable			(void) const				{ return m_backFaceCullable; }
	bool				isOcclusionWritable			(void) const				{ return m_occlusionWritable; }
	bool				isSolid						(void) const				{ return m_solid; }

	void				setBackFaceCullable			(bool v)					{ m_backFaceCullable=v; }
	void				setSolid					(bool v)					{ m_solid=v; }

protected:

	friend class OBBModel;	//sets sphere/OBB

	static float		estimateGeometryCost		(int vnum,int tnum);
	virtual void		computeExactOBB				(void);
	void				setOBB						(const OBB& obb, bool exact)	{ m_OBB = obb;  m_exactOBB = exact; }
	void				setSphere					(const Sphere& s)				{ m_sphere = s; }
	void				setOcclusionWritable		(bool v)						{ m_occlusionWritable=v; }

private:
						ImpModel	(const ImpModel &);		// not permitted
	ImpModel&			operator=	(const ImpModel &);		// not permitted

	OBB					m_OBB;								// oriented bounding box
	Sphere				m_sphere;							// bounding sphere
	bool				m_backFaceCullable		: 1;		// is the model back-face cullable?
	bool				m_occlusionWritable		: 1;		// is the write silhouette available?
	bool				m_exactOBB				: 1;		// is the OBB as good as it can be?
	bool				m_solid					: 1;		// is object solid?

};

DPVS_FORCE_INLINE class ImpModel* Model::getImplementation (void) const        
{ 
	return reinterpret_cast<ImpModel*>(const_cast<Model*>(this)+1);	// yeah baby!
}

} //DPVS

//------------------------------------------------------------------------
#endif //__DPVSIMPMODEL_HPP
