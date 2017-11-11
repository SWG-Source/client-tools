// ======================================================================
//
// ShaderAppearance.h
// Copyright 2000, 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/ShaderAppearance.h"

#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/StaticShader.h"
#include "sharedCollision/Extent.h"
#include "sharedCollision/ExtentList.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Transform.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Object.h"

#include "dpvsObject.hpp"

#include <algorithm>

// ======================================================================

const Tag TAG_DOT3 = TAG(D,O,T,3);

class ShaderAppearance::LocalShaderPrimitive: public ShaderPrimitive
{
public:

	explicit LocalShaderPrimitive (ShaderAppearance &appearance);
	~LocalShaderPrimitive ();

	virtual const StaticShader &prepareToView() const;
	virtual const Vector        getPosition_w() const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual void                prepareToDraw() const;
	virtual void                draw() const;

private:

	ShaderAppearance &m_appearance;

private:
	// disabled
	LocalShaderPrimitive ();
	LocalShaderPrimitive& operator = (const LocalShaderPrimitive&); //lint -esym(754, LocalShaderPrimitive::operator=) // unreferenced
};

// ======================================================================
// class ShaderAppearance::LocalShaderPrimitive
// ======================================================================

ShaderAppearance::LocalShaderPrimitive::LocalShaderPrimitive (ShaderAppearance &appearance)
:	ShaderPrimitive (),
	m_appearance(appearance)
{
}

// ----------------------------------------------------------------------

ShaderAppearance::LocalShaderPrimitive::~LocalShaderPrimitive ()
{
}

// ----------------------------------------------------------------------

const StaticShader &ShaderAppearance::LocalShaderPrimitive::prepareToView () const
{
	return m_appearance.getShader ().prepareToView ();
}

// ----------------------------------------------------------------------

const Vector ShaderAppearance::LocalShaderPrimitive::getPosition_w() const
{
	return m_appearance.getTransform_w ().getPosition_p ();
}
	
// ----------------------------------------------------------------------

float ShaderAppearance::LocalShaderPrimitive::getDepthSquaredSortKey () const
{
	return m_appearance.getTransform_w ().getPosition_p ().magnitudeBetweenSquared (ShaderPrimitiveSorter::getCurrentCameraPosition ());
}

// ----------------------------------------------------------------------

int ShaderAppearance::LocalShaderPrimitive::getVertexBufferSortKey () const
{
	return 0;
}

// ----------------------------------------------------------------------

void ShaderAppearance::LocalShaderPrimitive::prepareToDraw () const
{
	Graphics::setObjectToWorldTransformAndScale(m_appearance.getTransform_w(), m_appearance.getScale());

	//--
	const int textureCount = 8;

	VertexBufferFormat format;
	format.setPosition();
	format.setNormal();
	format.setColor0();
	format.setColor1();
	format.setNumberOfTextureCoordinateSets(textureCount);

	uint8 dot3 = 255;
	if (m_appearance.getShader ().getStaticShader () && !m_appearance.getShader ().getStaticShader ()->getTextureCoordinateSet(TAG_DOT3, dot3))
		dot3 = 255;

	int i;
	for (i = 0; i < textureCount; ++i)
		if (i == dot3)
			format.setTextureCoordinateSetDimension(i, 4);
		else
			format.setTextureCoordinateSetDimension(i, 2);

	VectorArgb const & color = m_appearance.getColor ();
	float const radiusX = m_appearance.getRadiusX ();
	float const radiusY = m_appearance.getRadiusY ();
	float const uvScale = m_appearance.getUvScale ();

	DynamicVertexBuffer vertexBuffer (format);
	vertexBuffer.lock(4);

		VertexBufferWriteIterator v = vertexBuffer.begin();

		v.setPosition (Vector (-radiusX, radiusY, 0.0f));
		v.setNormal (Vector::negativeUnitZ);
		v.setColor0 (color);
		v.setColor1 (color);
		v.setTextureCoordinates (0, 0.0f, 0.0f);
		for (i = 1; i < textureCount; i++)
			if (i == dot3)
				v.setTextureCoordinates ( i, 1.0f, 0.0f, 0.0f, 1.0f);
			else
				v.setTextureCoordinates ( i, uvScale * 0.0f, uvScale * 0.0f);
		++v;

		v.setPosition (Vector ( radiusX, radiusY, 0.0f));
		v.setNormal (Vector::negativeUnitZ);
		v.setColor0 (color);
		v.setColor1 (color);
		v.setTextureCoordinates (0, 1.0f, 0.0f);
		for (i = 1; i < textureCount; i++)
			if (i == dot3)
				v.setTextureCoordinates ( i, 1.0f, 0.0f, 0.0f, 1.0f);
			else
				v.setTextureCoordinates (i, uvScale * 1.0f, uvScale * 0.0f);
		++v;

		v.setPosition (Vector ( radiusX, -radiusY, 0.0f));
		v.setNormal (Vector::negativeUnitZ);
		v.setColor0 (color);
		v.setColor1 (color);
		v.setTextureCoordinates (0, 1.0f, 1.0f);
		for (i = 1; i < textureCount; i++)
			if (i == dot3)
				v.setTextureCoordinates ( i, 1.0f, 0.0f, 0.0f, 1.0f);
			else
				v.setTextureCoordinates (i, uvScale * 1.0f, uvScale * 1.0f);
		++v;

		v.setPosition (Vector (-radiusX, -radiusY, 0.0f));
		v.setNormal (Vector::negativeUnitZ);
		v.setColor0 (color);
		v.setColor1 (color);
		v.setTextureCoordinates (0, 0.0f, 1.0f);
		for (i = 1; i < textureCount; i++)
			if (i == dot3)
				v.setTextureCoordinates ( i, 1.0f, 0.0f, 0.0f, 1.0f);
			else
				v.setTextureCoordinates (i, uvScale * 0.0f, uvScale * 1.0f);

	vertexBuffer.unlock();

	Graphics::setVertexBuffer (vertexBuffer);
}

