//======================================================================
//
// CuiRadialMenuManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================


#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiRadialMenuManager.h"

#include "clientGame/BuildingObject.h"
#include "clientGame/CellObject.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientImageDesignerManager.h"
#include "clientGame/ClientMissionObject.h"
#include "clientGame/ClientObjectUsabilityManager.h"
#include "clientGame/ClientPlayerQuestObject.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/DroidProgramSizeManager.h"
#include "clientGame/FactoryObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/GroundCombatActionManager.h"
#include "clientGame/GroupObject.h"
#include "clientGame/InstallationObject.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/ResourceContainerObject.h"
#include "clientGame/SaddleManager.h"
#include "clientGame/ShipObject.h"
#include "clientGame/VehicleObject.h"
#include "clientGraphics/Camera.h"
#include "clientObject/ReticleManager.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiConversationManager.h"
#include "clientUserInterface/CuiCraftManager.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediator.h"
#include "clientUserInterface/CuiMenuInfoHelper.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiMissionManager.h"
#include "clientUserInterface/CuiObjectTextManager.h"
#include "clientUserInterface/CuiPersistentMessageManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSecureTradeManager.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiVoiceChatManager.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/Production.h"
#include "sharedFoundation/Timer.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/PlatformFeatureBits.h"
#include "sharedGame/SharedFactoryObjectTemplate.h"
#include "sharedGame/SharedImageDesignerManager.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/SharedResourceContainerObjectTemplate.h"
#include "sharedMath/Sphere.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/MessageQueueObjectMenuRequest.h"
#include "sharedNetworkMessages/ObjectMenuSelectMessage.h" 
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/ArrangementDescriptor.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Controller.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedTerrain/TerrainObject.h"

#include "UIData.h"
#include "UIDataSource.h"
#include "UIDataSourceContainer.h"
#include "UIEventCallback.h"
#include "UIManager.h"
#include "UIPopupMenu.h"
#include "UIRadialMenu.h"
#include "UnicodeUtils.h"

#include <cstdio>
#include <map>
#include <list>

//-- @todo broken include
#include "swgSharedUtility/States.def"

//======================================================================

using namespace Cui::MenuInfoTypes;

namespace CuiRadialMenuManagerNamespace
{
	class CanBeManipulatedManager
	{
	public:
		CanBeManipulatedManager();
		~CanBeManipulatedManager();
		bool canBeManipulated(Object const * object) const;
		void registerCanBeManipulated(CuiRadialMenuManager::CanBeManipulatedFunction function);
	private:
		CanBeManipulatedManager(CanBeManipulatedManager const &);
		CanBeManipulatedManager & operator=(CanBeManipulatedManager const &);

	private:
		typedef std::vector<CuiRadialMenuManager::CanBeManipulatedFunction> Functions;
		Functions m_functions;
	};

	CanBeManipulatedManager cs_canBeManipulatedManager;

	//----------------------------------------------------------------------

	Vector s_radialOffset_c;
	bool s_updateByOffset;
	UIPoint s_originalPosition;

	//----------------------------------------------------------------------

	CuiMenuInfoHelper s_helper;

	typedef std::pair<NetworkId, uint8> RequestId;
	typedef stdmap<RequestId, uint16>::fwd PendingServerNotifyMap;
	PendingServerNotifyMap s_pendingServerNotifications;

	uint8 s_sequenceGlobal = 0;
	uint8 s_sequenceActive = 0;

	bool s_inHandler = false;

	uint32 const hash_mount = Crc::normalizeAndCalculate("mount");
	uint32 const hash_dismount = Crc::normalizeAndCalculate("dismount");
	uint32 const hash_crafting = Crc::normalizeAndCalculate("requestCraftingSession");


	typedef CuiRadialMenuManager::ObjectMenuRequestDataVector ObjectMenuRequestDataVector;

	struct ObjectMenuCacheData
	{
		uint8                       sequence;
		ObjectMenuRequestDataVector clientData;
		ObjectMenuRequestDataVector combinedData;
		float                       timestamp;
		bool                        ok;
		bool                        responsePending;
		int                         lastCounter;
		int                         lastCondition;
	};

	typedef stdmap<NetworkId, ObjectMenuCacheData>::fwd CacheMap;

	CacheMap s_cacheMap;

	struct PendingResponseInfo
	{
		NetworkId networkId;
		uint8 sequence;
		float timestamp;
	};
	std::list<PendingResponseInfo> s_pendingResponses;

	float s_cacheTimeout    = 30.0f;
	float s_responseTimeout = 5.0f;
	
	enum GroundTargetMode
	{
		GTM_None = 0,
		GTM_Storyteller,
		GTM_Grenade,

		GTM_TotalModes,
	};

	GroundTargetMode  s_groundTargetMode;
	NetworkId s_groundTargetTokenId;
	int   s_groundTargetMenuSelection;

	//----------------------------------------------------------------------

	void updateCache (const NetworkId & id, int sequence, const ObjectMenuRequestDataVector & combinedData)
	{
		const CacheMap::iterator it = s_cacheMap.find (id);
		if (it != s_cacheMap.end ())
		{
			ObjectMenuCacheData & cd = (*it).second;

			if (cd.sequence > sequence)
			{
				WARNING (true, ("CuiRadialMenuManager updateCache ignoring stale response [%d] for [%s], wanted [%d]", sequence, id.getValueString ().c_str (), cd.sequence));
				return;
			}

			Object * const obj = NetworkIdManager::getObjectById (id);
			ClientObject const * const clientObject = obj ? obj->asClientObject() : NULL;
			TangibleObject const * const tangible   = clientObject ? clientObject->asTangibleObject () : 0;
			cd.lastCounter     = tangible ? tangible->getCount ()     : 0;
			cd.lastCondition   = tangible ? tangible->getCondition () : 0;

			cd.timestamp       = Game::getElapsedTime ();
			cd.ok              = true;
			cd.responsePending = false;

			// some scripts may overide default commands like ITEM_USE.
			// we want to make sure that we only copy in the latest
			// command based off of type.

			cd.combinedData.clear();

			cd.combinedData.reserve(combinedData.size());
			ObjectMenuRequestDataVector::const_iterator ii = combinedData.begin();
			ObjectMenuRequestDataVector::const_iterator iiEnd = combinedData.end();

			for (; ii != iiEnd; ++ii)
			{
				ObjectMenuRequestDataVector::iterator jj = cd.combinedData.begin();
				ObjectMenuRequestDataVector::iterator jjEnd = cd.combinedData.end();

				ObjectMenuRequestData const & source = *ii;

				bool alreadyExists = false;

				for (; jj != jjEnd; ++jj)
				{
					ObjectMenuRequestData const & destination = *jj;

					if (source.m_menuItemType == destination.m_menuItemType)
					{
						alreadyExists = true;
						*jj = *ii;
					}
				}

				if (!alreadyExists)
				{
					cd.combinedData.push_back(*ii);
				}
			}

			return;
		}

		WARNING (true, ("CuiRadialMenuManager updateCache failed because no cache data exists for [%s]", id.getValueString ().c_str ()));
	}

	//----------------------------------------------------------------------

	/**
	* Find an object menu entry for the specified object, and request new data from the server if necessary.
	*
	* If the entry is not in the map at all, a request is sent to the server and FALSE is returned
	* If the entry is in the map:
	*    If the entry has no valid menu data, and is still waiting for a response, return FALSE
	*    If the entry has valid menu data, and the timestamp indicates the data is 50% towards expiration, request new data but return TRUE
	*    If the entry has valid menu data, and the timestamp has not otherwise expired expired, return TRUE
	*/

	bool findOrRequestCache (const NetworkId & id, const ObjectMenuRequestDataVector & clientData, float expireSeconds, ObjectMenuRequestDataVector & combinedData, uint8 & sequence)
	{
		bool retval = false;

		sequence = s_sequenceGlobal;

		if (!id.isValid () || Waypoint::getWaypointById(id).isValid ())
		{
			combinedData = clientData;
			return true;
		}

		Object * const obj = NetworkIdManager::getObjectById (id);
		ClientObject const * const clientObject = obj ? obj->asClientObject() : NULL;
		TangibleObject const * const tangible   = clientObject ? clientObject->asTangibleObject () : 0;

		if ((tangible != 0) && (!cs_canBeManipulatedManager.canBeManipulated(tangible)))
		{
			return true;
		}

		if (clientObject && clientObject->getContainedBy() && clientObject->getContainedBy()->getGameObjectType() == SharedObjectTemplate::GOT_misc_container_public)
		{
			combinedData = clientData;
			return true;
		}

		const float currentTime = Game::getElapsedTime ();
					
		const CacheMap::iterator it = s_cacheMap.find (id);
		if (it != s_cacheMap.end ())
		{
			ObjectMenuCacheData & cd = (*it).second;

			if (clientData == cd.clientData)
			{
				if (!cd.ok)
				{
					//-- still waiting for a response, don't send another request yet
					if (cd.responsePending)
					{
						sequence = cd.sequence;
						return false;
					}
				}
				else
				{
					//-- not expired yet
					if ((cd.timestamp + expireSeconds) > currentTime)
					{
						const int counter   = tangible ? tangible->getCount ()     : 0;
						const int condition = tangible ? tangible->getCondition () : 0;
						
						//-- if condition or counter changed, invalidate the menu
						if (((counter == 0 || cd.lastCounter == 0) && counter != cd.lastCounter) ||
							condition != cd.lastCondition)
						{
							cd.lastCounter   = counter;
							cd.lastCondition = condition;
						}
						else
						{
							
							combinedData = cd.combinedData;
							sequence     = cd.sequence;
							
							//-- 50% to expiration, go ahead and re-request a new menu without invalidating the current one
							const float preEmptiveExpireSeconds = 2.0f;
							if (!cd.responsePending && ((cd.timestamp + preEmptiveExpireSeconds) < currentTime))
							{
								retval = true;
							}
							//-- otherwise just return the data
							else
								return true;
						}
					}
				}
			}
			else
				cd.clientData = clientData;

			if (!retval)
				cd.ok = false;

			cd.sequence        = s_sequenceGlobal;
			cd.responsePending = true;
		}

		//- no entry in the cache map
		else
		{
			ObjectMenuCacheData cd;
			cd.ok              = false;
			cd.clientData      = clientData;
			cd.timestamp       = 0.0f;
			cd.sequence        = s_sequenceGlobal;
			cd.responsePending = true;
			cd.lastCounter     = tangible ? tangible->getCount ()     : 0;
			cd.lastCondition   = tangible ? tangible->getCondition () : 0;

			s_cacheMap [id] = cd;
		}

		// set up pending response entry
		{
			PendingResponseInfo responseInfo;
			responseInfo.networkId = id;
			responseInfo.sequence  = s_sequenceGlobal;
			responseInfo.timestamp = currentTime;
			s_pendingResponses.push_back(responseInfo);
		}

		if (Game::getSinglePlayer ())
		{
			ObjectMenuRequestDataVector myData = clientData;
			ObjectMenuRequestData data (250, 0, static_cast<uint8>(CLIENT_MENU_LAST + 1), Unicode::narrowToWide ("server test"), true, true);
			myData.push_back (data);
			updateCache (id, s_sequenceGlobal, myData);
		}
		else if(ClientObject::isFakeNetworkId(id))
		{
			updateCache (id, s_sequenceGlobal, clientData);		
		}		
		else
		{
			Object * const player = Game::getPlayer ();
			if(player->getController ())
			{
				MessageQueueObjectMenuRequest * const msg = new MessageQueueObjectMenuRequest(id, player->getNetworkId(), clientData, s_sequenceGlobal);
				static const uint32 flags = GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER;
				player->getController ()->appendMessage (CM_objectMenuRequest, 0.0f, msg, flags);
			}
		}

		return retval;
	}

