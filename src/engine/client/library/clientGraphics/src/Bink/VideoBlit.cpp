// ======================================================================
// Shared texture blit extracted from BinkVideo.cpp.
// Copyright 2001 - 2003 Sony Online Entertainment
// All Rights Reserved
// ======================================================================
#include "clientGraphics/FirstClientGraphics.h"
#include "VideoBlit.h"
#include "clientGraphics/Video.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/TextureList.h"
#include "clientGraphics/Texture.def"
#include "clientGraphics/Texture.h"
#include "clientGraphics/VertexBufferFormat.h"
#include "clientGraphics/VertexBufferIterator.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"

namespace VideoBlit
{
	enum { MAX_WIDTH=1024, MAX_HEIGHT=1024 };
	static Texture *            s_dynamicTexture;
	static Shader *             s_videoBlitShader;
	static VertexBufferFormat   s_vertexFormat;
	static DynamicVertexBuffer *s_vertexBuffer;
	static GlMatrix4x4          s_projectionMatrix;
}

bool VideoBlit::construct()
{
	if (!Graphics::supportsDynamicTextures())
	{
		return false;
	}

	TextureFormat formats[] = { TF_ARGB_8888 };
	s_dynamicTexture  = TextureList::fetch(TCF_dynamic, MAX_WIDTH, MAX_HEIGHT, 1, formats, 1);
	if (!s_dynamicTexture)
	{
		return false;
	}

	s_videoBlitShader = ShaderTemplateList::fetchModifiableShader("shader/video_blit.sht");
	if (!s_videoBlitShader)
	{
		s_dynamicTexture->release();
		s_dynamicTexture=0;
		return false;
	}

	safe_cast<StaticShader *>(s_videoBlitShader)->setTexture(TAG(M,A,I,N), *s_dynamicTexture);

	s_vertexFormat.setPosition();
	s_vertexFormat.setColor0(true);
	s_vertexFormat.setNumberOfTextureCoordinateSets(1);
	s_vertexFormat.setTextureCoordinateSetDimension(0, 2);

	s_vertexBuffer = new DynamicVertexBuffer(s_vertexFormat);

	s_projectionMatrix.matrix[0][0] = 0.f;
	s_projectionMatrix.matrix[0][1] = 0.f;
	s_projectionMatrix.matrix[0][2] = 0.f;
	s_projectionMatrix.matrix[0][3] = 0.f;

	s_projectionMatrix.matrix[1][0] = 0.f;
	s_projectionMatrix.matrix[1][1] = 0.f;
	s_projectionMatrix.matrix[1][2] = 0.f;
	s_projectionMatrix.matrix[1][3] = 0.f;

	s_projectionMatrix.matrix[2][0] = 0.f;
	s_projectionMatrix.matrix[2][1] = 0.f;
	s_projectionMatrix.matrix[2][2] = 1.f;
	s_projectionMatrix.matrix[2][3] = 0.f;

	s_projectionMatrix.matrix[3][0] = 0.f;
	s_projectionMatrix.matrix[3][1] = 0.f;
	s_projectionMatrix.matrix[3][2] = 0.f;
	s_projectionMatrix.matrix[3][3] = 1.f;

	return true;
}

// ----------------------------------------------------------------------

void VideoBlit::destroy()
{
	if (s_vertexBuffer)
	{
		delete s_vertexBuffer;
		s_vertexBuffer=0;
	}

	if (s_videoBlitShader)
	{
		s_videoBlitShader->release();
		s_videoBlitShader=0;
	}

	if (s_dynamicTexture)
	{
		s_dynamicTexture->release();
		s_dynamicTexture=0;
	}
}

// ----------------------------------------------------------------------

