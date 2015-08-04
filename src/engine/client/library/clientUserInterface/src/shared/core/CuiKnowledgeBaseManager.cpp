//======================================================================
//
// CuiKnowledgeBaseManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiKnowledgeBaseManager.h"

#include "LocalizationManager.h"
#include "UnicodeUtils.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "sharedFile/Iff.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include <map>
#include <vector>

//======================================================================

namespace CuiKnowledgeBaseManagerNamespace
{
	bool s_installed = false;

	const std::string s_baseDir     ("datatables/knowledgebase/");
	const std::string s_filenameExt (".iff");
	const std::string s_rootName    ("Root");

	bool s_fatalOnBadKBEntry = true;
	bool s_tablesLoaded = false;

	typedef stdmap<std::string, CuiKnowledgeBaseManager::BaseKBNode*>::fwd NamePageMap;
	NamePageMap s_namePageMap;
}

using namespace CuiKnowledgeBaseManagerNamespace;

//----------------------------------------------------------------------

CuiKnowledgeBaseManager::PageKBNode* CuiKnowledgeBaseManager::m_kbRoot;

const std::string CuiKnowledgeBaseManager::s_pageType   = "Page";
const std::string CuiKnowledgeBaseManager::s_stringType = "String";
const std::string CuiKnowledgeBaseManager::s_imageType  = "Image";
const std::string CuiKnowledgeBaseManager::s_commandButtonType = "CommandButton";
const std::string CuiKnowledgeBaseManager::s_linkButtonType    = "LinkButton";
const std::string CuiKnowledgeBaseManager::s_scriptMessageButtonType = "ScriptMessageButton";   

//----------------------------------------------------------------------

CuiKnowledgeBaseManager::BaseKBNode::BaseKBNode(const std::string& type, BaseKBNode* parent, const std::string& name)
: m_parent(parent),
  m_name(name),
  m_children(),
  m_type(type)
{}

//----------------------------------------------------------------------

void CuiKnowledgeBaseManager::BaseKBNode::clear()
{
	m_parent = NULL;
	m_name.clear();
	m_children.clear();
	s_namePageMap.clear();
}

//----------------------------------------------------------------------

CuiKnowledgeBaseManager::BaseKBNode::~BaseKBNode()
{
	for(std::vector<BaseKBNode*>::iterator i = m_children.begin(); i != m_children.end(); ++i)
	{
		delete *i;
	}
	m_parent = NULL;
}

//----------------------------------------------------------------------

CuiKnowledgeBaseManager::StringKBNode::StringKBNode(BaseKBNode* parent, const std::string& name, const StringId& s)
: BaseKBNode(s_stringType, parent, name),
  m_string(s)
{}

//----------------------------------------------------------------------

CuiKnowledgeBaseManager::PageKBNode::PageKBNode(BaseKBNode* parent, const std::string& name, const StringId& s)
: BaseKBNode(s_pageType, parent, name),
  m_string(s)
{}

//----------------------------------------------------------------------

CuiKnowledgeBaseManager::ImageKBNode::ImageKBNode(BaseKBNode* parent, const std::string& name, const std::string& path)
: BaseKBNode(s_imageType, parent, name),
  m_path(path)
{}

//----------------------------------------------------------------------

CuiKnowledgeBaseManager::CommandButtonKBNode::CommandButtonKBNode(BaseKBNode* parent, const std::string& name, const std::string& str, const std::string& command)
: BaseKBNode(s_commandButtonType, parent, name),
  m_string(str),
  m_command(command)
{}

//----------------------------------------------------------------------

CuiKnowledgeBaseManager::LinkButtonKBNode::LinkButtonKBNode(BaseKBNode* parent, const std::string& name, const std::string& str, const std::string& link)
: BaseKBNode(s_linkButtonType, parent, name),
  m_string(str),
  m_link(link)
{}

//----------------------------------------------------------------------

CuiKnowledgeBaseManager::ScriptMessageButtonKBNode::ScriptMessageButtonKBNode(BaseKBNode* parent, const std::string& name, const std::string& str, const std::string& messageName)
: BaseKBNode(s_scriptMessageButtonType, parent, name),
  m_string(str),
  m_messageName(messageName)
{}

//----------------------------------------------------------------------

void CuiKnowledgeBaseManager::install ()
{
	DEBUG_FATAL (s_installed, ("installed"));
	s_installed = true;

	s_fatalOnBadKBEntry = ConfigClientUserInterface::getFatalOnBadKnowldgeBaseEntry();

	m_kbRoot = new PageKBNode(NULL, s_rootName, StringId::cms_invalid);
	m_kbRoot->clear();
	m_kbRoot->m_name = s_rootName;
	s_namePageMap.insert(std::make_pair(s_rootName, m_kbRoot));


	loadTables();
}

//----------------------------------------------------------------------

void CuiKnowledgeBaseManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	clearData();

	s_installed = false;

	delete m_kbRoot;
	m_kbRoot = NULL;
}

//----------------------------------------------------------------------

void CuiKnowledgeBaseManager::reloadData()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	clearData();
	loadTables();
}

//----------------------------------------------------------------------

void CuiKnowledgeBaseManager::clearData()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	delete m_kbRoot;
	m_kbRoot = new PageKBNode(NULL, s_rootName, StringId::cms_invalid);
	m_kbRoot->m_name = s_rootName;
	s_namePageMap.clear();
	s_namePageMap.insert(std::make_pair(s_rootName, m_kbRoot));
}

//----------------------------------------------------------------------

