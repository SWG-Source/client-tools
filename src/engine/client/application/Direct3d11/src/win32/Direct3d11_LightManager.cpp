// ======================================================================
//
// Direct3d11_LightManager.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_LightManager.h"

#include "Direct3d11.h"
#include "ConfigDirect3d11.h"
#include "Direct3d11_ConstantBuffers.h"
#include "Direct3d11_Metrics.h"
#include "Direct3d11_StateCache.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/ShaderConstantRegisters.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/Production.h"
#include "sharedMath/VectorArgb.h"

#include <cstddef>

// ======================================================================
// The register layout is asset ABI, so it is checked at compile time rather than trusted.
// DX9 asserts the first of these at runtime through DEBUG_FATAL, which means a Release build
// never checks it at all.

namespace
{
	int const cms_registerBytes = 4 * static_cast<int>(sizeof(float));
}

static_assert(sizeof(Direct3d11_LightManager::LightData) == 28 * 4 * sizeof(float),
	"LightData must be exactly 28 constant registers; the shipped vertex_shader_constants.inc depends on it");

static_assert(sizeof(Direct3d11_LightManager::ExtendedLightData) == 4 * 4 * sizeof(float),
	"ExtendedLightData must be exactly 4 constant registers at c60");

static_assert(sizeof(Direct3d11_LightManager::PixelDot3Data) == 5 * 4 * sizeof(float),
	"PixelDot3Data must be exactly 5 pixel constant registers");

static_assert(PSCR_dot3LightDiffuseColor + 1 == PSCR_dot3LightSpecularColor,
	"the dot3 pixel constants must be adjacent; the packed alpha components are written as one block");

// ======================================================================

namespace Direct3d11_LightManagerNamespace
{
	bool                                          ms_dirty;
	bool                                          ms_obeysLightScale;
	Transform                                     ms_objectToWorldTransform;
	Vector                                        ms_cameraPosition;
	Direct3d11_LightManager::LightList            ms_lightList;
	VectorRgba                                    ms_fullAmbient;
	Direct3d11_LightManager::SelectedLights        ms_currentLights;
	Direct3d11_LightManager::SelectedLights        ms_lastLights;

	// r: alphaFadeOpacityEnabled, g: bloomEnabled, a: alphaFadeOpacity. b is unused. The
	// packing is DX9's and is read by shipped shader code; see the header.
	VectorRgba                                    ms_alphaFadeAndBloom;
	bool                                          ms_alphaFadeOpacityEnabled;
	bool                                          ms_alphaFadeOpacityDirty;

#if PRODUCTION == 0
	bool ms_useOnlyFullAmbient;
	bool ms_setLightsOncePerFrame;
	bool ms_newFrame;
	bool ms_disableLightCaching;
#endif

	// The minimum scene ambient, and the two hemisphere ratios. Named rather than left as
	// literals because all three are tuning values inherited from the x64 DX9 build, and a
	// parity comparison needs to be able to find them.
	float const cms_minimumAmbient        = 0.3f;
	float const cms_syntheticTangentScale = 0.65f;
	float const cms_syntheticBackScale    = 0.30f;

	inline void copyClampedRgb(VectorRgba &destination, VectorArgb const &source);
	inline void swapLight(Light const *&a, Light const *&b);
}

using namespace Direct3d11_LightManagerNamespace;

// ======================================================================
/**
 * Copy a colour, clamping the channels non-negative.
 *
 * Inherited from the x64 DX9 build, whose comment records the diagnosis: a negative diffuse
 * out of the day/night ramp interpolator turns surfaces black, or worse subtracts from
 * ambient, because the HLSL diffuse term skips the saturate. Clamping at upload makes the
 * failure flat-lit rather than fully black.
 *
 * Alpha is passed through unclamped -- every one of these registers uses its alpha for
 * something other than a colour.
 */

inline void Direct3d11_LightManagerNamespace::copyClampedRgb(VectorRgba &destination, VectorArgb const &source)
{
	destination.r = source.r > 0.0f ? source.r : 0.0f;
	destination.g = source.g > 0.0f ? source.g : 0.0f;
	destination.b = source.b > 0.0f ? source.b : 0.0f;
	destination.a = source.a;
}

// ----------------------------------------------------------------------

inline void Direct3d11_LightManagerNamespace::swapLight(Light const *&a, Light const *&b)
{
	Light const * const temporary = a;
	a = b;
	b = temporary;
}

