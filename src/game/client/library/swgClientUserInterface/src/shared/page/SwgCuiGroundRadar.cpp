//======================================================================
//
// SwgCuiGroundRadar.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiGroundRadar.h"
#include "swgClientUserInterface/SwgCuiGroundRadar_BlipPane.h"
#include "swgClientUserInterface/SwgCuiGroundRadar_WaypointPane.h"

#include "clientGame/ClientRegionManager.h"
#include "clientGame/ClientTextManager.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/ShipStation.h"
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
#include "clientUserInterface/CuiWidgetGroundRadar.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedGame/GroundZoneManager.h"
#include "sharedGame/PlanetMapManager.h"
#include "sharedGame/SharedBuildoutAreaManager.h"
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
	const double CON_MODE_ANGLE = 0.8;
	const double ZOOM_TOP_ANGLE = 0.4;
	const double ZOOM_BOTTOM_ANGLE = -0.4;

	const int MAX_RADAR_RANGE = 1024;
	const int MIN_RADAR_RANGE = 32;

	struct RadarMinMaxRange
	{
		RadarMinMaxRange () : minRadarRange(MIN_RADAR_RANGE), maxRadarRange(MAX_RADAR_RANGE) {};

		int minRadarRange;
		int maxRadarRange;
	};

	typedef stdmap<std::string, RadarMinMaxRange>::fwd RadarMinMaxRangeList;

	int                  s_currentMaxRadarRange = MAX_RADAR_RANGE;
	int                  s_currentMinRadarRange = MIN_RADAR_RANGE;

	RadarMinMaxRangeList s_radarMinMaxRangeList;

	namespace Settings
	{
		const std::string s_range = "range";
		const std::string s_conMode = "conMode";
	}

	//----------------------------------------------------------------------

	void loadRadarMinMaxRangeList ()
	{
		if (s_radarMinMaxRangeList.empty ())
		{
			const std::string radarMinMaxRangeIff ("datatables/radar/radar_range.iff");
			RadarMinMaxRange radarMinMaxRange;

			DataTable * const t = DataTableManager::getTable (radarMinMaxRangeIff, true);
			if (!t)
			{
				WARNING (true, ("loadRadarMinMaxRangeList() - unable to load data table (%s)", radarMinMaxRangeIff.c_str ()));
			}
			else
			{
				for (int row = 0; row < t->getNumRows (); ++row)
				{
					radarMinMaxRange.minRadarRange = t->getIntValue ("MinRadarRange", row);
					radarMinMaxRange.maxRadarRange = t->getIntValue ("MaxRadarRange", row);

					// use default min/max range
					if (radarMinMaxRange.minRadarRange == -1)
						radarMinMaxRange.minRadarRange = MIN_RADAR_RANGE;

					if (radarMinMaxRange.maxRadarRange == -1)
						radarMinMaxRange.maxRadarRange = MAX_RADAR_RANGE;

					s_radarMinMaxRangeList[t->getStringValue ("Planet", row)] = radarMinMaxRange;
				}

				DataTableManager::close (radarMinMaxRangeIff);
			}
		}
	}

	//----------------------------------------------------------------------

	void updateRadarMinMaxRange ()
	{
		// see if there's a scene specific radar range
		RadarMinMaxRangeList::const_iterator iterFind = s_radarMinMaxRangeList.find (Game::getSceneId ());

		if (iterFind != s_radarMinMaxRangeList.end ())
		{
			s_currentMaxRadarRange = iterFind->second.maxRadarRange;
			s_currentMinRadarRange = iterFind->second.minRadarRange;
		}
		else
		{
			// use default radar range
			s_currentMaxRadarRange = MAX_RADAR_RANGE;
			s_currentMinRadarRange = MIN_RADAR_RANGE;
		}
	}

	Unicode::String s_toolTip;
}

//-----------------------------------------------------------------

class SwgCuiGroundRadar::SwgCuiGroundRadarAction : public CuiAction
{
public:
	
	SwgCuiGroundRadarAction (SwgCuiGroundRadar & radar) : CuiAction (), m_radar (radar) {}
	
