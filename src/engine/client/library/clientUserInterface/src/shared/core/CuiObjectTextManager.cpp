//======================================================================
//
// CuiObjectTextManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiObjectTextManager.h"

#include "clientGame/BuildingObject.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/FadingTextAppearance.h"
#include "clientGame/Game.h"
#include "clientGame/GroupObject.h"
#include "clientGame/GuildObject.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/ShipObject.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientObject/RibbonAppearance.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "clientUserInterface/CuiGameColorManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiStringIdsWho.h"
#include "clientUserInterface/CuiTextManager.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionUtils.h"
#include "sharedCollision/SimpleExtent.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/PvpData.h"
#include "sharedGame/SharedBuildingObjectTemplate.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedNetworkMessages/SceneChannelMessages.h"
#include "sharedObject/CellProperty.h"
#include "swgSharedUtility/States.def"

#include "UIBaseObject.h"
#include "UIImageStyle.h"
#include "UIManager.h"
#include "UIPage.h"

#include "UnicodeUtils.h"

#include <algorithm>
#include <list>

//======================================================================

//lint -esym(641,WorldObjectLists) // enum to int

namespace CuiObjectTextManagerNamespace
{
	class HeadFramePair
	{
	public:
		Vector pos;
		float  time;
		int    offset;
		int    lastFrameOffset;
		int    maxY;
		int    lastFrameMaxY;
		bool   canSee;
		bool   canSeeChecked;
		float  canSeeTimeFactor;

		HeadFramePair (Vector _pos, float _t) : pos (_pos), time (_t), offset (0), lastFrameOffset (0), maxY (100000), lastFrameMaxY (100000), canSee (false), canSeeChecked (false), canSeeTimeFactor (1.0f) {}
		HeadFramePair () : pos (), time (0.0f), offset (0), lastFrameOffset (0), maxY (100000), lastFrameMaxY (100000), canSee (false), canSeeChecked (false), canSeeTimeFactor (1.0f) {}
	};

	typedef std::map<CachedNetworkId, HeadFramePair> HeadMap;
	HeadMap s_headMap;

	class Listener : public MessageDispatch::Receiver 
	{
	public:
		
		Listener () : MessageDispatch::Receiver  ()
		{
			connectToMessage (Game::Messages::SCENE_CHANGED);
		}
			 
		void receiveMessage (const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
		{
			if (message.isType (Game::Messages::SCENE_CHANGED))
			{
				s_headMap.clear ();
			}
		}
	};

	Listener * s_listener = 0;

	struct NameTime
	{
		float timeIn;
		float timeOut;

		UIScalar nameWidgetOffset;

		NameTime () : timeIn (0.0f), timeOut (0.0f), nameWidgetOffset(0) {}
	};

	typedef stdmap<CachedNetworkId, NameTime>::fwd CachedNetworkIdMap;
	CachedNetworkIdMap                             s_objectNamesToDraw;

	//----------------------------------------------------------------------

	const float                     timeout_fadeout     = 0.5f;
	std::map<uint32, UIImageStyle*> s_factionImageStyles;
	char const * const              s_factionIconPath   = "/styles.icon.faction";
	char const * const              s_factionIconSuffix = "_logo";
	char const * const              s_groupIconPath     = "/Styles.Icon.Misc.match_group";
	UIImageStyle *                  s_groupIcon         = 0;
	bool                            s_installed         = false;
	const Unicode::String           s_guildPrefix       = Unicode::narrowToWide(" <");
	const Unicode::String           s_guildSuffix       = Unicode::narrowToWide(">");

	//---------------------------------------------------------------------- 
	
	bool canSeeInternal (const Vector & sourcePos, const CellProperty * sourceCell, const Object & targetObject, float & timeFactor)
	{
		timeFactor = 1.0f;
		const Appearance * const app = targetObject.getAppearance ();
		if (!app)
			return false;

		if (!app->isLoaded())
			return false;

		if (!app->getRenderedThisFrame())
			return false;

		CellProperty const * const worldCell = CellProperty::getWorldCellProperty();
		CellProperty const * const targetCell = targetObject.getParentCell();
		if (((sourceCell == worldCell) || (targetCell == worldCell)) && (sourceCell != targetCell))
			return false;

		Vector const & targetPos = targetObject.rotateTranslate_o2w(CuiObjectTextManager::getCurrentObjectHeadPoint_o (targetObject));
				
		// see if the objects can see each other
		
		CollisionInfo result;

		const uint16 flags = 
			ClientWorld::CF_terrain | 
			ClientWorld::CF_interiorGeometry |
			ClientWorld::CF_tangible;

		CollideParameters collideParameters;
		collideParameters.setQuality(CollideParameters::Q_high);
		collideParameters.setType(CollideParameters::T_opaqueSolid);

		if (!ClientWorld::collide(sourceCell, sourcePos, targetPos, collideParameters, result, flags) || result.getObject () == &targetObject)
		{
			timeFactor = 1.0f;
			return true;
		}
		
		return false;
	}
	
	//----------------------------------------------------------------------
	
