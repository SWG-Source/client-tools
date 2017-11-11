//======================================================================
//
// CuiCombatManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiCombatManager.h"

#include "UIUtils.h"
#include "clientGame/BuildingObject.h"
#include "clientGame/ClientCombatActionInfo.h"
#include "clientGame/ClientCombatPlaybackManager.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientCommandQueueEntry.h"
#include "clientGame/ClientTextManager.h"
#include "clientGame/CommunityManager.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroupObject.h"
#include "clientGame/ProsePackageManagerClient.h"
#include "clientGame/ShipController.h"
#include "clientGame/ShipObject.h"
#include "clientGame/WeaponObject.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/RenderWorld.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiAction.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiDamageManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiStringGrammarManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringIdsCombatSpam.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiTextManager.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/Crc.h"
#include "sharedGame/Command.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/PvpData.h"
#include "sharedGame/ShipChassis.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/VectorArgb.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueCombatSpam.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedUtility/LocalMachineOptionManager.h"
#include "sharedUtility/DataTable.h"
#include "UnicodeUtils.h"
#include "Unicode.h"
#include <algorithm>
#include <cstdio>
#include <deque>
#include <list>

//======================================================================

uint32                                         CuiCombatManager::ms_combatQueueSequenceId;
CachedNetworkId                                CuiCombatManager::ms_lastTargetedIdInQueue;
CuiCombatManager::CuiCombatManagerAction *     CuiCombatManager::ms_action;

std::string CuiCombatManager::ms_consoleActionName              = "homing_test";
int         CuiCombatManager::ms_consoleAttackerPostureEndIndex = 0;
int         CuiCombatManager::ms_consoleDefenderPostureEndIndex = 0;
int         CuiCombatManager::ms_consoleDefenderDefense         = 0;
bool        CuiCombatManager::ms_showDamageOverHeadOthers;
bool        CuiCombatManager::ms_showDamageOverHeadSelf;
bool        CuiCombatManager::ms_showDamageSnare = true;
bool        CuiCombatManager::ms_showDamageGlancingBlow = true;
bool        CuiCombatManager::ms_showDamageCriticalHit = true;
bool        CuiCombatManager::ms_showDamageLucky = true;
bool        CuiCombatManager::ms_showDamageDot = true;
bool        CuiCombatManager::ms_showDamageBleed = true;
bool        CuiCombatManager::ms_showDamageHeal = true;
bool        CuiCombatManager::ms_showDamageFreeshot = true;
float       CuiCombatManager::ms_damageDoneToMeSizeModifier = 1.0f;
float       CuiCombatManager::ms_damageDoneToOthersSizeModifier = 1.0f;
float       CuiCombatManager::ms_nonDamageDoneToMeSizeModifier = 1.0f;
float       CuiCombatManager::ms_nonDamageDoneToOthersSizeModifier = 1.0f;
bool        CuiCombatManager::ms_showEnteringCombat = true;

//----------------------------------------------------------------------

namespace CuiCombatManagerNamespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const CuiCombatManager::Messages::CombatSpamReceived::Payload&, CuiCombatManager::Messages::CombatSpamReceived > 
			combatSpamReceived;
	}

	typedef stdlist<CachedNetworkId>::fwd CachedNetworkIdVector;
	CachedNetworkIdVector s_targetHistory;

	bool s_installed = false;

	class Listener : public MessageDispatch::Receiver
	{
	public:

		Listener () :
		MessageDispatch::Receiver ()
		{
			connectToMessage (Game::Messages::SCENE_CHANGED);
		}

		//----------------------------------------------------------------------

		void receiveMessage(const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message)
		{
			if (message.isType (Game::Messages::SCENE_CHANGED))
			{
				CuiCombatManager::reset ();
			}
		}
	};

	Listener * s_listener = 0;

	//----------------------------------------------------------------------		
	class MyCallback : 
	public MessageDispatch::Callback
	{
	public:
		MyCallback () : MessageDispatch::Callback () {}
		
		void onIncapacityChanged (const CreatureObject & payload)
		{
			if (&payload == static_cast<const Object*>(Game::getPlayer ()))
			{
				CuiCombatManager::setCombatTarget (NetworkId::cms_invalid);
			}
		} //lint !e1762 //stfu noob

		void onLookAtTargetChanged (const CreatureObject & payload)
		{
			if (&payload == static_cast<const Object*>(Game::getPlayer ()))
			{
				CuiCombatManager::pushTargetHistory ();
			}
		} //lint !e1762 //stfu noob
	};
	
	MyCallback s_callback; 

	//----------------------------------------------------------------------

	bool canAttack(ClientObject *obj)
	{
		if (obj)
		{
			TangibleObject * const tangibleObj = obj->asTangibleObject ();
			if (tangibleObj && (tangibleObj->getPvpFlags() & PvpStatusFlags::YouCanAttack))
				return true;
		}
		return false;
	}

	const int MAX_TARGET_HISTORY = 10;

	bool s_debugDifficulty = false;

	int s_numConLevels = 0;

	typedef std::map<int, int> ConIndexMap;
	typedef std::map <int, ConIndexMap> PlayerLevelToConColorMap;
	PlayerLevelToConColorMap s_playerLevelToConColorMap;

	typedef std::pair<StringId, UIColor> ConLevelData;

	std::vector<ConLevelData> s_conLevelData;

	const int s_conIndexInvulnerable = 0;
	const int s_conIndexSelf = 5;

	int getConIndex(int levelSelf, int levelTarget);
	int getConIndex (CreatureObject const & enemy);

	struct TargetResult
	{
	public:
		Object const * const m_targetingObject;
		Object const * const m_currentTestTarget;

		typedef std::pair<float /* distance from squared */, Object *> Candidate;

		// distance is from targeting object
		Candidate m_closest;
		Candidate m_farthest;
		// distance is from currentTestTarget
		Candidate m_closestLessThanTestTarget;
		Candidate m_closestGreaterThanTestTarget;

		TargetResult(Object const * targetingObject, Object const * currentTestTarget);

		void testCandidate(Object * candidate);
	private:

		TargetResult();
		TargetResult(TargetResult const & copy);
		TargetResult & operator = (TargetResult const & copy);
	};

	typedef bool (*ActorAndTargetHaveReletionship)(Object const * actor, Object const * target);

	bool targetIsCreature(Object const * actor, Object const * target);
	bool targetIsCreatureAndFriendOfActor(Object const * actor, Object const * target);
	bool targetIsAttackable(Object const * actor, Object const * target);

	void findTargetsWithRelationship(Object const * targetingObject, ActorAndTargetHaveReletionship function, ClientWorld::ObjectVector & potentialObjects, TargetResult & result);

	typedef Object * (*GetDirectionTarget)(TargetResult const & result);

	Object * getNextTarget(TargetResult const & result);
	Object * getPreviousTarget(TargetResult const & result);

	CachedNetworkId const & cycleTargetsForPlayer(ActorAndTargetHaveReletionship relationShipFunction, GetDirectionTarget directionFunction, bool useIntendedTarget);

	bool isTargetCycleOk(TangibleObject const & obj);

	struct DelayedCombatActionObject
	{
		DelayedCombatActionObject(Unicode::String & flyText, float fadeTime, VectorArgb & color, float textScale, bool fadeOut, int damageAmount = 0, bool showText = true)
		: m_flyText(flyText)
		, m_fadeTime(fadeTime)
		, m_color(color)
		, m_textScale(textScale)
		, m_fadeOut(fadeOut)
		, m_elapsedTime(-1.f)
		, m_damageAmount(damageAmount)
		, m_showText(showText)
		{
		}

		DelayedCombatActionObject()
		: m_flyText()
		, m_fadeTime(0.f)
		, m_color()
		, m_textScale(1.f)
		, m_fadeOut(true)
		, m_elapsedTime(0.f)
		, m_damageAmount(0)
		, m_showText(false)
		{
		}

		Unicode::String m_flyText;
		float           m_fadeTime;
		VectorArgb      m_color;
		float           m_textScale;
		bool            m_fadeOut;
		float           m_elapsedTime;
		int             m_damageAmount;
		bool            m_showText;
	};

	typedef std::pair<NetworkId, NetworkId>              CombatActionPair;
	typedef std::list<DelayedCombatActionObject>         CombatActionList;
	typedef std::map<CombatActionPair, CombatActionList> DelayedCombatActionMap;

	DelayedCombatActionMap s_delayedDamageAction;

	VectorArgb s_damageToPlayerColor = VectorArgb::solidRed;
	VectorArgb s_damageToTargetColor = VectorArgb::solidYellow;
	VectorArgb s_groupMemberDamage   = VectorArgb::solidGray;
	VectorArgb s_otherDamageColor    = VectorArgb::solidWhite;

	// this is the maximum amount of time the action is in the list
	// before it is automatically removed from the list (fix for animations that are missing the hit/attack event)
	// the name of the attacker is logged so that this can be caught and removed
	const float s_maxDelayForCombatAction = 1.f;

	void processCombatActions(DelayedCombatActionMap::iterator &itr, const NetworkId & defenderId, bool updateExpiredOnly = false);
	bool attackerOrDefenderInPlayerGroup(const NetworkId &attackerId, const NetworkId &defenderId);

	//-- combat spam

	// this is an attack info message
	// the message will be displayed as a series of phrases that the player
	// can selectively filter out:		OLD----------------						NEW-------------------
	//    (attacker)					"Dave attacks"							"Dave attacks"
	//    (defender)					"Tom"									"Tom"
	//    (attack)						"with Stopping Shot"					"with Stopping Shot"
	//    (weapon)						"using a CDEF Pistol"					"using CDEF Pistol"
	//    (success connect)				"and"									"and"
	//    (success)						"hits"									"hits"
	//    (total damage)				"for 40 points"							"for 40 points"
	//    (damage detail start)			"("										"("
	//    (primary damage)				"30 points"								"30"
	//    (damage type)					"of energy damage"						"energy"
	//    (elemental damage connect)	"and"									"and"
	//    (elemental damage)			"10 points"								"10"
	//    (elemental type)				"of acid damage"						"acid"
	//    (damage detail end)			")"										")"
	//    (damage end sentence)         "."										"."
	//    (armor protection)			"Tom's Chitin Armor Left Bicep"			"Armor"
	//    (protection amount)			"absorbed 20 points"					"absorbed 20 points"
	//    (raw damage connect)          "out of"								"out of"
	//    (raw damage)					"60 possible points damage"				"60"
	//    (absorb end sentence)			"."										"."
	// the player may also flag that they want to just see the raw data 
	// Must be consecutive, starting with 0
	enum SentenceStructure
	{
		SENTENCE_STRUCTURE_ATTACKER					=  0,
		SENTENCE_STRUCTURE_DEFENDER					=  1,
		SENTENCE_STRUCTURE_ATTACK					=  2,
		SENTENCE_STRUCTURE_WEAPON					=  3,
		SENTENCE_STRUCTURE_SUCCESS_CONNECT			=  4,
		SENTENCE_STRUCTURE_SUCCESS					=  5,
		SENTENCE_STRUCTURE_TOTAL_DAMAGE				=  6,
		SENTENCE_STRUCTURE_DAMAGE_DETAIL_START		=  7,
		SENTENCE_STRUCTURE_PRIMARY_DAMAGE			=  8,
		SENTENCE_STRUCTURE_DAMAGE_TYPE				=  9,
		SENTENCE_STRUCTURE_ELEMENTAL_DAMAGE_CONNECT = 10,
		SENTENCE_STRUCTURE_ELEMENTAL_DAMAGE			= 11,
		SENTENCE_STRUCTURE_ELEMENTAL_TYPE			= 12,
		SENTENCE_STRUCTURE_DAMAGE_DETAIL_END		= 13,
		SENTENCE_STRUCTURE_DAMAGE_END_SENTENCE		= 14,
		SENTENCE_STRUCTURE_ARMOR_PROTECTION			= 15,
		SENTENCE_STRUCTURE_PROTECTION_AMOUNT		= 16,
		SENTENCE_STRUCTURE_RAW_DAMAGE_CONNECT       = 17,
		SENTENCE_STRUCTURE_RAW_DAMAGE				= 18,
		SENTENCE_STRUCTURE_ABSORB_END_SENTENCE		= 19,
		SENTENCE_STRUCTURE_NUMBER_STRUCTURES		= 20		// Always leave this on the end, 1 greater than the last value
	};
	enum SentenceStructureBrief
	{
		SENTENCE_STRUCTURE_ATTACKER_BRIEF			=  0,
		SENTENCE_STRUCTURE_SUCCESS_BRIEF			=  1,
		SENTENCE_STRUCTURE_DEFENDER_BRIEF			=  2,
		SENTENCE_STRUCTURE_TOTAL_DAMAGE_BRIEF		=  3,
		SENTENCE_STRUCTURE_NUMBER_STRUCTURES_BRIEF	=  4		// Always leave this on the end, 1 greater than the last value
	};

	// The arrays setup in the install() function will be used for order, and
	// will then be used for replacement with strings so proper spacing occurs.  The final function
	// which puts the strings together will include rules to toUpper the first character of each
	// sentence and remove excess spaces (which is easier than intelligently adding spaces on-the-fly).
	// Assumption:  Single-threaded client.
	short			ms_spamOrder[SENTENCE_STRUCTURE_NUMBER_STRUCTURES];
	short			ms_spamOrderBrief[SENTENCE_STRUCTURE_NUMBER_STRUCTURES_BRIEF];
	Unicode::String	ms_spamStrings[SENTENCE_STRUCTURE_NUMBER_STRUCTURES];
	bool			ms_spamStringsSet[SENTENCE_STRUCTURE_NUMBER_STRUCTURES];
	Unicode::String	ms_spamStringsBrief[SENTENCE_STRUCTURE_NUMBER_STRUCTURES_BRIEF];
	bool			ms_spamStringsSetBrief[SENTENCE_STRUCTURE_NUMBER_STRUCTURES_BRIEF];
	Unicode::String ms_space = Unicode::narrowToWide(" ");
	Unicode::String ms_period = Unicode::narrowToWide(".");


	const float DPS_TIME_INTERVAL = 10.0f;
	int ms_damageDone = 0;
	int ms_damageTaken = 0;
	int ms_damageDoneTotal = 0;
	int ms_damageTakenTotal = 0;
	std::vector<std::pair<int, float> > ms_damageDoneRecords;
	std::vector<std::pair<int, float> > ms_damageTakenRecords;

	void addDamageTaken(int damageAmount)
	{
		ms_damageTaken += damageAmount;
		ms_damageTakenTotal += damageAmount;
		ms_damageTakenRecords.push_back(std::make_pair(damageAmount, DPS_TIME_INTERVAL));
	}

	void clearDamageRecords()
	{
		ms_damageDone = 0;
		ms_damageDoneRecords.clear();
		ms_damageTaken = 0;
		ms_damageTakenRecords.clear();
	}

	void updateDamageRecords(float deltaTime)
	{
		std::vector<std::pair<int, float> >::iterator i;
		i = ms_damageDoneRecords.begin();
		while(i != ms_damageDoneRecords.end())
		{
			(*i).second -= deltaTime;
			if((*i).second <= 0.0f)
			{
				ms_damageDone -= (*i).first;
				i = ms_damageDoneRecords.erase(i);				
			}
			else
				++i;
		}
		std::vector<std::pair<int, float> >::iterator j;
		j = ms_damageTakenRecords.begin();
		while(j != ms_damageTakenRecords.end())
		{
			(*j).second -= deltaTime;
			if((*j).second <= 0.0f)
			{
				ms_damageTaken -= (*j).first;
				j = ms_damageTakenRecords.erase(j);				
			}
			else
				++j;
		}
	}
}

