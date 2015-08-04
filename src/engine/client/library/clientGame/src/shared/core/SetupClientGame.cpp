// ======================================================================
//
// SetupClientGame.cpp
// Portions Copyright 1999, Bootprint Entertainment, Inc.
// Portions Copyright 2000-2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/SetupClientGame.h"

#include "clientAudio/Sound2.h"
#include "clientGame/AiDebugStringManager.h"
#include "clientGame/AlarmManager.h"
#include "clientGame/AlignToHardpointActionTemplate.h"
#include "clientGame/ArcTargetActionTemplate.h"
#include "clientGame/AwayFromKeyBoardManager.h"
#include "clientGame/Bloom.h"
#include "clientGame/CellObject.h"
#include "clientGame/ChangePostureActionTemplate.h"
#include "clientGame/ChangeScaleActionTemplate.h"
#include "clientGame/ChatLogManager.h"
#include "clientGame/ClientAsteroidManager.h"
#include "clientGame/ClientBattlefieldMarkerObjectTemplate.h"
#include "clientGame/ClientBattlefieldMarkerOutlineObject.h"
#include "clientGame/ClientBattlefieldMarkerOutlineObjectNotification.h"
#include "clientGame/ClientBuffBuilderManager.h"
#include "clientGame/ClientBuffManager.h"
#include "clientGame/ClientBuildingObjectTemplate.h"
#include "clientGame/ClientCellObjectTemplate.h"
#include "clientGame/ClientCollisionProperty.h"
#include "clientGame/ClientConstructionContractObjectTemplate.h"
#include "clientGame/ClientCreatureObjectTemplate.h"
#include "clientGame/ClientDataFile.h"
#include "clientGame/ClientDataTemplateList.h"
#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectManager.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/ClientEventManager.h"
#include "clientGame/ClientExpertiseManager.h"
#include "clientGame/ClientObjectUsabilityManager.h"
#include "clientGame/ClientFactoryObjectTemplate.h"
#include "clientGame/ClientGameAppearanceEvents.h"
#include "clientGame/ClientGroupObjectTemplate.h"
#include "clientGame/ClientGuildObjectTemplate.h"
#include "clientGame/ClientHeadTracking.h"
#include "clientGame/ClientImageDesignerManager.h"
#include "clientGame/ClientIncubatorManager.h"
#include "clientGame/ClientInstallationObjectTemplate.h"
#include "clientGame/ClientIntangibleObjectTemplate.h"
#include "clientGame/ClientInteriorLayoutManager.h"
#include "clientGame/ClientManufactureSchematicObjectTemplate.h"
#include "clientGame/ClientMissionObjectTemplate.h"
#include "clientGame/ClientNoBuildNotification.h"
#include "clientGame/ClientObjectTemplate.h"
#include "clientGame/ClientObjectTerrainModificationNotification.h"
#include "clientGame/ClientObjectUsabilityManager.h"
#include "clientGame/ClientPathObject.h"
#include "clientGame/ClientPathObjectNotification.h"
#include "clientGame/ClientPlayerQuestObjectTemplate.h"
#include "clientGame/ClientPlayerObjectTemplate.h"
#include "clientGame/ClientResourceContainerObjectTemplate.h"
#include "clientGame/ClientSecureTradeManager.h"
#include "clientGame/ClientShipObjectTemplate.h"
#include "clientGame/ClientShipTargeting.h"
#include "clientGame/ClientStaticObjectTemplate.h"
#include "clientGame/ClientStructureFootprintObject.h"
#include "clientGame/ClientStructureFootprintObjectNotification.h"
#include "clientGame/ClientTangibleObjectTemplate.h"
#include "clientGame/ClientTextManager.h"
#include "clientGame/ClientUniverse.h"
#include "clientGame/ClientUniverseObjectTemplate.h"
#include "clientGame/ClientVehicleObjectTemplate.h"
#include "clientGame/ClientWaypointObjectTemplate.h"
#include "clientGame/ClientWeaponObjectTemplate.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CloneWeaponActionTemplate.h"
#include "clientGame/CockpitCamera.h"
#include "clientGame/CollisionCallbacks.h"
#include "clientGame/CombatEffectsManager.h"
#include "clientGame/CombatSpecialMoveVisualData.h"
#include "clientGame/CommunityManager.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/CreatureController.h"
#include "clientGame/CreatureInfo.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/CustomerServiceManager.h"
#include "clientGame/DroidProgramSizeManager.h"
#include "clientGame/DustAppearance.h"
#include "clientGame/DynamicDebrisObject.h"
#include "clientGame/FaceTargetActionTemplate.h"
#include "clientGame/FadingTextAppearance.h"
#include "clientGame/FireProjectileAction.h"
#include "clientGame/FireProjectileActionTemplate.h"
#include "clientGame/FireSetupActionTemplate.h"
#include "clientGame/FireTurretActionTemplate.h"
#include "clientGame/FormManagerClient.h"
#include "clientGame/Game.h"
#include "clientGame/GameAnimationMessageCallback.h"
#include "clientGame/GameNetworkConnection.h"
#include "clientGame/GamePlaybackScript.h"
#include "clientGame/GrenadeLobActionTemplate.h"
#include "clientGame/GroundCombatActionManager.h"
#include "clientGame/GroundScene.h"
#include "clientGame/HardpointTargetAction.h"
#include "clientGame/HardpointTargetActionTemplate.h"
#include "clientGame/HomingTargetActionTemplate.h"
#include "clientGame/HyperspaceIoWin.h"
#include "clientGame/InputActivityManager.h"
#include "clientGame/LightsaberAppearance.h"
#include "clientGame/LightsaberAppearanceTemplate.h"
#include "clientGame/LightsaberCollisionManager.h"
#include "clientGame/MatchMakingManager.h"
#include "clientGame/MissileManager.h"
#include "clientGame/NebulaManagerClient.h"
#include "clientGame/NonTrackingProjectile.h"
#include "clientGame/PlayParticleSystemAction.h"
#include "clientGame/PlayParticleSystemActionTemplate.h"
#include "clientGame/PlaySkeletalAnimationAction.h"
#include "clientGame/PlaySkeletalAnimationActionTemplate.h"
#include "clientGame/PlayerCreationManagerClient.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGame/PlayerMusicManager.h"
#include "clientGame/PlayerShipController.h"
#include "clientGame/PlayerShipTurretController.h"
#include "clientGame/PlotterManager.h"
#include "clientGame/ProjectileDynamics.h"
#include "clientGame/ProjectileManager.h"
#include "clientGame/PushCreatureActionTemplate.h"
#include "clientGame/RemoteCreatureController.h"
#include "clientGame/RemoteShipController.h"
#include "clientGame/RoleIconManager.h"
#include "clientGame/SaddleManager.h"
#include "clientGame/ShipComponentPowerEffectsManagerClient.h"
#include "clientGame/ShipController.h"
#include "clientGame/ShipHitEffectsManagerClient.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipObjectDestruction.h"
#include "clientGame/ShipProjectile.h"
#include "clientGame/ShipTargetAppearanceManager.h"
#include "clientGame/ShipWeaponGroupManager.h"
#include "clientGame/SlotCustomizationVariable.h"
#include "clientGame/SlotRule.h"
#include "clientGame/SlotRuleManager.h"
#include "clientGame/SpaceDeath.h"
#include "clientGame/SpacePreloadedAssetManager.h"
#include "clientGame/SpaceTargetBracketOverlay.h"
#include "clientGame/StartTrailsAction.h"
#include "clientGame/StartTrailsActionTemplate.h"
#include "clientGame/StopTrailsActionTemplate.h"
#include "clientGame/TangibleObject.h"
#include "clientGame/TemplateCommandMappingManager.h"
#include "clientGame/TemporaryAttachedObjectAction.h"
#include "clientGame/TemporaryAttachedObjectActionTemplate.h"
#include "clientGame/ThrowSwordAction.h"
#include "clientGame/ThrowSwordActionTemplate.h"
#include "clientGame/TrackingProjectile.h"
#include "clientGame/TurretObject.h"
#include "clientGame/UseLightningAction.h"
#include "clientGame/UseLightningActionTemplate.h"
#include "clientGame/WaitForBoltActionTemplate.h"
#include "clientGame/WanderController.h"
#include "clientGame/WeaponObject.h"
#include "clientGame/WorldSnapshot.h"
#include "clientGame/ZoneMapObject.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/PostProcessingEffectsManager.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientObject/DetailAppearance.h"
#include "clientObject/ForceFieldAppearance.h"
#include "clientObject/PortalBarrierAppearance.h"
#include "clientParticle/ParticleEmitter.h"
#include "clientParticle/SetupClientParticle.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientTerrain/CelestialObject.h"
#include "clientTerrain/ClientStaticRadialFloraManager.h"
#include "clientUserInterface/CuiTextManager.h"
#include "LocalizationManager.h"
#include "sharedCollision/SetupSharedCollision.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/ConfigSharedGame.h"
#include "sharedGame/SharedDraftSchematicObjectTemplate.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/SharedTerrainSurfaceObjectTemplate.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PortalProperty.h"
#include "sharedSkillSystem/ExpertiseManager.h"
#include "sharedSkillSystem/LevelManager.h"
#include "sharedTerrain/ProceduralTerrainAppearance.h"
#include "sharedTerrain/TerrainObject.h"

