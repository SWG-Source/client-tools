// ======================================================================
//
// ArmorExporterTool.cpp
//
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "FirstArmorExporterTool.h"
#include "ArmorExporterTool.h"

#include "ConfigArmorExporterTool.h"
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

// make sure the DatatableColumnsIndex below matches this list
const char * DATATABLE_COLUMNS[] = 
{
	"entry_type",
	"schematic_name",
	"schematic_description",
	"crafting_script",
	"items_per_container",
	"slots",
	"complexity",
	"xp",
	"xp_type",
	"volume",
	"crafted_object_template",
	"layer_index",
	"level",
	"category",
	"condition_multiplier",
	"no_crit_fail",
	"slot_type",
	"slot_optional",
	"slot_name",
	"slot_ingredient_name",
	"slot_ingredient_count"
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
	DCI_slots,
	DCI_complexity,
	DCI_xp,
	DCI_xpType,
	DCI_volume,
	DCI_craftedObjectTemplate,
	DCI_layerIndex,
	DCI_level,
	DCI_category,
	DCI_conditionMultiplier,
	DCI_noCritFail,
	DCI_slotType,
	DCI_slotOptional,
	DCI_slotName,
	DCI_slotIngredientName,
	DCI_slotIngredientCount
};

const char * ARMOR_CATEGORIES[] = 
{
	"AC_reconnaissance",
	"AC_battle",
	"AC_assault",
	"AC_psg"
};
#define ARMOR_CATEGORIES_SIZE (sizeof(ARMOR_CATEGORIES)/sizeof(ARMOR_CATEGORIES[0]))

const char * ARMOR_LEVELS[] = 
{
	"AL_basic",
	"AL_standard",
	"AL_advanced",

};
#define ARMOR_LEVELS_SIZE (sizeof(ARMOR_LEVELS)/sizeof(ARMOR_LEVELS[0]))

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


// ======================================================================

std::string ArmorExporterTool::m_inputFile;
std::string ArmorExporterTool::m_serverOutputFile;
std::string ArmorExporterTool::m_sharedOutputFile;

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
		std::string myExePath = ArmorExporterTool::getMyExecutablePath() + "/tools.cfg";
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
	ConfigArmorExporterTool::install();

	//-- iowin
	SetupSharedIoWin::install();

	SetupSharedFoundation::callbackWithExceptionHandling(ArmorExporterTool::run);
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	return 0;
}

