// ======================================================================
//
// BinkVideo.cpp
//
// Copyright 2001 - 2003 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/BinkVideo.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/TextureList.h"
#include "clientGraphics/Texture.def"
#include "clientGraphics/Texture.h"
#include "clientGraphics/VertexBufferFormat.h"
#include "clientGraphics/VertexBufferIterator.h"

#include "sharedFile/TreeFile.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"

#define USE_BINK_TREE_FILE_IO 1
#define USE_BINK_MEMORY_FILE 0

#if USE_BINK_TREE_FILE_IO
#include "clientGraphics/BinkTreeFileIO.h"
#endif

// ===============================================================================================

namespace BinkVideoNamespace
{
	// ----------------------------------------------------------------

	bool install(void *hMilesDigitalDriver);
	void remove();

	// ----------------------------------------------------------------

	static void _onDeviceLost();
	static void _onDeviceRestored();

	// ----------------------------------------------------------------

	static void * RADLINK _bink_malloc(U32 numBytes) { return new unsigned char[numBytes]; }
	static void   RADLINK _bink_free(void *p)        { delete [] (unsigned char *)p;       }

	// ----------------------------------------------------------------

	static bool            s_installed;
	static const char     *s_dllName = "binkw32.dll";
	static bool            s_dynamicTextures;

	// ----------------------------------------------------------------

	class TextureBlit
	{
	public:

		enum { MAX_WIDTH=1024, MAX_HEIGHT=1024 };

		static bool construct();
		static void destroy();
		static void doFrame(BinkVideo *video);
		static void draw(BinkVideo *video, int screenX, int screenY, int screenCX, int screenCY);

		static Texture *            s_dynamicTexture;
		static Shader *             s_videoBlitShader;
		static VertexBufferFormat   s_vertexFormat;
		static DynamicVertexBuffer *s_vertexBuffer;
		static GlMatrix4x4          s_projectionMatrix;
	};
	Texture *            TextureBlit::s_dynamicTexture;
	Shader *             TextureBlit::s_videoBlitShader;
	VertexBufferFormat   TextureBlit::s_vertexFormat;
	DynamicVertexBuffer *TextureBlit::s_vertexBuffer;
	GlMatrix4x4          TextureBlit::s_projectionMatrix;

	// ----------------------------------------------------------------

}
using namespace BinkVideoNamespace;

using namespace Bink;

// ===============================================================================================

bool BinkVideoNamespace::install(void *hMilesDigitalDriver)
{
	if (s_installed)
	{
		WARNING(true, ("Nested calls to Bink video install are not supported.\n"));
		return false;
	}

	if (!bindBink(s_dllName))
	{
		WARNING(true, ("Error binding to Bink video DLL (%s)!\n", s_dllName));
		return false;
	}

	if (!isBinkReady())
	{
		WARNING(true, ("Error initializing Bink video!\n"));
		unbindBink();
		return false;
	}

	BinkSetMemory(_bink_malloc, _bink_free);

	if (hMilesDigitalDriver)
	{
		BinkSoundUseMiles(hMilesDigitalDriver);
	}

	Graphics::addDeviceLostCallback(_onDeviceLost);
	Graphics::addDeviceRestoredCallback(_onDeviceRestored);

	_onDeviceRestored();

	s_installed=true;
	return true;
}

// ----------------------------------------------------------------------

void BinkVideoNamespace::remove()
{
	if (!s_installed)
	{
		WARNING(true, ("Rejected attempt to remove Bink video.  Bink video was not installed.\n"));
		return;
	}

	_onDeviceLost();

	Graphics::removeDeviceLostCallback(_onDeviceLost);
	Graphics::removeDeviceRestoredCallback(_onDeviceRestored);

	if (s_dynamicTextures)
	{
		TextureBlit::destroy();
		s_dynamicTextures = false;
	}

	unbindBink();
	s_installed=false;
}

// ----------------------------------------------------------------------

