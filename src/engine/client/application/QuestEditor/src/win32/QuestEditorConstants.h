// ======================================================================
//
// QuestEditorConstants.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_QuestEditorConstants_H
#define INCLUDED_QuestEditorConstants_H

// ======================================================================

//-- Version defines
#define QUEST_EDITOR_VERSION "QuestEditor Version 2.11"

#define QUEST_EDITOR_CONFIG_VERSION 4.0f
#define QUEST_EDITOR_MIN_CONFIG_VERSION QUEST_EDITOR_CONFIG_VERSION

//-- Configuration constants
char const * const cs_questEditorSection = "QuestEditor";
char const * const cs_defaultQuestDirectory = "defaultQuestDirectory";
char const * const cs_defaultTaskDirectory = "defaultTaskDirectory";
char const * const cs_defaultListDirectory = "defaultListDirectory";
char const * const cs_defaultStfDirectory = "defaultStfDirectory";
char const * const cs_stringPrefix = "stringPrefix";
char const * const cs_defaultBranch = "defaultBranch";
char const * const cs_toolDirectory = "toolDirectory";

//-- File constants
char const * const cs_questEditorConfigFile = "../../exe/win32/QuestEditorConfig.xml";
char const * const cs_unknownIcon = "../../data/internal/sys.client/built/questeditor/image/component/unknown.png";
char const * const cs_taskOnFailIcon = "../../data/internal/sys.client/built/questeditor/image/task/taskonfail.png";
char const * const cs_chanceToActivateIcon = "../../data/internal/sys.client/built/questeditor/image/task/chancetoactivate.png";

//-- XML tag and attribute name constants
char const * const cs_AllowSpaces = "allowSpaces";
char const * const cs_AvailableId = "availableId";
char const * const cs_AddEmptyString = "addEmptyString";
char const * const cs_Column = "column";
char const * const cs_ColumnName = "columnName";
char const * const cs_ColumnType = "columnType";
char const * const cs_Component = "component";
char const * const cs_Components = "components";
char const * const cs_Data = "data";
char const * const cs_DataTable = "datatable";
char const * const cs_DataTableColumn = "datatableColumn";
char const * const cs_DataTableQuest = "datatableQuest";
char const * const cs_DefaultDirectory = "defaultDirectory";
char const * const cs_DefaultExtension = "defaultExtension";
char const * const cs_DefaultName = "defaultName";
char const * const cs_DefaultTask = "defaultTask";
char const * const cs_DefaultValue = "defaultValue";
char const * const cs_DisplayName = "displayName";
char const * const cs_DontAddEmptyString = "dontAddEmptyString";
char const * const cs_DontSave = "dontSave";
char const * const cs_Editor = "editor";
char const * const cs_Editors = "editors";
char const * const cs_File = "file";
char const * const cs_Files = "files";
char const * const cs_Filter = "filter";
char const * const cs_Folder = "folder";
char const * const cs_Icon = "icon";
char const * const cs_Id = "id";
char const * const cs_Info = "info";
char const * const cs_Input = "input";
char const * const cs_InputName = "inputName";
char const * const cs_Item = "item";
char const * const cs_List = "list";
char const * const cs_MakeAttribute = "makeAttribute";
char const * const cs_MaxValue = "maxValue";
char const * const cs_MinValue = "minValue";
char const * const cs_Name = "name";
char const * const cs_NoOutput = "noOutput";
char const * const cs_Output = "output";
char const * const cs_Quest = "quest";
char const * const cs_QuestEditor = "QuestEditor";
char const * const cs_Script = "script";
char const * const cs_Task = "task";
char const * const cs_TaskOnFail = "taskOnFail";
char const * const cs_Tasks = "tasks";
char const * const cs_Type = "type";
char const * const cs_Types = "types";
char const * const cs_Value = "value";
char const * const cs_Version = "version";

//-- task and list constants
int const cs_MaxTaskRows = 16;
int const cs_MaxTaskColumns = 128;
int const cs_MaxListRows = 1;
int const cs_MaxListColumns = 128;

//-- quest/task data special purpose names
char const * const cs_journalEntryTitle = "journalEntryTitle";
char const * const cs_none = "none";
char const * const cs_taskName = "taskName";
char const * const cs_chanceToActivate = "chanceToActivate";
char const * const cs_questControlOnTaskCompletion = "questControlOnTaskCompletion";
char const * const cs_questControlOnTaskFailure = "questControlOnTaskFailure";

//-- macros
#define NODENAME(a) (a).nodeName()
#define NODETYPE(a) (a).toElement().attribute(cs_Type, NODENAME(a)).ascii()
#define NODEINFO(a) (a).toElement().attribute(cs_Name, NODETYPE(a)).ascii()

// ======================================================================

#endif // INCLUDED_QuestEditorConstants_H
