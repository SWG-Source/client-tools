// =====================================================================
//
// ActionsTool.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// =====================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "ActionsTool.h"
#include "ActionsTool.moc"

#include "fileInterface/StdioFile.h"

#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"

#include "ActionsEdit.h"
#include "ActionHack.h"
#include "BookmarkBrowser.h"
#include "BrushData.h"
#include "FilterWindow.h"
#include "GameWindow.h"
#include "GodClientData.h"
#include "GodClientPerforce.h"
#include "GroupObjectWindow.h"
#include "IconLoader.h"
#include "MainFrame.h"
#include "ObjectEditor.h"
#include "ServerCommander.h"
#include "SystemMessageWidget.h"
#include "TreeBrowser.h"
#include "Unicode.h"

#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qworkspace.h>

//----------------------------------------------------------------------
ActionsTool::ActionsTool()
: QObject(),
  Singleton<ActionsTool>(),
  m_snapToGrid(0),
  m_saveAsBrush(0),
  m_createBrush(0),
  m_deleteBrush(0),
  m_pasteBrushHere(0),
  m_createPalette(0),
  m_deletePalette(0),
  m_getSphereTree(0),
  m_grabRelativeCoordinates(0),
	m_sendSystemMessage(0)
{
	QWidget * const p = &MainFrame::getInstance();

	//create the actions
	m_snapToGrid              = new ActionHack("Snap To Grid...",           IL_PIXMAP(hi16_action_random_rotate), "&Snap To Grid...", 0, p, "snap_to_grid");
	m_saveAsBrush             = new ActionHack("Save As Brush",             IL_PIXMAP(hi16_action_editcopy),      "Save As &Brush",   0, p, "save_as_brush");
	m_createBrush             = new ActionHack("Create Brush",              IL_PIXMAP(hi16_action_editcopy),      "Create Brush",     0, p, "create_brush");
	m_deleteBrush             = new ActionHack("Delete Brush",              IL_PIXMAP(hi16_action_editdelete),    "Delete Brush",     0, p, "delete_brush");
	m_pasteBrushHere          = new ActionHack("Paste Brush Here",          IL_PIXMAP(hi16_action_editpaste),     "Paste Brush Here", 0, p, "paste_brush_here");
	m_createPalette           = new ActionHack("Create Palette",            IL_PIXMAP(hi16_action_editcopy),      "Create Palette",   0, p, "create_palette");
	m_deletePalette           = new ActionHack("Delete Palette",            IL_PIXMAP(hi16_action_editdelete),    "Delete Palette",   0, p, "delete_palette");
	m_getSphereTree           = new ActionHack("Get SphereTree Snapshot",   IL_PIXMAP(hi16_action_random_rotate), "Get SphereTree Snapshot",   0, p, "show_sphere_tree");
	m_grabRelativeCoordinates = new ActionHack("Grab Relative Coordinates", IL_PIXMAP(hi16_action_random_rotate), "Grab Relative Coordinates", 0, p, "grab_relative_coords");
	m_sendSystemMessage       = new ActionHack("Send a System Message",     IL_PIXMAP(hi16_action_editpaste),     "Send a System Message",     0, p, "send_system_message");

	//connect them to slots
	IGNORE_RETURN(connect(m_snapToGrid,              SIGNAL(activated()), this, SLOT(onSnapToGrid())));
	IGNORE_RETURN(connect(m_saveAsBrush,             SIGNAL(activated()), this, SLOT(onSaveAsBrush())));
	IGNORE_RETURN(connect(m_createBrush,             SIGNAL(activated()), this, SLOT(onCreateBrush())));
	IGNORE_RETURN(connect(m_deleteBrush,             SIGNAL(activated()), this, SLOT(onDeleteBrush())));
	IGNORE_RETURN(connect(m_pasteBrushHere,          SIGNAL(activated()), this, SLOT(onPasteBrushHere())));
	IGNORE_RETURN(connect(m_createPalette,           SIGNAL(activated()), this, SLOT(onCreatePalette())));
	IGNORE_RETURN(connect(m_deletePalette,           SIGNAL(activated()), this, SLOT(onDeletePalette())));
	IGNORE_RETURN(connect(m_getSphereTree,           SIGNAL(activated()), this, SLOT(onShowSphereTree())));
	IGNORE_RETURN(connect(m_grabRelativeCoordinates, SIGNAL(activated()), this, SLOT(onGrabRelativeCoordinates())));
	IGNORE_RETURN(connect(m_sendSystemMessage,       SIGNAL(activated()), this, SLOT(onSendSystemMessage())));
}

