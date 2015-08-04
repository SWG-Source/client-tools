package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.badge;
import script.library.factions;
import script.library.loot;
import script.library.money;
import script.library.prose;
import script.library.utils;
import script.library.weapons;
import script.library.xp;
import script.library.gcw;


public class groundquests extends script.base_script
{
	public groundquests()
	{
	}
	public static final String DISABLED_QUEST_TABLE = "datatables/quest/ground/disabled.iff";
	public static final String DISABLED_QUEST_COLUMN = "disabled_quests";
	
	public static final String questBaseObjVar = "quest";
	public static final String questAllObjVar = "all";
	public static final String questStaticEscortTargetObjVar = "staticEscortTarget";
	public static final String dot = ".";
	
	public static final String datatableColumnjournalEntryTitle = "JOURNAL_ENTRY_TITLE";
	public static final String datatableColumnShowSystemMessages = "SHOW_SYSTEM_MESSAGES";
	public static final String datatableColumnCategory = "CATEGORY";
	public static final String dataTableColumnTaskName = "TASK_NAME";
	public static final String dataTableColumnVisible = "IS_VISIBLE";
	public static final String dataTableColumnActionName = "ACTION_NAME";
	public static final String dataTableColumnQuestVisible = "VISIBLE";
	
	public static final int maxTasksPerQuest = 16;
	
	public static final String MUSIC_QUEST_ACTIVATED = "sound/ui_npe2_quest_received.snd";
	public static final String MUSIC_QUEST_COMPLETED = "sound/ui_npe2_quest_completed.snd";
	public static final String MUSIC_QUEST_RECEIVED_CREDITS = "sound/ui_npe2_quest_credits.snd";
	public static final String MUSIC_QUEST_RECEIVED_ITEM = "sound/ui_npe2_quest_reward.snd";
	public static final String MUSIC_QUEST_INCREMENT_COUNTER = "sound/ui_npe2_quest_step_completed.snd";
	
	public static final string_id SID_TASK_COMPLETED = new string_id( "quest/ground/system_message", "quest_task_completed");
	public static final string_id SID_TASK_FAILED = new string_id( "quest/ground/system_message", "quest_task_failed");
	public static final string_id SID_QUEST_RECEIVED = new string_id( "quest/ground/system_message", "quest_received");
	public static final string_id SID_PLACED_IN_INVENTORY = new string_id( "quest/ground/system_message", "placed_in_inventory");
	public static final string_id SID_MORE_THAN_ONE_PLACED_IN_INVENTORY = new string_id( "quest/ground/system_message", "placed_in_inventory_number");
	public static final string_id SID_REMOVED_FROM_INVENTORY = new string_id( "quest/ground/system_message", "removed_from_inventory");
	public static final string_id SID_TIME_REMAINING_BEFORE_COMPLETION = new string_id("quest/ground/system_message", "time_remaining_before_completion");
	public static final string_id SID_NO_XP_TRADER_ENTERTAINER = new string_id("quest/ground/system_message", "entertainer_trader_no_xp_combat");
	
	public static final String objvarGoingHome = "quest.static_escort.goingHome";
	public static final String objvarOnEscort = "quest.static_escort.onEscort";
	
	public static final String objvarGuaranteedSuccess_Count = "guaranteedSuccess.count";
	public static final String objvarGuaranteedSuccess_Target = "guaranteedSuccess.target";
	
	public static final String JOURNAL_UPDATED_MUSIC = "sound/ui_journal_updated.snd";
	
	public static final int INVALID_QUEST = 0;
	
	public static final int PERFORM_ANY = 0;
	public static final int PERFORM_DANCE = 1;
	public static final int PERFORM_MUSIC = 2;
	public static final int PERFORM_JUGGLE = 3;
	
	public static final String PERFORM_TYPE = "performType";
	
	public static final String QUEST_EXPERIENCE_TABLE = "datatables/quest/quest_experience.iff";
	
	public static final String BASE_NAME = "quest.";
	public static final String QUEST_CRC = BASE_NAME + "questCrc";
	public static final String QUEST_NAME = BASE_NAME + "questName";
	public static final String TASK_ID = BASE_NAME + "taskId";
	public static final String TASK_NAME = BASE_NAME + "taskName";
	
	public static final String dataTableColumnExperienceType = "EXPERIENCE_TYPE";
	public static final String dataTableColumnExperienceAmount = "EXPERIENCE_AMOUNT";
	public static final String dataTableColumnFactionName = "FACTION_NAME";
	public static final String dataTableColumnFactionAmount = "FACTION_AMOUNT";
	public static final String dataTableColumnGcwGrant = "GRANT_GCW";
	public static final String dataTableColumnGcwOverwriteAmt = "QUEST_GCW_REWARD_OVERWRITE_AMOUNT";
	public static final String dataTableColumnGcwOvrwrtSFMod = "QUEST_GCW_REWARD_OVERWRITE_SF_MODIFIER";
	public static final String dataTableColumnGcwRebReward = "QUEST_GCW_REBEL_REWARD_LOOT_NAME";
	public static final String dataTableColumnGcwRebRewardCnt = "QUEST_REWARD_REBEL_LOOT_COUNT";
	public static final String dataTableColumnGcwImpReward = "QUEST_GCW_IMPERIAL_REWARD_LOOT_NAME";
	public static final String dataTableColumnGcwImpRewardCnt = "QUEST_REWARD_IMPERIAL_LOOT_COUNT";
	public static final String dataTableColumnGcwRewardMultip = "QUEST_GCW_REWARD_COUNT_SF_MULTIPLIER";
	public static final String dataTableColumnBankCredits = "BANK_CREDITS";
	public static final String dataTableColumnItem = "ITEM";
	public static final String dataTableColumnCount = "COUNT";
	public static final String dataTableColumnWeapon = "WEAPON";
	public static final String dataTableColumnCountWeapon = "COUNT_WEAPON";
	public static final String dataTableColumnSpeed = "SPEED";
	public static final String dataTableColumnDamage = "DAMAGE";
	public static final String dataTableColumnEfficiency = "EFFICIENCY";
	public static final String dataTableColumnElementalValue = "ELEMENTAL_VALUE";
	public static final String dataTableColumnArmor = "ARMOR";
	public static final String dataTableColumnCountArmor = "COUNT_ARMOR";
	public static final String dataTableColumnQuality = "QUALITY";
	public static final String dataTableColumnLootName = "LOOT_NAME";
	public static final String dataTableColumnLootCount = "LOOT_COUNT";
	public static final String dataTableColumnLevel = "LEVEL";
	public static final String dataTableColumnTier = "TIER";
	
	public static final String dataTableColumnQuestRewardExperienceTeir1 = "TIER_1_EXPERIENCE";
	public static final String dataTableColumnQuestRewardExperienceTeir2 = "TIER_2_EXPERIENCE";
	public static final String dataTableColumnQuestRewardExperienceTeir3 = "TIER_3_EXPERIENCE";
	public static final String dataTableColumnQuestRewardExperienceTeir4 = "TIER_4_EXPERIENCE";
	public static final String dataTableColumnQuestRewardExperienceTeir5 = "TIER_5_EXPERIENCE";
	public static final String dataTableColumnQuestRewardExperienceTeir6 = "TIER_6_EXPERIENCE";
	public static final String dataTableColumnQuestRewardExperienceType = "QUEST_REWARD_EXPERIENCE_TYPE";
	public static final String dataTableColumnQuestRewardExperienceAmount = "QUEST_REWARD_EXPERIENCE_AMOUNT";
	public static final String dataTableColumnQuestRewardFactionName = "QUEST_REWARD_FACTION_NAME";
	public static final String dataTableColumnQuestRewardFactionAmount = "QUEST_REWARD_FACTION_AMOUNT";
	public static final String dataTableColumnQuestRewardBankCredits = "QUEST_REWARD_BANK_CREDITS";
	public static final String dataTableColumnQuestRewardItem = "QUEST_REWARD_ITEM";
	public static final String dataTableColumnQuestRewardCount = "QUEST_REWARD_COUNT";
	public static final String dataTableColumnQuestRewardWeapon = "QUEST_REWARD_WEAPON";
	public static final String dataTableColumnQuestRewardCountWeapon = "QUEST_REWARD_COUNT_WEAPON";
	public static final String dataTableColumnQuestRewardSpeed = "QUEST_REWARD_SPEED";
	public static final String dataTableColumnQuestRewardDamage = "QUEST_REWARD_DAMAGE";
	public static final String dataTableColumnQuestRewardEfficiency = "QUEST_REWARD_EFFICIENCY";
	public static final String dataTableColumnQuestRewardElementalValue = "QUEST_REWARD_ELEMENTAL_VALUE";
	public static final String dataTableColumnQuestRewardArmor = "QUEST_REWARD_ARMOR";
	public static final String dataTableColumnQuestRewardCountArmor = "QUEST_REWARD_COUNT_ARMOR";
	public static final String dataTableColumnQuestRewardQuality = "QUEST_REWARD_QUALITY";
	public static final String dataTableColumnQuestRewardLootName = "QUEST_REWARD_LOOT_NAME";
	public static final String dataTableColumnQuestRewardLootCount = "QUEST_REWARD_LOOT_COUNT";
	public static final String dataTableColumnQuestRewardLootName2 = "QUEST_REWARD_LOOT_NAME_2";
	public static final String dataTableColumnQuestRewardLootCount2 = "QUEST_REWARD_LOOT_COUNT_2";
	public static final String dataTableColumnQuestRewardLootName3 = "QUEST_REWARD_LOOT_NAME_3";
	public static final String dataTableColumnQuestRewardLootCount3 = "QUEST_REWARD_LOOT_COUNT_3";
	public static final String dataTableColumnQuestRewardExclusiveLootName = "QUEST_REWARD_EXCLUSIVE_LOOT_NAME";
	public static final String dataTableColumnQuestRewardExclusiveLootCount = "QUEST_REWARD_EXCLUSIVE_LOOT_COUNT";
	public static final String dataTableColumnQuestRewardExclusiveLootName2 = "QUEST_REWARD_EXCLUSIVE_LOOT_NAME_2";
	public static final String dataTableColumnQuestRewardExclusiveLootCount2 = "QUEST_REWARD_EXCLUSIVE_LOOT_COUNT_2";
	public static final String dataTableColumnQuestRewardExclusiveLootName3 = "QUEST_REWARD_EXCLUSIVE_LOOT_NAME_3";
	public static final String dataTableColumnQuestRewardExclusiveLootCount3 = "QUEST_REWARD_EXCLUSIVE_LOOT_COUNT_3";
	public static final String dataTableColumnQuestRewardExclusiveLootName4 = "QUEST_REWARD_EXCLUSIVE_LOOT_NAME_4";
	public static final String dataTableColumnQuestRewardExclusiveLootCount4 = "QUEST_REWARD_EXCLUSIVE_LOOT_COUNT_4";
	public static final String dataTableColumnQuestRewardExclusiveLootName5 = "QUEST_REWARD_EXCLUSIVE_LOOT_NAME_5";
	public static final String dataTableColumnQuestRewardExclusiveLootCount5 = "QUEST_REWARD_EXCLUSIVE_LOOT_COUNT_5";
	public static final String dataTableColumnQuestRewardExclusiveLootName6 = "QUEST_REWARD_EXCLUSIVE_LOOT_NAME_6";
	public static final String dataTableColumnQuestRewardExclusiveLootCount6 = "QUEST_REWARD_EXCLUSIVE_LOOT_COUNT_6";
	public static final String dataTableColumnQuestRewardExclusiveLootName7 = "QUEST_REWARD_EXCLUSIVE_LOOT_NAME_7";
	public static final String dataTableColumnQuestRewardExclusiveLootCount7 = "QUEST_REWARD_EXCLUSIVE_LOOT_COUNT_7";
	public static final String dataTableColumnQuestRewardExclusiveLootName8 = "QUEST_REWARD_EXCLUSIVE_LOOT_NAME_8";
	public static final String dataTableColumnQuestRewardExclusiveLootCount8 = "QUEST_REWARD_EXCLUSIVE_LOOT_COUNT_8";
	public static final String dataTableColumnQuestRewardExclusiveLootName9 = "QUEST_REWARD_EXCLUSIVE_LOOT_NAME_9";
	public static final String dataTableColumnQuestRewardExclusiveLootCount9 = "QUEST_REWARD_EXCLUSIVE_LOOT_COUNT_9";
	public static final String dataTableColumnQuestRewardExclusiveLootName10 = "QUEST_REWARD_EXCLUSIVE_LOOT_NAME_10";
	public static final String dataTableColumnQuestRewardExclusiveLootCount10 = "QUEST_REWARD_EXCLUSIVE_LOOT_COUNT_10";
	
