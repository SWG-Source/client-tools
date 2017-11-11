// ======================================================================
//
// SpaceQuest.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceQuestEditor.h"
#include "SpaceQuest.h"

#include "Configuration.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedMath/Vector.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableWriter.h"

// ======================================================================

namespace SpaceQuestNamespace
{
	int countNumberOfRows(CString const & packedRow);
	void setQuestTaskHeader(CString & buffer, bool const includeWaypoint = false);
	void addQuestTaskRow(CString & buffer, CString const & title, CString const & description, bool const includeEmptyWaypoint = false, bool const newLine = true);
	void addQuestTaskRow(CString & buffer, CString const & qualifier, bool const includeEmptyWaypoint = false, bool const newLine = true);
	void addQuestTaskWaypoint(CString & buffer, CString const & zone, Vector const & location);
	void addQuestTaskNavPointRows(CString & buffer, StringList const & qualifierList, CString const & navPoints);
	void addQuestTaskTravelToRow(CString & buffer, CString const & zone, bool const includeWaypoint = false);
	void addQuestTaskTravelToRow(CString & buffer, CString const & zone, CString const & description, bool const includeWaypoint = false);
}

using namespace SpaceQuestNamespace;

// ======================================================================

#define QUEST_JOURNAL_STRING(a) (CString(missionTemplateType) + "/" + rootName + ":" + missionTemplate->getQuestStringSuffixTemplate(a).m_suffix)

// ======================================================================

SpaceQuest::SpaceQuest() :
	m_propertyList(),
	m_stringTable(),
	m_questCategory()
{
}

// ----------------------------------------------------------------------

SpaceQuest::~SpaceQuest()
{
}

// ----------------------------------------------------------------------

bool SpaceQuest::load(char const * const rootName, char const * const missionTemplateType)
{
	loadMissionDataTable(rootName, missionTemplateType);
	loadStringTable(rootName, missionTemplateType);
	loadQuestListDataTable(rootName, missionTemplateType);
	loadQuestTaskDataTable(rootName, missionTemplateType);

	return true;
}

// ----------------------------------------------------------------------

bool SpaceQuest::loadMissionDataTable(char const * const rootName, char const * const missionTemplateType)
{
	Configuration::MissionTemplate const * const missionTemplate = Configuration::getMissionTemplate(missionTemplateType);
	if (!missionTemplate)
		return false;

	//-- Load mission data table
	CString const dataTableName = Configuration::createServerMissionDataTableName(rootName, missionTemplateType);
	if (TreeFile::exists(dataTableName))
	{
		DataTableWriter writer;
		writer.loadFromSpreadsheet(dataTableName);

		Iff iff(1024);
		iff.allowNonlinearFunctions();
		writer.save(iff);
		iff.goToTopOfForm();

		DataTable dataTable;
		dataTable.load(iff);

		for (int i = 0; i < dataTable.getNumColumns(); ++i)
		{
			std::string const & columnName = dataTable.getColumnName(i);
			Configuration::MissionTemplate::PropertyTemplate const * const propertyTemplate = missionTemplate->getPropertyTemplate(columnName.c_str());
			if (propertyTemplate)
			{
				CString rowData;

				switch (dataTable.getDataTypeForColumn(i).getBasicType())
				{
				case DataTableColumnType::DT_String:
					{
						for (int j = 0; j < dataTable.getNumRows(); ++j)
						{
							CString value;

							std::string const & stringValue = dataTable.getStringValue(i, j);
							value += stringValue.c_str();

							if (j != 0)
								rowData += '|';

							rowData += value;
						}
					}
					break;

				case DataTableColumnType::DT_Int:
					{
						switch (propertyTemplate->m_propertyType)
						{
						//-- For PT_string, it's stored in the datatable as an int, but we need to convert it to a string and collect all of them
						case Configuration::MissionTemplate::PropertyTemplate::PT_string:
							{
								for (int j = 0; j < dataTable.getNumRows(); ++j)
								{
									int const intValue = dataTable.getIntValue(i, j);

									CString value;
									value.Format("%i", intValue);

									if (j != 0)
										rowData += '|';

									rowData += value;
								}
							}
							break;

						//-- For PT_enumList, it's stored in the datatable as an int, but we need to convert it to a string
						case Configuration::MissionTemplate::PropertyTemplate::PT_enumList:
							{
								int const intValue = dataTable.getIntValue(i, 0);
								rowData = propertyTemplate->getPropertyData(intValue);
							}
							break;

						default:
							{
								int const intValue = dataTable.getIntValue(i, 0);
								rowData.Format("%i", intValue);
							}
							break;
						}
					}
					break;
				}

				while (!rowData.IsEmpty() && rowData[rowData.GetLength() - 1] == '|')
					rowData = rowData.Left(rowData.GetLength() - 1);

				setProperty(columnName.c_str(), rowData);
			}
		}
	}

	return true;
}

