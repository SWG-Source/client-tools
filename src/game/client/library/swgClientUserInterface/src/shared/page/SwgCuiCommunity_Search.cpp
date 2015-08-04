// ======================================================================
//
// SwgCuiCommunity_Search.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommunity_Search.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/Game.h"
#include "clientGame/MatchMakingManager.h"
#include "clientGame/WhoManager.h"
#include "clientUserInterface/CuiSettings.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedGame/LfgDataTable.h"
#include "sharedGame/MatchMakingCharacterPreferenceId.h"
#include "sharedGame/MatchMakingCharacterResult.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "swgClientUserInterface/SwgCuiLfg.h"

#include "UIButton.h"
#include "UIData.h"
#include "UITable.h"
#include "UITableModelDefault.h"
#include "UIText.h"
#include "UITextbox.h"

// ======================================================================
//
// SwgCuiCommunity::Search
//
// ======================================================================

//-----------------------------------------------------------------

SwgCuiCommunity::Search::Search(UIPage &page, SwgCuiCommunity const &communityMediator)
 : CuiMediator("SwgCuiCommunity_Search", page)
 , UIEventCallback()
 , m_communityMediator(communityMediator)
 , m_searchButton(NULL)
 , m_callBack(new MessageDispatch::Callback)
 , m_Lfg(NULL)
 , m_buttonTell(NULL) 
 , m_buttonInvite(NULL)
 , m_buttonTellAndInvite(NULL)
 , m_tellTextbox(NULL)
 , m_bioTextbox(NULL)
 , m_tooManyResultsText(NULL)
{
	m_callBack->connect(*this, &SwgCuiCommunity::Search::onCharacterMatchRetrieved, static_cast<PlayerCreatureController::Messages::CharacterMatchRetrieved *>(0));

	getCodeDataObject(TUIButton, m_searchButton, "buttonSearch");
	getCodeDataObject(TUITable, m_table, "table");

	UITableModelDefault * const model = safe_cast<UITableModelDefault *>(m_table->GetTableModel ());

	if (model)
		model->ClearTable();

	getCodeDataObject(TUIButton, m_buttonTell, "buttonTell");
	getCodeDataObject(TUIButton, m_buttonInvite, "buttonInvite");
	getCodeDataObject(TUIButton, m_buttonTellAndInvite, "buttonTellAndInvite");
	getCodeDataObject(TUITextbox, m_tellTextbox, "tellbox");
	getCodeDataObject(TUIText, m_bioTextbox, "biotext");
	getCodeDataObject(TUIText, m_tooManyResultsText, "toomanyresults");

	UIPage *lfgPage = NULL;
	getCodeDataObject(TUIPage, lfgPage, "pagelfg");
	m_Lfg = new SwgCuiLfg(*lfgPage);
	
	if (m_Lfg)
	{
		m_Lfg->fetch();

		lfgPage->Link();
		lfgPage->SetVisible(true);
		lfgPage->SetEnabled(true);
	}		
}

//-----------------------------------------------------------------

