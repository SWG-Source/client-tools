// ======================================================================
//
// TreeItem.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "QuestEditorConfig.h"
#include "QuestEditorConstants.h"
#include "TreeItem.h"

// ----------------------------------------------------------------------

#include <qimage.h>

// ----------------------------------------------------------------------

TreeItem::TreeItem(QListView * parentTreeItem, QListViewItem * afterTreeItem)
: QListViewItem(parentTreeItem, afterTreeItem)
, m_node()
{
}

// ----------------------------------------------------------------------

TreeItem::TreeItem(QListViewItem * parentTreeItem, QListViewItem * afterTreeItem)
: QListViewItem(parentTreeItem, afterTreeItem)
, m_node()
{
}

// ----------------------------------------------------------------------

TreeItem::~TreeItem()
{
}

// ----------------------------------------------------------------------

void TreeItem::setNode(QDomNode const & node)
{
	m_node = node;

	refresh();
}

// ----------------------------------------------------------------------

void TreeItem::refresh()
{
	QString taskType = m_node.toElement().attribute(cs_Type);
	QString icon = QuestEditorConfig::getTaskAttribute(taskType, cs_Icon, true);

	bool const taskOnFail = m_node.toElement().attribute(cs_TaskOnFail) == "true";

	QString journalEntryTitle;
	QString taskName;
	bool chanceToActivate = false;
	bool questControlOnTaskCompletion = false;
	bool questControlOnTaskFailure = false;

	bool journalEntryTitleFound = false;
	bool taskNameFound = false;
	bool chanceToActivateFound = false;
	bool questControlOnTaskCompletionFound = false;
	bool questControlOnTaskFailureFound = false;

	for (QDomNode data = m_node.firstChild(); !data.isNull(); data = data.nextSibling())
	{
		if (data.nodeName() != cs_Data)
			continue;

		if (data.toElement().attribute(cs_Name) == cs_journalEntryTitle)
		{
			journalEntryTitle = data.toElement().attribute(cs_Value);
			journalEntryTitleFound = true;
		}

		if (data.toElement().attribute(cs_Name) == cs_taskName)
		{
			taskName = data.toElement().attribute(cs_Value);
			taskNameFound = true;
		}

		if (data.toElement().attribute(cs_Name) == cs_chanceToActivate)
		{
			chanceToActivate = data.toElement().attribute(cs_Value).toInt() > 0;
			chanceToActivateFound = true;
		}

		if (data.toElement().attribute(cs_Name) == cs_questControlOnTaskCompletion)
		{
			questControlOnTaskCompletion = data.toElement().attribute(cs_Value) != "none";
			questControlOnTaskCompletionFound = true;
		}

		if (data.toElement().attribute(cs_Name) == cs_questControlOnTaskFailure)
		{
			questControlOnTaskFailure = data.toElement().attribute(cs_Value) != "none";
			questControlOnTaskFailureFound = true;
		}

		if (journalEntryTitleFound && taskNameFound && chanceToActivateFound
			&& questControlOnTaskCompletionFound && questControlOnTaskFailureFound)
			break;
	}

	QPixmap taskIcon(icon);

	if (taskOnFail)
	{
		QPixmap failureIcon(cs_taskOnFailIcon);
		copyBlt(&taskIcon, 0, 0, &failureIcon);
	}
	else if (chanceToActivate)
	{
		QPixmap chanceToActivateIcon(cs_chanceToActivateIcon);
		copyBlt(&taskIcon, 0, 0, &chanceToActivateIcon);
	}

	setPixmap(0, taskIcon);

	setText(0, journalEntryTitle + " [" + taskType + "] [" + taskName + "]"
		+ QString(questControlOnTaskCompletion || questControlOnTaskFailure ? " *" : ""));
}

// ======================================================================
