// ======================================================================
//
// RibbonAppearance.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/RibbonAppearance.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Timer.h"
#include "sharedFoundation/Watcher.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Transform.h"
#include "sharedMath/VectorArgb.h"
#include "sharedMath/Sphere.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/Object.h"

#include "dpvsModel.hpp"
#include "dpvsObject.hpp"

#include <vector>

// ======================================================================
// RibbonAppearance::LocalShaderPrimitive
// ======================================================================

class RibbonAppearance::LocalShaderPrimitive : public ShaderPrimitive
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	typedef RibbonAppearance::PointList PointList;

public:

	static VertexBufferFormat getVertexBufferFormat ();

public:

	LocalShaderPrimitive (const RibbonAppearance& owner, const PointList& pointList, float width, const VectorArgb& color, bool closed);
	virtual ~LocalShaderPrimitive ();

	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey () const;
	virtual int                 getVertexBufferSortKey () const;
	virtual const StaticShader& prepareToView () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;
	virtual const Sphere&       getSphere () const;

	const PointList&            getPointList () const;
	void                        setPointList (const PointList& pointList);
	bool                        shouldRender () const;

private:

	const RibbonAppearance&     m_owner;
	PointList                   m_pointList;
	const float                 m_width;
	const VectorArgb            m_color;
	bool const                  m_closed;
	mutable Sphere              m_sphere;
	mutable DynamicVertexBuffer m_vertexBuffer;

private:

	LocalShaderPrimitive ();
	LocalShaderPrimitive (const LocalShaderPrimitive&);             //lint !e754  //-- not referenced
	LocalShaderPrimitive& operator= (const LocalShaderPrimitive&);  //lint !e754  //-- not referenced
};

// ======================================================================
// STATIC PUBLIC RibbonAppearance::LocalShaderPrimitive
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL (RibbonAppearance::LocalShaderPrimitive, true, 0, 0, 0);

// ----------------------------------------------------------------------

VertexBufferFormat RibbonAppearance::LocalShaderPrimitive::getVertexBufferFormat ()
{
	VertexBufferFormat format;
	format.setPosition ();
	format.setColor0 ();

	return format;
}

// ======================================================================
// PUBLIC RibbonAppearance::LocalShaderPrimitive
// ======================================================================

RibbonAppearance::LocalShaderPrimitive::LocalShaderPrimitive (const RibbonAppearance& owner, const RibbonAppearance::PointList& pointList, const float width, const VectorArgb& color, bool closed) :
	ShaderPrimitive (),
	m_owner (owner),
	m_pointList (pointList),
	m_width (width),
	m_color (color),
	m_closed (closed),
	m_sphere (Sphere::unit),
	m_vertexBuffer (getVertexBufferFormat ())
{
}

// ----------------------------------------------------------------------

RibbonAppearance::LocalShaderPrimitive::~LocalShaderPrimitive ()
{
}

// ----------------------------------------------------------------------

const Vector RibbonAppearance::LocalShaderPrimitive::getPosition_w () const
{
	return m_owner.getTransform_w ().getPosition_p ();
}

// ----------------------------------------------------------------------

float RibbonAppearance::LocalShaderPrimitive::getDepthSquaredSortKey () const
{
	return m_owner.getTransform_w ().getPosition_p ().magnitudeBetweenSquared (ShaderPrimitiveSorter::getCurrentCameraPosition ());
}

// ----------------------------------------------------------------------

int RibbonAppearance::LocalShaderPrimitive::getVertexBufferSortKey () const
{
	return m_vertexBuffer.getSortKey ();
}

// ----------------------------------------------------------------------

const StaticShader&RibbonAppearance::LocalShaderPrimitive::prepareToView () const
{
	return ShaderTemplateList::get3dVertexColorAZStaticShader ().prepareToView ();
}

// ----------------------------------------------------------------------

void RibbonAppearance::LocalShaderPrimitive::prepareToDraw () const
{
	//-- setup vertexarray for drawing individual polygons
	m_vertexBuffer.lock ((m_pointList.size () + (m_closed ? 1 : 0)) * 2);

		VertexBufferWriteIterator v = m_vertexBuffer.begin ();

		Vector vList [2];

		const Transform& transform_a2w = m_owner.getTransform_w ();

		const uint n = m_pointList.size ();
		uint i;
		for (i = 0; i < n; ++i)
		{
			const Vector& position_w  = transform_a2w.rotateTranslate_l2p (m_pointList [i]);

			const Vector v0 (transform_a2w.rotateTranslate_p2l (position_w + Vector::unitY * m_width));
			v.setPosition (v0);
			v.setColor0 (m_color);
			++v;

			const Vector v1 (transform_a2w.rotateTranslate_p2l (position_w - Vector::unitY * m_width));
			v.setPosition (v1);
			v.setColor0 (m_color);
			++v;

			if (i == 0)
			{
				vList [0] = v0;
				vList [1] = v1;
			}
		}

		if (m_closed)
		{
			v.setPosition (vList [0]);
			v.setColor0 (m_color);
			++v;

			v.setPosition (vList [1]);
			v.setColor0 (m_color);
			++v;
		}

	m_vertexBuffer.unlock ();

	Graphics::setObjectToWorldTransformAndScale (m_owner.getTransform_w (), Vector::xyz111);
	Graphics::setVertexBuffer (m_vertexBuffer);
}

