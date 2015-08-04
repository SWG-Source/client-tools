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


public class yoda_fountain extends script.base_script
{
	public yoda_fountain()
	{
	}
	public static final String MUSIC_BATTLE_HEROES = "object/soundobject/soundobject_battle_heroes.iff";
	public static final String MUSIC_HAN_LEIA = "object/soundobject/soundobject_empire_day_romance_b.iff";
	
	public static final String EMP_DAY = "event/emperors_day";
	
	public static final string_id VENDOR_1 = new string_id(EMP_DAY, "reb_vendor_1");
	
	public static final string_id LEIA_1 = new string_id(EMP_DAY, "leia_1");
	
	public static final string_id VENDOR_2 = new string_id(EMP_DAY, "reb_vendor_2");
	
	public static final string_id LEIA_2 = new string_id(EMP_DAY, "leia_2");
	
	public static final string_id LEIA_3 = new string_id(EMP_DAY, "leia_3");
	
	public static final string_id LEIA_4 = new string_id(EMP_DAY, "leia_4");
	
	public static final string_id LEIA_5 = new string_id(EMP_DAY, "leia_5");
	
	public static final string_id LEIA_6 = new string_id(EMP_DAY, "leia_6_string");
	
	public static final string_id LEIA_6_ALT = new string_id(EMP_DAY, "leia_6_alt");
	
	public static final string_id VENDOR_3 = new string_id(EMP_DAY, "reb_vendor_3_string");
	
	public static final string_id LEIA_7 = new string_id(EMP_DAY, "leia_7");
	
	public static final string_id LEIA_ANNOYED = new string_id(EMP_DAY, "leia_annoyed_string");
	public static final string_id LEIA_ANNOYED_SINGULAR = new string_id(EMP_DAY, "leia_annoyed_string_singular");
	public static final string_id LEIA_ANNOYED_PLURAL = new string_id(EMP_DAY, "leia_annoyed_string_plural");
	
	public static final string_id VENDOR_REPLY_ANNOYED = new string_id(EMP_DAY, "reb_vendor_annoyed");
	
	public static final string_id LEIA_ANNOYED_2 = new string_id(EMP_DAY, "leia_annoyed_2");
	public static final string_id LEIA_ANNOYED_2_SINGULAR = new string_id(EMP_DAY, "leia_annoyed_2_string_singular");
	public static final string_id LEIA_ANNOYED_2_PLURAL = new string_id(EMP_DAY, "leia_annoyed_2_string_plural");
	
	public static final string_id VENDOR_REPLY_ANNOYED_2 = new string_id(EMP_DAY, "reb_vendor_annoyed_2");
	
	public static final string_id HAN_BYE = new string_id(EMP_DAY, "han_bye");
	
	public static final string_id HAN_ANGRY = new string_id(EMP_DAY, "han_angry");
	
	public static final string_id HAN_HAPPY = new string_id(EMP_DAY, "han_happy");
	
