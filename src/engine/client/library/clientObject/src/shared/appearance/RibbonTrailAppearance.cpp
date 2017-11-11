// ======================================================================
// 
// RibbonTrailAppearance.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/RibbonTrailAppearance.h"

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
#include "sharedFoundation/PointerDeleter.h"
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

#include <algorithm>
#include <vector>

// ======================================================================
// RibbonTrailAppearanceNamespace
// ======================================================================

namespace RibbonTrailAppearanceNamespace
{
	const float ms_radius = 4.f;
}

using namespace RibbonTrailAppearanceNamespace;

// ======================================================================
// RibbonTrailAppearance::LocalShaderPrimitive
// ======================================================================

class RibbonTrailAppearance::LocalShaderPrimitive : public ShaderPrimitive
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void               install ();
	static VertexBufferFormat getVertexBufferFormat ();

public:

	LocalShaderPrimitive (const RibbonTrailAppearance& owner, const Object* followObject, float width, const VectorArgb& color);
	virtual ~LocalShaderPrimitive ();

	virtual float                alter (float time);
	virtual const Vector         getPosition_w () const;
	virtual float                getDepthSquaredSortKey () const;
	virtual int                  getVertexBufferSortKey () const;
	virtual const StaticShader&  prepareToView () const;
	virtual void                 prepareToDraw () const;
	virtual void                 draw () const;
	virtual const Sphere&        getSphere () const;

	bool                         shouldRender () const;
	bool                         isAlive () const;
	void                         kill ();

private:

	typedef std::vector<std::pair <Vector, float> > PointList;

private:

	static void       remove ();
	static PointList* createPointList ();
	static void       destroyPointList (PointList* pointList);

private:

	typedef std::vector<PointList*> PointListList;
	static PointListList ms_pointListList;

private:

	const RibbonTrailAppearance& m_owner;
	ConstWatcher<Object>         m_followObject;
	const float                  m_width;
	const VectorArgb             m_color;
	mutable Sphere               m_sphere;
	mutable DynamicVertexBuffer  m_vertexBuffer;
	const Vector                 m_startPosition_w;
	Timer                        m_killTimer;
	bool                         m_kill;
	bool                         m_dead;
	PointList* const             m_pointList;
	
private:

	LocalShaderPrimitive ();
	LocalShaderPrimitive (const LocalShaderPrimitive&);             //lint !e754  //-- not referenced
	LocalShaderPrimitive& operator= (const LocalShaderPrimitive&);  //lint !e754  //-- not referenced
};

// ======================================================================
// STATIC PUBLIC RibbonTrailAppearance::LocalShaderPrimitive
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL (RibbonTrailAppearance::LocalShaderPrimitive, true, 0, 0, 0);

// ----------------------------------------------------------------------

void RibbonTrailAppearance::LocalShaderPrimitive::install ()
{
	installMemoryBlockManager ();
	ExitChain::add (remove, "RibbonTrailAppearance::LocalShaderPrimitive::remove");
}

// ----------------------------------------------------------------------

VertexBufferFormat RibbonTrailAppearance::LocalShaderPrimitive::getVertexBufferFormat ()
{
	VertexBufferFormat format;
	format.setPosition ();
	format.setColor0 ();

	return format;
}

// ======================================================================
// PUBLIC RibbonTrailAppearance::LocalShaderPrimitive
// ======================================================================

RibbonTrailAppearance::LocalShaderPrimitive::LocalShaderPrimitive (const RibbonTrailAppearance& owner, const Object* const followObject, const float width, const VectorArgb& color) : 
	ShaderPrimitive (),
	m_owner (owner),
	m_followObject (followObject),
	m_width (width),
	m_color (color),
	m_sphere (Vector::zero, ms_radius),
	m_vertexBuffer (getVertexBufferFormat ()),
	m_startPosition_w (followObject->getPosition_w ()),
	m_killTimer (1.f),
	m_kill (false),
	m_dead (false),
	m_pointList (createPointList ())
{
}

// ----------------------------------------------------------------------

RibbonTrailAppearance::LocalShaderPrimitive::~LocalShaderPrimitive ()
{
	destroyPointList (m_pointList);
}

// ----------------------------------------------------------------------

