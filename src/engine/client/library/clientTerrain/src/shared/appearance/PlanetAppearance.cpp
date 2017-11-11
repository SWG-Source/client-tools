// ======================================================================
//
//  PlanetAppearance.cpp
//  copyright 2000 Verant Interactive Inc.
//
// ======================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/PlanetAppearance.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderCapability.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/StaticVertexBuffer.h"
#include "clientTerrain/PlanetAppearanceTemplate.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedObject/AlterResult.h"

#include "dpvsObject.hpp"

#include <vector>
#include <algorithm>

// ======================================================================

class PlanetAppearance::LocalShaderPrimitive : public ShaderPrimitive
{
public:

	LocalShaderPrimitive (const Appearance& appearance, const Transform& transform, const Shader& shader, const StaticVertexBuffer& vertexBuffer);
	virtual ~LocalShaderPrimitive ();

	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader& prepareToView () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;

private:

	LocalShaderPrimitive ();
	LocalShaderPrimitive (const LocalShaderPrimitive& rhs);
	LocalShaderPrimitive& operator= (const LocalShaderPrimitive& rhs);

private:

	const Appearance&         m_appearance;
	const Transform&          m_transform;
	const Shader&             m_shader;
	const StaticVertexBuffer& m_vertexBuffer;
};

// ----------------------------------------------------------------------

PlanetAppearance::LocalShaderPrimitive::LocalShaderPrimitive (const Appearance& appearance, const Transform& transform, const Shader& shader, const StaticVertexBuffer& vertexBuffer) :
	ShaderPrimitive (),
	m_appearance (appearance),
	m_transform (transform),
	m_shader (shader),
	m_vertexBuffer (vertexBuffer)
{
}

// ----------------------------------------------------------------------
	
PlanetAppearance::LocalShaderPrimitive::~LocalShaderPrimitive ()
{
}

// ----------------------------------------------------------------------

const Vector PlanetAppearance::LocalShaderPrimitive::getPosition_w() const
{
	return Vector::zero;
}
	
// ----------------------------------------------------------------------

float PlanetAppearance::LocalShaderPrimitive::getDepthSquaredSortKey() const
{
	return m_appearance.getTransform_w ().getPosition_p ().magnitudeBetweenSquared (ShaderPrimitiveSorter::getCurrentCameraPosition ());
}

// ----------------------------------------------------------------------

int PlanetAppearance::LocalShaderPrimitive::getVertexBufferSortKey() const
{
	return m_vertexBuffer.getSortKey ();
}

// ----------------------------------------------------------------------

const StaticShader& PlanetAppearance::LocalShaderPrimitive::prepareToView () const
{
	return m_shader.prepareToView ();
}

// ----------------------------------------------------------------------

void PlanetAppearance::LocalShaderPrimitive::prepareToDraw () const
{
	Transform transform(Transform::IF_none);
	transform.multiply(m_appearance.getTransform_w(), m_transform);

	Graphics::setObjectToWorldTransformAndScale(transform, Vector::xyz111);
	Graphics::setVertexBuffer(m_vertexBuffer);
}

// ----------------------------------------------------------------------

void PlanetAppearance::LocalShaderPrimitive::draw () const
{
	Graphics::drawTriangleStrip ();
}
														  
// ======================================================================

class PlanetAppearance::LocalShaderPrimitiveHalo : public ShaderPrimitive
{
public:

	LocalShaderPrimitiveHalo (const Appearance& appearance, const Shader& shader, const StaticVertexBuffer& vertexBuffer);
	virtual ~LocalShaderPrimitiveHalo ();

	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader& prepareToView () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;

	void setHaloScale(float haloScale);
	void setHaloRoll(float haloRoll);

private:

	LocalShaderPrimitiveHalo ();
	LocalShaderPrimitiveHalo (const LocalShaderPrimitiveHalo& rhs);
	LocalShaderPrimitiveHalo& operator= (const LocalShaderPrimitiveHalo& rhs);

private:

	const Appearance&         m_appearance;
	const Shader&             m_shader;
	const StaticVertexBuffer& m_vertexBuffer;
	float m_haloScale;
	float m_haloRoll;
};

// ----------------------------------------------------------------------

PlanetAppearance::LocalShaderPrimitiveHalo::LocalShaderPrimitiveHalo (Appearance const & appearance, Shader const & shader, StaticVertexBuffer const & vertexBuffer) :
	ShaderPrimitive (),
	m_appearance (appearance),
	m_shader (shader),
	m_vertexBuffer (vertexBuffer),
	m_haloRoll(0.f),
	m_haloScale(1.f)
{
}

