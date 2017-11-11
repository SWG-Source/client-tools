//======================================================================
//
// SwgCuiGroundRadar_BlipPane.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiGroundRadar_BlipPane.h"
#include "swgClientUserInterface/SwgCuiGroundRadar_WaypointPane.h"

#include "clientGame/BuildingObject.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/GroupObject.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientObject/GameCamera.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiGameColorManager.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiUtils.h"
#include "clientUserInterface/CuiWidgetGroundRadar.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorMesh.h"
#include "sharedCollision/FloorTri.h"
#include "sharedFoundation/Clock.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/GroundZoneManager.h"
#include "sharedGame/PvpData.h"
#include "sharedGame/SharedBuildingObjectTemplate.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedGame/SharedWaypointObjectTemplate.h"
#include "sharedMath/Range.h"
#include "sharedObject/CellProperty.h"
#include "sharedTerrain/TerrainObject.h"

#include "UIButton.h"
#include "UICanvas.h"
#include "UIData.h"
#include "UIImage.h"
#include "UIManager.h"
#include "UIManager.h"
#include "UIText.h"

//======================================================================

namespace SwgCuiGroundRadarBlipPaneNamespace
{
	const float RADAR_VERTICAL_CLIPPING = 15.0f;
	
	const Unicode::String canvasName = Unicode::narrowToWide ("uicanvas_filtered_clamped:ui_radar_mask");

	UICanvas * mask_canvas = 0;
	
	typedef SwgCuiGroundRadar::BlipPane::LineSegmentVector LineVector;

	typedef stdmap<CachedNetworkId, LineVector>::fwd BuildingLineCache;
	BuildingLineCache s_lineCache;

	void getBuildingLines (const BuildingObject & building, SwgCuiGroundRadar::BlipPane::LineSegmentVector & lsv)
	{
		const CachedNetworkId id (const_cast<BuildingObject &>(building));
		BuildingLineCache::const_iterator it = s_lineCache.find (id);

		if (it != s_lineCache.end ())
		{
			lsv = (*it).second;
			return;
		}
		
		Appearance const * const app = building.getAppearance ();
		if (!app)
			return;

		CollisionProperty const * const collision = building.getCollisionProperty();
		if (!collision)
			return;

		const Floor * const floor = collision->getFloor();

		if (floor)
		{
			const FloorMesh * const floorMesh = floor->getFloorMesh ();

			if (floorMesh)
			{

//				floorMesh->clearMarks (0);

				const uint32 triCount = floorMesh->getTriCount ();

				for (uint32 i = 0; i < triCount; ++i)
				{
					const FloorTri & floorTri = floorMesh->getFloorTri (i);

					if (floorTri.isFallthrough ())
					{
						for (int j = 0; j < 3; ++j)
						{
							if (floorTri.isCrossable (j))
								continue;
							
//							floorTri.setEdgeMark (j, 1);

							const IndexedTri & indexedTri = floorMesh->getIndexedTri (i);
							
							const int a = indexedTri.getCornerIndex (j);
							const int b = indexedTri.getCornerIndex (j + 1);

							const Vector & va = floorMesh->getVertex (a);
							const Vector & vb = floorMesh->getVertex (b);

							lsv.push_back (UILine (UIFloatPoint (va.x, va.z), UIFloatPoint (vb.x, vb.z)));
						}
					}
				}
			}
		}
		
		if (lsv.empty ())
		{
			const BoxExtent * box = dynamic_cast<const BoxExtent *>(app->getExtent ());
			
			if (box)
			{
				const Vector bmin = box->getMin ();
				const Vector bmax = box->getMax ();
				
				const UIFloatPoint corners [4] =
				{
					UIFloatPoint (bmin.x, bmin.z),
						UIFloatPoint (bmin.x, bmax.z),
						UIFloatPoint (bmax.x, bmax.z),
						UIFloatPoint (bmax.x, bmin.z)
				};
				
				lsv.push_back (UILine (corners [0], corners [1]));
				lsv.push_back (UILine (corners [1], corners [2]));
				lsv.push_back (UILine (corners [2], corners [3]));
				lsv.push_back (UILine (corners [3], corners [0]));											
			}
		}

		s_lineCache.insert (std::make_pair (id, lsv));
	}

	typedef std::multimap<float,  ConstWatcher<ClientObject> > DistanceObjectMap;
	DistanceObjectMap s_cachedDistanceObjectMap;
	Range const s_updateRangeInterval(1.0, 3.0f);
	Timer s_distancObjectRefresh;

