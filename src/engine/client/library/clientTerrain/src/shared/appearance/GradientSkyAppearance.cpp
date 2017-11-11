//===================================================================
//
// GradientSkyAppearance.cpp
// asommers
//
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
//===================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/GradientSkyAppearance.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderCapability.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSet.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticIndexBuffer.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/StaticVertexBuffer.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientObject/MeshAppearance.h"
#include "clientObject/MeshAppearanceTemplate.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Timer.h"
#include "sharedFractal/MultiFractal.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Object.h"
#include "sharedRandom/Random.h"
#include "sharedUtility/FileName.h"

#include "dpvsObject.hpp"

//===================================================================

namespace GradientSkyAppearanceNamespace
{
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef _DEBUG
	static bool s_noRenderSky = false;
#endif

	const Tag TAG_MAIN = TAG (M,A,I,N);

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void remove ();
}

using namespace GradientSkyAppearanceNamespace;

//===================================================================
// GradientSkyAppearance::LocalShaderPrimitive
//===================================================================

class GradientSkyAppearance::LocalShaderPrimitive : public ShaderPrimitive
{
public:

	static VertexBufferFormat   getVertexBufferFormat ();

public:

	explicit LocalShaderPrimitive (const char* textureName);
	virtual ~LocalShaderPrimitive ();

	const char* getName () const;

	virtual float               alter (float elapsedTime);
	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey () const;
	virtual int                 getVertexBufferSortKey () const;
	virtual const StaticShader &prepareToView () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;

	void                        setAlpha (float alpha);
	void                        setTime (float time);

private:

	Shader*                     m_shader;
	StaticVertexBuffer*         m_vertexBuffer;
	StaticIndexBuffer*          m_indexBuffer;
	float                       m_time;

private:

	LocalShaderPrimitive ();
	LocalShaderPrimitive (const LocalShaderPrimitive &);              //lint -esym (754, LocalShaderPrimitive::LocalShaderPrimitive)  // not referenced
	LocalShaderPrimitive &operator = (const LocalShaderPrimitive &);  //lint -esym (754, LocalShaderPrimitive::operator=)  // not referenced
};

//===================================================================

GradientSkyAppearance::LocalShaderPrimitive::LocalShaderPrimitive (const char* textureName) : 
	ShaderPrimitive (),
	m_shader (ShaderTemplateList::fetchModifiableShader ("shader/gradient_sky.sht")),
	m_vertexBuffer (0),
	m_indexBuffer (0),
	m_time (0.f)
{
	const Texture* texture = TextureList::fetch (textureName);
	safe_cast<StaticShader*> (m_shader)->setTexture (TAG (M,A,I,N), *texture);
	texture->release ();

	AsynchronousLoader::disable();
		const MeshAppearanceTemplate* const mat = safe_cast<const MeshAppearanceTemplate*> (AppearanceTemplateList::fetch ("appearance/gradient_sky.msh"));
	AsynchronousLoader::enable();

	if (mat)
	{
		AsynchronousLoader::disable();
			const MeshAppearance* const ma = safe_cast<const MeshAppearance*> (mat->createAppearance ());
		AsynchronousLoader::enable();

		if (ma)
		{
			const ShaderPrimitiveSet* const sps = ma->getShaderPrimitiveSet ();
			NOT_NULL (sps);
			DEBUG_FATAL (sps->getNumberOfShaders () == 0, ("geometry not found"));

			const StaticVertexBuffer* const vertexBuffer = sps->getVertexBuffer (0);
			const StaticIndexBuffer* const  indexBuffer  = sps->getIndexBuffer (0);

			//-- copy vertex buffer
			{
				VertexBufferFormat format;
				format.setPosition ();

				m_vertexBuffer = new StaticVertexBuffer (format, vertexBuffer->getNumberOfVertices ());

				m_vertexBuffer->lock ();
					vertexBuffer->lockReadOnly ();

						VertexBufferReadIterator  sv = vertexBuffer->beginReadOnly ();
						VertexBufferWriteIterator dv = m_vertexBuffer->begin ();

						int i;
						for (i = 0; i < vertexBuffer->getNumberOfVertices (); ++i, ++sv, ++dv)
							dv.setPosition (sv.getPosition ());

					vertexBuffer->unlock ();
				m_vertexBuffer->unlock ();
			}

			//-- copy index buffer
			{
				m_indexBuffer = new StaticIndexBuffer (indexBuffer->getNumberOfIndices ());

				m_indexBuffer->lock ();
					indexBuffer->lockReadOnly ();

						const Index* const sourceIndices      = indexBuffer->beginReadOnly ();
						Index* const       destinationIndices = m_indexBuffer->begin ();
						memcpy (destinationIndices, sourceIndices, sizeof (Index) * indexBuffer->getNumberOfIndices ());

					indexBuffer->unlock ();
				m_indexBuffer->unlock ();
			}

			delete ma;
		}

		AppearanceTemplateList::release (mat);
	}

	DEBUG_WARNING (!m_shader->getStaticShader ()->hasTextureFactor (TAG_MAIN), ("GradientSkyAppearance: %s is missing texture factor tag MAIN.  Please open and save this shader template in the ShaderBuilder.", m_shader->getName ()));
	setAlpha (0.f);
}

//-------------------------------------------------------------------

