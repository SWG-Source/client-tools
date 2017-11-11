// ======================================================================
//
// Video.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Video_H
#define INCLUDED_Video_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/PersistentCrcString.h"

// ======================================================================

class Video
{
public:

	unsigned      getUniqueID() const { return m_uniqueID; }
	const char   *getName() const;
	void          fetch() const;
	void          release() const;

	virtual int   getWidth() const=0;
	virtual int   getHeight() const=0;

	virtual int   getLoopCount() const=0;
	bool          isFinished()                 const { return getLoopCount()>0; }
	bool          getLooping()                 const { return m_looping; }
	virtual bool  setLooping(bool shouldVideoLoop);

	virtual bool  canStretchBlt() const;

	// ----------------------------------------------------------
	// This function allows you to pause the video's playback. 
	// Pass in true to pause the movie, and a false to resume the 
	// movie after a pause. 
	// Returns: true if video is paused, false if it is playing.
	virtual bool pause(bool enable);
	// ----------------------------------------------------------

	// ----------------------------------------------------------
	// This function allows you to shut off the video decompression 
	// of a playing video. This is useful when you want to only 
	// play the audio track only. 
	// Returns: true if video is on, false if video is off.
	virtual bool setVideoOnOff(bool on);
	// ----------------------------------------------------------

	// ----------------------------------------------------------
	// This function allows you to shut off the audio playback of 
	// this video. This is useful if you want to stop all sound 
	// completely, or if you need to release the digital sound 
	// resource handles to play a wave or AVI file. 
	virtual bool setSoundOnOff(bool on);
	// ----------------------------------------------------------

	// ----------------------------------------------------------
	virtual void service();
	virtual bool performDrawing(int screenX, int screenY, int screenCX, int screenCY);
	virtual bool performBlitting(int screenX, int screenY);
	// ----------------------------------------------------------

	// ----------------------------------------------------------
	virtual void setVolume(unsigned trackID, int volume);
	// ----------------------------------------------------------

private:
	
	Video();                           /// Disabled.
	Video(const Video &);              /// Disabled.
	Video &operator =(const Video &);  /// Disabled.

protected:

	Video(const char *name);
	virtual ~Video();

	const unsigned m_uniqueID;
	mutable int    m_users;
	char          *m_name;
	bool           m_looping;
};

// ======================================================================

inline const char *Video::getName() const
{
	return m_name;
}

// ======================================================================

#endif

