// ======================================================================
//
// ActionsEdit.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "ActionsEdit.h"

#include "ActionHack.h"
#include "ActionsEdit.moc"
#include "ActionsTool.h"
#include "BuildoutAreaSupport.h"
#include "ConfigGodClient.h"
#include "FormWindow.h"
#include "GodClientData.h"
#include "IconLoader.h"
#include "MainFrame.h"
#include "ModificationHistory.h"
#include "ServerCommander.h"
#include "TriggerWindow.h"
#include "clientGame/ClientObject.h"
#include "clientGame/FormManagerClient.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedRandom/Random.h"
#include "sharedUtility/FileName.h"

#include <qaccel.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qfiledialog.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qspinbox.h>

namespace ActionsEditNamespace
{
	const std::string headerLine1 = "TEMPLATE	X	Y	Z	YAW\n";
	const std::string headerLine2 = "s	f	f	f	f\n";
}
// ======================================================================

const char * const ActionsEdit::DragMessages::PALETTE_DRAGGED_INTO_GAME           = "ActionsEdit::DragMessages::PALETTE_DRAGGED_INTO_GAME";
const char * const ActionsEdit::DragMessages::PALETTE_DRAGGED_ONTO_PALETTE_WIDGET = "ActionsEdit::DragMessages::PALETTE_DRAGGED_ONTO_PALETTE_WIDGET";

