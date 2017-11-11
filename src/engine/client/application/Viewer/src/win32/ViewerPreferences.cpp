//===================================================================
//
// ViewerPreferences.cpp
// asommers 
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "FirstViewer.h"
#include "ViewerPreferences.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientObject/GridAppearance.h"
#include "clientObject/LightManager.h"
#include "clientObject/ShadowManager.h"
#include "clientObject/ShadowVolume.h"
#include "clientObject/ShaderAppearance.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Os.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectList.h"

//===================================================================

namespace
{
	const Tag TAG_ENVM = TAG (E,N,V,M);
	const Tag TAG_VPRF = TAG (V,P,R,F);
}

//===================================================================
// PUBLIC STATIC ViewerPreferences
//===================================================================

void ViewerPreferences::install ()
{
	//-- full ambient light
	m_fullAmbientLight = new Light (Light::T_ambient, VectorArgb::solidWhite);

	//-- quarter ambient light
	m_quarterAmbientLight = new Light (Light::T_ambient, VectorArgb (1.f, 0.33f, 0.33f, 0.33f));

	//-- full directional light
	m_parallelLight = new Light (Light::T_parallel, VectorArgb (1.0f, 1.0f, 1.0f, 1.0f));
	m_parallelLight->setSpecularColor (VectorArgb::solidWhite);

	//-- user defined lights
	m_lightManager.load ("defaultLights.iff");

	m_userLightList = new ObjectList (m_lightManager.getNumberOfLights ());

	reloadLights ();

	//-- create reference plane
	m_referencePlane = new Object ();
	m_referencePlane->setAppearance (new ShaderAppearance (10.f, 10.f, ShaderTemplateList::get3dVertexColorZStaticShader ().getShaderTemplate ().fetchModifiableShader (), VectorArgb::solidGray));
	m_referencePlane->pitch_o (PI_OVER_2);
	RenderWorld::addObjectNotifications (*m_referencePlane);

	//-- grid plane
	m_grid = new Object ();
	m_grid->setAppearance (new GridAppearance (GridAppearance::GP_XZ, 20.f, 21, VectorArgb::solidGray));
	RenderWorld::addObjectNotifications (*m_grid);

	m_overrideShaderTemplateName = "shader/test_shader.sht";
	m_environmentMapName = TextureList::getDefaultEnvironmentTextureName ();

	ConfigSharedCollision::setBuildDebugData(true);

	reload ();
}

//-------------------------------------------------------------------

void ViewerPreferences::remove ()
{
	delete m_fullAmbientLight;
	m_fullAmbientLight = 0;

	delete m_quarterAmbientLight;
	m_quarterAmbientLight = 0;

	delete m_parallelLight;
	m_parallelLight = 0;

	m_lightManager.save ("defaultLights.iff");

	m_userLightList->removeAll (true);
	delete m_userLightList;
	m_userLightList = 0;

	delete m_referencePlane;
	m_referencePlane = 0;

	delete m_grid;
	m_grid = 0;

	CellProperty::getWorldCellProperty ()->setEnvironmentTexture (0);

	unload ();
}

//-------------------------------------------------------------------

void ViewerPreferences::alter (float elapsedTime)
{
	if (m_animateLight)
		m_lightYaw += elapsedTime * 0.5f;
}

//-------------------------------------------------------------------

bool ViewerPreferences::load (const char* const fileName)
{
	Iff iff;
	if (iff.open (fileName, true))
	{
		load (iff);

		return true;
	}

	return false;
}

//-------------------------------------------------------------------

bool ViewerPreferences::save (const char* const fileName)
{
	Iff iff (1024);
	iff.insertForm (TAG_VPRF);
		save (iff);
	iff.exitForm (TAG_VPRF);

	return iff.write (fileName, true);
}

//-------------------------------------------------------------------

ViewerPreferences::LightMode ViewerPreferences::getLightMode ()
{
	return m_lightMode;
}

//-------------------------------------------------------------------

void ViewerPreferences::setLightMode (ViewerPreferences::LightMode lightMode)
{
	m_lightMode = lightMode;
}

//-------------------------------------------------------------------

void ViewerPreferences::setShowHemisphericColors(bool showColors)
{
	if (m_lightShowHemisphericColors != showColors)
	{
		m_lightShowHemisphericColors = showColors;
	}
}

//-------------------------------------------------------------------

bool ViewerPreferences::getShowHemisphericColors()
{
	return m_lightShowHemisphericColors;
}

//-------------------------------------------------------------------

GlFillMode ViewerPreferences::getFillMode ()
{
	return m_fillMode;
}

//-------------------------------------------------------------------

void ViewerPreferences::setFillMode (GlFillMode fillMode)
{
	m_fillMode = fillMode;
}


