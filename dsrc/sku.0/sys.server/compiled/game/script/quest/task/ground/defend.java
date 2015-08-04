package script.quest.task.ground;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.ai.ai_combat;
import script.library.ai_lib;
import script.library.quests;
import script.library.create;
import script.library.groundquests;
import script.library.locations;
import script.library.prose;
import script.library.utils;



public class defend extends script.quest.task.ground.base_task
{
	public defend()
	{
	}
	public static final String OBJVAR_DEFENSE_TARGET = "defend_target";
	public static final String DEFEND_TARGET = "defendTarget";
	public static final String QUEST_CRC = "questCrc";
	public static final String TASK_ID = "taskId";
	public static final String PLAYER = "player";
	public static final String OBJVAR_BEEN_WARNED_DISTANCE = "beenWarned";
	public static final String TASK_TYPE = "defend";
	public static final String DOT = ".";
	
	public static final float DEFEND_DISTANCE_TIME_CHECK = 10;
	public static final float MAX_ALLOWED_DISTANCE = 40;
	public static final float CLEANUP_TIME = 120;
	
	public static final boolean LOGGING_ON = false;
	public static final String LOG_CAT = "defense_ground_task";
	
	
	public int OnTaskActivated(obj_id self, int questCrc, int taskId) throws InterruptedException
	{
		blog("OnTaskActivated init");
		groundquests.questOutputDebugInfo(self, questCrc, taskId, TASK_TYPE, "OnTaskActivated", TASK_TYPE + " task activated.");
		String baseObjVar = groundquests.setBaseObjVar(self, TASK_TYPE, questGetQuestName(questCrc), taskId);
		blog("baseObjVar: "+baseObjVar);
		
		if (!hasObjVar(self, OBJVAR_DEFENSE_TARGET))
		{
			return super.OnTaskActivated(self, questCrc, taskId);
		}
		blog("has var");
		
		obj_id defendTarget = getObjIdObjVar(self, OBJVAR_DEFENSE_TARGET);
		if (!isValidId(defendTarget) || !exists(defendTarget))
		{
			
			questFailTask(questCrc, taskId, self);
		}
		
		blog("defendTarget: "+defendTarget);
		groundquests.addDestroyNotification(defendTarget, self);
		setObjVar(self, baseObjVar + DOT + OBJVAR_DEFENSE_TARGET, defendTarget);
		removeObjVar(self, OBJVAR_DEFENSE_TARGET);
		
		dictionary distanceCheckParams = new dictionary();
		distanceCheckParams.put(PLAYER, self);
		distanceCheckParams.put(DEFEND_TARGET, defendTarget);
		distanceCheckParams.put(QUEST_CRC, questCrc);
		distanceCheckParams.put(TASK_ID, taskId);
		messageTo(self, "messageDefendCheckDistance", distanceCheckParams, DEFEND_DISTANCE_TIME_CHECK, false);
		return super.OnTaskActivated(self, questCrc, taskId);
	}
	
	
	public int destroyNotification(obj_id self, dictionary params) throws InterruptedException
	{
		blog("destroyNotification init");
		
		if (params == null)
		{
			blog("destroyNotification params null");
			
			return SCRIPT_CONTINUE;
		}
		
		if (!params.containsKey("source"))
		{
			blog("destroyNotification params source NULL");
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id source = params.getObjId("source");
		
		String taskObjVarName = groundquests.getTaskTypeObjVar(self, TASK_TYPE);
		if (taskObjVarName == null || taskObjVarName.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		blog("destroyNotification taskObjVarName: "+taskObjVarName);
		
		obj_var_list questList = getObjVarList(self, taskObjVarName);
		if (questList == null)
		{
			return SCRIPT_CONTINUE;
		}
		blog("destroyNotification questList: "+questList);
		
		int countQuests = questList.getNumItems();
		blog("destroyNotification countQuests: "+countQuests);
		
		for (int i = 0; i < countQuests; ++i)
		{
			testAbortScript();
			obj_var ov = questList.getObjVar(i);
			String questName = ov.getName();
			int questCrc = questGetQuestId(questName);
			
			String questObjVarName = taskObjVarName + DOT + questName;
			if (hasObjVar(self, questObjVarName))
			{
				
				obj_var_list taskList = getObjVarList(self, questObjVarName);
				if (taskList != null)
				{
					int countTasks = taskList.getNumItems();
					for (int j = 0; j < countTasks; ++j)
					{
						testAbortScript();
						obj_var ov2 = taskList.getObjVar(j);
						String taskName = ov2.getName();
						int taskId = utils.stringToInt(taskName);
						
						String escortTargetObjVarName = questObjVarName + DOT + taskName + DOT + OBJVAR_DEFENSE_TARGET;
						blog("destroyNotification escortTargetObjVarName: "+escortTargetObjVarName);
						
						if (hasObjVar(self, escortTargetObjVarName))
						{
							obj_id escortTarget = getObjIdObjVar(self, escortTargetObjVarName);
							if (source == escortTarget)
							{
								questFailTask(questCrc, taskId, self);
							}
							else if (!exists(source) && !exists(escortTarget))
							{
								questFailTask(questCrc, taskId, self);
							}
						}
					}
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int messageDefendCheckDistance(obj_id self, dictionary params) throws InterruptedException
	{
		blog("messageDefendCheckDistance init");
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey(PLAYER))
		{
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey(DEFEND_TARGET))
		{
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey(QUEST_CRC))
		{
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey(TASK_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("messageDefendCheckDistance contains all keys needed");
		obj_id player = params.getObjId(PLAYER);
		obj_id defendTarget = params.getObjId(DEFEND_TARGET);
		int questCrc = params.getInt (QUEST_CRC);
		int taskId = params.getInt (TASK_ID);
		
		if (questIsTaskActive(questCrc, taskId, player))
		{
			blog("messageDefendCheckDistance questIsTaskActive == true");
			String baseObjVar = groundquests.getBaseObjVar(player, TASK_TYPE, questGetQuestName(questCrc), taskId);
			blog("messageDefendCheckDistance baseObjVar: "+baseObjVar);
			if (isDefenseObjectNearby(questCrc, taskId, player, defendTarget))
			{
				blog("messageDefendCheckDistance isDefenseObjectNearby == true");
				
				if (hasObjVar(player, baseObjVar + OBJVAR_BEEN_WARNED_DISTANCE))
				{
					removeObjVar(player, baseObjVar + OBJVAR_BEEN_WARNED_DISTANCE);
				}
			}
			else
			{
				blog("messageDefendCheckDistance isDefenseObjectNearby == FALSE");
				
				if (hasObjVar(self, baseObjVar + DOT + OBJVAR_DEFENSE_TARGET))
				{
					obj_id defendTargetVar = getObjIdObjVar(self, baseObjVar + DOT + OBJVAR_DEFENSE_TARGET);
					if (defendTargetVar != defendTarget)
					{
						
						blog("messageDefendCheckDistance defendTargetVar is NOT the correct object.");
						return SCRIPT_CONTINUE;
					}
					blog("messageDefendCheckDistance defendTargetVar is the correct object.");
					blog("messageDefendCheckDistance defendTargetVar: "+defendTargetVar);
				}
				
				failDefenseDistanceCheck(player, questCrc, taskId, defendTarget);
				
			}
			
			messageTo(self, "messageDefendCheckDistance", params, DEFEND_DISTANCE_TIME_CHECK, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean isDefenseObjectNearby(int questCrc, int taskId, obj_id player, obj_id defendTarget) throws InterruptedException
	{
		blog("isDefenseObjectNearby init");
		
		boolean result = false;
		if (isIdValid(player) && isIdValid(defendTarget))
		{
			blog("isDefenseObjectNearby player and defendTarget are VALID");
			
			float distance = getDistance(player, defendTarget);
			if (distance < 0.0f)
			{
				return result;
			}
			
			result = distance <= MAX_ALLOWED_DISTANCE;
			blog("isDefenseObjectNearby DISTANCE: "+getDistance(player, defendTarget));
		}
		blog("isDefenseObjectNearby result: "+result);
		
		return result;
	}
	
	
	public void failDefenseDistanceCheck(obj_id player, int questCrc, int taskId, obj_id defenseTarget) throws InterruptedException
	{
		blog("failDefenseDistanceCheck init");
		
		String baseObjVar = groundquests.getBaseObjVar(player, TASK_TYPE, questGetQuestName(questCrc), taskId);
		if (!hasObjVar(player, baseObjVar + OBJVAR_BEEN_WARNED_DISTANCE))
		{
			
			setObjVar(player, baseObjVar + OBJVAR_BEEN_WARNED_DISTANCE, "true");
			
			string_id message = new string_id ("quest/groundquests", "static_defense_distance_warning");
			prose_package pp = prose.getPackage(message, player, player, 0);
			prose.setDF(pp, DEFEND_DISTANCE_TIME_CHECK);
			sendSystemMessageProse(player, pp);
		}
		else
		{
			string_id message = new string_id ("quest/groundquests", "static_defense_failed_due_to_distance");
			prose_package pp = prose.getPackage(message, player, player, 0);
			sendSystemMessageProse(player, pp);
			
			questFailTask(questCrc, taskId, player);
		}
	}
	
	
	public int OnTaskCompleted(obj_id self, int questCrc, int taskId) throws InterruptedException
	{
		blog("OnTaskCompleted init");
		groundquests.questOutputDebugInfo(self, questCrc, taskId, TASK_TYPE, "OnTaskCompleted", TASK_TYPE + " task completed.");
		
		String baseObjVar = groundquests.getBaseObjVar(self, TASK_TYPE, questGetQuestName(questCrc), taskId);
		obj_id escort = getObjIdObjVar(self, baseObjVar + DOT + OBJVAR_DEFENSE_TARGET);
		
		if (hasObjVar(escort, "questCallBack"))
		{
			dictionary webster = new dictionary();
			webster.put("questComplete", true);
			webster.put("player", self);
			messageTo(escort, "handleQuestCallBack", webster, 0, false);
		}
		
		cleanup(self, questCrc, taskId);
		return super.OnTaskCompleted(self, questCrc, taskId);
	}
	
	
	public int OnTaskFailed(obj_id self, int questCrc, int taskId) throws InterruptedException
	{
		blog("OnTaskFailed init");
		groundquests.questOutputDebugInfo(self, questCrc, taskId, TASK_TYPE, "OnTaskFailed", TASK_TYPE + " task failed.");
		
		String baseObjVar = groundquests.getBaseObjVar(self, TASK_TYPE, questGetQuestName(questCrc), taskId);
		obj_id escort = getObjIdObjVar(self, baseObjVar + DOT + OBJVAR_DEFENSE_TARGET);
		
		if (hasObjVar(escort, "questCallBack"))
		{
			dictionary webster = new dictionary();
			webster.put("questComplete", false);
			webster.put("player", self);
			messageTo(escort, "handleQuestCallBack", webster, 0, false);
		}
		
		cleanup(self, questCrc, taskId);
		return super.OnTaskFailed(self, questCrc, taskId);
	}
	
	
	public int OnTaskCleared(obj_id self, int questCrc, int taskId) throws InterruptedException
	{
		blog("OnTaskCleared init");
		groundquests.questOutputDebugInfo(self, questCrc, taskId, TASK_TYPE, "OnTaskCleared", TASK_TYPE + " task cleared.");
		
		String baseObjVar = groundquests.getBaseObjVar(self, TASK_TYPE, questGetQuestName(questCrc), taskId);
		obj_id escort = getObjIdObjVar(self, baseObjVar + DOT + OBJVAR_DEFENSE_TARGET);
		
		if (hasObjVar(escort, "questCallBack"))
		{
			dictionary webster = new dictionary();
			webster.put("questComplete", false);
			webster.put("player", self);
			messageTo(escort, "handleQuestCallBack", webster, 0, false);
		}
		
		cleanup(self, questCrc, taskId);
		return super.OnTaskCleared(self, questCrc, taskId);
	}
	
	
	public void cleanup(obj_id player, int questCrc, int taskId) throws InterruptedException
	{
		blog("cleanup init");
		
		String baseObjVar = groundquests.getBaseObjVar(player, TASK_TYPE, questGetQuestName(questCrc), taskId);
		String escortTargetObjVarName = baseObjVar + DOT + OBJVAR_DEFENSE_TARGET;
		
		obj_id escortTarget = getObjIdObjVar(player, escortTargetObjVarName);
		
		if (escortTarget != null)
		{
			removeObjVar(player, escortTargetObjVarName);
			if (!ai_lib.isAiDead(escortTarget))
			{
				
				dictionary params = new dictionary();
				messageTo(escortTarget, "messageEscortCleanup", params, CLEANUP_TIME, false);
				ai_lib.aiStopFollowing(escortTarget);
			}
		}
		
		groundquests.clearBaseObjVar(player, TASK_TYPE, questGetQuestName(questCrc), taskId);
	}
	
	
	public int OnLogout(obj_id self) throws InterruptedException
	{
		blog("OnLogout init");
		groundquests.failAllActiveTasksOfType(self, TASK_TYPE);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		blog("OnInitialize init");
		groundquests.failAllActiveTasksOfType(self, TASK_TYPE);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDetach(obj_id self) throws InterruptedException
	{
		blog("OnDetach init");
		
		removeObjVar(self, groundquests.getTaskTypeObjVar(self, TASK_TYPE));
		return SCRIPT_CONTINUE;
	}
	
	
	public void blog(String text) throws InterruptedException
	{
		if (LOGGING_ON)
		{
			LOG(LOG_CAT, text);
		}
	}
}
