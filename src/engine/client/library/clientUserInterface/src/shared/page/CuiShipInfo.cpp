//======================================================================
//
// CuiShipInfo.cpp
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "CuiShipInfo.h"

#include "UIComposite.h"
#include "UIText.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/ShipObject.h"
#include "sharedGame/ShipChassisSlotType.h"
#include "sharedGame/ShipComponentWeaponManager.h"

//======================================================================

//----------------------------------------------------------------------

char CuiShipInfo::ms_textBuffer[ CuiShipInfo::TextBufferLength ] = { 0 };

//----------------------------------------------------------------------

CuiShipInfo::CuiShipInfo(const char * const mediatorDebugName, UIPage & page) :
CuiMediator(mediatorDebugName, page),
m_comp(NULL),
m_textHitpoints(NULL),
m_textArmorFront(NULL),
m_textArmorBack(NULL),
m_textShieldFront(NULL),
m_textShieldBack(NULL),
m_textTitle(NULL),
m_textCapacitor(NULL),
m_textAmmo(NULL),
m_textRefire(NULL),
m_textTargetAcquired(NULL),
m_lastTextRefireLength(-1),
m_lastTextAmmoLength(-1)
{
	getCodeDataObject(TUIComposite, m_comp,      "comp");

	UIText * textSample = 0;
	getCodeDataObject(TUIText, textSample,       "textSample");

	m_textTitle = safe_cast<UIText *>(textSample->DuplicateObject());
	m_textHitpoints = safe_cast<UIText *>(textSample->DuplicateObject());
	m_textArmorFront = safe_cast<UIText *>(textSample->DuplicateObject());
	m_textArmorBack = safe_cast<UIText *>(textSample->DuplicateObject());
	m_textShieldFront = safe_cast<UIText *>(textSample->DuplicateObject());
	m_textShieldBack = safe_cast<UIText *>(textSample->DuplicateObject());
	m_textCapacitor = safe_cast<UIText *>(textSample->DuplicateObject());
	m_textAmmo = safe_cast<UIText *>(textSample->DuplicateObject());
	m_textRefire = safe_cast<UIText *>(textSample->DuplicateObject());
	m_textTargetAcquired = safe_cast<UIText *>(textSample->DuplicateObject());

	m_textTitle->SetPreLocalized(true);
	m_textHitpoints->SetPreLocalized(true);
	m_textArmorFront->SetPreLocalized(true);
	m_textArmorBack->SetPreLocalized(true);
	m_textShieldFront->SetPreLocalized(true);
	m_textShieldBack->SetPreLocalized(true);
	m_textCapacitor->SetPreLocalized(true);
	m_textAmmo->SetPreLocalized(true);
	m_textRefire->SetPreLocalized(true);
	m_textTargetAcquired->SetPreLocalized(true);

	m_textTitle->SetVisible(true);
	m_textHitpoints->SetVisible(true);
	m_textArmorFront->SetVisible(true);
	m_textArmorBack->SetVisible(true);
	m_textShieldFront->SetVisible(true);
	m_textShieldBack->SetVisible(true);
	m_textCapacitor->SetVisible(true);
	m_textAmmo->SetVisible(true);
	m_textRefire->SetVisible(true);
	m_textTargetAcquired->SetVisible(true);

	m_comp->AddChild(m_textTitle);
	m_comp->AddChild(m_textHitpoints);
	m_comp->AddChild(m_textArmorFront);
	m_comp->AddChild(m_textArmorBack);
	m_comp->AddChild(m_textShieldFront);
	m_comp->AddChild(m_textShieldBack);
	m_comp->AddChild(m_textCapacitor);
	m_comp->AddChild(m_textTargetAcquired);
	m_comp->AddChild(m_textAmmo);
	m_comp->AddChild(m_textRefire);

	textSample->SetVisible(false);

	setStickyVisible (true);
}

//----------------------------------------------------------------------

CuiShipInfo::~CuiShipInfo()
{
	m_comp             = NULL;
	m_textTitle        = NULL;
	m_textHitpoints    = NULL;
	m_textArmorFront   = NULL;
	m_textArmorBack    = NULL;
	m_textShieldFront  = NULL;
	m_textShieldBack   = NULL;
	m_textCapacitor    = NULL;
	m_textAmmo         = NULL;
}

//----------------------------------------------------------------------

void CuiShipInfo::performActivate()
{
	NOT_NULL(m_comp);

	m_comp->Pack();
	setIsUpdating (true);
}

//----------------------------------------------------------------------

void CuiShipInfo::performDeactivate()
{
	setIsUpdating (false);
}

//----------------------------------------------------------------------

void CuiShipInfo::setHitPoints(const float currentHP, const float maxHP)
{
	NOT_NULL(m_textHitpoints);

	snprintf(ms_textBuffer, TextBufferLength, "HP:           %7.2f / %7.2f", currentHP, maxHP);
	m_textHitpoints->SetLocalText(Unicode::narrowToWide(ms_textBuffer));
}

