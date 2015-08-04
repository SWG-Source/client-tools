// ======================================================================
//
// Configuration.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceQuestEditor.h"
#include "Configuration.h"

#include "fileInterface/AbstractFile.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/Vector.h"
#include "sharedUtility/DataTable.h"

// ======================================================================

namespace ConfigurationNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	CString const cms_empty = "";
	CString const cms_zero = "0";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	CString ms_serverMissionDataTablePath;
	CString ms_sharedStringFilePath;
	CString ms_spaceQuestDirectory;
	CString ms_spaceMobileDataTableFileName;
	CString ms_spaceZoneDataTablePath;

	class SpaceMobile
	{
	public:

		CString m_name;
		CString m_cargoName;

		bool operator<(SpaceMobile const & rhs) const
		{
			return m_name < rhs.m_name;
		}
	};

	typedef std::vector<SpaceMobile> SpaceMobileList;
	SpaceMobileList ms_spaceMobileList;

	typedef std::map<CString, StringList> CargoMap;
	CargoMap ms_cargoMap;

	StringSet ms_spaceFactions;

	StringSet ms_questCategories;

	class StringButton
	{
	public:

		CString m_name;
		CString m_data;
	};

	typedef std::vector<StringButton> StringButtonList;
	StringButtonList ms_stringButtonList;

	class SpaceZone
	{
	public:
		typedef std::map<CString, Vector> NavPointLocationMap;

		StringList m_navPointList;
		StringList m_spawnerList;
		NavPointLocationMap m_navPointLocationMap;
	};

	typedef std::map<CString, SpaceZone> SpaceZoneMap;
	SpaceZoneMap ms_spaceZoneMap;

	typedef std::map<CString, Configuration::MissionTemplate *> MissionTemplateMap;
	MissionTemplateMap ms_missionTemplateMap;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void trimEndingSlashes(CString & path)
	{
		path.Replace('\\', '/');

		while (!path.IsEmpty())
		{
			int const index = path.GetLength() - 1;
			if (path[index] == '/')
				path = path.Left(index);
			else
				break;
		}
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void createDirectory(CString const & directoryName)
	{
		BOOL result = CreateDirectory(directoryName, 0);
		if (result)
			DEBUG_REPORT_LOG(true, ("%s created\n", directoryName));
		else
			if (GetLastError() == ERROR_ALREADY_EXISTS)
				DEBUG_REPORT_LOG(true, ("%s exists\n", directoryName));
			else
				DEBUG_REPORT_LOG(true, ("%s ERROR\n", directoryName));
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

}

using namespace ConfigurationNamespace;

// ======================================================================

Configuration::MissionTemplate::PropertyTemplate::PropertyTemplate() :
	m_required(false),
	m_columnName(),
	m_dataTableHeader(),
	m_propertyType(PT_unknown),
	m_toolTipText()
{
}

// ----------------------------------------------------------------------

Configuration::MissionTemplate::PropertyTemplate::~PropertyTemplate()
{
}

// ----------------------------------------------------------------------

void Configuration::MissionTemplate::PropertyTemplate::parseIniString(CString const & iniString)
{
	CString buffer(iniString);

	int index = buffer.Find('|');
	if (index != -1)
	{
		m_columnName = buffer.Left(index);
		buffer = buffer.Right(buffer.GetLength() - index - 1);

		if (!m_columnName.IsEmpty() && m_columnName[0] == '*')
		{
			m_required = true;
			m_columnName = m_columnName.Right(m_columnName.GetLength() - 1);
		}
	}

	index = buffer.Find('|');
	if (index != -1)
	{
		m_dataTableHeader = buffer.Left(index);
		buffer = buffer.Right(buffer.GetLength() - index - 1);
	}

	index = buffer.Find('|');
	if (index != -1)
	{
		CString const propertyType = buffer.Left(index);
		buffer = buffer.Right(buffer.GetLength() - index - 1);

		if (propertyType == "PT_notImplemented")
			m_propertyType = PT_notImplemented;

		if (propertyType == "PT_bool")
			m_propertyType = PT_bool;

		if (propertyType == "PT_string")
			m_propertyType = PT_string;

		if (propertyType == "PT_integer")
			m_propertyType = PT_integer;

		if (propertyType == "PT_spaceZone")
			m_propertyType = PT_spaceZone;

		if (propertyType.Find("PT_enumList", 0) == 0)
		{
			m_propertyType = PT_enumList;

			//-- Parse enum values
			CString propertyData = propertyType.Right(propertyType.GetLength() - strlen("PT_enumList"));
			propertyData.Remove('(');
			propertyData.Remove(')');
			Configuration::unpackString(propertyData, m_propertyDataList, ',');
		}

		if (propertyType.Find("PT_cargo", 0) == 0)
		{
			m_propertyType = PT_cargo;

			//-- Parse enum values
			m_propertyData = propertyType.Right(propertyType.GetLength() - strlen("PT_cargo"));
			m_propertyData.Remove('(');
			m_propertyData.Remove(')');
		}

		if (propertyType == "PT_spaceFaction")
			m_propertyType = PT_spaceFaction;

		if (propertyType == "PT_spaceMobile")
			m_propertyType = PT_spaceMobile;

		if (propertyType == "PT_spaceMobileList")
			m_propertyType = PT_spaceMobileList;

		if (propertyType == "PT_navPoint")
			m_propertyType = PT_navPoint;

		if (propertyType == "PT_navPointList")
			m_propertyType = PT_navPointList;

		if (propertyType == "PT_spawner")
			m_propertyType = PT_spawner;

		if (propertyType == "PT_spawnerList")
			m_propertyType = PT_spawnerList;

		if (propertyType == "PT_questName")
			m_propertyType = PT_questName;

		if (propertyType.Find("PT_objectTemplate", 0) == 0)
		{
			m_propertyType = PT_objectTemplate;

			//-- Parse enum values
			m_propertyData = propertyType.Right(propertyType.GetLength() - strlen("PT_objectTemplate"));
			m_propertyData.Remove('(');
			m_propertyData.Remove(')');
		}
	}

	if (!buffer.IsEmpty())
		m_toolTipText = buffer;
}

// ----------------------------------------------------------------------

CString const Configuration::MissionTemplate::PropertyTemplate::getPropertyTemplateFormatString() const
{
	CString result;

	result += "  property = [" + m_columnName + "]: " + getPropertyTypeString() + '\n';

	return result;
}

// ----------------------------------------------------------------------

CString const Configuration::MissionTemplate::PropertyTemplate::getPropertyData(int const index) const
{
	if (index >= 0 && index < static_cast<int>(m_propertyDataList.size()))
		return m_propertyDataList[index];

	return "ERROR(1): see asommers";
}

// ----------------------------------------------------------------------

CString const Configuration::MissionTemplate::PropertyTemplate::getPropertyTypeString() const
{
	switch (m_propertyType)
	{
	case PT_unknown:
		return "PT_unknown";

	case PT_bool:
		return "PT_bool";
	
	case PT_integer:
		return "PT_integer";
	
	case PT_cargo:
		return "PT_cargo";
	
	case PT_string:
		return "PT_string";
	
	case PT_notImplemented:
		return "PT_notImplemented";
	
	case PT_spaceZone:
		return "PT_spaceZone";

	case PT_enumList:
		return "PT_enumList";

	case PT_spaceFaction:
		return "PT_spaceFaction";

	case PT_spaceMobile:
		return "PT_spaceMobile";

	case PT_spaceMobileList:
		return "PT_spaceMobileList";

	case PT_navPoint:
		return "PT_navPoint";

	case PT_navPointList:
		return "PT_navPointList";

	case PT_questName:
		return "PT_questName";

	case PT_objectTemplate:
		return "PT_objectTemplate";

	case PT_spawner:
		return "PT_spawner";

	case PT_spawnerList:
		return "PT_spawnerList";
	}

	return cms_empty;
}

// ======================================================================

Configuration::MissionTemplate::StringTemplate::StringTemplate() :
	m_required(false),
	m_stringId(),
	m_toolTipText()
{
}

// ----------------------------------------------------------------------

Configuration::MissionTemplate::StringTemplate::~StringTemplate()
{
}

// ----------------------------------------------------------------------

void Configuration::MissionTemplate::StringTemplate::parseIniString(CString const & iniString)
{
	CString buffer(iniString);

	int index = buffer.Find('|');
	if (index != -1)
	{
		m_stringId = buffer.Left(index);
		buffer = buffer.Right(buffer.GetLength() - index - 1);
	}
	else
	{
		m_stringId = buffer;
		buffer.Empty();
	}

	if (!m_stringId.IsEmpty() && m_stringId[0] == '*')
	{
		m_required = true;
		m_stringId = m_stringId.Right(m_stringId.GetLength() - 1);
	}

	if (!buffer.IsEmpty())
		m_toolTipText = buffer;
}

// ----------------------------------------------------------------------

CString const Configuration::MissionTemplate::StringTemplate::getStringTemplateFormatString() const
{
	CString result;

	result += "  string = [" + m_stringId + "]\n";

	return result;
}

// ======================================================================

Configuration::MissionTemplate::QuestStringSuffixTemplate::QuestStringSuffixTemplate() :
	m_suffix(),
	m_toolTipText()
{
}

// ----------------------------------------------------------------------

Configuration::MissionTemplate::QuestStringSuffixTemplate::~QuestStringSuffixTemplate()
{
}

// ----------------------------------------------------------------------

void Configuration::MissionTemplate::QuestStringSuffixTemplate::parseIniString(CString const & iniString)
{
	CString buffer(iniString);

	int index = buffer.Find('|');
	if (index != -1)
	{
		m_suffix = buffer.Left(index);
		buffer = buffer.Right(buffer.GetLength() - index - 1);
	}
	else
	{
		m_suffix = buffer;
		buffer.Empty();
	}

	if (!buffer.IsEmpty())
		m_toolTipText = buffer;
}

// ----------------------------------------------------------------------

CString const Configuration::MissionTemplate::QuestStringSuffixTemplate::getQuestStringSuffixTemplateFormatString() const
{
	CString result;

	result += "  questStringSuffix = [" + m_suffix + "]\n";

	return result;
}

// ======================================================================

Configuration::MissionTemplate::MissionTemplate(char const * const name) :
	m_name(name),
	m_allowNavPointsInMultipleZones(false),
	m_information(),
	m_propertyTemplateList(),
	m_stringTemplateList()
{
}

// ----------------------------------------------------------------------

Configuration::MissionTemplate::~MissionTemplate()
{
}

// ----------------------------------------------------------------------

CString const & Configuration::MissionTemplate::getName() const
{
	return m_name;
}

// ----------------------------------------------------------------------

bool Configuration::MissionTemplate::getAllowNavPointsInMultipleZones() const
{
	return m_allowNavPointsInMultipleZones;
}

// ----------------------------------------------------------------------

CString const & Configuration::MissionTemplate::getInformation() const
{
	return m_information;
}

// ----------------------------------------------------------------------

int Configuration::MissionTemplate::getNumberOfStringTemplates() const
{
	return static_cast<int>(m_stringTemplateList.size());
}

// ----------------------------------------------------------------------

Configuration::MissionTemplate::StringTemplate const & Configuration::MissionTemplate::getStringTemplate(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfStringTemplates());
	return m_stringTemplateList[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

Configuration::MissionTemplate::StringTemplate const * Configuration::MissionTemplate::getStringTemplate(CString const & key) const
{
	for (StringTemplateList::const_iterator iter = m_stringTemplateList.begin(); iter != m_stringTemplateList.end(); ++iter)
		if (iter->m_stringId == key)
			return &*iter;

	return 0;
}

// ----------------------------------------------------------------------

int Configuration::MissionTemplate::getNumberOfPropertyTemplates() const
{
	return static_cast<int>(m_propertyTemplateList.size());
}

// ----------------------------------------------------------------------

Configuration::MissionTemplate::PropertyTemplate const & Configuration::MissionTemplate::getPropertyTemplate(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfPropertyTemplates());
	return m_propertyTemplateList[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

Configuration::MissionTemplate::PropertyTemplate const * Configuration::MissionTemplate::getPropertyTemplate(CString const & key) const
{
	for (PropertyTemplateList::const_iterator iter = m_propertyTemplateList.begin(); iter != m_propertyTemplateList.end(); ++iter)
		if (iter->m_columnName == key)
			return &*iter;

	return 0;
}

// ----------------------------------------------------------------------

int Configuration::MissionTemplate::getNumberOfQuestStringSuffixTemplates() const
{
	return static_cast<int>(m_questStringSuffixTemplateList.size());
}

// ----------------------------------------------------------------------

Configuration::MissionTemplate::QuestStringSuffixTemplate const & Configuration::MissionTemplate::getQuestStringSuffixTemplate(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfQuestStringSuffixTemplates());
	return m_questStringSuffixTemplateList[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

bool Configuration::MissionTemplate::isQuestStringSuffixTemplate(CString const & key) const
{
	if (key == "title_d")
		return true;

	if (key == "Quest Log Data")
		return true;

	CString const shortKey = key.Left(key.GetLength() - 2);
	for (QuestStringSuffixTemplateList::const_iterator iter = m_questStringSuffixTemplateList.begin(); iter != m_questStringSuffixTemplateList.end(); ++iter)
		if (iter->m_suffix == key || iter->m_suffix == shortKey)
			return true;

	return false;
}

// ----------------------------------------------------------------------

CString const Configuration::MissionTemplate::getMissionTemplateFormatString() const
{
	CString result;

	result += "- " + getName() + "\n";
	result += CString("  allowNavPointsInMultipleZones=") + (m_allowNavPointsInMultipleZones ? "1" : "0") + "\r\n";
	
	{
		for (int i = 0; i < getNumberOfPropertyTemplates(); ++i)
			result += getPropertyTemplate(i).getPropertyTemplateFormatString();
	}

	{
		for (int i = 0; i < getNumberOfStringTemplates(); ++i)
			result += getStringTemplate(i).getStringTemplateFormatString();
	}

	{
		for (int i = 0; i < getNumberOfQuestStringSuffixTemplates(); ++i)
			result += getQuestStringSuffixTemplate(i).getQuestStringSuffixTemplateFormatString();
	}

	return result;
}

// ----------------------------------------------------------------------

void Configuration::MissionTemplate::setJournalType(CString const & journalType)
{
	if (journalType == "JT_default")
		m_journalType = JT_default;
	else if (journalType == "JT_generic")
		m_journalType = JT_generic;
	else if (journalType == "JT_genericLocationTitle")
		m_journalType = JT_genericLocationTitle;
	else if (journalType == "JT_navigationPoints")
		m_journalType = JT_navigationPoints;
	else if (journalType == "JT_spawners")
		m_journalType = JT_spawners;
	else
		m_journalType = JT_invalid;
}

// ----------------------------------------------------------------------

Configuration::MissionTemplate::JournalType Configuration::MissionTemplate::getJournalType() const
{
	return m_journalType;
}

// ======================================================================

bool Configuration::install()
{
	ExitChain::add(remove, "Configuration::remove");

	if (!loadCfg())
		return false;

	if (!loadIni())
		return false;

	//-- Create all directories
	CString serverMissionDataTableDirectory(getServerMissionDataTablePath());
	createDirectory(serverMissionDataTableDirectory);

	{
		CString iffDirectory(serverMissionDataTableDirectory);
		iffDirectory.Replace("/dsrc/", "/data/");
		createDirectory(iffDirectory);
	}
	
	serverMissionDataTableDirectory += '/';
	serverMissionDataTableDirectory += getSpaceQuestDirectory();
	createDirectory(serverMissionDataTableDirectory);

	{
		CString iffDirectory(serverMissionDataTableDirectory);
		iffDirectory.Replace("/dsrc/", "/data/");
		createDirectory(iffDirectory);
	}
	
	CString sharedStringTableDirectory(getSharedStringFilePath());
	createDirectory(sharedStringTableDirectory);
	
	sharedStringTableDirectory += '/';
	sharedStringTableDirectory += getSpaceQuestDirectory();
	createDirectory(serverMissionDataTableDirectory);

	CString sharedQuestListDataTableDirectory(getSharedQuestListDataTablePath());
	createDirectory(sharedQuestListDataTableDirectory);

	{
		CString iffDirectory(sharedQuestListDataTableDirectory);
		iffDirectory.Replace("/dsrc/", "/data/");
		createDirectory(iffDirectory);
	}
	
	sharedQuestListDataTableDirectory += '/';
	sharedQuestListDataTableDirectory += getSpaceQuestDirectory();
	createDirectory(sharedQuestListDataTableDirectory);

	{
		CString iffDirectory(sharedQuestListDataTableDirectory);
		iffDirectory.Replace("/dsrc/", "/data/");
		createDirectory(iffDirectory);
	}
	
	CString sharedQuestTaskDataTableDirectory(getSharedQuestTaskDataTablePath());
	createDirectory(sharedQuestTaskDataTableDirectory);

	{
		CString iffDirectory(sharedQuestTaskDataTableDirectory);
		iffDirectory.Replace("/dsrc/", "/data/");
		createDirectory(iffDirectory);
	}
	
	sharedQuestTaskDataTableDirectory += '/';
	sharedQuestTaskDataTableDirectory += getSpaceQuestDirectory();
	createDirectory(sharedQuestTaskDataTableDirectory);

	{
		CString iffDirectory(sharedQuestTaskDataTableDirectory);
		iffDirectory.Replace("/dsrc/", "/data/");
		createDirectory(iffDirectory);
	}
	
	for (int i = 0; i < getNumberOfMissionTemplates(); ++i)
	{
		MissionTemplate const * const missionTemplate = getMissionTemplate(i);

		{
			CString directoryName = serverMissionDataTableDirectory;

			directoryName += '/';
			directoryName += missionTemplate->getName();
			createDirectory(directoryName);

			directoryName.Replace("/dsrc/", "/data/");
			createDirectory(directoryName);
		}

		{
			CString directoryName = sharedQuestListDataTableDirectory;

			directoryName += '/';
			directoryName += missionTemplate->getName();
			createDirectory(directoryName);

			directoryName.Replace("/dsrc/", "/data/");
			createDirectory(directoryName);
		}

		{
			CString directoryName = sharedQuestTaskDataTableDirectory;

			directoryName += '/';
			directoryName += missionTemplate->getName();
			createDirectory(directoryName);

			directoryName.Replace("/dsrc/", "/data/");
			createDirectory(directoryName);
		}
	}

	return true;
}

// ----------------------------------------------------------------------

void Configuration::remove()
{
	std::for_each(ms_missionTemplateMap.begin(), ms_missionTemplateMap.end(), PointerDeleterPairSecond());
	ms_missionTemplateMap.clear();
}

// ----------------------------------------------------------------------

CString const Configuration::getConfiguration()
{
	CString buffer;

	CString result;
	result += "serverMissionDataTablePath: " + getServerMissionDataTablePath() + "\n";
	result += "sharedStringFilePath: " + getSharedStringFilePath() + "\n";
	result += "spaceQuestDirectory: " + getSpaceQuestDirectory() + "\n";

	buffer.Format("%i spaceZones\n", ms_spaceZoneMap.size());
	result += buffer;

	{
		SpaceZoneMap::iterator end = ms_spaceZoneMap.end();
		for (SpaceZoneMap::iterator iter = ms_spaceZoneMap.begin(); iter != end; ++iter)
		{
			result += "  " + iter->first + "\n";

			CString navPointList;
			Configuration::packString(iter->second.m_navPointList, navPointList, '|');
			result += "    navPointList (" + (!navPointList.IsEmpty() ? navPointList : "<None>") + ")\n";

			CString spawnerList;
			Configuration::packString(iter->second.m_spawnerList, spawnerList, '|');
			result += "    spawnerList (" + (!spawnerList.IsEmpty() ? spawnerList : "<None>") + ")\n";
		}
	}

	{
		buffer.Format("%i questCategories\n", ms_questCategories.size());
		result += buffer;

		StringSet::iterator end = ms_questCategories.end();
		for (StringSet::iterator iter = ms_questCategories.begin(); iter != end; ++iter)
			result += "  category = " + *iter + "\n";
	}

	buffer.Format("%i spaceMobiles and %i factions from %s\n", ms_spaceMobileList.size(), ms_spaceFactions.size(), ms_spaceMobileDataTableFileName);
	result += buffer;

	{
		SpaceMobileList::iterator end = ms_spaceMobileList.end();
		for (SpaceMobileList::iterator iter = ms_spaceMobileList.begin(); iter != end; ++iter)
			result += "  mobile = " + iter->m_name + ", cargo = " + iter->m_cargoName + "\n";
	}

	{
		StringSet::iterator end = ms_spaceFactions.end();
		for (StringSet::iterator iter = ms_spaceFactions.begin(); iter != end; ++iter)
			result += "  faction = " + *iter + "\n";
	}

	buffer.Format("%i cargo files\n", ms_cargoMap.size());
	result += buffer;

	{
		CargoMap::iterator end = ms_cargoMap.end();
		for (CargoMap::iterator iter = ms_cargoMap.begin(); iter != end; ++iter)
		{
			CString cargoList;
			Configuration::packString(iter->second, cargoList, '|');
			result += "  " + iter->first + " = (" + (!cargoList.IsEmpty() ? cargoList : "<None>") + ")\n";
		}
	}

	buffer.Format("%i missionTemplateTypes\n", ms_missionTemplateMap.size());
	result += buffer;

	{
		MissionTemplateMap::iterator end = ms_missionTemplateMap.end();
		for (MissionTemplateMap::iterator iter = ms_missionTemplateMap.begin(); iter != end; ++iter)
		{
			MissionTemplate const * const missionTemplate = iter->second;
			result += missionTemplate->getMissionTemplateFormatString();
		}
	}

	return result;
}

// ----------------------------------------------------------------------

CString const Configuration::extractMissionTemplateType(CString pathName)
{
	pathName.Replace('\\', '/');

	int begin = pathName.ReverseFind('/');
	pathName = pathName.Left(begin);
	begin = pathName.ReverseFind('/');

	return pathName.Right(pathName.GetLength() - begin - 1);
}

// ----------------------------------------------------------------------

CString const Configuration::extractRootName(CString pathName)
{
	pathName.Replace('\\', '/');

	int const begin = pathName.ReverseFind('/');
	if (begin != -1)
		pathName = pathName.Right(pathName.GetLength() - begin - 1);

	int const end = pathName.ReverseFind('.');
	if (end != -1)
		pathName = pathName.Left(end);

	return pathName;
}

// ----------------------------------------------------------------------

CString const Configuration::createServerMissionDataTableName(CString const & rootName, CString const & missionTemplateType)
{
	CString result(getServerMissionDataTablePath());
	result += '/';
	result += getSpaceQuestDirectory();
	result += '/';
	result += missionTemplateType;
	result += '/';
	result += rootName;
	result += ".tab";

	return result;
}

// ----------------------------------------------------------------------

CString const Configuration::createSharedStringTableName(CString const & rootName, CString const & missionTemplateType)
{
	CString result(getSharedStringFilePath());
	result += '/';
	result += getSpaceQuestDirectory();
	result += '/';
	result += missionTemplateType;
	result += '/';
	result += rootName;
	result += ".stf";

	return result;
}

// ----------------------------------------------------------------------

CString const Configuration::createSharedQuestListDataTableName(CString const & rootName, CString const & missionTemplateType)
{
	CString result(getServerMissionDataTablePath());
	result.Replace("/sys.server/", "/sys.shared/");
	result += "/questlist/";
	result += getSpaceQuestDirectory();
	result += '/';
	result += missionTemplateType;
	result += '/';
	result += rootName;
	result += ".tab";

	return result;
}

// ----------------------------------------------------------------------

CString const Configuration::createSharedQuestTaskDataTableName(CString const & rootName, CString const & missionTemplateType)
{
	CString result(getServerMissionDataTablePath());
	result.Replace("/sys.server/", "/sys.shared/");
	result += "/questtask/";
	result += getSpaceQuestDirectory();
	result += '/';
	result += missionTemplateType;
	result += '/';
	result += rootName;
	result += ".tab";

	return result;
}

// ----------------------------------------------------------------------

CString const & Configuration::getSpaceQuestDirectory()
{
	return ms_spaceQuestDirectory;
}

// ----------------------------------------------------------------------

CString const & Configuration::getServerMissionDataTablePath()
{
	return ms_serverMissionDataTablePath;
}

// ----------------------------------------------------------------------

CString const & Configuration::getSharedStringFilePath()
{
	return ms_sharedStringFilePath;
}

// ----------------------------------------------------------------------

CString const Configuration::getSharedQuestListDataTablePath()
{
	CString result(getServerMissionDataTablePath());
	result.Replace("sys.server", "sys.shared");
	result += "/questlist";
	return result;
}

// ----------------------------------------------------------------------

CString const Configuration::getSharedQuestTaskDataTablePath()
{
	CString result(getServerMissionDataTablePath());
	result.Replace("sys.server", "sys.shared");
	result += "/questtask";
	return result;
}

// ----------------------------------------------------------------------

int Configuration::getNumberOfStringButtons()
{
	return static_cast<int>(ms_stringButtonList.size());
}

// ----------------------------------------------------------------------

CString const & Configuration::getStringButtonName(int const index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfStringButtons());
	return ms_stringButtonList[index].m_name;
}

// ----------------------------------------------------------------------

CString const & Configuration::getStringButtonData(int const index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfStringButtons());
	return ms_stringButtonList[index].m_data;
}

// ----------------------------------------------------------------------

int Configuration::getNumberOfSpaceZones()
{
	return static_cast<int>(ms_spaceZoneMap.size());
}

// ----------------------------------------------------------------------

CString const & Configuration::getSpaceZone(int const index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfSpaceZones());

	SpaceZoneMap::iterator iter = ms_spaceZoneMap.begin();
	for (int i = 0; i < index; ++i, ++iter)
		;

	return iter->first;
}

// ----------------------------------------------------------------------

CString const Configuration::getSpaceZoneList()
{
	CString result;

	for (int i = 0; i < getNumberOfSpaceZones(); ++i)
	{
		if (i != 0)
			result += '!';

		result += getSpaceZone(i);
	}

	return result;
}

// ----------------------------------------------------------------------

int Configuration::getSpaceZoneIndex(CString const & key)
{
	int i = 0;
	for (i = 0; i < getNumberOfSpaceZones(); ++i)
		if (getSpaceZone(i) == key)
			return i;

	return 0;
}

// ----------------------------------------------------------------------

bool Configuration::isValidSpaceZone(CString const & key)
{
	return ms_spaceZoneMap.find(key) != ms_spaceZoneMap.end();
}

// ----------------------------------------------------------------------

bool Configuration::isValidNavPoint(CString const & spaceZone, CString const & navPoint, bool const allowNavPointsInMultipleZones)
{
	SpaceZoneMap::iterator iter = ms_spaceZoneMap.find(spaceZone);
	if (iter != ms_spaceZoneMap.end())
	{
		StringList stringList;
		unpackString(navPoint, stringList, ':');

		if (stringList.size() != 2)
			return false;

		if (allowNavPointsInMultipleZones)
			return isValidNavPoint(stringList[0], stringList[0] + ':' + stringList[1], false);
		else
		{
			if (spaceZone != stringList[0])
				return false;

			SpaceZone const & spaceZone = iter->second;
			return std::find(spaceZone.m_navPointList.begin(), spaceZone.m_navPointList.end(), stringList[1]) != spaceZone.m_navPointList.end();
		}
	}

	return false;
}

// ----------------------------------------------------------------------

bool Configuration::isValidSpawner(CString const & spaceZone, CString const & spawner)
{
	SpaceZoneMap::iterator iter = ms_spaceZoneMap.find(spaceZone);
	if (iter != ms_spaceZoneMap.end())
	{
		StringList stringList;
		unpackString(spawner, stringList, ':');

		if (stringList.size() != 2)
			return false;

		if (spaceZone != stringList[0])
			return false;

		SpaceZone const & spaceZone = iter->second;
		return std::find(spaceZone.m_spawnerList.begin(), spaceZone.m_spawnerList.end(), stringList[1]) != spaceZone.m_spawnerList.end();
	}

	return false;
}

// ----------------------------------------------------------------------

int Configuration::getNumberOfSpaceMobiles()
{
	return static_cast<int>(ms_spaceMobileList.size());
}

// ----------------------------------------------------------------------

CString const & Configuration::getSpaceMobile(int const index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfSpaceMobiles());
	return ms_spaceMobileList[index].m_name;
}