	inline void preFormatPlayerFlags (Unicode::String & s, const PlayerObject & playerObject)
	{
		static const Unicode::String str_ld     = Unicode::narrowToWide ("(") + CuiStringIdsWho::link_dead.localize()          + Unicode::narrowToWide (") ");
		static const Unicode::String str_afk    = Unicode::narrowToWide ("(") + CuiStringIdsWho::away_from_keyboard.localize() + Unicode::narrowToWide (") ");
		static const Unicode::String str_helper = Unicode::narrowToWide ("(") + CuiStringIdsWho::helper.localize()             + Unicode::narrowToWide (") ");
		static const Unicode::String str_lfg    = Unicode::narrowToWide ("(") + CuiStringIdsWho::looking_for_group.localize()  + Unicode::narrowToWide (") ");
		
		if (playerObject.isLinkDead ())
		{
			s += str_ld;
		}
		else
		{
			if (playerObject.isAwayFromKeyBoard ())
			{
				s += str_afk;
			}
			else
			{
				if (playerObject.isHelper())
				{
					s += str_helper;
				}
				
				if (playerObject.isLookingForGroup ())
				{
					s += str_lfg;
				}
			}
		}
		
		if (!s.empty ())
			s.push_back ('\n');
	}

	//----------------------------------------------------------------------
	
	inline void postFormatCreatureFlags (Unicode::String & s, const CreatureObject & creature, const PlayerObject * playerObject)
	{
		const int guildId = creature.getGuildId();
		if (guildId)
		{
			s += s_guildPrefix;
			s += GuildObject::getGuildAbbrevUnicode (guildId);
			s += s_guildSuffix;
		}
		
		if (playerObject)
		{
			if ( playerObject->isDisplayingFactionRank() )
			{
				const Unicode::String factionRank = CreatureObject::getLocalizedGcwRankString(playerObject->getCurrentGcwRank(), creature.getPvpFaction());
				if (!factionRank.empty ())
				{
					s.push_back ('\n');
					s.push_back ('(');
					s += factionRank;
					s.push_back (')');
				}
			}

			const Unicode::String & title = playerObject->getLocalizedTitle ();
			if (!title.empty ())
			{
				s.push_back ('\n');
				s.push_back ('(');
				s += title;
				s.push_back (')');
			}

			bool hasPriviledgedTitle = false;
			if (playerObject->isNormalPlayer())
			{
				hasPriviledgedTitle = false;
			}
			else if (playerObject->isCustomerServiceRepresentative())
			{
				s.push_back ('\n');
				s.push_back ('(');
				s += CuiStringIdsWho::customer_service_representative.localize();
				s.push_back (')');
				hasPriviledgedTitle = true;
			}
			else if (playerObject->isDeveloper())
			{
				s.push_back ('\n');
				s.push_back ('(');
				s += CuiStringIdsWho::developer.localize();
				s.push_back (')');
				hasPriviledgedTitle = true;
			}
			else if (playerObject->isQualityAssurance())
			{
				s.push_back ('\n');
				s.push_back ('(');
				s += CuiStringIdsWho::quality_assurance.localize();
				s.push_back (')');
				hasPriviledgedTitle = true;
			}
			else if (playerObject->isWarden())
			{
#if PRODUCTION == 0
				// optimized client always display a warden
#else
				// release client only display a warden if I'm also a warden (i.e. only warden can see each other)
				PlayerObject const * self = Game::getConstPlayerObject();
				if (self && self->isWarden())
#endif
				{
					s.push_back ('\n');
					s.push_back ('(');
					s += CuiStringIdsWho::warden.localize();
					s.push_back (')');
					hasPriviledgedTitle = true;
				}
			}

			int const secondsUntilUnsquelched = playerObject->getSecondsUntilUnsquelched();
#if PRODUCTION == 0
			// optimized client always display a squelched character with details of the squelch
			if (secondsUntilUnsquelched != 0)
			{
				if (hasPriviledgedTitle)
					s.push_back (' ');
				else
					s.push_back ('\n');

				s.push_back ('(');

				if (secondsUntilUnsquelched < 0)
				{
					s += CuiStringIdsWho::squelched.localize();
					s.push_back (' ');
					s += Unicode::narrowToWide(playerObject->getSquelchedByName());
					s.push_back (' ');
					s.push_back ('[');
					s += Unicode::narrowToWide(playerObject->getSquelchedById().getValueString());
					s.push_back (']');
				}
				else
				{
					s += CuiStringIdsWho::spammered.localize();
					s.push_back (' ');
					s += Unicode::narrowToWide(playerObject->getSquelchedByName());
					s.push_back (' ');
					s.push_back ('[');
					s += Unicode::narrowToWide(playerObject->getSquelchedById().getValueString());
					s.push_back (']');
					s.push_back (' ');
					s.push_back ('[');
					s += Unicode::narrowToWide(CalendarTime::convertSecondsToHMS(static_cast<unsigned int>(secondsUntilUnsquelched)));
					s.push_back (']');
				}

				s.push_back (')');
			}
#else
			// release client only display a squelched character if I'm the one who squelched
			// the character, or if I'm a warden, and display different text for a squelched
			// character depending on if I'm the one who squelched the character
			if (secondsUntilUnsquelched != 0)
			{
				if (playerObject->getSquelchedById() == Game::getPlayerNetworkId())
				{
					if (hasPriviledgedTitle)
						s.push_back (' ');
					else
						s.push_back ('\n');

					s.push_back ('(');

					if (secondsUntilUnsquelched < 0)
						s += CuiStringIdsWho::squelched_by_me.localize();
					else
						s += CuiStringIdsWho::spammered_by_me.localize();

					s.push_back (')');
				}
				else
				{
					PlayerObject const * self = Game::getConstPlayerObject();
					if (self && self->isWarden())
					{
						if (hasPriviledgedTitle)
							s.push_back (' ');
						else
							s.push_back ('\n');

						s.push_back ('(');

						s += CuiStringIdsWho::silenced.localize();

						s.push_back (')');
					}
				}
			}
#endif
		}
	}

