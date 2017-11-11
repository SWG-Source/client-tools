//===================================================================
//
// GroundEnvironment.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
// Layers:
//   clear color
//   skybox
//   gradient sky
//   stars
//   celestials
//   top clouds
//   bottom clouds
//
//===================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/GroundEnvironment.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderCapability.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientObject/GameCamera.h"
#include "clientObject/InteriorEnvironmentBlock.h"
#include "clientObject/InteriorEnvironmentBlockManager.h"
#include "clientObject/ShadowVolume.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientTerrain/CelestialAppearance.h"
#include "clientTerrain/CelestialObject.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientTerrain/CloudLayerAppearance.h"
#include "clientTerrain/ConfigClientTerrain.h"
#include "clientTerrain/EnvironmentBlock.h"
#include "clientTerrain/EnvironmentBlockManager.h"
#include "clientTerrain/GradientSkyAppearance.h"
#include "clientTerrain/PlanetAppearance.h"
#include "clientTerrain/SkyBox6SidedAppearance.h"
#include "clientTerrain/SkyBoxAppearance.h"
#include "clientTerrain/StarAppearance.h"
#include "clientTerrain/WeatherManager.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Object.h"
#include "sharedObject/PortalProperty.h"
#include "sharedTerrain/ConfigSharedTerrain.h"
#include "sharedTerrain/ProceduralTerrainAppearanceTemplate.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/FileName.h"

#include "clientGame/ClientRegionEffectManager.h"
#include "clientGame/Game.h"
#include "clientGame/GuildObject.h"
#include "clientGame/PlayerObject.h"

#include "UnicodeUtils.h"

#include <algorithm>
#include <cstdio>

//===================================================================
// GroundEnvironmentNamespace
//===================================================================

namespace GroundEnvironmentNamespace
{
	const Tag TAG_CELS = TAG (C,E,L,S);
	const Tag TAG_ENVM = TAG (E,N,V,M);
	const Tag TAG_MOON = TAG (M,O,O,N);
	const Tag TAG_SMOO = TAG (S,M,O,O);
	const Tag TAG_NSKY = TAG (N,S,K,Y);
	const Tag TAG_STAR = TAG (S,T,A,R);
	const Tag TAG_SUN  = TAG3 (S,U,N);
	const Tag TAG_SSUN = TAG (S,S,U,N);
	const Tag TAG__SKY = TAG (_,S,K,Y);
	const Tag TAG_TLOK = TAG(T,L,O,K);
	const Tag TAG_SKYB = TAG(S,K,Y,B);
	const Tag TAG_DIST = TAG(D,I,S,T);
	const Tag TAG_PLAN = TAG(P,L,A,N);

#ifdef _DEBUG
	bool ms_debugReport;
	bool ms_showLightDirections;
	bool ms_useBlackLights;
	bool ms_forceFallbackLighting;
#endif

	const float ms_dayNightSplit = 0.7f;

	float computeNormalizedTimeRatio (const float timeRatio)
	{
		if (timeRatio < ms_dayNightSplit)
			return linearInterpolate (0.f, 0.5f, timeRatio / ms_dayNightSplit);

		return linearInterpolate (0.5f, 1.f, (timeRatio - ms_dayNightSplit) / (1.f - ms_dayNightSplit));
	}

	const int ms_skyEnvironmentTextureSize = 32;

	GroundEnvironment * ms_instance = NULL;

	bool s_installed = false;

	void _normalizeColor(VectorArgb &c)
	{
		float m = std::max(c.r,std::max(c.g, c.b));
		if (m>1.0)
		{
			c.r/=m;
			c.g/=m;
			c.b/=m;
		}
	}

	// DEBUG STUFF
	//static float debugYaw[] = {0.0f, 7.0f, 14.0f, 20.0f, -12.0f };
	//static float debugPitch[] = { 45.0f, 53.0f, 42.0f, 58.0f, 49.0f };

	const float ms_factionYaw[] = {convertDegreesToRadians(0),
								   convertDegreesToRadians(7),
								   convertDegreesToRadians(14),
								   convertDegreesToRadians(20),
								   convertDegreesToRadians(-12)};

	const float ms_factionPitch[] = {convertDegreesToRadians(45),
								   convertDegreesToRadians(53),
								   convertDegreesToRadians(42),
								   convertDegreesToRadians(58),
								   convertDegreesToRadians(49)};

	const float ms_factionSize[] = {0.175f, 0.35f, 0.25f, 0.4f, 0.45f};

	//-- the planet camera is allowed to 'move' inside a diameter of 100 meters
	float const s_cameraMovementPlanetRange = 100.0f;

	float const s_dayTimeOverride = 0.25f;
	float const s_nightTimeOverride = 0.85f;

	const char * const ms_starDestroyerShader = "shader/cels_star_destroyer.sht";
	const char * const ms_moncalCruiserShader = "shader/cels_moncal_cruiser.sht";

	float const ms_factionUpdateFreq = 5.0f; // 5 seconds.

}

using namespace GroundEnvironmentNamespace;

//===================================================================
// GroundEnvironment::Data
//===================================================================

class GroundEnvironment::Data
{
public:

	class CelestialData
	{
	public:

		std::string   m_shaderTemplateName;
		float         m_size;
		std::string   m_glowShaderTemplateName;
		float         m_glowSize;
		float         m_yaw;
		float         m_pitch;
		float         m_pitchDirection;
		float         m_cycleTime;
		mutable float m_currentTime;

	public:

		CelestialData ();
	};


	struct DistantAppearanceData
	{
		PersistentCrcString m_appearanceTemplateName;
		Vector m_direction_w;
		Vector m_orientation_w;
		float m_haloRoll;
		float m_haloScale;
		bool m_infiniteDistance;
		int m_flag;

	private:

		DistantAppearanceData operator=(DistantAppearanceData const & rhs);
	};

public:

	Data ();

	void load (const char* fileName);

public:

	std::string                m_nightShaderTemplateName;
	std::string                m_starColorRampFileName;
	int                        m_numberOfStars;
	std::string                m_sunShaderTemplateName;
	float                      m_sunSize;
	std::string                m_sunGlowShaderTemplateName;
	float                      m_sunGlowSize;
	std::string                m_supplementalSunShaderTemplateName;
	float                      m_supplementalSunSize;
	std::string                m_supplementalSunGlowShaderTemplateName;
	float                      m_supplementalSunGlowSize;
	float                      m_supplementalSunYaw;
	float                      m_supplementalSunPitch;
	std::string                m_moonShaderTemplateName;
	float                      m_moonSize;
	std::string                m_moonGlowShaderTemplateName;
	float                      m_moonGlowSize;
	std::string                m_supplementalMoonShaderTemplateName;
	float                      m_supplementalMoonSize;
	std::string                m_supplementalMoonGlowShaderTemplateName;
	float                      m_supplementalMoonGlowSize;
	float                      m_supplementalMoonYaw;
	float                      m_supplementalMoonPitch;
	std::vector<CelestialData> m_celestialDataList;
	bool                       m_timeLocked;
	float                      m_lockTime;
	bool                       m_skyBoxCubeMap;
	std::string                m_skyBoxTextureNameMask;
	std::vector<DistantAppearanceData> m_distantAppearances;


private:

	void load_0000 (Iff& iff);
};

//-------------------------------------------------------------------

GroundEnvironment::Data::CelestialData::CelestialData () :
	m_shaderTemplateName (),
	m_size (0),
	m_glowShaderTemplateName (),
	m_glowSize (0),
	m_yaw (0.f),
	m_pitch (0.f),
	m_pitchDirection (0.f),
	m_cycleTime (0.f),
	m_currentTime (0.f)
{
}

//-------------------------------------------------------------------

GroundEnvironment::Data::Data () :
	m_nightShaderTemplateName (),
	m_starColorRampFileName (),
	m_numberOfStars (0),
	m_sunShaderTemplateName (),
	m_sunSize (0.f),
	m_sunGlowShaderTemplateName (),
	m_sunGlowSize (0.f),
	m_supplementalSunShaderTemplateName (),
	m_supplementalSunSize (0.f),
	m_supplementalSunGlowShaderTemplateName (),
	m_supplementalSunGlowSize (0.f),
	m_supplementalSunYaw (0.f),
	m_supplementalSunPitch (0.f),
	m_moonShaderTemplateName (),
	m_moonSize (0.f),
	m_moonGlowShaderTemplateName (),
	m_moonGlowSize (0.f),
	m_supplementalMoonShaderTemplateName (),
	m_supplementalMoonSize (0.f),
	m_supplementalMoonGlowShaderTemplateName (),
	m_supplementalMoonGlowSize (0.f),
	m_supplementalMoonYaw (0.f),
	m_supplementalMoonPitch (0.f),
	m_celestialDataList (),
	m_timeLocked(false),
	m_lockTime(0.0f),
	m_skyBoxCubeMap(false),
	m_skyBoxTextureNameMask()
{
}

//-------------------------------------------------------------------

void GroundEnvironment::Data::load (const char* const fileName)
{
	Iff iff;
	if (iff.open (fileName, true))
	{
		if (iff.getCurrentName () == TAG_ENVM)
		{
			iff.enterForm (TAG_ENVM);

				switch (iff.getCurrentName ())
				{
				case TAG_0000:
					{
						load_0000 (iff);
					}
					break;

				default:
					{
						char tagBuffer [5];
						ConvertTagToString (iff.getCurrentName (), tagBuffer);

						char buffer [128];
						iff.formatLocation (buffer, sizeof (buffer));
						DEBUG_WARNING (true, ("GroundEnvironment::Data::load: unknown client environment version %s/%s", buffer, tagBuffer));
					}
					break;
				}

			iff.exitForm (TAG_ENVM);
		}
		else
		{
			char tagBuffer [5];
			ConvertTagToString (iff.getCurrentName (), tagBuffer);

			char buffer [128];
			iff.formatLocation (buffer, sizeof (buffer));
			DEBUG_WARNING (true, ("GroundEnvironment::Data::load: unknown client environment format (expecting ENVM) %s/%s", buffer, tagBuffer));
		}
	}
	else
		DEBUG_WARNING (true, ("GroundEnvironment::Data::load: could not open %s", fileName));
}

//-------------------------------------------------------------------

