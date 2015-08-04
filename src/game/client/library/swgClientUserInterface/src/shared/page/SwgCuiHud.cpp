//======================================================================
//
// SwgCuiHud.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiHud.h"

#include "clientGame/BuildingObject.h"
#include "clientGame/CellObject.h"
#include "clientGame/ClientCommandChecks.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientTextManager.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CockpitCamera.h"
#include "clientGame/GroundCombatActionManager.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureController.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/DraftSchematicInfo.h"
#include "clientGame/DraftSchematicManager.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/GroupObject.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGame/PlayerMoneyManagerClient.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipStation.h"
#include "clientGame/WeaponObject.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/Graphics.h"
#include "clientObject/GameCamera.h"
#include "clientObject/ReticleManager.h"
#include "clientObject/ReticleObject.h"
#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiConversationManager.h"
#include "clientUserInterface/CuiDragInfo.h"
#include "clientUserInterface/CuiDragManager.h"
#include "clientUserInterface/CuiGameColorManager.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiObjectTextManager.h"
#include "clientUserInterface/CuiPersistentMessageManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiRadialMenuManager.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/TransformAnimationResolver.h"
#include "sharedCollision/BaseExtent.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/Production.h"
#include "sharedFoundation/Timer.h"
#include "sharedGame/CombatDataTable.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/PvpData.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/CurrentUserOptionManager.h"
#include "sharedUtility/LocalMachineOptionManager.h"
#include "sharedUtility/DataTableManager.h"
#include "sharedUtility/DataTable.h"
#include "swgClientUserInterface/ConfigSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiChatWindow.h"
#include "swgClientUserInterface/SwgCuiChatWindow_Tab.h"
#include "swgClientUserInterface/SwgCuiG15Lcd.h"
#include "swgClientUserInterface/SwgCuiHudAction.h"
#include "swgClientUserInterface/SwgCuiHudWindowManager.h"
#include "swgClientUserInterface/SwgCuiInventory.h"
#include "swgClientUserInterface/SwgCuiToolbar.h"
#include "swgClientUserInterface/SwgCuiWebBrowserManager.h"
#include "swgSharedUtility/Postures.h"
#include "swgSharedUtility/States.def"

#include "UICursor.h"
#include "UIData.h"
#include "UIIMEManager.h"
#include "UIImage.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UITextStyle.h"
#include "UITextbox.h"
#include "UIUtils.h"

#include "EZ_LCD.h"

#include <list>
#include <map>
#include <set>

//lint -esym(641,GameControllerMessage) //convert enum to int
//lint -esym(641,ReticleType) //convert enum to int

//-----------------------------------------------------------------
namespace SwgCuiHudNamespace
{
	//-----------------------------------------------------------------

	inline void testObject (Object * object, Object *& minimumObject, real & minimumDistanceSquared, const Vector & worldStart, const Vector & worldEnd )
	{
		if (object == 0 || object->getAppearance () == 0)
			return;

		const Vector objectStart = object->rotateTranslate_w2o (worldStart);
		const Vector objectEnd   = object->rotateTranslate_w2o (worldEnd);

		CollisionInfo info;
		info.setPoint(objectEnd);

		if (object->getAppearance ()->collide (objectStart, objectEnd, CollideParameters::cms_default, info))
		{
			const real dist    = info.getPoint().magnitudeBetweenSquared (objectStart);

			if (dist < minimumDistanceSquared)
			{
				minimumDistanceSquared = dist;
				minimumObject          = object;
			}
		}
	}
	
	//----------------------------------------------------------------------
	
	enum FindObjectPassType
	{
		FOPT_POLY,
		FOPT_BOUNDING,
	};


	Object * testFindObject(Object * obj)
	{
		if(obj == 0)
		{
			return 0;
		}

		//-- select the root parent of an object hierarchy unless the selected object is a mounted player.
		bool useRootParent = true;

		ClientObject const * const clientObj = obj->asClientObject();
		if (clientObj)
		{
			CreatureObject const * const creature = clientObj->asCreatureObject();
			if (creature)
			{
				if (creature->getState(States::RidingMount))
					useRootParent = false;
				else
				{
					int const shipStation = creature->getShipStation();
					if (shipStation == ShipStation::ShipStation_Pilot || shipStation == ShipStation::ShipStation_Operations)
					{
						Object const * const containedByObject = ContainerInterface::getContainedByObject(*creature);
						if (containedByObject && containedByObject->asClientObject() && !containedByObject->asClientObject()->asShipObject())
							useRootParent = false;
					}
				}
			}
		}

		if (useRootParent)
			obj = obj->getRootParent ();		

		//-- don't target appearanceless objects
		if (obj && obj->getAppearance () == 0)
		{
			return 0;
		}

		const ClientObject * const clientObject = obj ? obj->asClientObject () : 0;

#if PRODUCTION == 0
		const bool allowTargetAnything = CuiPreferences::getAllowTargetAnything ();
		//-- don't target buildings and shit
		if (!allowTargetAnything)
#endif
		{
			//-- we are looking at an untargettable object
			if (!clientObject)
			{
				return 0;
			}
			else
			{
				const int got = clientObject->getGameObjectType () ;
				if (!clientObject->isTargettable () || GameObjectTypes::isTypeOf (got, SharedObjectTemplate::GOT_building))
				{
					return 0;
				}
				else
				{
					const TangibleObject * const tangible = clientObject->asTangibleObject ();
					
					if (!clientObject->getNetworkId ().isValid () && tangible && !tangible->canSitOn ())
					{
						return 0;
					}
				}
			}
		}
		return obj;
	}

	//----------------------------------------------------------------------

	Object * findObjectByPolygon(Camera const & camera, Vector const & worldStart, Vector const & worldEnd, const Object & self)
	{
		CellProperty const * const cameraCell = camera.getParentCell();
		
		CollisionInfo firstCinfo;

		Object * pickedObject = 0;

		PROFILER_AUTO_BLOCK_DEFINE("mesh test");

		uint16 firstCollisionFlags = ClientWorld::CF_terrain
									| ClientWorld::CF_terrainFlora
									| ClientWorld::CF_tangible
									| ClientWorld::CF_tangibleNotTargetable
									| ClientWorld::CF_tangibleFlora
									| ClientWorld::CF_interiorObjects
									| ClientWorld::CF_interiorGeometry
									| ClientWorld::CF_skeletal
									| ClientWorld::CF_childObjects;

		bool const result = ClientWorld::collide(cameraCell, worldStart, worldEnd, CollideParameters::cms_default, firstCinfo, firstCollisionFlags, &self);

		TerrainObject const * const terrainObject = TerrainObject::getInstance();

		if (result)
		{
			pickedObject = const_cast<Object *>(firstCinfo.getObject());
			if (static_cast<const Object *>(pickedObject) == terrainObject)
			{
				pickedObject = 0;
			}
		}

		return testFindObject(pickedObject);
	}

	//----------------------------------------------------------------------

	bool hitReticleExcludeStealthCreatureFunction(Object const *o)
	{
		CreatureObject const * const co = CreatureObject::asCreatureObject(o);
		if (co && !co->getCoverVisibility() && !co->isPassiveRevealPlayerCharacter(Game::getPlayerNetworkId()))
			return true;

		return false;
	}

	//----------------------------------------------------------------------

	bool hitReticle(Camera const & camera, Vector const & worldStart, Vector const & worldEnd, const Object & self, Vector &outHit, CellProperty **outCell)
	{
		CellProperty const * const cameraCell = camera.getParentCell();
		
		CollisionInfo firstCinfo;

		PROFILER_AUTO_BLOCK_DEFINE("hit terrain test");

		uint16 firstCollisionFlags = ClientWorld::CF_terrain
									| ClientWorld::CF_terrainFlora
									| ClientWorld::CF_tangible
									| ClientWorld::CF_tangibleNotTargetable
									| ClientWorld::CF_tangibleFlora
									| ClientWorld::CF_interiorObjects
									| ClientWorld::CF_interiorGeometry
									| ClientWorld::CF_skeletal
									| ClientWorld::CF_childObjects;

		bool const result = ClientWorld::collide(cameraCell, worldStart, worldEnd, CollideParameters::cms_default, firstCinfo, firstCollisionFlags, &self, hitReticleExcludeStealthCreatureFunction);

		if(result)
		{		
			Object *pickedObject = const_cast<Object *>(firstCinfo.getObject());
			if(pickedObject)
				(*outCell) = pickedObject->getParentCell();
			
			outHit = firstCinfo.getPoint();			
		}
		return result;
	}

	//----------------------------------------------------------------------

	void findObjectsByExtent(Camera const & camera, Vector const & worldStart, Vector const & worldEnd, const Object & self, ClientWorld::CollisionInfoVector & finalResults)
	{
		CellProperty const * const cameraCell = camera.getParentCell();

		CollisionInfo firstCinfo;

		PROFILER_AUTO_BLOCK_DEFINE("box test fallback");

		uint16 const secondCollisionFlags = ClientWorld::CF_terrain
											| ClientWorld::CF_tangible
											| ClientWorld::CF_skeletal
											| ClientWorld::CF_extentsOnly;

		ClientWorld::CollisionInfoVector results;
		bool const collided = ClientWorld::collide(cameraCell, worldStart, worldEnd, CollideParameters::cms_default, results, secondCollisionFlags, &self);

		if (collided)
		{
			TerrainObject * const terrainObject = TerrainObject::getInstance();

			ClientWorld::CollisionInfoVector::iterator ii = results.begin();
			ClientWorld::CollisionInfoVector::iterator iiEnd = results.end();

			for (; ii != iiEnd; ++ii)
			{
				CollisionInfo & collisionInfo = *ii;
				Object * const objectToTest = const_cast<Object*>(collisionInfo.getObject());

				NOT_NULL(objectToTest);

				if (objectToTest == terrainObject)
				{
					break;
				}

				Object * result = testFindObject(objectToTest);

				if (result != 0)
				{
					finalResults.push_back(collisionInfo);
				}
			}
		}
	}

