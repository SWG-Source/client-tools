// ======================================================================
//
// BinkVideo.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_BinkVideo_H
#define INCLUDED_BinkVideo_H

// ======================================================================

#include "clientGraphics/Video.h"
#include "clientGraphics/BinkDLL.h"

// ======================================================================

class BinkVideo : public Video
{
public:

	static BinkVideo *newBinkVideo(const char *name);

	// ===================================================================
	// General Properties
	// ===================================================================

	virtual int   getWidth() const;
	virtual int   getHeight() const;
	virtual int   getLoopCount() const;

	virtual bool  canStretchBlt() const;

	// ===================================================================
	// Playback control.
	// ===================================================================

	// ----------------------------------------------------------
	// This function allows you to easily pause the Bink playback. 
	// Pass in true to pause the movie, and a false to resume the 
	// movie after a pause. Once you call this function, wait will 
	// return true until you resume the movie. The audio will also 
	// be paused after this function has been called.
	bool pause(bool enable)                                              { return Bink::BinkPause(m_video, enable?1:0)!=0; }
	// ----------------------------------------------------------

	// ----------------------------------------------------------
	// This function allows you to immediately skip to another 
	// frame in a Bink file. The frame_num parameter specifies which 
	// frame to skip to (the first frame is frame 1). 
	void goTo(unsigned frameNumber, unsigned binkGotoFlags)              { Bink::BinkGoto(m_video, frameNumber, binkGotoFlags); }
	// ----------------------------------------------------------

	// ----------------------------------------------------------
	// This function allows you to shut off the video decompression 
	// of a playing Bink file. This is useful when you want to only 
	// play the audio track of a Bink file and don't care about 
	// decompressing the video track. 
	bool setVideoOnOff(bool on)                                          { return Bink::BinkSetVideoOnOff(m_video, on?1:0)!=0; }
	// ----------------------------------------------------------

	// ----------------------------------------------------------
	// This function allows you to shut off the audio playback of 
	// a Bink file. This is useful if you want to stop all sound 
	// completely, or if you need to release the digital sound 
	// resource handles to play a wave or AVI file. 
	bool setSoundOnOff(bool on)                                          { return Bink::BinkSetSoundOnOff(m_video, on?1:0)!=0; }
	// ----------------------------------------------------------

	// ----------------------------------------------------------
	// This function sets the volume for the currently playing 
	// Bink sound track. 
	// The volume parameter specifies the new volume setting from 
	// 0 (silent) to 32768 (normal) to 65536 (maximum amplification). 
	void setVolume(unsigned trackID, int volume)                         { Bink::BinkSetVolume(m_video, trackID, volume); }
	// ----------------------------------------------------------

	// ----------------------------------------------------------
	// This function sets the speaker pan (left to right balance) 
	// for the currently playing Bink sound track
	void setPan(unsigned trackID, int pan)                               { Bink::BinkSetPan(m_video, trackID, pan); }
	// ----------------------------------------------------------

	// ----------------------------------------------------------
	// This function allows you to find key frames near the 
	// specified frame number. Simply pass in the frame number 
	// that you're interested in, and Bink will find the closest 
	// key frame using the search style specified by the flags 
	// parameter.
	unsigned getKeyFrame(unsigned frameNumber, unsigned binkGetKeyFlags) { Bink::BinkGetKeyFrame(m_video, frameNumber, binkGetKeyFlags); }
	// ----------------------------------------------------------

	// ===================================================================
	// Playback loop.
	// ===================================================================

	// ----------------------------------------------------------
	// Decompress on frame of video into internal buffers.
	// Returns true if the frame was skipped, false otherwise.
	bool doFrame()                                                       { return Bink::BinkDoFrame(m_video)!=0; }
	// ----------------------------------------------------------

