//===================================================================
//
// ViewerLightAppearance.cpp
// asommers
//
// copyright 1998 Bootprint Entertainment
//
//===================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/ViewerLightAppearance.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/DynamicIndexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/Object.h"

#include "dpvsObject.hpp"

//===================================================================

namespace
{
	bool ms_showLights;
}

//===================================================================

bool ViewerLightAppearance::getShowLights (void)
{
	return ms_showLights;
}

//-------------------------------------------------------------------

void ViewerLightAppearance::setShowLights (bool newShowLights)
{
	ms_showLights = newShowLights;
}

//===================================================================

namespace
{
	VertexBufferFormat getVertexBufferFormat ()
	{
		VertexBufferFormat format;
		format.setPosition ();
		format.setColor0 ();

		return format;
	};
}

//===================================================================

class ViewerLightAppearance::LocalShaderPrimitiveParallel : public ShaderPrimitive
{
public:

	LocalShaderPrimitiveParallel (const Appearance& appearance, float length, const VectorArgb& color);
	virtual ~LocalShaderPrimitiveParallel ();

	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader& prepareToView () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;

private:

	LocalShaderPrimitiveParallel ();
	LocalShaderPrimitiveParallel (const LocalShaderPrimitiveParallel&);
	LocalShaderPrimitiveParallel& operator= (const LocalShaderPrimitiveParallel&);

private:

	const Appearance&           m_appearance;
	const float                 m_length;
	const VectorArgb            m_color;
	mutable DynamicVertexBuffer m_vertexBuffer;
	mutable DynamicIndexBuffer  m_indexBuffer;
};

//-------------------------------------------------------------------

ViewerLightAppearance::LocalShaderPrimitiveParallel::LocalShaderPrimitiveParallel (const Appearance& m_appearance, float length, const VectorArgb& color) :
	ShaderPrimitive (),
	m_appearance (m_appearance),
	m_length (length),
	m_color (color),
	m_vertexBuffer (getVertexBufferFormat ()),
	m_indexBuffer ()
{
}

//-------------------------------------------------------------------

ViewerLightAppearance::LocalShaderPrimitiveParallel::~LocalShaderPrimitiveParallel ()
{
}

//-------------------------------------------------------------------

const Vector ViewerLightAppearance::LocalShaderPrimitiveParallel::getPosition_w() const
{
	return Vector::zero;
}
	
//-------------------------------------------------------------------

float ViewerLightAppearance::LocalShaderPrimitiveParallel::getDepthSquaredSortKey() const
{
	return 0.f;
}

//-------------------------------------------------------------------

int ViewerLightAppearance::LocalShaderPrimitiveParallel::getVertexBufferSortKey() const
{
	return m_vertexBuffer.getSortKey ();
}

//-------------------------------------------------------------------

const StaticShader& ViewerLightAppearance::LocalShaderPrimitiveParallel::prepareToView () const
{
	return ShaderTemplateList::get3dVertexColorZStaticShader ().prepareToView ();
}

//-------------------------------------------------------------------

void ViewerLightAppearance::LocalShaderPrimitiveParallel::prepareToDraw () const
{
	//--
	const Vector vList [5] =
	{
		m_length * Vector ( 0.f,  0.f, -1.f),
		m_length * Vector ( 0.f,  0.f,  0.f),
		m_length * Vector ( 0.f,  0.f,  1.f),
		m_length * Vector ( 0.2f, 0.f,  0.75f),
		m_length * Vector (-0.2f, 0.f,  0.75f)
	};

	m_vertexBuffer.lock(5);
		VertexBufferWriteIterator v = m_vertexBuffer.begin();
		for (int i = 0; i < 5; ++i, ++v)
		{
			v.setPosition(vList [i]);
			v.setColor0(m_color);
		}
	m_vertexBuffer.unlock();

	//--
	const Index index[] = 
	{
		0, 2,
		2, 3,
		3, 4,
		4, 2
	};

	m_indexBuffer.lock(8);
		memcpy(m_indexBuffer.begin(), index, sizeof(index));
	m_indexBuffer.unlock();

	//--
	Graphics::setVertexBuffer (m_vertexBuffer);
	Graphics::setIndexBuffer (m_indexBuffer);
}

//-------------------------------------------------------------------

void ViewerLightAppearance::LocalShaderPrimitiveParallel::draw () const
{
	Transform t = m_appearance.getTransform_w ();
	Graphics::setObjectToWorldTransformAndScale(t, Vector::xyz111);
	Graphics::drawIndexedLineList ();

	t.move_l (Vector (0.f, -0.25f * m_length, 0.f));
	Graphics::setObjectToWorldTransformAndScale(t, Vector::xyz111);
	Graphics::drawIndexedLineList ();

	t.move_l (Vector (0.f, 0.5f * m_length, 0.f));
	Graphics::setObjectToWorldTransformAndScale(t, Vector::xyz111);
	Graphics::drawIndexedLineList ();
}

