package script.theme_park.nym;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.ai_lib;
import script.library.chat;
import script.library.group;
import script.library.groundquests;
import script.library.hue;
import script.library.trial;
import script.library.utils;


public class boss_nyms_themepark extends script.base_script
{
	public boss_nyms_themepark()
	{
	}
	public static final int RADIUS = 100;
	public static final int MIN_DIST = 25;
	public static final string_id SID_SYS_TOO_FAR = new string_id("spam","too_far_from_spawn_point");
	public static final string_id SID_SYS_ONE_MINUTE_WARNING = new string_id("spam","one_minute_warning_before_despawn");
	
	public static final String DISTANCE_CHECK = "distance_check";
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		messageTo(self, "moveCreatureToWaypoint", null, 1, false);
		messageTo(self, "checkCombatStatus", null, 1, false);
		messageTo(self, "willDespawn", null, 540, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAboutToBeIncapacitated(obj_id self, obj_id killer) throws InterruptedException
	{
		CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.OnAboutToBeIncapacitated() Boss Mob "+self+" incapacitated by: "+killer);
		
		if (!hasObjVar(self, "theme_park_boss"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self, "theme_park_boss_slot"))
		{
			return SCRIPT_CONTINUE;
		}
		
		String bossQuest = getStringObjVar(self, "theme_park_boss");
		if (bossQuest == null || bossQuest.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		String bossQuestSlot = getStringObjVar(self, "theme_park_boss_slot");
		if (bossQuestSlot == null || bossQuestSlot.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.OnAboutToBeIncapacitated() Boss Mob slot and quest name data validated. Boss: "+bossQuest+" Slot: "+bossQuestSlot);
		
		obj_id[] attackerList = utils.getObjIdBatchScriptVar(self, "creditForKills.attackerList.attackers");
		
		if (attackerList != null && attackerList.length > 0)
		{
			CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.OnAboutToBeIncapacitated() Boss Mob attackerList = "+attackerList.length);
			
			for (int i = 0; i < attackerList.length; i++)
			{
				testAbortScript();
				if (!isIdValid(attackerList[i]))
				{
					continue;
				}
				
				if (group.isGroupObject(attackerList[i]))
				{
					obj_id[] members = getGroupMemberIds(attackerList[i]);
					
					for (int j = 0, k = members.length; j < k; j++)
					{
						testAbortScript();
						if (isIdValid(members[j]))
						{
							if (!groundquests.isQuestActive(attackerList[i], bossQuest))
							{
								CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.OnAboutToBeIncapacitated() Boss Mob attacker: "+attackerList[i]+" did not have boss quest: "+bossQuest);
								continue;
							}
							
							if (hasCompletedCollectionSlot(attackerList[i], bossQuestSlot))
							{
								CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.OnAboutToBeIncapacitated() Boss Mob attacker: "+attackerList[i]+" has already attained: "+bossQuestSlot);
								continue;
							}
							
							if (!hasCompletedCollectionSlotPrereq(attackerList[i], bossQuestSlot))
							{
								CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.OnAboutToBeIncapacitated() Boss Mob attacker: "+attackerList[i]+" hasn't fulfilled the prerequisite for boss slot/: "+bossQuestSlot);
								continue;
							}
							modifyCollectionSlotValue(attackerList[i], bossQuestSlot, 1);
						}
					}
				}
				else
				{
					if (!groundquests.isQuestActive(attackerList[i], bossQuest))
					{
						continue;
					}
					
					if (hasCompletedCollectionSlot(attackerList[i], bossQuestSlot))
					{
						continue;
					}
					
					if (!hasCompletedCollectionSlotPrereq(attackerList[i], bossQuestSlot))
					{
						continue;
					}
					
					modifyCollectionSlotValue(attackerList[i], bossQuestSlot, 1);
				}
			}
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int OnEnteredCombat(obj_id self) throws InterruptedException
	{
		if (!isIdValid(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, "shuttle"))
		{
			return SCRIPT_CONTINUE;
		}
		
		int maxDist = getIntObjVar(self, DISTANCE_CHECK);
		if (maxDist <= 0)
		{
			maxDist = MIN_DIST;
			CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.OnEnteredCombat() maximum distance from creation location for boss mob ("+self+") was invalid or NULL.");
		}
		
		obj_id parent = getObjIdObjVar(self, trial.PARENT);
		if (!isIdValid(parent) || !exists(parent))
		{
			destroyObject(self);
			CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.willDespawn() FAILED to find parent object.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id playerEnemy = utils.getObjIdScriptVar(parent, "waveEventPlayer");
		if (!isIdValid(playerEnemy) || !exists(playerEnemy))
		{
			CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.OnEnteredCombat() Mob could not find the wave event player: "+playerEnemy);
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] players = getPlayerCreaturesInRange(self, maxDist);
		if (players != null && players.length > 0)
		{
			for (int i = 0; i < players.length; i++)
			{
				testAbortScript();
				if (!isIdValid(players[i]) && !exists(players[i]) || isIncapacitated(players[i]) || !isDead(players[i]))
				{
					continue;
				}
				
				addHate(self, players[i], 1000.0f);
				startCombat(self, players[i]);
			}
		}
		
		messageTo(self, "handleBossDistanceCheck", null, 3, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnIncapacitateTarget(obj_id self, obj_id victim) throws InterruptedException
	{
		if (!isIdValid(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.isDead(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, "shuttle"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id parent = getObjIdObjVar(self, trial.PARENT);
		if (!isIdValid(parent) || !exists(parent))
		{
			destroyObject(self);
			CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.OnIncapacitateTarget() FAILED to find parent object.");
			return SCRIPT_CONTINUE;
		}
		
		if (getRandomCombatTarget(self, parent))
		{
			CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.OnIncapacitateTarget() Boss Mob has found someone in the player's group.");
			return SCRIPT_CONTINUE;
		}
		
		dictionary webster = trial.getSessionDict(parent);
		messageTo(parent, "defaultEventReset", webster, 2, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnExitedCombat(obj_id self) throws InterruptedException
	{
		if (!isIdValid(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.isDead(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, "shuttle"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id parent = getObjIdObjVar(self, trial.PARENT);
		if (!isIdValid(parent) || !exists(parent))
		{
			destroyObject(self);
			CustomerServiceLog("quest", "boss_nyms_themepark.OnExitedCombat() FAILED to find parent object.");
			return SCRIPT_CONTINUE;
		}
		
		if (getRandomCombatTarget(self, parent))
		{
			return SCRIPT_CONTINUE;
		}
		dictionary webster = trial.getSessionDict(parent);
		messageTo(parent, "defaultEventReset", webster, 2, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleBossDistanceCheck(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!ai_lib.isInCombat(self))
		{
			messageTo(self, "checkCombatStatus", null, 0, false);
			return SCRIPT_CONTINUE;
		}
		
		location currentLoc = getLocation(self);
		if (currentLoc == null)
		{
			CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.handleBossDistanceCheck() current boss mob ("+self+") location NULL.");
			return SCRIPT_CONTINUE;
		}
		
		location creationLoc = aiGetHomeLocation(self);
		if (creationLoc == null)
		{
			CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.handleBossDistanceCheck() creation location for boss mob ("+self+") was NULL.");
			return SCRIPT_CONTINUE;
		}
		
		float distanceCheck = utils.getDistance2D(currentLoc, creationLoc);
		int maxDist = getIntObjVar(self, DISTANCE_CHECK);
		if (maxDist <= 0)
		{
			maxDist = MIN_DIST;
			CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.handleBossDistanceCheck() maximum distance from creation location for boss mob ("+self+") was invalid or NULL.");
		}
		
		obj_id parent = getObjIdObjVar(self, trial.PARENT);
		if (!isIdValid(parent) || !exists(parent))
		{
			destroyObject(self);
			CustomerServiceLog("quest", "boss_nyms_themepark.handleBossDistanceCheck() FAILED to find parent object.");
			return SCRIPT_CONTINUE;
		}
		
		if (distanceCheck > maxDist)
		{
			
			obj_id playerEnemy = utils.getObjIdScriptVar(parent, "waveEventPlayer");
			if (isIdValid(playerEnemy) && exists(playerEnemy))
			{
				sendSystemMessage(playerEnemy, SID_SYS_TOO_FAR);
				
				obj_id myGroup = getGroupObject(playerEnemy);
				if (isValidId(myGroup))
				{
					obj_id[] members = getGroupMemberIds(myGroup);
					
					for (int i = 0; i < members.length; i++)
					{
						testAbortScript();
						if (!isIdValid(members[i]))
						{
							continue;
						}
						
						sendSystemMessage(members[i], SID_SYS_TOO_FAR);
					}
				}
			}
			
			CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.handleBossDistanceCheck() boss mob ("+self+") has moved too far from creation location. Moving back to creation location.");
			dictionary webster = trial.getSessionDict(parent);
			messageTo(parent, "defaultEventReset", webster, 2, false);
			
		}
		else
		{
			messageTo(self, "handleBossDistanceCheck", null, 3, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean getRandomCombatTarget(obj_id self, obj_id parent) throws InterruptedException
	{
		if (!isIdValid(self) || !exists(self))
		{
			CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.getRandomCombatTarget() FAILED to find self.");
			return false;
		}
		
		if (!isIdValid(parent) || !exists(parent))
		{
			destroyObject(self);
			CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.getRandomCombatTarget() FAILED to find parent object.");
			return false;
		}
		
		int maxDist = getIntObjVar(self, DISTANCE_CHECK);
		
		if (maxDist <= 0)
		{
			maxDist = MIN_DIST;
			CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.getRandomCombatTarget() maximum distance from creation location for boss mob ("+self+") was invalid or NULL.");
		}
		
		obj_id[] targets = trial.getValidTargetsInRadiusIgnoreLOS(self, maxDist);
		if (targets == null || targets.length == 0)
		{
			CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.getRandomCombatTarget() no targets for boss ("+self+") is: "+maxDist);
			dictionary webster = trial.getSessionDict(parent);
			messageTo(parent, "defaultEventReset", webster, 2, false);
			return false;
		}
		
		for (int i = 0; i < targets.length; i++)
		{
			testAbortScript();
			if (!isIdValid(targets[i]))
			{
				continue;
			}
			
			if (!isPlayer(targets[i]))
			{
				continue;
			}
			
			startCombat(self, targets[i]);
			CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.getRandomCombatTarget(): Player "+ targets[i] + " was found as a valid target. Boss Mob: "+self+" attacking player.");
			return true;
		}
		CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.getRandomCombatTarget(): Boss Mob: "+self+" failed to find a player to attack.");
		dictionary webster = trial.getSessionDict(parent);
		messageTo(parent, "defaultEventReset", webster, 2, false);
		return false;
		
	}
	
	
	public int moveCreatureToWaypoint(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if ((!hasObjVar(self, "arena") && !hasObjVar(self, "shuttle") && !hasObjVar(self, "henchmen")))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, "arena"))
		{
			location startPos = new location(472.5f, 34.4f, 4815.6f, "lok", null);
			
			setLocation(self, startPos);
			aiSetHomeLocation(self, startPos);
			setYaw(self, 160.0f);
		}
		if (hasObjVar(self, "shuttle"))
		{
			location locCenter = new location(473.0f, 12.0f, 4868.0f, "lok", null);
			location locDestination = utils.getRandomLocationInRing(locCenter, 4.0f, 6.0f);
			setLocation(self, locDestination);
			aiSetHomeLocation(self, locDestination);
		}
		if (hasObjVar(self, "henchmen"))
		{
			location locCenter = getLocation(self);
			location locDestination = utils.getRandomLocationInRing(locCenter, 4.0f, 6.0f);
			setLocation(self, locDestination);
			aiSetHomeLocation(self, locDestination);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int checkCombatStatus(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.isDead(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.isInCombat(self))
		{
			CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.checkCombatStatus(): Boss Mob: "+self+" is already in combat.");
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id parent = getObjIdObjVar(self, "parent");
		if (isIdValid(parent))
		{
			if (getRandomCombatTarget(self, parent))
			{
				return SCRIPT_CONTINUE;
			}
			dictionary webster = trial.getSessionDict(parent);
			messageTo(parent, "defaultEventReset", webster, 2, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int willDespawn(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.isDead(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!ai_lib.isInCombat(self))
		{
			messageTo(self, "checkCombatStatus", null, 0, false);
			return SCRIPT_CONTINUE;
		}
		
		obj_id parent = getObjIdObjVar(self, trial.PARENT);
		if (!isIdValid(parent) || !exists(parent))
		{
			destroyObject(self);
			CustomerServiceLog("nyms_themepark", "boss_nyms_themepark.willDespawn() FAILED to find parent object.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id playerEnemy = utils.getObjIdScriptVar(parent, "waveEventPlayer");
		if (isIdValid(playerEnemy) && exists(playerEnemy))
		{
			sendSystemMessage(playerEnemy, SID_SYS_ONE_MINUTE_WARNING);
			
			obj_id myGroup = getGroupObject(playerEnemy);
			if (isValidId(myGroup))
			{
				obj_id[] members = getGroupMemberIds(myGroup);
				
				for (int i = 0; i < members.length; i++)
				{
					testAbortScript();
					if (!isIdValid(members[i]))
					{
						continue;
					}
					
					sendSystemMessage(members[i], SID_SYS_ONE_MINUTE_WARNING);
				}
			}
			
		}
		
		return SCRIPT_CONTINUE;
	}
}
