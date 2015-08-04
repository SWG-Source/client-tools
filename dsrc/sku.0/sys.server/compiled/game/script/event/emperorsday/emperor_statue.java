package script.event.emperorsday;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.ai_lib;
import script.library.chat;
import script.library.collection;
import script.library.create;
import script.library.factions;
import script.library.firework;
import script.library.holiday;
import script.library.performance;
import script.library.prose;
import script.library.static_item;
import script.library.trial;
import script.library.utils;


public class emperor_statue extends script.base_script
{
	public emperor_statue()
	{
	}
	public static final String MUSIC_IMPERIAL_MARCH = "object/soundobject/soundobject_imperial_march.iff";
	public static final String MUSIC_IMPERIAL_ATTACK = "object/soundobject/soundobject_imperial_attack.iff";
	
	public static final String EMP_DAY = "event/emperors_day";
	
	public static final string_id VENDOR_1 = new string_id(EMP_DAY, "imp_vendor_1");
	
	public static final string_id DARTH_1 = new string_id(EMP_DAY, "imp_darth_1");
	
	public static final string_id VENDOR_2 = new string_id(EMP_DAY, "imp_vendor_2");
	
	public static final string_id DARTH_2 = new string_id(EMP_DAY, "imp_darth_2");
	
	public static final string_id DARTH_3 = new string_id(EMP_DAY, "imp_darth_3");
	
	public static final string_id DARTH_4 = new string_id(EMP_DAY, "imp_darth_4");
	
	public static final string_id DARTH_5 = new string_id(EMP_DAY, "imp_darth_5");
	
	public static final string_id DARTH_6 = new string_id(EMP_DAY, "imp_darth_6_string");
	
	public static final string_id DARTH_6_ALT = new string_id(EMP_DAY, "imp_darth_6_alt");
	
	public static final string_id VENDOR_3 = new string_id(EMP_DAY, "imp_vendor_3_string");
	
	public static final string_id DARTH_7 = new string_id(EMP_DAY, "imp_darth_7");
	
	public static final string_id DARTH_ANNOYED = new string_id(EMP_DAY, "imp_darth_annoyed_string");
	
	public static final string_id VENDOR_REPLY_ANNOYED = new string_id(EMP_DAY, "imp_vendor_annoyed");
	
	public static final string_id DARTH_ANNOYED_2_SINGULAR = new string_id(EMP_DAY, "imp_darth_annoyed_2_string_singular");
	public static final string_id DARTH_ANNOYED_2_PLURAL = new string_id(EMP_DAY, "imp_darth_annoyed_2_string_plural");
	
	public static final string_id VENDOR_REPLY_ANNOYED_2 = new string_id(EMP_DAY, "imp_vendor_annoyed_2");
	
	public static final string_id TREE_USE = new string_id(EMP_DAY, "emperor_statue_use");
	public static final string_id NOT_OLD_ENOUGH = new string_id(EMP_DAY, "not_old_enough");
	public static final string_id WINNER_HELP = new string_id(EMP_DAY, "winner_help_empire");
	
