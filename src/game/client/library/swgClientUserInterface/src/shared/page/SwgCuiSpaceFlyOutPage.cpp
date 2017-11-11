//=============================================================================
//
// SwgCuiSpaceFlyOutPage.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//=============================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiSpaceFlyOutPage.h"

#include "clientGame/Game.h"
#include "clientGame/ProsePackageManagerClient.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipWeaponGroupManager.h"
#include "clientUserInterface/CuiDamageManager.h"
#include "clientUserInterface/CuiStringIdsSpaceHud.h"
#include "clientUserInterface/CuiStringIdsSpaceHud.h"
#include "sharedFoundation/Fatal.h"
#include "sharedFoundation/Timer.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipChassisSlot.h"
#include "sharedGame/ShipComponentFlags.h"
#include "sharedGame/ShipComponentType.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ShipDamageMessage.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/Object.h"

#include "UIBaseObject.h"
#include "UIButton.h"
#include "UIComposite.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UIWidget.h"

#include <map>
#include <set>

//=============================================================================

namespace SwgCuiSpaceFlyOutPageNamespace
{
	//-------------------------------------------------------------------------

	// TODO: If we keep this feature, move to UI options.
	float const s_defaultFlyoutPageDuration = 3.5f;

	float const s_cargoFlyoutPageDuration = 20.0f;

	//-- Disable the page this duration prior release to trigger some effects.
	float const s_disablePageDuration = 0.25f;

	//-- Keep track of the sample page.
	UIPage * s_popOutPageSample = NULL;

	//-- Cache the ship object.
	CachedNetworkId s_playerShip = CachedNetworkId::cms_cachedInvalid;

	// Enable weapon group flyout sounds.
	bool const s_useWeaponGroupFlyoutSounds = false;
	UIString const s_noSoundModifier(L"_group");

	// Show the weapon group after the player stops shooting.
	int s_selectedWeaponGroup = -1;
	float const s_selectedWeaponGroupTimoutSeconds = 1.0f; 
	Timer s_selectedWeaponGroupTimer(s_selectedWeaponGroupTimoutSeconds);

	//-------------------------------------------------------------------------

	struct ComponentInfo
	{
		ComponentInfo() : 
			m_isInitialized(false),
			m_refreshInfo(false),
			m_damage(FLT_MAX), 
			m_flags(0)
		{
		}

		void refresh() 
		{ 
			m_damage	= FLT_MAX; 
			m_flags		= 0;
			m_refreshInfo = true;
		}

		bool setFlag(int const flag, bool const value)
		{
			if (value)
				m_flags |= flag;
			else
				m_flags &= static_cast<int>(~static_cast<unsigned>(flag));

			return m_flags & flag;
		}

		bool m_isInitialized;
		bool m_refreshInfo;
		float m_damage;
		int m_flags;
	};

	ComponentInfo s_componentInfo[ShipChassisSlotType::SCST_num_types];

	float const s_damageDiffThreshold = 0.001f; // 0.1% for now.
	float const s_repairDiffThreshold = 0.1f; //10% to avoid annoying tick up.

	//-- Store the long name.
	std::string s_componentName;
	UIString const spaceString(Unicode::narrowToWide(" "));



	//-- Update every 1/3 of a second.
	float const cs_updateDuration = 1.0f / 3.0f;
	Timer s_updateTimer;
	
	//-------------------------------------------------------------------------

	//-- Contains the flyout logic.
	class FlyOutTab
	{
	public:
		FlyOutTab(int chassisSlot, UIPage * parent, UIPage * popup, float const lifespan, float const disable);
		~FlyOutTab();
		
		void setPercent(Unicode::String const & strCompName, Unicode::String const & strSlotName, float const percent, UIColor const & color);
		void setText(Unicode::String const & strCompName, Unicode::String const & strSlotName, Unicode::String const & strDamageType, UIString const & indexString);
		void setTextOnly(Unicode::String const & str);
		bool update(const float timeDeltaSeconds);
		void resetLifespan();
		int getChassisSlot() const;
		
	private:
		FlyOutTab();
		