// ======================================================================

bool Direct3d11_LightManager::SelectedLights::operator !=(SelectedLights const &rhs) const
{
	if (dirty != rhs.dirty)
		return true;

	if (obeysLightScale != rhs.obeysLightScale)
		return true;

	if (ambient != rhs.ambient)
		return true;

	for (int i = 0; i < ParallelSpecularCount; ++i)
		if (parallelSpecular[i] != rhs.parallelSpecular[i])
			return true;

	for (int i = 0; i < ParallelCount; ++i)
		if (parallel[i] != rhs.parallel[i])
			return true;

	for (int i = 0; i < PointSpecularCount; ++i)
		if (pointSpecular[i] != rhs.pointSpecular[i])
			return true;

	for (int i = 0; i < PointCount; ++i)
		if (point[i] != rhs.point[i])
			return true;

	return false;
}

// ----------------------------------------------------------------------

Direct3d11_LightManager::SelectedLights &Direct3d11_LightManager::SelectedLights::operator =(SelectedLights const &rhs)
{
	dirty = rhs.dirty;
	obeysLightScale = rhs.obeysLightScale;
	ambient = rhs.ambient;

	for (int i = 0; i < ParallelSpecularCount; ++i)
		parallelSpecular[i] = rhs.parallelSpecular[i];

	for (int i = 0; i < ParallelCount; ++i)
		parallel[i] = rhs.parallel[i];

	for (int i = 0; i < PointSpecularCount; ++i)
		pointSpecular[i] = rhs.pointSpecular[i];

	for (int i = 0; i < PointCount; ++i)
		point[i] = rhs.point[i];

	return *this;
}

// ======================================================================

void Direct3d11_LightManager::install()
{
	ms_dirty = true;
	ms_obeysLightScale = false;
	ms_objectToWorldTransform = Transform::identity;
	// Constructed rather than Vector::zero, which is not exported across the DLL boundary.
	ms_cameraPosition = Vector(0.0f, 0.0f, 0.0f);

	Zero(ms_currentLights);
	Zero(ms_lastLights);
	ms_lastLights.dirty = true;

	ms_fullAmbient.r = 0.0f;
	ms_fullAmbient.g = 0.0f;
	ms_fullAmbient.b = 0.0f;
	ms_fullAmbient.a = 0.0f;

	// Opaque and unfaded, with bloom off, until the engine says otherwise. Dirty so that the
	// first object of the run uploads them rather than inheriting a zeroed register file, in
	// which alphaFadeOpacity of zero would make everything that reads it fully transparent.
	ms_alphaFadeAndBloom.r = 0.0f;
	ms_alphaFadeAndBloom.g = 0.0f;
	ms_alphaFadeAndBloom.b = 0.0f;
	ms_alphaFadeAndBloom.a = 1.0f;
	ms_alphaFadeOpacityEnabled = false;
	ms_alphaFadeOpacityDirty = true;

#if PRODUCTION == 0
	DebugFlags::registerFlag(ms_useOnlyFullAmbient,    "Direct3d11", "lightingFullAmbient");
	DebugFlags::registerFlag(ms_setLightsOncePerFrame, "Direct3d11", "setLightsOncePerFrame");
	DebugFlags::registerFlag(ms_disableLightCaching,   "Direct3d11", "disableLightCaching");
#endif
}

// ----------------------------------------------------------------------

void Direct3d11_LightManager::beginFrame()
{
#if PRODUCTION == 0
	ms_newFrame = true;
#endif

	// Force the next selection down the full-upload path. The register file is per-frame state
	// as far as this class is concerned, so nothing may be assumed to have survived.
	ms_lastLights.dirty = true;
}

// ======================================================================

void Direct3d11_LightManager::setLights(LightList const &lightList)
{
	ms_lightList = lightList;

	// Reported in the hitch line: a slow frame's light count is part of its description, and an
	// empty list in a lit interior is a bug rather than a performance note.
	Direct3d11_Metrics::lightsInList = static_cast<int>(ms_lightList.size());
	ms_dirty = true;
}

// ----------------------------------------------------------------------
/**
 * Whether this material's lighting obeys the day/night scale.
 *
 * The flag is recorded and then ignored, which is inherited from the x64 DX9 build. Its
 * comment records the diagnosis: with the flag set, the scaled-intensity comparison in
 * selectLights picks a different winner for the parallelSpecular slot -- an auxiliary light's
 * scaled specular outranks the sun's -- so the slot gets a dim cool light instead of the warm
 * sun. Character skin shaders set the flag, so characters rendered dark at midday. At noon the
 * ramp is close to 1.0 anyway, and losing the scaling at deep night is the better trade.
 *
 * It is still stored, because SelectedLights compares it: a material that flips the flag must
 * still be treated as a selection change.
 */