// ----------------------------------------------------------------------
	
PlanetAppearance::LocalShaderPrimitiveHalo::~LocalShaderPrimitiveHalo ()
{
}

// ----------------------------------------------------------------------

const Vector PlanetAppearance::LocalShaderPrimitiveHalo::getPosition_w() const
{
	return Vector::zero;
}
	
// ----------------------------------------------------------------------

float PlanetAppearance::LocalShaderPrimitiveHalo::getDepthSquaredSortKey() const
{
	return m_appearance.getTransform_w ().getPosition_p ().magnitudeBetweenSquared (ShaderPrimitiveSorter::getCurrentCameraPosition ());
}

// ----------------------------------------------------------------------

int PlanetAppearance::LocalShaderPrimitiveHalo::getVertexBufferSortKey() const
{
	return m_vertexBuffer.getSortKey ();
}

// ----------------------------------------------------------------------

const StaticShader& PlanetAppearance::LocalShaderPrimitiveHalo::prepareToView () const
{
	return m_shader.prepareToView ();
}

// ----------------------------------------------------------------------

void PlanetAppearance::LocalShaderPrimitiveHalo::prepareToDraw () const
{
	Object const * const owner = m_appearance.getOwner();
	Vector const objectPosition_w = (owner) ? owner->getPosition_w() : m_appearance.getTransform_w().getPosition_p();
	Camera const & camera = ShaderPrimitiveSorter::getCurrentCamera();
	Vector const & cameraPosition_w = camera.getPosition_w();

	Transform transform;
	transform.roll_l(m_haloRoll);
	Vector const direction_w = objectPosition_w - cameraPosition_w;
	Vector const direction_o = transform.rotate_p2l(direction_w);
	transform.setPosition_p(objectPosition_w);
	transform.yaw_l(direction_o.theta());
	transform.pitch_l(direction_o.phi());

	//-- when viewing a sphere from a finite distance, 
	//-- we observe a circular 'horizon' on the sphere, 
	//-- which is smaller than the actual circuference of the sphere

	//-- we want to place our halo on this horizon
	//-- the horizon is specified by these parameters:
	//--
	//-- horizonRadius: the radius of the horizon circle
	//-- horizonEdgeDistance:  the distance from the camera to the horizon edge
	//-- horizonCenterDistance: the distance from the camera to the center of the horizon circle

	float const sphereDistance = objectPosition_w.magnitudeBetween(camera.getPosition_w());
	float const sphereRadius = owner->getAppearanceSphereRadius();
	float horizonRadius = sphereRadius;

	if (sphereDistance > 0.01f)
	{
		float const horizonEdgeDistance = sqrt(sqr(sphereDistance) - sqr(sphereRadius));
		horizonRadius = horizonEdgeDistance * (sphereRadius / sphereDistance);
		float const horizonCenterDistance = sqr(horizonEdgeDistance) / sphereDistance;

		transform.move_l(Vector::unitZ * (horizonCenterDistance - sphereDistance));
	}

	Graphics::setObjectToWorldTransformAndScale(transform, Vector::xyz111 * (horizonRadius * m_haloScale));
	Graphics::setVertexBuffer(m_vertexBuffer);
}

// ----------------------------------------------------------------------

void PlanetAppearance::LocalShaderPrimitiveHalo::draw () const
{
	Graphics::drawTriangleFan ();
}

// ----------------------------------------------------------------------

void PlanetAppearance::LocalShaderPrimitiveHalo::setHaloScale(float const haloScale)
{
	m_haloScale = haloScale;
}

// ----------------------------------------------------------------------

void PlanetAppearance::LocalShaderPrimitiveHalo::setHaloRoll(float const haloRoll)
{
	m_haloRoll = haloRoll;
}

// ======================================================================

