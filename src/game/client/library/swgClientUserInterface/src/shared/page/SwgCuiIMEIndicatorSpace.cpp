//======================================================================
//
// SwgCuiIMEIndicatorSpace.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiIMEIndicatorSpace.h"

#include "UIPage.h"

//======================================================================

SwgCuiIMEIndicatorSpace::SwgCuiIMEIndicatorSpace     (UIPage & page) :
CuiMediator ("SwgCuiIMEIndicatorSpace", page)
{
	setStickyVisible(true);
	setSettingsAutoSizeLocation (false, true);
}

//----------------------------------------------------------------------

SwgCuiIMEIndicatorSpace::~SwgCuiIMEIndicatorSpace ()
{
}

//======================================================================