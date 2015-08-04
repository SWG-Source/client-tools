package script.conversation;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.ai_lib;
import script.library.badge;
import script.library.buff;
import script.library.chat;
import script.library.collection;
import script.library.conversation;
import script.library.create;
import script.library.factions;
import script.library.groundquests;
import script.library.holiday;
import script.library.space_quest;
import script.library.static_item;
import script.library.utils;


public class imperial_empire_day_major_tantor extends script.base_script
{
	public imperial_empire_day_major_tantor()
	{
	}
	String c_stringFile = "conversation/imperial_empire_day_major_tantor";
	
	
	public boolean imperial_empire_day_major_tantor_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_alreadyDoingRecruitment(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return buff.hasBuff(player, holiday.BUFF_IMP_EMPIREDAY_RECRUITMENT_COMBATANT) || buff.hasBuff(player, holiday.BUFF_IMP_EMPIREDAY_RECRUITMENT_SF);
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_hasCompletedBothCollections(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (imperial_empire_day_major_tantor_condition_hasCompletedPropagandaCollection(player, npc) && imperial_empire_day_major_tantor_condition_hasCompletedRecruitmentCollection(player, npc));
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_isSpecialForcesImperial(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (factions.isDeclared(player) && factions.isImperial(player));
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_hasRecruitmentCollectionPrerequisite(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasCompletedCollectionSlotPrereq(player, holiday.IMPERIAL_RECRUITING_COUNTER_SLOT);
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_alreadyDoingPropaganda(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return buff.hasBuff(player, holiday.BUFF_IMP_EMPIREDAY_ANTIPROP_COMBATANT) || buff.hasBuff(player, holiday.BUFF_IMP_EMPIREDAY_ANTIPROP_SF);
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_alreadyKnowsTantor(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (imperial_empire_day_major_tantor_condition_hasRecruitmentCollectionPrerequisite(player, npc) || imperial_empire_day_major_tantor_condition_hasPropagandaCollectionPrerequisite(player, npc));
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_hasPropagandaCollectionPrerequisite(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasCompletedCollectionSlotPrereq(player, holiday.IMPERIAL_ANTIPROP_COUNTER_SLOT);
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_hasEmpireDayBadge(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasCompletedCollectionSlot(player, holiday.EMPIRE_DAY_CHAMPION_BADGE);
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_isQualifiedRecruitmentSpecialForces(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (imperial_empire_day_major_tantor_condition_isSpecialForcesImperial(player, npc) && !imperial_empire_day_major_tantor_condition_alreadyDoingRecruitment(player, npc));
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_isQualifiedPropagandaSpecialForces(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (imperial_empire_day_major_tantor_condition_isSpecialForcesImperial(player, npc) && !imperial_empire_day_major_tantor_condition_alreadyDoingPropaganda(player, npc));
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_isRebelPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (factions.isRebel(player));
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_isNeutralPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return(!factions.isImperial(player) && !factions.isRebel(player));
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_isQualifiedRecruitmentCombatant(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (!imperial_empire_day_major_tantor_condition_isSpecialForcesImperial(player, npc) && !imperial_empire_day_major_tantor_condition_alreadyDoingRecruitment(player, npc));
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_isQualifiedPropagandaCombatant(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (!imperial_empire_day_major_tantor_condition_isSpecialForcesImperial(player, npc) && !imperial_empire_day_major_tantor_condition_alreadyDoingPropaganda(player, npc));
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_hasCompletedRecruitmentCollection(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasCompletedCollection(player, holiday.IMPERIAL_RECRUITING_COLLECTION);
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_hasCompletedPropagandaCollection(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasCompletedCollection(player, holiday.IMPERIAL_ANTIPROP_COLLECTION);
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_isLockedOutOfEvents(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (imperial_empire_day_major_tantor_condition_isLockedOutOfRecruitment(player, npc) && imperial_empire_day_major_tantor_condition_isLockedOutOfPropaganda(player, npc));
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_hasNotCompletedBothCollections(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (!imperial_empire_day_major_tantor_condition_hasCompletedPropagandaCollection(player, npc) && !imperial_empire_day_major_tantor_condition_hasCompletedRecruitmentCollection(player, npc));
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_alreadyDoingBothPropagandaRecruitment(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (imperial_empire_day_major_tantor_condition_alreadyDoingPropaganda(player, npc) && imperial_empire_day_major_tantor_condition_alreadyDoingRecruitment(player, npc));
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_isChampionDoesntKnowTantor(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (imperial_empire_day_major_tantor_condition_hasEmpireDayBadge(player, npc) && !imperial_empire_day_major_tantor_condition_alreadyKnowsTantor(player, npc));
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_isChampionKnowsTantor(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (imperial_empire_day_major_tantor_condition_hasEmpireDayBadge(player, npc) && imperial_empire_day_major_tantor_condition_alreadyKnowsTantor(player, npc));
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_hasCollectionsButNotCompleted(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (imperial_empire_day_major_tantor_condition_hasNotCompletedBothCollections(player, npc) && imperial_empire_day_major_tantor_condition_hasPropagandaCollectionPrerequisite(player, npc) && imperial_empire_day_major_tantor_condition_hasRecruitmentCollectionPrerequisite(player, npc));
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_isLockedOutOfPropaganda(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasObjVar(player, holiday.EMPIRE_DAY_PROPAGANDA_LOCKED_OUT);
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_isLockedOutOfRecruitment(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasObjVar(player, holiday.EMPIRE_DAY_RECRUITMENT_LOCKED_OUT);
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_canDoPropagandaEvent(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (!imperial_empire_day_major_tantor_condition_alreadyDoingPropaganda(player, npc) && !imperial_empire_day_major_tantor_condition_isLockedOutOfPropaganda(player, npc) && !imperial_empire_day_major_tantor_condition_hasCompletedPropagandaCollection(player, npc));
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_canDoRecruitmentEvent(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (!imperial_empire_day_major_tantor_condition_alreadyDoingRecruitment(player, npc) && !imperial_empire_day_major_tantor_condition_isLockedOutOfRecruitment(player, npc) && !imperial_empire_day_major_tantor_condition_hasCompletedRecruitmentCollection(player, npc));
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_canDoAtLeastOneEvent(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (imperial_empire_day_major_tantor_condition_canDoRecruitmentEvent(player, npc) || imperial_empire_day_major_tantor_condition_canDoPropagandaEvent(player, npc));
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_isTrader(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return utils.isProfession(player, utils.TRADER);
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_isEntertainer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return utils.isProfession(player, utils.ENTERTAINER);
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_isTraderOrEntertainer(obj_id player, obj_id npc) throws InterruptedException
	{
		return imperial_empire_day_major_tantor_condition_isTrader(player, npc) || imperial_empire_day_major_tantor_condition_isEntertainer(player, npc);
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_canCheckLeaderBoard(obj_id player, obj_id npc) throws InterruptedException
	{
		
		obj_id tatooine = getPlanetByName("tatooine");
		if (!isIdValid(tatooine) || !exists(tatooine))
		{
			CustomerServiceLog("holidayEvent", "tantor_conversation.getLeaderBoardSui: Could not find Tatooine OID.");
			return false;
		}
		
		if (!hasObjVar(player, holiday.PLAYER_EMPIRE_DAY_SCORE))
		{
			return false;
		}
		
		int playerScore = getIntObjVar(player, holiday.PLAYER_EMPIRE_DAY_SCORE);
		if (playerScore <= 0)
		{
			return false;
		}
		
		String playerName = getPlayerFullName(player);
		if (playerName == null || playerName.length() <= 0)
		{
			return false;
		}
		
		if (!holiday.setEmpireDayLeaderScores(tatooine, player, holiday.PLANET_VAR_EVENT_PREFIX + holiday.PLANET_VAR_EMPIRE_DAY + holiday.PLANET_VAR_SCORE, playerScore, playerName, holiday.IMPERIAL_PLAYER))
		{
			CustomerServiceLog("holidayEvent", "tantor_conversation.getLeaderBoardSui: Player: ("+player+") "+playerName+ " was unable to attain leader board data. Their score was: "+playerScore);
			return false;
		}
		return true;
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_hasScore(obj_id player, obj_id npc) throws InterruptedException
	{
		return hasObjVar(player, holiday.PLAYER_EMPIRE_DAY_SCORE);
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_isNeutralRebel(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasObjVar(player, holiday.PLAYER_EMPIRE_DAY_NEUTRAL + holiday.PLANET_VAR_PLAYER_FACTION_REB);
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_isNewNeutralPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (!hasObjVar(player, holiday.PLAYER_EMPIRE_DAY_NEUTRAL + holiday.PLANET_VAR_PLAYER_FACTION_IMP) && imperial_empire_day_major_tantor_condition_isNeutralPlayer(player, npc));
	}
	
	
	public boolean imperial_empire_day_major_tantor_condition_isNeutralRebelButImperial(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if ((hasObjVar(player, holiday.PLAYER_EMPIRE_DAY_NEUTRAL + holiday.PLANET_VAR_PLAYER_FACTION_REB) && factions.isImperial(player)))
		{
			return true;
		}
		return false;
	}
	
	
	public void imperial_empire_day_major_tantor_action_setSFRecruitmentEventOnPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		
		if (getCollectionSlotValue(player, holiday.IMPERIAL_RECRUITING_START_SLOT) <= 0)
		{
			imperial_empire_day_major_tantor_action_setRecruitmentCollectionStarterSlot(player, npc);
		}
		
		buff.applyBuff(player, holiday.BUFF_IMP_EMPIREDAY_RECRUITMENT_SF);
	}
	
	
	public void imperial_empire_day_major_tantor_action_resetPropagandaAndRecruitmentCollections(obj_id player, obj_id npc) throws InterruptedException
	{
		if (imperial_empire_day_major_tantor_condition_hasCompletedPropagandaCollection(player, npc) && imperial_empire_day_major_tantor_condition_hasCompletedRecruitmentCollection(player, npc))
		{
			
			String[] recruitingSlotsInCollection = getAllCollectionSlotsInCollection(holiday.IMPERIAL_RECRUITING_COLLECTION);
			
			for (int i = 0; i < recruitingSlotsInCollection.length; i++)
			{
				testAbortScript();
				
				long collectionSlotValue = getCollectionSlotValue(player, recruitingSlotsInCollection[i]) * -1;
				
				modifyCollectionSlotValue(player, recruitingSlotsInCollection[i], collectionSlotValue);
			}
			
			String[] slotsInCollection = getAllCollectionSlotsInCollection(holiday.IMPERIAL_ANTIPROP_COLLECTION);
			
			for (int i = 0; i < slotsInCollection.length; i++)
			{
				testAbortScript();
				
				long collectionSlotValue = getCollectionSlotValue(player, slotsInCollection[i]) * -1;
				
				modifyCollectionSlotValue(player, slotsInCollection[i], collectionSlotValue);
			}
			
			imperial_empire_day_major_tantor_action_setPropagandaCollectionStarterSlot(player, npc);
			imperial_empire_day_major_tantor_action_setRecruitmentCollectionStarterSlot(player, npc);
		}
	}
	
	
	public void imperial_empire_day_major_tantor_action_setRecruitmentTimeStampOnPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		holiday.setEventLockOutTimeStamp(player, holiday.EMPIRE_DAY_RECRUITMENT_TIMESTAMP);
	}
	
	
	public void imperial_empire_day_major_tantor_action_setSFPropagandaEventOnPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		
		if (getCollectionSlotValue(player, holiday.IMPERIAL_ANTIPROP_START_SLOT) <= 0)
		{
			imperial_empire_day_major_tantor_action_setPropagandaCollectionStarterSlot(player, npc);
		}
		
		buff.applyBuff(player, holiday.BUFF_IMP_EMPIREDAY_ANTIPROP_SF);
	}
	
	
	public void imperial_empire_day_major_tantor_action_giveMaraKaythreeWaypoint(obj_id player, obj_id npc) throws InterruptedException
	{
		location loc = new location(-5533, 6, 4372, "naboo");
		obj_id maraWpt = createWaypointInDatapad(player, loc);
		setWaypointName(maraWpt, "Unknown Imperial Parties");
		setWaypointActive(maraWpt, true);
	}
	
	
	public void imperial_empire_day_major_tantor_action_spawnTroopers(obj_id player, obj_id npc) throws InterruptedException
	{
		
		for (int i = 0; i < 3; i++)
		{
			testAbortScript();
			location guardLocation = groundquests.getRandom2DLocationAroundLocation(npc, 1, 1, 5, 12);
			int mobLevel = getLevel(player);
			obj_id guard = create.object("imperial_emperorsday_ceremony_sentry", guardLocation, mobLevel);
			attachScript(guard, "event.emp_day.factional_guard_self_destruct");
			startCombat(guard, player);
		}
	}
	
	
	public void imperial_empire_day_major_tantor_action_getLeaderBoardSui(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id tatooine = getPlanetByName("tatooine");
		if (!isIdValid(tatooine) || !exists(tatooine))
		{
			CustomerServiceLog("holidayEvent", "tantor_conversation.getLeaderBoardSui: Could not find Tatooine OID.");
			return;
		}
		
		String data = holiday.getEventHighScores(tatooine, holiday.PLANET_VAR_EVENT_PREFIX + holiday.PLANET_VAR_EMPIRE_DAY + holiday.PLANET_VAR_SCORE, true);
		if (data == null || data.length() <= 0)
		{
			CustomerServiceLog("holidayEvent", "tantor_conversation.getLeaderBoardSui: High Score Data Invalid.");
			return;
		}
		
		if (!holiday.createEventLeaderBoardUI(player, holiday.LEADER_BOARD_TITLE, data))
		{
			CustomerServiceLog("holidayEvent", "tantor_conversation.getLeaderBoardSui: createEventLeaderBoardUI failed.");
			return;
		}
		
		CustomerServiceLog("holidayEvent", "tantor_conversation.getLeaderBoardSui: High Score Data received by player: "+player+".");
	}
	
	
	public void imperial_empire_day_major_tantor_action_setRecruitmentCollectionStarterSlot(obj_id player, obj_id npc) throws InterruptedException
	{
		modifyCollectionSlotValue(player, holiday.IMPERIAL_RECRUITING_START_SLOT, 1);
	}
	
	
	public void imperial_empire_day_major_tantor_action_setPropagandaCollectionStarterSlot(obj_id player, obj_id npc) throws InterruptedException
	{
		modifyCollectionSlotValue(player, holiday.IMPERIAL_ANTIPROP_START_SLOT, 1);
	}
	
	
	public void imperial_empire_day_major_tantor_action_setCombatantRecruitmentEventOnPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		
		if (getCollectionSlotValue(player, holiday.IMPERIAL_RECRUITING_START_SLOT) <= 0)
		{
			imperial_empire_day_major_tantor_action_setRecruitmentCollectionStarterSlot(player, npc);
		}
		
		buff.applyBuff(player, holiday.BUFF_IMP_EMPIREDAY_RECRUITMENT_COMBATANT);
	}
	
	
	public void imperial_empire_day_major_tantor_action_setCombatantPropagandaEventOnPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		
		if (getCollectionSlotValue(player, holiday.IMPERIAL_ANTIPROP_START_SLOT) <= 0)
		{
			imperial_empire_day_major_tantor_action_setPropagandaCollectionStarterSlot(player, npc);
		}
		
		buff.applyBuff(player, holiday.BUFF_IMP_EMPIREDAY_ANTIPROP_COMBATANT);
	}
	
	
	public void imperial_empire_day_major_tantor_action_setPropagandaTimeStampOnPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		holiday.setEventLockOutTimeStamp(player, holiday.EMPIRE_DAY_PROPAGANDA_TIMESTAMP);
	}
	
	
	public void imperial_empire_day_major_tantor_action_checkHolidayTimeStampOnPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		holiday.checkEventLockOutRemoval(player, holiday.EMPIRE_DAY_RECRUITMENT_TIMESTAMP, holiday.EMPIRE_DAY_RECRUITMENT_LOCKED_OUT);
		holiday.checkEventLockOutRemoval(player, holiday.EMPIRE_DAY_PROPAGANDA_TIMESTAMP, holiday.EMPIRE_DAY_PROPAGANDA_LOCKED_OUT);
	}
	
	
	public void imperial_empire_day_major_tantor_action_setLeaderBoardScore(obj_id player, obj_id npc) throws InterruptedException
	{
		
		if (!hasObjVar(player, holiday.PLAYER_EMPIRE_DAY_SCORE))
		{
			return;
		}
		
		int playerScore = getIntObjVar(player, holiday.PLAYER_EMPIRE_DAY_SCORE);
		if (playerScore <= 0)
		{
			return;
		}
		
		String playerName = getPlayerFullName(player);
		if (playerName == null || playerName.length() <= 0)
		{
			return;
		}
		
		obj_id tatooine = getPlanetByName("tatooine");
		if (!isIdValid(tatooine) || !exists(tatooine))
		{
			CustomerServiceLog("holidayEvent", "tantor_conversation.getLeaderBoardSui: Could not find Tatooine OID.");
			return;
		}
		
		if (!holiday.setEmpireDayLeaderScores(tatooine, player, holiday.PLANET_VAR_EVENT_PREFIX + holiday.PLANET_VAR_EMPIRE_DAY + holiday.PLANET_VAR_SCORE, playerScore, playerName, holiday.IMPERIAL_PLAYER))
		{
			CustomerServiceLog("holidayEvent", "tantor_conversation.getLeaderBoardSui: Player: ("+player+") "+playerName+ " was unable to attain leader board data. Their score was: "+playerScore);
			return;
		}
		
	}
	
	
	public void imperial_empire_day_major_tantor_action_makeNeutralImperial(obj_id player, obj_id npc) throws InterruptedException
	{
		play2dNonLoopingSound(player, space_quest.MUSIC_QUEST_EVENT);
		setObjVar(player, holiday.PLAYER_EMPIRE_DAY_NEUTRAL + holiday.PLANET_VAR_PLAYER_FACTION_IMP, true);
	}
	
	
	public void imperial_empire_day_major_tantor_action_removeNeutralRebel(obj_id player, obj_id npc) throws InterruptedException
	{
		removeObjVar(player, holiday.PLAYER_EMPIRE_DAY_NEUTRAL + holiday.PLANET_VAR_PLAYER_FACTION_REB);
	}
	
	
	public String imperial_empire_day_major_tantor_tokenTO_timeLeftRecruitment(obj_id player, obj_id npc) throws InterruptedException
	{
		String returnString = holiday.getTimeRemainingBeforeLockoutRemoved(player, holiday.EMPIRE_DAY_RECRUITMENT_TIMESTAMP);
		if (returnString == null || returnString.equals(""))
		{
			return "might have to contact Customer Service. I can't seem to get any time data off your character.";
		}
		return returnString;
	}
	
	
	public String imperial_empire_day_major_tantor_tokenTO_timeLeftPropaganda(obj_id player, obj_id npc) throws InterruptedException
	{
		String returnString = holiday.getTimeRemainingBeforeLockoutRemoved(player, holiday.EMPIRE_DAY_PROPAGANDA_TIMESTAMP);
		if (returnString == null || returnString.equals(""))
		{
			return "might have to contact Customer Service. I can't seem to get any time data off your character.";
		}
		return returnString;
	}
	
	
	public String imperial_empire_day_major_tantor_tokenTO_currentScore(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasObjVar(player, holiday.PLAYER_EMPIRE_DAY_SCORE))
		{
			return "no points so far.";
		}
		
		int playerScore = getIntObjVar(player, holiday.PLAYER_EMPIRE_DAY_SCORE);
		if (playerScore <= 0)
		{
			return "no points so far.";
		}
		
		return playerScore+" points so far.";
	}
	
	
	public int imperial_empire_day_major_tantor_tokenDI_notUsed(obj_id player, obj_id npc) throws InterruptedException
	{
		return 0;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_173"))
		{
			
			if (imperial_empire_day_major_tantor_condition_hasCompletedBothCollections (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_212");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_214");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 6);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_major_tantor_condition_isLockedOutOfEvents (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_175");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_64");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_57");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 10);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (!imperial_empire_day_major_tantor_condition_canDoAtLeastOneEvent (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_56");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_major_tantor_condition_canDoAtLeastOneEvent (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_176");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition_canDoRecruitmentEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_major_tantor_condition_canDoPropagandaEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (imperial_empire_day_major_tantor_condition_isTrader (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (imperial_empire_day_major_tantor_condition_isEntertainer (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				boolean hasResponse5 = false;
				if (!imperial_empire_day_major_tantor_condition_hasEmpireDayBadge (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse5 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_250");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_276");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_156");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_157");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_301");
					}
					
					if (hasResponse5)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_146");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 31);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_248");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_121"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_123");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_125");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 19);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_122"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_124");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_126");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 21);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_108"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_110");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				pp.other.set (imperial_empire_day_major_tantor_tokenTO_currentScore (player, npc));
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_190"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_major_tantor_action_getLeaderBoardSui (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_192");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_173"))
		{
			
			if (imperial_empire_day_major_tantor_condition_hasCompletedBothCollections (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_212");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_214");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 6);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_major_tantor_condition_isLockedOutOfEvents (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_175");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_64");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_57");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 10);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (!imperial_empire_day_major_tantor_condition_canDoAtLeastOneEvent (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_56");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_major_tantor_condition_canDoAtLeastOneEvent (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_176");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition_canDoRecruitmentEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_major_tantor_condition_canDoPropagandaEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (imperial_empire_day_major_tantor_condition_isTrader (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (imperial_empire_day_major_tantor_condition_isEntertainer (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				boolean hasResponse5 = false;
				if (!imperial_empire_day_major_tantor_condition_hasEmpireDayBadge (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse5 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_250");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_276");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_156");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_157");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_301");
					}
					
					if (hasResponse5)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_146");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 31);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_248");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_121"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_123");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_125");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 19);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_122"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_124");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_126");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 21);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_108"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_110");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				pp.other.set (imperial_empire_day_major_tantor_tokenTO_currentScore (player, npc));
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_190"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_major_tantor_action_getLeaderBoardSui (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_192");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_214"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_216");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition_hasCompletedRecruitmentCollection (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_218");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 7);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_218"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_220");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_222");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_222"))
		{
			
			if (imperial_empire_day_major_tantor_condition_hasCompletedBothCollections (player, npc))
			{
				imperial_empire_day_major_tantor_action_resetPropagandaAndRecruitmentCollections (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_224");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition_canDoRecruitmentEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_major_tantor_condition_canDoPropagandaEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (imperial_empire_day_major_tantor_condition_isTrader (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (imperial_empire_day_major_tantor_condition_isEntertainer (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				boolean hasResponse5 = false;
				if (!imperial_empire_day_major_tantor_condition_hasEmpireDayBadge (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse5 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_250");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_276");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_156");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_157");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_301");
					}
					
					if (hasResponse5)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_146");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 31);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_250"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_252");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_254");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 32);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_276"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_278");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_280");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 40);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_156"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_158");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_162");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 48);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_157"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_160");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_166");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 50);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_301"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_major_tantor_action_getLeaderBoardSui (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_302");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_146"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_150");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_64"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_90");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_91");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_92");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 11);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_57"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_59");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_60");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 14);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch11(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_91"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_95");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				pp.other.set (imperial_empire_day_major_tantor_tokenTO_timeLeftRecruitment (player, npc));
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_92"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_96");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				pp.other.set (imperial_empire_day_major_tantor_tokenTO_timeLeftPropaganda (player, npc));
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch14(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_60"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_major_tantor_action_giveMaraKaythreeWaypoint (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_61");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch17(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_250"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_252");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_254");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 32);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_276"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_278");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_280");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 40);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_156"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_158");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_162");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 48);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_157"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_160");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_166");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 50);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_301"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_major_tantor_action_getLeaderBoardSui (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_302");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_146"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_150");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch19(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_125"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_major_tantor_action_giveMaraKaythreeWaypoint (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_127");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch21(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_126"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_major_tantor_action_giveMaraKaythreeWaypoint (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_128");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch25(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_69"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_72");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_76");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 30);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch26(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_103"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_104");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_105");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 27);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch27(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_105"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_106");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_107");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 28);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch28(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_107"))
		{
			doAnimationAction (player, "salute1");
			
			imperial_empire_day_major_tantor_action_makeNeutralImperial (player, npc);
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_72");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_76");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 30);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch29(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_69"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_72");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_76");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 30);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch30(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_76"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "pound_fist_palm");
				
				string_id message = new string_id (c_stringFile, "s_80");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition_canDoRecruitmentEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_major_tantor_condition_canDoPropagandaEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (imperial_empire_day_major_tantor_condition_isTrader (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (imperial_empire_day_major_tantor_condition_isEntertainer (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				boolean hasResponse5 = false;
				if (!imperial_empire_day_major_tantor_condition_hasEmpireDayBadge (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse5 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_250");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_276");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_156");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_157");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_301");
					}
					
					if (hasResponse5)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_146");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 31);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch31(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_250"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_252");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_254");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 32);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_276"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_278");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_280");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 40);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_156"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_158");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_162");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 48);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_157"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_160");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_166");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 50);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_301"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_major_tantor_action_getLeaderBoardSui (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_302");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_146"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_150");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch32(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_254"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_256");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_258");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 33);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch33(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_258"))
		{
			
			if (imperial_empire_day_major_tantor_condition_isQualifiedRecruitmentCombatant (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_260");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_262");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 34);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_major_tantor_condition_isQualifiedRecruitmentSpecialForces (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_268");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_270");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 37);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch34(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_262"))
		{
			
			if (imperial_empire_day_major_tantor_condition_canDoAtLeastOneEvent (player, npc))
			{
				imperial_empire_day_major_tantor_action_setCombatantRecruitmentEventOnPlayer (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_264");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition_canDoRecruitmentEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_major_tantor_condition_canDoPropagandaEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (imperial_empire_day_major_tantor_condition_isTrader (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (imperial_empire_day_major_tantor_condition_isEntertainer (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				boolean hasResponse5 = false;
				if (!imperial_empire_day_major_tantor_condition_hasEmpireDayBadge (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse5 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_250");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_276");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_156");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_157");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_301");
					}
					
					if (hasResponse5)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_146");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 31);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (!imperial_empire_day_major_tantor_condition_canDoAtLeastOneEvent (player, npc))
			{
				imperial_empire_day_major_tantor_action_setCombatantRecruitmentEventOnPlayer (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_266");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch35(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_250"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_252");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_254");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 32);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_276"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_278");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_280");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 40);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_156"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_158");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_162");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 48);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_157"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_160");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_166");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 50);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_301"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_major_tantor_action_getLeaderBoardSui (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_302");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_146"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_150");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch37(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_270"))
		{
			
			if (imperial_empire_day_major_tantor_condition_canDoAtLeastOneEvent (player, npc))
			{
				imperial_empire_day_major_tantor_action_setSFRecruitmentEventOnPlayer (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_272");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition_canDoRecruitmentEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_major_tantor_condition_canDoPropagandaEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (imperial_empire_day_major_tantor_condition_isTrader (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (imperial_empire_day_major_tantor_condition_isEntertainer (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				boolean hasResponse5 = false;
				if (!imperial_empire_day_major_tantor_condition_hasEmpireDayBadge (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse5 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_250");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_276");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_156");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_157");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_301");
					}
					
					if (hasResponse5)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_146");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 31);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (!imperial_empire_day_major_tantor_condition_canDoAtLeastOneEvent (player, npc))
			{
				imperial_empire_day_major_tantor_action_setSFRecruitmentEventOnPlayer (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_274");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch38(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_250"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_252");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_254");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 32);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_276"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_278");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_280");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 40);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_156"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_158");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_162");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 48);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_157"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_160");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_166");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 50);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_301"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_major_tantor_action_getLeaderBoardSui (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_302");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_146"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_150");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch40(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_280"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_282");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_284");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 41);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch41(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_284"))
		{
			
			if (imperial_empire_day_major_tantor_condition_isQualifiedPropagandaCombatant (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_286");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_288");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 42);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_major_tantor_condition_isQualifiedPropagandaSpecialForces (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_294");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_296");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 45);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch42(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_288"))
		{
			
			if (imperial_empire_day_major_tantor_condition_canDoAtLeastOneEvent (player, npc))
			{
				imperial_empire_day_major_tantor_action_setCombatantPropagandaEventOnPlayer (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_290");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition_canDoRecruitmentEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_major_tantor_condition_canDoPropagandaEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (imperial_empire_day_major_tantor_condition_isTrader (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (imperial_empire_day_major_tantor_condition_isEntertainer (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				boolean hasResponse5 = false;
				if (!imperial_empire_day_major_tantor_condition_hasEmpireDayBadge (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse5 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_250");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_276");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_156");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_157");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_301");
					}
					
					if (hasResponse5)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_146");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 31);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (!imperial_empire_day_major_tantor_condition_canDoAtLeastOneEvent (player, npc))
			{
				imperial_empire_day_major_tantor_action_setCombatantPropagandaEventOnPlayer (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_292");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch43(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_250"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_252");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_254");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 32);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_276"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_278");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_280");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 40);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_156"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_158");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_162");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 48);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_157"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_160");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_166");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 50);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_301"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_major_tantor_action_getLeaderBoardSui (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_302");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_146"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_150");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch45(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_296"))
		{
			
			if (imperial_empire_day_major_tantor_condition_canDoAtLeastOneEvent (player, npc))
			{
				imperial_empire_day_major_tantor_action_setSFPropagandaEventOnPlayer (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_298");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition_canDoRecruitmentEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_major_tantor_condition_canDoPropagandaEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (imperial_empire_day_major_tantor_condition_isTrader (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (imperial_empire_day_major_tantor_condition_isEntertainer (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				boolean hasResponse5 = false;
				if (!imperial_empire_day_major_tantor_condition_hasEmpireDayBadge (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse5 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_250");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_276");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_156");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_157");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_301");
					}
					
					if (hasResponse5)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_146");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 31);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (!imperial_empire_day_major_tantor_condition_canDoAtLeastOneEvent (player, npc))
			{
				imperial_empire_day_major_tantor_action_setSFPropagandaEventOnPlayer (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_300");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch46(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_250"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_252");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_254");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 32);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_276"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_278");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_280");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 40);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_156"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_158");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_162");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 48);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_157"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_160");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_166");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 50);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_301"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_major_tantor_action_getLeaderBoardSui (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_302");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_146"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_150");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch48(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_162"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_major_tantor_action_giveMaraKaythreeWaypoint (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_164");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_major_tantor_handleBranch50(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_166"))
		{
			
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_major_tantor_action_giveMaraKaythreeWaypoint (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_174");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		if ((!isTangible (self)) || (isPlayer (self)))
		{
			detachScript(self, "conversation.imperial_empire_day_major_tantor");
		}
		
		setCondition (self, CONDITION_CONVERSABLE);
		setCondition (self, CONDITION_INTERESTING);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		
		setCondition (self, CONDITION_CONVERSABLE);
		setCondition (self, CONDITION_INTERESTING);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info menuInfo) throws InterruptedException
	{
		int menu = menuInfo.addRootMenu (menu_info_types.CONVERSE_START, null);
		menu_info_data menuInfoData = menuInfo.getMenuItemById (menu);
		menuInfoData.setServerNotify (false);
		setCondition (self, CONDITION_CONVERSABLE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnIncapacitated(obj_id self, obj_id killer) throws InterruptedException
	{
		clearCondition (self, CONDITION_CONVERSABLE);
		detachScript (self, "conversation.imperial_empire_day_major_tantor");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDetach(obj_id self) throws InterruptedException
	{
		clearCondition (self, CONDITION_CONVERSABLE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean npcStartConversation(obj_id player, obj_id npc, String convoName, string_id greetingId, prose_package greetingProse, string_id[] responses) throws InterruptedException
	{
		Object[] objects = new Object[responses.length];
		System.arraycopy(responses, 0, objects, 0, responses.length);
		return npcStartConversation(player, npc, convoName, greetingId, greetingProse, objects);
	}
	
	
	public int OnStartNpcConversation(obj_id self, obj_id player) throws InterruptedException
	{
		obj_id npc = self;
		
		if (ai_lib.isInCombat (npc) || ai_lib.isInCombat (player))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (imperial_empire_day_major_tantor_condition_isRebelPlayer (player, npc))
		{
			doAnimationAction (npc, "backhand_threaten");
			
			string_id message = new string_id (c_stringFile, "s_50");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (imperial_empire_day_major_tantor_condition_isNeutralRebelButImperial (player, npc))
		{
			imperial_empire_day_major_tantor_action_removeNeutralRebel (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_112");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (imperial_empire_day_major_tantor_condition_isNeutralRebel (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_111");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (imperial_empire_day_major_tantor_condition_isChampionKnowsTantor (player, npc))
		{
			imperial_empire_day_major_tantor_action_checkHolidayTimeStampOnPlayer (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_67");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (imperial_empire_day_major_tantor_condition_isTrader (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse1 = true;
			}
			
			boolean hasResponse2 = false;
			if (imperial_empire_day_major_tantor_condition_isEntertainer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse2 = true;
			}
			
			boolean hasResponse3 = false;
			if (imperial_empire_day_major_tantor_condition_hasScore (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse3 = true;
			}
			
			boolean hasResponse4 = false;
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse4 = true;
			}
			
			if (hasResponse)
			{
				int responseIndex = 0;
				string_id responses[] = new string_id[numberOfResponses];
				
				if (hasResponse0)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_173");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_121");
				}
				
				if (hasResponse2)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_122");
				}
				
				if (hasResponse3)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_108");
				}
				
				if (hasResponse4)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_190");
				}
				
				utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 5);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "imperial_empire_day_major_tantor", null, pp, responses);
			}
			else
			{
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				chat.chat (npc, player, null, null, pp);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (imperial_empire_day_major_tantor_condition_alreadyKnowsTantor (player, npc))
		{
			imperial_empire_day_major_tantor_action_checkHolidayTimeStampOnPlayer (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_116");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (imperial_empire_day_major_tantor_condition_isTrader (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse1 = true;
			}
			
			boolean hasResponse2 = false;
			if (imperial_empire_day_major_tantor_condition_isEntertainer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse2 = true;
			}
			
			boolean hasResponse3 = false;
			if (imperial_empire_day_major_tantor_condition_hasScore (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse3 = true;
			}
			
			boolean hasResponse4 = false;
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse4 = true;
			}
			
			if (hasResponse)
			{
				int responseIndex = 0;
				string_id responses[] = new string_id[numberOfResponses];
				
				if (hasResponse0)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_173");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_121");
				}
				
				if (hasResponse2)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_122");
				}
				
				if (hasResponse3)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_108");
				}
				
				if (hasResponse4)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_190");
				}
				
				utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 5);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "imperial_empire_day_major_tantor", null, pp, responses);
			}
			else
			{
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				chat.chat (npc, player, null, null, pp);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (imperial_empire_day_major_tantor_condition_isChampionDoesntKnowTantor (player, npc))
		{
			doAnimationAction (npc, "salute2");
			
			doAnimationAction (player, "salute2");
			
			string_id message = new string_id (c_stringFile, "s_109");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			if (hasResponse)
			{
				int responseIndex = 0;
				string_id responses[] = new string_id[numberOfResponses];
				
				if (hasResponse0)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_69");
				}
				
				utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 29);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "imperial_empire_day_major_tantor", null, pp, responses);
			}
			else
			{
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				chat.chat (npc, player, null, null, pp);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (imperial_empire_day_major_tantor_condition_isNewNeutralPlayer (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_102");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			if (hasResponse)
			{
				int responseIndex = 0;
				string_id responses[] = new string_id[numberOfResponses];
				
				if (hasResponse0)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_103");
				}
				
				utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 26);
				
				npcStartConversation (player, npc, "imperial_empire_day_major_tantor", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
		{
			doAnimationAction (npc, "salute2");
			
			doAnimationAction (player, "salute2");
			
			string_id message = new string_id (c_stringFile, "s_58");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (imperial_empire_day_major_tantor_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			if (hasResponse)
			{
				int responseIndex = 0;
				string_id responses[] = new string_id[numberOfResponses];
				
				if (hasResponse0)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_69");
				}
				
				utils.setScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId", 29);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "imperial_empire_day_major_tantor", null, pp, responses);
			}
			else
			{
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				chat.chat (npc, player, null, null, pp);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("imperial_empire_day_major_tantor"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
		
		if (branchId == 4 && imperial_empire_day_major_tantor_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && imperial_empire_day_major_tantor_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && imperial_empire_day_major_tantor_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && imperial_empire_day_major_tantor_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && imperial_empire_day_major_tantor_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && imperial_empire_day_major_tantor_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && imperial_empire_day_major_tantor_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 11 && imperial_empire_day_major_tantor_handleBranch11 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 14 && imperial_empire_day_major_tantor_handleBranch14 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 17 && imperial_empire_day_major_tantor_handleBranch17 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 19 && imperial_empire_day_major_tantor_handleBranch19 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 21 && imperial_empire_day_major_tantor_handleBranch21 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 25 && imperial_empire_day_major_tantor_handleBranch25 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 26 && imperial_empire_day_major_tantor_handleBranch26 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 27 && imperial_empire_day_major_tantor_handleBranch27 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 28 && imperial_empire_day_major_tantor_handleBranch28 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 29 && imperial_empire_day_major_tantor_handleBranch29 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 30 && imperial_empire_day_major_tantor_handleBranch30 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 31 && imperial_empire_day_major_tantor_handleBranch31 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 32 && imperial_empire_day_major_tantor_handleBranch32 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 33 && imperial_empire_day_major_tantor_handleBranch33 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 34 && imperial_empire_day_major_tantor_handleBranch34 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 35 && imperial_empire_day_major_tantor_handleBranch35 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 37 && imperial_empire_day_major_tantor_handleBranch37 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 38 && imperial_empire_day_major_tantor_handleBranch38 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 40 && imperial_empire_day_major_tantor_handleBranch40 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 41 && imperial_empire_day_major_tantor_handleBranch41 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 42 && imperial_empire_day_major_tantor_handleBranch42 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 43 && imperial_empire_day_major_tantor_handleBranch43 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 45 && imperial_empire_day_major_tantor_handleBranch45 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 46 && imperial_empire_day_major_tantor_handleBranch46 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 48 && imperial_empire_day_major_tantor_handleBranch48 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 50 && imperial_empire_day_major_tantor_handleBranch50 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.imperial_empire_day_major_tantor.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
