// ======================================================================
//
// MarkerAppearance.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/MarkerAppearance.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientObject/MarkerAppearanceTemplate.h"
#include "sharedCollision/Extent.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Object.h"

#include "dpvsModel.hpp"
#include "dpvsObject.hpp"

#include <cstdlib>

// ======================================================================

MemoryBlockManager *MarkerAppearance::ms_memoryBlockManager;

// ======================================================================

// @todo MBM this class

class MarkerAppearance::LocalShaderPrimitive : public ShaderPrimitive
{
public:

	static VertexBufferFormat getVertexBufferFormat();

public:

	LocalShaderPrimitive(const MarkerAppearance &owner, const MarkerAppearanceTemplate &appearanceTemplate);
	virtual ~LocalShaderPrimitive();

	virtual float               alter(float deltaTime);
	virtual const Vector        getPosition_w() const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader &prepareToView() const;
	virtual void                prepareToDraw() const;
	virtual void                draw() const;

	const Sphere               &getSphere(void) const;
	void                        setAlpha(real newAlpha);

private:

	const Appearance           *getTargetAppearance() const;

private:

	const MarkerAppearance         &m_owner;
	const MarkerAppearanceTemplate &m_appearanceTemplate;
	mutable DynamicVertexBuffer     m_vertexBuffer;

	Sphere                          m_sphere;
	float                           m_width;
	float                           m_height;
	const Shader                   *m_shader;
	VectorArgb                      m_color;

	float                           m_texturesPerSecond;
	float                           m_textureStartIndex;
	float                           m_textureStopIndexPlusOne;
	float                           m_currentTextureIndex;

private:

	LocalShaderPrimitive();
	LocalShaderPrimitive(const LocalShaderPrimitive &); //lint -esym(754, LocalShaderPrimitive::LocalShaderPrimitive) // local structure member unreferenced // Defensive hiding.
	LocalShaderPrimitive &operator =(const LocalShaderPrimitive &);

};

// ======================================================================

VertexBufferFormat MarkerAppearance::LocalShaderPrimitive::getVertexBufferFormat()
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

MarkerAppearance::LocalShaderPrimitive::LocalShaderPrimitive(const MarkerAppearance &owner, const MarkerAppearanceTemplate &appearanceTemplate) :
	ShaderPrimitive(),
	m_owner(owner),
	m_appearanceTemplate(appearanceTemplate),
	m_vertexBuffer(getVertexBufferFormat()),
	m_sphere(Vector::zero, 1.0f),
	m_width(1.0f),
	m_height(1.0f),
	m_shader(appearanceTemplate.fetchShader()),
	m_color(VectorArgb::solidWhite),
	m_texturesPerSecond(appearanceTemplate.getTexturesPerSecond()),
	m_textureStartIndex(static_cast<float>(appearanceTemplate.getTextureStartIndex())),
	m_textureStopIndexPlusOne(static_cast<float>(appearanceTemplate.getTextureStopIndex()) + 1.0f),
	m_currentTextureIndex(0.0f)
{
	const Appearance *const targetAppearance = getTargetAppearance();
	if (targetAppearance)
	{
		//-- Setup sphere.
		const Sphere &targetSphere = targetAppearance->getSphere();
		m_sphere                   = targetSphere;

		//-- Setup width and height.
		m_width  = 2.0f * targetSphere.getRadius();
		m_height = m_width;
	}
}

// ----------------------------------------------------------------------

MarkerAppearance::LocalShaderPrimitive::~LocalShaderPrimitive()
{
	m_shader->release();
	m_shader = 0;
}

// ----------------------------------------------------------------------

