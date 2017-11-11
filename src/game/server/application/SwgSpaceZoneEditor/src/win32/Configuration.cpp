// ======================================================================
//
// Configuration.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceZoneEditor.h"
#include "Configuration.h"

#include "fileInterface/AbstractFile.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedUtility/DataTable.h"

// ======================================================================

namespace ConfigurationNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	CString const cms_empty = "";
	CString const cms_zero = "0";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	CString ms_spaceMobileDataTableFileName;
	CString ms_squadDataTableFileName;

	StringList ms_spaceMobileList;

	typedef std::vector<Configuration::PropertyTemplate> PropertyTemplateList;
	PropertyTemplateList ms_propertyTemplateList;

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
}

using namespace ConfigurationNamespace;

// ======================================================================

Configuration::PropertyTemplate::PropertyTemplate() :
	m_hidden(false),
	m_mangled(false),
	m_name(),
	m_propertyType(PT_unknown),
	m_propertyData(),
	m_propertyDataList(),
	m_toolTipText()
{
}

// ----------------------------------------------------------------------

Configuration::PropertyTemplate::~PropertyTemplate()
{
}

// ----------------------------------------------------------------------

void Configuration::PropertyTemplate::parseIniString(CString const & iniString)
{
	CString buffer(iniString);

	int index = buffer.Find('|');
	if (index != -1)
	{
		m_name = buffer.Left(index);
		buffer = buffer.Right(buffer.GetLength() - index - 1);

		if (!m_name.IsEmpty() && m_name[0] == '*')
		{
			m_hidden = true;
			m_name = m_name.Right(m_name.GetLength() - 1);
		}

		if (!m_name.IsEmpty() && m_name[0] == '~')
		{
			m_mangled = true;
			m_name = m_name.Right(m_name.GetLength() - 1);
		}
	}

	index = buffer.Find('|');
	if (index != -1)
	{
		CString const propertyType = buffer.Left(index);
		buffer = buffer.Right(buffer.GetLength() - index - 1);

		if (propertyType == "PT_bool")
			m_propertyType = PT_bool;

		if (propertyType == "PT_integer")
			m_propertyType = PT_integer;

		if (propertyType == "PT_float")
			m_propertyType = PT_float;

		if (propertyType == "PT_string")
			m_propertyType = PT_string;

		if (propertyType.Find("PT_enumList", 0) == 0)
		{
			m_propertyType = PT_enumList;

			//-- Parse enum values
			CString propertyData = propertyType.Right(propertyType.GetLength() - 11);
			propertyData.Remove('(');
			propertyData.Remove(')');
			Configuration::unpackString(propertyData, m_propertyDataList, ',');
		}

		if (propertyType == "PT_spaceMobile")
			m_propertyType = PT_spaceMobile;

		if (propertyType == "PT_spaceMobileList")
			m_propertyType = PT_spaceMobileList;

		if (propertyType == "PT_patrolPointList")
			m_propertyType = PT_patrolPointList;

		if (propertyType == "PT_scriptList")
			m_propertyType = PT_scriptList;
	}

	if (!buffer.IsEmpty())
		m_toolTipText = buffer;
}

// ----------------------------------------------------------------------

CString const Configuration::PropertyTemplate::getPropertyTemplateFormatString() const
{
	CString result;

	result += "  property = [" + m_name + "]: " + getPropertyTypeString() + '\n';

	return result;
}

// ----------------------------------------------------------------------

CString const Configuration::PropertyTemplate::getPropertyData(int const index) const
{
	if (index >= 0 && index < static_cast<int>(m_propertyDataList.size()))
		return m_propertyDataList[index];

	return "ERROR(1): see asommers";
}

// ----------------------------------------------------------------------

CString const Configuration::PropertyTemplate::getPropertyTypeString() const
{
	switch (m_propertyType)
	{
	case PT_unknown:
		return "PT_unknown";

	case PT_bool:
		return "PT_bool";
	
	case PT_float:
		return "PT_float";

	case PT_integer:
		return "PT_integer";

	case PT_string:
		return "PT_string";
	
	case PT_enumList:
		return "PT_enumList";

	case PT_spaceMobile:
		return "PT_spaceMobile";

	case PT_spaceMobileList:
		return "PT_spaceMobileList";

	case PT_patrolPointList:
		return "PT_patrolPointList";

	case PT_scriptList:
		return "PT_scriptList";
	}

	return cms_empty;
}

// ----------------------------------------------------------------------