//===================================================================

class ViewerLightAppearance::LocalShaderPrimitivePoint : public ShaderPrimitive
{
public:

	LocalShaderPrimitivePoint (const Appearance& appearance, float length, const VectorArgb& color);
	virtual ~LocalShaderPrimitivePoint ();

	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader& prepareToView () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;

private:

	LocalShaderPrimitivePoint ();
	LocalShaderPrimitivePoint (const LocalShaderPrimitivePoint&);
	LocalShaderPrimitivePoint& operator= (const LocalShaderPrimitivePoint&);

private:

	const Appearance&           m_appearance;
	const float                 m_length;
	const VectorArgb            m_color;
	mutable DynamicVertexBuffer m_vertexBuffer;
	mutable DynamicIndexBuffer  m_indexBuffer;
};

//-------------------------------------------------------------------

ViewerLightAppearance::LocalShaderPrimitivePoint::LocalShaderPrimitivePoint (const Appearance& m_appearance, float length, const VectorArgb& color) :
	ShaderPrimitive (),
	m_appearance (m_appearance),
	m_length (length),
	m_color (color),
	m_vertexBuffer (getVertexBufferFormat ()),
	m_indexBuffer ()
{
}

//-------------------------------------------------------------------

ViewerLightAppearance::LocalShaderPrimitivePoint::~LocalShaderPrimitivePoint ()
{
}

//-------------------------------------------------------------------

const Vector ViewerLightAppearance::LocalShaderPrimitivePoint::getPosition_w() const
{
	return Vector::zero;
}
	
//-------------------------------------------------------------------

float ViewerLightAppearance::LocalShaderPrimitivePoint::getDepthSquaredSortKey() const
{
	return 0.f;
}

//-------------------------------------------------------------------

int ViewerLightAppearance::LocalShaderPrimitivePoint::getVertexBufferSortKey() const
{
	return m_vertexBuffer.getSortKey ();
}

//-------------------------------------------------------------------

const StaticShader& ViewerLightAppearance::LocalShaderPrimitivePoint::prepareToView () const
{
	return ShaderTemplateList::get3dVertexColorZStaticShader ().prepareToView ();
}

//-------------------------------------------------------------------

void ViewerLightAppearance::LocalShaderPrimitivePoint::prepareToDraw () const
{
	//--
	const Vector vList [6] =
	{
		m_length * Vector::unitX,
		m_length * Vector::negativeUnitX,
		m_length * Vector::unitY,
		m_length * Vector::negativeUnitY,
		m_length * Vector::unitZ,
		m_length * Vector::negativeUnitZ
	};

	m_vertexBuffer.lock(6);
		VertexBufferWriteIterator v = m_vertexBuffer.begin();
		for (int i = 0; i < 6; ++i, ++v)
		{
			v.setPosition(vList [i]);
			v.setColor0(m_color);
		}
	m_vertexBuffer.unlock();

	//--
	const Index index[] = 
	{
		0, 1,
		2, 3,
		4, 5
	};

	m_indexBuffer.lock(6);
		memcpy(m_indexBuffer.begin(), index, sizeof(index));
	m_indexBuffer.unlock();

	//--
	Graphics::setVertexBuffer (m_vertexBuffer);
	Graphics::setIndexBuffer (m_indexBuffer);
}

//-------------------------------------------------------------------

void ViewerLightAppearance::LocalShaderPrimitivePoint::draw () const
{
	Transform t = m_appearance.getTransform_w ();
	Graphics::setObjectToWorldTransformAndScale(t, Vector::xyz111);
	Graphics::drawIndexedLineList ();

	t.yaw_l (PI_OVER_4);
	t.roll_l (PI_OVER_4);
	Graphics::setObjectToWorldTransformAndScale(t, Vector::xyz111);
	Graphics::drawIndexedLineList ();
}

//===================================================================

class ViewerLightAppearance::LocalShaderPrimitiveSpot : public ShaderPrimitive
{
public:

	LocalShaderPrimitiveSpot (const Appearance& appearance, float length, float phi, const VectorArgb& color);
	virtual ~LocalShaderPrimitiveSpot ();

	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader& prepareToView () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;

private:

	LocalShaderPrimitiveSpot ();
	LocalShaderPrimitiveSpot (const LocalShaderPrimitiveSpot&);
	LocalShaderPrimitiveSpot& operator= (const LocalShaderPrimitiveSpot&);

private:

	const Appearance&           m_appearance;
	const float                 m_length;
	const float                 m_phi;
	const VectorArgb            m_color;
	mutable DynamicVertexBuffer m_vertexBuffer;
	mutable DynamicIndexBuffer  m_indexBuffer;
};

