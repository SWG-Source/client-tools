// ======================================================================
//
// QuestEditor.cpp
// Copyright 2004, Sony Online Entertainment Inc.
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "ListPropertyEditor.h"
#include "MainWindow.h"
#include "PropertyList/PropertyList.h"
#include "Quest.h"
#include "QuestEditor.h"
#include "QuestEditorConfig.h"
#include "QuestEditorConstants.h"
#include "TaskPropertyEditor.h"
#include "ToolProcess.h"
#include "TreeItem.h"
#include "TreeListView.h"

// ----------------------------------------------------------------------

#include "sharedFile/FileNameUtils.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Os.h"

// ----------------------------------------------------------------------

#include <qbitmap.h>
#include <qcheckbox.h>
#include <qdragobject.h>
#include <qfiledialog.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qheader.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qtabwidget.h>
#include <qtextedit.h>

// ----------------------------------------------------------------------

#include <string>

// ----------------------------------------------------------------------

#include "QuestEditor.moc"

// ----------------------------------------------------------------------

#include "images/questeditor_small.xpm"
#include "images/delete.xpm"

// ----------------------------------------------------------------------

namespace QuestEditorNamespace
{
	void addFieldDataText(QTextEdit * helpText, QDomElement const & parent);
}

using namespace QuestEditorNamespace;

// ----------------------------------------------------------------------

QuestEditor::QuestEditor(QWidget *questWidgetParent, char const * const questWidgetName, WFlags questWidgetFlags)
: BaseQuestWidget(questWidgetParent, questWidgetName, questWidgetFlags)
, m_quest(0)
, m_taskPropertyEditor(0)
, m_listPropertyEditor(0)
, m_filename()
, m_basename()
, m_listTab()
, m_listIff()
, m_taskTab()
, m_taskIff()
, m_stringFile()
, m_dirty(false)
, m_stringTable()
{
	IGNORE_RETURN(m_taskTree->addColumn(tr("Task Tree")));
	m_taskTree->header()->setClickEnabled(false, m_taskTree->header()->count() - 1);
	m_taskTree->header()->setResizeEnabled(false, m_taskTree->header()->count() - 1);
	m_taskTree->setMinimumSize(QSize(0, 0));
	m_taskTree->setResizeMode(QListView::AllColumns);
	m_taskTree->setSorting(-1);
	m_taskTree->setRootIsDecorated(true);
	m_taskTree->setResizeMode(QListView::AllColumns);
	m_taskTree->header()->setClickEnabled(false);
	m_taskTree->header()->setResizeEnabled(false);

	IGNORE_RETURN(connect(m_taskTree, SIGNAL(contextMenuRequested(QListViewItem *, const QPoint &, int)),
		this, SLOT(rmbClicked(QListViewItem *, const QPoint&))));
	IGNORE_RETURN(connect(m_taskTree, SIGNAL(nodeDropped(const QString&, const QString&)),
		this, SLOT(nodeDropped(const QString&, const QString&))));
	IGNORE_RETURN(connect(m_taskTree, SIGNAL(selectionChanged(QListViewItem *)),
		this, SLOT(slotCurrentChanged(QListViewItem *))));

	m_listPropertyEditor = new ListPropertyEditor(this);
	m_questTabWidget->insertTab(m_listPropertyEditor, "QuestList", 0);
	IGNORE_RETURN(connect(m_listPropertyEditor, SIGNAL(dirty()), this, SLOT(setDirty())));

	m_taskPropertyEditor = new TaskPropertyEditor(this);
	m_questTabWidget->insertTab(m_taskPropertyEditor, "QuestTask", 0);
	IGNORE_RETURN(connect(m_taskPropertyEditor, SIGNAL(dirty()), this, SLOT(setDirty())));

	m_questTabWidget->setCurrentPage(0);
	BaseQuestWidget::setAcceptDrops(true);
}

// ----------------------------------------------------------------------

QuestEditor::~QuestEditor()
{
	delete m_quest;
	m_quest = 0;

	m_taskPropertyEditor = 0;
	m_listPropertyEditor = 0;
}

// ----------------------------------------------------------------------

void QuestEditor::load(char const * const filename)
{
	m_quest = new Quest;
	m_quest->loadQuest(filename);

	if (!filename)
	{
		//-- creating a new empty quest
		QDomElement tasks = m_quest->createElement(cs_Tasks);
		QDomElement list = m_quest->createElement(cs_List);

		//-- store available id so that each task can be uniquely identified
		tasks.setAttribute(cs_AvailableId, 0);

		IGNORE_RETURN(m_quest->documentElement().appendChild(tasks));
		IGNORE_RETURN(m_quest->documentElement().appendChild(list));

		initializeNewList(list);
	}
	else
	{
		//-- loading an existing quest
		setFilename(filename);
	}

	//-- task
	{
		QDomNode tasks = m_quest->getTasks();

		FATAL(tasks.isNull(), ("No quest task node found in file!"));

		buildTaskTree(tasks.firstChild(), 0);
	}

	//-- list
	{
		QDomNode list = m_quest->getList();

		FATAL(list.isNull(), ("No quest list node found in file!"));

		m_listPropertyEditor->setList(list);
	}

	updateCaption();
	setIcon(QPixmap(questeditor_small_xpm));
}