//----------------------------------------------------------------------

CuiCombatManagerNamespace::TargetResult::TargetResult(Object const * const targetingObject, Object const * const currentTestTarget)
: m_targetingObject(targetingObject)
, m_currentTestTarget(currentTestTarget)
, m_closest()
, m_farthest()
, m_closestLessThanTestTarget()
, m_closestGreaterThanTestTarget()
{
	NOT_NULL(targetingObject);

	m_closest.first = FLT_MAX;
	m_farthest.first = 0.0f;
	m_closestLessThanTestTarget.first = 0.0f;
	m_closestGreaterThanTestTarget.first = FLT_MAX;
}

// ----------------------------------------------------------------------

void CuiCombatManagerNamespace::TargetResult::testCandidate(Object * const candidate)
{
	NOT_NULL(candidate);
	NOT_NULL(m_targetingObject);

	if (candidate == 0)
	{
		return;
	}

	if (m_targetingObject == 0)
	{
		return;
	}

	if (m_targetingObject == candidate)
	{
		return;
	}

	// only test against things that were on screen
	if (!RenderWorld::wasObjectRenderedThisFrame(candidate->getNetworkId()))
	{
		return;
	}

	Vector const candidate_w(candidate->getPosition_w());
	Vector const targetingObject_w(m_targetingObject->getPosition_w());
	Vector const currentTestTarget_w((m_currentTestTarget != 0) ? m_currentTestTarget->getPosition_w() : targetingObject_w);

	float const targetingObjectToCandidateSqr = candidate_w.magnitudeBetweenSquared(targetingObject_w);
	float const targetingObjectToCurrentTestTargetSqr = targetingObject_w.magnitudeBetweenSquared(currentTestTarget_w);

	if (targetingObjectToCandidateSqr < m_closest.first)
	{
		m_closest.first = targetingObjectToCandidateSqr;
		m_closest.second = candidate;
	}

	if (targetingObjectToCandidateSqr > m_farthest.first)
	{
		m_farthest.first = targetingObjectToCandidateSqr;
		m_farthest.second = candidate;
	}

	if (m_currentTestTarget != 0)
	{
		if (targetingObjectToCandidateSqr < targetingObjectToCurrentTestTargetSqr)
		{
			// we are on the inside of the target
			if (targetingObjectToCandidateSqr > m_closestLessThanTestTarget.first)
			{
				m_closestLessThanTestTarget.first = targetingObjectToCandidateSqr;
				m_closestLessThanTestTarget.second = candidate;
			}
		}

		else
		{
			// we are on the outside of the target
			if (targetingObjectToCandidateSqr < m_closestGreaterThanTestTarget.first)
			{
				m_closestGreaterThanTestTarget.first = targetingObjectToCandidateSqr;
				m_closestGreaterThanTestTarget.second = candidate;
			}
		}
	}
}

//----------------------------------------------------------------------

