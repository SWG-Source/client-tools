// ======================================================================
//
// CoreWeaponExporterTool.h
// 
// copyright 2008 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CoreWeaponExporterTool_H
#define INCLUDED_CoreWeaponExporterTool_H

#include <string>
#include <vector>


// ======================================================================

class DataTable;


// ======================================================================

class CoreWeaponExporterTool
{
private:

	enum SchematicType
	{
		ST_weapon,
		ST_slot		// note: not a real schematic type, used to flag the slots for the previous schematic entry
	};

	struct SchematicData
	{
		SchematicType schematicType;
		std::string   schematicName;
		std::string   schematicDescription;
		std::string   craftingScript;
		int           type;
		int           itemsPerContainer;
		int           craftingType;
		int           slots;
		int           complexity;
		int           xp;
		int           xpType;
		int           volume;
		std::string   craftedObjectTemplate;
		uint32        craftedObjectTemplateHash;
		int           minDamageLow;
		int           minDamageHigh;
		int           maxDamageLow;
		int           maxDamageHigh;
		int           attackSpeedLow;
		int           attackSpeedHigh;
		int           woundChanceLow;
		int           woundChanceHigh;
		int           accuracyLow;
		int           accuracyHigh;
		int           hitPointsLow;
		int           hitPointsHigh;
		int           minRangeDistance;
		int           maxRangeDistance;
		int           specialAttackCostLow;
		int           specialAttackCostHigh;
		int           damageType;
		int           elementalType;
		int           elementalValueLow;
		int           elementalValueHigh;
		std::string   exportsTo;
		int           gasQualityLowMin;
		int			  gasQualityLowMax;
		int           gasQualityHighMin;
		int           gasQualityHighMax;
		int           coreLevelLow;
		int           coreLevelHigh;
		int           powerBitTypeLow;
		int           powerBitTypeHigh;
		int           powerBitLow;
		int           powerBitHigh;
		int           coreQualityLowMin;
		int           coreQualityLowMax;
		int           coreQualityHighMin;
		int           coreQualityHighMax;
		int           componentBonusLowMin;
		int           componentBonusLowMax;
		int           componentBonusHighMin;
		int           componentBonusHighMax;
		int			  meleeComponentQualityLowMin;
		int           meleeComponentQualityLowMax;
		int           meleeComponentQualityHighMin;
		int           meleeComponentQualityHighMax;
		int           appearanceBonusLowMin;
		int           appearanceBonusLowMax;
		int           appearanceBonusHighMin;
		int           appearanceBonusHighMax;
	};

	struct SlotData
	{
		int         slotType;
		int         slotOptional;
		std::string slotName;
		std::string slotIngredientName;
		int         slotIngredientCount;
		int         slotAppearance;
	};

private:

	CoreWeaponExporterTool(void);
	CoreWeaponExporterTool(const CoreWeaponExporterTool&);
	CoreWeaponExporterTool &operator =(const CoreWeaponExporterTool&);
	
public:

	CoreWeaponExporterTool(const char *fileName);
	~CoreWeaponExporterTool(void);

	static void run();
	static void usage();
	static void printConfig();
	static std::string getMyExecutablePath();

private:
	static void        fixPath(std::string & fileName);
	static void        createOutputDirectoryForFile(const std::string & fileName);
	static std::string makeSharedTemplateName(const std::string & serverTemplate);
	static std::string makeDataFilename(const std::string & dsrcFilename);
	static void        getDefaultOutputPath(const std::string & inputFileName, std::string & outputFileName);
	static void        removeFileName(std::string & outputFileName);
	static std::string getBaseFileName(const std::string & fileName);
	static bool        getFileFromPerforce(const std::string & filename);
	static bool        addFileToPerforce(const std::string & filename);

	SchematicData data;
	std::vector<SlotData> slotData;

	static bool readSchematicData(DataTable & dt, int row, SchematicData & data);
	static bool readSlotData(DataTable & dt, int row, SlotData & data);

	static bool writeServerTemplate(FILE & fp, const SchematicData & data, const std::vector<SlotData> & slotData);
	static bool writeServerTemplateSlots(FILE & fp, const std::vector<SlotData> & slotData);
	static bool writeSharedTemplate(FILE & fp, const SchematicData & data, const std::vector<SlotData> & slotData);

	static std::string m_inputFile;
	static std::string m_serverOutputFile;
	static std::string m_sharedOutputFile;
};

// ======================================================================

#endif