//-------------------------------------------------------------------

ViewerLightAppearance::LocalShaderPrimitiveSpot::LocalShaderPrimitiveSpot (const Appearance& m_appearance, float length, float phi, const VectorArgb& color) :
	ShaderPrimitive (),
	m_appearance (m_appearance),
	m_length (length),
	m_phi (phi),
	m_color (color),
	m_vertexBuffer (getVertexBufferFormat ()),
	m_indexBuffer ()
{
}

//-------------------------------------------------------------------

ViewerLightAppearance::LocalShaderPrimitiveSpot::~LocalShaderPrimitiveSpot ()
{
}

//-------------------------------------------------------------------

const Vector ViewerLightAppearance::LocalShaderPrimitiveSpot::getPosition_w() const
{
	return Vector::zero;
}
	
//-------------------------------------------------------------------

float ViewerLightAppearance::LocalShaderPrimitiveSpot::getDepthSquaredSortKey() const
{
	return 0.f;
}

//-------------------------------------------------------------------

int ViewerLightAppearance::LocalShaderPrimitiveSpot::getVertexBufferSortKey() const
{
	return m_vertexBuffer.getSortKey ();
}

//-------------------------------------------------------------------

const StaticShader& ViewerLightAppearance::LocalShaderPrimitiveSpot::prepareToView () const
{
	return ShaderTemplateList::get3dVertexColorZStaticShader ().prepareToView ();
}

//-------------------------------------------------------------------

void ViewerLightAppearance::LocalShaderPrimitiveSpot::prepareToDraw () const
{
	//--
	const real x = tan (m_phi * 0.5f) * m_length;
	const real y = x;
	const real z = m_length;

	const Vector vList [5] =
	{
		Vector::zero,
		Vector (-x,  y, z),
		Vector ( x,  y, z),
		Vector ( x, -y, z),
		Vector (-x, -y, z)
	};

	m_vertexBuffer.lock(5);
		VertexBufferWriteIterator v = m_vertexBuffer.begin();
		for (int i = 0; i < 5; ++i, ++v)
		{
			v.setPosition(vList [i]);
			v.setColor0(m_color);
		}
	m_vertexBuffer.unlock();

	//--
	const Index index[] = 
	{
		0, 1,
		0, 2,
		0, 3,
		0, 4,
		1, 2,
		2, 3,
		3, 4,
		4, 1
	};

	m_indexBuffer.lock(16);
		memcpy(m_indexBuffer.begin(), index, sizeof(index));
	m_indexBuffer.unlock();

	//--
	Graphics::setVertexBuffer (m_vertexBuffer);
	Graphics::setIndexBuffer (m_indexBuffer);
	Graphics::setObjectToWorldTransformAndScale(m_appearance.getTransform_w (), Vector::xyz111);
}

//-------------------------------------------------------------------

void ViewerLightAppearance::LocalShaderPrimitiveSpot::draw () const
{
	Graphics::drawLineList ();
}

//===================================================================

ViewerLightAppearance::ViewerLightAppearance (const Light* light, real length) :
	Appearance (0),
	m_localShaderPrimitive (0),
	m_sphere (Vector::zero, length * 2.0f),
	m_dpvsObject(NULL)
{
	switch (light->getType ())
	{
	case Light::T_parallel:
		m_localShaderPrimitive = new ViewerLightAppearance::LocalShaderPrimitiveParallel (*this, length, light->getDiffuseColor ());
		break;

	case Light::T_point:
		m_localShaderPrimitive = new ViewerLightAppearance::LocalShaderPrimitivePoint (*this, length, light->getDiffuseColor ());
		break;

	case Light::T_spot:
		m_localShaderPrimitive = new ViewerLightAppearance::LocalShaderPrimitiveSpot (*this, length, light->getSpotPhi (), light->getDiffuseColor ());
		break;

	case Light::T_ambient:
	default:
		break;
	}

	m_dpvsObject = RenderWorld::createObject(this, m_sphere.getRadius());
}

//-------------------------------------------------------------------

ViewerLightAppearance::~ViewerLightAppearance(void)
{
	delete m_localShaderPrimitive;
	m_localShaderPrimitive = 0;
	m_dpvsObject->release();
}

//-------------------------------------------------------------------

const Sphere& ViewerLightAppearance::getSphere (void) const
{
	return m_sphere;
}

//-------------------------------------------------------------------

DPVS::Object * ViewerLightAppearance::getDpvsObject() const
{
	return m_dpvsObject;
}

//-------------------------------------------------------------------

void ViewerLightAppearance::render () const
{
	if (ms_showLights)
		ShaderPrimitiveSorter::add (*m_localShaderPrimitive);
}

//===================================================================

