package script.systems.crafting;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.ai_lib;
import script.library.craftinglib;
import script.library.jedi_trials;
import script.library.pet_lib;
import script.library.utils;
import script.library.temp_schematic;
import script.library.session;


public class base_tool extends script.base_script
{
	public base_tool()
	{
	}
	public static final String PROTOTYPE_SLOT = "anythingNoMod2";
	
	public static final String OBJVAR_CRAFTING_FAKE_PROTOTYPE = "crafting.isFakePrototype";
	
	public static final float[] COMPLEXITY_LIMIT =
	{
		15,
		20,
		25
	};
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		
		setObjVar(self, craftinglib.OBJVAR_PROTOTYPE_TIME, 0.0f);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		
		float time = getFloatObjVar(self, craftinglib.OBJVAR_PROTOTYPE_TIME);
		if (time == 0.0f)
		{
			removeObjVar(self, craftinglib.OBJVAR_CRAFTER);
		}
		
		String template = getTemplateName(self);
		
		if (template != null && template.indexOf("weapon_tool.iff") >= 0)
		{
			int craftingType = getIntObjVar(self, craftinglib.OBJVAR_CRAFTING_TYPE);
			
			craftingType &= ~CT_genericItem;
			craftingType |= CT_misc;
			
			setObjVar(self, craftinglib.OBJVAR_CRAFTING_TYPE, craftingType);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int determineCraftingLevel(obj_id tool, obj_id player) throws InterruptedException
	{
		
		int craftingLevel = 0;
		obj_id stationId = obj_id.NULL_ID;
		debugServerConsoleMsg(tool, "OnObjectMenuRequest initial crafting level 0");
		
		int toolType = getIntObjVar(tool, craftinglib.OBJVAR_CRAFTING_TYPE);
		if (toolType == 0)
		{
			LOG("crafting", "No crafting type defined for object "+ tool);
			return -1;
		}
		
		if (toolType != CT_genericItem)
		{
			++craftingLevel;
			debugServerConsoleMsg(tool, "OnObjectMenuRequest crafting level 1");
			
			location myPos = getLocation(player);
			
			debugServerConsoleMsg(tool, "Looking for a crafting station, I think I'm at location "+ myPos);
			
			obj_id[] testIds = getObjectsInRange(myPos, craftinglib.STATION_AREA);
			if (testIds != null)
			{
				float closestLength = craftinglib.STATION_AREA + 100.0f;
				for (int i = 0; i < testIds.length; ++i)
				{
					testAbortScript();
					if (isIdValid(testIds[i]) && hasObjVar(testIds[i], craftinglib.OBJVAR_STATION))
					{
						debugServerConsoleMsg(tool, "Testing crafting station "+ testIds[i] + " at "+ getLocation(testIds[i]));
						float dist = getDistance(testIds[i], myPos);
						debugServerConsoleMsg(tool, "\tstation distance = "+ dist);
						if (dist >= 0 && dist < closestLength)
						{
							closestLength = dist;
							stationId = testIds[i];
						}
					}
				}
				
				if ((isIdValid(stationId) && ((ai_lib.getNiche(stationId) != NICHE_DROID && ai_lib.getNiche(stationId) != NICHE_ANDROID) || pet_lib.isMyPet(stationId, player))))
				{
					debugServerConsoleMsg(tool, "Found crafting station "+ stationId + " at "+ getLocation(stationId));
					
					int stationType = getIntObjVar(stationId, craftinglib.OBJVAR_CRAFTING_TYPE);
					if ((stationType & toolType) != 0)
					{
						++craftingLevel;
						debugServerConsoleMsg(tool, "OnObjectMenuRequest crafting level 2");
						
						int privateStation = getIntObjVar(stationId, craftinglib.OBJVAR_PRIVATE_STATION);
						if ((privateStation == 1 && (ai_lib.aiGetNiche(stationId) != NICHE_DROID || ai_lib.aiGetNiche(stationId) != NICHE_ANDROID)))
						{
							++craftingLevel;
							debugServerConsoleMsg(tool, "OnObjectMenuRequest crafting level 3");
						}
						else if (privateStation == 2 && (ai_lib.aiGetNiche(stationId) == NICHE_DROID || ai_lib.aiGetNiche(stationId) != NICHE_ANDROID))
						{
							craftingLevel = craftingLevel + 2;
							debugServerConsoleMsg(tool, "OnObjectMenuRequest crafting level 4");
						}
					}
					else
					{
						debugServerConsoleMsg(tool, "OnObjectMenuRequest station is wrong type "+ stationType);
					}
				}
				else
				{
					debugServerConsoleMsg(tool, "OnObjectMenuRequest no station objvar");
				}
			}
		}
		
		String template = getTemplateName(stationId);
		
		if (template != null && template.indexOf("weapon_station.iff") >= 0)
		{
			int craftingType = getIntObjVar(stationId, craftinglib.OBJVAR_CRAFTING_TYPE);
			
			craftingType &= ~CT_genericItem;
			craftingType |= CT_misc | CT_lightsaber;
			
			setObjVar(stationId, craftinglib.OBJVAR_CRAFTING_TYPE, craftingType);
		}
		
		if (!setCraftingLevelAndStation(player, craftingLevel, stationId))
		{
			debugServerConsoleMsg(tool, "OnObjectMenuRequest failed to set crafting level on player");
			craftingLevel = -1;
		}
		return craftingLevel;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		
		int craftingLevel = determineCraftingLevel(self, player);
		if (craftingLevel == -1)
		{
			return SCRIPT_OVERRIDE;
		}
		
		menu_info_data craft_menu = mi.getMenuItemByType (menu_info_types.CRAFT_START);
		if (craft_menu != null)
		{
			
			float time = getFloatObjVar(self, craftinglib.OBJVAR_PROTOTYPE_TIME);
			if (time == 0.0f && isIdValid(getObjectInSlot(self, PROTOTYPE_SLOT)))
			{
				mi.addSubMenu (craft_menu.getId(), menu_info_types.CRAFT_HOPPER_OUTPUT, null);
			}
			
			if (craftingLevel == 3)
			{
				mi.addSubMenu (craft_menu.getId(), menu_info_types.CRAFT_HOPPER_INPUT, null);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnRequestDraftSchematics(obj_id self, obj_id player, int[] schematics, float[] complexities) throws InterruptedException
	{
		debugServerConsoleMsg(self, "OnRequestDraftSchematics enter, num schematics = "+ schematics.length);
		
		if (schematics.length > 0 && schematics.length == complexities.length)
		{
			int craftingLevel = determineCraftingLevel(self, player);
			if (craftingLevel == -1)
			{
				return SCRIPT_OVERRIDE;
			}
			
			debugServerConsoleMsg(self, "OnRequestDraftSchematics crafting level = "+ craftingLevel);
			
			int[] allowedSchematics;
			if (craftingLevel < 3)
			{
				allowedSchematics = new int[schematics.length];
				for (int i = 0; i < schematics.length; ++i)
				{
					testAbortScript();
					if (complexities[i] <= COMPLEXITY_LIMIT[craftingLevel])
					{
						allowedSchematics[i] = schematics[i];
					}
					else
					{
						allowedSchematics[i] = 0;
					}
				}
			}
			else
			{
				allowedSchematics = schematics;
			}
			
			if (sendUseableDraftSchematics(player, allowedSchematics))
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		debugServerConsoleMsg(self, "OnRequestDraftSchematics exit - override");
		return SCRIPT_OVERRIDE;
	}
	
	
	public int OnAboutToReceiveItem(obj_id self, obj_id srcContainer, obj_id transferer, obj_id item) throws InterruptedException
	{
		if (transferer == obj_id.NULL_ID || transferer == srcContainer)
		{
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int OnAboutToLoseItem(obj_id self, obj_id destContainer, obj_id transferer, obj_id item) throws InterruptedException
	{
		debugServerConsoleMsg(self, "Crafting station OnAboutToLoseItem enter");
		float time = getFloatObjVar(self, craftinglib.OBJVAR_PROTOTYPE_TIME);
		if (time != 0.0f)
		{
			debugServerConsoleMsg(self, "Crafting station OnAboutToLoseItem preventing prototype from being removed");
			return SCRIPT_OVERRIDE;
		}
		debugServerConsoleMsg(self, "Crafting station OnAboutToLoseItem allowing prototype to be removed");
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAboutToOpenContainer(obj_id self, obj_id whoOpenedMe) throws InterruptedException
	{
		debugServerConsoleMsg(self, "Crafting station OnAboutToOpenContainer enter");
		float time = getFloatObjVar(self, craftinglib.OBJVAR_PROTOTYPE_TIME);
		if (time != 0.0f)
		{
			
			sendSystemMessage (whoOpenedMe, new string_id("system_msg", "cant_open_output_hopper"));
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnCraftingAddResource(obj_id self, obj_id player, obj_id resource, draft_schematic.slot ingredientSlot, modifiable_int[] resourceAmount) throws InterruptedException
	{
		int numResources = resourceAmount.length;
		int totalResources = ingredientSlot.amountRequired;
		
		int resourcesPer = totalResources / numResources;
		if (totalResources % numResources != 0)
		{
			++resourcesPer;
		}
		
		for (int i = 0; i < numResources; ++i)
		{
			testAbortScript();
			resourceAmount[i].set(resourcesPer);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnCraftingDone(obj_id self, obj_id player, String schematicName, int craftingStage, boolean normalExit) throws InterruptedException
	{
		session.logActivity(player, session.ACTIVITY_CRAFTING);
		if ((craftingStage != CS_selectDraftSchematic && craftingStage != CS_assembly && schematicName != null && schematicName.length() > 0))
		{
			temp_schematic.decrement(player, getObjectTemplateCrc(schematicName));
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		int idx = utils.getValidAttributeIndex(names);
		if (idx == -1)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		if (hasObjVar(self, "quality"))
		{
			names[idx] = "quality";
			float attrib = getFloatObjVar(self, "quality");
			attribs[idx] = " " + attrib;
			idx++;
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
		}
		int critAssembly = getIntObjVar(self, craftinglib.OBJVAR_FORCE_CRITICAL_ASSEMBLY);
		if (critAssembly > 0)
		{
			names[idx] = "@crafting:crit_assembly";
			attribs[idx] = "" + critAssembly;
			idx++;
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
		}
		int critExperiment = getIntObjVar(self, craftinglib.OBJVAR_FORCE_CRITICAL_EXPERIMENT);
		if (critExperiment > 0)
		{
			names[idx] = "@crafting:crit_experiment";
			attribs[idx] = "" + critExperiment;
			idx++;
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
		}
		
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public int prototypeDone(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id prototype = params.getObjId("prototype");
		obj_id crafter = params.getObjId("crafter");
		
		debugServerConsoleMsg(self, "prototypeDone enter, crafter = "+ crafter + ", prototype = "+ prototype);
		
		LOG("crafting_counter", "testing tool completion!");
		
		debugSpeakMsg(self, "PING! The prototype is done now.");
		setObjVar(self, craftinglib.OBJVAR_PROTOTYPE_TIME, 0.0f);
		removeObjVar(self, craftinglib.OBJVAR_CRAFTER);
		removeObjVar(self, craftinglib.OBJVAR_PROTOTYPE_START);
		removeObjVar(self, craftinglib.OBJVAR_CRAFTING_PROTOTYPE_OBJECT);
		removeObjVar(self, craftinglib.OBJVAR_CRAFTING_PROTOTYPE_CRAFTER);
		
		if (hasScript(crafter, "theme_park.new_player.new_player") || hasScript(crafter, jedi_trials.PADAWAN_TRIALS_SCRIPT))
		{
			if (isIdValid(prototype))
			{
				String templateName = getTemplateName(prototype);
				
				if (templateName != null && !templateName.equals(""))
				{
					dictionary webster = new dictionary();
					webster.put("prototype", prototype);
					webster.put("prototypeTemplate", templateName);
					messageTo(crafter, "handleQuestCraftingAction", webster, 1, false);
				}
				else
				{
					LOG("new_player", "New player "+ crafter + " successfully made prototype item "+ prototype + " but its template name is null, so the new player crafting action failed!");
				}
			}
		}
		
		endCraftingSession(crafter, self, params.getObjId("prototype"));
		
		getPrototype(self, params);
		
		logBalance( "crafting;"+getGameTime()+";"+crafter+";"+prototype+";"+getTemplateName(prototype) );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public obj_id getFirstParentInWorldOrPlayer(obj_id obj) throws InterruptedException
	{
		obj_id firstParent = getFirstParentInWorld(obj);
		while (obj != firstParent && !isPlayer(obj))
		{
			testAbortScript();
			obj = getContainedBy(obj);
		}
		return obj;
	}
	
	
	public int getPrototype(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id crafter = params.getObjId("crafter");
		obj_id owner = getFirstParentInWorldOrPlayer(self);
		
		float time = getFloatObjVar(self, craftinglib.OBJVAR_PROTOTYPE_TIME);
		if (time != 0.0f && isIdValid(owner))
		{
			sendSystemMessage (owner, new string_id("system_msg", "cant_open_output_hopper"));
			return SCRIPT_OVERRIDE;
		}
		
		if (isIdValid(owner))
		{
			obj_id inventory = getObjectInSlot(owner, utils.SLOT_INVENTORY);
			if (isIdValid(inventory))
			{
				obj_id prototype = params.getObjId("prototype");
				if (!isIdValid(prototype))
				{
					
					obj_id mightBePrototype = getObjectInSlot(self, PROTOTYPE_SLOT);
					if (!isIdValid(mightBePrototype))
					{
						return SCRIPT_CONTINUE;
					}
					prototype = mightBePrototype;
				}
				
				if (hasObjVar(prototype, OBJVAR_CRAFTING_FAKE_PROTOTYPE))
				{
					destroyObject(prototype);
				}
				else
				{
					
					if (putIn(prototype, inventory))
					{
						sendSystemMessage (owner, new string_id("system_msg", "prototype_transferred"));
					}
					else
					{
						sendSystemMessage (owner, new string_id("system_msg", "prototype_not_transferred"));
					}
				}
			}
			else
			{
				
				sendSystemMessage (owner, new string_id("system_msg", "prototype_done"));
			}
		}
		else if (isIdValid(crafter))
		{
			
			sendSystemMessage (crafter, new string_id("system_msg", "prototype_done"));
		}
		setCount(self, 0);
		return SCRIPT_CONTINUE;
	}
}
