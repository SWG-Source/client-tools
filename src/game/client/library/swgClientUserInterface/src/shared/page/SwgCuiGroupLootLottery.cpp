//===================================================================
//
// SwgCuiGroupLootLottery.cpp
// copyright 2004, sony online entertainment
// tford
//
//===================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiGroupLootLottery.h"

#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiIconManagerCallback.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiRadialMenuManager.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/Container.h"
#include "sharedObject/Controller.h"
#include "swgClientUserInterface/SwgCuiContainerProviderDefault.h"
#include "swgClientUserInterface/SwgCuiInventoryContainer.h"
#include "UIButton.h"
#include "UIMessage.h"
#include "UIVolumePage.h"

//===================================================================

namespace SwgCuiGroupLootLotteryNamespace
{
	std::string const cms_mediatorBaseName("SwgCuiGroupLootLottery");

	class IconCallback : public CuiIconManagerCallback
	{
	public:
		IconCallback();
		virtual ~IconCallback();
		virtual void overrideTooltip(const ClientObject & obj, Unicode::String & str) const;
		virtual bool overrideDoubleClick(const UIWidget & viewer) const;
	private:
		IconCallback(IconCallback const &);
		IconCallback & operator=(IconCallback const &);
	};

	class ContainerFilter : public SwgCuiInventoryContainerFilter
	{
	public:
		ContainerFilter();
		virtual ~ContainerFilter();
		virtual bool overrideMessage(const UIWidget * , const UIMessage & ) const;
	private:
		ContainerFilter(ContainerFilter const &);
		ContainerFilter & operator=(ContainerFilter const &);
	};

	typedef std::vector<SwgCuiGroupLootLottery *> GroupLootLotteries;
	GroupLootLotteries s_groupLootLotteries;

	SwgCuiGroupLootLottery * findWindow(ClientObject const & container);
	
	bool isNotInLootLotteryWindow(Object const * object);

	UIPoint const cs_cascade = UIPoint(20,40);
	bool registeredCanBePickedUp = false;

	float const s_numberOfSecondsWindowWillStayOpen = 30.0f;
}

//======================================================================

SwgCuiGroupLootLottery * SwgCuiGroupLootLotteryNamespace::findWindow(ClientObject const & container)
{
	GroupLootLotteries::const_iterator ii = s_groupLootLotteries.begin();
	GroupLootLotteries::const_iterator iiEnd = s_groupLootLotteries.end();

	for (; ii != iiEnd; ++ii)
	{
		SwgCuiGroupLootLottery * const window = *ii;

		if (window != 0)
		{
			ClientObject * const windowContainer = window->getContainer();
			if ((windowContainer != 0) && (windowContainer == &container))
			{
				return window;
			}
		}

		NOT_NULL(window);
	}
	return 0;
}

//----------------------------------------------------------------------

bool SwgCuiGroupLootLotteryNamespace::isNotInLootLotteryWindow(Object const * object)
{
	GroupLootLotteries::const_iterator ii = s_groupLootLotteries.begin();
	GroupLootLotteries::const_iterator iiEnd = s_groupLootLotteries.end();

	for (; ii != iiEnd; ++ii)
	{
		SwgCuiGroupLootLottery const * const window = *ii;

		if ((window != 0) && (window->objectIsInLottery(object)))
		{
			return false;
		}

		NOT_NULL(window);
	}
	return true;
}

//----------------------------------------------------------------------

SwgCuiGroupLootLotteryNamespace::IconCallback::IconCallback()
: CuiIconManagerCallback()
{
}

//----------------------------------------------------------------------

SwgCuiGroupLootLotteryNamespace::IconCallback::~IconCallback()
{
}

//----------------------------------------------------------------------

void SwgCuiGroupLootLotteryNamespace::IconCallback::overrideTooltip(const ClientObject &, Unicode::String &) const
{
}

//----------------------------------------------------------------------

bool SwgCuiGroupLootLotteryNamespace::IconCallback::overrideDoubleClick(const UIWidget &) const
{
	return false;
}

//======================================================================

SwgCuiGroupLootLotteryNamespace::ContainerFilter::ContainerFilter()
: SwgCuiInventoryContainerFilter(new SwgCuiGroupLootLotteryNamespace::IconCallback)
{
}

//----------------------------------------------------------------------

SwgCuiGroupLootLotteryNamespace::ContainerFilter::~ContainerFilter()
{
	delete iconCallback;
	iconCallback = 0;
}

//----------------------------------------------------------------------

bool SwgCuiGroupLootLotteryNamespace::ContainerFilter::overrideMessage(const UIWidget * , const UIMessage & message) const
{
	if ((message.Type >= UIMessage::DragFirst) && (message.Type <= UIMessage::DragLast))
	{
		return true;
	}

	if ((message.Type >= UIMessage::LeftMouseDoubleClick) && (message.Type <= UIMessage::RightMouseDoubleClick))
	{
		return true;
	}

	return false;
}