	public static final string_id TREE_USE = new string_id(EMP_DAY, "yoda_fountain_use");
	public static final string_id NOT_OLD_ENOUGH = new string_id(EMP_DAY, "rebel_not_old_enough");
	public static final string_id GIFT_GRANTED = new string_id(EMP_DAY, "gift_granted_rebel");
	public static final string_id WINNER_HELP = new string_id(EMP_DAY, "winner_help_rebel");
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "yoda_fountain.OnInitialize: Fountain is initializing.");
		
		if (!utils.hasScriptVar(self, "musicObject"))
		{
			location soundLoc = getLocation(self);
			obj_id soundObject = createObject(MUSIC_HAN_LEIA, soundLoc);
			
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
			obj_id soundObject = createObject(MUSIC_HAN_LEIA, soundLoc);
			
			utils.setScriptVar(self, "musicObject", soundObject);
		}
		
		CustomerServiceLog("holidayEvent", "yoda_fountain.OnInitialize: Preparing Parade.");
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
		
		String statusString = holiday.getEmpireDayEligibility(player, holiday.REBEL_PLAYER);
		if (statusString != null || !statusString.equals(""))
		{
			names[idx] = "remembrance_day_player_status";
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
		
		if (!factions.isRebel(player))
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
		
		if (!factions.isRebel(player))
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
			holiday.grantEmpireDayGift(player, holiday.REBEL_PLAYER);
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
		
		obj_id falcon = utils.getObjIdScriptVar(self, "falcon");
		
		if ((isIdValid(falcon)) && exists(falcon))
		{
			messageTo(falcon, "takeOff", null, 20.0f, false);
			utils.removeScriptVar(self, "falcon");
		}
		
		if (utils.hasScriptVar(self, "leia"))
		{
			Vector npc = utils.getResizeableObjIdArrayScriptVar(self, "leia");
			for (int i=0; i<npc.size(); i++)
			{
				testAbortScript();
				if (isIdValid(((obj_id)(npc.get(i)))) && exists(((obj_id)(npc.get(i)))))
				{
					destroyObject(((obj_id)(npc.get(i))));
				}
			}
			utils.removeScriptVar(self, "leia");
		}
		if (utils.hasScriptVar(self, "solo"))
		{
			Vector npc = utils.getResizeableObjIdArrayScriptVar(self, "solo");
			for (int i=0; i<npc.size(); i++)
			{
				testAbortScript();
				if (isIdValid(((obj_id)(npc.get(i)))) && exists(((obj_id)(npc.get(i)))))
				{
					destroyObject(((obj_id)(npc.get(i))));
				}
			}
			utils.removeScriptVar(self, "solo");
		}
		if (utils.hasScriptVar(self, "chewbacca"))
		{
			Vector npc = utils.getResizeableObjIdArrayScriptVar(self, "chewbacca");
			for (int i=0; i<npc.size(); i++)
			{
				testAbortScript();
				if (isIdValid(((obj_id)(npc.get(i)))) && exists(((obj_id)(npc.get(i)))))
				{
					destroyObject(((obj_id)(npc.get(i))));
				}
			}
			utils.removeScriptVar(self, "chewbacca");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnHearSpeech(obj_id self, obj_id speaker, String text) throws InterruptedException
	{
		if (!isGod(speaker))
		{
			return SCRIPT_CONTINUE;
		}
		else if (text.equals("startParade"))
		{
			obj_id oldSoundObject = utils.getObjIdScriptVar(self, "musicObject");
			
			if ((isIdValid(oldSoundObject)) && exists(oldSoundObject))
			{
				destroyObject(oldSoundObject);
			}
			
			sendSystemMessage(speaker, "starting parade", "");
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
			
			sendSystemMessage(speaker, "Destroying parade", "");
			messageTo(self, "destroyParade", null, 10.0f, false);
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
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int prepareParade(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "yoda_fountain.prepareParade: messageHandler initialized.");
		
		if (!utils.hasScriptVar(self, "paradeRunning"))
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.prepareParade: Parade is not running.");
			
			utils.setScriptVar(self, "paradeRunning", 1);
			obj_id oldSoundObject = utils.getObjIdScriptVar(self, "musicObject");
			
			if ((isIdValid(oldSoundObject)) && exists(oldSoundObject))
			{
				CustomerServiceLog("holidayEvent", "yoda_fountain.prepareParade: Destroying Sound Object.");
				destroyObject(oldSoundObject);
			}
			
			location loc = getLocation(self);
			obj_id[] objects = getObjectsInRange(loc, holiday.OBJECT_NEAR_CHECK_RANGE_100M);
			
			for (int i = 0; i < objects.length; i++)
			{
				testAbortScript();
				if (isIdValid(objects[i]))
				{
					if (hasObjVar(objects[i], "soldier"))
					{
						int soldierNumber = getIntObjVar(objects[i], "soldier");
						obj_id pathPoint = holiday.getEmpireDayWaypointObjectObjId(self, "pathsoldier"+soldierNumber, holiday.OBJECT_NEAR_CHECK_RANGE_100M);
						location pathLoc = getLocation(pathPoint);
						
						pathTo (objects[i], pathLoc);
					}
				}
			}
			messageTo(self, "parade", null, 10.0f, false);
		}
		else
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.prepareParade: Failed to start because the statue says the parade is running.");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int parade(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "yoda_fountain.parade: messageHandler initialized.");
		
		obj_id vendor = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vendor");
		if (isIdValid(vendor) && exists(vendor))
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.parade: removing conversation script from vendor so players cannot interrupt the ceremony.");
			detachScript(vendor, "conversation.rebel_emperorsday_vendor");
		}
		
		if (!utils.hasScriptVar(self, "musicObject"))
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.parade: Creating new music object.");
			
			location soundLoc = getLocation(self);
			obj_id soundObject = createObject(MUSIC_BATTLE_HEROES, soundLoc);
			
			utils.setScriptVar(self, "musicObject", soundObject);
		}
		else if (utils.hasScriptVar(self, "musicObject"))
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.parade: Getting existing music object.");
			
			obj_id oldSoundObject = utils.getObjIdScriptVar(self, "musicObject");
			
			if ((isIdValid(oldSoundObject)) && exists(oldSoundObject))
			{
				destroyObject(oldSoundObject);
			}
			
			location soundLoc = getLocation(self);
			obj_id soundObject = createObject(MUSIC_BATTLE_HEROES, soundLoc);
			
			utils.setScriptVar(self, "musicObject", soundObject);
		}
		
		CustomerServiceLog("holidayEvent", "yoda_fountain.parade: Getting Falcon waypoint data in preparation for shuttle landing.");
		
		obj_id spawnPoint = holiday.getEmpireDayWaypointObjectObjId(self, "shuttleLanding", holiday.OBJECT_NEAR_CHECK_RANGE_100M);
		location spawnLoc = getLocation(spawnPoint);
		boolean falconCreation = createFalconDropship(self, spawnLoc);
		CustomerServiceLog("holidayEvent", "yoda_fountain.parade: Falcon spawnPoint: "+spawnPoint);
		CustomerServiceLog("holidayEvent", "yoda_fountain.parade: Falcon spawnPoint spawnLoc: "+spawnLoc);
		CustomerServiceLog("holidayEvent", "yoda_fountain.parade: Falcon was created: "+falconCreation);
		
		if (!falconCreation)
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.parade: The Falcon failed to spawn!! Notify design immediately.");
			return SCRIPT_CONTINUE;
		}
		CustomerServiceLog("holidayEvent", "yoda_fountain.parade: Sending Message to Leia, Han and Chewie to spawn.");
		
		messageTo(self, "spawnLeia", null, 40.0f, false);
		
		messageTo(self, "spawnChewie", null, 50.0f, false);
		messageTo(self, "spawnHan", null, 55.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int spawnLeia(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "yoda_fountain.spawnLeia: Spawning Leia at Falcon ramp.");
		
		obj_id spawnPoint = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"falcon_ramp");
		if (!isIdValid(spawnPoint) || !exists(spawnPoint))
		{
			return SCRIPT_CONTINUE;
		}
		
		location spawnLoc = getLocation(spawnPoint);
		if (spawnLoc == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id leia = create.object("rebel_emperorsday_leia", spawnLoc);
		if (!isValidId(leia) || !exists(leia))
		{
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("holidayEvent", "yoda_fountain.spawnLeia: Leia spawned at Falcon ramp.");
		
		setYaw(leia, -180.0f);
		setObjVar(leia, performance.NPC_ENTERTAINMENT_NO_ENTERTAIN, 1);
		setObjVar(leia, "leia", 1);
		setObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"leia", leia);
		trial.markAsTempObject(leia, true);
		CustomerServiceLog("holidayEvent", "yoda_fountain.spawnLeia: Moving Leia to her lecture spot after a delay.");
		
		messageTo(self, "moveLeia", null, 5.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int moveLeia(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "yoda_fountain.moveLeia: Moving Leia to her lecture spot now.");
		
		obj_id leia = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"leia");
		if (!isIdValid(leia) || !exists(leia))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id leiaWaypoint = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"leia_waypoint");
		if (!isIdValid(leiaWaypoint) || !exists(leiaWaypoint))
		{
			return SCRIPT_CONTINUE;
		}
		
		location pathLoc = getLocation(leiaWaypoint);
		if (pathLoc == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		pathTo(leia, pathLoc);
		return SCRIPT_CONTINUE;
	}
	
	
	public int spawnChewie(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "yoda_fountain.spawnChewie: Spawning Chewie at Falcon ramp.");
		
		obj_id spawnPoint = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"falcon_ramp");
		if (!isIdValid(spawnPoint) || !exists(spawnPoint))
		{
			return SCRIPT_CONTINUE;
		}
		
		location spawnLoc = getLocation(spawnPoint);
		if (spawnLoc == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id chewie = create.object("rebel_emperorsday_chewbacca", spawnLoc);
		if (!isIdValid(chewie) || !exists(chewie))
		{
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("holidayEvent", "yoda_fountain.spawnChewie: Chewie spawned at Falcon ramp.");
		
		setYaw(chewie, -180.0f);
		setObjVar(chewie, performance.NPC_ENTERTAINMENT_NO_ENTERTAIN, 1);
		setObjVar(chewie, "chewie", 1);
		setObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"chewie", chewie);
		trial.markAsTempObject(chewie, true);
		CustomerServiceLog("holidayEvent", "yoda_fountain.spawnChewie: Moving Chewie to his spot after a delay.");
		
		messageTo(self, "moveChewie", null, 3.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int moveChewie(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "yoda_fountain.moveChewie: Moving Chewie to his spot now.");
		
		obj_id chewie = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"chewie");
		if (!isIdValid(chewie) || !exists(chewie))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id chewieWaypoint = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"chewie_waypoint");
		if (!isIdValid(chewieWaypoint) || !exists(chewieWaypoint))
		{
			return SCRIPT_CONTINUE;
		}
		
		location pathLoc = getLocation(chewieWaypoint);
		if (pathLoc == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		pathTo(chewie, pathLoc);
		return SCRIPT_CONTINUE;
	}
	
	
	public int spawnHan(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "yoda_fountain.spawnHan: Spawning Han at Falcon ramp.");
		
		obj_id spawnPoint = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"falcon_ramp");
		if (!isIdValid(spawnPoint) || !exists(spawnPoint))
		{
			return SCRIPT_CONTINUE;
		}
		
		location spawnLoc = getLocation(spawnPoint);
		if (spawnLoc == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id han = create.object("rebel_emperorsday_han_solo", spawnLoc);
		if (!isIdValid(han) || !exists(han))
		{
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("holidayEvent", "yoda_fountain.spawnHan: Han spawned at Falcon ramp.");
		
		setObjVar(han, performance.NPC_ENTERTAINMENT_NO_ENTERTAIN, 1);
		setObjVar(han, "han", 1);
		setObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"han", han);
		trial.markAsTempObject(han, true);
		CustomerServiceLog("holidayEvent", "yoda_fountain.spawnHan: Moving Han to his spot after a delay.");
		
		messageTo(self, "moveHan", null, 3.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int moveHan(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "yoda_fountain.moveChewie: Moving Chewie to his spot now.");
		
		obj_id han = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"han");
		if (!isIdValid(han) || !exists(han))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id hanWaypoint = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"han_waypoint");
		if (!isIdValid(hanWaypoint) || !exists(hanWaypoint))
		{
			return SCRIPT_CONTINUE;
		}
		
		location pathLoc = getLocation(hanWaypoint);
		if (pathLoc == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		pathTo(han, pathLoc);
		return SCRIPT_CONTINUE;
	}
	
	
	public int startConversation(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (utils.hasScriptVar(self, "soldier"))
		{
			Vector npc = utils.getResizeableObjIdArrayScriptVar(self, "soldier");
			for (int i=0; i<npc.size(); i++)
			{
				testAbortScript();
				if (isIdValid(((obj_id)(npc.get(i)))) && exists(((obj_id)(npc.get(i)))))
				{
					doAnimationAction(((obj_id)(npc.get(i))), "salute1");
				}
			}
		}
		
		obj_id vendor = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vendor");
		if (!isIdValid(vendor) || !exists(vendor))
		{
			return SCRIPT_CONTINUE;
		}
		
		doAnimationAction(vendor, "bow2");
		chat.chat(vendor, VENDOR_1);
		messageTo (self, "leia1", null, 5.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int leia1(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id leia = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"leia");
		if (!isIdValid(leia) || !exists(leia))
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat(leia, LEIA_1);
		messageTo (self, "vendor2", null, 10.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int vendor2(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id vendor = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vendor");
		if (!isIdValid(vendor) || !exists(vendor))
		{
			return SCRIPT_CONTINUE;
		}
		
		doAnimationAction(vendor, "nod_head_multiple");
		chat.chat(vendor, VENDOR_2);
		messageTo (self, "leia2", null, 5.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int leia2(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id leia = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"leia");
		if (!isIdValid(leia) || !exists(leia))
		{
			return SCRIPT_CONTINUE;
		}
		
		doAnimationAction(leia, "nod_head_once");
		chat.chat(leia, LEIA_2);
		messageTo (self, "leia3", null, 10.0f, false);
		setYaw(leia, 0.0f);
		return SCRIPT_CONTINUE;
	}
	
	
	public int leia3(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id leia = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"leia");
		if (!isIdValid(leia) || !exists(leia))
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat(leia, LEIA_3);
		messageTo (self, "leia4", null, 15.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int leia4(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id leia = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"leia");
		if (!isIdValid(leia) || !exists(leia))
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat(leia, LEIA_4);
		messageTo (self, "leia5", null, 15.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int leia5(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id leia = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"leia");
		if (!isIdValid(leia) || !exists(leia))
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat(leia, LEIA_5);
		messageTo (self, "leia6", null, 15.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int leia6(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "yoda_fountain.leia6: messageHandler Initialized.");
		
		obj_id leia = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"leia");
		if (!isIdValid(leia) || !exists(leia))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] playersInRange = getAllPlayers(getLocation(self), holiday.OBJECT_NEAR_CHECK_RANGE_100M);
		if (playersInRange == null || playersInRange.length <= 0)
		{
			
			CustomerServiceLog("holidayEvent", "yoda_fountain.leia6: There were no players in the area that were eligible. Leia is leaving due to Case 1.");
			
			chat.chat(leia, LEIA_6_ALT);
			messageTo(self, "leiaLeaving", null, 15.0f, false);
			return SCRIPT_CONTINUE;
		}
		
		Vector rebelPlayers = new Vector();
		rebelPlayers.setSize(0);
		for (int z = 0; z < playersInRange.length; z++)
		{
			testAbortScript();
			if (!isIdValid(playersInRange[z]) || !exists(playersInRange[z]))
			{
				continue;
			}
			
			if (holiday.isEmpireDayPlayerEligible(playersInRange[z], holiday.REBEL_PLAYER))
			{
				utils.addElement(rebelPlayers, playersInRange[z]);
			}
			
		}
		
		if (rebelPlayers == null || rebelPlayers.size() <= 0)
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.leia6: There were players in the area but none that were eligible. Leia is leaving.");
			
			chat.chat(leia, LEIA_6_ALT);
			messageTo(self, "leiaLeaving", null, 15.0f, false);
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] listOfWinners = holiday.getEmpireDayWinningPlayers(self, rebelPlayers);
		if (listOfWinners == null || listOfWinners.length <= 0)
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.leia6: A list of eligible players were sent to the getWinningPlayers function but it came back corrupted or null.");
			
			chat.chat(leia, LEIA_6_ALT);
			messageTo(self, "leiaLeaving", null, 15.0f, false);
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("holidayEvent", "yoda_fountain.leia6: A list of eligible players has been received and will now be announced. The list length is: "+listOfWinners.length);
		
		String winnerList = "";
		int winnerCount = listOfWinners.length;
		
		if (winnerCount == 1)
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.leia6: The list length is only one player: "+listOfWinners[0]);
			
			prose_package pp = prose.getPackage(LEIA_6);
			winnerList = getFirstName(listOfWinners[0]);
			prose.setTO (pp, winnerList);
			chat.chat(leia, listOfWinners[0], pp);
			sendSystemMessage(listOfWinners[0], WINNER_HELP);
			
			params.put("winnerList", winnerList);
			params.put("listOfWinners", listOfWinners);
			
			messageTo(self, "vendor3", params, 10.0f, false);
			utils.setScriptVar(listOfWinners[0], "emperorsDayBadge", 1);
			setObjVar(leia, "listOfWinners", listOfWinners);
			
			setObjVar(leia, "readyForBadge.winner_1", listOfWinners[0]);
			messageTo(self, "waitingHalf", params, 45.0f, false);
			messageTo(self, "waitingDone", params, 70.0f, false);
		}
		else
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.leia6: The list length is: "+listOfWinners.length);
			
			prose_package pp = prose.getPackage(LEIA_6);
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
				setObjVar(leia, "readyForBadge.winner_"+(i+1), listOfWinners[i]);
			}
			
			if (winnerList == null || winnerList.equals(""))
			{
				CustomerServiceLog("holidayEvent", "yoda_fountain.leia6: The player names to be called somehow got corrupted. Sending Leia to falcon.");
				chat.chat(leia, LEIA_6_ALT);
				messageTo(self, "leiaLeaving", null, 15.0f, false);
				return SCRIPT_CONTINUE;
			}
			
			prose.setTO(pp, winnerList);
			chat.chat(leia, listOfWinners[0], pp);
			
			CustomerServiceLog("holidayEvent", "yoda_fountain.leia6: setting winnerList on Leia.");
			setObjVar(leia, "listOfWinners", listOfWinners);
			
			params.put("winnerList", winnerList);
			params.put("listOfWinners", listOfWinners);
			messageTo(self, "vendor3", params, 10.0f, false);
			messageTo(self, "waitingHalf", params, 45.0f, false);
			messageTo(self, "waitingDone", params, 70.0f, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int hanAngry(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id rudePlayer = params.getObjId("rudePlayer");
		if (!isValidId(rudePlayer) || !exists(rudePlayer))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id han = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"han");
		if (!isIdValid(han) || !exists(han))
		{
			return SCRIPT_CONTINUE;
		}
		
		prose_package pp = prose.getPackage(HAN_ANGRY);
		prose.setTT (pp, rudePlayer);
		chat.chat(han, rudePlayer, pp);
		doAnimationAction(han, "pound_fist_palm");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int hanHappy(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id han = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"han");
		if (isIdValid(han) && exists(han))
		{
			chat.chat(han, HAN_HAPPY);
			doAnimationAction(han, "celebrate");
		}
		
		obj_id chewie = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"chewie");
		if (isIdValid(chewie) && exists(chewie))
		{
			doAnimationAction(chewie, "celebrate");
		}
		
		if (utils.hasScriptVar(self, "soldier"))
		{
			Vector npc = utils.getResizeableObjIdArrayScriptVar(self, "soldier");
			for (int i=0; i<npc.size(); i++)
			{
				testAbortScript();
				if (isIdValid(((obj_id)(npc.get(i)))) && exists(((obj_id)(npc.get(i)))))
				{
					doAnimationAction(((obj_id)(npc.get(i))), "celebrate");
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int vendor3(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "yoda_fountain.vendor3: messageHandler Initialized.");
		
		if (params == null || params.equals(""))
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.vendor3: There were no params passed to the vendor3 message handler.");
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey("winnerList"))
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.vendor3: There was no winnerlist key passed in the params.");
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey("listOfWinners"))
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.vendor3: There was no listOfWinners key passed in the params.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id leia = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"leia");
		if (isIdValid(leia) || exists(leia))
		{
			if (!hasObjVar(leia, "readyForBadge"))
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		obj_id vendor = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vendor");
		if (!isIdValid(vendor) || !exists(vendor))
		{
			return SCRIPT_CONTINUE;
		}
		
		String winnerList = params.getString("winnerList");
		if (winnerList == null || winnerList.equals(""))
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.vendor3: The vendor cannot speak out to ask the player to step forward because the player name string(s) were not found.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] listOfWinners = params.getObjIdArray("listOfWinners");
		if (listOfWinners == null || listOfWinners.length <= 0)
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.vendor3: The list of winners was not found. The vendor cannot ask the player(s) to step forward.");
			return SCRIPT_CONTINUE;
		}
		
		prose_package pp = prose.getPackage(VENDOR_3);
		prose.setTO (pp, winnerList);
		chat.chat(vendor, listOfWinners[0], pp);
		return SCRIPT_CONTINUE;
	}
	
	
	public int waitingHalf(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "yoda_fountain.waitingHalf: messageHandler Initialized.");
		
		if (params == null || params.equals(""))
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.waitingHalf: There were no params passed to the vendor3 message handler.");
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey("winnerList"))
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.waitingHalf: There was no winnerlist key passed in the params.");
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey("listOfWinners"))
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.waitingHalf: There was no listOfWinners key passed in the params.");
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("holidayEvent", "yoda_fountain.waitingHalf: All ceremony params found.");
		
		obj_id leia = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"leia");
		if (!isIdValid(leia) || !exists(leia))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(leia, "readyForBadge"))
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.waitingHalf: Leia has given out the badges already. No need to continue.");
			messageTo(self, "leiaLeaving", null, 1, false);
			setObjVar(leia, "alreadyLeaving", true);
			return SCRIPT_CONTINUE;
		}
		
		String winnerList = params.getString("winnerList");
		if (winnerList == null || winnerList.equals(""))
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.waitingHalf: Leia cannot speak out to ask the player to step forward because the player name string(s) were not found.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] listOfWinners = params.getObjIdArray("listOfWinners");
		if (listOfWinners == null || listOfWinners.length <= 0)
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.waitingHalf: The list of winners was not found. Leia cannot ask the player(s) to step forward.");
			return SCRIPT_CONTINUE;
		}
		
		obj_var_list allMissingPlayers = getObjVarList(leia, "readyForBadge");
		if (allMissingPlayers == null)
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.waitingHalf: Leia was about to talk smack about 1 or more players not bowing but none of the players are actually missing.");
			messageTo(self, "leiaLeaving", null, 1, false);
			setObjVar(leia, "alreadyLeaving", true);
			return SCRIPT_CONTINUE;
		}
		CustomerServiceLog("holidayEvent", "yoda_fountain.waitingHalf: Missing player data attained.");
		
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
			CustomerServiceLog("holidayEvent", "yoda_fountain.waitingHalf: We tried to get a list of missing players but failed. Sending Leia to Falcon.");
			messageTo(self, "leiaLeaving", null, 1, false);
			setObjVar(leia, "alreadyLeaving", true);
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("holidayEvent", "yoda_fountain.waitingHalf: miaList.length: "+miaList.size());
		
		String waitingOnList = "";
		if (miaList.size() == 1)
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.waitingHalf: The list length for MIA players is only one player: "+((obj_id)(miaList.get(0))));
			waitingOnList = getFirstName(((obj_id)(miaList.get(0))));
		}
		else
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.waitingHalf: The list length for MIA players is: "+((obj_id)(miaList.get(0))));
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
		
		CustomerServiceLog("holidayEvent", "yoda_fountain.waitingHalf: calling out missing players");
		prose_package pp = prose.getPackage(LEIA_ANNOYED_SINGULAR);
		if (miaList.size() > 1)
		{
			pp = prose.getPackage(LEIA_ANNOYED_PLURAL);
		}
		prose.setTO (pp, waitingOnList);
		chat.chat(leia, ((obj_id)(miaList.get(0))), pp);
		
		setObjVar(leia, "miaList", miaList);
		setObjVar(leia, "waitingOnList", waitingOnList);
		messageTo(self, "vendorReplyAnnoyed", null, 10.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int vendorReplyAnnoyed(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id vendor = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vendor");
		if (!isIdValid(vendor) || !exists(vendor))
		{
			return SCRIPT_CONTINUE;
		}
		
		doAnimationAction(vendor, "nervous");
		chat.chat(vendor, VENDOR_REPLY_ANNOYED);
		return SCRIPT_CONTINUE;
	}
	
	
	public int waitingDone(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "yoda_fountain.waitingDone: messageHandler Initialized.");
		if (params == null || params.equals(""))
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.waitingDone: There were no params passed to the vendor3 message handler.");
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey("winnerList"))
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.waitingDone: There was no winnerlist key passed in the params.");
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey("listOfWinners"))
		{
			CustomerServiceLog("holidayEvent", "yoda_fountain.waitingDone: There was no listOfWinners key passed in the params.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id leia = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"leia");
		if (!isIdValid(leia) || !exists(leia))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(leia, "alreadyLeaving"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(leia, "readyForBadge"))
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.waitingDone: Leia has given out the badges already. No need to continue.");
			messageTo(self, "leiaLeaving", null, 1, false);
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(leia, "miaList"))
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.waitingDone: Leia did not receive the MIA list. No need to continue.");
			messageTo(self, "leiaLeaving", null, 1, false);
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] oldMiaList = getObjIdArrayObjVar(leia, "miaList");
		if (oldMiaList == null || oldMiaList.length <= 0)
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.waitingDone: Leia had an old MIA player list but it was invalid or corrupt. Leia is leaving as a result.");
			messageTo(self, "leiaLeaving", null, 1, false);
			return SCRIPT_CONTINUE;
		}
		
		obj_var_list allMissingPlayers = getObjVarList(leia, "readyForBadge");
		if (allMissingPlayers == null)
		{
			CustomerServiceLog("holidayEvent", "emperor_statue.waitingDone: Leia was about to talk smack about 1 or more players not bowing but none of the players are actually missing.");
			messageTo(self, "leiaLeaving", null, 1, false);
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
				CustomerServiceLog("holidayEvent", "emperor_statue.waitingDone: We tried to get a list of missing players but failed. Sending Leia to Falcon.");
				messageTo(self, "leiaLeaving", null, 1, false);
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
			
			prose_package pp = prose.getPackage(LEIA_ANNOYED_2_SINGULAR);
			if (lastestMiaList.size() > 1)
			{
				pp = prose.getPackage(LEIA_ANNOYED_2_PLURAL);
			}
			prose.setTO (pp, waitingOnList);
			chat.chat(leia, ((obj_id)(lastestMiaList.get(0))), pp);
		}
		else
		{
			String waitingOnList = getStringObjVar(leia, "waitingOnList");
			prose_package pp = prose.getPackage(LEIA_ANNOYED_2_SINGULAR);
			if (oldMiaList.length > 1)
			{
				pp = prose.getPackage(LEIA_ANNOYED_2_PLURAL);
			}
			prose.setTO (pp, waitingOnList);
			chat.chat(leia, oldMiaList[0], pp);
			
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
		return SCRIPT_CONTINUE;
	}
	
	
	public int vendorReplyWaiting(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id vendor = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vendor");
		if (!isIdValid(vendor) || !exists(vendor))
		{
			return SCRIPT_CONTINUE;
		}
		
		doAnimationAction(vendor, "apologize");
		chat.chat(vendor, VENDOR_REPLY_ANNOYED_2);
		messageTo(self, "leiaLeaving", null, 10.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int launchRandomFirework(obj_id self, dictionary params) throws InterruptedException
	{
		location statue = getLocation(self);
		location here = (location)statue.clone();
		here.z = statue.z-30;
		
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
	
	
	public int leiaLeaving(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (utils.hasScriptVar(self, "soldier"))
		{
			Vector npc = utils.getResizeableObjIdArrayScriptVar(self, "soldier");
			for (int i=0; i<npc.size(); i++)
			{
				testAbortScript();
				if (isIdValid(((obj_id)(npc.get(i)))) && exists(((obj_id)(npc.get(i)))))
				{
					doAnimationAction(((obj_id)(npc.get(i))), "salute1");
				}
			}
		}
		
		obj_id vendor = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vendor");
		if (isIdValid(vendor) && exists(vendor))
		{
			doAnimationAction(vendor, "bow2");
		}
		
		obj_id leia = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"leia");
		if (isIdValid(leia) && exists(leia))
		{
			chat.chat(leia, LEIA_7);
			messageTo(self, "finalMarch", null, 5.0f, false);
		}
		
		if (!hasObjVar(leia, "readyForBadge"))
		{
			holiday.playEmpireDayFireWorksAndFlyBys(self, holiday.REBEL_PLAYER);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int finalMarch(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id falconRamp = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"falcon_ramp");
		if (!isIdValid(falconRamp) || !exists(falconRamp))
		{
			return SCRIPT_CONTINUE;
		}
		
		location falconRampLoc = getLocation(falconRamp);
		if (falconRampLoc == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id chewie = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"chewie");
		if (isIdValid(chewie) && exists(chewie))
		{
			pathTo (chewie, falconRampLoc);
		}
		
		obj_id han = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"han");
		if (isIdValid(han) && exists(han))
		{
			pathTo (han, falconRampLoc);
		}
		
		obj_id leia = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"leia");
		if (isIdValid(leia) && exists(leia))
		{
			pathTo (leia, falconRampLoc);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int destroyParade(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id vendor = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"vendor");
		if (!isIdValid(vendor) || !exists(vendor))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasScript(vendor, "conversation.rebel_emperorsday_vendor"))
		{
			attachScript(vendor, "conversation.rebel_emperorsday_vendor");
		}
		
		obj_id falcon = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"falcon");
		if (isIdValid(falcon) && exists(falcon))
		{
			messageTo(falcon, "takeOff", null, 20.0f, false);
			utils.removeScriptVar(self, "falcon");
		}
		
		obj_id leia = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"leia");
		if (isIdValid(leia) && exists(leia))
		{
			destroyObject(leia);
			utils.removeScriptVar(self, "leia");
		}
		
		obj_id han = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"han");
		if (isIdValid(han) && exists(han))
		{
			destroyObject(han);
			utils.removeScriptVar(self, "han");
		}
		
		obj_id chewie = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"chewie");
		if (isIdValid(chewie) && exists(chewie))
		{
			destroyObject(chewie);
			utils.removeScriptVar(self, "chewie");
		}
		
		location loc = getLocation(self);
		obj_id[] objects = getObjectsInRange(loc, holiday.OBJECT_NEAR_CHECK_RANGE_100M);
		
		for (int i = 0; i < objects.length; i++)
		{
			testAbortScript();
			if (isIdValid(objects[i]))
			{
				if (hasObjVar(objects[i], "soldier"))
				{
					int soldierNumber = getIntObjVar(objects[i], "soldier");
					obj_id pathPoint = holiday.getEmpireDayWaypointObjectObjId(self, "spawnsoldier"+soldierNumber, holiday.OBJECT_NEAR_CHECK_RANGE_100M);
					location pathLoc = getLocation(pathPoint);
					
					pathTo (objects[i], pathLoc);
				}
			}
		}
		
		messageTo(self, "normalMusic", null, 15.0f, false);
		
		messageTo(self, "prepareParade", null, holiday.EMPIRE_DAY_SECONDS_TO_START, false);
		
		setObjVar(self, "ceremonyTime", getCalendarTime() + holiday.EMPIRE_DAY_SECONDS_TO_START);
		
		CustomerServiceLog("holidayEvent", "yoda_fountain.destroyParade: Restarting Parade in "+holiday.EMPIRE_DAY_SECONDS_TO_START+" seconds.");
		utils.removeScriptVar(self, "paradeRunning");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int normalMusic(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, "musicObject"))
		{
			location soundLoc = getLocation(self);
			obj_id soundObject = createObject(MUSIC_HAN_LEIA, soundLoc);
			
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
			obj_id soundObject = createObject(MUSIC_HAN_LEIA, soundLoc);
			
			utils.setScriptVar(self, "musicObject", soundObject);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int launchRandomRebelShipFlyBy(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "yoda_fountain.launchRandomTieFighterFlyBy: messageHandler initialized.");
		int num = rand(1,8);
		obj_id right = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"right_most_waypoint");
		obj_id left = getObjIdObjVar(self, holiday.SPAWNER_PREFIX_OBJVAR+"left_most_waypoint");
		if (!isValidId(right) && !exists(right) && !isValidId(left) && !exists(left))
		{
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("holidayEvent", "yoda_fountain.launchRandomRebelShipFlyBy: random number received, firing something.");
		
		switch(num)
		{
			case 1:  
			break;
			case 2:  
			playRebelFlyBy02(self, right);
			break;
			case 3:  
			playRebelFlyBy03(self, left);
			break;
			case 4:  
			playRebelFlyBy04(self, right);
			break;
			case 5:  
			playRebelFlyBy01(self, right);
			break;
			case 6:  
			playRebelFlyBy02(self, left);
			break;
			case 7:  
			playRebelFlyBy03(self, right);
			break;
			case 8:  
			playRebelFlyBy04(self, left);
			break;
			default:
			playRebelFlyBy01(self, left);
			break;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean playRebelFlyBy01(obj_id self, obj_id playOnObject) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "yoda_fountain.playTieFighterGroup: Function initialized.");
		
		if (!isValidId(self) || !exists(self))
		{
			return false;
		}
		if (!isValidId(playOnObject) || !exists(playOnObject))
		{
			return false;
		}
		
		CustomerServiceLog("holidayEvent", "yoda_fountain.playRebelFlyBy01: playing.");
		location here = getLocation(playOnObject);
		playClientEffectLoc(getPlayerCreaturesInRange(here, 200.0f), holiday.REBEL_FLYBY_PARTICLE_01, here, 1.0f);
		return true;
	}
	
	
	public boolean playRebelFlyBy02(obj_id self, obj_id playOnObject) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "yoda_fountain.playRebelFlyBy02: Function initialized.");
		
		if (!isValidId(self) || !exists(self))
		{
			return false;
		}
		if (!isValidId(playOnObject) || !exists(playOnObject))
		{
			return false;
		}
		
		CustomerServiceLog("holidayEvent", "yoda_fountain.playRebelFlyBy02: playing.");
		location here = getLocation(playOnObject);
		playClientEffectLoc(getPlayerCreaturesInRange(here, 200.0f), holiday.REBEL_FLYBY_PARTICLE_02, here, 1.0f);
		return true;
	}
	
	
	public boolean playRebelFlyBy03(obj_id self, obj_id playOnObject) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "yoda_fountain.playRebelFlyBy03: Function initialized.");
		
		if (!isValidId(self) || !exists(self))
		{
			return false;
		}
		if (!isValidId(playOnObject) || !exists(playOnObject))
		{
			return false;
		}
		
		CustomerServiceLog("holidayEvent", "yoda_fountain.playRebelFlyBy03: playing.");
		location here = getLocation(playOnObject);
		playClientEffectLoc(getPlayerCreaturesInRange(here, 200.0f), holiday.REBEL_FLYBY_PARTICLE_03, here, 1.0f);
		return true;
	}
	
	
	public boolean playRebelFlyBy04(obj_id self, obj_id playOnObject) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "yoda_fountain.playRebelFlyBy04: Function initialized.");
		
		if (!isValidId(self) || !exists(self))
		{
			return false;
		}
		if (!isValidId(playOnObject) || !exists(playOnObject))
		{
			return false;
		}
		
		CustomerServiceLog("holidayEvent", "yoda_fountain.playRebelFlyBy04: playing.");
		location here = getLocation(playOnObject);
		playClientEffectLoc(getPlayerCreaturesInRange(here, 200.0f), holiday.REBEL_FLYBY_PARTICLE_04, here, 1.0f);
		return true;
	}
	
	
	public boolean createFalconDropship(obj_id fountain, location loc) throws InterruptedException
	{
		if (loc == null)
		{
			return false;
		}
		
		if (!loc.area.equals(getCurrentSceneName()))
		{
			return false;
		}
		
		obj_id falcon = create.object("object/creature/npc/theme_park/animated_yt1300.iff", loc);
		if (!isIdValid(falcon))
		{
			return false;
		}
		
		setYaw(falcon, 145.0f);
		setObjVar(fountain, holiday.SPAWNER_PREFIX_OBJVAR+"falcon", falcon);
		utils.setScriptVar(fountain, "falcon", falcon);
		attachScript(falcon, "systems.spawning.dropship.emperorsday_yt1300");
		setPosture(falcon, POSTURE_PRONE);
		
		return true;
	}
	
}
