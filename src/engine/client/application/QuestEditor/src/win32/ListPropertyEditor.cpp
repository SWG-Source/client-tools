// ======================================================================
//
// ListPropertyEditor.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "ListPropertyEditor.h"

// ----------------------------------------------------------------------

#include "QuestEditorConstants.h"
#include "PropertyList/PropertyItem.h"

// ----------------------------------------------------------------------

#include "ListPropertyEditor.moc"

// ----------------------------------------------------------------------

ListPropertyEditor::ListPropertyEditor(QWidget * listPropertyEditorParent)
: ElementPropertyEditor(listPropertyEditorParent)
{
}

// ----------------------------------------------------------------------

ListPropertyEditor::~ListPropertyEditor()
{
}

// ----------------------------------------------------------------------

void ListPropertyEditor::setList(QDomNode list)
{
	setElement(list.toElement());
}

// ----------------------------------------------------------------------

void ListPropertyEditor::clearList()
{
	clearElement();
}

// ----------------------------------------------------------------------

void ListPropertyEditor::setupProperties()
{
	PropertyItem * item = 0;

	QDomNode listNode = QuestEditorConfig::getEditorListNode();

	QDomNode configDataNode = listNode.firstChild();

	while(!configDataNode.isNull())
	{
		if(configDataNode.nodeName() == cs_Data)
			setupInput(item, configDataNode);

		configDataNode = configDataNode.nextSibling();
	}

	PropertyList::setupProperties();
}

// ----------------------------------------------------------------------

void ListPropertyEditor::setSelectedField(QString const & field)
{
	if (field == cs_none)
	{
		return;
	}

	QDomElement listElement = QuestEditorConfig::getEditorListNode().toElement();

	QDomNodeList dataNodeList = listElement.elementsByTagName(cs_Data);

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

// ======================================================================
