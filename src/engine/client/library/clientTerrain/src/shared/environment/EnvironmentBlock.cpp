//===================================================================
//
// EnvironmentBlock.cpp
// asommers
//
// copyright 2002, sony online entertainment
// 
//===================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/EnvironmentBlock.h"

#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedImage/Image.h"
#include "sharedImage/ImageFormatList.h"
#include "sharedMath/PackedArgb.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Controller.h"
#include "sharedObject/Object.h"

//===================================================================

namespace EnvironmentBlockNamespace
{
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class CameraAttachedController : public Controller
	{
	public:

		explicit CameraAttachedController (Object * owner);
		virtual ~CameraAttachedController ();

	protected:

		virtual float realAlter (float time);

	private:

		CameraAttachedController ();
		CameraAttachedController (CameraAttachedController const &);
		CameraAttachedController & operator = (CameraAttachedController const &);
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const float ms_oo128        = 1.0f / 128.0f;
	const float ms_oo255        = 1.0f / 255.0f;
	const float ms_lightScale   = 4.0f;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const int ms_numOldColorChannels = 8;
	const int ms_numNewColorChannels = ms_numOldColorChannels + 2; // 2 new colors for each for main light (back and tangent colors)

	bool isValid (const Image* const image)
	{
		if (image->getWidth () != 256)
			return false;
		
		if ((image->getHeight () != ms_numOldColorChannels) && (image->getHeight () != ms_numNewColorChannels))
			return false;

		if (image->getPixelFormat () == Image::PF_bgra_8888)
			return true;

		if (image->getPixelFormat () == Image::PF_abgr_8888)
			return true;

		if (image->getPixelFormat () == Image::PF_rgba_8888)
			return true;

		if (image->getPixelFormat () == Image::PF_argb_8888)
			return true;

		return false;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const PackedArgb getPixel (const Image* const image, int x, int y)
	{
		PackedArgb result = PackedArgb::solidBlack;

		const uint8* data = image->lockReadOnly ();

			data += y * image->getStride () + x * image->getBytesPerPixel ();

			switch (image->getPixelFormat ())
			{
			case Image::PF_bgra_8888:
				result.setB (*data++);
				result.setG (*data++);
				result.setR (*data++);
				result.setA (*data++);
				break;

			case Image::PF_abgr_8888:
				result.setA (*data++);
				result.setR (*data++);
				result.setG (*data++);
				result.setB (*data++);
				break;

			case Image::PF_rgba_8888:
				result.setR (*data++);
				result.setG (*data++);
				result.setB (*data++);
				result.setA (*data++);
				break;

			case Image::PF_argb_8888:
				result.setA (*data++);
				result.setR (*data++);
				result.setG (*data++);
				result.setB (*data++);
				break;
			}

		image->unlock ();

		return result;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace EnvironmentBlockNamespace;

//===================================================================
// CameraAttachedController
//===================================================================

CameraAttachedController::CameraAttachedController (Object * const owner) :
	Controller (owner)
{
}

//-------------------------------------------------------------------

CameraAttachedController::~CameraAttachedController ()
{
}

//-------------------------------------------------------------------

float CameraAttachedController::realAlter (float const /*time*/)
{
	Object * const owner = getOwner ();
	if (owner)
	{
		Object const * const parent = owner->getParent ();
		if (parent)
		{
			Vector const k = parent->rotate_w2o (Vector::unitZ);
			Vector const j = parent->rotate_w2o (Vector::unitY);
			owner->setTransformKJ_o2p (k, j);
		}
	}

	return AlterResult::cms_alterNextFrame;
}

//===================================================================
// PUBLIC EnvironmentBlockData
//===================================================================

EnvironmentBlockData::EnvironmentBlockData () :
	m_name (0),
	m_familyId (0),
	m_weatherIndex (0),
	m_gradientSkyTextureName (0),
	m_cloudLayerBottomShaderTemplateName (0),
	m_cloudLayerBottomShaderSize (0.f),
	m_cloudLayerBottomSpeed (0.f),
	m_cloudLayerTopShaderTemplateName (0),
	m_cloudLayerTopShaderSize (0.f),
	m_cloudLayerTopSpeed (0.f),
	m_colorRampFileName (0),
	m_shadowsEnabled (false),
	m_fogEnabled (false),
	m_minimumFogDensity (0.f),
	m_maximumFogDensity (0.f),
	m_cameraAppearanceTemplateName (0),
	m_dayEnvironmentTextureName (0),
	m_nightEnvironmentTextureName (0),
	m_day1AmbientSoundTemplateName (0),
	m_day2AmbientSoundTemplateName (0),
	m_night1AmbientSoundTemplateName (0),
	m_night2AmbientSoundTemplateName (0),
	m_firstMusicSoundTemplateName (0),
	m_sunriseMusicSoundTemplateName (0),
	m_sunsetMusicSoundTemplateName (0),
	m_windSpeedScale (0.f)
{
}

//===================================================================
// PUBLIC EnvironmentBlock
//===================================================================

EnvironmentBlock::EnvironmentBlock () :
	m_name (0),
	m_familyId (0),
	m_weatherIndex (0),
	m_gradientSkyTextureName (0),
	m_gradientSkyTexture (0),
	m_cloudLayerBottomShaderTemplateName (0),
	m_cloudLayerBottomShaderSize (0.f),
	m_cloudLayerBottomSpeed (0.f),
	m_cloudLayerTopShaderTemplateName (0),
	m_cloudLayerTopShaderSize (0.f),
	m_cloudLayerTopSpeed (0.f),
	m_shadowsEnabled (false),
	m_fogEnabled (false),
	m_minimumFogDensity (0),
	m_maximumFogDensity (0),
	m_cameraAttachedObject (0),
	m_dayEnvironmentTexture (0),
	m_nightEnvironmentTexture (0),
	m_day1AmbientSoundTemplateName (new PersistentCrcString),
	m_day2AmbientSoundTemplateName (new PersistentCrcString),
	m_night1AmbientSoundTemplateName (new PersistentCrcString),
	m_night2AmbientSoundTemplateName (new PersistentCrcString),
	m_firstMusicSoundTemplateName (new PersistentCrcString),
	m_sunriseMusicSoundTemplateName (new PersistentCrcString),
	m_sunsetMusicSoundTemplateName (new PersistentCrcString),
	m_windSpeedScale (0.f)
{
}

//-------------------------------------------------------------------

EnvironmentBlock::~EnvironmentBlock ()
{
	clear ();

	delete m_day1AmbientSoundTemplateName;
	delete m_day2AmbientSoundTemplateName;
	delete m_night1AmbientSoundTemplateName;
	delete m_night2AmbientSoundTemplateName;
	delete m_firstMusicSoundTemplateName;
	delete m_sunriseMusicSoundTemplateName;
	delete m_sunsetMusicSoundTemplateName;
}

//-------------------------------------------------------------------

void EnvironmentBlock::setData (const EnvironmentBlockData& data)
{
	clear ();

	NOT_NULL (data.m_name);
	m_name = DuplicateString (data.m_name);
	m_familyId = data.m_familyId;
	m_weatherIndex = data.m_weatherIndex;

	if (data.m_gradientSkyTextureName && *data.m_gradientSkyTextureName)
	{
		m_gradientSkyTextureName = DuplicateString (data.m_gradientSkyTextureName);
		m_gradientSkyTexture = TextureList::fetch (data.m_gradientSkyTextureName);
	}

	if (data.m_cloudLayerBottomShaderTemplateName && *data.m_cloudLayerBottomShaderTemplateName)
		m_cloudLayerBottomShaderTemplateName = DuplicateString (data.m_cloudLayerBottomShaderTemplateName);

	m_cloudLayerBottomShaderSize = data.m_cloudLayerBottomShaderSize;
	m_cloudLayerBottomSpeed = data.m_cloudLayerBottomSpeed;

	if (data.m_cloudLayerTopShaderTemplateName && *data.m_cloudLayerTopShaderTemplateName)
		m_cloudLayerTopShaderTemplateName = DuplicateString (data.m_cloudLayerTopShaderTemplateName);

	m_cloudLayerTopShaderSize = data.m_cloudLayerTopShaderSize;
	m_cloudLayerTopSpeed = data.m_cloudLayerTopSpeed;

	loadColorRamps (data.m_colorRampFileName);

	m_shadowsEnabled = data.m_shadowsEnabled;
	m_fogEnabled = data.m_fogEnabled;
	m_minimumFogDensity = data.m_minimumFogDensity;
	m_maximumFogDensity = data.m_maximumFogDensity;

	if (data.m_cameraAppearanceTemplateName && *data.m_cameraAppearanceTemplateName)
	{
		m_cameraAttachedObject = new Object ();
		m_cameraAttachedObject->setController (new CameraAttachedController (m_cameraAttachedObject));
		m_cameraAttachedObject->setAppearance (AppearanceTemplateList::createAppearance (data.m_cameraAppearanceTemplateName));

		RenderWorld::addObjectNotifications (*m_cameraAttachedObject);
	}

	if (data.m_dayEnvironmentTextureName && *data.m_dayEnvironmentTextureName)
		m_dayEnvironmentTexture = TextureList::fetch (data.m_dayEnvironmentTextureName);
	else
		m_dayEnvironmentTexture = TextureList::fetchDefaultEnvironmentTexture ();

	if (data.m_nightEnvironmentTextureName && *data.m_nightEnvironmentTextureName)
		m_nightEnvironmentTexture = TextureList::fetch (data.m_nightEnvironmentTextureName);
	else
		m_nightEnvironmentTexture = TextureList::fetchDefaultEnvironmentTexture ();

	if (data.m_day1AmbientSoundTemplateName && *data.m_day1AmbientSoundTemplateName)
		m_day1AmbientSoundTemplateName->set (data.m_day1AmbientSoundTemplateName, true);

	if (data.m_day2AmbientSoundTemplateName && *data.m_day2AmbientSoundTemplateName)
		m_day2AmbientSoundTemplateName->set (data.m_day2AmbientSoundTemplateName, true);

	if (data.m_night1AmbientSoundTemplateName && *data.m_night1AmbientSoundTemplateName)
		m_night1AmbientSoundTemplateName->set (data.m_night1AmbientSoundTemplateName, true);

	if (data.m_night2AmbientSoundTemplateName && *data.m_night2AmbientSoundTemplateName)
		m_night2AmbientSoundTemplateName->set (data.m_night2AmbientSoundTemplateName, true);

	if (data.m_firstMusicSoundTemplateName && *data.m_firstMusicSoundTemplateName)
		m_firstMusicSoundTemplateName->set (data.m_firstMusicSoundTemplateName, true);

	if (data.m_sunriseMusicSoundTemplateName && *data.m_sunriseMusicSoundTemplateName)
		m_sunriseMusicSoundTemplateName->set (data.m_sunriseMusicSoundTemplateName, true);

	if (data.m_sunsetMusicSoundTemplateName && *data.m_sunsetMusicSoundTemplateName)
		m_sunsetMusicSoundTemplateName->set (data.m_sunsetMusicSoundTemplateName, true);

	m_windSpeedScale = data.m_windSpeedScale;
}

//-------------------------------------------------------------------

const char* EnvironmentBlock::getName () const
{
	return m_name;
}

//-------------------------------------------------------------------

int EnvironmentBlock::getFamilyId () const
{
	return m_familyId;
}

//-------------------------------------------------------------------

int EnvironmentBlock::getWeatherIndex () const
{
	return m_weatherIndex;
}

//-------------------------------------------------------------------

const char* EnvironmentBlock::getGradientSkyTextureName () const
{
	return m_gradientSkyTextureName;
}

//-------------------------------------------------------------------

const Texture* EnvironmentBlock::getGradientSkyTexture () const
{
	return m_gradientSkyTexture;
}

//-------------------------------------------------------------------

const char* EnvironmentBlock::getCloudLayerBottomShaderTemplateName () const
{
	return m_cloudLayerBottomShaderTemplateName;
}

//-------------------------------------------------------------------

float EnvironmentBlock::getCloudLayerBottomShaderSize () const
{
	return m_cloudLayerBottomShaderSize;
}

//-------------------------------------------------------------------

float EnvironmentBlock::getCloudLayerBottomSpeed () const
{
	return m_cloudLayerBottomSpeed;
}

//-------------------------------------------------------------------

const char* EnvironmentBlock::getCloudLayerTopShaderTemplateName () const
{
	return m_cloudLayerTopShaderTemplateName;
}

//-------------------------------------------------------------------

float EnvironmentBlock::getCloudLayerTopShaderSize () const
{
	return m_cloudLayerTopShaderSize;
}

//-------------------------------------------------------------------

float EnvironmentBlock::getCloudLayerTopSpeed () const
{
	return m_cloudLayerTopSpeed;
}

//-------------------------------------------------------------------

int EnvironmentBlock::getNumberOfRampColors () const
{
	return 256;
}

//-------------------------------------------------------------------

const PackedRgb* EnvironmentBlock::getAmbientColorRamp () const
{
	return m_ambientColorRamp;
}

//-------------------------------------------------------------------

const float* EnvironmentBlock::getCelestialAlphaRamp () const
{
	return m_celestialAlphaRamp;
}

//-------------------------------------------------------------------

const PackedRgb* EnvironmentBlock::getMainDiffuseColorRamp () const
{
	return m_mainDiffuseColorRamp;
}

//-------------------------------------------------------------------

const float* EnvironmentBlock::getMainDiffuseColorScale () const
{
	return m_mainDiffuseColorScale;
}

//-------------------------------------------------------------------

const PackedRgb* EnvironmentBlock::getMainSpecularColorRamp () const
{
	return m_mainSpecularColorRamp;
}

//-------------------------------------------------------------------

const float* EnvironmentBlock::getMainSpecularColorScale () const
{
	return m_mainSpecularColorScale;
}

//-------------------------------------------------------------------

const PackedRgb* EnvironmentBlock::getFillColorRamp () const
{
	return m_fillColorRamp;
}

//-------------------------------------------------------------------

const float* EnvironmentBlock::getFillColorScale () const
{
	return m_fillColorScale;
}

//-------------------------------------------------------------------

const PackedRgb* EnvironmentBlock::getBounceColorRamp () const
{
	return m_bounceColorRamp;
}

//-------------------------------------------------------------------

const float* EnvironmentBlock::getBounceColorScale () const
{
	return m_bounceColorScale;
}

//-------------------------------------------------------------------

const PackedRgb* EnvironmentBlock::getClearColorRamp () const
{
	return m_clearColorRamp;
}

//-------------------------------------------------------------------

const float* EnvironmentBlock::getSunMoonAlphaRamp () const
{
	return m_sunMoonAlphaRamp;
}

//-------------------------------------------------------------------

bool EnvironmentBlock::getShadowsEnabled () const
{
	return m_shadowsEnabled;
}

//-------------------------------------------------------------------

bool EnvironmentBlock::getFogEnabled () const
{
	return m_fogEnabled;
}

//-------------------------------------------------------------------

const PackedRgb* EnvironmentBlock::getFogColorRamp () const
{
	return m_fogColorRamp;
}

//-------------------------------------------------------------------

const float* EnvironmentBlock::getStarAlphaRamp () const
{
	return m_starAlphaRamp;
}

//-------------------------------------------------------------------

float EnvironmentBlock::getMinimumFogDensity () const
{
	return m_minimumFogDensity;
}

//-------------------------------------------------------------------

float EnvironmentBlock::getMaximumFogDensity () const
{
	return m_maximumFogDensity;
}

//-------------------------------------------------------------------

const VectorArgb* EnvironmentBlock::getShadowRamp () const
{
	return m_shadowRamp;
}

//-------------------------------------------------------------------

const PackedRgb* EnvironmentBlock::getMainBackColorRamp () const
{
	return m_mainBackColorRamp;
}

//-------------------------------------------------------------------

const float* EnvironmentBlock::getMainBackColorScale () const
{
	return m_mainBackColorScale;
}

//-------------------------------------------------------------------

const PackedRgb* EnvironmentBlock::getMainTangentColorRamp () const
{
	return m_mainTangentColorRamp;
}

//-------------------------------------------------------------------

const float* EnvironmentBlock::getMainTangentColorScale () const
{
	return m_mainTangentColorScale;
}

//-------------------------------------------------------------------

Object* EnvironmentBlock::getCameraAttachedObject () const
{
	return m_cameraAttachedObject;
}

//-------------------------------------------------------------------

const Texture* EnvironmentBlock::getDayEnvironmentTexture () const
{
	return m_dayEnvironmentTexture;
}

//-------------------------------------------------------------------

const Texture* EnvironmentBlock::getNightEnvironmentTexture () const
{
	return m_nightEnvironmentTexture;
}

//-------------------------------------------------------------------

const CrcString* EnvironmentBlock::getDay1AmbientSoundTemplateName () const
{
	return m_day1AmbientSoundTemplateName;
}

//-------------------------------------------------------------------

const CrcString* EnvironmentBlock::getDay2AmbientSoundTemplateName () const
{
	return m_day2AmbientSoundTemplateName;
}

//-------------------------------------------------------------------

const CrcString* EnvironmentBlock::getNight1AmbientSoundTemplateName () const
{
	return m_night1AmbientSoundTemplateName;
}

//-------------------------------------------------------------------

const CrcString* EnvironmentBlock::getNight2AmbientSoundTemplateName () const
{
	return m_night2AmbientSoundTemplateName;
}

//-------------------------------------------------------------------

const CrcString* EnvironmentBlock::getFirstMusicSoundTemplateName () const
{
	return m_firstMusicSoundTemplateName;
}

//-------------------------------------------------------------------

const CrcString* EnvironmentBlock::getSunriseMusicSoundTemplateName () const
{
	return m_sunriseMusicSoundTemplateName;
}

//-------------------------------------------------------------------

const CrcString* EnvironmentBlock::getSunsetMusicSoundTemplateName () const
{
	return m_sunsetMusicSoundTemplateName;
}

//-------------------------------------------------------------------

float EnvironmentBlock::getWindSpeedScale () const
{
	return m_windSpeedScale;
}

//===================================================================
// PRIVATE EnvironmentBlock
//===================================================================

void EnvironmentBlock::clear ()
{
	delete [] m_name;
	m_name = 0;

	if (m_gradientSkyTextureName)
	{
		delete [] m_gradientSkyTextureName;
		m_gradientSkyTextureName = 0;
	}

	if (m_gradientSkyTexture)
	{
		m_gradientSkyTexture->release ();
		m_gradientSkyTexture = 0;
	}

	if (m_cloudLayerBottomShaderTemplateName)
	{
		delete [] m_cloudLayerBottomShaderTemplateName;
		m_cloudLayerBottomShaderTemplateName = 0;
	}

	if (m_cloudLayerTopShaderTemplateName)
	{
		delete [] m_cloudLayerTopShaderTemplateName;
		m_cloudLayerTopShaderTemplateName = 0;
	}

	if (m_cameraAttachedObject)
	{
		delete m_cameraAttachedObject;
		m_cameraAttachedObject = 0;
	}

	if (m_dayEnvironmentTexture)
	{
		m_dayEnvironmentTexture->release ();
		m_dayEnvironmentTexture = 0;
	}

	if (m_nightEnvironmentTexture)
	{
		m_nightEnvironmentTexture->release ();
		m_nightEnvironmentTexture = 0;
	}

	m_day1AmbientSoundTemplateName->set ("", false);
	m_day2AmbientSoundTemplateName->set ("", false);
	m_night1AmbientSoundTemplateName->set ("", false);
	m_night2AmbientSoundTemplateName->set ("", false);
	m_firstMusicSoundTemplateName->set ("", false);
	m_sunriseMusicSoundTemplateName->set ("", false);
	m_sunsetMusicSoundTemplateName->set ("", false);
}

//-------------------------------------------------------------------

void EnvironmentBlock::loadColorRamps (const char* fileName)
{
	//-- load the image
	Image* const image = fileName ? ImageFormatList::loadImage (fileName) : 0;
	if (image && isValid (image))
	{
		const bool hasHemisphericLightData = (image->getHeight() == ms_numNewColorChannels);
		PackedArgb color;

		//-- extract the color ramps
		int i;
		for (i = 0; i < 256; ++i)
		{
			color = getPixel (image, i, 0);
			m_ambientColorRamp [i].r = color.getR ();
			m_ambientColorRamp [i].g = color.getG ();
			m_ambientColorRamp [i].b = color.getB ();
			m_celestialAlphaRamp [i] = static_cast<float> (color.getA ()) * ms_oo255;
		
			color = getPixel (image, i, 1);
			m_mainDiffuseColorRamp [i].r = color.getR ();
			m_mainDiffuseColorRamp [i].g = color.getG ();
			m_mainDiffuseColorRamp [i].b = color.getB ();
			m_mainDiffuseColorScale [i] = ms_lightScale * ((static_cast<float> (color.getA ()) - 128.f) * ms_oo128);

			color = getPixel (image, i, 2);
			m_mainSpecularColorRamp [i].r = color.getR ();
			m_mainSpecularColorRamp [i].g = color.getG ();
			m_mainSpecularColorRamp [i].b = color.getB ();
			m_mainSpecularColorScale [i] = ms_lightScale * ((static_cast<float> (color.getA ()) - 128.f) * ms_oo128);

			color = getPixel (image, i, 3);
			m_fillColorRamp [i].r = color.getR ();
			m_fillColorRamp [i].g = color.getG ();
			m_fillColorRamp [i].b = color.getB ();
			m_fillColorScale [i] = ms_lightScale * ((static_cast<float> (color.getA ()) - 128.f) * ms_oo128);

			color = getPixel (image, i, 4);
			m_bounceColorRamp [i].r = color.getR ();
			m_bounceColorRamp [i].g = color.getG ();
			m_bounceColorRamp [i].b = color.getB ();
			m_bounceColorScale [i] = ms_lightScale * ((static_cast<float> (color.getA ()) - 128.f) * ms_oo128);

			color = getPixel (image, i, 5);
			m_clearColorRamp [i].r = color.getR ();
			m_clearColorRamp [i].g = color.getG ();
			m_clearColorRamp [i].b = color.getB ();
			m_sunMoonAlphaRamp [i] = static_cast<float> (color.getA ()) * ms_oo255;

			color = getPixel (image, i, 6);
			m_fogColorRamp [i].r = color.getR ();
			m_fogColorRamp [i].g = color.getG ();
			m_fogColorRamp [i].b = color.getB ();
			m_starAlphaRamp [i] = static_cast<float> (color.getA ()) * ms_oo255;

			m_shadowRamp [i] = getPixel (image, i, 7);

			if (hasHemisphericLightData)
			{
				color = getPixel (image, i, 8);
				m_mainBackColorRamp [i].r = color.getR ();
				m_mainBackColorRamp [i].g = color.getG ();
				m_mainBackColorRamp [i].b = color.getB ();
				m_mainBackColorScale [i] = ms_lightScale * ((static_cast<float> (color.getA ()) - 128.f) * ms_oo128);

				color = getPixel (image, i, 9);
				m_mainTangentColorRamp [i].r = color.getR ();
				m_mainTangentColorRamp [i].g = color.getG ();
				m_mainTangentColorRamp [i].b = color.getB ();
				m_mainTangentColorScale [i] = ms_lightScale * ((static_cast<float> (color.getA ()) - 128.f) * ms_oo128);
			}
		}

		delete image;
	}
	else
	{
		DEBUG_WARNING (fileName && *fileName, ("EnvironmentBlock::loadColorRamps - image [%s] loaded, but is not in the appropriate format (256w x 8h x 32b tga)", fileName));

		if (image)
			delete image;

		//-- default color ramps
		int i;
		for (i = 0; i < 256; ++i)
		{
			m_ambientColorRamp [i] = PackedRgb::solidGray;
			m_celestialAlphaRamp [i] = 1.f;
			m_mainDiffuseColorRamp [i] = PackedRgb::solidWhite;
			m_mainDiffuseColorScale [i] = 1.f;
			m_mainSpecularColorRamp [i] = PackedRgb::solidWhite;
			m_mainSpecularColorScale [i] = 1.f;
			m_fillColorRamp [i] = PackedRgb::solidBlack;
			m_fillColorScale [i] = 1.f;
			m_bounceColorRamp [i] = PackedRgb::solidBlack;
			m_bounceColorScale [i] = 1.f;
			m_clearColorRamp [i] = PackedRgb (255, 0, 255);
			m_sunMoonAlphaRamp [i] = 1.f;
			m_fogColorRamp [i] = PackedRgb::solidGray;
			m_starAlphaRamp [i] = 1.f;
			m_shadowRamp [i] = VectorArgb::solidGray;
			m_mainBackColorRamp [i] = PackedRgb::solidBlack;
			m_mainBackColorScale [i] = 1.f;
			m_mainTangentColorRamp [i] = PackedRgb::solidBlack;
			m_mainTangentColorScale [i] = 1.f;
		}
	}
}

//===================================================================