void VideoBlit::doFrame(void *context, void (*copy)(void *, void *, int, unsigned))
{
	// -----------------------------------------------------------------------
	// Lock the backbuffer and blit the video onto it.
	const TextureFormat format = s_dynamicTexture->getNativeFormat();
	Texture::LockData ldata(
		format,
		0,
		0,
		0,
		s_dynamicTexture->getWidth(),
		s_dynamicTexture->getHeight(),
		true
	);

	s_dynamicTexture->lock(ldata);
	if (ldata.getPixelData())
	{
		// copy data to texture.
		uint8 *destPixels = (uint8 *)ldata.getPixelData();
		copy(context, destPixels, ldata.getPitch(), ldata.getHeight());
	}
	s_dynamicTexture->unlock(ldata);
	// -----------------------------------------------------------------------
}

// ----------------------------------------------------------------------

void VideoBlit::draw(Video *video, int screenX, int screenY, int screenCX, int screenCY)
{
	// -----------------------------------------------------------------------

	int width = Graphics::getCurrentRenderTargetWidth();
	int height = Graphics::getCurrentRenderTargetHeight();
	Graphics::setViewport (0, 0, width, height);

	const GlCullMode previusCullMode = Graphics::getCullMode();
	Graphics::setCullMode(GCM_none);

	const GlFillMode previusFillMode = Graphics::getFillMode();
	Graphics::setFillMode(GFM_solid);

	Graphics::setScissorRect(false, 0, 0, 0, 0);

	Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);

	Graphics::setWorldToCameraTransform(Transform::identity, Vector::zero);

	// -----------------------------------------------------------------------
	//-- setup parallel projection matrix
	const float oodx = 1.0f/float(width);
	s_projectionMatrix.matrix[0][0] =  2.f * oodx;
	s_projectionMatrix.matrix[0][3] = -2.f * float(0) * oodx - 1.f;

	const float oody = 1.0f/float(height);
	s_projectionMatrix.matrix[1][1] = -2.f * oody;
	s_projectionMatrix.matrix[1][3] =  2.f * float(0) * oody + 1.f;

	Graphics::setProjectionMatrix(s_projectionMatrix);
	// -----------------------------------------------------------------------

	// ----------------------------------------------------------------------
	// Draw the video onto the screen using a textured quad
	const float videoX0 = float(0.5f) / float(MAX_WIDTH);
	const float videoY0 = float(0.5f) / float(MAX_HEIGHT);
	const float videoWidth = float(video->getWidth()) / float(MAX_WIDTH);
	const float videoHeight = float(video->getHeight()) / float(MAX_HEIGHT);
	s_vertexBuffer->lock(4);
	{
		VertexBufferWriteIterator vbiter = s_vertexBuffer->begin();

		vbiter.setPosition(float(screenX), float(screenY), 1);
		vbiter.setColor0(0xffffffff);
		vbiter.setTextureCoordinates(0, videoX0, videoY0);
		++vbiter;

		vbiter.setPosition(float(screenX), float(screenY + screenCY), 1);
		vbiter.setColor0(0xffffffff);
		vbiter.setTextureCoordinates(0, videoX0, videoHeight);
		++vbiter;

		vbiter.setPosition(float(screenX + screenCX), float(screenY + screenCY), 1);
		vbiter.setColor0(0xffffffff);
		vbiter.setTextureCoordinates(0, videoWidth, videoHeight);
		++vbiter;

		vbiter.setPosition(float(screenX + screenCX), float(screenY), 1);
		vbiter.setColor0(0xffffffff);
		vbiter.setTextureCoordinates(0, videoWidth, videoY0);
		++vbiter;
	}
	s_vertexBuffer->unlock();

	Graphics::setVertexBuffer(*s_vertexBuffer);
	// ----------------------------------------------------------------------

	Graphics::setStaticShader(s_videoBlitShader->prepareToView(), 0);
	Graphics::drawTriangleFan(0, 2);

	// -----------------------------------------------------------------------

	Graphics::setCullMode(previusCullMode);
	Graphics::setFillMode(previusFillMode);
}

// ===============================================================================================