// ----------------------------------------------------------------------

bool Configuration::isValidSpaceMobile(CString const & spaceMobile)
{
	for (SpaceMobileList::iterator iter = ms_spaceMobileList.begin(); iter != ms_spaceMobileList.end(); ++iter)
		if (iter->m_name == spaceMobile)
			return true;

	return false;
}

// ----------------------------------------------------------------------

CString const Configuration::getSpaceMobileCargoName(CString const & spaceMobile)
{
	for (SpaceMobileList::iterator iter = ms_spaceMobileList.begin(); iter != ms_spaceMobileList.end(); ++iter)
		if (iter->m_name == spaceMobile)
			return iter->m_cargoName;

	return "";
}

// ----------------------------------------------------------------------

int Configuration::getNumberOfSpaceFactions()
{
	return static_cast<int>(ms_spaceFactions.size());
}

// ----------------------------------------------------------------------

CString const & Configuration::getSpaceFaction(int const index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfSpaceFactions());

	StringSet::iterator iter = ms_spaceFactions.begin();
	for (int i = 0; i < index; ++i, ++iter)
		;

	return *iter;
}

// ----------------------------------------------------------------------

CString const Configuration::getSpaceFactionList()
{
	CString result;

	for (int i = 0; i < getNumberOfSpaceFactions(); ++i)
	{
		if (i != 0)
			result += '!';

		result += getSpaceFaction(i);
	}

	return result;
}

