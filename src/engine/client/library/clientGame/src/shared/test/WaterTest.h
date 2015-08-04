//===================================================================
//
// WaterTest.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_WaterTest_H
#define INCLUDED_WaterTest_H

//===================================================================

#include "clientGame/TestIoWin.h"

//===================================================================

class WaterTest : public TestIoWin
{
public:

	WaterTest ();
	virtual ~WaterTest ();

private:

	virtual void    createFloor ();
	virtual Object* createObjectAndAppearance ();

private:

	WaterTest (const WaterTest&);
	WaterTest& operator= (const WaterTest&);
};

//===================================================================

#endif
