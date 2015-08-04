//======================================================================
//
// SwgCuiShipView.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiShipView.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/Game.h"
#include "clientGame/GroupObject.h"
#include "clientGame/GroupManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsShipComponent.h"
#include "clientUserInterface/CuiStringIdsShipView.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipChassisSlot.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgClientUserInterface/SwgCuiSpaceGroupLaunch.h"
#include "swgClientUserInterface/SwgCuiTicketPurchase.h"
#include "UIButton.h"
#include "UIText.h"

//======================================================================

namespace SwgCuiShipViewNamespace
{
	NetworkId m_terminal;
	float m_timer = 0.0f;

	float const cms_maxRangeFromTerminal = 16.0f;
	float const cms_updateTime = 1.0f;

	std::string intToString(int const value);
}

using namespace SwgCuiShipViewNamespace;

//----------------------------------------------------------------------

std::string SwgCuiShipViewNamespace::intToString(int const value)
{
	char buffer[16];
	snprintf(buffer, sizeof(buffer) - 1, "%d", value);
	return buffer;
}

//======================================================================

SwgCuiShipView::SwgCuiShipView (UIPage & page) :
CuiMediator ("SwgCuiShipView", page),
UIEventCallback (),
m_callback (new MessageDispatch::Callback),
m_ship(NULL),
m_shipViewer(NULL),
m_title(NULL),
m_statsAttribs(NULL),
m_statsValues(NULL),
m_manageButton(NULL),
m_groupButton(NULL),
m_launchButton(NULL),
m_travelButton(NULL)
{
	UIWidget * wid = 0;
	//get the ship viewer (paperdoll)
	getCodeDataObject (TUIWidget, wid, "shipviewer");
	m_shipViewer = safe_cast<CuiWidget3dObjectListViewer *>(wid);
	m_shipViewer->setAlterObjects(false);

	getCodeDataObject (TUIText, m_title, "title");
	m_title->Clear();
	m_title->SetPreLocalized(true);

	getCodeDataObject (TUIText, m_statsAttribs, "statsattribs");
	m_statsAttribs->Clear();
	m_statsAttribs->SetPreLocalized(true);

	getCodeDataObject (TUIText, m_statsValues, "statsvalues");
	m_statsValues->Clear();
	m_statsValues->SetPreLocalized(true);

	getCodeDataObject (TUIButton, m_manageButton, "buttonmanage");
	registerMediatorObject(*m_manageButton, true);

	getCodeDataObject (TUIButton, m_groupButton, "buttongroup");
	registerMediatorObject(*m_groupButton, true);

	getCodeDataObject (TUIButton, m_launchButton, "buttonlaunch");
	registerMediatorObject(*m_launchButton, true);

	getCodeDataObject (TUIButton, m_travelButton, "buttontravel");
	registerMediatorObject(*m_travelButton, true);

	clearUi();

	setState (MS_closeable);

	SwgCuiSpaceGroupLaunch::resetAcceptedMembers();
}

//----------------------------------------------------------------------

SwgCuiShipView::~SwgCuiShipView ()
{
	clearUi();

	m_ship = NULL;
	m_shipViewer = NULL;
	m_title = NULL;
	m_statsAttribs = NULL;
	m_statsValues = NULL;
	m_manageButton = NULL;
	m_groupButton = NULL;
	m_launchButton = NULL;
	m_travelButton = NULL;

	delete m_callback;
	m_callback = 0;
}

//----------------------------------------------------------------------

void SwgCuiShipView::performActivate ()
{
	CuiManager::requestPointer (true);
	m_shipViewer->setPaused (false);
	populateUi();

	setIsUpdating(true);

	m_callback->connect(*this, &SwgCuiShipView::onComponentsChanged, static_cast<ShipObject::Messages::ComponentsChanged *>(0));

	GroupManager::openedLaunchIntoSpaceUI();
}

//----------------------------------------------------------------------

void SwgCuiShipView::performDeactivate ()
{
	CuiMediator * const mediator = CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::WS_SpaceGroupLaunch);
	if (mediator)
	{
		SwgCuiSpaceGroupLaunch * const launch = static_cast<SwgCuiSpaceGroupLaunch *>(mediator);
		if (launch)
		{
			launch->closeThroughWorkspace();
		}
	}

	m_callback->disconnect(*this, &SwgCuiShipView::onComponentsChanged, static_cast<ShipObject::Messages::ComponentsChanged *>(0));
	m_shipViewer->setPaused (true);

	setIsUpdating(false);

	CuiManager::requestPointer (false);

	GroupManager::closedLaunchIntoSpaceUI();
}

