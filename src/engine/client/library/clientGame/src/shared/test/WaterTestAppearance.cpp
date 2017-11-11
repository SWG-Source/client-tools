// ======================================================================
//
// WaterTestAppearance.h
// Copyright 2000, 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/WaterTestAppearance.h"

#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/StaticVertexBuffer.h"
#include "sharedCollision/Extent.h"
#include "sharedCollision/ExtentList.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Transform.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Object.h"

#include "dpvsObject.hpp"

#include <algorithm>

// ======================================================================

class WaterTestAppearance::LocalShaderPrimitive: public ShaderPrimitive
{
public:

	explicit LocalShaderPrimitive (WaterTestAppearance &appearance);
	~LocalShaderPrimitive ();

	virtual const StaticShader &prepareToView() const;
	virtual const Vector        getPosition_w() const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual void                prepareToDraw() const;
	virtual void                draw() const;

private:

	WaterTestAppearance &m_appearance;

private:
	// disabled
	LocalShaderPrimitive ();
	LocalShaderPrimitive& operator = (const LocalShaderPrimitive&); //lint -esym(754, LocalShaderPrimitive::operator=) // unreferenced
};

// ======================================================================
// class WaterTestAppearance::LocalShaderPrimitive
// ======================================================================

WaterTestAppearance::LocalShaderPrimitive::LocalShaderPrimitive (WaterTestAppearance &appearance)
:	ShaderPrimitive (),
	m_appearance(appearance)
{
}

// ----------------------------------------------------------------------

WaterTestAppearance::LocalShaderPrimitive::~LocalShaderPrimitive ()
{
}

// ----------------------------------------------------------------------

const StaticShader &WaterTestAppearance::LocalShaderPrimitive::prepareToView () const
{
	return m_appearance.getShader ().prepareToView ();
}

// ----------------------------------------------------------------------

const Vector WaterTestAppearance::LocalShaderPrimitive::getPosition_w() const
{
	return m_appearance.getTransform_w ().getPosition_p ();
}
	
// ----------------------------------------------------------------------

float WaterTestAppearance::LocalShaderPrimitive::getDepthSquaredSortKey () const
{
	return m_appearance.getTransform_w ().getPosition_p ().magnitudeBetweenSquared (ShaderPrimitiveSorter::getCurrentCameraPosition ());
}

// ----------------------------------------------------------------------

int WaterTestAppearance::LocalShaderPrimitive::getVertexBufferSortKey () const
{
	return reinterpret_cast<int> (&m_appearance.getVertexBuffer ());
}

// ----------------------------------------------------------------------

void WaterTestAppearance::LocalShaderPrimitive::prepareToDraw () const
{
	Graphics::setObjectToWorldTransformAndScale (m_appearance.getTransform_w (), m_appearance.getOwner ()->getScale ());
	Graphics::setVertexBuffer (m_appearance.getVertexBuffer ());
}

// ----------------------------------------------------------------------

void WaterTestAppearance::LocalShaderPrimitive::draw () const
{
	Graphics::drawTriangleFan ();
}

// ======================================================================
// class WaterTestAppearance
// ======================================================================

WaterTestAppearance::WaterTestAppearance (real newRadiusX, real newRadiusY, Shader* newShader, float uvScale)
:	Appearance (0),
	m_shader (newShader),
	m_vertexBuffer (0),
	m_radiusX (newRadiusX),
	m_radiusY (newRadiusY),
	m_uvScale (uvScale),
	m_shaderPrimitive (0),
	m_dpvsObject(0)
{
	m_shaderPrimitive = new LocalShaderPrimitive (*this);

	VertexBufferFormat format;
	format.setPosition ();
	format.setNormal ();
	format.setNumberOfTextureCoordinateSets (1);
	format.setTextureCoordinateSetDimension (0, 2);

	m_vertexBuffer = new StaticVertexBuffer (format, 4);
	m_vertexBuffer->lock();

		VertexBufferWriteIterator v = m_vertexBuffer->begin();

		v.setPosition (Vector (-m_radiusX, 0.0f, m_radiusY));
		v.setNormal (Vector::unitY);
		v.setTextureCoordinates (0, 0.0f, 0.0f);
		++v;

		v.setPosition (Vector ( m_radiusX, 0.0f, m_radiusY));
		v.setNormal (Vector::unitY);
		v.setTextureCoordinates (0, 1.0f, 0.0f);
		++v;

		v.setPosition (Vector ( m_radiusX, 0.0f, -m_radiusY));
		v.setNormal (Vector::unitY);
		v.setTextureCoordinates (0, 1.0f, 1.0f);
		++v;

		v.setPosition (Vector (-m_radiusX, 0.0f, -m_radiusY));
		v.setNormal (Vector::unitY);
		v.setTextureCoordinates (0, 0.0f, 1.0f);

	m_vertexBuffer->unlock();

	Extent* e = new Extent ();
	e->setSphere (Sphere (Vector::zero, std::max (m_radiusX, m_radiusY)));
	Appearance::setExtent (ExtentList::fetch (e));

	m_dpvsObject = RenderWorld::createObject(this, e->getSphere ().getRadius ());
} //lint !e429 // (Warning -- Custodial pointer 'e' (line 177) has not been freed or returned) // Appearance will release

// ----------------------------------------------------------------------

WaterTestAppearance::~WaterTestAppearance()
{
	delete m_shaderPrimitive;

	if (m_shader)
	{
		m_shader->release();
		m_shader = 0;
	}

	delete m_vertexBuffer;
	m_dpvsObject->release();
}

// ----------------------------------------------------------------------

const Sphere& WaterTestAppearance::getSphere () const
{
	// @todo short circuit this virtual function call when Appearance
	//       no longer stores its own Extent.  The Extent probably should be returned
	//       and stored in each Appearance-derived class, or it shouldn't be
	//       virtual at all.
	return getExtent()->getSphere ();
}

// ----------------------------------------------------------------------

DPVS::Object* WaterTestAppearance::getDpvsObject() const
{
	return m_dpvsObject;
}

// ----------------------------------------------------------------------

void WaterTestAppearance::render () const
{
	ShaderPrimitiveSorter::add (*m_shaderPrimitive);
}

// ----------------------------------------------------------------------

float WaterTestAppearance::alter (float time)
{
	IGNORE_RETURN(Appearance::alter (time));

	if (m_shader)
		m_shader->alter (time);

	// @todo figure out what this should really return.
	return AlterResult::cms_alterNextFrame;
}

// ======================================================================