// ----------------------------------------------------------------------
void ArmorExporterTool::run(void)
{
	printf ("Armor exporter tool " __DATE__ " " __TIME__ "\n");

	FATAL(ARMOR_CATEGORIES_SIZE - 1 != ServerObjectTemplate::ArmorCategory_Last, ("Error in the number of armor categories strings"));
	FATAL(ARMOR_LEVELS_SIZE - 1 != ServerObjectTemplate::ArmorLevel_Last, ("Error in the number of armor levels strings"));
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

std::string ArmorExporterTool::getMyExecutablePath()
{
#if defined(PLATFORM_WIN32)
	char buffer[MAX_PATH]; 
	HMODULE handle = GetModuleHandle("ArmorExporterTool.exe");
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

void ArmorExporterTool::fixPath(std::string & fileName)
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

void ArmorExporterTool::createOutputDirectoryForFile(const std::string & fileName)
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

std::string ArmorExporterTool::makeSharedTemplateName(const std::string & serverTemplate)
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

std::string ArmorExporterTool::makeDataFilename(const std::string & dsrcFilename)
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

void ArmorExporterTool::getDefaultOutputPath(const std::string & inputFileName, std::string & outputFileName)
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
	
	outputFileName = temp.substr(0, datatablesPos) + ConfigArmorExporterTool::getSchematicTemplatePath();
}

// ----------------------------------------------------------------------

void ArmorExporterTool::removeFileName(std::string & outputFileName)
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

std::string ArmorExporterTool::getBaseFileName(const std::string & fileName)
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

bool ArmorExporterTool::getFileFromPerforce(const std::string & filename)
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

bool ArmorExporterTool::addFileToPerforce(const std::string & filename)
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

void ArmorExporterTool::usage(void)
{
	printf("ArmorExporterTool -i <armor schematic datatable iff file>\n");
	printf("Optional Parameters: \n");
	printf("  -h print this stuff \n");
	printf("  -c print config file options \n");
	getchar();
}

// ----------------------------------------------------------------------

void ArmorExporterTool::printConfig()
{
	printf("[ArmorExporterTool]\n");
	printf("schematicTemplatePath = \"%s\"\n", ConfigArmorExporterTool::getSchematicTemplatePath());
	printf("slotNameTable = \"%s\"\n", ConfigArmorExporterTool::getSlotNameTable());
	printf("schematicNameTable = \"%s\"\n", ConfigArmorExporterTool::getSchematicNameTable());
	printf("schematicDescriptionTable = \"%s\"\n", ConfigArmorExporterTool::getSchematicDescriptionTable());
	getchar();
}

// ----------------------------------------------------------------------

bool ArmorExporterTool::readSchematicData(DataTable & dt, int row, SchematicData & data)
{
	data.schematicName = dt.getStringValue(DATATABLE_COLUMNS[DCI_schematicName], row);
	data.schematicDescription = dt.getStringValue(DATATABLE_COLUMNS[DCI_schematicDescription], row);
	data.craftingScript = dt.getStringValue(DATATABLE_COLUMNS[DCI_craftingScript], row);
	data.itemsPerContainer = dt.getIntValue(DATATABLE_COLUMNS[DCI_itemsPerContainer], row);
	data.slots = dt.getIntValue(DATATABLE_COLUMNS[DCI_slots], row);
	data.complexity = dt.getIntValue(DATATABLE_COLUMNS[DCI_complexity], row);
	data.xp = dt.getIntValue(DATATABLE_COLUMNS[DCI_xp], row);
	data.xpType = dt.getIntValue(DATATABLE_COLUMNS[DCI_xpType], row);
	data.volume = dt.getIntValue(DATATABLE_COLUMNS[DCI_volume], row);
	data.craftedObjectTemplate = dt.getStringValue(DATATABLE_COLUMNS[DCI_craftedObjectTemplate], row);
	data.layerIndex = dt.getIntValue(DATATABLE_COLUMNS[DCI_layerIndex], row);
	data.level = dt.getIntValue(DATATABLE_COLUMNS[DCI_level], row);
	data.category = dt.getIntValue(DATATABLE_COLUMNS[DCI_category], row);
	data.conditionMultiplier = dt.getFloatValue(DATATABLE_COLUMNS[DCI_conditionMultiplier], row);
	data.noCritFail = dt.getIntValue(DATATABLE_COLUMNS[DCI_noCritFail], row);

	// verify the data
	FATAL(data.schematicName.empty(), ("row %d missing schematic name", row));
	FATAL(data.craftingScript.empty(), ("row %d missing crafting script", row));
	FATAL(data.craftedObjectTemplate.empty(), ("row %d missing crafted template", row));
	FATAL(data.slots <= 0, ("row %d missing slots", row));
	FATAL(data.complexity <= 0, ("row %d missing complexity", row));
	FATAL(data.xpType != ServerObjectTemplate::XP_crafting && data.xpType != ServerObjectTemplate::XP_craftingClothingArmor, ("row %d has unsupported xp type", row));
	WARNING(data.xp <= 0, ("row %d has no xp", row));
	switch (data.schematicType)
	{
		case ST_layer:
			FATAL(data.layerIndex < 0, ("layer row %d missing layer", row));
			WARNING(data.level >= 0, ("layer row %d defining armor level", row));
			WARNING(data.category >= 0, ("layer row %d defining armor category", row));
			WARNING(data.conditionMultiplier > 1.0f, ("layer row %d defining condition multiplier", row));
			break;
		case ST_segment:
			WARNING(data.layerIndex > 0, ("segment row %d defining a layer", row));
			WARNING(data.level >= 0, ("segment row %d defining armor level", row));
			WARNING(data.category >= 0, ("segment row %d defining armor category", row));
			WARNING(data.conditionMultiplier > 1.0f, ("segment row %d defining condition multiplier", row));
			break;
		case ST_core:
			FATAL(data.level < 0, ("core row %d missing armor level", row));
			FATAL(data.category < 0, ("core row %d missing armor category", row));
			WARNING(data.layerIndex > 0, ("core row %d defining a layer", row));
			WARNING(data.conditionMultiplier > 1.0f, ("core row %d defining condition multiplier", row));
			break;
		case ST_final:
			FATAL(data.conditionMultiplier <= 0, ("final row %d condition multiplier <= 0", row));
			WARNING(data.layerIndex > 0, ("final row %d defining a layer", row));
			WARNING(data.level >= 0, ("final row %d defining armor level", row));
			break;
		case ST_clothing:
			WARNING(data.layerIndex > 0, ("clothing row %d defining a layer", row));
			WARNING(data.level >= 0, ("clothing row %d defining armor level", row));
			WARNING(data.category >= 0, ("clothing row %d defining armor category", row));
			WARNING(data.conditionMultiplier > 1.0f, ("clothing row %d defining condition multiplier", row));
			break;
		default:
			break;
	}

	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_slotType], row) >= 0, ("non-slot row %d defining a slot type", row));
	WARNING(dt.getStringValue(DATATABLE_COLUMNS[DCI_slotName], row)[0]!=0, ("non-slot row %d defining a slot name", row));
	WARNING(dt.getStringValue(DATATABLE_COLUMNS[DCI_slotIngredientName], row)[0]!=0, ("non-slot row %d defining a slot ingredient name", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_slotIngredientCount], row) > 0, ("non-slot row %d defining a slot ingredient count", row));

	return true;
}

