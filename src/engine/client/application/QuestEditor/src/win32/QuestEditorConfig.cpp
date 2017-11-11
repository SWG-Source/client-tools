// ======================================================================
//
// QuestEditorConfig.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "QuestEditorConfig.h"
#include "QuestEditorConstants.h"

// ----------------------------------------------------------------------

#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Os.h"

// ----------------------------------------------------------------------

#include <qdom.h>
#include <qfile.h>
#include <qmessagebox.h>

// ----------------------------------------------------------------------

QDomDocument * QuestEditorConfig::m_config = 0;

// ----------------------------------------------------------------------

bool QuestEditorConfig::install()
{
	//-- read the XML file and create the DOM tree
	QFile opmlFile(cs_questEditorConfigFile);

	if (!opmlFile.open(IO_ReadOnly))
	{
		IGNORE_RETURN(QMessageBox::critical(0, QObject::tr("Critical Error"),
			QObject::tr("Cannot open file [%1]").arg(cs_questEditorConfigFile)));
		return false;
	}

	m_config = new QDomDocument();

	if (!m_config->setContent(&opmlFile))
	{
		IGNORE_RETURN(QMessageBox::critical(0, QObject::tr("Critical Error"),
			QObject::tr("Parsing error for file [%1]").arg(cs_questEditorConfigFile)));
		return false;
	}

	float const configVersion = m_config->documentElement().attribute(cs_Version, "0").toFloat();

	if (configVersion < QUEST_EDITOR_MIN_CONFIG_VERSION)
	{
		IGNORE_RETURN(QMessageBox::critical(0, QObject::tr("Critical Error"),
			QObject::tr(
			"The config file [%1] is the wrong version!\n"
			"Sync the config file from Perforce or get a newer bootleg.\n"
			"The minimum version supported is [%2].\n"
			"The version of the config file is [%3].\n")
			.arg(cs_questEditorConfigFile)
			.arg(QUEST_EDITOR_MIN_CONFIG_VERSION)
			.arg(configVersion)
			));
		return false;
	}

	if (configVersion > QUEST_EDITOR_CONFIG_VERSION)
	{
		IGNORE_RETURN(QMessageBox::critical(0, QObject::tr("Critical Error"),
			QObject::tr(
			"The config file [%1] is the wrong version!\n"
			"Get a new version of the QuestEditor application from the all branch in Perforce.\n"
			"The version supported is [%2].\n"
			"The version of the config file is [%3].\n")
			.arg(cs_questEditorConfigFile)
			.arg(QUEST_EDITOR_CONFIG_VERSION)
			.arg(configVersion)
			));
		return false;
	}

	return true; //-- opmlFile closed by destructor
}

// ----------------------------------------------------------------------

void QuestEditorConfig::remove()
{
	delete m_config;
	m_config = 0;
}

// ----------------------------------------------------------------------

QDomNode QuestEditorConfig::getEditorNode(char const * const editorName)
{
	if (!m_config)
	{
		WARNING(true, ("QuestEditorConfig::getEditorNode() - configuration file not set."));
		return QDomNode();
	}

	QDomNodeList editors = getEditors();

	for (int i=0; i < static_cast<int>(editors.count()); ++i)
		if (editors.item(i).toElement().attribute(cs_Name) == editorName)
			return editors.item(i);

	return QDomNode();
}

// ----------------------------------------------------------------------

QDomNodeList QuestEditorConfig::getComponents()
{
	return getNodeList(cs_Components, cs_Component);
}

// ----------------------------------------------------------------------

QDomNodeList QuestEditorConfig::getEditors()
{
	return getNodeList(cs_Editors, cs_Editor);
}

// ----------------------------------------------------------------------

QDomNodeList QuestEditorConfig::getNodeList(char const * const parent, char const * const child)
{
	if (!m_config)
	{
		WARNING(true, ("QuestEditorConfig::getNodeList() - configuration file not set."));
		return QDomNodeList();
	}

	QDomNode node = m_config->documentElement().firstChild();

	while(!node.isNull() && node.nodeName() != parent)
		node = node.nextSibling();

	if(!node.isNull())
		return node.toElement().elementsByTagName(child);

	return QDomNodeList();
}

