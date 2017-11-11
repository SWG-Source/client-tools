//======================================================================
//
// CuiGameColorManager.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiGameColorManager.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroupObject.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Os.h"
#include "sharedGame/PvpData.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "swgSharedUtility/States.def"

#include "UITypes.h"

//======================================================================

namespace CuiGameColorManagerNamespace
{
	int const cs_blinksPerSecond = 2;
	float const cs_secondsToBlink = 5.0f;

	class NameBlinkManager
	{
	public:
		NameBlinkManager();
		void registerObjectToBlink(NetworkId const Id);
		bool shouldObjectNameBlink(NetworkId const Id);
		void update(float const deltaTimeSeconds);

	private:
		NameBlinkManager(NameBlinkManager const & copy);
		NameBlinkManager & operator = (NameBlinkManager const & copy);

	private:

		typedef std::map<NetworkId, float /*secondsToBlink*/> ObjectToTime;
		ObjectToTime m_registeredObjects;
	};

	NameBlinkManager s_nameBlinkManager;

	CuiGameColorManager::Type getAttackColorType(NetworkId const & id, CuiGameColorManager::Type defaultType);

	CuiGameColorManager::Type internalFindTypeForObject (const TangibleObject & obj, bool ignoreTarget)
	{
		const CreatureObject * const player = Game::getPlayerCreature ();
		if (!player)
			return CuiGameColorManager::T_default;
		if (player == &obj)
			return CuiGameColorManager::T_player;

		const NetworkId & id = obj.getNetworkId ();

		if (!ignoreTarget && id.isValid ())
		{
			if (player->getCombatTarget() == id && Game::isSpace())
			{
				return CuiGameColorManager::T_combatTarget;
			}

			if (player->getLookAtTarget() == id && player->getIntendedTarget() == id && !Game::isSpace())
			{
				return CuiGameColorManager::T_combatTarget;
			}

			if (player->getLookAtTarget() == id)
			{
				return CuiGameColorManager::T_target;
			}

			if (player->getIntendedTarget() == id && !Game::isSpace())
			{
				return CuiGameColorManager::T_intendedTarget;
			}
		}

		if (obj.getPvpFlags() & PvpStatusFlags::WillBeDeclared)
		{
			s_nameBlinkManager.registerObjectToBlink(id);
			if (s_nameBlinkManager.shouldObjectNameBlink(id))
			{
				if(Game::isSpace())
					return CuiGameColorManager::T_spaceWillBeDeclared;

				return CuiGameColorManager::T_willBeDeclared;
			}
		}

		if (obj.getPvpFlags() & PvpStatusFlags::WasDeclared)
		{
			s_nameBlinkManager.registerObjectToBlink(id);
			if (s_nameBlinkManager.shouldObjectNameBlink(id))
			{
				return CuiGameColorManager::T_wasDeclared;
			}
		}
		
		if (obj.getGameObjectType () == SharedObjectTemplate::GOT_corpse)
			return CuiGameColorManager::T_corpse;
	
		else
		{
			const GroupObject * const group = safe_cast<const GroupObject *>(player->getGroup ().getObject ());
			
			if (group && id.isValid ())
			{
				bool isLeader = false;
				if (group->findMember (id, isLeader))
				{
					if (isLeader)
						return CuiGameColorManager::T_groupLeader;
					else
						return CuiGameColorManager::T_group;
				}
			}

			const CreatureObject * creature = 0;

			if (obj.getObjectType () == SharedCreatureObjectTemplate::SharedCreatureObjectTemplate_tag)
			{
				creature = safe_cast<const CreatureObject *>(&obj);
				NOT_NULL (creature);
				if (creature->isDead ())
					return CuiGameColorManager::T_corpse;
			}

			if (obj.getGameObjectType () == SharedObjectTemplate::GOT_ship_mining_asteroid_static)
				return CuiGameColorManager::T_miningAsteroidStatic;

			if (obj.getGameObjectType () == SharedObjectTemplate::GOT_ship_mining_asteroid_dynamic)
				return CuiGameColorManager::T_miningAsteroidDynamic;

			if (creature && obj.isInvulnerable())
				return CuiGameColorManager::T_npc;

			if (obj.isEnemy())
				return getAttackColorType(id, CuiGameColorManager::T_canAttackYou);

			if (obj.isAttackable ())
			{
				if (!obj.isPlayer ())
				{
					if ((obj.getPvpFlags() & PvpStatusFlags::CanAttackYou) != 0)
					{
						if (obj.hasCondition (TangibleObject::C_aggressive) || obj.getPvpFaction () != 0)
							return getAttackColorType(id, CuiGameColorManager::T_canAttackYou);
					}
					
					return getAttackColorType(id, CuiGameColorManager::T_attackable);
				}
				else
				{
					if ((obj.getPvpFlags() & PvpStatusFlags::CanAttackYou) != 0)
						return getAttackColorType(id, CuiGameColorManager::T_canAttackYou);
				}
			}

			if (obj.isSameFaction (*player))
			{
				if (obj.isPlayer ())
					return CuiGameColorManager::T_faction;
				else
					return CuiGameColorManager::T_factionNpc;
			}

			else if (creature)
			{
				if (obj.isPlayer ())
					return CuiGameColorManager::T_player;

				else
					return CuiGameColorManager::T_npc;
			}
		}			
		
		return CuiGameColorManager::T_default;
	}

