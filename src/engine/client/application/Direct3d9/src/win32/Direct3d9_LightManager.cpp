// ======================================================================
//
// Direct3d9_LightManager.cpp
//
// Portions Copyright 1999 Bootprint Entertainment
// Portions Copyright 2000-2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "FirstDirect3d9.h"
#include "Direct3d9_LightManager.h"

#include "Direct3d9.h"
#include "Direct3d9_PixelShaderConstantRegisters.h"
#include "Direct3d9_StateCache.h"
#include "Direct3d9_VertexShaderConstantRegisters.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/ShaderCapability.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/Production.h"
#include "sharedMath/VectorRgba.h"

#include <limits>
#include <vector>
#include <algorithm>

#ifdef FIELD_OFFSET
#undef FIELD_OFFSET
#endif
#define FIELD_OFFSET(type, field)    ((int)&(((type *)0)->field))

// ======================================================================

bool                                        Direct3d9_LightManager::ms_dirty;
bool                                        Direct3d9_LightManager::ms_usingVertexShaderProgram;
#if PRODUCTION == 0
bool                                        Direct3d9_LightManager::ms_useOnlyfullAmbient;
bool                                        Direct3d9_LightManager::ms_setLightsOncePerFrame;
bool                                        Direct3d9_LightManager::ms_newFrame;
#endif
bool                                        Direct3d9_LightManager::ms_obeysLightScale;
Transform                                   Direct3d9_LightManager::ms_objectToWorldTransform;
Vector                                      Direct3d9_LightManager::ms_objectPosition;
Vector                                      Direct3d9_LightManager::ms_cameraPosition;
Direct3d9_LightManager::LightList           Direct3d9_LightManager::ms_lightList;
VectorRgba                                  Direct3d9_LightManager::ms_fullAmbient;
Direct3d9_LightManager::SelectedLights      Direct3d9_LightManager::ms_currentLights;
Direct3d9_LightManager::SelectedLights      Direct3d9_LightManager::ms_lastLights;
const Light                                *Direct3d9_LightManager::ms_fixedFunctionPipelineLight[FixedFunctionPipelineLightCount];

const VectorRgba                            Direct3d9_LightManager::ms_noDot3Light[3] = { VectorRgba(0.0f, 0.0f, 0.0f, 0.0f), VectorRgba(0.0f, 0.0f, 0.0f, 0.0f), VectorRgba(0.0f, 0.0f, 0.0f, 0.0f) };

namespace Direct3d9Namespace
{
	extern bool ms_alphaFadeOpacityDirty;
	extern float ms_currentTime;
}

namespace Direct3d9_LightManagerNamespace
{
	float              (Light::*getPossiblyScaledDiffuseIntensity)() const;
	const VectorArgb & (Light::*getPossiblyScaledDiffuseColor)() const;
	const VectorArgb & (Light::*getPossiblyScaledDiffuseBackColor)() const;
	const VectorArgb & (Light::*getPossiblyScaledDiffuseTangentColor)() const;
	float              (Light::*getPossiblyScaledSpecularIntensity)() const;
	const VectorArgb & (Light::*getPossiblyScaledSpecularColor)() const;

#if PRODUCTION == 0
	bool ms_disableLightCaching;
	bool ms_forceAllLightsOn;
	bool ms_forceAllLightsOff;
#endif
}
using namespace Direct3d9_LightManagerNamespace;

// ======================================================================

bool Direct3d9_LightManager::SelectedLights::operator !=(SelectedLights const &rhs)
{
	if (dirty != rhs.dirty)
		return true;

	if (obeysLightScale != rhs.obeysLightScale)
		return true;

	if (ambient != rhs.ambient)
		return true;

	{
		for (int i = 0; i < ParallelSpecularCount; ++i)
			if (parallelSpecular[i] != rhs.parallelSpecular[i])
				return true;
	}

	{
		for (int i = 0; i < ParallelCount; ++i)
			if (parallel[i] != rhs.parallel[i])
				return true;
	}

	{
		for (int i = 0; i < PointSpecularCount; ++i)
			if (pointSpecular[i] != rhs.pointSpecular[i])
				return true;;
	}

	{
		for (int i = 0; i < PointCount; ++i)
			if (point[i] != rhs.point[i])
				return true;
	}

	return false;
}
// ----------------------------------------------------------------------

Direct3d9_LightManager::SelectedLights & Direct3d9_LightManager::SelectedLights::operator =(SelectedLights const &rhs)
{
	dirty = rhs.dirty;
	obeysLightScale = rhs.obeysLightScale;
	ambient = rhs.ambient;

	{
		for (int i = 0; i < ParallelSpecularCount; ++i)
			parallelSpecular[i] = rhs.parallelSpecular[i];
	}

	{
		for (int i = 0; i < ParallelCount; ++i)
			parallel[i] = rhs.parallel[i];
	}

	{
		for (int i = 0; i < PointSpecularCount; ++i)
			pointSpecular[i] = rhs.pointSpecular[i];
	}

	{
		for (int i = 0; i < PointCount; ++i)
			point[i] = rhs.point[i];
	}

	return *this;
}