float RibbonTrailAppearance::LocalShaderPrimitive::alter (const float elapsedTime)
{
	//-- if we need to die, update the kill timer
	if (m_kill)
	{
		m_dead = m_killTimer.updateZero (elapsedTime);
		if (m_dead)
			return AlterResult::cms_kill;
		else
			return AlterResult::cms_alterNextFrame;
	}
	else
	{
		//-- if our follow object is still alive
		if (m_followObject)
		{
			//-- walk the list and kill existing particles
			uint n = m_pointList->size ();
			uint i = 0;
			while (i < n)
			{
				std::pair<Vector, float>& point = (*m_pointList) [i];

				point.second -= elapsedTime;
				if (point.second < 0.f)
				{
					m_pointList->erase (m_pointList->begin () + i);
					--n;
				}
				else
					++i;
			}

			//-- see if we need to add a new one
			const Vector& position_w = m_followObject->getPosition_w ();
			const bool shouldAdd = m_pointList->empty () || m_pointList->back ().first.magnitudeBetweenSquared (position_w) > sqr (0.1f);

			if (shouldAdd)
				m_pointList->push_back (std::make_pair (m_followObject->getPosition_w (), 1.f));

			return AlterResult::cms_alterNextFrame;
		}
		else
		{
			//-- our follow object is dead, so we should die as well
			kill ();
			return AlterResult::cms_kill;
		}
	}
}

// ----------------------------------------------------------------------

const Vector RibbonTrailAppearance::LocalShaderPrimitive::getPosition_w () const
{
	return m_owner.getTransform_w ().getPosition_p ();
}
	
// ----------------------------------------------------------------------

float RibbonTrailAppearance::LocalShaderPrimitive::getDepthSquaredSortKey () const
{
	return m_owner.getTransform_w ().getPosition_p ().magnitudeBetweenSquared (ShaderPrimitiveSorter::getCurrentCameraPosition ());
}

// ----------------------------------------------------------------------

int RibbonTrailAppearance::LocalShaderPrimitive::getVertexBufferSortKey () const
{
	return m_vertexBuffer.getSortKey ();
}

// ----------------------------------------------------------------------

const StaticShader&RibbonTrailAppearance::LocalShaderPrimitive::prepareToView () const
{
	return ShaderTemplateList::get3dVertexColorAZStaticShader ().prepareToView ();
}

// ----------------------------------------------------------------------

void RibbonTrailAppearance::LocalShaderPrimitive::prepareToDraw () const
{
	//-- setup vertexarray for drawing individual polygons
	m_vertexBuffer.lock (m_pointList->size () * 2);

		VertexBufferWriteIterator v = m_vertexBuffer.begin ();

		uint i;
		for (i = 0; i < m_pointList->size (); ++i)
		{
			const Vector& position_w = (*m_pointList) [i].first;
			const Vector  toCamera_w = position_w - ShaderPrimitiveSorter::getCurrentCamera ().getPosition_w ();

			Vector direction_w;
			if (i == m_pointList->size () - 1)
				direction_w = position_w - (*m_pointList) [i - 1].first;
			else
				direction_w = (*m_pointList) [i + 1].first - position_w;

			Vector width_w = direction_w.cross (toCamera_w);
			IGNORE_RETURN (width_w.normalize ());
			width_w *= m_width;

			const VectorArgb color (linearInterpolate (0.f, 0.5f * (m_kill ? m_killTimer.getRemainingRatio () : 1.f), static_cast<float> (i) / (m_pointList->size () - 1)), m_color.r, m_color.g, m_color.b);

			v.setPosition (m_owner.getTransform_w ().rotateTranslate_p2l (position_w + width_w));
			v.setColor0 (color);
			++v;

			v.setPosition (m_owner.getTransform_w ().rotateTranslate_p2l (position_w - width_w));
			v.setColor0 (color);
			++v;
		}

	m_vertexBuffer.unlock ();

	Graphics::setObjectToWorldTransformAndScale (m_owner.getTransform_w (), Vector::xyz111);
	Graphics::setVertexBuffer (m_vertexBuffer);
}

// ----------------------------------------------------------------------

void RibbonTrailAppearance::LocalShaderPrimitive::draw () const
{
	Graphics::drawTriangleStrip ();
}

// ----------------------------------------------------------------------

const Sphere& RibbonTrailAppearance::LocalShaderPrimitive::getSphere () const
{
	return m_sphere;
}

// ----------------------------------------------------------------------

bool RibbonTrailAppearance::LocalShaderPrimitive::shouldRender () const
{
	return m_pointList->size () > 1;
}

// ----------------------------------------------------------------------

