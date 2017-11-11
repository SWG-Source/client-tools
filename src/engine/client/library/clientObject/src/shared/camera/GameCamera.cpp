//===================================================================
//
// GameCamera.cpp
// asommers 2-26-99
//
// copyright 1999, bootprint entertainment
//
//===================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/GameCamera.h"

#include "clientObject/ConfigClientObject.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/AlterResult.h"
#include "sharedRandom/Random.h"
#include "sharedUtility/LocalMachineOptionManager.h"

//===================================================================

namespace GameCameraNamespace
{
	bool          ms_installed;
	bool          ms_userEnableGlare = true;
	bool          ms_disableScreenShader;
	bool          ms_disableCameraJitter;
	VectorArgb    ms_color (0.f, 0.f, 0.f, 0.f);
	const float   ms_alpha = 0.75f;
	bool          ms_disableGlare;
	const Shader* ms_screenShader;

	void remove ();
}

using namespace GameCameraNamespace;

//===================================================================
// STATIC PUBLIC GameCamera
//===================================================================

void GameCamera::install ()
{
	DEBUG_FATAL (ms_installed, ("GameCamera::install: already installed"));
	ms_installed = true;

	if (ConfigClientObject::getScreenShader () && TreeFile::exists (ConfigClientObject::getScreenShader ()))
		ms_screenShader = ShaderTemplateList::fetchShader (ConfigClientObject::getScreenShader ());
	
	LocalMachineOptionManager::registerOption(ms_userEnableGlare, "ClientObject", "enableGlare");

	DebugFlags::registerFlag (ms_disableScreenShader, "ClientObject/GameCamera", "disableScreenShader");
	DebugFlags::registerFlag (ms_disableCameraJitter, "ClientObject/GameCamera", "disableCameraJitter");

	ExitChain::add (GameCameraNamespace::remove, "GameCameraNamespace::remove");
}

//-------------------------------------------------------------------

void GameCamera::setColor (const VectorArgb& color)
{
	ms_color    = color;
	ms_color.a *= ms_alpha;
}

//-------------------------------------------------------------------

void GameCamera::setDisableGlare (const bool disableGlare)
{
	ms_disableGlare = disableGlare;
}

//-------------------------------------------------------------------

bool GameCamera::getDisableGlare ()
{
	return ms_disableGlare;
}

//-------------------------------------------------------------------

void GameCamera::setUserEnableGlare (bool enableGlare)
{
	ms_userEnableGlare = enableGlare;
}

//-------------------------------------------------------------------

bool GameCamera::getUserEnableGlare ()
{
	return ms_userEnableGlare;
}

//===================================================================
// STATIC PRIVATE GameCamera
//===================================================================

void GameCameraNamespace::remove ()
{
	DEBUG_FATAL (!ms_installed, ("GameCamera::install: already installed"));
	ms_installed = true;

	DebugFlags::unregisterFlag (ms_disableScreenShader);

	if (ms_screenShader)
	{
		ms_screenShader->release ();
		ms_screenShader = 0;
	}
}

//===================================================================
// PUBLIC GameCamera
//===================================================================

GameCamera::GameCamera () : 
	RenderWorldCamera (),
	m_jittering (false),
	m_jitterDirection_w (),
	m_jitterTimer (0.f),
	m_jitterAngle (0),
	m_jitterFrequency (0),
	m_flashing (false),
	m_flashTimer (0.f),
	m_flashColor ()
{
}

//-------------------------------------------------------------------

GameCamera::~GameCamera ()
{
}

//-------------------------------------------------------------------

void GameCamera::jitter (const Vector& jitterDirection_w, const float jitterTime, const float jitterAngle, const float jitterFrequency)
{
	m_jittering         = !ms_disableCameraJitter;
	m_jitterDirection_w = jitterDirection_w;
	m_jitterAngle       = jitterAngle;
	m_jitterFrequency   = jitterFrequency;
	m_jitterTimer.setExpireTime (jitterTime);
}