// ======================================================================
ActionsEdit::ActionsEdit()
: QObject(),
Singleton<ActionsEdit>(),
MessageDispatch::Receiver(),
undo(0),
redo(0),
copy(0),
paste(0),
pasteBrush(0),
del(0),
cut(0),
createObjectFromSelectedTemplate(0),
dropToTerrain(0),
randomRotate(0),
applyTransform(0),
unlockSelected(0),
unlockAll(0),
unlockNonStructures(0),
unlockServerOnly(0),
alignToTerrain(0),
rotateReset(0),
rotateModeGroup(0),
rotateModeYaw(0),
rotateModePitch(0),
rotateModeRoll(0),
rotatePivotModeGroup(0),
rotatePivotModeSelf(0),
rotatePivotModeCenter(0),
rotatePivotModeLastSelected(0),
rotateIncrement45(0),
rotateDecrement45(0),
selectionStoreF1(0),
selectionStoreF2(0),
selectionStoreF3(0),
selectionStoreF4(0),
selectionRestoreF1(0),
selectionRestoreF2(0),
selectionRestoreF3(0),
selectionRestoreF4(0),
setTransform(0),
editTriggers(0),
editFormData(0),
copyForPOI(0),
showObjectAxes(0),
togglePauseParticleSystems(0),
createTheater (0),
createClientDataFile (0),
m_toggleDropToTerrain (0),
m_toggleAlignToTerrain (0),
m_selectedServerTemplate(),
m_selectedClientTemplate()
{
	QWidget * const p = &MainFrame::getInstance();

	//create all the "edit" actions, used in toolbars, menus, and manually
	undo                             = new ActionHack("Undo", IL_PIXMAP(hi16_action_undo), "&Undo",                                            QT_ACCEL2(CTRL,Key_Z), p, "undo");
	IGNORE_RETURN(connect(undo, SIGNAL(activated()), this, SLOT(onUndo())));

	redo                             = new ActionHack("Redo", IL_PIXMAP(hi16_action_redo), "&Redo",                                            QT_ACCEL2(CTRL,Key_Y), p, "redo");
	IGNORE_RETURN(connect(redo, SIGNAL(activated()), this, SLOT(onRedo())));

	copy                             = new ActionHack("Copy Selection", IL_PIXMAP(hi16_action_editcopy), "&Copy",                              QT_ACCEL2(CTRL,Key_C), p, "copy");
	IGNORE_RETURN(connect(copy, SIGNAL(activated()), this, SLOT(onCopy())));

	paste                            = new ActionHack("Paste Selection", IL_PIXMAP(hi16_action_editpaste), "&Paste",                          QT_ACCEL2(CTRL,Key_V), p, "paste");
	IGNORE_RETURN(connect(paste, SIGNAL(activated()), this, SLOT(onPaste())));
	paste->setEnabled(false);

	pasteBrush                       = new ActionHack("Paste Brush", IL_PIXMAP(hi16_action_editpaste), "&Paste Brush",                          0, p, "paste_brush");
	IGNORE_RETURN(connect(pasteBrush, SIGNAL(activated()), this, SLOT(onPasteBrush())));

	del                              = new ActionHack("Delete Selection", IL_PIXMAP(hi16_action_editdelete), "&Delete",                         QT_ACCEL (Key_Delete), p, "delete");
	IGNORE_RETURN(connect(del, SIGNAL(activated()), this, SLOT(onDelete())));

	cut                              = new ActionHack("Cut Selection", IL_PIXMAP(hi16_action_editcut), "Cu&t",                                  QT_ACCEL2(CTRL,Key_X), p, "cut");
	IGNORE_RETURN(connect(cut, SIGNAL(activated()), this, SLOT(onCut())));

	applyTransform                   = new ActionHack("Apply Transform", IL_PIXMAP(hi16_action_apply_transform), "Apply &Transform", QT_ACCEL2(CTRL,Key_T), p, "apply_transform");
	IGNORE_RETURN(connect(applyTransform, SIGNAL(activated()), this, SLOT(onApplyTransform())));
	//"apply transform" gets a second accelerator - the space bar
	QAccel * acc                     = new QAccel(p, "accel");
	IGNORE_RETURN(acc->connectItem(acc->insertItem(QT_ACCEL(Key_Space)), applyTransform, SLOT(doActivate())));

	unlockSelected                   = new ActionHack("Unlock Selected", QIconSet(), "&Unlock Selected", QT_ACCEL2(CTRL,Key_U), p, "unlock_selected");
	IGNORE_RETURN(connect(unlockSelected, SIGNAL(activated()), this, SLOT(onUnlockSelected())));

	unlockAll                        = new ActionHack("Unlock All", QIconSet(), "Unlock All", 0, p, "unlock_all", true);
	IGNORE_RETURN(connect(unlockAll, SIGNAL(activated()), this, SLOT(onUnlockAll())));

	unlockNonStructures              = new ActionHack("Unlock Non-Structures", QIconSet(), "Unlock Non-Structures", 0, p, "unlock_non_structures", true);
	IGNORE_RETURN(connect(unlockNonStructures, SIGNAL(activated()), this, SLOT(onUnlockNonStructures())));

	unlockServerOnly                 = new ActionHack("Unlock Server Only Objects", QIconSet(), "Unlock Server Only Objects", 0, p, "unlock_server_only", true);
	IGNORE_RETURN(connect(unlockServerOnly, SIGNAL(activated()), this, SLOT(onUnlockServerOnly())));

	dropToTerrain                    = new ActionHack("Drop To Terrain", IL_PIXMAP(hi16_action_drop_to_terrain), "&Drop to Terrain", QT_ACCEL2(CTRL,Key_D), p, "drop_to_terrain");
	IGNORE_RETURN(connect(dropToTerrain, SIGNAL(activated()), this, SLOT(onDropToTerrain())));

	randomRotate                     = new ActionHack("Random Rotate", IL_PIXMAP(hi16_action_random_rotate), "Random &Rotate",       0, p, "random_rotate");
	IGNORE_RETURN(connect(randomRotate, SIGNAL(activated()), this, SLOT(onRandomRotate())));

	createObjectFromSelectedTemplate = new ActionHack("Create New Object []", IL_PIXMAP(hi16_action_window_new), "Create &New Object []", QT_ACCEL2(CTRL,Key_N), p, "create_new_object");
	createObjectFromSelectedTemplate->setEnabled (false);

	alignToTerrain                   = new ActionHack("Align to Terrain", IL_PIXMAP(hi16_action_align_to_terrain), "&Align to Terrain", QT_ACCEL3(CTRL,ALT,Key_D), p, "align_to_terrain");
	IGNORE_RETURN(connect(alignToTerrain, SIGNAL(activated()), this, SLOT(onAlignToTerrain())));

	rotateReset                      = new ActionHack("Reset Rotation",  IL_PIXMAP(hi16_action_rotate_reset),     "Re&set Rotation", 0,     p, "rotate_reset");
	IGNORE_RETURN(connect(rotateReset, SIGNAL(activated()), this, SLOT(onRotateReset())));

	selectionStoreF1                 = new ActionHack("Store Group 1",  IL_PIXMAP(hi16_action_editcopy), "Store Group 1", QT_ACCEL2(CTRL,Key_F1), p, "store_group1");
	IGNORE_RETURN(connect(selectionStoreF1, SIGNAL(activated()), this, SLOT(onStoreGroup1())));

	selectionStoreF2                 = new ActionHack("Store Group 2",  IL_PIXMAP(hi16_action_editcopy), "Store Group 2", QT_ACCEL2(CTRL,Key_F2), p, "store_group2");
	IGNORE_RETURN(connect(selectionStoreF2, SIGNAL(activated()), this, SLOT(onStoreGroup2())));

	selectionStoreF3                 = new ActionHack("Store Group 3",  IL_PIXMAP(hi16_action_editcopy), "Store Group 3", QT_ACCEL2(CTRL,Key_F3), p, "store_group3");
	IGNORE_RETURN(connect(selectionStoreF3, SIGNAL(activated()), this, SLOT(onStoreGroup3())));

	selectionStoreF4                 = new ActionHack("Store Group 4",  IL_PIXMAP(hi16_action_editcopy), "Store Group 4", QT_ACCEL2(CTRL,Key_F4), p, "store_group4");
	IGNORE_RETURN(connect(selectionStoreF4, SIGNAL(activated()), this, SLOT(onStoreGroup4())));

	selectionRestoreF1               = new ActionHack("Restore Group 1",  IL_PIXMAP(hi16_action_editpaste), "Restore Group 1", QT_ACCEL(Key_F1), p, "restore_group1");
	IGNORE_RETURN(connect(selectionRestoreF1, SIGNAL(activated()), this, SLOT(onRestoreGroup1())));

	selectionRestoreF2               = new ActionHack("Restore Group 2",  IL_PIXMAP(hi16_action_editpaste), "Restore Group 2", QT_ACCEL(Key_F2), p, "restore_group2");
	IGNORE_RETURN(connect(selectionRestoreF2, SIGNAL(activated()), this, SLOT(onRestoreGroup2())));

	selectionRestoreF3               = new ActionHack("Restore Group 3",  IL_PIXMAP(hi16_action_editpaste), "Restore Group 3", QT_ACCEL(Key_F3), p, "restore_group3");
	IGNORE_RETURN(connect(selectionRestoreF3, SIGNAL(activated()), this, SLOT(onRestoreGroup3())));

	selectionRestoreF4               = new ActionHack("Restore Group 4",  IL_PIXMAP(hi16_action_editpaste), "Restore Group 4", QT_ACCEL(Key_F4), p, "restore_group4");
	IGNORE_RETURN(connect(selectionRestoreF4, SIGNAL(activated()), this, SLOT(onRestoreGroup4())));

	setTransform                     = new ActionHack("Set Transform", IL_PIXMAP(hi16_action_random_rotate), "Set Transform", 0, p, "set_transform");
	IGNORE_RETURN(connect(setTransform, SIGNAL(activated()), this, SLOT(onSetTransform())));

	editTriggers                     = new ActionHack("Edit Triggers...", IL_PIXMAP(hi16_action_random_rotate), "Edit Triggers...", 0, p, "edit_triggers");
	IGNORE_RETURN(connect(editTriggers, SIGNAL(activated()), this, SLOT(onEditTriggers())));

	editFormData                     = new ActionHack("Edit Form Data...", IL_PIXMAP(hi16_action_random_rotate), "Edit Form Data...", 0, p, "edit_form_data");
	IGNORE_RETURN(connect(editFormData, SIGNAL(activated()), this, SLOT(onEditFormData())));

	copyForPOI                       = new ActionHack("Copy For POI",     IL_PIXMAP(hi16_action_editcopy),      "Copy For &POI", 0, p, "copy_for_poi");
	IGNORE_RETURN(connect(copyForPOI, SIGNAL(activated()), this, SLOT(onCopyForPOI())));

	showObjectAxes                   = new ActionHack("Show Object Axes", IL_PIXMAP(hi16_action_rotate_reset),      "Show Object &Axes", 0, p, "show_object_axes");
	IGNORE_RETURN(connect(showObjectAxes, SIGNAL(activated()), this, SLOT(onShowObjectAxes())));

	togglePauseParticleSystems       = new ActionHack("Toggle Particle Systems Pause", IL_PIXMAP(hi16_action_gear),      "Toggle Particle Systems &Pause", 0, p, "toggle_pause_particle_systems");
	IGNORE_RETURN(connect(togglePauseParticleSystems, SIGNAL(activated()), this, SLOT(onToggleParticleSystemPause())));

	createTheater = new ActionHack ("Create Theater", IL_PIXMAP (hi16_action_editcopy), "Create &Theater", 0, p, "createTheater");
	IGNORE_RETURN (connect (createTheater, SIGNAL (activated ()), this, SLOT (onCreateTheater ())));

	createClientDataFile = new ActionHack ("Create ClientDataFile", IL_PIXMAP (hi16_action_editcopy), "Create &ClientDataFile", 0, p, "createClientDataFile");
	IGNORE_RETURN (connect (createClientDataFile, SIGNAL (activated ()), this, SLOT (onCreateClientDataFile ())));

	rotateIncrement45                = new ActionHack("Rotate +45",  IL_PIXMAP(hi16_action_rotate_increment45),     "Rotate &+45", 0,     p, "rotateIncrement45");
	IGNORE_RETURN(connect(rotateIncrement45, SIGNAL(activated()), this, SLOT(onRotateIncrement45())));

	rotateDecrement45                = new ActionHack("Rotate -45",  IL_PIXMAP(hi16_action_rotate_decrement45),     "Rotate &-45", 0,     p, "rotateDecrement45");
	IGNORE_RETURN(connect(rotateDecrement45, SIGNAL(activated()), this, SLOT(onRotateDecrement45())));

	m_toggleDropToTerrain                    = new ActionHack("Toggle Drop To Terrain", IL_PIXMAP(hi16_action_drop_to_terrain), "Toggle Drop to Terrain", 0, p, "toggle_drop_to_terrain",true);
	IGNORE_RETURN(connect(m_toggleDropToTerrain, SIGNAL(toggled(bool)), this, SLOT(onToggleDropToTerrain())));

	m_toggleAlignToTerrain                    = new ActionHack("Toggle Align To Terrain", IL_PIXMAP(hi16_action_align_to_terrain), "Toggle Align to Terrain", 0, p, "toggle_align_to_terrain",true);
	IGNORE_RETURN(connect(m_toggleAlignToTerrain, SIGNAL(toggled(bool)), this, SLOT(onToggleAlignToTerrain())));

	//create a group of rotation axes actions, start in Yaw mode
	rotateModeGroup = new QActionGroup(p, "rotate mode group", true);
	rotateModeGroup->setUsesDropDown(true);
	rotateModeGroup->setText("Rotate Mode");
	rotateModeGroup->setMenuText("&Rotate Mode");
	rotateModeYaw    = new ActionHack("Yaw",   IL_PIXMAP(hi16_action_rotatemode_yaw),   "&Yaw",   0, rotateModeGroup, "rotateModeYaw",   true);
	rotateModePitch  = new ActionHack("Pitch", IL_PIXMAP(hi16_action_rotatemode_pitch), "&Pitch", 0, rotateModeGroup, "rotateModePitch", true);
	rotateModeRoll   = new ActionHack("Roll",  IL_PIXMAP(hi16_action_rotatemode_roll),  "&Roll",  0, rotateModeGroup, "rotateModeRoll",  true);

	//create a group of rotation center actions, start in self mode
	rotatePivotModeGroup = new QActionGroup(p, "rotate pivot mode group", true);
	rotatePivotModeGroup->setUsesDropDown(true);
	rotatePivotModeGroup->setText("Rotate Pivot Mode");
	rotatePivotModeGroup->setMenuText("Rotate &Pivot Mode");
	rotatePivotModeSelf         = new ActionHack("Self",         QIconSet(),   "&Selft",            0, rotatePivotModeGroup, "self",    true);
	rotatePivotModeCenter       = new ActionHack("Sel. Center",  QIconSet(),   "Selection &Center", 0, rotatePivotModeGroup, "center",  true);
	rotatePivotModeLastSelected = new ActionHack("Last Sel.",    QIconSet(),   "&Last Selection",   0, rotatePivotModeGroup, "lastsel", true);
	rotatePivotModeSelf->setOn(true);

	setSelectionControlsEnabled(false);

	//set up initial state of togglable actions
	rotateModeYaw->setOn(true);

	connectToMessage(GodClientData::Messages::SELECTION_CHANGED);
	connectToMessage(GodClientData::Messages::CLIPBOARD_CHANGED);
	connectToMessage(GodClientData::Messages::GHOSTS_CREATED);
	connectToMessage(GodClientData::Messages::GHOSTS_KILLED);

}

