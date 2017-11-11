//======================================================================
//
// SwgCuiLocationDisplay.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiLocationDisplay.h"

#include "clientGame/ClientRegionManager.h"
#include "clientGame/ClientTextManager.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientObject/GameCamera.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientUserInterface/CuiAction.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedGame/GroundZoneManager.h"
#include "sharedGame/PlanetMapManager.h"
#include "sharedMath/PackedRgb.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "swgClientUserInterface/SwgCuiActions.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIColorEffector.h"
#include "UIData.h"
#include "UIImage.h"
#include "UIMessage.h"
#include "UIText.h"

#include "UnicodeUtils.h"

#include <map>

//======================================================================

namespace
{
	Unicode::String s_toolTip;
}

//----------------------------------------------------------------------

SwgCuiLocationDisplay::SwgCuiLocationDisplay (UIPage & page) :
SwgCuiLockableMediator            ("SwgCuiLocationDisplay", page),
m_textLat              (0),
m_textHeight(0),
m_textLong             (0),
m_lastCoord            (-99999,-99999),
m_lastRegion           ()
{
	getCodeDataObject (TUIText,     m_textLat,            "textLat");
	getCodeDataObject (TUIText,     m_textHeight,         "textHeight");
	getCodeDataObject (TUIText,     m_textLong,           "textLong");
	
	m_textLong->SetPreLocalized (true);
	m_textHeight->SetPreLocalized  (true);
	m_textLat->SetPreLocalized  (true);

	setStickyVisible (true);
	setShowFocusedGlowRect(false);
	getPage ().SetVisible (true);

	registerMediatorObject(page, true);
}

//----------------------------------------------------------------------

SwgCuiLocationDisplay::~SwgCuiLocationDisplay ()
{	
	m_textLat  = 0;
	m_textHeight = 0;
	m_textLong = 0;
}

//-----------------------------------------------------------------

void SwgCuiLocationDisplay::performActivate()
{
	if(!CuiPreferences::getLocationDisplayEnabled() || Game::isTutorial())
		deactivate();
	else
		setIsUpdating (true);
}

//-----------------------------------------------------------------

void SwgCuiLocationDisplay::performDeactivate()
{
	setIsUpdating (false);
}

//-----------------------------------------------------------------

void  SwgCuiLocationDisplay::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);
	if (!getPage ().IsVisible ())
		return;

	static char buf[64];
	static const size_t buf_size = sizeof (buf);

	{
		const Object * const player = Game::getPlayer ();
		if(player)
		{
			Vector fictionalPos = GroundZoneManager::transformWorldLocationToZoneLocation(Game::getSceneId().c_str(), player->getPosition_w ());
			std::string zoneName;
			GroundZoneManager::getZoneName(Game::getSceneId().c_str(), player->getPosition_w(), zoneName);
			fictionalPos = GroundZoneManager::adjustForNonSquareZones(zoneName.c_str(), fictionalPos);

			const UIPoint pt (static_cast<long>(fictionalPos.x), static_cast<long>(fictionalPos.z));
			if (m_lastCoord.x != pt.x)
			{
				snprintf (buf, buf_size, "%d", pt.x);
				m_textLong->SetLocalText (Unicode::narrowToWide (buf));
			}

			{
				long const y = static_cast<long>(fictionalPos.y);
				snprintf (buf, buf_size, "%d", y);
				m_textHeight->SetLocalText (Unicode::narrowToWide (buf));
			}

			if (m_lastCoord.y != pt.y)
			{
				snprintf (buf, buf_size, "%d", pt.y);
				m_textLat->SetLocalText (Unicode::narrowToWide (buf));
			}

			if((m_lastCoord.x != pt.x) || (m_lastCoord.y != pt.y))
			{
				snprintf (buf, buf_size, "%d, %d", pt.x, pt.y);
				m_textLat->GetParentWidget()->SetLocalTooltip(Unicode::narrowToWide(buf));
			}

			m_lastCoord = pt;
		}
	}
}


//======================================================================
