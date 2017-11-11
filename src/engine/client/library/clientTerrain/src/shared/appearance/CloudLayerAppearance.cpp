//===================================================================
//
// CloudLayerAppearance.cpp
// asommers
//
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
//===================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/CloudLayerAppearance.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/GraphicsOptionTags.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticIndexBuffer.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/StaticVertexBuffer.h"
#include "clientTerrain/ConfigClientTerrain.h"
#include "clientTerrain/WeatherManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Object.h"
#include "sharedUtility/FileName.h"

#include "dpvsObject.hpp"

//===================================================================

namespace
{
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const Tag TAG_MAIN = TAG (M,A,I,N);
	const Tag TAG_DOT3 = TAG (D,O,T,3);

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool s_enableCloudLightScaling = false;

#ifdef _DEBUG
	bool s_noRenderClouds = false;
	bool s_darkenCloudsAtNight = false;
#endif

	bool ms_day;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

//===================================================================
// CloudLayerAppearance::LocalShaderPrimitive
//===================================================================

class CloudLayerAppearance::LocalShaderPrimitive : public ShaderPrimitive
{
public:

	static VertexBufferFormat   getVertexBufferFormat();
	static int                  getSubdivisions();

public:

	LocalShaderPrimitive(const char* shaderTemplateName, float shaderSize, float velocity);
	virtual ~LocalShaderPrimitive();

	const char* getName () const;

	virtual float               alter(float time);
	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader &prepareToView() const;
	virtual void                prepareToDraw() const;
	virtual void                draw() const;

	void setAlpha (float alpha);

private:

	StaticVertexBuffer          m_vertexBuffer;
	StaticIndexBuffer           m_indexBuffer;
	Shader*                     m_shader;
	const float                 m_velocity;
	Vector                      m_lastWindVelocity_w;

private:
	
