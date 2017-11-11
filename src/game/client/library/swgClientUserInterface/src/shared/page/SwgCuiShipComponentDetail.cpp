//======================================================================
//
// SwgCuiShipComponentDetail.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"


#include "swgClientUserInterface/SwgCuiShipComponentDetail.h"

#include "clientGame/ObjectAttributeManager.h"
#include "clientGame/ShipObjectAttachments.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiStringIdsShipComponent.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipChassisSlot.h"
#include "sharedGame/ShipChassisSlotType.h"
#include "sharedGame/ShipComponentData.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/Appearance.h"
#include "UIButton.h"
#include "UIComposite.h"
#include "UIImage.h"
#include "UIPage.h"
#include "UIText.h"

#include <list>

//======================================================================

namespace SwgCuiShipComponentDetailNamespace
{
	std::string const cs_membraneShader("shader/ui_membrane.sht");
	Unicode::String const cms_space(Unicode::narrowToWide(" "));
	Unicode::String const cms_colon(Unicode::narrowToWide(":"));
	Unicode::String const cms_newline(Unicode::narrowToWide("\n"));

	UILowerString cms_slotNameProperty("slotname");

	std::vector<UIButton *> ms_slotButtons;
}

using namespace SwgCuiShipComponentDetailNamespace;

//======================================================================

SwgCuiShipComponentDetail::SwgCuiShipComponentDetail (UIPage & page) :
CuiMediator ("SwgCuiShipComponentDetail", page),
UIEventCallback (),
m_callback (new MessageDispatch::Callback),
m_ship(NULL),
m_shipViewer(NULL),
m_detailsPage(NULL),
m_indicatorLine(NULL),
m_slotsComposite(NULL),
m_slotSample(NULL),
m_slotInfoText(NULL),
m_slotName(NULL),
m_healthBar(NULL),
m_healthBarHolster(NULL),
m_shipInfoText(NULL),
m_selectedSlotName(),
m_damageText(NULL)
{
	UIWidget * wid = 0;
	//get the ship viewer (paperdoll)
	getCodeDataObject (TUIWidget, wid, "shipviewer");
	m_shipViewer = safe_cast<CuiWidget3dObjectListViewer *>(wid);
	m_shipViewer->setAlterObjects(false);

	getCodeDataObject (TUIPage, m_detailsPage, "sampledetails");
	m_detailsPage->SetVisible(false);

	getCodeDataObject (TUIPage, m_indicatorLine, "indicator");
	m_indicatorLine->SetVisible(false);

	getCodeDataObject (TUIComposite, m_slotsComposite, "slots");
	m_slotsComposite->Clear();
	
	getCodeDataObject (TUIPage, m_slotSample, "slotsample");
	m_slotSample->SetVisible(false);

	getCodeDataObject (TUIText, m_slotInfoText, "slotinfotext");
	m_slotInfoText->Clear();

	getCodeDataObject (TUIText, m_slotName, "slotname");
	m_slotName->Clear();

	getCodeDataObject (TUIPage, m_healthBar, "healthbar");

	getCodeDataObject (TUIPage, m_healthBarHolster, "healthbarholster");

	getCodeDataObject (TUIText, m_shipInfoText, "shipinfotext");
	m_shipInfoText->Clear();

	getCodeDataObject (TUIText, m_damageText, "damagetext");
	m_damageText->Clear();

	setState (MS_closeable);
	setState (MS_closeDeactivates);
}

//----------------------------------------------------------------------

SwgCuiShipComponentDetail::~SwgCuiShipComponentDetail ()
{
	clearUI();

	m_ship = NULL;
	m_shipViewer = NULL;
	m_detailsPage = NULL;
	m_indicatorLine = NULL;
	m_slotsComposite = NULL;
	m_slotSample = NULL;
	m_slotInfoText = NULL;
	m_slotName = NULL;
	m_healthBar = NULL;
	m_healthBarHolster = NULL;
	m_shipInfoText = NULL;
	m_damageText = NULL;

	m_selectedSlotName.clear();

	delete m_callback;
	m_callback = 0;
}

//----------------------------------------------------------------------

