package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.callable;
import script.library.utils;
import script.library.space_utils;
import script.library.factions;
import script.library.space_crafting;
import script.library.ship_ai;
import script.library.xp;
import script.library.badge;
import script.library.buff;
import script.library.space_flags;
import script.library.groundquests;
import script.library.smuggler;


public class space_combat extends script.base_script
{
	public space_combat()
	{
	}
	public static final float SPACE_DEATH_DELAY = 10.0f;
	
	public static final int CHASSIS = 1;
	public static final int COMPONENT = 2;
	public static final int SHIELD = 3;
	public static final int ARMOR = 4;
	
	public static final int LIGHT = 1;
	public static final int HEAVY = 2;
	
	public static final int ALL_WEAPONS = -1;
	
	public static final float GROUND_XP_PERCENT = 0.01f;
	
	public static final int[] MANDATORY_COMPONENTS =
	{
		ship_chassis_slot_type.SCST_reactor,
		ship_chassis_slot_type.SCST_engine
	};
	
	public static final int[] WEAPON_SLOTS =
	{
		ship_chassis_slot_type.SCST_weapon_0,
		ship_chassis_slot_type.SCST_weapon_1,
		ship_chassis_slot_type.SCST_weapon_2,
		ship_chassis_slot_type.SCST_weapon_3,
		ship_chassis_slot_type.SCST_weapon_4,
		ship_chassis_slot_type.SCST_weapon_5,
		ship_chassis_slot_type.SCST_weapon_6,
		ship_chassis_slot_type.SCST_weapon_7
	};
	
	public static final float MINIMUM_DAMAGE_THRESHOLD = .05f;
	public static final float MINIMUM_EFFICIENCY = .10f;
	public static final float NO_CHANGE = -1f;
	
	public static final int SHIP = ship_chassis_slot_type.SCST_num_types;
	public static final int FRONT = 0;
	public static final int BACK = 1;
	public static final int NONE = -2;
	
	public static final int NOVA_ORION_RESOURCE_MIN = 1;
	public static final int NOVA_ORION_RESOURCE_MAX = 15;
	public static final int NOVA_ORION_RESOURCE_CHANCE = 25;
	public static final int NOVA_ORION_BUFF_MIDLITHE_BONUS = 5;
	
	public static final int NOVA_ORION_BUFF_COLLECTIBLES_BONUS = 2;
	
	public static final String NOVA_ORION_RESOURCE = "item_nova_orion_space_resource_01_01";
	
	public static final String SHIP_DATATABLE = "datatables/space_mobile/space_mobile.iff";
	
	public static final float DROID_VOCALIZE_DELAY_CONST = 1.0f;
	public static final String DROID_VOCALIZE_DATATABLE = "datatables/space_combat/droid_vocalizations.iff";
	
	public static final int AVERAGE_COMMAND_SUCCESS_RATE = 65;
	
	public static final int FIRST_WEAPON = ship_chassis_slot_type.SCST_weapon_first;
	public static final int LAST_WEAPON = ship_chassis_slot_type.SCST_weapon_last;
	
	public static final String PIRATE_EQUIPMENT_DATATABLE = "datatables/space_command/pirate_ship_table.iff";
	public static final String PIRATE_EQUIPMENT_AA = "PIRATE_EQUIPMENT_AA";
	public static final String PIRATE_EQUIPMENT_A = "PIRATE_EQUIPMENT_A";
	public static final String PIRATE_EQUIPMENT_B = "PIRATE_EQUIPMENT_B";
	public static final String PIRATE_EQUIPMENT_C = "PIRATE_EQUIPMENT_C";
	public static final String PIRATE_EQUIPMENT_D = "PIRATE_EQUIPMENT_D";
	
	public static final int NPC_DAMAGE_MULTIPLIER = 4;
	
	public static final string_id SID_DISABLED = new string_id("space/space_interaction", "disabled");
	
