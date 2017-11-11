//===================================================================
//
// GroundEnvironment.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_GroundEnvironment_H
#define INCLUDED_GroundEnvironment_H

//===================================================================

class ClientProceduralTerrainAppearance;
class CelestialObject;
class EnvironmentBlock;
class EnvironmentBlockManager;
class InteriorEnvironmentBlock;
class InteriorEnvironmentBlockManager;
class Light;
class Object;
class Texture;
class Camera;
class Shader;
class ObjectList;

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "sharedMath/Transform.h"
#include "sharedMath/VectorArgb.h"
#include "sharedTerrain/ColorRamp256.h"

//===================================================================

class GroundEnvironment
{
public:
	static void install();
	static void remove();

	static GroundEnvironment & getInstance();
	static void destroyInstance();

	void setReferenceCamera(Camera * referenceCamera);
	void setReferenceObject(Object const * referenceObject);
	void setWeatherIndex(int weatherIndex);
	void setEnableFog(bool enableFog);
	bool getEnableFog();

	void setClientProceduralTerrainAppearance(ClientProceduralTerrainAppearance * const clientProceduralTerrainAppearance, float const dayCycleTime);
	ClientProceduralTerrainAppearance * getClientProceduralTerrainAppearance();

	void alter(float elapsedTime);
	void draw() const;

	Shader const * getTerrainCloudShader () const;
	EnvironmentBlock const * getCurrentEnvironmentBlock () const;
	InteriorEnvironmentBlock const * getCurrentInteriorEnvironmentBlock () const;

	bool getPaused () const;
	void setPaused (bool paused);

	const PackedRgb getClearColor () const;
	const PackedRgb getFogColor () const;
	float getFogDensity () const;
	int   getHour () const;
	int   getMinute () const;
	float getTime () const;
	void  setTime (float time, bool force);
	bool  isDay () const;
	bool  isNight () const;
	bool isTimeLocked() const;

	void updateWaterEnvironmentMap();

	Light & getMainLight();

private:
	GroundEnvironment ();
	~GroundEnvironment ();
	GroundEnvironment (const GroundEnvironment&);
	GroundEnvironment& operator= (const GroundEnvironment&);

	void        apply (float t);
	void        updateFactionCelestials();
	void        cleanUpFactionCelestials();

#ifdef _DEBUG
	void        debugDump () const;
#endif

	void destroyObject();

	Camera*                           m_referenceCamera;
	const Object*                     m_referenceObject;
	int                               m_weatherIndex;
	bool                              m_enableFog;

private:

	ClientProceduralTerrainAppearance * m_clientProceduralTerrainAppearance;

	class Data;
	Data *                                   m_data;
	EnvironmentBlockManager*                 m_environmentBlockManager;
	const EnvironmentBlock*                  m_currentEnvironmentBlock;
	const EnvironmentBlock*                  m_previousEnvironmentBlock;
	Timer                                    m_timer;
	bool                                     m_transitioning;
	Object*                                  m_previousGradientSkyObject;
	Object*                                  m_currentGradientSkyObject;
	Object*                                  m_nightStarsObject;
	Object*                                  m_previousBottomCloudsObject;
	Object*                                  m_currentBottomCloudsObject;
	Object*                                  m_previousTopCloudsObject;
	Object*                                  m_currentTopCloudsObject;
	Object*                                  m_skyBoxObject;
	Light*                                   m_ambientLight;
	Light*                                   m_mainLight;
	float                                    m_mainYaw;
	Light*                                   m_fillLight;
	float                                    m_fillYaw;
	Light*                                   m_bounceLight;
	float                                    m_bounceYaw;
	bool                                     m_fogEnabled;
	VectorArgb                               m_fogColor;
	float                                    m_fogDensity;
	bool                                     m_paused;
	float                                    m_currentTime;
	float                                    m_cycleTime;
	float                                    m_timeRatio;
	float                                    m_normalizedTimeRatio;
	int                                      m_currentColorIndex;
	PackedRgb                                m_clearColor;
	int                                      m_hour;
	int                                      m_minute;
	const Shader*                            m_terrainCloudShader;
	CelestialObject*                         m_sunCelestialObject;
	CelestialObject*                         m_supplementalSunCelestialObject;
	CelestialObject*                         m_moonCelestialObject;
	CelestialObject*                         m_supplementalMoonCelestialObject;
	float                                    m_currentSkyAlpha;
	ObjectList* const                        m_celestialObjectList;

	const InteriorEnvironmentBlock*          m_currentInteriorEnvironmentBlock;
	const InteriorEnvironmentBlock*          m_previousInteriorEnvironmentBlock;

	Texture*                                 m_skyEnvironmentMap;
	Texture*                                 m_clearColorTexture;

	typedef stdmap<Object *, int>::fwd ObjectIntMap;
	typedef stdvector<Object *>::fwd ObjectVector;
	ObjectIntMap * m_objectIntMap;
	ObjectVector * m_objectsFollowCameraVector;

	typedef stdvector<CelestialObject *>::fwd CelestialObjectVector;
	CelestialObjectVector * m_factionCelestials;
	int					  m_factionStanding; // 0 = neutral, 1 = imperial, 2 = rebel.
	unsigned int		  m_factionOccupationLevel; // 1 - 5, how many ships we'll spawn. (2 - 10).
	float				  m_factionUpdateTimer;

};

//----------------------------------------------------------------------

inline Light & GroundEnvironment::getMainLight()
{
	return *m_mainLight;
}

//===================================================================

#endif
