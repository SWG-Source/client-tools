//===================================================================
//
// JgrillsTest.h
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_JgrillsTest_H
#define INCLUDED_JgrillsTest_H

//===================================================================

class Light;

#include "clientGame/TestIoWin.h"

//===================================================================

class JgrillsTest : public TestIoWin
{
public:

	JgrillsTest();
	virtual ~JgrillsTest();

	virtual IoResult processEvent(IoEvent* event);
	virtual void     draw() const;

protected:

	virtual Object* createObjectAndAppearance();
	virtual void createLights();
	virtual void createFloor();

protected:

	JgrillsTest(const JgrillsTest&);
	JgrillsTest& operator=(const JgrillsTest&);
};

//===================================================================

#endif
