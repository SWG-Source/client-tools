package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.callable;
import script.library.colors;
import script.library.money;
import script.library.pet_lib;
import script.library.hue;
import script.library.space_transition;
import script.library.group;
import script.library.ai_lib;
import script.library.space_dungeon;
import script.library.buff;
import script.library.instance;


public class vehicle extends script.base_script
{
	public vehicle()
	{
	}
	public static final int MAX_STORED_VEHICLES = 3;
	public static final int MAX_STORED_VEHICLES_MUSTAFAR_EXPANSION = 6;
	public static final int MAX_VEHICLES = 1;
	
	public static final int VAR_SPEED_MIN = 0;
	public static final int VAR_SPEED_MAX = 1;
	public static final int VAR_TURN_RATE_MIN = 2;
	public static final int VAR_TURN_RATE_MAX = 3;
	public static final int VAR_ACCEL_MIN = 4;
	public static final int VAR_ACCEL_MAX = 5;
	public static final int VAR_DECEL = 6;
	public static final int VAR_SLOPE_MOD = 7;
	public static final int VAR_DAMP_ROLL = 8;
	public static final int VAR_DAMP_PITCH = 9;
	public static final int VAR_DAMP_HEIGHT = 10;
	public static final int VAR_GLIDE = 11;
	public static final int VAR_BANKING = 12;
	public static final int VAR_HOVER_HEIGHT = 13;
	public static final int VAR_AUTO_LEVEL = 14;
	public static final int VAR_STRAFE = 15;
	
	public static final string_id SID_SYS_CANT_CALL_LOC = new string_id("pet/pet_menu","cant_call_vehicle");
	public static final string_id SID_SYS_CANT_CALL_NUM = new string_id("pet/pet_menu","cant_call_vehicle");
	public static final string_id SID_SYS_HAS_MAX_VEHICLE = new string_id("pet/pet_menu","has_max_vehicle");
	public static final string_id SID_NO_GROUND_VEHICLE_IN_SPACE = new string_id("space/space_interaction", "no_ground_vehicle_in_space");
	public static final string_id SID_NOT_WHILE_IN_COMBAT = new string_id("pet/pet_menu","cannot_call_in_combat");
	public static final string_id SID_NOT_WHILE_DEAD = new string_id("pet/pet_menu", "cannot_call_while_dead");
	public static final string_id SID_NO_CALL_INDOORS = new string_id("pet/pet_menu", "cannot_call_indoors");
	
	public static final String VEHICLE_FINISH = "player.vehicleCallFinish";
	
	public static final string_id SID_SYS_USES_LEFT_NEW = new string_id("pet/pet_menu","uses_left_new");
	public static final string_id SID_SYS_USES_LEFT = new string_id("pet/pet_menu","uses_left");
	public static final string_id SID_SYS_USES_LEFT_LAST = new string_id("pet/pet_menu","uses_left_last");
	public static final string_id SID_SYS_USES_COMPLETE = new string_id("pet/pet_menu","uses_complete");
	public static final string_id SID_SUI_CONFIRM_VEHICLE_REPAIRS = new string_id("pet/pet_menu","sui_confirm_vehicle_repairs");
	
	public static final String STF = "pet/pet_menu";
	public static final String VEHICLE_STAT_TABLE = "datatables/vehicle/vehicle_stats.iff";
	
	public static final int MOD_TYPE_MAX_SPEED = 1;
	public static final String OBJVAR_MOD_MAX_SPEED_DURATION = "vehicle.mod.maxSpeed.duration";
	public static final String OBJVAR_MOD_MAX_SPEED_OLD = "vehicle.mod.maxSpeed.old";
	
	public static final String[] s_varInfoNames = new String[]
	{
		"/private/index_speed_min",
		"/private/index_speed_max",
		"/private/index_turn_rate_min",
		"/private/index_turn_rate_max",
		"/private/index_accel_min",
		"/private/index_accel_max",
		"/private/index_decel",
		"/private/index_slope_mod",
		"/private/index_damp_roll",
		"/private/index_damp_pitch",
		"/private/index_damp_height",
		"/private/index_glide",
		"/private/index_banking",
		"/private/index_hover_height",
		"/private/index_auto_level",
		"/private/index_strafe",
	};
	
	public static final float[] s_varInfoConversions = new float[]
	{
		10.0f,
		10.0f,
		1.0f,
		1.0f,
		10.0f,
		10.0f,
		10.0f,
		10.0f,
		10.0f,
		10.0f,
		10.0f,
		10.0f,
		1.0f,
		10.0f,
		100.0f,
		1.0f,
	};
	
	public static final int VEHICLE_DECAY_CYCLE = 600;
	
	public static final int CUSTOMIZATION_COUNT = 200;
	
