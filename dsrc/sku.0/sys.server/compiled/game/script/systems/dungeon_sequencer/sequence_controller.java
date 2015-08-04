package script.systems.dungeon_sequencer;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.buff;
import script.library.chat;
import script.library.combat;
import script.library.create;
import script.library.factions;
import script.library.instance;
import script.library.movement;
import script.library.restuss_event;
import script.library.trial;
import script.library.utils;


public class sequence_controller extends script.base_script
{
	public sequence_controller()
	{
	}
	public static final String[] TRIGGER_DATA_TYPES = 
	{
		"name",
		"triggerInterest",
		"size",
		"occurance",
		"triggerDelay",
		"triggerType"
	};
	
	public static final int TYPE_AI = 0;
	public static final int TYPE_TRIGGER = 1;
	public static final int TYPE_EFFECT_MANAGER = 2;
	public static final int TYPE_OBJECT = 3;
	
	public static final String WFC_MASTER = "wait_for_completion";
	public static final String WFC_TREE = "wait_for_completion_tree";
	public static final String ABSOLUTE_TIMER = "absolute_time_sent_record";
	
	public static final String TABLE_DATA = "instance_data.table_data";
	
	public static final boolean LOGGING = true;
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		if (hasObjVar(self, "autoBeginSpawn"))
		{
			messageTo(self, "beginSpawn", null, 1.0f, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		
		if (hasObjVar(self, "autoBeginSpawn"))
		{
			messageTo(self, "beginSpawn", null, 1.0f, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean isController(obj_id object) throws InterruptedException
	{
		return hasScript(object, "systems.dungeon_sequencer.sequence_controller");
	}
	
	
	public void cacheInstanceTable() throws InterruptedException
	{
		if (!isController(getSelf()))
		{
			return;
		}
		
		String table = getStringObjVar(getSelf(), "instance.data_table");
		
		if (table == null || table.length() < 1)
		{
			CustomerServiceLog("instance", "sequence_controller.cacheInstanceTable missing table name");
			return;
		}
		
		String[] object = dataTableGetStringColumn(table, "object");
		
		if (object == null || object.length < 1)
		{
			CustomerServiceLog("instance", "sequence_controller.cacheInstanceTable missing object column");
			return;
		}
		
		dictionary dict = new dictionary();
		dictionary child_list = new dictionary();
		
		dict.put("object", object);
		dict.put("spawn_id", dataTableGetStringColumn(table, "spawn_id"));
		String[] triggerId = dataTableGetStringColumn(table, "triggerId");
		
		dict.put("triggerId", triggerId);
		String[] room = dataTableGetStringColumn(table, "room");
		dict.put("room", room);
		float[] loc_x = dataTableGetFloatColumn(table, "loc_x");
		dict.put("loc_x", loc_x);
		float[] loc_y = dataTableGetFloatColumn(table, "loc_y");
		dict.put("loc_y", loc_y);
		float[] loc_z = dataTableGetFloatColumn(table, "loc_z");
		dict.put("loc_z", loc_z);
		
		dict.put("yaw", dataTableGetFloatColumn(table, "yaw"));
		dict.put("script", dataTableGetStringColumn(table, "script"));
		dict.put("spawn_objvar", dataTableGetStringColumn(table, "spawn_objvar"));
		dict.put("trigger_event", dataTableGetStringColumn(table, "trigger_event"));
		dict.put("isInvulnerable", dataTableGetIntColumn(table, "isInvulnerable"));
		dict.put("respawn", dataTableGetIntColumn(table, "respawn"));
		dict.put("row", fillRowArray(dataTableGetNumRows(table)));
		
		String[] pp_location = getPathPointLocationArrayKeyList(object, room, loc_x, loc_y, loc_z);
		String[] path_data = getFullPathData(getSelf());
		
		setFullPathDataDictionary(pp_location, path_data);
		
		if (dataTableHasColumn(table, "mission_critical"))
		{
			dict.put("mission_critical", dataTableGetIntColumn(table, "mission_critical"));
		}
		
		dict = getTriggerKeyList(dict, triggerId);
		
		utils.setScriptVar(getSelf(), TABLE_DATA, dict);
		utils.setScriptVar(getSelf(), trial.PROT_CHILD_ARRAY, child_list);
	}
	
	
	public dictionary getTriggerKeyList(dictionary dict, String[] triggerId) throws InterruptedException
	{
		Vector trigger_key = new Vector();
		trigger_key.setSize(0);
		Vector key_list = new Vector();
		key_list.setSize(0);
		
		for (int i=0; i<triggerId.length; i++)
		{
			testAbortScript();
			key_list.clear();
			
			if (!trigger_key.contains(triggerId[i]))
			{
				trigger_key.add(triggerId[i]);
				key_list.add(""+i);
				dict.put("triggerId-"+triggerId[i], key_list);
			}
			else
			{
				key_list = dict.getResizeableStringArray("triggerId-"+triggerId[i]);
				key_list.add(""+i);
				dict.put("triggerId-"+triggerId[i], key_list);
			}
			
		}
		
		return dict;
	}
	
	
	public dictionary cachedDataGetRow(int row) throws InterruptedException
	{
		if (!isController(getSelf()))
		{
			return null;
		}
		
		dictionary cached_data = utils.getDictionaryScriptVar(getSelf(), TABLE_DATA);
		dictionary row_data = new dictionary();
		
		if (row < 0)
		{
			return null;
		}
		
		String[] object = cached_data.getStringArray("object");
		String[] spawn_id = cached_data.getStringArray("spawn_id");
		String[] triggerId = cached_data.getStringArray("triggerId");
		String[] room = cached_data.getStringArray("room");
		float[] loc_x = cached_data.getFloatArray("loc_x");
		float[] loc_y = cached_data.getFloatArray("loc_y");
		float[] loc_z = cached_data.getFloatArray("loc_z");
		float[] yaw = cached_data.getFloatArray("yaw");
		String[] script = cached_data.getStringArray("script");
		String[] spawn_objvar = cached_data.getStringArray("spawn_objvar");
		String[] trigger_event = cached_data.getStringArray("trigger_event");
		int[] isInvulnerable = cached_data.getIntArray("isInvulnerable");
		int[] respawn = cached_data.getIntArray("respawn");
		int[] spawn_row = cached_data.getIntArray("row");
		
		row_data.put("object", object[row]);
		row_data.put("spawn_id", spawn_id[row]);
		row_data.put("triggerId", triggerId[row]);
		row_data.put("room", room[row]);
		row_data.put("loc_x", loc_x[row]);
		row_data.put("loc_y", loc_y[row]);
		row_data.put("loc_z", loc_z[row]);
		row_data.put("yaw", yaw[row]);
		row_data.put("script", script[row]);
		row_data.put("spawn_objvar", spawn_objvar[row]);
		row_data.put("trigger_event", trigger_event[row]);
		row_data.put("isInvulnerable", isInvulnerable[row]);
		row_data.put("respawn", respawn[row]);
		row_data.put("row", spawn_row[row]);
		
		if (cached_data.containsKey("mission_critical"))
		{
			int[] mission_critical = cached_data.getIntArray("mission_critical");
			row_data.put("mission_critical", mission_critical[row]);
		}
		
		return row_data;
		
	}
	
	
	public int beginSpawn(obj_id self, dictionary params) throws InterruptedException
	{
		handleCleanup(self);
		cacheInstanceTable();
		messageTo(self, "delaySpawnActors", params != null ? params : null, 5.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int delaySpawnActors(obj_id self, dictionary params) throws InterruptedException
	{
		String defaultTrigger = "default";
		if (params != null && !params.isEmpty())
		{
			defaultTrigger = params.getString("defaultTrigger");
			defaultTrigger = defaultTrigger.length() > 0 && !defaultTrigger.equals("") ? defaultTrigger : "default";
		}
		
		spawnActors(self, defaultTrigger);
		return SCRIPT_CONTINUE;
	}
	
	
	public int cleanupSpawn(obj_id self, dictionary params) throws InterruptedException
	{
		handleCleanup(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		handleCleanup(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleCleanup(obj_id self) throws InterruptedException
	{
		trial.bumpSession(self);
		obj_id[] children = getSpawnedChildren(self);
		
		if (children != null && children.length > 0)
		{
			for (int i=0; i<children.length; i++)
			{
				testAbortScript();
				if (hasScript(children[i], "systems.dungeon_sequencer.sequence_controller"))
				{
					obj_id[] cells = getCellIds(children[i]);
					
					if (cells == null || cells.length == 0)
					{
						destroyObject(children[i]);
					}
					else
					{
						obj_id[] objects = trial.getAllObjectsInDungeon(children[i]);
						
						if (objects != null && objects.length > 0)
						{
							for (int q=0; q<objects.length; q++)
							{
								testAbortScript();
								if (isPlayer(objects[q]))
								{
									expelFromBuilding(objects[q]);
								}
							}
						}
						if (!exists(children[i]))
						{
							LIVE_LOG("handleCleanup::exists", "Tried to send a message to to an object that no longer exists on this game server");
						}
						messageTo(children[i], "handleDelayCleanup", null, 1.0f, false);
					}
				}
				else
				{
					trial.cleanupObject(children[i]);
				}
			}
		}
		
		utils.removeScriptVarTree(self, trial.PROT_CHILD);
		utils.removeScriptVarTree(self, WFC_MASTER);
		utils.removeScriptVarTree(self, WFC_TREE);
		utils.removeScriptVar(self, ABSOLUTE_TIMER);
		utils.removeScriptVar(self, trial.PATROL_PATH_FULL_DATA);
		utils.removeScriptVar(self, trial.SEQUENCER_PATH_DATA);
		utils.removeScriptVarTree(self, TABLE_DATA);
		
		obj_id[] tempObjects = getObjectsInRange(getLocation(self), 1000.0f);
		
		if (tempObjects != null && tempObjects.length > 0)
		{
			for (int k=0; k<tempObjects.length; k++)
			{
				testAbortScript();
				if (!isIdValid(tempObjects[k]) || !exists(tempObjects[k]))
				{
					continue;
				}
				
				if (!trial.isTempObject(tempObjects[k]))
				{
					continue;
				}
				
				if (trial.getParent(tempObjects[k]) != self)
				{
					continue;
				}
				
				if (hasScript(tempObjects[k], "systems.dungeon_sequencer.sequence_controller"))
				{
					obj_id[] cells = getCellIds(tempObjects[k]);
					
					if (cells == null || cells.length == 0)
					{
						destroyObject(tempObjects[k]);
					}
					else
					{
						obj_id[] objects = trial.getAllObjectsInDungeon(tempObjects[k]);
						
						if (objects != null && objects.length > 0)
						{
							for (int q=0; q<objects.length; q++)
							{
								testAbortScript();
								if (isPlayer(objects[q]))
								{
									expelFromBuilding(objects[q]);
								}
							}
						}
						
						if (!exists(tempObjects[k]))
						{
							LIVE_LOG("handleCleanup::exists", "Tried to send a message to to an object that no longer exists on this game server");
						}
						
						messageTo(tempObjects[k], "handleDelayCleanup", null, 1.0f, false);
					}
				}
				else
				{
					trial.cleanupObject(tempObjects[k]);
				}
			}
		}
		
		obj_id[] selfCellList = getCellIds(self);
		
		if (selfCellList != null && selfCellList.length > 0)
		{
			for (int r=0; r<selfCellList.length; r++)
			{
				testAbortScript();
				permissionsMakePublic(selfCellList[r]);
			}
		}
		
		return;
	}
	
	
	public int handleDelayCleanup(obj_id self, dictionary params) throws InterruptedException
	{
		destroyObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int restartSpawn(obj_id self, dictionary params) throws InterruptedException
	{
		messageTo(self, "cleanupSpawn", null, 0.0f, false);
		messageTo(self, "beginSpawn", null, 5.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public obj_id[] getSpawnedChildren(obj_id self) throws InterruptedException
	{
		
		dictionary spawnList = utils.getDictionaryScriptVar(self, trial.PROT_CHILD_ARRAY);
		if (spawnList == null || spawnList.isEmpty())
		{
			return null;
		}
		
		Vector keyList = spawnList.getResizeableStringArray(trial.PROT_CHILD_KEY_LIST);
		
		Vector allChildren = new Vector();
		allChildren.setSize(0);
		
		if (keyList == null || keyList.size() == 0)
		{
			return null;
		}
		
		for (int i=0; i<keyList.size(); i++)
		{
			testAbortScript();
			Vector thisList = spawnList.getResizeableObjIdArray(((String)(keyList.get(i))));
			
			if (thisList == null || thisList.size() == 0)
			{
				continue;
			}
			
			allChildren.addAll(thisList);
		}
		
		obj_id[] returnList = new obj_id[0];
		if (allChildren != null)
		{
			returnList = new obj_id[allChildren.size()];
			allChildren.toArray(returnList);
		}
		
		return returnList;
	}
	
	
	public void removeSpawnChild(obj_id object, String spawn_id) throws InterruptedException
	{
		
		dictionary spawnList = utils.getDictionaryScriptVar(getSelf(), trial.PROT_CHILD_ARRAY);
		
		if (spawnList == null || spawnList.isEmpty())
		{
			return;
		}
		
		Vector keyList = spawnList.getResizeableStringArray(trial.PROT_CHILD_KEY_LIST);
		
		if (!spawnList.containsKey(spawn_id))
		{
			return;
		}
		
		Vector thisList = spawnList.getResizeableObjIdArray(spawn_id);
		
		if (thisList != null && thisList.contains(object))
		{
			thisList.remove(object);
		}
		else
		{
			return;
		}
		
		if (thisList.isEmpty())
		{
			if (keyList != null)
			{
				keyList.remove(spawn_id);
			}
			
			spawnList.remove(spawn_id);
		}
		else
		{
			spawnList.put(spawn_id, thisList);
		}
		
		utils.setScriptVar(getSelf(), trial.PROT_CHILD_ARRAY, spawnList);
	}
	
	
	public void spawnActors(obj_id self, String triggerId) throws InterruptedException
	{
		if (!isController(self))
		{
			return;
		}
		
		dictionary dict = utils.getDictionaryScriptVar(getSelf(), TABLE_DATA);
		
		String[] trigger_rows = dict.getStringArray("triggerId-"+triggerId);
		
		if (trigger_rows == null || trigger_rows.length == 0)
		{
			doLogging("spawnActors", "No such tableData triggerId-"+triggerId);
			return;
		}
		
		for (int i=0; i<trigger_rows.length; i++)
		{
			testAbortScript();
			spawnActorRow(utils.stringToInt(trigger_rows[i]));
		}
		
	}
	
	
	public void spawnActorRow(int row) throws InterruptedException
	{
		dictionary dict = cachedDataGetRow(row);
		
		if (dict == null)
		{
			return;
		}
		
		spawnActorRow(dict);
	}
	
	
	public void spawnActorRow(dictionary dict) throws InterruptedException
	{
		boolean newPathData = false;
		obj_id self = getSelf();
		String SPAWN_TABLE = getStringObjVar(getSelf(), "instance.data_table");
		
		String object = dict.getString("object");
		String nameOverride = dict.getString("name");
		String room = dict.getString("room");
		float locX = dict.getFloat("loc_x");
		float locY = dict.getFloat("loc_y");
		float locZ = dict.getFloat("loc_z");
		float yaw = dict.getFloat("yaw");
		String spawnObjVar = dict.getString("spawn_objvar");
		String spawnScript = dict.getString("script");
		int isInvulnerable = dict.getInt("isInvulnerable");
		boolean makeInvulnerable = isInvulnerable == 1;
		String triggerEventData = dict.getString("trigger_event");
		String spawn_id = dict.getString("spawn_id");
		int respawn = dict.getInt("respawn");
		int row = dict.getInt("row");
		boolean missionCritical = dict.containsKey("mission_critical") ? dict.getInt("mission_critical") == 1 : false;
		obj_id newObject = null;
		location here = getLocation(getSelf());
		boolean indoor = false;
		location spawnLoc = null;
		
		PROFILER_START("SPAWN_ACTOR_ROW");
		
		if (room.equals("none"))
		{
			if (!hasObjVar(self, "worldLoc"))
			{
				locX = here.x + locX;
				locY = here.y + locY;
				locZ = here.z + locZ;
			}
			
			spawnLoc = new location(locX, locY, locZ, here.area);
		}
		else
		{
			indoor = true;
			spawnLoc = new location(locX, locY, locZ, here.area, getCellId(getSelf(), room));
		}
		
		int objType = -1;
		
		if (object.startsWith("object/"))
		{
			PROFILER_START("OBJECT");
			
			if (!indoor)
			{
				newObject = create.object(object, spawnLoc);
			}
			else
			{
				newObject = createObjectInCell(object, getSelf(), room, spawnLoc);
			}
			
			if (!isIdValid(newObject))
			{
				PROFILER_STOP("OBJECT");
				PROFILER_STOP("SPAWN_ACTOR_ROW");
				return;
			}
			
			objType = TYPE_OBJECT;
			newPathData = true;
			PROFILER_STOP("OBJECT");
		}
		else if (object.startsWith("trigger"))
		{
			PROFILER_START("TRIGGER");
			dictionary triggerData = parseTriggerData(object);
			
			if (!indoor)
			{
				newObject = createObject("object/tangible/theme_park/invisible_object.iff", spawnLoc);
			}
			else
			{
				newObject = createObjectInCell("object/tangible/theme_park/invisible_object.iff", getSelf(), room, spawnLoc);
			}
			
			trial.setInterest(newObject);
			messageTo(newObject, "triggerSetup", triggerData, 1, false);
			objType = TYPE_TRIGGER;
			PROFILER_STOP("TRIGGER");
		}
		else if (object.startsWith("clientfx"))
		{
			PROFILER_START("CLIENT_FX");
			if (!indoor)
			{
				newObject = createObject("object/tangible/theme_park/invisible_object.iff", spawnLoc);
			}
			else
			{
				newObject = createObjectInCell("object/tangible/theme_park/invisible_object.iff", getSelf(), room, spawnLoc);
			}
			
			objType = TYPE_EFFECT_MANAGER;
			setClientEffectData(newObject, object);
			setPlayerListOnObject(self, newObject);
			PROFILER_STOP("CLIENT_FX");
		}
		else if (object.startsWith("messageTo"))
		{
			PROFILER_START("MESSAGE_TO");
			doMessageTo(object);
			PROFILER_STOP("MESSAGE_TO");
			PROFILER_STOP("SPAWN_ACTOR_ROW");
			
			return;
		}
		else if (object.startsWith("music"))
		{
			PROFILER_START("MUSIC");
			doPlayMusicInArea(object);
			PROFILER_STOP("MUSIC");
			PROFILER_STOP("SPAWN_ACTOR_ROW");
			return;
		}
		else if (object.startsWith("fxOnPlayers"))
		{
			PROFILER_START("FX_ON_PLAYERS");
			doPlayEffectOnPlayers(object, spawnLoc);
			PROFILER_STOP("FX_ON_PLAYERS");
			PROFILER_STOP("SPAWN_ACTOR_ROW");
			return;
		}
		else if (object.startsWith("labeledFxOnPlayers"))
		{
			PROFILER_START("LABELED_FX_ON_PLAYERS");
			doPlayEffectInAreaWithLabel(object, spawnLoc);
			PROFILER_STOP("LABELED_FX_ON_PLAYERS");
			PROFILER_STOP("SPAWN_ACTOR_ROW");
			return;
		}
		else if (object.startsWith("deleteSpawn"))
		{
			PROFILER_START("DELETE_SPAWN");
			doDeleteSpawn(object);
			PROFILER_STOP("DELETE_SPAWN");
			PROFILER_STOP("SPAWN_ACTOR_ROW");
			return;
		}
		else if (object.startsWith("waitForComplete"))
		{
			PROFILER_START("WAIT_FOR_COMPLETE");
			setUpWaitTree(object);
			PROFILER_STOP("WAIT_FOR_COMPLETE");
			PROFILER_STOP("SPAWN_ACTOR_ROW");
			return;
		}
		else if (object.startsWith("signalComplete"))
		{
			PROFILER_START("SIGNAL_COMPLETE");
			sendTaskCompleteSignal(object);
			PROFILER_STOP("SIGNAL_COMPLETE");
			PROFILER_STOP("SPAWN_ACTOR_ROW");
			return;
		}
		else if (object.startsWith("pathPoint"))
		{
			PROFILER_START("PATH_POINT");
			newObject = establishPatrolPoint(object, spawnLoc, indoor);
			PROFILER_STOP("PATH_POINT");
			newPathData = true;
		}
		else if (object.startsWith("delayAction"))
		{
			PROFILER_START("DELAY_ACTION");
			queueDelayAction(object);
			PROFILER_STOP("DELAY_ACTION");
			PROFILER_STOP("SPAWN_ACTOR_ROW");
			return;
		}
		else if (object.startsWith("signalMaster"))
		{
			PROFILER_START("SIGNAL_MASTER");
			signalMaster(object);
			PROFILER_STOP("SIGNAL_MASTER");
			PROFILER_STOP("SPAWN_ACTOR_ROW");
			
			return;
		}
		else if (object.startsWith("randomTrigger"))
		{
			PROFILER_START("RANDOM_TRIGGER");
			executeRandomTrigger(object);
			PROFILER_STOP("RANDOM_TRIGGER");
			PROFILER_STOP("SPAWN_ACTOR_ROW");
			return;
		}
		else if (object.startsWith("messagePlayers"))
		{
			PROFILER_START("MESSAGE_PLAYERS");
			sendInstanceMessage(object);
			PROFILER_STOP("MESSAGE_PLAYERS");
			PROFILER_STOP("SPAWN_ACTOR_ROW");
			return;
		}
		else if (object.startsWith("doTheaterAction"))
		{
			PROFILER_START("DO_THEATER_ACTION");
			performTheaterAction(self, object);
			PROFILER_STOP("DO_THEATER_ACTION");
			PROFILER_STOP("SPAWN_ACTOR_ROW");
			return;
		}
		else if (object.startsWith("bumpSession"))
		{
			PROFILER_START("BUMP_SESSION");
			String[] session_split = split(object, ':');
			String channel = session_split[1];
			if (channel.equals("default"))
			{
				trial.bumpSession(self);
			}
			else
			{
				trial.bumpSession(self, channel);
			}
			PROFILER_STOP("SPAWN_ACTOR_ROW");
			PROFILER_STOP("BUMP_SESSION");
			return;
		}
		else if (object.startsWith("objVarAction"))
		{
			PROFILER_START("OBJ_VAR_ACTION");
			doObjVarAction(self, object);
			PROFILER_STOP("OBJ_VAR_ACTION");
			PROFILER_STOP("SPAWN_ACTOR_ROW");
			
			return;
		}
		else if (object.startsWith("stringAction"))
		{
			PROFILER_START("STRING_ACTION");
			doScriptAction(self, object);
			PROFILER_STOP("STRING_ACTION");
			PROFILER_STOP("SPAWN_ACTOR_ROW");
			
			return;
		}
		else if (object.startsWith("buffHandler"))
		{
			PROFILER_START("BUFF_HANDLER");
			setBuffHandler(self, object);
			PROFILER_STOP("BUFF_HANDLER");
			PROFILER_STOP("SPAWN_ACTOR_ROW");
			
			return;
		}
		else if (object.startsWith("cellPermission"))
		{
			PROFILER_START("CELL_PERMISSION");
			setCellPermission(self, object);
			PROFILER_STOP("CELL_PERMISSION");
			PROFILER_STOP("SPAWN_ACTOR_ROW");
			
			return;
		}
		else if (object.startsWith("taskId"))
		{
			PROFILER_START("TASK_ID");
			String[] the_split = split(object, ':');
			dict.put("waitForComplete", the_split[1]);
			messageTo(self, "waitForComplete", dict, 1.0f, false);
			PROFILER_STOP("TASK_ID");
			PROFILER_STOP("SPAWN_ACTOR_ROW");
			
			return;
		}
		else if (object.startsWith("setCellLabel"))
		{
			PROFILER_START("SET_CELL_LABEL");
			handleSetCellLabel(self, object);
			PROFILER_STOP("SET_CELL_LABEL");
			PROFILER_STOP("SPAWN_ACTOR_ROW");
			
			return;
		}
		else
		{
			PROFILER_START("MAKE_CREATURE");
			
			String[] multiSpawn = split(object, ',');
			if (multiSpawn.length == 1)
			{
				newObject = create.object (object, spawnLoc);
			}
			else
			{
				newObject = doRandomSpawn(multiSpawn, spawnLoc);
			}
			if (!isIdValid(newObject))
			{
				PROFILER_STOP("MAKE_CREATURE");
				PROFILER_STOP("SPAWN_ACTOR_ROW");
				return;
			}
			
			combat.setPersistCombatMode(newObject, true);
			objType = TYPE_AI;
			PROFILER_STOP("MAKE_CREATURE");
		}
		
		if (missionCritical)
		{
			updateNetworkTriggerVolume(newObject, 1024.0f);
		}
		
		PROFILER_START("LOWER_1");
		setYaw(newObject, yaw);
		setObjVar(newObject, "yaw", yaw);
		trial.markAsTempObject(newObject, true);
		trial.setParent(self, newObject, true);
		trial.setInterest(newObject);
		PROFILER_STOP("LOWER_1");
		PROFILER_START("LOWER_2");
		setSpawnObjVar(newObject, spawnObjVar);
		copyObjVar(self, newObject, SPAWN_TABLE);
		attachSpawnScripts(newObject, spawnScript, objType);
		setTriggerEventData(newObject, triggerEventData);
		PROFILER_STOP("LOWER_2");
		
		PROFILER_START("SET_SPAWN_OBJVAR");
		if (!spawn_id.equals("none"))
		{
			setObjVar(newObject, "spawn_id", spawn_id);
		}
		PROFILER_STOP("SET_SPAWN_OBJVAR");
		
		PROFILER_START("SET_RESPAWN");
		setObjVar(newObject, "respawn", respawn);
		PROFILER_STOP("SET_RESPAWN");
		
		PROFILER_START("STORE_SPAWN_CHILD");
		storeSpawnedChild(self, newObject, spawn_id);
		PROFILER_STOP("STORE_SPAWN_CHILD");
		
		PROFILER_START("SET_ROW");
		setObjVar(newObject, "row", row);
		PROFILER_STOP("SET_ROW");
		
		PROFILER_START("LOWER_4");
		
		if (objType == TYPE_AI)
		{
			setHibernationDelay(newObject, 3600.0f);
			
			dictionary path_data = utils.hasScriptVar(self, trial.SEQUENCER_PATH_DATA) ? utils.getDictionaryScriptVar(self, trial.SEQUENCER_PATH_DATA) : null;
			
			if (path_data != null && !path_data.isEmpty())
			{
				utils.setScriptVar(newObject, trial.SEQUENCER_PATH_DATA, utils.getDictionaryScriptVar(self, trial.SEQUENCER_PATH_DATA));
			}
			
		}
		PROFILER_STOP("LOWER_4");
		PROFILER_START("LOWER_5");
		if (makeInvulnerable)
		{
			setInvulnerable(newObject, true);
		}
		
		if (newPathData)
		{
			setWayPointLinks(self);
		}
		PROFILER_STOP("LOWER_5");
		PROFILER_STOP("SPAWN_ACTOR_ROW");
		
		return;
	}
	
	
	public void attachSpawnScripts(obj_id subject, String spawnScripts) throws InterruptedException
	{
		attachSpawnScripts(subject, spawnScripts, -1);
	}
	
	
	public void attachSpawnScripts(obj_id subject, String spawnScripts, int type) throws InterruptedException
	{
		if (type > -1)
		{
			switch (type)
			{
				case TYPE_AI:
				attachScript(subject, "systems.dungeon_sequencer.ai_controller");
				break;
				
				case TYPE_TRIGGER:
				attachScript(subject, "theme_park.restuss_event.trigger_controller");
				break;
				case TYPE_EFFECT_MANAGER:
				attachScript(subject, "theme_park.restuss_event.restuss_clientfx_controller");
				break;
				
				case TYPE_OBJECT:
				attachScript(subject, "systems.dungeon_sequencer.ai_controller");
				break;
				
			}
			
		}
		
		if (spawnScripts == null || spawnScripts.equals("none"))
		{
			return;
		}
		
		String[] scripts = split(spawnScripts,',');
		for (int q=0; q<scripts.length; q++)
		{
			testAbortScript();
			attachScript(subject, scripts[q]);
		}
	}
	
	
	public void setSpawnObjVar(obj_id newObject, String objvarString) throws InterruptedException
	{
		if (objvarString == null || objvarString.equals("none"))
		{
			return;
		}
		String[] parse = split(objvarString, ',');
		
		if (parse == null || parse.length == 0)
		{
			return;
		}
		
		for (int i=0; i<parse.length; i++)
		{
			testAbortScript();
			String[] typeDataSplit = split(parse[i], ':');
			String type = typeDataSplit[0];
			String data = typeDataSplit[1];
			
			String[] nameValueSplit = split(data, '=');
			String name = nameValueSplit[0];
			String value = nameValueSplit[1];
			
			if (type.equals("int"))
			{
				setObjVar(newObject, name, utils.stringToInt(value));
			}
			if (type.equals("float"))
			{
				setObjVar(newObject, name, utils.stringToFloat(value));
			}
			if (type.equals("string"))
			{
				setObjVar(newObject, name, value);
			}
			if (type.equals("boolean") && (value.equals("true")|| value.equals("1")))
			{
				setObjVar(newObject, name, true);
			}
			if (type.equals("boolean") && (value.equals("false") || value.equals("0")))
			{
				setObjVar(newObject, name, false);
			}
		}
		
	}
	
	
	public dictionary parseTriggerData(String data) throws InterruptedException
	{
		if (data == null || data.equals(""))
		{
			doLogging("parseTriggerData", "Tried to parse an empty dataset");
			return null;
		}
		
		dictionary dict = new dictionary();
		
		String[] parse = split(data, ':');
		
		if (parse == null || parse.length == 0)
		{
			doLogging("parseTriggerData", "Failed to parse dataset");
			return null;
		}
		
		for (int i=0; i<parse.length-1; i++)
		{
			testAbortScript();
			dict.put(TRIGGER_DATA_TYPES[i], parse[i+1]);
		}
		
		return dict;
		
	}
	
	
	public void setClientEffectData(obj_id object, String passedString) throws InterruptedException
	{
		String[] parse = split(passedString, ':');
		if (parse == null || parse.length < 3)
		{
			return;
		}
		
		setObjVar(object, restuss_event.EFFECT_NAME, parse[1]);
		setObjVar(object, restuss_event.EFFECT_VISABILITY, parse[2]);
		
		if (parse.length == 4)
		{
			setObjVar(object, restuss_event.EFFECT_DELTA, parse[3]);
		}
		else
		{
			setObjVar(object, restuss_event.EFFECT_DELTA, "0");
		}
	}
	
	
	public void setTriggerEventData(obj_id object, String passedEventData) throws InterruptedException
	{
		if (passedEventData == null || passedEventData.equals("none"))
		{
			return;
		}
		
		String[] allEntries = split(passedEventData, ',');
		
		for (int i=0; i<allEntries.length; i++)
		{
			testAbortScript();
			storeTriggerEvents(object, allEntries[i]);
		}
	}
	
	
	public int triggerFired(obj_id self, dictionary params) throws InterruptedException
	{
		if (!trial.verifySession(self, params))
		{
			return SCRIPT_CONTINUE;
		}
		String triggerName = params.getString("triggerName");
		String triggerType = params.getString("triggerType");
		
		if (triggerType.equals("triggerId"))
		{
			spawnActors(self, triggerName);
			return SCRIPT_CONTINUE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public void doMessageTo(String message) throws InterruptedException
	{
		int SPAWN_ID = 1;
		
		String[] completeParse = split(message, ';');
		if (completeParse == null || completeParse.length == 0)
		{
			doLogging("doMessageTo", "Failed to parse message");
			return;
		}
		
		obj_id[] spawn_id = getObjectsInSpawnedListWithObjVar(getSelf(), "spawn_id");
		
		if (spawn_id == null || spawn_id.length == 0)
		{
			doLogging("doMessageTo", "No spawn_id could be found");
			return;
		}
		
		for (int i=0; i<spawn_id.length; i++)
		{
			testAbortScript();
			String checkSpawn = getStringObjVar(spawn_id[i], "spawn_id");
			
			if (checkSpawn.equals(completeParse[SPAWN_ID]))
			{
				_doMessageTo(spawn_id[i], completeParse);
			}
		}
		
	}
	
	
	public void doMessgeTo(obj_id target, String message) throws InterruptedException
	{
		_doMessageTo(target, split(message, ';'));
	}
	
	
	public void _doMessageTo(obj_id target, String[] completeParse) throws InterruptedException
	{
		int HANDLER_NAME = 2;
		int PARAMS = 3;
		
		String handlerName = completeParse[HANDLER_NAME];
		dictionary dict = trial.getSessionDict(target);
		
		if (!completeParse[PARAMS].equals("none"))
		{
			String[] paramsParse = split(completeParse[PARAMS], ',');
			{
				for (int k=0; k<paramsParse.length; k++)
				{
					testAbortScript();
					String[] parse = split(paramsParse[k], ':');
					String paramType = parse[0];
					
					String[] valueSplit = split(parse[1], '=');
					String elementName = valueSplit[0];
					String elementValue = valueSplit[1];
					
					if (paramType.equals("int"))
					{
						dict.put(elementName, utils.stringToInt(elementValue));
					}
					if (paramType.equals("float"))
					{
						dict.put(elementName, utils.stringToFloat(elementValue));
					}
					else
					{
						dict.put(elementName, elementValue);
					}
				}
			}
		}
		
		messageTo(target, handlerName, dict, 0.0f, false);
		
	}
	
	
	public void signalMaster(String message) throws InterruptedException
	{
		int HANDLER_NAME = 1;
		int PARAMS = 2;
		
		String[] completeParse = split(message, ';');
		if (completeParse == null || completeParse.length == 0)
		{
			doLogging("doMessageTo", "Failed to parse message");
			return;
		}
		
		String handlerName = completeParse[HANDLER_NAME];
		dictionary dict = trial.getSessionDict(trial.getParent(getSelf()));
		
		if (!completeParse[PARAMS].equals("none"))
		{
			String[] paramsParse = split(completeParse[PARAMS], ',');
			{
				for (int k=0; k<paramsParse.length; k++)
				{
					testAbortScript();
					String[] parse = split(paramsParse[k], ':');
					String paramType = parse[0];
					
					String[] valueSplit = split(parse[1], '=');
					String elementName = valueSplit[0];
					String elementValue = valueSplit[1];
					
					if (paramType.equals("int"))
					{
						dict.put(elementName, utils.stringToInt(elementValue));
					}
					if (paramType.equals("float"))
					{
						dict.put(elementName, utils.stringToFloat(elementValue));
					}
					else
					{
						dict.put(elementName, elementValue);
					}
				}
			}
		}
		
		messageTo(trial.getParent(getSelf()), handlerName, dict, 0.0f, false);
	}
	
	
	public void doPlayMusicInArea(String message) throws InterruptedException
	{
		String[] parse = split(message, ':');
		if (parse == null || parse.length == 0)
		{
			doLogging("doPlayMusicInArea", "message was null or empty");
			return;
		}
		
		obj_id[] players;
		
		if (parse.length < 3)
		{
			players = instance.getPlayersInInstanceArea(getSelf());
		}
		else
		{
			float range = utils.stringToFloat(parse[2]);
			players = getPlayerCreaturesInRange(getSelf(), range);
			
		}
		
		if (players == null || players.length == 0)
		{
			return;
		}
		
		for (int i=0; i<players.length; i++)
		{
			testAbortScript();
			playMusic(players[i], players[i], parse[1], 0, false);
		}
		
		utils.setScriptVar(getSelf(), "instance_persistedMusic", parse[1]);
	}
	
	
	public void doPlayEffectOnPlayers(String message, location spawnLoc) throws InterruptedException
	{
		String[] parse = split(message, ':');
		
		if (parse == null || parse.length == 0)
		{
			doLogging("doPlayEffectOnPlayers", "message was null or empty");
			return;
		}
		
		obj_id[] players;
		
		if (parse.length < 3)
		{
			players = instance.getPlayersInInstanceArea(getSelf());
		}
		else
		{
			float range = utils.stringToFloat(parse[2]);
			players = getPlayerCreaturesInRange(spawnLoc, range);
		}
		
		if (players == null || players.length == 0)
		{
			return;
		}
		
		for (int i = 0, j = players.length; i < j; i++)
		{
			testAbortScript();
			obj_id[] onePlayer = new obj_id[1];
			
			onePlayer[0] = players[i];
			
			playClientEffectObj(onePlayer, parse[1], onePlayer[0], "");
		}
	}
	
	
	public void doPlayEffectInAreaWithLabel(String message, location spawnLoc) throws InterruptedException
	{
		
		String[] parse = split(message, ':');
		
		if (parse == null || parse.length == 0)
		{
			doLogging("doPlayEffectInAreaWithLabel", "message was null or empty");
			return;
		}
		
		if (parse.length < 3)
		{
			return;
		}
		
		String effectName = parse[1];
		String label = parse[2];
		float range = -1;
		
		if (parse.length >= 4)
		{
			range = utils.stringToFloat(parse[3]);
		}
		
		if (label == null || label.length() < 1)
		{
			return;
		}
		
		obj_id[] players;
		obj_id self = getSelf();
		
		if (range > -1 && spawnLoc != null)
		{
			players = getPlayerCreaturesInRange(spawnLoc, range);
		}
		else
		{
			players = instance.getPlayersInInstanceArea(self);
		}
		
		if (players == null || players.length == 0)
		{
			return;
		}
		
		for (int i=0; i<players.length; i++)
		{
			testAbortScript();
			obj_id player = players[i];
			if (isIdValid(player))
			{
				stopClientEffectObjByLabel(player, player, label);
				
				if (effectName != null && effectName.length() > 0 && !effectName.equals("none"))
				{
					playClientEffectObj(player, effectName, player, "", null, label);
				}
			}
		}
		
		if (effectName != null && effectName.length() > 0 && range < 0)
		{
			utils.setScriptVar(self, "instance_persistedLabeledEffect", effectName+":"+label);
		}
	}
	
	
	public void storeTriggerEvents(obj_id object, String event) throws InterruptedException
	{
		if (event.startsWith("OnDeath"))
		{
			storeDeathTrigger(object, event);
		}
		
		if (event.startsWith("OnEnterCombat"))
		{
			storeEnterCombatTrigger(object, event);
		}
		
		if (event.startsWith("OnExitCombat"))
		{
			storeExitCombatTrigger(object, event);
		}
		
		if (event.startsWith("OnArrivedAtLocation"))
		{
			storeArrivedLocationTrigger(object, event);
		}
		
		if (event.startsWith("customSignal"))
		{
			storeCustomSignal(object, event);
		}
	}
	
	
	public void storeDeathTrigger(obj_id object, String event) throws InterruptedException
	{
		String[] parse = split(event, ':');
		Vector OnDeathTrig = new Vector();
		OnDeathTrig.setSize(0);
		if (utils.hasScriptVar(object, restuss_event.TRIG_ONDEATH))
		{
			OnDeathTrig = utils.getResizeableStringArrayScriptVar(object, restuss_event.TRIG_ONDEATH);
			utils.removeScriptVar(object, restuss_event.TRIG_ONDEATH);
		}
		
		utils.addElement(OnDeathTrig, event.substring(8, event.length()));
		
		utils.setScriptVar(object, restuss_event.TRIG_ONDEATH, OnDeathTrig);
		
	}
	
	
	public void storeEnterCombatTrigger(obj_id object, String event) throws InterruptedException
	{
		String[] parse = split(event, ':');
		
		Vector OnEnterCombatTrig = new Vector();
		OnEnterCombatTrig.setSize(0);
		if (utils.hasScriptVar(object, restuss_event.TRIG_ENTERCOMBAT))
		{
			OnEnterCombatTrig = utils.getResizeableStringArrayScriptVar(object, restuss_event.TRIG_ENTERCOMBAT);
			utils.removeScriptVar(object, restuss_event.TRIG_ENTERCOMBAT);
		}
		
		utils.addElement(OnEnterCombatTrig, event.substring(14, event.length()));
		
		utils.setScriptVar(object, restuss_event.TRIG_ENTERCOMBAT, OnEnterCombatTrig);
	}
	
	
	public void storeExitCombatTrigger(obj_id object, String event) throws InterruptedException
	{
		String[] parse = split(event, ':');
		
		Vector OnExitCombatTrig = new Vector();
		OnExitCombatTrig.setSize(0);
		if (utils.hasScriptVar(object, restuss_event.TRIG_EXITCOMBAT))
		{
			OnExitCombatTrig = utils.getResizeableStringArrayScriptVar(object, restuss_event.TRIG_EXITCOMBAT);
			utils.removeScriptVar(object, restuss_event.TRIG_EXITCOMBAT);
		}
		
		utils.addElement(OnExitCombatTrig, event.substring(13, event.length()));
		
		utils.setScriptVar(object, restuss_event.TRIG_EXITCOMBAT, OnExitCombatTrig);
	}
	
	
	public void storeArrivedLocationTrigger(obj_id object, String event) throws InterruptedException
	{
		if (!isIdValid(object) || event.equals(""))
		{
			doLogging("storeArrivedLocationTrigger", "object was null or event was blank");
			return;
		}
		String[] parse = split(event, ':');
		String pointName = parse[1];
		
		String ppEntry = "pathPoint:"+pointName;
		
		int row = dataTableSearchColumnForString(ppEntry, "object", getStringObjVar(getSelf(), "instance.data_table"));
		dictionary dict = dataTableGetRow(getStringObjVar(getSelf(), "instance.data_table"), row);
		dict.put("wp_name", pointName);
		String room = dict.getString("room");
		float locX = dict.getFloat("loc_x");
		float locY = dict.getFloat("loc_y");
		float locZ = dict.getFloat("loc_z");
		location here = getLocation(getSelf());
		boolean indoor = false;
		location spawnLoc = null;
		if (room.equals("none"))
		{
			locX = here.x + locX;
			locY = here.y + locY;
			locZ = here.z + locZ;
			
			spawnLoc = new location(locX, locY, locZ, here.area);
		}
		else
		{
			indoor = true;
			spawnLoc = new location(locX, locY, locZ, here.area, getCellId(getSelf(), room));
		}
		
		dict.put("loc", spawnLoc);
		
		messageTo(object, "setLocationTarget", dict, 0.0f, false);
		String constructedTrigger = "";
		for (int i=2; i<parse.length; i++)
		{
			testAbortScript();
			constructedTrigger += ":"+parse[i];
		}
		
		Vector OnArriveLocTrig = new Vector();
		OnArriveLocTrig.setSize(0);
		if (utils.hasScriptVar(object, restuss_event.TRIG_ARRIVELOCATION))
		{
			OnArriveLocTrig = utils.getResizeableStringArrayScriptVar(object, restuss_event.TRIG_ARRIVELOCATION);
			utils.removeScriptVar(object, restuss_event.TRIG_ARRIVELOCATION);
		}
		
		utils.addElement(OnArriveLocTrig, constructedTrigger.substring(1, constructedTrigger.length())+":"+pointName);
		
		utils.setScriptVar(object, restuss_event.TRIG_ARRIVELOCATION, OnArriveLocTrig);
	}
	
	
	public void storeCustomSignal(obj_id object, String event) throws InterruptedException
	{
		String[] parse = split(event, ':');
		
		Vector customSignal = new Vector();
		customSignal.setSize(0);
		if (utils.hasScriptVar(object, restuss_event.TRIG_CUSTOMSIGNAL))
		{
			customSignal = utils.getResizeableStringArrayScriptVar(object, restuss_event.TRIG_CUSTOMSIGNAL);
			utils.removeScriptVar(object, restuss_event.TRIG_CUSTOMSIGNAL);
		}
		
		utils.addElement(customSignal, event.substring(13, event.length()));
		
		utils.setScriptVar(object, restuss_event.TRIG_CUSTOMSIGNAL, customSignal);
	}
	
	
	public void doDeleteSpawn(String deleteString) throws InterruptedException
	{
		String[] parse = split(deleteString, ':');
		
		if (parse == null || parse.length < 2)
		{
			doLogging("doDeleteSpawn", "Not enough arguments for deleteString: "+ deleteString);
			return;
		}
		
		String spawnId = parse[1];
		String effect = "none";
		
		if (parse.length > 2)
		{
			effect = parse[2];
		}
		
		obj_id[] spawn_id = getObjectsInSpawnedListWithObjVar(getSelf(), "spawn_id");
		
		if (spawn_id == null || spawn_id.length <= 0)
		{
			return;
		}
		
		for (int i = 0; i < spawn_id.length; i++)
		{
			testAbortScript();
			String checkSpawn = getStringObjVar(spawn_id[i], "spawn_id");
			
			if (checkSpawn.equals(spawnId))
			{
				if (!effect.equals("none"))
				{
					
					dictionary dict = trial.getSessionDict(getSelf());
					dict.put("to_delete", spawn_id[i]);
					
					location object_loc = getLocation(spawn_id[i]);
					obj_id newFxController = obj_id.NULL_ID;
					
					if (isIdValid(object_loc.cell))
					{
						newFxController = createObjectInCell("object/tangible/theme_park/invisible_object.iff", getSelf(), getCellName(object_loc.cell), object_loc);
					}
					else
					{
						newFxController = createObject("object/tangible/theme_park/invisible_object.iff", object_loc);
					}
					
					setYaw(newFxController, getYaw(spawn_id[i]));
					setClientEffectData(newFxController, "clientfx:"+effect+":forced_visability-150");
					attachScript(newFxController, "theme_park.restuss_event.restuss_clientfx_controller");
					messageTo(getSelf(), "handleDelayedDeleteSpawn", dict, 0.75f, false);
				}
				else
				{
					trial.cleanupObject(spawn_id[i]);
				}
			}
		}
	}
	
	
	public int handleDelayedDeleteSpawn(obj_id self, dictionary params) throws InterruptedException
	{
		if (!trial.verifySession(self, params))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id to_delete = params.getObjId("to_delete");
		trial.cleanupObject(to_delete);
		return SCRIPT_CONTINUE;
	}
	
	
	public void storeSpawnedChild(obj_id self, obj_id child, String spawn_id) throws InterruptedException
	{
		dictionary childArray = utils.getDictionaryScriptVar(self, trial.PROT_CHILD_ARRAY);
		
		Vector thisList = new Vector();
		thisList.setSize(0);
		Vector all_spawn_id = new Vector();
		all_spawn_id.setSize(0);
		
		if (childArray.containsKey(spawn_id))
		{
			thisList = childArray.getResizeableObjIdArray(spawn_id);
		}
		
		if (childArray.containsKey(trial.PROT_CHILD_KEY_LIST))
		{
			all_spawn_id = childArray.getResizeableStringArray(trial.PROT_CHILD_KEY_LIST);
		}
		
		thisList.add(child);
		
		if (!all_spawn_id.contains(spawn_id))
		{
			all_spawn_id.add(spawn_id);
		}
		
		childArray.put(spawn_id, thisList);
		childArray.put(trial.PROT_CHILD_KEY_LIST, all_spawn_id);
		
		utils.setScriptVar(self, trial.PROT_CHILD_ARRAY, childArray);
	}
	
	
	public void sendDelayedSpawnActors(String triggerId) throws InterruptedException
	{
		Vector sentIds = new Vector();
		sentIds.setSize(0);
		if (utils.hasScriptVar(getSelf(), ABSOLUTE_TIMER))
		{
			sentIds = utils.getResizeableStringArrayScriptVar(getSelf(), ABSOLUTE_TIMER);
		}
		
		if (sentIds.indexOf(triggerId) > -1)
		{
			return;
		}
		
		sentIds.add(triggerId);
		
		utils.setScriptVar(getSelf(), ABSOLUTE_TIMER, sentIds);
		
		int delay = utils.stringToInt(triggerId);
		dictionary dict = trial.getSessionDict(getSelf());
		dict.put("triggerId", triggerId);
		messageTo(getSelf(), "doDelayedSpawnActors", dict, delay, false);
	}
	
	
	public int doDelayedSpawnActors(obj_id self, dictionary params) throws InterruptedException
	{
		if (!trial.verifySession(self, params))
		{
			return SCRIPT_CONTINUE;
		}
		
		String triggerId = params.getString("triggerId");
		
		spawnActors(self, triggerId);
		return SCRIPT_CONTINUE;
	}
	
	
	public void setUpWaitTree(String object) throws InterruptedException
	{
		obj_id self = getSelf();
		String[] parse = split(object, ':');
		String action = parse[1];
		String signalName = parse[2];
		
		String[] tasks = split(parse[3], ',');
		
		if (tasks == null || tasks.length == 0)
		{
			return;
		}
		
		Vector masterList = new Vector();
		masterList.setSize(0);
		
		if (utils.hasScriptVar(self, WFC_MASTER))
		{
			masterList = utils.getResizeableStringArrayScriptVar(self, WFC_MASTER);
			utils.removeScriptVar(self, WFC_MASTER);
		}
		
		if (!masterList.contains(object.substring(16, object.length())))
		{
			utils.addElement(masterList, object.substring(16, object.length()));
		}
		
		utils.setScriptVar(self, WFC_MASTER, masterList);
		
		for (int i=0; i<tasks.length; i++)
		{
			testAbortScript();
			utils.setScriptVar(self, WFC_TREE+"."+signalName+"."+i+tasks[i], 0);
		}
	}
	
	
	public void sendTaskCompleteSignal(String object) throws InterruptedException
	{
		
		obj_id self = getSelf();
		String[] parse = split(object, ':');
		
		if (parse.length < 2)
		{
			return;
		}
		
		dictionary dict = trial.getSessionDict(self, "wfc_signal");
		dict.put("waitForComplete", parse[1]);
		messageTo(self, "waitForComplete", dict, 0.0f, false);
	}
	
	
	public int waitForComplete(obj_id self, dictionary params) throws InterruptedException
	{
		String signal = params.getString("waitForComplete");
		if (!utils.hasScriptVar(self, WFC_MASTER))
		{
			return SCRIPT_CONTINUE;
		}
		
		Vector masterList = utils.getResizeableStringArrayScriptVar(self, WFC_MASTER);
		
		for (int i=0; i<masterList.size(); i++)
		{
			testAbortScript();
			handleCheckForComplete(((String)(masterList.get(i))), signal);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void completeTaskId(String signal) throws InterruptedException
	{
		Vector masterList = utils.getResizeableStringArrayScriptVar(getSelf(), WFC_MASTER);
		
		for (int i=0; i<masterList.size(); i++)
		{
			testAbortScript();
			handleCheckForComplete(((String)(masterList.get(i))), signal);
		}
		
	}
	
	
	public void handleCheckForComplete(String masterList, String signal) throws InterruptedException
	{
		String[] parse = split(masterList, ':');
		String triggerType = parse[0];
		String triggerName = parse[1];
		
		String[] tasks = split(parse[2], ',');
		
		int isComplete = 0;
		for (int i=0; i<tasks.length; i++)
		{
			testAbortScript();
			if (tasks[i].equals(signal))
			{
				if (1 == utils.getIntScriptVar(getSelf(), WFC_TREE+"."+triggerName+"."+i+tasks[i]))
				{
					isComplete++;
					continue;
				}
				else
				{
					utils.setScriptVar(getSelf(), WFC_TREE+"."+triggerName+"."+i+tasks[i], 1);
					isComplete++;
					signal = "NEGATIVE_MATCH";
					continue;
				}
			}
			else
			{
				isComplete += utils.getIntScriptVar(getSelf(), WFC_TREE+"."+triggerName+"."+i+tasks[i]);
			}
		}
		
		if (tasks.length == isComplete)
		{
			dictionary dict = trial.getSessionDict(getSelf());
			dict.put("triggerType", triggerType);
			dict.put("triggerName", triggerName);
			messageTo(getSelf(), "triggerFired", dict, 0.0f, false);
			Vector vectorList = utils.getResizeableStringArrayScriptVar(getSelf(), WFC_MASTER);
			vectorList.remove(masterList);
			if (vectorList == null || vectorList.size() == 0)
			{
				utils.removeScriptVar(getSelf(), WFC_MASTER);
			}
			else
			{
				utils.setScriptVar(getSelf(), WFC_MASTER, vectorList);
			}
		}
		
		return;
	}
	
	
	public obj_id establishPatrolPoint(String ppData, location spawnLoc, boolean isIndoor) throws InterruptedException
	{
		String[] parse = split(ppData, ':');
		String ppName = parse[1];
		
		obj_id patrolPoint = create.object("object/tangible/ground_spawning/patrol_waypoint.iff", spawnLoc);
		
		setName(patrolPoint, ppName);
		setObjVar(patrolPoint, "sequence_controller.patrolPointName", ppName);
		trial.markAsTempObject(patrolPoint, true);
		trial.setParent(getSelf(), patrolPoint, true);
		
		return patrolPoint;
	}
	
	
	public obj_id[] getObjectsInSpawnedListWithObjVar(obj_id self, String objvar) throws InterruptedException
	{
		obj_id[] children = getSpawnedChildren(self);
		return trial.getObjectsInListWithObjVar(children, objvar);
	}
	
	
	public void queueDelayAction(String object) throws InterruptedException
	{
		String[] parse = split(object, ':');
		String triggerId = parse[1];
		float delay = utils.stringToFloat(parse[2]);
		
		dictionary dict = trial.getSessionDict(getSelf());
		dict.put("triggerId", triggerId);
		
		messageTo(getSelf(), "doDelayedSpawnActors", dict, delay, false);
	}
	
	
	public void setWayPointLinks(obj_id self) throws InterruptedException
	{
		
		if (true)
		{
			return;
		}
		
		obj_id[] pp = getObjectsInSpawnedListWithObjVar(self, "sequence_controller.patrolPointName");
		
		if (pp == null || pp.length == 0)
		{
			return;
		}
		
		for (int i=0; i<pp.length; i++)
		{
			testAbortScript();
			if (!isIdValid(pp[i]) || !exists(pp[i]))
			{
				continue;
			}
			
			establishConnectionData(pp[i], pp);
		}
	}
	
	
	public void establishConnectionData(obj_id point, obj_id[] ppl) throws InterruptedException
	{
		Vector ppData = new Vector();
		ppData.setSize(0);
		
		for (int i=0; i<ppl.length; i++)
		{
			testAbortScript();
			if (!isIdValid(ppl[i]) || !exists(ppl[i]))
			{
				continue;
			}
			
			if (!canSee(point, ppl[i]))
			{
				continue;
			}
			
			if (point == ppl[i])
			{
				continue;
			}
			
			ppData.add(""+ppl[i]+"-"+getDistance(point, ppl[i]));
		}
		
		utils.setScriptVar(point, trial.WP_DATA, ppData);
	}
	
	
	public int terminationCallback(obj_id self, dictionary params) throws InterruptedException
	{
		if (!trial.verifySession(self, params))
		{
			return SCRIPT_CONTINUE;
		}
		
		String spawn_id = params.getString("spawn_id");
		obj_id object = params.getObjId("object");
		int row = params.getInt("row");
		row = row - 1;
		int respawn = params.getInt("respawn");
		
		dictionary dict = trial.getSessionDict(self);
		
		if (!spawn_id.equals("none"))
		{
			dict.put("waitForComplete", spawn_id);
			messageTo(self, "waitForComplete", dict, 0.0f, false);
			
		}
		
		if (respawn > -1)
		{
			dict.put("row", row);
			messageTo(self, "handleDelayedSpawnRow", dict, respawn, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDelayedSpawnRow(obj_id self, dictionary params) throws InterruptedException
	{
		if (!trial.verifySession(self, params))
		{
			return SCRIPT_CONTINUE;
		}
		
		int row = params.getInt("row");
		
		spawnActorRow(row);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public obj_id doRandomSpawn(String[] list, location spawnLoc) throws InterruptedException
	{
		Vector spawnList = new Vector();
		spawnList.setSize(0);
		
		for (int i=0; i<list.length; i++)
		{
			testAbortScript();
			String[] subSplit = split(list[i], ':');
			if (subSplit.length == 1)
			{
				spawnList.add(list[i]);
			}
			else
			{
				int numberAdd = utils.stringToInt(subSplit[1]);
				
				for (int q=0; q<numberAdd; q++)
				{
					testAbortScript();
					spawnList.add(subSplit[0]);
				}
			}
		}
		
		if (spawnList == null || spawnList.size() == 0)
		{
			return null;
		}
		
		String toSpawn = ((String)(spawnList.get(rand(0, spawnList.size() -1))));
		
		return create.object(toSpawn, spawnLoc);
	}
	
	
	public void executeRandomTrigger(String object) throws InterruptedException
	{
		String[] parse = split(object, ':');
		String[] subSplit = split(parse[1], ',');
		
		String toExecute = subSplit[rand(0, subSplit.length -1)];
		
		dictionary dict = trial.getSessionDict(getSelf());
		dict.put("triggerType", "triggerId");
		dict.put("triggerName", toExecute);
		messageTo(getSelf(), "triggerFired", dict, 0.0f, false);
	}
	
	
	public void sendInstanceMessage(String object) throws InterruptedException
	{
		String[] parse = split(object, ':');
		
		String sid = parse[1];
		String templateOverride = parse[2];
		string_id message = new string_id("sequencer_spam", sid);
		float duration = 0.0f;
		
		if (parse.length > 3)
		{
			duration = utils.stringToFloat(parse[3]);
		}
		
		obj_id[] players = instance.getPlayersInInstanceArea(getSelf());
		
		if (players == null || players.length == 0)
		{
			return;
		}
		
		if (!templateOverride.equals("none"))
		{
			if (duration > 0.0f)
			{
				utils.messagePlayer(getSelf(), players, message, templateOverride, duration);
			}
			else
			{
				utils.messagePlayer(getSelf(), players, message, templateOverride);
			}
		}
		else
		{
			utils.sendSystemMessage(players, message);
		}
	}
	
	
	public void performTheaterAction(obj_id self, String object) throws InterruptedException
	{
		String[] parse = split(object, ':');
		String actor = parse[1];
		String action = parse[2];
		
		obj_id[] allIds = getObjectsInSpawnedListWithObjVar(self, "spawn_id");
		
		if (allIds == null || allIds.length <= 0)
		{
			return;
		}
		
		obj_id objActor = null;
		
		for (int i=0; i<allIds.length; i++)
		{
			testAbortScript();
			if (getStringObjVar(allIds[i], "spawn_id").equals(actor))
			{
				objActor = allIds[i];
			}
		}
		
		if (!isIdValid(objActor))
		{
			return;
		}
		
		if (action.equals("faceTo"))
		{
			performFacingAction(self, objActor, object);
		}
		
		if (action.equals("chat"))
		{
			performChatAction(self, objActor, object);
		}
		
		if (action.equals("animate"))
		{
			performAnimation(self, objActor, object);
		}
		
		if (action.equals("walk"))
		{
			movement.performWalk(objActor);
		}
		
		if (action.equals("run") && parse.length > 3)
		{
			movement.performRun(objActor, parse[3]);
		}
	}
	
	
	public void performFacingAction(obj_id self, obj_id actor, String object) throws InterruptedException
	{
		String[] parse = split(object, ':');
		
		String target = parse[3];
		String affector = parse[4];
		
		obj_id[] allIds = getObjectsInSpawnedListWithObjVar(self, "spawn_id");
		obj_id objTarget = null;
		
		if (target.equals("spawn_id"))
		{
			for (int i=0; i<allIds.length; i++)
			{
				testAbortScript();
				if (getStringObjVar(allIds[i], "spawn_id").equals(affector))
				{
					objTarget = allIds[i];
				}
			}
			
			if (!isIdValid(objTarget))
			{
				return;
			}
			
			faceTo(actor, objTarget);
		}
		
		if (target.equals("yaw"))
		{
			setYaw(actor, utils.stringToFloat(affector));
		}
		
		if (target.equals("player"))
		{
			obj_id player = getClosestPlayer(getLocation(actor));
			if (isIdValid(player))
			{
				faceTo(actor, getLocation(player));
			}
			
		}
		
	}
	
	
	public void performChatAction(obj_id self, obj_id actor, String object) throws InterruptedException
	{
		String[] parse = split(object, ':');
		
		String chatAction = parse[3];
		string_id stringId = new string_id("sequencer_spam", parse[4]);
		
		chat.chat(actor, chatAction, stringId);
	}
	
	
	public void performAnimation(obj_id self, obj_id actor, String object) throws InterruptedException
	{
		String[] parse = split(object, ':');
		String action = parse[3];
		
		doAnimationAction(actor, action);
	}
	
	
	public void doObjVarAction(obj_id self, String object) throws InterruptedException
	{
		String[] parse = split(object, ':');
		
		String id = parse[1];
		
		String action = parse[2];
		
		String type = parse[3];
		
		String name = parse[4];
		
		obj_id[] spawn_ids = getObjectsInSpawnedListWithObjVar(self, "spawn_id");
		
		for (int i=0; i<spawn_ids.length; i++)
		{
			testAbortScript();
			if (!getStringObjVar(spawn_ids[i], "spawn_id").equals(id))
			{
				continue;
			}
			
			if (action.equals("set"))
			{
				String value = parse[5];
				
				if (type.equals("int"))
				{
					setObjVar(spawn_ids[i], name, utils.stringToInt(value));
				}
				else if (type.equals("float"))
				{
					setObjVar(spawn_ids[i], name, utils.stringToFloat(value));
				}
				else
				{
					setObjVar(spawn_ids[i], name, value);
				}
			}
			
			if (action.equals("remove"))
			{
				if (hasObjVar(spawn_ids[i], name))
				{
					removeObjVar(spawn_ids[i], name);
				}
			}
		}
		
	}
	
	
	public void doScriptAction(obj_id self, String object) throws InterruptedException
	{
		String[] parse = split(object, ':');
		
		String id = parse[1];
		
		String action = parse[2];
		
		String name = parse[3];
		
		obj_id[] spawn_ids = getObjectsInSpawnedListWithObjVar(self, "spawn_id");
		
		for (int i=0; i<spawn_ids.length; i++)
		{
			testAbortScript();
			if (!getStringObjVar(spawn_ids[i], "spawn_id").equals(id))
			{
				continue;
			}
			
			if (action.equals("attach"))
			{
				attachScript(spawn_ids[i], name);
			}
			
			if (action.equals("detach"))
			{
				if (hasScript(spawn_ids[i], name))
				{
					detachScript(spawn_ids[i], name);
				}
			}
		}
		
	}
	
	
	public void setBuffHandler(obj_id self, String object) throws InterruptedException
	{
		String[] parse = split(object, ':');
		String action = parse[1];
		String buffs = parse[2];
		String who = parse[3];
		
		if (action.equals("add"))
		{
			String[] buffList = split(buffs, ',');
			
			if (who.startsWith("player"))
			{
				obj_id[] players = instance.getPlayersInInstanceArea(self);
				
				if (players != null && players.length > 0)
				{
					for (int i=0; i<players.length; i++)
					{
						testAbortScript();
						if (isIdValid(players[i]) && exists(players[i]))
						{
							buff.applyBuff(players[i], buffList);
						}
					}
				}
			}
			
			if (who.startsWith("spawn_id"))
			{
				String[] subWho = split(who, '-');
				String spawn_id = subWho[1];
				
				obj_id[] allIds = trial.getObjectsInInstanceBySpawnId(self, spawn_id);
				
				if (allIds != null && allIds.length > 0)
				{
					for (int q=0; q<allIds.length; q++)
					{
						testAbortScript();
						if (isIdValid(allIds[q]) && exists(allIds[q]))
						{
							buff.applyBuff(allIds[q], buffList);
						}
					}
				}
			}
		}
	}
	
	
	public void setCellPermission(obj_id self, String object) throws InterruptedException
	{
		String[] parse = split(object, ':');
		String cellName = parse[1];
		String status = parse[2];
		
		if (status.equals("public"))
		{
			if (!permissionsIsPublic(getCellId(self, cellName)))
			{
				permissionsMakePublic(getCellId(self, cellName));
			}
		}
		if (status.equals("private"))
		{
			if (permissionsIsPublic(getCellId(self, cellName)))
			{
				permissionsMakePrivate(getCellId(self, cellName));
			}
		}
	}
	
	
	public int sendProxySignal(obj_id self, dictionary params) throws InterruptedException
	{
		if (!trial.verifySession(self, params))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id target = params.getObjId("perp");
		String triggerName = params.getString("triggerName");
		
		int entryRow = dataTableSearchColumnForString(triggerName, "triggerId", getStringObjVar(getSelf(), "instance.data_table"));
		String message = dataTableGetString(getStringObjVar(getSelf(), "instance.data_table"), entryRow, "object");
		
		if (entryRow < 0)
		{
			doLogging("sendProxySignal", "Could not find triggerName: "+triggerName+" in table");
			return SCRIPT_CONTINUE;
		}
		
		if (!message.startsWith("messageTo"))
		{
			doLogging("sendProxySignal", "Tried to send a non messageTo: "+message+" as proxy");
			return SCRIPT_CONTINUE;
		}
		
		doMessgeTo(target, message);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void setFullPathData(obj_id object) throws InterruptedException
	{
		obj_id self = getSelf();
		String[] pathArray = getFullPathData(self);
		
		if (pathArray == null)
		{
			return;
		}
		
		utils.setScriptVar(object, trial.PATROL_PATH_FULL_DATA, pathArray);
	}
	
	
	public String[] getFullPathData(obj_id instance) throws InterruptedException
	{
		obj_id self = getSelf();
		
		if (utils.hasScriptVar(self, trial.PATROL_PATH_FULL_DATA))
		{
			return utils.getStringArrayScriptVar(self, trial.PATROL_PATH_FULL_DATA);
		}
		
		String[] path_column = dataTableGetStringColumnNoDefaults(getStringObjVar(getSelf(), "instance.data_table"), "paths");
		
		if (path_column == null || path_column.length == 0)
		{
			return null;
		}
		
		return path_column;
	}
	
	
	public int[] fillRowArray(int num_rows) throws InterruptedException
	{
		int int_array[] = new int[num_rows];
		
		for (int i=0; i<num_rows; i++)
		{
			testAbortScript();
			int_array[i] = i + 1;
		}
		
		return int_array;
	}
	
	
	public String[] getPathPointLocationArrayKeyList(String[] object, String[] room, float[] loc_x, float[] loc_y, float[] loc_z) throws InterruptedException
	{
		Vector newPathData = new Vector();
		newPathData.setSize(0);
		
		for (int i=0; i<object.length; i++)
		{
			testAbortScript();
			if (object[i].startsWith("pathPoint:"))
			{
				String[] parse = split(object[i], ':');
				String pointName = parse[1];
				
				float locx = loc_x[i];
				float locy = loc_y[i];
				float locz = loc_z[i];
				obj_id cell = room[i].equals("none") ? null : getCellId(getSelf(), room[i]);
				
				String pathLoc;
				location masterLoc = getLocation(getSelf());
				
				if (!isIdValid(cell) && !hasObjVar(getSelf(), "worldLoc"))
				{
					locx = masterLoc.x + locx;
					locy = masterLoc.y + locy;
					locz = masterLoc.z + locz;
					
				}
				
				pathLoc = ""+locx+","+locy+","+locz+","+cell+","+masterLoc.area;
				
				newPathData.add(pointName+"|"+ pathLoc);
			}
		}
		
		return (String[])newPathData.toArray(new String[0]);
	}
	
	
	public int view(obj_id self, dictionary params) throws InterruptedException
	{
		dictionary dict = utils.getDictionaryScriptVar(self, trial.SEQUENCER_PATH_DATA);
		
		String[] pathPointNameArray = dict.getStringArray("pathPointNameArray");
		location[] pathPointLocationArray = dict.getLocationArray("pathPointLocationArray");
		String[] patrolNameMaster = dict.getStringArray("patrolNameMaster");
		String[] patrolTypeMaster = dict.getStringArray("patrolTypeMaster");
		String[] patrolListMaster = dict.getStringArray("patrolListMaster");
		
		for (int i=0; i<patrolListMaster.length; i++)
		{
			testAbortScript();
			doLogging("xx", ""+patrolListMaster[i]);
		}
		doLogging("xx", ""+patrolListMaster.length);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void setFullPathDataDictionary(String[] pp_location, String[] path_data) throws InterruptedException
	{
		dictionary dict = new dictionary();
		dict = getPpLocDict(dict, pp_location);
		
		String[] points = dict.getStringArray("pathPointNameArray");
		location[] locs = dict.getLocationArray("pathPointLocationArray");
		
		dict = fillPathKeyList(dict, path_data, points, locs);
		utils.setScriptVar(getSelf(), trial.SEQUENCER_PATH_DATA, dict);
	}
	
	
	public dictionary getPpLocDict(dictionary dict, String[] pp_location) throws InterruptedException
	{
		String[] thesePoints = new String[pp_location.length];
		String[] theseLocs = new String[pp_location.length];
		
		for (int i=0; i<pp_location.length; i++)
		{
			testAbortScript();
			String[] split_pp = split(pp_location[i], '|');
			
			thesePoints[i] = split_pp[0];
			theseLocs[i] = split_pp[1];
		}
		
		location[] locArray = getLocArrayFromLocString(theseLocs);
		dict.put("pathPointNameArray", thesePoints);
		dict.put("pathPointLocationArray", locArray);
		
		return dict;
	}
	
	
	public location[] getLocArrayFromLocString(String[] locationString) throws InterruptedException
	{
		location[] locArray = new location[locationString.length];
		
		for (int i=0; i<locationString.length; i++)
		{
			testAbortScript();
			String[] split_loc = split(locationString[i], ',');
			float locx = utils.stringToFloat(split_loc[0]);
			float locy = utils.stringToFloat(split_loc[1]);
			float locz = utils.stringToFloat(split_loc[2]);
			obj_id cell = utils.stringToObjId(split_loc[3]);
			String area = split_loc[4];
			
			location newLoc;
			
			if (!isIdValid(cell) || !exists(cell))
			{
				newLoc = new location(locx, locy, locz, area);
			}
			else
			{
				newLoc = new location(locx, locy, locz, area, cell);
			}
			
			locArray[i] = newLoc;
		}
		
		return locArray;
	}
	
	
	public dictionary fillPathKeyList(dictionary dict, String[] path_data, String[] points, location[] locs) throws InterruptedException
	{
		if (path_data == null || path_data.length == 0)
		{
			return dict;
		}
		
		String[] patrolArray = new String[path_data.length];
		String[] typeArray = new String[path_data.length];
		String[] nameArray = new String[path_data.length];
		
		for (int i=0; i<path_data.length; i++)
		{
			testAbortScript();
			String[] thisSplit = split(path_data[i], ':');
			patrolArray[i] = thisSplit[0];
			typeArray[i] = thisSplit[1];
			nameArray[i] = thisSplit[2];
		}
		
		dict.put("patrolNameMaster", patrolArray);
		dict.put("patrolTypeMaster", typeArray);
		dict.put("patrolListMaster", nameArray);
		
		return dict;
	}
	
	
	public void handleSetCellLabel(obj_id self, String object) throws InterruptedException
	{
		String[] parse = split(object, ':');
		
		String cellName = parse[1];
		String cellLabel = parse[2].equals("none") ? "": parse[2];
		float flox = 0.0f;
		float floy = 0.0f;
		float floz = 0.0f;
		obj_id cellId = getCellId(self, cellName);
		setCellLabel(cellId, cellLabel);
		
		if (parse.length == 4)
		{
			String[] offset = split(parse[3], ',');
			flox = utils.stringToFloat(offset[0]);
			floy = utils.stringToFloat(offset[1]);
			floz = utils.stringToFloat(offset[2]);
			setCellLabelOffset(cellId, flox, floy, floz);
		}
	}
	
	
	public int registerObjectWithSequencer(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id object = params.getObjId("object");
		String spawn_id = hasObjVar(object, "spawn_id") ? getStringObjVar(object, "spawn_id") : "none";
		storeSpawnedChild(self, object, spawn_id);
		return SCRIPT_CONTINUE;
	}
	
	
	public int unregisterObjectWithSequencer(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("remove_from_sequencer","unregisterObjectWithSequencer init");
		
		obj_id object = params.getObjId("object");
		String spawn_id = params.getString("spawn_id");
		
		LOG("remove_from_sequencer","unregisterObjectWithSequencer object: "+object);
		LOG("remove_from_sequencer","unregisterObjectWithSequencer spawn_id: "+spawn_id);
		
		removeSpawnChild(object, spawn_id);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void setPlayerListOnObject(obj_id sequencer, obj_id object) throws InterruptedException
	{
		obj_id[] playerList = instance.getPlayerIdList(sequencer);
		
		if (playerList == null || playerList.length == 0)
		{
			return;
		}
		
		utils.setScriptVar(object, instance.PLAYER_ID_LIST, playerList);
	}
	
	public void doLogging(String section, String message) throws InterruptedException
	{
		if (LOGGING)
		{
			LOG("doLogging/sequence_controller/"+section, message);
		}
	}
	
}
