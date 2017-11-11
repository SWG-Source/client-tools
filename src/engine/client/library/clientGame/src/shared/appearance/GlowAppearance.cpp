//======================================================================
//
// GlowAppearance.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/GlowAppearance.h"

#include "clientGame/ClientWorld.h"
#include "clientGame/Game.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplate.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/Extent.h"
#include "sharedCollision/ExtentList.h"
#include "sharedFoundation/Clock.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Transform.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Object.h"
#include "sharedRandom/Random.h"

#include "dpvsModel.hpp"
#include "dpvsObject.hpp"

// ======================================================================

class GlowAppearance::LocalShaderPrimitive : public ShaderPrimitive
{
public:

	static VertexBufferFormat getVertexBufferFormat();

public:

	LocalShaderPrimitive(const GlowAppearance &owner, float scaleMin, float scaleMaxFront, float scaleMaxBack, Shader const * const shaderFront, Shader const * const shaderBack, VectorArgb const & color);
	virtual ~LocalShaderPrimitive();

	virtual float alter(float time);
	virtual const Vector getPosition_w() const;
	virtual float getDepthSquaredSortKey() const;
	virtual int getVertexBufferSortKey() const;
	virtual const StaticShader &prepareToView() const;
	virtual void prepareToDraw() const;
	virtual void draw() const;

	const Sphere &getSphere() const;

	float getScale() const;
	void setScale(float scale) const;

private:

	const GlowAppearance &m_owner;
	Shader const * const m_shaderFront;
	Shader const * const m_shaderBack;
	mutable Shader const * m_shaderCurrent;
	mutable DynamicVertexBuffer m_vertexBuffer;
	mutable float m_scale;
	float m_scaleMin;
	float m_scaleMaxFront;
	float m_scaleMaxBack;
	Sphere m_sphere;
	PackedArgb m_color;

private:

	LocalShaderPrimitive();
	LocalShaderPrimitive(const LocalShaderPrimitive &);             //lint -esym(754, LocalShaderPrimitive::LocalShaderPrimitive) // not referenced // defensive hiding
	LocalShaderPrimitive &operator =(const LocalShaderPrimitive &);

	static bool m_renderingEnabled;
};

//===================================================================

VertexBufferFormat GlowAppearance::LocalShaderPrimitive::getVertexBufferFormat()
{
	VertexBufferFormat format;
	format.setPosition();
	format.setColor0();
	format.setNumberOfTextureCoordinateSets(1);
	format.setTextureCoordinateSetDimension(0, 2);
	return format;
}

//===================================================================

GlowAppearance::LocalShaderPrimitive::LocalShaderPrimitive(const GlowAppearance &owner, float scaleMin, float scaleMaxFront, float scaleMaxBack, Shader const * const shaderFront, Shader const * const shaderBack, VectorArgb const & color) :
ShaderPrimitive(),
m_owner(owner),
m_shaderFront(shaderFront),
m_shaderBack(shaderBack),
m_shaderCurrent(shaderFront),
m_vertexBuffer(getVertexBufferFormat()),
m_scale(1.0f),
m_scaleMin(scaleMin),
m_scaleMaxFront(scaleMaxFront),
m_scaleMaxBack(scaleMaxBack),
m_sphere(),
m_color(color)
{
	DEBUG_FATAL(shaderFront == NULL && shaderBack == NULL, ("GlowAppearance::LocalShaderPrimitive constructed with no valid shaders."));

	if (m_shaderFront != NULL)
		m_shaderFront->fetch();

	if (m_shaderBack != NULL)
		m_shaderBack->fetch();

	m_sphere.setCenter(0.0f, 0.0f, 0.0f);
	m_sphere.setRadius(std::max(m_scaleMaxFront, m_scaleMaxBack));
	setScale(m_scaleMaxBack);
}

// ----------------------------------------------------------------------

GlowAppearance::LocalShaderPrimitive::~LocalShaderPrimitive()
{
	if (m_shaderFront != NULL)
		m_shaderFront->release();
	if (m_shaderBack != NULL)
		m_shaderBack->release();

	m_shaderCurrent = NULL;
}

// ----------------------------------------------------------------------

float GlowAppearance::LocalShaderPrimitive::alter(float time)
{
	float alterResult = NON_NULL(m_shaderCurrent)->alter(time);
	AlterResult::incorporateAlterResult(alterResult, AlterResult::cms_alterNextFrame);
	return alterResult;
}

// ----------------------------------------------------------------------

const Vector GlowAppearance::LocalShaderPrimitive::getPosition_w() const
{
	return m_owner.getTransform_w().getPosition_p();
}

