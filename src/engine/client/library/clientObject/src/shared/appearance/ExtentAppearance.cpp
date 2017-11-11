//======================================================================
//
// ExtentAppearance.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/ExtentAppearance.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "sharedCollision/Extent.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Vector.h"
#include "sharedMath/VectorArgb.h"

#include "dpvsModel.hpp"
#include "dpvsObject.hpp"

// ======================================================================

class ExtentAppearance::LocalShaderPrimitive: public ShaderPrimitive
{
public:

	explicit LocalShaderPrimitive(ExtentAppearance &appearance);
	virtual ~LocalShaderPrimitive();

	virtual Vector const getPosition_w() const;
	virtual StaticShader const & prepareToView() const;
	virtual float getDepthSquaredSortKey() const;
	virtual int getVertexBufferSortKey() const;
	virtual void prepareToDraw() const;
	virtual void draw() const;

private:

	ExtentAppearance & m_appearance;

private:
	LocalShaderPrimitive();
	LocalShaderPrimitive& operator =(const LocalShaderPrimitive&);
};

// ======================================================================

ExtentAppearance::LocalShaderPrimitive::LocalShaderPrimitive(ExtentAppearance &appearance)
: ShaderPrimitive()
, m_appearance(appearance)
{
}

// ----------------------------------------------------------------------

ExtentAppearance::LocalShaderPrimitive::~LocalShaderPrimitive()
{
}

// ----------------------------------------------------------------------

const StaticShader &ExtentAppearance::LocalShaderPrimitive::prepareToView() const
{
	return ShaderTemplateList::get3dVertexColorZStaticShader();
}

// ----------------------------------------------------------------------

const Vector ExtentAppearance::LocalShaderPrimitive::getPosition_w() const
{
	return m_appearance.getTransform_w().getPosition_p();
}
	
// ----------------------------------------------------------------------

float ExtentAppearance::LocalShaderPrimitive::getDepthSquaredSortKey() const
{
	return m_appearance.getTransform_w().getPosition_p().magnitudeBetweenSquared(ShaderPrimitiveSorter::getCurrentCameraPosition());
}

// ----------------------------------------------------------------------

int ExtentAppearance::LocalShaderPrimitive::getVertexBufferSortKey() const
{
	// not using vertex buffers
	return 0;
}

// ----------------------------------------------------------------------

void ExtentAppearance::LocalShaderPrimitive::prepareToDraw() const
{
	Graphics::setObjectToWorldTransformAndScale(m_appearance.getTransform_w(), Vector::xyz111);
}

// ----------------------------------------------------------------------

void ExtentAppearance::LocalShaderPrimitive::draw() const
{
	Graphics::setStaticShader(ShaderTemplateList::get3dVertexColorZStaticShader());

	Extent const * const extent = m_appearance.getExtentToRender();
	VectorArgb const & color = m_appearance.getColor();

	Graphics::drawExtent(extent, color);
}

// ======================================================================

ExtentAppearance::ExtentAppearance(Extent * const extent, VectorArgb const & color)
: Appearance(0)
, m_shaderPrimitive(0)
, m_dpvsObject(0)
, m_extentToRender(extent)
, m_color(new VectorArgb(color))
, m_sphere(new Sphere(m_extentToRender->getBoundingSphere()))
{
	m_shaderPrimitive = new LocalShaderPrimitive(*this);
}

// ----------------------------------------------------------------------

ExtentAppearance::~ExtentAppearance()
{
	delete m_color;
	m_color = 0;

	delete m_sphere;
	m_sphere = 0;

	delete m_shaderPrimitive;
	m_shaderPrimitive = 0;

	if (m_dpvsObject != 0)
	{
		m_dpvsObject->release();
		m_dpvsObject = 0;
	}

	delete m_extentToRender;
	m_extentToRender = 0;
}

// ----------------------------------------------------------------------

Sphere const & ExtentAppearance::getSphere() const
{
	return *m_sphere;
}

// ----------------------------------------------------------------------

DPVS::Object * ExtentAppearance::getDpvsObject() const
{
	return m_dpvsObject;
}

// ----------------------------------------------------------------------

void ExtentAppearance::render() const
{
	ShaderPrimitiveSorter::add(*m_shaderPrimitive);
}

// ----------------------------------------------------------------------

Extent const * ExtentAppearance::getExtentToRender() const
{
	return m_extentToRender;
}

// ----------------------------------------------------------------------

Extent * ExtentAppearance::getExtentToRender()
{
	return m_extentToRender;
}

// ----------------------------------------------------------------------

VectorArgb const & ExtentAppearance::getColor() const
{
	return *m_color;
}

// ======================================================================
