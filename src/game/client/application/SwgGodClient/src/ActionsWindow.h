// ======================================================================
//
// ActionsWindow.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ActionsWindow_H
#define INCLUDED_ActionsWindow_H

// ======================================================================

#include "Singleton/Singleton.h"

// ======================================================================

class ActionHack;

//----------------------------------------------------------------------

/**
 * ActionsWindow is the resting place of various window-menu related actions.
 * Its purpose is pretty simple - make the main windows of the application visible or
 * invisible.  All the actions should reflect that.
 */
class ActionsWindow : public QObject, public Singleton<ActionsWindow>
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:

	struct DragMessages
	{
		static const char* const SCRIPT_DRAGGED;
	};

	//lint -save
	//lint -e1925 // public data member
	ActionHack*             m_treeBrowser;
	ActionHack*             m_objectEditor;
	ActionHack*             m_bookmarks;
	ActionHack*             m_favorites;
	ActionHack*             m_groupObjectEditor;
	ActionHack*             m_filters;
	ActionHack*             m_console;
	ActionHack*             m_gameWindow;
	ActionHack*             m_regionsView;
	//lint -restore

public:
	ActionsWindow();
	~ActionsWindow();

private slots:
	void onTreeBrowser      (bool b) const;
	void onObjectEditor     (bool b) const;
	void onBookmarks        (bool b) const;
	void onFavorites        (bool b) const;
	void onGroupObjectEditor(bool b) const;
	void onFilters          (bool b) const;
	void onConsole          (bool b) const;
	void onGameWindow       (bool b) const;
	void onRegionsView      (bool b) const;

private:
	//disabled
	ActionsWindow(const ActionsWindow& rhs);
	ActionsWindow& operator=(const ActionsWindow& rhs);
};

// ======================================================================

#endif