//-------------------------------------------------------------------

float GlowAppearance::LocalShaderPrimitive::getDepthSquaredSortKey() const
{
	return m_owner.getTransform_w().getPosition_p().magnitudeBetweenSquared(ShaderPrimitiveSorter::getCurrentCameraPosition());
}

// ----------------------------------------------------------------------

int GlowAppearance::LocalShaderPrimitive::getVertexBufferSortKey() const
{
	return m_vertexBuffer.getSortKey();
}

// ----------------------------------------------------------------------

const StaticShader &GlowAppearance::LocalShaderPrimitive::prepareToView() const
{
	return NON_NULL(m_shaderCurrent)->prepareToView();
}

// ----------------------------------------------------------------------

void GlowAppearance::LocalShaderPrimitive::prepareToDraw() const
{
	Camera const & camera = ShaderPrimitiveSorter::getCurrentCamera();

	const Object *owner = m_owner.getOwner();
	const Transform &o2w = (owner) ? owner->getTransform_o2w() : m_owner.getTransform_w();
	Vector cameraPosition_o = o2w.rotateTranslate_p2l(camera.getPosition_w());

	cameraPosition_o.normalize();

	//-- the z axis is the 'up' axis for glow
	if (cameraPosition_o.z > 0.f)
	{
		//-- don't allow the current shader to go null
		if (m_shaderBack == NULL)
		{
			m_shaderCurrent = NON_NULL(m_shaderFront);
			setScale(0.0f);
		}
		else
		{
			m_shaderCurrent = m_shaderBack;
			setScale (linearInterpolate(m_scaleMin, m_scaleMaxBack, cameraPosition_o.z) * m_owner.getGlowBlockScale());
		}
	}
	else
	{
		//-- don't allow the current shader to go null
		if (m_shaderFront == NULL)
		{
			m_shaderCurrent = NON_NULL(m_shaderBack);
			setScale(0.0f);
		}
		else
		{
			m_shaderCurrent = m_shaderFront;
			setScale (linearInterpolate(m_scaleMin, m_scaleMaxFront, -cameraPosition_o.z) * m_owner.getGlowBlockScale());
		}
	}

	//-- setup vertexarray for drawing individual polygons
	m_vertexBuffer.lock(4);

		VertexBufferWriteIterator v = m_vertexBuffer.begin();

		float const halfSize = m_scale * 0.5f;

		v.setPosition(halfSize, halfSize, 0.0f);
		v.setTextureCoordinates(0, 0.0f, 0.0f);
		v.setColor0(m_color);
		++v;

		v.setPosition(halfSize, -halfSize, 0.0f);
		v.setTextureCoordinates(0, 1.0f , 0.0f);
		v.setColor0(m_color);
		++v;

		v.setPosition(-halfSize, -halfSize, 0.0f);
		v.setTextureCoordinates(0, 1.0f, 1.0f);
		v.setColor0(m_color);
		++v;

		v.setPosition(-halfSize, halfSize, 0.0f);
		v.setTextureCoordinates(0, 0.0f, 1.0f);
		v.setColor0(m_color);

	m_vertexBuffer.unlock();

	Transform const & transform_camera_o2w = camera.getTransform_o2w();
	Transform renderTransform(transform_camera_o2w);
	renderTransform.setPosition_p(o2w.getPosition_p());
	Graphics::setObjectToWorldTransformAndScale(renderTransform, Vector::xyz111 * m_scale);
	Graphics::setVertexBuffer(m_vertexBuffer);
}

// ----------------------------------------------------------------------

void GlowAppearance::LocalShaderPrimitive::draw() const
{
	Graphics::drawTriangleFan();
}

// ----------------------------------------------------------------------

inline float GlowAppearance::LocalShaderPrimitive::getScale() const
{
	return m_scale;
}

//----------------------------------------------------------------------

inline void GlowAppearance::LocalShaderPrimitive::setScale(float scale) const
{
	m_scale = scale;
}

// ----------------------------------------------------------------------

inline const Sphere &GlowAppearance::LocalShaderPrimitive::getSphere() const
{
	return m_sphere;
}

// ======================================================================

