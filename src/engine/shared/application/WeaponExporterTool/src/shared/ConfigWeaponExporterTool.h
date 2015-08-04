//
// ConfigWeaponExporterTool.h
//
// copyright 2000, verant interactive
//

#ifndef ConfigWeaponExporterTool_H
#define ConfigWeaponExporterTool_H


//-------------------------------------------------------------------

class ConfigWeaponExporterTool
{
  public:

	struct Data
	{
		const char * schematicTemplatePath;
		const char * slotNameTable;
		const char * schematicNameTable;
		const char * schematicDescriptionTable;
	};

  private:

	static Data *data;

  public:

	static void install    (void);
	static void remove     (void);

	static const char *     getSchematicTemplatePath();
	static const char *     getSlotNameTable();
	static const char *     getSchematicNameTable();
	static const char *     getSchematicDescriptionTable();
};

//-----------------------------------------------------------------------

inline const char * ConfigWeaponExporterTool::getSchematicTemplatePath()
{
	return data->schematicTemplatePath;
}

//-----------------------------------------------------------------------

inline const char * ConfigWeaponExporterTool::getSlotNameTable()
{
	return data->slotNameTable;
}

//-----------------------------------------------------------------------

inline const char * ConfigWeaponExporterTool::getSchematicNameTable()
{
	return data->schematicNameTable;
}

//-----------------------------------------------------------------------

inline const char * ConfigWeaponExporterTool::getSchematicDescriptionTable()
{
	return data->schematicDescriptionTable;
}


// ======================================================================

#endif