// ----------------------------------------------------------------------

int Configuration::getSpaceFactionIndex(CString const & key)
{
	int i = 0;
	for (i = 0; i < getNumberOfSpaceFactions(); ++i)
		if (getSpaceFaction(i) == key)
			return i;

	return 0;
}

// ----------------------------------------------------------------------

bool Configuration::isValidSpaceFaction(CString const & key)
{
	return ms_spaceFactions.find(key) != ms_spaceFactions.end();
}

// ----------------------------------------------------------------------

int Configuration::getNumberOfQuestCategories()
{
	return static_cast<int>(ms_questCategories.size());
}

// ----------------------------------------------------------------------

CString const & Configuration::getQuestCategory(int const index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfQuestCategories());

	StringSet::iterator iter = ms_questCategories.begin();
	for (int i = 0; i < index; ++i, ++iter)
		;

	return *iter;
}

// ----------------------------------------------------------------------

CString const Configuration::getQuestCategoryList()
{
	CString result;

	for (int i = 0; i < getNumberOfQuestCategories(); ++i)
	{
		if (i != 0)
			result += '!';

		result += getQuestCategory(i);
	}

	return result;
}

// ----------------------------------------------------------------------

int Configuration::getQuestCategoryIndex(CString const & key)
{
	int i = 0;
	for (i = 0; i < getNumberOfQuestCategories(); ++i)
		if (getQuestCategory(i) == key)
			return i;

	return 0;
}

