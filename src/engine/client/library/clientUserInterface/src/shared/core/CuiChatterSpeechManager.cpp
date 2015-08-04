// ======================================================================
//
// CuiChatterSpeechManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiChatterSpeechManager.h"

#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <algorithm>

// ================================================================

namespace CuiChatterSpeechManagerNamespace
{
	bool ms_installed = false;
	std::vector<std::string> ms_chatterData;

	std::string cms_chatterSpeechTableName = "datatables/player/chatter_speech.iff";

}

using namespace CuiChatterSpeechManagerNamespace;

//-----------------------------------------------------------------

void  CuiChatterSpeechManager::install ()
{
	DEBUG_FATAL (ms_installed, ("already installed\n"));

	ms_chatterData.clear();

	loadData();

	ms_installed = true;
}

//-----------------------------------------------------------------

void  CuiChatterSpeechManager::remove ()
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

	ms_chatterData.clear();

	ms_installed = false;
}

//-----------------------------------------------------------------

void CuiChatterSpeechManager::loadData()
{
	DataTable const * const table = DataTableManager::getTable (cms_chatterSpeechTableName, true);
	if (table)
	{
		int const numRows = table->getNumRows ();

		for (int i = 0; i < numRows; ++i)
		{
			std::string const & value = table->getStringValue("command", i);
			ms_chatterData.push_back(value);
		}

		std::sort (ms_chatterData.begin (), ms_chatterData.end ());
	}
	else
		WARNING (true, ("could not load data table [%s]", cms_chatterSpeechTableName.c_str ()));
}

//-----------------------------------------------------------------

std::vector<std::string> const & CuiChatterSpeechManager::getData()
{
	return ms_chatterData;
}

// ================================================================
