// ======================================================================
//
// ConfigArmorExporterTool.cpp
//
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "FirstArmorExporterTool.h"
#include "ConfigArmorExporterTool.h"
#include "sharedFoundation/ConfigFile.h"

//-------------------------------------------------------------------

ConfigArmorExporterTool::Data* ConfigArmorExporterTool::data;

//-------------------------------------------------------------------

static ConfigArmorExporterTool::Data staticData;

//-------------------------------------------------------------------

#define KEY_INT(a,b)    (data->a = ConfigFile::getKeyInt("ArmorExporterTool", #a, b))
#define KEY_BOOL(a,b)   (data->a = ConfigFile::getKeyBool("ArmorExporterTool", #a, b))
#define KEY_FLOAT(a,b)  (data->a = ConfigFile::getKeyFloat("ArmorExporterTool", #a, b))
#define KEY_STRING(a,b) (data->a = ConfigFile::getKeyString("ArmorExporterTool", #a, b))

//-------------------------------------------------------------------

void ConfigArmorExporterTool::install(void)
{
	data = &staticData;

	KEY_STRING (schematicTemplatePath, "object/draft_schematic/armor");
	KEY_STRING (slotNameTable, "craft_armor_ingredients_n");
	KEY_STRING (schematicNameTable, "craft_armor_ingredients_n");
	KEY_STRING (schematicDescriptionTable, "craft_armor_ingredients_d");
}

//-------------------------------------------------------------------

void ConfigArmorExporterTool::remove(void)
{
}


// ======================================================================