	// ----------------------------------------------------------
	// This function actually moves you to the next frame. It is 
	// usually called after you have decompressed the frame with 
	// doFrame and have blitted it onto the screen. This 
	// function is should be the last function in your playback 
	// logic before the BinkWait delay loop. Make sure that you 
	// always call this function before waiting in your BinkWait 
	// loop, or your playback will be jerky.
	void nextFrame()                                                     { Bink::BinkNextFrame(m_video); }
	// ----------------------------------------------------------

	// ----------------------------------------------------------
	// This function tells you whether it's time to move onto the 
	// next frame. It will return a 1 if you should keep waiting, 
	// and a 0 if you should move onto the next frame.
	bool wait()                                                          { return Bink::BinkWait(m_video)!=0; }
	// ----------------------------------------------------------

	// ----------------------------------------------------------
	// This function can be used to determine if a frame or blit 
	// should be skipped. It is useful to know when you are going 
	// to skip a frame in BinkCopyToBuffer before you lock a texture 
	// or DirectDraw surface. It's also handy when using the 
	// BinkRegisterFrameBuffers function with luminance textures, 
	// since you will never call BinkCopyToBuffer at all. 
	bool shouldSkip()                                                    { return Bink::BinkShouldSkip(m_video)!=0; }
	// ----------------------------------------------------------

	// ----------------------------------------------------------
	// This function will perform background Bink services 
	// This function should be called periodically - several
	// times the video frame-rate.
	virtual void service();
	// ----------------------------------------------------------

	// ===================================================================
	// Pixel update
	// ===================================================================

	virtual bool performDrawing(int screenX, int screenY, int screenCX, int screenCY);
	virtual bool performBlitting(int screenX, int screenY);

	// ----------------------------------------------------------
	// Use this function to copy the decompressed pixels 
	// from Bink's internal decompression buffers into a 
	// destination address. This destination address can be 
	// any linear piece of memory - malloc-ed memory, 
	// DirectDraw surface pointers, the Buffer address in a 
	// BinkBuffer structure, etc. If the destination uses the 
	// RGB colorspace, then this function also performs the 
	// conversion from YUV to RGB. 
	//
	// Returns: true if the frame was skipped, false otherwise.
	bool copyToBuffer(
		void *   destBuffer, 
		int      destYStride,
		unsigned destHeight, 
		unsigned destX,
		unsigned destY,
		unsigned binkCopyFlags
		)                         { return Bink::BinkCopyToBuffer(m_video, destBuffer, destYStride, destHeight, destX, destY, binkCopyFlags)!=0; }
	// ----------------------------------------------------------

	// ----------------------------------------------------------
	// Use this function to copy the decompressed pixels 
	// from Bink's internal decompression buffers into a 
	// destination address just like copyToBuffer.
	// Unlike copyToBuffer this method supports a source
	// rectangle.
	//
	// Returns: true if the frame was skipped, false otherwise.
	bool copyToBufferRect(
		void *   destBuffer, 
		int      destYStride,
		unsigned destHeight, 
		unsigned destX,
		unsigned destY,
		unsigned srcX,
		unsigned srcY,
		unsigned srcWidth,
		unsigned srcHeight,
		unsigned binkCopyFlags
		)                         { return Bink::BinkCopyToBufferRect(m_video, destBuffer, destYStride, destHeight, destX, destY, srcX, srcY, srcWidth, srcHeight, binkCopyFlags)!=0; }
	// ----------------------------------------------------------

	// ----------------------------------------------------------
	// This function returns one or more dirty rectangles that outline 
	// the frame areas that changed in the last call to BinkDoFrame. 
	// The dirty rectangles are placed in the FrameRects field of 
	// the BINK structure. You will usually pass the dirty rectangle 
	// information directly to the BinkBufferBlit function for 
	// blasting onto the screen. 
	// Return value: the number of dirty rectangles found.
	int getRects(unsigned binkGetRectsFlags)                             { return Bink::BinkGetRects(m_video, binkGetRectsFlags); }
	// ----------------------------------------------------------

	// ===================================================================
	// Frame Buffer Control
	// ===================================================================