	bool isInUsableRange(ClientObject const * const objectA, ClientObject const * const objectB, float const range)
	{
		bool inRange = false;

		if (objectA && objectB) 
		{
			float const combinedRadii = objectA->getAppearanceSphereRadius() + objectB->getAppearanceSphereRadius();
			float const magnitudeBetween = objectA->getPosition_w().magnitudeBetween(objectB->getPosition_w());

			inRange =  (magnitudeBetween - combinedRadii) < range;
		}

		return inRange;
	}

	bool canPickUp(SharedObjectTemplate::GameObjectType const got)
	{
		bool pickupable = true;

		if (got == SharedObjectTemplate::GOT_building                    ||
			got == SharedObjectTemplate::GOT_building_municipal          ||
			got == SharedObjectTemplate::GOT_building_player             || 
			got == SharedObjectTemplate::GOT_lair                        ||
			got == SharedObjectTemplate::GOT_camp                        ||
			got == SharedObjectTemplate::GOT_misc_furniture              ||
			got == SharedObjectTemplate::GOT_misc_crafting_station       ||
			got == SharedObjectTemplate::GOT_installation_factory        ||
			got == SharedObjectTemplate::GOT_installation_generator      ||
			got == SharedObjectTemplate::GOT_installation_harvester      ||
			got == SharedObjectTemplate::GOT_installation_turret         ||
			got == SharedObjectTemplate::GOT_terminal                    ||
			got == SharedObjectTemplate::GOT_vehicle                     ||
			got == SharedObjectTemplate::GOT_corpse                      ||
			GameObjectTypes::isTypeOf (got, SharedObjectTemplate::GOT_terminal) ||
			got == SharedObjectTemplate::GOT_terminal_bazaar             ||
			got == SharedObjectTemplate::GOT_terminal_cloning            ||
			got == SharedObjectTemplate::GOT_terminal_insurance          ||
			got == SharedObjectTemplate::GOT_terminal_mission            ||
			got == SharedObjectTemplate::GOT_terminal_player_structure   ||
			got == SharedObjectTemplate::GOT_terminal_shipping           ||
			got == SharedObjectTemplate::GOT_terminal_travel             ||
			got == SharedObjectTemplate::GOT_terminal_permissions        ||
			got == SharedObjectTemplate::GOT_terminal_manage
			)
		{
			pickupable = false;
		}

		return pickupable;
	}

	//////////////////////////////////////////////////////////////////////////
	// Debounce

	Timer s_doubleClickSpamTimer(0.20f);

	//////////////////////////////////////////////////////////////////////////
	// Get the intersect position.

	bool getObjectIntersectPosition_w(Object const * const object, Vector & position, Vector const & cameraPositionBegin_w, Vector const & cameraPositionEnd_w)
	{
		bool intersected = false;

		// Depth first search.
		int const children = object->getNumberOfChildObjects();

		for (int child = 0; child < children && !intersected; ++child) 
		{
			Object const * const childObject = object->getChildObject(child);
			intersected = getObjectIntersectPosition_w(childObject, position, cameraPositionBegin_w, cameraPositionEnd_w);
		}

		// No children intersected?  Check root object.
		if (!intersected)
		{
			Appearance const * const appearance = object->getAppearance();
			if (appearance) 
			{
				CollisionInfo colInfo;

				Vector const & cameraPositionBegin_o = object->rotateTranslate_w2o(cameraPositionBegin_w);
				Vector const & cameraPositionEnd_o = object->rotateTranslate_w2o(cameraPositionEnd_w);
				
				CollideParameters colParams(CollideParameters::Q_high, CollideParameters::T_collidable, false);
				
				if (appearance->collide(cameraPositionBegin_o, cameraPositionEnd_o, colParams, colInfo))
				{
					position = object->rotateTranslate_o2w(colInfo.getPoint());
					intersected = true;
				}
			}
		}

		return intersected;
	}

	//////////////////////////////////////////////////////////////////////////
	
	ShipObject* getFirstShipInDatapad(CreatureObject* player)
	{
		ShipObject* ship = 0;

		ClientObject const * const datapad = player->getDatapadObject();
		if (datapad)
		{
			VolumeContainer const * const datapadContainer = ContainerInterface::getVolumeContainer(*datapad);
			if(datapadContainer)
			{
				//for each item in the datapad, see if it contains a shipobject (that would make it a pcd)
				for(ContainerConstIterator i = datapadContainer->begin(); i != datapadContainer->end(); ++i)
				{
					Object const * const item = (*i).getObject();
					if(item)
					{
						Container const * const itemContainer = ContainerInterface::getContainer(*item);
						if(itemContainer)
						{
							//see if the item contains a shipobject
							for(ContainerConstIterator i2 = itemContainer->begin(); i2 != itemContainer->end(); ++i2)
							{
								Object * const o = (*i2).getObject();
								ClientObject * const co = o ? o->asClientObject() : NULL;
								ShipObject * const so = co ? co->asShipObject() : NULL;
								if(so)
								{
									ship = so;
									break;
								}
							}
						}
					}
				}
			}
		}

		return ship;
	}
}

using namespace CuiRadialMenuManagerNamespace;

//======================================================================

CanBeManipulatedManager::CanBeManipulatedManager()
: m_functions()
{
}

//----------------------------------------------------------------------

CanBeManipulatedManager::~CanBeManipulatedManager()
{
}


//----------------------------------------------------------------------

bool CanBeManipulatedManager::canBeManipulated(Object const * const object) const
{
	NOT_NULL(object);

	Functions::const_iterator ii = m_functions.begin();
	Functions::const_iterator iiEnd = m_functions.end();

	for (; ii != iiEnd; ++ii)
	{
		CuiRadialMenuManager::CanBeManipulatedFunction function = *ii;
		if (function != 0)
		{
			if (!function(object))
			{
				return false;
			}
		}
	}
	return true;
}

//----------------------------------------------------------------------

void CanBeManipulatedManager::registerCanBeManipulated(CuiRadialMenuManager::CanBeManipulatedFunction function)
{
	NOT_NULL(function);

	m_functions.push_back(function);
}

//======================================================================

struct CuiRadialMenuManager::Listener : public UIEventCallback
{

	//----------------------------------------------------------------------
	
	void OnHide( UIWidget * context)
	{
		NOT_NULL (context);
		
		if (context->IsA (TUIRadialMenu))
			CuiRadialMenuManager::onHideRadialMenu (safe_cast<UIRadialMenu *>(context));
		else if (context->IsA (TUIPopupMenu))
			CuiRadialMenuManager::onHidePopupMenu  (safe_cast<UIPopupMenu *>(context));
	}

	//----------------------------------------------------------------------

	void OnPopupMenuSelection( UIWidget * context)
	{
		NOT_NULL (context);
		CuiRadialMenuManager::OnPopupMenuSelection (context);
	}

};

//----------------------------------------------------------------------

namespace
{
	bool s_installed = false;

	CuiRadialMenuManager::Listener * s_listener = 0;

	const char * const RADIAL_STYLE        = "/styles.radialmenu.default.style";
	const char * const RADIAL_CENTER_PROTO = "/sample.pageRadial.center";

	Watcher<Object>  s_object;

	UIPoint s_radialStartPt;
}

//----------------------------------------------------------------------

UIRadialMenu * CuiRadialMenuManager::ms_radial;
UIPopupMenu *  CuiRadialMenuManager::ms_popup;
bool           CuiRadialMenuManager::ms_radialComplete;
uint8          CuiRadialMenuManager::ms_sequence;

//----------------------------------------------------------------------

void CuiRadialMenuManager::install ()
{
	DEBUG_FATAL (s_installed, ("already installed\n"));
	s_listener = new Listener;
	s_installed = true;
	s_object = 0;

	s_groundTargetMode = GTM_None;
	s_groundTargetTokenId = NetworkId::cms_invalid;
	s_groundTargetMenuSelection = 0;

	UIWidget * const w = safe_cast<UIWidget *>(UIManager::gUIManager ().GetObjectFromPath (RADIAL_CENTER_PROTO, TUIWidget));
	WARNING (!w, ("No such RADIAL_CENTER_PROTO [%s]", RADIAL_CENTER_PROTO));
	if (w)
	{
//		w->SetOpacity (0.0f);
//		w->SetEnabled (false);
	}

	GroundCombatActionManager::registerDefaultActionCallback(CuiRadialMenuManager::performDefaultAction);
}

//----------------------------------------------------------------------

void CuiRadialMenuManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));

	setRadial (0);

	delete s_listener;
	s_listener = 0;

	s_pendingServerNotifications.clear ();
	
	s_installed = false;

	GroundCombatActionManager::registerDefaultActionCallback(0);
}

//----------------------------------------------------------------------

void CuiRadialMenuManager::update()
{
	s_doubleClickSpamTimer.updateNoReset(Clock::frameTime());

	CreatureObject * const player = Game::getPlayerCreature();
	Object const * const object = s_object.getPointer();

	if (!s_pendingResponses.empty())
	{
		if (!player)
			s_pendingResponses.clear ();
		else
		{
			PendingResponseInfo & entry = s_pendingResponses.front();
			const float currentTime     = Game::getElapsedTime();
			
			if (currentTime - entry.timestamp >= s_responseTimeout)
			{
				if (!Game::getSinglePlayer())
				{
					const CacheMap::iterator i = s_cacheMap.find (entry.networkId);
					if (i != s_cacheMap.end())
					{
						const ObjectMenuCacheData & cacheData = (*i).second;
						
						if (cacheData.responsePending && cacheData.sequence == entry.sequence)
						{
							//-- dont refresh if the object is deleted
							if (NetworkIdManager::getObjectById (entry.networkId))
							{
								WARNING(true, ("CuiRadialMenuManager timing out and resending request with no response for [%s]", entry.networkId.getValueString ().c_str()));
								if (player->getController())
								{
									MessageQueueObjectMenuRequest * const msg = new MessageQueueObjectMenuRequest (entry.networkId, player->getNetworkId (), cacheData.clientData, entry.sequence);
									player->getController()->appendMessage(
										CM_objectMenuRequest,
										0.0f,
										msg,
										GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
									entry.timestamp = currentTime;
									return;
								}
							}
						}
					}
				}
				s_pendingResponses.erase (s_pendingResponses.begin());
			}
		}
	}


	// Update the radial menu location.
	if (ms_radial && object && object->isInWorld() && player && (player->getMountedCreature() != object))
	{
		Camera * camera = Game::getCamera();
		if (camera && s_updateByOffset) 
		{
			Vector worldPosition = object->getCollisionSphereExtent_w().getCenter();
			worldPosition += camera->rotate_o2w(s_radialOffset_c);

			Vector screenVect;
			if (camera->projectInWorldSpace(worldPosition, &screenVect.x, &screenVect.y, 0, false))
			{
				UIPoint screenPoint(static_cast<UIScalar>(screenVect.x), static_cast<UIScalar>(screenVect.y));
				ms_radial->Center(screenPoint);
				s_originalPosition = ms_radial->GetLocation();
			}
			else
			{
				clear();
			}

			if (ms_radial)
				UIManager::fitWidgetOnScreen (*ms_radial, UIManager::CWA_NW, true);
		}
	}
}

//----------------------------------------------------------------------

bool CuiRadialMenuManager::createMenu (Object & object, const UIPoint & pt, bool usePopup)
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));

	s_updateByOffset = false;

	if (&object == Game::getPlayer ())
		return false;

	s_helper.clear ();

	ClientObject * const clientObject = object.asClientObject();
	const int got = clientObject ? clientObject->getGameObjectType () : 0;

	if (populateMenu (s_helper, object, false))
	{
		updateRanges (s_helper, object);

		ClientObject const * const clientObjectContainer = clientObject ? clientObject->getContainedBy() : 0;
		bool const isInPublicContainer = clientObjectContainer ?
			clientObjectContainer->getGameObjectType() == SharedObjectTemplate::GOT_misc_container_public : false;

		//-- sending labels to the server has no effect, they will get stripped off anyway		
		if (!isInPublicContainer)
			s_helper.purgeLabels ();

		if (++s_sequenceGlobal == 0)
			++s_sequenceGlobal;

		//----------------------------------------------------------------------
		//-- setup the radial menu

		if (usePopup)
		{
			UIPage * const root = UIManager::gUIManager ().GetRootPage ();
			NOT_NULL (root);
			setPopup (new UIPopupMenu(root));
			NOT_NULL (ms_popup);
			ms_popup->SetStyle (root->FindPopupStyle ());
			s_helper.updatePopupMenu (*ms_popup, got);

			s_originalPosition = pt;
			
			ms_popup->SetLocation (pt);
			
			if (UIManager::gUIManager ().GetTopContextWidget ())
				UIManager::gUIManager ().PopContextWidgets (0);
			
			UIManager::gUIManager ().PushContextWidget (*ms_popup, UIManager::CWA_NW, true);
	
			//-- popups do not get the radial menu active hack treatment!
		}
		else
		{

			setRadial (new UIRadialMenu);
			NOT_NULL (ms_radial);
			ms_radial->SetProperty (UIRadialMenu::PropertyName::Style, Unicode::narrowToWide (RADIAL_STYLE));
			UIWidget * const w = safe_cast<UIWidget *>(UIManager::gUIManager ().GetObjectFromPath (RADIAL_CENTER_PROTO, TUIWidget));
			WARNING (!w, ("No such RADIAL_CENTER_PROTO [%s]", RADIAL_CENTER_PROTO));
			ms_radial->SetRadialCenterPrototype (w);
			
			UIWidget * const centerWidget = ms_radial->GetRadialCenterWidget ();
			if (centerWidget)
			{
				centerWidget->SetEnabled (false);
				centerWidget->SetEnabled (true);
			}

			s_helper.updateRadialMenu (*ms_radial, got);
			s_originalPosition = pt;
			
			ms_radial->SetLocation (pt);
			
			if (UIManager::gUIManager ().GetTopContextWidget ())
				UIManager::gUIManager ().PopContextWidgets (0);
			
			UIManager::gUIManager ().PushContextWidget (*ms_radial, UIManager::CWA_Center);
			
			CuiManager::getIoWin ().setRadialMenuActiveHack (true);
			CuiManager::requestPointer   (true);
		}
				
		s_object = &object;

		s_radialStartPt = pt;
				
		ObjectMenuRequestDataVector combinedData;
		ms_radialComplete = findOrRequestCache (object.getNetworkId (), s_helper.getDataVector (), s_cacheTimeout, combinedData, s_sequenceActive);

		if (ms_radialComplete)
		{
			mergeWithServerData (combinedData);
		}
	}

	return true;

}