void BinkVideoNamespace::_onDeviceLost()
{
	if (s_dynamicTextures)
	{
		TextureBlit::destroy();
		s_dynamicTextures = false;
	}
}

// ----------------------------------------------------------------------

void BinkVideoNamespace::_onDeviceRestored()
{
	s_dynamicTextures = TextureBlit::construct();
}

// ===============================================================================================

bool BinkVideoNamespace::TextureBlit::construct()
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

void BinkVideoNamespace::TextureBlit::destroy()
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

void BinkVideoNamespace::TextureBlit::doFrame(BinkVideo *video)
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
		BINK_COPY_FLAGS pixelFormat=BINKSURFACE32A;
		uint8 *destPixels = (uint8 *)ldata.getPixelData();
		//memset(destPixels, 0xff, ldata.getPitch() * ldata.getHeight());
		video->copyToBuffer(destPixels, ldata.getPitch(), ldata.getHeight(), 0, 0, pixelFormat | BINKCOPYALL);
	}
	s_dynamicTexture->unlock(ldata);
	// -----------------------------------------------------------------------
}

// ----------------------------------------------------------------------

void BinkVideoNamespace::TextureBlit::draw(BinkVideo *video, int screenX, int screenY, int screenCX, int screenCY)
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

BinkVideo *BinkVideo::newBinkVideo(const char *name)
{
	BinkVideo *returnValue=0;
	BINK_OPEN_FLAGS openFlags = 0;
	const char *openParam=0;

	// ----------------------------------------------

	#if USE_BINK_TREE_FILE_IO
	{
		BinkSetIO(BinkTreeFileIO::getBinkOpenFileFunction());
		BinkSetIOSize(1024*1024);
		openFlags |= (BINKIOPROCESSOR | BINKIOSIZE);
		openParam=name;
	}
	#elif USE_BINK_MEMORY_FILE
	{
		AbstractFile *videoFile = TreeFile::open(name, AbstractFile::PriorityAudioVideo, true);
		if (videoFile)
		{
			unsigned char *const data = videoFile->readEntireFileAndClose();
			if (data)
			{
				openFlags |= BINKFROMMEMORY;
				openParam=(const char *)data;
			}
			delete videoFile;
		}
	}
	#else
	{
		openParam=name;
	}
	#endif

	// ----------------------------------------------
	if (openParam)
	{
		HBINK hvideo = BinkOpen(openParam, openFlags);
		if (hvideo)
		{
			returnValue = new BinkVideo(name, openParam, hvideo);
		}
	}

	return returnValue;
}

// ===============================================================================================

BinkVideo::BinkVideo(const char *name, const char *const openParameter, HBINK video)
:	  Video(name)
	, m_video(video)
	, m_openParameter(openParameter)
	, m_loopCount(0)
	, m_didFrame(false)
	, m_nextFrame(false)
{
}

// ----------------------------------------------------------------------

BinkVideo::~BinkVideo()
{
	if (m_video)
	{
		BinkClose(m_video);
	}
	#if USE_BINK_MEMORY_FILE
	if (m_openParameter)
	{
		delete [] (unsigned char *)m_openParameter;
	}
	#endif
}

// ----------------------------------------------------------------------

int BinkVideo::getWidth() const
{
	return (m_video) ? m_video->Width : 0;
}

// ----------------------------------------------------------------------

int BinkVideo::getHeight() const
{
	return (m_video) ? m_video->Height : 0;
}

// ======================================================================

int BinkVideo::getLoopCount() const
{
	return m_loopCount;
}

// ======================================================================

bool BinkVideo::canStretchBlt() const
{
	return s_dynamicTextures;
}

// ======================================================================

inline bool BinkVideo::_isFirstFrame() const
{
	return !m_loopCount && m_video->FrameNum==1;
}

// ======================================================================

inline bool BinkVideo::_isFinished() const
{
	return !getLooping() && m_loopCount;
}

// ======================================================================

inline bool BinkVideo::_isPlaying() const
{
	return !_isFirstFrame() && !_isFinished();
}

