package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.buff;
import script.library.city;
import script.library.utils;


public class resource extends script.base_script
{
	public resource()
	{
	}
	public static final int SURVEY_RESOLUTION_MIN = 3;
	public static final int SURVEY_RESOLUTION_MAX = 6;
	public static final int SURVEY_RESOLUTION_DEFAULT = 3;
	
	public static final int SURVEY_RANGE_MIN = 16;
	public static final int SURVEY_RANGE_MAX = 384;
	public static final int SURVEY_RANGE_DEFAULT = 64;
	
	public static final String VAR_SURVEY_BASE = "survey";
	public static final String VAR_SURVEY_CLASS = VAR_SURVEY_BASE + ".resource_class";
	
	public static final String VAR_SURVEY_RESOLUTION_MIN = VAR_SURVEY_BASE + ".resolution.min";
	public static final String VAR_SURVEY_RESOLUTION_MAX = VAR_SURVEY_BASE + ".resolution.max";
	public static final String VAR_SURVEY_RESOLUTION_VALUE = VAR_SURVEY_BASE + ".resolution.value";
	
	public static final String VAR_SURVEY_RANGE_MIN = VAR_SURVEY_BASE + ".range.min";
	public static final String VAR_SURVEY_RANGE_MAX = VAR_SURVEY_BASE + ".range.max";
	public static final String VAR_SURVEY_RANGE_VALUE = VAR_SURVEY_BASE + ".range.value";
	
	public static final String VAR_SAMPLE_STAMP = VAR_SURVEY_BASE + ".sample.stamp";
	
	public static final String HANDLER_SET_RANGE = "handleSetRange";
	public static final String HANDLER_SET_RESOLUTION = "handleSetResolution";
	
	public static final String VAR_SETTINGS_BASE = VAR_SURVEY_BASE + ".settings";
	
	public static final String VAR_SETTINGS_PLAYER = VAR_SETTINGS_BASE + ".player";
	public static final String VAR_SETTINGS_INDEX = VAR_SETTINGS_BASE + ".index";
	public static final String VAR_SETTINGS_RANGES = VAR_SETTINGS_BASE + ".ranges";
	public static final String VAR_SETTINGS_RESOLUTIONS = VAR_SETTINGS_BASE + ".resolutions";
	
	public static final String PID_NAME = "resourceSpecialEvent";
	
	public static final int CLASS_RESOURCE = 0;
	public static final int CLASS_INORGANIC = 1;
	public static final int CLASS_MINERAL = 2;
	public static final int CLASS_WATER = 3;
	public static final int CLASS_CHEMICAL = 4;
	public static final int CLASS_GAS = 5;
	public static final int CLASS_ENERGY = 6;
	public static final int CLASS_ORGANIC = 7;
	public static final int CLASS_FOOD = 8;
	public static final int CLASS_CREATURE_STRUCTURAL = 9;
	public static final int CLASS_FLORA_RESOURCE = 10;
	public static final int CLASS_FLORA_STRUCTURAL = 11;
	public static final int CLASS_ENERGY_WIND = 12;
	public static final int CLASS_ENERGY_SOLAR = 13;
	public static final int CLASS_MAX = 14;
	
	public static final String[] CLASS_NAME =
	{
		"resource",
		"inorganic",
		"mineral",
		"water",
		"chemical",
		"gas",
		"energy",
		"organic",
		"creature_food",
		"creature_structural",
		"flora_resources",
		"flora_structural",
		"energy_renewable_unlimited_wind",
		"energy_renewable_unlimited_solar"
	};
	
	public static final String RT_ORGANIC = "organic";
	public static final String RT_INORGANIC = "inorganic";
	
	public static final String RT_MINERAL = "mineral";
	
	public static final String RT_GAS = "gas";
	public static final String RT_LIQUID_VAPORS = "liquid_vapors";
	
	public static final String RT_LIQUID_WATER = "water";
	public static final String RT_LIQUID_CHEMICAL = "chemical";
	
	public static final String RT_CREATURE_RESOURCES = "creature_resources";
	
	public static final String RT_FLORA_RESOURCES = "flora_resources";
	
	public static final String RT_FLORA_FOOD = "flora_food";
	
	public static final String RT_FLORA_STRUCTURAL = "flora_structural";
	
	public static final String RT_ENERGY_WIND = "energy_renewable_unlimited_wind";
	public static final String RT_ENERGY_SOLAR = "energy_renewable_unlimited_solar";
	public static final String RT_ENERGY_GEO = "energy_renewable_site_limited_geothermal";
	
	public static final String MOD_SURVEY_ORGANIC = "survey_organic";
	public static final String MOD_SURVEY_ORGANIC_RANGE = "survey_organic_range";
	public static final String MOD_SURVEY_ORGANIC_RESOLUTION = "survey_organic_resolution";
	
	public static final String MOD_SURVEY_INORGANIC = "survey_inorganic";
	public static final String MOD_SURVEY_INORGANIC_RANGE = "survey_inorganic_range";
	public static final String MOD_SURVEY_INORGANIC_RESOLUTION = "survey_inorganic_resolution";
	
	public static final String MOD_SURVEY_SOLID = "survey_solid";
	public static final String MOD_SURVEY_SOLID_RANGE = "survey_solid_range";
	public static final String MOD_SURVEY_SOLID_RESOLUTION = "survey_solid_resolution";
	
	public static final String MOD_SURVEY_GAS = "survey_gas";
	public static final String MOD_SURVEY_GAS_RANGE = "survey_gas_range";
	public static final String MOD_SURVEY_GAS_RESOLUTION = "survey_gas_resolution";
	
	public static final String MOD_SURVEY_LIQUID = "survey_liquid";
	public static final String MOD_SURVEY_LIQUID_RANGE = "survey_liquid_range";
	public static final String MOD_SURVEY_LIQUID_RESOLUTION = "survey_liquid_resolution";
	
	public static final String MOD_SURVEY_CREATURE = "survey_creature";
	public static final String MOD_SURVEY_CREATURE_RANGE = "survey_creature_range";
	public static final String MOD_SURVEY_CREATURE_RESOLUTION = "survey_creature_resolution";
	
