//======================================================================
//
// SwgCuiDpsMeter.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiDpsMeter.h"

#include "clientGame/ClientRegionManager.h"
#include "clientGame/ClientTextManager.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientObject/GameCamera.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientUserInterface/CuiAction.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedGame/GroundZoneManager.h"
#include "sharedGame/PlanetMapManager.h"
#include "sharedMath/PackedRgb.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "swgClientUserInterface/SwgCuiActions.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIColorEffector.h"
#include "UIData.h"
#include "UIImage.h"
#include "UIMessage.h"
#include "UIText.h"

#include "UnicodeUtils.h"

#include <map>

//======================================================================

namespace
{
	Unicode::String s_toolTip;
}

//----------------------------------------------------------------------

SwgCuiDpsMeter::SwgCuiDpsMeter (UIPage & page) :
SwgCuiLockableMediator            ("SwgCuiDpsMeter", page),
m_textDps(0),
m_textDpsTotal(0),
m_textTaken(0),
m_textTakenTotal(0),
m_buttonReset(0)
{
	getCodeDataObject (TUIText,     m_textDps,            "textDps");
	getCodeDataObject (TUIText,     m_textDpsTotal,       "textDpsTotal");
	getCodeDataObject (TUIText,     m_textTaken,          "textTaken");
	getCodeDataObject (TUIText,     m_textTakenTotal,     "textTakenTotal");
	getCodeDataObject (TUIButton,   m_buttonReset,        "buttonReset");

	m_textDps->SetPreLocalized (true);
	m_textDpsTotal->SetPreLocalized  (true);
	m_textTaken->SetPreLocalized  (true);
	m_textTakenTotal->SetPreLocalized  (true);

	registerMediatorObject(*m_buttonReset, true);
	registerMediatorObject(page, true);

	setStickyVisible (true);
	setShowFocusedGlowRect(false);
	getPage ().SetVisible (true);
}

//----------------------------------------------------------------------

SwgCuiDpsMeter::~SwgCuiDpsMeter ()
{	
	m_textDps = 0;
	m_textDpsTotal = 0;
	m_textTaken = 0;
	m_textTakenTotal = 0;

	m_buttonReset = 0;
}

//-----------------------------------------------------------------

void SwgCuiDpsMeter::performActivate()
{
	if(!CuiPreferences::getDpsMeterEnabled() || Game::isTutorial())
		deactivate();
	else
		setIsUpdating (true);
}

//-----------------------------------------------------------------

void SwgCuiDpsMeter::performDeactivate()
{
	setIsUpdating (false);
}

//-----------------------------------------------------------------

void  SwgCuiDpsMeter::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);
	if (!getPage ().IsVisible ())
		return;

	static char buf[64];
	static const size_t buf_size = sizeof (buf);

	float damageDone, damageTaken;
	int totalDamageDone, totalDamageTaken;
	CuiCombatManager::getCurrentDps(damageDone, damageTaken, totalDamageDone, totalDamageTaken);

	snprintf (buf, buf_size, "%6.2f", damageDone);
	m_textDps->SetLocalText (Unicode::narrowToWide (buf));
	snprintf (buf, buf_size, "%6.2f", damageTaken);
	m_textTaken->SetLocalText (Unicode::narrowToWide (buf));
	snprintf (buf, buf_size, "%d", totalDamageDone);
	m_textDpsTotal->SetLocalText (Unicode::narrowToWide (buf));
	snprintf (buf, buf_size, "%d", totalDamageTaken);
	m_textTakenTotal->SetLocalText (Unicode::narrowToWide (buf));
}

//-----------------------------------------------------------------

void SwgCuiDpsMeter::OnButtonPressed    (UIWidget *context )
{
	if(context == m_buttonReset)
	{
		CuiCombatManager::resetDps();
	}
}

//======================================================================