//-----------------------------------------------------------------

ActionsEdit::~ActionsEdit()
{
	disconnectFromMessage(GodClientData::Messages::SELECTION_CHANGED);
	disconnectFromMessage(GodClientData::Messages::CLIPBOARD_CHANGED);
	disconnectFromMessage(GodClientData::Messages::GHOSTS_CREATED);
	disconnectFromMessage(GodClientData::Messages::GHOSTS_KILLED);
	
	undo = 0;
	redo = 0;
	copy = 0;
	paste = 0;
	del = 0;
	cut = 0;
	createObjectFromSelectedTemplate = 0;
	dropToTerrain = 0;
	randomRotate = 0;
	applyTransform = 0;
	unlockSelected = 0;
	unlockAll = 0;
	unlockNonStructures = 0;
	unlockServerOnly = 0;
	alignToTerrain = 0;
	rotateReset = 0;
	rotateModeYaw = 0;
	rotateModePitch = 0;
	rotateModeRoll = 0;
	rotatePivotModeSelf = 0;
	rotatePivotModeCenter = 0;
	rotatePivotModeLastSelected = 0;
	rotateIncrement45 = 0;
	rotateDecrement45 = 0;
	setTransform = 0;
	editTriggers = 0;
	copyForPOI = 0;
	selectionStoreF1 = 0;
	selectionStoreF2 = 0;
	selectionStoreF3 = 0;
	selectionStoreF4 = 0;
	selectionRestoreF1 = 0;
	selectionRestoreF2 = 0;
	selectionRestoreF3 = 0;
	selectionRestoreF4 = 0;
	rotateModeGroup = 0;
	rotatePivotModeGroup = 0;
	showObjectAxes = 0;
	togglePauseParticleSystems = 0;
	createTheater = 0;
	createClientDataFile = 0;
	m_toggleDropToTerrain = 0;
	m_toggleAlignToTerrain = 0;
}

//-----------------------------------------------------------------
/**
 * Handler for undo message.
 */
void ActionsEdit::onUndo() const
{
	if(!ModificationHistory::getInstance().undo())
	{
		QApplication::beep();
		return;
	}
}

//-----------------------------------------------------------------
/**
 * Handler for redo message.
 */
void ActionsEdit::onRedo() const
{
	if(!ModificationHistory::getInstance().redo())
	{
		QApplication::beep();
		return;
	}
}

//-----------------------------------------------------------------
/**
 * Handler for copy message, pass it to GodClientData that actually handles these types of calls
 */
void ActionsEdit::onCopy() const
{
	GodClientData::getInstance().copySelection();
}

//-----------------------------------------------------------------
/**
 * Handler for paste message.
 * Find the intersection of the cursor with the terrain to decide the location to paste on.
 */
void ActionsEdit::onPaste() const
{
	GodClientData * const gcd = &GodClientData::getInstance();

	GodClientData::ClipboardList_t clip;
	gcd->getClipboard(clip);

	internalPaste(clip);
}

//-----------------------------------------------------------------
/**
 * Handler for paste message.
 * Find the intersection of the cursor with the terrain to decide the location to paste on.
 */
void ActionsEdit::onPasteBrush() const
{
	GodClientData * const gcd = &GodClientData::getInstance();

	GodClientData::ClipboardList_t clip;
	gcd->getCurrentBrush(clip);

	internalPaste(clip);
}

//-----------------------------------------------------------------
/**
 */
