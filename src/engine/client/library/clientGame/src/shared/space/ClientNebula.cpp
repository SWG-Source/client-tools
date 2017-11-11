//======================================================================
//
// ClientNebula.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientNebula.h"

#include "clientAudio/SoundTemplateList.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/NebulaManagerClient.h"
#include "clientGame/NebulaVisualQuadShaderGroup.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientTerrain/ClientSpaceTerrainAppearance.h"
#include "clientTerrain/GroundEnvironment.h"
#include "clientTerrain/SpaceEnvironment.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/Nebula.h"
#include "sharedGame/NebulaManager.h"
#include "sharedMath/Vector.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedRandom/Random.h"
#include "sharedTerrain/TerrainObject.h"
#include <algorithm>
#include <map>
#include <vector>

//======================================================================

namespace ClientNebulaNamespace
{
	/**
	* @param vectorFromNebulaCenter must be normalized (or approximately normalized)
	*/

	void modifyColors(Vector const & vectorToNebulaCenter, float const distanceRatioFromNebulaCenter, VectorArgb const * const colors, VectorArgb * modifiedColors, SpaceEnvironment::LightVector const & parallelLights)
	{
		Vector lightDirection;

		float maxIntensity = 0.0f;

		for (SpaceEnvironment::LightVector::const_iterator it = parallelLights.begin(); it != parallelLights.end(); ++it)
		{
			Light const * const light = *it;
			if (!light)
				continue;

			if (light->getType() != Light::T_parallel)
				continue;

			VectorArgb const & lightDiffuseColor = light->getDiffuseColor();			
			float const intensity = lightDiffuseColor.rgbIntensity();

			if (intensity < maxIntensity)
				continue;

			maxIntensity = intensity;
			
			lightDirection = light->getObjectFrameK_w();
		}
		
		
		float darkFactor = clamp(0.0f, -lightDirection.dot(vectorToNebulaCenter), 1.0f);

		if (darkFactor > 0.0f)
		{
			darkFactor *= distanceRatioFromNebulaCenter;

			modifiedColors[0] = VectorArgb::linearInterpolate(colors[0], VectorArgb::solidBlack, darkFactor);
			modifiedColors[1] = VectorArgb::linearInterpolate(colors[1], VectorArgb::solidBlack, darkFactor);
			modifiedColors[2] = VectorArgb::linearInterpolate(colors[2], VectorArgb::solidBlack, darkFactor);
			modifiedColors[3] = VectorArgb::linearInterpolate(colors[3], VectorArgb::solidBlack, darkFactor);
		}
		else
		{
			modifiedColors[0] = colors[0];
			modifiedColors[1] = colors[1];
			modifiedColors[2] = colors[2];
			modifiedColors[3] = colors[3];
		}
	}

	//----------------------------------------------------------------------

	bool s_printClientNebulaPopulate = false;
	bool s_flagsInstalled = false;

}

using namespace ClientNebulaNamespace;

//----------------------------------------------------------------------

