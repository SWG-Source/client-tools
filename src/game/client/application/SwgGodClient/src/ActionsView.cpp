// ======================================================================
//
// ActionsView.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "ActionsView.h"
#include "ActionsView.moc"

#include "clientGame/ClientWorld.h"
#include "clientGame/GroundScene.h"
#include "clientGame/FreeCamera.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"

#include "sharedCollision/BoxExtent.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"

#include "ActionHack.h"
#include "BookmarkData.h"
#include "GodClientData.h"
#include "IconLoader.h"
#include "MainFrame.h"
#include "ServerCommander.h"

#include <ctime>
#include <qdatetime.h>
#include <qmessagebox.h>

// ======================================================================

ActionsView::ActionsView()
: QObject(),
  Singleton<ActionsView>(),
  MessageDispatch::Receiver(),
  addCameraBookmark(0),
  addObjectBookmark(0),
  removeObjectBookmark(0),
  centerSelection(0),
  centerGhosts(0),
  moveToCursor(0),
  fitSelection(0),
  fitGhosts(0),
  showTriggerVolumes(0),
  turnOffSpheres(0),
  resetCamera(0)
{
	QWidget* const p =&MainFrame::getInstance();

	//create all the "view" actions, used in toolbars, menus, and manually
	addCameraBookmark    = new ActionHack("Add Camera Bookmark",        IL_PIXMAP(hi16_action_bookmark_add),     "Add&Camera Bookmark",     QT_ACCEL2(CTRL,Key_B), p,     "add_camera_bookmark");
	IGNORE_RETURN(connect(addCameraBookmark,    SIGNAL(activated()), this, SLOT(onAddCameraBookmark())));

	addObjectBookmark    = new ActionHack("Add Object Bookmark",        IL_PIXMAP(hi16_action_bookmark_red_add), "Add&Object Bookmark",     QT_ACCEL2(CTRL,Key_O), p,     "add_object_bookmark");
	IGNORE_RETURN(connect(addObjectBookmark,    SIGNAL(activated()), this, SLOT(onAddObjectBookmark())));

	removeObjectBookmark = new ActionHack("Remove Object Bookmark",     QIconSet(),                              "&Remove Object Bookmark",  QT_ACCEL3(CTRL,ALT,Key_O), p, "remove_object_bookmark");
	IGNORE_RETURN(connect(removeObjectBookmark, SIGNAL(activated()), this, SLOT(onAddObjectBookmark())));

	centerSelection      = new ActionHack("Center Camera on Selection", IL_PIXMAP(hi16_action_center_selection), "Center&Selection",        QT_ACCEL2(CTRL,Key_E), p,     "center_selection");
	IGNORE_RETURN(connect(centerSelection,      SIGNAL(activated()), this, SLOT(onCenterSelection())));

	centerGhosts         = new ActionHack("Center Camera on Ghosts",    IL_PIXMAP(hi16_action_center_ghosts),    "Center&Ghosts",           QT_ACCEL3(CTRL,ALT,Key_E), p, "center_ghosts");
	IGNORE_RETURN(connect(centerGhosts,         SIGNAL(activated()), this, SLOT(onCenterGhosts())));

	moveToCursor        = new ActionHack("Move camera to cursor",       IL_PIXMAP(hi16_action_center_selection), "MoveTo&Cursor",           QT_ACCEL(Key_C), p,     "move_to_cursor");
	IGNORE_RETURN(connect(moveToCursor,      SIGNAL(activated()), this, SLOT(onMoveToCursor())));

	fitSelection         = new ActionHack("Fit Selection to Screen",    IL_PIXMAP(hi16_action_fit_selection), "&Fit Selection to Screen", QT_ACCEL2(CTRL,Key_F), p,     "fit_selection");
	IGNORE_RETURN(connect(fitSelection,         SIGNAL(activated()), this, SLOT(onFitSelection())));

	fitGhosts            = new ActionHack("Fit Ghosts to Screen",       IL_PIXMAP(hi16_action_fit_ghosts),    "Fit&Ghosts to Screen",    QT_ACCEL3(CTRL,ALT,Key_F), p, "fit_Ghosts");
	IGNORE_RETURN(connect(fitGhosts,            SIGNAL(activated()), this, SLOT(onFitGhosts())));

	showTriggerVolumes   = new ActionHack("Show Trigger Volumes",       IL_PIXMAP(hi16_action_fit_ghosts),    "&Show Trigger Volumes",      0, p, "show_trigger_volumes");
	IGNORE_RETURN(connect(showTriggerVolumes,   SIGNAL(activated()), this, SLOT(onShowTriggerVolumes())));

	turnOffSpheres       = new ActionHack("Turn Off Debug Spheres",     IL_PIXMAP(hi16_action_fit_ghosts),    "&Turn Off Debug Spheres",    0, p, "turn_off_spheres");
	IGNORE_RETURN(connect(turnOffSpheres,       SIGNAL(activated()), this, SLOT(onTurnOffSpheres())));

	resetCamera          = new ActionHack("Reset Camera",               QIconSet(),                           "Reset &Camera",              0, p, "reset_camera");
	IGNORE_RETURN(connect(resetCamera,       SIGNAL(activated()), this, SLOT(onResetCamera())));

	centerGhosts->setEnabled   (false);
	centerSelection->setEnabled(false);

	connectToMessage(GodClientData::Messages::SELECTION_CHANGED);
	connectToMessage(GodClientData::Messages::GHOSTS_CREATED);
	connectToMessage(GodClientData::Messages::GHOSTS_KILLED);
}

