//======================================================================
//
// CuiKnowledgeBaseManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiKnowledgeBaseManager_H
#define INCLUDED_CuiKnowledgeBaseManager_H

//======================================================================

#include "StringId.h"

#include <vector>

class DataTable;

//----------------------------------------------------------------------

class CuiKnowledgeBaseManager
{
public:

	static const std::string s_pageType;
	static const std::string s_stringType;
	static const std::string s_imageType;
	static const std::string s_commandButtonType;
	static const std::string s_linkButtonType;
	static const std::string s_scriptMessageButtonType;

	struct BaseKBNode
	{
		BaseKBNode* m_parent;
		std::string m_name;
		std::vector<BaseKBNode*> m_children;
		std::string m_type;

		BaseKBNode(const std::string& type, BaseKBNode* parent, const std::string& name);
		void clear();
		virtual ~BaseKBNode();
	};

	struct StringKBNode : public BaseKBNode
	{
		StringId m_string;

		StringKBNode(BaseKBNode* parent, const std::string& name, const StringId& s);
	};

	struct PageKBNode : public BaseKBNode
	{
		StringId m_string;

		PageKBNode (BaseKBNode* parent, const std::string& name, const StringId& s);
	};

	struct ImageKBNode : public BaseKBNode
	{
		std::string m_path;

		ImageKBNode(BaseKBNode* parent, const std::string& name, const std::string& path);
	};

	struct CommandButtonKBNode : public BaseKBNode
	{
		std::string m_string;
		std::string m_command;

		CommandButtonKBNode(BaseKBNode* parent, const std::string& name, const std::string& str, const std::string& command);
	};

	struct LinkButtonKBNode : public BaseKBNode
	{
		std::string m_string;
		std::string m_link;

		LinkButtonKBNode(BaseKBNode* parent, const std::string& name, const std::string& str, const std::string& link);
	};

	struct ScriptMessageButtonKBNode : public BaseKBNode
	{
		std::string m_string;
		std::string m_messageName;

		ScriptMessageButtonKBNode(BaseKBNode* parent, const std::string& name, const std::string& str, const std::string& link);
	};

public:

	static void          install ();
	static void          remove  ();

	static BaseKBNode*   getRoot();
	static BaseKBNode*   findNode(const std::string& name);
	static void          reloadData();

private:
	static void          clearData();
	static void          loadTables();
	static void          loadTable(const DataTable& table, const std::string& tableName);
	static PageKBNode*   m_kbRoot;
};

//======================================================================

inline CuiKnowledgeBaseManager::BaseKBNode* CuiKnowledgeBaseManager::getRoot()
{
	return m_kbRoot;
}

//======================================================================

#endif
