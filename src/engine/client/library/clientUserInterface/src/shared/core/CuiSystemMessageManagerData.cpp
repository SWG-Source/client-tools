//======================================================================
//
// CuiSystemMessageManagerData.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiSystemMessageManagerData.h"

#include "UIManager.h"

//======================================================================

CuiSystemMessageManagerData::CuiSystemMessageManagerData (uint32 _flags, const Unicode::String & _msg, const Unicode::String & _oob) :
flags      (_flags),
msg        (_msg),
oob        (_oob),
translated ()
{
	UIManager::gUIManager ().CreateLocalizedString (msg, translated);
}

//======================================================================
