// ======================================================================
//
// SaddleManager.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/SaddleManager.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/VehicleHoverDynamicsClient.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/HardpointObject.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/SkeletalAppearanceTemplate.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/WearableAppearanceMap.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/MemoryBlockManagedObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <algorithm>
#include <string>
#include <vector>

#ifdef _DEBUG
#include "sharedDebug/DebugFlags.h"
#endif

// ======================================================================

namespace SaddleManagerNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class SaddleNonHardpointObject : public Object
	{
	public:

		SaddleNonHardpointObject() : Object() {}

		virtual void scheduleForAlter()
			{
				scheduleForAlter_scheduleTopmostWorldParent();
			}

	private:

		// Disabled.
		SaddleNonHardpointObject(SaddleNonHardpointObject const&);
		SaddleNonHardpointObject &operator =(SaddleNonHardpointObject const&);
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void    remove();

	int     getRequiredColumnNumberFromNamedTable(char const *filename, DataTable const *table, char const *columnName);

	void    createAndWearWearable(CreatureObject &object, SkeletalAppearance2 &appearance, CrcString const &newAppearanceName);
	Object *createAndAttachAppearance(CreatureObject &object, CrcString const &attachmentHardpointName, CrcString const &newAppearanceName);
	CrcString const * getLogicalSaddleNameFromMount(CreatureObject const & mount);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	ConstCharCrcString const cs_defaultCoveringLogicalAppearanceName("lookup/mnt_wr_default_covering");
	ConstCharCrcString const cs_saddleHardpointName("saddle");
	char const * const cs_driverHardpointName = "player";
	char const * const cs_passengerHardpointName = "passenger";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool                            s_installed;

#ifdef _DEBUG
	bool                            s_logSaddleScale;
#endif
}

using namespace SaddleManagerNamespace;

// ======================================================================
// namespace SaddleManagerNamespace
// ======================================================================

void SaddleManagerNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("SaddleManager not installed."));
	s_installed = false;

#ifdef _DEBUG
	DebugFlags::unregisterFlag(s_logSaddleScale);
#endif
}

// ----------------------------------------------------------------------

int SaddleManagerNamespace::getRequiredColumnNumberFromNamedTable(char const *filename, DataTable const *table, char const *columnName)
{
	NOT_NULL(columnName);

	int const columnNumber = table->findColumnNumber(columnName);
	FATAL(columnNumber < 0, ("failed to find column name [%s] in file [%s].", columnName, filename));

	return columnNumber;
}

// ----------------------------------------------------------------------

void SaddleManagerNamespace::createAndWearWearable(CreatureObject &object, SkeletalAppearance2 &appearance, CrcString const &newAppearanceName)
{
	//-- Create the new appearance.
	Appearance *const newBaseAppearance = AppearanceTemplateList::createAppearance(newAppearanceName.getString());
	if (!newBaseAppearance)
	{
		DEBUG_WARNING(true, ("createAndWearWearable(): failed to create new appearance [%s].", newAppearanceName.getString()));
		return;
	}

	//-- Ensure its a SkeletalAppearance2.
	SkeletalAppearance2 *const newAppearance = newBaseAppearance->asSkeletalAppearance2();
	if (!newAppearance)
	{
		DEBUG_WARNING(true, ("createAndWearWearable(): new appearance [%s] is not skeletal, bad data.", newAppearanceName.getString()));
		delete newBaseAppearance;
		return;
	}

	//-- Create the new object for the wearable.
	Object *const newObject = new MemoryBlockManagedObject();
	if (!newObject)
	{
		DEBUG_WARNING(true, ("createAndWearWearable(): failed to create new object for new appearance [%s].", newAppearanceName.getString()));
		delete newAppearance;
		return;
	}

	//-- Set object appearance.
	newObject->setAppearance(newAppearance);

	//-- Wear the new appearance.
	appearance.wear(newObject);

	//-- We want the wearable to get altered but we don't want it to get rendered
	//   via object camera list.
	object.addAuxilliaryObject(newObject);
}

// ----------------------------------------------------------------------

