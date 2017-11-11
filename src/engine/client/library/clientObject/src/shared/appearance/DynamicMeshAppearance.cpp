//======================================================================
//
// DynamicMeshAppearance.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/DynamicMeshAppearance.h"

#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSet.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticIndexBuffer.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/StaticVertexBuffer.h"
#include "clientObject/ComponentAppearance.h"
#include "clientObject/DetailAppearance.h"
#include "clientObject/DynamicMeshAppearanceShaderSet.h"
#include "clientObject/MeshAppearance.h"
#include "clientObject/ObjectListCamera.h"
#include "dpvsModel.hpp"
#include "dpvsObject.hpp"
#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/ExtentList.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Plane.h"
#include "sharedMath/Sphere.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Hardpoint.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectList.h"
#include <algorithm>
#include <vector>

//======================================================================

namespace DynamicMeshAppearanceNamespace
{
	//lint -e751 //Edge accessed
	typedef DynamicMeshAppearanceShaderSet::Edge Edge;
	typedef DynamicMeshAppearanceShaderSet::EdgeVector EdgeVector;

	bool s_installed = false;
	bool s_showSplitEdge = false;
	bool s_showSpheresPrimitives = false;
	bool s_showSpheresAppearances = false;
	bool s_showOriginAppearances = false;

	int s_detailLevelBias = 2;

	void install()
	{
		if (s_installed)
			return;

		s_installed = true;
		DebugFlags::registerFlag(s_showSplitEdge, "ClientObject/DynamicMeshAppearance", "showSplitEdge");
		DebugFlags::registerFlag(s_showSpheresPrimitives, "ClientObject/DynamicMeshAppearance", "showSpheresPrimitives");
		DebugFlags::registerFlag(s_showSpheresAppearances, "ClientObject/DynamicMeshAppearance", "showSpheresAppearances");
		DebugFlags::registerFlag(s_showOriginAppearances, "ClientObject/DynamicMeshAppearance", "showOriginsAppearances");
	}
	
	//----------------------------------------------------------------------
	
	Vector computeEdgeVectorCenter(DynamicMeshAppearance::EdgeVector const & edgeVector)
	{
		Vector result;
		if (!edgeVector.empty())
		{
			for (EdgeVector::const_iterator it = edgeVector.begin(); it != edgeVector.end(); ++it)
			{
				DynamicMeshAppearance::Edge const & edge = *it;
				result += edge.v[0];
				result += edge.v[1];
			}
			
			result /= static_cast<float>(edgeVector.size() * 2);
		}
		
		return result;		
	}
}

using namespace DynamicMeshAppearanceNamespace;

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

//----------------------------------------------------------------------

class DynamicMeshAppearance::LocalShaderPrimitive : public ShaderPrimitive
{
public:
	
	static VertexBufferFormat getVertexBufferFormat();
	
public:

	LocalShaderPrimitive(DynamicMeshAppearance const & owner, DynamicMeshAppearanceShaderSet * shaderSet);
	virtual ~LocalShaderPrimitive();
	
	virtual float alter(float time);
	virtual const Vector getPosition_w() const;
	virtual float getDepthSquaredSortKey() const;
	virtual int getVertexBufferSortKey() const;
	virtual const StaticShader &prepareToView() const;
	virtual void prepareToDraw() const;
	virtual void draw() const;
	virtual bool collide(const Vector &start_o, const Vector &end_o, CollisionInfo &result) const;
	
	const Sphere &getSphere() const;
	const BoxExtent & getBoxExtent() const;
	DynamicMeshAppearanceShaderSet const & getShaderSet() const;
		
private:
	
	const DynamicMeshAppearance &m_owner;
	Sphere m_sphere;
	BoxExtent m_boxExtent;
	DynamicMeshAppearanceShaderSet * m_shaderSet;
	
private:
	
	LocalShaderPrimitive();
	LocalShaderPrimitive(const LocalShaderPrimitive &);             //lint -esym(754, LocalShaderPrimitive::LocalShaderPrimitive) // not referenced // defensive hiding
	LocalShaderPrimitive &operator =(const LocalShaderPrimitive &);
};

//----------------------------------------------------------------------

DynamicMeshAppearanceShaderSet const & DynamicMeshAppearance::LocalShaderPrimitive::getShaderSet() const
{
	return *NON_NULL(m_shaderSet);
}

