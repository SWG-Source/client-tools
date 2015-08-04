package script.systems.dungeon_sequencer;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.ai_lib;
import script.library.beast_lib;
import script.library.buff;
import script.library.chat;
import script.library.factions;
import script.library.utils;
import script.library.trial;
import script.library.create;
import script.library.restuss_event;


public class ai_controller extends script.base_script
{
	public ai_controller()
	{
	}
	public static final String PATH = "patrol_path";
	public static final String MOVETO = "moveTo";
	
	public static final boolean LOGGING = true;
	
	
	public int OnHearSpeech(obj_id self, obj_id objSpeaker, String strText) throws InterruptedException
	{
		if (!isGod(objSpeaker))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (strText.equals("gcwpath"))
		{
			if (hasObjVar(self, "patrol_path"))
			{
				dictionary dict = new dictionary();
				dict.put("pathAction", PATH);
				dict.put("pathName", getStringObjVar(self, "patrol_path"));
				messageTo(self, "doPathAction", dict, 1.0f, false);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		if (!isIdValid(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, "ai_controller.behavior"))
		{
			
			ai_lib.setDefaultCalmBehavior(self, getIntObjVar(self, "ai_controller.behavior"));
		}
		else
		{
			ai_lib.setDefaultCalmBehavior(self, ai_lib.BEHAVIOR_SENTINEL);
		}
		
		trial.setInterest(self);
		
		if (hasObjVar(self, "patrol_path"))
		{
			dictionary dict = new dictionary();
			dict.put("pathAction", PATH);
			dict.put("pathName", getStringObjVar(self, "patrol_path"));
			messageTo(self, "doPathAction", dict, 1.0f, false);
		}
		
		if (hasObjVar(self, "moveTo"))
		{
			dictionary dict = new dictionary();
			dict.put("pathAction", MOVETO);
			dict.put("pathName", getStringObjVar(self, "moveTo"));
			messageTo(self, "doPathAction", dict, 1.0f, false);
		}
		
		if (hasObjVar(self, "posture"))
		{
			String posture = getStringObjVar(self, "posture");
			
			if (posture.equals("prone"))
			{
				setPosture(self, POSTURE_PRONE);
			}
			
			if (posture.equals("kneeling"))
			{
				setPosture(self, POSTURE_CROUCHED);
			}
		}
		
		if (hasObjVar(self, "name"))
		{
			String name = getStringObjVar(self, "name");
			
			setName(self, name);
		}
		
		if (hasObjVar(self, "passive"))
		{
			removeTriggerVolume(ai_lib.ALERT_VOLUME_NAME);
			removeTriggerVolume(ai_lib.AGGRO_VOLUME_NAME);
			
		}
		
		if (hasObjVar(self, "equip"))
		{
			aiEquipPrimaryWeapon(self);
		}
		
		if (hasObjVar(self, "faction"))
		{
			String factionName = getStringObjVar(self, "faction");
			
			if (factionName != null && factionName.length() > 0 && (factionName.equals("Imperial") || factionName.equals("Rebel")))
			{
				factions.setFaction(self, factionName, true, 0);
			}
		}
		
		if (hasObjVar(self, "buff"))
		{
			String buffName = getStringObjVar(self, "buff");
			if (buffName != null && buffName.length() > 0)
			{
				buff.applyBuff(self, self, buffName);
			}
		}
		
		if (hasObjVar(self, "hp_value"))
		{
			trial.setHp(self, getIntObjVar(self, "hp_value"));
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnRemovingFromWorld(obj_id self) throws InterruptedException
	{
		LOG("remove_from_sequencer","OnRemovingFromWorld init");
		dictionary dict = new dictionary();
		dict.put("object", self);
		String spawn_id = hasObjVar(self, "spawn_id") ? getStringObjVar(self, "spawn_id") : "none";
		dict.put("spawn_id", spawn_id);
		LOG("remove_from_sequencer","OnRemovingFromWorld spawn_id of obj: "+spawn_id);
		
		messageTo(trial.getParent(self), "unregisterObjectWithSequencer", dict, 0.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAiTetherStart(obj_id self) throws InterruptedException
	{
		messageTo(self, "tether_delay", null, 3.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int tether_delay(obj_id self, dictionary params) throws InterruptedException
	{
		if (aiIsTethered(self))
		{
			messageTo(self, "resumeDefaultCalmBehavior", null, 3.0f, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void resetSequenceMovement() throws InterruptedException
	{
		obj_id self = getSelf();
		
		if (!isIdValid(self) || !exists(self))
		{
			return;
		}
		
		if (ai_lib.isInCombat(self))
		{
			setMovementPercent(self, 1.0f);
		}
		
		if (hasObjVar(self, "run"))
		{
			int testInt = getIntObjVar(self, "run");
			float testFloat = getFloatObjVar(self, "run");
			
			if (testInt != 0)
			{
				float newMove = (1.0f + (float)testInt);
				
				setMovementPercent(self, newMove);
			}
			
			if (testFloat != 0)
			{
				float newMove = 1.0f + testFloat;
				
				setMovementPercent(self, newMove);
			}
		}
	}
	
	
	public int OnMoveMoving(obj_id self) throws InterruptedException
	{
		resetSequenceMovement();
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int resumeDefaultCalmBehavior(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isIdValid(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, "patrol_path"))
		{
			dictionary dict = new dictionary();
			dict.put("pathAction", PATH);
			dict.put("pathName", getStringObjVar(self, "patrol_path"));
			messageTo(self, "doPathAction", dict, 1.0f, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int doPathAction(obj_id self, dictionary params) throws InterruptedException
	{
		String pathAction = params.getString("pathAction");
		String pathName = params.getString("pathName");
		
		if (pathAction.equals(PATH))
		{
			beginPathRoutine(self, pathName);
		}
		
		if (pathAction.equals(MOVETO))
		{
			pathToPoint(self, pathName);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int destroySelf(obj_id self, dictionary params) throws InterruptedException
	{
		trial.cleanupObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int wsDoAnimation(obj_id self, dictionary params) throws InterruptedException
	{
		String anim = params.getString("animation");
		doAnimationAction(self, anim);
		return SCRIPT_CONTINUE;
	}
	
	
	public int wsPlayEmote(obj_id self, dictionary params) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int wsSignalMaster(obj_id self, dictionary params) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int wsDespawn(obj_id self, dictionary params) throws InterruptedException
	{
		messageTo(self, "destroySelf", null, 5.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int wsToggleInvulnerability(obj_id self, dictionary params) throws InterruptedException
	{
		
		boolean toggleVulnerable = !isInvulnerable(self);
		
		if (toggleVulnerable)
		{
			clearHateList(self);
			stopCombat(self);
		}
		
		setInvulnerable(self, toggleVulnerable);
		return SCRIPT_CONTINUE;
	}
	
	
	public int customSignal(obj_id self, dictionary params) throws InterruptedException
	{
		String signalName = params.getString("triggerName");
		
		if (signalName == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.hasScriptVar(self, restuss_event.TRIG_CUSTOMSIGNAL))
		{
			Vector customTrigger = utils.getResizeableStringArrayScriptVar(self, restuss_event.TRIG_CUSTOMSIGNAL);
			
			if (customTrigger != null && customTrigger.size() > 0)
			{
				for (int i=0; i<customTrigger.size(); i++)
				{
					testAbortScript();
					if (((String)(customTrigger.get(i))).startsWith(signalName))
					{
						executeTriggerData(self, ((String)(customTrigger.get(i))).substring(signalName.length() + 1, ((String)(customTrigger.get(i))).length()));
					}
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDeath(obj_id self, obj_id killer, obj_id corpseId) throws InterruptedException
	{
		handleDeathTrigger(self, killer);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectDisabled(obj_id self, obj_id killer) throws InterruptedException
	{
		handleDeathTrigger(self, killer);
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleDeathTrigger(obj_id self, obj_id killer) throws InterruptedException
	{
		if (utils.hasScriptVar(self, restuss_event.TRIG_ONDEATH))
		{
			Vector deathTriggers = utils.getResizeableStringArrayScriptVar(self, restuss_event.TRIG_ONDEATH);
			
			if (deathTriggers != null && deathTriggers.size() > 0)
			{
				for (int i=0; i<deathTriggers.size(); i++)
				{
					testAbortScript();
					executeTriggerData(self, ((String)(deathTriggers.get(i))).concat(":"+killer));
				}
			}
		}
		
		reportDeath(self, killer);
		
		if (hasObjVar(self, "explosion_cleanup_on_death"))
		{
			location death = getLocation( self );
			playClientEffectObj(killer, "clienteffect/combat_explosion_lair_large.cef", self, "");
			playClientEffectLoc(killer, "clienteffect/combat_explosion_lair_large.cef", death, 0);
			
			setInvulnerable( self, true );
			messageTo( self, "handleDelayedCleanup", null, 1f, false );
		}
		
		return;
	}
	
	
	public int OnEnteredCombat(obj_id self) throws InterruptedException
	{
		resetSequenceMovement();
		
		if (utils.hasScriptVar(self, restuss_event.TRIG_ENTERCOMBAT))
		{
			Vector combatTriggers = utils.getResizeableStringArrayScriptVar(self, restuss_event.TRIG_ENTERCOMBAT);
			
			if (combatTriggers != null && combatTriggers.size() > 0)
			{
				for (int i=0; i<combatTriggers.size(); i++)
				{
					testAbortScript();
					executeTriggerData(self, ((String)(combatTriggers.get(i))));
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnExitedCombat(obj_id self) throws InterruptedException
	{
		if (ai_lib.isDead(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		resetSequenceMovement();
		
		if (utils.hasScriptVar(self, restuss_event.TRIG_EXITCOMBAT))
		{
			Vector combatTriggers = utils.getResizeableStringArrayScriptVar(self, restuss_event.TRIG_EXITCOMBAT);
			
			if (combatTriggers != null && combatTriggers.size() > 0)
			{
				for (int i=0; i<combatTriggers.size(); i++)
				{
					testAbortScript();
					executeTriggerData(self, ((String)(combatTriggers.get(i))));
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnArrivedAtLocation(obj_id self, String location) throws InterruptedException
	{
		
		boolean executedAction = false;
		
		if (location.equals("remove_path"))
		{
			removeObjVar(self, "patrol_path");
		}
		
		if (utils.hasScriptVar(self, restuss_event.TRIG_ARRIVELOCATION))
		{
			Vector arriveTrigger = utils.getResizeableStringArrayScriptVar(self, restuss_event.TRIG_ARRIVELOCATION);
			
			if (arriveTrigger != null && arriveTrigger.size() > 0)
			{
				for (int i=0; i<arriveTrigger.size(); i++)
				{
					testAbortScript();
					if (((String)(arriveTrigger.get(i))).endsWith(location))
					{
						dictionary dict = new dictionary();
						dict.put("triggerData", ((String)(arriveTrigger.get(i))));
						dict.put("suspend", true);
						messageTo(self, "handleTriggerData", dict, 1.0f, false);
						executedAction = true;
					}
				}
			}
		}
		
		if (hasObjVar(self, "delete_on_arrive") && executedAction)
		{
			messageTo(self, "handleDelayedCleanup", null, 3.0f, false);
		}
		
		if (hasObjVar(self, "gcw.entertainer_rally"))
		{
			messageTo(self, "handleSetDefaultBehavior", null, 1, false);
			loiterLocation(self, getLocation(self), 1f, 5f, 2f, 8f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDelayedCleanup(obj_id self, dictionary params) throws InterruptedException
	{
		trial.cleanupObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public void executeTriggerData(obj_id self, String triggerData) throws InterruptedException
	{
		executeTriggerData(self, triggerData, false);
	}
	
	
	public int handleTriggerData(obj_id self, dictionary params) throws InterruptedException
	{
		String triggerData = params.getString("triggerData");
		boolean suspend = params.getBoolean("suspend");
		
		executeTriggerData(self, triggerData, suspend);
		return SCRIPT_CONTINUE;
	}
	
	
	public void executeTriggerData(obj_id self, String triggerData, boolean suspend) throws InterruptedException
	{
		String[] parse = split(triggerData, ':');
		
		if (false)
		{
			suspendMovement(self);
			messageTo(self, "resumeMovement", null, 5.0f, false);
		}
		
		if (triggerData.startsWith("playEmote"))
		{
			executeEmoteTrigger(self, parse);
		}
		
		if (triggerData.startsWith("signalMaster"))
		{
			executeSignalMasterTrigger(self, parse);
		}
		
		if (triggerData.startsWith("signalKiller"))
		{
			
		}
		
		if (triggerData.startsWith("doAnimationAction"))
		{
			executeAnimationTrigger(self, parse);
		}
		
		if (triggerData.startsWith("toggleInvulnerable"))
		{
			messageTo(self, "wsToggleInvulnerability", null, 0.0f, false);
		}
		
		if (triggerData.startsWith("broadcastMessage"))
		{
			
		}
		
		if (triggerData.startsWith("faceTo"))
		{
			executeFacingTrigger(self, parse);
		}
		
		if (triggerData.startsWith("messageSelf"))
		{
			executeMessageSelfTrigger(self, parse);
		}
		
		if (triggerData.startsWith("triggerId"))
		{
			executeIdTrigger(self, parse);
		}
		
		if (triggerData.startsWith("signalComplete"))
		{
			executeSignalCompleteMessage(self, parse);
		}
	}
	
	
	public void executeEmoteTrigger(obj_id self, String[] parse) throws InterruptedException
	{
		String chatType = parse[1];
		String chatMessage = parse[2];
		
		chat.chat(self, chatType, new string_id("sequencer_spam", chatMessage));
		
	}
	
	
	public void executeSignalCompleteMessage(obj_id self, String[] parse) throws InterruptedException
	{
		dictionary dict = trial.getSessionDict(trial.getParent(self), "wfc_signal");
		dict.put("waitForComplete", parse[1]);
		
		messageTo(trial.getParent(self), "waitForComplete", dict, 0.0f, false);
		
	}
	
	
	public void executeSignalMasterTrigger(obj_id self, String[] parse) throws InterruptedException
	{
		String handlerName = parse[1];
		dictionary dict = trial.getSessionDict(trial.getParent(self));
		
		if (!parse[2].equals("none"))
		{
			String[] valueSplit = split(parse[2], '=');
			
			if (valueSplit[0].equals("int"))
			{
				dict.put(parse[1], utils.stringToInt(valueSplit[1]));
			}
			if (valueSplit[0].equals("float"))
			{
				dict.put(parse[1], utils.stringToFloat(valueSplit[1]));
			}
			else
			{
				dict.put(parse[1], valueSplit[1]);
			}
		}
		
		messageTo(trial.getParent(self), handlerName, dict, 0.0f, false);
	}
	
	
	public void executeAnimationTrigger(obj_id self, String[] parse) throws InterruptedException
	{
		String animation = parse[1];
		dictionary dict = new dictionary();
		dict.put("animation", animation);
		messageTo(self, "wsDoAnimation", dict, 1.0f, false);
	}
	
	
	public void executeFacingTrigger(obj_id self, String[] parse) throws InterruptedException
	{
		String target = parse[1];
		String condition = parse[2];
		float yaw = 0.0f;
		
		if (target.equals("player"))
		{
			if (condition.equals("nearest"))
			{
				obj_id player = getClosestPlayer(getLocation(self));
				if (isIdValid(player))
				{
					faceTo(self, player);
				}
			}
		}
		else if (target.equals("spawn_id"))
		{
			Vector children = new Vector();
			children.setSize(0);
			
			if (utils.hasResizeableObjIdBatchScriptVar(trial.getParent(self), trial.PROT_CHILD_ARRAY))
			{
				children = utils.getResizeableObjIdBatchScriptVar(trial.getParent(self), trial.PROT_CHILD_ARRAY);
			}
			
			obj_id[] list = new obj_id[0];
			if (children != null)
			{
				list = new obj_id[children.size()];
				children.toArray(list);
			}
			
			obj_id[] spawnId = trial.getObjectsInListWithObjVar(list, "spawn_id");
			
			obj_id intended = getSpawnIdFromList(spawnId, condition);
			
			faceTo(self, intended);
		}
		else if (target.equals("yaw"))
		{
			if (condition.equals("default"))
			{
				yaw = getFloatObjVar(self, "yaw");
				setYaw(self, yaw);
			}
			if (condition.equals("previous"))
			{
				if (utils.hasScriptVar(self, "rec_yaw"))
				{
					yaw = utils.getFloatScriptVar(self, "rec_yaw");
					setYaw(self, yaw);
				}
			}
			if (condition.equals("random"))
			{
				yaw = rand(-180, 180);
				setYaw(self, yaw);
			}
		}
		
		utils.setScriptVar(self, "rec_yaw", yaw);
	}
	
	
	public void executeMessageSelfTrigger(obj_id self, String[] parse) throws InterruptedException
	{
		String handlerName = parse[1];
		dictionary dict = new dictionary();
		
		if (!parse[2].equals("none"))
		{
			String[] valueSplit = split(parse[3], '=');
			
			if (valueSplit[0].equals("int"))
			{
				dict.put(parse[2], utils.stringToInt(valueSplit[1]));
			}
			if (valueSplit[0].equals("float"))
			{
				dict.put(parse[2], utils.stringToFloat(valueSplit[1]));
			}
			else
			{
				dict.put(parse[2], valueSplit[1]);
			}
		}
		
		messageTo(self, handlerName, dict, 5.0f, false);
	}
	
	
	public void executeIdTrigger(obj_id self, String[] parse) throws InterruptedException
	{
		dictionary dict = trial.getSessionDict(trial.getParent(self));
		dict.put("triggerName", parse[1]);
		dict.put("triggerType", "triggerId");
		
		messageTo(trial.getParent(self), "triggerFired", dict, 0.0f, false);
	}
	
	
	public int resumeMovement(obj_id self, dictionary params) throws InterruptedException
	{
		if (hasSuspendedMovement(self))
		{
			resumeMovement(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void pathToPoint(obj_id self, String pointName) throws InterruptedException
	{
		location toPoint = getPointLocation(pointName);
		pathTo(self, toPoint);
		return;
	}
	
	
	public void calculatePathPoint(obj_id self, String pointName) throws InterruptedException
	{
		
	}
	
	
	public void beginPathRoutine(obj_id self, String pathName) throws InterruptedException
	{
		setObjVar(self, "patrol_path", pathName);
		
		if (ai_lib.isInCombat(self))
		{
			return;
		}
		
		ai_lib.clearPatrolPath(self);
		
		dictionary pathInfo = utils.getDictionaryScriptVar(self, trial.SEQUENCER_PATH_DATA);
		
		if (pathInfo == null || pathInfo.isEmpty())
		{
			return;
		}
		
		String[] pathPointNameArray = pathInfo.getStringArray("pathPointNameArray");
		location[] pathPointLocationArray = pathInfo.getLocationArray("pathPointLocationArray");
		String[] patrolNameMaster = pathInfo.getStringArray("patrolNameMaster");
		String[] patrolTypeMaster = pathInfo.getStringArray("patrolTypeMaster");
		String[] patrolListMaster = pathInfo.getStringArray("patrolListMaster");
		
		int idx = utils.getElementPositionInArray(patrolNameMaster, pathName);
		
		if (idx < 0 || idx >= patrolTypeMaster.length)
		{
			doLogging("beginPathRoutine", "Bad idx: "+ idx + " patrolTypeMaster.length: "+ patrolTypeMaster.length + " pathName: "+ pathName);
			return;
		}
		
		String patrolType = patrolTypeMaster[idx];
		
		if (idx < 0 || idx >= patrolListMaster.length)
		{
			doLogging("beginPathRoutine", "Bad idx: "+ idx + " patrolListMaster.length: "+ patrolListMaster.length + " pathName: "+ pathName);
			return;
		}
		
		String[] pathPointList = filterRandomPoints(patrolListMaster[idx]);
		location[] pathPointLocationList = getLocationListFromPointList(pathPointList);
		
		initiatePatrol(self, patrolType , pathPointLocationList);
		
	}
	
	
	public String[] filterRandomPoints(String patrolListString) throws InterruptedException
	{
		String[] thisList = split(patrolListString, ',');
		
		for (int i=0; i<thisList.length; i++)
		{
			testAbortScript();
			String[] parse = split(thisList[i], '|');
			thisList[i] = parse[rand(0, parse.length - 1)];
		}
		
		return thisList;
	}
	
	
	public location[] getLocationListFromPointList(String[] pointList) throws InterruptedException
	{
		location[] locList = new location[pointList.length];
		
		for (int i=0; i<pointList.length; i++)
		{
			testAbortScript();
			locList[i] = getPointLocation(pointList[i]);
		}
		
		return locList;
	}
	
	
	public location[] getPatrolPathData(String patrolData) throws InterruptedException
	{
		debugSpeakMsg(getSelf(), patrolData);
		return null;
	}
	
	
	public void initiatePatrol(obj_id self, String patrolType, location[] patrolPoints) throws InterruptedException
	{
		location[] ppl = getPplForNpc(self, patrolType, patrolPoints);
		
		if (patrolType.equals("patrol"))
		{
			ai_lib.setPatrolPath(self, ppl);
		}
		if (patrolType.equals("patrolOnce"))
		{
			ai_lib.setPatrolOncePath(self, ppl);
			addLocationTarget("remove_path", ppl[ppl.length -1], 1f);
		}
		if (patrolType.equals("patrolFlip"))
		{
			ai_lib.setPatrolFlipPath(self, ppl);
		}
		if (patrolType.equals("patrolFlipOnce"))
		{
			ai_lib.setPatrolFlipOncePath(self, ppl);
			addLocationTarget("remove_path", ppl[ppl.length -1], 1f);
		}
		if (patrolType.equals("patrolRandom"))
		{
			ai_lib.setPatrolRandomPath(self, ppl);
		}
		if (patrolType.equals("patrolRandomOnce"))
		{
			ai_lib.setPatrolRandomOncePath(self, ppl);
		}
		
		return;
	}
	
	
	public int setLocationTarget(obj_id self, dictionary params) throws InterruptedException
	{
		String wp_name = params.getString("wp_name");
		location spawnLoc = params.getLocation("loc");
		
		addLocationTarget(wp_name, spawnLoc, 1.0f);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void reportDeath(obj_id self, obj_id killer) throws InterruptedException
	{
		int row = getIntObjVar(self, "row");
		
		String spawn_id = "none";
		if (hasObjVar(self, "spawn_id"))
		{
			spawn_id = getStringObjVar(self, "spawn_id");
		}
		
		int respawn = getIntObjVar(self, "respawn");
		
		dictionary dict = trial.getSessionDict(trial.getParent(self));
		dict.put("object", self);
		dict.put("row", row);
		dict.put("spawn_id", spawn_id);
		dict.put("respawn", respawn);
		dict.put("killer", killer);
		
		messageTo(trial.getParent(self), "terminationCallback", dict, 0.0f, false);
	}
	
	
	public obj_id getSpawnIdFromList(obj_id[] list, String spawn) throws InterruptedException
	{
		for (int i=0; i<list.length; i++)
		{
			testAbortScript();
			if (getStringObjVar(list[i], "spawn_id").equals(spawn))
			{
				return list[i];
			}
		}
		
		return null;
	}
	
	
	public location getPointLocation(String pointName) throws InterruptedException
	{
		obj_id self = getSelf();
		
		dictionary dict = utils.getDictionaryScriptVar(self, trial.SEQUENCER_PATH_DATA);
		
		if (dict == null || dict.isEmpty())
		{
			return null;
		}
		
		String[] pointList = dict.getStringArray("pathPointNameArray");
		location[] locationList = dict.getLocationArray("pathPointLocationArray");
		
		if (pointList == null || pointList.length <= 0)
		{
			doLogging("getPointLocation", "pointList null or empty. pointName: "+ pointName + " ["+ self + "] - "+ getName(self));
			return null;
		}
		
		if (locationList == null || locationList.length <= 0)
		{
			doLogging("getPointLocation", "locationList null or empty. pointName: "+ pointName + " ["+ self + "] - "+ getName(self));
			return null;
		}
		
		int idx = utils.getElementPositionInArray(pointList, pointName);
		
		if (idx < 0 || idx > pointList.length - 1)
		{
			doLogging("getPointLocation", "Failed to get good index: "+ idx + " for point: "+ pointName + " ["+ self + "] - "+ getName(self));
			return null;
		}
		
		return locationList[idx];
	}
	
	
	public location[] getPplForNpc(obj_id sender, String patrolType, location[] patrolPoints) throws InterruptedException
	{
		int idx = getNearestPatrolPointIndex(sender, patrolPoints);
		patrolPoints = trial.shuffleByIndex(patrolType, patrolPoints, idx);
		return patrolPoints;
	}
	
	
	public int getNearestPatrolPointIndex(obj_id sender, location[] patrolLoc) throws InterruptedException
	{
		
		float distance = 2000.0f;
		int idx = 0;
		
		for (int i=0; i<patrolLoc.length; i++)
		{
			testAbortScript();
			
			float thisDistance = getDistance(sender, patrolLoc[i]);
			
			if (thisDistance < 0.0f)
			{
				continue;
			}
			
			if (thisDistance < distance)
			{
				idx = i;
				distance = thisDistance;
			}
		}
		return idx;
		
	}
	
	
	public void doLogging(String section, String message) throws InterruptedException
	{
		if (LOGGING)
		{
			LOG("doLogging/ai_controller/"+section, message);
		}
	}
}
