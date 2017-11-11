// =====================================================================
//
// MainFrame.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// =====================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "MainFrame.h"
#include "MainFrame.moc"

#include "sharedFoundation/Os.h"

#include "ActionHack.h"
#include "ActionsEdit.h"
#include "ActionsFile.h"
#include "ActionsGame.h"
#include "ActionsObjectTemplate.h"
#include "ActionsScript.h"
#include "ActionsTool.h"
#include "ActionsView.h"
#include "ActionsWindow.h"
#include "BookmarkBrowser.h"
#include "BookmarkData.h"
#include "BrushData.h"
#include "ConsoleWindow.h"
#include "FavoritesWindow.h"
#include "FilterManager.h"
#include "FilterWindow.h"
#include "GameWidget.h"
#include "GameWindow.h"
#include "GodClientData.h"
#include "GodClientPerforce.h"
#include "GroupObjectWindow.h"
#include "ModificationHistory.h"
#include "ObjectEditor.h"
#include "ObjectTemplateData.h"
#include "RegionBrowser.h"
#include "ServerCommander.h"
#include "ServerObjectData.h"
#include "TreeBrowser.h"

#include <qaction.h>
#include <qapplication.h>
#include <qdockwindow.h>
#include <qlayout.h>
#include <qlcdnumber.h>
#include <qmenubar.h>
#include <qpushbutton.h>
#include <qsettings.h>
#include <qstatusbar.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qworkspace.h>

#include <cassert>

// =====================================================================

namespace MainFrameNamespace
{
	int const c_WindowStateVersion = 1;
}

using namespace MainFrameNamespace;

//----------------------------------------------------------------------

//static definitions
MainFrame * MainFrame::ms_singleton;

//----------------------------------------------------------------------