//-------------------------------------------------------------------

void GameCamera::flash (const VectorArgb& flashColor, const float flashTime)
{
	m_flashing   = true;
	m_flashColor = flashColor;
	m_flashTimer.setExpireTime (flashTime);
}

//-------------------------------------------------------------------

float GameCamera::alter (float elapsedTime)
{
	//-- jitter
	{
		if (m_jittering && m_jitterTimer.updateZero (elapsedTime))
			m_jittering = false;

		if (m_jittering)
		{
			const float angle = m_jitterAngle * m_jitterTimer.getRemainingRatio ();

			yaw_o (Random::randomReal (-angle * 0.5f, angle * 0.5f));
			pitch_o (Random::randomReal (-angle * 0.5f, angle * 0.5f));

			if (m_jitterTimer.updateZero (elapsedTime))
				m_jittering = false;
		}
	}

	//-- flash
	{
		if (m_flashing && m_flashTimer.updateZero (elapsedTime))
			m_flashing = false;

		if (m_flashing)
			m_flashColor.a = m_flashTimer.getRemainingRatio () * 0.5f;
	}

	//-- Chain up to base class.
	float const baseAlterResult = RenderWorldCamera::alter (elapsedTime);
	UNREF(baseAlterResult);

	// @todo consider returning AlterResult::cms_keepNoAlter and requesting
	//       an alter when jitter or flash occur.
	return AlterResult::cms_alterNextFrame;
}

//-------------------------------------------------------------------

void GameCamera::renderFlash () const
{
	//-- do the glare
	if (ms_userEnableGlare && !ms_disableGlare && ms_color.a > 0.f)
	{
		Graphics::setStaticShader (ShaderTemplateList::get2dVertexColorAStaticShader ().prepareToView());
		setVertexBuffer (ms_color);
		Graphics::drawTriangleFan ();
	}

	//-- screen flash
	if (m_flashing)
	{
		Graphics::setStaticShader (ShaderTemplateList::get2dVertexColorAStaticShader ().prepareToView());
		setVertexBuffer (m_flashColor);
		Graphics::drawTriangleFan ();
	}

	//-- screen shader
	if (!ms_disableScreenShader && ms_screenShader)
	{
		Graphics::setStaticShader (ms_screenShader->prepareToView ());
		setVertexBuffer (VectorArgb::solidWhite);
		Graphics::drawTriangleFan ();
	}
}

//===================================================================
// PROTECTED GameCamera
//===================================================================

void GameCamera::drawScene () const
{
	//-- draw everything normally
	RenderWorldCamera::drawScene ();
}

//===================================================================
// PROTECTED GameCamera
//===================================================================

void GameCamera::setVertexBuffer (const VectorArgb& color) const
{
	VertexBufferFormat format;
	format.setPosition ();
	format.setTransformed ();
	format.setColor0 ();
	DynamicVertexBuffer vertexBuffer (format);

	vertexBuffer.lock (4);

		VertexBufferWriteIterator v = vertexBuffer.begin ();

		float screenX0;
		float screenX1;
		float screenY0;
		float screenY1;
		getViewport (screenX0, screenY0, screenX1, screenY1);

		v.setPosition (Vector (screenX0, screenY0, 1.f));
		v.setOoz (1.f);
		v.setColor0 (color);
		++v;

		v.setPosition (Vector (screenX1, screenY0, 1.f));
		v.setOoz (1.f);
		v.setColor0 (color);
		++v;

		v.setPosition (Vector (screenX1, screenY1, 1.f));
		v.setOoz (1.f);
		v.setColor0 (color);
		++v;

		v.setPosition (Vector (screenX0, screenY1, 1.f));
		v.setOoz (1.f);
		v.setColor0 (color);

	vertexBuffer.unlock ();

	Graphics::setVertexBuffer (vertexBuffer);
}

//===================================================================

