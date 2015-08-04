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


public class gcw_smuggler_device extends script.base_script
{
	public gcw_smuggler_device()
	{
	}
	public static final String PARENT = "parent";
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (!hasObjVar(self, PARENT))
		{
			LOG("gcw_smuggler_functionality","no parent var on obj");
			return SCRIPT_CONTINUE;
		}
		
		obj_id parent = getObjIdObjVar(self, PARENT);
		if (!isValidId(parent))
		{
			LOG("gcw_smuggler_functionality","parent var not valid");
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, "faction"))
		{
			LOG("gcw_smuggler_functionality","no faction on turret obj");
			return SCRIPT_CONTINUE;
		}
		
		int faction = utils.getIntScriptVar(self, "faction");
		if (faction < 0)
		{
			LOG("gcw_smuggler_functionality","faction invalid on turret obj");
			return SCRIPT_CONTINUE;
		}
		
		if (faction != factions.getFactionFlag(player))
		{
			LOG("gcw_smuggler_functionality","faction invalid on turret obj");
			return SCRIPT_CONTINUE;
		}
		
		if (faction == factions.getFactionFlag(player))
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
		LOG("gcw_smuggler_functionality","OnObjectMenuSelect");
		if (!hasObjVar(self, PARENT))
		{
			LOG("gcw_smuggler_functionality","no parent var on obj");
			return SCRIPT_CONTINUE;
		}
		
		obj_id parent = getObjIdObjVar(self, PARENT);
		if (!isValidId(parent))
		{
			LOG("gcw_smuggler_functionality","parent var not valid");
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, "faction"))
		{
			LOG("gcw_smuggler_functionality","no faction on patrol obj");
			return SCRIPT_CONTINUE;
		}
		
		int faction = utils.getIntScriptVar(self, "faction");
		if (faction < 0)
		{
			LOG("gcw_smuggler_functionality","faction invalid on patrol obj");
			return SCRIPT_CONTINUE;
		}
		
		if (faction != factions.getFactionFlag(player))
		{
			LOG("gcw_smuggler_functionality","Wrong Faction");
			return SCRIPT_CONTINUE;
		}
		
		LOG("gcw_smuggler_functionality","player has used patrol point");
		
		if (faction == factions.getFactionFlag(player))
		{
			LOG("gcw_smuggler_functionality","player is of correct faction");
			
			String questName = gcw.GCW_DEFEND_TOWER_REBEL;
			if (faction == 2)
			{
				questName = gcw.GCW_DEFEND_TOWER_IMPERIAL;
			}
			LOG("gcw_smuggler_functionality","questName: "+questName);
			
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
	
}
