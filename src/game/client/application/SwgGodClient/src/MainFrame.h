// ======================================================================
//
// MainFrame.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MainFrame_H
#define INCLUDED_MainFrame_H

// ======================================================================

#include <qmainwindow.h>
#include <qpopupmenu.h>

// ======================================================================

class ActionsGame;
class ActionsEdit;
class ActionsView;
class ActionsFile;
class ActionsObjectTemplate;
class ActionsScript;
class ActionsTool;
class ActionsWindow;
class BookmarkBrowser;
class ConsoleWindow;
class FavoritesWindow;
class FilterWindow;
class GameWindow;
class GroupObjectWindow;
class MainToolBar;
class RegionBrowser;
class ObjectEditor;
class QDockWidget;
class QSettings;
class QWorkspace;
class TreeBrowser;

// ======================================================================

/**
* MainFrame is the GodClient's main window
*/
class MainFrame: public QMainWindow
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

	//we want window actions to directly show or hide these member windows
	friend ActionsWindow;

public:

	explicit MainFrame( QWidget *parent=0, const char *name=0 );
	virtual ~MainFrame();

	/**
	* @todo: move the singleton accessor for this out of the class itself.
	* clients who simply need the widget or object pointer don't need to include this other baggage
	*/
	static MainFrame & getInstance();

	bool             isActive() const;
	void             textToConsole(const char * text);
	QWorkspace*      getWorkspace() const;

	RegionBrowser*   getRegionBrowser();
	FavoritesWindow* getFavoritesWindow();

	void showEvent(QShowEvent * event);
public slots:
	bool             eventFilter(QObject *, QEvent *ev);

private slots:
	void             updateBookmarksMenu();
	void             updateGameMenu();
	void             onFileExit();

protected:
	void closeEvent( QCloseEvent * e );
	void focusOutEvent( QFocusEvent * e);

private:
	//disabled
	MainFrame(const MainFrame & rhs);
	MainFrame & operator=(const MainFrame & rhs);

private:
	void loadWindowSettings(QSettings* settings, QWidget* widget, bool move = true) const;
	void saveWindowSettings(QSettings* settings, const QWidget* widget) const;
	void loadDockWindowSettings(QSettings* settings, QDockWindow* window);
	void saveDockWindowSettings(QSettings* settings, QDockWindow* window) const;

private:
	struct Menu
	{
		struct Base
		{
			QPopupMenu menu;
			virtual ~Base() = 0 {}
		};

		struct File : public Base
		{
			int exitId;
		} file;

		struct View : public Base
		{
			QPopupMenu bookmarksMenu;
		} view;

		struct Edit : public Base
		{
		} edit;

		struct Game : public Base
		{
			int toggleHUDId;
		} game;

		struct Script : public Base
		{
		} script;

		struct ObjectTemplate : public Base
		{
		} objectTemplate;

		struct Tools : public Base
		{
		} tool;

		struct Window : public Base
		{
		} window;

		struct Help : public Base
		{
		} help;

	} m_menus;

	QWorkspace*        m_workspace;

	GameWindow*        m_gameWindow;
	ConsoleWindow*     m_console;
	TreeBrowser*       m_treeBrowser;
	ObjectEditor*      m_objectEditor;
	GroupObjectWindow* m_groupObjectWindow;
	FilterWindow*      m_filterWindow;
	BookmarkBrowser*   m_bookmarkBrowser;
	FavoritesWindow*   m_favoritesWindow;

	QDockWindow*       m_consoleDock;
	QDockWindow*       m_treeBrowserDock;
	QDockWindow*       m_objectEditorDock;
	QDockWindow*       m_groupObjectWindowDock;
	QDockWindow*       m_filterWindowDock;
	QDockWindow*       m_bookmarkBrowserDock;
	QDockWindow*       m_regionsViewDock;
	QDockWindow*       m_favoritesWindowDock;

	QSettings*         m_settings;

	ActionsGame*           m_actionsGame;
	ActionsEdit*           m_actionsEdit;
	ActionsView*           m_actionsView;
	ActionsFile*           m_actionsFile;
	ActionsScript*         m_actionsScript;
	ActionsObjectTemplate* m_actionsObjectTemplate;
	ActionsTool*           m_actionsTool;
	ActionsWindow*         m_actionsWindow;

	RegionBrowser*         m_regionsView;

	bool                   m_active;
	bool                   m_windowSettingsRestored;

	static MainFrame*      ms_singleton;
};

//----------------------------------------------------------------------

inline MainFrame & MainFrame::getInstance()
{
	NOT_NULL(ms_singleton);
	return *ms_singleton;
}

//----------------------------------------------------------------------

inline RegionBrowser * MainFrame::getRegionBrowser()
{
	return m_regionsView;
}

//----------------------------------------------------------------------

inline FavoritesWindow * MainFrame::getFavoritesWindow()
{
	return m_favoritesWindow;
}

//----------------------------------------------------------------------

inline bool MainFrame::isActive() const
{
	return m_active;
}

// ======================================================================


#include <qapplication.h>
class GodClientApplication : public QApplication
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	GodClientApplication(int& argc, char** argv) : QApplication(argc, argv) {}
	
	virtual bool winEventFilter(MSG*);
	static void  setGameHWND(HWND);

private:
	GodClientApplication();
	GodClientApplication(const GodClientApplication& rhs);
	GodClientApplication& operator=(const GodClientApplication& rhs);

private:
	static HWND m_gameHWND;
};

#endif