bool CuiRadialMenuManager::createMenu (Object & object, const UIPoint & pt, const Vector & radialOffset_c, bool usePopup)
{
	bool retVal = createMenu(object, pt, usePopup);
	s_radialOffset_c = radialOffset_c;
	s_updateByOffset = true;
	return retVal;
}

//----------------------------------------------------------------------

//----------------------------------------------------------------------

CuiMenuInfoHelper * CuiRadialMenuManager::createMenu (Object & object, const UIPoint & pt, UIPopupMenu * pop)
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));

	s_updateByOffset = false;

	if (&object == Game::getPlayer ())
		return NULL;

	s_helper.clear ();

	ClientObject * const clientObject = object.asClientObject();
	const int got = clientObject ? clientObject->getGameObjectType () : 0;

	if (populateMenu (s_helper, object, false))
	{
		updateRanges (s_helper, object);

		ClientObject const * const clientObjectContainer = clientObject ? clientObject->getContainedBy() : 0;
		bool const isInPublicContainer = clientObjectContainer ?
			clientObjectContainer->getGameObjectType() == SharedObjectTemplate::GOT_misc_container_public : false;

		//-- sending labels to the server has no effect, they will get stripped off anyway		
		if (!isInPublicContainer)
			s_helper.purgeLabels ();

		if (++s_sequenceGlobal == 0)
			++s_sequenceGlobal;

		//----------------------------------------------------------------------
		//-- setup the radial menu

		setPopup (pop, false);
		
		s_helper.updatePopupMenu (*ms_popup, got);

		s_originalPosition = pt;

		if (UIManager::gUIManager ().GetTopContextWidget ())
			UIManager::gUIManager ().PopContextWidgets (0);

		s_object = &object;

		s_radialStartPt = pt;

		ObjectMenuRequestDataVector combinedData;
		ms_radialComplete = findOrRequestCache (object.getNetworkId (), s_helper.getDataVector (), s_cacheTimeout, combinedData, s_sequenceActive);

		if (ms_radialComplete)
		{
			mergeWithServerData (combinedData);
		}
	}

	return &s_helper;
}


//----------------------------------------------------------------------

namespace
{
	void setupCombatMenu (CuiMenuInfoHelper & helper, const TangibleObject & tangible)
	{
		CreatureObject * const player           = Game::getPlayerCreature ();
		if (!player)
			return;
		
		CreatureObject const * const creature = tangible.asCreatureObject();

		const int got = tangible.getGameObjectType ();

		if (creature && creature->isIncapacitated ())
		{
			if (creature->isPlayer ())
				helper.addRootMenu (COMBAT_DEATH_BLOW, got);
		}
		else
		{
			helper.addRootMenu (COMBAT_ATTACK, got);
		}
	}
}

//----------------------------------------------------------------------