PlanetAppearance::PlanetAppearance(PlanetAppearanceTemplate const * const planetAppearanceTemplate) :
	Appearance(planetAppearanceTemplate),
	m_planetAppearanceTemplate(NON_NULL(planetAppearanceTemplate)),
	m_surfaceShader(planetAppearanceTemplate->fetchSurfaceShader()),
	m_surfaceTransform(),
	m_surfaceVertexBufferList(new VertexBufferList),
	m_surfaceLocalShaderPrimitiveList(new LocalShaderPrimitiveList),
	m_cloudShader(planetAppearanceTemplate->fetchCloudShader()),
	m_cloudTransform(),
	m_cloudVertexBufferList(0),
	m_cloudLocalShaderPrimitiveList(0),
	m_haloShader(planetAppearanceTemplate->fetchHaloShader()),
	m_haloVertexBuffer(0),
	m_localShaderPrimitiveHalo(0),
	m_sphere(planetAppearanceTemplate->getSphere()),
	m_dpvsObject(0)
{
	{
		createVertexBuffers(planetAppearanceTemplate->getSurfaceRadius(), planetAppearanceTemplate->getSurfaceUScaleMAIN(), planetAppearanceTemplate->getSurfaceVScaleMAIN(), planetAppearanceTemplate->getSurfaceUScaleDETA(), planetAppearanceTemplate->getSurfaceVScaleDETA(), m_surfaceVertexBufferList);
		
		if (m_cloudShader)
		{
			m_cloudVertexBufferList = new VertexBufferList;
			createVertexBuffers(planetAppearanceTemplate->getCloudRadius(), planetAppearanceTemplate->getCloudUScaleMAIN(), planetAppearanceTemplate->getCloudVScaleMAIN(), 0.f, 0.f, m_cloudVertexBufferList);
			m_cloudLocalShaderPrimitiveList = new LocalShaderPrimitiveList;
		}

		size_t const numberOfLatitudeLines = static_cast<size_t>(planetAppearanceTemplate->getNumberOfLatitudeLines());
		for (size_t i = 0; i < numberOfLatitudeLines; ++i)
		{
			m_surfaceLocalShaderPrimitiveList->push_back(new LocalShaderPrimitive(*this, m_surfaceTransform, *m_surfaceShader, *(*m_surfaceVertexBufferList)[i]));

			if (m_cloudShader)
			{
				NOT_NULL(m_cloudLocalShaderPrimitiveList);
				NOT_NULL(m_cloudVertexBufferList);
				m_cloudLocalShaderPrimitiveList->push_back(new LocalShaderPrimitive (*this, m_cloudTransform, *m_cloudShader, *(*m_cloudVertexBufferList)[i]));
			}
		}
	}

	if (m_haloShader) 
	{
		VertexBufferFormat format;
		format.setPosition();
		format.setNormal();
		format.setNumberOfTextureCoordinateSets(1);
		format.setTextureCoordinateSetDimension(0, 2);
		m_haloVertexBuffer = new StaticVertexBuffer(format, 4);

		m_haloVertexBuffer->lock();
		{
			//-- all halo scaling is now handled at runtime
			float const haloRadius = 1.0f;
	
			VertexBufferReadWriteIterator v = static_cast<VertexBufferReadWriteIterator>(m_haloVertexBuffer->begin());

			v.setPosition(haloRadius, -haloRadius, 0.f);
			v.setNormal(0.f, 0.f, -1.f);
			v.setTextureCoordinates(0, 0.f, 1.f);
			++v;

			v.setPosition(-haloRadius, -haloRadius, 0.f);
			v.setNormal(0.f, 0.f, -1.f);
			v.setTextureCoordinates(0, 1.f, 1.f);
			++v;

			v.setPosition(-haloRadius, haloRadius, 0.f);
			v.setNormal(0.f, 0.f, -1.f);
			v.setTextureCoordinates(0, 1.f, 0.f);
			++v;

			v.setPosition(haloRadius, haloRadius, 0.f);
			v.setNormal(0.f, 0.f, -1.f);
			v.setTextureCoordinates(0, 0.f, 0.f);
		}
		m_haloVertexBuffer->unlock ();

		m_localShaderPrimitiveHalo = new LocalShaderPrimitiveHalo(*this, *m_haloShader, *m_haloVertexBuffer);
		m_localShaderPrimitiveHalo->setHaloScale(planetAppearanceTemplate->getHaloScale());

	}

	m_dpvsObject = RenderWorld::createObject(this, m_sphere.getRadius());
}

// ----------------------------------------------------------------------
	
