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


public class rebel_remembrance_day_captain_derlin extends script.base_script
{
	public rebel_remembrance_day_captain_derlin()
	{
	}
	String c_stringFile = "conversation/rebel_remembrance_day_captain_derlin";
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_alreadyDoingResistance(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return buff.hasBuff(player, holiday.BUFF_REB_EMPIREDAY_RESISTANCE_COMBATANT) || buff.hasBuff(player, holiday.BUFF_REB_EMPIREDAY_RESISTANCE_SF);
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_hasCompletedOneOrMoreCollections(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (rebel_remembrance_day_captain_derlin_condition_hasCompletedVandalCollection(player, npc) || rebel_remembrance_day_captain_derlin_condition_hasCompletedResistanceCollection(player, npc));
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_isSpecialForcesRebel(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (factions.isDeclared(player) && factions.isRebel(player));
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_hasResistanceCollectionPrerequisite(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasCompletedCollectionSlotPrereq(player, holiday.REBEL_RESISTANCE_COUNTER_SLOT);
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_alreadyDoingVandal(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return buff.hasBuff(player, holiday.BUFF_REB_EMPIREDAY_VANDAL_COMBATANT) || buff.hasBuff(player, holiday.BUFF_REB_EMPIREDAY_VANDAL_SF);
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_alreadyKnowsDerlin(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (rebel_remembrance_day_captain_derlin_condition_hasResistanceCollectionPrerequisite(player, npc) || rebel_remembrance_day_captain_derlin_condition_hasVandalCollectionPrerequisite(player, npc));
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_hasVandalCollectionPrerequisite(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasCompletedCollectionSlotPrereq(player, holiday.REBEL_VANDAL_COUNTER_SLOT);
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_hasRemembranceDayBadge(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasCompletedCollectionSlot(player, holiday.REMEMBRANCE_DAY_CHAMPION_BADGE);
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_isQualifiedResistanceSpecialForces(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (rebel_remembrance_day_captain_derlin_condition_isSpecialForcesRebel(player, npc) && !rebel_remembrance_day_captain_derlin_condition_alreadyDoingResistance(player, npc));
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_isQualifiedVandalSpecialForces(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (rebel_remembrance_day_captain_derlin_condition_isSpecialForcesRebel(player, npc) && !rebel_remembrance_day_captain_derlin_condition_alreadyDoingVandal(player, npc));
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_isImperialPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (factions.isImperial(player));
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_isNeutralPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return(!factions.isImperial(player) && !factions.isRebel(player));
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_isQualifiedResistanceCombatant(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (!rebel_remembrance_day_captain_derlin_condition_isSpecialForcesRebel(player, npc) && !rebel_remembrance_day_captain_derlin_condition_alreadyDoingResistance(player, npc));
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_isQualifiedVandalCombatant(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (!rebel_remembrance_day_captain_derlin_condition_isSpecialForcesRebel(player, npc) && !rebel_remembrance_day_captain_derlin_condition_alreadyDoingVandal(player, npc));
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_hasCompletedResistanceCollection(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasCompletedCollection(player, holiday.REBEL_RESISTANCE_COLLECTION);
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_hasCompletedVandalCollection(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasCompletedCollection(player, holiday.REBEL_VANDAL_COLLECTION);
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_isLockedOutOfEvents(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (rebel_remembrance_day_captain_derlin_condition_isLockedOutOfResistance(player, npc) && rebel_remembrance_day_captain_derlin_condition_isLockedOutOfVandal(player, npc));
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_hasNotCompletedBothCollections(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (!rebel_remembrance_day_captain_derlin_condition_hasCompletedVandalCollection(player, npc) && !rebel_remembrance_day_captain_derlin_condition_hasCompletedResistanceCollection(player, npc));
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_alreadyDoingBothVandalResistance(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (rebel_remembrance_day_captain_derlin_condition_alreadyDoingResistance(player, npc) && rebel_remembrance_day_captain_derlin_condition_alreadyDoingVandal(player, npc));
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_isChampionDoesntKnowDerlin(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (rebel_remembrance_day_captain_derlin_condition_hasRemembranceDayBadge(player, npc) && !rebel_remembrance_day_captain_derlin_condition_alreadyKnowsDerlin(player, npc));
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_isChampionKnowsDerlin(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (rebel_remembrance_day_captain_derlin_condition_hasRemembranceDayBadge(player, npc) && rebel_remembrance_day_captain_derlin_condition_alreadyKnowsDerlin(player, npc));
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_hasCollectionsButNotCompleted(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if (rebel_remembrance_day_captain_derlin_condition_hasVandalCollectionPrerequisite(player, npc) && rebel_remembrance_day_captain_derlin_condition_hasResistanceCollectionPrerequisite(player, npc) && rebel_remembrance_day_captain_derlin_condition_hasNotCompletedBothCollections(player, npc))
		{
			if (!rebel_remembrance_day_captain_derlin_condition_alreadyDoingVandal(player, npc) || !rebel_remembrance_day_captain_derlin_condition_alreadyDoingResistance(player, npc))
			{
				return false;
			}
		}
		return true;
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_isLockedOutOfVandal(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasObjVar(player, holiday.EMPIRE_DAY_VANDAL_LOCKED_OUT);
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_isLockedOutOfResistance(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasObjVar(player, holiday.EMPIRE_DAY_RESISTANCE_LOCKED_OUT);
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_canDoVandalEvent(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (!rebel_remembrance_day_captain_derlin_condition_alreadyDoingVandal(player, npc) && !rebel_remembrance_day_captain_derlin_condition_isLockedOutOfVandal(player, npc));
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_canDoResistanceEvent(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (!rebel_remembrance_day_captain_derlin_condition_alreadyDoingResistance(player, npc) && !rebel_remembrance_day_captain_derlin_condition_isLockedOutOfResistance(player, npc));
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_canDoAtLeastOneEvent(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (rebel_remembrance_day_captain_derlin_condition_canDoResistanceEvent(player, npc) || rebel_remembrance_day_captain_derlin_condition_canDoVandalEvent(player, npc));
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_isTrader(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return utils.isProfession(player, utils.TRADER);
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_isEntertainer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return utils.isProfession(player, utils.ENTERTAINER);
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_isTraderOrEntertainer(obj_id player, obj_id npc) throws InterruptedException
	{
		return rebel_remembrance_day_captain_derlin_condition_isTrader(player, npc) || rebel_remembrance_day_captain_derlin_condition_isEntertainer(player, npc);
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_hasCompletedBothCollections(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (rebel_remembrance_day_captain_derlin_condition_hasCompletedVandalCollection(player, npc) && rebel_remembrance_day_captain_derlin_condition_hasCompletedResistanceCollection(player, npc));
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_hasScore(obj_id player, obj_id npc) throws InterruptedException
	{
		return hasObjVar(player, holiday.PLAYER_EMPIRE_DAY_SCORE);
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_isNeutralImperial(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasObjVar(player, holiday.PLAYER_EMPIRE_DAY_NEUTRAL + holiday.PLANET_VAR_PLAYER_FACTION_IMP);
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_isNewNeutralPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (!hasObjVar(player, holiday.PLAYER_EMPIRE_DAY_NEUTRAL + holiday.PLANET_VAR_PLAYER_FACTION_REB) && rebel_remembrance_day_captain_derlin_condition_isNeutralPlayer(player, npc));
	}
	
	
	public boolean rebel_remembrance_day_captain_derlin_condition_isNeutralImperialButRebel(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if ((hasObjVar(player, holiday.PLAYER_EMPIRE_DAY_NEUTRAL + holiday.PLANET_VAR_PLAYER_FACTION_IMP) && factions.isRebel(player)))
		{
			return true;
		}
		return false;
	}
	
	
	public void rebel_remembrance_day_captain_derlin_action_setSFResistanceEventOnPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		
		if (getCollectionSlotValue(player, holiday.REBEL_RESISTANCE_START_SLOT) <= 0)
		{
			rebel_remembrance_day_captain_derlin_action_setResistanceCollectionStarterSlot(player, npc);
		}
		
		buff.applyBuff(player, holiday.BUFF_REB_EMPIREDAY_RESISTANCE_SF);
	}
	
	
	public void rebel_remembrance_day_captain_derlin_action_resetBothCollections(obj_id player, obj_id npc) throws InterruptedException
	{
		if (rebel_remembrance_day_captain_derlin_condition_hasCompletedResistanceCollection(player, npc) && rebel_remembrance_day_captain_derlin_condition_hasCompletedVandalCollection(player, npc))
		{
			
			String[] resistanceSlotsInCollection = getAllCollectionSlotsInCollection(holiday.REBEL_RESISTANCE_COLLECTION);
			
			for (int i = 0; i < resistanceSlotsInCollection.length; i++)
			{
				testAbortScript();
				
				long collectionSlotValue = getCollectionSlotValue(player, resistanceSlotsInCollection[i]) * -1;
				
				modifyCollectionSlotValue(player, resistanceSlotsInCollection[i], collectionSlotValue);
			}
			
			String[] slotsInCollection = getAllCollectionSlotsInCollection(holiday.REBEL_VANDAL_COLLECTION);
			
			for (int i = 0; i < slotsInCollection.length; i++)
			{
				testAbortScript();
				
				long collectionSlotValue = getCollectionSlotValue(player, slotsInCollection[i]) * -1;
				
				modifyCollectionSlotValue(player, slotsInCollection[i], collectionSlotValue);
			}
			
			rebel_remembrance_day_captain_derlin_action_setResistanceCollectionStarterSlot(player, npc);
			rebel_remembrance_day_captain_derlin_action_setVandalCollectionStarterSlot(player, npc);
		}
	}
	
	
	public void rebel_remembrance_day_captain_derlin_action_getLeaderBoardSui(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id tatooine = getPlanetByName("tatooine");
		if (!isIdValid(tatooine) || !exists(tatooine))
		{
			CustomerServiceLog("holidayEvent", "derlin_conversation.getLeaderBoardSui: Could not find Tatooine OID.");
			return;
		}
		
		String data = holiday.getEventHighScores(tatooine, holiday.PLANET_VAR_EVENT_PREFIX + holiday.PLANET_VAR_EMPIRE_DAY + holiday.PLANET_VAR_SCORE, true);
		if (data == null || data.length() <= 0)
		{
			CustomerServiceLog("holidayEvent", "derlin_conversation.getLeaderBoardSui: High Score Data Invalid.");
			return;
		}
		
		if (!holiday.createEventLeaderBoardUI(player, holiday.LEADER_BOARD_TITLE, data))
		{
			CustomerServiceLog("holidayEvent", "derlin_conversation.getLeaderBoardSui: createEventLeaderBoardUI failed.");
			return;
		}
		
		CustomerServiceLog("holidayEvent", "derlin_conversation.getLeaderBoardSui: High Score Data received by player: "+player+".");
	}
	
	
	public void rebel_remembrance_day_captain_derlin_action_setSFVandalEventOnPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		
		if (getCollectionSlotValue(player, holiday.REBEL_VANDAL_START_SLOT) <= 0)
		{
			rebel_remembrance_day_captain_derlin_action_setVandalCollectionStarterSlot(player, npc);
		}
		
		buff.applyBuff(player, holiday.BUFF_REB_EMPIREDAY_VANDAL_SF);
	}
	
	
	public void rebel_remembrance_day_captain_derlin_action_giveWedgeAntillesWaypoint(obj_id player, obj_id npc) throws InterruptedException
	{
		location loc = new location(-247, 28, 4151, "corellia");
		obj_id wedgeWpt = createWaypointInDatapad(player, loc);
		setWaypointName(wedgeWpt, "Squadron Representative");
		setWaypointActive(wedgeWpt, true);
	}
	
	
	public void rebel_remembrance_day_captain_derlin_action_spawnTroopers(obj_id player, obj_id npc) throws InterruptedException
	{
		
		for (int i = 0; i < 3; i++)
		{
			testAbortScript();
			location guardLocation = groundquests.getRandom2DLocationAroundLocation(npc, 1, 1, 5, 12);
			int mobLevel = getLevel(player);
			obj_id guard = create.object("rebel_emperorsday_ceremony_sentry", guardLocation, mobLevel);
			attachScript(guard, "event.emp_day.factional_guard_self_destruct");
			startCombat(guard, player);
		}
	}
	
	
	public void rebel_remembrance_day_captain_derlin_action_setLeaderBoardScore(obj_id player, obj_id npc) throws InterruptedException
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
			CustomerServiceLog("holidayEvent", "derlin_conversation.getLeaderBoardSui: Could not find Tatooine OID.");
			return;
		}
		
		if (!holiday.setEmpireDayLeaderScores(tatooine, player, holiday.PLANET_VAR_EVENT_PREFIX + holiday.PLANET_VAR_EMPIRE_DAY + holiday.PLANET_VAR_SCORE, playerScore, playerName, holiday.REBEL_PLAYER))
		{
			CustomerServiceLog("holidayEvent", "derlin_conversation.getLeaderBoardSui: Player: ("+player+") "+playerName+ " was unable to attain leader board data. Their score was: "+playerScore);
			return;
		}
		
	}
	
	
	public void rebel_remembrance_day_captain_derlin_action_setResistanceCollectionStarterSlot(obj_id player, obj_id npc) throws InterruptedException
	{
		modifyCollectionSlotValue(player, holiday.REBEL_RESISTANCE_START_SLOT, 1);
	}
	
	
	public void rebel_remembrance_day_captain_derlin_action_setVandalCollectionStarterSlot(obj_id player, obj_id npc) throws InterruptedException
	{
		modifyCollectionSlotValue(player, holiday.REBEL_VANDAL_START_SLOT, 1);
	}
	
	
	public void rebel_remembrance_day_captain_derlin_action_setCombatantResistanceEventOnPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		
		if (getCollectionSlotValue(player, holiday.REBEL_RESISTANCE_START_SLOT) <= 0)
		{
			rebel_remembrance_day_captain_derlin_action_setResistanceCollectionStarterSlot(player, npc);
		}
		
		buff.applyBuff(player, holiday.BUFF_REB_EMPIREDAY_RESISTANCE_COMBATANT);
	}
	
	
	public void rebel_remembrance_day_captain_derlin_action_setCombatantVandalEventOnPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		
		if (getCollectionSlotValue(player, holiday.REBEL_VANDAL_START_SLOT) <= 0)
		{
			rebel_remembrance_day_captain_derlin_action_setVandalCollectionStarterSlot(player, npc);
		}
		
		buff.applyBuff(player, holiday.BUFF_REB_EMPIREDAY_VANDAL_COMBATANT);
	}
	
	
	public void rebel_remembrance_day_captain_derlin_action_makeNeutralRebel(obj_id player, obj_id npc) throws InterruptedException
	{
		play2dNonLoopingSound(player, space_quest.MUSIC_QUEST_EVENT);
		setObjVar(player, holiday.PLAYER_EMPIRE_DAY_NEUTRAL + holiday.PLANET_VAR_PLAYER_FACTION_REB, true);
	}
	
	
	public void rebel_remembrance_day_captain_derlin_action_checkHolidayTimeStampOnPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		holiday.checkEventLockOutRemoval(player, holiday.EMPIRE_DAY_RESISTANCE_TIMESTAMP, holiday.EMPIRE_DAY_RESISTANCE_LOCKED_OUT);
		holiday.checkEventLockOutRemoval(player, holiday.EMPIRE_DAY_VANDAL_TIMESTAMP, holiday.EMPIRE_DAY_VANDAL_LOCKED_OUT);
	}
	
	
	public void rebel_remembrance_day_captain_derlin_action_removeNeutralImperial(obj_id player, obj_id npc) throws InterruptedException
	{
		removeObjVar(player, holiday.PLAYER_EMPIRE_DAY_NEUTRAL + holiday.PLANET_VAR_PLAYER_FACTION_IMP);
	}
	
	
	public String rebel_remembrance_day_captain_derlin_tokenTO_timeLeftVandal(obj_id player, obj_id npc) throws InterruptedException
	{
		String returnString = holiday.getTimeRemainingBeforeLockoutRemoved(player, holiday.EMPIRE_DAY_VANDAL_TIMESTAMP);
		if (returnString == null || returnString.equals(""))
		{
			return "might have to contact Customer Service. I can't seem to get any time data off your character.";
		}
		return returnString;
	}
	
	
	public String rebel_remembrance_day_captain_derlin_tokenTO_timeLeftResistance(obj_id player, obj_id npc) throws InterruptedException
	{
		String returnString = holiday.getTimeRemainingBeforeLockoutRemoved(player, holiday.EMPIRE_DAY_RESISTANCE_TIMESTAMP);
		if (returnString == null || returnString.equals(""))
		{
			return "might have to contact Customer Service. I can't seem to get any time data off your character.";
		}
		return returnString;
	}
	
	
	public String rebel_remembrance_day_captain_derlin_tokenTO_currentScore(obj_id player, obj_id npc) throws InterruptedException
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
	
	
	public int rebel_remembrance_day_captain_derlin_tokenDI_notUsed(obj_id player, obj_id npc) throws InterruptedException
	{
		return 0;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_173"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition_hasCompletedBothCollections (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_178");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_179");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 6);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_captain_derlin_condition_isLockedOutOfEvents (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_175");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (!rebel_remembrance_day_captain_derlin_condition_canDoAtLeastOneEvent (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_56");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_captain_derlin_condition_canDoAtLeastOneEvent (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_176");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition_canDoResistanceEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_captain_derlin_condition_canDoVandalEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (rebel_remembrance_day_captain_derlin_condition_isTrader (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (rebel_remembrance_day_captain_derlin_condition_isEntertainer (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				boolean hasResponse5 = false;
				if (!rebel_remembrance_day_captain_derlin_condition_hasRemembranceDayBadge (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_97");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_166");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_208");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_212");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_216");
					}
					
					if (hasResponse5)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_220");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 30);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_117");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_121"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_127");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_123"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_128");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_93"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_94");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				pp.other.set (rebel_remembrance_day_captain_derlin_tokenTO_currentScore (player, npc));
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_126"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				rebel_remembrance_day_captain_derlin_action_getLeaderBoardSui (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_136");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_173"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition_hasCompletedBothCollections (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_178");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_179");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 6);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_captain_derlin_condition_isLockedOutOfEvents (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_175");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (!rebel_remembrance_day_captain_derlin_condition_canDoAtLeastOneEvent (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_56");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_captain_derlin_condition_canDoAtLeastOneEvent (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_176");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition_canDoResistanceEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_captain_derlin_condition_canDoVandalEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (rebel_remembrance_day_captain_derlin_condition_isTrader (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (rebel_remembrance_day_captain_derlin_condition_isEntertainer (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				boolean hasResponse5 = false;
				if (!rebel_remembrance_day_captain_derlin_condition_hasRemembranceDayBadge (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_97");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_166");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_208");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_212");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_216");
					}
					
					if (hasResponse5)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_220");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 30);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_117");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_121"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_127");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_123"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_128");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_93"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_94");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				pp.other.set (rebel_remembrance_day_captain_derlin_tokenTO_currentScore (player, npc));
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_126"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				rebel_remembrance_day_captain_derlin_action_getLeaderBoardSui (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_136");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_179"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_180");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition_hasCompletedResistanceCollection (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_181");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 7);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_181"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_182");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_185");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_185"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition_hasCompletedBothCollections (player, npc))
			{
				rebel_remembrance_day_captain_derlin_action_resetBothCollections (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_188");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition_canDoResistanceEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_captain_derlin_condition_canDoVandalEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (rebel_remembrance_day_captain_derlin_condition_isTrader (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (rebel_remembrance_day_captain_derlin_condition_isEntertainer (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				boolean hasResponse5 = false;
				if (!rebel_remembrance_day_captain_derlin_condition_hasRemembranceDayBadge (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_97");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_166");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_208");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_212");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_216");
					}
					
					if (hasResponse5)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_220");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 30);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_97"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_99");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_106");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 31);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_166"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_177");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_184");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 39);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_208"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_210");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_212"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_214");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_216"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				rebel_remembrance_day_captain_derlin_action_getLeaderBoardSui (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_218");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_220"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_222");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_64"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_90");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 11);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_57"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition_isTraderOrEntertainer (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_59");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (!rebel_remembrance_day_captain_derlin_condition_isTraderOrEntertainer (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_120");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_122");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 15);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch11(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_91"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_95");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				pp.other.set (rebel_remembrance_day_captain_derlin_tokenTO_timeLeftVandal (player, npc));
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_92"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_96");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				pp.other.set (rebel_remembrance_day_captain_derlin_tokenTO_timeLeftResistance (player, npc));
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch15(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_122"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				rebel_remembrance_day_captain_derlin_action_giveWedgeAntillesWaypoint (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_124");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch18(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_97"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_99");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_106");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 31);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_166"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_177");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_184");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 39);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_208"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_210");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_212"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_214");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_216"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				rebel_remembrance_day_captain_derlin_action_getLeaderBoardSui (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_218");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_220"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_222");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch24(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_69"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_72");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 29);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch25(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_98"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_100");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_101");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 26);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch26(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_101"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_102");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 27);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch27(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_103"))
		{
			doAnimationAction (player, "salute1");
			
			rebel_remembrance_day_captain_derlin_action_makeNeutralRebel (player, npc);
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_72");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 29);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch28(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_69"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_72");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 29);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch29(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_76"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "pound_fist_palm");
				
				string_id message = new string_id (c_stringFile, "s_80");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition_canDoResistanceEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_captain_derlin_condition_canDoVandalEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (rebel_remembrance_day_captain_derlin_condition_isTrader (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (rebel_remembrance_day_captain_derlin_condition_isEntertainer (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				boolean hasResponse5 = false;
				if (!rebel_remembrance_day_captain_derlin_condition_hasRemembranceDayBadge (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_97");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_166");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_208");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_212");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_216");
					}
					
					if (hasResponse5)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_220");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 30);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
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
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch30(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_97"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_99");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_106");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 31);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_166"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_177");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_184");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 39);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_208"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_210");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_212"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_214");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_216"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				rebel_remembrance_day_captain_derlin_action_getLeaderBoardSui (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_218");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_220"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_222");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch31(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_106"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_110");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_114");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 32);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch32(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_114"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition_isQualifiedResistanceCombatant (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_138");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_142");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 33);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_captain_derlin_condition_isQualifiedResistanceSpecialForces (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_154");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_156");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 36);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch33(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_142"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition_canDoAtLeastOneEvent (player, npc))
			{
				rebel_remembrance_day_captain_derlin_action_setCombatantResistanceEventOnPlayer (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_146");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition_canDoResistanceEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_captain_derlin_condition_canDoVandalEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (rebel_remembrance_day_captain_derlin_condition_isTrader (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (rebel_remembrance_day_captain_derlin_condition_isEntertainer (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				boolean hasResponse5 = false;
				if (!rebel_remembrance_day_captain_derlin_condition_hasRemembranceDayBadge (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_97");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_166");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_208");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_212");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_216");
					}
					
					if (hasResponse5)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_220");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 30);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (!rebel_remembrance_day_captain_derlin_condition_canDoAtLeastOneEvent (player, npc))
			{
				rebel_remembrance_day_captain_derlin_action_setCombatantResistanceEventOnPlayer (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_150");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch34(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_97"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_99");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_106");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 31);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_166"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_177");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_184");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 39);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_208"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_210");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_212"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_214");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_216"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				rebel_remembrance_day_captain_derlin_action_getLeaderBoardSui (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_218");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_220"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_222");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch36(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_156"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition_canDoAtLeastOneEvent (player, npc))
			{
				rebel_remembrance_day_captain_derlin_action_setSFResistanceEventOnPlayer (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_158");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition_canDoResistanceEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_captain_derlin_condition_canDoVandalEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (rebel_remembrance_day_captain_derlin_condition_isTrader (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (rebel_remembrance_day_captain_derlin_condition_isEntertainer (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				boolean hasResponse5 = false;
				if (!rebel_remembrance_day_captain_derlin_condition_hasRemembranceDayBadge (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_97");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_166");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_208");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_212");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_216");
					}
					
					if (hasResponse5)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_220");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 30);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (!rebel_remembrance_day_captain_derlin_condition_canDoAtLeastOneEvent (player, npc))
			{
				rebel_remembrance_day_captain_derlin_action_setSFResistanceEventOnPlayer (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_162");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch37(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_97"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_99");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_106");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 31);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_166"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_177");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_184");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 39);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_208"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_210");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_212"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_214");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_216"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				rebel_remembrance_day_captain_derlin_action_getLeaderBoardSui (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_218");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_220"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_222");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch39(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_184"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_187");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_190");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 40);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch40(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_190"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition_isQualifiedVandalCombatant (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_192");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_194");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 41);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_captain_derlin_condition_isQualifiedVandalSpecialForces (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_200");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_202");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 44);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch41(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_194"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition_canDoAtLeastOneEvent (player, npc))
			{
				rebel_remembrance_day_captain_derlin_action_setCombatantVandalEventOnPlayer (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_196");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition_canDoResistanceEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_captain_derlin_condition_canDoVandalEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (rebel_remembrance_day_captain_derlin_condition_isTrader (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (rebel_remembrance_day_captain_derlin_condition_isEntertainer (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				boolean hasResponse5 = false;
				if (!rebel_remembrance_day_captain_derlin_condition_hasRemembranceDayBadge (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_97");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_166");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_208");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_212");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_216");
					}
					
					if (hasResponse5)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_220");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 30);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (!rebel_remembrance_day_captain_derlin_condition_canDoAtLeastOneEvent (player, npc))
			{
				rebel_remembrance_day_captain_derlin_action_setCombatantVandalEventOnPlayer (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_198");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch42(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_97"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_99");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_106");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 31);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_166"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_177");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_184");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 39);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_208"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_210");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_212"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_214");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_216"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				rebel_remembrance_day_captain_derlin_action_getLeaderBoardSui (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_218");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_220"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_222");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch44(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_202"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition_canDoAtLeastOneEvent (player, npc))
			{
				rebel_remembrance_day_captain_derlin_action_setSFVandalEventOnPlayer (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_204");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition_canDoResistanceEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_captain_derlin_condition_canDoVandalEvent (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (rebel_remembrance_day_captain_derlin_condition_isTrader (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (rebel_remembrance_day_captain_derlin_condition_isEntertainer (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				boolean hasResponse5 = false;
				if (!rebel_remembrance_day_captain_derlin_condition_hasRemembranceDayBadge (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_97");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_166");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_208");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_212");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_216");
					}
					
					if (hasResponse5)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_220");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 30);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (!rebel_remembrance_day_captain_derlin_condition_canDoAtLeastOneEvent (player, npc))
			{
				rebel_remembrance_day_captain_derlin_action_setSFVandalEventOnPlayer (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_206");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_captain_derlin_handleBranch45(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_97"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_99");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_106");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 31);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_166"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_177");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_184");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 39);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_208"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_210");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_212"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_214");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_216"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				rebel_remembrance_day_captain_derlin_action_getLeaderBoardSui (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_218");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_220"))
		{
			
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_222");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
				
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
			detachScript(self, "conversation.rebel_remembrance_day_captain_derlin");
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
		detachScript (self, "conversation.rebel_remembrance_day_captain_derlin");
		
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
		
		if (rebel_remembrance_day_captain_derlin_condition_isImperialPlayer (player, npc))
		{
			doAnimationAction (npc, "backhand_threaten");
			
			string_id message = new string_id (c_stringFile, "s_50");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (rebel_remembrance_day_captain_derlin_condition_isNeutralImperialButRebel (player, npc))
		{
			rebel_remembrance_day_captain_derlin_action_removeNeutralImperial (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_104");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (rebel_remembrance_day_captain_derlin_condition_isNeutralImperial (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_88");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (rebel_remembrance_day_captain_derlin_condition_isChampionKnowsDerlin (player, npc))
		{
			rebel_remembrance_day_captain_derlin_action_checkHolidayTimeStampOnPlayer (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_67");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (rebel_remembrance_day_captain_derlin_condition_isTrader (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse1 = true;
			}
			
			boolean hasResponse2 = false;
			if (rebel_remembrance_day_captain_derlin_condition_isEntertainer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse2 = true;
			}
			
			boolean hasResponse3 = false;
			if (rebel_remembrance_day_captain_derlin_condition_hasScore (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse3 = true;
			}
			
			boolean hasResponse4 = false;
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_123");
				}
				
				if (hasResponse3)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_93");
				}
				
				if (hasResponse4)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_126");
				}
				
				utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 5);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "rebel_remembrance_day_captain_derlin", null, pp, responses);
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
		
		if (rebel_remembrance_day_captain_derlin_condition_alreadyKnowsDerlin (player, npc))
		{
			rebel_remembrance_day_captain_derlin_action_checkHolidayTimeStampOnPlayer (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_116");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (rebel_remembrance_day_captain_derlin_condition_isTrader (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse1 = true;
			}
			
			boolean hasResponse2 = false;
			if (rebel_remembrance_day_captain_derlin_condition_isEntertainer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse2 = true;
			}
			
			boolean hasResponse3 = false;
			if (rebel_remembrance_day_captain_derlin_condition_hasScore (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse3 = true;
			}
			
			boolean hasResponse4 = false;
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_123");
				}
				
				if (hasResponse3)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_93");
				}
				
				if (hasResponse4)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_126");
				}
				
				utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 5);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "rebel_remembrance_day_captain_derlin", null, pp, responses);
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
		
		if (rebel_remembrance_day_captain_derlin_condition_isChampionDoesntKnowDerlin (player, npc))
		{
			doAnimationAction (npc, "salute2");
			
			doAnimationAction (player, "salute2");
			
			string_id message = new string_id (c_stringFile, "s_109");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 28);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "rebel_remembrance_day_captain_derlin", null, pp, responses);
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
		
		if (rebel_remembrance_day_captain_derlin_condition_isNewNeutralPlayer (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_89");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_98");
				}
				
				utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 25);
				
				npcStartConversation (player, npc, "rebel_remembrance_day_captain_derlin", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
		{
			doAnimationAction (npc, "salute2");
			
			doAnimationAction (player, "salute2");
			
			string_id message = new string_id (c_stringFile, "s_58");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rebel_remembrance_day_captain_derlin_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId", 28);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "rebel_remembrance_day_captain_derlin", null, pp, responses);
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
		if (!conversationId.equals("rebel_remembrance_day_captain_derlin"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
		
		if (branchId == 4 && rebel_remembrance_day_captain_derlin_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && rebel_remembrance_day_captain_derlin_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && rebel_remembrance_day_captain_derlin_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && rebel_remembrance_day_captain_derlin_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && rebel_remembrance_day_captain_derlin_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && rebel_remembrance_day_captain_derlin_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && rebel_remembrance_day_captain_derlin_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 11 && rebel_remembrance_day_captain_derlin_handleBranch11 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 15 && rebel_remembrance_day_captain_derlin_handleBranch15 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 18 && rebel_remembrance_day_captain_derlin_handleBranch18 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 24 && rebel_remembrance_day_captain_derlin_handleBranch24 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 25 && rebel_remembrance_day_captain_derlin_handleBranch25 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 26 && rebel_remembrance_day_captain_derlin_handleBranch26 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 27 && rebel_remembrance_day_captain_derlin_handleBranch27 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 28 && rebel_remembrance_day_captain_derlin_handleBranch28 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 29 && rebel_remembrance_day_captain_derlin_handleBranch29 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 30 && rebel_remembrance_day_captain_derlin_handleBranch30 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 31 && rebel_remembrance_day_captain_derlin_handleBranch31 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 32 && rebel_remembrance_day_captain_derlin_handleBranch32 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 33 && rebel_remembrance_day_captain_derlin_handleBranch33 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 34 && rebel_remembrance_day_captain_derlin_handleBranch34 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 36 && rebel_remembrance_day_captain_derlin_handleBranch36 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 37 && rebel_remembrance_day_captain_derlin_handleBranch37 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 39 && rebel_remembrance_day_captain_derlin_handleBranch39 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 40 && rebel_remembrance_day_captain_derlin_handleBranch40 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 41 && rebel_remembrance_day_captain_derlin_handleBranch41 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 42 && rebel_remembrance_day_captain_derlin_handleBranch42 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 44 && rebel_remembrance_day_captain_derlin_handleBranch44 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 45 && rebel_remembrance_day_captain_derlin_handleBranch45 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.rebel_remembrance_day_captain_derlin.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