// ----------------------------------------------------------------------

bool Configuration::isValidQuestCategory(CString const & key)
{
	return ms_questCategories.find(key) != ms_questCategories.end();
}

// ----------------------------------------------------------------------

StringList const * Configuration::getCargo(CString const & cargoName)
{
	CargoMap::iterator iter = ms_cargoMap.find(cargoName);
	if (iter != ms_cargoMap.end())
		return &iter->second;

	return 0;
}

// ----------------------------------------------------------------------

int Configuration::getNumberOfMissionTemplates()
{
	return static_cast<int>(ms_missionTemplateMap.size());
}

// ----------------------------------------------------------------------

Configuration::MissionTemplate const * Configuration::getMissionTemplate(int const index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfMissionTemplates());

	MissionTemplateMap::iterator iter = ms_missionTemplateMap.begin();
	for (int i = 0; i < index; ++i, ++iter)
		;

	return iter->second;
}

// ----------------------------------------------------------------------

Configuration::MissionTemplate const * Configuration::getMissionTemplate(char const * const missionTemplateType)
{
	if (isValidMissionTemplateType(missionTemplateType))
		return ms_missionTemplateMap.find(missionTemplateType)->second;

	return 0;
}

// ----------------------------------------------------------------------

bool Configuration::isValidMissionTemplateType(char const * const missionTemplateType)
{
	return ms_missionTemplateMap.find(missionTemplateType) != ms_missionTemplateMap.end();
}

