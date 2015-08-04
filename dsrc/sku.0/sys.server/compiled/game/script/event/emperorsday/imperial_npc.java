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


public class imperial_npc extends script.base_script
{
	public imperial_npc()
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
		if (!hasObjVar(self, "vader"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id spawnPoint = holiday.getEmpireDayWaypointObjectObjId(self, "spawnVader", holiday.OBJECT_NEAR_CHECK_RANGE_10M);
		obj_id pathPoint = holiday.getEmpireDayWaypointObjectObjId(self, "pathVader", holiday.OBJECT_NEAR_CHECK_RANGE_10M);
		if (isIdValid(pathPoint) && exists(pathPoint))
		{
			obj_id masterObject = holiday.masterObjectObjId(self);
			if (isIdValid(masterObject))
			{
				messageTo(masterObject, "startConversation", null, 3.0f, false);
			}
		}
		
		if (isIdValid(spawnPoint) && exists(spawnPoint))
		{
			obj_id masterObject = holiday.masterObjectObjId(self);
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
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (!isIdValid(player) || ai_lib.isInCombat(player) || isIncapacitated(player) || isDead(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, "vader") && hasObjVar(self, "readyForBadge") && utils.hasScriptVar(player, "emperorsDayBadge"))
		{
			CustomerServiceLog("holidayEvent", "imperial_npc.OnObjectMenuSelect: The player is authorized to access Vader and receive reward badge.");
			
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
		
		if (hasObjVar(self, "vader") && hasObjVar(self, "readyForBadge") && utils.hasScriptVar(player, "emperorsDayBadge"))
		{
			obj_id[] listOfWinners = getObjIdArrayObjVar(self, "listOfWinners");
			if (listOfWinners == null || listOfWinners.length <= 0)
			{
				CustomerServiceLog("holidayEvent", "imperial_npc.OnObjectMenuSelect: THE LIST OF WINNERS WASNT FOUND ON VADER. Vader cannot give the player his badge.");
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
			
			CustomerServiceLog("holidayEvent", "imperial_npc.OnObjectMenuSelect: The list of winners was received. Player: "+player+" has the variable to claim reward.");
			if (!holiday.grantEmperorDayBadge(player, self, listOfWinners, holiday.IMPERIAL_PLAYER))
			{
				CustomerServiceLog("holidayEvent", "imperial_npc.OnObjectMenuSelect: The grantEmperorDayBadge function shows that rewarding the player: "+player+" FAILED!");
			}
		}
		sendDirtyObjectMenuNotification(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnSawEmote(obj_id self, obj_id emoteSayer, String emotein) throws InterruptedException
	{
		obj_id emotetarget = getLookAtTarget(emoteSayer);
		if (!isIdValid(emotetarget) || ai_lib.isInCombat(self) || ai_lib.isInCombat(emoteSayer) || isIncapacitated(self) || isDead(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, "vader") && factions.isRebel(emoteSayer) && emotetarget == self)
		{
			forceChokePlayer(self, emoteSayer);
		}
		
		if (!hasObjVar(self, "vader") && factions.isRebel(emoteSayer) && emotetarget == self)
		{
			holiday.slapPlayerDownForBeingRude(self, emoteSayer);
			CustomerServiceLog("holidayEvent", "imperial_npc.OnObjectMenuSelect: Player: "+emoteSayer+" was rude to an NPC at the ceremony and will now be punished.");
		}
		
		if (hasObjVar(self, "vader") && hasObjVar(self, "readyForBadge") && utils.hasScriptVar(emoteSayer, "emperorsDayBadge"))
		{
			
			obj_id masterObject = holiday.masterObjectObjId(self);
			if (!isIdValid(masterObject) && !exists(masterObject))
			{
				return SCRIPT_CONTINUE;
			}
			if (emotetarget != self)
			{
				return SCRIPT_CONTINUE;
			}
			
			if (factions.isImperial(emoteSayer))
			{
				switch ( holiday.getEmoteBeligerence(emotein) )
				{
					
					case 2:
					break;
					
					case 1:
					doAnimationAction(self, "shake_head_no");
					
					utils.removeScriptVar(emoteSayer, "emperorsDayBadge");
					factions.addFactionStanding(emoteSayer, factions.FACTION_IMPERIAL, -250);
					
					prose_package pp = prose.getPackage(holiday.DARTH_BADGE_2);
					prose.setTT (pp, emoteSayer);
					chat.chat(self, emoteSayer, pp);
					forceChokePlayer(self, emoteSayer);
					
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
		
		CustomerServiceLog("holidayEvent", "imperial_npc.playerKnockedOut: The player: "+player+" was knocked down by an Empire Day Ceremony NPC.");
		damage(player, DAMAGE_KINETIC, HIT_LOCATION_BODY, 100);
		setPosture(player, POSTURE_INCAPACITATED);
		if (factions.isRebel(player))
		{
			holiday.giveCeremonyAreaMobsHate(player, holiday.NPC_ATTACK_RANGE);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean forceChokePlayer(obj_id npcObject, obj_id playerToBeIncapped) throws InterruptedException
	{
		stealth.testInvisNonCombatAction(playerToBeIncapped, npcObject);
		doAnimationAction(npcObject, "force_choke");
		doAnimationAction(playerToBeIncapped, "heavy_cough_vomit");
		
		dictionary eDict = new dictionary();
		eDict.put("myTarget", playerToBeIncapped);
		messageTo(npcObject, "playerKnockedOut", eDict, 6, false);
		
		for (int i = 0; i < 4; i++)
		{
			testAbortScript();
			string_id strText = new string_id("combat_effects", "choke");
			color colTest = colors.RED;
			showFlyText(playerToBeIncapped, strText, 1, colTest);
		}
		string_id strSpam = new string_id("combat_effects", "choke_single");
		sendSystemMessage(playerToBeIncapped, strSpam);
		return true;
	}
	
}