// ----------------------------------------------------------------------

bool ArmorExporterTool::readSlotData(DataTable & dt, int row, SlotData & data)
{
	int rowType = dt.getIntValue(DATATABLE_COLUMNS[DCI_entryType], row);
	FATAL(rowType != ST_slot, ("Row %d should be flagged as a slot row", row));
	data.slotType = dt.getIntValue(DATATABLE_COLUMNS[DCI_slotType], row);
	data.slotOptional = dt.getIntValue(DATATABLE_COLUMNS[DCI_slotOptional], row);
	data.slotName = dt.getStringValue(DATATABLE_COLUMNS[DCI_slotName], row);
	data.slotIngredientName = dt.getStringValue(DATATABLE_COLUMNS[DCI_slotIngredientName], row);
	data.slotIngredientCount = dt.getIntValue(DATATABLE_COLUMNS[DCI_slotIngredientCount], row);

	// verify the data
	FATAL(data.slotType < 0, ("slot row %d missing slot type"));
	FATAL(data.slotIngredientCount <= 0, ("slot row %d missing slot ingredient count"));
	FATAL(data.slotName.empty(), ("slot row %d missing slot name"));
	FATAL(data.slotIngredientName.empty(), ("slot row %d missing slot ingredient name"));
	WARNING(dt.getStringValue(DATATABLE_COLUMNS[DCI_schematicName], row)[0]!=0, ("slot row %d defining a schematic name", row));
	WARNING(dt.getStringValue(DATATABLE_COLUMNS[DCI_schematicDescription], row)[0]!=0, ("slot row %d defining a schematic description", row));
	WARNING(dt.getStringValue(DATATABLE_COLUMNS[DCI_craftingScript], row)[0]!=0, ("slot row %d defining a crafting script", row));
	WARNING(dt.getStringValue(DATATABLE_COLUMNS[DCI_craftedObjectTemplate], row)[0]!=0, ("slot row %d defining a crafted template", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_itemsPerContainer], row) > 0, ("slot row %d defining items per container", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_slots], row) > 0, ("slot row %d defining number of slots", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_complexity], row) > 0, ("slot row %d defining complexity", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_xp], row) > 0, ("slot row %d defining xp", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_layerIndex], row) > 0, ("slot row %d defining layer index", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_level], row) > 0, ("slot row %d defining armor level", row));
	WARNING(dt.getIntValue(DATATABLE_COLUMNS[DCI_category], row) > 0, ("slot row %d defining armor category", row));

	return true;
}


