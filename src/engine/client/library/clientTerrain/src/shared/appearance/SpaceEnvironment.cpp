// ======================================================================
//
// SpaceEnvironment.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/SpaceEnvironment.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientTerrain/CelestialObject.h"
#include "clientTerrain/ClientSpaceTerrainAppearance.h"
#include "clientTerrain/PlanetAppearance.h"
#include "clientTerrain/SkyBox6SidedAppearance.h"
#include "clientTerrain/SkyBoxAppearance.h"
#include "clientTerrain/StarAppearance.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedTerrain/SpaceTerrainAppearanceTemplate.h"
#include "sharedTerrain/TerrainObject.h"
#include <vector>

// ======================================================================

namespace SpaceEnvironmentNamespace
{
	float ms_lastAlterTime;

	//-- the planet camera is allowed to 'move' inside a diameter of 100 meters
	float const s_cameraMovementPlanetRange = 100.0f;

	Watcher<Camera> s_camera;

	SpaceEnvironment * ms_spaceEnvironment = NULL;
}

using namespace SpaceEnvironmentNamespace;

// ======================================================================
// PUBLIC SpaceEnvironment
// ======================================================================

SpaceEnvironment::SpaceEnvironment(ClientSpaceTerrainAppearance const & clientSpaceTerrainAppearance) :
	m_clientSpaceTerrainAppearance(clientSpaceTerrainAppearance),
	m_clearColor(),
	m_lightVector(new LightVector),
	m_parallelLightVector(new LightVector),
	m_objectVector(new ObjectVector),
	m_objectsFollowCameraVector(new ObjectVector),
	m_objectsToDisableForHyperspace(new ObjectVector)
{
	FATAL(ms_spaceEnvironment, ("SpaceEnvironment already exists"));
	ms_spaceEnvironment = this;

	RenderWorld::setClearDepthAndStencilBufferAfterRenderingEnvironment(true);

	SpaceTerrainAppearanceTemplate const * const spaceTerrainAppearanceTemplate = safe_cast<SpaceTerrainAppearanceTemplate const *>(clientSpaceTerrainAppearance.getAppearanceTemplate());
	NOT_NULL(spaceTerrainAppearanceTemplate);

	//-- Clear color
	{
		PackedRgb const & clearColor = spaceTerrainAppearanceTemplate->getClearColor();
		m_clearColor = clearColor;
	}

	//-- Lights
	{
		VectorArgb const & ambientColor = spaceTerrainAppearanceTemplate->getAmbientColor();

		Light * const light = new Light(Light::T_ambient, VectorArgb::solidBlack);
		light->setDebugName("ambient light");
		light->setDiffuseColor(ambientColor);
		light->addToWorld();
		RenderWorld::addWorldEnvironmentLight(light);

		m_lightVector->push_back(light);
	}

	{
		int const numberOfParallelLights = spaceTerrainAppearanceTemplate->getNumberOfParallelLights();
		for (int i = 0; i < numberOfParallelLights; ++i)
		{
			VectorArgb const & diffuseColor = spaceTerrainAppearanceTemplate->getParallelLightDiffuseColor(i);
			VectorArgb const & specularColor = spaceTerrainAppearanceTemplate->getParallelLightSpecularColor(i);
			Vector const & direction_w = spaceTerrainAppearanceTemplate->getParallelLightDirection_w(i);

			Light * const light = new Light(Light::T_parallel, VectorArgb::solidBlack);

			char name[128];
			IGNORE_RETURN(snprintf(name, sizeof(name) - 1, "parallel light %i", i));
			light->setDebugName(name);
			light->setDiffuseColor(diffuseColor);
			light->setSpecularColor(specularColor);
			light->yaw_o(direction_w.theta());
			light->pitch_o(direction_w.phi());
			light->addToWorld();
			RenderWorld::addWorldEnvironmentLight(light);

			m_lightVector->push_back(light);
			m_parallelLightVector->push_back(light);
		}
	}

	//-- Fog
	{
		bool const fogEnabled = spaceTerrainAppearanceTemplate->getFogEnabled();
		PackedArgb const & fogColor = spaceTerrainAppearanceTemplate->getFogColor();
		float const fogDensity = spaceTerrainAppearanceTemplate->getFogDensity();

		CellProperty::getWorldCellProperty()->setFogEnabled(fogEnabled);
		CellProperty::getWorldCellProperty()->setFogColor(fogColor);
		CellProperty::getWorldCellProperty()->setFogDensity(fogDensity);
	}

	//-- Environment map texture
	{
		CrcString const & textureName = spaceTerrainAppearanceTemplate->getEnvironmentTextureName();

		Texture const * const texture = TextureList::fetch(textureName);
		CellProperty::getWorldCellProperty()->setEnvironmentTexture(texture);
		texture->release();
	}

	//-- SkyBox
	{
		bool const cubeMap = spaceTerrainAppearanceTemplate->getSkyBoxCubeMap();
		CrcString const & textureNameMask = spaceTerrainAppearanceTemplate->getSkyBoxTextureNameMask();

		if (*textureNameMask.getString())
		{
			Object * const object = new Object;
			object->setDebugName("skybox");

			if (cubeMap)
			{
				Texture const * const texture = TextureList::fetch(textureNameMask.getString());
				object->setAppearance(new SkyBoxAppearance(texture, 1.f));
				texture->release();
			}
			else
			{
				object->setAppearance(new SkyBox6SidedAppearance(textureNameMask.getString()));
			}

			RenderWorld::addWorldEnvironmentObject(object);

			m_objectVector->push_back(object);
		}
	}

	//-- StarAppearance
	{
		int const numberOfStars = spaceTerrainAppearanceTemplate->getNumberOfStars();
		if (numberOfStars > 0)
		{
			CrcString const & colorRampName = spaceTerrainAppearanceTemplate->getStarColorRampName();

			Object * const object = new Object;
			object->setDebugName("stars");
			object->setAppearance(new StarAppearance(colorRampName.getString(), numberOfStars, false));
			RenderWorld::addWorldEnvironmentObject(object);

			m_objectVector->push_back(object);
		}
	}

	//-- Celestials
	{
		int const numberOfCelestials = spaceTerrainAppearanceTemplate->getNumberOfCelestials();
		for (int i = 0; i < numberOfCelestials; ++i)
		{
			CrcString const & frontShaderTemplateName = spaceTerrainAppearanceTemplate->getCelestialFrontShaderTemplateName(i);
			float const frontSize = spaceTerrainAppearanceTemplate->getCelestialFrontSize(i);
			CrcString const & backShaderTemplateName = spaceTerrainAppearanceTemplate->getCelestialBackShaderTemplateName(i);
			float const backSize = spaceTerrainAppearanceTemplate->getCelestialBackSize(i);
			Vector const & direction_w = spaceTerrainAppearanceTemplate->getCelestialDirection_w(i);

			CelestialObject * const celestialObject = new CelestialObject(backShaderTemplateName.getString(), backSize, frontShaderTemplateName.getString(), frontSize, true);
			celestialObject->setAlpha(1.f);
			celestialObject->yaw_o(direction_w.theta());
			celestialObject->pitch_o(direction_w.phi());
			RenderWorld::addWorldEnvironmentObject(celestialObject);

			m_objectVector->push_back(celestialObject);
		}
	}

	//-- DistantAppearances
	{

		int const numberOfDistantAppearances = spaceTerrainAppearanceTemplate->getNumberOfDistantAppearances();
		for (int i = 0; i < numberOfDistantAppearances; ++i)
		{
			CrcString const & appearanceTemplateName = spaceTerrainAppearanceTemplate->getDistantAppearanceTemplateName(i);
			Vector const & direction_w = spaceTerrainAppearanceTemplate->getDistantAppearanceDirection_w(i);
			Vector const & orientation_w = spaceTerrainAppearanceTemplate->getDistantAppearanceOrientation_w(i);

			bool const isInfiniteDistance = spaceTerrainAppearanceTemplate->isDistantAppearanceInfiniteDistance(i);

			Object * const object = new Object();
			object->setAppearance(AppearanceTemplateList::createAppearance(appearanceTemplateName.getString()));
			object->setPosition_p(direction_w);
			object->yaw_o(orientation_w.x);
			object->pitch_o(orientation_w.y);
			object->roll_o(orientation_w.z);

			PlanetAppearance * const planetAppearance = dynamic_cast<PlanetAppearance *>(object->getAppearance());
			if (planetAppearance)
			{
				float const haloRoll = spaceTerrainAppearanceTemplate->getPlanetHaloRoll(i);
				if (haloRoll > 0.f)
					planetAppearance->setHaloRoll(haloRoll);

				//-- halo scaling is now handled automagically by the code at runtime

				float const planetFaceDistance = direction_w.magnitude() - object->getAppearanceSphereRadius();
				if (planetFaceDistance < s_cameraMovementPlanetRange * 0.5f)
				{
					WARNING(true, ("SpaceEnvironment planet [%s] is too close [%f] to camera.", appearanceTemplateName.getString(), planetFaceDistance));
				}

			}

			Object * const parent = new Object;
			parent->addChildObject_p(object);

			if (isInfiniteDistance)
			{
				RenderWorld::addMoveWithCameraWorldEnvironmentObject(parent);
			}
			else
			{
				RenderWorld::addWorldEnvironmentObject(parent);
				m_objectsFollowCameraVector->push_back(parent);
			}

			m_objectVector->push_back(parent);
			m_objectsToDisableForHyperspace->push_back(parent);
		}
	}
}