	//--------------------------------------------------------------------
	// Return true if we should show the object.
	bool filterObjectType(int const drawNetworkIds, CreatureObject const * const player, Object const * const object)
	{
		ClientObject const * const clientObject = object ? object->asClientObject() : NULL;
		if (clientObject == NULL)
			return false;
		
		bool insert = false;
		
		// ---------------------------------------------------------------
		// Process UI selections.

		if (CuiPreferences::getDrawObjectNamesSigns() && (clientObject->getGameObjectType() == SharedObjectTemplate::GOT_misc_sign))
		{
			insert = true;
		}
		else if (CuiPreferences::getDrawObjectNames() && (clientObject->asTangibleObject() || drawNetworkIds))
		{
			insert = true;
		}
		else if (clientObject->asCreatureObject() != NULL)
		{
			CreatureObject const * const creature = clientObject->asCreatureObject();

			if (CuiPreferences::getDrawObjectNamesPlayers() && creature->isPlayer())
				insert = true;
			else if (CuiPreferences::getDrawObjectNamesNpcs() && !creature->isPlayer() && !creature->isBeast())
				insert = true;
			else if (CuiPreferences::getDrawObjectNamesGuild() && player->getGuildId() > 0 && (player->getGuildId() == creature->getGuildId()))
				insert = true;
			else if (CuiPreferences::getDrawObjectNamesGroup() && player->isGroupedWith(*creature))
				insert = true;
			else if (CuiPreferences::getDrawObjectNamesMyName() && player == object)
				insert = true;
			else if (CuiPreferences::getDrawObjectNamesBeasts() && creature->isBeast())
				insert = true;
		}

		// ---------------------------------------------------------------
		// Do not draw names above these objects -- unless draw network flag is enabled.
		if (!drawNetworkIds && insert) 
		{
			if (player == object && !CuiPreferences::getDrawObjectNamesMyName())
			{
				insert = false;
			}
			else if (object && (object->getObjectType () == SharedBuildingObjectTemplate::SharedBuildingObjectTemplate_tag))
			{
				insert = false;
			}
		}

		return insert;
	}
}

// ======================================================================

using namespace CuiObjectTextManagerNamespace;

// ======================================================================

void CuiObjectTextManager::install ()
{
	DEBUG_FATAL (s_installed, ("installed"));
	s_listener  = new Listener;

	// grab references to any faction icons
	UIPage * const root = UIManager::gUIManager().GetRootPage();
	NOT_NULL (root);
	if (root)
	{
		UIBaseObject * const factionIcons = root->GetObjectFromPath(s_factionIconPath);
		if (factionIcons)
		{
			UIBaseObject::UIObjectList objList;
			factionIcons->GetChildren(objList);
			for (UIBaseObject::UIObjectList::const_iterator i = objList.begin(); i != objList.end(); ++i)
			{
				UIBaseObject * const obj = *i;
				// find any images ending in the faction icon suffix, and put them in
				// the factionImageStyles map, using the string hash of the name
				// without the suffix
				if (obj->IsA(TUIImageStyle))
				{
					std::string s(obj->GetName());
					size_t pos = s.find(s_factionIconSuffix);
					if (pos != std::string::npos)
					{
						s.erase(pos, s.size()-pos);
						CrcLowerString tag(s.c_str());
						if (s_factionImageStyles.find(tag.getCrc()) == s_factionImageStyles.end())
						{
							s_factionImageStyles[tag.getCrc()] = safe_cast<UIImageStyle*>(obj);
							obj->Attach(0);
						}
					}
				}
			}
		}

		s_groupIcon = safe_cast<UIImageStyle *>(root->GetObjectFromPath (s_groupIconPath, TUIImageStyle));
		NOT_NULL (s_groupIcon);
		if (s_groupIcon)
			s_groupIcon->Attach (0);
	}


	s_installed = true;
}

//----------------------------------------------------------------------

void CuiObjectTextManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	s_objectNamesToDraw.clear ();
	delete s_listener;
	s_listener  = 0;

	while (s_factionImageStyles.begin() != s_factionImageStyles.end())
	{
		(*s_factionImageStyles.begin()).second->Detach(0);
		s_factionImageStyles.erase(s_factionImageStyles.begin());
	}

	if (s_groupIcon)
	{
		s_groupIcon->Detach (0);
		s_groupIcon = 0;
	}

	s_installed = false;
}


//----------------------------------------------------------------------

void CuiObjectTextManager::reset ()
{
	s_objectNamesToDraw.clear ();
	s_headMap.clear           ();
}

//----------------------------------------------------------------------

