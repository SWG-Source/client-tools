//======================================================================
//
// CuiCombatManager.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiCombatManager_H
#define INCLUDED_CuiCombatManager_H

//======================================================================

class CachedNetworkId;
class ClientObject;
class Command;
class Controller;
class CreatureObject;
class MessageQueueCombatSpam;
class NetworkId;
class TangibleObject;
class StringId;
class Vector;
class VectorArgb;


struct UIColor;

//----------------------------------------------------------------------

class CuiCombatManager
{
public:

	struct Messages
	{
		struct CombatSpamReceived
		{
			typedef std::pair<Unicode::String, int> Payload; // string, spam type
		};
	};

	enum CombatResult // This is synced with base_class.java, search for COMBAT_RESULT_
	{
		COMBAT_RESULT_MISS               = 0,
		COMBAT_RESULT_HIT                = 1,
		COMBAT_RESULT_BLOCK              = 2,
		COMBAT_RESULT_EVADE              = 3,
		COMBAT_RESULT_REDIRECT           = 4,
		COMBAT_RESULT_COUNTER            = 5,
		COMBAT_RESULT_FUMBLE             = 6,
		COMBAT_RESULT_LIGHTSABER_BLOCK   = 7,
		COMBAT_RESULT_LIGHTSABER_COUNTER = 8,
		COMBAT_RESULT_LIGHTSABER_COUNTER_TARGET = 9,  // !!!!! this is the last valid combat result that can be used in doCombatResults() - if you change this, make sure method is updated
		COMBAT_RESULT_GENERIC            = 10,
		COMBAT_RESULT_OUT_OF_RANGE       = 11,
		COMBAT_RESULT_POSTURE_CHANGE     = 12,
		COMBAT_RESULT_TETHERED           = 13, // AI tether is forcing AI home
		COMBAT_RESULT_MEDICAL            = 14,
		COMBAT_RESULT_BUFF               = 15,
		COMBAT_RESULT_DEBUFF             = 16
	};

	static void                            install ();
	static void                            remove ();

	static void                            update (float deltaTime);

	static bool                            getShowDamageOverHeadOthers   ();
	static void                            setShowDamageOverHeadOthers   (bool b);

	static bool                            getShowDamageOverHeadSelf     ();
	static void                            setShowDamageOverHeadSelf     (bool b);

	static bool                            getShowDamageSnare     ();
	static void                            setShowDamageSnare     (bool b);
	static bool                            getShowDamageGlancingBlow     ();
	static void                            setShowDamageGlancingBlow     (bool b);
	static bool                            getShowDamageCriticalHit     ();
	static void                            setShowDamageCriticalHit     (bool b);
	static bool                            getShowDamageLucky     ();
	static void                            setShowDamageLucky     (bool b);
	static bool                            getShowDamageDot     ();
	static void                            setShowDamageDot     (bool b);
	static bool                            getShowDamageBleed     ();
	static void                            setShowDamageBleed     (bool b);
	static bool                            getShowDamageHeal     ();
	static void                            setShowDamageHeal     (bool b);
	static bool                            getShowDamageFreeshot     ();
	static void                            setShowDamageFreeshot     (bool b);
	static bool                            getShowEnteringCombat     ();
	static void                            setShowEnteringCombat     (bool b);

	static float                           getDamageDoneToMeSizeModifier ();
	static void                            setDamageDoneToMeSizeModifier (float f);
	static float                           getDamageDoneToOthersSizeModifier ();
	static void                            setDamageDoneToOthersSizeModifier (float f);
	static float                           getNonDamageDoneToMeSizeModifier ();
	static void                            setNonDamageDoneToMeSizeModifier (float f);
	static float                           getNonDamageDoneToOthersSizeModifier ();
	static void                            setNonDamageDoneToOthersSizeModifier (float f);


	static void                            processCombatSpam          (const MessageQueueCombatSpam & spamMsg);

	static const CachedNetworkId &         getCombatTarget            ();
	static void                            setCombatTarget            (const NetworkId & id);

	static bool                            isInCombat                 (const CreatureObject * actor, CachedNetworkId & id);

	static const CachedNetworkId &         getLookAtTarget            ();
	static void                            setLookAtTarget            (const NetworkId & id);

	static void                            requestPeace               ();

	/**
	* start a default attack against this object, adding to the list if needed, and making the target the current selection
	*/

	static bool                            simulateSinglePlayerCombat ();
	static bool                            simulateSinglePlayerCombat (const NetworkId & id);

	static bool                            executeCommand             (const std::string & name);
	static bool                            describeCommand            (const std::string & name);

	static void                            reset                      ();