// ======================================================================

void BinkVideo::_doFrame()
{
	doFrame();
	m_didFrame=true;
	m_nextFrame=true;
}

// ======================================================================

void BinkVideo::_nextFrame()
{
	if (m_nextFrame)
	{
		const int currentFrame = m_video->FrameNum;
		nextFrame();
		const int nextFrame = m_video->FrameNum;

		if (nextFrame<currentFrame)
		{
			m_loopCount++;
		}

		m_nextFrame=false;
	}
}

// ======================================================================

void BinkVideo::service()
{ 
	if (_isPlaying())
	{
		while (!wait())
		{
			_doFrame();
			_nextFrame();
		}
	}

	BinkService(m_video);
}

// ======================================================================

bool BinkVideo::performDrawing(int screenX, int screenY, int screenCX, int screenCY)
{
	if (!s_dynamicTextures)
	{
		return false;
	}

	// ----------------------------------------------------------------------

	if (_isPlaying())
	{
		service();
	}
	else if (_isFirstFrame() && !m_didFrame)
	{
		_doFrame();
		_nextFrame();
	}

	// ----------------------------------------------------------------------

	if (m_didFrame)
	{
		TextureBlit::doFrame(this);
		m_didFrame=false;
	}

	if (screenCX<0)
	{
		screenCX=m_video->Width;
	}
	if (screenCY<0)
	{
		screenCY=m_video->Height;
	}
	TextureBlit::draw(this, screenX, screenY, screenCX, screenCY);

	//REPORT_LOG_PRINT(true, ("Video frame: %i\n", m_video->FrameNum));

	return true;
}

// ======================================================================

bool BinkVideo::performBlitting(int screenX, int screenY)
{
	if (s_dynamicTextures)
	{
		return false;
	}
	// ----------------------------------------------------------------------

	// ----------------------------------------------------------------------

	if (_isPlaying())
	{
		service();
	}
	else if (_isFirstFrame() && !m_didFrame)
	{
		_doFrame();
		_nextFrame();
	}

	// ----------------------------------------------------------------------

	// ----------------------------------------------------------------------
	// Lock the backbuffer and blit the video onto it.
	Gl_rect lockRect;

	lockRect.x0 = screenX;
	lockRect.y0 = screenY;
	lockRect.x1 = screenX + m_video->Width;
	lockRect.y1 = screenY + m_video->Height;
	Gl_pixelRect pixels;
	if (Graphics::lockBackBuffer(pixels, 0))
	{
		// - - - - - - - - - - - - - - - - - - - - - - - - - - 
		// find a Bink pixel format to match the back-buffer.
		unsigned pixelFormat=0;
		int bpp=0;

		if (pixels.colorBits==24)
		{
			if (pixels.alphaBits==8)
			{
				pixelFormat=BINKSURFACE32A;
				bpp=4;
			}
		}
		else if (pixels.colorBits==16)
		{
			if (pixels.alphaBits==0)
			{
				pixelFormat=BINKSURFACE565;
				bpp=2;
			}
		}
		else if (pixels.colorBits==15)
		{
			if (pixels.alphaBits==1)
			{
				pixelFormat=BINKSURFACE5551;
				bpp=2;
			}
		}

		// - - - - - - - - - - - - - - - - - - - - - - - 
		// if a format was matched, copy/reformat the pixels.
		if (bpp>0)
		{
			uint8 *destPixels = (uint8 *)(pixels.pixels) + screenY*pixels.pitch + screenX*bpp;
			copyToBuffer(destPixels, pixels.pitch, m_video->Height, 0, 0, pixelFormat | BINKCOPYALL);
		}

		Graphics::unlockBackBuffer();
	}
	m_didFrame=false;
	// ----------------------------------------------------------------------

	//REPORT_LOG_PRINT(true, ("Video frame: %i\n", m_video->FrameNum));

	return true;
}

// ----------------------------------------------------------------------

// ======================================================================
