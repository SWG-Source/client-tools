//
// SpriteAppearance.cpp
//
// Portions Copyright 1998 Bootprint Entertainment
// Portions Copyright 2000-2001, Sony Online Entertainment Inc.
// All Rights Reserved.
//

//-------------------------------------------------------------------

#include "clientObject/FirstClientObject.h"
#include "clientObject/SpriteAppearance.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientObject/SpriteAppearanceTemplate.h"
#include "sharedCollision/Extent.h"
#include "sharedCollision/ExtentList.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Timer.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedMath/AxialBox.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Object.h"

#include "dpvsModel.hpp"
#include "dpvsObject.hpp"

#include <cstdlib>
#include <algorithm>

//-------------------------------------------------------------------

MemoryBlockManager *SpriteAppearance::ms_memoryBlockManager;

//-------------------------------------------------------------------

// @todo MBM this class

class SpriteAppearance::LocalShaderPrimitive : public ShaderPrimitive
{
public:

	static VertexBufferFormat getVertexBufferFormat();

public:

	LocalShaderPrimitive(const SpriteAppearance &owner, const Vector &newCenter, real newRadius, Shader *newShader, const VectorArgb &newColor);
	virtual ~LocalShaderPrimitive();

	virtual float               alter(float time);
	virtual const Vector        getPosition_w() const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader &prepareToView() const;
	virtual void                prepareToDraw() const;
	virtual void                draw() const;
	virtual float               getRadius() const;

	const Sphere               &getSphere (void) const;

	const Shader               *getShader() const;
	Shader                     *getShader();

	void                        setPulse(bool newPulse);

	VectorArgb                 &getColor();
	const VectorArgb           &getColor() const;
	void                        setColor(const VectorArgb &newColor);

	void                        setAlpha(real newAlpha);
	void                        setRadius(real newRadius);

private:

	const SpriteAppearance        &m_owner;
	mutable DynamicVertexBuffer    m_vertexBuffer;

	Sphere                         m_sphere;
	float                          m_width;
	float                          m_height;
	Shader                        *m_shader;
	VectorArgb                     m_color;

	Timer                          m_timer;
	bool                           m_forward;
	bool                           m_pulse;

private:

	LocalShaderPrimitive(const LocalShaderPrimitive &);
	LocalShaderPrimitive &operator =(const LocalShaderPrimitive &);
};

// ======================================================================

VertexBufferFormat SpriteAppearance::LocalShaderPrimitive::getVertexBufferFormat()
{
	VertexBufferFormat format;
	format.setPosition();
	format.setNormal();
	format.setColor0();
	format.setNumberOfTextureCoordinateSets(1);
	format.setTextureCoordinateSetDimension(0, 2);
	return format;
}

// ======================================================================

SpriteAppearance::LocalShaderPrimitive::LocalShaderPrimitive(const SpriteAppearance &owner, const Vector &newCenter, real newRadius, Shader *newShader, const VectorArgb &newColor)
: ShaderPrimitive(),
	m_owner(owner),
	m_vertexBuffer(getVertexBufferFormat()),
	m_sphere(newCenter, newRadius),
	m_width(newRadius),
	m_height(newRadius),
	m_shader(newShader),
	m_color(newColor),
	m_timer(),
	m_forward(true),
	m_pulse(false)
{
	const real time = static_cast<real>(rand() % 100) / 100.0f;
	m_timer.setExpireTime(time + 0.25f);
	m_timer.reset();

	//-- hack start animation at different frames
	m_shader->alter(static_cast<float>(rand() % 100) / 100.0f);
}

// ----------------------------------------------------------------------

SpriteAppearance::LocalShaderPrimitive::~LocalShaderPrimitive()
{
	m_shader->release();
}

// ----------------------------------------------------------------------

float SpriteAppearance::LocalShaderPrimitive::alter(float elapsedTime)
{
	if (m_pulse)
	{
		if (m_timer.updateZero(elapsedTime))
			m_forward = !m_forward;

		if (m_forward)
			m_color.a = m_timer.getElapsedRatio();
		else
			m_color.a = m_timer.getRemainingRatio();

		m_color.a = clamp(0.0f, m_color.a, 1.0f);

		return AlterResult::cms_alterNextFrame;
	}
	else
		return AlterResult::cms_keepNoAlter;
}

// ----------------------------------------------------------------------

const Vector SpriteAppearance::LocalShaderPrimitive::getPosition_w() const
{
	return m_owner.getTransform_w().getPosition_p();
}
	
// ----------------------------------------------------------------------