	bool  performAction (const std::string & id, const Unicode::String &params) const
	{
		if (id == SwgCuiActions::radarRangeDecrease)
			m_radar.zoomIn ();			
		else if (id == SwgCuiActions::radarRangeIncrease)
			m_radar.zoomOut ();		
		else if (id == SwgCuiActions::radarConMode)
			m_radar.setConMode (atoi(Unicode::wideToNarrow(params).c_str()) != 0);
		else if (id == SwgCuiActions::radarAlwaysShowRange)
		{
			bool const alwaysShow = atoi(Unicode::wideToNarrow(params).c_str()) != 0;

			m_radar.setAlwaysShowRangeInRadar(alwaysShow);
		}
		else
			return false;

		return true;
	}
	
private:

	SwgCuiGroundRadarAction ();
	SwgCuiGroundRadarAction (const SwgCuiGroundRadarAction & rhs);
	SwgCuiGroundRadarAction & operator= (const SwgCuiGroundRadarAction & rhs);

	SwgCuiGroundRadar & m_radar;
};

//----------------------------------------------------------------------

SwgCuiGroundRadar::SwgCuiGroundRadar (UIPage & page) :
SwgCuiLockableMediator ("SwgCuiGroundRadar", page),
m_radarCompass         (0),
m_radarCompassTop      (0),
m_radarCompassTopSmall (0),
m_radarCompassTopMedium(0),
m_radarCompassTopLarge (0),
m_groundRadar          (0),
m_rangeIndicatorText   (0),
m_regionIndicatorText  (0),
m_lastRadarUpdate      (0.0f),
m_topZoomButton        (0),
m_bottomZoomButton     (0), 
m_iconImage            (0),
m_blipImage(0),
m_blipPane             (0),
m_waypointPane         (0),
m_radarRange           (64),
m_radarDirty           (true),
m_action               (0),
m_textLat              (0),
m_textHeight(0),
m_textLong             (0),
m_lastCoord            (-99999,-99999),
m_pageSquare           (0),
m_conModeCheckbox      (0),
m_conMode              (0),
m_lastRegion           (),
m_previousSize         (),
m_radarSkinSmall       (0),
m_radarSkinMedium      (0),
m_radarSkinLarge       (0),
m_radarSkinSmallWidth  (0),
m_radarSkinMediumWidth (0),
m_radarSkinLargeWidth  (0)
{
	UIWidget *widget = 0;
	getCodeDataObject (TUIWidget, widget, "RadarWidget");
	m_groundRadar = NON_NULL (dynamic_cast<CuiWidgetGroundRadar *>(widget));

	getCodeDataObject (TUIImage,    m_radarSkinSmall,    "RadarSkinSmall");
	getCodeDataObject (TUIImage,    m_radarSkinMedium,   "RadarSkinMedium");
	getCodeDataObject (TUIImage,    m_radarSkinLarge,    "RadarSkinLarge");

	getCodeDataObject (TUIImage,    m_radarCompass,       "RadarCompass");
	getCodeDataObject (TUIPage,     m_radarCompassTop,    "RadarCompassTop");
	getCodeDataObject (TUIImage,    m_radarCompassTopSmall, "RadarCompassTopSmall");
	getCodeDataObject (TUIImage,    m_radarCompassTopMedium,"RadarCompassTopMedium");
	getCodeDataObject (TUIImage,    m_radarCompassTopLarge, "RadarCompassTopLarge");
	getCodeDataObject (TUIText,     m_rangeIndicatorText, "RangeIndicator");
	getCodeDataObject (TUIText,     m_regionIndicatorText,"RegionIndicator");
	getCodeDataObject (TUIButton,   m_topZoomButton,      "ZoomTop");
	getCodeDataObject (TUIButton,   m_bottomZoomButton,   "ZoomBottom");

	getCodeDataObject (TUIText,     m_textLat,            "textLat");
	getCodeDataObject (TUIText,     m_textHeight,         "textHeight");
	getCodeDataObject (TUIText,     m_textLong,           "textLong");
	
	getCodeDataObject (TUIPage,     m_pageSquare,         "square");

	getCodeDataObject (TUICheckbox, m_conModeCheckbox,    "conMode");
	
	UIImage * iconImage = 0;
	getCodeDataObject (TUIImage,    iconImage,          "iconImage");
	m_iconImage = NON_NULL (dynamic_cast<UIImage *>(iconImage->DuplicateObject ()));
	iconImage->SetVisible (false);
	
	UIImage * arrowImage = 0;
	UIImage * arrowWaypointImage = 0;
	UIImage * arrowBracket       = 0;
	UIImage * arrowGroupImage    = 0;
	
	getCodeDataObject (TUIImage,     arrowImage,               "arrow");
	getCodeDataObject (TUIImage,     arrowWaypointImage,       "arrowWaypoint");
	getCodeDataObject (TUIImage,     arrowBracket,             "arrowBracket");
	getCodeDataObject (TUIImage,     arrowGroupImage,          "arrowGroup");
	
	UIColorEffector * effectorWaypoint = 0;
	getCodeDataObject (TUIColorEffector,     effectorWaypoint,             "effectorWaypoint");
	
	arrowWaypointImage->ExecuteEffector (*effectorWaypoint);

	arrowImage->SetVisible         (false);
	arrowWaypointImage->SetVisible (false);
	arrowBracket->SetVisible       (false);
	arrowGroupImage->SetVisible    (false);
	
	m_textLong->SetPreLocalized (true);
	m_textHeight->SetPreLocalized  (true);
	m_textLat->SetPreLocalized  (true);
	
	getPage ().SetVisible (true); 
	
	setLockedAspectRatio (UIFloatPoint::one);
	
	UIPage * const blipPaneParent = safe_cast<UIPage *>(arrowImage->GetParentWidget ());
	blipPaneParent->SetVisible (false);
	
	m_waypointPane = new WaypointPane (*m_groundRadar, *arrowImage, *arrowWaypointImage, *arrowGroupImage, *arrowBracket);
	m_pageSquare->InsertChildBefore (m_waypointPane, blipPaneParent);
	
	Unicode::String tmp;
	
	m_waypointPane->SetLocation(m_groundRadar->GetLocation());
	m_waypointPane->SetSize(m_groundRadar->GetSize());
	
	{
		UIImage * blip          = 0;
		UIImage * blipCorpse    = 0;
		UIImage * blipWaypoint  = 0;
		UIImage * blipStructure = 0;
		UIImage * blipBracket   = 0;
		UIImage * blipGroup     = 0;		
		UIImage * blipRing      = 0;
		UIImage * blipEntrance  = 0;

		getCodeDataObject (TUIImage, blip,          "blipDefault");
		getCodeDataObject (TUIImage, blipCorpse,    "blipCorpse");
		getCodeDataObject (TUIImage, blipWaypoint,  "blipWaypoint");
		getCodeDataObject (TUIImage, blipStructure, "blipStructure");
		getCodeDataObject (TUIImage, blipBracket,   "blipBracket");
		getCodeDataObject (TUIImage, blipGroup,     "blipGroup");
		getCodeDataObject (TUIImage, blipRing,      "blipRing");
		getCodeDataObject (TUIImage, blipEntrance,  "blipEntrance");

		blipWaypoint->ExecuteEffector (*effectorWaypoint);
		
		BlipPane * const blipPane = new BlipPane (*m_groundRadar, *blip, *blipCorpse, *blipWaypoint, *blipStructure, *blipBracket, *blipGroup, *blipRing, *blipEntrance, *m_waypointPane, getConMode());
		m_pageSquare->InsertChildAfter (blipPane, m_waypointPane);
		m_blipPane = blipPane;

		m_blipPane->SetLocation(m_groundRadar->GetLocation());
		m_blipPane->SetSize(m_groundRadar->GetSize());

	}

	//----------------------------------------------------------------------
	//-- setup the icon

	CuiWorkspaceIcon * const icon = new CuiWorkspaceIcon (this);
	icon->SetName ("RadarIcon");
	icon->SetSize (UISize (32, 32));
	icon->SetBackgroundColor (UIColor (0, 0, 0, 50));
	icon->SetLocation (0, 64);

	m_iconImage->SetSize (icon->GetSize ());
	m_iconImage->SetLocation (0, 0);
	m_iconImage->SetProperty (UIWidget::PropertyName::PackSize, Unicode::narrowToWide ("1,1"));
	icon->AddChild (m_iconImage);
	m_iconImage->Link ();

	setIcon (icon);

	//-----------------------------------------------------------------

	m_action = new SwgCuiGroundRadarAction (*this);

	CuiActionManager::addAction (SwgCuiActions::radarRangeDecrease,    m_action, false);
	CuiActionManager::addAction (SwgCuiActions::radarRangeIncrease,    m_action, false);
	CuiActionManager::addAction (SwgCuiActions::radarConMode,          m_action, false);
	CuiActionManager::addAction (SwgCuiActions::radarAlwaysShowRange,  m_action, false);

	registerMediatorObject (*m_topZoomButton,    true);
	registerMediatorObject (*m_bottomZoomButton, true);
	registerMediatorObject (*m_conModeCheckbox,    true);
	registerMediatorObject (getPage (),          true);
	
	//-----------------------------------------------------------------
	// one time loading of the per-planet radar min/max range list

	loadRadarMinMaxRangeList();

	getPage().SetLockDiagonal(true);

	m_radarSkinSmall->GetPropertyInteger(UILowerString("SkinWidth"), m_radarSkinSmallWidth);
	m_radarSkinMedium->GetPropertyInteger(UILowerString("SkinWidth"), m_radarSkinMediumWidth);
	m_radarSkinLarge->GetPropertyInteger(UILowerString("SkinWidth"), m_radarSkinLargeWidth);

	m_radarSkinSmall->SetVisible(false);
	m_radarSkinMedium->SetVisible(false);
	m_radarSkinLarge->SetVisible(false);

	m_radarCompassTopSmall->SetVisible(false);
	m_radarCompassTopMedium->SetVisible(false);
	m_radarCompassTopLarge->SetVisible(false);

	getPage().GetPropertyInteger(UILowerString("RadarToParentOffset"), m_radarToParentOffset);
}