//----------------------------------------------------------------------

ActionsView::~ActionsView()
{
	addCameraBookmark    = 0;
	addObjectBookmark    = 0;
	removeObjectBookmark = 0;
	centerSelection      = 0;
	centerGhosts         = 0;
	moveToCursor         = 0;
	fitSelection         = 0;
	fitGhosts            = 0;
	showTriggerVolumes   = 0;
	turnOffSpheres       = 0;
	resetCamera          = 0;
}

//-----------------------------------------------------------------

/**
 * Add a new camera bookmark, use the current bookmark location.
 *
 */
void ActionsView::onAddCameraBookmark() const
{	
	GroundScene* const m_gs = dynamic_cast<GroundScene*>(Game::getScene());

	if(m_gs == 0)
		return;

	FreeCamera* const camera = NON_NULL(m_gs->getGodClientCamera());

	const FreeCamera::Info& info = camera->getInfo();

	const time_t t = time(0);
	BookmarkData::getInstance().addCameraBookmark(ctime(&t), info);
}

//-----------------------------------------------------------------

void ActionsView::onAddObjectBookmark() const
{
}

//-----------------------------------------------------------------

void ActionsView::onRemoveObjectBookmark() const
{
}

//-----------------------------------------------------------------

/**
 * Move the camera to center on the selected objects
 */
void ActionsView::onCenterSelection() const
{
	internalCenterSelection(false, Fit_dontCare);
}

//-----------------------------------------------------------------

/**
 * Move the camera to center on the ghosts
 */
void ActionsView::onCenterGhosts() const
{
	internalCenterSelection(true, Fit_dontCare);
}

//-----------------------------------------------------------------

/**
 * Move the camera to center on the selected objects but positioned above the cursor
 */
void ActionsView::onMoveToCursor() const
{
	float const eyeHeight = 2.5f;
	float const pivotDist = 5.f;

	GroundScene* const gs = dynamic_cast<GroundScene*>(Game::getScene());

	if(gs == 0)
		return;

	FreeCamera* const camera = NON_NULL(gs->getGodClientCamera());

	GodClientData * gcd =  &GodClientData::getInstance();

	if (gcd->getSelectionEmpty())
	{
		//-- if nothing is selected, position the camera at eye height above the cursor position
		camera->setPivotPoint(gcd->getMouseCursorIntersection() + Vector(0.f, eyeHeight, 0.f));
		camera->setPivotDistance(pivotDist);
	}
	else
	{
		//-- if something is selected, position the camera just above the cursor and center on it
		Vector v = gcd->getMouseCursorIntersection() + Vector(0.f, eyeHeight, 0.f);
		camera->setPosition_p(v);

		onCenterSelection();
	}
}

//-----------------------------------------------------------------