// ----------------------------------------------------------------------

void QuestEditor::buildTaskTree(QDomNode node, TreeItem * parent)
{
	for (; !node.isNull(); node = node.nextSibling())
		if (node.nodeName() == cs_Task)
		{
			QDomElement task = node.toElement();
			TreeItem * current = 0;

			QString taskType = task.attribute(cs_Type);

			//-- if this task was saved without an id, add one
			if (!task.hasAttribute(cs_Id))
			{
				DEBUG_REPORT_LOG(true,
					("QuestEditor::buildTaskTree() - [%s] not found for task [%s].\n",
					cs_Id, taskType.latin1()));
				task.setAttribute(cs_Id, m_quest->getUniqueTaskId());
			}

			if (task.hasAttribute(cs_Name))
			{
				DEBUG_REPORT_LOG(true, ("QuestEditor::buildTaskTree() - Task has name attribute - removing it.\n"));
				task.removeAttribute(cs_Name);
				setDirty();
			}

			if (parent)
				current = new TreeItem(parent, 0);
			else
				current = new TreeItem(m_taskTree, 0);

			current->setNode(task);
			current->setOpen(true);

			buildTaskTree(task.firstChild(), current);
		}
}

// ----------------------------------------------------------------------

void QuestEditor::saveAs()
{
	char const * const defaultQuestDirectory = ConfigFile::getKeyString(cs_questEditorSection, cs_defaultQuestDirectory, "");

	char absolutePath[1024];

	bool result = Os::getAbsolutePath(defaultQuestDirectory, absolutePath, 1024);
	UNREF(result);

	QString filename = QFileDialog::getSaveFileName(absolutePath, "Quest (*.qst)", this,
		"Save Quest", "Choose a filename to Save");
	
	if (!filename.isEmpty())
	{
		// Automatically add the correct extension
		std::string path = FileNameUtils::get(filename.latin1(), FileNameUtils::drive | FileNameUtils::directory | FileNameUtils::fileName);
		path += ".qst";
		
		if (QFile::exists(path.c_str()))
		{
			int result = QMessageBox::warning(this, "File exists!", "Do you want to overwrite?",
				QMessageBox::Yes, QMessageBox::No);

			if (result == QMessageBox::No)
				return;
		}

		setFilename(path.c_str());
		save();
	}
}

// ----------------------------------------------------------------------

void QuestEditor::save()
{
	//-- filename already set?
	if (m_filename.isEmpty())
	{
		saveAs();
		return;
	}

	//-- make sure all files are writable
	if (!filesWritable())
	{
		int result = QMessageBox::warning(this, "All files not writable!", "Do you want to attempt to save anyways?",
			QMessageBox::Yes, QMessageBox::No);

		if (result == QMessageBox::No)
			return;
	}

	if (!m_quest->saveQuest(m_filename))
		return;

	m_dirty = false;

	exportDataTables();
	compile();
	check();

	updateCaption();
}

// ----------------------------------------------------------------------

void QuestEditor::rmbClicked(QListViewItem *i, const QPoint& pos)
{
	QPopupMenu menu(this);
	QPopupMenu taskMenu(this);
	int idDelete = 0;

	//-- if the tree is not empty, i must be a list item
	if(m_taskTree->firstChild() && !i)
		return;

	//-- If task count is greater than or equal to max, do not add menu item
	if (m_quest->elementsByTagName(cs_Task).count() < cs_MaxTaskRows)
	{
		//-- get the editor info for quests
		{
			QDomElement editor = QuestEditorConfig::getEditorNode(className()).toElement();

			QDomNodeList tasks = editor.elementsByTagName(cs_Task);

			for (uint i=0; i<tasks.count(); ++i)
			{
				QDomElement task = tasks.item(i).toElement();
				QString icon = QuestEditorConfig::getFileAttribute(task, cs_Icon);
				int id = taskMenu.insertItem(QIconSet(QPixmap(icon)), task.attribute(cs_Type));

				taskMenu.setWhatsThis(id, QString(cs_whatsThisTextFormat)
					.arg(task.attribute(cs_Type))
					.arg(task.attribute(cs_Info, cs_defaultInfoText)));
			}
		}

		IGNORE_RETURN(menu.insertItem(QIconSet(QPixmap(cs_unknownIcon)),tr("Add New &Task"), &taskMenu));
	}
	else
		IGNORE_RETURN(menu.insertItem(tr("No More Tasks")));

	IGNORE_RETURN(menu.insertSeparator());

	idDelete = menu.insertItem(QIconSet(QPixmap(delete_xpm)),tr("&Delete"));

	int id = menu.exec(pos);

	QString taskType = taskMenu.text(id);

	if (id == idDelete)
	{
		TreeItem * item = dynamic_cast<TreeItem *>(i);

		if (item)
		{
			QDomNode parent = item->getNode().parentNode();
			item->getNode().clear();

			if (!parent.isNull())
				IGNORE_RETURN(parent.removeChild(item->getNode()));
			else
				IGNORE_RETURN(m_quest->removeChild(item->getNode()));
			delete i;

			setDirty();
		}

		return;
	}

	//-- was add new task selected?
	if(!taskType.isNull())
	{
		TreeItem * item;

		QDomElement task = m_quest->createElement(cs_Task);

		task.setAttribute(cs_Type, taskType);
		
		if (i)
		{
			item = new TreeItem(i, 0);

			TreeItem * t = (TreeItem *)i;

			t->getNode().appendChild(task);

			i->setOpen(true);
		}
		else
		{
			item = new TreeItem(m_taskTree, 0);

			QDomNodeList tasks = m_quest->elementsByTagName(cs_Tasks);

			tasks.item(0).appendChild(task);
		}

		initializeNewTask(task);

		item->setNode(task);

		setDirty();
	}
}

