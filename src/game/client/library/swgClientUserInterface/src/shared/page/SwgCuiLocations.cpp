// ======================================================================
//
// SwgCuiLocations.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiLocations.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIPage.h"
#include "UIText.h"
#include "UIVolumePage.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientObject.h"
#include "clientGame/DebugPortalCamera.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientUserInterface/CuiLocationManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedNetworkMessages/MessageQueueDataTransform.h"
#include "sharedObject/Controller.h"
#include "swgClientUserInterface/SwgCuiHud.h"
#include "swgClientUserInterface/SwgCuiHudFactory.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

// @todo the following should be removed when single-player scene switches are handled elsewhere
#include "sharedObject/Appearance.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedFile/TreeFile.h"
#include "clientGame/GroundScene.h"
#include "sharedObject/AppearanceTemplate.h"

#include <cstdio>

// ======================================================================

SwgCuiLocations::SwgCuiLocations (UIPage & page) :
CuiMediator           ("SwgCuiLocations", page),
UIEventCallback       (),
m_cancelButton        (0),
m_okButton            (0),
m_planetsVolumePage   (0),
m_locationsVolumePage (0),
m_sample              (0),
m_locationManager     (0),
m_loadedCameraPathName()
{
	getCodeDataObject (TUIButton,   m_cancelButton,     "CancelButton");
	getCodeDataObject (TUIButton,   m_okButton,         "OkButton");

	getCodeDataObject (TUIVolumePage,  m_planetsVolumePage,   "planetsVolumePage");
	getCodeDataObject (TUIVolumePage,  m_locationsVolumePage, "locationsVolumePage");

	getCodeDataObject (TUIText,  m_sample, "sample");

	m_sample->SetVisible (false);
}

//-----------------------------------------------------------------

SwgCuiLocations::~SwgCuiLocations ()
{
	deactivate ();

	m_cancelButton = 0;
	m_okButton     = 0;

	m_planetsVolumePage = 0;
	m_locationsVolumePage = 0;

	m_sample = 0;

	delete m_locationManager;
	m_locationManager = 0;
}

//-----------------------------------------------------------------

void SwgCuiLocations::performActivate ()
{
	setPointerInputActive  (true);
	setKeyboardInputActive (true);
	setInputToggleActive   (false);
	
	reset ();
	
	m_cancelButton->AddCallback (this);
	m_okButton->AddCallback (this);
	m_planetsVolumePage->AddCallback (this);
}

//-----------------------------------------------------------------

void SwgCuiLocations::performDeactivate ()
{
	setPointerInputActive  (false);
	setKeyboardInputActive (false);
	setInputToggleActive   (true);
	
	m_cancelButton->RemoveCallback (this);
	m_okButton->RemoveCallback (this);
	m_planetsVolumePage->RemoveCallback (this);

	delete m_locationManager;
	m_locationManager = 0;

	m_planetsVolumePage->Clear ();
	m_locationsVolumePage->Clear ();
}

//-----------------------------------------------------------------

