// ======================================================================
//
// ActionsGame.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ActionsGame_H
#define INCLUDED_ActionsGame_H

// ======================================================================

#include "Singleton/Singleton.h"

// ======================================================================

class ActionHack;

// ======================================================================

/**
* ActionsGame is the resting place of various game-menu related actions
*/
class ActionsGame : public QObject, public Singleton<ActionsGame>
{
	Q_OBJECT;  //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	ActionsGame();
	~ActionsGame();
	bool getHUDEnabled(bool & enabled) const;

public:
	//lint -save
	//lint -e1925 // public data member
	ActionHack*          hud;
	ActionHack*          gameFocusAllowed;
	ActionHack*          interiorCameraAllowed;
	ActionHack*          renderWireframe;
	ActionHack*          renderTextured;
	ActionHack*          showObjectNames;
	ActionHack*          showObjectNetworkIds;
	ActionHack*          console;
	ActionHack*          showDebugMenu;
	ActionHack*          watchTargetPath;
	ActionHack*          ignoreTargetPath;
	ActionHack*          togglePathMap;
	ActionHack*          showRegionBrowser;
	ActionHack*          purgeTerrain;
	ActionHack*          reloadTerrain;
	ActionHack*          m_saveInteriorLayout;
	//lint -restore

private slots:
	void                  onToggleHud               (bool b) const;
	void                  onRenderWireframe         (bool b) const;
	void                  onRenderTextured          (bool b) const;
	void                  onShowObjectNames         (bool b) const;
	void                  onShowObjectNetworkIds    (bool b) const;
	void                  onToggleConsole           (bool b) const;
	void                  onTogglePathMap           (bool b) const;
	void                  onShowDebugMenu           ()       const;
	void                  onWatchTargetPath         ()       const;
	void                  onIgnoreTargetPath        ()       const;
	void                  onShowRegionBrowser       ()       const;
	void                  onPurgeTerrain            ()       const;
	void                  onReloadTerrain           ()       const;
	void                  onSaveInteriorLayout      ()       const;

private:
	//disabled
	ActionsGame(const ActionsGame & rhs);
	ActionsGame & operator=(const ActionsGame & rhs);
};

// ======================================================================

#endif