// ----------------------------------------------------------------------

void QuestEditor::exportDataTables()
{
	if (m_dirty || m_filename.isEmpty())
	{
		QMessageBox::warning(this, "Export Warning", "You must save the quest before exporting!", "Ok");
		return;
	}

	m_stringTable.clear();

	exportListDataTable(m_listTab);
	exportTaskDataTable(m_taskTab);

	//-- write localized string file
	emit consoleOutput(QString("Exporting stf file [%1]").arg(m_stringFile));
	m_stringTable.save(m_stringFile);
}

// ----------------------------------------------------------------------

QString QuestEditor::getOutputValue(QDomElement const &config, QDomElement const &instance, bool first, std::map<int, QString> &idRowMap)
{
	//-- config - data input/output defined in the configuration file
	//-- instance - an instance of config with values set by the designer

	QString value = "";

	//-- determine the output type
	QDomNode output = config.firstChild();
	while (!output.isNull() && output.nodeName() != cs_Output)
		output = output.nextSibling();

	FATAL(output.isNull(), ("Output type not defined!"));

	QuestEditorConfig::QuestOutputType qot = QuestEditorConfig::getQOTFromString(output.toElement().attribute(cs_Type));

	switch (qot)
	{
	case QuestEditorConfig::QOT_Script:
		{
			//-- use the script defined in the task specific definition
			QDomElement taskType = QuestEditorConfig::getEditorTaskNode(instance.attribute(cs_Type)).toElement();
			value = taskType.attribute(cs_Script);
		}
		break;
	case QuestEditorConfig::QOT_Completion:
		{
			//-- include child tasks that are not marked as failure tasks
			for (QDomNode child = instance.firstChild(); !child.isNull(); child = child.nextSibling())
				if (child.nodeName() == cs_Task && child.toElement().attribute(cs_TaskOnFail) == "false")
				{
					if (!value.isEmpty())
						value.append(",");

					value.append(idRowMap[child.toElement().attribute(cs_Id).toInt()]);
				}
		}
		break;
	case QuestEditorConfig::QOT_Failure:
		{
			//-- include child tasks that are marked as failure tasks
			for (QDomNode child = instance.firstChild(); !child.isNull(); child = child.nextSibling())
			{
				if (child.nodeName() == cs_Task && child.toElement().attribute(cs_TaskOnFail) == "true")
				{
					if (!value.isEmpty())
						value.append(",");

					value.append(idRowMap[child.toElement().attribute(cs_Id).toInt()]);
				}
			}
		}
		break;
	case QuestEditorConfig::QOT_Prerequisite:
		if (!first)
		{
			value = QuestEditorConfig::getTaskValue(instance, config.attribute(cs_Name), config.attribute(cs_DefaultValue));

			//-- same as exclusion but also include parent task id as a row number
			QString newValue = "";

#if defined(QUEST_SET_PARENT_AS_PREREQUISITE)
			//-- do not include parent task id as a prerequisite if this task is a failure task
			if (instance.attribute(cs_TaskOnFail) != "true")
				newValue = idRowMap[instance.parentNode().toElement().attribute(cs_Id).toInt()];
#endif

			QStringList ids = QStringList::split(",", value);

			for (QStringList::iterator it = ids.begin(); it != ids.end(); ++it)
			{
				if (!newValue.isEmpty())
					newValue.append(",");

				newValue.append(idRowMap[(*it).toInt()]);
			}

			value = newValue;
		}
		break;
	case QuestEditorConfig::QOT_Exclusion:
		if (!first)
		{
			value = QuestEditorConfig::getTaskValue(instance, config.attribute(cs_Name), config.attribute(cs_DefaultValue));

			if (!value.isEmpty())
			{
				//-- convert comma delimited list of task ids to row numbers
				QString newValue = "";

				QStringList ids = QStringList::split(",", value);

				for (QStringList::iterator it = ids.begin(); it != ids.end(); ++it)
				{
					if (!newValue.isEmpty())
						newValue.append(",");

					newValue.append(idRowMap[(*it).toInt()]);
				}

				value = newValue;
			}
		}
		break;
	case QuestEditorConfig::QOT_None:
		break;
	case QuestEditorConfig::QOT_LocalizedString:
		{
			QString stringValue = QuestEditorConfig::getTaskValue(instance, config.attribute(cs_Name), config.attribute(cs_DefaultValue));

			//-- do not place empty strings in the localized string file
			if (!stringValue.isEmpty())
			{
				QString columnName = config.attribute(cs_ColumnName).lower();

				if (instance.nodeName() == cs_Task)
				{
					int id = idRowMap[instance.attribute(cs_Id).toInt()].toInt();
					columnName = QString().sprintf("%s%02d_%s", cs_Task, id, columnName.ascii());
				}

				m_stringTable.setString(columnName, stringValue);

				char const * const stringPrefix = ConfigFile::getKeyString(cs_questEditorSection, cs_stringPrefix, "");

				value = stringPrefix + m_basename.lower() + ":" + columnName;
			}
		}
		break;
	case QuestEditorConfig::QOT_String:
		value = QuestEditorConfig::getTaskValue(instance, config.attribute(cs_Name), config.attribute(cs_DefaultValue));
		break;
	case QuestEditorConfig::QOT_Bool:
		value = QuestEditorConfig::getTaskValue(instance, config.attribute(cs_Name), config.attribute(cs_DefaultValue));
		if(value.at(0) == 'T' || value.at(0) == 't' || value.at(0) == '1')
			value = "1";
		else
			value = "0";
		break;
	default:
		FATAL(true, ("Unsupported output type [%s]", output.toElement().attribute(cs_Type).ascii()));
		break;
	}

	return value;
}