void ActionsEdit::internalPaste(GodClientData::ClipboardList_t& clip) const
{
	GodClientData * const gcd = &GodClientData::getInstance();

	Vector center;
	IGNORE_RETURN(gcd->calculateClipboardCenter(clip, center));

	float clipBottomY = 0;
	IGNORE_RETURN(gcd->calculateClipboardBottom(clip, clipBottomY));

	float clipDelta = center.y - clipBottomY;

	const CellProperty* cellProperty = CellProperty::getWorldCellProperty ();
	Vector        intersection_p;
	//somone may have assigned a pre-known paste location for this action, grab it if it exists
	if(gcd->pasteLocationKnown())
	{
		intersection_p = gcd->absorbPasteLocation();
	}
	else
	{
		//find the location that we intersect with, only paste if we actually hit something
		Vector2d  cursorPosition = gcd->getCursorScreenPosition();
		IGNORE_RETURN(gcd->findIntersection_p(static_cast<int>(cursorPosition.x), static_cast<int>(cursorPosition.y), cellProperty, intersection_p));
	}
	Transform newObjTransform;

	if(intersection_p.x == 0 && intersection_p.y == 0 && intersection_p.z == 0)
		return;

	for(GodClientData::ClipboardList_t::iterator it = clip.begin(); it != clip.end(); ++it)
	{
		GodClientData::ClipboardObject * const clipObj = *it;
		
		if(clipObj)
		{
			//if we collided with something, place the object there
			Vector objOldPosition = clipObj->transform.getPosition_p();
			const Vector objRelativeToCenter = center - objOldPosition;
			//initialize transform with original location and(more importantly, rotation)
			newObjTransform = clipObj->transform;

			//offset the object from the cursor by it's relative position
			newObjTransform.setPosition_p(intersection_p - objRelativeToCenter + Vector(0, clipDelta, 0));
			IGNORE_RETURN(ServerCommander::getInstance().createObject("toolbar pasted", clipObj->serverObjectTemplateName.empty() ? clipObj->sharedObjectTemplateName : clipObj->serverObjectTemplateName, cellProperty, newObjTransform));
		}
	}
}

//-----------------------------------------------------------------
/**
 * Handler for copy message, pass it to GodClientData that actually handles these types of calls
 */
void ActionsEdit::onCut() const
{
	GodClientData::getInstance().copySelection();
	internalDeleteSelection(false);
}

//-----------------------------------------------------------------
/**
 * Handler for copy message.
 */
void ActionsEdit::onDelete() const
{
	internalDeleteSelection(true);
}

//-----------------------------------------------------------------
/**
 * Function used to delete objects from the world, used both by cut and delete
 */
