package script.event.emperorsday;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.ai_lib;
import script.library.anims;
import script.library.chat;
import script.library.colors;
import script.library.factions;
import script.library.holiday;
import script.library.pclib;
import script.library.prose;
import script.library.stealth;
import script.library.utils;


public class rebel_npc extends script.base_script
{
	public rebel_npc()
	{
	}
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		String npcType = getStringObjVar (self, "emperorsday.npcType");
		if (npcType != null)
		{
			obj_id masterObject = holiday.masterObjectObjId(self);
			if (isIdValid(masterObject))
			{
				dictionary params = new dictionary();
				params.put("type", npcType);
				params.put("npcObjId", self);
				
				messageTo(masterObject, "npcTypeList", params, 3.0f, false);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnMovePathComplete(obj_id self) throws InterruptedException
	{
		obj_id masterObject = holiday.masterObjectObjId(self);
		
		if (hasObjVar (self, "leia"))
		{
			CustomerServiceLog("holidayEvent", "imperial_npc.OnMovePathComplete: Leia has come to a stop at a location.");
			
			obj_id spawnPoint = holiday.getEmpireDayWaypointObjectObjId(self, "spawnCeleb", holiday.OBJECT_NEAR_CHECK_RANGE_2M);
			obj_id pathPoint = holiday.getEmpireDayWaypointObjectObjId(self, "pathLeia", holiday.OBJECT_NEAR_CHECK_RANGE_2M);
			
			if (isIdValid(pathPoint) && exists(pathPoint))
			{
				if (isIdValid(masterObject))
				{
					messageTo(masterObject, "startConversation", null, 3.0f, false);
					setYaw (self, -180.0f);
				}
			}
			if (isIdValid(spawnPoint) && exists(spawnPoint))
			{
				if (isIdValid(masterObject))
				{
					
					obj_id oldSoundObject = utils.getObjIdScriptVar(self, "musicObject");
					
					if ((isIdValid(oldSoundObject)) && (exists(oldSoundObject)))
					{
						destroyObject(oldSoundObject);
					}
					
					messageTo(masterObject, "destroyParade", null, 5.0f, false);
				}
			}
			
		}
		
		else if (hasObjVar(self, "han"))
		{
			CustomerServiceLog("holidayEvent", "imperial_npc.OnMovePathComplete: Han has come to a stop at a location.");
			
			obj_id spawnPoint = holiday.getEmpireDayWaypointObjectObjId(self, "spawnCeleb", holiday.OBJECT_NEAR_CHECK_RANGE_2M);
			obj_id pathPoint = holiday.getEmpireDayWaypointObjectObjId(self, "pathHan", holiday.OBJECT_NEAR_CHECK_RANGE_2M);
			
			if (isIdValid(spawnPoint) && exists(spawnPoint))
			{
				if (isIdValid(masterObject))
				{
					if (hasObjVar(self, "han"))
					{
						destroyObject(self);
						if (utils.hasScriptVar(masterObject, "solo"))
						{
							utils.removeScriptVar(masterObject, "solo");
						}
					}
				}
			}
			if (isIdValid(pathPoint) && exists(pathPoint))
			{
				CustomerServiceLog("holidayEvent", "imperial_npc.OnMovePathComplete: Han has a valid pathPoint.");
				setYaw (self, -140.0f);
			}
		}
		
		else if (hasObjVar(self, "chewie"))
		{
			CustomerServiceLog("holidayEvent", "imperial_npc.OnMovePathComplete: Chewie has come to a stop at a location.");
			
			obj_id spawnPoint = holiday.getEmpireDayWaypointObjectObjId(self, "spawnCeleb", holiday.OBJECT_NEAR_CHECK_RANGE_2M);
			obj_id pathPoint = holiday.getEmpireDayWaypointObjectObjId(self, "pathChewie", holiday.OBJECT_NEAR_CHECK_RANGE_2M);
			
			if (isIdValid(spawnPoint) && exists(spawnPoint))
			{
				if (isIdValid(masterObject))
				{
					if (hasObjVar(self, "chewie"))
					{
						destroyObject(self);
						if (utils.hasScriptVar(masterObject, "chewbacca"))
						{
							utils.removeScriptVar(masterObject, "chewbacca");
						}
					}
				}
			}
			if (isIdValid(pathPoint) && exists(pathPoint))
			{
				CustomerServiceLog("holidayEvent", "imperial_npc.OnMovePathComplete: Chewie has a valid pathPoint.");
				setYaw (self, -140.0f);
			}
		}
		
		else if (hasObjVar(self, "soldier"))
		{
			int soldierNumber = getIntObjVar(self, "soldier");
			if (soldierNumber != 0)
			{
				obj_id spawnPoint = holiday.getEmpireDayWaypointObjectObjId(self, "spawnsoldier"+soldierNumber, holiday.OBJECT_NEAR_CHECK_RANGE_2M);
				obj_id pathPoint = holiday.getEmpireDayWaypointObjectObjId(self, "pathsoldier"+soldierNumber, holiday.OBJECT_NEAR_CHECK_RANGE_2M);
				
				if (isIdValid(pathPoint) && exists(pathPoint))
				{
					location exactLoc = getLocation(pathPoint);
					pathTo(self, exactLoc);
					float pointYaw = getYaw (pathPoint);
					setYaw(self, pointYaw);
					ai_lib.setMood (self, "npc_imperial");
				}
				if (isIdValid(spawnPoint) && exists(spawnPoint))
				{
					float pointYaw = getYaw (spawnPoint);
					setYaw(self, pointYaw);
					
					dictionary params = new dictionary();
					params.put("objvarValue", soldierNumber);
					params.put("soldierObjId", self);
					
					messageTo(masterObject, "setOriginalMood", params, 2.0f, false);
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (!isIdValid(player) || ai_lib.isInCombat(player) || isIncapacitated(player) || isDead(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, "leia") && hasObjVar(self, "readyForBadge") && utils.hasScriptVar(player, "emperorsDayBadge"))
		{
			mi.addRootMenu(menu_info_types.ITEM_USE, holiday.BOW);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (!isIdValid(player) || ai_lib.isInCombat(player) || isIncapacitated(player) || isDead(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, "leia") && hasObjVar(self, "readyForBadge") && utils.hasScriptVar(player, "emperorsDayBadge"))
		{
			obj_id[] listOfWinners = getObjIdArrayObjVar(self, "listOfWinners");
			if (listOfWinners == null || listOfWinners.length <= 0)
			{
				CustomerServiceLog("holidayEvent", "rebel_npc.OnObjectMenuSelect: THE LIST OF WINNERS WASNT FOUND ON VADER. Vader cannot give the player his badge.");
				return SCRIPT_CONTINUE;
			}
			
			boolean playerReward = false;
			for (int i = 0; i < listOfWinners.length; i++)
			{
				testAbortScript();
				if (player != listOfWinners[i])
				{
					continue;
				}
				
				playerReward = true;
				break;
			}
			
			if (!playerReward)
			{
				return SCRIPT_CONTINUE;
			}
			
			if (!holiday.grantEmperorDayBadge(player, self, listOfWinners, holiday.REBEL_PLAYER))
			{
				CustomerServiceLog("holidayEvent", "rebel_npc.OnObjectMenuSelect: The grantEmperorDayBadge function shows that rewarding the player: "+player+" FAILED!");
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnSawEmote(obj_id self, obj_id emoteSayer, String emotein) throws InterruptedException
	{
		if (!isValidId(emoteSayer) || !exists(emoteSayer))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (emotein == null || emotein.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		obj_id emotetarget = getLookAtTarget(emoteSayer);
		if (!isIdValid(emotetarget) || ai_lib.isInCombat(self) || ai_lib.isInCombat(emoteSayer) || isIncapacitated(self) || isDead(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (factions.isImperial(emoteSayer) && emotetarget == self)
		{
			holiday.slapPlayerDownForBeingRude(self, emoteSayer);
			CustomerServiceLog("holidayEvent", "imperial_npc.OnObjectMenuSelect: Player: "+emoteSayer+" was rude to an NPC at the ceremony and will now be punished.");
		}
		
		if ((hasObjVar(self, "leia") && hasObjVar(self, "readyForBadge") && utils.hasScriptVar(emoteSayer, "emperorsDayBadge") && emotetarget == self))
		{
			
			obj_id masterObject = holiday.masterObjectObjId(self);
			
			if (!isIdValid(masterObject) && !exists(masterObject))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (factions.isRebel(emoteSayer))
			{
				switch (holiday.getEmoteBeligerence(emotein))
				{
					
					case 2:
					break;
					
					case 1:
					doAnimationAction(self, "shake_head_no");
					utils.removeScriptVar(emoteSayer, "emperorsDayBadge");
					factions.addFactionStanding(emoteSayer, factions.FACTION_REBEL, -250);
					
					prose_package pp = prose.getPackage(holiday.LEIA_BADGE_2);
					prose.setTT (pp, emoteSayer);
					chat.chat(self, emoteSayer, pp);
					
					dictionary params = new dictionary();
					params.put("rudePlayer", emoteSayer);
					messageTo(masterObject, "hanAngry", params, 2.0f, false);
					
					break;
					
					case 0:
					break;
					
					default:
					break;
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int playerKnockedOut(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("myTarget");
		if (!isValidId(player) && !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("holidayEvent", "rebel_npc.playerKnockedOut: The player: "+player+" was knocked down by an Empire Day Ceremony NPC.");
		damage(player, DAMAGE_KINETIC, HIT_LOCATION_BODY, 100);
		setPosture(player, POSTURE_INCAPACITATED);
		if (factions.isImperial(player))
		{
			holiday.giveCeremonyAreaMobsHate(player, holiday.NPC_ATTACK_RANGE);
		}
		
		return SCRIPT_CONTINUE;
	}
	
}
