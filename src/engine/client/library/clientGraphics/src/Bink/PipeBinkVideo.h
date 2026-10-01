#ifndef INCLUDED_PipeBinkVideo_H
#define INCLUDED_PipeBinkVideo_H

#ifdef CLIENT_MILES_DEV_FACADE
#include "clientGraphics/Video.h"
#include "api/ClientBink.h"

// Development selection only. The native Bink decoder and its real Miles
// driver stay together in the private x86 host; presentation stays in SWG.
namespace PipeBinkVideoNamespace
{
	bool install(void *milesDriver);
	void remove();
}

class PipeBinkVideo : public Video
{
public:
	static Video *create(const char *name);
	virtual int getWidth() const;
	virtual int getHeight() const;
	virtual int getLoopCount() const;
	virtual bool canStretchBlt() const;
	virtual bool pause(bool enabled);
	virtual bool setVideoOnOff(bool enabled);
	virtual bool setSoundOnOff(bool enabled);
	virtual void setVolume(unsigned track, int volume);
	virtual void service();
	virtual bool performDrawing(int x, int y, int width, int height);
	virtual bool performBlitting(int x, int y);

private:
	PipeBinkVideo(const char *name, ClientBink::Handle video);
	virtual ~PipeBinkVideo();
	PipeBinkVideo(const PipeBinkVideo &);
	PipeBinkVideo &operator=(const PipeBinkVideo &);
	bool firstFrame() const;
	bool playing() const;
	void decodeFrame();
	void advanceFrame();
	void prepareFrame();
	static void copyTexture(void *context, void *pixels, int pitch, unsigned height);

	ClientBink::Handle m_video;
	ClientBink::Info m_info;
	int m_loopCount;
	bool m_didFrame;
	bool m_nextFrame;
};
#endif
#endif
