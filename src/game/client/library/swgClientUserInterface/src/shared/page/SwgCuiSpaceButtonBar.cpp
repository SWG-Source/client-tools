//=============================================================================
//
// SwgCuiSpaceButtonBar.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//=============================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiSpaceButtonBar.h"

//=============================================================================

namespace SwgCuiSpaceButtonBarNamespace
{
}

using namespace SwgCuiSpaceButtonBarNamespace;

//=============================================================================

SwgCuiSpaceButtonBar::SwgCuiSpaceButtonBar(UIPage & page) :
CuiMediator("SpaceButtonBar", page),
UIEventCallback()
{
	getPage().SetVisible(true);
}

//-----------------------------------------------------------------------------

SwgCuiSpaceButtonBar::~SwgCuiSpaceButtonBar()
{
}

//=============================================================================