// ----------------------------------------------------------------------

void QuestEditor::exportTaskDataTable(QString const & filename)
{
	emit consoleOutput(QString("Exporting task data table [%1]").arg(filename));

	StringVectorVector taskDataTable;

	//-- allocate the max dimensions for the data table
	taskDataTable.resize(cs_MaxTaskRows + 2);
	for(int i=0; i<cs_MaxTaskRows + 2; ++i)
		taskDataTable[i].resize(cs_MaxTaskColumns, "");

	StringVector & columnName = taskDataTable[0];
	StringVector & columnType = taskDataTable[1];

	QDomNode defaultTask = QuestEditorConfig::getEditorDefaultTaskNode();
	QDomNodeList dataNodes = defaultTask.toElement().elementsByTagName(cs_Data);

	//-- setup the columns for the task defaults
	int lastColumn = 0;
	{
		for (uint i = 0; i < dataNodes.count(); ++i)
		{
			QDomNode data = dataNodes.item(i);

			if (data.toElement().attribute(cs_NoOutput) != "true")
			{
				QDomElement dataElement = data.toElement();

				columnName[lastColumn] = dataElement.attribute(cs_ColumnName);

				FATAL(columnName[lastColumn].isEmpty(),
					("No columnName defined in configuration XML for [%s]", dataElement.attribute(cs_Name).ascii())
					);

				columnType[lastColumn++] = dataElement.attribute(cs_ColumnType);
			}
		}
	}

	QDomNodeList tasks = m_quest->elementsByTagName(cs_Task);

	//-- create a map of ids to row numbers
	std::map<int, QString> idRowMap;

	for (uint j=0; j<tasks.count(); ++j)
		idRowMap[tasks.item(j).toElement().attribute(cs_Id).toInt()] = QString().number(j);

	//-- iterate through each task instance
	for (uint row=0; row<tasks.count(); ++row)
	{
		QDomElement task = tasks.item(row).toElement();

		//-- default task data
		{
			int index = 0;

			for (uint i = 0; i < dataNodes.count(); ++i)
			{
				QDomNode data = dataNodes.item(i);

				if (data.toElement().attribute(cs_NoOutput) != "true")
					taskDataTable[row + 2][index++] = getOutputValue(data.toElement(), task, row == 0, idRowMap);
			}
		}

		//-- specific task data
		{
			QDomNodeList taskDataNodes = QuestEditorConfig::getEditorTaskNode(task.attribute(cs_Type)).toElement().elementsByTagName(cs_Data);

			//-- based on the type of task, add data specific to it
			for (uint i = 0; i < taskDataNodes.count(); ++i)
			{
				QDomNode data = taskDataNodes.item(i);

				if (data.toElement().attribute(cs_NoOutput) != "true")
				{
					QDomElement dataElement = data.toElement();
					int index = 0;
					bool found = false;

					//-- find the index of the column if it exists
					for (; index < cs_MaxTaskColumns && !columnName[index].isEmpty(); ++index)
						if (columnName[index] == dataElement.attribute(cs_ColumnName))
						{
							found = true;
							break;
						}

					//-- if the column is not found, add a new one
					if (!found)
					{
						columnName[lastColumn] = dataElement.attribute(cs_ColumnName);
						columnType[lastColumn] = dataElement.attribute(cs_ColumnType);

						DEBUG_REPORT_LOG(false, ("[%d] [%s]\n", lastColumn, dataElement.attribute(cs_ColumnName).ascii()));

						index = lastColumn++;
					}
					
					taskDataTable[row + 2][index] = getOutputValue(data.toElement(), task, row == 0, idRowMap);
				}
			}
		}
	}

	writeDataTable(filename, taskDataTable, tasks.count() + 2, lastColumn);
}

// ----------------------------------------------------------------------

