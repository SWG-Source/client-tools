// ======================================================================
//
// QuestEditorConfig.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_QuestEditorConfig_H
#define INCLUDED_QuestEditorConfig_H

// ----------------------------------------------------------------------

class QDomDocument;
class QDomElement;
class QDomNode;
class QDomNodeList;
class QString;

// ----------------------------------------------------------------------

class QuestEditorConfig
{
public:

	enum QuestInputType
	{
		QIT_Invalid = -1,
		QIT_None,

		QIT_Bool,
		QIT_Compound,
		QIT_DataTableColumn,
		QIT_Date,
		QIT_DateTime,
		QIT_Double,
		QIT_Enum,
		QIT_File,
		QIT_Int,
		QIT_List,
		QIT_LocalizedString,
		QIT_RelativeFile,
		QIT_ReadOnlyString,
		QIT_String,
		QIT_StringList,
		QIT_Time
	};

	enum QuestOutputType
	{
		QOT_Invalid = -1,
		QOT_None,

		QOT_Bool,
		QOT_Completion,
		QOT_Exclusion,
		QOT_Failure,
		QOT_LocalizedString,
		QOT_Prerequisite,
		QOT_Script,
		QOT_String
	};

	enum QuestValidatorType
	{
		QVT_Invalid = -1,
		QVT_None
	};

	static bool install();
	static void remove();

	static QDomNode getEditorNode(char const * const editorName);
	static QDomNode getEditorForType(char const * const name);

	static QDomNodeList getNodeList(char const * const parent, char const * const child);
	static QDomNodeList getComponents();
	static QDomNodeList getEditors();

	static QString getFileAttribute(QDomElement const &element, char const * const name, bool treeFileSearch = true);
	static QString getTaskAttribute(QString const &taskType, QString const &attributeName, bool isFile);
	static QString getTaskValue(QDomNode task, char const * const name, char const * const defaultValue = "");

	static QuestInputType getQITFromString(QString const & qitString);
	static QuestOutputType getQOTFromString(QString const & qotString);

	static QDomNode getEditorDefaultTaskNode();
	static QDomNode getEditorTaskNode(QString const & taskType);
	static QDomNode getEditorListNode();
	static QDomNode getEditorTasksNode();

private:
	static QDomDocument * m_config;

private: //-- disabled
	QuestEditorConfig();
};

// ======================================================================

#endif