	public static final String VAR_PALVAR_BASE = "ai.pet.palvar";
	public static final String VAR_PALVAR_VARS = VAR_PALVAR_BASE + ".vars";
	public static final String VAR_PALVAR_CNT = VAR_PALVAR_BASE + ".cnt";
	
	
	public static int setValue(obj_id vehicle, float value, int var_index) throws InterruptedException
	{
		String vi_name = s_varInfoNames[var_index];
		float vi_conversion = s_varInfoConversions[var_index];
		
		int ivalue = (int)(value * vi_conversion);
		setRangedIntCustomVarValue(vehicle, vi_name, ivalue);
		
		return ivalue;
	}
	
	
	public static float getValue(obj_id vehicle, int var_index) throws InterruptedException
	{
		String vi_name = s_varInfoNames[var_index];
		float vi_conversion = s_varInfoConversions[var_index];
		
		int ivalue = getRangedIntCustomVarValue(vehicle, vi_name);
		float value = ((float)ivalue) / vi_conversion;
		
		return value;
	}
	
	
	public static void setMinimumSpeed(obj_id vehicle, float value) throws InterruptedException
	{
		setValue (vehicle, value, VAR_SPEED_MIN);
	}
	
	
	public static void setMaximumSpeed(obj_id vehicle, float value) throws InterruptedException
	{
		setValue (vehicle, value, VAR_SPEED_MAX);
	}
	
	
	public static void setTurnRateMin(obj_id vehicle, float value) throws InterruptedException
	{
		setValue (vehicle, value, VAR_TURN_RATE_MIN);
	}
	
	
	public static void setTurnRateMax(obj_id vehicle, float value) throws InterruptedException
	{
		setValue (vehicle, value, VAR_TURN_RATE_MAX);
	}
	
	
	public static void setAccelMin(obj_id vehicle, float value) throws InterruptedException
	{
		setValue (vehicle, value, VAR_ACCEL_MIN);
	}
	
	
	public static void setAccelMax(obj_id vehicle, float value) throws InterruptedException
	{
		setValue (vehicle, value, VAR_ACCEL_MAX);
	}
	
	
	public static void setDecel(obj_id vehicle, float value) throws InterruptedException
	{
		setValue (vehicle, value, VAR_DECEL);
	}
	
	
	public static void setDampingRoll(obj_id vehicle, float value) throws InterruptedException
	{
		setValue (vehicle, value, VAR_DAMP_ROLL);
	}
	
	
	public static void setDampingPitch(obj_id vehicle, float value) throws InterruptedException
	{
		setValue (vehicle, value, VAR_DAMP_PITCH);
	}
	
	
	public static void setDampingHeight(obj_id vehicle, float value) throws InterruptedException
	{
		setValue (vehicle, value, VAR_DAMP_HEIGHT);
	}
	
	
	public static void setGlide(obj_id vehicle, float value) throws InterruptedException
	{
		setValue (vehicle, value, VAR_GLIDE);
	}
	
	
	public static void setBankingAngle(obj_id vehicle, float value) throws InterruptedException
	{
		setValue (vehicle, value, VAR_BANKING);
	}
	
	
	public static void setHoverHeight(obj_id vehicle, float value) throws InterruptedException
	{
		setValue (vehicle, value, VAR_HOVER_HEIGHT);
	}
	
	
	public static void setAutoLevelling(obj_id vehicle, float value) throws InterruptedException
	{
		setValue (vehicle, value, VAR_AUTO_LEVEL);
	}
	
	
	public static void setStrafe(obj_id vehicle, boolean canStrafe) throws InterruptedException
	{
		float value = canStrafe ? 1.0f : 0.0f;
		setValue (vehicle, value, VAR_STRAFE);
	}
	
	
	public static float getMinimumSpeed(obj_id vehicle) throws InterruptedException
	{
		return getValue (vehicle, VAR_SPEED_MIN);
	}
	
	
	public static float getMaximumSpeed(obj_id vehicle) throws InterruptedException
	{
		return getValue (vehicle, VAR_SPEED_MAX);
	}
	
	
	public static float getTurnRateMin(obj_id vehicle) throws InterruptedException
	{
		return getValue (vehicle, VAR_TURN_RATE_MIN);
	}
	
	
	public static float getTurnRateMax(obj_id vehicle) throws InterruptedException
	{
		return getValue (vehicle, VAR_TURN_RATE_MAX);
	}
	
	
	public static float getAccelMin(obj_id vehicle) throws InterruptedException
	{
		return getValue (vehicle, VAR_ACCEL_MIN);
	}
	
	
	public static float getAccelMax(obj_id vehicle) throws InterruptedException
	{
		return getValue (vehicle, VAR_ACCEL_MAX);
	}
	
	
	public static float getDecel(obj_id vehicle) throws InterruptedException
	{
		return getValue (vehicle, VAR_DECEL);
	}
	
	
	public static float getDampingRoll(obj_id vehicle) throws InterruptedException
	{
		return getValue (vehicle, VAR_DAMP_ROLL);
	}
	
	
	public static float getDampingPitch(obj_id vehicle) throws InterruptedException
	{
		return getValue (vehicle, VAR_DAMP_PITCH);
	}
	
	
	public static float getDampingHeight(obj_id vehicle) throws InterruptedException
	{
		return getValue (vehicle, VAR_DAMP_HEIGHT);
	}
	
	
	public static float getGlide(obj_id vehicle) throws InterruptedException
	{
		return getValue (vehicle, VAR_GLIDE);
	}
	
	
	public static float getBankingAngle(obj_id vehicle) throws InterruptedException
	{
		return getValue (vehicle, VAR_BANKING);
	}
	
	
	public static float getHoverHeight(obj_id vehicle) throws InterruptedException
	{
		return getValue (vehicle, VAR_HOVER_HEIGHT);
	}
	
	
	public static float getAutoLevelling(obj_id vehicle) throws InterruptedException
	{
		return getValue (vehicle, VAR_AUTO_LEVEL);
	}
	
	
	public static boolean getStrafe(obj_id vehicle) throws InterruptedException
	{
		float value = getValue (vehicle, VAR_STRAFE);
		return value > 0.0f ? true : false;
	}
	
	
	public static boolean isRidingVehicle(obj_id objPlayer) throws InterruptedException
	{
		obj_id objMount = getMountId(objPlayer);
		
		if (isIdValid(objMount))
		{
			if (hasScript(objMount,"systems.vehicle_system.vehicle_base") || isBattlefieldVehicle(objMount))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isRidingBattlefieldVehicle(obj_id objPlayer) throws InterruptedException
	{
		obj_id objMount = getMountId(objPlayer);
		
		if (isIdValid(objMount))
		{
			if (isBattlefieldVehicle(objMount))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isDriveableVehicle(obj_id objThing) throws InterruptedException
	{
		if (hasScript(objThing,"systems.vehicle_system.vehicle_base"))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isVehicle(obj_id obj) throws InterruptedException
	{
		return isGameObjectTypeOf (getGameObjectType (obj), GOT_vehicle);
	}
	
	
	public static boolean isBattlefieldVehicle(obj_id obj) throws InterruptedException
	{
		return isVehicle(obj) && hasScript(obj, "systems.vehicle_system.battlefield_vehicle");
	}
	
	
	public static String getVehicleTemplate(obj_id controlDevice) throws InterruptedException
	{
		if (!isIdValid(controlDevice))
		{
			return null;
		}
		
		String ref = getVehicleReference(controlDevice);
		if (ref == null || ref.equals(""))
		{
			return null;
		}
		
		return dataTableGetString(create.VEHICLE_TABLE, ref, "OBJECT_TEMPLATE");
	}
	
	
	public static void doTempMaxSpeedReduction(obj_id vehicle, float factor, float duration) throws InterruptedException
	{
		if (factor < 0.01f || factor > 1.0f)
		{
			return;
		}
		
		if (hasObjVar(vehicle, OBJVAR_MOD_MAX_SPEED_OLD))
		{
			return;
		}
		
		float currentMaxSpeed = getMaximumSpeed(vehicle);
		setObjVar(vehicle, OBJVAR_MOD_MAX_SPEED_OLD, currentMaxSpeed);
		setObjVar(vehicle, OBJVAR_MOD_MAX_SPEED_DURATION, duration);
		dictionary params = new dictionary();
		params.put("type", MOD_TYPE_MAX_SPEED);
		messageTo(vehicle, "revertVehicleMod", params, duration, false);
		setMaximumSpeed(vehicle, currentMaxSpeed * factor);
		
		return;
	}
	
	
	public static String getVehicleReference(obj_id controlDevice) throws InterruptedException
	{
		if (!isIdValid(controlDevice))
		{
			return null;
		}
		
		return getStringObjVar(controlDevice, "vehicle_attribs.object_ref");
	}
	
	
	public static boolean repairVehicle(obj_id player, obj_id vehicle) throws InterruptedException
	{
		if (!isIdValid(player) || !isIdValid(vehicle) || !vehicle.isLoaded())
		{
			return false;
		}
		
		obj_id petControlDevice = callable.getCallableCD(vehicle);
		
		if ((isDisabled(vehicle) || getHitpoints(vehicle) < 1) && !canRepairDisabledVehicle(petControlDevice))
		{
			sendSystemMessage(player, new string_id("pet/pet_menu", "cannot_repair_disabled"));
			return false;
		}
		
		obj_id garage = utils.getObjIdScriptVar(vehicle, "inRepairZone");
		if (!isIdValid(garage) || !garage.isLoaded())
		{
			sendSystemMessage(player, new string_id("pet/pet_menu", "repair_unrecognized_garages"));
			return false;
		}
		
		if (utils.hasScriptVar(player, "vehicleRepair.pid"))
		{
			int oldpid = utils.getIntScriptVar(player, "vehicleRepair.pid");
			sui.closeSUI(player, oldpid);
			utils.removeScriptVarTree(player, "vehicleRepair");
		}
		
		int cost = getVehicleRepairCost(vehicle);
		
		if (cost == 0)
		{
			sendSystemMessage(player, new string_id("pet/pet_menu", "undamaged_vehicle"));
			return false;
		}
		else if (cost < 0)
		{
			sendSystemMessage(player, new string_id("pet/pet_menu", "repair_error"));
			return false;
		}
		
		int total = getTotalMoney(player);
		
		if (total < cost)
		{
			prose_package ppLackFunds = prose.getPackage(new string_id(STF, "lacking_funds"));
			prose.setDI(ppLackFunds, cost - total);
			sendSystemMessageProse(player, ppLackFunds);
			return false;
		}
		
		String prompt = getString(new string_id(STF, "vehicle_repair_d"));
		
		prompt += "\n\n"+ getString(new string_id(STF, "vehicle_prompt")) +getString(getNameStringId(vehicle));
		prompt += "\n"+ getString(new string_id(STF, "repair_cost_prompt")) + cost;
		prompt += "\n"+ getString(new string_id(STF, "total_funds_prompt")) + total;
		
		String title = utils.packStringId(SID_SUI_CONFIRM_VEHICLE_REPAIRS);
		
		int pid = sui.msgbox(garage, player, prompt, sui.OK_CANCEL, title, "handleRepairConfirm");
		
		if (pid > -1)
		{
			
			utils.setScriptVar(player, "vehicleRepair.pid", pid);
			utils.setScriptVar(player, "vehicleRepair.vehicle", vehicle);
			utils.setScriptVar(player, "vehicleRepair.cost", cost);
			return true;
		}
		
		return false;
	}
	
	
	public static int getVehicleRepairCost(obj_id vehicle) throws InterruptedException
	{
		if (!isIdValid(vehicle))
		{
			return -1;
		}
		
		int toHeal = getMaxHitpoints(vehicle)-getHitpoints(vehicle);
		
		float repair_rate = getVehicleRepairRate(vehicle);
		float city_base = 100f;
		float city_add = 0f;
		
		int city_id = getCityAtLocation(getLocation(vehicle), 0);
		obj_id city = cityGetCityHall(city_id);
		if (isIdValid(city))
		{
			String cityName = cityGetName(city_id);
			String cityGarage = cityName + ".garageFee";
			if (hasObjVar(city, cityGarage))
			{
				city_add = (float)getIntObjVar(city, cityGarage);
			}
		}
		
		if (!isIdValid(city))
		{
			city = obj_id.NULL_ID;
		}
		utils.setScriptVar(vehicle, "vehicleRepair.city_id", city);
		
		float city_tax = ((city_base + city_add)/100f);
		float city_cut = (((toHeal * repair_rate) * city_tax) - (toHeal * repair_rate));
		utils.setScriptVar(vehicle, "vehicleRepair.city_tax", city_cut);
		
		return Math.round((toHeal * repair_rate) * city_tax);
	}
	
	
	public static float getVehicleRepairRate(obj_id vehicle) throws InterruptedException
	{
		if (!isIdValid(vehicle))
		{
			return -1f;
		}
		
		obj_id controlDevice = callable.getCallableCD(vehicle);
		
		if (!isIdValid(controlDevice))
		{
			return -1f;
		}
		
		String ref = getVehicleReference(controlDevice);
		if (ref == null || ref.equals(""))
		{
			return -1f;
		}
		
		return dataTableGetFloat(create.VEHICLE_TABLE, ref, "REPAIR_RATE");
	}
	
	
	public static void decayVehicle(obj_id vehicle) throws InterruptedException
	{
		if (!isIdValid(vehicle))
		{
			return;
		}
		
		int now = getGameTime();
		float decay_rate = getVehicleDecayRate(vehicle);
		if (decay_rate <= 0f)
		{
			return;
		}
		
		int decayAmt = 0;
		if (utils.hasScriptVar(vehicle, "decay.stamp"))
		{
			int stamp = utils.getIntScriptVar(vehicle, "decay.stamp");
			int delta = now - stamp;
			
			float ratio = delta / VEHICLE_DECAY_CYCLE;
			
			decayAmt = Math.round(ratio * decay_rate);
		}
		else
		{
			decayAmt = Math.round(decay_rate / 2f);
		}
		
		if (decayAmt <= 0)
		{
			return;
		}
		
		int currentHP = getHitpoints(vehicle);
		currentHP -= decayAmt;
		
		setHitpoints(vehicle, currentHP);
		
		obj_id vcd = callable.getCallableCD(vehicle);
		
		dictionary params = new dictionary();
		params.put("hp", currentHP);
		params.put("penalty", decayAmt);
		messageTo(vcd, "handleStoreVehicleDamage", params, 0f, false);
		
		utils.setScriptVar(vehicle, "decay.stamp", now);
		messageTo(vehicle, "handleVehicleDecay", null, VEHICLE_DECAY_CYCLE, false);
	}
	
	
	public static float getVehicleDecayRate(obj_id vehicle) throws InterruptedException
	{
		if (!isIdValid(vehicle))
		{
			return -1f;
		}
		
		obj_id controlDevice = callable.getCallableCD(vehicle);
		
		if (!isIdValid(controlDevice))
		{
			return -1f;
		}
		
		String ref = getVehicleReference(controlDevice);
		
		if (ref == null || ref.equals(""))
		{
			return -1f;
		}
		
		return dataTableGetFloat(create.VEHICLE_TABLE, ref, "DECAY_RATE");
	}
	
	
	public static boolean isInValidUnpackLocation(obj_id master) throws InterruptedException
	{
		
		if (isSpaceScene())
		{
			sendSystemMessage(master, SID_NO_GROUND_VEHICLE_IN_SPACE);
			return false;
		}
		
		location yourLoc = getLocation(master);
		
		if (isIdValid(yourLoc.cell))
		{
			sendSystemMessage(master, SID_NO_CALL_INDOORS);
			return false;
		}
		if (isInRestrictedScene(master))
		{
			sendSystemMessage(master, pet_lib.SID_SYS_VEHICLE_RESTRICTED_SCENE);
			return false;
		}
		
		return true;
	}
	
	
	public static boolean isInRestrictedScene(obj_id master) throws InterruptedException
	{
		String scene = getCurrentSceneName();
		String restrictionTable = "datatables/vehicle/vehicle_restrictions.iff";
		String[] restrictedScenes = dataTableGetStringColumn(restrictionTable, 0);
		int[] restrictions = dataTableGetIntColumn(restrictionTable, 1);
		
		if (isIdValid(instance.getAreaInstanceController(master)) && !instance.vehicleAllowedInInstance(instance.getAreaInstanceController(master)))
		{
			sendSystemMessage(master, new string_id("instance", "no_vehicle"));
			return true;
		}
		
		if (hasObjVar(master, space_dungeon.VAR_DUNGEON_ID))
		{
			return true;
		}
		
		for (int i = 0; i < restrictedScenes.length; i++)
		{
			testAbortScript();
			if (restrictedScenes[i].equals(scene))
			{
				if (restrictions[i] == 1)
				{
					if (isGod(master))
					{
						sendSystemMessageTestingOnly(master, "GODMODE MSG: You are in a vehicle restricted scene.");
					}
					return true;
				}
			}
		}
		
		return false;
	}
	
	
	public static boolean canQuickUnpack(obj_id master) throws InterruptedException
	{
		location yourLoc = getLocation(master);
		if (isIdValid(yourLoc.cell))
		{
			return false;
		}
		
		if (isInRestrictedScene(master))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean isInValidCallState(obj_id player) throws InterruptedException
	{
		if (ai_lib.aiIsDead(player))
		{
			sendSystemMessage(player, SID_NOT_WHILE_DEAD);
			return false;
		}
		
		if (ai_lib.isInCombat(player))
		{
			sendSystemMessage(player, SID_NOT_WHILE_IN_COMBAT);
			return false;
		}
		
		return true;
	}
	
	
	public static boolean storeVehicle(obj_id vehicleControlDevice, obj_id player) throws InterruptedException
	{
		return storeVehicle(vehicleControlDevice, player, true);
	}
	
	
	public static boolean storeVehicle(obj_id vehicleControlDevice, obj_id player, boolean checkCombat) throws InterruptedException
	{
		obj_id currentVehicle = callable.getCDCallable(vehicleControlDevice);
		
		if (!isIdValid(currentVehicle))
		{
			debugServerConsoleMsg(player, "+++ VEHICLE library . storeVehicle +++ current vehicle obj_id is NOT valid");
			return false;
		}
		
		if (checkCombat)
		{
			if (pet_lib.wasInCombatRecently(currentVehicle, player, true) && !isJetPack(vehicleControlDevice))
			{
				return false;
			}
		}
		
		debugServerConsoleMsg(player, "+++ VEHICLE library . storeVehicle +++ current vehicle obj_id is valid. Doing tons of stuff.");
		
		if (hasObjVar(vehicleControlDevice, "pet.uses_left"))
		{
			int uses = getIntObjVar(vehicleControlDevice, "pet.uses_left");
			
			if (uses <= 0)
			{
				sendSystemMessage(player, SID_SYS_USES_COMPLETE);
				destroyObject(vehicleControlDevice);
				
				return true;
			}
		}
		
		setTimeStored(vehicleControlDevice);
		
		int currentHP = getHitpoints(currentVehicle);
		
		dictionary params = new dictionary();
		params.put("hp", currentHP);
		params.put("penalty", 0);
		
		messageTo(vehicleControlDevice, "handleStoreVehicleDamage", params, 0f, false);
		
		messageTo(currentVehicle, "handlePackRequest", null, 1, false);
		
		callable.setCDCallable(vehicleControlDevice, null);
		
		return true;
	}
	
	
	public static void setTimeStored(obj_id PCD) throws InterruptedException
	{
		setObjVar(PCD, "pet.timeStored", getGameTime());
	}
	
	
	public static void storeAllVehicles(obj_id master) throws InterruptedException
	{
		debugServerConsoleMsg(master, "+++ VEHICLE library . storeAllVehicles +++ just entered function. master obj_id fed in is: "+ master);
		
		obj_id objVehicle = callable.getCallable(master, callable.CALLABLE_TYPE_RIDEABLE);
		
		if (isIdValid(objVehicle))
		{
			if (!objVehicle.isLoaded() == false)
			{
				
				messageTo(objVehicle, "handlePackRequest", null, 0, false);
			}
			else
			{
				if (group.isGrouped(objVehicle))
				{
					obj_id group = getGroupObject(objVehicle);
					
					if (isIdValid(group))
					{
						queueCommand(objVehicle, (1348589140), group, "", COMMAND_PRIORITY_IMMEDIATE);
					}
				}
				
				if (ai_lib.aiIsDead(master))
				{
					debugServerConsoleMsg(master, "+++ VEHICLE library . storeAllVehicles +++ Game thinks that we're dead. Following that branch of storeAllVehicles");
					
					boolean isFactionPet = (ai_lib.isNpc(objVehicle) || ai_lib.aiGetNiche(objVehicle) == NICHE_VEHICLE || ai_lib.isAndroid(objVehicle));
					
					if (hasObjVar(objVehicle, battlefield.VAR_CONSTRUCTED) || isFactionPet)
					{
						
						obj_id petControlDevice = callable.getCallableCD(objVehicle);
						
						if (isIdValid(petControlDevice))
						{
							messageTo(petControlDevice, "handleFlagDeadCreature", null, 0, false);
						}
						
						destroyObject(objVehicle);
					}
					else
					{
						debugServerConsoleMsg(master, "+++ VEHICLE library . storeAllVehicles +++ Looks like we're dead, but pet's weren't faction. Firing off a storeVehicle function.");
						
						storeVehicle(objVehicle, master);
					}
				}
				else
				{
					
					debugServerConsoleMsg(master, "+++ VEHICLE library . storeAllVehicles +++ We aren't dead. Firing off a storeVehicle function.");
					storeVehicle(objVehicle, master);
				}
			}
		}
	}
	
	
	public static boolean hasMaxStoredVehicles(obj_id player) throws InterruptedException
	{
		return callable.hasMaxStoredRideables(player);
	}
	
	
	public static void reHueVehicle(obj_id vehicleControlDevice, obj_id vehicle) throws InterruptedException
	{
		
		int colorIdx = hue.getVarColorIndex(vehicleControlDevice, hue.INDEX_1);
		if (hasObjVar(vehicleControlDevice, "creature_attribs.hue"))
		{
			colorIdx = getIntObjVar(vehicleControlDevice, "creature_attribs.hue");
		}
		
		hue.setColor(vehicle, hue.INDEX_1, colorIdx);
		
		return;
	}
	
	
	public static void saveVehicleInfo(obj_id vehicleControlDevice, obj_id vehicle) throws InterruptedException
	{
		if (!isIdValid(vehicle) || !isIdValid(vehicleControlDevice))
		{
			return;
		}
		
		if (!vehicle.isLoaded())
		{
			return;
		}
		
		obj_id master = getMaster(vehicle);
		if (isIdValid(master) && master.isLoaded())
		{
			setHomeLocation(vehicle, getLocation(master));
		}
		
		pet_lib.storeDamage(vehicle, vehicleControlDevice);
		setObjVar(vehicleControlDevice, "pet.timeStored", getGameTime());
		
		ranged_int_custom_var[] ri = hue.getPalcolorVars(vehicle);
		if (ri != null && ri.length > 0)
		{
			boolean wasUpdated = false;
			for (int i = 0; i < ri.length; i++)
			{
				testAbortScript();
				int val = ri[i].getValue();
				if (val > -1)
				{
					String varpath = VAR_PALVAR_VARS + "."+ ri[i].getVarName();
					int cur = getIntObjVar(vehicleControlDevice, varpath);
					if (cur != val)
					{
						setObjVar(vehicleControlDevice, varpath, val);
					}
				}
			}
			
			if (utils.hasScriptVar(vehicle, "customizationUpdated"))
			{
				setObjVar(vehicleControlDevice, VAR_PALVAR_CNT, CUSTOMIZATION_COUNT);
			}
		}
	}
	
	
	public static void storeDamage(obj_id vcd, int currentHP, int penalty) throws InterruptedException
	{
		
		if (!isIdValid(vcd))
		{
			return;
		}
		
		if (currentHP == ATTRIB_ERROR)
		{
			
			currentHP = getIntObjVar(vcd, "attrib.hit_points");
			currentHP -= penalty;
			
			if (currentHP < 0)
			{
				currentHP = 0;
			}
			
		}
		
		setObjVar(vcd, "attrib.hit_points", currentHP);
	}
	
	
	public static void initializeVehicle(obj_id vehicle) throws InterruptedException
	{
		initializeVehicle(vehicle, null);
	}
	
	
	public static void initializeVehicle(obj_id vehicle, obj_id player) throws InterruptedException
	{
		
		if (!isIdValid(player))
		{
			player = vehicle;
		}
		
		String datatable = VEHICLE_STAT_TABLE;
		String row = getTemplateName (vehicle);
		
		if (dataTableHasColumn (datatable, "min_speed"))
		{
			float minSpeed = dataTableGetFloat (datatable, row, "min_speed");
			setMinimumSpeed (vehicle, minSpeed);
		}
		
		if (dataTableHasColumn (datatable, "speed"))
		{
			float speed = dataTableGetFloat (datatable, row, "speed");
			setMaximumSpeed (vehicle, speed);
		}
		
		if (dataTableHasColumn (datatable, "turn_rate"))
		{
			float turnRate = dataTableGetFloat (datatable, row, "turn_rate");
			float turnSkillMod = getEnhancedSkillStatisticModifier (player, "force_vehicle_control");
			
			turnRate = turnRate + turnSkillMod;
			setTurnRateMin (vehicle, turnRate);
		}
		
		if (dataTableHasColumn (datatable, "turn_rate_max"))
		{
			float turnRateMax = dataTableGetFloat (datatable, row, "turn_rate_max");
			float maxTurnSkillMod = getEnhancedSkillStatisticModifier (player, "force_vehicle_control");
			
			turnRateMax = turnRateMax + maxTurnSkillMod;
			setTurnRateMax (vehicle, turnRateMax);
		}
		
		if (dataTableHasColumn (datatable, "accel_min"))
		{
			float accelMin = dataTableGetFloat (datatable, row, "accel_min");
			float accelSkillMod = getEnhancedSkillStatisticModifier (player, "force_vehicle_speed");
			
			accelMin = accelMin + accelSkillMod;
			setAccelMin (vehicle, accelMin);
		}
		
		if (dataTableHasColumn (datatable, "accel_max"))
		{
			float accelMax = dataTableGetFloat (datatable, row, "accel_max");
			float maxAccelSkillMod = getEnhancedSkillStatisticModifier (player, "force_vehicle_speed");
			
			accelMax = accelMax + maxAccelSkillMod;
			setAccelMax (vehicle, accelMax);
		}
		
		if (dataTableHasColumn (datatable, "decel"))
		{
			float decel = dataTableGetFloat (datatable, row, "decel");
			float decelSkillMod = getEnhancedSkillStatisticModifier (player, "force_vehicle_speed");
			
			decel = decel + decelSkillMod;
			setDecel (vehicle, decel);
		}
		
		if (dataTableHasColumn (datatable, "damping_roll"))
		{
			float dampingRoll = dataTableGetFloat (datatable, row, "damping_roll");
			setDampingRoll (vehicle, dampingRoll);
		}
		
		if (dataTableHasColumn (datatable, "damping_pitch"))
		{
			float dampingPitch = dataTableGetFloat (datatable, row, "damping_pitch");
			setDampingPitch (vehicle, dampingPitch);
		}
		
		if (dataTableHasColumn (datatable, "damping_height"))
		{
			float dampingHeight = dataTableGetFloat (datatable, row, "damping_height");
			setDampingHeight (vehicle, dampingHeight);
		}
		
		if (dataTableHasColumn (datatable, "glide"))
		{
			float glide = dataTableGetFloat (datatable, row, "glide");
			setGlide (vehicle, glide);
		}
		
		if (dataTableHasColumn (datatable, "banking_angle"))
		{
			float bankingAngle = dataTableGetFloat (datatable, row, "banking_angle");
			setBankingAngle (vehicle, bankingAngle);
		}
		
		if (dataTableHasColumn (datatable, "hover_height"))
		{
			float hoverHeight = dataTableGetFloat (datatable, row, "hover_height");
			setHoverHeight (vehicle, hoverHeight);
		}
		
		if (dataTableHasColumn (datatable, "auto_level"))
		{
			float autoLevel = dataTableGetFloat (datatable, row, "auto_level");
			setAutoLevelling (vehicle, autoLevel);
		}
		
		if (dataTableHasColumn (datatable, "strafe"))
		{
			int strafe = dataTableGetInt(datatable, row, "strafe");
			setStrafe (vehicle, strafe != 0);
		}
		
		return;
	}
	
	
	public static boolean mountPermissionCheck(obj_id vehicle, obj_id rider, boolean verbose) throws InterruptedException
	{
		if (!isIdValid(vehicle) || !isIdValid(rider))
		{
			return false;
		}
		
		obj_id driver = getRiderId(vehicle);
		
		if (!isIdValid(driver))
		{
			return false;
		}
		
		if (getOwner(vehicle) == rider)
		{
			return true;
		}
		
		if (!group.inSameGroup(rider, driver))
		{
			return false;
		}
		
		if (getState(rider, STATE_COMBAT) == 1)
		{
			if (verbose)
			{
				string_id strSpam = new string_id("vehicle/vehicle", "rider_in_combat");
				sendSystemMessage(rider, strSpam);
			}
			return false;
		}
		
		if (factions.isDeclared(rider))
		{
			if (factions.isDeclared(driver) && pvpAreFactionsOpposed(pvpGetAlignedFaction(rider), pvpGetAlignedFaction(driver)))
			{
				if (verbose)
				{
					string_id strSpam = new string_id("vehicle/vehicle", "rider_opposing_faction");
					sendSystemMessage(rider, strSpam);
				}
				return false;
			}
			else if (!factions.isDeclared(driver))
			{
				if (verbose)
				{
					string_id strSpam = new string_id("vehicle/vehicle", "rider_overt_driver_not");
					sendSystemMessage(rider, strSpam);
				}
				return false;
			}
		}
		
		return true;
	}
	
	
	public static void restoreVehicle(obj_id player, obj_id vehicle) throws InterruptedException
	{
		obj_id tool = utils.getItemPlayerHasByTemplate(player, "object/tangible/item/ep3/barc_repair_tool.iff");
		if (!isIdValid(tool))
		{
			return;
		}
		
		clearCondition(vehicle, CONDITION_DISABLED);
		int maxHp = getMaxHitpoints(vehicle);
		setHitpoints(vehicle, maxHp);
		
		int currentCount = getCount(tool);
		if (currentCount > 1)
		{
			--currentCount;
			setCount(tool, currentCount);
		}
		else
		{
			destroyObject(tool);
		}
	}
	
	
	public static boolean isJetPack(obj_id vehicleControlDevice) throws InterruptedException
	{
		String name = getVehicleTemplate(vehicleControlDevice);
		if (name.equals("object/mobile/vehicle/jetpack.iff"))
		{
			return true;
		}
		return false;
	}
	
	
	public static void applyVehicleBuffs(obj_id player, obj_id vehicle) throws InterruptedException
	{
		String template = getTemplateName(vehicle);
		if (template == null || template.equals(""))
		{
			LOG("mount", template + " bailed out because template is bad");
			return;
		}
		
		dictionary vehicleData = new dictionary();
		vehicleData = dataTableGetRow(VEHICLE_STAT_TABLE, template);
		if (vehicleData == null || vehicleData.equals(""))
		{
			LOG("mount", vehicleData + " bailed out because row in datatable is bad");
			return;
		}
		
		String buffName = vehicleData.getString("player_buff");
		String vehicleBuffName = vehicleData.getString("vehicle_buff");
		String clientEffect = vehicleData.getString("buff_client_effect");
		
		boolean hasPlayerBuff = buffName != null && !buffName.equals("");
		boolean hasVehicleBuff = vehicleBuffName != null && !vehicleBuffName.equals("");
		
		if (!hasPlayerBuff && !hasVehicleBuff)
		{
			LOG("mount","bailed out because this vehicle has no buffs");
			return;
		}
		
		if (hasPlayerBuff && buff.canApplyBuff(player, buffName))
		{
			buff.applyBuff(player, buffName);
			
			if (clientEffect != null && !clientEffect.equals(""))
			{
				playClientEffectObj(player, clientEffect, vehicle, "");
			}
		}
		
		if (hasVehicleBuff && buff.canApplyBuff(vehicle, vehicleBuffName))
		{
			buff.applyBuff(vehicle, vehicleBuffName);
		}
		
		return;
	}
	
	
	public static boolean canRepairDisabledVehicle(obj_id controlDevice) throws InterruptedException
	{
		String ref = getVehicleReference(controlDevice);
		if (ref == null || ref.equals(""))
		{
			return false;
		}
		
		if (dataTableGetInt(create.VEHICLE_TABLE, ref, "CAN_REPAIR_DISABLED") == 1)
		{
			return true;
		}
		
		return false;
	}
}
