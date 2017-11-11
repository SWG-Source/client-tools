// ======================================================================
//
// DraftSchematic.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgDraftSchematicEditor.h"
#include "DraftSchematic.h"

#include "Configuration.h"

#include <algorithm>

// ======================================================================

namespace DraftSchematicNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	CString getLine (CString & file)
	{
		int index = file.Find ('\n');
		if (index == -1)
			index = 0;

		int const length = file.GetLength ();
		CString const left = file.Left (index);
		file = file.Right (length - index - 1);

		return left;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	CString getToken (CString const & file)
	{
		int index = file.Find ('\n');
		if (index == -1)
			index = 0;

		return file.Left (index);
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	CString loadFileUnmodified (CString const & objectTemplateName)
	{
		CString file;
	
		CStdioFile infile;
		if (!infile.Open (objectTemplateName, CFile::modeRead | CFile::typeText))
			return file;

		//-- read each line...
		CString line;
		while (infile.ReadString (line))
			file += line + "\n";

		return file;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	CString loadFile (CString const & objectTemplateName)
	{
		CString file;
	
		CStdioFile infile;
		if (!infile.Open (objectTemplateName, CFile::modeRead | CFile::typeText))
			return file;

		//-- read each line...
		CString line;
		while (infile.ReadString (line))
		{
			//-- see if the line is empty
			line.TrimLeft ();
			line.TrimRight ();
			if (line.GetLength () == 0)
				continue;

			//-- skip comments
			if (line [0] == '/' && line [1] == '/')
				continue;

			file += line + '\n';
		}

		file.Replace (' ', '\n');
		file.Replace (',', '\n');
		file.Replace ('=', '\n');
		file.Replace ('[', '\n');
		file.Replace (']', '\n');

		while (file.Find ("\n\n") != -1)
			file.Replace ("\n\n", "\n");

		return file;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void parseAttributes (DraftSchematic::Attribute & attribute, CString const & line)
	{
		int const index = line.Find ('.');
		if (index == -1)
		{
			attribute.m_valueType = DraftSchematic::Attribute::VT_integer;
			attribute.m_minimumValue = line;
			attribute.m_maximumValue = line;
		}
		else
		{
			attribute.m_minimumValue = line.Left (index);
			attribute.m_maximumValue = line.Right (line.GetLength () - 2 - index);

			if (line [0] == 'A')
				attribute.m_valueType = DraftSchematic::Attribute::VT_armorRating;

			if (line [0] == 'D')
				attribute.m_valueType = DraftSchematic::Attribute::VT_damageType;
		}
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	CString getShortFileName (CString const & pathName)
	{
		CString result = pathName;

		int const start = max (pathName.ReverseFind ('/'), pathName.ReverseFind ('\\'));
		if (start != -1)
			result = result.Right (result.GetLength () - start - 1);

		int const end = result.ReverseFind ('.');
		if (end != -1)
			result = result.Left (end);

		return result;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
}

using namespace DraftSchematicNamespace;

// ======================================================================

DraftSchematic::Attribute::Attribute () :
	m_nameStringTable (),
	m_nameStringId (),
	m_experimentStringTable (),
	m_experimentStringId (),
	m_valueType (VT_integer),
	m_minimumValue (),
	m_maximumValue ()
{
}

// ----------------------------------------------------------------------

DraftSchematic::Attribute::~Attribute ()
{
}

// ----------------------------------------------------------------------

CString DraftSchematic::Attribute::getName () const
{
	return "[" + m_nameStringTable + "]:@" + m_nameStringId + " [" + m_minimumValue + " to " + m_maximumValue + "]";
}

// ======================================================================

DraftSchematic::Slot::Slot () :
	m_optional (false),
	m_nameStringTable (),
	m_nameStringId (),
	m_ingredientType (),
	m_ingredientName (),
	m_ingredientCount (0)
{
}

// ----------------------------------------------------------------------

DraftSchematic::Slot::~Slot ()
{
}

// ----------------------------------------------------------------------

CString DraftSchematic::Slot::getName () const
{
	CString val;

	val.Format("%d", m_ingredientCount);

	return "[" + m_nameStringTable + "]:@" + m_nameStringId + " [" + m_ingredientType + " / " + m_ingredientName + " / " + val + "]";
}

// ======================================================================

CString DraftSchematic::createServerObjectTemplateName (CString const & rootName)
{
	CString serverObjectTemplateName (rootName);
	serverObjectTemplateName.Replace ('\\', '/');

	return serverObjectTemplateName;
}

// ----------------------------------------------------------------------

CString DraftSchematic::createSharedObjectTemplateName (CString const & rootName)
{
	CString sharedObjectTemplateName (createServerObjectTemplateName (rootName));
	sharedObjectTemplateName.Replace ("server/", "shared/");

	int index = sharedObjectTemplateName.ReverseFind ('/');
	if (index == -1)
		index = 0;
	else
		++index;

	sharedObjectTemplateName.Insert (index, "shared_");

	return sharedObjectTemplateName;
}

// ----------------------------------------------------------------------

void DraftSchematic::createDefaultAttribute (DraftSchematic::Attribute & attribute)
{
	attribute.m_nameStringTable = "crafting";
	attribute.m_nameStringId = "attribute";
	attribute.m_maximumValue = Configuration::getDefaultInteger ();
	attribute.m_minimumValue = Configuration::getDefaultInteger ();
}

// ----------------------------------------------------------------------

void DraftSchematic::createDefaultSlot (DraftSchematic::Slot & slot)
{
	slot.m_nameStringTable = Configuration::getDefaultStringTable ();
	slot.m_nameStringId = "slot";
	slot.m_ingredientType = Configuration::getDefaultIngredientType ();
}

// ======================================================================

DraftSchematic::DraftSchematic () :
	m_baseServerObjectTemplate (),
	m_itemsPerContainer (0),
	m_craftedServerObjectTemplate (),
	m_slotList (),
	m_manufactureScript (),
	m_complexity (0),
	m_overrideCraftingType (false),
	m_overrideCraftingTypeValue (),
	m_overrideDestroyIngredients (false),
	m_overrideDestroyIngredientsValue (false),
	m_overrideXpType (false),
	m_overrideXpTypeValue (),
	m_baseSharedObjectTemplate (),
	m_attributeList (),
	m_craftedSharedObjectTemplate ()
{
}

// ----------------------------------------------------------------------

DraftSchematic::~DraftSchematic ()
{
	reset ();
}

// ----------------------------------------------------------------------

bool DraftSchematic::load (char const * const pathName)
{
	reset ();

	CString rootName (pathName);
	rootName.Replace ('\\', '/');

	CString const serverObjectTemplateName (rootName);
	CString const sharedObjectTemplateName (createSharedObjectTemplateName (rootName));

	CString const serverObjectTemplate = loadFile (serverObjectTemplateName);
	if (serverObjectTemplate.IsEmpty ())
		return false;

	CString const sharedObjectTemplate = loadFile (sharedObjectTemplateName);
	if (sharedObjectTemplate.IsEmpty ())
		return false;

	if (!parseServerObjectTemplate (serverObjectTemplate))
		return false;

	if (!parseSharedObjectTemplate (sharedObjectTemplate))
		return false;

	m_oldServerObjectTemplate = loadFileUnmodified (serverObjectTemplateName);
	m_oldSharedObjectTemplate = loadFileUnmodified (sharedObjectTemplateName);

	return true;
}
	
// ----------------------------------------------------------------------

bool DraftSchematic::save (char const * const rootName)
{
	CString const serverObjectTemplateName (rootName);
	CString const sharedObjectTemplateName (createSharedObjectTemplateName (rootName));

	CString const serverObjectTemplate = generateServerObjectTemplate (serverObjectTemplateName);
	CString const sharedObjectTemplate = generateSharedObjectTemplate (sharedObjectTemplateName);

	{
		CStdioFile outfile;
		if (!outfile.Open (serverObjectTemplateName, CFile::modeWrite | CFile::modeCreate | CFile::typeText))
			return false;

		outfile.WriteString (serverObjectTemplate);
	}

	{
		CStdioFile outfile;
		if (!outfile.Open (sharedObjectTemplateName, CFile::modeWrite | CFile::modeCreate | CFile::typeText))
			return false;

		outfile.WriteString (sharedObjectTemplate);
	}

	return true;
}

// ----------------------------------------------------------------------

void DraftSchematic::reset ()
{
	m_baseServerObjectTemplate.Empty ();
	m_itemsPerContainer = 0;
	m_craftedServerObjectTemplate.Empty ();
	std::for_each (m_slotList.begin (), m_slotList.end (), PointerDeleter ());
	m_slotList.clear ();
	m_manufactureScript.Empty ();
	m_overrideXpTypeValue.Empty ();
	m_complexity = 0;
	m_baseSharedObjectTemplate.Empty ();
	std::for_each (m_attributeList.begin (), m_attributeList.end (), PointerDeleter ());
	m_attributeList.clear ();
	m_craftedSharedObjectTemplate.Empty ();
}

// ----------------------------------------------------------------------

int DraftSchematic::getNumberOfSlots () const
{
	return static_cast<int> (m_slotList.size ());
}

// ----------------------------------------------------------------------

DraftSchematic::Slot const * DraftSchematic::getSlot (int const index) const
{
	return m_slotList [index];
}

// ----------------------------------------------------------------------

DraftSchematic::Slot * DraftSchematic::getSlot (int const index)
{
	return m_slotList [index];
}

// ----------------------------------------------------------------------

void DraftSchematic::addSlot (DraftSchematic::Slot * const slot)
{
	m_slotList.push_back (slot);
}

// ----------------------------------------------------------------------

void DraftSchematic::removeSlot (DraftSchematic::Slot * const slot)
{
	SlotList::iterator iter = std::find (m_slotList.begin (), m_slotList.end (), slot);
	if (iter != m_slotList.end ())
		m_slotList.erase (iter);
}

// ----------------------------------------------------------------------

int DraftSchematic::getNumberOfAttributes () const
{
	return static_cast<int> (m_attributeList.size ());
}

// ----------------------------------------------------------------------

DraftSchematic::Attribute const * DraftSchematic::getAttribute (int const index) const
{
	return m_attributeList [index];
}

// ----------------------------------------------------------------------

DraftSchematic::Attribute * DraftSchematic::getAttribute (int const index)
{
	return m_attributeList [index];
}

// ----------------------------------------------------------------------

void DraftSchematic::addAttribute (DraftSchematic::Attribute * const attribute)
{
	m_attributeList.push_back (attribute);
}

// ----------------------------------------------------------------------

void DraftSchematic::removeAttribute (DraftSchematic::Attribute * const attribute)
{
	AttributeList::iterator iter = std::find (m_attributeList.begin (), m_attributeList.end (), attribute);
	if (iter != m_attributeList.end ())
		m_attributeList.erase (iter);
}

// ----------------------------------------------------------------------

bool DraftSchematic::getOverrideCraftingType () const
{
	return m_overrideCraftingType;
}

// ----------------------------------------------------------------------

void DraftSchematic::setOverrideCraftingType (bool const overrideCraftingType)
{
	m_overrideCraftingType = overrideCraftingType;
}

// ----------------------------------------------------------------------

CString const & DraftSchematic::getOverrideCraftingTypeValue () const
{
	return m_overrideCraftingTypeValue;
}

// ----------------------------------------------------------------------

void DraftSchematic::setOverrideCraftingTypeValue (CString const & overrideCraftingTypeValue)
{
	m_overrideCraftingTypeValue = overrideCraftingTypeValue;
}

// ----------------------------------------------------------------------

bool DraftSchematic::getOverrideDestroyIngredients () const
{
	return m_overrideDestroyIngredients;
}

// ----------------------------------------------------------------------

void DraftSchematic::setOverrideDestroyIngredients (bool const overrideDestroyIngredients)
{
	m_overrideDestroyIngredients = overrideDestroyIngredients;
}

// ----------------------------------------------------------------------

bool DraftSchematic::getOverrideDestroyIngredientsValue () const
{
	return m_overrideDestroyIngredientsValue;
}

// ----------------------------------------------------------------------

void DraftSchematic::setOverrideDestroyIngredientsValue (bool const overrideDestroyIngredientsValue)
{
	m_overrideDestroyIngredientsValue = overrideDestroyIngredientsValue;
}

// ----------------------------------------------------------------------

bool DraftSchematic::getOverrideXpType () const
{
	return m_overrideXpType;
}

// ----------------------------------------------------------------------

void DraftSchematic::setOverrideXpType (bool const overrideXpType)
{
	m_overrideXpType = overrideXpType;
}

// ----------------------------------------------------------------------

CString const & DraftSchematic::getOverrideXpTypeValue () const
{
	return m_overrideXpTypeValue;
}

// ----------------------------------------------------------------------

void DraftSchematic::setOverrideXpTypeValue (CString const & overrideXpTypeValue)
{
	m_overrideXpTypeValue = overrideXpTypeValue;
}

// ----------------------------------------------------------------------

CString const & DraftSchematic::getBaseServerObjectTemplate () const
{
	return m_baseServerObjectTemplate;
}

// ----------------------------------------------------------------------

void DraftSchematic::setBaseServerObjectTemplate (CString const & baseServerObjectTemplate)
{
	m_baseServerObjectTemplate = baseServerObjectTemplate;
}

// ----------------------------------------------------------------------

CString const & DraftSchematic::getBaseSharedObjectTemplate () const
{
	return m_baseSharedObjectTemplate;
}

// ----------------------------------------------------------------------

void DraftSchematic::setBaseSharedObjectTemplate (CString const & baseSharedObjectTemplate)
{
	m_baseSharedObjectTemplate = baseSharedObjectTemplate;
}

// ----------------------------------------------------------------------

int DraftSchematic::getItemsPerContainer () const
{
	return m_itemsPerContainer;
}

// ----------------------------------------------------------------------

void DraftSchematic::setItemsPerContainer (int itemsPerContainer)
{
	m_itemsPerContainer = itemsPerContainer;
}

// ----------------------------------------------------------------------

CString const & DraftSchematic::getCraftedServerObjectTemplate () const
{
	return m_craftedServerObjectTemplate;
}

// ----------------------------------------------------------------------

void DraftSchematic::setCraftedServerObjectTemplate (CString const & craftedServerObjectTemplate)
{
	m_craftedServerObjectTemplate = craftedServerObjectTemplate;
}

// ----------------------------------------------------------------------

CString const & DraftSchematic::getCraftedSharedObjectTemplate () const
{
	return m_craftedSharedObjectTemplate;
}

// ----------------------------------------------------------------------

void DraftSchematic::setCraftedSharedObjectTemplate (CString const & craftedSharedObjectTemplate)
{
	m_craftedSharedObjectTemplate = craftedSharedObjectTemplate;
}

// ----------------------------------------------------------------------

CString const & DraftSchematic::getManufactureScript () const
{
	return m_manufactureScript;
}

// ----------------------------------------------------------------------

void DraftSchematic::setManufactureScript (CString const & manufactureScript)
{
	m_manufactureScript = manufactureScript;
}

// ----------------------------------------------------------------------

int DraftSchematic::getComplexity () const
{
	return m_complexity;
}

// ----------------------------------------------------------------------

void DraftSchematic::setComplexity (int complexity)
{
	m_complexity = complexity;
}

// ----------------------------------------------------------------------

CString const & DraftSchematic::getOldServerObjectTemplate () const
{
	return m_oldServerObjectTemplate;
}

// ----------------------------------------------------------------------

CString const & DraftSchematic::getOldSharedObjectTemplate () const
{
	return m_oldSharedObjectTemplate;
}

// ----------------------------------------------------------------------

CString DraftSchematic::generateServerObjectTemplate (CString const & serverObjectTemplateName, bool const header) const
{
	CString result;
	CString buffer;

	if (header)
	{
		result += "// ======================================================================\n//\n";
		result += "// " + getShortFileName (serverObjectTemplateName) + ".tpf\n";
		result += "// generated with SwgDraftSchematicEditor -- DO NOT EDIT THIS FILE!\n";
		result += "//\n// ======================================================================\n\n";
	}

	result += "@base " + m_baseServerObjectTemplate + "\n\n";
	result += "@class draft_schematic_object_template " + Configuration::getServerDraftSchematicObjectTemplateVersion () + "\n\n";

	buffer.Format ("itemsPerContainer = %i\n", m_itemsPerContainer);
	result += buffer;

	if (m_overrideCraftingType)
		result += "category = " + m_overrideCraftingTypeValue + "\n";
	
	result += "craftedObjectTemplate = \"" + m_craftedServerObjectTemplate + "\"\n";
	result += "slots = [\n";
	
	for (int i = 0; i < getNumberOfSlots (); ++i)
	{
		DraftSchematic::Slot const * const slot = getSlot (i);
		result += "\t[\n";

		buffer.Format ("\toptional = %s,\n", slot->m_optional ? "true" : "false");
		result += buffer;

		result += "\tname = \"" + slot->m_nameStringTable + "\" \"" + slot->m_nameStringId + "\",\n";
		result += "\toptions = [\n";

		buffer.Format ("\t\t[ingredientType=%s, ingredients = [[name=\"%s\" \"%s\", ingredient=\"%s\", count=%i]], complexity = 0, skillCommand = \"unskilled\"]],\n", slot->m_ingredientType, slot->m_nameStringTable, slot->m_nameStringId, slot->m_ingredientName, slot->m_ingredientCount);
		result += buffer;

		result += "\toptionalSkillCommand = \"\",\n";
		result += "\tcomplexity = 0,\n";
		result += "\tappearance = \"\" ]";

		if (i < getNumberOfSlots () - 1)
			result += ",\n";
	}

	result += "]\n";

	result += "skillCommands = [\"unskilled\"]\n";

	if (m_overrideDestroyIngredients)
	{
		buffer.Format ("destroyIngredients = %s\n", m_overrideDestroyIngredientsValue ? "true" : "false");
		result += buffer;
	}

	result += "manufactureScripts = [ \"" + m_manufactureScript + "\" ]\n";
	result += "\n";
	result += "@class intangible_object_template " + Configuration::getServerIntangibleObjectTemplateVersion () + "\n\n";
	result += "@class object_template " + Configuration::getServerObjectTemplateVersion () + "\n\n";

	if (m_overrideXpType)
		result += "xpPoints = [[type = " + m_overrideXpTypeValue + ", level = 0, value = 0]]\n";

	buffer.Format ("complexity = %i\n", m_complexity);
	result += buffer;

	CString sharedObjectTemplateName (createSharedObjectTemplateName (serverObjectTemplateName));
	int const index = sharedObjectTemplateName.Find ("object/");
	if (index != -1)
		sharedObjectTemplateName = sharedObjectTemplateName.Right (sharedObjectTemplateName.GetLength () - index);
	sharedObjectTemplateName.Replace (".tpf", ".iff");

	result += "sharedTemplate = \"" + sharedObjectTemplateName + "\"\n";
	result += "volume = 1\n";
	
	return result;
}

// ----------------------------------------------------------------------

CString DraftSchematic::generateSharedObjectTemplate (CString const & sharedObjectTemplateName, bool const header) const
{
	CString result;
	CString buffer;

	if (header)
	{
		result += "// ======================================================================\n//\n";
		result += "// " + getShortFileName (sharedObjectTemplateName) + ".tpf\n";
		result += "// generated with SwgDraftSchematicEditor -- DO NOT EDIT THIS FILE!\n";
		result += "//\n// ======================================================================\n\n";
	}

	result += "@base " + m_baseSharedObjectTemplate + "\n\n";
	result += "@class draft_schematic_object_template " + Configuration::getSharedDraftSchematicObjectTemplateVersion () + "\n\n";

	result += "attributes = [\n";
	{
		for (int i = 0; i < getNumberOfAttributes (); ++i)
		{
			DraftSchematic::Attribute const * const attribute = getAttribute (i);

			buffer.Format ("\t[name = \"%s\" \"%s\", experiment = \"%s\" \"%s\", value = %s..%s]", attribute->m_nameStringTable, attribute->m_nameStringId, attribute->m_experimentStringTable, attribute->m_experimentStringId, attribute->m_minimumValue, attribute->m_maximumValue);
			result += buffer;

			if (i < getNumberOfAttributes () - 1)
				result += ",\n";
		}

		result += "]\n\n";
	}

	result += "slots = [\n";
	{
		for (int i = 0; i < getNumberOfSlots (); ++i)
		{
			DraftSchematic::Slot const * const slot = getSlot (i);

			buffer.Format ("\t[name = \"%s\" \"%s\", hardpoint = \"\"]", slot->m_nameStringTable, slot->m_nameStringId);
			result += buffer;

			if (i < getNumberOfSlots () - 1)
				result += ",\n";
		}

		result += "]\n\n";
	}

	result += "craftedSharedTemplate=\"" + m_craftedSharedObjectTemplate + "\"\n";
	result += "\n";

	result += "@class intangible_object_template " + Configuration::getSharedIntangibleObjectTemplateVersion () + "\n\n";
	result += "@class object_template " + Configuration::getSharedObjectTemplateVersion () + "\n\n";

	result += "snapToTerrain = false\n";
	result += "containerType = CT_none\n";
	result += "tintPalette = \"\"\n";
	result += "slotDescriptorFilename = \"\"\n";
	result += "arrangementDescriptorFilename = \"abstract/slot/arrangement/arrangement_datapad.iff\"\n";
	result += "portalLayoutFilename = \"\"\n";
	result += "clientDataFile = \"\"\n";

	return result;
}

// ======================================================================

bool DraftSchematic::parseServerObjectTemplate (CString file)
{
	//-- read base
	if (getLine (file) != "@base")
		return false;

	m_baseServerObjectTemplate = getLine (file);

	if (getLine (file) != "@class")
		return false;

	if (getLine (file) != "draft_schematic_object_template")
		return false;

	getLine (file);

	while (!file.IsEmpty ())
	{
		CString line = getLine (file);

		if (line == "@class")
			break;
		else
			if (line == "slots")
			{
				do
				{
					CString const token = getToken (file);
					if (token != "optional")
						break;

					Slot slot;

					if (getLine (file) != "optional")
						return false;

					slot.m_optional = getLine (file) == "true";

					if (getLine (file) != "name")
						return false;

					slot.m_nameStringTable = getLine (file);
					slot.m_nameStringTable.Remove ('\"');

					slot.m_nameStringId = getLine (file);
					slot.m_nameStringId.Remove ('\"');

					if (getLine (file) != "options")
						return false;

					if (getLine (file) != "ingredientType")
						return false;

					slot.m_ingredientType = getLine (file);

					if (getLine (file) != "ingredients")
						return false;

					if (getLine (file) != "name")
						return false;

					getLine (file);
					getLine (file);

					if (getLine (file) != "ingredient")
						return false;

					slot.m_ingredientName = getLine (file);
					slot.m_ingredientName.Remove ('\"');

					if (getLine (file) != "count")
						return false;

					slot.m_ingredientCount = atoi (getLine (file));

					if (getLine (file) != "complexity")
						return false;

					getLine (file);

					if (getLine (file) != "skillCommand")
						return false;

					getLine (file);

					if (getLine (file) != "optionalSkillCommand")
						return false;

					getLine (file);

					if (getLine (file) != "complexity")
						return false;

					getLine (file);

					if (getLine (file) != "appearance")
						return false;

					getLine (file);

					m_slotList.push_back (new Slot (slot));
				} while (!file.IsEmpty ());
			}
			else
				if (line == "itemsPerContainer")
					m_itemsPerContainer = atoi (getLine (file));
				else
					if (line == "craftedObjectTemplate")
					{
						m_craftedServerObjectTemplate = getLine (file);
						m_craftedServerObjectTemplate.Remove ('\"');
					}
					else
						if (line == "category")
						{
							m_overrideCraftingType = true;
							m_overrideCraftingTypeValue = getLine (file);
						}
						else
							if (line == "skillCommands")
								getLine (file);
							else
								if (line == "destroyIngredients")
								{
									m_overrideDestroyIngredients = true;
									m_overrideDestroyIngredientsValue = getLine (file) == "true";
								}
								else
									if (line == "manufactureScripts")
									{
										m_manufactureScript = getLine (file);
										m_manufactureScript.Remove ('\"');
									}
	}

	if (getLine (file) != "intangible_object_template")
		return false;

	getLine (file);

	if (getLine (file) != "@class")
		return false;

	if (getLine (file) != "object_template")
		return false;

	getLine (file);

	while (!file.IsEmpty ())
	{
		CString line = getLine (file);
		if (line == "xpPoints")
		{
			if (getLine (file) != "type")
				return false;

			m_overrideXpType = true;
			m_overrideXpTypeValue = getLine (file);

			getLine (file);
			getLine (file);
			getLine (file);
			getLine (file);
		}
		else
			if (line == "complexity")
				m_complexity = atoi (getLine (file));
			else
				getLine (file);
	}

	return true;
}

// ----------------------------------------------------------------------

bool DraftSchematic::parseSharedObjectTemplate (CString file)
{
	//-- read base
	if (getLine (file) != "@base")
		return false;

	m_baseSharedObjectTemplate = getLine (file);

	if (getLine (file) != "@class")
		return false;

	if (getLine (file) != "draft_schematic_object_template")
		return false;

	getLine (file);

	if (getLine (file) != "attributes")
		return false;

	CString previousLine;
	bool doneSlots = false;
	do
	{
		previousLine = getLine (file);
		doneSlots = previousLine != "name";
		if (!doneSlots)
		{
			Attribute attribute;

			attribute.m_nameStringTable = getLine (file);
			attribute.m_nameStringTable.Remove ('\"');

			attribute.m_nameStringId = getLine (file);
			attribute.m_nameStringId.Remove ('\"');

			if (getLine (file) != "experiment")
				return false;

			attribute.m_experimentStringTable = getLine (file);
			attribute.m_experimentStringTable.Remove ('\"');

			attribute.m_experimentStringId = getLine (file);
			attribute.m_experimentStringId.Remove ('\"');

			if (getLine (file) != "value")
				return false;

			parseAttributes (attribute, getLine (file));

			m_attributeList.push_back (new Attribute (attribute));
		}
	} while (!doneSlots && !file.IsEmpty ());

	if (previousLine != "slots")
		return false;

	doneSlots = false;
	do
	{
		previousLine = getLine (file);
		doneSlots = previousLine != "name";
		if (!doneSlots)
		{
			getLine (file);
			getLine (file);

			if (getLine (file) != "hardpoint")
				return false;

			getLine (file);
		}
	} while (!doneSlots && !file.IsEmpty ());

	if (previousLine != "craftedSharedTemplate")
		return false;

	m_craftedSharedObjectTemplate = getLine (file);
	m_craftedSharedObjectTemplate.Remove ('\"');
	
	return true;
}

// ======================================================================

