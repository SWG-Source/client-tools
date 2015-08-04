package script.quest.task.ground;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.groundquests;
import script.library.utils;



public class wait_for_signal extends script.quest.task.ground.base_task
{
	public wait_for_signal()
	{
	}
	public static final String dataTableColumnSignalName = "SIGNAL_NAME";
	
	public static final String dataTableColumnWaitMarkerCreate = "WAIT_MARKER_CREATE";
	public static final String dataTableColumnWaitMarkerTemplate = "WAIT_MARKER_TEMPLATE";
	public static final String dataTableColumnWaitMarkerPlanetName = "WAIT_MARKER_PLANET_NAME";
	public static final String dataTableColumnWaitMarkerBuilding = "WAIT_MARKER_BUILDING";
	public static final String dataTableColumnWaitMarkerCellName = "WAIT_MARKER_CELL_NAME";
	public static final String dataTableColumnWaitMarkerX = "WAIT_MARKER_X";
	public static final String dataTableColumnWaitMarkerY = "WAIT_MARKER_Y";
	public static final String dataTableColumnWaitMarkerZ = "WAIT_MARKER_Z";
	public static final String dataTableColumnNumRequired = "NUM_REQUIRED";
	
	public static final String defaultWaitMarkerTemplate = "object/static/structure/general/tutorial_waypoint.iff";
	
	public static final String waitMarkerVarName = "waitMarker";
	