void CuiCombatManagerNamespace::findTargetsWithRelationship(Object const * const targetingObject, ActorAndTargetHaveReletionship function, ClientWorld::ObjectVector & potentialObjects, TargetResult & result)
{
	NOT_NULL(targetingObject);
	NOT_NULL(function);

	if (targetingObject == 0)
	{
		return;
	}

	if (function == 0)
	{
		return;
	}

	ClientWorld::ObjectVector::const_iterator ii = potentialObjects.begin();
	ClientWorld::ObjectVector::const_iterator iiEnd = potentialObjects.end();

	for (; ii != iiEnd; ++ii)
	{
		Object * const objectToTest = *ii;
		if (objectToTest != 0)
		{
			if (objectToTest != targetingObject->getRootParent())
			{
				if (function(targetingObject, objectToTest))
				{
					result.testCandidate(objectToTest);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

Object * CuiCombatManagerNamespace::getNextTarget(TargetResult const & result)
{
	if (result.m_closestGreaterThanTestTarget.second != 0)
	{
		return result.m_closestGreaterThanTestTarget.second;
	}

	return result.m_closest.second;
}

// ----------------------------------------------------------------------

Object * CuiCombatManagerNamespace::getPreviousTarget(TargetResult const & result)
{
	if (result.m_closestLessThanTestTarget.second != 0)
	{
		return result.m_closestLessThanTestTarget.second;
	}

	return result.m_farthest.second;
}

//----------------------------------------------------------------------

bool CuiCombatManagerNamespace::targetIsCreature(Object const * const actor, Object const * const target)
{
	NOT_NULL(actor); // remove for release
	NOT_NULL(target);

	ClientObject const * const targetClientObject = (target != 0) ? target->asClientObject() : 0;
	TangibleObject const * const targetTangibleObject = (targetClientObject != 0) ? targetClientObject->asTangibleObject() : 0;
	if (targetTangibleObject != 0)
	{
		if (isTargetCycleOk(*targetTangibleObject))
		{
			return targetTangibleObject->asCreatureObject() != 0;
		}
	}
	return false;
}

//----------------------------------------------------------------------

bool CuiCombatManagerNamespace::targetIsCreatureAndFriendOfActor(Object const * const actor, Object const * const target)
{
	if (targetIsCreature(actor, target))
	{
		ClientObject const * const clientObject = target->asClientObject();
		if (clientObject != 0)
		{
			TangibleObject const * const tangibleObject = clientObject->asTangibleObject();
			if (tangibleObject != 0)
			{
				if (tangibleObject->isAttackable())
				{
					return false;
				}

				return !tangibleObject->isEnemy();
			}
		}
	}
	return false;
}

//----------------------------------------------------------------------

bool CuiCombatManagerNamespace::targetIsAttackable(Object const * const /*actor*/, Object const * const target)
{
	ClientObject const * const clientObject = target->asClientObject();
	if (clientObject != 0)
	{
		TangibleObject const * const tangibleObject = clientObject->asTangibleObject();
		if (tangibleObject != 0)
		{
			return isTargetCycleOk(*tangibleObject) && tangibleObject->isAttackable();
}
	}

	return false;
}

//----------------------------------------------------------------------

CachedNetworkId const & CuiCombatManagerNamespace::cycleTargetsForPlayer(ActorAndTargetHaveReletionship relationShipFunction, GetDirectionTarget directionFunction, bool useIntendedTarget)
{
	NOT_NULL(relationShipFunction);
	if (relationShipFunction == 0)
	{
		return CachedNetworkId::cms_cachedInvalid;
	}

	NOT_NULL(directionFunction);
	if (directionFunction == 0)
	{
		return CachedNetworkId::cms_cachedInvalid;
	}

	CreatureObject * const player = Game::getPlayerCreature();

	NOT_NULL(player);
	if (player == 0)
	{
		return CachedNetworkId::cms_cachedInvalid;
	}

	CachedNetworkId const & target = useIntendedTarget ? player->getIntendedTarget() : player->getLookAtTarget();

	Vector const playerPosition(player->getPosition_w());
	float const targetingRange = ConfigClientGame::getTargetingRange ();

	ClientWorld::ObjectVector objectsInRange;
	ClientWorld::findObjectsInRange(playerPosition, targetingRange, objectsInRange);

	TargetResult result(player, target.getObject());
	findTargetsWithRelationship(player, relationShipFunction, objectsInRange, result);

	Object const * const newTarget = directionFunction(result);

	if (newTarget != 0)
	{
		useIntendedTarget ? player->setIntendedTarget(newTarget->getNetworkId()) : player->setLookAtTarget(newTarget->getNetworkId());
	}

	return useIntendedTarget ? player->getIntendedTarget() : player->getLookAtTarget();
}


using namespace CuiCombatManagerNamespace;

//----------------------------------------------------------------------

class CuiCombatManager::CuiCombatManagerAction : public CuiAction
{
public:
	
	CuiCombatManagerAction () : CuiAction () {}
	
	bool  performAction (const std::string & id, const Unicode::String & ) const
	{
		if (id == CuiActions::cycleTargetInward)
		{
			CuiCombatManager::cycleTargetsInward ();
		}
		else if (id == CuiActions::cycleTargetOutward)
		{
			CuiCombatManager::cycleTargetsOutward ();
		}
		else if (id == CuiActions::cycleTargetNext)
		{
			CuiCombatManager::cycleTargetsNext ();
		}
		else if (id == CuiActions::cycleTargetPrev)
		{
			CuiCombatManager::cycleTargetsPrevious ();
		}
		else if (id == CuiActions::cycleTargetGroupPrev)
		{
			CuiCombatManager::cycleTargetsGroupPrevious ();
		}
		else if (id == CuiActions::cycleTargetGroupNext)
		{
			CuiCombatManager::cycleTargetsGroupNext ();
		}
		else if (id == CuiActions::cycleComponentTargetForward)
		{
			CuiCombatManager::cycleComponentTargetForward ();
		}
		else if (id == CuiActions::cycleComponentTargetBackward)
		{
			CuiCombatManager::cycleComponentTargetBackward ();
		}
		else if (id == CuiActions::cycleTargetsOutwardFriendly)
		{
			CuiCombatManager::cycleTargetsOutwardFriendly();
		}
		else if (id == CuiActions::cycleTargetsInwardFriendly)
		{
			CuiCombatManager::cycleTargetsInwardFriendly();
		}
		else if (id == CuiActions::cycleTargetsOutwardAll)
		{
			CuiCombatManager::cycleTargetsOutwardAll();
		}
		else if (id == CuiActions::cycleTargetsInwardAll)
		{
			CuiCombatManager::cycleTargetsInwardAll();
		}
		else
		{
			return false;
		}
		
		return true;
	}
};

//----------------------------------------------------------------------

using namespace CuiCombatManagerNamespace;

//----------------------------------------------------------------------

void CuiCombatManager::install ()
{
	DEBUG_FATAL (s_installed, ("already installed\n"));
	s_listener               = new Listener;
	ms_combatQueueSequenceId = 0;
	ms_lastTargetedIdInQueue = NetworkId::cms_invalid;
	ms_action                = new CuiCombatManagerAction;

	CuiActionManager::addAction (CuiActions::cycleTargetInward,       ms_action, false);
	CuiActionManager::addAction (CuiActions::cycleTargetOutward,      ms_action, false);
	CuiActionManager::addAction (CuiActions::cycleTargetNext,         ms_action, false);
	CuiActionManager::addAction (CuiActions::cycleTargetPrev,         ms_action, false);
	CuiActionManager::addAction (CuiActions::cycleTargetGroupNext,    ms_action, false);
	CuiActionManager::addAction (CuiActions::cycleTargetGroupPrev,    ms_action, false);
	CuiActionManager::addAction (CuiActions::cycleComponentTargetForward, ms_action, false);
	CuiActionManager::addAction (CuiActions::cycleComponentTargetBackward, ms_action, false);
	CuiActionManager::addAction (CuiActions::cycleTargetsOutwardFriendly, ms_action, false);
	CuiActionManager::addAction (CuiActions::cycleTargetsInwardFriendly, ms_action, false);
	CuiActionManager::addAction (CuiActions::cycleTargetsOutwardAll, ms_action, false);
	CuiActionManager::addAction (CuiActions::cycleTargetsInwardAll, ms_action, false);

	s_callback.connect (s_callback, &MyCallback::onIncapacityChanged,         static_cast<CreatureObject::Messages::IncapacityChanged *>      (0));
	s_callback.connect (s_callback, &MyCallback::onLookAtTargetChanged,       static_cast<CreatureObject::Messages::LookAtTargetChanged *>    (0));

	ms_showDamageOverHeadOthers  = ConfigClientUserInterface::getShowDamageOverHeadOthers   ();
	ms_showDamageOverHeadSelf    = ConfigClientUserInterface::getShowDamageOverHeadSelf     ();
	ms_showDamageSnare           = ConfigClientUserInterface::getShowDamageSnare            ();
	ms_damageDoneToMeSizeModifier = 1.0f;
	ms_damageDoneToOthersSizeModifier = 1.0f;
	ms_nonDamageDoneToMeSizeModifier = 1.0f;
	ms_nonDamageDoneToOthersSizeModifier = 1.0f;
	ms_showEnteringCombat        = ConfigClientUserInterface::getShowEnteringCombatFlyText();


	const char * const section = "CuiCombatManager";
	LocalMachineOptionManager::registerOption (ms_showDamageOverHeadOthers,  section, "showDamageOverHeadOthers");
	LocalMachineOptionManager::registerOption (ms_showDamageOverHeadSelf,    section, "showDamageOverHeadSelf");
	LocalMachineOptionManager::registerOption (ms_showDamageSnare,    section, "showDamageSnare");
	LocalMachineOptionManager::registerOption (ms_showDamageGlancingBlow,    section, "showDamageGlancingBlow");
	LocalMachineOptionManager::registerOption (ms_showDamageCriticalHit,    section, "showDamageCriticalHit");
	LocalMachineOptionManager::registerOption (ms_showDamageLucky,    section, "showDamageLucky");
	LocalMachineOptionManager::registerOption (ms_showDamageDot,    section, "showDamageDot");
	LocalMachineOptionManager::registerOption (ms_showDamageBleed,    section, "showDamageBleed");
	LocalMachineOptionManager::registerOption (ms_showDamageHeal,    section, "showDamageHeal");
	LocalMachineOptionManager::registerOption (ms_showDamageFreeshot,    section, "showDamageFreeshot");
	LocalMachineOptionManager::registerOption (ms_damageDoneToMeSizeModifier, section, "damageDoneToMeSizeModifier");
	LocalMachineOptionManager::registerOption (ms_damageDoneToOthersSizeModifier, section, "damageDoneToOthersSizeModifier");
	LocalMachineOptionManager::registerOption (ms_nonDamageDoneToMeSizeModifier, section, "nonDamageDoneToMeSizeModifier");
	LocalMachineOptionManager::registerOption (ms_nonDamageDoneToOthersSizeModifier, section, "nonDamageDoneToOthersSizeModifier");
	LocalMachineOptionManager::registerOption (ms_showEnteringCombat, section, "showEnteringCombat");

	// conning data table
	const char * const con_level_table = "datatables/player/level_con_table.iff";
	Iff dataTableIff;

	//-- load the special attack data
	if (dataTableIff.open(con_level_table, true))
	{
		DataTable conDataTable;

		conDataTable.load(dataTableIff);

		s_numConLevels = conDataTable.getNumColumns() - 1; // first column is the player level, the rest are con levels

		const int numRows = conDataTable.getNumRows();

		int lastPlayerLevel = 0;
		for (int row = 0; row < numRows; ++row)
		{
			const int playerLevel = conDataTable.getIntValue(0, row);

			if (playerLevel != lastPlayerLevel + 1)
			{
				while (playerLevel > lastPlayerLevel)
				{
					WARNING(true, ("CuiCombatManager::install: level data table [%s] is missing data for level %d", con_level_table, lastPlayerLevel + 1));
					lastPlayerLevel++;
				}
			}
			++lastPlayerLevel;

			ConIndexMap conIndexMap = s_playerLevelToConColorMap[row];

			int lastConValue = 0;
			for (int column = 1; column <= s_numConLevels; column++)
			{
				const int conValue = conDataTable.getIntValue(column, row);
				conIndexMap[column] = conValue;
				if (conValue < lastConValue)
				{
					WARNING(true, ("CuiCombatManager::install: con level [%d] out of order level data table [%s] is missing data for level %d at row %d col %d", column, con_level_table, conValue, row+1, column+1));
				}
				lastConValue = conValue;
			}

			s_playerLevelToConColorMap[playerLevel] = conIndexMap;
		}
		dataTableIff.close();
	}
	else
	{
		WARNING(true, ("CuiCombatManager::install: unable to open con level data table [%s]", con_level_table));
	}

	const char * const con_data_table = "datatables/player/con_display_data.iff";
	if (dataTableIff.open(con_data_table, true))
	{
		DataTable conDataTable;

		conDataTable.load(dataTableIff);

		const int redValue             = conDataTable.findColumnNumber("red");
		const int greenValue           = conDataTable.findColumnNumber("green");
		const int blueValue            = conDataTable.findColumnNumber("blue");
		const int strIdFile            = conDataTable.findColumnNumber("strIdFile");
		const int strIdName            = conDataTable.findColumnNumber("strIdName");

		const int numRows = conDataTable.getNumRows();

		if ((s_numConLevels+1) != numRows) // con_0 is not in level_con_table
		{
			WARNING(true, ("CuiCombatManager::install: number of con levels in [%s] doesn't match number of con levels in [%s]", con_data_table, con_level_table));
		}

		s_conLevelData.resize(s_numConLevels+1); // num levels + invulnerable

		int row;
		for (row = 0; row < numRows; ++row)
		{
			const unsigned char red   = (unsigned char) conDataTable.getIntValue(redValue, row);
			const unsigned char green = (unsigned char) conDataTable.getIntValue(greenValue, row);
			const unsigned char blue  = (unsigned char) conDataTable.getIntValue(blueValue, row);

			std::string const &fileName = conDataTable.getStringValue(strIdFile, row);
			std::string const &stringName = conDataTable.getStringValue(strIdName, row);

			s_conLevelData[row] = std::make_pair (StringId (fileName.c_str(), stringName.c_str()), UIColor (red, green, blue));
		}

		for (int missingLevel = row; missingLevel < s_numConLevels; missingLevel++)
		{
			s_conLevelData[missingLevel] = std::make_pair (StringId ("client", ""), UIColor (255, 64, 200));
			WARNING(true, ("CuiCombatManager::install: adding data for missing con level %d in con data table [%s]", missingLevel, con_data_table));
		}
	}
	else
	{
		WARNING(true, ("CuiCombatManager::install: unable to open con level data table [%s]", con_data_table));
	}

	// don't include index 0 in the test, that's for invulnerable objects
	if (s_numConLevels != (int) (s_conLevelData.size() - 1))
	{
		WARNING(true, ("CuiCombatManager::install conIndexData size [%d] doesn't match data table size [%d]", s_conLevelData.size(), s_numConLevels));

		s_numConLevels = s_conLevelData.size() - 1;
	}

	// The template will only be used to set the spamOrder values.
	StringId spamTemplateId("@cbt_spam:attack_result_template");
	StringId spamTemplateBriefId("@cbt_spam:attack_result_template_brief");
	Unicode::String spamTemplate = spamTemplateId.localize();
	Unicode::String spamTemplateBrief = spamTemplateBriefId.localize();
	fillSpamOrder(ms_spamOrder, spamTemplate);
	fillSpamOrder(ms_spamOrderBrief, spamTemplateBrief);

	DebugFlags::registerFlag (s_debugDifficulty,       "ClientUserInterface", "difficulty");

	// the values aren't set when these are initialized, so they need to be set again
	s_damageToPlayerColor = VectorArgb::solidRed;
	s_damageToTargetColor = VectorArgb::solidYellow;
	s_groupMemberDamage   = VectorArgb::solidGray;
	s_otherDamageColor    = VectorArgb::solidWhite;

	s_installed      = true;
}

//----------------------------------------------------------------------

void CuiCombatManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));

	s_callback.disconnect (s_callback, &MyCallback::onLookAtTargetChanged,       static_cast<CreatureObject::Messages::LookAtTargetChanged *>     (0));
	s_callback.disconnect (s_callback, &MyCallback::onIncapacityChanged,         static_cast<CreatureObject::Messages::IncapacityChanged *>     (0));
	delete s_listener;
	s_listener        = 0;

	CuiActionManager::removeAction (ms_action);
	delete ms_action;
	ms_action = 0;

	s_delayedDamageAction.clear();

	s_installed      = false;
}

//----------------------------------------------------------------------

bool CuiCombatManager::simulateSinglePlayerCombat ()
{
	return simulateSinglePlayerCombat (getLookAtTarget ());
}

//----------------------------------------------------------------------

bool CuiCombatManager::simulateSinglePlayerCombat (const NetworkId & id)
{
	setCombatTarget (id);

	if (Game::getSinglePlayer ())
	{
		// This is a single player system to add a combat playback animation.
		// The actual combat action and parameters used are set via the combat commands.

		// get the player
		CreatureObject * const player       = Game::getPlayerCreature ();
		Object         *       playerWeapon = NULL;

  		// Deduce the weapon to use for the attack.  For now, use what's in the attacker's right hand.
  		if (player)
  		{
			player->setState (States::Combat, true);
  			SlottedContainer *const container = ContainerInterface::getSlottedContainer(*player);
  			if (container)
  			{
					// Note: this will not work for grenades.
					// @todo add a weapon object id setter in the combat console menu.
				Container::ContainerErrorCode tmp = Container::CEC_Success;
				CachedNetworkId weaponId = container->getObjectInSlot(SlotIdManager::findSlotId(ConstCharCrcLowerString("hold_r")), tmp);
  				if (weaponId != NetworkId::cms_invalid)
  					playerWeapon = weaponId.getObject();
  			}
  		}

		
		if (!player)
			return false;

		// get the defender
		TangibleObject * const defender = dynamic_cast<TangibleObject *>(CachedNetworkId (id).getObject ());
		if (!defender)
		{
			DEBUG_WARNING (true, ("defender object could be not retrieved by network id.\n"));
			return true;
		}

		CreatureObject * const defenderCreature = defender->asCreatureObject ();
		if (defenderCreature)
			defenderCreature->setState (States::Combat, true);

		//-- Setup an attack.
		const uint32  actionId                = Crc::normalizeAndCalculate (ms_consoleActionName.c_str ());
		const int     attackerPostureEndIndex = ms_consoleAttackerPostureEndIndex;
		const int     attackerTrailBits       = 0;
		const int     attackerClientEffectId  = 0;
		const int     attackerClientActionNameCrc  = Crc::normalizeAndCalculate (ms_consoleActionName.c_str ());
		const bool    attackerUseLocation     = false;
		const Vector  attackerTargetLocation;
		const NetworkId attackerTargetCell = NetworkId::cms_invalid;
		const int     defenderEndPostureIndex = ms_consoleDefenderPostureEndIndex;
		const ClientCombatActionInfo::DefenderDefense defense = static_cast<ClientCombatActionInfo::DefenderDefense> (ms_consoleDefenderDefense);
		const int     defenderClientEffectId  = 0;
		const int     defenderHitLocation     = 0;
		const int     damageAmount            = (defense == ClientCombatActionInfo::DD_hit) ? 100 : 0;
		
		const ClientCombatActionInfo actionInfo(player, playerWeapon, attackerPostureEndIndex, attackerTrailBits, attackerClientEffectId, attackerClientActionNameCrc, attackerUseLocation, attackerTargetLocation, attackerTargetCell, actionId, defender, defenderEndPostureIndex, defense, defenderClientEffectId, defenderHitLocation, damageAmount);

		// submit combat action for playback
		ClientCombatPlaybackManager::handleCombatAction(actionInfo);

		if (defense == ClientCombatActionInfo::DD_hit)
			DEBUG_REPORT_LOG(true, (">> hit <<\n"));
		else
			DEBUG_REPORT_LOG(true, (">> miss <<\n"));
	}
	
	return true;
}

//----------------------------------------------------------------------

/**
* Set the currently selected target.  Add the target to the list of target if necessary.
*
*/

void CuiCombatManager::setCombatTarget (const NetworkId & id)
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if (player)
	{
		TangibleObject * const tangible = TangibleObject::getTangibleObject(id);

		if (tangible)
			player->setLookAtTarget (id);
	}
}

//----------------------------------------------------------------------

bool CuiCombatManager::isInCombat (const CreatureObject * actor, CachedNetworkId & id)
{
	if (!actor)
		actor = Game::getPlayerCreature ();

	if (   (actor != NULL)
	    && actor->getState(States::Combat))
	{
		id = actor->getLookAtTarget();
		return true;
	}
	else
	{
		id = CachedNetworkId::cms_cachedInvalid;
		return false;
	}
}

//----------------------------------------------------------------------

CachedNetworkId const & CuiCombatManager::getCombatTarget ()
{
	CreatureObject * const player = Game::getPlayerCreature ();

	if (player != NULL)
	{
		return player->getCombatTarget();
	}

	return CachedNetworkId::cms_cachedInvalid;
}

//----------------------------------------------------------------------

void CuiCombatManager::setLookAtTarget (const NetworkId & id)
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if (player)
		player->setLookAtTarget (id);
}

//----------------------------------------------------------------------

const CachedNetworkId & CuiCombatManager::getLookAtTarget ()
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if (player)
		return player->getLookAtTarget ();
	else
		return CachedNetworkId::cms_cachedInvalid;
}

//----------------------------------------------------------------------