MainFrame::MainFrame(QWidget *theParent, const char *theName)
: QMainWindow(theParent, theName),
  m_menus(),
  m_workspace(0),
  m_gameWindow(0),
  m_console(0),
  m_treeBrowser(0),
  m_objectEditor(0),
  m_groupObjectWindow(0),
  m_filterWindow(0),
  m_bookmarkBrowser(0),
  m_favoritesWindow(0),
  m_consoleDock(0),
  m_treeBrowserDock(0),
  m_objectEditorDock(0),
  m_groupObjectWindowDock(0),
  m_filterWindowDock(0),
  m_bookmarkBrowserDock(0),
  m_regionsViewDock(0),
  m_favoritesWindowDock(0),
  m_settings(0),
  m_actionsGame(0),
  m_actionsEdit(0),
  m_actionsView(0),
  m_actionsFile(0),
  m_actionsScript(0),
  m_actionsObjectTemplate(0),
  m_actionsTool(0),
  m_actionsWindow(0),
  m_regionsView(0),
  m_active(false),
  m_windowSettingsRestored(false)
{
	QMainWindow::setFocusPolicy(QWidget::StrongFocus);

	//get the settings set up
	m_settings = new QSettings;
	m_settings->insertSearchPath(QSettings::Windows, "/SOE/SwgGodClient");

	DEBUG_FATAL(ms_singleton,("Multiple mainframes!\n"));
	ms_singleton = this;

	QMainWindow::setDockMenuEnabled(true);

	//initialize static singletons
	FilterManager::install();

	//-- setup the windows and gamewindow first
	{
		m_workspace = new QWorkspace(this, "Game Window Workspace");
		m_workspace->setBackgroundColor(palette().normal().dark());
		m_workspace->setCaption(m_workspace->name());
		m_gameWindow = new GameWindow(m_workspace, "Game Window");
		QMainWindow::setCentralWidget(m_workspace);
		m_gameWindow->showMaximized();
//		loadWindowSettings(m_settings, m_gameWindow);

		statusBar()->setSizeGripEnabled(true);
		statusBar()->message("Ready");

		QMainWindow::setDockEnabled(Qt::Top,    true);
		QMainWindow::setDockEnabled(Qt::Bottom, true);
		QMainWindow::setDockEnabled(Qt::Left,   true);
		QMainWindow::setDockEnabled(Qt::Right,  true);

		m_consoleDock = new QDockWindow(QDockWindow::InDock, this, "Console Window");
		m_console     = new ConsoleWindow(m_consoleDock, "Console Widget");
		m_consoleDock->setWidget(m_console);
		m_consoleDock->setResizeEnabled(true);
		QMainWindow::addDockWindow(m_consoleDock, Qt::Bottom);
		m_consoleDock->setCloseMode(QDockWindow::Always);
//		loadDockWindowSettings(m_settings, m_consoleDock);


		m_filterWindowDock = new QDockWindow(QDockWindow::InDock, this, "Filter Window");
		m_filterWindow     = new FilterWindow(m_filterWindowDock, "Filter Widget");
		m_filterWindowDock->setWidget(m_filterWindow);
		m_filterWindowDock->setResizeEnabled(true);
		QMainWindow::addDockWindow(m_filterWindowDock, Qt::Right);
		m_filterWindowDock->setCloseMode(QDockWindow::Always);
//		loadDockWindowSettings(m_settings, m_filterWindowDock);

		m_treeBrowserDock = new QDockWindow(QDockWindow::InDock, this, "TreeBrowser Window");
		m_treeBrowser     = new TreeBrowser(m_treeBrowserDock, "treeBrowser Widget");
		m_treeBrowserDock->setWidget(m_treeBrowser);
		m_treeBrowserDock->setResizeEnabled(true);
		QMainWindow::addDockWindow(m_treeBrowserDock, Qt::Left);
		m_treeBrowserDock->setCloseMode(QDockWindow::Always);
//		loadDockWindowSettings(m_settings, m_treeBrowserDock);

		m_objectEditorDock = new QDockWindow(QDockWindow::InDock, this, "ObjectEditor Window");
		m_objectEditor     = new ObjectEditor(m_objectEditorDock, "objectEditor Widget");
		m_objectEditorDock->setWidget(m_objectEditor);
		m_objectEditorDock->setResizeEnabled(true);
		QMainWindow::addDockWindow(m_objectEditorDock, Qt::Left);
		m_objectEditorDock->setCloseMode(QDockWindow::Always);
//		loadDockWindowSettings(m_settings, m_objectEditorDock);

		m_bookmarkBrowserDock = new QDockWindow(QDockWindow::InDock, this, "BookmarkBrowser Window");
		m_bookmarkBrowser     = new BookmarkBrowser(m_bookmarkBrowserDock, "BookmarkBrowser Widget");
		m_bookmarkBrowserDock->setWidget(m_bookmarkBrowser);
		m_bookmarkBrowserDock->setResizeEnabled(true);
		QMainWindow::addDockWindow(m_bookmarkBrowserDock, Qt::Left);
		m_bookmarkBrowserDock->setCloseMode(QDockWindow::Always);
//		loadDockWindowSettings(m_settings, m_bookmarkBrowserDock);

		m_favoritesWindowDock = new QDockWindow(QDockWindow::InDock, this, "FavoritesWindow Window");
		m_favoritesWindow     = new FavoritesWindow(m_favoritesWindowDock, "FavoritesWindow Widget");
		m_favoritesWindowDock->setWidget(m_favoritesWindow);
		m_favoritesWindowDock->setResizeEnabled(true);
		QMainWindow::addDockWindow(m_favoritesWindowDock, Qt::Left);
		m_favoritesWindowDock->setCloseMode(QDockWindow::Always);

		m_groupObjectWindowDock = new QDockWindow(QDockWindow::InDock, this, "GroupObject Window");
		m_groupObjectWindow     = new GroupObjectWindow(m_groupObjectWindowDock, "GroupObject Widget");
		m_groupObjectWindowDock->setWidget(m_groupObjectWindow);
		m_groupObjectWindowDock->setResizeEnabled(true);
		QMainWindow::addDockWindow(m_groupObjectWindowDock, Qt::Left);
		m_groupObjectWindowDock->setCloseMode(QDockWindow::Always);
		m_groupObjectWindowDock->hide();
//		loadDockWindowSettings(m_settings, m_groupObjectWindowDock);

		m_regionsViewDock = new QDockWindow(QDockWindow::InDock, this, "Region Viewer");
		m_regionsView = new RegionBrowser(m_regionsViewDock, "Region Viewer Widget");
		m_regionsViewDock->setWidget(m_regionsView);
		m_regionsViewDock->setResizeEnabled(true);
		m_regionsViewDock->resize(QSize(800,600));
		QMainWindow::addDockWindow(m_regionsViewDock, Qt::TornOff);
		m_regionsViewDock->setCloseMode(QDockWindow::Always);
		m_regionsViewDock->hide();

		//don't show the dock window list on right click (VERY annoying for some windows like the region viewer)
		setDockMenuEnabled (false);

		m_actionsGame           = &ActionsGame::getInstance();
		m_actionsEdit           = &ActionsEdit::getInstance();
		m_actionsView           = &ActionsView::getInstance();
		m_actionsFile           = &ActionsFile::getInstance();
		m_actionsScript         = &ActionsScript::getInstance();
		m_actionsObjectTemplate = &ActionsObjectTemplate::getInstance();
		m_actionsTool           = &ActionsTool::getInstance();
		m_actionsWindow         = &ActionsWindow::getInstance();

		IGNORE_RETURN(connect(m_treeBrowser,     SIGNAL(objectDoubleClickedSignal(const Object&, bool, int)), m_actionsView, SLOT(centerCameraOnObject(const Object &, bool, int))));
	}

	//-- setup the toolbar
	{
		QToolBar* m_toolbar_edit = new QToolBar("Edit", this);
		{

			IGNORE_RETURN(m_actionsEdit->undo->addTo(m_toolbar_edit));
			IGNORE_RETURN(m_actionsEdit->redo->addTo(m_toolbar_edit));
			IGNORE_RETURN(m_actionsEdit->cut->addTo  (m_toolbar_edit));
			IGNORE_RETURN(m_actionsEdit->copy->addTo (m_toolbar_edit));
			IGNORE_RETURN(m_actionsEdit->paste->addTo(m_toolbar_edit));
			IGNORE_RETURN(m_actionsEdit->del->addTo  (m_toolbar_edit));
			IGNORE_RETURN(m_actionsEdit->applyTransform->addTo(m_toolbar_edit));
			IGNORE_RETURN(m_actionsEdit->dropToTerrain->addTo (m_toolbar_edit));
			IGNORE_RETURN(m_actionsEdit->alignToTerrain->addTo(m_toolbar_edit));
			IGNORE_RETURN(m_actionsEdit->rotateReset->addTo   (m_toolbar_edit));
			IGNORE_RETURN(m_actionsEdit->rotateModeGroup->addTo     (m_toolbar_edit));
			IGNORE_RETURN(m_actionsEdit->rotatePivotModeGroup->addTo(m_toolbar_edit));
			IGNORE_RETURN(m_actionsEdit->rotateIncrement45->addTo(m_toolbar_edit));
			IGNORE_RETURN(m_actionsEdit->rotateDecrement45->addTo(m_toolbar_edit));
			IGNORE_RETURN(m_actionsEdit->randomRotate->addTo     (m_toolbar_edit));
			IGNORE_RETURN(m_actionsEdit->createObjectFromSelectedTemplate->addTo(m_toolbar_edit));
			IGNORE_RETURN(m_actionsEdit->m_toggleDropToTerrain->addTo (m_toolbar_edit));
			IGNORE_RETURN(m_actionsEdit->m_toggleAlignToTerrain->addTo (m_toolbar_edit));
		} 
		
		QToolBar* m_toolbar_game = new QToolBar("Game", this);
		{
			IGNORE_RETURN(m_actionsGame->hud->addTo             (m_toolbar_game));
			IGNORE_RETURN(m_actionsGame->gameFocusAllowed->addTo(m_toolbar_game));
			IGNORE_RETURN(m_actionsGame->console->addTo         (m_toolbar_game));
		}
		
		QToolBar* m_toolbar_file = new QToolBar("File", this);
		{
			UNREF(m_toolbar_file);
		}
		
		QToolBar* m_toolbar_view = new QToolBar("View", this);
		{
			IGNORE_RETURN(m_actionsView->addCameraBookmark->addTo   (m_toolbar_view));
			IGNORE_RETURN(m_actionsView->addObjectBookmark->addTo   (m_toolbar_view));
			IGNORE_RETURN(m_actionsView->removeObjectBookmark->addTo(m_toolbar_view));
			IGNORE_RETURN(m_actionsView->centerSelection->addTo     (m_toolbar_view));
			IGNORE_RETURN(m_actionsView->centerGhosts->addTo        (m_toolbar_view));
			IGNORE_RETURN(m_actionsView->fitSelection->addTo        (m_toolbar_view));
			IGNORE_RETURN(m_actionsView->fitGhosts->addTo           (m_toolbar_view));
		}
		
		QToolBar* m_toolbar_script = new QToolBar("Script", this);
		{
			IGNORE_RETURN(m_actionsScript->refresh->addTo     (m_toolbar_script));
			IGNORE_RETURN(m_actionsScript->create->addTo      (m_toolbar_script));
			IGNORE_RETURN(m_actionsScript->edit->addTo        (m_toolbar_script));
			IGNORE_RETURN(m_actionsScript->view->addTo        (m_toolbar_script));
			IGNORE_RETURN(m_actionsScript->revert->addTo      (m_toolbar_script));
			IGNORE_RETURN(m_actionsScript->submit->addTo      (m_toolbar_script));
			IGNORE_RETURN(m_actionsScript->compile->addTo     (m_toolbar_script));
			IGNORE_RETURN(m_actionsScript->serverReload->addTo(m_toolbar_script));
		}

		QToolBar* m_toolbar_serverObjectTemplate = new QToolBar("ServerObjectTemplate", this);
		{
			IGNORE_RETURN(m_actionsObjectTemplate->m_serverRefresh->addTo(m_toolbar_serverObjectTemplate));
			IGNORE_RETURN(m_actionsObjectTemplate->m_serverCreate->addTo (m_toolbar_serverObjectTemplate));
			IGNORE_RETURN(m_actionsObjectTemplate->m_serverEdit->addTo   (m_toolbar_serverObjectTemplate));
			IGNORE_RETURN(m_actionsObjectTemplate->m_serverView->addTo   (m_toolbar_serverObjectTemplate));
			IGNORE_RETURN(m_actionsObjectTemplate->m_serverRevert->addTo (m_toolbar_serverObjectTemplate));
			IGNORE_RETURN(m_actionsObjectTemplate->m_serverSubmit->addTo (m_toolbar_serverObjectTemplate));
			IGNORE_RETURN(m_actionsObjectTemplate->m_serverCompile->addTo(m_toolbar_serverObjectTemplate));
			IGNORE_RETURN(m_actionsObjectTemplate->m_serverReload->addTo (m_toolbar_serverObjectTemplate));
			IGNORE_RETURN(m_actionsObjectTemplate->m_serverReplace->addTo(m_toolbar_serverObjectTemplate));
		}

		QToolBar* m_toolbar_clientObjectTemplate = new QToolBar("ClientObjectTemplate", this);
		{
			IGNORE_RETURN(m_actionsObjectTemplate->m_clientRefresh->addTo(m_toolbar_clientObjectTemplate));
			IGNORE_RETURN(m_actionsObjectTemplate->m_clientCreate->addTo (m_toolbar_clientObjectTemplate));
			IGNORE_RETURN(m_actionsObjectTemplate->m_clientEdit->addTo   (m_toolbar_clientObjectTemplate));
			IGNORE_RETURN(m_actionsObjectTemplate->m_clientView->addTo   (m_toolbar_clientObjectTemplate));
			IGNORE_RETURN(m_actionsObjectTemplate->m_clientRevert->addTo (m_toolbar_clientObjectTemplate));
			IGNORE_RETURN(m_actionsObjectTemplate->m_clientSubmit->addTo (m_toolbar_clientObjectTemplate));
			IGNORE_RETURN(m_actionsObjectTemplate->m_clientCompile->addTo(m_toolbar_clientObjectTemplate));
			IGNORE_RETURN(m_actionsObjectTemplate->m_clientReplace->addTo(m_toolbar_clientObjectTemplate));
		}

	}//end toolbar setup

	//-- setup the menuBar
	{
		IGNORE_RETURN(menuBar()->insertItem("&File",           &m_menus.file.menu));
		IGNORE_RETURN(menuBar()->insertItem("&Edit",           &m_menus.edit.menu));
		IGNORE_RETURN(menuBar()->insertItem("&View",           &m_menus.view.menu));
		IGNORE_RETURN(menuBar()->insertItem("&Game",           &m_menus.game.menu));
		IGNORE_RETURN(menuBar()->insertItem("&Script",         &m_menus.script.menu));
		IGNORE_RETURN(menuBar()->insertItem("&ObjectTemplate", &m_menus.objectTemplate.menu));
		IGNORE_RETURN(menuBar()->insertItem("&Tools",          &m_menus.tool.menu));
		IGNORE_RETURN(menuBar()->insertItem("&Window",         &m_menus.window.menu));
		IGNORE_RETURN(menuBar()->insertItem("&Help",           &m_menus.help.menu));
		
		//-- File menu stuff
		{
			QPopupMenu * fileMenu = &m_menus.file.menu;
			IGNORE_RETURN(m_actionsFile->exit->addTo(fileMenu));
		}
		
		//-- View menu stuff
		{
			int tmp_id;
			
			IGNORE_RETURN(m_actionsView->addCameraBookmark->addTo   (&m_menus.view.menu));
			IGNORE_RETURN(m_actionsView->addObjectBookmark->addTo   (&m_menus.view.menu));
			IGNORE_RETURN(m_actionsView->removeObjectBookmark->addTo(&m_menus.view.menu));
			
			tmp_id = m_menus.view.menu.insertItem ("&Bookmarks",  &m_menus.view.bookmarksMenu);
			m_menus.view.menu.setWhatsThis(tmp_id, "3D Viewing Bookmarks");
			IGNORE_RETURN(connect(&m_menus.view.bookmarksMenu, SIGNAL(aboutToShow()), this, SLOT(updateBookmarksMenu())));
			
			IGNORE_RETURN(m_menus.view.menu.insertSeparator());
			IGNORE_RETURN(m_actionsView->centerSelection->addTo     (&m_menus.view.menu));
			IGNORE_RETURN(m_actionsView->centerGhosts->addTo        (&m_menus.view.menu));
			IGNORE_RETURN(m_actionsView->fitSelection->addTo        (&m_menus.view.menu));
			IGNORE_RETURN(m_actionsView->fitGhosts->addTo           (&m_menus.view.menu));
			IGNORE_RETURN(m_actionsView->showTriggerVolumes->addTo  (&m_menus.view.menu));
			IGNORE_RETURN(m_actionsTool->m_getSphereTree->addTo     (&m_menus.view.menu));
			IGNORE_RETURN(m_actionsView->turnOffSpheres->addTo      (&m_menus.view.menu));
			IGNORE_RETURN(m_actionsView->resetCamera->addTo         (&m_menus.view.menu));
		}
		
		//-- Edit menu stuff
		{
			m_menus.edit.menu.setCheckable(true);

			IGNORE_RETURN(m_actionsEdit->undo->addTo(&m_menus.edit.menu));
			IGNORE_RETURN(m_actionsEdit->redo->addTo(&m_menus.edit.menu));

			IGNORE_RETURN(m_menus.edit.menu.insertSeparator());

			IGNORE_RETURN(m_actionsEdit->copy->addTo (&m_menus.edit.menu));
			IGNORE_RETURN(m_actionsEdit->cut->addTo  (&m_menus.edit.menu));
			IGNORE_RETURN(m_actionsEdit->paste->addTo(&m_menus.edit.menu));
			IGNORE_RETURN(m_actionsEdit->del->addTo  (&m_menus.edit.menu));
			
			IGNORE_RETURN(m_menus.edit.menu.insertSeparator());
			
			IGNORE_RETURN(m_actionsEdit->applyTransform->addTo(&m_menus.edit.menu));
			IGNORE_RETURN(m_actionsEdit->dropToTerrain->addTo (&m_menus.edit.menu));
			IGNORE_RETURN(m_actionsEdit->alignToTerrain->addTo(&m_menus.edit.menu));
	
			IGNORE_RETURN(m_menus.edit.menu.insertSeparator());

			IGNORE_RETURN(m_actionsEdit->rotateReset->addTo   (&m_menus.edit.menu));

			IGNORE_RETURN(m_menus.edit.menu.insertSeparator());

			IGNORE_RETURN(m_actionsEdit->rotateModeGroup->addTo     (&m_menus.edit.menu));
			IGNORE_RETURN(m_actionsEdit->rotatePivotModeGroup->addTo(&m_menus.edit.menu));

			IGNORE_RETURN(m_menus.edit.menu.insertSeparator());

			IGNORE_RETURN(m_actionsEdit->rotateIncrement45->addTo               (&m_menus.edit.menu));
			IGNORE_RETURN(m_actionsEdit->rotateDecrement45->addTo               (&m_menus.edit.menu));
			IGNORE_RETURN(m_actionsEdit->randomRotate->addTo                    (&m_menus.edit.menu));
			IGNORE_RETURN(m_actionsEdit->setTransform->addTo                    (&m_menus.edit.menu));
			IGNORE_RETURN(m_actionsEdit->createObjectFromSelectedTemplate->addTo(&m_menus.edit.menu));
			IGNORE_RETURN(m_actionsEdit->copyForPOI->addTo                      (&m_menus.edit.menu));
			IGNORE_RETURN(m_actionsEdit->createTheater->addTo (&m_menus.edit.menu));
			IGNORE_RETURN(m_actionsEdit->createClientDataFile->addTo (&m_menus.edit.menu));

			IGNORE_RETURN(m_actionsEdit->m_toggleDropToTerrain->addTo (&m_menus.edit.menu));
			IGNORE_RETURN(m_actionsEdit->m_toggleAlignToTerrain->addTo (&m_menus.edit.menu));
		}
		//-- Game menu stuff
		{
			ActionsGame* const ga = &ActionsGame::getInstance();
			IGNORE_RETURN(connect(&m_menus.game.menu, SIGNAL(aboutToShow()), this, SLOT(updateGameMenu())));
			m_menus.game.menu.setCheckable(true);

			IGNORE_RETURN(ga->gameFocusAllowed->addTo  (&m_menus.game.menu));
			IGNORE_RETURN(ga->hud->addTo               (&m_menus.game.menu));
			IGNORE_RETURN(m_actionsGame->console->addTo(&m_menus.game.menu));

			IGNORE_RETURN(m_menus.game.menu.insertSeparator());

			IGNORE_RETURN(ga->renderWireframe->addTo     (&m_menus.game.menu));
			IGNORE_RETURN(ga->renderTextured->addTo      (&m_menus.game.menu));
			IGNORE_RETURN(ga->showObjectNames->addTo     (&m_menus.game.menu));
			IGNORE_RETURN(ga->showObjectNetworkIds->addTo(&m_menus.game.menu));

			//this function is only available in the game from a Debug build, so only show it then
#ifdef _DEBUG
				IGNORE_RETURN(ga->showDebugMenu->addTo       (&m_menus.game.menu));
#endif

			IGNORE_RETURN(m_menus.game.menu.insertSeparator());

			QPopupMenu* m_terrainMenu = new QPopupMenu(this);
			IGNORE_RETURN(m_menus.game.menu.insertItem("&Terrain", m_terrainMenu));

			{
				IGNORE_RETURN(ga->purgeTerrain->addTo            (m_terrainMenu));
				IGNORE_RETURN(ga->reloadTerrain->addTo           (m_terrainMenu));
			}

			IGNORE_RETURN(m_terrainMenu->insertSeparator());

			IGNORE_RETURN(ga->m_saveInteriorLayout->addTo(&m_menus.tool.menu));
		}

		//-- Tool menu stuff
		{
			IGNORE_RETURN(m_actionsTool->m_saveAsBrush->addTo                     (&m_menus.tool.menu));
			IGNORE_RETURN(m_actionsTool->m_snapToGrid->addTo                      (&m_menus.tool.menu));
			IGNORE_RETURN(m_actionsTool->m_sendSystemMessage->addTo               (&m_menus.tool.menu));
		}

		//-- Window menu stuff
		{
			IGNORE_RETURN(m_actionsWindow->m_treeBrowser->addTo      (&m_menus.window.menu));
			IGNORE_RETURN(m_actionsWindow->m_objectEditor->addTo     (&m_menus.window.menu));
			IGNORE_RETURN(m_actionsWindow->m_bookmarks->addTo        (&m_menus.window.menu));
			IGNORE_RETURN(m_actionsWindow->m_favorites->addTo        (&m_menus.window.menu));
			IGNORE_RETURN(m_actionsWindow->m_groupObjectEditor->addTo(&m_menus.window.menu));
			IGNORE_RETURN(m_actionsWindow->m_filters->addTo          (&m_menus.window.menu));
			IGNORE_RETURN(m_actionsWindow->m_console->addTo          (&m_menus.window.menu));
			IGNORE_RETURN(m_actionsWindow->m_gameWindow->addTo       (&m_menus.window.menu));
			IGNORE_RETURN(m_actionsWindow->m_regionsView->addTo      (&m_menus.window.menu));
		}

		//-- Script menu stuff
		{
			IGNORE_RETURN(m_actionsScript->refresh->addTo(&m_menus.script.menu));

			IGNORE_RETURN(m_menus.script.menu.insertSeparator());

			IGNORE_RETURN(m_actionsScript->create->addTo(&m_menus.script.menu));
			IGNORE_RETURN(m_actionsScript->edit->addTo  (&m_menus.script.menu));
			IGNORE_RETURN(m_actionsScript->view->addTo  (&m_menus.script.menu));
			IGNORE_RETURN(m_actionsScript->revert->addTo(&m_menus.script.menu));
			IGNORE_RETURN(m_actionsScript->submit->addTo(&m_menus.script.menu));

			IGNORE_RETURN(m_menus.script.menu.insertSeparator());

			IGNORE_RETURN(m_actionsScript->compile->addTo     (&m_menus.script.menu));
			IGNORE_RETURN(m_actionsScript->serverReload->addTo(&m_menus.script.menu));
		}

		//Server ObjectTemplate menu stuff
		QPopupMenu* m_serverObjectTemplateMenu = new QPopupMenu(this);
		IGNORE_RETURN(m_menus.objectTemplate.menu.insertItem("&Server", m_serverObjectTemplateMenu));
		{
			IGNORE_RETURN(m_actionsObjectTemplate->m_serverRefresh->addTo(m_serverObjectTemplateMenu));

			IGNORE_RETURN(m_menus.script.menu.insertSeparator());

			IGNORE_RETURN(m_actionsObjectTemplate->m_serverCreate->addTo(m_serverObjectTemplateMenu));
			IGNORE_RETURN(m_actionsObjectTemplate->m_serverEdit->addTo  (m_serverObjectTemplateMenu));
			IGNORE_RETURN(m_actionsObjectTemplate->m_serverView->addTo  (m_serverObjectTemplateMenu));
			IGNORE_RETURN(m_actionsObjectTemplate->m_serverRevert->addTo(m_serverObjectTemplateMenu));
			IGNORE_RETURN(m_actionsObjectTemplate->m_serverSubmit->addTo(m_serverObjectTemplateMenu));

			IGNORE_RETURN(m_menus.objectTemplate.menu.insertSeparator());

			IGNORE_RETURN(m_actionsObjectTemplate->m_serverCompile->addTo(m_serverObjectTemplateMenu));
			IGNORE_RETURN(m_actionsObjectTemplate->m_serverReload->addTo (m_serverObjectTemplateMenu));
		}

		//Client ObjectTemplate menu stuff
		QPopupMenu* m_clientObjectTemplateMenu = new QPopupMenu(this);
		IGNORE_RETURN(m_menus.objectTemplate.menu.insertItem("&Client", m_clientObjectTemplateMenu));
		{
			IGNORE_RETURN(m_actionsObjectTemplate->m_clientRefresh->addTo(m_clientObjectTemplateMenu));

			IGNORE_RETURN(m_menus.script.menu.insertSeparator());

			IGNORE_RETURN(m_actionsObjectTemplate->m_clientCreate->addTo(m_clientObjectTemplateMenu));
			IGNORE_RETURN(m_actionsObjectTemplate->m_clientEdit->addTo  (m_clientObjectTemplateMenu));
			IGNORE_RETURN(m_actionsObjectTemplate->m_clientView->addTo  (m_clientObjectTemplateMenu));
			IGNORE_RETURN(m_actionsObjectTemplate->m_clientRevert->addTo(m_clientObjectTemplateMenu));
			IGNORE_RETURN(m_actionsObjectTemplate->m_clientSubmit->addTo(m_clientObjectTemplateMenu));

			IGNORE_RETURN(m_menus.objectTemplate.menu.insertSeparator());

			IGNORE_RETURN(m_actionsObjectTemplate->m_clientCompile->addTo(m_clientObjectTemplateMenu));
			IGNORE_RETURN(m_actionsObjectTemplate->m_clientReplace->addTo(m_clientObjectTemplateMenu));
		}


	}//end menubar setup

	m_workspace->show();
//	m_gameWindow->showMaximized();

	//load the brushes from disk
	BrushData::getInstance().unserialize();

	//-- force a frame update so we can update the camera
	m_gameWindow->m_gameWidget->runGameLoop();//paintEvent(0);

	GodClientData* gcd = &GodClientData::getInstance();
	gcd->initialize();

	//get the objects into the treebrowser
	m_treeBrowser->refreshObjects();
}

