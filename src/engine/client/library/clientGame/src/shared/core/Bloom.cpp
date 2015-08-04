// ======================================================================
//
// Bloom.cpp
// Copyright 2004, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/Bloom.h"

#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/GraphicsOptionTags.h"
#include "clientGraphics/PostProcessingEffectsManager.h"
#include "clientGraphics/ShaderCapability.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/FloatMath.h"
#include "sharedMath/VectorRgba.h"
#include "sharedUtility/LocalMachineOptionManager.h"

// ======================================================================

namespace BloomNamespace
{
	void deviceRestored();
	void deviceLost();
	void setBlurPixelShaderUserConstants(float xStep, float yStep);

	bool ms_enable = true;
	bool ms_enabled;
#ifdef _DEBUG
	bool ms_viewOriginalAlpha;
	bool ms_viewBlurredAlpha;
#endif

	Texture * ms_smallBackBuffer;
	Texture * ms_blurHorizontalTexture;
	Texture * ms_blurVerticalTexture;

	StaticShader * ms_downSampleShader;
	StaticShader * ms_blurShader;
	StaticShader * ms_bloomShader;

#ifdef _DEBUG
	StaticShader * ms_copyShader;
	StaticShader * ms_viewAlphaShader;
#endif

	float ms_standardDeviation;
	float ms_weightMultiplier;
}
using namespace BloomNamespace;

// ======================================================================

void Bloom::install()
{
	InstallTimer const installTimer("Bloom::install");

	ExitChain::add(Bloom::remove, "Bloom::remove");

	LocalMachineOptionManager::registerOption(ms_enable, "ClientGame/Bloom", "enable");

#ifdef _DEBUG
	DebugFlags::registerFlag (ms_enable, "ClientGame/Bloom", "enabled");
	DebugFlags::registerFlag (ms_viewOriginalAlpha, "ClientGame/Bloom", "viewOriginalAlpha");
	DebugFlags::registerFlag (ms_viewBlurredAlpha,  "ClientGame/Bloom", "viewBlurredAlpha");
#endif

	ms_standardDeviation = ConfigFile::getKeyFloat ("ClientGame/Bloom", "standardDeviation", 6.0f);
	ms_weightMultiplier = ConfigFile::getKeyFloat ("ClientGame/Bloom",  "weightMultiplier", 1.75f);

	if (ms_enable)
		enable();
}

// ----------------------------------------------------------------------

void Bloom::remove()
{
	disable();
}

// ----------------------------------------------------------------------

bool Bloom::isSupported()
{
	return GraphicsOptionTags::get(TAG(P,O,S,T)) && Graphics::getShaderCapability() >= ShaderCapability(2,0);
}

// ----------------------------------------------------------------------

bool Bloom::isEnabled()
{
	return ms_enable;
}

// ----------------------------------------------------------------------

void Bloom::setEnabled(bool const enable)
{
	ms_enable = enable;
}

// ----------------------------------------------------------------------

void Bloom::enable()
{
	if (!ms_enabled)
	{
		if (Bloom::isSupported())
		{
			Graphics::addDeviceLostCallback(BloomNamespace::deviceLost);
			Graphics::addDeviceRestoredCallback(BloomNamespace::deviceRestored);
			Graphics::setBloomEnabled(true);
			deviceRestored();
			ms_enabled = true;
		}
		else
		{
			ms_enable = false;
			ms_enabled = false;
		}
	}
}

// ----------------------------------------------------------------------

void Bloom::disable()
{
	if (ms_enabled)
	{
		deviceLost();
		Graphics::removeDeviceLostCallback(deviceLost);
		Graphics::removeDeviceRestoredCallback(deviceRestored);
		Graphics::setBloomEnabled(false);

		ms_enable = false;
		ms_enabled = false;
	}
}

// ----------------------------------------------------------------------