PlanetAppearance::~PlanetAppearance () 
{
	{
		m_surfaceShader->release();

		std::for_each (m_surfaceVertexBufferList->begin (), m_surfaceVertexBufferList->end (), PointerDeleter ());
		m_surfaceVertexBufferList->clear ();
		delete m_surfaceVertexBufferList;

		std::for_each (m_surfaceLocalShaderPrimitiveList->begin (), m_surfaceLocalShaderPrimitiveList->end (), PointerDeleter ());
		m_surfaceLocalShaderPrimitiveList->clear ();
		delete m_surfaceLocalShaderPrimitiveList;
	}

	if (m_cloudShader)
	{
		m_cloudShader->release();

		NOT_NULL(m_cloudVertexBufferList);
		std::for_each (m_cloudVertexBufferList->begin (), m_cloudVertexBufferList->end (), PointerDeleter ());
		m_cloudVertexBufferList->clear ();
		delete m_cloudVertexBufferList;
		m_cloudVertexBufferList = 0;

		NOT_NULL(m_cloudLocalShaderPrimitiveList);
		std::for_each (m_cloudLocalShaderPrimitiveList->begin (), m_cloudLocalShaderPrimitiveList->end (), PointerDeleter ());
		m_cloudLocalShaderPrimitiveList->clear ();
		delete m_cloudLocalShaderPrimitiveList;
		m_cloudLocalShaderPrimitiveList = 0;
	}

	if (m_haloShader)
	{
		m_haloShader->release();

		delete m_haloVertexBuffer;
		m_haloVertexBuffer = 0;

		delete m_localShaderPrimitiveHalo;
		m_localShaderPrimitiveHalo = 0;
	}

	IGNORE_RETURN(m_dpvsObject->release());
}

// ----------------------------------------------------------------------

const Sphere& PlanetAppearance::getSphere (void) const
{
	return m_sphere;
}

// ----------------------------------------------------------------------

void PlanetAppearance::createVertexBuffers (float const radius, float const uScaleMAIN, float const vScaleMAIN, float const uScaleDETA, float const vScaleDETA, VertexBufferList* vertexBufferList) const
{
	// Each ring is a triangle strip organized as below:
	//
	//		1   3   5
	//		*---*---*
	//		|\  |\  |
	//		| \ |  \|
	//		*---*---*
	//		0   2   4
	//
	// Only need to do half as many calculations and make use of upper and lower hemisphere symmetry.

	int numberOfLatitideLines = m_planetAppearanceTemplate->getNumberOfLatitudeLines();
	int numberOfLongitudeLines = m_planetAppearanceTemplate->getNumberOfLatitudeLines();

	int i;
	for (i = 0; i < numberOfLatitideLines; ++i)
		vertexBufferList->push_back(0);

	const float vertDeltaRad = PI         / static_cast<float> (numberOfLatitideLines);
	const float horzDeltaRad = PI_TIMES_2 / static_cast<float> (numberOfLongitudeLines);

	bool const hasSecondUvSet = uScaleDETA > 0.f && vScaleDETA > 0.f;

	int r;
	for (r = 0; r < numberOfLatitideLines / 2; ++r)
	{
		const float vertAngleTopRad    = r * vertDeltaRad;
		const float vertAngleBottomRad = (r + 1) * vertDeltaRad;

		const float yTopU    = cos (vertAngleTopRad);
		const float yBottomU = cos (vertAngleBottomRad);

		const float vTop     = vertAngleTopRad / PI;
		const float vBottom  = vertAngleBottomRad / PI;

		size_t const topIndex = static_cast<size_t>(r);
		size_t const bottomIndex = static_cast<size_t>((numberOfLatitideLines - 1) - r);

		VertexBufferFormat format;
		format.setPosition ();
		format.setNormal ();
		format.setNumberOfTextureCoordinateSets (hasSecondUvSet ? 2 : 1);
		format.setTextureCoordinateSetDimension (0, 2);

		if (hasSecondUvSet)
			format.setTextureCoordinateSetDimension (1, 2);

		(*vertexBufferList)[topIndex] = new StaticVertexBuffer(format, (numberOfLatitideLines + 1) * 2);
		(*vertexBufferList)[bottomIndex] = new StaticVertexBuffer(format, (numberOfLatitideLines + 1) * 2);

		(*vertexBufferList)[topIndex]->lock();
		(*vertexBufferList)[bottomIndex]->lock();

			VertexBufferWriteIterator rv0 = (*vertexBufferList)[topIndex]->begin();
			VertexBufferWriteIterator rv1 = (*vertexBufferList)[bottomIndex]->begin();

			int j;
			for (j = 0; j <= numberOfLongitudeLines; ++j)
			{
				float const horzAngleRad = j * horzDeltaRad;
				float const xTopU = sin(vertAngleTopRad) * sin(horzAngleRad);
				float const zTopU = sin(vertAngleTopRad) * cos(horzAngleRad);
				float const xBottomU = sin(vertAngleBottomRad) * sin(horzAngleRad);
				float const zBottomU = sin(vertAngleBottomRad) * cos(horzAngleRad);
				float const u = 1.f - horzAngleRad / PI_TIMES_2;

				rv0.setPosition (xTopU * radius, yTopU * radius, zTopU * radius);
				rv0.setNormal (xTopU, yTopU, zTopU);
				rv0.setTextureCoordinates (0, u * uScaleMAIN, vTop * vScaleMAIN);

				if (hasSecondUvSet)
					rv0.setTextureCoordinates (1, u * uScaleDETA, vTop * vScaleDETA);

				++rv0;

				rv0.setPosition (xBottomU * radius, yBottomU * radius, zBottomU * radius);			
				rv0.setNormal (xBottomU, yBottomU, zBottomU);
				rv0.setTextureCoordinates (0, u * uScaleMAIN, vBottom * vScaleMAIN);

				if (hasSecondUvSet)
					rv0.setTextureCoordinates (1, u * uScaleDETA, vBottom * vScaleDETA);

				++rv0;
				
				rv1.setPosition (-xBottomU * radius, -yBottomU * radius, -zBottomU * radius);			
				rv1.setNormal (-xBottomU, -yBottomU, -zBottomU);
				rv1.setTextureCoordinates (0, (u - 0.5f) * uScaleMAIN, (1.f - vBottom) * vScaleMAIN);

				if (hasSecondUvSet)
					rv1.setTextureCoordinates (1, (u - 0.5f) * uScaleDETA, (1.f - vBottom) * vScaleDETA);

				++rv1;

				rv1.setPosition (-xTopU * radius, -yTopU * radius, -zTopU * radius);
				rv1.setNormal (-xTopU, -yTopU, -zTopU);
				rv1.setTextureCoordinates (0, (u - 0.5f) * uScaleMAIN, (1.f - vTop) * vScaleMAIN);

				if (hasSecondUvSet)
					rv1.setTextureCoordinates (1, (u - 0.5f) * uScaleDETA, (1.f - vTop) * vScaleDETA);

				++rv1;
			}
		
		(*vertexBufferList)[topIndex]->unlock();
		(*vertexBufferList)[bottomIndex]->unlock();
	}
}