	//----------------------------------------------------------------------

	typedef Watcher<Object> ObjectWatcher;
	typedef std::vector<ObjectWatcher> ObjectVector;

	void findAllTargettableObjects(Camera const & camera, Vector const & worldStart, Vector const & worldEndConst, const Object & self, ObjectVector & orderedTargets)
	{
		Object * polygonTarget = findObjectByPolygon(camera, worldStart, worldEndConst, self);
		bool allowTargetAnything = CuiPreferences::getAllowTargetAnything();
		if (polygonTarget != 0)
		{
			ClientObject * const clientPolygonObject = polygonTarget->asClientObject();
			if ((clientPolygonObject && clientPolygonObject->isTargettable()) || allowTargetAnything)
			{
				ObjectWatcher watcher(polygonTarget);
				orderedTargets.push_back(watcher);
			}
		}

		ClientWorld::CollisionInfoVector results;
		findObjectsByExtent(camera, worldStart, worldEndConst, self, results);

		ClientWorld::CollisionInfoVector::iterator ii = results.begin();
		ClientWorld::CollisionInfoVector::iterator iiEnd = results.end();

		for (; ii != iiEnd; ++ii)
		{
			CollisionInfo & collisionInfo = *ii;
			Object * const object = const_cast<Object*>(collisionInfo.getObject());
			NOT_NULL(object);

            ClientObject * const clientObject = object->asClientObject();
			
			if ((object != polygonTarget && clientObject && clientObject->isTargettable()) || allowTargetAnything)
			{
				ObjectWatcher watcher(object);
				orderedTargets.push_back(watcher);
			}
		}
	}

	//----------------------------------------------------------------------

	bool computeScreenRect (const Camera & camera, const TerrainObject & terrain, const Object & foundObject, UIRect & rect)
	{
		if (!foundObject.getAppearance ())
			return false;
		
		BoxExtent box;
		CuiObjectTextManager::computeBestFitBoxExtent (foundObject, box);
	
		const Transform & transform_o2w = foundObject.getTransform_o2w ();
		
		Vector vs [8];
		box.getCornerVectors (vs);

		Vector minScreenVect (100000.0f, 1000000.0f, 0.0f);
		Vector maxScreenVect;
		
		bool found = false;

		const bool isOnTerrain = !Game::isSpace() && foundObject.isInWorldCell ();

		for (int i = 0; i < 8; ++i)
		{
			Vector tmpScreenVect;
			
			Vector testVector (transform_o2w.rotateTranslate_l2p (vs [i]));
			
			//-- don't reveal extents for objects imbedded in terrain
			if (isOnTerrain)
			{
				float height = 0.0f;
				if (terrain.getHeight (testVector, height))
					testVector.y = std::max (testVector.y, height);
			}

			float projectedX = 0.0f;
			float projectedY = 0.0f;
			bool projectedOk = false;
			
			if (camera.projectInWorldSpace (testVector, tmpScreenVect.x, tmpScreenVect.y, 0, projectedX, projectedY, projectedOk))
			{
				minScreenVect.x = std::min (tmpScreenVect.x, minScreenVect.x);
				minScreenVect.y = std::min (tmpScreenVect.y, minScreenVect.y);
				maxScreenVect.x = std::max (tmpScreenVect.x, maxScreenVect.x);
				maxScreenVect.y = std::max (tmpScreenVect.y, maxScreenVect.y);
				found = true;
			}

			//-- if the projection failed, take a look at the raw projected x and y values to determine
			//-- where on the screen they should fall

			else if (projectedOk)
			{
				if (projectedX >= -1.0f && projectedX <= 1.0f)
				{
					minScreenVect.x = std::min (tmpScreenVect.x, minScreenVect.x);
					maxScreenVect.x = std::max (tmpScreenVect.x, maxScreenVect.x);
				}
				else if (projectedX < -1.0f)
					minScreenVect.x = -10000.0f;
				else if (projectedX > 1.0f)
					maxScreenVect.x = 10000.0f;

				if (projectedY >= -1.0f && projectedY <= 1.0f)
				{
					minScreenVect.y = std::min (tmpScreenVect.y, minScreenVect.y);
					maxScreenVect.y = std::max (tmpScreenVect.y, maxScreenVect.y);
				}
				else if (projectedY < -1.0f)
					maxScreenVect.y = 10000.0f;
				else if (projectedY > 1.0f)
					minScreenVect.y = -10000.0f;

				found = true;
			}
		}
		
		if (!found)
			return false;

		rect.left   = static_cast<long>(minScreenVect.x);
		rect.top    = static_cast<long>(minScreenVect.y);
		rect.right  = static_cast<long>(maxScreenVect.x);
		rect.bottom = static_cast<long>(maxScreenVect.y);

		//-- fudge factor

		const UIPoint margin (12L, 8L);
		rect.left    -= margin.x;
		rect.top     -= margin.y;
		rect.right   += margin.x;
		rect.bottom  += margin.y;

		return true;
	}
	
	//----------------------------------------------------------------------

	/**
	* Sloppily clip the rectangle and make sure it stays above its minimum size
	*/

	bool clipRectSloppy (UIRect & rect, const UIRect & clipper, const UISize & minSize)
	{
		//--
		if (rect.left < clipper.left)
			rect.left = (clipper.left + rect.left) / 2L;
		if (rect.right > clipper.right)
			rect.right = (clipper.right + rect.right) / 2L;
		if (rect.top < clipper.top)
			rect.top = (clipper.top + rect.top) / 2L;
		if (rect.bottom > clipper.bottom)
			rect.bottom = (clipper.bottom + rect.bottom) / 2L;

		//-- enforce minimum rectangle size
		const long height = rect.bottom - rect.top;
		if (height < minSize.y)
		{
			const long pad = (minSize.y - height) / 2L;
			rect.top    -= pad;
			rect.bottom += pad;
		}

		const long width = rect.right - rect.left;
		if (width < minSize.x)
		{
			const long pad = (minSize.x - width) / 2L;
			rect.left   -= pad;
			rect.right  += pad;
		}

		return (rect.right >= clipper.left && rect.left <= clipper.right && rect.top <= clipper.bottom && rect.bottom >= clipper.top);
	}

	//----------------------------------------------------------------------

	inline float computeNameTimeIn (const ClientObject & obj)
	{
		float timeIn = 0.0f;
		if (GameObjectTypes::isTypeOf (obj.getGameObjectType (), SharedObjectTemplate::GOT_misc_furniture))
			timeIn = 2.0f;

		return timeIn;
	}

	//----------------------------------------------------------------------

	const float hover_name_timeout = 1.0f;

	//----------------------------------------------------------------------

	const std::string ChatTabDragCommand  = "/ui action chatTab ";
	const size_t ChatTabDragCommandLength = ChatTabDragCommand.size ();

	//----------------------------------------------------------------------

	const float THROTTLE_LOOK_AT_TIME = 0.5f;

	//----------------------------------------------------------------------
	UIPoint targetAndRadialCursorPos;

	UILowerString const c_propertyNameAllowLookAtTargetSelection("AllowLookAtTargetSelection");

}

//-----------------------------------------------------------------

using namespace SwgCuiHudNamespace;
	
//-----------------------------------------------------------------

