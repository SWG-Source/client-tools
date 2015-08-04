package script.systems.gcw;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.buff;
import script.library.factions;
import script.library.gcw;
import script.library.groundquests;
import script.library.resource;
import script.library.restuss_event;
import script.library.static_item;
import script.library.trial;
import script.library.utils;


public class gcw_tower extends script.base_script
{
	public gcw_tower()
	{
	}
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		setObjVar(self, "questCallBack", 1);
		
		dictionary params = new dictionary();
		location loc = getLocation(self);
		params.put("particleLoc", loc);
		int playIconTime = getGameTime();
		params.put("iconMessageTime", playIconTime);
		utils.setScriptVar(self, "iconMessageTime", playIconTime);
		
		messageTo(self, "playQuestIcon", params, 1.0f, false);
		
		setInvulnerable(self, false);
		pvpSetAttackableOverride(self, true);
		
		messageTo(self, "handleGCWTower", null, 2, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, "faction"))
		{
			LOG("gcw_patrol_point","no faction on turret obj");
			return SCRIPT_CONTINUE;
		}
		
		if (!factions.isPlayerSameGcwFactionAsSchedulerObject(player, self))
		{
			LOG("gcw_patrol_point","faction invalid on turret obj");
			return SCRIPT_CONTINUE;
		}
		else
		{
			menu_info_data data = mi.getMenuItemByType(menu_info_types.ITEM_USE);
			
			if (data != null)
			{
				data.setServerNotify (true);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		LOG("gcw_patrol_point","OnObjectMenuSelect");
		
		if (!isIdValid(player) || !exists(player) || isIncapacitated(player) || isDead(player) || factions.isOnLeave(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, "faction"))
		{
			LOG("gcw_patrol_point","no faction on patrol obj");
			return SCRIPT_CONTINUE;
		}
		
		int faction = utils.getIntScriptVar(self, "faction");
		if (faction < 0)
		{
			LOG("gcw_patrol_point","faction invalid on patrol obj");
			return SCRIPT_CONTINUE;
		}
		
		if (!factions.isPlayerSameGcwFactionAsSchedulerObject(player, self))
		{
			LOG("gcw_patrol_point","Wrong Faction");
			return SCRIPT_CONTINUE;
		}
		else
		{
			LOG("gcw_patrol_point","player is of correct faction");
			
			String questName = gcw.GCW_DEFEND_TOWER_REBEL;
			if (faction == factions.FACTION_FLAG_IMPERIAL)
			{
				questName = gcw.GCW_DEFEND_TOWER_IMPERIAL;
			}
			LOG("gcw_patrol_point","questName: "+questName);
			
			if (groundquests.isQuestActive(player, questName))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (groundquests.hasCompletedQuest(player, questName))
			{
				groundquests.clearQuest(player, questName);
			}
			if (!groundquests.isQuestActive(player, questName))
			{
				groundquests.grantQuest(player, questName);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int destroyGCWTower(obj_id self, dictionary params) throws InterruptedException
	{
		trial.cleanupObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int playQuestIcon(obj_id self, dictionary params) throws InterruptedException
	{
		params.put("offset", 12.0f);
		
		gcw.playQuestIconHandler(self, params);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDeath(obj_id self, obj_id killer, obj_id corpseId) throws InterruptedException
	{
		handleDestroyTower(self, killer);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectDisabled(obj_id self, obj_id killer) throws InterruptedException
	{
		handleDestroyTower(self, killer);
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleDestroyTower(obj_id self, obj_id killer) throws InterruptedException
	{
		playClientEffectLoc(self, "clienteffect/combat_explosion_lair_large.cef", getLocation(self), 0);
		setInvulnerable( self, true );
		messageTo(self, "destroyGCWTower", null, 1f, false);
		return;
	}
	
	
	public int OnObjectDamaged(obj_id self, obj_id attacker, obj_id weapon, int damage) throws InterruptedException
	{
		if (!isIdValid(attacker) || !exists(attacker) || !isIdValid(weapon) || !exists(weapon) || damage < 1)
		{
			return SCRIPT_CONTINUE;
		}
		
		String template = getTemplateName(attacker);
		
		if (template == null || (!template.equals("object/mobile/atst.iff") && !template.equals("object/mobile/atxt.iff") && !template.equals("object/mobile/atat.iff")))
		{
			LOG("gcw_vehicle", "Tower being attacked by non-vehicle: "+ attacker);
			return SCRIPT_CONTINUE;
		}
		
		LOG("gcw_vehicle", "Tower being attacked by vehicle: "+ attacker + " damage: "+ damage);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleGCWTower(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id[] objects = getObjectsInRange(self, 25.0f);
		
		int faction = -1;
		
		if (utils.hasScriptVar(self, "faction"))
		{
			faction = utils.getIntScriptVar(self, "faction");
		}
		
		if (objects != null && objects.length > 0)
		{
			for (int i = 0; i < objects.length; i++)
			{
				testAbortScript();
				obj_id object = objects[i];
				
				if (isIdValid(object) && isMob(object) && !isPlayer(object) && !isIdValid(getMaster(object)) && factions.isPlayerSameGcwFactionAsSchedulerObject(object, self))
				{
					buff.applyBuff(object, self, "tower_defender");
				}
			}
		}
		
		messageTo(self, "handleGCWTower", null, 10, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleQuestCallBack(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!params.containsKey("questComplete"))
		{
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey("player"))
		{
			return SCRIPT_CONTINUE;
		}
		if (params.getBoolean("questComplete"))
		{
			obj_id player = params.getObjId("player");
			if (!isValidId(player))
			{
				return SCRIPT_CONTINUE;
			}
			
			trial.addNonInstanceFactionParticipant(player, self);
		}
		return SCRIPT_CONTINUE;
	}
}