//----------------------------------------------------------------------

SwgCuiGroundRadar::~SwgCuiGroundRadar ()
{	
	getPage().SetTooltipCallback(NULL);

	CuiActionManager::removeAction (m_action);
	delete m_action;
	m_action = 0;

	m_groundRadar        = 0;
	m_radarCompass       = 0;
	m_radarCompassTop    = 0;
	m_radarCompassTopSmall = 0;
	m_radarCompassTopMedium = 0;
	m_radarCompassTopLarge = 0;
	m_rangeIndicatorText = 0;
	m_regionIndicatorText = 0;
	m_topZoomButton      = 0;
	m_bottomZoomButton   = 0;

	m_textLat  = 0;
	m_textHeight = 0;
	m_textLong = 0;
	m_iconImage = 0;
	m_blipImage = 0;
	m_blipPane = 0;
	m_waypointPane = 0;
}

//-----------------------------------------------------------------

void SwgCuiGroundRadar::OnCheckboxSet( UIWidget *Context )
{
	if(Context == m_conModeCheckbox)
		CuiActionManager::performAction(SwgCuiActions::radarConMode, Unicode::narrowToWide("1"));
}

//-----------------------------------------------------------------
	
void SwgCuiGroundRadar::OnCheckboxUnset( UIWidget *Context )
{
	if(Context == m_conModeCheckbox)
		CuiActionManager::performAction(SwgCuiActions::radarConMode, Unicode::narrowToWide("0"));
}

