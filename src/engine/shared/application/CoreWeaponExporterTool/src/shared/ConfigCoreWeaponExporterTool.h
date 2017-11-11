//
// ConfigCoreWeaponExporterTool.h
//

#ifndef ConfigCoreWeaponExporterTool_H
#define ConfigCoreWeaponExporterTool_H


//-------------------------------------------------------------------

class ConfigCoreWeaponExporterTool
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

inline const char * ConfigCoreWeaponExporterTool::getSchematicTemplatePath()
{
	return data->schematicTemplatePath;
}

//-----------------------------------------------------------------------

inline const char * ConfigCoreWeaponExporterTool::getSlotNameTable()
{
	return data->slotNameTable;
}

//-----------------------------------------------------------------------

inline const char * ConfigCoreWeaponExporterTool::getSchematicNameTable()
{
	return data->schematicNameTable;
}

//-----------------------------------------------------------------------

inline const char * ConfigCoreWeaponExporterTool::getSchematicDescriptionTable()
{
	return data->schematicDescriptionTable;
}


// ======================================================================

#endif

