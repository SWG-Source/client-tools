// ======================================================================
//
// WeaponExporterTool.cpp
//
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "FirstWeaponExporterTool.h"
#include "WeaponExporterTool.h"

#include "ConfigWeaponExporterTool.h"
#include "serverGame/ServerObjectTemplate.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFile/Iff.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CommandLine.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedGame/CraftingData.h"
#include "sharedIoWin/SetupSharedIoWin.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <cstdio>
#include <string>

#if defined(PLATFORM_WIN32)
#define popen _popen
#define pclose _pclose
const static std::string REDIRECT_TO_NULL(" >NUL 2>&1");
#undef WARNING
#define WARNING CONSOLE_WARNING
#elif defined(PLATFORM_LINUX)
const static std::string REDIRECT_TO_NULL(" >/dev/null 2>&1");
#endif

// ======================================================================

const std::string NULL_STRING_ID("\"\" \"\"");
const std::string EXP_DAMAGE_STRING_ID("\"crafting\" \"expDamage\"");
const std::string EXP_ELEMENTAL_VALUE_STRING_ID("\"crafting\" \"expElementalValue\"");
const std::string EXP_SPEED_STRING_ID("\"crafting\" \"expSpeed\"");
const std::string EXP_EFFECIENCY_STRING_ID("\"crafting\" \"expEffeciency\"");
const std::string EXP_ACCURACY_STRING_ID("\"crafting\" \"expAccuracy\"");


// make sure the DatatableColumnsIndex below matches this list
const char * DATATABLE_COLUMNS[] = 
{
	"entry_type",
	"schematic_name",
	"schematic_description",
	"crafting_script",
	"items_per_container",
	"crafting_type",
	"slots",
	"complexity",
	"xp",
	"xp_type",
	"volume",
	"crafted_object_template",
	"min_damage_low",
	"min_damage_high",
	"max_damage_low",
	"max_damage_high",
	"attack_speed_low",
	"attack_speed_high",
	"wound_chance_low",
	"wound_chance_high",
	"accuracy_low",
	"accuracy_high",
	"hit_points_low",
	"hit_points_high",
	"min_range_distance",
	"max_range_distance",
	"special_attack_cost_low",
	"special_attack_cost_high",
	"damage_type",
	"elemental_type",
	"elemental_value_low",
	"elemental_value_high",
	"slot_type",
	"slot_optional",
	"slot_name",
	"slot_ingredient_name",
	"slot_ingredient_count",
	"slot_appearance"
};
#define DATATABLE_COLUMNS_SIZE (sizeof(DATATABLE_COLUMNS)/sizeof(DATATABLE_COLUMNS[0]))

// make sure these are in the proper order to match DATATABLE_COLUMNS
enum DatatableColumnsIndex
{
	DCI_entryType,
	DCI_schematicName,
	DCI_schematicDescription,
	DCI_craftingScript,
	DCI_itemsPerContainer,
	DCI_craftingType,
	DCI_slots,
	DCI_complexity,
	DCI_xp,
	DCI_xpType,
	DCI_volume,
	DCI_craftedObjectTemplate,
	DCI_minDamageLow,
	DCI_minDamageHigh,
	DCI_maxDamageLow,
	DCI_maxDamageHigh,
	DCI_attackSpeedLow,
	DCI_attackSpeedHigh,
	DCI_woundChanceLow,
	DCI_woundChanceHigh,
	DCI_accuracyLow,
	DCI_accuracyHigh,
	DCI_hitPointsLow,
	DCI_hitPointsHigh,
	DCI_minRangeDistance,
	DCI_maxRangeDistance,
	DCI_specialAttackCostLow,
	DCI_specialAttackCostHigh,
	DCI_damageType,
	DCI_elementalType,
	DCI_elementalValueLow,
	DCI_elementalValueHigh,
	DCI_slotType,
	DCI_slotOptional,
	DCI_slotName,
	DCI_slotIngredientName,
	DCI_slotIngredientCount,
	DCI_slotAppearance
};

const char * INGREDIENT_TYPES[] = 
{
	"IT_none",
	"IT_item",
	"IT_template",
	"IT_resourceType",
	"IT_resourceClass",
	"IT_templateGeneric",
	"IT_schematic",
	"IT_schematicGeneric"
};
#define INGREDIENT_TYPES_SIZE (sizeof(INGREDIENT_TYPES)/sizeof(INGREDIENT_TYPES[0]))

const char * SLOT_APPEARANCES[] =
{
	"",
	"scope",
	"muzzle",
	"stock"
};
#define SLOT_APPEARANCES_SIZE (sizeof(SLOT_APPEARANCES)/sizeof(SLOT_APPEARANCES[0]))

const char * XP_TYPES[] = 
{
	"XP_crafting",
	"XP_craftingWeapons",
	"XP_craftingWeaponsMelee",
	"XP_craftingWeaponsRanged",
	"XP_craftingWeaponsMunition",
	"XP_jediGeneral"
};
#define XP_TYPES_SIZE (sizeof(XP_TYPES)/sizeof(XP_TYPES[0]))


// ======================================================================

std::string WeaponExporterTool::m_inputFile;
std::string WeaponExporterTool::m_serverOutputFile;
std::string WeaponExporterTool::m_sharedOutputFile;

extern int main(int argc, char **argv);


// ======================================================================
// command line stuff
// ======================================================================

static const char * const LNAME_HELP                 = "help";
static const char * const LNAME_INPUT_FILE           = "inputFile";
static const char * const LNAME_OUTPUT_FILE          = "outputFile";
static const char * const LNAME_TEST                 = "test";
static const char * const LNAME_CONFIG_OPTIONS       = "config";
static const char         SNAME_HELP                 = 'h';
static const char         SNAME_INPUT_FILE           = 'i';
static const char         SNAME_CONFIG_OPTIONS       = 'c';

