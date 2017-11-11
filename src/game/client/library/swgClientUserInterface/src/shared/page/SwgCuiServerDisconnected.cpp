//======================================================================
//
// SwgCuiServerDisconnected.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiServerDisconnected.h"

#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "clientUserInterface/CuiSharedPageManager.h"
#include "UIPage.h"
//======================================================================

SwgCuiServerDisconnected::SwgCuiServerDisconnected     (UIPage & page) :
CuiMediator ("SwgCuiServerDisconnected", page)
{
	UIPage * const parent = safe_cast<UIPage *>(getPage ().GetParent ());
	NOT_NULL (parent);
	CuiSharedPageManager::registerMediatorType (parent->GetName (), CuiMediatorTypes::ServerDisconnected);
}

//----------------------------------------------------------------------

SwgCuiServerDisconnected::~SwgCuiServerDisconnected ()
{
}

//----------------------------------------------------------------------

void SwgCuiServerDisconnected::performActivate   ()
{
	UIPage * const parent = safe_cast<UIPage *>(getPage ().GetParent ());
	if (parent)
		CuiSharedPageManager::checkPageVisibility (parent->GetName (), *parent, &getPage (), CuiMediatorTypes::ServerDisconnected, true);
}

//----------------------------------------------------------------------

void SwgCuiServerDisconnected::performDeactivate ()
{
	UIPage * const parent = safe_cast<UIPage *>(getPage ().GetParent ());
	if (parent)
		CuiSharedPageManager::checkPageVisibility (parent->GetName (), *parent, &getPage (), CuiMediatorTypes::ServerDisconnected, false);
}

//======================================================================