// ----------------------------------------------------------------------

void ShaderAppearance::LocalShaderPrimitive::draw () const
{
	Graphics::drawTriangleFan ();
}

// ======================================================================
// class ShaderAppearance
// ======================================================================

ShaderAppearance::ShaderAppearance (float const radiusX, float const radiusY, Shader * const shader, VectorArgb const & color, float const uvScale)
:	Appearance (0),
	m_ignoreMipmapLevelCount (0),
	m_shader (shader),
	m_color (color),
	m_radiusX (radiusX),
	m_radiusY (radiusY),
	m_uvScale (uvScale),
	m_shaderPrimitive (0),
	m_dpvsObject(0)
{
	m_shaderPrimitive = new LocalShaderPrimitive (*this);

	Extent* e = new Extent ();
	e->setSphere (Sphere (Vector::zero, std::max (m_radiusX, m_radiusY)));
	Appearance::setExtent (ExtentList::fetch (e));

	m_dpvsObject = RenderWorld::createObject(this, e->getSphere ().getRadius ());
} //lint !e429 // (Warning -- Custodial pointer 'e' (line 177) has not been freed or returned) // Appearance will release

// ----------------------------------------------------------------------

ShaderAppearance::~ShaderAppearance()
{
	delete m_shaderPrimitive;

	m_dpvsObject->release();
	m_dpvsObject = NULL;

	if (m_shader)
		m_shader->release();
}

// ----------------------------------------------------------------------

const Sphere& ShaderAppearance::getSphere () const
{
	// @todo short circuit this virtual function call when Appearance
	//       no longer stores its own Extent.  The Extent probably should be returned
	//       and stored in each Appearance-derived class, or it shouldn't be
	//       virtual at all.
	return getExtent()->getSphere ();
}

// ----------------------------------------------------------------------

DPVS::Object* ShaderAppearance::getDpvsObject() const
{
	return m_dpvsObject;
}

// ----------------------------------------------------------------------

void ShaderAppearance::render () const
{
	ShaderPrimitiveSorter::add (*m_shaderPrimitive);
}

// ----------------------------------------------------------------------

float ShaderAppearance::alter (float time)
{
	IGNORE_RETURN(Appearance::alter (time));

	if (m_shader)
		m_shader->alter (time);

	// @todo figure out what this should really return.
	return AlterResult::cms_alterNextFrame;
}

// ----------------------------------------------------------------------

void ShaderAppearance::setAlpha (bool const /* opaqueEnabled */, float const /* opaqueAlpha */, bool const /* alphaEnabled */, float const alphaAlpha)
{
	m_color.a = alphaAlpha;
}

// ----------------------------------------------------------------------

void ShaderAppearance::setColor (VectorArgb const & color)
{
	m_color = color;
}

// ======================================================================