//----------------------------------------------------------------------

void CuiShipInfo::setArmorFront(const float currentArmor, const float maxArmor)
{
	NOT_NULL(m_textArmorFront);

	snprintf(ms_textBuffer, TextBufferLength, "ARMOR front:  %7.2f / %7.2f", currentArmor, maxArmor);
	m_textArmorFront->SetLocalText(Unicode::narrowToWide(ms_textBuffer));
}

//----------------------------------------------------------------------

void CuiShipInfo::setArmorBack(const float currentArmor, const float maxArmor)
{
	NOT_NULL(m_textArmorBack);

	snprintf(ms_textBuffer, TextBufferLength, "      back:   %7.2f / %7.2f", currentArmor, maxArmor);
	m_textArmorBack->SetLocalText(Unicode::narrowToWide(ms_textBuffer));
}

//----------------------------------------------------------------------

void CuiShipInfo::setShieldFront(const float currentShield, const float maxShield)
{
	NOT_NULL(m_textShieldFront);

	snprintf(ms_textBuffer, TextBufferLength, "SHIELD front: %7.2f / %7.2f", currentShield, maxShield);
	m_textShieldFront->SetLocalText(Unicode::narrowToWide(ms_textBuffer));
}

//----------------------------------------------------------------------

void CuiShipInfo::setShieldBack(const float currentShield, const float maxShield)
{
	NOT_NULL(m_textHitpoints);

	snprintf(ms_textBuffer, TextBufferLength, "       back:  %7.2f / %7.2f", currentShield, maxShield);
	m_textShieldBack->SetLocalText(Unicode::narrowToWide(ms_textBuffer));
}

//----------------------------------------------------------------------

void CuiShipInfo::setCapacitor(const float currentCapacitor, const float maxCapacitor)
{
	NOT_NULL(m_textHitpoints);

	snprintf(ms_textBuffer, TextBufferLength, "CAPACITOR:    %7.2f / %7.2f", currentCapacitor, maxCapacitor);
	m_textCapacitor->SetLocalText(Unicode::narrowToWide(ms_textBuffer));
}


//----------------------------------------------------------------------

bool CuiShipInfo::setAmmo(ShipObject const & shipObject)
{
	static std::string strbuf;
	strbuf.clear();

	int numLines = 0;

	for (int i = static_cast<int>(ShipChassisSlotType::SCST_weapon_first); i < static_cast<int>(ShipChassisSlotType::SCST_weapon_last); ++i)
	{
		uint32 const componentCrc = shipObject.getComponentCrc(i);
		if (componentCrc != 0 && ShipComponentWeaponManager::isAmmoConsuming(componentCrc))
		{
			m_textAmmo->SetVisible(true);
			int const ammoCurrent = shipObject.getWeaponAmmoCurrent(i);
			int const ammoMaximum = shipObject.getWeaponAmmoMaximum(i);

			int const weaponIndex = i - static_cast<int>(ShipChassisSlotType::SCST_weapon_first);

			snprintf(ms_textBuffer, TextBufferLength, "AMMO [%2d]:         %3d / %3d", weaponIndex, ammoCurrent, ammoMaximum);

			if (numLines > 0)
				strbuf.push_back('\n');
			strbuf += ms_textBuffer;

			++numLines;
		}
	}

	bool const retval = numLines != m_lastTextAmmoLength;
	m_lastTextAmmoLength = numLines;

	m_textAmmo->SetLocalText(Unicode::narrowToWide(strbuf));
	m_textAmmo->SetVisible(m_lastTextAmmoLength > 0);

	return retval;
}

//----------------------------------------------------------------------

bool CuiShipInfo::setRefireTimer(ShipObject const & shipObject)
{
	static std::string strbuf;
	strbuf.clear();

	int numLines = 0;

	for (int i = 0; i < ShipChassisSlotType::cms_numWeaponIndices; ++i)
	{
		int const chassisSlot = ShipChassisSlotType::SCST_weapon_first + i;

		if (shipObject.isSlotInstalled(chassisSlot))
		{
			float timeRemaining = 0.0f;
			float expireTime = 0.0f;

			shipObject.getWeaponRefireTimeRemaining(i, timeRemaining, expireTime);

			timeRemaining = std::max (0.0f, timeRemaining);

			if (expireTime > 0.0f)
			{
				float const ratio = clamp(0.0f, timeRemaining / expireTime, 1.0f);
				int tenScaleRatio = static_cast<int>(ratio * 10.0f);

				snprintf(ms_textBuffer, TextBufferLength, "Refire: |%*s*%*s| %5.1f", tenScaleRatio, "", 10 - tenScaleRatio, "", timeRemaining);
				if (numLines > 0)
					strbuf.push_back('\n');
				strbuf += ms_textBuffer;
				++numLines;
			}
		}
	}

	bool const retval = numLines != m_lastTextRefireLength;
	m_lastTextRefireLength = numLines;
		
	m_textRefire->SetLocalText(Unicode::narrowToWide(strbuf));
	m_textRefire->SetVisible(m_lastTextRefireLength > 0);

	return retval;
}