// ======================================================================

struct Direct3d9_LightManager::SortLightsDistance
{
	bool operator()(const Light *lhs, const Light *rhs) const;
};

// ----------------------------------------------------------------------

inline bool Direct3d9_LightManager::SortLightsDistance::operator()(const Light *lhs, const Light *rhs) const
{
	return ms_objectPosition.magnitudeBetweenSquared(lhs->getPosition_w()) < ms_objectPosition.magnitudeBetweenSquared(rhs->getPosition_w());
}

// ======================================================================

void Direct3d9_LightManager::install()
{
	DEBUG_FATAL(sizeof(LightData) != 28 * 4 * sizeof(float), ("LightData bad size %d (should be %d)", sizeof(LightData), 28 * 4 * sizeof(float)));
	DEBUG_FATAL(PSCR_dot3LightDiffuseColor + 1 != PSCR_dot3LightSpecularColor, ("dot3 constants aren't next to each other"));

#if PRODUCTION == 0
	DebugFlags::registerFlag(ms_useOnlyfullAmbient,      "Direct3d9", "lightingFullAmbient");
	DebugFlags::registerFlag(ms_setLightsOncePerFrame,   "Direct3d9", "setLightsOncePerFrame");
	DebugFlags::registerFlag(ms_disableLightCaching, "Direct3d9", "disableLightCaching");
	if (Direct3d9::getShaderCapability() >= ShaderCapability(2,0))
		DebugFlags::registerFlag(ms_forceAllLightsOn,       "Direct3d9", "forceAllLightsOn");
	if (Direct3d9::getShaderCapability() >= ShaderCapability(2,0))
		DebugFlags::registerFlag(ms_forceAllLightsOff,       "Direct3d9", "forceAllLightsOff");
#endif
}

// ----------------------------------------------------------------------

void Direct3d9_LightManager::updateLightDirection()
{
}

// ======================================================================

#ifdef FFP

void Direct3d9_LightManager::setupLightData(const Light &light, D3DLIGHT9 &lightData)
{
	Zero(lightData);

	const VectorArgb &diffuseColor  = (light.*getPossiblyScaledDiffuseColor)();
	const VectorArgb &specularColor = (light.*getPossiblyScaledSpecularColor)();

	switch (light.getType())
	{
		case Light::T_parallel:
			lightData.Type        = D3DLIGHT_DIRECTIONAL;
			lightData.Diffuse.r   = diffuseColor.r;
			lightData.Diffuse.g   = diffuseColor.g;
			lightData.Diffuse.b   = diffuseColor.b;
			lightData.Diffuse.a   = diffuseColor.a;
			lightData.Specular.r  = specularColor.r;
			lightData.Specular.g  = specularColor.g;
			lightData.Specular.b  = specularColor.b;
			lightData.Specular.a  = specularColor.a;
			{
				const Vector &direction = light.getObjectFrameK_w();
				lightData.Direction.x = direction.x;
				lightData.Direction.y = direction.y;
				lightData.Direction.z = direction.z;
			}
			break;

		case Light::T_point:
		case Light::T_point_multicell:
			lightData.Type         = D3DLIGHT_POINT;
			lightData.Diffuse.r    = diffuseColor.r;
			lightData.Diffuse.g    = diffuseColor.g;
			lightData.Diffuse.b    = diffuseColor.b;
			lightData.Diffuse.a    = diffuseColor.a;
			lightData.Specular.r   = specularColor.r;
			lightData.Specular.g   = specularColor.g;
			lightData.Specular.b   = specularColor.b;
			lightData.Specular.a   = specularColor.a;
			{
				const Vector &position = light.getPosition_w();
				lightData.Position.x   = position.x;
				lightData.Position.y   = position.y;
				lightData.Position.z   = position.z;
			}
			lightData.Range        = light.getRange();
			lightData.Attenuation0 = light.getConstantAttenuation();
			lightData.Attenuation1 = light.getLinearAttenuation();
			lightData.Attenuation2 = light.getQuadraticAttenuation();
			break;

		case Light::T_spot:
			lightData.Type         = D3DLIGHT_SPOT;
			lightData.Diffuse.r    = diffuseColor.r;
			lightData.Diffuse.g    = diffuseColor.g;
			lightData.Diffuse.b    = diffuseColor.b;
			lightData.Diffuse.a    = diffuseColor.a;
			lightData.Specular.r   = specularColor.r;
			lightData.Specular.g   = specularColor.g;
			lightData.Specular.b   = specularColor.b;
			lightData.Specular.a   = specularColor.a;
			{
				const Vector &position = light.getPosition_w();
				lightData.Position.x   = position.x;
				lightData.Position.y   = position.y;
				lightData.Position.z   = position.z;
			}
			{
				const Vector &direction = light.getObjectFrameK_w();
				lightData.Direction.x = direction.x;
				lightData.Direction.y = direction.y;
				lightData.Direction.z = direction.z;
			}
			lightData.Range        = light.getRange();
			lightData.Falloff      = light.getSpotFallOff();
			lightData.Attenuation0 = light.getConstantAttenuation();
			lightData.Attenuation1 = light.getLinearAttenuation();
			lightData.Attenuation2 = light.getQuadraticAttenuation();
			lightData.Theta        = light.getSpotTheta();
			lightData.Phi          = light.getSpotPhi();
			break;

		case Light::T_ambient:
		default:
			DEBUG_FATAL(true, ("bad case"));
			return;  //lint !e527 // unreachable
	}
}

