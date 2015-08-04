//======================================================================
//
// ShipComponentDataManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShipComponentDataManager.h"

#include "clientGame/ShipComponentDataArmor.h"
#include "clientGame/ShipComponentDataBooster.h"
#include "clientGame/ShipComponentDataBridge.h"
#include "clientGame/ShipComponentDataCapacitor.h"
#include "clientGame/ShipComponentDataCargoHold.h"
#include "clientGame/ShipComponentDataDroidInterface.h"
#include "clientGame/ShipComponentDataEngine.h"
#include "clientGame/ShipComponentDataHangar.h"
#include "clientGame/ShipComponentDataModification.h"
#include "clientGame/ShipComponentDataReactor.h"
#include "clientGame/ShipComponentDataShield.h"
#include "clientGame/ShipComponentDataTargetingStation.h"
#include "clientGame/ShipComponentDataWeapon.h"
#include "clientGame/TangibleObject.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedGame/ShipComponentData.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedGame/ShipComponentType.h"
#include "sharedObject/ObjectTemplate.h"

//======================================================================

namespace ShipComponentDataManagerNamespace
{
}

using namespace ShipComponentDataManagerNamespace;

//----------------------------------------------------------------------

ShipComponentData * ShipComponentDataManager::create (TangibleObject const & component)
{
	ShipComponentDescriptor const * const shipComponentDescriptor = 
		ShipComponentDescriptor::findShipComponentDescriptorByObjectTemplate (component.getObjectTemplate ()->getCrcName ().getCrc ());

	if (shipComponentDescriptor == NULL)
	{
		WARNING (true, ("ShipComponentDataManager [%s] is not a component", component.getNetworkId ().getValueString ().c_str ()));
		return NULL;
	}

	ShipComponentData * const shipComponent = create (*shipComponentDescriptor);

	if (!shipComponent)
		return NULL;

	shipComponent->readDataFromComponent (component);
	return shipComponent;
}

//----------------------------------------------------------------------

ShipComponentData * ShipComponentDataManager::create (ShipComponentDescriptor const & shipComponentDescriptor)
{
	ShipComponentData * shipComponent = NULL;

	switch (shipComponentDescriptor.getComponentType ())
	{
	case ShipComponentType::SCT_reactor:
			shipComponent = new ShipComponentDataReactor (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_engine:
			shipComponent = new ShipComponentDataEngine (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_shield:
			shipComponent = new ShipComponentDataShield (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_armor:
			shipComponent = new ShipComponentDataArmor (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_weapon:
			shipComponent = new ShipComponentDataWeapon (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_capacitor:
			shipComponent = new ShipComponentDataCapacitor (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_booster:
			shipComponent = new ShipComponentDataBooster (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_droid_interface:
			shipComponent = new ShipComponentDataDroidInterface (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_bridge:
			shipComponent = new ShipComponentDataBridge (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_hangar:
			shipComponent = new ShipComponentDataHangar (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_targeting_station:
			shipComponent = new ShipComponentDataTargetingStation (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_cargo_hold:
			shipComponent = new ShipComponentDataCargoHold(shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_modification:
			shipComponent = new ShipComponentDataModification(shipComponentDescriptor);
			break;

	default:
		WARNING (true, ("ShipComponentDataManager::create descriptor has type [%d] invalid", shipComponentDescriptor.getComponentType ()));
		return NULL;
	}

	return shipComponent;
}

//======================================================================