//-------------------------------------------------------------------

bool ViewerPreferences::getShowFaceNormals ()
{
	return m_showFaceNormals;
}

//-------------------------------------------------------------------

void ViewerPreferences::setShowFaceNormals (bool showFaceNormals)
{
	m_showFaceNormals = showFaceNormals;
}

//-------------------------------------------------------------------

bool ViewerPreferences::getShowVertexNormals ()
{
	return m_showVertexNormals;
}

//-------------------------------------------------------------------

void ViewerPreferences::setShowVertexNormals (bool showVertexNormals)
{
	m_showVertexNormals = showVertexNormals;
}

//-------------------------------------------------------------------

bool ViewerPreferences::getShowExtents ()
{
	return m_showExtents;
}

//-------------------------------------------------------------------

void ViewerPreferences::setShowExtents (bool showExtents)
{
	m_showExtents = showExtents;
}

//-------------------------------------------------------------------

bool ViewerPreferences::getShowObject ()
{
	return m_showObject;
}

//-------------------------------------------------------------------

void ViewerPreferences::setShowObject (bool showObject)
{
	m_showObject = showObject;
}

//-------------------------------------------------------------------

bool ViewerPreferences::getShowSkeleton ()
{
	return m_showSkeleton;
}

//-------------------------------------------------------------------

void ViewerPreferences::setShowSkeleton (bool showSkeleton)
{
	m_showSkeleton = showSkeleton;
}

//-------------------------------------------------------------------

bool ViewerPreferences::getAnimateLight ()
{
	return m_animateLight;
}

//-------------------------------------------------------------------

void ViewerPreferences::setAnimateLight (bool animateLight)
{
	m_animateLight = animateLight;
}

//-------------------------------------------------------------------

void ViewerPreferences::reloadLights ()
{
	m_userLightList->removeAll (true);

	int i;
	for (i = 0; i < m_lightManager.getNumberOfLights (); ++i)
		m_userLightList->addObject (m_lightManager.createLight (i));
}

//-------------------------------------------------------------------

void ViewerPreferences::beginScene (Object* const documentObject, CellProperty* const cellProperty)
{
	UNREF (documentObject);
	Os::setProcessPriority (m_lowPriority ? Os::P_low : Os::P_normal);

	//-- set the cell's global texture
	Texture const * const texture = TextureList::fetch(!m_environmentMapName.empty() ? m_environmentMapName.c_str() : "texture/defaultcubemap.dds");
	cellProperty->setEnvironmentTexture (texture);
	CellProperty::getWorldCellProperty ()->setEnvironmentTexture (texture);
	texture->release ();

	//-- clear viewport
	VectorArgb clearColor = m_clearColor;

	if (m_alternateRgb)
	{
		static int clearColorToggle = 0;

		switch (++clearColorToggle % 3)
		{
		case 0: clearColor = VectorArgb::solidRed;    break;
		case 1: clearColor = VectorArgb::solidGreen;  break;
		case 2: clearColor = VectorArgb::solidBlue;   break;
		}		
	}

	Graphics::clearViewport (true, clearColor.convertToUint32 (), true, 1.0, true, 0);

	//-- set the draw mode
	Graphics::setFillMode (m_fillMode);

	//-- textures
#ifdef _DEBUG
	Graphics::setTexturesEnabled (m_enableTextures);
#endif

	m_parallelLight->setParentCell (cellProperty);
	m_parallelLight->resetRotateTranslate_o2p ();
	m_parallelLight->yaw_o (m_lightYaw);
	m_parallelLight->pitch_o (m_lightPitch);

	m_parallelLight->setDiffuseTangentColor(m_lightShowHemisphericColors ? VectorArgb::solidRed : VectorArgb::solidBlack);
	m_parallelLight->setDiffuseBackColor(m_lightShowHemisphericColors ? VectorArgb::solidBlue : VectorArgb::solidBlack);

	//-- add lights to scene
	switch (m_lightMode)
	{
	case LM_off:
		{
		}
		break;

	case LM_full:
		{
			m_fullAmbientLight->setParentCell (cellProperty);
			m_fullAmbientLight->addToWorld ();
		}
		break;

	case LM_quarterAmbientFullDirectional:
		{
			m_quarterAmbientLight->setParentCell (cellProperty);
			m_quarterAmbientLight->addToWorld ();

			m_parallelLight->addToWorld ();
		}
		break;

	case LM_userDefined:
		{
			int i;
			for (i = 0; i < m_userLightList->getNumberOfObjects (); ++i)
			{
				Light* const light = safe_cast<Light*> (m_userLightList->getObject (i));
				light->setParentCell (cellProperty);
				light->addToWorld ();
			}
		}
		break;
	}

	//-- grid
	if (m_showGrid && m_grid && !m_grid->isInWorld ())
	{
		m_grid->setParentCell(cellProperty);
		m_grid->addToWorld ();
	}

	//-- reference plane
	if (ShadowManager::getEnabled () && m_referencePlane && !m_referencePlane->isInWorld ())
	{
		m_referencePlane->setScale (Vector::xyz111 * 10.f * documentObject->getAppearanceSphereRadius ());
		m_referencePlane->setParentCell(cellProperty);

		if (m_lowerReferencePlane)
		{
			BoxExtent const * boxExtent = dynamic_cast<BoxExtent const *> (documentObject->getAppearance ()->getExtent ());
			if (boxExtent)
				m_referencePlane->setPosition_p (Vector (0.f, boxExtent->getMin ().y, 0.f));
		}

		m_referencePlane->addToWorld ();
	}

	const Vector toLight = -m_parallelLight->getObjectFrameK_w ();

	//-- shadows
	ShadowVolume::setDirectionToLight (toLight, true);
//	ShadowVolume::setShadowColor (VectorArgb::solidBlack);
	ShadowVolume::setShadowColor (VectorArgb (0.5f, 0.f, 0.f, 0.f));

	//-- fog
	cellProperty->setFogEnabled(m_fogEnabled);
	cellProperty->setFogColor(m_fogColor);
	cellProperty->setFogDensity(m_fogDensity);

	//-- override shader
	if (m_useOverrideShader && m_overrideShader)
		ShaderPrimitiveSorter::setPrepareToViewOverrideFunction(prepareToViewOverrideFunction);
}

