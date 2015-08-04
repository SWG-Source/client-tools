//======================================================================
//
// SwgCuiAvatarLocation2.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAvatarLocation2.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIDataSourceContainer.h"
#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIList.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIText.h"
#include "UIVolumePage.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/Game.h"
#include "clientGame/PlanetInfoManagerClient.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGame/TangibleObject.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiTurntableAdapter.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFoundation/Crc.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/RotationDynamics.h"
#include "sharedRandom/Random.h"
#include "sharedUtility/StartingLocationData.h"
#include "sharedUtility/StartingLocationManager.h"
#include "swgClientUserInterface/SwgCuiAvatarCreationHelper.h"
#include "swgClientUserInterface/SwgCuiHud.h"
#include "swgClientUserInterface/SwgCuiHudFactory.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include <algorithm>
#include <list>

//======================================================================

namespace
{
	namespace StringTables
	{
		const std::string LocationName = "loc_n";
		const std::string LocationDesc = "loc_d";
	};
}

//----------------------------------------------------------------------

SwgCuiAvatarLocation2::SwgCuiAvatarLocation2 (UIPage & page) :
CuiMediator                 ("SwgCuiAvatarLocation2", page),
UIEventCallback             (),
m_buttonBack                (0),
m_buttonNext                (0),
m_volumeCities              (0),
m_text                      (0),
m_textPlanet                (0),
m_viewer                    (0),
m_planets                   (new TangibleVector),
m_turntable                 (0),
m_locationStatusMap         (new LocationStatusMap),
m_locationStatusVector      (new LocationStatusVector),
m_selectedLocation          (),
m_selectedLocationAvailable (false),
m_callback                  (new MessageDispatch::Callback),
m_messageBox                (0),
m_isFinishing               (false)
{
	getCodeDataObject (TUIButton,     m_buttonBack,    "buttonBack");
	getCodeDataObject (TUIButton,     m_buttonNext,    "buttonNext");

	getCodeDataObject (TUIVolumePage, m_volumeCities,  "volumeCities");
	getCodeDataObject (TUIText,       m_text,          "text");
	getCodeDataObject (TUIText,       m_textPlanet,    "textPlanet");

	{
		UIWidget * widget = 0;
		getCodeDataObject (TUIWidget,  widget,        "viewer");
		
		m_viewer = NON_NULL (dynamic_cast<CuiWidget3dObjectListViewer *>(widget));
	}

	m_turntable = new CuiTurntableAdapter (*m_viewer);

	m_text->SetPreLocalized (true);
	m_textPlanet->SetPreLocalized (true);

	m_volumeCities->Clear ();

	m_turntable->setSpacingModifier (2.0f);
	m_turntable->setTurnToFace      (false);

	registerMediatorObject (*m_buttonBack,    true);
	registerMediatorObject (*m_buttonNext,    true);
	registerMediatorObject (*m_volumeCities,  true);
	registerMediatorObject (*m_viewer,        true);

	m_text->SetTextUnroll     (false);
	m_text->SetTextUnrollOnce (true);	
}

//----------------------------------------------------------------------

SwgCuiAvatarLocation2::~SwgCuiAvatarLocation2 ()
{
	delete m_locationStatusMap;
	m_locationStatusMap = 0;

	delete m_locationStatusVector;
	m_locationStatusVector = 0;

	m_buttonBack     = 0;
	m_buttonNext     = 0;
	m_volumeCities   = 0;
	m_text           = 0;
	m_textPlanet     = 0;
	m_viewer         = 0;

	delete m_planets;
	m_planets        = 0;

	delete m_turntable;
	m_turntable      = 0;	

	delete m_callback;
	m_callback = 0;
}

//----------------------------------------------------------------------

