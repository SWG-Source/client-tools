// =====================================================================
//
// ActionsBuildoutArea.cpp
//
// Copyright 2005 Sony Online Entertainment
//
// =====================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "ActionsBuildoutArea.h"
#include "ActionsBuildoutArea.moc"

#include "clientGame/Game.h"
#include "sharedFile/Iff.h"
#include "sharedUtility/DataTable.h"

#include "ActionHack.h"
#include "ConfigGodClient.h"
#include "GodClientData.h"
#include "GodClientPerforce.h"
#include "IconLoader.h"
#include "MainFrame.h"
#include "ServerCommander.h"
#include "Unicode.h"

#include <qmessagebox.h>
#include <process.h>
#include <qfiledialog.h>

// =====================================================================

ActionsBuildoutArea::ActionsBuildoutArea() :
	QObject(),
	Singleton<ActionsBuildoutArea>(),
	actionRefresh(0),
	actionEdit(0),
	actionSave(0)
{
	QWidget * const p = &MainFrame::getInstance();

	actionRefresh = new ActionHack("Refresh Buildout Area List", IL_PIXMAP(hi16_action_reload),   "&Refresh", 0, p, "buildoutarea_refresh");
	actionEdit = new ActionHack("Mark Selected Buildout Area for Edit", IL_PIXMAP(hi16_action_finish), "&Edit",  0, p, "buildoutarea_edit");
	actionSave = new ActionHack("Save Selected Buildout Area", IL_PIXMAP(hi16_action_finish), "&Save",  0, p, "buildoutarea_save");

	actionEdit->setEnabled(false);
	actionSave->setEnabled(false);
}

//----------------------------------------------------------------------

ActionsBuildoutArea::~ActionsBuildoutArea()
{
	actionRefresh = 0;
	actionEdit = 0;
	actionSave = 0;
}

// ======================================================================
