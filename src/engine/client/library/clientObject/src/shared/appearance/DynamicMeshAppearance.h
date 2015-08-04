//======================================================================
//
// DynamicMeshAppearance.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_DynamicMeshAppearance_H
#define INCLUDED_DynamicMeshAppearance_H

//======================================================================

#include "sharedCollision/BoxExtent.h"
#include "sharedMath/Sphere.h"
#include "sharedObject/Appearance.h"
#include <vector>

class DynamicMeshAppearanceShaderSet;
class Sphere;
class DetailAppearance;
class MeshAppearance;
class ShaderPrimitiveSet;
class Plane;
class DynamicMeshAppearanceShaderSetEdge;
class Hardpoint;

namespace DPVS
{
	class Object;
}

//----------------------------------------------------------------------

class DynamicMeshAppearance : public Appearance
{
public:

	class LocalShaderPrimitive;
	typedef stdvector<LocalShaderPrimitive * >::fwd LocalShaderPrimitiveVector;	

	typedef stdvector<DynamicMeshAppearanceShaderSet *>::fwd ShaderSetVector;
	typedef DynamicMeshAppearanceShaderSetEdge Edge;
	typedef stdvector<Edge>::fwd EdgeVector;

	explicit DynamicMeshAppearance (ShaderSetVector & shaderSetVector);
	~DynamicMeshAppearance();

	virtual void render() const;		  
	virtual Sphere const & getSphere() const;
	virtual bool collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const;
	virtual bool implementsCollide() const;

	void setAlphaFade(float alphaFade);

	//----------------------------------------------------------------------
	
	void updateDpvsTestModel();
	DPVS::Object * getDpvsObject() const;

	//----------------------------------------------------------------------

	static bool splitAppearance(Appearance const & appearance, Plane const & plane, Vector const & randomization, DynamicMeshAppearance *& outputAppearanceFront, DynamicMeshAppearance *&outputAppearanceBack, float debrisParticleThreshold);
	static bool splitAppearance(Appearance const & appearance, Plane const & plane, Vector const & randomization, Object *& outputObjectFront, Object *& outputObjectBack, DynamicMeshAppearance *& outputAppearanceFront, DynamicMeshAppearance *&outputAppearanceBack, float debrisParticleThreshold);
	static bool splitDynamicMeshAppearance(DynamicMeshAppearance const & appearance, Plane const & plane, Vector const & randomization, DynamicMeshAppearance *& outputAppearanceFront, DynamicMeshAppearance *&outputAppearanceBack);
	static bool splitDetailAppearance(DetailAppearance const & appearance, Plane const & plane, Vector const & randomization, Object *& outputObjectFront, Object *& outputObjectBack, DynamicMeshAppearance *& outputAppearanceFront, DynamicMeshAppearance *&outputAppearanceBack, float debrisParticleThreshold);
	static bool splitMeshAppearance(MeshAppearance const & appearance, Plane const & plane, Vector const & randomization, DynamicMeshAppearance *& outputAppearanceFront, DynamicMeshAppearance *&outputAppearanceBack);
	static bool splitShaderPrimitiveSet(ShaderPrimitiveSet const & appearance, Plane const & plane, Vector const & randomization, DynamicMeshAppearance *& outputAppearanceFront, DynamicMeshAppearance *&outputAppearanceBack);
	static bool splitComponentAppearance(ComponentAppearance const & componentAppearance, Plane const& plane, Vector const & randomization, Object *& outputObjectFront, Object *& outputObjectBack, float debrisParticleThreshold);

	EdgeVector const * getSplitEdgeVector() const;
	
	typedef stdvector<Hardpoint*>::fwd HardpointList;

	virtual bool findHardpoint(CrcString const &hardpointName, Transform &hardpointTransform) const;
	virtual int getHardpointCount() const;
	virtual int getHardpointIndex(CrcString const &hardpointName, bool optional = false) const;
	virtual const Hardpoint & getHardpoint(int index) const;

	virtual AxialBox const getTangibleExtent() const;

	static void setDetailLevelBias(int bias);
	static int getDetailLevelBias();

	Vector const & getSplitEdgeCenter() const;

	static void attachDebrisParticles(Object & debrisObject, Vector const & planeNormal, float thresholdRadius);

private:
	
	DynamicMeshAppearance(DynamicMeshAppearance const & rhs);
	DynamicMeshAppearance & operator=(DynamicMeshAppearance const & rhs);

	void createSphere();
	
private:
	Sphere m_sphere;
	BoxExtent m_boxExtent;
	DPVS::Object * m_dpvsObject;
	LocalShaderPrimitiveVector m_localShaderPrimitiveVector;

	EdgeVector * m_splitEdgeVector;

	float m_alphaFade;

	HardpointList * m_hardpointList;
	Vector m_splitEdgeCenter;
};

//======================================================================

#endif