//----------------------------------------------------------------------

MainFrame::~MainFrame()
{
	//the QSettings isn't owned by the Qt app, we have to delete it ourselves
	if(m_settings)
		delete m_settings;

	NOT_NULL(ms_singleton);
	ms_singleton = 0;
	
	m_consoleDock           = 0;
	m_console               = 0;
	m_workspace             = 0;
	m_gameWindow            = 0;
	m_treeBrowserDock       = 0;
	m_treeBrowser           = 0;
	m_objectEditorDock      = 0;
	m_groupObjectWindow     = 0;
	m_groupObjectWindowDock = 0;
	m_objectEditor          = 0;
	m_bookmarkBrowser       = 0;
	m_bookmarkBrowserDock   = 0;
	m_favoritesWindow       = 0;
	m_favoritesWindowDock   = 0;
	m_regionsViewDock       = 0;
	m_actionsGame           = 0;
	m_actionsEdit           = 0;
	m_actionsView           = 0;
	m_actionsFile           = 0;
	m_actionsScript         = 0;
	m_actionsObjectTemplate = 0;
	m_settings              = 0;
} //lint !e1740 member pointers not freed in destructor, this is because Qt "owns" them and deals with it

//----------------------------------------------------------------------

void MainFrame::updateBookmarksMenu()
{
	const BookmarkData & bd = BookmarkData::getInstance();

	m_menus.view.bookmarksMenu.clear();

	const size_t numCam = bd.getNumCameraBookmarks();

	REPORT_LOG_PRINT(true,("num cams=%d\n", numCam));

	IGNORE_RETURN(m_menus.view.bookmarksMenu.insertItem(new QLabel("Camera Positions", &m_menus.view.bookmarksMenu, "CameraPositionsLabel")));
	IGNORE_RETURN(m_menus.view.bookmarksMenu.insertSeparator());
	
	if(numCam)
	{

		for(size_t i = 0; i < numCam; ++i)
		{
			const BookmarkData::CameraBookmark & cam = bd.getCameraBookmark(i);
			IGNORE_RETURN(m_menus.view.bookmarksMenu.insertItem(cam.name.c_str(), &ActionsView::getInstance(), SLOT(gotoCameraBookmark(int)), 0, static_cast<int>(i)));
		}
	}

	IGNORE_RETURN(m_menus.view.bookmarksMenu.insertItem(new QLabel("Object Bookmarks", &m_menus.view.bookmarksMenu, "ObjectBookmarksLabel")));
	IGNORE_RETURN(m_menus.view.bookmarksMenu.insertSeparator());

	const size_t numObj = bd.getNumObjectBookmarks();
	if(numObj)
	{

		for(size_t i = 0; i < numObj; ++i)
		{
			const BookmarkData::ObjectBookmark & obj = bd.getObjectBookmark(i);
			IGNORE_RETURN(m_menus.view.bookmarksMenu.insertItem(obj.cameraBookmark.name.c_str(), &ActionsView::getInstance(), SLOT(gotoObjectBookmark(int)), 0, static_cast<int>(i)));
		}
	}
}


