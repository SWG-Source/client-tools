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

class Configuration
{
public:

	static bool install ();

	static CString getConfiguration ();

	static CString const & getServerObjectTemplatePath ();
	static CString const & getDraftSchematicDirectory ();

	static CString const & getServerDraftSchematicObjectTemplateVersion ();
	static CString const & getServerIntangibleObjectTemplateVersion ();
	static CString const & getServerObjectTemplateVersion ();
	static CString const & getSharedDraftSchematicObjectTemplateVersion ();
	static CString const & getSharedIntangibleObjectTemplateVersion ();
	static CString const & getSharedObjectTemplateVersion ();

	static void populateBaseServerDraftSchematicObjectTemplates (CComboBox & comboBox);
	static void populateArmorRatings (CComboBox & comboBox);
	static void populateCraftingTypes (CComboBox & comboBox);
	static void populateDamageTypes (CComboBox & comboBox);
	static void populateIngredientTypes (CComboBox & comboBox);
	static void populateResourceTypes (CTreeCtrl & treeCtrl);
	static void populateStringTables (CComboBox & comboBox);
	static void populateXpTypes (CComboBox & comboBox);

	static CString const & getDefaultBaseServerDraftSchematicObjectTemplate ();
	static CString const & getDefaultArmorRating ();
	static CString const & getDefaultInteger ();
	static CString const & getDefaultCraftingType ();
	static CString const & getDefaultDamageType ();
	static CString const & getDefaultIngredientType ();
	static CString const & getDefaultStringTable ();
	static CString const & getDefaultXpType ();

private:

	static bool loadCfg ();
	static bool loadIni ();
};

// ======================================================================

#endif