//----------------------------------------------------------------------

void CuiShipInfo::setMissileTargetAcquired(ShipObject const & shipObject)
{
	int threatCount = 0;
	{
		ShipObject::ShipVector const & shipsTargetingPlayer = ShipObject::getShipsTargetingPlayer();
		for (ShipObject::ShipVector::const_iterator it = shipsTargetingPlayer.begin(); it != shipsTargetingPlayer.end(); ++it)
		{
			ShipObject const * const shipTargetingPlayer = NON_NULL(*it);
			if (shipTargetingPlayer->hasMissileTargetAcquisition())
				++threatCount;
		}
	}

	bool const acquired = shipObject.hasMissileTargetAcquisition();
	float const timeRemaining = shipObject.getTimeUntilMissileTargetAcquisition();
	snprintf(ms_textBuffer, TextBufferLength, "ACQUIRE: -%c- %5.1f,  THRT: {%d}", acquired ? 'X' : 'o', timeRemaining, threatCount);

	m_textTargetAcquired->SetLocalText(Unicode::narrowToWide(ms_textBuffer));
}

//----------------------------------------------------------------------

void CuiShipInfo::setTitle(const char * const dialogTitle, const char * const pilot)
{
	NOT_NULL(m_textHitpoints);
	NOT_NULL(dialogTitle);

	if(pilot)
	{
		snprintf(ms_textBuffer, TextBufferLength, "%s: %s", dialogTitle, pilot);
		m_textTitle->SetLocalText(Unicode::narrowToWide(ms_textBuffer));
	}
	else
	{
		m_textTitle->SetLocalText(Unicode::narrowToWide(dialogTitle));
	}
}

//----------------------------------------------------------------------

void CuiShipInfo::setVisible(bool isVisible)
{
	NOT_NULL(m_comp);
	m_comp->SetVisible(isVisible);
}

//----------------------------------------------------------------------

void CuiShipInfo::setCommonShipInfo(ShipObject const * const shipObject)
{
	if(shipObject == NULL)
	{
		setVisible(false);
		return;
	}

	setVisible(true);

	const float hpCurrent = shipObject->getCurrentChassisHitPoints();
	const float hpMax     = shipObject->getMaximumChassisHitPoints();
	setHitPoints(hpCurrent, hpMax);
	
	const float armorFrontCurrent = shipObject->getComponentArmorHitpointsCurrent(ShipChassisSlotType::SCST_armor_0);
	const float armorFrontMax     = shipObject->getComponentArmorHitpointsMaximum(ShipChassisSlotType::SCST_armor_0);
	setArmorFront(armorFrontCurrent, armorFrontMax);
	
	const float armorBackCurrent = shipObject->getComponentArmorHitpointsCurrent(ShipChassisSlotType::SCST_armor_1);
	const float armorBackMax     = shipObject->getComponentArmorHitpointsMaximum(ShipChassisSlotType::SCST_armor_1);
	setArmorBack(armorBackCurrent, armorBackMax);

	//-- certain values are not synchronized down to all clients, ignore them
	if (static_cast<Object const *>(shipObject->getPilot()) != Game::getPlayer())
	{
		m_textShieldFront->SetVisible(false);
		m_textShieldBack->SetVisible(false);
		m_textCapacitor->SetVisible(false);
		m_textAmmo->SetVisible(false);
		m_textRefire->SetVisible(false);
		m_textTargetAcquired->SetVisible(false);
		return;
	}
	
	m_textShieldFront->SetVisible(true);
	m_textShieldBack->SetVisible(true);
	m_textCapacitor->SetVisible(true);
		
	const float shieldFrontCurrent = shipObject->getShieldHitpointsFrontCurrent();
	const float shieldFrontMax     = shipObject->getShieldHitpointsFrontMaximum();
	setShieldFront(shieldFrontCurrent, shieldFrontMax);
	
	const float shieldBackCurrent = shipObject->getShieldHitpointsBackCurrent();
	const float shieldBackMax     = shipObject->getShieldHitpointsBackMaximum();
	setShieldBack(shieldBackCurrent, shieldBackMax);
	
	const float capacitorCurrent = shipObject->getCapacitorEnergyCurrent();
	const float capacitorMaximum = shipObject->getCapacitorEnergyMaximum();
	setCapacitor(capacitorCurrent, capacitorMaximum);
	
	bool needsRepack = false;

	needsRepack = setAmmo(*shipObject) || needsRepack;
	needsRepack = setRefireTimer(*shipObject) || needsRepack;
	setMissileTargetAcquired(*shipObject);

	if (needsRepack)
		m_comp->Pack();
}


//======================================================================
