package script.systems.gcw;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.ai_lib;
import script.library.buff;
import script.library.chat;
import script.library.create;
import script.library.factions;
import script.library.gcw;
import script.library.groundquests;
import script.library.restuss_event;
import script.library.resource;
import script.library.static_item;
import script.library.sui;
import script.library.stealth;
import script.library.trial;
import script.library.utils;


public class gcw_patrol extends script.base_script
{
	public gcw_patrol()
	{
	}
	public static final string_id SID_THERE_IS_NO_GUARD = new string_id("gcw", "there_is_no_guard_to_entertain");
	public static final string_id SID_PATROL_ALREADY_ENTERTAINED = new string_id("gcw", "patrol_point_already_entertained");
	public static final int ENTERTAINMENT_TIMER = 300;
	
	public static final String[] lowImperials = 
	{
		"gcw_city_imperial_1"
	};
	
	public static final String[] normalImperials = 
	{
		"gcw_city_imperial_black",
		"gcw_city_imperial_2",
		"gcw_city_imperial_sandtrooper"
	};
	
	public static final String[] eliteImperials = 
	{
		"gcw_city_imperial_3",
		"gcw_city_imperial_black_sgt",
		"gcw_city_imperial_black_darkt",
		"gcw_city_imperial_sandtrooper_sgt"
	};
	
	public static final String[] lowRebels = 
	{
		"gcw_city_rebel_grunt_assault",
		"gcw_city_rebel_grunt_defense"
	};
	
	public static final String[] normalRebels = 
	{
		"gcw_city_rebel_commando_assault",
		"gcw_city_rebel_commando_defense"

	};
	
	public static final String[] eliteRebels = 
	{
		"gcw_city_rebel_specforce_assault",
		"gcw_city_rebel_specforce_defense"
	};
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setObjVar(self, gcw.GCW_DEF_TOOL_TEMPLATE_OBJVAR, "object/tangible/gcw/crafting_quest/gcw_spawner_tool.iff");
		setObjVar(self, gcw.GCW_OFF_TOOL_TEMPLATE_OBJVAR, "object/tangible/gcw/crafting_quest/gcw_patrol_tool.iff");
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		
		messageTo(self, "handleGCWPatrol", null, rand(5, 10), false);
		
		dictionary params = new dictionary();
		location loc = getLocation(self);
		params.put("particleLoc", loc);
		int playIconTime = getGameTime();
		params.put("iconMessageTime", playIconTime);
		utils.setScriptVar(self, "iconMessageTime", playIconTime);
		
		messageTo(self, "playQuestIcon", params, 2.0f, false);
		