// ----------------------------------------------------------------------

DPVS::Object *PlanetAppearance::getDpvsObject () const
{
	return m_dpvsObject;
}

// ----------------------------------------------------------------------

void PlanetAppearance::render () const
{
	{
		for (LocalShaderPrimitiveList::const_iterator iterator = m_surfaceLocalShaderPrimitiveList->begin (); iterator != m_surfaceLocalShaderPrimitiveList->end (); ++iterator)
			ShaderPrimitiveSorter::add (*(*iterator), 0);
	}

	if (m_cloudShader)
	{
		NOT_NULL(m_cloudLocalShaderPrimitiveList);
		for (LocalShaderPrimitiveList::const_iterator iterator = m_cloudLocalShaderPrimitiveList->begin (); iterator != m_cloudLocalShaderPrimitiveList->end (); ++iterator)
			ShaderPrimitiveSorter::add (*(*iterator), 0);
	}

	if (m_haloShader)
	{
		NOT_NULL(m_localShaderPrimitiveHalo);
		ShaderPrimitiveSorter::add (*m_localShaderPrimitiveHalo, 0);
	}
}

// ----------------------------------------------------------------------

float PlanetAppearance::alter (float const elapsedTime)
{
	m_surfaceTransform.yaw_l(m_planetAppearanceTemplate->getSurfaceRotationRate() * elapsedTime);
	m_cloudTransform.yaw_l(m_planetAppearanceTemplate->getCloudRotationRate() * elapsedTime);

	return AlterResult::cms_alterNextFrame;
}

// ----------------------------------------------------------------------

void PlanetAppearance::setHaloScale(float const haloScale)
{
	if (m_localShaderPrimitiveHalo)
		m_localShaderPrimitiveHalo->setHaloScale(haloScale);
	else
		DEBUG_WARNING(true, ("PlanetAppearance::setHaloScale: called for PlanetAppearance %s with no halo", getAppearanceTemplateName()));
}

// ----------------------------------------------------------------------

void PlanetAppearance::setHaloRoll(float const haloRoll)
{
	if (m_localShaderPrimitiveHalo)
		m_localShaderPrimitiveHalo->setHaloRoll(haloRoll);
	else
		DEBUG_WARNING(true, ("PlanetAppearance::setHaloRoll: called for PlanetAppearance %s with no halo", getAppearanceTemplateName()));
}

// ======================================================================