void Direct3d11_LightManager::setObeysLightScale(bool obeysLightScale)
{
	ms_obeysLightScale = obeysLightScale;
	ms_dirty = true;
}

// ----------------------------------------------------------------------

void Direct3d11_LightManager::setObjectToWorldTransform(Transform const &objectToWorldTransform)
{
	ms_objectToWorldTransform = objectToWorldTransform;
	ms_dirty = true;
}

// ----------------------------------------------------------------------

void Direct3d11_LightManager::setCameraPosition(Vector const &cameraPosition)
{
	ms_cameraPosition = cameraPosition;
	ms_dirty = true;
}

// ----------------------------------------------------------------------

void Direct3d11_LightManager::setFullAmbientOn(bool on)
{
	float const value = on ? 1.0f : 0.0f;

	ms_fullAmbient.r = value;
	ms_fullAmbient.g = value;
	ms_fullAmbient.b = value;
	ms_fullAmbient.a = 0.0f;
}

// ----------------------------------------------------------------------

void Direct3d11_LightManager::setAlphaFadeOpacity(bool enabled, float opacity)
{
	if (ms_alphaFadeOpacityEnabled != enabled)
	{
		ms_alphaFadeOpacityEnabled = enabled;
		ms_alphaFadeAndBloom.r = enabled ? 1.0f : 0.0f;
		ms_alphaFadeOpacityDirty = true;
	}

	if (opacity != ms_alphaFadeAndBloom.a)
	{
		ms_alphaFadeAndBloom.a = opacity;
		ms_alphaFadeOpacityDirty = true;
	}
}

// ----------------------------------------------------------------------

void Direct3d11_LightManager::setBloomEnabled(bool enabled)
{
	ms_alphaFadeAndBloom.g = enabled ? 1.0f : 0.0f;
	ms_alphaFadeOpacityDirty = true;
}

// ======================================================================
/**
 * Choose the eight lights, then upload them.
 *
 * The selection is a set of greedy passes, one per slot class, each keeping the brightest
 * candidate seen so far and pushing the loser onward. A light demoted out of the specular
 * slots gets a second chance in the non-specular ones, which is why swapLight passes the
 * candidate back out rather than dropping it.
 */