	public static final String SPACE_COLLECTIONS_LOOT_TABLE = "datatables/space_loot/space_collection_loot.iff";
	
	
	public static void addToCombatDamage(obj_id objAttacker, obj_id objDefender, int intDamage) throws InterruptedException
	{
		Vector objAttackers = utils.getResizeableObjIdArrayLocalVar(objDefender, "damage.objAttackers");
		Vector intDamageDone = utils.getResizeableIntArrayLocalVar(objDefender, "damage.intDamageDone");
		int intIndex = utils.getElementPositionInArray(objAttackers, objAttacker);
		
		if (intIndex < 0)
		{
			objAttackers = utils.addElement(objAttackers, objAttacker);
			intDamageDone = utils.addElement(intDamageDone, intDamage);
			utils.setLocalVar(objDefender, "damage.objAttackers", objAttackers);
			utils.setLocalVar(objDefender, "damage.intDamageDone", intDamageDone);
			
			return;
		}
		else
		{
			intDamageDone.set(intIndex, new Integer(((Integer)(intDamageDone.get(intIndex))).intValue() + intDamage));
			utils.setLocalVar(objDefender, "damage.intDamageDone", intDamageDone);
		}
		
		return;
	}
	
	
	public static void targetDestroyed(obj_id objTarget) throws InterruptedException
	{
		
		dictionary dctParams = new dictionary();
		
		space_utils.notifyObject(objTarget, "objectDestroyed", dctParams);
		
		return;
	}
	
	
	public static void normalizeAllComponents(obj_id objShip) throws InterruptedException
	{
		String[] normalizeList =
		{
			"weapons_normalize", "engine_normalize", "shields_normalize", "reactor_normalize", "weapcap_equalize"
		};
		
		for (int i = 0; i < normalizeList.length; i++)
		{
			testAbortScript();
			space_combat.unrestrictedDroidCommand(objShip, normalizeList[i]);
		}
	}
	
	
	public static void unrestrictedDroidCommand(obj_id objShip, String strCommand) throws InterruptedException
	{
		if (!isIdValid(objShip) || !exists(objShip))
		{
			return;
		}
		
		dictionary dctCommandInfo = dataTableGetRow("datatables/space_combat/droid_commands.iff", strCommand);
		
		if (dctCommandInfo == null)
		{
			debugServerConsoleMsg(null, "SPACE_COMBAT.performDroidCommands - FAILED TO GET COMMAND INFO DICTIONARY. STRING NOT FOUND WAS:"+ strCommand);
			return;
		}
		
		String strClientEffect = dctCommandInfo.getString("strClientEffect");
		String strStringId = dctCommandInfo.getString("strStringId");
		int intComponentToModify = dctCommandInfo.getInt("intComponentToModify");
		float fltEnergyEfficiency = dctCommandInfo.getFloat("fltEnergyEfficiency");
		float fltGeneralEfficiency = dctCommandInfo.getFloat("fltGeneralEfficiency");
		float fltRepairMin = dctCommandInfo.getFloat("fltRepairMin");
		float fltRepairMax = dctCommandInfo.getFloat("fltRepairMax");
		float fltBaseDelay = dctCommandInfo.getFloat("fltBaseDelay");
		float fltRepairKitConsumed = dctCommandInfo.getFloat("fltRepairKitConsumed");
		float fltCountermeasureCapacitorCost = dctCommandInfo.getFloat("fltCountermeasureCapacitorCost");
		float fltCountermeasureShieldCost = dctCommandInfo.getFloat("fltCountermeasureShieldCost");
		float fltFrontToBackShieldReinforcePercentage = dctCommandInfo.getFloat("fltFrontToBackShieldReinforcePercentage");
		float fltBackToFrontShieldReinforcePercentage = dctCommandInfo.getFloat("fltBackToFrontShieldReinforcePercentage");
		float fltCapacitorReinforcePercentage = dctCommandInfo.getFloat("fltCapacitorReinforcePercentage");
		float fltMissileSavingThrow = dctCommandInfo.getFloat("fltMissileSavingThrow");
		String strMessageHandlerOnPlayer = dctCommandInfo.getString("strMessageHandlerOnPlayer");
		float fltDamageToComponent = dctCommandInfo.getFloat("fltDamageToComponent");
		float fltFrontShieldRatio = dctCommandInfo.getFloat("fltFrontShieldRatio");
		
		if ((intComponentToModify != NONE) && (intComponentToModify != ALL_WEAPONS))
		{
			if (!isShipSlotInstalled(objShip, intComponentToModify))
			{
				return;
			}
			
			if (fltDamageToComponent > 0)
			{
				float fltCurrentHitpoints = getShipComponentHitpointsCurrent(objShip, intComponentToModify);
				float fltCurrentArmorHitpoints = getShipComponentArmorHitpointsCurrent(objShip, intComponentToModify);
				
				fltCurrentArmorHitpoints = fltCurrentArmorHitpoints - fltDamageToComponent;
				
				if (fltCurrentArmorHitpoints < 0)
				{
					float fltRemainingDamage = Math.abs(fltCurrentArmorHitpoints);
					
					fltCurrentHitpoints = fltCurrentHitpoints - fltRemainingDamage;
					
					if (fltCurrentHitpoints < 0)
					{
						return;
					}
					else
					{
						setShipComponentArmorHitpointsCurrent(objShip, intComponentToModify, 0);
						setShipComponentHitpointsCurrent(objShip, intComponentToModify, fltCurrentHitpoints);
						recalculateEfficiencyGeneral(intComponentToModify, objShip);
					}
				}
				else
				{
					setShipComponentArmorHitpointsCurrent(objShip, intComponentToModify, fltCurrentArmorHitpoints);
					
				}
			}
			
			if ((fltGeneralEfficiency != 0) && (fltEnergyEfficiency != 0))
			{
				if (isEfficiencyModified(intComponentToModify, objShip))
				{
					setEfficiencyModifier(intComponentToModify, objShip, 1.0f, 1.0f);
				}
				
				setEfficiencyModifier(intComponentToModify, objShip, fltGeneralEfficiency, fltEnergyEfficiency);
			}
			
			if (fltFrontShieldRatio > 0)
			{
				
				float fltShieldTotal = getShipShieldHitpointsFrontMaximum(objShip) + getShipShieldHitpointsBackMaximum(objShip);
				float fltFrontMax = (fltShieldTotal / 2) * fltFrontShieldRatio;
				float fltBackMax = fltShieldTotal - fltFrontMax;
				
				setShipShieldHitpointsFrontMaximum(objShip, fltFrontMax);
				setShipShieldHitpointsBackMaximum(objShip, fltBackMax);
			}
		}
		else if (intComponentToModify == ALL_WEAPONS)
		{
			
			for (int intI = 0; intI < WEAPON_SLOTS.length; intI++)
			{
				testAbortScript();
				if (isShipSlotInstalled(objShip, WEAPON_SLOTS[intI]))
				{
					if (fltDamageToComponent > 0)
					{
						float fltCurrentHitpoints = getShipComponentHitpointsCurrent(objShip, WEAPON_SLOTS[intI]);
						float fltCurrentArmorHitpoints = getShipComponentArmorHitpointsCurrent(objShip, WEAPON_SLOTS[intI]);
						
						fltCurrentArmorHitpoints = fltCurrentArmorHitpoints - fltDamageToComponent;
						
						if (fltCurrentArmorHitpoints < 0)
						{
							float fltRemainingDamage = Math.abs(fltCurrentArmorHitpoints);
							
							fltCurrentHitpoints = fltCurrentHitpoints - fltRemainingDamage;
							
							if (fltCurrentHitpoints < 1)
							{
								return;
							}
							else
							{
								setShipComponentArmorHitpointsCurrent(objShip, WEAPON_SLOTS[intI], 0);
								setShipComponentHitpointsCurrent(objShip, WEAPON_SLOTS[intI], fltCurrentHitpoints);
								recalculateEfficiencyGeneral(WEAPON_SLOTS[intI], objShip);
							}
						}
						else
						{
							setShipComponentArmorHitpointsCurrent(objShip, WEAPON_SLOTS[intI], fltCurrentArmorHitpoints);
							
						}
						
					}
					
					if ((fltGeneralEfficiency != 0) && (fltEnergyEfficiency != 0))
					{
						if (isEfficiencyModified(WEAPON_SLOTS[intI], objShip))
						{
							setEfficiencyModifier(WEAPON_SLOTS[intI], objShip, 1.0f, 1.0f);
						}
						
						setEfficiencyModifier(WEAPON_SLOTS[intI], objShip, fltGeneralEfficiency, fltEnergyEfficiency);
					}
				}
			}
		}
		
		if (fltCapacitorReinforcePercentage != 0)
		{
			if (!reinforceShieldsFromCapacitor(objShip, fltCapacitorReinforcePercentage))
			{
				return;
			}
		}
		
		if (fltFrontToBackShieldReinforcePercentage != 0)
		{
			if (!transferFrontShieldToBack(objShip, fltFrontToBackShieldReinforcePercentage))
			{
				return;
			}
		}
		
		if (fltBackToFrontShieldReinforcePercentage != 0)
		{
			if (!transferBackShieldToFront(objShip, fltBackToFrontShieldReinforcePercentage))
			{
				return;
			}
		}
	}
	
	
	public static void performDroidCommands(obj_id objPlayer, String strCommand) throws InterruptedException
	{
		
		obj_id objShip = space_transition.getContainingShip(objPlayer);
		
		if (!isIdValid(objShip))
		{
			
			debugServerConsoleMsg(null, "SPACE_COMBAT.performDroidCommands - FAILED TO GET SHIP OBJID through getContainingShip(objPlayer) method");
			return;
		}
		
		if (doDroidPreCheck(objPlayer, objShip, strCommand))
		{
			dictionary dctCommandInfo = dataTableGetRow("datatables/space_combat/droid_commands.iff", strCommand);
			
			if (dctCommandInfo == null)
			{
				debugServerConsoleMsg(null, "SPACE_COMBAT.performDroidCommands - FAILED TO GET COMMAND INFO DICTIONARY. STRING NOT FOUND WAS:"+ strCommand);
				return;
			}
			
			String strClientEffect = dctCommandInfo.getString("strClientEffect");
			
			String strStringId = dctCommandInfo.getString("strStringId");
			
			int intComponentToModify = dctCommandInfo.getInt("intComponentToModify");
			
			float fltEnergyEfficiency = dctCommandInfo.getFloat("fltEnergyEfficiency");
			
			float fltGeneralEfficiency = dctCommandInfo.getFloat("fltGeneralEfficiency");
			
			float fltRepairMin = dctCommandInfo.getFloat("fltRepairMin");
			
			float fltRepairMax = dctCommandInfo.getFloat("fltRepairMax");
			
			float fltBaseDelay = dctCommandInfo.getFloat("fltBaseDelay");
			
			float fltRepairKitConsumed = dctCommandInfo.getFloat("fltRepairKitConsumed");
			
			float fltCountermeasureCapacitorCost = dctCommandInfo.getFloat("fltCountermeasureCapacitorCost");
			
			float fltCountermeasureShieldCost = dctCommandInfo.getFloat("fltCountermeasureShieldCost");
			
			float fltFrontToBackShieldReinforcePercentage = dctCommandInfo.getFloat("fltFrontToBackShieldReinforcePercentage");
			
			float fltBackToFrontShieldReinforcePercentage = dctCommandInfo.getFloat("fltBackToFrontShieldReinforcePercentage");
			
			float fltCapacitorReinforcePercentage = dctCommandInfo.getFloat("fltCapacitorReinforcePercentage");
			
			float fltMissileSavingThrow = dctCommandInfo.getFloat("fltMissileSavingThrow");
			
			String strMessageHandlerOnPlayer = dctCommandInfo.getString("strMessageHandlerOnPlayer");
			
			float fltDamageToComponent = dctCommandInfo.getFloat("fltDamageToComponent");
			
			float fltFrontShieldRatio = dctCommandInfo.getFloat("fltFrontShieldRatio");
			
			float fltComponentDelay = getShipDroidInterfaceCommandSpeed(objShip);
			
			if (fltComponentDelay < 1)
			{
				fltComponentDelay = 1.0f;
			}
			
			if ((intComponentToModify != NONE) && (intComponentToModify != ALL_WEAPONS))
			{
				if (!isShipSlotInstalled(objShip, intComponentToModify))
				{
					string_id strSpam = new string_id("space/space_interaction", "droid_component_not_installed");
					
					sendSystemMessage(objPlayer, strSpam);
					return;
				}
				
				if (fltDamageToComponent > 0)
				{
					
					float fltCurrentHitpoints = getShipComponentHitpointsCurrent(objShip, intComponentToModify);
					float fltCurrentArmorHitpoints = getShipComponentArmorHitpointsCurrent(objShip, intComponentToModify);
					
					fltCurrentArmorHitpoints = fltCurrentArmorHitpoints - fltDamageToComponent;
					
					if (fltCurrentArmorHitpoints < 0)
					{
						float fltRemainingDamage = Math.abs(fltCurrentArmorHitpoints);
						
						fltCurrentHitpoints = fltCurrentHitpoints - fltRemainingDamage;
						
						if (fltCurrentHitpoints < 0)
						{
							string_id strSpam = new string_id("space/space_interaction", "droid_component_too_damaged");
							string_id strComponent = new string_id("space/space_interaction", space_crafting.getShipComponentStringType(intComponentToModify));
							prose_package proseTest = prose.getPackage(strSpam, strComponent);
							
							return;
						}
						else
						{
							setShipComponentArmorHitpointsCurrent(objShip, intComponentToModify, 0);
							setShipComponentHitpointsCurrent(objShip, intComponentToModify, fltCurrentHitpoints);
							recalculateEfficiencyGeneral(intComponentToModify, objShip);
						}
					}
					else
					{
						setShipComponentArmorHitpointsCurrent(objShip, intComponentToModify, fltCurrentArmorHitpoints);
						
					}
					
				}
				
				if ((fltGeneralEfficiency != 0) && (fltEnergyEfficiency != 0))
				{
					if (isEfficiencyModified(intComponentToModify, objShip))
					{
						setEfficiencyModifier(intComponentToModify, objShip, 1.0f, 1.0f);
						
					}
					
					setEfficiencyModifier(intComponentToModify, objShip, fltGeneralEfficiency, fltEnergyEfficiency);
				}
				
				if (fltFrontShieldRatio > 0)
				{
					
					float fltShieldTotal = getShipShieldHitpointsFrontMaximum(objShip) + getShipShieldHitpointsBackMaximum(objShip);
					float fltFrontMax = (fltShieldTotal / 2) * fltFrontShieldRatio;
					float fltBackMax = fltShieldTotal - fltFrontMax;
					
					setShipShieldHitpointsFrontMaximum(objShip, fltFrontMax);
					setShipShieldHitpointsBackMaximum(objShip, fltBackMax);
				}
			}
			else if (intComponentToModify == ALL_WEAPONS)
			{
				
				for (int intI = 0; intI < WEAPON_SLOTS.length; intI++)
				{
					testAbortScript();
					if (isShipSlotInstalled(objShip, WEAPON_SLOTS[intI]))
					{
						if (fltDamageToComponent > 0)
						{
							float fltCurrentHitpoints = getShipComponentHitpointsCurrent(objShip, WEAPON_SLOTS[intI]);
							float fltCurrentArmorHitpoints = getShipComponentArmorHitpointsCurrent(objShip, WEAPON_SLOTS[intI]);
							
							fltCurrentArmorHitpoints = fltCurrentArmorHitpoints - fltDamageToComponent;
							
							if (fltCurrentArmorHitpoints < 0)
							{
								float fltRemainingDamage = Math.abs(fltCurrentArmorHitpoints);
								
								fltCurrentHitpoints = fltCurrentHitpoints - fltRemainingDamage;
								
								if (fltCurrentHitpoints < 1)
								{
									string_id strSpam = new string_id("space/space_interaction", "droid_command_weapons_too_damaged");
									
									space_utils.sendSystemMessageShip(objShip, strSpam, true, true, true, true);
									return;
								}
								else
								{
									setShipComponentArmorHitpointsCurrent(objShip, WEAPON_SLOTS[intI], 0);
									setShipComponentHitpointsCurrent(objShip, WEAPON_SLOTS[intI], fltCurrentHitpoints);
									recalculateEfficiencyGeneral(WEAPON_SLOTS[intI], objShip);
								}
							}
							else
							{
								setShipComponentArmorHitpointsCurrent(objShip, WEAPON_SLOTS[intI], fltCurrentArmorHitpoints);
								
							}
							
						}
						
						if ((fltGeneralEfficiency != 0) && (fltEnergyEfficiency != 0))
						{
							if (isEfficiencyModified(WEAPON_SLOTS[intI], objShip))
							{
								setEfficiencyModifier(WEAPON_SLOTS[intI], objShip, 1.0f, 1.0f);
							}
							
							setEfficiencyModifier(WEAPON_SLOTS[intI], objShip, fltGeneralEfficiency, fltEnergyEfficiency);
						}
					}
					
				}
				
			}
			
			if (fltCapacitorReinforcePercentage != 0)
			{
				
				if (!reinforceShieldsFromCapacitor(objShip, fltCapacitorReinforcePercentage))
				{
					string_id strSpam = new string_id("space/space_interaction", "no_capacitor_energy");
					
					sendSystemMessage(objPlayer, strSpam);
					return;
				}
			}
			
			if (fltFrontToBackShieldReinforcePercentage != 0)
			{
				
				if (!transferFrontShieldToBack(objShip, fltFrontToBackShieldReinforcePercentage))
				{
					
					string_id strSpam = new string_id("space/space_interaction", "no_front_shield_energy");
					
					sendSystemMessage(objPlayer, strSpam);
					return;
					
				}
			}
			
			if (fltBackToFrontShieldReinforcePercentage != 0)
			{
				if (!transferBackShieldToFront(objShip, fltBackToFrontShieldReinforcePercentage))
				{
					
					string_id strSpam = new string_id("space/space_interaction", "no_back_shield_energy");
					
					sendSystemMessage(objPlayer, strSpam);
					return;
				}
			}
			
			if (!strMessageHandlerOnPlayer.equals(""))
			{
				space_utils.notifyObject(objPlayer, strMessageHandlerOnPlayer, dctCommandInfo);
			}
			
			if (fltBaseDelay > 0)
			{
				int intTime = getGameTime();
				float fltFullDelay = fltComponentDelay * fltBaseDelay;
				
				intTime = intTime + (int) fltFullDelay;
				utils.setLocalVar(objShip, "intDroidTimestamp", intTime);
				string_id strSpam = new string_id("space/space_interaction", "droid_delay_ready");
				
				space_utils.sendDelayedSystemMessage(objPlayer, strSpam, fltFullDelay);
			}
			
			if (!strStringId.equals(""))
			{
				string_id strSpam = new string_id("space/droid_commands", strStringId);
				prose_package pp = prose.getPackage(strSpam);
				
				flightDroidVocalize(objShip, 2, pp);
			}
			else
			{
				flightDroidVocalize(objShip, 2);
			}
		}
	}
	
	
	public static boolean doDroidPreCheck(obj_id objPlayer, obj_id objShip, String strCommand) throws InterruptedException
	{
		
		int intCurrentTime = getGameTime();
		int intLastTime = utils.getIntLocalVar(objShip, "intDroidTimestamp");
		
		if (intCurrentTime < intLastTime)
		{
			string_id strSpam = new string_id("space/space_interaction", "droid_command_delay");
			prose_package pp = prose.getPackage(strSpam, intLastTime - intCurrentTime);
			
			sendSystemMessageProse(objPlayer, pp);
			
			return false;
		}
		
		if (!isShipSlotInstalled(objShip, ship_chassis_slot_type.SCST_droid_interface))
		{
			
			string_id strSpam = new string_id("space/space_interaction", "droid_command_fail_no_interface");
			
			sendSystemMessage(objPlayer, strSpam);
			
			return false;
		}
		
		if (isShipComponentDisabled(objShip, ship_chassis_slot_type.SCST_droid_interface))
		{
			
			sendSystemMessage(objPlayer, SID_DISABLED);
			string_id strSpam = new string_id("space/space_interaction", "droid_command_fail_interface_disabled");
			
			sendSystemMessage(objPlayer, strSpam);
			
			return false;
		}
		
		obj_id objControlDevice = getDroidControlDeviceForShip(objShip);
		
		if (!isIdValid(objControlDevice))
		{
			string_id strSpam = new string_id("space/space_interaction", "droid_command_fail_no_droid");
			
			sendSystemMessage(objPlayer, strSpam);
			
			return false;
		}
		if (!droidHasAbility(objControlDevice, strCommand))
		{
			string_id strSpam = new string_id("space/space_interaction", "droid_command_fail_no_command");
			
			sendSystemMessage(objPlayer, strSpam);
			
			return false;
		}
		
		return true;
	}
	
	
	public static boolean droidHasAbility(obj_id objControlDevice, String strCommand) throws InterruptedException
	{
		
		obj_id objDataPad = utils.getDatapad(objControlDevice);
		
		if (!isIdValid(objDataPad))
		{
			
			return false;
		}
		
		obj_id[] objContents = utils.getContents(objDataPad);
		boolean retval = false;
		
		for (int intI = 0; intI < objContents.length; intI++)
		{
			testAbortScript();
			
			if (hasObjVar(objContents[intI], "strDroidCommand"))
			{
				
				String strTest = getStringObjVar(objContents[intI], "strDroidCommand");
				
				if (strTest.equals(strCommand))
				{
					retval = true;
				}
			}
		}
		
		return retval;
	}
	
	
	public static float getCommandEfficacy(obj_id objDroid, String strCategory, float fltEfficacy) throws InterruptedException
	{
		
		return 1.0f;
	}
	
	
	public static void registerDamageDoneToShip(obj_id objAttacker, obj_id objDefender, float fltDamage) throws InterruptedException
	{
		
		if (fltDamage == 0)
		{
			return;
			
		}
		
		if (!space_utils.isPlayerControlledShip(objAttacker))
		{
			
			if (hasObjVar(objAttacker, "commanderPlayer"))
			{
				obj_id objCommander = getObjIdObjVar(objAttacker, "commanderPlayer");
				
				if (isIdValid(objCommander) && (objCommander.isLoaded()))
				{
					obj_id objShip = space_transition.getContainingShip(objCommander);
					
					if (isIdValid(objShip))
					{
						objAttacker = objShip;
					}
					else
					{
						return;
					}
				}
				else
				{
					return;
				}
			}
			else
			{
				return;
			}
		}
		
		if (space_utils.isPlayerControlledShip(objDefender))
		{
			
			return;
		}
		
		Vector objAttackers = new Vector();
		objAttackers.setSize(0);
		Vector fltDamageTracked = new Vector();
		fltDamageTracked.setSize(0);
		Vector fltGroupDamage = new Vector();
		fltGroupDamage.setSize(0);
		
		if (utils.hasLocalVar(objDefender, "combat.objAttackers"))
		{
			
			objAttackers = utils.getResizeableObjIdArrayLocalVar(objDefender, "combat.objAttackers");
			fltDamageTracked = utils.getResizeableFloatArrayLocalVar(objDefender, "combat.fltDamageTracked");
			
		}
		else
		{
			
		}
		
		int intIndex = -1;
		
		obj_id objGroup = getGroupObject(objAttacker);
		
		if (!isIdValid(objGroup))
		{
			
			intIndex = utils.getElementPositionInArray(objAttackers, objAttacker);
			
		}
		else
		{
			intIndex = utils.getElementPositionInArray(objAttackers, objGroup);
			
		}
		
		if (intIndex > -1)
		{
			
			fltDamageTracked.set(intIndex, new Float(((Float)(fltDamageTracked.get(intIndex))).floatValue() + fltDamage));
			
			if (isIdValid(objGroup))
			{
				
				Vector objGroupMembers = utils.getResizeableObjIdArrayLocalVar(objDefender, "combat.groups."+ objGroup);
				
				intIndex = utils.getElementPositionInArray(objGroupMembers, objAttacker);
				fltGroupDamage = utils.getResizeableFloatArrayLocalVar(objDefender, "combat.groups."+ objGroup + ".fltDamage");
				
				if (intIndex < 0)
				{
					
					objGroupMembers = utils.addElement(objGroupMembers, objAttacker);
					fltGroupDamage = utils.addElement(fltGroupDamage, fltDamage);
					utils.setLocalVar(objDefender, "combat.groups."+ objGroup, objGroupMembers);
					utils.setLocalVar(objDefender, "combat.groups."+ objGroup + ".fltDamage", fltGroupDamage);
				}
				else
				{
					fltGroupDamage.set(intIndex, new Float(((Float)(fltGroupDamage.get(intIndex))).floatValue() + fltDamage));
					utils.setLocalVar(objDefender, "combat.groups."+ objGroup + ".fltDamage", fltGroupDamage);
				}
			}
		}
		else
		{
			if (isIdValid(objGroup))
			{
				
				objAttackers = utils.addElement(objAttackers, objGroup);
				fltDamageTracked = utils.addElement(fltDamageTracked, fltDamage);
				Vector objGroupMembers = new Vector();
				objGroupMembers.setSize(0);
				
				objGroupMembers = utils.addElement(objGroupMembers, objAttacker);
				fltGroupDamage = utils.addElement(fltGroupDamage, fltDamage);
				utils.setLocalVar(objDefender, "combat.groups."+ objGroup, objGroupMembers);
				utils.setLocalVar(objDefender, "combat.groups."+ objGroup + ".fltDamage", fltGroupDamage);
			}
			
			objAttackers = utils.addElement(objAttackers, objAttacker);
			fltDamageTracked = utils.addElement(fltDamageTracked, fltDamage);
			
		}
		
		utils.setLocalVar(objDefender, "combat.objAttackers", objAttackers);
		utils.setLocalVar(objDefender, "combat.fltDamageTracked", fltDamageTracked);
		
		return;
	}
	
	
	public static void doFactionPointGrant(obj_id objPlayer, int intImperialFactionPoints, int intRebelFactionPoints) throws InterruptedException
	{
		if (intImperialFactionPoints != 0)
		{
			factions.addFactionStanding(objPlayer, factions.FACTION_IMPERIAL, (float) intImperialFactionPoints);
		}
		
		if (intRebelFactionPoints != 0)
		{
			factions.addFactionStanding(objPlayer, factions.FACTION_REBEL, (float) intRebelFactionPoints);
		}
		
		return;
	}
	
	
	public static void grantRewardsAndCreditForKills(obj_id objDefender) throws InterruptedException
	{
		
		dictionary dctParams = getKillPercentages(objDefender);
		
		if (dctParams == null)
		{
			
			return;
		}
		
		obj_id[] objAttackers = dctParams.getObjIdArray("objAttackers");
		
		if (objAttackers == null)
		{
			return;
		}
		
		obj_id objWinner = dctParams.getObjId("objWinner");
		
		if (!isIdValid(objWinner))
		{
			return;
		}
		
		float[] fltPercentages = dctParams.getFloatArray("fltPercentages");
		
		obj_id[] objWinningMembers = dctParams.getObjIdArray("objWinningMembers");
		float[] fltWinningMemberDamage = dctParams.getFloatArray("fltWinningMemberDamage");
		
		int intXP = getIntObjVar(objDefender, "xp.intXP");
		
		for (int intI = 0; intI < objAttackers.length; intI++)
		{
			testAbortScript();
			if ((objAttackers[intI].isLoaded() && (objAttackers[intI].isAuthoritative())))
			{
				
				if (fltPercentages[intI] > MINIMUM_DAMAGE_THRESHOLD)
				{
					
					notifyAttacker(objAttackers[intI], objDefender, fltPercentages[intI]);
					Vector objPlayers = space_transition.getContainedPlayers(objAttackers[intI], null);
					
					if ((objPlayers != null) && (objPlayers.size() > 0))
					{
						
						int intAmount = intXP;
						float fltTest = (float) intAmount;
						
						fltTest = fltTest / objPlayers.size();
						
						intAmount = (int) fltTest;
						
						String defenderType = getStringObjVar(objDefender, "ship.shipName");
						
						String[] slotNames = getAllCollectionSlotsInCategory("kill_"+ defenderType);
						
						int intImperialFactionPoints = 0;
						int intRebelFactionPoints = 0;
						
						if ((pvpGetType(objAttackers[intI]) == PVPTYPE_DECLARED))
						{
							intImperialFactionPoints = utils.getIntLocalVar(objDefender, "intImperialFactionPoints");
							intRebelFactionPoints = utils.getIntLocalVar(objDefender, "intRebelFactionPoints");
							
							fltTest = (float) intImperialFactionPoints;
							fltTest = fltTest / objPlayers.size();
							intImperialFactionPoints = (int) fltTest;
							
							fltTest = (float) intRebelFactionPoints;
							fltTest = fltTest / objPlayers.size();
							intRebelFactionPoints = (int) fltTest;
						}
						
						for (int intJ = 0; intJ < objPlayers.size(); intJ++)
						{
							testAbortScript();
							collection.spaceGetCreditForKills(((obj_id)(objPlayers.get(intJ))), slotNames);
							String tier = getPlayerTierString(((obj_id)(objPlayers.get(intJ))));
							
							if (utils.getIntLocalVar(objDefender, "intRebelFactionPoints") > 0 && factions.isRebel(((obj_id)(objPlayers.get(intJ)))))
							{
								int credit = 0;
								
								if (isCapShip(objDefender))
								{
									credit = 30;
								}
								else
								{
									credit = gcw.getSpaceKillCredit(space_flags.getPilotTier(((obj_id)(objPlayers.get(intJ)))), getShipDifficulty(objDefender));
								}
								
								String information = getName(objDefender);
								
								gcw.grantModifiedGcwPoints(((obj_id)(objPlayers.get(intJ))), credit, gcw.GCW_POINT_TYPE_SPACE_PVE, information);
							}
							
							if (utils.getIntLocalVar(objDefender, "intImperialFactionPoints") > 0 && factions.isImperial(((obj_id)(objPlayers.get(intJ)))))
							{
								int credit = 0;
								
								if (isCapShip(objDefender))
								{
									credit = 30;
								}
								else
								{
									credit = gcw.getSpaceKillCredit(space_flags.getPilotTier(((obj_id)(objPlayers.get(intJ)))), getShipDifficulty(objDefender));
								}
								
								String information = getName(objDefender);
								
								gcw.grantModifiedGcwPoints(((obj_id)(objPlayers.get(intJ))), credit, gcw.GCW_POINT_TYPE_SPACE_PVE, information);
							}
							
							String disableGroundXp = getConfigSetting("GameServer", "disableGroundXpInSpace");
							
							if (disableGroundXp == null || disableGroundXp.equals("false") || disableGroundXp.equals("0"))
							{
								if (!utils.isProfession(((obj_id)(objPlayers.get(intJ))), utils.TRADER) && !utils.isProfession(((obj_id)(objPlayers.get(intJ))), utils.ENTERTAINER))
								{
									int combatLevel = getLevel(((obj_id)(objPlayers.get(intJ))));
									
									if (combatLevel < 90)
									{
										float fltGroundXp = intAmount * GROUND_XP_PERCENT;
										int intGroundXp = (int) fltGroundXp;
										
										xp.grant(((obj_id)(objPlayers.get(intJ))), "combat_general", intGroundXp);
									}
								}
							}
							
							doFactionPointGrant(((obj_id)(objPlayers.get(intJ))), intImperialFactionPoints, intRebelFactionPoints);
							
							if (hasSkill(((obj_id)(objPlayers.get(intJ))), "pilot_rebel_navy_master"))
							{
								xp.grant(((obj_id)(objPlayers.get(intJ))), xp.SPACE_PRESTIGE_REBEL, intAmount, true);
								CustomerServiceLog("space_xp", "SPACE_PRESTIGE_REBEL|V1|"+ tier + "|TIME:"+ getGameTime() + "|PLAYER:"+ ((obj_id)(objPlayers.get(intJ))) + "|PLAYER_SHIP:"+ getPilotedShip(((obj_id)(objPlayers.get(intJ)))) + "|DEFENDER:"+ objDefender + "|DEFENDER_TYPE:"+ defenderType + "|AMT:"+ intAmount);
							}
							else if (hasSkill(((obj_id)(objPlayers.get(intJ))), "pilot_imperial_navy_master"))
							{
								xp.grant(((obj_id)(objPlayers.get(intJ))), xp.SPACE_PRESTIGE_IMPERIAL, intAmount, true);
								CustomerServiceLog("space_xp", "SPACE_PRESTIGE_IMPERIAL|V1|"+ tier + "|TIME:"+ getGameTime() + "|PLAYER:"+ ((obj_id)(objPlayers.get(intJ))) + "|PLAYER_SHIP:"+ getPilotedShip(((obj_id)(objPlayers.get(intJ)))) + "|DEFENDER:"+ objDefender + "|DEFENDER_TYPE:"+ defenderType + "|AMT:"+ intAmount);
								
							}
							else if (hasSkill(((obj_id)(objPlayers.get(intJ))), "pilot_neutral_master"))
							{
								xp.grant(((obj_id)(objPlayers.get(intJ))), xp.SPACE_PRESTIGE_PILOT, intAmount, true);
								CustomerServiceLog("space_xp", "SPACE_PRESTIGE_PILOT|V1|"+ tier + "|TIME:"+ getGameTime() + "|PLAYER:"+ ((obj_id)(objPlayers.get(intJ))) + "|PLAYER_SHIP:"+ getPilotedShip(((obj_id)(objPlayers.get(intJ)))) + "|DEFENDER:"+ objDefender + "|DEFENDER_TYPE:"+ defenderType + "|AMT:"+ intAmount);
								
							}
							else
							{
								xp.grant(((obj_id)(objPlayers.get(intJ))), xp.SPACE_COMBAT_GENERAL, intAmount, true);
								
								CustomerServiceLog("space_xp", "SPACE_COMBAT_GENERAL|V1|"+ tier + "|TIME:"+ getGameTime() + "|PLAYER:"+ ((obj_id)(objPlayers.get(intJ))) + "|PLAYER_SHIP:"+ getPilotedShip(((obj_id)(objPlayers.get(intJ)))) + "|DEFENDER:"+ objDefender + "|DEFENDER_TYPE:"+ defenderType + "|AMT:"+ intAmount);
								int badgeCount = 0;
								
								if (badge.hasBadge(((obj_id)(objPlayers.get(intJ))), "pilot_rebel_navy_naboo"))
								{
									badgeCount++;
								}
								else if (badge.hasBadge(((obj_id)(objPlayers.get(intJ))), "pilot_rebel_navy_corellia"))
								{
									badgeCount++;
								}
								else if (badge.hasBadge(((obj_id)(objPlayers.get(intJ))), "pilot_rebel_navy_tatooine"))
								{
									badgeCount++;
								}
								
								if (badge.hasBadge(((obj_id)(objPlayers.get(intJ))), "pilot_imperial_navy_naboo"))
								{
									badgeCount++;
								}
								else if (badge.hasBadge(((obj_id)(objPlayers.get(intJ))), "pilot_imperial_navy_corellia"))
								{
									badgeCount++;
								}
								else if (badge.hasBadge(((obj_id)(objPlayers.get(intJ))), "pilot_imperial_navy_tatooine"))
								{
									badgeCount++;
								}
								
								if (badge.hasBadge(((obj_id)(objPlayers.get(intJ))), "pilot_neutral_naboo"))
								{
									badgeCount++;
								}
								else if (badge.hasBadge(((obj_id)(objPlayers.get(intJ))), "pilot_neutral_corellia"))
								{
									badgeCount++;
								}
								else if (badge.hasBadge(((obj_id)(objPlayers.get(intJ))), "pilot_neutral_tatooine"))
								{
									badgeCount++;
								}
								
								if (badgeCount > 0)
								{
									int bonus = intAmount * badgeCount;
									string_id sid_xp = new string_id(xp.STF_XP_N, xp.SPACE_COMBAT_GENERAL);
									
									xp.grant(((obj_id)(objPlayers.get(intJ))), xp.SPACE_COMBAT_GENERAL, bonus, false);
									CustomerServiceLog("space_xp", "SPACE_COMBAT_GENERAL_BONUS|TIME:"+ getGameTime() + "|PLAYER:"+ ((obj_id)(objPlayers.get(intJ))) + "|PLAYER_SHIP:"+ getPilotedShip(((obj_id)(objPlayers.get(intJ)))) + "|DEFENDER:"+ objDefender + "|DEFENDER_TYPE:"+ defenderType + "|AMT:"+ bonus);
									
									prose_package pp = prose.getPackage(xp.PROSE_GRANT_XP_BONUS, sid_xp);
									
									if (exists(((obj_id)(objPlayers.get(intJ)))) && (((obj_id)(objPlayers.get(intJ))).isLoaded()))
									{
										sendSystemMessageProse(((obj_id)(objPlayers.get(intJ))), pp);
									}
								}
							}
						}
					}
				}
				
			}
		}
		
		if (objWinningMembers != null)
		{
			int intRoll = rand(0, objWinningMembers.length - 1);
			
			objWinner = objWinningMembers[rand(0, objWinningMembers.length - 1)];
		}
		
		if ((objWinner.isLoaded() && (objWinner.isAuthoritative())))
		{
			createLoot(objDefender, objWinner);
			CustomerServiceLog("space_loot", "Creating Loot from "+ objDefender + " in "+ getLocation(objDefender) + " for "+ objWinner);
			
			if (space_battlefield.isInBattlefield(objWinner))
			{
				CustomerServiceLog("battlefield", "Creating Loot from "+ objDefender + " in "+ getLocation(objDefender) + " for "+ objWinner);
			}
		}
		
		return;
	}
	
	
	public static String getPlayerTierString(obj_id player) throws InterruptedException
	{
		if (space_flags.isInTierOne(player))
		{
			return "TIER1";
		}
		else if (space_flags.isInTierTwo(player))
		{
			return "TIER2";
		}
		else if (space_flags.isInTierThree(player))
		{
			return "TIER3";
		}
		else if (space_flags.isInTierFour(player))
		{
			return "TIER4";
		}
		else
		{
			return "MASTER";
		}
	}
	
	
	public static void sendCreditNotification(obj_id objShip, int intCredits) throws InterruptedException
	{
		Vector objPlayers = space_transition.getContainedPlayers(objShip);
		
		if (space_utils.isShipWithInterior(objShip))
		{
			
			obj_id objGroup = group.getGroupObject(((obj_id)(objPlayers.get(0))));
			
			if (isIdValid(objGroup))
			{
				obj_id[] objGroupMembers = space_utils.getSpaceGroupMemberIds(objGroup);
				
				if (objGroupMembers != null)
				{
					string_id strSpam = new string_id("space/space_loot", "looted_credits");
					
					prose_package proseTest = new prose_package();
					
					proseTest = prose.setStringId(proseTest, strSpam);
					proseTest = prose.setTT(proseTest, getEncodedName(objShip));
					proseTest = prose.setDI(proseTest, intCredits);
					
					for (int intI = 0; intI < objGroupMembers.length; intI++)
					{
						testAbortScript();
						if (exists(objGroupMembers[intI]))
						{
							sendSystemMessageProse(objGroupMembers[intI], proseTest);
						}
					}
				}
			}
			else
			{
				string_id strSpam = new string_id("space/space_loot", "looted_credits_you");
				
				prose_package proseTest = new prose_package();
				
				proseTest = prose.setStringId(proseTest, strSpam);
				proseTest = prose.setDI(proseTest, intCredits);
				
				sendSystemMessageProse(((obj_id)(objPlayers.get(0))), proseTest);
			}
		}
		else
		{
			obj_id objGroup = group.getGroupObject(((obj_id)(objPlayers.get(0))));
			
			if (isIdValid(objGroup))
			{
				
				obj_id objPilot = getPilotId(objShip);
				
				if (!isIdValid(objPilot))
				{
					
					return;
				}
				
				string_id strSpam = new string_id("space/space_loot", "looted_credits");
				
				prose_package proseTest = new prose_package();
				
				proseTest = prose.setStringId(proseTest, strSpam);
				proseTest = prose.setTT(proseTest, getEncodedName(objPilot));
				proseTest = prose.setDI(proseTest, intCredits);
				
				if (isIdValid(objGroup))
				{
					obj_id[] objGroupMembers = space_utils.getSpaceGroupMemberIds(objGroup);
					if (objGroupMembers != null)
					{
						for (int intI = 0; intI < objGroupMembers.length; intI++)
						{
							testAbortScript();
							if (exists(objGroupMembers[intI]))
							{
								sendSystemMessageProse(objGroupMembers[intI], proseTest);
							}
						}
					}
				}
				
			}
			else
			{
				
				obj_id objPilot = getPilotId(objShip);
				
				if (!isIdValid(objPilot))
				{
					
					return;
				}
				string_id strSpam = new string_id("space/space_loot", "looted_credits_you");
				
				prose_package proseTest = new prose_package();
				
				proseTest = prose.setStringId(proseTest, strSpam);
				proseTest = prose.setDI(proseTest, intCredits);
				
				sendSystemMessageProse(objPilot, proseTest);
			}
		}
	}
	
	
	public static void sendItemNotification(obj_id objShip, obj_id objItem) throws InterruptedException
	{
		Vector objPlayers = space_transition.getContainedPlayers(objShip);
		
		if (space_utils.isShipWithInterior(objShip))
		{
			
			obj_id objGroup = group.getGroupObject(((obj_id)(objPlayers.get(0))));
			
			if (isIdValid(objGroup))
			{
				string_id strSpam = new string_id("space/space_loot", "looted_item");
				
				prose_package proseTest = new prose_package();
				
				proseTest = prose.setStringId(proseTest, strSpam);
				proseTest = prose.setTT(proseTest, getEncodedName(objShip));
				proseTest = prose.setTO(proseTest, getEncodedName(objItem));
				
				obj_id[] objGroupMembers = space_utils.getSpaceGroupMemberIds(objGroup);
				if (objGroupMembers != null)
				{
					for (int intI = 0; intI < objGroupMembers.length; intI++)
					{
						testAbortScript();
						if (exists(objGroupMembers[intI]))
						{
							sendSystemMessageProse(objGroupMembers[intI], proseTest);
						}
					}
				}
			}
			else
			{
				string_id strSpam = new string_id("space/space_loot", "looted_item_you");
				prose_package proseTest = new prose_package();
				
				proseTest = prose.setStringId(proseTest, strSpam);
				proseTest = prose.setTO(proseTest, getEncodedName(objItem));
				
				sendSystemMessageProse(((obj_id)(objPlayers.get(0))), proseTest);
			}
		}
		else
		{
			obj_id objGroup = group.getGroupObject(((obj_id)(objPlayers.get(0))));
			
			if (isIdValid(objGroup))
			{
				
				obj_id objPilot = getPilotId(objShip);
				
				if (!isIdValid(objPilot))
				{
					
					return;
				}
				string_id strSpam = new string_id("space/space_loot", "looted_item");
				
				prose_package proseTest = new prose_package();
				
				proseTest = prose.setStringId(proseTest, strSpam);
				proseTest = prose.setTT(proseTest, getEncodedName(objPilot));
				proseTest = prose.setTO(proseTest, getEncodedName(objItem));
				
				if (isIdValid(objGroup))
				{
					obj_id[] objGroupMembers = space_utils.getSpaceGroupMemberIds(objGroup);
					if (objGroupMembers != null)
					{
						for (int intI = 0; intI < objGroupMembers.length; intI++)
						{
							testAbortScript();
							if (exists(objGroupMembers[intI]))
							{
								sendSystemMessageProse(objGroupMembers[intI], proseTest);
							}
						}
					}
				}
			}
			else
			{
				
				obj_id objPilot = getPilotId(objShip);
				
				if (!isIdValid(objPilot))
				{
					
					return;
				}
				string_id strSpam = new string_id("space/space_loot", "looted_item_you");
				
				prose_package proseTest = new prose_package();
				
				proseTest = prose.setStringId(proseTest, strSpam);
				
				proseTest = prose.setTO(proseTest, getEncodedName(objItem));
				
				sendSystemMessageProse(objPilot, proseTest);
			}
		}
	}
	
	
	public static void createLoot(obj_id objDefender, obj_id objAttacker) throws InterruptedException
	{
		final String LOOT_TABLE = "datatables/space_loot/loot_items.iff";
		final String LOOKUP_TABLE = "datatables/space_loot/loot_lookup.iff";
		String strContainer = "object/tangible/container/drum/warren_drum_loot.iff";
		obj_id objContainer = null;
		obj_id objPilot = getPilotId(objAttacker);
		
		if (space_utils.isShipWithInterior(objAttacker))
		{
			
			objContainer = getObjIdObjVar(objAttacker, "objLootBox");
		}
		else
		{
			
			if (!isIdValid(objPilot))
			{
				
				return;
			}
			objContainer = utils.getInventoryContainer(objPilot);
			
		}
		
		if (!isIdValid(objContainer))
		{
			
			return;
		}
		
		int intCredits = getIntObjVar(objDefender, "loot.intCredits");
		if (intCredits > 0)
		{
			obj_id creditChip = space_create.makeCreditChip(objContainer, intCredits);
			if (isIdValid(creditChip))
			{
				CustomerServiceLog("space_loot", "Made credit chip worth "+ intCredits + " in container "+ objContainer + " owned by "+ objAttacker, getOwner(objAttacker));
				sendCreditNotification(objAttacker, intCredits);
			}
			else
			{
				CustomerServiceLog("space_loot", "Failed to make credit chip due to inventory/loot box being full "+ intCredits + " in container "+ objContainer + " owned by "+ objAttacker, getOwner(objAttacker));
				string_id strSpam = new string_id("space/space_loot", "no_more_credits");
				space_utils.sendSystemMessageShip(objAttacker, strSpam, true, true, true, true);
				
			}
		}
		
		if (hasObjVar(objDefender, "collection.collectionLoot") && hasObjVar(objDefender, "collection.rollBaseChance"))
		{
			
			int playerRoll = rand(1, 100);
			
			int rollBaseChance = getIntObjVar(objDefender, "collection.rollBaseChance");
			
			if (buff.hasBuff(objPilot, "nova_orion_rank6_lucky_salvage"))
			{
				rollBaseChance = rollBaseChance + NOVA_ORION_BUFF_COLLECTIBLES_BONUS;
			}
			
			if (playerRoll <= rollBaseChance)
			{
				
				String collectionLoot = getStringObjVar(objDefender, "collection.collectionLoot");
				
				boolean success = getCollectionLoot(objAttacker, objPilot, objContainer, collectionLoot);
				if (!success)
				{
					CustomerServiceLog("CollectionLootChannel: ", "LootFailed: "+ objDefender + "("+ objPilot + ")"+ " failed to receive dropped space collection item due to a failure in the getCollectionLoot function. Notify development.");
				}
			}
		}
		
		int intNumItems = getIntObjVar(objDefender, "loot.intNumItems");
		String strTable = getStringObjVar(objDefender, "loot.strLootTable");
		
		String[] strColumns = dataTableGetStringColumnNoDefaults(LOOKUP_TABLE, strTable);
		
		if (strColumns == null)
		{
			return;
		}
		
		if (isIdValid(objPilot) && (utils.getPlayerProfession(objPilot) == utils.SMUGGLER))
		{
			smuggler.spaceContrabandDropCheck(objPilot);
		}
		
		for (int intI = 0; intI < intNumItems; intI++)
		{
			testAbortScript();
			
			int intRoll = rand(0, strColumns.length - 1);
			
			String[] strItems = dataTableGetStringColumnNoDefaults(LOOT_TABLE, strColumns[intRoll]);
			
			if (strItems == null)
			{
				return;
			}
			
			if ((strItems != null) && (strItems.length > 0))
			{
				int intRoll2 = rand(0, strItems.length - 1);
				String itemTemplateName = "";
				
				if (getConfigSetting("GameServer", "enableLevelUpLoot") != null && (rand(1, 10000) == 1))
				{
					itemTemplateName = "object/tangible/loot/quest/levelup_lifeday_orb.iff";
				}
				else
				{
					itemTemplateName = strItems[intRoll2];
				}
				
				if (space_battlefield.isInBattlefield(objAttacker))
				{
					CustomerServiceLog("battlefield", "%TU Created "+ itemTemplateName + " in "+ objContainer + " contained by "+ objAttacker, getOwner(objAttacker));
				}
				
				boolean success = attemptToGrantLootItem(itemTemplateName, objAttacker, objContainer);
				if (!success)
				{
					break;
				}
			}
			else
			{
				
				return;
			}
		}
		
		if (hasObjVar(objDefender, "loot.novaOrionLoot"))
		{
			
			int chance = rand(1,100);
			
			if (buff.hasBuff(objPilot, "nova_orion_rank6_lucky_salvage"))
			{
				chance = chance - NOVA_ORION_BUFF_MIDLITHE_BONUS;
			}
			
			if (chance <= NOVA_ORION_RESOURCE_CHANCE)
			{
				int novaOrionLootModifier = getIntObjVar(objDefender, "loot.novaOrionLoot");
				
				int min = NOVA_ORION_RESOURCE_MIN + novaOrionLootModifier;
				int max = NOVA_ORION_RESOURCE_MAX + novaOrionLootModifier;
				int count = rand(min, max);
				
				obj_id[] contents = getContents(objContainer);
				boolean foundCrystal = false;
				if (contents != null && contents.length > 0)
				{
					for (int i = 0; i < contents.length; ++i)
					{
						testAbortScript();
						if (static_item.isStaticItem(contents[i]) && getStaticItemName(contents[i]).equals(NOVA_ORION_RESOURCE))
						{
							int oldCount = getCount(contents[i]);
							int newCount = oldCount + count;
							setCount(contents[i], newCount);
							foundCrystal = true;
							break;
						}
					}
				}
				if (!foundCrystal)
				{
					obj_id lootItem = static_item.createNewItemFunction(NOVA_ORION_RESOURCE, objContainer);
					setCount(lootItem, count);
				}
				string_id strSpam = new string_id("space/space_loot", "looted_item_you");
				
				prose_package proseTest = new prose_package();
				
				proseTest = prose.setStringId(proseTest, strSpam);
				
				proseTest = prose.setTO(proseTest, utils.packStringId(new string_id(static_item.STATIC_ITEM_NAME, NOVA_ORION_RESOURCE)));
				
				sendSystemMessageProse(objPilot, proseTest);
				
			}
		}
		
		return;
	}
	
	
	public static void grantLootPermission(obj_id objWinner, obj_id objContainer) throws InterruptedException
	{
		
		Vector objWinners = new Vector();
		objWinners.setSize(0);
		
		if (hasObjVar(objContainer, "objLooters"))
		{
			objWinners = getResizeableObjIdArrayObjVar(objContainer, "objLooters");
		}
		
		objWinners = utils.addElement(objWinners, objWinner);
		setObjVar(objContainer, "objLooters", objWinners);
		
		return;
	}
	
	
	public static void notifyAttacker(obj_id objAttacker, obj_id objDefender, float fltPercentage) throws InterruptedException
	{
		dictionary dctParams = new dictionary();
		
		dctParams.put("ship", objDefender);
		String strShipType = getStringObjVar(objDefender, "ship.shipName");
		
		if (strShipType != null)
		{
			dctParams.put("strShipType", strShipType);
		}
		
		obj_id sourceSpawner = getObjIdObjVar(objDefender, "objParent");
		
		if (isIdValid(sourceSpawner))
		{
			String spawner_name = getStringObjVar(sourceSpawner, "strSpawnerName");
			
			if (spawner_name != null)
			{
				dctParams.put("strSpawnerName", spawner_name);
			}
		}
		
		dctParams.put("fltPercentage", fltPercentage);
		space_utils.notifyObject(objAttacker, "targetDestroyed", dctParams);
		
		return;
	}
	
	
	public static void notifyAttackerDisabled(obj_id objAttacker, obj_id objDefender, dictionary dctParams) throws InterruptedException
	{
		dctParams.put("ship", objDefender);
		String strShipType = getStringObjVar(objDefender, "ship.shipName");
		
		if (strShipType != null)
		{
			dctParams.put("strShipType", strShipType);
		}
		
		obj_id sourceSpawner = getObjIdObjVar(objDefender, "objParent");
		
		if (isIdValid(sourceSpawner))
		{
			String spawner_name = getStringObjVar(sourceSpawner, "strSpawnerName");
			
			if (spawner_name != null)
			{
				dctParams.put("strSpawnerName", spawner_name);
			}
		}
		
		space_utils.notifyObject(objAttacker, "targetDisabled", dctParams);
		
		return;
	}
	
	
	public static dictionary getKillPercentages(obj_id objDefender) throws InterruptedException
	{
		
		Vector objAttackers = new Vector();
		objAttackers.setSize(0);
		Vector fltPercentages = new Vector();
		fltPercentages.setSize(0);
		float fltTotalDamage = 0;
		dictionary dctParams = new dictionary();
		
		float fltMaxDamage = 0;
		int intMaxDamageIndex = 0;
		
		obj_id[] objTrackedAttackers = utils.getObjIdArrayLocalVar(objDefender, "combat.objAttackers");
		
		float[] fltTrackedDamage = utils.getFloatArrayLocalVar(objDefender, "combat.fltDamageTracked");
		
		if (fltTrackedDamage != null && objTrackedAttackers != null)
		{
			for (int intI = 0; intI < fltTrackedDamage.length; intI++)
			{
				testAbortScript();
				if ((objTrackedAttackers[intI].isLoaded() && (objTrackedAttackers[intI].isAuthoritative())))
				{
					if (fltMaxDamage < fltTrackedDamage[intI])
					{
						fltMaxDamage = fltTrackedDamage[intI];
						intMaxDamageIndex = intI;
					}
					
					fltTotalDamage = fltTotalDamage + fltTrackedDamage[intI];
				}
				
			}
			
			dctParams.put("fltTotalDamage", fltTotalDamage);
			dctParams.put("fltMaxDamage", fltMaxDamage);
			dctParams.put("intMaxDamageIndex", intMaxDamageIndex);
			dctParams.put("objWinner", objTrackedAttackers[intMaxDamageIndex]);
			
			if (utils.hasLocalVar(objDefender, "combat.groups." + objTrackedAttackers[intMaxDamageIndex]))
			{
				dctParams.put("objWinningMembers", utils.getObjIdArrayLocalVar(objDefender, "combat.groups."+ objTrackedAttackers[intMaxDamageIndex]));
				dctParams.put("fltWinningMemberDamage", utils.getFloatArrayLocalVar(objDefender, "combat.groups."+ objTrackedAttackers[intMaxDamageIndex] + "fltDamage"));
			}
			
			float fltDamage = 0;
			float fltPercentage = 0;
			
			for (int intI = 0; intI < objTrackedAttackers.length; intI++)
			{
				testAbortScript();
				
				fltDamage = fltTrackedDamage[intI];
				fltPercentage = fltDamage / fltTotalDamage;
				
				if (utils.hasLocalVar(objDefender, "combat.groups." + objTrackedAttackers[intI]))
				{
					obj_id[] objGroupMembers = utils.getObjIdArrayLocalVar(objDefender, "combat.groups."+ objTrackedAttackers[intI]);
					
					for (int intJ = 0; intJ < objGroupMembers.length; intJ++)
					{
						testAbortScript();
						
						if ((objGroupMembers[intJ].isLoaded() && (objGroupMembers[intJ].isAuthoritative())))
						{
							objAttackers = utils.addElement(objAttackers, objGroupMembers[intJ]);
							fltPercentages = utils.addElement(fltPercentages, fltPercentage);
						}
					}
				}
				else
				{
					
					if (objTrackedAttackers[intI].isLoaded() && objTrackedAttackers[intI].isAuthoritative())
					{
						objAttackers = utils.addElement(objAttackers, objTrackedAttackers[intI]);
						fltPercentages = utils.addElement(fltPercentages, fltPercentage);
					}
				}
			}
			
			if (objAttackers.size() > 0)
			{
				dctParams.put("objAttackers", objAttackers);
				dctParams.put("fltPercentages", fltPercentages);
			}
		}
		
		return dctParams;
	}
	
	
	public static void setEfficiencyModifier(int intSlot, obj_id objShip, float fltGeneralModifier, float fltEnergyModifier) throws InterruptedException
	{
		
		if ((fltGeneralModifier == 1.0f) && (fltEnergyModifier == 1.0f))
		{
			clearEfficiencyModifier(intSlot, objShip);
			return;
			
		}
		else
		{
			String strName = ship_chassis_slot_type.names[intSlot];
			
			utils.setLocalVar(objShip, strName + "Efficiency", 1);
			utils.setLocalVar(objShip, strName + "EfficiencyGeneral", fltGeneralModifier);
			utils.setLocalVar(objShip, strName + "EfficiencyEnergy", fltEnergyModifier);
			recalculateEfficiency(intSlot, objShip);
		}
		
		return;
	}
	
	
	public static void clearEfficiencyModifier(int intSlot, obj_id objShip) throws InterruptedException
	{
		String strName = ship_chassis_slot_type.names[intSlot];
		
		utils.removeLocalVar(objShip, strName + "Efficiency");
		utils.removeLocalVar(objShip, strName + "EfficiencyGeneral");
		utils.removeLocalVar(objShip, strName + "EfficiencyEnergy");
		recalculateEfficiency(intSlot, objShip);
		
		return;
	}
	
	
	public static boolean isEfficiencyModified(int intSlot, obj_id objShip) throws InterruptedException
	{
		String strName = ship_chassis_slot_type.names[intSlot];
		
		if (utils.hasLocalVar(objShip, strName + "Efficiency"))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static void recalculateEfficiency(int intSlot, obj_id objShip, float fltRawEfficiency) throws InterruptedException
	{
		
		if (fltRawEfficiency == 0)
		{
			return;
		}
		
		String strName = ship_chassis_slot_type.names[intSlot];
		float fltEnergyModifier = 1.0f;
		float fltGeneralModifier = 1.0f;
		
		if (utils.hasLocalVar(objShip, strName + "Efficiency"))
		{
			fltGeneralModifier = utils.getFloatLocalVar(objShip, strName + "EfficiencyGeneral");
			fltEnergyModifier = utils.getFloatLocalVar(objShip, strName + "EfficiencyEnergy");
			
		}
		float fltFinalGeneralEfficiency = fltGeneralModifier * fltRawEfficiency;
		float fltFinalEnergyEfficiency = fltEnergyModifier;
		
		setEfficiencies(intSlot, objShip, fltFinalGeneralEfficiency, fltFinalEnergyEfficiency);
	}
	
	
	public static void recalculateEfficiencyGeneral(int intSlot, obj_id objShip) throws InterruptedException
	{
		recalculateEfficiencyGeneral(intSlot, objShip, getRawEfficiency(intSlot, objShip));
	}
	
	
	public static void recalculateEfficiencyGeneral(int intSlot, obj_id objShip, float fltRawEfficiency) throws InterruptedException
	{
		
		if (fltRawEfficiency == 0)
		{
			return;
		}
		
		String strName = ship_chassis_slot_type.names[intSlot];
		float fltGeneralModifier = 1.0f;
		
		if (utils.hasLocalVar(objShip, strName + "Efficiency"))
		{
			fltGeneralModifier = utils.getFloatLocalVar(objShip, strName + "EfficiencyGeneral");
		}
		
		float fltFinalGeneralEfficiency = fltGeneralModifier * fltRawEfficiency;
		
		setEfficiencyGeneral(intSlot, objShip, fltFinalGeneralEfficiency);
	}
	
	
	public static void recalculateEfficiency(int intSlot, obj_id objShip) throws InterruptedException
	{
		float fltRawEfficiency = getRawEfficiency(intSlot, objShip);
		
		recalculateEfficiency(intSlot, objShip, fltRawEfficiency);
	}
	
	
	public static void setEfficiencyGeneral(int intSlot, obj_id objShip, float fltGeneralModifier) throws InterruptedException
	{
		if (fltGeneralModifier < MINIMUM_EFFICIENCY)
		{
			fltGeneralModifier = MINIMUM_EFFICIENCY;
		}
		setShipComponentEfficiencyGeneral(objShip, intSlot, fltGeneralModifier);
		
		return;
	}
	
	
	public static void setEfficiencies(int intSlot, obj_id objShip, float fltGeneralModifier, float fltEnergyModifier) throws InterruptedException
	{
		
		if (fltEnergyModifier < MINIMUM_EFFICIENCY)
		{
			fltEnergyModifier = MINIMUM_EFFICIENCY;
		}
		if (fltGeneralModifier < MINIMUM_EFFICIENCY)
		{
			fltGeneralModifier = MINIMUM_EFFICIENCY;
		}
		
		setShipComponentEfficiencyGeneral(objShip, intSlot, fltGeneralModifier);
		setShipComponentEfficiencyEnergy(objShip, intSlot, fltEnergyModifier);
		
		return;
	}
	
	
	public static void zeroEfficienciesClearPowerups(int intSlot, obj_id objShip) throws InterruptedException
	{
		String strName = ship_chassis_slot_type.names[intSlot];
		
		utils.removeLocalVar(objShip, strName + "Efficiency");
		utils.removeLocalVar(objShip, strName + "EfficiencyGeneral");
		utils.removeLocalVar(objShip, strName + "EfficiencyEnergy");
		setEfficiencies(intSlot, objShip, 0.0f, 0.0f);
		
		return;
	}
	
	
	public static float getRawEfficiency(int intSlot, obj_id objShip) throws InterruptedException
	{
		
		boolean boolUpdateComponent = true;
		float fltCurrentHitpoints = getShipComponentHitpointsCurrent(objShip, intSlot);
		float fltMaximumHitpoints = getShipComponentHitpointsMaximum(objShip, intSlot);
		
		if (fltMaximumHitpoints == 0)
		{
			if (hasScript(objShip, "space.ship.ship_interior"))
			{
				dictionary dctParams = new dictionary();
				
				dctParams.put("intSlot", intSlot);
				space_utils.notifyObject(objShip, "componentDisabledByDamage", dctParams);
				boolUpdateComponent = false;
			}
			
			space_utils.setComponentDisabled(objShip, intSlot, true);
			
			return 0;
		}
		
		if (fltCurrentHitpoints == 0)
		{
			if (hasScript(objShip, "space.ship.ship_interior"))
			{
				dictionary dctParams = new dictionary();
				
				dctParams.put("intSlot", intSlot);
				space_utils.notifyObject(objShip, "componentDisabledByDamage", dctParams);
				boolUpdateComponent = false;
			}
			
			space_utils.setComponentDisabled(objShip, intSlot, true);
			
			return 0;
		}
		
		if (boolUpdateComponent)
		{
			if (hasScript(objShip, "space.ship.ship_interior"))
			{
				dictionary dctParams = new dictionary();
				
				dctParams.put("intSlot", intSlot);
				dctParams.put("fltCurrentHitpoints", fltCurrentHitpoints);
				dctParams.put("fltMaximumHitpoints", fltMaximumHitpoints);
				space_utils.notifyObject(objShip, "componentEfficiencyChanged", dctParams);
			}
		}
		
		float fltEfficiency = fltCurrentHitpoints / fltMaximumHitpoints;
		
		return fltEfficiency;
	}
	
	
	public static void recalculateEfficiencies(obj_id objComponent) throws InterruptedException
	{
		
		float fltMaximumHitpoints = space_crafting.getComponentMaximumHitpoints(objComponent);
		float fltCurrentHitpoints = space_crafting.getComponentCurrentHitpoints(objComponent);
		float fltEfficiency = 0;
		
		if (fltCurrentHitpoints > 0)
		{
			fltEfficiency = fltMaximumHitpoints / fltCurrentHitpoints;
		}
		else
		{
			fltEfficiency = 0;
		}
		
		setEfficiencies(objComponent, fltEfficiency, fltEfficiency);
	}
	
	
	public static void setEfficiencies(obj_id objComponent, float fltGeneralModifier, float fltEnergyModifier) throws InterruptedException
	{
		
		if (fltEnergyModifier < MINIMUM_EFFICIENCY)
		{
			fltEnergyModifier = MINIMUM_EFFICIENCY;
		}
		if (fltGeneralModifier < MINIMUM_EFFICIENCY)
		{
			fltGeneralModifier = MINIMUM_EFFICIENCY;
		}
		
		space_crafting.setComponentGeneralEfficiency(objComponent, fltGeneralModifier);
		space_crafting.setComponentEnergyEfficiency(objComponent, fltEnergyModifier);
		
		return;
	}
	
	
	public static boolean drainEnergyFromCapacitor(obj_id objShip, float fltEnergy, boolean boolOverride) throws InterruptedException
	{
		
		float fltCurrentEnergy = getShipCapacitorEnergyCurrent(objShip);
		
		if (fltCurrentEnergy < fltEnergy)
		{
			if (boolOverride)
			{
				fltCurrentEnergy = 0;
			}
			else
			{
				return false;
			}
			
		}
		else
		{
			fltCurrentEnergy = fltCurrentEnergy - fltEnergy;
		}
		
		setShipCapacitorEnergyCurrent(objShip, fltCurrentEnergy);
		
		return true;
	}
	
	
	public static boolean drainEnergyFromShield(obj_id objShip, int intSide, float fltEnergy, boolean boolOverride) throws InterruptedException
	{
		if (intSide == FRONT)
		{
			float fltCurrentEnergy = getShipShieldHitpointsFrontCurrent(objShip);
			
			if (fltCurrentEnergy < fltEnergy)
			{
				if (boolOverride)
				{
					fltCurrentEnergy = 0;
				}
				else
				{
					return false;
				}
				
			}
			else
			{
				fltCurrentEnergy = fltCurrentEnergy - fltEnergy;
			}
			
			setShipShieldHitpointsFrontCurrent(objShip, fltCurrentEnergy);
			
			return true;
		}
		else
		{
			float fltCurrentEnergy = getShipShieldHitpointsBackCurrent(objShip);
			
			if (fltCurrentEnergy < fltEnergy)
			{
				if (boolOverride)
				{
					fltCurrentEnergy = 0;
				}
				else
				{
					return false;
				}
				
			}
			else
			{
				fltCurrentEnergy = fltCurrentEnergy - fltEnergy;
			}
			
			setShipShieldHitpointsBackCurrent(objShip, fltCurrentEnergy);
			
			return true;
		}
	}
	
	
	public static boolean addEnergyToCapacitor(obj_id objShip, int intWeaponCapacitor, float fltEnergy, boolean boolOverride) throws InterruptedException
	{
		
		float fltCurrentEnergy = getShipCapacitorEnergyCurrent(objShip);
		float fltMaximumEnergy = getShipCapacitorEnergyMaximum(objShip);
		float fltDifference = fltMaximumEnergy - fltCurrentEnergy;
		
		if (fltDifference < fltEnergy)
		{
			if (boolOverride)
			{
				fltCurrentEnergy = fltMaximumEnergy;
			}
			else
			{
				return false;
			}
		}
		else
		{
			fltCurrentEnergy = fltCurrentEnergy + fltEnergy;
		}
		
		setShipCapacitorEnergyCurrent(objShip, fltCurrentEnergy);
		
		return true;
	}
	
	
	public static boolean addEnergyToShield(obj_id objShip, int intSide, float fltEnergy, boolean boolOverride) throws InterruptedException
	{
		if (intSide == FRONT)
		{
			float fltCurrentEnergy = getShipShieldHitpointsFrontCurrent(objShip);
			float fltMaximumEnergy = getShipShieldHitpointsFrontMaximum(objShip);
			float fltDifference = fltMaximumEnergy - fltCurrentEnergy;
			
			if (fltDifference < fltEnergy)
			{
				if (boolOverride)
				{
					fltCurrentEnergy = fltMaximumEnergy;
				}
				else
				{
					return false;
				}
			}
			else
			{
				fltCurrentEnergy = fltCurrentEnergy + fltEnergy;
			}
			
			setShipShieldHitpointsFrontCurrent(objShip, fltCurrentEnergy);
			
			return true;
		}
		else
		{
			float fltCurrentEnergy = getShipShieldHitpointsBackCurrent(objShip);
			float fltMaximumEnergy = getShipShieldHitpointsBackMaximum(objShip);
			float fltDifference = fltMaximumEnergy - fltCurrentEnergy;
			
			if (fltDifference < fltEnergy)
			{
				if (boolOverride)
				{
					fltCurrentEnergy = fltMaximumEnergy;
				}
				else
				{
					return false;
				}
			}
			else
			{
				fltCurrentEnergy = fltCurrentEnergy + fltEnergy;
			}
			
			setShipShieldHitpointsBackCurrent(objShip, fltCurrentEnergy);
			
			return true;
		}
	}
	
	
	public static boolean transferFrontShieldToBack(obj_id objShip, float fltPercentage) throws InterruptedException
	{
		float fltCurrentEnergy = getShipShieldHitpointsFrontCurrent(objShip);
		float fltEnergy = fltCurrentEnergy * fltPercentage;
		
		if (drainEnergyFromShield(objShip, space_combat.FRONT, fltEnergy, false))
		{
			addEnergyToShield(objShip, space_combat.BACK, fltEnergy, true);
			
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean transferBackShieldToFront(obj_id objShip, float fltPercentage) throws InterruptedException
	{
		float fltCurrentEnergy = getShipShieldHitpointsBackCurrent(objShip);
		float fltEnergy = fltCurrentEnergy * fltPercentage;
		
		if (drainEnergyFromShield(objShip, space_combat.BACK, fltEnergy, false))
		{
			addEnergyToShield(objShip, space_combat.FRONT, fltEnergy, true);
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean reinforceShieldsFromCapacitor(obj_id objShip, float fltPercentage) throws InterruptedException
	{
		float fltCurrentEnergy = getShipCapacitorEnergyCurrent(objShip);
		
		float fltEnergy = fltCurrentEnergy * fltPercentage;
		
		if (drainEnergyFromCapacitor(objShip, fltEnergy, false))
		{
			
			fltCurrentEnergy = getShipCapacitorEnergyCurrent(objShip);
			
			addEnergyToShield(objShip, space_combat.FRONT, fltEnergy / 2, true);
			
			addEnergyToShield(objShip, space_combat.BACK, fltEnergy / 2, true);
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isComponentMandatory(obj_id objShip, int intSlot) throws InterruptedException
	{
		for (int intI = 0; intI < MANDATORY_COMPONENTS.length; intI++)
		{
			testAbortScript();
			if (intSlot == MANDATORY_COMPONENTS[intI])
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public static boolean isShipDisabled(obj_id objShip, int intSlot) throws InterruptedException
	{
		if (isComponentMandatory(objShip, intSlot))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isShipDisabled(obj_id objShip) throws InterruptedException
	{
		
		for (int intI = 0; intI < MANDATORY_COMPONENTS.length; intI++)
		{
			testAbortScript();
			if (isShipComponentDisabled(objShip, MANDATORY_COMPONENTS[intI]))
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public static void killSpacePlayer(obj_id objShip) throws InterruptedException
	{
		
		LOG("space", "Killing "+ objShip);
		Vector objPlayers = space_utils.getPlayersInShip(objShip);
		
		if (utils.hasScriptVar(objShip, "intEjecting"))
		{
			utils.removeScriptVar(objShip, "intEjecting");
		}
		
		if (hasObjVar(objShip, "intAlarmsOn"))
		{
			space_crafting.turnOnInteriorAlarms(objShip);
		}
		
		space_crafting.fixAllPlasmaConduits(objShip);
		
		clearHyperspace(objShip);
		
		if (objPlayers != null)
		{
			for (int intI = 0; intI < objPlayers.size(); intI++)
			{
				testAbortScript();
				
				groundquests.sendSignal(((obj_id)(objPlayers.get(intI))), "smugglerEnemyIncap");
				string_id strSpam = new string_id("space/space_interaction", "your_ship_esploded");
				
				sendSystemMessage(((obj_id)(objPlayers.get(intI))), strSpam);
				CustomerServiceLog("space_death", "Wounding "+ ((obj_id)(objPlayers.get(intI))) + " because "+ objShip + " exploded", ((obj_id)(objPlayers.get(intI))));
				healing.healClone(((obj_id)(objPlayers.get(intI))), false);
			}
		}
		
		ship_ai.unitRemoveFromAllAttackTargetLists(objShip);
		ship_ai.unitSetAutoAggroImmune(objShip, true);
		
		space_pilot_command.allPurposeShipComponentReset(objShip);
		space_transition.clearOvertStatus(objShip);
		obj_id objPilot = getPilotId(objShip);
		
		if (utils.hasScriptVar(objShip, "intPVPKill"))
		{
			space_crafting.repairDamageToPercentage(objPilot, objShip, 1.0f);
		}
		
		clearDeathFlags(objShip);
		clearCondition(objShip, CONDITION_EJECT);
		
		if (space_battlefield.isInBattlefield(objShip))
		{
			CustomerServiceLog("battlefield", "Battlefield death for "+ objShip + " removing them from "+ getLocation(objShip), getOwner(objShip));
			doBattlefieldRepairCheck(objShip);
			location locTest = getLocationObjVar(objShip, "battlefield.locRespawnLocation");
			
			if (locTest == null)
			{
				locTest = getLocation(objShip);
				locTest.x = 0;
				locTest.y = 0;
				locTest.z = 0;
				
				warpPlayer(objShip, locTest.area, locTest.x, locTest.y, locTest.z, null, locTest.x, locTest.y, locTest.z, null, true);
				
			}
			else
			{
				removeObjVar(objShip, "battlefield.locRespawnLocation");
				warpPlayer(objShip, locTest.area, locTest.x, locTest.y, locTest.z, null, locTest.x, locTest.y, locTest.z, null, true);
			}
			
			return;
		}
		else
		{
			obj_id objClosestStation = getClosestSpaceStation(objShip);
			
			if (isIdValid(objClosestStation) && (!exists(objClosestStation)))
			{
				objClosestStation = null;
			}
			
			if (!isIdValid(objClosestStation))
			{
				
				if (isIdValid(objPilot))
				{
					sendSystemMessageTestingOnly(getPilotId(objShip), "Respawn failure. You don't have any space stations defined, or no quest manager for them to register with. Sorry :(");
				}
				
				location locTest = getLocation(objShip);
				
				locTest.x = 0;
				locTest.y = 0;
				locTest.z = 0;
				
				warpPlayer(objShip, locTest.area, locTest.x, locTest.y, locTest.z, null, locTest.x, locTest.y, locTest.z, null, true);
			}
			else
			{
				
				transform trTest = space_utils.getRandomPositionInSphere(getTransform_o2p(objClosestStation), 425f, 450f, false);
				
				vector vctTest = space_utils.getVector(objClosestStation);
				
				CustomerServiceLog("space_loot", "Killing "+ objShip + " and sending them to "+ getLocation(objClosestStation), getOwner(objShip));
				transform trFinalTransform = space_utils.faceTransformToVector(trTest, vctTest);
				
				location locTest = getLocation(objShip);
				
				space_utils.openCommChannelAfterLoad(objShip, objClosestStation);
				warpPlayer(objShip, locTest.area, vctTest.x, vctTest.y, vctTest.z, null, vctTest.x, vctTest.y, vctTest.z, null, true);
				setTransform_o2p(objShip, trFinalTransform);
				
			}
			
		}
		
		return;
	}
	
	
	public static obj_id getClosestSpaceStation(obj_id objShip) throws InterruptedException
	{
		obj_id objQuestManager = null;
		
		if (!utils.checkConfigFlag("ScriptFlags", "liveSpaceServer"))
		{
			try
			{
				objQuestManager = getNamedObject(space_quest.QUEST_MANAGER);
			}
			catch(Throwable err)
			{
				return null;
			}
		}
		else
		{
			objQuestManager = getNamedObject(space_quest.QUEST_MANAGER);
		}
		
		if (!isIdValid(objQuestManager))
		{
			
			return null;
		}
		
		location locTest = getLocation(objShip);
		
		obj_id[] objSpaceStations = utils.getObjIdArrayScriptVar(objQuestManager, "objSpaceStations");
		
		if (objSpaceStations == null || objSpaceStations.length == 0)
		{
			
			return null;
		}
		
		float fltDistance = 320000;
		obj_id objClosestStation = null;
		
		for (int intI = 0; intI < objSpaceStations.length; intI++)
		{
			testAbortScript();
			
			location locStation = getLocation(objSpaceStations[intI]);
			
			float fltTest = getDistance(locTest, locStation);
			
			if (fltTest < fltDistance)
			{
				objClosestStation = objSpaceStations[intI];
				
				fltDistance = fltTest;
			}
		}
		
		return objClosestStation;
	}
	
	
	public static void initializeCommandTimer(obj_id pilot, int commandDelay) throws InterruptedException
	{
		int time = getGameTime();
		
		time = time + commandDelay;
		utils.setScriptVar(pilot, "cmd.commandTimeStamp", time);
		dictionary outparams = new dictionary();
		
		if (!messageTo(pilot, "commandTimerTimeout", outparams, (float) (commandDelay), false))
		{
			debugServerConsoleMsg(null, "+++ space_combat . initializeCommandTimer +++ FAILED to send messageTo. ObjID of pilot was: "+ pilot + " amount of time delay was: "+ commandDelay);
		}
	}
	
	
	public static boolean commandTimePassed(obj_id pilot) throws InterruptedException
	{
		int intCurrentTime = getGameTime();
		int intLastTime = utils.getIntScriptVar(pilot, "cmd.commandTimeStamp");
		
		if (intCurrentTime < intLastTime)
		{
			string_id strSpam = new string_id("space/space_interaction", "pilot_command_delay");
			
			sendSystemMessage(pilot, strSpam);
			
			if ((intLastTime - intCurrentTime) < 60)
			{
				
				string_id strSpam2 = new string_id("space/space_interaction", "pilot_command_delay_bit");
				
				sendSystemMessage(pilot, strSpam2);
			}
			else if ((intLastTime - intCurrentTime) < 120)
			{
				
				string_id strSpam2 = new string_id("space/space_interaction", "pilot_command_delay_some");
				
				sendSystemMessage(pilot, strSpam2);
			}
			
			return false;
		}
		
		return true;
	}
	
	
	public static boolean doReactorPumpPulse(obj_id pilot, obj_id ship) throws InterruptedException
	{
		if (!isShipSlotInstalled(ship, ship_chassis_slot_type.SCST_reactor))
		{
			debugServerConsoleMsg(null, "+++ SPACE COMMAND . doReactorPumpPulse +++ No reactor onboard. What the!?.");
			return false;
		}
		
		flightDroidVocalize(ship, 1);
		
		if (space_combat.doesReactorScram(pilot, ship, "level1command"))
		{
			space_combat.scramReactor(pilot, ship);
			return true;
		}
		
		if (isEfficiencyModified(ship_chassis_slot_type.SCST_reactor, ship))
		{
			setEfficiencyModifier(ship_chassis_slot_type.SCST_reactor, ship, 1.0f, 1.0f);
			
			string_id strSpam = new string_id("space/space_interaction", "aborting_reactor_pump");
			
			sendSystemMessage(pilot, strSpam);
		}
		else
		{
			string_id strSpam = new string_id("space/space_interaction", "do_reactor_pump_one");
			string_id strSpamTwo = new string_id("space/space_interaction", "do_reactor_pump_two");
			
			sendSystemMessage(pilot, strSpam);
			sendSystemMessage(pilot, strSpamTwo);
			
			float rndReactorUpgradeEffect = rand(1.1f, 1.4f);
			
			debugServerConsoleMsg(null, "+++ SPACE COMMAND . doReactorPumpPulse +++ Looks like the upgrade will be: "+ rndReactorUpgradeEffect);
			
			setEfficiencyModifier(ship_chassis_slot_type.SCST_reactor, ship, rndReactorUpgradeEffect, 1.0f);
			setEfficiencyModifier(ship_chassis_slot_type.SCST_engine, ship, rndReactorUpgradeEffect, 1.0f);
			
			int pumpPulseLoops = (int) (rand(1.0f, 10.0f));
			
			dictionary outparams = new dictionary();
			
			outparams.put("ship", ship);
			outparams.put("loops", pumpPulseLoops);
			outparams.put("pilot", pilot);
			
			if (!messageTo(ship, "reactorPumpPulseTimeout", outparams, 5.0f, false))
			{
				debugServerConsoleMsg(null, "+++ SPACE COMMAND . doReactorPumpPulse +++ FAILED to send messageTo. ObjID of ship was: "+ ship + " objID of pilot was: "+ pilot + " number of loops was: "+ pumpPulseLoops);
			}
		}
		
		return true;
		
	}
	
	
	public static boolean doesReactorScram(obj_id pilot, obj_id ship, String difficultyOfCommand) throws InterruptedException
	{
		if (!isIdValid(pilot) || !isIdValid(ship))
		{
			return true;
		}
		
		int successLevel = doPilotCommandSkillCheck(pilot, difficultyOfCommand);
		
		if (successLevel > 4)
		{
			return true;
		}
		
		return false;
	}
	
	
	public static void scramReactor(obj_id pilot, obj_id ship) throws InterruptedException
	{
		string_id strSpam = new string_id("space/space_interaction", "scramming_reactor");
		
		sendSystemMessage(pilot, strSpam);
		
		space_utils.setComponentDisabled(ship, ship_chassis_slot_type.SCST_reactor, true);
		space_utils.setComponentDisabled(ship, ship_chassis_slot_type.SCST_engine, true);
		
		if (isShipSlotInstalled(ship, ship_chassis_slot_type.SCST_booster))
		{
			space_utils.setComponentDisabled(ship, ship_chassis_slot_type.SCST_booster, true);
		}
		
		if (getShipHasWings(ship))
		{
			clearCondition(ship, CONDITION_WINGS_OPENED);
		}
		
		String cefPlayBackHardpoint = targetHardpointForCefPlayback(ship);
		
		playClientEffectObj(pilot, "clienteffect/space_command/scram_reactor_shuttingdown_siren.cef", ship, cefPlayBackHardpoint);
		playClientEffectObj(pilot, "clienteffect/space_command/scram_reactor_shutdown_engine.cef", ship, cefPlayBackHardpoint);
		
		int scramLoops = (int) (rand(1.0f, 10.0f));
		
		dictionary outparams = new dictionary();
		
		outparams.put("ship", ship);
		outparams.put("loops", scramLoops);
		outparams.put("pilot", pilot);
		
		messageTo(ship, "unScramReactor", outparams, 5.0f, false);
	}
	
	
	public static int getPlayerCommandSkill(obj_id pilot) throws InterruptedException
	{
		int playerSkill = 0;
		
		if (hasSkill(pilot, "pilot_rebel_navy_master") || hasSkill(pilot, "pilot_imperial_navy_master") || hasSkill(pilot, "pilot_neutral_master"))
		{
			playerSkill = 10;
		}
		else if (hasSkill(pilot, "pilot_imperial_navy_procedures_04") || hasSkill(pilot, "pilot_neutral_procedures_04") || hasSkill(pilot, "pilot_rebel_navy_procedures_04"))
		{
			playerSkill = 7;
		}
		else if (hasSkill(pilot, "pilot_imperial_navy_procedures_03") || hasSkill(pilot, "pilot_neutral_procedures_03") || hasSkill(pilot, "pilot_rebel_navy_procedures_03"))
		{
			playerSkill = 5;
		}
		else if (hasSkill(pilot, "pilot_imperial_navy_procedures_02") || hasSkill(pilot, "pilot_neutral_procedures_02") || hasSkill(pilot, "pilot_rebel_navy_procedures_02"))
		{
			playerSkill = 3;
		}
		else
		{
			playerSkill = 1;
		}
		
		return playerSkill;
	}
	
	
	public static int getAttackerSkill(obj_id targetShip) throws InterruptedException
	{
		final int CONST_PILOT_FAIL = 0;
		
		int pilotSkill = 0;
		
		if (ship_ai.isPlayerShip(targetShip))
		{
			obj_id playerPilot = space_utils.getCommandExecutor(targetShip);
			
			if (isIdValid(playerPilot))
			{
				pilotSkill = getPlayerCommandSkill(playerPilot);
			}
		}
		else if (hasObjVar(targetShip, "ship.pilotType"))
		{
			String strPilot = getStringObjVar(targetShip, "ship.pilotType");
			
			if (strPilot == null)
			{
				debugServerConsoleMsg(null, "space_combat.getNpcPilotSkill -- strPilot checked out as NULL");
				pilotSkill = CONST_PILOT_FAIL;
			}
			else if (strPilot.equals("ace"))
			{
				pilotSkill = 10;
			}
			else if (strPilot.equals("average"))
			{
				pilotSkill = 5;
			}
			else if (strPilot.equals("rookie"))
			{
				pilotSkill = 2;
			}
			else
			{
				pilotSkill = CONST_PILOT_FAIL;
			}
			return pilotSkill;
		}
		return CONST_PILOT_FAIL;
	}
	
	
	public static boolean doIffScramble(obj_id pilot, obj_id ship) throws InterruptedException
	{
		debugServerConsoleMsg(null, "space_combat.doIffScramble ++ entered function");
		
		obj_id[] myAttackers = ship_ai.unitGetWhoIsTargetingMe(ship);
		
		if ((myAttackers == null) || (myAttackers.length == 0))
		{
			string_id strSpam = new string_id("space/space_interaction", "iff_spoof_dont_bother");
			
			sendSystemMessage(pilot, strSpam);
			return false;
		}
		
		flightDroidVocalize(ship, 1);
		
		int rnd = rand(1, 10);
		
		int playerSkill = getPlayerCommandSkill(pilot);
		int spoofedAttackers = 0;
		dictionary params = new dictionary();
		
		int numberOfJammedDudes = 0;
		int sumTotalOfSkillCheckDeltas = 0;
		int lowestSkillCheckDelta = 0;
		int highestSkillCheckDelta = 0;
		
		for (int i = 0; i < myAttackers.length; i++)
		{
			testAbortScript();
			if (isIdValid(myAttackers[i]))
			{
				obj_id thisAttacker = myAttackers[i];
				
				if (exists(thisAttacker))
				{
					if (!space_utils.isPlayerControlledShip(myAttackers[i]))
					{
						int attackerSkill = getAttackerSkill(myAttackers[i]);
						
						if ((playerSkill + rnd) > (attackerSkill + 3))
						{
							debugServerConsoleMsg(null, "space_combat.doIffScramble ++ iff scrambled ship "+ thisAttacker + " with my skill "+ playerSkill + " and botskill "+ attackerSkill + " and rnd "+ rnd);
							
							ship_ai.unitRemoveAttackTarget(thisAttacker, ship);
							
							playClientEffectObj(ship, "clienteffect/space_command/shp_shocked_radio_01_noshake.cef", thisAttacker, "");
							
							spoofedAttackers++;
							int skillCheckDelta = (playerSkill + rnd) - (attackerSkill + 3);
							
							sumTotalOfSkillCheckDeltas += skillCheckDelta;
							numberOfJammedDudes++;
							
						}
						else
						{
							debugServerConsoleMsg(null, "space_combat.doIffScramble ++ iff did NOT scramble ship "+ thisAttacker + " with my skill "+ playerSkill + " and botskill "+ attackerSkill + " and rnd "+ rnd);
						}
					}
				}
			}
		}
		
		if (spoofedAttackers <= 0)
		{
			string_id strSpam = new string_id("space/space_interaction", "iff_spoofing_unsuccessful");
			
			sendSystemMessage(pilot, strSpam);
			
			dictionary outparams = new dictionary();
			
			outparams.put("pilot", pilot);
			outparams.put("ship", ship);
			messageTo(pilot, "unIffScramble", outparams, 360.0f, false);
			
			return false;
		}
		
		float agroImmuneTime = (sumTotalOfSkillCheckDeltas / numberOfJammedDudes) * 5;
		
		if (agroImmuneTime < 20.0f)
		{
			agroImmuneTime = 20.0f;
		}
		
		ship_ai.unitSetAutoAggroImmuneTime(ship, agroImmuneTime);
		
		string_id strSpam = new string_id("space/space_interaction", "iff_spoofed");
		
		sendSystemMessage(pilot, strSpam);
		
		dictionary outparams = new dictionary();
		
		outparams.put("pilot", pilot);
		outparams.put("ship", ship);
		messageTo(pilot, "unIffScramble", outparams, 360.0f, false);
		
		return true;
	}
	
	
	public static void disableWeaponry(obj_id ship, boolean disable) throws InterruptedException
	{
		for (int chassisSlot = ship_chassis_slot_type.SCST_weapon_first; chassisSlot < ship_chassis_slot_type.SCST_weapon_last; chassisSlot++)
		{
			testAbortScript();
			if (isShipSlotInstalled(ship, chassisSlot))
			{
				if (disable == true)
				{
					space_utils.setComponentDisabled(ship, chassisSlot, true);
				}
				else
				{
					float fltCurrentHitPoints = getShipComponentHitpointsCurrent(ship, chassisSlot);
					
					if (fltCurrentHitPoints > 0)
					{
						space_utils.setComponentDisabled(ship, chassisSlot, false);
						recalculateEfficiency(chassisSlot, ship);
					}
				}
			}
		}
		
		return;
	}
	
	
	public static String[] getProgrammedDroidCommands(obj_id objPlayer) throws InterruptedException
	{
		
		obj_id objShip = space_transition.getContainingShip(objPlayer);
		
		if (!isIdValid(objShip))
		{
			return null;
		}
		
		obj_id objControlDevice = getDroidControlDeviceForShip(objShip);
		
		if (!isIdValid(objControlDevice))
		{
			return null;
		}
		obj_id objDataPad = utils.getDatapad(objControlDevice);
		
		if (!isIdValid(objDataPad))
		{
			
			return null;
		}
		
		obj_id[] objContents = utils.getContents(objDataPad);
		Vector strCommands = new Vector();
		
		for (int intI = 0; intI < objContents.length; intI++)
		{
			testAbortScript();
			if (hasObjVar(objContents[intI], "strDroidCommand"))
			{
				String strTest = getStringObjVar(objContents[intI], "strDroidCommand");
				
				strCommands = utils.addElement(strCommands, strTest);
			}
		}
		
		return (String[])strCommands.toArray(new String[0]);
	}
	
	
	public static String[] getDroidCommands(obj_id objPlayer) throws InterruptedException
	{
		String[] strRawCommands = getCommandListingForPlayer(objPlayer);
		
		Vector strDroidCommands = new Vector();
		strDroidCommands.setSize(0);
		
		for (int intI = 0; intI < strRawCommands.length; intI++)
		{
			testAbortScript();
			int intIndex = strRawCommands[intI].indexOf("droidcommand");
			
			if (intIndex > -1)
			{
				strDroidCommands = utils.addElement(strDroidCommands, strRawCommands[intI]);
			}
		}
		
		if (strDroidCommands.size() > 0)
		{
			return (String[])strDroidCommands.toArray(new String[0]);
		}
		else
		{
			return null;
		}
	}
	
	
	public static void grantCapitalShipSequenceRewardsAndCreditForKills(obj_id objDefender) throws InterruptedException
	{
		
		dictionary dctParams = getKillPercentages(objDefender);
		
		if (dctParams == null)
		{
			
			return;
		}
		obj_id[] objAttackers = dctParams.getObjIdArray("objAttackers");
		obj_id objWinner = dctParams.getObjId("objWinner");
		float[] fltPercentages = dctParams.getFloatArray("fltPercentages");
		
		obj_id[] objWinningMembers = utils.getObjIdArrayLocalVar(objDefender, "objWinningMembers");
		
		utils.removeLocalVar(objDefender, "objAttackers");
		utils.removeLocalVar(objDefender, "objWinner");
		utils.removeLocalVar(objDefender, "fltPercentages");
		utils.removeLocalVar(objDefender, "objWinningMembers");
		
		int intXP = 500;
		
		if (objAttackers != null)
		{
			for (int intI = 0; intI < objAttackers.length; intI++)
			{
				testAbortScript();
				
				if (fltPercentages[intI] > MINIMUM_DAMAGE_THRESHOLD)
				{
					
					notifyAttacker(objAttackers[intI], objDefender, fltPercentages[intI]);
					
					int intAmount = intXP;
					float fltTest = (float) intAmount;
					
					fltTest = fltTest * fltPercentages[intI];
					intAmount = (int) fltTest;
					xp.grant(getPilotId(objAttackers[intI]), xp.SPACE_COMBAT_GENERAL, intAmount, true);
					
				}
			}
		}
		return;
		
	}
	
	
	public static void flightDroidVocalize(obj_id ship, int vocalizePriority) throws InterruptedException
	{
		flightDroidVocalize(ship, vocalizePriority, null);
	}
	
	
	public static void flightDroidVocalize(obj_id ship, int vocalizePriority, prose_package pp) throws InterruptedException
	{
		debugServerConsoleMsg(null, "SPACE_COMBAT.flightDroidVocalize: ############################################################# Just Entered function.");
		
		if (!isIdValid(ship))
		{
			return;
		}
		
		obj_id droidPcd = getDroidControlDeviceForShip(ship);
		
		debugServerConsoleMsg(null, "SPACE_COMBAT.flightDroidVocalize: Ship's droidPCD is: "+ droidPcd);
		
		if (!isIdValid(droidPcd))
		{
			debugServerConsoleMsg(null, "SPACE_COMBAT.flightDroidVocalize: Wanted to vocalize, but no droid PCD assigned to ship, so no vocalize.");
			return;
		}
		
		if (utils.hasLocalVar(ship, "droidcmd.muteDroid"))
		{
			debugServerConsoleMsg(null, "SPACE_COMBAT.flightDroidVocalize: Wanted to vocalize, but ship has muteDroid localvar, so aborted.");
			return;
		}
		
		obj_id player = getShipPilot(ship);
		
		int currentTime = getGameTime();
		int storedVocalizeTime = 0;
		
		if (utils.hasLocalVar(ship, "droidVocTime"))
		{
			storedVocalizeTime = utils.getIntLocalVar(ship, "droidVocTime");
		}
		int timeDelta = currentTime - storedVocalizeTime;
		
		if (vocalizePriority < 2)
		{
			if (!utils.hasLocalVar(ship, "droidVocTime") || timeDelta > 3)
			{
				utils.setLocalVar(ship, "droidVocTime", getGameTime());
				vocalizeSpaceDroid(droidPcd, player, ship, vocalizePriority);
				
				if (pp != null)
				{
					obj_id droid = callable.getCDCallable(droidPcd);
					
					if (isIdValid(droid))
					{
						space_utils.tauntPlayer(player, droid, pp);
					}
					else if (space_crafting.isFlightComputerController(droidPcd))
					{
						space_utils.tauntPlayer(player, droidPcd, pp);
					}
					else
					{
						debugServerConsoleMsg(null, "SPACE_COMBAT.flightDroidVocalize: ERROR! COULD NOT FIND A VALID DROID OBJ_ID. space_utils.tauntPlayer call loc #1");
					}
				}
			}
			
			return;
		}
		else if (vocalizePriority < 3)
		{
			if (!utils.hasLocalVar(ship, "droidVocTime") || timeDelta > 7)
			{
				utils.setLocalVar(ship, "droidVocTime", getGameTime());
				obj_id droid = callable.getCDCallable(droidPcd);
				
				if (isIdValid(droid))
				{
					space_utils.tauntPlayer(player, droid, pp);
				}
				else if (space_crafting.isFlightComputerController(droidPcd))
				{
					space_utils.tauntPlayer(player, droidPcd, pp);
				}
				else
				{
					debugServerConsoleMsg(null, "SPACE_COMBAT.flightDroidVocalize: ERROR! COULD NOT FIND A VALID DROID OBJ_ID. space_utils.tauntPlayer call loc #2");
				}
				
				vocalizeSpaceDroid(droidPcd, player, ship, vocalizePriority);
			}
			
			return;
		}
		else
		{
			if (!utils.hasLocalVar(ship, "droidVocalizationQueued"))
			{
				dictionary outparams = new dictionary();
				
				outparams.put("ship", ship);
				outparams.put("vocalizePriority", vocalizePriority);
				int droidVocalizeDelay = rndDroidVocalizeDelay();
				
				utils.setLocalVar(ship, "droidVocalizationQueued", droidVocalizeDelay);
				messageTo(ship, "flightDroidVocalize", outparams, droidVocalizeDelay, false);
				
				return;
			}
			else
			{
				if (utils.getIntLocalVar(ship, "droidVocalizationQueued") >= getGameTime())
				{
					if (!utils.hasLocalVar(ship, "droidVocTime") || timeDelta > 7)
					{
						utils.setLocalVar(ship, "droidVocTime", getGameTime());
						vocalizeSpaceDroid(droidPcd, player, ship, vocalizePriority);
					}
					utils.removeLocalVar(ship, "droidVocalizationQueued");
				}
			}
		}
		
		return;
	}
	
	
	public static int rndDroidVocalizeDelay() throws InterruptedException
	{
		int currentTime = getGameTime();
		int rndVocDelay = (int) (rand(5.0f * DROID_VOCALIZE_DELAY_CONST, 30.0f * DROID_VOCALIZE_DELAY_CONST));
		int newProjectedVocTime = currentTime + rndVocDelay;
		
		return newProjectedVocTime;
	}
	
	
	public static void vocalizeSpaceDroid(obj_id droidPcd, obj_id player, obj_id ship, int vocalizePriority) throws InterruptedException
	{
		debugServerConsoleMsg(null, "SPACE_COMBAT.vocalizeSpaceDroid: ############################################################# Just Entered function.");
		
		obj_id droid = null;
		
		debugServerConsoleMsg(null, "SPACE_COMBAT.vocalizeSpaceDroid: Just read in pet.current ObjIdScriptVar off of droidPCD. The droidPcd we grabbed from was: "+ droidPcd + " and the pet.current obj_id was: "+ droid);
		int droidSpecies = 0;
		
		if (space_crafting.isFlightComputerController(droidPcd))
		{
			droidSpecies = 216;
		}
		else
		{
			droid = callable.getCDCallable(droidPcd);
			droidSpecies = getSpecies(droid);
		}
		
		debugServerConsoleMsg(null, "SPACE_COMBAT.vocalizeSpaceDroid: just got the species off the droid. The value was: "+ droidSpecies);
		int speciesChecksum = dataTableSearchColumnForInt(droidSpecies, 0, DROID_VOCALIZE_DATATABLE);
		
		if (speciesChecksum == -1)
		{
			debugServerConsoleMsg(null, "SPACE_COMBAT.vocalizeSpaceDroid: speciesChecksum failed out as being -1");
			return;
		}
		
		int column = 0;
		
		if (vocalizePriority == 1)
		{
			column = 7;
		}
		else if (vocalizePriority == 2)
		{
			column = rand(4, 6);
		}
		else
		{
			column = rand(1, 3);
		}
		
		int droidSpeciesRow = dataTableSearchColumnForInt(droidSpecies, 0, DROID_VOCALIZE_DATATABLE);
		
		debugServerConsoleMsg(null, "SPACE_COMBAT.vocalizeSpaceDroid: Found the droid species designator int in row: "+ droidSpeciesRow + " of the DROID_VOCALIZE_DATATABLE");
		String droidVocalizationEffectFile = dataTableGetString(DROID_VOCALIZE_DATATABLE, droidSpeciesRow, column);
		
		debugServerConsoleMsg(null, "SPACE_COMBAT.vocalizeSpaceDroid: Droid vocalization we are going to play is from file:"+ droidVocalizationEffectFile);
		
		if (droidVocalizationEffectFile == null)
		{
			debugServerConsoleMsg(null, "SPACE_COMBAT.vocalizeSpaceDroid: droidVocalizationEffectFile was null. Kicking out! ");
			
			return;
		}
		else if (droidSpecies == 214 || droidSpecies == 215 || droidSpecies == 216)
		{
			playClientEffectObj(player, droidVocalizationEffectFile, ship, "engine_glow1");
		}
		else
		{
			playClientEffectObj(player, droidVocalizationEffectFile, ship, "astromech");
		}
		
		return;
	}
	
	
	public static String targetHardpointForCefPlayback(obj_id ship) throws InterruptedException
	{
		String templateName = utils.getTemplateFilenameNoPath(ship);
		String shipUsesThis = dataTableGetString(space_crafting.STARSHIP_DROID_TABLE, templateName, "usesDroidOrFlightComputer");
		
		debugServerConsoleMsg(null, "+++ SPACE_CRAFTING . Got shipUsesThis. It was: "+ shipUsesThis);
		
		if (shipUsesThis == null || shipUsesThis.equals(""))
		{
			debugServerConsoleMsg(null, "SPACE_COMBAT.targetHardpointForCefPlayback: shipUsesThis failed out as being -1");
			
			return "engine_glow1";
		}
		
		if (shipUsesThis.equals("astromech"))
		{
			return "astromech";
		}
		
		return "engine_glow1";
	}
	
	
	public static void createFlightDroidFromData(obj_id petControlDevice, obj_id objContainer) throws InterruptedException
	{
		String creatureName = getStringObjVar(petControlDevice, "pet.creatureName");
		
		if (creatureName == null || creatureName.equals(""))
		{
			return;
		}
		
		obj_id datapad = getContainedBy(petControlDevice);
		
		if (!isIdValid(datapad))
		{
			debugServerConsoleMsg(null, "SPACE_COMBAT.createFlightDroidFromData: FAILURE. Kicking out after databad isIdValid check failed. Datapad was: "+ datapad);
			return;
		}
		
		obj_id master = getContainedBy(datapad);
		
		if (!isIdValid(master))
		{
			debugServerConsoleMsg(null, "SPACE_COMBAT.createFlightDroidFromData: FAILURE. Kicking out after master isIdValid check failed. Master was: "+ master);
			return;
		}
		
		if (!hasScript(master, "ai.pet_master"))
		{
			debugServerConsoleMsg(null, "SPACE_COMBAT.createFlightDroidFromData: No ai.pet_master script detected. Adding it.");
			attachScript(master, "ai.pet_master");
		}
		
		obj_id pet = null;
		
		if (!isIdValid(objContainer))
		{
			debugServerConsoleMsg(null, "SPACE_COMBAT.createFlightDroidFromData: FIRST CUSTOM SPACEFLIGHT DROID CREATION BIT. objContainer isIdValid check failed. objContainer was: "+ objContainer);
			return;
		}
		else
		{
			pet = create.object(creatureName, objContainer, false, true);
		}
		
		if (!isIdValid(pet))
		{
			debugServerConsoleMsg(null, "SPACE_COMBAT.createFlightDroidFromData: FAILURE. new pet obj_id failed isIdValid check "+ pet);
			sendSystemMessage(master, pet_lib.SID_PRIVATE_HOUSE);
			return;
		}
		
		sendDirtyObjectMenuNotification(petControlDevice);
		
		callable.setCallableLinks(master, petControlDevice, pet);
		
		if (hasObjVar(petControlDevice, "pet.crafted"))
		{
			pet_lib.setCraftedPetStatsByGrowth(petControlDevice, pet, 10);
			
			if (ai_lib.aiGetNiche(pet) == NICHE_DROID)
			{
				pet_lib.initDroidCraftedInventoryDroid(pet, petControlDevice);
				pet_lib.initDroidCraftedDatapadDroid(pet, petControlDevice);
				pet_lib.initDroidPersonality(pet, petControlDevice);
				pet_lib.initDroidRepairPower(pet, petControlDevice);
				
			}
		}
		
		pet_lib.reduceStatsForVitality(pet, petControlDevice);
		
		pet_lib.restoreDamage(pet, petControlDevice);
		
		if (hasObjVar(petControlDevice, "ai.pet.command"))
		{
			pet_lib.initializePetCommandList(pet, petControlDevice);
		}
		
		if (hasObjVar(petControlDevice, "alreadyTrained"))
		{
			setObjVar(pet, "alreadyTrained", true);
		}
		
		if (hasObjVar(petControlDevice, pet_lib.VAR_PALVAR_BASE))
		{
			pet_lib.restoreCustomization(pet, petControlDevice);
		}
		
		if (pet_lib.isDroidPet(pet))
		{
			copyObjVar(petControlDevice, pet, "module_data");
		}
		
		String name = getAssignedName(petControlDevice);
		
		setName(pet, name);
	}
	
	
	public static boolean removeFlightDroidFromShip(obj_id droidControlDevice, obj_id droid) throws InterruptedException
	{
		if (isIdValid(droidControlDevice))
		{
			debugServerConsoleMsg(null, "SPACE_COMBAT.removeFlightDroidFromShip: petControlDevice obj_id isIdValid check passed.");
			
			obj_id currentPet = callable.getCDCallable(droidControlDevice);
			
			debugServerConsoleMsg(null, "SPACE_COMBAT.removeFlightDroidFromShip: got the objIdScriptVar off of the pcd. currentPet obj_id value was: "+ currentPet);
			
			if (isIdValid(currentPet) && currentPet == droid)
			{
				debugServerConsoleMsg(null, "SPACE_COMBAT.removeFlightDroidFromShip: currentPet checked out as a a valid obj_id, and it equals the pet obj_id that was passed into the function at start. Proceeding.");
				pet_lib.savePetInfo(droid, droidControlDevice);
				setObjVar(droidControlDevice, "pet.timeStored", getGameTime());
				callable.setCDCallable(droidControlDevice, null);
			}
		}
		
		debugServerConsoleMsg(null, "+++ SPACE_COMBAT.removeFlightDroidFromShip: +++ destroying the pet now");
		utils.setScriptVar(droid, "stored", true);
		
		if (destroyObject(droid))
		{
			return true;
		}
		
		debugServerConsoleMsg(null, "+++ SPACE_COMBAT.removeFlightDroidFromShip: +++ WARNINGWARNING - FAILED TO DESTROY SELF");
		
		return false;
	}
	
	
	public static void openFlightComputerDatapad(obj_id navicompControlDevice, obj_id owner) throws InterruptedException
	{
		
		obj_id navicompDPad = utils.getDatapad(navicompControlDevice);
		
		if (!isIdValid(navicompDPad))
		{
			debugServerConsoleMsg(null, "********** aaa.a navicompDPad objId failed isIdValid check, and we're kicking out.");
			return;
		}
		
		obj_id myContainer = getContainedBy(navicompControlDevice);
		obj_id yourPad = utils.getPlayerDatapad(owner);
		
		if (myContainer != yourPad)
		{
			if (hasScript(myContainer, "item.tool.navicomputer_control_device"))
			{
				
				putIn(navicompControlDevice, yourPad);
			}
			return;
		}
		
		utils.requestContainerOpen(owner, navicompDPad);
	}
	
	
	public static boolean isAnInitializedNavicomp(obj_id navicomp) throws InterruptedException
	{
		if (!isIdValid(navicomp))
		{
			return false;
		}
		
		if (!hasObjVar(navicomp, "item.controlDevice"))
		{
			return false;
		}
		
		obj_id controlDevice = getObjIdObjVar(navicomp, "item.controlDevice");
		
		if (!isIdValid(controlDevice))
		{
			return false;
		}
		
		if (!hasObjVar(controlDevice, "item.current"))
		{
			return false;
		}
		
		obj_id currentItem = getObjIdObjVar(controlDevice, "item.current");
		
		if (currentItem != navicomp)
		{
			return false;
		}
		
		obj_id controllerDpad = utils.getDatapad(controlDevice);
		
		if (!isIdValid(controllerDpad))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean readyForEmergencyPower(obj_id ship) throws InterruptedException
	{
		if (!isIdValid(ship))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static int[] getInstalledMissileWeapons(obj_id objShip) throws InterruptedException
	{
		Vector intWeapons = new Vector();
		intWeapons.setSize(0);
		
		for (int intI = FIRST_WEAPON; intI < LAST_WEAPON; intI++)
		{
			testAbortScript();
			if (isShipSlotInstalled(objShip, intI))
			{
				int intCrc = getShipComponentCrc(objShip, intI);
				
				if ((getShipComponentDescriptorWeaponIsMissile(intCrc)) && (!getShipComponentDescriptorWeaponIsCountermeasure(intCrc)))
				{
					intWeapons = utils.addElement(intWeapons, intI);
				}
			}
		}
		
		int[] _intWeapons = new int[0];
		if (intWeapons != null)
		{
			_intWeapons = new int[intWeapons.size()];
			for (int _i = 0; _i < intWeapons.size(); ++_i)
			{
				_intWeapons[_i] = ((Integer)intWeapons.get(_i)).intValue();
			}
		}
		return _intWeapons;
	}
	
	
	public static void emergencyCmdDamageToShipsSystems(obj_id pilot, obj_id ship, float percentToDamageTo, String clientEffect, int primaryComponent) throws InterruptedException
	{
		if (!isIdValid(ship))
		{
			return;
		}
		
		final int[] WEAPON_SLOTS =
		{
			ship_chassis_slot_type.SCST_weapon_0,
			ship_chassis_slot_type.SCST_weapon_1,
			ship_chassis_slot_type.SCST_weapon_2,
			ship_chassis_slot_type.SCST_weapon_3,
			ship_chassis_slot_type.SCST_weapon_4,
			ship_chassis_slot_type.SCST_weapon_5,
			ship_chassis_slot_type.SCST_weapon_6,
			ship_chassis_slot_type.SCST_weapon_7
		};
		
		if (!clientEffect.equals("") && clientEffect != null)
		{
			playClientEffectObj(pilot, clientEffect, ship, targetHardpointForCefPlayback(ship));
		}
		
		float fltDamageToComponent = (getShipComponentHitpointsMaximum(ship, primaryComponent) * (percentToDamageTo / 100));
		
		if ((primaryComponent != NONE) && (primaryComponent != ALL_WEAPONS))
		{
			if (!isShipSlotInstalled(ship, primaryComponent))
			{
				
				string_id strSpam = new string_id("space/space_interaction", "component_not_installed");
				
				sendSystemMessage(pilot, strSpam);
				return;
			}
			
			float fltCurrentHitpoints = getShipComponentHitpointsCurrent(ship, primaryComponent);
			float fltCurrentArmorHitpoints = getShipComponentArmorHitpointsCurrent(ship, primaryComponent);
			
			fltCurrentArmorHitpoints = fltCurrentArmorHitpoints - fltDamageToComponent;
			
			if (fltDamageToComponent > 0)
			{
				float currentHitpoints = getShipComponentHitpointsCurrent(ship, primaryComponent);
				float currentArmorHitpoints = getShipComponentArmorHitpointsCurrent(ship, primaryComponent);
				
				currentArmorHitpoints = currentArmorHitpoints - fltDamageToComponent;
				if (currentArmorHitpoints < 0)
				{
					float fltRemainingDamage = Math.abs(currentArmorHitpoints);
					
					currentHitpoints = currentHitpoints - fltRemainingDamage;
					if (currentHitpoints < 0)
					{
						string_id strSpam = new string_id("space/space_interaction", "droid_component_too_damaged");
						string_id strComponent = new string_id("space/space_interaction", space_crafting.getShipComponentStringType(primaryComponent));
						prose_package proseTest = prose.getPackage(strSpam, strComponent);
						
						return;
					}
					else
					{
						setShipComponentArmorHitpointsCurrent(ship, primaryComponent, 0);
						setShipComponentHitpointsCurrent(ship, primaryComponent, currentHitpoints);
						recalculateEfficiency(primaryComponent, ship);
					}
				}
				else
				{
					setShipComponentArmorHitpointsCurrent(ship, primaryComponent, currentArmorHitpoints);
					
				}
			}
		}
		
		return;
	}
	
	
	public static void vampiricTypeShipsSystemsRepair(obj_id pilot, obj_id ship, int successLevel) throws InterruptedException
	{
		
		if (!isIdValid(ship))
		{
			return;
		}
		
		float highestDamagedComponentPercentage = 0.0f;
		int highestDamagedComponentIndexNumber = 0;
		float componentDamagePercentageSum = 0.0f;
		int badComponentCounter = 0;
		float highestDamagedComponentMaxPoints = 0.0f;
		float totalComponentMaxHitPoints = 0.0f;
		
		int[] componentSlotsTarget = space_crafting.getShipInstalledSlots(ship);
		float[] currentDamagePercentage = new float[componentSlotsTarget.length];
		
		for (int currentShipComponent = 0; currentShipComponent < componentSlotsTarget.length; currentShipComponent++)
		{
			testAbortScript();
			
			float currentHitPoints = getShipComponentHitpointsCurrent(ship, currentShipComponent);
			
			float maxHitPoints = getShipComponentHitpointsMaximum(ship, currentShipComponent);
			
			if (maxHitPoints >= 1.0f)
			{
				
				totalComponentMaxHitPoints += maxHitPoints;
				
				currentDamagePercentage[currentShipComponent] = (((maxHitPoints - currentHitPoints) / maxHitPoints) * 100);
				
				componentDamagePercentageSum += currentDamagePercentage[currentShipComponent];
				
				if (currentDamagePercentage[currentShipComponent] > highestDamagedComponentPercentage)
				{
					
					highestDamagedComponentPercentage = currentDamagePercentage[currentShipComponent];
					
					highestDamagedComponentIndexNumber = currentShipComponent;
					
					highestDamagedComponentMaxPoints = getShipComponentHitpointsMaximum(ship, currentShipComponent);
					
				}
			}
			else
			{
				
				badComponentCounter++;
			}
		}
		
		float averageDamagePercentageMinusMaxOne = (componentDamagePercentageSum - highestDamagedComponentPercentage) / (componentSlotsTarget.length - (badComponentCounter + 1));
		
		float damagePercentageDelta = highestDamagedComponentPercentage - averageDamagePercentageMinusMaxOne;
		
		if (damagePercentageDelta < 5.0f)
		{
			
			string_id strSpam = new string_id("space/space_interaction", "vampiric_repair_useless_abort");
			
			sendSystemMessage(pilot, strSpam);
			return;
		}
		
		float damagePointDispersionPool = (damagePercentageDelta / 100.0f) * highestDamagedComponentMaxPoints;
		
		float currentHitPointAdjustment = getShipComponentHitpointsCurrent(ship, highestDamagedComponentIndexNumber) + damagePointDispersionPool;
		
		if (currentHitPointAdjustment > highestDamagedComponentMaxPoints)
		{
			currentHitPointAdjustment = highestDamagedComponentMaxPoints;
		}
		setShipComponentHitpointsCurrent(ship, highestDamagedComponentIndexNumber, currentHitPointAdjustment);
		
		if (successLevel == 1)
		{
			damagePointDispersionPool = damagePointDispersionPool - (damagePointDispersionPool * 0.15f);
		}
		else if (successLevel == 2)
		{
			damagePointDispersionPool = damagePointDispersionPool - (damagePointDispersionPool * 0.05f);
		}
		else if (successLevel == 4)
		{
			damagePointDispersionPool = damagePointDispersionPool + (damagePointDispersionPool * 0.05f);
		}
		else if (successLevel == 6)
		{
			damagePointDispersionPool = damagePointDispersionPool + (damagePointDispersionPool * 0.20f);
		}
		
		debugServerConsoleMsg(null, "vampiricTypeShipsSystemsRepair ********** Just resized the damagePointDispersionPool based upon successlevel of skill check.");
		debugServerConsoleMsg(null, "vampiricTypeShipsSystemsRepair ********** Size of skill-modified damagePointDispersionPool is: "+ damagePointDispersionPool);
		
		int damageLoops = (int) (damagePointDispersionPool / 10);
		
		if (damageLoops > 5)
		{
			damageLoops = 5;
		}
		dictionary params = new dictionary();
		
		params.put("damage_loops", damageLoops);
		params.put("pilot", pilot);
		messageTo(ship, "vRepairDamageCEFLoop", params, 3.0f, false);
		
		for (int currentShipComponent = 0; currentShipComponent < componentSlotsTarget.length; currentShipComponent++)
		{
			testAbortScript();
			float mmaxHitPoints = getShipComponentHitpointsMaximum(ship, currentShipComponent);
			
			if (mmaxHitPoints >= 1.0f)
			{
				float percentageOfTotalComponentHitPoints = mmaxHitPoints / totalComponentMaxHitPoints;
				float hitPointDelta = percentageOfTotalComponentHitPoints * damagePointDispersionPool;
				float currentHitPoints = getShipComponentHitpointsCurrent(ship, currentShipComponent);
				
				if (hitPointDelta > currentHitPoints)
				{
					hitPointDelta = (currentHitPoints - 1);
				}
				float newHitPoints = currentHitPoints - hitPointDelta;
				
				setShipComponentHitpointsCurrent(ship, currentShipComponent, newHitPoints);
				
			}
			else
			{
				
			}
		}
		
		if (damagePointDispersionPool > 0.0f)
		{
			
			float currentHitPoints = getShipComponentHitpointsCurrent(ship, highestDamagedComponentIndexNumber);
			
			setShipComponentHitpointsCurrent(ship, highestDamagedComponentIndexNumber, currentHitPoints - damagePointDispersionPool);
		}
		
		string_id strSpam = new string_id("space/space_interaction", "vampiric_repair_underway");
		
		sendSystemMessage(pilot, strSpam);
		
		space_pilot_command.allPurposeShipComponentReset(ship);
		
		successLevel--;
		switch (successLevel)
		{
			case 5: 	
			strSpam = new string_id("space/space_interaction", "vampiric_big_fail");
			sendSystemMessage(pilot, strSpam);
			break;
			case 4:
			break;
			case 3: 	
			strSpam = new string_id("space/space_interaction", "vampiric_slight_fail");
			sendSystemMessage(pilot, strSpam);
			break;
			case 2: 	
			strSpam = new string_id("space/space_interaction", "vampiric_success");
			sendSystemMessage(pilot, strSpam);
			break;
			case 1: 	
			strSpam = new string_id("space/space_interaction", "vampiric_good_success");
			sendSystemMessage(pilot, strSpam);
			break;
			case 0: 	
			strSpam = new string_id("space/space_interaction", "vampiric_great_success");
			sendSystemMessage(pilot, strSpam);
			break;
		}
		
		return;
	}
	
	
	public static int getShipsPilotCommandSkillCheckMods(obj_id ship) throws InterruptedException
	{
		int mods = 0;
		int currentTime = getGameTime();
		
		if (utils.hasLocalVar(ship, "cmd.wasDamagedSkillMod"))
		{
			int duration = utils.getIntLocalVar(ship, "cmd.wasDamagedSkillMod");
			
			if (currentTime < duration)
			{
				mods += 20;
			}
		}
		
		if (utils.hasLocalVar(ship, "cmd.firedWeaponsSkillMod"))
		{
			int duration = utils.getIntLocalVar(ship, "cmd.firedWeaponsSkillMod");
			
			if (currentTime < duration)
			{
				mods += 10;
			}
		}
		
		return mods;
	}
	
	
	public static int doPilotCommandSkillCheck(obj_id player, String difficultyOfCommand) throws InterruptedException
	{
		
		debugServerConsoleMsg(null, "=========================================================");
		debugServerConsoleMsg(null, "SPACE_COMBAT.doPilotCommandSkillCheck ============= ENTERED FUNCTION ");
		debugServerConsoleMsg(null, "passed in player obj_id of: "+ player + " and passed in difficulty of command of: "+ difficultyOfCommand);
		
		int risk = 0;
		
		if (difficultyOfCommand.equals("level1command"))
		{
			risk = 20;
		}
		else if (difficultyOfCommand.equals("level2command"))
		{
			risk = 40;
		}
		else if (difficultyOfCommand.equals("level3command"))
		{
			risk = 60;
		}
		else if (difficultyOfCommand.equals("level4command"))
		{
			risk = 80;
		}
		else if (difficultyOfCommand.equals("level5command"))
		{
			risk = 100;
		}
		else
		{
			risk = 60;
		}
		
		debugServerConsoleMsg(null, "Risk is computed as being "+ risk);
		
		int playerSkill = getEnhancedSkillStatisticModifier(player, "pilot_special_tactics");
		
		debugServerConsoleMsg(null, "Player skill is "+ playerSkill);
		
		int shipMods = 0;
		obj_id ship = space_transition.getContainingShip(player);
		
		if (isIdValid(ship))
		{
			shipMods = getShipsPilotCommandSkillCheckMods(ship);
		}
		
		debugServerConsoleMsg(null, "shipMods came back as "+ shipMods);
		
		float dieRoll = rand(1f, 100f);
		float moddedRoll = dieRoll + (float) (risk) + (float) (shipMods) - (float) (playerSkill);
		
		debugServerConsoleMsg(null, "moddedRoll came back as "+ moddedRoll);
		
		float goodSuccess = AVERAGE_COMMAND_SUCCESS_RATE - (AVERAGE_COMMAND_SUCCESS_RATE * 0.25f);
		
		float greatSuccess = AVERAGE_COMMAND_SUCCESS_RATE - (AVERAGE_COMMAND_SUCCESS_RATE * 0.50f);
		
		float moderateFailure = AVERAGE_COMMAND_SUCCESS_RATE + (AVERAGE_COMMAND_SUCCESS_RATE * 0.20f);
		
		float bigFailure = AVERAGE_COMMAND_SUCCESS_RATE + (AVERAGE_COMMAND_SUCCESS_RATE * 0.40f);
		
		int successLevel = 0;
		
		if (moddedRoll < greatSuccess)
		{
			successLevel = 1;
		}
		else if (moddedRoll < goodSuccess)
		{
			successLevel = 2;
		}
		else if (moddedRoll < AVERAGE_COMMAND_SUCCESS_RATE)
		{
			successLevel = 3;
		}
		else if (moddedRoll < moderateFailure)
		{
			successLevel = 4;
		}
		else if (rand(1f, 100f) < 50f)
		{
			successLevel = 5;
		}
		else
		{
			successLevel = 6;
		}
		
		debugServerConsoleMsg(null, "returning a success level of "+ successLevel);
		
		return successLevel;
	}
	
	
	public static boolean readyForBomberStrike(obj_id ship, obj_id commander) throws InterruptedException
	{
		debugServerConsoleMsg(null, "space_combat - readyForBomberStrike");
		
		if (utils.hasLocalVar(commander, "cmd.bmb.bmbrStrike"))
		{
			
			string_id strSpam = new string_id("space/space_interaction", "already_bmbr_strike");
			
			sendSystemMessage(commander, strSpam);
			return false;
		}
		
		if (isShipComponentDisabled(ship, ship_chassis_slot_type.SCST_reactor))
		{
			
			string_id strSpam = new string_id("space/space_interaction", "systems_disabled_strike_pack");
			
			sendSystemMessage(commander, strSpam);
			return false;
		}
		
		int localCalledBomberCount = 0;
		
		obj_id[] nearbyPlayers = getObservedPlayerShips(ship, true);
		
		for (int i = 0; i < nearbyPlayers.length; i++)
		{
			testAbortScript();
			if (isIdValid(nearbyPlayers[i]))
			{
				if (utils.hasLocalVar(nearbyPlayers[i], "cmd.bmb.bmbrStrike"))
				{
					localCalledBomberCount++;
				}
			}
		}
		
		if (localCalledBomberCount > 9)
		{
			
			string_id strSpam = new string_id("space/space_interaction", "bomberstrike_none_available");
			
			sendSystemMessage(commander, strSpam);
			return false;
		}
		
		obj_id target = getLookAtTarget(ship);
		
		if (!isIdValid(target))
		{
			
			string_id strSpam = new string_id("space/space_pilot_command", "bomberstrike_not_at_player");
			
			sendSystemMessage(commander, strSpam);
			return false;
		}
		
		if (space_utils.isPlayerControlledShip(target) || hasObjVar(target, "intInvincible") || space_pilot_command.isStation(target))
		{
			
			string_id strSpam = new string_id("space/space_pilot_command", "bomberstrike_not_at_player");
			
			sendSystemMessage(commander, strSpam);
			return false;
		}
		else if (pvpCanAttack(ship, target))
		{
			
			string_id strSpam = new string_id("space/space_interaction", "calling_bomberstrike");
			
			sendSystemMessage(commander, strSpam);
			return true;
		}
		
		string_id strSpam = new string_id("space/space_interaction", "bomber_not_enemy_1");
		string_id strSpam2 = new string_id("space/space_interaction", "bomber_not_enemy_2");
		
		sendSystemMessage(commander, strSpam);
		sendSystemMessage(commander, strSpam2);
		return false;
	}
	
	
	public static boolean executeLightBomberStrike(obj_id ship, obj_id commander, obj_id target, int CMD_LIGHT_STRIKE_DELAY) throws InterruptedException
	{
		debugServerConsoleMsg(null, "space_combat.executeLightBomberStrike ***** Entered Function");
		
		int skillCheck = doPilotCommandSkillCheck(commander, "level3command");
		
		if (skillCheck > 4)
		{
			string_id strSpam = new string_id("space/space_interaction", "bomberstrike_bad_skillcheck");
			
			sendSystemMessage(commander, strSpam);
			strSpam = new string_id("space/space_interaction", "bomberstrike_none_available");
			sendSystemMessage(commander, strSpam);
			initializeCommandTimer(commander, CMD_LIGHT_STRIKE_DELAY);
			return false;
		}
		
		String packageType = "light";
		
		String attackType = "many";
		
		if (!space_combat.spawnStrikeBombers(ship, commander, packageType, attackType, target))
		{
			debugServerConsoleMsg(null, "space_combat.executeLightBomberStrike ***** This script has created a FATAL ERROR! Couldn't spawn for some reason.");
			return false;
		}
		
		playClientEffectObj(commander, "clienteffect/space_command/sys_comm_imperial.cef", ship, "");
		
		space_combat.flightDroidVocalize(ship, 1);
		
		utils.setLocalVar(ship, "cmd.bmb.bmbrStrike", 1);
		
		initializeCommandTimer(commander, CMD_LIGHT_STRIKE_DELAY);
		
		String cefPlayBackHardpoint = space_combat.targetHardpointForCefPlayback(ship);
		
		playClientEffectObj(commander, "clienteffect/space_command/emergency_power_on.cef", ship, cefPlayBackHardpoint);
		
		return true;
	}
	
	
	public static boolean executeMediumBomberStrike(obj_id ship, obj_id commander, obj_id target, int CMD_LIGHT_STRIKE_DELAY) throws InterruptedException
	{
		
		int skillCheck = doPilotCommandSkillCheck(commander, "level4command");
		
		if (skillCheck > 4)
		{
			string_id strSpam = new string_id("space/space_interaction", "bomberstrike_bad_skillcheck");
			
			sendSystemMessage(commander, strSpam);
			strSpam = new string_id("space/space_interaction", "bomberstrike_none_available");
			sendSystemMessage(commander, strSpam);
			initializeCommandTimer(commander, CMD_LIGHT_STRIKE_DELAY);
			return false;
		}
		
		String packageType = "medium";
		
		String attackType = "many";
		
		if (!space_combat.spawnStrikeBombers(ship, commander, packageType, attackType, target))
		{
			debugServerConsoleMsg(null, "space_combat.executeMediumBomberStrike ***** This script has created a FATAL ERROR! Couldn't spawn for some reason.");
			return false;
		}
		
		playClientEffectObj(commander, "clienteffect/space_command/sys_comm_imperial.cef", ship, "");
		
		space_combat.flightDroidVocalize(ship, 1);
		
		utils.setLocalVar(ship, "cmd.bmb.bmbrStrike", 2);
		
		initializeCommandTimer(commander, CMD_LIGHT_STRIKE_DELAY);
		
		String cefPlayBackHardpoint = space_combat.targetHardpointForCefPlayback(ship);
		
		playClientEffectObj(commander, "clienteffect/space_command/emergency_power_on.cef", ship, cefPlayBackHardpoint);
		
		return true;
	}
	
	
	public static boolean executeHeavyBomberStrike(obj_id ship, obj_id commander, obj_id target, int CMD_LIGHT_STRIKE_DELAY) throws InterruptedException
	{
		int skillCheck = doPilotCommandSkillCheck(commander, "level5command");
		
		if (skillCheck > 4)
		{
			string_id strSpam = new string_id("space/space_interaction", "bomberstrike_bad_skillcheck");
			
			sendSystemMessage(commander, strSpam);
			strSpam = new string_id("space/space_interaction", "bomberstrike_none_available");
			sendSystemMessage(commander, strSpam);
			initializeCommandTimer(commander, CMD_LIGHT_STRIKE_DELAY);
			return false;
		}
		
		String packageType = "heavy";
		
		String attackType = "many";
		
		if (!space_combat.spawnStrikeBombers(ship, commander, packageType, attackType, target))
		{
			debugServerConsoleMsg(null, "space_combat.executeHeavyBomberStrike ***** This script has created a FATAL ERROR! Couldn't spawn for some reason.");
			return false;
		}
		
		playClientEffectObj(commander, "clienteffect/space_command/sys_comm_imperial.cef", ship, "");
		
		space_combat.flightDroidVocalize(ship, 1);
		
		utils.setLocalVar(ship, "cmd.bmb.bmbrStrike", 3);
		
		initializeCommandTimer(commander, CMD_LIGHT_STRIKE_DELAY);
		
		String cefPlayBackHardpoint = space_combat.targetHardpointForCefPlayback(ship);
		
		playClientEffectObj(commander, "clienteffect/space_command/emergency_power_on.cef", ship, cefPlayBackHardpoint);
		
		return true;
	}
	
	
	public static transform playerCommandSpawnerLocGetter(obj_id ship, boolean flip) throws InterruptedException
	{
		debugServerConsoleMsg(null, "PLAYERcOMMANDsPAWNERlOCgETTER");
		
		transform loc = getTransform_o2w(ship);
		
		float dist = rand(600.f, 700.f);
		vector n = loc.getLocalFrameK_p().normalize().multiply(dist);
		
		loc = loc.move_p(n);
		
		if (flip == true)
		{
			loc = loc.yaw_l(3.14f);
		}
		
		vector vi = loc.getLocalFrameI_p().normalize().multiply(rand(-150.f, 150.f));
		vector vj = loc.getLocalFrameJ_p().normalize().multiply(rand(-150.f, 150.f));
		vector vd = vi.add(vj);
		
		loc = loc.move_p(vd);
		
		return loc;
	}
	
	
	public static boolean spawnStrikeBombers(obj_id ship, obj_id commander, String packageType, String attack_type, obj_id target) throws InterruptedException
	{
		debugServerConsoleMsg(null, "space_combat.spawnStrikeBombers ***** Entered Function");
		
		if (!isIdValid(ship) || !isIdValid(commander) || !isIdValid(target))
		{
			return false;
		}
		
		transform loc = playerCommandSpawnerLocGetter(ship, true);
		
		String targetDifficultyTier = "_tier3";
		
		int targetSquad = ship_ai.unitGetSquadId(target);
		int targetSquadSize = ship_ai.squadGetSize(targetSquad);
		
		targetDifficultyTier = "_tier"+ space_pilot_command.targetTierDetect(target);
		String strikePackageType = "squad_plyr_cmd_bmbr_basic"+ targetDifficultyTier;
		
		if (packageType.equals("medium"))
		{
			strikePackageType = "squad_plyr_cmd_bmbr_advanced"+ targetDifficultyTier;
		}
		else if (packageType.equals("heavy"))
		{
			strikePackageType = "squad_plyr_cmd_bmbr_extreme"+ targetDifficultyTier;
		}
		
		Vector objStrikePack = space_create.createSquadHyperspace(null, strikePackageType, loc, 20.0f, null);
		
		if (!isIdValid(((obj_id)(objStrikePack.get(0)))))
		{
			return false;
		}
		
		int bomberSquadId = ship_ai.unitGetSquadId(((obj_id)(objStrikePack.get(0))));
		
		obj_id[] squaddyList = null;
		if (ship_ai.isSquadIdValid(bomberSquadId))
		{
			squaddyList = ship_ai.squadGetUnitList(bomberSquadId);
		}
		else
		{
			return false;
		}
		
		if (objStrikePack.size() > 1)
		{
			ship_ai.squadSetFormationSpacing(bomberSquadId, 3.0f);
		}
		
		for (int i = 0; i < squaddyList.length; i++)
		{
			testAbortScript();
			setObjVar(squaddyList[i], "commanderPlayer", commander);
		}
		
		if (utils.hasLocalVar(commander, "cmd.bmb.bmbrSquadId"))
		{
			utils.removeLocalVar(commander, "cmd.bmb.bmbrSquadId");
		}
		utils.setLocalVar(commander, "cmd.bmb.bmbrSquadId", bomberSquadId);
		
		if (utils.hasLocalVar(commander, "cmd.bmb.initialStrikePackSize"))
		{
			utils.removeLocalVar(commander, "cmd.bmb.initialStrikePackSize");
		}
		utils.setLocalVar(commander, "cmd.bmb.initialStrikePackSize", objStrikePack.size());
		utils.setLocalVar(commander, "cmd.bmb.currentStrikePackSize", objStrikePack.size());
		
		transform targetLoc = getTransform_o2w(target);
		
		transform[]loiterPath = ship_ai.createPatrolPathLoiter(targetLoc, 100.0f, 100.0f);
		ship_ai.squadPatrol(bomberSquadId, loiterPath);
		
		if (attack_type.equals("single"))
		{
			ship_ai.squadAddTarget(bomberSquadId, target);
		}
		else
		{
			ship_ai.squadAddTarget(bomberSquadId, targetSquad);
		}
		
		initializeTargetTracking(commander, bomberSquadId, targetSquad);
		
		string_id sid = new string_id("space/space_interaction", "bombers_arriving");
		prose_package pp = new prose_package();
		
		pp.stringId = sid;
		commPlayers(squaddyList[0], null, null, 6.0f, commander, pp);
		
		return true;
	}
	
	
	public static boolean setupStrikePackageEscort(int bomberSquadId, String packageType, obj_id commander, String targetDifficultyTier) throws InterruptedException
	{
		debugServerConsoleMsg(null, "SETUPsTRIKEpACKAGEeSCORT");
		
		obj_id[] bomberSquaddyList = null;
		if (ship_ai.isSquadIdValid(bomberSquadId))
		{
			bomberSquaddyList = ship_ai.squadGetUnitList(bomberSquadId);
		}
		else
		{
			return false;
		}
		
		int numBombers = ship_ai.squadGetSize(bomberSquadId);
		
		int successLevel = doPilotCommandSkillCheck(commander, "level3command");
		
		String escortPackageType = "";
		
		debugServerConsoleMsg(null, "============================================");
		debugServerConsoleMsg(null, "SPACE_COMBAT.SETUPsTRIKEpACKAGEeSCORT = successLevel came back as: "+ successLevel);
		
		switch (successLevel)
		{
			case 6:
			escortPackageType = "squad_plyr_cmd_bmbr_escort_basic"+ targetDifficultyTier;
			break;
			case 5:
			escortPackageType = "squad_plyr_cmd_bmbr_escort_basic"+ targetDifficultyTier;
			break;
			case 4:
			escortPackageType = "squad_plyr_cmd_bmbr_escort_basic"+ targetDifficultyTier;
			break;
			case 3:
			escortPackageType = "squad_plyr_cmd_bmbr_escort_interceptor"+ targetDifficultyTier;
			break;
			case 2:
			escortPackageType = "squad_plyr_cmd_bmbr_escort_interceptor"+ targetDifficultyTier;
			break;
			case 1:
			escortPackageType = "squad_plyr_cmd_bmbr_escort_advanced"+ targetDifficultyTier;
			break;
			case 0:
			escortPackageType = "squad_plyr_cmd_bmbr_escort_advanced"+ targetDifficultyTier;
			break;
		}
		
		transform loc = playerCommandSpawnerLocGetter(bomberSquaddyList[0], false);
		
		int totalEscorts = 0;
		
		for (int i = 0; i < bomberSquaddyList.length; i++)
		{
			testAbortScript();
			Vector objStrikeEscortPack = space_create.createSquadHyperspace(null, escortPackageType, loc, 20.0f, null);
			
			int escortSquadId = ship_ai.unitGetSquadId(((obj_id)(objStrikeEscortPack.get(0))));
			
			obj_id[] escortSquaddyList = null;
			if (ship_ai.isSquadIdValid(escortSquadId))
			{
				escortSquaddyList = ship_ai.squadGetUnitList(escortSquadId);
			}
			else
			{
				return false;
			}
			
			totalEscorts += objStrikeEscortPack.size();
			
			utils.setLocalVar(bomberSquaddyList[i], "escortSquadId", escortSquadId);
			utils.setLocalVar(bomberSquaddyList[i], "crrntEscrtSqdSz", escortSquaddyList.length);
			
			for (int j = 0; j < escortSquaddyList.length; j++)
			{
				testAbortScript();
				setObjVar(escortSquaddyList[j], "commanderPlayer", commander);
			}
			
			if (utils.hasLocalVar(commander, "cmd.bmb.initEscrtPckSize"))
			{
				utils.removeLocalVar(commander, "cmd.bmb.initEscrtPckSize");
			}
			
			utils.setLocalVar(commander, "cmd.bmb.initEscrtPckSize", totalEscorts);
			
			ship_ai.squadFollow(escortSquadId, bomberSquaddyList[i], new vector(0.0f, 0.0f, -1.0f), 2.0f);
			ship_ai.squadSetGuardTarget(escortSquadId, bomberSquadId);
			
		}
		
		return true;
	}
	
	
	public static void strikePackageEvac(obj_id commander, int bomberSquadId, int fighterSquadId) throws InterruptedException
	{
		debugServerConsoleMsg(null, "STRIKEpACKAGEeVAC");
		
		utils.removeLocalVar(commander, "cmd.bmb");
		
		obj_id ship = space_transition.getContainingShip(commander);
		
		obj_id strikeLeader = null;
		int strikeSurvivorSquad = -1;
		
		int currentStrikePackageSize = utils.getIntLocalVar(commander, "cmd.bmb.currentStrikePackSize");
		
		if (bomberSquadId > 0 && fighterSquadId < 0)
		{
			obj_id[] bomberSquaddyList = null;
			if (ship_ai.isSquadIdValid(bomberSquadId))
			{
				bomberSquaddyList = ship_ai.squadGetUnitList(bomberSquadId);
			}
			else
			{
				return;
			}
			
			if (hasObjVar(bomberSquaddyList[0], "evacuate"))
			{
				return;
			}
			
			for (int i = 0; i < bomberSquaddyList.length; i++)
			{
				testAbortScript();
				setObjVar(bomberSquaddyList[i], "evacuate", 1);
				
			}
			strikeLeader = bomberSquaddyList[0];
			strikeSurvivorSquad = bomberSquadId;
		}
		
		obj_id squadTarget = ship_ai.unitGetPrimaryAttackTarget(strikeLeader);
		
		if (!isIdValid(ship))
		{
			ship = strikeLeader;
		}
		
		transform loc = space_combat.playerCommandSpawnerLocGetter(ship, true);
		
		ship_ai.squadSetAttackOrders(strikeSurvivorSquad, ship_ai.ATTACK_ORDERS_HOLD_FIRE);
		ship_ai.squadMoveTo(strikeSurvivorSquad, loc);
		
		string_id sid = new string_id("space/space_interaction", "strikeforce_withdrawing");
		prose_package pp = new prose_package();
		
		pp.stringId = sid;
		commPlayers(strikeLeader, null, null, 6.0f, commander, pp);
		
		if (utils.hasLocalVar(commander, "cmd.bmb.targetSquadId") && !utils.hasLocalVar(commander, "cmd.bmb.targetSquadCrrntSz"))
		{
			int targetSquadId = utils.getIntLocalVar(commander, "cmd.bmb.targetSquadId");
			
			obj_id[] targetSquaddyList = null;
			if (ship_ai.isSquadIdValid(targetSquadId))
			{
				targetSquaddyList = ship_ai.squadGetUnitList(targetSquadId);
			}
			else
			{
				return;
			}
			
			for (int i = 0; i < targetSquaddyList.length; i++)
			{
				testAbortScript();
				if (hasScript(targetSquaddyList[i], "space.command.player_cmd_bomber_strike_target"))
				{
					detachScript(targetSquaddyList[i], "space.command.player_cmd_bomber_strike_target");
				}
				if (hasObjVar(targetSquaddyList[i], "targetedByPlayerObjId"))
				{
					removeObjVar(targetSquaddyList[i], "targetedByPlayerObjId");
				}
				if (hasObjVar(targetSquaddyList[i], "damageMultiplier"))
				{
					removeObjVar(targetSquaddyList[i], "damageMultiplier");
				}
			}
		}
		
		return;
	}
	
	
	public static void initializeTargetTracking(obj_id commander, int bomberSquadId, int targetSquadId) throws InterruptedException
	{
		if (utils.hasLocalVar(commander, "cmd.bmb.targetSquadId"))
		{
			utils.removeLocalVar(commander, "cmd.bmb.targetSquadId");
		}
		
		obj_id[] targetSquaddyList = null;
		if (ship_ai.isSquadIdValid(targetSquadId))
		{
			targetSquaddyList = ship_ai.squadGetUnitList(targetSquadId);
		}
		else
		{
			return;
		}
		
		utils.setLocalVar(commander, "cmd.bmb.targetSquadId", targetSquadId);
		utils.setLocalVar(commander, "cmd.bmb.targetSquadCrrntSz", targetSquaddyList.length);
		
		for (int i = 0; i < targetSquaddyList.length; i++)
		{
			testAbortScript();
			attachScript(targetSquaddyList[i], "space.command.player_cmd_bomber_strike_target");
			setObjVar(targetSquaddyList[i], "targetedByPlayerObjId", commander);
			setObjVar(targetSquaddyList[i], "damageMultiplier", NPC_DAMAGE_MULTIPLIER);
		}
		
		return;
	}
	
	
	public static void strikeBomberCleanup(obj_id commander) throws InterruptedException
	{
		if (utils.hasLocalVar(commander, "cmd.bmb.bmbrSquadId"))
		{
			int bomberSquadId = utils.getIntLocalVar(commander, "cmd.bmb.bmbrSquadId");
			
			if (ship_ai.isSquadIdValid(bomberSquadId))
			{
				strikePackageEvac(commander, bomberSquadId, -1);
			}
		}
		
		return;
	}
	
	
	public static boolean readyForPirateRaid(obj_id ship, obj_id commander) throws InterruptedException
	{
		debugServerConsoleMsg(null, "SPACE_COMBAT.readyForPirateRaid");
		
		if (utils.hasLocalVar(ship, "cmd.pirate.goRaid"))
		{
			
			string_id strSpam = new string_id("space/space_interaction", "already_pirate_raid");
			
			sendSystemMessage(commander, strSpam);
			return false;
		}
		
		if (isShipComponentDisabled(ship, ship_chassis_slot_type.SCST_reactor))
		{
			
			string_id strSpam = new string_id("space/space_interaction", "systems_disabled_pirate_raid");
			
			sendSystemMessage(commander, strSpam);
			return false;
		}
		
		int localCalledPirateCount = 0;
		
		obj_id[] nearbyPlayers = getObservedPlayerShips(ship, true);
		
		for (int i = 0; i < nearbyPlayers.length; i++)
		{
			testAbortScript();
			if (isIdValid(nearbyPlayers[i]))
			{
				if (utils.hasLocalVar(nearbyPlayers[i], "cmd.pirate.goRaid"))
				{
					localCalledPirateCount++;
				}
			}
		}
		
		if (localCalledPirateCount > 4)
		{
			
			string_id strSpam = new string_id("space/space_pilot_command", "pirateraid_none_available");
			
			sendSystemMessage(commander, strSpam);
			return false;
		}
		
		obj_id target = getLookAtTarget(ship);
		
		if (isIdValid(target))
		{
			if (!space_pilot_command.isStation(target))
			{
				if (space_utils.isShip(target))
				{
					if (pvpCanAttack(ship, target))
					{
						
						string_id strSpam = new string_id("space/space_interaction", "calling_pirate_raid");
						
						sendSystemMessage(commander, strSpam);
						return true;
					}
				}
			}
		}
		else if (!isIdValid(target))
		{
			
			string_id strSpam = new string_id("space/space_interaction", "calling_pirate_raid");
			
			sendSystemMessage(commander, strSpam);
			return true;
		}
		
		string_id strSpam = new string_id("space/space_interaction", "pirate_raid_not_enemy");
		
		sendSystemMessage(commander, strSpam);
		return false;
	}
	
	
	public static boolean executePirateRaidOne(obj_id ship, obj_id commander, obj_id target, int CMD_PIRATE_LURE_DELAY) throws InterruptedException
	{
		debugServerConsoleMsg(null, "SPACE_COMBAT.executePirateRaidOne");
		
		String packageType = "light";
		
		String attackType = "many";
		String level = "level1";
		
		if (!space_combat.spawnPirateRaiders(ship, commander, level, attackType, target))
		{
			string_id strSpam = new string_id("space/space_interaction", "pirate_raid_systems_not_ready");
			
			sendSystemMessage(commander, strSpam);
			debugServerConsoleMsg(null, "space_combat.executePirateRaidone ***** script has created a FATAL ERROR! Could not spawn for some reason");
			return false;
		}
		
		playClientEffectObj(commander, "clienteffect/space_command/sys_comm_generic.cef", ship, "");
		
		space_combat.flightDroidVocalize(ship, 1);
		
		utils.setLocalVar(ship, "cmd.pirate.goRaid", 1);
		
		initializeCommandTimer(commander, CMD_PIRATE_LURE_DELAY);
		
		String cefPlayBackHardpoint = space_combat.targetHardpointForCefPlayback(ship);
		
		playClientEffectObj(commander, "clienteffect/space_command/emergency_power_on.cef", ship, cefPlayBackHardpoint);
		
		return true;
	}
	
	
	public static boolean executePirateRaidTwo(obj_id ship, obj_id commander, obj_id target, int CMD_PIRATE_LURE_DELAY) throws InterruptedException
	{
		debugServerConsoleMsg(null, "SPACE_COMBAT.executePirateRaidOne");
		
		String packageType = "light";
		
		String attackType = "many";
		String level = "level2";
		
		if (!space_combat.spawnPirateRaiders(ship, commander, level, attackType, target))
		{
			string_id strSpam = new string_id("space/space_interaction", "pirate_raid_systems_not_ready");
			
			sendSystemMessage(commander, strSpam);
			debugServerConsoleMsg(null, "space_combat.executePirateRaidTwo ***** script has created a FATAL ERROR! Could not spawn for some reason");
			return false;
		}
		
		playClientEffectObj(commander, "clienteffect/space_command/sys_comm_generic.cef", ship, "");
		
		space_combat.flightDroidVocalize(ship, 1);
		
		utils.setLocalVar(ship, "cmd.pirate.goRaid", 2);
		
		initializeCommandTimer(commander, CMD_PIRATE_LURE_DELAY);
		
		String cefPlayBackHardpoint = space_combat.targetHardpointForCefPlayback(ship);
		
		playClientEffectObj(commander, "clienteffect/space_command/emergency_power_on.cef", ship, cefPlayBackHardpoint);
		
		return true;
	}
	
	
	public static boolean spawnPirateRaiders(obj_id ship, obj_id commander, String level, String attack_type, obj_id target) throws InterruptedException
	{
		debugServerConsoleMsg(null, "SPACE_COMBAT.spawnPirateRaiders");
		
		if (!isIdValid(ship) || !isIdValid(commander))
		{
			return false;
		}
		
		if (!isIdValid(target))
		{
			debugServerConsoleMsg(null, "SPACE_COMBAT.spawnPirateRaiders -- Skipping spawning stuff. Target was invalid and wasn't null");
			return false;
		}
		
		transform loc = playerCommandSpawnerLocGetter(ship, true);
		
		int pirateForceMaxSize = 1;
		String pirateForceGear = PIRATE_EQUIPMENT_D;
		int successLevel = 4;
		
		if (level.equals("level1"))
		{
			successLevel = doPilotCommandSkillCheck(commander, "level2command");
			
			debugServerConsoleMsg(null, "============================================");
			debugServerConsoleMsg(null, "SPACE_COMBAT.spawnPirateRaiders = successLevel came back as: "+ successLevel);
			
			switch (successLevel)
			{
				case 6:
				pirateForceMaxSize = 3;
				pirateForceGear = PIRATE_EQUIPMENT_B;
				break;
				case 5:
				pirateForceMaxSize = 0;
				break;
				case 4:
				pirateForceMaxSize = 2;
				pirateForceGear = PIRATE_EQUIPMENT_D;
				break;
				case 3:
				pirateForceMaxSize = 4;
				pirateForceGear = PIRATE_EQUIPMENT_C;
				break;
				case 2:
				pirateForceMaxSize = 4;
				pirateForceGear = PIRATE_EQUIPMENT_C;
				break;
				case 1:
				pirateForceMaxSize = 4;
				pirateForceGear = PIRATE_EQUIPMENT_B;
				break;
				case 0:
				pirateForceMaxSize = 6;
				pirateForceGear = PIRATE_EQUIPMENT_B;
				break;
			}
			
			if (successLevel == 5)
			{
				
				string_id strSpam = new string_id("space/space_interaction", "none_around_pirate_raid");
				
				sendSystemMessage(commander, strSpam);
				return false;
			}
		}
		else if (level.equals("level2"))
		{
			successLevel = doPilotCommandSkillCheck(commander, "level4command");
			
			debugServerConsoleMsg(null, "============================================");
			debugServerConsoleMsg(null, "SPACE_COMBAT.spawnPirateRaiders = successLevel came back as: "+ successLevel);
			
			switch (successLevel)
			{
				case 6:
				pirateForceMaxSize = 3;
				pirateForceGear = PIRATE_EQUIPMENT_A;
				break;
				case 5:
				pirateForceMaxSize = 0;
				break;
				case 4:
				pirateForceMaxSize = 2;
				pirateForceGear = PIRATE_EQUIPMENT_C;
				break;
				case 3:
				pirateForceMaxSize = 4;
				pirateForceGear = PIRATE_EQUIPMENT_B;
				break;
				case 2:
				pirateForceMaxSize = 4;
				pirateForceGear = PIRATE_EQUIPMENT_A;
				break;
				case 1:
				pirateForceMaxSize = 4;
				pirateForceGear = PIRATE_EQUIPMENT_A;
				break;
				case 0:
				pirateForceMaxSize = 6;
				pirateForceGear = PIRATE_EQUIPMENT_AA;
				break;
			}
			
			if (successLevel == 5)
			{
				
				string_id strSpam = new string_id("space/space_interaction", "none_around_pirate_raid");
				
				sendSystemMessage(commander, strSpam);
				return false;
			}
			else if (successLevel < 4)
			{
				ship_ai.unitSetAutoAggroImmuneTime(ship, 60.0f);
			}
		}
		
		String[] possibleShips = dataTableGetStringColumnNoDefaults(PIRATE_EQUIPMENT_DATATABLE, pirateForceGear);
		
		String leadPirate = possibleShips[(int) (rand(0, possibleShips.length - 1))];
		
		debugServerConsoleMsg(null, "SPACE_COMBAT.spawnPirateRaiders ------------ leadPirate chosen was: "+ leadPirate);
		
		obj_id pirateLeader = space_create.createShipHyperspace(leadPirate, loc, null);
		
		if (!isIdValid(pirateLeader))
		{
			return false;
		}
		
		int pirateSquadId = ship_ai.unitGetSquadId(pirateLeader);
		
		int pirateForceSize = (int) (rand(1, pirateForceMaxSize - 1));
		
		for (int i = 0; i < pirateForceSize; i++)
		{
			testAbortScript();
			String spawnShip = possibleShips[(int) (rand(0, possibleShips.length - 1))];
			
			debugServerConsoleMsg(null, "SPACE_COMBAT.spawnPirateRaiders ------------ secondary pirate chosen was: "+ spawnShip);
			loc = playerCommandSpawnerLocGetter(ship, true);
			obj_id currentPirate = space_create.createShipHyperspace(spawnShip, loc, null);
			
			ship_ai.unitSetSquadId(currentPirate, pirateSquadId);
		}
		
		obj_id[] squaddyList = null;
		
		if (ship_ai.isSquadIdValid(pirateSquadId))
		{
			squaddyList = ship_ai.squadGetUnitList(pirateSquadId);
		}
		else
		{
			return false;
		}
		
		pirateForceSize = ship_ai.squadGetSize(pirateSquadId);
		
		if (squaddyList.length > 1)
		{
			ship_ai.squadSetFormationSpacing(pirateSquadId, 3.0f);
		}
		
		for (int i = 0; i < squaddyList.length; i++)
		{
			testAbortScript();
			setObjVar(squaddyList[i], "commanderPlayer", commander);
			attachScript(squaddyList[i], "space.command.player_cmd_pirate");
		}
		
		if (utils.hasLocalVar(commander, "cmd.pirate.squadId"))
		{
			utils.removeLocalVar(commander, "cmd.pirate.squadId");
		}
		
		utils.setLocalVar(commander, "cmd.pirate.squadId", pirateSquadId);
		
		if (utils.hasLocalVar(commander, "cmd.pirate.initialSize"))
		{
			utils.removeLocalVar(commander, "cmd.pirate.initialSize");
		}
		utils.setLocalVar(commander, "cmd.pirate.initialSize", pirateForceSize);
		
		if (isIdValid(target))
		{
			transform targetLoc = getTransform_o2w(target);
			
			transform[]loiterPath = ship_ai.createPatrolPathLoiter(targetLoc, 100.0f, 100.0f);
			ship_ai.squadPatrol(pirateSquadId, loiterPath);
		}
		else
		{
			transform targetLoc = getTransform_o2w(ship);
			
			transform[]loiterPath = ship_ai.createPatrolPathLoiter(targetLoc, 100.0f, 100.0f);
			ship_ai.squadPatrol(pirateSquadId, loiterPath);
		}
		
		dictionary params = new dictionary();
		
		for (int j = 0; j < squaddyList.length; j++)
		{
			testAbortScript();
			obj_id thisAttacker = squaddyList[j];
			
			params.put("loopsleft", 2);
			params.put("targetship", ship);
			messageTo(squaddyList[j], "iAmSpoofed", params, 2.0f, false);
		}
		
		if (successLevel == 6)
		{
			ship_ai.squadAddTarget(pirateSquadId, ship);
		}
		else
		{
			if (isIdValid(target))
			{
				if (!space_utils.isPlayerControlledShip(target))
				{
					int targetSquad = ship_ai.unitGetSquadId(target);
					int targetSquadSize = ship_ai.squadGetSize(targetSquad);
					
					if (targetSquadSize > 1)
					{
						obj_id[] targetSquaddyList = null;
						
						if (ship_ai.isSquadIdValid(targetSquad))
						{
							targetSquaddyList = ship_ai.squadGetUnitList(targetSquad);
						}
						else
						{
							return true;
						}
						
						for (int k = 0; k < targetSquaddyList.length; k++)
						{
							testAbortScript();
							setObjVar(targetSquaddyList[k], "damageMultiplier", NPC_DAMAGE_MULTIPLIER);
						}
						
						ship_ai.squadSetPrimaryTarget(pirateSquadId, targetSquad);
					}
					else
					{
						if (ship_ai.isSquadIdValid(targetSquad))
						{
							setObjVar(target, "damageMultiplier", NPC_DAMAGE_MULTIPLIER);
							ship_ai.squadSetPrimaryTarget(pirateSquadId, target);
						}
						else
						{
							return true;
						}
					}
				}
				else
				{
					
					ship_ai.squadSetPrimaryTarget(pirateSquadId, target);
					
				}
			}
		}
		
		return true;
	}
	
	
	public static void piratePackageEvac(obj_id commander, int pirateSquadId) throws InterruptedException
	{
		debugServerConsoleMsg(null, "PIRATEpACKAGEeVAC");
		
		utils.removeLocalVar(commander, "cmd.pirate");
		
		obj_id ship = space_transition.getContainingShip(commander);
		
		obj_id[] pirateSquaddyList = null;
		
		if (ship_ai.isSquadIdValid(pirateSquadId))
		{
			pirateSquaddyList = ship_ai.squadGetUnitList(pirateSquadId);
		}
		else
		{
			return;
		}
		
		if (hasObjVar(pirateSquaddyList[0], "evacuate"))
		{
			return;
		}
		
		for (int i = 0; i < pirateSquaddyList.length; i++)
		{
			testAbortScript();
			setObjVar(pirateSquaddyList[i], "evacuate", 1);
		}
		
		obj_id pirateLeader = pirateSquaddyList[0];
		obj_id squadTarget = ship_ai.unitGetPrimaryAttackTarget(pirateLeader);
		
		if (!isIdValid(ship))
		{
			ship = pirateLeader;
		}
		
		transform loc = playerCommandSpawnerLocGetter(ship, true);
		
		ship_ai.squadSetAttackOrders(pirateSquadId, ship_ai.ATTACK_ORDERS_HOLD_FIRE);
		ship_ai.squadMoveTo(pirateSquadId, loc);
		
		if (utils.hasLocalVar(commander, "cmd.pirate.squadId"))
		{
			int targetSquadId = utils.getIntLocalVar(commander, "cmd.pirate.squadId");
			
			obj_id[] targetSquaddyList = null;
			
			if (ship_ai.isSquadIdValid(targetSquadId))
			{
				targetSquaddyList = ship_ai.squadGetUnitList(targetSquadId);
			}
			else
			{
				return;
			}
			
			for (int i = 0; i < targetSquaddyList.length; i++)
			{
				testAbortScript();
				if (hasObjVar(targetSquaddyList[i], "damageMultiplier"))
				{
					removeObjVar(targetSquaddyList[i], "damageMultiplier");
				}
			}
		}
		
		return;
	}
	
	
	public static float getShipWeaponDamage(obj_id objAttacker, obj_id objDefender, int intWeaponSlot) throws InterruptedException
	{
		final float NPC_NPC_DAMAGE_MULTIPLIER_DEEP_SPACE = 10.0f;
		final float NPC_NPC_DAMAGE_MULTIPLIER = 1.0f;
		final float PVP_MULTIPLIER = .5f;
		
		float fltMinDamage = getShipWeaponDamageMinimum(objAttacker, intWeaponSlot);
		float fltMaxDamage = getShipWeaponDamageMaximum(objAttacker, intWeaponSlot);
		
		float fltGeneralEfficiency = getShipComponentEfficiencyGeneral(objAttacker, intWeaponSlot);
		
		fltMinDamage = fltMinDamage * fltGeneralEfficiency;
		fltMaxDamage = fltMaxDamage * fltGeneralEfficiency;
		float fltDamage = rand(fltMinDamage, fltMaxDamage);
		
		if ((!space_utils.isPlayerControlledShip(objAttacker) && (!space_utils.isPlayerControlledShip(objDefender))))
		{
			location locTest = getLocation(objAttacker);
			
			if (locTest.area.equals("space_heavy"))
			{
				fltDamage = fltDamage * NPC_NPC_DAMAGE_MULTIPLIER_DEEP_SPACE;
			}
			else
			{
				if (hasObjVar(objDefender, "damageMultiplier"))
				{
					int intTest = getIntObjVar(objDefender, "damageMultiplier");
					float fltMultiplier = (float) (intTest);
					
					if (fltMultiplier <= 0)
					{
						fltMultiplier = 1.0f;
					}
					
					fltDamage = fltDamage * fltMultiplier;
				}
				else
				{
					fltDamage = fltDamage * NPC_NPC_DAMAGE_MULTIPLIER;
				}
			}
		}
		else if (space_utils.isPlayerControlledShip(objAttacker) && (space_utils.isPlayerControlledShip(objDefender)))
		{
			fltDamage = fltDamage * PVP_MULTIPLIER;
		}
		
		float skillMod = (float)getEnhancedSkillStatisticModifierUncapped(objAttacker, "ship_weapon_damage")/100;
		fltDamage = fltDamage * (1.0f + skillMod);
		
		return fltDamage;
		
	}
	
	
	public static float doShieldDamage(obj_id objAttacker, obj_id objDefender, int intWeaponSlot, float fltDamage, int intSide) throws InterruptedException
	{
		
		fltDamage = damageShield(objAttacker, objDefender, intWeaponSlot, fltDamage, intSide);
		return fltDamage;
	}
	
	
	public static float doArmorDamage(obj_id objAttacker, obj_id objDefender, int intWeaponSlot, float fltDamage, int intSide) throws InterruptedException
	{
		fltDamage = damageArmor(objAttacker, objDefender, intWeaponSlot, fltDamage, intSide);
		
		return fltDamage;
	}
	
	
	public static float doChassisDamage(obj_id objAttacker, obj_id objDefender, int intWeaponSlot, float fltDamage) throws InterruptedException
	{
		
		float fltChassisHitPoints = getShipCurrentChassisHitPoints(objDefender);
		
		float fltMaximumChassisHitPoints = getShipMaximumChassisHitPoints(objDefender);
		
		if ((fltChassisHitPoints - fltDamage) < (fltMaximumChassisHitPoints / 2) && fltChassisHitPoints > (fltMaximumChassisHitPoints / 2))
		{
			
			dictionary dctParams = new dictionary();
			
			space_utils.notifyObject(objDefender, "OnHullNearlyDestroyed", dctParams);
		}
		
		fltChassisHitPoints = fltChassisHitPoints - fltDamage;
		
		doInteriorDamageNotification(objDefender, space_combat.CHASSIS, fltMaximumChassisHitPoints, fltChassisHitPoints);
		
		if ((fltChassisHitPoints + fltDamage) > 0.0f && fltMaximumChassisHitPoints > 0.0f)
		{
			location locDefender = getLocation(objDefender);
			location locAttacker = isIdValid(objAttacker) ? getLocation(objAttacker) : locDefender;
			
			vector up_w = new vector(locAttacker.x - locDefender.x, locAttacker.y - locDefender.y, locAttacker.z - locDefender.z);
			
			float integrity = fltChassisHitPoints / fltMaximumChassisHitPoints;
			float previousIntegrity = (fltChassisHitPoints + fltDamage) / fltMaximumChassisHitPoints;
			
			if (Math.abs(integrity - previousIntegrity) > 0.01f)
			{
				notifyShipHit(objDefender, up_w, vector.zero, ship_hit_type.HT_chassis, integrity, previousIntegrity);
			}
		}
		
		if (fltChassisHitPoints <= 0)
		{
			space_combat.registerDamageDoneToShip(objAttacker, objDefender, fltDamage + fltChassisHitPoints);
			
			location locTest = getLocation(objDefender);
			
			setShipCurrentChassisHitPoints(objDefender, 0);
			doChassisDamageSpam(objAttacker, objDefender, fltDamage, fltChassisHitPoints);
			return 10;
		}
		else
		{
			space_combat.registerDamageDoneToShip(objAttacker, objDefender, fltDamage);
			
			if (fltChassisHitPoints > fltMaximumChassisHitPoints)
			{
				fltChassisHitPoints = fltMaximumChassisHitPoints;
			}
			
			doChassisDamageSpam(objAttacker, objDefender, fltDamage, fltChassisHitPoints);
			setShipCurrentChassisHitPoints(objDefender, fltChassisHitPoints);
			return 0;
		}
		
	}
	
	
	public static float getShieldHitPoints(obj_id objDefender, int intSide) throws InterruptedException
	{
		if (isShipSlotInstalled(objDefender, ship_chassis_slot_type.SCST_shield_0))
		{
			if (intSide == space_combat.FRONT)
			{
				return getShipShieldHitpointsFrontCurrent(objDefender);
			}
			else if (intSide == space_combat.BACK)
			{
				return getShipShieldHitpointsBackCurrent(objDefender);
			}
		}
		
		return 0;
	}
	
	
	public static float getShieldHitPointsMaximum(obj_id objDefender, int intSide) throws InterruptedException
	{
		if (isShipSlotInstalled(objDefender, ship_chassis_slot_type.SCST_shield_0))
		{
			if (intSide == space_combat.FRONT)
			{
				return getShipShieldHitpointsFrontMaximum(objDefender);
			}
			else if (intSide == space_combat.BACK)
			{
				return getShipShieldHitpointsBackMaximum(objDefender);
			}
		}
		
		return 0;
	}
	
	
	public static float getArmorHitPoints(obj_id objDefender, int intSide) throws InterruptedException
	{
		if (intSide == space_combat.FRONT)
		{
			
			int intSlot = ship_chassis_slot_type.SCST_armor_0;
			
			if (isShipSlotInstalled(objDefender, intSlot))
			{
				return getShipComponentArmorHitpointsCurrent(objDefender, intSlot);
			}
		}
		else if (intSide == space_combat.BACK)
		{
			int intSlot = ship_chassis_slot_type.SCST_armor_1;
			
			if (isShipSlotInstalled(objDefender, intSlot))
			{
				return getShipComponentArmorHitpointsCurrent(objDefender, intSlot);
			}
		}
		
		return 0;
	}
	
	
	public static float getArmorHitPointsMaximum(obj_id objDefender, int intSide) throws InterruptedException
	{
		if (intSide == space_combat.FRONT)
		{
			
			int intSlot = ship_chassis_slot_type.SCST_armor_0;
			
			if (isShipSlotInstalled(objDefender, intSlot))
			{
				return getShipComponentArmorHitpointsMaximum(objDefender, intSlot);
			}
			
		}
		else if (intSide == space_combat.BACK)
		{
			int intSlot = ship_chassis_slot_type.SCST_armor_1;
			
			if (isShipSlotInstalled(objDefender, intSlot))
			{
				return getShipComponentArmorHitpointsMaximum(objDefender, intSlot);
			}
		}
		
		return 0;
	}
	
	
	public static void setShieldHitPoints(obj_id objDefender, int intSide, float fltHitPoints) throws InterruptedException
	{
		
		if (isShipSlotInstalled(objDefender, ship_chassis_slot_type.SCST_shield_0))
		{
			
			if (intSide == space_combat.FRONT)
			{
				setShipShieldHitpointsFrontCurrent(objDefender, fltHitPoints);
				
			}
			else if (intSide == space_combat.BACK)
			{
				setShipShieldHitpointsBackCurrent(objDefender, fltHitPoints);
			}
			else
			{
				
			}
		}
	}
	
	public static void setArmorHitPoints(obj_id objDefender, int intSide, float fltHitPoints) throws InterruptedException
	{
		if (intSide == space_combat.FRONT)
		{
			int intSlot = ship_chassis_slot_type.SCST_armor_0;
			
			if (isShipSlotInstalled(objDefender, intSlot))
			{
				setShipComponentArmorHitpointsCurrent(objDefender, intSlot, fltHitPoints);
			}
		}
		else if (intSide == space_combat.BACK)
		{
			int intSlot = ship_chassis_slot_type.SCST_armor_1;
			
			if (isShipSlotInstalled(objDefender, intSlot))
			{
				setShipComponentArmorHitpointsCurrent(objDefender, intSlot, fltHitPoints);
			}
		}
		else
		{
			
		}
	}
	
	
	public static float damageShield(obj_id objAttacker, obj_id objDefender, int intWeaponSlot, float fltDamage, int intSide) throws InterruptedException
	{
		float fltShieldHitPoints = getShieldHitPoints(objDefender, intSide);
		
		if (fltShieldHitPoints > 0)
		{
			if (isIdValid(objAttacker))
			{
				if (isShipSlotInstalled(objAttacker, intWeaponSlot))
				{
					float fltWeaponShieldRating = getShipWeaponEffectivenessShields(objAttacker, intWeaponSlot);
					
					fltDamage = fltDamage * fltWeaponShieldRating;
				}
			}
			
			int intDamage = (int) fltDamage;
			
			fltDamage = (float) intDamage;
		}
		
		float fltOldShieldHitPoints = fltShieldHitPoints;
		
		fltShieldHitPoints = fltShieldHitPoints - fltDamage;
		
		float fltShieldHitPointsMaximum = getShieldHitPointsMaximum(objDefender, intSide);
		
		if (fltOldShieldHitPoints > 0.0f && fltShieldHitPointsMaximum > 0.0f)
		{
			location locDefender = getLocation(objDefender);
			location locAttacker = isIdValid(objAttacker) ? getLocation(objAttacker) : locDefender;
			
			vector up_w = new vector(locAttacker.x - locDefender.x, locAttacker.y - locDefender.y, locAttacker.z - locDefender.z);
			
			float integrity = fltShieldHitPoints / fltShieldHitPointsMaximum;
			float previousIntegrity = (fltShieldHitPoints + fltDamage) / fltShieldHitPointsMaximum;
			
			if (Math.abs(integrity - previousIntegrity) > 0.01f)
			{
				notifyShipHit(objDefender, up_w, vector.zero, ship_hit_type.HT_shield, integrity, previousIntegrity);
			}
		}
		
		if (fltShieldHitPoints < 0)
		{
			if (((fltOldShieldHitPoints - fltDamage) < 0) && (fltOldShieldHitPoints > 0))
			{
				dictionary dctParams = new dictionary();
				
				space_utils.notifyObject(objDefender, "OnShieldsDepleted", dctParams);
			}
			
			space_combat.registerDamageDoneToShip(objAttacker, objDefender, fltDamage + fltShieldHitPoints);
			setShieldHitPoints(objDefender, intSide, 0);
			
			return Math.abs(fltShieldHitPoints);
		}
		else
		{
			if (intSide == space_combat.FRONT)
			{
				
				float fltMaximumShieldHitPoints = getShipShieldHitpointsFrontMaximum(objDefender);
				
				if (((fltOldShieldHitPoints - fltDamage) < fltMaximumShieldHitPoints / 2) && (fltOldShieldHitPoints > (fltMaximumShieldHitPoints / 2)))
				{
					
					dictionary dctParams = new dictionary();
					
					space_utils.notifyObject(objDefender, "OnShieldsHalf", dctParams);
				}
				
				doInteriorDamageNotification(objDefender, space_combat.SHIELD, fltMaximumShieldHitPoints, fltShieldHitPoints);
			}
			else if (intSide == space_combat.BACK)
			{
				float fltMaximumShieldHitPoints = getShipShieldHitpointsBackMaximum(objDefender);
				
				if (((fltOldShieldHitPoints - fltDamage) < fltMaximumShieldHitPoints / 2) && (fltOldShieldHitPoints > (fltMaximumShieldHitPoints / 2)))
				{
					
					dictionary dctParams = new dictionary();
					
					space_utils.notifyObject(objDefender, "OnShieldsHalf", dctParams);
				}
				
				doInteriorDamageNotification(objDefender, space_combat.SHIELD, fltMaximumShieldHitPoints, fltShieldHitPoints);
			}
			
			setShieldHitPoints(objDefender, intSide, fltShieldHitPoints);
			doShieldDamageSpam(objAttacker, objDefender, intSide, fltDamage, fltShieldHitPoints);
			space_combat.registerDamageDoneToShip(objAttacker, objDefender, fltDamage);
			
			return 0;
		}
	}
	
	
	public static float damageArmor(obj_id objAttacker, obj_id objDefender, int intWeaponSlot, float fltDamage, int intSide) throws InterruptedException
	{
		float fltArmorHitPoints = getArmorHitPoints(objDefender, intSide);
		
		if (fltArmorHitPoints > 0)
		{
			if (isIdValid(objAttacker))
			{
				if (isShipSlotInstalled(objAttacker, intWeaponSlot))
				{
					float fltWeaponArmorRating = getShipWeaponEffectivenessArmor(objAttacker, intWeaponSlot);
					
					fltDamage = fltDamage * fltWeaponArmorRating;
				}
			}
			
			int intDamage = (int) fltDamage;
			
			fltDamage = (float) intDamage;
		}
		
		fltArmorHitPoints = fltArmorHitPoints - fltDamage;
		
		float fltMaximumArmorHitPoints = getArmorHitPointsMaximum(objDefender, intSide);
		
		if ((fltArmorHitPoints + fltDamage) > 0.0f && fltMaximumArmorHitPoints > 0.0f)
		{
			location locDefender = getLocation(objDefender);
			location locAttacker = isIdValid(objAttacker) ? getLocation(objAttacker) : locDefender;
			
			vector up_w = new vector(locAttacker.x - locDefender.x, locAttacker.y - locDefender.y, locAttacker.z - locDefender.z);
			
			float integrity = fltArmorHitPoints / fltMaximumArmorHitPoints;
			float previousIntegrity = (fltArmorHitPoints + fltDamage) / fltMaximumArmorHitPoints;
			
			if (Math.abs(integrity - previousIntegrity) > 0.01f)
			{
				notifyShipHit(objDefender, up_w, vector.zero, ship_hit_type.HT_armor, integrity, previousIntegrity);
			}
		}
		
		if (fltArmorHitPoints < 0)
		{
			space_combat.registerDamageDoneToShip(objAttacker, objDefender, fltDamage + fltArmorHitPoints);
			setArmorHitPoints(objDefender, intSide, 0);
			doArmorDamageSpam(objAttacker, objDefender, intSide, fltDamage + fltArmorHitPoints, 0);
			return Math.abs(fltArmorHitPoints);
		}
		else
		{
			if (intSide == space_combat.FRONT)
			{
				int intSlot = ship_chassis_slot_type.SCST_armor_0;
				
				doInteriorDamageNotification(objDefender, space_combat.ARMOR, getShipComponentArmorHitpointsMaximum(objDefender, intSlot), fltArmorHitPoints);
			}
			else if (intSide == space_combat.BACK)
			{
				int intSlot = ship_chassis_slot_type.SCST_armor_1;
				
				doInteriorDamageNotification(objDefender, space_combat.ARMOR, getShipComponentArmorHitpointsMaximum(objDefender, intSlot), fltArmorHitPoints);
			}
			
			space_combat.registerDamageDoneToShip(objAttacker, objDefender, fltDamage + fltDamage);
			setArmorHitPoints(objDefender, intSide, fltArmorHitPoints);
			doArmorDamageSpam(objAttacker, objDefender, intSide, fltDamage, fltArmorHitPoints);
			
			return 0;
		}
	}
	
	
	public static float doComponentDamage(obj_id objAttacker, obj_id objDefender, int intWeaponSlot, int intTargetedComponent, float fltDamage, int intSide) throws InterruptedException
	{
		
		boolean boolNoPassThrough = false;
		
		if (intTargetedComponent != space_combat.SHIP)
		{
			if (!isShipSlotInstalled(objDefender, intTargetedComponent))
			{
				intTargetedComponent = getComponentToDamage(objAttacker, objDefender, intTargetedComponent, intSide);
				if (intTargetedComponent == space_combat.SHIP)
				{
					
					return fltDamage;
				}
			}
			else
			{
				
				fltDamage = fltDamage * 1.0f;
				
				boolNoPassThrough = true;
			}
		}
		else
		{
			intTargetedComponent = getComponentToDamage(objAttacker, objDefender, intTargetedComponent, intSide);
			
			if (intTargetedComponent == space_combat.SHIP)
			{
				
				return fltDamage;
			}
		}
		
		fltDamage = damageComponent(objAttacker, objDefender, intWeaponSlot, intTargetedComponent, fltDamage, intSide, boolNoPassThrough);
		
		if (boolNoPassThrough)
		{
			return 0;
			
		}
		
		return fltDamage;
	}
	
	
	public static dictionary getComponentWeightings(obj_id objDefender, int intTargetedComponent) throws InterruptedException
	{
		if (intTargetedComponent != space_combat.SHIP)
		{
			
			if (!isShipSlotInstalled(objDefender, intTargetedComponent))
			{
				intTargetedComponent = space_combat.SHIP;
			}
			else
			{
				dictionary dctReturn = new dictionary();
				int[] intSlots = new int[1];
				int[] intWeightings = new int[1];
				
				intSlots[0] = intTargetedComponent;
				intWeightings[0] = 100;
				dctReturn.put("intWeightings", intWeightings);
				dctReturn.put("intSlots", intSlots);
				return dctReturn;
			}
			
		}
		
		String strFileName = "datatables/space/ship_chassis.iff";
		
		String strChassisType = getShipChassisType(objDefender);
		dictionary dctRow = dataTableGetRow(strFileName, strChassisType);
		
		if (dctRow == null)
		{
			
			dctRow = dataTableGetRow(strFileName, "xwing");
		}
		
		Vector intWeightings = new Vector();
		intWeightings.setSize(0);
		Vector intSlots = new Vector();
		intSlots.setSize(0);
		
		int[] intRawSlots = getShipChassisSlots(objDefender);
		
		for (int intI = 0; intI < intRawSlots.length; intI++)
		{
			testAbortScript();
			
			if ((isShipSlotInstalled(objDefender, intRawSlots[intI])) && (intRawSlots[intI] != ship_chassis_slot_type.SCST_armor_0) && (intRawSlots[intI] != ship_chassis_slot_type.SCST_armor_1))
			{
				
				String strSlotName = space_crafting.getComponentSlotNameString(intRawSlots[intI]);
				
				strSlotName = strSlotName + "_hitweight";
				
				intWeightings = utils.addElement(intWeightings, dctRow.getInt(strSlotName));
				
				intSlots = utils.addElement(intSlots, intRawSlots[intI]);
			}
			else
			{
				
			}
		}
		
		if (intWeightings.size() == 0)
		{
			
			return null;
		}
		
		dictionary dctReturn = new dictionary();
		
		dctReturn.put("intWeightings", intWeightings);
		dctReturn.put("intSlots", intSlots);
		return dctReturn;
		
	}
	
	
	public static int getComponentToDamage(obj_id objAttacker, obj_id objDefender, int intTargetedComponent, int intSide) throws InterruptedException
	{
		
		dictionary dctReturn = getComponentWeightings(objDefender, intTargetedComponent);
		
		if (dctReturn == null)
		{
			
			return space_combat.SHIP;
			
		}
		
		int[] intSlots = dctReturn.getIntArray("intSlots");
		int[] intRawWeightings = dctReturn.getIntArray("intWeightings");
		int[] intNewWeightings = new int[intRawWeightings.length * 2];
		
		int intCounter = 0;
		
		if (intRawWeightings.length > 1)
		{
			
			int intJ = 0;
			
			for (int intI = 0; intI < intNewWeightings.length; intI = intI + 2)
			{
				testAbortScript();
				intNewWeightings[intI] = intCounter;
				
				intCounter = intCounter + intRawWeightings[intJ];
				intJ = intJ + 1;
				
				intNewWeightings[intI + 1] = intCounter;
				intCounter = intCounter + 1;
			}
			
			int intRoll = rand(0, intNewWeightings[intNewWeightings.length - 1]);
			
			intCounter = 0;
			
			for (int intI = 0; intI < intNewWeightings.length; intI = intI + 2)
			{
				testAbortScript();
				
				if ((intNewWeightings[intI] <= intRoll) && (intNewWeightings[intI + 1] >= intRoll))
				{
					
					break;
				}
				
				intCounter++;
			}
		}
		else
		{
			
		}
		
		int intComponent = intSlots[intCounter];
		
		return intComponent;
	}
	
	
	public static float damageComponent(obj_id objAttacker, obj_id objDefender, int intWeaponSlot, int intComponent, float fltDamage, int intSide, boolean boolTrackDisabled) throws InterruptedException
	{
		int intDamage = (int) fltDamage;
		
		fltDamage = (float) intDamage;
		
		float fltOldComponentArmor = getShipComponentArmorHitpointsCurrent(objDefender, intComponent);
		float fltOldComponentHitPoints = getShipComponentHitpointsCurrent(objDefender, intComponent);
		
		float fltComponentArmor = fltOldComponentArmor;
		float fltComponentHitPoints = fltOldComponentHitPoints;
		
		float fltRemainingDamage = 0;
		
		if ((fltComponentHitPoints == 0) && (fltComponentArmor == 0))
		{
			return fltDamage;
		}
		
		fltComponentArmor = fltComponentArmor - fltDamage;
		
		if (fltComponentArmor < 0)
		{
			
			space_combat.registerDamageDoneToShip(objAttacker, objDefender, fltDamage + fltComponentArmor);
			setShipComponentArmorHitpointsCurrent(objDefender, intComponent, 0);
			fltDamage = Math.abs(fltComponentArmor);
			fltComponentArmor = 0;
			fltComponentHitPoints = fltComponentHitPoints - fltDamage;
			
			if (fltComponentHitPoints < 0)
			{
				
				space_combat.registerDamageDoneToShip(objAttacker, objDefender, fltDamage + fltComponentHitPoints);
				fltRemainingDamage = Math.abs(fltComponentHitPoints);
				setShipComponentHitpointsCurrent(objDefender, intComponent, 0);
				space_combat.recalculateEfficiency(intComponent, objDefender);
				
				if (boolTrackDisabled)
				{
					if (space_combat.isShipDisabled(objDefender, intComponent) && !hasObjVar(objDefender, "isDisabled"))
					{
						dictionary dctParams = new dictionary();
						
						dctParams.put("objAttacker", objAttacker);
						dctParams.put("objDefender", objDefender);
						
						if (space_utils.isPlayerControlledShip(objDefender) && (!space_utils.isShipWithInterior(objDefender)))
						{
							setCondition(objDefender, CONDITION_EJECT);
						}
						
						space_utils.notifyObject(objDefender, "OnShipDisabled", dctParams);
						
						if (!hasObjVar(objDefender, "noNotifyDisable"))
						{
							space_combat.notifyAttackerDisabled(objAttacker, objDefender, dctParams);
						}
						
						setObjVar(objDefender, "isDisabled", getGameTime() + 5);
					}
				}
				else
				{
					if (space_utils.isPlayerControlledShip(objDefender))
					{
						if (space_utils.isPlayerControlledShip(objDefender) && (!space_utils.isShipWithInterior(objDefender)))
						{
							setCondition(objDefender, CONDITION_EJECT);
						}
					}
				}
				
				doComponentDamageSpam(objAttacker, objDefender, intComponent, fltDamage);
				fltComponentHitPoints = 0;
			}
			else
			{
				
				space_combat.recalculateEfficiency(intComponent, objDefender);
				doInteriorDamageNotification(objDefender, space_combat.COMPONENT, 90, 100);
				space_combat.registerDamageDoneToShip(objAttacker, objDefender, fltDamage);
				setShipComponentHitpointsCurrent(objDefender, intComponent, fltComponentHitPoints);
				space_combat.recalculateEfficiency(intComponent, objDefender);
				doComponentDamageSpam(objAttacker, objDefender, intComponent, fltDamage);
				fltRemainingDamage = 0;
			}
		}
		else
		{
			doInteriorDamageNotification(objDefender, space_combat.COMPONENT, 20, 100);
			
			space_combat.registerDamageDoneToShip(objAttacker, objDefender, fltDamage);
			
			setShipComponentArmorHitpointsCurrent(objDefender, intComponent, fltComponentArmor);
			doComponentDamageSpam(objAttacker, objDefender, intComponent, fltDamage);
			fltRemainingDamage = 0;
		}
		
		float fltComponentHitpointsArmorMaximum = getShipComponentArmorHitpointsMaximum(objDefender, intComponent) + getShipComponentHitpointsMaximum(objDefender, intComponent);
		float fltOldComponentHitpointsArmor = fltOldComponentArmor + fltOldComponentHitPoints;
		
		if (fltOldComponentHitpointsArmor > 0.0f && fltComponentHitpointsArmorMaximum > 0.0f)
		{
			float fltComponentHitpointsArmor = fltComponentArmor + fltComponentHitPoints;
			
			location locDefender = getLocation(objDefender);
			location locAttacker = isIdValid(objAttacker) ? getLocation(objAttacker) : locDefender;
			
			vector up_w = new vector(locAttacker.x - locDefender.x, locAttacker.y - locDefender.y, locAttacker.z - locDefender.z);
			
			float integrity = fltComponentHitpointsArmor / fltComponentHitpointsArmorMaximum;
			float previousIntegrity = (fltOldComponentHitpointsArmor) / fltComponentHitpointsArmorMaximum;
			
			if (Math.abs(integrity - previousIntegrity) > 0.01f)
			{
				notifyShipHit(objDefender, up_w, vector.zero, ship_hit_type.HT_component, integrity, previousIntegrity);
			}
		}
		
		return fltRemainingDamage;
	}
	
	
	public static String getSideName(int intSide) throws InterruptedException
	{
		if (intSide == space_combat.FRONT)
		{
			return "Front";
		}
		
		if (intSide == space_combat.BACK)
		{
			return "Back";
		}
		return "";
	}
	
	
	public static void doShieldDamageSpam(obj_id objAttacker, obj_id objDefender, int intSide, float fltDamage, float fltRemainingHitPoints) throws InterruptedException
	{
		if (!space_utils.isShip(objAttacker) || (!space_utils.isShip(objDefender)))
		{
			return;
		}
		
		obj_id objAttackerPilot = getPilotId(objAttacker);
		obj_id objDefenderPilot = getPilotId(objDefender);
		
		if (isIdValid(objAttackerPilot) && hasObjVar(objAttackerPilot, "intCombatDebug"))
		{
			if (fltDamage > 0)
			{
				String strSide = getSideName(intSide);
				
				sendSystemMessageTestingOnly(objAttackerPilot, "Succesful Shield Hit! Side: "+ strSide + " Damage:"+ fltDamage + " Remaining Shield Points:"+ fltRemainingHitPoints);
			}
		}
		
		if (isIdValid(objDefenderPilot))
		{
			
		}
		
		return;
	}
	
	
	public static void doArmorDamageSpam(obj_id objAttacker, obj_id objDefender, int intSide, float fltDamage, float fltRemainingHitPoints) throws InterruptedException
	{
		if (!space_utils.isShip(objAttacker) || (!space_utils.isShip(objDefender)))
		{
			return;
		}
		
		obj_id objAttackerPilot = getPilotId(objAttacker);
		obj_id objDefenderPilot = getPilotId(objDefender);
		
		if (!isIdValid(objAttackerPilot))
		{
			return;
		}
		
		if (hasObjVar(objAttackerPilot, "intCombatDebug"))
		{
			if (fltDamage > 0)
			{
				String strSide = getSideName(intSide);
				
				if (isIdValid(objAttackerPilot))
				{
					
				}
			}
		}
		
		if (isIdValid(objDefenderPilot))
		{
			
		}
		
		return;
	}
	
	
	public static void doComponentDamageSpam(obj_id objAttacker, obj_id objDefender, int intComponent, float fltDamage) throws InterruptedException
	{
		if (!space_utils.isShip(objAttacker) || (!space_utils.isShip(objDefender)))
		{
			return;
		}
		
		obj_id objAttackerPilot = getPilotId(objAttacker);
		obj_id objDefenderPilot = getPilotId(objDefender);
		
		if (!isIdValid(objAttackerPilot))
		{
			return;
		}
		
		if (hasObjVar(objAttackerPilot, "intCombatDebug"))
		{
			if (fltDamage > 0)
			{
				
				float fltHp = getShipComponentHitpointsCurrent(objDefender, intComponent);
				float fltGeneralEfficiency = getShipComponentEfficiencyGeneral(objDefender, intComponent);
				float fltEnergyEfficiency = getShipComponentEfficiencyEnergy(objDefender, intComponent);
				
				float fltArmor = getShipComponentArmorHitpointsCurrent(objDefender, intComponent);
				
				if (isIdValid(objAttackerPilot))
				{
					sendSystemMessageTestingOnly(objAttackerPilot, "Succesful Component Hit! Component Name:"+ space_crafting.getComponentSlotNameString(intComponent) + " Damage:"+ fltDamage);
				}
				
				if (fltHp <= 0)
				{
					if (isIdValid(objAttackerPilot))
					{
						sendSystemMessageTestingOnly(objAttackerPilot, "Component Destroyed!");
					}
				}
				else
				{
					if (isIdValid(objAttackerPilot))
					{
						sendSystemMessageTestingOnly(objAttackerPilot, "Component Armor Integrity:"+ fltArmor + " Component Energy Efficacy:"+ fltEnergyEfficiency + "%"+ " General Efficiency: "+ fltGeneralEfficiency);
					}
				}
			}
		}
		
		if (isIdValid(objDefenderPilot))
		{
			
		}
		
		return;
		
	}
	
	
	public static void doChassisDamageSpam(obj_id objAttacker, obj_id objDefender, float fltDamage, float fltRemainingHitPoints) throws InterruptedException
	{
		if (!space_utils.isShip(objAttacker) || !space_utils.isShip(objDefender))
		{
			return;
		}
		
		obj_id objAttackerPilot = getPilotId(objAttacker);
		
		if (!isIdValid(objAttackerPilot))
		{
			return;
		}
		
		obj_id objDefenderPilot = getPilotId(objDefender);
		
		if (hasObjVar(objAttackerPilot, "intCombatDebug"))
		{
			if (fltDamage > 0)
			{
				
				if (isIdValid(objAttackerPilot))
				{
					
					sendSystemMessageTestingOnly(objAttackerPilot, "Succesful Chasis Hit! Damage:"+ fltDamage + " Reamining Chassis Integrity:"+ fltRemainingHitPoints);
				}
			}
		}
		
		if (isIdValid(objDefenderPilot))
		{
			
		}
		
		return;
	}
	
	
	public static void doInteriorDamageNotification(obj_id objShip, int intDamageType, float fltCurrentHitpoints, float fltMaximumHitpoints) throws InterruptedException
	{
		
		if (hasScript(objShip, "space.ship.ship_interior"))
		{
			dictionary dctParams = new dictionary();
			
			dctParams.put("intDamageType", intDamageType);
			int intDamageIntensity = space_combat.LIGHT;
			
			if ((fltCurrentHitpoints > (fltMaximumHitpoints / 2)))
			{
				intDamageIntensity = space_combat.HEAVY;
			}
			
			dctParams.put("intDamageIntensity", intDamageIntensity);
			space_utils.notifyObject(objShip, "interiorDamageNotification", dctParams);
			
		}
	}
	
	
	public static void checkAndPerformCombatTaunts(obj_id objTaunter, obj_id objTauntee, String strLocalVar, String strType, float fltRange) throws InterruptedException
	{
		final int TAUNT_DELAY = 3;
		final int TAUNT_QUANTITY = 5;
		final int TAUNT_LISTEN_DELAY = 2;
		
		if ((isIdValid(objTauntee) && (!space_utils.isPlayerControlledShip(objTauntee))))
		{
			
			return;
		}
		
		if ((isIdValid(objTaunter) && (space_utils.isPlayerControlledShip(objTaunter))))
		{
			
			return;
		}
		
		obj_id objOwner = getOwner(objTauntee);
		
		if (hasObjVar(objOwner, "jtl.combatTauntsDisabled"))
		{
			return;
		}
		
		int intGameTime = getGameTime();
		int intLastTime = 0;
		
		if (isIdValid(objTaunter))
		{
			intLastTime = utils.getIntLocalVar(objTaunter, "intTauntTime");
			
			if (((intGameTime - intLastTime) < TAUNT_DELAY))
			{
				
				return;
			}
			
		}
		
		if (isIdValid(objTauntee))
		{
			intLastTime = utils.getIntLocalVar(objTauntee, "intTauntTime");
			
			if (((intGameTime - intLastTime) < TAUNT_DELAY))
			{
				
				return;
			}
			
		}
		
		float fltChance = utils.getFloatLocalVar(objTaunter, strLocalVar);
		float fltRoll = rand();
		
		fltRoll = fltRoll * 100;
		fltChance = fltChance * 100;
		
		if (fltRoll < fltChance)
		{
			
			String strDiction = utils.getStringLocalVar(objTaunter, "strTauntDiction");
			
			if ((strDiction != null) && (!strDiction.equals("")))
			{
				String strFileName = "space/taunts/"+ strDiction;
				
				strType = strType + rand(1, TAUNT_QUANTITY);
				string_id strSpam = new string_id(strFileName, strType);
				
				if (fltRange > 0)
				{
					obj_id[] objPlayerShips = getObservedPlayerShips(objTaunter, true);
					
					if (objPlayerShips != null)
					{
						for (int intI = 0; intI < objPlayerShips.length; intI++)
						{
							testAbortScript();
							intLastTime = utils.getIntLocalVar(objPlayerShips[intI], "intTauntTime");
							
							if (((intGameTime - intLastTime) > TAUNT_LISTEN_DELAY))
							{
								if ((getDistance(objPlayerShips[intI], objTaunter) < fltRange))
								{
									prose_package pp = new prose_package();
									
									pp.stringId = strSpam;
									pp.actor.set(objOwner);
									pp.target.set(objOwner);
									space_utils.tauntShip(objPlayerShips[intI], objTaunter, pp, true, false, true, true);
									utils.setLocalVar(objTaunter, "intTauntTime", intGameTime);
									utils.setLocalVar(objTauntee, "intTauntTime", intGameTime);
								}
							}
						}
					}
				}
				else
				{
					
					prose_package pp = new prose_package();
					
					pp.stringId = strSpam;
					pp.actor.set(objOwner);
					pp.target.set(objOwner);
					
					space_utils.tauntShip(objTauntee, objTaunter, pp, true, false, true, true);
					utils.setLocalVar(objTaunter, "intTauntTime", intGameTime);
					utils.setLocalVar(objTauntee, "intTauntTime", intGameTime);
				}
			}
		}
	}
	
	
	public static void setupCapitalShipFromTurretDefinition(obj_id objShip, String strDefinition) throws InterruptedException
	{
		String strFileName = "datatables/space_combat/turret_definitions/"+ strDefinition + ".iff";
		
		String[] strSlots = dataTableGetStringColumn(strFileName, "strSlot");
		int[] intComponents = dataTableGetIntColumn(strFileName, "intComponent");
		float[] fltComponentHitpoints = dataTableGetFloatColumn(strFileName, "fltComponentHitpoints");
		float[] fltArmorHitpoints = dataTableGetFloatColumn(strFileName, "fltArmorHitpoints");
		float[] fltMinDamage = dataTableGetFloatColumn(strFileName, "fltMinDamage");
		float[] fltMaxDamage = dataTableGetFloatColumn(strFileName, "fltMaxDamage");
		float[] fltRefire = dataTableGetFloatColumn(strFileName, "fltRefire");
		float[] fltDrain = dataTableGetFloatColumn(strFileName, "fltDrain");
		float[] fltShieldEfficiency = dataTableGetFloatColumn(strFileName, "fltShieldEfficiency");
		float[] fltArmorEfficiency = dataTableGetFloatColumn(strFileName, "fltArmorEfficiency");
		
		for (int intI = 0; intI < strSlots.length; intI++)
		{
			testAbortScript();
			if (intComponents[intI] != 0)
			{
				int intSlot = space_crafting.getComponentSlotInt(strSlots[intI]);
				
				if (!shipPseudoInstallComponent(objShip, intSlot, intComponents[intI]))
				{
					
				}
				else
				{
					setShipSlotTargetable(objShip, intSlot, true);
					setShipComponentArmorHitpointsMaximum(objShip, intSlot, fltArmorHitpoints[intI]);
					setShipComponentArmorHitpointsCurrent(objShip, intSlot, fltArmorHitpoints[intI]);
					setShipComponentHitpointsMaximum(objShip, intSlot, fltComponentHitpoints[intI]);
					setShipComponentHitpointsCurrent(objShip, intSlot, fltComponentHitpoints[intI]);
					setShipWeaponDamageMaximum(objShip, intSlot, fltMinDamage[intI]);
					setShipWeaponDamageMinimum(objShip, intSlot, fltMaxDamage[intI]);
					setShipWeaponEffectivenessShields(objShip, intSlot, fltShieldEfficiency[intI]);
					setShipWeaponEffectivenessArmor(objShip, intSlot, fltShieldEfficiency[intI]);
					setShipWeaponEnergyPerShot(objShip, intSlot, fltDrain[intI]);
					setShipWeaponRefireRate(objShip, intSlot, fltRefire[intI]);
					setShipWeaponEfficiencyRefireRate(objShip, intSlot, 1.0f);
					setShipComponentEfficiencyGeneral(objShip, intSlot, 1.0f);
					setShipComponentEfficiencyEnergy(objShip, intSlot, 1.0f);
				}
			}
		}
		
		String strPilotType = getStringObjVar(objShip, "ship.pilotType");
		
		ship_ai.unitSetPilotType(objShip, strPilotType);
		
		return;
	}
	
	
	public static void playCombatTauntSound(obj_id objPlayer) throws InterruptedException
	{
		String[] strSounds = 
		{
			"sound/sys_comm_generic.snd",
			"sound/sys_comm_imperial.snd",
			"sound/sys_comm_rebel.snd"
		};
		
		if (!isIdValid(objPlayer))
		{
			return;
		}
		
		playMusic(objPlayer, strSounds[rand(0, strSounds.length - 1)]);
	}
	
	
	public static transform getClosestStationRespawnLocation(obj_id objShip) throws InterruptedException
	{
		obj_id objClosestStation = getClosestSpaceStation(objShip);
		
		if (!isIdValid(objClosestStation) || !exists(objClosestStation))
		{
			return null;
			
		}
		
		transform trTest = space_utils.getRandomPositionInSphere(getTransform_o2p(objClosestStation), 425f, 450f, false);
		vector vctTest = space_utils.getVector(objClosestStation);
		transform trFinalTransform = space_utils.faceTransformToVector(trTest, vctTest);
		
		return trFinalTransform;
	}
	
	
	public static location getClosestStationLocation(obj_id objShip) throws InterruptedException
	{
		obj_id objClosestStation = getClosestSpaceStation(objShip);
		
		if (!isIdValid(objClosestStation) || !exists(objClosestStation))
		{
			return null;
		}
		
		return getLocation(objClosestStation);
	}
	
	
	public static void sendDestructionNotification(obj_id objShip, obj_id objAttacker) throws InterruptedException
	{
		if (space_utils.isPlayerControlledShip(objShip))
		{
			Vector objOfficers = space_utils.getShipOfficers(objShip);
			dictionary dctParams = new dictionary();
			
			dctParams.put("objAttacker", objAttacker);
			dctParams.put("objShip", objShip);
			
			for (int intI = 0; intI < objOfficers.size(); intI++)
			{
				testAbortScript();
				space_utils.notifyObject(((obj_id)(objOfficers.get(intI))), "playerShipDestroyed", dctParams);
			}
		}
		
		return;
	}
	
	
	public static void clearHyperspace(obj_id objShip) throws InterruptedException
	{
		if (utils.hasScriptVar(objShip, "intHyperspacing"))
		{
			utils.removeScriptVar(objShip, "intHyperspacing");
		}
		
		if (utils.hasScriptVar(objShip, "intHyperspaceDelayStamp"))
		{
			utils.removeScriptVar(objShip, "intHyperspaceDelayStamp");
		}
		
		obj_id objPilot = getPilotId(objShip);
		
		if (isIdValid(objPilot))
		{
			hyperspaceRestoreShipOnClientFromAbortedHyperspace(objPilot);
		}
	}
	
	
	public static void doDeathCleanup(obj_id self) throws InterruptedException
	{
		if (hasObjVar(self, "intAlarmsOn"))
		{
			space_crafting.turnOffInteriorAlarms(self);
			setObjVar(self, "intAlarmsOn", 1);
		}
		
		utils.removeScriptVar(self, "intHyperspacing");
	}
	
	
	public static void setDeathFlags(obj_id objShip) throws InterruptedException
	{
		
		utils.setScriptVar(objShip, "intDestroyed", getGameTime());
		return;
	}
	
	
	public static void clearDeathFlags(obj_id objShip) throws InterruptedException
	{
		
		utils.removeScriptVar(objShip, "intDestroyed");
		utils.removeScriptVar(objShip, "intPVPKill");
		
		return;
	}
	
	
	public static boolean hasDeathFlags(obj_id objShip) throws InterruptedException
	{
		
		if (utils.hasScriptVar(objShip, "intDestroyed"))
		{
			int intTime = getGameTime();
			int intFlagTime = utils.getIntScriptVar(objShip, "intDestroyed");
			int intDifference = intTime - intFlagTime;
			
			if (intDifference > 20)
			{
				utils.removeScriptVar(objShip, "intDestroyed");
				
				return false;
			}
			
			return true;
		}
		
		return false;
	}
	
	
	public static void addModuleToDatapad(obj_id objModule, obj_id objDatapad) throws InterruptedException
	{
		String strDroidCommand = getStringObjVar(objModule, "strDroidCommand");
		
		int programSize = 1;
		
		if (hasObjVar(objModule, "programSize"))
		{
			programSize = getIntObjVar(objModule, "programSize");
			addModuleToDatapad(strDroidCommand, objDatapad, programSize);
			destroyObject(objModule);
		}
		else
		{
			addModuleToDatapad(strDroidCommand, objDatapad);
		}
		
		return;
	}
	
	
	public static void addModuleToDatapad(String strDroidCommand, obj_id objDatapad) throws InterruptedException
	{
		int programSize = space_pilot_command.getDroidProgramSize(strDroidCommand, null);
		
		addModuleToDatapad(strDroidCommand, objDatapad, programSize);
		
		return;
	}
	
	
	public static void addModuleToDatapad(String strDroidCommand, obj_id objDatapad, int intProgramSize) throws InterruptedException
	{
		String programDataTemplate = "object/intangible/data_item/droid_command.iff";
		
		switch (intProgramSize)
		{
			case 35:
			programDataTemplate = "object/intangible/data_item/droid_command_5.iff";
			break;
			case 25:
			programDataTemplate = "object/intangible/data_item/droid_command_4.iff";
			break;
			case 20:
			programDataTemplate = "object/intangible/data_item/droid_command_3.iff";
			break;
			case 10:
			programDataTemplate = "object/intangible/data_item/droid_command_2.iff";
			break;
			case 5:
			programDataTemplate = "object/intangible/data_item/droid_command_1.iff";
			break;
		}
		
		obj_id objNewCommand = createObject(programDataTemplate, objDatapad, "");
		
		setObjVar(objNewCommand, "strDroidCommand", strDroidCommand);
		setName(objNewCommand, new string_id("space/droid_commands", strDroidCommand + "_chipname"));
		setObjVar(objNewCommand, "programSize", intProgramSize);
		
		return;
	}
	
	
	public static void doBattlefieldRepairCheck(obj_id objShip) throws InterruptedException
	{
		location locTest = getLocation(objShip);
		
		if (locTest.area.equals("space_heavy1"))
		{
			
			obj_id objPlayer = getOwner(objShip);
			
			space_crafting.repairDamage(objPlayer, objShip, 1.0f, 0);
		}
		
		return;
	}
	
	
	public static void simpleShieldRatioRebalance(obj_id ship) throws InterruptedException
	{
		float fltShieldFrontMaximum = getShipShieldHitpointsFrontMaximum(ship);
		float fltShieldBackMaximum = getShipShieldHitpointsBackMaximum(ship);
		
		if (fltShieldFrontMaximum != fltShieldBackMaximum)
		{
			float fltShieldTotal = fltShieldFrontMaximum + fltShieldBackMaximum;
			float fltFrontMax = (fltShieldTotal / 2);
			float fltBackMax = fltShieldTotal - fltFrontMax;
			
			setShipShieldHitpointsFrontMaximum(ship, fltFrontMax);
			setShipShieldHitpointsBackMaximum(ship, fltBackMax);
		}
		
		return;
	}
	
	
	public static boolean isCapShip(obj_id ship) throws InterruptedException
	{
		return (dataTableGetInt("datatables/space_mobile/space_mobile.iff", getStringObjVar(ship, "ship.shipName"), "shipClass") == 2);
	}
	
	
	public static boolean getCollectionLoot(obj_id objAttacker, obj_id objPilot, obj_id objContainer, String collectionLoot) throws InterruptedException
	{
		if (!isIdValid(objAttacker))
		{
			return false;
		}
		
		if (!isIdValid(objPilot))
		{
			return false;
		}
		
		if (!isIdValid(objContainer))
		{
			return false;
		}
		
		if (collectionLoot.equals(""))
		{
			return false;
		}
		
		String[] collectionsInColumn = split(collectionLoot, ',');
		int collectionIndex = rand(0, (collectionsInColumn.length - 1));
		collectionLoot = collectionsInColumn[collectionIndex];
		
		String[] lootArray = dataTableGetStringColumnNoDefaults(SPACE_COLLECTIONS_LOOT_TABLE, collectionLoot);
		int max = lootArray.length -1;
		int idx = rand(0, max);
		String lootToGrant = lootArray[idx];
		
		attemptToGrantLootItem(lootToGrant, objAttacker, objContainer);
		
		return true;
	}
	
	
	public static boolean attemptToGrantLootItem(String lootItem, obj_id objAttacker, obj_id objContainer) throws InterruptedException
	{
		
		if (!isIdValid(objAttacker))
		{
			return false;
		}
		
		if (!isIdValid(objContainer))
		{
			return false;
		}
		
		if (lootItem.equals(""))
		{
			return false;
		}
		
		obj_id objItem = space_utils.createObjectTrackItemCount(lootItem, objContainer);
		
		if (isIdValid(objItem))
		{
			sendItemNotification(objAttacker, objItem);
			return true;
		}
		
		string_id strSpam = new string_id("space/space_loot", "no_more_loot");
		
		if (space_utils.isShipWithInterior(objAttacker))
		{
			strSpam = new string_id("space/space_loot", "loot_box_full");
		}
		
		space_utils.sendSystemMessageShip(objAttacker, strSpam, true, true, true, true);
		CustomerServiceLog("space_loot", "Failed to create "+ lootItem + " in "+ objContainer + " contained by "+ objAttacker + ". The Ship or Player Inventory container was full.", getOwner(objAttacker));
		return false;
	}
}
