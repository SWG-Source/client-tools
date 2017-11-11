// ======================================================================
//
// Light.cpp
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2001, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/Light.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"

#include "sharedMath/Sphere.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/CellProperty.h"

#include "dpvsObject.hpp"
#include "dpvsModel.hpp"

// ======================================================================
// Create a new light
//
// @param newType   Type of the light.
// @param newColor  Color of the light.
// Remarks:
//
//   Some light types may need additional data setup.

Light::Light(Type newType, const VectorArgb &newColor)
: Object(),
	m_type(newType),
	m_diffuseColor(newColor),
	m_diffuseIntensity(newColor.rgbIntensity()),
	m_diffuseColorScale(1.0f),
	m_diffuseColorScaled(newColor),
	m_diffuseBackColor(VectorArgb::solidBlack),
	m_diffuseBackColorScale(1.0f),
	m_diffuseBackColorScaled(VectorArgb::solidBlack),
	m_diffuseTangentColor(VectorArgb::solidBlack),
	m_diffuseTangentColorScale(1.0f),
	m_diffuseTangentColorScaled(VectorArgb::solidBlack),
	m_diffuseIntensityScaled(newColor.rgbIntensity()),
	m_specularColor(VectorArgb::solidBlack),
	m_specularIntensity(0.0f),
	m_specularColorScale(1.0f),
	m_specularColorScaled(VectorArgb::solidBlack),
	m_specularIntensityScaled(0.0f),
	m_range(5000),
	m_constantAttenuation(1),
	m_linearAttenuation(0),
	m_quadraticAttenuation(0),
	m_theta(0),
	m_phi(0),
	m_fallOff(0),
	m_dpvsRegionOfInfluence(0),
	m_ghostLightsDirty(false),
	m_affectsShadersWithoutPrecalculatedVertexLighting(true),
	m_affectsShadersWithPrecalculatedVertexLighting(true),
	m_shaderPrimitiveSorterLightIndex(-1)
{
	RenderWorld::addObjectNotifications(*this);

	switch (m_type)
	{
		case T_ambient:
		case T_parallel:
			m_dpvsRegionOfInfluence = RenderWorld::createUnboundedRegionOfInfluence(this);
			break;

		case T_point:
			m_dpvsRegionOfInfluence = RenderWorld::createRegionOfInfluence(this, m_range);
			break;

		case T_point_multicell:
			m_dpvsRegionOfInfluence = RenderWorld::createRegionOfInfluence(this, m_range);
			m_ghostLightsDirty = true;
			scheduleForAlter();
			break;

		case T_spot:
			{
				// position the sphere around the center of the light volume.  We should probably have better approximation using a indexed triangle list
				DPVS::SphereModel *testModel = RenderWorld::fetchSphereModel(Sphere(0, 0, m_range *0.5f, m_range *0.5f));
				m_dpvsRegionOfInfluence = RenderWorld::createRegionOfInfluence(this, testModel);
				testModel->release();
			}
			break;

		default:
			DEBUG_FATAL(true, ("unknown light type"));
	};
}

// ----------------------------------------------------------------------
/**
 * Destroy a light.
 */

Light::~Light(void)
{
	if (isInWorld())
		removeFromWorld();

	for (size_t i = 0; i < m_ghostLights.size(); ++i)
		delete m_ghostLights[i];

	m_ghostLights.clear();

	m_dpvsRegionOfInfluence->release();
#ifdef _DEBUG
	m_dpvsRegionOfInfluence = NULL;
#endif
}

// ----------------------------------------------------------------------
/**
 * Set the range of the light.
 * 
 * Do not change the range of a light in the middle of a scene.
 * 
 * This value is only valid for parallelPoint, point, and spot lights.
 * 
 * @param range  New range for the light
 */