	private:
		UIPage * m_flyout;
		UIText * m_text;
		UIText * m_percent;
		UIText * m_component;
		Timer m_lifespan;
		float m_disable;
		int m_chassisSlot;
	};

	//-------------------------------------------------------------------------

	typedef std::map<ShipChassisSlotType::Type, UIButton * /*flyout object*/> FlyOutRegisteredTabs;
	FlyOutRegisteredTabs s_registeredFlyOutPages;
	std::multiset<UIButton const *> s_buttonVisibleSet;

	//-------------------------------------------------------------------------

	bool hasValidFlyOutPage(int const componentIndex)
	{
		return s_registeredFlyOutPages.find(static_cast<ShipChassisSlotType::Type>(componentIndex)) != s_registeredFlyOutPages.end();
	}
	
	//-------------------------------------------------------------------------

	void addTypeIndex(UIButton * button, ShipChassisSlotType::Type const type, int const index)
	{
		if (button)
		{
			char indexProperty[32];
			sprintf(indexProperty, "TypeIndex%d", type);

			char value[32];
			_itoa(index, value, 10);
			
			button->SetProperty(UILowerString(indexProperty), Unicode::narrowToWide(value));
		}
	}

	//-------------------------------------------------------------------------
	
	bool getTypeIndex(UIButton const * const button, ShipChassisSlotType::Type const type, UIString & indexString)
	{
		bool hasProperty = false;

		if (button)
		{
			char indexProperty[32];
			sprintf(indexProperty, "TypeIndex%d", type);
			hasProperty = button->GetProperty(UILowerString(indexProperty), indexString);
		}

		return hasProperty;
	}

	//-------------------------------------------------------------------------

	UIButton * getFlyoutButton(int const componentIndex)
	{
		UIButton * button = NULL;

		FlyOutRegisteredTabs::iterator itRegSlot = s_registeredFlyOutPages.find(static_cast<ShipChassisSlotType::Type>(componentIndex));

		if(itRegSlot != s_registeredFlyOutPages.end())
		{
			button = itRegSlot->second;
		}

		return button;
	}
	
	//-------------------------------------------------------------------------

	typedef std::map<int /*slot index*/, FlyOutTab *> FlyOutTabList;
	FlyOutTabList s_flyOutTabs;

	//-------------------------------------------------------------------------

	FlyOutTab * createTab(int const slotIndex, UIPage * parentPage, bool useSound = true)
	{
		FlyOutTab * flyout = NULL;

		if (!parentPage)
			return NULL;

		FlyOutTabList::iterator itExistingTab = s_flyOutTabs.find(slotIndex);

		if (itExistingTab == s_flyOutTabs.end())
		{
			//-- Create new page.
			UIPage * const flyOutPage = NON_NULL(UI_ASOBJECT(UIPage, s_popOutPageSample->DuplicateObject()));

			//-- Disable sounds.
			if (!useSound)
			{
				UIString effector;
				flyOutPage->GetProperty(UIWidget::PropertyName::OnShowEffector, effector);
				flyOutPage->SetProperty(UIWidget::PropertyName::OnShowEffector, effector + s_noSoundModifier);
			}

			float const duration = (slotIndex == ShipChassisSlotType::SCST_cargo_hold) ? s_cargoFlyoutPageDuration : s_defaultFlyoutPageDuration;
			//-- Create new FlyOut.
			flyout = NON_NULL(new FlyOutTab(slotIndex, parentPage, flyOutPage, duration, s_disablePageDuration));

			//-- Set the location.
			UIButton * targetButton = getFlyoutButton(slotIndex);
			if (targetButton)
				flyOutPage->SetLocation(targetButton->GetLocation());


			//-- Push back the data.
			s_flyOutTabs[slotIndex] = flyout;
		}
		else
		{
			flyout = itExistingTab->second;
		}

		// Reset the timers.
		flyout->resetLifespan();

		//-- Done!
		return flyout;
	}


	//-------------------------------------------------------------------------

