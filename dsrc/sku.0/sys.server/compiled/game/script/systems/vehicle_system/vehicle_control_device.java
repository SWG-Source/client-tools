package script.systems.vehicle_system;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.beast_lib;
import script.library.callable;
import script.library.hue;
import script.library.utils;
import script.library.sui;
import script.library.pet_lib;
import script.library.create;
import script.library.chat;
import script.library.ai_lib;
import script.library.battlefield;
import script.library.factions;
import script.library.consumable;
import script.library.healing;
import script.library.attrib;
import script.library.vehicle;
import script.library.pclib;
import script.library.prose;
import script.library.buff;


public class vehicle_control_device extends script.base_script
{
	public vehicle_control_device()
	{
	}
	public static final boolean debug = false;
	
	public static final String CREATURE_TABLE = "datatables/mob/creatures.iff";
	public static final String VCDPING_VCD_SCRIPT_NAME = "systems.vehicle_system.vcd_ping_response";
	public static final String MESSAGE_VEHICLE_ID = "vehicleId";
	public static final string_id SID_FAILED_TO_CALL = new string_id("pet/pet_menu", "failed_to_call_vehicle");
	
	public static final String[] TCG_VEHICLE_UPDATE_LIST =  
	{
		"object/intangible/vehicle/landspeeder_organa_pcd.iff",
		"object/intangible/vehicle/mechno_chair_pcd.iff",
		"object/intangible/vehicle/sith_speeder_pcd.iff"
	};
	
	public static final int TCG_PCD_MAX_HEALTH = 15000;
	public static final int TCG_PCD_OLD_MAX_HEALTH = 1500;
	public static final int TCG_PCD_OLD_NEW_DIFFERENCE = 13500;
	public static final String TCG_VEHICLE_PCD_UPDATED = "tcg.vehicle_pcd_maxhealth_update";
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		if (debug)
		{
			LOG("vcdping-debug", "vehicle_control_device.OnInitialize(): self=["+ self + "] entered");
		}
		
		if (!hasScript(self, VCDPING_VCD_SCRIPT_NAME))
		{
			if (debug)
			{
				LOG("vcdping-debug", "vehicle_control_device.OnInitialize(): attaching script ["+ VCDPING_VCD_SCRIPT_NAME + "] to VCD id=["+ self + "]");
			}
			attachScript(self, VCDPING_VCD_SCRIPT_NAME);
		}
		
		if (debug)
		{
			LOG("vcdping-debug", "vehicle_control_device.OnInitialize(): self=["+ self + "] exited");
		}
		
		if (hasObjVar(self, "pet.uses_left"))
		{
			int uses = getIntObjVar(self, "pet.uses_left");
			
			if (uses <= 0)
			{
				destroyObject(self);
			}
		}
		
		if (!hasObjVar(self, "attrib.max_hp") || !hasObjVar(self, "attrib.hit_points"))
		{
			
			obj_id owner = self;
			
			do
			{
				testAbortScript();
				owner = getContainedBy(owner);
				
			}
			while (isIdValid(owner) && !isPlayer(owner));
			
			if (isIdValid(owner))
			{
				CustomerServiceLog("40K_HAM_vehicle_fixup", "vehicle PCD "+ getName(self)
				+ " ("+ self + ") with current hp (" + (hasObjVar(self, "attrib.hit_points") ? Integer.toString(getIntObjVar(self, "attrib.hit_points")) : "not set")
				+ ") max hp (" + (hasObjVar(self, "attrib.max_hp") ? Integer.toString(getIntObjVar(self, "attrib.max_hp")) : "not set")
				+ ") owned by %TU " + "fixed to current hp (500) max hp (1500)", owner);
			}
			else
			{
				CustomerServiceLog("40K_HAM_vehicle_fixup", "vehicle PCD "+ getName(self)
				+ " ("+ self + ") with current hp (" + (hasObjVar(self, "attrib.hit_points") ? Integer.toString(getIntObjVar(self, "attrib.hit_points")) : "not set")
				+ ") max hp (" + (hasObjVar(self, "attrib.max_hp") ? Integer.toString(getIntObjVar(self, "attrib.max_hp")) : "not set")
				+ ") " + "fixed to current hp (500) max hp (1500)");
			}
			
			setObjVar(self, "attrib.max_hp", 1500);
			setObjVar(self, "attrib.hit_points", 500);
		}
		