void QuestEditor::exportListDataTable(QString const & filename)
{
	emit consoleOutput(QString("Exporting list data table [%1]").arg(filename));

	StringVectorVector listDataTable;

	//-- allocate the max dimensions
	listDataTable.resize(cs_MaxListRows + 2);
	for(int i=0; i<cs_MaxListRows + 2; ++i)
		listDataTable[i].resize(cs_MaxListColumns, "");

	int lastColumn = 0;

	//-- read the list config data first
	QDomNode list = QuestEditorConfig::getEditorListNode().toElement();
	QDomNodeList dataNodes = list.toElement().elementsByTagName(cs_Data);

	//-- setup the column header and type
	{
		StringVector & columnName = listDataTable[0];
		StringVector & columnType = listDataTable[1];

		//-- for each data node, update the name and type vectors of the data table
		for (uint i = 0; i < dataNodes.count(); ++i)
		{
			QDomNode data = dataNodes.item(i);

			if (data.toElement().attribute(cs_NoOutput) != "true")
			{
				QDomElement dataElement = data.toElement();

				columnName[lastColumn] = dataElement.attribute(cs_ColumnName);

				FATAL(columnName[lastColumn].isEmpty(),
					("No columnName defined in configuration XML for [%s]", dataElement.attribute(cs_Name).ascii())
					);

				columnType[lastColumn++] = dataElement.attribute(cs_ColumnType);
			}
		}
	}

	//-- write the row data
	{
		//-- run through all the default columns
		int index = 0;
		for (uint i = 0; i < dataNodes.count(); ++i)
		{
			QDomNode data = dataNodes.item(i);

			if (data.toElement().attribute(cs_NoOutput) != "true")
			{
				std::map<int, QString> idRowMap;
				listDataTable[2][index++] = getOutputValue(data.toElement(), m_quest->getList().toElement(), false, idRowMap);
			}
		}
	}

	writeDataTable(filename, listDataTable, cs_MaxListRows + 2, lastColumn);
}

// ----------------------------------------------------------------------

void QuestEditor::writeDataTable(char const * const filename, StringVectorVector const & stringVectorVector, int length, int width)
{
		FILE * file = fopen(filename, "w");

		if (!file)
		{
			WARNING(true, ("QuestEditor::writeDataTable() - %s", filename));
			return;
		}

		for (int row=0; row < length; ++row)
		{
			for (int col=0; col < width; ++col)
			{
				if (col)
					fputs("\t", file);

				QString const & value = stringVectorVector[row][col];

				FATAL(value.isNull(), ("Encountered null value in data table. [%s] (%d, %d)",
					stringVectorVector[0][col].isNull() ? "(null)" : stringVectorVector[0][col].ascii(), row, col));

				fputs(value, file);
			}

			fputs("\n", file);
		}

		fclose(file);
}

// ----------------------------------------------------------------------

void QuestEditor::initializeNewTask(QDomElement task)
{
	//-- set the id
	task.setAttribute(cs_Id, m_quest->getUniqueTaskId());

	//-- set the task defaults first
	addNewQuestData(QuestEditorConfig::getEditorDefaultTaskNode(), task);

	//-- based on the type of task, add data specific to it
	QString taskType = task.attribute(cs_Type);

	addNewQuestData(QuestEditorConfig::getEditorTaskNode(taskType), task);
}

// ----------------------------------------------------------------------

void QuestEditor::initializeNewList(QDomElement list)
{
	addNewQuestData(QuestEditorConfig::getEditorListNode(), list);
}

// ----------------------------------------------------------------------

void QuestEditor::addNewQuestData(QDomNode configNode, QDomElement parent)
{
	//-- for each data node defined in the config file,
	//--   add quest name/value data
	QDomNodeList dataNodes = configNode.toElement().elementsByTagName(cs_Data);

	for (uint i = 0; i < dataNodes.count(); ++i)
	{
		QDomElement configDataElement = dataNodes.item(i).toElement();

		if (configDataElement.attribute(cs_DontSave) != "true")
		{
			if (configDataElement.attribute(cs_MakeAttribute) != "true")
			{
				QDomElement dataElement = m_quest->createElement(cs_Data);

				dataElement.setAttribute(cs_Value, configDataElement.attribute(cs_DefaultValue));
				dataElement.setAttribute(cs_Name, configDataElement.attribute(cs_Name));

				parent.appendChild(dataElement);
			}
			else
				parent.setAttribute(configDataElement.attribute(cs_Name), configDataElement.attribute(cs_DefaultValue));
		}
	}
}

// ----------------------------------------------------------------------