	typedef std::pair<UIRect, UIColor> RectColor;
	typedef std::pair<ConstWatcher<ClientObject>, RectColor> ObjectRectPair;
	typedef std::vector<ObjectRectPair> ObjectToScreenVector;
	ObjectToScreenVector s_objectsToScreen;

	void renderToCanvas(bool const draw, float scale, ClientObject const & object, UIWidget const & thisPage, CuiWidgetGroundRadar const & groundRadar, UICanvas & canvas, Vector const & rot_object_pos, UIImage & blip, UIColor const & color, bool isWaypoint, bool isBracket, bool /*isTargetGroup*/, bool isRing)
	{
		const UIColor oldColor (canvas.GetColor());
		canvas.SetColor (color);
		canvas.ModifyColor(blip.GetBackgroundTint());

		const float pixels_to_world_ratio = groundRadar.getPixelsToWorldRatio () * 3.0f;
		long blipWidth = 0L;
		
		if (isWaypoint)
			blipWidth = std::min (24L, std::max (16L, static_cast<long>(pixels_to_world_ratio * 4.0f)));
		else 
			blipWidth = std::min (12L, std::max (8L,  static_cast<long>(pixels_to_world_ratio * 2.0f)));

		//std::min (maxBlipWidth, std::max (6L, static_cast<long>(worldRadius * pixels_to_world_ratio)));
		
		//These "magic numbers" represent the difference between the width of the bracket/ring images and the
		//width of the standard blip image.  It is assumed, by the way, that these images are square
		if (isBracket)
			blipWidth += 2;
		if (isRing)
			blipWidth += 8;

		UIPoint blipPos(static_cast<long>(rot_object_pos.x) + (thisPage.GetWidth() - blipWidth) / 2, static_cast<long>(rot_object_pos.z) + (thisPage.GetHeight() - blipWidth) / 2);
		
		if (draw) 
		{
			blip.SetSize (UISize (blipWidth, blipWidth));
			canvas.Translate(blipPos.x,  blipPos.y);
			blip.Render(canvas);
			canvas.Translate(-blipPos.x, -blipPos.y);
			canvas.SetColor(oldColor);
		}

		if (!isBracket) 
		{
			// Add the object to the tooltip list.
			UIPoint const blipLocation = blip.GetLocation();
			UIScalar blipSize = static_cast<UIScalar>(static_cast<float>(blipWidth) * scale);
			blip.SetSize(UISize(blipSize, blipSize));
			blip.SetLocation(blipPos, true);
			RectColor const & rectColor = std::make_pair(blip.GetWorldRect(), color);
			s_objectsToScreen.push_back(std::make_pair(ConstWatcher<ClientObject>(&object), rectColor));
			
			blip.SetLocation(blipLocation);
		}
}
}

using namespace SwgCuiGroundRadarBlipPaneNamespace;

//----------------------------------------------------------------------

SwgCuiGroundRadar::BlipPane::BlipPane (const CuiWidgetGroundRadar & groundRadar, UIImage & blipImage, UIImage & blipImageCorpse, UIImage & blipImageWaypoint, UIImage & blipImageStructure, UIImage & bracket, UIImage & blipImageGroup, UIImage & blipImageRing, UIImage & blipImageEntrance, WaypointPane & waypointPane, bool conMode) :
UIWidget                 (),
m_groundRadar            (groundRadar),
m_blipImage              (blipImage),
m_blipImageCorpse        (blipImageCorpse),
m_blipImageWaypoint      (blipImageWaypoint),
m_blipImageStructure     (blipImageStructure),
m_blipImageGroup         (blipImageGroup),
m_blipImageRing          (blipImageRing),
m_blipImageTargetBracket (bracket),
m_blipImageEntrance      (blipImageEntrance),
m_waypointPane           (waypointPane),
m_cosAngle               (0.0f),
m_sinAngle               (0.0f),
m_conMode                (conMode)
{
	SetName ("BlipPane");

	m_blipImage.Attach              (0);
	m_blipImageCorpse.Attach        (0);
	m_blipImageWaypoint.Attach      (0);
	m_blipImageStructure.Attach     (0);
	m_blipImageGroup.Attach         (0);
	m_blipImageTargetBracket.Attach (0);
	m_blipImageRing.Attach          (0);
	m_blipImageEntrance.Attach      (0);
	m_waypointPane.Attach           (0);

	m_blipImage.SetVisible              (false);
	m_blipImageCorpse.SetVisible        (false);
	m_blipImageWaypoint.SetVisible      (false);
	m_blipImageStructure.SetVisible     (false);
	m_blipImageGroup.SetVisible         (false);
	m_blipImageTargetBracket.SetVisible (false);
	m_blipImageRing.SetVisible          (false);
	m_blipImageEntrance.SetVisible      (false);

	mask_canvas = UIManager::gUIManager ().GetCanvas (canvasName);
	NOT_NULL (mask_canvas);
}

