package script.space.combat;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.space_combat;
import script.library.space_battlefield;
import script.library.space_crafting;
import script.library.pclib;
import script.library.space_utils;
import script.library.space_content;
import script.library.ship_ai;
import script.library.utils;
import script.library.space_transition;
import script.library.space_pilot_command;
import script.library.gcw;


public class combat_ship extends script.base_script
{
	public combat_ship()
	{
	}
	public static final float BROKEN_COMPONENT_DEFAULT_MASS = 50000f;
	public static final float SPACE_YACHT_COMPONENT_DEFAULT_MASS = 0f;
	
	public static final string_id SID_TARGET_DISABLED = new string_id( "space/quest", "target_disabled2");
	
	public static final int DROID_VOCALIZE_REACT_CHANCE = 2;
	
	public static final int SHIP_DAMAGED_SKILLMOD_PENALTY_TIME = 10;
	public static final int SHIP_FIRED_SKILLMOD_PENALTY_TIME = 5;
	
	public static final float STUNNED_COMPONENT_LOOP_TIME = 5.0f;
	
	public static final String NO_DAMAGE_WARN = "clienteffect/cbt_friendlyfire_warn.cef";
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		int[] intSlots = getShipChassisSlots(self);
		for (int intI = 0; intI<intSlots[intI]; intI++)
		{
			testAbortScript();
			
			setShipComponentEfficiencyGeneral(self,intSlots[intI], 1.0f);
			setShipComponentEfficiencyEnergy(self, intSlots[intI], 1.0f);
		}
		messageTo(self, "setupRotationalVelocity", null, 2, false);
		return SCRIPT_CONTINUE;
	}
	
	public int setupRotationalVelocity(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (isShipSlotInstalled(self, space_crafting.ENGINE))
		{
			
			space_crafting.setupChassisDifferentiation(self);
		}
		else
		{
			
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnLogin(obj_id self) throws InterruptedException
	{
		space_combat.clearDeathFlags(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnShipHitByLightning(obj_id self, int frontBack, float damage) throws InterruptedException
	{
		
		if (damage > 0.f)
		{
			notifyShipDamage(self, null, damage);
		}
		
		float fltRemainingDamage = space_combat.doShieldDamage(null, self, space_combat.SHIP, damage, frontBack);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnShipHitByEnvironment(obj_id self, int frontBack, float damage) throws InterruptedException
	{
		
		if (damage > 0.f)
		{
			notifyShipDamage(self, null, damage);
		}
		
		float fltRemainingDamage = space_combat.doShieldDamage(null, self, space_combat.SHIP, damage, frontBack);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnShipWasHit(obj_id self, obj_id objAttacker, int intWeaponIndex, boolean isMissile, int missileType, int intTargetedComponent, boolean fromPlayerAutoTurret, float hitLocationX_o, float hitLocationY_o, float hitLocationZ_o) throws InterruptedException
	{
		
		int intDisabledTime = getIntObjVar(self, "isDisabled");
		if (intDisabledTime > 0)
		{
			
			int intTime = getGameTime();
			
			if (intTime > intDisabledTime)
			{
				
				setObjVar(self, "isDisabled", 0);
			}
			else
			{
				
				return SCRIPT_CONTINUE;
			}
		}
		int intWeaponSlot = intWeaponIndex + ship_chassis_slot_type.SCST_weapon_0;
		
		if (hasObjVar(self, "intInvincible"))
		{
			
			ship_ai.unitAddDamageTaken(self, objAttacker, 1.0f);
			return SCRIPT_CONTINUE;
		}
		
		if (space_utils.isPlayerControlledShip( objAttacker ) && hasObjVar( self, "objMissionOwner" ))
		{
			obj_id objOwner = getObjIdObjVar( self, "objMissionOwner");
			if (isIdValid( objOwner ) && exists( objOwner ))
			{
				boolean absorb = true;
				
				obj_id group_id = getGroupObject( objOwner );
				
				if (isIdValid(group_id))
				{
					
					obj_id[] groupMembers = space_utils.getSpaceGroupMemberIds( group_id );
					if (groupMembers != null)
					{
						for (int i=0; i<groupMembers.length; i++)
						{
							testAbortScript();
							if (objAttacker == space_transition.getContainingShip( groupMembers[i] ))
							{
								
								absorb = false;
								break;
							}
						}
					}
				}
				if (space_transition.getContainingShip( objOwner ) == objAttacker)
				{
					
					absorb = false;
				}
				
				if (absorb)
				{
					
					Vector gunners = space_utils.getGunnersInShip( objAttacker );
					if (gunners == null || gunners.size() == 0)
					{
						playClientEffectObj( getPilotId( objAttacker ), NO_DAMAGE_WARN, getPilotId( objAttacker ), "");
					}
					else
					{
						for (int i=0; i<gunners.size(); i++)
						{
							testAbortScript();
							playClientEffectObj( ((obj_id)(gunners.get(i))), NO_DAMAGE_WARN, ((obj_id)(gunners.get(i))), "");
						}
					}
					return SCRIPT_CONTINUE;
				}
			}
		}
		
		if (space_combat.hasDeathFlags(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar( self, "intNoPlayerDamage" ) && space_utils.isPlayerControlledShip( objAttacker ))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isShipSlotTargetable(self, intTargetedComponent))
		{
			
			intTargetedComponent = space_combat.SHIP;
			
		}
		if (hasScript(self, "space.combat.combat_ship_capital"))
		{
			return SCRIPT_CONTINUE;
			
		}
		if (!pvpCanAttack(objAttacker, self))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		pvpAttackPerformed(objAttacker, self);
		
		if (hasScript(self, "e3demo.spawner_nebulon")||(hasScript(self, "e3demo.nebulon_damaged")))
		{
			return SCRIPT_CONTINUE;
		}
		
		space_combat.checkAndPerformCombatTaunts(objAttacker, self, "fltAttackTauntChance", "hitYou", 0);
		space_combat.checkAndPerformCombatTaunts(self, objAttacker, "fltDefendTauntChance", "gotHit", 0);
		
		obj_id objPilot = getPilotId(objAttacker);
		
		transform attackerTransform_w = getTransform_o2w(objAttacker);
		
		transform defenderTransform_w = getTransform_o2w(self);
		
		vector hitDirection_o = defenderTransform_w.rotateTranslate_p2l(attackerTransform_w.getPosition_p());
		
		int intSide = 0;
		if (hitDirection_o.z < 0.f)
		{
			intSide = 1;
		}
		
		if (utils.checkConfigFlag("ScriptFlags","e3Demo"))
		{
			
			if (space_utils.isPlayerControlledShip(self))
			{
				int intSlot = rand(1, 4);
				int intIntensity = rand(1, 100);
				space_combat.doInteriorDamageNotification(self, intSlot, 100, intIntensity);
				return SCRIPT_CONTINUE;
			}
		}
		float fltDamage = space_combat.getShipWeaponDamage(objAttacker, self, intWeaponSlot);
		
		if (isIdValid(getPilotId(self)))
		{
			if (hasObjVar(getPilotId(self), "intCombatDebug"))
			{
				if (isMissile)
				{
					sendSystemMessageTestingOnly(getPilotId(self), "MISSILED! for "+fltDamage+ "from "+objAttacker+" missile type is "+missileType);
				}
			}
		}
		
		if (fltDamage > 0.f)
		{
			notifyShipDamage(self, objAttacker, fltDamage);
			ship_ai.unitAddDamageTaken(self, objAttacker, fltDamage);
		}
		
		if (space_utils.isPlayerControlledShip(self))
		{
			if (!utils.hasLocalVar(self, "cmd.wasDamagedSkillMod"))
			{
				int time = getGameTime();
				utils.setLocalVar(self, "cmd.wasDamagedSkillMod", SHIP_DAMAGED_SKILLMOD_PENALTY_TIME+time);
			}
		}
		float fltRemainingDamage = space_combat.doShieldDamage(objAttacker, self, intWeaponSlot, fltDamage, intSide);
		
		if (fltRemainingDamage > 0)
		{
			fltRemainingDamage = space_combat.doArmorDamage(objAttacker, self, intWeaponSlot, fltRemainingDamage, intSide);
			
			if (fltRemainingDamage > 0)
			{
				fltRemainingDamage = space_combat.doComponentDamage(objAttacker, self, intWeaponSlot, intTargetedComponent, fltRemainingDamage, intSide);
				
				if (fltRemainingDamage > 0)
				{
					if (rand (1, 10) < DROID_VOCALIZE_REACT_CHANCE)
					{
						if (space_utils.isPlayerControlledShip(self))
						{
							space_combat.flightDroidVocalize(self, 1);
						}
					}
					
					fltRemainingDamage = space_combat.doChassisDamage(objAttacker, self, intWeaponSlot,fltRemainingDamage);
					
					if (fltRemainingDamage > 0)
					{
						setShipCurrentChassisHitPoints(self, 0.0f);
						
						obj_id objDefenderPilot = getPilotId(self);
						if (!space_utils.isPlayerControlledShip(self))
						{
							
							if (space_utils.isPlayerControlledShip(objAttacker)||(hasObjVar(objAttacker, "commanderPlayer")))
							{
								
								if (utils.hasLocalVar(self, "space.give_rewards" ))
								{
									
									utils.setLocalVar(self, "space.give_rewards", 2);
								}
								else
								{
									utils.setLocalVar(self, "space.give_rewards", 1);
								}
								
								space_combat.checkAndPerformCombatTaunts(self, objAttacker, "fltDieTauntChance", "death", 0);
								
								space_combat.targetDestroyed(self);
								return SCRIPT_CONTINUE;
							}
							else
							{
								space_combat.targetDestroyed(self);
								return SCRIPT_CONTINUE;
							}
						}
						else
						{
							
							space_combat.setDeathFlags(self);
							space_combat.sendDestructionNotification(self, objAttacker);
							
							float fltIntensity = rand(0, 1.0f);
							handleShipDestruction(self, fltIntensity);
							
							if (space_utils.isPlayerControlledShip(objAttacker))
							{
								
								obj_id[] crew = space_utils.getAllPlayersInShip(objAttacker);
								gcw.grantSpacePvpKillCredit(getPilotId(self), crew);
								utils.setScriptVar(self, "intPVPKill", 1);
							}
							messageTo(self, "killSpacePlayer", null, 10.0f, true);
							space_combat.doDeathCleanup(self);
							CustomerServiceLog("space_death", "%TU "+self+" Has been killed by "+objAttacker, getOwner(objAttacker));
							if (space_battlefield.isInBattlefield(self))
							{
								CustomerServiceLog("battlefield", "%TU "+self+" Has been killed by "+objAttacker, getOwner(objAttacker));
							}
						}
					}
				}
				
				if (rand (1, 10) < DROID_VOCALIZE_REACT_CHANCE)
				{
					if (space_utils.isPlayerControlledShip(self))
					{
						space_combat.flightDroidVocalize(self, 2);
					}
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int killSpacePlayer(obj_id self, dictionary params) throws InterruptedException
	{
		Vector objPlayers = space_transition.getContainedPlayers(self, null);
		if (objPlayers != null)
		{
			for (int intI = 0; intI< objPlayers.size(); intI++)
			{
				testAbortScript();
				space_combat.strikeBomberCleanup(((obj_id)(objPlayers.get(intI))));
			}
		}
		
		space_combat.killSpacePlayer(self);
		space_combat.clearDeathFlags(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnSpaceUnitEnterCombat(obj_id self, obj_id objTarget) throws InterruptedException
	{
		
		setCondition(self, CONDITION_WINGS_OPENED);
		space_combat.checkAndPerformCombatTaunts(self, objTarget, "fltIntroTauntChance", "entercombat", 0);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		debugServerConsoleMsg( null, "combat_ship.OnInitialize: entered!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		setCondition( self, CONDITION_ON );
		String strChassisType = getShipChassisType(self);
		int[] intSlots = space_crafting.getShipInstalledSlots(self);
		for (int intI = 0; intI< intSlots.length; intI++)
		{
			testAbortScript();
			
			int currentSlotComponentType = ship_chassis_slot_type.getComponentTypeForSlot(intSlots[intI]);
			if (currentSlotComponentType != ship_component_type.SCT_modification)
			{
				float currentComponentMass = getShipComponentMass(self, intSlots[intI]);
				if (strChassisType.equals("player_sorosuub_space_yacht"))
				{
					if (currentComponentMass != 0)
					{
						setShipComponentMass(self, intSlots[intI], SPACE_YACHT_COMPONENT_DEFAULT_MASS);
					}
				}
				else if (currentComponentMass == 0)
				{
					setShipComponentMass(self, intSlots[intI], BROKEN_COMPONENT_DEFAULT_MASS);
				}
			}
			if (intSlots[intI] == space_crafting.ENGINE)
			{
				
				space_crafting.setupChassisDifferentiation(self);
			}
			space_combat.recalculateEfficiency(intSlots[intI],self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		debugSpeakMsg(self, "KABOOM");
		
		obj_id[] notifylist = getObjIdArrayObjVar( self, "destroynotify");
		if (notifylist != null)
		{
			dictionary outparams = new dictionary();
			outparams.put( "object", self );
			for (int i=0; i<notifylist.length; i++)
			{
				testAbortScript();
				if (exists(notifylist[i])&&(notifylist[i].isLoaded()))
				{
					space_utils.notifyObject( notifylist[i], "shipDestroyed", outparams );
				}
			}
		}
		
		if (!space_utils.isPlayerControlledShip(self))
		{
			if (hasObjVar(self, "objParent"))
			{
				space_content.notifySpawner(self);
			}
			
		}
		if (utils.hasLocalVar(self, "space.give_rewards"))
		{
			if (utils.getIntLocalVar(self, "space.give_rewards" ) == 1)
			{
				space_combat.grantRewardsAndCreditForKills(self);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int objectDestroyed(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id objPilot = getPilotId(self);
		if (!space_utils.isPlayerControlledShip(self))
		{
			
			if (hasObjVar(self, "objParent"))
			{
				space_content.notifySpawner(self);
			}
			float fltIntensity = rand(0, 1.0f);
			handleShipDestruction(self, fltIntensity);
		}
		else
		{
			
			float fltIntensity = rand(0, 1.0f);
			handleShipDestruction(self, fltIntensity);
			return SCRIPT_CONTINUE;
			
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int targetDestroyed(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (!space_utils.isPlayerControlledShip(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		Vector objOfficers = space_utils.getShipOfficers(self);
		
		for (int intI = 0; intI < objOfficers.size(); intI++)
		{
			testAbortScript();
			space_utils.notifyObject( ((obj_id)(objOfficers.get(intI))), "targetDestroyed", params );
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int targetDisabled(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id objDefender = params.getObjId( "objDefender");
		
		if (!space_utils.isPlayerControlledShip(self))
		{
			return SCRIPT_CONTINUE;
		}
		Vector objOfficers = space_utils.getShipOfficers(self);
		for (int intI = 0; intI < objOfficers.size(); intI++)
		{
			testAbortScript();
			space_utils.notifyObject( ((obj_id)(objOfficers.get(intI))), "targetDisabled", params );
			space_utils.sendSystemMessageShip(self, SID_TARGET_DISABLED, true, true, true, false);
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int disableSelf(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id objAttacker = params.getObjId("objShip");
		float fltRemainingDamage = space_combat.doComponentDamage(objAttacker, self, 0, ship_chassis_slot_type.SCST_reactor, 500000, 0);
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int megaDamage(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id objAttacker = params.getObjId("objShip");
		if (space_utils.isPlayerControlledShip(objAttacker))
		{
			obj_id objPilot = getPilotId(objAttacker);
			if (isIdValid(objPilot))
			{
				if (!isGod(objPilot))
				{
					if (!utils.hasLocalVar(self, "intEjecting"))
					{
						return SCRIPT_CONTINUE;
					}
					else
					{
						CustomerServiceLog("space_death", "%TU "+objAttacker+" is EJECTING!", getOwner(objAttacker));
						utils.removeLocalVar(self, "intEjecting");
						float fltDamage = getShipMaximumChassisHitPoints(self) * .20f;
						space_combat.doChassisDamage(objAttacker, self, 1, fltDamage);
						
						obj_id objDefenderPilot = getPilotId(self);
						if (!isIdValid(objDefenderPilot))
						{
							objDefenderPilot = getOwner(self);
						}
						space_combat.sendDestructionNotification(self, objAttacker);
						
						float fltIntensity = rand(0, 1.0f);
						handleShipDestruction(self, fltIntensity);
						
						messageTo(self, "killSpacePlayer", null, space_combat.SPACE_DEATH_DELAY, true);
						space_combat.doDeathCleanup(self);
						
					}
				}
			}
		}
		
		float fltDamage = 200000;
		int intSide = 1;
		int intTargetedComponent = 112;
		int intWeaponSlot = space_crafting.WEAPON_0;
		space_combat.doArmorDamage(objAttacker, self, intWeaponSlot, fltDamage, intSide);
		space_combat.doComponentDamage(objAttacker, self, intWeaponSlot, intTargetedComponent, fltDamage, intSide);
		space_combat.doChassisDamage(objAttacker, self, 0, fltDamage);
		
		if (!space_utils.isPlayerControlledShip(self))
		{
			space_combat.grantRewardsAndCreditForKills(self);
			
			space_combat.targetDestroyed(self);
			return SCRIPT_CONTINUE;
		}
		else
		{
			obj_id objDefenderPilot = getPilotId(self);
			if (!isIdValid(objDefenderPilot))
			{
				objDefenderPilot = getOwner(self);
			}
			
			if (space_utils.isPlayerControlledShip(objAttacker))
			{
				
				utils.setScriptVar(self, "intPVPKill", 1);
			}
			
			space_combat.setDeathFlags(self);
			
			dictionary dctParams = new dictionary();
			dctParams.put("objAttacker", objAttacker);
			dctParams.put("objShip", self);
			space_utils.notifyObject(objDefenderPilot, "playerShipDestroyed", dctParams);
			
			float fltIntensity = rand(0, 1.0f);
			handleShipDestruction(self, fltIntensity);
			
			messageTo(self, "killSpacePlayer", null, space_combat.SPACE_DEATH_DELAY, true);
			space_combat.doDeathCleanup(self);
			
			CustomerServiceLog("space_death", "%TU "+self+" Has been killed by "+objAttacker, getOwner(objAttacker));
			if (space_battlefield.isInBattlefield(self))
			{
				CustomerServiceLog("battlefield", "%TU "+self+" Has been killed by "+objAttacker, getOwner(objAttacker));
			}
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTryToEquipDroidControlDeviceInShip(obj_id self, obj_id objPlayer, obj_id objControlDevice) throws InterruptedException
	{
		
		if (!isIdValid(objControlDevice))
		{
			return SCRIPT_CONTINUE;
		}
		if (space_crafting.isUsableAstromechPet(objControlDevice))
		{
			if (space_crafting.isCertifiedForAstromech(objControlDevice, objPlayer))
			{
				if (space_crafting.isUsingCorrectComputer(objControlDevice, self ))
				{
					if (isShipSlotInstalled(self, ship_chassis_slot_type.SCST_droid_interface))
					{
						if (!isShipComponentDisabled(self, ship_chassis_slot_type.SCST_droid_interface))
						{
							associateDroidControlDeviceWithShip(self, objControlDevice);
						}
						else
						{
							string_id strSpam = new string_id("space/space_interaction", "droid_interface_disabled");
							sendSystemMessage(objPlayer, strSpam);
							associateDroidControlDeviceWithShip(self, objControlDevice);
						}
						return SCRIPT_CONTINUE;
					}
					else
					{
						associateDroidControlDeviceWithShip(self, objControlDevice);
						string_id strSpam = new string_id("space/space_interaction", "no_droid_command_module");
						sendSystemMessage(objPlayer, strSpam);
						return SCRIPT_CONTINUE;
					}
				}
				else
				{
					
					if (hasObjVar(objControlDevice, "pet.creatureName" ))
					{
						string_id strSpam = new string_id("space/space_interaction", "need_flight_computer");
						sendSystemMessage(objPlayer, strSpam);
					}
					else
					{
						string_id strSpam = new string_id("space/space_interaction", "need_astromech");
						sendSystemMessage(objPlayer, strSpam);
					}
				}
			}
			else
			{
				string_id strSpam = new string_id("space/space_interaction", "droid_not_certified");
				sendSystemMessage(objPlayer, strSpam);
				return SCRIPT_CONTINUE;
			}
		}
		else
		{
			string_id strSpam = new string_id("space/space_interaction", "not_an_astromech_for_space");
			sendSystemMessage(objPlayer, strSpam);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnShipComponentUninstalling(obj_id self, obj_id uninstallerId, int intSlot, obj_id targetContainer) throws InterruptedException
	{
		
		if (intSlot == ship_component_type.SCT_droid_interface)
		{
			obj_id objDroidControlDevice = getDroidControlDeviceForShip(self);
			if (isIdValid(objDroidControlDevice))
			{
				removeDroidControlDeviceFromShip(self);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnShipComponentUninstalled(obj_id self, obj_id uninstallerId, obj_id componentId, int slot, obj_id targetContainer) throws InterruptedException
	{
		if (slot == ship_chassis_slot_type.SCST_cargo_hold)
		{
			
			removeObjVar(self, "ship_comp.cargo_hold.contents_types");
			removeObjVar(self, "ship_comp.cargo_hold.contents_amounts");
			setObjVar(self, "ship_comp.cargo_hold.contents_current", (int)0);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDroppedItemOntoShipComponent(obj_id self, int intSlot, obj_id objItem, obj_id objPlayer) throws InterruptedException
	{
		
		if (hasObjVar(objItem, "weapon.intAmmoType"))
		{
			
			if (space_crafting.isWeaponAmmo(objItem))
			{
				if (space_crafting.isProperAmmoForWeapon(objItem, self, intSlot))
				{
					space_crafting.applyAmmoToWeapon(self,objItem, intSlot, objPlayer, true);
					
				}
				else
				{
					string_id strSpam = new string_id("space/space_interaction", "no_ammo_allowed");
					sendSystemMessage(objPlayer, strSpam);
					return SCRIPT_CONTINUE;
				}
			}
			else
			{
				string_id strSpam = new string_id("space/space_interaction", "not_missile_ammo");
				sendSystemMessage(objPlayer, strSpam);
				return SCRIPT_CONTINUE;
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int notifyOnDestroy(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id obj = params.getObjId( "object");
		if (!isIdValid( obj ))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] notifyobjs = getObjIdArrayObjVar( self, "destroynotify");
		obj_id[] newnotifyobjs = null;
		if (notifyobjs == null)
		{
			newnotifyobjs = new obj_id[1];
			newnotifyobjs[0] = obj;
		}
		else
		{
			newnotifyobjs = new obj_id[notifyobjs.length+1];
			for (int i=0; i<notifyobjs.length; i++)
			{
				testAbortScript();
				newnotifyobjs[i] = notifyobjs[i];
			}
			newnotifyobjs[notifyobjs.length] = obj;
		}
		setObjVar( self, "destroynotify", newnotifyobjs );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnShipDisabled(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id objAttacker = params.getObjId("objAttacker");
		
		if (hasScript( self, "space.ai.space_ai" ))
		{
			ship_ai.unitIdle( self );
			ship_ai.unitSetAttackOrders( self, ship_ai.ATTACK_ORDERS_HOLD_FIRE );
			
			detachScript( self, "space.ai.space_ai");
			
			dictionary outparams = new dictionary();
			outparams.put( "attacker", objAttacker );
			messageTo( self, "selfDestruct", outparams, 120.f + rand()*60.f, false );
			
			if (!hasObjVar( self, "objMissionOwner" ))
			{
				setObjVar( self, "objMissionOwner", getPilotId( objAttacker ) );
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int selfDestruct(obj_id self, dictionary params) throws InterruptedException
	{
		removeObjVar( self, "objMissionOwner");
		
		if (utils.hasScriptVar( self, "being_docked" ))
		{
			
			messageTo( self, "selfDestruct", params, 30.f, false );
			return SCRIPT_CONTINUE;
		}
		
		setObjVar( self, "selfDestruct", 1 );
		obj_id attacker = params.getObjId( "attacker");
		if (isIdValid(attacker))
		{
			if ((!exists(attacker)||(!attacker.isLoaded())))
			{
				attacker = self;
			}
		}
		space_combat.doChassisDamage( attacker, self, 0, 45000000 );
		space_combat.targetDestroyed( self );
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int reactorPumpPulseTimeout(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id ship = params.getObjId( "ship");
		int pumpPulseLoops = params.getInt( "loops");
		obj_id pilot = params.getObjId( "pilot");
		
		debugServerConsoleMsg( null, "+++ SPACE COMMAND . combat_ship.reactorPumpPulseTimeout +++ ARRIVED in messagehandler. ObjID of ship was: "+ship+" objID of pilot was: "+pilot+" number of loops was: "+pumpPulseLoops );
		
		if (pumpPulseLoops > 1)
		{
			
			string_id strSpam = new string_id("space/space_interaction", "power_spike");
			space_utils.sendSystemMessageShip(self, strSpam, true, false, true, true);
			
			--pumpPulseLoops;
			params.put( "ship", ship );
			params.put( "loops", pumpPulseLoops );
			params.put( "pilot", pilot );
			
			messageTo( self, "reactorPumpPulseTimeout", params, 5.0f, false );
		}
		else if (pumpPulseLoops == 1)
		{
			string_id strSpam = new string_id("space/space_interaction", "reactor_normalizing");
			space_utils.sendSystemMessageShip(self, strSpam, true, false, true, true);
			
			--pumpPulseLoops;
			params.put( "ship", ship );
			params.put( "loops", pumpPulseLoops );
			params.put( "pilot", pilot );
			messageTo( self, "reactorPumpPulseTimeout", params, 5.0f, false );
		}
		else
		{
			if (isIdValid(ship))
			{
				string_id strSpam = new string_id("space/space_interaction", "reactor_stabilized");
				space_utils.sendSystemMessageShip(self, strSpam, true, false, true, true);
				
				space_pilot_command.allPurposeShipComponentReset(ship);
				
				utils.removeScriptVar(pilot, "cmd.reactorPumpPulse");
			}
			else
			{
				debugServerConsoleMsg( null, "+++ MH reactorPumpPulseTimeout . obj_id of the ship passed into the reactor reset function doesn't come back as valid. What the!?.");
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int unScramReactor(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id ship = params.getObjId( "ship");
		int scramLoops = params.getInt( "loops");
		obj_id pilot = params.getObjId( "pilot");
		
		if (scramLoops > 1)
		{
			
			string_id strSpam = new string_id("space/space_interaction", "reactor_standby");
			space_utils.sendSystemMessageShip(self, strSpam, true, false, true, true);
			
			String cefPlayBackHardpoint = space_combat.targetHardpointForCefPlayback(ship);
			playClientEffectObj(pilot, "clienteffect/space_command/scram_reactor_shutdown_alarm.cef", ship, cefPlayBackHardpoint);
			
			--scramLoops;
			params.put( "ship", ship );
			params.put( "loops", scramLoops );
			params.put( "pilot", pilot );
			messageTo( self, "unScramReactor", params, 5.0f, false );
		}
		else if (scramLoops == 1)
		{
			
			string_id strSpam = new string_id("space/space_interaction", "beginning_reactor_restart");
			space_utils.sendSystemMessageShip(self, strSpam, true, false, true, true);
			
			String cefPlayBackHardpoint = space_combat.targetHardpointForCefPlayback(ship);
			playClientEffectObj(pilot, "clienteffect/space_command/scram_reactor_startup_engine.cef", ship, cefPlayBackHardpoint);
			
			--scramLoops;
			params.put( "ship", ship );
			params.put( "loops", scramLoops );
			params.put( "pilot", pilot );
			messageTo( self, "unScramReactor", params, 8.0f, false );
		}
		else
		{
			space_pilot_command.allPurposeShipComponentReset(ship);
			utils.removeScriptVar(pilot, "cmd.reactorPumpPulse");
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnShipComponentPowerSufficient(obj_id self, int intSlot, float fltPowerReceived) throws InterruptedException
	{
		obj_id objPilot = getPilotId(self);
		float fltCurrentHitPoints = getShipComponentHitpointsCurrent(self, intSlot);
		
		if (fltCurrentHitPoints > 0)
		{
			space_utils.setComponentDisabled(self, intSlot, false);
			space_combat.recalculateEfficiency(intSlot, self);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnShipComponentPowerInsufficient(obj_id self, int intSlot, float fltPowerRequired, float fltPowerReceived) throws InterruptedException
	{
		if (fltPowerRequired == 0)
		{
			fltPowerRequired = 1.0f;
		}
		float fltTest = fltPowerReceived / fltPowerRequired;
		
		if (fltTest < space_combat.MINIMUM_EFFICIENCY)
		{
			space_utils.setComponentDisabled(self, intSlot, true);
			setShipComponentDisabledNeedsPower(self, intSlot, true);
		}
		else
		{
			space_combat.recalculateEfficiencyGeneral(intSlot, self, fltTest);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnShipComponentInstalling(obj_id self, obj_id installerId, obj_id componentId, int slot) throws InterruptedException
	{
		obj_id owner = getOwner (self);
		if (isIdValid(owner))
		{
			if (!hasCertificationsForItem (owner, self))
			{
				
				string_id strSpam = new string_id("space/space_interaction", "certification_ship_none");
				space_utils.sendSystemMessageShip(self, strSpam, true, false, true, true);
				
				return SCRIPT_OVERRIDE;
			}
			else
			{
				if (!hasCertificationsForItem (owner, componentId))
				{
					
					string_id strSpam = new string_id("space/space_interaction", "certification_ordnance_none");
					space_utils.sendSystemMessageShip(self, strSpam, true, false, true, true);
					return SCRIPT_OVERRIDE;
				}
			}
			if (!isGameObjectTypeOf (componentId, GOT_ship_component_modification))
			{
				if (hasObjVar(componentId, "ship_comp.mass"))
				{
					float componentMass = space_crafting.getComponentMass(componentId);
					if (componentMass == 0)
					{
						string_id strSpam = new string_id("space/space_interaction", "installing_zero_mass_component");
						space_utils.sendSystemMessageShip(self, strSpam, true, false, true, true);
						return SCRIPT_OVERRIDE;
					}
				}
			}
			
		}
		else
		{
			debugServerConsoleMsg( null, "+++ COMBAT_SHIP.OnShipComponentInstalling +++ Unable to find ships owner, so cannot check certifications. What the!?.");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnShipComponentInstalled(obj_id self, obj_id objInstaller, int intSlot) throws InterruptedException
	{
		if (intSlot == space_crafting.ENGINE)
		{
			
			space_crafting.setupChassisDifferentiation(self);
			
		}
		space_pilot_command.allPurposeShipComponentReset(self);
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int OnShipFiredCountermeasure(obj_id self, int intWeaponIndex, obj_id objPlayer) throws InterruptedException
	{
		int intSlot = intWeaponIndex + ship_chassis_slot_type.SCST_weapon_first;
		
		float fltMinDefense = getShipWeaponDamageMaximum(self, intSlot);
		float fltMaxDefense = getShipWeaponDamageMaximum(self, intSlot);
		float fltRoll = rand(fltMinDefense, fltMaxDefense);
		
		int intMissile = getNearestUnlockedMissileForTarget(self);
		
		if (intMissile == 0)
		{
			launchCountermeasure(self, 0, false, 0);
		}
		else
		{
			
			if (fltRoll>getMissileDefenseRoll(intMissile))
			{
				
				launchCountermeasure(self, intMissile, true, 0);
				
			}
			else
			{
				
				launchCountermeasure(self, intMissile, false, 0);
				
			}
		}
		
		applyFiredWeaponsSkillMod(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnShipFiredMissile(obj_id self, int intMissileId, int intWeaponIndex, int intMissileType, obj_id objPilot, obj_id objDefender, int intTargetedSlot) throws InterruptedException
	{
		int intSlot = intWeaponIndex + ship_chassis_slot_type.SCST_weapon_first;
		
		if (isIdValid(objPilot))
		{
			if (utils.checkConfigFlag("ScriptFlags","e3Demo"))
			{
				setShipWeaponAmmoCurrent(self, intSlot,getShipWeaponAmmoCurrent(self, intSlot));
				return SCRIPT_CONTINUE;
			}
			
			applyFiredWeaponsSkillMod(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int getMissileDefenseRoll(int intMissileId) throws InterruptedException
	{
		
		int intMissileType = getTypeByMissile(intMissileId);
		
		dictionary dctRow = dataTableGetRow("datatables/space/missiles.iff", intMissileType);
		
		return(dctRow.getInt("intCountermeasureDifficulty"));
	}
	
	
	public int flightDroidVocalize(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id ship = params.getObjId( "ship");
		int vocalizePriority = params.getInt( "vocalizePriority");
		
		if (space_utils.isPlayerControlledShip(self))
		{
			space_combat.flightDroidVocalize(ship, vocalizePriority);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int emergencyPowerTimeout(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id ship = params.getObjId( "ship");
		obj_id pilot = params.getObjId( "pilot");
		float emergencyPowerTime = params.getFloat( "emergencyPowerTime");
		
		if (!isIdValid(ship))
		{
			debugServerConsoleMsg( null, "+++ MH emergencyPowerTimeout . obj_id of the ship passed into the emergency power timeout function doesn't come back as valid. What the!?.");
			return SCRIPT_CONTINUE;
		}
		
		string_id strSpam = new string_id("space/space_interaction", "emergency_reset");
		space_utils.sendSystemMessageShip(self, strSpam, true, false, true, true);
		
		String cefPlayBackHardpoint = space_combat.targetHardpointForCefPlayback(ship);
		playClientEffectObj(self, "clienteffect/space_command/emergency_power_off.cef", ship, cefPlayBackHardpoint);
		
		space_pilot_command.allPurposeShipComponentReset(ship);
		
		if (utils.hasLocalVar(ship, "cmd.emergWeapon"))
		{
			utils.removeLocalVar(ship, "cmd.emergWeapon");
		}
		if (utils.hasLocalVar(ship, "cmd.emergShields"))
		{
			utils.removeLocalVar(ship, "cmd.emergShields");
		}
		if (utils.hasLocalVar(ship, "cmd.emergThrust"))
		{
			utils.removeLocalVar(ship, "cmd.emergThrust");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void applyFiredWeaponsSkillMod(obj_id ship) throws InterruptedException
	{
		if (!utils.hasLocalVar(ship, "cmd.firedWeaponsSkillMod"))
		{
			int time = getGameTime();
			utils.setLocalVar(ship, "cmd.firedWeaponsSkillMod", SHIP_FIRED_SKILLMOD_PENALTY_TIME+time);
		}
		return;
	}
	
	
	public int componentsStunned(obj_id self, dictionary params) throws InterruptedException
	{
		Vector stunnedComponents = params.getResizeableIntArray( "stunned_components");
		int stunDuration = params.getInt( "stun_loops");
		obj_id pilot = null;
		String cefPlayBackHardpoint = space_combat.targetHardpointForCefPlayback(self);
		
		boolean boolPlayerShip = false;
		if (!space_utils.isPlayerControlledShip(self))
		{
			boolPlayerShip = true;
		}
		if (stunDuration > 20)
		{
			if (boolPlayerShip)
			{
				
				playClientEffectObj(self, "clienteffect/space_command/cbt_impact_emp_hvy.cef", self, "");
				
				if (stunnedComponents.size() > 1)
				{
					string_id strSpam = new string_id("space/space_pilot_command", "multiple_systems_disrupted");
					space_utils.sendSystemMessageShip(self, strSpam, true, false, true, true);
				}
				else
				{
					
					string_id strSpam = new string_id("space/space_pilot_command", "system_disrupted");
					space_utils.sendSystemMessageShip(self, strSpam, true, false, true, true);
				}
			}
			else
			{
				playClientEffectObj(self, "clienteffect/space_command/cbt_impact_emp_hvy_noshake.cef", self, "");
			}
		}
		else if (stunDuration < 20 && stunDuration > 0)
		{
			if (boolPlayerShip)
			{
				
				string_id strSpam = new string_id("space/space_pilot_command", "disrupted_standby");
				space_utils.sendSystemMessageShip(self, strSpam, true, false, true, true);
			}
		}
		else
		{
			if (boolPlayerShip)
			{
				string_id strSpam = new string_id("space/space_pilot_command", "sub_system_restart");
				space_utils.sendSystemMessageShip(self, strSpam, true, false, true, true);
			}
			
			for (int i = 0; i < stunnedComponents.size(); i++)
			{
				testAbortScript();
				space_utils.setComponentDisabled(self,((Integer)(stunnedComponents.get(i))).intValue(),false);
				space_combat.recalculateEfficiency(((Integer)(stunnedComponents.get(i))).intValue(), self);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		stunDuration--;
		
		params.put( "stunned_components", stunnedComponents );
		params.put( "stun_loops", stunDuration );
		messageTo( self, "componentsStunned", params, STUNNED_COMPONENT_LOOP_TIME, false );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int vRepairDamageCEFLoop(obj_id self, dictionary params) throws InterruptedException
	{
		int damageLoops = params.getInt( "damage_loops");
		obj_id pilot = params.getObjId( "pilot");
		
		debugServerConsoleMsg(null, "vRepairDamageCEFLoop ********** just entered message handler. Recieved number of loops of: "+damageLoops+" and pilot objId of: "+pilot);
		
		String cefPlayBackHardpoint = space_combat.targetHardpointForCefPlayback(self);
		
		String clientEffect = space_pilot_command.randomWeldingCEFPicker();
		debugServerConsoleMsg(null, "vRepairDamageCEFLoop ********** cef chosen for playback is "+clientEffect);
		
		transform t = new transform();
		t = t.move_p(new vector(0.0f, 1.7f, 0.0f));
		
		debugServerConsoleMsg(null, "vRepairDamageCEFLoop ********** DID CRAZY TRANSFORM STUFF. HERE COMES THE EFFECT!!! ");
		
		if (!playClientEffectObj(pilot, clientEffect, self, null, t))
		{
			debugServerConsoleMsg(null, "vRepairDamageCEFLoop ********** FAILED TO PLAYBACK CEF ");
		}
		
		damageLoops--;
		
		if (damageLoops > 0)
		{
			params.put( "damage_loops", damageLoops );
			params.put( "pilot", pilot );
			messageTo( self, "vRepairDamageCEFLoop", params, 3.0f, false );
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int OnSpaceUnitDocked(obj_id self, obj_id target) throws InterruptedException
	{
		obj_id objPilot = getPilotId( self );
		if (!space_utils.isPlayerControlledShip(self))
		{
			return SCRIPT_CONTINUE;
		}
		dictionary outparams = new dictionary();
		outparams.put( "target", target );
		space_utils.notifyObject( objPilot, "spaceUnitDocked", outparams );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnSpaceUnitUnDocked(obj_id self, obj_id target, boolean dockSuccessful) throws InterruptedException
	{
		obj_id objPilot = getPilotId( self );
		if (!space_utils.isPlayerControlledShip(self))
		{
			return SCRIPT_CONTINUE;
		}
		dictionary outparams = new dictionary();
		outparams.put( "target", target );
		
		if (dockSuccessful)
		{
			space_utils.notifyObject( objPilot, "spaceUnitUnDocked", outparams );
		}
		else
		{
			space_utils.notifyObject( objPilot, "spaceUnitDockingFailed", outparams );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int openComm(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id objStation = params.getObjId("objStation");
		obj_id objPilot = getPilotId(self);
		
		queueCommand(objPilot, (80588750), objStation, "0 ", COMMAND_PRIORITY_FRONT);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnSpeaking(obj_id self, String strText) throws InterruptedException
	{
		obj_id objPilot = getPilotId(self);
		if (isIdValid(objPilot))
		{
			
			if (isGod(objPilot)||(utils.checkConfigFlag("scriptFlags", "e3Demo")))
			{
				Object[] newParams = new Object[2];
				newParams[0] = objPilot;
				newParams[1] = strText;
				space_utils.callTrigger("OnSpeaking", newParams );
			}
			
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int checkSpacePVPStatus(obj_id self, dictionary params) throws InterruptedException
	{
		space_transition.updatePVPStatus(self);
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int destroySelf(obj_id self, dictionary params) throws InterruptedException
	{
		setObjVar(self, "intCleaningUp", 1);
		destroyObject(self);
		return SCRIPT_CONTINUE;
		
	}
}
