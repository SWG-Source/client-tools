//===================================================================
//
// ShadowVolumeTest.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ShadowVolumeTest_H
#define INCLUDED_ShadowVolumeTest_H

//===================================================================

#include "clientGame/TestIoWin.h"

//===================================================================

class ShadowVolumeTest : public TestIoWin
{
public:

	ShadowVolumeTest ();
	virtual ~ShadowVolumeTest ();

	virtual IoResult processEvent (IoEvent* event);
	virtual void     draw () const;

private:

	ShadowVolumeTest (const ShadowVolumeTest&);
	ShadowVolumeTest& operator= (const ShadowVolumeTest&);
};

//===================================================================

#endif