// ----------------------------------------------------------------------

void Configuration::populateNavPoints(CTreeCtrl & treeCtrl)
{
	treeCtrl.DeleteAllItems ();

	SpaceZoneMap::iterator end = ms_spaceZoneMap.end();
	for (SpaceZoneMap::iterator iter = ms_spaceZoneMap.begin(); iter != end; ++iter)
	{
		SpaceZone const & spaceZone = iter->second;

		if (spaceZone.m_navPointList.empty())
			continue;

		HTREEITEM treeItem = treeCtrl.InsertItem(iter->first, TVI_ROOT, TVI_SORT);
		for (StringList::const_iterator iter = spaceZone.m_navPointList.begin(); iter != spaceZone.m_navPointList.end(); ++iter)
			treeCtrl.InsertItem(*iter, treeItem, TVI_SORT);
	}
}

// ----------------------------------------------------------------------

void Configuration::populateSpawners(CTreeCtrl & treeCtrl)
{
	treeCtrl.DeleteAllItems ();

	SpaceZoneMap::iterator end = ms_spaceZoneMap.end();
	for (SpaceZoneMap::iterator iter = ms_spaceZoneMap.begin(); iter != end; ++iter)
	{
		SpaceZone const & spaceZone = iter->second;

		if (spaceZone.m_spawnerList.empty())
			continue;

		HTREEITEM treeItem = treeCtrl.InsertItem(iter->first, TVI_ROOT, TVI_SORT);
		for (StringList::const_iterator iter = spaceZone.m_spawnerList.begin(); iter != spaceZone.m_spawnerList.end(); ++iter)
			treeCtrl.InsertItem(*iter, treeItem, TVI_SORT);
	}
}

