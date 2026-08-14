#ifndef INCLUDED_Headless_TextureGraphicsData_H
#define INCLUDED_Headless_TextureGraphicsData_H

#include "clientGraphics/Texture.h"

class Headless_TextureGraphicsData : public TextureGraphicsData
{
public:
	Headless_TextureGraphicsData( TextureFormat format );

	~Headless_TextureGraphicsData() {}
	
	virtual TextureFormat getNativeFormat() const;
	virtual void        lock(LockData &lockData);
	virtual void        unlock(LockData &lockData);
	virtual void copyFrom(int surfaceLevel, TextureGraphicsData const &rhs,
						  int srcX, int srcY, int srcWidth, int srcHeight,
						  int dstX, int dstY, int dstWidth, int dstHeight);

private:
	TextureFormat       m_format;
};

#endif // INCLUDED_Headless_TextureGraphicsData_H