void GroundEnvironment::Data::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		while (iff.getNumberOfBlocksLeft ())
		{
			switch (iff.getCurrentName ())
			{
			case TAG_NSKY:
				{
					iff.enterChunk (TAG_NSKY);
						iff.read_string (m_nightShaderTemplateName);
					iff.exitChunk (TAG_NSKY);
				}
				break;

			case TAG_STAR:
				{
					iff.enterChunk (TAG_STAR);
						iff.read_string (m_starColorRampFileName);
						m_numberOfStars = iff.read_int32 ();
					iff.exitChunk (TAG_STAR);
				}
				break;

			case TAG_SUN:
				{
					iff.enterChunk (TAG_SUN);
						iff.read_string (m_sunShaderTemplateName);
						m_sunSize = iff.read_float ();
						iff.read_string (m_sunGlowShaderTemplateName);
						m_sunGlowSize = iff.read_float ();
					iff.exitChunk (TAG_SUN);
				}
				break;

			case TAG_SSUN:
				{
					iff.enterChunk (TAG_SSUN);
						iff.read_string (m_supplementalSunShaderTemplateName);
						m_supplementalSunSize = iff.read_float ();
						iff.read_string (m_supplementalSunGlowShaderTemplateName);
						m_supplementalSunGlowSize = iff.read_float ();
						m_supplementalSunYaw = convertDegreesToRadians (iff.read_float ());
						m_supplementalSunPitch = convertDegreesToRadians (iff.read_float ());
					iff.exitChunk (TAG_SSUN);
				}
				break;

			case TAG_MOON:
				{
					iff.enterChunk (TAG_MOON);
						iff.read_string (m_moonShaderTemplateName);
						m_moonSize = iff.read_float ();
						iff.read_string (m_moonGlowShaderTemplateName);
						m_moonGlowSize = iff.read_float ();
					iff.exitChunk (TAG_MOON);
				}
				break;

			case TAG_SMOO:
				{
					iff.enterChunk (TAG_SMOO);
						iff.read_string (m_supplementalMoonShaderTemplateName);
						m_supplementalMoonSize = iff.read_float ();
						iff.read_string (m_supplementalMoonGlowShaderTemplateName);
						m_supplementalMoonGlowSize = iff.read_float ();
						m_supplementalMoonYaw = convertDegreesToRadians (iff.read_float ());
						m_supplementalMoonPitch = convertDegreesToRadians (iff.read_float ());
					iff.exitChunk (TAG_SMOO);
				}
				break;

			case TAG_CELS:
				{
					iff.enterChunk (TAG_CELS);
						CelestialData celestialData;
						iff.read_string (celestialData.m_shaderTemplateName);
						celestialData.m_size = iff.read_float ();
						iff.read_string (celestialData.m_glowShaderTemplateName);
						celestialData.m_glowSize = iff.read_float ();
						celestialData.m_yaw = convertDegreesToRadians (iff.read_float ());
						celestialData.m_pitch = convertDegreesToRadians (iff.read_float ());
						celestialData.m_pitchDirection = iff.read_float ();
						celestialData.m_cycleTime = iff.read_float ();
						m_celestialDataList.push_back (celestialData);
					iff.exitChunk (TAG_CELS);
				}
				break;

			case TAG_TLOK:
				{
					iff.enterChunk(TAG_TLOK);
					{
						m_timeLocked = iff.read_uint8() != 0;
						m_lockTime = iff.read_float();
					}
				
					iff.exitChunk(TAG_TLOK);
				}
				break;

			case TAG_SKYB:
				{
					iff.enterChunk(TAG_SKYB);
					{
						m_skyBoxCubeMap = iff.read_uint8() != 0;
					    m_skyBoxTextureNameMask = iff.read_stdstring();
					}
					iff.exitChunk(TAG_SKYB);
				}

				break;
				
			case TAG_DIST:
				{
					iff.enterForm(TAG_DIST);
					iff.enterChunk(TAG_0000);
					
					DistantAppearanceData distantAppearanceData;
					std::string appearanceTemplateName;
					iff.read_string(appearanceTemplateName);
					distantAppearanceData.m_appearanceTemplateName.set(appearanceTemplateName.c_str(), true);
					distantAppearanceData.m_direction_w = iff.read_floatVector();
					distantAppearanceData.m_orientation_w.x = convertDegreesToRadians(iff.read_float());
					distantAppearanceData.m_orientation_w.y = convertDegreesToRadians(iff.read_float());
					distantAppearanceData.m_orientation_w.z = convertDegreesToRadians(iff.read_float());
					distantAppearanceData.m_haloRoll = 0.f;
					distantAppearanceData.m_haloScale = 1.f;
					distantAppearanceData.m_infiniteDistance = (iff.read_int8() != 0);
					distantAppearanceData.m_flag = iff.read_int32();
					m_distantAppearances.push_back(distantAppearanceData);
					
					iff.exitChunk(TAG_0000);
					iff.exitForm(TAG_DIST);
				}
				break;
				
			case TAG_PLAN:
				{
					iff.enterForm(TAG_PLAN);
					iff.enterChunk(TAG_0000);
					
					DistantAppearanceData distantAppearanceData;
					std::string appearanceTemplateName;
					iff.read_string(appearanceTemplateName);
					distantAppearanceData.m_appearanceTemplateName.set(appearanceTemplateName.c_str(), true);
					distantAppearanceData.m_direction_w = iff.read_floatVector();
					distantAppearanceData.m_orientation_w.x = convertDegreesToRadians(iff.read_float());
					distantAppearanceData.m_orientation_w.y = convertDegreesToRadians(iff.read_float());
					distantAppearanceData.m_orientation_w.z = convertDegreesToRadians(iff.read_float());
					distantAppearanceData.m_haloRoll = convertDegreesToRadians(iff.read_float());
					distantAppearanceData.m_haloScale = iff.read_float();
					distantAppearanceData.m_infiniteDistance = (iff.read_int8() != 0);
					distantAppearanceData.m_flag = 0;
					m_distantAppearances.push_back(distantAppearanceData);
					
					iff.exitChunk(TAG_0000);
					iff.exitForm(TAG_PLAN);
				}
				break;
			default:
				{
#ifdef _DEBUG
					char tagString [5];
					ConvertTagToString (iff.getCurrentName (), tagString);
					DEBUG_WARNING (true, ("unknown chunk type %s, expecting ASND, CEFT, CHLD, CSND, DAMA, HLOB, HOBJ, or LOBJ", tagString));
#endif

					iff.enterChunk ();
					iff.exitChunk (true);
				}
				break;
			}
		}

	iff.exitForm (TAG_0000);
}

//===================================================================
// STATIC GroundEnvironment
//===================================================================


//===================================================================

void GroundEnvironment::install()
{
	DEBUG_FATAL(s_installed, ("GroundEnvironment already installed"));
	DEBUG_REPORT_LOG_PRINT (ConfigSharedTerrain::getDebugReportInstall (), ("GroundEnvironment::install\n"));

#ifdef _DEBUG
	DebugFlags::registerFlag (ms_debugReport,         "ClientTerrain", "reportEnvironment");
	DebugFlags::registerFlag (ms_showLightDirections, "ClientTerrain", "showLightDirections");
	DebugFlags::registerFlag (ms_useBlackLights,      "ClientTerrain", "useBlackLights");
	DebugFlags::registerFlag (ms_forceFallbackLighting,"ClientTerrain", "forceFallbackLighting");
#endif

	s_installed = true;
}

//-------------------------------------------------------------------

void GroundEnvironment::remove()
{
	GroundEnvironment::destroyInstance();

#ifdef _DEBUG
	DebugFlags::unregisterFlag (ms_showLightDirections);
	DebugFlags::unregisterFlag (ms_debugReport);
	DebugFlags::unregisterFlag (ms_useBlackLights);
	DebugFlags::unregisterFlag (ms_forceFallbackLighting);
#endif

	s_installed = false;
}

//-------------------------------------------------------------------

void GroundEnvironment::setReferenceCamera (Camera* const referenceCamera)
{
	m_referenceCamera = referenceCamera;
}

//-------------------------------------------------------------------

void GroundEnvironment::setReferenceObject (const Object* const referenceObject)
{
	m_referenceObject = referenceObject;
}

//-------------------------------------------------------------------

void GroundEnvironment::setWeatherIndex (const int weatherIndex)
{
	m_weatherIndex = weatherIndex;
}

//-------------------------------------------------------------------

void GroundEnvironment::setEnableFog (const bool enableFog)
{
	m_enableFog = enableFog;
}

//-------------------------------------------------------------------

bool GroundEnvironment::getEnableFog ()
{
	return m_enableFog;
}

//===================================================================

GroundEnvironment::GroundEnvironment() :
	m_clientProceduralTerrainAppearance(NULL),
	m_referenceCamera(NULL),
	m_referenceObject(NULL),
	m_weatherIndex(0),
	m_enableFog(true),
	m_data (new Data),
	m_environmentBlockManager (0),
	m_currentEnvironmentBlock (0),
	m_previousEnvironmentBlock (0),
	m_timer (1.f),
	m_transitioning (false),
	m_previousGradientSkyObject (0),
	m_currentGradientSkyObject (0),
	m_nightStarsObject (0),
	m_previousBottomCloudsObject (0),
	m_currentBottomCloudsObject (0),
	m_previousTopCloudsObject (0),
	m_currentTopCloudsObject (0),
	m_skyBoxObject(NULL),
	m_ambientLight (0),
	m_mainLight (0),
	m_mainYaw (0),
	m_fillLight (0),
	m_fillYaw (PI),
	m_bounceLight (0),
	m_bounceYaw (PI_OVER_2),
	m_fogEnabled (false),
	m_fogColor (VectorArgb::solidWhite),
	m_fogDensity (0.f),
	m_paused (ConfigClientTerrain::getDisableTimeOfDay ()),
	m_currentTime (0.f),
	m_cycleTime (86400.0f),
	m_timeRatio (0.f),
	m_normalizedTimeRatio (0.f),
	m_currentColorIndex (0),
	m_clearColor (),
	m_hour (0),
	m_minute (0),
	m_terrainCloudShader (0),
	m_sunCelestialObject (0),
	m_supplementalSunCelestialObject (0),
	m_moonCelestialObject (0),
	m_supplementalMoonCelestialObject (0),
	m_currentSkyAlpha (0.f),
	m_celestialObjectList (new ObjectList),
	m_currentInteriorEnvironmentBlock (0),
	m_previousInteriorEnvironmentBlock (0),
	m_skyEnvironmentMap (0),
	m_clearColorTexture (0),
	m_objectIntMap(new ObjectIntMap),
	m_objectsFollowCameraVector(new ObjectVector),
	m_factionCelestials(new CelestialObjectVector),
	m_factionStanding(0),
	m_factionOccupationLevel(0),
	m_factionUpdateTimer(0.0f)
{
}

//-------------------------------------------------------------------