//======================================================================

using namespace SwgCuiGroupLootLotteryNamespace;

//======================================================================

SwgCuiGroupLootLottery::SwgCuiGroupLootLottery(UIPage & page, ClientObject & container)
: UIEventCallback()
, CuiMediator("SwgCuiGroupLootLottery", page)
, m_buttonNone(0)
, m_buttonSelection(0)
, m_buttonAll(0)
, m_volumeLoot(0)
, m_timerHolster(0)
, m_timer(0)
, m_containerMediator(0)
, m_containerProvider(new SwgCuiContainerProviderDefault)
, m_containerFilter(new ContainerFilter)
, m_lifeTimeSeconds(0.0f)
{
	getCodeDataObject(TUIButton, m_buttonNone, "buttonNone");
	getCodeDataObject(TUIButton, m_buttonSelection, "buttonSelection");
	getCodeDataObject(TUIButton, m_buttonAll, "buttonAll");
	getCodeDataObject(TUIPage, m_timerHolster, "timerHolster");
	getCodeDataObject(TUIPage, m_timer, "timer");

	registerMediatorObject(*m_buttonNone, true);
	registerMediatorObject(*m_buttonSelection, true);
	registerMediatorObject(*m_buttonAll, true);
	registerMediatorObject(*m_timerHolster, true);
	registerMediatorObject(*m_timer, true);

	UIPage * containerPage = 0;
	getCodeDataObject(TUIPage, containerPage, "ContainerPage");
	m_containerMediator = new SwgCuiInventoryContainer(*containerPage);
	m_containerMediator->fetch();
	m_containerMediator->setContainerProvider(m_containerProvider);
	m_containerMediator->setFilter(m_containerFilter);
	m_containerMediator->setContainerObject(&container, "");

	setState(MS_closeable);
	setState(MS_closeDeactivates);

	if (!registeredCanBePickedUp)
	{
		CuiRadialMenuManager::registerCanBeManipulated(isNotInLootLotteryWindow);
		registeredCanBePickedUp = true;
	}
}

//----------------------------------------------------------------------

SwgCuiGroupLootLottery::~SwgCuiGroupLootLottery()
{
	m_buttonNone = 0;
	m_buttonSelection = 0;
	m_buttonAll = 0;
	m_volumeLoot = 0;
	m_timerHolster = 0;
	m_timer = 0;

	m_containerMediator->setContainerProvider(0);
	m_containerMediator->setFilter(0);
	m_containerMediator->release();
	m_containerMediator = 0;

	delete m_containerProvider;
	m_containerProvider = 0;

	delete m_containerFilter;
	m_containerFilter = 0;
}

//----------------------------------------------------------------------

SwgCuiGroupLootLottery * SwgCuiGroupLootLottery::createInto(UIPage & parent, ClientObject & container)
{
	UIPage * const dupe = UIPage::DuplicateInto(parent, "/PDA.lottery");

	if (dupe != 0)
	{
		SwgCuiGroupLootLottery * const mediator = new SwgCuiGroupLootLottery(*dupe, container);

		if (!s_groupLootLotteries.empty())
		{
			SwgCuiGroupLootLottery const * const lastCreated = *(s_groupLootLotteries.rbegin());
			if (lastCreated != 0)
			{
				UIPoint location = lastCreated->getPage().GetLocation();
				location += cs_cascade;
				mediator->getPage().SetLocation(location);
			}
		}

		return mediator; //lint !e429 msg not freed (controller owns it)
	}

	return 0;
}

//----------------------------------------------------------------------

