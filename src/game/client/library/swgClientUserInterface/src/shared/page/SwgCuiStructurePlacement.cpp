//===================================================================
//
// SwgCuiStructurePlacement.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiStructurePlacement.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/GroundScene.h"
#include "clientGame/StructurePlacementCamera.h"
#include "clientGraphics/RenderWorld.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedGame/SharedBuildingObjectTemplate.h"
#include "sharedGame/SharedInstallationObjectTemplate.h"
#include "sharedGame/SharedTangibleObjectTemplate.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/LotManager.h" 
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/PortalPropertyTemplate.h"
#include "sharedObject/PortalPropertyTemplateList.h"
#include "sharedObject/StructureFootprint.h"
#include "sharedTerrain/TerrainObject.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include <cstdio>

//===================================================================

namespace SwgCuiStructurePlacementNamespace
{
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void clientCreate (const ObjectTemplate* const objectTemplate, const Vector& createLocation, const RotationType rotationType)
	{
		Object* const object = objectTemplate->createObject ();
		if (object)
		{
			switch (rotationType)
			{
			default:      
			case RT_0:    break;
			case RT_90:   object->yaw_o (PI_OVER_2);   break;
			case RT_180:  object->yaw_o (PI);          break;
			case RT_270:  object->yaw_o (-PI_OVER_2);  break;
			}

			object->setPosition_p (createLocation);

			ClientWorld::snapObjectToTerrain (object);
			RenderWorld::addObjectNotifications (*object);

			PortalProperty* const property = object->getPortalProperty ();
			if (property)
				property->clientSinglePlayerInitializeFirstTimeObject ();

			safe_cast<ClientObject*> (object)->endBaselines ();
			object->addToWorld();
		}	
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void serverCreate (const NetworkId& deedNetworkId, const Vector& position, const RotationType rotationType)
	{
		static const uint32 hash_placeStructure = Crc::normalizeAndCalculate ("placeStructure");

		char buffer [1024];
		sprintf (buffer, "%s %1.2f %1.2f %i", deedNetworkId.getValueString ().c_str (), position.x, position.z, static_cast<int> (rotationType));
		ClientCommandQueue::enqueueCommand (hash_placeStructure, NetworkId::cms_invalid, Unicode::narrowToWide (buffer));

#ifdef _DEBUG
		int const numberOfAdditionalStructureCreateMessages = ConfigFile::getKeyInt ("ClientGame", "additionalStructureCreateMessages", 0);
		for (int i = 0; i < numberOfAdditionalStructureCreateMessages; ++i)
			ClientCommandQueue::enqueueCommand (hash_placeStructure, NetworkId::cms_invalid, Unicode::narrowToWide (buffer));
#endif
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace SwgCuiStructurePlacementNamespace;

//===================================================================

SwgCuiStructurePlacement::SwgCuiStructurePlacement (UIPage & page) :
	CuiMediator ("SwgCuiStructurePlacement", page),
	UIEventCallback (),
	m_structureSharedObjectTemplateName (0),
	m_deedNetworkId (),
	m_oldView (0),
	m_structureObjectTemplate (0),
	m_structureFootprint (0),
	m_structureObject (0),
	m_rotationType (RT_0)
{
	getPage ().SetSelectable (true);

	UIButton * button = 0;
	getCodeDataObject (TUIButton, button, "cancelbutton");
	button->SetSelectable (true);
}

//-------------------------------------------------------------------
	
SwgCuiStructurePlacement::~SwgCuiStructurePlacement ()
{
	DEBUG_FATAL (m_structureSharedObjectTemplateName, ("SwgCuiStructurePlacement::~SwgCuiStructurePlacement(): m_structureSharedObjectTemplateName should be 0"));
	DEBUG_FATAL (m_structureObjectTemplate, ("SwgCuiStructurePlacement::~SwgCuiStructurePlacement(): m_structureObjectTemplate should be 0"));
	DEBUG_FATAL (m_structureObject, ("SwgCuiStructurePlacement::~SwgCuiStructurePlacement(): m_structureObject should be 0"));
}

//-------------------------------------------------------------------

//-- return value means keep processing
bool SwgCuiStructurePlacement::OnMessage (UIWidget* context, const UIMessage& msg)
{
	if (context != &getPage ())
		return true;

	GroundScene* const groundScene = safe_cast<GroundScene*> (Game::getScene ());
	if (groundScene)
	{
		StructurePlacementCamera* const camera = safe_cast<StructurePlacementCamera*> (groundScene->getCamera (GroundScene::CI_structurePlacement));
		camera->setMouseCoordinates (msg.MouseCoords.x, msg.MouseCoords.y);

		if (msg.Type == UIMessage::KeyDown || msg.Type == UIMessage::RightMouseDown)
		{
			bool updateStructure = false;

			if (msg.Keystroke == UIMessage::PageUp)
			{
				//-- modify rotation
				if (m_rotationType == RT_0)
					m_rotationType = RT_270;
				else
					m_rotationType = static_cast<RotationType> (static_cast<int> (m_rotationType) - 1);

				updateStructure = true;
			}

			if (msg.Keystroke == UIMessage::PageDown || msg.Type == UIMessage::RightMouseDown)
			{
				//-- modify rotation
				if (m_rotationType == RT_270)
					m_rotationType = RT_0;
				else
					m_rotationType = static_cast<RotationType> (static_cast<int> (m_rotationType) + 1);

				updateStructure = true;
			}

			if (updateStructure)
			{
				//-- tell camera new structure rotation
				camera->setRotation (m_rotationType);
			}
		}

		if (msg.Type == UIMessage::KeyDown && msg.Keystroke == UIMessage::Escape)
		{
			deactivate ();
			return false;
		}

		if ((msg.Type == UIMessage::LeftMouseDown))
		{
			const TerrainObject* const terrainObject = TerrainObject::getConstInstance ();
			NOT_NULL (terrainObject);

			const Vector createLocation = camera->getCreateLocation ();
			const int x = terrainObject->calculateChunkX (createLocation);
			const int z = terrainObject->calculateChunkZ (createLocation);

			const LotManager* const lotManager = ClientWorld::getLotManager ();
			float height = createLocation.y;
			if (m_structureFootprint && lotManager->canPlace (m_structureFootprint, x, z, m_rotationType, height))
			{
				if (Game::getSinglePlayer ())
					clientCreate (m_structureObjectTemplate, createLocation, m_rotationType);
				else
					serverCreate (m_deedNetworkId, createLocation, m_rotationType);

				deactivate ();

				return false;
			}
		}
	}

	return true;
}

//-------------------------------------------------------------------

void SwgCuiStructurePlacement::performActivate ()
{
	//-- setup callbacks
	getPage ().AddCallback (this);

	//-- acquire desired input
	setInputToggleActive (false);
	setPointerInputActive (true);

	//-- desctivate the hud
	CuiMediatorFactory::deactivate (CuiMediatorTypes::HudGround);

	//-- set the view
	GroundScene* const groundScene = safe_cast<GroundScene*> (Game::getScene ());
	if (groundScene)
	{
		m_oldView = groundScene->getCurrentView ();
		groundScene->setView (GroundScene::CI_structurePlacement);
	}

	m_rotationType = RT_0;
}

//-------------------------------------------------------------------

void SwgCuiStructurePlacement::performDeactivate ()
{
	//-- remove callbacks
	getPage ().RemoveCallback (this);

	//-- acquire desired input
	setPointerInputActive (false);
	setInputToggleActive (true);

	//-- reactivate the hud
	CuiMediatorFactory::activate (CuiMediatorTypes::HudGround);

	//-- reset the view
	GroundScene* const groundScene = safe_cast<GroundScene*> (Game::getScene ());
	if (groundScene)
		groundScene->setView (m_oldView);

	cleanup ();
}

//-------------------------------------------------------------------

bool SwgCuiStructurePlacement::setData (const NetworkId& deedNetworkId, const char* const structureSharedObjectTemplateName)
{
	cleanup ();

	m_deedNetworkId = deedNetworkId;

	//-- see if the object template name is valid
	if (!structureSharedObjectTemplateName || !*structureSharedObjectTemplateName)
	{
		DEBUG_WARNING (true, ("SwgCuiStructurePlacement::setData: PB structureSharedObjectTemplateName is null or empty"));
		return false;
	}

	//-- get the object template
	m_structureObjectTemplate = safe_cast<const SharedObjectTemplate*> (ObjectTemplateList::fetch (structureSharedObjectTemplateName));
	if (!m_structureObjectTemplate)
	{
		DEBUG_WARNING (true, ("SwgCuiStructurePlacement::setData: DB structureSharedObjectTemplateName [%s] could not be opened", structureSharedObjectTemplateName));
		return false;
	}

	//-- see if it is a tangible object template
	const SharedTangibleObjectTemplate* const tangibleObjectTemplate = dynamic_cast<const SharedTangibleObjectTemplate*> (m_structureObjectTemplate);
	if (!tangibleObjectTemplate)
	{
		DEBUG_WARNING (true, ("SwgCuiStructurePlacement::setData: DB structureSharedObjectTemplateName [%s] is not a tangible object template", structureSharedObjectTemplateName));
		cleanup ();

		return false;
	}

	//-- copy the new name
	m_structureSharedObjectTemplateName = DuplicateString (structureSharedObjectTemplateName);

	//-- grab the footprint
	m_structureFootprint = tangibleObjectTemplate->getStructureFootprint ();
	NOT_NULL (m_structureFootprint);

	//-- find the appearance template name
	TemporaryCrcString appearanceTemplateName;
	if (tangibleObjectTemplate->getId () == SharedBuildingObjectTemplate::SharedBuildingObjectTemplate_tag)
	{
		const std::string& pobName = tangibleObjectTemplate->getPortalLayoutFilename ();
		if (pobName.empty ())
		{
			DEBUG_WARNING (true, ("SwgCuiStructurePlacement::setData: DB structureSharedObjectTemplate [%s] has missing pob name", structureSharedObjectTemplateName));
			cleanup ();

			return false;
		}

		//-- open the pob
		const CrcLowerString pobCrcName (pobName.c_str ());
		if (!TreeFile::exists (pobCrcName.getString ()))
		{
			DEBUG_WARNING (true, ("SwgCuiStructurePlacement::setData: DB structureSharedObjectTemplate [%s] has missing pob [%s]", structureSharedObjectTemplateName, pobCrcName.getString ()));
			cleanup ();

			return false;
		}

		const PortalPropertyTemplate* const portalPropertyTemplate = PortalPropertyTemplateList::fetch (pobCrcName);
		NOT_NULL (portalPropertyTemplate);
		
		//--
		appearanceTemplateName.set (portalPropertyTemplate->getExteriorAppearanceName (), true);

		portalPropertyTemplate->release ();
	}
	else
		if (tangibleObjectTemplate->getId () == SharedInstallationObjectTemplate::SharedInstallationObjectTemplate_tag)
		{
			appearanceTemplateName.set (tangibleObjectTemplate->getAppearanceFilename ().c_str (), true);
		}
		else
		{
			DEBUG_WARNING (true, ("SwgCuiStructurePlacement::setData: DB structureSharedObjectTemplateName [%s] is not an installation or building object template", structureSharedObjectTemplateName));
			cleanup ();

			return false;
		}

	//-- create an object
	m_structureObject = new Object ();
	m_structureObject->setAppearance (AppearanceTemplateList::createAppearance (appearanceTemplateName.getString ()));
	m_structureObject->addNotification (ClientWorld::getIntangibleNotification ());
	RenderWorld::addObjectNotifications (*m_structureObject);

	//-- tell the view about the structure footprint
	GroundScene* const groundScene = safe_cast<GroundScene*> (Game::getScene ());
	if (groundScene)
	{
		StructurePlacementCamera* const camera = safe_cast<StructurePlacementCamera*> (groundScene->getCamera (GroundScene::CI_structurePlacement));
		
		camera->setStructureFootprint (m_structureFootprint);
		camera->setStructureObject (m_structureObject);
	}

	return true;
}

//===================================================================
// PRIVATE SwgCuiStructurePlacement
//===================================================================

void SwgCuiStructurePlacement::cleanup ()
{
	if (m_structureSharedObjectTemplateName)
	{
		delete [] m_structureSharedObjectTemplateName;
		m_structureSharedObjectTemplateName = 0;
	}

	if (m_structureObjectTemplate)
	{
		m_structureObjectTemplate->releaseReference ();
		m_structureObjectTemplate = 0;
	}

	m_structureFootprint = 0;

	if (m_structureObject)
	{
		delete m_structureObject;
		m_structureObject = 0;
	}
}

//===================================================================