	public static final String MOD_SURVEY_FOOD = "survey_food";
	public static final String MOD_SURVEY_FOOD_RANGE = "survey_food_range";
	public static final String MOD_SURVEY_FOOD_RESOLUTION = "survey_food_resolution";
	
	public static final String MOD_SURVEY_FLORA_STRUCTURAL = "survey_flora_structural";
	public static final String MOD_SURVEY_FLORA_STRUCTURAL_RANGE = "survey_flora_structural_range";
	public static final String MOD_SURVEY_FLORA_STRUCTURAL_RESOLUTION = "survey_flora_structural_resolution";
	
	public static final String MOD_SURVEY_ENERGY_WIND = "survey_energy_wind";
	public static final String MOD_SURVEY_ENERGY_WIND_RANGE = "survey_energy_wind_range";
	public static final String MOD_SURVEY_ENERGY_WIND_RESOLUTION = "survey_energy_wind_resolution";
	
	public static final String MOD_SURVEY_ENERGY_SOLAR = "survey_energy_solar";
	public static final String MOD_SURVEY_ENERGY_SOLAR_RANGE = "survey_energy_solar_range";
	public static final String MOD_SURVEY_ENERGY_SOLAR_RESOLUTION = "survey_energy_solar_resolution";
	
	public static final String MOD_SURVEY_ENERGY_GEO = "survey_energy_geo_thermal";
	public static final String MOD_SURVEY_ENERGY_GEO_RANGE = "survey_energy_geo_thermal_range";
	public static final String MOD_SURVEY_ENERGY_GEO_RESOLUTION = "survey_energy_geo_thermal_resolution";
	
	public static final String MOD_SURVEY_ALL = "survey_resources_all";
	
	public static final float SAMPLE_DENSITY_THRESHOLD = 0.3f;
	public static final int BASE_HARVEST_XP = 40;
	
	public static final int TIME_SAMPLE = 18;
	
	public static final int TIME_FAIL_BONUS = 180;
	
	public static final int BASE_SAMPLE_AMOUNT = 22;
	
	public static final String STF_SURVEY = "survey";
	public static final string_id SID_WRONG_TOOL = new string_id(STF_SURVEY, "wrong_tool");
	
	public static final string_id SID_NSF_SKILL = new string_id("error_message","prose_nsf_skill");
	public static final string_id SID_NSF_SKILL_PLURAL = new string_id("error_message","prose_nsf_skill_plural");
	
	public static final string_id SID_SAMPLE_LOCATED = new string_id(STF_SURVEY, "sample_located");
	public static final string_id SID_SAMPLE_FAILED = new string_id(STF_SURVEY, "sample_failed");
	
	public static final string_id SID_DENSITY_BELOW_THESHOLD = new string_id(STF_SURVEY, "density_below_threshold");
	public static final string_id SID_TRACE_AMOUNT = new string_id(STF_SURVEY, "trace_amount");
	public static final string_id SID_EFFICIENCY_TOO_LOW = new string_id(STF_SURVEY, "efficiency_too_low");
	public static final string_id SID_TOOL_RECHARGE_TIME = new string_id(STF_SURVEY, "tool_recharge_time");
	public static final string_id SID_NO_INV_SPACE = new string_id(STF_SURVEY, "no_inv_space");
	
	public static final string_id SID_CRITICAL_SUCCESS = new string_id(STF_SURVEY, "critical_success");
	public static final string_id SID_NODE_NOT_CLOSE = new string_id(STF_SURVEY, "node_not_close");
	public static final string_id SID_NODE_RECOVERY = new string_id(STF_SURVEY, "node_recovery");
	public static final string_id SID_GAMBLE_SUCCESS = new string_id(STF_SURVEY, "gamble_success");
	public static final string_id SID_GAMBLE_FAIL = new string_id(STF_SURVEY, "gamble_fail");
	public static final string_id SID_PET_SEARCH_SUCCESS = new string_id(STF_SURVEY, "pet_search_success");
	public static final string_id SID_PET_SEARCH_FAIL = new string_id(STF_SURVEY, "pet_search_fail");
	public static final string_id SID_GAMBLE_RARE = new string_id(STF_SURVEY, "gamble_rare");
	
	public static final string_id SID_EFFECTS_OF_RADIATION_SICKNESS = new string_id(STF_SURVEY, "effects_of_radiation_sickness");
	
	public static final string_id SID_SUI_SURVEY_TOOL_PROPERTIES_TITLE = new string_id(STF_SURVEY, "sui_survey_tool_properties_title");
	public static final string_id SID_SUI_SURVEY_TOOL_PROPERTIES_HEADER = new string_id(STF_SURVEY, "sui_survey_tool_properties_header");
	public static final string_id SID_SUI_SURVEY_TOOL_PROPERTIES_RESOURCE_HEADER = new string_id(STF_SURVEY, "sui_survey_tool_properties_resource_header");
	public static final string_id SID_SUI_SURVEY_TOOL_PROPERTIES_RESOURCE = new string_id(STF_SURVEY, "sui_survey_tool_properties_resource");
	public static final string_id SID_SUI_SURVEY_TOOL_PROPERTIES_RESOLUTION = new string_id(STF_SURVEY, "sui_survey_tool_properties_resolution");
	public static final string_id SID_SUI_SURVEY_TOOL_PROPERTIES_RESOLUTION_MIN = new string_id(STF_SURVEY, "sui_survey_tool_properties_resolution_min");
	public static final string_id SID_SUI_SURVEY_TOOL_PROPERTIES_RESOLUTION_MAX = new string_id(STF_SURVEY, "sui_survey_tool_properties_resolution_max");
	public static final string_id SID_SUI_SURVEY_TOOL_PROPERTIES_RESOLUTION_VAL = new string_id(STF_SURVEY, "sui_survey_tool_properties_resolution_val");
	public static final string_id SID_SUI_SURVEY_TOOL_PROPERTIES_RANGE = new string_id(STF_SURVEY, "sui_survey_tool_properties_range");
	public static final string_id SID_SUI_SURVEY_TOOL_PROPERTIES_RANGE_MIN = new string_id(STF_SURVEY, "sui_survey_tool_properties_range_min");
	public static final string_id SID_SUI_SURVEY_TOOL_PROPERTIES_RANGE_MAX = new string_id(STF_SURVEY, "sui_survey_tool_properties_range_max");
	public static final string_id SID_SUI_SURVEY_TOOL_PROPERTIES_RANGE_VAL = new string_id(STF_SURVEY, "sui_survey_tool_properties_range_val");
	