void ActionsView::internalCenterSelection(bool ghosts, FitType fit) const
{
	Vector ghostCenter;
	
	if(GodClientData::getInstance().calculateSelectionCenter(ghostCenter, ghosts))
	{
		
		static QTime last_centertime[2] = 
		{
			QTime(),
			QTime()
		};
		
		centerCameraOnPoint(ghostCenter, true, static_cast<int>(fit));
		
		//-----------------------------------------------------------------
		//-- decide whether to fit
		
		if(fit == Fit_dontCare)
		{
			QTime& last = ghosts ? last_centertime [1] : last_centertime [0];
			const QTime cur_centertime = QTime::currentTime();
			fit = last.msecsTo(cur_centertime) < 500 ? Fit_yes : Fit_no;
			last = cur_centertime;
		}

		if(fit == Fit_yes)
		{
			BoxExtent extent;
			
			if(GodClientData::getInstance().getSelectionExtent(false, extent))
			{
				GroundScene* const m_gs = dynamic_cast<GroundScene*>(Game::getScene());
				
				if(m_gs == 0)
					return;

				FreeCamera* camera = m_gs->getGodClientCamera();
				FreeCamera::Info target;
				camera->getTargetInfo(target);
				target.distance = extent.getSphere().getRadius()* CONST_REAL(1.5);
				camera->setTargetInfo(target, CONST_REAL(1));
			}
		}
	}
}

//-----------------------------------------------------------------

void ActionsView::centerCameraOnPoint(const Vector& pos, bool interpolate, int fit) const
{
	GroundScene* const m_gs = dynamic_cast<GroundScene*>(Game::getScene());

	if(m_gs == 0)
		return;

	m_gs->activateGodClientCamera();
	
	FreeCamera* const camera = m_gs->getGodClientCamera();
	
	const Vector cpos = camera->getPosition_w();
	
	FreeCamera::Info target;
	target.translate = pos;
	target.distance  = pos.magnitudeBetween(camera->getPosition_w());
	target.yaw       = camera->getYaw();
	
	const Vector dv = pos - cpos;
	
	if(fabs(dv.x) > CONST_REAL(0.01) || fabs(dv.z) > CONST_REAL(0.01))
	{
		target.yaw = dv.theta();
	}
	
	target.pitch = dv.phi();
	
	if(fit == static_cast<int>(Fit_point))
	{
		static QTime last_centertime = QTime();
		const QTime cur_centertime = QTime::currentTime();
		if(last_centertime.msecsTo(cur_centertime) < 500)
		{
			target.distance*= CONST_REAL(0.3);
		}

		last_centertime = cur_centertime;
	}

	camera->setTargetInfo(target, CONST_REAL(1));

	camera->setInterpolating(interpolate);
	camera->setMode(FreeCamera::M_pivot);	
}

//-----------------------------------------------------------------

void ActionsView::centerCameraOnObject(const Object& obj, bool interpolate, int fit) const
{
	static NetworkId last_networkId = obj.getNetworkId();
	static QTime last_centertime = QTime();
	const QTime cur_centertime = QTime::currentTime();		

	if(last_networkId == obj.getNetworkId()&& fit == static_cast<int>(Fit_dontCare))
	{
		const int diff = last_centertime.msecsTo(cur_centertime);
		fit = static_cast<int>(diff < 500 ? Fit_yes : Fit_no);
	}

	last_networkId = obj.getNetworkId();
	last_centertime = cur_centertime;
	
	centerCameraOnPoint(obj.getAppearanceSphereCenter_w(), interpolate, fit);
	if(fit == static_cast<int>(Fit_yes))
	{
		GroundScene* const m_gs = dynamic_cast<GroundScene*>(Game::getScene());
		
		if(m_gs == 0)
			return;		
		
		FreeCamera* camera = m_gs->getGodClientCamera();
		FreeCamera::Info target;
		camera->getTargetInfo(target);
		target.distance = obj.getAppearanceSphereRadius()* CONST_REAL(2.0);
		camera->setTargetInfo(target, CONST_REAL(1));
	}
}

//-----------------------------------------------------------------

void ActionsView::onFitSelection() const
{
	internalCenterSelection(false, Fit_yes);
}

//-----------------------------------------------------------------

void ActionsView::onFitGhosts() const
{
	internalCenterSelection(true, Fit_yes);
}

//-----------------------------------------------------------------