	LocalShaderPrimitive();
	LocalShaderPrimitive(const LocalShaderPrimitive &);              //lint -esym(754, LocalShaderPrimitive::LocalShaderPrimitive)  // not referenced
	LocalShaderPrimitive &operator =(const LocalShaderPrimitive &);  //lint -esym(754, LocalShaderPrimitive::operator=)  // not referenced
};

//===================================================================
// STATIC PUBLIC CloudLayerAppearance::LocalShaderPrimitive
//===================================================================

VertexBufferFormat CloudLayerAppearance::LocalShaderPrimitive::getVertexBufferFormat()
{
	VertexBufferFormat format;
	format.setPosition ();
	format.setNormal ();
	format.setColor0 ();
	
	if (GraphicsOptionTags::get (TAG_DOT3))
	{
		format.setNumberOfTextureCoordinateSets (2);
		format.setTextureCoordinateSetDimension (0, 2);
		format.setTextureCoordinateSetDimension (1, 4);
	}
	else
	{
		format.setNumberOfTextureCoordinateSets (1);
		format.setTextureCoordinateSetDimension (0, 2);
	}

	return format;
}

//-------------------------------------------------------------------

int CloudLayerAppearance::LocalShaderPrimitive::getSubdivisions()
{
	return 10;
}

//===================================================================
// PUBLIC CloudLayerAppearance::LocalShaderPrimitive
//===================================================================

CloudLayerAppearance::LocalShaderPrimitive::LocalShaderPrimitive(const char* const shaderTemplateName, const float shaderSize, const float velocity) : 
	ShaderPrimitive(),
	m_vertexBuffer(getVertexBufferFormat(), getSubdivisions() * getSubdivisions()),
	m_indexBuffer((getSubdivisions() - 1) * (getSubdivisions() - 1) * 6),
	m_shader (ShaderTemplateList::fetchModifiableShader (FileName (FileName::P_shader, shaderTemplateName))),
	m_velocity (velocity),
	m_lastWindVelocity_w ()
{
	//-- 
	const int    subdivisions = getSubdivisions();
	const float  distanceBetweenPoles = 50.f;
	const Vector start (-subdivisions * 0.5f * distanceBetweenPoles, 10.f, -subdivisions * 0.5f * distanceBetweenPoles);

	const float ooShaderSize = shaderSize != 0.f ? (1.0f / shaderSize) : 1.f;

	const bool  dot3 = GraphicsOptionTags::get (TAG_DOT3);

	//-- construct vertexbuffer
	m_vertexBuffer.lock ();
	{
		int x;
		int z;
		VertexBufferWriteIterator v = m_vertexBuffer.begin();
		for (z = 0; z < subdivisions; z++)
		{
			for (x = 0; x < subdivisions; x++)
			{
				//-- create position
				const float x0 = start.x + (x * distanceBetweenPoles);
				const float y0 = start.y;  //lint !e578  // y0 hides y0 (double)
				const float z0 = start.z + (z * distanceBetweenPoles);

				//-- set that data in the height buffer
				const VectorArgb color (1.0, 1.0, 1.0, (x == 0 || x == subdivisions - 1 || z == 0 || z == subdivisions - 1) ? 0.f : 1.f);

				v.setPosition (Vector (x0, y0, z0));
				v.setNormal (Vector::unitY);
				v.setColor0 (color);
				v.setTextureCoordinates (0, x0 * ooShaderSize, z0 * ooShaderSize);

				if (dot3)
					v.setTextureCoordinates (1, 1.f, 0.f, 0.f, 1.f);

				++v;
			}
		}
	}
	m_vertexBuffer.unlock ();

	//-- construct index buffer
	m_indexBuffer.lock ();
	{
		Index *index = m_indexBuffer.begin();

		int x;
		int z;
		for (z = 0; z < subdivisions - 1; z++)
			for (x = 0; x < subdivisions - 1; x++)
			{
				const int vertices [] = 
				{ 
					(z + 1) * subdivisions + x, 
					(z + 1) * subdivisions + x + 1,
					(z    ) * subdivisions + x + 1,
					(z    ) * subdivisions + x
				};

				const int indices []  = { 0, 2, 1, 0, 3, 2 };

				int j;
				for (j = 0; j < 6; ++j, ++index)
					*index = static_cast<Index> (vertices [indices [j]]);
			}
	}
	m_indexBuffer.unlock ();

	DEBUG_WARNING (!m_shader->getStaticShader ()->hasTextureFactor (TAG_MAIN), ("CloudLayerAppearance: %s is missing texture factor tag MAIN.  Please open and save this shader template in the ShaderBuilder.", shaderTemplateName));
	DEBUG_WARNING (!m_shader->getStaticShader ()->hasTextureScroll (TAG_MAIN), ("CloudLayerAppearance: %s is missing texture scroll tag MAIN.  Please open and save this shader template in the ShaderBuilder.", shaderTemplateName));
	setAlpha (0.f);
}

//-------------------------------------------------------------------

CloudLayerAppearance::LocalShaderPrimitive::~LocalShaderPrimitive()
{
	m_shader->release();
	m_shader = 0;
}

//-------------------------------------------------------------------

const char* CloudLayerAppearance::LocalShaderPrimitive::getName () const
{
	return m_shader->getName ();
}

//-------------------------------------------------------------------

float CloudLayerAppearance::LocalShaderPrimitive::alter(float time)
{
	// @todo if this can flip after the first alter, this function needs to
	//       return AlterResult::cms_alterNextFrame instead of whatever the
	//       shader returns.
	m_shader->getStaticShader ()->setObeysLightScale (s_enableCloudLightScaling);

	if (WeatherManager::getScaledWindVelocity_w () != m_lastWindVelocity_w && m_shader->getStaticShader ()->hasTextureScroll (TAG_MAIN))
	{
		m_lastWindVelocity_w = WeatherManager::getScaledWindVelocity_w ();

		bool const hasDirection = m_lastWindVelocity_w.magnitude () > Vector::NORMALIZE_THRESHOLD;
		if (hasDirection)
		{
			//-- need to negate wind because we're looking at it from the bottom
			Vector normalizedWindVelocity_w = -m_lastWindVelocity_w;
			normalizedWindVelocity_w.normalize ();

			StaticShaderTemplate::TextureScroll textureScroll;
			textureScroll.u1 = normalizedWindVelocity_w.x * m_velocity;
			textureScroll.v1 = normalizedWindVelocity_w.z * m_velocity;
			textureScroll.u2 = 0.f;
			textureScroll.v2 = 0.f;
			m_shader->getStaticShader ()->setTextureScroll (TAG_MAIN, textureScroll);
		}
	}

	return m_shader->alter(time);
}

// ----------------------------------------------------------------------

const Vector CloudLayerAppearance::LocalShaderPrimitive::getPosition_w() const
{
	return Vector::zero;
}
	
//-------------------------------------------------------------------

float CloudLayerAppearance::LocalShaderPrimitive::getDepthSquaredSortKey() const
{
	return 0.0f;
}

// ----------------------------------------------------------------------

int CloudLayerAppearance::LocalShaderPrimitive::getVertexBufferSortKey() const
{
	return m_vertexBuffer.getSortKey();
}

//-------------------------------------------------------------------

const StaticShader &CloudLayerAppearance::LocalShaderPrimitive::prepareToView() const
{ 
	return m_shader->prepareToView();
}

//-------------------------------------------------------------------

void CloudLayerAppearance::LocalShaderPrimitive::prepareToDraw() const
{
	Transform t;
	t.setPosition_p (ShaderPrimitiveSorter::getCurrentCameraPosition());

	Graphics::setObjectToWorldTransformAndScale (t, Vector::xyz111);
	Graphics::setVertexBuffer (m_vertexBuffer);
	Graphics::setIndexBuffer (m_indexBuffer);
}

//-------------------------------------------------------------------

void CloudLayerAppearance::LocalShaderPrimitive::draw() const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("CloudLayerAppearance::LocalShaderPrimitive::draw");
	