//-------------------------------------------------------------------

void ViewerPreferences::endScene ()
{
	//-- override shader
	if (m_useOverrideShader)
		ShaderPrimitiveSorter::setPrepareToViewOverrideFunction(0);

	//-- reset fill mode
	Graphics::setFillMode (GFM_solid);

	m_parallelLight->setParentCell(CellProperty::getWorldCellProperty());

	//-- remove lights from scene
	switch (m_lightMode)
	{
	case LM_off:
		{
		}
		break;

	case LM_full:
		{
			m_fullAmbientLight->setParentCell(CellProperty::getWorldCellProperty());
			m_fullAmbientLight->removeFromWorld ();
		}
		break;

	case LM_quarterAmbientFullDirectional:
		{
			m_quarterAmbientLight->setParentCell(CellProperty::getWorldCellProperty());
			m_quarterAmbientLight->removeFromWorld ();

			m_parallelLight->removeFromWorld ();
		}
		break;

	case LM_userDefined:
		{
			int i;
			for (i = 0; i < m_userLightList->getNumberOfObjects (); ++i)
			{
				m_userLightList->getObject (i)->setParentCell(CellProperty::getWorldCellProperty());
				m_userLightList->getObject (i)->removeFromWorld ();
			}
		}
		break;
	}

	//-- reference plane
	if (m_referencePlane && m_referencePlane->isInWorld ())
	{
		m_referencePlane->setParentCell(CellProperty::getWorldCellProperty());
		m_referencePlane->removeFromWorld ();
	}

	//-- grid
	if (m_grid && m_grid->isInWorld ())
	{
		m_grid->setParentCell(CellProperty::getWorldCellProperty());
		m_grid->removeFromWorld ();
	}

	//-- textures
#ifdef _DEBUG
	Graphics::setTexturesEnabled (true);
#endif
}

//-------------------------------------------------------------------

const VectorArgb& ViewerPreferences::getClearColor ()
{
	return m_clearColor;
}

//-------------------------------------------------------------------

void ViewerPreferences::setClearColor (const VectorArgb& clearColor)
{
	m_clearColor = clearColor;
}

//-------------------------------------------------------------------

bool ViewerPreferences::getAlternateRgb ()
{
	return m_alternateRgb;
}

//-------------------------------------------------------------------

void ViewerPreferences::setAlternateRgb (bool alternateRgb)
{
	m_alternateRgb = alternateRgb;
}

//-------------------------------------------------------------------

bool ViewerPreferences::getShowAxis ()
{
	return m_showAxis;
}

//-------------------------------------------------------------------

void ViewerPreferences::setShowAxis (bool showAxis)
{
	m_showAxis = showAxis;
}

//-------------------------------------------------------------------

bool ViewerPreferences::getShowGrid ()
{
	return m_showGrid;
}

//-------------------------------------------------------------------

void ViewerPreferences::setShowGrid (bool showGrid)
{
	m_showGrid = showGrid;
}

//-------------------------------------------------------------------

ViewerPreferences::CameraMode ViewerPreferences::getCameraMode ()
{
	return m_cameraMode;
}

//-------------------------------------------------------------------