bool SwgCuiGroupLootLottery::closeForContainer(ClientObject const 
											   & container)
{
	SwgCuiGroupLootLottery * const window = findWindow(container);
	if (window != 0)
	{
		window->sendSelectionToServer(RM_selection);
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

ClientObject * SwgCuiGroupLootLottery::getContainer() const
{
	if (m_containerMediator != 0)
	{
		return m_containerMediator->getContainerObject();
	}

	return 0;
}

//----------------------------------------------------------------------

bool SwgCuiGroupLootLottery::objectIsInLottery(Object const * const object) const
{
	if (m_containerMediator != 0)
	{
		SwgCuiInventoryContainer::ObjectWatcherVector const & objects = m_containerMediator->getObjects();
		SwgCuiInventoryContainer::ObjectWatcherVector::const_iterator ii = objects.begin();
		SwgCuiInventoryContainer::ObjectWatcherVector::const_iterator iiEnd = objects.end();

		for (; ii != iiEnd; ++ii)
		{
			SwgCuiInventoryContainer::ObjectWatcher watcher = *ii;

			if ((watcher != 0) && (watcher == object))
			{
				return true;
			}
		}
	}
	return false;
}

//----------------------------------------------------------------------

void SwgCuiGroupLootLottery::performActivate()
{
	CuiMediator::performActivate();
	CuiManager::requestPointer(true);
	setIsUpdating(true);

	if (m_containerMediator != 0)
	{
		m_containerMediator->activate();

		UIPage * const containerPage = safe_cast<UIPage *>(m_containerMediator->getPage().GetParentWidget());
		if (containerPage != 0)
		{
			containerPage->Pack();
		}
	}

	s_groupLootLotteries.push_back(this);
}

//-------------------------------------------------------------------

void SwgCuiGroupLootLottery::performDeactivate()
{
	if (m_containerMediator != 0)
	{
		m_containerMediator->deactivate();
	}

	setIsUpdating(false);
	CuiMediator::performDeactivate();
	CuiManager::requestPointer(false);

	GroupLootLotteries::iterator ii = std::find(s_groupLootLotteries.begin(), s_groupLootLotteries.end(), this);

	if (ii != s_groupLootLotteries.end())
	{
		IGNORE_RETURN(s_groupLootLotteries.erase(ii));
	}
}

//-------------------------------------------------------------------

void SwgCuiGroupLootLottery::sendSelectionToServer(ResponseMode const responseMode)
{
	if (m_containerMediator == 0)
	{
		return;
	}

	typedef std::vector<NetworkId> NetworkIds;
	NetworkIds selectedIds;

	switch(responseMode)
	{
		case RM_none:
			break;
		case RM_selection:
			{
				SwgCuiInventoryContainer::ObjectWatcherVector const & objects = m_containerMediator->getSelection();
				SwgCuiInventoryContainer::ObjectWatcherVector::const_iterator ii = objects.begin();
				SwgCuiInventoryContainer::ObjectWatcherVector::const_iterator iiEnd = objects.end();

				for (; ii != iiEnd; ++ii)
				{
					SwgCuiInventoryContainer::ObjectWatcher watcher = *ii;

					if (watcher != 0)
					{
						selectedIds.push_back(watcher->getNetworkId());
					}
				}
			}
			break;
		case RM_all:
			{
				SwgCuiInventoryContainer::ObjectWatcherVector const & objects = m_containerMediator->getObjects();
				SwgCuiInventoryContainer::ObjectWatcherVector::const_iterator ii = objects.begin();
				SwgCuiInventoryContainer::ObjectWatcherVector::const_iterator iiEnd = objects.end();

				for (; ii != iiEnd; ++ii)
				{
					SwgCuiInventoryContainer::ObjectWatcher watcher = *ii;

					if (watcher != 0)
					{
						selectedIds.push_back(watcher->getNetworkId());
					}
				}
			}
			break;
		default:
			break;
	}

	CreatureObject * const player = Game::getPlayerCreature();
	Controller * const controller = (player != 0) ? player->getController() : 0;

	Object * const container = getContainer();
	NetworkId const containerId = (container != 0) ? container->getNetworkId() : NetworkId::cms_invalid;

	if (controller != 0)
	{
		typedef std::pair<NetworkId, std::vector<NetworkId> > Payload;
		Payload payload;
		payload.first = containerId;
		payload.second = selectedIds;

		typedef MessageQueueGenericValueType<Payload> Message;
		Message * const message = new Message(payload);

		controller->appendMessage(
			CM_groupLotteryWindowCloseResults,
			0.0f,
			message,
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_AUTH_SERVER);
		closeNextFrame();
	} //lint !e429 custodial pointer
}

//-------------------------------------------------------------------

void SwgCuiGroupLootLottery::OnButtonPressed(UIWidget * const context)
{
	if (context == m_buttonNone)
	{
		sendSelectionToServer(RM_none);
	}
	else if (context == m_buttonSelection)
	{
		sendSelectionToServer(RM_selection);
	}
	else if (context == m_buttonAll)
	{
		sendSelectionToServer(RM_all);
	}
}

//-------------------------------------------------------------------

void SwgCuiGroupLootLottery::update(float const deltaTimeSecs)
{
	m_lifeTimeSeconds += deltaTimeSecs;

	if (m_lifeTimeSeconds > s_numberOfSecondsWindowWillStayOpen)
	{
		sendSelectionToServer(RM_selection);
	}

	int const totalWidth = m_timerHolster->GetWidth();
	float const percentage = 1.0f - (m_lifeTimeSeconds / s_numberOfSecondsWindowWillStayOpen);
	int const timerWidth = static_cast<int>(percentage * static_cast<float>(totalWidth));
	UISize size = m_timer->GetSize();
	size.x = timerWidth;
	m_timer->SetSize(size);
	m_timer->SetMaximumSize(size);
	m_timer->SetMinimumSize(size);

	if (m_containerMediator != 0)
	{
		ClientObject const * const container = getContainer();

		if (container == 0)
		{
			closeNextFrame();
			return;
		}
	}
}

//======================================================================