//----------------------------------------------------------------------

void MainFrame::onFileExit()
{
	IGNORE_RETURN(close());
}

//----------------------------------------------------------------------

void MainFrame::updateGameMenu()
{
	bool enabled = false;
	const bool result = m_actionsGame->getHUDEnabled(enabled);

	m_menus.game.menu.setItemChecked(m_menus.game.toggleHUDId, enabled);
	m_menus.game.menu.setItemEnabled(m_menus.game.toggleHUDId, result);
}

//----------------------------------------------------------------------

void MainFrame::closeEvent(QCloseEvent* e)
{
	//-- clean up house (prevents a crash on exit if a ghost exists)
	GodClientData::getInstance().killGhosts();
	GodClientData::getInstance().clearSelection();

	//-- save all settings before closing
	saveWindowSettings(m_settings, this);
	saveWindowSettings(m_settings, m_gameWindow);

	saveDockWindowSettings(m_settings, m_groupObjectWindowDock);
	saveDockWindowSettings(m_settings, m_filterWindowDock);
	saveDockWindowSettings(m_settings, m_treeBrowserDock);
	saveDockWindowSettings(m_settings, m_objectEditorDock);
	saveDockWindowSettings(m_settings, m_bookmarkBrowserDock);
	saveDockWindowSettings(m_settings, m_favoritesWindowDock);
	saveDockWindowSettings(m_settings, m_consoleDock);

	e->accept();
}