		obj_id current_vehicle = callable.getCDCallable(self);
		
		if (!isIdValid(current_vehicle))
		{
			setCount(self, 0);
		}
		else
		{
			setCount(self, 1);
		}
		
		if (hasObjVar(self, TCG_VEHICLE_PCD_UPDATED))
		{
			return SCRIPT_CONTINUE;
		}
		
		String templateName = getTemplateName(self);
		if (templateName == null || templateName.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		
		String tcgPcd = "";
		
		for (int i = 0; i < TCG_VEHICLE_UPDATE_LIST.length; i++)
		{
			testAbortScript();
			
			if (!TCG_VEHICLE_UPDATE_LIST[i].startsWith(templateName))
			{
				continue;
			}
			
			tcgPcd = TCG_VEHICLE_UPDATE_LIST[i];
			break;
		}
		if (tcgPcd == null || tcgPcd.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!updateTcgVehicleMaxHealth(self))
		{
			CustomerServiceLog("vehicle_bug", "vehicle PCD "+ getName(self) + "failed TCG PCD Max Health update.");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		
		obj_id myContainer = getContainedBy( self );
		obj_id yourPad = utils.getPlayerDatapad(player);
		if (myContainer != yourPad)
		{
			if (hasScript( myContainer, "ai.pet_control_device" ))
			{
				
				putIn( self, yourPad );
			}
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (item == menu_info_types.VEHICLE_GENERATE)
		{
			if (ai_lib.aiIsDead(player))
			{
				sendSystemMessage(player, vehicle.SID_NOT_WHILE_DEAD);
				return SCRIPT_CONTINUE;
			}
			
			if (callable.hasCallable(player, callable.CALLABLE_TYPE_RIDEABLE) && callable.getControlDeviceType(self) == callable.CALLABLE_TYPE_RIDEABLE)
			{
				obj_id cd = callable.getCallableCD(callable.getCallable(player, callable.CALLABLE_TYPE_RIDEABLE));
				
				if (cd != self)
				{
					sendSystemMessage(player, pet_lib.SID_SYS_CANT_CALL_ANOTHER_RIDEABLE);
					return SCRIPT_CONTINUE;
				}
				else
				{
					vehicle.storeVehicle(self, player);
					callable.restoreCallable(player);
					return SCRIPT_CONTINUE;
				}
			}
			
			if (callable.hasCDCallable(self))
			{
				obj_id currentPet = callable.getCDCallable(self);
				
				if (isIdValid(currentPet))
				{
					dictionary dict = new dictionary();
					dict.put("signalId", getGameTime());
					
					sendSystemMessage(player, SID_FAILED_TO_CALL);
					
					messageTo(currentPet, "destroyNow", dict, 0, false);
					
					callable.setCDCallable(self, null);
					
					sendDirtyObjectMenuNotification(self);
					CustomerServiceLog("vehicle_bug", "VCD-OnObjectMenuSelect::Sent signal to destroy with ID: "+dict.getInt("signalId"));
					return SCRIPT_CONTINUE;
				}
				else
				{
					callable.setCDCallable(self, null);
					sendDirtyObjectMenuNotification(self);
				}
				
			}
			
			if (!vehicle.isInValidUnpackLocation(player))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (callable.getControlDeviceType(self) == callable.CALLABLE_TYPE_RIDEABLE && getMovementPercent(player) == 0.0f)
			{
				sendSystemMessage(player, new string_id("pet/pet_menu", "cant_call_vehicle_rooted"));
				return SCRIPT_CONTINUE;
			}
			
			if (ai_lib.isInCombat(player))
			{
				sendSystemMessage(player, vehicle.SID_NOT_WHILE_IN_COMBAT);
				return SCRIPT_CONTINUE;
			}
			
			if (callable.getControlDeviceType(self) == callable.CALLABLE_TYPE_RIDEABLE && callable.hasCallable(player, callable.CALLABLE_TYPE_FAMILIAR))
			{
				obj_id objCallable = callable.getCallable(player, callable.CALLABLE_TYPE_FAMILIAR);
				
				callable.storeCallable(player, objCallable);
			}
			
			if (callable.getControlDeviceType(self) == callable.CALLABLE_TYPE_RIDEABLE && callable.hasCallable(player, callable.CALLABLE_TYPE_COMBAT_PET))
			{
				obj_id objCallable = callable.getCallable(player, callable.CALLABLE_TYPE_COMBAT_PET);
				
				if (beast_lib.isBeast(objCallable))
				{
					utils.setScriptVar(player, callable.SCRIPTVAR_RIDEABLE_PACKED_CALLABLE, callable.getCallableCD(objCallable));
				}
				
				callable.storeCallable(player, objCallable);
			}
			
			obj_id vehicle = createVehicle(player, self);
			
			if (!isIdValid(vehicle))
			{
				LOG("vehicle-bug", "OnObjectMenuSelect(): failed to create vehicle");
				return SCRIPT_CONTINUE;
			}
			
			return SCRIPT_CONTINUE;
		}
		else if (item == menu_info_types.VEHICLE_STORE)
		{
			if (callable.hasCDCallable(self))
			{
				obj_id currentPet = callable.getCDCallable(self);
				
				if (isIdValid( currentPet ))
				{
					vehicle.storeVehicle(self, player);
					callable.restoreCallable(player);
					return SCRIPT_CONTINUE;
				}
				else
				{
					callable.setCDCallable(self, null);
					
					sendDirtyObjectMenuNotification(self);
					LOG("vehicle-bug", "OnObjectMenuSelect(): vcd id=["+ self + "] selected VEHICLE_STORE but the scriptvar on the VCD is INVALID, setting count to zero anyway.");
				}
			}
			else
			{
				sendDirtyObjectMenuNotification(self);
				LOG("vehicle-bug", "OnObjectMenuSelect(): vcd id=["+ self + "] selected VEHICLE_STORE but the VCD has no scriptvar, setting count to zero anyway.");
			}
			
			return SCRIPT_CONTINUE;
		}
		else
		{
			LOG("", "Radial menu selection not in available list: "+ item);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void destroyCurrentPet(obj_id petControlDevice) throws InterruptedException
	{
		if (callable.hasCDCallable(petControlDevice))
		{
			obj_id currentVehicle = callable.getCDCallable(petControlDevice);
			
			if (isIdValid(currentVehicle))
			{
				
				obj_id datapad = getContainedBy(petControlDevice);
				
				if (!isIdValid(datapad))
				{
					return;
				}
				
				obj_id master = getContainedBy( datapad );
				
				if (!isIdValid(master))
				{
					return;
				}
				
				callable.setCallable(master, null, callable.CALLABLE_TYPE_RIDEABLE);
				
				vehicle.saveVehicleInfo(petControlDevice, currentVehicle);
				messageTo(currentVehicle, "handlePackRequest", null, 1, false);
			}
		}
		
		callable.setCDCallable(petControlDevice, null);
	}
	
	
	public int handleStorePetRequest(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id currentPet = callable.getCDCallable(self);
		
		if (!isIdValid(currentPet) || !exists(currentPet))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = params.getObjId("master");
		
		if (ai_lib.isInCombat(currentPet) || pet_lib.wasInCombatRecently(currentPet, player, true))
		{
			return SCRIPT_CONTINUE;
		}
		
		vehicle.setTimeStored(self);
		destroyCurrentPet(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAboutToReceiveItem(obj_id self, obj_id srcContainer, obj_id transferer, obj_id item) throws InterruptedException
	{
		if (hasScript( item, "ai.pet_control_device" ))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(transferer))
		{
			debugServerConsoleMsg(null, "********** ONABOUTTORECIEVEITEM triggered on the pcd script. We're allowing it, as it appears to come from something that !isIdValid.");
			return SCRIPT_CONTINUE;
		}
		
		debugServerConsoleMsg(null, "********** ONABOUTTORECIEVEITEM triggered on the pcd script. We're NOT allowing it, as it appears to come from something that isIdValid.");
		return SCRIPT_OVERRIDE;
	}
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		if (!hasObjVar( self, "pet.releasingPet" ))
		{
			destroyCurrentPet( self );
		}
		
		obj_id datapad = getContainedBy( self );
		if (!isIdValid( datapad ))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id master = getContainedBy( datapad );
		if (!isIdValid( master ))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar( self, "pet.isDead" ))
		{
			int petType = getIntObjVar( self, "ai.pet.type");
			if (petType == pet_lib.PET_TYPE_NON_AGGRO || petType == pet_lib.PET_TYPE_AGGRO)
			{
				
				sendSystemMessage( master, new string_id ( "pet/pet_menu", "vehicle_released"));
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTransferred(obj_id self, obj_id sourceContainer, obj_id destContainer, obj_id transferer) throws InterruptedException
	{
		obj_id master = getContainedBy( sourceContainer );
		if (!isIdValid( master ))
		{
			return SCRIPT_CONTINUE;
		}
		
		destroyCurrentPet( self );
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		obj_id datapad = getContainedBy( self );
		if (!isIdValid( datapad ))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id master = getContainedBy( datapad );
		if (!isIdValid( master ))
		{
			return SCRIPT_CONTINUE;
		}
		
		sendSystemMessage( master, new string_id( "pet/pet_menu", "device_added"));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		int idx = utils.getValidAttributeIndex(names);
		
		if (idx == -1)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		if (hasObjVar(self, vehicle.VAR_PALVAR_CNT))
		{
			
			names[idx] = "customization_cnt";
			attribs[idx] = Integer.toString(getIntObjVar(self, vehicle.VAR_PALVAR_CNT));
			idx++;
			
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
		}
		
		if (hasObjVar(self, "pet.uses_left"))
		{
			
			names[idx] = "new_player_vehicle_uses_left";
			
			int uses = getIntObjVar(self, "pet.uses_left");
			
			attribs[idx] = " " + uses;
			idx++;
			
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
		}
		
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public boolean isSameFaction(obj_id petControlDevice) throws InterruptedException
	{
		obj_id datapad = getContainedBy( petControlDevice );
		if (!isIdValid( datapad ))
		{
			return false;
		}
		
		obj_id master = getContainedBy( datapad );
		if (!isIdValid( master ))
		{
			return false;
		}
		
		String creatureName = getStringObjVar( petControlDevice, "pet.creatureName");
		if (creatureName == null || creatureName.equals(""))
		{
			return false;
		}
		
		String petFaction = utils.dataTableGetString(CREATURE_TABLE, creatureName, "pvpFaction");
		if (petFaction == null || petFaction.equals(""))
		{
			
			return true;
		}
		
		if (!petFaction.equals("Imperial") && !petFaction.equals("Rebel"))
		{
			
			return true;
		}
		
		if (pvpGetType( master ) != PVPTYPE_DECLARED)
		{
			
			sendSystemMessage( master, new string_id ( pet_lib.MENU_FILE, "not_declared"));
			return false;
		}
		String masterFaction = factions.getFaction( master );
		if (masterFaction == null || !masterFaction.equals(petFaction))
		{
			
			sendSystemMessage( master, new string_id ( pet_lib.MENU_FILE, "not_declared"));
			return false;
		}
		
		return true;
	}
	
	
	public int OnAboutToBeTransferred(obj_id self, obj_id destContainer, obj_id transferer) throws InterruptedException
	{
		obj_id newMaster = getContainedBy( destContainer );
		if (!isPlayer( newMaster ))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (isIdValid( transferer ))
		{
			
			obj_id currentContainer = getContainedBy( self );
			if (isIdValid(currentContainer))
			{
				obj_id currentContOwner = getContainedBy( currentContainer );
				if (isIdValid( currentContOwner ))
				{
					
					if (hasScript( currentContOwner, "ai.pet_control_device" ))
					{
						
						obj_id yourPad = utils.getPlayerDatapad(transferer);
						if (destContainer == yourPad)
						{
							return SCRIPT_CONTINUE;
						}
					}
				}
			}
		}
		
		if (!hasScript( newMaster, "ai.pet_master"))
		{
			attachScript( newMaster, "ai.pet_master");
		}
		
		if (vehicle.hasMaxStoredVehicles( newMaster ))
		{
			sendSystemMessage( newMaster, vehicle.SID_SYS_HAS_MAX_VEHICLE );
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int getLevelFromPetControlDevice(obj_id petControlDevice) throws InterruptedException
	{
		int growthStage = 10;
		if (hasObjVar( petControlDevice, "ai.petAdvance.growthStage" ))
		{
			growthStage = getIntObjVar( petControlDevice, "ai.petAdvance.growthStage");
		}
		
		String creatureName = getStringObjVar( petControlDevice, "pet.creatureName");
		if (creatureName == null)
		{
			return 0;
		}
		
		int maxDiff = create.calcCreatureLevel( creatureName );
		if (maxDiff < 1)
		{
			maxDiff = 1;
		}
		
		if (growthStage < 10)
		{
			maxDiff = (maxDiff/10)*growthStage;
		}
		
		return maxDiff;
	}
	
	
	public int handlePetDeathblow(obj_id self, dictionary params) throws InterruptedException
	{
		int vitality = getIntObjVar( self, "pet.vitality");
		vitality += 2;
		
		if (vitality > pet_lib.MAX_VITALITY_LOSS)
		{
			vitality = pet_lib.MAX_VITALITY_LOSS;
		}
		
		setObjVar( self, "pet.vitality", vitality );
		return SCRIPT_CONTINUE;
	}
	
	
	public void restoreCustomization(obj_id pet, obj_id petControlDevice) throws InterruptedException
	{
		if (hasObjVar(petControlDevice, pet_lib.VAR_PALVAR_BASE))
		{
			obj_var_list ovl = getObjVarList(petControlDevice, pet_lib.VAR_PALVAR_VARS);
			if (ovl != null)
			{
				int numItem = ovl.getNumItems();
				for (int i = 0; i < numItem; i++)
				{
					testAbortScript();
					obj_var ov = ovl.getObjVar(i);
					String var = ov.getName();
					int idx = ov.getIntData();
					hue.setColor(pet, var, idx);
				}
			}
		}
	}
	
	
	public int handleFlagDeadCreature(obj_id self, dictionary params) throws InterruptedException
	{
		setObjVar( self, "pet.isDead", true );
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean petIsDead(obj_id petControlDevice, obj_id player, int petType) throws InterruptedException
	{
		if (hasObjVar( petControlDevice, "pet.isDead" ))
		{
			if (petType == pet_lib.PET_TYPE_NON_AGGRO || petType == pet_lib.PET_TYPE_AGGRO)
			{
				
				removeObjVar( petControlDevice, "pet.isDead");
				return false;
			}
			sendSystemMessage( player, new string_id( "pet/pet_menu", "dead_pet") );
			return true;
		}
		return false;
	}
	
	
	public boolean validatePetStats(obj_id pcd, obj_id player) throws InterruptedException
	{
		
		if (!hasObjVar(pcd, "pet.crafted"))
		{
			return true;
		}
		
		final int MAX_HAM_VALUE = 16000;
		final float MAX_ATK_VALUE = 30.0f;
		final float MAX_HIT_VALUE = 1.0f;
		final int MAX_DAM_VALUE = 1500;
		
		boolean status = true;
		
		if (getIntObjVar(pcd, "creature_attribs." + create.MAXATTRIBNAMES[HEALTH]) > MAX_HAM_VALUE)
		{
			status = false;
		}
		
		if (getIntObjVar(pcd, "creature_attribs." + create.MAXATTRIBNAMES[ACTION]) > MAX_HAM_VALUE)
		{
			status = false;
		}
		
		if (getIntObjVar(pcd, "creature_attribs." + create.MAXATTRIBNAMES[MIND]) > MAX_HAM_VALUE)
		{
			status = false;
		}
		
		if (getFloatObjVar(pcd, "creature_attribs.attackSpeed") > MAX_ATK_VALUE)
		{
			status = false;
		}
		
		if (getFloatObjVar(pcd, "creature_attribs.toHitChance") > MAX_HIT_VALUE)
		{
			status = false;
		}
		
		if (getIntObjVar(pcd, "creature_attribs.minDamage") > MAX_DAM_VALUE)
		{
			status = false;
		}
		
		if (getIntObjVar(pcd, "creature_attribs.maxDamage") > MAX_DAM_VALUE)
		{
			status = false;
		}
		
		if (status == false)
		{
			sendSystemMessage(player, pet_lib.SID_INVALID_CRAFTED_PET);
		}
		
		return status;
	}
	
	
	public int handleValidatePet(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id currentPet = callable.getCDCallable(self);
		
		obj_id calledPet = params.getObjId( "calledPet");
		
		if (!isIdValid( currentPet ))
		{
			
			if (isIdValid( calledPet ))
			{
				if (calledPet.isLoaded())
				{
					destroyObject( calledPet );
				}
				else
				{
					messageTo( calledPet, "handlePackRequest", null, 1, false );
				}
			}
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid( calledPet ))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (calledPet == currentPet)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (isIdValid( calledPet ))
		{
			if (calledPet.isLoaded())
			{
				destroyObject( calledPet );
			}
			else
			{
				messageTo( calledPet, "handlePackRequest", null, 1, false );
			}
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int handleGrowthChoice(obj_id self, dictionary params) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public obj_id createVehicle(obj_id player, obj_id petControlDevice) throws InterruptedException
	{
		debugServerConsoleMsg( player, "+++ VEHICLE_CONTROL_DEVICE . createVehicle +++ Entered PCD/Pet creation function.");
		
		String objectTemplate = vehicle.getVehicleTemplate(petControlDevice);
		
		if (callable.hasCallable(player, callable.CALLABLE_TYPE_RIDEABLE))
		{
			return null;
		}
		
		obj_id playerCurrentMount = getMountId (player);
		
		if (isIdValid(playerCurrentMount) && vehicle.isBattlefieldVehicle(playerCurrentMount))
		{
			string_id noBattlefieldVehicle = new string_id ("pet/pet_menu", "vehicle_battlefield_vehicle_restriction");
			sendSystemMessage (player, noBattlefieldVehicle);
			return null;
		}
		
		int shapechange = buff.getBuffOnTargetFromGroup(player, "shapechange");
		if (shapechange != 0)
		{
			sendSystemMessage(player, new string_id("spam","not_while_shapechanged"));
			return null;
		}
		
		if (objectTemplate.equals("object/mobile/vehicle/jetpack.iff"))
		{
			if (getLocomotion(player) == LOCOMOTION_RUNNING)
			{
				string_id noMove = new string_id ( "pet/pet_menu", "no_moving");
				sendSystemMessage (player, noMove);
				return null;
			}
		}
		
		obj_id pet = createObject(objectTemplate, getLocation(player));
		
		if (!isIdValid(pet))
		{
			debugServerConsoleMsg( player, "+++ VEHICLE_CONTROL_DEVICE . createVehicle +++ OBJ_ID Validity check on Vehicular_Prototype pet object came back as NOT VALID");
			return petControlDevice;
		}
		
		sendDirtyObjectMenuNotification(petControlDevice);
		
		callable.setCallableCD(pet, petControlDevice);
		
		setObjVar(pet, "vehicularTestBed", 1);
		setObjVar(pet, "ai.pet.masterName", getEncodedName(player));
		
		setAttributeAttained(pet, attrib.VEHICLE);
		
		vehicle.reHueVehicle(petControlDevice, pet);
		
		if (!(couldPetBeMadeMountable(pet)==0 ))
		{
			debugServerConsoleMsg( null, "+++ VEHICLE_CONTROL_DEVICE . createVehicle +++ couldPetBeMadeMountable(pet) returned FALSE.");
			return pet;
		}
		
		if (!makePetMountable( pet ))
		{
			debugServerConsoleMsg( null, "+++ VEHICLE_CONTROL_DEVICE . createVehicle +++ makePetMountable(pet) returned FALSE.");
			return pet;
		}
		else
		{
			setObjVar( petControlDevice, "ai.pet.trainedMount", 1 );
			setObjVar( pet, "ai.pet.trainedMount", 1 );
		}
		
		if (hasObjVar( petControlDevice, "attrib.max_hp" ))
		{
			debugServerConsoleMsg( player, "+++ VEHICLE_CONTROL_DEVICE . createVehicle +++ ");
			setMaxHitpoints(pet, getIntObjVar(petControlDevice, "attrib.max_hp"));
		}
		
		if (hasObjVar( petControlDevice, "attrib.hit_points" ))
		{
			setHitpoints(pet, getIntObjVar(petControlDevice, "attrib.hit_points"));
		}
		
		if (hasObjVar(petControlDevice, vehicle.VAR_PALVAR_BASE))
		{
			restoreCustomization(pet, petControlDevice);
			
			if (hasObjVar(petControlDevice, vehicle.VAR_PALVAR_CNT))
			{
				int cnt = getIntObjVar(petControlDevice, vehicle.VAR_PALVAR_CNT);
				
				if (cnt > -1)
				{
					cnt--;
					setObjVar(petControlDevice, vehicle.VAR_PALVAR_CNT, cnt);
					
					if (cnt < 6)
					{
						switch(cnt)
						{
							case 5:
							customizationFadeToWhite(pet, 0.15f);
							sendSystemMessage(player, pet_lib.SID_SYS_VEH_CUST_FADING);
							break;
							case 4:
							customizationFadeToWhite(pet, 0.20f);
							sendSystemMessage(player, pet_lib.SID_SYS_VEH_CUST_FADING);
							break;
							case 3:
							customizationFadeToWhite(pet, 0.25f);
							sendSystemMessage(player, pet_lib.SID_SYS_VEH_CUST_FADING);
							break;
							case 2:
							customizationFadeToWhite(pet, 0.30f);
							sendSystemMessage(player, pet_lib.SID_SYS_VEH_CUST_FADING);
							break;
							case 1:
							customizationFadeToWhite(pet, 0.35f);
							sendSystemMessage(player, pet_lib.SID_SYS_VEH_CUST_FADING);
							break;
							case 0:
							customizationFadeToWhite(pet, 0.40f);
							sendSystemMessage(player, pet_lib.SID_SYS_VEH_CUST_GONE);
							removeObjVar(petControlDevice, vehicle.VAR_PALVAR_CNT);
							break;
						}
					}
				}
			}
		}
		
		if (hasObjVar(petControlDevice, "pet.uses_left"))
		{
			int uses = getIntObjVar(petControlDevice, "pet.uses_left");
			uses--;
			
			if (uses > 0)
			{
				prose_package pp = prose.getPackage(vehicle.SID_SYS_USES_LEFT, uses);
				sendSystemMessageProse(player, pp);
				
			}
			else
			{
				sendSystemMessage(player, vehicle.SID_SYS_USES_LEFT_LAST);
			}
			
			setObjVar(petControlDevice, "pet.uses_left", uses);
		}
		
		messageTo(pet, "handleVehicleDecay", null, 5f, false);
		
		vehicle.initializeVehicle (pet, player);
		
		dictionary web = new dictionary ();
		web.put ("player", player);
		messageTo(pet, "checkForJetpack", web, 1, false);
		
		String vehicleType = getTemplateName (pet);
		
		if (vehicleType.equals("object/mobile/vehicle/jetpack.iff"))
		{
			vehicle.setHoverHeight(pet, 4);
		}
		
		callable.setCallableLinks(player, petControlDevice, pet);
		
		return pet;
	}
	
	
	public void customizationFadeToWhite(obj_id pet, float percent) throws InterruptedException
	{
		
		ranged_int_custom_var[] ricv = hue.getPalcolorVars(pet);
		
		Vector pcv = new Vector();
		pcv.setSize(0);
		
		if (ricv == null || ricv.length == 0)
		{
			return;
		}
		
		for (int i = 0; i < ricv.length; i++)
		{
			testAbortScript();
			
			if (ricv[i].isPalColor())
			{
				
				pcv = utils.addElement(pcv, (palcolor_custom_var)ricv[i]);
			}
		}
		
		if (pcv == null || pcv.size() == 0)
		{
			return;
		}
		
		for (int j = 0; j < pcv.size(); j++)
		{
			testAbortScript();
			
			color old_color = ((palcolor_custom_var)(pcv.get(j))).getSelectedColor();
			
			int old_r = old_color.getR();
			int old_g = old_color.getG();
			int old_b = old_color.getB();
			int old_a = old_color.getA();
			
			float d_r = (255 - old_r) * percent;
			float d_g = (255 - old_g) * percent;
			float d_b = (255 - old_b) * percent;
			float d_a = (255 - old_a) * percent;
			
			int new_r = old_r + (int)d_r;
			int new_g = old_g + (int)d_g;
			int new_b = old_b + (int)d_b;
			int new_a = old_a + (int)d_a;
			
			color new_color = new color(new_r, new_g, new_b, new_a);
			
			((palcolor_custom_var)(pcv.get(j))).setToClosestColor(new_color);
		}
	}
	
	
	public int handleRemoveCurrentVehicle(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id sendingVehicleId = params.getObjId(MESSAGE_VEHICLE_ID);
		
		if (!isIdValid(sendingVehicleId))
		{
			if (debug)
			{
				LOG("vcd-debug", "vehicle_control_device.handleRemoveCurrentVehicle(): vcd id=["+ self + "]: sending vehicle's ID is invalid, ignoring");
			}
			return SCRIPT_CONTINUE;
		}
		
		debugServerConsoleMsg( null, "+++ vehicle_control_device.messageHandler handleRemoveCurrentVehicle +++ entered HANDLEREMOVECURRENTVEHICLE message handler");
		
		if (callable.hasCDCallable(self))
		{
			sendDirtyObjectMenuNotification (self);
			
			obj_id controlledVehicleId = callable.getCDCallable(self);
			
			if (controlledVehicleId == sendingVehicleId)
			{
				
				callable.setCDCallable(self, null);
				
				if (debug)
				{
					LOG("vcd-debug", "vehicle_control_device.handleRemoveCurrentVehicle(): vcd id=["+ self + "]: removing scriptvar requested by vehicle id=["+ sendingVehicleId + "]");
				}
			}
			else
			{
				
				LOG("vcd", "vehicle_control_device.handleRemoveCurrentVehicle(): vcd id=["+ self + "]: ignoring request from vehicle id=["+ sendingVehicleId + "]: VCD thinks it is controlling a different vehicle, id=["+ controlledVehicleId + "]");
			}
		}
		else
		{
			debugServerConsoleMsg( null, "+++ vehicle_control_device.messageHandler handleRemoveCurrentVehicle +++ WHOOPS! no callable.current scriptvar on VCD.");
			debugServerConsoleMsg( null, "+++ vehicle_control_device.messageHandler handleRemoveCurrentVehicle +++ How'd that happen?.");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleStoreVehicleDamage(obj_id self, dictionary params) throws InterruptedException
	{
		int hp = params.getInt("hp");
		int penalty = params.getInt("penalty");
		
		vehicle.storeDamage(self, hp, penalty);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean updateTcgVehicleMaxHealth(obj_id tcgVehiclePcd) throws InterruptedException
	{
		if (!isValidId(tcgVehiclePcd) || !exists(tcgVehiclePcd))
		{
			return false;
		}
		if (!hasObjVar(tcgVehiclePcd, "attrib.max_hp"))
		{
			return false;
		}
		
		int currentHp = 0;
		if (hasObjVar(tcgVehiclePcd, "attrib.hit_points"))
		{
			currentHp = getIntObjVar(tcgVehiclePcd, "attrib.hit_points");
			if (currentHp < 0)
			{
				currentHp = 0;
			}
			else if (currentHp == 0)
			{
				currentHp = TCG_PCD_OLD_NEW_DIFFERENCE;
			}
			else if (currentHp > 0 && currentHp < TCG_PCD_OLD_MAX_HEALTH)
			{
				int damage = TCG_PCD_OLD_MAX_HEALTH - currentHp;
				currentHp = TCG_PCD_MAX_HEALTH - damage;
			}
			else
			{
				currentHp = TCG_PCD_MAX_HEALTH;
			}
		}
		else
		{
			currentHp = TCG_PCD_MAX_HEALTH;
		}
		
		if (currentHp < 0)
		{
			return false;
		}
		
		setObjVar(tcgVehiclePcd, "attrib.max_hp", TCG_PCD_MAX_HEALTH);
		setObjVar(tcgVehiclePcd, "attrib.hit_points", currentHp);
		setObjVar(tcgVehiclePcd, TCG_VEHICLE_PCD_UPDATED, true);
		return true;
	}
	
	
	public int modifyPaletteIndex(int idx) throws InterruptedException
	{
		int series_idx = idx%8;
		if (series_idx == 0)
		{
			return idx;
		}
		
		return idx-1;
	}
	
}