float SpriteAppearance::LocalShaderPrimitive::getDepthSquaredSortKey() const
{
	return m_owner.getTransform_w().getPosition_p().magnitudeBetweenSquared(ShaderPrimitiveSorter::getCurrentCameraPosition());
}

// ----------------------------------------------------------------------

int SpriteAppearance::LocalShaderPrimitive::getVertexBufferSortKey() const
{
	return m_vertexBuffer.getSortKey();
}

// ----------------------------------------------------------------------

const StaticShader &SpriteAppearance::LocalShaderPrimitive::prepareToView() const
{
	// @todo prepareForView prepareToView should have the same name
	return m_shader->prepareToView();
}

// ----------------------------------------------------------------------

void SpriteAppearance::LocalShaderPrimitive::prepareToDraw() const
{
	const float scale = m_owner.getScale().x;

	// get position of object from objectToWorld
	Vector pos = ShaderPrimitiveSorter::getCurrentCamera().rotateTranslate_w2o(m_owner.getTransform_w().rotateTranslate_l2p(m_sphere.getCenter() * scale));

	// add offsets to make a camera space sprite (always facing the camera)
	const float width_2  = m_width  * 0.5f * scale;
	const float height_2 = m_height * 0.5f * scale;

	m_vertexBuffer.lock(4);

		VertexBufferWriteIterator v = m_vertexBuffer.begin();

		v.setPosition(Vector(pos.x - width_2, pos.y + height_2, pos.z));
		v.setNormal(Vector::negativeUnitZ);
		v.setColor0(m_color);
		v.setTextureCoordinates(0, 0.0f, 0.0f);
		++v;

		v.setPosition(Vector(pos.x + width_2, pos.y + height_2, pos.z));
		v.setNormal(Vector::negativeUnitZ);
		v.setColor0(m_color);
		v.setTextureCoordinates(0, 1.0f , 0.0f);
		++v;

		v.setPosition(Vector(pos.x + width_2, pos.y - height_2, pos.z));
		v.setNormal(Vector::negativeUnitZ);
		v.setColor0(m_color);
		v.setTextureCoordinates(0, 1.0f, 1.0f);
		++v;

		v.setPosition(Vector(pos.x - width_2, pos.y - height_2, pos.z));
		v.setNormal(Vector::negativeUnitZ);
		v.setColor0(m_color);
		v.setTextureCoordinates(0, 0.0f, 1.0f);

	m_vertexBuffer.unlock();

	Graphics::setVertexBuffer(m_vertexBuffer);
	Graphics::setObjectToWorldTransformAndScale(ShaderPrimitiveSorter::getCurrentCamera().getTransform_o2w(), Vector::xyz111);
}

// ----------------------------------------------------------------------

void SpriteAppearance::LocalShaderPrimitive::draw() const
{
	Graphics::drawTriangleFan();
}

// ----------------------------------------------------------------------

inline void SpriteAppearance::LocalShaderPrimitive::setPulse(bool newPulse)
{
	m_pulse = newPulse;
}

// ----------------------------------------------------------------------

inline const Shader *SpriteAppearance::LocalShaderPrimitive::getShader() const
{
	return m_shader;
}

// ----------------------------------------------------------------------

inline Shader *SpriteAppearance::LocalShaderPrimitive::getShader()
{
	return m_shader;
}

// ----------------------------------------------------------------------

inline VectorArgb &SpriteAppearance::LocalShaderPrimitive::getColor()
{
	return m_color; //lint !e1536 // exposing low aaccess member
}

// ----------------------------------------------------------------------

inline const VectorArgb &SpriteAppearance::LocalShaderPrimitive::getColor() const
{
	return m_color;
}

// ----------------------------------------------------------------------

inline void SpriteAppearance::LocalShaderPrimitive::setColor(const VectorArgb &color)
{
	m_color = color;
}

// ----------------------------------------------------------------------

inline void SpriteAppearance::LocalShaderPrimitive::setAlpha(float alpha)
{
	m_color.a = alpha;
}

// ----------------------------------------------------------------------

inline void SpriteAppearance::LocalShaderPrimitive::setRadius(float radius)
{
	m_width = radius;
	m_height = radius;
}

// ----------------------------------------------------------------------

inline const Sphere &SpriteAppearance::LocalShaderPrimitive::getSphere() const
{
	return m_sphere;
}

//----------------------------------------------------------------------

float SpriteAppearance::LocalShaderPrimitive::getRadius() const
{
	return std::max(m_width, m_height) * 1.45f;
}

// ======================================================================