void SwgCuiAvatarLocation2::performActivate   ()
{
	m_isFinishing = false;
	m_selectedLocation.clear ();
	m_selectedLocationAvailable = false;
	setPointerInputActive  (true);
	setKeyboardInputActive (true);
	setInputToggleActive   (false);

	m_callback->connect (*this, &SwgCuiAvatarLocation2::onStartingLocationsReceived,       static_cast<PlayerCreatureController::Messages::StartingLocationsReceived*>(0));
	m_callback->connect (*this, &SwgCuiAvatarLocation2::onStartingLocationSelectionResult, static_cast<PlayerCreatureController::Messages::StartingLocationSelectionResult*>(0));
	
	if (Game::getSinglePlayer ())
	{
		typedef std::pair <StartingLocationData, bool>          LocationStatus;
		typedef stdvector<LocationStatus>::fwd                  LocationStatusVector;
		typedef StartingLocationManager::StartingLocationVector StartingLocationVector;
		const StartingLocationVector & slv = StartingLocationManager::getLocations ();
		
		LocationStatusVector lsv;
		lsv.reserve (slv.size ());
		
		for (StartingLocationVector::const_iterator it = slv.begin (); it != slv.end (); ++it)
		{
			const StartingLocationData & data = *it;
			lsv.push_back (LocationStatus (data, true));
		}
		
		setLocations (lsv);
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarLocation2::performDeactivate ()
{
	if (m_messageBox)
	{
		m_messageBox->closeMessageBox ();
		m_messageBox= 0;
	}

	m_callback->disconnect (*this, &SwgCuiAvatarLocation2::onStartingLocationsReceived,       static_cast<PlayerCreatureController::Messages::StartingLocationsReceived*>(0));
	m_callback->disconnect (*this, &SwgCuiAvatarLocation2::onStartingLocationSelectionResult, static_cast<PlayerCreatureController::Messages::StartingLocationSelectionResult*>(0));

	clearPlanets ();
}

//----------------------------------------------------------------------

void SwgCuiAvatarLocation2::clearPlanets ()
{
	{
		for (TangibleVector::iterator it = m_planets->begin (); it != m_planets->end (); ++it)
		{
			TangibleObject * const obj = *it;
			m_viewer->removeObject (*obj);
			delete obj;
		}
	}
	
	m_planets->clear ();
}

//----------------------------------------------------------------------

void SwgCuiAvatarLocation2::OnButtonPressed   (UIWidget *context)
{
	if (context == m_buttonBack)
	{
		if (Game::getScene ())
		{
			SwgCuiHud * const hud = SwgCuiHudFactory::findMediatorForCurrentHud();
			if (NULL != hud)
				hud->activate();

			deactivate ();
		}
		else
			CuiManager::exitGame (false);
	}
	else if (context == m_buttonNext)
	{
		ok ();
	}
}

//----------------------------------------------------------------------

bool SwgCuiAvatarLocation2::OnMessage (UIWidget *context, const UIMessage & msg)
{
	if (context == m_viewer)
	{
		if (msg.Type == UIMessage::LeftMouseDown)
		{
			const ClientObject * const obj = m_viewer->getObjectAt (msg.MouseCoords);
			if (obj)
			{
				const TangibleVector::iterator fit = std::find (m_planets->begin (), m_planets->end (), obj);
				if (fit != m_planets->end ())
				{
					const int index = std::distance (m_planets->begin (), fit);
					selectPlanet (index, true);
				}
				return false;
			}
		}
	}
	
	return true;
}

//----------------------------------------------------------------------

void SwgCuiAvatarLocation2::OnVolumePageSelectionChanged (UIWidget * context)
{
	if (context == m_volumeCities)
	{
		updateDescription ();
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarLocation2::setupPlanets         ()
{
	clearPlanets ();
	
	const bool asynchronousLoaderEnabled = AsynchronousLoader::isEnabled ();
	AsynchronousLoader::disable ();
		
	typedef std::set<std::string> StringSet;
	StringSet stringSet;

	for (LocationStatusVector::const_iterator it = m_locationStatusVector->begin (); it != m_locationStatusVector->end (); ++it)
	{
		const LocationStatus & status     = *it;
		const StartingLocationData & data = status.first;
		const std::string & planetName    = data.planet;
		
		//-- only put the planet in the scene once
		if (stringSet.find (planetName) != stringSet.end ())
			continue;

		TangibleObject * const planetObject = PlanetInfoManagerClient::createObjectForPlanet (planetName, true);
		if (planetObject)
			m_viewer->addObject (*planetObject);
		else
		{
			WARNING (true, ("setupPlanets Unable to load planet [%s]", planetName.c_str ()));
		}

		m_planets->push_back (planetObject);
		stringSet.insert (planetName);
	}
	
	if (asynchronousLoaderEnabled)
		AsynchronousLoader::enable ();

	m_turntable->arrange ();
	m_turntable->turnTo (-1, false);

	m_viewer->setCameraLookAt (Vector::zero);
	m_viewer->setPaused       (false);

	m_viewer->setViewDirty             (true);
	m_viewer->setCameraForceTarget     (true);
	m_viewer->recomputeZoom            ();
	m_viewer->setCameraForceTarget     (false);
}

//----------------------------------------------------------------------

void SwgCuiAvatarLocation2::selectPlanet (int index, bool playSound)
{
	DEBUG_FATAL (index < 0 || index >= static_cast<int>(m_planets->size ()), ("bad"));

	m_turntable->turnTo (index, playSound);

	const TangibleObject * const planet = (*m_planets) [static_cast<size_t>(index)];
	NOT_NULL (planet);

	m_volumeCities->Clear ();
	m_volumeCities->SetSelection (0);

	const std::string planetName (NON_NULL (planet->getDebugName ()));
	const LocationStatusMap::const_iterator lit = m_locationStatusMap->find (planetName);
	if (lit != m_locationStatusMap->end ())
	{
		const LocationStatusVector & lsv = (*lit).second;

		for (LocationStatusVector::const_iterator it = lsv.begin (); it != lsv.end (); ++it)
		{
			const LocationStatus & status     = (*it);
			const StartingLocationData & data = status.first;
			const bool available              = status.second;

			const StringId nameId (StringTables::LocationName, data.name);

			UIImage * const image = new UIImage;
			image->SetName           (data.name);
			image->SetProperty       (UIList::DataProperties::LOCALTEXT, nameId.localize ());
			image->SetPropertyNarrow (UIImage::PropertyName::Style,      data.image);

			//@todo: update ui to reflect unavailable location

			image->SetEnabled        (available);
			if (!available)
				image->SetOpacity (0.5f);
			m_volumeCities->AddChild (image);
			image->Link ();
		}
	}
	else
		WARNING (true, ("No city info for planet: %s", planetName.c_str ()));

	updateDescription ();
}

//----------------------------------------------------------------------

void SwgCuiAvatarLocation2::updateDescription ()
{		
	const TangibleObject * const planet = safe_cast<TangibleObject *>(m_turntable->getCurrentObject ());

	if (!planet)
		return;

	const UIWidget * const citySelection      = m_volumeCities->GetLastSelectedChild ();
	const int              citySelectionIndex = m_volumeCities->GetLastSelectedIndex ();

	m_textPlanet->SetLocalText (planet->getLocalizedName ());

	if (citySelection && citySelectionIndex >= 0)
	{
		const std::string & name = citySelection->GetName ();
		const StringId descId (StringTables::LocationDesc, name);

		m_selectedLocation = name;
		m_selectedLocationAvailable = citySelection->IsEnabled ();

		Unicode::String desc;

		if (!m_selectedLocationAvailable)
			desc = CuiStringIds::avatar_location_desc_unavailable.localize ();

		desc += descId.localize ();
		
		m_text->SetLocalText      (desc);
		m_buttonNext->SetEnabled  (true);
	}
	else
	{
		Unicode::String desc;
		planet->getLocalizedDescription (desc);
		m_text->SetLocalText     (desc);
		m_buttonNext->SetEnabled (false);
		m_selectedLocation.clear ();
		m_selectedLocationAvailable = false;
	}

	m_text->SetScrollLocation  (UIPoint::zero);
}

//----------------------------------------------------------------------

const SwgCuiAvatarLocation2::LocationStatus *   SwgCuiAvatarLocation2::findLocationStatus    (const std::string & planetName, int cityIndex)
{
	const LocationStatusMap::const_iterator lit = m_locationStatusMap->find (planetName);
	if (lit != m_locationStatusMap->end ())
	{
		const LocationStatusVector & lsv = (*lit).second;

		if (cityIndex >= 0 && cityIndex < static_cast<int>(lsv.size ()))
			return &lsv [static_cast<size_t>(cityIndex)];
	}

	return 0;
}

//----------------------------------------------------------------------

void SwgCuiAvatarLocation2::setLocations (const LocationStatusVector & lsv)
{
	*m_locationStatusVector = lsv;
	m_locationStatusMap->clear ();

	for (LocationStatusVector::const_iterator it = lsv.begin (); it != lsv.end (); ++it)
	{
		const LocationStatus & status     = *it;
		const StartingLocationData & data = status.first;

		(*m_locationStatusMap)[data.planet].push_back (status);
	}

	setupPlanets ();

	//-- select the first planet/city.  This is the starting location most prefered by the server
	selectPlanet      (0, false);
	m_volumeCities->SetSelectionIndex (0);
	updateDescription ();
}

//----------------------------------------------------------------------

void SwgCuiAvatarLocation2::onStartingLocationsReceived (const PlayerCreatureController::Messages::StartingLocationsReceived::Payload & lsv)
{
	setLocations (lsv);
}

//----------------------------------------------------------------------

void SwgCuiAvatarLocation2::onStartingLocationSelectionResult (const PlayerCreatureController::Messages::StartingLocationSelectionResult::Payload & payload)
{
	if (m_messageBox)
	{
		m_messageBox->closeMessageBox ();
		m_messageBox = 0;
	}
	
	if (payload.second)
	{
		m_isFinishing = true;

		m_messageBox = CuiMessageBox::createMessageBox (CuiStringIds::avatar_wait_transport_location.localize ());
		m_messageBox->setRunner (true);
		m_callback->connect (m_messageBox->getTransceiverClosed (), *this, &SwgCuiAvatarLocation2::onMessageBoxClosed);
		//@todo: transfer into some kind of cut scene?
	}
	else
	{
		CuiMessageBox::createInfoBox (CuiStringIds::avatar_select_location_failed.localize ());
		//@todo: update ui to reflect unavailable location
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarLocation2::ok ()
{
	if (m_selectedLocation.empty ())
		return;

	if (m_isFinishing)
	{
		m_messageBox = CuiMessageBox::createMessageBox (CuiStringIds::avatar_wait_transport_location.localize ());
		m_messageBox->setRunner (true);
		m_callback->connect (m_messageBox->getTransceiverClosed (), *this, &SwgCuiAvatarLocation2::onMessageBoxClosed);
		return;
	}

	if (!m_selectedLocationAvailable)
	{
		CuiMessageBox::createInfoBox (CuiStringIds::avatar_location_unavailable.localize ());
		return;
	}

	static const uint32 newbieSelectStartingLocation  = Crc::normalizeAndCalculate("newbieSelectStartingLocation");
	const Unicode::String & params = Unicode::narrowToWide (m_selectedLocation);
	ClientCommandQueue::enqueueCommand (newbieSelectStartingLocation, NetworkId::cms_invalid, params);

	m_messageBox = CuiMessageBox::createMessageBox (CuiStringIds::avatar_wait_select_location.localize ());
	m_messageBox->setRunner (true);
	m_callback->connect (m_messageBox->getTransceiverClosed (), *this, &SwgCuiAvatarLocation2::onMessageBoxClosed);
}

//----------------------------------------------------------------------

void SwgCuiAvatarLocation2::onMessageBoxClosed (const CuiMessageBox & box)
{
	if (&box == m_messageBox)
		m_messageBox = 0;
}

//======================================================================
