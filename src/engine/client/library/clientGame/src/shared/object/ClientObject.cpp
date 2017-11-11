/**
 *  ClientObject.cpp
 *
 *  Copyright 2001 Sony Online Entertainment
 */

/* Implementation file for the client side version of ClientObject */

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientObject.h"

#include "LocalizationManager.h"
#include "clientAudio/Audio.h"
#include "clientGame/CellObject.h"
#include "clientGame/ClientClearCollidableFloraNotification.h"
#include "clientGame/ClientController.h"
#include "clientGame/ClientDataFile.h"
#include "clientGame/ClientNoBuildNotification.h"
#include "clientGame/ClientObjectTemplate.h"
#include "clientGame/ClientObjectTerrainModificationNotification.h"
#include "clientGame/ClientSynchronizedUi.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/FadingTextAppearance.h"
#include "clientGame/Game.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/WorldSnapshot.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/TimerObject.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiObjectTextManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiTextManager.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/BaselinesMessage.h"
#include "sharedNetworkMessages/DeltasMessage.h"
#include "sharedObject/ArrangementDescriptor.h"
#include "sharedObject/ArrangementDescriptorList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/DebugInfoManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedFoundation/Production.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/Property.h"
#include "sharedObject/SimpleDynamics.h"
#include "sharedObject/SlotDescriptor.h"
#include "sharedObject/SlotDescriptorList.h"
#include "sharedObject/SlotId.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include "sharedObject/LayerProperty.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedObject/VolumeContainmentProperty.h"
#include "sharedRandom/RandomGenerator.h"
#include "swgSharedUtility/States.h"

#include <algorithm>

//----------------------------------------------------------------------

namespace ClientObjectNamespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const ClientObject::Messages::ContainerMsg &, ClientObject::Messages::AddedToContainer >
			s_addedToContainer;
		MessageDispatch::Transceiver<const ClientObject::Messages::ContainerMsg &, ClientObject::Messages::RemovedFromContainer >
			s_removedFromContainer;
		MessageDispatch::Transceiver<const ClientObject &, ClientObject::Messages::ComplexityChange >
			s_complexityChange;
		MessageDispatch::Transceiver<const ClientObject &, ClientObject::Messages::BankBalance >
			s_bankBalance;
		MessageDispatch::Transceiver<const ClientObject &, ClientObject::Messages::CashBalance>
			s_cashBalance;
		MessageDispatch::Transceiver<const ClientObject::Messages::NameChanged::Payload &, ClientObject::Messages::NameChanged>
			nameChanged;
	}

	typedef stdmap<uint32, ClientObject *>::fwd IdObjectMap;
	IdObjectMap ms_idObjectMap;

	bool ms_renderStartTransform;
	bool ms_renderLineToStartTransform;
	bool ms_verbose;
	bool ms_viewer;

	uint32 ms_nextUniqueId = 1;

	// client side fake network id occupies the highest non-sign bit location in the 8 byte signed NetworkId
	// 01000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	NetworkId::NetworkIdType const ms_firstFakeNetworkId = 0x4000000000000000ll;
	NetworkId::NetworkIdType ms_nextFakeNetworkId =        0x4000000000000000ll;

	const float s_playerFlyTextScale = 1.1f;     // additional scale factor to apply to player's fly text
	const float s_randomFlyTextDistance = 0.4f;   // amount to randomly offset fly text

	std::string const & ms_debugInfoSectionName = "ClientObject";

	StringId const s_invalidTargetStringId("combat_effects", "target_invalid_fly");
	Unicode::String s_invalidTargetString;
	float const INVALID_TARGET_ENFORCE_DELAY = 0.1f;

	// ----------------------------------------------------------------------------
	bool isAlpha(Unicode::unicode_char_t const character)
	{
		bool result = false;

		if (   (character <= 255)
			&& isalpha(character))
		{
			result = true;
		}

		return result;
	}
}

using namespace ClientObjectNamespace;

//-----------------------------------------------------------------------

void ClientObject::install (bool viewer)
{
	InstallTimer const installTimer("ClientObject::install");

#ifdef _DEBUG
	DebugFlags::registerFlag (ms_renderStartTransform, "ClientGame/ClientObject", "renderStartTransform");
	DebugFlags::registerFlag (ms_renderLineToStartTransform, "ClientGame/ClientObject", "renderLineToStartTransform");
	DebugFlags::registerFlag (ms_verbose, "ClientGame/ClientObject", "verbose");
#endif

	ms_viewer = viewer;

	ExitChain::add (remove, "ClientObject::remove");
}

//-----------------------------------------------------------------------

void ClientObject::remove ()
{
#ifdef _DEBUG
	DebugFlags::unregisterFlag (ms_renderStartTransform);
	DebugFlags::unregisterFlag (ms_renderLineToStartTransform);
	DebugFlags::unregisterFlag (ms_verbose);
#endif
}

//-----------------------------------------------------------------------

ClientObject::ClientObject(const SharedObjectTemplate* newTemplate, const ObjectNotification &notification) :
	Object(newTemplate, NetworkId::cms_invalid),
	m_arrangementDescriptor(0),
	m_authoritativeClientServerPackage(),
	m_authoritativeClientServerPackage_np(),
	m_firstParentAuthClientServerPackage(),
	m_firstParentAuthClientServerPackage_np(),
	m_sharedPackage(),
	m_sharedPackage_np(),
	m_synchronizedUi(0),
	m_transferInProgress(false),
#ifdef _DEBUG
	m_startTransform_w(),
#endif
	m_containedByChange(0),
	m_arrangementChange(0),
	m_beginBaselines(false),
	m_initialized(false),
	m_localizedName(),
	m_localizedEnglishName(),
	m_localizedNameDirty(true),
	m_lastFlyTextWasInvalidTarget(false),
	m_volume(0),
	m_objectName(),
	m_nameStringId(),
	m_complexity(0.0f),
	m_authServerProcessId(0),
	m_cashBalance(0),
	m_bankBalance(0),
	m_clientCached(false),
	m_uniqueId(ms_nextUniqueId++),
	m_auxilliaryObjectVector(NULL),
	m_lastFlyTextTime(0.f)

#ifdef _DEBUG
	, m_warnedNotInitialized (false)