bool CuiCombatManager::describeCommand (const std::string & )
{
	CuiChatRoomManager::sendPrelocalizedChat(CuiStringIdsCombatSpam::no_command_description.localize());
	return true;
}

//-----------------------------------------------------------------

void CuiCombatManager::reset ()
{

}

//----------------------------------------------------------------------

bool CuiCombatManager::isCombatCommand (const Command & command)
{
	if (!command.isNull())
	{
		static uint32 combatGroupSpecCrcs [] = 
		{
			Crc::normalizeAndCalculate ("combat_ranged"),
			Crc::normalizeAndCalculate ("combat_melee"),
			Crc::normalizeAndCalculate ("combat_general"),
			Crc::normalizeAndCalculate ("combat_non_loop")
		};

		return (command.m_commandGroup == combatGroupSpecCrcs [0] ||
				command.m_commandGroup == combatGroupSpecCrcs [1] ||
				command.m_commandGroup == combatGroupSpecCrcs [2] ||
				command.m_commandGroup == combatGroupSpecCrcs [3]);
	}

	return false;
}

//----------------------------------------------------------------------

bool CuiCombatManager::isCombatCommand (uint32 commandHash)
{
	const Command & command = CommandTable::getCommand (commandHash);
	return isCombatCommand (command);
}

//----------------------------------------------------------------------

bool CuiCombatManager::isCombatCommand (const std::string & command)
{
	const uint32 commandHash = Crc::normalizeAndCalculate(command.c_str ());
	return isCombatCommand (commandHash);
}

//----------------------------------------------------------------------

bool CuiCombatManager::isCombatQueueCommand (const Command & command)
{
	return command.m_addToCombatQueue;
}

//----------------------------------------------------------------------

bool CuiCombatManager::isCombatQueueCommand (uint32 commandHash)
{
	const Command & command = CommandTable::getCommand (commandHash);
	return isCombatQueueCommand (command);
}

//----------------------------------------------------------------------

bool CuiCombatManager::isCombatQueueCommand (const std::string & command)
{
	const uint32 commandHash = Crc::normalizeAndCalculate(command.c_str ());
	return isCombatQueueCommand (commandHash);
}

//----------------------------------------------------------------------

void CuiCombatManager::getCombatCommandsFromQueue (IntVector & iv)
{
	const ClientCommandQueue::EntryMap & em = ClientCommandQueue::get ();

	for (ClientCommandQueue::EntryMap::const_iterator it = em.begin (); it != em.end (); ++it)
	{
		const uint32 sequenceId                 = (*it).first;
		const ClientCommandQueue::Entry & entry = (*it).second;

		if (entry.m_command && isCombatQueueCommand (*entry.m_command))
			iv.push_back (sequenceId);
	}
}

//----------------------------------------------------------------------

const std::string& CuiCombatManager::getConsoleActionName ()
{
	return ms_consoleActionName;
}

//----------------------------------------------------------------------

void CuiCombatManager::setConsoleActionName (const std::string& actionName)
{
	ms_consoleActionName = actionName;
}

//----------------------------------------------------------------------

int CuiCombatManager::getConsoleAttackerPostureEndIndex ()
{
	return ms_consoleAttackerPostureEndIndex;
}

//----------------------------------------------------------------------

void CuiCombatManager::setConsoleAttackerPostureEndIndex (int attackerPostureEndIndex)
{
	ms_consoleAttackerPostureEndIndex = attackerPostureEndIndex;
}

//----------------------------------------------------------------------

int CuiCombatManager::getConsoleDefenderPostureEndIndex ()
{
	return ms_consoleDefenderPostureEndIndex;
}

//----------------------------------------------------------------------

void CuiCombatManager::setConsoleDefenderPostureEndIndex (int defenderPostureEndIndex)
{
	ms_consoleDefenderPostureEndIndex = defenderPostureEndIndex;
}

//----------------------------------------------------------------------

int CuiCombatManager::getConsoleDefenderDefense ()
{
	return ms_consoleDefenderDefense;
}

//----------------------------------------------------------------------

void CuiCombatManager::setConsoleDefenderDefense (int defenderDefense)
{
	ms_consoleDefenderDefense = defenderDefense;
}

//----------------------------------------------------------------------

const CachedNetworkId & CuiCombatManager::cycleTargetsGroup (int increment)
{
	bool useIntendedTarget = !Game::isSpace();

	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player)
		return CachedNetworkId::cms_cachedInvalid;

	const NetworkId & playerId     = player->getNetworkId ();
	const CachedNetworkId & target = useIntendedTarget ? player->getIntendedTarget() : player->getLookAtTarget ();

	const GroupObject * const group = safe_cast<GroupObject *>(player->getGroup ().getObject ());
	if (!group)
		return CachedNetworkId::cms_cachedInvalid;

	const GroupObject::GroupMemberVector & gmv = group->getGroupMembers ();

	int cur_group_index  = -1;
	int self_group_index = -1;
	{
		int index = 0;
		for (GroupObject::GroupMemberVector::const_iterator it = gmv.begin (); it != gmv.end (); ++it, ++index)
		{
			const GroupObject::GroupMember & mem = *it;
			const NetworkId & memId = mem.first;

			if (memId == target)
				cur_group_index = index;
			if (memId == playerId)
				self_group_index = index;
		}
	}

	if (cur_group_index < 0)
	{
		cur_group_index = 0;
	}
	else
		cur_group_index = cur_group_index + increment;

	const int numMembers  = static_cast<int>(gmv.size ());

	cur_group_index       = cur_group_index % numMembers;
	if (cur_group_index < 0)
		cur_group_index       = numMembers + cur_group_index;
	
	const int start_index = cur_group_index;

	int i = 0;

	for (; i < numMembers; ++i)
	{
		if (cur_group_index == self_group_index)
			cur_group_index += increment;

		cur_group_index       = cur_group_index % numMembers;
		if (cur_group_index < 0)
			cur_group_index       = numMembers + cur_group_index;

		const CachedNetworkId curId (gmv [static_cast<size_t>(cur_group_index)].first);

		if (!curId.getObject ())
		{
			cur_group_index += increment;
			cur_group_index       = cur_group_index % numMembers;
			if (cur_group_index < 0)
				cur_group_index       = numMembers + cur_group_index;
		}
		else
			break;

		if (cur_group_index == start_index)
		{
			cur_group_index = -1;
			break;
		}
	}

	if (i > numMembers)
	{
		cur_group_index = -1;
	}

	if (cur_group_index > 0)
	{
		useIntendedTarget ? player->setIntendedTarget (gmv [static_cast<size_t>(cur_group_index)].first) :
			                player->setLookAtTarget (gmv [static_cast<size_t>(cur_group_index)].first);
	}
	
	return useIntendedTarget ? player->getIntendedTarget() : player->getLookAtTarget ();
}

//----------------------------------------------------------------------

const CachedNetworkId & CuiCombatManager::cycleTargetsGroupPrevious ()
{
	return cycleTargetsGroup (-1);
}

//----------------------------------------------------------------------

const CachedNetworkId & CuiCombatManager::cycleTargetsGroupNext     ()
{
	return cycleTargetsGroup (1);

}

//----------------------------------------------------------------------

const CachedNetworkId & CuiCombatManager::cycleTargetsOutward  ()
{
	// In space we use the look at target here, on ground we use the intended target, 
	// see cycleTargetsForPlayer fourth argument is useIntendedTarget
	return cycleTargetsForPlayer(targetIsAttackable, getNextTarget, !Game::isSpace());
}

//----------------------------------------------------------------------

const CachedNetworkId & CuiCombatManager::cycleTargetsInward()
{
	// In space we use the look at target here, on ground we use the intended target, 
	// see cycleTargetsForPlayer fourth argument is useIntendedTarget
	return cycleTargetsForPlayer(targetIsAttackable, getPreviousTarget, !Game::isSpace());
}

//----------------------------------------------------------------------

const CachedNetworkId & CuiCombatManager::cycleTargetsOutwardFriendly()
{
	// In space we use the look at target here, on ground we use the intended target, 
	// see cycleTargetsForPlayer fourth argument is useIntendedTarget
	return cycleTargetsForPlayer(targetIsCreatureAndFriendOfActor, getNextTarget, !Game::isSpace());
}

//----------------------------------------------------------------------

const CachedNetworkId & CuiCombatManager::cycleTargetsInwardFriendly()
{
	// In space we use the look at target here, on ground we use the intended target, 
	// see cycleTargetsForPlayer fourth argument is useIntendedTarget
	return cycleTargetsForPlayer(targetIsCreatureAndFriendOfActor, getPreviousTarget, !Game::isSpace());
}

//----------------------------------------------------------------------

const CachedNetworkId & CuiCombatManager::cycleTargetsOutwardAll()
{
	// In space we use the look at target here, on ground we use the intended target, 
	// see cycleTargetsForPlayer fourth argument is useIntendedTarget
	return cycleTargetsForPlayer(targetIsCreature, getNextTarget, !Game::isSpace());
}

//----------------------------------------------------------------------

const CachedNetworkId & CuiCombatManager::cycleTargetsInwardAll()
{
	// In space we use the look at target here, on ground we use the intended target, 
	// see cycleTargetsForPlayer fourth argument is useIntendedTarget
	return cycleTargetsForPlayer(targetIsCreature, getPreviousTarget, !Game::isSpace());
}

//----------------------------------------------------------------------

bool CuiCombatManagerNamespace::isTargetCycleOk(TangibleObject const & obj)
{
	bool useIntendedTarget = !Game::isSpace();
	
	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player)
		return false;

	if(&obj == player)
		return false;
	if((useIntendedTarget ? player->getIntendedTarget() : player->getLookAtTarget()) == obj.getNetworkId())
		return false;
	
	//-- we are lookoing at an untargettable object
	if (!obj.isTargettable ())
		return false;
	
	ShipObject const * const shipObject = obj.asShipObject ();
	if (shipObject)
	{
		//-- @todo: ships need to be flagged attackable
		if (player == shipObject->getPilot())
			return false;
	}
	
	CreatureObject const * const creature = obj.asCreatureObject ();
	if (creature && creature->isDead ())
		return false;
	
	// frustum test

	if (!Game::isHudSceneTypeSpace())
	{
		const Camera * const camera = Game::getCamera ();
		if (!camera)
			return false;

		const Sphere sphere (obj.getAppearanceSphereCenter_w (), obj.getAppearanceSphereRadius ());
		if (!camera->testVisibility_w (sphere))
			return false;
	}

	return true;
}

//----------------------------------------------------------------------

const CachedNetworkId & CuiCombatManager::cycleTargetsPrevious ()
{
	bool useIntendedTarget = !Game::isSpace();

	CreatureObject * const player = Game::getPlayerCreature ();

	if (!player)
		return CachedNetworkId::cms_cachedInvalid;

	if (s_targetHistory.size () < 2)
		return useIntendedTarget ? player->getIntendedTarget() : player->getLookAtTarget ();

	const CachedNetworkId cur_id = s_targetHistory.back ();

	s_targetHistory.pop_back ();

	//- changing the lookat target here should not perturb the id's position at the back of the history;
	if (cur_id.getObject ())
		s_targetHistory.push_front (cur_id);

	const CachedNetworkId & id = s_targetHistory.back ();

	if (!id.getObject () || (id.getObject() && id.getObject()->asClientObject() && id.getObject()->asClientObject()->isTargettable()))
	{
		s_targetHistory.pop_back ();
		return cycleTargetsPrevious ();
	}

	useIntendedTarget ? player->setIntendedTarget (id) : player->setLookAtTarget (id);

	return useIntendedTarget ? player->getIntendedTarget() : player->getLookAtTarget ();
}

//----------------------------------------------------------------------

const CachedNetworkId & CuiCombatManager::cycleTargetsNext ()
{
	bool useIntendedTarget = !Game::isSpace();

	CreatureObject * const player = Game::getPlayerCreature ();

	if (!player)
		return CachedNetworkId::cms_cachedInvalid;

	if (s_targetHistory.size () < 2)
		return useIntendedTarget ? player->getIntendedTarget() : player->getLookAtTarget ();

	const CachedNetworkId & id = s_targetHistory.front ();

	if (!id.getObject () || (id.getObject() && id.getObject()->asClientObject() && id.getObject()->asClientObject()->isTargettable()))
	{
		s_targetHistory.pop_front ();
		return cycleTargetsNext ();
	}

	//- changing the lookat target automatically moves the target to the back of the history
	useIntendedTarget ? player->setIntendedTarget (id) : player->setLookAtTarget (id);
	return useIntendedTarget ? player->getIntendedTarget() : player->getLookAtTarget ();
}

//----------------------------------------------------------------------

void  CuiCombatManager::pushTargetHistory    ()
{
	bool useIntendedTarget = !Game::isSpace();

	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player)
		return;

	const CachedNetworkId & target = useIntendedTarget ? player->getIntendedTarget() : player->getLookAtTarget ();

	if (target.getObject ())
	{
		const CachedNetworkIdVector::iterator it = std::remove (s_targetHistory.begin (), s_targetHistory.end (), target);
		s_targetHistory.erase (it, s_targetHistory.end ());

		s_targetHistory.push_back (useIntendedTarget ? player->getIntendedTarget() : player->getLookAtTarget ());
	}

	const int size = static_cast<int>(s_targetHistory.size ());
	if (size > MAX_TARGET_HISTORY)
	{
		CachedNetworkIdVector::iterator it = s_targetHistory.begin ();
		std::advance (it, static_cast<size_t>(MAX_TARGET_HISTORY));
		s_targetHistory.erase (s_targetHistory.begin (), it);
	}
}

