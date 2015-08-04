//
// CelestialAppearanceance.cpp
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2000-2001, Sony Online Entertainment Inc.
// All Rights Reserved.

// ----------------------------------------------------------------------

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/CelestialAppearance.h"

#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "sharedDebug/Profiler.h"
#include "sharedMath/Transform.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Object.h"

#include "dpvsObject.hpp"

// ----------------------------------------------------------------------

class CelestialAppearance::LocalShaderPrimitive : public ShaderPrimitive
{
public:

	static VertexBufferFormat getVertexBufferFormat();

public:

	LocalShaderPrimitive(const CelestialAppearance &owner, float newWidth, float newHeight, float newRoll, const char* shaderTemplateName, const VectorArgb& newColor);
	virtual ~LocalShaderPrimitive();

	virtual float               alter(float time);
	virtual const StaticShader &prepareToView() const;
	virtual const Vector        getPosition_w() const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual void                prepareToDraw() const;
	virtual void                draw() const;

	const Sphere               &getSphere() const;
	void                        setColor(const VectorArgb& newColor);
	void                        setAlpha(float newAlpha);

private:

	const Appearance            &m_owner;
	const Shader*                m_shader;

	mutable DynamicVertexBuffer  m_vertexBuffer;

	float                        m_width;
	float                        m_height;
	float                        m_roll;

	VectorArgb                   m_color;

private:
	
	LocalShaderPrimitive(const LocalShaderPrimitive &);
	LocalShaderPrimitive &operator =(const LocalShaderPrimitive &);
};

// ----------------------------------------------------------------------

VertexBufferFormat CelestialAppearance::LocalShaderPrimitive::getVertexBufferFormat()
{
	VertexBufferFormat format;
	format.setPosition();
	format.setColor0();
	format.setNumberOfTextureCoordinateSets(1);
	format.setTextureCoordinateSetDimension(0, 2);

	return format;
}

// ----------------------------------------------------------------------

CelestialAppearance::LocalShaderPrimitive::LocalShaderPrimitive(const CelestialAppearance &owner, float newWidth, float newHeight, float newRoll, const char* shaderTemplateName, const VectorArgb& newColor) :
	ShaderPrimitive (),
	m_owner(owner),
	m_shader(ShaderTemplateList::fetchShader (shaderTemplateName)),
	m_vertexBuffer(getVertexBufferFormat()),
	m_width(newWidth),
	m_height(newHeight),
	m_roll(newRoll),
	m_color(newColor)
{
}

// ----------------------------------------------------------------------

CelestialAppearance::LocalShaderPrimitive::~LocalShaderPrimitive()
{
	m_shader->release();
	m_shader = 0;
}

// ----------------------------------------------------------------------

float CelestialAppearance::LocalShaderPrimitive::alter(float time)
{
	return m_shader->alter(time);
}

// ----------------------------------------------------------------------

const Vector CelestialAppearance::LocalShaderPrimitive::getPosition_w() const
{
	return Vector::zero;
}
	
//-------------------------------------------------------------------

float CelestialAppearance::LocalShaderPrimitive::getDepthSquaredSortKey() const
{
	return 0.0f;
}

// ----------------------------------------------------------------------

int CelestialAppearance::LocalShaderPrimitive::getVertexBufferSortKey() const
{
	return 0;
}

// ----------------------------------------------------------------------

const StaticShader &CelestialAppearance::LocalShaderPrimitive::prepareToView() const
{
	return m_shader->prepareToView();
}

// ----------------------------------------------------------------------

void CelestialAppearance::LocalShaderPrimitive::prepareToDraw() const
{
	Transform t = m_owner.getTransform_w ();
	t.setPosition_p (ShaderPrimitiveSorter::getCurrentCameraPosition ());
	t.move_l (Vector::negativeUnitZ * 3.f);
	t.yaw_l (PI);

	Graphics::setObjectToWorldTransformAndScale (t, Vector::xyz111);

	m_vertexBuffer.lock(4);

		VertexBufferWriteIterator v = m_vertexBuffer.begin();

		v.setPosition(-m_width,  m_height, 0.0f);
		v.setTextureCoordinates(0, 0.0f, 0.0f);
		v.setColor0(m_color);
		++v;

		v.setPosition(m_width,  m_height, 0.0f);
		v.setTextureCoordinates(0, 1.0f , 0.0f);
		v.setColor0(m_color);
		++v;

		v.setPosition(m_width, -m_height, 0.0f);
		v.setTextureCoordinates(0, 1.0f, 1.0f);
		v.setColor0(m_color);
		++v;

		v.setPosition(-m_width, -m_height, 0.0f);
		v.setTextureCoordinates(0, 0.0f, 1.0f);
		v.setColor0(m_color);
	
	m_vertexBuffer.unlock();

	Graphics::setVertexBuffer(m_vertexBuffer);
}

// ----------------------------------------------------------------------

void CelestialAppearance::LocalShaderPrimitive::draw() const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("CelestialAppearance::LocalShaderPrimitive::draw");
	
	Graphics::drawTriangleFan();
}

// ----------------------------------------------------------------------

void CelestialAppearance::LocalShaderPrimitive::setColor(const VectorArgb& newColor)
{
	m_color = newColor;
}

// ----------------------------------------------------------------------

void CelestialAppearance::LocalShaderPrimitive::setAlpha(float newAlpha)
{
	m_color.a = newAlpha;
}

// ======================================================================

CelestialAppearance::CelestialAppearance(float newWidth, float newHeight, float newRoll, const char* shaderTemplateName, const VectorArgb &newColor)
: Appearance(0),
	m_localShaderPrimitive(NULL),
	m_dpvsObject(NULL)
{
	m_localShaderPrimitive = new LocalShaderPrimitive(*this, newWidth, newHeight, newRoll, shaderTemplateName, newColor);
	m_dpvsObject = RenderWorld::createUnboundedObject(this);
}

// ----------------------------------------------------------------------

CelestialAppearance::~CelestialAppearance()
{
	delete m_localShaderPrimitive;
	m_dpvsObject->release();
}

//-------------------------------------------------------------------

DPVS::Object* CelestialAppearance::getDpvsObject() const
{
	return m_dpvsObject;
}

//-------------------------------------------------------------------

void CelestialAppearance::render() const
{
	ShaderPrimitiveSorter::add(*m_localShaderPrimitive);
}

// ----------------------------------------------------------------------

float CelestialAppearance::alter(float time)
{
	return m_localShaderPrimitive->alter(time);
}

// ----------------------------------------------------------------------

void CelestialAppearance::setAlpha(bool const /* opaqueEnabled */, float const /* opaqueAlpha */, bool const /* alphaEnabled */, float const alphaAlpha)
{
	m_localShaderPrimitive->setAlpha(alphaAlpha);
}

// ----------------------------------------------------------------------

void CelestialAppearance::setColor(const VectorArgb& newColor)
{
	m_localShaderPrimitive->setColor(newColor);
}

// ----------------------------------------------------------------------