#endif
{
	IGNORE_RETURN(ms_idObjectMap.insert (std::make_pair (m_uniqueId, this)));

	if (newTemplate != NULL)
		m_nameStringId = newTemplate->getObjectName();

	//-- add properties that allow the object to be added to containers and slots.
	ContainedByProperty* containedBy = new ContainedByProperty(*this, NULL);
	addProperty(*containedBy);

	//-- get ArrangementDescriptor if specified
	SlottedContainmentProperty* slottedProperty = new SlottedContainmentProperty(*this);
	addProperty(*slottedProperty);

	if (newTemplate != NULL)
	{
		const std::string &arrangementDescriptorName = newTemplate->getArrangementDescriptorFilename();
		if (!arrangementDescriptorName.empty())
		{
			//-- fetch valid arrangements
			//@todo Todd, do we really need m_arrangementDescriptor.  Unless I'm wrong, it is only used here.
			m_arrangementDescriptor = ArrangementDescriptorList::fetch(arrangementDescriptorName);
			//-- create the SlottedContainment property
			DEBUG_FATAL(!m_arrangementDescriptor, ("Arrangement Descriptor NULL for %s", newTemplate->DataResource::getName()));
			const int arrangementCount = m_arrangementDescriptor->getArrangementCount();
			for (int i = 0; i < arrangementCount; ++i)
			{
				slottedProperty->addArrangement(m_arrangementDescriptor->getArrangement(i));
			}
		}
	}

	m_descriptionStringId.set(safe_cast<const SharedObjectTemplate *>(getObjectTemplate())->getDetailedDescription ());
	addSharedVariable    (m_complexity);
	addSharedVariable    (m_nameStringId);
	addSharedVariable    (m_objectName);
	addSharedVariable    (m_volume);

	addClientServerVariable    (m_bankBalance);
	addClientServerVariable    (m_cashBalance);

	addSharedVariable_np (m_authServerProcessId);
	addSharedVariable_np (m_descriptionStringId);


	containedBy = 0; //lint !e423 // creation of memory leak // no, managed by property list
	slottedProperty = 0;  //lint !e423 // creation of memory leak // no, managed by property list

	// In single-player mode, assign all ClientObjects a fake NetworkId.
	// All NetworkId-based systems (such as containers) are totally broken
	// in single player without this.  Pretend to be in single player whenever
	// there is no scene available

	if (Game::getSinglePlayer() || !Game::getScene ())
	{
		setNetworkId(getNextFakeNetworkId());
		m_volume = 1;
	}

	//If the object template specifies a volume, give it a volumeContainment
	//@todo get the volume from the template. Right now just give everything a unit volume of 1.
	addProperty(*(new VolumeContainmentProperty(*this, (m_volume.get() < 1 ? 1 : m_volume.get()) )));

	//set up containers on this object if it has any
	if (newTemplate != NULL)
	{
		SharedObjectTemplate::ContainerType const containerType = newTemplate->getContainerType();
		switch (containerType)
		{
		case SharedObjectTemplate::CT_none:
			break;

		case SharedObjectTemplate::CT_slotted:
		case SharedObjectTemplate::CT_ridable:
		{
			const SlotDescriptor *const slotDescriptor = newTemplate->getSlotDescriptor();
			if (slotDescriptor)
			{
				//-- create a SlottedContainer with the given slots
				addProperty(*(new SlottedContainer(*this, slotDescriptor->getSlots())));
			}
			else
			{
				DEBUG_WARNING(true, ("DATA ERROR: specified slotted container type but could not get slot descriptor for %s", newTemplate->getName()));
			}
			break;
		}
		case SharedObjectTemplate::CT_volume:
		case SharedObjectTemplate::CT_volumeIntangible:
		case SharedObjectTemplate::CT_volumeGeneric:
		{
			addProperty(*(new VolumeContainer(*this, newTemplate->getContainerVolumeLimit ()))); //@todo load max volume from template.  100 for now.
			break;
		}

		//	case SharedObjectTemplate::CT_spatial:
		default:
			DEBUG_WARNING(true, ("Invalid container type specified."));
			break;
		} //lint !e788 // ContainerType::ContainerType_Last did not appear in default switch
	}

	// add the portal property as specified by the object template
	if (newTemplate != NULL)
	{
		const std::string &portalLayoutFileName = newTemplate->getPortalLayoutFilename();
		if (!portalLayoutFileName.empty())
		{
			PortalProperty *portalProperty = new PortalProperty(*this, portalLayoutFileName.c_str());
			addProperty(*portalProperty);
			portalProperty->createAppearance();
		} //lint !e429 // Custodial pointer has not been freed or returned // addProperty() absorbed the pointer
	}

	m_objectName.setSourceObject   (this);
	m_nameStringId.setSourceObject (this);
	m_complexity.setSourceObject   (this);
	m_bankBalance.setSourceObject  (this);
	m_cashBalance.setSourceObject  (this);

	if (!ms_viewer)
		addNotification(notification);

	//-- add the no build notification
	{
		if (newTemplate && newTemplate->getNoBuildRadius () > 0.f)
			addNotification (ClientNoBuildNotification::getInstance ());
	}

	//-- add the clear flora notification
	{
		if (newTemplate && newTemplate->getClearFloraRadius () > 0.f)
			addNotification (ClientClearCollidableFloraNotification::getInstance ());
	}
}

//-----------------------------------------------------------------------

ClientObject::~ClientObject()
{
	//-- This must be the first line in the destructor to invalidate any watchers watching this object
	nullWatchers();

	if (m_auxilliaryObjectVector)
	{
		std::for_each(m_auxilliaryObjectVector->begin(), m_auxilliaryObjectVector->end(), PointerDeleter());
		delete m_auxilliaryObjectVector;
	}

	IGNORE_RETURN(ms_idObjectMap.erase (m_uniqueId));

	if (m_arrangementDescriptor)
	{
		m_arrangementDescriptor->release();
		m_arrangementDescriptor = 0;
	}

	if (m_containedByChange)
	{
		handleContainerChange();
		delete m_containedByChange;
		m_containedByChange = 0;
	}

	delete m_arrangementChange;
	m_arrangementChange = 0;
	delete m_synchronizedUi;
	m_synchronizedUi = 0;

	if (isInWorld ())
		ClientObject::removeFromWorld ();

}

//-----------------------------------------------------------------------

const NetworkId ClientObject::getNextFakeNetworkId()
{
	return NetworkId(ms_nextFakeNetworkId++);
}

//-----------------------------------------------------------------------

const boolean ClientObject::isFakeNetworkId(const NetworkId& id)
{
	return (id.getValue() >= ms_firstFakeNetworkId);
}

//-----------------------------------------------------------------------

ClientObject * ClientObject::asClientObject()
{
	return this;
}

//-----------------------------------------------------------------------

ClientObject const * ClientObject::asClientObject() const
{
	return this;
}

//-----------------------------------------------------------------------

CreatureObject * ClientObject::asCreatureObject()
{
	return 0;
}

//-----------------------------------------------------------------------

CreatureObject const * ClientObject::asCreatureObject() const
{
	return 0;
}

//-----------------------------------------------------------------------

StaticObject * ClientObject::asStaticObject()
{
	return 0;
}

//-----------------------------------------------------------------------

StaticObject const * ClientObject::asStaticObject() const
{
	return 0;
}

//-----------------------------------------------------------------------

TangibleObject * ClientObject::asTangibleObject()
{
	return 0;
}

//-----------------------------------------------------------------------

TangibleObject const * ClientObject::asTangibleObject() const
{
	return 0;
}

//-----------------------------------------------------------------------

ShipObject * ClientObject::asShipObject()
{
	return 0;
}

//-----------------------------------------------------------------------

ShipObject const * ClientObject::asShipObject() const
{
	return 0;
}

//-----------------------------------------------------------------------

WeaponObject * ClientObject::asWeaponObject()
{
	return 0;
}

//-----------------------------------------------------------------------

WeaponObject const * ClientObject::asWeaponObject() const
{
	return 0;
}

//-----------------------------------------------------------------------

CellObject * ClientObject::asCellObject()
{
	return 0;
}

