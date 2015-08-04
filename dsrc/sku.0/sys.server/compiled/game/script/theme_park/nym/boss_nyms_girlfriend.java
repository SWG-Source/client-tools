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
import script.library.hue;
import script.library.trial;
import script.library.utils;


public class boss_nyms_girlfriend extends script.base_script
{
	public boss_nyms_girlfriend()
	{
	}
	public static final int RADIUS = 100;
	public static final int MIN_DIST = 20;
	
	public static final String DISTANCE_CHECK = "distance_check";
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		if (!isIdValid(self))
		{
			return SCRIPT_CONTINUE;
		}
		
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
		
		obj_id parent = getObjIdObjVar(self, trial.PARENT);
		if (!isIdValid(parent) || !exists(parent))
		{
			destroyObject(self);
			CustomerServiceLog("outbreak_themepark", "boss_fight_functionality.setDynamicDataOnMob() FAILED to find parent object.");
			return SCRIPT_CONTINUE;
		}
		
		if (getRandomCombatTarget(self, parent))
		{
			CustomerServiceLog("outbreak_themepark", "boss_fight_functionality.OnIncapacitateTarget() Boss Mob has found someone in the player's group.");
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
		
		obj_id parent = getObjIdObjVar(self, trial.PARENT);
		if (!isIdValid(parent) || !exists(parent))
		{
			destroyObject(self);
			CustomerServiceLog("quest", "dynamic_mob_opponent.OnExitedCombat() FAILED to find parent object.");
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
		
		location currentLoc = getLocation(self);
		if (currentLoc == null)
		{
			CustomerServiceLog("outbreak_themepark", "boss_fight_functionality.handleBossDistanceCheck() current boss mob ("+self+") location NULL.");
			return SCRIPT_CONTINUE;
		}
		
		location creationLoc = aiGetHomeLocation(self);
		if (creationLoc == null)
		{
			CustomerServiceLog("outbreak_themepark", "boss_fight_functionality.handleBossDistanceCheck() creation location for boss mob ("+self+") was NULL.");
			return SCRIPT_CONTINUE;
		}
		
		float distanceCheck = utils.getDistance2D(currentLoc, creationLoc);
		int maxDist = getIntObjVar(self, DISTANCE_CHECK);
		if (maxDist <= 0)
		{
			maxDist = MIN_DIST;
			CustomerServiceLog("outbreak_themepark", "boss_fight_functionality.handleBossDistanceCheck() maximum distance from creation location for boss mob ("+self+") was invalid or NULL.");
		}
		
		obj_id parent = getObjIdObjVar(self, trial.PARENT);
		if (!isIdValid(parent) || !exists(parent))
		{
			destroyObject(self);
			CustomerServiceLog("quest", "dynamic_mob_opponent.OnExitedCombat() FAILED to find parent object.");
			return SCRIPT_CONTINUE;
		}
		
		if (distanceCheck > maxDist)
		{
			CustomerServiceLog("outbreak_themepark", "boss_fight_functionality.handleBossDistanceCheck() boss mob ("+self+") has moved too far from creation location. Moving back to creation location.");
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
			CustomerServiceLog("outbreak_themepark", "boss_fight_functionality.getRandomCombatTarget() FAILED to find self.");
			return false;
		}
		
		if (!isIdValid(parent) || !exists(parent))
		{
			destroyObject(self);
			CustomerServiceLog("outbreak_themepark", "boss_fight_functionality.getRandomCombatTarget() FAILED to find parent object.");
			return false;
		}
		
		int maxDist = getIntObjVar(self, DISTANCE_CHECK);
		
		if (maxDist <= 0)
		{
			maxDist = MIN_DIST;
			CustomerServiceLog("outbreak_themepark", "boss_fight_functionality.getRandomCombatTarget() maximum distance from creation location for boss mob ("+self+") was invalid or NULL.");
		}
		
		obj_id[] targets = trial.getValidTargetsInRadiusIgnoreLOS(self, maxDist);
		if (targets == null || targets.length == 0)
		{
			CustomerServiceLog("outbreak_themepark", "boss_fight_functionality.getRandomCombatTarget() no targets for boss ("+self+") is: "+maxDist);
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
			
			startCombat(self, targets[i]);
			CustomerServiceLog("outbreak_themepark", "boss_fight_functionality.getRandomCombatTarget(): Player "+ targets[i] + " was found as a valid target. Boss Mob: "+self+" attacking player.");
			return true;
		}
		CustomerServiceLog("outbreak_themepark", "boss_fight_functionality.OnIncapacitateTarget(): Boss Mob: "+self+" failed to find a player to attack.");
		dictionary webster = trial.getSessionDict(parent);
		messageTo(parent, "defaultEventReset", webster, 2, false);
		return false;
		
	}
}