//===================================================================

VertexBufferFormat DynamicMeshAppearance::LocalShaderPrimitive::getVertexBufferFormat()
{
	VertexBufferFormat format;
	format.setPosition();
	format.setColor0();
	format.setNumberOfTextureCoordinateSets(1);
	format.setTextureCoordinateSetDimension(0, 2);
	return format;
}

//===================================================================

/**
* This method takes ownership of the shaderSet
*/

DynamicMeshAppearance::LocalShaderPrimitive::LocalShaderPrimitive(DynamicMeshAppearance const & owner, DynamicMeshAppearanceShaderSet * shaderSet) :
ShaderPrimitive(),
m_owner(owner),
m_sphere(),
m_boxExtent(),
m_shaderSet(NON_NULL(shaderSet))
{
	m_boxExtent.setBox(shaderSet->calculateAxialBox());
	m_sphere.set(m_boxExtent.getCenter(), m_boxExtent.getRadius());
}

// ----------------------------------------------------------------------

DynamicMeshAppearance::LocalShaderPrimitive::~LocalShaderPrimitive()
{
	delete m_shaderSet;
	m_shaderSet = NULL;
}

// ----------------------------------------------------------------------

float DynamicMeshAppearance::LocalShaderPrimitive::alter(float time)
{
	return NON_NULL(m_shaderSet->getShader())->alter(time);
}

// ----------------------------------------------------------------------

const Vector DynamicMeshAppearance::LocalShaderPrimitive::getPosition_w() const
{
	return m_owner.getTransform_w().getPosition_p();
}

//-------------------------------------------------------------------

float DynamicMeshAppearance::LocalShaderPrimitive::getDepthSquaredSortKey() const
{
	return m_owner.getTransform_w().getPosition_p().magnitudeBetweenSquared(ShaderPrimitiveSorter::getCurrentCameraPosition());
}

// ----------------------------------------------------------------------

int DynamicMeshAppearance::LocalShaderPrimitive::getVertexBufferSortKey() const
{
	return NON_NULL(m_shaderSet->getVertexBuffer())->getSortKey();
}

// ----------------------------------------------------------------------

const StaticShader &DynamicMeshAppearance::LocalShaderPrimitive::prepareToView() const
{
	return NON_NULL(m_shaderSet->getShader())->prepareToView();
}

// ----------------------------------------------------------------------

void DynamicMeshAppearance::LocalShaderPrimitive::prepareToDraw() const
{
	Transform const & transform_a2w = m_owner.getTransform_w();
	Graphics::setObjectToWorldTransformAndScale(transform_a2w, Vector::xyz111);

	Graphics::setVertexBuffer(*NON_NULL(m_shaderSet->getVertexBuffer()));
	Graphics::setIndexBuffer(*NON_NULL(m_shaderSet->getIndexBuffer()));
}

// ----------------------------------------------------------------------

void DynamicMeshAppearance::LocalShaderPrimitive::draw() const
{
	GlCullMode const oldCullMode = Graphics::getCullMode();
	Graphics::setCullMode(GCM_none);

	Graphics::drawIndexedTriangleList();

	Graphics::setCullMode(oldCullMode);
}

// ----------------------------------------------------------------------

inline const Sphere &DynamicMeshAppearance::LocalShaderPrimitive::getSphere() const
{
	return m_sphere;
}

//----------------------------------------------------------------------

inline const BoxExtent & DynamicMeshAppearance::LocalShaderPrimitive::getBoxExtent() const
{
	return m_boxExtent;
}

//----------------------------------------------------------------------