//-----------------------------------------------------------------------

void SwgCuiShipView::update (float const deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	m_timer += deltaTimeSecs;
	if(m_timer >= cms_updateTime)
	{
		m_timer = 0.0f;
		populateUiText();
	}
}

//-----------------------------------------------------------------------

void SwgCuiShipView::OnButtonPressed(UIWidget * const context)
{
	if(context == m_manageButton)
	{
		if(!m_ship)
			return;

		char buf[256];
		_snprintf (buf, 255, "%s %s", m_ship->getNetworkId().getValueString().c_str(), m_terminal.getValueString().c_str());
		IGNORE_RETURN(CuiActionManager::performAction(CuiActions::manageShipComponents, Unicode::narrowToWide(buf)));
	}
	else if(context == m_groupButton)
	{
		CuiMediator * const mediator = CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::WS_SpaceGroupLaunch);
		SwgCuiSpaceGroupLaunch * const launch = static_cast<SwgCuiSpaceGroupLaunch *>(mediator);
		if (launch != 0)
		{
			launch->setShip(m_ship);
		}
		NOT_NULL(launch);
	}
	else if(context == m_launchButton)
	{
		if(!m_ship)
			return;

		if(!m_terminal.isValid())
		{
			WARNING(true, ("SwgCuiShipView launch invalid terminal [%s]", m_terminal.getValueString().c_str()));
			return;
		}

		ContainedByProperty const* const containedBy = ContainerInterface::getContainedByProperty(*m_ship);
		if(!containedBy)
		{
			WARNING(true, ("SwgCuiShipView launch invalid ship contained by"));
			return;
		}

		Unicode::String errorMsg;

		ShipObject::ShipLaunchable const launchable = m_ship->getShipLaunchable();
		if(launchable == ShipObject::SL_noReactor)
		{
			errorMsg = CuiStringIdsShipView::condition_reactor_uninstalled.localize();
		}
		else if(launchable == ShipObject::SL_reactorDisabled)
		{
			errorMsg = CuiStringIdsShipView::condition_reactor_disabled.localize();
		}
		else if(launchable == ShipObject::SL_noEngine)
		{
			errorMsg = CuiStringIdsShipView::condition_engine_uninstalled.localize();
		}
		else if(launchable == ShipObject::SL_engineDisabled)
		{
			errorMsg = CuiStringIdsShipView::condition_engine_disabled.localize();
		}

		if(!errorMsg.empty())
		{
			CuiMessageBox::createInfoBox (errorMsg);
			return;
		}

		SwgCuiSpaceGroupLaunch::NetworkIdSet const & acceptedMembers = SwgCuiSpaceGroupLaunch::getAcceptedMembers();
		SwgCuiSpaceGroupLaunch::NetworkIdSet::const_iterator ii = acceptedMembers.begin();
		SwgCuiSpaceGroupLaunch::NetworkIdSet::const_iterator iiEnd = acceptedMembers.end();

		std::string parameterString = containedBy->getContainedByNetworkId().getValueString();
		parameterString += " " + intToString(acceptedMembers.size());

		for (; ii != iiEnd; ++ii)
		{
			NetworkId const & Id = *ii;
			parameterString += " " + Id.getValueString();
		}

		DEBUG_REPORT_LOG_PRINT(true, ("Requesting launch with terminal [%s] and params(%s)", m_terminal.getValueString().c_str(), parameterString.c_str()));

		ClientCommandQueue::enqueueCommand ("launchIntoSpace", m_terminal, Unicode::narrowToWide(parameterString));
		closeThroughWorkspace();
	}
	else if(context == m_travelButton)
	{
		SwgCuiTicketPurchase * const mediator = safe_cast<SwgCuiTicketPurchase *>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_TicketPurchase));
		if(mediator && m_ship)
		{
			mediator->setTravelType(SwgCuiTicketPurchase::TT_personalShip);
			mediator->setData(Game::getSceneId(), std::string());
			mediator->setTerminalId(m_terminal);

			ContainedByProperty const* const containedBy = ContainerInterface::getContainedByProperty(*m_ship);
			if(containedBy)
			{
				mediator->setShipControlDeviceId(containedBy->getContainedByNetworkId());
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiShipView::setShip(ShipObject * const ship)
{
	if(ship)
	{
		m_ship = ship;
		populateUi();
	}
}

//----------------------------------------------------------------------

void SwgCuiShipView::setTerminal(NetworkId const & nid)
{
	Object const * const o = NetworkIdManager::getObjectById(nid);
	ClientObject const * const co = o ? o->asClientObject() : NULL;
	if(co && (co->getGameObjectType() == SharedObjectTemplate::GOT_terminal_space))
	{
		m_terminal = nid;
		setAssociatedObjectId(m_terminal);
		setMaxRangeFromObject(cms_maxRangeFromTerminal);

		DEBUG_REPORT_LOG_PRINT(true, ("SwgCuiShipView - terminal is [%s].\n", m_terminal.getValueString().c_str()));
	}
	else
	{
		DEBUG_REPORT_LOG_PRINT(true, ("SwgCuiShipView - BAD terminal [%s].\n", nid.getValueString().c_str()));
	}
}

//----------------------------------------------------------------------

void SwgCuiShipView::clearUi()
{
	m_shipViewer->clearObjects ();
	m_title->Clear();
	m_statsAttribs->Clear();
	m_statsValues->Clear();
}

//----------------------------------------------------------------------

void SwgCuiShipView::populateUi()
{
	if(!m_ship)
		return;

	clearUi();
	setupViewer();
	populateUiText();
	populateGroupButton();
}

//----------------------------------------------------------------------

void SwgCuiShipView::setupViewer()
{
	if(!m_ship)
		return;

	m_shipViewer->clearObjects ();
	m_shipViewer->addObject (*m_ship);
	m_shipViewer->setViewDirty         (true);
	m_shipViewer->setCameraForceTarget (true);
	m_shipViewer->recomputeZoom ();
	m_shipViewer->setCameraForceTarget (false);
	m_shipViewer->setRotateSpeed(0.2f);
}

//----------------------------------------------------------------------

void SwgCuiShipView::populateUiText()
{
	if(!m_ship)
		return;

	//set the ship's name
	m_title->SetLocalText(m_ship->getLocalizedName());

	//set up the ship's detail text (a readout of each component's percentage)
	char buffer[64];
	Unicode::String resultAttribs;
	Unicode::String resultValues;
	float const chassisRatio = (m_ship->getMaximumChassisHitPoints() != 0.0f) ? m_ship->getCurrentChassisHitPoints() / m_ship->getMaximumChassisHitPoints() : 0.0f;
	float const chassisRatioClamped = clamp(0.0f, chassisRatio, 1.0f);
	int const chassisRatioPercent = static_cast<int>(chassisRatioClamped * 100.0f);
	resultAttribs += CuiStringIdsShipComponent::chassis.localize() + Unicode::narrowToWide(":\n");
	snprintf (buffer, 63, "%d%%\n", chassisRatioPercent);
	resultValues += Unicode::narrowToWide(buffer);

	ShipChassis const * const shipChassis = ShipChassis::findShipChassisByCrc (m_ship->getChassisType());
	if (shipChassis)
	{
		ShipChassis::SlotVector const & slots = shipChassis->getSlots ();
		for(ShipChassis::SlotVector::const_iterator i = slots.begin(); i != slots.end(); ++i)
		{
			ShipChassisSlot const & slot = *i;
			if(m_ship->isSlotInstalled(slot.getSlotType()))
			{
				ShipChassisSlotType::Type const & slotType = slot.getSlotType();
				Unicode::String const & slotName = slot.getLocalizedSlotName();
				resultAttribs += slotName + Unicode::narrowToWide(":\n");

				float const componentRatio = (m_ship->getComponentHitpointsMaximum(slotType) != 0.0f) ? m_ship->getComponentHitpointsCurrent(slotType) / m_ship->getComponentHitpointsMaximum(slotType) : 0.0f;
				float const componentRatioClamped = clamp(0.0f, componentRatio, 1.0f);
				int const componentRatioPercent = static_cast<int>(componentRatioClamped * 100.0f);
				snprintf (buffer, 63, "%d%%\n", componentRatioPercent);
				resultValues += Unicode::narrowToWide(buffer);
			}
		}
	}
	m_statsAttribs->SetLocalText(resultAttribs);
	m_statsValues->SetLocalText(resultValues);
}

//----------------------------------------------------------------------

void SwgCuiShipView::populateGroupButton()
{
	if (m_groupButton != 0)
	{
		if (m_ship != 0)
		{
			m_groupButton->SetEnabled(m_ship->isMultiPassenger());
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiShipView::onComponentsChanged (ShipObject::Messages::ComponentsChanged::Payload & ship)
{
	if(&ship == m_ship)
		populateUi();
} //lint !e1764 ship could be const (not it couldn't - need to match message payload)

//======================================================================