	public static final String taskType = "wait_for_signal";
	public static final String dot = ".";
	
	
	public int OnTaskActivated(obj_id self, int questCrc, int taskId) throws InterruptedException
	{
		groundquests.questOutputDebugInfo(self, questCrc, taskId, taskType, "OnTaskActivated", taskType + " task activated.");
		
		groundquests.setBaseObjVar(self, taskType, questGetQuestName(questCrc), taskId);
		
		createMarker(self, questCrc, taskId);
		
		int count = groundquests.getTaskIntDataEntry(questCrc, taskId, dataTableColumnNumRequired);
		
		if (count > 1)
		{
			questSetQuestTaskCounter(self, questGetQuestName(questCrc), taskId, "quest/groundquests:signals_counter", 0, count);
		}
		
		return super.OnTaskActivated(self, questCrc, taskId);
	}
	
	
	public int OnTaskCompleted(obj_id self, int questCrc, int taskId) throws InterruptedException
	{
		cleanup(self, questCrc, taskId);
		groundquests.questOutputDebugInfo(self, questCrc, taskId, taskType, "OnTaskCompleted", taskType + " task completed.");
		return super.OnTaskCompleted(self, questCrc, taskId);
	}
	
	
	public int OnTaskFailed(obj_id self, int questCrc, int taskId) throws InterruptedException
	{
		cleanup(self, questCrc, taskId);
		groundquests.questOutputDebugInfo(self, questCrc, taskId, taskType, "OnTaskFailed", taskType + " task failed.");
		return super.OnTaskFailed(self, questCrc, taskId);
	}
	
	
	public int OnTaskCleared(obj_id self, int questCrc, int taskId) throws InterruptedException
	{
		cleanup(self, questCrc, taskId);
		groundquests.questOutputDebugInfo(self, questCrc, taskId, taskType, "OnTaskCleared", taskType + " task cleared.");
		return super.OnTaskCleared(self, questCrc, taskId);
	}
	
	
	public int OnDetach(obj_id self) throws InterruptedException
	{
		
		removeObjVar(self, groundquests.getTaskTypeObjVar(self, taskType));
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		groundquests.questOutputDebugLog(taskType, "OnInitialize", "Updating wait markers.");
		
		updateAllMarkers(self, true);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnLogout(obj_id self) throws InterruptedException
	{
		groundquests.questOutputDebugLog(taskType, "OnLogout", "Updating wait markers.");
		
		updateAllMarkers(self, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int questSignal(obj_id self, dictionary params) throws InterruptedException
	{
		String signalReceived = params.getString("signal");
		
		LOG("nym_logging","signalReceived: "+signalReceived);
		
		String taskObjVarName = groundquests.getTaskTypeObjVar(self, taskType);
		
		if (taskObjVarName == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_var_list questList = getObjVarList(self, taskObjVarName);
		
		if (questList == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		int countQuests = questList.getNumItems();
		
		for (int i = 0; i < countQuests; ++i)
		{
			testAbortScript();
			obj_var ov = questList.getObjVar(i);
			String questName = ov.getName();
			int questCrc = questGetQuestId(questName);
			
			String questObjVarName = taskObjVarName + dot + questName;
			
			if (!hasObjVar(self, questObjVarName))
			{
				continue;
			}
			
			obj_var_list taskList = getObjVarList(self, questObjVarName);
			
			if (taskList == null)
			{
				continue;
			}
			
			int countTasks = taskList.getNumItems();
			
			for (int j = 0; j < countTasks; ++j)
			{
				testAbortScript();
				obj_var ov2 = taskList.getObjVar(j);
				String taskName = ov2.getName();
				int taskId = utils.stringToInt(taskName);
				
				String baseObjVar = groundquests.getBaseObjVar(self, taskType, questGetQuestName(questCrc), taskId);
				
				String signalName = groundquests.getTaskStringDataEntry(questCrc, taskId, dataTableColumnSignalName);
				
				if (signalName == null || signalName.equals(""))
				{
					String errText = "SIGNALNAME IS NULL: questCrc: "+ questCrc +
					" - taskId: "+ taskId +
					" - dataTableColumnSignalName: "+ dataTableColumnSignalName;
					logScriptDataError(errText);
				}
				else
				{
					String[] signalNames = split(signalName, ',');
					
					for (int k = 0; k < signalNames.length; k++)
					{
						testAbortScript();
						if (signalNames[k].equals(signalReceived))
						{
							groundquests.questOutputDebugInfo(self, questCrc, taskId, taskType, "questSignal", "Signal "+ signalNames[k] + " received.");
							
							String signalObjvar = baseObjVar + dot + signalReceived + dot + "count";
							
							int signalsMaximum = groundquests.getTaskIntDataEntry(questCrc, taskId, dataTableColumnNumRequired);
							int signalsCompleted = getIntObjVar(self, signalObjvar) + 1;
							
							setObjVar(self, signalObjvar, signalsCompleted);
							
							if (signalsMaximum > 1 && signalsCompleted < signalsMaximum)
							{
								questSetQuestTaskCounter(self, questGetQuestName(questCrc), taskId, "quest/groundquests:signals_counter", signalsCompleted, signalsMaximum);
								play2dNonLoopingSound(self, groundquests.MUSIC_QUEST_INCREMENT_COUNTER);
							}
							else
							{
								questCompleteTask(questCrc, taskId, self);
							}
						}
					}
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public void cleanup(obj_id player, int questCrc, int taskId) throws InterruptedException
	{
		destroyMarker(player, questCrc, taskId);
		
		groundquests.clearBaseObjVar(player, taskType, questGetQuestName(questCrc), taskId);
	}
	
	
	public void createMarker(obj_id player, int questCrc, int taskId) throws InterruptedException
	{
		String baseObjVar = groundquests.getBaseObjVar(player, taskType, questGetQuestName(questCrc), taskId);
		
		boolean createWaitMarker = groundquests.getTaskIntDataEntry(questCrc, taskId, dataTableColumnWaitMarkerCreate) > 0;
		
		if (createWaitMarker)
		{
			
			destroyMarker(player, questCrc, taskId);
			
			String waitMarkerTemplate = groundquests.getTaskStringDataEntry(questCrc, taskId, dataTableColumnWaitMarkerTemplate);
			String waitMarkerPlanetName = groundquests.getTaskStringDataEntry(questCrc, taskId, dataTableColumnWaitMarkerPlanetName);
			String waitMarkerBuilding = groundquests.getTaskStringDataEntry(questCrc, taskId, dataTableColumnWaitMarkerBuilding);
			String waitMarkerCellName = groundquests.getTaskStringDataEntry(questCrc, taskId, dataTableColumnWaitMarkerCellName);
			float waitMarkerX = groundquests.getTaskFloatDataEntry(questCrc, taskId, dataTableColumnWaitMarkerX);
			float waitMarkerY = groundquests.getTaskFloatDataEntry(questCrc, taskId, dataTableColumnWaitMarkerY);
			float waitMarkerZ = groundquests.getTaskFloatDataEntry(questCrc, taskId, dataTableColumnWaitMarkerZ);
			
			location waitMarkerLocation;
			
			if (waitMarkerCellName.length() != 0)
			{
				obj_id buildingObjId;
				
				if (utils.stringToInt(waitMarkerBuilding) == 0)
				{
					buildingObjId = getTopMostContainer(player);
				}
				else
				{
					buildingObjId = utils.stringToObjId(waitMarkerBuilding);
				}
				
				if (!isValidId(buildingObjId))
				{
					groundquests.questOutputDebugLog(taskType, "createMarker", "Invalid building!");
					return;
				}
				
				waitMarkerPlanetName = getLocation(buildingObjId).area;
				
				obj_id cellObjId = getCellId(buildingObjId, waitMarkerCellName);
				
				if (!isValidId(cellObjId))
				{
					groundquests.questOutputDebugLog(taskType, "createMarker", "Invalid cell! "+ waitMarkerCellName);
					return;
				}
				
				waitMarkerLocation = new location(waitMarkerX, waitMarkerY, waitMarkerZ, waitMarkerPlanetName, cellObjId);
			}
			else
			{
				waitMarkerLocation = new location(waitMarkerX, waitMarkerY, waitMarkerZ, waitMarkerPlanetName);
			}
			
			if (waitMarkerTemplate.length() == 0)
			{
				waitMarkerTemplate = defaultWaitMarkerTemplate;
			}
			
			groundquests.questOutputDebugInfo(player, questCrc, taskId, taskType, "OnTaskActivated",
			"Creating wait marker ["+ waitMarkerTemplate + "] at "+ waitMarkerLocation.toString());
			
			obj_id waitMarker = createObject(waitMarkerTemplate, waitMarkerLocation);
			
			setObjVar(player, baseObjVar + dot + waitMarkerVarName, waitMarker);
		}
	}
	
	
	public void destroyMarker(obj_id player, int questCrc, int taskId) throws InterruptedException
	{
		String baseObjVar = groundquests.getBaseObjVar(player, taskType, questGetQuestName(questCrc), taskId);
		obj_id waitMarker = getObjIdObjVar(player, baseObjVar + dot + waitMarkerVarName);
		
		if (isIdValid(waitMarker))
		{
			groundquests.questOutputDebugInfo(player, questCrc, taskId, taskType, "destroyMarker",
			"Destroying wait marker: "+ waitMarker);
			
			destroyObject(waitMarker);
			
			removeObjVar(player, baseObjVar + dot + waitMarkerVarName);
		}
	}
	
	
	public void updateAllMarkers(obj_id player, boolean create) throws InterruptedException
	{
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
					
					if (create)
					{
						createMarker(player, questCrc, taskId);
					}
					else
					{
						destroyMarker(player, questCrc, taskId);
					}
				}
			}
		}
	}
	
}