SwgCuiHud::SwgCuiHud (UIPage & page) : 
CuiMediator             ("SwgCuiHud", page),
UIEventCallback         (),
m_targetingEnabled(true),
m_workspace             (new CuiWorkspace (page, true)),
m_lastSelectedObject    (NULL),
m_action                (NULL),
m_lastReticlePoint      (),
m_windowManager         (NULL),
m_lastDragInfo          (new CuiDragInfo),
m_hudEnabled            (true),
m_reticleDefault        (NULL),
m_reticleImage          (NULL),
m_radialMenuObject         (NULL),
m_radialMenuCountingDown   (true),
m_radialMenuTimeout        (0.0f),
m_radialMenuStartTime      (0.0f),
m_radialMenuTimerCursorPos (),
m_playerIdentificationString(),
m_fireDelay             (0.0f),
m_reticleObject         (NULL),
m_adjacentReticleObjects()
{

	m_playerIdentificationString = createPlayerIdentificationString();

	CuiWorkspace::setGameWorkspace(m_workspace);

	getCodeDataObject (TUIImage, m_reticleImage, "reticleImage");
	m_reticleImage->SetColor   (UIColor::white);
	m_reticleImage->SetOpacity (1.0f);

	m_reticleDefault = safe_cast<UICursor *>(UIManager::gUIManager ().GetObjectFromPath ("/styles.cursors.reticle_default", TUICursor));
	m_reticleDefault->Attach (0);

	{
		UIPage * selBox = 0;
		getCodeDataObject (TUIPage,    selBox,    "selectionBoxPage");
		
		UIPage * const selBoxParent = safe_cast<UIPage *>(selBox->GetParent ());
		NOT_NULL (selBoxParent);
		
		{
			const UIBaseObject::UIObjectList & olist = selBox->GetChildrenRef ();
			for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
			{
				UIBaseObject * const obj = *it;
				if (obj->IsA (TUIWidget))
					safe_cast<UIWidget *>(obj)->SetOpacity (1.0f);
			}
		}


		for (int i = 0; i < SBT_numTypes; ++i)
		{
			m_selectionBoxPages[i] = safe_cast<UIPage *>(selBox->DuplicateObject ());
			selBoxParent->InsertChildBefore(m_selectionBoxPages [i], selBox);
			m_selectionBoxPages[i]->Link();
			m_selectionBoxPages[i]->SetEnabled(false);

			// Force the page to visible to avoid any data issues.
			m_selectionBoxPages[i]->SetVisible(true); 

			switch(i)
			{
			case SBT_intended:
				m_selectionBoxPages[i]->SetName("hover");
				m_selectionBoxPages[i]->SetColor(UIColor(0x00, 0xff, 0x44));
				break;

			case SBT_target:
				m_selectionBoxPages[i]->SetName("target");
				m_selectionBoxPages[i]->SetColor(CuiGameColorManager::getColorForType(CuiGameColorManager::T_target));
				break;

			case SBT_assist:
				m_selectionBoxPages[i]->SetName("assist");
				m_selectionBoxPages[i]->SetColor(CuiGameColorManager::getColorForType(CuiGameColorManager::T_assist));
				break;

			default:
				m_selectionBoxPages[i]->SetName("mission_group");
				m_selectionBoxPages[i]->SetColor(CuiGameColorManager::getColorForType(CuiGameColorManager::T_combatTarget));
				break;
			}
		}
		
		selBox->SetEnabled (false);
		selBox->SetVisible (false);
	}


	{
		UIString dragAccepts;
		getPage ().GetProperty (UIWidget::PropertyName::DragAccepts, dragAccepts);
		if (!dragAccepts.empty ())
			dragAccepts.push_back(',');

		dragAccepts += Unicode::narrowToWide (CuiDragInfo::DragTypes::GameObject);

		dragAccepts.push_back(',');
		dragAccepts += Unicode::narrowToWide (CuiDragInfo::DragTypes::Command);

		getPage ().SetProperty (UIWidget::PropertyName::DragAccepts, dragAccepts);
	}

	getPage ().SetMouseCursor (0);
	SwgCuiG15Lcd::initializeLcd();

} //lint !e1401 //m_reticles

//-----------------------------------------------------------------

SwgCuiHud::~SwgCuiHud ()
{
	SwgCuiG15Lcd::remove();
	for(std::vector<Watcher<ReticleObject> >::iterator i = m_adjacentReticleObjects.begin(); i != m_adjacentReticleObjects.end(); ++i)
	{
		if(i->getPointer())
		{		
			i->getPointer()->removeFromWorld();
			ReticleManager::disableReticleObject(*(i->getPointer()));
			ReticleManager::giveBackReticleObject(i->getPointer());
		}
	}
	if(m_reticleObject.getPointer())
	{
		m_reticleObject.getPointer()->removeFromWorld();
		delete m_reticleObject.getPointer();
	}

	m_reticleDefault->Detach (0);
	m_reticleDefault = 0;

	delete m_lastDragInfo;
	m_lastDragInfo = 0;

	delete m_windowManager;
	m_windowManager = 0;

	if (m_action)
	{
		CuiActionManager::removeAction(m_action);
	}
	delete m_action;
	m_action = 0;
	
	{
		for (int i = 0; i < SBT_numTypes; ++i)
		{
			UIBaseObject * const parent = m_selectionBoxPages [i]->GetParent ();
			if (parent)
				parent->RemoveChild (m_selectionBoxPages [i]);
			m_selectionBoxPages [i] = 0;
		}
	}
	
	CuiWorkspace::setGameWorkspace (0);

	delete m_workspace;
	m_workspace = 0;

}

//-----------------------------------------------------------------

void SwgCuiHud::performActivate()
{	
	CuiWorkspace::setGameWorkspace(m_workspace);

	CuiManager::getIoWin ().setRadialMenuActiveHack (false);

	setInputToggleActive    (true);
	getPage ().AddCallback (this);

	setIsUpdating          (true);

	if (m_hudEnabled && m_windowManager != NULL)
		m_windowManager->handlePerformActivate ();

	CuiManager::getIoWin().resetScreenCenter();
}

//-----------------------------------------------------------------

void SwgCuiHud::performDeactivate()
{
	CuiManager::getIoWin ().setRadialMenuActiveHack (false);

	setIsUpdating          (false);

	getPage ().RemoveCallback (this);

	if (m_windowManager != NULL)
		m_windowManager->handlePerformDeactivate ();

	CuiRadialMenuManager::clear ();

	CuiSettings::save   ();

	CurrentUserOptionManager::save ();
	LocalMachineOptionManager::save ();
}

//-----------------------------------------------------------------

void SwgCuiHud::setHudEnabled        (bool b)
{
	if (b)
	{
		CuiWorkspace::setGameWorkspace(m_workspace);
	}

	CuiManager::getIoWin ().setRadialMenuActiveHack (false);

	for (int i = 0; i < SBT_numTypes; ++i)
	{
		m_selectionBoxPages [i]->SetEnabled (false);
	}

	m_workspace->setEnabled (b);
	
	m_hudEnabled = b;

	if (b)
	{
		if (m_windowManager != NULL)
			m_windowManager->handlePerformActivate ();
	}
	else
	{
		if (m_windowManager != NULL)
			m_windowManager->handlePerformDeactivate ();
		m_reticleImage->SetVisible (false);
	}
}

//-----------------------------------------------------------------

bool SwgCuiHud::getHudEnabled        () const
{
	return m_hudEnabled;
}

//-----------------------------------------------------------------