		return SCRIPT_CONTINUE;
	};
	
	
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
		
		names[idx] = "patrol_entertained";
		
		String status = "No";
		if (utils.getBooleanScriptVar(self, "entertained"))
		{
			status = "Yes";
		}
		attribs[idx] = status;
		idx++;
		
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
			LOG("gcw_patrol_point","no faction on patrol obj");
			return SCRIPT_CONTINUE;
		}
		
		if (!factions.isPlayerSameGcwFactionAsSchedulerObject(player, self))
		{
			LOG("gcw_patrol_point","player not my faction");
			if ((!groundquests.isQuestActive(player, gcw.GCW_SPY_PATROL_SCOUT_QUEST) && !groundquests.isQuestActive(player, gcw.GCW_SPY_PATROL_DESTROY_QUEST)))
			{
				LOG("gcw_patrol_point","player doesn't have a quest that works with patrol point");
				return SCRIPT_CONTINUE;
			}
			LOG("gcw_patrol_point","OnObjectMenuRequest: Opposing player has a quest that works with patrol point");
			menu_info_data data = mi.getMenuItemByType(menu_info_types.ITEM_USE);
			
			if (data != null)
			{
				data.setServerNotify (true);
			}
		}
		else
		{
			if (utils.isProfession(player, utils.ENTERTAINER))
			{
				LOG("gcw_patrol_point","player is entertainer");
				
				if (utils.hasScriptVar(self, "entertained") && utils.getBooleanScriptVar(self, "entertained"))
				{
					sendSystemMessage(player, SID_PATROL_ALREADY_ENTERTAINED);
					return SCRIPT_CONTINUE;
				}
			}
			else if (utils.isProfession(player, utils.TRADER))
			{
				LOG("gcw_patrol_point","player is trader");
				
				if (!gcw.canGcwObjectBeRepaired(self))
				{
					LOG("gcw_patrol_point","OnObjectMenuRequest no repair needed");
					
					sendSystemMessage(player, gcw.SID_DOESNT_NEED_REPAIR);
					return SCRIPT_CONTINUE;
				}
				if (!gcw.hasConstructionOrRepairTool(player, self))
				{
					LOG("gcw_patrol_point","OnObjectMenuRequest no resources");
					
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
		
		if (item != menu_info_types.ITEM_USE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!factions.isPlayerSameGcwFactionAsSchedulerObject(player, self))
		{
			LOG("gcw_patrol_point","OnObjectMenuSelect - player not my faction");
			
			if ((!groundquests.isQuestActive(player, gcw.GCW_SPY_PATROL_SCOUT_QUEST) && !groundquests.isQuestActive(player, gcw.GCW_SPY_PATROL_DESTROY_QUEST)))
			{
				LOG("gcw_patrol_point","player doesn't have a quest that works with patrol point");
				
				return SCRIPT_CONTINUE;
			}
			
			LOG("gcw_patrol_point","OnObjectMenuSelect - Opposing player has a quest that works with patrol point");
			
			if (groundquests.isQuestActive(player, gcw.GCW_SPY_PATROL_SCOUT_QUEST))
			{
				LOG("gcw_patrol_point","gcw_spy_scout_patrol");
				
				gcw.useGcwObjectForQuest(player, self, gcw.GCW_SPY_PATROL_SCOUT_QUEST);
				
				return SCRIPT_CONTINUE;
			}
			else if (groundquests.isQuestActive(player, gcw.GCW_SPY_PATROL_DESTROY_QUEST))
			{
				LOG("gcw_patrol_point","gcw_spy_destroy_patrol");
				if (!buff.hasBuff(player, gcw.BUFF_SPY_EXPLOSIVES))
				{
					return SCRIPT_CONTINUE;
				}
				
				long counter = buff.getBuffStackCount(player, gcw.BUFF_SPY_EXPLOSIVES);
				if (counter <= 0)
				{
					return SCRIPT_CONTINUE;
				}
				
				gcw.useGcwObjectForQuest(player, self, gcw.GCW_SPY_PATROL_DESTROY_QUEST);
				
				return SCRIPT_CONTINUE;
			}
			
			LOG("gcw_patrol_point","could not find a valid quest!!!");
			
			return SCRIPT_CONTINUE;
		}
		else
		{
			if (utils.isProfession(player, utils.ENTERTAINER))
			{
				LOG("gcw_patrol_point","is entertainer");
				
				if (utils.hasScriptVar(self, "entertained") && utils.getBooleanScriptVar(self, "entertained"))
				{
					sendSystemMessage(player, SID_PATROL_ALREADY_ENTERTAINED);
					return SCRIPT_CONTINUE;
				}
				
				obj_id kit = utils.getObjIdScriptVar(self, "creator");
				
				if (!isIdValid(kit) || !exists(kit))
				{
					return SCRIPT_CONTINUE;
				}
				
				int patrolType = getIntObjVar(kit, "patrolType");
				if (patrolType == 0)
				{
					LOG("gcw_patrol_point","patrol is defensive");
					
					obj_id guardToEntertain = utils.getObjIdScriptVar(self, gcw.GCW_NPC_SCRIPTVAR_FLAG);
					if (!isValidId(guardToEntertain) || !exists(guardToEntertain))
					{
						
						sendSystemMessage(player, SID_THERE_IS_NO_GUARD);
						return SCRIPT_CONTINUE;
					}
				}
				else if (patrolType == 1)
				{
					LOG("gcw_patrol_point","patrol is offensive");
					
					if (utils.hasScriptVar(self, gcw.GCW_NPC_BEING_ENTERTAINED))
					{
						LOG("gcw_patrol_point","GCW_NPC_BEING_ENTERTAINED scriptvar found");
						
						obj_id npcBeingEntertained = utils.getObjIdScriptVar(self, gcw.GCW_NPC_BEING_ENTERTAINED);
						if (isValidId(npcBeingEntertained) && exists(npcBeingEntertained) && !ai_lib.isDead(npcBeingEntertained))
						{
							return SCRIPT_CONTINUE;
						}
					}
					
					String npcName = lowRebels[rand(0, lowRebels.length - 1)];
					if (faction == factions.FACTION_FLAG_IMPERIAL)
					{
						npcName = lowImperials[rand(0, lowImperials.length - 1)];
					}
					
					obj_id npc = create.object(npcName, getLocation(self));
					if (!isIdValid(npc) || !exists(npc))
					{
						return SCRIPT_CONTINUE;
					}
					
					utils.setScriptVar(self, gcw.GCW_NPC_BEING_ENTERTAINED, npc);
					utils.setScriptVar(npc, gcw.GCW_NPC_CLEANUP_FLAG, 1);
					setObjVar(npc, gcw.GCW_PATROL_OBJ, self);
					
					pathTo(npc, getLocation(player));
					
					chat.chat(npc, new string_id("gcw", "you_have_thirty_seconds"));
					
					dictionary webster = new dictionary();
					webster.put("npc",npc);
					webster.put("player",player);
					messageTo(self, "handleOffensiveEntertainedNpcCleanUp", webster, 30.0f, false);
				}
				
				if (groundquests.hasCompletedQuest(player, gcw.GCW_ENTERTAINER_PATROL_QUEST))
				{
					groundquests.clearQuest(player, gcw.GCW_ENTERTAINER_PATROL_QUEST);
				}
				if (!groundquests.isQuestActive(player, gcw.GCW_ENTERTAINER_PATROL_QUEST))
				{
					groundquests.grantQuest(player, gcw.GCW_ENTERTAINER_PATROL_QUEST);
				}
			}
			else if (utils.isProfession(player, utils.TRADER))
			{
				LOG("gcw_patrol_point","player is trader");
				
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
				
				if (groundquests.isQuestActive(player, gcw.GCW_REPAIR_PATROL_QUEST))
				{
					if (gcw.useGcwObjectForQuest(player, self, gcw.GCW_REPAIR_PATROL_QUEST))
					{
						gcw.useConstructionOrRepairTool(player, self);
					}
					return SCRIPT_CONTINUE;
				}
				
				if (groundquests.hasCompletedQuest(player, gcw.GCW_REPAIR_PATROL_QUEST))
				{
					groundquests.clearQuest(player, gcw.GCW_REPAIR_PATROL_QUEST);
				}
				if (!groundquests.isQuestActive(player, gcw.GCW_REPAIR_PATROL_QUEST))
				{
					groundquests.grantQuest(player, gcw.GCW_REPAIR_PATROL_QUEST);
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int getConstructionQuestsCompleted(obj_id pylon) throws InterruptedException
	{
		int completed = 0;
		
		if (!isIdValid(pylon) || !exists(pylon))
		{
			return 0;
		}
		
		if (hasObjVar(pylon, "gcw.constructionQuestsCompleted"))
		{
			completed = getIntObjVar(pylon, "gcw.constructionQuestsCompleted");
		}
		
		return completed;
	}
	
	
	public int handleGCWPatrol(obj_id self, dictionary params) throws InterruptedException
	{
		int faction = -1;
		
		if (utils.hasScriptVar(self, "faction"))
		{
			faction = utils.getIntScriptVar(self, "faction");
		}
		
		obj_id kit = utils.getObjIdScriptVar(self, "creator");
		
		if (!isIdValid(kit) || !exists(kit))
		{
			return SCRIPT_CONTINUE;
		}
		
		int patrolType = getIntObjVar(kit, "patrolType");
		int construction = getConstructionQuestsCompleted(kit);
		
		String npcName = "";
		
		if (construction < 1)
		{
			return SCRIPT_CONTINUE;
		}
		else if (construction < gcw.GCW_CONSTRUCTION_MAXIMUM / 2)
		{
			if (faction == factions.FACTION_FLAG_REBEL)
			{
				npcName = lowRebels[rand(0, lowRebels.length - 1)];
			}
			else if (faction == factions.FACTION_FLAG_IMPERIAL)
			{
				npcName = lowImperials[rand(0, lowImperials.length - 1)];
			}
		}
		else if (construction < gcw.GCW_CONSTRUCTION_MAXIMUM)
		{
			if (faction == factions.FACTION_FLAG_REBEL)
			{
				npcName = normalRebels[rand(0, normalRebels.length - 1)];
			}
			else if (faction == factions.FACTION_FLAG_IMPERIAL)
			{
				npcName = normalImperials[rand(0, normalImperials.length - 1)];
			}
		}
		else if (construction >= gcw.GCW_CONSTRUCTION_MAXIMUM)
		{
			if (faction == factions.FACTION_FLAG_REBEL)
			{
				npcName = eliteRebels[rand(0, eliteRebels.length - 1)];
			}
			else if (faction == factions.FACTION_FLAG_IMPERIAL)
			{
				npcName = eliteImperials[rand(0, eliteImperials.length - 1)];
			}
		}
		
		if (getSchedulerNPCs(kit, "gcwPatrol") < 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = null;
		
		if (patrolType == 0 && getSchedulerNPCs(kit, "gcwPatrol") < 500)
		{
			obj_id currentPatrol = utils.getObjIdScriptVar(kit, "currentPatrol");
			
			if (!isIdValid(currentPatrol) || !exists(currentPatrol) || isIncapacitated(currentPatrol) || isDead(currentPatrol))
			{
				npc = trial.createSchedulerNPC(kit, self, npcName);
				utils.setScriptVar(kit, "currentPatrol", npc);
			}
			else
			{
				
				npc = currentPatrol;
			}
			
			if (isIdValid(npc) && exists(npc))
			{
				setBaseRunSpeed(npc, 0);
				setBaseWalkSpeed(npc, 0);
			}
		}
		
		if (patrolType == 1 && getSchedulerNPCs(kit, "gcwPatrol") < 500)
		{
			npc = trial.createSchedulerNPC(kit, self, npcName);
		}
		
		setObjVar(npc, gcw.GCW_PATROL_OBJ, self);
		if (utils.hasScriptVar(self, "entertained") && isValidId(npc))
		{
			buff.applyBuff(npc, gcw.GCW_NPC_ENTERTAINED_BUFF, -1.0f);
		}
		
		messageTo(self, "handleGCWPatrol", null, 60.0f + rand(0, 30), false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int getSchedulerNPCs(obj_id kit, String npcID) throws InterruptedException
	{
		if (!isIdValid(kit) || !exists(kit))
		{
			return -1;
		}
		
		obj_id kitParent = trial.getParent(kit);
		
		if (!isIdValid(kitParent) || !exists(kitParent))
		{
			return -1;
		}
		
		obj_id[] npcs = trial.getObjectsInInstanceBySpawnId(trial.getParent(kit), npcID);
		
		if (npcs == null)
		{
			return 0;
		}
		
		return npcs.length;
	}
	
	
	public int destroyGCWPatrol(obj_id self, dictionary params) throws InterruptedException
	{
		trial.cleanupObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleOffensiveEntertainedNpcCleanUp(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = params.getObjId("npc");
		if (!isValidId(npc) || !exists(npc))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = params.getObjId("player");
		if (isValidId(player) || exists(player))
		{
			
			if (utils.hasScriptVar(npc, "ai.listeningTo"))
			{
				obj_id entertainedBy = utils.getObjIdScriptVar(npc, "ai.listeningTo");
				if (isValidId(entertainedBy) && exists(entertainedBy))
				{
					if (entertainedBy == player)
					{
						return SCRIPT_CONTINUE;
					}
				}
			}
		}
		
		int pid = getIntObjVar(player, sui.COUNTDOWNTIMER_SUI_VAR);
		if (pid > 0)
		{
			forceCloseSUIPage(pid);
		}
		
		trial.cleanupObject(npc);
		return SCRIPT_CONTINUE;
	}
	
	
	public int flagGCWPatrolEntertained(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id kit = utils.getObjIdScriptVar(self, "creator");
		if (!isValidId(kit))
		{
			return SCRIPT_CONTINUE;
		}
		
		int construction = getConstructionQuestsCompleted(kit);
		if (construction < 1)
		{
			return SCRIPT_CONTINUE;
		}
		
		int patrolType = getIntObjVar(kit, "patrolType");
		if (patrolType == 0)
		{
			obj_id currentPatrol = utils.getObjIdScriptVar(kit, "currentPatrol");
			if (isValidId(currentPatrol) && exists(currentPatrol))
			{
				buff.applyBuff(currentPatrol, gcw.GCW_NPC_ENTERTAINED_BUFF, -1.0f);
			}
		}
		
		utils.setScriptVar(self, "entertained", true);
		messageTo(self, "removeGCWPatrolEntertained", null, ENTERTAINMENT_TIMER, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int removeGCWPatrolEntertained(obj_id self, dictionary params) throws InterruptedException
	{
		utils.removeScriptVar(self, "entertained");
		utils.removeScriptVar(self, "entertainment_locked");
		return SCRIPT_CONTINUE;
	}
	
	
	public int playQuestIcon(obj_id self, dictionary params) throws InterruptedException
	{
		gcw.playQuestIconHandler(self, params);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDeath(obj_id self, obj_id killer, obj_id corpseId) throws InterruptedException
	{
		handleDestroyPatrol(self, killer);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectDisabled(obj_id self, obj_id killer) throws InterruptedException
	{
		handleDestroyPatrol(self, killer);
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleDestroyPatrol(obj_id self, obj_id killer) throws InterruptedException
	{
		playClientEffectLoc(self, "clienteffect/combat_explosion_lair_large.cef", getLocation(self), 0);
		setInvulnerable(self, true);
		messageTo(self, "destroyGCWPatrol", null, 1f, false);
		
		gcw.gcwInvasionCreditForDestroy(killer);
		
		return;
	}
}