void QuestEditor::nodeDropped(const QString &sourceId, const QString& destinationId)
{
	DEBUG_REPORT_LOG(true, ("nodeDropped() - sourceId [%s] destinationId [%s]\n",
		sourceId.latin1(), destinationId.latin1()));

	TreeItem * sourceItem = 0;
	TreeItem * destinationItem = 0;

	for (QListViewItemIterator it(m_taskTree); it.current() && !(sourceItem && destinationItem); ++it)
	{
		TreeItem * item = static_cast<TreeItem *>(*it);

		if (!item)
			continue;
	
		if (!sourceItem && item->getNode().toElement().attribute(cs_Id) == sourceId)
			sourceItem = item;

		if (!destinationItem && item->getNode().toElement().attribute(cs_Id) == destinationId)
			destinationItem = item;
	}

	if (!sourceItem || !destinationItem)
	{
		DEBUG_REPORT_LOG(true, ("nodeDropped() - Unable to find tree items: sourceId [%s] destinationId [%s]\n",
			sourceId.latin1(), destinationId.latin1()));

		return;
	}

	//-- see if the destination is an ancestor of source
	QDomNodeList sourceChildTasks = sourceItem->getNode().toElement().elementsByTagName(cs_Task);

	for (uint i=0; i < sourceChildTasks.count(); ++i)
		if (sourceChildTasks.item(i) == destinationItem->getNode())
		{
			//-- exchange the nodes if the parent was dropped on a child
			exchangeNodes(sourceItem, destinationItem);
			return;
		}
	
	//-- move destination in the to the new location in the tree
	TreeItem * parent = static_cast<TreeItem *>(sourceItem->parent());

	if (parent)
	{
		//-- if the destination is the same as parent, don't do the drop
		if (parent == destinationItem)
		{
			DEBUG_REPORT_LOG(true, ("nodeDropped() - Tried to move to current parent!\n"));
		}
		else
		{
			//-- update the gui
			parent->takeItem(sourceItem);
			destinationItem->insertItem(sourceItem);

			//-- update the dom
			QDomNode node = parent->getNode().removeChild(sourceItem->getNode());
			destinationItem->getNode().appendChild(node);

			setDirty();

			destinationItem->setOpen(true);
		}
	}
	else
		DEBUG_REPORT_LOG(true, ("nodeDropped() - Tried to move root item!\n"));
}

// ----------------------------------------------------------------------

void QuestEditor::slotCurrentChanged(QListViewItem * current)
{
	TreeItem * i = dynamic_cast<TreeItem *>(current);

	if (i)
	{
		QDomElement task = i->getNode().toElement();
		m_taskPropertyEditor->setTask(task);

		DEBUG_REPORT_LOG(false, ("QuestEditor::slotCurrentChanged() - [%s]\n", task.attribute(cs_Type)));
	}
	else
	{
		m_taskPropertyEditor->clearTask();

		DEBUG_REPORT_LOG(false, ("QuestEditor::slotCurrentChanged() - [NONE]\n"));
	}
}

// ----------------------------------------------------------------------

void QuestEditor::closeEvent(QCloseEvent * eventClose)
{
	if (m_dirty)
	{
		int result = QMessageBox::warning(this, "Changes not saved!", "Are you sure you want to close?",
			QMessageBox::Yes, QMessageBox::No);

		if (result == QMessageBox::No)
		{
			eventClose->ignore();
			return;
		}
	}

	BaseQuestWidget::closeEvent(eventClose);
}

// ----------------------------------------------------------------------

void QuestEditor::paintEvent(QPaintEvent * eventPaint)
{
	BaseQuestWidget::paintEvent(eventPaint);

	updateCaption();
}

// ----------------------------------------------------------------------

void QuestEditor::updateCaption()
{
	// use the quest list journal entry title as the caption in addition to the file name
	QString journalEntryTitle;

	for (QDomNode data = m_quest->getList().firstChild(); !data.isNull(); data = data.nextSibling())
	{
		if (data.nodeName() == cs_Data && data.toElement().attribute(cs_Name) == cs_journalEntryTitle)
		{
			journalEntryTitle = data.toElement().attribute(cs_Value);
			break;
		}
	}

	if (journalEntryTitle.isNull() || journalEntryTitle.isEmpty())
		journalEntryTitle = "Unnamed";

	QString editorCaption = QString("%1%2 [%3]")
		.arg(m_dirty ? "* " : "")
		.arg(journalEntryTitle)
		.arg(m_filename.isEmpty() ? QString("Unsaved") : m_filename);

	setCaption(editorCaption);
}

// ----------------------------------------------------------------------

ToolProcess * QuestEditor::getToolProcess() const
{
	return dynamic_cast<MainWindow *>(qApp->mainWidget())->getToolProcess();
}

// ----------------------------------------------------------------------

void QuestEditor::addToPerforce()
{
	if (m_filename.isEmpty())
	{
		QMessageBox::warning(this, "Perforce Warning", "You must save the quest before adding to perforce!", "Ok");
		return;
	}

	if (getToolProcess()->isRunning())
	{
		QMessageBox::warning(this, "Perforce Warning", "Processes still running!", "Ok");
		return;
	}

	emit consoleOutput(QString("Perforcing quest [%1]! Please wait.").arg(m_filename));

	getToolProcess()->addToPerforce(m_filename.lower());
	getToolProcess()->addToPerforce(m_listTab);
	getToolProcess()->addToPerforce(m_listIff);
	getToolProcess()->addToPerforce(m_taskTab);
	getToolProcess()->addToPerforce(m_taskIff);
	getToolProcess()->addToPerforce(m_stringFile);
}

// ----------------------------------------------------------------------

