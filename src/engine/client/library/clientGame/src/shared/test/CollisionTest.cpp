// ======================================================================
//
// CollisionTest.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================




// This test application is OBSOLETE and will not work anymore.





#include "clientGame/FirstClientGame.h"
#include "clientGame/CollisionTest.h"


#include "clientGame/Game.h"

// ======================================================================

CollisionTest::CollisionTest() : IoWin("CollisionTest")
{
}

// ----------------------------------------------------------------------

CollisionTest::~CollisionTest()
{
}

// ----------------------------------------------------------------------

IoResult CollisionTest::processEvent(IoEvent *event)
{
	if (event->type == IOET_Character && tolower(event->arg2) == 'q')
	{
		Game::quit();
		return IOR_BlockKillMe;
	}

	return IOR_Block;
}

// ----------------------------------------------------------------------

void CollisionTest::draw() const
{
}

// ======================================================================
