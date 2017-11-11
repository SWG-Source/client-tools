// ======================================================================
//
// VideoPlaybackManager.cpp
//
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/VideoPlaybackManager.h"
#include "clientGraphics/Video.h"

#include <map>

// ======================================================================

namespace VideoPlaybackManagerNamespace
{
	struct VideoInfo
	{
		void init(Video &i_video)
		{
			m_video=&i_video;
			m_screenX=m_screenY=0;
			m_screenCX=m_screenCY=-1;
		}

		Video *m_video;
		int    m_screenX;
		int    m_screenY;
		int    m_screenCX;
		int    m_screenCY;
	};

	typedef std::map<unsigned, VideoInfo> VideoList;
	static VideoList s_videos;

	static VideoInfo *findVideo(unsigned uid)
	{
		VideoList::iterator vi = s_videos.find(uid);
		if (vi!=s_videos.end())
		{
			return &vi->second;
		}
		else
		{
			return 0;
		}
	}

	static inline VideoInfo *findVideo(Video &v)
	{
		return findVideo(v.getUniqueID());
	}
}
using namespace VideoPlaybackManagerNamespace;

// ======================================================================

bool VideoPlaybackManager::insert(Video &i_video, bool /*startNow*/, bool /*loop*/)
{
	unsigned uid = i_video.getUniqueID();
	VideoList::iterator vi = s_videos.lower_bound(uid);
	const bool exists = vi!=s_videos.end() && s_videos.key_comp()(uid, vi->first);
	if (exists)
	{
		return false;
	}

	i_video.fetch();

	vi = s_videos.insert(vi, VideoList::value_type(uid, VideoInfo()));
	vi->second.init(i_video);

	return true;
}

// ======================================================================

void VideoPlaybackManager::remove(Video &i_video)
{
	unsigned uid = i_video.getUniqueID();
	VideoList::iterator vi = s_videos.find(uid);
	if (vi==s_videos.end())
	{
		return;
	}
	vi->second.m_video->release();
	s_videos.erase(vi);
}

// ======================================================================

bool VideoPlaybackManager::has(Video &i_video)
{
	unsigned uid = i_video.getUniqueID();
	VideoList::iterator vi = s_videos.find(uid);
	return vi!=s_videos.end();
}

// ======================================================================

bool VideoPlaybackManager::setPosition(Video &i_video, int screenX, int screenY)
{
	VideoInfo *info = findVideo(i_video);
	if (!info)
	{
		return false;
	}
	info->m_screenX=screenX;
	info->m_screenY=screenY;

	return true;
}

// ======================================================================

bool VideoPlaybackManager::setPosition(Video &i_video, int screenX, int screenY, int screenCX, int screenCY)
{
	VideoInfo *info = findVideo(i_video);
	if (!info)
	{
		return false;
	}
	info->m_screenX=screenX;
	info->m_screenY=screenY;
	info->m_screenCX=screenCX;
	info->m_screenCY=screenCY;

	return true;
}

// ======================================================================

void VideoPlaybackManager::service()
{
	for (VideoList::iterator vi=s_videos.begin();vi!=s_videos.end();++vi)
	{
		const VideoInfo &info = vi->second;
		info.m_video->service();
	}
}

// ======================================================================

void VideoPlaybackManager::performDrawing()
{
	for (VideoList::iterator vi=s_videos.begin();vi!=s_videos.end();++vi)
	{
		const VideoInfo &info = vi->second;
		info.m_video->performDrawing(info.m_screenX, info.m_screenY, info.m_screenCX, info.m_screenCY);
	}
}

// ======================================================================

void VideoPlaybackManager::performBlitting()
{
	for (VideoList::iterator vi=s_videos.begin();vi!=s_videos.end();++vi)
	{
		const VideoInfo &info = vi->second;
		info.m_video->performBlitting(info.m_screenX, info.m_screenY);
	}
}

// ======================================================================