bool DynamicMeshAppearance::LocalShaderPrimitive::collide(const Vector &start_o, const Vector &end_o, CollisionInfo &result) const
{
	if (!m_boxExtent.intersect(start_o, end_o))
		return false;

	bool found = false;
	
	result.setPoint (end_o);
	
	NOT_NULL(m_shaderSet);
	NOT_NULL(m_shaderSet->getShader());
	
	if (m_shaderSet->getShader()->getShaderTemplate().isCollidable() && m_boxExtent.intersect(start_o, result.getPoint ()))
	{
		NOT_NULL(m_shaderSet->getVertexBuffer());
		NOT_NULL(m_shaderSet->getIndexBuffer());

		m_shaderSet->getVertexBuffer()->lockReadOnly();
		m_shaderSet->getIndexBuffer()->lockReadOnly();
		
		VertexBufferReadIterator vi = m_shaderSet->getVertexBuffer()->beginReadOnly();
		Index const * indices = m_shaderSet->getIndexBuffer()->beginReadOnly();
		int const numberOfFaces = m_shaderSet->getIndexBuffer()->getNumberOfIndices() / 3;
		
		Vector normal;
		Plane plane;
		Vector intersection;
		
		for (int i = 0; i < numberOfFaces; i++)
		{			
			Vector const & v0 = (vi + *indices++).getPosition();
			Vector const & v1 = (vi + *indices++).getPosition();
			Vector const & v2 = (vi + *indices++).getPosition();
			
			//-- compute normal(scale by 100 to fix collision detection with small polygons)
			normal =(v0 - v2).cross(v1 - v0);
			normal *= 100.f;
			
			//-- don't ignore backfaces!
			
			//-- it doesn't matter that the normal is not normalized
			plane.set(normal, v0);
			
			//-- see if the end points intersect the plane the polygon lies on, lies within the polygon, and is closer to start than the previous point
			if ((plane.findIntersection(start_o, result.getPoint(), intersection)) &&
				(start_o.magnitudeBetweenSquared(intersection) < start_o.magnitudeBetweenSquared(result.getPoint())) &&
				(intersection.inPolygon(v0, v1, v2)))
			{
				
				found = true;
				
				IGNORE_RETURN(normal.normalize());
				
				result.setPoint(intersection);
				result.setNormal(normal);
			}
		}
		
		m_shaderSet->getVertexBuffer()->unlock();
		m_shaderSet->getIndexBuffer()->unlock();
	}

	return found;
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

/**
* This method takes ownership of the contenst of shaderSetVector
*/

DynamicMeshAppearance::DynamicMeshAppearance (ShaderSetVector & shaderSetVector) :
Appearance(NULL),
m_sphere(),
m_boxExtent(),
m_dpvsObject(NULL),
m_localShaderPrimitiveVector(),
m_splitEdgeVector(new EdgeVector),
m_alphaFade(1.0f),
m_hardpointList(new HardpointList),
m_splitEdgeCenter()
{
	if (!s_installed)
		DynamicMeshAppearanceNamespace::install();

	m_localShaderPrimitiveVector.reserve(shaderSetVector.size());
	for (ShaderSetVector::const_iterator it = shaderSetVector.begin(); it != shaderSetVector.end(); ++it)
	{
		DynamicMeshAppearanceShaderSet * const shaderSet = *it;
		LocalShaderPrimitive * const localShaderPrimitive = new LocalShaderPrimitive(*this, shaderSet);
		m_localShaderPrimitiveVector.push_back(localShaderPrimitive);
	}

	shaderSetVector.clear();

	createSphere();

	updateDpvsTestModel();
}

//----------------------------------------------------------------------

DynamicMeshAppearance::~DynamicMeshAppearance()
{
	std::for_each(m_localShaderPrimitiveVector.begin(), m_localShaderPrimitiveVector.end(), PointerDeleter());
	m_localShaderPrimitiveVector.clear();

	delete m_splitEdgeVector;
	m_splitEdgeVector = NULL;

	delete m_hardpointList;
	m_hardpointList = NULL;

	m_dpvsObject->release();
	m_dpvsObject = NULL;
}

//----------------------------------------------------------------------

void DynamicMeshAppearance::createSphere()
{
	AxialBox box;

	for (LocalShaderPrimitiveVector::const_iterator it = m_localShaderPrimitiveVector.begin(); it != m_localShaderPrimitiveVector.end(); ++it)
	{
		LocalShaderPrimitive const * localShaderPrimitive = *it;
		box.add(localShaderPrimitive->getBoxExtent().getBox());
	}

	m_sphere.set(box.getCenter(), box.getRadius());
	m_boxExtent.setBox(box);
}

//----------------------------------------------------------------------

void DynamicMeshAppearance::render() const
{
	{
		for (LocalShaderPrimitiveVector::const_iterator it = m_localShaderPrimitiveVector.begin(); it != m_localShaderPrimitiveVector.end(); ++it)
		{
			LocalShaderPrimitive const * localShaderPrimitive = *it;

			if (m_alphaFade < 1.0f)
				ShaderPrimitiveSorter::addWithAlphaFadeOpacity(*localShaderPrimitive, true, m_alphaFade, true, m_alphaFade);
			else
				ShaderPrimitiveSorter::add(*localShaderPrimitive);

#ifdef _DEBUG
			if (s_showSpheresPrimitives)
				ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive (new SphereDebugPrimitive(UtilityDebugPrimitive::S_z, getOwner()->getTransform_o2w(), localShaderPrimitive->getSphere().getCenter(), localShaderPrimitive->getSphere().getRadius(), 16, 16));
#endif
		}
	}

#ifdef _DEBUG
	if (s_showSplitEdge)
	{
		for (EdgeVector::const_iterator it = m_splitEdgeVector->begin(); it != m_splitEdgeVector->end(); ++it)
		{
			Edge const & edge = *it;
			ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive (new Line3dDebugPrimitive(UtilityDebugPrimitive::S_z, getOwner()->getTransform_o2w(), edge.v[0], edge.v[1], VectorArgb::solidMagenta));
		}
	}

	if (s_showSpheresAppearances)
		ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive (new SphereDebugPrimitive(UtilityDebugPrimitive::S_z, getOwner()->getTransform_o2w(), m_sphere.getCenter(), m_sphere.getRadius(), 16, 16));

	if (s_showOriginAppearances)
	{
		Transform o2w = getOwner()->getTransform_o2w();
		o2w.move_l(m_sphere.getCenter());
		ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive (new FrameDebugPrimitive(UtilityDebugPrimitive::S_z, o2w, m_sphere.getRadius()));
	}	
#endif
}

//----------------------------------------------------------------------

Sphere const & DynamicMeshAppearance::getSphere() const
{
	return m_sphere;
}

//----------------------------------------------------------------------

void DynamicMeshAppearance::updateDpvsTestModel()
{			  
	Extent * const e = new Extent();
	e->setSphere(m_sphere);
	setExtent(ExtentList::fetch(e));
	
	{
		DPVS::Model * const sphereModel = RenderWorld::fetchSphereModel(getSphere());
		m_dpvsObject = RenderWorld::createObject(this, sphereModel);
		IGNORE_RETURN(sphereModel->release());
	}
} //lint !e429 //e ptr

// ----------------------------------------------------------------------

DPVS::Object * DynamicMeshAppearance::getDpvsObject() const
{
	return m_dpvsObject;
} //lint !e1763 //const func

//----------------------------------------------------------------------

bool DynamicMeshAppearance::splitAppearance(Appearance const & appearance, Plane const & plane, Vector const & randomization, Object *& outputObjectFront, Object *& outputObjectBack, DynamicMeshAppearance *& outputAppearanceFront, DynamicMeshAppearance *&outputAppearanceBack, float const debrisParticleThreshold)
{
	MeshAppearance const * const meshAppearance = dynamic_cast<MeshAppearance const *>(&appearance);
	if (meshAppearance != NULL)
		return splitMeshAppearance(*meshAppearance, plane, randomization, outputAppearanceFront, outputAppearanceBack);
	
	DetailAppearance const * const detailAppearance = dynamic_cast<DetailAppearance const *>(&appearance);
	if (detailAppearance != NULL)
		return splitDetailAppearance(*detailAppearance, plane, randomization, outputObjectFront, outputObjectBack, outputAppearanceFront, outputAppearanceBack, debrisParticleThreshold);

	DynamicMeshAppearance const * const dynamicMeshAppearance = dynamic_cast<DynamicMeshAppearance const *>(&appearance);
	if (dynamicMeshAppearance != NULL)
		return splitDynamicMeshAppearance(*dynamicMeshAppearance, plane, randomization, outputAppearanceFront, outputAppearanceBack);
	
	ComponentAppearance const * const componentAppearance = appearance.asComponentAppearance();
	if (NULL != componentAppearance)
		return splitComponentAppearance(*componentAppearance, plane, randomization, outputObjectFront, outputObjectBack, debrisParticleThreshold);

	return false;
}

//----------------------------------------------------------------------

bool DynamicMeshAppearance::splitAppearance(Appearance const & appearance, Plane const & plane, Vector const & randomization, DynamicMeshAppearance *& outputAppearanceFront, DynamicMeshAppearance *&outputAppearanceBack, float const debrisParticleThreshold)
{
	Object * outputObjectFront = NULL;
	Object * outputObjectBack = NULL;
	return DynamicMeshAppearance::splitAppearance(appearance, plane, randomization, outputObjectFront, outputObjectBack, outputAppearanceFront, outputAppearanceBack, debrisParticleThreshold);
}

//----------------------------------------------------------------------

bool DynamicMeshAppearance::splitComponentAppearance(ComponentAppearance const & componentAppearance, Plane const& plane, Vector const & randomization, Object *& outputObjectFront, Object *& outputObjectBack, float debrisParticleThreshold)
{
	ObjectList const & objectList = componentAppearance.getObjectList();
	int const numberOfComponentObjects = objectList.getNumberOfObjects();
	
	for (int i = 0; i < numberOfComponentObjects; ++i)
	{
		Object * componentFront = NULL;
		Object * componentBack = NULL;
		DynamicMeshAppearance * appearanceComponentResultFront = NULL;
		DynamicMeshAppearance * appearanceComponentResultBack = NULL;
		
		Object const * const componentSubObject = NON_NULL(objectList.getObject(i));
		Appearance const * const appearanceComponentSubObject = componentSubObject->getAppearance();

		if (NULL == appearanceComponentSubObject)
			continue;

		if (splitAppearance(*appearanceComponentSubObject, plane, randomization, componentFront, componentBack, appearanceComponentResultFront, appearanceComponentResultBack, debrisParticleThreshold))
		{
			if (NULL != appearanceComponentResultFront)
			{
				componentFront = new Object;
				componentFront->setAppearance(appearanceComponentResultFront);
				DynamicMeshAppearance::attachDebrisParticles(*componentFront, -plane.getNormal(), debrisParticleThreshold);
			}

			if (NULL != componentFront)
			{
				if (NULL == outputObjectFront)
					outputObjectFront = new Object;
				
				outputObjectFront->addChildObject_o(componentFront);
				componentFront->setTransform_o2p(componentSubObject->getTransform_o2p());
			}
			
			if (NULL != appearanceComponentResultBack)
			{
				componentBack = new Object;
				componentBack->setAppearance(appearanceComponentResultBack);
				DynamicMeshAppearance::attachDebrisParticles(*componentBack, plane.getNormal(), debrisParticleThreshold);
			}

			if (NULL != componentBack)
			{
				if (NULL == outputObjectBack)
					outputObjectBack = new Object;
				
				outputObjectBack->addChildObject_o(componentBack);
				componentBack->setTransform_o2p(componentSubObject->getTransform_o2p());
			}
		}
	} //lint !e429 //custodial componentBack componentFront
	
	if (NULL == outputObjectFront && NULL == outputObjectBack)
		return false;

	return true;
}

//----------------------------------------------------------------------

bool DynamicMeshAppearance::splitDynamicMeshAppearance(DynamicMeshAppearance const & appearance, Plane const & plane, Vector const & randomization, DynamicMeshAppearance *& outputAppearanceFront, DynamicMeshAppearance *&outputAppearanceBack)
{
	DynamicMeshAppearance::ShaderSetVector shaderSetVectorFront;
	DynamicMeshAppearance::ShaderSetVector shaderSetVectorBack;
	
	PerformanceTimer ptimer;
	ptimer.start();
	
	EdgeVector edgeVector;
	edgeVector.reserve(1000);
	
	{
		for (LocalShaderPrimitiveVector::const_iterator it = appearance.m_localShaderPrimitiveVector.begin(); it != appearance.m_localShaderPrimitiveVector.end(); ++it)
		{
			LocalShaderPrimitive const * localShaderPrimitive = *it;
			DynamicMeshAppearanceShaderSet const & shaderSet = localShaderPrimitive->getShaderSet();
	
///			ShaderTemplate const * const shaderTemplate = shaderPrimitiveSet.getShaderTemplate(i);
//			UNREF(shaderTemplate);
			StaticVertexBuffer const * const staticVertexBuffer = shaderSet.getVertexBuffer();
			StaticIndexBuffer const * const staticIndexBuffer = shaderSet.getIndexBuffer();
			Shader const * const shader = shaderSet.getShader();
			
			if (shader == NULL || staticVertexBuffer == NULL || staticIndexBuffer == NULL)
				continue;
			
			DynamicMeshAppearanceShaderSet * shaderSetFront = NULL;
			DynamicMeshAppearanceShaderSet * shaderSetBack = NULL;
			DynamicMeshAppearanceShaderSet::split(randomization, plane, *shader, *staticVertexBuffer, *staticIndexBuffer, shaderSetFront, shaderSetBack, edgeVector);
			
			if (shaderSetFront != NULL)
				shaderSetVectorFront.push_back(shaderSetFront);
			
			if (shaderSetBack != NULL)
				shaderSetVectorBack.push_back(shaderSetBack);
		}
	}
	
	ptimer.stop();
	
#if 0
	ptimer.start();
	
	Shader const * shaderForCapping = NULL;

	if (shaderForCapping != NULL)
	{
		DynamicMeshAppearanceShaderSet * shaderSetFront = NULL;
		DynamicMeshAppearanceShaderSet * shaderSetBack = NULL;
		
		DynamicMeshAppearanceShaderSet::generateCaps(plane, *shaderForCapping, shaderSetFront, shaderSetBack, edgeVector);
		
		if (shaderSetFront != NULL)
			shaderSetVectorFront.push_back(shaderSetFront);
		
		if (shaderSetBack != NULL)
			shaderSetVectorBack.push_back(shaderSetBack);
	}
	
	ptimer.stop();
	totalTimeElapsed = ptimer.getElapsedTime();
	
	DEBUG_REPORT_LOG_PRINT(true, ("DynamicMeshAppearance completed capping in [%5.3f]s, [%d] edges\n", totalTimeElapsed, static_cast<int>(edgeVector.size())));
#endif

	Vector const splitEdgeCenter = computeEdgeVectorCenter(edgeVector);
	if (!shaderSetVectorFront.empty())
	{
		outputAppearanceFront = new DynamicMeshAppearance(shaderSetVectorFront);
		if (s_showSplitEdge)
			*(outputAppearanceFront->m_splitEdgeVector) = edgeVector;

		outputAppearanceFront->m_splitEdgeCenter = splitEdgeCenter;
	}

	if (!shaderSetVectorBack.empty())
	{
		outputAppearanceBack = new DynamicMeshAppearance(shaderSetVectorBack);
		if (s_showSplitEdge)
			*(outputAppearanceBack->m_splitEdgeVector) = edgeVector;

		outputAppearanceBack->m_splitEdgeCenter = splitEdgeCenter;
	}

	return outputAppearanceFront != NULL || outputAppearanceBack != NULL;
}

//----------------------------------------------------------------------

bool DynamicMeshAppearance::splitDetailAppearance(DetailAppearance const & detailAppearance, Plane const & plane, Vector const & randomization, Object *& outputObjectFront, Object *& outputObjectBack, DynamicMeshAppearance *& outputAppearanceFront, DynamicMeshAppearance *&outputAppearanceBack, float const debrisParticleThreshold)
{
	int const detailLevelCount = detailAppearance.getDetailLevelCount();
	if (detailLevelCount > 0)
	{
		int const currentDetailLevel = detailAppearance.getCurrentDetailLevel();
		if (currentDetailLevel >= 0 && currentDetailLevel < detailLevelCount)
		{
			int detailLevelToUse = clamp(0, currentDetailLevel - s_detailLevelBias, detailLevelCount - 1);
			for (;detailLevelToUse < detailLevelCount;++detailLevelToUse)
			{
				Appearance const * const detailSubAppearance = detailAppearance.getAppearance(detailLevelToUse);
				if (NULL != detailSubAppearance && detailSubAppearance->isLoaded())
					return splitAppearance(*detailSubAppearance, plane, randomization, outputObjectFront, outputObjectBack, outputAppearanceFront, outputAppearanceBack, debrisParticleThreshold);
			}
		}
	}
	
	return false;
}

//----------------------------------------------------------------------

bool DynamicMeshAppearance::splitMeshAppearance(MeshAppearance const & meshAppearance, Plane const & plane, Vector const & randomization, DynamicMeshAppearance *& outputAppearanceFront, DynamicMeshAppearance *&outputAppearanceBack)
{
	ShaderPrimitiveSet const * const shaderPrimitiveSet = meshAppearance.getShaderPrimitiveSet();
	if (shaderPrimitiveSet != NULL)
		return splitShaderPrimitiveSet(*shaderPrimitiveSet, plane, randomization, outputAppearanceFront, outputAppearanceBack);
	
	return false;	
}

//----------------------------------------------------------------------

bool DynamicMeshAppearance::splitShaderPrimitiveSet(ShaderPrimitiveSet const & shaderPrimitiveSet, Plane const & plane, Vector const & randomization, DynamicMeshAppearance *& outputAppearanceFront, DynamicMeshAppearance *&outputAppearanceBack)
{
	DynamicMeshAppearance::ShaderSetVector shaderSetVectorFront;
	DynamicMeshAppearance::ShaderSetVector shaderSetVectorBack;
	
	PerformanceTimer ptimer;
	ptimer.start();
	
	EdgeVector edgeVector;
	edgeVector.reserve(1000);
		
	int const numberOfShaders = shaderPrimitiveSet.getNumberOfShaders();
	
	{
		for (int i = 0;	i < numberOfShaders; ++i)
		{
			ShaderTemplate const * const shaderTemplate = shaderPrimitiveSet.getShaderTemplate(i);
			UNREF(shaderTemplate);
			StaticVertexBuffer const * const staticVertexBuffer = shaderPrimitiveSet.getVertexBuffer(i);
			StaticIndexBuffer const * const staticIndexBuffer = shaderPrimitiveSet.getIndexBuffer(i);
			Shader const * const shader = shaderPrimitiveSet.getShader(i);
			
			if (shader == NULL || staticVertexBuffer == NULL || staticIndexBuffer == NULL)
				continue;
						
			DynamicMeshAppearanceShaderSet * shaderSetFront = NULL;
			DynamicMeshAppearanceShaderSet * shaderSetBack = NULL;
			DynamicMeshAppearanceShaderSet::split(randomization, plane, *shader, *staticVertexBuffer, *staticIndexBuffer, shaderSetFront, shaderSetBack, edgeVector);
			
			if (shaderSetFront != NULL)
				shaderSetVectorFront.push_back(shaderSetFront);
			
			if (shaderSetBack != NULL)
				shaderSetVectorBack.push_back(shaderSetBack);
		}
	}
	
	ptimer.stop();
	
	Vector const splitEdgeCenter = computeEdgeVectorCenter(edgeVector);

	if (!shaderSetVectorFront.empty())
	{
		outputAppearanceFront = new DynamicMeshAppearance(shaderSetVectorFront);
		if (s_showSplitEdge)
			*(outputAppearanceFront->m_splitEdgeVector) = edgeVector;
		outputAppearanceFront->m_splitEdgeCenter = splitEdgeCenter;
	}

	if (!shaderSetVectorBack.empty())
	{
		outputAppearanceBack = new DynamicMeshAppearance(shaderSetVectorBack);
		if (s_showSplitEdge)
			*(outputAppearanceBack->m_splitEdgeVector) = edgeVector;
		outputAppearanceBack->m_splitEdgeCenter = splitEdgeCenter;
	}

	return outputAppearanceFront != NULL || outputAppearanceBack != NULL;
}

//----------------------------------------------------------------------

EdgeVector const * DynamicMeshAppearance::getSplitEdgeVector() const
{
	return m_splitEdgeVector;
}

//----------------------------------------------------------------------

void DynamicMeshAppearance::setAlphaFade(float alphaFade)
{
	m_alphaFade = alphaFade;
}

//----------------------------------------------------------------------

bool DynamicMeshAppearance::collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const
{
	//-- Collide w/ extent only
	if (collideParameters.getQuality() == CollideParameters::Q_low)
	{
		Extent const * const extent = getExtent();
		if (extent)
		{
			Vector normal;
			float time;
			if (extent->intersect(start_o, end_o, &normal, &time))
			{
				result.setPoint(Vector::linearInterpolate(start_o, end_o, time));
				result.setNormal(normal);
				result.setTime(time);

				return true;
			}
		}

		return false;
	}

	//-- All other collide qualities do collision with actual mesh
	if (getExtent() && getExtent()->intersect(start_o, end_o))
	{
		for (LocalShaderPrimitiveVector::const_iterator it = m_localShaderPrimitiveVector.begin(); it != m_localShaderPrimitiveVector.end(); ++it)
		{
			LocalShaderPrimitive const * localShaderPrimitive = *it;

			if (localShaderPrimitive->collide(start_o, end_o, result))
				return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

bool DynamicMeshAppearance::implementsCollide() const
{
	return true;
}

// ----------------------------------------------------------------------

int DynamicMeshAppearance::getHardpointCount() const
{
	return m_hardpointList != NULL ? static_cast<int>(m_hardpointList->size()) : 0;
}

//----------------------------------------------------------------------

int DynamicMeshAppearance::getHardpointIndex(CrcString const &hardpointName, bool optional) const
{
	if (m_hardpointList != NULL)
	{
		int index = 0;
		for (HardpointList::const_iterator it = m_hardpointList->begin(); it != m_hardpointList->end(); ++it, ++index)
		{
			Hardpoint const * const hardpoint = NON_NULL(*it);
			if (hardpoint->getName() == hardpointName)
				return index;
		}
	}

	FATAL(!optional, ("DynamicMeshAppearance::getHardpointIndex failed for hardpoint [%s]", hardpointName.getString()));
	return -1;
}

//----------------------------------------------------------------------

const Hardpoint & DynamicMeshAppearance::getHardpoint(int index) const
{
	NOT_NULL(m_hardpointList);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_hardpointList->size()));
	return *NON_NULL((*m_hardpointList)[static_cast<size_t>(index)]);
}

//----------------------------------------------------------------------

bool DynamicMeshAppearance::findHardpoint(CrcString const &hardpointName, Transform &hardpointTransform) const
{

	//-- The default implementation is to lookup the hardpoint via the appearance template.
	const int hardpointIndex = getHardpointIndex(hardpointName, true);
	if (hardpointIndex < 0)
		return false;
	else
	{
		//-- Handle the appearance scale.
		Transform  scaleTransform(Transform::IF_none);
		scaleTransform.setToScale(getScale());

		//-- Get the hardpoint transform from the hardpoint object.
		const Hardpoint &hardpoint = getHardpoint(hardpointIndex);
		hardpointTransform.multiply(scaleTransform, hardpoint.getTransform());

		return true;
	}
}

//----------------------------------------------------------------------

AxialBox const DynamicMeshAppearance::getTangibleExtent() const
{
	return m_boxExtent.getBox();
}

//----------------------------------------------------------------------

void DynamicMeshAppearance::setDetailLevelBias(int bias)
{
	s_detailLevelBias = bias;
}

//----------------------------------------------------------------------

int DynamicMeshAppearance::getDetailLevelBias()
{
	return s_detailLevelBias;
}

//----------------------------------------------------------------------

Vector const & DynamicMeshAppearance::getSplitEdgeCenter() const
{
	return m_splitEdgeCenter;
}

//----------------------------------------------------------------------

void DynamicMeshAppearance::attachDebrisParticles(Object & debrisObject, Vector const & planeNormal, float thresholdRadius)
{
	if (thresholdRadius < 0.0f)
		return;

	Appearance * const appearance = debrisObject.getAppearance();
	if (NULL == appearance)
		return;
	
	float const radius = debrisObject.getTangibleExtent().getRadius();
	
	if (radius < thresholdRadius)
		return;
	
	DEBUG_FATAL(planeNormal.approximateMagnitude() < Vector::NORMALIZE_THRESHOLD, ("DynamicDebrisObject invalid planeNormal"));
	
	Appearance * const newAppearance = AppearanceTemplateList::createAppearance("appearance/pt_ship_trailing_damage_5.prt");
	if (newAppearance != NULL)
	{
		Object * const newParticleObject = new Object;
		newParticleObject->setAppearance(newAppearance);
		newAppearance->setUniformScale(clamp(1.0f, radius * 0.2f, 50.0f));
		RenderWorld::addObjectNotifications (*newParticleObject);
		debrisObject.addChildObject_o(newParticleObject);
		Transform t;
		t.setLocalFrameKJ_p(Vector::perpendicular(planeNormal), planeNormal);
		newParticleObject->setTransform_o2p(t);
		
		DynamicMeshAppearance * const dynamicMeshAppearance = dynamic_cast<DynamicMeshAppearance *>(appearance);
		if (NULL != dynamicMeshAppearance)
		{
			Vector const & splitEdgeCenter = dynamicMeshAppearance->getSplitEdgeCenter();				
			newParticleObject->move_p(splitEdgeCenter);
		}
	}
}

//======================================================================