	void updateTabs(float const timeDelta)
	{
		for(FlyOutTabList::iterator itTabs = s_flyOutTabs.begin(); itTabs != s_flyOutTabs.end(); /*inline*/)
		{
			FlyOutTab * const flyout = itTabs->second;
			if (flyout && flyout->update(timeDelta))
			{
				s_flyOutTabs.erase(itTabs++);
				delete flyout;
			}
			else
			{
				++itTabs;
			}
		}
	}

	//-------------------------------------------------------------------------

	void deleteTabs()
	{
		for(FlyOutTabList::iterator itTabs = s_flyOutTabs.begin(); itTabs != s_flyOutTabs.end(); /*inline*/)
		{
			FlyOutTab * const flyout = itTabs->second;
			if (flyout)
			{
				s_flyOutTabs.erase(itTabs++);
				delete flyout;
			}
			else
			{
				++itTabs;
			}
		}
	}
	
	//-------------------------------------------------------------------------
	
	void deleteTab(FlyOutTab const * const tab)
	{
		if (tab)
		{
			FlyOutTabList::iterator it = s_flyOutTabs.find(tab->getChassisSlot());
			if (it != s_flyOutTabs.end())
			{
				FlyOutTab * const flyout = it->second;
				if (flyout == tab)
				{
					s_flyOutTabs.erase(it);
					delete flyout;
				}
			}
		}
	}

	//-------------------------------------------------------------------------
	
	void deleteTab(int const chassisSlot)
	{
		FlyOutTabList::iterator itExistingTab = s_flyOutTabs.find(chassisSlot);
		
		if (itExistingTab != s_flyOutTabs.end())
		{
			FlyOutTab * const flyout = itExistingTab->second;
			if (flyout)
			{
				s_flyOutTabs.erase(itExistingTab);
				delete flyout;
			}
		}
	}

	
	//-------------------------------------------------------------------------

	Unicode::String getChassisSlotLongName(int const chassisSlot)
	{
		int const componentType = ShipChassisSlotType::getComponentTypeForSlot(static_cast<ShipChassisSlotType::Type>(chassisSlot));
		std::string const & componentShortName = ShipComponentType::getNameFromType(static_cast<ShipComponentType::Type>(componentType));
		StringId stringId("ship_slot_n", componentShortName);
		return stringId.localize();
	}

	//-------------------------------------------------------------------------
	
	void resetComponentInfo()
	{
		for (int chassisSlot = ShipChassisSlotType::SCST_first; chassisSlot < ShipChassisSlotType::SCST_num_types; ++chassisSlot)
		{
			s_componentInfo[chassisSlot].m_isInitialized = false;
		}
	}

	//-------------------------------------------------------------------------
	
	void updatePlayerShip()
	{
		ShipObject * playerShip = Game::getPlayerContainingShip();
		
		if (s_playerShip.getObject() != playerShip)
		{
			//-- Cache off the ShipObject Id.
			s_playerShip = playerShip ? playerShip->getNetworkId() : CachedNetworkId::cms_cachedInvalid;
			resetComponentInfo();
			deleteTabs();

			//-- Ensure the visible set is empty. If it is not, something is not being cleaned up properly.
			DEBUG_FATAL(!s_buttonVisibleSet.empty(), ("SwgCuiSpaceFlyoutPageNamespace::updatePlayerShip - s_buttonVisibleSet is not empty."));
		}
	}
	
	//-------------------------------------------------------------------------
	
	bool isDesiredComponentFlag(int const componentFlag)
	{
		return	(componentFlag & static_cast<int>(ShipComponentFlags::F_disabled)) ||
				(componentFlag & static_cast<int>(ShipComponentFlags::F_lowPower)) ||
				(componentFlag & static_cast<int>(ShipComponentFlags::F_demolished)) ||
				(componentFlag & static_cast<int>(ShipComponentFlags::F_active));
	}

	//----------------------------------------------------------------------

