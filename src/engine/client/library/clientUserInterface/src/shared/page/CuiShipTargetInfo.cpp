//======================================================================
//
// CuiShipTargetInfo.cpp
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiShipTargetInfo.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/ShipObject.h"
#include "sharedGame/ShipChassisSlotType.h"

#include "UIComposite.h"
#include "UIText.h"

//======================================================================

char CuiShipTargetInfo::ms_textBuffer[ CuiShipInfo::TextBufferLength ] = { 0 };

//======================================================================

CuiShipTargetInfo::CuiShipTargetInfo(UIPage& page) :
CuiShipInfo("CuiShipTargetInfo", page),
m_textTargetedComponent(NULL)
{
	UIText * textSample = 0;
	getCodeDataObject(TUIText, textSample,       "textSample");

	m_textTargetedComponent = safe_cast<UIText *>(textSample->DuplicateObject());
	m_textTargetedComponent->SetPreLocalized(true);
	m_textTargetedComponent->SetVisible(true);
	m_comp->AddChild(m_textTargetedComponent);
}

//----------------------------------------------------------------------

void CuiShipTargetInfo::update(const float timeElapsedSecs)
{
	UNREF(timeElapsedSecs);

	CreatureObject const * const player = Game::getPlayerCreature();
	if(!player)
		return;

	Object const * const lookAtObject = player ? player->getLookAtTarget().getObject() : NULL;
	ClientObject const * const targetClientObject = lookAtObject ? lookAtObject->asClientObject() : NULL;
	ShipObject const * const targetShipObject = targetClientObject ? targetClientObject->asShipObject() : NULL;

	if (targetShipObject && targetShipObject->getPilot())
	{
		Unicode::String const & localizedName = targetShipObject->getPilot()->getLocalizedName();
		Unicode::NarrowString const narrowLocalizedName = Unicode::wideToNarrow(localizedName);
		setTitle("Target Ship Info", narrowLocalizedName.c_str());
	}
	else
	{
		setTitle("Target Ship Info", NULL);
	}

	setCommonShipInfo( targetShipObject );

	ShipObject * const playerShip = Game::getPlayerPilotedShip();
	if(playerShip)
	{
		ShipChassisSlotType::Type const slot = player->getLookAtTargetSlot();
		std::string const & slotName = ShipChassisSlotType::getNameFromType (slot);
		StringId sid("ship_slot_n", slotName);
		Unicode::String const & text = sid.localize();
		snprintf(ms_textBuffer, TextBufferLength, "Targeted Component: %s", Unicode::wideToNarrow(text).c_str());
		m_textTargetedComponent->SetLocalText(Unicode::narrowToWide(ms_textBuffer));
	}
}

//======================================================================