// ----------------------------------------------------------------------

void Configuration::packString(StringList const & stringList, CString & string, char const separator)
{
	for (StringList::const_iterator iter = stringList.begin(); iter != stringList.end(); ++iter)
	{
		if (iter != stringList.begin())
			string += separator;

		string += *iter;
	}
}

// ----------------------------------------------------------------------

void Configuration::unpackString(CString const & string, StringList & stringList, char const separator)
{
	CString buffer(string);
	while (!buffer.IsEmpty())
	{
		int const index = buffer.Find(separator);
		if (index == -1)
		{
			stringList.push_back(buffer);
			buffer.Empty();
		}
		else
		{
			stringList.push_back(buffer.Left(index));
			buffer = buffer.Right(buffer.GetLength() - index - 1);
		}
	}
}

// ======================================================================

bool Configuration::loadCfg()
{
	ms_serverMissionDataTablePath = ConfigFile::getKeyString("SwgSpaceQuestEditor", "serverMissionDataTablePath", ms_serverMissionDataTablePath);
	trimEndingSlashes(ms_serverMissionDataTablePath);

	ms_sharedStringFilePath = ConfigFile::getKeyString("SwgSpaceQuestEditor", "sharedStringFilePath", ms_sharedStringFilePath);
	trimEndingSlashes(ms_sharedStringFilePath);

	ms_spaceQuestDirectory = ConfigFile::getKeyString("SwgSpaceQuestEditor", "spaceQuestDirectory", ms_spaceQuestDirectory);
	trimEndingSlashes(ms_spaceQuestDirectory);

	ms_spaceMobileDataTableFileName = ConfigFile::getKeyString("SwgSpaceQuestEditor", "spaceMobileDataTable", ms_spaceMobileDataTableFileName);
	trimEndingSlashes(ms_spaceMobileDataTableFileName);

	//-- Load space_mobile.iff
	{
		Iff iff;
		if (!iff.open(ms_spaceMobileDataTableFileName, true))
		{
			DEBUG_WARNING(true, ("Configuration::loadCfg: could not open file %s\n", ms_spaceMobileDataTableFileName));
			return false;
		}
		
		DataTable dataTable;
		dataTable.load(iff);

		{
			std::string const columnName("strIndex");
			std::string const columnCargoName("cargo");
			for (int i = 0; i < dataTable.getNumRows(); ++i)
			{
				const char *name = dataTable.getStringValue(columnName, i);
				const char *cargoName = dataTable.getStringValue(columnCargoName, i);

				SpaceMobile spaceMobile;
				spaceMobile.m_name = name;
				spaceMobile.m_cargoName = cargoName;
				ms_spaceMobileList.push_back(spaceMobile);

				loadCargo(cargoName);
			}

			std::sort(ms_spaceMobileList.begin(), ms_spaceMobileList.end());
		}

		{
			std::string const columnName("space_faction");
			for (int i = 0; i < dataTable.getNumRows(); ++i)
			{
				std::string const & name = dataTable.getStringValue(columnName, i);

				if (*name.c_str())
					ms_spaceFactions.insert(name.c_str());
			}
		}
	}

	ms_spaceZoneDataTablePath = ConfigFile::getKeyString("SwgSpaceQuestEditor", "spaceZoneDataTablePath", ms_spaceZoneDataTablePath);
	trimEndingSlashes(ms_spaceZoneDataTablePath);

	return 
		!ms_serverMissionDataTablePath.IsEmpty() &&
		!ms_sharedStringFilePath.IsEmpty() && 
		!ms_spaceQuestDirectory.IsEmpty() &&
		!ms_spaceMobileList.empty() &&
		!ms_spaceFactions.empty() &&
		!ms_spaceZoneDataTablePath.IsEmpty();
}