#endif

// ----------------------------------------------------------------------

void Direct3d9_LightManager::setUsingVertexShaderProgram(bool usingVertexShaderProgram)
{
	if (ms_usingVertexShaderProgram != usingVertexShaderProgram)
	{
		ms_usingVertexShaderProgram = usingVertexShaderProgram;
		ms_dirty = true;
	}
}

// ----------------------------------------------------------------------

void Direct3d9_LightManager::setObjectToWorldTransform(const Transform &objectToWorldTransform)
{
	ms_objectToWorldTransform = objectToWorldTransform;
	ms_objectPosition = ms_objectToWorldTransform.getPosition_p();
	ms_dirty = true;
}

// ----------------------------------------------------------------------

void Direct3d9_LightManager::setCameraPosition(const Vector &cameraPosition)
{
	ms_cameraPosition = cameraPosition;
	ms_dirty = true;
}

// ----------------------------------------------------------------------

void Direct3d9_LightManager::setLights(const LightList &lightList)
{
	ms_lightList = lightList;
	ms_dirty = true;
}

// ----------------------------------------------------------------------

void Direct3d9_LightManager::setObeysLightScale(const bool obeysLightScale)
{
	ms_obeysLightScale = obeysLightScale;
	if (ms_obeysLightScale)
	{
		getPossiblyScaledDiffuseIntensity   = &Light::getScaledDiffuseIntensity;
		getPossiblyScaledDiffuseColor       = &Light::getScaledDiffuseColor;
		getPossiblyScaledDiffuseBackColor   = &Light::getScaledDiffuseBackColor;
		getPossiblyScaledDiffuseTangentColor= &Light::getScaledDiffuseTangentColor;
		getPossiblyScaledSpecularIntensity  = &Light::getScaledSpecularIntensity;
		getPossiblyScaledSpecularColor      = &Light::getScaledSpecularColor;
	}
	else
	{
		getPossiblyScaledDiffuseIntensity   = &Light::getDiffuseIntensity;
		getPossiblyScaledDiffuseColor       = &Light::getDiffuseColor;
		getPossiblyScaledDiffuseBackColor   = &Light::getDiffuseBackColor;
		getPossiblyScaledDiffuseTangentColor= &Light::getDiffuseTangentColor;
		getPossiblyScaledSpecularIntensity  = &Light::getSpecularIntensity;
		getPossiblyScaledSpecularColor      = &Light::getSpecularColor;
	}
	ms_dirty = true;	
}

// ----------------------------------------------------------------------

void Direct3d9_LightManager::beginFrame()
{
#if PRODUCTION == 0
	ms_newFrame = true;
#endif

	ms_lastLights.dirty = true;
	for (int i = 0; i < 8; ++i)
		ms_fixedFunctionPipelineLight[i] = NULL;
}

// ----------------------------------------------------------------------

inline void swap(Light const *&a, Light const *&b)
{
	Light const * swapTemporary = a;
	a = b;
	b = swapTemporary;
}

// ----------------------------------------------------------------------

