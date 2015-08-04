#ifndef __UIDIRECTSOUNDSOUNDCANVAS_H__
#define __UIDIRECTSOUNDSOUNDCANVAS_H__

#include "UITypes.h"
#include "UISoundCanvas.h"

#define DIRECTSOUND_VERSION 0x800

#include <dsound.h>

#include <map>

typedef std::map<const char *, LPDIRECTSOUNDBUFFER> UINamedSoundMap;

class UIDirectSoundSoundCanvas : public UISoundCanvas
{
public:

							 UIDirectSoundSoundCanvas( void );
	virtual			~UIDirectSoundSoundCanvas( void );

	virtual void Play( const char * );

private:

	LPDIRECTSOUND   mPlaybackDevice;
	UINamedSoundMap	mSoundMap;
};

#endif // __UIDIRECTSOUNDSOUNDCANVAS_H__