//----------------------------------------------------------------------

void CuiCombatManager::clearHistory ()
{
	s_targetHistory.clear ();
}

//----------------------------------------------------------------------

void CuiCombatManager::cycleComponentTargetForward()
{
	DEBUG_FATAL(!Game::isSpace(), ("CuiCombatManager::cycleComponentTargetForward is not intended to be called on the ground"));
	CreatureObject * const player = Game::getPlayerCreature();
	if (!player)
		return;

	CachedNetworkId const & targetId = player->getLookAtTarget ();

	Object * const targetObj = targetId.getObject ();
	ClientObject * const targetClientObj = targetObj ? targetObj->asClientObject() : NULL;
	ShipObject * const targetShipObj = targetClientObj ? targetClientObj->asShipObject() : NULL;
	if(targetShipObj)
	{
		ShipChassisSlotType::Type const currentSlot = player->getLookAtTargetSlot();
		ShipChassisSlotType::Type const newSlot = targetShipObj->getNextValidTargetableSlot(currentSlot);
		player->setLookAtTargetSlot(newSlot);
	}
}

//----------------------------------------------------------------------

void CuiCombatManager::cycleComponentTargetBackward()
{
	DEBUG_FATAL(!Game::isSpace(), ("CuiCombatManager::cycleComponentTargetBackward is not intended to be called on the ground"));
	CreatureObject * const player = Game::getPlayerCreature();
	if (!player)
		return;

	CachedNetworkId const & targetId = player->getLookAtTarget ();

	Object * const targetObj = targetId.getObject ();
	ClientObject * const targetClientObj = targetObj ? targetObj->asClientObject() : NULL;
	ShipObject * const targetShipObj = targetClientObj ? targetClientObj->asShipObject() : NULL;
	if(targetShipObj)
	{
		ShipChassisSlotType::Type const currentSlot = player->getLookAtTargetSlot();
		ShipChassisSlotType::Type const newSlot = targetShipObj->getPreviousValidTargetableSlot(currentSlot);
		player->setLookAtTargetSlot(newSlot);
	}
}

//----------------------------------------------------------------------

void CuiCombatManager::sendFakeCombatSpam (const NetworkId & idSource, const Vector & position_wSource, const NetworkId & idTarget, const Vector & position_wTarget, const NetworkId & idOther, int digit_i, float digit_f, const Unicode::String & str, int spamType)
{
	if (str.empty ())
		return;
	
	if (str [0] == '@')
	{
		const StringId stringId (Unicode::wideToNarrow (str.substr (1)));
		
		ProsePackage pp;
		pp.stringId = stringId;
		pp.actor.id = idSource;
		pp.target.id = idTarget;
		pp.other.id = idOther;
		pp.digitInteger = digit_i;
		pp.digitFloat = digit_f;
		Unicode::String oob;
		OutOfBandPackager::pack(pp, 0, oob);
		MessageQueueCombatSpam spamMsg(idSource, position_wSource, idTarget, position_wTarget, oob, false, false, false, spamType);
		processCombatSpam (spamMsg);
		
	}
	else
	{
//		MessageQueueCombatSpam spamMsg (CachedNetworkId (idSource), CachedNetworkId (idTarget),  CachedNetworkId (idOther), digit_i, str, 0);
//		spamMsg.m_floatValue = digit_f;
//		processCombatSpam (spamMsg);
	}
}

//----------------------------------------------------------------------

void CuiCombatManager::processCombatSpam (const MessageQueueCombatSpam & spamMsg)
{
	ClientTextManager::TextType spamType = ClientTextManager::TT_combatGeneric;

	Unicode::String decoded1;

	const ClientObject * const attacker = safe_cast<const ClientObject *>(NetworkIdManager::getObjectById(spamMsg.m_attacker));
	if (attacker && CommunityManager::isIgnored(attacker->getLocalizedName()))
		return;

	const ClientObject * const defender = safe_cast<const ClientObject *>(NetworkIdManager::getObjectById(spamMsg.m_defender));

	if (spamMsg.m_spamMessage.empty())
	{
		// Fill ms_spamStrings with the indexing in the same order as the above comment, and the flags given below.
		// In other words, pass the array to each of the functions and fill the array value at the correct index
		// instead of appending.  Then pass the array and the template into a new function which replaces the
		// number with the index value of the array.  If an array index is empty we remove the following space.
		// The string created from the replacement would then be the returned string.  This same system would be used

		const WeaponObject * weapon = NULL;
		if (spamMsg.m_weapon != NetworkId::cms_invalid)
		{
			const ClientObject * const object = safe_cast<const ClientObject *>(NetworkIdManager::getObjectById(spamMsg.m_weapon));
			if (object != NULL)
				weapon = object->asWeaponObject();
		}
		if (!CuiPreferences::getCombatSpamBrief())
		{
			if (attacker != NULL)
			{
				buildAttackMessageAttacker(*attacker, SENTENCE_STRUCTURE_ATTACKER);
			}
			if (defender != NULL)
			{
				buildAttackMessageDefender(*defender, SENTENCE_STRUCTURE_DEFENDER);
			}

			if (spamMsg.m_attackName != StringId::cms_invalid)
				buildAttackMessageAttackName(spamMsg.m_attackName, SENTENCE_STRUCTURE_ATTACK);

			if ((weapon != NULL || spamMsg.m_weaponName != StringId::cms_invalid) && CuiPreferences::getSpamShowWeapon())
			{
				if (weapon != NULL)
					buildAttackMessageWeapon(*weapon, SENTENCE_STRUCTURE_WEAPON);
				else
					buildAttackMessageWeapon(spamMsg.m_weaponName, SENTENCE_STRUCTURE_WEAPON);
			}
			buildAttackMessageGeneric(CuiStringIdsCombatSpam::attack_result_connect, SENTENCE_STRUCTURE_SUCCESS_CONNECT);
			buildAttackMessageSuccess(spamMsg.m_success, spamMsg.m_critical, spamMsg.m_glancing, spamMsg.m_crushing, spamMsg.m_strikethrough, spamMsg.m_strikethroughAmmount, spamMsg.m_evadeResult, spamMsg.m_evadeAmmount, spamMsg.m_blockResult, spamMsg.m_block, spamMsg.m_dodge, spamMsg.m_parry, SENTENCE_STRUCTURE_SUCCESS);

			buildAttackMessageGeneric(CuiStringIdsCombatSpam::attack_result_end_sentence, SENTENCE_STRUCTURE_DAMAGE_END_SENTENCE);
			if (spamMsg.m_success)
			{
				buildAttackMessageTotalDamage(spamMsg.m_finalDamage + spamMsg.m_elementalDamage, SENTENCE_STRUCTURE_TOTAL_DAMAGE);

				if (CuiPreferences::getSpamShowDamageDetail())
				{
					bool showDamage = false;
					bool showElemental = false;
					if ((spamMsg.m_finalDamage > 0 || weapon == NULL || weapon->getDamageType() != WeaponObject::DT_none))
						showDamage = true;
					if ((spamMsg.m_elementalDamage > 0 || weapon == NULL || weapon->getElementalType() != WeaponObject::DT_none))
						showElemental = true;
					if (showDamage || showElemental)
					{
						bool damageDisplayed = false;
						if (showDamage)
						{
							damageDisplayed = true;
							buildAttackMessageDamageDetail(spamMsg.m_finalDamage, SENTENCE_STRUCTURE_PRIMARY_DAMAGE);
							if (spamMsg.m_damageType != WeaponObject::DT_none)
							{
								buildAttackMessageDamageType(spamMsg.m_damageType, SENTENCE_STRUCTURE_DAMAGE_TYPE);
							}
							else if (weapon != NULL)
							{
								buildAttackMessageDamageType(weapon->getDamageType(), SENTENCE_STRUCTURE_DAMAGE_TYPE);
							}
						}
						if (showElemental && spamMsg.m_elementalDamage > 0)
						{
							damageDisplayed = true;
							if (showDamage && damageDisplayed)
							{
								buildAttackMessageGeneric(CuiStringIdsCombatSpam::attack_result_connect, SENTENCE_STRUCTURE_ELEMENTAL_DAMAGE_CONNECT);
							}

							buildAttackMessageDamageDetail(spamMsg.m_elementalDamage, SENTENCE_STRUCTURE_ELEMENTAL_DAMAGE);

							if (spamMsg.m_elementalDamageType != WeaponObject::DT_none)
							{
								buildAttackMessageDamageType(spamMsg.m_elementalDamageType, SENTENCE_STRUCTURE_ELEMENTAL_TYPE);
							}
							else if (weapon != NULL)
							{
								buildAttackMessageDamageType(weapon->getElementalType(), SENTENCE_STRUCTURE_ELEMENTAL_TYPE);
							}
						}
						if (damageDisplayed)
						{
							buildAttackMessageGeneric(CuiStringIdsCombatSpam::attack_result_damage_detail_start, SENTENCE_STRUCTURE_DAMAGE_DETAIL_START);
							buildAttackMessageGeneric(CuiStringIdsCombatSpam::attack_result_damage_detail_end, SENTENCE_STRUCTURE_DAMAGE_DETAIL_END);
						}
					}
				}		// End damage detail
				if (CuiPreferences::getSpamShowArmorAbsorption()&&
					spamMsg.m_blockedDamage > 0)
				{
					buildAttackMessageGeneric(CuiStringIdsCombatSpam::attack_result_armor_unowned, SENTENCE_STRUCTURE_ARMOR_PROTECTION);
					buildAttackMessageArmorProtection(spamMsg.m_blockedDamage, SENTENCE_STRUCTURE_PROTECTION_AMOUNT);
					buildAttackMessageGeneric(CuiStringIdsCombatSpam::attack_result_raw_damage_connect, SENTENCE_STRUCTURE_RAW_DAMAGE_CONNECT);
					buildAttackMessageRawDamage(spamMsg.m_finalDamage+spamMsg.m_elementalDamage+spamMsg.m_blockedDamage, SENTENCE_STRUCTURE_RAW_DAMAGE);
					buildAttackMessageGeneric(CuiStringIdsCombatSpam::attack_result_end_sentence, SENTENCE_STRUCTURE_ABSORB_END_SENTENCE);
				}
			}
			buildAttackMessage(decoded1);
		}
		else	// Brief mode
		{
			//You         Hit          Thomas      40 pts
			//Thomas      Misses
			//John        Hits         You         50 pts
			//You         Miss
			if (attacker != NULL)
			{
				buildAttackMessageAttackerBrief(*attacker, SENTENCE_STRUCTURE_ATTACKER_BRIEF);
			}

			buildAttackMessageSuccessBrief(spamMsg.m_success, spamMsg.m_critical, spamMsg.m_glancing, spamMsg.m_crushing, spamMsg.m_strikethrough, spamMsg.m_evadeResult, spamMsg.m_blockResult, spamMsg.m_dodge, spamMsg.m_parry, SENTENCE_STRUCTURE_SUCCESS_BRIEF);
			if (spamMsg.m_success)
			{
				if (defender != NULL)
				{
					buildAttackMessageDefenderBrief(*defender, SENTENCE_STRUCTURE_DEFENDER_BRIEF);
				}

				buildAttackMessageTotalDamageBrief(spamMsg.m_finalDamage + spamMsg.m_elementalDamage, SENTENCE_STRUCTURE_TOTAL_DAMAGE_BRIEF);
			}
			buildAttackMessageBrief(decoded1);
		}
	}
	else if (spamMsg.m_spamMessage[0] == '@')
	{
		// this is a specialized string id message
		ProsePackage pp;
		pp.stringId = StringId(Unicode::wideToNarrow(spamMsg.m_spamMessage).substr(1));
		pp.actor.id = spamMsg.m_attacker;
		pp.target.id = spamMsg.m_defender;
		ProsePackageManagerClient::setTranslation(pp, decoded1);
	}
	else
	{
		// this is a specialized prose package/oob message
		ProsePackageManagerClient::appendAllProsePackages(spamMsg.m_spamMessage, decoded1);
	}

	switch (static_cast<CombatResult>(spamMsg.m_spamType))
	{
		case COMBAT_RESULT_HIT:
			{
				if (attacker == Game::getClientPlayer())
				{
					if (spamMsg.m_proc)
						spamType = ClientTextManager::TT_combatYouHitOtherProc;
					else if (spamMsg.m_critical)
						spamType = ClientTextManager::TT_combatYouHitOtherCritical;
					else if (spamMsg.m_glancing)
						spamType = ClientTextManager::TT_combatYouHitOtherGlance;
					else
						spamType = ClientTextManager::TT_combatYouHitOther;
				}
				else if (defender == Game::getClientPlayer())
				{
					if (spamMsg.m_proc)
						spamType = ClientTextManager::TT_combatOtherHitYouProc;
					else if (spamMsg.m_critical)
						spamType = ClientTextManager::TT_combatOtherHitYouCritical;
					else if (spamMsg.m_glancing)
						spamType = ClientTextManager::TT_combatOtherHitYouGlance;
					else
						spamType = ClientTextManager::TT_combatOtherHitYou;
				}
				else
				{
					if (spamMsg.m_proc)
						spamType = ClientTextManager::TT_combatOtherHitOtherProc;
					else if (spamMsg.m_critical)
						spamType = ClientTextManager::TT_combatOtherHitOtherCritical;
					else if (spamMsg.m_glancing)
						spamType = ClientTextManager::TT_combatOtherHitOtherGlance;
					else
						spamType = ClientTextManager::TT_combatOtherHitOther;
				}
			}
			break;
		case COMBAT_RESULT_MISS:
			{
				if (attacker == Game::getClientPlayer())
				{
					spamType = ClientTextManager::TT_combatYouMissOther;
				}
				else if (defender == Game::getClientPlayer())
				{
					spamType = ClientTextManager::TT_combatOtherMissYou;
				}
				else
				{
					spamType = ClientTextManager::TT_combatOtherMissOther;
				}
			}
			break;
		case COMBAT_RESULT_OUT_OF_RANGE:
			{
				spamType = ClientTextManager::TT_combatOutOfRange;
			}
			break;
		case COMBAT_RESULT_POSTURE_CHANGE:
			{
				spamType = ClientTextManager::TT_combatPostureChange;
			}
			break;
		case COMBAT_RESULT_MEDICAL:
			{
				spamType = ClientTextManager::TT_combatOtherMedical;
			}
			break;
		case COMBAT_RESULT_BUFF:
			{
				spamType = ClientTextManager::TT_combatOtherBuff;
			}
			break;
		case COMBAT_RESULT_DEBUFF:
			{
				spamType = ClientTextManager::TT_combatOtherDebuff;
			}
			break;
		default:
			{
			}
			break;
	}

	Transceivers::combatSpamReceived.emitMessage (std::make_pair(decoded1, static_cast<int>(spamType)));
}