void SwgCuiShipComponentDetail::performActivate ()
{
	populateUI();
	CuiManager::requestPointer (true);
	m_shipViewer->setPaused (false);
	setIsUpdating (true);

	/*for(std::vector<UIButton *>::iterator i = ms_slotButtons.begin(); i != ms_slotButtons.end(); ++i)
	{
		UIButton * const b = *i;
		if(b)
		{
			b->AddCallback(this);
		}
	}*/

	updateShipSelectionVisuals();
}

//----------------------------------------------------------------------

void SwgCuiShipComponentDetail::performDeactivate ()
{	
	for(std::vector<UIButton *>::iterator i = ms_slotButtons.begin(); i != ms_slotButtons.end(); ++i)
	{
		UIButton * const b = *i;
		if(b)
		{
			b->RemoveCallback(this);
		}
	}
	clearUI();

	m_shipViewer->setPaused (true);
	CuiManager::requestPointer (false);
	setIsUpdating (false);
}

//----------------------------------------------------------------------

void SwgCuiShipComponentDetail::OnButtonPressed   (UIWidget * const context)
{
	UIButton const * const button = dynamic_cast<UIButton const *>(context);
	if(button)
	{
		Unicode::String slotName;
		bool const & result = button->GetProperty(cms_slotNameProperty, slotName);
		if(result)
		{
			m_detailsPage->SetVisible(true);

			m_selectedSlotName = Unicode::wideToNarrow(slotName);
			updateShipSelectionVisuals();
		}
	}
}	

//----------------------------------------------------------------------

void SwgCuiShipComponentDetail::updateShipSelectionVisuals() const
{
	if(!m_ship)
		return;
#if 0
//TODO figure out how to accent the selected components (alpha, hue, etc.)
	if(!m_selectedSlotName.empty())
	{
		ShipObjectAttachments & attachmentsObject = m_ship->getShipObjectAttachments();
		ShipObjectAttachments::WatcherVectorMap const * const attachmentsMap = attachmentsObject.getAllComponentAttachments();
		ShipObjectAttachments::WatcherVector attachmentsVector;
		if(attachmentsMap)
		{
			for(ShipObjectAttachments::WatcherVectorMap::const_iterator i = attachmentsMap->begin(); i != attachmentsMap->end(); ++i)
			{
				ShipObjectAttachments::WatcherVector const & watcherVector = i->second;
				for(ShipObjectAttachments::WatcherVector::const_iterator j = watcherVector.begin(); j != watcherVector.end(); ++j)
				{
					attachmentsVector.push_back(*j);
				}
			}
		}

		ShipChassisSlotType::Type const & slotType = ShipChassisSlotType::getTypeFromName(m_selectedSlotName);
		ShipObjectAttachments::WatcherVector const * const attachmentsForSelectedComponent = attachmentsObject.getComponentAttachments(slotType);
		if(attachmentsForSelectedComponent)
		{
			for(ShipObjectAttachments::WatcherVector::const_iterator k = attachmentsVector.begin(); k != attachmentsVector.end(); ++k)
			{
				Object const * const shipComponent = k->getPointer();
				bool isASelectedComponent = false;
				for(ShipObjectAttachments::WatcherVector::const_iterator l = attachmentsForSelectedComponent->begin(); l != attachmentsForSelectedComponent->end(); ++l)
				{
					Object const * const selectedComponent = l->getPointer();
					if(shipComponent == selectedComponent)
						isASelectedComponent = true;
				}
				Appearance * const appearance = const_cast<Appearance *>(shipComponent->getAppearance());
				if(appearance)
				{
					if(isASelectedComponent)
					{
						appearance->setAlpha(true, 0.75f, true, 0.25f);
					}
					else
					{
						appearance->setAlpha(true, 0.25f, true, 0.75f);
					}
				}
			}
		}
	}
#endif
}

//----------------------------------------------------------------------