void ActionsEdit::internalDeleteSelection(bool confirm) const
{
	GodClientData * const gcd = &GodClientData::getInstance();

	GodClientData::ObjectList_t olist;
	gcd->getSelection(olist);

	if(!olist.empty() && confirm)
	{
		char buf [1024];
		IGNORE_RETURN(_snprintf(buf, 1024, "Delete %d objects?", olist.size()));
		const int retval = QMessageBox::warning(&MainFrame::getInstance(), "Delete?", buf, QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

		if(retval == QMessageBox::No || retval == QMessageBox::NoButton)
			return;
	}

	gcd->clearSelection();

	for(GodClientData::ObjectList_t::iterator it = olist.begin(); it != olist.end(); ++it)
	{
		ClientObject * const obj = *it;
		if(obj)
			IGNORE_RETURN(ServerCommander::getInstance().deleteObject(obj));
	}
}

//-----------------------------------------------------------------
/**
 * Handler for applying transform message.  Move the objects to the location of their ghosts
 */
void ActionsEdit::onApplyTransform() const
{
	GodClientData::getInstance().synchronizeSelectionWithGhosts();
} 

//-----------------------------------------------------------------

void ActionsEdit::onUnlockSelected() const
{
	GodClientData::getInstance().unlockSelectedObjects();
} 

//-----------------------------------------------------------------

void ActionsEdit::onUnlockAll() const
{
	BuildoutAreaSupport::unlockAll(unlockAll->isOn());
} 

//-----------------------------------------------------------------

void ActionsEdit::onUnlockNonStructures() const
{
	BuildoutAreaSupport::unlockNonStructures(unlockNonStructures->isOn());
} 

//-----------------------------------------------------------------

void ActionsEdit::onUnlockServerOnly() const
{
	BuildoutAreaSupport::unlockServerOnly(unlockServerOnly->isOn());
} 

//-----------------------------------------------------------------
/**
 * Randomly rotate the ghosts.  Use GodClientData for some support data
 */
void ActionsEdit::onRandomRotate() const
{
	const GodClientData::RotationType type = 
		rotateModeRoll->isOn() ? GodClientData::Rotate_roll :(rotateModePitch->isOn() ?  GodClientData::Rotate_pitch : GodClientData::Rotate_yaw);
	
	const GodClientData::RotationPivotType pivotType = static_cast<GodClientData::RotationPivotType>(getRotatePivotMode());

	GodClientData::getInstance().rotateGhosts (Random::randomReal(CONST_REAL(0), PI_TIMES_2), type,   pivotType);
	
}

//-----------------------------------------------------------------
/**
 * Handler for drop to terrain message. Pass it to the GodClientData
 */
void ActionsEdit::onDropToTerrain() const
{
	GodClientData::getInstance().dropGhostsToTerrain();
}

//-----------------------------------------------------------------
/**
 * Handler for toggle drop to terrain message. Pass it to the GodClientData
 */
void ActionsEdit::onToggleDropToTerrain() const
{
	GodClientData::getInstance().toggleDropToTerrain(); 
}

//-----------------------------------------------------------------
/**
 * Handler for toggle align to terrain message. Pass it to the GodClientData
 */
void ActionsEdit::onToggleAlignToTerrain() const
{
	GodClientData::getInstance().toggleAlignToTerrain(); 
}

//-----------------------------------------------------------------

/**
 * Track the currently selected object template
 */
void ActionsEdit::onServerTemplateSelectionChanged(const std::string & str)
{
	m_selectedServerTemplate = str;
	
	const std::string text     = std::string("Create New Server Object [") + str + "]";
	const std::string menuText = std::string("Create &New Server Object [") + str + "]";

	createObjectFromSelectedTemplate->setEnabled (!str.empty());
	createObjectFromSelectedTemplate->setText    (text.c_str());
	createObjectFromSelectedTemplate->setMenuText(menuText.c_str());
}

//-----------------------------------------------------------------

/**
 * Track the currently selected object template
 */
void ActionsEdit::onClientTemplateSelectionChanged(const std::string & str)
{
	m_selectedClientTemplate = str;
	
	const std::string text     = std::string("Create New Client Object [") + str + "]";
	const std::string menuText = std::string("Create &New Client Object [") + str + "]";

	createObjectFromSelectedTemplate->setEnabled (!str.empty());
	createObjectFromSelectedTemplate->setText    (text.c_str());
	createObjectFromSelectedTemplate->setMenuText(menuText.c_str());
}

//-----------------------------------------------------------------

/**
 * Align the selections to the terrain.  Pass it to GodClientData
 */
void ActionsEdit::onAlignToTerrain() const
{
	GodClientData::getInstance().alignGhostsToTerrain();
}

//-----------------------------------------------------------------

/**
 * Reset the rotations of any ghosts
 */
void ActionsEdit::onRotateReset() const
{
	GodClientData::getInstance().resetGhostsRotations();
}

//-----------------------------------------------------------------

/**
 * Reset the rotations of any ghosts
 */
void ActionsEdit::onRotateIncrement45() const
{
	internalRotateDiscrete(PI * CONST_REAL(0.25));
}

//-----------------------------------------------------------------

void ActionsEdit::onRotateDecrement45() const
{
	internalRotateDiscrete(-PI * CONST_REAL(0.25));
}

//-----------------------------------------------------------------

void ActionsEdit::onStoreGroup1() const
{
	GodClientData::getInstance().storeSelection(1);
}

//-----------------------------------------------------------------

void ActionsEdit::onRestoreGroup1() const
{
	GodClientData::getInstance().restoreSelection(1);
}

//-----------------------------------------------------------------

void ActionsEdit::onStoreGroup2() const
{
	GodClientData::getInstance().storeSelection(2);
}

//-----------------------------------------------------------------

void ActionsEdit::onRestoreGroup2() const
{
	GodClientData::getInstance().restoreSelection(2);
}

//-----------------------------------------------------------------

void ActionsEdit::onStoreGroup3() const
{
	GodClientData::getInstance().storeSelection(3);
}

//-----------------------------------------------------------------

void ActionsEdit::onRestoreGroup3() const
{
	GodClientData::getInstance().restoreSelection(3);
}

//-----------------------------------------------------------------

void ActionsEdit::onStoreGroup4() const
{
	GodClientData::getInstance().storeSelection(4);
}

//-----------------------------------------------------------------

void ActionsEdit::onRestoreGroup4() const
{
	GodClientData::getInstance().restoreSelection(4);
}

//----------------------------------------------------------------------

/**
 * Show a dialog that allowing the manual setting of an objects position/rotation
 */
void ActionsEdit::onSetTransform() const
{
	GodClientData::getInstance().setTransformDlg();
}

//-----------------------------------------------------------------

/**
 * Show the triggers for this object.  Allow editing of them.
 */
void ActionsEdit::onEditTriggers() const
{
	TriggerWindow* triggerWindow = new TriggerWindow(&MainFrame::getInstance(), "Trigger Settings");

	GodClientData::ObjectList_t olist;
	GodClientData::getInstance().getSelection(olist);
	if(olist.empty())
		return;
	if(olist.size() > 1)
	{
		IGNORE_RETURN(QMessageBox::warning(0, "Error", "More than 1 object selected, can't view triggers"));
	}

	ClientObject * const obj = *(olist.begin());
	triggerWindow->setNetworkId(obj->getNetworkId());

	triggerWindow->show();
}

//-----------------------------------------------------------------

/**
 * Edit the form Data
 */
void ActionsEdit::onEditFormData() const
{
	GodClientData::ObjectList_t olist;
	GodClientData::getInstance().getSelection(olist);
	if(olist.empty())
		return;
	if(olist.size() > 1)
	{
		IGNORE_RETURN(QMessageBox::warning(0, "Error", "More than 1 object selected, can't edit form data"));
	}

	ClientObject * const obj = *(olist.begin());
	if(obj)
		FormManagerClient::requestEditObject(obj->getNetworkId());
}

//-----------------------------------------------------------------

/**
 * Take the current selection, put it into the clipboard, and copy some of the informtion into the system clipboard.
 * Specifically, put the server template name and relative (to the selection's center) position.  This is useful when
 * building sets for POI's.
 */
void ActionsEdit::onCopyForPOI() const
{
	if(Game::getSinglePlayer())
	{
		IGNORE_RETURN(QMessageBox::warning(0, "Error", "Copy for POI Cannot be used in single player, connect to a server to use this feature."));
		return;
	}

	//put the selection in the clipboard
	GodClientData::getInstance().copySelection();

	//get the GodClient clipboard
	GodClientData::ClipboardList_t objectList;
	GodClientData::getInstance().getClipboard(objectList);

	//build the clipboard text string
	std::string clipboardText;
	QString bufferString;

	//find the clipboard center, since we want to store only relative locations
	Vector center;
	IGNORE_RETURN(GodClientData::getInstance().calculateClipboardCenter(objectList, center));

	bool outputTabFormat = true;

	if(outputTabFormat)
	{
		clipboardText += ActionsEditNamespace::headerLine1;
		clipboardText += ActionsEditNamespace::headerLine2;
	}

	for(GodClientData::ClipboardList_t::iterator itr = objectList.begin(); itr != objectList.end(); ++itr)
	{
		GodClientData::ClipboardObject* clipObj = (*itr);
		clipboardText += clipObj->serverObjectTemplateName;
		clipboardText += "\t";

		const Vector pos                 = clipObj->transform.getPosition_p();
		const Vector posRelativeToCenter = pos - center;

		IGNORE_RETURN(bufferString.setNum(posRelativeToCenter.x));
		clipboardText += bufferString.latin1();
		clipboardText += "\t";

		IGNORE_RETURN(bufferString.setNum(posRelativeToCenter.y));
		clipboardText += bufferString.latin1();
		clipboardText += "\t";
		
		IGNORE_RETURN(bufferString.setNum(posRelativeToCenter.z));
		clipboardText += bufferString.latin1();
		clipboardText += "\t";

		const float yaw = convertRadiansToDegrees(clipObj->transform.getLocalFrameK_p().theta());

		IGNORE_RETURN(bufferString.setNum(yaw));
		clipboardText += bufferString.latin1();

		clipboardText += "\n";
	}

	//get the Qt clipboard and fill it in
	QClipboard* clipboard = QApplication::clipboard();
	clipboard->clear();
	clipboard->setText(clipboardText.c_str());
}

//-----------------------------------------------------------------

void ActionsEdit::onShowObjectAxes() const
{
	GodClientData::ObjectList_t olist;
	GodClientData::getInstance().getSelection(olist);
	if(olist.empty())
		return;

	for(GodClientData::ObjectList_t::iterator i = olist.begin(); i != olist.end(); ++i)
	{
		GodClientData::getInstance().toggleShowObjectAxes(*i);
	}
}

//-----------------------------------------------------------------

void ActionsEdit::onToggleParticleSystemPause() const
{
	GodClientData::ObjectList_t olist;
	GodClientData::getInstance().getSelection(olist);
	if(olist.empty())
		return;

	for(GodClientData::ObjectList_t::iterator i = olist.begin(); i != olist.end(); ++i)
	{
		GodClientData::getInstance().togglePauseParticleSystem(*i);
	}
}

//-----------------------------------------------------------------

void ActionsEdit::internalRotateDiscrete(const real rad) const
{
	const GodClientData::RotationType type = 
		rotateModeRoll->isOn() ? GodClientData::Rotate_roll :(rotateModePitch->isOn() ?  GodClientData::Rotate_pitch : GodClientData::Rotate_yaw);

	const GodClientData::RotationPivotType pivotType = static_cast<GodClientData::RotationPivotType>(getRotatePivotMode());

	GodClientData::getInstance().rotateGhosts(rad, type, pivotType);
}

//-----------------------------------------------------------------

/**
 * Enable/disable selection-related functions as appropriate
 */
void ActionsEdit::setSelectionControlsEnabled(bool b)
{
	copy->setEnabled  (b);
	del->setEnabled   (b);
	cut->setEnabled   (b);
	
	dropToTerrain->setEnabled    (b);
	randomRotate->setEnabled     (b);
	setTransform->setEnabled     (b);
	alignToTerrain->setEnabled   (b);
	rotateReset->setEnabled      (b);
	rotateIncrement45->setEnabled(b);
	rotateDecrement45->setEnabled(b);
	ActionsTool::getInstance().m_saveAsBrush->setEnabled      (b);
}

//-----------------------------------------------------------------

/**
 * Handle MessageDispatch messages.  We care about Selections, clipboards and ghosts.
 *
 */
void ActionsEdit::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	UNREF(source);
		
	GodClientData * const gcd = &GodClientData::getInstance();
	
	if(message.isType(GodClientData::Messages::SELECTION_CHANGED))
	{
		setSelectionControlsEnabled(!gcd->getSelectionEmpty());
	}
	else if(message.isType(GodClientData::Messages::CLIPBOARD_CHANGED))
	{
		const bool b = !gcd->getClipboardEmpty();
		paste->setEnabled (b);
	}	
	else if(message.isType(GodClientData::Messages::GHOSTS_CREATED))
	{
		applyTransform->setEnabled (true);
	}
	else if(message.isType(GodClientData::Messages::GHOSTS_KILLED))
	{
		applyTransform->setEnabled (false);
	}
}