void BloomNamespace::deviceRestored()
{
	int const smallWidth = Graphics::getFrameBufferMaxWidth() / 4;
	int const smallHeight = Graphics::getFrameBufferMaxHeight() / 4;

	TextureFormat formats[] = { TF_ARGB_8888 };

	ms_downSampleShader = dynamic_cast<StaticShader *>(ShaderTemplateList::fetchModifiableShader("shader/2d_downsample_4x4.sht"));
	ms_smallBackBuffer = TextureList::fetch(TCF_renderTarget, smallWidth, smallHeight, 1, formats, sizeof(formats) / sizeof(formats[0]));

	ms_blurShader = dynamic_cast<StaticShader *>(ShaderTemplateList::fetchModifiableShader("shader/2d_blur.sht"));
	ms_blurHorizontalTexture = TextureList::fetch(TCF_renderTarget, smallWidth, smallHeight, 1, formats, sizeof(formats) / sizeof(formats[0]));
	ms_blurVerticalTexture = TextureList::fetch(TCF_renderTarget, smallWidth, smallHeight, 1, formats, sizeof(formats) / sizeof(formats[0]));

	ms_bloomShader = dynamic_cast<StaticShader *>(ShaderTemplateList::fetchModifiableShader("shader/2d_bloom.sht"));
	ms_bloomShader->setTexture(TAG(S,M,A,L), *ms_smallBackBuffer);

#ifdef _DEBUG
	ms_copyShader = dynamic_cast<StaticShader *>(ShaderTemplateList::fetchModifiableShader("shader/2d_texture.sht"));
	ms_viewAlphaShader = dynamic_cast<StaticShader *>(ShaderTemplateList::fetchModifiableShader("shader/2d_view_alpha.sht"));
#endif
}

// ----------------------------------------------------------------------

void BloomNamespace::deviceLost()
{
	if (ms_downSampleShader)
	{
		ms_downSampleShader->release();
		ms_downSampleShader = NULL;
	}

	if (ms_blurShader)
	{
		ms_blurShader->release();
		ms_blurShader = NULL;
	}

	if (ms_bloomShader)
	{
		ms_bloomShader->release();
		ms_bloomShader= NULL;
	}

#ifdef _DEBUG
	if (ms_copyShader)
	{
		ms_copyShader->release();
		ms_copyShader= NULL;
	}

	if (ms_viewAlphaShader)
	{
		ms_viewAlphaShader->release();
		ms_viewAlphaShader= NULL;
	}
#endif

	if (ms_smallBackBuffer)
	{
		ms_smallBackBuffer->release();
		ms_smallBackBuffer = NULL;
	}

	if (ms_blurHorizontalTexture)
	{
		ms_blurHorizontalTexture->release();
		ms_blurHorizontalTexture = NULL;
	}

	if (ms_blurVerticalTexture)
	{
		ms_blurVerticalTexture->release();
		ms_blurVerticalTexture = NULL;
	}
}

// ----------------------------------------------------------------------

void BloomNamespace::setBlurPixelShaderUserConstants(float xStep, float yStep)
{
	VectorRgba weights[17];

	weights[16].r = xStep;
	weights[16].g = yStep;

	int x;
	for (x = 0; x < 16; ++x)
	{
		float result = static_cast<float>(GaussianDistribution(static_cast<float>(x), ms_standardDeviation, 0.0f));

		// texel 0 will be sampled twice, so weight it half as much
		if (x == 0)
			result /= 2.0f;

		weights[x].r = result;
		weights[x].g = result;
		weights[x].b = result;
		weights[x].a = result;
	}

	// normalize and scale
	{
		float sum = 0.0;

		for (x = 0; x < 16; ++x)
			sum += weights[x].r * 2;

		for (x = 0; x < 16; ++x)
		{
			float const result = (weights[x].r / static_cast<float>(sum)) * ms_weightMultiplier;
			weights[x].r = result;
			weights[x].g = result;
			weights[x].b = result;
			weights[x].a = result;
		}
	}

	Graphics::setPixelShaderUserConstants(weights, sizeof(weights) / sizeof(weights[0]));
}

// ----------------------------------------------------------------------

void Bloom::preSceneRender()
{
	// handle switching between bloom enabled & disabled
	if (ms_enabled && !ms_enable)
		disable();
	else if (!ms_enabled && ms_enable)
		enable();
}

// ----------------------------------------------------------------------