	static const CachedNetworkId &         cycleTargetsOutward        ();
	static const CachedNetworkId &         cycleTargetsInward         ();
	static const CachedNetworkId &         cycleTargetsPrevious       ();
	static const CachedNetworkId &         cycleTargetsNext           ();
	static const CachedNetworkId &         cycleTargetsGroupPrevious  ();
	static const CachedNetworkId &         cycleTargetsGroupNext      ();
	static const CachedNetworkId &         cycleTargetsOutwardFriendly();
	static const CachedNetworkId &         cycleTargetsInwardFriendly();
	static const CachedNetworkId &         cycleTargetsOutwardAll();
	static const CachedNetworkId &         cycleTargetsInwardAll();

	static void                            pushTargetHistory          ();
	static void                            clearHistory               ();

	static void                            cycleComponentTargetForward();
	static void                            cycleComponentTargetBackward();

	static const CachedNetworkId &         getLastTargetedIdInQueue   ();

	static bool                            isCombatCommand            (uint32 commandHash);
	static bool                            isCombatCommand            (const std::string & command);
	static bool                            isCombatCommand            (const Command & cmd);

	typedef stdvector<uint32>::fwd         IntVector;
	static void                            getCombatCommandsFromQueue (IntVector & iv);

	static const std::string& getConsoleActionName                    ();
	static void               setConsoleActionName                    (const std::string& actionName);
	static int                getConsoleAttackerPostureEndIndex       ();
	static void               setConsoleAttackerPostureEndIndex       (int attackerPostureEndIndex);
	static int                getConsoleDefenderPostureEndIndex       ();
	static void               setConsoleDefenderPostureEndIndex       (int defenderPostureEndIndex);
	static int                getConsoleDefenderDefense               ();
	static void               setConsoleDefenderDefense               (int defenderDefense);

	static void               sendFakeCombatSpam                      (const NetworkId & idSource, const Vector & position_wSource, const NetworkId & idTarget, const Vector & position_wTarget, const NetworkId & idOther, int digit_i, float digit_f, const Unicode::String & str, int spamType = COMBAT_RESULT_GENERIC);

	static bool               getConMessage                           (const CreatureObject & enemy, Unicode::String & _message);
	static const UIColor &    getConColor                             (const CreatureObject & enemy);
	static const UIColor &    getInvulnerableConColor                 ();
	static const UIColor &    getConColor                             (int playerLevel, int targetLevel);

	static bool				  isCombatQueueCommand					  (const Command & cmd);
	static bool				  isCombatQueueCommand					  (const std::string & command);
	static bool				  isCombatQueueCommand					  (uint32 commandHash);

	static uint32             getPendingCommand                       ();
	static bool               hasAnyCommandsInQueue                   ();

	static void               removeCompletedCombatAction             (const NetworkId & attackerId, const NetworkId & defenderId);
	static void               addDeferredCombatActionDamage           (const NetworkId & attackerId, const NetworkId & defenderId, int damageAmount);
	static void               addDeferredCombatAction                 (const NetworkId & attackerId, const NetworkId & defenderId, Unicode::String & flyText, float fadeTime, VectorArgb & color, float scale, bool fadeOut = true, int damageAmount = 0, bool showText = true);
	static bool               hasPendingAttack                        (const NetworkId & defenderId);

	static void               addDamageDone                           (int damageDone);
	static void               getCurrentDps                           (float &damageDone, float &damageTaken, int &totalDamageDone, int &totalDamageTaken);
	static void               resetDps                                ();

private:
	
	static bool                        enqueueCommand    (const std::string & commandName, Controller * controller);
	static const CachedNetworkId &     cycleTargetsGroup (int increment);

	static void buildAttackMessageAttacker(const ClientObject & attacker, int resultIndex);
	static void buildAttackMessageAttackerBrief(const ClientObject & attacker, int resultIndex);
	static void buildAttackMessageDefender(const ClientObject & defender, int resultIndex);
	static void buildAttackMessageDefenderBrief(const ClientObject & defender, int resultIndex);
	static void buildAttackMessageAttackName(const StringId & attackName, int resultIndex);
	static void buildAttackMessageWeapon(const ClientObject & weapon, int resultIndex);
	static void buildAttackMessageWeapon(const StringId & weaponName, int resultIndex);
	static void buildAttackMessageSuccess(bool success, bool critical, bool glance, bool crushing, bool strikethrough, float strikethroughAmmount, bool evadeResult, float evadeAmmount, bool blockResult, int block, bool dodge, bool parry, int resultIndex);
	static void buildAttackMessageSuccessBrief(bool success, bool critical, bool glance, bool crushing, bool strikethrough, bool evadeResult, bool blockResult, bool dodge, bool parry, int resultIndex);
	static void buildAttackMessageTotalDamage(int damage, int resultIndex);
	static void buildAttackMessageTotalDamageBrief(int damage, int resultIndex);
	static void buildAttackMessageDamageDetail(int baseDamage, int resultIndex);
	static void buildAttackMessageDamageType(int damageType, int resultIndex);
	static void buildAttackMessageArmor(const ClientObject & defender, int resultIndex);
	static void buildAttackMessageArmorProtection(int blockedDamage, int resultIndex);
	static void buildAttackMessageRawDamage(int rawDamage, int resultIndex);
	static void buildAttackMessageGeneric(const StringId & stringId, int resultIndex);