bool SwgCuiHud::OnMessage( UIWidget * const context, const UIMessage & msg)
{	
	static bool s_mouse1Down = false;
	static bool s_mouse2Down = false;


	if (context == &getPage ())
	{
		//-- this action should be cancelled if the mouse moves too much during the mouse press, regardless of cursor mode
		if (msg.Type == UIMessage::MouseMove)
		{
			const UIPoint & Delta = msg.MouseCoords - targetAndRadialCursorPos;
			const float mag = Delta.Magnitude ();
			if (mag > UIManager::gUIManager ().GetDragThreshold ())
				setIntendedAndSummonRadialMenu(true, true);
		}

		//-- right mouse button down should move the player relative to the camera in ui mode
		if (msg.Type == UIMessage::MouseMove)
		{
			if (m_radialMenuCountingDown && !CuiRadialMenuManager::isActive ())
			{
				const UIPoint & Delta  = msg.MouseCoords - m_radialMenuTimerCursorPos;
				const float mag = Delta.Magnitude ();
				if (mag > UIManager::gUIManager ().GetDragThreshold ())
				{
					//-- force radial timeout
					m_radialMenuTimeout = 0.0f;
					return false;
				}
			}

			return true;
		}

		UIWidget const * const widgetUnderMouse = getPage ().GetWidgetFromPoint(msg.MouseCoords, true);

		if (!Game::isHudSceneTypeSpace())
		{
			bool mouseMoveStart = false;
			bool mouseMoveStop = false;

			// If using option to have holding both mouse buttons down make you walk is enabled...
			if (CuiPreferences::getMouseLeftAndRightDrivesMovementToggle())
			{
				bool mayDoPress = (!s_mouse1Down || !s_mouse2Down) && (msg.Type == UIMessage::LeftMouseDown || msg.Type == UIMessage::RightMouseDown);
				bool mayDoRelease = (s_mouse1Down && s_mouse2Down) && (msg.Type == UIMessage::LeftMouseUp || msg.Type == UIMessage::RightMouseUp);
				if (msg.Type == UIMessage::LeftMouseDown && widgetUnderMouse == &getPage())
					s_mouse1Down = true;
				if (msg.Type == UIMessage::RightMouseDown && widgetUnderMouse == &getPage())
					s_mouse2Down = true;
				if (msg.Type == UIMessage::LeftMouseUp) 
					s_mouse1Down = false;
				if (msg.Type == UIMessage::RightMouseUp) 
					s_mouse2Down = false;

				if (mayDoPress && s_mouse1Down && s_mouse2Down)
					mouseMoveStart = true;
				else if (mayDoRelease && (!s_mouse1Down || !s_mouse2Down))
					mouseMoveStop = true;
			}
			
			// If using option to have holding middle mouse button down make you walk is enabled...
			if (CuiPreferences::getMiddleMouseDrivesMovementToggle())
			{
				if (msg.Type == UIMessage::MiddleMouseDown && widgetUnderMouse == &getPage())
					mouseMoveStart = true;
				else if (msg.Type == UIMessage::MiddleMouseUp)
					mouseMoveStop = true;
			}

			if (msg.Type == UIMessage::LeftMouseDown && GroundCombatActionManager::wantSecondaryAttackLocation())
			{

				if (ReticleManager::getReticleCurrentlyValid())
				{
					Vector attackerTargetLocation = ReticleManager::getLastGroundReticlePoint();
					const NetworkId attackerTargetCell = ReticleManager::getLastGroundReticleCell();
					GroundCombatActionManager::setSecondaryAttackLocation(attackerTargetLocation, attackerTargetCell);
				}

			}

			if (msg.Type == UIMessage::LeftMouseDown)
			{
				if(ReticleManager::getReticleCurrentlyValid() && (CuiRadialMenuManager::isStorytellerMode() || CuiRadialMenuManager::isGrenadeTargetMode()) )
				{
					NetworkId const lastCell = ReticleManager::getLastGroundReticleCell();
					Vector locationCell = ReticleManager::getLastGroundReticlePoint();
					CuiRadialMenuManager::createGroundPlacementMessage(locationCell, lastCell);
					CuiRadialMenuManager::setStorytellerMode(false);
					CuiRadialMenuManager::setGrenadeTargetMode(false);
				}
			}

			if (mouseMoveStart)
			{
				GroundCombatActionManager::ActionType actionType = GroundCombatActionManager::AT_none;
				actionType = GroundCombatActionManager::AT_walkForward;
				GroundCombatActionManager::attemptAction(actionType);

				setIntendedAndSummonRadialMenu(true, true);
				return false;
			}
			else if (mouseMoveStop)
			{
				GroundCombatActionManager::ActionType actionType = GroundCombatActionManager::AT_none;
				actionType = GroundCombatActionManager::AT_walkForward;
				GroundCombatActionManager::clearAction(actionType);

				setIntendedAndSummonRadialMenu(true, true);
				return true;
			}

			if (msg.Type == UIMessage::LeftMouseDoubleClick && CuiPreferences::getAutoAimToggle())
			{
				if (m_lastSelectedObject.getPointer() && CuiRadialMenuManager::findDefaultAction(*m_lastSelectedObject.getPointer()) == Cui::MenuInfoTypes::COMBAT_ATTACK)
				{
					NetworkId selectedId = m_lastSelectedObject.getPointer()->getNetworkId();
					CreatureObject * const player = Game::getPlayerCreature();
					if (player && 
						(!player->getIntendedTarget().isValid() || player->getIntendedTarget() == selectedId) && 
						!GroundCombatActionManager::getRepeatAttackEnabled())
					{
						player->setIntendedTarget(selectedId);
						GroundCombatActionManager::ActionType actionType = GroundCombatActionManager::AT_toggleRepeatPrimaryAttack;
						GroundCombatActionManager::attemptAction(actionType);
					}
				}
			}

			if (msg.Type == UIMessage::MouseWheel)
			{
				if (widgetUnderMouse == &getPage())
				{
					if (CuiPreferences::getScrollThroughDefaultActions())
					{
						SwgCuiToolbar * const toolBar = SwgCuiToolbar::getActiveToolbar();

						if (toolBar != 0)
						{
							if (msg.Data > 0)
							{
								toolBar->selectNextAvailableSlot();
							}
							else if (msg.Data < 0)
							{
								toolBar->selectPreviousAvailableSlot();
							}

							return false;
						}
					}
				}
			}

			// Cancels the secondary attack target location reticle on ESC...
			// Also cancels story teller targetting!
			if (msg.Type == UIMessage::KeyDown && msg.Keystroke == UIMessage::Escape)
			{
				if (GroundCombatActionManager::wantSecondaryAttackLocation())
				{
					GroundCombatActionManager::clearAction(GroundCombatActionManager::AT_secondaryAttack);
					GroundCombatActionManager::clearAction(GroundCombatActionManager::AT_secondaryAttackFromToolbar);
					return false;
				}
			}
		}
		
		if (msg.Type == UIMessage::LeftMouseDoubleClick)
		{
			bool const isControlDown = msg.Modifiers.isControlDown();
			
			if (widgetUnderMouse && widgetUnderMouse->IsA(TUI3DObjectListViewer) && isControlDown && !msg.Modifiers.isShiftDown())
			{
				CuiWidget3dObjectListViewer const * const viewer = dynamic_cast<CuiWidget3dObjectListViewer const * const>(widgetUnderMouse);
				if (viewer) 
				{
					Object const * const object = viewer->getLastObject();
					if (object) 
					{
						CuiRadialMenuManager::performDefaultDoubleClickAction(*object, true);
					}
				}

				return false;
			}

			return true;
		}
				
		else if ((msg.Type == UIMessage::KeyDown || msg.Type == UIMessage::KeyRepeat || msg.Type == UIMessage::Character ||
			      msg.Type == UIMessage::IMEComposition || msg.Type == UIMessage::IMEChangeCandidate) &&
				msg.Keystroke != UIMessage::Escape && 
				msg.Keystroke != UIMessage::Enter && 
				msg.Keystroke != UIMessage::Tab) 
		{
			if( CuiPreferences::getModalChat() && 
				(msg.Type == UIMessage::IMEComposition || msg.Type == UIMessage::IMEChangeCandidate) &&
				(UIManager::gUIManager().getUIIMEManager()->GetContextWidget() == NULL))
			{
				return false;
			}
			//-- if the pointer is active, only switch the input to the chat window
			//-- if the currently focused widget is NOT another TEXTBOX
			
			if (CuiManager::getPointerInputActive ())
			{
				const UIWidget * const focused = UIManager::gUIManager ().GetRootPage ()->GetFocusedLeafWidget ();
				if (focused && (focused->IsA (TUITextbox) || focused->IsA (TUIText) || focused->IsA (TUIComboBox) || focused->IsA (TUIList) || focused->IsA (TUITable) || focused->IsA(TUIWebBrowser) ))
					return true;
			}

			if (m_windowManager != NULL)
			{
				if (msg.Keystroke == UIMessage::PageUp || msg.Keystroke == UIMessage::PageDown)
					m_windowManager->acceptTextInput (false);
				else
					m_windowManager->acceptTextInput (true);
			}
		}
		else if (msg.Type == UIMessage::DragOver)
		{
			getPage ().SetDropFlagOk (false);
			if (msg.DragObject)
			{
				m_lastDragInfo->set (*msg.DragObject);
				checkDropOk ();
			}

			return false;
		}
		else if (msg.Type == UIMessage::DragEnd)
		{
			if (msg.DragObject)
			{
				m_lastDragInfo->set (*msg.DragObject);
				handleDrop ();
			}

			return false;
		}
	}
	
	return true;
}

//----------------------------------------------------------------------

void SwgCuiHud::checkDropOk ()
{
	switch (m_lastDragInfo->type)
	{
	case CuiDragInfoTypes::CDIT_object:
		{
			ClientObject * const selectedObject = dynamic_cast<ClientObject *>(m_lastSelectedObject.getPointer ());
			
			if (selectedObject && selectedObject->getNetworkId () != NetworkId::cms_invalid)
			{
				if (CuiDragManager::isDropOk (*m_lastDragInfo, *selectedObject, std::string ()))
					getPage ().SetDropFlagOk (true);
			}
		}
		break;
	case CuiDragInfoTypes::CDIT_command:
	case CuiDragInfoTypes::CDIT_macro:
		{
			if (m_lastDragInfo->commandValueValid && m_lastDragInfo->str.compare (0, ChatTabDragCommandLength, ChatTabDragCommand) == 0)
				getPage ().SetDropFlagOk (true);
		}
		break;
	case CuiDragInfoTypes::CDIT_none:
	case CuiDragInfoTypes::CDIT_numTypes:
	default:
		break;
	} //lint !e788 //types not used
}

//----------------------------------------------------------------------

void SwgCuiHud::handleDrop ()
{
	switch (m_lastDragInfo->type)
	{
	case CuiDragInfoTypes::CDIT_object:
		{
			ClientObject * const selectedObject = dynamic_cast<ClientObject *>(m_lastSelectedObject.getPointer ());
			
			if (selectedObject && selectedObject->getNetworkId () != NetworkId::cms_invalid)
			{
				if (!CuiDragManager::handleDrop (*m_lastDragInfo, *selectedObject, std::string ()))
				{
					//@todo: emit a warning beep or feedback of some sort
				}
			}
		}
		break;
	case CuiDragInfoTypes::CDIT_command:
	case CuiDragInfoTypes::CDIT_macro:
		{
			if (m_windowManager != NULL)
			{
				if (m_lastDragInfo->commandValueValid && m_lastDragInfo->str.compare (0, ChatTabDragCommandLength, ChatTabDragCommand) == 0)
				{
					const UIPoint pt = m_lastReticlePoint + UIManager::gUIManager ().getDragEchoOffset ();
					m_windowManager->receiveDroppedChatTab (pt, m_lastDragInfo->commandValue);
				}
			}
		}
		break;
	case CuiDragInfoTypes::CDIT_none:
	case CuiDragInfoTypes::CDIT_numTypes:
	default:
		break;
	} //lint !e788 //types not used

	m_lastDragInfo->clear ();
}

//-----------------------------------------------------------------