	// ----------------------------------------------------------
	// This function fills out a BINKFRAMEBUFFERS structure with 
	// the details you need to allocate your own Bink frame buffers.
	void getFrameBuffersInfo(Bink::BINKFRAMEBUFFERS *set)                { Bink::BinkGetFrameBuffersInfo(m_video, set); }
	// ----------------------------------------------------------

	// ----------------------------------------------------------
	// This function tells Bink what addresses and scanline pitch 
	// values to use when decompressing to frame buffers that you 
	// have allocated manually. 
	void registerFrameBuffers(Bink::BINKFRAMEBUFFERS const *set)         { Bink::BinkRegisterFrameBuffers(m_video, set); }
	// ----------------------------------------------------------

	// ===================================================================
	// Metrics
	// ===================================================================

	// ----------------------------------------------------------
	// This function returns all kinds of useful information about 
	// a Bink file that you have just played. You will usually call 
	// this function right before you close the Bink file.
	void getSummary(Bink::BINKSUMMARY *summary)                          { Bink::BinkGetSummary(m_video, summary); }
	// ----------------------------------------------------------

	// ----------------------------------------------------------
	// This function returns realtime information about the 
	// currently playing Bink file. You can display exactly what's 
	// going on in the Bink playback while you watch the movie! 
	void getRealtime(Bink::BINKREALTIME *realTime, unsigned frameWindowLength) { Bink::BinkGetRealtime(m_video, realTime, frameWindowLength); }
	// ----------------------------------------------------------

	// ===================================================================
	// IO
	// ===================================================================

	// ----------------------------------------------------------
	// This function allows you to control Bink's background IO. 
	// This is usually used when loading game data while a video is 
	// playing. The best way to load data while a video is playing 
	// is to alternate between filling the Bink IO buffer and doing 
	// your game IO. You can use BinkGetRealtime to monitor how 
	// full the Bink buffer is and then suspend the background IO 
	// when it gets too full and resume it again when it starts to 
	// get empty. 
	// Returns: true if IO is suspended, false otherwise.
	bool controlBackgroundIO(unsigned binkBGControlFlags)                 { return Bink::BinkControlBackgroundIO(m_video, binkBGControlFlags)!=0; }
	// ----------------------------------------------------------

	// ===================================================================
	// Advanced Sound
	// ===================================================================

	// ----------------------------------------------------------
	// Returns the track ID of the specified track index. Track 
	// indexes in Bink always range from zero to the maximum number 
	// of audio tracks minus 1 in the file. This function allows 
	// you to get the original track ID that was entered by the 
	// person that mixed your Bink files. 
	unsigned getTrackID(unsigned trackIndex)                              { return Bink::BinkGetTrackID(m_video, trackIndex); }
	// ----------------------------------------------------------

/*
{ return Bink::BinkOpenTrack = (HBINKTRACK (BINK_CALL *)(HBINK,U32))_bind("_BinkOpenTrack@8");
*/

	// ===================================================================
	// 
	// ===================================================================

	static Bink::BINK_COPY_FLAGS DX9SurfaceType(LPDIRECT3DSURFACE9 lpDirect3DSurface) { return Bink::BinkDX9SurfaceType(lpDirect3DSurface); }

private:
	
	BinkVideo();                              /// Disabled.
	BinkVideo(const BinkVideo &);             /// Disabled.
	BinkVideo &operator =(const BinkVideo &); /// Disabled.

private:

	BinkVideo(const char *name, const char *const openParameter, Bink::HBINK);
	virtual ~BinkVideo();

	// ----------------------------------------------------------

	bool _isFirstFrame() const;
	bool _isFinished() const;
	bool _isPlaying() const;

	void _doFrame();
	void _nextFrame();

	// ----------------------------------------------------------

	const Bink::HBINK m_video;
	const char *const m_openParameter;

	int               m_loopCount;
	bool              m_didFrame;
	bool              m_nextFrame;
};

// ======================================================================

#endif