// ----------------------------------------------------------------------

SpaceEnvironment::~SpaceEnvironment()
{
	FATAL(!ms_spaceEnvironment, ("No SpaceEnvironment"));
	ms_spaceEnvironment = NULL;

	RenderWorld::setClearDepthAndStencilBufferAfterRenderingEnvironment(false);

	{
		for (size_t i = 0; i < m_lightVector->size(); ++i)
		{
			Light * const light = (*m_lightVector)[i];

			RenderWorld::removeWorldEnvironmentLight(light);

			if (light->isInWorld())
				light->removeFromWorld();

			delete light;
		}

		delete m_lightVector;
	}
	
	delete m_parallelLightVector;
	m_parallelLightVector = NULL;

	{
		for (size_t i = 0; i < m_objectVector->size(); ++i)
		{
			Object * const object = (*m_objectVector)[i];

			RenderWorld::removeWorldEnvironmentObject(object);

			if (object->isInWorld())
				object->removeFromWorld();

			delete object;
		}

		delete m_objectVector;
	}

	delete m_objectsToDisableForHyperspace;
	delete m_objectsFollowCameraVector;
}

// ----------------------------------------------------------------------

void SpaceEnvironment::alterInternal(float const elapsedTime)
{
	if (s_camera.getPointer() != NULL)
	{
		float const zoneSize = TerrainObject::getConstInstance()->getMapWidthInMeters();
		float const cameraMovementPlanetFactor = -(s_cameraMovementPlanetRange / zoneSize);
		
		Vector const & cameraPos_w = s_camera->getPosition_w();
		
		{
			for (size_t i = 0; i < m_objectsFollowCameraVector->size(); ++i)
			{
				Object * const object = (*m_objectsFollowCameraVector)[i];				
				Vector const & cameraPosRatio = (cameraPos_w * cameraMovementPlanetFactor);
				Vector const & pos = cameraPos_w + cameraPosRatio;
				object->setPosition_p(pos);
			}
		}
	}
	
	//---

	for (size_t i = 0; i < m_objectVector->size(); ++i)
	{
		Object * const object = (*m_objectVector)[i];
		object->alter(elapsedTime);			
	}
	
	ms_lastAlterTime = elapsedTime;
}