Object *SaddleManagerNamespace::createAndAttachAppearance(CreatureObject &object, CrcString const &attachmentHardpointName, CrcString const &newAppearanceName)
{
	//-- Create the new appearance.
	Appearance *const newAppearance = AppearanceTemplateList::createAppearance(newAppearanceName.getString());
	if (!newAppearance)
	{
		DEBUG_WARNING(true, ("createAndAttachAppearance(): failed to create new appearance [%s].", newAppearanceName.getString()));
		return NULL;
	}

	//-- Create the new object for the attachment.
	Object *newObject = NULL;

	bool useHardpoint = strlen(attachmentHardpointName.getString()) > 0;
	if (!useHardpoint)
		newObject = new SaddleNonHardpointObject();
	else
	{
		//   ** NOTE ** this is potentially an expensive implementation strategy:
		//   it will cause the mount skeleton to get evaluated every time the hardpoint
		//   object alters.  We might want to look it iup and position the object there.
		newObject = new HardpointObject(attachmentHardpointName);
	}

	if (!newObject)
	{
		DEBUG_WARNING(true, ("createAndAttachAppearance(): failed to create new object for new appearance [%s].", newAppearanceName.getString()));
		delete newAppearance;
		return NULL;
	}

	//-- Modify scale of attached object so that the creature's scale times the appearance scale is 1.0.
	if (useHardpoint)
	{
		float const creatureScale   = object.getScaleFactor();
		float const attachmentScale = (creatureScale > 0.0f ? 1.0f / creatureScale : 1.0f);
		DEBUG_REPORT_LOG(s_logSaddleScale, ("SaddleManager: mount id=[%s], mount scale=[%.2f], saddle reverse scale=[%.2f].\n", object.getNetworkId().getValueString().c_str(), creatureScale, attachmentScale));

		newObject->setScale(Vector(attachmentScale, attachmentScale, attachmentScale));
	}

	//-- Set object appearance.
	newObject->setAppearance(newAppearance);

	//-- Add to render world so it renders properly --- not using attachment system.
	//   This prevents the creature from scaling it.
	RenderWorld::addObjectNotifications(*newObject);

	//-- Attach the new object so it gets deleted properly.
	newObject->attachToObject_p(&object, true);

	//-- Return the saddle object to the caller.
	return newObject;
}

// ----------------------------------------------------------------------

CrcString const * SaddleManagerNamespace::getLogicalSaddleNameFromMount(CreatureObject const & mount)
{
	//-- Get mount appearance.
	Appearance const *const mountAppearance = mount.getAppearance();
	if (!mountAppearance)
	{
		DEBUG_WARNING(true, ("SaddleManagerNamespace::getLogicalSaddleName(): mount id=[%s],template=[%s] has NULL appearance.", mount.getNetworkId().getValueString().c_str(), mount.getObjectTemplateName()));
		return NULL;
	}

	AppearanceTemplate const *const mountAppearanceTemplate = mountAppearance->getAppearanceTemplate();
	if (!mountAppearanceTemplate)
	{
		DEBUG_WARNING(true, ("SaddleManagerNamespace::getLogicalSaddleName(): mount id=[%s],template=[%s] has NULL appearance template.", mount.getNetworkId().getValueString().c_str(), mount.getObjectTemplateName()));
		return NULL;
	}

	//-- Get logical saddle name.
	// ... first lookup logical saddle name by appearance template name.
	CrcString const *logicalSaddleName = SharedSaddleManager::getLogicalSaddleNameForMountAppearance(mountAppearanceTemplate->getCrcName());
	if (!logicalSaddleName)
	{
		// ... second, lookup logical saddle name by the primary skeleton template name.
		SkeletalAppearanceTemplate const *const mountSat = dynamic_cast<SkeletalAppearanceTemplate const*>(mountAppearanceTemplate);
		if (!mountSat)
		{
			DEBUG_WARNING(true, ("SaddleManagerNamespace::getLogicalSaddleName(): failed: mount id=[%s],template=[%s]: appearance template is not a SkeletalAppearanceTemplate.", mount.getNetworkId().getValueString().c_str(), mount.getObjectTemplateName()));
			return NULL;
		}

		CrcString const &primarySkeletonTemplateName = mountSat->getSkeletonTemplateName(mountSat->getSkeletonTemplateInfo(0));
		logicalSaddleName = SaddleManager::getLogicalSaddleNameForMountAppearance(primarySkeletonTemplateName);

		if (!logicalSaddleName)
		{
			DEBUG_WARNING(true, ("SaddleManagerNamespace::getLogicalSaddleName(): failed: mount id=[%s],template=[%s]: failed to find logical mount id for appearance name [%s] or skeleton name [%s].", mount.getNetworkId().getValueString().c_str(), mount.getObjectTemplateName(), mountAppearanceTemplate->getCrcName().getString(), primarySkeletonTemplateName.getString()));
			return NULL;
		}
	}

	return logicalSaddleName;
}

