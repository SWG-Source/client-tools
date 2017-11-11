// ======================================================================
//
// Configuration.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgDraftSchematicEditor.h"
#include "Configuration.h"

// ======================================================================

namespace ConfigurationNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

#if 1
	CString const cms_root = "../../exe/win32/";
#else
	CString const cms_root = "";
#endif

	CString const cms_empty = "";
	CString const cms_zero = "0";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	CString ms_serverObjectTemplatePath;
	CString ms_draftSchematicDirectory;

	CString ms_serverDraftSchematicObjectTemplateVersion;
	CString ms_serverIntangibleObjectTemplateVersion;
	CString ms_serverObjectTemplateVersion;
	CString ms_sharedDraftSchematicObjectTemplateVersion;
	CString ms_sharedIntangibleObjectTemplateVersion;
	CString ms_sharedObjectTemplateVersion;

	typedef std::vector<CString> StringList;
	StringList ms_baseServerDraftSchematicObjectTemplates;
	StringList ms_armorRatings;
	StringList ms_craftingTypes;
	StringList ms_damageTypes;
	StringList ms_ingredientTypes;
	StringList ms_resourceTypes;
	StringList ms_stringTables;
	StringList ms_xpTypes;

	CString ms_defaultBaseServerDraftSchematicObjectTemplate;
	CString ms_defaultArmorRating;
	CString ms_defaultInteger;
	CString ms_defaultCraftingType;
	CString ms_defaultDamageType;
	CString ms_defaultIngredientType;
	CString ms_defaultStringTable;
	CString ms_defaultXpType;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void populate (StringList const & stringList, CComboBox & comboBox)
	{
		comboBox.ResetContent ();

		StringList::const_iterator end = stringList.end ();
		for (StringList::const_iterator iter = stringList.begin (); iter != end; ++iter)
			comboBox.AddString (*iter);
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void populate (StringList const & stringList, CTreeCtrl & treeCtrl)
	{
		treeCtrl.DeleteAllItems ();

		StringList::const_iterator end = stringList.end ();
		for (StringList::const_iterator iter = stringList.begin (); iter != end; ++iter)
			treeCtrl.InsertItem (*iter);
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
}

using namespace ConfigurationNamespace;

// ======================================================================

bool Configuration::install ()
{
	ms_serverObjectTemplatePath = "d:/work/swg/current/dsrc/sku.0/sys.server/compiled/game/object";
	ms_draftSchematicDirectory = "draft_schematic";

	if (!loadCfg ())
		return false;

	if (!loadIni ())
		return false;

	return true;
}

// ----------------------------------------------------------------------

CString Configuration::getConfiguration ()
{
	CString result;
	result += "serverObjectTemplatePath: " + ms_serverObjectTemplatePath + "\n";
	result += "draftSchematicDirectory: " + ms_draftSchematicDirectory + "\n";

	CString buffer;
	buffer.Format ("%3i baseServerDraftSchematicObjectTemplates; default=%s\n", ms_baseServerDraftSchematicObjectTemplates.size (), ms_defaultBaseServerDraftSchematicObjectTemplate);
	result += buffer;

	buffer.Format ("%3i armorRatings; default=%s\n", ms_armorRatings.size (), ms_defaultArmorRating);
	result += buffer;

	buffer.Format ("%3i craftingTypes; default=%s\n", ms_craftingTypes.size (), ms_defaultCraftingType);
	result += buffer;

	buffer.Format ("%3i damageTypes; default=%s\n", ms_damageTypes.size (), ms_defaultDamageType);
	result += buffer;

	buffer.Format ("%3i ingredientTypes; default=%s\n", ms_ingredientTypes.size (), ms_defaultIngredientType);
	result += buffer;

	buffer.Format ("%3i resourceTypes\n", ms_resourceTypes.size ());
	result += buffer;

	buffer.Format ("%3i stringTables\n", ms_stringTables.size ());
	result += buffer;

	buffer.Format ("%3i xpTypes; default=%s\n", ms_xpTypes.size (), ms_defaultXpType);
	result += buffer;

	return result;
}

// ----------------------------------------------------------------------

CString const & Configuration::getServerObjectTemplatePath ()
{
	return ms_serverObjectTemplatePath;
}

// ----------------------------------------------------------------------

CString const & Configuration::getDraftSchematicDirectory ()
{
	return ms_draftSchematicDirectory;
}

// ----------------------------------------------------------------------

CString const & Configuration::getServerDraftSchematicObjectTemplateVersion ()
{
	return ms_serverDraftSchematicObjectTemplateVersion;
}

// ----------------------------------------------------------------------

CString const & Configuration::getServerIntangibleObjectTemplateVersion ()
{
	return ms_serverIntangibleObjectTemplateVersion;
}

// ----------------------------------------------------------------------

CString const & Configuration::getServerObjectTemplateVersion ()
{
	return ms_serverObjectTemplateVersion;
}

// ----------------------------------------------------------------------

CString const & Configuration::getSharedDraftSchematicObjectTemplateVersion ()
{
	return ms_sharedDraftSchematicObjectTemplateVersion;
}

// ----------------------------------------------------------------------

CString const & Configuration::getSharedIntangibleObjectTemplateVersion ()
{
	return ms_sharedIntangibleObjectTemplateVersion;
}

// ----------------------------------------------------------------------

CString const & Configuration::getSharedObjectTemplateVersion ()
{
	return ms_sharedObjectTemplateVersion;
}

// ----------------------------------------------------------------------

void Configuration::populateBaseServerDraftSchematicObjectTemplates (CComboBox & comboBox)
{
	populate (ms_baseServerDraftSchematicObjectTemplates, comboBox);
}

// ----------------------------------------------------------------------

void Configuration::populateArmorRatings (CComboBox & comboBox)
{
	populate (ms_armorRatings, comboBox);
}

// ----------------------------------------------------------------------

void Configuration::populateDamageTypes (CComboBox & comboBox)
{
	populate (ms_damageTypes, comboBox);
}

// ----------------------------------------------------------------------

void Configuration::populateCraftingTypes (CComboBox & comboBox)
{
	populate (ms_craftingTypes, comboBox);
}

// ----------------------------------------------------------------------

void Configuration::populateIngredientTypes (CComboBox & comboBox)
{
	populate (ms_ingredientTypes, comboBox);
}

// ----------------------------------------------------------------------

void Configuration::populateResourceTypes (CTreeCtrl & treeCtrl)
{
	populate (ms_resourceTypes, treeCtrl);
}

// ----------------------------------------------------------------------

void Configuration::populateStringTables (CComboBox & comboBox)
{
	populate (ms_stringTables, comboBox);
}

// ----------------------------------------------------------------------

void Configuration::populateXpTypes (CComboBox & comboBox)
{
	populate (ms_xpTypes, comboBox);
}

// ----------------------------------------------------------------------

CString const & Configuration::getDefaultBaseServerDraftSchematicObjectTemplate ()
{
	return !ms_defaultBaseServerDraftSchematicObjectTemplate.IsEmpty () ? ms_defaultBaseServerDraftSchematicObjectTemplate : (!ms_baseServerDraftSchematicObjectTemplates.empty () ? ms_baseServerDraftSchematicObjectTemplates [0] : cms_empty);
}

// ----------------------------------------------------------------------

CString const & Configuration::getDefaultArmorRating ()
{
	return !ms_defaultArmorRating.IsEmpty () ? ms_defaultArmorRating : (!ms_armorRatings.empty () ? ms_armorRatings [0] : cms_empty);
}

// ----------------------------------------------------------------------

CString const & Configuration::getDefaultInteger ()
{
	return cms_zero;
}

// ----------------------------------------------------------------------

CString const & Configuration::getDefaultCraftingType ()
{
	return !ms_defaultCraftingType.IsEmpty () ? ms_defaultCraftingType : (!ms_craftingTypes.empty () ? ms_craftingTypes [0] : cms_empty);
}

// ----------------------------------------------------------------------

CString const & Configuration::getDefaultDamageType ()
{
	return !ms_defaultDamageType.IsEmpty () ? ms_defaultDamageType : (!ms_damageTypes.empty () ? ms_damageTypes [0] : cms_empty);
}

// ----------------------------------------------------------------------

CString const & Configuration::getDefaultIngredientType ()
{
	return !ms_defaultIngredientType.IsEmpty () ? ms_defaultIngredientType : (!ms_ingredientTypes.empty () ? ms_ingredientTypes [0] : cms_empty);
}

// ----------------------------------------------------------------------

CString const & Configuration::getDefaultStringTable ()
{
	return !ms_defaultStringTable.IsEmpty () ? ms_defaultStringTable : (!ms_stringTables.empty () ? ms_stringTables [0] : cms_empty);
}

// ----------------------------------------------------------------------

CString const & Configuration::getDefaultXpType ()
{
	return !ms_defaultXpType.IsEmpty () ? ms_defaultXpType : (!ms_xpTypes.empty () ? ms_xpTypes [0] : cms_empty);
}

// ======================================================================

bool Configuration::loadCfg ()
{
	//-- open the config file
	CStdioFile infile;
	if (!infile.Open (cms_root + "SwgDraftSchematicEditor.cfg", CFile::modeRead | CFile::typeText))
		return false;

	//-- read each line...
	CString line;
	while (infile.ReadString (line))
	{
		//-- see if the line is empty
		line.TrimLeft ();
		line.TrimRight ();
		if (line.GetLength () == 0)
			continue;

		//-- see if the first character is a comment
		int index = line.Find ("#");
		if (index == 0)
			continue;

		if (line == "[SwgDraftSchematicEditor]")
			continue;

		//-- find the =
		index = line.Find ("=");
		if (index == -1)
			continue;

		//-- left half goes in key, right half goes in value
		int const length = line.GetLength ();
		CString const left = line.Left (index);
		CString const right = line.Right (length - index - 1);

		if (left == "serverObjectTemplatePath")
		{
			ms_serverObjectTemplatePath = right;
			ms_serverObjectTemplatePath.Replace ('\\', '/');
		}

		if (left == "draftSchematicDirectory")
			ms_draftSchematicDirectory = right;
	}

	return 
		!ms_serverObjectTemplatePath .IsEmpty () &&
		!ms_draftSchematicDirectory .IsEmpty ();
}

// ======================================================================

bool Configuration::loadIni ()
{
	//-- open the config file
	CStdioFile infile;
	if (!infile.Open (cms_root + "SwgDraftSchematicEditor.ini", CFile::modeRead | CFile::typeText))
		return false;

	//-- read each line...
	CString line;
	while (infile.ReadString (line))
	{
		//-- see if the line is empty
		line.TrimLeft ();
		line.TrimRight ();
		if (line.GetLength () == 0)
			continue;

		//-- see if the first character is a comment
		int index = line.Find ("#");
		if (index == 0)
			continue;

		//-- find the =
		index = line.Find ("=");
		if (index == -1)
			continue;

		//-- left half goes in key, right half goes in value
		int const length = line.GetLength ();
		CString const left = line.Left (index);
		CString right = line.Right (length - index - 1);

		if (left == "serverDraftSchematicObjectTemplateVersion")
			ms_serverDraftSchematicObjectTemplateVersion = right;

		if (left == "serverIntangibleObjectTemplateVersion")
			ms_serverIntangibleObjectTemplateVersion = right;

		if (left == "serverObjectTemplateVersion")
			ms_serverObjectTemplateVersion = right;

		if (left == "sharedDraftSchematicObjectTemplateVersion")
			ms_sharedDraftSchematicObjectTemplateVersion = right;

		if (left == "sharedIntangibleObjectTemplateVersion")
			ms_sharedIntangibleObjectTemplateVersion = right;

		if (left == "sharedObjectTemplateVersion")
			ms_sharedObjectTemplateVersion = right;

		if (left == "baseServerDraftSchematicObjectTemplate")
		{
			if (right [0] == '*')
			{
				right = right.Right (right.GetLength () - 1);
				ms_defaultBaseServerDraftSchematicObjectTemplate = right;
			}

			ms_baseServerDraftSchematicObjectTemplates.push_back (right);
		}

		if (left == "armorRating")
		{
			if (right [0] == '*')
			{
				right = right.Right (right.GetLength () - 1);
				ms_defaultArmorRating = right;
			}

			ms_armorRatings.push_back (right);
		}

		if (left == "damageType")
		{
			if (right [0] == '*')
			{
				right = right.Right (right.GetLength () - 1);
				ms_defaultDamageType = right;
			}

			ms_damageTypes.push_back (right);
		}

		if (left == "craftingType")
		{
			if (right [0] == '*')
			{
				right = right.Right (right.GetLength () - 1);
				ms_defaultCraftingType = right;
			}

			ms_craftingTypes.push_back (right);
		}


		if (left == "ingredientType")
		{
			if (right [0] == '*')
			{
				right = right.Right (right.GetLength () - 1);
				ms_defaultIngredientType = right;
			}

			ms_ingredientTypes.push_back (right);
		}

		if (left == "stringTable")
		{
			if (right [0] == '*')
			{
				right = right.Right (right.GetLength () - 1);
				ms_defaultStringTable = right;
			}

			ms_stringTables.push_back (right);
		}

		if (left == "resourceType")
			ms_resourceTypes.push_back (right);

		if (left == "xpType")
		{
			if (right [0] == '*')
			{
				right = right.Right (right.GetLength () - 1);
				ms_defaultXpType = right;
			}

			ms_xpTypes.push_back (right);
		}
	}

	if (ms_armorRatings.empty () || ms_craftingTypes.empty () || ms_damageTypes.empty () || ms_ingredientTypes.empty () || ms_resourceTypes.empty () || ms_stringTables.empty () || ms_xpTypes.empty ())
		return false;

	return true;
}

// ======================================================================