static CommandLine::OptionSpec optionSpecArray[] =
{
	OP_BEGIN_SWITCH(OP_NODE_REQUIRED),

		// help
		OP_SINGLE_SWITCH_NODE(SNAME_HELP, LNAME_HELP, OP_ARG_NONE, OP_MULTIPLE_DENIED),

		// get config options
		OP_SINGLE_SWITCH_NODE(SNAME_CONFIG_OPTIONS, LNAME_CONFIG_OPTIONS, OP_ARG_NONE, OP_MULTIPLE_DENIED),

		// real options
		OP_BEGIN_SWITCH_NODE(OP_MULTIPLE_DENIED),
			OP_BEGIN_LIST(),

				// iff file required
				OP_SINGLE_LIST_NODE(SNAME_INPUT_FILE, LNAME_INPUT_FILE, OP_ARG_REQUIRED, OP_MULTIPLE_DENIED,  OP_NODE_REQUIRED),

			OP_END_LIST(),
		OP_END_SWITCH_NODE(),


	OP_END_SWITCH()
};

static const int optionSpecCount = sizeof(optionSpecArray) / sizeof(optionSpecArray[0]);

// ======================================================================

int main(int argc, char **argv)
{
	//-- thread
	SetupSharedThread::install();

	//-- debug
	SetupSharedDebug::install(4096);

	{
		std::string myExePath = WeaponExporterTool::getMyExecutablePath() + "/tools.cfg";
		SetupSharedFoundation::Data data(SetupSharedFoundation::Data::D_console);
		data.argc  = argc;
		data.argv  = argv;
		data.configFile = myExePath.c_str();
		SetupSharedFoundation::install(data);
	}

	//-- file
	SetupSharedCompression::install();
	SetupSharedFile::install(false);
	DataTableManager::install();
	ConfigWeaponExporterTool::install();

	//-- iowin
	SetupSharedIoWin::install();

	SetupSharedFoundation::callbackWithExceptionHandling(WeaponExporterTool::run);
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	return 0;
}

// ----------------------------------------------------------------------
void WeaponExporterTool::run(void)
{
	printf ("Weapon exporter tool " __DATE__ " " __TIME__ "\n");

	FATAL(INGREDIENT_TYPES_SIZE - 1 != Crafting::IngredientType_Last, ("Error in the number of ingredient types strings"));


	// handle options
	const CommandLine::MatchCode mc = CommandLine::parseOptions(optionSpecArray, optionSpecCount);

	if (mc != CommandLine::MC_MATCH)
	{
		printf("Invalid command line specified.  Printing usage...\n");
		usage();
		return;
	}

	if (CommandLine::getOccurrenceCount(SNAME_HELP))
	{
		usage();
		return;
	}

	if (CommandLine::getOccurrenceCount(SNAME_CONFIG_OPTIONS))
	{
		printConfig();
		return;
	}

	m_inputFile = CommandLine::getOptionString(SNAME_INPUT_FILE);

	if (m_inputFile.find(".iff") == std::string::npos)
	{
		usage();
		return;
	}

	DataTable * dt = DataTableManager::getTable(m_inputFile, true);
	if (dt == NULL)
	{
		printf("Unable to open datatable %s\n", m_inputFile.c_str());
		return;
	}

	// verify the datatable
	for (unsigned i = 0; i < DATATABLE_COLUMNS_SIZE; ++i)
	{
		if (dt->findColumnNumber(DATATABLE_COLUMNS[i]) < 0)
		{
			printf("Error in datatable, unable to find column %s\n", DATATABLE_COLUMNS[i]);
			return;
		}
	}

	std::string outputPath;
	getDefaultOutputPath(m_inputFile, outputPath);

	SchematicData data;
	std::vector<SlotData> slotData;
	for (int row = 0; row < dt->getNumRows(); ++row)
	{
		// read the schematic type we're going to create
		int schematic_type = dt->getIntValue(DATATABLE_COLUMNS[DCI_entryType], row);

		// read the schematic and slot data
		data.schematicType = static_cast<SchematicType>(schematic_type);
		FATAL(data.schematicType == ST_slot, ("Row %d is marked as a slot row, but we expected schematic data", row));
		if (!readSchematicData(*dt, row, data))
			return;

		slotData.clear();
		for (int i = 0; i < data.slots; ++i)
		{
			slotData.push_back(SlotData());
			if (!readSlotData(*dt, ++row, slotData.back()))
				return;
		}

		printf("Creating %s\n", data.schematicName.c_str());

		m_serverOutputFile = outputPath + "/" + data.schematicName + ".tpf";
		m_sharedOutputFile = makeSharedTemplateName(m_serverOutputFile);

		bool editedServerFile = getFileFromPerforce(m_serverOutputFile);
		bool editedSharedFile = getFileFromPerforce(m_sharedOutputFile);

		// write the server template
		FILE * fp = fopen(m_serverOutputFile.c_str(), "wt");
		FATAL(fp == NULL, ("Unable to open/create file %s", m_serverOutputFile.c_str()));
		bool result = writeServerTemplate(*fp, data, slotData);
		fclose(fp);
		FATAL(result == false, ("Error creating server template %s", m_serverOutputFile.c_str()));

		// write the shared template		
		fp = fopen(m_sharedOutputFile.c_str(), "wt");
		FATAL(fp == NULL, ("Unable to open/create file %s", m_sharedOutputFile.c_str()));
		result = writeSharedTemplate(*fp, data, slotData);
		fclose(fp);
		FATAL(result == false, ("Error creating shared template %s", m_sharedOutputFile.c_str()));

		if (!editedServerFile)
			addFileToPerforce(m_serverOutputFile);
		if (!editedSharedFile)
			addFileToPerforce(m_sharedOutputFile);

		// compile the templates
		editedServerFile = getFileFromPerforce(makeDataFilename(m_serverOutputFile));
		editedSharedFile = getFileFromPerforce(makeDataFilename(m_sharedOutputFile));

		std::string command = "TemplateCompiler -compile " + m_serverOutputFile;
		int compileResult = system(command.c_str());
		FATAL(compileResult != 0, ("Error compiling template %s", m_serverOutputFile.c_str()));
		command = "TemplateCompiler -compile " + m_sharedOutputFile;
		compileResult = system(command.c_str());
		FATAL(compileResult != 0, ("Error compiling template %s", m_sharedOutputFile.c_str()));

		if (!editedServerFile)
			addFileToPerforce(makeDataFilename(m_serverOutputFile));
		if (!editedSharedFile)
			addFileToPerforce(makeDataFilename(m_sharedOutputFile));
	}
}