	public static final string_id SID_SURVEY_ERROR = new string_id("error_message","survey_error");
	
	public static final String DATATABLE_RESOURCES = "datatables/resource/resource_tree.iff";
	public static final String DATATABLE_COL_ENUM = "Enum";
	public static final String DATATABLE_COL_RESOURCE_CRATE_TYPE = "Resource Container Type";
	
	public static final String DEFAULT_CONTAINER = "object/resource_container/simple.iff";
	
	public static final int CONTAINER_VOLUME_MAX = 100000;
	
	public static final String MSG_SELECT_RANGE = "@"+ STF_SURVEY + ":select_range";
	public static final String MSG_SELECT_CLASS = "@"+ STF_SURVEY + ":select_class";
	
	public static final int SAMPLE_STOP_LOOP = 0;
	public static final int SAMPLE_CONTINUE_LOOP = 1;
	public static final int SAMPLE_CONTINUE_LOOP_NOSAMPLE = 2;
	public static final int SAMPLE_PAUSE_LOOP_EVENT = 3;
	
	
	public static boolean isResourceDerivedFrom(String resource_type, String parent_class) throws InterruptedException
	{
		if ((resource_type.equals("")) || (parent_class.equals("")))
		{
			return false;
		}
		
		if (resource_type.equals(parent_class))
		{
			return true;
		}
		
		obj_id resourceId = getResourceTypeByName(resource_type);
		if ((resourceId == null) || (resourceId == obj_id.NULL_ID))
		{
			return false;
		}
		
		if (isResourceDerivedFrom(resourceId, parent_class))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static obj_id[] create(obj_id resourceId, int amt, obj_id targetContainer, obj_id playerId) throws InterruptedException
	{
		if ((resourceId == null) || (resourceId == obj_id.NULL_ID))
		{
			return null;
		}
		
		if ((targetContainer == null) || (targetContainer == obj_id.NULL_ID))
		{
			return null;
		}
		
		if (amt < 1)
		{
			return null;
		}
		
		int total = amt;
		
		Vector ret = new Vector();
		ret.setSize(0);
		
		String containerTemplate = getResourceContainerTemplate(resourceId);
		if ((containerTemplate == null) || (containerTemplate.equals("")))
		{
			containerTemplate = DEFAULT_CONTAINER;
		}
		
		int chunks = (int)(amt / CONTAINER_VOLUME_MAX);
		if (amt % CONTAINER_VOLUME_MAX > 0)
		{
			chunks++;
		}
		
		for (int i = 0; i < chunks; i++)
		{
			testAbortScript();
			if (total > 0)
			{
				obj_id crate = createObject(containerTemplate, targetContainer, "");
				if ((crate == null) || (crate == obj_id.NULL_ID))
				{
				}
				else
				{
					int containerAmount = CONTAINER_VOLUME_MAX;
					if (total < CONTAINER_VOLUME_MAX)
					{
						containerAmount = total;
					}
					
					addResourceToContainer(crate, resourceId, containerAmount, playerId);
					total -= containerAmount;
					
					ret = utils.addElement(ret, crate);
				}
			}
		}
		
		if ((ret == null) || (ret.size() == 0))
		{
			return null;
		}
		
		obj_id[] _ret = new obj_id[0];
		if (ret != null)
		{
			_ret = new obj_id[ret.size()];
			ret.toArray(_ret);
		}
		return _ret;
	}
	
	
	public static obj_id[] create(obj_id resourceId, int amt, obj_id targetContainer) throws InterruptedException
	{
		return create(resourceId, amt, targetContainer, obj_id.NULL_ID);
	}
	
	
	public static obj_id[] createRandom(String resourceClass, int amt, location loc, obj_id targetContainer, obj_id playerId, int min) throws InterruptedException
	{
		if ((resourceClass == null) || (resourceClass.equals("")))
		{
			return null;
		}
		
		if (loc == null)
		{
			return null;
		}
		
		if (min < 1)
		{
			min = 1;
		}
		
		if (!isIdValid(targetContainer))
		{
			return null;
		}
		
		if (getVolumeFree(targetContainer) <= 0)
		{
			return null;
		}
		
		obj_id resourceId = null;
		
		if ((loc == null || loc.compareTo(new location()) == 0 || (loc.cell != null && loc.cell != obj_id.NULL_ID)) && isIdValid(targetContainer))
		{
			obj_id topmost = getTopMostContainer(targetContainer);
			if (topmost != targetContainer)
			{
				loc = getLocation(topmost);
			}
			
			resourceId = pickRandomNonDepeletedResource(resourceClass);
			if ((resourceId == null) || (resourceId == obj_id.NULL_ID))
			{
				return null;
			}
		}
		else
		{
			
			resource_density[] availableResources = requestResourceList(loc, 0.0f, 1.0f, resourceClass);
			
			if (availableResources == null || availableResources.length == 0)
			{
				return null;
			}
			
			int idx = rand(0, availableResources.length-1);
			resourceId = availableResources[idx].getResourceType();
			if ((resourceId == null) || (resourceId == obj_id.NULL_ID))
			{
				return null;
			}
		}
		
		float eff = getResourceEfficiency( resourceId, loc );
		
		int finalAmount = oneTimeHarvest( resourceId, amt, loc );
		if (finalAmount < min)
		{
			finalAmount = min;
		}
		
		utils.setScriptVar( playerId, "resource.lastEff", eff );
		utils.setScriptVar( playerId, "resource.lastAmt", finalAmount );
		utils.setScriptVar( playerId, "resource.lastId", resourceId );
		
		return create( resourceId, finalAmount, targetContainer, playerId );
	}
	
	
	public static obj_id[] createRandom(String resourceClass, int amt, location loc, obj_id targetContainer, obj_id playerId) throws InterruptedException
	{
		return createRandom(resourceClass, amt, loc, targetContainer, playerId, 1);
	}
	
	
	public static obj_id[] createRandom(String resourceClass, int amt, location loc, obj_id targetContainer) throws InterruptedException
	{
		return createRandom(resourceClass, amt, loc, targetContainer, obj_id.NULL_ID, 1);
	}
	
	
	public static boolean setToolClass(obj_id tool, int resource_class) throws InterruptedException
	{
		if (tool == null)
		{
			return false;
		}
		
		if ((resource_class < CLASS_RESOURCE) || (resource_class >= CLASS_MAX))
		{
			return false;
		}
		
		return setObjVar(tool, VAR_SURVEY_CLASS, CLASS_NAME[resource_class]);
	}
	
	
	public static boolean requestSetToolRatio(obj_id user, obj_id tool) throws InterruptedException
	{
		if ((user == null) || (tool == null))
		{
			return false;
		}
		
		String resource_class = getStringObjVar(tool, resource.VAR_SURVEY_CLASS);
		if (resource_class.equals(""))
		{
			sendSystemMessageTestingOnly(user, "ERROR: survey tool broken -> no assigned resource class");
			return false;
		}
		
		String baseMod = getSkillModForClass(resource_class);
		if (baseMod.equals(""))
		{
			sendSystemMessageTestingOnly(user, "ERROR: survey tool broken -> unknown resource class");
			return false;
		}
		
		int rangeVal = getSkillStatMod(user, "surveying") / 20;
		if (rangeVal < 1)
		{
			rangeVal = 1;
		}
		int resVal = getSkillStatMod(user, "surveying") / 20;
		
		int range_min = getIntObjVar(tool, VAR_SURVEY_RANGE_MIN);
		int range_max = getIntObjVar(tool, VAR_SURVEY_RANGE_MAX);
		
		int res_min = getIntObjVar(tool, VAR_SURVEY_RESOLUTION_MIN);
		int res_max = getIntObjVar(tool, VAR_SURVEY_RESOLUTION_MAX);
		
		Vector options = new Vector();
		options.setSize(0);
		Vector ranges = new Vector();
		ranges.setSize(0);
		Vector resolutions = new Vector();
		resolutions.setSize(0);
		float res = SURVEY_RESOLUTION_MIN;
		int count = 1;
		for (int range = range_min; range <= range_max; range += ((range_max-range_min)/5))
		{
			testAbortScript();
			if (count > rangeVal)
			{
				break;
			}
			res += (res_max - res_min) / 5.0;
			ranges = utils.addElement(ranges, range);
			resolutions = utils.addElement(resolutions, (int) res);
			options = utils.addElement(options, ""+ range + "m x "+ ((int) res) + "pts");
			count++;
		}
		
		if (ranges.size() == 0 || resolutions.size() == 0)
		{
			
			return false;
		}
		
		removeObjVar(tool, VAR_SETTINGS_BASE);
		
		setObjVar(tool, VAR_SETTINGS_PLAYER, user);
		setObjVar(tool, VAR_SETTINGS_RANGES, ranges, resizeableArrayTypeint);
		setObjVar(tool, VAR_SETTINGS_RESOLUTIONS, resolutions, resizeableArrayTypeint);
		
		sui.listbox(tool, user, MSG_SELECT_RANGE, options, HANDLER_SET_RANGE);
		
		return true;
	}
	
	
	public static int getSample(obj_id user, obj_id tool, String type) throws InterruptedException
	{
		if ((user == null) || (tool == null) || (type.equals("")))
		{
			return SAMPLE_STOP_LOOP;
		}
		
		obj_id typeId = getResourceTypeByName(type);
		if (!isIdValid(typeId))
		{
			debugSpeakMsg(user, "resource::getSample: unable to retrieve obj_id for "+ type);
			sendSystemMessage(user, SID_SURVEY_ERROR);
			return SAMPLE_STOP_LOOP;
		}
		
		String resource_class = getStringObjVar(tool, resource.VAR_SURVEY_CLASS);
		if (resource_class == null || resource_class.equals(""))
		{
			sendSystemMessageTestingOnly(user, "ERROR: survey tool broken -> no assigned resource class");
			return SAMPLE_STOP_LOOP;
		}
		if (!isResourceDerivedFrom(typeId, resource_class))
		{
			prose_package pp = prose.getPackage(SID_WRONG_TOOL, type);
			sendSystemMessageProse(user, pp);
			return SAMPLE_STOP_LOOP;
		}
		
		float density = getResourceEfficiency(typeId, getLocation(user));
		
		if (density < 0)
		{
			return SAMPLE_STOP_LOOP;
		}
		else if (density < 0.1f)
		{
			prose_package proseFailed = prose.getPackage(SID_EFFICIENCY_TOO_LOW, type);
			sendSystemMessageProse(user, proseFailed);
			return SAMPLE_STOP_LOOP;
		}
		
		int modVal = getSkillStatMod(user, "surveying");
		
		float threshold = SAMPLE_DENSITY_THRESHOLD * ((100f - modVal)/100f);
		if (density > threshold)
		{
			float deltaDensity = density - threshold;
			
			float famt = BASE_SAMPLE_AMOUNT * deltaDensity;
			
			if (famt < 1)
			{
				prose_package proseFailed = prose.getPackage(SID_TRACE_AMOUNT, type);
				sendSystemMessageProse(user, proseFailed);
				return SAMPLE_STOP_LOOP;
			}
			
			location nodecritloc = null;
			int gamble = 0;
			if (utils.hasScriptVar( user, "survey_event.tool" ))
			{
				if (utils.hasScriptVar( user, "survey_event.gamble" ))
				{
					
					gamble = utils.getIntScriptVar( user, "survey_event.gamble");;
					utils.removeScriptVar( user, "survey_event.gamble");
					utils.removeScriptVar( user, "survey_event.tool");
				}
				else
				{
					
					obj_id crittool = utils.getObjIdScriptVar( user, "survey_event.tool");
					if (crittool != tool)
					{
						utils.removeScriptVar( user, "survey_event.tool");
					}
					else
					{
						nodecritloc = utils.getLocationScriptVar( user, "survey_event.location");
						
						location ploc = getLocation( user );
						if (utils.getDistance2D( ploc, nodecritloc ) > 5)
						{
							utils.removeScriptVar( user, "survey_event.tool");
							utils.removeScriptVar( user, "survey_event.location");
							nodecritloc = null;
							sendSystemMessage( user, SID_NODE_NOT_CLOSE );
						}
					}
				}
			}
			
			int city_id = city.checkCity( user, false );
			
			float chance = 50f + 20f*((modVal-15f)/85f);
			if (chance > 70)
			{
				chance = 70;
			}
			int roll = rand( 1, 100 );
			
			if (city_id > 0 && city.cityHasSpec( city_id, city.SF_SPEC_SAMPLE_RICH ))
			{
				chance += 10;
			}
			
			if ((roll <= chance) || (gamble > 0) || (nodecritloc != null))
			{
				float resultModifier = ((2*chance) - roll)/(2*chance);
				int amt = (int)(famt * resultModifier);
				if (amt == 0)
				{
					amt = 1;
				}
				
				if (city_id > 0 && city.cityHasSpec( city_id, city.SF_SPEC_SAMPLE_RICH ))
				{
					amt *= 1.2;
				}
				
				if (gamble == 1)
				{
					sendSystemMessage( user, SID_GAMBLE_SUCCESS );
					amt *= 4;
				}
				else if (gamble == 2)
				{
					sendSystemMessage( user, SID_GAMBLE_FAIL );
				}
				else if (gamble == 3)
				{
					sendSystemMessage( user, SID_PET_SEARCH_SUCCESS );
					amt *= 3;
				}
				else if (gamble == 4)
				{
					sendSystemMessage( user, SID_PET_SEARCH_FAIL);
				}
				else if (gamble == 5)
				{
					sendSystemMessage( user, SID_GAMBLE_RARE);
				}
				else if (nodecritloc != null)
				{
					
					sendSystemMessage( user, SID_NODE_RECOVERY );
					amt *= 3;
					utils.removeScriptVar( user, "survey_event.location");
					utils.removeScriptVar( user, "survey_event.tool");
				}
				else
				{
					
					int critRoll = rand( 1, 100 );
					int rollResult = 10;
					if (isGod (user))
					{
						rollResult = 100;
					}
					if (roll <= rollResult)
					{
						critRoll = rand( 1, 100 );
						if (critRoll <= 50)
						{
							
							sendSystemMessage( user, SID_CRITICAL_SUCCESS );
							amt *= 2;
						}
						else if (critRoll <= 75)
						{
							
							utils.setScriptVar( user, "survey_event.tool", tool );
							
							obj_id beastCheck = (beast_lib.getBeastOnPlayer(user));
							if (beastCheck != null)
							{
								
								String[] nodeOptions = new String[3];
								nodeOptions[0] = "@survey:cnode_1";
								nodeOptions[1] = "@survey:cnode_2";
								nodeOptions[2] = "@survey:cnode_collection";
								
								if (!sui.hasPid(user, PID_NAME))
								{
									int pid = sui.listbox( user, user, "@survey:cnode_d", sui.OK_CANCEL, "@survey:cnode_t", nodeOptions, "handleSurveyNodeChoice", true );
									sui.setPid(user, pid , PID_NAME);
								}
								
							}
							
							else
							{
								
								String[] nodeOptions = new String[2];
								nodeOptions[0] = "@survey:cnode_1";
								nodeOptions[1] = "@survey:cnode_2";
								
								if (!sui.hasPid(user, PID_NAME))
								{
									int pid = sui.listbox( user, user, "@survey:cnode_d", sui.OK_CANCEL, "@survey:cnode_t", nodeOptions, "handleSurveyNodeChoice", true );
									sui.setPid(user, pid , PID_NAME);
								}
							}
							
							return SAMPLE_PAUSE_LOOP_EVENT;
						}
						else
						{
							
							utils.setScriptVar( user, "survey_event.tool", tool );
							
							String collectionName = "col_resource_"+ resource_class + "_01";
							if (!hasCompletedCollectionSlot(user, collectionName))
							{
								String[] nodeOptions = new String[3];
								nodeOptions[0] = "@survey:gnode_1";
								nodeOptions[1] = "@survey:gnode_2";
								nodeOptions[2] = "@survey:gnode_collection";
								
								if (!sui.hasPid(user, PID_NAME))
								{
									int pid = sui.listbox( user, user, "@survey:gnode_d", sui.OK_CANCEL, "@survey:gnode_t", nodeOptions, "handleSurveyGambleChoice", true );
									sui.setPid(user, pid , PID_NAME);
								}
								
							}
							
							else
							{
								
								String[] nodeOptions = new String[2];
								nodeOptions[0] = "@survey:gnode_1";
								nodeOptions[1] = "@survey:gnode_2";
								
								if (!sui.hasPid(user, PID_NAME))
								{
									int pid = sui.listbox( user, user, "@survey:gnode_d", sui.OK_CANCEL, "@survey:gnode_t", nodeOptions, "handleSurveyGambleChoice", true );
									sui.setPid(user, pid , PID_NAME);
								}
							}
							
							return SAMPLE_PAUSE_LOOP_EVENT;
						}
					}
				}
				
				int expertiseResourceIncrease = (int)getSkillStatisticModifier(user, "expertise_resource_sampling_increase");
				
				if (expertiseResourceIncrease > 0)
				{
					amt += (int)(amt * (float)(expertiseResourceIncrease / 100.0f));
				}
				
				if (buff.hasBuff(user, "tcg_series4_falleens_fist"))
				{
					amt = (int)(amt * 1.5f);
					LOG("sissynoid", "Granting 50% Increase due to Falleen's Fist Buff");
				}
				
				String crateTemplate = getResourceContainerTemplate(typeId);
				if (!crateTemplate.equals(""))
				{
					obj_id pInv = utils.getInventoryContainer(user);
					if (pInv != null)
					{
						obj_id crate = createObject( crateTemplate, pInv, "");
						
						if (addResourceToContainer( crate, typeId, amt, user ))
						{
							location curloc = getLocation( user );
							setLocation( crate, curloc );
							putIn( crate, pInv, user );
							
							if (hasScript(user, "theme_park.new_player.new_player"))
							{
								dictionary webster = new dictionary();
								webster.put("sampling", 1);
								messageTo(user, "handleNewPlayerArtisanAction", webster, 1, false);
							}
							
							if (hasScript (user, "toad.survey_quest"))
							{
								dictionary merriam = new dictionary();
								merriam.put ("resource", typeId);
								messageTo (user, "forceSensitiveQuestInfo", merriam, 1, false);
							}
							
							prose_package proseSuccess = prose.getPackage( SID_SAMPLE_LOCATED, type, amt );
							sendSystemMessageProse( user, proseSuccess );
							
							if (isResourceDerivedFrom(typeId, "radioactive"))
							{
								
								int pe = resource.getResourceAttribute(typeId, "res_potential_energy");
								
								if (pe > 500)
								{
									int damage = (pe - 500) / 2;
									if (damage < 1)
									{
										damage = 1;
									}
									
									int current = getAttrib(user, HEALTH);
									if (damage > current)
									{
										damage = (current - 1);
									}
									
									addAttribModifier(user, HEALTH, (damage * -1), 0, 0, MOD_POOL);
									
									int fatigue = damage / 4;
									addShockWound(user, fatigue);
									
									sendSystemMessage(user, SID_EFFECTS_OF_RADIATION_SICKNESS);
								}
							}
							
							return SAMPLE_CONTINUE_LOOP;
						}
						else
						{
							setObjVar( user, "sampleCrateGenFailed", 1 );
							sendSystemMessage( user, SID_NO_INV_SPACE );
							setObjVar(user, VAR_SAMPLE_STAMP, getGameTime()- TIME_FAIL_BONUS );
							return SAMPLE_STOP_LOOP;
						}
					}
				}
			}
		}
		else
		{
			prose_package proseFailed = prose.getPackage( SID_DENSITY_BELOW_THESHOLD, type );
			sendSystemMessageProse( user, proseFailed );
			return SAMPLE_STOP_LOOP;
		}
		
		setObjVar(user, VAR_SAMPLE_STAMP, getGameTime()- TIME_FAIL_BONUS );
		
		prose_package proseFailed = prose.getPackage( SID_SAMPLE_FAILED, type );
		sendSystemMessageProse( user, proseFailed );
		
		return SAMPLE_CONTINUE_LOOP_NOSAMPLE;
	}
	
	
	public static String getResourceContainerTemplate(obj_id typeId) throws InterruptedException
	{
		if (typeId == null)
		{
			return "";
		}
		
		String tpf = getResourceContainerForType(typeId);
		if (tpf.equals(""))
		{
			return DEFAULT_CONTAINER;
		}
		
		return tpf;
	}
	
	
	public static boolean showToolProperties(obj_id target, obj_id tool) throws InterruptedException
	{
		if ((target == null) || (tool == null))
		{
			return false;
		}
		
		Vector dsrc = new Vector();
		dsrc.setSize(0);
		
		String resource_class = getStringObjVar(tool, resource.VAR_SURVEY_CLASS);
		dsrc = utils.addElement(dsrc, utils.packStringId(SID_SUI_SURVEY_TOOL_PROPERTIES_RESOURCE_HEADER));
		prose_package ppResource = prose.getPackage(SID_SUI_SURVEY_TOOL_PROPERTIES_RESOURCE);
		prose.setTO(ppResource, resource_class);
		dsrc = utils.addElement(dsrc, " \0"+ packOutOfBandProsePackage(null, ppResource));
		
		dsrc = utils.addElement(dsrc, " ");
		
		int res_min = getIntObjVar(tool, resource.VAR_SURVEY_RESOLUTION_MIN);
		int res_max = getIntObjVar(tool, resource.VAR_SURVEY_RESOLUTION_MAX);
		int res_val = getIntObjVar(tool, resource.VAR_SURVEY_RESOLUTION_VALUE);
		dsrc = utils.addElement(dsrc, utils.packStringId(SID_SUI_SURVEY_TOOL_PROPERTIES_RESOLUTION));
		prose_package ppResMin = prose.getPackage(SID_SUI_SURVEY_TOOL_PROPERTIES_RESOLUTION_MIN);
		prose.setDI(ppResMin, res_min);
		dsrc = utils.addElement(dsrc, " \0"+ packOutOfBandProsePackage(null, ppResMin));
		prose_package ppResMax = prose.getPackage(SID_SUI_SURVEY_TOOL_PROPERTIES_RESOLUTION_MAX);
		prose.setDI(ppResMax, res_max);
		dsrc = utils.addElement(dsrc, " \0"+ packOutOfBandProsePackage(null, ppResMax));
		prose_package ppResVal = prose.getPackage(SID_SUI_SURVEY_TOOL_PROPERTIES_RESOLUTION_VAL);
		prose.setDI(ppResVal, res_val);
		dsrc = utils.addElement(dsrc, " \0"+ packOutOfBandProsePackage(null, ppResVal));
		
		dsrc = utils.addElement(dsrc, " ");
		
		int range_min = getIntObjVar(tool, resource.VAR_SURVEY_RANGE_MIN);
		int range_max = getIntObjVar(tool, resource.VAR_SURVEY_RANGE_MAX);
		int range_val = getIntObjVar(tool, resource.VAR_SURVEY_RANGE_VALUE);
		dsrc = utils.addElement(dsrc, utils.packStringId(SID_SUI_SURVEY_TOOL_PROPERTIES_RANGE));
		prose_package ppRangeMin = prose.getPackage(SID_SUI_SURVEY_TOOL_PROPERTIES_RANGE_MIN);
		prose.setDI(ppRangeMin, range_min);
		dsrc = utils.addElement(dsrc, " \0"+ packOutOfBandProsePackage(null, ppRangeMin));
		prose_package ppRangeMax = prose.getPackage(SID_SUI_SURVEY_TOOL_PROPERTIES_RANGE_MAX);
		prose.setDI(ppRangeMax, range_max);
		dsrc = utils.addElement(dsrc, " \0"+ packOutOfBandProsePackage(null, ppRangeMax));
		prose_package ppRangeVal = prose.getPackage(SID_SUI_SURVEY_TOOL_PROPERTIES_RANGE_VAL);
		prose.setDI(ppRangeVal, range_val);
		dsrc = utils.addElement(dsrc, " \0"+ packOutOfBandProsePackage(null, ppRangeVal));
		
		if ((dsrc != null) && (dsrc.size() > 0))
		{
			String title = utils.packStringId(SID_SUI_SURVEY_TOOL_PROPERTIES_TITLE);
			String header = utils.packStringId(SID_SUI_SURVEY_TOOL_PROPERTIES_HEADER);
			sui.listbox(target, title, header, dsrc);
			return true;
		}
		
		return false;
	}
	
	
	public static String getSkillModForClass(String resource_class) throws InterruptedException
	{
		if (resource_class.equals("resource"))
		{
			return MOD_SURVEY_ALL;
		}
		
		if (resource_class.equals(""))
		{
			return "";
		}
		
		if (isFloraResource(resource_class))
		{
			return MOD_SURVEY_FLORA_STRUCTURAL;
		}
		
		if (isCreatureResource(resource_class))
		{
			return MOD_SURVEY_CREATURE;
		}
		
		if (isLiquid(resource_class))
		{
			return MOD_SURVEY_LIQUID;
		}
		
		if (isGas(resource_class))
		{
			return MOD_SURVEY_GAS;
		}
		
		if (isSolid(resource_class))
		{
			return MOD_SURVEY_SOLID;
		}
		
		if (isEnergyWindResource(resource_class))
		{
			return MOD_SURVEY_ENERGY_WIND;
		}
		
		if (isEnergySolarResource(resource_class))
		{
			return MOD_SURVEY_ENERGY_SOLAR;
		}
		
		if (isEnergyGeoThermal(resource_class))
		{
			return MOD_SURVEY_ENERGY_GEO;
		}
		
		if (isInorganic(resource_class))
		{
			return MOD_SURVEY_INORGANIC;
		}
		
		if (isOrganic(resource_class))
		{
			return MOD_SURVEY_ORGANIC;
		}
		
		return "";
	}
	
	
	public static boolean isEnergyGeoThermal(String resource_class) throws InterruptedException
	{
		if (resource_class.equals(""))
		{
			return false;
		}
		
		if (isResourceClassDerivedFrom(resource_class, RT_ENERGY_GEO))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isOrganic(String resource_class) throws InterruptedException
	{
		if (resource_class.equals(""))
		{
			return false;
		}
		
		if (isResourceClassDerivedFrom(resource_class, RT_ORGANIC))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isInorganic(String resource_class) throws InterruptedException
	{
		if (resource_class.equals(""))
		{
			return false;
		}
		
		if (isResourceClassDerivedFrom(resource_class, RT_INORGANIC))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isSolid(String resource_class) throws InterruptedException
	{
		if (resource_class.equals(""))
		{
			return false;
		}
		
		if (isResourceClassDerivedFrom(resource_class, RT_MINERAL))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isGas(String resource_class) throws InterruptedException
	{
		if (resource_class.equals(""))
		{
			return false;
		}
		
		if (isResourceClassDerivedFrom(resource_class, RT_GAS))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isLiquid(String resource_class) throws InterruptedException
	{
		if (resource_class.equals(""))
		{
			return false;
		}
		
		if (isResourceClassDerivedFrom(resource_class, RT_LIQUID_WATER))
		{
			return true;
		}
		
		if (isResourceClassDerivedFrom(resource_class, RT_LIQUID_CHEMICAL))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isCreatureResource(String resource_class) throws InterruptedException
	{
		if (resource_class.equals(""))
		{
			return false;
		}
		
		if (isResourceClassDerivedFrom(resource_class, RT_CREATURE_RESOURCES))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isFloraResource(String resource_class) throws InterruptedException
	{
		if (resource_class.equals(""))
		{
			return false;
		}
		
		if (isResourceClassDerivedFrom(resource_class, RT_FLORA_RESOURCES))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isFloraFoodResource(String resource_class) throws InterruptedException
	{
		if (resource_class.equals(""))
		{
			return false;
		}
		
		if (isResourceClassDerivedFrom(resource_class, RT_FLORA_FOOD))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isFloraStructuralResource(String resource_class) throws InterruptedException
	{
		if (resource_class.equals(""))
		{
			return false;
		}
		
		if (isResourceClassDerivedFrom(resource_class, RT_FLORA_STRUCTURAL))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isEnergyWindResource(String resource_class) throws InterruptedException
	{
		if (resource_class.equals(""))
		{
			return false;
		}
		
		if (isResourceClassDerivedFrom(resource_class, RT_ENERGY_WIND))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isEnergySolarResource(String resource_class) throws InterruptedException
	{
		if (resource_class.equals(""))
		{
			return false;
		}
		
		if (isResourceClassDerivedFrom(resource_class, RT_ENERGY_SOLAR))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static String getResourceName(String resource_class) throws InterruptedException
	{
		int rowNum = dataTableSearchColumnForString( resource_class, 1, DATATABLE_RESOURCES );
		if (rowNum < 0)
		{
			return "";
		}
		
		dictionary row = dataTableGetRow( DATATABLE_RESOURCES, rowNum );
		
		int i = 0;
		String res = null;
		while ((res == null) || (res.equals("")))
		{
			testAbortScript();
			res = row.getString( "CLASS "+ i++ );
		}
		return res;
	}
	
	
	public static int getEnergyTotalOnTarget(obj_id target) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return -1;
		}
		
		int cnt = 0;
		
		obj_id[] items = getInventoryAndEquipment(target);
		if ((items != null) && (items.length > 0))
		{
			for (int i = 0; i < items.length; i++)
			{
				testAbortScript();
				if (isGameObjectTypeOf(getGameObjectType(items[i]), GOT_resource_container))
				{
					obj_id rId = getResourceContainerResourceType(items[i]);
					if (isIdValid(rId))
					{
						if (isResourceDerivedFrom(rId, "energy") || isResourceDerivedFrom(rId, "radioactive"))
						{
							cnt += getPotentialEnergyValue(items[i]);
						}
					}
				}
			}
		}
		
		return cnt;
	}
	
	
	public static int getPotentialEnergyValue(obj_id crate) throws InterruptedException
	{
		if (!isIdValid(crate))
		{
			return 0;
		}
		
		int amt = getResourceContainerQuantity(crate);
		if (amt < 1)
		{
			return 0;
		}
		
		obj_id rType = getResourceContainerResourceType(crate);
		if (!isIdValid(rType))
		{
			return 0;
		}
		
		float ratio = getEnergyPowerRatio(rType);
		return Math.round(ratio * amt);
		
	}
	
	
	public static float getEnergyPowerRatio(obj_id rType) throws InterruptedException
	{
		int potential = getResourceAttribute(rType, "res_potential_energy");
		if (potential < 1)
		{
			return 1f;
		}
		
		if (potential <= 500)
		{
			return 1f;
		}
		else
		{
			return 1f + (potential-500f)/500f;
		}
	}
	
	
	public static int getResourceAttribute(String resourceName, String attribName) throws InterruptedException
	{
		if (resourceName == null || resourceName.equals(""))
		{
			return -1;
		}
		
		return getResourceAttribute(getResourceTypeByName(resourceName), attribName);
	}
	
	
	public static dictionary getGeoThermalEnergyInformationOnTarget(obj_id target) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return null;
		}
		
		int cnt = 0;
		String resourceName = "";
		int resourceQuality = 0;
		
		obj_id[] items = getInventoryAndEquipment(target);
		Vector tempListUniqueList = new Vector();
		tempListUniqueList.setSize(0);
		dictionary resourceInfo = new dictionary();
		
		if ((items != null) && (items.length > 0))
		{
			for (int i = 0; i < items.length; i++)
			{
				testAbortScript();
				if (isGameObjectTypeOf(getGameObjectType(items[i]), GOT_resource_container))
				{
					obj_id resourceType = getResourceContainerResourceType(items[i]);
					if (isIdValid(resourceType))
					{
						if (isResourceDerivedFrom(resourceType, "energy_renewable_site_limited_geothermal"))
						{
							if (tempListUniqueList.size() == 0)
							{
								tempListUniqueList.addElement(resourceType);
								resourceName = getResourceName(resourceType);
								resourceQuality = getResourceAttribute(resourceType, "res_quality");
							}
							
							if (tempListUniqueList.contains(resourceType))
							{
								cnt += getResourceContainerQuantity(items[i]);
							}
						}
					}
				}
			}
		}
		
		resourceInfo.put("resourceName", resourceName);
		resourceInfo.put("cnt", cnt);
		resourceInfo.put("resourceQuality", resourceQuality);
		return resourceInfo;
	}
	
	
	public static void cleanupTool(obj_id player, obj_id tool) throws InterruptedException
	{
		if (isIdValid(player))
		{
			removeObjVar( player, "surveying.sampleLocation");
			utils.removeScriptVar(player, "surveying.takingSamples");
			utils.removeScriptVar(player, "surveying.tool");
			utils.removeScriptVar(player, "surveying.surveying");
		}
		if (isIdValid(tool))
		{
			utils.removeScriptVar(tool, "surveying.takingSamples");
			utils.removeScriptVar(tool, "surveying.player");
			utils.removeScriptVar(tool, "surveying.surveying");
		}
	}
	
	
	public static boolean consumeResource(obj_id player, String resource, int quantity) throws InterruptedException
	{
		obj_id[] contents = getInventoryAndEquipment(player);
		
		if (contents == null || contents.length < 1)
		{
			return false;
		}
		
		for (int i = 0, j = contents.length; i < j; i++)
		{
			testAbortScript();
			if (!isIdValid(contents[i]) || !exists(contents[i]))
			{
				continue;
			}
			
			if (!isResourceContainer(contents[i]))
			{
				continue;
			}
			
			obj_id container = getResourceContainerResourceType(contents[i]);
			
			if (!isIdValid(container))
			{
				continue;
			}
			
			String name = getResourceClass(container);
			String typeName = getResourceName(container);
			String parent = getResourceParentClass(name);
			parent = getResourceParentClass(parent);
			parent = getResourceParentClass(parent);
			
			if (resource.equals(parent) && getResourceContainerQuantity(contents[i]) >= quantity)
			{
				
				removeResourceFromContainer(contents[i], getResourceContainerResourceType(contents[i]), quantity);
				return true;
			}
		}
		
		return false;
	}
	
	
	public static boolean hasResource(obj_id player, String resource, int quantity) throws InterruptedException
	{
		obj_id[] contents = getInventoryAndEquipment(player);
		
		if (contents == null || contents.length < 1)
		{
			return false;
		}
		
		for (int i = 0, j = contents.length; i < j; i++)
		{
			testAbortScript();
			if (!isIdValid(contents[i]) || !exists(contents[i]))
			{
				continue;
			}
			
			if (!isResourceContainer(contents[i]))
			{
				continue;
			}
			
			obj_id container = getResourceContainerResourceType(contents[i]);
			
			if (!isIdValid(container))
			{
				continue;
			}
			
			String name = getResourceClass(container);
			String typeName = getResourceName(container);
			String parent = getResourceParentClass(name);
			parent = getResourceParentClass(parent);
			parent = getResourceParentClass(parent);
			
			if (resource.equals(parent) && getResourceContainerQuantity(contents[i]) >= quantity)
			{
				
				return true;
			}
		}
		
		return false;
	}
}