// ======================================================================
// class SaddleManager: PUBLIC STATIC
// ======================================================================

void SaddleManager::install(char const *logicalSaddleNameMapFile, char const *saddleAppearanceMapFile, char const *riderPoseMapFile)
{
	InstallTimer const installTimer("SaddleManager::install");

	NOT_NULL(logicalSaddleNameMapFile);
	NOT_NULL(saddleAppearanceMapFile);
	NOT_NULL(riderPoseMapFile);
	DEBUG_FATAL(s_installed, ("SaddleManager already installed."));

#ifdef _DEBUG
	DebugFlags::registerFlag(s_logSaddleScale, "ClientGame/Mounts", "logSaddleScale");
#endif

	s_installed = true;
	ExitChain::add(SaddleManagerNamespace::remove, "SaddleManager");
}

// ----------------------------------------------------------------------

CrcString const *SaddleManager::getSaddleAppearanceNameForMount(CreatureObject const &mount, CrcString & clientDataFilename)
{
	CrcString const * const logicalSaddleName = getLogicalSaddleNameFromMount(mount);

	//-- Get saddle appearance name from logical saddle name and capacity.
	NOT_NULL(logicalSaddleName);

	CrcString const *saddleAppearanceName = getSaddleAppearanceForLogicalSaddleName(*logicalSaddleName);
	if (saddleAppearanceName)
	{
		const CrcString * const cdfName = getClientDataFilenameForLogicalSaddleName(*logicalSaddleName);
		if (cdfName)
			clientDataFilename.set(*cdfName);
	}

	return saddleAppearanceName;
}

// ----------------------------------------------------------------------

CrcString const *SaddleManager::getRiderPoseNameForMountSeatIndex(CreatureObject const &mount, int seatIndex)
{
	PersistentCrcString clientDataFilename;
	//-- Lookup the saddle appearance name for the mount and seating capacity.
	CrcString const * const saddleAppearanceName = getSaddleAppearanceNameForMount(mount, clientDataFilename);
	if (!saddleAppearanceName)
	{
		DEBUG_WARNING(true, ("SaddleManager::getRiderPoseNameForMountSeatingCapacityAndSeatIndex(): failed: mount id=[%s],template=[%s]: could not lookup saddle name for mount.", mount.getNetworkId().getValueString().c_str(), mount.getObjectTemplateName()));
		return NULL;
	}

	//-- Lookup rider pose name that corresponds to saddle appearance name and seat index.
	CrcString const *const riderPoseName = getRiderPoseForSaddleAppearanceNameAndRiderSeatIndex(*saddleAppearanceName, seatIndex);
	return riderPoseName;
}

// end of shared

// ----------------------------------------------------------------------

