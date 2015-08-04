// ======================================================================
//
// SkyBoxAppearance.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/SkyBoxAppearance.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderCapability.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/StaticIndexBuffer.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/StaticVertexBuffer.h"
#include "clientGraphics/Texture.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Tag.h"

#include "dpvsObject.hpp"

// ======================================================================
// SkyBoxAppearanceNamespace
// ======================================================================

namespace SkyBoxAppearanceNamespace
{
	const Tag TAG_MAIN = TAG (M,A,I,N);

	bool ms_noRenderSkyBox;

	void remove ();
}

using namespace SkyBoxAppearanceNamespace;

// ======================================================================
// SkyBoxAppearance::LocalShaderPrimitive
// ======================================================================

class SkyBoxAppearance::LocalShaderPrimitive : public ShaderPrimitive
{
public:

	static VertexBufferFormat getVertexBufferFormat ();

public:

	LocalShaderPrimitive (const SkyBoxAppearance &owner, const char* shaderTemplateName);
	LocalShaderPrimitive (const SkyBoxAppearance &owner, const Texture* texture);
	virtual ~LocalShaderPrimitive ();

	const char* getName () const;

	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey () const;
	virtual int                 getVertexBufferSortKey () const;
	virtual const StaticShader& prepareToView () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;

private:

	void init ();

private:

	const SkyBoxAppearance& m_owner;	
	Shader* const           m_shader;
	Texture const * const   m_texture;
	StaticVertexBuffer      m_vertexBuffer;
	StaticIndexBuffer       m_indexBuffer;

private:

	LocalShaderPrimitive ();
	LocalShaderPrimitive (const LocalShaderPrimitive&);
	LocalShaderPrimitive& operator= (const LocalShaderPrimitive&);
};

// ======================================================================
// STATIC PUBLIC SkyBoxAppearance::LocalShaderPrimitive
// ======================================================================

VertexBufferFormat SkyBoxAppearance::LocalShaderPrimitive::getVertexBufferFormat ()
{
	VertexBufferFormat format;
	format.setPosition ();
	format.setNumberOfTextureCoordinateSets (1);
	format.setTextureCoordinateSetDimension (0, 3);
	return format;
}

// ======================================================================
// PUBLIC SkyBoxAppearance::LocalShaderPrimitive
// ======================================================================

SkyBoxAppearance::LocalShaderPrimitive::LocalShaderPrimitive (const SkyBoxAppearance &owner, const char* const shaderTemplateName) : 
	ShaderPrimitive (),
	m_owner (owner),
	m_shader (ShaderTemplateList::fetchModifiableShader (shaderTemplateName)),
	m_texture (0),
	m_vertexBuffer (getVertexBufferFormat (), 8),
	m_indexBuffer (36)
{
	init ();
}

// ----------------------------------------------------------------------

SkyBoxAppearance::LocalShaderPrimitive::LocalShaderPrimitive (const SkyBoxAppearance &owner, const Texture* const texture) : 
	ShaderPrimitive (),
	m_owner (owner),
	m_shader (ShaderTemplateList::fetchModifiableShader ("shader/skybox.sht")),
	m_texture (texture),
	m_vertexBuffer (getVertexBufferFormat (), 8),
	m_indexBuffer (36)
{
	init ();

	if (m_texture)
	{
		m_texture->fetch ();
		m_shader->getStaticShader ()->setTexture (TAG_MAIN, *texture);
	}
}

// ----------------------------------------------------------------------

const char* SkyBoxAppearance::LocalShaderPrimitive::getName () const
{
	return m_texture ? m_texture->getName () : "null";
}

// ----------------------------------------------------------------------

void SkyBoxAppearance::LocalShaderPrimitive::init ()
{
	//-- setup vertex buffer
	const Vector vertices [8] =
	{
		Vector (-1.f,  1.f,  1.f),
		Vector ( 1.f,  1.f,  1.f),
		Vector ( 1.f,  1.f, -1.f),
		Vector (-1.f,  1.f, -1.f),
		Vector (-1.f, -1.f,  1.f),
		Vector ( 1.f, -1.f,  1.f),
		Vector ( 1.f, -1.f, -1.f),
		Vector (-1.f, -1.f, -1.f)
	};

	m_vertexBuffer.lock ();
	{
		VertexBufferWriteIterator v = m_vertexBuffer.begin ();

		int i;
		for (i = 0; i < 8; ++i, ++v)
		{
			v.setPosition (vertices [i]);
			v.setTextureCoordinates (0, vertices [i].x, vertices [i].y, vertices [i].z);
		}
	}
	m_vertexBuffer.unlock ();

	//-- setup index buffer
	const Index indices [36] =
	{
		0, 2, 1, 
		0, 3, 2, 
		2, 5, 1, 
		2, 6, 5, 
		3, 6, 2, 
		3, 7, 6, 
		0, 7, 3, 
		0, 4, 7, 
		1, 4, 0, 
		1, 5, 4, 
		7, 5, 6, 
		7, 4, 5
	};

	m_indexBuffer.lock ();
	{
		Index* const indexBuffer = m_indexBuffer.begin ();
		memcpy (indexBuffer, indices, sizeof (Index) * 36);
	}
	m_indexBuffer.unlock ();
}