void Bloom::postSceneRender()
{
	if (ms_enabled)
	{
		Texture * const primaryBuffer = PostProcessingEffectsManager::getPrimaryBuffer();
		Texture * const secondaryBuffer = PostProcessingEffectsManager::getSecondaryBuffer();

		if (!primaryBuffer || !secondaryBuffer)
			return;

		GlFillMode const fillMode = Graphics::getFillMode();
		Graphics::setFillMode(GFM_solid);

		// create the vertex format that will be reused throughout this routine
		VertexBufferFormat format;
		format.setPosition ();
		format.setTransformed ();
		format.setNumberOfTextureCoordinateSets(1);
		format.setTextureCoordinateSetDimension(0, 2);

		// downsample
		{
			int const destinationWidth = ms_smallBackBuffer->getWidth();
			int const destinationHeight = ms_smallBackBuffer->getHeight();

			DynamicVertexBuffer vertexBuffer (format);
			vertexBuffer.lock (4);

				VertexBufferWriteIterator v = vertexBuffer.begin ();

				v.setPosition (Vector (0.0f - 0.5f, 0.0f - 0.5f, 1.f));
				v.setOoz (1.f);
				v.setTextureCoordinates(0, 0.0f, 0.0f);
				++v;

				v.setPosition (Vector (static_cast<float>(destinationWidth) - 0.5f, 0.0f - 0.5f, 1.f));
				v.setOoz (1.f);
				v.setTextureCoordinates(0, 1.0f, 0.0f);
				++v;

				v.setPosition (Vector (static_cast<float>(destinationWidth) - 0.5f, static_cast<float>(destinationHeight) - 0.5f, 1.f));
				v.setOoz (1.f);
				v.setTextureCoordinates(0, 1.0f, 1.0f);
				++v;

				v.setPosition (Vector (0.0f - 0.5f, static_cast<float>(destinationHeight) - 0.5f, 1.f));
				v.setOoz (1.f);
				v.setTextureCoordinates(0, 0.0f, 1.0f);

			vertexBuffer.unlock ();

			VectorRgba rgba[16];
			int index = 0;
			for (int y = 0; y < 4; ++y)
			{
				for (int x = 0; x < 4; ++x, ++index)
				{
					rgba[index].r = (static_cast<float>(x) - 1.5f) / static_cast<float>(primaryBuffer->getWidth());
					rgba[index].g = (static_cast<float>(y) - 1.5f) / static_cast<float>(primaryBuffer->getHeight());
				}
			}

			ms_downSampleShader->setTexture(TAG(M,A,I,N), *primaryBuffer);

			Graphics::setPixelShaderUserConstants(rgba, sizeof(rgba) / sizeof(rgba[0]));
			Graphics::setRenderTarget(ms_smallBackBuffer, CF_none, 0);
			Graphics::setViewport(0, 0, destinationWidth, destinationHeight, 0.0f, 1.0f);
			Graphics::setVertexBuffer(vertexBuffer);
			Graphics::setStaticShader(*ms_downSampleShader);
			Graphics::drawTriangleFan();
		}

		// blur
		{
			DEBUG_FATAL(ms_smallBackBuffer->getWidth() != ms_blurHorizontalTexture->getWidth(), ("small and horiztonal blur widths do not match"));
			DEBUG_FATAL(ms_smallBackBuffer->getHeight() != ms_blurHorizontalTexture->getHeight(), ("small and horiztonal blur heights do not match"));
			DEBUG_FATAL(ms_blurHorizontalTexture->getWidth() != ms_blurVerticalTexture->getWidth(), ("horiztonal blur and vertical blur widths do not match"));
			DEBUG_FATAL(ms_blurHorizontalTexture->getHeight() != ms_blurVerticalTexture->getHeight(), ("horiztonal blur and vertical blur heights do not match"));

			int const width = ms_smallBackBuffer->getWidth();
			int const height = ms_smallBackBuffer->getHeight();

			DynamicVertexBuffer vertexBuffer (format);
			vertexBuffer.lock (4);

				VertexBufferWriteIterator v = vertexBuffer.begin ();

				v.setPosition (Vector (0.0f - 0.5f, 0.0f - 0.5f, 1.f));
				v.setOoz (1.f);
				v.setTextureCoordinates(0, 0.0f, 0.0f);
				++v;

				v.setPosition (Vector (static_cast<float>(width) - 0.5f, 0.0f - 0.5f, 1.f));
				v.setOoz (1.f);
				v.setTextureCoordinates(0, 1.0f, 0.0f);
				++v;

				v.setPosition (Vector (static_cast<float>(width) - 0.5f, static_cast<float>(height) - 0.5f, 1.f));
				v.setOoz (1.f);
				v.setTextureCoordinates(0, 1.0f, 1.0f);
				++v;

				v.setPosition (Vector (0.0f - 0.5f, static_cast<float>(height) - 0.5f, 1.f));
				v.setOoz (1.f);
				v.setTextureCoordinates(0, 0.0f, 1.0f);

			vertexBuffer.unlock ();

			Graphics::setVertexBuffer(vertexBuffer);

			ms_blurShader->setTexture(TAG(M,A,I,N), *ms_smallBackBuffer);
			Graphics::setRenderTarget(ms_blurHorizontalTexture, CF_none, 0);
			Graphics::setViewport(0, 0, width, height, 0.0f, 1.0f);
			Graphics::clearViewport(true, 0, false, 0.0f, false, 0);
			Graphics::setStaticShader(*ms_blurShader);

			setBlurPixelShaderUserConstants( 1.0f / static_cast<float>(width), 0.0f);
			Graphics::drawTriangleFan();

			setBlurPixelShaderUserConstants(-1.0f / static_cast<float>(width), 0.0f);
			Graphics::drawTriangleFan();

			ms_blurShader->setTexture(TAG(M,A,I,N), *ms_blurHorizontalTexture);
			Graphics::setRenderTarget(ms_blurVerticalTexture, CF_none, 0);
			Graphics::setViewport(0, 0, width, height, 0.0f, 1.0f);
			Graphics::clearViewport(true, 0, false, 0.0f, false, 0);
			Graphics::setStaticShader(*ms_blurShader);

			setBlurPixelShaderUserConstants( 0.0f,  1.0f / static_cast<float>(height));
			Graphics::drawTriangleFan();

			setBlurPixelShaderUserConstants( 0.0f, -1.0f / static_cast<float>(height));
			Graphics::drawTriangleFan();
		}

		// copy the back buffer to the frame buffer
		{
			DynamicVertexBuffer vertexBuffer (format);

			StaticShader const * shader = 0;
			int destinationWidth = primaryBuffer->getWidth();
			int destinationHeight = primaryBuffer->getHeight();

			vertexBuffer.lock (4);

				VertexBufferWriteIterator v = vertexBuffer.begin ();

				v.setPosition (Vector (0.0f - 0.5f, 0.0f - 0.5f, 1.f));
				v.setOoz (1.f);
				v.setTextureCoordinates(0, 0.0f, 0.0f);
				++v;

				v.setPosition (Vector (static_cast<float>(destinationWidth) - 0.5f, 0.0f - 0.5f, 1.f));
				v.setOoz (1.f);
				v.setTextureCoordinates(0, 1.0f, 0.0f);
				++v;

				v.setPosition (Vector (static_cast<float>(destinationWidth) - 0.5f, static_cast<float>(destinationHeight) - 0.5f, 1.f));
				v.setOoz (1.f);
				v.setTextureCoordinates(0, 1.0f, 1.0f);
				++v;

				v.setPosition (Vector (0.0f - 0.5f, static_cast<float>(destinationHeight) - 0.5f, 1.f));
				v.setOoz (1.f);
				v.setTextureCoordinates(0, 0.0f, 1.0f);

			vertexBuffer.unlock ();

#ifdef _DEBUG
			if (ms_viewOriginalAlpha)
			{
				// view the original source alpha
				ms_viewAlphaShader->setTexture(TAG(M,A,I,N), *primaryBuffer);
				shader = ms_viewAlphaShader;
			}
			else if (ms_viewBlurredAlpha)
			{
				// view the blurred alpha
				ms_viewAlphaShader->setTexture(TAG(M,A,I,N), *ms_blurVerticalTexture);
				shader = ms_viewAlphaShader;
			}
			else
#endif
			{
				// view the scene with the blurred alpha composited on top
				ms_bloomShader->setTexture(TAG(M,A,I,N), *primaryBuffer);
				ms_bloomShader->setTexture(TAG(S,M,A,L), *ms_blurVerticalTexture);
				shader = ms_bloomShader;
			}

			Graphics::setRenderTarget(secondaryBuffer, CF_none, 0);
			Graphics::setViewport(0, 0, destinationWidth, destinationHeight, 0.0f, 1.0f);
			Graphics::setVertexBuffer(vertexBuffer);
			Graphics::setStaticShader(*shader);
			Graphics::drawTriangleFan();

			//-- after rendering to the secondary buffer, make it the primary
			PostProcessingEffectsManager::swapBuffers();
		}

		Graphics::setFillMode(fillMode);
	}
}

// ======================================================================