Vector CuiObjectTextManager::computeCurHeadPoint_o (const Object & obj)
{
	if (!obj.getAppearance() || !obj.getAppearance()->isLoaded())
		return Vector::zero;
	
	Vector curHeadPoint;
	BoxExtent box;
	computeBestFitBoxExtent (obj, box);

	const Vector & vmin = box.getMin ();
	const Vector & vmax = box.getMax ();

	curHeadPoint.x   = (vmin.x + vmax.x) * 0.5f;
	curHeadPoint.z   = (vmin.z + vmax.z) * 0.5f;
	curHeadPoint.y   = vmax.y;

	return curHeadPoint;
}

//----------------------------------------------------------------------

Vector CuiObjectTextManager::getCurrentObjectHeadPoint_o (const Object & obj)
{
	Vector headPoint;

	const float curTime          = Game::getElapsedTime ();
	const CachedNetworkId id (obj);
	const HeadMap::iterator it   = s_headMap.find (id);
	
	static const Vector INTERPOLATION_RATE (2.0f, 5.0f, 2.0f);

	if (it != s_headMap.end ())
	{
		HeadFramePair & hfp = (*it).second;

		if (hfp.time < curTime)
		{
			const float deltaTime = curTime - hfp.time;

			const Vector & curHeadPoint = computeCurHeadPoint_o (obj);

			Vector newPos = curHeadPoint;
			const Vector & deltaVector = newPos - hfp.pos;

			Vector absDeltaVector (static_cast<float>(fabs (deltaVector.x)), static_cast<float>(fabs (deltaVector.y)), static_cast<float>(fabs (deltaVector.z)));

			Vector t = deltaTime * INTERPOLATION_RATE;

			//-- if name is moving downwards, we can go slower
			if (deltaVector.y < 0.0f)
				t.y *= 0.5f;
			else
				t.y *= 4.0f;

			t.x *= absDeltaVector.x;
			t.y *= absDeltaVector.y;
			t.z *= absDeltaVector.z;

			t.x = clamp (0.0f, t.x, 1.0f);
			t.y = clamp (0.0f, t.y, 1.0f);
			t.z = clamp (0.0f, t.z, 1.0f);
			
			newPos.y = hfp.pos.y + (deltaVector.y * t.y);
			newPos.x = hfp.pos.x + (deltaVector.x * t.x);
			newPos.z = hfp.pos.z + (deltaVector.z * t.z);

			hfp.pos = newPos;
		}

		//-- this should not happen
		else if (hfp.time > curTime)
		{
			WARNING (true, ("time bad"));
			hfp.pos = Vector::zero;
		}

		hfp.time = curTime;
		headPoint = hfp.pos;
	}

	else
	{
		const Vector & curHeadPoint = computeCurHeadPoint_o (obj);
		const HeadFramePair hfp(curHeadPoint, curTime);
		s_headMap.insert (std::make_pair (id, hfp));
		headPoint = curHeadPoint;
	}

	return headPoint;
}

//----------------------------------------------------------------------

void CuiObjectTextManager::getObjectFullName(Unicode::String & name, const ClientObject & object)
{
	getObjectFullName(name, object, 0);
}

//----------------------------------------------------------------------

void CuiObjectTextManager::getObjectFullName(Unicode::String & name, const ClientObject & object, float /*rangeToCameraSquared*/)
{
	CreatureObject const * creature = object.asCreatureObject();
	if (creature)
	{
		// @todo -TRF- need to revist API when we add multi-seater support.
		ClientObject const *const riderObject = creature->getRiderDriverCreature() ? creature->getRiderDriverCreature()->asClientObject() : NULL;
		if (riderObject) 
		{
			name = riderObject->getLocalizedName();
		}
		else
		{
			name = creature->getLocalizedName();
		}
	}
	else
	{
		name = object.getLocalizedName();
	}
}

//----------------------------------------------------------------------