//-----------------------------------------------------------------

void SwgCuiGroundRadar::performActivate()
{
	if(Game::isTutorial())
	{
		deactivate();
		return;
	}

	setIsUpdating (true);

	// set the radar's min/max range based on the scene
	updateRadarMinMaxRange ();

	if (m_blipPane) 
	{
		m_blipPane->resetSearchCache();
	}

	getPage().SetTooltipCallback(this);
	m_previousSize = getPage().GetSize();

	UIRect pageRect = getPage().GetRect();
	OnWidgetRectChanging(&getPage(), pageRect);

	setAlwaysShowRangeInRadar(CuiPreferences::getAlwaysShowRangeInGroundRadar());
}

//-----------------------------------------------------------------

void SwgCuiGroundRadar::performDeactivate()
{
	setIsUpdating (false);

	getPage().SetTooltipCallback(NULL);
}

//-----------------------------------------------------------------

void  SwgCuiGroundRadar::updateRadar (const Vector & pos, const ClientProceduralTerrainAppearance * cmtat, bool force)
{
	if (!getPage ().IsVisible ())
		return;

	static char buf[64];
	static const size_t buf_size = sizeof (buf);

	const float range = static_cast<float>(m_radarRange);
	
	const Object * const player = Game::getPlayer ();
	const CreatureObject * const playerObject = Game::getPlayerCreature();

	bool terrainEnabled = CuiPreferences::getGroundRadarTerrainEnabled ();

	BuildoutArea const * const ba = SharedBuildoutAreaManager::findBuildoutAreaAtPosition(pos.x, pos.z, true);
	if (NULL != ba)
	{
		if (!ba->allowRadarTerrain)
			terrainEnabled = false;
	}
	if(Game::isSpace() && playerObject && (playerObject->getShipStation() == ShipStation::ShipStation_None))
		terrainEnabled = false;


	if (terrainEnabled && !m_groundRadar->IsEnabled ())
		force = true;

	m_groundRadar->SetEnabled (terrainEnabled);

	if((force || m_radarDirty) && m_blipPane)
	{
		m_blipPane->setConMode(getConMode());
	}

	if (m_groundRadar->updateRadarShader (pos, range, cmtat, force || m_radarDirty))
		m_lastRadarUpdate = Game::getElapsedTime ();

	if (force || m_radarDirty)
	{

		snprintf (buf, buf_size,  "%1.0f m", range);
		m_rangeIndicatorText->SetText (Unicode::narrowToWide (buf));
	}


	// display the region
	StringId regionId = ClientRegionManager::getLastCurrentRegion();
	if(regionId != m_lastRegion)
	{
		m_lastRegion = regionId;
		Unicode::String regionString;
		regionString.clear();
		if(regionId.isValid())
		{
			regionString = regionId.localize();
		}
		else
		{
			const Object * const player = Game::getPlayer ();
			if(player)
			{
				std::string zoneName;
				GroundZoneManager::getZoneName(Game::getSceneId().c_str(), player->getPosition_w(), zoneName);			
				regionString = StringId ("zone_n", zoneName).localize ();			
			}
		}
		m_regionIndicatorText->SetText(regionString);
	}


	{
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
				m_radarTooltip = Unicode::narrowToWide(buf);
			}

			m_lastCoord = pt;
		}
	}

	m_radarDirty = false;
}