//-----------------------------------------------------------------------

CellObject const * ClientObject::asCellObject() const
{
	return 0;
}

//-----------------------------------------------------------------------
static void bp_trap_clientobjectAlter()
{
	return;
}

float ClientObject::alter(float time)
{
#ifdef _DEBUG
	if (!m_warnedNotInitialized && !isInitialized ())
	{
		DEBUG_WARNING (ms_verbose, ("Calling alter before object has been initialized.  Was endBaselines called on this object?"));
		m_warnedNotInitialized = true;
	}
#endif

// JU_TODO: debug
#if 1
	if(Game::getPlayer() == this)
	{
		//DEBUG_REPORT_LOG(true,("ClientObject::alter - bp trap for the player\n"));
		bp_trap_clientobjectAlter();
		
	}
#endif
// JU_TODO: end debug
	float alterResult = Object::alter(time);

	if (alterResult == AlterResult::cms_kill) //lint !e777 // testing floats for equality // both set and test use a constant
		return AlterResult::cms_kill;

	handleContainerChange();

#ifdef _DEBUG
	//-- render start transform
	// @todo something is going to need to trigger an AlterScheduler::submitForAlter() when
	//       ms_renderStartTransform is true.
	if (ms_renderStartTransform && Game::getCamera () && isInWorld ())
	{
		Game::getCamera ()->addDebugPrimitive (new FrameDebugPrimitive (FrameDebugPrimitive::S_none, m_startTransform_w, 1.f));

		if (ms_renderLineToStartTransform)
		{
			const Vector start_w = getPosition_w ();
			const Vector end_w   = m_startTransform_w.getPosition_p ();
			Game::getCamera ()->addDebugPrimitive (new Line3dDebugPrimitive (Line3dDebugPrimitive::S_none, Transform::identity, start_w, end_w, PackedArgb::solidWhite));
		}

		IGNORE_RETURN(Object::alter(time));

		// Tell the alter system we'll need another alter next frame.
		alterResult = AlterResult::cms_alterNextFrame;
	}
#endif

	//-- Handle altering auxilliary objects.
	if (m_auxilliaryObjectVector)
	{
		ObjectVector::iterator const endIt = m_auxilliaryObjectVector->end();
		for (ObjectVector::iterator it = m_auxilliaryObjectVector->begin(); it != endIt; ++it)
		{
			Object *const auxilliaryObject = *it;
			if (auxilliaryObject)
			{
				float const auxAlterResult = auxilliaryObject->alter(time);
				AlterResult::incorporateAlterResult(alterResult, auxAlterResult);
			}
		}
	}

	// @todo adjust return value appropriately.
	return alterResult;
}

//-----------------------------------------------------------------------

void ClientObject::applyBaselines(const BaselinesMessage & source)
{

	Archive::ReadIterator ri = source.getPackage().begin();
	switch(source.getPackageId())
	{
		case BaselinesMessage::BASELINES_CLIENT_SERVER:
			m_authoritativeClientServerPackage.unpack(ri);
			break;
		case BaselinesMessage::BASELINES_SHARED:
			m_sharedPackage.unpack(ri);
			break;
		case BaselinesMessage::BASELINES_CLIENT_SERVER_NP:
			m_authoritativeClientServerPackage_np.unpack(ri);
			break;
		case BaselinesMessage::BASELINES_SHARED_NP:
			m_sharedPackage_np.unpack(ri);
			break;
		case BaselinesMessage::BASELINES_FIRST_PARENT_CLIENT_SERVER:
			m_firstParentAuthClientServerPackage.unpack(ri);
			break;
		case BaselinesMessage::BASELINES_FIRST_PARENT_CLIENT_SERVER_NP:
			m_firstParentAuthClientServerPackage_np.unpack(ri);
			break;
		case BaselinesMessage::BASELINES_UI:
			WARNING_STRICT_FATAL(!m_synchronizedUi, ("Got a ui package for %s with no ui", getNetworkId().getValueString().c_str()));
			if (m_synchronizedUi)
				m_synchronizedUi->applyBaselines(source);
			break;
		default:
			DEBUG_FATAL(true, ("UNKNOWN PACKAGE TYPE\n"));
			break;
	}
}

//-----------------------------------------------------------------------

void ClientObject::applyDeltas(const DeltasMessage & source)
{
	Archive::ReadIterator ri = source.getPackage().begin();
	switch(source.getPackageId())
	{
		case DeltasMessage::DELTAS_CLIENT_SERVER:
			m_authoritativeClientServerPackage.unpackDeltas(ri);
			m_authoritativeClientServerPackage.clearDeltas();
			break;
		case DeltasMessage::DELTAS_SHARED:
			m_sharedPackage.unpackDeltas(ri);
			m_sharedPackage.clearDeltas();
			break;
		case DeltasMessage::DELTAS_CLIENT_SERVER_NP:
			m_authoritativeClientServerPackage_np.unpackDeltas(ri);
			m_authoritativeClientServerPackage_np.clearDeltas();
			break;
		case DeltasMessage::DELTAS_SHARED_NP:
			m_sharedPackage_np.unpackDeltas(ri);
			m_sharedPackage_np.clearDeltas();
			break;
		case DeltasMessage::DELTAS_FIRST_PARENT_CLIENT_SERVER:
			m_firstParentAuthClientServerPackage.unpackDeltas(ri);
			m_firstParentAuthClientServerPackage.clearDeltas();
			break;
		case DeltasMessage::DELTAS_FIRST_PARENT_CLIENT_SERVER_NP:
			m_firstParentAuthClientServerPackage_np.unpackDeltas(ri);
			m_firstParentAuthClientServerPackage_np.clearDeltas();
			break;
		case DeltasMessage::DELTAS_UI:
			WARNING_STRICT_FATAL(!m_synchronizedUi, ("Got a ui package for %s with no ui", getNetworkId().getValueString().c_str()));
			if (m_synchronizedUi)
			{
				m_synchronizedUi->applyDeltas(source);
				m_synchronizedUi->clearDeltas();
			}
			break;

		default:
			DEBUG_FATAL(true, ("UNKNOWN PACKAGE TYPE\n"));
			break;
	}
}

//-----------------------------------------------------------------------

void ClientObject::beginBaselines()
{
#ifdef _DEBUG
	if (m_beginBaselines)
		DEBUG_WARNING (ms_verbose, ("ClientObject::beginBaselines: beginBaselines called multiple times for this object"));
#endif

	setBeginBaselines(true);
}

//-----------------------------------------------------------------------

void ClientObject::updateContainment(NetworkId const &containerId, int slotArrangement)
{
	if (!NetworkIdManager::getObjectById(containerId))
		WorldSnapshot::loadIfClientCached(containerId);

	getContainedByProperty()->setContainedBy(containerId, false);
	ContainerInterface::getSlottedContainmentProperty(*this)->setCurrentArrangement(slotArrangement, false);
}

//-----------------------------------------------------------------------
/**
 * Called when the object has all its initialization data.
 *
 * This function gets called both in multiplayer and in singleplayer.
 * Derived classes must chain down to their base class version of
 * this function since all layers of the hierachy may be doing
 * important post-creation activity here.
 */