void Direct3d9_LightManager::selectLights()
{
	if (!ms_dirty)
		return;

	{
		PROFILER_AUTO_BLOCK_DEFINE("Direct3d9_LightManager::selectLights");

		// all the previous lights are whack.  bling bling!
		ms_currentLights.obeysLightScale = ms_obeysLightScale;
		{
			for (int i = 0; i < ParallelSpecularCount; ++i)
				ms_currentLights.parallelSpecular[i] = NULL;
		}
		{
			for (int i = 0; i < ParallelCount; ++i)
				ms_currentLights.parallel[i] = NULL;
		}
		{
			for (int i = 0; i < PointSpecularCount; ++i)
				ms_currentLights.pointSpecular[i] = NULL;
		}
		{
			for (int i = 0; i < PointCount; ++i)
				ms_currentLights.point[i] = NULL;
		}

#if PRODUCTION == 0
		if (ms_useOnlyfullAmbient)
		{
			ms_currentLights.ambient.r = 1.0f;
			ms_currentLights.ambient.g = 1.0f;
			ms_currentLights.ambient.b = 1.0f;
			ms_currentLights.ambient.a = 1.0f;
		}
		else
#endif
		{
			ms_currentLights.ambient = ms_fullAmbient;

			LightList::iterator iEnd = ms_lightList.end();
			for (LightList::iterator i = ms_lightList.begin(); i != iEnd; ++i)
			{
				const Light *light = *i;
				switch (light->getType())
				{
					case Light::T_ambient:
					{
						// collapse all ambient lights into a single value
						const VectorArgb &diffuseColor = (light->*getPossiblyScaledDiffuseColor)();
						ms_currentLights.ambient.r += diffuseColor.r;
						ms_currentLights.ambient.g += diffuseColor.g;
						ms_currentLights.ambient.b += diffuseColor.b;
						light = NULL;
						break;
					}

					case Light::T_parallel:
					{
						// try to fit the light in a specular slot
						{
							for (int i = 0; light && i < ParallelSpecularCount; ++i)
								if (!ms_currentLights.parallelSpecular[i] || (light->*getPossiblyScaledSpecularIntensity)() > (ms_currentLights.parallelSpecular[i]->*getPossiblyScaledSpecularIntensity)())
									swap(ms_currentLights.parallelSpecular[i], light);
						}

						// non-specular light, or all specular slots were full
						{
							for (int i = 0; light && i < ParallelCount; ++i)
								if (!ms_currentLights.parallel[i] || (light->*getPossiblyScaledDiffuseIntensity)() > (ms_currentLights.parallel[i]->*getPossiblyScaledDiffuseIntensity)())
									swap(ms_currentLights.parallel[i], light);
						}

						break;
					}

					case Light::T_point:
					case Light::T_point_multicell:
					{
						// try to fit the light in a specular slot
						{
							for (int i = 0; light && i < PointSpecularCount; ++i)
							{
								if (!ms_currentLights.pointSpecular[i] || (light->*getPossiblyScaledSpecularIntensity)() > (ms_currentLights.pointSpecular[i]->*getPossiblyScaledSpecularIntensity)())
									swap(ms_currentLights.pointSpecular[i], light);
							}
						}

						// non-specular light, or all specular slots were full
						{
							for (int i = 0; light && i < PointCount; ++i)
							{
								if (!ms_currentLights.point[i] || (light->*getPossiblyScaledDiffuseIntensity)() > (ms_currentLights.point[i]->*getPossiblyScaledDiffuseIntensity)())
									swap(ms_currentLights.point[i], light);
							}
						}
						break;
					}

					case Light::T_spot:
					{
						// discard all spot lights
						break;
					}

					default:
					{
						DEBUG_FATAL(true, ("bad light type %d", static_cast<int>(light->getType())));
					}
				}
			}
		}

#if PRODUCTION == 0
		if (ms_setLightsOncePerFrame)
		{
			if (!ms_newFrame)
				return;
			ms_newFrame = false;
		}
#endif
	}

	{
		PROFILER_AUTO_BLOCK_DEFINE("Direct3d9_LightManager::applyLights");
#if defined(FFP) && defined(VSPS)
		if (ms_usingVertexShaderProgram)
#endif
#ifdef VSPS
		{
			if (
#if PRODUCTION == 0
					ms_disableLightCaching || 
#endif
					ms_currentLights != ms_lastLights)
				applyLights_vertexShader();
			else
				applyLights_vertexShader_dot3();
		}
#endif
#if defined(FFP) && defined(VSPS)
		else
#endif
#ifdef FFP
		{
			if (
#if PRODUCTION == 0
				ms_disableLightCaching || 
#endif
				ms_currentLights != ms_lastLights)
				applyLights_fixedFunctionPipeline();
		}
#endif
	}

	ms_lastLights = ms_currentLights;
}

// ----------------------------------------------------------------------

#ifdef VSPS

