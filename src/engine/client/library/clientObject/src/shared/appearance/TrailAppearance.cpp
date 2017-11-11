// ======================================================================
// 
// TrailAppearance.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/TrailAppearance.h"

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
#include "sharedObject/AlterResult.h"
#include "sharedObject/Object.h"

#include "dpvsModel.hpp"
#include "dpvsObject.hpp"

// ======================================================================
// TrailAppearance::LocalShaderPrimitive
// ======================================================================

class TrailAppearance::LocalShaderPrimitive : public ShaderPrimitive
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	static VertexBufferFormat getVertexBufferFormat ();

public:

	LocalShaderPrimitive (const TrailAppearance& owner, const Object* followObject, float width, const VectorArgb& color);
	virtual ~LocalShaderPrimitive ();

	virtual float               alter (float time);
	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey () const;
	virtual int                 getVertexBufferSortKey () const;
	virtual const StaticShader& prepareToView () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;
	virtual const Sphere&       getSphere () const;

	bool                        isAlive () const;
	float                       getLength () const;
	float                       getWidth () const;

private:

	const TrailAppearance&      m_owner;
	ConstWatcher<Object>        m_followObject;
	const float                 m_width;
	const VectorArgb            m_color;
	mutable Sphere              m_sphere;
	mutable DynamicVertexBuffer m_vertexBuffer;
	const Vector                m_startPosition_w;
	Vector                      m_endPosition_w;
	Timer                       m_killTimer;
	bool                        m_dead;

private:

	LocalShaderPrimitive ();
	LocalShaderPrimitive (const LocalShaderPrimitive&);             //lint !e754  //-- not referenced
	LocalShaderPrimitive& operator= (const LocalShaderPrimitive&);  //lint !e754  //-- not referenced
};

// ======================================================================
// STATIC PUBLIC TrailAppearance::LocalShaderPrimitive
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL (TrailAppearance::LocalShaderPrimitive, true, 0, 0, 0);

// ----------------------------------------------------------------------

VertexBufferFormat TrailAppearance::LocalShaderPrimitive::getVertexBufferFormat ()
{
	VertexBufferFormat format;
	format.setPosition ();
	format.setColor0 ();

	return format;
}

// ======================================================================
// PUBLIC TrailAppearance::LocalShaderPrimitive
// ======================================================================

TrailAppearance::LocalShaderPrimitive::LocalShaderPrimitive (const TrailAppearance& owner, const Object* const followObject, const float width, const VectorArgb& color) : 
	ShaderPrimitive (),
	m_owner (owner),
	m_followObject (followObject),
	m_width (width),
	m_color (color),
	m_sphere (Sphere::unit),
	m_vertexBuffer (getVertexBufferFormat ()),
	m_startPosition_w (followObject->getPosition_w ()),
	m_endPosition_w (m_startPosition_w),
	m_killTimer (1.f),
	m_dead (false)
{
}

// ----------------------------------------------------------------------

TrailAppearance::LocalShaderPrimitive::~LocalShaderPrimitive ()
{
}

// ----------------------------------------------------------------------

float TrailAppearance::LocalShaderPrimitive::alter (const float elapsedTime)
{
	if (m_followObject)
	{
		m_endPosition_w = m_followObject->getPosition_w ();
		return AlterResult::cms_alterNextFrame;
	}
	else
	{
		m_dead = m_killTimer.updateZero (elapsedTime);
		return m_dead ? AlterResult::cms_kill : AlterResult::cms_alterNextFrame;
	}
}

// ----------------------------------------------------------------------

const Vector TrailAppearance::LocalShaderPrimitive::getPosition_w () const
{
	return m_owner.getTransform_w ().getPosition_p ();
}
	
// ----------------------------------------------------------------------

float TrailAppearance::LocalShaderPrimitive::getDepthSquaredSortKey () const
{
	return m_owner.getTransform_w ().getPosition_p ().magnitudeBetweenSquared (ShaderPrimitiveSorter::getCurrentCameraPosition ());
}

// ----------------------------------------------------------------------

int TrailAppearance::LocalShaderPrimitive::getVertexBufferSortKey () const
{
	return m_vertexBuffer.getSortKey ();
}

// ----------------------------------------------------------------------

const StaticShader&TrailAppearance::LocalShaderPrimitive::prepareToView () const
{
	return ShaderTemplateList::get3dVertexColorAStaticShader ().prepareToView ();
}

// ----------------------------------------------------------------------