void ClientObject::endBaselines()
{
#ifdef _DEBUG
	if (!m_beginBaselines)
		DEBUG_WARNING (ms_verbose, ("ClientObject::endBaselines: beginBaselines has not been called for this object"));

	if (m_initialized)
		DEBUG_WARNING (ms_verbose, ("ClientObject::endBaselines: endBaselines called multiple times for this object"));
#endif

	setBeginBaselines(false);
	m_initialized = true;

	VolumeContainmentProperty * const vcp = ContainerInterface::getVolumeContainmentProperty (*this);
	if (vcp)
	{
		const int volume = getVolume () > 0 ? getVolume () : 1;
		if (volume != vcp->getVolume ())
		{
			vcp->setVolume (volume);

			Object * const containedBy = ContainerInterface::getContainedByObject (*this);
			if (containedBy)
			{
				VolumeContainer * const vc = containedBy->getVolumeContainerProperty ();
				if (vc)
					IGNORE_RETURN(vc->recalculateVolume ());
			}
		}
	}

	if (Game::getPlayer () == this)
	{
		//Receiving end baselines for the player, which means we have all the objects we need.
		//If we are contained, add us to our containing cell.

		{
			Object *cellObject = ContainerInterface::getContainedByObject(*this);

#ifdef _DEBUG
			ContainedByProperty * prop = ContainerInterface::getContainedByProperty(*this);
			if (!cellObject && prop->getContainedByNetworkId() != NetworkId::cms_invalid)
			{
				const bool isClientCachedResult = WorldSnapshot::isClientCached ( prop->getContainedByNetworkId ().getValue () );
				DEBUG_FATAL(true, ("Received endBaselines for player with non-zero cell id [%s] (which %s client-cached) but could not find cell", prop->getContainedByNetworkId().getValueString ().c_str (), isClientCachedResult ? "is" : "is not"));
			}
#endif

			//-- We only want to set the cell of the player if the object we're contained by is a CellObject.  We could be contained by a mount.
			if (dynamic_cast<CellObject *>(cellObject))
			{
				if ((getAttachedTo() == NULL) || (getAttachedTo() != cellObject))
				{
					const Vector position_w = getPosition_w ();

					//-- if we have no current cell, we must assume the player is in the world.
					//   we must then transform his world position to cell space.
					CellProperty *cellProperty = cellObject->getCellProperty();
					FATAL(!cellProperty, ("non-NULL cellObject [%s] has NULL cellProperty", cellObject->getNetworkId().getValueString()));
					setParentCell(cellProperty);

					CellProperty::setPortalTransitionsEnabled(false);
					setPosition_p(cellProperty == CellProperty::getWorldCellProperty() ? position_w : cellProperty->getOwner ().rotateTranslate_w2o (position_w));
					CellProperty::setPortalTransitionsEnabled(true);
				}
			}
		}
	}

	//-- allow the controller to initialize
	ClientController * const clientController = dynamic_cast<ClientController *>(getController());
	if (clientController)
		clientController->endBaselines();

	//-- hook up any client-side child objects
	{
		ClientDataFile const *const clientDataFile = getClientData();
		if (clientDataFile)
			clientDataFile->apply (this);
	}

#ifdef _DEBUG
	m_startTransform_w = getTransform_o2w ();
#endif

	//-- If the object is in the world, schedule it for alter.  We do this because
	//   World() properly does not allow an uninitialized object to go into the
	//   alter scheduler. (I could argue that an uninitialized object should not go
	//   into the World either, but that is a harder problem to solve).  If the
	//   object is in the world, it should be in the alter scheduler.  At this point
	//   we know that the object is initialized so we can do this check here.
	if (isInWorld())
		scheduleForAlter();
}

//-----------------------------------------------------------------------

bool ClientObject::isInitialized() const
{
	return m_initialized;
}

//-----------------------------------------------------------------------
/**
 * Creates a default controller for this object.
 *
 * @return the object's controller
 */
void ClientObject::createDefaultController(void)
{
	setController(NON_NULL (new ClientController(this)));
}

//-----------------------------------------------------------------------

void ClientObject::setObjectName(const Unicode::String &newName)
{
	m_localizedNameDirty = true;
	m_objectName = newName;
}

//-----------------------------------------------------------------

void ClientObject::setParentCell(CellProperty *cellProperty)
{
	Object::setParentCell(cellProperty);
}

//-----------------------------------------------------------------

void ClientObject::addClientServerVariable(Archive::AutoDeltaVariableBase & source)
{
	m_authoritativeClientServerPackage.addVariable(source);
}

//-----------------------------------------------------------------

void ClientObject::addClientServerVariable_np(Archive::AutoDeltaVariableBase & source)
{
	m_authoritativeClientServerPackage_np.addVariable(source);
}

//-----------------------------------------------------------------

void ClientObject::addFirstParentClientServerVariable(Archive::AutoDeltaVariableBase & source)
{
	m_firstParentAuthClientServerPackage.addVariable(source);
}

//-----------------------------------------------------------------

void ClientObject::addFirstParentClientServerVariable_np(Archive::AutoDeltaVariableBase & source)
{
	m_firstParentAuthClientServerPackage_np.addVariable(source);
}

//-----------------------------------------------------------------

void ClientObject::addSharedVariable(Archive::AutoDeltaVariableBase & source)
{
	m_sharedPackage.addVariable(source);
}

//-----------------------------------------------------------------

void ClientObject::addSharedVariable_np(Archive::AutoDeltaVariableBase & source)
{
	m_sharedPackage_np.addVariable(source);
}

//-----------------------------------------------------------------------

bool ClientObject::isSettingBaselines(void) const
{
	return m_beginBaselines;
}

//-----------------------------------------------------------------------

void ClientObject::setBeginBaselines(bool newBeginBaselines)
{
	m_beginBaselines = newBeginBaselines;
}

//-----------------------------------------------------------------------

const char * ClientObject::getTemplateName(void) const
{
	return getObjectTemplate()->getName();
}

//-----------------------------------------------------------------------

const Unicode::String & ClientObject::getObjectName(void) const
{
	return m_objectName.get();
}

//-----------------------------------------------------------------------

ClientObject* ClientObject::getContainedBy(void) const
{
	ContainedByProperty *prop = ContainerInterface::getContainedByProperty(const_cast<ClientObject&>(*this));
	if (prop && prop->getContainedBy())
	{
		Object * const object = prop->getContainedBy();
		return object->asClientObject();
	}

	return NULL;
}

//-----------------------------------------------------------------------

int ClientObject::getVolume(void) const
{
	return m_volume.get();
}

//-----------------------------------------------------------------------

/**
 * Set the object's volume.
 * Warning:  this is dangerous to call on objects that the server has sent
 * down, because it will make the client's notion of volume limits be wrong.
 * Use it only on client-side objects or in very special cases.
 */
void ClientObject::setVolumeClientSideOnly(int newVolume)
{
	m_volume = newVolume;
	VolumeContainmentProperty * const prop = safe_cast<VolumeContainmentProperty *>(getProperty(VolumeContainmentProperty::getClassPropertyId()));
	if (prop)
		prop->setVolume(newVolume);
}

// ----------------------------------------------------------------------