GroundEnvironment::~GroundEnvironment ()
{
	RenderWorld::setClearDepthAndStencilBufferAfterRenderingEnvironment(false);

	CellProperty::getWorldCellProperty ()->setEnvironmentTexture (NULL);

	delete m_environmentBlockManager;

	int i;
	for (i = 0; i < m_celestialObjectList->getNumberOfObjects (); ++i)
		RenderWorld::removeWorldEnvironmentObject (m_celestialObjectList->getObject (i));

	m_celestialObjectList->removeAll (true);
	delete m_celestialObjectList;

	if (m_supplementalSunCelestialObject)
	{
		RenderWorld::removeWorldEnvironmentObject(m_supplementalSunCelestialObject);
		delete m_supplementalSunCelestialObject;
		m_supplementalSunCelestialObject = 0;
	}

	if (m_sunCelestialObject)
	{
		RenderWorld::removeWorldEnvironmentObject(m_sunCelestialObject);
		delete m_sunCelestialObject;
		m_sunCelestialObject = 0;
	}

	if (m_supplementalMoonCelestialObject)
	{
		RenderWorld::removeWorldEnvironmentObject(m_supplementalMoonCelestialObject);
		delete m_supplementalMoonCelestialObject;
		m_supplementalMoonCelestialObject = 0;
	}

	if (m_moonCelestialObject)
	{
		RenderWorld::removeWorldEnvironmentObject(m_moonCelestialObject);
		delete m_moonCelestialObject;
		m_moonCelestialObject = 0;
	}

	if (m_previousGradientSkyObject)
	{
		RenderWorld::removeWorldEnvironmentObject(m_previousGradientSkyObject);
		delete m_previousGradientSkyObject;
		m_previousGradientSkyObject = 0;
	}

	if (m_currentGradientSkyObject)
	{
		RenderWorld::removeWorldEnvironmentObject(m_currentGradientSkyObject);
		delete m_currentGradientSkyObject;
		m_currentGradientSkyObject = 0;
	}

	if (m_nightStarsObject)
	{
		RenderWorld::removeWorldEnvironmentObject(m_nightStarsObject);
		delete m_nightStarsObject;
		m_nightStarsObject = 0;
	}

	if (m_previousBottomCloudsObject)
	{
		RenderWorld::removeWorldEnvironmentObject(m_previousBottomCloudsObject);
		delete m_previousBottomCloudsObject;
		m_previousBottomCloudsObject= 0;
	}

	if (m_currentBottomCloudsObject)
	{
		RenderWorld::removeWorldEnvironmentObject(m_currentBottomCloudsObject);
		delete m_currentBottomCloudsObject;
		m_currentBottomCloudsObject = 0;
	}

	if (m_previousTopCloudsObject)
	{
		RenderWorld::removeWorldEnvironmentObject(m_previousTopCloudsObject);
		delete m_previousTopCloudsObject;
		m_previousTopCloudsObject= 0;
	}

	if (m_currentTopCloudsObject)
	{
		RenderWorld::removeWorldEnvironmentObject(m_currentTopCloudsObject);
		delete m_currentTopCloudsObject;
		m_currentTopCloudsObject = 0;
	}

	if (m_skyBoxObject)
	{
		RenderWorld::removeWorldEnvironmentObject(m_skyBoxObject);
		delete m_skyBoxObject;
		m_skyBoxObject = NULL;
	}

	if (m_ambientLight)
	{
		RenderWorld::removeWorldEnvironmentLight(m_ambientLight);
		m_ambientLight->removeFromWorld ();
		delete m_ambientLight;
		m_ambientLight = 0;
	}

	if (m_mainLight)
	{
		RenderWorld::removeWorldEnvironmentLight(m_mainLight);
		m_mainLight->removeFromWorld ();
		delete m_mainLight;
		m_mainLight = 0;
	}

	if (m_fillLight)
	{
		RenderWorld::removeWorldEnvironmentLight(m_fillLight);
		m_fillLight->removeFromWorld ();
		delete m_fillLight;
		m_fillLight = 0;
	}

	if (m_bounceLight)
	{
		RenderWorld::removeWorldEnvironmentLight(m_bounceLight);
		m_bounceLight->removeFromWorld ();
		delete m_bounceLight;
		m_bounceLight = 0;
	}

	if (m_terrainCloudShader)
	{
		m_terrainCloudShader->release ();
		m_terrainCloudShader = 0;
	}

	delete m_data;
	m_data = NULL;

	if (m_skyEnvironmentMap) 
	{
		m_skyEnvironmentMap->release ();
		m_skyEnvironmentMap = 0;
	}

	if (m_clearColorTexture) 
	{
		m_clearColorTexture->release ();
		m_clearColorTexture = 0;
	}

	m_currentEnvironmentBlock = NULL;
	m_previousEnvironmentBlock = NULL;
	m_currentInteriorEnvironmentBlock = NULL;
	m_previousInteriorEnvironmentBlock = NULL;


	{
		for (ObjectIntMap::iterator it = m_objectIntMap->begin(); it != m_objectIntMap->end(); ++it)
		{
			Object * const object = it->first;

			RenderWorld::removeWorldEnvironmentObject(object);

			if (object->isInWorld())
				object->removeFromWorld();

			delete object;
		}
	}

	delete m_objectIntMap;
	m_objectIntMap = NULL;
	delete m_objectsFollowCameraVector;
	m_objectsFollowCameraVector = NULL;

	cleanUpFactionCelestials();
	delete m_factionCelestials;
	m_factionCelestials = NULL;

}

void GroundEnvironment::cleanUpFactionCelestials()
{
	for(CelestialObjectVector::iterator iter = m_factionCelestials->begin(); iter != m_factionCelestials->end(); ++iter)
	{
		Object * const object = (*iter);

		RenderWorld::removeWorldEnvironmentObject(object);

		if (object->isInWorld())
			object->removeFromWorld();

		delete object;
	}

	m_factionCelestials->clear();
}

//-------------------------------------------------------------------

const PackedRgb GroundEnvironment::getClearColor () const
{
	return m_clearColor;
}

//-------------------------------------------------------------------

const PackedRgb GroundEnvironment::getFogColor () const
{
	PackedRgb fogColor;
	fogColor.convert (m_fogColor);

	return fogColor;
}

//-------------------------------------------------------------------

float GroundEnvironment::getFogDensity() const
{
	return m_fogDensity;
}

//-------------------------------------------------------------------

int GroundEnvironment::getHour () const
{
	return m_hour;
}

//-------------------------------------------------------------------

int GroundEnvironment::getMinute () const
{
	return m_minute;
}

//-------------------------------------------------------------------

float GroundEnvironment::getTime () const
{
	return m_currentTime / m_cycleTime;
}

//-------------------------------------------------------------------

void GroundEnvironment::setTime (const float time, const bool force)
{
	if ((!m_paused && !m_data->m_timeLocked) || force)
		m_currentTime = time * m_cycleTime;

	//-- update celestials
	int i;
	for (i = 0; i < m_celestialObjectList->getNumberOfObjects (); ++i)
	{
		Data::CelestialData& celestialData = m_data->m_celestialDataList [i];

		if (celestialData.m_cycleTime > 0.f)
			celestialData.m_currentTime = fmodf (m_currentTime, celestialData.m_cycleTime);
	}
}

//-------------------------------------------------------------------

bool GroundEnvironment::isDay () const
{
	return m_timeRatio < ms_dayNightSplit || ( Game::getPlayerObject() ? ClientRegionEffectManager::isCurrentRegionPermanentDay( Game::getPlayerObject()->getEnvironmentFlags()) : 0 );
}

//-------------------------------------------------------------------

bool GroundEnvironment::isNight () const
{
	return m_timeRatio >= ms_dayNightSplit ||  ( Game::getPlayerObject() ? ClientRegionEffectManager::isCurrentRegionPermanentNight( Game::getPlayerObject()->getEnvironmentFlags()) : 0 );
}

//----------------------------------------------------------------------

bool GroundEnvironment::isTimeLocked() const
{
	return m_data->m_timeLocked;
}

//-------------------------------------------------------------------

const Shader* GroundEnvironment::getTerrainCloudShader () const
{
	return m_terrainCloudShader;
}

//-------------------------------------------------------------------

const EnvironmentBlock* GroundEnvironment::getCurrentEnvironmentBlock () const
{
	return m_currentEnvironmentBlock;
}

//-------------------------------------------------------------------

const InteriorEnvironmentBlock* GroundEnvironment::getCurrentInteriorEnvironmentBlock () const
{
	return m_currentInteriorEnvironmentBlock;
}

//-------------------------------------------------------------------

bool GroundEnvironment::getPaused () const
{
	return m_paused;
}

//-------------------------------------------------------------------

void GroundEnvironment::setPaused (bool const paused)
{
	m_paused = paused;
}

//-------------------------------------------------------------------

