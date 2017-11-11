#include "FirstUiBuilder.h"
#include "UIDirectSoundSoundCanvas.h"

UIDirectSoundSoundCanvas::UIDirectSoundSoundCanvas()
{
	HRESULT hr;

	mPlaybackDevice = 0;

	hr = DirectSoundCreate( 0, &mPlaybackDevice, 0 );
}

UIDirectSoundSoundCanvas::~UIDirectSoundSoundCanvas()
{
	for( UINamedSoundMap::iterator i = mSoundMap.begin(); i != mSoundMap.end(); ++i )
	{
		delete const_cast<char *>( i->first );
		i->second->Release();
	}
	mSoundMap.clear();

	if( mPlaybackDevice )
		mPlaybackDevice->Release();
}

void UIDirectSoundSoundCanvas::Play( const char *FileToPlay )
{
	static_cast<void> (FileToPlay);

	/*
	HRESULT hr;

	if( mPlaybackDevice )
	{
		UINamedSoundMap::iterator CachedSound = mSoundMap.find( FileToPlay );
		IDirectSoundBuffer			 *theSound;

		if( CachedSound == mSoundMap.end() )
		{
			WAVEFORMATEX WaveFormat;
			DSBUFFERDESC dsbd;

			dsbd.dwSize						= sizeof( dsbd );
			dsbd.dwFlags					= 0;
			dsbd.dwBufferBytes		= 1000;
			dsbd.dwReserved				= 0;
			dsbd.lpwfxFormat			= &WaveFormat;
			dsbd.guid3DAlgorithm	= GUID_NULL;

			hr = mPlaybackDevice->CreateSoundBuffer( &dsbd, &theSound, 0 );
		}
		else
			theSound = CachedSound->second;

		theSound->SetCurrentPosition( 0 );
		theSound->Play( 0, 0, 0 );
	}
	*/
}