void SwgCuiLocations::OnButtonPressed   (UIWidget *context)
{
	UNREF (context);

	NOT_NULL (context);

	if (context == m_okButton)
	{
		const int planetIndex   = m_planetsVolumePage->GetLastSelectedIndex ();
		const int locationIndex = m_locationsVolumePage->GetLastSelectedIndex ();

		if (planetIndex < 0 || planetIndex >= m_locationManager->getNumberOfPlanets () ||
			locationIndex < 0 || locationIndex >= m_locationManager->getNumberOfLocations (planetIndex))
		{
				//-- create the message box
			CuiMessageBox * const box = CuiMessageBox::createMessageBox ();
			if (box)
			{
				box->addButton (CuiMessageBox::GBT_Ok, true, true);
				box->setText (Unicode::narrowToWide ("You must select a valid location."));
				box->layout ();
			}
			return;
		}
		
		const Vector position = m_locationManager->getLocationPosition (planetIndex, locationIndex);
		
		ClientObject * player = Game::getClientPlayer ();
		
		if (player)
		{
			SwgCuiHud * const hud = SwgCuiHudFactory::findMediatorForCurrentHud();
			if (NULL != hud)
				hud->activate();

			char buf [1024];
			
			if (Game::getSinglePlayer ())
			{
				_snprintf (buf, 1024, "terrain/%s.trn", m_locationManager->getPlanetName (planetIndex).c_str ());
				const std::string locationCameraPathName = m_locationManager->getLocationCameraPathName (planetIndex, locationIndex);
				
				const TerrainObject * const terrain = TerrainObject::getConstInstance ();
				if (_stricmp (terrain->getAppearance ()->getAppearanceTemplate ()->getName (), buf))
				{
					if (TreeFile::exists (buf))
					{
						const std::string templateName = player->getTemplateName ();
						GroundScene * gs = dynamic_cast<GroundScene*> (Game::getScene ());
						if (gs != 0)
						{
							gs->close ();
							delete gs;
							gs = 0;
						}					

						Game::setScene(true, buf, templateName.c_str(), 0);
						player = Game::getClientPlayer ();
					}
					else
					{
						WARNING (true, ("The specified terrain file does not exist."));
						return;
					}
				}

				if (m_loadedCameraPathName != locationCameraPathName)
				{
					m_loadedCameraPathName = locationCameraPathName;

					GroundScene* const groundScene = dynamic_cast<GroundScene*> (Game::getScene ());
					if (!groundScene)
						return;

					DebugPortalCamera* const debugPortalCamera = dynamic_cast<DebugPortalCamera*> (groundScene->getCamera (GroundScene::CI_debugPortal));
					if (!debugPortalCamera)
						return;

					debugPortalCamera->loadPath (m_loadedCameraPathName.c_str ());
				}

				Transform transform_w (player->getTransform_o2w ());
				transform_w.setPosition_p (position);
				PlayerCreatureController *const controller = dynamic_cast<PlayerCreatureController*> (player->getController ());
				
				if (controller)
					controller->warpClient (transform_w);
			}
			else
			{
				std::string const &planetName = m_locationManager->getPlanetName(planetIndex);
				snprintf(buf, sizeof(buf)-1, "%s %1.2f %1.2f %1.2f", planetName.c_str(), position.x, position.y, position.z);
				buf[sizeof(buf)-1] = '\0';
				ClientCommandQueue::enqueueCommand("planetwarp", NetworkId::cms_invalid, Unicode::narrowToWide(buf));
			}
		}
		
		deactivate ();
	}
	else if (context == m_cancelButton)
	{
		CuiMediatorFactory::activate (CuiMediatorTypes::GameMenu);
		deactivate ();
	}
}

//----------------------------------------------------------------------

void SwgCuiLocations::OnVolumePageSelectionChanged (UIWidget * context)
{
	if (context != m_planetsVolumePage || !m_locationManager)
		return;

	const int index = m_planetsVolumePage->GetLastSelectedIndex ();

	m_locationsVolumePage->Clear ();

	if (index < 0)
		return;

	const int numLocations = m_locationManager->getNumberOfLocations (index);

	std::vector<UIWidget *> children;
	children.reserve (numLocations);

	for (int i = 0; i < numLocations; ++i)
	{
		UIText * const text = safe_cast<UIText *>(m_sample->DuplicateObject ());
		NOT_NULL (text);

		text->SetVisible (true);
		text->SetText (Unicode::narrowToWide (m_locationManager->getLocationName (index, i)));
		children.push_back (text);
	}

	// @todo: volume page should support AddChildren ()
	for (std::vector<UIWidget *>::const_iterator it = children.begin (); it != children.end (); ++it)
		m_locationsVolumePage->AddChild (*it);

	m_locationsVolumePage->Link ();
}

//----------------------------------------------------------------------

void SwgCuiLocations::reset ()
{
	m_planetsVolumePage->Clear ();
	m_locationsVolumePage->Clear ();

	if (m_locationManager)
	{
		delete m_locationManager;
		m_locationManager = 0;
	}

	m_locationManager = new CuiLocationManager ("locations.txt");

	const int numPlanets = m_locationManager->getNumberOfPlanets ();

	std::vector<UIWidget *> children;
	children.reserve (numPlanets);

	for (int i = 0; i < numPlanets; ++i)
	{
		UIText * const text = safe_cast<UIText *>(m_sample->DuplicateObject ());
		NOT_NULL (text);

		text->SetVisible (true);
		text->SetText (Unicode::narrowToWide (m_locationManager->getPlanetDisplayName (i)));
		children.push_back (text);
	}

	// @todo: volume page should support AddChildren ()
	for (std::vector<UIWidget *>::const_iterator it = children.begin (); it != children.end (); ++it)
		m_planetsVolumePage->AddChild (*it);

	m_planetsVolumePage->Link ();
}

// ======================================================================