// ----------------------------------------------------------------------

std::string WeaponExporterTool::getMyExecutablePath()
{
#if defined(PLATFORM_WIN32)
	char buffer[MAX_PATH]; 
	HMODULE handle = GetModuleHandle("WeaponExporterTool.exe");
	if (GetModuleFileName(handle, buffer, MAX_PATH) != 0)
	{
		char * lastSlash = strrchr(buffer, '\\');
		if (lastSlash != NULL)
			*lastSlash = '\0';
		return buffer;
	}
#elif defined(PLATFORM_LINUX)
	char linkname[64]; /* /proc/<pid>/exe */
	
	if (snprintf(linkname, sizeof(linkname), "/proc/%i/exe", static_cast<int>(getpid())) >= 0)
	{
		char buffer[1024 * 2];
		if (readlink(linkname, buffer, sizeof(buffer)) != -1)
		{
			char * lastSlash = strrchr(buffer, '/');
			if (lastSlash != NULL)
				*lastSlash = '\0';
			return buffer;
		}
	}
#endif
	return "";
}

// ----------------------------------------------------------------------

void WeaponExporterTool::fixPath(std::string & fileName)
{
#if defined(PLATFORM_WIN32)
char fromChar = '/';
char toChar = '\\';
#elif defined(PLATFORM_LINUX)
char fromChar = '\\';
char toChar = '/';
#endif

	// convert the path separators to the correct type
	std::string::size_type i;
	while ((i = fileName.find(fromChar)) != std::string::npos)
		fileName[i] = toChar;

	// force the path to be lowercase
	for (std::string::iterator iter = fileName.begin(); iter != fileName.end(); ++iter)
	{
		if (isupper(*iter))
			*iter = static_cast<char>(tolower(*iter));
	}
}

// ----------------------------------------------------------------------

void WeaponExporterTool::createOutputDirectoryForFile(const std::string & fileName)
{
	std::string::size_type slashPos = fileName.find_last_of('\\');
	if (slashPos == std::string::npos)
	{
		slashPos = fileName.find_last_of('/');
		if (slashPos == std::string::npos)
			return;
	}

	std::string directoryName(fileName, 0, slashPos);
#if defined(PLATFORM_WIN32)
	int result = CreateDirectory(directoryName.c_str(), NULL);
#elif defined(PLATFORM_LINUX)
	// make sure slashes are forward
	{
		std::string::size_type i;
		while ((i = directoryName.find('\\')) != std::string::npos)
			directoryName[i] = '/';
	} 
	int result = mkdir(directoryName.c_str(), 0777);
#else
	#error unsupported platform
#endif
	UNREF(result);
//	DEBUG_FATAL(result == 0, ("Could not create directory %s from file %s", directoryName.c_str(), fileName.c_str()));
}

// ----------------------------------------------------------------------

std::string WeaponExporterTool::makeSharedTemplateName(const std::string & serverTemplate)
{
	std::string sharedTemplate(serverTemplate);
	std::string::size_type sysServerPos = sharedTemplate.find("sys.server"); 
	if (sysServerPos != std::string::npos)
		sharedTemplate.replace(sysServerPos, 10, "sys.shared");

	// find the last slash
	std::string::size_type fwdSlashPos = sharedTemplate.find_last_of('/');
	std::string::size_type bkSlashPos = sharedTemplate.find_last_of('\\');
	
	// if there is no slash, just prepend shared_
	if (fwdSlashPos == std::string::npos && bkSlashPos == std::string::npos)
		return "shared_" + sharedTemplate;
	
	// use the last slash as the insertion point
	std::string::size_type pos;
	if (fwdSlashPos == std::string::npos)
		pos = bkSlashPos;
	else if (bkSlashPos == std::string::npos)
		pos = fwdSlashPos;
	else
		pos = fwdSlashPos > bkSlashPos ? fwdSlashPos : bkSlashPos;
	sharedTemplate.insert(pos + 1, "shared_");
	return sharedTemplate;
}

// ----------------------------------------------------------------------

std::string WeaponExporterTool::makeDataFilename(const std::string & dsrcFilename)
{
	std::string dataFilename(dsrcFilename);
	dataFilename.replace(dataFilename.find("dsrc"), 4, "data");

	std::string::size_type extPos = dataFilename.find(".tpf");
	if (extPos == std::string::npos)
		extPos = dataFilename.find(".xml");
	if (extPos == std::string::npos)
		extPos = dataFilename.find(".tab");
	
	if (extPos != std::string::npos)
		dataFilename.replace(extPos, 4, ".iff");

	return dataFilename;
}