	void handleInfoForSlot(UIPage * parentPage, ShipObject const * const shipObject, ShipChassis const * const shipChassis, int const chassisSlot, bool const forceUpdateAll)
	{
	// Need to check to see if this button is visible.
		UIButton const * const buttonForSlot = getFlyoutButton(chassisSlot);
		
		// Get a ref to the component info.
		ComponentInfo & compInfo = s_componentInfo[chassisSlot];
		
		// If flagged for reset, kill the tab first.
		if (compInfo.m_isInitialized && compInfo.m_refreshInfo)
		{
			deleteTab(chassisSlot);
		}
		
		// If weapon is already updated, stop here.
		if (s_buttonVisibleSet.find(buttonForSlot) == s_buttonVisibleSet.end() && hasValidFlyOutPage(chassisSlot) && shipObject->isSlotInstalled(chassisSlot))
		{
			float hpCurrent = 0.0f;
			float hpMax  = 0.0f;
			
			hpMax  = shipObject->getComponentHitpointsMaximum(chassisSlot);
			hpCurrent = shipObject->getComponentHitpointsCurrent(chassisSlot);
			
			float hpPercent = (hpMax > FLT_MIN) ? hpCurrent / hpMax : 1.0f;
			int const componentFlags = shipObject->getComponentFlags(chassisSlot);
			
			FlyOutTab * newTab = NULL;
			
			//-- cargo holds don't display their hitpoints, only their contents
			if (chassisSlot == ShipChassisSlotType::SCST_cargo_hold)
			{					
				int const cargoCurrent = shipObject->getCargoHoldContentsCurrent();
				int const cargoMaximum = shipObject->getCargoHoldContentsMaximum();				
				hpPercent = (cargoMaximum ? static_cast<float>(cargoCurrent) / cargoMaximum : 0.0f);
			}

			if (compInfo.m_isInitialized && (compInfo.m_refreshInfo || forceUpdateAll))
			{
				Unicode::String const & componentName = shipObject->getComponentName(chassisSlot);
				ShipChassisSlot const * const slot = shipChassis->getSlot(static_cast<ShipChassisSlotType::Type>(chassisSlot));
				Unicode::String const componentSlotName = slot ? slot->getLocalizedSlotName() : Unicode::emptyString;

				if (chassisSlot == ShipChassisSlotType::SCST_cargo_hold)
				{
					compInfo.m_refreshInfo = false;
										
					if (!WithinEpsilonExclusive(hpPercent, compInfo.m_damage, 0.005f))
					{
						compInfo.m_damage = hpPercent;
						newTab = createTab(chassisSlot, parentPage);
						if (newTab)
						{
							newTab->setPercent(componentName, componentSlotName, hpPercent, UIColor::green);
						}
					}

					return;
				}
				
				UIString indexString;
				IGNORE_RETURN(getTypeIndex(buttonForSlot, static_cast<ShipChassisSlotType::Type>(chassisSlot), indexString));
				
				float const damageThreshold = (hpPercent < compInfo.m_damage) ? s_damageDiffThreshold : s_repairDiffThreshold;
				bool const damageUpdateRequired = !WithinEpsilonInclusive(hpPercent, compInfo.m_damage, damageThreshold);
				bool const componentUpdateRequired = (compInfo.m_flags != componentFlags) && isDesiredComponentFlag(componentFlags);
				
				if (componentUpdateRequired)
				{
					if (compInfo.m_flags & ShipComponentFlags::F_disabled)
					{
						bool const value = compInfo.setFlag(ShipComponentFlags::F_disabled, (componentFlags & ShipComponentFlags::F_disabled) > 0);
						newTab = createTab(chassisSlot, parentPage);
						if (newTab)
						{
							newTab->setText(componentName, componentSlotName, 
								value ? CuiStringIdsSpaceHud::component_disabled.localize() : CuiStringIdsSpaceHud::component_enabled.localize(), 
								indexString);
						}
					}
					else if (compInfo.m_flags & ShipComponentFlags::F_demolished)
					{
						bool const value = compInfo.setFlag(ShipComponentFlags::F_demolished, (componentFlags & ShipComponentFlags::F_demolished) > 0);
						newTab = createTab(chassisSlot, parentPage);
						if (newTab)
						{
							newTab->setText(componentName, componentSlotName, 
								value ? CuiStringIdsSpaceHud::component_demolished.localize() : CuiStringIdsSpaceHud::component_repaired.localize(),
								indexString);
						}
					}
					else if (compInfo.m_flags & ShipComponentFlags::F_active)
					{
						newTab = createTab(chassisSlot, parentPage);
						if (newTab)
						{
							bool const value = compInfo.setFlag(ShipComponentFlags::F_active, (componentFlags & ShipComponentFlags::F_active) > 0);
							newTab->setText(componentName, 
								componentSlotName, value ? CuiStringIdsSpaceHud::component_active.localize() : CuiStringIdsSpaceHud::component_inactive.localize(),
								indexString);
						}
					}
					else if (compInfo.m_flags & ShipComponentFlags::F_lowPower)
					{
						bool const value = compInfo.setFlag(ShipComponentFlags::F_lowPower, (componentFlags & ShipComponentFlags::F_lowPower) > 0);
						newTab = createTab(chassisSlot, parentPage);
						if (newTab)
						{
							newTab->setText(componentName, componentSlotName, 
								value ? CuiStringIdsSpaceHud::component_lowpower.localize() : CuiStringIdsSpaceHud::component_fullpower.localize(),
								indexString);
						}
					}
					
					
					compInfo.m_flags = componentFlags;
					
				}
				else if (damageUpdateRequired)
				{
					newTab = createTab(chassisSlot, parentPage);
					if (newTab)
					{
						compInfo.m_damage = hpPercent;
						newTab->setPercent(componentName, componentSlotName, hpPercent, UIColor::red);
					}
				}
				
				
				if (newTab)
				{
					// Reset the force refresh if we create a page to display the information.
					compInfo.m_refreshInfo = damageUpdateRequired && componentUpdateRequired;
				}

				return;
			}
			
			if (!compInfo.m_isInitialized)
			{
				compInfo.m_damage = hpPercent;
				compInfo.m_flags = componentFlags;
				compInfo.m_refreshInfo = false;
				compInfo.m_isInitialized = true;
			}
		}
	}