// ----------------------------------------------------------------------

bool Configuration::loadIni()
{
	//-- Load SwgSpaceQuestEditor.ini
	{
		AbstractFile * const abstractFile = TreeFile::open("SwgSpaceQuestEditor.ini", AbstractFile::PriorityData, true);
		if (abstractFile)
		{
			int const length = abstractFile->length();
			byte * const data = abstractFile->readEntireFileAndClose();
			IGNORE_RETURN(ConfigFile::loadFromBuffer(reinterpret_cast<char const *>(data), length));
			delete []data;
			delete abstractFile;
		}
	}

	//-- Process button data
	{
		char const * stringButton = 0;
		int i = 0;
		do
		{
			stringButton = ConfigFile::getKeyString ("SwgSpaceQuestEditor", "stringButton", i, 0);
			if (stringButton && *stringButton)
			{
				StringList stringList;
				unpackString(stringButton, stringList, '|');
				if (stringList.size() == 2)
				{
					StringButton stringButton;
					stringButton.m_name = stringList[0];
					stringButton.m_data = stringList[1];
					ms_stringButtonList.push_back(stringButton);
				}

				++i;
			}
		} 
		while (stringButton);
	}

	//-- Process space zones
	{
		char const * zone = 0;
		int i = 0;
		do
		{
			zone = ConfigFile::getKeyString("SpaceZones", "zone", i, 0);
			if (zone && *zone)
			{
				SpaceZone spaceZone;

				//-- Load space zone data table
				{
					CString const spaceZoneDataTableName(ms_spaceZoneDataTablePath + '/' + zone + ".iff");

					Iff iff;
					if (iff.open(spaceZoneDataTableName, true))
					{
						DataTable dataTable;
						dataTable.load(iff);

						CString const navPointIdentifier("nav_name|4|");
						CString const spawnerIdentifier("strSpawnerName|4|");

						std::string const columnName("strObjVars");
						for (int i = 0; i < dataTable.getNumRows(); ++i)
						{
							CString objVar = dataTable.getStringValue(columnName, i);

							int index = objVar.Find(navPointIdentifier, 0);
							if (index != -1)
							{
								objVar = objVar.Right(objVar.GetLength() - index - navPointIdentifier.GetLength());

								int const index = objVar.Find('|');
								objVar = objVar.Left(index);

								if (!objVar.IsEmpty())
								{
									spaceZone.m_navPointList.push_back(objVar);

									//-- grab x, y, z here
									spaceZone.m_navPointLocationMap[objVar].x = dataTable.getFloatValue("fltPX", i);
									spaceZone.m_navPointLocationMap[objVar].y = dataTable.getFloatValue("fltPY", i);
									spaceZone.m_navPointLocationMap[objVar].z = dataTable.getFloatValue("fltPZ", i);
								}
							}

							index = objVar.Find(spawnerIdentifier, 0);
							if (index != -1)
							{
								objVar = objVar.Right(objVar.GetLength() - index - spawnerIdentifier.GetLength());

								int const index = objVar.Find('|');
								objVar = objVar.Left(index);

								if (!objVar.IsEmpty())
									spaceZone.m_spawnerList.push_back(objVar);
							}
						}
					}
				}

				ms_spaceZoneMap.insert(std::make_pair(zone, spaceZone));

				++i;
			}
		} 
		while (zone);

		if (ms_spaceZoneMap.empty())
		{
			MessageBox(0, CString("Could not find any valid space zones [SpaceZones] in SwgSpaceQuestEditor.ini"), "SwgSpaceQuestEditor", MB_OK);
			return false;
		}
	}

	//-- Process quest categories
	{
		char const * category = 0;
		int i = 0;
		do
		{
			category = ConfigFile::getKeyString("QuestCategories", "category", i, 0);
			if (category && *category)
			{
				ms_questCategories.insert(category);

				++i;
			}
		} 
		while (category);

		if (ms_questCategories.empty())
		{
			MessageBox(0, CString("Could not find any categories [QuestCategories] in SwgSpaceQuestEditor.ini"), "SwgSpaceQuestEditor", MB_OK);
			return false;
		}
	}

	//-- Process mission template types
	{
		char const * type = 0;
		int i = 0;
		do
		{
			type = ConfigFile::getKeyString ("MissionTemplateTypes", "type", i, 0);
			if (type && *type)
			{
				std::pair<MissionTemplateMap::iterator, bool> result = ms_missionTemplateMap.insert(std::make_pair(type, new MissionTemplate(type)));
				DEBUG_REPORT_LOG(!result.second, ("%s is already a mission template type\n", type));

				++i;
			}
		} 
		while (type);

		if (ms_missionTemplateMap.empty())
		{
			MessageBox(0, CString("Could not find any mission template types [MissionTemplateTypes] in SwgSpaceQuestEditor.ini"), "SwgSpaceQuestEditor", MB_OK);
			return false;
		}
	}

	MissionTemplateMap::iterator end = ms_missionTemplateMap.end();
	for (MissionTemplateMap::iterator iter = ms_missionTemplateMap.begin(); iter != end; ++iter)
	{
		char const * const missionTemplateType = iter->first;
		MissionTemplate * const missionTemplate = iter->second;

		//-- Read mission template specific flags
		{
			missionTemplate->m_allowNavPointsInMultipleZones = ConfigFile::getKeyBool(missionTemplateType, "allowNavPointsInMultipleZones", false);
			missionTemplate->setJournalType(ConfigFile::getKeyString(missionTemplateType, "journalType", ""));
		}

		//-- Read information keys
		{
			char const * informationKey = 0;
			int i = 0;
			do
			{
				informationKey = ConfigFile::getKeyString(missionTemplateType, "information", i, 0);
				if (informationKey)
				{
					missionTemplate->m_information += informationKey;
					missionTemplate->m_information += "\r\n";

					++i;
				}
			} 
			while (informationKey);
		}

		//-- Read property template template keys
		{
			char const * propertyKey = 0;
			int i = 0;
			do
			{
				propertyKey = ConfigFile::getKeyString(missionTemplateType, "property", i, 0);
				if (propertyKey && *propertyKey)
				{
					MissionTemplate::PropertyTemplate propertyTemplate;
					propertyTemplate.parseIniString(propertyKey);
					missionTemplate->m_propertyTemplateList.push_back(propertyTemplate);

					if (propertyTemplate.m_propertyType == MissionTemplate::PropertyTemplate::PT_unknown)
					{
						MessageBox(0, CString("Mission template type ") + missionTemplateType + " specifies unknown property type for property key " + propertyKey, "SwgSpaceQuestEditor", MB_OK);
						return false;
					}

					++i;
				}
			} 
			while (propertyKey);
		}

		//-- Read string keys
		{
			char const * stringKey = 0;
			int i = 0;
			do
			{
				stringKey = ConfigFile::getKeyString(missionTemplateType, "string", i, 0);
				if (stringKey && *stringKey)
				{
					MissionTemplate::StringTemplate stringTemplate;
					stringTemplate.parseIniString(stringKey);
					missionTemplate->m_stringTemplateList.push_back(stringTemplate);

					++i;
				}
			} 
			while (stringKey);
		}

		//-- Read quest string suffix keys
		{
			char const * questStringSuffix = 0;
			int i = 0;
			do
			{
				questStringSuffix = ConfigFile::getKeyString(missionTemplateType, "questStringSuffix", i, 0);
				if (questStringSuffix && *questStringSuffix)
				{
					MissionTemplate::QuestStringSuffixTemplate questStringSuffixTemplate;
					questStringSuffixTemplate.parseIniString(questStringSuffix);
					missionTemplate->m_questStringSuffixTemplateList.push_back(questStringSuffixTemplate);

					++i;
				}
			} 
			while (questStringSuffix);
		}
	}

	return true;
}