void Direct3d9_LightManager::applyLights_vertexShader()
{
	LightData lightData;
	Zero(lightData);
	ExtendedLightData extendedLightData;
	Zero(extendedLightData);

	{
		lightData.ambient = ms_currentLights.ambient;
	}

	{
		for (int i = 0; i < ParallelSpecularCount; ++i)
		{
			const Light *light = ms_currentLights.parallelSpecular[i];
			if (light)
			{
				ParallelSpecularData &parallelSpecular = lightData.parallelSpecular[i];

				const Vector &direction = light->getObjectFrameK_w();
				parallelSpecular.direction.x = -direction.x;
				parallelSpecular.direction.y = -direction.y;
				parallelSpecular.direction.z = -direction.z;

				const VectorArgb &diffuseColor = (light->*getPossiblyScaledDiffuseColor)();
				parallelSpecular.diffuseColor.r = diffuseColor.r;
				parallelSpecular.diffuseColor.g = diffuseColor.g;
				parallelSpecular.diffuseColor.b = diffuseColor.b;
				parallelSpecular.diffuseColor.a = diffuseColor.a;

				const VectorArgb &specularColor = (light->*getPossiblyScaledSpecularColor)();
				parallelSpecular.specularColor.r = specularColor.r;
				parallelSpecular.specularColor.g = specularColor.g;
				parallelSpecular.specularColor.b = specularColor.b;
				parallelSpecular.specularColor.a = specularColor.a;

				if (i == 0)
				{
					// update the dot3 data and add the alpha fade and bloom constants to their new packed locations
					const Vector c = ms_objectToWorldTransform.rotateTranslate_p2l(ms_cameraPosition);
					lightData.dot3.localCameraPosition.x  = c.x;
					lightData.dot3.localCameraPosition.y  = c.y;
					lightData.dot3.localCameraPosition.z  = c.z;
					lightData.dot3.localCameraPosition.w  = 1.f;

					const Vector d = ms_objectToWorldTransform.rotate_p2l(direction);
					lightData.dot3.localDirection.x  = -d.x;
					lightData.dot3.localDirection.y  = -d.y;
					lightData.dot3.localDirection.z  = -d.z;
					lightData.dot3.localDirection.w = Direct3d9_StateCache::getSpecularPower(); // materialSpecularPower stored in localDirection.w

					VectorRgba alphaFadeAndBloomSettings = Direct3d9::getAlphaFadeAndBloomSettings();

					lightData.dot3.diffuseColor     = parallelSpecular.diffuseColor;
					lightData.dot3.diffuseColor.a = alphaFadeAndBloomSettings.r; //alphaFadeOpacityEnabled in diffuseColor.a
					lightData.dot3.specularColor = parallelSpecular.specularColor;
					lightData.dot3.specularColor.a = alphaFadeAndBloomSettings.a; //alphaFadeOpacity in specularColor.a

					/////////////////////////////////////////////////////////////////////////////////////////////////
					// set hemishpheric lighting data
					HemisphericLightData &extendedParallelSpecular = extendedLightData.parallelSpecular[i];
					_vsps_setExtendedLightData(extendedParallelSpecular, light, diffuseColor, alphaFadeAndBloomSettings.g);
					/////////////////////////////////////////////////////////////////////////////////////////////////

					struct PixelDot3Data pixelDot3Data;

					pixelDot3Data.localDirection             = lightData.dot3.localDirection;
					pixelDot3Data.diffuseColor               = lightData.dot3.diffuseColor;
					pixelDot3Data.specularColor              = lightData.dot3.specularColor;
					pixelDot3Data.tangentMinusDiffuseColor   = extendedParallelSpecular.tangentMinusDiffuseColor;
					pixelDot3Data.tangentMinusBackColor      = extendedParallelSpecular.tangentMinusBackColor;
					pixelDot3Data.tangentMinusBackColor.a    = Direct3d9Namespace::ms_currentTime;

					Direct3d9_StateCache::setPixelShaderConstants(PSCR_dot3LightDirection, &pixelDot3Data, 5);

					//Direct3d9_StateCache::setPixelShaderConstants(PSCR_dot3LightDirection, &lightData.dot3.localDirection, 3);
					//Direct3d9_StateCache::setPixelShaderConstants(PSCR_dot3LightTangentMinusDiffuseColor, &extendedParallelSpecular.tangentMinusDiffuseColor, 1);
					//Direct3d9_StateCache::setPixelShaderConstants(PSCR_dot3LightTangentMinusBackColor, &extendedParallelSpecular.tangentMinusBackColor, 1);

					Direct3d9Namespace::ms_alphaFadeOpacityDirty=false;
				}
			}
			else
			{
				if (i == 0)
				{
					struct PixelDot3Data pixelDot3Data;
					Zero(pixelDot3Data);

					VectorRgba alphaFadeAndBloomSettings = Direct3d9::getAlphaFadeAndBloomSettings();

					pixelDot3Data.diffuseColor.a = alphaFadeAndBloomSettings.r; //alphaFadeOpacityEnabled in diffuseColor.a
					pixelDot3Data.specularColor.a = alphaFadeAndBloomSettings.a; //alphaFadeOpacity in specularColor.a
					pixelDot3Data.tangentMinusDiffuseColor.a = alphaFadeAndBloomSettings.g; //bloomEnabled in tangentMinusDiffuseColor.a
					pixelDot3Data.tangentMinusBackColor.a    = Direct3d9Namespace::ms_currentTime;

					Direct3d9_StateCache::setPixelShaderConstants(PSCR_dot3LightDirection, &pixelDot3Data, 5);
				}
			}
		}
	}

	{
		for (int i = 0; i < ParallelCount; ++i)
		{
			const Light *light = ms_currentLights.parallel[i];
			if (light)
			{
				ParallelData &parallel = lightData.parallel[i];

				const Vector &direction = light->getObjectFrameK_w();
				parallel.direction.x = -direction.x;
				parallel.direction.y = -direction.y;
				parallel.direction.z = -direction.z;

				const VectorArgb &diffuseColor = (light->*getPossiblyScaledDiffuseColor)();
				parallel.diffuseColor.r = diffuseColor.r;
				parallel.diffuseColor.g = diffuseColor.g;
				parallel.diffuseColor.b = diffuseColor.b;
				parallel.diffuseColor.a = diffuseColor.a;
			}
		}
	}

	{
		for (int i = 0; i < PointSpecularCount; ++i)
		{
			const Light *light = ms_currentLights.pointSpecular[i];
			if (light)
			{
				PointSpecularData &pointSpecular = lightData.pointSpecular[i];

				const Vector &position = light->getPosition_w();
				pointSpecular.position.x = position.x;
				pointSpecular.position.y = position.y;
				pointSpecular.position.z = position.z;
				pointSpecular.position.w = 1.0f;

				const VectorArgb &diffuseColor = (light->*getPossiblyScaledDiffuseColor)();
				pointSpecular.diffuseColor.r = diffuseColor.r;
				pointSpecular.diffuseColor.g = diffuseColor.g;
				pointSpecular.diffuseColor.b = diffuseColor.b;
				pointSpecular.diffuseColor.a = diffuseColor.a;

				pointSpecular.attenuation.k0 = light->getConstantAttenuation();
				pointSpecular.attenuation.k1 = light->getLinearAttenuation();
				pointSpecular.attenuation.k2 = light->getQuadraticAttenuation();
				pointSpecular.attenuation.k3 = 0.0f;

				const VectorArgb &specularColor = (light->*getPossiblyScaledSpecularColor)();
				pointSpecular.specularColor.r = specularColor.r;
				pointSpecular.specularColor.g = specularColor.g;
				pointSpecular.specularColor.b = specularColor.b;
				pointSpecular.specularColor.a = specularColor.a;
			}
			else
			{
				lightData.pointSpecular[i].attenuation.k0 = 1.0f;
			}
		}
	}

	{
		for (int i = 0; i < PointCount; ++i)
		{
			const Light *light = ms_currentLights.point[i];
			if (light)
			{
				PointData &point = lightData.point[i];

				const Vector &position = light->getPosition_w();
				point.position.x = position.x;
				point.position.y = position.y;
				point.position.z = position.z;
				point.position.w = 1.0f;

				const VectorArgb &diffuseColor = (light->*getPossiblyScaledDiffuseColor)();
				point.diffuseColor.r = diffuseColor.r;
				point.diffuseColor.g = diffuseColor.g;
				point.diffuseColor.b = diffuseColor.b;
				point.diffuseColor.a = diffuseColor.a;

				point.attenuation.k0 = light->getConstantAttenuation();
				point.attenuation.k1 = light->getLinearAttenuation();
				point.attenuation.k2 = light->getQuadraticAttenuation();
				point.attenuation.k3 = 0.0f;
			}
			else
			{
				lightData.point[i].attenuation.k0 = 1.0f;
			}
		}
	}

	// set the light data
	Direct3d9_StateCache::setVertexShaderConstants(VSCR_lightData, &lightData, sizeof(LightData) / (4 * sizeof(float)));
	// set the extra light data
	Direct3d9_StateCache::setVertexShaderConstants(VCSR_extendedLightData, &extendedLightData, sizeof(ExtendedLightData) / (4 * sizeof(float)));
}