void ClientObject::containedByModified(const NetworkId & oldValue, const NetworkId & newValue, bool isLocal)
{
	if (oldValue == newValue)
		return;

	if (m_containedByChange)
	{
		//already a pending change.
		m_containedByChange->m_newParent = newValue;
	}
	else
		m_containedByChange = new ContainedByChange(oldValue, newValue, isLocal);

	depersistContainedBy(oldValue, newValue, isLocal);
	handleContainerChangeWithWorld();
}



//------------------------------------------------------------------------------------

void ClientObject::depersistContainedBy(NetworkId const &oldParent, NetworkId const &newParent, bool isLocal)
{
	//**** Handle depersitance
	if (m_clientCached || !isLocal)
	{
		//Handle sync'ing up the contents from the containedBy change for non-auth objects
		ClientObject * const oldContainerObj = dynamic_cast<ClientObject*>(NetworkIdManager::getObjectById(oldParent));
		Container* oldContainer = NULL;
		//remove from the old container
		if (oldContainerObj)
		{
			oldContainer = ContainerInterface::getContainer(*oldContainerObj);
			if (oldContainer)
				IGNORE_RETURN(oldContainer->internalItemRemoved(*this));
		}
		//add to the new container
		ClientObject * const newContainerObj = dynamic_cast<ClientObject*>(NetworkIdManager::getObjectById(newParent));
		if (newContainerObj)
		{
			SlottedContainer* const slotContainer = ContainerInterface::getSlottedContainer(*newContainerObj);
			if (slotContainer)
			{
				SlottedContainmentProperty* const slotted = ContainerInterface::getSlottedContainmentProperty(*this);
				DEBUG_FATAL(!slotted, ("Error unpacking non slotted objects into a slotted container"));
				UNREF (slotted);
				slotContainer->depersistSlotContents(*this, -1);
			}
			else
			{
				Container* const newContainer = ContainerInterface::getContainer(*newContainerObj);

				// Okay, super mega hack warning.  Typically, the player is authoritative for his own position, including his container when it is a cell.
				// However, in the case of mounts, the server will send down a container transfer to put the player in the mount.
				// So the intent here is to ignore all container change messages for the player of this client, except when the destination is a mount or vehicle.
				// One last snag, though, is that this message has to be obeyed when logging into an interior, or the player will not be put in the
				// cell and never be added to the world.
				if (newContainer && (this != Game::getPlayer() || !isInitialized() || (newContainer->getOwner().asClientObject() && newContainer->getOwner().asClientObject()->asCreatureObject())))
				{
					// Even more hackery: we want to update containment, but not update attachment for initialized objects in cells,
					// so we explicitly call the base container version.
					if (isInitialized() && newContainer->getPropertyId() == CellProperty::getClassPropertyId())
						IGNORE_RETURN(newContainer->Container::depersistContents(*this));
					else
						IGNORE_RETURN(newContainer->depersistContents(*this));
				}
			}
		}
	}
//**** Depersitance complete
}

// ----------------------------------------------------------------------

void ClientObject::handleContainerChangeWithWorld()
{
	//**** Handle possible transfer to world
	if (!isAuthoritative() && isInitialized())
	{
		bool wasInWorld = isInWorld();

		bool shouldBeInWorld = true;
		if (m_containedByChange && (m_containedByChange->m_newParent != NetworkId::cms_invalid))
		{
			Object *containerObject = NetworkIdManager::getObjectById(m_containedByChange->m_newParent);
			if (containerObject)
			{
				Container *container = ContainerInterface::getContainer(*containerObject);
				shouldBeInWorld = container->isContentItemExposedWith(*this) && container->getOwner().isInWorld();
			}
			else
			{
				//-- container does not exist, so this object should not be in the world
#ifdef _DEBUG
				const bool containerIsClientCached = WorldSnapshot::isClientCached ( m_containedByChange->m_newParent.getValue () );
				DEBUG_REPORT_LOG_PRINT(true, ("object %s tried to transfer to non-existant container %s [which %s client-cached]\n",
					getNetworkId().getValueString().c_str(),
					m_containedByChange->m_newParent.getValueString().c_str(),
					containerIsClientCached ? "is" : "is not"));
#endif

				shouldBeInWorld = false;
			}
		}

		if (ClientWorld::isInstalled ()) //lint !e1060 // World::isInstalled() isn't visible // Lint bug.
		{
			if (wasInWorld && !shouldBeInWorld)
				removeFromWorld();
			else
				if (!wasInWorld && shouldBeInWorld)
					addToWorld();
		}
	}
	//****World transfers complete
}


// ----------------------------------------------------------------------
/**
 * Called after the specified object has its slotted arrangement changed
 *
 * This default implementation does nothing.
 *
 * Each object is assumed to have no more than one container associated
 * with it; therefore, it is unambiguous which container is under
 * consideration for this object.
 *
 * @param oldValue  The old arrangement
 * @param newValue  The new arrangement
 */

void ClientObject::arrangementModified(int oldValue, int newValue, bool isLocal)
{

	if (oldValue == newValue)
		return;
	if (m_arrangementChange)
	{
		//already a pending change.
		m_arrangementChange->m_newArrangement = newValue;
//		DEBUG_FATAL(isLocal != m_arrangementChange->m_isLocal, ("received remote update in same frame as local...this should never happen"));
	}
	else
		m_arrangementChange = new ArrangementChange(oldValue, newValue, isLocal);
}

//-------------------------------------------------------------------

void ClientObject::handleContainerChange()
{

	//begin code moved from modified
	if (!m_containedByChange && !m_arrangementChange)
		return;

	SlottedContainmentProperty * slotProperty = ContainerInterface::getSlottedContainmentProperty(*this);
	if (!slotProperty)
		WARNING_STRICT_FATAL(true, ("Could not find slotted arrangement property."));

	ClientObject *newContainerObject = dynamic_cast<ClientObject*>(ContainerInterface::getContainedByObject(*this));

	// call updateArrangement if necessary (ie there was an arrangement change).
	if (m_arrangementChange && (m_clientCached || !m_arrangementChange->m_isLocal))
	{
		if (newContainerObject)
		{
			SlottedContainer *const slotContainer	= ContainerInterface::getSlottedContainer(*newContainerObject);
			if (slotContainer)
				slotContainer->updateArrangement(*this, m_arrangementChange->m_oldArrangement, m_arrangementChange->m_newArrangement);
		}
	}

	//-- remove object from old container
	if (m_containedByChange)
	{
		if (!m_transferInProgress || m_containedByChange->m_oldParent != NetworkId::cms_invalid)
		{
			ClientObject *const oldContainerObject = dynamic_cast<ClientObject*>(NetworkIdManager::getObjectById(m_containedByChange->m_oldParent));
			if (oldContainerObject)
			{
				oldContainerObject->removedFromContainer(*this);
			}
		}
	}
	// if there was no container change, there was an arrangement change, and we'll have to remove the
	// object from the current container and re-add it.
	else if (newContainerObject)
	{
		newContainerObject->removedFromContainer(*this);
	}

	//-- add object to new container
	if (newContainerObject && slotProperty)
		newContainerObject->addedToContainer(*this, slotProperty->getCurrentArrangement());

	if(newContainerObject)
	{
		SlottedContainmentProperty * creatureContainerSlotProperty = ContainerInterface::getSlottedContainmentProperty(*newContainerObject);
		if(creatureContainerSlotProperty)
		{
			CreatureObject * const parent = dynamic_cast<CreatureObject*>(ContainerInterface::getContainedByObject(*newContainerObject));
			if (parent)
			{
				int oldArrangement = m_arrangementChange ? m_arrangementChange->m_oldArrangement : -1;
				if(creatureContainerSlotProperty && parent != newContainerObject)
					parent->onContainedSlottedContainerChanged(*this, *newContainerObject, slotProperty->getCurrentArrangement(), oldArrangement);
			}
		}
	}

	delete m_containedByChange;
	m_containedByChange = 0;
	delete m_arrangementChange;
	m_arrangementChange = 0;
}