void QuestEditor::compile()
{
	if (m_dirty || m_filename.isEmpty())
	{
		QMessageBox::warning(this, "Compile Warning", "You must save the quest before compiling!", "Ok");
		return;
	}

	getToolProcess()->compileDataTable(m_listTab);
	getToolProcess()->compileDataTable(m_taskTab);
}

// ----------------------------------------------------------------------

void QuestEditor::check()
{
	if (m_dirty || m_filename.isEmpty())
	{
		QMessageBox::warning(this, "Check Warning", "You must save the quest before checking!", "Ok");
		return;
	}

	getToolProcess()->checkQuest(m_filename);
}

// ----------------------------------------------------------------------

void QuestEditor::setFilename(QString const & filename)
{
	m_filename = filename;

	//-- derive the questtask and questlist file names from the quest file name
	m_basename = m_filename;
	m_basename.replace(QRegExp(".*([^/]*)\\.qst"), "/\\1");

	char const * const defaultTaskDirectory = ConfigFile::getKeyString(cs_questEditorSection, cs_defaultTaskDirectory, "");
	char const * const defaultListDirectory = ConfigFile::getKeyString(cs_questEditorSection, cs_defaultListDirectory, "");

	m_listTab = defaultListDirectory + m_basename + ".tab";
	m_taskTab = defaultTaskDirectory + m_basename + ".tab";

	m_listIff = m_listTab;
	m_listIff.replace("/dsrc/", "/data/");
	m_listIff.replace(".tab", ".iff");

	m_taskIff = m_taskTab;
	m_taskIff.replace("/dsrc/", "/data/");
	m_taskIff.replace(".tab", ".iff");

	char const * const defaultStfDirectory = ConfigFile::getKeyString(cs_questEditorSection, cs_defaultStfDirectory, "");

	m_stringFile = defaultStfDirectory + m_basename + ".stf";
}

// ----------------------------------------------------------------------

bool QuestEditor::filesWritable()
{
	bool writable = true;

	if (m_filename.isEmpty())
		return false;

	writable &= FileNameUtils::isWritable(m_filename.ascii());
	writable &= FileNameUtils::isWritable(m_listTab.ascii());
	writable &= FileNameUtils::isWritable(m_listIff.ascii());
	writable &= FileNameUtils::isWritable(m_taskTab.ascii());
	writable &= FileNameUtils::isWritable(m_taskIff.ascii());
	writable &= FileNameUtils::isWritable(m_stringFile.ascii());

	return writable;
}

// ----------------------------------------------------------------------

void QuestEditor::addHelpText(QTextEdit * helpText)
{
	//-- "&nbsp;" is a non-breakable space (used for indentation)
	char const * const editorTextFormat = "<center><u><b>%1</b></u></center>&nbsp;&nbsp;&nbsp;&nbsp;%2";
	char const * const headingTextFormat = "<br><b>%1</b><br>";
	char const * const paragraphTextFormat = "&nbsp;&nbsp;&nbsp;&nbsp;%1<br>";
	char const * const taskTextFormat = "<br><b>%1</b> - %2";
	char const * const questListHeading = "Quest List";
	char const * const questTaskHeading = "Quest Task";
	char const * const questTaskDetailHeading = "Quest Task Detail";

	//-- get the editor info for quests
	QDomElement editor = QuestEditorConfig::getEditorNode(cs_QuestEditor).toElement();

	//-- QuestEditor header and summary
	{
		QString const name = editor.attribute(cs_Name);
		QString const info = editor.attribute(cs_Info, cs_defaultInfoText);

		helpText->append(QString(editorTextFormat).arg(name).arg(info));
	}

	//-- Iterate through list data
	helpText->append(QString(headingTextFormat).arg(questListHeading));
	{
		QDomElement list = QuestEditorConfig::getEditorListNode().toElement();

		QString const info = list.attribute(cs_Info, cs_defaultInfoText);

		helpText->append(QString(paragraphTextFormat).arg(info));

		addFieldDataText(helpText, list);
	}

	//-- Iterate through default task fields
	helpText->append(QString(headingTextFormat).arg(questTaskHeading));
	{
		QDomElement defaultTask = QuestEditorConfig::getEditorDefaultTaskNode().toElement();

		QString const info = defaultTask.attribute(cs_Info, cs_defaultInfoText);

		helpText->append(QString(paragraphTextFormat).arg(info));

		addFieldDataText(helpText, defaultTask);
	}

	//-- Iterate through tasks and describe each one with specific field detail
	helpText->append(QString(headingTextFormat).arg(questTaskDetailHeading));
	{
		QDomNodeList tasks = editor.elementsByTagName(cs_Task);

		QString const tasksInfo = QuestEditorConfig::getEditorTasksNode().toElement().attribute(cs_Info, cs_defaultInfoText);

		helpText->append(QString(paragraphTextFormat).arg(tasksInfo));

		for (uint i=0; i<tasks.count(); ++i)
		{
			QDomElement task = tasks.item(i).toElement();

			QString const type = task.attribute(cs_Type);
			QString const info = task.attribute(cs_Info, cs_defaultInfoText);

			helpText->append(QString(taskTextFormat).arg(type).arg(info));

			addFieldDataText(helpText, task);
		}
	}
}