// ----------------------------------------------------------------------

QDomNode QuestEditorConfig::getEditorForType(char const * const name)
{
	NOT_NULL(name);

	//-- Given a type name, find the editor associated with that type in the type definitions
	QDomNodeList types = getNodeList(cs_Types, cs_Type);

	for (int i=0; i < static_cast<int>(types.count()); ++i)
		if (types.item(i).toElement().attribute(cs_Name) == name)
			return types.item(i);

	WARNING(true, ("QuestEditorConfig::getEditorForType() - No editor defined for type [%s]\n", name));

	return QDomNode();
}

// ----------------------------------------------------------------------

QString QuestEditorConfig::getFileAttribute(QDomElement const &element, char const * const name, bool treeFileSearch)
{
	QString filename = element.attribute(name);

	//-- element may have a value that is defined in the config file
	//--  where the notation is: ?section:key
	if (filename[0] == '?')
	{
		int split = filename.find(':');

		FATAL(split < 0, ("Bad config redirection (missing ':')."));

		QString section = filename.mid(1, split - 1); //lint !e732
		QString key = filename.mid(split + 1); //lint !e732

		DEBUG_REPORT_LOG(true, ("QuestEditorConfig::getFileAttribute() - ?[%s]:[%s].\n",
			section.latin1(), key.latin1()));

		filename = ConfigFile::getKeyString(section, key, "");
	}

	//-- the value may optionally have a relative file path that can
	//--  get converted using a tree file search
	if (treeFileSearch)
	{
		char pathName[Os::MAX_PATH_LENGTH] = "";

		IGNORE_RETURN(TreeFile::getPathName(filename.latin1(), pathName, sizeof(pathName)));

		filename = pathName;
	}

	DEBUG_REPORT_LOG(false, ("QuestEditorConfig::getFileAttribute() - [%s].\n", filename.latin1()));

	return filename;
}

// ----------------------------------------------------------------------

QString QuestEditorConfig::getTaskAttribute(QString const &taskType, QString const &attributeName, bool isFile)
{
	QDomElement editor = getEditorNode(cs_QuestEditor).toElement();

	QDomNodeList tasks = editor.elementsByTagName(cs_Task);

	for (int i=0; i<static_cast<int>(tasks.count()); ++i)
		if (tasks.item(i).toElement().attribute(cs_Type) == taskType)
		{
			QString value;
			if (isFile)
				value = getFileAttribute(tasks.item(i).toElement(), attributeName, true);
			else
				value = tasks.item(i).toElement().attribute(attributeName);

			return value;
		}

	return "";
}

// ----------------------------------------------------------------------

QuestEditorConfig::QuestInputType QuestEditorConfig::getQITFromString(QString const & qitString)
{
	if (qitString == "QIT_None")
		return QIT_None;

	else if (qitString == "QIT_Bool")
		return QIT_Bool;
	else if (qitString == "QIT_Compound")
		return QIT_Compound;
	else if (qitString == "QIT_DataTableColumn")
		return QIT_DataTableColumn;
	else if (qitString == "QIT_Date")
		return QIT_Date;
	else if (qitString == "QIT_DateTime")
		return QIT_DateTime;
	else if (qitString == "QIT_Double")
		return QIT_Double;
	else if (qitString == "QIT_Enum")
		return QIT_Enum;
	else if (qitString == "QIT_File")
		return QIT_File;
	else if (qitString == "QIT_Int")
		return QIT_Int;
	else if (qitString == "QIT_List")
		return QIT_List;
	else if (qitString == "QIT_LocalizedString")
		return QIT_LocalizedString;
	else if (qitString == "QIT_ReadOnlyString")
		return QIT_ReadOnlyString;
	else if (qitString == "QIT_RelativeFile")
		return QIT_RelativeFile;
	else if (qitString == "QIT_String")
		return QIT_String;
	else if (qitString == "QIT_StringList")
		return QIT_StringList;
	else if (qitString == "QIT_Time")
		return QIT_Time;

	return QIT_Invalid;
}