void SwgCuiHud::update (float deltaTimeSecs)
{
	SwgCuiG15Lcd::updateLcd();

	CuiMediator::update (deltaTimeSecs);

	ReticleManager::update (deltaTimeSecs);

	GroundCombatActionManager::ObjectVector orderedTargets;

	GroundScene * const gs = dynamic_cast<GroundScene *>(Game::getScene ());
	const Camera * const camera = gs ? NON_NULL (gs->getCurrentCamera ()) : 0;
	
	if (m_windowManager != NULL)
		m_windowManager->update ();

	if (!camera)
		return;

	CreatureObject * const player = Game::getPlayerCreature ();

	if (!player)
		return;

	CachedNetworkId const & intendedTargetId = player->getIntendedTarget();

	static const float viewDistance = ConfigClientGame::getTargetingRange ();
	
	Vector viewDirection;
	Vector worldStart;
		
	UIPoint reticlePoint(UIManager::gUIManager ().GetLastMouseCoord ());

	Object * foundObject = 0;
	bool updateCursor = false;
	bool allowLookAtTargetSelection = false;

		//update the reticle
	CreatureObject *playerCreature = Game::getPlayerCreature();
	if(playerCreature && !Game::isSpace())
	{
		if (GroundCombatActionManager::wantSecondaryAttackLocation())
		{
			ReticleManager::setEnabled(true);
			ReticleManager::setRenderReticles(true);
		}
		else if (!playerCreature->getCurrentWeapon())
			ReticleManager::setEnabled(false);
		else
		{		
			const WeaponObject *playerWeapon = playerCreature->getCurrentWeapon();
			if(playerCreature->getPrimaryActionWantsGroundReticule())
			{
				// No splat if in auto aim mode with not target...
				if (CuiPreferences::getAutoAimToggle() && intendedTargetId == NetworkId::cms_invalid)
					ReticleManager::setEnabled(false);
				else
					ReticleManager::setEnabled(true);
				ReticleManager::setRenderReticles(true);
			}
			else if(playerWeapon->isDirectionalTargetting())
			{
				ReticleManager::setEnabled(true);
				ReticleManager::setRenderReticles(false);
			}
			else
			{
				ReticleManager::setEnabled(false);
			}
		}

		if(CuiRadialMenuManager::isStorytellerMode())
		{
			ReticleManager::setEnabled(true);
			ReticleManager::setRenderReticles(true);
			ReticleManager::setReticleType(ReticleManager::RMT_StoryTeller);
		}
		else if(CuiRadialMenuManager::isGrenadeTargetMode())
		{
			ReticleManager::setEnabled(true);
			ReticleManager::setRenderReticles(true);
			ReticleManager::setReticleType(ReticleManager::RMT_GrenadeTarget);
		}
		else
		{
			ReticleManager::setReticleType(ReticleManager::RMT_HeavyWeapons);
		}
	}

	//----------------------------------------------------------------------
	
	if (m_targetingEnabled)
	{
		if (!CuiRadialMenuManager::isActive())
		{
			//-- update radial menu
			if (m_radialMenuCountingDown)
			{
				if (Game::getElapsedTime () > m_radialMenuTimeout)
				{
					m_radialMenuCountingDown = false;				
					Object * const object = m_radialMenuObject.getPointer ();
					if (object)
						IGNORE_RETURN (CuiRadialMenuManager::createMenu (*object, m_radialMenuTimerCursorPos));
				}
			}
		}
		
		{
			PROFILER_AUTO_BLOCK_DEFINE("targeting tests");
			UIWidget const * const widgetUnderMouse = getPage().GetWidgetFromPoint(reticlePoint, true);

			bool usedReticles = false;
			updateCursor = widgetUnderMouse == &getPage();

			for (UIBaseObject const * baseObject = widgetUnderMouse; baseObject; baseObject = baseObject->GetParent())
			{

				if (baseObject->HasProperty(c_propertyNameAllowLookAtTargetSelection))
				{
					baseObject->GetPropertyBoolean(c_propertyNameAllowLookAtTargetSelection, allowLookAtTargetSelection);
					break;
				}
			}

			Vector worldEnd;

			if (updateCursor || allowLookAtTargetSelection)
			{
				worldStart = camera->getPosition_w ();
				viewDirection = camera->rotate_o2w( camera->reverseProjectInScreenSpace (reticlePoint.x, reticlePoint.y));
				viewDirection.normalize ();

				worldEnd = worldStart + viewDirection * viewDistance;

				// @todo godclient users want to be able to disable targeting
				findAllTargettableObjects(*camera, worldStart, worldEnd, *player, orderedTargets);
				foundObject = (!orderedTargets.empty()) ? (orderedTargets.front().getPointer()) : 0;

				m_lastReticlePoint     = reticlePoint;
				

				// We don't want to show the reticle if the action that would use it is not possible anyhow
				// don't show a reticle in space
				// heavy weapons should not show reticle if we are riding a mount sitting or skill animating (dancing)
				// overridden primary actions should show it if the command checks will allow the action
				// secondary attacks that require a location should be allowed if the command checks will allow the action
				bool showReticle = !Game::isSpace() && ReticleManager::getEnabled();
				if(showReticle)
				{
					if(GroundCombatActionManager::wantSecondaryAttackLocation())
					{
						//if we made it this far we know the command will be allowed so show the reticle
						showReticle = true;
					}
					else if (player->getPrimaryActionOverridden())
					{
						std::string const & primaryCommandName = player->getCurrentPrimaryActionName();
						uint32 const primaryCmdHash = Crc::normalizeAndCalculate(primaryCommandName.c_str());
						Command const & cmd = CommandTable::getCommand(primaryCmdHash);

						showReticle = !(ClientCommandChecks::doesStateInvalidateCommand(&cmd, player) || ClientCommandChecks::doesLocomotionInvalidateCommand(&cmd, player));
					}
					else // !player->getPrimaryActionOverridden()
					{
						showReticle = !(player->getState(States::RidingMount)) && !(player->getVisualPosture() == Postures::Sitting) && !(player->getVisualPosture() == Postures::SkillAnimating);
					}
				}

				if(showReticle)
				{
					//-- show reticles
					// Default reticle position is 20 yards away from the player the direction they're facing...
					CellProperty *hitCell = CellProperty::getWorldCellProperty();
					Vector hitPoint = player->getPosition_w() + 20.0f * player->getObjectFrameK_w ();
					bool didHit = false;

					if (!GroundCombatActionManager::wantSecondaryAttackLocation() && CuiPreferences::getAutoAimToggle() && intendedTargetId != NetworkId::cms_invalid)
					{
						// We have auto aim on, overload the position of the heavy weapon splat
						// We're ALWAYS gonna be attacking our locked target.
						Object *intendedTarget = NetworkIdManager::getObjectById(intendedTargetId);
						if (intendedTarget)
						{
							hitCell = intendedTarget->getParentCell();
							hitPoint = intendedTarget->getPosition_w();
							didHit = true;
						}
					}
					else
					{
						// Look for world/object collision to figure out if our heavy weapon will hit something...
						didHit = hitReticle(*camera, worldStart, worldEnd, *player, hitPoint, &hitCell);
					}

					float distance = hitPoint.magnitudeBetween(player->getPosition_w());
					bool inRange = true;
					if (!GroundCombatActionManager::isReticleLocationInRange(distance))
						inRange = false;

					ReticleManager::setLastGroundReticlePoint(hitPoint);	
					ReticleManager::setLastGroundReticleCell(hitCell ? hitCell->getOwner().getNetworkId() : NetworkId::cms_invalid);

					if (updateCursor)
					{
						if(!m_reticleObject.getPointer())
						{
							m_reticleObject = ReticleManager::getReticleObject();
							m_reticleObject.getPointer()->setRadius(5.0f);
							m_reticleObject->addToWorld();
							ReticleManager::enableReticleObject(*m_reticleObject.getPointer());						
						}							
						m_reticleObject->setPosition_w(hitPoint);
						m_reticleObject->setParentCell(hitCell);

						//Give back the old reticles
						for(std::vector<Watcher<ReticleObject> >::iterator i = m_adjacentReticleObjects.begin(); i != m_adjacentReticleObjects.end(); ++i)
						{
							if(i->getPointer())
							{							
								i->getPointer()->removeFromWorld();
								ReticleManager::disableReticleObject(*(i->getPointer()));
								ReticleManager::giveBackReticleObject(i->getPointer());
							}
						}
						m_adjacentReticleObjects.clear();

						Vector reticleCenter = hitPoint;
						if(!hitCell->isWorldCell())
						{
							const Object* owner = hitCell->getAppearanceObject();
							if(owner)
							{
								reticleCenter = owner->rotateTranslate_w2p(reticleCenter);
							}
						}

						Sphere retSphere(reticleCenter,2.5f);
						std::vector<CellProperty *> cellVect;
						hitCell->getDestinationCells(retSphere,cellVect);
						for(uint i = 0; i < cellVect.size(); ++i)
						{
							ReticleObject *reticle = ReticleManager::getReticleObject();
							reticle->addToWorld();
							reticle->setRadius(5.0f);
							reticle->setPosition_w(hitPoint);
							reticle->setParentCell(const_cast<CellProperty*> (cellVect[i]));
							ReticleManager::enableReticleObject(*reticle);
							m_adjacentReticleObjects.push_back(Watcher<ReticleObject>(reticle));
						}

						//Set up the fake animation "point at" target
						CreatureController *const controller = dynamic_cast<CreatureController*>(player->getController());
						if (controller && !CuiRadialMenuManager::isStorytellerMode())
							controller->overrideAnimationTarget(m_reticleObject, true, CrcLowerString());

						Vector lookAtVector = hitPoint - player->getPosition_w();
						player->setLookAtYaw(lookAtVector.theta(), true);					
						ReticleManager::setReticleCurrentlyValid(didHit && inRange);	

						usedReticles = true;
					}
				} 
			}

			if(!usedReticles) //-- mouse is over a UI widget, or we're in space, or we turned off reticles
			{
				if(player->getUseLookAtYaw()) // player still thinks we're in this mode
				{
					player->setLookAtYaw(0.f, false);	
				}
				ReticleManager::setEnabled(false);

				//Give back the old reticles
				for(std::vector<Watcher<ReticleObject> >::iterator i = m_adjacentReticleObjects.begin(); i != m_adjacentReticleObjects.end(); ++i)
				{
					if(i->getPointer())
					{						
						i->getPointer()->removeFromWorld();
						ReticleManager::disableReticleObject(*(i->getPointer()));
						ReticleManager::giveBackReticleObject(i->getPointer());
					}
				}
				m_adjacentReticleObjects.clear();

				if(m_reticleObject.getPointer())
				{					
					m_reticleObject.getPointer()->removeFromWorld();
					ReticleManager::disableReticleObject( *(m_reticleObject.getPointer()) );
					ReticleManager::giveBackReticleObject(m_reticleObject.getPointer());
					m_reticleObject = NULL;
				}
				ReticleManager::setReticleCurrentlyValid(false);
			}
			//-- end reticle code
			player->setTargetUnderCursor(foundObject	? foundObject->getNetworkId() : NetworkId::cms_invalid);
			if (m_lastSelectedObject.getPointer() != foundObject || (foundObject && player->getLookAtTarget().getObject() != foundObject))
			{
				player->setLookAtTarget(foundObject ? foundObject->getNetworkId() : NetworkId::cms_invalid);
				m_lastSelectedObject = foundObject;
			}
		}
		
		{
			foundObject = m_lastSelectedObject.getPointer ();
			IGNORE_RETURN(CuiRadialMenuManager::updateRanges ());
		}
	}
	else
		m_radialMenuCountingDown = false;
			
	m_reticleImage->SetVisible (shouldRenderReticle());
	
	//----------------------------------------------------------------------
	
	UICursor * theCursor = 0;

	const PlayerCreatureController *const playerController = safe_cast<const PlayerCreatureController*> (player->getController ());
	NOT_NULL (playerController);


	//setup default based on auto aim state
	if ( CuiPreferences::getAutoAimToggle() )
	{
		theCursor = Cui::MenuInfoTypes::getIntendedAttackInactiveCursor();
	}
	else
	{
		theCursor = m_reticleDefault;
	}

	if (foundObject && updateCursor)
	{
		ClientObject * const clientObject = foundObject->asClientObject ();

		if (clientObject)
		{
			const float timeIn = computeNameTimeIn (*clientObject);
			IGNORE_RETURN(CuiObjectTextManager::setNameRender (*clientObject, true, timeIn, hover_name_timeout, false));
			UICursor * tmpCursor = Cui::MenuInfoTypes::findDefaultCursor (*clientObject);

			if (tmpCursor)
			{
				theCursor = tmpCursor;
			}
			
			if( Cui::MenuInfoTypes::isAttackCursor(theCursor) &&
				CuiPreferences::getAutoAimToggle() )  // auto aim is on
			{
				theCursor = Cui::MenuInfoTypes::getIntendedAttackInactiveCursor();
			}
		}
	}

	if (!m_reticleImage->IsVisible ())
	{
		getPage ().SetMouseCursor (foundObject && updateCursor ? theCursor : NULL);
	}
	
	//----------------------------------------------------------------------

	TerrainObject const * const terrainObject = TerrainObject::getInstance();

	if (terrainObject)
		manageTargetBoxes(NetworkIdManager::getObjectById(player->getIntendedTarget()), *camera, *terrainObject);

	//----------------------------------------------------------------------

	if (m_reticleImage->IsVisible () && theCursor)
	{
		m_reticleImage->SetStyle (theCursor->GetImageStyle ());
		reticlePoint -= theCursor->GetHotSpot ();
		m_reticleImage->SetSize     (theCursor->GetSize ());
		m_reticleImage->SetLocation (reticlePoint);
	}

	getPage ().SetDropFlagOk (false);
	if (m_lastDragInfo->ok ())
		checkDropOk ();

	if (m_hudEnabled && m_windowManager != NULL)
		m_windowManager->updateWindowManager (deltaTimeSecs);

	if (m_workspace)
	{
		m_workspace->update(deltaTimeSecs);

		if (m_workspace->getShowGlow()) 
		{
			UIPoint const & point = UIManager::gUIManager().GetLastMouseCoord();
			UIWidget * const widgetUnderMouse = getPage().GetWidgetFromPoint(point, true);
			if (widgetUnderMouse)
			{
				UIPage  * const page = UI_ASOBJECT(UIPage, widgetUnderMouse);
				if (page) 
				{
					m_workspace->focusGlowMediatorByPage(*page);
				}
			}
		}
	}


	GroundCombatActionManager::update(deltaTimeSecs, orderedTargets);
}