void CuiObjectTextManager::drawObjectLabels(Camera const & camera)
{
	// Get frame time.
	float const frameTime = Clock::frameTime();

	// Update the visible object list first.
	updateVisibleObjectList(frameTime);

	CellProperty const * const selfCell = camera.getParentCell();
	Vector const & selfPos = camera.findPosition_w();

	static ClientWorld::ObjectVector cov;
	cov.clear ();

	Object const * const thePlayer = Game::getPlayer();
	float const distanceCameraToPlayer = thePlayer ? thePlayer->getPosition_w().magnitudeBetween(selfPos) : 0.0f;
	
	ClientWorld::findObjectsInRange(selfPos, (CuiPreferences::getObjectNameRange() + distanceCameraToPlayer) * 2.0f, cov);
	
	CreatureObject const * const player = Game::getPlayerCreature ();
	int const drawNetworkIds = CuiPreferences::getDrawNetworkIds();

	ClientWorld::ObjectVector::size_type objectCount = cov.size();
	for (ClientWorld::ObjectVector::size_type objectIndex = 0; objectIndex < objectCount; ++objectIndex)
	{
		Object const * const obj_base = cov[objectIndex];
		ClientObject const * const object = obj_base ? obj_base->asClientObject () : 0;
		if (!object)
			continue;

		CachedNetworkId const id(object->getNetworkId());
		CreatureObject const * creature = object->asCreatureObject ();			
		TangibleObject const * tangible = creature;
		PlayerObject const * playerObject = 0;

		if (creature)
		{
			playerObject = creature->getPlayerObject ();
			
			//-- Mounts: if we've got a mount, add the mount's rider to the object list.
			//   Otherwise, the rider object text would not show up.
			if (creature->getState(States::MountedCreature))
			{
				// @todo -TRF- need to revist API when we add multi-seater support.
				Object const *const riderObject = creature->getRiderDriverCreature();
				if (riderObject)
				{
					// Add rider to list of objects to process.
					cov.push_back(const_cast<Object*>(riderObject));
					++objectCount;
				}
			}
		}
		
		if (!creature)
		{
			tangible = object->asTangibleObject();
		}
		
		if (!tangible)
			continue;
		if (!filterObjectType(drawNetworkIds, player, object))
		{
			continue;
		}
		bool isVisibleThisFrame = false;
		if (object) 
		{
			Appearance const * const app = object->getAppearance();
			if (app && app->getRenderedThisFrame())
			{
				isVisibleThisFrame = true;
			}
		}

		
		float canSeeTime = 0.0f;
		if (!isVisibleThisFrame || !CuiObjectTextManager::canSee(selfPos, selfCell, *object, canSeeTime))
			continue;
		
		setNameRender(*tangible, true, 0.0f, frameTime, false);
	}
}

//----------------------------------------------------------------------

int CuiObjectTextManager::getObjectHeadPointOffset (const NetworkId & id, int * lastFrameOffset)
{
	const HeadMap::iterator it   = s_headMap.find (CachedNetworkId(id));
	if (it != s_headMap.end ())
	{
		const HeadFramePair & hfp = (*it).second;
		if (lastFrameOffset)
			*lastFrameOffset = hfp.lastFrameOffset;
		return (hfp.offset >> 2) + ((hfp.lastFrameOffset * 3) >> 2);
	}

	return 0;
}

//----------------------------------------------------------------------

void CuiObjectTextManager::setObjectHeadPointOffset (const NetworkId & id, int offset)
{
	if (id != NetworkId::cms_invalid)
	{
		CachedNetworkId const cid(id);

		HeadMap::iterator itHead = s_headMap.find(cid);
		if (itHead == s_headMap.end()) 
		{
			s_headMap[cid].offset = offset;
		}
		else
		{
			HeadFramePair & headFrame = itHead->second;
			headFrame.offset = (headFrame.offset + offset) >> 1;
		}
	}
}

//----------------------------------------------------------------------

int CuiObjectTextManager::getObjectHeadPointMaxY    (const NetworkId & id, int * lastFrameMaxY)
{
	const HeadMap::iterator it   = s_headMap.find (CachedNetworkId (id));
	if (it != s_headMap.end ())
	{
		const HeadFramePair & hfp = (*it).second;
		if (lastFrameMaxY)
			*lastFrameMaxY = hfp.lastFrameMaxY;

		return (hfp.maxY >> 4) + ((hfp.lastFrameMaxY * 3) >> 2);
	}

	if (lastFrameMaxY)
		*lastFrameMaxY = 10000;

	return 10000;
}

//----------------------------------------------------------------------

void CuiObjectTextManager::setObjectHeadPointMaxY (const NetworkId & id, int maxY)
{
	if (id != NetworkId::cms_invalid)
	{
		CachedNetworkId const cid(id);
		
		HeadMap::iterator itHead = s_headMap.find(cid);
		if (itHead == s_headMap.end()) 
		{
			s_headMap[cid].maxY = maxY;
		}
		else
		{
			HeadFramePair & headFrame = itHead->second;
			headFrame.maxY = (headFrame.maxY + maxY) >> 1;
		}
	}
}

//----------------------------------------------------------------------

bool  CuiObjectTextManager::getObjectHeadPoint     (const Object & object, const Camera & camera, float offset, UIPoint & pt)
{
	Vector currentHeadPoint = getCurrentObjectHeadPoint_o (object);
//	currentHeadPoint.y *= 1.05f;
	currentHeadPoint.y += offset * 1.05f;
	
	const Vector & headPoint = object.rotateTranslate_o2w (currentHeadPoint);
	Vector screenVect;
	
	if (camera.projectInWorldSpace(headPoint, &screenVect.x, &screenVect.y, 0, false))
	{
		pt.x = static_cast<long>(screenVect.x);
		pt.y = static_cast<long>(screenVect.y);

		return true;
	}

	return false;
}

//----------------------------------------------------------------------