// ----------------------------------------------------------------------

bool SpaceQuest::loadQuestListDataTable(char const * const rootName, char const * const missionTemplateType)
{
	Configuration::MissionTemplate const * const missionTemplate = Configuration::getMissionTemplate(missionTemplateType);
	if (!missionTemplate)
		return false;

	//-- Load quest list data table
	CString dataTableName = Configuration::createSharedQuestListDataTableName(rootName, missionTemplateType);
	if (TreeFile::exists(dataTableName))
	{
		DataTableWriter writer;
		writer.loadFromSpreadsheet(dataTableName);

		Iff iff(1024);
		iff.allowNonlinearFunctions();
		writer.save(iff);
		iff.goToTopOfForm();

		DataTable dataTable;
		dataTable.load(iff);

		if (dataTable.getNumRows() > 0)
			setQuestCategory(dataTable.getStringValue(std::string("CATEGORY"), 0));
	}

	return true;
}

// ----------------------------------------------------------------------

bool SpaceQuest::loadQuestTaskDataTable(char const * const rootName, char const * const missionTemplateType)
{
	UNREF(rootName);
	UNREF(missionTemplateType);
	return true;
}

// ----------------------------------------------------------------------

bool SpaceQuest::loadStringTable(char const * const rootName, char const * const missionTemplateType)
{
	//-- Load mission string table
	CString const stringTableName = Configuration::createSharedStringTableName(rootName, missionTemplateType);
	if (TreeFile::exists(stringTableName))
		m_stringTable.load(stringTableName);

	return true;
}

// ----------------------------------------------------------------------

bool SpaceQuest::save(char const * const rootName, char const * const missionTemplateType) const
{
	bool result = saveMissionDataTable(rootName, missionTemplateType);
	if (!result)
		return false;

	result = saveStringTable(rootName, missionTemplateType);
	if (!result)
		return false;

	result = saveQuestListDataTable(rootName, missionTemplateType);
	if (!result)
		return false;

	result = saveQuestTaskDataTable(rootName, missionTemplateType);
	if (!result)
		return false;

	return true;
}

// ----------------------------------------------------------------------