//----------------------------------------------------------------------

void SwgCuiHud::manageTargetBoxes (const Object * intendedObject, const Camera & camera, const TerrainObject & terrain)
{
	const CreatureObject * const player = Game::getPlayerCreature ();
	
	if (player == NULL)
		return;
	
	const UISize  pageSize (getPage ().GetSize ());
	const UIPoint pageTargetMargin (16L, 16L);
	const UIRect  targetClipRect (pageTargetMargin, pageSize - (pageTargetMargin * 2L));
	
	//----------------------------------------------------------------------
	//-- selection box
	
	bool found[SBT_numTypes];
	
	for (int i = 0; i < SBT_numTypes; ++i)
	{
		m_selectionBoxPages[i]->SetEnabled(false);
		found[i] = false;
	}
	
	
	// Check for critical ships in space.
	if (Game::isHudSceneTypeSpace())
	{
		// In space, the mission critical ship is your combat target..
		ShipObject const * const shipObject = Game::getPlayerContainingShip();
		if (shipObject)
		{
			// RLS TODO: How do we know if the target is critical for the current player???
			CreatureObject::GroupMissionCriticalObjectSet const & groupMissionCriticalObjects = player->getGroupMissionCriticalObjects();
			if (!groupMissionCriticalObjects.empty() )
			{
				int criticalObjectCount = 0;
				int const criticalObjectCountMax = static_cast<int>(SBT_critical_end) - static_cast<int>(SBT_critical_begin);
				CuiGameColorManager::Type const defaultTargetColor = CuiGameColorManager::T_combatTarget;
				
				for (CreatureObject::GroupMissionCriticalObjectSet::const_iterator itCritical = groupMissionCriticalObjects.begin(); itCritical != groupMissionCriticalObjects.end(); ++itCritical)
				{
					CachedNetworkId cachedID(itCritical->second);
					ClientObject const * criticalTarget = cachedID.getObject() ? cachedID.getObject()->asClientObject() : NULL;
					
					if (criticalTarget)
					{
						if (criticalObjectCount < criticalObjectCountMax)
						{
							int const combatIconIndex = SBT_critical_begin + criticalObjectCount;
							
							UIPage * const currentPage = m_selectionBoxPages[combatIconIndex];
							
							UIRect rect_selectedObject;
							if (computeScreenRect(camera, terrain, *criticalTarget, rect_selectedObject))
							{
								if (clipRectSloppy(rect_selectedObject, targetClipRect, currentPage->GetMinimumSize()))
								{
									currentPage->SetEnabled(true);
									currentPage->SetRect(rect_selectedObject);
									
									// Pick a color.
									CuiGameColorManager::Type targetColorType = defaultTargetColor;
									
									TangibleObject const * const tangibleObject = criticalTarget->asTangibleObject();
									if (tangibleObject)
									{
										// Look in the color manager.
										targetColorType = CuiGameColorManager::findTypeForObject(*tangibleObject, true);
									}
									
									currentPage->SetColor(CuiGameColorManager::getColorForType(targetColorType), true);
								}
							}
							
							found[combatIconIndex] = true;
							
							// Increment the critical object count here.
							++criticalObjectCount;
						}
						
						IGNORE_RETURN (CuiObjectTextManager::setNameRender(*criticalTarget, true, 0.0f, hover_name_timeout, false));
					}
				}
			}
			
			// In space, put some rects around the group members.
			CachedNetworkId const id(player->getGroup());
			GroupObject const * const group = safe_cast<const GroupObject *>(id.getObject());
			if (group)
			{
				int groupObjectCount = 0;
				int const groupObjectCountMax = static_cast<int>(SBT_group_end) - static_cast<int>(SBT_group_begin);
				
				static std::set<ShipObject const *> isDisplayed;
				isDisplayed.clear();
				
				// Always hide the selection rectangle in first person.
				CockpitCamera const * const cockpitcamera = dynamic_cast<CockpitCamera const *>(Game::getConstCamera());
				if (cockpitcamera && cockpitcamera->isZoomWithinFirstPersonDistance())
				{
					isDisplayed.insert(shipObject);
				}
				
				CachedNetworkId leaderId(group->getLeader());
				CachedNetworkId leaderShipId(group->getShipFromMember(leaderId));
				GroupObject::GroupMemberVector const & groupMembers = group->getGroupMembers();
				for (GroupObject::GroupMemberVector::const_iterator itMember = groupMembers.begin(); itMember != groupMembers.end(); ++itMember)
				{
					// Get the ship of the member in the group.
					GroupObject::GroupMember const & member = *itMember;
					CachedNetworkId shipNetworkId(group->getShipFromMember(member.first));
					Object const * const ship = shipNetworkId.getObject();
					ClientObject const * const clientShipObject =  ship ? ship->asClientObject() : NULL;
					ShipObject const * const memberShip = clientShipObject ? clientShipObject->asShipObject() : NULL;
					
					// Only display the ship if the ship is not null, it is not already displayed, and it is not mission critical.
					if (memberShip && (isDisplayed.find(memberShip) == isDisplayed.end()) && !player->isGroupMissionCriticalObject(memberShip->getNetworkId()))
					{
						// Add ship to the ships that have been displayed.
						IGNORE_RETURN(isDisplayed.insert(memberShip));
						
						if (groupObjectCount < groupObjectCountMax)
						{
							int const groupIconIndex = SBT_group_begin + groupObjectCount;
							
							UIPage * const currentPage = m_selectionBoxPages[groupIconIndex];
							
							UIRect rect_selectedObject;
							if (computeScreenRect(camera, terrain, *memberShip, rect_selectedObject))
							{
								if (clipRectSloppy(rect_selectedObject, targetClipRect, currentPage->GetMinimumSize()))
								{
									currentPage->SetEnabled(true);
									currentPage->SetRect(rect_selectedObject);
									
									// Pick a color.
									CuiGameColorManager::Type targetColorType = (memberShip->getNetworkId() == leaderShipId) ? CuiGameColorManager::T_groupLeader : CuiGameColorManager::T_group;
									currentPage->SetColor(CuiGameColorManager::getColorForType(targetColorType), true);
								}
							}
							
							found[groupObjectCount] = true;
							
							// Increment the group object count here.
							++groupObjectCount;
						}
						
						IGNORE_RETURN (CuiObjectTextManager::setNameRender(*memberShip, true, 0.0f, hover_name_timeout, false));
					}
				}
			}
		}
	}

	if (!found [SBT_target])
	{
		const ClientObject * const lookAtTarget = safe_cast<ClientObject *>(player->getLookAtTarget ().getObject ());
		
		if (lookAtTarget)
		{
			if(!Game::isHudSceneTypeSpace())
			{
				UIRect rect_selectedObject;
				if (computeScreenRect (camera, terrain, *lookAtTarget, rect_selectedObject))
				{
					if (clipRectSloppy (rect_selectedObject, targetClipRect, m_selectionBoxPages [SBT_target]->GetMinimumSize ()))
					{
						m_selectionBoxPages [SBT_target]->SetEnabled (true);
						m_selectionBoxPages [SBT_target]->SetRect    (rect_selectedObject);
					}
				}
				
				found [SBT_target] = true;
			}
			
			IGNORE_RETURN(CuiObjectTextManager::setNameRender (*lookAtTarget, true, 0.0f, hover_name_timeout, false));
		}
	}
	
	if ( intendedObject && !found [SBT_intended] )
	{
		UIRect rect_selectedObject;
		if (computeScreenRect (camera, terrain, *intendedObject, rect_selectedObject))
		{
			if (clipRectSloppy (rect_selectedObject, targetClipRect, m_selectionBoxPages [SBT_intended]->GetMinimumSize ()))
			{
				m_selectionBoxPages [SBT_intended]->SetEnabled  (true);
				m_selectionBoxPages [SBT_intended]->SetRect     (rect_selectedObject);
			}
		}

		ClientObject const * const intendedTarget = safe_cast<ClientObject const *>(intendedObject);

		if (intendedTarget && CuiPreferences::getShowStatusOverIntendedTarget())
			IGNORE_RETURN(CuiObjectTextManager::setNameRender(*intendedTarget, true, 0.0f, hover_name_timeout, false));
	}

	// If the player is looking at their intended target make the intended reticule a little smaller so its visible.
	if (m_selectionBoxPages [SBT_intended]->IsEnabled() && m_selectionBoxPages [SBT_target]->IsEnabled())
	{
		if (player->getLookAtTarget() == player->getIntendedTarget())
		{
			UIRect tempRect = m_selectionBoxPages [SBT_intended]->GetRect();
			if (tempRect.Width() > 10)
			{
				tempRect.left += 4;
				tempRect.right -= 4;
			}
			if (tempRect.Height() > 10)
			{
				tempRect.top += 4;
				tempRect.bottom -= 4;
			}
			m_selectionBoxPages [SBT_intended]->SetRect(tempRect);
		}
	}


	// see if we need to show the assist target
	if (found[SBT_target])
	{
		CachedNetworkId const & assistId = getHudAssistId();
		if(assistId != CachedNetworkId::cms_invalid)
		{		
			const Object * assistObject = assistId.getObject();
			if (assistObject != NULL && assistId != player->getNetworkId())
			{
				UIRect rect_selectedObject;
				if (computeScreenRect (camera, terrain, *assistObject, rect_selectedObject))
				{
					if (clipRectSloppy (rect_selectedObject, targetClipRect, m_selectionBoxPages [SBT_assist]->GetMinimumSize ()))
					{
						m_selectionBoxPages[SBT_assist]->SetEnabled (true);
						m_selectionBoxPages[SBT_assist]->SetRect    (rect_selectedObject);
					}
				}				
				const ClientObject * assistAsClientObject = assistObject->asClientObject();
				if(assistAsClientObject)
					IGNORE_RETURN(CuiObjectTextManager::setNameRender (*assistAsClientObject, true, 0.0f, hover_name_timeout, false));
				found[SBT_assist] = true;
			}
		}
	}

	// If disabled, reduce opacity.
	for (int selectionPageIndex = 0; selectionPageIndex < SBT_numTypes; ++selectionPageIndex)
	{
		UIPage * const currentPage = m_selectionBoxPages[selectionPageIndex];
		if (!currentPage->IsEnabled())
			currentPage->SetOpacity(0.0f);
	}
}