void CuiObjectTextManager::renderTextAbove (const ClientObject & object, const Camera & camera, const Unicode::String & str, float opacity, const UIColor &textColor, UIImageStyle *imageStyle, UIImageStyle* imageStyle2)
{
	//-- Don't render text above objects no longer in the world.
	if (!object.isInWorld())
		return;

	Vector currentHeadPoint = getCurrentObjectHeadPoint_o (object);
//	currentHeadPoint.y *= 1.05f;
	
	const Vector & headPoint = object.rotateTranslate_o2w (currentHeadPoint);
	
	CuiTextManager::TextEnqueueInfo info;
	if (camera.projectInWorldSpace (headPoint, &info.screenVect.x, &info.screenVect.y, &info.screenVect.z, false))
	{
		int maxY = 10000;
		getObjectHeadPointMaxY (object.getNetworkId (), &maxY);

		info.screenVect.y      = std::min (static_cast<float>(maxY), info.screenVect.y);
		//-- pull text forward to 0.9 the distance to be in front of any chat boxes
		info.screenVect.z     *= 0.9f;
		info.worldDistance     = camera.getPosition_w().magnitudeBetween (object.getPosition_w());
		info.backgroundOpacity = 0.0f;
		info.textColor         = textColor;
		info.opacity           = opacity;
		info.imageStyle        = imageStyle;
		info.imageStyle2       = imageStyle2;
		info.id                = object.getNetworkId ();

		if (object.getGameObjectType () == SharedObjectTemplate::GOT_misc_sign)
		{
			info.textWeight     = CuiTextManager::TextEnqueueInfo::TW_heavy;
			info.worldDistance *= 0.25f;
		}

		CuiTextManager::enqueueText (str, info);
	}
}

//----------------------------------------------------------------------