// ----------------------------------------------------------------------

void WeaponExporterTool::getDefaultOutputPath(const std::string & inputFileName, std::string & outputFileName)
{
	// we expect a path "/foo/.../data/.../game/datatables/..."
	// we want to convert it to "/foo/.../dsrc/.../game/", and the concat the schematic path from the config setting

	char buffer [2048];
#if defined(PLATFORM_WIN32)
	char* tmp = 0;
	int retval = GetFullPathName (inputFileName.c_str(), sizeof(buffer), buffer, &tmp);
	FATAL(retval <= 0 || retval >= sizeof(buffer), ("Get FullPathName failed with %d", retval));
#elif defined(PLATFORM_LINUX)
	{
		// make sure slashes are forward
		std::string fileName = inputFileName;
		std::string::size_type i;
		while ((i = fileName.find('\\')) != std::string::npos)
			fileName[i] = '/';
		char *retval = realpath(fileName.c_str(), buffer);
		FATAL(!retval, ("realpath failed"));
	} 
#else
	#error unsupported platform
#endif

	std::string temp(buffer);
	std::string::size_type dataPos = temp.find("data");
	if (dataPos != std::string::npos)
		temp.replace(dataPos, 4, "dsrc");
	else
	{
		FATAL(temp.find("dsrc") == std::string::npos, ("can't find <data> or <dsrc> in input path %s", inputFileName.c_str()));
	}

	std::string::size_type datatablesPos = temp.find("datatables");
	FATAL(datatablesPos == std::string::npos, ("can't find <datatables> in input path %s", inputFileName.c_str()));
	
	outputFileName = temp.substr(0, datatablesPos) + ConfigWeaponExporterTool::getSchematicTemplatePath();
}

// ----------------------------------------------------------------------

void WeaponExporterTool::removeFileName(std::string & outputFileName)
{
#if defined(PLATFORM_WIN32)
	outputFileName.erase(outputFileName.find_last_of('\\'));
	outputFileName.append("\\");
#elif defined(PLATFORM_LINUX)
	outputFileName.erase(outputFileName.find_last_of('/'));
	outputFileName.append("/");
#else
	#error unsupported platform
#endif
}

// ----------------------------------------------------------------------

std::string WeaponExporterTool::getBaseFileName(const std::string & fileName)
{
	std::string baseName;
	std::string::size_type fwdSlashPos = fileName.find_last_of('/');
	std::string::size_type bkSlashPos = fileName.find_last_of('\\');
	
	if (fwdSlashPos == std::string::npos && bkSlashPos == std::string::npos)
	{
		baseName = fileName;
	}
	else
	{	
		std::string::size_type pos;
		if (fwdSlashPos == std::string::npos)
			pos = bkSlashPos;
		else if (bkSlashPos == std::string::npos)
			pos = fwdSlashPos;
		else
			pos = fwdSlashPos > bkSlashPos ? fwdSlashPos : bkSlashPos;

		baseName = fileName.substr(pos + 1);
	}

	std::string::size_type dotPos = baseName.find_last_of('.');
	if (dotPos == std::string::npos)
		return baseName;
	return baseName.substr(0, dotPos);
}

// ----------------------------------------------------------------------

bool WeaponExporterTool::getFileFromPerforce(const std::string & filename)
{
	std::string fixedname(filename);
	fixPath(fixedname);

	char buffer[1024 * 2];
	std::string p4command = "p4 fstat " + fixedname + " 2>&1";
	FILE * p4 = popen(p4command.c_str(), "r");
	FATAL(p4 == NULL, ("Cannot access Perforce"));
	if (fgets(buffer, sizeof(buffer), p4) == NULL)
	{
		buffer[0] = '\0';
		if (feof(p4) == 0)
		{
			int error = ferror(p4);
			char * errString = strerror(error);
			FATAL(true, (errString));
		}
	}
	pclose(p4);

	if (strstr(buffer, "no such file") != NULL)
		return false;

	p4command = "p4 edit " + fixedname + REDIRECT_TO_NULL;
	system(p4command.c_str());
	return true;
}

// ----------------------------------------------------------------------

bool WeaponExporterTool::addFileToPerforce(const std::string & filename)
{
	std::string fixedname(filename);
	fixPath(fixedname);

	char buffer[1024 * 2];
	std::string p4command = "p4 fstat " + fixedname + " 2>&1";
	FILE * p4 = popen(p4command.c_str(), "r");
	FATAL(p4 == NULL, ("Cannot access Perforce"));
	if (fgets(buffer, sizeof(buffer), p4) == NULL)
	{
		buffer[0] = '\0';
		if (feof(p4) == 0)
		{
			int error = ferror(p4);
			char * errString = strerror(error);
			FATAL(true, (errString));
		}
	}
	pclose(p4);

	if (strstr(buffer, "no such file") == NULL)
		return false;

	p4command = "p4 add " + fixedname + REDIRECT_TO_NULL;
	system(p4command.c_str());
	return true;
}

// ----------------------------------------------------------------------

void WeaponExporterTool::usage(void)
{
	printf("WeaponExporterTool -i <weapon schematic datatable iff file>\n");
	printf("Optional Parameters: \n");
	printf("  -h print this stuff \n");
	printf("  -c print config file options \n");
	getchar();
}

// ----------------------------------------------------------------------

