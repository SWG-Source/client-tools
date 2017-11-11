//======================================================================
//
// SwgCuiIMEIndicator.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiIMEIndicator.h"

#include "UIPage.h"

//======================================================================

SwgCuiIMEIndicator::SwgCuiIMEIndicator     (UIPage & page) :
CuiMediator ("SwgCuiIMEIndicator", page)
{
	setStickyVisible(true);
	setSettingsAutoSizeLocation (false, true);
}

//----------------------------------------------------------------------

SwgCuiIMEIndicator::~SwgCuiIMEIndicator ()
{
}

//======================================================================