//-----------------------------------------------------------------

void SwgCuiGroundRadar::setAngle (float theta, bool force)
{
	m_groundRadar->setAngle (theta, force);
	static const float ONE_OVER_PI_TIMES_2 = RECIP (PI_TIMES_2);

	const float uiRotation = - theta * ONE_OVER_PI_TIMES_2;
	m_radarCompass->SetRotation    (uiRotation);
	m_radarCompassTop->SetRotation (uiRotation);
	m_iconImage->SetRotation       (uiRotation);
}

//-----------------------------------------------------------------

bool SwgCuiGroundRadar::OnMessage (UIWidget * context, const UIMessage & msg)
{
	if (context == &getPage ())
	{
		if (msg.Type == UIMessage::MouseWheel)
		{
			if (msg.Data > 0)
				zoomIn ();
			else if (msg.Data < 0)
				zoomOut ();

			return false;
		}
		else if (msg.Type == UIMessage::LeftMouseUp)
		{
			UIColor color;

			UIRect const & worldRect = getPage().GetWorldRect();

			// 4,4 is a pixel offset between altRadarFg page and its parent page.
			UIPoint const point(msg.MouseCoords + worldRect.Location() - UIPoint(4,4)); 

			ClientObject const * const clientObject = m_blipPane ? m_blipPane->getToolTipObjectAtPoint(point, color) : 0;

			// only allow selection of tangible objects the player can see
			if (clientObject && clientObject->asTangibleObject())
			{
				CreatureObject * const player = Game::getPlayerCreature();

				if (player && player != clientObject)
					player->setIntendedTarget(clientObject->getNetworkId());
			}
		}
		else if (msg.Type == UIMessage::RightMouseUp)
		{
			generateLockablePopup(context, msg);
			return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------

void SwgCuiGroundRadar::OnButtonPressed (UIWidget *)
{
}

//----------------------------------------------------------------------

void SwgCuiGroundRadar::onSceneChanged ()
{
	// set the radar's min/max range based on the new scene
	updateRadarMinMaxRange ();

	// make sure current radar zoom setting is within the radar min/max range
	if (m_radarRange > s_currentMaxRadarRange)
	{
		m_radarRange = s_currentMaxRadarRange;
		m_radarDirty = true;
		saveSettings ();
	}
	else if (m_radarRange < s_currentMinRadarRange)
	{
		m_radarRange = s_currentMinRadarRange;
		m_radarDirty = true;
		saveSettings ();
	}
}

//-----------------------------------------------------------------

void SwgCuiGroundRadar::zoomIn ()
{
	if (m_radarRange > s_currentMinRadarRange)
	{
		m_radarRange /= 2;
		m_radarDirty = true;
		CuiSoundManager::play (CuiSounds::item_zoom_in_st);
		saveSettings ();
	}
}

//-----------------------------------------------------------------

void SwgCuiGroundRadar::zoomOut ()
{
	if (m_radarRange < s_currentMaxRadarRange)
	{
		m_radarRange *= 2;
		m_radarDirty = true;
		CuiSoundManager::play (CuiSounds::item_zoom_out_st);
		saveSettings ();
	}
}

//----------------------------------------------------------------------

void SwgCuiGroundRadar::setConMode (bool conMode)
{
	m_conMode = conMode;	
	m_radarDirty = true;
	saveSettings();
}

//----------------------------------------------------------------------

bool SwgCuiGroundRadar::getConMode () const
{
	return m_conMode;
}

//----------------------------------------------------------------------

void SwgCuiGroundRadar::update             (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	getPage().SetResizeInset( static_cast<unsigned char>((getPage().GetWidth() - m_groundRadar->GetWidth()) / 2) );

	m_blipPane->SetVisible(true);
	m_waypointPane->SetVisible(true);
	m_blipPane->SetLocation(m_groundRadar->GetLocation());
	m_blipPane->SetSize(m_groundRadar->GetSize());
	m_waypointPane->SetLocation(m_groundRadar->GetLocation());
	m_waypointPane->SetSize(m_groundRadar->GetSize());

	int radarWidth = m_groundRadar->GetWidth();
	if(radarWidth < (m_radarSkinSmallWidth + m_radarSkinMediumWidth) / 2)
	{
		m_radarSkinSmall->SetVisible(true);
		m_radarSkinMedium->SetVisible(false);
		m_radarSkinLarge->SetVisible(false);
		m_radarCompassTopSmall->SetVisible(true);
		m_radarCompassTopMedium->SetVisible(false);
		m_radarCompassTopLarge->SetVisible(false);
	}
	else if(radarWidth < (m_radarSkinMediumWidth + m_radarSkinLargeWidth) / 2)
	{
		m_radarSkinSmall->SetVisible(false);
		m_radarSkinMedium->SetVisible(true);
		m_radarSkinLarge->SetVisible(false);
		m_radarCompassTopSmall->SetVisible(false);
		m_radarCompassTopMedium->SetVisible(true);
		m_radarCompassTopLarge->SetVisible(false);
	}
	else
	{
		m_radarSkinSmall->SetVisible(false);
		m_radarSkinMedium->SetVisible(false);
		m_radarSkinLarge->SetVisible(true);
		m_radarCompassTopSmall->SetVisible(false);
		m_radarCompassTopMedium->SetVisible(false);
		m_radarCompassTopLarge->SetVisible(true);
	}


	const Camera * const camera = Game::getCamera ();

	if (camera)
	{
		const real theta = camera->getObjectFrameK_w ().theta ();
		setAngle (theta, false);
	}

	const TerrainObject * const terrainObject = TerrainObject::getInstance();
	const ClientProceduralTerrainAppearance * const cmtat = terrainObject ? dynamic_cast<const ClientProceduralTerrainAppearance *> (terrainObject->getAppearance ()) : 0;			
	const Object * const player = Game::getPlayer ();

	const float curTime = Game::getElapsedTime ();

	if (player)
	{
		updateRadar (player->getPosition_w (), cmtat, curTime > (m_lastRadarUpdate + 3.0f));
	}
}

//----------------------------------------------------------------------

void SwgCuiGroundRadar::saveSettings         () const
{
	SwgCuiLockableMediator::saveSettings ();
	CuiSettings::saveInteger (getMediatorDebugName (), Settings::s_range, m_radarRange);
	CuiSettings::saveBoolean (getMediatorDebugName (), Settings::s_conMode, m_conMode);
}

//----------------------------------------------------------------------

void SwgCuiGroundRadar::loadSettings         ()
{
	SwgCuiLockableMediator::loadSettings ();

	int range = 0;
	if (CuiSettings::loadInteger (getMediatorDebugName (), Settings::s_range, range))
	{
		range = clamp (s_currentMinRadarRange, range, s_currentMaxRadarRange);
		m_radarRange = range;
		m_radarDirty = true;
	}
	bool conMode = false;
	if (CuiSettings::loadBoolean (getMediatorDebugName (), Settings::s_conMode, conMode))
	{
		m_conMode = conMode;
		m_conModeCheckbox->SetChecked(m_conMode);
		m_radarDirty = true;
	}

	UIRect pageRect = getPage().GetRect();
	OnWidgetRectChanging(&getPage(), pageRect);
}

//----------------------------------------------------------------------

UIString const & SwgCuiGroundRadar::getTooltip(UIPoint const & p)
{
	s_toolTip = Unicode::emptyString;

	if (m_blipPane)
	{	
		UIRect const & worldRect = getPage().GetWorldRect();

		// 4,4 is a pixel offset between altRadarFg page and its parent page.
		UIPoint point(p + worldRect.Location() - UIPoint(4,4)); 

		UIColor tipColor;

		Object const * object = m_blipPane->getToolTipObjectAtPoint(point, tipColor);
		if (object) 
		{
			ClientObject const * const clientObject = object->asClientObject();
			CreatureObject const * const creatureObject = clientObject ? clientObject->asCreatureObject() : NULL;
			if (creatureObject && creatureObject->getRiderDriverCreature()) 
			{
				object = creatureObject->getRiderDriverCreature();
			}

			Unicode::String header;
			Unicode::String desc;
			Unicode::String attribs;
			
			if (ObjectAttributeManager::formatDescription(object->getNetworkId(), header, desc, attribs, true))
			{
				PackedRgb textColor(tipColor.r, tipColor.g, tipColor.b);

				s_toolTip = ClientTextManager::getColorCode(textColor);
					
				s_toolTip += header;


				if (!desc.empty() && (desc != header) && (header.find(desc) == std::string::npos) && (desc.find(header) == std::string::npos))
				{
					s_toolTip += ClientTextManager::getColorCode(PackedRgb::solidWhite);
					s_toolTip += Unicode::narrowToWide("\n");
					s_toolTip += desc;
				}

				s_toolTip += ClientTextManager::getColorCode(PackedRgb::solidGreen);

				Object const * const player = Game::getPlayer();
				if (player) 
				{
					Vector objectLocation;
					float distanceToObject = 0.0f;

					ClientWaypointObject const * const clientWaypoint = dynamic_cast<ClientWaypointObject const *>(object);
					if (clientWaypoint)
					{
						objectLocation = clientWaypoint->getLocation();
						distanceToObject = player->getPosition_w().magnitudeBetween(objectLocation);
					}
					else
					{
						objectLocation = object->getPosition_w();
						distanceToObject = player->getDistanceBetweenCollisionSpheres_w(*object);
					}

					FormattedString<128> fs;
					s_toolTip += Unicode::narrowToWide(fs.sprintf("\nLocation: %.0f, %.0f, %.0f", objectLocation.x, objectLocation.y, objectLocation.z));
					s_toolTip += Unicode::narrowToWide(fs.sprintf("\nDistance: %.0fm", ceilf(std::max(0.0f, distanceToObject))));
				}
			}
		}
		else
			return m_radarTooltip;
	}

	return s_toolTip;
}

//----------------------------------------------------------------------

void SwgCuiGroundRadar::OnSizeChanged(UIWidget * context)
{
	UNREF(context);
}

//----------------------------------------------------------------------

void SwgCuiGroundRadar::OnWidgetRectChanging (UIWidget * context, UIRect & targetRect)
{
	if(context == &getPage())
	{
		positionWidgetAroundRadarWithRect(m_conModeCheckbox, CON_MODE_ANGLE, targetRect);
		positionWidgetAroundRadarWithRect(m_topZoomButton, ZOOM_TOP_ANGLE, targetRect);
		positionWidgetAroundRadarWithRect(m_bottomZoomButton, ZOOM_BOTTOM_ANGLE, targetRect);
		m_blipPane->SetVisible(false);
		m_waypointPane->SetVisible(false);
	}
}
void SwgCuiGroundRadar::positionWidgetAroundRadarWithRect(UIWidget *widget, double angle, UIRect & targetRect)
{
	int radarToParentOffset = (m_radarToParentOffset * m_groundRadar->GetWidth()) / m_radarSkinSmallWidth;
	if(targetRect.right < targetRect.left + m_radarSkinSmallWidth + m_radarToParentOffset * 2)
		targetRect.right = targetRect.left + m_radarSkinSmallWidth + m_radarToParentOffset * 2;
	if(targetRect.bottom < targetRect.top + m_radarSkinSmallWidth + m_radarToParentOffset * 2)
		targetRect.bottom = targetRect.top + m_radarSkinSmallWidth + m_radarToParentOffset * 2;
	UIPoint squareCenter(static_cast<int>((targetRect.right - targetRect.left) / 2), static_cast<int>((targetRect.bottom - targetRect.top) / 2));
	UIPoint offset(static_cast<int>((targetRect.right - targetRect.left - radarToParentOffset * 2 - 2) * cos(angle) / 2), static_cast<int>((targetRect.bottom - targetRect.top - radarToParentOffset * 2 - 2) * sin(angle) / 2));

	int borderOffsetX, borderOffsetY;
	widget->GetPropertyInteger(UILowerString("BorderOffsetX"), borderOffsetX);
	widget->GetPropertyInteger(UILowerString("BorderOffsetY"), borderOffsetY);

	UIPoint finalLocation(static_cast<int>(squareCenter.x + offset.x - borderOffsetX), static_cast<int>(squareCenter.y - offset.y - borderOffsetY));
	widget->SetLocation(finalLocation);
	return;
}

//----------------------------------------------------------------------

void SwgCuiGroundRadar::setAlwaysShowRangeInRadar(bool alwaysShow)
{
	if (alwaysShow)
	{
		getPage().SetPropertyBoolean(UILowerString("RangeFadeEnabled"), false);
		m_rangeIndicatorText->SetOpacity(1.0f);
	}
	else
	{
		getPage().SetPropertyBoolean(UILowerString("RangeFadeEnabled"), true);
		m_rangeIndicatorText->SetOpacity(0.f);
	}
}

//======================================================================
