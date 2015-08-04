package script.systems.gcw;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.camping;
import script.library.create;
import script.library.factions;
import script.library.gcw;
import script.library.theater;
import script.library.trial;
import script.library.utils;


public class gcw_camp extends script.base_script
{
	public gcw_camp()
	{
	}
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		messageTo(self, "initializeAdvancedCamp", null, 30.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int initializeAdvancedCamp(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("gcw_camp_functionality", "initializeAdvancedCamp");
		
		if (!hasObjVar(self, "factionFlag"))
		{
			return SCRIPT_CONTINUE;
		}
		
		int factionFlag = getIntObjVar(self, "factionFlag");
		if (factionFlag < 0)
		{
			return SCRIPT_CONTINUE;
		}
		LOG("gcw_camp_functionality", "initializeAdvancedCamp factionFlag: "+ factionFlag);
		
		int numNodes = camping.getAdvancedCampNumberOfNodes(self);
		if (numNodes > 6)
		{
			numNodes = 6;
		}
		LOG("gcw_camp_functionality", "initializeAdvancedCamp - numNodes: "+numNodes);
		
		obj_id[] nodeObjects = new obj_id[numNodes];
		int arrayIdx = 0;
		
		obj_var_list ovl = getObjVarList(self, "modules");
		LOG("gcw_camp_functionality", "initializeAdvancedCamp - obj_var_list: "+ovl);
		
		if (ovl == null)
		{
			LOG("gcw_camp_functionality", "initializeAdvancedCamp - ovl == null");
			return SCRIPT_CONTINUE;
		}
		LOG("gcw_camp_functionality", "initializeAdvancedCamp - ovl not null");
		
		String[] modules = ovl.getAllObjVarNames();
		if (modules == null || modules.length <= 0)
		{
			LOG("gcw_camp_functionality", "initializeAdvancedCamp - modules == null");
			return SCRIPT_CONTINUE;
		}
		int nodeCount = 1;
		int i = 0;
		int recruitersPlaced = 0;
		
		while (nodeCount <= numNodes && i < modules.length)
		{
			testAbortScript();
			if (modules[i] == null || modules[i].equals(""))
			{
				LOG("gcw_camp_functionality", "initializeAdvancedCamp - modules == null");
				continue;
			}
			
			LOG("gcw_camp_functionality", "initializeAdvancedCamp - modules[i]: "+modules[i]);
			
			location tempLoc = getLocation(self);
			location nodeLoc = camping.getAdvancedCampNodeLocation(self, tempLoc, nodeCount);
			int nodeYaw = camping.getAdvancedCampNodeYaw(self, nodeCount);
			
			String module_template = "";
			
			if (modules[i].equals("shuttle_beacon"))
			{
				module_template = "object/building/poi/player_camp_shuttle_beacon.iff";
			}
			else if (modules[i].equals("cloning_tube"))
			{
				module_template = "object/building/poi/player_camp_clone_tube.iff";
			}
			
			else if (modules[i].equals("entertainer"))
			{
				if (factionFlag == factions.FACTION_FLAG_REBEL)
				{
					LOG("gcw_camp_functionality", "initializeAdvancedCamp - Entertainment Camp is FACTION_FLAG_REBEL");
					module_template = "object/building/poi/gcw_camp_entertainment_rebel.iff";
				}
				else if (factionFlag == factions.FACTION_FLAG_IMPERIAL)
				{
					LOG("gcw_camp_functionality", "initializeAdvancedCamp - Entertainment Camp is FACTION_FLAG_IMPERIAL");
					module_template = "object/building/poi/gcw_camp_entertainment_imperial.iff";
				}
				else
				{
					LOG("gcw_camp_functionality", "initializeAdvancedCamp - Entertainment Camp is FACTION_FLAG_UNKNOWN");
					module_template = "object/building/poi/player_camp_entertainment_area.iff";
				}
			}
			
			else if (modules[i].equals("junk_dealer"))
			{
				if (factionFlag == factions.FACTION_FLAG_REBEL)
				{
					LOG("gcw_camp_functionality", "initializeAdvancedCamp - Junk Dealer is FACTION_FLAG_REBEL");
					module_template = "object/building/poi/gcw_camp_junk_dealer_rebel.iff";
				}
				else if (factionFlag == factions.FACTION_FLAG_IMPERIAL)
				{
					LOG("gcw_camp_functionality", "initializeAdvancedCamp - Junk Dealer is FACTION_FLAG_IMPERIAL");
					module_template = "object/building/poi/gcw_camp_junk_dealer_imperial.iff";
				}
				else
				{
					LOG("gcw_camp_functionality", "initializeAdvancedCamp - Junk Dealer is FACTION_FLAG_UNKNOWN");
					module_template = "object/building/poi/player_camp_junk_dealer.iff";
				}
			}
			
			else if (modules[i].equals("imperial"))
			{
				module_template = "object/building/poi/gcw_camp_imperial_recruiter.iff";
			}
			else if (modules[i].equals("rebel"))
			{
				module_template = "object/building/poi/gcw_camp_rebel_recruiter.iff";
			}
			else if (modules[i].equals("rebel_clone"))
			{
				module_template = "object/building/poi/gcw_rebel_clone_tent_small.iff";
			}
			else if (modules[i].equals("imperial_clone"))
			{
				module_template = "object/building/poi/gcw_imperial_clone_tent_small.iff";
			}
			
			else if (modules[i].equals("clothing_station"))
			{
				module_template = "object/building/poi/player_camp_crafting_clothing.iff";
			}
			else if (modules[i].equals("food_station"))
			{
				module_template = "object/building/poi/player_camp_crafting_food.iff";
			}
			else if (modules[i].equals("ship_station"))
			{
				module_template = "object/building/poi/player_camp_crafting_space.iff";
			}
			else if (modules[i].equals("structure_station"))
			{
				module_template = "object/building/poi/player_camp_crafting_structure.iff";
			}
			else if (modules[i].equals("weapon_station"))
			{
				module_template = "object/building/poi/player_camp_crafting_weapon.iff";
			}
			
			if (!module_template.equals(""))
			{
				obj_id module_id = create.object(module_template, nodeLoc);
				if (camping.isRecruiterModule(module_template))
				{
					recruitersPlaced = recruitersPlaced +1;
				}
				if (isIdValid(module_id))
				{
					setYaw(module_id, nodeYaw);
					nodeObjects[arrayIdx] = module_id;
					arrayIdx++;
					nodeCount++;
				}
			}
			
			i++;
		}
		
		LOG("gcw_camp_functionality", "initializeAdvancedCamp - Setting Vars!!");
		
		setObjVar(self, "modules.ids", nodeObjects);
		setObjVar(self, camping.VAR_CREATION_TIME, getCalendarTime());
		
		Vector children = getResizeableObjIdArrayObjVar(self, theater.VAR_CHILDREN);
		LOG("gcw_camp_functionality", "initializeAdvancedCamp - children.length: "+children.size());
		for (int b = 0; b < nodeObjects.length; b++)
		{
			testAbortScript();
			children = utils.addElement(children, nodeObjects[b]);
		}
		
		setObjVar(self, theater.VAR_CHILDREN, children, resizeableArrayTypeobj_id);
		return SCRIPT_CONTINUE;
	}
}
