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


public class imperial_empire_day_kaythree extends script.base_script
{
	public imperial_empire_day_kaythree()
	{
	}
	String c_stringFile = "conversation/imperial_empire_day_kaythree";
	
	
	public boolean imperial_empire_day_kaythree_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_isEntertainerOrTrader(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (imperial_empire_day_kaythree_condition_isEntertainer(player, npc) || imperial_empire_day_kaythree_condition_isTrader(player, npc));
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_hasNotCraftedSupplies(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		for (int i = 0; i < holiday.ALL_IMPERIAL_QUESTS.length; i++)
		{
			testAbortScript();
			if ((groundquests.isQuestActive(player, holiday.ALL_IMPERIAL_QUESTS[i]) && groundquests.isTaskActive(player, holiday.ALL_IMPERIAL_QUESTS[i], "crafteSupplies")))
			{
				return true;
			}
		}
		return false;
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_isEntertainerOrTraderAndNeverMetNotChampion(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (imperial_empire_day_kaythree_condition_isEntertainerOrTrader(player, npc) && !imperial_empire_day_kaythree_condition_alreadyKnowsKaythree(player, npc) && !imperial_empire_day_kaythree_condition_isChampion(player, npc));
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_hasCompletedEntertainerOneNotTwo(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_ENTERTAINER_01) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_ENTERTAINER_02));
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_hasAMissionActive(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		for (int i = 0; i < holiday.ALL_IMPERIAL_QUESTS.length; i++)
		{
			testAbortScript();
			if (groundquests.isQuestActive(player, holiday.ALL_IMPERIAL_QUESTS[i]))
			{
				return true;
			}
		}
		return false;
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_alreadyKnowsKaythree(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_ENTERTAINER_01) || groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_01_DOMESTICS) || groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_01_ENG) || groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_01_MUNITIONS) || groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_01_STRUC);
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_hasRecoveryCollectionPrerequisite(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasCompletedCollectionSlotPrereq(player, holiday.IMPERIAL_ANTIPROP_COUNTER_SLOT);
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_isChampion(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasCompletedCollectionSlot(player, holiday.EMPIRE_DAY_CHAMPION_BADGE);
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_hasCompletedEntertainerTwoNotThree(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_ENTERTAINER_02) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_ENTERTAINER_03));
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_hasCompletedEntertainerThreeNotFour(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_ENTERTAINER_03) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_ENTERTAINER_04));
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_isRebelPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (factions.isRebel(player));
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_isNeutralPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return(!factions.isImperial(player) && !factions.isRebel(player));
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_hasCompletedEntertainerFourNotFive(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_ENTERTAINER_04) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_ENTERTAINER_05));
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_hasCompletedEntertainerFiveNotSix(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_ENTERTAINER_05) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_ENTERTAINER_06));
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_isEntertainerWaitingForSignal(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (imperial_empire_day_kaythree_condition_isEntertainer(player, npc) && imperial_empire_day_kaythree_condition_alreadyKnowsKaythree(player, npc) && imperial_empire_day_kaythree_condition_hasEntertainerMissionWithTask(player, npc));
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_hasEntertainerMissionWithTask(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		for (int i = 0; i < holiday.ALL_IMPERIAL_ENTERTAINER_QUESTS.length; i++)
		{
			testAbortScript();
			if ((groundquests.isQuestActive(player, holiday.ALL_IMPERIAL_ENTERTAINER_QUESTS[i]) && groundquests.isTaskActive(player, holiday.ALL_IMPERIAL_ENTERTAINER_QUESTS[i], "waitingForCredit")))
			{
				return true;
			}
		}
		return false;
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_hasCompleted1stMissionNot2nd(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_01_DOMESTICS) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_02_DOMESTICS)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_01_ENG) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_02_ENG)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_01_MUNITIONS) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_02_MUNITIONS)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_01_STRUC) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_02_STRUC));
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_hasCompleted2ndMissionNot3rd(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_02_DOMESTICS) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_03_DOMESTICS)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_02_ENG) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_03_ENG)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_02_MUNITIONS) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_03_MUNITIONS)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_02_STRUC) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_03_STRUC));
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_isEnterainerOrTraderAndNeverMetKaythreeIsChampion(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (imperial_empire_day_kaythree_condition_isChampion(player, npc) && !imperial_empire_day_kaythree_condition_alreadyKnowsKaythree(player, npc));
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_isChampionKnowsKaythree(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (imperial_empire_day_kaythree_condition_isChampion(player, npc) && imperial_empire_day_kaythree_condition_alreadyKnowsKaythree(player, npc));
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_hasCompleted3rdMissionNot4th(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_03_DOMESTICS) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_04_DOMESTICS)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_03_ENG) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_04_ENG)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_03_MUNITIONS) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_04_MUNITIONS)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_03_STRUC) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_04_STRUC));
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_hasCompleted4thMissionNot5th(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_04_DOMESTICS) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_05_DOMESTICS)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_04_ENG) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_05_ENG)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_04_MUNITIONS) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_05_MUNITIONS)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_04_STRUC) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_05_STRUC));
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_hasCompleted5thMissionNot6th(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_05_DOMESTICS) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_06_DOMESTICS)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_05_ENG) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_06_ENG)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_05_MUNITIONS) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_06_MUNITIONS)) || (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_05_STRUC) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_06_STRUC));
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_hasCompleted6thMission(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_ENTERTAINER_06) || groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_06_DOMESTICS) || groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_06_ENG) || groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_06_MUNITIONS) || groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_06_STRUC);
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_isEntertainer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return utils.isProfession(player, utils.ENTERTAINER);
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_isTrader(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return utils.isProfession(player, utils.TRADER);
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_isStructuresTrader(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasSkill(player, "class_structures_phase1_novice");
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_isEngineeringTrader(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasSkill(player, "class_engineering_phase1_novice");
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_isMunitionsTrader(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasSkill(player, "class_munitions_phase1_novice");
	}
	
	
	public boolean imperial_empire_day_kaythree_condition_isDomesticsTrader(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasSkill(player, "class_domestics_phase1_novice");
	}
	
	
	public void imperial_empire_day_kaythree_action_grantDomesticsMissionOne(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_01_DOMESTICS);
		if (getCollectionSlotValue(player, holiday.IMPERIAL_ENGINEERING_START_SLOT) <= 0)
		{
			imperial_empire_day_kaythree_action_grantEngineeringCollectionStart(player, npc);
		}
	}
	
	
	public void imperial_empire_day_kaythree_action_grantDomesticsMissionTwo(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_02_DOMESTICS);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantDomesticsMissionThree(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_03_DOMESTICS);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantDomesticsMissionFour(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_04_DOMESTICS);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantDomesticsMissionFive(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_05_DOMESTICS);
	}
	
	
	public void imperial_empire_day_kaythree_action_spawnTroopers(obj_id player, obj_id npc) throws InterruptedException
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
	
	
	public void imperial_empire_day_kaythree_action_grantDomesticsMissionSix(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_06_DOMESTICS);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantEntertainerOne(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_ENTERTAINER_01);
		modifyCollectionSlotValue(player, holiday.IMPERIAL_ENTERTAINER_START_SLOT, 1);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantEntertainerTwo(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_ENTERTAINER_02);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantEntertainerThree(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_ENTERTAINER_03);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantEntertainerFour(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_ENTERTAINER_04);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantEntertainerFive(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_ENTERTAINER_05);
	}
	
	
	public void imperial_empire_day_kaythree_action_checkHolidayTimeStampOnPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		
	}
	
	
	public void imperial_empire_day_kaythree_action_grantEntertainerSix(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_ENTERTAINER_06);
	}
	
	
	public void imperial_empire_day_kaythree_action_entertainerSendSignalIncrementCollection(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "creditReceived");
		if (hasCompletedCollectionSlotPrereq(player, holiday.IMPERIAL_ENTERTAINER_COUNTER_SLOT))
		{
			modifyCollectionSlotValue(player, holiday.IMPERIAL_ENTERTAINER_COUNTER_SLOT, 1);
		}
	}
	
	
	public void imperial_empire_day_kaythree_action_grantEngineeringCollectionStart(obj_id player, obj_id npc) throws InterruptedException
	{
		if (getCollectionSlotValue(player, holiday.IMPERIAL_ENGINEERING_START_SLOT) <= 0)
		{
			modifyCollectionSlotValue(player, holiday.IMPERIAL_ENGINEERING_START_SLOT, 1);
		}
		
	}
	
	
	public void imperial_empire_day_kaythree_action_grantEngineeringMissionOne(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_01_ENG);
		if (getCollectionSlotValue(player, holiday.IMPERIAL_ENGINEERING_START_SLOT) <= 0)
		{
			imperial_empire_day_kaythree_action_grantEngineeringCollectionStart(player, npc);
		}
	}
	
	
	public void imperial_empire_day_kaythree_action_grantEngineeringMissionTwo(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_02_ENG);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantEngineeringMissionThree(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_03_ENG);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantEngineeringMissionFour(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_04_ENG);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantEngineeringMissionFive(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_05_ENG);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantEnigineeringMissionSix(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_06_ENG);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantMunitionsMissionOne(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_01_MUNITIONS);
		if (getCollectionSlotValue(player, holiday.IMPERIAL_ENGINEERING_START_SLOT) <= 0)
		{
			imperial_empire_day_kaythree_action_grantEngineeringCollectionStart(player, npc);
		}
	}
	
	
	public void imperial_empire_day_kaythree_action_grantMunitionsMissionTwo(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_02_MUNITIONS);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantMunitionsMissionThree(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_03_MUNITIONS);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantMunitionsMissionFour(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_04_MUNITIONS);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantMunitionsMissionFive(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_05_MUNITIONS);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantMunitionsMissionSix(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_06_MUNITIONS);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantStructuresMissionOne(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_01_STRUC);
		if (getCollectionSlotValue(player, holiday.IMPERIAL_ENGINEERING_START_SLOT) <= 0)
		{
			imperial_empire_day_kaythree_action_grantEngineeringCollectionStart(player, npc);
		}
	}
	
	
	public void imperial_empire_day_kaythree_action_grantStructuresMissionTwo(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_02_STRUC);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantStructuresMissionThree(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_03_STRUC);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantStructuresMissionFour(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_04_STRUC);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantStructuresMissionFive(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_05_STRUC);
	}
	
	
	public void imperial_empire_day_kaythree_action_grantStructuresMissionSix(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_06_STRUC);
	}
	
	
	public String imperial_empire_day_kaythree_tokenTO_timeLeftRecruitment(obj_id player, obj_id npc) throws InterruptedException
	{
		String returnString = holiday.getTimeRemainingBeforeLockoutRemoved(player, holiday.EMPIRE_DAY_RECRUITMENT_TIMESTAMP);
		if (returnString == null || returnString.equals(""))
		{
			return "might have to contact Customer Service. I can't seem to get any time data off your character.";
		}
		return returnString;
	}
	
	
	public String imperial_empire_day_kaythree_tokenTO_timeLeftPropaganda(obj_id player, obj_id npc) throws InterruptedException
	{
		String returnString = holiday.getTimeRemainingBeforeLockoutRemoved(player, holiday.EMPIRE_DAY_PROPAGANDA_TIMESTAMP);
		if (returnString == null || returnString.equals(""))
		{
			return "might have to contact Customer Service. I can't seem to get any time data off your character.";
		}
		return returnString;
	}
	
	
	public int imperial_empire_day_kaythree_tokenDI_notUsed(obj_id player, obj_id npc) throws InterruptedException
	{
		return 0;
	}
	
	
	public int imperial_empire_day_kaythree_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_298"))
		{
			
			if (imperial_empire_day_kaythree_condition_hasCompletedEntertainerOneNotTwo (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_148");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_150");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 73);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompletedEntertainerTwoNotThree (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_157");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_161");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 75);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompletedEntertainerThreeNotFour (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_189");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_253");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 77);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompletedEntertainerFourNotFive (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_257");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_259");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 79);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompletedEntertainerFiveNotSix (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_263");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_265");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 81);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompleted6thMission (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_301");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_303");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
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
	
	
	public int imperial_empire_day_kaythree_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_192"))
		{
			
			if (imperial_empire_day_kaythree_condition_hasCompleted1stMissionNot2nd (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_197");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompleted2ndMissionNot3rd (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_207");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_208");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_233");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 19);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompleted3rdMissionNot4th (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_214");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 30);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompleted4thMissionNot5th (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_225");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_226");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_237");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 46);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompleted5thMissionNot6th (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_230");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_231");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_243");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_247");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 56);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_289");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_146"))
		{
			
			if (imperial_empire_day_kaythree_condition_hasCompletedEntertainerOneNotTwo (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_148");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_150");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 73);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompletedEntertainerTwoNotThree (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_157");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_161");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 75);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompletedEntertainerThreeNotFour (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_189");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_253");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 77);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompletedEntertainerFourNotFive (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_257");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_259");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 79);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompletedEntertainerFiveNotSix (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_263");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_265");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 81);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompleted6thMission (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_301");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_303");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
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
	
	
	public int imperial_empire_day_kaythree_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_192"))
		{
			
			if (imperial_empire_day_kaythree_condition_hasCompleted1stMissionNot2nd (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_197");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompleted2ndMissionNot3rd (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_207");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_208");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_233");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 19);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompleted3rdMissionNot4th (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_214");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 30);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompleted4thMissionNot5th (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_225");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_226");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_237");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 46);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompleted5thMissionNot6th (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_230");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_231");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_243");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_247");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 56);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_289");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_146"))
		{
			
			if (imperial_empire_day_kaythree_condition_hasCompletedEntertainerOneNotTwo (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_148");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_150");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 73);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompletedEntertainerTwoNotThree (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_157");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_161");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 75);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompletedEntertainerThreeNotFour (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_189");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_253");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 77);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompletedEntertainerFourNotFive (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_257");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_259");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 79);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompletedEntertainerFiveNotSix (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_263");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_265");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 81);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_hasCompleted6thMission (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_301");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_303");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
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
	
	
	public int imperial_empire_day_kaythree_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_198"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_199");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 9);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
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
	
	
	public int imperial_empire_day_kaythree_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_200"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_201");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_202");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_205");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 10);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
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
	
	
	public int imperial_empire_day_kaythree_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_202"))
		{
			imperial_empire_day_kaythree_action_grantDomesticsMissionTwo (player, npc);
			
			if (imperial_empire_day_kaythree_condition_isDomesticsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantDomesticsMissionTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_203");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isEngineeringTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantEngineeringMissionTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_139");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isMunitionsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantMunitionsMissionTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_141");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isStructuresTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantStructuresMissionTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_143");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_205"))
		{
			
			if (imperial_empire_day_kaythree_condition_isDomesticsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantDomesticsMissionTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_206");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isEngineeringTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantEngineeringMissionTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_153");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isMunitionsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantMunitionsMissionTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_159");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isStructuresTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantStructuresMissionTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_160");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_kaythree_handleBranch19(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_208"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_209");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_210");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 20);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_233"))
		{
			
			if (imperial_empire_day_kaythree_condition_isDomesticsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantDomesticsMissionThree (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_234");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isEngineeringTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantEngineeringMissionThree (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_165");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isMunitionsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantMunitionsMissionThree (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_166");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isStructuresTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantStructuresMissionThree (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_167");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_kaythree_handleBranch20(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_210"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_211");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_212");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 21);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
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
	
	
	public int imperial_empire_day_kaythree_handleBranch21(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_212"))
		{
			
			if (imperial_empire_day_kaythree_condition_isDomesticsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantDomesticsMissionThree (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_213");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isEngineeringTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantEngineeringMissionThree (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_162");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isMunitionsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantMunitionsMissionThree (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_163");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isStructuresTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantStructuresMissionThree (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_164");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_kaythree_handleBranch30(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_215"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_216");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_217");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_235");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 31);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
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
	
	
	public int imperial_empire_day_kaythree_handleBranch31(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_217"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_218");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_219");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_220");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 32);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_235"))
		{
			
			if (imperial_empire_day_kaythree_condition_isDomesticsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantDomesticsMissionFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_236");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isEngineeringTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantEngineeringMissionFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_176");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isMunitionsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantMunitionsMissionFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_178");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isStructuresTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantStructuresMissionFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_179");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_kaythree_handleBranch32(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_219"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_221");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 33);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_220"))
		{
			
			if (imperial_empire_day_kaythree_condition_isDomesticsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantDomesticsMissionFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_223");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isEngineeringTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantEngineeringMissionFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_172");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isMunitionsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantMunitionsMissionFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_173");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isStructuresTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantStructuresMissionFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_175");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_kaythree_handleBranch33(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_222"))
		{
			
			if (imperial_empire_day_kaythree_condition_isDomesticsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantDomesticsMissionFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_224");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isEngineeringTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantEngineeringMissionFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_168");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isMunitionsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantMunitionsMissionFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_169");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isStructuresTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantStructuresMissionFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_170");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_kaythree_handleBranch46(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_226"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_227");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_228");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 47);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_237"))
		{
			
			if (imperial_empire_day_kaythree_condition_isDomesticsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantDomesticsMissionFive (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_238");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isEngineeringTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantEngineeringMissionFive (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_183");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isMunitionsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantMunitionsMissionFive (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_184");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isStructuresTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantStructuresMissionFive (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_185");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_kaythree_handleBranch47(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_228"))
		{
			
			if (imperial_empire_day_kaythree_condition_isDomesticsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantDomesticsMissionFive (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_229");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isEngineeringTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantEngineeringMissionFive (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_180");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isMunitionsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantMunitionsMissionFive (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_181");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isStructuresTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantStructuresMissionFive (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_182");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_kaythree_handleBranch56(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_231"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_232");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_239");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 57);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_243"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_244");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 63);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_247"))
		{
			
			if (imperial_empire_day_kaythree_condition_isDomesticsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantDomesticsMissionSix (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_248");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isEngineeringTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantEnigineeringMissionSix (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_250");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isMunitionsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantMunitionsMissionSix (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_251");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isStructuresTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantStructuresMissionSix (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_252");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_kaythree_handleBranch57(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_239"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_240");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_241");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 58);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
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
	
	
	public int imperial_empire_day_kaythree_handleBranch58(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_241"))
		{
			
			if (imperial_empire_day_kaythree_condition_isDomesticsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantDomesticsMissionThree (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_242");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isEngineeringTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantEnigineeringMissionSix (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_186");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isMunitionsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantMunitionsMissionSix (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_187");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isStructuresTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantStructuresMissionSix (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_188");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
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
	
	
	public int imperial_empire_day_kaythree_handleBranch63(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_245"))
		{
			
			if (imperial_empire_day_kaythree_condition_isDomesticsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantDomesticsMissionSix (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_246");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isEngineeringTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantEnigineeringMissionSix (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_191");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isMunitionsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantMunitionsMissionSix (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_204");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isStructuresTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantStructuresMissionSix (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_249");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
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
	
	
	public int imperial_empire_day_kaythree_handleBranch73(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_150"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_kaythree_action_grantEntertainerTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_155");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
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
	
	
	public int imperial_empire_day_kaythree_handleBranch75(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_161"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_kaythree_action_grantEntertainerThree (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_174");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
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
	
	
	public int imperial_empire_day_kaythree_handleBranch77(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_253"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_kaythree_action_grantEntertainerFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_255");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
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
	
	
	public int imperial_empire_day_kaythree_handleBranch79(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_259"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_kaythree_action_grantEntertainerFive (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_261");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
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
	
	
	public int imperial_empire_day_kaythree_handleBranch81(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_265"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_kaythree_action_grantEntertainerSix (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_267");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
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
	
	
	public int imperial_empire_day_kaythree_handleBranch85(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_193"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_194");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_151");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_292");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_296");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 90);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_195"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_196");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_151");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_292");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_296");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 90);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_kaythree_handleBranch86(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_151"))
		{
			
			if (imperial_empire_day_kaythree_condition_isEntertainer (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_154");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_156");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_171");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 91);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isTrader (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_260");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_264");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_280");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 95);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_292"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_294");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_296"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_300");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
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
	
	
	public int imperial_empire_day_kaythree_handleBranch87(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_151"))
		{
			
			if (imperial_empire_day_kaythree_condition_isEntertainer (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_154");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_156");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_171");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 91);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isTrader (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_260");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_264");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_280");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 95);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_292"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_294");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_296"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_300");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
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
	
	
	public int imperial_empire_day_kaythree_handleBranch88(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_140"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_144");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_151");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_292");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_296");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 90);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_147"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_149");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_151");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_292");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_296");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 90);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_kaythree_handleBranch89(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_151"))
		{
			
			if (imperial_empire_day_kaythree_condition_isEntertainer (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_154");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_156");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_171");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 91);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isTrader (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_260");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_264");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_280");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 95);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_292"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_294");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_296"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_300");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
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
	
	
	public int imperial_empire_day_kaythree_handleBranch90(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_151"))
		{
			
			if (imperial_empire_day_kaythree_condition_isEntertainer (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_154");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_156");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_171");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 91);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isTrader (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_260");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_264");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_280");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 95);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_292"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_294");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_296"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_300");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
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
	
	
	public int imperial_empire_day_kaythree_handleBranch91(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_156"))
		{
			
			if (imperial_empire_day_kaythree_condition_isEntertainer (player, npc))
			{
				imperial_empire_day_kaythree_action_grantEntertainerOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_158");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_171"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_177");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 93);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_kaythree_handleBranch93(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_190"))
		{
			
			if (imperial_empire_day_kaythree_condition_isEntertainerOrTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantEntertainerOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_256");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_kaythree_handleBranch95(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_264"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_268");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 96);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_280"))
		{
			
			if (imperial_empire_day_kaythree_condition_isDomesticsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantDomesticsMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_282");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isEngineeringTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantEngineeringMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_284");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isMunitionsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantMunitionsMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_286");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isStructuresTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantStructuresMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_288");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_kaythree_handleBranch96(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_270"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_272");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_274");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 97);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
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
	
	
	public int imperial_empire_day_kaythree_handleBranch97(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_274"))
		{
			
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_276");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_278");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 98);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
					
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
	
	
	public int imperial_empire_day_kaythree_handleBranch98(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_278"))
		{
			
			if (imperial_empire_day_kaythree_condition_isDomesticsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantDomesticsMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_282");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isEngineeringTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantEngineeringMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_284");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isMunitionsTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantMunitionsMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_286");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_kaythree_condition_isStructuresTrader (player, npc))
			{
				imperial_empire_day_kaythree_action_grantStructuresMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_288");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
				
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
			detachScript(self, "conversation.imperial_empire_day_kaythree");
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
		detachScript (self, "conversation.imperial_empire_day_kaythree");
		
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
		
		if (imperial_empire_day_kaythree_condition_isNeutralPlayer (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_291");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (imperial_empire_day_kaythree_condition_isRebelPlayer (player, npc))
		{
			doAnimationAction (npc, "backhand_threaten");
			
			string_id message = new string_id (c_stringFile, "s_50");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (imperial_empire_day_kaythree_condition_hasCompleted6thMission (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_145");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (imperial_empire_day_kaythree_condition_hasNotCraftedSupplies (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_138");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (imperial_empire_day_kaythree_condition_isEntertainerWaitingForSignal (player, npc))
		{
			imperial_empire_day_kaythree_action_entertainerSendSignalIncrementCollection (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_297");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_298");
				}
				
				utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 5);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "imperial_empire_day_kaythree", null, pp, responses);
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
		
		if (imperial_empire_day_kaythree_condition_isChampionKnowsKaythree (player, npc))
		{
			imperial_empire_day_kaythree_action_checkHolidayTimeStampOnPlayer (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_67");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (imperial_empire_day_kaythree_condition_isTrader (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (imperial_empire_day_kaythree_condition_isEntertainer (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_192");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_146");
				}
				
				utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 7);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "imperial_empire_day_kaythree", null, pp, responses);
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
		
		if (imperial_empire_day_kaythree_condition_alreadyKnowsKaythree (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_116");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (imperial_empire_day_kaythree_condition_isTrader (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (imperial_empire_day_kaythree_condition_isEntertainer (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_192");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_146");
				}
				
				utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 7);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "imperial_empire_day_kaythree", null, pp, responses);
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
		
		if (imperial_empire_day_kaythree_condition_isEnterainerOrTraderAndNeverMetKaythreeIsChampion (player, npc))
		{
			doAnimationAction (npc, "salute2");
			
			doAnimationAction (player, "salute2");
			
			string_id message = new string_id (c_stringFile, "s_131");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 85);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "imperial_empire_day_kaythree", null, pp, responses);
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
		
		if (imperial_empire_day_kaythree_condition_isEntertainerOrTraderAndNeverMetNotChampion (player, npc))
		{
			doAnimationAction (npc, "salute2");
			
			string_id message = new string_id (c_stringFile, "s_137");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_140");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_147");
				}
				
				utils.setScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId", 88);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "imperial_empire_day_kaythree", null, pp, responses);
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
		
		if (imperial_empire_day_kaythree_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_304");
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
		if (!conversationId.equals("imperial_empire_day_kaythree"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
		
		if (branchId == 5 && imperial_empire_day_kaythree_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && imperial_empire_day_kaythree_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && imperial_empire_day_kaythree_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && imperial_empire_day_kaythree_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && imperial_empire_day_kaythree_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && imperial_empire_day_kaythree_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 19 && imperial_empire_day_kaythree_handleBranch19 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 20 && imperial_empire_day_kaythree_handleBranch20 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 21 && imperial_empire_day_kaythree_handleBranch21 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 30 && imperial_empire_day_kaythree_handleBranch30 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 31 && imperial_empire_day_kaythree_handleBranch31 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 32 && imperial_empire_day_kaythree_handleBranch32 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 33 && imperial_empire_day_kaythree_handleBranch33 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 46 && imperial_empire_day_kaythree_handleBranch46 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 47 && imperial_empire_day_kaythree_handleBranch47 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 56 && imperial_empire_day_kaythree_handleBranch56 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 57 && imperial_empire_day_kaythree_handleBranch57 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 58 && imperial_empire_day_kaythree_handleBranch58 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 63 && imperial_empire_day_kaythree_handleBranch63 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 73 && imperial_empire_day_kaythree_handleBranch73 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 75 && imperial_empire_day_kaythree_handleBranch75 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 77 && imperial_empire_day_kaythree_handleBranch77 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 79 && imperial_empire_day_kaythree_handleBranch79 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 81 && imperial_empire_day_kaythree_handleBranch81 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 85 && imperial_empire_day_kaythree_handleBranch85 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 86 && imperial_empire_day_kaythree_handleBranch86 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 87 && imperial_empire_day_kaythree_handleBranch87 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 88 && imperial_empire_day_kaythree_handleBranch88 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 89 && imperial_empire_day_kaythree_handleBranch89 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 90 && imperial_empire_day_kaythree_handleBranch90 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 91 && imperial_empire_day_kaythree_handleBranch91 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 93 && imperial_empire_day_kaythree_handleBranch93 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 95 && imperial_empire_day_kaythree_handleBranch95 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 96 && imperial_empire_day_kaythree_handleBranch96 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 97 && imperial_empire_day_kaythree_handleBranch97 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 98 && imperial_empire_day_kaythree_handleBranch98 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.imperial_empire_day_kaythree.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