bool CuiRadialMenuManager::populateMenu (CuiMenuInfoHelper & helper, const Object & object, bool defaultOnly)
{
	ClientObject const * const clientObject = object.asClientObject();

	if (!clientObject && !CuiPreferences::getAllowTargetAnything ())
		return false;

	TangibleObject const * const tangible     = clientObject ? clientObject->asTangibleObject() : NULL;
	IntangibleObject const * const intangible = dynamic_cast<const IntangibleObject *>(clientObject);
	CreatureObject const * const creature     = tangible ? tangible->asCreatureObject() : NULL;
	CreatureObject * const player             = Game::getPlayerCreature ();

	SharedObjectTemplate::GameObjectType got  = SharedObjectTemplate::GOT_none;
	if (clientObject)
		got = static_cast<SharedObjectTemplate::GameObjectType> (clientObject->getGameObjectType ());

	if (clientObject && clientObject->getContainedBy() && clientObject->getContainedBy()->getGameObjectType() == SharedObjectTemplate::GOT_misc_container_public)
	{
		std::string defaultUse1Text = clientObject->getContainedBy()->getObjectNameStringId().getCanonicalRepresentation() + "_use1";
		helper.addRootMenu(ITEM_PUBLIC_CONTAINER_USE1, StringId(defaultUse1Text).localize(), true);
		helper.addRootMenu(EXAMINE, got);
		return true;
	}

	const bool isVendor = (got == SharedObjectTemplate::GOT_vendor || (tangible && tangible->hasCondition (TangibleObject::C_vendor)));
	bool attackable                     = !isVendor;
	attackable                          = attackable && tangible != 0 && tangible->isAttackable ();
	attackable                          = attackable && (!creature || !creature->isDead ());
	attackable                          = attackable && got != SharedObjectTemplate::GOT_corpse;

	const bool isNestedInventory = tangible ? CuiInventoryManager::isNestedInventory (*tangible) : 0;
	const bool isNestedEquipped  = tangible ? CuiInventoryManager::isNestedEquipped (*tangible)  : 0;
	const bool isOnPlayer        = isNestedInventory || isNestedEquipped;
	bool const isBuildoutObject = object.getNetworkId() < NetworkId::cms_invalid;

	if ((tangible != 0) && (!cs_canBeManipulatedManager.canBeManipulated(tangible)))
	{
		helper.addRootMenu(EXAMINE, got);
		return true;
	}

	// Non-creature conversable items get a conversation menu item first
	if(tangible && !creature && tangible->hasCondition(TangibleObject::C_conversable))
	{
		helper.addRootMenu (CONVERSE_START, got);
	}

	// @todo: all these actions should be stored somewhere else to allow binding to them
		
	if (creature)
	{
		if (isVendor)
		{
			if (isOnPlayer)
				helper.addRootMenu (ITEM_DROP, got);
			else
				helper.addRootMenu (TERMINAL_AUCTION_USE, got);
			
			if (defaultOnly)
				return true;

			helper.addRootMenu (EXAMINE, got);

			if (isOnPlayer)
				helper.addRootMenu (ITEM_DROP, got);
		}

		//- dead creature
		else if (creature->isDead ())
		{
			helper.addRootMenu (LOOT, got);
			
			if (defaultOnly)
				return true;

			helper.addRootMenu (EXAMINE, got);
		}

		//- not dead
		else
		{
			//-- converse can't be the default on aggressive creatures
			bool needs_converse_menu = creature->hasCondition(TangibleObject::C_conversable);
			if (needs_converse_menu)
			{
				if (!creature->hasCondition(TangibleObject::C_aggressive))
				{
					helper.addRootMenu (CONVERSE_START, got);
					needs_converse_menu = false;
				}
			}

			//-- attackable
			if (attackable)
			{
				setupCombatMenu (helper, *creature);

				// We now allowing trading with attackable players
				if (creature->isPlayer ())
				{
					helper.addRootMenu (TRADE_START, got);
				}
			}			
			//-- not attackable
			else
			{
				if (creature->isPlayer ())
				{
					helper.addRootMenu (TRADE_START, got);

					if (defaultOnly)
						return true;

					if (!Game::isTutorial())
					{
						helper.addRootMenu (COMBAT_DUEL, got);
					}
				}
				else
				{
					if (isOnPlayer)
					{
						helper.addRootMenu (PET_CALL, got);
						if (defaultOnly)
							return true;
					}
				}
			}

			if (needs_converse_menu)
			{
				helper.addRootMenu (CONVERSE_START, got);
				needs_converse_menu = false;
			}

			if (!isBuildoutObject) 
			{
				helper.addRootMenu (EXAMINE, got);
			}

			//gate based on image design skillmods
			if (creature->isPlayer () || ConfigClientUserInterface::getTestImageDesignWithNpcs ())
			{
				//gate base on group (must be in a group with the target)
				if (player->getGroup().isValid () && creature->getGroup() == player->getGroup())
				{
					SharedImageDesignerManager::SkillMods const skillMods = ClientImageDesignerManager::getSkillModsForDesigner(player->getNetworkId());
					if(skillMods.bodySkillMod > 0 || skillMods.faceSkillMod > 0 || skillMods.markingsSkillMod > 0 || skillMods.hairSkillMod > 0)
					{
						helper.addRootMenu (IMAGEDESIGN, got);
					}
				}
			}

			//----------------------------------------------------------------------
			//-- group stuff
			
			if (creature->isPlayer () || ConfigClientUserInterface::getTestGroupWithNpcs ())
			{
				const CachedNetworkId groupId (player->getGroup ());
				const GroupObject * const group  = safe_cast<const GroupObject *>(groupId.getObject ());
				
				int const examineCharacterSheetId = helper.addRootMenu(EXAMINE_CHARACTERSHEET, got);
				helper.addSubMenu(examineCharacterSheetId, COLLECTIONS, got);

				if (!group)
				{
					if (player->getGroupInviter().first.first == creature->getNetworkId ())
					{
						helper.addRootMenu (GROUP_JOIN, got);
						helper.addRootMenu (GROUP_DECLINE, got);
					}
					else
						helper.addRootMenu (GROUP_INVITE, got);
				}
				else
				{
					bool isLeader = false;
					group->findMember (player->getNetworkId (), isLeader);
					
					bool isOtherLeader = false;
					const GroupObject::GroupMember * const otherGroupMember = group->findMember (creature->getNetworkId (), isOtherLeader);
					
					if (otherGroupMember)
					{
						if ((!creature->isRidingMount()) && (player->isRidingMount()))
						{
							CreatureObject const * const mount = player->getMountedCreature();
							int const capacity = (mount != 0) ? SaddleManager::getSaddleSeatingCapacity(*mount) : 0;
							if (capacity > 1)
							{
								helper.addRootMenu(VEHICLE_OFFER_RIDE, got);
							}
						}

						if (isLeader)
						{
							const int group_kick_id = helper.addRootMenu (GROUP_KICK, got);
							helper.addSubMenu (group_kick_id, GROUP_DISBAND, got);
						}
					}
					else
					{
						helper.addRootMenu (GROUP_INVITE, got);
					}
				}
			}

			if(creature->isPlayer() && CuiVoiceChatManager::isLoggedIn())
			{
				helper.addRootMenu(VOICE_INVITE, got);
			}

			if (Game::getSinglePlayer ())
			{
				const int conv_start = helper.addRootMenu (CONVERSE_START, got);
				helper.addSubMenu (conv_start, CONVERSE_STOP, got);
			}

			/*
			//-- Handle population of auto follow commands.
			const PlayerCreatureController *const playerController = safe_cast<const PlayerCreatureController*> (player->getController ());
			NOT_NULL (playerController);

			if (playerController->getAutoFollowTarget () == creature)
				helper.addRootMenu (CREATURE_STOP_FOLLOW, got);
			else
			{
				// Allow following when selected creature is in the same group as the player
				// or when the creature is the current combat target for the player.
				bool allowFollow = (player->getCombatTarget().getObject () == static_cast<const Object*>(creature));
				if (!allowFollow)
				{
					const CachedNetworkId &creatureGroup = creature->getGroup ();
					const CachedNetworkId &playerGroup   = player->getGroup ();

					allowFollow = (creatureGroup != CachedNetworkId::cms_invalid) && (playerGroup != CachedNetworkId::cms_invalid) && (creatureGroup == playerGroup);
				}

				if (allowFollow)
					helper.addRootMenu (CREATURE_FOLLOW, got);
			}
			*/
		}
	}

	//----------------------------------------------------------------------
	//-- tangible

	else if (tangible)
	{

		bool pickupable = true;

		//----------------------------------------------------------------------

		if (got == SharedObjectTemplate::GOT_building                    ||
			got == SharedObjectTemplate::GOT_building_municipal          ||
			got == SharedObjectTemplate::GOT_building_player             || 
			got == SharedObjectTemplate::GOT_lair                        ||
			got == SharedObjectTemplate::GOT_camp                        ||
			got == SharedObjectTemplate::GOT_misc_furniture              ||
			got == SharedObjectTemplate::GOT_misc_crafting_station       ||
			got == SharedObjectTemplate::GOT_installation_factory        ||
			got == SharedObjectTemplate::GOT_installation_generator      ||
			got == SharedObjectTemplate::GOT_installation_harvester      ||
			got == SharedObjectTemplate::GOT_installation_turret         ||
			got == SharedObjectTemplate::GOT_terminal                    ||
			got == SharedObjectTemplate::GOT_vehicle                     ||
			got == SharedObjectTemplate::GOT_corpse                      ||
			GameObjectTypes::isTypeOf (got, SharedObjectTemplate::GOT_terminal) ||
			got == SharedObjectTemplate::GOT_terminal_bazaar             ||
			got == SharedObjectTemplate::GOT_terminal_cloning            ||
			got == SharedObjectTemplate::GOT_terminal_insurance          ||
			got == SharedObjectTemplate::GOT_terminal_mission            ||
			got == SharedObjectTemplate::GOT_terminal_player_structure   ||
			got == SharedObjectTemplate::GOT_terminal_shipping           ||
			got == SharedObjectTemplate::GOT_terminal_travel             ||
			got == SharedObjectTemplate::GOT_terminal_permissions        ||
			got == SharedObjectTemplate::GOT_terminal_manage
			)
		{
			if (attackable)
			{
				setupCombatMenu (helper, *tangible);
			}

			pickupable = false;
		}

		pickupable = pickupable && !isOnPlayer;

		//-- containers

		if (got == SharedObjectTemplate::GOT_corpse)
		{
			const int loot_id = helper.addRootMenu (LOOT_ALL, got);

			if (defaultOnly)
				return true;

			helper.addSubMenu (loot_id, LOOT, got);
		}

		else if (got == SharedObjectTemplate::GOT_terminal_bazaar || got == SharedObjectTemplate::GOT_vendor || tangible->hasCondition (TangibleObject::C_vendor))
		{
			if (!isOnPlayer)
			{
				helper.addRootMenu (TERMINAL_AUCTION_USE, got);
				if (defaultOnly)
					return true;
			}
		}
		//-- note that the auction/vendor sell window does not use this code to open items
		else if (   (   got == SharedObjectTemplate::GOT_misc_container
		             || got == SharedObjectTemplate::GOT_misc_container_wearable
		             || got == SharedObjectTemplate::GOT_misc_container_ship_loot)
		         && ContainerInterface::getVolumeContainer(*tangible))
		{
			helper.addRootMenu (ITEM_OPEN, got);
			
			if (defaultOnly)
				return true;			
		}

		const Object * const containedByObject = ContainerInterface::getContainedByObject (*tangible);
		const bool isInWorld = !containedByObject || containedByObject->getCellProperty ();
		const bool allowPickups = ConfigClientUserInterface::getAllowRadialMenuPickup ();

		if(containedByObject && containedByObject->asClientObject() && ( containedByObject->asClientObject()->getGameObjectType() == SharedObjectTemplate::GOT_chronicles_quest_holocron || containedByObject->asClientObject()->getGameObjectType() == SharedObjectTemplate::GOT_chronicles_quest_holocron_recipe ) )
		{
			helper.addRootMenu(EXAMINE, got);

			return true;
		}

		if(got == SharedObjectTemplate::GOT_misc_furniture)
		{
			if(!isInWorld && !isOnPlayer)
				pickupable = true;
		}

		if ((pickupable && !isInWorld) || allowPickups)
		{
			pickupable  = false;
			bool isLooting = false;

			if (clientObject)
			{
				GroupObject const * const group = safe_cast<GroupObject const *>(player->getGroup().getObject());

				if (group)
				{
					//if we are in a group and the group mode is lottery or random then we
					//need to call the loot function, otherwise we just grab the one item 
					//we are selecting
					if (group->getLootRule() == GroupObject::LR_lottery || group->getLootRule() == GroupObject::LR_random)
					{

						ClientObject const * const clientObjectParent = clientObject->getContainedBy();

						if (clientObjectParent)
						{
							ClientObject const * const clientObjectGrandparent = clientObjectParent->getContainedBy();

							if (clientObjectGrandparent)
							{
								CreatureObject const * const creature = clientObjectGrandparent->asCreatureObject();
							
								if (creature && creature->isDead())
								{
									helper.addRootMenu(LOOT, got);
									isLooting = true;
								}
							}
						}
					}
				}
			}
			
			if (isLooting == false)
				helper.addRootMenu(ITEM_PICKUP, got);

			if (defaultOnly)
				return true;
		}

		if (tangible->getObjectType() == SharedResourceContainerObjectTemplate::SharedResourceContainerObjectTemplate_tag)
		{
			ResourceContainerObject const *resourceContainerObject = safe_cast<ResourceContainerObject const *>(tangible);

			if ((resourceContainerObject != NULL) &&
			    (resourceContainerObject->getQuantity() > 1))
			{
				helper.addRootMenu (SPLIT, got);
			}
		}

		//----------------------------------------------------------------------

		if (got == SharedObjectTemplate::GOT_misc_container_wearable ||
			got == SharedObjectTemplate::GOT_weapon_heavy_special ||
			got == SharedObjectTemplate::GOT_weapon_ranged_misc ||
			got == SharedObjectTemplate::GOT_weapon_ranged_pistol ||
			got == SharedObjectTemplate::GOT_weapon_ranged_carbine ||
			got == SharedObjectTemplate::GOT_weapon_ranged_rifle ||
			got == SharedObjectTemplate::GOT_weapon_melee_misc ||
			got == SharedObjectTemplate::GOT_weapon_melee_1h ||
			got == SharedObjectTemplate::GOT_weapon_melee_2h ||
			got == SharedObjectTemplate::GOT_weapon_melee_polearm ||
			GameObjectTypes::isTypeOf (got, SharedObjectTemplate::GOT_armor) ||
			GameObjectTypes::isTypeOf (got, SharedObjectTemplate::GOT_clothing) ||
			GameObjectTypes::isTypeOf (got, SharedObjectTemplate::GOT_jewelry) ||
			got == SharedObjectTemplate::GOT_misc_instrument ||
			got == SharedObjectTemplate::GOT_misc_fishing_pole
			)
		{
			if (isOnPlayer || Game::getSinglePlayer ())
			{
				if (!player->isIncapacitated() && !player->isDead())
				{
					const SlottedContainmentProperty * const slottedProperty = ContainerInterface::getSlottedContainmentProperty(*tangible);
					if (slottedProperty && slottedProperty->getNumberOfArrangements () > 0)
					{
						if (CuiInventoryManager::canPotentiallyEquip (*tangible, *player))
						{
							//-- don't allow equip for objects in the world, since you generally can't drop them back
							ClientObject * appearanceInventory = CuiInventoryManager::getPlayerAppearanceInventory();
							TangibleObject * tangibleAppearance = NULL;

							if(appearanceInventory)
								tangibleAppearance = appearanceInventory->asTangibleObject();

							if (isInWorld && !helper.getDefault ())
								helper.addRootMenu (EXAMINE, got);
							
							if (CuiInventoryManager::itemIsEquipped (*tangible, *player))
								helper.addRootMenu (ITEM_UNEQUIP, got);
							else if (tangibleAppearance && CuiInventoryManager::itemIsEquipped(*tangible, *tangibleAppearance))
							{
								if(	(GameObjectTypes::isTypeOf (got, SharedObjectTemplate::GOT_armor) ||	GameObjectTypes::isTypeOf (got, SharedObjectTemplate::GOT_clothing) ||
									GameObjectTypes::isTypeOf (got, SharedObjectTemplate::GOT_jewelry) || got == SharedObjectTemplate::GOT_misc_container_wearable ))
									helper.addRootMenu(ITEM_UNEQUIP_APPEARANCE, got);
							}
							else
							{
								helper.addRootMenu (ITEM_EQUIP, got);

								if( tangibleAppearance && ( GameObjectTypes::isTypeOf (got, SharedObjectTemplate::GOT_armor) ||	GameObjectTypes::isTypeOf (got, SharedObjectTemplate::GOT_clothing)||
									GameObjectTypes::isTypeOf (got, SharedObjectTemplate::GOT_jewelry) || got == SharedObjectTemplate::GOT_misc_container_wearable) && ClientObjectUsabilityManager::canWear(player, tangible))
								{
									if(CuiInventoryManager::canPotentiallyEquip(*tangible, *tangibleAppearance))
										helper.addRootMenu(ITEM_EQUIP_APPEARANCE, got);
								}
							}
							
							if (defaultOnly)
								return true;
						}
					}
				}
			}
		}

		if(got == SharedObjectTemplate::GOT_misc_appearance_only || got == SharedObjectTemplate::GOT_misc_appearance_only_invisible)
		{
			if (isOnPlayer || Game::getSinglePlayer ())
			{
				if (!player->isIncapacitated() && !player->isDead())
				{
					const SlottedContainmentProperty * const slottedProperty = ContainerInterface::getSlottedContainmentProperty(*tangible);
					if (slottedProperty && slottedProperty->getNumberOfArrangements () > 0)
					{
						ClientObject * appearanceInventory = CuiInventoryManager::getPlayerAppearanceInventory();
						TangibleObject * tangibleAppearance = NULL;

						if(appearanceInventory)
							tangibleAppearance = appearanceInventory->asTangibleObject();
						if (tangibleAppearance && CuiInventoryManager::itemIsEquipped(*tangible, *tangibleAppearance))
						{
								helper.addRootMenu(ITEM_UNEQUIP_APPEARANCE, got);
						}
						else if (tangibleAppearance && CuiInventoryManager::canPotentiallyEquip (*tangible, *tangibleAppearance) && ContainerInterface::getContainer(*tangible) == NULL)
						{
							helper.addRootMenu(ITEM_EQUIP_APPEARANCE, got);
						}
					}
				}
			}
		}

		//----------------------------------------------------------------------
		//-- 

		switch (got)
		{

		case SharedObjectTemplate::GOT_tool_crafting:
			{
				if (isOnPlayer)
				{
					helper.addRootMenu (CRAFT_START, got);

					if (defaultOnly)
						return true;
				}
			}
			break;

		case SharedObjectTemplate::GOT_misc_factory_crate:
			{
				if (isOnPlayer)
				{
					helper.addRootMenu (EXTRACT_OBJECT, got);

					if (defaultOnly)
						return true;

					FactoryObject const * factoryObject = safe_cast<const FactoryObject *>(tangible);
					if ((factoryObject != NULL) &&
						(factoryObject->getCount() > 1))
					{
						helper.addRootMenu (SPLIT, got);
					}
				}
			}
			break;

		case SharedObjectTemplate::GOT_misc_furniture:
			{
				if (!isOnPlayer && tangible->canSitOn () && (player->getClosestChair () == clientObject))
				{
					helper.addRootMenu (ITEM_SIT, got);
					if (defaultOnly)
						return true;
				}
			}
			break;

		case SharedObjectTemplate::GOT_deed:
		case SharedObjectTemplate::GOT_deed_building:
		case SharedObjectTemplate::GOT_deed_installation:
		case SharedObjectTemplate::GOT_misc_drink:
		case SharedObjectTemplate::GOT_misc_food:
		case SharedObjectTemplate::GOT_misc_food_pet:
		case SharedObjectTemplate::GOT_misc_item_usable:
		case SharedObjectTemplate::GOT_misc_ground_target:
		case SharedObjectTemplate::GOT_misc_blueprint:
		case SharedObjectTemplate::GOT_misc_pharmaceutical:
		case SharedObjectTemplate::GOT_misc_ticket_travel:
		case SharedObjectTemplate::GOT_misc_trap:
		case SharedObjectTemplate::GOT_tool:
		case SharedObjectTemplate::GOT_tool_camp_kit:
		case SharedObjectTemplate::GOT_tool_repair:
		case SharedObjectTemplate::GOT_tool_survey:
		case SharedObjectTemplate::GOT_weapon_heavy_misc:
		case SharedObjectTemplate::GOT_weapon_ranged_thrown:
		case SharedObjectTemplate::GOT_misc_firework:
		case SharedObjectTemplate::GOT_misc_tcg_card:
			{
				if (isOnPlayer)
				{
					if (!player->isIncapacitated() && !player->isDead())
					{
						helper.addRootMenu (ITEM_USE, got);
						if (defaultOnly)
							return true;
					}
				}
			}
			break;

		case SharedObjectTemplate::GOT_deed_vehicle:
			{
				if (isOnPlayer)
				{
					if (!player->isIncapacitated() && !player->isDead())
					{
						helper.addRootMenu (VEHICLE_GENERATE, got);
						if (defaultOnly)
							return true;
					}
				}
			}
			break;

			//-- non-equippable instruments
		case SharedObjectTemplate::GOT_misc_instrument:
			{
				if (!isOnPlayer && !player->isIncapacitated() && !player->isDead())
				{
					const SlottedContainmentProperty * const slottedProperty = ContainerInterface::getSlottedContainmentProperty(*tangible);
					if (!slottedProperty || slottedProperty->getNumberOfArrangements () <= 0 || !CuiInventoryManager::canPotentiallyEquip (*tangible, *player))
					{
						helper.addRootMenu (ITEM_USE, got);
						if (defaultOnly)
							return true;
					}
				}
			}
			
			break;

		case SharedObjectTemplate::GOT_misc_sign:
		case SharedObjectTemplate::GOT_terminal_bank:
		case SharedObjectTemplate::GOT_terminal_cloning:
		case SharedObjectTemplate::GOT_terminal_insurance:
		case SharedObjectTemplate::GOT_terminal_manage:
		case SharedObjectTemplate::GOT_terminal_misc:
		case SharedObjectTemplate::GOT_terminal_permissions:
		case SharedObjectTemplate::GOT_terminal_shipping:
		case SharedObjectTemplate::GOT_terminal_travel:
		case SharedObjectTemplate::GOT_terminal_space:
			{
				if (!player->isIncapacitated() && !player->isDead())
				{
					helper.addRootMenu (ITEM_USE, got);
					if (defaultOnly)
						return true;
				}
			}
			break;

		case SharedObjectTemplate::GOT_terminal_space_npe:
			{
				if (!player->isIncapacitated() && !player->isDead())
				{
					ShipObject* shipToManage = getFirstShipInDatapad( player );

					if (shipToManage != 0)
					{
						helper.addRootMenu (ITEM_USE, got);
						if (defaultOnly)
							return true;
	
						helper.addRootMenu (SHIP_MANAGE_COMPONENTS, got);
					}
				}
			}
			break;

		case  SharedObjectTemplate::GOT_misc_fishing_pole:
			{
				if (ContainerInterface::getVolumeContainer (*tangible))
				{
					const int id = helper.addRootMenu (ITEM_OPEN, got);
					
					if (defaultOnly)
						return true;
					
					if (isOnPlayer)
						helper.addSubMenu (id, ITEM_OPEN_NEW_WINDOW, got);
				}
			}
			break;
			
			
		case SharedObjectTemplate::GOT_terminal_mission:
			{
				if (!player->isIncapacitated() && !player->isDead())
				{
					helper.addRootMenu (TERMINAL_MISSION_LIST, got);
					if (defaultOnly)
						return true;
				}
			}
			break;

		case SharedObjectTemplate::GOT_misc_force_crystal:
			{
				helper.addRootMenu (ITEM_USE, got, true);
				helper.addRootMenu (EXAMINE, got);
				helper.addRootMenu (ITEM_DESTROY, got);
					
			}
			break;
			
		}
		
		const int examineId = helper.addRootMenu (EXAMINE, got);
		
		if (Game::getSinglePlayer ())
		{
			helper.addSubMenu (examineId, PET_CALL, got);
			helper.addSubMenu (examineId, CONVERSE_START, got);
		}

		if (defaultOnly)
			return true;
				
		else if (isOnPlayer)
		{
			bool appearanceItem = ContainerInterface::getContainedByObject(*tangible) == player->getAppearanceInventoryObject();
			const CellProperty * const parentCell = player->getParentCell ();
			if (parentCell && parentCell != CellProperty::getWorldCellProperty () && !appearanceItem)
				helper.addRootMenu (ITEM_DROP, got);
			
			if(!appearanceItem)
				helper.addRootMenu (ITEM_DESTROY, got);
		}

		else if (player)
		{
			const ClientObject * const bank = player->getBankObject ();
			if (bank && CuiInventoryManager::isNestedInContainer (*tangible, *bank, 0))
				helper.addRootMenu (ITEM_DESTROY, got);
		}


	}

	//----------------------------------------------------------------------
	//-- intangible
	
	else if (intangible)
	{
		if (got == SharedObjectTemplate::GOT_data_vehicle_control_device
			|| got == SharedObjectTemplate::GOT_data_pet_control_device
			|| got == SharedObjectTemplate::GOT_data_droid_control_device)
		{
			int count = intangible->getCount();
			if (count == 0)
			{
				helper.addRootMenu (got == SharedObjectTemplate::GOT_data_vehicle_control_device ? VEHICLE_GENERATE : PET_CALL, got, true);
			}
			else if (count == 1)
			{
				helper.addRootMenu (got == SharedObjectTemplate::GOT_data_vehicle_control_device ? VEHICLE_STORE : PET_STORE, got, true);		
			}
			else
			{
				DEBUG_WARNING(true, ("Invalid count variable on a pet/vehicle control device: %d\n", count));
			}
			helper.addRootMenu (ITEM_DESTROY, got);
			helper.addRootMenu (EXAMINE,      got);	

			if(got == SharedObjectTemplate::GOT_data_droid_control_device)
			{
				if(Game::getGameFeatureBits() & ClientGameFeature::SpaceExpansionRetail)
				{
					if(!Game::getPlayerPilotedShip())
					{
						if(clientObject)
						{
							if(DroidProgramSizeManager::isDroidProgrammable(*clientObject))
							{
								if(player && player->isAPilot())
								{
									helper.addRootMenu (PROGRAM_DROID, got);
								}
							}
						}
					}
				}
			}

			return true;
		}
		const ClientMissionObject * const mission = dynamic_cast<const ClientMissionObject *>(&object);
		if (mission)
		{
			helper.addRootMenu (MISSION_DETAILS, got);

			const Container* c = ContainerInterface::getContainer(*mission);
			for (ContainerConstIterator i = c->begin(); i != c->end(); ++i)
			{
				Container::ContainedItem item = *i;
				Object * const o = item.getObject();
				ClientWaypointObject* const w = dynamic_cast<ClientWaypointObject*>(o);
				if (w && w->getColor() != Waypoint::getColorNameById(Waypoint::Invisible))
				{
					// Some missions (such as space missions) don't have a valid waypoint.
					// Add the "activate" option only if the waypoint is valid:
					if (!w->getPlanetName().empty())
					{
						if (w->isWaypointActive ())
							helper.addRootMenu (ITEM_DEACTIVATE, got);
						else
							helper.addRootMenu (ITEM_ACTIVATE, got);
						break;
					}
				}
			}

			if (mission->isDuty())
				helper.addRootMenu (MISSION_END_DUTY, got);
			else
				helper.addRootMenu (MISSION_ABORT, got);
			return true;
		}

		bool isUnownedWaypoint = false;

		const ClientWaypointObject * const waypoint = dynamic_cast<const ClientWaypointObject *>(&object);
		if (waypoint)
		{
			if (waypoint->isWaypointActive ())
				helper.addRootMenu (ITEM_DEACTIVATE, got);
			else
				helper.addRootMenu (ITEM_ACTIVATE, got);

			if ((waypoint->getPlanetName() == Game::getNonInstanceSceneId())				
				&& Game::getPlayerPilotedShip())
			{
				helper.addRootMenu (WAYPOINT_AUTOPILOT, got);
			}

			if (defaultOnly)
				return true;

			if(!waypoint->isPoi() && !waypoint->isGroupWaypoint())
			{
				helper.addRootMenu(SET_NAME, got);

				int const setColorMenuId = helper.addRootMenu(ITEM_WP_SETCOLOR, got);
				helper.addSubMenu(setColorMenuId, ITEM_WP_SETCOLOR_BLUE, got);
				helper.addSubMenu(setColorMenuId, ITEM_WP_SETCOLOR_GREEN, got);
				helper.addSubMenu(setColorMenuId, ITEM_WP_SETCOLOR_ORANGE, got);
				helper.addSubMenu(setColorMenuId, ITEM_WP_SETCOLOR_PURPLE, got);
				helper.addSubMenu(setColorMenuId, ITEM_WP_SETCOLOR_WHITE, got);
				helper.addSubMenu(setColorMenuId, ITEM_WP_SETCOLOR_YELLOW, got);

				if (CuiPersistentMessageManager::getCompositionWindowCount () > 0)
				{
					helper.addRootMenu (ITEM_MAIL, got);
				}
			}
			else
				isUnownedWaypoint = true;
		}

		//allow bringing up ship page on ground only for players, in ground or space for gods
		if((got == SharedObjectTemplate::GOT_data_ship_control_device) && (!Game::isSpace() || PlayerObject::isAdmin()))
		{
			helper.addRootMenu (SHIP_MANAGE_COMPONENTS, got);
		}

		if (!isUnownedWaypoint)
		{

			helper.addRootMenu (EXAMINE,      got);
			if (got != SharedObjectTemplate::GOT_data_house_control_device)
				helper.addRootMenu (ITEM_DESTROY, got);

		}
	}
	else
	{
		helper.addRootMenu (EXAMINE,      got);
	}

	return true;
}

