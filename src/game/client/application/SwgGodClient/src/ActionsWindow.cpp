// =====================================================================
//
// ActionsWindow.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// =====================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "ActionsWindow.h"
#include "ActionsWindow.moc"

#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "fileInterface/StdioFile.h"

#include "ActionHack.h"
#include "BookmarkBrowser.h"
#include "FavoritesWindow.h"
#include "FilterWindow.h"
#include "GameWindow.h"
#include "GodClientData.h"
#include "GodClientPerforce.h"
#include "GroupObjectWindow.h"
#include "IconLoader.h"
#include "MainFrame.h"
#include "ObjectEditor.h"
#include "ServerCommander.h"
#include "TreeBrowser.h"
#include "Unicode.h"

#include <qmessagebox.h>
#include <qfiledialog.h>

// =====================================================================

const char * const ActionsWindow::DragMessages::SCRIPT_DRAGGED = "ScriptListView::DragMessages::SCRIPT_DRAGGED";

//----------------------------------------------------------------------
ActionsWindow::ActionsWindow()
: QObject(),
  Singleton<ActionsWindow>(),
  m_treeBrowser(0),
  m_objectEditor(0),
  m_bookmarks(0),
  m_favorites(0),
  m_groupObjectEditor(0),
  m_filters(0),
  m_console(0),
  m_gameWindow(0),
  m_regionsView(0)
{
	QWidget * const p = &MainFrame::getInstance();

	//create the actions
	m_objectEditor      = new ActionHack("Object Editor",       IL_PIXMAP(hi16_action_window_new),   "&Object Editor",        0, p, "objecteditor",      true);
	m_bookmarks         = new ActionHack("Bookmarks",           IL_PIXMAP(hi16_action_window_new),   "&Bookmarks",            0, p, "bookmarks",         true);
	m_favorites         = new ActionHack("Favorites",           IL_PIXMAP(hi16_action_window_new),   "Fa&vorites",            0, p, "favorites",         true);
	m_groupObjectEditor = new ActionHack("Group Object Editor", IL_PIXMAP(hi16_action_window_new),   "G&roup Object Editor",  0, p, "groupobjecteditor", true);
	m_filters           = new ActionHack("Filters",             IL_PIXMAP(hi16_action_window_new),   "&Filters",              0, p, "filters",           true);
	m_treeBrowser       = new ActionHack("Tree Browser",        IL_PIXMAP(hi16_action_window_new),   "&Tree Browser",         0, p, "treebrowser",       true);
	m_console           = new ActionHack("Console",             IL_PIXMAP(hi16_action_window_new),   "&Console",              0, p, "console",           true);
	m_gameWindow        = new ActionHack("Game Window",         IL_PIXMAP(hi16_action_window_new),   "&Game Window",          0, p, "game_window",       true);
	m_regionsView       = new ActionHack("Region Viewer",       IL_PIXMAP(hi16_action_window_new),   "&Region Viewer",        0, p, "regionviewer",      true);

	//connect the actions to slots
	IGNORE_RETURN(connect(m_treeBrowser,       SIGNAL(toggled(bool)), this, SLOT(onTreeBrowser      (bool))));
	IGNORE_RETURN(connect(m_objectEditor,      SIGNAL(toggled(bool)), this, SLOT(onObjectEditor     (bool))));
	IGNORE_RETURN(connect(m_bookmarks,         SIGNAL(toggled(bool)), this, SLOT(onBookmarks        (bool))));
	IGNORE_RETURN(connect(m_favorites,         SIGNAL(toggled(bool)), this, SLOT(onFavorites        (bool))));
	IGNORE_RETURN(connect(m_groupObjectEditor, SIGNAL(toggled(bool)), this, SLOT(onGroupObjectEditor(bool))));
	IGNORE_RETURN(connect(m_filters,           SIGNAL(toggled(bool)), this, SLOT(onFilters          (bool))));
	IGNORE_RETURN(connect(m_console,           SIGNAL(toggled(bool)), this, SLOT(onConsole          (bool))));
	IGNORE_RETURN(connect(m_gameWindow,        SIGNAL(toggled(bool)), this, SLOT(onGameWindow       (bool))));
	IGNORE_RETURN(connect(m_regionsView,       SIGNAL(toggled(bool)), this, SLOT(onRegionsView      (bool))));


	//set up initial state of togglable actions
	//these windows are created visible, so make the "checkboxes" on the items match that
	m_treeBrowser->setOn       (true);
	m_objectEditor->setOn      (true);
	m_bookmarks->setOn         (true);
	m_favorites->setOn         (true);
	m_groupObjectEditor->setOn (true);
	m_filters->setOn           (true);
	m_console->setOn           (true);
	m_gameWindow->setOn        (true);

	//we want the filter window hidden initially
	m_filters->doToggle        (false);
}

//----------------------------------------------------------------------

ActionsWindow::~ActionsWindow()
{
	m_treeBrowser       = 0;
	m_objectEditor      = 0;
	m_bookmarks         = 0;
	m_favorites         = 0;
	m_groupObjectEditor = 0;
	m_filters           = 0;
	m_console           = 0;
	m_gameWindow        = 0;
}

//----------------------------------------------------------------------

void ActionsWindow::onTreeBrowser(bool b) const
{
	if(b)
		MainFrame::getInstance().m_treeBrowserDock->show();
	else
		MainFrame::getInstance().m_treeBrowserDock->hide();
}

//----------------------------------------------------------------------

void ActionsWindow::onObjectEditor(bool b) const
{
	if(b)
		MainFrame::getInstance().m_objectEditorDock->show();
	else
		MainFrame::getInstance().m_objectEditorDock->hide();
}

//----------------------------------------------------------------------

void ActionsWindow::onBookmarks(bool b) const
{
	if(b)
		MainFrame::getInstance().m_bookmarkBrowserDock->show();
	else
		MainFrame::getInstance().m_bookmarkBrowserDock->hide();
}

//----------------------------------------------------------------------

void ActionsWindow::onFavorites(bool b) const
{
	if(b)
		MainFrame::getInstance().m_favoritesWindowDock->show();
	else
		MainFrame::getInstance().m_favoritesWindowDock->hide();
}

//----------------------------------------------------------------------

void ActionsWindow::onGroupObjectEditor(bool b) const
{
	if(b)
		MainFrame::getInstance().m_groupObjectWindowDock->show();
	else
		MainFrame::getInstance().m_groupObjectWindowDock->hide();
}

//----------------------------------------------------------------------

void ActionsWindow::onFilters(bool b) const
{
	if(b)
		MainFrame::getInstance().m_filterWindowDock->show();
	else
		MainFrame::getInstance().m_filterWindowDock->hide();
}

//----------------------------------------------------------------------

void ActionsWindow::onConsole(bool b) const
{
	if(b)
		MainFrame::getInstance().m_consoleDock->show();
	else
		MainFrame::getInstance().m_consoleDock->hide();
}

//----------------------------------------------------------------------

void ActionsWindow::onGameWindow(bool b) const
{
	if(b)
		MainFrame::getInstance().m_gameWindow->show();
	else
		MainFrame::getInstance().m_gameWindow->hide();
}

//----------------------------------------------------------------------

void ActionsWindow::onRegionsView(bool b) const
{
	if(b)
		MainFrame::getInstance().m_regionsViewDock->show();
	else
		MainFrame::getInstance().m_regionsViewDock->hide();
}

//----------------------------------------------------------------------
