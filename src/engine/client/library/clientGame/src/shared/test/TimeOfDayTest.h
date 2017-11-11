//===================================================================
//
// TimeOfDayTest.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_TimeOfDayTest_H
#define INCLUDED_TimeOfDayTest_H

//===================================================================

#include "clientGame/TestIoWin.h"

//===================================================================

class TimeOfDayTest : public TestIoWin
{
public:

	TimeOfDayTest ();
	virtual ~TimeOfDayTest ();

	virtual IoResult processEvent (IoEvent* event);
	virtual void     draw () const;

private:

	virtual void     createFloor ();

private:

	TimeOfDayTest (const TimeOfDayTest&);
	TimeOfDayTest& operator= (const TimeOfDayTest&);
};

//===================================================================

#endif