//----------------------------------------------------------------------

void MainFrame::focusOutEvent(QFocusEvent*)
{
	REPORT_LOG_PRINT(true,("focus out.\n"));
}

//----------------------------------------------------------------------

bool MainFrame::eventFilter(QObject*, QEvent *ev)
{
	switch(ev->type())
	{
	case QEvent::WindowActivate:
		m_active = true;
		break;
	case QEvent::WindowDeactivate:
		m_active = false;
		break;
	default:
		break;
	} //lint !e788 not all event types explicitly handled (but the default case covers it)
	return false;
} //lint !e818 ev could be const since we don't modify it, but don't change signature of overridden Qt function

// ======================================================================

void MainFrame::loadWindowSettings(QSettings* settings, QWidget* widget, bool move) const
{
	NOT_NULL(settings);
	NOT_NULL(widget);

	std::string entry = "/WindowState/";
	entry += widget->name();

	const std::string xEntry          = entry + "/x";
	const std::string yEntry          = entry + "/y";
	const std::string widthEntry      = entry + "/width";
	const std::string heightEntry     = entry + "/height";
	const std::string maximizedEntry  = entry + "/maximized";
	const std::string visibleEntry    = entry + "/visible";
	const std::string windowStateVersion = entry + "/version";

	const int x          = settings->readNumEntry (xEntry.c_str(),         widget->pos().x());
	const int y          = settings->readNumEntry (yEntry.c_str(),         widget->pos().y());
	const int width      = settings->readNumEntry (widthEntry.c_str(),     widget->size().width());
	const int height     = settings->readNumEntry (heightEntry.c_str(),    widget->size().height());
	const bool maximized = settings->readBoolEntry(maximizedEntry.c_str(), widget->isMaximized());
	const bool visible   = settings->readBoolEntry(visibleEntry.c_str(),   widget->isVisible());
	const int version = settings->readNumEntry(windowStateVersion.c_str(), 0);

	//-- ignore entries 
	if (version != c_WindowStateVersion)
		return;

	//-- only resize the window if it is not maximized, otherwise the window has problems
	if(maximized)
		widget->showMaximized();
	else
	{
		widget->showNormal();

		if (move)
		{
			widget->move(x,y);
			widget->resize(width, height);
		}
	}

	if(visible)
		widget->show();
	else
		widget->hide();
}

