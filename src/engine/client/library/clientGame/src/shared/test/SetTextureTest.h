//===================================================================
//
// SetTextureTest.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_SetTextureTest_H
#define INCLUDED_SetTextureTest_H

//===================================================================

#include "clientGame/TestIoWin.h"

class Texture;

//===================================================================

class SetTextureTest : public TestIoWin
{
public:

	SetTextureTest ();
	virtual ~SetTextureTest ();

	virtual IoResult processEvent (IoEvent* event);
	virtual void     initialize ();
	virtual void     createFloor ();

private:

	SetTextureTest (const SetTextureTest&);
	SetTextureTest& operator= (const SetTextureTest&);

private:

	typedef stdvector<const Texture*>::fwd TextureList2;
	TextureList2* const m_textureList;
};

//===================================================================

#endif