void CuiKnowledgeBaseManager::loadTables()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	//for table in kb directory
		//load table

	//read in the file list, read each file
	static std::string const fileTableName = "datatables/knowledgebase/filelist.iff";

	//reload the table if necessary in case we're rebuilding the kb 
	if(s_tablesLoaded)
		DataTableManager::reload(fileTableName);

	const DataTable * const fileTable = DataTableManager::getTable (fileTableName.c_str(), true);
	if (fileTable)
	{
		const int numRows = fileTable->getNumRows ();

		for (int i = 0; i < numRows; ++i)
		{
			const std::string & filename = fileTable->getStringValue (0, i);

			const std::string & tableName = s_baseDir + filename;
			if(s_tablesLoaded)
				DataTableManager::reload(tableName);
			const DataTable * const table = DataTableManager::getTable (tableName.c_str(), true);
			if (table)
			{
				//reload the table first in case we're rebuilding the kb 
				loadTable(*table, tableName);
			}
			else
			{
				DEBUG_FATAL(s_fatalOnBadKBEntry, ("Could not load knowledgebase file %s", filename.c_str()));
			}
		}
	}

	s_tablesLoaded = true;
}

//----------------------------------------------------------------------

void CuiKnowledgeBaseManager::loadTable(const DataTable& table, const std::string& tableName)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	//unref necessary for release build
	UNREF(tableName);
	//for entry in table
		//build into metadata structure

	const int numRows = table.getNumRows ();

	for (int i = 0; i < numRows; ++i)
	{
		const std::string & parent = table.getStringValue (0, i);
		const std::string & name   = table.getStringValue (1, i);
		const std::string & type   = table.getStringValue (2, i);
		const std::string & data   = table.getStringValue (3, i);
		const std::string & data2  = table.getStringValue (4, i);

		BaseKBNode* parentNode = NULL;
		if(parent.empty())
			parentNode = findNode(s_rootName);
		else
			parentNode = findNode(parent);
				
		if(!parentNode)
		{
			DEBUG_FATAL(s_fatalOnBadKBEntry, ("Row %d in table %s has unknown parent %s", i+3, tableName.c_str(), parent.c_str()));
			return;
		}

		std::string fullName = (parentNode->m_name == s_rootName) ? "" : parentNode->m_name + ".";
		fullName += name;

		BaseKBNode* const dupeNode = findNode(fullName);
		if(dupeNode)
		{
			DEBUG_FATAL(s_fatalOnBadKBEntry, ("Duplicate node name %s found in Row %d in table %s", fullName.c_str(), i+3, tableName.c_str()));
			return;
		}

		BaseKBNode* newNode = NULL;

		if(type == s_pageType)
		{
			if(data.empty())
			{
				DEBUG_FATAL(s_fatalOnBadKBEntry, ("No text found for item %s in data field", fullName.c_str()));
				return;
			}
			newNode = new PageKBNode(parentNode, fullName, StringId(data));
		}
		else if(type == s_stringType)
		{
			if(data.empty())
			{
				DEBUG_FATAL(s_fatalOnBadKBEntry, ("No text found for item %s in data field", fullName.c_str()));
				return;
			}
			newNode = new StringKBNode(parentNode, fullName, StringId(data));
		}
		else if(type == s_imageType)
		{
			if(data.empty())
			{
				DEBUG_FATAL(s_fatalOnBadKBEntry, ("No image name found for item %s in data field", fullName.c_str()));
				return;
			}
			newNode = new ImageKBNode(parentNode, fullName, data);
		}
		else if(type == s_commandButtonType)
		{
			if(data.empty())
			{
				DEBUG_FATAL(s_fatalOnBadKBEntry, ("No button name found for item %s in data field", fullName.c_str()));
				return;
			}
			if(data2.empty())
			{
				DEBUG_FATAL(s_fatalOnBadKBEntry, ("No command name found for item %s in data2 field", fullName.c_str()));
				return;
			}
			newNode = new CommandButtonKBNode(parentNode, fullName, data, data2);
		}
		else if(type == s_linkButtonType)
		{
			if(data.empty())
			{
				DEBUG_FATAL(s_fatalOnBadKBEntry, ("No button name found for item %s in data field", fullName.c_str()));
				return;
			}
			if(data2.empty())
			{
				DEBUG_FATAL(s_fatalOnBadKBEntry, ("No link found for item %s in data2 field", fullName.c_str()));
				return;
			}
			newNode = new LinkButtonKBNode(parentNode, fullName, data, data2);
		}
		else if(type == s_scriptMessageButtonType)
		{
			if(data.empty())
			{
				DEBUG_FATAL(s_fatalOnBadKBEntry, ("No button name found for item %s in data field", fullName.c_str()));
				return;
			}
			if(data2.empty())
			{
				DEBUG_FATAL(s_fatalOnBadKBEntry, ("No message name found for item %s in data2 field", fullName.c_str()));
				return;
			}
			newNode = new ScriptMessageButtonKBNode(parentNode, fullName, data, data2);
		}
		else
		{
			DEBUG_FATAL(s_fatalOnBadKBEntry, ("Row %d in table %s has unknown type %s", i+3, tableName.c_str(), type.c_str()));
			return;
		}

		parentNode->m_children.push_back(newNode);
		s_namePageMap.insert(std::make_pair(newNode->m_name, newNode));
	}
}

//----------------------------------------------------------------------

CuiKnowledgeBaseManager::BaseKBNode* CuiKnowledgeBaseManager::findNode(const std::string& name)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	if(name.empty())
	{
		DEBUG_WARNING(true, ("Empty page name passed to CuiKnowledgeBaseManager::findNode"));
		return NULL;
	}

	NamePageMap::iterator const it = s_namePageMap.find(name);
	if (it != s_namePageMap.end())
		return (*it).second;

	return NULL;
}

//======================================================================


