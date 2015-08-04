package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.utils;
import java.lang.Math;
import script.library.space_utils;
import script.library.sui;
import script.library.badge;
import script.library.groundquests;
import script.library.space_quest;
import script.library.space_combat;


public class space_crafting extends script.base_script
{
	public space_crafting()
	{
	}
	public static final int CHASSIS = space_combat.SHIP;
	
	public static final int HULL = -1;
	public static final int LIFE_SUPPORT = -2;
	public static final int PLASMA_CONDUIT = -3;
	public static final int AMMO_BAY = -4;
	public static final int HYPERDRIVE = -5;
	
	public static final int HULL_BREACH_DAMAGE = 50;
	public static final float HULL_BREACH_DAMAGE_TIMER = 5;
	
	public static final String SHIP_DROID_SLOT_NAME = "ship_droid";
	
	public static final String STF_COMPONENT_TOOL = "reverse_engineering_tool";
	public static final String SCRIPT_COMPONENT_NAME_HANDLER = "space.crafting.component_name_handler";
	
	public static final int DAMAGE_CHANCE = 5;
	public static final int[] NON_COMPONENT_DAMAGEABLES_LIGHT =
	{
		PLASMA_CONDUIT
	};
	
	public static final int[] NON_COMPONENT_DAMAGEABLES_MEDIUM =
	{
		PLASMA_CONDUIT,
	};
	
	public static final int[] NON_COMPONENT_DAMAGEABLES_HEAVY =
	{
		PLASMA_CONDUIT,
	};
	
	public static final int REACTOR = ship_chassis_slot_type.SCST_reactor;
	public static final int ENGINE = ship_chassis_slot_type.SCST_engine;
	public static final int SHIELD_GENERATOR = ship_chassis_slot_type.SCST_shield_0;
	public static final int ARMOR_0 = ship_chassis_slot_type.SCST_armor_0;
	public static final int ARMOR_1 = ship_chassis_slot_type.SCST_armor_1;
	public static final int CAPACITOR = ship_chassis_slot_type.SCST_capacitor;
	public static final int BOOSTER = ship_chassis_slot_type.SCST_booster;
	public static final int DROID_INTERFACE = ship_chassis_slot_type.SCST_droid_interface;
	public static final int TARGETING_STATION = ship_chassis_slot_type.SCST_targeting_station;
	public static final int WEAPON_0 = ship_chassis_slot_type.SCST_weapon_0;
	public static final int WEAPON_1 = ship_chassis_slot_type.SCST_weapon_1;
	public static final int WEAPON_2 = ship_chassis_slot_type.SCST_weapon_2;
	public static final int WEAPON_3 = ship_chassis_slot_type.SCST_weapon_3;
	public static final int WEAPON_4 = ship_chassis_slot_type.SCST_weapon_4;
	public static final int WEAPON_5 = ship_chassis_slot_type.SCST_weapon_5;
	public static final int WEAPON_6 = ship_chassis_slot_type.SCST_weapon_6;
	public static final int WEAPON_7 = ship_chassis_slot_type.SCST_weapon_7;
	public static final int CARGO = ship_chassis_slot_type.SCST_cargo_hold;
	
	public static final string_id SID_INCORRECTLY_CONFIGURED_ITEM = new string_id("space/space_interaction", "incorrectly_configured_item");
	
