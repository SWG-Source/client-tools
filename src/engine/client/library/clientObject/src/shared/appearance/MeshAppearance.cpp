// ======================================================================
//
// MeshAppearance.cpp
// Portions Copyright 1999 Bootprint Entertainment
// Portions Copyright 2000-2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/MeshAppearance.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitiveSet.h"
#include "clientGraphics/ShaderPrimitiveSetTemplate.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/StaticShaderTemplate.h"
#include "clientGraphics/Texture.h"
#include "clientObject/MeshAppearanceTemplate.h"
#include "clientObject/ShadowBlobManager.h"
#include "clientObject/ShadowManager.h"
#include "clientObject/ReticleManager.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/Extent.h"
#include "sharedCollision/ExtentList.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedMath/DebugShapeRenderer.h"
#include "sharedMath/IndexedTriangleList.h"
#include "sharedMath/Plane.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/CustomizationData.h"

#include "dpvsObject.hpp"
#include "dpvsModel.hpp"

#include <stdio.h>
#include <string>
#include <vector>

// ======================================================================

namespace MeshAppearanceNamespace
{
	bool ms_fadeInEnabled = true;
}

using namespace MeshAppearanceNamespace;

// ======================================================================

void MeshAppearance::install ()
{
}

// ----------------------------------------------------------------------

void MeshAppearance::setCollideAgainstAllGeometry(bool const collideAgainstAllGeometry)
{
	ShaderPrimitiveSetTemplate::setCollideAgainstAllGeometry(collideAgainstAllGeometry);
}

// ----------------------------------------------------------------------

void MeshAppearance::setFadeInEnabled(bool enabled)
{
	ms_fadeInEnabled = enabled;
}

// ======================================================================

const MeshAppearanceTemplate *MeshAppearance::getMeshAppearanceTemplate() const
{
	return safe_cast<const MeshAppearanceTemplate *>(getAppearanceTemplate());
}

// ----------------------------------------------------------------------

MeshAppearance::MeshAppearance(const MeshAppearanceTemplate *meshAppearanceTemplate)
:	Appearance(NON_NULL(meshAppearanceTemplate)),
	m_shaderPrimitiveSet(NULL),
	m_dpvsObject(NULL),
	m_needsSetTexture(false),
	m_obeyOpaqueAlpha(false),
	m_obeyAlphaAlpha(false),
	m_setTextureTag(),
	m_setTextureTexture(0),
	m_opaqueAlpha(1.0f),
	m_alphaAlpha(1.0f),
	m_customizationData(0),
	m_lastCouldRenderShadow(false),
	m_lastCouldRenderReticles(false)
{
	{
		DPVS::Model *const testModel = RenderWorld::fetchDefaultModel();
		m_dpvsObject = RenderWorld::createObject(this, testModel);
		testModel->release();
	}
	create();

	meshAppearanceTemplate->meshAppearanceCreated();
}

// ----------------------------------------------------------------------

MeshAppearance::~MeshAppearance()
{
	if (m_customizationData)
	{
		m_customizationData->release();
		m_customizationData = NULL;
	}

	if (!m_shaderPrimitiveSet)
		getMeshAppearanceTemplate()->removeAsynchronouslyLoadedAppearance(this);

	m_dpvsObject->release();
	m_dpvsObject = NULL;

	delete m_shaderPrimitiveSet;
	m_shaderPrimitiveSet = NULL;

	//-- if m_needsSetTexture is true, we were never able to set the texture, so release it
	if (m_needsSetTexture && m_setTextureTexture)
	{
		m_setTextureTexture->release();
		m_setTextureTexture = 0;
	}

	const_cast<MeshAppearanceTemplate *>(getMeshAppearanceTemplate())->meshAppearanceDestroyed();
}

// ----------------------------------------------------------------------

bool MeshAppearance::isLoaded() const
{
	return m_shaderPrimitiveSet != NULL;
}

// ----------------------------------------------------------------------

