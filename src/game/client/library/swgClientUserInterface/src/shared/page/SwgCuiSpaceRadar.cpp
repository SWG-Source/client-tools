//======================================================================
//
// SwgCuiSpaceRadar.cpp
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiSpaceRadar.h"

#include "UIBaseObject.h"
#include "UIButton.h"
#include "UIColorEffector.h"
#include "UIData.h"
#include "UIImage.h"
#include "UIMessage.h"
#include "UIText.h"
#include "UnicodeUtils.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerShipController.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipStation.h"
#include "clientGraphics/Graphics.h"
#include "clientUserInterface/CuiAction.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"
#include "sharedGame/PlanetMapManager.h"
#include "sharedUtility/Callback.h"
#include "swgClientUserInterface/SwgCuiActions.h"
#include "swgClientUserInterface/SwgCuiSpaceRadarPlanar.h"
#include "swgClientUserInterface/SwgCuiSpaceRadarSpherical.h"
#include "swgClientUserInterface/SwgCuiSpaceRadarSpherical2.h"

#include <unordered_map>
#include <list>

//======================================================================

namespace SwgCuiSpaceRadarNamespace
{
	const int MAX_RADAR_RANGE = 8192;
	const int MIN_RADAR_RANGE = 64;
	
	namespace Settings
	{
		const std::string range = "range";
	}

	char const * const c_radarSphericalPage = "radarSpherical";
	char const * const c_radarSpherical2Page = "radarSpherical2";
	char const * const c_radarPlanarPage = "radarPlanar";
}

using namespace SwgCuiSpaceRadarNamespace;

//-----------------------------------------------------------------

class SwgCuiSpaceRadar::SwgCuiSpaceRadarAction : public CuiAction
{
public:

	explicit SwgCuiSpaceRadarAction(SwgCuiSpaceRadar& radar) : CuiAction(), m_radar(radar) {}
	
	bool performAction(const std::string & id, const Unicode::String &) const
	{
		if (id == SwgCuiActions::radarRangeDecrease)
			m_radar.zoomIn();			
		else if (id == SwgCuiActions::radarRangeIncrease)
			m_radar.zoomOut();
		else
			return false;
		
		return true;
	}
	
private:
	
	SwgCuiSpaceRadarAction();
	SwgCuiSpaceRadarAction(const SwgCuiSpaceRadarAction & rhs);
	SwgCuiSpaceRadarAction & operator=(const SwgCuiSpaceRadarAction & rhs);

	SwgCuiSpaceRadar& m_radar;
};

//-----------------------------------------------------------------

SwgCuiSpaceRadar::SwgCuiSpaceRadar(UIPage & page) :
SwgCuiLockableMediator("SwgCuiSpaceRadar", page), 
CallbackReceiver(),
m_lastRadarUpdate(0.0f),
m_radarDirty(true),
m_iconImage(NULL),
m_lastCoord(),
m_lastSize(),
m_topZoomButton(NULL),
m_bottomZoomButton(NULL), 
m_textLat(NULL),
m_textHeight(NULL),
m_textLong(NULL),
m_rangeIndicatorText(NULL),
m_action(0),
m_radarMediators(new CuiMediatorVector),
m_radarActiveIndex(-1),
m_stationImageMap(new StationToImageMap),
m_stationImageCurrent(NULL)
{
	getPage().SetVisible(true); 
	setLockedAspectRatio(UIFloatPoint::one);
	registerMediatorObject(getPage(), true);

	//----------------------------------------------------------------------
	//-- setup the icon

	UIImage * iconImage = 0;
	getCodeDataObject(TUIImage, iconImage, "iconImage");
	m_iconImage = NON_NULL(dynamic_cast<UIImage *>(iconImage->DuplicateObject()));
	iconImage->SetVisible(false);
	
	CuiWorkspaceIcon * const icon = new CuiWorkspaceIcon(this);
	icon->SetName("RadarIcon");
	icon->SetSize(UISize(32, 32));
	icon->SetBackgroundColor(UIColor(0, 0, 0, 50));
	icon->SetLocation(0, 64);
	
	m_iconImage->SetSize(icon->GetSize());
	m_iconImage->SetLocation(0, 0);
	IGNORE_RETURN(m_iconImage->SetProperty(UIWidget::PropertyName::PackSize, Unicode::narrowToWide("1,1")));
	icon->AddChild(m_iconImage);
	m_iconImage->Link();

	setIcon(icon);

	//-----------------------------------------------------------------
	m_action = new SwgCuiSpaceRadarAction(*this);

	getCodeDataObject(TUIButton,   m_topZoomButton,      "ZoomTop");
	getCodeDataObject(TUIButton,   m_bottomZoomButton,   "ZoomBottom");

	getCodeDataObject(TUIText,     m_textLat,            "textLat");
	getCodeDataObject(TUIText,     m_textHeight,         "textHeight");
	getCodeDataObject(TUIText,     m_textLong,           "textLong");

	getCodeDataObject(TUIText,     m_rangeIndicatorText, "RangeIndicator");
	
	
	CuiActionManager::addAction(SwgCuiActions::radarRangeDecrease,    m_action, false);
	CuiActionManager::addAction(SwgCuiActions::radarRangeIncrease,    m_action, false);

	//-----------------------------------------------------------------
	buildRadarPages();
	setRadarActiveIndex(CuiPreferences::getRadarSelect());
	CuiPreferences::getRadarSelectCallback().attachReceiver(*this);

	//-----------------------------------------------------------------
	initializeStations();

	//-----------------------------------------------------------------
	registerMediatorObject(*m_topZoomButton, true);
	registerMediatorObject(*m_bottomZoomButton, true);
	registerMediatorObject(getPage(), true);
}