//----------------------------------------------------------------------

SwgCuiGroundRadar::BlipPane::~BlipPane ()
{
	m_blipImage.Detach              (0);
	m_blipImageCorpse.Detach        (0);
	m_blipImageWaypoint.Detach      (0);
	m_blipImageStructure.Detach     (0);
	m_blipImageGroup.Detach         (0);
	m_blipImageTargetBracket.Detach (0);
	m_blipImageRing.Detach          (0);
	m_blipImageEntrance.Detach      (0);
	m_waypointPane.Detach           (0);
}

//----------------------------------------------------------------------

void SwgCuiGroundRadar::BlipPane::setConMode(bool conMode)
{
	m_conMode = conMode;
}

//----------------------------------------------------------------------

const bool SwgCuiGroundRadar::BlipPane::getConMode() const
{
	return m_conMode;
}

//----------------------------------------------------------------------

bool SwgCuiGroundRadar::BlipPane::convertToWidgetSpace(Vector const & radarCenter, float & x, float & y, bool abortOutOfRange) const
{	
	const float pixels_to_world_ratio = m_groundRadar.getPixelsToWorldRatio ();
	
	Vector trans_object_pos(x - radarCenter.x, 0.0f, y - radarCenter.z);

	// transform it into screen space
	
	const float distance_squared              = trans_object_pos.magnitudeSquared ();
	const float screen_distance_squared       = distance_squared * sqr (pixels_to_world_ratio);
	const float screen_distance_max_squared   = m_groundRadar.getRadarRangeSquared () * sqr (pixels_to_world_ratio) - sqr (static_cast<float>(GetWidth ())) * 0.05f;
	
	if (abortOutOfRange && screen_distance_squared >= screen_distance_max_squared )
	{
		x = trans_object_pos.x;
		y = trans_object_pos.z;
		return false;
	}
	else if (screen_distance_squared >= screen_distance_max_squared)
	{
		trans_object_pos.normalize();
		trans_object_pos *= (sqrtf(m_groundRadar.getRadarRangeSquared()) * 0.9f);
	}
	
	trans_object_pos.x *= pixels_to_world_ratio;
	trans_object_pos.z *= pixels_to_world_ratio;
	
	// y (screen) direction is reversed to match screen space conventions
	x = trans_object_pos.x * m_cosAngle - trans_object_pos.z * m_sinAngle;
	y = - trans_object_pos.x * m_sinAngle - trans_object_pos.z * m_cosAngle;

	return true;
}

//----------------------------------------------------------------------

void SwgCuiGroundRadar::BlipPane::renderBlip(ClientObject const & object, UICanvas & canvas, const Vector & worldPos, const float worldRadius, UIImage & blip, const UIColor & color, bool allowWaypointArrow, bool isWaypoint, bool isBracket, bool isTargetGroup, bool isRing) const
{
	Vector rot_object_pos (worldPos);

	UNREF(worldRadius);

	if (!convertToWidgetSpace (m_groundRadar.getRadarWorldCenter (), rot_object_pos.x, rot_object_pos.z, true))
	{
		if(ConfigClientUserInterface::getDrawArrowToActiveWaypointOnly())
		{
			if(!isWaypoint)
				return;
			const ClientWaypointObject       * waypoint = safe_cast<const ClientWaypointObject *>(&object);
			if(!waypoint->isWaypointActive())
				return;
		}

		if (CuiPreferences::getShowRadarNPCs()) 
		{
			if(isWaypoint || (allowWaypointArrow && ConfigClientUserInterface::getDrawRadarArrows()))
			{
				static const float ONE_OVER_2_PI = RECIP (PI_TIMES_2);
				
				const float distance_squared              = rot_object_pos.magnitudeSquared ();
				
				const WaypointPane::Info info ( color,
					(-atan2 (rot_object_pos.z, rot_object_pos.x) + PI_OVER_2 - m_groundRadar.getAngle ()) * ONE_OVER_2_PI,
					distance_squared - m_groundRadar.getRadarRangeSquared (), 
					isWaypoint,
					isBracket,
					isTargetGroup);
				
				m_waypointPane.addWaypoint (info);
				
				{
					Vector tooltipPos(worldPos);
					convertToWidgetSpace(m_groundRadar.getRadarWorldCenter(), tooltipPos.x, tooltipPos.z, false);
					renderToCanvas(false, 3.0f, object, *this, m_groundRadar, canvas, tooltipPos, m_blipImageTargetBracket, color, isWaypoint, isBracket, isTargetGroup, isRing);
				}
			}
		}
		else
		{
			m_waypointPane.clearWaypoints();
		}

		return;
	}
	
	renderToCanvas(true, 1.0f, object, *this, m_groundRadar, canvas, rot_object_pos, blip, color, isWaypoint, isBracket, isTargetGroup, isRing);
}

