#include "FirstHeadless.H"
#include "Headless_TextureGraphicsData.h"


Headless_TextureGraphicsData::Headless_TextureGraphicsData( TextureFormat format ) :
m_format( format )
{
}

TextureFormat Headless_TextureGraphicsData::getNativeFormat() const
{
	return m_format;
}

//	virtual void        lock(LockData &lockData);

void Headless_TextureGraphicsData::lock( LockData &lockData )
{
	lockData.m_pixelData = new int[ lockData.m_width * lockData.m_height * sizeof( uint32 ) ];
	lockData.m_pitch     = lockData.m_width * sizeof( uint32 );
}

void Headless_TextureGraphicsData::unlock( LockData &lockData )
{
	delete [] (int*)lockData.m_pixelData;
}
