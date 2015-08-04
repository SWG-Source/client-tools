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
import script.library.spawning;
import script.library.utils;



public class escort extends script.quest.task.ground.base_task
{
	public escort()
	{
	}
	public static final String dataTableColumnServerTemplate = "SERVER_TEMPLATE";
	public static final String dataTableColumnDestinationNodeName = "DESTINATION_NODE_NAME";
	public static final String dataTableColumnMovementType = "MOVEMENT_TYPE";
	
	public static final String objvarEscortTarget = "target";
	public static final String objvarBeenWarnedDistance = "beenWarned";
	
	public static final String scriptNameEscortOnTarget = "quest.task.ground.escort_on_target";
	
	public static final String taskType = "escort";
	public static final String dot = ".";
	
	public static final float escortDistanceTimeCheck = 10;
	public static final float maximumAllowedDistance = 25;
	public static final float cleanupTime = 120;
	
	
	public int OnTaskActivated(obj_id self, int questCrc, int taskId) throws InterruptedException
	{
		groundquests.questOutputDebugInfo(self, questCrc, taskId, taskType, "OnTaskActivated", taskType + " task activated.");
		
		String baseObjVar = groundquests.setBaseObjVar(self, taskType, questGetQuestName(questCrc), taskId);
		
		String serverTemplate = groundquests.getTaskStringDataEntry(questCrc, taskId, dataTableColumnServerTemplate);
		String destinationNodeName = groundquests.getTaskStringDataEntry(questCrc, taskId, dataTableColumnDestinationNodeName);
		String movementType = groundquests.getTaskStringDataEntry(questCrc, taskId, dataTableColumnMovementType);
		
		location l = groundquests.getRandom2DLocationAroundPlayer(self, 0, 1);
		
		LOG("nym_spawn","serverTemplate: "+serverTemplate);
		
		obj_id escortTarget = obj_id.NULL_ID;
		if (serverTemplate.indexOf(".iff") > -1)
		{
			escortTarget = spawning.createSpawnInLegacyCell(self, l, serverTemplate);
			LOG("nym_spawn","spawing actual template, not creature: "+serverTemplate);
			
		}
		else
		{
			LOG("nym_spawn","spawing creature: "+serverTemplate);
			
			escortTarget = create.createCreature(serverTemplate, l, true);
		}
		if (!isValidId(escortTarget))
		{
			LOG("nym_spawn","spawing creature: "+serverTemplate);
			
			return super.OnTaskActivated(self, questCrc, taskId);
		}
		
		attachScript(escortTarget, scriptNameEscortOnTarget);
		pathTo(escortTarget, getLocation(self));
		
		setMaster(escortTarget, self);
		ai_lib.aiFollow(escortTarget, self, 1.0f, 8.0f);
		
		groundquests.addDestroyNotification(escortTarget, self);
		
		setObjVar(self, baseObjVar + dot + objvarEscortTarget, escortTarget);
		
		dictionary distanceCheckParams = new dictionary();
		distanceCheckParams.put("player", self);
		distanceCheckParams.put("escortTarget", escortTarget);
		distanceCheckParams.put("questCrc", questCrc);
		distanceCheckParams.put("taskId", taskId);
		messageTo(self, "messageEscortCheckDistance", distanceCheckParams, escortDistanceTimeCheck, false);
		
		return super.OnTaskActivated(self, questCrc, taskId);
	}
	
	
	public int destroyNotification(obj_id self, dictionary params) throws InterruptedException
	{
		if (params != null)
		{
			obj_id source = params.getObjId("source");
			
			String taskObjVarName = groundquests.getTaskTypeObjVar(self, taskType);
			if (taskObjVarName != null)
			{
				
				obj_var_list questList = getObjVarList(self, taskObjVarName);
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
									
									String escortTargetObjVarName = questObjVarName + dot + taskName + dot + objvarEscortTarget;
									if (hasObjVar(self, escortTargetObjVarName))
									{
										obj_id escortTarget = getObjIdObjVar(self, escortTargetObjVarName);
										if (source == escortTarget)
										{
											questFailTask(questCrc, taskId, self);
										}
									}
								}
							}
						}
					}
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int messageEscortCheckDistance(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		obj_id escortTarget = params.getObjId("escortTarget");
		int questCrc = params.getInt ("questCrc");
		int taskId = params.getInt ("taskId");
		
		if (questIsTaskActive(questCrc, taskId, player))
		{
			String baseObjVar = groundquests.getBaseObjVar(player, taskType, questGetQuestName(questCrc), taskId);
			if (isEscortNearby(questCrc, taskId, player, escortTarget))
			{
				if (hasObjVar(player, baseObjVar + objvarBeenWarnedDistance))
				{
					removeObjVar(player, baseObjVar + objvarBeenWarnedDistance);
				}
			}
			else
			{
				failEscortDistanceCheck(player, questCrc, taskId, escortTarget);
			}
			
			ai_lib.aiFollow(escortTarget, self, 1.0f, 8.0f);
			
			messageTo(self, "messageEscortCheckDistance", params, escortDistanceTimeCheck, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean isEscortNearby(int questCrc, int taskId, obj_id player, obj_id escortTarget) throws InterruptedException
	{
		boolean result = false;
		if (isIdValid(player) && isIdValid(escortTarget))
		{
			result = getDistance(player, escortTarget) <= maximumAllowedDistance;
		}
		return result;
	}
	
	
	public void failEscortDistanceCheck(obj_id player, int questCrc, int taskId, obj_id escortTarget) throws InterruptedException
	{
		String baseObjVar = groundquests.getBaseObjVar(player, taskType, questGetQuestName(questCrc), taskId);
		if (!hasObjVar(player, baseObjVar + objvarBeenWarnedDistance))
		{
			
			setObjVar(player, baseObjVar + objvarBeenWarnedDistance, "true");
			
			string_id message = new string_id ("quest/groundquests", "static_escort_distance_warning");
			prose_package pp = prose.getPackage(message, player, player, 0);
			prose.setDF(pp, escortDistanceTimeCheck);
			sendSystemMessageProse(player, pp);
		}
		else
		{
			string_id message = new string_id ("quest/groundquests", "static_escort_failed_due_to_distance");
			prose_package pp = prose.getPackage(message, player, player, 0);
			sendSystemMessageProse(player, pp);
			
			questFailTask(questCrc, taskId, player);
		}
	}
	
	
	public int OnTaskCompleted(obj_id self, int questCrc, int taskId) throws InterruptedException
	{
		groundquests.questOutputDebugInfo(self, questCrc, taskId, taskType, "OnTaskCompleted", taskType + " task completed.");
		
		String baseObjVar = groundquests.getBaseObjVar(self, taskType, questGetQuestName(questCrc), taskId);
		obj_id escort = getObjIdObjVar(self, baseObjVar + dot + objvarEscortTarget);
		
		ai_lib.aiStopFollowing(escort);
		
		cleanup(self, questCrc, taskId);
		return super.OnTaskCompleted(self, questCrc, taskId);
	}
	
	
	public int OnTaskFailed(obj_id self, int questCrc, int taskId) throws InterruptedException
	{
		groundquests.questOutputDebugInfo(self, questCrc, taskId, taskType, "OnTaskFailed", taskType + " task failed.");
		
		String baseObjVar = groundquests.getBaseObjVar(self, taskType, questGetQuestName(questCrc), taskId);
		obj_id escort = getObjIdObjVar(self, baseObjVar + dot + objvarEscortTarget);
		
		ai_lib.aiStopFollowing(escort);
		
		cleanup(self, questCrc, taskId);
		return super.OnTaskFailed(self, questCrc, taskId);
	}
	
	
	public int OnTaskCleared(obj_id self, int questCrc, int taskId) throws InterruptedException
	{
		groundquests.questOutputDebugInfo(self, questCrc, taskId, taskType, "OnTaskCleared", taskType + " task cleared.");
		
		String baseObjVar = groundquests.getBaseObjVar(self, taskType, questGetQuestName(questCrc), taskId);
		obj_id escort = getObjIdObjVar(self, baseObjVar + dot + objvarEscortTarget);
		
		ai_lib.aiStopFollowing(escort);
		
		cleanup(self, questCrc, taskId);
		return super.OnTaskCleared(self, questCrc, taskId);
	}
	
	
	public void cleanup(obj_id player, int questCrc, int taskId) throws InterruptedException
	{
		
		String baseObjVar = groundquests.getBaseObjVar(player, taskType, questGetQuestName(questCrc), taskId);
		String escortTargetObjVarName = baseObjVar + dot + objvarEscortTarget;
		
		obj_id escortTarget = getObjIdObjVar(player, escortTargetObjVarName);
		
		if (isValidId(escortTarget))
		{
			removeObjVar(player, escortTargetObjVarName);
			if (!ai_lib.isAiDead(escortTarget))
			{
				
				dictionary params = new dictionary();
				messageTo(escortTarget, "messageEscortCleanup", params, cleanupTime, false);
				ai_lib.aiStopFollowing(escortTarget);
			}
		}
		
		groundquests.clearBaseObjVar(player, taskType, questGetQuestName(questCrc), taskId);
	}
	
	
	public int OnLogout(obj_id self) throws InterruptedException
	{
		groundquests.failAllActiveTasksOfType(self, taskType);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		groundquests.failAllActiveTasksOfType(self, taskType);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDetach(obj_id self) throws InterruptedException
	{
		
		removeObjVar(self, groundquests.getTaskTypeObjVar(self, taskType));
		return SCRIPT_CONTINUE;
	}
	
}