CString const Configuration::PropertyTemplate::getPropertyPackedObjVarTypeString() const
{
	switch (m_propertyType)
	{
	case PT_float:
		return "2";

	case PT_integer:
		return "0";

	case PT_string:
	case PT_enumList:
		return "4";

	case PT_unknown:
	case PT_bool:
	case PT_spaceMobile:
	case PT_spaceMobileList:
	case PT_patrolPointList:
	case PT_scriptList:
		return "-1";
	}

	return "-2";
}

// ======================================================================

bool Configuration::install()
{
	ExitChain::add(remove, "Configuration::remove");

	if (!loadCfg())
		return false;

	if (!loadIni())
		return false;

	return true;
}

// ----------------------------------------------------------------------

void Configuration::remove()
{
}

// ----------------------------------------------------------------------

CString const Configuration::getConfiguration()
{
	CString buffer;

	CString result;

	buffer.Format("%i spaceMobiles from %s\n", ms_spaceMobileList.size(), ms_spaceMobileDataTableFileName);
	result += buffer;

	{
		StringList::iterator end = ms_spaceMobileList.end();
		for (StringList::iterator iter = ms_spaceMobileList.begin(); iter != end; ++iter)
			result += "  mobile = " + *iter + "\n";
	}

	result += "Properties\n";

	{
		for (int i = 0; i < getNumberOfPropertyTemplates(); ++i)
			result += getPropertyTemplate(i).getPropertyTemplateFormatString();
			
	}

	return result;
}

// ----------------------------------------------------------------------

CString const & Configuration::getSpaceMobileDataTableFileName()
{
	return ms_spaceMobileDataTableFileName;
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
	return ms_spaceMobileList[index];
}

// ----------------------------------------------------------------------

bool Configuration::isValidSpaceMobile(CString const & spaceMobile)
{
	return std::find(ms_spaceMobileList.begin(), ms_spaceMobileList.end(), spaceMobile) != ms_spaceMobileList.end();
}

// ----------------------------------------------------------------------

int Configuration::getNumberOfPropertyTemplates() 
{
	return static_cast<int>(ms_propertyTemplateList.size());
}

// ----------------------------------------------------------------------