// ======================================================================

void MainFrame::saveWindowSettings(QSettings* settings, const QWidget* widget) const
{ 
	NOT_NULL(settings);
	NOT_NULL(widget);

	QWidget const *parent = (widget->parentWidget() == NULL) ? widget : widget->parentWidget();

	std::string entry = "/WindowState/";
	entry += widget->name();

	const std::string xEntry          = entry + "/x";
	const std::string yEntry          = entry + "/y";
	const std::string widthEntry      = entry + "/width";
	const std::string heightEntry     = entry + "/height";
	const std::string maximizedEntry  = entry + "/maximized";
	const std::string visibleEntry    = entry + "/visible";
	const std::string windowStateVersion = entry + "/version";

	const int x         = parent->pos().x();
	const int y         = parent->pos().y();
	const int width     = parent->size().width();
	const int height    = parent->size().height();

	const int maximized = widget->isMaximized();
	const bool visible  = widget->isVisible();

	IGNORE_RETURN(settings->writeEntry(xEntry.c_str(),         x));
	IGNORE_RETURN(settings->writeEntry(yEntry.c_str(),         y));
	IGNORE_RETURN(settings->writeEntry(widthEntry.c_str(),     width));
	IGNORE_RETURN(settings->writeEntry(heightEntry.c_str(),    height));
	IGNORE_RETURN(settings->writeEntry(maximizedEntry.c_str(), maximized));
	IGNORE_RETURN(settings->writeEntry(visibleEntry.c_str(),   visible));
	IGNORE_RETURN(settings->writeEntry(windowStateVersion.c_str(), c_WindowStateVersion));
}