// ----------------------------------------------------------------------

/**
 * Called after the specified object is added to the container associated
 * with this object.
 *
 * This default implementation does nothing.
 *
 * Each object is assumed to have no more than one container associated
 * with it; therefore, it is unambiguous which container is under
 * consideration for this object.
 *
 * @param containedObject  The argument has been added to the container
 *                         associated with this object.
 */

void ClientObject::addedToContainer(ClientObject &containedObject, int currentArrangement)
{
	//-- recalculate volume if a volume container
	VolumeContainer * const volumeContainer = ContainerInterface::getVolumeContainer (*this);
	if (volumeContainer)
		IGNORE_RETURN (volumeContainer->recalculateVolume ());

	UNREF(currentArrangement);
	Transceivers::s_addedToContainer.emitMessage (Messages::ContainerMsg (this, &containedObject));
}

// ----------------------------------------------------------------------
/**
 * Called after the specified object is removed from the container associated
 * with this object.
 *
 * This default implementation does nothing.
 *
 * Each object is assumed to have no more than one container associated
 * with it; therefore, it is unambiguous which container is under
 * consideration for this object.
 *
 * @param containedObject  The argument has been removed from the container
 *                         associated with this object.
 */

void ClientObject::removedFromContainer(ClientObject &containedObject)
{
	//-- recalculate volume if a volume container
	VolumeContainer * const volumeContainer = ContainerInterface::getVolumeContainer (*this);
	if (volumeContainer)
		IGNORE_RETURN (volumeContainer->recalculateVolume ());

	Transceivers::s_removedFromContainer.emitMessage (Messages::ContainerMsg (this, &containedObject));
}

//----------------------------------------------------------------------

void ClientObject::setSynchronizedUi (ClientSynchronizedUi * sync)
{
	if (m_synchronizedUi != NULL)
	{
		if (m_synchronizedUi == sync)
		{
			DEBUG_WARNING (true, ("ClientObject::setSynchronizedUi received same sui object\n"));
		}
		else
		{
			WARNING_STRICT_FATAL (true, ("ClientObject::setSynchronizedUi received "
				"sui object without the previous one being cleared"));
			delete m_synchronizedUi;
			m_synchronizedUi = 0;
		}
	}
	m_synchronizedUi = sync;
}

//----------------------------------------------------------------------

ClientSynchronizedUi * ClientObject::createSynchronizedUi ()
{
	return 0;
}

//----------------------------------------------------------------------

void ClientObject::setTransferInProgress (bool b)
{
	m_transferInProgress = b;
}

//-----------------------------------------------------------------

void ClientObject::updateLocalizedName () const
{
	bool localizedNameSameAsEnglishLocalizedName = false;

	if (!m_objectName.get().empty())
	{
		m_localizedName = m_objectName.get();
		m_localizedEnglishName = m_localizedName;
		localizedNameSameAsEnglishLocalizedName = true;
	}
	else
	{
		if (!m_nameStringId.get ().isInvalid ())
		{
			IGNORE_RETURN (LocalizationManager::getManager ().getLocalizedStringValue (m_nameStringId.get (), m_localizedName));
			IGNORE_RETURN (LocalizationManager::getManager ().getLocalizedStringValue (m_nameStringId.get (), m_localizedEnglishName, true));
			localizedNameSameAsEnglishLocalizedName = (m_localizedName == m_localizedEnglishName);
		}
		else
		{
			const char * const otname = getTemplateName();
			if (otname)
				m_localizedName = Unicode::narrowToWide(otname);
			else
				m_localizedName = Unicode::narrowToWide("error_name");

			m_localizedEnglishName = m_localizedName;
			localizedNameSameAsEnglishLocalizedName = true;
		}
	}

	filterLocalizedName (m_localizedName);
	
	if (localizedNameSameAsEnglishLocalizedName)
		m_localizedEnglishName = m_localizedName;
	else
		filterLocalizedName (m_localizedEnglishName);
}

//-----------------------------------------------------------------

void ClientObject::Callbacks::NameChange::modified (ClientObject & target, const Unicode::String &, const Unicode::String &, bool) const
{
	target.m_localizedNameDirty = true;
	Transceivers::nameChanged.emitMessage (target);
}

//-----------------------------------------------------------------

void ClientObject::Callbacks::StringIdChange::modified (ClientObject & target, const StringId &, const StringId &, bool) const
{
	target.m_localizedNameDirty = true;
	Transceivers::nameChanged.emitMessage (target);
}

//-----------------------------------------------------------------

void ClientObject::Callbacks::ComplexityChange::modified (ClientObject & target, const float &, const float &, bool) const
{
	Transceivers::s_complexityChange.emitMessage (target);
} //lint !e1764 // target could be made const // unfortunately not, the signature is locked in for us.

//-----------------------------------------------------------------

void ClientObject::Callbacks::BankBalance::modified (ClientObject & target, const int &, const int &, bool) const
{
	Transceivers::s_bankBalance.emitMessage (target);
} //lint !e1764 // target could be made const // unfortunately not, the signature is locked in for us.

//----------------------------------------------------------------------

void ClientObject::Callbacks::CashBalance::modified (ClientObject & target, const int &, const int &, bool) const
{
	Transceivers::s_cashBalance.emitMessage (target);
} //lint !e1764 // target could be made const // unfortunately not, the signature is locked in for us.

//-----------------------------------------------------------------

const Unicode::String &ClientObject::getLocalizedName() const
{
	if (m_localizedNameDirty)
	{
		updateLocalizedName();
		m_localizedNameDirty = false;
	}

	return m_localizedName;
}

//-----------------------------------------------------------------

const Unicode::String &ClientObject::getLocalizedEnglishName() const
{
	if (m_localizedNameDirty)
	{
		updateLocalizedName();
		m_localizedNameDirty = false;
	}

	return m_localizedEnglishName;
}

//----------------------------------------------------------------------

const Unicode::String ClientObject::getLocalizedFirstName() const
{
	const Unicode::String & name = getLocalizedName();

	std::string::size_type spacePos = name.find(' ');

	if (spacePos != std::string::npos)
	{
		return name.substr(0, spacePos);
	}

	return name;
}

//-----------------------------------------------------------------

void ClientObject::setObjectNameStringId(const StringId & id)
{
	m_nameStringId = id;
}

//-----------------------------------------------------------------

const StringId & ClientObject::getObjectNameStringId () const
{
	return m_nameStringId.get();
}

