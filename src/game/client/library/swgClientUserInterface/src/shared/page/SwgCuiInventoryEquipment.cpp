// ======================================================================
//
// SwgCuiInventoryEquipment.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiInventoryEquipment.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIText.h"
#include "clientGame/ClientObject.h"
#include "clientGame/CreatureController.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/ClientWorld.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedObject/Container.h"
#include "sharedObject/World.h"
#include "sharedMessageDispatch/Transceiver.h"

// ======================================================================


SwgCuiInventoryEquipment::SwgCuiInventoryEquipment (UIPage & page) :
CuiMediator            ("SwgCuiInventoryEquipment", page),
UIEventCallback        (),
m_characterViewer      (0),
m_zoomInButton         (0),
m_zoomOutButton        (0),
m_leftButton           (0),
m_rightButton          (0),
m_centerButton         (0),
m_label                (0),
m_callback             (new MessageDispatch::Callback)
{
	UIWidget *widget = 0;
	getCodeDataObject (TUIWidget, widget, "CharacterViewer");
	m_characterViewer = NON_NULL (dynamic_cast<CuiWidget3dObjectListViewer *>(widget));

	getCodeDataObject (TUIButton, m_zoomInButton,  "zoomInButton");
	getCodeDataObject (TUIButton, m_zoomOutButton, "zoomOutButton");
	getCodeDataObject (TUIButton, m_leftButton,    "leftButton");
	getCodeDataObject (TUIButton, m_rightButton,   "rightButton");
	getCodeDataObject (TUIButton, m_centerButton,  "centerButton");

	getCodeDataObject (TUIText,   m_label,         "label"); 

	registerMediatorObject (*m_characterViewer, true);
	registerMediatorObject (*m_zoomInButton, true);
	registerMediatorObject (*m_zoomOutButton, true);
	registerMediatorObject (*m_leftButton, true);
	registerMediatorObject (*m_rightButton, true);
	registerMediatorObject (*m_centerButton, true);

}

//-----------------------------------------------------------------

SwgCuiInventoryEquipment::~SwgCuiInventoryEquipment ()
{
	deactivate ();
	setupCharacterViewer (0);
	m_characterViewer = 0;

	m_zoomInButton  = 0;
	m_zoomOutButton = 0;
	m_leftButton    = 0;
	m_rightButton   = 0;
	m_centerButton  = 0;

	m_label = 0;

	delete m_callback;
	m_callback = 0;
}

//-----------------------------------------------------------------

void SwgCuiInventoryEquipment::performActivate ()
{
	m_characterViewer->setPaused (false);
}

//-----------------------------------------------------------------

void SwgCuiInventoryEquipment::performDeactivate ()
{
	m_characterViewer->setPaused (true);
	setupCharacterViewer (0);
}

//-----------------------------------------------------------------

void SwgCuiInventoryEquipment::setupCharacterViewer (ClientObject * object)
{
	m_characterViewer->setCameraLodBias (2.0f);
	m_characterViewer->setAutoZoomOutOnly       (false);
	m_characterViewer->setCameraZoomInWhileTurn (true);
	m_characterViewer->setAlterObjects          (false);
	m_characterViewer->setCameraLookAtCenter    (false);
	m_characterViewer->setDragYawOk             (true);
	m_characterViewer->setPaused                (false);
	m_characterViewer->SetDragable              (false);	
	m_characterViewer->SetContextCapable        (true, false);
	m_characterViewer->setRotateSpeed           (0.0f);
	m_characterViewer->setCameraForceTarget     (false);
	m_characterViewer->setCameraTransformToObj  (true);
	m_characterViewer->setCameraLodBias         (3.0f);
	m_characterViewer->setCameraLodBiasOverride (true);
	m_characterViewer->setCameraForceTarget   (true);
	m_characterViewer->setObject              (object);
	m_characterViewer->recomputeZoom          ();
	m_characterViewer->setRotationSlowsToStop (true);

	m_characterViewer->setCameraForceTarget   (false);
	m_characterViewer->setCameraAutoZoom      (true);
	m_characterViewer->setCameraLookAtBone    ("root");
	m_characterViewer->setCameraZoomLookAtBone("head");
	

	m_label->SetLocalText (object ? object->getObjectName () : Unicode::String ());
}

//-----------------------------------------------------------------

void SwgCuiInventoryEquipment::OnButtonPressed( UIWidget *context )
{
	UNREF(context);
}

//----------------------------------------------------------------------

void SwgCuiInventoryEquipment::setInventoryType(SwgCuiInventory::InventoryType type)
{
	switch(type)
	{
	case SwgCuiInventory::IT_NORMAL:
		break;
	case SwgCuiInventory::IT_LOOT:
		break;
	case SwgCuiInventory::IT_LIGHTSABER:
		break;
	case SwgCuiInventory::IT_PUBLIC:
		break;
	}
}

// ======================================================================