//----------------------------------------------------------------------

/**
 * Return the current rotation pivot mode
 */
int ActionsEdit::getRotatePivotMode() const
{
	return static_cast<int>(
		rotatePivotModeLastSelected->isOn() ?
		GodClientData::RotatePivot_lastSelection :
	(rotatePivotModeCenter->isOn() ?  GodClientData::RotatePivot_selectionCenter : GodClientData::RotatePivot_self));
	
}

//----------------------------------------------------------------------

namespace ActionsEditNamespace
{
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class TheaterData
	{
	public:

		explicit TheaterData (const QString& serverObjectTemplateBaseName);
		~TheaterData ();

	public:

		QString serverObjectTemplateNameTpf;
		QString serverObjectTemplateNameIff;
		QString serverObjectTemplateFullNameTpf;
		QString serverDataTableNameTab;
		QString serverDataTableNameIff;
		QString serverDataTableFullNameTab;
		QString sharedObjectTemplateNameTpf;
		QString sharedObjectTemplateNameIff;
		QString sharedObjectTemplateFullNameTpf;
		QString clientDataFileNameMif;
		QString clientDataFileNameCdf;
		QString clientDataFileFullNameMif;
		mutable QString terrainModificationFileName;

	private:

		TheaterData ();
		TheaterData (const TheaterData&);
		TheaterData& operator= (const TheaterData&);
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	TheaterData::TheaterData (const QString& serverObjectTemplateBaseName)
	{
		//-- object/building/poi/sample_theater.tpf
		serverObjectTemplateNameTpf += ConfigGodClient::getServerObjectTemplateTheaterPath ();
		serverObjectTemplateNameTpf += "/";
		serverObjectTemplateNameTpf += serverObjectTemplateBaseName;
		serverObjectTemplateNameTpf += ".tpf";

		//-- object/building/poi/sample_theater.iff
		serverObjectTemplateNameIff += ConfigGodClient::getServerObjectTemplateTheaterPath ();
		serverObjectTemplateNameIff += "/";
		serverObjectTemplateNameIff += serverObjectTemplateBaseName;
		serverObjectTemplateNameIff += ".iff";

		//-- ../../dsrc/sku.0/sys.server/compiled/game/object/building/poi/sample_theater.tpf
		serverObjectTemplateFullNameTpf += ConfigGodClient::getServerObjectTemplateTheaterDirectory ();
		serverObjectTemplateFullNameTpf += "/";
		serverObjectTemplateFullNameTpf += serverObjectTemplateNameTpf;

		//-- datatables/poi/sample_theater.tab
		serverDataTableNameTab += ConfigGodClient::getServerDataTableTheaterPath ();
		serverDataTableNameTab += "/";
		serverDataTableNameTab += serverObjectTemplateBaseName;
		serverDataTableNameTab += ".tab";

		//-- datatables/poi/sample_theater.iff
		serverDataTableNameIff += ConfigGodClient::getServerDataTableTheaterPath ();
		serverDataTableNameIff += "/";
		serverDataTableNameIff += serverObjectTemplateBaseName;
		serverDataTableNameIff += ".iff";

		//-- ../../dsrc/sku.0/sys.server/compiled/game/object/poi/sample_theater.tab
		serverDataTableFullNameTab += ConfigGodClient::getServerDataTableTheaterDirectory ();
		serverDataTableFullNameTab += "/";
		serverDataTableFullNameTab += serverDataTableNameTab;

		//-- object/building/poi/shared_sample_theater.tpf
		sharedObjectTemplateNameTpf += ConfigGodClient::getSharedObjectTemplateTheaterPath ();
		sharedObjectTemplateNameTpf += "/shared_";
		sharedObjectTemplateNameTpf += serverObjectTemplateBaseName;
		sharedObjectTemplateNameTpf += ".tpf";

		//-- object/building/poi/shared_sample_theater.iff
		sharedObjectTemplateNameIff += ConfigGodClient::getSharedObjectTemplateTheaterPath ();
		sharedObjectTemplateNameIff += "/shared_";
		sharedObjectTemplateNameIff += serverObjectTemplateBaseName;
		sharedObjectTemplateNameIff += ".iff";

		//-- ../../dsrc/sku.0/sys.shared/compiled/game/object/building/poi/shared_sample_theater.tpf
		sharedObjectTemplateFullNameTpf += ConfigGodClient::getSharedObjectTemplateTheaterDirectory ();
		sharedObjectTemplateFullNameTpf += "/";
		sharedObjectTemplateFullNameTpf += sharedObjectTemplateNameTpf;

		//-- clientdata/poi/shared_sample_theater.mif
		clientDataFileNameMif += ConfigGodClient::getClientDataFileTheaterDirectory ();
		clientDataFileNameMif += "/shared_";
		clientDataFileNameMif += serverObjectTemplateBaseName;
		clientDataFileNameMif += ".mif";

		//-- clientdata/poi/shared_sample_theater.cdf
		clientDataFileNameCdf += ConfigGodClient::getClientDataFileTheaterPath ();
		clientDataFileNameCdf += "/shared_";
		clientDataFileNameCdf += serverObjectTemplateBaseName;
		clientDataFileNameCdf += ".cdf";

		//-- ../../dsrc/sku.0/sys.client/compiled/game/clientdata/poi/shared_sample_theater.mif
		clientDataFileFullNameMif += ConfigGodClient::getClientDataFileTheaterDirectory ();
		clientDataFileFullNameMif += "/";
		clientDataFileFullNameMif += clientDataFileNameMif;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	TheaterData::~TheaterData ()
	{
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool createServerObjectTemplate (const ActionsEditNamespace::TheaterData& theaterData)
	{
		FILE* const outfile = fopen (theaterData.serverObjectTemplateFullNameTpf, "wt");
		if (!outfile)
			return false;

		fprintf (outfile, "@base object/building/poi/base/base_poi_building_egg.iff\n\n");
		fprintf (outfile, "@class building_object_template 1\n\n");
		fprintf (outfile, "@class tangible_object_template 3\n\n");
		fprintf (outfile, "@class object_template 8\n\n");
		fprintf (outfile, "sharedTemplate = \"%s\"\n\n", theaterData.sharedObjectTemplateNameIff.latin1 ());
		fprintf (outfile, "objvars = [\n\t\"theater\" = [\n\t\t\"tbl\" = \"%s\",\n\t\t\"spawnMethod\" = 1,\n\t\t\"persist\" = 1]]\n", theaterData.serverDataTableNameIff.latin1 ());
		fclose (outfile);

		return true;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool createServerDataTable (const ActionsEditNamespace::TheaterData& theaterData, GodClientData::ClipboardList_t& objectList)
	{
		FILE* const outfile = fopen (theaterData.serverDataTableFullNameTab, "wt");
		if (!outfile)
			return false;

		fprintf (outfile, "TEMPLATE\tX\tY\tZ\tYAW\n");
		fprintf (outfile, "s\tf\tf\tf\tf\n");

		GodClientData::ClipboardList_t::iterator end = objectList.end ();
		for (GodClientData::ClipboardList_t::iterator iter = objectList.begin (); iter != end; ++iter)
		{
			const GodClientData::ClipboardObject* const object = *iter;
			const char* const serverObjectTemplateName = object->serverObjectTemplateName.c_str ();
			const Transform& transform = object->transform;
			const Vector position = transform.getPosition_p ();
			const float theta = convertRadiansToDegrees (transform.getLocalFrameK_p ().theta ());
			fprintf (outfile, "%s\t%1.2f\t%1.2f\t%1.2f\t%1.2f\n", serverObjectTemplateName, position.x, position.y, position.z, theta);
		}

		fclose (outfile);

		return true;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool createSharedObjectTemplate (const ActionsEditNamespace::TheaterData& theaterData)
	{
		FILE* const outfile = fopen (theaterData.sharedObjectTemplateFullNameTpf, "wt");
		if (!outfile)
			return false;

		fprintf (outfile, "@base  object/building/poi/base/shared_base_poi_building.iff\n\n");
		fprintf (outfile, "@class building_object_template 1\n\n");

		if (!theaterData.terrainModificationFileName.isEmpty ())
			fprintf (outfile, "terrainModificationFileName = \"%s\"\n\n", theaterData.terrainModificationFileName.latin1 ());

		fprintf (outfile, "@class tangible_object_template 7\n\n");
		fprintf (outfile, "@class object_template 7\n\n");
		fprintf (outfile, "clientDataFile = \"%s\"\n", theaterData.clientDataFileNameCdf.latin1 ());
		
		fclose (outfile);

		return true;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool createClientDataFile (const QString& filename, GodClientData::ClipboardList_t& objectList)
	{
		FILE* const outfile = fopen (filename, "wt");
		if (!outfile)
			return false;

		const char* const clientDataFileStart = "\n#include \"../../../../../../include/ClientDataFileManager.h\"\n\nFORM \"CLDF\"\n{\n\tFORM \"0000\"\n\t{\n\n";
		const char* const clientDataFileChildObjectAppearanceTemplateBegin = "\t\tBeginChildObject\n";
		const char* const clientDataFileChildObjectObjectTemplateBegin = "\t\tBeginObjectTemplateChildObject\n";
		const char* const clientDataFileChildObjectAppearanceName = "\t\t\tp1_AppearanceName(\"%s\")\n";
		const char* const clientDataFileChildObjectObjectName = "\t\t\tp1_ObjectName(\"%s\")\n";
		const char* const clientDataFileChildObjectPositionOrientation = "\t\t\tp2_PositionOrientation(%1.2f, %1.2f, %1.2f, %1.2f, 0, 0)\n";
		const char* const clientDataFileChildObjectDynamics = "\t\t\tp3_DynamicsNone\n";
		const char* const clientDataFileChildObjectAppearanceTemplateEnd = "\t\tEndChildObject\n\n";
		const char* const clientDataFileChildObjectObjectTemplateEnd = "\t\tEndObjectTemplateChildObject\n\n";
		const char* const clientDataFileEnd = "\t}\n}\n";

		fprintf (outfile, clientDataFileStart);

		GodClientData::ClipboardList_t::iterator end = objectList.end ();
		for (GodClientData::ClipboardList_t::iterator iter = objectList.begin (); iter != end; ++iter)
		{
			GodClientData::ClipboardObject const * const object = *iter;

			//-- see if we need to use the object template or the appearance template (appearance template is cheaper)
			char const * const objectTemplateName = object->sharedObjectTemplateName.c_str ();
			if (!objectTemplateName || !*objectTemplateName)
				continue;

			bool valid = false;
			bool useAppearanceTemplate = false;
			std::string appearanceTemplateName;

			if (objectTemplateName)
			{
				ObjectTemplate const * const objectTemplate = ObjectTemplateList::fetch (objectTemplateName);
				if (objectTemplate)
				{
					SharedObjectTemplate const * const sharedObjectTemplate = dynamic_cast<SharedObjectTemplate const *> (objectTemplate);
					if (sharedObjectTemplate)
					{
						valid = true;
						useAppearanceTemplate = sharedObjectTemplate->getClientData () == 0;

						if (useAppearanceTemplate)
							appearanceTemplateName = sharedObjectTemplate->getAppearanceFilename ();
					}

					objectTemplate->releaseReference ();
				}
			}

			if (!valid)
				continue;

			const Transform& transform = object->transform;
			const Vector position = transform.getPosition_p ();
			const float theta = convertRadiansToDegrees (transform.getLocalFrameK_p ().theta ());

			if (useAppearanceTemplate)
			{
				fprintf (outfile, clientDataFileChildObjectAppearanceTemplateBegin);
				fprintf (outfile, clientDataFileChildObjectAppearanceName, appearanceTemplateName.c_str ());
				fprintf (outfile, clientDataFileChildObjectPositionOrientation, position.x, position.y, position.z, theta);
				fprintf (outfile, clientDataFileChildObjectDynamics);
				fprintf (outfile, clientDataFileChildObjectAppearanceTemplateEnd);
			}
			else
			{
				fprintf (outfile, clientDataFileChildObjectObjectTemplateBegin);
				fprintf (outfile, clientDataFileChildObjectObjectName, objectTemplateName);
				fprintf (outfile, clientDataFileChildObjectPositionOrientation, position.x, position.y, position.z, theta);
				fprintf (outfile, clientDataFileChildObjectDynamics);
				fprintf (outfile, clientDataFileChildObjectObjectTemplateEnd);
			}
		}

		fprintf (outfile, clientDataFileEnd);
		fclose (outfile);

		return true;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

//----------------------------------------------------------------------

void ActionsEdit::onCreateTheater () const
{
	const char* const operationName = "Create Theater";

	//
	//-- start message
	//
	{
		QString message = "This process will create the server object template, server datatable, and shared object template for the objects you have selected.";
		QMessageBox::information (0, operationName, message, QMessageBox::Ok);
	}

	//
	//-- verify selection 
	//

	//-- put the selection in the clipboard
	GodClientData::getInstance ().copySelection ();
	GodClientData::ClipboardList_t objectList;
	GodClientData::getInstance ().getClipboard (objectList);

	//-- make sure we have objects
	if (objectList.empty ())
	{
		QMessageBox::information (0, operationName, "There are no objects selected.", QMessageBox::Ok);

		return;
	}

	//
	//-- create filenames
	//

	//-- browse for a save name
	QString baseDirectory (ConfigGodClient::getServerObjectTemplateTheaterDirectory ());
	baseDirectory += "/";
	baseDirectory += ConfigGodClient::getServerObjectTemplateTheaterPath ();

	QString serverObjectTemplateBaseName = QFileDialog::getSaveFileName (baseDirectory, "*.tpf", &MainFrame::getInstance (), "", "Select server object template");

	if (serverObjectTemplateBaseName.isEmpty ())
	{
		QMessageBox::information (0, operationName, "Theater creation aborted", QMessageBox::Ok);

		return;
	}
	else
	{
		FileName serverObjectTemplateName (serverObjectTemplateBaseName);
		serverObjectTemplateName.stripPathAndExt ();
		serverObjectTemplateBaseName = serverObjectTemplateName.getString ();
	}

	//
	//-- create files
	//
	const ActionsEditNamespace::TheaterData theaterData (serverObjectTemplateBaseName);

	QString files;
	files += "  ";
	files += theaterData.serverObjectTemplateFullNameTpf;
	files += "\n";
	files += "  ";
	files += theaterData.serverDataTableFullNameTab;
	files += "\n";
	files += "  ";
	files += theaterData.sharedObjectTemplateFullNameTpf;
	files += "\n";

	//-- see if the files exist
	if (QFile::exists (theaterData.serverObjectTemplateFullNameTpf) ||
		QFile::exists (theaterData.serverDataTableFullNameTab) ||
		QFile::exists (theaterData.sharedObjectTemplateFullNameTpf))
	{
		QString message;
		message += "One or more of the following files currently exists:\n";
		message += files;
		message += "\nDo you wish to overwrite them?";

		switch (QMessageBox::warning (0, operationName, message, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No))
		{
		case QMessageBox::No: 
			{
				QMessageBox::information (0, operationName, "Theater creation aborted", QMessageBox::Ok);

				return;
			}
			break;

		default:
			break;
		}
	}

	//-- browse for the layer filename
	QString terrainLayerBaseName = QFileDialog::getOpenFileName (ConfigGodClient::getSharedTerrainDirectory (), "poi*.lay", &MainFrame::getInstance (), "", "Select terrain layer filename");

	if (!terrainLayerBaseName.isEmpty ())
	{
		FileName terrainLayerName (terrainLayerBaseName);
		terrainLayerName.stripPathAndExt ();
		theaterData.terrainModificationFileName = "terrain/";
		theaterData.terrainModificationFileName += terrainLayerName.getString ();
		theaterData.terrainModificationFileName += ".lay";
	}

	//-- create server object template
	if (!ActionsEditNamespace::createServerObjectTemplate (theaterData))
	{
		QString message;
		message += "The following file could not be created:\n  ";
		message += theaterData.serverObjectTemplateFullNameTpf;
		QMessageBox::information (0, operationName, message, QMessageBox::Ok);

		return;
	}

	//-- create server datatable
	if (!ActionsEditNamespace::createServerDataTable (theaterData, objectList))
	{
		QString message;
		message += "The following file could not be created:\n  ";
		message += theaterData.serverDataTableFullNameTab;
		QMessageBox::information (0, operationName, message, QMessageBox::Ok);

		return;
	}

	//-- create shared object template
	if (!ActionsEditNamespace::createSharedObjectTemplate (theaterData))
	{
		QString message;
		message += "The following file could not be created:\n  ";
		message += theaterData.sharedObjectTemplateFullNameTpf;
		QMessageBox::information (0, operationName, message, QMessageBox::Ok);

		return;
	}

	//-- success
	{
		QString message;
		message += "All of the following files were successfully created:\n";
		message += files;
		QMessageBox::information (0, operationName, message, QMessageBox::Ok);
	}

	//-- empty the clipboard
	QClipboard* const clipboard = QApplication::clipboard ();
	clipboard->clear ();
}

//----------------------------------------------------------------------

void ActionsEdit::onCreateClientDataFile () const
{
	const char* const operationName = "Create ClientDataFile";

	//
	//-- start message
	//
	{
		QString message = "This process will create the client data file for the objects you have selected.";
		QMessageBox::information (0, operationName, message, QMessageBox::Ok);
	}

	//
	//-- verify selection 
	//

	//-- put the selection in the clipboard
	GodClientData::getInstance ().copySelection ();
	GodClientData::ClipboardList_t objectList;
	GodClientData::getInstance ().getClipboard (objectList);

	//-- make sure we have objects
	if (objectList.empty ())
	{
		QMessageBox::information (0, operationName, "There are no objects selected.", QMessageBox::Ok);

		return;
	}

	//
	//-- create filenames
	//

	//-- browse for a save name
	QString baseDirectory (ConfigGodClient::getClientDataFileTheaterDirectory ());
	baseDirectory += "/clientdata";

	QString clientDataFileFullName = QFileDialog::getSaveFileName (baseDirectory, "*.mif", &MainFrame::getInstance (), "", "Select ClientClientData File Name");

	if (clientDataFileFullName.isEmpty ())
	{
		QMessageBox::information (0, operationName, "ClientDataFile creation aborted", QMessageBox::Ok);

		return;
	}

	if (clientDataFileFullName.find (".mif") == -1)
		clientDataFileFullName += ".mif";

	//
	//-- create files
	//
	QString files;
	files += "  ";
	files += clientDataFileFullName;
	files += "\n";

	//-- see if the files exist
	if (QFile::exists (clientDataFileFullName))
	{
		QString message;
		message += "The following file currently exists:\n";
		message += files;
		message += "\nDo you wish to overwrite it?";

		switch (QMessageBox::warning (0, operationName, message, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No))
		{
		case QMessageBox::No: 
			{
				QMessageBox::information (0, operationName, "ClientDataFile creation aborted", QMessageBox::Ok);

				return;
			}
			break;

		default:
			break;
		}
	}

	//-- create client data file
	if (!ActionsEditNamespace::createClientDataFile (clientDataFileFullName, objectList))
	{
		QString message;
		message += "The following file could not be created:\n  ";
		message += clientDataFileFullName;
		QMessageBox::information (0, operationName, message, QMessageBox::Ok);

		return;
	}

	//-- success
	{
		QString message;
		message += "The following file was successfully created:\n";
		message += files;
		QMessageBox::information (0, operationName, message, QMessageBox::Ok);
	}

	//-- empty the clipboard
	QClipboard* const clipboard = QApplication::clipboard ();
	clipboard->clear ();
}

// ======================================================================