void WeaponExporterTool::printConfig()
{
	printf("[WeaponExporterTool]\n");
	printf("schematicTemplatePath = \"%s\"\n", ConfigWeaponExporterTool::getSchematicTemplatePath());
	printf("slotNameTable = \"%s\"\n", ConfigWeaponExporterTool::getSlotNameTable());
	printf("schematicNameTable = \"%s\"\n", ConfigWeaponExporterTool::getSchematicNameTable());
	printf("schematicDescriptionTable = \"%s\"\n", ConfigWeaponExporterTool::getSchematicDescriptionTable());
	getchar();
}

// ----------------------------------------------------------------------

bool WeaponExporterTool::readSchematicData(DataTable & dt, int row, SchematicData & data)
{
	data.schematicName = dt.getStringValue(DATATABLE_COLUMNS[DCI_schematicName], row);
	data.schematicDescription = dt.getStringValue(DATATABLE_COLUMNS[DCI_schematicDescription], row);
	data.craftingScript = dt.getStringValue(DATATABLE_COLUMNS[DCI_craftingScript], row);
	data.itemsPerContainer = dt.getIntValue(DATATABLE_COLUMNS[DCI_itemsPerContainer], row);
	data.craftingType = dt.getIntValue(DATATABLE_COLUMNS[DCI_craftingType], row);
	data.slots = dt.getIntValue(DATATABLE_COLUMNS[DCI_slots], row);
	data.complexity = dt.getIntValue(DATATABLE_COLUMNS[DCI_complexity], row);
	data.xp = dt.getIntValue(DATATABLE_COLUMNS[DCI_xp], row);
	data.xpType = dt.getIntValue(DATATABLE_COLUMNS[DCI_xpType], row);
	data.volume = dt.getIntValue(DATATABLE_COLUMNS[DCI_volume], row);
	data.craftedObjectTemplate = dt.getStringValue(DATATABLE_COLUMNS[DCI_craftedObjectTemplate], row);
	data.minDamageLow = dt.getIntValue(DATATABLE_COLUMNS[DCI_minDamageLow], row);
	data.minDamageHigh = dt.getIntValue(DATATABLE_COLUMNS[DCI_minDamageHigh], row);
	data.maxDamageLow = dt.getIntValue(DATATABLE_COLUMNS[DCI_maxDamageLow], row);
	data.maxDamageHigh = dt.getIntValue(DATATABLE_COLUMNS[DCI_maxDamageHigh], row);
	data.attackSpeedLow = dt.getIntValue(DATATABLE_COLUMNS[DCI_attackSpeedLow], row);
	data.attackSpeedHigh = dt.getIntValue(DATATABLE_COLUMNS[DCI_attackSpeedHigh], row);
	data.woundChanceLow = dt.getIntValue(DATATABLE_COLUMNS[DCI_woundChanceLow], row);
	data.woundChanceHigh = dt.getIntValue(DATATABLE_COLUMNS[DCI_woundChanceHigh], row);
	data.accuracyLow = dt.getIntValue(DATATABLE_COLUMNS[DCI_accuracyLow], row);
	data.accuracyHigh = dt.getIntValue(DATATABLE_COLUMNS[DCI_accuracyHigh], row);
	data.hitPointsLow = dt.getIntValue(DATATABLE_COLUMNS[DCI_hitPointsLow], row);
	data.hitPointsHigh = dt.getIntValue(DATATABLE_COLUMNS[DCI_hitPointsHigh], row);
	data.minRangeDistance = dt.getIntValue(DATATABLE_COLUMNS[DCI_minRangeDistance], row);
	data.maxRangeDistance = dt.getIntValue(DATATABLE_COLUMNS[DCI_maxRangeDistance], row);
	data.specialAttackCostLow = dt.getIntValue(DATATABLE_COLUMNS[DCI_specialAttackCostLow], row);
	data.specialAttackCostHigh = dt.getIntValue(DATATABLE_COLUMNS[DCI_specialAttackCostHigh], row);
	data.damageType = dt.getIntValue(DATATABLE_COLUMNS[DCI_damageType], row);
	data.elementalType = dt.getIntValue(DATATABLE_COLUMNS[DCI_elementalType], row);
	data.elementalValueLow = dt.getIntValue(DATATABLE_COLUMNS[DCI_elementalValueLow], row);
	data.elementalValueHigh = dt.getIntValue(DATATABLE_COLUMNS[DCI_elementalValueHigh], row);

	// verify the data
	FATAL(data.schematicName.empty(), ("row %d missing schematic name", row));
	FATAL(data.craftingScript.empty(), ("row %d missing crafting script", row));
	FATAL(data.craftedObjectTemplate.empty(), ("row %d missing crafted template", row));
	FATAL(data.slots <= 0, ("row %d missing slots", row));
	FATAL(data.complexity <= 0, ("row %d missing complexity", row));
	FATAL(data.xpType >= static_cast<int>(XP_TYPES_SIZE), ("row %d using unsupported xp type", row));
	WARNING(data.minDamageLow < 0, ("row %d has negative min damage low", row));
	WARNING(data.minDamageHigh < 0, ("row %d has negative min damage high", row));
	WARNING(data.maxDamageLow < 0, ("row %d has negative max damage low", row));
	WARNING(data.maxDamageHigh < 0, ("row %d has negative max damage high", row));
	//WARNING(data.attackSpeedLow < 0, ("row %d has negative attack speed low", row));
	//WARNING(data.attackSpeedHigh < 0, ("row %d has negative attack speed high", row));
	WARNING(data.woundChanceLow < 0, ("row %d has negative wound chance low", row));
	WARNING(data.woundChanceHigh < 0, ("row %d has negative wound chance high", row));
	WARNING(data.hitPointsLow <= 0, ("row %d has no hit points low defined", row));
	WARNING(data.hitPointsHigh <= 0, ("row %d has no points high defined", row));
	WARNING(data.minRangeDistance < 0, ("row %d has negative min range distance", row));
	WARNING(data.maxRangeDistance < 0, ("row %d has negative max range distance", row));
	WARNING(data.xp <= 0, ("row %d has no xp", row));
	WARNING(data.minDamageLow > data.minDamageHigh, ("row %d min damage low > min damage high"));
	WARNING(data.maxDamageLow > data.maxDamageHigh, ("row %d max damage low > max damage high"));
	WARNING(data.attackSpeedLow > data.attackSpeedHigh, ("row %d attack speed low > attack speed high"));
	WARNING(data.woundChanceLow > data.woundChanceHigh, ("row %d wound chance low > wound chance high"));
	WARNING(data.accuracyLow > data.accuracyHigh, ("row %d accuracy low > accuracy high"));
	WARNING(data.hitPointsLow > data.hitPointsHigh, ("row %d hit points low > hit points high"));
	WARNING(data.specialAttackCostLow > data.specialAttackCostHigh, ("row %d special attack cost low > special attack cost high"));
	WARNING(data.elementalValueLow > data.elementalValueHigh, ("row %d elemental value low > elemental value high"));
	WARNING((data.damageType != 0x00000000 && (data.damageType < 0x00000001) || (data.damageType > 0x00000002)), ("row %d damage type not kinetic or energy"));
	WARNING((data.elementalType != 0x00000000 && (data.elementalType < 0x00000020) || (data.elementalType > 0x00000100)), ("row %d elemental type not a valid elemental damage type"));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_slotType], row) >= 0, ("non-slot row %d defining a slot type", row));
	WARNING(dt.getStringValue(DATATABLE_COLUMNS[DCI_slotName], row)[0]!=0, ("non-slot row %d defining a slot name", row));
	WARNING(dt.getStringValue(DATATABLE_COLUMNS[DCI_slotIngredientName], row)[0]!=0, ("non-slot row %d defining a slot ingredient name", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_slotIngredientCount], row) > 0, ("non-slot row %d defining a slot ingredient count", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_slotAppearance], row) > 0, ("non-slot row %d defining a slot appearance name", row));

	return true;
}