bool CuiObjectTextManager::setNameRender          (const ClientObject & object, bool value, float timein, float timeout, bool overrideInfiniteTimeout)
{
	const CachedNetworkId id (object.getNetworkId ());

	CachedNetworkIdMap::iterator it = s_objectNamesToDraw.find (id);

	if (timeout > 0.0f)
		timeout = std::max (timeout_fadeout, timeout);

	if (it == s_objectNamesToDraw.end ())
	{
		if (value)
		{
			NameTime nt;
			nt.timeOut = timeout;
			nt.timeIn  = timein;
			s_objectNamesToDraw.insert (std::make_pair (id, nt));
			return true;
		}
	}
	else
	{
		NameTime & nt = (*it).second;
		if (value)
		{
			if (overrideInfiniteTimeout || nt.timeOut > 0.0f)
			{
				nt.timeOut = timeout;
				nt.timeIn  = std::min (nt.timeIn, timein);
			}
		}
		else
		{
			//-- if we are trying to toggle off, but timeout is infinite and the current timeout is positive, simply toggle on infinite

			if (timeout <= 0.0f && nt.timeOut > 0.0f)
			{
				nt.timeOut = timeout;
				nt.timeIn  = std::min (nt.timeIn, timein);
			}
			else if (s_objectNamesToDraw.erase (id))
				return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------

bool  CuiObjectTextManager::toggleNameRender       (const ClientObject & object, float timeout, bool overrideInfiniteTimeout)
{
	if (!setNameRender (object, false, 0.0f, timeout, overrideInfiniteTimeout))
		return setNameRender (object, true, 0.0f, timeout, overrideInfiniteTimeout);

	return false;
}

//----------------------------------------------------------------------

bool CuiObjectTextManager::canSee(const Vector & sourcePos, const CellProperty * sourceCell, const Object & targetObject, float & timeFactor)
{
	HeadMap::iterator it = s_headMap.find(CachedNetworkId (targetObject));

	if (it == s_headMap.end())
	{
		const Vector & curHeadPoint = computeCurHeadPoint_o(targetObject);
		const float curTime         = Game::getElapsedTime ();
		
		const std::pair<HeadMap::iterator, bool> retval = s_headMap.insert (std::make_pair (targetObject.getNetworkId (), HeadFramePair (curHeadPoint, curTime)));
		it = retval.first;
	}
	
	DEBUG_FATAL (it == s_headMap.end (), ("bad headmap"));
	
	HeadFramePair & hfp = (*it).second;
	if (!hfp.canSeeChecked)
	{
		hfp.canSee = canSeeInternal (sourcePos, sourceCell, targetObject, hfp.canSeeTimeFactor);
		hfp.canSeeChecked = true;
	}
	
	timeFactor = hfp.canSeeTimeFactor;
	return hfp.canSee;	
}

//----------------------------------------------------------------------

bool CuiObjectTextManager::canSee (const Object & targetObject, float & timeFactor)
{
	const Camera * const camera = Game::getCamera ();
	if (camera)
	{
		const CellProperty * const selfCell      = camera->getParentCell ();
		const Vector &             selfPos       = camera->getAppearanceSphereCenter_w();
	
		return canSee(selfPos, selfCell, targetObject, timeFactor);
	}

	return false;
}

//----------------------------------------------------------------------

void  CuiObjectTextManager::update  ()
{
	static const float TIMEOUT   = 3.0f;
	const float curTime          = Game::getElapsedTime ();

	for (HeadMap::iterator it = s_headMap.begin (); it != s_headMap.end ();)
	{
		const CachedNetworkId id = (*it).first;

		//-- remove deleted objects from the list
		if (!id.getObject ())
			s_headMap.erase (it++);
		else
		{
			HeadFramePair & hfp = (*it).second;
			
			///-- remove timed out objects from the list
			if ((hfp.time + TIMEOUT) < curTime)
				s_headMap.erase (it++);
			else
			{
				hfp.lastFrameOffset = hfp.offset;
				hfp.lastFrameMaxY   = hfp.maxY;
				hfp.offset          = 0;
				hfp.maxY            = 10000;
				hfp.canSeeChecked   = false;
				++it;
			}
		}
	}

	CuiGameColorManager::update(Clock::frameTime());
}

//----------------------------------------------------------------------

void CuiObjectTextManager::transformBox (BoxExtent & box, const Transform & transform)
{
	static Vector vs[8];
	box.getCornerVectors (vs);
	
	Vector tMin (Vector::maxXYZ);
	Vector tMax (Vector::negativeMaxXYZ);
	
	for (int i = 0; i < 8; ++i)
	{
		Vector & v = vs [i];
		v = transform.rotateTranslate_l2p (v);
		
		tMin.x = std::min (v.x, tMin.x);
		tMin.y = std::min (v.y, tMin.y);
		tMin.z = std::min (v.z, tMin.z);
		
		tMax.x = std::max (v.x, tMax.x);
		tMax.y = std::max (v.y, tMax.y);
		tMax.z = std::max (v.z, tMax.z);
	}
	
	box.setMin (tMin);
	box.setMax (tMax);
}

//----------------------------------------------------------------------

void CuiObjectTextManager::computeTransformedBoxExtent (const Object & obj, BoxExtent & boxExtent)
{
	NOT_NULL (obj.getAppearance ());
	
	typedef std::pair<const Object *, Transform> ObjectInfo;
	
	typedef stdvector<ObjectInfo>::fwd ObjectVector;
	static ObjectVector ov;
	ov.clear ();
	
	ov.push_back (ObjectInfo (&obj, Transform::identity));
	
	bool root = true;

	const ClientObject * const clientObject = obj.asClientObject ();
	const CreatureObject * const creature   = clientObject ? clientObject->asCreatureObject () : 0;
	
	while (!ov.empty ())
	{
		const Object * const parent   = ov.back ().first;
		const Transform   transform   = ov.back ().second;
		
		ov.pop_back ();
		
		if (!parent)
			continue;
		
		const int numChildren = parent->getNumberOfChildObjects ();
		
		for (int i = 0; i < numChildren; ++i)
		{
			const Object * const child = parent->getChildObject (i);

			if (!creature || child->asClientObject ())
			{
				Transform t;
				t.multiply (transform, child->getTransform_o2p ());
				ov.push_back (ObjectInfo (child, t));
			}
		}
		
		//----------------------------------------------------------------------
		
		const Appearance * const app = parent->getAppearance ();
		
		if (!app)
			continue;		
		
		BoxExtent theBox;
		
		const SkeletalAppearance2 * const skelApp = app->asSkeletalAppearance2 ();
		if (skelApp)
		{
			theBox.copy (static_cast<const BoxExtent *>(const_cast<SkeletalAppearance2 *>(skelApp)->getExtent ()));
		}
		else
		{
			if (root == true || (dynamic_cast<const FadingTextAppearance *>(app) == 0 && dynamic_cast<const RibbonAppearance *>(app) == 0 && dynamic_cast<const ParticleEffectAppearance *>(app) == 0))
			{		
				const Extent * const extent = app->getExtent ();
				const BoxExtent * const box = dynamic_cast<const BoxExtent *>(extent);
				if (box)
					theBox.copy (box);
			}
		}
		
		if (!theBox.getBox ().isEmpty ())
		{
			BoxExtent tmpBox (theBox.getBox ());
			transformBox     (tmpBox, transform);
			boxExtent.grow   (tmpBox);
		}
		
		root = false;
	}

	// make sure the box cannot leave empty
	if (boxExtent.getBox().isEmpty())
	{
		boxExtent.setMin (-Vector::xyz111);
		boxExtent.setMax ( Vector::xyz111);
	}

	boxExtent.calculateCenterAndRadius ();
}

//----------------------------------------------------------------------

void CuiObjectTextManager::computeBestFitBoxExtent (const Object & foundObject, BoxExtent & box)
{
	if (!foundObject.getAppearance() || !foundObject.getAppearance()->isLoaded())
		return;

	box.setBox(foundObject.getTangibleExtent());
	
	const Vector vMin = box.getMin ();
	const Vector vMax = box.getMax ();
	
	const SkeletalAppearance2 * const skelApp = foundObject.getAppearance () ? foundObject.getAppearance ()->asSkeletalAppearance2 () : 0;
	
	//--
	//-- if the apperance is skeletal, attempt to wrap the rectangle around the head bone
	//--
	
	Vector newMin;
	Vector newMax;
	
	if (skelApp)
	{
		const Skeleton * const skeleton = skelApp->getDisplayLodSkeleton ();
		static const ConstCharCrcString s_head ("head");
		const Transform * const headTransform = (skeleton ? skeleton->findTransform (s_head) : 0);
		if (headTransform)
		{
			const Vector & headPos = headTransform->getPosition_p ();
			
			Vector delta       = vMax - vMin;
			const Vector deltaMargin = delta * 0.5f;
			
			newMin = headPos - deltaMargin;
			newMax = headPos + deltaMargin;
			
			newMax.x = std::min (vMax.x, newMax.x);
			newMin.x = std::max (vMin.x, newMin.x);				
			newMax.y = std::min (vMax.y, newMax.y);
			newMin.y = std::max (vMin.y, newMin.y);				
			newMax.z = std::min (vMax.z, newMax.z);
			newMin.z = std::max (vMin.z, newMin.z);				
		}
		
		//-- skeleton has no head, just wrap the rectangle around the whole appearance
		else
		{
			Vector delta = (vMax - vMin);
			delta.x *= 0.2f;
			delta.y *= 0.10f;
			delta.z *= 0.2f;
			newMin = vMin + delta;
			newMax = vMax - delta;
		}
	}
	
	//-- if the the appearance is non-skeletal, try some other mojo
	else
	{			
		const CollisionProperty * const prop = foundObject.getCollisionProperty();
		const BaseExtent * const baseExtent = prop ? prop->getExtent_l () : 0;
		
		bool handled = false;
		
		//-- if the appearance has a spherical collision extent, try to wrap the rectangle
		//-- around the intersection a shrunken collision sphere and the apperance box extent
		
		if (baseExtent)
		{
			const ExtentType type = baseExtent->getType ();
			if (type == ET_Simple)
			{
				const SimpleExtent * const simple = safe_cast<const SimpleExtent *>(prop->getExtent_l ());
				const bool isSphere = simple->getShape ().getBaseType () == MultiShape::MSBT_Sphere;
				
				if (isSphere)
				{
					const Sphere & sphere = simple->getBoundingSphere ();
					const Vector & sphereCenter = sphere.getCenter ();
					const float    sphereRadius = sphere.getRadius ();
					
					Vector sphereOffset(sphereRadius * 0.5f, sphereRadius, sphereRadius * 0.5f);

					newMin = sphereCenter - sphereOffset;
					newMax = sphereCenter + sphereOffset;
					
					newMax.x = std::min (vMax.x, newMax.x);
					newMin.x = std::max (vMin.x, newMin.x);
					newMax.y = std::min (vMax.y, newMax.y);
					newMin.y = std::max (vMin.y, newMin.y);				
					newMax.z = std::min (vMax.z, newMax.z);
					newMin.z = std::max (vMin.z, newMin.z);
					
					handled = true;
				}
			}
		}
		
		//-- just shrink the box down a bit if nothing else works
		if (!handled)
		{
			Vector delta = (vMax - vMin);
			delta.x *= 0.1f;
			delta.y *= 0.05f;
			delta.z *= 0.1f;
			newMin = vMin + delta;
			newMax = vMax - delta;
		}
		
	}

	box.setMin(newMin);
	box.setMax(newMax);
}

//----------------------------------------------------------------------

UIImageStyle * CuiObjectTextManager::getObjFactionIcon(const TangibleObject & obj)
{
	uint32 faction = obj.getPvpFaction();
	if (faction)
	{
		std::map<uint32, UIImageStyle*>::iterator i = s_factionImageStyles.find(faction);
		if (i != s_factionImageStyles.end())
			return (*i).second;
	}
	return 0;
}

//----------------------------------------------------------------------

bool CuiObjectTextManager::canSee(Object const * const obj)
{
	bool isVisible = false;

	if (obj) 
	{
		CachedNetworkIdMap::iterator itObj = s_objectNamesToDraw.find(CachedNetworkId(obj->getNetworkId()));
		isVisible = (itObj != s_objectNamesToDraw.end()); 
	}

	return isVisible;
}

//----------------------------------------------------------------------

void CuiObjectTextManager::getVisibleObjects(stdvector<Object*>::fwd & objectVector)
{
	objectVector.reserve(s_objectNamesToDraw.size());
	
	for (CachedNetworkIdMap::iterator it = s_objectNamesToDraw.begin (); it != s_objectNamesToDraw.end (); ++it)
	{
		Object * const object = it->first.getObject();
		if (object) 
		{
			objectVector.push_back(object);
		}
	}
}

//----------------------------------------------------------------------

void CuiObjectTextManager::updateVisibleObjectList(float const frameTime)
{
	for (CachedNetworkIdMap::iterator it = s_objectNamesToDraw.begin(); it != s_objectNamesToDraw.end(); /*++it*/)
	{
		NameTime & nt = (*it).second;
		
		if (nt.timeIn > 0.0f)
			nt.timeIn = std::max (0.0f, nt.timeIn - frameTime);
		
		if (nt.timeOut > 0.0f)
		{
			nt.timeOut -= frameTime;

			if (nt.timeOut <= 0.0f)
			{
				s_objectNamesToDraw.erase(it++);
				continue;
			}
		}

		++it;
	}
}

//----------------------------------------------------------------------

void CuiObjectTextManager::updateNameWidgetHeight(CachedNetworkId const & id, UIScalar const offset)
{
	CachedNetworkIdMap::iterator itObj = s_objectNamesToDraw.find(id);
	if (itObj != s_objectNamesToDraw.end()) 
	{
		NameTime & nt = itObj->second;
		nt.nameWidgetOffset = offset;
	}
}

//----------------------------------------------------------------------

UIScalar CuiObjectTextManager::getNameWidgetHeight(CachedNetworkId const & id)
{
	UIScalar offset = 0;

	CachedNetworkIdMap::iterator itObj = s_objectNamesToDraw.find(id);
	if (itObj != s_objectNamesToDraw.end()) 
	{
		NameTime & nt = itObj->second;
		offset = nt.nameWidgetOffset;
	}

	return offset;
}


//======================================================================