/**
 * Get a snapshot of the trigger volumes for the currently selected objects, and show them
 *
 */
void ActionsView::onShowTriggerVolumes() const
{
	GodClientData::getInstance().showTriggerVolumes();
}

//-----------------------------------------------------------------

/**
 * Turn off the tracking of all debug sphere, specifically trigger sphere and the sphere tree representation
 *
 */
void ActionsView::onTurnOffSpheres() const
{
	GodClientData::getInstance().clearSpheres();
}

//-----------------------------------------------------------------

/**
 * Move the camera to the world cell, and to the avatar's world position.  A good command for when the camera get's whacked.
 */
void ActionsView::onResetCamera() const
{
	GroundScene* const gs = dynamic_cast<GroundScene*>(Game::getScene());

	if(gs == 0)
		return;

	FreeCamera* const camera = NON_NULL(gs->getGodClientCamera());
	camera->setParentCell(CellProperty::getWorldCellProperty());
	const Object* player    = gs->getPlayer();
	const Vector& playerLoc = player->getPosition_w();
	camera->setPivotPoint(playerLoc);
	camera->setPivotDistance(1.0f);
}

//-----------------------------------------------------------------

/**
 * Handle MessageDispatch messages.  We care about selections and ghosts
 */
void ActionsView::receiveMessage(const MessageDispatch::Emitter& source, const MessageDispatch::MessageBase& message)
{
	UNREF(source);
	
	GroundScene* const gs = dynamic_cast<GroundScene*>(Game::getScene());

	if(gs == 0)
		return;
	
	GodClientData* const gcd =&GodClientData::getInstance();
	
	if(message.isType(GodClientData::Messages::SELECTION_CHANGED))
	{
		const bool b = !gcd->getSelectionEmpty();
		centerSelection->setEnabled(b);
		fitSelection->setEnabled   (b);
	}
	else if(message.isType(GodClientData::Messages::GHOSTS_CREATED))
	{
		centerGhosts->setEnabled(true);
		fitGhosts->setEnabled   (true);
	}
	else if(message.isType(GodClientData::Messages::GHOSTS_KILLED))
	{
		centerGhosts->setEnabled(false);
		fitGhosts->setEnabled   (false);
	}
}

//-----------------------------------------------------------------

/**
 * Move the camera to a camera bookmark
 */
void ActionsView::gotoCameraBookmark(int index) const
{
	GroundScene* const m_gs = dynamic_cast<GroundScene*>(Game::getScene());

	if(m_gs == 0 || index < 0)
		return;

	const BookmarkData::CameraBookmark& cam = BookmarkData::getInstance().getCameraBookmark(static_cast<size_t>(index));

	FreeCamera* const camera = NON_NULL(m_gs->getGodClientCamera());
	camera->setTargetInfo(cam.info, CONST_REAL(1));
	camera->setInterpolating(true);
}

//-----------------------------------------------------------------

/**
 * Move the camera to an object bookmark
 */
void ActionsView::gotoObjectBookmark(int index) const
{
	GroundScene* const m_gs = dynamic_cast<GroundScene*>(Game::getScene());

	if(m_gs == 0 || index < 0)
		return;
	
	const BookmarkData::ObjectBookmark& cam = BookmarkData::getInstance().getObjectBookmark(static_cast<size_t>(index));

	Object* const obj = NetworkIdManager::getObjectById (cam.id);
		
	if(obj == 0)
	{
		char buf [128];
		IGNORE_RETURN(_snprintf(buf, 128, "Object with Network ID %d, name %s not found in world.\n", cam.id, cam.cameraBookmark.name.c_str()));
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", buf));
		return;
	}
	
	m_gs->activateGodClientCamera();
	FreeCamera* const camera = m_gs->getGodClientCamera();
	
	FreeCamera::Info target;
	target.translate = obj->getAppearanceSphereCenter_w();
	target.distance  = cam.cameraBookmark.info.distance;
	target.yaw       = cam.cameraBookmark.info.yaw + obj->getObjectFrameK_p().theta();
	target.pitch     = cam.cameraBookmark.info.pitch;
	camera->setTargetInfo(target, CONST_REAL(1));
	camera->setInterpolating(true);
	camera->setMode(FreeCamera::M_pivot);
}

// ======================================================================
