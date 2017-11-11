// ======================================================================
//
// VideoList.h
//
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_VideoList_H
#define INCLUDED_VideoList_H

// ======================================================================

class Video;
class StringCompare;
class RecursiveMutex;

// ======================================================================

class VideoList
{
public:

	// ----------------------------------------------------------
	// If hMilesDigitalDriver is NULL then no sound will accompany
	// play-back
	static void install(void *hMilesDigitalDriver);
	
	static Video *fetch(const char *name);
};

// ======================================================================

#endif