GradientSkyAppearance::LocalShaderPrimitive::~LocalShaderPrimitive ()
{
	if (m_shader)
	{
		m_shader->release ();
		m_shader = 0;
	}

	delete m_vertexBuffer;
	m_vertexBuffer = 0;
	
	delete m_indexBuffer;
	m_indexBuffer = 0;
}

//-------------------------------------------------------------------

const char* GradientSkyAppearance::LocalShaderPrimitive::getName () const
{
	return m_shader->getName ();
}

//-------------------------------------------------------------------

float GradientSkyAppearance::LocalShaderPrimitive::alter (float elapsedTime)
{
	return m_shader->alter (elapsedTime);
}

// ----------------------------------------------------------------------

void GradientSkyAppearance::LocalShaderPrimitive::setAlpha (const float alpha)
{
	PackedArgb color (PackedArgb::solidWhite);
	color.setA (static_cast<uint8> (alpha * 255));

	if (m_shader->getStaticShader ()->hasTextureFactor (TAG_MAIN))
		m_shader->getStaticShader ()->setTextureFactor (TAG_MAIN, color.getArgb ());
}

//-----------------------------------------------------------------

void GradientSkyAppearance::LocalShaderPrimitive::setTime (const float time)
{
	m_time = time;
}

//-----------------------------------------------------------------

const Vector GradientSkyAppearance::LocalShaderPrimitive::getPosition_w() const
{
	return Vector::zero;
}
	
//-------------------------------------------------------------------

float GradientSkyAppearance::LocalShaderPrimitive::getDepthSquaredSortKey () const
{
	return 0.f;
}

// ----------------------------------------------------------------------

int GradientSkyAppearance::LocalShaderPrimitive::getVertexBufferSortKey () const
{
	return m_vertexBuffer->getSortKey ();
}

//-------------------------------------------------------------------

const StaticShader &GradientSkyAppearance::LocalShaderPrimitive::prepareToView () const
{
	return m_shader->prepareToView ();
}

//-------------------------------------------------------------------

void GradientSkyAppearance::LocalShaderPrimitive::prepareToDraw () const
{
	Transform t;
	t.setPosition_p (ShaderPrimitiveSorter::getCurrentCameraPosition ());

	Graphics::setObjectToWorldTransformAndScale (t, Vector::xyz111);
	Graphics::setVertexBuffer (*m_vertexBuffer);
	Graphics::setIndexBuffer (*m_indexBuffer);
}

//-------------------------------------------------------------------

void GradientSkyAppearance::LocalShaderPrimitive::draw () const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("GradientSkyAppearance::LocalShaderPrimitive::draw");

	if (Graphics::getShaderCapability () >= ShaderCapability (1,1))
		Graphics::setVertexShaderUserConstants(0, m_time);

	Graphics::drawIndexedTriangleList ();
}

//===================================================================
// STATIC PUBLIC GradientSkyAppearance
//===================================================================

void GradientSkyAppearance::install ()
{
#ifdef _DEBUG
	DebugFlags::registerFlag (s_noRenderSky, "ClientTerrain", "noRenderSky");
	ExitChain::add (remove, "GradientSkyAppearanceNamespace::remove");
#endif
}

//-------------------------------------------------------------------

void GradientSkyAppearanceNamespace::remove ()
{
#ifdef _DEBUG
	DebugFlags::unregisterFlag (s_noRenderSky);
#endif
}

//===================================================================
// PUBLIC GradientSkyAppearance
//===================================================================

GradientSkyAppearance::GradientSkyAppearance (const char* shaderTemplateName) :
	Appearance (0),
	m_localShaderPrimitive (0),
	m_dpvsObject(0)
{
	m_dpvsObject = RenderWorld::createUnboundedObject (this);

	if (Graphics::getShaderCapability () >= ShaderCapability (1,1))
		m_localShaderPrimitive = new LocalShaderPrimitive (shaderTemplateName);
}

//-------------------------------------------------------------------

GradientSkyAppearance::~GradientSkyAppearance ()
{
	delete m_localShaderPrimitive;
	m_dpvsObject->release();
}
	
//-------------------------------------------------------------------

const char* GradientSkyAppearance::getName () const
{
	return m_localShaderPrimitive->getName ();
}

//-------------------------------------------------------------------

DPVS::Object* GradientSkyAppearance::getDpvsObject() const
{
	return m_dpvsObject;
}

//-------------------------------------------------------------------

void GradientSkyAppearance::render () const
{
#ifdef _DEBUG
	if (s_noRenderSky)
		return;
#endif

	if (m_localShaderPrimitive)
		ShaderPrimitiveSorter::add (*m_localShaderPrimitive);
}

//-------------------------------------------------------------------

float GradientSkyAppearance::alter (float elapsedTime)
{
	if (m_localShaderPrimitive)
		return m_localShaderPrimitive->alter (elapsedTime);

	return AlterResult::cms_keepNoAlter;
}

//-----------------------------------------------------------------

void GradientSkyAppearance::setAlpha (bool const /* opaqueEnabled */, float const /* opaqueAlpha */, bool const /* alphaEnabled */, float const alphaAlpha)
{
	if (m_localShaderPrimitive)
		m_localShaderPrimitive->setAlpha (alphaAlpha);
}

//-----------------------------------------------------------------

void GradientSkyAppearance::setTime (const float time)
{
	if (m_localShaderPrimitive)
		m_localShaderPrimitive->setTime (time);
}

//===================================================================