	Graphics::drawIndexedTriangleList ();
}

//-------------------------------------------------------------------

void CloudLayerAppearance::LocalShaderPrimitive::setAlpha (const float alpha)
{
	PackedArgb color (ms_day ? PackedArgb::solidWhite : PackedArgb::solidGray);
	color.setA (static_cast<uint8> (alpha * 255));

	if (m_shader->getStaticShader ()->hasTextureFactor (TAG_MAIN))
		m_shader->getStaticShader ()->setTextureFactor (TAG_MAIN, color.getArgb ());
}

//===================================================================
// STATIC PUBLIC CloudLayerAppearance
//===================================================================

void CloudLayerAppearance::install ()
{
#ifdef _DEBUG
	DebugFlags::registerFlag (s_noRenderClouds, "ClientTerrain", "noRenderClouds");
	DebugFlags::registerFlag (s_enableCloudLightScaling, "ClientTerrain", "enableCloudLightScaling");
	DebugFlags::registerFlag (s_darkenCloudsAtNight, "ClientTerrain", "darkenCloudsAtNight");
	ExitChain::add (remove, "CloudLayerAppearance::remove");
#endif
}

//-------------------------------------------------------------------

void CloudLayerAppearance::remove ()
{
#ifdef _DEBUG
	DebugFlags::unregisterFlag (s_noRenderClouds);
	DebugFlags::unregisterFlag (s_enableCloudLightScaling);
	DebugFlags::unregisterFlag (s_darkenCloudsAtNight);
#endif
}

// ----------------------------------------------------------------------

void CloudLayerAppearance::setDay (bool const day)
{
	ms_day = day;
}

//===================================================================
// PUBLIC CloudLayerAppearance::LocalShaderPrimitive
//===================================================================

CloudLayerAppearance::CloudLayerAppearance (const char* shaderTemplateName, float shaderSize, float velocity) :
	Appearance (0),
	m_localShaderPrimitive (new LocalShaderPrimitive (shaderTemplateName, shaderSize, velocity)),
	m_dpvsObject(NULL)
{
	m_dpvsObject = RenderWorld::createUnboundedObject(this);
}

//-------------------------------------------------------------------

CloudLayerAppearance::~CloudLayerAppearance ()
{
	delete m_localShaderPrimitive;
	m_dpvsObject->release();
}
	
//-------------------------------------------------------------------

const char* CloudLayerAppearance::getName () const
{
	return m_localShaderPrimitive->getName ();
}

//-------------------------------------------------------------------

DPVS::Object* CloudLayerAppearance::getDpvsObject() const
{
	return m_dpvsObject;
}

//-------------------------------------------------------------------

void CloudLayerAppearance::render () const
{
#ifdef _DEBUG
	if (s_noRenderClouds)
		return;
#endif

	if (ConfigClientTerrain::getDisableClouds ())
		return;

	ShaderPrimitiveSorter::add(*m_localShaderPrimitive);
}

//-------------------------------------------------------------------

float CloudLayerAppearance::alter (const float time)
{
	return m_localShaderPrimitive->alter (time);
}

//-------------------------------------------------------------------

void CloudLayerAppearance::setAlpha (bool const /* opaqueEnabled */ , float const /* opaqueAlpha */ , bool const /* alphaEnabled */, float const alphaAlpha)
{
	m_localShaderPrimitive->setAlpha (alphaAlpha);
}

//===================================================================