bool SpaceQuest::saveMissionDataTable(char const * const rootName, char const * const missionTemplateType) const
{
	Configuration::MissionTemplate const * const missionTemplate = Configuration::getMissionTemplate(missionTemplateType);
	if (!missionTemplate)
		return false;

	//-- Determine the number of columns
	int const numberOfColumns = missionTemplate->getNumberOfPropertyTemplates();

	//-- Determine the number of rows
	int numberOfRows = 0;
	{
		for (int i = 0; i < numberOfColumns; ++i)
		{
			Configuration::MissionTemplate::PropertyTemplate const & propertyTemplate = missionTemplate->getPropertyTemplate(i);
			CString const property = getProperty(propertyTemplate.m_columnName);
			numberOfRows = max(countNumberOfRows(property), numberOfRows);
		}
	}

	//-- Fill out the dataTable data
	std::vector<CString> dataTableData;
	dataTableData.resize(numberOfColumns * numberOfRows);
	{
		for (int i = 0; i < numberOfColumns; ++i)
		{
			Configuration::MissionTemplate::PropertyTemplate const & propertyTemplate = missionTemplate->getPropertyTemplate(i);
			CString property = getProperty(propertyTemplate.m_columnName);

			int j = 0;
			while (!property.IsEmpty())
			{
				int index = property.Find('|');
				if (index == -1)
				{
					dataTableData[j * numberOfColumns + i] = property;
					property.Empty();
				}
				else
				{
					dataTableData[j * numberOfColumns + i] = property.Left(index);
					property = property.Right(property.GetLength() - index - 1);
				}

				++j;
			}
		}
	}

	CString const dataTableName = Configuration::createServerMissionDataTableName(rootName, missionTemplateType);

	//-- Write tab
	{
		//-- Create the destination directory if it doesn't exist
		CStdioFile outfile;
		if (outfile.Open(dataTableName, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
		{
			//-- Write column headers
			{
				for (int i = 0; i < numberOfColumns; ++i)
				{
					Configuration::MissionTemplate::PropertyTemplate const & propertyTemplate = missionTemplate->getPropertyTemplate(i);
					outfile.WriteString(propertyTemplate.m_columnName);

					if (i < numberOfColumns - 1)
						outfile.WriteString("\t");
				}

				outfile.WriteString("\n");
			}

			//-- Write column types
			{
				for (int i = 0; i < numberOfColumns; ++i)
				{
					Configuration::MissionTemplate::PropertyTemplate const & propertyTemplate = missionTemplate->getPropertyTemplate(i);
					outfile.WriteString(propertyTemplate.m_dataTableHeader);

					if (i < numberOfColumns - 1)
						outfile.WriteString("\t");
				}

				outfile.WriteString("\n");
			}

			//-- Write rows
			{
				for (int i = 0; i < numberOfRows; ++i)
				{
					for (int j = 0; j < numberOfColumns; ++j)
					{
						outfile.WriteString(dataTableData[i * numberOfColumns + j]);

						if (j < numberOfColumns - 1)
							outfile.WriteString("\t");
					}

					outfile.WriteString("\n");
				}
			}
		}	
		else
		{
			CString const message = dataTableName + " could not be written.  Does the directory exist?  Is the file currently open in another application?";
			MessageBox(0, message, "", MB_ICONWARNING | MB_OK);

			return false;
		}
	}

	//-- write iff
	{
		CString iffFileName(dataTableName);

		//-- Replace dsrc w/ data
		iffFileName.Replace("dsrc", "data");

		//-- Replace .tab w/ .iff
		iffFileName.Replace(".tab", ".iff");

		DataTableWriter writer;
		writer.loadFromSpreadsheet(dataTableName);

		Iff iff(1024);
		writer.save(iff);

		if (!iff.write(iffFileName, true))
		{
			MessageBox(0, CString("Could not write ") + iffFileName, 0, MB_ICONWARNING | MB_OK);

			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------

bool SpaceQuest::saveStringTable(char const * const rootName, char const * const missionTemplateType) const
{
	//-- Save string table
	CString const stringTableName = Configuration::createSharedStringTableName(rootName, missionTemplateType);

	//-- Create the destination directory if it doesn't exist
	if (!m_stringTable.save(stringTableName))
	{
		CString const message = stringTableName + " could not be written.  Does the directory exist?  Is the file currently open in another application?";
		MessageBox(0, message, "", MB_ICONWARNING | MB_OK);

		return false;
	}

	return true;
}

// ----------------------------------------------------------------------

bool SpaceQuest::saveQuestListDataTable(char const * const rootName, char const * const missionTemplateType) const
{
	Configuration::MissionTemplate const * const missionTemplate = Configuration::getMissionTemplate(missionTemplateType);
	if (!missionTemplate)
		return false;

	if (missionTemplate->getNumberOfQuestStringSuffixTemplates() == 0)
		return true;

	CString buffer;
	buffer += "JOURNAL_ENTRY_TITLE\tJOURNAL_ENTRY_DESCRIPTION\tPREREQUISITE_QUESTS\tEXCLUSION_QUESTS\tALLOW_REPEATS\tCATEGORY\n";
	buffer += "s\ts\ts\ts\tb\ts\n";
	buffer += CString("@spacequest/") + missionTemplateType + "/" + rootName + ":title\t";
	buffer += CString("@spacequest/") + missionTemplateType + "/" + rootName + ":title_d\t";
	buffer += "\t\t1\t" + getQuestCategory() + "\n";

	CString const dataTableName = Configuration::createSharedQuestListDataTableName(rootName, missionTemplateType);

	//-- write tab
	{
		//-- Save data table
		CStdioFile outfile;
		if (outfile.Open(dataTableName, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
			outfile.WriteString(buffer);
		else
		{
			MessageBox(0, CString("Could not write ") + dataTableName, 0, MB_ICONWARNING | MB_OK);

			return false;
		}
	}

	//-- write iff
	{
		CString iffFileName(dataTableName);

		//-- Replace dsrc w/ data
		iffFileName.Replace("dsrc", "data");

		//-- Replace .tab w/ .iff
		iffFileName.Replace(".tab", ".iff");

		DataTableWriter writer;
		writer.loadFromSpreadsheet(dataTableName);

		Iff iff(1024);
		writer.save(iff);

		if (!iff.write(iffFileName, true))
		{
			MessageBox(0, CString("Could not write ") + iffFileName, 0, MB_ICONWARNING | MB_OK);

			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------

bool SpaceQuest::saveQuestTaskDataTable(char const * const rootName, char const * const missionTemplateType) const
{
	Configuration::MissionTemplate const * const missionTemplate = Configuration::getMissionTemplate(missionTemplateType);
	if (!missionTemplate)
		return false;

	if (missionTemplate->getNumberOfQuestStringSuffixTemplates() == 0)
		return true;

	CString buffer;

	setQuestTaskHeader(buffer);

	switch (missionTemplate->getJournalType())
	{
	case Configuration::MissionTemplate::JT_default:
		{
			for (int i = 0; i < missionTemplate->getNumberOfQuestStringSuffixTemplates(); ++i)
			{
				addQuestTaskRow(buffer, QUEST_JOURNAL_STRING(i));
			}
		}
		break;

	case Configuration::MissionTemplate::JT_generic:
		{
			addQuestTaskTravelToRow(buffer, getProperty("questZone"));

			addQuestTaskRow(buffer, "generic:" + CString(missionTemplateType));
		}
		break;

	case Configuration::MissionTemplate::JT_genericLocationTitle:
		{
			addQuestTaskTravelToRow(buffer, getProperty("questZone"), QUEST_JOURNAL_STRING(0));

			for (int i = 1; i < missionTemplate->getNumberOfQuestStringSuffixTemplates(); ++i)
			{
				addQuestTaskRow(buffer, QUEST_JOURNAL_STRING(i));
			}
		}
		break;

	case Configuration::MissionTemplate::JT_navigationPoints:
		{
			addQuestTaskTravelToRow(buffer, getProperty("questZone"), QUEST_JOURNAL_STRING(0));
			
			//-- use the target as normal
			addQuestTaskRow(buffer, QUEST_JOURNAL_STRING(1));

			//-- for each navigation point, add the default text plus the waypoint data
			CString const & navPoints = getProperty("navPoints");
			int const numberOfNavPoints = countNumberOfRows(navPoints);

			//-- use the default text title and description for each navigation point
			for (int i = 0; i < numberOfNavPoints; ++i)
			{
				addQuestTaskRow(buffer, QUEST_JOURNAL_STRING(2));
			}
		}
		break;

	case Configuration::MissionTemplate::JT_spawners:
		{
			addQuestTaskTravelToRow(buffer, getProperty("questZone"), QUEST_JOURNAL_STRING(0));
			
			for (int j = 1; j < missionTemplate->getNumberOfQuestStringSuffixTemplates(); ++j)
			{
				addQuestTaskRow(buffer, QUEST_JOURNAL_STRING(j));
			}

			CString const & validSpawns = getProperty("validSpawns");
			int const numberOfValidSpawns = countNumberOfRows(validSpawns);

			for (int i = 0; i < numberOfValidSpawns; ++i)
			{
				addQuestTaskRow(buffer, "generic:possible_target_location");
			}
		}
		break;

	default:
		{
			MessageBox(0, CString("Invalid journal type for ") + missionTemplateType, "SwgSpaceQuestEditor", MB_OK);
			return false;
		}
	}

	CString const dataTableName = Configuration::createSharedQuestTaskDataTableName(rootName, missionTemplateType);

	//-- write tab
	{
		//-- Save data table
		CStdioFile outfile;
		if (outfile.Open(dataTableName, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
			outfile.WriteString(buffer);
		else
		{
			MessageBox(0, CString("Could not write ") + dataTableName, 0, MB_ICONWARNING | MB_OK);

			return false;
		}
	}

	//-- write iff
	{
		CString iffFileName(dataTableName);

		//-- Replace dsrc w/ data
		iffFileName.Replace("dsrc", "data");

		//-- Replace .tab w/ .iff
		iffFileName.Replace(".tab", ".iff");

		DataTableWriter writer;
		writer.loadFromSpreadsheet(dataTableName);

		Iff iff(1024);
		writer.save(iff);

		if (!iff.write(iffFileName, true))
		{
			MessageBox(0, CString("Could not write ") + iffFileName, 0, MB_ICONWARNING | MB_OK);

			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------

CString const & SpaceQuest::getProperty(CString const & key) const
{
	return m_propertyList.getString(key);
}

// ----------------------------------------------------------------------

void SpaceQuest::setProperty(CString const & key, CString const & value)
{
	m_propertyList.setString(key, value);
}

// ----------------------------------------------------------------------

int SpaceQuest::getNumberOfStrings() const
{
	return m_stringTable.getNumberOfStrings();
}

// ----------------------------------------------------------------------

CString const & SpaceQuest::getStringKey(int const index) const
{
	return m_stringTable.getStringKey(index);
}

// ----------------------------------------------------------------------

CString const & SpaceQuest::getString(CString const & key) const
{
	return m_stringTable.getString(key);
}

// ----------------------------------------------------------------------

bool SpaceQuest::hasStringKey(CString const & key) const
{
	return m_stringTable.hasStringKey(key);
}

// ----------------------------------------------------------------------

void SpaceQuest::setString(CString const & key, CString const & value)
{
	m_stringTable.setString(key, value);
}

// ----------------------------------------------------------------------

void SpaceQuest::removeString(CString const & key)
{
	m_stringTable.removeString(key);
}

// ----------------------------------------------------------------------

CString const & SpaceQuest::getQuestCategory() const
{
	return m_questCategory;
}

// ----------------------------------------------------------------------

void SpaceQuest::setQuestCategory(CString const & questCategory)
{
	m_questCategory = questCategory;
}

// ======================================================================

int SpaceQuestNamespace::countNumberOfRows(CString const & packedRow)
{
	int count = 1;
	for (int i = 0; i < packedRow.GetLength(); ++i)
		if (packedRow[i] == '|')
			++count;

	return count;
}

// ----------------------------------------------------------------------

void SpaceQuestNamespace::setQuestTaskHeader(CString & buffer, bool const includeWaypoint)
{
	char const * const columnNames = 
		"PREREQUISITE_TASKS\tEXCLUSION_TASKS\tATTACH_SCRIPT\t"
		"JOURNAL_ENTRY_TITLE\tJOURNAL_ENTRY_DESCRIPTION\tTASKS_ON_COMPLETE\t"
		"TASKS_ON_FAIL\tTARGET\tPARAMETER\tIS_VISIBLE\tSHOW_SYSTEM_MESSAGES\tALLOW_REPEATS";

	char const * const columnTypes = "s\ts\ts\t" "s\ts\ts\t" "s\ts\ts\tb\tb\tb";

	char const * const columnNamesWaypoint = "\tCREATE_WAYPOINT\tPLANET_NAME\tLOCATION_X\tLOCATION_Y\tLOCATION_Z";

	char const * const columnTypesWaypoint = "\tb\ts\ts\ts\ts";

	buffer.Empty();

	//-- names
	{
		buffer += columnNames;

		if (includeWaypoint)
			buffer += columnNamesWaypoint;

		buffer += "\n";
	}

	//-- types
	{
		buffer += columnTypes;

		if (includeWaypoint)
			buffer += columnTypesWaypoint;

		buffer += "\n";
	}
}

// ----------------------------------------------------------------------

void SpaceQuestNamespace::addQuestTaskRow(CString & buffer, CString const & title, CString const & description, bool const includeEmptyWaypoint, bool const newLine)
{
	char const * const rowPrefix = "\t\t\t";
	char const * const rowPostfix = "\t\t\t\t1\t\t1";
	char const * const rowPostfixEmptyWaypoint = "\t0\t\t\t\t";

	buffer += rowPrefix;
	buffer += CString("@spacequest/") + title + "\t";
	buffer += CString("@spacequest/") + description + "\t";
	buffer += rowPostfix;

	if (includeEmptyWaypoint)
	{
		buffer += rowPostfixEmptyWaypoint;
	}

	if (newLine)
	{
		buffer += "\n";
	}
}

// ----------------------------------------------------------------------

void SpaceQuestNamespace::addQuestTaskRow(CString & buffer, CString const & qualifier, bool const includeEmptyWaypoint, bool const newLine)
{
	addQuestTaskRow(buffer, qualifier + "_t", qualifier + "_d", includeEmptyWaypoint, newLine);
}

// ----------------------------------------------------------------------

void SpaceQuestNamespace::addQuestTaskWaypoint(CString & buffer, CString const & zone, Vector const & location)
{
	buffer += FormattedString<256>().sprintf("\t1" "\t%s" "\t%f" "\t%f" "\t%f" "\n",
		(LPCSTR)zone, location.x, location.y, location.z);
}

// ----------------------------------------------------------------------

void SpaceQuestNamespace::addQuestTaskNavPointRows(CString & buffer, StringList const & qualifierList, CString const & navPoints)
{
	StringList navPointList;
	Configuration::unpackString(navPoints, navPointList, '|');
	
	size_t const numberOfNavPoints = navPointList.size();

	for (size_t i = 0; i < numberOfNavPoints; ++i)
	{
		StringList zoneNavPoint;
		Configuration::unpackString(navPointList[i], zoneNavPoint, ':');

		Vector navLocation = Configuration::getNavPointLocation(zoneNavPoint[0], zoneNavPoint[1]);

		addQuestTaskRow(buffer, qualifierList[i], false, false);

		addQuestTaskWaypoint(buffer, zoneNavPoint[0], navLocation);
	}
}

// ----------------------------------------------------------------------

void SpaceQuestNamespace::addQuestTaskTravelToRow(CString & buffer, CString const & zone, bool const includeWaypoint)
{
	//-- use the questZone to specify "travel to" journal text
	addQuestTaskRow(buffer, CString("generic:") + zone, includeWaypoint);
}

// ----------------------------------------------------------------------

void SpaceQuestNamespace::addQuestTaskTravelToRow(CString & buffer, CString const & zone, CString const & description, bool const includeWaypoint)
{
	//-- use the questZone to specify "travel to" journal text
	addQuestTaskRow(buffer, CString("generic:") + zone + "_t", description + "_d", includeWaypoint);
}

// ======================================================================
