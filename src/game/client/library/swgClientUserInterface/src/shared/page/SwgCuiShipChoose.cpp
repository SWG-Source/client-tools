//======================================================================
//
// SwgCuiShipChoose.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiShipChoose.h"

#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/Game.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsShipChoose.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/VolumeContainer.h"
#include "UIButton.h"
#include "UIComposite.h"
#include "UIText.h"

//======================================================================

namespace SwgCuiShipChooseNamespace
{
	float const cms_maxRangeFromTerminal = 16.0f;

	bool ms_closing = false;
	NetworkId ms_terminal;
	bool ms_needToBuildCompPages = true;

	namespace Properties
	{
		UILowerString const ShipNetworkId("ShipNetworkId");
	};//lint !e19 useless declaration (no, it's used)

	std::map<NetworkId, UIComposite *> ms_shipCompPages;
}

using namespace SwgCuiShipChooseNamespace;

//======================================================================

SwgCuiShipChoose::SwgCuiShipChoose (UIPage & page) :
CuiMediator ("SwgCuiShipChoose", page),
UIEventCallback (),
m_callback (new MessageDispatch::Callback),
m_shipComp(NULL),
m_sample(NULL),
m_closeButton(NULL),
m_parkingDataReceived(false)
{
	getCodeDataObject (TUIComposite, m_shipComp, "shipComp");
	m_shipComp->Clear();

	getCodeDataObject (TUIPage, m_sample, "sampleShip");

	getCodeDataObject (TUIButton, m_closeButton , "closeButton");
	registerMediatorObject(*m_closeButton, true);

	ms_shipCompPages.clear();
	ms_terminal = NetworkId::cms_invalid;
	ms_closing = false;
	ms_needToBuildCompPages = true;

	clearUI();

	setState (MS_closeable);
	m_callback->connect (*this, &SwgCuiShipChoose::onShipParkingDataReceived, static_cast<PlayerCreatureController::Messages::ShipParkingDataReceived *>(0));
}
//----------------------------------------------------------------------

SwgCuiShipChoose::~SwgCuiShipChoose ()
{
	m_callback->disconnect (*this, &SwgCuiShipChoose::onShipParkingDataReceived, static_cast<PlayerCreatureController::Messages::ShipParkingDataReceived *>(0));

	clearUI();

	m_closeButton = NULL;

	delete m_callback;
	m_callback = 0;
}

//----------------------------------------------------------------------

void SwgCuiShipChoose::performActivate ()
{
	ms_closing = false;
	CuiManager::requestPointer (true);
	if(ms_needToBuildCompPages)
	{
		ms_needToBuildCompPages = false;
		populateUI();
	}
}

//----------------------------------------------------------------------

void SwgCuiShipChoose::performDeactivate ()
{
	CuiManager::requestPointer (false);
}

//-----------------------------------------------------------------------