// ----------------------------------------------------------------------

bool WeaponExporterTool::readSlotData(DataTable & dt, int row, SlotData & data)
{
	int rowType = dt.getIntValue(DATATABLE_COLUMNS[DCI_entryType], row);
	FATAL(rowType != ST_slot, ("Row %d should be flagged as a slot row", row));
	data.slotType = dt.getIntValue(DATATABLE_COLUMNS[DCI_slotType], row);
	data.slotOptional = dt.getIntValue(DATATABLE_COLUMNS[DCI_slotOptional], row);
	data.slotName = dt.getStringValue(DATATABLE_COLUMNS[DCI_slotName], row);
	data.slotIngredientName = dt.getStringValue(DATATABLE_COLUMNS[DCI_slotIngredientName], row);
	data.slotIngredientCount = dt.getIntValue(DATATABLE_COLUMNS[DCI_slotIngredientCount], row);
	data.slotAppearance = dt.getIntValue(DATATABLE_COLUMNS[DCI_slotAppearance], row);

	// verify the data
	FATAL(data.slotType < 0, ("slot row %d missing slot type"));
	FATAL(data.slotType >= static_cast<int>(INGREDIENT_TYPES_SIZE), ("slot row %d has unsupported slot type"));
	FATAL(data.slotIngredientCount <= 0, ("slot row %d missing slot ingredient count"));
	FATAL(data.slotName.empty(), ("slot row %d missing slot name"));
	FATAL(data.slotIngredientName.empty(), ("slot row %d missing slot ingredient name"));
	FATAL(data.slotAppearance < 0 || data.slotAppearance >= static_cast<int>(SLOT_APPEARANCES_SIZE), ("slot row %d has invalid slot appearance"));
	WARNING(dt.getStringValue(DATATABLE_COLUMNS[DCI_schematicName], row)[0]!=0, ("slot row %d defining a schematic name", row));
	WARNING(dt.getStringValue(DATATABLE_COLUMNS[DCI_schematicDescription], row)[0]!=0, ("slot row %d defining a schematic description", row));
	WARNING(dt.getStringValue(DATATABLE_COLUMNS[DCI_craftingScript], row)[0]!=0, ("slot row %d defining a crafting script", row));
	WARNING(dt.getStringValue(DATATABLE_COLUMNS[DCI_craftedObjectTemplate], row)[0]!=0, ("slot row %d defining a crafted template", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_itemsPerContainer], row) > 0, ("slot row %d defining items per container", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_craftingType], row) > 0, ("slot row %d defining crafting type", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_slots], row) > 0, ("slot row %d defining number of slots", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_complexity], row) > 0, ("slot row %d defining complexity", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_xp], row) > 0, ("slot row %d defining xp", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_xpType], row) >= 0, ("slot row %d defining xp type", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_minDamageLow], row) > 0, ("slot row %d defining min damage low", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_minDamageHigh], row) > 0, ("slot row %d defining min damage high", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_maxDamageLow], row) > 0, ("slot row %d defining max damage low", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_maxDamageHigh], row) > 0, ("slot row %d defining max damage high", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_attackSpeedLow], row) > 0, ("slot row %d defining attack speed low", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_attackSpeedHigh], row) > 0, ("slot row %d defining attack speed high", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_woundChanceLow], row) > 0, ("slot row %d defining wound chance low", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_woundChanceHigh], row) > 0, ("slot row %d defining wound chance high", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_accuracyLow], row) != 0, ("slot row %d defining accuracy low", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_accuracyHigh], row) != 0, ("slot row %d defining accuracy high", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_hitPointsLow], row) > 1, ("slot row %d defining hit points low", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_hitPointsHigh], row) > 1, ("slot row %d defining hit points high", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_minRangeDistance], row) > 0, ("slot row %d defining min range distance", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_maxRangeDistance], row) != dt.getIntDefaultForColumn(DATATABLE_COLUMNS[DCI_maxRangeDistance]), ("slot row %d defining max range distance", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_specialAttackCostLow], row) != 0, ("slot row %d defining special attack cost low", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_specialAttackCostHigh], row) != 0, ("slot row %d defining special attack cost high", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_elementalType], row) != 0, ("slot row %d defining elemental type", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_elementalValueLow], row) != 0, ("slot row %d defining elemental value low", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_elementalValueHigh], row) != 0, ("slot row %d defining elemental value high", row));

	return true;
}