//----------------------------------------------------------------------

float ClientObject::getComplexity () const
{
	return m_complexity.get ();
}

//----------------------------------------------------------------------

void ClientObject::setComplexity (float c)
{
	m_complexity = c;
}

//----------------------------------------------------------------------

void ClientObject::setClientCached ()
{
	m_clientCached = true;
}

//----------------------------------------------------------------------

bool ClientObject::isClientCached () const
{
	return m_clientCached;
}

//----------------------------------------------------------------------

const StringId & ClientObject::getDescriptionStringId () const
{
	return m_descriptionStringId.get();
}

//----------------------------------------------------------------------

const bool ClientObject::getLocalizedDescription (Unicode::String & str) const
{
	const StringId & stringId = getDescriptionStringId ();

	if (stringId.isValid ())
		return LocalizationManager::SVC_ok == LocalizationManager::getManager ().getLocalizedStringValue (stringId, str);
	else
	{
		str.clear ();
		return true;
	}
}

//-----------------------------------------------------------------

ClientObject::ContainedByChange::ContainedByChange(const NetworkId & old, const NetworkId & n, bool local) :
m_oldParent(old),
m_newParent(n),
m_isLocal(local)
{
}

//-----------------------------------------------------------------

ClientObject::ArrangementChange::ArrangementChange(int const & old, int const & n, bool local) :
m_oldArrangement(old),
m_newArrangement(n),
m_isLocal(local)
{
}

//----------------------------------------------------------------------

int ClientObject::getGameObjectType          () const
{
	const SharedObjectTemplate * const sot = safe_cast<const SharedObjectTemplate *> (getObjectTemplate());
	NOT_NULL (sot);
	return static_cast<int>(sot->getGameObjectType());
}

//----------------------------------------------------------------------

const std::string & ClientObject::getGameObjectTypeName      () const
{
	return GameObjectTypes::getCanonicalName (getGameObjectType ());
}

//----------------------------------------------------------------------

const StringId & ClientObject::getGameObjectTypeStringId  () const
{
	return GameObjectTypes::getStringId (getGameObjectType ());
}

//----------------------------------------------------------------------

const Unicode::String & ClientObject::getGameObjectTypeLocalizedName () const
{
	return GameObjectTypes::getLocalizedName (getGameObjectType ());
}

//----------------------------------------------------------------------

bool ClientObject::isGameObjectTypeOf             (int type) const
{
	return GameObjectTypes::isTypeOf (getGameObjectType (), type);
}

//-----------------------------------------------------------------
/**
 * Add fly text to appear over the top of this object.
 *
 * Fly text is used by damage and may be produced via script on the
 * server.  The text flies over the top of the object, traveling upward
 * for a specified amount of time, fading out during the time interval.
 *
 * @param flyText    The text to display above the object.
 * @param fadeTime   The amount of time, in seconds, during which the text will
 *                   float up above the object.
 * @param color      The color to make the rendered text.
 * @param textScale  A text scale value of 1.0 is the normal user text font size.
 *                   A value of 1.5 is what is used as of this writing for
 *                   the large damage text.
 */

Object * ClientObject::addFlyText (const Unicode::String & flyText, float fadeTime, const VectorArgb & color, float textScale, int textType, bool fadeOut, bool renderWithChatBubbles, bool /*createInWorldspace*/)
{
	UNREF(textType);
	textScale *= CuiPreferences::getFlyTextSize();
	bool isAlphaText = false;
	for(Unicode::String::const_iterator it = flyText.begin(); it != flyText.end(); ++it)
	{
		if(isAlpha(*it))
			isAlphaText = true;
	}
	bool isDoneToMe = false;
	if(Game::getPlayer() == this)
		isDoneToMe = true;
	if(!isAlphaText && isDoneToMe)
		textScale *= CuiCombatManager::getDamageDoneToMeSizeModifier();
	else if(!isAlphaText && !isDoneToMe)
		textScale *= CuiCombatManager::getDamageDoneToOthersSizeModifier();
	else if(isAlphaText && isDoneToMe)
		textScale *= CuiCombatManager::getNonDamageDoneToMeSizeModifier();
	else if(isAlphaText && !isDoneToMe)
		textScale *= CuiCombatManager::getNonDamageDoneToOthersSizeModifier();

	if (asShipObject() != 0)
	{
		return 0;
	}

	Appearance const * const appearance = getAppearance();
	if ((appearance != 0) && (Game::getPlayer() != this))
	{
		// don't render if not on screen unless it is the player
		bool const renderedLastFrame = appearance->getRenderedLastFrame();
		bool const renderedThisFrame = appearance->getRenderedThisFrame();

		if (!(renderedLastFrame || renderedThisFrame))
		{
			return 0;
		}
	}

	//float flyTextSpeedPixelsPerSecond = -500.f; // pixels/second
	//float flyTextSpeedModifier = 0.9f;
	float const flyTextSpeedPixelsPerSecond = ConfigClientGame::getFlyTextSpeedPixelsPerSecond();
	float const flyTextSpeedModifier = ConfigClientGame::getFlyTextSpeedModifier();

	if(s_invalidTargetString.empty())
		s_invalidTargetString = s_invalidTargetStringId.localize();	

	//-- Create the text object.
	bool const skipFirstAlter = true;
	Object * const textObject = new TimerObject(fadeTime, skipFirstAlter);
	textObject->setParentCell(getParentCell());
	RenderWorld::addObjectNotifications(*textObject);
	textObject->addNotification(ClientWorld::getIntangibleNotification());

	// offset multiple text so that they stack and don't overlap
	const double currentTime = Clock::getCurrentTime();
	static float textOffsetFactor = 1.0f;
	if(m_lastFlyTextWasInvalidTarget && ((currentTime - m_lastFlyTextTime) < INVALID_TARGET_ENFORCE_DELAY))		
		textOffsetFactor += 1.0f;
	else
		textOffsetFactor = 1.0f;
	m_lastFlyTextWasInvalidTarget = true;
	
	m_lastFlyTextTime = static_cast<float>(currentTime);

	{
		Camera const * const camera = Game::getCamera ();
		if (camera)
		{
			//-- Setup text object's appearance.
			const float approximateTextHeight = 15.f * textScale;
			const float yOffset = approximateTextHeight * textOffsetFactor;
			Vector screenSpacePosition;

			// make the text scroll if fadeOut is set
			Vector screenSpaceVelocity(0.f, ((fadeOut) ? flyTextSpeedPixelsPerSecond : 0.0f), 0.f);

			textScale *= s_playerFlyTextScale;

			bool useThisObjectScreenSpace = true;
			// otherwise focus on center of screen

			if (Game::getPlayer () != this)
			{
				Vector const headPoint(CuiObjectTextManager::getCurrentObjectHeadPoint_o(*this));
				Vector const headPoint_w(rotateTranslate_o2w(headPoint));

				float x = 0.0f;
				float y = 0.0f;
				float z = 0.0f;

				if (camera->projectInWorldSpace(headPoint_w, &x, &y, &z, false))
				{
					screenSpacePosition.set(x, y + yOffset, z);
					useThisObjectScreenSpace = false;
				}
			}

			if (useThisObjectScreenSpace)
			{
				float x0, x1, y0, y1;
				camera->getViewport(x0,y0,x1,y1);
				// center the player text on screen w/a random placement
				screenSpacePosition.set(x0 + ((x1-x0) * 0.5f), y0 + ((y1-y0) * 0.5f) + yOffset, 1.f);
			}

			FadingTextAppearance * const app = new FadingTextAppearance(flyText, color, 1.0f, fadeTime, textScale, CuiTextManagerTextEnqueueInfo::TW_starwars, screenSpacePosition, screenSpaceVelocity, flyTextSpeedModifier);

			app->setFadeEnabled(fadeOut);
			app->setRenderWithChatBubbles(renderWithChatBubbles);
			textObject->setAppearance(app);

			textObject->addToWorld();
			textObject->scheduleForAlter();
		}
	}

	//-- Make sure this object gets at least one alter so the fly text child shows up.
	scheduleForAlter();

	return textObject; //lint !e429 // custodial pointer app not freed or returned // it's okay, the appearance is owned by textObject.
}

