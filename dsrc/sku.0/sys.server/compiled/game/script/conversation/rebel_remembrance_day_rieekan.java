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
import script.library.static_item;
import script.library.utils;


public class rebel_remembrance_day_rieekan extends script.base_script
{
	public rebel_remembrance_day_rieekan()
	{
	}
	String c_stringFile = "conversation/rebel_remembrance_day_rieekan";
	
	
	public boolean rebel_remembrance_day_rieekan_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_isEntertainerOrTrader(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (rebel_remembrance_day_rieekan_condition_isEntertainer(player, npc) || rebel_remembrance_day_rieekan_condition_isTrader(player, npc));
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_isNotFirstSupplyDropMission(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return rebel_remembrance_day_rieekan_condition_hasSupplyDropMissionWithTask(player, npc) && !rebel_remembrance_day_rieekan_condition_isReadyForFirstSupplyDrop(player, npc);
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_isEntertainerOrTraderAndNeverMetNotChampion(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (rebel_remembrance_day_rieekan_condition_isEntertainerOrTrader(player, npc) && !rebel_remembrance_day_rieekan_condition_alreadyKnowsRieekan(player, npc) && !rebel_remembrance_day_rieekan_condition_isChampion(player, npc));
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_hasNotCraftedSupplies(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		for (int i = 0; i < holiday.ALL_REBEL_QUESTS.length; i++)
		{
			testAbortScript();
			if ((groundquests.isQuestActive(player, holiday.ALL_REBEL_QUESTS[i]) && groundquests.isTaskActive(player, holiday.ALL_REBEL_QUESTS[i], "crafteSupplies")))
			{
				return true;
			}
		}
		return false;
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_hasAMisionActive(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		for (int i = 0; i < holiday.ALL_REBEL_QUESTS.length; i++)
		{
			testAbortScript();
			if (groundquests.isQuestActive(player, holiday.ALL_REBEL_QUESTS[i]))
			{
				return true;
			}
		}
		return false;
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_alreadyKnowsRieekan(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_ENTERTAINER_01) || groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_01_DOMESTICS) || groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_01_ENG) || groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_01_MUNITIONS) || groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_01_STRUC);
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_isEntertainerAndIsWaitingForSignal(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (rebel_remembrance_day_rieekan_condition_isEntertainer(player, npc) && rebel_remembrance_day_rieekan_condition_alreadyKnowsRieekan(player, npc) && rebel_remembrance_day_rieekan_condition_hasEntertainerMissionWithTask(player, npc));
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_isChampion(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasCompletedCollectionSlot(player, holiday.REMEMBRANCE_DAY_CHAMPION_BADGE);
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_hasEntertainerMissionWithTask(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		for (int i = 0; i < holiday.ALL_REBEL_ENTERTAINER_QUESTS.length; i++)
		{
			testAbortScript();
			if ((groundquests.isQuestActive(player, holiday.ALL_REBEL_ENTERTAINER_QUESTS[i]) && groundquests.isTaskActive(player, holiday.ALL_REBEL_ENTERTAINER_QUESTS[i], "waitingForCredit")))
			{
				return true;
			}
		}
		return false;
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_hasCompletedEntertainerOneNotTwo(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_ENTERTAINER_01) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_ENTERTAINER_02));
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_isImperialPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (factions.isImperial(player));
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_isNeutralPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return(!factions.isImperial(player) && !factions.isRebel(player));
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_hasCompletedEntertainerTwoNotThree(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_ENTERTAINER_02) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_ENTERTAINER_03));
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_hasCompletedEntertainerThreeNotFour(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_ENTERTAINER_03) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_ENTERTAINER_04));
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_hasCompletedEntertainerFourNotFive(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_ENTERTAINER_04) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_ENTERTAINER_05));
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_hasCompeletedEntertainerFiveNotSix(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_ENTERTAINER_05) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_ENTERTAINER_06));
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_isLockedOutOfRecoveryEvent(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return false;
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_hasCompleted1stMissionNot2nd(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_01_DOMESTICS) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_02_DOMESTICS)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_01_ENG) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_02_ENG)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_01_MUNITIONS) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_02_MUNITIONS)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_01_STRUC) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_02_STRUC));
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_hasCompleted2ndMissionNot3rd(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_02_DOMESTICS) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_03_DOMESTICS)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_02_ENG) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_03_ENG)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_02_MUNITIONS) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_03_MUNITIONS)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_02_STRUC) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_03_STRUC));
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_isEnterainerOrTraderAndNeverMetRiekanIsChampion(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (rebel_remembrance_day_rieekan_condition_isEntertainerOrTrader(player, npc) && rebel_remembrance_day_rieekan_condition_isChampion(player, npc) && !rebel_remembrance_day_rieekan_condition_alreadyKnowsRieekan(player, npc));
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_isChampionKnowsKaythree(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (rebel_remembrance_day_rieekan_condition_isChampion(player, npc) && rebel_remembrance_day_rieekan_condition_alreadyKnowsRieekan(player, npc));
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_isImperialOnLeave(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (factions.isImperial(player) && factions.isOnLeave(player));
		
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_hasCompleted3rdMissionNot4th(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_03_DOMESTICS) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_04_DOMESTICS)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_03_ENG) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_04_ENG)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_03_MUNITIONS) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_04_MUNITIONS)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_03_STRUC) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_04_STRUC));
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_hasCompleted4thMissionNot5th(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_04_DOMESTICS) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_05_DOMESTICS)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_04_ENG) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_05_ENG)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_04_MUNITIONS) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_05_MUNITIONS)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_04_STRUC) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_05_STRUC));
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_hasCompleted5thMissionNot6th(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_05_DOMESTICS) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_06_DOMESTICS)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_05_ENG) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_06_ENG)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_05_MUNITIONS) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_06_MUNITIONS)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_05_STRUC) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_06_STRUC));
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_hasCompleted6thMission(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_06_DOMESTICS) || groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_06_ENG) || groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_06_MUNITIONS) || groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_06_STRUC) || groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_ENTERTAINER_06);
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_isReadyForFirstSupplyDrop(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return rebel_remembrance_day_rieekan_condition_hasCompletedFirstMission(player, npc) && rebel_remembrance_day_rieekan_condition_hasSupplyDropMissionWithTask(player, npc);
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_isEntertainer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return utils.isProfession(player, utils.ENTERTAINER);
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_isTrader(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return utils.isProfession(player, utils.TRADER);
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_isStructuresTrader(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasSkill(player, "class_structures_phase1_novice");
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_isEngineeringTrader(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasSkill(player, "class_engineering_phase1_novice");
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_isMunitionsTrader(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasSkill(player, "class_munitions_phase1_novice");
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_isDomesticsTrader(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasSkill(player, "class_domestics_phase1_novice");
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_hasCompletedFirstMission(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_01_DOMESTICS) || groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_01_ENG) || groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_01_MUNITIONS) || groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_01_STRUC);
	}
	
	
	public boolean rebel_remembrance_day_rieekan_condition_hasSupplyDropMissionWithTask(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		for (int i = 0; i < holiday.ALL_REBEL_QUESTS.length; i++)
		{
			testAbortScript();
			if ((groundquests.isQuestActive(player, holiday.ALL_REBEL_QUESTS[i]) && groundquests.isTaskActive(player, holiday.ALL_REBEL_QUESTS[i], holiday.ALL_REBEL_QUESTS[i])))
			{
				return true;
			}
		}
		return false;
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantDomesticsMissionOne(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_01_DOMESTICS);
		if (getCollectionSlotValue(player, holiday.REBEL_ENGINEERING_START_SLOT) <= 0)
		{
			rebel_remembrance_day_rieekan_action_grantEngineeringCollectionStart(player, npc);
		}
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantDomesticsMissionTwo(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_02_DOMESTICS);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantDomesticsMissionThree(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_03_DOMESTICS);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantDomesticsMissionFour(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_04_DOMESTICS);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantDomesticsMissionFive(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_05_DOMESTICS);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantDomesticsMissionSix(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_06_DOMESTICS);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantEntertainerMissionOne(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_ENTERTAINER_01);
		modifyCollectionSlotValue(player, holiday.REBEL_ENTERTAINER_START_SLOT, 1);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantEntertainerMissionTwo(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_ENTERTAINER_02);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_checkHolidayTimeStampOnPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantEngineeringCollectionStart(obj_id player, obj_id npc) throws InterruptedException
	{
		if (getCollectionSlotValue(player, holiday.REBEL_ENGINEERING_START_SLOT) <= 0)
		{
			modifyCollectionSlotValue(player, holiday.REBEL_ENGINEERING_START_SLOT, 1);
		}
		
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantEngineeringMissionOne(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_01_ENG);
		if (getCollectionSlotValue(player, holiday.REBEL_ENGINEERING_START_SLOT) <= 0)
		{
			rebel_remembrance_day_rieekan_action_grantEngineeringCollectionStart(player, npc);
		}
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantEngineeringMissionTwo(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_02_ENG);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantEngineeringMissionThree(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_03_ENG);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantEngineeringMissionFour(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_04_ENG);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantEngineeringMissionFive(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_05_ENG);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantEnigineeringMissionSix(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_06_ENG);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantMunitionsMissionOne(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_01_MUNITIONS);
		if (getCollectionSlotValue(player, holiday.REBEL_ENGINEERING_START_SLOT) <= 0)
		{
			rebel_remembrance_day_rieekan_action_grantEngineeringCollectionStart(player, npc);
		}
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantMunitionsMissionTwo(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_02_MUNITIONS);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantMunitionsMissionThree(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_03_MUNITIONS);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantMunitionsMissionFour(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_04_MUNITIONS);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantMunitionsMissionFive(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_05_MUNITIONS);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantMunitionsMissionSix(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_06_MUNITIONS);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantStructuresMissionOne(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_01_STRUC);
		if (getCollectionSlotValue(player, holiday.REBEL_ENGINEERING_START_SLOT) <= 0)
		{
			rebel_remembrance_day_rieekan_action_grantEngineeringCollectionStart(player, npc);
		}
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantStructuresMissionTwo(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_02_STRUC);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantStructuresMissionThree(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_03_STRUC);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantStructuresMissionFour(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_04_STRUC);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantStructuresMissionFive(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_05_STRUC);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantStructuresMissionSix(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_CRASHSITE_06_STRUC);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_activateDeliverySignal(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "spokeToSupplyOfficer");
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantEntertainerMissionThree(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_ENTERTAINER_03);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantEntertainerMissionFour(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_ENTERTAINER_04);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantEntertainerMissionFive(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_ENTERTAINER_05);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_grantEntertainerMissionSix(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_ENTERTAINER_06);
	}
	
	
	public void rebel_remembrance_day_rieekan_action_entertainerSignalIncrementCollection(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "creditReceived");
		if (hasCompletedCollectionSlotPrereq(player, holiday.REBEL_ENTERTAINER_COUNTER_SLOT))
		{
			modifyCollectionSlotValue(player, holiday.REBEL_ENTERTAINER_COUNTER_SLOT, 1);
		}
	}
	
	
	public String rebel_remembrance_day_rieekan_tokenTO_timeLeftRecruitment(obj_id player, obj_id npc) throws InterruptedException
	{
		String returnString = holiday.getTimeRemainingBeforeLockoutRemoved(player, holiday.EMPIRE_DAY_RECRUITMENT_TIMESTAMP);
		if (returnString == null || returnString.equals(""))
		{
			return "might have to contact Customer Service. I can't seem to get any time data off your character.";
		}
		return returnString;
	}
	
	
	public String rebel_remembrance_day_rieekan_tokenTO_timeLeftPropaganda(obj_id player, obj_id npc) throws InterruptedException
	{
		String returnString = holiday.getTimeRemainingBeforeLockoutRemoved(player, holiday.EMPIRE_DAY_PROPAGANDA_TIMESTAMP);
		if (returnString == null || returnString.equals(""))
		{
			return "might have to contact Customer Service. I can't seem to get any time data off your character.";
		}
		return returnString;
	}
	
	
	public int rebel_remembrance_day_rieekan_tokenDI_notUsed(obj_id player, obj_id npc) throws InterruptedException
	{
		return 0;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_90"))
		{
			
			if (rebel_remembrance_day_rieekan_condition_hasCompletedEntertainerOneNotTwo (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_92");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_93");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompletedEntertainerTwoNotThree (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_95");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_100");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 12);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompletedEntertainerThreeNotFour (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_96");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 14);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompletedEntertainerFourNotFive (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_97");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_102");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 16);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompeletedEntertainerFiveNotSix (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_99");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 18);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompleted6thMission (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_128");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_108");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_91"))
		{
			
			if (rebel_remembrance_day_rieekan_condition_hasCompletedEntertainerOneNotTwo (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_92");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_93");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompletedEntertainerTwoNotThree (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_95");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_100");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 12);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompletedEntertainerThreeNotFour (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_96");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 14);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompletedEntertainerFourNotFive (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_97");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_102");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 16);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompeletedEntertainerFiveNotSix (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_99");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 18);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompleted6thMission (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_128");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_108");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_192"))
		{
			
			if (rebel_remembrance_day_rieekan_condition_hasCompleted1stMissionNot2nd (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_197");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_198");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 22);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompleted2ndMissionNot3rd (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_207");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_233");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 29);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompleted3rdMissionNot4th (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_214");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_215");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 34);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompleted4thMissionNot5th (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_225");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_237");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 41);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompleted5thMissionNot6th (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_230");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_243");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 46);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_289");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_91"))
		{
			
			if (rebel_remembrance_day_rieekan_condition_hasCompletedEntertainerOneNotTwo (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_92");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_93");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompletedEntertainerTwoNotThree (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_95");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_100");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 12);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompletedEntertainerThreeNotFour (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_96");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 14);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompletedEntertainerFourNotFive (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_97");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_102");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 16);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompeletedEntertainerFiveNotSix (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_99");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 18);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompleted6thMission (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_128");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_108");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_192"))
		{
			
			if (rebel_remembrance_day_rieekan_condition_hasCompleted1stMissionNot2nd (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_197");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_198");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 22);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompleted2ndMissionNot3rd (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_207");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_233");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 29);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompleted3rdMissionNot4th (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_214");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_215");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 34);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompleted4thMissionNot5th (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_225");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_237");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 41);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_hasCompleted5thMissionNot6th (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_230");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_243");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 46);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_289");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_93"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantEntertainerMissionTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_94");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch12(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_100"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantEntertainerMissionThree (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_104");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch14(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_101"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantEntertainerMissionFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_105");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch16(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_102"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantEntertainerMissionFive (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_106");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch18(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_103"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantEntertainerMissionSix (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_107");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch22(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_198"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_199");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_200");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 23);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
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
	
	
	public int rebel_remembrance_day_rieekan_handleBranch23(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_200"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_201");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_205");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 24);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
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
	
	
	public int rebel_remembrance_day_rieekan_handleBranch24(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_205"))
		{
			
			if (rebel_remembrance_day_rieekan_condition_isDomesticsTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantDomesticsMissionTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_206");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isEngineeringTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantEngineeringMissionTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_153");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isMunitionsTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantMunitionsMissionTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_159");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isStructuresTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantStructuresMissionTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_160");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch29(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_233"))
		{
			
			if (rebel_remembrance_day_rieekan_condition_isDomesticsTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantDomesticsMissionThree (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_234");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isEngineeringTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantEngineeringMissionThree (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_165");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isMunitionsTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantMunitionsMissionThree (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_166");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isStructuresTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantStructuresMissionThree (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_167");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch34(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_215"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_216");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_217");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 35);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch35(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_217"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_218");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_291");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 36);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch36(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_291"))
		{
			
			if (rebel_remembrance_day_rieekan_condition_isDomesticsTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantDomesticsMissionFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_293");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isEngineeringTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantEngineeringMissionFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_295");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isMunitionsTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantMunitionsMissionFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_297");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isStructuresTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantStructuresMissionFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_299");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch41(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_237"))
		{
			
			if (rebel_remembrance_day_rieekan_condition_isDomesticsTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantDomesticsMissionFive (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_238");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isEngineeringTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantEngineeringMissionFive (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_183");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isMunitionsTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantMunitionsMissionFive (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_184");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isStructuresTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantStructuresMissionFive (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_185");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch46(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_243"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_244");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_245");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 47);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
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
	
	
	public int rebel_remembrance_day_rieekan_handleBranch47(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_245"))
		{
			
			if (rebel_remembrance_day_rieekan_condition_isDomesticsTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantDomesticsMissionSix (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_246");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isEngineeringTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantEnigineeringMissionSix (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_191");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isMunitionsTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantMunitionsMissionSix (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_204");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isStructuresTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantStructuresMissionSix (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_249");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch53(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_193"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_194");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_149");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_280");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_284");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 58);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_195"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_196");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_149");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_280");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_284");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 58);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch54(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_149"))
		{
			
			if (rebel_remembrance_day_rieekan_condition_isEntertainer (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_151");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_154");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_158");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 59);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isTrader (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_254");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_256");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_270");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 63);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_280"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_282");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_284"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_286");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch55(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_149"))
		{
			
			if (rebel_remembrance_day_rieekan_condition_isEntertainer (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_151");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_154");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_158");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 59);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isTrader (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_254");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_256");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_270");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 63);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_280"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_282");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_284"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_286");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch56(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_137"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_140");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_149");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_280");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_284");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 58);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_144"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_147");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_149");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_280");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_284");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 58);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch57(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_149"))
		{
			
			if (rebel_remembrance_day_rieekan_condition_isEntertainer (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_151");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_154");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_158");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 59);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isTrader (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_254");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_256");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_270");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 63);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_280"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_282");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_284"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_286");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch58(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_149"))
		{
			
			if (rebel_remembrance_day_rieekan_condition_isEntertainer (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_151");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_154");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_158");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 59);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isTrader (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_254");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_256");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_270");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 63);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_280"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_282");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_284"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_286");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch59(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_154"))
		{
			
			if (rebel_remembrance_day_rieekan_condition_isEntertainer (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantEntertainerMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_156");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_158"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_171");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_177");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 61);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch61(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_177"))
		{
			
			if (rebel_remembrance_day_rieekan_condition_isEntertainer (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantEntertainerMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_190");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch63(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_256"))
		{
			
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_258");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_260");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 64);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_270"))
		{
			
			if (rebel_remembrance_day_rieekan_condition_isDomesticsTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantDomesticsMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_272");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isEngineeringTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantEngineeringMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_274");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isMunitionsTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantMunitionsMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_276");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isStructuresTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantStructuresMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_278");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_rieekan_handleBranch64(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_260"))
		{
			
			if (rebel_remembrance_day_rieekan_condition_isDomesticsTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantDomesticsMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_272");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isEngineeringTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantEngineeringMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_274");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isMunitionsTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantMunitionsMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_276");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_remembrance_day_rieekan_condition_isStructuresTrader (player, npc))
			{
				rebel_remembrance_day_rieekan_action_grantStructuresMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_278");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
				
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
			detachScript(self, "conversation.rebel_remembrance_day_rieekan");
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
		detachScript (self, "conversation.rebel_remembrance_day_rieekan");
		
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
		
		if (rebel_remembrance_day_rieekan_condition_isNeutralPlayer (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_85");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (rebel_remembrance_day_rieekan_condition_isImperialPlayer (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_133");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (rebel_remembrance_day_rieekan_condition_isReadyForFirstSupplyDrop (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_300");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (rebel_remembrance_day_rieekan_condition_hasNotCraftedSupplies (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_87");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (rebel_remembrance_day_rieekan_condition_isNotFirstSupplyDropMission (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_301");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (rebel_remembrance_day_rieekan_condition_hasCompleted6thMission (player, npc))
		{
			doAnimationAction (npc, "salute1");
			
			doAnimationAction (player, "salute1");
			
			string_id message = new string_id (c_stringFile, "s_145");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (rebel_remembrance_day_rieekan_condition_isEntertainerAndIsWaitingForSignal (player, npc))
		{
			rebel_remembrance_day_rieekan_action_entertainerSignalIncrementCollection (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_89");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_90");
				}
				
				utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 7);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "rebel_remembrance_day_rieekan", null, pp, responses);
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
		
		if (rebel_remembrance_day_rieekan_condition_isChampionKnowsKaythree (player, npc))
		{
			rebel_remembrance_day_rieekan_action_checkHolidayTimeStampOnPlayer (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_67");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rebel_remembrance_day_rieekan_condition_isEntertainer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (rebel_remembrance_day_rieekan_condition_isTrader (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_192");
				}
				
				utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 9);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "rebel_remembrance_day_rieekan", null, pp, responses);
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
		
		if (rebel_remembrance_day_rieekan_condition_alreadyKnowsRieekan (player, npc))
		{
			doAnimationAction (npc, "salute1");
			
			doAnimationAction (player, "salute1");
			
			string_id message = new string_id (c_stringFile, "s_116");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rebel_remembrance_day_rieekan_condition_isEntertainer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (rebel_remembrance_day_rieekan_condition_isTrader (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_192");
				}
				
				utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 9);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "rebel_remembrance_day_rieekan", null, pp, responses);
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
		
		if (rebel_remembrance_day_rieekan_condition_isEnterainerOrTraderAndNeverMetRiekanIsChampion (player, npc))
		{
			doAnimationAction (npc, "salute2");
			
			doAnimationAction (player, "salute2");
			
			string_id message = new string_id (c_stringFile, "s_129");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rebel_remembrance_day_rieekan_condition_isTrader (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (rebel_remembrance_day_rieekan_condition_isEntertainer (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_193");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_195");
				}
				
				utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 53);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "rebel_remembrance_day_rieekan", null, pp, responses);
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
		
		if (rebel_remembrance_day_rieekan_condition_isEntertainerOrTraderAndNeverMetNotChampion (player, npc))
		{
			doAnimationAction (npc, "salute2");
			
			doAnimationAction (player, "salute2");
			
			string_id message = new string_id (c_stringFile, "s_135");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_137");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_144");
				}
				
				utils.setScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId", 56);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "rebel_remembrance_day_rieekan", null, pp, responses);
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
		
		if (rebel_remembrance_day_rieekan_condition_isChampion (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_131");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (rebel_remembrance_day_rieekan_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_288");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("rebel_remembrance_day_rieekan"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
		
		if (branchId == 7 && rebel_remembrance_day_rieekan_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && rebel_remembrance_day_rieekan_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && rebel_remembrance_day_rieekan_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && rebel_remembrance_day_rieekan_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 12 && rebel_remembrance_day_rieekan_handleBranch12 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 14 && rebel_remembrance_day_rieekan_handleBranch14 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 16 && rebel_remembrance_day_rieekan_handleBranch16 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 18 && rebel_remembrance_day_rieekan_handleBranch18 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 22 && rebel_remembrance_day_rieekan_handleBranch22 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 23 && rebel_remembrance_day_rieekan_handleBranch23 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 24 && rebel_remembrance_day_rieekan_handleBranch24 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 29 && rebel_remembrance_day_rieekan_handleBranch29 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 34 && rebel_remembrance_day_rieekan_handleBranch34 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 35 && rebel_remembrance_day_rieekan_handleBranch35 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 36 && rebel_remembrance_day_rieekan_handleBranch36 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 41 && rebel_remembrance_day_rieekan_handleBranch41 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 46 && rebel_remembrance_day_rieekan_handleBranch46 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 47 && rebel_remembrance_day_rieekan_handleBranch47 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 53 && rebel_remembrance_day_rieekan_handleBranch53 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 54 && rebel_remembrance_day_rieekan_handleBranch54 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 55 && rebel_remembrance_day_rieekan_handleBranch55 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 56 && rebel_remembrance_day_rieekan_handleBranch56 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 57 && rebel_remembrance_day_rieekan_handleBranch57 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 58 && rebel_remembrance_day_rieekan_handleBranch58 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 59 && rebel_remembrance_day_rieekan_handleBranch59 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 61 && rebel_remembrance_day_rieekan_handleBranch61 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 63 && rebel_remembrance_day_rieekan_handleBranch63 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 64 && rebel_remembrance_day_rieekan_handleBranch64 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.rebel_remembrance_day_rieekan.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
