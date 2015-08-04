package script.theme_park.outbreak;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.utils;
import script.library.groundquests;

public class outbreak_defense_enemy extends script.base_script
{
	public outbreak_defense_enemy()
	{
	}
	public static final String GUARD_OCCUPIED = "occupied";
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		messageTo(self, "checkValidityOfOwner", null, 5, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnMovePathComplete(obj_id self) throws InterruptedException
	{
		CustomerServiceLog("outbreak_themepark", "outbreak_defense_enemy.OnMovePathComplete() initialized.");
		if (!hasObjVar(self, "owner"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id owner = getObjIdObjVar(self, "owner");
		if (!isValidId(owner) || !exists(owner))
		{
			return SCRIPT_CONTINUE;
		}
		CustomerServiceLog("outbreak_themepark", "outbreak_defense_enemy.OnMovePathComplete() Starting Combat with Owner.");
		
		startCombat(self, owner);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDeath(obj_id self, obj_id killer, obj_id corpseId) throws InterruptedException
	{
		CustomerServiceLog("outbreak_themepark", "outbreak_defense_enemy.OnDeath() Undead NPC has died. Crediting Player.");
		
		if (!hasObjVar(self, "questName"))
		{
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(self, "survivalTaskName"))
		{
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(self, "cleanUpTaskName"))
		{
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("outbreak_themepark", "outbreak_defense_enemy.OnDeath() Undead NPC Validation completed.");
		
		obj_id player = getObjIdObjVar(self, "owner");
		if (isValidId(player) && exists(player))
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_defense_enemy.OnDeath() Undead NPC is owned by player: "+player);
			
			String questName = getStringObjVar(self, "questName");
			if (questName == null || questName.length() <= 0)
			{
				return SCRIPT_CONTINUE;
			}
			String survivalTaskName = getStringObjVar(self, "survivalTaskName");
			if (survivalTaskName == null || survivalTaskName.length() <= 0)
			{
				return SCRIPT_CONTINUE;
			}
			String cleanUpTaskName = getStringObjVar(self, "cleanUpTaskName");
			if (cleanUpTaskName == null || cleanUpTaskName.length() <= 0)
			{
				return SCRIPT_CONTINUE;
			}
			
			CustomerServiceLog("outbreak_themepark", "outbreak_defense_enemy.OnDeath() Undead NPC is is sending message to player: "+player);
			
			dictionary webster = new dictionary();
			webster.put("questName", questName);
			webster.put("survivalTaskName", survivalTaskName);
			webster.put("cleanUpTaskName", cleanUpTaskName);
			messageTo(player, "recalculateCampDefenseUndeadArray", webster, 0, false );
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnIncapacitateTarget(obj_id self, obj_id victim) throws InterruptedException
	{
		CustomerServiceLog("outbreak_themepark", "outbreak_defense_enemy.OnIncapacitateTarget() Player has died. Checking if quest needs updating.");
		if (!hasObjVar(self, "questName"))
		{
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(self, "survivalTaskName"))
		{
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(self, "cleanUpTaskName"))
		{
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(self, "owner"))
		{
			return SCRIPT_CONTINUE;
		}
		CustomerServiceLog("outbreak_themepark", "outbreak_defense_enemy.OnIncapacitateTarget() Undead NPC Validation completed.");
		
		obj_id player = getObjIdObjVar(self, "owner");
		if (!isValidId(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (victim != player)
		{
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("outbreak_themepark", "outbreak_defense_enemy.OnIncapacitateTarget() Failing player's quest.");
		groundquests.sendSignal(player, "campDefenseFailed");
		
		if (utils.hasScriptVar(player, GUARD_OCCUPIED))
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_defense_enemy.OnIncapacitateTarget() Attempting to set the guard post to available.");
			
			obj_id guardPost = utils.getObjIdScriptVar(self,GUARD_OCCUPIED);
			if (!isValidId(guardPost) || !exists(guardPost))
			{
				CustomerServiceLog("outbreak_themepark", "outbreak_defense_enemy.OnIncapacitateTarget() could not find guard post objvar on player!!!");
				return SCRIPT_CONTINUE;
			}
			utils.removeScriptVar(guardPost, GUARD_OCCUPIED);
			CustomerServiceLog("outbreak_themepark", "outbreak_defense_enemy.OnIncapacitateTarget() Set guard post: "+guardPost+" to available.");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int checkValidityOfOwner(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("outbreak_themepark", "outbreak_defense_enemy.checkValidityOfOwner() Initalized Message Handler.");
		
		if (!hasObjVar(self, "questName"))
		{
			messageTo(self, "cleanUpEnemy", null, 0, false);
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(self, "survivalTaskName"))
		{
			messageTo(self, "cleanUpEnemy", null, 0, false);
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(self, "cleanUpTaskName"))
		{
			messageTo(self, "cleanUpEnemy", null, 0, false);
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(self, "owner"))
		{
			messageTo(self, "cleanUpEnemy", null, 0, false);
			return SCRIPT_CONTINUE;
		}
		CustomerServiceLog("outbreak_themepark", "outbreak_defense_enemy.checkValidityOfOwner() Undead NPC Validation completed.");
		
		obj_id player = getObjIdObjVar(self, "owner");
		if (!isValidId(player) || !exists(player))
		{
			messageTo(self, "cleanUpEnemy", null, 0, false);
			return SCRIPT_CONTINUE;
		}
		String questName = getStringObjVar(self, "questName");
		if (questName == null || questName.length() <= 0)
		{
			messageTo(self, "cleanUpEnemy", null, 0, false);
			return SCRIPT_CONTINUE;
		}
		if (!groundquests.isQuestActive(player, questName))
		{
			removeGuardPostFlag(player);
			messageTo(self, "cleanUpEnemy", null, 0, false);
			return SCRIPT_CONTINUE;
		}
		messageTo(self, "checkValidityOfOwner", null, 5, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int cleanUpEnemy(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("outbreak_themepark", "outbreak_defense_enemy.cleanUpEnemy() Initalized Message Handler.");
		
		destroyObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean removeGuardPostFlag(obj_id player) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		CustomerServiceLog("outbreak_themepark", "outbreak_defense_enemy.removeGuardPostFlag() Player "+player+" is attempting to remove the guard post occupied flagging.");
		
		if (utils.hasScriptVar(player, GUARD_OCCUPIED))
		{
			obj_id guardPost = utils.getObjIdScriptVar(player, GUARD_OCCUPIED);
			if (isValidId(guardPost) && exists(guardPost))
			{
				utils.removeScriptVar(guardPost, GUARD_OCCUPIED);
				return true;
			}
		}
		CustomerServiceLog("outbreak_themepark", "outbreak_defense_enemy.removeGuardPostFlag() Player "+player+" did not have the guard post oid or the oid was invalid.");
		return false;
	}
}
