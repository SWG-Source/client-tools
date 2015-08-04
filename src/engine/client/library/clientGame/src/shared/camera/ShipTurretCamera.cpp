// ======================================================================
//
// ShipTurretCamera.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShipTurretCamera.h"

#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/DustAppearance.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipObjectAttachments.h"
#include "clientGame/ShipStation.h"
#include "clientGraphics/RenderWorld.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MessageQueue.h"
#include "sharedGame/SharedShipObjectTemplate.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/CellProperty.h"
#include "sharedTerrain/SpaceTerrainAppearanceTemplate.h"
#include "sharedTerrain/TerrainObject.h"

// ======================================================================

namespace ShipTurretCameraNamespace
{
	ConstCharCrcString const c_cameraHardpointName ("turretcamera1");


	Object const *findTurretMuzzleObject(Object const *target, Transform &cameraTransform)
	{
		if (target)
		{
			ClientObject const * const clientObject = target->asClientObject();
			if (clientObject)
			{
				CreatureObject const * const creatureObject = clientObject->asCreatureObject();
				if (creatureObject)
				{
					int weaponIndex = ShipStation::getWeaponIndexForGunnerStation(creatureObject->getShipStation());
					ShipObject const * const ship = ShipObject::getContainingShip(*creatureObject);
					if (ship)
					{
						Transform t;
						Object const * const turret = ship->getShipObjectAttachments().getFirstAttachedObjectForWeapon(weaponIndex, t);
						if (turret)
						{
							Object const &muzzle = ShipObject::getShipTurretMuzzle(*turret);
							IGNORE_RETURN(NON_NULL(muzzle.getAppearance())->findHardpoint(c_cameraHardpointName, cameraTransform));
							return &muzzle;
						}
					}
				}
			}
		}
		return 0;
	}

}

using namespace ShipTurretCameraNamespace;

// ======================================================================

ShipTurretCamera::ShipTurretCamera() :
	GameCamera(),
	m_queue(0),
	m_target(0),
	m_turretTarget(0),
	m_cameraTransform(),
	m_dustObject(0),
	m_zoomSettingList(new ZoomSettingList),
	m_zoomSetting(0),
	m_zoom(0.f),
	m_currentZoom(0.f)
{
	for (int i = 0; i < 20; ++i)
		m_zoomSettingList->push_back(10.0f*i);
}

// ----------------------------------------------------------------------

ShipTurretCamera::~ShipTurretCamera()
{
	m_queue = 0;
	m_dustObject = 0;
	delete m_zoomSettingList;
}

// ----------------------------------------------------------------------

void ShipTurretCamera::setActive(bool const active)
{
	if (active)
		m_turretTarget = findTurretMuzzleObject(m_target, m_cameraTransform);

	GameCamera::setActive(active);

	if (CuiPreferences::getActualUseModelessInterface()
		|| CuiManager::getPointerInputActive())
	{
		CuiManager::setPointerToggledOn(false);
	}
}

// ----------------------------------------------------------------------

float ShipTurretCamera::alter(float const elapsedTime)
{
	if (!isActive())
	{
		setDustEnabled(false);
		return AlterResult::cms_alterNextFrame;
	}

	if (!m_turretTarget)
	{
		m_turretTarget = findTurretMuzzleObject(m_target, m_cameraTransform);
		if (!m_turretTarget)
		{
			setDustEnabled(false);
			return AlterResult::cms_alterNextFrame;
		}
	}

	setDustEnabled(true);

	CellProperty * const cellProperty = m_turretTarget->getParentCell();
	setParentCell(cellProperty);

	CellProperty::setPortalTransitionsEnabled(false);
		Transform const turretTransform_p(m_turretTarget->getTransform_o2c());
		Transform transform_p(Transform::IF_none);
		transform_p.multiply(turretTransform_p, m_cameraTransform);
		setTransform_o2p(transform_p);
	CellProperty::setPortalTransitionsEnabled(true);

	NOT_NULL(m_queue);
	for (int i = 0; i < m_queue->getNumberOfMessages(); ++i)
	{
		int message = 0;
		float value = 0.f;
		m_queue->getMessage (i, &message, &value);

		switch (message)
		{
		case CM_cameraZoom:
		case CM_mouseWheel:
			{
				if (m_zoomSettingList)
				{
					if (value > 0)
						m_zoomSetting = std::max (m_zoomSetting - 1, 0);
					else
						m_zoomSetting = std::min (m_zoomSetting + 1, static_cast<int>(m_zoomSettingList->size()) - 1);
				}
			}
			break;

		default:
			break;
		}
	}

	//-- clamp values to the acceptable range
	if (m_zoomSettingList)
	{
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_zoomSetting, static_cast<int>(m_zoomSettingList->size()));
		m_zoom = clamp(0.f, (*m_zoomSettingList)[static_cast<size_t>(m_zoomSetting)], m_zoomSettingList->back());
	}

	m_currentZoom = linearInterpolate(m_currentZoom, m_zoom, 0.1f);

	move_o(Vector::unitY);
	pitch_o(PI_OVER_2 * 0.125f);

	Vector const cameraZ_p = Vector::negativeUnitZ * m_zoom;
	Vector const start_w = getPosition_w();
	Vector const end_w = start_w + rotate_o2w(cameraZ_p);

	CollisionInfo result;
	if (ClientWorld::collide(getParentCell(), start_w, end_w, CollideParameters::cms_default, result, ClientWorld::CF_allCamera))
	{
		const float lineDistance = start_w.magnitudeBetween(end_w);
		const float t = clamp (0.f, (start_w.magnitudeBetween(result.getPoint ()) / lineDistance) - (0.25f / lineDistance), 1.f);
		m_currentZoom = Vector::linearInterpolate(start_w, end_w, t).magnitudeBetween (start_w);
	}

	move_o(Vector::negativeUnitZ * m_currentZoom);

	//-- Chain back up to parent
	float alterResult = GameCamera::alter(elapsedTime);
	AlterResult::incorporateAlterResult(alterResult, AlterResult::cms_alterNextFrame);
	return alterResult;
}

// ----------------------------------------------------------------------

void ShipTurretCamera::setMessageQueue(MessageQueue const * const queue)
{
	m_queue = queue;
}

// ----------------------------------------------------------------------

void ShipTurretCamera::setTarget(Object const * const object)
{
	m_target = object;
}

// ----------------------------------------------------------------------

void ShipTurretCamera::setDustEnabled(bool const dustEnabled)
{
	if (dustEnabled)
	{
		//-- Create the dust if it doesn't exist
		if (!m_dustObject)
		{
			TerrainObject const * const terrainObject = TerrainObject::getConstInstance();
			if (terrainObject && terrainObject->getAppearance())
			{
				SpaceTerrainAppearanceTemplate const * const spaceTerrainAppearanceTemplate = dynamic_cast<SpaceTerrainAppearanceTemplate const *>(terrainObject->getAppearance()->getAppearanceTemplate());
				if (spaceTerrainAppearanceTemplate)
				{
					m_dustObject = new Object;
					m_dustObject->setAppearance(new DustAppearance(m_turretTarget, spaceTerrainAppearanceTemplate->getNumberOfDust(), spaceTerrainAppearanceTemplate->getDustRadius()));
					RenderWorld::addObjectNotifications(*m_dustObject);
					addChildObject_o(m_dustObject);
				}
			}
		}
	}
	else
	{
		//-- Destroy the dust if it exists
		if (m_dustObject)
		{
			m_dustObject->removeFromWorld();

			delete m_dustObject;
			m_dustObject = 0;
		}
	}
}

// ======================================================================