void ViewerPreferences::setCameraMode (ViewerPreferences::CameraMode cameraMode)
{
	m_cameraMode = cameraMode;
}

//-------------------------------------------------------------------

void ViewerPreferences::setLightYaw (float lightYaw)
{
	m_lightYaw = lightYaw;
}

//-------------------------------------------------------------------

float ViewerPreferences::getLightYaw ()
{
	return m_lightYaw;
}

//-------------------------------------------------------------------

void ViewerPreferences::setLightPitch (float lightPitch)
{
	m_lightPitch = lightPitch;
}

//-------------------------------------------------------------------

float ViewerPreferences::getLightPitch ()
{
	return m_lightPitch;
}

//-------------------------------------------------------------------

LightManager& ViewerPreferences::getLightManager ()
{
	return m_lightManager;
}

//-------------------------------------------------------------------

const LightManager& ViewerPreferences::getConstLightManager ()
{
	return m_lightManager;
}

//-------------------------------------------------------------------

bool ViewerPreferences::getEnableWheelZoom ()
{
	return m_enableWheelZoom;
}

//-------------------------------------------------------------------

void ViewerPreferences::setEnableWheelZoom (bool enableWheelZoom)
{
	m_enableWheelZoom = enableWheelZoom;
}

//-------------------------------------------------------------------

float ViewerPreferences::getWheelZoomScale ()
{
	return m_wheelZoomScale;
}

//-------------------------------------------------------------------

void ViewerPreferences::setWheelZoomScale (float wheelZoomScale)
{
	m_wheelZoomScale = wheelZoomScale;
}

//-------------------------------------------------------------------

bool ViewerPreferences::getDisplayStatistics ()
{
	return m_displayStatistics;
}

//-------------------------------------------------------------------

void ViewerPreferences::setDisplayStatistics (bool displayStatistics)
{
	m_displayStatistics = displayStatistics;
}

//-------------------------------------------------------------------

bool ViewerPreferences::getEnableTextures ()
{
	return m_enableTextures;
}

//-------------------------------------------------------------------

void ViewerPreferences::setEnableTextures (bool enableTextures)
{
	m_enableTextures = enableTextures;
}

//-------------------------------------------------------------------

const Shader* ViewerPreferences::getOverrideShader ()
{
	return m_overrideShader;
}

//-------------------------------------------------------------------

bool ViewerPreferences::getUseOverrideShader ()
{
	return m_useOverrideShader;
}

//-------------------------------------------------------------------

void ViewerPreferences::setUseOverrideShader (bool useOverrideShader)
{
	m_useOverrideShader = useOverrideShader;
}

//-------------------------------------------------------------------

void ViewerPreferences::setOverrideShaderTemplateName (const char* overrideShaderTemplateName)
{
	m_overrideShaderTemplateName = overrideShaderTemplateName;

	reload ();
}

//-------------------------------------------------------------------

const char* ViewerPreferences::getOverrideShaderTemplateName ()
{
	return m_overrideShaderTemplateName.c_str ();
}

//-------------------------------------------------------------------

bool ViewerPreferences::getFogEnabled ()
{
	return m_fogEnabled;
}

//-------------------------------------------------------------------

void ViewerPreferences::setFogEnabled (bool fogEnabled)
{
	m_fogEnabled = fogEnabled;
}

//-------------------------------------------------------------------

float ViewerPreferences::getFogDensity ()
{
	return m_fogDensity;
}

//-------------------------------------------------------------------

const VectorArgb& ViewerPreferences::getFogColor ()
{
	return m_fogColor;
}

//-------------------------------------------------------------------

void ViewerPreferences::setFogColor (const VectorArgb& fogColor)
{
	m_fogColor = fogColor;
}

//-------------------------------------------------------------------

void ViewerPreferences::setFogDensity (float fogDensity)
{
	m_fogDensity = fogDensity;
}

//-------------------------------------------------------------------

bool ViewerPreferences::getLowPriority ()
{
	return m_lowPriority;
}

//-------------------------------------------------------------------

void ViewerPreferences::setLowPriority (bool lowPriority)
{
	m_lowPriority = lowPriority;
}

//-------------------------------------------------------------------

bool ViewerPreferences::getShowHardpoints ()
{
	return m_showHardpoints;
}

//-------------------------------------------------------------------

void ViewerPreferences::setShowHardpoints (bool showHardpoints)
{
	m_showHardpoints = showHardpoints;
}

//-------------------------------------------------------------------

bool ViewerPreferences::getShowCMeshes ()
{
	return m_showCMeshes;
}

//-------------------------------------------------------------------

void ViewerPreferences::setShowCMeshes (bool showCMeshes)
{
	m_showCMeshes = showCMeshes;
	ConfigSharedCollision::setDrawCMeshes(m_showCMeshes);
}