void GroundEnvironment::apply (float const t)
{
	if (!m_clientProceduralTerrainAppearance)
		return;

	DEBUG_REPORT_PRINT (ms_debugReport, ("t                   = %1.2f\n", t));
	DEBUG_REPORT_PRINT (ms_debugReport, ("name                = %s\n", m_currentEnvironmentBlock->getName () ? m_currentEnvironmentBlock->getName () : "null"));
	
	// ----------------------------------------------------------------------
	// lighting colors
	VectorArgb ambientColor      = VectorArgb::linearInterpolate (m_previousEnvironmentBlock->getAmbientColorRamp () [m_currentColorIndex].convert (), m_currentEnvironmentBlock->getAmbientColorRamp () [m_currentColorIndex].convert (), t);
	VectorArgb mainSpecularColor = VectorArgb::linearInterpolate (m_previousEnvironmentBlock->getMainSpecularColorRamp () [m_currentColorIndex].convert (), m_currentEnvironmentBlock->getMainSpecularColorRamp () [m_currentColorIndex].convert (), t);
	VectorArgb mainDiffuseColor  = VectorArgb::linearInterpolate (m_previousEnvironmentBlock->getMainDiffuseColorRamp () [m_currentColorIndex].convert (), m_currentEnvironmentBlock->getMainDiffuseColorRamp () [m_currentColorIndex].convert (), t);
	VectorArgb mainBackColor     = VectorArgb::linearInterpolate (m_previousEnvironmentBlock->getMainBackColorRamp () [m_currentColorIndex].convert (), m_currentEnvironmentBlock->getMainBackColorRamp () [m_currentColorIndex].convert (), t);
	VectorArgb mainTangentColor  = VectorArgb::linearInterpolate (m_previousEnvironmentBlock->getMainTangentColorRamp () [m_currentColorIndex].convert (), m_currentEnvironmentBlock->getMainTangentColorRamp () [m_currentColorIndex].convert (), t);
	VectorArgb fillColor         = VectorArgb::linearInterpolate (m_previousEnvironmentBlock->getFillColorRamp () [m_currentColorIndex].convert (), m_currentEnvironmentBlock->getFillColorRamp () [m_currentColorIndex].convert (), t);
	VectorArgb bounceColor       = VectorArgb::linearInterpolate (m_previousEnvironmentBlock->getBounceColorRamp () [m_currentColorIndex].convert (), m_currentEnvironmentBlock->getBounceColorRamp () [m_currentColorIndex].convert (), t);

	const float mainDiffuseColorScale  = ::linearInterpolate (m_previousEnvironmentBlock->getMainDiffuseColorScale () [m_currentColorIndex], m_currentEnvironmentBlock->getMainDiffuseColorScale () [m_currentColorIndex], t);
	const float mainSpecularColorScale = ::linearInterpolate (m_previousEnvironmentBlock->getMainSpecularColorScale () [m_currentColorIndex], m_currentEnvironmentBlock->getMainSpecularColorScale () [m_currentColorIndex], t);
	      float fillColorScale         = ::linearInterpolate (m_previousEnvironmentBlock->getFillColorScale () [m_currentColorIndex], m_currentEnvironmentBlock->getFillColorScale () [m_currentColorIndex], t);
	      float bounceColorScale       = ::linearInterpolate (m_previousEnvironmentBlock->getBounceColorScale () [m_currentColorIndex], m_currentEnvironmentBlock->getBounceColorScale () [m_currentColorIndex], t);
	const float mainBackColorScale     = ::linearInterpolate (m_previousEnvironmentBlock->getMainBackColorScale () [m_currentColorIndex], m_currentEnvironmentBlock->getMainBackColorScale () [m_currentColorIndex], t);
	const float mainTangentColorScale  = ::linearInterpolate (m_previousEnvironmentBlock->getMainTangentColorScale () [m_currentColorIndex], m_currentEnvironmentBlock->getMainTangentColorScale () [m_currentColorIndex], t);

#ifdef _DEBUG
	if (ms_useBlackLights)
	{
		ambientColor      = VectorArgb::solidBlack;
		mainDiffuseColor  = VectorArgb::solidBlack;
		mainSpecularColor = VectorArgb::solidBlack;
		fillColor         = VectorArgb::solidBlack;
		bounceColor       = VectorArgb::solidBlack;
		mainBackColor     = VectorArgb::solidBlack;
		mainTangentColor  = VectorArgb::solidBlack;
	}
#endif

	bool hasHemiLighting = Graphics::getShaderCapability() >= ShaderCapability(2,0);
#ifdef _DEBUG
	if (ms_forceFallbackLighting)
	{
		hasHemiLighting=false;
	}
#endif
	if (!hasHemiLighting)
	{
		bounceColor = bounceColor*bounceColorScale + mainBackColor*mainBackColorScale;
		bounceColorScale=1.0f;
		_normalizeColor(bounceColor);
		mainBackColor.set(0,0,0,0);

		// put 65% of tangent into fill and 35% into ambient.
		fillColor = fillColor*fillColorScale + mainTangentColor*(mainTangentColorScale*0.65f);
		fillColorScale=1.0f;
		_normalizeColor(fillColor);
		ambientColor = ambientColor + mainTangentColor*(mainTangentColorScale*0.35f);
		_normalizeColor(ambientColor);
		mainTangentColor.set(0,0,0,0);
	}

	// ----------------------------------------------------------------------

	DEBUG_REPORT_PRINT (ms_debugReport, ("ambient             = %3i, %3i, %3i\n", static_cast<int> (255.f * ambientColor.r), static_cast<int> (255.f * ambientColor.g), static_cast<int> (255.f * ambientColor.b)));
	DEBUG_REPORT_PRINT (ms_debugReport, ("main diffuse        = %3i, %3i, %3i\n", static_cast<int> (255.f * mainDiffuseColor.r), static_cast<int> (255.f * mainDiffuseColor.g), static_cast<int> (255.f * mainDiffuseColor.b)));
	DEBUG_REPORT_PRINT (ms_debugReport, ("main diffuse scale  = %1.2f\n", mainDiffuseColorScale));
	DEBUG_REPORT_PRINT (ms_debugReport, ("main specular       = %3i, %3i, %3i\n", static_cast<int> (255.f * mainSpecularColor.r), static_cast<int> (255.f * mainSpecularColor.g), static_cast<int> (255.f * mainSpecularColor.b)));
	DEBUG_REPORT_PRINT (ms_debugReport, ("main specular scale = %1.2f\n", mainSpecularColorScale));
	DEBUG_REPORT_PRINT (ms_debugReport, ("fill                = %3i, %3i, %3i\n", static_cast<int> (255.f * fillColor.r), static_cast<int> (255.f * fillColor.g), static_cast<int> (255.f * fillColor.b)));
	DEBUG_REPORT_PRINT (ms_debugReport, ("fill scale          = %1.2f\n", fillColorScale));
	DEBUG_REPORT_PRINT (ms_debugReport, ("bounce              = %3i, %3i, %3i\n", static_cast<int> (255.f * bounceColor.r), static_cast<int> (255.f * bounceColor.g), static_cast<int> (255.f * bounceColor.b)));
	DEBUG_REPORT_PRINT (ms_debugReport, ("bounce scale        = %1.2f\n", bounceColorScale));
	DEBUG_REPORT_PRINT (ms_debugReport, ("main back color     = %3i, %3i, %3i\n", static_cast<int> (255.f * mainBackColor.r), static_cast<int> (255.f * mainBackColor.g), static_cast<int> (255.f * mainBackColor.b)));
	DEBUG_REPORT_PRINT (ms_debugReport, ("main back color scale = %1.2f\n", mainBackColorScale));
	DEBUG_REPORT_PRINT (ms_debugReport, ("main tangent color  = %3i, %3i, %3i\n", static_cast<int> (255.f * mainTangentColor.r), static_cast<int> (255.f * mainTangentColor.g), static_cast<int> (255.f * mainTangentColor.b)));
	DEBUG_REPORT_PRINT (ms_debugReport, ("main tangent color scale = %1.2f\n", mainTangentColorScale));

	m_ambientLight->setDiffuseColor (ambientColor);

	m_mainLight->setDiffuseColor (mainDiffuseColor);

	m_mainLight->setDiffuseColorScale (mainDiffuseColorScale);

	m_mainLight->setSpecularColor (mainSpecularColor);

	m_mainLight->setSpecularColorScale (mainSpecularColorScale);

	m_mainLight->setDiffuseBackColor (mainBackColor);

	m_mainLight->setDiffuseBackColorScale (mainBackColorScale);

	m_mainLight->setDiffuseTangentColor (mainTangentColor);

	m_mainLight->setDiffuseTangentColorScale (mainTangentColorScale);

	m_fillLight->setDiffuseColor (fillColor);
	m_fillLight->setDiffuseColorScale (fillColorScale);

	m_bounceLight->setDiffuseColor (bounceColor);

	m_bounceLight->setDiffuseColorScale (bounceColorScale);

	// ----------------------------------------------------------------------

	m_fogColor = VectorArgb::linearInterpolate (m_previousEnvironmentBlock->getFogColorRamp () [m_currentColorIndex].convert (), m_currentEnvironmentBlock->getFogColorRamp () [m_currentColorIndex].convert (), t);

	VectorArgb previousShadowColor = m_previousEnvironmentBlock->getShadowRamp () [m_currentColorIndex];
	VectorArgb currentShadowColor = m_currentEnvironmentBlock->getShadowRamp () [m_currentColorIndex];
	const VectorArgb shadowColor = VectorArgb::linearInterpolate (previousShadowColor, currentShadowColor, t);

	m_clearColor = PackedRgb::linearInterpolate (m_previousEnvironmentBlock->getClearColorRamp () [m_currentColorIndex], m_currentEnvironmentBlock->getClearColorRamp () [m_currentColorIndex], t);

	// ----------------------------------------------------------------------

	const float sunMoonAlpha = ::linearInterpolate (m_previousEnvironmentBlock->getSunMoonAlphaRamp () [m_currentColorIndex], m_currentEnvironmentBlock->getSunMoonAlphaRamp () [m_currentColorIndex], t);
	float const minimumFogDensity = ::linearInterpolate (m_previousEnvironmentBlock->getFogEnabled () ? m_previousEnvironmentBlock->getMinimumFogDensity () : 0.f, m_currentEnvironmentBlock->getFogEnabled () ? m_currentEnvironmentBlock->getMinimumFogDensity () : 0.f, t);
	float const maximumFogDensity = ::linearInterpolate (m_previousEnvironmentBlock->getFogEnabled () ? m_previousEnvironmentBlock->getMaximumFogDensity () : 0.f, m_currentEnvironmentBlock->getFogEnabled () ? m_currentEnvironmentBlock->getMaximumFogDensity () : 0.f, t);
	m_fogDensity = ::linearInterpolate (minimumFogDensity, maximumFogDensity, clamp (0.f, (m_referenceCamera->getFarPlane () - 512.f) / (2048.f - 512.f), 1.f));
	const float desiredCelestialAlpha = ::linearInterpolate (m_previousEnvironmentBlock->getCelestialAlphaRamp () [m_currentColorIndex], m_currentEnvironmentBlock->getCelestialAlphaRamp () [m_currentColorIndex], t);
	const bool previousCelestialAlphaEnabled = m_previousEnvironmentBlock->getGradientSkyTexture () != 0;
	const bool currentCelestialAlphaEnabled = m_currentEnvironmentBlock->getGradientSkyTexture () != 0;
	const float celestialAlphaModifier = ::linearInterpolate (previousCelestialAlphaEnabled ? 1.f : 0.f, currentCelestialAlphaEnabled ? 1.f : 0.f, t);
	const float starAlpha = ::linearInterpolate (m_previousEnvironmentBlock->getStarAlphaRamp () [m_currentColorIndex], m_currentEnvironmentBlock->getStarAlphaRamp () [m_currentColorIndex], t);
	float const windScale = ::linearInterpolate (m_previousEnvironmentBlock->getWindSpeedScale (), m_currentEnvironmentBlock->getWindSpeedScale (), t);

	// ----------------------------------------------------------------------

	if (m_sunCelestialObject)
		m_sunCelestialObject->setAlpha (sunMoonAlpha);

	if (m_supplementalSunCelestialObject)
		m_supplementalSunCelestialObject->setAlpha (sunMoonAlpha);

	if (m_moonCelestialObject)
		m_moonCelestialObject->setAlpha (sunMoonAlpha);

	if (m_supplementalMoonCelestialObject)
		m_supplementalMoonCelestialObject->setAlpha (sunMoonAlpha);


	m_fogEnabled = m_previousEnvironmentBlock->getFogEnabled () || m_currentEnvironmentBlock->getFogEnabled ();
	DEBUG_REPORT_PRINT (ms_debugReport, ("fog enabled         = %s\n", m_fogEnabled ? "yes" : "no"));

	DEBUG_REPORT_PRINT (ms_debugReport, ("fog color           = %3i, %3i, %3i\n", static_cast<int> (255.f * m_fogColor.r), static_cast<int> (255.f * m_fogColor.g), static_cast<int> (255.f * m_fogColor.b)));

	DEBUG_REPORT_PRINT (ms_debugReport, ("minimum fog density = %1.5f\n", minimumFogDensity));

	DEBUG_REPORT_PRINT (ms_debugReport, ("maximum fog density = %1.5f\n", maximumFogDensity));

	DEBUG_REPORT_PRINT (ms_debugReport, ("fog density         = %1.5f\n", m_fogDensity));

	DEBUG_REPORT_PRINT (ms_debugReport, ("clear               = %3i, %3i, %3i\n", static_cast<int> (255.f * m_clearColor.r), static_cast<int> (255.f * m_clearColor.g), static_cast<int> (255.f * m_clearColor.b)));

	DEBUG_REPORT_PRINT (ms_debugReport, ("desiredCelestialAlpha = %1.2f\n", desiredCelestialAlpha));
	const float currentCelestialAlpha = desiredCelestialAlpha * celestialAlphaModifier;
	DEBUG_REPORT_PRINT (ms_debugReport, ("currentCelestialAlpha = %1.2f\n", currentCelestialAlpha));
	UNREF(currentCelestialAlpha);

	int i;
	for (i = 0; i < m_celestialObjectList->getNumberOfObjects (); ++i)
	{
		CelestialObject* const celestialObject = safe_cast<CelestialObject*> (m_celestialObjectList->getObject (i));
		celestialObject->setAlpha (currentCelestialAlpha);
	}

	DEBUG_REPORT_PRINT (ms_debugReport, ("           star alpha = %1.2f\n", starAlpha));

	if (m_nightStarsObject)
		m_nightStarsObject->getAppearance ()->setAlpha (true, starAlpha, true, starAlpha);

	const float fadeInAlpha  = t;
	const float fadeOutAlpha = 1.f - t;

	if (m_previousGradientSkyObject && m_currentGradientSkyObject)
	{
		if (m_previousGradientSkyObject->getAppearance ())
			m_previousGradientSkyObject->getAppearance ()->setAlpha (true, fadeOutAlpha, true, fadeOutAlpha);

		if (m_currentGradientSkyObject->getAppearance ())
			m_currentGradientSkyObject->getAppearance ()->setAlpha (true, fadeInAlpha, true, fadeInAlpha);
	}

#ifdef _DEBUG
	if (ms_debugReport)
	{
		GradientSkyAppearance const * const gradientSkyAppearance = dynamic_cast<GradientSkyAppearance const *> (m_currentGradientSkyObject->getAppearance ());
		if (gradientSkyAppearance)
			DEBUG_REPORT_PRINT (true, ("gradientSkyName     = %s\n", gradientSkyAppearance->getName ()));
		else
		{
			SkyBoxAppearance const * const skyBoxAppearance = dynamic_cast<SkyBoxAppearance const *> (m_currentGradientSkyObject->getAppearance ());
			if (skyBoxAppearance)
				DEBUG_REPORT_PRINT (true, ("gradientSkyName     = dynamic [%s]\n", skyBoxAppearance->getName ()));
		}
	}
#endif

	if (m_previousTopCloudsObject->getAppearance ())
		m_previousTopCloudsObject->getAppearance ()->setAlpha (true, fadeOutAlpha, true, fadeOutAlpha);

	if (m_currentTopCloudsObject->getAppearance ())
		m_currentTopCloudsObject->getAppearance ()->setAlpha (true, fadeInAlpha, true, fadeInAlpha);

	DEBUG_REPORT_PRINT (ms_debugReport, ("topCloudsName       = %s\n", m_currentTopCloudsObject->getAppearance () ? safe_cast<const CloudLayerAppearance*> (m_currentTopCloudsObject->getAppearance ())->getName () : "none"));

	if (m_previousBottomCloudsObject->getAppearance ())
		m_previousBottomCloudsObject->getAppearance ()->setAlpha (true, fadeOutAlpha, true, fadeOutAlpha);

	if (m_currentBottomCloudsObject->getAppearance ())
		m_currentBottomCloudsObject->getAppearance ()->setAlpha (true, fadeInAlpha, true, fadeInAlpha);

	DEBUG_REPORT_PRINT (ms_debugReport, ("bottomCloudsName    = %s\n", m_currentBottomCloudsObject->getAppearance () ? safe_cast<const CloudLayerAppearance*> (m_currentBottomCloudsObject->getAppearance ())->getName () : "none"));

	const bool shadowsEnabled = m_previousEnvironmentBlock->getShadowsEnabled () || m_currentEnvironmentBlock->getShadowsEnabled ();
	DEBUG_REPORT_PRINT (ms_debugReport, ("shadows enabled     = %s\n", shadowsEnabled ? "yes" : "no"));
	UNREF(shadowsEnabled);

	if (!m_previousEnvironmentBlock->getShadowsEnabled ())
		previousShadowColor.a = 0.f;

	if (!m_currentEnvironmentBlock->getShadowsEnabled ())
		currentShadowColor.a = 0.f;

	DEBUG_REPORT_PRINT (ms_debugReport, ("shadow color        = %3i, %3i, %3i, %3i\n", static_cast<int> (255.f * shadowColor.a), static_cast<int> (255.f * shadowColor.r), static_cast<int> (255.f * shadowColor.g), static_cast<int> (255.f * shadowColor.b)));
	ShadowVolume::setShadowColor (shadowColor);

	DEBUG_REPORT_PRINT (ms_debugReport, ("environment texture = %s\n", isDay () ? (m_currentEnvironmentBlock->getDayEnvironmentTexture () ? m_currentEnvironmentBlock->getDayEnvironmentTexture ()->getName () : "none") : (m_currentEnvironmentBlock->getNightEnvironmentTexture () ? m_currentEnvironmentBlock->getNightEnvironmentTexture ()->getName () : "none")));

	DEBUG_REPORT_PRINT (ms_debugReport, ("         wind scale = %1.2f\n", windScale));
	WeatherManager::setWindScale (windScale);
}

