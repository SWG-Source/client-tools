//======================================================================
//
// SwgCuiStatusSpace.cpp
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiStatusSpace.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/ShipObject.h"
#include "clientGame/TangibleObject.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiStringIdsShipView.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/NetworkIdManager.h"

#include "UIComposite.h"
#include "UIImage.h"
#include "UIText.h"
#include "UIUtils.h"

#include <map>

//======================================================================

namespace SwgCuiStatusSpaceNamespace
{
	//fixme this must be data driven
	std::string const cs_membraneShader("shader/ui_membrane.sht");

	void textClear(UIText * const widget)
	{
		if (widget)
		{
			widget->Clear();
		}
	}

	void widgetSetVisible(UIWidget * const widget, bool const isVisible)
	{
		if (widget)
		{
			bool previousVisible = widget->IsVisible();
			widget->SetVisible(isVisible);

			if (!boolEqual(previousVisible, isVisible))
			{
				UIComposite * const composite = static_cast<UIComposite*>(widget->GetParent(TUIComposite)); //lint !e1774
				if (composite)
				{
					composite->SetPackDirty(true);
				}
			}
		}
	}
}

using namespace SwgCuiStatusSpaceNamespace;

//----------------------------------------------------------------------

SwgCuiStatusSpace::SwgCuiStatusSpace(UIPage & page) :
SwgCuiMfdStatus("SwgCuiStatusSpace", page),
m_displayStats(false),
m_targetNamePrefix(),
m_lastNameUpdateSecs(0.0f),
m_showRange(false),
m_iconDifficulty(NULL),
m_textName(NULL),
m_textType(NULL),
m_textDifficulty(NULL),
m_shieldsPercent(NULL),
m_armorPercent(NULL),
m_systemPercent(NULL),
m_chassisPercent(NULL),
m_textComponent(NULL),
m_imperialFaction(NULL),
m_rebelFaction(NULL),
m_objectViewer(NULL),
m_lastRenderObject(CachedNetworkId::cms_invalid),
m_canFollow(NULL),
m_canMatchSpeed(NULL),
m_buttonBar(NULL),
m_targetConditionWidgets(new ConditionMap)
{
	getCodeDataObject(TUIImage, m_iconDifficulty, "iconDifficulty", true);

	getCodeDataObject(TUIText, m_textName, "targetName", true);
	getCodeDataObject(TUIText, m_textType, "targetType", true);
	getCodeDataObject(TUIText, m_textDifficulty, "targetDifficulty", true);
	getCodeDataObject(TUIText, m_shieldsPercent, "shieldPercent", true);
	getCodeDataObject(TUIText, m_armorPercent, "armorPercent", true);
	getCodeDataObject(TUIText, m_systemPercent, "systemPercent", true);
	getCodeDataObject(TUIText, m_chassisPercent, "chassisPercent", true);
	getCodeDataObject(TUIText, m_textComponent, "targetComponent", true);
	getCodeDataObject(TUIPage, m_imperialFaction, "imperialfactionimage", true);
	getCodeDataObject(TUIPage, m_rebelFaction, "rebelfactionimage", true);

	getCodeDataObject(TUIWidget, m_canFollow, "canFollow", true);
	getCodeDataObject(TUIWidget, m_canMatchSpeed, "canMatch", true);
	getCodeDataObject(TUIComposite, m_buttonBar, "buttonBar", true);
	

	textClear(m_textName);
	textClear(m_textType);
	textClear(m_textDifficulty);
	textClear(m_textComponent);
	textClear(m_shieldsPercent);
	textClear(m_armorPercent);
	textClear(m_systemPercent);
	textClear(m_chassisPercent);

	widgetSetVisible(m_imperialFaction, false);
	widgetSetVisible(m_rebelFaction, false);
	widgetSetVisible(m_canFollow, false);
	widgetSetVisible(m_canMatchSpeed, false);

	{
		UIWidget * targetConditionWidget = NULL;
		getCodeDataObject(TUIWidget, targetConditionWidget, "canCom", true);
		IGNORE_RETURN(m_targetConditionWidgets->insert(std::make_pair(TangibleObject::C_commable, targetConditionWidget)));
		widgetSetVisible(targetConditionWidget, false);
	}

	{
		UIWidget * targetConditionWidget = NULL;
		getCodeDataObject(TUIWidget, targetConditionWidget, "canDock", true);
		IGNORE_RETURN(m_targetConditionWidgets->insert(std::make_pair(TangibleObject::C_dockable,targetConditionWidget)));
		widgetSetVisible(targetConditionWidget, false);
	}

	{
		UIWidget * targetConditionWidget = NULL;
		getCodeDataObject(TUIWidget, targetConditionWidget, "canInspect", true);
		IGNORE_RETURN(m_targetConditionWidgets->insert(std::make_pair(TangibleObject::C_inspectable,targetConditionWidget)));
		widgetSetVisible(targetConditionWidget, false);
	}

	//-- setup the icon
	CuiWorkspaceIcon * const icon = new CuiWorkspaceIcon(this);
	icon->SetName("ChatIcon");
	icon->SetSize(UISize(32, 32));
	icon->SetBackgroundColor(UIColor(0, 0, 0, 50));
	icon->SetLocation(0, 256);
	setIcon(icon);

	// Do not set visible until needed.
	getPage().SetVisible(false);


	UIWidget *widget = NULL;
	getCodeDataObject(TUIWidget, widget, "ObjectViewer", true);
	if (widget)
	{
		m_objectViewer = NON_NULL(dynamic_cast<CuiWidget3dObjectListViewer *>(widget));
		registerMediatorObject(*m_objectViewer, true);
	}
	registerMediatorObject(getPage(), true);
} //lint !e429 // custodial pointer that is owned by the interface.