// ----------------------------------------------------------------------

void QuestEditor::exchangeNodes(TreeItem * sourceItem, TreeItem * destinationItem)
{
	//-- exchange the nodes
	QDomNode destinationNode = destinationItem->getNode();
	QDomNode sourceNode = sourceItem->getNode();

	//-- source and destination node shallow copies
	QDomNode destinationCopy = destinationNode.cloneNode(FALSE);
	QDomNode sourceCopy = sourceNode.cloneNode(FALSE);

	//-- parents
	QDomNode destinationParent = destinationNode.parentNode();
	QDomNode sourceParent = sourceNode.parentNode();

	DEBUG_REPORT_LOG(false, ("Destination = [%s]\n", NODEINFO(destinationNode)));

	//-- destination
	while (destinationNode.hasChildNodes())
	{
		QDomNode child = destinationNode.lastChild();
		QDomNode result;

		DEBUG_REPORT_LOG(false, ("Child = [%s]\n", NODEINFO(child)));

		//-- move data nodes to the destination copy
		if (child.nodeName() == cs_Data)
			result = destinationCopy.appendChild(child);

		//-- move task nodes to the source copy
		else if (child.nodeName() == cs_Task)
			result = sourceCopy.appendChild(child);

		else
		{
			QMessageBox::warning(this, "Exchange Warning", QString("Unknown node [%1]").arg(NODEINFO(child)), "Ok");
			result = destinationNode.removeChild(child);
		}

		WARNING(result.isNull(), ("Failed to move node [%s]", NODEINFO(child)));
	}

	DEBUG_REPORT_LOG(false, ("Source = [%s]\n", NODEINFO(sourceNode)));

	bool destinationIsChildOfSource = false;

	//-- source
	while (sourceNode.hasChildNodes())
	{
		QDomNode child = sourceNode.lastChild();
		QDomNode result;

		DEBUG_REPORT_LOG(false, ("Child = [%s]\n", NODEINFO(child)));

		//-- special case if the child is the destination
		if (child == destinationNode)
		{
			destinationIsChildOfSource = true;
			result = sourceNode.removeChild(child);
		}

		//-- move data nodes to the source copy
		else if (child.nodeName() == cs_Data)
			result = sourceCopy.appendChild(child);

		//-- move task nodes to the destination copy
		else if (child.nodeName() == cs_Task)
			result = destinationCopy.appendChild(child);

		else
		{
			QMessageBox::warning(this, "Exchange Warning", QString("Unknown node [%1]").arg(NODEINFO(child)), "Ok");
			result = sourceNode.removeChild(child);
		}

		WARNING(result.isNull(), ("Failed to move node [%s]", NODEINFO(child)));
	}

	//-- update the DOM
	if (!destinationIsChildOfSource)
		destinationParent.replaceChild(sourceCopy, destinationNode);
	else
		destinationCopy.appendChild(sourceCopy);

	sourceParent.replaceChild(destinationCopy, sourceNode);

	//-- update the tree view
	destinationItem->setNode(sourceCopy);
	sourceItem->setNode(destinationCopy);

	setDirty();
}

// ======================================================================

void QuestEditorNamespace::addFieldDataText(QTextEdit * helpText, QDomElement const & parent)
{
	char const * const fieldDataTextFormat = "&nbsp;&nbsp;&nbsp;&nbsp;<i><b>%1</b></i> - %2";

	QDomNodeList dataNodes = parent.elementsByTagName(cs_Data);
	for (uint i=0; i<dataNodes.count(); ++i)
	{
		QDomElement dataElement = dataNodes.item(i).toElement();

		QString const name = dataElement.attribute(cs_Name);
		QString const displayName = dataElement.attribute(cs_DisplayName, name);
		QString const info = dataElement.attribute(cs_Info, cs_defaultInfoText);

		if (!info.isEmpty())
			helpText->append(QString(fieldDataTextFormat).arg(displayName).arg(info));
	}
}

// ----------------------------------------------------------------------

void QuestEditor::setDirty()
{
	m_dirty = true;
	updateCaption();

	TreeItem * item = dynamic_cast<TreeItem *>(m_taskTree->currentItem());

	if (item)
	{
		item->refresh();
	}
}

// ----------------------------------------------------------------------

QString QuestEditor::getFileName() const
{
	return m_filename;
}

// ----------------------------------------------------------------------

void QuestEditor::selectTaskField(QString const & taskId, QString const & field)
{
	m_questTabWidget->setCurrentPage(0);

	for (QListViewItemIterator it(m_taskTree); it.current(); ++it)
	{
		TreeItem * item = static_cast<TreeItem *>(*it);

		if (item && item->getNode().toElement().attribute(cs_Id) == taskId)
		{
			m_taskTree->setSelected(item, true);
			m_taskPropertyEditor->setSelectedField(field);
			break;
		}
	}
}

// ----------------------------------------------------------------------

void QuestEditor::selectListField(QString const & field)
{
	m_questTabWidget->setCurrentPage(1);
	m_listPropertyEditor->setSelectedField(field);
}

// ======================================================================