ClientNebula::ClientNebula(int const nebulaId) :
m_nebulaVisualQuadShaderGroup(NULL),
m_nebulaId(nebulaId),
m_isPopulated(false),
m_lightningAppearanceTemplate(0),
m_lightningSoundTemplate(0),
m_clientHitLightningClientEffectTemplate(0),
m_serverHitLightningClientEffectTemplate(0),
m_environmentalDamageClientEffectTemplate(0)
{
	if (!s_flagsInstalled)
	{
		DebugFlags::registerFlag(s_printClientNebulaPopulate, "ClientGame/NebulaManagerClient", "printClientNebulaPopulate");
		s_flagsInstalled = true;
	}

	//-- Preload nebula assets
	Nebula const * const nebula = NebulaManager::getNebulaById(nebulaId);
	if (nebula)
	{
		std::string const & lightningAppearanceTemplateName = nebula->getLightningAppearance();
		if (!lightningAppearanceTemplateName.empty())
		{
			if (TreeFile::exists(lightningAppearanceTemplateName.c_str()))
				m_lightningAppearanceTemplate = AppearanceTemplateList::fetch(lightningAppearanceTemplateName.c_str());
			else
				DEBUG_WARNING(true, ("ClientNebula::ClientNebula: nebula id %i specifies invalid lighting appearance template name %s", lightningAppearanceTemplateName.c_str()));
		}

		std::string const & lightningSoundTemplateName = nebula->getLightningSound();
		if (!lightningSoundTemplateName.empty())
		{
			if (TreeFile::exists(lightningSoundTemplateName.c_str()))
				m_lightningSoundTemplate = SoundTemplateList::fetch(lightningSoundTemplateName.c_str());
			else
				DEBUG_WARNING(true, ("ClientNebula::ClientNebula: nebula id %i specifies invalid lightning sound template name %s", lightningSoundTemplateName.c_str()));
		}

		std::string const & clientHitLightningClientEffectTemplateName = nebula->getLightningClientEffectHitClient();
		if (!clientHitLightningClientEffectTemplateName.empty())
		{
			if (TreeFile::exists(clientHitLightningClientEffectTemplateName.c_str()))
				m_clientHitLightningClientEffectTemplate = ClientEffectTemplateList::fetch(CrcLowerString(clientHitLightningClientEffectTemplateName.c_str()));
			else
				DEBUG_WARNING(true, ("ClientNebula::ClientNebula: nebula id %i specifies invalid client hit lighting client effect template name %s", clientHitLightningClientEffectTemplateName.c_str()));
		}

		std::string const & serverHitLightningClientEffectTemplateName = nebula->getLightningClientEffectHitServer();
		if (!serverHitLightningClientEffectTemplateName.empty())
		{
			if (TreeFile::exists(serverHitLightningClientEffectTemplateName.c_str()))
				m_serverHitLightningClientEffectTemplate = ClientEffectTemplateList::fetch(CrcLowerString(serverHitLightningClientEffectTemplateName.c_str()));
			else
				DEBUG_WARNING(true, ("ClientNebula::ClientNebula: nebula id %i specifies invalid server hit lighting client effect template name %s", serverHitLightningClientEffectTemplateName.c_str()));
		}

		std::string const & environmentalDamageClientEffectTemplateName = nebula->getEnvironmentalDamageClientEffect();
		if (!environmentalDamageClientEffectTemplateName.empty())
		{
			if (TreeFile::exists(environmentalDamageClientEffectTemplateName.c_str()))
				m_environmentalDamageClientEffectTemplate = ClientEffectTemplateList::fetch(CrcLowerString(environmentalDamageClientEffectTemplateName.c_str()));
			else
				DEBUG_WARNING(true, ("ClientNebula::ClientNebula: nebula id %i specifies invalid environmental damage client effect template name %s", environmentalDamageClientEffectTemplateName.c_str()));
		}
	}
	else 
		DEBUG_WARNING(true, ("ClientNebula::ClientNebula: nebula id %i does not exist", nebulaId));
}

//----------------------------------------------------------------------

ClientNebula::~ClientNebula()
{
	clear();
	delete m_nebulaVisualQuadShaderGroup;
	m_nebulaVisualQuadShaderGroup = NULL;

	if (m_lightningAppearanceTemplate)
	{
		AppearanceTemplateList::release(m_lightningAppearanceTemplate);
		m_lightningAppearanceTemplate = 0;
	}

	if (m_lightningSoundTemplate)
	{
		SoundTemplateList::release(m_lightningSoundTemplate);
		m_lightningSoundTemplate = 0;
	}

	if (m_clientHitLightningClientEffectTemplate)
	{
		m_clientHitLightningClientEffectTemplate->release();
		m_clientHitLightningClientEffectTemplate = 0;
	}

	if (m_serverHitLightningClientEffectTemplate)
	{
		m_serverHitLightningClientEffectTemplate->release();
		m_serverHitLightningClientEffectTemplate = 0;
	}

	if (m_environmentalDamageClientEffectTemplate)
	{
		m_environmentalDamageClientEffectTemplate->release();
		m_environmentalDamageClientEffectTemplate = 0;
	}
}

//----------------------------------------------------------------------

void ClientNebula::setPopulated(bool populated)
{
	m_isPopulated = populated;
}

//----------------------------------------------------------------------