//----------------------------------------------------------------------

SwgCuiStatusSpace::~SwgCuiStatusSpace()
{
	m_iconDifficulty = NULL;
	m_textName = NULL;
	m_textComponent = NULL;
	m_textType = NULL;
	m_textDifficulty = NULL;
	m_objectViewer = NULL;
	m_shieldsPercent = NULL;
	m_armorPercent = NULL;
	m_systemPercent = NULL;
	m_chassisPercent = NULL;
	m_imperialFaction = NULL;
	m_rebelFaction = NULL;

	set3DObject(NULL);
	m_objectViewer = NULL;

	m_lastRenderObject = CachedNetworkId::cms_invalid;

	m_canFollow = NULL;
	m_canMatchSpeed = NULL;
	m_buttonBar = NULL;

	delete m_targetConditionWidgets;
	m_targetConditionWidgets = NULL;
}

//-----------------------------------------------------------------

void SwgCuiStatusSpace::performActivate()
{
	if (m_iconDifficulty)
		m_iconDifficulty->SetVisible(false);

	if (m_objectViewer)
		m_objectViewer->setPaused(false);
	setIsUpdating(true);
}

//-----------------------------------------------------------------

void SwgCuiStatusSpace::performDeactivate()
{
	setIsUpdating(false);
	
	if (m_objectViewer)
		m_objectViewer->setPaused(true);
	set3DObject(NULL);
}

//----------------------------------------------------------------------

void SwgCuiStatusSpace::update(float deltaTimeSecs)
{
	//-- Note: If you add an update to the base class, be sure to modify this code accordingly.
	CuiMediator::update(deltaTimeSecs);

	Object const * const obj = m_objectId.getObject();
	ClientObject const * const clientObj = obj ? obj->asClientObject() : 0;
	TangibleObject const * const tangObj = clientObj ? clientObj->asTangibleObject() : 0;
	if (tangObj)
	{
		updateTangible(*tangObj, deltaTimeSecs);

		ShipObject const * const ship = tangObj->asShipObject() ? tangObj->asShipObject() : ShipObject::getContainingShip(*tangObj);
		if (ship) 
		{
			updateButtonBar(ship);
			updateShip(*ship);
		}
		else
		{
			updateButtonBar(tangObj);
		}
	}
	else
	{
		set3DObject(NULL);
		setShipTarget(NULL);

		if (m_iconDifficulty)
			m_iconDifficulty->SetVisible(false);
	}
}