Configuration::PropertyTemplate const & Configuration::getPropertyTemplate(int const index) 
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfPropertyTemplates());
	return ms_propertyTemplateList[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

Configuration::PropertyTemplate const * Configuration::getPropertyTemplate(CString const & key) 
{
	for (PropertyTemplateList::const_iterator iter = ms_propertyTemplateList.begin(); iter != ms_propertyTemplateList.end(); ++iter)
		if (iter->m_name == key)
			return &*iter;

	return 0;
}

// ----------------------------------------------------------------------

CString const Configuration::extractBranch(CString const & path)
{
	CString buffer(path);
	buffer.Replace('\\', '/');
	buffer = buffer.Right(buffer.GetLength() - buffer.Find("swg") - 4);
	int const index = buffer.Find('/');
	buffer = buffer.Left(index);
	return buffer;
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

// ----------------------------------------------------------------------

void Configuration::packObjVars(ObjVarList const & objVarList, CString & string)
{
	for (size_t i = 0; i < objVarList.size(); ++i)
	{
		ObjVar const & objVar = objVarList[i];

		PropertyTemplate const & propertyTemplate = *NON_NULL(getPropertyTemplate(objVar.m_key));

		//-- For counted objvars, we count what we're pointing to
		if (propertyTemplate.m_propertyType == PropertyTemplate::PT_count)
		{
		}
		else
		{
			//-- For mangled objvars, we must break up the objvar into segments
			if (propertyTemplate.m_mangled)
			{
				StringList stringList;
				Configuration::unpackString(objVar.m_value, stringList, ':'); 

				//-- Break up string list into segments
				StringList segmentList;
				{
					int count = 0;
					segmentList.push_back(cms_empty);

					for (size_t j = 0; j < stringList.size(); ++j)
					{
						if (count >= 10)
						{
							count = 0;
							segmentList.push_back(cms_empty);
						}

						++count;
						segmentList.back() += stringList[j] + ":";
					}
				}
				
				char buffer[128];

				//-- First write the count
				string += objVar.m_key + "_mangled.count";
				string += '|';
				string += "0";
				string += '|';
				string += _itoa(stringList.size(), buffer, 10);
				string += '|';


				//-- Write segments
				for (size_t j = 0; j < segmentList.size(); ++j)
				{
					string += objVar.m_key + "_mangled.segment." + _itoa(j, buffer, 10);
					string += '|';
					string += "5";
					string += '|';
					string += segmentList[j];
					string += '|';
				}
			}
			else
			{
				//-- For standard objvars, we just write the value
				string += objVar.m_key;
				string += '|';
				string += propertyTemplate.getPropertyPackedObjVarTypeString();
				string += '|';
				string += objVar.m_value;
				string += '|';
			}
		}
	}

	string += "$|";
}

// ----------------------------------------------------------------------

void Configuration::unpackObjVars(CString const & string, ObjVarList & objVarList)
{
	StringList stringList;
	Configuration::unpackString(string, stringList, '|');

	typedef std::map<CString, std::pair<int, CString> > ObjVarMap;
	ObjVarMap objVarMap;

	CString result;
	for (size_t i = 0; i < stringList.size();)
	{
		if (stringList[i] == "$")
			break;

		CString key = stringList[i++];
		CString type = stringList[i++];
		CString value = stringList[i++];

		if (type == "2")
		{
			char buffer[128];
			value = _itoa(atoi(value), buffer, 10);
		}

		int index = key.Find("_mangled.count");
		if (index != -1)
			continue;

		index = key.Find("_mangled.segment");
		if (index != -1)
		{
			std::pair<int, CString> & data = objVarMap[key.Left(index)];
			data.first = atoi(type);
			data.second += value;
		}
		else
		{
			objVarMap[key] = std::make_pair(atoi(type), value);
		}
	}

	for (ObjVarMap::iterator iter = objVarMap.begin(); iter != objVarMap.end(); ++iter)
	{
		ObjVar objVar;
		objVar.m_key = iter->first;
		objVar.m_value = iter->second.second;
		objVarList.push_back(objVar);
	}
}

// ======================================================================

bool Configuration::loadCfg()
{
	ms_spaceMobileDataTableFileName = ConfigFile::getKeyString("SwgSpaceZoneEditor", "spaceMobileDataTable", ms_spaceMobileDataTableFileName);
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
			for (int i = 0; i < dataTable.getNumRows(); ++i)
			{
				std::string const & name = dataTable.getStringValue(columnName, i);
				ms_spaceMobileList.push_back(name.c_str());
			}
		}
	}

	ms_squadDataTableFileName = ConfigFile::getKeyString("SwgSpaceZoneEditor", "squadDataTable", ms_squadDataTableFileName);
	trimEndingSlashes(ms_squadDataTableFileName);

	//-- Load space_mobile.iff
	{
		Iff iff;
		if (!iff.open(ms_squadDataTableFileName, true))
		{
			DEBUG_WARNING(true, ("Configuration::loadCfg: could not open file %s\n", ms_squadDataTableFileName));
			return false;
		}
		
		DataTable dataTable;
		dataTable.load(iff);

		{
			std::string const columnName("strSquadName");
			for (int i = 0; i < dataTable.getNumRows(); ++i)
			{
				std::string const & name = dataTable.getStringValue(columnName, i);
				ms_spaceMobileList.push_back(name.c_str());
			}
		}
	}

	std::sort(ms_spaceMobileList.begin(), ms_spaceMobileList.end());

	return !ms_spaceMobileList.empty();
}

// ----------------------------------------------------------------------

bool Configuration::loadIni()
{
	//-- Load SwgSpaceQuestEditor.ini
	{
		AbstractFile * const abstractFile = TreeFile::open("SwgSpaceZoneEditor.ini", AbstractFile::PriorityData, true);
		if (abstractFile)
		{
			int const length = abstractFile->length();
			byte * const data = abstractFile->readEntireFileAndClose();
			IGNORE_RETURN(ConfigFile::loadFromBuffer(reinterpret_cast<char const *>(data), length));
			delete []data;
			delete abstractFile;
		}
	}

	//-- Read properties
	{
		char const * propertyKey = 0;
		int i = 0;
		do
		{
			propertyKey = ConfigFile::getKeyString("Properties", "property", i, 0);
			if (propertyKey && *propertyKey)
			{
				PropertyTemplate propertyTemplate;
				propertyTemplate.parseIniString(propertyKey);
				ms_propertyTemplateList.push_back(propertyTemplate);

				if (propertyTemplate.m_propertyType == PropertyTemplate::PT_unknown)
				{
					MessageBox(0, CString("Unknown property type for property key ") + propertyKey, 0, MB_OK);

					return false;
				}

				++i;
			}
		} 
		while (propertyKey);
	}

	return true;
}

// ======================================================================