SwgCuiCommunity::Search::~Search()
{
	if (m_Lfg)		
		m_Lfg->release();
	
	m_Lfg = NULL;

	delete m_callBack;
	m_callBack = NULL;

	m_searchButton = NULL;
	m_buttonTell = NULL;
	m_buttonInvite = NULL;
	m_buttonTellAndInvite = NULL;
	m_tellTextbox = NULL;
	m_bioTextbox = NULL;
	m_tooManyResultsText = NULL;
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Search::performActivate()
{
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Search::performDeactivate()
{
	m_Lfg->saveOrLoadBackup(true);
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Search::OnButtonPressed(UIWidget *context)
{
	if (context == m_searchButton)
	{
		MatchMakingCharacterPreferenceId matchMakingCharacterPreferenceId;
		BitArray & searchAttribute = matchMakingCharacterPreferenceId.getSearchAttribute();

		m_Lfg->getAllSearchCriteria(searchAttribute);

		WhoManager::requestWhoMatch(matchMakingCharacterPreferenceId);
	}
	else if (context == m_buttonInvite)
	{
		sendInviteToSelection();
	}
	else if (context == m_buttonTell)
	{
		sendTellToSelection();	
	}
	else if (context == m_buttonTellAndInvite)
	{
		sendTellToSelection();
		sendInviteToSelection();
	}
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Search::sendTellToSelection() const
{
	UITableModelDefault * const model = safe_cast<UITableModelDefault *>(m_table->GetTableModel ());

	if (!model)
		return;

	UIData const * const  cellData = model->GetCellDataVisual(m_table->GetLastSelectedRow(), 0);

	if (!cellData)
		return;
	
	UIString tellText;

	if (!cellData->GetProperty(UILowerString("FirstName"), tellText))
		return;

	tellText += Unicode::narrowToWide(" ") + m_tellTextbox->GetLocalText();

	ClientCommandQueue::enqueueCommand("tell", NetworkId::cms_invalid, tellText);
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Search::sendInviteToSelection(void) const
{
	UITableModelDefault * const model = safe_cast<UITableModelDefault *>(m_table->GetTableModel ());

	if (!model)
		return;

	UIData const * const  cellData = model->GetCellDataVisual(m_table->GetLastSelectedRow(), 0);

	if (!cellData)
		return;

	UIString firstName;

	if (!cellData->GetProperty(UILowerString("FirstName"), firstName))
		return;
	
	ClientCommandQueue::enqueueCommand("invite", NetworkId::cms_invalid, firstName);	
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Search::OnTextboxChanged(UIWidget *context)
{
	if (context == m_tellTextbox)
	{
		updateButtonStates();
	}
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Search::OnGenericSelectionChanged(UIWidget * context)
{
	if (context == m_table)
	{
		UITableModelDefault * const model = safe_cast<UITableModelDefault *>(m_table->GetTableModel ());

		if (!model)
			return;
		
		UIString bio;
		UIData const * const cellData = model->GetCellDataVisual(m_table->GetLastSelectedRow(), 0);

		if (cellData)
		{
			cellData->GetProperty(UILowerString("PlayerBio"), bio);
		}

		m_bioTextbox->SetLocalText(bio);

		updateButtonStates();
  	}
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Search::onCharacterMatchRetrieved(PlayerCreatureController::Messages::CharacterMatchRetrieved::MatchResults const &matchResults)
{
	UITableModelDefault * const model = safe_cast<UITableModelDefault *>(m_table->GetTableModel ());

	if (!model)
		return;

	model->ClearTable();
	m_table->SelectRow(-1);

	WhoManager::buildResultTable(matchResults, model);

	m_tooManyResultsText->SetVisible(matchResults.m_hasMoreMatches);
}


//-----------------------------------------------------------------

void SwgCuiCommunity::Search::saveSettings() const
{
	CuiMediator::saveSettings();
	
	UITableModelDefault * const model = safe_cast<UITableModelDefault *>(m_table->GetTableModel ());

	if (!model)
		return;
	
	std::string const keyWidth("ColumnWidth");
	std::string const keyProportion("ColumnProportion");
 
	std::vector<UITable::ColumnSizeInfo> colSizes = m_table->GetColumnSizeInfo();

	for (uint32 i = 0; i < colSizes.size(); i++)
	{
		CuiSettings::saveInteger(getMediatorDebugName(), keyWidth + static_cast<char> (i + 1), colSizes[i].width);
		CuiSettings::saveData(getMediatorDebugName(), keyProportion + static_cast<char> (i + 1), FormattedString<16>().sprintf("%.4f", colSizes[i].proportion));
	}
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Search::loadSettings()
{
	CuiMediator::loadSettings();

	UITableModelDefault * const model = safe_cast<UITableModelDefault *>(m_table->GetTableModel ());

	if (!model)
		return;

	std::string const keyWidth("ColumnWidth");
	std::string const keyProportion("ColumnProportion");

	std::vector<UITable::ColumnSizeInfo> colSizes = m_table->GetColumnSizeInfo();
	
	for (uint32 i = 0; i < colSizes.size(); i++)
	{
		int width = 0;
		std::string proportion;

		if (CuiSettings::loadInteger(getMediatorDebugName(), keyWidth + static_cast<char> (i + 1), width))
		{
			colSizes[i].width  = width;
		}
		if (CuiSettings::loadData(getMediatorDebugName(), keyProportion + static_cast<char> (i + 1), proportion))
		{
			colSizes[i].proportion  = static_cast<float> (atof(proportion.c_str()));
		}
  	}
	
	m_table->SetColumnSizeInfo(colSizes);
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Search::updateButtonStates()
{
	int const charCount = m_tellTextbox->GetCharacterCount();

	if (charCount > 0)
	{
		m_buttonTell->SetEnabled(true);
		m_buttonTellAndInvite->SetEnabled(true);

		m_buttonInvite->SetVisible(false);
		m_buttonTellAndInvite->SetVisible(true);

	}
	else
	{
		m_buttonTell->SetEnabled(false);
		m_buttonTellAndInvite->SetEnabled(false);

		m_buttonInvite->SetVisible(true);
		m_buttonTellAndInvite->SetVisible(false);
	}


	UITableModelDefault * const model = safe_cast<UITableModelDefault *>(m_table->GetTableModel ());

	if (!model)
		return;

	bool isPlayerGrouped = false;

	UIData const * const cellData = model->GetCellDataVisual(m_table->GetLastSelectedRow(), 0);

	if (cellData)
	{
		isPlayerGrouped = cellData->GetPropertyBoolean(UILowerString("IsGrouped"), isPlayerGrouped);
	}

	if (m_table->GetLastSelectedRow() == -1)
	{
		m_buttonInvite->SetEnabled(false);
		m_buttonTell->SetEnabled(false);
		m_buttonTellAndInvite->SetEnabled(false);
	}
	else if (isPlayerGrouped)
	{
		m_buttonInvite->SetEnabled(false);
		m_buttonTellAndInvite->SetEnabled(false);
	}
	else
	{
		m_buttonInvite->SetEnabled(true);
	}
}

// ======================================================================
