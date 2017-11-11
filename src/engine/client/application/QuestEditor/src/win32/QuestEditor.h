// ======================================================================
//
// QuestEditor.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_QuestEditor_H
#define INCLUDED_QuestEditor_H

// ----------------------------------------------------------------------

#include "BaseQuestWidget.h"

// ----------------------------------------------------------------------

#include "StringTable.h"

// ----------------------------------------------------------------------

#include <map>
#include <vector>

// ----------------------------------------------------------------------

class BaseTaskWidget;
class ListPropertyEditor;
class QDomElement;
class QDomNode;
class QListViewItem;
class Quest;
class TaskPropertyEditor;
class ToolProcess;
class TreeItem;

// ----------------------------------------------------------------------

class QuestEditor : public BaseQuestWidget
{
	Q_OBJECT //lint !e830 !e1511 !e1516 !e1924

	typedef std::vector<QString> StringVector;
	typedef std::vector<StringVector> StringVectorVector;

public:
	explicit QuestEditor(QWidget *questWidgetParent, char const * const questWidgetName = 0, WFlags questWidgetFlags = 0);
	~QuestEditor();

	void save();
	void saveAs();
	void exportDataTables();
	void load(char const * const filename);

	void buildTaskTree(QDomNode node, TreeItem * parent);

	void addToPerforce();
	void compile();
	void check();

	QString getFileName() const;

	void selectTaskField(QString const & taskId, QString const & field);
	void selectListField(QString const & field);

	static void addHelpText(QTextEdit * helpText);

protected:
	void initializeNewTask(QDomElement task);
	void initializeNewList(QDomElement list);
	void addNewQuestData(QDomNode configNode, QDomElement parent);

	void closeEvent(QCloseEvent * eventClose);
	void paintEvent(QPaintEvent * eventPaint);
	void updateCaption();

	ToolProcess * getToolProcess() const;

	void setFilename(QString const & filename);
	bool filesWritable();

	void exchangeNodes(TreeItem * sourceItem, TreeItem * destinationItem);

signals:
	void consoleOutput(const QString&);

private slots:
	void rmbClicked(QListViewItem *i, const QPoint& pos);

	void nodeDropped(const QString&, const QString&);
	void slotCurrentChanged(QListViewItem *);

	void setDirty();

private:
	QString getOutputValue(QDomElement const &config, QDomElement const &instance, bool first, std::map<int, QString> &idRowMap);

	void exportTaskDataTable(QString const & filename);
	void exportListDataTable(QString const & filename);

	void writeDataTable(char const * const filename, StringVectorVector const & stringVectorVector, int length, int width);

private:
	Quest *m_quest;
	TaskPropertyEditor *m_taskPropertyEditor;
	ListPropertyEditor *m_listPropertyEditor;
	QString m_filename;
	QString m_basename;
	QString m_listTab;
	QString m_listIff;
	QString m_taskTab;
	QString m_taskIff;
	QString m_stringFile;
	bool m_dirty;
	StringTable m_stringTable;

private: //-- disabled
	QuestEditor(QuestEditor const &);
	QuestEditor &operator=(QuestEditor const &);
	QuestEditor();
};

// ======================================================================

#endif // INCLUDED_QuestEditor_H
