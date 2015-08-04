package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.utils;
import script.library.factions;
import script.library.trace;
import script.library.pclib;
import script.library.weapons;
import script.library.vehicle;
import script.library.stealth;


public class advanced_turret extends script.base_script
{
	public advanced_turret()
	{
	}
	public static final String ALERT_VOLUME_NAME = "turretAggroVolume";
	public static final float DEFAULT_TURRET_RANGE = 80.0f;
	public static final float RANGE_TOO_CLOSE = 10f;
	public static final String VAR_TURRET_BASE = "turret";
	public static final String IS_ACTIVE = VAR_TURRET_BASE + ".isActive";
	public static final String TARGETS = VAR_TURRET_BASE + ".targetList";
	public static final String TURRET_FRIEND = VAR_TURRET_BASE + ".isFriend";
	public static final String IS_ENGAGED = VAR_TURRET_BASE + ".isTrackingTarget";
	public static final String IS_OUTDOORS = VAR_TURRET_BASE + ".isOutdoors";
	public static final String FACTION = VAR_TURRET_BASE + ".faction";
	public static final String RANGE = VAR_TURRET_BASE + ".range";
	public static final String MY_WEAPON = VAR_TURRET_BASE + ".myWeapon";
	
	public static final int TYPE_BLOCK = 1;
	public static final int TYPE_DISH = 2;
	public static final int TYPE_TOWER = 3;
	