//-------------------------------------------------------------------

bool ViewerPreferences::getShowColExtents ()
{
	return m_showColExtents;
}

//-------------------------------------------------------------------

void ViewerPreferences::setShowColExtents (bool showExtents)
{
	m_showColExtents = showExtents;
	ConfigSharedCollision::setDrawExtents(m_showColExtents);
}

//-------------------------------------------------------------------

bool ViewerPreferences::getShowFloors ()
{
	return m_showFloors;
}

//-------------------------------------------------------------------

void ViewerPreferences::setShowFloors (bool showFloors)
{
	m_showFloors = showFloors;
	ConfigSharedCollision::setDrawFloors(m_showFloors);
}

//-------------------------------------------------------------------

bool ViewerPreferences::getShowPathNodes ()
{
	return m_showPathNodes;
}

//-------------------------------------------------------------------

void ViewerPreferences::setShowPathNodes (bool showPathNodes)
{
	m_showPathNodes = showPathNodes;
	ConfigSharedCollision::setDrawPathNodes(m_showPathNodes);
}

//-------------------------------------------------------------------

void ViewerPreferences::unload ()
{
	if (m_overrideShader)
	{
		m_overrideShader->release ();
		m_overrideShader = 0;
	}
}

//-------------------------------------------------------------------

void ViewerPreferences::reload ()
{
	//-- reload the override shader
	if (m_overrideShader)
	{
		m_overrideShader->release ();
		m_overrideShader = 0;
	}

	if (m_overrideShaderTemplateName.length () && TreeFile::exists(m_overrideShaderTemplateName.c_str ()))
		m_overrideShader = ShaderTemplateList::fetchShader (m_overrideShaderTemplateName.c_str ());
}

//-------------------------------------------------------------------

void ViewerPreferences::setEnvironmentMapName (const char* environmentMapName)
{
	m_environmentMapName = environmentMapName;
}

//-------------------------------------------------------------------

const char* ViewerPreferences::getEnvironmentMapName ()
{
	return m_environmentMapName.c_str ();
}

//-------------------------------------------------------------------

bool ViewerPreferences::getLowerReferencePlane ()
{
	return m_lowerReferencePlane;
}

//-------------------------------------------------------------------

void ViewerPreferences::setLowerReferencePlane (const bool lowerReferencePlane)
{
	m_lowerReferencePlane = lowerReferencePlane;
}

//-------------------------------------------------------------------

bool ViewerPreferences::getBloomEnabled ()
{
	return m_bloom;
}

//-------------------------------------------------------------------

void ViewerPreferences::setBloomEnabled (bool bloomEnabled)
{
	m_bloom = bloomEnabled;
}

//===================================================================
// PRIVATE STATIC ViewerPreferences
//===================================================================

GlFillMode       ViewerPreferences::m_fillMode             = GFM_solid;
bool             ViewerPreferences::m_showFaceNormals      = false;
bool             ViewerPreferences::m_showVertexNormals    = false;
bool             ViewerPreferences::m_showExtents          = false;
bool             ViewerPreferences::m_showObject           = true;
bool             ViewerPreferences::m_showSkeleton         = false;

bool             ViewerPreferences::m_alternateRgb         = false;
VectorArgb       ViewerPreferences::m_clearColor (0.f, 161.f * RECIP (255), 161.f * RECIP (255), 161.f * RECIP (255));
ViewerPreferences::CameraMode ViewerPreferences::m_cameraMode = ViewerPreferences::CM_maya;
bool             ViewerPreferences::m_fogEnabled           = false;
VectorArgb       ViewerPreferences::m_fogColor (0.f, 161.f * RECIP (255), 161.f * RECIP (255), 161.f * RECIP (255));
float            ViewerPreferences::m_fogDensity           = 0.001f;
bool             ViewerPreferences::m_enableWheelZoom      = true;
float            ViewerPreferences::m_wheelZoomScale       = 1.f;

ViewerPreferences::LightMode ViewerPreferences::m_lightMode = ViewerPreferences::LM_full;
Light*           ViewerPreferences::m_fullAmbientLight;
Light*           ViewerPreferences::m_quarterAmbientLight;
Light*           ViewerPreferences::m_parallelLight;
bool             ViewerPreferences::m_animateLight;
float            ViewerPreferences::m_lightYaw             = PI_OVER_4;
float            ViewerPreferences::m_lightPitch           = PI_OVER_4;
bool             ViewerPreferences::m_lightShowHemisphericColors = false;
ObjectList*      ViewerPreferences::m_userLightList;

LightManager ViewerPreferences::m_lightManager;

