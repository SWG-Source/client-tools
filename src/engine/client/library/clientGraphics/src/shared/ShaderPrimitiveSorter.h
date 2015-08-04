// ======================================================================
//
// ShaderPrimitiveSorter.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ShaderPrimitiveSorter_H
#define INCLUDED_ShaderPrimitiveSorter_H

// ======================================================================

class Camera;
class CellProperty;
class Light;
class PackedArgb;
class Rectangle2d;
class Shader;
class ShaderPrimitive;
class StaticShader;
class Texture;
class Vector;

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

// ======================================================================

class ShaderPrimitiveSorter
{
public:

	enum
	{
		MAX_NUMBER_OF_LIGHTS = 32
	};

	enum Sort
	{
		S_unknown,
		S_performance,
		S_fifo,
		S_painters
	};

	enum DrawTime
	{
		D_unknown,
		D_beforeNextPush,
		D_beforeCurrentPop,
	};

	typedef stdvector<const Light *>::fwd        LightList;
	typedef std::bitset<MAX_NUMBER_OF_LIGHTS>    LightBitSet;

	class Phase;
	friend class Phase;

	typedef bool(*BelowTransparentWaterFunction)(const Vector& position_w);
	typedef StaticShader const & (*PrepareToViewOverrideFunction)(const ShaderPrimitive &shaderPrimitive);

public:

	static void               install(int numberOfPhases);
	static bool               isInstalled();

	static void               setPhase(Tag tag, int index);
	static int                getPhase(Tag tag);
	static void               setPhaseSort(int phase, Sort sort);
	static void               setPhaseDrawTime(int phase, DrawTime drawTime);

#ifdef _DEBUG
	static void               setPhaseDebugName(int phase, const char *debugName);
#endif

	static void               setCurrentCamera(const Camera &camera);
	static void               clearCurrentCamera();
	static const Camera      &getCurrentCamera();
	static const Vector      &getCurrentCameraPosition();

	static void               enableLight(const Light &light);
	static void               disableLight(const Light &light);
	static void               add(const ShaderPrimitive &shaderPrimitive);
	static void               add(const ShaderPrimitive &shaderPrimitive, int phase);
	static void               addWithAlphaFadeOpacity(const ShaderPrimitive &shaderPrimitive, bool enableOpaque, float opaqueAlpha, bool enableAlpha, float alphaAlpha);

	static const LightList   &getLightList();
	static const LightBitSet &getLightsAffectingShadersWithoutPrecalculatedVertexLighting();
	static const LightBitSet &getLightsAffectingShadersWithPrecalculatedVertexLighting();
	static void               add(const ShaderPrimitive &shaderPrimitive, const LightBitSet &lightBitSet);

	static void               pushCell(Texture const * environmentTexture, bool fogEnabled, float fogDensity, PackedArgb const & fogColor);
	static void               pushCell(CellProperty const & cellProperty);
	static void               popCell();

	static void               setPrepareToViewOverrideFunction(PrepareToViewOverrideFunction prepareToViewOverrideFunction);

	static void               setBelowTransparentWaterFunctionAndPhases(BelowTransparentWaterFunction belowTransparentWaterFunction, int belowTransparentWaterTestPhase, int belowTransparentWaterPhase, int aboveWaterPhase);

	static void alter(float elapsedTimeSecs);

	static void setHeatShadersEnabled(bool enabled);
	static bool getHeatShadersEnabled();

	static bool getHeatShadersCapable();

	static bool startCompositing(ShaderPrimitive const & shaderPrimitive, int & sectionRectX0, int & sectionRectY0, int & sectionRectX1, int & sectionRectY1);
	static void finishCompositing(int sectionRectX0, int sectionRectY0, int sectionRectX1, int sectionRectY1, StaticShader & compositingShader);

	static bool getUseClipRectangle();
	static Rectangle2d const & getClipRectangle();
	static int getClipEnvironmentFlags();

	static void setUseClipRectangle(bool b);
	static void setClipRectangle(float left, float bottom, float right, float top);
	static void setClipEnvironmentFlags(int flags);
	static void setUseWaterTests(bool b);

private:

	typedef stdmap<Tag, int>::fwd                PhaseMap;
	typedef stdvector<Phase>::fwd                Phases;
	typedef stdvector<Texture const *>::fwd      EnvironmentTextures;

#ifdef _DEBUG
	struct DebugPhase;
	typedef stdvector<DebugPhase>::fwd DebugPhaseList;
#endif

private:

	static void remove();
	static void setLights(const LightBitSet &);
	static void pushCell(CellProperty const * cellProperty, Texture const * environmentTexture, bool fogEnabled, float fogDensity, PackedArgb const & fogColor);

#ifdef _DEBUG
	static void debugDump();
#endif

	static StaticShader const & defaultPrepareToView(const ShaderPrimitive &);

	// disabled
	ShaderPrimitiveSorter();
	ShaderPrimitiveSorter(const ShaderPrimitiveSorter &);
	ShaderPrimitiveSorter &operator =(const ShaderPrimitiveSorter &);

private:


	static Phases                     ms_phase;
	static PhaseMap                   ms_phaseMap;
	static const Camera              *ms_currentCamera;
	static Vector                     ms_currentCameraPosition;
	static LightList                  ms_lightList;
	static LightList                  ms_activeLightList;
	static LightBitSet                ms_lightsAffectingShadersWithoutPrecalculatedVertexLighting;
	static LightBitSet                ms_lightsAffectingShadersWithPrecalculatedVertexLighting;
	static stdvector<int>::fwd        ms_lightListStackOffset;
	static Texture const             *ms_defaultEnvironmentTexture;
	static EnvironmentTextures        ms_environmentTextures;
	static PrepareToViewOverrideFunction ms_prepareToViewFunction;

	static BelowTransparentWaterFunction ms_belowTransparentWaterFunction;
	static int                        ms_belowTransparentWaterTestPhase;
	static int                        ms_belowTransparentWaterPhase;
	static int                        ms_aboveWaterPhase;

	static bool                       ms_popped;


	static float                      ms_elapsedTime;
	static bool                       ms_heatShadersEnabled;

	static Rectangle2d ms_clipRectangle;
	static bool ms_useClipRectangle;
	static int ms_clipEnvironmentFlags;
	static bool ms_useWaterTests;

#ifdef _DEBUG
	static bool                       ms_lightListOverflow;
	static bool                       ms_debugReport;
	static DebugPhaseList             ms_debugPhase;
#endif
};

// ======================================================================

inline const ShaderPrimitiveSorter::LightList &ShaderPrimitiveSorter::getLightList()
{
	return ms_lightList;
}

// ----------------------------------------------------------------------

inline const ShaderPrimitiveSorter::LightBitSet &ShaderPrimitiveSorter::getLightsAffectingShadersWithoutPrecalculatedVertexLighting()
{
	return ms_lightsAffectingShadersWithoutPrecalculatedVertexLighting;
}

// ----------------------------------------------------------------------

inline const ShaderPrimitiveSorter::LightBitSet &ShaderPrimitiveSorter::getLightsAffectingShadersWithPrecalculatedVertexLighting()
{
	return ms_lightsAffectingShadersWithPrecalculatedVertexLighting;
}

// ----------------------------------------------------------------------

inline const Camera &ShaderPrimitiveSorter::getCurrentCamera()
{
	NOT_NULL(ms_currentCamera);
	return *ms_currentCamera;
}

// ----------------------------------------------------------------------

inline const Vector &ShaderPrimitiveSorter::getCurrentCameraPosition()
{
	NOT_NULL(ms_currentCamera);
	return ms_currentCameraPosition;
}

//----------------------------------------------------------------------

inline bool ShaderPrimitiveSorter::getUseClipRectangle()
{
	return ms_useClipRectangle;
}

//----------------------------------------------------------------------

inline Rectangle2d const & ShaderPrimitiveSorter::getClipRectangle()
{
	return ms_clipRectangle;
}

//----------------------------------------------------------------------

inline int ShaderPrimitiveSorter::getClipEnvironmentFlags()
{
	return ms_clipEnvironmentFlags;
}

// ======================================================================

#endif
