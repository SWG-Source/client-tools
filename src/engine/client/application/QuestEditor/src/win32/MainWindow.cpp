// ======================================================================
//
// MainWindow.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "BaseHelpDialog.h"
#include "MainWindow.h"
#include "QuestEditor.h"
#include "QuestEditorConfig.h"
#include "QuestEditorConstants.h"
#include "ToolProcess.h"

// ----------------------------------------------------------------------

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedUtility/SetupSharedUtility.h"

// ----------------------------------------------------------------------

#include <qdom.h>
#include <qdockwindow.h>
#include <qfiledialog.h>
#include <qiconview.h>
#include <qimage.h>
#include <qlistbox.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qprinter.h>
#include <qregexp.h>
#include <qsplitter.h>
#include <qstatusbar.h>
#include <qtextedit.h>
#include <qtoolbutton.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <qwidgetfactory.h>
#include <qworkspace.h>

// ----------------------------------------------------------------------

#include "MainWindow.moc"

// ----------------------------------------------------------------------

#include "images/cascade.xpm"
#include "images/questeditor.xpm"
#include "images/tile.xpm"
#include "images/tile_horizontal.xpm"

// ----------------------------------------------------------------------

MainWindow::MainWindow()
: BaseMainWindow(0, "QuestEditor Main Window", WDestructiveClose)
, m_ws(0)
, m_consoleDock(0)
, m_console(0)
, m_toolProcess(0)
{
	QMainWindow::setDockMenuEnabled(true);

	//-- setup the actions
	{
		//-- file
		IGNORE_RETURN(connect(m_fileNewAction, SIGNAL(activated()), this, SLOT(newQuest())));
		IGNORE_RETURN(connect(m_fileOpenAction, SIGNAL(activated()), this, SLOT(loadQuest())));

		IGNORE_RETURN(connect(m_fileSaveAction, SIGNAL(activated()), this, SLOT(save())));
		IGNORE_RETURN(connect(m_fileSaveAsAction, SIGNAL(activated()), this, SLOT(saveAs())));
		
		IGNORE_RETURN(connect(m_fileExportAction, SIGNAL(activated()), this, SLOT(exportQuest())));
		
		IGNORE_RETURN(connect(m_fileCloseAction, SIGNAL(activated()), this, SLOT(closeWindow())));
		IGNORE_RETURN(connect(m_fileQuitAction, SIGNAL(activated()), this, SLOT(close())));

		//-- tool
		IGNORE_RETURN(connect(m_toolAddToPerforceAction, SIGNAL(activated()), this, SLOT(addToPerforce())));
		IGNORE_RETURN(connect(m_toolCompileAction, SIGNAL(activated()), this, SLOT(compile())));
		IGNORE_RETURN(connect(m_toolBuildQuestCrcTablesAction, SIGNAL(activated()), this, SLOT(buildQuestCrcStringTables())));

		//-- window
		m_windowsMenu->setCheckable(true);
		IGNORE_RETURN(connect(m_windowsMenu, SIGNAL(aboutToShow()), this, SLOT(windowsMenuAboutToShow())));
	}
	
	//-- setup the workspace
	{
		QVBox* vb = new QVBox(this);
		vb->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
		m_ws = new QWorkspace(vb);
		m_ws->setScrollBarsEnabled(true);
		QMainWindow::setCentralWidget(vb);
	}//lint !e429

	//-- setup the console window
	{
		m_consoleDock = new QDockWindow(QDockWindow::InDock, this, "ConsoleToolbar");
		m_console = new QTextEdit(m_consoleDock, "ConsoleWindow");

		QFont serifFont("Courier New", 8);
		m_console->setFont(serifFont);
		m_console->setReadOnly(true);

		m_consoleDock->setCaption("Console Toolbar");
		m_consoleDock->setWidget(m_console);
		m_consoleDock->setResizeEnabled(true);
		m_consoleDock->setCloseMode(QDockWindow::Always);

		QMainWindow::addDockWindow(m_consoleDock, Qt::Bottom);

		IGNORE_RETURN(connect(m_console, SIGNAL(doubleClicked(int, int)), SLOT(consoleDoubleClicked(int, int))));
	}

	//-- setup the game systems
	{
		//-- thread
		SetupSharedThread::install();
		
		//-- debug
		SetupSharedDebug::install(4096);
		
		//-- install the engine
		SetupSharedFoundation::Data setupSharedFoundationData(SetupSharedFoundation::Data::D_mfc);
		setupSharedFoundationData.useWindowHandle  = false;
		setupSharedFoundationData.configFile = "QuestEditor.cfg";
		SetupSharedFoundation::install(setupSharedFoundationData);
		
		//-- compression
		SetupSharedCompression::install();
		
		//-- file
		SetupSharedFile::install(false);
		
		//-- utility
		SetupSharedUtility::Data setupSharedUtilityData;
		SetupSharedUtility::setupToolData(setupSharedUtilityData);
		SetupSharedUtility::install(setupSharedUtilityData);

		TreeFile::addSearchAbsolute(0);
	}

	//-- setup the tool process object
	{
		char const * const toolDirectory = ConfigFile::getKeyString(cs_questEditorSection, cs_toolDirectory, "");
		m_toolProcess = new ToolProcess(toolDirectory);
		IGNORE_RETURN(connect(m_toolProcess, SIGNAL(consoleOutput(const QString&)), this, SLOT(outputToConsole(const QString&))));
	}

	statusBar()->message("Ready", 2000);
}

