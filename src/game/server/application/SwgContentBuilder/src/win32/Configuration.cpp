// ======================================================================
//
// Configuration.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgContentBuilder.h"
#include "Configuration.h"

#include "sharedFoundation/ConfigFile.h"

#include <string>
#include <vector>

// ======================================================================

namespace ConfigurationNamespace
{
	typedef std::vector<std::string> StringList;
	
	StringList ms_questTypes;
	StringList ms_missionConversationTypes;
	StringList ms_npcConversationTypes;
	StringList ms_planets;
	StringList ms_factionRewards;
	StringList ms_actions;
	StringList ms_dispositions;
	StringList ms_npcGiverEntries;
	StringList ms_npcTargetEntries;

	bool load ();

	void populate (StringList const & stringList, CComboBox & comboBox)
	{
		comboBox.ResetContent ();

		StringList::const_iterator end = stringList.end ();
		for (StringList::const_iterator iter = stringList.begin (); iter != end; ++iter)
			comboBox.AddString (iter->c_str ());
	}

	void populate (StringList const & stringList, CTreeCtrl & treeCtrl)
	{
		treeCtrl.DeleteAllItems ();

		StringList::const_iterator end = stringList.end ();
		for (StringList::const_iterator iter = stringList.begin (); iter != end; ++iter)
			treeCtrl.InsertItem (iter->c_str ());
	}
};

using namespace ConfigurationNamespace;

// ======================================================================

bool Configuration::install ()
{
	if (!load ())
	{
		ms_questTypes.clear ();
		ms_questTypes.push_back ("none");

		ms_planets.clear ();
		ms_planets.push_back ("none");

		ms_factionRewards.clear ();
		ms_factionRewards.push_back ("none");

		ms_actions.clear ();
		ms_actions.push_back ("none");

		ms_dispositions.clear ();
		ms_dispositions.push_back ("none");

		ms_npcGiverEntries.clear ();
		ms_npcGiverEntries.push_back ("none");

		ms_npcTargetEntries.clear ();
		ms_npcTargetEntries.push_back ("none");

		return false;
	}

	return true;
}

// ----------------------------------------------------------------------

bool ConfigurationNamespace::load ()
{
	{
		char const * result = 0;
		int i = 0;
		do
		{
			result = ConfigFile::getKeyString ("SwgContentBuilder", "questType", i, 0);
			if (result)
			{
				ms_questTypes.push_back (result);

				++i;
			}
		} 
		while (result);
	}

	ms_missionConversationTypes.push_back ("terse");
	ms_missionConversationTypes.push_back ("normal");
	ms_missionConversationTypes.push_back ("verbose");

	ms_npcConversationTypes.push_back ("terse");
	ms_npcConversationTypes.push_back ("normal");
	ms_npcConversationTypes.push_back ("extended");
	ms_npcConversationTypes.push_back ("verbose");

	{
		char const * result = 0;
		int i = 0;
		do
		{
			result = ConfigFile::getKeyString ("SwgContentBuilder", "planet", i, 0);
			if (result)
			{
				ms_planets.push_back (result);

				++i;
			}
		} 
		while (result);
	}

	{
		char const * result = 0;
		int i = 0;
		do
		{
			result = ConfigFile::getKeyString ("SwgContentBuilder", "faction", i, 0);
			if (result)
			{
				ms_factionRewards.push_back (result);

				++i;
			}
		} 
		while (result);
	}

	{
		char const * result = 0;
		int i = 0;
		do
		{
			result = ConfigFile::getKeyString ("SwgContentBuilder", "action", i, 0);
			if (result)
			{
				ms_actions.push_back (result);

				++i;
			}
		} 
		while (result);
	}

	{
		char const * result = 0;
		int i = 0;
		do
		{
			result = ConfigFile::getKeyString ("SwgContentBuilder", "disposition", i, 0);
			if (result)
			{
				ms_dispositions.push_back (result);

				++i;
			}
		} 
		while (result);
	}

	{
		char const * result = 0;
		int i = 0;
		do
		{
			result = ConfigFile::getKeyString ("SwgContentBuilder", "npcGiver", i, 0);
			if (result)
			{
				ms_npcGiverEntries.push_back (result);

				++i;
			}
		} 
		while (result);
	}

	{
		char const * result = 0;
		int i = 0;
		do
		{
			result = ConfigFile::getKeyString ("SwgContentBuilder", "npcTarget", i, 0);
			if (result)
			{
				ms_npcTargetEntries.push_back (result);

				++i;
			}
		} 
		while (result);
	}

	return 
		!ms_questTypes.empty () &&
		!ms_missionConversationTypes.empty () &&
		!ms_npcConversationTypes.empty () &&
		!ms_planets.empty () &&
		!ms_factionRewards.empty () &&
		!ms_actions.empty () &&
		!ms_dispositions.empty () &&
		!ms_npcGiverEntries.empty () &&
		!ms_npcTargetEntries.empty ();
}

// ----------------------------------------------------------------------

void Configuration::populateQuestTypes (CComboBox & comboBox)
{
	populate (ms_questTypes, comboBox);
	comboBox.SetCurSel (ms_questTypes.size () > 1 ? 1 : 0);
}

// ----------------------------------------------------------------------

void Configuration::populateMissionConversationTypes (CComboBox & comboBox)
{
	populate (ms_missionConversationTypes, comboBox);
	comboBox.SetCurSel (ms_missionConversationTypes.size () > 1 ? 1 : 0);
}

// ----------------------------------------------------------------------

void Configuration::populateNpcConversationTypes (CComboBox & comboBox)
{
	populate (ms_npcConversationTypes, comboBox);
	comboBox.SetCurSel (ms_npcConversationTypes.size () > 1 ? 1 : 0);
}

// ----------------------------------------------------------------------

void Configuration::populatePlanets (CComboBox & comboBox)
{
	populate (ms_planets, comboBox);
	comboBox.SetCurSel (0);
}

// ----------------------------------------------------------------------

void Configuration::populateFactionRewards (CComboBox & comboBox)
{
	populate (ms_factionRewards, comboBox);
	comboBox.SetCurSel (0);
}

// ----------------------------------------------------------------------

void Configuration::populateActions (CComboBox & comboBox)
{
	populate (ms_actions, comboBox);
	comboBox.SetCurSel (0);
}

// ----------------------------------------------------------------------

void Configuration::populateDispositions (CComboBox & comboBox)
{
	populate (ms_dispositions, comboBox);
	comboBox.SetCurSel (ms_dispositions.size () > 1 ? 1 : 0);
}

// ----------------------------------------------------------------------

void Configuration::populateNpcGiverEntries (CTreeCtrl & treeCtrl)
{
	populate (ms_npcGiverEntries, treeCtrl);
}

// ----------------------------------------------------------------------

void Configuration::populateNpcTargetEntries (CTreeCtrl & treeCtrl)
{
	populate (ms_npcTargetEntries, treeCtrl);
}

// ======================================================================