	//-------------------------------------------------------------------------

	void triggerDamageInformation(UIPage * parentPage, bool const forceUpdateAll = false)
	{
		if (!parentPage)
			return;

		updatePlayerShip();

		//-- Get the ship object from the CachedNetworkId
		Object const * const object = s_playerShip.getObject();
		ClientObject const * const clientObject = object ? object->asClientObject() : NULL;
		ShipObject const * const shipObject = clientObject ? clientObject->asShipObject() : NULL;

		ShipChassis const * const shipChassis = shipObject ? ShipChassis::findShipChassisByCrc (shipObject->getChassisType()) : NULL;

		//-- Update the damage information.
		if (!shipObject || !shipChassis || !shipObject->isInitialized())
			return;
		
		for (int chassisSlot = ShipChassisSlotType::SCST_first; chassisSlot < ShipChassisSlotType::SCST_num_types; ++chassisSlot)
		{
			handleInfoForSlot(parentPage, shipObject, shipChassis, chassisSlot, forceUpdateAll);
		}
	}
}

using namespace SwgCuiSpaceFlyOutPageNamespace;

//=============================================================================

SwgCuiSpaceFlyOutPageNamespace::FlyOutTab::FlyOutTab(int const chassisSlot, UIPage * const parent, UIPage * const popup, float const lifespan, float const disable) :
m_flyout(popup)
, m_text(NULL)
, m_percent(NULL)
, m_component(NULL)
, m_lifespan(lifespan)
, m_disable(disable)
, m_chassisSlot(chassisSlot)
{
	parent->AddChild(m_flyout);
	parent->Link();
	
	m_flyout->SetEnabled(false);
	m_flyout->SetEnabled(true);
	m_flyout->SetVisible(true);
	
	m_flyout->GetCodeDataObject(TUIText, m_text, "text");
	m_flyout->GetCodeDataObject(TUIText, m_percent, "percent");
	m_flyout->GetCodeDataObject(TUIText, m_component, "component");


	// Clear out the text.
	m_text->Clear();
	m_percent->Clear();
	m_component->Clear();

	IGNORE_RETURN(s_buttonVisibleSet.insert(getFlyoutButton(m_chassisSlot)));
}

//-----------------------------------------------------------------------------