bool RibbonTrailAppearance::LocalShaderPrimitive::isAlive () const
{
	return !m_dead;
}

// ----------------------------------------------------------------------

void RibbonTrailAppearance::LocalShaderPrimitive::kill ()
{
	m_kill = true;
}

// ======================================================================
// STATIC PRIVATE RibbonTrailAppearance::LocalShaderPrimitive
// ======================================================================

std::vector<RibbonTrailAppearance::LocalShaderPrimitive::PointList*> RibbonTrailAppearance::LocalShaderPrimitive::ms_pointListList;

// ----------------------------------------------------------------------

void RibbonTrailAppearance::LocalShaderPrimitive::remove ()
{
	std::for_each (ms_pointListList.begin (), ms_pointListList.end (), PointerDeleter ());
	ms_pointListList.clear ();

	removeMemoryBlockManager ();
}

// ----------------------------------------------------------------------

std::vector<std::pair<Vector, float> >* RibbonTrailAppearance::LocalShaderPrimitive::createPointList ()
{
	if (!ms_pointListList.empty ())
	{
		PointList* const pointList = ms_pointListList.back ();
		ms_pointListList.pop_back ();
		pointList->clear ();

		return pointList;
	}

	return new PointList;
}

// ----------------------------------------------------------------------

void RibbonTrailAppearance::LocalShaderPrimitive::destroyPointList (PointList* const pointList)
{
	ms_pointListList.push_back (pointList);
}

// ======================================================================
// STATIC PUBLIC RibbonTrailAppearance
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL (RibbonTrailAppearance, true, 0, 0, 0);

// ----------------------------------------------------------------------

void RibbonTrailAppearance::install ()
{
	installMemoryBlockManager ();
	RibbonTrailAppearance::LocalShaderPrimitive::install ();

	ExitChain::add (RibbonTrailAppearance::remove, "RibbonTrailAppearanceNamespace::remove");
}

// ----------------------------------------------------------------------

void RibbonTrailAppearance::remove ()
{
	removeMemoryBlockManager ();
}

// ======================================================================
// PUBLIC RibbonTrailAppearance
// ======================================================================

RibbonTrailAppearance::RibbonTrailAppearance (const Object* const followObject, const float width, const VectorArgb& color) : 
	Appearance (0),
	m_localShaderPrimitive (NULL),
	m_dpvsObject (NULL)
{
	m_localShaderPrimitive = new LocalShaderPrimitive (*this, followObject, width, color);
	updateDpvsTestModel ();
	useRenderEffectsFlag(true);
}

// ----------------------------------------------------------------------

RibbonTrailAppearance::~RibbonTrailAppearance ()
{
	delete m_localShaderPrimitive;

	NOT_NULL (m_dpvsObject);
	IGNORE_RETURN (m_dpvsObject->release ());
}

// ----------------------------------------------------------------------

const Sphere&RibbonTrailAppearance::getSphere () const
{
	return m_localShaderPrimitive->getSphere ();
}

// ----------------------------------------------------------------------

float RibbonTrailAppearance::alter (float elapsedTime)
{
	NOT_NULL (m_localShaderPrimitive);
	return m_localShaderPrimitive->alter (elapsedTime);
}

// ----------------------------------------------------------------------

void RibbonTrailAppearance::render () const
{
	NOT_NULL (m_localShaderPrimitive);

	if (m_localShaderPrimitive->shouldRender ())
		ShaderPrimitiveSorter::add (*m_localShaderPrimitive);
}

// ----------------------------------------------------------------------

bool RibbonTrailAppearance::isAlive () const
{
	NOT_NULL (m_localShaderPrimitive);
	return m_localShaderPrimitive->isAlive ();
}

// ----------------------------------------------------------------------

void RibbonTrailAppearance::kill ()
{
	NOT_NULL (m_localShaderPrimitive);
	m_localShaderPrimitive->kill ();
}

// ======================================================================
// PRIVATE RibbonTrailAppearance
// ======================================================================

DPVS::Object* RibbonTrailAppearance::getDpvsObject () const
{
	return m_dpvsObject;
}

// ----------------------------------------------------------------------

void RibbonTrailAppearance::updateDpvsTestModel ()
{
	NOT_NULL (m_localShaderPrimitive);

	AxialBox box;
	box.setMin (Vector (-ms_radius, -ms_radius, -ms_radius));
	box.setMax (Vector (ms_radius, ms_radius, ms_radius));

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