void SaddleManager::addDressingToMount(CreatureObject &mount)
{
	DEBUG_FATAL(!s_installed, ("SaddleManager not installed."));

	//-- Get the skeletal appearance.
	// Get base appearance.
	Appearance *const baseAppearance = mount.getAppearance();
	if (!baseAppearance)
	{
		DEBUG_WARNING(true, ("SaddleManager::addDressingToMount(): mount id=[%s],template=[%s] has NULL appearance.", mount.getNetworkId().getValueString().c_str(), mount.getObjectTemplateName()));
		return;
	}

	// Convert to skeletal appearance.
	SkeletalAppearance2 *const appearance = baseAppearance->asSkeletalAppearance2();
	if (!appearance)
	{
		DEBUG_WARNING(true, ("SaddleManager::addDressingToMount(): mount id=[%s],template=[%s] doesn't have a skeletal appearance, appearance template name=[%s].", mount.getNetworkId().getValueString().c_str(), mount.getObjectTemplateName(), baseAppearance->getAppearanceTemplateName()));
		return;
	}

	//-- Lookup the name of the default dressing appearance for this species.
	AppearanceTemplate const *wearerAppearanceTemplate = appearance->getAppearanceTemplate();
	if (!wearerAppearanceTemplate)
	{
		DEBUG_WARNING(true, ("addDressingToMount(): failed because wearer object id=[%s],template=[%s] had an appearance with a NULL appearance template.", mount.getNetworkId().getValueString().c_str(), mount.getObjectTemplateName()));
		return;
	}

	WearableAppearanceMap::MapResult const coveringMapResult = WearableAppearanceMap::getMapResultForWearableAndWearer(cs_defaultCoveringLogicalAppearanceName, wearerAppearanceTemplate->getCrcName());
	if (!coveringMapResult.hasMapping())
	{
		DEBUG_WARNING(true, ("addDressingToMount(): failed because wearer appearance name [%s] did not have a wearable map entry for wearable [%s].", wearerAppearanceTemplate->getCrcName().getString(), cs_defaultCoveringLogicalAppearanceName.getString()));
		return;
	}

	if (coveringMapResult.isWearableForbidden())
	{
		DEBUG_WARNING(true, ("addDressingToMount(): failed because wearer appearance name [%s] had wearable [%s] marked as forbidden in the wearable appearance map.", wearerAppearanceTemplate->getCrcName().getString(), cs_defaultCoveringLogicalAppearanceName.getString()));
		return;
	}

	CrcString const *coveringAppearanceName = coveringMapResult.getMappedWearableAppearanceName();
	if (!coveringAppearanceName)
	{
		DEBUG_WARNING(true, ("addDressingToMount(): failed: wearer appearance name [%s], wearable [%s] returned a NULL mapped appearance name in what otherwise appeared to be a valid map entry.", wearerAppearanceTemplate->getCrcName().getString(), cs_defaultCoveringLogicalAppearanceName.getString()));
		return;
	}

	//-- Attach covering/saddle dressing wearable.
	createAndWearWearable(mount, *appearance, *coveringAppearanceName);
}

// ----------------------------------------------------------------------

Object *SaddleManager::addRiderSaddleToMount(CreatureObject &mount)
{
	DEBUG_FATAL(!s_installed, ("SaddleManager not installed."));

	//-- Specify seating capacity.  Later this will need to come from an attribute on the server.
	PersistentCrcString clientDataFilename;
	CrcString const * const saddleAppearanceName = getSaddleAppearanceNameForMount(mount, clientDataFilename);

	if (!saddleAppearanceName)
	{
		DEBUG_WARNING(true, ("SaddleManager::addRiderSaddleToMount(): failed to find an appropriate saddle appearance for mount id=[%s],template=[%s].", mount.getNetworkId().getValueString().c_str(), mount.getObjectTemplateName()));
		return NULL;
	}

	//-- Attach saddle static mesh.
	bool const isVehicle = GameObjectTypes::isTypeOf (mount.getGameObjectType (), static_cast<int>(SharedObjectTemplate::GOT_vehicle));
	bool const isHoveringVehicle = (mount.getGameObjectType() == SharedObjectTemplate::GOT_vehicle_hover || mount.getGameObjectType() == SharedObjectTemplate::GOT_vehicle_hover_ai);

	CrcString const *saddleHardpointName = !isVehicle ? static_cast<CrcString const*>(&cs_saddleHardpointName) : static_cast<CrcString const*>(&PersistentCrcString::empty);

	Object *const newSaddleObject = createAndAttachAppearance(mount, *saddleHardpointName, *saddleAppearanceName);

	if (newSaddleObject && isVehicle)
	{
		//-- Attach hover dynamics to the vehicle.  We do this here because we have the client datafile settings for it here.
		{
			newSaddleObject->setDynamics (new VehicleHoverDynamicsClient (newSaddleObject, 0.0f, 0.5f, clientDataFilename.getString ()));
			SkeletalAppearance2 * const skelApp = mount.getAppearance ()->asSkeletalAppearance2 ();
			if (skelApp)
				skelApp->setExtentDelegateObject (newSaddleObject);
		}

		//-- If the mount is a vehicle, share the mount object's customization variables with the saddle (visible vehicle portion) appearance.
		//   This is what enables a scripter and persistence on the server to affect the visuals of the vehicle (saddle) appearance.
		{
			CustomizationData *const mountCustomizationData = mount.fetchCustomizationData();
			if (mountCustomizationData)
			{
				Appearance *const saddleAppearance = newSaddleObject->getAppearance();
				if (saddleAppearance)
					saddleAppearance->setCustomizationData(mountCustomizationData);

				mountCustomizationData->release();
			}
		}
	
		if(!isHoveringVehicle)
		{
			// hide the saddle for non-hovering vehicles
			SkeletalAppearance2 *const appearance = const_cast<SkeletalAppearance2 *> (newSaddleObject->getAppearance() ? newSaddleObject->getAppearance()->asSkeletalAppearance2 () : 0);
			if (appearance)
			{
				appearance->setShowMesh (false);
				//appearance->setShowMesh (true); // ju_todo: debug
			}
		}
	}
	return newSaddleObject;
}