Object*          ViewerPreferences::m_grid;
Object*          ViewerPreferences::m_referencePlane;
bool             ViewerPreferences::m_lowerReferencePlane  = true;
bool             ViewerPreferences::m_showAxis             = true;
bool             ViewerPreferences::m_showGrid             = true;
bool             ViewerPreferences::m_displayStatistics    = true;
bool             ViewerPreferences::m_enableTextures       = true;

const Shader*    ViewerPreferences::m_overrideShader;
bool             ViewerPreferences::m_useOverrideShader    = false;
std::string      ViewerPreferences::m_overrideShaderTemplateName;

bool             ViewerPreferences::m_lowPriority          = false;
bool             ViewerPreferences::m_showHardpoints       = false;
bool             ViewerPreferences::m_showCMeshes       = false;
bool             ViewerPreferences::m_showColExtents       = false;
bool             ViewerPreferences::m_showFloors       = false;
bool             ViewerPreferences::m_showPathNodes       = false;

std::string      ViewerPreferences::m_environmentMapName;

bool             ViewerPreferences::m_bloom                = false;

//-------------------------------------------------------------------

void ViewerPreferences::load (Iff& iff)
{
	if (iff.enterForm (TAG_VPRF, true))
	{
		switch (iff.getCurrentName ())
		{
		case TAG_0000:
			load_0000 (iff);
			break;

		case TAG_0001:
			load_0001 (iff);
			break;

		case TAG_0002:
			load_0002 (iff);
			break;

		case TAG_0003:
			load_0003 (iff);
			break;

		case TAG_0004:
			load_0004 (iff);
			break;

		default:
			WARNING (true, ("unknown preferences format\n"));
			break;
		}

		iff.exitForm (TAG_VPRF);
	}
}

//-------------------------------------------------------------------

void ViewerPreferences::load_0000 (Iff& iff)
{
	iff.enterChunk (TAG_0000);

		m_fillMode          = static_cast<GlFillMode> (iff.read_int8 ());
		m_showFaceNormals   = iff.read_bool8 ();
		m_showVertexNormals = iff.read_bool8 ();
		m_showExtents       = iff.read_bool8 ();
		m_showObject        = iff.read_bool8 ();
		m_showSkeleton      = iff.read_bool8 ();
		m_alternateRgb      = iff.read_bool8 ();
		m_clearColor        = iff.read_floatVectorArgb ();
		m_cameraMode        = static_cast<CameraMode> (iff.read_int8 ());
		m_fogEnabled        = iff.read_bool8 ();
		m_fogDensity        = iff.read_float ();
		m_enableWheelZoom   = iff.read_bool8 ();
		m_wheelZoomScale    = iff.read_float ();
		m_lightMode         = static_cast<LightMode> (iff.read_int8 ());
		m_animateLight      = iff.read_bool8 ();
		m_lightYaw          = iff.read_float ();
		m_lightPitch        = iff.read_float ();
		m_lowerReferencePlane = iff.read_bool8 ();
		m_showAxis          = iff.read_bool8 ();
		m_showGrid          = iff.read_bool8 ();
		m_displayStatistics = iff.read_bool8 ();
		m_enableTextures    = iff.read_bool8 ();
		m_useOverrideShader = iff.read_bool8 ();
		m_lowPriority       = iff.read_bool8 ();
		
	iff.exitChunk (TAG_0000);
}

//-------------------------------------------------------------------

void ViewerPreferences::load_0001 (Iff& iff)
{
	iff.enterChunk (TAG_0001);

		m_fillMode          = static_cast<GlFillMode> (iff.read_int8 ());
		m_showFaceNormals   = iff.read_bool8 ();
		m_showVertexNormals = iff.read_bool8 ();
		m_showExtents       = iff.read_bool8 ();
		m_showObject        = iff.read_bool8 ();
		m_showSkeleton      = iff.read_bool8 ();
		m_alternateRgb      = iff.read_bool8 ();
		m_clearColor        = iff.read_floatVectorArgb ();
		m_cameraMode        = static_cast<CameraMode> (iff.read_int8 ());
		m_fogEnabled        = iff.read_bool8 ();
		m_fogColor          = iff.read_floatVectorArgb ();
		m_fogDensity        = iff.read_float ();
		m_enableWheelZoom   = iff.read_bool8 ();
		m_wheelZoomScale    = iff.read_float ();
		m_lightMode         = static_cast<LightMode> (iff.read_int8 ());
		m_animateLight      = iff.read_bool8 ();
		m_lightYaw          = iff.read_float ();
		m_lightPitch        = iff.read_float ();
		m_lowerReferencePlane = iff.read_bool8 ();
		m_showAxis          = iff.read_bool8 ();
		m_showGrid          = iff.read_bool8 ();
		m_displayStatistics = iff.read_bool8 ();
		m_enableTextures    = iff.read_bool8 ();
		m_useOverrideShader = iff.read_bool8 ();
		m_lowPriority       = iff.read_bool8 ();
		m_showHardpoints    = iff.read_bool8 ();

	iff.exitChunk (TAG_0001);
}