int SwgCuiSpaceFlyOutPageNamespace::FlyOutTab::getChassisSlot() const
{
	return m_chassisSlot;
}

//-----------------------------------------------------------------------------

void SwgCuiSpaceFlyOutPageNamespace::FlyOutTab::setPercent(Unicode::String const & strCompName, Unicode::String const & strSlotName, float const percent, UIColor const & color)
{
	// Clear the page.
	if (m_text && m_percent && m_component)
	{
		// Set the percent damaged.
		int const perc100 = static_cast<int>(percent * 100.f);
		char buff[32];
		_snprintf(buff, sizeof(buff) - 1, "%d%%", perc100);
	
		// Set the info.
		m_percent->SetText(UIUnicode::narrowToWide(buff));
		m_percent->SetTextColor(color);
		m_text->SetText(strCompName);
		m_component->SetText(strSlotName);

		m_flyout->Pack();
	}
}

//-----------------------------------------------------------------------------

void SwgCuiSpaceFlyOutPageNamespace::FlyOutTab::setText(Unicode::String const & strCompName, Unicode::String const & strSlotName, Unicode::String const & strDamageType, UIString const & indexString)
{
	// Clear the page.
	if (m_text && m_percent && m_component)
	{
		// Set the info.
		m_percent->SetText(strDamageType);
		m_text->SetText(strCompName);
		m_component->SetText(strSlotName + spaceString + indexString);

		m_flyout->Pack();
	}
}

//-----------------------------------------------------------------------------

void SwgCuiSpaceFlyOutPageNamespace::FlyOutTab::setTextOnly(Unicode::String const & str)
{
	// Clear the page.
	if (m_text && m_percent && m_component)
	{
		m_text->SetText(str);
		m_percent->Clear();
		m_component->Clear();

		m_flyout->Pack();
	}
}

//-----------------------------------------------------------------------------

bool SwgCuiSpaceFlyOutPageNamespace::FlyOutTab::update(const float timeDeltaSeconds)
{
	if (m_flyout->IsEnabled() && m_lifespan.getRemainingTime() <= m_disable)
		m_flyout->SetEnabled(false);

	return m_lifespan.updateNoReset(timeDeltaSeconds);
}


//-----------------------------------------------------------------------------

void SwgCuiSpaceFlyOutPageNamespace::FlyOutTab::resetLifespan()
{
	m_lifespan.reset();
}

//-----------------------------------------------------------------------------

SwgCuiSpaceFlyOutPageNamespace::FlyOutTab::~FlyOutTab()
{
	IGNORE_RETURN(s_buttonVisibleSet.erase(getFlyoutButton(m_chassisSlot)));
	m_flyout->RemoveFromParent();
	m_flyout = NULL;
	m_text = NULL;
	m_percent = NULL;
	m_disable = 0.0f;
}

//=============================================================================