// ----------------------------------------------------------------------

MainWindow::~MainWindow()
{
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	delete m_toolProcess;
	m_toolProcess = 0;
}

// ----------------------------------------------------------------------

void MainWindow::save() const
{
	QuestEditor * editor = dynamic_cast<QuestEditor *>(m_ws->activeWindow());
	if (editor)
		editor->save();
}


// ----------------------------------------------------------------------

void MainWindow::saveAs() const
{
	QuestEditor * editor = (QuestEditor *)m_ws->activeWindow();
	if (editor)
		editor->saveAs();
}

// ----------------------------------------------------------------------

void MainWindow::exportQuest() const
{
	QuestEditor * editor = dynamic_cast<QuestEditor *>(m_ws->activeWindow());
	if (editor)
		editor->exportDataTables();
}

// ----------------------------------------------------------------------

void MainWindow::closeWindow()
{
	QWidget* m = dynamic_cast<QWidget *>(m_ws->activeWindow());
	if (m)
		m->close();
}

// ----------------------------------------------------------------------

void MainWindow::about()
{
	char const * const aboutText = QUEST_EDITOR_VERSION "\n\n"
		"Copyright 2004-2006, Sony Online Entertainment LLC (SOE)\n"
		"Portions Copyright Bootprint Entertainment";

	QMessageBox mb(cs_QuestEditor, aboutText, QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);

	mb.setIconPixmap(QPixmap(questeditor_xpm));

	mb.exec();
}

// ----------------------------------------------------------------------

void MainWindow::showHelp()
{
	BaseHelpDialog helpDialog;

	QTextEdit * helpText = helpDialog.m_helpTextEdit;

	helpText->clear();

	QuestEditor::addHelpText(helpText);

	helpDialog.exec();
}

// ----------------------------------------------------------------------

void MainWindow::windowsMenuAboutToShow()
{
	m_windowsMenu->clear();
	
	int cascadeId = m_windowsMenu->insertItem(QPixmap(cascade_xpm), "&Cascade", m_ws, SLOT(cascade()), Key_F2);
	int tileId = m_windowsMenu->insertItem(QPixmap(tile_xpm), "&Tile", m_ws, SLOT(tile()), Key_F3);
	int horTileId = m_windowsMenu->insertItem(QPixmap(tile_horizontal_xpm), "Tile &Horizontally", this, SLOT(tileHorizontal()), Key_F4);
	
	if (m_ws->windowList().isEmpty())
	{
		m_windowsMenu->setItemEnabled(cascadeId, false);
		m_windowsMenu->setItemEnabled(tileId, false);
		m_windowsMenu->setItemEnabled(horTileId, false);
	}
	IGNORE_RETURN(m_windowsMenu->insertSeparator());
	
	QWidgetList windows = m_ws->windowList();
	
	for (int i = 0; i < int(windows.count()); ++i)
	{
		int id = m_windowsMenu->insertItem(windows.at(i)->caption(), this, SLOT(windowsMenuActivated(int)));
		m_windowsMenu->setItemParameter(id, i);
		m_windowsMenu->setItemChecked(id, m_ws->activeWindow() == windows.at(i));
	}
}

// ----------------------------------------------------------------------

void MainWindow::windowsMenuActivated(int id)
{
	QWidget* w = m_ws->windowList().at(id);
	if (w)
		w->showNormal();
	w->setFocus();
}

// ----------------------------------------------------------------------

void MainWindow::tileHorizontal()
{
	//-- primitive horizontal tiling
	QWidgetList windows = m_ws->windowList();
	if (!windows.count())
		return;
	
	int heightForEach = m_ws->height() / windows.count();
	int y = 0;
	for (int i = 0; i < int(windows.count()); ++i)
	{
		QWidget *window = windows.at(i);
		if (window->testWState(WState_Maximized))
		{
			//-- prevent flicker
			window->hide();
			window->showNormal();
		}
		
		int preferredHeight = window->minimumHeight()+window->parentWidget()->baseSize().height();
		int actHeight = QMAX(heightForEach, preferredHeight);
		
		window->parentWidget()->setGeometry(0, y, m_ws->width(), actHeight);
		y += actHeight;
	}
}