#endif

#ifdef VSPS

void Direct3d9_LightManager::_vsps_setExtendedLightData(
	HemisphericLightData &extendedParallelSpecular, 
	const Light *light, 
	const VectorArgb &diffuseColor, 
	float bloomEnabled
	)
{
	const VectorArgb &diffuseBackColor = (light->*getPossiblyScaledDiffuseBackColor)();
	extendedParallelSpecular.backColor.r = diffuseBackColor.r;
	extendedParallelSpecular.backColor.g = diffuseBackColor.g;
	extendedParallelSpecular.backColor.b = diffuseBackColor.b;
	extendedParallelSpecular.backColor.a = diffuseBackColor.a;

	const VectorArgb &diffuseTangentColor = (light->*getPossiblyScaledDiffuseTangentColor)();
	extendedParallelSpecular.tangentColor.r = diffuseTangentColor.r;
	extendedParallelSpecular.tangentColor.g = diffuseTangentColor.g;
	extendedParallelSpecular.tangentColor.b = diffuseTangentColor.b;
	extendedParallelSpecular.tangentColor.a = diffuseTangentColor.a;

	extendedParallelSpecular.tangentMinusBackColor.r = diffuseTangentColor.r - diffuseBackColor.r;
	extendedParallelSpecular.tangentMinusBackColor.g = diffuseTangentColor.g - diffuseBackColor.g;
	extendedParallelSpecular.tangentMinusBackColor.b = diffuseTangentColor.b - diffuseBackColor.b;
	extendedParallelSpecular.tangentMinusBackColor.a = diffuseTangentColor.a - diffuseBackColor.a;

	extendedParallelSpecular.tangentMinusDiffuseColor.r = diffuseTangentColor.r - diffuseColor.r;
	extendedParallelSpecular.tangentMinusDiffuseColor.g = diffuseTangentColor.g - diffuseColor.g;
	extendedParallelSpecular.tangentMinusDiffuseColor.b = diffuseTangentColor.b - diffuseColor.b;
	extendedParallelSpecular.tangentMinusDiffuseColor.a = bloomEnabled; //bloomEnabled in tangentMinusDiffuseColor.a
}

#endif

// ----------------------------------------------------------------------

#ifdef VSPS