//-------------------------------------------------------------------

void GroundEnvironment::alter(float elapsedTime)
{
	if (!m_clientProceduralTerrainAppearance)
		return;

	// The following code checks to see if we are currently in a region that specifically wants to be night/day.
	float timeOverride = 0.0f;
	bool useOverride = false;

	m_factionUpdateTimer += elapsedTime;

	if ( Game::getPlayerObject() )
	{

		if( ClientRegionEffectManager::isCurrentRegionPermanentNight( Game::getPlayerObject()->getEnvironmentFlags()) )
		{
			timeOverride = s_nightTimeOverride * m_cycleTime;
			useOverride = true;
		}

		if( ClientRegionEffectManager::isCurrentRegionPermanentDay( Game::getPlayerObject()->getEnvironmentFlags()) )
		{
			timeOverride = s_dayTimeOverride * m_cycleTime;
			useOverride = true;
		}
		
	}

	if (!m_paused && !m_data->m_timeLocked)
		m_currentTime += elapsedTime;

	if (m_currentTime > m_cycleTime)
		m_currentTime = fmodf (m_currentTime, m_cycleTime);

	// We make a copy of our "Real" time so we can set it back(if we're using an override) at the end of this frame.
	float savedCurrentTime = m_currentTime;

	if(useOverride)
		m_currentTime = timeOverride;

	DEBUG_REPORT_PRINT (ms_debugReport, ("current/cycle time  = %1.2f/%1.2f\n", m_currentTime, m_cycleTime));

	m_timeRatio = clamp (0.f, m_currentTime / m_cycleTime, 1.f);
	m_normalizedTimeRatio = computeNormalizedTimeRatio(m_timeRatio);
	if (m_normalizedTimeRatio==1.0)
	{
		m_normalizedTimeRatio=0;
	}

	DEBUG_REPORT_PRINT (ms_debugReport, ("timeRatio           = %1.2f\n", m_timeRatio));
	DEBUG_REPORT_PRINT (ms_debugReport, ("normalizedTimeRatio = %1.2f\n", m_normalizedTimeRatio));

	m_hour   = 6 + static_cast<int> (m_timeRatio * 24.f * 60.f) / 60;
	if (m_hour >= 24)
		m_hour -= 24;
	m_minute = static_cast<int> (fmodf (m_timeRatio * 24.f * 60.f, 60.f));

	DEBUG_REPORT_PRINT (ms_debugReport, ("clock time          = %02i:%02i\n", m_hour, m_minute));

	if (m_currentGradientSkyObject && m_previousGradientSkyObject)
	{
		NOT_NULL (m_previousGradientSkyObject);
		if (dynamic_cast<GradientSkyAppearance*> (m_previousGradientSkyObject->getAppearance ()))
			safe_cast<GradientSkyAppearance*> (m_previousGradientSkyObject->getAppearance ())->setTime (m_normalizedTimeRatio);

		NOT_NULL (m_currentGradientSkyObject);
		if (dynamic_cast<GradientSkyAppearance*> (m_currentGradientSkyObject->getAppearance ()))
			safe_cast<GradientSkyAppearance*> (m_currentGradientSkyObject->getAppearance ())->setTime (m_normalizedTimeRatio);
	}

	//-- jake wants the main light at 12 degrees from the y axis
	if (isDay ())
		m_mainYaw = -PI_OVER_2 + m_normalizedTimeRatio * PI_TIMES_2;
	else
		m_mainYaw = -PI - PI_OVER_2 + m_normalizedTimeRatio * PI_TIMES_2;

	Transform t;
	t.pitch_l(float(90.0 - 22.5) * PI_OVER_180);
	t.yaw_l (m_mainYaw);

	const Vector k = t.getLocalFrameK_p ();
	Vector i = Vector::unitZ;
	Vector j = k.cross (i);
	i = j.cross(k);

	t.setLocalFrameIJK_p (i, j, k);
	t.reorthonormalize ();
	m_mainLight->setTransform_o2p (t);

	const Vector mainDirection = -m_mainLight->getObjectFrameK_w();

	if (ClientProceduralTerrainAppearance::getDot3Terrain())
		ClientProceduralTerrainAppearance::setDirectionToLight (mainDirection);

	//-- bounce is opposite of main
	t.yaw_l (PI);
	m_bounceLight->setTransform_o2p (t);

	//-- fill is 45 degrees off of main
	Transform fillTransform;
	fillTransform.yaw_l(180 * PI_OVER_180);
	fillTransform.pitch_l(45 * PI_OVER_180);
	m_fillLight->setTransform_o2p (fillTransform);

	//-- update the movement objects

	if (m_referenceCamera)
	{
		float const zoneSize = TerrainObject::getConstInstance()->getMapWidthInMeters();
		float const cameraMovementPlanetFactor = -(s_cameraMovementPlanetRange / zoneSize);
		
		Vector const & cameraPos_w = m_referenceCamera->getPosition_w();
		
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

	{
		for (ObjectIntMap::iterator it = m_objectIntMap->begin(); it != m_objectIntMap->end(); ++it)
		{
			Object * const obj = it->first;
			int flags = it->second;

			obj->setActive(true);
			if (flags)
			{
				int environmentFlags = ShaderPrimitiveSorter::getClipEnvironmentFlags();

				if ((flags & environmentFlags) == 0)
				{
					obj->setActive(false);
				}
			}

			obj->alter(elapsedTime);
		}
	}

	{
		//-- tell the coulds what day it is
		CloudLayerAppearance::setDay (isDay ());

		//-- update the shadow system with the main light's position
		ShadowVolume::setDirectionToLight (mainDirection);

		//-- update skybox and stars
		{
			Transform transform;
			transform.roll_l (PI_TIMES_2 * m_normalizedTimeRatio);

			const float spill = 0.05f;
			m_currentSkyAlpha = 0.f;

			if (isDay ())
				m_currentSkyAlpha = 1.f;
			else
				if (m_normalizedTimeRatio < (0.5f + spill))
					m_currentSkyAlpha = ::linearInterpolate (1.f, 0.f, (m_normalizedTimeRatio - 0.5f) / spill);
				else
					if (m_normalizedTimeRatio > (1.f - spill))
						m_currentSkyAlpha = ::linearInterpolate (1.f, 0.f, (1.f - m_normalizedTimeRatio) / spill);

			if (m_nightStarsObject)
				m_nightStarsObject->setTransform_o2p (transform);
		}

		//-- update the celestials
		{
			int j;
			for (j = 0; j < m_celestialObjectList->getNumberOfObjects (); ++j)
			{
				CelestialObject* const celestialObject = safe_cast<CelestialObject*> (m_celestialObjectList->getObject (j));
				const Data::CelestialData& celestialData = m_data->m_celestialDataList [j];

				Transform transform;
				transform.yaw_l (celestialData.m_yaw);
				transform.pitch_l (celestialData.m_pitch);

				if (celestialData.m_cycleTime > 0.f)
				{
					celestialData.m_currentTime = fmodf (m_currentTime, celestialData.m_cycleTime);

					const float pitch = celestialData.m_pitchDirection * linearInterpolate (0.f, PI_TIMES_2, celestialData.m_currentTime / celestialData.m_cycleTime);
					transform.pitch_l (pitch);
				}

				celestialObject->setTransform_o2p (transform);
				celestialObject->setActive (true);
				celestialObject->update (elapsedTime, *m_referenceCamera);
			}
		}

		// Faction Cruisers in the sky
		{
			for (unsigned int j = 0; j < m_factionCelestials->size(); ++j)
			{
				CelestialObject* const celestialObject = safe_cast<CelestialObject*> ( (*m_factionCelestials)[j]);

				Transform transform;
				transform.yaw_l (ms_factionYaw[j]);
				transform.pitch_l (ms_factionPitch[j]);

				/*if (celestialData.m_cycleTime > 0.f)
				{
					celestialData.m_currentTime = fmodf (m_currentTime, celestialData.m_cycleTime);

					const float pitch = celestialData.m_pitchDirection * linearInterpolate (0.f, PI_TIMES_2, celestialData.m_currentTime / celestialData.m_cycleTime);
					transform.pitch_l (pitch);
				}*/

				celestialObject->setTransform_o2p (transform);
				celestialObject->setActive (true);
				celestialObject->update (elapsedTime, *m_referenceCamera);
				celestialObject->setAlpha(65.0f);
			}
		}

		//-- update the sun/moon
		{
			if (isDay ())
			{
				if (m_sunCelestialObject)
				{
					if (!m_sunCelestialObject->isActive ())
						m_sunCelestialObject->setActive (true);

					m_sunCelestialObject->setTransform_o2p (m_mainLight->getTransform_o2p ());
					m_sunCelestialObject->update (elapsedTime, *m_referenceCamera);
				}

				if (m_supplementalSunCelestialObject)
				{
					if (!m_supplementalSunCelestialObject->isActive ())
						m_supplementalSunCelestialObject->setActive (true);

					m_supplementalSunCelestialObject->update (elapsedTime, *m_referenceCamera);
				}

				if (m_moonCelestialObject && m_moonCelestialObject->isActive ())
					m_moonCelestialObject->setActive (false);

				if (m_supplementalMoonCelestialObject && m_supplementalMoonCelestialObject->isActive ())
					m_supplementalMoonCelestialObject->setActive (false);
			}
			else
			{
				if (m_sunCelestialObject && m_sunCelestialObject->isActive ())
					m_sunCelestialObject->setActive (false);

				if (m_supplementalSunCelestialObject && m_supplementalSunCelestialObject->isActive ())
					m_supplementalSunCelestialObject->setActive (false);

				if (m_moonCelestialObject)
				{
					if (!m_moonCelestialObject->isActive ())
						m_moonCelestialObject->setActive (true);

					m_moonCelestialObject->setTransform_o2p (m_mainLight->getTransform_o2p ());
					m_moonCelestialObject->update (elapsedTime, *m_referenceCamera);
				}

				if (m_supplementalMoonCelestialObject)
				{
					if (!m_supplementalMoonCelestialObject->isActive ())
						m_supplementalMoonCelestialObject->setActive (true);

					m_supplementalMoonCelestialObject->update (elapsedTime, *m_referenceCamera);
				}
			}
		}

		{
			const float angle = acos (m_referenceCamera->getObjectFrameK_w ().dot (mainDirection));

			//If the camera is in an interior cell (not in the world cell), turn the sun glare off.
			bool cameraNotInWorldCell = (m_referenceCamera && !(m_referenceCamera->isInWorldCell()));
			GameCamera::setDisableGlare (isNight () || cameraNotInWorldCell);			
			static const float PI_OVER_12 = PI_OVER_4 / 3.0f;
			if (angle < PI_OVER_12)
			{
				VectorArgb color = m_mainLight->getDiffuseColor ();
				color.r = clamp (0.f, 2.f * color.r, 1.f);
				color.g = clamp (0.f, 2.f * color.g, 1.f);
				color.b = clamp (0.f, 2.f * color.b, 1.f);
				color.a = (isDay () ? m_sunCelestialObject->getGlowAlpha () : m_moonCelestialObject->getGlowAlpha ()) * (1.f - (angle / PI_OVER_12)) * mainDirection.dot (Vector::unitY);
				GameCamera::setColor (color);
			}
			else
				GameCamera::setColor (VectorArgb (0.0f, 0.0f, 0.0f, 0.0f));
		}
	}

	//-- 
	// the clamp shouldn't really be necessary here but a little extra range-checking won't hurt 
	// since this is just a once-per-frame call.
	m_currentColorIndex = clamp(0, static_cast<int> (256.0f * m_normalizedTimeRatio), 255);

	if (m_previousTopCloudsObject)
		IGNORE_RETURN (m_previousTopCloudsObject->alter (elapsedTime));

	if (m_currentTopCloudsObject)
		IGNORE_RETURN (m_currentTopCloudsObject->alter (elapsedTime));

	if (m_previousBottomCloudsObject)
		IGNORE_RETURN (m_previousBottomCloudsObject->alter (elapsedTime));

	if (m_currentBottomCloudsObject)
		IGNORE_RETURN (m_currentBottomCloudsObject->alter (elapsedTime));

	if (m_transitioning)
	{
		if (m_timer.updateZero (elapsedTime))
			m_transitioning = false;
		else
			apply (m_timer.getElapsedRatio ());
	}
	else
	{
		apply (1.f);

		if (!m_referenceObject->isInWorldCell ())
		{
			const CellProperty* const cellProperty = m_referenceObject->getParentCell ();
			if (cellProperty)
			{
				const PortalProperty* const portalProperty = cellProperty->getPortalProperty ();
				if (portalProperty)
				{
					const char* const cellName = cellProperty->getCellName ();
					const char* const pobShortName = portalProperty->getPobShortName ();
					const InteriorEnvironmentBlock* const desiredInteriorEnvironmentBlock = InteriorEnvironmentBlockManager::getEnvironmentBlock (pobShortName, cellName);
					if (desiredInteriorEnvironmentBlock != m_currentInteriorEnvironmentBlock)
					{
						m_previousInteriorEnvironmentBlock = m_currentInteriorEnvironmentBlock;
						m_currentInteriorEnvironmentBlock  = desiredInteriorEnvironmentBlock;
					}
				}
			}
		}

		//-- ask the terrain system what the current block is
		ClientProceduralTerrainAppearance::EnvironmentData environmentData;
		if (m_clientProceduralTerrainAppearance->findEnvironment (m_referenceObject->getPosition_w (), environmentData))
		{
			EnvironmentBlock const * const desiredEnvironmentBlock = m_environmentBlockManager->getEnvironmentBlock(environmentData.familyId, m_weatherIndex);
			if (desiredEnvironmentBlock != m_currentEnvironmentBlock)
			{
				//-- set up the lerp data
				m_previousEnvironmentBlock = m_currentEnvironmentBlock;
				m_currentEnvironmentBlock  = desiredEnvironmentBlock;

				//-- set the world cell's environment texture
				CellProperty::getWorldCellProperty ()->setEnvironmentTexture (isDay () ? m_currentEnvironmentBlock->getDayEnvironmentTexture () : m_currentEnvironmentBlock->getNightEnvironmentTexture ());

				//-- handle sky
				{
					if (m_currentGradientSkyObject && m_previousGradientSkyObject)
					{
						NOT_NULL (m_previousGradientSkyObject);
						NOT_NULL (m_currentGradientSkyObject);

						Appearance* const previousGradientSkyAppearance = m_previousGradientSkyObject->stealAppearance ();
						delete previousGradientSkyAppearance;

						m_previousGradientSkyObject->setAppearance (m_currentGradientSkyObject->stealAppearance ());
				
						const Texture* const gradientSkyTexture = m_currentEnvironmentBlock->getGradientSkyTexture ();

						if (gradientSkyTexture)
						{
							if (Graphics::getShaderCapability () >= ShaderCapability (1, 1))
								m_currentGradientSkyObject->setAppearance (new GradientSkyAppearance (m_currentEnvironmentBlock->getGradientSkyTextureName ()));
							else
								m_currentGradientSkyObject->setAppearance (new SkyBoxAppearance (m_skyEnvironmentMap));
						}
						else
							m_currentGradientSkyObject->setAppearance (0);
					}
				}

				//-- handle clouds
				{
					//-- bottom
					{
						NOT_NULL (m_previousBottomCloudsObject);
						NOT_NULL (m_currentBottomCloudsObject);

						Appearance* const previousCloudsAppearance = m_previousBottomCloudsObject->stealAppearance ();
						delete previousCloudsAppearance;

						m_previousBottomCloudsObject->setAppearance (m_currentBottomCloudsObject->stealAppearance ());
					
						const char* const cloudFileName = m_currentEnvironmentBlock->getCloudLayerBottomShaderTemplateName ();

						if (cloudFileName && istrlen (cloudFileName))
							m_currentBottomCloudsObject->setAppearance (new CloudLayerAppearance (cloudFileName, m_currentEnvironmentBlock->getCloudLayerBottomShaderSize (), m_currentEnvironmentBlock->getCloudLayerBottomSpeed ()));
						else
							m_currentBottomCloudsObject->setAppearance (0);
					}

					//-- top
					{
						NOT_NULL (m_previousTopCloudsObject);
						NOT_NULL (m_currentTopCloudsObject);

						Appearance* const previousCloudsAppearance = m_previousTopCloudsObject->stealAppearance ();
						delete previousCloudsAppearance;

						m_previousTopCloudsObject->setAppearance (m_currentTopCloudsObject->stealAppearance ());
					
						const char* const cloudFileName = m_currentEnvironmentBlock->getCloudLayerTopShaderTemplateName ();

						if (cloudFileName && istrlen (cloudFileName))
							m_currentTopCloudsObject->setAppearance (new CloudLayerAppearance (cloudFileName, m_currentEnvironmentBlock->getCloudLayerTopShaderSize (), m_currentEnvironmentBlock->getCloudLayerTopSpeed ()));
						else
							m_currentTopCloudsObject->setAppearance (0);
					}

					//-- detach previous object
					{
						Object* const object = m_previousEnvironmentBlock->getCameraAttachedObject ();
						if (object && object->getAttachedTo () != 0)
						{
							object->detachFromObject(Object::DF_none);

							if (object->isInWorld())
								object->removeFromWorld();
						}
					}
				}

				//-- set the timer
				m_timer.setExpireTime (5.f);
				m_transitioning = true;
			}

			//-- handle camera attached appearances
			{
				Object* const object = m_currentEnvironmentBlock->getCameraAttachedObject ();
				if (object)
				{
					//-- attach object if camera is in world cell
					if (m_referenceCamera->isInWorldCell ())
					{
						if (object->getAttachedTo () == 0)
						{
							object->setTransform_o2p (Transform::identity);
							object->attachToObject_p (m_referenceCamera, true);
						}
						else if (object->getAttachedTo() != m_referenceCamera )
						{
							object->detachFromObject(Object::DF_none);

							object->setTransform_o2p (Transform::identity);
							object->attachToObject_p (m_referenceCamera, true);
						}
					}
					else
					{
						if (object->getAttachedTo () != 0)
						{
							object->detachFromObject(Object::DF_none);

							if (object->isInWorld())
								object->removeFromWorld();
						}
					}
				}

			}
		}
	}

#ifdef _DEBUG
	debugDump ();
#endif
	if(m_factionUpdateTimer > ms_factionUpdateFreq)
	{
		updateFactionCelestials();
		m_factionUpdateTimer = 0.0f;
	}


	// Set the time back to it's real value.
	m_currentTime = savedCurrentTime;
	//--
	// updateWaterEnvironmentMap ();
}

//-------------------------------------------------------------------

void GroundEnvironment::draw () const
{
	if (m_enableFog)
	{
		CellProperty* const worldCellProperty = CellProperty::getWorldCellProperty ();
		worldCellProperty->setFogEnabled (m_fogEnabled);
		worldCellProperty->setFogColor (m_fogColor);
		worldCellProperty->setFogDensity (m_fogDensity);
	}

#ifdef _DEBUG
	if (ms_showLightDirections) 
	{
		Transform t;
		t = m_mainLight->getTransform_o2p ();
		t.move_l (Vector::negativeUnitZ * 3.f);
		t.yaw_l (PI);
		ShaderPrimitiveSorter::getCurrentCamera ().addDebugPrimitive (new FrameDebugPrimitive  (FrameDebugPrimitive::S_none,  t, 1.f));

		t = m_fillLight->getTransform_o2p ();
		t.move_l (Vector::negativeUnitZ * 3.f);
		t.yaw_l (PI);
		ShaderPrimitiveSorter::getCurrentCamera ().addDebugPrimitive (new FrameDebugPrimitive  (FrameDebugPrimitive::S_none,  t, 1.f));

		t = m_bounceLight->getTransform_o2p ();
		t.move_l (Vector::negativeUnitZ * 3.f);
		t.yaw_l (PI);
		ShaderPrimitiveSorter::getCurrentCamera ().addDebugPrimitive (new FrameDebugPrimitive  (FrameDebugPrimitive::S_none,  t, 1.f));
	}
#endif
}

//-------------------------------------------------------------------

#ifdef _DEBUG

void GroundEnvironment::debugDump () const
{
	if (ms_debugReport)
	{
		DEBUG_REPORT_PRINT (true, ("currentSkyAlpha           = %1.2f\n", m_currentSkyAlpha));
		DEBUG_REPORT_PRINT (true, ("weatherIndex              = %i\n", m_weatherIndex));

		Object* const object = m_currentEnvironmentBlock->getCameraAttachedObject ();
		if (object && object->getAppearance () && object->getAppearance ()->getAppearanceTemplate ())
			DEBUG_REPORT_PRINT (true, ("camera attached object    = %s\n", object->getAppearance ()->getAppearanceTemplate ()->getCrcName ().getString ()));
		else
			DEBUG_REPORT_PRINT (true, ("camera attached object    = none\n"));
	}
}

#endif

//-------------------------------------------------------------------

void GroundEnvironment::updateWaterEnvironmentMap ()
{
	const Texture* const texture = m_currentEnvironmentBlock->getGradientSkyTexture ();
	if (texture)
	{
		//-- copy the strip from the environment texture
		PackedRgb colorRamp [ms_skyEnvironmentTextureSize];

		{
			TextureGraphicsData::LockData lockData (TF_ARGB_8888, 0, 0, 0, texture->getWidth (), texture->getHeight (), false);
			texture->lockReadOnly (lockData);

				const int strip = static_cast<int> (linearInterpolate (0, lockData.getWidth () - 1, m_normalizedTimeRatio));
				const uint8* const pixelData = reinterpret_cast<const uint8*> (lockData.getPixelData ());
				const int dy = lockData.getHeight () / ms_skyEnvironmentTextureSize;

				int y;
				for (y = 0; y < ms_skyEnvironmentTextureSize; ++y)
				{
					const uint8* destination = pixelData + (y * dy * lockData.getPitch ()) + (strip * 4);

					colorRamp [y].b = *destination++;
					colorRamp [y].g = *destination++;
					colorRamp [y].r = *destination++;
				}

			texture->unlock (lockData);
		}

		//-- scale the color ramp
		{
			int i;
			for (i = 0; i < ms_skyEnvironmentTextureSize; ++i)
				colorRamp [i] = PackedRgb::linearInterpolate (colorRamp [i], PackedRgb::solidBlack, 0.25f);
		}

		//-- create +y
		{
			Texture::LockData lockData (TF_ARGB_8888, CF_positiveY, 0, 0, 0, 0, ms_skyEnvironmentTextureSize, ms_skyEnvironmentTextureSize, true);
			m_skyEnvironmentMap->lock (lockData);

				uint8* const pixelData = reinterpret_cast<uint8*> (lockData.getPixelData ());

				const PackedRgb& color = colorRamp [0];

				int y;
				for (y = 0; y < ms_skyEnvironmentTextureSize; ++y)
				{
					uint8* destination = pixelData + (y * lockData.getPitch ());

					int x;
					for (x = 0; x < ms_skyEnvironmentTextureSize; ++x)
					{
						*destination++ = color.b;
						*destination++ = color.g;
						*destination++ = color.r;
						*destination++ = 255;
					}
				}

			m_skyEnvironmentMap->unlock (lockData);
		}

		//-- create -y
		{
			Texture::LockData lockData (TF_ARGB_8888, CF_negativeY, 0, 0, 0, 0, ms_skyEnvironmentTextureSize, ms_skyEnvironmentTextureSize, true);
			m_skyEnvironmentMap->lock (lockData);

				uint8* const pixelData = reinterpret_cast<uint8*> (lockData.getPixelData ());

				const PackedRgb& color = colorRamp [ms_skyEnvironmentTextureSize - 1];

				int y;
				for (y = 0; y < ms_skyEnvironmentTextureSize; ++y)
				{
					uint8* destination = pixelData + (y * lockData.getPitch ());

					int x;
					for (x = 0; x < ms_skyEnvironmentTextureSize; ++x)
					{
						*destination++ = color.b;
						*destination++ = color.g;
						*destination++ = color.r;
						*destination++ = 255;
					}
				}

			m_skyEnvironmentMap->unlock (lockData);
		}

		//-- create +x, -x, +z, -z
		{
			int i;
			for (i = 0; i < CF_none; ++i)
			{
				if (i == CF_positiveY || i == CF_negativeY)
					continue;

				Texture::LockData lockData (TF_ARGB_8888, static_cast<CubeFace> (i), 0, 0, 0, 0, ms_skyEnvironmentTextureSize, ms_skyEnvironmentTextureSize, true);
				m_skyEnvironmentMap->lock (lockData);

					uint8* const pixelData = reinterpret_cast<uint8*> (lockData.getPixelData ());

					int y;
					for (y = 0; y < ms_skyEnvironmentTextureSize / 2; ++y)
					{
						const float dy = sqr (static_cast<float> (y) / (ms_skyEnvironmentTextureSize / 2));
						const PackedRgb& color = colorRamp [static_cast<int> (dy * ms_skyEnvironmentTextureSize)];

						uint8* destination = pixelData + (y * lockData.getPitch ());

						int x;
						for (x = 0; x < ms_skyEnvironmentTextureSize; ++x)
						{
							*destination++ = color.b;
							*destination++ = color.g;
							*destination++ = color.r;
							*destination++ = 255;
						}
					}

					for (y = ms_skyEnvironmentTextureSize / 2; y < ms_skyEnvironmentTextureSize; ++y)
					{
						const PackedRgb& color = colorRamp [ms_skyEnvironmentTextureSize - 1];

						uint8* destination = pixelData + (y * lockData.getPitch ());

						int x;
						for (x = 0; x < ms_skyEnvironmentTextureSize; ++x)
						{
							*destination++ = color.b;
							*destination++ = color.g;
							*destination++ = color.r;
							*destination++ = 255;
						}
					}

				m_skyEnvironmentMap->unlock (lockData);
			}
		}

		Graphics::setGlobalTexture (TAG__SKY, *m_skyEnvironmentMap);
	}
	else
	{
		for (uint i = 0; i < CF_none; ++i)
		{
			Texture::LockData lockData (TF_ARGB_8888, static_cast<CubeFace> (i), 0, 0, 0, 0, 1, 1, true);
			m_clearColorTexture->lock (lockData);

				uint8* const pixelData = reinterpret_cast<uint8*> (lockData.getPixelData ());
				uint8* destination = pixelData;
				
				*destination++ = m_clearColor.b;
				*destination++ = m_clearColor.g;
				*destination++ = m_clearColor.r;
				*destination++ = 255;

			m_clearColorTexture->unlock (lockData);
		}

		Graphics::setGlobalTexture (TAG__SKY, *m_clearColorTexture);
	}
}

//----------------------------------------------------------------------

GroundEnvironment & GroundEnvironment::getInstance()
{
	if (!ms_instance) 
	{
		ms_instance = new GroundEnvironment;
	}

	return *ms_instance;
}

//----------------------------------------------------------------------

void GroundEnvironment::setClientProceduralTerrainAppearance(ClientProceduralTerrainAppearance * const clientProceduralTerrainAppearance, float const dayCycleTime)
{
	if (clientProceduralTerrainAppearance) 
	{
		RenderWorld::setClearDepthAndStencilBufferAfterRenderingEnvironment(false);

		m_cycleTime = dayCycleTime;

		setWeatherIndex(0);

		FileName fileName(clientProceduralTerrainAppearance->getAppearanceTemplate ()->getName ());
		fileName.stripPathAndExt ();

		char environmentDataFileName [1024];
		sprintf (environmentDataFileName, "terrain/environment/%s.iff", fileName.getString());
		m_data->load (environmentDataFileName);

		char environmentTableFileName [1024];
		sprintf (environmentTableFileName, "datatables/environment/%s.iff", fileName.getString());
		m_environmentBlockManager = new EnvironmentBlockManager (&safe_cast<const ProceduralTerrainAppearanceTemplate*> (clientProceduralTerrainAppearance->getAppearanceTemplate ())->getTerrainGenerator ()->getEnvironmentGroup (), environmentTableFileName);
		m_currentEnvironmentBlock = m_previousEnvironmentBlock = m_environmentBlockManager->getDefaultEnvironmentBlock ();

		m_currentInteriorEnvironmentBlock = m_previousInteriorEnvironmentBlock = InteriorEnvironmentBlockManager::getDefaultEnvironmentBlock ();

		//-- SkyBox
		{
			bool const cubeMap = m_data->m_skyBoxCubeMap;
			std::string const & textureNameMask = m_data->m_skyBoxTextureNameMask;

			if (!textureNameMask.empty())
			{
				m_skyBoxObject = new Object;
				m_skyBoxObject->setDebugName("skybox");

				if (cubeMap)
				{
					Texture const * const texture = TextureList::fetch(textureNameMask.c_str());
					m_skyBoxObject->setAppearance(new SkyBoxAppearance(texture, 1.f));
					texture->release();
				}
				else
				{
					m_skyBoxObject->setAppearance(new SkyBox6SidedAppearance(textureNameMask.c_str()));
				}

				RenderWorld::addWorldEnvironmentObject(m_skyBoxObject);
			}
		}
		
		if (!m_skyBoxObject)
		{
			//-- create gradient skies
			{
				m_previousGradientSkyObject = new Object ();
				m_previousGradientSkyObject->setDebugName ("previous sky");
				m_previousGradientSkyObject->setScale (Vector::xyz111 * 10.f);
				RenderWorld::addWorldEnvironmentObject(m_previousGradientSkyObject);
				
				m_currentGradientSkyObject = new Object ();
				m_currentGradientSkyObject->setDebugName ("current sky");
				m_previousGradientSkyObject->setScale (Vector::xyz111 * 10.f);
				RenderWorld::addWorldEnvironmentObject(m_currentGradientSkyObject);
			}
		}


		//-- create stars
		if (m_data->m_numberOfStars > 0)
		{
			m_nightStarsObject = new Object ();
			m_nightStarsObject->setDebugName ("stars");
			m_nightStarsObject->setAppearance (new StarAppearance (m_data->m_starColorRampFileName.c_str (), m_data->m_numberOfStars, true, 0.75f));
			RenderWorld::addWorldEnvironmentObject(m_nightStarsObject);
		}


		//-- create misc celestial objects
		if (m_data->m_celestialDataList.size ())
		{
			uint i;
			for (i = 0; i < m_data->m_celestialDataList.size (); ++i)
			{
				const Data::CelestialData& celestialData = m_data->m_celestialDataList [i];

				CelestialObject* const celestialObject = new CelestialObject (celestialData.m_shaderTemplateName.c_str (), celestialData.m_size, celestialData.m_glowShaderTemplateName.c_str (), celestialData.m_glowSize);
				RenderWorld::addWorldEnvironmentObject (celestialObject);

				m_celestialObjectList->addObject (celestialObject);
			}
		}

		//-- create sun object
		{
			m_sunCelestialObject = new CelestialObject (m_data->m_sunShaderTemplateName.c_str (), m_data->m_sunSize, m_data->m_sunGlowShaderTemplateName.c_str (), m_data->m_sunGlowSize);
			RenderWorld::addWorldEnvironmentObject (m_sunCelestialObject);

			if (*m_data->m_supplementalSunShaderTemplateName.c_str ())
			{
				m_supplementalSunCelestialObject = new CelestialObject (m_data->m_supplementalSunShaderTemplateName.c_str (), m_data->m_supplementalSunSize, m_data->m_supplementalSunGlowShaderTemplateName.c_str (), m_data->m_supplementalSunGlowSize);
				m_supplementalSunCelestialObject->yaw_o (m_data->m_supplementalSunYaw);
				m_supplementalSunCelestialObject->pitch_o (m_data->m_supplementalSunPitch);
				m_supplementalSunCelestialObject->attachToObject_p (m_sunCelestialObject, false);
				RenderWorld::addWorldEnvironmentObject (m_supplementalSunCelestialObject);
			}
		}

		//-- create moon object
		{
			m_moonCelestialObject = new CelestialObject (m_data->m_moonShaderTemplateName.c_str (), m_data->m_moonSize, m_data->m_moonGlowShaderTemplateName.c_str (), m_data->m_moonGlowSize);
			RenderWorld::addWorldEnvironmentObject (m_moonCelestialObject);

			if (*m_data->m_supplementalMoonShaderTemplateName.c_str ())
			{
				m_supplementalMoonCelestialObject = new CelestialObject (m_data->m_supplementalMoonShaderTemplateName.c_str (), m_data->m_supplementalMoonSize, m_data->m_supplementalMoonGlowShaderTemplateName.c_str (), m_data->m_supplementalMoonGlowSize);
				m_supplementalMoonCelestialObject->yaw_o (m_data->m_supplementalMoonYaw);
				m_supplementalMoonCelestialObject->pitch_o (m_data->m_supplementalMoonPitch);
				m_supplementalMoonCelestialObject->attachToObject_p (m_moonCelestialObject, false);
				RenderWorld::addWorldEnvironmentObject (m_supplementalMoonCelestialObject);
			}
		}

		//-- DistantAppearances
		{

			int const numberOfDistantAppearances = m_data->m_distantAppearances.size();

			for (int i = 0; i < numberOfDistantAppearances; ++i)
			{
				Data::DistantAppearanceData const & dad = m_data->m_distantAppearances[i];

				Vector const & direction_w = dad.m_direction_w;
				Vector const & orientation_w = dad.m_orientation_w;

				bool const isInfiniteDistance = dad.m_infiniteDistance;

				Object * const object = new Object();
				object->setAppearance(AppearanceTemplateList::createAppearance(dad.m_appearanceTemplateName.getString()));
				object->setPosition_p(direction_w);
				object->yaw_o(orientation_w.x);
				object->pitch_o(orientation_w.y);
				object->roll_o(orientation_w.z);

				PlanetAppearance * const planetAppearance = dynamic_cast<PlanetAppearance *>(object->getAppearance());
				if (planetAppearance)
				{
					float const haloRoll = dad.m_haloRoll;
					if (haloRoll > 0.f)
						planetAppearance->setHaloRoll(haloRoll);

					//-- halo scaling is now handled automagically by the code at runtime

					float const planetFaceDistance = direction_w.magnitude() - object->getAppearanceSphereRadius();
					if (planetFaceDistance < s_cameraMovementPlanetRange * 0.5f)
					{
						WARNING(true, ("GroundEnvironment planet [%s] is too close [%f] to camera.", dad.m_appearanceTemplateName.getString(), planetFaceDistance));
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

				m_objectIntMap->insert(std::make_pair(parent, dad.m_flag));
			}
		}


		//-- create clouds
		m_previousTopCloudsObject = new Object ();
		m_previousTopCloudsObject->setDebugName ("previous top clouds");
		RenderWorld::addWorldEnvironmentObject(m_previousTopCloudsObject);

		m_currentTopCloudsObject = new Object ();
		m_currentTopCloudsObject->setDebugName ("current top clouds");
		RenderWorld::addWorldEnvironmentObject(m_currentTopCloudsObject);

		m_previousBottomCloudsObject = new Object ();
		m_previousBottomCloudsObject->setDebugName ("previous bottom clouds");
		RenderWorld::addWorldEnvironmentObject(m_previousBottomCloudsObject);

		m_currentBottomCloudsObject = new Object ();
		m_currentBottomCloudsObject->setDebugName ("current bottom clouds");
		RenderWorld::addWorldEnvironmentObject(m_currentBottomCloudsObject);

		//-- create ambient light
		m_ambientLight = new Light (Light::T_ambient, VectorArgb::solidBlack);
		m_ambientLight->setDebugName ("ambient light");
		m_ambientLight->addToWorld ();
		RenderWorld::addWorldEnvironmentLight(m_ambientLight);

		//-- create main light
		m_mainLight = new Light (Light::T_parallel, VectorArgb::solidBlack);
		m_mainLight->setDebugName ("main light");
		m_mainLight->addToWorld ();
		RenderWorld::addWorldEnvironmentLight(m_mainLight);

		//-- create fill light
		m_fillLight = new Light (Light::T_parallel, VectorArgb::solidBlack);
		m_fillLight->setDebugName ("fill light");
		m_fillLight->addToWorld ();
		RenderWorld::addWorldEnvironmentLight(m_fillLight);

		//-- create bounce light
		m_bounceLight = new Light (Light::T_parallel, VectorArgb::solidBlack);
		m_bounceLight->setDebugName ("bounce light");
		m_bounceLight->addToWorld ();
		RenderWorld::addWorldEnvironmentLight(m_bounceLight);

		if (Graphics::getShaderCapability () >= ShaderCapability(1,1) && ClientProceduralTerrainAppearance::getDot3Terrain () && !ConfigClientTerrain::getDisableTerrainClouds ())
			m_terrainCloudShader = ShaderTemplateList::fetchShader ("shader/terrain_cloudtile.sht");

		const TextureFormat runtimeFormats [] = { TF_ARGB_8888 };
		const int numberOfRuntimeFormats = sizeof (runtimeFormats) / sizeof (runtimeFormats [0]);
		m_skyEnvironmentMap = TextureList::fetch (TCF_cubeMap, ms_skyEnvironmentTextureSize, ms_skyEnvironmentTextureSize, 1, runtimeFormats, numberOfRuntimeFormats);
		m_clearColorTexture = TextureList::fetch (TCF_cubeMap, 1, 1, 1, runtimeFormats, numberOfRuntimeFormats);

		if (m_data->m_timeLocked)
			setTime(m_data->m_lockTime, true);
	}


	m_clientProceduralTerrainAppearance = clientProceduralTerrainAppearance;
}

//----------------------------------------------------------------------

void GroundEnvironment::destroyInstance()
{
	delete ms_instance;
	ms_instance = 0;
}

//----------------------------------------------------------------------

ClientProceduralTerrainAppearance * GroundEnvironment::getClientProceduralTerrainAppearance()
{
	return m_clientProceduralTerrainAppearance;
}

//----------------------------------------------------------------------

void GroundEnvironment::updateFactionCelestials()
{
	if(!GuildObject::getGuildObject())
		return;
	
	std::string const & currentPlanet = Game::getSceneId();
	std::string planetKeyValue = currentPlanet;

	if(planetKeyValue.find("yavin") != std::string::npos)
		planetKeyValue = "yavin4";
	else
		planetKeyValue = Unicode::toLower(planetKeyValue);
	

	std::map<std::string, int> const & thisGalaxy = GuildObject::getGuildObject()->getGcwGroupImperialScorePercentileThisGalaxy();
	std::map<std::string, int>::const_iterator findIter = thisGalaxy.find(planetKeyValue);
	if(findIter == thisGalaxy.end())
	{
		return;
	}

	int imperialScore = findIter->second;
	int rebelScore = 100 - imperialScore;

	bool resetClouds = false;

	static int impAdvantage = 0;
	static int rebAdvantage = 0;

	imperialScore += impAdvantage;
	rebelScore += rebAdvantage;

	if(imperialScore > rebelScore)
	{
		unsigned int level = imperialScore / 10;
		if(m_factionStanding == 1 && m_factionOccupationLevel == level) // Already setup or the change wasn't enough, bail out.
			return;
		else if (m_factionStanding != 1 || m_factionOccupationLevel != level)
		{
			cleanUpFactionCelestials();
			for(unsigned int i = 0; i < level / 2; ++i)
			{
				CelestialObject* const celestialObject = new CelestialObject (ms_starDestroyerShader, ms_factionSize[i], "", 0);
				RenderWorld::addWorldEnvironmentObject (celestialObject);

				m_factionCelestials->push_back (celestialObject);
			}

			resetClouds = true;

		}

		m_factionStanding = 1;
		m_factionOccupationLevel = level;
	}
	else if (rebelScore > imperialScore)
	{
		unsigned int level = rebelScore / 10;
		if(m_factionStanding == 2 && m_factionOccupationLevel == level) // Already setup or the change wasn't enough, bail out.
			return;
		else if (m_factionStanding != 2 || m_factionOccupationLevel != level)
		{
			cleanUpFactionCelestials();
			for(unsigned int i = 0; i < level / 2; ++i)
			{
				CelestialObject* const celestialObject = new CelestialObject (ms_moncalCruiserShader, ms_factionSize[i], "", 0);
				RenderWorld::addWorldEnvironmentObject (celestialObject);

				m_factionCelestials->push_back (celestialObject);
			}

			resetClouds = true;
		}

		m_factionStanding = 2;
		m_factionOccupationLevel = level;
	}
	else
	{
		m_factionStanding = 0;
		m_factionOccupationLevel = 0;
		cleanUpFactionCelestials();
	}

	if(resetClouds)
	{
		if(m_previousTopCloudsObject)
		{
			RenderWorld::removeWorldEnvironmentObject(m_previousTopCloudsObject);
			RenderWorld::addWorldEnvironmentObject(m_previousTopCloudsObject);
		}

		if(m_currentTopCloudsObject)
		{
			RenderWorld::removeWorldEnvironmentObject(m_currentTopCloudsObject);
			RenderWorld::addWorldEnvironmentObject(m_currentTopCloudsObject);
		}

		if(m_previousBottomCloudsObject)
		{
			RenderWorld::removeWorldEnvironmentObject(m_previousBottomCloudsObject);
			RenderWorld::addWorldEnvironmentObject(m_previousBottomCloudsObject);
		}

		if(m_currentBottomCloudsObject)
		{
			RenderWorld::removeWorldEnvironmentObject(m_currentBottomCloudsObject);
			RenderWorld::addWorldEnvironmentObject(m_currentBottomCloudsObject);
		}
	}

}

//===================================================================

