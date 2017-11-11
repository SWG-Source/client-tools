// ======================================================================
//
// SwgCuiService_HowToSearch.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiService_HowToSearch.h"
#include "clientGame/CustomerServiceManager.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiCsManagerListener.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsCustomerService.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "UIButton.h"
#include "UIMessage.h"
#include "unicodeArchive/UnicodeArchive.h"


// ======================================================================
//
// SwgCuiService::HowToSearch
//
// ======================================================================

SwgCuiService::HowToSearch::HowToSearch(UIPage &page, SwgCuiService &serviceMediator)
 : CuiMediator("SwgCuiService_HowToSearch", page)
 , m_callBack(new MessageDispatch::Callback)
 , UIEventCallback()
 , m_serviceMediator(serviceMediator)
{

}

//-----------------------------------------------------------------

SwgCuiService::HowToSearch::~HowToSearch()
{
	delete m_callBack;
	m_callBack = NULL;
}

//-----------------------------------------------------------------

void SwgCuiService::HowToSearch::performActivate()
{



}

//-----------------------------------------------------------------

void SwgCuiService::HowToSearch::performDeactivate()
{

}

//-----------------------------------------------------------------

void SwgCuiService::HowToSearch::update(float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);
}

// ======================================================================