//----------------------------------------------------------------------

void CuiRadialMenuManager::performDefaultAction (Object & object, bool allowOpenOnly, int typeToUse, bool fromButton)
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));

	if (&object == Game::getPlayer ())
		return;

	// RH 1/9/07
	// save the old s_object, so that we can restore it when we're done here.
	// this prevents macros calling this method (or anything else) from breaking
	// existing radial dialogs.

	Object *obj = s_object.getPointer();
	s_object = &object;
		
	CuiMenuInfoHelper helper;
	if (populateMenu (helper, object, false))
	{
		updateRanges (helper, object);

		//-- sending labels to the server has no effect, they will get stripped off anyway		
		helper.purgeLabels ();

		if (++s_sequenceGlobal == 0)
			++s_sequenceGlobal;
			
		ObjectMenuRequestDataVector combinedData;
		uint8 mySequence = 0;
		const bool radialComplete = findOrRequestCache (object.getNetworkId (), helper.getDataVector (), s_cacheTimeout, combinedData, mySequence);

		if (radialComplete)
			helper = CuiMenuInfoHelper(combinedData);

		const ObjectMenuRequestData * omrd = 0;
		
		if (typeToUse == 0)
			omrd = helper.getDefault ();
		else
			omrd = helper.findDataByType (typeToUse);

		if (omrd)
		{
			// rls - move this information into the command table.

			if (performDefaultDoubleClickAction(object, false))
			{
				s_object = obj;
				return;
			}

			if (omrd->m_menuItemType == EXAMINE)
			{
				//DO NOTHING
			}

			else if (omrd->m_menuItemType == COMBAT_ATTACK)
			{
				//DO NOTHING
			}

			else if (omrd->m_menuItemType == ITEM_PUBLIC_CONTAINER_USE1 && !fromButton && CuiPreferences::getNewVendorDoubleClickExamine())
			{
				Cui::MenuInfoTypes::executeCommandForMenu(EXAMINE, object.getNetworkId(), 0);
				return;
			}

			else if (omrd->isOutOfRange ())
			{
				ClientObject const * const clientObject = object.asClientObject();
				if (clientObject)
				{
					Unicode::String label = omrd->m_label;
					if (label.empty () && omrd->m_labelId.isValid ())
						label = omrd->m_labelId.localize ();
					if (label.empty ())
						label = Cui::MenuInfoTypes::getLocalizedLabel (static_cast<Cui::MenuInfoTypes::Type>(omrd->m_menuItemType), clientObject->getGameObjectType ());

					Unicode::String result;
					CuiStringVariablesManager::process (CuiStringIds::radial_out_of_range_prose, 
						Unicode::emptyString, 
						clientObject->getLocalizedName (), 
						label,
						result);
					CuiSystemMessageManager::sendFakeSystemMessage (result);
				}
				CuiSoundManager::play (CuiSounds::negative);
			}
			else if (!omrd->isEnabled ())
			{
				CuiSoundManager::play (CuiSounds::negative);
			}
			else
			{
				const int type_open     = Cui::MenuInfoTypes::ITEM_OPEN;
				const int type_open_new = Cui::MenuInfoTypes::ITEM_OPEN_NEW_WINDOW;
				const int type_extract  = Cui::MenuInfoTypes::EXTRACT_OBJECT;
				const int type_use1     = Cui::MenuInfoTypes::ITEM_PUBLIC_CONTAINER_USE1;

				if (!allowOpenOnly || omrd->m_menuItemType == type_open || omrd->m_menuItemType == type_open_new || omrd->m_menuItemType == type_extract || omrd->m_menuItemType == type_use1)
					performMenuAction (omrd->m_menuItemType, -1, omrd->isServerNotify (), mySequence, radialComplete);
			}
		}
	}
	// restore s_object.
	s_object = obj;

	//@todo: make network messages and implement
}