//----------------------------------------------------------------------

void CuiCombatManager::fillSpamOrder(short spamOrder[], Unicode::String spamTemplate)
{
	unsigned int pos = 0;
	Unicode::NarrowString token;
	Unicode::NarrowString narrowTemplate = Unicode::wideToNarrow(spamTemplate);
	for (int i = 0; pos != narrowTemplate.npos; i++)
	{
		// Each token will be "%nn" where nn is a two digit number starting with 01.  This was
		// done to make it easier on translators, so doing the -1 for array friendliness.
		Unicode::getFirstToken(narrowTemplate, pos, pos, token, " ");
		Unicode::NarrowString numericValue = token.substr(1,2);
		spamOrder[i] = static_cast<short>(atoi(numericValue.c_str()) - 1);
		if (spamOrder[i] < 0)
			spamOrder[i] = 0;
	}
}

//----------------------------------------------------------------------

void CuiCombatManager::requestPeace ()
{
	static const uint32 hash_peace = Crc::normalizeAndCalculate ("peace");
	ClientCommandQueue::enqueueCommand (hash_peace, NetworkId::cms_invalid, Unicode::String());
}

//----------------------------------------------------------------------

void CuiCombatManager::setShowDamageOverHeadOthers (bool b)
{
	ms_showDamageOverHeadOthers = b;
}

//----------------------------------------------------------------------

void CuiCombatManager::setShowDamageOverHeadSelf (bool b)
{
	ms_showDamageOverHeadSelf = b;
}

//----------------------------------------------------------------------

void CuiCombatManager::setShowDamageSnare (bool b)
{
	ms_showDamageSnare = b;
}

//----------------------------------------------------------------------

void CuiCombatManager::setShowDamageGlancingBlow (bool b)
{
	ms_showDamageGlancingBlow = b;
}

//----------------------------------------------------------------------

void CuiCombatManager::setShowDamageCriticalHit (bool b)
{
	ms_showDamageCriticalHit = b;
}

//----------------------------------------------------------------------

void CuiCombatManager::setShowDamageLucky (bool b)
{
	ms_showDamageLucky = b;
}

//----------------------------------------------------------------------

void CuiCombatManager::setShowDamageDot (bool b)
{
	ms_showDamageDot = b;
}

//----------------------------------------------------------------------

void CuiCombatManager::setShowDamageBleed (bool b)
{
	ms_showDamageBleed = b;
}

//----------------------------------------------------------------------

void CuiCombatManager::setShowDamageHeal (bool b)
{
	ms_showDamageHeal = b;
}

//----------------------------------------------------------------------

void CuiCombatManager::setShowDamageFreeshot (bool b)
{
	ms_showDamageFreeshot = b;
}

//----------------------------------------------------------------------

void CuiCombatManager::setDamageDoneToMeSizeModifier (float f)
{
	ms_damageDoneToMeSizeModifier = f;
}

//----------------------------------------------------------------------

void CuiCombatManager::setDamageDoneToOthersSizeModifier (float f)
{
	ms_damageDoneToOthersSizeModifier = f;
}

//----------------------------------------------------------------------

void CuiCombatManager::setNonDamageDoneToMeSizeModifier (float f)
{
	ms_nonDamageDoneToMeSizeModifier = f;
}

//----------------------------------------------------------------------

void CuiCombatManager::setNonDamageDoneToOthersSizeModifier (float f)
{
	ms_nonDamageDoneToOthersSizeModifier = f;
}

//----------------------------------------------------------------------

void CuiCombatManager::setShowEnteringCombat (bool b)
{
	ms_showEnteringCombat = b;
}

//----------------------------------------------------------------------

int CuiCombatManagerNamespace::getConIndex (CreatureObject const & enemy)
{
	CreatureObject const * const player = Game::getPlayerCreature ();
	if(player == &enemy)
		return s_conIndexSelf;

	if (enemy.isInvulnerable() || enemy.isDead())
		return s_conIndexInvulnerable;

	int const levelTarget = enemy.getGroupLevel ();
	int const levelSelf   = player->getGroupLevel  ();

	return getConIndex(levelSelf, levelTarget);
}

//----------------------------------------------------------------------

int CuiCombatManagerNamespace::getConIndex (int const level_self, int const level_target)
{
	static int previousPlayerLevel = -1;
	static int previousTargetLevel = -1;
	static int previousConIndex = 1;

	int conIndex = 1;

	if (previousPlayerLevel == level_self && previousTargetLevel == level_target)
	{
		conIndex = previousConIndex;
	}
	else
	{
		bool foundPlayerLevel = false;
		int levelDifference = 0;                                  // for higher level characters - used to extrapolate beyond the last player level in the data table
		ConIndexMap conIndexMap = s_playerLevelToConColorMap[0];  // fallback is the first entry in the table

		// check to see if the player level is outside of the range in the table and, if so, use the last entry and extrapolate the value
		PlayerLevelToConColorMap::reverse_iterator rPlayerLevelItr = s_playerLevelToConColorMap.rbegin();
		if (rPlayerLevelItr != s_playerLevelToConColorMap.rend())
		{
			const int difference = level_self - (*rPlayerLevelItr).first;
			if (difference > 0)
			{
				levelDifference = difference;
				conIndexMap = (*rPlayerLevelItr).second;
				foundPlayerLevel = true;
			}
		}

		if (levelDifference == 0)
		{
			// look for the level in the datatable
			// @NOTE: this code assumes that their are no level holes in the table 
			//    (that is, all levels from 1 to the last level in the table have an entry)
			PlayerLevelToConColorMap::iterator playerLevelItr = s_playerLevelToConColorMap.find(level_self);
			if (playerLevelItr != s_playerLevelToConColorMap.end())
			{
				conIndexMap = (*playerLevelItr).second;
				foundPlayerLevel = true;
			}
		}

		ConIndexMap::iterator conItr = conIndexMap.begin();

		if (foundPlayerLevel)
		{
			while (conItr != conIndexMap.end())
			{
				const int level = (*conItr).second;
				if (level + levelDifference > level_target)
				{
					break; // use the last level that was set
				}
			
				conIndex = (*conItr).first;
				++conItr;
			}
		}
	}

	previousPlayerLevel = level_self;
	previousTargetLevel = level_target;
	previousConIndex    = conIndex;

	return conIndex;
}

//----------------------------------------------------------------------

bool CuiCombatManager::getConMessage (const CreatureObject & enemy, Unicode::String & _message)
{
	static const StringId s_self_id ("client", "con_self");
	
	int conIndex = 0;
	const StringId * the_sid = &s_self_id;
	const CreatureObject * const player = Game::getPlayerCreature ();

	if(enemy.isInvulnerable())
		conIndex = 0;
	else
	{	
		int const levelSelf = player->getGroupLevel();
		int const levelTarget = enemy.getGroupLevel();

		conIndex = getConIndex(levelSelf, levelTarget);
	}

	UIColor color = s_conLevelData[conIndex].second;

	if (&enemy != player)
	{
		the_sid  = &s_conLevelData [conIndex].first;
	}

	CuiStringVariablesManager::process (*the_sid, Unicode::emptyString, enemy.getLocalizedName (), Unicode::emptyString, _message);
	if (!_message.empty ())
		(_message) [0] = static_cast<Unicode::unicode_char_t>(toupper ((_message) [0]));

	static Unicode::String colorString;
	colorString.clear ();
	UIUtils::FormatColor (colorString, color);

	static const Unicode::String endl = Unicode::narrowToWide ("\\#.");

	static Unicode::String str;
	str.clear ();
	str.push_back ('\\');
	str += colorString;
	str += _message;
	str += endl;
	_message = str;

	if (s_debugDifficulty)
	{
		const int level_target = enemy.getGroupLevel ();
		const int level_self   = player->getGroupLevel  ();
		char buf [256];
			const size_t buf_size = sizeof (buf);
		snprintf (buf, buf_size, " (%d) %d/%d", conIndex, level_self, level_target);
		_message += Unicode::narrowToWide (buf);
	}

	return true;
}

//----------------------------------------------------------------------

const UIColor & CuiCombatManager::getConColor (const CreatureObject & enemy)
{
	CreatureObject const * const playerObj = Game::getPlayerCreature();
	if(!playerObj)
		return UIColor::white;

	int const conIndex = getConIndex (enemy);
	if (conIndex >= 0 && conIndex < static_cast<int>(s_conLevelData.size()))
	{
		return s_conLevelData[conIndex].second;
	}
	else
		return UIColor::white;
}

//----------------------------------------------------------------------

UIColor const & CuiCombatManager::getConColor (int const playerLevel, int const targetLevel)
{
	int const conIndex = getConIndex (playerLevel, targetLevel);
	if (conIndex >= 0 && conIndex < static_cast<int>(s_conLevelData.size()))
	{
		return s_conLevelData[conIndex].second;
	}
	else
		return UIColor::white;
}

//----------------------------------------------------------------------

const UIColor & CuiCombatManager::getInvulnerableConColor()
{
	return s_conLevelData[s_conIndexInvulnerable].second;
}

//======================================================================
/**
 * All of the buildAttackMessageX functions below are used to build up an attack result
 * combat spam message. The result of each function is appended by the next one called.
 * One or more functions can be skipped in the call chain and still create a readable
 * message, as long as the call order is preserved.
 */

void CuiCombatManager::buildAttackMessageAttacker(const ClientObject & attacker, int resultIndex)
{
	ProsePackage pp;
	if (attacker.asTangibleObject() != NULL && attacker.asTangibleObject()->isPlayer())
		pp.stringId = CuiStringIdsCombatSpam::attack_result_attacker;
	else
		pp.stringId = CuiStringIdsCombatSpam::attack_result_attacker_fullname;
	pp.actor.id = attacker.getNetworkId();
	ProsePackageManagerClient::setTranslation(pp, ms_spamStrings[resultIndex]);
	ms_spamStringsSet[resultIndex] = true;
}

//----------------------------------------------------------------------

void CuiCombatManager::buildAttackMessageAttackerBrief(const ClientObject & attacker, int resultIndex)
{
	ProsePackage pp;
	if (attacker.asTangibleObject() != NULL && attacker.asTangibleObject()->isPlayer())
		pp.stringId = CuiStringIdsCombatSpam::attack_result_attacker_brief;
	else
		pp.stringId = CuiStringIdsCombatSpam::attack_result_attacker_fullname_brief;
	pp.actor.id = attacker.getNetworkId();
	ProsePackageManagerClient::setTranslation(pp, ms_spamStringsBrief[resultIndex]);
	ms_spamStringsSetBrief[resultIndex] = true;
}

//----------------------------------------------------------------------

void CuiCombatManager::buildAttackMessageDefender(const ClientObject & defender, int resultIndex)
{
	ProsePackage pp;
	if (defender.asTangibleObject() != NULL && defender.asTangibleObject()->isPlayer())
		pp.stringId = CuiStringIdsCombatSpam::attack_result_defender;
	else
		pp.stringId = CuiStringIdsCombatSpam::attack_result_defender_fullname;
	pp.actor.id = defender.getNetworkId();
	ProsePackageManagerClient::setTranslation(pp, ms_spamStrings[resultIndex]);
	ms_spamStringsSet[resultIndex] = true;
}

//----------------------------------------------------------------------

void CuiCombatManager::buildAttackMessageDefenderBrief(const ClientObject & defender, int resultIndex)
{
	ProsePackage pp;
	if (defender.asTangibleObject() != NULL && defender.asTangibleObject()->isPlayer())
		pp.stringId = CuiStringIdsCombatSpam::attack_result_defender_brief;
	else
		pp.stringId = CuiStringIdsCombatSpam::attack_result_defender_fullname_brief;
	pp.actor.id = defender.getNetworkId();
	ProsePackageManagerClient::setTranslation(pp, ms_spamStringsBrief[resultIndex]);
	ms_spamStringsSetBrief[resultIndex] = true;
}

//----------------------------------------------------------------------