// ----------------------------------------------------------------------

void RibbonAppearance::LocalShaderPrimitive::draw () const
{
	const GlCullMode cullMode = Graphics::getCullMode ();
	Graphics::setCullMode (GCM_none);
	Graphics::drawTriangleStrip ();
	Graphics::setCullMode (cullMode);
}

// ----------------------------------------------------------------------

const Sphere& RibbonAppearance::LocalShaderPrimitive::getSphere () const
{
	return m_sphere;
}

// ----------------------------------------------------------------------

const RibbonAppearance::PointList& RibbonAppearance::LocalShaderPrimitive::getPointList () const
{
	return m_pointList;
}

// ----------------------------------------------------------------------

void RibbonAppearance::LocalShaderPrimitive::setPointList (const PointList& pointList)
{
	m_pointList = pointList;

	if (m_pointList.size () > 0)
	{
		//-- calculate the sphere
		Vector center;
		uint i;
		for (i = 0; i < m_pointList.size (); ++i)
			center += m_pointList [i];

		center /= static_cast<float> (i);

		float radius = 0.f;
		for (i = 0; i < m_pointList.size (); ++i)
			radius = std::max (center.magnitudeBetweenSquared (m_pointList [i]), radius);

		radius = sqrt (radius);

		m_sphere.setCenter (center);
		m_sphere.setRadius (radius);
	}
	else
		m_sphere = Sphere::unit;
}

// ----------------------------------------------------------------------

bool RibbonAppearance::LocalShaderPrimitive::shouldRender () const
{
	return !m_pointList.empty ();
}

// ======================================================================
// STATIC PUBLIC RibbonAppearance
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL (RibbonAppearance, true, 0, 0, 0);

// ----------------------------------------------------------------------

void RibbonAppearance::install ()
{
	installMemoryBlockManager ();
	RibbonAppearance::LocalShaderPrimitive::install ();

	ExitChain::add (RibbonAppearance::remove, "RibbonAppearanceNamespace::remove");
}

// ----------------------------------------------------------------------

void RibbonAppearance::remove ()
{
	removeMemoryBlockManager ();
}

// ======================================================================
// PUBLIC RibbonAppearance
// ======================================================================

RibbonAppearance::RibbonAppearance (const RibbonAppearance::PointList& pointList, const float width, const VectorArgb& color, bool const closed) :
	Appearance (0),
	m_localShaderPrimitive (NULL),
	m_dpvsObject (NULL)
{
	m_localShaderPrimitive = new LocalShaderPrimitive (*this, pointList, width, color, closed);
	updateDpvsTestModel ();
}

// ----------------------------------------------------------------------

RibbonAppearance::~RibbonAppearance ()
{
	delete m_localShaderPrimitive;

	NOT_NULL (m_dpvsObject);
	IGNORE_RETURN (m_dpvsObject->release ());
}

// ----------------------------------------------------------------------

const Sphere&RibbonAppearance::getSphere () const
{
	return m_localShaderPrimitive->getSphere ();
}

// ----------------------------------------------------------------------

void RibbonAppearance::render () const
{
	NOT_NULL (m_localShaderPrimitive);

	if (m_localShaderPrimitive->shouldRender ())
		ShaderPrimitiveSorter::add (*m_localShaderPrimitive);
}

// ----------------------------------------------------------------------

const RibbonAppearance::PointList& RibbonAppearance::getPointList () const
{
	NOT_NULL (m_localShaderPrimitive);
	return m_localShaderPrimitive->getPointList ();
}

// ----------------------------------------------------------------------

void RibbonAppearance::setPointList (const PointList& pointList)
{
	NOT_NULL (m_localShaderPrimitive);
	m_localShaderPrimitive->setPointList (pointList);
	updateDpvsTestModel ();
}

// ======================================================================
// PRIVATE RibbonAppearance
// ======================================================================

DPVS::Object* RibbonAppearance::getDpvsObject () const
{
	return m_dpvsObject;
}

// ----------------------------------------------------------------------

void RibbonAppearance::updateDpvsTestModel ()
{
	NOT_NULL (m_localShaderPrimitive);
	const Sphere& sphere = m_localShaderPrimitive->getSphere ();
	const Vector& center = sphere.getCenter ();
	const float   radius = sphere.getRadius ();
	const Vector  corner (radius, radius, radius);

	AxialBox box;
	box.setMin (center - corner);
	box.setMax (center + corner);

	if (m_dpvsObject)
	{
		DPVS::OBBModel *const model = RenderWorld::fetchBoxModel (box);
		m_dpvsObject->setTestModel (model);
		IGNORE_RETURN (model->release ());
	}
	else
		m_dpvsObject = RenderWorld::createObject (this, box);
}

// ======================================================================