//----------------------------------------------------------------------

void SwgCuiStatusSpace::updateTangible(TangibleObject const & tangible, float deltaTimeSecs)
{
	ShipObject const * const ship = tangible.asShipObject();
	if (ship)
	{
		const int maxHit = static_cast<int>(ship->getMaximumChassisHitPoints());
		const int current = static_cast<int>(ship->getCurrentChassisHitPoints());
		updateTangible(tangible, current, maxHit, deltaTimeSecs);
	}
	else
	{
		const int maxHit = tangible.getMaxHitPoints();
		const int current = maxHit - tangible.getDamageTaken();
		updateTangible(tangible, current, maxHit, deltaTimeSecs);
	}
}

//----------------------------------------------------------------------
	
void SwgCuiStatusSpace::updateTangible(TangibleObject const & tangible, int const current, int const maxHit, float const /*deltaTimeSecs*/)
{
	UNREF(tangible);
	UNREF(maxHit);
	UNREF(current);

	if (m_textName)
	{
		const float curTime = Game::getElapsedTime();
		
		const float updateTime = m_showRange ? 0.25f : 0.5f;
		
		if (curTime >(m_lastNameUpdateSecs + updateTime))
		{
			updateTargetName(tangible);
			m_lastNameUpdateSecs = curTime;
		}
	}

	if (m_iconDifficulty)
	{
		if (tangible.isPlayer())
		{
			m_iconDifficulty->SetVisible(false);
		}
		else
		{
			CreatureObject const * const creature = tangible.asCreatureObject();
			if (creature)
			{
				m_iconDifficulty->SetVisible(true);
				UIColor color = CuiCombatManager::getConColor(*creature);
				m_iconDifficulty->SetColor(color);
			}
		}
	}

	UIComposite * const parent = static_cast<UIComposite*>(getPage().GetParent(TUIComposite)); //lint !e1774 // GetParent checks type.  No need to dynamic cast.
	if (parent)
		parent->SetPackDirty(true);
} 

//----------------------------------------------------------------------

void SwgCuiStatusSpace::updateShip(ShipObject const & ship)
{
	updateTargetDamage(ship);
}

//----------------------------------------------------------------------

void SwgCuiStatusSpace::setTarget(const NetworkId & id)
{
	Object * const object = NetworkIdManager::getObjectById(id);
	ClientObject * const clientObject = object ? object->asClientObject() : NULL;
	TangibleObject * const tangibleObject = clientObject ? clientObject->asTangibleObject() : NULL;

	setTarget(tangibleObject);	
}

//----------------------------------------------------------------------

void SwgCuiStatusSpace::setTarget(TangibleObject * tangible)
{
	set3DObject(tangible);

	ClientObject const * const clienTarget = tangible ? tangible->asClientObject() : 0;
	if (clienTarget)
	{	
		m_objectId = clienTarget->getNetworkId();

		// Set target to be the target ship or the containing ship.
		ShipObject const * const targetShip = clienTarget->asShipObject() ? clienTarget->asShipObject() : ShipObject::getContainingShip(*clienTarget);
		setShipTarget(targetShip);
	}
	else
	{
		m_objectId = NetworkId::cms_invalid;
		setShipTarget(NULL);
	}

	// Pack the button bar.
	if (m_buttonBar)
	{
		m_buttonBar->SetPackDirty(true);
	}

	update(1000.0f);
}

//----------------------------------------------------------------------