void MeshAppearance::create()
{
	const MeshAppearanceTemplate *meshAppearanceTemplate = getMeshAppearanceTemplate();

	if (meshAppearanceTemplate->isLoaded())
	{
		m_shaderPrimitiveSet = meshAppearanceTemplate->getShaderPrimitiveSetTemplate()->createShaderPrimitiveSet(*this);
		m_dpvsObject->setTestModel(meshAppearanceTemplate->getDpvsTestShape());
		setExtent(ExtentList::fetch(meshAppearanceTemplate->getExtent()));

		m_shaderPrimitiveSet->setCustomizationData(m_customizationData);
	}
}

// ----------------------------------------------------------------------

float MeshAppearance::alter(float time)
{
	float alterResult;

	if (m_shaderPrimitiveSet)
	{
		alterResult = m_shaderPrimitiveSet->alter(time);

		if (m_setTextureTexture && m_needsSetTexture)
		{
			m_needsSetTexture = false;
			m_shaderPrimitiveSet->setTexture(m_setTextureTag, *m_setTextureTexture);
			m_setTextureTexture->release();
			m_setTextureTexture = 0;
		}
	}
	else
	{
		//-- if the appearance doesn't have a shader primitive set, that means it is being
		//   asynchronously loaded.  in this case, we need to return alter next frame until
		//   the object has finished loading
		alterResult = AlterResult::cms_alterNextFrame;
	}

	// alpha fade in a mesh if it's the root appearance on an object
	const Object *owner = getOwner();
	if (m_obeyOpaqueAlpha && m_obeyAlphaAlpha && owner && owner->getAppearance() == this)
	{
		if (isLoaded())
		{
			m_opaqueAlpha += time;
			m_alphaAlpha += time;
			if (!ms_fadeInEnabled || m_opaqueAlpha >= 1.0f)
			{
				m_obeyOpaqueAlpha = false;
				m_obeyAlphaAlpha = false;
				m_opaqueAlpha = 0.0f;
				m_alphaAlpha = 0.0f;
			}
		}

		alterResult = AlterResult::cms_alterNextFrame;
	}

	return alterResult;
}

// ----------------------------------------------------------------------

DPVS::Object *MeshAppearance::getDpvsObject() const
{
	return m_dpvsObject;
}

// ----------------------------------------------------------------------

void MeshAppearance::addToWorld()
{
	// check if we should alpha fade this object in
	m_obeyOpaqueAlpha = ms_fadeInEnabled;
	m_obeyAlphaAlpha = ms_fadeInEnabled;
	m_opaqueAlpha = 0.0f;
	m_alphaAlpha = 0.0f;

	Appearance::addToWorld();
}

// ----------------------------------------------------------------------

void MeshAppearance::removeFromWorld()
{
	Appearance::removeFromWorld();
	m_obeyOpaqueAlpha = false;
	m_obeyAlphaAlpha = false;
	m_opaqueAlpha = 0.0f;
	m_alphaAlpha = 0.0f;
}

// ----------------------------------------------------------------------

void MeshAppearance::setAlpha(bool const opaqueEnabled, float const opaqueAlpha, bool const alphaEnabled, float const alphaAlpha)
{
	m_obeyOpaqueAlpha = opaqueEnabled;
	m_obeyAlphaAlpha = alphaEnabled;
	m_opaqueAlpha = opaqueAlpha;
	m_alphaAlpha = alphaAlpha;
}

// ----------------------------------------------------------------------