// ----------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent *e)
{
	if (getToolProcess()->isRunning())
	{
		int result = QMessageBox::warning(this, "Exit Warning", "Processes still running! Exit anyways?",
			QMessageBox::Yes, QMessageBox::No);

		if (result == QMessageBox::No)
		{
			e->ignore();
			return;
		}
	}

	QWidgetList windows = m_ws->windowList();
	if (windows.count())
	{
		for (int i = 0; i < int(windows.count()); ++i)
		{
			QWidget *window = windows.at(i);
			if (!window->close())
			{
				e->ignore();
				return;
			}
		}
	}
	
	QMainWindow::closeEvent(e);
}

// ----------------------------------------------------------------------

void MainWindow::loadQuest()
{
	char const * const defaultQuestDirectory = ConfigFile::getKeyString(cs_questEditorSection, cs_defaultQuestDirectory, "");

	QString filename = QFileDialog::getOpenFileName(defaultQuestDirectory, "Quest (*.qst)", this,
		"open file dialog", "Choose a filename to open");
	
	if (!filename.isEmpty())
	{
		QuestEditor *w = new QuestEditor(m_ws, 0, WDestructiveClose);

		IGNORE_RETURN(connect(w, SIGNAL(consoleOutput(const QString&)), this, SLOT(outputToConsole(const QString&))));

		w->load(filename);
		w->show();
	}
	else
		statusBar()->message("Loading aborted", 2000);
}

// ----------------------------------------------------------------------

void MainWindow::newQuest()
{
	QuestEditor *w = new QuestEditor(m_ws, 0, WDestructiveClose);

	IGNORE_RETURN(connect(w, SIGNAL(consoleOutput(const QString&)), this, SLOT(outputToConsole(const QString&))));

	w->load(0);
	w->show();
}

// ----------------------------------------------------------------------

void MainWindow::outputToConsole(const QString& text)
{
	m_console->append(text);
	m_console->scrollToBottom();
}

// ----------------------------------------------------------------------

void MainWindow::addToPerforce()
{
	QuestEditor * editor = dynamic_cast<QuestEditor *>(m_ws->activeWindow());
	if (editor)
		editor->addToPerforce();
}

// ----------------------------------------------------------------------

void MainWindow::compile()
{
	QuestEditor * editor = dynamic_cast<QuestEditor *>(m_ws->activeWindow());
	if (editor)
		editor->compile();
}

// ----------------------------------------------------------------------

void MainWindow::buildQuestCrcStringTables()
{
	char const * const defaultBranch = ConfigFile::getKeyString(cs_questEditorSection, cs_defaultBranch, "");

	getToolProcess()->buildQuestCrcStringTables(defaultBranch);
}

// ----------------------------------------------------------------------

void MainWindow::consoleDoubleClicked(int para, int)
{
	m_console->setSelection(para, 0, para, 9999);

	// filename (location) : message
	QRegExp messageExp("^([^(]*) \\((.*)\\) : (.*)$");

	if (messageExp.search(m_console->text(para)) != -1)
	{
		DEBUG_REPORT_LOG(false, ("%s\n", messageExp.cap(1).ascii()));
		DEBUG_REPORT_LOG(false, ("%s\n", messageExp.cap(2).ascii()));
		DEBUG_REPORT_LOG(false, ("%s\n", messageExp.cap(3).ascii()));

		QuestEditor * editor = getQuestEditor(messageExp.cap(1));

		if (editor)
		{
			editor->setFocus();

			QStringList locationParts = QStringList::split(':', messageExp.cap(2));

			if (locationParts[0] == "TASK")
			{
				editor->selectTaskField(locationParts[1], locationParts[3]);
			}
			else if (locationParts[0] == "LIST")
			{
				editor->selectListField(locationParts[1]);
			}
		}

		statusBar()->message(messageExp.cap(3));
	}
}

// ----------------------------------------------------------------------

QuestEditor * MainWindow::getQuestEditor(QString const & filename)
{
	QWidgetList windows = m_ws->windowList();
	if (windows.count())
	{
		for (int i = 0; i < int(windows.count()); ++i)
		{
			QuestEditor * editor = dynamic_cast<QuestEditor * >(windows.at(i));
			if (editor && editor->getFileName() == filename)
			{
				return editor;
			}
		}
	}

	return NULL;
}

// ======================================================================