//----------------------------------------------------------------------

void SwgCuiGroundRadar::BlipPane::renderLineSegments (const UIPoint & objectPos, UICanvas & canvas, const LineSegmentVector & lsv, const UIColor & color) const 
{
	canvas.PushState ();
	canvas.ModifyColor (color);

	canvas.Translate (objectPos);
	VectorArgb argb (CuiUtils::convertToVectorArgb (color));

	UIFloatPoint fSize (GetSize ());

	if (fSize.x != 0.0 && fSize.y != 0.0f)
	{
		static LineSegmentVector lineVector;
		static LineSegmentVector uvVector;

		lineVector.clear ();
		uvVector.clear   ();
		lineVector.reserve (lsv.size ());
		uvVector.reserve   (lsv.size ());

		for (LineSegmentVector::const_iterator it = lsv.begin (); it != lsv.end (); ++it)
		{
			const UILine & segment = *it;

			UILine screenSegment (segment);

			convertToWidgetSpace (Vector::zero, screenSegment.p1.x, screenSegment.p1.y, false);
			convertToWidgetSpace (Vector::zero, screenSegment.p2.x, screenSegment.p2.y, false);

			UILine uvs;

			uvs.p1.x = (objectPos.x + screenSegment.p1.x) / fSize.x;
			uvs.p1.y = (objectPos.y + screenSegment.p1.y) / fSize.y;

			uvs.p2.x = (objectPos.x + screenSegment.p2.x) / fSize.x;
			uvs.p2.y = (objectPos.y + screenSegment.p2.y) / fSize.y;

			lineVector.push_back (screenSegment);
			uvVector.push_back   (uvs);
		}

		canvas.RenderLines (mask_canvas, lineVector.size (), &lineVector.front (), &uvVector.front ());
	}

	canvas.PopState ();
}

//----------------------------------------------------------------------

