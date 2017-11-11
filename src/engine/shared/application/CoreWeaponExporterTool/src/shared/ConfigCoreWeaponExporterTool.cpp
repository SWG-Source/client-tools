// ======================================================================
//
// ConfigCoreWeaponExporterTool.cpp
//
// copyright 2008 Sony Online Entertainment
//
// ======================================================================

#include "FirstCoreWeaponExporterTool.h"
#include "ConfigCoreWeaponExporterTool.h"
#include "sharedFoundation/ConfigFile.h"

//-------------------------------------------------------------------

ConfigCoreWeaponExporterTool::Data* ConfigCoreWeaponExporterTool::data;

//-------------------------------------------------------------------

static ConfigCoreWeaponExporterTool::Data staticData;

//-------------------------------------------------------------------

#define KEY_INT(a,b)    (data->a = ConfigFile::getKeyInt("CoreWeaponExporterTool", #a, b))
#define KEY_BOOL(a,b)   (data->a = ConfigFile::getKeyBool("CoreWeaponExporterTool", #a, b))
#define KEY_FLOAT(a,b)  (data->a = ConfigFile::getKeyFloat("CoreWeaponExporterTool", #a, b))
#define KEY_STRING(a,b) (data->a = ConfigFile::getKeyString("CoreWeaponExporterTool", #a, b))

//-------------------------------------------------------------------

void ConfigCoreWeaponExporterTool::install(void)
{
	data = &staticData;

	KEY_STRING (schematicTemplatePath, "object/draft_schematic/weapon");
	KEY_STRING (slotNameTable, "craft_weapon_ingredients_n");
	KEY_STRING (schematicNameTable, "craft_weapon_ingredients_n");
	KEY_STRING (schematicDescriptionTable, "craft_weapon_ingredients_d");
}

//-------------------------------------------------------------------

void ConfigCoreWeaponExporterTool::remove(void)
{
}


// ======================================================================