void MeshAppearance::render() const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("MeshAppearance::render");

	if (m_shaderPrimitiveSet)
	{
		if (m_obeyOpaqueAlpha || m_obeyAlphaAlpha)
			m_shaderPrimitiveSet->addWithAlphaFadeOpacity(m_obeyOpaqueAlpha, m_opaqueAlpha, m_obeyAlphaAlpha, m_alphaAlpha);
		else
		{
			m_shaderPrimitiveSet->add();

			//-- queue shadow
			if (m_shaderPrimitiveSet->getNumberOfShaders())
			{
				const Object *owner=getOwner();
				if (owner)
				{
					getMeshAppearanceTemplate()->renderVolumetricShadow(this, m_shaderPrimitiveSet->getShader(0)->usesVertexShader());
				}
			}
		}

		if (getShadowBlobAllowed())
		{
			bool const couldRenderShadow = ShadowManager::getEnabled() && ShadowManager::getAllowed() && ShadowManager::getSkeletalShadowsSimple();
			bool const couldRenderReticle = ReticleManager::getEnabled();

			if (couldRenderShadow)
			{
				if (m_opaqueAlpha == 0.f)
					ShadowBlobManager::renderShadowBlobs(*this);

				m_lastCouldRenderShadow = true;
			}

			if(couldRenderReticle)
			{
				if(m_opaqueAlpha == 0.f)
					ReticleManager::renderReticles(*this);

				m_lastCouldRenderReticles = true;
			}

			if(!couldRenderShadow && !couldRenderReticle && (m_lastCouldRenderReticles || m_lastCouldRenderShadow))
			{				
				getMeshAppearanceTemplate()->clearUpwardIndices();
			}
			m_lastCouldRenderShadow = couldRenderShadow;
			m_lastCouldRenderReticles = couldRenderReticle;
		}
	}

	Appearance::render();
}

// ----------------------------------------------------------------------

const Sphere &MeshAppearance::getSphere() const
{
	return getMeshAppearanceTemplate()->getSphere();
}

// ----------------------------------------------------------------------

void MeshAppearance::setCustomizationData(CustomizationData *customizationData)
{
	//-- Hang on to customization data.
	if (customizationData)
		customizationData->fetch();

	if (m_customizationData)
		m_customizationData->release();

	m_customizationData = customizationData;

	if (m_shaderPrimitiveSet)
		m_shaderPrimitiveSet->setCustomizationData(customizationData);
}

// ----------------------------------------------------------------------

void MeshAppearance::addCustomizationVariables(CustomizationData &customizationData) const
{
	if (m_shaderPrimitiveSet)
		m_shaderPrimitiveSet->addCustomizationVariables(customizationData);
}

// ----------------------------------------------------------------------

bool MeshAppearance::collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const
{
	//-- Collide w/ extent only
	if (collideParameters.getQuality() == CollideParameters::Q_low)
	{
		Extent const * const extent = getExtent();
		if (extent)
		{
			Vector normal;
			float time;
			if (extent->intersect(start_o, end_o, &normal, &time))
			{
				result.setPoint(Vector::linearInterpolate(start_o, end_o, time));
				result.setNormal(normal);
				result.setTime(time);

				return true;
			}
		}

		return false;
	}

	//-- All other collide qualities do collision with actual mesh
	if (m_shaderPrimitiveSet && getExtent() && getExtent()->intersect(start_o, end_o))
		return m_shaderPrimitiveSet->collide(start_o, end_o, collideParameters, result);

	return false;
}

// ----------------------------------------------------------------------

bool MeshAppearance::implementsCollide() const
{
	return true;
}

// ----------------------------------------------------------------------

void MeshAppearance::setTexture(const Tag tag, const Texture &texture)
{
	m_needsSetTexture = true;
	m_setTextureTag = tag;
	m_setTextureTexture = &texture;
	m_setTextureTexture->fetch();
}

// ----------------------------------------------------------------------

void MeshAppearance::renderShadowBlob(Vector const & position_o, float const radius) const
{
	getMeshAppearanceTemplate()->renderSimpleShadow(position_o, radius);
}

// ----------------------------------------------------------------------

void MeshAppearance::renderReticle(Vector const & position_o, float const radius) const
{
	getMeshAppearanceTemplate()->renderReticle(position_o, radius);
}

// ----------------------------------------------------------------------

