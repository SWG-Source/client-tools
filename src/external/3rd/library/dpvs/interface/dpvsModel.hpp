#ifndef __DPVSMODEL_HPP
#define __DPVSMODEL_HPP
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
 * Description:     Model interfaces
 *
 * $Id: //depot/products/dpvs/interface/dpvsModel.hpp#4 $
 * $Date: 2004/11/12 $
 * $Author: wili $
 * 
 ******************************************************************************/

#if !defined (__DPVSREFERENCECOUNT_HPP)
#   include "dpvsReferenceCount.hpp"
#endif

namespace DPVS
{
/******************************************************************************
 *
 * Class:           DPVS::Model
 *
 * Description:     Base class for all models in DPVS
 *
 * Notes:           Models are used to describe shapes of objects. They don't
 *                  "exist" by themselves - in order to physically place a model
 *                  into a world, an DPVS::Object must be created to provide
 *                  the location and orientation information. Models can
 *                  be shared by multiple objects.
 *
 * See Also:        DPVS::MeshModel, DPVS::OBBModel, DPVS::SphereModel
 *
 *****************************************************************************/

class Model : public ReferenceCount
{
public:
    enum Property
    {
        BACKFACE_CULLABLE   = 1,                        // model can be FULLY backface culled
		SOLID				= 2							// model is solid (see reference manual)
    };

	DPVSDEC void		getAABB				(Vector3& mn, Vector3& mx) const;
	DPVSDEC void		getOBB				(Matrix4x4&) const;
	DPVSDEC void		getSphere			(Vector3& center, float& radius) const;
    DPVSDEC bool		test                (Property) const;
    DPVSDEC void		set                 (Property, bool);
    class ImpModel*     getImplementation   (void) const;
protected:
                        Model               (class ImpReferenceCount*);	// internal
	virtual void		destruct			(void) const;
private:
                        Model               (const Model&); // not permitted
    Model&              operator=           (const Model&); // not permitted
};

/******************************************************************************
 *
 * Class:           DPVS::MeshModel
 *
 * Description:     A model type where topology is described using triangles and vertices
 *
 * Notes:           This is the most common model type used. Note that when
 *                  using DPVS::MeshModel as a test model, any simplified
 *                  model that is (at least) larger than the true model can
 *                  be used. When using them as write models, the shape
 *                  must be conservatively smaller - otherwise artifacts will
 *                  occur.
 *
 *****************************************************************************/

class MeshModel : public Model
{
public:
	static DPVSDEC MeshModel*	create				(const Vector3* vertices, const Vector3i* triangles,int numVertices,int numTriangles, bool clockwise = true);
protected:
								MeshModel			(class ImpReferenceCount*);	// internal
private:
								MeshModel           (const MeshModel&);			// not permitted
    MeshModel&					operator=           (const MeshModel&);			// not permitted
};

/******************************************************************************
 *
 * Class:           DPVS::OBBModel
 *
 * Description:     A model type describing an oriented bounding box. This
 *					class is also used to describe an axis-aligned bounding
 *					box (there's no separate AABBModel class).
 *
 *****************************************************************************/

class OBBModel : public Model
{
public:
	static DPVSDEC OBBModel*	create				(const Matrix4x4& obb);
	static DPVSDEC OBBModel*	create				(const Vector3* vertices, int numVertices);
	static DPVSDEC OBBModel*	create				(const Vector3& mn, const Vector3& mx);
protected:
								OBBModel			(class ImpReferenceCount*);		// internal
private:									
								OBBModel			(const OBBModel&);		// not permitted
    OBBModel&					operator=			(const OBBModel&);		// not permitted
};

/******************************************************************************
 *
 * Class:           DPVS::SphereModel
 *
 * Description:     A model type describing a sphere with a center position and 
 *					a radius
 *
 *****************************************************************************/

class SphereModel : public Model
{
public:
	static DPVSDEC SphereModel*		create				(const Vector3& center,   float radius);
	static DPVSDEC SphereModel*		create				(const Vector3* vertices, int numVertices);
protected:	
									SphereModel			(class ImpReferenceCount*);	// internal
private:
									SphereModel         (const SphereModel&);   // not permitted
    SphereModel&					operator=           (const SphereModel&);   // not permitted
};

} //DPVS

//------------------------------------------------------------------------
#endif //__DPVSMODEL_HPP