//-------------------------------------------------------------------

void ViewerPreferences::load_0002 (Iff& iff)
{
	iff.enterChunk (TAG_0002);

		m_fillMode          = static_cast<GlFillMode> (iff.read_int8 ());
		m_showFaceNormals   = iff.read_bool8 ();
		m_showVertexNormals = iff.read_bool8 ();
		m_showExtents       = iff.read_bool8 ();
		m_showObject        = iff.read_bool8 ();
		m_showSkeleton      = iff.read_bool8 ();
		m_alternateRgb      = iff.read_bool8 ();
		m_clearColor        = iff.read_floatVectorArgb ();
		m_cameraMode        = static_cast<CameraMode> (iff.read_int8 ());
		m_fogEnabled        = iff.read_bool8 ();
		m_fogColor          = iff.read_floatVectorArgb ();
		m_fogDensity        = iff.read_float ();
		m_enableWheelZoom   = iff.read_bool8 ();
		m_wheelZoomScale    = iff.read_float ();
		m_lightMode         = static_cast<LightMode> (iff.read_int8 ());
		m_animateLight      = iff.read_bool8 ();
		m_lightYaw          = iff.read_float ();
		m_lightPitch        = iff.read_float ();
		m_lowerReferencePlane = iff.read_bool8 ();
		m_showAxis          = iff.read_bool8 ();
		m_showGrid          = iff.read_bool8 ();
		m_displayStatistics = iff.read_bool8 ();
		m_enableTextures    = iff.read_bool8 ();
		m_useOverrideShader = iff.read_bool8 ();
		m_lowPriority       = iff.read_bool8 ();
		m_showHardpoints    = iff.read_bool8 ();
		iff.read_string (m_environmentMapName);

	iff.exitChunk (TAG_0002);
}

//-------------------------------------------------------------------

void ViewerPreferences::load_0003 (Iff& iff)
{
	iff.enterChunk (TAG_0003);

		m_fillMode          = static_cast<GlFillMode> (iff.read_int8 ());
		m_showFaceNormals   = iff.read_bool8 ();
		m_showVertexNormals = iff.read_bool8 ();
		m_showExtents       = iff.read_bool8 ();
		m_showObject        = iff.read_bool8 ();
		m_showSkeleton      = iff.read_bool8 ();
		m_alternateRgb      = iff.read_bool8 ();
		m_clearColor        = iff.read_floatVectorArgb ();
		m_cameraMode        = static_cast<CameraMode> (iff.read_int8 ());
		m_fogEnabled        = iff.read_bool8 ();
		m_fogColor          = iff.read_floatVectorArgb ();
		m_fogDensity        = iff.read_float ();
		m_enableWheelZoom   = iff.read_bool8 ();
		m_wheelZoomScale    = iff.read_float ();
		m_lightMode         = static_cast<LightMode> (iff.read_int8 ());
		m_animateLight      = iff.read_bool8 ();
		m_lightYaw          = iff.read_float ();
		m_lightPitch        = iff.read_float ();
		m_lowerReferencePlane = iff.read_bool8 ();
		m_showAxis          = iff.read_bool8 ();
		m_showGrid          = iff.read_bool8 ();
		m_displayStatistics = iff.read_bool8 ();
		m_enableTextures    = iff.read_bool8 ();
		m_useOverrideShader = iff.read_bool8 ();
		m_lowPriority       = iff.read_bool8 ();
		m_showHardpoints    = iff.read_bool8 ();
		iff.read_string (m_environmentMapName);
		m_bloom             = iff.read_bool8 ();

	iff.exitChunk (TAG_0003);
}

//-------------------------------------------------------------------