	public static final String STARSHIP_DROID_TABLE = "datatables/space_combat/ship_droid_assignments.iff";
	
	
	public static double randBell(double avg, double var) throws InterruptedException
	{
		var = var / 2;
		double r, v1, v2;
		
		do
		{
			testAbortScript();
			v1 = 2.0 * rand() - 1.0;
			v2 = 2.0 * rand() - 1.0;
			r = v1 * v1 + v2 * v2;
		}
		while(r >= 1.0 || r == 0.0);
		
		double fac = Math.sqrt(-2.0 * Math.log(r) / r);
		double value = (avg + (v1 * fac) * (avg * var));
		
		if (value < 0)
		{
			return 0;
		}
		else
		{
			return value;
		}
	}
	
	
	public static float getBellValue(float fltValue, float fltModifier) throws InterruptedException
	{
		
		return (float) (randBell(fltValue, fltModifier));
	}
	
	
	public static float getModifiedValue(float fltValue, float fltModifier) throws InterruptedException
	{
		
		fltValue = fltValue + rand(-1 * fltModifier, fltModifier);
		return fltValue;
	}
	
	
	public static float getRawModifiedValue(float fltModifier) throws InterruptedException
	{
		return rand(-1 * fltModifier, fltModifier);
	}
	
	
	public static String getShipComponentStringType(obj_id objComponent) throws InterruptedException
	{
		int intComponentType = getShipComponentDescriptorType(objComponent);
		
		return getShipComponentStringType(intComponentType);
	}
	
	
	public static String getShipComponentStringType(int intComponentType) throws InterruptedException
	{
		if (intComponentType < ship_component_type.names.length)
		{
			return ship_component_type.names[intComponentType];
		}
		
		LOG("space", "type is "+ intComponentType);
		return null;
	}
	
	
	public static int getComponentSlotInt(String strName) throws InterruptedException
	{
		return ship_chassis_slot_type.getTypeByName(strName);
	}
	
	
	public static String getComponentSlotNameString(int intSlot) throws InterruptedException
	{
		return ship_chassis_slot_type.getNameByType(intSlot);
	}
	
	
	public static obj_id setupStandardStatistics(obj_id objComponent, dictionary dctParams) throws InterruptedException
	{
		
		float fltCurrentHitpoints = dctParams.getFloat("fltCurrentHitpoints");
		float fltMaximumHitpoints = dctParams.getFloat("fltMaximumHitpoints");
		
		float fltEnergyMaintenanceModifier = dctParams.getFloat("fltEnergyMaintenanceModifier");
		float fltEnergyMaintenance = dctParams.getFloat("fltEnergyMaintenance");
		
		fltEnergyMaintenance = getBellValue(fltEnergyMaintenance, fltEnergyMaintenanceModifier);
		
		float fltMassModifier = dctParams.getFloat("fltMassModifier");
		float fltMass = dctParams.getFloat("fltMass");
		
		fltMass = getBellValue(fltMass, fltMassModifier);
		
		float fltCurrentConduitHitpoints = dctParams.getFloat("fltCurrentConduitHitpoints");
		float fltMaximumConduitHitpoints = dctParams.getFloat("fltMaximumConduitHitpoints");
		
		float fltEfficiency = dctParams.getFloat("fltEfficiency");
		float fltCurrentArmorHitpoints = dctParams.getFloat("fltCurrentArmorHitpoints");
		
		String strComponentType = getShipComponentStringType(objComponent);
		int armorfactor = 2;
		
		if (strComponentType.equals("armor"))
		{
			armorfactor = 1;
		}
		
		float fltArmor = dctParams.getFloat("fltMaximumArmorHitpoints");
		float fltArmorMod = dctParams.getFloat("fltMaximumArmorHitpointsMod");
		
		fltArmor = getBellValue(fltArmor, fltArmorMod);
		setComponentCurrentArmorHitpoints(objComponent, fltArmor);
		setComponentMaximumArmorHitpoints(objComponent, fltArmor);
		setComponentMaximumConduitHitpoints(objComponent, fltArmor);
		setComponentCurrentConduitHitpoints(objComponent, fltArmor);
		setComponentCurrentHitpoints(objComponent, fltArmor);
		setComponentMaximumHitpoints(objComponent, fltArmor);
		
		setComponentMass(objComponent, fltMass);
		setComponentEnergyMaintenance(objComponent, fltEnergyMaintenance);
		
		setComponentEnergyEfficiency(objComponent, fltEfficiency);
		setComponentGeneralEfficiency(objComponent, fltEfficiency);
		
		return objComponent;
	}
	
	
	public static int getReverseEngineeringLevel(obj_id objComponent) throws InterruptedException
	{
		if (hasObjVar(objComponent, "character.builder"))
		{
			return SCRIPT_CONTINUE;
		}
		
		String strComponentType = getShipComponentStringType(objComponent);
		
		LOG("CARGO_HOLD", "COMPONENT IS OF TYPE "+ strComponentType);
		
		if ((strComponentType != null) && (!strComponentType.equals("")))
		{
			dictionary dctParams = dataTableGetRow("datatables/ship/components/"+ strComponentType + ".iff", getTemplateName(objComponent));
			
			if (dctParams == null)
			{
				LOG("ERROR", "TEMPLATE OF TYPE "+ getTemplateName(objComponent) + " HAS BEEN PASSED TO SETUP SPACE COMPONENT. THIS DOES NOT EXIST IN THE DATATBLE of "+ "datatables/ship/components/"+ strComponentType + ".iff");
				return 0;
			}
			return dctParams.getInt("reverseEngineeringLevel");
		}
		else
		{
			LOG("space", "MEGA FUCKUP WITH "+ objComponent + " template is "+ objComponent);
		}
		return 0;
	}
	
	
	public static obj_id initializeSpaceShipComponent(obj_id objComponent) throws InterruptedException
	{
		
		String strComponentType = getShipComponentStringType(objComponent);
		
		if (strComponentType == null)
		{
			LOG("space", "MAJOR FUCKUP! "+ objComponent + " is 9 kinds of fucked up");
			setName(objComponent, "BAD COMPONENT: "+ getTemplateName(objComponent) + " : PLEASE REPORT BUG");
			return null;
		}
		
		if (!strComponentType.equals(""))
		{
			
			dictionary dctParams = dataTableGetRow("datatables/ship/components/"+ strComponentType + ".iff", getTemplateName(objComponent));
			
			if (dctParams == null)
			{
				LOG("ERROR", "TEMPLATE OF TYPE "+ getTemplateName(objComponent) + " HAS BEEN PASSED TO SETUP SPACE COMPONENT. THIS DOES NOT EXIST IN THE DATATBLE of "+ "datatables/ship/components/"+ strComponentType + ".iff");
				
				return null;
			}
			
			objComponent = setupStandardStatistics(objComponent, dctParams);
			
			if (strComponentType.equals("armor"))
			{
				
			}
			else if (strComponentType.equals("booster"))
			{
				float fltCurrentEnergy = dctParams.getFloat("fltCurrentEnergy");
				
				float fltMaximumEnergyModifier = dctParams.getFloat("fltMaximumEnergyModifier");
				float fltMaximumEnergy = dctParams.getFloat("fltMaximumEnergy");
				float fltRoll = rand(-1 * fltMaximumEnergyModifier, fltMaximumEnergyModifier);
				
				fltMaximumEnergy = fltMaximumEnergy + fltRoll;
				fltCurrentEnergy = fltCurrentEnergy + fltRoll;
				
				setBoosterMaximumEnergy(objComponent, fltMaximumEnergy);
				setBoosterCurrentEnergy(objComponent, fltCurrentEnergy);
				
				float fltRechargeRateModifier = dctParams.getFloat("fltRechargeRateModifier");
				float fltRechargeRate = dctParams.getFloat("fltRechargeRate");
				
				fltRechargeRate = getBellValue(fltRechargeRate, fltRechargeRateModifier);
				
				setBoosterEnergyRechargeRate(objComponent, fltRechargeRate);
				
				float fltConsumptionRateModifier = dctParams.getFloat("fltConsumptionRateModifier");
				float fltConsumptionRate = dctParams.getFloat("fltConsumptionRate");
				
				fltConsumptionRate = getBellValue(fltConsumptionRate, fltConsumptionRateModifier);
				
				setBoosterEnergyConsumptionRate(objComponent, fltConsumptionRate);
				
				float fltAccelerationModifier = dctParams.getFloat("fltAccelerationModifier");
				float fltAcceleration = dctParams.getFloat("fltAcceleration");
				
				fltAcceleration = getBellValue(fltAcceleration, fltAccelerationModifier);
				
				setBoosterAcceleration(objComponent, fltAcceleration);
				
				float fltMaxSpeedModifier = dctParams.getFloat("fltMaxSpeedModifier");
				float fltMaxSpeed = dctParams.getFloat("fltMaxSpeed");
				
				fltMaxSpeed = getBellValue(fltMaxSpeed, fltMaxSpeedModifier);
				
				setBoosterMaximumSpeed(objComponent, fltMaxSpeed);
			}
			else if (strComponentType.equals("cargo_hold"))
			{
				int intCargoHoldCapacity = dctParams.getInt("intCargoHoldCapacity");
				
				setCargoHoldMaxCapacity(objComponent, intCargoHoldCapacity);
			}
			else if (strComponentType.equals("droid_interface"))
			{
				float fltCommandSpeedModifier = dctParams.getFloat("fltCommandSpeedModifier");
				float fltCommandSpeed = dctParams.getFloat("fltCommandSpeed");
				
				fltCommandSpeed = getBellValue(fltCommandSpeed, fltCommandSpeedModifier);
				
				setDroidInterfaceCommandSpeed(objComponent, fltCommandSpeed);
			}
			else if (strComponentType.equals("engine"))
			{
				float fltMaxSpeedModifier = dctParams.getFloat("fltMaxSpeedModifier");
				float fltMaxSpeed = dctParams.getFloat("fltMaxSpeed");
				
				fltMaxSpeed = getBellValue(fltMaxSpeed, fltMaxSpeedModifier);
				
				setEngineMaximumSpeed(objComponent, fltMaxSpeed);
				
				float fltMaxPitchModifier = dctParams.getFloat("fltMaxPitchModifier");
				float fltMaxPitch = dctParams.getFloat("fltMaxPitch");
				
				fltMaxPitch = getBellValue(fltMaxPitch, fltMaxPitchModifier);
				
				setEngineMaximumPitch(objComponent, fltMaxPitch);
				
				float fltMaxRollModifier = dctParams.getFloat("fltMaxRollModifier");
				float fltMaxRoll = dctParams.getFloat("fltMaxRoll");
				
				fltMaxRoll = getBellValue(fltMaxRoll, fltMaxRollModifier);
				
				setEngineMaximumRoll(objComponent, fltMaxRoll);
				
				float fltMaxYawModifier = dctParams.getFloat("fltMaxYawModifier");
				float fltMaxYaw = dctParams.getFloat("fltMaxYaw");
				
				fltMaxYaw = getBellValue(fltMaxYaw, fltMaxYawModifier);
				
				setEngineMaximumYaw(objComponent, fltMaxYaw);
			}
			else if (strComponentType.equals("reactor"))
			{
				float fltEnergyGenerationModifier = dctParams.getFloat("fltEnergyGenerationModifier");
				float fltEnergyGeneration = dctParams.getFloat("fltEnergyGeneration");
				
				fltEnergyGeneration = getBellValue(fltEnergyGeneration, fltEnergyGenerationModifier);
				
				setReactorEnergyGeneration(objComponent, fltEnergyGeneration);
			}
			else if (strComponentType.equals("shield"))
			{
				float fltShieldHitpointsMaximumFrontModifier = dctParams.getFloat("fltShieldHitpointsMaximumFrontModifier");
				float fltShieldHitpointsMaximumFront = dctParams.getFloat("fltShieldHitpointsMaximumFront");
				
				fltShieldHitpointsMaximumFront = getBellValue(fltShieldHitpointsMaximumFront, fltShieldHitpointsMaximumFrontModifier);
				
				float fltShieldHitpointsMaximumBackModifier = dctParams.getFloat("fltShieldHitpointsMaximumBackModifier");
				float fltShieldHitpointsMaximumBack = dctParams.getFloat("fltShieldHitpointsMaximumFront");
				
				fltShieldHitpointsMaximumBack = getBellValue(fltShieldHitpointsMaximumBack, fltShieldHitpointsMaximumBackModifier);
				
				setShieldGeneratorCurrentFrontHitpoints(objComponent, 0f);
				setShieldGeneratorCurrentBackHitpoints(objComponent, 0f);
				
				setShieldGeneratorMaximumFrontHitpoints(objComponent, fltShieldHitpointsMaximumFront);
				setShieldGeneratorMaximumBackHitpoints(objComponent, fltShieldHitpointsMaximumBack);
				float fltShieldRechargeRateModifier = dctParams.getFloat("fltShieldRechargeRateModifier");
				float fltShieldRechargeRate = dctParams.getFloat("fltShieldRechargeRate");
				
				fltShieldRechargeRate = getBellValue(fltShieldRechargeRate, fltShieldRechargeRateModifier);
				
				setShieldGeneratorRechargeRate(objComponent, fltShieldRechargeRate);
			}
			else if (strComponentType.equals("weapon"))
			{
				float fltMinDamageModifier = dctParams.getFloat("fltMinDamageModifier");
				float fltMinDamage = dctParams.getFloat("fltMinDamage");
				
				fltMinDamage = getBellValue(fltMinDamage, fltMinDamageModifier);
				
				setWeaponMinimumDamage(objComponent, fltMinDamage);
				
				float fltMaxDamageModifier = dctParams.getFloat("fltMaxDamageModifier");
				float fltMaxDamage = dctParams.getFloat("fltMaxDamage");
				
				fltMaxDamage = getBellValue(fltMaxDamage, fltMaxDamageModifier);
				
				setWeaponMaximumDamage(objComponent, fltMaxDamage);
				
				float fltShieldEffectivenessModifier = dctParams.getFloat("fltShieldEffectivenessModifier");
				float fltShieldEffectiveness = dctParams.getFloat("fltShieldEffectiveness");
				
				fltShieldEffectiveness = getBellValue(fltShieldEffectiveness, fltShieldEffectivenessModifier);
				
				setWeaponShieldEffectiveness(objComponent, fltShieldEffectiveness);
				
				float fltArmorEffectivenessModifier = dctParams.getFloat("fltArmorEffectivenessModifier");
				float fltArmorEffectiveness = dctParams.getFloat("fltArmorEffectiveness");
				
				fltArmorEffectiveness = getBellValue(fltArmorEffectiveness, fltArmorEffectivenessModifier);
				
				setWeaponArmorEffectiveness(objComponent, fltArmorEffectiveness);
				
				float fltEnergyPerShotModifier = dctParams.getFloat("fltEnergyPerShotModifier");
				float fltEnergyPerShot = dctParams.getFloat("fltEnergyPerShot");
				
				fltEnergyPerShot = getBellValue(fltEnergyPerShot, fltEnergyPerShotModifier);
				
				setWeaponEnergyPerShot(objComponent, fltEnergyPerShot);
				
				float fltRefireRateModifier = dctParams.getFloat("fltRefireRateModifier");
				float fltRefireRate = dctParams.getFloat("fltRefireRate");
				
				fltRefireRate = getBellValue(fltRefireRate, fltRefireRateModifier);
				
				setWeaponRefireRate(objComponent, fltRefireRate);
			}
			else if (strComponentType.equals("capacitor"))
			{
				float fltCurrentEnergy = dctParams.getFloat("fltCurrentEnergy");
				float fltMaxEnergyModifier = dctParams.getFloat("fltMaxEnergyModifier");
				float fltMaxEnergy = dctParams.getFloat("fltMaxEnergy");
				
				fltMaxEnergy = getBellValue(fltMaxEnergy, fltMaxEnergyModifier);
				
				setWeaponCapacitorMaximumEnergy(objComponent, fltMaxEnergy);
				setWeaponCapacitorCurrentEnergy(objComponent, fltMaxEnergy);
				
				float fltRechargeRateModifier = dctParams.getFloat("fltRechargeRateModifier");
				float fltRechargeRate = dctParams.getFloat("fltRechargeRate");
				
				fltRechargeRate = getBellValue(fltRechargeRate, fltRechargeRateModifier);
				
				setWeaponCapacitorRechargeRate(objComponent, fltRechargeRate);
			}
			
			return objComponent;
		}
		else
		{
			return objComponent;
		}
	}
	
	
	public static void uninstallAll(obj_id ship) throws InterruptedException
	{
		if (isIdValid(ship))
		{
			for (int i = 0; i < ship_chassis_slot_type.SCST_num_types; i++)
			{
				testAbortScript();
				if (isShipSlotInstalled(ship, i))
				{
					obj_id kill = shipUninstallComponent(null, ship, i, null);
				}
			}
		}
	}
	
	
	public static boolean setComponentCurrentHitpoints(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.hitpoints_current";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setComponentMaximumHitpoints(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.hitpoints_maximum";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setComponentCurrentArmorHitpoints(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.armor_hitpoints_current";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setComponentMaximumArmorHitpoints(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.armor_hitpoints_maximum";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setComponentGeneralEfficiency(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.efficiency_general";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setComponentEnergyEfficiency(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.efficiency_energy";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setComponentEnergyMaintenance(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.energy_maintenance_requirement";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setComponentMass(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.mass";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setComponentCurrentConduitHitpoints(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.conduit_hitpoints_current";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setComponentMaximumConduitHitpoints(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.conduit_hitpoints_maximum";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setBoosterCurrentEnergy(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.booster.energy_current";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setBoosterMaximumEnergy(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.booster.energy_maximum";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setBoosterEnergyRechargeRate(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.booster.energy_recharge_rate";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setBoosterEnergyConsumptionRate(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.booster.energy_consumption_rate";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setBoosterAcceleration(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.booster.acceleration";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setBoosterMaximumSpeed(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.booster.speed_maximum";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setCargoHoldMaxCapacity(obj_id objComponent, int intValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.cargo_hold.contents_maximum";
		
		return setObjVar(objComponent, OBJVAR_NAME, intValue);
	}
	
	
	public static boolean setCargoHoldCurrentContents(obj_id objComponent, int intValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.cargo_hold.contents_current";
		
		return setObjVar(objComponent, OBJVAR_NAME, intValue);
	}
	
	
	public static boolean setWeaponCapacitorCurrentEnergy(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.capacitor.energy_current";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setWeaponCapacitorMaximumEnergy(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.capacitor.energy_maximum";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setWeaponCapacitorRechargeRate(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.capacitor.energy_recharge_rate";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setDroidInterfaceCommandSpeed(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.droid_interface.command_speed";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setEngineAcceleration(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.acceleration_rate";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setEngineDeceleration(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.deceleration_rate";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setEnginePitch(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.pitch_acceleration_rate";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setEngineYaw(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.yaw_acceleration_rate";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setEngineRoll(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.roll_acceleration_rate";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setEngineSpeedRotationFactorMaximum(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.speed_rotation_factor_max";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setEngineSpeedRotationFactorMinimum(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.speed_rotation_factor_min";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setEngineSpeedRotationFactorOptimal(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.speed_rotation_factor_optimal";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setEngineMaximumPitch(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.pitch_rate_maximum";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setEngineMaximumYaw(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.yaw_rate_maximum";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setEngineMaximumRoll(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.roll_rate_maximum";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setEngineMaximumSpeed(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.speed_maximum";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setReactorEnergyGeneration(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.reactor.energy_generation_rate";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setShieldGeneratorCurrentFrontHitpoints(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.shield.hitpoints_front_current";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setShieldGeneratorMaximumFrontHitpoints(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.shield.hitpoints_front_maximum";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setShieldGeneratorCurrentBackHitpoints(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.shield.hitpoints_back_current";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setShieldGeneratorMaximumBackHitpoints(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.shield.hitpoints_back_maximum";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setShieldGeneratorRechargeRate(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.shield.recharge_rate";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setWeaponMaximumDamage(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.weapon.damage_maximum";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setWeaponMinimumDamage(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.weapon.damage_minimum";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setWeaponShieldEffectiveness(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.weapon.effectiveness_shields";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setWeaponArmorEffectiveness(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.weapon.effectiveness_armor";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setWeaponEnergyPerShot(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.weapon.energy_per_shot";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setWeaponRefireRate(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.weapon.refire_rate";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static boolean setWeaponProjectileSpeed(obj_id objComponent, float fltValue) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.weapon.projectile_speed";
		
		return setObjVar(objComponent, OBJVAR_NAME, fltValue);
	}
	
	
	public static float getComponentCurrentHitpoints(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.hitpoints_current";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getComponentMaximumHitpoints(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.hitpoints_maximum";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getComponentCurrentArmorHitpoints(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.armor_hitpoints_current";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getComponentMaximumArmorHitpoints(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.armor_hitpoints_maximum";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getComponentEnergyEfficiency(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.efficiency_energy";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getComponentGeneralEfficiency(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.efficiency_general";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getComponentEnergyMaintenance(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.energy_maintenance_requirement";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getComponentMass(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.mass";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getComponentCurrentConduitHitpoints(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.conduit_hitpoints_current";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getComponentMaximumConduitHitpoints(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.conduit_hitpoints_maximum";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getBoosterCurrentEnergy(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.booster.energy_current";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getBoosterMaximumEnergy(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.booster.energy_maximum";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getBoosterEnergyRechargeRate(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.booster.energy_recharge_rate";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getboosterEnergyConsumptionRate(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.booster.energy_consumption_rate";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getBoosterAcceleration(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.booster.acceleration";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getBoosterMaximumSpeed(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.booster.speed_maximum";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getWeaponCapacitorCurrentEnergy(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.capacitor.energy_current";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getWeaponCapacitorMaximumEnergy(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.capacitor.energy_maximum";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getWeaponCapacitorRechargeRate(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.capacitor.energy_recharge_rate";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getDroidInterfaceCommandSpeed(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.droid_interface.command_speed";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getEngineAcceleration(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.acceleration_rate";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getEngineDeceleration(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.deceleration_rate";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getEnginePitch(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.pitch_acceleration_rate";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getEngineYaw(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.yaw_acceleration_rate";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getEngineRoll(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.roll_acceleration_rate";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getEngineSpeedRotationFactorMaximum(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.speed_rotation_factor_max";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getEngineSpeedRotationFactorMinimum(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.speed_rotation_factor_min";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getEngineSpeedRotationFactorOptimal(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.speed_rotation_factor_optimal";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getEngineMaximumPitch(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.pitch_rate_maximum";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getEngineMaximumYaw(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.yaw_rate_maximum";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getEngineMaximumRoll(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.roll_rate_maximum";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getEngineMaximumSpeed(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.engine.speed_maximum";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getReactorEnergyGeneration(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.reactor.energy_generation_rate";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getShieldGeneratorCurrenFrontHitpointst(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.shield.hitpoints_front_current";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getShieldGeneratorMaximumFrontHitpoints(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.shield.hitpoints_front_maximum";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getShieldGeneratorCurrentBackHitpoints(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.shield.hitpoints_back_current";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getShieldGeneratorMaximumBackHitpoints(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.shield.hitpoints_back_maximum";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getShieldGeneratorRechargeRate(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.shield.recharge_rate";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getWeaponMaximumDamage(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.weapon.damage_maximum";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getWeaponMinimumDamage(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.weapon.damage_minimum";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getWeaponShieldEffectiveness(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.weapon.effectiveness_shields";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getWeaponArmorEffectiveness(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.weapon.effectiveness_armor";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getWeaponEnergyPerShot(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.weapon.energy_per_shot";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getWeaponRefireRate(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.weapon.refire_rate";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static float getWeaponProjectileSpeed(obj_id objComponent) throws InterruptedException
	{
		String OBJVAR_NAME = "ship_comp.weapon.projectile_speed";
		
		return getFloatObjVar(objComponent, OBJVAR_NAME);
	}
	
	
	public static Vector getRepairKitsForItem(obj_id objPlayer, int intSlot) throws InterruptedException
	{
		String strComponent = "";
		
		if (intSlot == CHASSIS)
		{
			strComponent = "chassis";
		}
		else if (intSlot == PLASMA_CONDUIT)
		{
			strComponent = "plasma_conduit";
		}
		else
		{
			int intType = ship_chassis_slot_type.getComponentTypeForSlot(intSlot);
			
			strComponent = getShipComponentStringType(intType);
			LOG("space", "strComponent is "+ strComponent);
		}
		
		return getRepairKitsForItem(objPlayer, strComponent);
	}
	
	
	public static Vector getRepairKitsForItem(obj_id objPlayer, String strComponent) throws InterruptedException
	{
		obj_id[] objContents = getInventoryAndEquipment(objPlayer);
		Vector objKits = new Vector();
		
		for (int intI = 0; intI < objContents.length; intI++)
		{
			testAbortScript();
			if (hasObjVar(objContents[intI], "strKitType"))
			{
				String strKitType = getStringObjVar(objContents[intI], "strKitType");
				
				if ((strKitType.equals(strComponent)) || (strKitType.equals("all_purpose")))
				{
					objKits = utils.addElement(objKits, objContents[intI]);
				}
			}
		}
		
		if (objKits.size() > 0)
		{
			return objKits;
		}
		
		return null;
	}
	
	
	public static boolean repairComponentOnShip(int intSlot, Vector objRepairKits, obj_id objPlayer, obj_id objShip) throws InterruptedException
	{
		
		if ((objRepairKits == null) || (objRepairKits.size() < 1))
		{
			return false;
		}
		
		boolean boolHealed = false;
		
		String strComponent = "";
		
		if (intSlot == CHASSIS)
		{
			strComponent = "chassis";
		}
		else
		{
			strComponent = getShipComponentStringType(intSlot);
		}
		
		obj_id objRepairKit = ((obj_id)(objRepairKits.get(0)));
		
		if (intSlot == CHASSIS)
		{
			
			float fltMaximumHitpoints = getShipMaximumChassisHitPoints(objShip);
			float fltCurrentHitpoints = getShipCurrentChassisHitPoints(objShip);
			
			if (fltCurrentHitpoints == fltMaximumHitpoints)
			{
				
				string_id strSpam = new string_id("space/space_interaction", "no_damage_to_repair");
				
				sendSystemMessage(objPlayer, strSpam);
				return false;
			}
			else
			{
				float fltDifference = fltMaximumHitpoints - fltCurrentHitpoints;
				float fltRemainingRepairPoints = (float) getCount(objRepairKit);
				
				if (fltDifference > fltRemainingRepairPoints)
				{
					
					fltCurrentHitpoints = fltCurrentHitpoints + fltRemainingRepairPoints;
					
					setShipCurrentChassisHitPoints(objShip, fltCurrentHitpoints);
					
					string_id strSpam = new string_id("space/space_interaction", "repaired_x_chassis_damage_destroy_kit");
					prose_package ppRepairInfo = prose.getPackage(strSpam, (int) fltRemainingRepairPoints);
					
					sendSystemMessageProse(objPlayer, ppRepairInfo);
					
					destroyObject(objRepairKit);
					
					objRepairKits = utils.removeElementAt(objRepairKits, 0);
					
					if (objRepairKits.size() > 0)
					{
						repairComponentOnShip(intSlot, objRepairKits, objPlayer, objShip);
						return true;
					}
					else
					{
						strSpam = new string_id("space/space_interaction", "damage_left_chassis");
						int intDifference = (int) fltDifference - (int) fltRemainingRepairPoints;
						
						ppRepairInfo = prose.getPackage(strSpam, intDifference);
						sendSystemMessageProse(objPlayer, ppRepairInfo);
						return true;
					}
					
				}
				else
				{
					fltCurrentHitpoints = fltCurrentHitpoints + fltDifference;
					setShipCurrentChassisHitPoints(objShip, fltCurrentHitpoints);
					
					string_id strSpam = new string_id("space/space_interaction", "repaired_x_chassis_damage");
					prose_package ppRepairInfo = prose.getPackage(strSpam, (int) fltDifference);
					
					sendSystemMessageProse(objPlayer, ppRepairInfo);
					
					fltRemainingRepairPoints = fltRemainingRepairPoints - fltDifference;
					setCount(objRepairKit, (int) fltRemainingRepairPoints);
					
					return true;
				}
			}
		}
		
		float fltMaximumHitpoints = getShipComponentHitpointsMaximum(objShip, intSlot);
		float fltCurrentHitpoints = getShipComponentHitpointsCurrent(objShip, intSlot);
		
		float fltMaximumArmorHitpoints = getShipComponentArmorHitpointsMaximum(objShip, intSlot);
		float fltCurrentArmorHitpoints = getShipComponentArmorHitpointsCurrent(objShip, intSlot);
		
		float fltEnergyEfficiency = getShipComponentEfficiencyEnergy(objShip, intSlot);
		float fltGeneralEfficiency = getShipComponentEfficiencyGeneral(objShip, intSlot);
		
		if ((fltCurrentHitpoints == fltMaximumHitpoints) && (fltMaximumArmorHitpoints == fltCurrentArmorHitpoints))
		{
			string_id strSpam = new string_id("space/space_interaction", "no_damage_to_repair");
			
			sendSystemMessage(objPlayer, strSpam);
			return false;
		}
		
		float fltRemainingRepairPoints = (float) getCount(objRepairKit);
		float fltDifference = fltMaximumHitpoints - fltCurrentHitpoints;
		
		if (fltDifference > fltRemainingRepairPoints)
		{
			
			fltCurrentHitpoints = fltCurrentHitpoints + fltRemainingRepairPoints;
			
			setShipComponentHitpointsCurrent(objShip, intSlot, fltCurrentHitpoints);
			
			string_id strSpam = new string_id("space/space_interaction", "repaired_x_component_damage_destroy_kit");
			prose_package ppRepairInfo = prose.getPackage(strSpam, getShipComponentName(objShip, intSlot), (int) fltRemainingRepairPoints);
			
			sendSystemMessageProse(objPlayer, ppRepairInfo);
			
			destroyObject(objRepairKit);
			space_utils.setComponentDisabled(objShip, intSlot, false);
			space_combat.recalculateEfficiency(intSlot, objShip);
			objRepairKits = utils.removeElementAt(objRepairKits, 0);
			
			if (objRepairKits.size() > 0)
			{
				repairComponentOnShip(intSlot, objRepairKits, objPlayer, objShip);
				return true;
			}
			else
			{
				strSpam = new string_id("space/space_interaction", "damage_left_component");
				int intDifference = (int) fltDifference - (int) fltRemainingRepairPoints;
				
				ppRepairInfo = prose.getPackage(strSpam, intDifference);
				sendSystemMessageProse(objPlayer, ppRepairInfo);
				return true;
			}
		}
		else if (fltDifference > 0)
		{
			
			setShipComponentHitpointsCurrent(objShip, intSlot, fltMaximumHitpoints);
			
			string_id strSpam = new string_id("space/space_interaction", "repaired_x_component_damage");
			prose_package ppRepairInfo = prose.getPackage(strSpam, getShipComponentName(objShip, intSlot), (int) fltDifference);
			
			sendSystemMessageProse(objPlayer, ppRepairInfo);
			
			fltRemainingRepairPoints = fltRemainingRepairPoints - fltDifference;
			setCount(objRepairKit, (int) fltRemainingRepairPoints);
			space_utils.setComponentDisabled(objShip, intSlot, false);
			space_combat.recalculateEfficiency(intSlot, objShip);
			boolHealed = true;
		}
		
		fltDifference = fltMaximumArmorHitpoints - fltCurrentArmorHitpoints;
		
		if (fltDifference > fltRemainingRepairPoints)
		{
			
			fltCurrentHitpoints = fltCurrentHitpoints + fltRemainingRepairPoints;
			setShipComponentArmorHitpointsCurrent(objShip, intSlot, fltCurrentHitpoints);
			
			string_id strSpam = new string_id("space/space_interaction", "repaired_x_armor_component_damage_destroy_kit");
			prose_package ppRepairInfo = prose.getPackage(strSpam, getShipComponentName(objShip, intSlot), (int) fltRemainingRepairPoints);
			
			sendSystemMessageProse(objPlayer, ppRepairInfo);
			
			destroyObject(objRepairKit);
			space_utils.setComponentDisabled(objShip, intSlot, false);
			space_combat.recalculateEfficiency(intSlot, objShip);
			objRepairKits = utils.removeElementAt(objRepairKits, 0);
			
			if (objRepairKits.size() > 0)
			{
				repairComponentOnShip(intSlot, objRepairKits, objPlayer, objShip);
				return true;
			}
			else
			{
				strSpam = new string_id("space/space_interaction", "damage_left_component_armor");
				int intDifference = (int) fltDifference - (int) fltRemainingRepairPoints;
				
				ppRepairInfo = prose.getPackage(strSpam, intDifference);
				sendSystemMessageProse(objPlayer, ppRepairInfo);
				return true;
			}
			
		}
		else if (fltDifference > 0)
		{
			
			setShipComponentArmorHitpointsCurrent(objShip, intSlot, fltMaximumArmorHitpoints);
			
			string_id strSpam = new string_id("space/space_interaction", "repaired_x_armor_component_damage");
			prose_package ppRepairInfo = prose.getPackage(strSpam, getShipComponentName(objShip, intSlot), (int) fltDifference);
			
			sendSystemMessageProse(objPlayer, ppRepairInfo);
			
			fltRemainingRepairPoints = fltRemainingRepairPoints - fltDifference;
			setCount(objRepairKit, (int) fltRemainingRepairPoints);
			space_utils.setComponentDisabled(objShip, intSlot, false);
			space_combat.recalculateEfficiency(intSlot, objShip);
			boolHealed = true;
		}
		
		return boolHealed;
	}
	
	
	public static boolean repairComponentInInventory(obj_id objComponent, Vector objRepairKits, obj_id objPlayer) throws InterruptedException
	{
		
		if ((objRepairKits == null) || (objRepairKits.size() < 1))
		{
			return false;
		}
		
		obj_id objRepairKit = ((obj_id)(objRepairKits.get(0)));
		boolean boolHealed = false;
		String strComponent = getShipComponentStringType(objComponent);
		
		float fltMaximumHitpoints = getComponentMaximumHitpoints(objComponent);
		float fltCurrentHitpoints = getComponentCurrentHitpoints(objComponent);
		
		float fltMaximumArmorHitpoints = getComponentMaximumArmorHitpoints(objComponent);
		float fltCurrentArmorHitpoints = getComponentCurrentArmorHitpoints(objComponent);
		
		float fltEnergyEfficiency = getComponentEnergyEfficiency(objComponent);
		float fltGeneralEfficiency = getComponentGeneralEfficiency(objComponent);
		
		if ((fltCurrentHitpoints == fltMaximumHitpoints) && (fltMaximumArmorHitpoints == fltCurrentArmorHitpoints))
		{
			string_id strSpam = new string_id("space/space_interaction", "no_damage_to_repair");
			
			sendSystemMessage(objPlayer, strSpam);
			return false;
		}
		
		float fltRemainingRepairPoints = (float) getCount(objRepairKit);
		float fltDifference = fltMaximumHitpoints - fltCurrentHitpoints;
		
		if (fltDifference > fltRemainingRepairPoints)
		{
			
			fltCurrentHitpoints = fltCurrentHitpoints + fltRemainingRepairPoints;
			setComponentCurrentHitpoints(objComponent, fltCurrentHitpoints);
			clearComponentDisabledFlag(objComponent);
			
			string_id strSpam = new string_id("space/space_interaction", "repaired_x_component_damage_destroy_kit");
			prose_package ppRepairInfo = prose.getPackage(strSpam, getEncodedName(objComponent), (int) fltRemainingRepairPoints);
			
			sendSystemMessageProse(objPlayer, ppRepairInfo);
			
			objRepairKits = utils.removeElementAt(objRepairKits, 0);
			destroyObject(objRepairKit);
			repairComponentInInventory(objComponent, objRepairKits, objPlayer);
			return true;
		}
		else if (fltDifference > 0)
		{
			
			setComponentCurrentHitpoints(objComponent, fltMaximumHitpoints);
			clearComponentDisabledFlag(objComponent);
			string_id strSpam = new string_id("space/space_interaction", "repaired_x_component_damage");
			prose_package ppRepairInfo = prose.getPackage(strSpam, getEncodedName(objComponent), (int) fltDifference);
			
			sendSystemMessageProse(objPlayer, ppRepairInfo);
			
			fltRemainingRepairPoints = fltRemainingRepairPoints - fltDifference;
			setCount(objRepairKit, (int) fltRemainingRepairPoints);
			boolHealed = true;
		}
		
		fltDifference = fltMaximumArmorHitpoints - fltCurrentArmorHitpoints;
		
		if (fltDifference > fltRemainingRepairPoints)
		{
			
			fltCurrentArmorHitpoints = fltCurrentArmorHitpoints + fltRemainingRepairPoints;
			setComponentCurrentArmorHitpoints(objComponent, fltCurrentArmorHitpoints);
			
			string_id strSpam = new string_id("space/space_interaction", "repaired_x_armor_component_damage_destroy_kit");
			prose_package ppRepairInfo = prose.getPackage(strSpam, getEncodedName(objComponent), (int) fltRemainingRepairPoints);
			
			sendSystemMessageProse(objPlayer, ppRepairInfo);
			
			objRepairKits = utils.removeElementAt(objRepairKits, 0);
			destroyObject(objRepairKit);
			repairComponentInInventory(objComponent, objRepairKits, objPlayer);
			return true;
		}
		else if (fltDifference > 0)
		{
			
			setComponentCurrentArmorHitpoints(objComponent, fltMaximumArmorHitpoints);
			
			string_id strSpam = new string_id("space/space_interaction", "repaired_x_armor_component_damage");
			prose_package ppRepairInfo = prose.getPackage(strSpam, getEncodedName(objComponent), (int) fltDifference);
			
			sendSystemMessageProse(objPlayer, ppRepairInfo);
			
			fltRemainingRepairPoints = fltRemainingRepairPoints - fltDifference;
			setCount(objRepairKit, (int) fltRemainingRepairPoints);
			boolHealed = true;
		}
		return boolHealed;
	}
	
	
	public static boolean isRepairKit(obj_id objItem) throws InterruptedException
	{
		String strTest = getTemplateName(objItem);
		int intIndex = strTest.indexOf("object/tangible/ship/crafted/repair");
		
		if (intIndex < 0)
		{
			return false;
		}
		
		return true;
	}
	
	
	public static obj_id getDroidContainer(obj_id objShip) throws InterruptedException
	{
		return getObjectInSlot(objShip, SHIP_DROID_SLOT_NAME);
	}
	
	
	public static obj_id[] getDatapadDroidCommands(obj_id objControlDevice) throws InterruptedException
	{
		obj_id objDatapad = utils.getDatapad(objControlDevice);
		
		if (!isIdValid(objDatapad))
		{
			debugServerConsoleMsg(null, "SPACE_CRAFTING.getDatapadDroidCommands *** DATAPAD OBJ-ID WAS NULL!!! ARGH! ");
			return null;
		}
		
		obj_id[] objContents = utils.getContents(objDatapad);
		Vector objCommands = new Vector();
		objCommands.setSize(0);
		
		for (int intI = 0; intI < objContents.length; intI++)
		{
			testAbortScript();
			if (hasObjVar(objContents[intI], "strDroidCommand"))
			{
				objCommands = utils.addElement(objCommands, objContents[intI]);
			}
		}
		
		if (objCommands.size() > 0)
		{
			obj_id[] _objCommands = new obj_id[0];
			if (objCommands != null)
			{
				_objCommands = new obj_id[objCommands.size()];
				objCommands.toArray(_objCommands);
			}
			return _objCommands;
		}
		else
		{
			return null;
		}
	}
	
	
	public static int getDroidCommandLevelFromDatapad(obj_id objControlDevice) throws InterruptedException
	{
		obj_id objDatapad = utils.getDatapad(objControlDevice);
		
		if (!isIdValid(objDatapad))
		{
			return 1;
		}
		
		String strName = getTemplateName(objDatapad);
		String strRaw = "object/tangible/datapad/droid_datapad_";
		
		for (int intI = 1; intI <= 6; intI++)
		{
			testAbortScript();
			String strTest = strRaw + intI + ".iff";
			
			if (strName.equals(strTest))
			{
				return intI;
			}
		}
		
		return -1;
	}
	
	
	public static boolean isUsableAstromechPet(obj_id objControlDevice) throws InterruptedException
	{
		int intDroidLevel = getDroidCommandLevelFromDatapad(objControlDevice);
		
		if (intDroidLevel == -1)
		{
			return false;
		}
		
		String[] strDroidTypes = 
		{
			"r2",
			"r2_crafted",
			"r2d2",
			"r3",
			"r3_crafted",
			"r4",
			"r4_crafted",
			"r5",
			"r5_crafted",
			"navicomputer_1",
			"navicomputer_2",
			"navicomputer_3",
			"navicomputer_4",
			"navicomputer_5",
			"navicomputer_6",
		};
		
		String strName = getStringObjVar(objControlDevice, "pet.creatureName");
		
		if (strName != null)
		{
			int intIndex = utils.getElementPositionInArray(strDroidTypes, strName);
			
			LOG("space", "intIndex: "+ intIndex + " strName: "+ strName);
			
			if (intIndex > -1)
			{
				return true;
			}
		}
		else
		{
			if (hasObjVar(objControlDevice, "item.objectName"))
			{
				strName = getStringObjVar(objControlDevice, "item.objectName");
				int intIndex = utils.getElementPositionInArray(strDroidTypes, strName);
				
				LOG("space", "intIndex: "+ intIndex + " strName: "+ strName);
				
				if (intIndex > -1)
				{
					return true;
				}
			}
			else
			{
				LOG("space", "NULL!");
			}
		}
		return false;
	}
	
	
	public static boolean isFlightComputerController(obj_id controlDevice) throws InterruptedException
	{
		String[] computerTypes = 
		{
			"navicomputer_1",
			"navicomputer_2",
			"navicomputer_3",
			"navicomputer_4",
			"navicomputer_5",
			"navicomputer_6",
		};
		
		if (hasObjVar(controlDevice, "item.objectName"))
		{
			String name = getStringObjVar(controlDevice, "item.objectName");
			int intIndex = utils.getElementPositionInArray(computerTypes, name);
			
			if (intIndex > -1)
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public static boolean isCertifiedForAstromech(obj_id objControlDevice, obj_id objPlayer) throws InterruptedException
	{
		
		int intDroidLevel = getDroidCommandLevelFromDatapad(objControlDevice);
		
		LOG("space", "level is "+ intDroidLevel + " Looking for command cert_droid_astromech_"+ intDroidLevel);
		
		if (!hasCommand(objPlayer, "cert_droid_astromech_" + intDroidLevel))
		{
			LOG("space", objPlayer + " doesnt have the cert!");
			return false;
		}
		
		return true;
	}
	
	
	public static obj_id getAstromechDroidForShip(obj_id objPlayer) throws InterruptedException
	{
		obj_id objDataPad = utils.getDatapad(objPlayer);
		
		obj_id[] objContents = utils.getContents(objDataPad);
		
		if (objContents == null)
		{
			return null;
		}
		
		for (int intI = 0; intI < objContents.length; intI++)
		{
			testAbortScript();
			
			if (hasObjVar(objContents[intI], "objActiveShipDroid"))
			{
				obj_id objTest = getObjIdObjVar(objContents[intI], "objActiveShipDroid");
				
				if (objTest != objPlayer)
				{
					
					removeObjVar(objTest, "objActiveShipDroid");
				}
				else
				{
					return objContents[intI];
				}
			}
		}
		return null;
	}
	
	
	public static boolean isUsingCorrectComputer(obj_id objControlDevice, obj_id ship) throws InterruptedException
	{
		if (!usesFlightComputerNotAstromech(ship) && hasObjVar(objControlDevice, "pet.creatureName"))
		{
			return true;
		}
		else if (usesFlightComputerNotAstromech(ship) && !hasObjVar(objControlDevice, "pet.creatureName"))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean usesFlightComputerNotAstromech(obj_id ship) throws InterruptedException
	{
		
		String templateName = utils.getTemplateFilenameNoPath(ship);
		String shipUsesThis = dataTableGetString(STARSHIP_DROID_TABLE, templateName, "usesDroidOrFlightComputer");
		
		debugServerConsoleMsg(null, "+++ SPACE_CRAFTING . Got shipUsesThis. It was: "+ shipUsesThis);
		
		if ((shipUsesThis.equals("")) || (shipUsesThis == null))
		{
			debugServerConsoleMsg(null, "SPACE_CRAFTING.usesFlightComputerNotAstromech: shipUsesThis failed out as being -1");
			return false;
		}
		
		if (shipUsesThis.equals("flight_computer"))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isMissileSlot(obj_id objShip, int intSlot) throws InterruptedException
	{
		if (isShipSlotInstalled(objShip, intSlot))
		{
			int intCrc = getShipComponentCrc(objShip, intSlot);
			
			return getShipComponentDescriptorWeaponIsMissile(intCrc);
		}
		return false;
		
	}
	
	
	public static boolean isCounterMeasureSlot(obj_id objShip, int intSlot) throws InterruptedException
	{
		if (isShipSlotInstalled(objShip, intSlot))
		{
			int intCrc = getShipComponentCrc(objShip, intSlot);
			
			return getShipComponentDescriptorWeaponIsCountermeasure(intCrc);
		}
		return false;
	}
	
	
	public static boolean isProperAmmoForWeapon(obj_id objAmmo, obj_id objShip, int intSlot) throws InterruptedException
	{
		
		int intWeaponAmmoType = getShipWeaponAmmoType(objShip, intSlot);
		int intWeaponType = getIntObjVar(objAmmo, "weapon.intAmmoType");
		
		if (intWeaponAmmoType != intWeaponType)
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean applyAmmoToWeapon(obj_id objShip, obj_id objAmmo, int intSlot, obj_id objPlayer, boolean boolVerbose) throws InterruptedException
	{
		
		int intCurrentAmmo = getShipWeaponAmmoCurrent(objShip, intSlot);
		int intCount = getCount(objAmmo);
		
		float fltMinDamage = getFloatObjVar(objAmmo, "fltMinDamage");
		float fltMaxDamage = getFloatObjVar(objAmmo, "fltMaxDamage");
		float fltRefireRate = getFloatObjVar(objAmmo, "fltRefireRate");
		float fltWeaponEffectivenessShields = getFloatObjVar(objAmmo, "fltShieldEffectiveness");
		float fltWeaponEffectivenessArmor = getFloatObjVar(objAmmo, "fltArmorEffectiveness");
		
		setShipWeaponDamageMaximum(objShip, intSlot, fltMaxDamage);
		setShipWeaponDamageMinimum(objShip, intSlot, fltMinDamage);
		setShipWeaponRefireRate(objShip, intSlot, fltRefireRate);
		setShipWeaponEffectivenessArmor(objShip, intSlot, fltWeaponEffectivenessArmor);
		setShipWeaponEffectivenessShields(objShip, intSlot, fltWeaponEffectivenessShields);
		setShipWeaponAmmoMaximum(objShip, intSlot, intCount);
		
		setShipWeaponAmmoCurrent(objShip, intSlot, intCount);
		
		string_id strSpam = new string_id("space/space_interaction", "reloaded_x_missile_ammo");
		prose_package ppRepairInfo = prose.getPackage(strSpam, getName(objAmmo), (int) intCount);
		
		sendSystemMessageProse(objPlayer, ppRepairInfo);
		
		destroyObject(objAmmo);
		
		return true;
		
	}
	
	
	public static boolean applyCountermeasuresToLauncher(obj_id objAmmo, obj_id objShip, int intSlot, obj_id objPlayer, boolean boolVerbose) throws InterruptedException
	{
		
		int intCurrentAmmo = getShipWeaponAmmoCurrent(objShip, intSlot);
		
		if (intCurrentAmmo != 0)
		{
			if (boolVerbose)
			{
				
				string_id strSpam = new string_id("space/space_interaction", "countermeasure_full_no_reload");
				
				sendSystemMessage(objPlayer, strSpam);
			}
			return false;
		}
		
		int intCount = getCount(objAmmo);
		
		float fltMinDamage = getFloatObjVar(objAmmo, "fltMinDamage");
		float fltMaxDamage = getFloatObjVar(objAmmo, "fltMaxDamage");
		float fltRefireRate = getFloatObjVar(objAmmo, "fltRefireRate");
		float fltEnergyPerShot = getFloatObjVar(objAmmo, "fltEnergyPerShot");
		
		setShipWeaponDamageMaximum(objShip, intSlot, fltMaxDamage);
		setShipWeaponDamageMinimum(objShip, intSlot, fltMinDamage);
		setShipWeaponRefireRate(objShip, intSlot, fltRefireRate);
		setShipWeaponAmmoMaximum(objShip, intSlot, intCount);
		setShipWeaponAmmoCurrent(objShip, intSlot, intCount);
		setShipWeaponEnergyPerShot(objShip, intSlot, fltEnergyPerShot);
		
		string_id strSpam = new string_id("space/space_interaction", "reloaded_x_countermeasure_ammo");
		prose_package ppRepairInfo = prose.getPackage(strSpam, getName(objAmmo), (int) intCount);
		
		sendSystemMessageProse(objPlayer, ppRepairInfo);
		
		destroyObject(objAmmo);
		
		return true;
		
	}
	
	
	public static boolean isWeaponAmmo(obj_id objAmmo) throws InterruptedException
	{
		if (hasObjVar(objAmmo, "weapon.intAmmoType"))
		{
			return true;
		}
		return false;
	}
	
	
	public static boolean isDamaged(obj_id objPlayer) throws InterruptedException
	{
		float fltDamage = getDamageTotalModified(space_transition.getContainingShip(objPlayer), 1.0f);
		
		if (fltDamage > 0)
		{
			return true;
		}
		return false;
	}
	
	
	public static float getDamageTotal(obj_id objPlayer, obj_id objShip) throws InterruptedException
	{
		return getDamageTotalModified(objShip, 1.0f);
	}
	
	
	public static float getDamageTotalModified(obj_id objShip, float fltPercentage) throws InterruptedException
	{
		
		float fltMaximumHitpoints = getShipMaximumChassisHitPoints(objShip);
		float fltCurrentHitpoints = getShipCurrentChassisHitPoints(objShip);
		
		float fltDifference = fltMaximumHitpoints - fltCurrentHitpoints;
		float fltRepairValue = 0;
		
		fltRepairValue = fltRepairValue + fltDifference;
		int[] intSlots = getShipInstalledSlots(objShip);
		
		for (int intI = 0; intI < intSlots.length; intI++)
		{
			testAbortScript();
			
			fltMaximumHitpoints = getShipComponentHitpointsMaximum(objShip, intSlots[intI]);
			fltCurrentHitpoints = getShipComponentHitpointsCurrent(objShip, intSlots[intI]);
			
			float fltMaximumArmorHitpoints = getShipComponentArmorHitpointsMaximum(objShip, intSlots[intI]);
			float fltCurrentArmorHitpoints = getShipComponentArmorHitpointsCurrent(objShip, intSlots[intI]);
			
			fltDifference = fltMaximumHitpoints - fltCurrentHitpoints;
			fltRepairValue = fltRepairValue + fltDifference;
			fltDifference = fltMaximumArmorHitpoints - fltCurrentArmorHitpoints;
			fltRepairValue = fltRepairValue + fltDifference;
		}
		
		fltRepairValue = fltRepairValue * fltPercentage;
		LOG("space", "Raw damage pre % of "+ fltPercentage + "is "+ fltRepairValue + " Modified damage POst % of "+ fltPercentage + " is "+ fltRepairValue);
		
		return fltRepairValue;
	}
	
	
	public static boolean repairDamageToPercentage(obj_id objPlayer, obj_id objShip, float fltPercentage) throws InterruptedException
	{
		LOG("space", "Repairing ship of "+ objPlayer + " to "+ fltPercentage);
		
		if (fltPercentage > 1.0f)
		{
			fltPercentage = 1.0f;
		}
		
		float fltMaximumHitpoints = getShipMaximumChassisHitPoints(objShip);
		float fltCurrentHitpoints = getShipCurrentChassisHitPoints(objShip);
		
		float fltDifference = fltMaximumHitpoints - fltCurrentHitpoints;
		
		LOG("space", "Chassis difference is "+ fltDifference);
		fltDifference = fltDifference * fltPercentage;
		
		if (fltDifference > 0)
		{
			fltCurrentHitpoints = fltCurrentHitpoints + fltDifference;
			setShipCurrentChassisHitPoints(objShip, fltCurrentHitpoints);
		}
		
		fltDifference = 0;
		
		int[] intSlots = getShipInstalledSlots(objShip);
		
		for (int intI = 0; intI < intSlots.length; intI++)
		{
			testAbortScript();
			
			fltMaximumHitpoints = getShipComponentHitpointsMaximum(objShip, intSlots[intI]);
			fltCurrentHitpoints = getShipComponentHitpointsCurrent(objShip, intSlots[intI]);
			
			float fltMaximumArmorHitpoints = getShipComponentArmorHitpointsMaximum(objShip, intSlots[intI]);
			float fltCurrentArmorHitpoints = getShipComponentArmorHitpointsCurrent(objShip, intSlots[intI]);
			
			float fltEnergyEfficiency = getShipComponentEfficiencyEnergy(objShip, intSlots[intI]);
			float fltGeneralEfficiency = getShipComponentEfficiencyGeneral(objShip, intSlots[intI]);
			
			fltDifference = fltMaximumHitpoints - fltCurrentHitpoints;
			fltDifference = fltDifference * fltPercentage;
			
			LOG("space", "RAW for "+ fltDifference + " worth of points on "+ intSlots[intI] + " Repairing "+ fltDifference + " worth of points");
			
			if (fltDifference > 0)
			{
				fltCurrentHitpoints = fltCurrentHitpoints + fltDifference;
				setShipComponentHitpointsCurrent(objShip, intSlots[intI], fltCurrentHitpoints);
			}
			else
			{
				clearCondition(objShip, CONDITION_EJECT);
				space_utils.setComponentDisabled(objShip, intSlots[intI], false);
			}
			
			fltDifference = 0;
			
			fltDifference = fltMaximumArmorHitpoints - fltCurrentArmorHitpoints;
			fltDifference = fltDifference * fltPercentage;
			
			LOG("space", "RAW ARMOR for "+ fltDifference + " worth of points on "+ intSlots[intI] + " Repairing "+ fltDifference + " worth of points");
			
			if (fltDifference > 0)
			{
				fltCurrentArmorHitpoints = fltCurrentArmorHitpoints + fltDifference;
				
				setShipComponentArmorHitpointsCurrent(objShip, intSlots[intI], fltCurrentArmorHitpoints);
				
				clearCondition(objShip, CONDITION_EJECT);
				space_utils.setComponentDisabled(objShip, intSlots[intI], false);
			}
			
			space_combat.recalculateEfficiency(intSlots[intI], objShip);
		}
		
		space_combat.normalizeAllComponents(objShip);
		
		return true;
	}
	
	
	public static boolean repairDamage(obj_id objPlayer, obj_id objShip, float fltPercentage) throws InterruptedException
	{
		LOG("space", "Calling repairDamage ");
		return repairDamage(objPlayer, objShip, fltPercentage, 0.0f);
	}
	
	
	public static boolean repairDamage(obj_id objPlayer, obj_id objShip, float fltPercentage, float decayRate) throws InterruptedException
	{
		return repairDamageToPercentage(objPlayer, objShip, fltPercentage);
	}
	
	
	public static int[] getShipInstalledSlots(obj_id objShip) throws InterruptedException
	{
		int[] intRawSlots = getShipChassisSlots(objShip);
		Vector intInstalledSlots = new Vector();
		intInstalledSlots.setSize(0);
		
		for (int intI = 0; intI < intRawSlots.length; intI++)
		{
			testAbortScript();
			if (isShipSlotInstalled(objShip, intRawSlots[intI]))
			{
				intInstalledSlots = utils.addElement(intInstalledSlots, intRawSlots[intI]);
			}
		}
		
		int[] _intInstalledSlots = new int[0];
		if (intInstalledSlots != null)
		{
			_intInstalledSlots = new int[intInstalledSlots.size()];
			for (int _i = 0; _i < intInstalledSlots.size(); ++_i)
			{
				_intInstalledSlots[_i] = ((Integer)intInstalledSlots.get(_i)).intValue();
			}
		}
		return _intInstalledSlots;
	}
	
	
	public static boolean canAffordShipRepairs(obj_id objPlayer, obj_id objStation, float fltPercentage) throws InterruptedException
	{
		obj_id objShip = space_transition.getContainingShip(objPlayer);
		
		if (!isIdValid(objShip))
		{
			return false;
		}
		
		float fltDamageTotal = getDamageTotal(objPlayer, objShip);
		
		fltDamageTotal = fltDamageTotal * fltPercentage;
		float fltCostPerPoint = getFloatObjVar(objStation, "fltCostPerDamagePoint");
		
		int intTotalCost = (int) (fltCostPerPoint * fltDamageTotal);
		
		if (space_utils.isBasicShip(objShip) || space_utils.isPrototypeShip(objShip))
		{
			
			intTotalCost = (int) (fltCostPerPoint * (100.0f * fltPercentage));
		}
		
		if (money.hasFunds(objPlayer, money.MT_TOTAL, intTotalCost))
		{
			LOG("space", "CANAFFORD");
			return true;
		}
		
		return false;
	}
	
	
	public static int getStationRepairCost(obj_id objPlayer, obj_id objStation, float fltPercentage) throws InterruptedException
	{
		obj_id objShip = space_transition.getContainingShip(objPlayer);
		
		if (!isIdValid(objShip))
		{
			LOG("Space", "Returning asdsa");
			return 0;
		}
		
		float fltDamageTotal = getDamageTotal(objPlayer, objShip);
		
		LOG("space", "Total 1 damage i s"+ fltDamageTotal);
		float fltCostPerPoint = getFloatObjVar(objStation, "fltCostPerDamagePoint");
		
		fltDamageTotal = fltDamageTotal * fltPercentage;
		LOG("space", "Total damage 2 is"+ fltDamageTotal);
		
		int intTotalCost = (int) (fltCostPerPoint * fltDamageTotal);
		
		if (space_utils.isBasicShip(objShip) || space_utils.isPrototypeShip(objShip))
		{
			
			intTotalCost = (int) (fltCostPerPoint * (100.0f * fltPercentage));
		}
		
		LOG("space", "returning "+ intTotalCost);
		
		return intTotalCost;
	}
	
	
	public static void doStationToShipRepairs(obj_id objPlayer, obj_id objStation, float fltPercentage) throws InterruptedException
	{
		LOG("space", "Doing station to ship repairs");
		float fltDistance = getDistance(objStation, space_transition.getContainingShip(objPlayer));
		
		if (fltDistance > space_transition.STATION_COMM_MAX_DISTANCE)
		{
			string_id strSpam = new string_id("space/space_interaction", "too_far");
			
			sendSystemMessage(objPlayer, strSpam);
			return;
		}
		
		fixAllPlasmaConduits(space_transition.getContainingShip(objPlayer));
		doStationToShipRepairs(objPlayer, objStation, fltPercentage, 0.0f);
	}
	
	
	public static void doStationToShipRepairs(obj_id objPlayer, obj_id objStation, float fltPercentage, float decayRate) throws InterruptedException
	{
		obj_id objShip = space_transition.getContainingShip(objPlayer);
		
		LOG("space", "Doing station to ship repairs2 ");
		
		if (!isIdValid(objShip))
		{
			return;
		}
		
		float fltDamageTotal = getDamageTotal(objPlayer, objShip);
		float fltCostPerPoint = getFloatObjVar(objStation, "fltCostPerDamagePoint");
		
		fltDamageTotal = fltDamageTotal * fltPercentage;
		
		int intTotalCost = (int) (fltCostPerPoint * fltDamageTotal);
		
		if (space_utils.isBasicShip(objShip) || space_utils.isPrototypeShip(objShip))
		{
			
			intTotalCost = (int) (fltCostPerPoint * (100.0f * fltPercentage));
		}
		
		LOG("space", "Doing station to ship repairs3, charginh "+ intTotalCost);
		
		if (money.hasFunds(objPlayer, money.MT_TOTAL, intTotalCost))
		{
			money.requestPayment(objPlayer, "SPACE_STATION_REPAIR", intTotalCost, "repairSuccess", null, false);
			LOG("space", "Taking cash");
			CustomerServiceLog("space_repair", "Taking "+ intTotalCost + " from %TU for repairs", getOwner(objShip));
			repairDamage(objPlayer, objShip, fltPercentage);
		}
		else
		{
			LOG("space", "NOFUNDS");
		}
		
		return;
	}
	
	
	public static float getDamagePercentage(obj_id objShip, int intSlot) throws InterruptedException
	{
		float fltMaximumHitpoints = getShipComponentArmorHitpointsMaximum(objShip, intSlot);
		
		fltMaximumHitpoints = fltMaximumHitpoints + getShipComponentHitpointsMaximum(objShip, intSlot);
		
		float fltCurrentHitpoints = getShipComponentArmorHitpointsCurrent(objShip, intSlot);
		
		fltCurrentHitpoints = fltCurrentHitpoints + getShipComponentHitpointsCurrent(objShip, intSlot);
		
		if (fltCurrentHitpoints == 0)
		{
			return 0;
		}
		
		float fltPercent = fltCurrentHitpoints / fltMaximumHitpoints;
		
		return fltPercent;
	}
	
	
	public static int getExtendedComponentType(String strSlot) throws InterruptedException
	{
		int intSlot = ship_chassis_slot_type.getTypeByName(strSlot);
		
		if (intSlot == ship_chassis_slot_type.SCST_num_types)
		{
			
			if (strSlot.equals("hull"))
			{
				return HULL;
			}
			if (strSlot.equals("life_support"))
			{
				return LIFE_SUPPORT;
			}
			if (strSlot.equals("plasma_conduit"))
			{
				return PLASMA_CONDUIT;
			}
			if (strSlot.equals("ammo_bay"))
			{
				return AMMO_BAY;
				
			}
			if (strSlot.equals("hyperdrive"))
			{
				return HYPERDRIVE;
			}
		}
		
		return intSlot;
	}
	
	
	public static String getExtendedComponentType(int intSlot) throws InterruptedException
	{
		String strType = ship_chassis_slot_type.getNameByType(intSlot);
		
		if (strType.equals("none"))
		{
			
			if (intSlot == HULL)
			{
				return "hull";
				
			}
			if (intSlot == LIFE_SUPPORT)
			{
				return "life_support";
			}
			if (intSlot == PLASMA_CONDUIT)
			{
				return "plasma_conduit";
				
			}
			if (intSlot == AMMO_BAY)
			{
				return "ammo_bay";
			}
			if (intSlot == HYPERDRIVE)
			{
				return "hyperdrive";
				
			}
			return null;
		}
		
		return strType;
	}
	
	
	public static void breakHullPanel(obj_id objPanel, obj_id objShip, location locTest, boolean boolAddToList) throws InterruptedException
	{
		obj_id objCell = locTest.cell;
		int intHitpoints = getHitpoints(objPanel);
		
		if (intHitpoints == 1)
		{
			return;
		}
		
		if (boolAddToList)
		{
			Vector locBrokenComponents = new Vector();
			locBrokenComponents.setSize(0);
			
			if (hasObjVar(objShip, "locBrokenCompents"))
			{
				locBrokenComponents = getResizeableLocationArrayObjVar(objShip, "locBrokenComponents");
			}
			locBrokenComponents = utils.addElement(locBrokenComponents, locTest);
			setObjVar(objShip, "locBrokenComponents", locBrokenComponents);
		}
		
		setInvulnerableHitpoints(objPanel, 1);
		
		if (hasObjVar(objShip, "intHullBreached"))
		{
			float intDamage = getIntObjVar(objShip, "intHullBreached");
			
			intDamage = intDamage + HULL_BREACH_DAMAGE;
			setObjVar(objShip, "intHullBreached", intDamage);
		}
		else
		{
			setObjVar(objShip, "intHullBreached", HULL_BREACH_DAMAGE);
			messageTo(objShip, "doHullBreach", null, HULL_BREACH_DAMAGE_TIMER, false);
			turnOnInteriorAlarms(objShip);
			string_id strSpam = new string_id("space/space_interaction", "hull_breach");
			
			space_utils.sendSystemMessageShip(objShip, strSpam, true, true, true, true);
			
		}
		
		float fltDamage = 0;
		
		if (hasShipInternalDamageOverTime(objShip, ship_chassis_slot_type.SCST_num_types))
		{
			fltDamage = getShipInternalDamageOverTimeDamageRate(objShip, ship_chassis_slot_type.SCST_num_types);
		}
		
		fltDamage = fltDamage + 0.5f;
		setShipInternalDamageOverTime(objShip, ship_chassis_slot_type.SCST_num_types, fltDamage, 0);
		
		return;
	}
	
	
	public static void fixHullPanel(obj_id objPanel, obj_id objShip, location locTest) throws InterruptedException
	{
		obj_id objCell = locTest.cell;
		int intHitpoints = getHitpoints(objPanel);
		
		if (intHitpoints == getMaxHitpoints(objPanel))
		{
			return;
		}
		
		if (!hasObjVar(objShip, "locBrokenComponents"))
		{
			LOG("space", "ship dot system has faled on "+ objShip);
			return;
		}
		
		Vector locBrokenComponents = getResizeableLocationArrayObjVar(objShip, "locBrokenComponents");
		int intIndex = utils.getElementPositionInArray(locBrokenComponents, locTest);
		
		locBrokenComponents = utils.removeElementAt(locBrokenComponents, intIndex);
		
		if (locBrokenComponents.size() > 0)
		{
			setObjVar(objShip, "locBrokenComponents", locBrokenComponents);
		}
		else
		{
			removeObjVar(objShip, "locBrokenComponents");
		}
		
		float intHullBreached = getIntObjVar(objShip, "intHullBreached");
		
		intHullBreached = intHullBreached - HULL_BREACH_DAMAGE;
		
		if (intHullBreached <= 0)
		{
			removeObjVar(objShip, "intHullBreached");
			turnOffInteriorAlarms(objShip);
		}
		else
		{
			setObjVar(objShip, "intHullBreached", intHullBreached);
		}
		
		float fltDamage = 0;
		
		if (hasShipInternalDamageOverTime(objShip, ship_chassis_slot_type.SCST_num_types))
		{
			fltDamage = getShipInternalDamageOverTimeDamageRate(objShip, ship_chassis_slot_type.SCST_num_types);
		}
		
		fltDamage = fltDamage - 0.5f;
		
		if (fltDamage < 0)
		{
			fltDamage = 0;
		}
		
		setShipInternalDamageOverTime(objShip, ship_chassis_slot_type.SCST_num_types, fltDamage, 0);
		setInvulnerableHitpoints(objPanel, getMaxHitpoints(objPanel));
		
		return;
	}
	
	
	public static void breakPlasmaConduit(obj_id objConduit, obj_id objShip, location locTest, boolean boolAddToList) throws InterruptedException
	{
		if (locTest == null)
		{
			return;
		}
		
		locTest.area = "";
		LOG("space", "Breaking "+ objConduit + " boolAddToList is "+ boolAddToList);
		LOG("space", "locTest is "+ locTest);
		
		obj_id objCell = locTest.cell;
		
		if (!isIdValid(objCell))
		{
			LOG("space", "Bad Cell!");
			return;
		}
		
		int intHitpoints = getHitpoints(objConduit);
		
		if (hasCondition(objConduit, CONDITION_ON))
		{
			LOG("space", "Already on!!");
			return;
		}
		
		if (!hasScript(objCell, "space.ship.ship_cell_manager"))
		{
			attachScript(objCell, "space.ship.ship_cell_manager");
		}
		
		turnOnInteriorAlarms(objShip);
		
		if (boolAddToList)
		{
			LOG("space", "adding "+ locTest + " to list");
			Vector locBrokenComponents = new Vector();
			locBrokenComponents.setSize(0);
			
			if (hasObjVar(objShip, "locBrokenComponents"))
			{
				locBrokenComponents = getResizeableLocationArrayObjVar(objShip, "locBrokenComponents");
			}
			locBrokenComponents = utils.addElement(locBrokenComponents, locTest);
			LOG("space", "locBrokenComponetns is a resizeable array of length "+ locBrokenComponents.size());
			setObjVar(objShip, "locBrokenComponents", locBrokenComponents);
		}
		
		if (hasObjVar(objConduit, "intConduitSlot"))
		{
			LOG("space", "has slot and breaking it ");
			int intConduitSlot = getIntObjVar(objConduit, "intConduitSlot");
			
			if (isShipSlotInstalled(objShip, intConduitSlot))
			{
				float fltDamage = 0;
				
				if (hasShipInternalDamageOverTime(objShip, intConduitSlot))
				{
					fltDamage = getShipInternalDamageOverTimeDamageRate(objShip, intConduitSlot);
				}
				
				fltDamage = fltDamage + 0.5f;
				setShipInternalDamageOverTime(objShip, intConduitSlot, fltDamage, 0);
			}
			
			string_id strSpam = new string_id("space/space_interaction", "conduit_burst");
			prose_package pp = prose.getPackage(strSpam, getEncodedName(objConduit));
			
			space_utils.sendSystemMessageShip(objShip, pp, true, true, true, true);
			
		}
		
		dictionary dctParams = new dictionary();
		
		dctParams.put("intSlot", space_crafting.PLASMA_CONDUIT);
		
		LOG("space", "Notifying cell");
		
		space_utils.notifyObject(objCell, "incrementDotDamage", dctParams);
		
		setCondition(objConduit, CONDITION_ON);
		
		return;
	}
	
	
	public static void fixPlasmaConduit(obj_id objConduit, obj_id objShip, location locTest) throws InterruptedException
	{
		locTest.area = "";
		obj_id objCell = getLocation(objConduit).cell;
		
		LOG("space", "fixPlasmaConduit() Cell is "+ objCell);
		
		if (!hasObjVar(objShip, "locBrokenComponents"))
		{
			LOG("space", "fixPlasmaConduit() ship dot system has faled on "+ objShip);
			return;
		}
		
		Vector locBrokenComponents = getResizeableLocationArrayObjVar(objShip, "locBrokenComponents");
		int intIndex = utils.getElementPositionInArray(locBrokenComponents, locTest);
		
		LOG("space", "fixPlasmaConduit() conduit loc: "+ locTest + " intIndex: "+ intIndex);
		
		if (locBrokenComponents == null)
		{
			LOG("space", "fixPlasmaConduit() Broken components is null for some reason");
		}
		
		if (intIndex > -1)
		{
			locBrokenComponents = utils.removeElementAt(locBrokenComponents, intIndex);
			
			LOG("space", "fixPlasmaConduit() Removing "+ intIndex + " locBrokenComponents.length is "+ locBrokenComponents.size());
		}
		
		if (locBrokenComponents.size() > 0)
		{
			setObjVar(objShip, "locBrokenComponents", locBrokenComponents);
		}
		else
		{
			turnOffInteriorAlarms(objShip);
			removeObjVar(objShip, "intAlarmsOn");
			removeObjVar(objShip, "locBrokenComponents");
		}
		
		if (hasObjVar(objConduit, "intConduitSlot"))
		{
			int intConduitSlot = getIntObjVar(objConduit, "intConduitSlot");
			float fltDamage = 0;
			
			if (isShipSlotInstalled(objShip, intConduitSlot))
			{
				if (hasShipInternalDamageOverTime(objShip, intConduitSlot))
				{
					fltDamage = getShipInternalDamageOverTimeDamageRate(objShip, intConduitSlot);
				}
				
				fltDamage = fltDamage - 0.5f;
				
				if (fltDamage < 0)
				{
					fltDamage = 0;
				}
				
				setShipInternalDamageOverTime(objShip, intConduitSlot, fltDamage, 0);
			}
		}
		
		dictionary dctParams = new dictionary();
		
		dctParams.put("intSlot", space_crafting.PLASMA_CONDUIT);
		
		LOG("space", "fixPlasmaConduit() Notifying "+ objCell);
		
		space_utils.notifyObject(objCell, "decrementDotDamage", dctParams);
		clearCondition(objConduit, CONDITION_ON);
		
		return;
	}
	
	
	public static boolean grantAmmoBay(obj_id objShip, int intSlot, transform trTest, obj_id objCell, int intLevel) throws InterruptedException
	{
		
		String strContainer = "object/tangible/container/drum/warren_drum_loot.iff";
		obj_id objContainer = createObject(strContainer, trTest, objCell);
		
		if (!isIdValid(objContainer))
		{
			return false;
		}
		
		setObjVar(objShip, "weapon.objAmmoBay"+ intSlot, objContainer);
		persistObject(objContainer);
		setObjVar(objContainer, "intWeaponSlot", intSlot);
		attachScript(objContainer, "space.crafting.ammo_bay");
		setObjVar(objContainer, "intBayLevel", intLevel);
		
		return true;
	}
	
	
	public static boolean removeAmmoBay(obj_id objShip, int intSlot) throws InterruptedException
	{
		obj_id objBay = getObjIdObjVar(objShip, "weapon.objAmmoBay"+ intSlot);
		
		if (isIdValid(objBay))
		{
			destroyObject(objBay);
			return true;
		}
		return false;
	}
	
	
	public static void reloadWeaponSlot(obj_id objShip, obj_id objItem, int intSlot, obj_id objPlayer, boolean boolVerbose) throws InterruptedException
	{
		if (space_crafting.isMissileSlot(objShip, intSlot))
		{
			space_crafting.applyAmmoToWeapon(objShip, objItem, intSlot, objPlayer, boolVerbose);
		}
		else if (space_crafting.isCounterMeasureSlot(objShip, intSlot))
		{
			space_crafting.applyCountermeasuresToLauncher(objShip, objItem, intSlot, objPlayer, boolVerbose);
		}
		else
		{
			sendSystemMessage(objPlayer, SID_INCORRECTLY_CONFIGURED_ITEM);
			return;
		}
	}
	
	
	public static Vector getAllInstalledComponents(obj_id objShip) throws InterruptedException
	{
		int[] intRawSlots = getShipChassisSlots(objShip);
		Vector intSlots = new Vector();
		intSlots.setSize(0);
		
		for (int intI = 0; intI < intRawSlots.length; intI++)
		{
			testAbortScript();
			if (isShipSlotInstalled(objShip, intRawSlots[intI]))
			{
				intSlots = utils.addElement(intSlots, intRawSlots[intI]);
			}
		}
		
		return intSlots;
	}
	
	
	public static void turnOnInteriorAlarms(obj_id objShip) throws InterruptedException
	{
		if (utils.hasScriptVar(objShip, "objAlarms"))
		{
			setObjVar(objShip, "intAlarmsOn", 1);
			obj_id[] objAlarms = utils.getObjIdArrayScriptVar(objShip, "objAlarms");
			for (int intI = 0; intI < objAlarms.length; intI++)
			{
				testAbortScript();
				
				setCondition(objAlarms[intI], CONDITION_ON);
			}
		}
		
		return;
	}
	
	
	public static void turnOffInteriorAlarms(obj_id objShip) throws InterruptedException
	{
		if (utils.hasScriptVar(objShip, "objAlarms"))
		{
			removeObjVar(objShip, "intAlarmsOn");
			obj_id[] objAlarms = utils.getObjIdArrayScriptVar(objShip, "objAlarms");
			for (int intI = 0; intI < objAlarms.length; intI++)
			{
				testAbortScript();
				
				clearCondition(objAlarms[intI], CONDITION_ON);
			}
		}
		
		return;
	}
	
	
	public static boolean isValidShipComponent(obj_id objShipComponent) throws InterruptedException
	{
		if (isIdValid(objShipComponent))
		{
			String type = getShipComponentStringType(objShipComponent);
			
			if (type == null || type.equals(""))
			{
				return false;
			}
			
			else if ((getShipComponentStringType(objShipComponent) == "armor" || getShipComponentStringType(objShipComponent) == "booster" || getShipComponentStringType(objShipComponent) == "capacitor" || getShipComponentStringType(objShipComponent) == "droid_interface" || getShipComponentStringType(objShipComponent) == "engine" || getShipComponentStringType(objShipComponent) == "reactor" || getShipComponentStringType(objShipComponent) == "shield" || getShipComponentStringType(objShipComponent) == "weapon"))
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
	
	
	public static void setShipEngineRotationRate(obj_id objPlayer, obj_id objShip, float fltRateMax, float fltRateMin, float fltOptimal) throws InterruptedException
	{
		setShipEngineSpeedRotationFactorMaximum(objShip, fltRateMax);
		setShipEngineSpeedRotationFactorMinimum(objShip, fltRateMin);
		setShipEngineSpeedRotationFactorOptimal(objShip, fltOptimal);
		return;
	}
	
	
	public static void setShipChassisPerformanceVariations(obj_id objShip, String strType, obj_id objPlayer) throws InterruptedException
	{
		String strFileName = "datatables/ship/chassis_modifiers.iff";
		dictionary dctShipInfo = dataTableGetRow(strFileName, strType);
		
		if (dctShipInfo == null)
		{
			sendSystemMessageTestingOnly(objPlayer, "No Ship Type of "+ strType + " in "+ strFileName);
			LOG("space", "Bad ship type of "+ strType);
			return;
		}
		
		setShipEngineRotationRate(objPlayer, objShip, dctShipInfo.getFloat("speed_rotation_factor_max"), dctShipInfo.getFloat("speed_rotation_factor_min"), dctShipInfo.getFloat("speed_rotation_factor_optimal"));
		return;
	}
	
	
	public static void purchaseChassisFromBroker(obj_id player, obj_id broker) throws InterruptedException
	{
		
		final String SHIP_TABLE = "datatables/space_crafting/chassis_npc.iff";
		final String BROKER_STF = "chassis_npc";
		
		final String SCRIPTVAR_CHASSIS_SUI = "chassis_npc.sui";
		
		final String BUY_SHIP_TITLE = "@"+ BROKER_STF + ":buy_ship_title";
		final String BUY_SHIP_PROMPT = "@"+ BROKER_STF + ":buy_ship_prompt";
		final String BTN_BUY = "@"+ BROKER_STF + ":btn_buy";
		
		int rows = dataTableGetNumRows(SHIP_TABLE);
		
		String shipTypes[] = new String[rows];
		int shipPrices[] = new int[rows];
		
		int shipCount = 0;
		
		obj_id[] inv = getInventoryAndEquipment(player);
		obj_id[] shipDeeds = new obj_id[inv.length];
		String entries[] = new String[inv.length];
		
		for (int i = 0; i < inv.length; i++)
		{
			testAbortScript();
			if (hasObjVar(inv[i], "shiptype"))
			{
				if (!hasObjVar(getContainedBy(inv[i]), "crafting.tool"))
				{
					if (utils.getContainingPlayer(inv[i]) == player)
					{
						shipDeeds[shipCount] = inv[i];
						String type = getStringObjVar(inv[i], "shiptype");
						int price = dataTableGetInt(SHIP_TABLE, type, 1);
						String name = dataTableGetString(SHIP_TABLE, type, 2);
						
						entries[shipCount] = ("[" + price + "] " + " --- " + name + " --- " + getName(inv[i]));
						shipCount++;
					}
				}
			}
		}
		
		String realEntries[] = new String[shipCount];
		
		for (int i = 0; i < realEntries.length; i++)
		{
			testAbortScript();
			realEntries[i] = entries[i];
		}
		
		if (entries[0] != null && entries.length > 0)
		{
			int pid = sui.listbox(broker, player, BUY_SHIP_PROMPT, sui.OK_CANCEL, BUY_SHIP_TITLE, realEntries, "handleCheckShip", false, false);
			
			if (pid > -1)
			{
				
				setSUIProperty(pid, sui.LISTBOX_BTN_OK, sui.PROP_TEXT, BTN_BUY);
				showSUIPage(pid);
				
				utils.setScriptVar(player, SCRIPTVAR_CHASSIS_SUI, pid);
				utils.setScriptVar(player, "chassis_npc.price", shipPrices);
				utils.setScriptVar(player, "chassis_npc.deed", shipDeeds);
				
			}
		}
		else
		{
			string_id msgNoDeeds = new string_id(BROKER_STF, "no_deeds");
			
			chat.publicChat(broker, player, msgNoDeeds);
			utils.removeScriptVar(player, SCRIPTVAR_CHASSIS_SUI);
			utils.removeScriptVar(player, "chassis_npc.price");
			utils.removeBatchScriptVar(player, "chassis_npc.type");
			utils.removeScriptVar(player, "chassis_npc.deed");
		}
		
		return;
	}
	
	
	public static boolean sellLootItem(obj_id objPlayer, obj_id objBroker, obj_id objItem) throws InterruptedException
	{
		if (utils.hasScriptVar(objItem, "intSold"))
		{
			return false;
		}
		
		obj_id objOwner = utils.getContainingPlayer(objItem);
		
		if (objOwner != objPlayer)
		{
			return false;
		}
		
		float fltDistance = getDistance(objPlayer, objBroker);
		
		if (fltDistance > 24)
		{
			return false;
		}
		
		utils.setScriptVar(objItem, "intSold", 1);
		int intValue = getComponentSellPrice(objItem);
		
		CustomerServiceLog("chassis_broker", "%TU Sold item "+ objItem + " ("+ getTemplateName(objItem) + ") To Dealer "+ objBroker + " FOR "+ intValue, objPlayer);
		
		string_id strSpam = new string_id("space/space_loot", "sold_item");
		prose_package proseTest = new prose_package();
		
		proseTest = prose.setStringId(proseTest, strSpam);
		proseTest = prose.setTO(proseTest, getEncodedName(objItem));
		proseTest = prose.setDI(proseTest, intValue);
		sendSystemMessageProse(objPlayer, proseTest);
		
		destroyObject(objItem);
		money.bankTo("space_component_sale", objPlayer, intValue);
		
		return true;
	}
	
	
	public static boolean hasLootToSell(obj_id objPlayer) throws InterruptedException
	{
		obj_id[] objContents = getInventoryAndEquipment(objPlayer);
		
		for (int intI = 0; intI < objContents.length; intI++)
		{
			testAbortScript();
			if (hasScript(objContents[intI], "space.crafting.component_loot"))
			{
				return true;
			}
		}
		return false;
	}
	
	
	public static int getComponentSellPrice(obj_id objComponent) throws InterruptedException
	{
		final String[] strLootLevels =
		{
			"cert_ordnance_level1",
			"cert_ordnance_level2",
			"cert_ordnance_level3",
			"cert_ordnance_level4",
			"cert_ordnance_level5",
			"cert_ordnance_level6",
			"cert_ordnance_level7",
			"cert_ordnance_level8",
			"cert_ordnance_level9",
			"cert_ordnance_levelten"
		};
		final int[] intValue =
		{
			1000,
			2000,
			3000,
			4000,
			5000,
			6000,
			7000,
			8000,
			9000,
			10000,
		};
		
		String[] strCerts = getRequiredCertifications(objComponent);
		
		if ((strCerts != null) && (strCerts.length > 0))
		{
			int intIndex = utils.getElementPositionInArray(strLootLevels, strCerts[0]);
			
			if (intIndex < 0)
			{
				return 500;
			}
			
			return intValue[intIndex];
		}
		else
		{
			return 500;
		}
	}
	
	
	public static void sellComponentsToBroker(obj_id objPlayer, obj_id objBroker) throws InterruptedException
	{
		String strTitle = "@space/space_loot:sell_loot_title";
		String strPrompt = "@space/space_loot:sell_loot_prompt";
		String strSellButton = "@space/space_loot:sell_loot_button";
		
		obj_id[] objContents = getInventoryAndEquipment(objPlayer);
		
		Vector objLoot = new Vector();
		objLoot.setSize(0);
		Vector ppEntries = new Vector();
		ppEntries.setSize(0);
		string_id strSpam = new string_id("space/space_loot", "item_list");
		
		for (int intI = 0; intI < objContents.length; intI++)
		{
			testAbortScript();
			if (hasScript(objContents[intI], "space.crafting.component_loot"))
			{
				int intValue = getComponentSellPrice(objContents[intI]);
				
				prose_package ppTest = new prose_package();
				
				prose.setTO(ppTest, getEncodedName(objContents[intI]));
				prose.setDI(ppTest, intValue);
				prose.setStringId(ppTest, strSpam);
				
				String strLine = "["+ intValue + "] "+ " ------- "+ getName(objContents[intI]);
				
				objLoot = utils.addElement(objLoot, objContents[intI]);
				ppEntries = utils.addElement(ppEntries, ppTest);
			}
		}
		
		if (ppEntries != null && ppEntries.size() > 0)
		{
			
			prose_package[] ppRawEntries = new prose_package[ppEntries.size()];
			
			for (int intI = 0; intI < ppEntries.size(); intI++)
			{
				testAbortScript();
				prose_package ppTest = ((prose_package)(ppEntries.get(intI)));
				
				ppRawEntries[intI] = ppTest;
			}
			
			int pid = sui.listbox(objBroker, objPlayer, strPrompt, sui.OK_CANCEL, strTitle, ppRawEntries, "sellLootComponents", false, false);
			
			if (pid > -1)
			{
				
				setSUIProperty(pid, sui.LISTBOX_BTN_OK, sui.PROP_TEXT, strSellButton);
				showSUIPage(pid);
				utils.setScriptVar(objPlayer, "objLootToSell", objLoot);
			}
		}
		else
		{
			strSpam = new string_id("space/space_loot", "nothing_to_sell");
			sendSystemMessage(objPlayer, strSpam);
			
		}
		return;
	}
	
	
	public static obj_id createDeedFromBlueprints(obj_id player, String type, obj_id inventory, float mass, float hp) throws InterruptedException
	{
		if (isIdValid(player))
		{
			obj_id newDeed = createObject("object/tangible/ship/crafted/chassis/"+ type + "_deed.iff", inventory, "");
			
			setObjVar(newDeed, "ship_chassis.mass", mass);
			setObjVar(newDeed, "ship_chassis.hp", hp);
			setObjVar(newDeed, "ship_chassis.type", type);
			return newDeed;
		}
		else
		{
			return null;
		}
	}
	
	
	public static boolean setupChassisDifferentiation(obj_id objShip) throws InterruptedException
	{
		String strChassis = getShipChassisType(objShip);
		
		return setupChassisDifferentiation(objShip, strChassis);
	}
	
	
	public static boolean setupChassisDifferentiation(obj_id objShip, String strChassis) throws InterruptedException
	{
		if (space_utils.isPlayerControlledShip(objShip))
		{
			LOG("space", "setting modifier to "+ strChassis);
			String strFileName = "datatables/ship/chassis_modifiers.iff";
			dictionary dctShipInfo = dataTableGetRow(strFileName, strChassis);
			
			if (dctShipInfo == null)
			{
				LOG("space", "No entry for "+ strChassis);
				return false;
			}
			LOG("space", "setting modifier to "+ dctShipInfo);
			setShipEngineAccelerationRate(objShip, dctShipInfo.getFloat("engine_accel"));
			setShipEngineDecelerationRate(objShip, dctShipInfo.getFloat("engine_decel"));
			setShipEnginePitchAccelerationRateDegrees(objShip, dctShipInfo.getFloat("engine_pitch_accel"));
			setShipEngineYawAccelerationRateDegrees(objShip, dctShipInfo.getFloat("engine_yaw_accel"));
			setShipEngineRollAccelerationRateDegrees(objShip, dctShipInfo.getFloat("engine_roll_accel"));
			setShipEngineSpeedRotationFactorMaximum(objShip, dctShipInfo.getFloat("speed_rotation_factor_max"));
			setShipEngineSpeedRotationFactorMinimum(objShip, dctShipInfo.getFloat("speed_rotation_factor_min"));
			setShipEngineSpeedRotationFactorOptimal(objShip, dctShipInfo.getFloat("speed_rotation_factor_optimal"));
			setShipSlideDampener(objShip, dctShipInfo.getFloat("slideDamp"));
			setShipChassisSpeedMaximumModifier(objShip, dctShipInfo.getFloat("fltMaxSpeedModifier"));
		}
		else
		{
			LOG("space", "NO ROTATIONAL RESET");
			return false;
		}
		return true;
	}
	
	
	public static void resetChassisRotationalVelocity(obj_id objShip) throws InterruptedException
	{
		if (space_utils.isPlayerControlledShip(objShip))
		{
			String strChassis = getShipChassisType(objShip);
			String strFileName = "datatables/ship/chassis_modifiers.iff";
			
			float fltRotationSpeedFactorMaximum = dataTableGetFloat(strFileName, strChassis, "speed_rotation_factor");
			
			if (fltRotationSpeedFactorMaximum <= 0)
			{
				fltRotationSpeedFactorMaximum = 1.0f;
			}
			
			setShipEngineSpeedRotationFactorMaximum(objShip, fltRotationSpeedFactorMaximum);
		}
		
		return;
	}
	
	
	public static void clearComponentDisabledFlag(obj_id objComponent) throws InterruptedException
	{
		int flags = getIntObjVar(objComponent, "ship_comp.flags");
		
		flags &= ~(base_class.ship_component_flags.SCF_disabled);
		flags &= ~(base_class.ship_component_flags.SCF_demolished);
		setObjVar(objComponent, "ship_comp.flags", flags);
		return;
	}
	
	
	public static obj_id createChassisFromDeed(obj_id player, obj_id deed, float hp, float currentHp, float mass, String type) throws InterruptedException
	{
		final String STF = "chassis_npc";
		
		if (isIdValid(player))
		{
			obj_id datapad = utils.getDatapad(player);
			obj_id pcd = createObject("object/intangible/ship/"+ type + "_pcd.iff", datapad, "");
			
			if (!isIdValid(pcd))
			{
				return null;
			}
			else
			{
				obj_id ship = createObject("object/ship/player/player_"+ type + ".iff", pcd, "");
				
				if (!hasCertificationsForItem(player, ship))
				{
					destroyObject(ship);
					destroyObject(pcd);
					sendSystemMessage(player, new string_id(STF, "not_certified"));
					return null;
				}
				
				if (hasObjVar(deed, "ship_chassis.badgeRestricted") && !isGod(player))
				{
					int badgeReq = getIntObjVar(deed, "ship_chassis.badgeRestricted");
					String badgeName = getCollectionSlotName(badgeReq);
					
					if ((badgeName == null) || (badgeName.length() == 0) || (!badge.hasBadge(player, badgeName)))
					{
						destroyObject(ship);
						destroyObject(pcd);
						sendSystemMessage(player, new string_id(STF, "no_badge"));
						return null;
					}
				}
				
				if (hasObjVar(deed, "ship_chassis.groundQuestRestricted") && !isGod(player))
				{
					String reqGroundQuest = getStringObjVar(deed, "ship_chassis.groundQuestRestricted");
					
					if (!groundquests.hasCompletedQuest(player, reqGroundQuest))
					{
						destroyObject(ship);
						destroyObject(pcd);
						sendSystemMessage(player, new string_id(STF, "no_quest"));
						return null;
					}
					
				}
				
				if (hasObjVar(deed, "ship_chassis.spaceQuestRestricted") && !isGod(player))
				{
					String reqSpaceQuest = getStringObjVar(deed, "ship_chassis.spaceQuestRestricted");
					String reqSpaceQuestType = getStringObjVar(deed, "ship_chassis.spaceQuestRestrictedType");
					
					if (!space_quest.hasReceivedReward(player, reqSpaceQuestType, reqSpaceQuest) && !space_quest.hasWonQuest(player, reqSpaceQuestType, reqSpaceQuest))
					{
						destroyObject(ship);
						destroyObject(pcd);
						sendSystemMessage(player, new string_id(STF, "no_quest"));
						return null;
					}
					
				}
				
				if (isIdValid(ship))
				{
					CustomerServiceLog("ship_deed", "ship deed used: deed="+ deed + " scd="+ pcd + " ship="+ ship + " player="+ player + "("+ getPlayerName(player) + ")");
					
					if (space_utils.isPobType(type))
					{
						if (!hasObjVar(pcd, "lotReqRemoved"))
						{
							setObjVar(pcd, "lotReqRemoved", true);
						}
						
					}
					
					setOwner(ship, player);
					
					if (!type.equals("sorosuub_space_yacht"))
					{
						space_crafting.uninstallAll(ship);
					}
					
					setShipMaximumChassisHitPoints(ship, hp);
					
					if (currentHp > hp)
					{
						currentHp = hp;
					}
					
					setShipCurrentChassisHitPoints(ship, currentHp);
					setChassisComponentMassMaximum(ship, mass);
					String strParkingLocation = getStringObjVar(deed, "strParkingLocation");
					
					if (strParkingLocation != null)
					{
						setObjVar(pcd, "strParkingLocation", strParkingLocation);
					}
					
					if (hasObjVar(deed, "noTrade"))
					{
						setObjVar(pcd, "noTrade", true);
					}
					
					if (!hasScript(deed, "item.special.nodestroy"))
					{
						destroyObject(deed);
					}
					
					string_id successMessage = new string_id(STF, "succeed");
					
					sendSystemMessage(player, successMessage);
					
				}
				else
				{
					string_id message3 = new string_id(STF, "failed");
					
					sendSystemMessage(player, message3);
				}
			}
		}
		
		return null;
	}
	
	
	public static void fixAllPlasmaConduits(obj_id objShip) throws InterruptedException
	{
		LOG("space", "FIXING ALL PLASMA CONDUITS");
		
		if (utils.hasScriptVar(objShip, "objPlasmaConduits"))
		{
			Vector objPlasmaConduits = utils.getResizeableObjIdArrayScriptVar(objShip, "objPlasmaConduits");
			
			for (int intI = 0; intI < objPlasmaConduits.size(); intI++)
			{
				testAbortScript();
				LOG("space", "Plasma conduit ["+ intI + "]");
				
				if (hasCondition(((obj_id)(objPlasmaConduits.get(intI))), CONDITION_ON))
				{
					LOG("space", "Conduit is on");
					fixPlasmaConduit(((obj_id)(objPlasmaConduits.get(intI))), objShip, getLocation(((obj_id)(objPlasmaConduits.get(intI)))));
				}
			}
		}
		
		return;
	}
	
	
	public static void sellResourcesToSpaceStation(obj_id player, obj_id npc) throws InterruptedException
	{
		String priceList = getStringObjVar(npc, "space_mining.priceList");
		
		if (isIdValid(player) && isIdValid(npc))
		{
			openSpaceMiningUi(player, npc, priceList);
		}
		
		return;
	}
}