//----------------------------------------------------------------------

void SwgCuiHud::startConversingWithSelectedCreature () const
{
	if (m_lastSelectedObject.getPointer () != 0)
	{
		const CreatureObject * const creature = dynamic_cast<const CreatureObject *>(m_lastSelectedObject.getPointer ());

		if (creature)
			CuiConversationManager::start (creature->getNetworkId ());
	}
}

//----------------------------------------------------------------------

SwgCuiChatWindow *  SwgCuiHud::getChatWindow ()
{
	return m_windowManager != NULL ? m_windowManager->getChatWindow () : NULL;
}

//----------------------------------------------------------------------

void SwgCuiHud::colorTest (TangibleObject & object)
{
	if (m_windowManager != NULL)
		m_windowManager->colorTest (object);
}

//----------------------------------------------------------------------

void SwgCuiHud::hueObjectTest (const NetworkId & id, int maxIndex1, int maxIndex2)
{
	if (m_windowManager != NULL)
		m_windowManager->hueObjectTest (id, maxIndex1, maxIndex2);
}

//----------------------------------------------------------------------

void SwgCuiHud::internalCreateFreshHud (std::string const & pageName, std::string const & templateName)
{
	std::string const & pagePath = std::string("/") + pageName;
	std::string const & templatePath = std::string("/") + templateName;

	UIPage * const oldPage      = safe_cast<UIPage *>(UIManager::gUIManager ().GetObjectFromPath (pagePath.c_str (),     TUIPage));
	UIPage * const templatePage = safe_cast<UIPage *>(UIManager::gUIManager ().GetObjectFromPath (templatePath.c_str (), TUIPage));

	NOT_NULL (templatePage);
	if (templatePage) //lint !e774 //true
	{
		UIPage * const parent = UIManager::gUIManager().GetRootPage();
		NOT_NULL (parent);

		if (parent) //lint !e774 //true
		{
			parent->Pack ();
			templatePage->ForcePackChildren ();

			if (NULL != oldPage)
				parent->RemoveChild (oldPage);

			UIPage * const newPage = safe_cast<UIPage *>(templatePage->DuplicateObject ());
			NOT_NULL (newPage);

			newPage->SetName (pageName);
			parent->InsertChildBefore (newPage, templatePage);
			newPage->Link ();
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiHud::internalCreateHudTemplate (std::string const & pageName, std::string const & templateName)
{
	std::string const & pagePath = std::string("/") + pageName;
	std::string const & templatePath = std::string("/") + templateName;

	UIPage * const oldPage      = safe_cast<UIPage *>(UIManager::gUIManager ().GetObjectFromPath (pagePath.c_str (),     TUIPage));
	UIPage * const templatePage = safe_cast<UIPage *>(UIManager::gUIManager ().GetObjectFromPath (templatePath.c_str (), TUIPage));
	
	UNREF(templatePage);
	
	DEBUG_FATAL (templatePage, ("SwgCuiHud::createHUDTemplate\n"));
	NOT_NULL (oldPage);

	if (oldPage) //lint !e774 //true
	{
		UIPage * const parent = dynamic_cast<UIPage *>(oldPage->GetParent ());
		NOT_NULL (parent);

		if (parent) //lint !e774 //true
		{
			oldPage->SetName(templateName);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiHud::targetAtCursor                      (bool startStop)
{
	if ( CuiPreferences::getUseModelessInterface() )
	{
		targetAtCursorModeless( startStop );
	}
	else
	{
		targetAtCursorModal( startStop );
	}
}

void SwgCuiHud::targetAtCursorModeless( bool startStop )
{
	static float startTime = 0.f;

	if ( startStop )
	{
		startTime = Game::getElapsedTime();
	}
	else
	{
		if ( Game::getElapsedTime() - startTime < 0.25f )
		{
			CreatureObject * const player = Game::getPlayerCreature ();
			const UIPoint & mouseCoord = UIManager::gUIManager ().GetLastMouseCoord ();
			const UIWidget * const widgetUnderMouse = getPage ().GetWidgetFromPoint( mouseCoord, true );
			
			if (widgetUnderMouse == &getPage ())
			{
				Object * const selectedObject = m_lastSelectedObject.getPointer();

				if ( player && player != selectedObject )
				{
					if ( selectedObject && CuiIoWin::getMouseLookState() == CuiIoWin::MouseLookState_Disabled )
					{
						player->setLookAtTarget( selectedObject->getNetworkId() );

						if ( CuiIoWin::getLastMouseLookState() == CuiIoWin::MouseLookState_Avatar && CuiRadialMenuManager::isActive() == false )
						{
							CuiRadialMenuManager::createMenu( *selectedObject, mouseCoord );
						}
					}
					else if ( CuiPreferences::getTargetNothingUntargets() )
					{
						player->setLookAtTarget( NetworkId::cms_invalid );
					}
				}
			}
		}
	}
}

void SwgCuiHud::targetAtCursorModal( bool startStop )
{
	if (!m_targetingEnabled)
		return;

	CreatureObject * const player = Game::getPlayerCreature ();

	if (startStop)
	{
		const UIPoint & mouseCoord = UIManager::gUIManager ().GetLastMouseCoord ();
		const UIWidget * const widgetUnderMouse = getPage ().GetWidgetFromPoint( mouseCoord, true );
		
		if (widgetUnderMouse == &getPage ())
		{
			Object * const selectedObject = m_lastSelectedObject.getPointer ();
			
			if (player && player != selectedObject)
			{
				if (selectedObject)
				{
					const float curTime =  Game::getElapsedTime ();
					
					const float doubleClickTimeoutSecs = CuiIoWin::getDoubleClickTimeoutSecs ();
					
					if (m_radialMenuCountingDown && m_radialMenuObject == m_lastSelectedObject && 
						(curTime - m_radialMenuStartTime) < doubleClickTimeoutSecs)
					{
						CuiRadialMenuManager::clear ();
						CuiRadialMenuManager::performDefaultAction (*selectedObject);
					}
					else
					{
						CuiRadialMenuManager::touchCache (selectedObject->getNetworkId ());
						
						m_radialMenuObject = m_lastSelectedObject;
						m_radialMenuCountingDown = true;
						m_radialMenuStartTime      = curTime;
						m_radialMenuTimeout        = curTime + 0.25f;
						m_radialMenuTimerCursorPos = mouseCoord;
						player->setLookAtTarget (selectedObject->getNetworkId ());
						return;
					}
				}
				else 
				{
					if (CuiPreferences::getTargetNothingUntargets ())
						player->setLookAtTarget (NetworkId::cms_invalid);
				}
			}
		}
		m_radialMenuCountingDown = false;
	}
	else
	{
		CuiRadialMenuManager::clear ();
		m_radialMenuCountingDown = false;
	}
}

void SwgCuiHud::summonRadialMenu(const Unicode::String & params)
{
	if (CuiRadialMenuManager::isActive ())
		CuiRadialMenuManager::clear ();
	else
	{
		//-- don't allow radial menu on lookat target in space
		bool const shouldUseLookAtTarget = !Game::isHudSceneTypeSpace();
		Object * const obj = CuiAction::findObjectFromFirstParam (params, shouldUseLookAtTarget, false, CuiActions::radialMenu);

		if (obj)
		{
			UIPoint pt;
			pt = UIManager::gUIManager ().GetLastMouseCoord ();
			if (obj->isInWorld())
			{
				Camera * camera = Game::getCamera();
				UIPoint reticlePoint(UIManager::gUIManager ().GetLastMouseCoord ());
				static const float viewDistance = ConfigClientGame::getTargetingRange ();
				Vector worldStart = camera->getPosition_w ();
				Vector viewDirection = camera->rotate_o2w( camera->reverseProjectInScreenSpace (reticlePoint.x, reticlePoint.y));
				viewDirection.normalize ();
				const Vector worldEnd = worldStart + viewDirection * viewDistance;

				Vector offset = obj->getCollisionSphereExtent_w().getCenter().findClosestPointOnLine(worldStart, worldEnd) - obj->getCollisionSphereExtent_w().getCenter();

				if (offset.magnitude() < obj->getCollisionSphereExtent_w().getRadius() + 1.0f)
				{
					offset = camera->rotate_w2o(offset);
					IGNORE_RETURN(CuiRadialMenuManager::createMenu (*obj, pt, offset));
				}
				else
					IGNORE_RETURN(CuiRadialMenuManager::createMenu (*obj, pt, Vector(0, 0, 0)));
			}
			else
			{
				IGNORE_RETURN(CuiRadialMenuManager::createMenu (*obj, pt));
			}
		}
		else
		{
			SwgCuiInventory *inventory = SwgCuiInventory::findSelectedInventoryPage();
			if(inventory)
				inventory->openSelectedRadial();
		}
	}
}

void SwgCuiHud::setIntendedAndSummonRadialMenu(bool mouseUp, bool cancel)
{
	static bool tryingTargetAndRadial = false;
	static NetworkId mouseDownObject = NetworkId::cms_invalid;
	static bool canOpenRadial = true;

	if (cancel)
	{
		tryingTargetAndRadial = false;
		return;
	}

	NetworkId workingObjectId = NetworkId::cms_invalid;
	if (m_lastSelectedObject.getPointer())
		workingObjectId = m_lastSelectedObject.getPointer()->getNetworkId();

	if (!mouseUp)
	{
		// If menu is already up, close it and make sure we don't open it on the "up" call.
		tryingTargetAndRadial = true;
		canOpenRadial = true;
		targetAndRadialCursorPos = UIManager::gUIManager ().GetLastMouseCoord ();


		if (CuiRadialMenuManager::isActive ())
		{
			CuiRadialMenuManager::clear ();
			canOpenRadial = false;
		}

		// On down get what object the cursor was over...
		mouseDownObject = workingObjectId;
	}
	else 
	{
		// Mouse got dragged too much too consider this a click...
		if (!tryingTargetAndRadial)
			return;
		tryingTargetAndRadial = false;

		if (mouseDownObject == workingObjectId)
		{
			CreatureObject * const player = Game::getPlayerCreature ();
			if (!player)
				return;

			if (mouseDownObject == NetworkId::cms_invalid)
				canOpenRadial = false;

			// On up, assuming we're over the same object the button went down on...
			// Select intended target.
			// If target is a new target AND not attackable open radial menu.
			// If target isn't changing just open radial menu.
			const NetworkId currentIntended = player->getIntendedTarget();
			if (currentIntended != mouseDownObject)
			{
				if (CuiPreferences::getTargetNothingUntargets() || mouseDownObject != NetworkId::cms_invalid)
				{
					player->setLookAtTarget(mouseDownObject);
					player->setIntendedTarget(mouseDownObject);
					Object *intendedTargetObj = NetworkIdManager::getObjectById(mouseDownObject);
					if (intendedTargetObj && CuiRadialMenuManager::findDefaultAction(*intendedTargetObj) == Cui::MenuInfoTypes::COMBAT_ATTACK)
						canOpenRadial = false;
				}
			}
			
			if (canOpenRadial)
				IGNORE_RETURN(CuiActionManager::performAction (CuiActions::radialMenu, Unicode::emptyString));	
		}
	}
}


//----------------------------------------------------------------------

bool SwgCuiHud::getAppropriateScreenPointForObject  (const Object & obj, UIPoint & pt) const
{
	float dummy = 0.0f;
	if (!CuiObjectTextManager::canSee (obj, dummy))
		return false;

	GroundScene * const gs = dynamic_cast<GroundScene *>(Game::getScene ());
	const Camera * const camera = gs ? NON_NULL (gs->getCurrentCamera ()) : 0;	
	const TerrainObject * const terrainObject = TerrainObject::getInstance();
	
	if (!camera || !terrainObject)
		return false;
		
	UIRect rect;
	if (computeScreenRect (*camera, *terrainObject, obj, rect))
	{
		pt = rect.Location () + (rect.Size () / 2L);
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

void SwgCuiHud::setAction                     (SwgCuiHudAction * action)
{
	m_action = action;
}

//----------------------------------------------------------------------

void SwgCuiHud::setWindowManager              (SwgCuiHudWindowManager * windowManager)
{
	m_windowManager = windowManager;
}

//----------------------------------------------------------------------

CuiWorkspace & SwgCuiHud::getWorkspace                  ()
{
	return *NON_NULL(m_workspace);
}

//----------------------------------------------------------------------

bool SwgCuiHud::shouldRenderReticle() const
{
	return !UIManager::gUIManager ().GetDrawCursor () && !CuiManager::getPointerInputActive ();
}

//----------------------------------------------------------------------

std::string const & SwgCuiHud::getPlayerIdentificationString() const
{
	return m_playerIdentificationString;
}

//----------------------------------------------------------------------

std::string SwgCuiHud::createPlayerIdentificationString()
{
	std::string loginId;
	std::string clusterName;
	Unicode::String name;
	NetworkId id;
	IGNORE_RETURN(Game::getPlayerPath(loginId, clusterName, name, id));
	return loginId + "/" + clusterName + "/" + id.getValueString();	
}

//----------------------------------------------------------------------

SwgCuiHudWindowManager * SwgCuiHud::getWindowManager() const
{
	return m_windowManager;
}

//-----------------------------------------------------------------

Object * SwgCuiHud::getLastSelectedObject() const
{
	return m_lastSelectedObject.getPointer();
}

CachedNetworkId const & SwgCuiHud::getHudAssistId()
{
	const CreatureObject * const player = Game::getPlayerCreature ();
	
	if (player == NULL)
		return CachedNetworkId::cms_cachedInvalid;
	
	CreatureObject const * const lookAtCreatureObject = CreatureObject::asCreatureObject(player->getLookAtTarget().getObject());

	if (   (lookAtCreatureObject != NULL)
		&& lookAtCreatureObject->getState(States::Combat)
		&& player->isGroupedWith(*lookAtCreatureObject))
	{
		const CachedNetworkId & assistId = player->getAssistTarget();
		const Object * assistObject = assistId.getObject();
		if (assistObject != NULL && assistId != player->getNetworkId())
		{
			return assistId;
		}
	}
	
	return CachedNetworkId::cms_cachedInvalid;
}

//-----------------------------------------------------------------