//----------------------------------------------------------------------

ActionsTool::~ActionsTool()
{
	m_snapToGrid              = 0;
	m_saveAsBrush             = 0;
	m_createBrush             = 0;
	m_deleteBrush             = 0;
	m_createPalette           = 0;
	m_deletePalette           = 0;
	m_getSphereTree           = 0;
	m_grabRelativeCoordinates = 0;
	m_sendSystemMessage       = 0;
}

//----------------------------------------------------------------------

void ActionsTool::onSaveAsBrush() const
{
	bool ok = false;
	QString qName = (QInputDialog::getText(tr("Brush Creation"), tr("Please name the brush"), QLineEdit::Normal, QString::null, &ok));
	if(!qName.isNull())
	{
		std::string name = qName.latin1();
		if(ok && !name.empty())
			GodClientData::getInstance().saveCurrentSelectionAsBrush(name);
	}
}

//----------------------------------------------------------------------

void ActionsTool::onCreateBrush() const
{
}

//----------------------------------------------------------------------

void ActionsTool::onDeleteBrush() const
{
}

//----------------------------------------------------------------------

void ActionsTool::onPasteBrushHere() const
{
	//set the clipboard with the current brush
	GodClientData::getInstance().setCurrentBrush(*BrushData::getInstance().getSelectedBrush());

	//get the avatar's location, to use as the paste location
	GroundScene* const gs = dynamic_cast<GroundScene*>(Game::getScene());
	if(gs == 0)
		return;
	const Object* player    = gs->getPlayer();
	if(!player)
		return;
	const Vector& playerLocW = player->getPosition_w();

	//set the paste location to be the avatar
	GodClientData::getInstance().setPasteLocation(playerLocW);

	//now paste that brush into the world as usual
	ActionsEdit::getInstance().pasteBrush->doActivate();
}

//----------------------------------------------------------------------
/**
 * Create a new palette.  It must be named before objects can go in it.
 */
void ActionsTool::onCreatePalette() const
{
	bool ok = false;
	QString qName = (QInputDialog::getText(tr("Palette Creation"), tr("Please name the palette"), QLineEdit::Normal, QString::null, &ok));
	if(!qName.isNull())
	{
		std::string name = qName.latin1();
		if(ok && !name.empty())
			GodClientData::getInstance().addPalette(name);
	}
}

//----------------------------------------------------------------------

/**
 * Delete the currently selected palette
 */
void ActionsTool::onDeletePalette() const
{
	QMessageBox mb("Palette Deletion", 
	               "Really delete this palette?", 
	               QMessageBox::NoIcon, 
	               QMessageBox::Yes | QMessageBox::Default, 
	               QMessageBox::No  | QMessageBox::Escape, 
	               QMessageBox::NoButton);

	//TODO finalize this code
	if(mb.exec() == QMessageBox::Yes)
		GodClientData::getInstance().deletePalette("test");
}

//-----------------------------------------------------------------

/**
 * Get a snapshot of the current sphere tree.
 */
void ActionsTool::onShowSphereTree() const
{
	GodClientData::getInstance().getSphereTreeSnapshot();
}

//-----------------------------------------------------------------

/**
 * Show a dialog that allows setting the snap to grid parameters.
 */
void ActionsTool::onSnapToGrid() const
{
	GodClientData::getInstance().snapToGridDlg();
}

//-----------------------------------------------------------------

/**
 * Store the coordinates for use in building scripts
 */
void ActionsTool::onGrabRelativeCoordinates() const
{
}

//-----------------------------------------------------------------

/**
 */
void ActionsTool::onSendSystemMessage() const
{
	SystemMessageWidget* w = new SystemMessageWidget(MainFrame::getInstance().getWorkspace(), "Send System Message");
	w->show();
}

//-----------------------------------------------------------------