AxialBox const MeshAppearance::getTangibleExtent() const
{
	if (m_shaderPrimitiveSet)
	{
		Extent const * const extent = getExtent();
		if (extent)
			return extent->getBoundingBox();
	}

	return AxialBox();
}

// ======================================================================

void MeshAppearance::drawDebugShapes (DebugShapeRenderer* renderer) const
{
	if (!renderer)
		return;

#ifdef _DEBUG
	Appearance::drawDebugShapes (renderer);
#endif
}

// ======================================================================

#ifdef _DEBUG

int MeshAppearance::getPolygonCount () const
{
	int total = 0;

	for (int i = 0; i < m_shaderPrimitiveSet->getNumberOfShaders(); ++i)
		total += m_shaderPrimitiveSet->getIndexBuffer (i)->getNumberOfIndices () / 3;

	return total;
}

static void indent (std::string &result, const int indentLevel)
{
	int i;
	for (i = 0; i < indentLevel; ++i)
		result += "  ";
}

void MeshAppearance::debugDump(std::string &result, const int indentLevel) const
{
	char buffer [1024];

	sprintf (buffer, "MeshAppearance %s\r\n", getAppearanceTemplate ()->getName ());
	indent (result, indentLevel);
	result += buffer;

	sprintf (buffer, "%i shader primitives\r\n", m_shaderPrimitiveSet->getNumberOfShaders ());
	indent (result, indentLevel);
	result += buffer;

	int i;
	for (i = 0; i < m_shaderPrimitiveSet->getNumberOfShaders(); ++i)
	{
		const ShaderTemplate* const shaderTemplate = m_shaderPrimitiveSet->getShaderTemplate(i);

		sprintf (buffer, "shader %s\r\n", shaderTemplate->getName ().getString ());
		indent (result, indentLevel + 1);
		result += buffer;

		const StaticShaderTemplate* const staticShaderTemplate = dynamic_cast<const StaticShaderTemplate *>(shaderTemplate);

		sprintf (buffer, "isStatic=%s\r\n", staticShaderTemplate ? "yes" : "no");
		indent (result, indentLevel + 2);
		result += buffer;

		sprintf (buffer, "isOpaque=%s\r\n", shaderTemplate->isOpaqueSolid () ? "yes" : "no");
		indent (result, indentLevel + 2);
		result += buffer;

		sprintf (buffer, "isCollidable=%s\r\n", shaderTemplate->isCollidable () ? "yes" : "no");
		indent (result, indentLevel + 2);
		result += buffer;

		if (staticShaderTemplate)
		{
			sprintf (buffer, "numberOfPasses=%i\r\n", static_cast<const StaticShader*> (m_shaderPrimitiveSet->getShader (i))->getNumberOfPasses ());
			indent (result, indentLevel + 2);
			result += buffer;
		}

		const Shader* const shader = m_shaderPrimitiveSet->getShader (i);
		const StaticShader* const staticShader = shader->getStaticShader ();
		if (staticShader)
		{
			typedef std::vector<Tag> TagList;
			TagList tagList;

			staticShader->getTextureTags (tagList);
			TagList::const_iterator end = tagList.end ();
			for (TagList::const_iterator iter = tagList.begin (); iter != end; ++iter)
			{
				const Texture* texture = 0;
				if (staticShader->getTexture (*iter, texture) && texture)
				{
					sprintf (buffer, "texture=%s  %i x %i\r\n", texture->getName (), texture->getWidth (), texture->getHeight ());
					indent (result, indentLevel + 2);
					result += buffer;
				}
			}
		}

		const StaticVertexBuffer* const vertexBuffer = m_shaderPrimitiveSet->getVertexBuffer(i);
		const StaticIndexBuffer* const indexBuffer  = m_shaderPrimitiveSet->getIndexBuffer(i);

		sprintf (buffer, "%i vertices, %i triangles\r\n", vertexBuffer->getNumberOfVertices (), indexBuffer->getNumberOfIndices () / 3);
		indent (result, indentLevel + 2);
		result += buffer;
	}
}

#endif

// ======================================================================