// ----------------------------------------------------------------------

void Configuration::loadCargo(CString const & cargoName)
{
	if (cargoName.IsEmpty())
		return;

	CargoMap::iterator iter = ms_cargoMap.find(cargoName);
	if (iter == ms_cargoMap.end())
	{
		CString dataTableName = Configuration::createServerMissionDataTableName(cargoName, "inspect_cargo");
		dataTableName.Replace("/dsrc/", "/data/");
		dataTableName.Replace(".tab", ".iff");

		StringList stringList;

		Iff iff;
		if (iff.open(dataTableName, true))
		{
			DataTable dataTable;
			dataTable.load(iff);

			std::string const columnName("name");
			for (int i = 0; i < dataTable.getNumRows(); ++i)
			{
				std::string const & name = dataTable.getStringValue(columnName, i);
				stringList.push_back(name.c_str());
			}
		}

		ms_cargoMap.insert(std::make_pair(cargoName, stringList));
		DEBUG_REPORT_LOG(stringList.empty(), ("cargo list for %s is empty\n", cargoName));		
	}
}

// ----------------------------------------------------------------------

Vector Configuration::getNavPointLocation(CString const & spaceZone, CString const & navPoint)
{
	SpaceZoneMap::iterator iter = ms_spaceZoneMap.find(spaceZone);

	if (iter != ms_spaceZoneMap.end())
	{
		SpaceZone const & spaceZone = iter->second;
		SpaceZone::NavPointLocationMap::const_iterator navLocationIter = spaceZone.m_navPointLocationMap.find(navPoint);

		if (navLocationIter != spaceZone.m_navPointLocationMap.end())
			return navLocationIter->second;
	}

	return Vector::zero;
}

// ======================================================================