void SwgCuiStatusSpace::setShipTarget(ShipObject const * const ship)
{
	if(!ship)
	{
		if (m_textType)
			m_textType->Clear();
		if (m_textDifficulty)
			m_textDifficulty->Clear();
		if(m_rebelFaction)
			m_rebelFaction->SetVisible(false);
		if(m_imperialFaction)
			m_imperialFaction->SetVisible(false);
		return;
	}

	if(m_textType)
	{
		std::string const & shipType = ship->getTypeName();
		if(shipType.empty())
			m_textType->Clear();
		else
		{
			StringId s2("space/space_mobile_type", shipType);
			m_textType->SetText(s2.localize());
		}
	}

	if(m_textDifficulty)
	{
		std::string const & shipDifficulty = ship->getDifficulty();
		if(shipDifficulty.empty())
			m_textDifficulty->Clear();
		else
		{
			StringId s("space/space_difficulty", shipDifficulty);
			m_textDifficulty->SetText(s.localize());
		}
	}

	if(m_rebelFaction)
		m_rebelFaction->SetVisible(false);
	if(m_imperialFaction)
		m_imperialFaction->SetVisible(false);

	std::string const & shipFaction = ship->getFaction();
	if(shipFaction == "rebel")
	{
		if(m_rebelFaction)
		{
			m_rebelFaction->SetVisible(true);
		}
	}
	else if(shipFaction == "imperial")
	{
		if(m_imperialFaction)
		{
			m_imperialFaction->SetVisible(true);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiStatusSpace::updateTargetName(const ClientObject & obj)
{
	const Object * const player = Game::getPlayer();
	
	if (!player)
		return;
	
	if (m_showRange)
	{
		static char buf [128];
		static const size_t buf_size = sizeof(buf);
		
		const Vector & pos_w = player->getPosition_w();
		snprintf(buf, buf_size, "\\#ffffff(%.0fm)\\#. ", obj.getPosition_w().magnitudeBetween(pos_w));
		setTargetName(Unicode::narrowToWide(buf) + obj.getLocalizedName());
	}
	else
		setTargetName(obj.getLocalizedName());
}

//----------------------------------------------------------------------

void SwgCuiStatusSpace::setTargetName(const Unicode::String & name)
{
	if (m_textName)
	{
		m_textName->SetVisible(true);

		const Unicode::String & newText = m_targetNamePrefix + name;

		if (newText != m_textName->GetLocalText())
			m_textName->SetLocalText(newText);

		m_lastNameUpdateSecs = Game::getElapsedTime();
	}
}

//----------------------------------------------------------------------

void SwgCuiStatusSpace::setTargetNamePrefix(const Unicode::String & prefix)
{
	m_targetNamePrefix = prefix;
	const ClientObject * const clientObject = safe_cast<const ClientObject *>(m_objectId.getObject());

	if (clientObject)
		setTargetName(clientObject->getLocalizedName());
}

//----------------------------------------------------------------------

void SwgCuiStatusSpace::setShowRange(bool b)
{
	m_showRange = b;
	m_lastNameUpdateSecs = 0;
}

//----------------------------------------------------------------------

void SwgCuiStatusSpace::setDisplayStates(bool /*stats*/, bool /*states*/, bool /*posture*/)
{
	getPage().SetVisible(true);
}

//----------------------------------------------------------------------

void SwgCuiStatusSpace::updateTargetDamage(ShipObject const & ship)
{
	CreatureObject const * const playerCreature = Game::getPlayerCreature();
	if(playerCreature)
	{
		ShipChassisSlotType::Type const slot = playerCreature->getLookAtTargetSlot();
		bool const isSlotTargetable = ship.isValidTargetableSlot(slot);

		float const shieldsFrontRatio = (ship.getShieldHitpointsFrontMaximum() != 0.0f) ? ship.getShieldHitpointsFrontCurrent() / ship.getShieldHitpointsFrontMaximum() : 0.0f;
		float const shieldsBackRatio = (ship.getShieldHitpointsBackMaximum() != 0.0f) ? ship.getShieldHitpointsBackCurrent() / ship.getShieldHitpointsBackMaximum() : 0.0f;
		float const armorFrontRatio = (ship.getComponentArmorHitpointsMaximum(ShipChassisSlotType::SCST_armor_0) != 0.0f) ? ship.getComponentArmorHitpointsCurrent(ShipChassisSlotType::SCST_armor_0) / ship.getComponentArmorHitpointsMaximum(ShipChassisSlotType::SCST_armor_0) : 0.0f;
		float const armorBackRatio = (ship.getComponentArmorHitpointsMaximum(ShipChassisSlotType::SCST_armor_1) != 0.0f) ? ship.getComponentArmorHitpointsCurrent(ShipChassisSlotType::SCST_armor_1) / ship.getComponentArmorHitpointsMaximum(ShipChassisSlotType::SCST_armor_1) : 0.0f;
		float const chassisRatio = (ship.getMaximumChassisHitPoints() != 0.0f) ? ship.getCurrentChassisHitPoints() / ship.getMaximumChassisHitPoints() : 0.0f;
		
		float const componentHitpointsCurrent = ship.getComponentHitpointsCurrent(slot);
		float const componentHitpointsMax = ship.getComponentHitpointsMaximum(slot);
		float const componentArmorCurrent = ship.getComponentArmorHitpointsCurrent(slot);
		float const componentArmorMax = ship.getComponentArmorHitpointsMaximum(slot);
		
		float const componentRatioArmor = (componentArmorMax != 0.0f) ? componentArmorCurrent / componentArmorMax : 0.0f;
		float const componentRatioHitpoints = (componentHitpointsMax != 0.0f) ? componentHitpointsCurrent / componentHitpointsMax : 0.0f;

		float componentRatio = 0.0f;

		if(componentArmorMax > 0.0f || componentHitpointsMax > 0.0f)
			componentRatio = ((componentRatioArmor * componentArmorMax) + (componentRatioHitpoints * componentHitpointsMax)) / (componentArmorMax + componentHitpointsMax);

		float const shieldsAverageClamped = (clamp(0.0f, shieldsFrontRatio, 1.0f) + clamp(0.0f, shieldsBackRatio, 1.0f)) / 2.0f;
		float const armorAverageClamped = (clamp(0.0f, armorFrontRatio, 1.0f) + clamp(0.0f, armorBackRatio, 1.0f)) / 2.0f;

		float totalCurrent = 0.0f;
		float totalMaximum = 0.0f;
		float averageSystemRatio = 0.0f;
		//calculate average system percent
		for(int i = ShipChassisSlotType::SCST_first; i < ShipChassisSlotType::SCST_invalid; ++i)
		{
			if(ship.isSlotInstalled(i))
			{
				if(i != ShipChassisSlotType::SCST_armor_0 && i != ShipChassisSlotType::SCST_armor_1)
				{
					totalCurrent += ship.getComponentHitpointsCurrent(i);
					totalMaximum += ship.getComponentHitpointsMaximum(i);
				}
			}
		}

		if(totalMaximum > 0.0f)
		{
			averageSystemRatio = totalCurrent /= totalMaximum;
		}

		float const systemRatioClamped = clamp(0.0f, averageSystemRatio, 1.0f);
		float const chassisRatioClamped = clamp(0.0f, chassisRatio, 1.0f);
		float const componentRatioClamped = clamp(0.0f, componentRatio, 1.0f);

		int const shieldsAveragePercent = static_cast<int>(shieldsAverageClamped * 100.0f);
		int const armorAveragePercent = static_cast<int>(armorAverageClamped * 100.0f);
		int const systemRatioPercent = static_cast<int>(systemRatioClamped * 100.0f);
		int const chassisRatioPercent = static_cast<int>(chassisRatioClamped * 100.0f);
		int const componentRatioPercent = static_cast<int>(componentRatioClamped * 100.0f);

		const int got = ship.getGameObjectType();
		const bool isMiningAsteroid = got == SharedObjectTemplate::GOT_ship_mining_asteroid_static || got == SharedObjectTemplate::GOT_ship_mining_asteroid_dynamic;

		char buffer[64];
		if(m_shieldsPercent)
		{
			if (isMiningAsteroid)
			{
				m_shieldsPercent->SetVisible(false);
			}
			else
			{
				m_shieldsPercent->SetVisible(true);

				if(ship.getShieldHitpointsFrontMaximum() <= 0.0f && ship.getShieldHitpointsBackMaximum() <= 0.0f)
				{
					m_shieldsPercent->SetLocalText(CuiStringIdsShipView::na.localize());
				}
				else
				{
					snprintf (buffer, 63,  "%d%%", shieldsAveragePercent);
					m_shieldsPercent->SetLocalText(Unicode::narrowToWide(buffer));
				}
			}
		}
		if(m_armorPercent)
		{
			if (isMiningAsteroid)
			{
				m_armorPercent->SetVisible(false);
			}
			else
			{
				m_armorPercent->SetVisible(true);

				if(ship.getComponentArmorHitpointsMaximum(ShipChassisSlotType::SCST_armor_0) <= 0.0f && ship.getComponentArmorHitpointsMaximum(ShipChassisSlotType::SCST_armor_1) <= 0.0f)
				{
					m_armorPercent->SetLocalText(CuiStringIdsShipView::na.localize());
				}
				else
				{
					snprintf (buffer, 63,  "%d%%", armorAveragePercent);
					m_armorPercent->SetLocalText(Unicode::narrowToWide(buffer));
				}
			}
		}
		if(m_systemPercent)
		{
			if (isMiningAsteroid)
			{
				m_systemPercent->SetVisible(false);
			}
			else
			{				
				m_systemPercent->SetVisible(true);

				snprintf (buffer, 63,  "%d%%", systemRatioPercent);
				m_systemPercent->SetLocalText(Unicode::narrowToWide(buffer));
			}
		}
		if(m_chassisPercent)
		{
			if (isMiningAsteroid)
			{
				m_chassisPercent->SetVisible(false);
			}
			else
			{
				m_chassisPercent->SetVisible(true);

				if(ship.getMaximumChassisHitPoints() <= 0.0f)
				{
					m_chassisPercent->SetLocalText(CuiStringIdsShipView::na.localize());
				}
				else
				{
					snprintf (buffer, 63,  "%d%%", chassisRatioPercent);
					m_chassisPercent->SetLocalText(Unicode::narrowToWide(buffer));
				}
			}
		}

		//set the selected component
		if (m_textComponent)
		{
			if (isMiningAsteroid)
			{
				m_textComponent->SetVisible(false);
			}
			else
			{
				m_textComponent->SetVisible(true);

				if(slot == ShipChassisSlotType::SCST_invalid || !isSlotTargetable)
				{
					m_textComponent->Clear(); 
				}
				else
				{
					std::string const & slotName = ShipChassisSlotType::getNameFromType (slot);
					StringId sid("ship_slot_n", slotName);
					Unicode::String text = sid.localize();
					snprintf (buffer, 63,  " %d%%", componentRatioPercent);
					text += Unicode::narrowToWide(buffer);
					m_textComponent->SetLocalText(text); 
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiStatusSpace::set3DObject(Object * object)
{
	if (m_objectViewer)
	{
		Object const * const lastObject = m_lastRenderObject.getObject();
		
		if (lastObject != object)
		{
			if (object)
			{
				m_objectViewer->SetDragable(false);
				m_objectViewer->setRotateSpeed(1.0f);
				m_objectViewer->setCameraForceTarget(true);
				m_objectViewer->setCameraLookAtCenter(true);
				m_objectViewer->setCameraAutoZoom(true);
				m_objectViewer->setAlterObjects(false);
				m_objectViewer->setUseOverrideShader(cs_membraneShader, true);
			}
			
			m_lastRenderObject = object ? object->getNetworkId() : CachedNetworkId::cms_invalid;
			
			m_objectViewer->setObject(object);
		}
	}
	else
	{
		m_lastRenderObject = CachedNetworkId::cms_invalid;
	}
}

//----------------------------------------------------------------------

void SwgCuiStatusSpace::updateButtonBar(TangibleObject const * const tangible)
{
	ShipObject const * const targetShip = tangible ? tangible->asShipObject() : NULL;

	//------------------------------------------------------------------
	// Update the context sensitive buttons.
	
	bool canFollow = targetShip && targetShip->isPlayerControlled();
	widgetSetVisible(m_canFollow, canFollow);
	widgetSetVisible(m_canMatchSpeed, canFollow);
	
	// Update the condition widgets.
	ConditionMap::iterator const itEnd = m_targetConditionWidgets->end();
	ConditionMap::iterator itCondition = m_targetConditionWidgets->begin();
	for(;itCondition != itEnd; ++itCondition)
	{
		widgetSetVisible(itCondition->second, targetShip && targetShip->hasCondition(itCondition->first));
	}
}


//======================================================================