// ======================================================================

void MainFrame::loadDockWindowSettings(QSettings* settings, QDockWindow* window)
{
	NOT_NULL(settings);
	NOT_NULL(window);

	std::string entry = "/WindowState/";
	entry += window->name();

	const std::string dockEntry        = entry + "/dock";
	const std::string indexEntry       = entry + "/index";
	const std::string newlineEntry     = entry + "/newline";
	const std::string extraOffsetEntry = entry + "/extraOffset";

	Dock d          = Qt::Left;
	int  i          = 0;
	bool nl         = false;
	int  eo         = 0;
	if(QMainWindow::getLocation(window, d, i, nl, eo))
	{
		const Dock dock       = static_cast<Dock>(settings->readNumEntry(dockEntry.c_str(),       static_cast<int>(d)));
		const int index       = settings->readNumEntry                 (indexEntry.c_str(),       i);
		const bool newLine    = settings->readBoolEntry                (newlineEntry.c_str(),     nl);
		const int extraOffset = settings->readNumEntry                 (extraOffsetEntry.c_str(), eo);

		QMainWindow::moveDockWindow(window, dock, newLine, index, extraOffset);
		window->setResizeEnabled(true);
	}

	const std::string xEntry          = entry + "/x";
	const std::string yEntry          = entry + "/y";
	const std::string widthEntry      = entry + "/width";
	const std::string heightEntry     = entry + "/height";
	const std::string maximizedEntry  = entry + "/maximized";
	const std::string visibleEntry    = entry + "/visible";

	const int x          = settings->readNumEntry (xEntry.c_str(),         window->pos().x());
	const int y          = settings->readNumEntry (yEntry.c_str(),         window->pos().y());
	const int width      = settings->readNumEntry (widthEntry.c_str(),     window->size().width());
	const int height     = settings->readNumEntry (heightEntry.c_str(),    window->size().height());
	const bool maximized = settings->readBoolEntry(maximizedEntry.c_str(), window->isMaximized());
	const bool visible   = settings->readBoolEntry(visibleEntry.c_str(),   window->isVisible());

	UNREF(maximized);

	window->move(x,y);
	window->resize(width, height);
	if(visible)
		window->show();
	else
		window->hide();
//	if(maximized)
//		widget->showMaximized();
}