	public static final String dataTableColumnQuestPenaltyFactionName = "QUEST_PENALTY_FACTION_NAME";
	public static final String dataTableColumnQuestPenaltyFactionAmount = "QUEST_PENALTY_FACTION_AMOUNT";
	
	public static final String dataTableColumnConditionQuestGrantQuest = "CONDITIONAL_QUEST_GRANT_QUEST";
	public static final String dataTableColumnConditionQuestGrantListOfCompletedQuests = "CONDITIONAL_QUEST_GRANT_QUEST_LIST_OF_COMPLETED_QUESTS";
	public static final String dataTableColumnBadge = "REWARD_BADGE";
	
	public static final String dataTableColumnGuaranteedSuccessMin = "GUARANTEED_SUCCESS_MIN";
	public static final String dataTableColumnGuaranteedSuccessMax = "GUARANTEED_SUCCESS_MAX";
	
	public static final String dataTableColumnRemainingTime = "REMAINING_TIME";
	
	public static final String dataTableColumnCreateWaypoint = "CREATE_WAYPOINT";
	public static final String dataTableColumnPlanetName = "PLANET_NAME";
	public static final String dataTableColumnLocationX = "LOCATION_X";
	public static final String dataTableColumnLocationY = "LOCATION_Y";
	public static final String dataTableColumnLocationZ = "LOCATION_Z";
	public static final String dataTableColumnWaypointName = "WAYPOINT_NAME";
	public static final String dataTableColumnInteriorWaypointAppearance = "INTERIOR_WAYPOINT_APPEARANCE";
	public static final String dataTableColumnWaypointBuildingCellName = "WAYPOINT_BUILDING_CELL_NAME";
	public static final String dataTableColumnEntranceCreateWaypoint = "CREATE_ENTRANCE_WAYPOINT";
	public static final String dataTableColumnEntranceLocationX = "ENTRANCE_LOCATION_X";
	public static final String dataTableColumnEntranceLocationY = "ENTRANCE_LOCATION_Y";
	public static final String dataTableColumnEntranceLocationZ = "ENTRANCE_LOCATION_Z";
	public static final String dataTableColumnEntranceWaypointName = "ENTRANCE_WAYPOINT_NAME";
	public static final String taskType = "base_task";
	public static final String objvarWaypoint = "waypoint";
	public static final String objvarEntranceWaypoint = "entranceWaypoint";
	public static final String objvarWaypointInActive = "waypointInactive";
	public static final String timeObjVar = "playedTimeEnd";
	
	
	public static String getStringDataEntry(String datatableName, int questId, int taskId, String columnName) throws InterruptedException
	{
		String result = null;
		String questName = questGetQuestName(questId);
		if (questName == null || questName.equals(""))
		{
			String errText = "QUEST NAME IS NULL: datatableName: "+ datatableName +
			" - questId:"+ questId +
			" - taskId: "+ taskId +
			" - columnName: "+ columnName;
			
			logScriptDataError(errText);
		}
		else
		{
			String datatable = "datatables/"+ datatableName + "/"+ questName + ".iff";
			if (isValidTaskId(taskId))
			{
				if (dataTableHasColumn(datatable, columnName))
				{
					result = dataTableGetString(datatable, taskId, columnName);
				}
			}
		}
		return result;
	}
	
	
	public static int getIntDataEntry(String datatableName, int questId, int taskId, String columnName) throws InterruptedException
	{
		int result = -1;
		String questName = questGetQuestName(questId);
		String datatable = "datatables/"+ datatableName + "/"+ questName + ".iff";
		if (isValidTaskId(taskId))
		{
			if (dataTableHasColumn(datatable, columnName))
			{
				result = dataTableGetInt(datatable, taskId, columnName);
			}
		}
		return result;
	}
	
	
	public static float getFloatDataEntry(String datatableName, int questId, int taskId, String columnName) throws InterruptedException
	{
		float result = 0.0f;
		String questName = questGetQuestName(questId);
		String datatable = "datatables/"+ datatableName + "/"+ questName + ".iff";
		if (isValidTaskId(taskId))
		{
			if (dataTableHasColumn(datatable, columnName))
			{
				result = dataTableGetFloat(datatable, taskId, columnName);
			}
		}
		return result;
	}
	
	
	public static boolean getBoolDataEntry(String datatableName, int questId, int taskId, String columnName, boolean defaultValue) throws InterruptedException
	{
		int result = -1;
		String questName = questGetQuestName(questId);
		String datatable = "datatables/"+ datatableName + "/"+ questName + ".iff";
		if (isValidTaskId(taskId))
		{
			if (dataTableHasColumn(datatable, columnName))
			{
				result = dataTableGetInt(datatable, taskId, columnName);
			}
		}
		
		if (result == -1)
		{
			return defaultValue;
		}
		
		return result != 0;
	}
	
	
	public static String setBaseObjVar(obj_id player, String taskType, String questName, int taskId) throws InterruptedException
	{
		String objvar = questBaseObjVar + dot + taskType + dot + questName + dot + taskId;
		setObjVar(player, objvar + dot + "inProgress", "true");
		return objvar;
	}
	
	
	public static void clearBaseObjVar(obj_id player, String taskType, String questName, int taskId) throws InterruptedException
	{
		String objvarTask = questBaseObjVar + dot + taskType + dot + questName + dot + taskId;
		removeObjVar(player, objvarTask);
	}
	
	
	public static String getBaseObjVar(obj_id player, String taskType, String questName, int taskId) throws InterruptedException
	{
		return questBaseObjVar + dot + taskType + dot + questName + dot + taskId;
	}
	
	
	public static String getTaskTypeObjVar(obj_id player, String taskType) throws InterruptedException
	{
		String objvar = questBaseObjVar + dot + taskType;
		if (hasObjVar(player, objvar))
		{
			return objvar;
		}
		else
		{
			return null;
		}
	}
	
	
	public static String getTaskTypeObjVar(String taskType) throws InterruptedException
	{
		return questBaseObjVar + dot + taskType;
	}
	
	
	public static void questOutputDebugInfo(obj_id player, int questCrc, int taskId, String taskType, String method, String message) throws InterruptedException
	{
		if (isGod(player) && questGetDebugging())
		{
			debugSpeakMsg (player, "Quest: player["+ player + "], "+ taskType + dot + method + ", "+ questGetQuestName(questCrc) + ":task"+ taskId + " - "+ message);
		}
	}
	
	
	public static void questOutputDebugInfo(obj_id player, String questName, int taskId, String taskType, String method, String message) throws InterruptedException
	{
		questOutputDebugInfo(player, questGetQuestId(questName), taskId, taskType, method, message);
	}
	
	
	public static void questOutputDebugInfo(obj_id player, String taskType, String method, String message) throws InterruptedException
	{
		if (isGod(player) && questGetDebugging())
		{
			debugSpeakMsg (player, "Quest: player["+ player + "], "+ taskType + dot + method + " - "+ message);
		}
	}
	
	
	public static void questOutputDebugLog(String taskType, String method, String message) throws InterruptedException
	{
		if (questGetDebugging())
		{
			LOG("QUEST_DEBUG", taskType + dot + method + " - "+ message);
		}
	}
	
	
	public static void addDestroyNotification(obj_id target, obj_id player) throws InterruptedException
	{
		attachScript(target, "quest.utility.destroy_notification");
		dictionary params = new dictionary();
		params.put("target", player);
		params.put("parameter", "");
		messageTo(target, "addNotification", params, 0.0f, false);
	}
	
	
	public static void failAllActiveTasksOfType(obj_id player, String taskType) throws InterruptedException
	{
		dictionary tasks = getActiveTasksForTaskType(player, taskType);
		if ((tasks != null) && !tasks.isEmpty())
		{
			java.util.Enumeration keys = tasks.keys();
			while (keys.hasMoreElements())
			{
				testAbortScript();
				String questCrcString = (String)keys.nextElement();
				int questCrc = utils.stringToInt(questCrcString);
				int[] tasksForCurrentQuest = tasks.getIntArray(questCrcString);
				
				for (int i = 0; i < tasksForCurrentQuest.length; ++i)
				{
					testAbortScript();
					int taskId = tasksForCurrentQuest[i];
					
					if (isValidTaskId(taskId))
					{
						questFailTask(questCrc, taskId, player);
					}
				}
			}
		}
	}
	
	
	public static String getTaskStringDataEntry(int questId, int taskId, String columnName) throws InterruptedException
	{
		return getStringDataEntry("questtask", questId, taskId, columnName);
	}
	
	
	public static int getTaskIntDataEntry(int questId, int taskId, String columnName) throws InterruptedException
	{
		return getIntDataEntry("questtask", questId, taskId, columnName);
	}
	
	
	public static float getTaskFloatDataEntry(int questId, int taskId, String columnName) throws InterruptedException
	{
		return getFloatDataEntry("questtask", questId, taskId, columnName);
	}
	
	
	public static boolean getTaskBoolDataEntry(int questId, int taskId, String columnName, boolean defaultValue) throws InterruptedException
	{
		return getBoolDataEntry("questtask", questId, taskId, columnName, defaultValue);
	}
	
	
	public static String getQuestStringDataEntry(int questId, String columnName) throws InterruptedException
	{
		return getStringDataEntry("questlist", questId, 0, columnName);
	}
	
	
	public static int getQuestIntDataEntry(int questId, String columnName) throws InterruptedException
	{
		return getIntDataEntry("questlist", questId, 0, columnName);
	}
	
	
	public static float getQuestFloatDataEntry(int questId, String columnName) throws InterruptedException
	{
		return getFloatDataEntry("questlist", questId, 0, columnName);
	}
	
	
	public static boolean getQuestBoolDataEntry(int questId, String columnName, boolean defaultValue) throws InterruptedException
	{
		return getBoolDataEntry("questlist", questId, 0, columnName, defaultValue);
	}
	
	
	public static boolean getShowSystemMessages(int questCrc, int taskId) throws InterruptedException
	{
		return getTaskIntDataEntry(questCrc, taskId, datatableColumnShowSystemMessages) > 0;
	}
	
	
	public static void sendGroundQuestSystemMessage(string_id message, int questCrc, int taskId, obj_id player) throws InterruptedException
	{
		String taskTitle = getTaskStringDataEntry(questCrc, taskId, datatableColumnjournalEntryTitle);
		
		prose_package pp = prose.getPackage(message);
		prose.setTO(pp, taskTitle);
		sendQuestSystemMessage(player, pp);
	}
	
	
	public static void sendTaskCompletedSystemMessage(int questCrc, int taskId, obj_id player) throws InterruptedException
	{
		if (getShowSystemMessages(questCrc, taskId))
		{
			sendGroundQuestSystemMessage(SID_TASK_COMPLETED, questCrc, taskId, player);
		}
	}
	
	
	public static void sendTaskFailedSystemMessage(int questCrc, int taskId, obj_id player) throws InterruptedException
	{
		if (getShowSystemMessages(questCrc, taskId))
		{
			sendGroundQuestSystemMessage(SID_TASK_FAILED, questCrc, taskId, player);
		}
	}
	
	
	public static void requestGrantQuest(obj_id player, String questName) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		requestGrantQuest(questNum, player, player, true);
	}
	
	
	public static void requestGrantQuest(obj_id[] players, String questName) throws InterruptedException
	{
		if (players == null || players.length == 0)
		{
			return;
		}
		
		for (int i=0; i<players.length; i++)
		{
			testAbortScript();
			if (!isIdValid(players[i]) || !exists(players[i]))
			{
				continue;
			}
			
			requestGrantQuest(players[i], questName);
		}
	}
	
	
	public static void requestGrantQuest(int questCrc, obj_id player, obj_id npc) throws InterruptedException
	{
		requestGrantQuest(questCrc, player, npc, false);
	}
	
	
	public static void requestGrantQuest(obj_id player, String questName, boolean showMessage) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		requestGrantQuest(questNum, player, player, showMessage);
	}
	
	
	public static void requestGrantQuest(int questCrc, obj_id player, obj_id npc, boolean showSystemMessage) throws InterruptedException
	{
		if (utils.isProfession(player, utils.TRADER) || utils.isProfession(player, utils.ENTERTAINER))
		{
			doTraderEntertainerXpWarning(questCrc, player);
		}
		requestActivateQuest(questCrc, player, npc);
		return;
	}
	
	
	public static int grantQuest(obj_id player, String questName) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		requestGrantQuest(questNum, player, player, true);
		return 1;
	}
	
	
	public static int grantQuest(int questCrc, obj_id player, obj_id npc) throws InterruptedException
	{
		requestGrantQuest(questCrc, player, npc, false);
		return 1;
	}
	
	
	public static int grantQuest(obj_id player, String questName, boolean showMessage) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		requestGrantQuest(questNum, player, player, showMessage);
		return 1;
	}
	
	
	public static int grantQuest(int questCrc, obj_id player, obj_id npc, boolean showSystemMessage) throws InterruptedException
	{
		if (utils.isProfession(player, utils.TRADER) || utils.isProfession(player, utils.ENTERTAINER))
		{
			doTraderEntertainerXpWarning(questCrc, player);
		}
		requestActivateQuest(questCrc, player, npc);
		return 1;
	}
	
	
	public static void doTraderEntertainerXpWarning(int questCrc, obj_id player) throws InterruptedException
	{
		String xpType = "";
		int xpAmount = -1;
		
		xpType = getQuestStringDataEntry (questCrc, dataTableColumnQuestRewardExperienceType);
		xpAmount = getQuestIntDataEntry (questCrc, dataTableColumnQuestRewardExperienceAmount);
		
		if (xpType != null && !xpType.equals("") && xp.isCombatXpType(xpType) && xpAmount > 0)
		{
			sendSystemMessage(player, SID_NO_XP_TRADER_ENTERTAINER);
		}
		
		return;
	}
	
	
	public static int grantQuestNoAcceptUI(obj_id player, String questName, boolean showSystemMessage) throws InterruptedException
	{
		int questCrc = getQuestIdFromString(questName);
		return questActivateQuest(questCrc, player, player);
	}
	
	
	public static boolean isInNamedRegion(obj_id player, String requiredRegion) throws InterruptedException
	{
		if (requiredRegion == null || requiredRegion.length() == 0)
		{
			return true;
		}
		
		location here = getLocation (player);
		region[] regions = getRegionsAtPoint(here);
		if (regions != null)
		{
			for (int k = 0; k < regions.length; k++)
			{
				testAbortScript();
				String regionWeAreIn = regions[k].getName();
				
				if (regionWeAreIn.equals(requiredRegion))
				{
					return true;
				}
			}
		}
		return false;
	}
	
	
	public static void sendSignal(obj_id player, String signalName) throws InterruptedException
	{
		dictionary params = new dictionary();
		params.put("signal", signalName);
		messageTo(player, "questSignal", params, 0, false);
	}
	
	
	public static void sendSignal(obj_id[] players, String signalName) throws InterruptedException
	{
		if (players == null || players.length == 0)
		{
			return;
		}
		
		for (int i=0; i<players.length; i++)
		{
			testAbortScript();
			if (isIdValid(players[i]) && exists(players[i]))
			{
				sendSignal(players[i], signalName);
			}
		}
	}
	
	
	public static void sendSignals(obj_id player, String[] signalNames) throws InterruptedException
	{
		if (signalNames == null || signalNames.length == 0 || !isIdValid(player))
		{
			return;
		}
		
		for (int i=0; i < signalNames.length; ++i)
		{
			testAbortScript();
			if (signalNames[i] != null && signalNames[i].length() > 0)
			{
				sendSignal(player, signalNames[i]);
			}
		}
	}
	
	
	public static boolean isQuestDisabled(String questName) throws InterruptedException
	{
		if (questName.startsWith("quest/"))
		{
			questName = questName.substring(6);
		}
		
		int disabled = dataTableSearchColumnForString(questName, DISABLED_QUEST_COLUMN, DISABLED_QUEST_TABLE);
		
		if (disabled >= 0)
		{
			return true;
		}
		return false;
	}
	
	
	public static int getQuestIdFromString(String questName) throws InterruptedException
	{
		
		if (questName.startsWith("quest/"))
		{
			return questGetQuestId(questName);
		}
		else
		{
			String fullPathQuestName = "quest/"+ questName;
			return questGetQuestId(fullPathQuestName);
		}
	}
	
	
	public static boolean hasCompletedQuest(obj_id player, String questName) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		return questIsQuestComplete(questNum, player);
	}
	
	
	public static int canActivateQuest(obj_id player, String questName) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		return questCanActivateQuest(questNum, player);
	}
	
	
	public static void completeQuest(obj_id player, String questName) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		requestCompleteQuest(questNum, player);
	}
	
	
	public static void clearQuest(obj_id player, String questName) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		questClearQuest(questNum, player);
	}
	
	
	public static void clearQuest(obj_id[] players, String questName) throws InterruptedException
	{
		if (players == null || players.length == 0)
		{
			return;
		}
		
		for (int i=0; i<players.length; i++)
		{
			testAbortScript();
			if (!isIdValid(players[i]) || !exists(players[i]))
			{
				continue;
			}
			
			if (isQuestActiveOrComplete(players[i], questName))
			{
				clearQuest(players[i], questName);
			}
		}
	}
	
	
	public static boolean isQuestActive(obj_id player, String questName) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		return questIsQuestActive(questNum, player);
	}
	
	
	public static boolean isQuestActiveOrComplete(obj_id player, String questName) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		return questIsQuestActive(questNum, player) || questIsQuestComplete(questNum, player);
	}
	
	
	public static boolean hasCompletedTask(obj_id player, String questName, int taskNum) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		return questIsTaskComplete(questNum, taskNum, player);
	}
	
	
	public static boolean isTaskActive(obj_id player, String questName, int taskNum) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		return questIsTaskActive(questNum, taskNum, player);
	}
	
	
	public static void activateTask(obj_id player, String questName, int taskNum) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		if (isValidQuestName(questName) && isValidTaskId(taskNum))
		{
			questActivateTask(questNum, taskNum, player);
		}
	}
	
	
	public static void completeTask(obj_id player, String questName, int taskNum) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		if (isValidQuestName(questName) && isValidTaskId(taskNum))
		{
			questCompleteTask(questNum, taskNum, player);
		}
	}
	
	
	public static void failTask(obj_id player, String questName, int taskNum) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		if (isValidQuestName(questName) && isValidTaskId(taskNum))
		{
			questFailTask(questNum, taskNum, player);
		}
	}
	
	
	public static void clearTask(obj_id player, String questName, int taskNum) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		if (isValidQuestName(questName) && isValidTaskId(taskNum))
		{
			questClearQuestTask(questNum, taskNum, player);
		}
	}
	
	
	public static boolean hasCompletedTask(obj_id player, String questName, String taskName) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		int taskNum = getTaskId(questNum, taskName);
		return questIsTaskComplete(questNum, taskNum, player);
	}
	
	
	public static boolean isTaskActive(obj_id player, String questName, String taskName) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		int taskNum = getTaskId(questNum, taskName);
		return questIsTaskActive(questNum, taskNum, player);
	}
	
	
	public static void activateTask(obj_id player, String questName, String taskName) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		int taskNum = getTaskId(questNum, taskName);
		if (isValidQuestName(questName) && isValidTaskId(taskNum))
		{
			questActivateTask(questNum, taskNum, player);
		}
	}
	
	
	public static void completeTask(obj_id player, String questName, String taskName) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		int taskNum = getTaskId(questNum, taskName);
		if (isValidQuestName(questName) && isValidTaskId(taskNum))
		{
			questCompleteTask(questNum, taskNum, player);
		}
	}
	
	
	public static void failTask(obj_id player, String questName, String taskName) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		int taskNum = getTaskId(questNum, taskName);
		if (isValidQuestName(questName) && isValidTaskId(taskNum))
		{
			questFailTask(questNum, taskNum, player);
		}
	}
	
	
	public static void clearTask(obj_id player, String questName, String taskName) throws InterruptedException
	{
		int questNum = getQuestIdFromString(questName);
		int taskNum = getTaskId(questNum, taskName);
		if (isValidQuestName(questName) && isValidTaskId(taskNum))
		{
			questClearQuestTask(questNum, taskNum, player);
		}
	}
	
	
	public static int getTaskId(int questCrc, String taskName) throws InterruptedException
	{
		for (int i = 0; i < maxTasksPerQuest; ++i)
		{
			testAbortScript();
			String name = getTaskStringDataEntry(questCrc, i, dataTableColumnTaskName);
			if ((name != null) && (name.equals(taskName)))
			{
				return i;
			}
		}
		return -1;
	}
	
	
	public static boolean isValidQuestName(String questName) throws InterruptedException
	{
		return getQuestIdFromString(questName) != INVALID_QUEST;
	}
	
	
	public static dictionary getActiveTasksForTaskType(obj_id player, String taskType) throws InterruptedException
	{
		dictionary dict = new dictionary();
		Vector tasks = new Vector(0);
		
		String taskObjVarName = getTaskTypeObjVar(player, taskType);
		if (taskObjVarName != null)
		{
			
			obj_var_list questList = getObjVarList(player, taskObjVarName);
			if (questList != null)
			{
				int countQuests = questList.getNumItems();
				for (int i = 0; i < countQuests; ++i)
				{
					testAbortScript();
					obj_var ov = questList.getObjVar(i);
					String questName = ov.getName();
					int questCrc = questGetQuestId(questName);
					
					String questObjVarName = taskObjVarName + dot + questName;
					if (hasObjVar(player, questObjVarName))
					{
						
						obj_var_list taskList = getObjVarList(player, questObjVarName);
						if (taskList != null)
						{
							tasks.clear();
							
							int countTasks = taskList.getNumItems();
							for (int j = 0; j < countTasks; ++j)
							{
								testAbortScript();
								obj_var ov2 = taskList.getObjVar(j);
								String taskName = ov2.getName();
								int taskId = utils.stringToInt(taskName);
								
								utils.addElement(tasks, taskId);
							}
							dict.put(Integer.toString(questCrc), tasks);
						}
					}
				}
			}
		}
		
		return dict;
	}
	
	
	public static location getRandom2DLocationAroundPlayer(obj_id player, float minimumDistance, float maximumDistance) throws InterruptedException
	{
		location playerLocation = getLocation(player);
		return getRandom2DLocationAroundLocation(player, 0, 0, minimumDistance, maximumDistance);
	}
	
	
	public static location getRandom2DLocationAroundLocation(obj_id player, float relativeOffsetX, float relativeOffsetZ, float minimumDistance, float maximumDistance) throws InterruptedException
	{
		boolean onAFloor = isOnAFloor(player);
		location playerLocation = getLocation(player);
		boolean inACell = (playerLocation.cell != null) && (playerLocation.cell != obj_id.NULL_ID);
		
		location newLocation = new location(playerLocation);
		newLocation.x += relativeOffsetX;
		newLocation.z += relativeOffsetZ;
		
		newLocation = utils.getRandomLocationInRing(newLocation, minimumDistance, maximumDistance);
		
		if (!isValidQuestSpawnPoint(player, newLocation, onAFloor, inACell))
		{
			float xDelta = (playerLocation.x - newLocation.x) * 2;
			float zDelta = (playerLocation.z - newLocation.z) * 2;
			
			if (newLocation.x < playerLocation.x)
			{
				newLocation.x += xDelta;
			}
			else
			{
				newLocation.x -= xDelta;
			}
			
			if (!isValidQuestSpawnPoint(player, newLocation, onAFloor, inACell))
			{
				
				if (newLocation.z < playerLocation.z)
				{
					newLocation.z += zDelta;
				}
				else
				{
					newLocation.z -= zDelta;
				}
				
				if (!isValidQuestSpawnPoint(player, newLocation, onAFloor, inACell))
				{
					
					if (newLocation.x < playerLocation.x)
					{
						newLocation.x += xDelta;
					}
					else
					{
						newLocation.x -= xDelta;
					}
					
					if (!isValidQuestSpawnPoint(player, newLocation, onAFloor, inACell))
					{
						
						newLocation = (location)playerLocation.clone();
					}
				}
			}
		}
		
		if (onAFloor)
		{
			newLocation.y = getFloorHeightAtRelativePointOnSameFloorAsObject(player, newLocation.x - playerLocation.x, newLocation.z - playerLocation.z);
		}
		else
		{
			
			newLocation.y = getHeightAtLocation(newLocation.x, newLocation.z);
		}
		
		return newLocation;
	}
	
	
	public static boolean isValidQuestSpawnPoint(obj_id player, location newLocation, boolean onAFloor, boolean inACell) throws InterruptedException
	{
		boolean validPoint = true;
		location playerLocation = getLocation(player);
		if (validPoint)
		{
			validPoint = !getCollidesWithObject(newLocation, 1.0f);
		}
		
		if (validPoint && onAFloor)
		{
			validPoint = isRelativePointOnSameFloorAsObject(player, newLocation.x - playerLocation.x, newLocation.z - playerLocation.z);
		}
		
		if (validPoint && inACell)
		{
			validPoint = isValidInteriorLocation(newLocation);
		}
		
		return validPoint;
	}
	
	
	public static location getRandom2DLocationInRing(location startLocation, float minimumRadius, float maximumRadius) throws InterruptedException
	{
		location newLocation = (location)startLocation.clone();
		float deltaX = rand(minimumRadius, maximumRadius);
		if (rand(1,2) == 2)
		{
			deltaX = -deltaX;
		}
		float deltaZ = rand(minimumRadius, maximumRadius);
		if (rand(1,2) == 2)
		{
			deltaZ = -deltaZ;
		}
		newLocation.x += deltaX;
		newLocation.z += deltaZ;
		newLocation.y = getHeightAtLocation(newLocation.x, newLocation.z);
		return newLocation;
	}
	
	
	public static void setPendingStaticEscortTarget(obj_id player, obj_id newEscortTarget) throws InterruptedException
	{
		setObjVar(player, questBaseObjVar + dot + questAllObjVar + dot + questStaticEscortTargetObjVar, newEscortTarget);
	}
	
	
	public static boolean isEscortTargetReadyForStaticEscortTask(obj_id escortTarget) throws InterruptedException
	{
		
		if (hasObjVar(escortTarget, objvarOnEscort) || hasObjVar(escortTarget, objvarGoingHome))
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	
	
	public static obj_id getPendingStaticEscortTarget(obj_id player) throws InterruptedException
	{
		return getObjIdObjVar(player, questBaseObjVar + dot + questAllObjVar + dot + questStaticEscortTargetObjVar);
	}
	
	
	public static void clearPendingStaticEscortTarget(obj_id player) throws InterruptedException
	{
		removeObjVar(player, questBaseObjVar + dot + questAllObjVar + dot + questStaticEscortTargetObjVar);
	}
	
	
	public static boolean isTaskVisible(int questCrc, int taskId) throws InterruptedException
	{
		return groundquests.getTaskIntDataEntry(questCrc, taskId, dataTableColumnVisible) != 0;
	}
	
	
	public static boolean isQuestVisible(int questCrc) throws InterruptedException
	{
		return getQuestBoolDataEntry(questCrc, dataTableColumnQuestVisible, true);
	}
	
	
	public static void playJournalUpdatedMusic(int questCrc, int taskId, obj_id player) throws InterruptedException
	{
		
		if (groundquests.isTaskVisible(questCrc, taskId))
		{
			play2dNonLoopingSound(player, JOURNAL_UPDATED_MUSIC);
		}
	}
	
	
	public static boolean isValidTaskId(int taskId) throws InterruptedException
	{
		return (taskId >= 0) && (taskId < maxTasksPerQuest);
	}
	
	
	public static void questStartPerforming(obj_id player, int performType) throws InterruptedException
	{
		dictionary params = new dictionary();
		params.put(PERFORM_TYPE, performType);
		messageTo(player, "startPerform", params, 1, false);
	}
	
	
	public static void questStopPerforming(obj_id player) throws InterruptedException
	{
		messageTo(player, "stopPerform", null, 1, false);
	}
	
	
	public static void questStartDance(obj_id player) throws InterruptedException
	{
		questStartPerforming(player, PERFORM_DANCE);
	}
	
	
	public static void questStartMusic(obj_id player) throws InterruptedException
	{
		questStartPerforming(player, PERFORM_MUSIC);
	}
	
	
	public static void questStartJuggle(obj_id player) throws InterruptedException
	{
		questStartPerforming(player, PERFORM_JUGGLE);
	}
	
	
	public static void sendPlacedMoreThanOneInInventorySystemMessage(obj_id player, obj_id objectPlaced, int count) throws InterruptedException
	{
		if (isValidId(objectPlaced))
		{
			string_id name = getNameStringId(objectPlaced);
			prose_package pp = new prose_package();
			pp = prose.setStringId(pp, SID_MORE_THAN_ONE_PLACED_IN_INVENTORY);
			pp = prose.setDI(pp, count);
			pp = prose.setTO(pp, name);
			sendQuestSystemMessage(player, pp);
		}
	}
	
	
	public static void sendPlacedInInventorySystemMessage(obj_id player, obj_id objectPlaced, String objectTemplate) throws InterruptedException
	{
		if (isValidId(objectPlaced))
		{
			prose_package pp = null;
			string_id name = getNameStringId(objectPlaced);
			
			pp = prose.getPackage(SID_PLACED_IN_INVENTORY, name);
			sendQuestSystemMessage(player, pp);
		}
		else
		{
			if (objectTemplate != null && objectTemplate.length() > 0)
			{
				LOG("quest", "WARNING: Failed to place ["+ objectTemplate + "] in player's inventory.");
			}
		}
	}
	
	
	public static void sendPlacedInInventorySystemMessage(obj_id player, obj_id objectPlaced) throws InterruptedException
	{
		sendPlacedInInventorySystemMessage(player, objectPlaced, null);
	}
	
	
	public static void sendRemovedFromInventorySystemMessage(obj_id player, obj_id objectPlaced, String objectTemplate) throws InterruptedException
	{
		if (isValidId(objectPlaced))
		{
			prose_package pp = null;
			string_id name = getNameStringId(objectPlaced);
			
			pp = prose.getPackage(SID_REMOVED_FROM_INVENTORY, name);
			sendQuestSystemMessage(player, pp);
		}
		else
		{
			if (objectTemplate != null && objectTemplate.length() > 0)
			{
				LOG("quest", "WARNING: Failed to remove ["+ objectTemplate + "] from player's inventory.");
			}
		}
	}
	
	
	public static void sendRemovedFromInventorySystemMessage(obj_id player, obj_id objectPlaced) throws InterruptedException
	{
		sendRemovedFromInventorySystemMessage(player, objectPlaced, null);
	}
	
	
	public static boolean isActionAllowedForQuest(obj_id player, obj_id npc, String actionName) throws InterruptedException
	{
		
		if (!hasObjVar(npc, QUEST_NAME))
		{
			return false;
		}
		
		if (!hasObjVar(npc, TASK_NAME))
		{
			return false;
		}
		
		int questCrc = getQuestIdFromString(getStringObjVar(npc, QUEST_NAME));
		int taskId = getTaskId(questCrc, getStringObjVar(npc, TASK_NAME));
		
		if (!questIsTaskActive(questCrc, taskId, player))
		{
			return false;
		}
		
		String questActionName = getTaskStringDataEntry(questCrc, taskId, dataTableColumnActionName);
		
		if (questActionName == null)
		{
			return false;
		}
		
		String baseQuest, baseAction;
		int versionQuest, versionAction;
		
		int indexQuest = questActionName.lastIndexOf('_');
		int indexAction = actionName.lastIndexOf('_');
		
		if (indexQuest < 0)
		{
			baseQuest = questActionName;
			versionQuest = 0;
		}
		else
		{
			baseQuest = questActionName.substring(0, indexQuest);
			versionQuest = utils.stringToInt(questActionName.substring(indexQuest+1));
		}
		
		if (indexAction < 0)
		{
			baseAction = actionName;
			versionAction = 0;
		}
		else
		{
			baseAction = actionName.substring(0, indexAction);
			versionAction = utils.stringToInt(actionName.substring(indexAction+1));
		}
		
		questOutputDebugInfo(player, "groundquests", "isActionAllowedForQuest",
		questActionName + ": "+ baseQuest + " - "+ versionQuest + ", "+ baseAction + " - "+ versionAction);
		
		boolean result = baseAction.equals(baseQuest) && versionAction >= versionQuest;
		
		questOutputDebugInfo(player, "groundquests", "isActionAllowedForQuest",
		"result = "+ (result ? "true": "false"));
		
		return result;
	}
	
	
	public static boolean questActionCompleted(obj_id player, obj_id npc, String actionName) throws InterruptedException
	{
		if (isActionAllowedForQuest(player, npc, actionName))
		{
			int questCrc = getQuestIdFromString(getStringObjVar(npc, QUEST_NAME));
			int taskId = getTaskId(questCrc, getStringObjVar(npc, TASK_NAME));
			
			dictionary params = new dictionary();
			params.put(QUEST_CRC, questCrc);
			params.put(TASK_ID, taskId);
			
			questOutputDebugInfo(player, "groundquests", "questActionCompleted",
			"sending questActionCompleted");
			
			messageTo(player, "questActionCompleted", params, 0, false);
			
			return true;
		}
		
		return false;
	}
	
	
	public static void applyQuestPenalty(obj_id player, String factionName, int factionAmount) throws InterruptedException
	{
		
		if (factionName != null && factionName.length() > 0 && factionAmount > 0)
		{
			float currentFactionStanding = factions.getFactionStanding(player, factionName);
			factions.setFactionStanding(player, factionName, currentFactionStanding - factionAmount);
			prose_package pp = factions.getFactionProsePackage(factionName, -factionAmount);
			sendQuestSystemMessage(player, pp);
		}
	}
	
	
	public static void grantQuestReward(obj_id player, int questCrc, int questLevel, int questTier, String experienceType, int experienceAmount, String factionName, int factionAmount, boolean grantGcwReward, int bankCredits, String item, int itemCount, String weapon, int weaponCount, float weaponSpeed, float weaponDamage, float weaponEfficiency, float weaponElementalValue, String armor, int armorCount, int armorQuality, String[] inclusiveLootNames, int[] inclusiveLootCounts, String exclusiveLootChoiceName, int exclusiveLootChoiceCount, String badgeName, boolean useShowLootBox) throws InterruptedException
	{
		grantQuestReward(player, questCrc, questLevel, questTier, experienceType, experienceAmount, factionName, factionAmount, grantGcwReward, bankCredits, item, itemCount, weapon, weaponCount, weaponSpeed, weaponDamage, weaponEfficiency, weaponElementalValue, armor, armorCount, armorQuality, inclusiveLootNames, inclusiveLootCounts, exclusiveLootChoiceName, exclusiveLootChoiceCount, badgeName, useShowLootBox, 0, 0, null, 0, null, 0, 0);
	}
	
	
	public static void grantQuestReward(obj_id player, int questCrc, int questLevel, int questTier, String experienceType, int experienceAmount, String factionName, int factionAmount, boolean grantGcwReward, int bankCredits, String item, int itemCount, String weapon, int weaponCount, float weaponSpeed, float weaponDamage, float weaponEfficiency, float weaponElementalValue, String armor, int armorCount, int armorQuality, String[] inclusiveLootNames, int[] inclusiveLootCounts, String exclusiveLootChoiceName, int exclusiveLootChoiceCount, String badgeName, boolean useShowLootBox, int grantGcwOverwriteAmt, int grantGcwSFModifier, String grantGcwRebReward, int grantGcwRebRewardCount, String grantGcwImpReward, int grantGcwImpRewardCount, int grantGcwSFRewardMultip) throws InterruptedException
	{
		
		if (itemCount == -1)
		{
			itemCount = 1;
		}
		if (weaponCount == -1)
		{
			weaponCount = 1;
		}
		if (armorCount == -1)
		{
			armorCount = 1;
		}
		
		experienceAmount = getQuestExperienceReward(player, questLevel, questTier, experienceAmount);
		obj_id playerInv = utils.getInventoryContainer(player);
		Vector lootItemsResizeable = new Vector();
		lootItemsResizeable.setSize(0);
		
		if ((experienceType != null) && (experienceType.length() > 0) && (experienceAmount > 0))
		{
			if (xp.isCombatXpType(experienceType))
			{
				xp.grantCombatStyleXp(player, experienceType, experienceAmount);
			}
			else if (xp.isCraftingXpType(experienceType))
			{
				
				xp.grantCraftingQuestXp(player, experienceAmount);
			}
			else if (xp.isSocialXpType(experienceType))
			{
				xp.grantSocialStyleXp(player, experienceType, experienceAmount);
			}
			else
			{
				xp.grantUnmodifiedExperience(player, experienceType, experienceAmount, false);
			}
			xp.displayXpFlyText(player, player, experienceAmount);
			xp.displayXpMsg(player, experienceType, experienceAmount);
		}
		
		if (bankCredits > 0)
		{
			money.bankTo(money.ACCT_NEW_PLAYER_QUESTS, player, bankCredits);
			play2dNonLoopingSound(player, groundquests.MUSIC_QUEST_RECEIVED_CREDITS);
		}
		
		if ((factionName != null) && (factionName.length() > 0) && (factionAmount != 0))
		{
			float currentFactionStanding = factions.getFactionStanding(player, factionName);
			factions.setFactionStanding(player, factionName, currentFactionStanding + factionAmount);
			prose_package pp = factions.getFactionProsePackage(factionName, factionAmount);
			sendQuestSystemMessage(player, pp);
		}
		
		if (grantGcwReward)
		{
			int intFaction = pvpGetAlignedFaction(player);
			if (intFaction == 0 && !factions.isRebelHelper(player) && !factions.isImperialHelper(player))
			{
				return;
			}
			
			int playerPvpType = pvpGetType(player);
			
			if (!factions.isRebelHelper(player) && !factions.isImperialHelper(player))
			{
				if (grantGcwOverwriteAmt == 0)
				{
					int gcwGroundQuestValue = gcw.getGcwGroundQuestAward(player, questTier);
					gcw._grantGcwPoints(null, player, gcwGroundQuestValue, false, gcw.GCW_POINT_TYPE_GROUND_QUEST, questGetQuestName(questCrc));
				}
				else if (grantGcwOverwriteAmt > 0)
				{
					if (playerPvpType == PVPTYPE_DECLARED && grantGcwSFModifier != 0)
					{
						grantGcwOverwriteAmt *= grantGcwSFModifier;
					}
					
					gcw._grantGcwPoints(null, player, grantGcwOverwriteAmt, false, gcw.GCW_POINT_TYPE_GROUND_QUEST, questGetQuestName(questCrc));
				}
			}
			
			if (factions.isRebelorRebelHelper(player) && grantGcwRebReward != null && grantGcwRebReward.length() > 0 && grantGcwRebRewardCount != 0)
			{
				boolean declared = false;
				if (playerPvpType == PVPTYPE_DECLARED)
				{
					declared = true;
				}
				else if (factions.isRebelHelper(player) && pvpNeutralIsMercenaryDeclared(player))
				{
					declared = true;
				}
				
				if (declared && grantGcwSFRewardMultip != 0)
				{
					grantGcwRebRewardCount *= grantGcwSFRewardMultip;
				}
				
				for (int i = 0; i < grantGcwRebRewardCount; i++)
				{
					testAbortScript();
					obj_id lootItem = static_item.createNewItemFunction(grantGcwRebReward, playerInv);
					if (i == 0)
					{
						utils.addElement(lootItemsResizeable, lootItem);
						if (grantGcwRebRewardCount == 1)
						{
							sendPlacedInInventorySystemMessage(player, lootItem);
						}
						else if (grantGcwRebRewardCount > 1)
						{
							sendPlacedMoreThanOneInInventorySystemMessage(player, lootItem, grantGcwRebRewardCount);
						}
					}
					
					CustomerServiceLog("QUEST_REWARD", player +" received reward "+ grantGcwRebReward + " ("+lootItem+")");
				}
			}
			else if (factions.isImperialorImperialHelper(player) && grantGcwImpReward != null && grantGcwImpReward.length() > 0 & grantGcwImpRewardCount != 0)
			{
				boolean declared = false;
				
				if (playerPvpType == PVPTYPE_DECLARED)
				{
					declared = true;
				}
				else if (factions.isImperialHelper(player) && pvpNeutralIsMercenaryDeclared(player))
				{
					declared = true;
				}
				
				if (declared && grantGcwSFRewardMultip != 0)
				{
					grantGcwImpRewardCount *= grantGcwSFRewardMultip;
				}
				
				for (int i = 0; i < grantGcwImpRewardCount; i++)
				{
					testAbortScript();
					obj_id lootItem = static_item.createNewItemFunction(grantGcwImpReward, playerInv);
					if (i == 0)
					{
						utils.addElement(lootItemsResizeable, lootItem);
						if (grantGcwImpRewardCount == 1)
						{
							sendPlacedInInventorySystemMessage(player, lootItem);
						}
						else if (grantGcwImpRewardCount > 1)
						{
							sendPlacedMoreThanOneInInventorySystemMessage(player, lootItem, grantGcwImpRewardCount);
						}
					}
					
					CustomerServiceLog("QUEST_REWARD", player +" received reward "+ grantGcwImpReward+ " ("+lootItem+")");
				}
			}
		}
		
		if ((badgeName != null) && (!badgeName.equals("")) && (badgeName.length() > 0))
		{
			if (!badge.hasBadge (player, badgeName))
			{
				badge.grantBadge (player, badgeName);
			}
		}
		
		if ((item != null) && (item.length() > 0))
		{
			for (int i = 0; i < itemCount; ++i)
			{
				testAbortScript();
				obj_id newItem = createObjectInInventoryAllowOverload(item, player);
				sendPlacedInInventorySystemMessage(player, newItem, item);
				CustomerServiceLog("QUEST_REWARD", player +" received reward "+ item +" ("+newItem+")");
			}
			play2dNonLoopingSound(player, groundquests.MUSIC_QUEST_RECEIVED_ITEM);
		}
		
		if ((weapon != null) && (weapon.length() > 0))
		{
			for (int i = 0; i < weaponCount; ++i)
			{
				testAbortScript();
				obj_id newWeapon = weapons.createWeaponOverloaded(weapon, playerInv, weaponSpeed, weaponDamage, weaponEfficiency, weaponElementalValue);
				sendPlacedInInventorySystemMessage(player, newWeapon, weapon);
				CustomerServiceLog("QUEST_REWARD", player +" received reward "+ weapon +" ("+newWeapon+")");
			}
		}
		
		if ((armor != null) && (armor.length() > 0))
		{
			for (int i = 0; i < armorCount; ++i)
			{
				testAbortScript();
				obj_id armorItem = createObjectInInventoryAllowOverload (armor, player);
				loot.randomizeArmor(armorItem, armorQuality);
				sendPlacedInInventorySystemMessage(player, armorItem, armor);
				CustomerServiceLog("QUEST_REWARD", player +" received reward "+ armor +" ("+armorItem+")");
			}
		}
		
		for (int j = 0; j < inclusiveLootNames.length; ++j)
		{
			testAbortScript();
			if ((inclusiveLootNames[j] != null) && (inclusiveLootNames[j].length() > 0))
			{
				dictionary inclusiveItemData = static_item.getMasterItemDictionary(inclusiveLootNames[j]);
				boolean giveInclusiveItemInventoryMessage = true;
				if (inclusiveItemData != null)
				{
					for (int i = 0; i < inclusiveLootCounts[j]; ++i)
					{
						testAbortScript();
						obj_id lootItem = static_item.createNewItemFunction(inclusiveLootNames[j], player);
						
						if (i == 0)
						{
							utils.addElement(lootItemsResizeable, lootItem);
						}
						
						CustomerServiceLog("QUEST_REWARD", player +" received reward "+ inclusiveLootNames[j]+ " ("+lootItem+")");
						
						if (giveInclusiveItemInventoryMessage)
						{
							sendPlacedInInventorySystemMessage(player, lootItem, inclusiveLootNames[j]);
						}
						if (inclusiveItemData.containsKey("scripts"))
						{
							String inclusiveItemScriptList = inclusiveItemData.getString("scripts");
							if (inclusiveItemScriptList != null && inclusiveItemScriptList.length() > 0 && inclusiveItemScriptList.indexOf("autostack") > -1)
							{
								giveInclusiveItemInventoryMessage = false;
							}
						}
					}
				}
			}
		}
		
		if ((exclusiveLootChoiceName != null) && (exclusiveLootChoiceName.length() > 0))
		{
			dictionary exclusiveItemData = static_item.getMasterItemDictionary(exclusiveLootChoiceName);
			boolean giveExclusiveItemInventoryMessage = true;
			if (exclusiveItemData != null)
			{
				for (int i = 0; i < exclusiveLootChoiceCount; ++i)
				{
					testAbortScript();
					obj_id lootItem = static_item.createNewItemFunction(exclusiveLootChoiceName, player);
					
					if (i == 0)
					{
						utils.addElement(lootItemsResizeable, lootItem);
					}
					
					CustomerServiceLog("QUEST_REWARD", player +" received reward "+ exclusiveLootChoiceName+ " ("+lootItem+")");
					
					if (giveExclusiveItemInventoryMessage)
					{
						sendPlacedInInventorySystemMessage(player, lootItem, exclusiveLootChoiceName);
					}
					if (exclusiveItemData.containsKey("scripts"))
					{
						String exclusiveItemScriptList = exclusiveItemData.getString("scripts");
						if (exclusiveItemScriptList != null && exclusiveItemScriptList.length() > 0 && exclusiveItemScriptList.indexOf("autostack") > -1)
						{
							giveExclusiveItemInventoryMessage = false;
						}
					}
				}
			}
		}
		
		if (useShowLootBox)
		{
			if (lootItemsResizeable.size() > 0)
			{
				obj_id[] lootItems = new obj_id[0];
				if (lootItemsResizeable != null)
				{
					lootItems = new obj_id[lootItemsResizeable.size()];
					lootItemsResizeable.toArray(lootItems);
				}
				showLootBox(player, lootItems);
			}
		}
		
		return;
	}
	
	
	public static boolean playerNeedsToRetrieveThisItem(obj_id player, obj_id item) throws InterruptedException
	{
		
		return playerNeedsToRetrieveThisItem(player, item, "retrieve_item");
	}
	
	
	public static boolean playerNeedsToRetrieveThisItem(obj_id player, obj_id item, String taskType) throws InterruptedException
	{
		if (isMob(item) && isIncapacitated(item))
		{
			return false;
		}
		
		boolean needed = false;
		
		String itemTemplateName = getTemplateName(item);
		
		dictionary tasks = groundquests.getActiveTasksForTaskType(player, taskType);
		if ((tasks != null) && !tasks.isEmpty())
		{
			java.util.Enumeration keys = tasks.keys();
			while (keys.hasMoreElements())
			{
				testAbortScript();
				String questCrcString = (String)keys.nextElement();
				int questCrc = utils.stringToInt(questCrcString);
				int[] tasksForCurrentQuest = tasks.getIntArray(questCrcString);
				
				for (int i = 0; i < tasksForCurrentQuest.length; ++i)
				{
					testAbortScript();
					int taskId = tasksForCurrentQuest[i];
					String baseObjVar = groundquests.getBaseObjVar(player, taskType, questGetQuestName(questCrc), taskId);
					String retrieveTemplateName = groundquests.getTaskStringDataEntry(questCrc, taskId, "SERVER_TEMPLATE");
					if (retrieveTemplateName != null && itemTemplateName.equals(retrieveTemplateName))
					{
						needed = true;
						
						String objvarRetrievedFull = baseObjVar + dot + "retrieved_items";
						if (hasObjVar(player, objvarRetrievedFull))
						{
							obj_id[] itemsAlreadyRetrieved = getObjIdArrayObjVar(player, objvarRetrievedFull);
							for (int j = 0; j < itemsAlreadyRetrieved.length; ++j)
							{
								testAbortScript();
								obj_id itemAlredyRetrieved = itemsAlreadyRetrieved[j];
								if (item == itemAlredyRetrieved)
								{
									needed = false;
								}
							}
						}
					}
				}
			}
		}
		
		return needed;
	}
	
	
	public static String getRetrieveMenuText(obj_id player, obj_id item) throws InterruptedException
	{
		
		return getRetrieveMenuText(player, item, "retrieve_item");
	}
	
	
	public static String getRetrieveMenuText(obj_id player, obj_id item, String taskType) throws InterruptedException
	{
		String menuText = null;
		String itemTemplateName = getTemplateName(item);
		boolean menuTextSet = false;
		
		dictionary tasks = groundquests.getActiveTasksForTaskType(player, taskType);
		if ((tasks != null) && !tasks.isEmpty())
		{
			java.util.Enumeration keys = tasks.keys();
			while (keys.hasMoreElements())
			{
				testAbortScript();
				String questCrcString = (String)keys.nextElement();
				int questCrc = utils.stringToInt(questCrcString);
				int[] tasksForCurrentQuest = tasks.getIntArray(questCrcString);
				
				for (int i = 0; i < tasksForCurrentQuest.length; ++i)
				{
					testAbortScript();
					int taskId = tasksForCurrentQuest[i];
					String baseObjVar = groundquests.getBaseObjVar(player, taskType, questGetQuestName(questCrc), taskId);
					String retrieveTemplateName = groundquests.getTaskStringDataEntry(questCrc, taskId, "SERVER_TEMPLATE");
					
					if (retrieveTemplateName != null && itemTemplateName.equals(retrieveTemplateName))
					{
						
						String tempMenuText = groundquests.getTaskStringDataEntry(questCrc, taskId, "RETRIEVE_MENU_TEXT");
						
						if (tempMenuText != null && tempMenuText.length() > 0)
						{
							if (!menuTextSet)
							{
								menuText = tempMenuText;
								groundquests.questOutputDebugInfo(player, questCrc, taskId, taskType, "getRetrieveMenuText",
								"Menu text is ["+ menuText + "]");
								
								menuTextSet = true;
							}
							else
							{
								
								groundquests.questOutputDebugInfo(player, questCrc, taskId, taskType, "getRetrieveMenuText",
								"Item used for multiple quests! Resetting menu to default. ["+ itemTemplateName + "]");
								
								menuText = null;
							}
						}
					}
				}
			}
		}
		
		return menuText;
	}
	
	
	public static int getQuestCountdownTime(obj_id player, obj_id item) throws InterruptedException
	{
		
		return getQuestCountdownTime(player, item, "retrieve_item");
	}
	
	
	public static int getQuestCountdownTime(obj_id player, obj_id item, String taskType) throws InterruptedException
	{
		int countdownTime = 0;
		String itemTemplateName = getTemplateName(item);
		
		dictionary tasks = groundquests.getActiveTasksForTaskType(player, taskType);
		if ((tasks != null) && !tasks.isEmpty())
		{
			java.util.Enumeration keys = tasks.keys();
			while (keys.hasMoreElements())
			{
				testAbortScript();
				String questCrcString = (String)keys.nextElement();
				int questCrc = utils.stringToInt(questCrcString);
				int[] tasksForCurrentQuest = tasks.getIntArray(questCrcString);
				
				for (int i = 0; i < tasksForCurrentQuest.length; ++i)
				{
					testAbortScript();
					int taskId = tasksForCurrentQuest[i];
					String baseObjVar = groundquests.getBaseObjVar(player, taskType, questGetQuestName(questCrc), taskId);
					String retrieveTemplateName = groundquests.getTaskStringDataEntry(questCrc, taskId, "SERVER_TEMPLATE");
					
					if (retrieveTemplateName != null && itemTemplateName.equals(retrieveTemplateName))
					{
						countdownTime = groundquests.getTaskIntDataEntry(questCrc, taskId, "COUNTDOWN_TIMER");
						
						if (countdownTime < 0)
						{
							countdownTime = 0;
						}
					}
				}
			}
		}
		
		return countdownTime;
	}
	
	
	public static int getQuestExperienceReward(obj_id player, int questLevel, int questTier, int experienceAmount) throws InterruptedException
	{
		
		if (questLevel < 1 || questTier < 0)
		{
			return experienceAmount;
		}
		
		if (questTier == 0)
		{
			return 0;
		}
		
		if (questTier > 6)
		{
			questTier = 6;
		}
		
		final String[] tierColumns =
		{
			dataTableColumnQuestRewardExperienceTeir1,
			dataTableColumnQuestRewardExperienceTeir2,
			dataTableColumnQuestRewardExperienceTeir3,
			dataTableColumnQuestRewardExperienceTeir4,
			dataTableColumnQuestRewardExperienceTeir5,
			dataTableColumnQuestRewardExperienceTeir6
		};
		
		int questXp = dataTableGetInt(QUEST_EXPERIENCE_TABLE, ""+questLevel, tierColumns[questTier-1]);
		int xpCap = getQuestXpCap(player);
		
		if (questXp > xpCap)
		{
			questXp = xpCap;
		}
		
		return questXp;
	}
	
	
	public static int getQuestXpCap(obj_id player) throws InterruptedException
	{
		int level = getLevel(player);
		
		int xpCurrentLevel = dataTableGetInt("datatables/player/player_level.iff", level, "xp_required");
		int xpPreviousLevel = dataTableGetInt("datatables/player/player_level.iff", level-1, "xp_required");
		
		int xpRequired = xpCurrentLevel - xpPreviousLevel;
		int xpCap = xpRequired / 2;
		
		if (xpCap < 0)
		{
			xpCap = 0;
		}
		
		return xpCap;
	}
	
	
	public static void createQuestWaypoints(int questCrc, int taskId, obj_id self) throws InterruptedException
	{
		boolean createWaypoint = groundquests.getTaskIntDataEntry(questCrc, taskId, dataTableColumnCreateWaypoint) > 0;
		
		boolean createEntranceWaypoint = groundquests.getTaskIntDataEntry(questCrc, taskId, dataTableColumnEntranceCreateWaypoint) > 0;
		
		if (createWaypoint && createEntranceWaypoint)
		{
			String entranceWaypointName = groundquests.getTaskStringDataEntry(questCrc, taskId, dataTableColumnEntranceWaypointName);
			float entranceWorldLocationX = utils.stringToFloat(groundquests.getTaskStringDataEntry(questCrc, taskId, dataTableColumnEntranceLocationX));
			float entranceWorldLocationY = utils.stringToFloat(groundquests.getTaskStringDataEntry(questCrc, taskId, dataTableColumnEntranceLocationY));
			float entranceWorldLocationZ = utils.stringToFloat(groundquests.getTaskStringDataEntry(questCrc, taskId, dataTableColumnEntranceLocationZ));
			String planetName = groundquests.getTaskStringDataEntry(questCrc, taskId, dataTableColumnPlanetName);
			
			location loc = new location(entranceWorldLocationX, entranceWorldLocationY, entranceWorldLocationZ, planetName);
			
			obj_id entranceWaypoint = createWaypointInDatapad(self, loc);
			
			setWaypointColor(entranceWaypoint, "entrance");
			
			String baseObjVar = groundquests.getBaseObjVar(self, taskType, questGetQuestName(questCrc), taskId);
			setObjVar(self, baseObjVar + dot + objvarEntranceWaypoint, entranceWaypoint);
			
			setWaypointName(entranceWaypoint, entranceWaypointName);
			
			setQuestWaypointActive(entranceWaypoint, self, baseObjVar);
		}
		
		if (createWaypoint)
		{
			String planetName = groundquests.getTaskStringDataEntry(questCrc, taskId, dataTableColumnPlanetName);
			String waypointName = groundquests.getTaskStringDataEntry(questCrc, taskId, dataTableColumnWaypointName);
			float worldLocationX = utils.stringToFloat(groundquests.getTaskStringDataEntry(questCrc, taskId, dataTableColumnLocationX));
			float worldLocationY = utils.stringToFloat(groundquests.getTaskStringDataEntry(questCrc, taskId, dataTableColumnLocationY));
			float worldLocationZ = utils.stringToFloat(groundquests.getTaskStringDataEntry(questCrc, taskId, dataTableColumnLocationZ));
			String interiorWaypointAppearance = groundquests.getTaskStringDataEntry(questCrc, taskId, dataTableColumnInteriorWaypointAppearance);
			String waypointBuildingCellName = groundquests.getTaskStringDataEntry(questCrc, taskId, dataTableColumnWaypointBuildingCellName);
			
			groundquests.questOutputDebugInfo(self, questCrc, taskId, taskType, "OnTaskActivated",
			taskType + " creating waypoint: "+ planetName + ": "+ worldLocationX + ", "+ worldLocationY + ", "+ worldLocationZ);
			
			location loc = new location(worldLocationX, worldLocationY, worldLocationZ, planetName);
			obj_id waypoint = null;
			
			if (interiorWaypointAppearance != null && waypointBuildingCellName != null && interiorWaypointAppearance.length() > 0 && waypointBuildingCellName.length() > 0)
			{
				waypoint = createWaypointInDatapadInternal(self, loc, interiorWaypointAppearance, waypointBuildingCellName);
			}
			else
			{
				waypoint = createWaypointInDatapad(self, loc);
			}
			
			if (createEntranceWaypoint)
			{
				setWaypointColor(waypoint, "small");
			}
			
			String baseObjVar = groundquests.getBaseObjVar(self, taskType, questGetQuestName(questCrc), taskId);
			setObjVar(self, baseObjVar + dot + objvarWaypoint, waypoint);
			
			setWaypointName(waypoint, waypointName);
			
			setQuestWaypointActive(waypoint, self, baseObjVar);
		}
	}
	
	
	public static void setQuestWaypointActive(obj_id waypoint, obj_id player, String baseObjVar) throws InterruptedException
	{
		String objvarFullName = baseObjVar + dot + groundquests.objvarWaypointInActive;
		
		if (utils.waypointExists(player, waypoint))
		{
			location waypointLoc = getWaypointLocation(waypoint);
			String waypointPlanet = waypointLoc.area;
			
			location playerLoc = getLocation(player);
			String playerPlanet = playerLoc.area;
			
			if (waypointPlanet.equals(playerPlanet))
			{
				if (hasObjVar(player, objvarFullName))
				{
					removeObjVar(player, objvarFullName);
				}
				
				setWaypointActive(waypoint, true);
			}
			else
			{
				if (!hasObjVar(player, objvarFullName))
				{
					setObjVar(player, objvarFullName, true);
				}
			}
		}
		else
		{
			if (hasObjVar(player, objvarFullName))
			{
				removeObjVar(player, objvarFullName);
			}
		}
		return;
	}
	
	
	public static obj_id getObjIdForWaypoint(int questCrc, int taskId, obj_id self) throws InterruptedException
	{
		String baseObjVar = groundquests.getBaseObjVar(self, taskType, questGetQuestName(questCrc), taskId);
		String baseWaypoint = baseObjVar + dot + objvarWaypoint;
		return getObjIdObjVar(self, baseWaypoint);
	}
	
	
	public static obj_id getObjIdForEntranceWaypoint(int questCrc, int taskId, obj_id self) throws InterruptedException
	{
		String baseObjVar = groundquests.getBaseObjVar(self, taskType, questGetQuestName(questCrc), taskId);
		String doorWaypoint = baseObjVar + dot + objvarEntranceWaypoint;
		return getObjIdObjVar(self, doorWaypoint);
	}
	
	
	public static void deleteQuestWaypoints(int questCrc, int taskId, boolean isForCTS, obj_id self) throws InterruptedException
	{
		String baseObjVar = groundquests.getBaseObjVar(self, taskType, questGetQuestName(questCrc), taskId);
		String baseWaypoint = baseObjVar + dot + objvarWaypoint;
		String doorWaypoint = baseObjVar + dot + objvarEntranceWaypoint;
		obj_id waypoint = getObjIdObjVar(self, baseWaypoint);
		
		if (isIdValid(waypoint))
		{
			removeObjVar(self, baseWaypoint);
			
			if (isForCTS == false)
			{
				destroyWaypointInDatapad(waypoint, self);
			}
		}
		
		obj_id entranceWaypoint = getObjIdObjVar(self, doorWaypoint);
		
		if (isIdValid(entranceWaypoint))
		{
			removeObjVar(self, doorWaypoint);
			
			if (isForCTS == false)
			{
				destroyWaypointInDatapad(entranceWaypoint, self);
			}
		}
	}
	
	
	public static void reattachQuestScripts(obj_id self) throws InterruptedException
	{
		
		obj_var_list scriptList = getObjVarList(self, questBaseObjVar + dot + "script");
		
		if (scriptList != null)
		{
			String[] scriptNames = scriptList.getAllObjVarNames();
			int numScripts = scriptNames.length;
			for (int i = 0; i < numScripts; ++i)
			{
				testAbortScript();
				
				obj_var oneScript = scriptList.getObjVar(scriptNames[i]);
				if (oneScript.getIntData() > 0)
				{
					
					if (!self.hasScript(scriptNames[i]))
					{
						attachScript(self, scriptNames[i]);
					}
				}
			}
			
		}
	}
	
	
	public static void refreshQuestWaypoints(int questCrc, int taskId, boolean isForCTS, obj_id self) throws InterruptedException
	{
		deleteQuestWaypoints (questCrc, taskId, isForCTS, self);
		
		createQuestWaypoints( questCrc, taskId, self);
	}
	
	
	public static void setGuaranteedSuccessTarget(obj_id player, int questCrc, int taskId, String baseObjVar) throws InterruptedException
	{
		String guaranteedSuccessTargetObjVar = baseObjVar + dot + objvarGuaranteedSuccess_Target;
		String guaranteedSuccessCountObjVar = baseObjVar + dot + objvarGuaranteedSuccess_Count;
		
		if (hasObjVar(player, guaranteedSuccessTargetObjVar))
		{
			removeObjVar(player, guaranteedSuccessTargetObjVar);
		}
		if (hasObjVar(player, guaranteedSuccessCountObjVar))
		{
			removeObjVar(player, guaranteedSuccessCountObjVar);
		}
		
		int minNumAttempts = groundquests.getTaskIntDataEntry(questCrc, taskId, dataTableColumnGuaranteedSuccessMin);
		int maxNumAttempts = groundquests.getTaskIntDataEntry(questCrc, taskId, dataTableColumnGuaranteedSuccessMax);
		
		if (minNumAttempts > 0 && maxNumAttempts > 0)
		{
			int target = 0;
			
			if (minNumAttempts >= maxNumAttempts)
			{
				target = minNumAttempts;
			}
			else
			{
				target = rand(minNumAttempts,maxNumAttempts);
			}
			
			if (target > 0)
			{
				setObjVar(player, guaranteedSuccessCountObjVar, 0);
				setObjVar(player, guaranteedSuccessTargetObjVar, target);
			}
		}
		
		return;
	}
	
	
	public static boolean checkForGuaranteedSuccess(obj_id player, String baseObjVar) throws InterruptedException
	{
		String guaranteedSuccessTargetObjVar = baseObjVar + dot + objvarGuaranteedSuccess_Target;
		String guaranteedSuccessCountObjVar = baseObjVar + dot + objvarGuaranteedSuccess_Count;
		
		boolean passed = false;
		
		if (hasObjVar(player, guaranteedSuccessTargetObjVar))
		{
			int guaranteedSuccessTarget = getIntObjVar(player, guaranteedSuccessTargetObjVar);
			
			int guaranteedSuccessCount = 1;
			if (hasObjVar(player, guaranteedSuccessCountObjVar))
			{
				guaranteedSuccessCount = getIntObjVar(player, guaranteedSuccessCountObjVar) + 1;
			}
			
			setObjVar(player, guaranteedSuccessCountObjVar, guaranteedSuccessCount);
			
			if (guaranteedSuccessCount >= guaranteedSuccessTarget)
			{
				passed = true;
			}
		}
		
		return passed;
	}
	
	
	public static boolean isDoingSmugglerMission(obj_id player) throws InterruptedException
	{
		if ((isQuestActive (player, "smuggle_generic_1") || isQuestActive (player, "smuggle_generic_2") || isQuestActive (player, "smuggle_generic_3") || isQuestActive (player, "smuggle_generic_4") || isQuestActive (player, "smuggle_generic_5")))
		{
			return true;
		}
		
		if ((isQuestActive (player, "smuggle_illicit_1") || isQuestActive (player, "smuggle_illicit_2") || isQuestActive (player, "smuggle_illicit_3") || isQuestActive (player, "smuggle_illicit_4") || isQuestActive (player, "smuggle_illicit_5")))
		{
			return true;
		}
		
		if (isQuestActive (player, "smuggle_pvp_4") || isQuestActive (player, "smuggle_pvp_5"))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static float getPlayerPlayedTimeWhenTimerEnds(obj_id player, int questCrc, int taskId) throws InterruptedException
	{
		String baseObjVar = groundquests.getBaseObjVar(player, "base_task", questGetQuestName(questCrc), taskId);
		
		if (hasObjVar(player, baseObjVar + dot + timeObjVar))
		{
			return getFloatObjVar(player, baseObjVar + dot + timeObjVar);
		}
		
		return -1.f;
	}
	
	
	public static boolean isTimeRemainingBeforeCompletion(obj_id player, int questCrc, int taskId) throws InterruptedException
	{
		final int remainingTime = getTaskIntDataEntry(questCrc, taskId, dataTableColumnRemainingTime);
		
		if (remainingTime > 0)
		{
			final int endTime = (int)getPlayerPlayedTimeWhenTimerEnds(player, questCrc, taskId);
			
			if (endTime > 0)
			{
				final int timeLeft = endTime - getPlayerPlayedTime(player) - remainingTime;
				
				if (timeLeft > 0)
				{
					prose_package pp = prose.getPackage(SID_TIME_REMAINING_BEFORE_COMPLETION, player, player);
					prose.setDI(pp, timeLeft);
					sendSystemMessageProse(player, pp);
					
					return true;
				}
			}
		}
		
		return false;
	}
	
	
	public static boolean gcwActiveDutyCheck(obj_id self, obj_id player) throws InterruptedException
	{
		if (hasObjVar(self, "questRequireActiveDuty"))
		{
			if (factions.isOnLeave(player))
			{
				return false;
			}
		}
		
		return true;
	}
	
	
	public static boolean completeTaskForGroupMembersInRange(obj_id questObj, obj_id player, int questCrc, int taskId) throws InterruptedException
	{
		return completeTaskForGroupMembersInRange(questObj, player, questCrc, taskId, 200.0f);
	}
	
	
	public static boolean completeTaskForGroupMembersInRange(obj_id questObj, obj_id player, int questCrc, int taskId, float range) throws InterruptedException
	{
		boolean result = false;
		
		if (!isIdValid(player) || !exists(player) || !player.isLoaded())
		{
			return result;
		}
		
		obj_id[] groupies = getGroupMembersInRange(questObj, player, range);
		if (groupies != null && groupies.length > 0)
		{
			for (int i = 0; i < groupies.length; i++)
			{
				testAbortScript();
				obj_id groupMember = groupies[i];
				if (isIdValid(groupMember))
				{
					if (questIsTaskActive(questCrc, taskId, groupMember))
					{
						questCompleteTask(questCrc, taskId, groupMember);
						result = true;
					}
				}
			}
		}
		
		return result;
	}
	
	
	public static obj_id[] getGroupMembersInRange(obj_id questObj, obj_id player) throws InterruptedException
	{
		return getGroupMembersInRange(questObj, player, 200.0f);
	}
	
	
	public static obj_id[] getGroupMembersInRange(obj_id questObj, obj_id player, float range) throws InterruptedException
	{
		Vector groupMembersInRange = new Vector();
		groupMembersInRange.setSize(0);
		
		obj_id gid = getGroupObject(player);
		if (!isIdValid(gid))
		{
			return null;
		}
		
		obj_id[] groupies = getGroupMemberIds(gid);
		if (groupies != null && groupies.length > 0)
		{
			location controllerLoc = getWorldLocation(questObj);
			for (int i = 0; i < groupies.length; i++)
			{
				testAbortScript();
				obj_id groupMember = groupies[i];
				if (isIdValid(groupMember) && groupMember.isLoaded() && isPlayer(groupMember))
				{
					location groupieLoc = getWorldLocation(groupMember);
					if (getDistance(controllerLoc, groupieLoc) <= range)
					{
						utils.addElement(groupMembersInRange, groupMember);
					}
				}
			}
		}
		
		obj_id[] staticGroupMembersInRange = utils.toStaticObjIdArray(groupMembersInRange);
		if (staticGroupMembersInRange != null && staticGroupMembersInRange.length > 0)
		{
			return staticGroupMembersInRange;
		}
		
		return null;
	}
	
}
