//=============================================================================
//
// SwgCuiSpaceMissileCount.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//=============================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiSpaceMissileCount.h"

#include "sharedFoundation/FormattedString.h"
#include "clientGame/Game.h"
#include "clientGame/ShipObject.h"
#include "sharedGame/ShipChassisSlotType.h"
#include "sharedGame/ShipComponentWeaponManager.h"

#include "UIText.h"

//=============================================================================

namespace SwgCuiSpaceMissileCountNamespace
{
	char ms_buffer[10];
	int ms_currentAmmo = 0;

	bool setupAmmoConsumingSlot(ShipObject const & shipObject, UIText & name, UIText & value, ShipChassisSlotType::Type const slotType)
	{
		if(slotType != ShipChassisSlotType::SCST_invalid)
		{
			name.SetVisible(true);
			value.SetVisible(true);
			ms_currentAmmo = shipObject.getWeaponAmmoCurrent(slotType);
			_itoa(ms_currentAmmo, ms_buffer, 10);
			value.SetLocalText(Unicode::narrowToWide(ms_buffer));
			return true;
		}
		else
		{
			name.SetVisible(false);
			value.SetVisible(false);
			return false;
		}
	}
}

using namespace SwgCuiSpaceMissileCountNamespace;

//=============================================================================

SwgCuiSpaceMissileCount::SwgCuiSpaceMissileCount(UIPage & page) :
	SwgCuiLockableMediator("MissileCount", page)
{
	for(int i = 0; i < c_maxNumMissileSlots; ++i)
	{
		//Not strictly necessary to null these out as getCodeDataObject should write them anyhow
		m_missileNames[i] = NULL;
		m_missileValues[i] = NULL;
		
		//The UI elements are numbered starting from 1
		FormattedString<36> formater;
		getCodeDataObject(TUIText, m_missileNames[i], formater.sprintf("namemissilelaunch%d", i+1));
		getCodeDataObject(TUIText, m_missileValues[i], formater.sprintf("valuem%d", i+1));
		
		m_missileNames[i]->SetVisible(false);
		m_missileValues[i]->SetVisible(false);
		m_missileValues[i]->SetPreLocalized(true);
	}

	for(int i = 0; i < c_maxNumCounterMeasureSlots; ++i)
	{
		//Not strictly necessary to null these out as getCodeDataObject should write them anyhow
		m_counterMeasureNames[i] = NULL;
		m_counterMeasureValues[i] = NULL;

		//The UI elements are numbered starting from 1
		FormattedString<36> formater;
		getCodeDataObject(TUIText, m_counterMeasureNames[i], (formater).sprintf("namecountermeasures%d", i+1));
		getCodeDataObject(TUIText, m_counterMeasureValues[i], formater.sprintf("valuecountermeasures%d", i+1));
		
		m_counterMeasureNames[i]->SetVisible(false);
		m_counterMeasureValues[i]->SetVisible(false);
		m_counterMeasureValues[i]->SetPreLocalized(true);
	}

	setIsUpdating(true);
	registerMediatorObject(getPage(), true);
}

//-----------------------------------------------------------------------------

SwgCuiSpaceMissileCount::~SwgCuiSpaceMissileCount()
{
	for(int i = 0; i < c_maxNumMissileSlots; ++i)
	{
		m_missileNames[i] = NULL;
		m_missileValues[i] = NULL;
	}
	for(int i = 0; i < c_maxNumCounterMeasureSlots; ++i)
	{
		m_counterMeasureNames[i] = NULL;
		m_counterMeasureValues[i] = NULL;
	}

	setIsUpdating(false);
}

//-----------------------------------------------------------------------------

void SwgCuiSpaceMissileCount::update(float const updateDeltaSeconds)
{
	CuiMediator::update(updateDeltaSeconds);

	//gather consumables data from from the ship
	ShipObject const * const shipObject = Game::getPlayerContainingShip();
	if(shipObject)
	{
		//mark all slots as empty
		ShipChassisSlotType::Type missileLaunchers[c_maxNumMissileSlots];
		int firstEmptyMissileSlot = 0;
		for(int i = 0; i < c_maxNumMissileSlots; ++i)
		{ 
			missileLaunchers[i] = ShipChassisSlotType::SCST_invalid;
		}

		ShipChassisSlotType::Type cmLaunchers[c_maxNumCounterMeasureSlots];
		int firstEmptyCMSlot = 0;
		for(int i = 0; i < c_maxNumCounterMeasureSlots; ++i)
		{
			cmLaunchers[i] = ShipChassisSlotType::SCST_invalid;
		}

		for(int i = ShipChassisSlotType::SCST_first; i < ShipChassisSlotType::SCST_invalid; ++i)
		{
			ShipChassisSlotType::Type const slotType = static_cast<ShipChassisSlotType::Type>(i);
			if(shipObject->isSlotInstalled(slotType))
			{
				uint32 const crc = shipObject->getComponentCrc(slotType);
				if(ShipComponentWeaponManager::isMissile(crc))
				{
					//claim a slot if we have one free
					if(firstEmptyMissileSlot < c_maxNumMissileSlots)
					{
						missileLaunchers[firstEmptyMissileSlot] = slotType;
						++firstEmptyMissileSlot;
					}
					else
					{
						// this will fatal a debug client if we add a ship
						// with more than MAX_NUM_MISSILE_SLOTS to tell us
						// to update the ui page
						DEBUG_FATAL(true, ("Too many missile launchers."));
					}
				}
				else if(ShipComponentWeaponManager::isCountermeasure(crc))
				{
					if(firstEmptyCMSlot < c_maxNumCounterMeasureSlots)
					{
						cmLaunchers[firstEmptyCMSlot] = slotType;
						++firstEmptyCMSlot;
					}
					else
					{
						DEBUG_FATAL(true, ("Too many countermeasure launchers."));
					}
				}
			}
		}

		//set the data into the text fields, showing or hiding them as necessary
		bool somethingVisible = false;

		for(int i = 0; i < c_maxNumMissileSlots; ++i)
		{ 
			somethingVisible |= setupAmmoConsumingSlot(*shipObject, *(m_missileNames[i]), *(m_missileValues[i]), missileLaunchers[i]);
		}
		for(int i = 0; i < c_maxNumCounterMeasureSlots; ++i)
		{
			somethingVisible |= setupAmmoConsumingSlot(*shipObject, *(m_counterMeasureNames[i]), *(m_counterMeasureValues[i]), cmLaunchers[i]);
		}

		//show or hide widget as necessary
		if(!isActive() && somethingVisible)
			activate();
		else if(isActive() && !somethingVisible)
			deactivate();
	}
}

//=============================================================================