// ======================================================================

void MainFrame::saveDockWindowSettings(QSettings* settings, QDockWindow* window) const
{ 
	NOT_NULL(settings);
	NOT_NULL(window);

	std::string entry = "/WindowState/";
	entry += window->name();

	Dock dock       = Qt::Left;
	int index       = 0;
	bool nl         = false;
	int extraOffset = 0;
	if (QMainWindow::getLocation(window, dock, index, nl, extraOffset))
	{
		const std::string dockEntry        = entry + "/dock";
		const std::string indexEntry       = entry + "/index";
		const std::string newlineEntry     = entry + "/newline";
		const std::string extraOffsetEntry = entry + "/extraOffset";

		IGNORE_RETURN(settings->writeEntry(dockEntry.c_str(),        dock));
		IGNORE_RETURN(settings->writeEntry(indexEntry.c_str(),       index));
		IGNORE_RETURN(settings->writeEntry(newlineEntry.c_str(),     nl));
		IGNORE_RETURN(settings->writeEntry(extraOffsetEntry.c_str(), extraOffset));
	}

	const std::string xEntry          = entry + "/x";
	const std::string yEntry          = entry + "/y";
	const std::string widthEntry      = entry + "/width";
	const std::string heightEntry     = entry + "/height";
	const std::string maximizedEntry  = entry + "/maximized";
	const std::string visibleEntry    = entry + "/visible";

	const int x         = window->pos().x();
	const int y         = window->pos().y();
	const int width     = window->size().width();
	const int height    = window->size().height();
	const int maximized = window->isMaximized();
	const bool visible  = window->isVisible();

	IGNORE_RETURN(settings->writeEntry(xEntry.c_str(),         x));
	IGNORE_RETURN(settings->writeEntry(yEntry.c_str(),         y));
	IGNORE_RETURN(settings->writeEntry(widthEntry.c_str(),     width));
	IGNORE_RETURN(settings->writeEntry(heightEntry.c_str(),    height));
	IGNORE_RETURN(settings->writeEntry(maximizedEntry.c_str(), maximized));
	IGNORE_RETURN(settings->writeEntry(visibleEntry.c_str(),   visible));
}

// ======================================================================

void MainFrame::textToConsole(const char * text)
{
	DEBUG_FATAL(!m_console, ("can't print text to console, no console object!"));
	if (m_console)
		m_console->onConsoleCommandIssued(text);
}

// ======================================================================

QWorkspace* MainFrame::getWorkspace() const
{
	return m_workspace;
}

// ======================================================================

HWND GodClientApplication::m_gameHWND = 0;

bool GodClientApplication::winEventFilter(MSG* msg)
{
	switch(msg->message)
	{
		case WM_COMMAND:
			return true;
//		if(m_gameHWND)
//		{
//			//emit new WM_COMMAND with the game client's hwnd and other params the same
//			HWND osWindow = Os::getWindow();
//			SendMessage(osWindow, msg->message, msg->wParam, msg->lParam);
//		}
//		break;

		default:
			break;
	}
	return false;
} //lint !e818 msg could be const (no, don't change sig of Qt function)


// ======================================================================

void GodClientApplication::setGameHWND(HWND hwnd)
{
	m_gameHWND = hwnd;
}

// ======================================================================

void MainFrame::showEvent(QShowEvent * event)
{
	if (!m_windowSettingsRestored)
	{
		m_windowSettingsRestored = true;

		loadWindowSettings(m_settings, this);
		loadWindowSettings(m_settings, m_gameWindow, false);

		loadDockWindowSettings(m_settings, m_consoleDock);
		loadDockWindowSettings(m_settings, m_groupObjectWindowDock);
		loadDockWindowSettings(m_settings, m_filterWindowDock);
		loadDockWindowSettings(m_settings, m_treeBrowserDock);
		loadDockWindowSettings(m_settings, m_objectEditorDock);
		loadDockWindowSettings(m_settings, m_bookmarkBrowserDock);
		loadDockWindowSettings(m_settings, m_favoritesWindowDock);

		//set up initial state of actions that can be toggled
		m_actionsWindow->m_treeBrowser->setOn(m_treeBrowserDock->isVisible());
		m_actionsWindow->m_objectEditor->setOn(m_objectEditorDock->isVisible());
		m_actionsWindow->m_bookmarks->setOn(m_bookmarkBrowserDock->isVisible());
		m_actionsWindow->m_favorites->setOn(m_favoritesWindowDock->isVisible());
		m_actionsWindow->m_groupObjectEditor->setOn(m_groupObjectWindowDock->isVisible());
		m_actionsWindow->m_filters->setOn(m_filterWindowDock->isVisible());
		m_actionsWindow->m_console->setOn(m_consoleDock->isVisible());
		m_actionsWindow->m_gameWindow->setOn(m_gameWindow->isVisible());
	}

	QMainWindow::showEvent(event);
}

// ======================================================================