void Direct3d11_LightManager::selectLights()
{
	if (!ms_dirty)
		return;

	{
		PROFILER_AUTO_BLOCK_DEFINE("Direct3d11_LightManager::selectLights");

		ms_currentLights.obeysLightScale = ms_obeysLightScale;

		for (int i = 0; i < ParallelSpecularCount; ++i)
			ms_currentLights.parallelSpecular[i] = NULL;

		for (int i = 0; i < ParallelCount; ++i)
			ms_currentLights.parallel[i] = NULL;

		for (int i = 0; i < PointSpecularCount; ++i)
			ms_currentLights.pointSpecular[i] = NULL;

		for (int i = 0; i < PointCount; ++i)
			ms_currentLights.point[i] = NULL;

#if PRODUCTION == 0
		if (ms_useOnlyFullAmbient)
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

			LightList::const_iterator const end = ms_lightList.end();
			for (LightList::const_iterator i = ms_lightList.begin(); i != end; ++i)
			{
				Light const *light = *i;

				switch (light->getType())
				{
					case Light::T_ambient:
					{
						// Every ambient light collapses into the one ambient register.
						VectorArgb const &diffuseColor = light->getDiffuseColor();
						ms_currentLights.ambient.r += diffuseColor.r;
						ms_currentLights.ambient.g += diffuseColor.g;
						ms_currentLights.ambient.b += diffuseColor.b;
						light = NULL;
						break;
					}

					case Light::T_parallel:
					{
						// Ranked on specular intensity for the specular slot...
						for (int slot = 0; light && slot < ParallelSpecularCount; ++slot)
							if (!ms_currentLights.parallelSpecular[slot] || light->getSpecularIntensity() > ms_currentLights.parallelSpecular[slot]->getSpecularIntensity())
								swapLight(ms_currentLights.parallelSpecular[slot], light);

						// ...and on diffuse intensity for what is left.
						for (int slot = 0; light && slot < ParallelCount; ++slot)
							if (!ms_currentLights.parallel[slot] || light->getDiffuseIntensity() > ms_currentLights.parallel[slot]->getDiffuseIntensity())
								swapLight(ms_currentLights.parallel[slot], light);

						break;
					}

					case Light::T_point:
					case Light::T_point_multicell:
					{
						for (int slot = 0; light && slot < PointSpecularCount; ++slot)
							if (!ms_currentLights.pointSpecular[slot] || light->getSpecularIntensity() > ms_currentLights.pointSpecular[slot]->getSpecularIntensity())
								swapLight(ms_currentLights.pointSpecular[slot], light);

						for (int slot = 0; light && slot < PointCount; ++slot)
							if (!ms_currentLights.point[slot] || light->getDiffuseIntensity() > ms_currentLights.point[slot]->getDiffuseIntensity())
								swapLight(ms_currentLights.point[slot], light);

						break;
					}

					case Light::T_spot:
					{
						// Spot lights are discarded. No shader in the corpus has a cone term.
						break;
					}

					default:
					{
						DEBUG_FATAL(true, ("Direct3d11: unhandled light type %d", static_cast<int>(light->getType())));
						break;
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
		PROFILER_AUTO_BLOCK_DEFINE("Direct3d11_LightManager::applyLights");

		if (
#if PRODUCTION == 0
			ms_disableLightCaching ||
#endif
			ms_currentLights != ms_lastLights)
			applyLights_vertexShader();
		else
			applyLights_vertexShader_dot3();
	}

	ms_lastLights = ms_currentLights;

	// DX9 leaves ms_dirty set and relies on being called once per draw; clearing it here is
	// the same behaviour with one fewer redundant selection, because every setter that can
	// invalidate the choice sets it again.
	ms_dirty = false;
}

// ======================================================================
/**
 * The full upload: twenty-eight registers at c16, four at c60, five pixel registers at 0.
 *
 * Taken when the selection changed.
 */

void Direct3d11_LightManager::applyLights_vertexShader()
{
	LightData lightData;
	Zero(lightData);

	ExtendedLightData extendedLightData;
	Zero(extendedLightData);

	// ----------------------------------------------------------
	// Ambient, clamped non-negative and then floored.
	//
	// The floor is inherited from the x64 DX9 build. Its comment records why: skinned
	// customizable meshes arrive with vColor0 at zero and, in a scene whose parallel sun is
	// near zero, have nothing else to be lit by. A half-grey floor is roughly what the
	// GroundEnvironment ramps produced in normal play.

	lightData.ambient = ms_currentLights.ambient;

	if (lightData.ambient.r < 0.0f) lightData.ambient.r = 0.0f;
	if (lightData.ambient.g < 0.0f) lightData.ambient.g = 0.0f;
	if (lightData.ambient.b < 0.0f) lightData.ambient.b = 0.0f;

	// Report what the floor is doing before it does it. The floor exists for scenes whose real
	// ambient is near zero, and an interior is exactly such a scene, so the question "is this
	// hack what washes out the starport" is answered by the size of the correction, not by
	// reading the code. Bounded, and only when the floor actually changes something.
	{
		// Every Nth firing rather than the first N. The first N are all loading screen, where an
		// empty light list is expected and says nothing about a starport.
		static int firings = 0;
		static int reportsRemaining = 40;
		int const cms_reportEvery = 600;
		bool const floored = (lightData.ambient.r < cms_minimumAmbient) || (lightData.ambient.g < cms_minimumAmbient) || (lightData.ambient.b < cms_minimumAmbient);
		if (floored && reportsRemaining > 0 && ((firings++ % cms_reportEvery) == 0))
		{
			--reportsRemaining;
			WARNING(true, ("Direct3d11 AMBIENT: scene ambient is %.4f %.4f %.4f and the %.2f floor is raising it. %d light(s) in the list.",
				lightData.ambient.r, lightData.ambient.g, lightData.ambient.b, cms_minimumAmbient,
				static_cast<int>(ms_lightList.size())));
		}
	}

	if (lightData.ambient.r < cms_minimumAmbient) lightData.ambient.r = cms_minimumAmbient;
	if (lightData.ambient.g < cms_minimumAmbient) lightData.ambient.g = cms_minimumAmbient;
	if (lightData.ambient.b < cms_minimumAmbient) lightData.ambient.b = cms_minimumAmbient;

	// ----------------------------------------------------------
	// Parallel lights with specular. Slot zero doubles as the dot3 light, which is where the
	// per-pixel lighting in the corpus gets its direction from.

	for (int i = 0; i < ParallelSpecularCount; ++i)
	{
		Light const * const light = ms_currentLights.parallelSpecular[i];

		if (light)
		{
			ParallelSpecularData &parallelSpecular = lightData.parallelSpecular[i];

			// Negated: the engine's light frame points along the light's travel direction and
			// the shaders want the direction TO the light.
			Vector const &direction = light->getObjectFrameK_w();
			parallelSpecular.direction.x = -direction.x;
			parallelSpecular.direction.y = -direction.y;
			parallelSpecular.direction.z = -direction.z;

			VectorArgb const &diffuseColor = light->getDiffuseColor();
			copyClampedRgb(parallelSpecular.diffuseColor, diffuseColor);

			VectorArgb const &specularColor = light->getSpecularColor();
			copyClampedRgb(parallelSpecular.specularColor, specularColor);

			if (i != 0)
				continue;

			// The dot3 block is in OBJECT space, which is what makes it the only part the
			// cheap path has to refresh when an object moves.
			Vector const localCamera = ms_objectToWorldTransform.rotateTranslate_p2l(ms_cameraPosition);
			lightData.dot3.localCameraPosition.x = localCamera.x;
			lightData.dot3.localCameraPosition.y = localCamera.y;
			lightData.dot3.localCameraPosition.z = localCamera.z;
			lightData.dot3.localCameraPosition.w = 1.0f;

			Vector const localDirection = ms_objectToWorldTransform.rotate_p2l(direction);
			lightData.dot3.localDirection.x = -localDirection.x;
			lightData.dot3.localDirection.y = -localDirection.y;
			lightData.dot3.localDirection.z = -localDirection.z;
			lightData.dot3.localDirection.w = Direct3d11_StateCache::getSpecularPower();

			lightData.dot3.diffuseColor    = parallelSpecular.diffuseColor;
			lightData.dot3.diffuseColor.a  = ms_alphaFadeAndBloom.r;
			lightData.dot3.specularColor   = parallelSpecular.specularColor;
			lightData.dot3.specularColor.a = ms_alphaFadeAndBloom.a;

			HemisphericLightData &extendedParallelSpecular = extendedLightData.parallelSpecular[i];
			setExtendedLightData(extendedParallelSpecular, light, diffuseColor, ms_alphaFadeAndBloom.g);

			PixelDot3Data pixelDot3Data;
			pixelDot3Data.localDirection           = lightData.dot3.localDirection;
			pixelDot3Data.diffuseColor             = lightData.dot3.diffuseColor;
			pixelDot3Data.specularColor            = lightData.dot3.specularColor;
			pixelDot3Data.tangentMinusDiffuseColor = extendedParallelSpecular.tangentMinusDiffuseColor;
			pixelDot3Data.tangentMinusBackColor    = extendedParallelSpecular.tangentMinusBackColor;
			pixelDot3Data.tangentMinusBackColor.a  = Direct3d11::getCurrentTimeValue();

			Direct3d11_ConstantBuffers::setPixelShaderConstants(PSCR_dot3LightDirection, &pixelDot3Data, 5);

			ms_alphaFadeOpacityDirty = false;
		}
		else if (i == 0)
		{
			// No parallel-specular light. Inherited from the x64 DX9 build, whose comment
			// records the diagnosis: h_specmap_bump.eft, which is humanoid skin and clothing,
			// computes saturate(vertexDiffuse + dot3LightDiffuseColor + ...), so leaving the
			// dot3 diffuse at zero lights characters by the small vertexDiffuse term alone and
			// they look unlit. Falling back to the scene ambient gives them at least
			// ambient-level light, matching how the ambient term is used elsewhere.

			PixelDot3Data pixelDot3Data;
			Zero(pixelDot3Data);

			pixelDot3Data.diffuseColor.r = lightData.ambient.r;
			pixelDot3Data.diffuseColor.g = lightData.ambient.g;
			pixelDot3Data.diffuseColor.b = lightData.ambient.b;

			pixelDot3Data.diffuseColor.a             = ms_alphaFadeAndBloom.r;
			pixelDot3Data.specularColor.a            = ms_alphaFadeAndBloom.a;
			pixelDot3Data.tangentMinusDiffuseColor.a = ms_alphaFadeAndBloom.g;
			pixelDot3Data.tangentMinusBackColor.a    = Direct3d11::getCurrentTimeValue();

			Direct3d11_ConstantBuffers::setPixelShaderConstants(PSCR_dot3LightDirection, &pixelDot3Data, 5);
		}
	}

	// ----------------------------------------------------------
	// Parallel lights without specular.

	for (int i = 0; i < ParallelCount; ++i)
	{
		Light const * const light = ms_currentLights.parallel[i];
		if (!light)
			continue;

		ParallelData &parallel = lightData.parallel[i];

		Vector const &direction = light->getObjectFrameK_w();
		parallel.direction.x = -direction.x;
		parallel.direction.y = -direction.y;
		parallel.direction.z = -direction.z;

		copyClampedRgb(parallel.diffuseColor, light->getDiffuseColor());
	}

	// ----------------------------------------------------------
	// Point lights with specular.
	//
	// An empty slot gets a constant attenuation of one rather than zero. The shaders divide by
	// the attenuation polynomial unconditionally, so a zeroed slot would divide by zero.

	for (int i = 0; i < PointSpecularCount; ++i)
	{
		Light const * const light = ms_currentLights.pointSpecular[i];

		if (!light)
		{
			lightData.pointSpecular[i].attenuation.k0 = 1.0f;
			continue;
		}

		PointSpecularData &pointSpecular = lightData.pointSpecular[i];

		Vector const &position = light->getPosition_w();
		pointSpecular.position.x = position.x;
		pointSpecular.position.y = position.y;
		pointSpecular.position.z = position.z;
		pointSpecular.position.w = 1.0f;

		copyClampedRgb(pointSpecular.diffuseColor, light->getDiffuseColor());

		pointSpecular.attenuation.k0 = light->getConstantAttenuation();
		pointSpecular.attenuation.k1 = light->getLinearAttenuation();
		pointSpecular.attenuation.k2 = light->getQuadraticAttenuation();
		pointSpecular.attenuation.k3 = 0.0f;

		copyClampedRgb(pointSpecular.specularColor, light->getSpecularColor());
	}

	// ----------------------------------------------------------
	// Point lights without specular.

	for (int i = 0; i < PointCount; ++i)
	{
		Light const * const light = ms_currentLights.point[i];

		if (!light)
		{
			lightData.point[i].attenuation.k0 = 1.0f;
			continue;
		}

		PointData &point = lightData.point[i];

		Vector const &position = light->getPosition_w();
		point.position.x = position.x;
		point.position.y = position.y;
		point.position.z = position.z;
		point.position.w = 1.0f;

		copyClampedRgb(point.diffuseColor, light->getDiffuseColor());

		point.attenuation.k0 = light->getConstantAttenuation();
		point.attenuation.k1 = light->getLinearAttenuation();
		point.attenuation.k2 = light->getQuadraticAttenuation();
		point.attenuation.k3 = 0.0f;
	}

	// TEMPORARY DIAGNOSTIC: white ambient, every other light black. A lit surface then renders as
	// its raw texture, which says whether a wrong colour is coming from the lighting or from the
	// texture without having to guess which.
	Direct3d11_ConstantBuffers::setVertexShaderConstants(VSCR_lightData, &lightData, isizeof(LightData) / cms_registerBytes);
	Direct3d11_ConstantBuffers::setVertexShaderConstants(VCSR_extendedLightData, &extendedLightData, isizeof(ExtendedLightData) / cms_registerBytes);
}

// ======================================================================
/**
 * Build the hemispheric colours for the dot3 light.
 *
 * The synthesis branch is inherited from the x64 DX9 build. Its comment records the diagnosis:
 * h_specmap_bump.eft's calculateHemisphericLighting computes
 *
 *   light = vertexDiffuse + tmd + dot3Diffuse - max(0,dp)*tmd + min(0,dp)*tmb
 *
 * with tmd = tangent - diffuse and tmb = tangent - back. A plain directional sun carries no
 * hemispheric colours, so both arrive as black, tmd becomes -diffuse, and the expression
 * collapses to vertexDiffuse + max(0,dp)*diffuse -- every surface facing away from the sun
 * falls to the ambient floor. Synthesising a hemisphere from the light's own diffuse gives the
 * shader something sensible to subtract instead of subtracting the light away.
 */

void Direct3d11_LightManager::setExtendedLightData(
	HemisphericLightData &extendedParallelSpecular,
	Light const          *light,
	VectorArgb const     &diffuseColor,
	float                 bloomEnabled)
{
	NOT_NULL(light);

	VectorArgb diffuseBackColor    = light->getDiffuseBackColor();
	VectorArgb diffuseTangentColor = light->getDiffuseTangentColor();

	{
		float const tangentMagnitude = diffuseTangentColor.r + diffuseTangentColor.g + diffuseTangentColor.b;
		float const backMagnitude    = diffuseBackColor.r    + diffuseBackColor.g    + diffuseBackColor.b;

		if (tangentMagnitude <= 0.001f && backMagnitude <= 0.001f)
		{
			// Horizon at 65% of the key light, ground bounce at 30%. The shadow side lands near
			// vertexDiffuse + 0.65 * diffuse, which is visible, while the lit side still
			// reaches full diffuse.
			diffuseTangentColor.r = diffuseColor.r * cms_syntheticTangentScale;
			diffuseTangentColor.g = diffuseColor.g * cms_syntheticTangentScale;
			diffuseTangentColor.b = diffuseColor.b * cms_syntheticTangentScale;
			diffuseTangentColor.a = diffuseColor.a;

			diffuseBackColor.r = diffuseColor.r * cms_syntheticBackScale;
			diffuseBackColor.g = diffuseColor.g * cms_syntheticBackScale;
			diffuseBackColor.b = diffuseColor.b * cms_syntheticBackScale;
			diffuseBackColor.a = diffuseColor.a;
		}
	}

	extendedParallelSpecular.backColor.r = diffuseBackColor.r;
	extendedParallelSpecular.backColor.g = diffuseBackColor.g;
	extendedParallelSpecular.backColor.b = diffuseBackColor.b;
	extendedParallelSpecular.backColor.a = diffuseBackColor.a;

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
	extendedParallelSpecular.tangentMinusDiffuseColor.a = bloomEnabled;
}

// ======================================================================
/**
 * The cheap upload: the dot3 block only.
 *
 * Taken when the selection is unchanged. Two registers if nothing but the object moved, four
 * plus the extended one and up to five pixel registers if the alpha fade or bloom also
 * changed. This is the common case in a frame -- adjacent objects usually share their lights.
 */

void Direct3d11_LightManager::applyLights_vertexShader_dot3()
{
	// The dot3 block's base register, derived from the struct rather than written down, so it
	// cannot drift from the layout the static_asserts guard.
	int const vertexDot3Register = VSCR_lightData + (static_cast<int>(offsetof(LightData, dot3)) / cms_registerBytes);

	Light const * const light = ms_currentLights.parallelSpecular[0];

	if (light)
	{
		Dot3Data dot3Data;
		Zero(dot3Data);

		Vector const localCamera = ms_objectToWorldTransform.rotateTranslate_p2l(ms_cameraPosition);
		dot3Data.localCameraPosition.x = localCamera.x;
		dot3Data.localCameraPosition.y = localCamera.y;
		dot3Data.localCameraPosition.z = localCamera.z;
		dot3Data.localCameraPosition.w = 1.0f;

		Vector const &direction = light->getObjectFrameK_w();
		Vector const localDirection = ms_objectToWorldTransform.rotate_p2l(direction);
		dot3Data.localDirection.x = -localDirection.x;
		dot3Data.localDirection.y = -localDirection.y;
		dot3Data.localDirection.z = -localDirection.z;
		dot3Data.localDirection.w = Direct3d11_StateCache::getSpecularPower();

		if (!ms_alphaFadeOpacityDirty)
		{
			// Only the two object-space registers moved.
			Direct3d11_ConstantBuffers::setPixelShaderConstants(PSCR_dot3LightDirection, &dot3Data.localDirection, 1);
			Direct3d11_ConstantBuffers::setVertexShaderConstants(vertexDot3Register, &dot3Data, 2);
			return;
		}

		ms_alphaFadeOpacityDirty = false;

		VectorArgb const &diffuseColor = light->getDiffuseColor();
		dot3Data.diffuseColor.r = diffuseColor.r;
		dot3Data.diffuseColor.g = diffuseColor.g;
		dot3Data.diffuseColor.b = diffuseColor.b;
		dot3Data.diffuseColor.a = ms_alphaFadeAndBloom.r;

		VectorArgb const &specularColor = light->getSpecularColor();
		dot3Data.specularColor.r = specularColor.r;
		dot3Data.specularColor.g = specularColor.g;
		dot3Data.specularColor.b = specularColor.b;
		dot3Data.specularColor.a = ms_alphaFadeAndBloom.a;

		ExtendedLightData extendedLightData;
		Zero(extendedLightData);
		HemisphericLightData &extendedParallelSpecular = extendedLightData.parallelSpecular[0];
		setExtendedLightData(extendedParallelSpecular, light, diffuseColor, ms_alphaFadeAndBloom.g);

		PixelDot3Data pixelDot3Data;
		pixelDot3Data.localDirection           = dot3Data.localDirection;
		pixelDot3Data.diffuseColor             = dot3Data.diffuseColor;
		pixelDot3Data.specularColor            = dot3Data.specularColor;
		pixelDot3Data.tangentMinusDiffuseColor = extendedParallelSpecular.tangentMinusDiffuseColor;
		pixelDot3Data.tangentMinusBackColor    = extendedParallelSpecular.tangentMinusBackColor;
		pixelDot3Data.tangentMinusBackColor.a  = Direct3d11::getCurrentTimeValue();

		Direct3d11_ConstantBuffers::setPixelShaderConstants(PSCR_dot3LightDirection, &pixelDot3Data, 5);
		Direct3d11_ConstantBuffers::setVertexShaderConstants(vertexDot3Register, &dot3Data, 4);

		int const tangentMinusDiffuseRegister = VCSR_extendedLightData + (static_cast<int>(offsetof(ExtendedLightData, parallelSpecular[0].tangentMinusDiffuseColor)) / cms_registerBytes);
		Direct3d11_ConstantBuffers::setVertexShaderConstants(tangentMinusDiffuseRegister, &extendedParallelSpecular.tangentMinusDiffuseColor, 1);

		return;
	}

	// No parallel-specular light. Nothing object-space to refresh, so this only has work to do
	// when the alpha fade or bloom changed.
	if (!ms_alphaFadeOpacityDirty)
		return;

	ms_alphaFadeOpacityDirty = false;

	Dot3Data dot3Data;
	Zero(dot3Data);
	dot3Data.diffuseColor.a  = ms_alphaFadeAndBloom.r;
	dot3Data.specularColor.a = ms_alphaFadeAndBloom.a;

	ExtendedLightData extendedLightData;
	Zero(extendedLightData);
	HemisphericLightData &extendedParallelSpecular = extendedLightData.parallelSpecular[0];
	extendedParallelSpecular.tangentMinusDiffuseColor.a = ms_alphaFadeAndBloom.g;

	// Zeroed, unlike DX9, which declares this uninitialised and then uploads four registers
	// starting at diffuseColor -- so the rgb of tangentMinusBackColor is whatever the stack
	// held. That cannot be reproduced faithfully because it is not deterministic, and zero is
	// the value the rest of this path uses.
	PixelDot3Data pixelDot3Data;
	Zero(pixelDot3Data);
	pixelDot3Data.diffuseColor             = dot3Data.diffuseColor;
	pixelDot3Data.specularColor            = dot3Data.specularColor;
	pixelDot3Data.tangentMinusDiffuseColor = extendedParallelSpecular.tangentMinusDiffuseColor;
	pixelDot3Data.tangentMinusBackColor.a  = Direct3d11::getCurrentTimeValue();

	int const pixelAlphaFadeBloomRegister = PSCR_dot3LightDirection + (static_cast<int>(offsetof(PixelDot3Data, diffuseColor)) / cms_registerBytes);
	Direct3d11_ConstantBuffers::setPixelShaderConstants(pixelAlphaFadeBloomRegister, &pixelDot3Data.diffuseColor, 4);

	int const vertexAlphaFadeBloomRegister = VSCR_lightData + (static_cast<int>(offsetof(LightData, dot3.diffuseColor)) / cms_registerBytes);
	Direct3d11_ConstantBuffers::setVertexShaderConstants(vertexAlphaFadeBloomRegister, &dot3Data.diffuseColor, 2);

	int const tangentMinusDiffuseRegister = VCSR_extendedLightData + (static_cast<int>(offsetof(ExtendedLightData, parallelSpecular[0].tangentMinusDiffuseColor)) / cms_registerBytes);
	Direct3d11_ConstantBuffers::setVertexShaderConstants(tangentMinusDiffuseRegister, &extendedParallelSpecular.tangentMinusDiffuseColor, 1);
}

// ======================================================================