//----------------------------------------------------------------------

int CuiRadialMenuManager::findDefaultAction (Object & object)
{
	static bool inFunction = false;

	DEBUG_FATAL (inFunction, ("already in function CuiRadialMenuManager::findDefaultAction"));
	
	inFunction = true;
	
	DEBUG_FATAL (!s_installed, ("not installed\n"));
	
	int action = 0;

	CreatureObject * const player = Game::getPlayerCreature();
	if (&object != player)
	{		
		static CuiMenuInfoHelper helper;
		helper.clear ();
		
		if (populateMenu (helper, object, false))
		{
			const ObjectMenuRequestData * const omrd = helper.getDefault ();
			
			if (omrd)
				action = omrd->m_menuItemType;

			// Item usability override. Can I wear this item?  If not, you
			// can't equip it.
			if (Cui::MenuInfoTypes::ITEM_EQUIP == action)
			{
				ClientObject const * const clientObject = object.asClientObject();
				TangibleObject const * const tangibleObject = clientObject ? clientObject->asTangibleObject() : NULL;
				if (tangibleObject) 
				{
					if (!ClientObjectUsabilityManager::canWear(player, tangibleObject)) 
					{
						action = Cui::MenuInfoTypes::ITEM_UNEQUIP;
					}
				}
			}
		}
	}

	inFunction = false;
	return action;
}

//----------------------------------------------------------------------

void CuiRadialMenuManager::handleObjectMenuRequestMessage (const MessageQueueObjectMenuRequest & msg)
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));

	const NetworkId & id = msg.getTargetId ();
	updateCache (id, msg.m_sequence, msg.getData ());

	{
		const RequestId rid (msg.getTargetId (), msg.m_sequence);
		const PendingServerNotifyMap::iterator it = s_pendingServerNotifications.find (rid);
		if (it != s_pendingServerNotifications.end ())
		{
			GameNetwork::send (ObjectMenuSelectMessage (rid.first, (*it).second), true);
			s_pendingServerNotifications.erase (it);
			return;
		}
	}

	if (s_object.getPointer () && id == s_object->getNetworkId ())
	{
		//-- it is possible to receive updated cache information after the radial menu is complete
		//-- just ignore it since this menu is now fully populated
		if (s_sequenceActive == msg.m_sequence && !ms_radialComplete)
			mergeWithServerData (msg.getData ());
	}
}

//----------------------------------------------------------------------

void CuiRadialMenuManager::mergeWithServerData (const ObjectMenuRequestDataVector & dataVector)
{
	if (!s_object.getPointer ())
		return;

	ClientObject * const clientObject = dynamic_cast<ClientObject *>(s_object.getPointer ());
	const int got = clientObject ? clientObject->getGameObjectType () : 0;

	if (ms_radial)
	{
		UIWidget * const centerWidget = ms_radial->GetRadialCenterWidget ();
		if (centerWidget)
			centerWidget->SetEnabled (false);
	}

	ms_radialComplete = true;
	s_helper = CuiMenuInfoHelper (dataVector);

	if (ms_radial)
	{
		const UISize oldSize = ms_radial->GetSize ();
		
		s_helper.updateRadialMenu (*ms_radial, got);
		
		if (!ms_radial->GetParent ())
			return;
		
		//- make sure the new radial menu is as centered horizontally on the
		//- old center as possible,
		//-- then make sure it is fully on-screen
		
		const UISize newSize = ms_radial->GetSize ();
		UIPoint location (ms_radial->GetLocation ());
		
		if (newSize.x != oldSize.x)
		{
			location.x -= (newSize.x - oldSize.x) / 2;
			ms_radial->SetLocation (location);
		}
		
		UIRect newRect (ms_radial->GetRect ());
		const UISize pageSize (safe_cast<UIWidget *>(ms_radial->GetParent ())->GetSize ());
		
		if (newRect.bottom >= pageSize.y)
			newRect -= UIPoint (0L, (newRect.bottom - pageSize.y));
		if (newRect.top < 0)
			newRect -= UIPoint (0L, newRect.top);
		
		if (newRect.right >= pageSize.x)
			newRect -= UIPoint (0L, (newRect.right - pageSize.x));
		if (newRect.left < 0)
			newRect -= UIPoint (0L, newRect.left);
		
		ms_radial->SetLocation (newRect.Location ());
	}
	else if (ms_popup)
	{
		s_helper.updatePopupMenu (*ms_popup, got);
	}
}

//----------------------------------------------------------------------

void CuiRadialMenuManager::onHideRadialMenu (const UIRadialMenu * menu)
{
	if (menu != ms_radial)
		return;

	CuiManager::getIoWin ().setRadialMenuActiveHack (false);
	CuiManager::requestPointer                 (false);

	setRadial (0);
}

//----------------------------------------------------------------------

void CuiRadialMenuManager::onHidePopupMenu (const UIPopupMenu * menu)
{
	if (menu != ms_popup)
		return;

	setPopup (0);
}

//----------------------------------------------------------------------

void CuiRadialMenuManager::OnPopupMenuSelection (UIWidget * context)
{
	UNREF (context);

	if ((!ms_radial && !ms_popup) || !s_object.getPointer ())
	{
		WARNING_STRICT_FATAL (true, ("bogus object in radial menu selection"));
		return;
	}

	UINarrowString selStr;
	
	UIRadialMenu * radial   = 0;
	UIPopupMenu * pop       = 0;

	bool serverNotify       = false;
	bool outOfRange         = false;

	Unicode::String label;

	if (context->IsA (TUIRadialMenu))
	{
		radial = safe_cast<UIRadialMenu *>(context);
		
		if (radial != ms_radial)
			return;
		
		selStr = radial->GetSelectedName ();

		const UIDataSource * const ds = radial->GetDataSource ();
		const UIData * const data = ds ? safe_cast<const UIData *>(ds->GetChild (selStr.c_str ())) : 0;
		if (data)
		{
			float opacity = 0.0f;
			data->GetPropertyBoolean (CuiMenuInfoHelper::PROP_SERVER_NOTIFY, serverNotify);
			if (data->GetPropertyFloat   (UIWidget::PropertyName::Opacity, opacity))
				outOfRange = opacity < 1.0f;
			data->GetProperty (CuiMenuInfoHelper::DataProperties::ActualLabel, label);
		}
	}
	else if (context->IsA (TUIPopupMenu))
	{
		pop    = safe_cast<UIPopupMenu *>(context);
		selStr = pop->GetSelectedName ();
		const UIDataSourceContainer * const dsc = pop->GetDataSourceContainer ();
		const UIDataSourceBase * const dsb      = dsc ? safe_cast<const UIDataSourceBase *>(dsc->GetChild (selStr.c_str ())) : 0;
		if (dsb)
		{
			float opacity = 0.0f;
			dsb->GetPropertyBoolean (CuiMenuInfoHelper::PROP_SERVER_NOTIFY, serverNotify);
			if (dsb->GetPropertyFloat   (UIWidget::PropertyName::Opacity, opacity))
				outOfRange = opacity < 1.0f;
			dsb->GetProperty (UIPopupMenu::DataProperties::Text, label);
		}
	}
	

	if (selStr.empty ())
		return;
 
	Unicode::NarrowString numericToken;
	size_t endpos;

	if (!Unicode::getFirstToken (selStr, 0, endpos, numericToken) || numericToken.empty ())
		return;

	s_inHandler = true;

	const int type = atoi (numericToken.c_str ());

	//-- out of range
	if (outOfRange)
	{
		const ClientObject * const clientObject = dynamic_cast<const ClientObject *>(s_object.getPointer ());
		if (clientObject)
		{
			if (label.empty ())
				label = Cui::MenuInfoTypes::getLocalizedLabel (static_cast<Cui::MenuInfoTypes::Type>(type), clientObject->getGameObjectType ());
			
			Unicode::String result;
			CuiStringVariablesManager::process (CuiStringIds::radial_out_of_range_prose, 
				Unicode::emptyString, 
				clientObject->getLocalizedName (),
				label,
				result);
			CuiSystemMessageManager::sendFakeSystemMessage (result);
		}
		CuiSoundManager::play (CuiSounds::negative);
	}
	else
		performMenuAction (type, pop ? pop->GetSelectedIndex () : -1, serverNotify, s_sequenceGlobal, ms_radialComplete);

	s_inHandler = false;

	if (++s_sequenceGlobal == 0)
		++s_sequenceGlobal;
}