float MarkerAppearance::LocalShaderPrimitive::alter(float deltaTime)
{
	//-- Calculate current sub-texture index.
	m_currentTextureIndex += deltaTime * m_texturesPerSecond;

	do
	{
		if (m_currentTextureIndex < m_textureStartIndex)
		{
			// We must be ping ponging.  Reverse the direction of motion.
			m_texturesPerSecond = -m_texturesPerSecond;

			// Adjust time.
			m_currentTextureIndex = m_textureStartIndex +  (m_textureStartIndex - m_currentTextureIndex);
		}
		else if (m_currentTextureIndex >= m_textureStopIndexPlusOne)
		{
			switch (m_appearanceTemplate.getTextureAnimationMode())
			{
				case MarkerAppearanceTemplate::AM_timeBasedWrap:
					// Wrap, adjust time.
					m_currentTextureIndex = m_textureStartIndex + (m_currentTextureIndex - m_textureStopIndexPlusOne);
					break;

				case MarkerAppearanceTemplate::AM_timeBasedPingPong:
					// Reverse the direction of motion.
					m_texturesPerSecond = -m_texturesPerSecond;

					// Adjust time.
					m_currentTextureIndex = m_textureStopIndexPlusOne - (m_currentTextureIndex - m_textureStopIndexPlusOne);
					break;

				case MarkerAppearanceTemplate::AM_none:
				case MarkerAppearanceTemplate::AM_COUNT:
				default:
					// Do nothing.
					break;
			}
		}
	} while ((m_currentTextureIndex < m_textureStartIndex) || (m_currentTextureIndex >= m_textureStopIndexPlusOne));

	//-- This instance does work every alter, request an alter next frame.
	return AlterResult::cms_alterNextFrame;
}

// ----------------------------------------------------------------------

const Vector MarkerAppearance::LocalShaderPrimitive::getPosition_w() const
{
	return m_owner.getTransform_w().getPosition_p();
}
	
// ----------------------------------------------------------------------

float MarkerAppearance::LocalShaderPrimitive::getDepthSquaredSortKey() const
{
	// @todo this is not exactly accurate, it shows as being slighting farther away than it really is.  Fix but
	//       don't make it expensive.
	return m_owner.getTransform_w().getPosition_p().magnitudeBetweenSquared(ShaderPrimitiveSorter::getCurrentCameraPosition());
}

// ----------------------------------------------------------------------

int MarkerAppearance::LocalShaderPrimitive::getVertexBufferSortKey() const
{
	return m_vertexBuffer.getSortKey();
}

// ----------------------------------------------------------------------

const StaticShader &MarkerAppearance::LocalShaderPrimitive::prepareToView() const
{
	// @todo prepareForView prepareToView should have the same name
	return m_shader->prepareToView();
}

// ----------------------------------------------------------------------

void MarkerAppearance::LocalShaderPrimitive::prepareToDraw() const
{
	//-- Get position of object from objectToWorld.
	const Appearance *targetAppearance = getTargetAppearance();
	if (!targetAppearance)
	{
		// @todo handle this properly.
		targetAppearance = &m_owner;
	}
	NOT_NULL(targetAppearance);

	//-- Move the rendered quad X meters closer to the camera than the target object,
	//   where X = (target appearance radius) * radius multiplier + radius addition.
	const Sphere &targetSphere      = targetAppearance->getSphere();
	const float   zCameraAdjustment = targetSphere.getRadius() * m_appearanceTemplate.getParentRadiusMultiplier() + m_appearanceTemplate.getParentRadiusAddition();

	Vector pos  = ShaderPrimitiveSorter::getCurrentCamera().rotateTranslate_w2o(targetAppearance->getTransform_w().rotateTranslate_l2p(targetSphere.getCenter()));
	pos.z      -= zCameraAdjustment;

	//-- Get UV coordinates for current animated sub-texture.
	float left;
	float top;
	float right;
	float bottom;

	const int textureIndex = static_cast<int>(m_currentTextureIndex);
	m_appearanceTemplate.getUvCoordinates(textureIndex, left, top, right, bottom);

	// add offsets to make a camera space sprite (always facing the camera)
	const float width_2  = targetSphere.getRadius()  * 0.5f;
	const float height_2 = width_2;

	m_vertexBuffer.lock(4);

		VertexBufferWriteIterator v = m_vertexBuffer.begin();

		v.setPosition(Vector(pos.x - width_2, pos.y + height_2, pos.z));
		v.setNormal(Vector::negativeUnitZ);
		v.setColor0(m_color);
		v.setTextureCoordinates(0, left, top);
		++v;

		v.setPosition(Vector(pos.x + width_2, pos.y + height_2, pos.z));
		v.setNormal(Vector::negativeUnitZ);
		v.setColor0(m_color);
		v.setTextureCoordinates(0, right, top);
		++v;

		v.setPosition(Vector(pos.x + width_2, pos.y - height_2, pos.z));
		v.setNormal(Vector::negativeUnitZ);
		v.setColor0(m_color);
		v.setTextureCoordinates(0, right, bottom);
		++v;

		v.setPosition(Vector(pos.x - width_2, pos.y - height_2, pos.z));
		v.setNormal(Vector::negativeUnitZ);
		v.setColor0(m_color);
		v.setTextureCoordinates(0, left, bottom);

	m_vertexBuffer.unlock();

	Graphics::setVertexBuffer(m_vertexBuffer);
	Graphics::setObjectToWorldTransformAndScale(ShaderPrimitiveSorter::getCurrentCamera().getTransform_o2w(), Vector::xyz111);
}