SwgCuiSpaceFlyOutPage::SwgCuiSpaceFlyOutPage(UIPage & page) :
	SwgCuiLockableMediator("FlyOut", page),
	Receiver(),
	m_callback(new MessageDispatch::Callback)
{
	// Force visible.
	getPage().SetVisible(true);

	// Fixup any data issues.
	if (getPage().GetMinimumSize() == UISize::zero)
	{
		getPage().SetMinimumSize(getPage().GetSize());
		getPage().SetMaximumSize(getPage().GetSize());
	}

	//-- Register flyOut.
	getCodeDataObject(TUIPage, s_popOutPageSample, "popOut");
	s_popOutPageSample->SetVisible(false);


	//-- Register buttons with the FlyOut.  Move to a method.
	UIButton * flyoutButton = NULL;
	getCodeDataObject(TUIButton, flyoutButton, "buttonArmor");
	registerMediatorObject(*flyoutButton, true);
	s_registeredFlyOutPages[ShipChassisSlotType::SCST_armor_0] = flyoutButton;
	addTypeIndex(flyoutButton, ShipChassisSlotType::SCST_armor_0, 1);

	s_registeredFlyOutPages[ShipChassisSlotType::SCST_armor_1] = flyoutButton;
	addTypeIndex(flyoutButton, ShipChassisSlotType::SCST_armor_1, 2);

	//--
	getCodeDataObject(TUIButton, flyoutButton, "buttonCapacitor");
	registerMediatorObject(*flyoutButton, true);
	s_registeredFlyOutPages[ShipChassisSlotType::SCST_capacitor] = flyoutButton;

	//--
	getCodeDataObject(TUIButton, flyoutButton, "buttonEngine");
	registerMediatorObject(*flyoutButton, true);
	s_registeredFlyOutPages[ShipChassisSlotType::SCST_engine] = flyoutButton;

	//--
	getCodeDataObject(TUIButton, flyoutButton, "buttonWeapon");
	registerMediatorObject(*flyoutButton, true);
	int weaponCount = 1;
	for (int weaponIndex = ShipChassisSlotType::SCST_weapon_first; weaponIndex <= ShipChassisSlotType::SCST_weapon_last; ++weaponIndex)
	{
		s_registeredFlyOutPages[static_cast<ShipChassisSlotType::Type>(weaponIndex)] = flyoutButton;
		addTypeIndex(flyoutButton, static_cast<ShipChassisSlotType::Type>(weaponIndex), weaponCount++);
	}

	getCodeDataObject(TUIButton, flyoutButton, "buttonShield");
	registerMediatorObject(*flyoutButton, true);
	s_registeredFlyOutPages[ShipChassisSlotType::SCST_shield_0] = flyoutButton;
	addTypeIndex(flyoutButton, ShipChassisSlotType::SCST_shield_0, 1);

	s_registeredFlyOutPages[ShipChassisSlotType::SCST_shield_1] = flyoutButton;
	addTypeIndex(flyoutButton, ShipChassisSlotType::SCST_shield_1, 2);

	//--
	getCodeDataObject(TUIButton, flyoutButton, "buttonReactor");
	registerMediatorObject(*flyoutButton, true);
	s_registeredFlyOutPages[ShipChassisSlotType::SCST_reactor] = flyoutButton;

	getCodeDataObject(TUIButton, flyoutButton, "buttonCargo");
	registerMediatorObject(*flyoutButton, true);
	s_registeredFlyOutPages[ShipChassisSlotType::SCST_cargo_hold] = flyoutButton;


	//--
	s_updateTimer.setExpireTime(cs_updateDuration);

	resetComponentInfo();

	connectToMessage(Game::Messages::SCENE_CHANGED);

	registerMediatorObject(getPage(), true);
}

//-----------------------------------------------------------------------------

SwgCuiSpaceFlyOutPage::~SwgCuiSpaceFlyOutPage()
{
	deleteTabs();

	s_playerShip = CachedNetworkId::cms_cachedInvalid;

	disconnectFromMessage(Game::Messages::SCENE_CHANGED);

	delete m_callback;
	m_callback = 0;
}

//-----------------------------------------------------------------------------

void SwgCuiSpaceFlyOutPage::performActivate()
{
	CuiMediator::performActivate();
	m_callback->connect(*this, &SwgCuiSpaceFlyOutPage::onShipDamaged, static_cast<CuiDamageManager::Messages::ShipDamage *>(0));
	m_callback->connect(*this, &SwgCuiSpaceFlyOutPage::onWeaponGroupChanged, static_cast<ShipWeaponGroupManager::Messages::WeaponGroupChanged *>(0));
	m_callback->connect(*this, &SwgCuiSpaceFlyOutPage::onCargoChanged, static_cast<ShipObject::Messages::CargoChanged *>(0));
	setIsUpdating(true);
}

//-----------------------------------------------------------------------------

void SwgCuiSpaceFlyOutPage::performDeactivate()
{
	CuiMediator::performDeactivate();
	m_callback->disconnect (*this, &SwgCuiSpaceFlyOutPage::onWeaponGroupChanged, static_cast<ShipWeaponGroupManager::Messages::WeaponGroupChanged *>(0));
	m_callback->disconnect(*this, &SwgCuiSpaceFlyOutPage::onShipDamaged, static_cast<CuiDamageManager::Messages::ShipDamage *>(0));
	m_callback->disconnect(*this, &SwgCuiSpaceFlyOutPage::onCargoChanged, static_cast<ShipObject::Messages::CargoChanged *>(0));
	s_playerShip = CachedNetworkId::cms_cachedInvalid;
	setIsUpdating(false);
}

