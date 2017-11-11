// ============================================================================
//
// TemplateEditor.h
//
// Main Window that the user interfaces with for template editing. Contains a
// menu bar, a parameter/value pair list, and other relevant data windows.
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TemplateEditor_H
#define INCLUDED_TemplateEditor_H

#include "BaseTemplateEditor.h"

class TemplateEditorManager;

//-----------------------------------------------------------------------------
class TemplateEditor : public BaseTemplateEditor
{
	Q_OBJECT

	friend TemplateEditorManager;

public:

	TemplateEditor(QWidget *parent, const char *name);
	~TemplateEditor();

	void getColor(QString const &path, QColor &color, int &index);
	bool open(QString const &path, bool const initializeTemplateTable = true, bool const forceUpdateWorkingFileList = true, bool const quickLoad = false);
	void autoSave();

private slots:

	void onFileActionActivated();
	void onNewActionActivated();
	void onOpenActionActivated();
	void onSaveActionActivated();
	void onSaveAsActionActivated();
	void onExitActionActivated();
	void onTestAllTemplatesActionActivated();
	void slotFileListViewDoubleClicked(QListViewItem *listViewItem);
	void slotFileListViewRightButtonClicked(QListViewItem *listViewItem, const QPoint &point, int column);

signals:

protected:

	virtual void mouseReleaseEvent(QMouseEvent *mouseEvent);

private:

	typedef std::set<QString> StringSet;

	void buildLegend();
	void updateWorkingFileList();
	bool sourceControlCheckOutFile(QString const &path) const;
	bool sourceControlAddFile(QString const &path) const;
	void compileFile(QString const &path);
	void searchAllDirectories(QString const &currentDirectory, QStringList &allFiles, StringSet &directorySet);
	void setCurrentFile(QString const &path, bool const forceUpdateWorkingFileList);
	void addFileToWorkingList(QString const &path, bool const forceUpdate);
	void save(QString const &path);

	TemplateEditorManager *m_templateEditorManager;
	QFileInfo              m_fileInfoCurrent;
};

// ============================================================================

#endif // INCLUDED_TemplateEditor_H