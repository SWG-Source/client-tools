#include "clientGraphics/FirstClientGraphics.h"

#ifdef CLIENT_MILES_DEV_FACADE
#include "PipeBinkVideo.h"
#include "VideoBlit.h"
#include "clientGraphics/Graphics.h"
#include <limits>

namespace PipeBinkVideoNamespace
{
	bool installed = false;
	bool dynamicTextures = false;
	void deviceLost()
	{
		if (dynamicTextures)
		{
			VideoBlit::destroy();
			dynamicTextures = false;
		}
	}
	void deviceRestored() { dynamicTextures = VideoBlit::construct(); }

	bool install(void *milesDriver)
	{
		if (installed || !milesDriver)
		{
			WARNING(true, ("Bink development adapter requires one active Miles driver.\n"));
			return false;
		}
		// The original renderer uses a 1024x1024 texture. This bounded private
		// staging allocation is separate from the genuine decoder's buffers.
		if (!ClientBink::initialize(static_cast<ClientMiles::HDIGDRIVER>(milesDriver), 4 * 1024 * 1024))
		{
			const char *error = ClientBink::lastError();
			WARNING(true, ("Bink initialization failed: %s\n", error ? error : "unknown"));
			ClientBink::shutdown();
			return false;
		}
		Graphics::addDeviceLostCallback(deviceLost);
		Graphics::addDeviceRestoredCallback(deviceRestored);
		deviceRestored();
		installed = true;
		return true;
	}
	void remove()
	{
		if (!installed)
			return;
		deviceLost();
		Graphics::removeDeviceLostCallback(deviceLost);
		Graphics::removeDeviceRestoredCallback(deviceRestored);
		ClientBink::shutdown();
		installed = false;
	}
}

Video *PipeBinkVideo::create(const char *name)
{
	ClientBink::Handle video = ClientBink::open(name);
	if (!video)
		return 0;
	try { return new PipeBinkVideo(name, video); }
	catch (...) { ClientBink::close(video); throw; }
}

PipeBinkVideo::PipeBinkVideo(const char *name, ClientBink::Handle video)
:	Video(name), m_video(video), m_info(ClientBink::info(video)),
	m_loopCount(0), m_didFrame(false), m_nextFrame(false)
{
}
PipeBinkVideo::~PipeBinkVideo() { ClientBink::close(m_video); }
int PipeBinkVideo::getWidth() const { return static_cast<int>(m_info.width); }
int PipeBinkVideo::getHeight() const { return static_cast<int>(m_info.height); }
int PipeBinkVideo::getLoopCount() const { return m_loopCount; }
bool PipeBinkVideo::canStretchBlt() const { return PipeBinkVideoNamespace::dynamicTextures; }
bool PipeBinkVideo::pause(bool value) { return ClientBink::pause(m_video, value) != 0; }
bool PipeBinkVideo::setVideoOnOff(bool value) { return ClientBink::setVideoOnOff(m_video, value) != 0; }
bool PipeBinkVideo::setSoundOnOff(bool value) { return ClientBink::setSoundOnOff(m_video, value) != 0; }
void PipeBinkVideo::setVolume(unsigned track, int volume) { ClientBink::setVolume(m_video, track, volume); }
bool PipeBinkVideo::firstFrame() const { return !m_loopCount && m_info.frame == 1; }
bool PipeBinkVideo::playing() const { return !firstFrame() && !(!getLooping() && m_loopCount); }

void PipeBinkVideo::decodeFrame()
{
	ClientBink::doFrame(m_video);
	m_didFrame = true;
	m_nextFrame = true;
}
void PipeBinkVideo::advanceFrame()
{
	if (m_nextFrame)
	{
		unsigned oldFrame = m_info.frame;
		ClientBink::nextFrame(m_video);
		m_info = ClientBink::info(m_video);
		if (m_info.frame < oldFrame)
			++m_loopCount;
		m_nextFrame = false;
	}
}
void PipeBinkVideo::service()
{
	// Keep the existing BinkVideo frame scheduling and loop policy.
	if (playing())
		while (!ClientBink::wait(m_video))
		{
			decodeFrame();
			advanceFrame();
		}
	ClientBink::service(m_video);
}
void PipeBinkVideo::prepareFrame()
{
	if (playing())
		service();
	else if (firstFrame() && !m_didFrame)
	{
		decodeFrame();
		advanceFrame();
	}
}
void PipeBinkVideo::copyTexture(void *context, void *pixels, int pitch, unsigned height)
{
	PipeBinkVideo *video = static_cast<PipeBinkVideo *>(context);
	ClientBink::copyToBuffer(video->m_video, pixels, pitch, height, ClientBink::Surface32A);
}
bool PipeBinkVideo::performDrawing(int x, int y, int width, int height)
{
	if (!PipeBinkVideoNamespace::dynamicTextures)
		return false;
	prepareFrame();
	if (m_didFrame)
	{
		VideoBlit::doFrame(this, copyTexture);
		m_didFrame = false;
	}
	VideoBlit::draw(this, x, y, width < 0 ? getWidth() : width, height < 0 ? getHeight() : height);
	return true;
}
bool PipeBinkVideo::performBlitting(int x, int y)
{
	if (PipeBinkVideoNamespace::dynamicTextures)
		return false;
	// Validate the destination before computing a pointer into the backbuffer.
	int width = Graphics::getCurrentRenderTargetWidth();
	int height = Graphics::getCurrentRenderTargetHeight();
	if (x < 0 || y < 0 || x > width || y > height || getWidth() > width - x || getHeight() > height - y)
		return false;
	prepareFrame();
	Gl_pixelRect pixels;
	if (Graphics::lockBackBuffer(pixels, 0))
	{
		ClientBink::PixelFormat format = ClientBink::Surface32A;
		unsigned bytes = 0;
		if (pixels.colorBits == 24 && pixels.alphaBits == 8) bytes = 4;
		else if (pixels.colorBits == 16 && pixels.alphaBits == 0) { format = ClientBink::Surface565; bytes = 2; }
		else if (pixels.colorBits == 15 && pixels.alphaBits == 1) { format = ClientBink::Surface5551; bytes = 2; }
		if (bytes && pixels.pitch <= static_cast<unsigned>((std::numeric_limits<int>::max)()))
		{
			uint8 *destination = static_cast<uint8 *>(pixels.pixels) + size_t(y) * pixels.pitch + size_t(x) * bytes;
			ClientBink::copyToBuffer(m_video, destination, static_cast<int>(pixels.pitch), m_info.height, format);
		}
		Graphics::unlockBackBuffer();
	}
	m_didFrame = false;
	return true;
}
#endif