// ----------------------------------------------------------------------

bool ArmorExporterTool::writeServerTemplate(FILE & fp, const SchematicData & data, const std::vector<SlotData> & slotData)
{
	fprintf(&fp, "// TEMPLATE GENERATED BY ARMOR EXPORTER, DO NOT EDIT\n");
	fprintf(&fp, "@base object/draft_schematic/base/base_draft_schematic.iff\n");
	fprintf(&fp, "@class draft_schematic_object_template 7\n");
	fprintf(&fp, "\n");
	fprintf(&fp, "category = CT_armor\n");
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
	fprintf(&fp, "objvars = +[");
	char comma = ' ';
	switch (data.schematicType)
	{
		case ST_layer:
			fprintf(&fp, "\"layer\" = %d", data.layerIndex);
			comma = ',';
			break;
		case ST_segment:
			// no special objvars at the moment
			break;
		case ST_core:
			if (data.level >= 0 && data.level < static_cast<int>(ARMOR_LEVELS_SIZE))
			{
				fprintf(&fp, "\"armorLevel\" = %s", ARMOR_LEVELS[data.level]);
				comma = ',';
			}
			else
				return false;
			if (data.category >= 0 && data.category < static_cast<int>(ARMOR_CATEGORIES_SIZE))
			{
				fprintf(&fp, "%c \"armorCategory\" = %s", comma, ARMOR_CATEGORIES[data.category]);
				comma = ',';
			}
			else
				return false;
			break;
		case ST_final:
			fprintf(&fp, "\"conditionMultiplier\" = %03.2f", data.conditionMultiplier);
			comma = ',';
			break;
		case ST_clothing:
			// fall though
		default:
			break;
	}
	if (data.noCritFail != 0)
		fprintf(&fp, "%c \"crafting.no_critical_fail\" = %d", comma, data.noCritFail);
	fprintf(&fp, "]\n");
	fprintf(&fp, "sharedTemplate = \"%s/%s.iff\"\n", ConfigArmorExporterTool::getSchematicTemplatePath(), makeSharedTemplateName(data.schematicName).c_str());
	fprintf(&fp, "xpPoints = [[type = %s, level = 0, value = 0]]\n", (data.xpType == ServerObjectTemplate::XP_crafting ? "XP_crafting" : "XP_craftingClothingArmor"));
	fprintf(&fp, "\n");
	return true;
}

// ----------------------------------------------------------------------

bool ArmorExporterTool::writeServerTemplateSlots(FILE & fp, const std::vector<SlotData> & slotData)
{
	if (slotData.empty())
		return true;

	fprintf(&fp, "slots = [\n");
	
	for (std::vector<SlotData>::const_iterator i = slotData.begin(); i != slotData.end(); ++i)
	{
		const SlotData & data = *i;
		fprintf(&fp, "\t[\n");
		fprintf(&fp, "\toptional=%s,\n", (data.slotOptional == 0 ? "false":"true"));
		fprintf(&fp, "\tname = \"%s\" \"%s\",\n", ConfigArmorExporterTool::getSlotNameTable(), data.slotName.c_str());
		fprintf(&fp, "\toptions = [\n");
		fprintf(&fp, "\t\t[");
		if (data.slotType >= 0 && data.slotType < static_cast<int>(INGREDIENT_TYPES_SIZE))
			fprintf(&fp, "ingredientType=%s, ", INGREDIENT_TYPES[data.slotType]);
		else
			return false;
		fprintf(&fp, "ingredients = [[name=\"%s\" \"%s\", ", ConfigArmorExporterTool::getSlotNameTable(), data.slotName.c_str());
		fprintf(&fp, "ingredient=\"%s\", count=%d", data.slotIngredientName.c_str(), data.slotIngredientCount);
		fprintf(&fp, "]], complexity = 0, skillCommand = \"unskilled\"]],\n");
		fprintf(&fp, "\toptionalSkillCommand = \"\",\n");
		fprintf(&fp, "\tcomplexity = 0,\n");
		fprintf(&fp, "\tappearance = \"\" ]");
		if (i + 1 != slotData.end())
			fprintf(&fp, ",\n");
		else
			fprintf(&fp, "]\n");
	}

	return true;
}

