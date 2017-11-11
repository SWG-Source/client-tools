// ============================================================================
//
// MainWindow.h
// Copyright 2004, Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_MainWindow_H
#define INCLUDED_MainWindow_H

#include "BaseMainWindow.h"
#include "sharedUtility/DataTable.h"
#include "ObjectTemplateWriter.h"

#include <list>
#include <map>
#include <qfileinfo.h>
#include <qscrollview.h>
#include <qslider.h>
#include <qvbox.h>
#include <string>
#include <vector>

class TemplateConfigDialog;
class BaseConsoleWindow;
class QProcess;

class CustomizationVariable;

// ----------------------------------------------------------------------------

class MainWindow : public BaseMainWindow
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	MainWindow(QWidget *myParent, char const *myName);
	virtual ~MainWindow();

public slots:

	void slotViewerDirty();

	void slotFileNewAction();
	void slotFileOpenAction();
	void slotFileSaveAction();
	void slotFileSaveAsAction();
	void slotFileExitAction();
	
	void slotToolConfigureTemplatesAction();
	void slotToolCompileAction();
	void slotToolAddToPerforceAction();
	void slotToolRandomAction();

	void slotZoomHeadAction();
	void slotZoomRootAction();

	void slotAddWearableAction();
	void slotRemoveWearableAction();
	void slotSearchAction();

	void slotComboBoxPresetsActivated(int);

	void slotReadProcessOutput();
	void slotProcessExited();

	void slotShowConsoleAction();

	void slotAvatarCustomizationVariableChanged(int, const char *);
	void slotWearableCustomizationVariableChanged(int, const char *);

	void slotChangeBackdrop(int);
	void slotFilterTextChanged(const QString &);

protected:

	virtual void paintEvent(QPaintEvent *paintEvent);
	void updateCaption();

private: //-- data

	QFileInfo m_clientFileInfo;
	QFileInfo m_serverFileInfo;
	QFileInfo m_sharedFileInfo;

	QFileInfo m_clientCdfFileInfo;
	QFileInfo m_serverIffFileInfo;
	QFileInfo m_sharedIffFileInfo;

	DataTable m_npcDataTable;
	int m_row;
	ObjectTemplateWriter::ObjectTemplateParameterMap m_serverParameters;
	ObjectTemplateWriter::ObjectTemplateParameterMap m_sharedParameters;

	BaseConsoleWindow * m_consoleWindow;
	QProcess * m_proc;

	QVBox * m_avatarVBox;
	QScrollView * m_avatarScrollView;

	QVBox * m_wearableVBox;
	QScrollView * m_wearableScrollView;

	std::list< std::vector< std::string > > m_procArgList;

	bool m_openExistingMode;
	bool m_dirty;
	bool m_overwriteTemplatesMode;

private: //-- methods

	void setFileInfo(QString const &path, QFileInfo &dsrcFileInfo, char const * dsrcExt,
		QFileInfo &dataFileInfo, char const * dataExt);

	void setClientDataFile(QString const &path);
	void setServerTemplate(QString const &path);
	void setSharedTemplate(QString const &path);

	void saveClientDataFile(QString const &originalFile);
	void saveServerTemplate(QString const &originalFile);
	void saveSharedTemplate(QString const &originalFile);

	void loadNpcDataTable();

	void setColors(char const * currentText, bool wearable);
	void showColorPicker(QSlider * slider);

	void startNextProcess();
	void AddToPerforce(const QFileInfo &fi);

	bool haveFileNames() const;

	void createAvatarCustomVariableWidgets();
	static void  callbackCreateAvatarCustomVariableWidgets(const std::string &fullVariablePathName,
		CustomizationVariable *cv, void *context);

	void createWearableCustomVariableWidgets();
	static void  callbackCreateWearableCustomVariableWidgets(const std::string &fullVariablePathName,
		CustomizationVariable *cv, void *context);

	void clearFiles();
	void clearTemplateParameters();

private: //-- disabled

	MainWindow(MainWindow const &);
	MainWindow &operator =(MainWindow const &);
	MainWindow();
};

// ============================================================================

#endif // INCLUDED_MainWindow_H
