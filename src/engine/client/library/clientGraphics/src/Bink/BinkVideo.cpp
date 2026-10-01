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
#include "VideoBlit.h"
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

	static void copyTexture(void *context, void *pixels, int pitch, unsigned height);

	// ----------------------------------------------------------------

}
using namespace BinkVideoNamespace;

using namespace Bink;

void BinkVideoNamespace::copyTexture(void *context, void *pixels, int pitch, unsigned height)
{
	static_cast<BinkVideo *>(context)->copyToBuffer(pixels, pitch, height, 0, 0, BINKSURFACE32A | BINKCOPYALL);
}

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
		VideoBlit::destroy();
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
		VideoBlit::destroy();
		s_dynamicTextures = false;
	}
}

// ----------------------------------------------------------------------

void BinkVideoNamespace::_onDeviceRestored()
{
	s_dynamicTextures = VideoBlit::construct();
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
		VideoBlit::doFrame(this, copyTexture);
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
	VideoBlit::draw(this, screenX, screenY, screenCX, screenCY);

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