// ----------------------------------------------------------------------

Object *SaddleManager::getSaddleObjectFromMount(Object &mount)
{
	int const childCount = mount.getNumberOfChildObjects();
	for (int i = 0; i < childCount; ++i)
	{
		// Get the child object.
		Object *const childObject = mount.getChildObject(i);

		// Check if it's the appropriate hardpoint object.
		HardpointObject * const childHardpointObject = dynamic_cast<HardpointObject*>(childObject);
		if (childHardpointObject)
		{
			if (childHardpointObject->getHardpointName() == cs_saddleHardpointName)
			{
				// Found it.
				return childObject;
			}
		}
		else
		{
			// Check if it's the non-hardpoint saddle object.
			SaddleNonHardpointObject *const childSaddleObject = dynamic_cast<SaddleNonHardpointObject*>(childObject);
			if (childSaddleObject)
				return childObject;
		}
	}

	//-- Didn't find it.
	return NULL;
}

//----------------------------------------------------------------------

const Object *SaddleManager::getSaddleObjectFromMount(const Object &mount)
{
	return SaddleManager::getSaddleObjectFromMount (const_cast<Object &>(mount));
}

//----------------------------------------------------------------------

bool SaddleManager::isThereRoomForPassenger(Object const & mount)
{
	int const totalNumberOfPassengers = countTotalNumberOfPassengers(mount);
	int const capacity = getSaddleSeatingCapacity(mount);

	return totalNumberOfPassengers < capacity;
}

//----------------------------------------------------------------------

int SaddleManager::getSaddleSeatingCapacity(Object const & mount)
{
	ClientObject const * const clientObject = mount.asClientObject();
	TangibleObject const * const tangibleObject = (clientObject != 0) ? clientObject->asTangibleObject() : 0;
	CreatureObject const * const creatureObject = (tangibleObject != 0) ? tangibleObject->asCreatureObject() : 0;

	if (creatureObject == 0)
	{
		return 0;
	}

	CrcString const * const logicalSaddleName = getLogicalSaddleNameFromMount(*creatureObject);
	NOT_NULL(logicalSaddleName);
	return getSaddleSeatingCapacityForLogicalSaddleName(*logicalSaddleName);
}

//----------------------------------------------------------------------