void CuiCombatManager::buildAttackMessageAttackName(const StringId & attackName, int resultIndex)
{
	ProsePackage pp;
	pp.stringId = CuiStringIdsCombatSpam::attack_result_attack_name;
	pp.actor.stringId = attackName;
	ProsePackageManagerClient::setTranslation(pp, ms_spamStrings[resultIndex]);
	ms_spamStringsSet[resultIndex] = true;
}

//----------------------------------------------------------------------

void CuiCombatManager::buildAttackMessageWeapon(const ClientObject & weapon, int resultIndex)
{
	ProsePackage pp;
	pp.stringId = CuiStringIdsCombatSpam::attack_result_weapon;
	pp.actor.id = weapon.getNetworkId();
	ProsePackageManagerClient::setTranslation(pp, ms_spamStrings[resultIndex]);
	ms_spamStringsSet[resultIndex] = true;
}

//----------------------------------------------------------------------

void CuiCombatManager::buildAttackMessageWeapon(const StringId & weaponName, int resultIndex)
{
	ProsePackage pp;
	pp.stringId = CuiStringIdsCombatSpam::attack_result_weapon;
	pp.actor.stringId = weaponName;
	ProsePackageManagerClient::setTranslation(pp, ms_spamStrings[resultIndex]);
	ms_spamStringsSet[resultIndex] = true;
}

//----------------------------------------------------------------------

void CuiCombatManager::buildAttackMessageSuccess(bool success, bool critical, bool glance, bool crushing, bool strikethrough, float strikethroughAmmount, bool evadeResult, float evadeAmmount, bool blockResult, int block, bool dodge, bool parry, int resultIndex)
{
	CuiStringVariablesData data;
	if (success)
	{
		if (critical)
			CuiStringVariablesManager::process(CuiStringIdsCombatSpam::attack_result_success_critical, data, ms_spamStrings[resultIndex]);
		else if (glance)
			CuiStringVariablesManager::process(CuiStringIdsCombatSpam::attack_result_success_glance, data, ms_spamStrings[resultIndex]);
		else if (crushing)
			CuiStringVariablesManager::process(CuiStringIdsCombatSpam::attack_result_success_crush, data, ms_spamStrings[resultIndex]);
		else if (strikethrough)
		{
			ProsePackage pp;
			pp.stringId = CuiStringIdsCombatSpam::attack_result_success_strikethrough;
			pp.digitInteger = static_cast<int>(strikethroughAmmount);
			ProsePackageManagerClient::setTranslation(pp, ms_spamStrings[resultIndex]);
		}
		else if (evadeResult)
		{
			ProsePackage pp;
			pp.stringId = CuiStringIdsCombatSpam::attack_result_success_evade;
			pp.digitInteger = static_cast<int>(evadeAmmount);
			ProsePackageManagerClient::setTranslation(pp, ms_spamStrings[resultIndex]);
		}
		else if (blockResult)
		{
			ProsePackage pp;
			pp.stringId = CuiStringIdsCombatSpam::attack_result_success_blocked;
			pp.digitInteger = block;
			ProsePackageManagerClient::setTranslation(pp, ms_spamStrings[resultIndex]);
		}
		else
			CuiStringVariablesManager::process(CuiStringIdsCombatSpam::attack_result_success, data, ms_spamStrings[resultIndex]);
	}
	else
	{
		if (dodge)
			CuiStringVariablesManager::process(CuiStringIdsCombatSpam::attack_result_fail_dodge, data, ms_spamStrings[resultIndex]);
		else if (parry)
			CuiStringVariablesManager::process(CuiStringIdsCombatSpam::attack_result_fail_parry, data, ms_spamStrings[resultIndex]);
		else
			CuiStringVariablesManager::process(CuiStringIdsCombatSpam::attack_result_fail, data, ms_spamStrings[resultIndex]);
	}
	ms_spamStringsSet[resultIndex] = true;
}

//----------------------------------------------------------------------

void CuiCombatManager::buildAttackMessageSuccessBrief(bool success, bool critical, bool glance, bool crushing, bool strikethrough, bool evadeResult, bool blockResult, bool dodge, bool parry, int resultIndex)
{
	CuiStringVariablesData data;
	if (success)
	{
		if (critical)
			CuiStringVariablesManager::process(CuiStringIdsCombatSpam::attack_result_success_brief_critical, data, ms_spamStringsBrief[resultIndex]);
		else if (glance)
			CuiStringVariablesManager::process(CuiStringIdsCombatSpam::attack_result_success_brief_glance, data, ms_spamStringsBrief[resultIndex]);
		else if (crushing)
			CuiStringVariablesManager::process(CuiStringIdsCombatSpam::attack_result_success_brief_crush, data, ms_spamStringsBrief[resultIndex]);
		else if (strikethrough)
			CuiStringVariablesManager::process(CuiStringIdsCombatSpam::attack_result_success_brief_strikethrough, data, ms_spamStringsBrief[resultIndex]);
		else if (evadeResult)
			CuiStringVariablesManager::process(CuiStringIdsCombatSpam::attack_result_success_brief_evade, data, ms_spamStringsBrief[resultIndex]);
		else if (blockResult)
			CuiStringVariablesManager::process(CuiStringIdsCombatSpam::attack_result_success_brief_blocked, data, ms_spamStringsBrief[resultIndex]);
		else
			CuiStringVariablesManager::process(CuiStringIdsCombatSpam::attack_result_success_brief, data, ms_spamStringsBrief[resultIndex]);
	}
	else
	{
		if (dodge)
			CuiStringVariablesManager::process(CuiStringIdsCombatSpam::attack_result_fail_brief_dodge, data, ms_spamStringsBrief[resultIndex]);
		else if (parry)
			CuiStringVariablesManager::process(CuiStringIdsCombatSpam::attack_result_fail_brief_parry, data, ms_spamStringsBrief[resultIndex]);
		else
			CuiStringVariablesManager::process(CuiStringIdsCombatSpam::attack_result_fail_brief, data, ms_spamStringsBrief[resultIndex]);
	}
	ms_spamStringsSetBrief[resultIndex] = true;
}

//----------------------------------------------------------------------

void CuiCombatManager::buildAttackMessageTotalDamage(int damage, int resultIndex)
{
	ProsePackage pp;
	pp.stringId = CuiStringIdsCombatSpam::attack_result_total_damage;
	pp.digitInteger = damage;
	ProsePackageManagerClient::setTranslation(pp, ms_spamStrings[resultIndex]);
	ms_spamStringsSet[resultIndex] = true;
}

//----------------------------------------------------------------------

void CuiCombatManager::buildAttackMessageTotalDamageBrief(int damage, int resultIndex)
{
	ProsePackage pp;
	pp.stringId = CuiStringIdsCombatSpam::attack_result_total_damage_brief;
	pp.digitInteger = damage;
	ProsePackageManagerClient::setTranslation(pp, ms_spamStringsBrief[resultIndex]);
	ms_spamStringsSetBrief[resultIndex] = true;
}

//----------------------------------------------------------------------

void CuiCombatManager::buildAttackMessageDamageDetail(int damage, int resultIndex)
{
	ProsePackage pp;
	pp.stringId = CuiStringIdsCombatSpam::attack_result_damage;
	pp.digitInteger = damage;
	ProsePackageManagerClient::setTranslation(pp, ms_spamStrings[resultIndex]);
	ms_spamStringsSet[resultIndex] = true;
}

//----------------------------------------------------------------------

void CuiCombatManager::buildAttackMessageDamageType(int damageType, int resultIndex)
{
	StringId sid;
	CuiStringVariablesData data;
	switch (damageType)
	{
		case WeaponObject::DT_kinetic:
			sid = CuiStringIdsCombatSpam::attack_result_damage_kinetic;
			break;
		case WeaponObject::DT_energy:
			sid = CuiStringIdsCombatSpam::attack_result_damage_energy;
			break;
		case WeaponObject::DT_elemental_heat:
			sid = CuiStringIdsCombatSpam::attack_result_elemental_heat;
			break;
		case WeaponObject::DT_elemental_cold:
			sid = CuiStringIdsCombatSpam::attack_result_elemental_cold;
			break;
		case WeaponObject::DT_elemental_acid:
			sid = CuiStringIdsCombatSpam::attack_result_elemental_acid;
			break;
		case WeaponObject::DT_elemental_electrical:
			sid = CuiStringIdsCombatSpam::attack_result_elemental_electrical;
			break;
		case WeaponObject::DT_environmental_heat:
			sid = CuiStringIdsCombatSpam::attack_result_environmental_heat;
			break;
		case WeaponObject::DT_environmental_cold:
			sid = CuiStringIdsCombatSpam::attack_result_environmental_cold;
			break;
		case WeaponObject::DT_environmental_acid:
			sid = CuiStringIdsCombatSpam::attack_result_environmental_acid;
			break;
		case WeaponObject::DT_environmental_electrical:
			sid = CuiStringIdsCombatSpam::attack_result_environmental_electrical;
			break;
		default:
			WARNING(true, ("CuiCombatManager::buildAttackMessageDamageType unexpected damage type %d", damageType));
			return;
	}
	CuiStringVariablesManager::process(sid, data, ms_spamStrings[resultIndex]);
	ms_spamStringsSet[resultIndex] = true;
}

//----------------------------------------------------------------------

void CuiCombatManager::buildAttackMessageArmor(const ClientObject & defender, int resultIndex)
{
	ProsePackage pp;
	if (defender.asTangibleObject() != NULL && defender.asTangibleObject()->isPlayer())
		pp.stringId = CuiStringIdsCombatSpam::attack_result_armor_generic;
	else
		pp.stringId = CuiStringIdsCombatSpam::attack_result_armor_generic_fullname;
	pp.actor.id = defender.getNetworkId();
	ProsePackageManagerClient::setTranslation(pp, ms_spamStrings[resultIndex]);
	ms_spamStringsSet[resultIndex] = true;
}

//----------------------------------------------------------------------

void CuiCombatManager::buildAttackMessageArmorProtection(int blockedDamage, int resultIndex)
{
	ProsePackage pp;
	pp.stringId = CuiStringIdsCombatSpam::attack_result_armor_protection;
	pp.digitInteger = blockedDamage;
	ProsePackageManagerClient::setTranslation(pp, ms_spamStrings[resultIndex]);
	ms_spamStringsSet[resultIndex] = true;
}

//----------------------------------------------------------------------

void CuiCombatManager::buildAttackMessageRawDamage(int rawDamage, int resultIndex)
{
	ProsePackage pp;
	pp.stringId = CuiStringIdsCombatSpam::attack_result_raw_damage;
	pp.digitInteger = rawDamage;
	ProsePackageManagerClient::setTranslation(pp, ms_spamStrings[resultIndex]);
	ms_spamStringsSet[resultIndex] = true;
}

//----------------------------------------------------------------------

void CuiCombatManager::buildAttackMessageGeneric(const StringId & stringId, int resultIndex)
{
	CuiStringVariablesData data;
	CuiStringVariablesManager::process(stringId, data, ms_spamStrings[resultIndex]);
	ms_spamStringsSet[resultIndex] = true;
}

//----------------------------------------------------------------------
// The following two must be kept in sync.  They were separated to keep work being done
// off of the stack for speed.
void CuiCombatManager::buildAttackMessage(Unicode::String & result)
{
	int size = SENTENCE_STRUCTURE_NUMBER_STRUCTURES;
	Unicode::String preformatted;
	Unicode::unicode_char_t resultChars[1024];
	int i;
	for (i = 0; i < size; i++)
	{
		if (ms_spamStringsSet[ms_spamOrder[i]] == true)
		{
			preformatted += ms_spamStrings[ms_spamOrder[i]] + ms_space;
			ms_spamStringsSet[ms_spamOrder[i]] = false;
		}
	}

	// Remove excess spaces and toUpper() the first character
	preformatted = Unicode::trim(preformatted);
	preformatted = Unicode::toUpper(preformatted.substr(0,1)) + preformatted.substr(1);
	int length = preformatted.length();
	int periodPos = preformatted.find(ms_period);
	if (periodPos != preformatted.npos &&
		periodPos < length - 1)
	{
		int toUpperPos = preformatted.find_first_not_of(' ', periodPos+1);
		preformatted = preformatted.substr(0, toUpperPos-1) +
		               Unicode::toUpper(preformatted.substr(toUpperPos, 1)) +
		               preformatted.substr(toUpperPos+1);
	}

	// Remove spaces prior to characters that shouldn't have spaces before them
	int pos = 0;
	const Unicode::unicode_char_t *chars = preformatted.c_str();
	for (i = 0; i < length - 1; i++)
	{
		if (isSpace(chars[i]) &&
		  !shouldHaveSpacesBefore(chars[i+1]))
		{
			continue;
		}
		else if (!shouldHaveSpacesAfter(chars[i]) &&
		         isSpace(chars[i+1]))
		{
			resultChars[pos++] = chars[i++];
		}
		else
		{
			resultChars[pos++] = chars[i];
		}
	}
	resultChars[pos++] = chars[length-1];
	resultChars[pos] = 0;

	result = Unicode::String(resultChars);
}

//----------------------------------------------------------------------

