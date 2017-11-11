// ======================================================================
//
// SwgCuiCommunity_History.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommunity_History.h"

#include "clientGame/MatchMakingManager.h"
#include "clientGame/MatchMakingResult.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UITable.h"
#include "UITableModelDefault.h"
#include "UIList.h"
#include "UIPage.h"
#include <cstdio>

// ======================================================================
//
// SwgCuiCommunityHistoryNamespace
//
// ======================================================================

namespace SwgCuiCommunityHistoryNamespace
{
	int const s_historyCountMax = 100;
}

using namespace SwgCuiCommunityHistoryNamespace;

// ======================================================================
//
// SwgCuiCommunity::History
//
// ======================================================================

SwgCuiCommunity::History::History(UIPage &page, SwgCuiCommunity const &communityMediator)
 : CuiMediator("SwgCuiCommunity_History", page)
 , UIEventCallback()
 , m_communityMediator(communityMediator)
 , m_callBack(new MessageDispatch::Callback)
 , m_historyTable(NULL)
{
	m_callBack->connect(*this, &SwgCuiCommunity::History::onMatchFound, static_cast<MatchMakingManager::Messages::MatchFoundResult *>(0));

	getCodeDataObject(TUITable, m_historyTable, "TableHistory");
	registerMediatorObject(*m_historyTable, true);

	// Clear out any data from the UI builder

	UITableModelDefault *tableModel = dynamic_cast<UITableModelDefault *>(m_historyTable->GetTableModel());

	if (tableModel != NULL)
	{
		tableModel->ClearTable();
	}
}

//-----------------------------------------------------------------

SwgCuiCommunity::History::~History()
{
	m_callBack->disconnect(*this, &SwgCuiCommunity::History::onMatchFound, static_cast<MatchMakingManager::Messages::MatchFoundResultString *>(0));
	delete m_callBack;
	m_callBack = NULL;

	m_historyTable = NULL;
}

//-----------------------------------------------------------------

void SwgCuiCommunity::History::performActivate()
{
}

//-----------------------------------------------------------------

void SwgCuiCommunity::History::performDeactivate()
{
}

//-----------------------------------------------------------------

void SwgCuiCommunity::History::onMatchFound(MatchMakingResult const &matchMakingResult)
{
	UITableModelDefault *tableModel = dynamic_cast<UITableModelDefault *>(m_historyTable->GetTableModel());

	if (tableModel != NULL)
	{
		int const rowCount = tableModel->GetRowCount();

		for (int column = 0; column < static_cast<int>(C_count); ++column)
		{
			UIDataSource *dataSource = tableModel->GetColumnDataSource(column);

			if (dataSource != NULL)
			{
				// If there is too many history items, remove the oldest

				if (rowCount > s_historyCountMax)
				{
					UIData *data = tableModel->GetCellDataLogical(0, column);
					IGNORE_RETURN(dataSource->RemoveChild(data));
					data = NULL;
				}

				switch (column)
				{
					case C_time:
						{
							Unicode::String timeStamp;
							IGNORE_RETURN(CuiUtils::FormatDate(timeStamp, matchMakingResult.getSystemTime()));
							IGNORE_RETURN(tableModel->AppendCell(column, NULL, timeStamp));
						}
						break;
					case C_name:
						{
							IGNORE_RETURN(tableModel->AppendCell(column, NULL, matchMakingResult.getPlayerName()));
						}
						break;
					case C_matchToThem:
						{
							char text[256];
							sprintf(text, "%d%%", static_cast<int>(100.0f * matchMakingResult.getPreferenceToProfileMatchPercent()));
							IGNORE_RETURN(tableModel->AppendCell(column, NULL, Unicode::narrowToWide(text)));
						}
						break;
					case C_matchToYou:
						{
							char text[256];
							sprintf(text, "%d%%", static_cast<int>(100.0f * matchMakingResult.getProfileMatchPercent()));
							IGNORE_RETURN(tableModel->AppendCell(column, NULL, Unicode::narrowToWide(text)));
						}
						break;
					default:
						{
						}
						break;
				}
			}
		}

		tableModel->fireColumnsChanged();
	}
}

// ======================================================================