// ----------------------------------------------------------------------

QuestEditorConfig::QuestOutputType QuestEditorConfig::getQOTFromString(QString const & qotString)
{
	if (qotString == "QOT_None")
		return QOT_None;

	else if (qotString == "QOT_Prerequisite")
		return QOT_Prerequisite;
	else if (qotString == "QOT_Exclusion")
		return QOT_Exclusion;
	else if (qotString == "QOT_Script")
		return QOT_Script;
	else if (qotString == "QOT_String")
		return QOT_String;
	else if (qotString == "QOT_LocalizedString")
		return QOT_LocalizedString;
	else if (qotString == "QOT_Completion")
		return QOT_Completion;
	else if (qotString == "QOT_Failure")
		return QOT_Failure;
	else if (qotString == "QOT_Bool")
		return QOT_Bool;

	return QOT_Invalid;
}

// ----------------------------------------------------------------------

QDomNode QuestEditorConfig::getEditorDefaultTaskNode()
{
	QDomNode editor = getEditorNode(cs_QuestEditor);

	QDomNode defaultTask = editor.toElement().firstChild();
	
	while(!defaultTask.isNull() && defaultTask.nodeName() != cs_DefaultTask)
		defaultTask = defaultTask.nextSibling();

	FATAL(defaultTask.isNull(), ("Missing defaultTask node in '%s'!", cs_questEditorConfigFile));

	return defaultTask;
}

// ----------------------------------------------------------------------

QDomNode QuestEditorConfig::getEditorTaskNode(QString const & taskType)
{
	QDomNode editor = getEditorNode(cs_QuestEditor);

	QDomNode tasks = editor.firstChild();
	
	while(!tasks.isNull() && tasks.nodeName() != cs_Tasks)
		tasks = tasks.nextSibling();

	FATAL(tasks.isNull(), ("Missing tasks node in '%s'!", cs_questEditorConfigFile));

	QDomNode task = tasks.firstChild();

	while(!task.isNull())
	{
		if (task.nodeName() == cs_Task && task.toElement().attribute(cs_Type) == taskType)
			break;

		task = task.nextSibling();
	}

	FATAL(task.isNull(), ("Missing task type [%s] in '%s'!", taskType.ascii(), cs_questEditorConfigFile));

	return task;
}

// ----------------------------------------------------------------------

QDomNode QuestEditorConfig::getEditorListNode()
{
	QDomNode editor = getEditorNode(cs_QuestEditor);

	QDomNode list = editor.firstChild();
	
	while(!list.isNull() && list.nodeName() != cs_List)
		list = list.nextSibling();

	FATAL(list.isNull(), ("Missing list node in '%s'!", cs_questEditorConfigFile));

	return list;
}

// ----------------------------------------------------------------------

QDomNode QuestEditorConfig::getEditorTasksNode()
{
	QDomNode editor = getEditorNode(cs_QuestEditor);

	QDomNode tasks = editor.firstChild();
	
	while(!tasks.isNull() && tasks.nodeName() != cs_Tasks)
		tasks = tasks.nextSibling();

	FATAL(tasks.isNull(), ("Missing tasks node in '%s'!", cs_questEditorConfigFile));

	return tasks;
}

// ----------------------------------------------------------------------

QString QuestEditorConfig::getTaskValue(QDomNode task, char const * const name, char const * const defaultValue)
{
	for (QDomNode data = task.firstChild(); !data.isNull(); data = data.nextSibling())
		if (data.nodeName() == cs_Data && data.toElement().attribute(cs_Name) == name)
			return data.toElement().attribute(cs_Value);

	return defaultValue;
}

// ======================================================================