void ClientNebula::populate()
{
	if (isPopulated())
		return;

	TerrainObject const * const terrainObject = TerrainObject::getInstance();
	if (terrainObject == NULL)
		return;

	SpaceEnvironment::LightVector parallelLights;
	
	{
		ClientSpaceTerrainAppearance const * const clientSpaceTerrainAppearance = dynamic_cast<ClientSpaceTerrainAppearance const *>(terrainObject->getAppearance());	
		
		if (clientSpaceTerrainAppearance)
			parallelLights = clientSpaceTerrainAppearance->getSpaceEnvironment().getParallelLights();
		else
			parallelLights.push_back(&GroundEnvironment::getInstance().getMainLight());
	}


	UNREF(parallelLights);

	setPopulated(true);

	Nebula const * const nebula = NebulaManager::getNebulaById(getId());
	if (nebula == NULL)
	{
		WARNING(true, ("ClientNebula::populate with no nebula [%d]", getId()));
		return;
	}

	if (nebula->getDensity() <= 0.0f)
		return;

	int const randomSeedOffset = NebulaManagerClient::Config::getRandomSeedOffset();

	if (randomSeedOffset >= 0)
		Random::setSeed(static_cast<uint32>(randomSeedOffset + nebula->getId()));

	Sphere const & sphere = nebula->getSphere();
	float const density = nebula->getDensity();
	float const nebulaQuadDensity = nebula->getDensity() * CuiPreferences::getGlobalNebulaDensity();
	float const radius = sphere.getRadius();
	int const numQuads = clamp(1, static_cast<int>((density) * nebulaQuadDensity), 100);
	Vector const & center = sphere.getCenter();
	int const shaderIndex = nebula->getShaderIndex();

	Shader const * const shader = 
		ShaderTemplateList::fetchShader(shaderIndex == 0 ? "shader/pt_nebulae_gas_4_2.sht" : "shader/pt_nebulae_gas_4_alpha_2.sht");

	m_nebulaVisualQuadShaderGroup = new NebulaVisualQuadShaderGroup(*shader, numQuads, true);

	VectorArgb colors[4];
	VectorArgb modifiedColors[4];

	float const facingPercent = nebula->getFacingPercent() - (nebula->getFacingPercent() * NebulaManagerClient::Config::getOrientedPercent());
	float const quadGenerationRadius = std::max(0.1f, NebulaManagerClient::Config::getQuadGenerationRadius());

	for (int i = 0; i < numQuads; ++i)
	{
		int const style = Random::random(0, 3);

		float const quad_distance_factor = Random::randomReal(0.0f, quadGenerationRadius);
		float const quad_distance = radius * quad_distance_factor;
		Vector const & quad_center = center + Vector::randomUnit() * quad_distance;

		static float const s_generationDistanceSquareThreshold = 10.0f;

		if (m_nebulaVisualQuadShaderGroup->hasQuadWithinRangeSquared(quad_center, s_generationDistanceSquareThreshold))
			continue;

		float const low = (radius - quad_distance) * 0.05f;
		float const high = std::max(low + 1.0f, low * 20.0f);
		float const quad_radius = Random::randomReal(low, high);

		Vector vectorToNebulaCenter = center - quad_center;
		IGNORE_RETURN(vectorToNebulaCenter.approximateNormalize());	

		//-- camera facing quads
		if (Random::randomReal(0.0f, 1.0f) < facingPercent)
		{
			VectorArgb const & colorFacing = nebula->getColorFacing();
			VectorArgb const & colorRamp = nebula->getColorRampFacing();
			colors[0] = VectorArgb::linearInterpolate(colorFacing, colorRamp, Random::randomReal(0.0f, 1.0f));
			colors[1] = VectorArgb::linearInterpolate(colorFacing, colorRamp, Random::randomReal(0.0f, 1.0f));
			colors[2] = VectorArgb::linearInterpolate(colorFacing, colorRamp, Random::randomReal(0.0f, 1.0f));
			colors[3] = VectorArgb::linearInterpolate(colorFacing, colorRamp, Random::randomReal(0.0f, 1.0f));

			modifyColors(vectorToNebulaCenter, quad_distance_factor / quadGenerationRadius, colors, modifiedColors, parallelLights);

			NebulaVisualQuad * const nvqY = new NebulaVisualQuad(quad_center, quad_radius, modifiedColors, style, Vector::unitY, false);
			m_nebulaVisualQuadShaderGroup->addQuad(nvqY);

			NebulaVisualQuad * const nvqX = new NebulaVisualQuad(quad_center, quad_radius, modifiedColors, style, Vector::unitX, false);
			m_nebulaVisualQuadShaderGroup->addQuad(nvqX);
		}
		
		//-- world oriented quads
		else
		{
			Vector const & rotation = Vector::randomUnit() * Random::randomReal(0.0f, PI_TIMES_2);

			VectorArgb const & colorOriented = nebula->getColorOriented();
			VectorArgb const & colorRamp = nebula->getColorRampOriented();
			colors[0] = VectorArgb::linearInterpolate(colorOriented, colorRamp, Random::randomReal(0.0f, 1.0f));
			colors[1] = VectorArgb::linearInterpolate(colorOriented, colorRamp, Random::randomReal(0.0f, 1.0f));
			colors[2] = VectorArgb::linearInterpolate(colorOriented, colorRamp, Random::randomReal(0.0f, 1.0f));
			colors[3] = VectorArgb::linearInterpolate(colorOriented, colorRamp, Random::randomReal(0.0f, 1.0f));

			modifyColors(vectorToNebulaCenter, quad_distance_factor / quadGenerationRadius, colors, modifiedColors, parallelLights);	

			NebulaVisualQuad * const nvq = new NebulaVisualQuad(quad_center, quad_radius, modifiedColors, style, rotation, true);
			m_nebulaVisualQuadShaderGroup->addQuad(nvq);
		}
	}

	shader->release();

	DEBUG_REPORT_LOG(s_printClientNebulaPopulate, ("ClientNebula::populate nebula [%d] generated [%d] quads\n", nebula->getId(), numQuads));
}

//----------------------------------------------------------------------

void ClientNebula::clear()
{
	if (!isPopulated())
		return;

	delete m_nebulaVisualQuadShaderGroup;
	m_nebulaVisualQuadShaderGroup = NULL;

	setPopulated(false);
}

//======================================================================
