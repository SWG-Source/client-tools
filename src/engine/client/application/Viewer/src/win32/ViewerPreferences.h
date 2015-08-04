//===================================================================
//
// ViewerPreferences.h
// asommers 
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ViewerPreferences_H
#define INCLUDED_ViewerPreferences_H

//===================================================================

#include "clientGraphics/Graphics.def"

class Camera;
class CellProperty;
class LightManager;
class Iff;
class Light;
class Object;
class ObjectList;
class Shader;
class ShaderPrimitive;
class StaticShader;
class Texture;
class VectorArgb;

//===================================================================

class ViewerPreferences
{
public:

	enum CameraMode
	{
		CM_maya,
		CM_free
	};

	enum LightMode
	{
		LM_off,
		LM_full,
		LM_quarterAmbientFullDirectional,
		LM_userDefined
	};

public:

	static void        install ();
	static void        remove ();

	static void        alter (float elapsedTime);

	static bool        load (const char* fileName);
	static bool        save (const char* fileName);

	static LightMode   getLightMode ();
	static void        setLightMode (LightMode lightMode);
	static void        setShowHemisphericColors(bool showColors);
	static bool        getShowHemisphericColors();
	static GlFillMode  getFillMode ();
	static void        setFillMode (GlFillMode fillMode);
	static bool        getShowFaceNormals ();
	static void        setShowFaceNormals (bool showFaceNormals);
	static bool        getShowVertexNormals ();
	static void        setShowVertexNormals (bool showVertexNormals);
	static bool        getShowExtents ();
	static void        setShowExtents (bool showExtents);
	static bool        getShowObject ();
	static void        setShowObject (bool showObject);
	static bool        getShowSkeleton ();
	static void        setShowSkeleton (bool showSkeleton);
	static bool        getAnimateLight ();
	static void        setAnimateLight (bool animateLight);

	static void        beginScene (Object* documentObject, CellProperty* cellProperty);
	static void        endScene ();

	static const VectorArgb& getClearColor ();
	static void        setClearColor (const VectorArgb& clearColor);
	static bool        getAlternateRgb ();
	static void        setAlternateRgb (bool alternateRgb);
	static bool        getShowAxis ();
	static void        setShowAxis (bool showAxis);
	static bool        getShowGrid ();
	static void        setShowGrid (bool showGrid);
	static CameraMode  getCameraMode ();
	static void        setCameraMode (CameraMode cameraMode);
	static void        setLightYaw (float lightYaw);
	static float       getLightYaw ();
	static void        setLightPitch (float lightPitch);
	static float       getLightPitch ();

	static LightManager& getLightManager ();
	static const LightManager& getConstLightManager ();

	static bool        getEnableWheelZoom ();
	static void        setEnableWheelZoom (bool enableWheelZoom);
	static float       getWheelZoomScale ();
	static void        setWheelZoomScale (float wheelZoomScale);
	static bool        getDisplayStatistics ();
	static void        setDisplayStatistics (bool displayStatistics);
	static bool        getEnableTextures ();
	static void        setEnableTextures (bool enableTextures);
     
	static const Shader* getOverrideShader ();
	static bool        getUseOverrideShader ();
	static void        setUseOverrideShader (bool useOverrideShader);
	static void        setOverrideShaderTemplateName (const char* overrideShaderTemplateName);
	static const char* getOverrideShaderTemplateName ();

	static bool        getFogEnabled ();
	static void        setFogEnabled (bool fogEnabled);
	static const VectorArgb& getFogColor ();
	static void        setFogColor (const VectorArgb& FogColor);
	static float       getFogDensity ();
	static void        setFogDensity (float fogEnabled);

	static bool        getLowPriority ();
	static void        setLowPriority (bool lowPriority);

	static void        unload ();
	static void        reload ();
	static void        reloadLights ();

	static bool        getShowHardpoints ();
	static void        setShowHardpoints (bool showHardpoints);

	static bool        getShowCMeshes ();
	static void        setShowCMeshes (bool showCMeshes);

	static bool        getShowColExtents ();
	static void        setShowColExtents (bool showExtents);

	static bool        getShowFloors ();
	static void        setShowFloors (bool showFloors);

	static bool        getShowPathNodes ();
	static void        setShowPathNodes (bool showPathNodes);

	static void        setEnvironmentMapName (const char* environmentMapName);
	static const char* getEnvironmentMapName ();

	static bool        getLowerReferencePlane ();
	static void        setLowerReferencePlane (bool lowerReferencePlane);

	static bool        getBloomEnabled ();
	static void        setBloomEnabled (bool bloomEnabled);

private:

	static StaticShader const & prepareToViewOverrideFunction(ShaderPrimitive const & shaderPrimitive);

private:

	static GlFillMode       m_fillMode;
	static bool             m_showFaceNormals;
	static bool             m_showVertexNormals;
	static bool             m_showExtents;
	static bool             m_showObject;
	static bool             m_showSkeleton;

	static bool             m_alternateRgb;
	static VectorArgb       m_clearColor;
	static CameraMode       m_cameraMode;
	static bool             m_fogEnabled;
	static VectorArgb       m_fogColor;
	static float            m_fogDensity;
	static bool             m_enableWheelZoom;
	static float            m_wheelZoomScale;

	static LightMode        m_lightMode;
	static Light*           m_fullAmbientLight;
	static Light*           m_quarterAmbientLight;
	static Light*           m_parallelLight;
	static bool             m_animateLight;
	static float            m_lightYaw;
	static float            m_lightPitch;
	static ObjectList*      m_userLightList;
	static bool             m_lightShowHemisphericColors;
	static LightManager     m_lightManager;

	static Object*          m_grid;
	static Object*          m_referencePlane;
	static bool             m_lowerReferencePlane;
	static bool             m_showAxis;
	static bool             m_showGrid;
	static bool             m_displayStatistics;
	static bool             m_enableTextures;

	static const Shader*    m_overrideShader;
	static bool             m_useOverrideShader;
	static std::string      m_overrideShaderTemplateName;

	static bool             m_lowPriority;
	static bool             m_showHardpoints;
	static bool             m_showCMeshes;
	static bool             m_showColExtents;
	static bool             m_showFloors;
	static bool             m_showPathNodes;

	static std::string      m_environmentMapName;

	static bool             m_bloom;
	
private:

	static void load (Iff& iff);
	static void load_0000 (Iff& iff);
	static void load_0001 (Iff& iff);
	static void load_0002 (Iff& iff);
	static void load_0003 (Iff& iff);
	static void load_0004 (Iff& iff);
	static void save (Iff& iff);
};

//===================================================================

#endif
