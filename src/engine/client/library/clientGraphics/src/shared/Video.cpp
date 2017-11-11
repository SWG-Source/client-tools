// ======================================================================
//
// Video.cpp
//
// Copyright 2001 - 2003 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/Video.h"

#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedSynchronization/RecursiveMutex.h"
#include "clientGraphics/VideoList.h"

// ======================================================================

namespace VideoNamespace
{
	unsigned s_nextUniqueID;
}
using namespace VideoNamespace;

// ======================================================================

namespace VideoListNamespace
{
	extern void _onVideoDelete(const Video *video);
	extern RecursiveMutex s_criticalSection;
}

// ======================================================================

Video::Video(const char *name)
:	m_uniqueID(s_nextUniqueID++),
	m_users(0),
	m_name(DuplicateString(name)),
	m_looping(false)
{
}

// ----------------------------------------------------------------------

Video::~Video()
{
	DEBUG_FATAL(m_users < 0, ("Negative user count"));
	delete [] m_name;
}

// ----------------------------------------------------------------------

void Video::fetch() const
{
	VideoListNamespace::s_criticalSection.enter();
		++m_users;
	VideoListNamespace::s_criticalSection.leave();
}

// ----------------------------------------------------------------------

void Video::release() const
{
	VideoListNamespace::s_criticalSection.enter();

		if (--m_users <= 0)
		{
			VideoListNamespace::_onVideoDelete(this);
			delete this; //lint !e605 // Increase in pointer capability
		}

	VideoListNamespace::s_criticalSection.leave();
}

// ----------------------------------------------------------------------

bool Video::pause(bool)
{ 
	return false; 
}

// ----------------------------------------------------------------------

bool Video::setVideoOnOff(bool)
{
	return true;
}

// ----------------------------------------------------------------------

bool Video::setSoundOnOff(bool)
{
	return true;
}

// ----------------------------------------------------------------------

void Video::service()
{
}

// ----------------------------------------------------------------------

bool Video::performDrawing(int, int, int, int)
{
	return false;
}

// ----------------------------------------------------------------------

bool Video::performBlitting(int, int)
{
	return false;
}

// ----------------------------------------------------------------------

void Video::setVolume(unsigned, int)
{
}

// ----------------------------------------------------------------------

bool Video::setLooping(bool shouldVideoLoop)
{
	m_looping=shouldVideoLoop;
	return m_looping;
}

// ----------------------------------------------------------------------

bool Video::canStretchBlt() const
{
	return false;
}

// ======================================================================