	public static final float LAMBDA_YAW = -0.01f;
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "emperor_statue.OnInitialize: Init.");
		
		if (!utils.hasScriptVar(self, "musicObject"))
		{
			location soundLoc = getLocation(self);
			obj_id soundObject = createObject(MUSIC_IMPERIAL_ATTACK, soundLoc);
			
			utils.setScriptVar(self, "musicObject", soundObject);
		}
		else if (utils.hasScriptVar(self, "musicObject"))
		{
			obj_id oldSoundObject = utils.getObjIdScriptVar(self, "musicObject");
			
			if ((isIdValid(oldSoundObject)) && exists(oldSoundObject))
			{
				destroyObject(oldSoundObject);
			}
			
			location soundLoc = getLocation(self);
			obj_id soundObject = createObject(MUSIC_IMPERIAL_ATTACK, soundLoc);
			
			utils.setScriptVar(self, "musicObject", soundObject);
		}
		
		CustomerServiceLog("holidayEvent", "emperor_statue.OnInitialize: Preparing Parade.");
		messageTo(self, "prepareParade", null, 10.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		int idx = utils.getValidAttributeIndex(names);
		if (idx == -1)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		if (utils.hasScriptVar(self, "paradeRunning"))
		{
			names[idx] = "next_ceremony";
			attribs[idx] = "Now";
			idx++;
		}
		else
		{
			
			int nextCeremony = getIntObjVar(self, "ceremonyTime");
			if (nextCeremony <= 0)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
			
			names[idx] = "next_ceremony";
			attribs[idx] = getCalendarTimeStringLocal(nextCeremony);
			idx++;
			
			names[idx] = "count_down";
			attribs[idx] = utils.formatTimeVerbose(nextCeremony - getCalendarTime());
			idx++;
		}
		
		String statusString = holiday.getEmpireDayEligibility(player, holiday.IMPERIAL_PLAYER);
		if (statusString != null && !statusString.equals(""))
		{
			names[idx] = "empire_day_player_status";
			attribs[idx] = statusString;
			idx++;
		}
		
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (!isIdValid(player) || ai_lib.isInCombat(player) || isIncapacitated(player) || isDead(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!factions.isImperial(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(player, holiday.currentYearObjVar()))
		{
			mi.addRootMenu(menu_info_types.ITEM_USE, TREE_USE);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (!isIdValid(player) || ai_lib.isInCombat(player) || isIncapacitated(player) || isDead(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!factions.isImperial(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (item != menu_info_types.ITEM_USE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if ((getCurrentBirthDate() - getPlayerBirthDate(player)) < 10)
		{
			sendSystemMessage(player, NOT_OLD_ENOUGH);
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(player, holiday.currentYearObjVar()))
		{
			holiday.grantEmpireDayGift(player, holiday.IMPERIAL_PLAYER);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		
		obj_id oldSoundObject = utils.getObjIdScriptVar(self, "musicObject");
		
		if ((isIdValid(oldSoundObject)) && exists(oldSoundObject))
		{
			destroyObject(oldSoundObject);
		}
		
		if (utils.hasScriptVar(self, "vader"))
		{
			Vector npc = utils.getResizeableObjIdArrayScriptVar(self, "vader");
			for (int i=0; i<npc.size(); i++)
			{
				testAbortScript();
				if (isIdValid(((obj_id)(npc.get(i)))) && exists(((obj_id)(npc.get(i)))))
				{
					destroyObject(((obj_id)(npc.get(i))));
				}
			}
			utils.removeScriptVar(self, "vader");
		}
		if (utils.hasScriptVar(self, "dark_trooper"))
		{
			Vector npc = utils.getResizeableObjIdArrayScriptVar(self, "dark_trooper");
			for (int i=0; i<npc.size(); i++)
			{
				testAbortScript();
				if (isIdValid(((obj_id)(npc.get(i)))) && exists(((obj_id)(npc.get(i)))))
				{
					destroyObject(((obj_id)(npc.get(i))));
				}
			}
			utils.removeScriptVar(self, "dark_trooper");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnHearSpeech(obj_id self, obj_id speaker, String text) throws InterruptedException
	{
		if (!isGod(speaker))
		{
			return SCRIPT_CONTINUE;
		}
		if (text.equals("getyaw"))
		{
			sendSystemMessage(speaker, "Statue Yaw: "+getYaw(self), "");
			return SCRIPT_OVERRIDE;
		}
		else if (text.equals("createlambda"))
		{
			sendSystemMessage(speaker, "Creating Shuttle", "");
			
			createLambdaDropship(self, getLocation(speaker));
			return SCRIPT_OVERRIDE;
		}
		
		else if (text.equals("startParade"))
		{
			obj_id oldSoundObject = utils.getObjIdScriptVar(self, "musicObject");
			
			if ((isIdValid(oldSoundObject)) && exists(oldSoundObject))
			{
				destroyObject(oldSoundObject);
			}
			
			sendSystemMessage(speaker, "Manually starting parade", "");
			messageTo(self, "prepareParade", null, 10.0f, false);
			return SCRIPT_OVERRIDE;
		}
		else if (text.equals("removeParade"))
		{
			obj_id oldSoundObject = utils.getObjIdScriptVar(self, "musicObject");
			
			if ((isIdValid(oldSoundObject)) && exists(oldSoundObject))
			{
				destroyObject(oldSoundObject);
			}
			
			sendSystemMessage(speaker, "Manually destroying parade", "");
			messageTo(self, "destroyParade", null, 10.0f, false);
			return SCRIPT_OVERRIDE;
		}
		else if (text.equals("correctLocation"))
		{
			location yodaLoc = new location(-5495.0f, 6.0f, 4407.3f);
			setLocation(self, yodaLoc);
			location actualLocation = getLocation(self);
			sendSystemMessage(speaker, "Fountain location is "+actualLocation, "");
			return SCRIPT_OVERRIDE;
		}
		else if (text.equals("removeMyBadges") || text.equals("removeBadges"))
		{
			if (hasCompletedCollectionSlot(speaker, holiday.EMPIRE_DAY_CHAMPION_BADGE))
			{
				sendSystemMessage(speaker, "Removed Imperial Badge", "");
				if (modifyCollectionSlotValue(speaker, holiday.EMPIRE_DAY_CHAMPION_BADGE, -1))
				{
					sendSystemMessage(speaker, holiday.SID_REM_EMP_DAY_BADGE);
				}
			}
			if (hasCompletedCollectionSlot(speaker, holiday.REMEMBRANCE_DAY_CHAMPION_BADGE))
			{
				sendSystemMessage(speaker, "Removed Rebel Badge", "");
				if (modifyCollectionSlotValue(speaker, holiday.REMEMBRANCE_DAY_CHAMPION_BADGE, -1))
				{
					sendSystemMessage(speaker, holiday.SID_REM_REM_DAY_BADGE);
				}
			}
		}
		else if (text.equals("removeRewardFlag") || text.equals("removeRewards") || text.equals("removeReward"))
		{
			removeObjVar(speaker, holiday.currentYearObjVar());
			sendSystemMessage(speaker, holiday.SID_REMOVED_REWARD_FLAG);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int npcTypeList(obj_id self, dictionary params) throws InterruptedException
	{
		
		String npcType = params.getString("type");
		obj_id npc = params.getObjId("npcObjId");
		
		Vector npcs = new Vector();
		npcs.setSize(0);
		
		if (utils.hasScriptVar(self, npcType))
		{
			npcs = utils.getResizeableObjIdArrayScriptVar(self, npcType);
		}
		utils.addElement(npcs, npc);
		utils.setScriptVar(self, npcType, npcs);
		
		if (npcType.equals("vader"))
		{
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int prepareParade(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "emperor_statue.prepareParade: messageHandler initialized.");
		
		if (!utils.hasScriptVar(self, "paradeRunning"))
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.prepareParade: Parade is not running.");
			
			utils.setScriptVar(self, "paradeRunning", 1);
			obj_id oldSoundObject = utils.getObjIdScriptVar(self, "musicObject");
			
			if ((isIdValid(oldSoundObject)) && exists(oldSoundObject))
			{
				CustomerServiceLog("holidayEvent", "emperor_statue.prepareParade: Destroying Sound Object.");
				destroyObject(oldSoundObject);
			}
			
			messageTo(self, "parade", null, 10.0f, false);
			
		}
		else
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.prepareParade: Failed to start because the statue says the parade is running.");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int parade(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "emperor_statue.parade: messageHandler initialized.");
		
		obj_id vendor = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vendor");
		if (isIdValid(vendor) || exists(vendor))
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.parade: removing conversation script from vendor so players cannot interrupt the ceremony.");
			detachScript(vendor, "conversation.imperial_emperorsday_vendor");
		}
		
		if (!utils.hasScriptVar(self, "musicObject"))
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.parade: Creating new music object.");
			
			location soundLoc = getLocation(self);
			obj_id soundObject = createObject(MUSIC_IMPERIAL_MARCH, soundLoc);
			
			utils.setScriptVar(self, "musicObject", soundObject);
		}
		else if (utils.hasScriptVar(self, "musicObject"))
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.parade: Getting existing music object.");
			
			obj_id oldSoundObject = utils.getObjIdScriptVar(self, "musicObject");
			
			if ((isIdValid(oldSoundObject)) && exists(oldSoundObject))
			{
				destroyObject(oldSoundObject);
			}
			
			location soundLoc = getLocation(self);
			obj_id soundObject = createObject(MUSIC_IMPERIAL_MARCH, soundLoc);
			
			utils.setScriptVar(self, "musicObject", soundObject);
		}
		
		CustomerServiceLog("holidayEvent", "emperor_statue.parade: Getting shuttle waypoint data in preparation for shuttle landing.");
		
		obj_id spawnPoint = holiday.getEmpireDayWaypointObjectObjId(self, "shuttlePathingLastPoint", holiday.OBJECT_NEAR_CHECK_RANGE_100M);
		obj_id exitPoint = holiday.getEmpireDayWaypointObjectObjId(self, "shuttlePathingMidPoint", holiday.OBJECT_NEAR_CHECK_RANGE_100M);
		
		location spawnLoc = getLocation(spawnPoint);
		boolean lambdaCreation = createLambdaDropship(self, spawnLoc);
		
		CustomerServiceLog("holidayEvent", "emperor_statue.parade: Shuttle spawnPoint: "+spawnPoint);
		CustomerServiceLog("holidayEvent", "emperor_statue.parade: Shuttle spawnPoint spawnLoc: "+spawnLoc);
		CustomerServiceLog("holidayEvent", "emperor_statue.parade: lambda was created: "+lambdaCreation);
		
		if (!lambdaCreation)
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.parade: The shuttle failed to spawn!! Notify design immediately.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id lambda = utils.getObjIdScriptVar(self, "lambdaShuttle");
		if (isValidId(lambda) && exists(lambda))
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.parade: Sending Message to move to new point in X seconds");
			setObjVar(lambda, "exitpoint", exitPoint);
			messageTo(self, "shuttleAdjust", null, 5, false);
		}
		
		CustomerServiceLog("holidayEvent", "emperor_statue.parade: Sending Message to NPCS to spawn in 25 seconds");
		
		messageTo(self, "spawnDarkTroopers1", null, 28.0f, false);
		messageTo(self, "spawnDarkTroopers2", null, 28.0f, false);
		
		messageTo(self, "spawnVader", null, 35.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int shuttleAdjust(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "emperor_statue.parade: Adjusting location now");
		
		obj_id lambda = utils.getObjIdScriptVar(self, "lambdaShuttle");
		if (isValidId(lambda) && exists(lambda))
		{
			setMovementPercent(lambda, 18);
			
			obj_id spawnPoint = holiday.getEmpireDayWaypointObjectObjId(self, "shuttleLanding", holiday.OBJECT_NEAR_CHECK_RANGE_100M);
			location loc = getLocation(spawnPoint);
			pathTo(lambda, loc);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int spawnDarkTroopers1(obj_id self, dictionary params) throws InterruptedException
	{
		int squadSize = 4;
		int formation = ai_lib.FORMATION_COLUMN;
		
		obj_id[] squad = new obj_id[4];
		
		obj_id spawnPoint = holiday.getEmpireDayWaypointObjectObjId(self, "spawnDark1", holiday.OBJECT_NEAR_CHECK_RANGE_100M);
		location spawnLoc = getLocation(spawnPoint);
		location tempLoc = (location)spawnLoc.clone();
		
		for (int i=0; i<squad.length; i++)
		{
			testAbortScript();
			tempLoc.z = spawnLoc.z+i*2;
			squad[i] = create.object("imperial_emperorsday_dark_trooper", tempLoc);
			setYaw(squad[i], -90.0f);
			ai_lib.setDefaultCalmBehavior(squad[i], ai_lib.BEHAVIOR_SENTINEL);
			
			if (isIdValid(squad[i]))
			{
				setObjVar(squad[i], "darkTroopers1", 1);
				trial.markAsTempObject(squad[i], true);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int spawnDarkTroopers2(obj_id self, dictionary params) throws InterruptedException
	{
		int squadSize = 4;
		int formation = ai_lib.FORMATION_COLUMN;
		
		obj_id[] squad = new obj_id[4];
		
		obj_id spawnPoint = holiday.getEmpireDayWaypointObjectObjId(self, "spawnDark2", holiday.OBJECT_NEAR_CHECK_RANGE_100M);
		location spawnLoc = getLocation(spawnPoint);
		location tempLoc = (location)spawnLoc.clone();
		
		for (int i=0; i<squad.length; i++)
		{
			testAbortScript();
			tempLoc.z = spawnLoc.z+i*2;
			squad[i] = create.object("imperial_emperorsday_dark_trooper", tempLoc);
			setYaw(squad[i], 90.0f);
			ai_lib.setDefaultCalmBehavior(squad[i], ai_lib.BEHAVIOR_SENTINEL);
			if (isIdValid(squad[i]))
			{
				setObjVar(squad[i], "darkTroopers2", 1);
				trial.markAsTempObject(squad[i], true);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int spawnVader(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id spawnPoint = holiday.getEmpireDayWaypointObjectObjId(self, "spawnVader", holiday.OBJECT_NEAR_CHECK_RANGE_100M);
		location spawnLoc = getLocation(spawnPoint);
		
		obj_id vader = create.object("imperial_emperorsday_darth_vader", spawnLoc);
		setYaw(vader, 0.0f);
		if (isIdValid(vader))
		{
			setObjVar(vader, performance.NPC_ENTERTAINMENT_NO_ENTERTAIN, 1);
			setObjVar(vader, "vader", 1);
			trial.markAsTempObject(vader, true);
			setObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vader", vader);
			
			messageTo(self, "moveParade", null, 10.0f, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int moveParade(obj_id self, dictionary params) throws InterruptedException
	{
		location loc = getLocation(self);
		obj_id[] objects = getObjectsInRange(loc, holiday.OBJECT_NEAR_CHECK_RANGE_100M);
		Vector squad1 = new Vector();
		squad1.setSize(0);
		Vector squad2 = new Vector();
		squad2.setSize(0);
		for (int i = 0; i < objects.length; i++)
		{
			testAbortScript();
			if (isIdValid(objects[i]))
			{
				if (hasObjVar(objects[i], "darkTroopers1"))
				{
					utils.addElement(squad1, objects[i]);
				}
				if (hasObjVar(objects[i], "darkTroopers2"))
				{
					utils.addElement(squad2, objects[i]);
				}
				if (hasObjVar(objects[i], "vader"))
				{
					obj_id pathPoint = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vader_path");
					if (isIdValid(pathPoint) && exists(pathPoint))
					{
						location pathLoc = getLocation(pathPoint);
						pathTo (objects[i], pathLoc);
					}
				}
			}
		}
		if (squad1.size() == squad2.size())
		{
			for (int i = 0; i < squad1.size(); i++)
			{
				testAbortScript();
				if (isIdValid(((obj_id)(squad1.get(i))))&&isIdValid(((obj_id)(squad2.get(i)))))
				{
					obj_id pathPoint1 = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"darktrooper_path_right");
					if (isIdValid(pathPoint1) && exists(pathPoint1))
					{
						location pathLoc1 = getLocation(pathPoint1);
						location tempLoc1 = (location)pathLoc1.clone();
						tempLoc1.z = pathLoc1.z-i*2;
						pathTo (((obj_id)(squad1.get(i))), tempLoc1);
					}
					
					obj_id pathPoint2 = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"darktrooper_path_left");
					if (isIdValid(pathPoint2) && exists(pathPoint2))
					{
						location pathLoc2 = getLocation(pathPoint2);
						location tempLoc2 = (location)pathLoc2.clone();
						tempLoc2.z = pathLoc2.z-i*2;
						pathTo (((obj_id)(squad2.get(i))), tempLoc2);
					}
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int launchRandomTieFighterFlyBy(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "emperor_statue.launchRandomTieFighterFlyBy: messageHandler initialized.");
		int num = rand(1,4);
		obj_id cauldronRight = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"cauldron_right");
		obj_id cauldronLeft = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"cauldron_left");
		if (!isValidId(cauldronRight) && !exists(cauldronRight) && !isValidId(cauldronRight) && !exists(cauldronRight))
		{
			return SCRIPT_CONTINUE;
		}
		CustomerServiceLog("holidayEvent", "emperor_statue.launchRandomTieFighterFlyBy: random number received, firing something.");
		
		switch(num)
		{
			case 1:  
			CustomerServiceLog("holidayEvent", "emperor_statue.launchRandomTieFighterFlyBy: case 1.");
			playTieFighterPair(self, cauldronLeft);
			break;
			case 2:  
			CustomerServiceLog("holidayEvent", "emperor_statue.launchRandomTieFighterFlyBy: case 2.");
			playTieFighterGroup(self, cauldronLeft);
			break;
			case 3:  
			CustomerServiceLog("holidayEvent", "emperor_statue.launchRandomTieFighterFlyBy: case 3.");
			playTieFighterPair(self, cauldronRight);
			break;
			case 4:  
			CustomerServiceLog("holidayEvent", "emperor_statue.launchRandomTieFighterFlyBy: case 4.");
			playTieFighterGroup(self, cauldronRight);
			break;
			default:
			CustomerServiceLog("holidayEvent", "emperor_statue.launchRandomTieFighterFlyBy: case 5.");
			playTieFighterGroup(self, cauldronRight);
			break;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int launchRandomFirework(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "emperor_statue.launchRandomFirework: messageHandler initialized.");
		
		location statue = getLocation(self);
		location here = (location)statue.clone();
		here.z = statue.z+20;
		
		location there = utils.getRandomLocationInRing(here, 0, 10);
		
		int tableLength = dataTableGetNumRows(firework.TBL_FX);
		int roll = rand(1, tableLength);
		String template = dataTableGetString(firework.TBL_FX, roll, "template");
		
		obj_id effect = create.object(template, there);
		if (isIdValid(effect))
		{
			attachScript(effect, firework.SCRIPT_FIREWORK_CLEANUP);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int vaderLeaving(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "emperor_statue.vaderLeaving: messageHandler initialized.");
		
		if (utils.hasScriptVar(self, "stormtrooper_squadleader"))
		{
			Vector npc = utils.getResizeableObjIdArrayScriptVar(self, "stormtrooper_squadleader");
			for (int i=0; i<npc.size(); i++)
			{
				testAbortScript();
				if (isIdValid(((obj_id)(npc.get(i)))) && exists(((obj_id)(npc.get(i)))))
				{
					doAnimationAction(((obj_id)(npc.get(i))), "salute2");
				}
			}
		}
		
		obj_id vendor = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vendor");
		if (isIdValid(vendor) || exists(vendor))
		{
			doAnimationAction(vendor, "salute2");
		}
		
		obj_id vader = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vader");
		if (isIdValid(vader) || exists(vader))
		{
			chat.chat(vader, DARTH_7);
			messageTo(self, "finalMarch", null, 5.0f, false);
		}
		
		if (!hasObjVar(vader, "readyForBadge"))
		{
			holiday.playEmpireDayFireWorksAndFlyBys(self, holiday.IMPERIAL_PLAYER);
		}
		
		if (utils.hasScriptVar(self, "dark_trooper"))
		{
			Vector npc = utils.getResizeableObjIdArrayScriptVar(self, "dark_trooper");
			for (int i=0; i<npc.size(); i++)
			{
				testAbortScript();
				if (isIdValid(((obj_id)(npc.get(i)))) && exists(((obj_id)(npc.get(i)))))
				{
					if (hasObjVar(((obj_id)(npc.get(i))), "darkTroopers1"))
					{
						setYaw(((obj_id)(npc.get(i))), -180.0f);
					}
					if (hasObjVar(((obj_id)(npc.get(i))), "darkTroopers2"))
					{
						setYaw(((obj_id)(npc.get(i))), -180.0f);
					}
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int finalMarch(obj_id self, dictionary params) throws InterruptedException
	{
		location loc = getLocation(self);
		obj_id[] objects = getObjectsInRange(loc, holiday.OBJECT_NEAR_CHECK_RANGE_100M);
		Vector squad1 = new Vector();
		squad1.setSize(0);
		Vector squad2 = new Vector();
		squad2.setSize(0);
		for (int i = 0; i < objects.length; i++)
		{
			testAbortScript();
			if (isIdValid(objects[i]))
			{
				if (hasObjVar(objects[i], "darkTroopers1"))
				{
					utils.addElement(squad1, objects[i]);
				}
				if (hasObjVar(objects[i], "darkTroopers2"))
				{
					utils.addElement(squad2, objects[i]);
				}
				if (hasObjVar(objects[i], "vader"))
				{
					obj_id pathPoint = holiday.getEmpireDayWaypointObjectObjId(self, "spawnVader", holiday.OBJECT_NEAR_CHECK_RANGE_100M);
					location pathLoc = getLocation(pathPoint);
					
					pathTo (objects[i], pathLoc);
				}
			}
		}
		if (squad1.size() == squad2.size())
		{
			for (int i = 0; i < squad1.size(); i++)
			{
				testAbortScript();
				if (isIdValid(((obj_id)(squad1.get(i))))&&isIdValid(((obj_id)(squad2.get(i)))))
				{
					obj_id pathPoint1 = holiday.getEmpireDayWaypointObjectObjId(self, "spawnDark1", holiday.OBJECT_NEAR_CHECK_RANGE_100M);
					location pathLoc1 = getLocation(pathPoint1);
					location tempLoc1 = (location)pathLoc1.clone();
					tempLoc1.z = pathLoc1.z+i*2;
					
					pathTo (((obj_id)(squad1.get(i))), tempLoc1);
					
					obj_id pathPoint2 = holiday.getEmpireDayWaypointObjectObjId(self, "spawnDark2", holiday.OBJECT_NEAR_CHECK_RANGE_100M);
					location pathLoc2 = getLocation(pathPoint2);
					location tempLoc2 = (location)pathLoc2.clone();
					tempLoc2.z = pathLoc2.z+i*2;
					
					pathTo (((obj_id)(squad2.get(i))), tempLoc2);
				}
			}
		}
		
		messageTo(self, "destroyParade", null, 60, false);
		obj_id lambda = utils.getObjIdScriptVar(self, "lambdaShuttle");
		if (!isValidId(lambda) || !exists(lambda))
		{
			return SCRIPT_CONTINUE;
		}
		
		messageTo(lambda, "takeOff", null, 90f, true);
		return SCRIPT_CONTINUE;
	}
	
	
	public int destroyParade(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "emperor_statue.destroyParade: Destroy Parade, Destroying Parade now.");
		
		obj_id vendor = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vendor");
		if (isIdValid(vendor) || exists(vendor))
		{
			if (!hasScript(vendor, "conversation.imperial_emperorsday_vendor"))
			{
				attachScript(vendor, "conversation.imperial_emperorsday_vendor");
			}
		}
		
		obj_id lambda = utils.getObjIdScriptVar(self, "lambdaShuttle");
		
		if ((isIdValid(lambda)) && exists(lambda))
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.destroyParade: Lambda getting take off instructions in 20 seconds.");
			
			messageTo(lambda, "takeOff", null, 20.0f, false);
			utils.removeScriptVar(self, "lambdaShuttle");
		}
		
		obj_id vader = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vader");
		if (isIdValid(vader) || exists(vader))
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.destroyParade: Destroying all parade NPCs that are vader.");
			
			Vector npc = utils.getResizeableObjIdArrayScriptVar(self, "vader");
			
			for (int i=0; i<npc.size(); i++)
			{
				testAbortScript();
				if (isIdValid(vader) && exists(vader))
				{
					destroyObject(((obj_id)(npc.get(i))));
				}
			}
			utils.removeScriptVar(self, "vader");
			removeObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vader");
		}
		
		if (utils.hasScriptVar(self, "dark_trooper"))
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.destroyParade: Destroying all parade NPCs that are dark troppers.");
			
			Vector npc = utils.getResizeableObjIdArrayScriptVar(self, "dark_trooper");
			for (int i=0; i<npc.size(); i++)
			{
				testAbortScript();
				if (isIdValid(((obj_id)(npc.get(i)))) && exists(((obj_id)(npc.get(i)))))
				{
					destroyObject(((obj_id)(npc.get(i))));
				}
			}
			utils.removeScriptVar(self, "dark_trooper");
		}
		
		messageTo(self, "normalMusic", null, 15.0f, false);
		
		messageTo(self, "prepareParade", null, holiday.EMPIRE_DAY_SECONDS_TO_START, false);
		
		setObjVar(self, "ceremonyTime", getCalendarTime() + holiday.EMPIRE_DAY_SECONDS_TO_START);
		
		CustomerServiceLog("holidayEvent", "emperor_statue.destroyParade: Restarting Parade in "+holiday.EMPIRE_DAY_SECONDS_TO_START+" seconds.");
		utils.removeScriptVar(self, "paradeRunning");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int normalMusic(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, "musicObject"))
		{
			location soundLoc = getLocation(self);
			obj_id soundObject = createObject(MUSIC_IMPERIAL_ATTACK, soundLoc);
			
			utils.setScriptVar(self, "musicObject", soundObject);
		}
		else if (utils.hasScriptVar(self, "musicObject"))
		{
			obj_id oldSoundObject = utils.getObjIdScriptVar(self, "musicObject");
			
			if ((isIdValid(oldSoundObject)) && exists(oldSoundObject))
			{
				destroyObject(oldSoundObject);
			}
			
			location soundLoc = getLocation(self);
			obj_id soundObject = createObject(MUSIC_IMPERIAL_ATTACK, soundLoc);
			
			utils.setScriptVar(self, "musicObject", soundObject);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int startConversation(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (utils.hasScriptVar(self, "stormtrooper_squadleader"))
		{
			Vector npc = utils.getResizeableObjIdArrayScriptVar(self, "stormtrooper_squadleader");
			for (int i=0; i<npc.size(); i++)
			{
				testAbortScript();
				if (isIdValid(((obj_id)(npc.get(i)))) && exists(((obj_id)(npc.get(i)))))
				{
					doAnimationAction(((obj_id)(npc.get(i))), "salute2");
				}
			}
		}
		
		obj_id vendor = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vendor");
		if (isIdValid(vendor) || exists(vendor))
		{
			doAnimationAction(vendor, "salute2");
			chat.chat(vendor, VENDOR_1);
			messageTo (self, "darth1", null, 5.0f, false);
		}
		
		if (utils.hasScriptVar(self, "dark_trooper"))
		{
			Vector squad1 = new Vector();
			squad1.setSize(0);
			Vector squad2 = new Vector();
			squad2.setSize(0);
			Vector npc = utils.getResizeableObjIdArrayScriptVar(self, "dark_trooper");
			for (int i=0; i<npc.size(); i++)
			{
				testAbortScript();
				if (isIdValid(((obj_id)(npc.get(i)))) && exists(((obj_id)(npc.get(i)))))
				{
					setYaw(((obj_id)(npc.get(i))), 0.0f);
					if (hasObjVar(((obj_id)(npc.get(i))), "darkTroopers1"))
					{
						utils.addElement(squad1,((obj_id)(npc.get(i))));
					}
					if (hasObjVar(((obj_id)(npc.get(i))), "darkTroopers2"))
					{
						utils.addElement(squad2, ((obj_id)(npc.get(i))));
					}
				}
			}
			if (squad1.size() == squad2.size())
			{
				for (int i = 0; i < squad1.size(); i++)
				{
					testAbortScript();
					if (isIdValid(((obj_id)(squad1.get(i))))&&isIdValid(((obj_id)(squad2.get(i)))))
					{
						setYaw(((obj_id)(squad1.get(i))), 90.0f);
						setYaw(((obj_id)(squad2.get(i))), -110.0f);
					}
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int darth1(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id vader = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vader");
		if (isIdValid(vader) || exists(vader))
		{
			chat.chat(vader, DARTH_1);
			messageTo (self, "vendor2", null, 10.0f, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int vendor2(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id vendor = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vendor");
		if (isIdValid(vendor) || exists(vendor))
		{
			doAnimationAction(vendor, "nod_head_multiple");
			chat.chat(vendor, VENDOR_2);
			messageTo (self, "darth2", null, 5.0f, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int darth2(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id vader = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vader");
		if (isIdValid(vader) || exists(vader))
		{
			doAnimationAction(vader, "nod_head_once");
			chat.chat(vader, DARTH_2);
			messageTo (self, "darth3", null, 10.0f, false);
			setYaw(vader, -180.0f);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int darth3(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id vader = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vader");
		if (isIdValid(vader) || exists(vader))
		{
			chat.chat(vader, DARTH_3);
			messageTo (self, "darth4", null, 15.0f, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int darth4(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id vader = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vader");
		if (isIdValid(vader) || exists(vader))
		{
			chat.chat(vader, DARTH_4);
			messageTo (self, "darth5", null, 15.0f, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int darth5(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id vader = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vader");
		if (isIdValid(vader) || exists(vader))
		{
			chat.chat(vader, DARTH_5);
			messageTo (self, "darth6", null, 15.0f, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int darth6(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "emperor_statue.darth6: messageHandler Initialized.");
		
		obj_id vader = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vader");
		if (isIdValid(vader) || exists(vader))
		{
			
			obj_id[] playersInRange = getAllPlayers(getLocation(self), 100.0f);
			if (playersInRange == null || playersInRange.length <= 0)
			{
				
				CustomerServiceLog("holidayEvent", "emperor_statue.darth6: There were no players in the area that were eligible. Vader is leaving due to Case 1.");
				
				chat.chat(vader, DARTH_6_ALT);
				messageTo(self, "vaderLeaving", null, 15.0f, false);
				return SCRIPT_CONTINUE;
			}
			
			Vector imperialPlayers = new Vector();
			imperialPlayers.setSize(0);
			for (int z = 0; z < playersInRange.length; z++)
			{
				testAbortScript();
				if (!isIdValid(playersInRange[z]) || !exists(playersInRange[z]))
				{
					continue;
				}
				
				if (holiday.isEmpireDayPlayerEligible(playersInRange[z], holiday.IMPERIAL_PLAYER))
				{
					utils.addElement(imperialPlayers, playersInRange[z]);
				}
			}
			
			if (imperialPlayers == null || imperialPlayers.size() <= 0)
			{
				CustomerServiceLog("holidayEvent", "emperor_statue.darth6: There were players in the area but none that were eligible. Vader is leaving.");
				
				chat.chat(vader, DARTH_6_ALT);
				messageTo(self, "vaderLeaving", null, 15.0f, false);
				return SCRIPT_CONTINUE;
			}
			
			obj_id[] listOfWinners = holiday.getEmpireDayWinningPlayers(self, imperialPlayers);
			if (listOfWinners == null || listOfWinners.length <= 0)
			{
				CustomerServiceLog("holidayEvent", "emperor_statue.darth6: A list of eligible players were sent to the getWinningPlayers function but it came back corrupted or null.");
				
				chat.chat(vader, DARTH_6_ALT);
				messageTo(self, "vaderLeaving", null, 15.0f, false);
				return SCRIPT_CONTINUE;
			}
			
			CustomerServiceLog("holidayEvent", "emperor_statue.darth6: A list of eligible players has been received and will now be announced. The list length is: "+listOfWinners.length);
			
			String winnerList = "";
			int winnerCount = listOfWinners.length;
			
			if (winnerCount == 1)
			{
				CustomerServiceLog("holidayEvent", "emperor_statue.darth6: The list length is only one player: "+listOfWinners[0]);
				
				prose_package pp = prose.getPackage(DARTH_6);
				winnerList = getFirstName(listOfWinners[0]);
				prose.setTO (pp, winnerList);
				chat.chat(vader, listOfWinners[0], pp);
				sendSystemMessage(listOfWinners[0], WINNER_HELP);
				
				params.put("winnerList", winnerList);
				params.put("listOfWinners", listOfWinners);
				
				messageTo(self, "vendor3", params, 10.0f, false);
				utils.setScriptVar(listOfWinners[0], "emperorsDayBadge", 1);
				setObjVar(vader, "listOfWinners", listOfWinners);
				
				setObjVar(vader, "readyForBadge.winner_1", listOfWinners[0]);
				messageTo(self, "waitingHalf", params, 45.0f, false);
				messageTo(self, "waitingDone", params, 70.0f, false);
			}
			else
			{
				CustomerServiceLog("holidayEvent", "emperor_statue.darth6: The list length is: "+listOfWinners.length);
				prose_package pp = prose.getPackage(DARTH_6);
				for (int i = 0; i < winnerCount; i++)
				{
					testAbortScript();
					if (i == 0)
					{
						winnerList += getFirstName(listOfWinners[i]);
					}
					else if (i == (winnerCount-1))
					{
						winnerList += " and "+getFirstName(listOfWinners[i]);
					}
					else
					{
						winnerList += ", "+getFirstName(listOfWinners[i]);
					}
					
					sendSystemMessage(listOfWinners[i], WINNER_HELP);
					utils.setScriptVar(listOfWinners[i], "emperorsDayBadge", 1);
					setObjVar(vader, "readyForBadge.winner_"+(i+1), listOfWinners[i]);
				}
				
				if (winnerList == null || winnerList.equals(""))
				{
					CustomerServiceLog("holidayEvent", "yoda_fountain.leia6: The player names to be called somehow got corrupted. Sending Leia to falcon.");
					chat.chat(vader, DARTH_6_ALT);
					messageTo(self, "vaderLeaving", null, 15.0f, false);
					return SCRIPT_CONTINUE;
				}
				
				prose.setTO(pp, winnerList);
				chat.chat(vader, listOfWinners[0], pp);
				
				CustomerServiceLog("holidayEvent", "yoda_fountain.leia6: setting winnerList on Leia.");
				setObjVar(vader, "listOfWinners", listOfWinners);
				
				params.put("winnerList", winnerList);
				params.put("listOfWinners", listOfWinners);
				messageTo(self, "vendor3", params, 10.0f, false);
				messageTo(self, "waitingHalf", params, 45.0f, false);
				messageTo(self, "waitingDone", params, 70.0f, false);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int vendor3(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "emperor_statue.vendor3: messageHandler Initialized.");
		
		if (params == null || params.equals(""))
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.vendor3: There were no params passed to the vendor3 message handler.");
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey("winnerList"))
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.vendor3: There was no winnerlist key passed in the params.");
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey("listOfWinners"))
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.vendor3: There was no listOfWinners key passed in the params.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id vader = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vendor");
		if (isIdValid(vader) || exists(vader))
		{
			if (!hasObjVar(vader, "readyForBadge"))
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		obj_id vendor = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vendor");
		if (isIdValid(vendor) || exists(vendor))
		{
			String winnerList = params.getString("winnerList");
			if (winnerList == null || winnerList.equals(""))
			{
				CustomerServiceLog("holidayEvent", "emperor_statue.vendor3: The vendor cannot speak out to ask the player to step forward because the player name string(s) were not found.");
				return SCRIPT_CONTINUE;
			}
			
			obj_id[] listOfWinners = params.getObjIdArray("listOfWinners");
			if (listOfWinners == null || listOfWinners.length <= 0)
			{
				CustomerServiceLog("holidayEvent", "emperor_statue.vendor3: The list of winners was not found. The vendor cannot ask the player(s) to step forward.");
				return SCRIPT_CONTINUE;
			}
			
			prose_package pp = prose.getPackage(VENDOR_3);
			prose.setTO (pp, winnerList);
			chat.chat(vendor, listOfWinners[0], pp);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int waitingHalf(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "emperor_statue.waitingHalf: messageHandler Initialized.");
		if (params == null || params.equals(""))
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.waitingHalf: There were no params passed to the vendor3 message handler.");
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey("winnerList"))
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.waitingHalf: There was no winnerlist key passed in the params.");
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey("listOfWinners"))
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.waitingHalf: There was no listOfWinners key passed in the params.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id vader = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vader");
		if (isIdValid(vader) || exists(vader))
		{
			if (!hasObjVar(vader, "readyForBadge"))
			{
				CustomerServiceLog("holidayEvent", "emperor_statue.waitingHalf: Vader has given out the badges already. No need to continue.");
				messageTo(self, "vaderLeaving", null, 1, false);
				setObjVar(vader, "alreadyLeaving", true);
				return SCRIPT_CONTINUE;
			}
			
			String winnerList = params.getString("winnerList");
			if (winnerList == null || winnerList.equals(""))
			{
				CustomerServiceLog("holidayEvent", "emperor_statue.waitingHalf: Vader cannot speak out to ask the player to step forward because the player name string(s) were not found.");
				return SCRIPT_CONTINUE;
			}
			
			obj_id[] listOfWinners = params.getObjIdArray("listOfWinners");
			if (listOfWinners == null || listOfWinners.length <= 0)
			{
				CustomerServiceLog("holidayEvent", "emperor_statue.waitingHalf: The list of winners was not found. Vader cannot ask the player(s) to step forward.");
				return SCRIPT_CONTINUE;
			}
			
			obj_var_list allMissingPlayers = getObjVarList(vader, "readyForBadge");
			if (allMissingPlayers == null)
			{
				CustomerServiceLog("holidayEvent", "emperor_statue.waitingHalf: Vader was about to talk smack about 1 or more players not bowing but none of the players are actually missing.");
				messageTo(self, "vaderLeaving", null, 1, false);
				setObjVar(vader, "alreadyLeaving", true);
				return SCRIPT_CONTINUE;
			}
			
			Vector miaList = new Vector();
			miaList.setSize(0);
			for (int i = 0; i < allMissingPlayers.getNumItems(); i++)
			{
				testAbortScript();
				obj_var playerPlace = allMissingPlayers.getObjVar(i);
				utils.addElement(miaList, playerPlace.getObjIdData());
			}
			
			if (miaList.size() <= 0)
			{
				CustomerServiceLog("holidayEvent", "emperor_statue.waitingHalf: We tried to get a list of missing players but failed. Sending Vader to shuttle.");
				messageTo(self, "vaderLeaving", null, 1, false);
				setObjVar(vader, "alreadyLeaving", true);
				return SCRIPT_CONTINUE;
			}
			
			String waitingOnList = "";
			if (miaList.size() == 1)
			{
				CustomerServiceLog("holidayEvent", "emperor_statue.waitingHalf: The list length for MIA players is only one player: "+((obj_id)(miaList.get(0))));
				waitingOnList = getFirstName(((obj_id)(miaList.get(0))));
			}
			else
			{
				CustomerServiceLog("holidayEvent", "emperor_statue.waitingHalf: The list length for MIA players is: "+miaList.size());
				for (int i = 0; i < miaList.size(); i++)
				{
					testAbortScript();
					if (i == 0)
					{
						waitingOnList += getFirstName(((obj_id)(miaList.get(i))));
					}
					else if (i == (miaList.size()-1))
					{
						waitingOnList += " and "+getFirstName(((obj_id)(miaList.get(i))));
					}
					else
					{
						waitingOnList += ", "+getFirstName(((obj_id)(miaList.get(i))));
					}
				}
			}
			prose_package pp = prose.getPackage(DARTH_ANNOYED);
			prose.setTO (pp, waitingOnList);
			chat.chat(vader, ((obj_id)(miaList.get(0))), pp);
			
			setObjVar(vader, "miaList", miaList);
			setObjVar(vader, "waitingOnList", waitingOnList);
			messageTo(self, "vendorReplyAnnoyed", null, 10.0f, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int vendorReplyAnnoyed(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "emperor_statue.vendorReplyAnnoyed: messageHandler Initialized.");
		
		obj_id vendor = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vendor");
		if (isIdValid(vendor) || exists(vendor))
		{
			doAnimationAction(vendor, "nervous");
			chat.chat(vendor, VENDOR_REPLY_ANNOYED);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int waitingDone(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "emperor_statue.waitingDone: messageHandler Initialized.");
		if (params == null || params.equals(""))
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.waitingDone: There were no params passed to the vendor3 message handler.");
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey("winnerList"))
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.waitingDone: There was no winnerlist key passed in the params.");
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey("listOfWinners"))
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.waitingDone: There was no listOfWinners key passed in the params.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id vader = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vader");
		if (isIdValid(vader) || exists(vader))
		{
			if (hasObjVar(vader, "alreadyLeaving"))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (!hasObjVar(vader, "readyForBadge"))
			{
				CustomerServiceLog("holidayEvent", "emperor_statue.waitingDone: Vader has given out the badges already. No need to continue.");
				messageTo(self, "vaderLeaving", null, 1, false);
				return SCRIPT_CONTINUE;
			}
			
			if (!hasObjVar(vader, "miaList"))
			{
				CustomerServiceLog("holidayEvent", "emperor_statue.waitingDone: Vader did not receive the MIA list. No need to continue.");
				messageTo(self, "vaderLeaving", null, 1, false);
				return SCRIPT_CONTINUE;
			}
			
			obj_id[] oldMiaList = getObjIdArrayObjVar(vader, "miaList");
			if (oldMiaList == null || oldMiaList.length <= 0)
			{
				CustomerServiceLog("holidayEvent", "emperor_statue.waitingDone: Vader had an old MIA player list but it was invalid or corrupt. Vader is leaving as a result.");
				messageTo(self, "vaderLeaving", null, 1, false);
				return SCRIPT_CONTINUE;
			}
			
			obj_var_list allMissingPlayers = getObjVarList(vader, "readyForBadge");
			if (allMissingPlayers == null)
			{
				CustomerServiceLog("holidayEvent", "emperor_statue.waitingDone: Vader was about to talk smack about 1 or more players not bowing but none of the players are actually missing.");
				messageTo(self, "vaderLeaving", null, 1, false);
				return SCRIPT_CONTINUE;
			}
			
			int latestMiaListLen = allMissingPlayers.getNumItems();
			if (latestMiaListLen != oldMiaList.length)
			{
				Vector lastestMiaList = new Vector();
				lastestMiaList.setSize(0);
				for (int i = 0; i < latestMiaListLen; i++)
				{
					testAbortScript();
					obj_var playerPlace = allMissingPlayers.getObjVar(i);
					utils.addElement(lastestMiaList, playerPlace.getObjIdData());
				}
				
				if (lastestMiaList.size() <= 0)
				{
					CustomerServiceLog("holidayEvent", "emperor_statue.waitingDone: We tried to get a list of missing players but failed. Sending Vader to shuttle.");
					messageTo(self, "vaderLeaving", null, 1, false);
					setObjVar(vader, "alreadyLeaving", true);
					return SCRIPT_CONTINUE;
				}
				
				String waitingOnList = "";
				if (lastestMiaList.size() == 1)
				{
					CustomerServiceLog("holidayEvent", "emperor_statue.waitingDone: The list length for MIA players is only one player: "+((obj_id)(lastestMiaList.get(0))));
					waitingOnList = getFirstName(((obj_id)(lastestMiaList.get(0))));
				}
				else
				{
					CustomerServiceLog("holidayEvent", "emperor_statue.waitingDone: The list length for MIA players is: "+lastestMiaList.size());
					for (int i = 0; i < lastestMiaList.size(); i++)
					{
						testAbortScript();
						if (i == 0)
						{
							waitingOnList += getFirstName(((obj_id)(lastestMiaList.get(i))));
						}
						else if (i == (lastestMiaList.size()-1))
						{
							waitingOnList += " and "+getFirstName(((obj_id)(lastestMiaList.get(i))));
						}
						else
						{
							waitingOnList += ", "+getFirstName(((obj_id)(lastestMiaList.get(i))));
						}
						
						utils.removeScriptVar(((obj_id)(lastestMiaList.get(i))), "emperorsDayBadge");
					}
				}
				
				prose_package pp = prose.getPackage(DARTH_ANNOYED_2_SINGULAR);
				if (lastestMiaList.size() > 1)
				{
					pp = prose.getPackage(DARTH_ANNOYED_2_PLURAL);
				}
				prose.setTO (pp, waitingOnList);
				chat.chat(vader, ((obj_id)(lastestMiaList.get(0))), pp);
			}
			else
			{
				String waitingOnList = getStringObjVar(vader, "waitingOnList");
				prose_package pp = prose.getPackage(DARTH_ANNOYED_2_SINGULAR);
				if (oldMiaList.length > 1)
				{
					pp = prose.getPackage(DARTH_ANNOYED_2_PLURAL);
				}
				prose.setTO (pp, waitingOnList);
				chat.chat(vader, oldMiaList[0], pp);
				
				for (int i = 0; i < oldMiaList.length; i++)
				{
					testAbortScript();
					if (isIdValid(oldMiaList[i]) && exists(oldMiaList[i]))
					{
						utils.removeScriptVar(oldMiaList[i], "emperorsDayBadge");
					}
				}
			}
			messageTo(self, "vendorReplyWaiting", null, 10.0f, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int vendorReplyWaiting(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "emperor_statue.vendorReplyWaiting: messageHandler Initialized.");
		
		obj_id vendor = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vendor");
		if (isIdValid(vendor) || exists(vendor))
		{
			doAnimationAction(vendor, "apologize");
			chat.chat(vendor, VENDOR_REPLY_ANNOYED_2);
			messageTo(self, "vaderLeaving", null, 10.0f, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean playTieFighterPair(obj_id self, obj_id playOnObject) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "emperor_statue.playTieFighterPair: Function initialized.");
		
		if (!isValidId(self) || !exists(self))
		{
			return false;
		}
		if (!isValidId(playOnObject) || !exists(playOnObject))
		{
			return false;
		}
		
		CustomerServiceLog("holidayEvent", "emperor_statue.playTieFighterPair: playing.");
		location here = getLocation(playOnObject);
		playClientEffectLoc(getPlayerCreaturesInRange(here, 200.0f), holiday.IMPERIAL_FLYBY_PARTICLE_01, here, 1.0f);
		return true;
	}
	
	
	public boolean playTieFighterGroup(obj_id self, obj_id playOnObject) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "emperor_statue.playTieFighterGroup: Function initialized.");
		
		if (!isValidId(self) || !exists(self))
		{
			return false;
		}
		if (!isValidId(playOnObject) || !exists(playOnObject))
		{
			return false;
		}
		
		CustomerServiceLog("holidayEvent", "emperor_statue.playTieFighterGroup: playing.");
		location here = getLocation(playOnObject);
		playClientEffectLoc(getPlayerCreaturesInRange(here, 200.0f), holiday.IMPERIAL_FLYBY_PARTICLE_02, here, 1.0f);
		return true;
	}
	
	
	public boolean createLambdaDropship(obj_id self, location loc) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "emperor_statue.createLambdaDropship: Init.");
		if (loc == null)
		{
			return false;
		}
		
		if (!loc.area.equals(getCurrentSceneName()))
		{
			return false;
		}
		
		CustomerServiceLog("holidayEvent", "emperor_statue.createLambdaDropship: Creating shuttle at location: "+loc);
		obj_id lambda = create.object("object/creature/npc/theme_park/lambda_shuttle.iff", loc);
		if (!isIdValid(lambda))
		{
			return false;
		}
		
		CustomerServiceLog("holidayEvent", "emperor_statue.createLambdaDropship: Lambda OID: "+lambda);
		
		setYaw(lambda, LAMBDA_YAW);
		
		CustomerServiceLog("holidayEvent", "emperor_statue.createLambdaDropship: Lambda Yaw: "+getYaw(lambda));
		
		utils.setScriptVar(self, "lambdaShuttle", lambda);
		utils.setScriptVar(lambda, "statue", self);
		attachScript(lambda, "systems.spawning.dropship.emperorsday_lambda");
		
		return true;
	}
	
}