GlowAppearance::GlowAppearance(ShaderTemplate const * const shaderTemplateFront, ShaderTemplate const * const shaderTemplateBack, VectorArgb const & color, float scaleMin, float scaleMaxFront, float scaleMaxBack) :
Appearance(NULL),
m_localShaderPrimitive(NULL),
m_dpvsObject(NULL),
m_glowBlocked(false),
m_glowBlockScale(0.0f),
m_glowTimer(Random::randomReal(0.25f, 0.5f))
{
	Shader const * const shaderFront = (shaderTemplateFront != NULL) ? shaderTemplateFront->fetchShader() : NULL;
	Shader const * const shaderBack = (shaderTemplateBack != NULL) ? shaderTemplateBack->fetchShader() : NULL;
	DEBUG_FATAL(shaderFront == NULL && shaderBack == NULL, ("GlowAppearance constructed with no valid shaders."));

	m_localShaderPrimitive = new LocalShaderPrimitive(*this, scaleMin, scaleMaxFront, scaleMaxBack, shaderFront, shaderBack, color);

	if (shaderFront)
		shaderFront->release();

	if (shaderBack)
		shaderBack->release();

	updateDpvsTestModel();
	NOT_NULL(m_dpvsObject);

	useRenderEffectsFlag(true);
}

// ----------------------------------------------------------------------

GlowAppearance::~GlowAppearance()
{
	delete m_localShaderPrimitive;
	m_localShaderPrimitive = NULL;
	IGNORE_RETURN(m_dpvsObject->release());

}

// ----------------------------------------------------------------------

const Sphere &GlowAppearance::getSphere() const
{
	return m_localShaderPrimitive->getSphere();
}

// ----------------------------------------------------------------------

float GlowAppearance::alter(float time)
{
	float alterResult = m_localShaderPrimitive->alter(time);
	AlterResult::incorporateAlterResult(alterResult, AlterResult::cms_alterNextFrame);
	return alterResult;
}

// ----------------------------------------------------------------------

void GlowAppearance::render() const
{
	updateGlowVisibility(Clock::frameTime());

	if (!m_glowBlocked || m_glowBlockScale > 0.0f)
		ShaderPrimitiveSorter::add(*m_localShaderPrimitive);
}

//----------------------------------------------------------------------

void GlowAppearance::updateGlowVisibility(float const elapsedTimeSecs) const
{
	Camera const & camera = ShaderPrimitiveSorter::getCurrentCamera();

	//-- Only do the collision test if the hardpoint is facing the opposite direction of the camera
	if (m_glowTimer.updateNoReset(elapsedTimeSecs))
	{
		if (getRenderedLastFrame())
		{
			m_glowBlocked = false;
			Vector const cameraPosition_w = camera.getPosition_w();

			const Object *owner = getOwner();
			const Transform &o2w = (owner) ? owner->getTransform_o2w() : getTransform_w();
			const Vector cameraPosition_o = o2w.rotateTranslate_p2l(cameraPosition_w);
			if (cameraPosition_o.z > 0.f)
			{
				CollideParameters collideParameters;
				collideParameters.setQuality(CollideParameters::Q_medium);

				CollisionInfo cinfo;
				uint16 const flags = ClientWorld::CF_tangible | ClientWorld::CF_tangibleNotTargetable | ClientWorld::CF_skeletal | ClientWorld::CF_childObjects;
				CellProperty const * const cameraCell = camera.getParentCell();
				m_glowBlocked = ClientWorld::collide(cameraCell, cameraPosition_w, o2w.getPosition_p(), collideParameters, cinfo, flags, 0);

				m_glowTimer.setExpireTime(Random::randomReal(0.25f, 0.5f));
			}
		}
		else
			m_glowBlocked = true;
	}

	if (m_glowBlocked)
		m_glowBlockScale = std::max(0.0f, m_glowBlockScale - (elapsedTimeSecs * 4.0f));
	else
		m_glowBlockScale = std::min(1.0f, m_glowBlockScale + (elapsedTimeSecs * 8.0f));
}

//----------------------------------------------------------------------

void GlowAppearance::updateDpvsTestModel()
{
	if (m_localShaderPrimitive == NULL)
		return;

	Extent * const e = new Extent();
	e->setSphere(m_localShaderPrimitive->getSphere());
	setExtent(ExtentList::fetch(e));

	{
		DPVS::Model * const sphereModel = RenderWorld::fetchSphereModel(getSphere());
		m_dpvsObject = RenderWorld::createObject(this, sphereModel);
		IGNORE_RETURN(sphereModel->release());
	}
} //lint !e429 //e ptr

// ----------------------------------------------------------------------

DPVS::Object * GlowAppearance::getDpvsObject() const
{
	return m_dpvsObject;
} //lint !e1763 //const func

//----------------------------------------------------------------------

float GlowAppearance::getGlowBlockScale() const
{
	return m_glowBlockScale;
}

//----------------------------------------------------------------------

void GlowAppearance::reset()
{
	m_glowBlockScale = 0.0f;
}

// ======================================================================
