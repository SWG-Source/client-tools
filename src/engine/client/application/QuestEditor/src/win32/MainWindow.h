// ======================================================================
//
// MainWindow.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MainWindow_H
#define INCLUDED_MainWindow_H

// ----------------------------------------------------------------------

#include "BaseMainWindow.h"

// ----------------------------------------------------------------------

#include <qmainwindow.h>

// ----------------------------------------------------------------------

class QDockWindow;
class QPopupMenu;
class QTextEdit;
class QuestEditor;
class QWorkspace;
class ToolProcess;

// ----------------------------------------------------------------------

class MainWindow: public BaseMainWindow
{
	Q_OBJECT //lint !e830 !e1511 !e1516 !e1924
		
public:
	MainWindow();
	~MainWindow();

	ToolProcess * getToolProcess() const;
	
protected:
	void closeEvent(QCloseEvent *);
	QuestEditor * getQuestEditor(QString const & filename);
	
private slots:
	
	void newQuest();
	void loadQuest();

	void save() const;
	void saveAs() const;
	
	void exportQuest() const;

	void closeWindow();
	void tileHorizontal();
	
	void about();
	void showHelp();

	void windowsMenuAboutToShow();
	void windowsMenuActivated(int id);

	void outputToConsole(const QString& text);

	void addToPerforce();
	void compile();
	void buildQuestCrcStringTables();

	void consoleDoubleClicked(int para, int pos);
	
private:
	QWorkspace * m_ws;
	QDockWindow * m_consoleDock;
	QTextEdit * m_console;
	ToolProcess * m_toolProcess;

private: //-- disabled
	MainWindow(MainWindow const &);
	MainWindow &operator=(MainWindow const &);
};

// ----------------------------------------------------------------------

inline ToolProcess * MainWindow::getToolProcess() const
{
	return m_toolProcess;
}

// ======================================================================

#endif // INCLUDED_MainWindow_H