void SwgCuiGroundRadar::BlipPane::renderWorldObject (UICanvas & canvas, const ClientObject& clientObject, const NetworkId & lookAtTargetGroupId) const
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player)
		return;

	const int got = clientObject.getGameObjectType ();
	if (got == SharedObjectTemplate::GOT_misc_furniture || GameObjectTypes::isTypeOf (got, SharedObjectTemplate::GOT_building))
		return;

	const Tag objectType = clientObject.getObjectType ();
	if (objectType == SharedBuildingObjectTemplate::SharedBuildingObjectTemplate_tag)
		return;

	const TangibleObject * const tangible = clientObject.asTangibleObject ();
	const ClientWaypointObject       * waypoint = 0;
	
	if (!tangible && objectType == SharedWaypointObjectTemplate::SharedWaypointObjectTemplate_tag)
	{
		waypoint = safe_cast<const ClientWaypointObject *>(&clientObject);
	}

	if (!tangible && !waypoint)
		return;

	UIColor color;
	bool    colorFound = false;

	float maxRangeSquared = 0.0f;

	float const radarRange = m_groundRadar.getRadarRange();

	UIImage * blip = 0;

	CuiGameColorManager::Type type             = CuiGameColorManager::T_default;
	CuiGameColorManager::Type nonTargetType    = CuiGameColorManager::T_default;

	const CreatureObject * rider = 0;

	if (tangible)
	{
		colorFound = true;
		
		type = CuiGameColorManager::findTypeForObject (*tangible, false);

		// don't draw NPCs on the radar
		if ( ( type == CuiGameColorManager::T_npc ) && ( CuiPreferences::getShowRadarNPCs() == false ) )
		{
			return;
		}

		nonTargetType = type;
		if (CuiGameColorManager::isTargetType (nonTargetType))
			nonTargetType = CuiGameColorManager::findTypeForObject (*tangible, true);
		
		if (type != CuiGameColorManager::T_target && 
			type != CuiGameColorManager::T_corpse &&
			type != CuiGameColorManager::T_yourCorpse &&
			type != CuiGameColorManager::T_group &&
			type != CuiGameColorManager::T_groupLeader &&
			type != CuiGameColorManager::T_combatTarget &&
			type != CuiGameColorManager::T_intendedTarget &&
			got  != SharedObjectTemplate::GOT_creature_droid_probe)

		{
			// always show the main character on the radar
			if (tangible != player)
			{
				if (tangible->isInvulnerable () || !tangible->isTargettable ())
					return;
			}
		}

		if (ConfigClientUserInterface::getDontDrawNpcsOnRadar() && (type == CuiGameColorManager::T_npc))		
			return;		
		if (ConfigClientUserInterface::getDontDrawCorpsesOnRadar() && (type == CuiGameColorManager::T_corpse))
			return;
		if (ConfigClientUserInterface::getDontDrawObjectsOnRadar() && (type == CuiGameColorManager::T_default))
			return;
		
		if (nonTargetType == CuiGameColorManager::T_corpse || nonTargetType == CuiGameColorManager::T_yourCorpse)
		{
			blip = &m_blipImageCorpse;
		}
		
		if(CuiGameColorManager::objectHasOverrideColor(*tangible))
		{
			color = CuiGameColorManager::findColorForObject(*tangible, true, true);
		}
		else
		{
			color = CuiGameColorManager::getColorForType(nonTargetType);
		}
		float rangeMod = std::max (CuiGameColorManager::getRangeModForType (type), CuiGameColorManager::getRangeModForType (nonTargetType));
		if (got == SharedObjectTemplate::GOT_creature_droid_probe)
		{
			//-- test to see if the probe droid is owned by you
			rangeMod = 1000.0f;
		}
		
		const CreatureObject * const targetCreature = tangible->asCreatureObject ();
		if (targetCreature)
		{
			rider = targetCreature->getRiderDriverCreature ();
			if (rider)
			{
				CuiGameColorManager::Type riderType             = CuiGameColorManager::findTypeForObject (*rider, false);
				rangeMod = std::max (CuiGameColorManager::getRangeModForType (riderType), rangeMod);
			}
		}
		
		maxRangeSquared = sqr (rangeMod * radarRange * 1.5f);
	}
	else if (waypoint)
	{
		maxRangeSquared = 16384.0f;
		colorFound = true;
		const VectorArgb & colorArgb = waypoint->getColorArgb ();
		color = CuiUtils::convertColor (colorArgb);
	}

	if (!blip)
	{
		if (objectType != SharedCreatureObjectTemplate::SharedCreatureObjectTemplate_tag)
			blip = &m_blipImageStructure;
		else
		{
			if (lookAtTargetGroupId.isValid ())
			{
				const CreatureObject * const creature = clientObject.asCreatureObject ();
				if (creature && creature->getGroup () == lookAtTargetGroupId)
					blip = &m_blipImageGroup;
			}
			
			if (!blip)
				blip = &m_blipImage;
		}
	}

	NOT_NULL (blip);

	const Vector& object_pos = clientObject.getAppearanceSphereCenter_w ();

	const float distanceSquared = object_pos.magnitudeBetweenSquared (player->getPosition_w ());
	if (distanceSquared > maxRangeSquared)
		return;

	float sphereRadius = clientObject.getAppearanceSphereRadius ();
	if (nonTargetType == CuiGameColorManager::T_corpse)
		sphereRadius *= 2.0f;
	else if (nonTargetType == CuiGameColorManager::T_yourCorpse)
		sphereRadius *= 3.0f;

	if (CuiGameColorManager::isTargetType (type))
	{
		const UIColor & targetColor = CuiGameColorManager::getColorForType (type);
		renderBlip(clientObject, canvas, object_pos, sphereRadius, m_blipImageTargetBracket, targetColor, true, waypoint != 0, true, false, false);
	}
	else
	{
		if(getConMode())
		{
			if (tangible && tangible->isTargettable())
			{
				const CreatureObject* creatureObject = tangible->asCreatureObject();
				if (creatureObject && !creatureObject->isPlayer() && (type == CuiGameColorManager::T_attackable || type == CuiGameColorManager::T_canAttackYou))
				{
					UIColor difficultyColor = CuiCombatManager::getConColor (*creatureObject);
					renderBlip(clientObject, canvas, object_pos, sphereRadius, m_blipImageRing, difficultyColor, false, false, false, false, true);	
				}
			}
		}
	}
	
	bool drawWaypointArrow = ( waypoint != 0 ) || CuiPreferences::getShowGameObjectArrowsOnRadar();
	if(((type == CuiGameColorManager::T_npc) || (type == CuiGameColorManager::T_attackable)) && !ConfigClientUserInterface::getDrawNpcRadarArrows())
	{
		drawWaypointArrow = false;
	}

	renderBlip(clientObject, canvas, object_pos, sphereRadius, *blip, color, drawWaypointArrow, waypoint != 0, false, blip == &m_blipImageGroup, false);

	//-- render the rider
	if (rider)
		renderWorldObject (canvas, *rider, lookAtTargetGroupId);