// ----------------------------------------------------------------------

void SpaceEnvironment::render() const
{
	for (size_t i = 0; i < m_objectVector->size(); ++i)
	{
		Object * const object = (*m_objectVector)[i];

		CelestialObject * const celestialObject = dynamic_cast<CelestialObject *>(object);
		if (celestialObject)
			celestialObject->update(ms_lastAlterTime, ShaderPrimitiveSorter::getCurrentCamera());
	}
}

// ----------------------------------------------------------------------

PackedRgb const & SpaceEnvironment::getClearColor() const
{
	return m_clearColor;
}

//----------------------------------------------------------------------

SpaceEnvironment::LightVector const & SpaceEnvironment::getLightVector() const
{
	return *NON_NULL(m_lightVector);
}

//----------------------------------------------------------------------

SpaceEnvironment::LightVector const & SpaceEnvironment::getParallelLights() const
{
	return *NON_NULL(m_parallelLightVector);
}

//----------------------------------------------------------------------

void SpaceEnvironment::addEnvironmentObject(Object & obj)
{
	RenderWorld::addWorldEnvironmentObject(&obj);
	m_objectVector->push_back(&obj);
}

//----------------------------------------------------------------------

void SpaceEnvironment::disableEnvironmentForHyperspace()
{
	NON_NULL(m_objectsToDisableForHyperspace);

	SpaceEnvironment::ObjectVector::const_iterator ii = m_objectsToDisableForHyperspace->begin();
	SpaceEnvironment::ObjectVector::const_iterator iiEnd = m_objectsToDisableForHyperspace->end();

	for (; ii != iiEnd; ++ii)
	{
		Object * const object = *ii;
		if (object != 0)
		{
			object->setActive(false);
		}
	}
}

//----------------------------------------------------------------------

void SpaceEnvironment::setCamera(Camera * camera)
{
	s_camera = camera;
}

//----------------------------------------------------------------------

void SpaceEnvironment::alter(float elapsedTime)
{
	if (ms_spaceEnvironment)
	{
		ms_spaceEnvironment->alterInternal(elapsedTime);
	}
}

// ======================================================================
