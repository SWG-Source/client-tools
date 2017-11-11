// ======================================================================
//
// ActionsFile.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "ActionsFile.h"
#include "ActionsFile.moc"

#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"

#include "ActionHack.h"
#include "GodClientData.h"
#include "IconLoader.h"
#include "MainFrame.h"
#include "ServerCommander.h"

// ======================================================================

ActionsFile::ActionsFile()
: QObject(),
  Singleton<ActionsFile>(),
  exit(0)
{
	QWidget * const p = &MainFrame::getInstance();

	exit   = new ActionHack("Exit", IL_PIXMAP(hi16_action_editcopy), "E&xit", 0, p, "exit");
	exit->setWhatsThis("Exit the God Client");
	IGNORE_RETURN(connect(exit, SIGNAL(activated()), this, SLOT(onExit())));
}

//-----------------------------------------------------------------

ActionsFile::~ActionsFile()
{
	exit = 0;
}

//----------------------------------------------------------------------

/**
 * Action that allows quitting of the game
 */
void ActionsFile::onExit() const
{
	IGNORE_RETURN(MainFrame::getInstance().close());
}

//-----------------------------------------------------------------