void SpriteAppearance::install()
{
	DEBUG_FATAL(ms_memoryBlockManager, ("SpriteAppearance already installed"));
	ms_memoryBlockManager = new MemoryBlockManager("SpriteAppearance::memoryBlockManager", true, sizeof(SpriteAppearance), 0, 0, 0);

	ExitChain::add (remove, "SpriteAppearance::remove");
}

// ----------------------------------------------------------------------

void SpriteAppearance::remove()
{
	DEBUG_FATAL(!ms_memoryBlockManager, ("SpriteAppearance is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

// ----------------------------------------------------------------------

void *SpriteAppearance::operator new(size_t size)
{
	UNREF(size);
	NOT_NULL(ms_memoryBlockManager);
	DEBUG_FATAL(size != sizeof(SpriteAppearance), ("bad size"));
	DEBUG_FATAL(size != static_cast<size_t>(ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void SpriteAppearance::operator delete(void *pointer)
{
	NOT_NULL(ms_memoryBlockManager);
	ms_memoryBlockManager->free(pointer);
}

// ----------------------------------------------------------------------

SpriteAppearance::SpriteAppearance(const SpriteAppearanceTemplate* spriteAppearanceTemplate, const Vector& newCenter, float newRadius, Shader* newShader, const VectorArgb& newColor)
: Appearance(spriteAppearanceTemplate),
	m_localShaderPrimitive(NULL),
	m_dpvsObject(NULL)
{
	//-- make extent fit inside square rather than contain it
	static const float sqrt_2 = sqrt (2.0f);
	newRadius *= sqrt_2;

	m_localShaderPrimitive = new LocalShaderPrimitive(*this, newCenter, newRadius, newShader, newColor);

	if (!spriteAppearanceTemplate)
	{
		Extent *e = new Extent();
		e->setSphere(m_localShaderPrimitive->getSphere());
		setExtent(ExtentList::fetch(e));
	}

	{
		DPVS::Model *const sphereModel = RenderWorld::fetchSphereModel(getSphere());
		m_dpvsObject = RenderWorld::createObject(this, sphereModel);
		sphereModel->release();
	}
}

// ----------------------------------------------------------------------

SpriteAppearance::~SpriteAppearance()
{
	delete m_localShaderPrimitive;
	m_dpvsObject->release();
}

// ----------------------------------------------------------------------

void SpriteAppearance::render() const
{
	ShaderPrimitiveSorter::add(*m_localShaderPrimitive);

#ifdef _DEBUG
	Appearance::render();
#endif
}

// ----------------------------------------------------------------------

float SpriteAppearance::alter(float alterTime)
{
	return m_localShaderPrimitive->alter(alterTime);
}

// ----------------------------------------------------------------------

const Sphere& SpriteAppearance::getSphere() const
{
	return m_localShaderPrimitive->getSphere();
}

// ----------------------------------------------------------------------

void SpriteAppearance::setPulse(bool pulse)
{
	m_localShaderPrimitive->setPulse(pulse);
}

// ----------------------------------------------------------------------

const Shader* SpriteAppearance::getShader() const
{
	return m_localShaderPrimitive->getShader();
}

// ----------------------------------------------------------------------

Shader* SpriteAppearance::getShader()
{
	return m_localShaderPrimitive->getShader();
}

// ----------------------------------------------------------------------

VectorArgb &SpriteAppearance::getColor()
{
	return m_localShaderPrimitive->getColor();
}

// ----------------------------------------------------------------------

const VectorArgb &SpriteAppearance::getColor() const
{
	return m_localShaderPrimitive->getColor();
}

// ----------------------------------------------------------------------

void SpriteAppearance::setColor(const VectorArgb &color)
{
	m_localShaderPrimitive->setColor(color);
}

// ----------------------------------------------------------------------

void SpriteAppearance::setAlpha(bool const /* opaqueEnabled */, float const /* opaqueAlpha */, bool const /* alphaEnabled */, float const alphaAlpha)
{
	m_localShaderPrimitive->setAlpha(alphaAlpha);
}

// ----------------------------------------------------------------------

void SpriteAppearance::setRadius(float radius)
{
	m_localShaderPrimitive->setRadius(radius);
}

//-----------------------------------------------------------------

DPVS::Object *SpriteAppearance::getDpvsObject() const
{
	return m_dpvsObject;
}

//-----------------------------------------------------------------

AxialBox const SpriteAppearance::getTangibleExtent() const
{
	Extent const * const extent = getExtent();
	if (extent)
		return extent->getBoundingBox();
	else
		return AxialBox();
}

// ======================================================================