	// These must be kept in sync.  Separated for speed, given the number of times these are called
	static void buildAttackMessage(Unicode::String & result);
	static void buildAttackMessageBrief(Unicode::String & result);

	static void fillSpamOrder(short spamOrder[], Unicode::String spamTemplate);

	static bool shouldHaveSpacesAfter(Unicode::unicode_char_t value);
	static bool shouldHaveSpacesBefore(Unicode::unicode_char_t value);
	static bool isSpace(Unicode::unicode_char_t value);

	static void updateDeferredCombatActions(float deltaTime);

	static uint32                      ms_combatQueueSequenceId;
	static CachedNetworkId             ms_lastTargetedIdInQueue;
	
	
	class CuiCombatManagerAction;
	static CuiCombatManagerAction *    ms_action;

	//-- console interface
	static std::string ms_consoleActionName;
	static int         ms_consoleAttackerPostureEndIndex;
	static int         ms_consoleDefenderPostureEndIndex;
	static int         ms_consoleDefenderDefense;
	static bool        ms_showDamageOverHeadOthers;
	static bool        ms_showDamageOverHeadSelf;
	static bool        ms_showDamageSnare;
	static bool        ms_showDamageGlancingBlow;
	static bool        ms_showDamageCriticalHit;
	static bool        ms_showDamageLucky;
	static bool        ms_showDamageDot;
	static bool        ms_showDamageBleed;
	static bool        ms_showDamageHeal;
	static bool        ms_showDamageFreeshot;
	static float       ms_damageDoneToMeSizeModifier;
	static float       ms_damageDoneToOthersSizeModifier;
	static float       ms_nonDamageDoneToMeSizeModifier;
	static float       ms_nonDamageDoneToOthersSizeModifier;
	static bool        ms_showEnteringCombat;
};

//----------------------------------------------------------------------

inline const CachedNetworkId &  CuiCombatManager::getLastTargetedIdInQueue ()
{
	return ms_lastTargetedIdInQueue;
}

//----------------------------------------------------------------------

inline bool CuiCombatManager::getShowDamageOverHeadOthers ()
{
	return ms_showDamageOverHeadOthers;
}


//----------------------------------------------------------------------

inline bool CuiCombatManager::getShowDamageGlancingBlow()
{
	return ms_showDamageGlancingBlow;
}

//----------------------------------------------------------------------

inline bool CuiCombatManager::getShowDamageCriticalHit()
{
	return ms_showDamageCriticalHit;
}

//----------------------------------------------------------------------

inline bool CuiCombatManager::getShowDamageLucky()
{
	return ms_showDamageLucky;
}

//----------------------------------------------------------------------

inline bool CuiCombatManager::getShowDamageDot()
{
	return ms_showDamageDot;
}

//----------------------------------------------------------------------

inline bool CuiCombatManager::getShowDamageBleed()
{
	return ms_showDamageBleed;
}

//----------------------------------------------------------------------

inline bool CuiCombatManager::getShowDamageHeal()
{
	return ms_showDamageHeal;
}

//----------------------------------------------------------------------

inline bool CuiCombatManager::getShowDamageFreeshot()
{
	return ms_showDamageFreeshot;
}

//----------------------------------------------------------------------

inline bool CuiCombatManager::getShowDamageOverHeadSelf ()
{
	return ms_showDamageOverHeadSelf;
}

//----------------------------------------------------------------------

inline bool CuiCombatManager::getShowDamageSnare()
{
	return ms_showDamageSnare;
}

//----------------------------------------------------------------------

inline float CuiCombatManager::getDamageDoneToMeSizeModifier ()
{
	return ms_damageDoneToMeSizeModifier;
}

//----------------------------------------------------------------------

inline float CuiCombatManager::getDamageDoneToOthersSizeModifier ()
{
	return ms_damageDoneToOthersSizeModifier;
}

//----------------------------------------------------------------------

inline float CuiCombatManager::getNonDamageDoneToMeSizeModifier ()
{
	return ms_nonDamageDoneToMeSizeModifier;
}

//----------------------------------------------------------------------

inline float CuiCombatManager::getNonDamageDoneToOthersSizeModifier ()
{
	return ms_nonDamageDoneToOthersSizeModifier;
}

//----------------------------------------------------------------------

inline bool CuiCombatManager::getShowEnteringCombat()
{
	return ms_showEnteringCombat;
}
//======================================================================

#endif