// ----------------------------------------------------------------------

SkyBoxAppearance::LocalShaderPrimitive::~LocalShaderPrimitive ()
{
	if (m_texture)
		m_texture->release ();

	m_shader->release ();
}

// ----------------------------------------------------------------------

const Vector SkyBoxAppearance::LocalShaderPrimitive::getPosition_w () const
{
	return Vector::zero;
}
	
//-------------------------------------------------------------------

float SkyBoxAppearance::LocalShaderPrimitive::getDepthSquaredSortKey () const
{
	return 0.f;
}

// ----------------------------------------------------------------------

int SkyBoxAppearance::LocalShaderPrimitive::getVertexBufferSortKey () const
{
	return m_vertexBuffer.getSortKey ();
}

// ----------------------------------------------------------------------

const StaticShader &SkyBoxAppearance::LocalShaderPrimitive::prepareToView () const
{
	return m_shader->prepareToView ();
}

// ----------------------------------------------------------------------

void SkyBoxAppearance::LocalShaderPrimitive::prepareToDraw () const
{
	Transform transform = m_owner.getTransform_w ();
	transform.setPosition_p (ShaderPrimitiveSorter::getCurrentCameraPosition ());
	Graphics::setObjectToWorldTransformAndScale (transform, m_owner.getScale ());
	Graphics::setVertexBuffer (m_vertexBuffer);
	Graphics::setIndexBuffer (m_indexBuffer);
}

// ----------------------------------------------------------------------

void SkyBoxAppearance::LocalShaderPrimitive::draw () const
{
	Graphics::drawIndexedTriangleList ();
}

// ======================================================================
// STATIC PUBLIC SkyBoxAppearance::LocalShaderPrimitive
// ======================================================================

void SkyBoxAppearance::install ()
{
	DebugFlags::registerFlag (ms_noRenderSkyBox, "ClientTerrain", "noRenderSkyBox");
	ExitChain::add (SkyBoxAppearanceNamespace::remove, "SkyBoxAppearanceNamespace::remove");
}

// ----------------------------------------------------------------------

void SkyBoxAppearanceNamespace::remove ()
{
	DebugFlags::unregisterFlag (ms_noRenderSkyBox);
}

// ======================================================================
// PUBLIC SkyBoxAppearance::LocalShaderPrimitive
// ======================================================================

SkyBoxAppearance::SkyBoxAppearance(char const * const shaderTemplateName, float const alpha) : 
	Appearance (0),
	m_localShaderPrimitive (0),
	m_dpvsObject (0),
	m_alpha(alpha)
{
	m_localShaderPrimitive = new LocalShaderPrimitive (*this, shaderTemplateName);
	m_dpvsObject = RenderWorld::createUnboundedObject (this);
}

// ----------------------------------------------------------------------

SkyBoxAppearance::SkyBoxAppearance(Texture const * const texture, float const alpha) :
	Appearance (0),
	m_localShaderPrimitive (0),
	m_dpvsObject (0),
	m_alpha (alpha)
{
	m_localShaderPrimitive = new LocalShaderPrimitive (*this, texture);
	m_dpvsObject = RenderWorld::createUnboundedObject (this);
}

// ----------------------------------------------------------------------

SkyBoxAppearance::~SkyBoxAppearance ()
{
	delete m_localShaderPrimitive;
	IGNORE_RETURN(m_dpvsObject->release());
}

// ----------------------------------------------------------------------

const char* SkyBoxAppearance::getName () const
{
	return m_localShaderPrimitive->getName ();
}

// ----------------------------------------------------------------------

DPVS::Object* SkyBoxAppearance::getDpvsObject () const
{
	return m_dpvsObject;
}

// ----------------------------------------------------------------------

void SkyBoxAppearance::setAlpha (bool const /* opaqueEnabled */, float const /* opaqueAlpha */, bool const /* alphaEnabled */, float const alphaAlpha)
{
	m_alpha = alphaAlpha;
}

// ----------------------------------------------------------------------

void SkyBoxAppearance::render () const
{
	if (ms_noRenderSkyBox)
		return;

	if (m_localShaderPrimitive && m_alpha > 0.f)
		ShaderPrimitiveSorter::add (*m_localShaderPrimitive);
}

// ======================================================================

