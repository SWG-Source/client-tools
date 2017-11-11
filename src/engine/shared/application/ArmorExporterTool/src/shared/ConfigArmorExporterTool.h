//
// ConfigArmorExporterTool.h
//
// copyright 2000, verant interactive
//

#ifndef ConfigArmorExporterTool_H
#define ConfigArmorExporterTool_H


//-------------------------------------------------------------------

class ConfigArmorExporterTool
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

inline const char * ConfigArmorExporterTool::getSchematicTemplatePath()
{
	return data->schematicTemplatePath;
}

//-----------------------------------------------------------------------

inline const char * ConfigArmorExporterTool::getSlotNameTable()
{
	return data->slotNameTable;
}

//-----------------------------------------------------------------------

inline const char * ConfigArmorExporterTool::getSchematicNameTable()
{
	return data->schematicNameTable;
}

//-----------------------------------------------------------------------

inline const char * ConfigArmorExporterTool::getSchematicDescriptionTable()
{
	return data->schematicDescriptionTable;
}


// ======================================================================

#endif