void TrailAppearance::LocalShaderPrimitive::prepareToDraw () const
{
	const Vector toCamera_o = m_owner.getTransform_w ().rotateTranslate_p2l (ShaderPrimitiveSorter::getCurrentCamera ().getPosition_w ());

	Vector width = Vector::unitZ.cross (toCamera_o);
	IGNORE_RETURN (width.normalize ());
	width *= m_width;

	const float distance = m_endPosition_w.magnitudeBetween (m_startPosition_w);

	//-- setup vertexarray for drawing individual polygons
	m_vertexBuffer.lock (4);

		VertexBufferWriteIterator v = m_vertexBuffer.begin ();

		const VectorArgb colorNear (0.0f, m_color.r, m_color.g, m_color.b);
		const VectorArgb colorFar  (0.33f * (m_followObject ? 1.f : m_killTimer.getRemainingRatio ()), m_color.r, m_color.g, m_color.b);

		v.setPosition (Vector::unitZ * distance + width);
		v.setColor0 (colorFar);
		++v;

		v.setPosition (Vector::unitZ * distance - width);
		v.setColor0 (colorFar);
		++v;

		v.setPosition (-width);
		v.setColor0 (colorNear);
		++v;

		v.setPosition (width);
		v.setColor0 (colorNear);

	m_vertexBuffer.unlock ();

	Graphics::setObjectToWorldTransformAndScale (m_owner.getTransform_w (), Vector::xyz111);
	Graphics::setVertexBuffer (m_vertexBuffer);
}

// ----------------------------------------------------------------------

void TrailAppearance::LocalShaderPrimitive::draw () const
{
	Graphics::drawTriangleFan ();
}

// ----------------------------------------------------------------------

const Sphere& TrailAppearance::LocalShaderPrimitive::getSphere () const
{
	const float radius = (m_endPosition_w - m_startPosition_w).approximateMagnitude ();
	m_sphere.setCenter (Vector::unitZ * radius);
	m_sphere.setRadius (radius);

	return m_sphere;
}

// ----------------------------------------------------------------------

bool TrailAppearance::LocalShaderPrimitive::isAlive () const
{
	return !m_dead;
}

// ----------------------------------------------------------------------

float TrailAppearance::LocalShaderPrimitive::getLength () const
{
	return m_startPosition_w.magnitudeBetween (m_endPosition_w);
}

// ----------------------------------------------------------------------

float TrailAppearance::LocalShaderPrimitive::getWidth () const
{
	return m_width;
}

// ======================================================================
// STATIC PUBLIC TrailAppearance
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL (TrailAppearance, true, 0, 0, 0);

// ----------------------------------------------------------------------

void TrailAppearance::install ()
{
	installMemoryBlockManager ();
	TrailAppearance::LocalShaderPrimitive::install ();

	ExitChain::add (TrailAppearance::remove, "TrailAppearanceNamespace::remove");
}

// ----------------------------------------------------------------------

void TrailAppearance::remove ()
{
	removeMemoryBlockManager ();
}

// ======================================================================
// PUBLIC TrailAppearance
// ======================================================================

TrailAppearance::TrailAppearance (const Object* const followObject, const float width, const VectorArgb& color) : 
	Appearance (0),
	m_localShaderPrimitive (NULL),
	m_dpvsObject (NULL)
{
	m_localShaderPrimitive = new LocalShaderPrimitive (*this, followObject, width, color);
	updateDpvsTestModel ();
	useRenderEffectsFlag(true);
}

// ----------------------------------------------------------------------

TrailAppearance::~TrailAppearance ()
{
	delete m_localShaderPrimitive;

	NOT_NULL (m_dpvsObject);
	IGNORE_RETURN (m_dpvsObject->release ());
}

// ----------------------------------------------------------------------

const Sphere&TrailAppearance::getSphere () const
{
	return m_localShaderPrimitive->getSphere ();
}

// ----------------------------------------------------------------------

float TrailAppearance::alter (float elapsedTime)
{
	NOT_NULL (m_localShaderPrimitive);
	return m_localShaderPrimitive->alter (elapsedTime);
}

// ----------------------------------------------------------------------

void TrailAppearance::render () const
{
	NOT_NULL (m_localShaderPrimitive);
	ShaderPrimitiveSorter::add (*m_localShaderPrimitive);
}

// ----------------------------------------------------------------------

bool TrailAppearance::isAlive () const
{
	NOT_NULL (m_localShaderPrimitive);
	return m_localShaderPrimitive->isAlive ();
}

// ======================================================================
// PRIVATE TrailAppearance
// ======================================================================

DPVS::Object* TrailAppearance::getDpvsObject () const
{
	return m_dpvsObject;
}

// ----------------------------------------------------------------------

void TrailAppearance::updateDpvsTestModel ()
{
	NOT_NULL (m_localShaderPrimitive);
	const float length = 128.f;
	const float width  = m_localShaderPrimitive->getWidth ();

	AxialBox box;
	box.setMin (Vector (-width, -width, 0.0f));
	box.setMax (Vector ( width,  width, length));

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

