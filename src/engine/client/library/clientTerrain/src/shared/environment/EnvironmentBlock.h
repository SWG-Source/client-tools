//===================================================================
//
// EnvironmentBlock.h
// asommers
//
// copyright 2002, sony online entertainment
// 
//===================================================================

#ifndef INCLUDED_EnvironmentBlock_H
#define INCLUDED_EnvironmentBlock_H

//===================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedMath/PackedRgb.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/Object.h"

class CrcString;
class Object;
class PersistentCrcString;
class Shader;
class Texture;

//===================================================================

class EnvironmentBlockData
{
public:

	EnvironmentBlockData ();

public:

	const char* m_name;
	int         m_familyId;
	int         m_weatherIndex;
	const char* m_gradientSkyTextureName;
	const char* m_cloudLayerBottomShaderTemplateName;
	float       m_cloudLayerBottomShaderSize;
	float       m_cloudLayerBottomSpeed;
	const char* m_cloudLayerTopShaderTemplateName;
	float       m_cloudLayerTopShaderSize;
	float       m_cloudLayerTopSpeed;
	const char* m_colorRampFileName;
	bool        m_shadowsEnabled;
	bool        m_fogEnabled;
	float       m_minimumFogDensity;
	float       m_maximumFogDensity;
	const char* m_cameraAppearanceTemplateName;
	const char* m_dayEnvironmentTextureName;
	const char* m_nightEnvironmentTextureName;
	const char* m_day1AmbientSoundTemplateName;
	const char* m_day2AmbientSoundTemplateName;
	const char* m_night1AmbientSoundTemplateName;
	const char* m_night2AmbientSoundTemplateName;
	const char* m_firstMusicSoundTemplateName;
	const char* m_sunriseMusicSoundTemplateName;
	const char* m_sunsetMusicSoundTemplateName;
	float       m_windSpeedScale;
};

//===================================================================

class EnvironmentBlock
{
public:

	EnvironmentBlock ();
	~EnvironmentBlock ();

	void                  setData (const EnvironmentBlockData& data);

	const char*           getName () const;
	int                   getFamilyId () const;
	int                   getWeatherIndex () const;
	const char*           getGradientSkyTextureName () const;
	const Texture*        getGradientSkyTexture () const;
	const char*           getCloudLayerBottomShaderTemplateName () const;
	float                 getCloudLayerBottomShaderSize () const;
	float                 getCloudLayerBottomSpeed () const;
	const char*           getCloudLayerTopShaderTemplateName () const;
	float                 getCloudLayerTopShaderSize () const;
	float                 getCloudLayerTopSpeed () const;
	int                   getNumberOfRampColors () const;
	const PackedRgb*      getAmbientColorRamp () const;
	const float*          getCelestialAlphaRamp () const;
	const PackedRgb*      getMainDiffuseColorRamp () const;
	const float*          getMainDiffuseColorScale () const;
	const PackedRgb*      getMainSpecularColorRamp () const;
	const float*          getMainSpecularColorScale () const;
	const PackedRgb*      getFillColorRamp () const;
	const float*          getFillColorScale () const;
	const PackedRgb*      getBounceColorRamp () const;
	const float*          getBounceColorScale () const;
	const PackedRgb*      getClearColorRamp () const;
	const float*          getSunMoonAlphaRamp () const;
	bool                  getShadowsEnabled () const;
	bool                  getFogEnabled () const;
	const PackedRgb*      getFogColorRamp () const;
	const float*          getStarAlphaRamp () const;
	float                 getMinimumFogDensity () const;
	float                 getMaximumFogDensity () const;
	const VectorArgb*     getShadowRamp () const;
	const PackedRgb*      getMainBackColorRamp () const;
	const float*          getMainBackColorScale () const;
	const PackedRgb*      getMainTangentColorRamp () const;
	const float*          getMainTangentColorScale () const;
	Object*               getCameraAttachedObject () const;
	const Texture*        getDayEnvironmentTexture () const;
	const Texture*        getNightEnvironmentTexture () const;
	const CrcString*      getDay1AmbientSoundTemplateName () const;
	const CrcString*      getDay2AmbientSoundTemplateName () const;
	const CrcString*      getNight1AmbientSoundTemplateName () const;
	const CrcString*      getNight2AmbientSoundTemplateName () const;
	const CrcString*      getFirstMusicSoundTemplateName () const;
	const CrcString*      getSunriseMusicSoundTemplateName () const;
	const CrcString*      getSunsetMusicSoundTemplateName () const;
	float                 getWindSpeedScale () const;

private:

	void                  loadColorRamps (const char* fileName);
	void                  clear ();

private:

	EnvironmentBlock (const EnvironmentBlock&);
	EnvironmentBlock& operator= (const EnvironmentBlock&);

private:

	char*                 m_name;
	int                   m_familyId;
	int                   m_weatherIndex;
	char*                 m_gradientSkyTextureName;
	const Texture*        m_gradientSkyTexture;
	char*                 m_cloudLayerBottomShaderTemplateName;
	float                 m_cloudLayerBottomShaderSize;
	float                 m_cloudLayerBottomSpeed;
	char*                 m_cloudLayerTopShaderTemplateName;
	float                 m_cloudLayerTopShaderSize;
	float                 m_cloudLayerTopSpeed;
	PackedRgb             m_ambientColorRamp [256];
	float                 m_celestialAlphaRamp [256];
	PackedRgb             m_mainDiffuseColorRamp [256];
	float                 m_mainDiffuseColorScale [256];
	PackedRgb             m_mainSpecularColorRamp [256];
	float                 m_mainSpecularColorScale [256];
	PackedRgb             m_fillColorRamp [256];
	float                 m_fillColorScale [256];
	PackedRgb             m_bounceColorRamp [256];
	float                 m_bounceColorScale [256];
	PackedRgb             m_clearColorRamp [256];
	float                 m_sunMoonAlphaRamp [256];
	bool                  m_shadowsEnabled;
	bool                  m_fogEnabled;
	PackedRgb             m_fogColorRamp [256];
	float                 m_starAlphaRamp [256];
	float                 m_minimumFogDensity;
	float                 m_maximumFogDensity;
	VectorArgb            m_shadowRamp [256];
	PackedRgb             m_mainBackColorRamp [256];
	float                 m_mainBackColorScale [256];
	PackedRgb             m_mainTangentColorRamp [256];
	float                 m_mainTangentColorScale [256];
	Watcher<Object>       m_cameraAttachedObject;
	const Texture*        m_dayEnvironmentTexture;
	const Texture*        m_nightEnvironmentTexture;
	PersistentCrcString* const m_day1AmbientSoundTemplateName;
	PersistentCrcString* const m_day2AmbientSoundTemplateName;
	PersistentCrcString* const m_night1AmbientSoundTemplateName;
	PersistentCrcString* const m_night2AmbientSoundTemplateName;
	PersistentCrcString* const m_firstMusicSoundTemplateName;
	PersistentCrcString* const m_sunriseMusicSoundTemplateName;
	PersistentCrcString* const m_sunsetMusicSoundTemplateName;
	float                 m_windSpeedScale;
};

//===================================================================

#endif