//-----------------------------------------------------------------------------

void SwgCuiSpaceFlyOutPage::onShipDamaged(const CuiDamageManager::Messages::ShipDamage::Payload &)
{
	triggerDamageInformation(&getPage(), true);

} //lint !e1762 could be made const.

//-----------------------------------------------------------------------------

void SwgCuiSpaceFlyOutPage::update(float const updateDeltaSeconds)
{
	CuiMediator::update(updateDeltaSeconds);

	if (updateDeltaSeconds < cs_updateDuration && s_updateTimer.updateZero(updateDeltaSeconds))
	{
		triggerDamageInformation(&getPage(), true);
	}

	updateTabs(updateDeltaSeconds);

	// Update weapon group flyout.
	if (s_selectedWeaponGroupTimer.updateZero(updateDeltaSeconds) && (s_selectedWeaponGroup != -1))
	{
		// Delete the old tab.
		deleteTab(ShipChassisSlotType::SCST_weapon_first);
		
		// Create the new tab.
		FlyOutTab * const newTab = createTab(ShipChassisSlotType::SCST_weapon_first, &getPage(), s_useWeaponGroupFlyoutSounds);
		if (newTab)
		{
			ProsePackage pp;
			pp.stringId   = CuiStringIdsSpaceHud::selectedgroupprose;
			pp.digitInteger = s_selectedWeaponGroup + 1;
			Unicode::String resultStr;
			IGNORE_RETURN(ProsePackageManagerClient::appendTranslation(pp, resultStr));
			newTab->setTextOnly(resultStr);
		}

		// Reset the weapon group index.
		s_selectedWeaponGroup = -1;
	} 
}

//----------------------------------------------------------------------

void SwgCuiSpaceFlyOutPage::onWeaponGroupChanged(const ShipWeaponGroupManager::Messages::WeaponGroupChanged::Payload & weaponGroup)
{
	s_selectedWeaponGroup = weaponGroup.second;
	s_selectedWeaponGroupTimer.reset();
}

//----------------------------------------------------------------------

void SwgCuiSpaceFlyOutPage::OnButtonPressed(UIWidget *context)
{
	bool triggerUpdate = false;

	if (context)
	{
		for (FlyOutRegisteredTabs::const_iterator itPage = s_registeredFlyOutPages.begin(); itPage != s_registeredFlyOutPages.end(); ++itPage)
		{
			ShipChassisSlotType::Type const slotType = (*itPage).first;
			UIButton const * const pageButton = (*itPage).second;
			UIWidget const * const pageButtonWidget = pageButton;
			// If we have a context page and it is not already visible, refresh.
			if (pageButtonWidget == context)
			{
				ComponentInfo & componentInfo = s_componentInfo[itPage->first];

				FlyOutTabList::const_iterator const itExistingTab = s_flyOutTabs.find(static_cast<int>(slotType));
				if (itExistingTab != s_flyOutTabs.end())
				{
					deleteTab(static_cast<int>(slotType));
				}
				else
				{
					componentInfo.refresh();
					triggerUpdate = true;
				}
			}
		}
	}
	
	if (triggerUpdate)
	{
		triggerDamageInformation(&getPage());
	}
}


//-----------------------------------------------------------------

void SwgCuiSpaceFlyOutPage::receiveMessage(const MessageDispatch::Emitter& , const MessageDispatch::MessageBase& message)
{
	if (message.isType(Game::Messages::SCENE_CHANGED))
	{
		resetComponentInfo();
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceFlyOutPage::onCargoChanged(ShipObject::Messages::CargoChanged::Payload & payload)
{
	if (s_playerShip == payload.getNetworkId())
	{
		s_componentInfo[static_cast<int>(ShipChassisSlotType::SCST_cargo_hold)].refresh();
		triggerDamageInformation(&getPage());
	}
}

//=============================================================================
