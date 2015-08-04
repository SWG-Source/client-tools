//======================================================================
//
// SwgCuiWaypointMonitor.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiWaypointMonitor.h"

//----------------------------------------------------------------------

#include "clientGame/ClientWaypointObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientGraphics/Camera.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedGame/GroundZoneManager.h"
#include "sharedGame/SharedBuildoutAreaManager.h"
#include "sharedGame/Waypoint.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/NetworkIdManager.h"

//----------------------------------------------------------------------

#include "UIData.h"
#include "UIDataSource.h"
#include "UIImage.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIPopupMenu.h"
#include "UIText.h"

//======================================================================

SwgCuiWaypointMonitor::SwgCuiWaypointMonitor    (UIPage & page, const NetworkId & waypointId) :
CuiMediator     ("SwgCuiWaypointMonitor", page),
UIEventCallback (),
m_textName      (0),
m_textDistance  (0),
m_arrow         (0),
m_callback      (new MessageDispatch::Callback),
m_waypointId    (new NetworkId (waypointId)),
m_lastDistance  (-1),
m_elapsedTime   (0.0f)
{
	getCodeDataObject (TUIText,  m_textName,     "textName");
	getCodeDataObject (TUIText,  m_textDistance, "textDistance");
	getCodeDataObject (TUIImage, m_arrow,        "arrow");
	getCodeDataObject (TUIImage, m_waypointMarker,"waypoint");

	m_textName->SetPreLocalized     (true);
	m_textDistance->SetPreLocalized (true);

	registerMediatorObject (getPage (), true);
}

//----------------------------------------------------------------------

SwgCuiWaypointMonitor::~SwgCuiWaypointMonitor ()
{
	delete m_callback;
	m_callback = 0;

	delete m_waypointId;
	m_waypointId = 0;
}

//----------------------------------------------------------------------

void SwgCuiWaypointMonitor::performActivate   () 
{
	setIsUpdating (true);
	refresh (100.0f);
}

//----------------------------------------------------------------------

void SwgCuiWaypointMonitor::performDeactivate ()
{
	setIsUpdating (false);
}

//----------------------------------------------------------------------

bool SwgCuiWaypointMonitor::close                    ()
{
	return CuiMediator::close ();
}

//----------------------------------------------------------------------

bool SwgCuiWaypointMonitor::OnMessage                (UIWidget * context, const UIMessage & msg)
{
	if (context == &getPage ())
	{
		if (msg.Type == UIMessage::LeftMouseDoubleClick)
		{
			ClientWaypointObject * const clientWaypoint = safe_cast<ClientWaypointObject *>(NetworkIdManager::getObjectById (*m_waypointId));
			if (clientWaypoint)
				clientWaypoint->setWaypointActive (false);
			
			return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------

void SwgCuiWaypointMonitor::update                   (float deltaTimeSecs)
{
	//-- update every half second
	m_elapsedTime += deltaTimeSecs;
	refresh (m_elapsedTime);
}

//----------------------------------------------------------------------

void SwgCuiWaypointMonitor::refresh (float elapsedTimeToUse)
{
	ClientWaypointObject * const clientWaypoint = safe_cast<ClientWaypointObject *>(NetworkIdManager::getObjectById (*m_waypointId));
	if (!clientWaypoint)
		return;
	
	const Object * const player = Game::getPlayer ();
	if (!player)
		return;

	const VectorArgb &      colorArgb = clientWaypoint->getColorArgb     ();
	const UIColor    &      color     = CuiUtils::convertColor (colorArgb);
	
	Vector const & waypointLocation_w = clientWaypoint->getWorldWaypointLocation_w();

	//-- only update the name, distance & color every 200 ms
	if (elapsedTimeToUse > 0.20f)
	{
		m_elapsedTime = 0.0f;
		const Unicode::String & name      = clientWaypoint->getLocalizedName ();
		m_textName->SetTextColor (color);
		if (m_textName->GetLocalText () != name)
			m_textName->SetLocalText (name);
		
		Vector player_pos_w = player->getPosition_w();
		//only test x-z distance in the ground game
		if(!Game::isSpace())
			player_pos_w.y = clientWaypoint->getRelativeWaypointPosition().y;
		const int distance = static_cast<int>(clientWaypoint->getRelativeWaypointPosition().magnitudeBetween(player_pos_w));
		if (distance != m_lastDistance)
		{
			static char buf [64];
			static const size_t buf_size = sizeof (buf);
			snprintf (buf, buf_size, "%dm", distance);
			m_lastDistance = distance;
			m_textDistance->SetLocalText (Unicode::narrowToWide (buf));
		}
	}

	//-- update the arrow angle every frame
	Camera const * const camera = Game::getCamera ();
	if (camera)
	{
		Vector const & cameraPos_w = camera->getPosition_w();
		
		if (Game::isSpace())
		{
			Vector waypointPos = clientWaypoint->getLocation();
			Vector delta_vector = waypointLocation_w - cameraPos_w;
			Vector const delta_in_camera = camera->rotate_w2o(delta_vector);
			bool const normalized = delta_vector.normalize();
			float const theta = atan2(delta_in_camera.x, delta_in_camera.y);
			float const dotProduct = camera->getObjectFrameK_w().dot(delta_vector);
			if (!normalized || acos(dotProduct) < std::min(camera->getVerticalFieldOfView(), camera->getHorizontalFieldOfView()) / 2.f)
			{
				m_arrow->SetVisible(false);
				m_waypointMarker->SetVisible(true);
			}
			else
			{
				m_arrow->SetVisible(true);
				m_arrow->SetRotation(theta / PI_TIMES_2 - 0.25f);
				m_arrow->SetColor(color);	
				m_waypointMarker->SetVisible(false);
			}		
		}
		else
		{
			Vector waypointPos = clientWaypoint->getRelativeWaypointPosition();
			Vector waypointPos_w = waypointLocation_w;
			waypointPos_w.y = cameraPos_w.y;				
			const float camera_angle  = camera->getObjectFrameK_w ().theta ();
			const Vector delta_vector = waypointPos_w - cameraPos_w;
			const float target_angle  = delta_vector.theta ();
			const float theta = target_angle - camera_angle;
			m_arrow->SetRotation (theta / PI_TIMES_2 - 0.25f);
			m_arrow->SetColor    (color);	
		}		
	}
}

//----------------------------------------------------------------------

void SwgCuiWaypointMonitor::setTarget                (const NetworkId & waypointId)
{
	*m_waypointId = waypointId;
	refresh (100.0f);
}

//======================================================================