int SaddleManager::countTotalNumberOfPassengers(Object const & mount)
{
	Object const * const saddleObject = SaddleManager::getSaddleObjectFromMount(mount);
	DEBUG_FATAL(!saddleObject, ("CreatureObject::countTotalNumberOfPassengers: SaddleManager failed to retrieve the saddle for creature mount id=[%s],template=[%s]", mount.getNetworkId().getValueString().c_str(), mount.getObjectTemplateName()));

	int totalNumberOfPassengers = 0;

	CreatureObject const * const creatureObjectMount = dynamic_cast<CreatureObject const *> (&mount);
	bool isNonHoveringVehicle = false;
	if(creatureObjectMount)
	{
		bool const isVehicle = GameObjectTypes::isTypeOf (creatureObjectMount->getGameObjectType (), static_cast<int>(SharedObjectTemplate::GOT_vehicle));
		isNonHoveringVehicle = (isVehicle && (creatureObjectMount->getGameObjectType () != SharedObjectTemplate::GOT_vehicle_hover));

	}

	//  Non Hovering Mount with no objects on the saddle. This means most likely a creature mount (AT-ST, AT-RT).
	//  MIND SWAP! - swap the pointers to point to the creature since he/she/it actually contains the passengers.
	bool useCreature = 	(isNonHoveringVehicle && saddleObject->getNumberOfChildObjects() == 0) ? true : false;
	
	Object const * const parentObject = useCreature ? &mount : saddleObject;

	int const childCount = parentObject->getNumberOfChildObjects();
	for (int i = 0; i < childCount; ++i)
	{
		Object const * const childObject = parentObject->getChildObject(i);

		// Check if it's the appropriate hardpoint object.
		HardpointObject const * const childHardpointObject = dynamic_cast<HardpointObject const *>(childObject);
		if (childHardpointObject != 0)
		{
			char const * const hardpointName = childHardpointObject->getHardpointName().getString();

			HardpointObject const * childHardpointObjectToTest = 0;

			if (strcmp(hardpointName, cs_driverHardpointName) == 0)
			{
				childHardpointObjectToTest = childHardpointObject;
			}
			else if (strstr(hardpointName, cs_passengerHardpointName) != 0)
			{
				childHardpointObjectToTest = childHardpointObject;
			}

			if (childHardpointObjectToTest != 0)
			{
				// technically there should ever be one child per object
				int const hardPointChildCount = childHardpointObjectToTest->getNumberOfChildObjects();

				for (int j = 0; j < hardPointChildCount; ++j)
				{
					Object const * const rider = childHardpointObjectToTest->getChildObject(j);
					ClientObject const * const clientObject = (rider != 0) ? rider->asClientObject() : 0;
					TangibleObject const * const tangibleObject = (clientObject != 0) ? clientObject->asTangibleObject() : 0;
					CreatureObject const * const creatureObject = (tangibleObject != 0) ? tangibleObject->asCreatureObject() : 0;

					DEBUG_WARNING(hardPointChildCount != 1, ("object on mount with multiple children: %s", rider->getObjectTemplateName()));

					if (creatureObject != 0)
					{
						++totalNumberOfPassengers;
					}
				}

				DEBUG_FATAL(hardPointChildCount != 1, ("CreatureObject::countTotalNumberOfPassengers: other than one child for creature mount id=[%s],template=[%s],count=[%d]", mount.getNetworkId().getValueString().c_str(), mount.getObjectTemplateName(), hardPointChildCount));

			}
		}
	}
	return totalNumberOfPassengers;
}

//----------------------------------------------------------------------

int SaddleManager::getRiderSeatIndex(Object const & mount, Object const & rider)
{
	Object const * const saddleObject = SaddleManager::getSaddleObjectFromMount(mount);
	DEBUG_FATAL(!saddleObject, ("CreatureObject::getRiderSeatIndex: SaddleManager failed to retrieve the saddle for creature mount id=[%s],template=[%s]", mount.getNetworkId().getValueString().c_str(), mount.getObjectTemplateName()));

	int const childCount = saddleObject->getNumberOfChildObjects();
	for (int i = 0; i < childCount; ++i)
	{
		Object const * const childObject = saddleObject->getChildObject(i);

		// Check if it's the appropriate hardpoint object.
		HardpointObject const * const childHardpointObject = dynamic_cast<HardpointObject const *>(childObject);
		if (childHardpointObject != 0)
		{
			char const * const hardpointName = childHardpointObject->getHardpointName().getString();

			if (strcmp(hardpointName, cs_passengerHardpointName) == 0)
			{
				// there should ever be one child per seat
				int const hardPointChildCount = childHardpointObject->getNumberOfChildObjects();
				DEBUG_FATAL(hardPointChildCount != 1, ("CreatureObject::getRiderSeatIndex: more than one child for creature mount id=[%s],template=[%s]", mount.getNetworkId().getValueString().c_str(), mount.getObjectTemplateName()));

				for (int j = 0; j < hardPointChildCount; ++j)
				{
					if (childHardpointObject->getChildObject(j) == &rider)
					{
						return 1;
					}
				}
			}
			else if (strstr(hardpointName, cs_passengerHardpointName) != 0)
			{
				// there should ever be one child per seat
				int const hardPointChildCount = childHardpointObject->getNumberOfChildObjects();
				DEBUG_FATAL(hardPointChildCount != 1, ("CreatureObject::getRiderSeatIndex: more than one child for creature mount id=[%s],template=[%s]", mount.getNetworkId().getValueString().c_str(), mount.getObjectTemplateName()));

				for (int j = 0; j < hardPointChildCount; ++j)
				{
					if (childHardpointObject->getChildObject(j) == &rider)
					{
						int seatIndex = 0;
						sscanf(hardpointName, "passenger_%d", &seatIndex);
						return seatIndex + 1;
					}
				}
			}
		}
	}

	return 1;
}