//----------------------------------------------------------------------

void CuiRadialMenuManager::performMenuAction (int sel, int index, bool serverNotify, uint8 sequence, bool radialComplete)
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));

	REPORT_LOG_PRINT (sel < 0 || sel >= 65535, ("we selected an invalid menu type: %d\n", sel));

	Object * const object             = s_object.getPointer ();

	if (!object)
		return;

	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player)
		return;

	ClientObject * const clientObject = object ? object->asClientObject() : NULL;

	// We handle this on the client now, so lets do this before we let the server consume the message.
	if (sel == COMBAT_ATTACK)
	{
		CuiPreferences::setAutoAimToggle(true);
		GroundCombatActionManager::ActionType actionType = GroundCombatActionManager::AT_toggleRepeatPrimaryAttack;
		GroundCombatActionManager::attemptAction(actionType);
	}

	// Trying to place a story teller object. We need to intercept this so the user can enter targeting
	// mode and get us a location before we tell the server anything.
	if (sel == ITEM_USE || sel == SERVER_MENU1 || sel == SERVER_MENU2)
	{
		if (clientObject &&
			(clientObject->getGameObjectType() == SharedObjectTemplate::GOT_misc_ground_target ||
			clientObject->getGameObjectType() == SharedObjectTemplate::GOT_misc_blueprint)
			)
		{
			if(Game::isSpace())
			{
				// We're in space. Drop the prop at our feet.
				CuiRadialMenuManagerNamespace::s_groundTargetTokenId = clientObject->getNetworkId();
				CuiRadialMenuManagerNamespace::s_groundTargetMenuSelection = sel;
				
				Vector location = player->getPosition_w();
				NetworkId cellId = NetworkId::cms_invalid;
				
				if(player->getParentCell())
					cellId = player->getParentCell()->getOwner().getNetworkId();
				CuiRadialMenuManager::setStorytellerMode(true);
				CuiRadialMenuManager::createGroundPlacementMessage(location, cellId);
				CuiRadialMenuManager::setStorytellerMode(false);
				
				return;
			}

			CuiSystemMessageManager::sendFakeSystemMessage(CuiStringIds::groundplace.localize());
			CuiRadialMenuManager::setStorytellerMode(true);
			CuiRadialMenuManagerNamespace::s_groundTargetTokenId = clientObject->getNetworkId();
			CuiRadialMenuManagerNamespace::s_groundTargetMenuSelection = sel;

			return;

		}

	}

	//----------------------------------------------------------------------
	//-- notify the server, overriding client-side actions?  no override for now

	if (serverNotify)
	{
		bool mountAction = ((sel == SERVER_PET_MOUNT) || ((sel == SERVER_VEHICLE_ENTER_EXIT) && !player->isRidingMount()));

		//send the mount command directly, otherwise send the menu selection off to script to deal with
		if (mountAction)
		{
			bool const isMountable = ClientCommandQueue::allowMountCommand(*player, *object);
			bool const isMounted = player->isRidingMount();

			if (isMountable && !player->isIncapacitated() && !player->isDead())
			{
				if (isMounted) 
				{
					ClientCommandQueue::enqueueCommand(hash_dismount, object->getNetworkId (), Unicode::emptyString);
				}
				else
				{
					ClientCommandQueue::enqueueCommand(hash_mount, object->getNetworkId (), Unicode::emptyString);
				}
			}
		}
		else
		{
			GameNetwork::send (ObjectMenuSelectMessage (object->getNetworkId (), static_cast<uint16>(sel)), true);
		}
	}
	else if (!radialComplete)
	{
		waitForServerNotifyState (static_cast<uint16>(sel), object->getNetworkId (), sequence);
	}

	if (Cui::MenuInfoTypes::executeCommandForMenu (static_cast<Cui::MenuInfoTypes::Type>(sel), object->getNetworkId (), clientObject ? clientObject->getUniqueId () : 0))
		return;

	//----------------------------------------------------------------------
	//-- conversation subchoices
		
	else if (index >= 0 && sel == CONVERSE_RESPONSE)
	{
		CuiConversationManager::respond (object->getNetworkId (), index);
	}

	else if (sel == CRAFT_START)
	{
		if (clientObject)
			CuiCraftManager::startCrafting (clientObject->getNetworkId ());
	}
	else if (sel == CRAFT_HOPPER_INPUT)
	{
		if (clientObject)
		{
			ClientObject * const hopper = CuiCraftManager::getCraftingStationHopperInput (clientObject);
			if (hopper)
				CuiInventoryManager::requestItemOpen (*hopper, std::string (), 0, 0, true, false);
		}
	}
	else if (sel == TERMINAL_MISSION_LIST)
	{
		if (clientObject)
		{
			CuiMissionManager::setTerminalId (clientObject->getNetworkId());
			CuiActionManager::performAction  (CuiActions::missionBrowser, Unicode::emptyString);
		}
	}
	else if (sel == ITEM_USE)
	{
		if(clientObject && (clientObject->getGameObjectType() == SharedObjectTemplate::GOT_terminal_space))
		{
			CuiActionManager::performAction  (CuiActions::shipChoose, Unicode::emptyString);
		}
	}
	else if (sel == MISSION_DETAILS)
	{
		const ClientMissionObject* m = dynamic_cast<const ClientMissionObject*>(clientObject);
		if(m)
		{
			if (m->isSpaceMission())
			{
				static const Unicode::String activateFlag(Unicode::narrowToWide("activate"));
				CuiActionManager::performAction  (CuiActions::questJournal, activateFlag);
			}
			else
			{
				CuiMissionManager::setDetails (m);
				CuiActionManager::performAction  (CuiActions::missionDetails, Unicode::emptyString);
			}
		}
	}
	else if (sel == MISSION_ABORT || sel == MISSION_END_DUTY)
	{
		const ClientMissionObject* m = dynamic_cast<const ClientMissionObject*>(clientObject);
		if(m)
		{
			CuiMissionManager::removeMission (m->getNetworkId(), false);
		}
	}
	else if (sel == SET_NAME)
	{
		if(clientObject)
		{
			ClientWaypointObject const * const cwo = dynamic_cast<ClientWaypointObject const *>(clientObject);
			if (cwo)
			{
				Unicode::String params;
				params += Unicode::narrowToWide (cwo->getNetworkId ().getValueString ());
				params.push_back (' ');
				params += CuiStringIds::waypoint_name.localize();
				CuiActionManager::performAction  (CuiActions::setName, params);
			}
		}
	}
	else if ((sel == ITEM_WP_SETCOLOR_BLUE) || (sel == ITEM_WP_SETCOLOR_GREEN) || (sel == ITEM_WP_SETCOLOR_ORANGE) || (sel == ITEM_WP_SETCOLOR_YELLOW) || (sel == ITEM_WP_SETCOLOR_PURPLE) || (sel == ITEM_WP_SETCOLOR_WHITE))
	{
		if(clientObject)
		{
			ClientWaypointObject const * const cwo = dynamic_cast<ClientWaypointObject const *>(clientObject);
			if (cwo)
			{
				std::string color;
				if (sel == ITEM_WP_SETCOLOR_BLUE)
					color = "blue";
				else if (sel == ITEM_WP_SETCOLOR_GREEN)
					color = "green";
				else if (sel == ITEM_WP_SETCOLOR_ORANGE)
					color = "orange";
				else if (sel == ITEM_WP_SETCOLOR_YELLOW)
					color = "yellow";
				else if (sel == ITEM_WP_SETCOLOR_PURPLE)
					color = "purple";
				else if (sel == ITEM_WP_SETCOLOR_WHITE)
					color = "white";
				else
					color = "blue";

				GenericValueTypeMessage<std::pair<NetworkId, std::string> > const msg("SetWaypointColor", std::make_pair(cwo->getNetworkId(), color));
				GameNetwork::send(msg, true);
			}
		}
	}
	else if (sel == SHIP_MANAGE_COMPONENTS)
	{
		if(clientObject)
		{
			ShipObject const * shipToManage = NULL;
			NetworkId terminalId(NetworkId::cms_invalid);

			if (clientObject->getGameObjectType() == SharedObjectTemplate::GOT_data_ship_control_device)
			{
		
				if(Game::isSpace() && PlayerObject::isAdmin())
				{
					shipToManage = Game::getPlayerPilotedShip();
					CuiSystemMessageManager::sendFakeSystemMessage (Unicode::narrowToWide("Allowing ship management in space due to GOD mode."));
				}
				else
				{
					//get ship in scd and send that as the param
					const Container * const c = ContainerInterface::getContainer(*clientObject);
					if(c)
					{
						for (ContainerConstIterator i = c->begin(); i != c->end(); ++i)
						{
							Container::ContainedItem const item = *i;
							Object const * const o = item.getObject();
							ClientObject const * const co = o ? o->asClientObject() : NULL;
							ShipObject const * const s = co ? co->asShipObject() : NULL;
							if(s)
							{
								shipToManage = s;
								break;
							}
						}
					}
				}
			}
			else if (clientObject->getGameObjectType() == SharedObjectTemplate::GOT_terminal_space_npe)
			{
				shipToManage = getFirstShipInDatapad( player );

				if (shipToManage != 0)
				{
					terminalId = clientObject->getNetworkId();
				}
			}

			if(shipToManage)
			{
				char buf[256];
				_snprintf (buf, 255, "%s %s", shipToManage->getNetworkId().getValueString().c_str(), terminalId.getValueString().c_str());
				CuiActionManager::performAction(CuiActions::manageShipComponents, Unicode::narrowToWide(buf));
			}
		}
	}
	else if(sel == PROGRAM_DROID)
	{
		if(clientObject && clientObject->getGameObjectType() == SharedObjectTemplate::GOT_data_droid_control_device)
		{
			if(Game::getPlayerPilotedShip())
				return;
			CuiActionManager::performAction  (CuiActions::droidCommand, Unicode::narrowToWide(clientObject->getNetworkId().getValueString()));
		}
	}
}

//----------------------------------------------------------------------

void CuiRadialMenuManager::clear ()
{
	if (s_inHandler)
		return;

	if (ms_radial)
	{
		UIManager::gUIManager ().PopContextWidgets (ms_radial);
		setRadial (0);
	}

	if (ms_popup)
	{
		UIManager::gUIManager ().PopContextWidgets (ms_radial);
		setPopup (0);
	}
}

//----------------------------------------------------------------------

void CuiRadialMenuManager::setRadial (UIRadialMenu * menu)
{
	if (menu != ms_radial) 
	{
		if (menu)
		{
			menu->Attach (0);
			setPopup (0);
			//		CuiMediator::incrementKeyboardInputActiveCount (1);
		}
		
		if (ms_radial)
		{
			ms_radial->RemoveCallback (s_listener);
			ms_radial->Detach (0);
			ms_radial = 0;
			//		CuiMediator::incrementKeyboardInputActiveCount (-1);
			
		}
		
		ms_radial = menu;
		
		if (ms_radial)
			ms_radial->AddCallback (s_listener);
	}
}

//----------------------------------------------------------------------

void CuiRadialMenuManager::setPopup (UIPopupMenu * menu, bool setCallback)
{
	if (menu)
	{
		menu->Attach (0);
		setRadial (0);
		//CuiMediator::incrementKeyboardInputActiveCount (1);
	}

	if (ms_popup)
	{
		ms_popup->RemoveCallback (s_listener);
		ms_popup->Detach (0);
		ms_popup = 0;
		//CuiMediator::incrementKeyboardInputActiveCount (-1);
	}

	ms_popup = menu;

	if (ms_popup && setCallback)
		ms_popup->AddCallback (s_listener);
}