	public static final int SIZE_SMALL = 1;
	public static final int SIZE_MEDIUM = 2;
	public static final int SIZE_LARGE = 3;
	
	
	public static obj_id createTurret(location loc, float yaw, int minDamage, int maxDamage, float attackSpeed, float range) throws InterruptedException
	{
		return createTurret(loc, yaw, TYPE_BLOCK, SIZE_SMALL, DAMAGE_ENERGY, minDamage, maxDamage, range, attackSpeed, "");
	}
	
	
	public static obj_id createTurret(location loc, float yaw, int type, int size, int damageType, int minDamage, int maxDamage, float range, float attackSpeed, String faction) throws InterruptedException
	{
		return createTurret(loc, yaw, TYPE_BLOCK, SIZE_SMALL, DAMAGE_ENERGY, minDamage, maxDamage, 10000, range, attackSpeed, "");
	}
	
	
	public static obj_id createTurret(location loc, float yaw, int type, int size, int damageType, int minDamage, int maxDamage, int hitpoints, float range, float attackSpeed, String faction) throws InterruptedException
	{
		String template = "object/installation/turret/gcw/adv_turret";
		switch(type)
		{
			case TYPE_BLOCK:
			template += "_block";
			break;
			case TYPE_DISH:
			template += "_dish";
			break;
			case TYPE_TOWER:
			template += "_tower";
			break;
		}
		
		switch(size)
		{
			case SIZE_SMALL:
			template += "_sm";
			break;
			case SIZE_MEDIUM:
			if (type == TYPE_DISH)
			{
				template += "_sm";
			}
			else
			{
				template += "_med";
			}
			break;
			case SIZE_LARGE:
			template += "_large";
			break;
		}
		
		switch(damageType)
		{
			
			case DAMAGE_ELEMENTAL_HEAT:
			template += "_heat";
			break;
			case DAMAGE_ELEMENTAL_COLD:
			template += "_cold";
			break;
			case DAMAGE_ELEMENTAL_ACID:
			template += "_acid";
			break;
			case DAMAGE_ELEMENTAL_ELECTRICAL:
			template += "_electricity";
			break;
			case DAMAGE_KINETIC:
			template += "_kinetic";
			break;
			default: 
			template += "_energy";
			break;
		}
		
		template += ".iff";
		
		obj_id turret = createObject(template, loc);
		if (!isIdValid(turret))
		{
			return null;
		}
		
		setYaw(turret, yaw);
		initTurret(turret, faction, damageType, range, minDamage, maxDamage, hitpoints, attackSpeed);
		
		return turret;
	}
	
	
	public static void activateTurret(obj_id turret) throws InterruptedException
	{
		LOG("TURRET", "activateTurret "+ turret);
		if (!isIdValid(turret))
		{
			return;
		}
		
		obj_id turretWeapon = getTurretWeapon(turret);
		if (!isIdValid(turretWeapon))
		{
			return;
		}
		
		utils.setScriptVar(turret, IS_ACTIVE, true);
		setInvulnerable(turret, false);
		pvpSetAttackableOverride(turret, true);
		
		weapons.setWeaponData(turretWeapon);
		float range = getFloatObjVar(turret, RANGE);
		dictionary dict = new dictionary();
		dict.put("range", range);
		messageTo(turret, "createTriggerVolume", dict, 0.0f, false);
		return;
	}
	
	
	public static void deactivateTurret(obj_id turret) throws InterruptedException
	{
		LOG("TURRET", "deactivateTurret "+ turret);
		if (!isIdValid(turret))
		{
			return;
		}
		
		stopTrackingTarget(turret);
		utils.removeScriptVar(turret, IS_ACTIVE);
		setInvulnerable(turret, true);
		
		return;
	}
	
	
	public static boolean isActive(obj_id turret) throws InterruptedException
	{
		if (!isIdValid(turret))
		{
			return false;
		}
		
		if (utils.hasScriptVar(turret, IS_ACTIVE))
		{
			LOG("TURRET", "Turret "+ turret + " is active.");
			return true;
		}
		
		LOG("TURRET", "Turret "+ turret + " is not active.");
		return false;
	}
	
	
	public static void startTrackingTarget(obj_id turret, obj_id target) throws InterruptedException
	{
		LOG("TURRET", "Turret "+ turret + " start tracking target "+ target);
		if (!isIdValid(turret) || !isIdValid(target))
		{
			return;
		}
		
		if (!isActive(turret))
		{
			LOG("TURRET", "Turret "+ turret + " can't track target - turret inactive.");
			return;
		}
		
		utils.setScriptVar(turret, IS_ENGAGED, target);
		
		return;
	}
	
	
	public static void stopTrackingTarget(obj_id turret) throws InterruptedException
	{
		LOG("TURRET", "Turret "+ turret + " stop tracking target.");
		if (!isIdValid(turret))
		{
			return;
		}
		
		utils.removeScriptVar(turret, IS_ENGAGED);
		
		return;
	}
	
	
	public static boolean isEngaged(obj_id turret) throws InterruptedException
	{
		if (!isIdValid(turret))
		{
			return false;
		}
		
		if (utils.hasScriptVar(turret, IS_ENGAGED))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static obj_id getCurrentTarget(obj_id turret) throws InterruptedException
	{
		if (!isEngaged(turret))
		{
			return null;
		}
		
		return utils.getObjIdScriptVar(turret, IS_ENGAGED);
	}
	
	
	public static obj_id getTurretWeapon(obj_id turret) throws InterruptedException
	{
		if (!hasObjVar(turret, MY_WEAPON))
		{
			return null;
		}
		
		return getObjIdObjVar(turret, MY_WEAPON);
	}
	
	
	public static void initTurret(obj_id turret, String faction, int damageType, float range, int minDamage, int maxDamage, int hitpoints, float attackSpeed) throws InterruptedException
	{
		detachScript(turret, "systems.turret.turret_ai");
		attachScript(turret, "systems.turret.advanced_turret_ai");
		
		boolean isOutdoors = getTopMostContainer(turret) == turret;
		setObjVar(turret, IS_OUTDOORS, isOutdoors);
		setObjVar(turret, FACTION, faction.equals("")? "renegade":faction);
		setObjVar(turret, RANGE, range == -1f? DEFAULT_TURRET_RANGE : range);
		factions.setFaction(turret, faction);
		
		setMaxHitpoints(turret, hitpoints);
		setHitpoints(turret, hitpoints);
		setObjVar(turret, "pvpCanAttack", 1);
		setInvulnerable(turret, false);
		
		String weaponTemplate = "object/weapon/ranged/turret/turret_block_large.iff";
		obj_id objWeapon = createObject(weaponTemplate, turret, "");
		if (!isIdValid(objWeapon))
		{
			explodeTurret(turret, turret);
			return;
		}
		
		setObjVar(turret, MY_WEAPON, objWeapon);
		setWeaponAttackSpeed(objWeapon, attackSpeed < 1f? 1f : attackSpeed);
		
		int primaryType = DAMAGE_ENERGY;
		int secondaryType = DAMAGE_ELEMENTAL_HEAT;
		int primaryMinDamage = 0;
		int primaryMaxDamage = 0;
		int secondaryDamage = 0;
		
		switch(damageType)
		{
			case DAMAGE_ELEMENTAL_COLD:
			case DAMAGE_ELEMENTAL_ACID:
			case DAMAGE_ELEMENTAL_ELECTRICAL:
			case DAMAGE_ELEMENTAL_HEAT:
			primaryType = DAMAGE_ENERGY;
			secondaryType = damageType;
			primaryMinDamage = 1;
			primaryMaxDamage = 1;
			secondaryDamage = (int)((float)minDamage + (float)maxDamage / 2.0f);
			break;
			case DAMAGE_KINETIC:
			case DAMAGE_ENERGY:
			primaryType = damageType;
			secondaryType = DAMAGE_ELEMENTAL_HEAT;
			primaryMinDamage = minDamage;
			primaryMaxDamage = maxDamage;
			secondaryDamage = 0;
			break;
		}
		
		setWeaponDamageType(objWeapon, primaryType);
		setWeaponElementalType(objWeapon, secondaryType);
		
		setWeaponMinDamage(objWeapon, minDamage);
		setWeaponMaxDamage(objWeapon, maxDamage);
		setWeaponElementalValue(objWeapon, secondaryDamage);
		
		range_info ri = new range_info();
		ri.maxRange = range;
		setWeaponRangeInfo(objWeapon, ri);
		
		activateTurret(turret);
		return;
	}
	
	
	public static boolean isValidTarget(obj_id turret, obj_id target) throws InterruptedException
	{
		
		if (!isIdValid(turret) || !isIdValid(target) || !exists(turret) || !exists(target))
		{
			doLogging("isValidTarget", "I or "+target+" is not valid or does not exist: "+isIdValid(turret)+"/"+isIdValid(target)+"/"+exists(turret)+"/"+exists(target));
			return false;
		}
		
		if (vehicle.isVehicle(target))
		{
			doLogging("isValidTarget", "Target "+getName(target)+" is a vehicle");
			return false;
		}
		
		if (stealth.hasInvisibleBuff(target))
		{
			doLogging("isValidTarget", "Target "+getName(target)+" has a stealth buff");
			return false;
		}
		
		if (target == turret)
		{
			doLogging("isValidTarget", "I'm attempting to attack myself");
			return false;
		}
		
		location there = getLocation(target);
		location here = getLocation(turret);
		
		float range = hasObjVar(turret, RANGE) ? getFloatObjVar(turret, RANGE) : DEFAULT_TURRET_RANGE;
		
		float dist = getDistance(here, there);
		if (dist > range + 1)
		{
			doLogging("isValidTarget", "Target out of range.");
			return false;
		}
		
		if (utils.hasScriptVar(target, TURRET_FRIEND))
		{
			doLogging("isValidTarget", "Target is a friend.");
			return false;
		}
		
		if (isDead(target) || isIncapacitated(target))
		{
			doLogging("isValidTarget", "Target is dead or incapped");
			return false;
		}
		
		String turretFaction = hasObjVar(turret, FACTION) ? getStringObjVar(turret, FACTION) : getBaseFaction(turret);
		if (!turretFaction.equals("renegade"))
		{
			String targetFaction = factions.getFaction(target);
			if (!factions.areFactionsOpposed(turretFaction, targetFaction))
			{
				doLogging("isValidTarget", "Our factions are not opposed: Self/Target: "+factions.getFaction(turret)+"/"+factions.getFaction(target));
				return false;
			}
		}
		
		return true;
	}
	
	
	public static String getBaseFaction(obj_id turret) throws InterruptedException
	{
		obj_id parentBase = getObjIdObjVar(turret, "hq.defense.parent");
		
		if (!isIdValid(parentBase))
		{
			return "renegade";
		}
		
		return getStringObjVar(parentBase, "faction");
		
	}
	
	
	public static void addTarget(obj_id turret, obj_id target) throws InterruptedException
	{
		if (!isValidTarget(turret, target))
		{
			doLogging("addTarget", ""+getName(target)+"/"+target+" was invalid");
			return;
		}
		
		Vector targets = null;
		if (!utils.hasScriptVar(turret, TARGETS))
		{
			targets = new Vector();
		}
		else
		{
			targets = utils.getResizeableObjIdBatchScriptVar(turret, TARGETS);
		}
		
		if (targets != null && targets.indexOf(target) == -1)
		{
			targets.add(target);
		}
		
		if (targets.size() > 0)
		{
			utils.setBatchScriptVar(turret, TARGETS, targets);
			if (!isEngaged(turret))
			{
				attackPulse(turret);
			}
		}
		
		return;
	}
	
	
	public static void addTargets(obj_id turret, obj_id[] newTargets) throws InterruptedException
	{
		Vector targets = null;
		if (!utils.hasScriptVar(turret, TARGETS))
		{
			targets = new Vector();
		}
		else
		{
			targets = utils.getResizeableObjIdBatchScriptVar(turret, TARGETS);
		}
		
		for (int i = 0; i < newTargets.length; i++)
		{
			testAbortScript();
			if (!isValidTarget(turret, newTargets[i]) || targets.indexOf(newTargets[i]) != -1)
			{
				doLogging("addTargets", ""+getName(newTargets[i])+"/"+newTargets[i]+" was invalid or already in my target array");
				continue;
			}
			
			targets.add(newTargets[i]);
		}
		
		if (targets.size() > 0)
		{
			utils.setBatchScriptVar(turret, TARGETS, targets);
			if (!isEngaged(turret))
			{
				messageTo(turret, "handleTurretAttack", null, 1, false);
			}
		}
		
		return;
	}
	
	
	public static void removeTarget(obj_id turret, obj_id target) throws InterruptedException
	{
		if (!isIdValid(turret) || !isIdValid(target))
		{
			doLogging("removeTarget", "Removed "+target);
			return;
		}
		
		if (!utils.hasScriptVar(turret, TARGETS))
		{
			return;
		}
		
		Vector targets = utils.getResizeableObjIdBatchScriptVar(turret, TARGETS);
		obj_id engageTarget = utils.getObjIdScriptVar(turret, IS_ENGAGED);
		
		int idx = targets.indexOf(target);
		if (idx > -1)
		{
			if (isIdValid(engageTarget) && target == engageTarget)
			{
				stopTrackingTarget(turret);
			}
			
			targets.remove(idx);
			if (targets.size() == 0)
			{
				utils.removeBatchScriptVar(turret, TARGETS);
				
			}
			else
			{
				utils.setBatchScriptVar(turret, TARGETS, targets);
			}
		}
		
		return;
	}
	
	
	public static boolean handleTurretDamage(obj_id turret, obj_id attacker, obj_id weapon, int dmgAmount) throws InterruptedException
	{
		if (!validateAttacker(turret, attacker))
		{
			doLogging("handleTurretDamage", getName(attacker)+"/"+attacker+" was not a valid attacker.");
			return false;
		}
		int curHP = getHitpoints(turret);
		if (curHP < 1)
		{
			explodeTurret(turret, attacker);
			return true;
		}
		
		addTarget(turret, attacker);
		
		if (!utils.hasScriptVar(turret, "playingEffect"))
		{
			int smolder = 2000;
			int fire = 1000;
			
			if (curHP < smolder)
			{
				location death = getLocation(turret);
				utils.setScriptVar(turret, "playingEffect", 1);
				String effect = "";
				if (curHP < fire)
				{
					effect = "clienteffect/lair_hvy_damage_fire.cef";
				}
				else
				{
					effect = "clienteffect/lair_med_damage_smoke.cef";
				}
				
				playClientEffectLoc(attacker, effect , death, 0);
				messageTo(turret, "effectManager", null, 15, true);
			}
		}
		
		return true;
	}
	
	
	public static boolean validateAttacker(obj_id turret, obj_id attacker) throws InterruptedException
	{
		return pvpIsEnemy(turret, attacker);
	}
	
	
	public static void explodeTurret(obj_id turret, obj_id killer) throws InterruptedException
	{
		location death = getLocation(turret);
		deactivateTurret(turret);
		playClientEffectLoc(turret, "clienteffect/combat_explosion_lair_large.cef", death, 0);
		messageTo(turret, "handleDestroyTurret", null, 2, false);
		
		return;
	}
	
	
	public static obj_id getGoodTurretTarget(obj_id turret) throws InterruptedException
	{
		
		obj_id target = getCurrentTarget(turret);
		if (!isValidTarget(turret, target) || isIncapacitated(target) || isDead(target) || !canSee(turret, target))
		{
			if (isIncapacitated(target) && !isDead(target))
			{
				pclib.coupDeGrace(target, turret, false);
			}
			target = null;
		}
		else
		{
			return target;
		}
		
		if (!utils.hasScriptVar(turret, TARGETS))
		{
			return null;
		}
		
		Vector targets = utils.getResizeableObjIdBatchScriptVar(turret, TARGETS);
		if ((targets == null) || (targets.size() == 0))
		{
			doLogging("getGoodTurretTarget", "I had no target batchvar so I am returning null");
			return null;
		}
		
		target = null;
		while (!isIdValid(target))
		{
			testAbortScript();
			if (targets.size() < 1)
			{
				break;
			}
			
			target = (obj_id)targets.get(rand(0, targets.size() - 1));
			if ((!isValidTarget(turret, target) || isDead(target) || isIncapacitated(target) || !canSee(turret, target)|| vehicle.isVehicle(target) || stealth.hasInvisibleBuff(target)))
			{
				targets.removeElement(target);
				target = null;
				continue;
			}
		}
		
		if (targets.size() < 1)
		{
			stopTrackingTarget(turret);
			utils.removeBatchScriptVar(turret, TARGETS);
		}
		else
		{
			utils.setBatchScriptVar(turret, TARGETS, targets);
		}
		return target;
	}
	
	
	public static void attackPulse(obj_id turret) throws InterruptedException
	{
		if (!isActive(turret))
		{
			return;
		}
		
		obj_id target = getGoodTurretTarget(turret);
		if (!isIdValid(target))
		{
			stopTrackingTarget(turret);
			return;
		}
		
		startTrackingTarget(turret, target);
		executeAttackAction(turret, target);
	}
	
	
	public static void executeAttackAction(obj_id turret, obj_id target) throws InterruptedException
	{
		obj_id weapon = getTurretWeapon(turret);
		if (!isIdValid(weapon))
		{
			stopTrackingTarget(turret);
		}
		
		dictionary params = new dictionary();
		params.put("target", target);
		messageTo(turret, "turretShot", params, getWeaponAttackSpeed(weapon), false);
		
		return;
	}
	
	
	public static void doLogging(String section, String message) throws InterruptedException
	{
		LOG("doLogging/advanced_turret.scriptlib/"+section, message);
	}
}