void ViewerPreferences::load_0004 (Iff& iff)
{
	iff.enterChunk (TAG_0004);

		m_fillMode          = static_cast<GlFillMode> (iff.read_int8 ());
		m_showFaceNormals   = iff.read_bool8 ();
		m_showVertexNormals = iff.read_bool8 ();
		m_showExtents       = iff.read_bool8 ();
		m_showObject        = iff.read_bool8 ();
		m_showSkeleton      = iff.read_bool8 ();
		m_alternateRgb      = iff.read_bool8 ();
		m_clearColor        = iff.read_floatVectorArgb ();
		m_cameraMode        = static_cast<CameraMode> (iff.read_int8 ());
		m_fogEnabled        = iff.read_bool8 ();
		m_fogColor          = iff.read_floatVectorArgb ();
		m_fogDensity        = iff.read_float ();
		m_enableWheelZoom   = iff.read_bool8 ();
		m_wheelZoomScale    = iff.read_float ();
		m_lightMode         = static_cast<LightMode> (iff.read_int8 ());
		m_animateLight      = iff.read_bool8 ();
		m_lightYaw          = iff.read_float ();
		m_lightPitch        = iff.read_float ();
		m_lowerReferencePlane = iff.read_bool8 ();
		m_showAxis          = iff.read_bool8 ();
		m_showGrid          = iff.read_bool8 ();
		m_displayStatistics = iff.read_bool8 ();
		m_enableTextures    = iff.read_bool8 ();
		m_useOverrideShader = iff.read_bool8 ();
		m_lowPriority       = iff.read_bool8 ();
		m_showHardpoints    = iff.read_bool8 ();
		setShowCMeshes(iff.read_bool8());
		setShowColExtents(iff.read_bool8());
		setShowFloors(iff.read_bool8());
		setShowPathNodes(iff.read_bool8());
		iff.read_string (m_environmentMapName);
		m_bloom             = iff.read_bool8 ();

	iff.exitChunk (TAG_0004);
}

//-------------------------------------------------------------------

void ViewerPreferences::save (Iff& iff)
{
	iff.insertChunk (TAG_0004);

		iff.insertChunkData (static_cast<int8> (m_fillMode));
		iff.insertChunkData (m_showFaceNormals   ? static_cast<int8> (1) : static_cast<int8> (0));
		iff.insertChunkData (m_showVertexNormals ? static_cast<int8> (1) : static_cast<int8> (0));
		iff.insertChunkData (m_showExtents       ? static_cast<int8> (1) : static_cast<int8> (0));
		iff.insertChunkData (m_showObject        ? static_cast<int8> (1) : static_cast<int8> (0));
		iff.insertChunkData (m_showSkeleton      ? static_cast<int8> (1) : static_cast<int8> (0));
		iff.insertChunkData (m_alternateRgb      ? static_cast<int8> (1) : static_cast<int8> (0));
		iff.insertChunkFloatVectorArgb (m_clearColor);
		iff.insertChunkData (static_cast<int8> (m_cameraMode));
		iff.insertChunkData (m_fogEnabled        ? static_cast<int8> (1) : static_cast<int8> (0));
		iff.insertChunkFloatVectorArgb (m_fogColor);
		iff.insertChunkData (m_fogDensity);
		iff.insertChunkData (m_enableWheelZoom   ? static_cast<int8> (1) : static_cast<int8> (0));
		iff.insertChunkData (m_wheelZoomScale);
		iff.insertChunkData (static_cast<int8> (m_lightMode));
		iff.insertChunkData (m_animateLight      ? static_cast<int8> (1) : static_cast<int8> (0));
		iff.insertChunkData (m_lightYaw);
		iff.insertChunkData (m_lightPitch);
		iff.insertChunkData (m_lowerReferencePlane ? static_cast<int8> (1) : static_cast<int8> (0));
		iff.insertChunkData (m_showAxis          ? static_cast<int8> (1) : static_cast<int8> (0));
		iff.insertChunkData (m_showGrid          ? static_cast<int8> (1) : static_cast<int8> (0));
		iff.insertChunkData (m_displayStatistics ? static_cast<int8> (1) : static_cast<int8> (0));
		iff.insertChunkData (m_enableTextures    ? static_cast<int8> (1) : static_cast<int8> (0));
		iff.insertChunkData (m_useOverrideShader ? static_cast<int8> (1) : static_cast<int8> (0));
		iff.insertChunkData (m_lowPriority       ? static_cast<int8> (1) : static_cast<int8> (0));
		iff.insertChunkData (m_showHardpoints    ? static_cast<int8> (1) : static_cast<int8> (0));
		iff.insertChunkData (m_showCMeshes    ? static_cast<int8> (1) : static_cast<int8> (0));
		iff.insertChunkData (m_showColExtents    ? static_cast<int8> (1) : static_cast<int8> (0));
		iff.insertChunkData (m_showFloors    ? static_cast<int8> (1) : static_cast<int8> (0));
		iff.insertChunkData (m_showPathNodes    ? static_cast<int8> (1) : static_cast<int8> (0));
		iff.insertChunkString (m_environmentMapName.c_str ());
		iff.insertChunkData (m_bloom             ? static_cast<int8> (1) : static_cast<int8> (0));

	iff.exitChunk (TAG_0004);
}

//===================================================================

StaticShader const & ViewerPreferences::prepareToViewOverrideFunction(ShaderPrimitive const & shaderPrimitive)
{
	if (m_overrideShader)
		return m_overrideShader->prepareToView();

	return shaderPrimitive.prepareToView();
}

//===================================================================