//----------------------------------------------------------------------

bool CuiRadialMenuManager::updateRanges ()
{ 
	if (!ms_radial && !ms_popup)
		return false;

	const Object * const object = s_object.getPointer ();

	if (!object)
	{
		CuiRadialMenuManager::clear ();
		return false;
	}

	ClientObject const * const clientObject = object ? object->asClientObject() : NULL;
	if (!clientObject)
		return false;

	// dismiss radial menu if the target is stealth and
	// player has not passively revealed the stealth target
	CreatureObject const * const creatureObject = clientObject->asCreatureObject();
	if (creatureObject && !creatureObject->getCoverVisibility() && !creatureObject->isPassiveRevealPlayerCharacter(Game::getPlayerNetworkId()))
	{
		CuiRadialMenuManager::clear ();
		return false;
	}

	const int got = clientObject ? clientObject->getGameObjectType () : 0;

	if (ms_radial)
	{
		if (updateRanges (s_helper, *clientObject))
		{
			const UIRect oldRect = ms_radial->GetRect ();
			
			s_helper.updateRadialMenu (*ms_radial, got);
			
			UIRect newRect = ms_radial->GetRect ();
			
			if (newRect != oldRect)
			{
				ms_radial->SetLocation (s_originalPosition);
				UIManager::fitWidgetOnScreen (*ms_radial, UIManager::CWA_Center, true);
			}

			return true;
		}
	}
	else if (ms_popup)
	{
		if (updateRanges (s_helper, *clientObject))
		{
			s_helper.updatePopupMenu (*ms_popup, got);
		}
	}

	return false;
}

//----------------------------------------------------------------------

bool CuiRadialMenuManager::updateRanges (CuiMenuInfoHelper & helper, const Object & object)
{
	Object * const player = Game::getPlayer ();
	if (player)
	{
		const Object * const parent = ContainerInterface::getFirstParentInWorld (object);
		if (parent)
		{
			float range = parent->findPosition_w ().magnitudeBetween (player->findPosition_w ());
			range -= parent->getAppearanceSphereRadius () + player->getAppearanceSphereRadius ();
			return helper.updateRanges (range, object);
		}
		else
		{
			WARNING (true, ("CuiRadialMenuManager no topmost container"));
			return false;
		}
	}

	return false;
}

//----------------------------------------------------------------------

void CuiRadialMenuManager::waitForServerNotifyState (uint16 sel, const NetworkId & id, uint8 sequence)
{
	s_pendingServerNotifications [RequestId (id, sequence)] = sel;
}

//----------------------------------------------------------------------

void CuiRadialMenuManager::touchCache  (const NetworkId & id)
{
	const Object * const obj = NetworkIdManager::getObjectById (id);

	if (!obj)
		return;

	static CuiMenuInfoHelper local_s_helper;
	local_s_helper.clear ();

	if (populateMenu (local_s_helper, *obj, false))
	{
		updateRanges (local_s_helper, *obj);

		local_s_helper.purgeLabels ();

		if (++s_sequenceGlobal == 0)
			++s_sequenceGlobal;

		static ObjectMenuRequestDataVector combinedData;
		combinedData.clear ();

		uint8 mySequence = 0;
		findOrRequestCache (id, local_s_helper.getDataVector (), s_cacheTimeout, combinedData, mySequence);
	}
}

//----------------------------------------------------------------------

void  CuiRadialMenuManager::setObjectMenuDirty (NetworkId const & id)
{
	const CacheMap::iterator it = s_cacheMap.find (id);
	if (it != s_cacheMap.end ())
	{
		static ObjectMenuRequestDataVector omrdv;
		static ObjectMenuRequestDataVector omrdv2;
		omrdv.clear ();
		omrdv2.clear ();
		uint8 sequence = 0;

		//-- erase the entry from the cache map to force a re-request
		s_cacheMap.erase (it);
		findOrRequestCache (id, omrdv, 0.0f, omrdv, sequence);
	}
}

//----------------------------------------------------------------------

void CuiRadialMenuManager::registerCanBeManipulated(CanBeManipulatedFunction function)
{
	cs_canBeManipulatedManager.registerCanBeManipulated(function);
}


//----------------------------------------------------------------------

bool CuiRadialMenuManager::performDefaultDoubleClickAction(Object const & object, bool const modifierPressed)
{
	////////////////////////////////////
	// Absorb input while the debounce timer has not expired.
	if (!s_doubleClickSpamTimer.isExpired()) 
	{
		return true;
	}

	////////////////////////////////////
	
	ClientObject const * const clientObject = object.asClientObject();
	
	if (!clientObject && !CuiPreferences::getAllowTargetAnything ())
	{
		return false;
	}

	CreatureObject * const player = Game::getPlayerCreature();
	if (!player || player->isIncapacitated() || player->isDead())
	{
		return false;
	}

	////////////////////////////////////

	TangibleObject const * const tangible = clientObject ? clientObject->asTangibleObject() : NULL;
	IntangibleObject const * const intangible = dynamic_cast<const IntangibleObject *>(clientObject);
	CreatureObject const * const creature = tangible ? tangible->asCreatureObject() : NULL;
	Object const * const containedByObject = tangible ? ContainerInterface::getContainedByObject(*tangible) : NULL;
	
	////////////////////////////////////

	SharedObjectTemplate::GameObjectType got  = SharedObjectTemplate::GOT_none;
	if (clientObject)
		got = static_cast<SharedObjectTemplate::GameObjectType> (clientObject->getGameObjectType ());
	
	////////////////////////////////////
	
	const bool isVendor = (got == SharedObjectTemplate::GOT_vendor || (tangible && tangible->hasCondition (TangibleObject::C_vendor)));
	bool isAttackable = !isVendor;
	isAttackable = isAttackable && tangible != 0 && tangible->isAttackable();
	isAttackable = isAttackable && (!creature || !creature->isDead ());
	isAttackable = isAttackable && got != SharedObjectTemplate::GOT_corpse;
	
	////////////////////////////////////

	bool const isNestedInventory = tangible ? CuiInventoryManager::isNestedInventory(*tangible) : 0;
	bool const isNestedEquipped = tangible ? CuiInventoryManager::isNestedEquipped(*tangible)  : 0;
	bool const isOnPlayer = isNestedInventory || isNestedEquipped;
	bool const isConversable = creature ? creature->hasCondition(TangibleObject::C_conversable) : false;
	bool const isMount = creature ? creature->hasCondition(TangibleObject::C_mount) : false;
	bool const isMounted = player->isRidingMount();
	bool const isInRange = clientObject ? isInUsableRange(clientObject, player, 7.0f) : true;
	bool const isIncapacitated = creature ? creature->isIncapacitated() : false;
	bool const isEnemy = creature ? creature->isEnemy() : false;
	bool const pickupable = !isOnPlayer && canPickUp(got);
	bool const isPlayer = creature ? creature->isPlayer() : false;
	bool const isInWorld = !containedByObject || containedByObject->getCellProperty ();
	bool const isOwner = creature ? creature->getMasterId() == player->getNetworkId() : false;
	bool const isCraftingStation = got == SharedObjectTemplate::GOT_misc_crafting_station;

	////////////////////////////////////
	
	uint32 const clientObjectUniqueId = clientObject ? clientObject->getUniqueId() : 0;
	NetworkId const & networkId = object.getNetworkId();

	////////////////////////////////////

	bool processedByDoubleClickManager = false;
		
	if (isInRange) 
	{
		if(isEnemy && isIncapacitated && isPlayer)
		{
			processedByDoubleClickManager = Cui::MenuInfoTypes::executeCommandForMenu(COMBAT_DEATH_BLOW, networkId, clientObjectUniqueId);
		}
		else if (isMount && isInWorld)
		{
			bool isMountable = isOwner;

			// Check to see if the player is in the group of the owner.
			if (!isMountable && player->getGroup().isValid())
			{
				GroupObject const * const group  = safe_cast<GroupObject const *>(player->getGroup().getObject());
				bool isLeader = false;
				GroupObject::GroupMember const * const member = group->findMember(creature->getMasterId(), isLeader);
				isMountable = member != NULL;
			}

			if (isMountable) 
			{
				// Note: Multi-passenger vehicles could use this code and switch off the isOwner flag.
				if (isMounted) 
				{
					ClientCommandQueue::enqueueCommand(hash_dismount, networkId, Unicode::emptyString);
					processedByDoubleClickManager = true;
				}
				else
				{
					if (isOwner)
					{
						ClientCommandQueue::enqueueCommand(hash_mount, networkId, Unicode::emptyString);
						processedByDoubleClickManager = true;
					}
				}
			}
		}
		else if(isCraftingStation)
		{
			if (!CuiCraftManager::isCrafting())
			{
				ClientCommandQueue::enqueueCommand(hash_crafting, networkId, Unicode::emptyString);
			}

			processedByDoubleClickManager = true;
		}
	}

	UNREF(intangible);
	UNREF(modifierPressed);
	UNREF(pickupable);
	UNREF(isConversable);
	
	// Give instant feedback.

	if (processedByDoubleClickManager) 
	{
		CuiSoundManager::play(CuiSounds::select);
		s_doubleClickSpamTimer.reset();
	}
	
	return processedByDoubleClickManager;
}

bool CuiRadialMenuManager::isStorytellerMode()
{
	return CuiRadialMenuManagerNamespace::s_groundTargetMode == CuiRadialMenuManagerNamespace::GTM_Storyteller;
}

bool CuiRadialMenuManager::isGrenadeTargetMode()
{
	return CuiRadialMenuManagerNamespace::s_groundTargetMode == CuiRadialMenuManagerNamespace::GTM_Grenade;
}

void CuiRadialMenuManager::setStorytellerMode(bool enable)
{
	
	if(!enable)
	{
		CuiRadialMenuManagerNamespace::s_groundTargetMode = CuiRadialMenuManagerNamespace::GTM_None;
	}
	else
	{
		CuiRadialMenuManagerNamespace::s_groundTargetMode = CuiRadialMenuManagerNamespace::GTM_Storyteller;
	}

	CuiRadialMenuManagerNamespace::s_groundTargetTokenId = NetworkId::cms_invalid;
}

void CuiRadialMenuManager::setGrenadeTargetMode(bool enable)
{
	if(!enable)
	{
		CuiRadialMenuManagerNamespace::s_groundTargetMode = CuiRadialMenuManagerNamespace::GTM_None;
	}
	else
	{
		CuiRadialMenuManagerNamespace::s_groundTargetMode = CuiRadialMenuManagerNamespace::GTM_Grenade;
	}

	CuiRadialMenuManagerNamespace::s_groundTargetTokenId = NetworkId::cms_invalid;
}

void CuiRadialMenuManager::createGroundPlacementMessage(Vector & location, NetworkId const & cellId)
{
	if(CuiRadialMenuManagerNamespace::s_groundTargetTokenId == NetworkId::cms_invalid || CuiRadialMenuManagerNamespace::s_groundTargetMode == CuiRadialMenuManagerNamespace::GTM_None)
		return;

	if(cellId != NetworkId::cms_invalid)
	{
		CellObject *cellObject = dynamic_cast<CellObject *>(NetworkIdManager::getObjectById(cellId));
		if(cellObject)
		{
			Vector reticlePoint = location;

			reticlePoint = cellObject->getTransform_o2w().rotateTranslate_p2l(reticlePoint);
			location = reticlePoint;
		}
	}

	
	GenericValueTypeMessage<std::pair<NetworkId, std::pair<std::pair<int, float>, std::pair<float, float> > > > const msg("RequestGroundObjectPlacement", std::make_pair(CuiRadialMenuManagerNamespace::s_groundTargetTokenId, std::make_pair(std::make_pair(CuiRadialMenuManagerNamespace::s_groundTargetMenuSelection, location.x), std::make_pair(location.y, location.z))));
	GameNetwork::send(msg, true);
}

//======================================================================