void Direct3d9_LightManager::applyLights_vertexShader_dot3()
{
	const Light *light = ms_currentLights.parallelSpecular[0];
	if (light)
	{
		Dot3Data dot3Data;

		////////////////////////////////////////////////////////

		const Vector c = ms_objectToWorldTransform.rotateTranslate_p2l(ms_cameraPosition);
		dot3Data.localCameraPosition.x  = c.x;
		dot3Data.localCameraPosition.y  = c.y;
		dot3Data.localCameraPosition.z  = c.z;
		dot3Data.localCameraPosition.w  = 1.f;

		////////////////////////////////////////////////////////

		const Vector &direction = light->getObjectFrameK_w();
		const Vector d = ms_objectToWorldTransform.rotate_p2l(direction);
		dot3Data.localDirection.x  = -d.x;
		dot3Data.localDirection.y  = -d.y;
		dot3Data.localDirection.z  = -d.z;
		dot3Data.localDirection.w = Direct3d9_StateCache::getSpecularPower(); // materialSpecularPower stored in localDirection.w

		////////////////////////////////////////////////////////
		const int vertexDot3Register = VSCR_lightData + FIELD_OFFSET(LightData, dot3)/16;

		if (Direct3d9Namespace::ms_alphaFadeOpacityDirty)
		{
			Direct3d9Namespace::ms_alphaFadeOpacityDirty=false;

			VectorRgba alphaFadeAndBloomSettings = Direct3d9::getAlphaFadeAndBloomSettings();

			const VectorArgb &diffuseColor = (light->*getPossiblyScaledDiffuseColor)();
			dot3Data.diffuseColor.r = diffuseColor.r;
			dot3Data.diffuseColor.g = diffuseColor.g;
			dot3Data.diffuseColor.b = diffuseColor.b;
			dot3Data.diffuseColor.a = alphaFadeAndBloomSettings.r; //alphaFadeOpacityEnabled in diffuseColor.a

			const VectorArgb &specularColor = (light->*getPossiblyScaledSpecularColor)();
			dot3Data.specularColor.r = specularColor.r;
			dot3Data.specularColor.g = specularColor.g;
			dot3Data.specularColor.b = specularColor.b;
			dot3Data.specularColor.a = alphaFadeAndBloomSettings.a; //alphaFadeOpacity in specularColor.a

			/////////////////////////////////////////////////////////////////////////////////////////////////
			// set hemishpheric lighting data
			ExtendedLightData extendedLightData;
			Zero(extendedLightData);
			HemisphericLightData &extendedParallelSpecular = extendedLightData.parallelSpecular[0];
			_vsps_setExtendedLightData(extendedParallelSpecular, light, diffuseColor, alphaFadeAndBloomSettings.g);
			/////////////////////////////////////////////////////////////////////////////////////////////////

			/////////////////////////////////////////////////////////////////////////////////////////////////
			// set the pixel shader dot3 light data
			struct PixelDot3Data pixelDot3Data;
			pixelDot3Data.localDirection             = dot3Data.localDirection;
			pixelDot3Data.diffuseColor               = dot3Data.diffuseColor;
			pixelDot3Data.specularColor              = dot3Data.specularColor;
			pixelDot3Data.tangentMinusDiffuseColor   = extendedParallelSpecular.tangentMinusDiffuseColor;
			pixelDot3Data.tangentMinusBackColor      = extendedParallelSpecular.tangentMinusBackColor;
			pixelDot3Data.tangentMinusBackColor.a    = Direct3d9Namespace::ms_currentTime;
			Direct3d9_StateCache::setPixelShaderConstants(PSCR_dot3LightDirection, &pixelDot3Data, 5); // last field ignored.
			/////////////////////////////////////////////////////////////////////////////////////////////////

			/////////////////////////////////////////////////////////////////////////////////////////////////
			// set the vertex shader dot3 light data
			Direct3d9_StateCache::setVertexShaderConstants(vertexDot3Register, &dot3Data, 4);

			const int VSCR_tangentMinusDiffuseColor = VCSR_extendedLightData + FIELD_OFFSET(ExtendedLightData, parallelSpecular[0].tangentMinusDiffuseColor)/16;
			Direct3d9_StateCache::setVertexShaderConstants(VSCR_tangentMinusDiffuseColor, &extendedLightData.parallelSpecular[0].tangentMinusDiffuseColor, 1);
			/////////////////////////////////////////////////////////////////////////////////////////////////
		}
		else
		{
			// set the light data
			Direct3d9_StateCache::setPixelShaderConstants(PSCR_dot3LightDirection, &dot3Data.localDirection, 1);
			Direct3d9_StateCache::setVertexShaderConstants(vertexDot3Register, &dot3Data, 2);
		}
		////////////////////////////////////////////////////////
	}
	else
	{
		////////////////////////////////////////////////////////
		if (Direct3d9Namespace::ms_alphaFadeOpacityDirty)
		{
			Direct3d9Namespace::ms_alphaFadeOpacityDirty=false;

			Dot3Data dot3Data;
			Zero(dot3Data);

			VectorRgba alphaFadeAndBloomSettings = Direct3d9::getAlphaFadeAndBloomSettings();

			dot3Data.diffuseColor.a = alphaFadeAndBloomSettings.r; //alphaFadeOpacityEnabled in diffuseColor.a
			dot3Data.specularColor.a = alphaFadeAndBloomSettings.a; //alphaFadeOpacity in specularColor.a

			/////////////////////////////////////////////////////////////////////////////////////////////////
			// set hemishpheric lighting data
			ExtendedLightData extendedLightData;
			Zero(extendedLightData);
			HemisphericLightData &extendedParallelSpecular = extendedLightData.parallelSpecular[0];
			extendedParallelSpecular.tangentMinusDiffuseColor.a = alphaFadeAndBloomSettings.g; //bloomEnabled in tangentMinusDiffuseColor.a
			/////////////////////////////////////////////////////////////////////////////////////////////////

			/////////////////////////////////////////////////////////////////////////////////////////////////
			// set the pixel shader dot3 light data
			struct PixelDot3Data pixelDot3Data;
			pixelDot3Data.diffuseColor               = dot3Data.diffuseColor;
			pixelDot3Data.specularColor              = dot3Data.specularColor;
			pixelDot3Data.tangentMinusDiffuseColor   = extendedParallelSpecular.tangentMinusDiffuseColor;
			pixelDot3Data.tangentMinusBackColor.a    = Direct3d9Namespace::ms_currentTime;

			const int PSCR_pixelAlphaFadeBloomUpdate = PSCR_dot3LightDirection + FIELD_OFFSET(PixelDot3Data, diffuseColor)/16;
			Direct3d9_StateCache::setPixelShaderConstants(PSCR_pixelAlphaFadeBloomUpdate, &pixelDot3Data.diffuseColor, 4); // last field ignored.
			/////////////////////////////////////////////////////////////////////////////////////////////////

			/////////////////////////////////////////////////////////////////////////////////////////////////
			// set the vertex shader dot3 light data
			const int VSCR_vertexAlphaFadeBloomUpdate = VSCR_lightData + FIELD_OFFSET(LightData, dot3.diffuseColor)/16;
			Direct3d9_StateCache::setVertexShaderConstants(VSCR_vertexAlphaFadeBloomUpdate, &dot3Data.diffuseColor, 2);

			const int VSCR_tangentMinusDiffuseColor = VCSR_extendedLightData + FIELD_OFFSET(ExtendedLightData, parallelSpecular[0].tangentMinusDiffuseColor)/16;
			Direct3d9_StateCache::setVertexShaderConstants(VSCR_tangentMinusDiffuseColor, &extendedLightData.parallelSpecular[0].tangentMinusDiffuseColor, 1);
			/////////////////////////////////////////////////////////////////////////////////////////////////
		}
	}
}

