package script.conversation;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.ai_lib;
import script.library.chat;
import script.library.conversation;
import script.library.groundquests;
import script.library.utils;


public class hx_138 extends script.base_script
{
	public hx_138()
	{
	}
	String c_stringFile = "conversation/hx_138";
	
	
	public boolean hx_138_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean hx_138_condition_firstMeeting(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.isTaskActive(player, "outbreak_quest_02_imperial", "findTroopers") || groundquests.isTaskActive(player, "outbreak_quest_02_rebel", "findTroopers") || groundquests.isTaskActive(player, "outbreak_quest_02_neutral", "findTroopers") );
	}
	
	
	public boolean hx_138_condition_hasRescueQuestActive(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.isTaskActive(player, "outbreak_quest_rescue_01", "spawnSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_01", "coaxingPleyer") || groundquests.isTaskActive(player, "outbreak_quest_rescue_02", "spawnSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_02", "coaxingPleyer") || groundquests.isTaskActive(player, "outbreak_quest_rescue_03", "spawnSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_03", "coaxingPleyer") || groundquests.isTaskActive(player, "outbreak_quest_rescue_04", "spawnSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_04", "coaxingPleyer") || groundquests.isTaskActive(player, "outbreak_quest_rescue_05", "spawnSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_05", "coaxingPleyer") || groundquests.isTaskActive(player, "outbreak_quest_rescue_06", "spawnSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_06", "coaxingPleyer") || groundquests.isTaskActive(player, "outbreak_quest_rescue_07", "spawnSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_07", "coaxingPleyer") || groundquests.isTaskActive(player, "outbreak_quest_rescue_08", "spawnSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_08", "coaxingPleyer") || groundquests.isTaskActive(player, "outbreak_quest_rescue_09", "spawnSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_09", "coaxingPleyer") || groundquests.isTaskActive(player, "outbreak_quest_rescue_010", "spawnSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_010", "coaxingPleyer") || groundquests.isTaskActive(player, "outbreak_quest_rescue_011", "spawnSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_012", "coaxingPleyer") || groundquests.isTaskActive(player, "outbreak_quest_rescue_013", "spawnSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_013", "coaxingPleyer") || groundquests.isTaskActive(player, "outbreak_quest_rescue_014", "spawnSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_014", "coaxingPleyer") || groundquests.isTaskActive(player, "outbreak_quest_rescue_015", "spawnSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_015", "coaxingPleyer") || groundquests.isTaskActive(player, "outbreak_quest_rescue_016", "spawnSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_016", "coaxingPleyer") );
	}
	
	
	public boolean hx_138_condition_failedRescueOne(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "outbreak_quest_rescue_01_fail");
	}
	
	
	public boolean hx_138_condition_failedRescueTwo(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "outbreak_quest_rescue_02_fail");
	}
	
	
	public boolean hx_138_condition_failedRescueThree(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "outbreak_quest_rescue_03_fail");
	}
	
	
	public boolean hx_138_condition_failedRescueFour(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "outbreak_quest_rescue_04_fail");
	}
	
	
	public boolean hx_138_condition_successRescueOne(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "outbreak_quest_rescue_01_success");
	}
	
	
	public boolean hx_138_condition_successRescueTwo(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "outbreak_quest_rescue_02_success");
	}
	
	
	public boolean hx_138_condition_successRescueThree(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "outbreak_quest_rescue_03_success");
	}
	
	
	public boolean hx_138_condition_successRescueFour(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "outbreak_quest_rescue_04_success");
	}
	
	
	public boolean hx_138_condition_hasShuttlePartQuestActive(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "outbreak_quest_shuttle_parts");
	}
	
	
	public boolean hx_138_condition_isStuckRescuing(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		
		return (groundquests.isTaskActive(player, "outbreak_quest_rescue_01", "rescueCompleted") || groundquests.isTaskActive(player, "outbreak_quest_rescue_02", "rescueCompleted") || groundquests.isTaskActive(player, "outbreak_quest_rescue_03", "rescueCompleted") || groundquests.isTaskActive(player, "outbreak_quest_rescue_04", "rescueCompleted") || groundquests.isTaskActive(player, "outbreak_quest_rescue_01", "speakSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_02", "speakSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_03", "speakSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_04", "speakSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_05", "rescueCompleted") || groundquests.isTaskActive(player, "outbreak_quest_rescue_06", "rescueCompleted") || groundquests.isTaskActive(player, "outbreak_quest_rescue_07", "rescueCompleted") || groundquests.isTaskActive(player, "outbreak_quest_rescue_08", "rescueCompleted") || groundquests.isTaskActive(player, "outbreak_quest_rescue_05", "speakSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_06", "speakSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_07", "speakSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_08", "speakSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_09", "rescueCompleted") || groundquests.isTaskActive(player, "outbreak_quest_rescue_010", "rescueCompleted") || groundquests.isTaskActive(player, "outbreak_quest_rescue_011", "rescueCompleted") || groundquests.isTaskActive(player, "outbreak_quest_rescue_012", "rescueCompleted") || groundquests.isTaskActive(player, "outbreak_quest_rescue_09", "speakSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_010", "speakSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_011", "speakSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_012", "speakSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_013", "rescueCompleted") || groundquests.isTaskActive(player, "outbreak_quest_rescue_014", "rescueCompleted") || groundquests.isTaskActive(player, "outbreak_quest_rescue_015", "rescueCompleted") || groundquests.isTaskActive(player, "outbreak_quest_rescue_016", "rescueCompleted") || groundquests.isTaskActive(player, "outbreak_quest_rescue_013", "speakSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_014", "speakSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_015", "speakSurvivor") || groundquests.isTaskActive(player, "outbreak_quest_rescue_016", "speakSurvivor") );
	}
	
	
	public boolean hx_138_condition_finishedRescueCanceledShuttle(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "outbreak_quest_rescue_04_success") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_shuttle_parts");
	}
	
	
	public boolean hx_138_condition_hasShuttlePartsNotMechanic(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "outbreak_quest_shuttle_parts", "deliverMechanic");
	}
	
	
	public boolean hx_138_condition_isImperialQuestPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "outbreak_quest_02_imperial");
	}
	
	
	public boolean hx_138_condition_isRebelQuestPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "outbreak_quest_02_rebel");
	}
	
	
	public boolean hx_138_condition_isNeutralQuestPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "outbreak_quest_02_neutral");
	}
	
	
	public boolean hx_138_condition_hasGivenPartsMechanic(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "outbreak_quest_shuttle_parts", "seeHX138");
	}
	
	
	public boolean hx_138_condition_readyFirstRescue(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, "outbreak_quest_02_imperial") || groundquests.hasCompletedQuest(player, "outbreak_quest_02_rebel") || groundquests.hasCompletedQuest(player, "outbreak_quest_02_neutral")) && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_01") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_05") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_06") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_07");
	}
	
	
	public boolean hx_138_condition_hasFinishedShuttleParts(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "outbreak_quest_shuttle_parts");
	}
	
	
	public boolean hx_138_condition_isPlayerGodMode(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return isGod(player);
	}
	
	
	public boolean hx_138_condition_hasCompletedRescue3Del4(obj_id player, obj_id npc) throws InterruptedException
	{
		return (groundquests.hasCompletedQuest(player, "outbreak_quest_rescue_03") || groundquests.hasCompletedQuest(player, "outbreak_quest_rescue_011") || groundquests.hasCompletedQuest(player, "outbreak_quest_rescue_012") || groundquests.hasCompletedQuest(player, "outbreak_quest_rescue_013")) && (!groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_04") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_014") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_015") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_016"));
	}
	
	
	public boolean hx_138_condition_hasCompletedRescue2Del3(obj_id player, obj_id npc) throws InterruptedException
	{
		return (groundquests.hasCompletedQuest(player, "outbreak_quest_rescue_02") || groundquests.hasCompletedQuest(player, "outbreak_quest_rescue_08") || groundquests.hasCompletedQuest(player, "outbreak_quest_rescue_09") || groundquests.hasCompletedQuest(player, "outbreak_quest_rescue_010")) && (!groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_03") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_011") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_012") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_013"));
	}
	
	
	public boolean hx_138_condition_hasCompletedRescue1Del2(obj_id player, obj_id npc) throws InterruptedException
	{
		return (groundquests.hasCompletedQuest(player, "outbreak_quest_rescue_01") || groundquests.hasCompletedQuest(player, "outbreak_quest_rescue_05") || groundquests.hasCompletedQuest(player, "outbreak_quest_rescue_06") || groundquests.hasCompletedQuest(player, "outbreak_quest_rescue_07")) && (!groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_02") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_08") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_09") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_010"));
	}
	
	
	public boolean hx_138_condition_isOnDungeonMissions(obj_id player, obj_id npc) throws InterruptedException
	{
		String faction = "";
		if (hx_138_condition_isImperialQuestPlayer(player,npc))
		{
			faction = "imperial";
		}
		else if (hx_138_condition_isRebelQuestPlayer(player,npc))
		{
			faction = "rebel";
		}
		else
		{
			faction = "neutral";
		}
		
		if (faction == null || faction.length() <= 0)
		{
			return false;
		}
		
		boolean hasAValidQuest = false;
		for (int i = 1; i <= 5; i++)
		{
			testAbortScript();
			if (!groundquests.isQuestActiveOrComplete(player, "outbreak_quest_administrative_building_" + faction + "_0" + i))
			{
				continue;
			}
			
			hasAValidQuest = true;
			break;
		}
		
		return hasAValidQuest;
	}
	
	
	public boolean hx_138_condition_shuttleEscape(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.isTaskActive(player, "outbreak_quest_facility_05_imperial", "seeHX138") || groundquests.isTaskActive(player, "outbreak_quest_facility_05_rebel", "seeHX138") || groundquests.isTaskActive(player, "outbreak_quest_facility_05_neutral", "seeHX138"));
	}
	
	
	public boolean hx_138_condition_hasCompletedRescue4DelShuttle(obj_id player, obj_id npc) throws InterruptedException
	{
		return (groundquests.hasCompletedQuest(player, "outbreak_quest_rescue_04") || groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_014") || groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_015") || groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_016") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_shuttle_parts"));
	}
	
	
	public boolean hx_138_condition_hasRadio(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "outbreak_radio_delivery_03", "findEpsilonContact");
	}
	
	
	public void hx_138_action_sendSignalMetHX138(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.sendSignal(player, "playerFoundStormtroopers");
	}
	
	
	public void hx_138_action_resetRescueOne(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.clearQuest(player, "outbreak_quest_rescue_01_fail");
		if (groundquests.isQuestActive(player, "outbreak_quest_rescue_01"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_01");
		}
		else if (groundquests.isQuestActive(player, "outbreak_quest_rescue_05"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_05");
		}
		else if (groundquests.isQuestActive(player, "outbreak_quest_rescue_06"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_06");
		}
		else if (groundquests.isQuestActive(player, "outbreak_quest_rescue_07"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_07");
		}
		
		hx_138_action_giveFirstSurvivorQuest(player,npc);
	}
	
	
	public void hx_138_action_giveSecondSurvivorQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.clearQuest(player, "outbreak_quest_rescue_01_success");
		
		int randNum = rand(1,4);
		if (randNum <= 0 || randNum > 4)
		{
			return;
		}
		switch(randNum)
		{
			case 1:
			groundquests.grantQuest(player, "outbreak_quest_rescue_02");
			break;
			case 2:
			groundquests.grantQuest(player, "outbreak_quest_rescue_08");
			break;
			case 3:
			groundquests.grantQuest(player, "outbreak_quest_rescue_09");
			break;
			case 4:
			groundquests.grantQuest(player, "outbreak_quest_rescue_010");
			break;
		}
	}
	
	
	public void hx_138_action_giveThirdSurvivorQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.clearQuest(player, "outbreak_quest_rescue_02_success");
		
		int randNum = rand(1,4);
		if (randNum <= 0 || randNum > 4)
		{
			return;
		}
		switch(randNum)
		{
			case 1:
			groundquests.grantQuest(player, "outbreak_quest_rescue_03");
			break;
			case 2:
			groundquests.grantQuest(player, "outbreak_quest_rescue_011");
			break;
			case 3:
			groundquests.grantQuest(player, "outbreak_quest_rescue_012");
			break;
			case 4:
			groundquests.grantQuest(player, "outbreak_quest_rescue_013");
			break;
		}
		
	}
	
	
	public void hx_138_action_giveFourthSurvivorQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.clearQuest(player, "outbreak_quest_rescue_03_success");
		
		int randNum = rand(1,4);
		if (randNum <= 0 || randNum > 4)
		{
			return;
		}
		switch(randNum)
		{
			case 1:
			groundquests.grantQuest(player, "outbreak_quest_rescue_04");
			break;
			case 2:
			groundquests.grantQuest(player, "outbreak_quest_rescue_014");
			break;
			case 3:
			groundquests.grantQuest(player, "outbreak_quest_rescue_015");
			break;
			case 4:
			groundquests.grantQuest(player, "outbreak_quest_rescue_016");
			break;
		}
	}
	
	
	public void hx_138_action_resetRescueTwo(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.clearQuest(player, "outbreak_quest_rescue_02_fail");
		
		if (groundquests.isQuestActive(player, "outbreak_quest_rescue_02"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_02");
		}
		else if (groundquests.isQuestActive(player, "outbreak_quest_rescue_08"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_08");
		}
		else if (groundquests.isQuestActive(player, "outbreak_quest_rescue_09"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_09");
		}
		else if (groundquests.isQuestActive(player, "outbreak_quest_rescue_010"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_010");
		}
		
		hx_138_action_giveSecondSurvivorQuest(player,npc);
	}
	
	
	public void hx_138_action_resetRescueThree(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.clearQuest(player, "outbreak_quest_rescue_03_fail");
		
		if (groundquests.isQuestActive(player, "outbreak_quest_rescue_03"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_03");
		}
		else if (groundquests.isQuestActive(player, "outbreak_quest_rescue_011"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_011");
		}
		else if (groundquests.isQuestActive(player, "outbreak_quest_rescue_012"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_012");
		}
		else if (groundquests.isQuestActive(player, "outbreak_quest_rescue_013"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_013");
		}
		
		hx_138_action_giveThirdSurvivorQuest(player,npc);
	}
	
	
	public void hx_138_action_resetRescueFour(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.clearQuest(player, "outbreak_quest_rescue_04_fail");
		
		if (groundquests.isQuestActive(player, "outbreak_quest_rescue_04"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_04");
		}
		else if (groundquests.isQuestActive(player, "outbreak_quest_rescue_014"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_014");
		}
		else if (groundquests.isQuestActive(player, "outbreak_quest_rescue_015"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_015");
		}
		else if (groundquests.isQuestActive(player, "outbreak_quest_rescue_016"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_016");
		}
		
		hx_138_action_giveFourthSurvivorQuest(player,npc);
	}
	
	
	public void hx_138_action_fixPlayerRescueQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if (groundquests.isTaskActive(player, "outbreak_quest_rescue_01", "rescueCompleted"))
		{
			hx_138_action_resetRescueOne (player, npc);
		}
		else if (groundquests.isTaskActive(player, "outbreak_quest_rescue_02", "rescueCompleted"))
		{
			hx_138_action_resetRescueTwo (player, npc);
		}
		else if (groundquests.isTaskActive(player, "outbreak_quest_rescue_03", "rescueCompleted"))
		{
			hx_138_action_resetRescueThree (player, npc);
		}
		else
		{
			hx_138_action_resetRescueFour (player, npc);
		}
	}
	
	
	public void hx_138_action_finishRescueGiveShuttleQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.clearQuest(player, "outbreak_quest_rescue_04_success");
		hx_138_action_giveShuttleQuest(player,npc);
	}
	
	
	public void hx_138_action_giveShuttleQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.grantQuest(player, "outbreak_quest_shuttle_parts");
	}
	
	
	public void hx_138_action_finishShuttleParts(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.sendSignal(player, "deliveredShuttleSupplies");
	}
	
	
	public void hx_138_action_giveFirstSurvivorQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		int randNum = rand(1,4);
		if (randNum <= 0 || randNum > 4)
		{
			return;
		}
		switch(randNum)
		{
			case 1:
			groundquests.grantQuest(player, "outbreak_quest_rescue_01");
			break;
			case 2:
			groundquests.grantQuest(player, "outbreak_quest_rescue_05");
			break;
			case 3:
			groundquests.grantQuest(player, "outbreak_quest_rescue_06");
			break;
			case 4:
			groundquests.grantQuest(player, "outbreak_quest_rescue_07");
			break;
		}
		
	}
	
	
	public void hx_138_action_giveRandomInquisitorQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		int randNumber = rand(1,5);
		if (groundquests.hasCompletedQuest(player, "outbreak_quest_02_imperial"))
		{
			groundquests.grantQuest(player, "outbreak_quest_administrative_building_imperial_0"+randNumber);
		}
		else if (groundquests.hasCompletedQuest(player, "outbreak_quest_02_rebel"))
		{
			groundquests.grantQuest(player, "outbreak_quest_administrative_building_rebel_0"+randNumber);
		}
		else
		{
			groundquests.grantQuest(player, "outbreak_quest_administrative_building_neutral_0"+randNumber);
		}
	}
	
	
	public void hx_138_action_giveInquisitorQuestOne(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if (groundquests.hasCompletedQuest(player, "outbreak_quest_02_imperial"))
		{
			groundquests.grantQuest(player, "outbreak_quest_administrative_building_imperial_01");
		}
		else if (groundquests.hasCompletedQuest(player, "outbreak_quest_02_rebel"))
		{
			groundquests.grantQuest(player, "outbreak_quest_administrative_building_rebel_01");
		}
		else
		{
			groundquests.grantQuest(player, "outbreak_quest_administrative_building_neutral_01");
		}
	}
	
	
	public void hx_138_action_giveInquisitorQuestTwo(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if (groundquests.hasCompletedQuest(player, "outbreak_quest_02_imperial"))
		{
			groundquests.grantQuest(player, "outbreak_quest_administrative_building_imperial_02");
		}
		else if (groundquests.hasCompletedQuest(player, "outbreak_quest_02_rebel"))
		{
			groundquests.grantQuest(player, "outbreak_quest_administrative_building_rebel_02");
		}
		else
		{
			groundquests.grantQuest(player, "outbreak_quest_administrative_building_neutral_02");
		}
	}
	
	
	public void hx_138_action_giveInquisitorQuestFive(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if (groundquests.hasCompletedQuest(player, "outbreak_quest_02_imperial"))
		{
			groundquests.grantQuest(player, "outbreak_quest_administrative_building_imperial_05");
		}
		else if (groundquests.hasCompletedQuest(player, "outbreak_quest_02_rebel"))
		{
			groundquests.grantQuest(player, "outbreak_quest_administrative_building_rebel_05");
		}
		else
		{
			groundquests.grantQuest(player, "outbreak_quest_administrative_building_neutral_05");
		}
	}
	
	
	public void hx_138_action_giveInquisitorQuestFour(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if (groundquests.hasCompletedQuest(player, "outbreak_quest_02_imperial"))
		{
			groundquests.grantQuest(player, "outbreak_quest_administrative_building_imperial_04");
		}
		else if (groundquests.hasCompletedQuest(player, "outbreak_quest_02_rebel"))
		{
			groundquests.grantQuest(player, "outbreak_quest_administrative_building_rebel_04");
		}
		else
		{
			groundquests.grantQuest(player, "outbreak_quest_administrative_building_neutral_04");
		}
	}
	
	
	public void hx_138_action_giveInquisitorQuestThree(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if (groundquests.hasCompletedQuest(player, "outbreak_quest_02_imperial"))
		{
			groundquests.grantQuest(player, "outbreak_quest_administrative_building_imperial_03");
		}
		else if (groundquests.hasCompletedQuest(player, "outbreak_quest_02_rebel"))
		{
			groundquests.grantQuest(player, "outbreak_quest_administrative_building_rebel_03");
		}
		else
		{
			groundquests.grantQuest(player, "outbreak_quest_administrative_building_neutral_03");
		}
	}
	
	
	public void hx_138_action_fixRescueGates(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		for (int i = 1; i <= 5; i++)
		{
			testAbortScript();
			if ((groundquests.isTaskActive(player, "outbreak_quest_rescue_0"+i, "rescueCompleted") || groundquests.isTaskActive(player, "outbreak_quest_rescue_0"+i, "speakSurvivor")))
			{
				groundquests.clearQuest(player, "outbreak_quest_rescue_0"+i);
				groundquests.grantQuest(player, "outbreak_quest_rescue_0"+i);
				return;
			}
		}
	}
	
	
	public void hx_138_action_godModeRescue01a(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_quest_rescue_01");
	}
	
	
	public void hx_138_action_godModeRescue01d(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_quest_rescue_07");
	}
	
	
	public void hx_138_action_godModeRescue01c(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_quest_rescue_06");
	}
	
	
	public void hx_138_action_godModeRescue01b(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_quest_rescue_05");
	}
	
	
	public void hx_138_action_godModeRescue02a(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_quest_rescue_02");
	}
	
	
	public void hx_138_action_godModeRescue02d(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_quest_rescue_010");
	}
	
	
	public void hx_138_action_godModeRescue02c(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_quest_rescue_09");
	}
	
	
	public void hx_138_action_godModeRescue02b(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_quest_rescue_08");
	}
	
	
	public void hx_138_action_godModeRescue03d(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_quest_rescue_013");
	}
	
	
	public void hx_138_action_godModeRescue03c(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_quest_rescue_012");
	}
	
	
	public void hx_138_action_godModeRescue03b(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_quest_rescue_011");
	}
	
	
	public void hx_138_action_godModeRescue03a(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_quest_rescue_03");
	}
	
	
	public void hx_138_action_godModeRescue04d(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_quest_rescue_016");
	}
	
	
	public void hx_138_action_godModeRescue04c(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_quest_rescue_015");
	}
	
	
	public void hx_138_action_godModeRescue04b(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_quest_rescue_014");
	}
	
	
	public void hx_138_action_godModeRescue04a(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_quest_rescue_04");
	}
	
	
	public int hx_138_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_122"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shrug_shoulders");
				
				string_id message = new string_id (c_stringFile, "s_123");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (hx_138_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_138");
					}
					
					utils.setScriptVar (player, "conversation.hx_138.branchId", 4);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.hx_138.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_177"))
		{
			
			if (hx_138_condition_isImperialQuestPlayer (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_178");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (hx_138_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_181");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_182");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_183");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_184");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_185");
					}
					
					utils.setScriptVar (player, "conversation.hx_138.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.hx_138.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (hx_138_condition_isRebelQuestPlayer (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_179");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (hx_138_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_191");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_192");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_193");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_194");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_195");
					}
					
					utils.setScriptVar (player, "conversation.hx_138.branchId", 16);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.hx_138.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (hx_138_condition_isNeutralQuestPlayer (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_180");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (hx_138_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_196");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_197");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_198");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_199");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_200");
					}
					
					utils.setScriptVar (player, "conversation.hx_138.branchId", 22);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.hx_138.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_138"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "laugh");
				
				string_id message = new string_id (c_stringFile, "s_139");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (hx_138_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_140");
					}
					
					utils.setScriptVar (player, "conversation.hx_138.branchId", 5);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.hx_138.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_140"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_141");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (hx_138_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.hx_138.branchId", 6);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.hx_138.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_142"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_143");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (hx_138_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_144");
					}
					
					utils.setScriptVar (player, "conversation.hx_138.branchId", 7);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.hx_138.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_144"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_145");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (hx_138_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_146");
					}
					
					utils.setScriptVar (player, "conversation.hx_138.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.hx_138.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_146"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "bow3");
				
				hx_138_action_giveRandomInquisitorQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_147");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
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
	
	
	public int hx_138_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_181"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_giveInquisitorQuestOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_186");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_182"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_giveInquisitorQuestTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_187");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_183"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_giveInquisitorQuestThree (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_188");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_184"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_giveInquisitorQuestFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_189");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_185"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_giveInquisitorQuestFive (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_190");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch16(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_191"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_giveInquisitorQuestOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_201");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_192"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_giveInquisitorQuestTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_202");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_193"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_giveInquisitorQuestThree (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_203");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_194"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_giveInquisitorQuestFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_204");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_195"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_giveInquisitorQuestFive (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_205");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch22(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_196"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_giveInquisitorQuestOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_206");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_197"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_giveInquisitorQuestTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_207");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_198"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_giveInquisitorQuestThree (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_208");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_199"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_giveInquisitorQuestFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_209");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_200"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_giveInquisitorQuestFive (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_210");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch32(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_104"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_fixRescueGates (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_105");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch34(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_63"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_65");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (hx_138_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_67");
					}
					
					utils.setScriptVar (player, "conversation.hx_138.branchId", 35);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.hx_138.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch35(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_67"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_69");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (hx_138_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_71");
					}
					
					utils.setScriptVar (player, "conversation.hx_138.branchId", 36);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.hx_138.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch36(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_71"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_73");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (hx_138_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_75");
					}
					
					utils.setScriptVar (player, "conversation.hx_138.branchId", 37);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.hx_138.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch37(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_75"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_77");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (hx_138_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_79");
					}
					
					utils.setScriptVar (player, "conversation.hx_138.branchId", 38);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.hx_138.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch38(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_79"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_finishRescueGiveShuttleQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_81");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch40(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_85"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_resetRescueFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_87");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
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
	
	
	public int hx_138_handleBranch42(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_91"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_93");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (hx_138_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_95");
					}
					
					utils.setScriptVar (player, "conversation.hx_138.branchId", 43);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.hx_138.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_99"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_giveFourthSurvivorQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_101");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_356"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_357");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_358");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_359");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_360");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_361");
					}
					
					utils.setScriptVar (player, "conversation.hx_138.branchId", 46);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.hx_138.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch43(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_95"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_giveFourthSurvivorQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_97");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
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
	
	
	public int hx_138_handleBranch46(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_358"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_godModeRescue04a (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_362");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_359"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_godModeRescue04b (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_364");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_360"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_godModeRescue04c (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_363");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_361"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_godModeRescue04d (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_365");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch51(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_121"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_resetRescueThree (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_125");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch53(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_129"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_131");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (hx_138_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_133");
					}
					
					utils.setScriptVar (player, "conversation.hx_138.branchId", 54);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.hx_138.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_345"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_346");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_347");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_349");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_350");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_351");
					}
					
					utils.setScriptVar (player, "conversation.hx_138.branchId", 56);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.hx_138.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch54(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_133"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_giveThirdSurvivorQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_135");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
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
	
	
	public int hx_138_handleBranch56(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_347"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_godModeRescue03a (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_348");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_349"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_godModeRescue03b (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_352");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_350"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_godModeRescue03c (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_353");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_351"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_godModeRescue03d (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_354");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch61(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_159"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_resetRescueTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_161");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch63(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_165"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_giveSecondSurvivorQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_167");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_320"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_321");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_325");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_334");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_338");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_342");
					}
					
					utils.setScriptVar (player, "conversation.hx_138.branchId", 65);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.hx_138.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch65(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_325"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_godModeRescue02a (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_326");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_334"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_godModeRescue02b (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_336");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_338"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_godModeRescue02c (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_340");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_342"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_godModeRescue02d (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_344");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch70(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_216"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shakefist");
				
				hx_138_action_resetRescueOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_218");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch77(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_305"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_307");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (hx_138_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_308");
					}
					
					utils.setScriptVar (player, "conversation.hx_138.branchId", 78);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.hx_138.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_310"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_311");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (hx_138_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_312");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_313");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_314");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_315");
					}
					
					utils.setScriptVar (player, "conversation.hx_138.branchId", 80);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.hx_138.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch78(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_308"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_giveFirstSurvivorQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_309");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch80(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_312"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_godModeRescue01a (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_316");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_313"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_godModeRescue01b (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_317");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_314"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_godModeRescue01c (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_318");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_315"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_godModeRescue01d (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_319");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int hx_138_handleBranch85(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_244"))
		{
			
			if (hx_138_condition__defaultCondition (player, npc))
			{
				hx_138_action_sendSignalMetHX138 (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_249");
				utils.removeScriptVar (player, "conversation.hx_138.branchId");
				
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
			detachScript(self, "conversation.hx_138");
		}
		
		setCondition (self, CONDITION_CONVERSABLE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setCondition (self, CONDITION_CONVERSABLE);
		
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
		detachScript (self, "conversation.hx_138");
		
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
		
		if (hx_138_condition_shuttleEscape (player, npc))
		{
			doAnimationAction (npc, "gesticulate_wildly");
			
			string_id message = new string_id (c_stringFile, "s_253");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (hx_138_condition_isOnDungeonMissions (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_251");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (hx_138_condition_hasFinishedShuttleParts (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_113");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (hx_138_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (hx_138_condition_isPlayerGodMode (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_122");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_177");
				}
				
				utils.setScriptVar (player, "conversation.hx_138.branchId", 3);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "hx_138", null, pp, responses);
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
		
		if (hx_138_condition_hasGivenPartsMechanic (player, npc))
		{
			doAnimationAction (npc, "celebrate");
			
			hx_138_action_finishShuttleParts (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_174");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (hx_138_condition_hasShuttlePartsNotMechanic (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_110");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (hx_138_condition_finishedRescueCanceledShuttle (player, npc))
		{
			hx_138_action_giveShuttleQuest (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_109");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (hx_138_condition_hasShuttlePartQuestActive (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_56");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (hx_138_condition_isStuckRescuing (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_103");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (hx_138_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_104");
				}
				
				utils.setScriptVar (player, "conversation.hx_138.branchId", 32);
				
				npcStartConversation (player, npc, "hx_138", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (hx_138_condition_successRescueFour (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_61");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (hx_138_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_63");
				}
				
				utils.setScriptVar (player, "conversation.hx_138.branchId", 34);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "hx_138", null, pp, responses);
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
		
		if (hx_138_condition_failedRescueFour (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_83");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (hx_138_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_85");
				}
				
				utils.setScriptVar (player, "conversation.hx_138.branchId", 40);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "hx_138", null, pp, responses);
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
		
		if (hx_138_condition_successRescueThree (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_89");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (hx_138_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (hx_138_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse1 = true;
			}
			
			boolean hasResponse2 = false;
			if (hx_138_condition_isPlayerGodMode (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_91");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_99");
				}
				
				if (hasResponse2)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_356");
				}
				
				utils.setScriptVar (player, "conversation.hx_138.branchId", 42);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "hx_138", null, pp, responses);
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
		
		if (hx_138_condition_failedRescueThree (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_119");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (hx_138_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_121");
				}
				
				utils.setScriptVar (player, "conversation.hx_138.branchId", 51);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "hx_138", null, pp, responses);
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
		
		if (hx_138_condition_successRescueTwo (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_127");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (hx_138_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (hx_138_condition_isPlayerGodMode (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_129");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_345");
				}
				
				utils.setScriptVar (player, "conversation.hx_138.branchId", 53);
				
				npcStartConversation (player, npc, "hx_138", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (hx_138_condition_failedRescueTwo (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_157");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (hx_138_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_159");
				}
				
				utils.setScriptVar (player, "conversation.hx_138.branchId", 61);
				
				npcStartConversation (player, npc, "hx_138", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (hx_138_condition_successRescueOne (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_163");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (hx_138_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (hx_138_condition_isPlayerGodMode (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_165");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_320");
				}
				
				utils.setScriptVar (player, "conversation.hx_138.branchId", 63);
				
				npcStartConversation (player, npc, "hx_138", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (hx_138_condition_failedRescueOne (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_214");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (hx_138_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_216");
				}
				
				utils.setScriptVar (player, "conversation.hx_138.branchId", 70);
				
				npcStartConversation (player, npc, "hx_138", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (hx_138_condition_hasRescueQuestActive (player, npc))
		{
			doAnimationAction (npc, "shrug_shoulders");
			
			string_id message = new string_id (c_stringFile, "s_220");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (hx_138_condition_hasCompletedRescue4DelShuttle (player, npc))
		{
			hx_138_action_giveShuttleQuest (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_255");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (hx_138_condition_hasCompletedRescue3Del4 (player, npc))
		{
			hx_138_action_giveFourthSurvivorQuest (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_245");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (hx_138_condition_hasCompletedRescue2Del3 (player, npc))
		{
			hx_138_action_giveThirdSurvivorQuest (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_246");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (hx_138_condition_hasCompletedRescue1Del2 (player, npc))
		{
			hx_138_action_giveSecondSurvivorQuest (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_247");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (hx_138_condition_readyFirstRescue (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_226");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (hx_138_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (hx_138_condition_isPlayerGodMode (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_305");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_310");
				}
				
				utils.setScriptVar (player, "conversation.hx_138.branchId", 77);
				
				npcStartConversation (player, npc, "hx_138", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (hx_138_condition_firstMeeting (player, npc))
		{
			doAnimationAction (npc, "nod");
			
			string_id message = new string_id (c_stringFile, "s_242");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (hx_138_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_244");
				}
				
				utils.setScriptVar (player, "conversation.hx_138.branchId", 85);
				
				npcStartConversation (player, npc, "hx_138", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (hx_138_condition_hasRadio (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_257");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (hx_138_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_258");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("hx_138"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.hx_138.branchId");
		
		if (branchId == 3 && hx_138_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 4 && hx_138_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && hx_138_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && hx_138_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && hx_138_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && hx_138_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && hx_138_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 16 && hx_138_handleBranch16 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 22 && hx_138_handleBranch22 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 32 && hx_138_handleBranch32 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 34 && hx_138_handleBranch34 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 35 && hx_138_handleBranch35 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 36 && hx_138_handleBranch36 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 37 && hx_138_handleBranch37 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 38 && hx_138_handleBranch38 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 40 && hx_138_handleBranch40 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 42 && hx_138_handleBranch42 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 43 && hx_138_handleBranch43 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 46 && hx_138_handleBranch46 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 51 && hx_138_handleBranch51 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 53 && hx_138_handleBranch53 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 54 && hx_138_handleBranch54 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 56 && hx_138_handleBranch56 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 61 && hx_138_handleBranch61 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 63 && hx_138_handleBranch63 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 65 && hx_138_handleBranch65 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 70 && hx_138_handleBranch70 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 77 && hx_138_handleBranch77 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 78 && hx_138_handleBranch78 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 80 && hx_138_handleBranch80 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 85 && hx_138_handleBranch85 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.hx_138.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
