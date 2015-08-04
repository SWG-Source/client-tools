// ======================================================================
//
// ConfigWeaponExporterTool.cpp
//
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "FirstWeaponExporterTool.h"
#include "ConfigWeaponExporterTool.h"
#include "sharedFoundation/ConfigFile.h"

//-------------------------------------------------------------------

ConfigWeaponExporterTool::Data* ConfigWeaponExporterTool::data;

//-------------------------------------------------------------------

static ConfigWeaponExporterTool::Data staticData;

//-------------------------------------------------------------------

#define KEY_INT(a,b)    (data->a = ConfigFile::getKeyInt("WeaponExporterTool", #a, b))
#define KEY_BOOL(a,b)   (data->a = ConfigFile::getKeyBool("WeaponExporterTool", #a, b))
#define KEY_FLOAT(a,b)  (data->a = ConfigFile::getKeyFloat("WeaponExporterTool", #a, b))
#define KEY_STRING(a,b) (data->a = ConfigFile::getKeyString("WeaponExporterTool", #a, b))

//-------------------------------------------------------------------

void ConfigWeaponExporterTool::install(void)
{
	data = &staticData;

	KEY_STRING (schematicTemplatePath, "object/draft_schematic/weapon");
	KEY_STRING (slotNameTable, "craft_weapon_ingredients_n");
	KEY_STRING (schematicNameTable, "craft_weapon_ingredients_n");
	KEY_STRING (schematicDescriptionTable, "craft_weapon_ingredients_d");
}

//-------------------------------------------------------------------

void ConfigWeaponExporterTool::remove(void)
{
}


// ======================================================================
