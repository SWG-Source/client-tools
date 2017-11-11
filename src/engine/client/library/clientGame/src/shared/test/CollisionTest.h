// ======================================================================
//
// CollisionTest.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CollisionTest_H
#define INCLUDED_CollisionTest_H

// ======================================================================

#include "sharedIoWin/IoWin.h"

class CollisionTest : public IoWin
{
public:
	CollisionTest();
	virtual ~CollisionTest();

	virtual IoResult processEvent(IoEvent *event);
	virtual void     draw(void) const;

private:
	CollisionTest(const CollisionTest &);
	CollisionTest &operator =(const CollisionTest &);
};

// ======================================================================

#endif
