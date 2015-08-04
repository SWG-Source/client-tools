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

public class gcw_barricade extends script.base_script
{
	public gcw_barricade()
	{
	}
	public static final boolean LOGGING_ON = false;
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setObjVar(self, gcw.GCW_TOOL_TEMPLATE_OBJVAR, "object/tangible/gcw/crafting_quest/gcw_barricade_tool.iff");
		setObjVar(self, "questCallBack", 1);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		
		messageTo(self, "handleGCWBarricade", null, 2, false);
		
		dictionary params = new dictionary();
		location loc = getLocation(self);
		params.put("particleLoc", loc);
		int playIconTime = getGameTime();
		params.put("iconMessageTime", playIconTime);
		utils.setScriptVar(self, "iconMessageTime", playIconTime);
		messageTo(self, "playQuestIcon", params, 2.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		if (!exists(self))
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		int idx = utils.getValidAttributeIndex(names);
		
		if (idx == -1)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		names[idx] = "object_repaired";
		int repairCount = getIntObjVar(self, gcw.GCW_OBJECT_REPAIR_COUNT);
		if (repairCount > 0)
		{
			attribs[idx] = ""+repairCount+" out of "+gcw.REPAIR_COUNT_MAX;
		}
		else
		{
			attribs[idx] = "Never Repaired";
		}
		idx++;
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, "faction"))
		{
			blog("no faction on turret obj");
			return SCRIPT_CONTINUE;
		}
		
		if (!factions.isPlayerSameGcwFactionAsSchedulerObject(player, self))
		{
			blog("faction invalid on turret obj");
			return SCRIPT_CONTINUE;
		}
		else
		{
			if (utils.isProfession(player, utils.TRADER))
			{
				blog("player is trader");
				
				if (!gcw.canGcwObjectBeRepaired(self))
				{
					blog("OnObjectMenuRequest no repair needed");
					
					sendSystemMessage(player, gcw.SID_DOESNT_NEED_REPAIR);
					return SCRIPT_CONTINUE;
				}
				if (!gcw.hasConstructionOrRepairTool(player, self))
				{
					blog("OnObjectMenuRequest no resources");
					
					gcw.playerSystemMessageResourceNeeded(player, self, false);
					return SCRIPT_CONTINUE;
				}
			}
			
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
		blog("OnObjectMenuSelect");
		
		if (!isIdValid(player) || !exists(player) || isIncapacitated(player) || isDead(player) || factions.isOnLeave(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, "faction"))
		{
			blog("no faction on patrol obj");
			return SCRIPT_CONTINUE;
		}
		
		int faction = utils.getIntScriptVar(self, "faction");
		if (faction < 0)
		{
			blog("faction invalid on patrol obj");
			return SCRIPT_CONTINUE;
		}
		
		if (!factions.isPlayerSameGcwFactionAsSchedulerObject(player, self))
		{
			blog("Wrong Faction");
			return SCRIPT_CONTINUE;
		}
		else
		{
			if (utils.isProfession(player, utils.TRADER))
			{
				blog("player is trader");
				
				if (!gcw.canGcwObjectBeRepaired(self))
				{
					sendSystemMessage(player, gcw.SID_DOESNT_NEED_REPAIR);
					return SCRIPT_CONTINUE;
				}
				if (!gcw.hasConstructionOrRepairTool(player, self))
				{
					gcw.playerSystemMessageResourceNeeded(player, self, false);
					return SCRIPT_CONTINUE;
				}
				
				if (groundquests.isQuestActive(player, gcw.GCW_REPAIR_BARRICADE_QUEST))
				{
					if (gcw.useGcwObjectForQuest(player, self, gcw.GCW_REPAIR_BARRICADE_QUEST))
					{
						gcw.useConstructionOrRepairTool(player, self);
					}
					return SCRIPT_CONTINUE;
				}
				
				if (groundquests.hasCompletedQuest(player, gcw.GCW_REPAIR_BARRICADE_QUEST))
				{
					groundquests.clearQuest(player, gcw.GCW_REPAIR_BARRICADE_QUEST);
				}
				if (!groundquests.isQuestActive(player, gcw.GCW_REPAIR_BARRICADE_QUEST))
				{
					groundquests.grantQuest(player, gcw.GCW_REPAIR_BARRICADE_QUEST);
				}
			}
			else
			{
				String questName = gcw.GCW_DEFEND_BARRICADE_REBEL;
				if (faction == factions.FACTION_FLAG_IMPERIAL)
				{
					questName = gcw.GCW_DEFEND_BARRICADE_IMPERIAL;
				}
				
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
					setObjVar(player,"defend_target", self);
					groundquests.grantQuest(player, questName);
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int playQuestIcon(obj_id self, dictionary params) throws InterruptedException
	{
		gcw.playQuestIconHandler(self, params);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleGCWBarricade(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id[] objects = getObjectsInRange(self, 5.0f);
		
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
				
				if (isIdValid(object) && isMob(object) && factions.isPlayerSameGcwFactionAsSchedulerObject(object, self))
				{
					buff.applyBuff(object, self, "barricade_defender");
				}
			}
		}
		
		messageTo(self, "handleGCWBarricade", null, 2, false);
		
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
	
	
	public int destroyGCWBarricade(obj_id self, dictionary params) throws InterruptedException
	{
		trial.cleanupObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDeath(obj_id self, obj_id killer, obj_id corpseId) throws InterruptedException
	{
		handleDestroyBarricade(self, killer);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectDisabled(obj_id self, obj_id killer) throws InterruptedException
	{
		handleDestroyBarricade(self, killer);
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleDestroyBarricade(obj_id self, obj_id killer) throws InterruptedException
	{
		playClientEffectLoc(self, "clienteffect/combat_explosion_lair_large.cef", getLocation(self), 0);
		setInvulnerable( self, true );
		messageTo(self, "destroyGCWBarricade", null, 1f, false);
		
		int faction = utils.getIntScriptVar(self, "faction");
		if (faction < 0)
		{
			blog("handleDestroyBarricade - faction invalid on obj");
			return;
		}
		
		if (faction == factions.FACTION_FLAG_IMPERIAL)
		{
			gcw.signalAllParticipantsForDamage(self, gcw.GCW_DESTROY_BARRICADE, gcw.GCW_IMP_DESTROY_BARRICADE_TASK, gcw.GCW_IMP_DESTROY_BARRICADE_SIGNAL);
		}
		else if (faction == factions.FACTION_FLAG_REBEL)
		{
			gcw.signalAllParticipantsForDamage(self, gcw.GCW_DESTROY_BARRICADE, gcw.GCW_REB_DESTROY_BARRICADE_TASK, gcw.GCW_REB_DESTROY_BARRICADE_SIGNAL);
		}
		return;
	}
	
	
	public void blog(String text) throws InterruptedException
	{
		if (LOGGING_ON)
		{
			LOG("gcw_barricade", text);
		}
	}
	
}