#if 0
	if (building)
	{
		static LineSegmentVector lsv;
		getBuildingLines (*building, lsv);
		
		if (!lsv.empty ())
		{
			Vector pos (object_pos);
			
			convertToWidgetSpace (m_groundRadar.getRadarWorldCenter (), pos.x, pos.z, false);
			
			const UIPoint objectPos (
				static_cast<long>(pos.x) + (GetWidth ()) / 2,
				static_cast<long>(pos.z) + (GetHeight ()) / 2);
			
			renderLineSegments (objectPos, canvas, lsv, UIColor::red);
		}
	}
#endif

}

//----------------------------------------------------------------------

void SwgCuiGroundRadar::BlipPane::Render (UICanvas & canvas) const
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player)
		return;

	if (!Game::getScene ())
		return;

	NetworkId lookAtTargetGroupId;
	const Object * const lookAtObject             = player->getLookAtTarget ().getObject ();
	const ClientObject * const lookAtClientObject = (lookAtObject != NULL) ? lookAtObject->asClientObject() : NULL;
	const CreatureObject * const lookAtTarget     = (lookAtClientObject != NULL) ? lookAtClientObject->asCreatureObject() : NULL;

	const CachedNetworkId & playerGroupId = player->getGroup ();

	NetworkId playerGroupLeaderId;

	{
		const GroupObject * const group = safe_cast<const GroupObject *>(playerGroupId.getObject ());
		if (group)
			playerGroupLeaderId = group->getLeader ();
	}


	if (lookAtTarget)
	{
		lookAtTargetGroupId = lookAtTarget->getGroup ();
		if (lookAtTargetGroupId == playerGroupId)
			lookAtTargetGroupId = NetworkId::cms_invalid;
	}

	m_waypointPane.clearWaypoints ();

	m_cosAngle = cos (m_groundRadar.getAngle ());
	m_sinAngle = sin (m_groundRadar.getAngle ());
	
	const UIPoint & point     = GetWorldLocation ();
	const UISize & widgetSize = GetSize ();

	const UIPoint screenCenter (point.x + widgetSize.x / 2, point.y + widgetSize.y / 2);
	const UIRect widgetExtent  (point.x, point.y, point.x +  widgetSize.x, point.y + widgetSize.y);

	Graphics::setStaticShader (ShaderTemplateList::get2dVertexColorStaticShader ());
	Graphics::setObjectToWorldTransformAndScale (Transform::identity, Vector::xyz111);
	canvas.SetOpacity (0.8f);
	
	{
		const Vector & pos_w = m_groundRadar.getRadarWorldCenter ();
	
		// Cache the ranged searches.
		if (s_distancObjectRefresh.updateNoReset(Clock::frameTime()))
		{
			// Randomize the interval.
			resetSearchCache();

			// Clear old map.
			s_cachedDistanceObjectMap.clear();
		
			float const maxRange = m_groundRadar.getRadarRange() * 1.5f;
			static ClientWorld::ObjectVector cov;
			cov.clear ();
			ClientWorld::findObjectsInRange (pos_w, maxRange, cov);
			
			ClientWorld::ObjectVector::const_iterator end = cov.end ();
			for (ClientWorld::ObjectVector::const_iterator it = cov.begin (); it != end; ++it)
			{
				const ClientObject * const obj = dynamic_cast<ClientObject*> (*it);
				
				//Perform vertical distance check
				const Vector &obj_w = obj->getPosition_w();
				float distance = static_cast<float>(fabs(pos_w.y - obj_w.y));
				bool distanceCheck = true;
				if(obj && player && (player->getParentCell() != CellProperty::getWorldCellProperty()) && (obj->getParentCell() != player->getParentCell()) )
					distanceCheck = (distance < RADAR_VERTICAL_CLIPPING);

				if (obj && distanceCheck && obj->isActive () && obj->getAppearance ())
				{
					//don't render waypoints here
					if (obj->getObjectType () == SharedWaypointObjectTemplate::SharedWaypointObjectTemplate_tag)
						continue;
					
					const CreatureObject * const creature = obj->asCreatureObject();

					// should creature be shown on the radar?
					// always show the main character on the radar
					if (creature && (creature != player))
					{
						if (!creature->getVisibleOnMapAndRadar()
							|| (!creature->getCoverVisibility() && !creature->isPassiveRevealPlayerCharacter(Game::getPlayerNetworkId()))
						)
						{	
							continue;
						}
					}

					float distance = -pos_w.magnitudeBetweenSquared (obj->findPosition_w ());
					
					if (obj == player)
						distance += 10000.0f;
					else if (obj == lookAtTarget)
						distance += 9000.0f;
					else if (playerGroupLeaderId == obj->getNetworkId ())
						distance += 8500.0f;
					else if (creature && playerGroupId.isValid () && creature->getGroup () == playerGroupId)
						distance += 8000.0f;
					else if (creature && (creature->getPvpFlags() & PvpStatusFlags::CanAttackYou) != 0)
						distance += 7000.0f;
					else if (creature && creature->isPlayer ())
						distance += 6000.0f;
					//-- bias non creatures fully below the creatures
					else if (!creature)
						distance -= 16384.0f;

					s_cachedDistanceObjectMap.insert(std::make_pair(distance, ConstWatcher<ClientObject>(obj)));
				}
			}
		}

		// Clear the tooltip info map.
		s_objectsToScreen.clear();

		// Draw add waypoints.
		{
			const ClientWaypointObject::WaypointVector & wv = ClientWaypointObject::getActiveWaypoints ();
			
			for (ClientWaypointObject::WaypointVector::const_iterator it = wv.begin (); it != wv.end (); ++it)
			{
				ClientWaypointObject const * const waypoint = *it;
				if (waypoint)
				{
					const VectorArgb & colorArgb = waypoint->getColorArgb ();
					const UIColor & color = CuiUtils::convertColor (colorArgb);

					if (waypoint->getNetworkId().isValid()) 
					{
						if (waypoint->isEntrance())
							renderBlip(*waypoint, canvas, waypoint->getLocation(), 3.0f, m_blipImageEntrance, color, true, true, false, false, false);
						else
							renderBlip(*waypoint, canvas, waypoint->getLocation(), 3.0f, m_blipImageWaypoint, color, true, true, false, false, false);
					}
				}
			}
		}
		
		for (DistanceObjectMap::const_iterator it = s_cachedDistanceObjectMap.begin(); it != s_cachedDistanceObjectMap.end(); ++it)
		{
			ClientObject const * const obj = (*it).second;
			if (obj) 
			{
				renderWorldObject (canvas, *obj, lookAtTargetGroupId);
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiGroundRadar::BlipPane::resetSearchCache() const
{
	// Randomize the interval.
	s_distancObjectRefresh.setExpireTime(s_updateRangeInterval.random());
	s_distancObjectRefresh.reset();
}

//----------------------------------------------------------------------

ClientObject const * SwgCuiGroundRadar::BlipPane::getToolTipObjectAtPoint(UIPoint const & cursor, UIColor & color) const
{
	for (ObjectToScreenVector::const_iterator itTool = s_objectsToScreen.begin(); itTool != s_objectsToScreen.end(); ++itTool)
	{
		UIRect const & objRect = (*itTool).second.first;
		
		if (objRect.ContainsPoint(cursor))
		{
			if ((*itTool).first.getPointer()) 
			{
				color = (*itTool).second.second;
				return (*itTool).first.getPointer();
			}
		}
	}		

	return NULL;
}


//----------------------------------------------------------------------