void SwgCuiShipComponentDetail::setShip(ShipObject * const ship)
{
	DEBUG_WARNING(!ship, ("SwgCuiShipComponentDetail::setShip called with a null ship"));

	if(ship == m_ship.getPointer())
		return;

	if(ship)
	{
		m_ship = ship;

		m_shipViewer->clearObjects();
		m_shipViewer->addObject(*m_ship);
		m_shipViewer->setViewDirty(true);
		m_shipViewer->setAutoZoomOutOnly(false);
		m_shipViewer->setCameraZoomInWhileTurn(false);
		m_shipViewer->setAlterObjects(false);
		m_shipViewer->setCameraLookAtCenter(true);
		m_shipViewer->setCameraPitch(PI / 2.0f);
		m_shipViewer->setCameraYaw(0.0f);
		m_shipViewer->setDragYawOk(false);	
		m_shipViewer->SetDragable(false);	
		m_shipViewer->SetContextCapable(true, false);
		m_shipViewer->setRotateSpeed(0.0f);
		m_shipViewer->setCameraTransformToObj(true);
		m_shipViewer->setCameraLodBias(3.0f);
		m_shipViewer->setCameraLodBiasOverride(true);
		m_shipViewer->setCameraForceTarget(true);
		m_shipViewer->recomputeZoom();
		m_shipViewer->setCameraForceTarget(false);
		m_shipViewer->setUseOverrideShader(cs_membraneShader, true);
		m_shipViewer->setViewDirty(true);
	}

	if(isActive())
		populateUI();
}

//----------------------------------------------------------------------

void SwgCuiShipComponentDetail::populateUI()
{
	if(!m_ship)
		return;

	clearUI();

	m_slotsComposite->Clear();

	ShipChassis const * const shipChassis = ShipChassis::findShipChassisByCrc (m_ship->getChassisType());
	if (shipChassis)
	{
		ShipChassis::SlotVector const & slots = shipChassis->getSlots ();
		for(ShipChassis::SlotVector::const_iterator i = slots.begin(); i != slots.end(); ++i)
		{
			ShipChassisSlot const & slot = *i;
			if(m_ship->isSlotInstalled(slot.getSlotType()))
			{
				std::string const & name = ShipChassisSlotType::getNameFromType (slot.getSlotType());
				UIPage * const newSlotPage = dynamic_cast<UIPage *>(m_slotSample->DuplicateObject());
				if(newSlotPage)
				{
					UIButton * const newSlotButton = getButtonFromSlotPage(*newSlotPage);
					if(newSlotButton)
					{
						IGNORE_RETURN(newSlotButton->SetProperty(cms_slotNameProperty, Unicode::narrowToWide(name)));
						newSlotButton->SetText(slot.getLocalizedSlotName());
						newSlotButton->SetTooltip(slot.getLocalizedSlotDescription());
						newSlotButton->Attach (0);
						if(isActive())
							newSlotButton->AddCallback(this);
						ms_slotButtons.push_back(newSlotButton);
					}
					UIImage * const hitpointsImage = getHitpointsImageFromSlotPage(*newSlotPage);
					if(hitpointsImage)
						hitpointsImage->SetVisible(false);
					UIImage * const unpoweredImage = getUnpoweredImageFromSlotPage(*newSlotPage);
					if(unpoweredImage)
						unpoweredImage->SetVisible(false);
					UIImage * const disabledImage = getDisabledImageFromSlotPage(*newSlotPage);
					if(disabledImage)
						disabledImage->SetVisible(false);
					newSlotPage->SetVisible(true);
					m_slotsComposite->AddChild(newSlotPage);
				}
			}
		}
		m_slotsComposite->Link();
	}
}

//----------------------------------------------------------------------

void SwgCuiShipComponentDetail::clearUI()
{
	for(std::vector<UIButton *>::iterator i = ms_slotButtons.begin(); i != ms_slotButtons.end(); ++i)
	{
		UIButton * const b = *i;
		if(b)
		{
			if(isActive())
				b->RemoveCallback(this);

			UIBaseObject * const bo = b->GetParent();
			if(bo && bo->IsA(TUIPage))
			{
				UIPage * const p = safe_cast<UIPage *>(bo);
				if(p)
					p->RemoveChild(b);
			}
			b->Detach (0);
		}
	}
	ms_slotButtons.clear();
}

//----------------------------------------------------------------------

UIButton * SwgCuiShipComponentDetail::getButtonFromSlotPage(UIPage const & slotPage) const
{
	return safe_cast<UIButton *>(slotPage.GetChild("sampleButton"));
}

//----------------------------------------------------------------------

UIImage * SwgCuiShipComponentDetail::getHitpointsImageFromSlotPage(UIPage const & slotPage) const
{
	return safe_cast<UIImage *>(slotPage.GetChild("hitpoints"));
}

