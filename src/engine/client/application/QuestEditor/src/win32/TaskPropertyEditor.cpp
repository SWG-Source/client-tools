// ======================================================================
//
// TaskPropertyEditor.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "TaskPropertyEditor.h"

// ----------------------------------------------------------------------

#include "QuestEditorConstants.h"
#include "PropertyList/PropertyItem.h"

// ----------------------------------------------------------------------

#include "TaskPropertyEditor.moc"

// ----------------------------------------------------------------------

TaskPropertyEditor::TaskPropertyEditor(QWidget * taskPropertyEditorParent)
: ElementPropertyEditor(taskPropertyEditorParent)
{
}

// ----------------------------------------------------------------------

TaskPropertyEditor::~TaskPropertyEditor()
{
}

// ----------------------------------------------------------------------

void TaskPropertyEditor::setTask(QDomNode task)
{
	setElement(task.toElement());
}

// ----------------------------------------------------------------------

void TaskPropertyEditor::clearTask()
{
	clearElement();
}

// ----------------------------------------------------------------------

void TaskPropertyEditor::setupProperties()
{
	//-- NOTE: Assume new tasks are already initialized with defaults

	//-- read the task defaults first
	QDomNode defaultTask = QuestEditorConfig::getEditorDefaultTaskNode().toElement();

	//-- for each data node, create a new PropertyItem based on the input type
	QDomNode configDataNode = defaultTask.firstChild();

	PropertyItem * item = 0;

	while(!configDataNode.isNull())
	{
		if(configDataNode.nodeName() == cs_Data)
			setupInput(item, configDataNode);

		configDataNode = configDataNode.nextSibling();
	}
	
	//-- based on the type of task, add controls specific to it
	QString taskType = getElement().attribute(cs_Type);

	QDomNode taskNode = QuestEditorConfig::getEditorTaskNode(taskType);

	configDataNode = taskNode.firstChild();

	while(!configDataNode.isNull())
	{
		if(configDataNode.nodeName() == cs_Data)
			setupInput(item, configDataNode);

		configDataNode = configDataNode.nextSibling();
	}

	//-- At this point, all the data defined in the configuration file for the task
	//--   has an associated control.

	//-- TTODO: run through every property in the task and make sure a control exists for each one
	//--   otherwise generate a warning. (task is out of sync with config data)

	PropertyList::setupProperties();
}

// ----------------------------------------------------------------------

void TaskPropertyEditor::setSelectedField(QString const & field)
{
	if (field == cs_none)
	{
		return;
	}

	{
		QDomElement defaultTask = QuestEditorConfig::getEditorDefaultTaskNode().toElement();

		QDomNodeList dataNodeList = defaultTask.elementsByTagName(cs_Data);

		for (uint i = 0; i < dataNodeList.count(); ++i)
		{
			QDomElement configDataElement = dataNodeList.item(i).toElement();

			if (!configDataElement.isNull() && configDataElement.attribute(cs_Name) == field)
			{
				openParentCompoundInput(configDataElement);
				setCurrentProperty(field, true);
				return;
			}
		}
	}

	{
		QString taskType = getElement().attribute(cs_Type);

		QDomElement taskElement = QuestEditorConfig::getEditorTaskNode(taskType).toElement();

		QDomNodeList dataNodeList = taskElement.elementsByTagName(cs_Data);

		for (uint i = 0; i < dataNodeList.count(); ++i)
		{
			QDomElement configDataElement = dataNodeList.item(i).toElement();

			if (!configDataElement.isNull() && configDataElement.attribute(cs_Name) == field)
			{
				openParentCompoundInput(configDataElement);
				setCurrentProperty(field, true);
				return;
			}
		}
	}
}

// ======================================================================