// ----------------------------------------------------------------------

bool WeaponExporterTool::writeServerTemplate(FILE & fp, const SchematicData & data, const std::vector<SlotData> & slotData)
{
	fprintf(&fp, "// TEMPLATE GENERATED BY WEAPON EXPORTER, DO NOT EDIT\n");
	fprintf(&fp, "@base object/draft_schematic/weapon/base_weapon_schematic.iff\n");
	fprintf(&fp, "@class draft_schematic_object_template 7\n");
	fprintf(&fp, "\n");
	fprintf(&fp, "category = %s\n", (data.craftingType == ServerObjectTemplate::CT_weapon ? "CT_weapon" : "CT_lightsaber"));
	fprintf(&fp, "itemsPerContainer = %d\n", data.itemsPerContainer);
	fprintf(&fp, "craftedObjectTemplate = \"%s\"\n", data.craftedObjectTemplate.c_str());
	if (!writeServerTemplateSlots(fp, slotData))
		return false;
	fprintf(&fp, "skillCommands = [\"unskilled\"]\n");
	fprintf(&fp, "manufactureScripts = [\"%s\"]\n", data.craftingScript.c_str());
	fprintf(&fp, "\n");
	fprintf(&fp, "@class intangible_object_template 1\n");
	fprintf(&fp, "\n");
	fprintf(&fp, "@class object_template 8\n");
	fprintf(&fp, "\n");
	fprintf(&fp, "complexity = %d\n", data.complexity);
	fprintf(&fp, "volume = %d\n", data.volume);
	fprintf(&fp, "sharedTemplate = \"%s/%s.iff\"\n", ConfigWeaponExporterTool::getSchematicTemplatePath(), makeSharedTemplateName(data.schematicName).c_str());
	if (data.xpType >= 0)
		fprintf(&fp, "xpPoints = [[type = %s, level = 0, value = 0]]\n", XP_TYPES[data.xpType]);
	fprintf(&fp, "\n");
	return true;
}

// ----------------------------------------------------------------------

bool WeaponExporterTool::writeServerTemplateSlots(FILE & fp, const std::vector<SlotData> & slotData)
{
	if (slotData.empty())
		return true;

	fprintf(&fp, "slots = [\n");
	
	for (std::vector<SlotData>::const_iterator i = slotData.begin(); i != slotData.end(); ++i)
	{
		const SlotData & data = *i;
		fprintf(&fp, "\t[\n");
		fprintf(&fp, "\toptional=%s,\n", (data.slotOptional == 0 ? "false":"true"));
		fprintf(&fp, "\tname = \"%s\" \"%s\",\n", ConfigWeaponExporterTool::getSlotNameTable(), data.slotName.c_str());
		fprintf(&fp, "\toptions = [\n");
		fprintf(&fp, "\t\t[");
		if (data.slotType >= 0 && data.slotType < static_cast<int>(INGREDIENT_TYPES_SIZE))
			fprintf(&fp, "ingredientType=%s, ", INGREDIENT_TYPES[data.slotType]);
		else
			return false;
		fprintf(&fp, "ingredients = [[name=\"%s\" \"%s\", ", ConfigWeaponExporterTool::getSlotNameTable(), data.slotName.c_str());
		fprintf(&fp, "ingredient=\"%s\", count=%d", data.slotIngredientName.c_str(), data.slotIngredientCount);
		fprintf(&fp, "]], complexity = 0, skillCommand = \"unskilled\"]],\n");
		fprintf(&fp, "\toptionalSkillCommand = \"\",\n");
		fprintf(&fp, "\tcomplexity = 0,\n");
		fprintf(&fp, "\tappearance = \"%s\" ]", SLOT_APPEARANCES[data.slotAppearance]);
		if (i + 1 != slotData.end())
			fprintf(&fp, ",\n");
		else
			fprintf(&fp, "]\n");
	}

	return true;
}