//----------------------------------------------------------------------

UIImage * SwgCuiShipComponentDetail::getUnpoweredImageFromSlotPage(UIPage const & slotPage) const
{
	return safe_cast<UIImage *>(slotPage.GetChild("unpowered"));
}

//----------------------------------------------------------------------

UIImage * SwgCuiShipComponentDetail::getDisabledImageFromSlotPage(UIPage const & slotPage) const
{
	return safe_cast<UIImage *>(slotPage.GetChild("disabled"));
}

//----------------------------------------------------------------------

void SwgCuiShipComponentDetail::updateUI()
{
	if(!m_ship)
		return;

	m_shipInfoText->Clear();
	Unicode::String newText;
	StringId noUnits;

	//max speed
	appendShipItemToText(newText, CuiStringIdsShipComponent::engine_speed_max,  m_ship->getEngineSpeedMaximum(), noUnits);
	newText += cms_newline;
	//yaw/pitch/roll accel and max rate
	appendShipItemToText(newText, CuiStringIdsShipComponent::engine_yaw_accel,  m_ship->getEngineYawAccelerationRate(), noUnits);
	appendShipItemToText(newText, CuiStringIdsShipComponent::engine_pitch_accel,  m_ship->getEnginePitchAccelerationRate(), noUnits);
	appendShipItemToText(newText, CuiStringIdsShipComponent::engine_roll_accel,  m_ship->getEngineRollAccelerationRate(), noUnits);
	appendShipItemToText(newText, CuiStringIdsShipComponent::engine_yaw_max,  m_ship->getEngineYawRateMaximum(), noUnits);
	appendShipItemToText(newText, CuiStringIdsShipComponent::engine_pitch_max,  m_ship->getEnginePitchRateMaximum(), noUnits);
	appendShipItemToText(newText, CuiStringIdsShipComponent::engine_roll_max,  m_ship->getEngineRollRateMaximum(), noUnits);
	newText += cms_newline;
	//chassis current/max
	appendShipItemToText(newText, CuiStringIdsShipComponent::chassis_hp,  m_ship->getCurrentChassisHitPoints(), noUnits);
	appendShipItemToText(newText, CuiStringIdsShipComponent::chassis_hp_max,  m_ship->getMaximumChassisHitPoints(), noUnits);
	newText += cms_newline;
	//shield current/max and recharge rate
	appendShipItemToText(newText, CuiStringIdsShipComponent::shield_front_hp,  m_ship->getShieldHitpointsFrontCurrent(), noUnits);
	appendShipItemToText(newText, CuiStringIdsShipComponent::shield_front_hp_max,  m_ship->getShieldHitpointsFrontMaximum(), noUnits);
	appendShipItemToText(newText, CuiStringIdsShipComponent::shield_back_hp,  m_ship->getShieldHitpointsBackCurrent(), noUnits);
	appendShipItemToText(newText, CuiStringIdsShipComponent::shield_back_hp_max,  m_ship->getShieldHitpointsBackMaximum(), noUnits);
	appendShipItemToText(newText, CuiStringIdsShipComponent::shield_recharge_rate,  m_ship->getShieldRechargeRate() * m_ship->getComponentEfficiencyGeneral(ShipChassisSlotType::SCST_shield_0), noUnits);
	newText += cms_newline;
	//chassis current/max
	appendShipItemToText(newText, CuiStringIdsShipComponent::mass,  m_ship->getChassisComponentMassCurrent(), noUnits);
	appendShipItemToText(newText, CuiStringIdsShipComponent::mass_max,  m_ship->getChassisComponentMassMaximum(), noUnits);
	newText += cms_newline;
	//chassis current/max/recharge
	float const capacitorEnergyMaximum = m_ship->getCapacitorEnergyMaximum () * m_ship->getComponentEfficiencyGeneral(ShipChassisSlotType::SCST_capacitor);
	appendShipItemToText(newText, CuiStringIdsShipComponent::capacitor, m_ship->getCapacitorEnergyCurrent(), noUnits);
	appendShipItemToText(newText, CuiStringIdsShipComponent::capacitor_max, capacitorEnergyMaximum, noUnits);
	appendShipItemToText(newText, CuiStringIdsShipComponent::capacitor_recharge_rate, m_ship->getCapacitorEnergyRechargeRate() * m_ship->getComponentEfficiencyGeneral(ShipChassisSlotType::SCST_capacitor), noUnits);

	newText += cms_newline;
	m_shipInfoText->SetText(newText);

	//update selected slot healthbar
	if(!m_selectedSlotName.empty())
	{
		ShipChassisSlotType::Type const & slotType = ShipChassisSlotType::getTypeFromName(m_selectedSlotName);
		ShipComponentData * const data = m_ship->createShipComponentData(slotType);
		if(data)
		{
			std::vector<std::pair<std::string, Unicode::String> > attribs;
			data->getAttributes(attribs);
			Unicode::String result;
			ObjectAttributeManager::formatAttributes(attribs, result, NULL, NULL, false);
			m_slotInfoText->SetLocalText(result);
			ShipChassis const * const shipChassis = ShipChassis::findShipChassisByCrc (m_ship->getChassisType());
			if (shipChassis)
			{
				ShipChassisSlot const * const slot = shipChassis->getSlot(slotType);
				if(slot)
				{
					m_slotName->SetText(m_ship->getComponentName(slotType));
					if(data->m_hitpointsCurrent >= 0.0f)
					{
						float const ratio = data->m_hitpointsCurrent / data->m_hitpointsMaximum;
						long const holsterLength = m_healthBarHolster->GetWidth ();
						m_healthBar->SetWidth (static_cast<long>(holsterLength * ratio));
						float const ratioPercent = ratio * 100.0f;
						char buffer[20];
						sprintf (buffer, "%.0f%%", ratioPercent);
						m_damageText->SetLocalText(Unicode::narrowToWide(buffer));
					}
				}
			}
			delete data;
		}
	}

	//show or hide status icons per slot
	std::list<UIBaseObject *> children;
	m_slotsComposite->GetChildren(children);
	for(std::list<UIBaseObject *>::iterator i = children.begin(); i != children.end(); ++i)
	{
		UIPage * const page = safe_cast<UIPage *>(*i);
		if(page)
		{
			UIButton const * const button = getButtonFromSlotPage(*page);
			if(button)
			{
				Unicode::String slotName;
				bool const & result = button->GetProperty(cms_slotNameProperty, slotName);
				if(result)
				{
					//show or hide the low hitpoints icon
					ShipChassisSlotType::Type const & slotType = ShipChassisSlotType::getTypeFromName(Unicode::wideToNarrow(slotName));
					if(m_ship->getComponentHitpointsMaximum(slotType) > 0.0f)
					{
						float const hitpointsRatio = m_ship->getComponentHitpointsCurrent(slotType) / m_ship->getComponentHitpointsMaximum(slotType);
						UIImage * const hitpointsImage = getHitpointsImageFromSlotPage(*page);
						if(hitpointsRatio < 0.25f)
							hitpointsImage->SetVisible(true);
						else
							hitpointsImage->SetVisible(false);
					}

					//show or hide the low power icon
					UIImage * const unpoweredImage = getUnpoweredImageFromSlotPage(*page);
					if(unpoweredImage)
					{
						if(m_ship->isComponentLowPower(slotType))
							unpoweredImage->SetVisible(true);
						else
							unpoweredImage->SetVisible(false);
					}

					//show or hide the disabled icon
					UIImage * const disabledImage = getDisabledImageFromSlotPage(*page);
					if(disabledImage)
					{
						if(m_ship->isComponentDisabled(slotType))
							disabledImage->SetVisible(true);
						else
							disabledImage->SetVisible(false);
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

/** Given a text item, a prelabel and a value, append the following to text
    "<localized-preLabel> value <localized-units>\n"
*/
void SwgCuiShipComponentDetail::appendShipItemToText(Unicode::String & text, StringId const & preLabel, float const value, StringId const & units) const
{
	text += preLabel.localize() + cms_colon + cms_space;
	char buf[256];
	sprintf (buf, "%.1f", value);
	text += Unicode::narrowToWide (buf);

	//add units if we have a good stringId
	if(!units.isInvalid())
		text += cms_space + units.localize();
	
	text += cms_newline;
}

//----------------------------------------------------------------------

void SwgCuiShipComponentDetail::update (float const deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	updateUI();
}

//======================================================================