// ----------------------------------------------------------------------
/**
 * Return the number of auxilliary objects associated with this object.
 *
 * Auxilliary objects are objects with lifetimes that are controlled by
 * the client object that had the object added to its auxilliary object list.
 * Auxilliary objects do not participate in the Object parent-child or attachment
 * relationship but they are altered with the owning object.  Auxilliary
 * objects do not get rendered via the object list camera.
 *
 * Example usage: the mounts system adds the mount's covering (a wearable)
 * as an auxilliary object so that the covering doesn't get rendered twice.
 *
 * @return the number of auxilliary objects associated with this object.
 */

int ClientObject::getNumberOfAuxilliaryObjects() const
{
	if (!m_auxilliaryObjectVector)
		return 0;
	else
		return static_cast<int>(m_auxilliaryObjectVector->size());
}

// ----------------------------------------------------------------------

void ClientObject::addAuxilliaryObject(Object *object)
{
	if (!object)
	{
		DEBUG_WARNING(true, ("ClientObject::addAuxilliaryObject() called with a NULL object."));
		return;
	}

	if (!m_auxilliaryObjectVector)
		m_auxilliaryObjectVector = new ObjectVector;

	m_auxilliaryObjectVector->push_back(object);
}

// ----------------------------------------------------------------------

Object *ClientObject::getAuxilliaryObject(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfAuxilliaryObjects());
	NOT_NULL(m_auxilliaryObjectVector);

	return (*m_auxilliaryObjectVector)[static_cast<ObjectVector::size_type>(index)];
}

// ----------------------------------------------------------------------

Object const *ClientObject::getAuxilliaryObject(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfAuxilliaryObjects());
	NOT_NULL(m_auxilliaryObjectVector);

	return (*m_auxilliaryObjectVector)[static_cast<ObjectVector::size_type>(index)];
}

//----------------------------------------------------------------------

void ClientObject::filterLocalizedName (Unicode::String &) const
{
	//-- subclasses can override this to make strings like 'a corpse of %s'.
}

//----------------------------------------------------------------------

void ClientObject::setLocalizedNameDirty ()
{
	m_localizedNameDirty = true;
}

//----------------------------------------------------------------------

const ClientDataFile* ClientObject::getClientData () const
{
	const SharedObjectTemplate* const sharedObjectTemplate = dynamic_cast<const SharedObjectTemplate*> (getObjectTemplate ());
	if (sharedObjectTemplate && sharedObjectTemplate->getClientData ())
	{
		ClientDataFile const *const cdFile = dynamic_cast<const ClientDataFile*> (sharedObjectTemplate->getClientData ());
		return cdFile;
	}
	else
		return NULL;
}

//----------------------------------------------------------------------

ClientObject * ClientObject::findClientObjectByUniqueId (uint32 id)
{
	IdObjectMap::const_iterator it = ms_idObjectMap.find (id);
	if (it != ms_idObjectMap.end ())
		return (*it).second;

	return 0;
}

//----------------------------------------------------------------------

TerrainGenerator::Layer* ClientObject::getLayer () const
{
	const Property * property = getProperty(LayerProperty::getClassPropertyId());
	if (property != NULL)
	{
		const LayerProperty * layerProperty = safe_cast<const LayerProperty *>(property);
		return layerProperty->getLayer();
	}
	return NULL;
}

//----------------------------------------------------------------------

void ClientObject::setLayer(TerrainGenerator::Layer* layer)
{
	LayerProperty * layerProperty = NULL;
	Property * property = getProperty(LayerProperty::getClassPropertyId());
	if (property != NULL)
		layerProperty = safe_cast<LayerProperty *>(property);
	else
		layerProperty = new LayerProperty(*this);

	layerProperty->setLayer(layer);
	if (property == NULL)
	{
		addProperty(*layerProperty, true);
		addNotification (ClientObjectTerrainModificationNotification::getInstance ());
	}
}

// ----------------------------------------------------------------------

void ClientObject::getObjectInfo(std::map<std::string, std::map<std::string, Unicode::String> > & propertyMap) const
{
#if PRODUCTION == 0
/**
  When adding a variable to this class, please add it here.  Variable that aren't easily displayable are still listed, for tracking purposes.
*/

/**
	Don't compile in production build because this maps human-readable values to data members and makes hacking easier
*/
//	const ArrangementDescriptor     *m_arrangementDescriptor;
//	Archive::AutoDeltaByteStream     m_authoritativeClientServerPackage;
//	Archive::AutoDeltaByteStream     m_authoritativeClientServerPackage_np;
//	Archive::AutoDeltaByteStream     m_firstParentAuthClientServerPackage;
//	Archive::AutoDeltaByteStream     m_firstParentAuthClientServerPackage_np;
//	Archive::AutoDeltaByteStream     m_sharedPackage;
//	Archive::AutoDeltaByteStream     m_sharedPackage_np; //np refers to the not persisted version of this package
//	ClientSynchronizedUi            *m_synchronizedUi;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "TransferInProgress", m_transferInProgress);
#ifdef _DEBUG
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "StartTransform_w", m_startTransform_w);
#endif
//	ContainedByChange*              m_containedByChange;
//	ArrangementChange*              m_arrangementChange;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "BeginBaselines", m_beginBaselines);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Initialized", m_initialized);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "LocalizedName", m_localizedName);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "LocalizedEnglishName", m_localizedEnglishName);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "LocalizedNameDirty", m_localizedNameDirty);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Volume", m_volume.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "ObjectName", m_objectName.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "ObjectNameStringId", m_nameStringId.get().getCanonicalRepresentation());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Complexity", m_complexity.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "AuthServerProcessId", m_authServerProcessId.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "CashBalance", m_cashBalance.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "BankBalance", m_bankBalance.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "ClientCached", m_clientCached);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "UniqueId", m_uniqueId);
//	ObjectVector                    *m_auxilliaryObjectVector;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "LastFlyTextTime", m_lastFlyTextTime);
#ifdef _DEBUG
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "WarnedNotInitialized", m_warnedNotInitialized);
#endif

	Object::getObjectInfo(propertyMap);

#else
	UNREF(propertyMap);
#endif
}


// ======================================================================