void SwgCuiShipChoose::OnButtonPressed(UIWidget * const context)
{
	if(context == m_closeButton)
		closeThroughWorkspace();
	else
	{
		UIButton * const button = dynamic_cast<UIButton *>(context);
		if(!button)
			return;

		if(ms_closing)
			return;

		UINarrowString shipIdStr;
		button->GetPropertyNarrow(Properties::ShipNetworkId, shipIdStr);
		NetworkId shipId(shipIdStr.c_str());
		if(!shipId.isValid())
			return;

		if(!PlayerObject::isAdmin() && !m_parkingDataReceived && ConfigClientGame::getValidateShipParkingLocation())
		{
			CuiMessageBox::createInfoBox (CuiStringIdsShipChoose::no_parking_data.localize ());
			return;
		}

		if(!ms_terminal.isValid())
			return;

		Object * const o = NetworkIdManager::getObjectById(shipId);
		ClientObject * const co = o ? o->asClientObject() : NULL;
		ShipObject * const so = co ? co->asShipObject() : NULL;
		if(so)
		{
			char buf[256];
			_snprintf (buf, 255, "%s %s", shipIdStr.c_str(), ms_terminal.getValueString().c_str());
			ms_closing = true;
			//shipView's params are "<shipId> <terminalId>"
			IGNORE_RETURN(CuiActionManager::performAction (CuiActions::shipView, Unicode::narrowToWide(buf)));
			closeThroughWorkspace();
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiShipChoose::setTerminal(NetworkId const & nid)
{
	Object const * const o = NetworkIdManager::getObjectById(nid);
	ClientObject const * const co = o ? o->asClientObject() : NULL;
	if(co && (co->getGameObjectType() == SharedObjectTemplate::GOT_terminal_space))
	{
		setAssociatedObjectId(nid);
		setMaxRangeFromObject(cms_maxRangeFromTerminal);
		ms_terminal = nid;
		DEBUG_REPORT_LOG_PRINT(true, ("SwgCuiShipChoose - terminal is [%s].\n", ms_terminal.getValueString().c_str()));
	}
	else
	{
		DEBUG_REPORT_LOG_PRINT(true, ("SwgCuiShipChoose - BAD terminal [%s].\n", nid.getValueString().c_str()));
	}
}

//----------------------------------------------------------------------

void SwgCuiShipChoose::clearUI()
{
	m_shipComp->Clear();
	ms_shipCompPages.clear();
}

//----------------------------------------------------------------------

void SwgCuiShipChoose::populateUI()
{
	clearUI();

	CreatureObject const * const player = Game::getPlayerCreature();
	if(!player)
		return;

	ClientObject const * const datapad = player->getDatapadObject();
	if(!datapad)
		return;

	VolumeContainer const * const datapadContainer = ContainerInterface::getVolumeContainer(*datapad);
	if(!datapadContainer)
		return;

	bool playerOwnsAShip = false;

	//for each item in the datapad, see if it contains a shipobject (that would make it a pcd)
	for(ContainerConstIterator i = datapadContainer->begin(); i != datapadContainer->end(); ++i)
	{
		ShipObject * ship = NULL;
		Object const * const item = (*i).getObject();
		if(item)
		{
			Container const * const itemContainer = ContainerInterface::getContainer(*item);
			if(itemContainer)
			{
				//see if the item contains a shipobject
				for(ContainerConstIterator i2 = itemContainer->begin(); i2 != itemContainer->end(); ++i2)
				{
					Object * const o = (*i2).getObject();
					ClientObject * const co = o ? o->asClientObject() : NULL;
					ShipObject * const so = co ? co->asShipObject() : NULL;
					if(so)
					{
						//refresh the object attribs, since we'll be displaying them
						ObjectAttributeManager::requestUpdate(so->getNetworkId());
						ship = so;
						playerOwnsAShip = true;
						break;
					}
				}
			}
		}

		if(ship)
		{
			addShipCompPage(*ship);
		}
	}

	if(!playerOwnsAShip)
	{
		CuiSystemMessageManager::sendFakeSystemMessage (CuiStringIdsShipChoose::dont_own_a_ship.localize());
		closeThroughWorkspace();
	}
}

//----------------------------------------------------------------------

void SwgCuiShipChoose::addShipCompPage(ShipObject & ship)
{
	//duplicate, set up ship page data
	UIComposite * const newShipPage = dynamic_cast<UIComposite *>(m_sample->DuplicateObject());
	if(!newShipPage)
		return;
	newShipPage->SetVisible(true);

	//set up viewer
	UIBaseObject * baseObject = newShipPage->GetChild("viewer.viewer");
	CuiWidget3dObjectListViewer * const viewer = safe_cast<CuiWidget3dObjectListViewer *>(baseObject);
	if(viewer)
	{
		viewer->setAlterObjects(false);
		viewer->clearObjects ();
		viewer->addObject (ship);
		viewer->setViewDirty         (true);
		viewer->setCameraForceTarget (true);
		viewer->recomputeZoom ();
		viewer->setCameraForceTarget (false);
		viewer->setRotateSpeed(0.2f);
	}

	//set ship name
	baseObject = newShipPage->GetChild("stats.title.name");
	UIText * const title = safe_cast<UIText *>(baseObject);
	if(title)
	{
		title->SetPreLocalized(true);
		title->SetLocalText(ship.getLocalizedName());
	}

	//set condition text
	baseObject = newShipPage->GetChild("stats.condition.text");
	UIText * const condition = safe_cast<UIText *>(baseObject);
	if(condition)
	{
		condition->Clear();
		condition->SetPreLocalized(true);

		Unicode::String conditionText;

		ShipObject::ShipLaunchable const launchable = ship.getShipLaunchable();
		if(launchable == ShipObject::SL_noReactor)
		{
			conditionText = CuiStringIdsShipChoose::condition_reactor_uninstalled.localize();
		}
		else if(launchable == ShipObject::SL_reactorDisabled)
		{
			conditionText = CuiStringIdsShipChoose::condition_reactor_disabled.localize();
		}
		else if(launchable == ShipObject::SL_noEngine)
		{
			conditionText = CuiStringIdsShipChoose::condition_engine_uninstalled.localize();
		}
		else if(launchable == ShipObject::SL_engineDisabled)
		{
			conditionText = CuiStringIdsShipChoose::condition_engine_disabled.localize();
		}
		else if(launchable == ShipObject::SL_damaged)
		{
			conditionText = CuiStringIdsShipChoose::condition_damaged.localize();
		}
		else if(launchable == ShipObject::SL_pristine)
		{
			conditionText = CuiStringIdsShipChoose::condition_pristine.localize();
		}
		else
		{
			DEBUG_FATAL(true, ("Unknown ShipObject::ShipLaunchable value returned."));
		}

		condition->SetLocalText(conditionText);
	}

	ms_shipCompPages[ship.getNetworkId()] = newShipPage;

	//hook up selection button
	baseObject = newShipPage->GetChild("stats.buttons.buttonSelect");
	UIButton * const selectButton = safe_cast<UIButton *>(baseObject);
	if(selectButton)
	{
		registerMediatorObject(*selectButton, true);
		selectButton->SetProperty(Properties::ShipNetworkId, Unicode::narrowToWide(ship.getNetworkId().getValueString().c_str()));
		if(!PlayerObject::isAdmin() && !m_parkingDataReceived && ConfigClientGame::getValidateShipParkingLocation())
		{
			//button is disabled until we receive the parking data from the server
			selectButton->SetEnabled(false);
		}
	}

	m_shipComp->AddChild(newShipPage);
	newShipPage->Link();
}

//----------------------------------------------------------------------

void SwgCuiShipChoose::onShipParkingDataReceived(PlayerCreatureController::Messages::ShipParkingDataReceived::Payload const & payload)
{
	std::vector<std::pair<NetworkId, std::string> > const & data = payload;
	if(data.empty())
		return;

	std::string terminalLocation;

	//first pair is terminalid and location
	std::vector<std::pair<NetworkId, std::string> >::const_iterator i = data.begin();
	if(i == data.end())
		return;

	NetworkId const & terminalId = i->first;
	if(terminalId != ms_terminal)
		return;
	terminalLocation = i->second;

	//move past terminal entry
	++i;

	UIBaseObject * baseObject = NULL;
	for(; i != data.end(); ++i)
	{
		NetworkId const & shipId = i->first;

		std::map<NetworkId, UIComposite *>::iterator j = ms_shipCompPages.find(shipId);
		if(j != ms_shipCompPages.end())
		{
			UIComposite * const comp = j->second;
			if(comp)
			{
				baseObject = comp->GetChild("stats.buttons.buttonSelect");
				UIButton * const selectButton = safe_cast<UIButton *>(baseObject);
				if(selectButton)
				{
					baseObject = comp->GetChild("stats.parking.text");
					UIText * const parkingLocationText = safe_cast<UIText *>(baseObject);
					if(parkingLocationText)
					{
						selectButton->SetEnabled(true);
						selectButton->SetLocalText(CuiStringIdsShipChoose::select.localize());
					}
				}
			}
		}
	}

	m_parkingDataReceived = true;
}

////====================================================================