	UIColor     s_colors         [CuiGameColorManager::T_numTypes];
	std::string s_names          [CuiGameColorManager::T_numTypes];
	float       s_ranges         [CuiGameColorManager::T_numTypes];
	bool        s_cullByBuilding [CuiGameColorManager::T_numTypes];

	bool    s_installed = false;

	void install ()
	{
		if (s_installed)
			return;

		s_installed = true;

#define MAKE_NAME_COLOR(a, b, c) \
		s_names          [CuiGameColorManager::T_##a]       = #a; \
		s_ranges         [CuiGameColorManager::T_##a]       = b; \
		s_colors         [CuiGameColorManager::T_##a]       = c;

		MAKE_NAME_COLOR (default,        1.0f,    UIColor::white);
		MAKE_NAME_COLOR (player,         1.5f,    UIColor (0x00, 0xcc, 0xdd));
		MAKE_NAME_COLOR (group,          1000.0f, UIColor (0x00, 0xff, 0x00));
		MAKE_NAME_COLOR (groupLeader,    1000.0f, UIColor (0xff, 0x44, 0xcc));
		MAKE_NAME_COLOR (npc,            1.0f,    UIColor (0xff, 0xff, 0xff));
		MAKE_NAME_COLOR (attackable,     1.0f,    UIColor (0xff, 0xdd, 0x00));
		MAKE_NAME_COLOR (canAttackYou,   1.5f,    UIColor (0xff, 0x00, 0x00));
		MAKE_NAME_COLOR (canAttackYouDark,1.5f,   UIColor (0xbf, 0x00, 0x00));
		MAKE_NAME_COLOR (willBeDeclared, 1.5f,    UIColor (0xaa, 0xff, 0xaa));
		MAKE_NAME_COLOR (wasDeclared,    1.5f,    UIColor (0x44, 0xff, 0x44));	
		MAKE_NAME_COLOR (faction,        1.0f,    UIColor (0xaa, 0x00, 0xff));
		MAKE_NAME_COLOR (factionNpc,     1.0f,    UIColor (0xee, 0xaa, 0xff));
		MAKE_NAME_COLOR (target,         1000.0f, UIColor (0x00, 0x44, 0xff));
		MAKE_NAME_COLOR (combatTarget,   1000.0f, UIColor (0xff, 0xaa, 0x00));
		MAKE_NAME_COLOR (intendedTarget, 1000.0f, UIColor (0x00, 0xff, 0x00));
		MAKE_NAME_COLOR (corpse,         1.0f,    UIColor (0xbb, 0xbb, 0xbb));
		MAKE_NAME_COLOR (yourCorpse,     1000.0f, UIColor (0xbb, 0xbb, 0xbb));
		MAKE_NAME_COLOR (turretTarget,   1000.0f, UIColor (0xbb, 0xbb, 0xbb));
		MAKE_NAME_COLOR (miningAsteroidDynamic, 4000.0f, UIColor(0xaa, 0xff, 0xaa));
		MAKE_NAME_COLOR (miningAsteroidStatic, 4000.0f, UIColor(0x44, 0xff, 0x44));
		MAKE_NAME_COLOR (assist,         1000.0f, UIColor (0x80, 0x80, 0x80));
		MAKE_NAME_COLOR (spaceWillBeDeclared, 1.5f, UIColor::red);

#undef MAKE_NAME_COLOR
	}
}

// ======================================================================

CuiGameColorManagerNamespace::NameBlinkManager::NameBlinkManager()
: m_registeredObjects()
{
}

//----------------------------------------------------------------------

void CuiGameColorManagerNamespace::NameBlinkManager::registerObjectToBlink(NetworkId const Id)
{
	ObjectToTime::const_iterator ii = m_registeredObjects.find(Id);
	if (ii == m_registeredObjects.end())
	{
		m_registeredObjects[Id] = cs_secondsToBlink;
	}
}

//----------------------------------------------------------------------

bool CuiGameColorManagerNamespace::NameBlinkManager::shouldObjectNameBlink(NetworkId const Id)
{
	ObjectToTime::const_iterator ii = m_registeredObjects.find(Id);
	if (ii != m_registeredObjects.end())
	{
		float const secondsToBlink = ii->second;
		float const fractionPart = secondsToBlink - static_cast<float>(static_cast<int>(secondsToBlink));
		float const blinksPerSecondReciprocal = 1.0f / static_cast<float>(cs_blinksPerSecond);

		float valueMod = fractionPart - blinksPerSecondReciprocal;

		while (valueMod > blinksPerSecondReciprocal)
		{
			valueMod -= blinksPerSecondReciprocal;
		}

		return valueMod > 0.0f;
	}

	return false;
}

//----------------------------------------------------------------------

void CuiGameColorManagerNamespace::NameBlinkManager::update(float const deltaTimeSeconds)
{
	typedef std::vector<ObjectToTime::iterator> Iterators;
	Iterators toErase;

	{
		ObjectToTime::iterator ii = m_registeredObjects.begin();
		ObjectToTime::iterator iiEnd = m_registeredObjects.end();

		for (; ii != iiEnd; ++ii)
		{
			ii->second -= deltaTimeSeconds;

			if (ii->second < 0.0f)
			{
				toErase.push_back(ii);
			}
		}
	}

	{
		Iterators::iterator ii = toErase.begin();
		Iterators::iterator iiEnd = toErase.end();

		for (; ii != iiEnd; ++ii)
		{
			m_registeredObjects.erase(*ii);
		}
	}
}

//----------------------------------------------------------------------

CuiGameColorManager::Type CuiGameColorManagerNamespace::getAttackColorType(NetworkId const & id, CuiGameColorManager::Type defaultType)
{
	if (Game::isSpace() || !CuiPreferences::getGroundRadarBlinkCombatEnabled())
		return defaultType;

	PlayerObject * playerObject = Game::getPlayerObject();

	if (playerObject && playerObject->isInPlayerHateList(id))
	{
		s_nameBlinkManager.registerObjectToBlink(id);

		if (!s_nameBlinkManager.shouldObjectNameBlink(id))
			return CuiGameColorManager::T_canAttackYou;
		else
			return CuiGameColorManager::T_canAttackYouDark;
	}

	return defaultType;
}

// ======================================================================

using namespace CuiGameColorManagerNamespace;

// ======================================================================

CuiGameColorManager::Type   CuiGameColorManager::findTypeForObject (const TangibleObject & obj, bool ignoreTarget)
{
	if (!s_installed)
		install ();

	return internalFindTypeForObject (obj, ignoreTarget);
}

//----------------------------------------------------------------------

UIColor const CuiGameColorManager::findColorForObject (const TangibleObject & obj, bool ignoreTarget, bool allowOverride)
{
	if (!s_installed)
		install ();

	if(allowOverride)
	{
		uint32 rawOverrideColor = obj.getOverriddenMapColor();
		UIColor overrideColor;
		overrideColor.Set(rawOverrideColor);
		if(overrideColor.a != 0)
		{
			return overrideColor;
		}
	}

	Type t = internalFindTypeForObject (obj, ignoreTarget);
	UNREF (t);

	return s_colors [static_cast<int>(t)];
}

//----------------------------------------------------------------------

const UIColor & CuiGameColorManager::getColorForType    (Type type)
{
	if (!s_installed)
		install ();

	return s_colors [static_cast<int>(type)];
}

//----------------------------------------------------------------------

float CuiGameColorManager::getRangeModForType (Type type)
{
	if (!s_installed)
		install ();

	return s_ranges [static_cast<int>(type)];
}

void CuiGameColorManager::update(float const deltaTimeSeconds)
{
	s_nameBlinkManager.update(deltaTimeSeconds);
}

//----------------------------------------------------------------------

UIColor const CuiGameColorManager::getCombatColor()
{
	UIColor result(UIColor::red);
	static float combatColorPercent = 0.0f;
	static bool direction = true;
	static int lastUpdate = 0;

	if (lastUpdate != Os::getNumberOfUpdates())
	{
		lastUpdate = Os::getNumberOfUpdates();

		if (direction)
		{
			combatColorPercent += Clock::frameTime() * 2.0f;
		}
		else
		{
			combatColorPercent -= Clock::frameTime() * 2.0f;
		}

		if (   (combatColorPercent > 1.0f)
			|| (combatColorPercent < 0.0f))
		{
			combatColorPercent = clamp(0.0f, combatColorPercent, 1.0f);
			direction = !direction;
		}
	}

	result.g = static_cast<uint8>(255.0f * combatColorPercent);

	return result;
}

//----------------------------------------------------------------------

bool CuiGameColorManager::objectHasOverrideColor(const TangibleObject & obj)
{
	uint32 rawOverrideColor = obj.getOverriddenMapColor();
	UIColor overrideColor;
	overrideColor.Set(rawOverrideColor);
	return overrideColor.a != 0;
}

//======================================================================