// This must be kept in sync with the above buildAttackMessage() function
void CuiCombatManager::buildAttackMessageBrief(Unicode::String & result)
{
	int size = SENTENCE_STRUCTURE_NUMBER_STRUCTURES_BRIEF;
	Unicode::String preformatted;
	Unicode::unicode_char_t resultChars[1024];
	int i;
	for (i = 0; i < size; i++)
	{
		if (ms_spamStringsSetBrief[ms_spamOrderBrief[i]] == true)
		{
			preformatted += ms_spamStringsBrief[ms_spamOrderBrief[i]] + ms_space;
			ms_spamStringsSetBrief[ms_spamOrderBrief[i]] = false;
		}
	}

	// Remove excess spaces and toUpper() the first character
	preformatted = Unicode::trim(preformatted);
	preformatted = Unicode::toUpper(preformatted.substr(0,1)) + preformatted.substr(1);
	int length = preformatted.length();
	int periodPos = preformatted.find(ms_period);
	if (periodPos != preformatted.npos &&
		periodPos < length - 1)
	{
		int toUpperPos = preformatted.find_first_not_of(' ', periodPos+1);
		preformatted = preformatted.substr(0, toUpperPos-1) +
		               Unicode::toUpper(preformatted.substr(toUpperPos, 1)) +
		               preformatted.substr(toUpperPos+1);
	}

	// Remove spaces prior to characters that shouldn't have spaces before them
	int pos = 0;
	const Unicode::unicode_char_t *chars = preformatted.c_str();
	for (i = 0; i < length - 1; i++)
	{
		if (isSpace(chars[i]) &&
		    !shouldHaveSpacesBefore(chars[i+1]))
		{
			continue;
		}
		else if (!shouldHaveSpacesAfter(chars[i]) &&
		         isSpace(chars[i+1]))
		{
			resultChars[pos++] = chars[i++];
		}
		else
		{
			resultChars[pos++] = chars[i];
		}
	}
	resultChars[pos++] = chars[length-1];
	resultChars[pos] = 0;

	result = Unicode::String(resultChars);
}

//----------------------------------------------------------------------

// WARNING:  This is only to be used in this class.  It only works with
// the many assumptions made about combat spam.
bool CuiCombatManager::shouldHaveSpacesAfter(Unicode::unicode_char_t value)
{
	return (value < 0x2E80 &&
	        value != 0x0028);    // Open parentheses '('
}

//----------------------------------------------------------------------

// WARNING:  This is only to be used in this class.  It only works with
// the many assumptions made about combat spam.
bool CuiCombatManager::shouldHaveSpacesBefore(Unicode::unicode_char_t value)
{
	return (value < 0x2E80 &&
	        value != 0x0029 &&    // Close parentheses ')'
	        value != 0x002E);     // Period '.'
}

//----------------------------------------------------------------------

// WARNING:  This is only to be used in this class.  It only works with
// the many assumptions made about combat spam.  This is only for spaces,
// not whitespace in general.
bool CuiCombatManager::isSpace(Unicode::unicode_char_t value)
{
	return (value == 0x0020);     // Space ' '
}

//----------------------------------------------------------------------

uint32 CuiCombatManager::getPendingCommand()
{
	std::vector<uint32> iv;
	getCombatCommandsFromQueue (iv);
	std::vector<uint32>::const_iterator ivIt = iv.begin ();
	if((ivIt != iv.end()) && (++ivIt != iv.end()))
	{
		const uint32 sequenceId = (*ivIt);
		return sequenceId;
	}
	return 0;
}

//----------------------------------------------------------------------

bool CuiCombatManager::hasAnyCommandsInQueue()
{
	const ClientCommandQueue::EntryMap & em = ClientCommandQueue::get ();

	for (ClientCommandQueue::EntryMap::const_iterator it = em.begin (); it != em.end (); ++it)
	{
		const ClientCommandQueue::Entry & entry = (*it).second;

		if (entry.m_command && isCombatQueueCommand (*entry.m_command))
			return true;
	}
	return false;
}

//----------------------------------------------------------------------

void CuiCombatManager::update  (float deltaTime)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	updateDeferredCombatActions(deltaTime);
	updateDamageRecords(deltaTime);
}

//----------------------------------------------------------------------

void CuiCombatManager::updateDeferredCombatActions(float deltaTime)
{
	DelayedCombatActionMap::iterator itr = s_delayedDamageAction.begin();
	while (itr != s_delayedDamageAction.end())
	{
		bool foundExpiredActions = false;

		NetworkId const &defenderId = ((*itr).first).second;

		CombatActionList &combatActions = (*itr).second;
		
		CombatActionList::iterator combatActionItr = combatActions.begin();
		while (combatActionItr != combatActions.end())
		{
			DelayedCombatActionObject &combatActionObject = (*combatActionItr);
			if (combatActionObject.m_elapsedTime < 0.f)
			{
				// make sure that the action gets at least one full frame before being processed
				combatActionObject.m_elapsedTime = 0.f;
			}
			else
			{
				combatActionObject.m_elapsedTime += deltaTime;
				if (combatActionObject.m_elapsedTime >= s_maxDelayForCombatAction)
				{
					foundExpiredActions = true;
#ifdef _DEBUG
					NetworkId const &attackerId = ((*itr).first).first;

					// only warn once per attacker per session
					static std::map<NetworkId, int> objectsReported;
					std::map<NetworkId, int>::iterator reportItr = objectsReported.find(attackerId);
					if (reportItr == objectsReported.end())
					{
						Object const * const attackerObj = NetworkIdManager::getObjectById(attackerId);
						Object const * const defenderObj = NetworkIdManager::getObjectById(defenderId);
						DEBUG_WARNING(true, ("validateCombatActionText() found text %s that was never removed attacker[%s - %s] defender[%s - %s].\n", combatActionObject.m_flyText.c_str(), attackerId.getValueString().c_str(), attackerObj ? attackerObj->getObjectTemplateName() : "invalid object", defenderId.getValueString().c_str(), defenderObj ? defenderObj->getObjectTemplateName() : "invalid object"));
						objectsReported[attackerId] = 1;
					}
					else
					{
						(*reportItr).second += 1;
					}
#endif
				}
			}
			++combatActionItr;
		}

		// force the expired events to be processed
		if (foundExpiredActions)
		{
			const bool updateExpiredOnly = true;
			processCombatActions(itr, defenderId, updateExpiredOnly);
		}

		// if there are no more combat actions for this key, remove it
		combatActions = (*itr).second;

		if (combatActions.begin() == combatActions.end())
		{
			s_delayedDamageAction.erase(itr);
		}
		++itr;
	}
}

//----------------------------------------------------------------------

void CuiCombatManager::removeCompletedCombatAction(const NetworkId & attackerId, const NetworkId & defenderId)
{
	CombatActionPair key = std::make_pair(attackerId, defenderId);

	DelayedCombatActionMap::iterator itr = s_delayedDamageAction.find(key);
	if (itr != s_delayedDamageAction.end())
	{
		processCombatActions(itr, defenderId);

		CombatActionList &combatActions = (*itr).second;
		if (combatActions.begin() == combatActions.end())
		{
			s_delayedDamageAction.erase(itr);
		}
	}
}

//----------------------------------------------------------------------

void CuiCombatManager::addDeferredCombatActionDamage(const NetworkId & attackerId, const NetworkId & defenderId, int damageAmount)
{
	// add fly text for damage
	//-- Construct the damage string.
	char buffer[32];
	_snprintf (buffer, sizeof (buffer), "-%d", damageAmount);
	Unicode::String damageText = Unicode::narrowToWide (buffer);

	//-- Retrieve remaining fly text parameters.
	float fadeTime     = 3.0f;

	Object const * const defenderObj = NetworkIdManager::getObjectById(defenderId);
	Object const * const attackerObj = NetworkIdManager::getObjectById(attackerId);
	Object const * const playerObj = Game::getPlayer();

	VectorArgb color = s_otherDamageColor;
	float textScale = 1.f;

	// @NOTE: only show damage to/from self or group members until UI options are hooked up
	bool showText = false;

	if(playerObj == defenderObj)
		addDamageTaken(damageAmount);
	if(playerObj == attackerObj)
		addDamageDone(damageAmount);

	if (damageAmount > 0)
	{
		if ((playerObj == defenderObj) && ms_showDamageOverHeadSelf)
		{
			color = s_damageToPlayerColor;
			showText = true;
		}
		else if ((playerObj == attackerObj) && ms_showDamageOverHeadOthers)
		{
			color = s_damageToTargetColor;
			showText = true;
		}
		else if ((attackerOrDefenderInPlayerGroup(attackerId, defenderId)) && ms_showDamageOverHeadSelf)
		{
			color = s_groupMemberDamage;
			showText = true;
		}
	}
	else if ((playerObj == defenderObj) || (playerObj == attackerObj) || attackerOrDefenderInPlayerGroup(attackerId, defenderId))
	{
		color == VectorArgb::solidGreen;
		showText = true;
	}

	static VectorArgb currentColor(color);

	// add the text to the defender (this object)
	addDeferredCombatAction(attackerId, defenderId, damageText, fadeTime, color, textScale, true, damageAmount, showText);
}

//----------------------------------------------------------------------

void CuiCombatManager::addDeferredCombatAction(const NetworkId & attackerId, const NetworkId & defenderId, Unicode::String & flyText, float fadeTime, VectorArgb & color, float scale, bool fadeOut, int damageAmount, bool showText)
{
	DelayedCombatActionObject combatAction(flyText, fadeTime, color, scale, fadeOut, damageAmount, showText);

	CombatActionPair key = std::make_pair(attackerId, defenderId);

	DelayedCombatActionMap::iterator itr = s_delayedDamageAction.find(key);

	if (itr != s_delayedDamageAction.end())
	{
		CombatActionList &combatActions = (*itr).second;

		combatActions.push_back(combatAction);
	}
	else
	{
		CombatActionList newList;
		newList.push_back(combatAction);

		s_delayedDamageAction[key] = newList;
	}
}

bool CuiCombatManager::hasPendingAttack (const NetworkId & defenderId)
{
	bool hasPendingAttack = false;

	DelayedCombatActionMap::iterator itr = s_delayedDamageAction.begin();
	while (itr != s_delayedDamageAction.end())
	{
		if (defenderId == ((*itr).first).second)
		{
			hasPendingAttack = true;
			break;
		}
		++itr;
	}

	return hasPendingAttack;
}

//----------------------------------------------------------------------

void CuiCombatManagerNamespace::processCombatActions(DelayedCombatActionMap::iterator &itr, const NetworkId & defenderId, bool updateExpiredOnly)
{
	if (itr != s_delayedDamageAction.end())
	{
		Object * defenderObj = NetworkIdManager::getObjectById(defenderId);
		ClientObject * defenderClientObj = defenderObj ? defenderObj->asClientObject() : 0;

		const int heavyText = (Game::getPlayer () == defenderObj) ? CuiTextManagerTextEnqueueInfo::TW_starwars : CuiTextManagerTextEnqueueInfo::TW_starwars;

		CombatActionList &combatActions = (*itr).second;
		CombatActionList::iterator combatActionItr = combatActions.begin();

		while (combatActionItr != combatActions.end())
		{
			DelayedCombatActionObject &combatActionObject = (*combatActionItr);

			bool updateAction = updateExpiredOnly ? (s_maxDelayForCombatAction < combatActionObject.m_elapsedTime) : true;

			// try to avoid the case where a new event was added just as the previous event was resolved
			if (updateAction)
			{	
				// add the text to the defender (this object)
				if (defenderClientObj && combatActionObject.m_showText)
				{
					const bool renderWithChatBubbles = true;
					const bool useScreenSpace = true;
					VectorArgb const & color = VectorArgb::solidYellow;

					defenderClientObj->addFlyText (combatActionObject.m_flyText, 1.0f, color, combatActionObject.m_textScale, heavyText, combatActionObject.m_fadeOut, renderWithChatBubbles, useScreenSpace);
				}

				combatActionItr = combatActions.erase(combatActionItr);
			}
			else
			{
				++combatActionItr;
			}
		}
	}
}

//----------------------------------------------------------------------

bool CuiCombatManagerNamespace::attackerOrDefenderInPlayerGroup(const NetworkId &attackerId, const NetworkId &defenderId)
{
	bool inGroup = false;

	CreatureObject * const player = Game::getPlayerCreature ();
	if (player)
	{
		const GroupObject * const group = safe_cast<GroupObject *>(player->getGroup ().getObject ());
		if (group)
		{
			const GroupObject::GroupMemberVector & gmv = group->getGroupMembers ();
			{
				for (GroupObject::GroupMemberVector::const_iterator it = gmv.begin (); it != gmv.end (); ++it)
				{
					const GroupObject::GroupMember & mem = *it;
					const NetworkId & memId = mem.first;
					if ((attackerId == memId) || (defenderId == memId))
					{
						inGroup = true;
						break;
					}
				}
			}
		}
	}

	return inGroup;
}

//----------------------------------------------------------------------

void CuiCombatManager::getCurrentDps(float &damageDone, float &damageTaken, int &totalDamageDone, int &totalDamageTaken)
{
	damageDone = ms_damageDone / DPS_TIME_INTERVAL;
	damageTaken = ms_damageTaken / DPS_TIME_INTERVAL;
	totalDamageDone = ms_damageDoneTotal;
	totalDamageTaken = ms_damageTakenTotal;
}

//----------------------------------------------------------------------

void CuiCombatManager::addDamageDone(int damageAmount)
{
	ms_damageDone += damageAmount;
	ms_damageDoneTotal += damageAmount;
	ms_damageDoneRecords.push_back(std::make_pair(damageAmount, DPS_TIME_INTERVAL));
}

//----------------------------------------------------------------------

void CuiCombatManager::resetDps()
{
	clearDamageRecords();
	ms_damageDoneTotal = 0;
	ms_damageTakenTotal = 0;
}

//======================================================================