bool WeaponExporterTool::writeSharedTemplate(FILE & fp, const SchematicData & data, const std::vector<SlotData> & slotData)
{
	fprintf(&fp, "// TEMPLATE GENERATED BY WEAPON EXPORTER, DO NOT EDIT\n");
	fprintf(&fp, "@base object/draft_schematic/base/shared_base_draft_schematic.iff\n");
	fprintf(&fp, "@class draft_schematic_object_template 2\n");
	fprintf(&fp, "\n");
	fprintf(&fp, "attributes = [\n");
	if (data.minDamageLow != 0 || data.minDamageHigh != 0)
		fprintf(&fp, "\t[name = \"crafting\" \"minDamage\", experiment = %s, value = %d..%d],\n", (data.minDamageLow == data.minDamageHigh ? NULL_STRING_ID.c_str() : EXP_DAMAGE_STRING_ID.c_str()), data.minDamageLow, data.minDamageHigh);
	if (data.maxDamageLow != 0 || data.maxDamageHigh != 0)
		fprintf(&fp, "\t[name = \"crafting\" \"maxDamage\", experiment = %s, value = %d..%d],\n", (data.maxDamageLow == data.maxDamageHigh ? NULL_STRING_ID.c_str() : EXP_DAMAGE_STRING_ID.c_str()), data.maxDamageLow, data.maxDamageHigh);
	if (data.woundChanceLow != 0 || data.woundChanceHigh != 0)
		fprintf(&fp, "\t[name = \"crafting\" \"woundChance\", experiment = %s, value = %d..%d],\n", (data.woundChanceLow == data.woundChanceHigh ? NULL_STRING_ID.c_str() : EXP_DAMAGE_STRING_ID.c_str()), data.woundChanceLow, data.woundChanceHigh);
	if (data.attackSpeedLow != 0 || data.attackSpeedHigh != 0)
		fprintf(&fp, "\t[name = \"crafting\" \"attackSpeed\", experiment = %s, value = %d..%d],\n", (data.attackSpeedLow == data.attackSpeedHigh ? NULL_STRING_ID.c_str() : EXP_SPEED_STRING_ID.c_str()), data.attackSpeedLow, data.attackSpeedHigh);
	if (data.accuracyLow != 0 || data.accuracyHigh != 0)
		fprintf(&fp, "\t[name = \"crafting\" \"accuracy\", experiment = %s, value = %d..%d],\n", (data.accuracyLow == data.accuracyHigh ? NULL_STRING_ID.c_str() : EXP_ACCURACY_STRING_ID.c_str()), data.accuracyLow, data.accuracyHigh);
	if (data.hitPointsLow != 0 || data.hitPointsHigh != 0)
		fprintf(&fp, "\t[name = \"crafting\" \"hitPoints\", experiment = \"\" \"\", value = %d..%d],\n", data.hitPointsLow, data.hitPointsHigh);
	if (data.minRangeDistance)
		fprintf(&fp, "\t[name = \"crafting\" \"minRange\", experiment = \"\" \"\", value = %d..%d],\n", data.minRangeDistance, data.minRangeDistance);
	if (data.maxRangeDistance != 0)
		fprintf(&fp, "\t[name = \"crafting\" \"maxRange\", experiment = \"\" \"\", value = %d..%d],\n", data.maxRangeDistance, data.maxRangeDistance);
	if (data.specialAttackCostLow != 0 || data.specialAttackCostHigh != 0)
		fprintf(&fp, "\t[name = \"crafting\" \"attackCost\", experiment = %s, value = %d..%d],\n", (data.specialAttackCostLow == data.specialAttackCostHigh ? NULL_STRING_ID.c_str() : EXP_EFFECIENCY_STRING_ID.c_str()), data.specialAttackCostLow, data.specialAttackCostHigh);
	if (data.damageType != 0)
		fprintf(&fp, "\t[name = \"crafting\" \"damageType\", experiment = \"\" \"\", value = %d..%d],\n", data.damageType, data.damageType);
	if (data.elementalType != 0)
		fprintf(&fp, "\t[name = \"crafting\" \"elementalType\", experiment = \"\" \"\", value = %d..%d],\n", data.elementalType, data.elementalType);
	if (data.elementalValueLow != 0 || data.elementalValueHigh != 0)
		fprintf(&fp, "\t[name = \"crafting\" \"elementalValue\", experiment = %s, value = %d..%d],\n", (data.elementalValueLow == data.elementalValueHigh ? NULL_STRING_ID.c_str() : EXP_ELEMENTAL_VALUE_STRING_ID.c_str()), data.elementalValueLow, data.elementalValueHigh);
	fprintf(&fp, "\t[name = \"crafting\" \"complexity\", experiment = \"\" \"\", value = %d..%d],\n", data.complexity, data.complexity);
	fprintf(&fp, "\t[name = \"crafting\" \"xp\", experiment = \"\" \"\", value = %d..%d]]\n", data.xp, data.xp);
	fprintf(&fp, "\n");
	
	fprintf(&fp, "slots = [\n");
	for (std::vector<SlotData>::const_iterator i = slotData.begin(); i != slotData.end(); ++i)
	{
		fprintf(&fp, "\t[name = \"%s\" \"%s\", hardpoint = \"\"]", ConfigWeaponExporterTool::getSlotNameTable(), (*i).slotName.c_str());
		if (i + 1 != slotData.end())
			fprintf(&fp, ",\n");
		else
			fprintf(&fp, "]\n");
	}
	fprintf(&fp, "\n");

	fprintf(&fp, "craftedSharedTemplate = \"%s\"\n", makeSharedTemplateName(data.craftedObjectTemplate).c_str());
	fprintf(&fp, "\n");
	fprintf(&fp, "@class intangible_object_template 0\n");
	fprintf(&fp, "\n");
	fprintf(&fp, "@class object_template 9\n");
//	fprintf(&fp, "objectName = \"%s\" \"%s\"\n", ConfigWeaponExporterTool::getSchematicNameTable(), getBaseFileName(data.schematicName).c_str());
//	if (!data.schematicDescription.empty())
//		fprintf(&fp, "detailedDescription = \"%s\" \"%s\"\n", ConfigWeaponExporterTool::getSchematicDescriptionTable(), data.schematicDescription.c_str());

	return true;
}

// ======================================================================
