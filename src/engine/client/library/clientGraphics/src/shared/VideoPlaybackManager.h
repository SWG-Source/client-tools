// ======================================================================
//
// VideoPlaybackManager.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_VideoPlaybackManager_H
#define INCLUDED_VideoPlaybackManager_H

class Video;

class VideoPlaybackManager
{
public:
	static bool insert(Video &i_video, bool startNow=true, bool loop=false);
	static void remove(Video &i_video);
	static bool has(Video &i_video);

	static bool setPosition(Video &i_video, int screenX, int screenY);
	static bool setPosition(Video &i_video, int screenX, int screenY, int screenCX, int screenCY);

	static void service();
	static void performDrawing();
	static void performBlitting();
};

#endif