// ----------------------------------------------------------------------

void MarkerAppearance::LocalShaderPrimitive::draw() const
{
	Graphics::drawTriangleFan();
}

// ----------------------------------------------------------------------

inline const Sphere &MarkerAppearance::LocalShaderPrimitive::getSphere() const
{
	return m_sphere;
}

// ----------------------------------------------------------------------

inline void MarkerAppearance::LocalShaderPrimitive::setAlpha(float alpha)
{
	m_color.a = alpha;
}

// ----------------------------------------------------------------------

const Appearance *MarkerAppearance::LocalShaderPrimitive::getTargetAppearance() const
{
	const Object *const ownerObject = m_owner.getOwner();
	if (!ownerObject)
		return 0;

	const Object *const parentObject = ownerObject->getParent();
	if (!parentObject)
		return 0;

	return parentObject->getAppearance();
}

// ======================================================================

void MarkerAppearance::install()
{
	DEBUG_FATAL(ms_memoryBlockManager, ("MarkerAppearance already installed"));
	ms_memoryBlockManager = new MemoryBlockManager("MarkerAppearance::memoryBlockManager", true, sizeof(MarkerAppearance), 0, 0, 0);

	ExitChain::add(remove, "MarkerAppearance");
}

// ----------------------------------------------------------------------

void *MarkerAppearance::operator new(size_t size)
{
	UNREF(size);
	NOT_NULL(ms_memoryBlockManager);
	DEBUG_FATAL(size != sizeof(MarkerAppearance), ("bad size"));
	DEBUG_FATAL(size != static_cast<size_t>(ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void MarkerAppearance::operator delete(void *pointer)
{
	NOT_NULL(ms_memoryBlockManager);
	ms_memoryBlockManager->free(pointer);
}

// ----------------------------------------------------------------------

MarkerAppearance::MarkerAppearance(const MarkerAppearanceTemplate &appearanceTemplate) :
	Appearance(&appearanceTemplate),
	m_localShaderPrimitive(NULL),
	m_dpvsObject(NULL)
{
	m_localShaderPrimitive = new LocalShaderPrimitive(*this, appearanceTemplate);

	{
		DPVS::Model *const sphereModel = RenderWorld::fetchSphereModel(MarkerAppearance::getSphere());
		m_dpvsObject = RenderWorld::createObject(this, sphereModel);
		IGNORE_RETURN(sphereModel->release());
	}
}

// ----------------------------------------------------------------------

MarkerAppearance::~MarkerAppearance()
{
	delete m_localShaderPrimitive;

	IGNORE_RETURN(m_dpvsObject->release());
	m_dpvsObject = 0;
}

// ----------------------------------------------------------------------

void MarkerAppearance::render() const
{
	ShaderPrimitiveSorter::add(*m_localShaderPrimitive);

#ifdef _DEBUG
	Appearance::render();
#endif
}

// ----------------------------------------------------------------------

float MarkerAppearance::alter(float deltaTime)
{
	return m_localShaderPrimitive->alter(deltaTime);
}

// ----------------------------------------------------------------------

const Sphere& MarkerAppearance::getSphere() const
{
	return m_localShaderPrimitive->getSphere();
}

// ----------------------------------------------------------------------

void MarkerAppearance::setAlpha(bool const /* opaqueEnabled */, float const /* opaqueAlpha */, bool const /* alphaEnabled */, float const alphaAlpha)
{
	m_localShaderPrimitive->setAlpha(alphaAlpha);
}

// ======================================================================

void MarkerAppearance::remove()
{
	DEBUG_FATAL(!ms_memoryBlockManager, ("MarkerAppearance is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

// ======================================================================

DPVS::Object *MarkerAppearance::getDpvsObject() const
{
	return m_dpvsObject;
} //lint !e1763 // (Info -- Member function 'MarkerAppearance::getDpvsObject(void) const' marked as const indirectly modifies class) // JG wants it this way.  I think it's a bit dangerous.

// ======================================================================