//----------------------------------------------------------------------

SwgCuiSpaceRadar::~SwgCuiSpaceRadar()
{	
	CuiPreferences::getRadarSelectCallback().detachReceiver(*this);

	releaseRadarPages();

	NOT_NULL(m_action);
	CuiActionManager::removeAction(m_action);
	delete m_action;
	m_action = 0;
	
	m_rangeIndicatorText = 0;
	m_topZoomButton = 0;
	m_bottomZoomButton = 0;
	
	m_textLat  = 0;
	m_textHeight = 0;
	m_textLong = 0;

	m_iconImage = 0;

	delete m_stationImageMap;
	m_stationImageMap = NULL;
	m_stationImageCurrent = NULL;
}

//-----------------------------------------------------------------

void SwgCuiSpaceRadar::performActivate()
{
	setIsUpdating(true);
	
	CuiMediator * activePage = getRadarActiveMediator();
	if (activePage)
	{
		activePage->activate();
	}
}

//-----------------------------------------------------------------

void SwgCuiSpaceRadar::performDeactivate()
{
	setIsUpdating(false);

	CuiMediator * activePage = getRadarActiveMediator();
	if (activePage)
	{
		activePage->deactivate();
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceRadar::update(float deltaTimeSecs)
{
	CuiMediator::update(deltaTimeSecs);
	
	CreatureObject const * const player = Game::getPlayerCreature();
	float const curTime = Game::getElapsedTime();
	ShipStation::Type shipStationIndex = ShipStation::ShipStation_Pilot;
	
	if (player)
	{
		updateRadar(player->getPosition_w(), curTime > (m_lastRadarUpdate + 1.0f));

		ShipObject const * const shipObject = Game::getPlayerContainingShip();
		if(shipObject)
		{
			// Set the station index.
			
			// Check for AutoPilot.
			bool isAutoPilot = false;
			PlayerShipController const * const playerShipController = dynamic_cast<PlayerShipController const *>(shipObject->getController());;
			if (playerShipController)
			{
				isAutoPilot = playerShipController->isAutoPilotEngaged() || playerShipController->isFollowing() || shipObject->hasCondition(TangibleObject::C_docking);
			}
		
			shipStationIndex = isAutoPilot ? ShipStation::ShipStation_None : static_cast<ShipStation::Type>(player->getShipStation());
		}
	}

	setStationIcon(shipStationIndex);
}

//----------------------------------------------------------------------

void SwgCuiSpaceRadar::saveSettings() const
{
	SwgCuiLockableMediator::saveSettings();
	CuiSettings::saveInteger(getMediatorDebugName(), Settings::range, static_cast<int>(Game::getRadarRange()));
}

//----------------------------------------------------------------------

void SwgCuiSpaceRadar::loadSettings()
{
	SwgCuiLockableMediator::loadSettings();
	
	int range = 0;
	if (CuiSettings::loadInteger(getMediatorDebugName(), Settings::range, range))
	{		
		range = clamp(MIN_RADAR_RANGE, range, MAX_RADAR_RANGE);
		Game::setRadarRange(static_cast<float>(range));
		m_radarDirty = true;
	}
}


//-----------------------------------------------------------------

void SwgCuiSpaceRadar::zoomIn()
{
	if (Game::getRadarRange() > MIN_RADAR_RANGE)
	{
		m_radarDirty = true;
		Game::setRadarRange(Game::getRadarRange() / 2.f);
		CuiSoundManager::play(CuiSounds::item_zoom_in_st);
		saveSettings();
	}
}

//-----------------------------------------------------------------

void SwgCuiSpaceRadar::zoomOut()
{
	if (Game::getRadarRange() < MAX_RADAR_RANGE)
	{
		m_radarDirty = true;
		Game::setRadarRange(Game::getRadarRange() * 2.f);
		CuiSoundManager::play(CuiSounds::item_zoom_out_st);
		saveSettings();
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceRadar::OnButtonPressed(UIWidget *)
{
}

//-----------------------------------------------------------------

bool SwgCuiSpaceRadar::OnMessage(UIWidget * context, const UIMessage & msg)
{
	if (context == &getPage())
	{
		if (msg.Type == UIMessage::MouseWheel)
		{
			if (msg.Modifiers.isControlDown())
			{
				if (msg.Data > 0)
					zoomIn();
				else if (msg.Data < 0)
					zoomOut();
				
				return false;
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

//-----------------------------------------------------------------

void  SwgCuiSpaceRadar::updateRadar(const Vector & pos, bool )
{
	if (!getPage().IsVisible())
		return;
	
	static char buf[64];
	static const size_t buf_size = sizeof(buf);
	{
		const Vector & fictionalPos = PlanetMapManager::convertPositionToFictional(Game::getSceneId(), pos);
		
		const UIPoint pt(static_cast<long>(fictionalPos.x), static_cast<long>(fictionalPos.z));
	
		snprintf(buf, buf_size, "%d", pt.x);
		m_textLong->SetLocalText(Unicode::narrowToWide(buf));
		
		long const y = static_cast<long>(fictionalPos.y);
		snprintf(buf, buf_size, "%d", y);
		m_textHeight->SetLocalText(Unicode::narrowToWide(buf));
		
		snprintf(buf, buf_size, "%d", pt.y);
		m_textLat->SetLocalText(Unicode::narrowToWide(buf));

		snprintf(buf, buf_size, "%d", static_cast<int>(Game::getRadarRange()));
		m_rangeIndicatorText->SetLocalText(Unicode::narrowToWide(buf));
		
		m_lastCoord = pt;
	}

	m_radarDirty = false;
}

//-----------------------------------------------------------------

void SwgCuiSpaceRadar::buildRadarPages()
{
	UIPage * radarPages = NULL;
	getCodeDataObject(TUIPage, radarPages, "RadarTypes");

	UIBaseObject::UIObjectList const & childObjects = radarPages->GetChildrenRef();
	for (UIBaseObject::UIObjectList::const_iterator itChild = childObjects.begin(); itChild != childObjects.end(); ++itChild)
	{
		UIBaseObject * pageObject = *itChild;
		if (pageObject && pageObject->IsA(TUIPage))
		{
			UIPage & page = *safe_cast<UIPage *>(pageObject);

			char const * const name = pageObject->GetName().c_str();
			if (!_stricmp(name, c_radarSphericalPage))
			{
				SwgCuiSpaceRadarSpherical * radarSpherical = new SwgCuiSpaceRadarSpherical(page);
				radarSpherical->fetch();

				m_radarMediators->push_back(radarSpherical);
			}
			else if (!_stricmp(name, c_radarSpherical2Page))
			{
				SwgCuiSpaceRadarSpherical2 * radarSpherical = new SwgCuiSpaceRadarSpherical2(page);
				radarSpherical->fetch();
				m_radarMediators->push_back(radarSpherical);
			}
			else if (!_stricmp(name, c_radarPlanarPage))
			{
				SwgCuiSpaceRadarPlanar * radarPlanar = new SwgCuiSpaceRadarPlanar(page);
				radarPlanar->fetch();
				m_radarMediators->push_back(radarPlanar);
			}

			page.SetVisible(false);
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiSpaceRadar::releaseRadarPages()
{
	for (CuiMediatorVector::iterator itRadar = m_radarMediators->begin(); itRadar != m_radarMediators->end(); ++itRadar)
	{
		CuiMediator * page = *itRadar;
		if (page)
		{
			page->release();
		}
	}
	
	delete m_radarMediators;
}

//-----------------------------------------------------------------

void SwgCuiSpaceRadar::setRadarActiveIndex(int const index)
{
	if (index != m_radarActiveIndex && !m_radarMediators->empty())
	{
		m_radarActiveIndex = clamp(0, index, static_cast<int>(m_radarMediators->size()) - 1);
		
		for (CuiMediatorVector::iterator itRadar = m_radarMediators->begin(); itRadar != m_radarMediators->end(); ++itRadar)
		{
			CuiMediator * page = *itRadar;
			if (page)
			{
				page->deactivate();
			}
		}
		
		CuiMediator * activePage = getRadarActiveMediator();
		if (activePage)
		{
			activePage->activate();
		}
	}
}

//-----------------------------------------------------------------

int SwgCuiSpaceRadar::getRadarActiveIndex() const
{
	return m_radarActiveIndex;
}

//-----------------------------------------------------------------

CuiMediator * SwgCuiSpaceRadar::getRadarActiveMediator()
{
	if (!m_radarMediators->empty())
	{
		int const index = clamp(0, m_radarActiveIndex, static_cast<int>(m_radarMediators->size()) -1);
		return (*m_radarMediators)[static_cast<size_t>(index)];
	}

	return NULL;
}

//-----------------------------------------------------------------

void SwgCuiSpaceRadar::performCallback()
{
	setRadarActiveIndex(CuiPreferences::getRadarSelect());

	getPage().Pack();

	CuiMediator * activeMediator = getRadarActiveMediator();
	if (activeMediator)
	{
		activeMediator->getPage().Pack();
	}
}

//-----------------------------------------------------------------

void SwgCuiSpaceRadar::initializeStations()
{
	// Get the station page.
	UIPage * stationPage = NULL;
	getCodeDataObject(TUIPage, stationPage, "stations");
	stationPage->SetVisible(true);

	// Set the child pages to zero.
	UIBaseObject::UIObjectList children;
	stationPage->GetChildren(children);
	for (UIBaseObject::UIObjectList::iterator itChild = children.begin(); itChild != children.end(); ++itChild)
	{
		UIBaseObject * const childObject = *itChild;
		if (childObject && childObject->IsA(TUIWidget))
		{
			static_cast<UIWidget *>(childObject)->SetVisible(false);
		}
	}

	// Get the stations.
	UIImage * stationImage = NULL;
	getCodeDataObject(TUIImage, stationImage, "pilotStation");
	(*m_stationImageMap)[ShipStation::ShipStation_Pilot] = stationImage;

	// Get the operations station.
	getCodeDataObject(TUIImage, stationImage, "operationsStation");
	(*m_stationImageMap)[ShipStation::ShipStation_Operations] = stationImage;

	// Get the autopilot hack.
	getCodeDataObject(TUIImage, stationImage, "autoStation");
	(*m_stationImageMap)[ShipStation::ShipStation_None] = stationImage;


	// Get the gunners!
	getCodeDataObject(TUIImage, stationImage, "gunnerStation");
	for (int gunner = ShipStation::ShipStation_Gunner_First; gunner <= ShipStation::ShipStation_Gunner_Last; ++gunner)
	{
		(*m_stationImageMap)[static_cast<ShipStation::Type>(gunner)] = stationImage;
	}
}

//-----------------------------------------------------------------

void SwgCuiSpaceRadar::setStationIcon(ShipStation::Type const station)
{
	// Reset the old image.
	if (m_stationImageCurrent)
	{
		m_stationImageCurrent->SetVisible(false);
		m_stationImageCurrent = NULL;
	}

	// Get the current image.
	StationToImageMap::iterator itImage = m_stationImageMap->find(station);
	if (itImage != m_stationImageMap->end())
	{
		m_stationImageCurrent = itImage->second;

		if (m_stationImageCurrent)
		{
			m_stationImageCurrent->SetVisible(true);
		}
	}
}


//======================================================================
