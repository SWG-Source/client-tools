// ======================================================================
//
// Configuration.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_Configuration_H
#define INCLUDED_Configuration_H

// ======================================================================

class CComboBox;

// ======================================================================

class Configuration
{
public:

	static bool install ();

	static void populateQuestTypes (CComboBox & comboBox);
	static void populateMissionConversationTypes (CComboBox & comboBox);
	static void populateNpcConversationTypes (CComboBox & comboBox);
	static void populatePlanets (CComboBox & comboBox);
	static void populateFactionRewards (CComboBox & comboBox);
	static void populateActions (CComboBox & comboBox);
	static void populateDispositions (CComboBox & comboBox);
	static void populateNpcGiverEntries (CTreeCtrl & treeCtrl);
	static void populateNpcTargetEntries (CTreeCtrl & treeCtrl);
};

// ======================================================================

#endif