bool ArmorExporterTool::writeSharedTemplate(FILE & fp, const SchematicData & data, const std::vector<SlotData> & slotData)
{
	fprintf(&fp, "// TEMPLATE GENERATED BY ARMOR EXPORTER, DO NOT EDIT\n");
	fprintf(&fp, "@base object/draft_schematic/base/shared_base_draft_schematic.iff\n");
	fprintf(&fp, "@class draft_schematic_object_template 2\n");
	fprintf(&fp, "\n");
	fprintf(&fp, "attributes = [\n");
	fprintf(&fp, "\t[name = \"crafting\" \"complexity\", experiment = \"\" \"\", value = %d..%d],\n", data.complexity, data.complexity);
	fprintf(&fp, "\t[name = \"crafting\" \"xp\", experiment = \"\" \"\", value = %d..%d]%s\n", data.xp, data.xp, data.schematicType != ST_clothing ? "," : "]");
	switch (data.schematicType)
	{
		case ST_layer:
			fprintf(&fp, "\t[name = \"crafting\" \"special_protection\", experiment = \"crafting\" \"special_protection\", value = 0..1]]\n");
			break;
		case ST_segment:
			fprintf(&fp, "\t[name = \"crafting\" \"condition\", experiment = \"crafting\" \"condition\", value = 0..1],\n");
			fprintf(&fp, "\t[name = \"crafting\" \"general_protection\", experiment = \"crafting\" \"general_protection\", value = 0..1]]\n");
			break;
		case ST_core:
			fprintf(&fp, "\t[name = \"crafting\" \"condition\", experiment = \"crafting\" \"condition\", value = 0..1],\n");
			fprintf(&fp, "\t[name = \"crafting\" \"general_protection\", experiment = \"crafting\" \"general_protection\", value = 0..1]]\n");
			break;
		case ST_final:
			fprintf(&fp, "\t[name = \"crafting\" \"sockets\", experiment = \"\" \"\", value = 0..0],\n");
			fprintf(&fp, "\t[name = \"crafting\" \"condition\", experiment = \"crafting\" \"condition\", value = 0..1],\n");
			fprintf(&fp, "\t[name = \"crafting\" \"general_protection\", experiment = \"\" \"\", value = 0..1]]\n");
			break;
		case ST_clothing:
			// fall though
		default:
			break;
	}
	fprintf(&fp, "\n");
	
	fprintf(&fp, "slots = [\n");
	for (std::vector<SlotData>::const_iterator i = slotData.begin(); i != slotData.end(); ++i)
	{
		fprintf(&fp, "\t[name = \"%s\" \"%s\", hardpoint = \"\"]", ConfigArmorExporterTool::getSlotNameTable(), (*i).slotName.c_str());
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
	fprintf(&fp, "objectName = \"%s\" \"%s\"\n", ConfigArmorExporterTool::getSchematicNameTable(), getBaseFileName(data.schematicName).c_str());
	if (!data.schematicDescription.empty())
		fprintf(&fp, "detailedDescription = \"%s\" \"%s\"\n", ConfigArmorExporterTool::getSchematicDescriptionTable(), data.schematicDescription.c_str());

	return true;
}

// ======================================================================
