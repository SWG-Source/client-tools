// ======================================================================
//
// GodClientIoWin.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "GodClientIoWin.h"
#include "GodClientData.h"

// ======================================================================

GodClientIoWin::GodClientIoWin()
: IoWin ("GodClientIoWin")
{
}

//-----------------------------------------------------------------

void GodClientIoWin::draw() const
{
	IoWin::draw();
	GodClientData::getInstance().draw();
}
// ======================================================================