//----------------------------------------------------------------------

int SaddleManager::findFirstOpenSeat(Object const & mount)
{
	Object const * const saddleObject = SaddleManager::getSaddleObjectFromMount(mount);
	DEBUG_FATAL(!saddleObject, ("CreatureObject::countTotalNumberOfPassengers: SaddleManager failed to retrieve the saddle for creature mount id=[%s],template=[%s]", mount.getNetworkId().getValueString().c_str(), mount.getObjectTemplateName()));

	int const capacity = getSaddleSeatingCapacity(mount);

	typedef std::vector<bool> OccupiedSeats;
	OccupiedSeats occupiedSeats;

	occupiedSeats.reserve(capacity);
	occupiedSeats.resize(capacity, false);

	{
		int const childCount = saddleObject->getNumberOfChildObjects();
		for (int i = 0; i < childCount; ++i)
		{
			Object const * const childObject = saddleObject->getChildObject(i);

			// Check if it's the appropriate hardpoint object.
			HardpointObject const * const childHardpointObject = dynamic_cast<HardpointObject const *>(childObject);
			if (childHardpointObject != 0)
			{
				char const * const hardpointName = childHardpointObject->getHardpointName().getString();

				if (strcmp(hardpointName, cs_driverHardpointName) == 0)
				{
					if (!occupiedSeats.empty())
					{
						occupiedSeats[0] = true;
					}
				}
				else if (strstr(hardpointName, cs_passengerHardpointName) != 0)
				{
					int seatIndex = 0;
					sscanf(hardpointName, "passenger_%d", &seatIndex);

					if (seatIndex < capacity)
					{
						occupiedSeats[seatIndex] = true;
					}
				}
			}
		}
	}

	int firstOpenSeat = 0;

	{
		for (int i = 0; i < capacity; ++i)
		{
			if (!occupiedSeats[i])
			{
				firstOpenSeat = i;
				break;
			}
		}
	}

	return firstOpenSeat;
}

//----------------------------------------------------------------------

HardpointObject * SaddleManager::createRiderHardpointObjectAndAttachToSaddle(Object & mount)
{
	int totalNumberOfPassengers = countTotalNumberOfPassengers(mount);
	int const capacity = getSaddleSeatingCapacity(mount);

	bool const thereIsRoom = totalNumberOfPassengers < capacity;

	if (thereIsRoom)
	{
		Object * const saddleObject = SaddleManager::getSaddleObjectFromMount(mount);
		DEBUG_FATAL(!saddleObject, ("CreatureObject::getSaddleObjectFromMount(): SaddleManager failed to retrieve the saddle for creature mount id=[%s],template=[%s], aborting visuals side ofmount.", mount.getNetworkId().getValueString().c_str(), mount.getObjectTemplateName()));

		char hardpointName[256];

		if (totalNumberOfPassengers != 0)
		{
			int const firstOpenSeat = findFirstOpenSeat(mount);
			snprintf(hardpointName, 256, "%s_%d", cs_passengerHardpointName, firstOpenSeat);
		}
		else
		{
			strncpy(hardpointName, cs_driverHardpointName, sizeof(hardpointName) - 1);
		}
		
		HardpointObject * const riderHardpointObject = new HardpointObject(ConstCharCrcString(hardpointName));
		CreatureObject* creatureObjectMount = dynamic_cast<CreatureObject *> (&mount);
		bool isNonHoveringVehicle = false;
		if(creatureObjectMount)
		{
			bool const isVehicle = GameObjectTypes::isTypeOf (creatureObjectMount->getGameObjectType (), static_cast<int>(SharedObjectTemplate::GOT_vehicle));
			isNonHoveringVehicle = (isVehicle && (creatureObjectMount->getGameObjectType () != SharedObjectTemplate::GOT_vehicle_hover));

		}

		if(isNonHoveringVehicle)
		{
			// for nonhovering vehicles, do not attach them to the saddle, attach them to the animating sat
			riderHardpointObject->attachToObject_p(&mount,true);
		}
		else
		{
			riderHardpointObject->attachToObject_p((saddleObject != NULL) ? saddleObject : &mount, true);
		}

		return riderHardpointObject;
	}

	return 0;
}

// ======================================================================