void Light::setRange(real range)
{
	if (range != m_range)
	{
		m_range = range;

		switch (m_type)
		{
			case T_ambient:
			case T_parallel:
				break;

			case T_point_multicell:
				m_ghostLightsDirty = true;
				// fall through to T_point...

			case T_point:
				{
					DPVS::SphereModel * testModel = RenderWorld::fetchSphereModel(m_range);
					m_dpvsRegionOfInfluence->setTestModel(testModel);
					testModel->release();
				}
				break;

			case T_spot:
				{
					// position the sphere around the center of the light volume.  We should probably have better approximation though.
					DPVS::SphereModel * testModel = RenderWorld::fetchSphereModel(m_range);
					m_dpvsRegionOfInfluence->setTestModel(testModel);
					testModel->release();
				}
				break;

			default:
				DEBUG_FATAL(true, ("unknown light type"));
		};
	}
}

// ----------------------------------------------------------------------

void Light::updateGhostLights()
{
	if (m_type == T_point_multicell)
	{
		size_t ghostLightCount = 0;

		if (isInWorld())
		{
			//-- find all visible cells
			RenderWorld::CellPropertyList const & cellPropertyList = RenderWorld::getVisibleCells();

			for (size_t i = 0; i < cellPropertyList.size(); ++i)
			{
				CellProperty const * const cellProperty = cellPropertyList[i];
				if (!cellProperty || cellProperty == getParentCell())
					continue;

				Light *newLight = 0;
				bool addedLight;

				if (ghostLightCount < m_ghostLights.size())
				{
					newLight = m_ghostLights[ghostLightCount];
					addedLight = false;
				}
				else
				{
					newLight = new Light(T_point, getDiffuseColor());
					m_ghostLights.push_back(newLight);
					addedLight = true;
				}

				if (m_ghostLightsDirty || addedLight)
				{
					newLight->setRange (m_range);
					newLight->setLinearAttenuation (m_linearAttenuation);
					newLight->setConstantAttenuation(m_constantAttenuation);
					newLight->setQuadraticAttenuation(m_quadraticAttenuation);

					newLight->setDiffuseColor(m_diffuseColor);
					newLight->setDiffuseColorScale(m_diffuseColorScale);
					newLight->setDiffuseBackColor(m_diffuseBackColor);
					newLight->setDiffuseBackColorScale(m_diffuseBackColorScale);
					newLight->setDiffuseTangentColor(m_diffuseTangentColor);
					newLight->setDiffuseTangentColorScale(m_diffuseTangentColorScale);

					newLight->setSpecularColor(m_specularColor);
					newLight->setSpecularColorScale(m_specularColorScale);

					newLight->setAffectsShadersWithoutPrecalculatedVertexLighting(m_affectsShadersWithoutPrecalculatedVertexLighting);
					newLight->setAffectsShadersWithPrecalculatedVertexLighting(m_affectsShadersWithPrecalculatedVertexLighting);
				}

				newLight->setPosition_w (getPosition_w());
				newLight->setParentCell(const_cast<CellProperty*>(cellProperty));

				if (!newLight->isInWorld())
					newLight->addToWorld();

				++ghostLightCount;
			}
		}

		for (size_t i = ghostLightCount; i < m_ghostLights.size(); ++i)
		{
			if (m_ghostLights[i]->isInWorld())
				m_ghostLights[i]->removeFromWorld();
		}

		m_ghostLightsDirty = false;

	}
}

// ----------------------------------------------------------------------

float Light::alter(float elapsedTime)
{
	float ret = Object::alter(elapsedTime);
	
	if (m_type == T_point_multicell)
	{
		updateGhostLights();

		if (ret != AlterResult::cms_kill)
			ret = AlterResult::cms_alterNextFrame;
	}

	return ret;
}

// ----------------------------------------------------------------------

void Light::addToWorld()
{
	addDpvsObject(m_dpvsRegionOfInfluence);
	Object::addToWorld();
}

// ----------------------------------------------------------------------

void Light::removeFromWorld()
{
	removeDpvsObject(m_dpvsRegionOfInfluence);
	Object::removeFromWorld();
}

// ----------------------------------------------------------------------
/**
 * Add the light to the ShaderPrimitiveSorter scene.
 * 
 * @param camera  Camera rendering the scene
 */

void Light::setRegionOfInfluenceEnabled(bool enabled) const
{
	if (isActive())
	{
		if (enabled)
			ShaderPrimitiveSorter::enableLight(*this);
		else
			ShaderPrimitiveSorter::disableLight(*this);
	}
}

// ======================================================================
