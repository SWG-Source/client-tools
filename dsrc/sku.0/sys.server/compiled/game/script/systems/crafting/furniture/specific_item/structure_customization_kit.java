package script.systems.crafting.furniture.specific_item;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.utils;
import script.library.craftinglib;



public class structure_customization_kit extends script.systems.crafting.furniture.crafting_base_furniture_customization
{
	public structure_customization_kit()
	{
	}
	public static final String VERSION = "v0.00.00";
	
	public static final String[] REQUIRED_SKILLS =
	{
		"crafting_architect_novice"
	};
	
	public static final String[] ASSEMBLY_SKILL_MODS =
	{
		"structure_assembly"
	};
	
	public static final String[] EXPERIMENT_SKILL_MODS =
	{
		"structure_experimentation"
	};
	
	public static final resource_weight[] OBJ_ASSEMBLY_ATTRIBUTE_RESOURCES =
	{
		new resource_weight("charges", new resource_weight.weight[]
		{
			new resource_weight.weight(craftinglib.RESOURCE_DECAY_RESIST, 1),
			new resource_weight.weight(craftinglib.RESOURCE_QUALITY, 1)
		}
		)
	};
	
	public static final resource_weight[] OBJ_MAX_ATTRIBUTE_RESOURCES =
	{
		new resource_weight("charges", new resource_weight.weight[]
		{
			new resource_weight.weight(craftinglib.RESOURCE_DECAY_RESIST, 1),
			new resource_weight.weight(craftinglib.RESOURCE_QUALITY, 1)
		}
		)
	};
	
	
	public String[] getRequiredSkills() throws InterruptedException
	{
		return REQUIRED_SKILLS;
	}
	
	
	public String[] getAssemblySkillMods() throws InterruptedException
	{
		return ASSEMBLY_SKILL_MODS;
	}
	
	
	public String[] getExperimentSkillMods() throws InterruptedException
	{
		return EXPERIMENT_SKILL_MODS;
	}
	
	
	public resource_weight[] getResourceMaxResourceWeights() throws InterruptedException
	{
		return OBJ_MAX_ATTRIBUTE_RESOURCES;
	}
	
	
	public resource_weight[] getAssemblyResourceWeights() throws InterruptedException
	{
		return OBJ_ASSEMBLY_ATTRIBUTE_RESOURCES;
	}
}
