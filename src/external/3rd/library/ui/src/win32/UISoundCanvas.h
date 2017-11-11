#ifndef __UISOUNDCANVAS_H__
#define __UISOUNDCANVAS_H__

class UISoundCanvas
{
public:

							 UISoundCanvas( void ){};
	virtual     ~UISoundCanvas( void ) = 0 {};

	virtual void Play                     (const char *SoundFile ) = 0;
	virtual void PlaySoundGenericNegative () const {}
};

#endif // __UISOUNDCANVAS_H__