#include "UILowerString.h"
#include "UITextStyleManager.h"

//-------------------------------------------------------------------

namespace SetupClientGameNamespace
{
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool ms_installed;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void addToRenderWorld (Object& object)
	{
		RenderWorld::addObjectNotifications (object);
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void addToClientWorld (Object& object)
	{
		object.addNotification (ClientWorld::getIntangibleNotification ());
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void createFloraHookFunction (Object& object)
	{
		RenderWorld::addObjectNotifications (object);
		object.addNotification (ClientWorld::getTangibleFloraNotification ());
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const SharedObjectTemplateClientData* createClientDataFunction (const char* filename)
	{
		return ClientDataTemplateList::fetch (filename);
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Appearance* portalBarrierFactory (const VertexList& vertices, const VectorArgb& color)
	{
		return new PortalBarrierAppearance (vertices, color);
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Appearance* forceFieldFactory (const VertexList& vertices, const VectorArgb& color)
	{
		return new ForceFieldAppearance (vertices, color);
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Object* createTextAppearanceObject (const char* text, const VectorArgb& color, const float scale)
	{
		Object* const object = new Object;
		object->addNotification (ClientWorld::getIntangibleNotification ());
		RenderWorld::addObjectNotifications (*object);

		FadingTextAppearance* const appearance = new FadingTextAppearance (Unicode::narrowToWide (text), color, 1.0f, 0.f, scale, CuiTextManagerTextEnqueueInfo::TW_starwars);
		appearance->setFadeEnabled (false);
		object->setAppearance (appearance);

		return object; //lint !e429 // custodial pointer 'appearance' has not been freed or returned // no problem, it's owned by object.
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void drawCircle (const Vector &center, const Vector &normal, real radius, int segments, const VectorArgb &argb)
	{
		UNREF(center);
		UNREF(normal);
		UNREF(radius);
		UNREF(segments);
		UNREF(argb);

#ifdef _DEBUG
		const Camera *camera = Game::getCamera();

		if (camera != NULL)
		{
			CircleDebugPrimitive * const debugPrimitive = new CircleDebugPrimitive(CircleDebugPrimitive::S_z, Transform::identity, center, radius, segments);
			
			debugPrimitive->setColor(argb);
			camera->addDebugPrimitive(debugPrimitive);
		}
#endif // _DEBUG
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void drawBox (AxialBox const & box, const VectorArgb &argb)
	{
		UNREF(box);
		UNREF(argb);

#ifdef _DEBUG
		const Camera *camera = Game::getCamera();

		if (camera != NULL)
		{
			BoxDebugPrimitive * const debugPrimitive = new BoxDebugPrimitive(BoxDebugPrimitive::S_z, Transform::identity, box);
			
			debugPrimitive->setColor(argb);
			camera->addDebugPrimitive(debugPrimitive);
		}
#endif // _DEBUG
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void drawCylinder (const Vector &base, float radius, float height, int tessTheta, int tessRho, int tessZ, int nSpokes, const VectorArgb &argb)
	{
		UNREF(base);
		UNREF(radius);
		UNREF(height);
		UNREF(tessTheta);
		UNREF(tessRho);
		UNREF(tessZ);
		UNREF(nSpokes);
		UNREF(argb);

#ifdef _DEBUG
		const Camera *camera = Game::getCamera();

		if (camera != NULL)
		{
			CylinderDebugPrimitive * const debugPrimitive = new CylinderDebugPrimitive(CylinderDebugPrimitive::S_z, Transform::identity, base, radius, height, tessTheta, tessRho, tessZ, nSpokes);
			
			debugPrimitive->setColor(argb);
			camera->addDebugPrimitive(debugPrimitive);
		}
#endif // _DEBUG
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Object *beginCreateClientCellObject(int index)
	{
		// create the object
		const ObjectTemplate *objectTemplate = ObjectTemplateList::fetch("object/cell/shared_cell.iff");
		Object *object = objectTemplate->createObject();
		objectTemplate->releaseReference();
		objectTemplate = NULL;

		// set its cell
		safe_cast<CellObject *>(object)->setCell(index);

		return object;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void endCreateClientCellObject(Object *object)
	{
		// tell the cell it's done depersisting
		safe_cast<CellObject*>(object)->endBaselines();
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void renderDetailLevel (const Object* object)
	{
		const Camera* const camera = Game::getCamera ();
		if (!camera)
			return;

		const Appearance* const appearance = object->getAppearance ();
		if (!appearance || !appearance->getRenderedThisFrame())
			return;

		const SkeletalAppearance2* const skeletalAppearance = dynamic_cast<const SkeletalAppearance2*> (appearance);
		const DetailAppearance* const detailAppearance = dynamic_cast<const DetailAppearance*> (appearance);
		const BoxExtent* const boxExtent = dynamic_cast<const BoxExtent*> (appearance->getExtent ());

		if (boxExtent && (skeletalAppearance || detailAppearance))
		{
			const Vector position_w = object->getPosition_w () + Vector::unitY * (-boxExtent->getBottom () + boxExtent->getTop ());
			const int detailLevel = skeletalAppearance ? skeletalAppearance->getDisplayLodIndex () : ((detailAppearance->getDetailLevelCount () - 1) - detailAppearance->getCurrentDetailLevel ()); //lint !e613 // possible use of null pointer detailAppearance // no, see logic.

			CuiTextManager::TextEnqueueInfo info;
			if (camera->projectInWorldSpace (position_w, &info.screenVect.x, &info.screenVect.y, &info.screenVect.z))
			{
				char buffer [5];
				sprintf (buffer, "%i", detailLevel);

				info.worldDistance = camera->getPosition_w ().magnitudeBetween (position_w);
				info.backgroundOpacity = 0.0f;
				info.textColor         = UIColor::white;
				CuiTextManager::enqueueText (Unicode::narrowToWide (buffer), info);
			}
		}
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool isPlayerHouseHook(Object const *object)
	{
		if (object && object->getPortalProperty())
		{
			ClientObject const * const clientObject = object->asClientObject();
			if (clientObject)
			{
				if (clientObject->asShipObject())
					return true;

				ObjectTemplate const * const objTemplate = clientObject->getObjectTemplate();
				if (objTemplate != 0)
				{
					if (objTemplate->derivesFrom("object/building/base/shared_base_player_building.iff"))
					{
						return true;
					}

					if (objTemplate->derivesFrom("object/building/faction_perk/base/shared_factional_building_base.iff"))
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef _DEBUG
	void renderHardpoint(Transform const & transform_h2w, bool renderName, char const * name)
	{
		Camera const & camera = ShaderPrimitiveSorter::getCurrentCamera();
		camera.addDebugPrimitive(new FrameDebugPrimitive(FrameDebugPrimitive::S_z, transform_h2w, 1.f));

		if (renderName)
		{
			Vector const position_w = transform_h2w.getPosition_p();

			CuiTextManager::TextEnqueueInfo info;
			if (camera.projectInWorldSpace(position_w, &info.screenVect.x, &info.screenVect.y, &info.screenVect.z))
			{
				info.worldDistance = camera.getPosition_w().magnitudeBetween(position_w);
				info.backgroundOpacity = 0.0f;
				info.textColor = UIColor::white;
				CuiTextManager::enqueueText(Unicode::narrowToWide(name), info);
			}
		}
	}
#endif

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace SetupClientGameNamespace;

//-------------------------------------------------------------------

SetupClientGame::Data::Data () :
	m_viewer(false),
	m_disableLazyInteriorLayoutCreation(false)
{
}

//-------------------------------------------------------------------

void SetupClientGame::setupGameData (Data &data)
{
	data.m_viewer = false;
	data.m_disableLazyInteriorLayoutCreation = false;
}

//-------------------------------------------------------------------

void SetupClientGame::setupToolData (Data &data)
{
	data.m_viewer = true;
	data.m_disableLazyInteriorLayoutCreation = false;
}

//-------------------------------------------------------------------

void SetupClientGame::install (const Data &data)
{
	InstallTimer const installTimer("SetupClientGame::install");

	ExitChain::add(SetupClientGame::remove, "SetupClientGame::remove");

	//-- core
	UITextStyleManager::install(ConfigSharedGame::getFontLocale());

	ConfigClientGame::install ();

	LocalizationManager::debugDisplayStrings(ConfigClientGame::getDebugStringIds());

	ClientObject::install (data.m_viewer);

	PortalProperty::install(beginCreateClientCellObject, endCreateClientCellObject);

	{
		SetupSharedCollision::Data scData;

		scData.installExtents = true;
		scData.installCollisionWorld = true;
		scData.playEffect = &ClientEffectManager::playClientEffect;
		scData.isPlayerHouse = &isPlayerHouseHook;
		scData.serverSide = false;

		SetupSharedCollision::install(scData);
	}

	CellProperty::setAddToRenderWorldHook(addToRenderWorld);
	CellProperty::setPortalBarrierFactory(portalBarrierFactory);
	CellProperty::setForceFieldFactory(forceFieldFactory);

	ClientObjectTemplate::install(true);
	ClientCellObjectTemplate::install();
	ClientCollisionProperty::install();

	//-- Appearance
#ifdef _DEBUG
	Appearance::setRenderHardpointFunction(renderHardpoint);
#endif

	ClientGameAppearanceEvents::install ();
	LightsaberAppearanceTemplate::install ();
	LightsaberAppearance::install ();

	LightsaberCollisionManager::install ();

	PostProcessingEffectsManager::install();
	Bloom::install ();

#ifdef _DEBUG
	CreatureInfo::install();
#endif

	if (!data.m_viewer)
	{
		//-- callbacks
		ProceduralTerrainAppearance::setCreateFloraHookFunction (createFloraHookFunction);
		SharedObjectTemplate::setCreateClientDataFunction (createClientDataFunction);
		CelestialObject::setCollideFunction (ClientWorld::collide, static_cast<int>(ClientWorld::CF_all));
		CelestialObject::setNotificationFunction (addToClientWorld);
		Sound2::setCreateTextAppearanceObjectFunction (createTextAppearanceObject);
		Sound2::setDrawCircleFunction(drawCircle);
		ParticleEmitter::setCreateTextAppearanceObjectFunction (createTextAppearanceObject);
		ParticleEmitter::setDrawBoxFunction(drawBox);
		ParticleEmitter::setDrawCylinderFunction(drawCylinder);
		ClientStaticRadialFloraManager::setRenderDetailLevelFunction (renderDetailLevel);
		GroundScene::setRenderDetailLevelFunction (renderDetailLevel);

		//-- appearance-related systems
		SaddleManager::install("datatables/mount/logical_saddle_name_map.iff", "datatables/mount/saddle_appearance_map.iff", "datatables/mount/rider_pose_map.iff");

		//-- objects
		TangibleObject::install ();
		CreatureObject::install();
		CreatureController::install ();
		RemoteCreatureController::install ();
		PlayerCreatureController::install ();
		GameAnimationMessageCallback::install ();
		ClientStructureFootprintObject::install ();
		ClientStructureFootprintObjectNotification::install ();
		TerrainObject::addTerrainChangedFunction (ClientStructureFootprintObjectNotification::terrainChanged);
		ClientBattlefieldMarkerOutlineObject::install ();
		ClientBattlefieldMarkerOutlineObjectNotification::install();
		TerrainObject::addTerrainChangedFunction (ClientBattlefieldMarkerOutlineObjectNotification::terrainChanged);
		ClientPathObject::install ();
		ClientPathObjectNotification::install();
		TerrainObject::addTerrainChangedFunction (ClientPathObjectNotification::terrainChanged);
		DynamicDebrisObject::install();
		WanderController::install ();
		ClientNoBuildNotification::install ();
		TurretObject::install ();
		PlayerShipController::install ();
		ShipController::install ();
		RemoteShipController::install ();
		PlayerShipTurretController::install ();
		ShipObject::install();
		ShipObjectDestruction::install();

		// install the object templates
		ClientBattlefieldMarkerObjectTemplate::install();
		ClientBuildingObjectTemplate::install();
		ClientObjectTerrainModificationNotification::install ();
		ClientConstructionContractObjectTemplate::install();
		ClientCreatureObjectTemplate::install();
		ClientFactoryObjectTemplate::install();
		ClientGroupObjectTemplate::install();
		ClientGuildObjectTemplate::install();
		ClientInstallationObjectTemplate::install();
		ClientMissionObjectTemplate::install();
		ClientPlayerQuestObjectTemplate::install();
		ClientPlayerObjectTemplate::install(true);
		ClientResourceContainerObjectTemplate::install();
		ClientShipObjectTemplate::install();
		ClientStaticObjectTemplate::install();
		ClientTangibleObjectTemplate::install(true);
		ClientUniverseObjectTemplate::install();
		ClientVehicleObjectTemplate::install();
		ClientWaypointObjectTemplate::install();
		ClientWeaponObjectTemplate::install();
		ClientIntangibleObjectTemplate::install(true);
		ClientManufactureSchematicObjectTemplate::install(true);
		SharedDraftSchematicObjectTemplate::install ();
		SharedTerrainSurfaceObjectTemplate::install ();

		ClientDataTemplateList::install ();
		ClientDataFile::install ();

		TrackingProjectile::install ();
		NonTrackingProjectile::install ();
		ShipProjectile::install();

		//-- PlaybackScript actions
		AlignToHardpointActionTemplate::install ();
		ArcTargetActionTemplate::install ();
		ChangePostureActionTemplate::install ();
		ChangeScaleActionTemplate::install ();
		CloneWeaponActionTemplate::install ();
		FaceTargetActionTemplate::install ();
		FireSetupActionTemplate::install ();
		WaitForBoltActionTemplate::install ();
		FireProjectileActionTemplate::install ();
		FireProjectileAction::install ();
		FireTurretActionTemplate::install ();
		GrenadeLobActionTemplate::install ();
		HardpointTargetActionTemplate::install ();
		HardpointTargetAction::install ();
		HomingTargetActionTemplate::install ();
		PlayParticleSystemAction::install ();
		PlayParticleSystemActionTemplate::install ();
		PlaySkeletalAnimationActionTemplate::install ();
		PlaySkeletalAnimationAction::install ();
		PushCreatureActionTemplate::install ();
		StartTrailsActionTemplate::install ();
		StopTrailsActionTemplate::install ();
		TemporaryAttachedObjectActionTemplate::install ();
		TemporaryAttachedObjectAction::install ();
		ThrowSwordActionTemplate::install ();
		ThrowSwordAction::install ();
		UseLightningActionTemplate::install ();
		UseLightningAction::install ();

		// This must be installed after the clientGame-defined PlaybackScript actions.
		GamePlaybackScript::install ();

		//-- ClientEffect system
		ClientEffectTemplateList::install();
		ClientEffectManager::install();
		ClientEventManager::install();
		PositionClientEffect::install();
		ObjectClientEffect::install();

		GroundCombatActionManager::install();

		ClientSecureTradeManager::install();
		PlayerMusicManager::install();
		MatchMakingManager::install();
		CustomerServiceManager::install();
		AlarmManager::install();
		ChatLogManager::install();
		ClientTextManager::install();
		CommunityManager::install();
		CombatSpecialMoveVisualData::install ("combat/special_move_visuals.iff");
		GameNetworkConnection::install();
		AwayFromKeyBoardManager::install();
		InputActivityManager::install();


		StartTrailsAction::install();
		ClientImageDesignerManager::install();
		ClientBuffBuilderManager::install();

		ClientIncubatorManager::install();

		//-- setup particle system client world hook
		SetupClientParticle::setAddNotificationFunction (addToClientWorld);

		//-- setup universe system (resource tree)
		ClientUniverse::install();
		ClientUniverse::setupResourceTree();

		GroundScene::install ();
		WorldSnapshot::install ();
		ClientInteriorLayoutManager::install(data.m_disableLazyInteriorLayoutCreation);

		PlayerCreationManagerClient::install();

		SlotCustomizationVariable::install ();
		SlotRule::install ();
		SlotRuleManager::install ("customization/slot_rule_manager.iff");
		NetworkHandler::install();

#if PRODUCTION == 0
		PlotterManager::install();
#endif // PRODUCTION == 0

		ProjectileDynamics::install();
		ProjectileManager::install();
		MissileManager::install();

		FormManagerClient::install();

		ClientAsteroidManager::install();
		ShipWeaponGroupManager::install();
		CollisionCallbacks::install();
		ShipTargetAppearanceManager::install();
		SpaceDeath::install();

		NebulaManagerClient::install();

		ClientShipTargeting::install();

#ifdef _DEBUG
		AiDebugStringManager::install();
#endif // _DEBUG

		DustAppearance::install();
		SpacePreloadedAssetManager::install();
		ShipComponentDescriptor::fetchSharedObjectTemplates();

		CockpitCamera::install();	

		SpaceTargetBracketOverlay::install();
		ZoneMapObject::install();
		HyperspaceIoWin::install();

		ShipHitEffectsManagerClient::install();
		ShipComponentPowerEffectsManagerClient::install();

		DroidProgramSizeManager::install();
		ClientHeadTracking::install();

		CombatEffectsManager::install();

		ClientBuffManager::install();
		RoleIconManager::install();
		TemplateCommandMappingManager::install();

		ClientObjectUsabilityManager::install();

		LevelManager::install();
		ExpertiseManager::install();
		ClientExpertiseManager::install();

		WeaponObject::install();
	}
}

// ----------------------------------------------------------------------

void SetupClientGame::remove (void)
{
	NetworkHandler::remove();
	PlayerCreationManagerClient::remove();
	ProceduralTerrainAppearance::setCreateFloraHookFunction (0);
	SharedObjectTemplate::setCreateClientDataFunction (0);
	CelestialObject::setCollideFunction (0, 0);
	CelestialObject::setNotificationFunction (0);
	Sound2::setCreateTextAppearanceObjectFunction (0);
	Sound2::setDrawCircleFunction (0);
	ParticleEmitter::setCreateTextAppearanceObjectFunction (0);
	ParticleEmitter::setDrawBoxFunction (0);
	ParticleEmitter::setDrawCylinderFunction (0);
	ClientStaticRadialFloraManager::setRenderDetailLevelFunction (0);
	GroundScene::setRenderDetailLevelFunction (0);
}

//-------------------------------------------------------------------