#endif

// ----------------------------------------------------------------------

#ifdef FFP

void Direct3d9_LightManager::applyLights_fixedFunctionPipeline()
{
	IDirect3DDevice9 *device = Direct3d9::getDevice();

	const Light **array[4] =
	{
		ms_currentLights.parallelSpecular,
		ms_currentLights.parallel,
		ms_currentLights.pointSpecular,
		ms_currentLights.point
	};

	const int max[4] =
	{
		ParallelSpecularCount,
		ParallelCount,
		PointSpecularCount,
		PointCount
	};

	Direct3d9_StateCache::setRenderState(
		D3DRS_AMBIENT,static_cast<DWORD>(
		(clamp(0U, static_cast<uint>(ms_currentLights.ambient.r * 255.0f), 255U) << 16) |
		(clamp(0U, static_cast<uint>(ms_currentLights.ambient.g * 255.0f), 255U) <<  8) |
		(clamp(0U, static_cast<uint>(ms_currentLights.ambient.b * 255.0f), 255U) <<  0)));

	int       lightIndex = 0;
	D3DLIGHT9 d3dLight;

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < max[i] && array[i][j]; ++j, ++lightIndex)
		{
			// only reset lights when they have changed
			const Light *light = array[i][j];
			if (ms_fixedFunctionPipelineLight[lightIndex] != light)
			{
				setupLightData(*light, d3dLight);

				HRESULT hresult = device->SetLight(lightIndex, &d3dLight);
				FATAL_DX_HR("SetLight failed %s", hresult);

				hresult = device->LightEnable(lightIndex, TRUE);
				FATAL_DX_HR("LightEnable failed %s", hresult);

				ms_fixedFunctionPipelineLight[lightIndex] = light;
			}
		}
	}

	for ( ; lightIndex < 8 && ms_fixedFunctionPipelineLight[lightIndex]; ++lightIndex)
	{
		HRESULT hresult = device->LightEnable(lightIndex, FALSE);
		FATAL_DX_HR("LightEnable failed %s", hresult);
		ms_fixedFunctionPipelineLight[lightIndex] = NULL;
	}
}

#endif

// ======================================================================
