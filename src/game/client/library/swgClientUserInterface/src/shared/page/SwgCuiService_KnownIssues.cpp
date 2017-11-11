// ======================================================================
//
// SwgCuiService_KnownIssues.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiService_KnownIssues.h"

#include "clientGame/GameNetwork.h"
#include "clientUserInterface/CuiCSManagerListener.h"
#include "clientUserInterface/CuiStringIdsCustomerService.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/GetArticleMessage.h"
#include "UIMessage.h"
#include "UIText.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================
//
// SwgCuiService::KnowledgeBase::KnownIssues
//
// ======================================================================

SwgCuiService::KnownIssues::KnownIssues(UIPage &page, SwgCuiService &serviceMediator)
 : CuiMediator("SwgCuiService_KnownIssues", page)
 , m_callBack(new MessageDispatch::Callback)
 , UIEventCallback()
 , m_serviceMediator(serviceMediator)
 , m_listBoxText(NULL)
{
	m_callBack->connect(*this, &SwgCuiService::KnownIssues::onGetFixedArticleResponse, static_cast<CuiCSManagerListener::Messages::GetArticleResponse *>(0));

	getCodeDataObject(TUIText, m_listBoxText, "listBoxText");
	registerMediatorObject(*m_listBoxText, true);
	m_listBoxText->Clear();
	m_listBoxText->SetMaximumCharacters(50 * 1024);

}

//-----------------------------------------------------------------

SwgCuiService::KnownIssues::~KnownIssues()
{
	delete m_callBack;
	m_callBack = NULL;

	m_listBoxText = NULL;

}

//-----------------------------------------------------------------

void SwgCuiService::KnownIssues::performActivate()
{


	initializeListBox();
}

//-----------------------------------------------------------------

void SwgCuiService::KnownIssues::performDeactivate()
{
	

}

//-----------------------------------------------------------------

void SwgCuiService::KnownIssues::update(float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);
}

//-----------------------------------------------------------------

void SwgCuiService::KnownIssues::initializeListBox()
{
	getKnownIssuesArticle();
}

//------------------------------------------------------------------

void SwgCuiService::KnownIssues::onGetFixedArticleResponse(std::pair<bool, Unicode::String> const &result)
{
	bool const success = result.first;

	if (success)
	{
		Unicode::String article = result.second;
		m_listBoxText->SetLocalText(article);
	}
	else
	{
		m_listBoxText->SetLocalText(CuiStringIdsCustomerService::know_article_response_error.localize());
	}

}

//-----------------------------------------------------------------

void SwgCuiService::KnownIssues::getKnownIssuesArticle()
{
	m_listBoxText->SetLocalText(CuiStringIdsCustomerService::know_article_retrieve.localize());
	char const * const defaultKnownIssuesArticle = "11092";
	char const * const knownIssuesArticle = ConfigFile::getKeyString("SwgClientUserInterface/SwgCuiService", "knownIssuesArticle", defaultKnownIssuesArticle);
	GameNetwork::send(GetArticleMessage(knownIssuesArticle, std::string("en")), true);		
}

// ======================================================================
