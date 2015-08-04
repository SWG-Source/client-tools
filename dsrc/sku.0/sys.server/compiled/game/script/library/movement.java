package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.utils;


public class movement extends script.base_script
{
	public movement()
	{
	}
	public static final int MT_BOOST = 1;
	public static final int MT_SNARE = 2;
	public static final int MT_PERMABOOST = 3;
	public static final int MT_PERMASNARE = 4;
	public static final int MT_ROOT = 5;
	public static final int MT_STUN = 6;
	public static final int MT_MEZMERIZE = 7;
	public static final int MT_ALL = 99;
	
	public static final String MOVEMENT_OBJVAR = "movement";
	public static final String MOVEMENT_TABLE = "datatables/movement/movement.iff";
	
	
	public static boolean refresh(obj_id target) throws InterruptedException
	{
		boolean result = false;
		
		if (isIdValid(target))
		{
			float modifier = _recalculateMovementModifiers(target);
			
			if (setMovementPercent(target, modifier))
			{
				result = true;
			}
			else
			{
				LOG("combat", "MOVEMENT MANAGER - ERROR: Could not set movement percent of "+ modifier + " on "+ getName(target) + " ("+ target + ")");
			}
		}
		
		return result;
	}
	
	
	public static boolean canApplyMovementModifier(obj_id target, String name) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		
		if (!isPlayer(target) && !isMob(target))
		{
			return false;
		}
		
		if (!isValidModifier(name))
		{
			return false;
		}
		
		if (!isPlayer(target) && !canAffectOnFoot(name))
		{
			return false;
		}
		
		if (checkForMovementImmunity(target, name))
		{
			return false;
		}
		
		if (hasMovementModifier(target, name))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean applyMovementModifier(obj_id target, String name) throws InterruptedException
	{
		return applyMovementModifier(target, name, -1.0f);
	}
	
	
	public static boolean applyMovementModifier(obj_id target, String name, float strength) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		
		if (!isPlayer(target) && !isMob(target))
		{
			return false;
		}
		
		if (!canApplyMovementModifier(target, name))
		{
			return false;
		}
		
		boolean customStr = true;
		
		if (strength == -1)
		{
			customStr = false;
			strength = getStrength(name);
		}
		
		if (getSkillStatisticModifier(target, "expertise_movement_buff_" + name) != 0)
		{
			customStr = true;
			strength += strength * (getSkillStatisticModifier(target, "expertise_movement_buff_"+ name) / 100.0f);
		}
		
		utils.setScriptVar(target, MOVEMENT_OBJVAR + "."+ name + ".time", getGameTime());
		
		if (customStr)
		{
			utils.setScriptVar(target, MOVEMENT_OBJVAR + "."+ name + ".strength", strength);
		}
		
		final boolean result = refresh(target);
		
		return result;
	}
	
	
	public static boolean removeMovementModifier(obj_id target, String name) throws InterruptedException
	{
		return removeMovementModifier(target, name, true);
	}
	
	
	public static boolean removeMovementModifier(obj_id target, String name, boolean recalculate) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if (!isPlayer(target) && !isMob(target))
		{
			return false;
		}
		if (!hasMovementModifier(target, name))
		{
			debugSpeakMsg(target, "I have no movement modifier, thus I cannot remove one");
			return false;
		}
		
		if (utils.hasScriptVar(target, MOVEMENT_OBJVAR + "." + name + ".time"))
		{
			utils.removeScriptVarTree(target, MOVEMENT_OBJVAR + "."+ name);
		}
		
		combat.removeCombatMovementModifierEffect(target, name);
		
		boolean result = true;
		
		if (recalculate)
		{
			result = refresh(target);
			
		}
		
		return result;
	}
	
	
	public static boolean removeAllModifiers(obj_id target) throws InterruptedException
	{
		String[] mods = getAllModifiers(target);
		
		if (mods == null || mods.length == 0)
		{
			return false;
		}
		
		for (int i = 0; i < mods.length; i++)
		{
			testAbortScript();
			removeMovementModifier(target, mods[i], false);
		}
		
		return refresh(target);
	}
	
	
	public static boolean removeAllModifiersOfType(obj_id target, int type) throws InterruptedException
	{
		String[] mods = getAllModifiers(target);
		
		if (mods == null || mods.length == 0)
		{
			return false;
		}
		
		boolean removed = false;
		for (int i = 0; i < mods.length; i++)
		{
			testAbortScript();
			if (getType(mods[i])==type && buff.canBeDispelled(mods[i]))
			{
				removeMovementModifier(target, mods[i], false);
				removed = true;
			}
		}
		
		if (removed)
		{
			return refresh(target);
		}
		else
		{
			return false;
		}
		
	}
	
	
	public static boolean hasMovementModifier(obj_id target) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		java.util.Enumeration keys = dd.keys();
		while (keys.hasMoreElements())
		{
			testAbortScript();
			String key = (String)(keys.nextElement());
			if (key.startsWith(MOVEMENT_OBJVAR + "."))
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public static boolean hasMovementModifier(obj_id target, String name) throws InterruptedException
	{
		if (utils.hasScriptVar(target, MOVEMENT_OBJVAR + "." + name + ".time"))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static int getType(String name) throws InterruptedException
	{
		return getType(getStringCrc(name.toLowerCase()));
	}
	
	
	public static int getType(int nameCrc) throws InterruptedException
	{
		int row = dataTableSearchColumnForInt(nameCrc, "name", MOVEMENT_TABLE);
		if (row >= 0)
		{
			return dataTableGetInt(MOVEMENT_TABLE, row, "type");
		}
		return MT_ROOT;
	}
	
	
	public static float getStrength(String name) throws InterruptedException
	{
		return getStrength(getStringCrc(name.toLowerCase()));
	}
	
	
	public static float getStrength(int nameCrc) throws InterruptedException
	{
		int row = dataTableSearchColumnForInt(nameCrc, "name", MOVEMENT_TABLE);
		if (row >= 0)
		{
			return dataTableGetFloat(MOVEMENT_TABLE, row, "strength");
		}
		return 0;
	}
	
	
	public static float getCurrentStrength(obj_id target, String name) throws InterruptedException
	{
		if (hasObjVar(target, MOVEMENT_OBJVAR + "." + name + ".strength"))
		{
			return getFloatObjVar(target, MOVEMENT_OBJVAR + "." + name + ".strength");
		}
		
		if (utils.hasScriptVar(target, MOVEMENT_OBJVAR + "." + name + ".strength"))
		{
			return utils.getFloatScriptVar(target, MOVEMENT_OBJVAR + "." + name + ".strength");
		}
		
		return getStrength(name);
	}
	
	
	public static boolean isValidModifier(String name) throws InterruptedException
	{
		return isValidModifier(getStringCrc(name.toLowerCase()));
	}
	
	
	public static boolean isValidModifier(int nameCrc) throws InterruptedException
	{
		int row = dataTableSearchColumnForInt(nameCrc, "name", MOVEMENT_TABLE);
		return (row >= 0);
	}
	
	
	public static boolean isRoot(String name) throws InterruptedException
	{
		return (isRoot(getStringCrc(name.toLowerCase())) || isStunEffect(name));
	}
	
	
	public static boolean isRoot(int nameCrc) throws InterruptedException
	{
		int row = dataTableSearchColumnForInt(nameCrc, "name", MOVEMENT_TABLE);
		if (row >= 0)
		{
			return (dataTableGetInt(MOVEMENT_TABLE, row, "type") == MT_ROOT );
		}
		return false;
	}
	
	
	public static boolean isSnare(String name) throws InterruptedException
	{
		return isSnare(getStringCrc(name.toLowerCase()));
	}
	
	
	public static boolean isSnare(int nameCrc) throws InterruptedException
	{
		int row = dataTableSearchColumnForInt(nameCrc, "name", MOVEMENT_TABLE);
		if (row >= 0)
		{
			int type = dataTableGetInt(MOVEMENT_TABLE, row, "type");
			if (type == MT_SNARE || type == MT_PERMASNARE)
			{
				return true;
			}
		}
		return false;
	}
	
	
	public static boolean isPersisted(String name) throws InterruptedException
	{
		return isPersisted(getStringCrc(name.toLowerCase()));
	}
	
	
	public static boolean isPersisted(int nameCrc) throws InterruptedException
	{
		int row = dataTableSearchColumnForInt(nameCrc, "name", MOVEMENT_TABLE);
		if (row >= 0)
		{
			return (dataTableGetInt(MOVEMENT_TABLE, row, "persistence") == 1);
		}
		return false;
	}
	
	
	public static boolean canAffectOnFoot(String name) throws InterruptedException
	{
		return canAffectOnFoot(getStringCrc(name.toLowerCase()));
	}
	
	
	public static boolean canAffectOnFoot(int nameCrc) throws InterruptedException
	{
		int row = dataTableSearchColumnForInt(nameCrc, "name", MOVEMENT_TABLE);
		if (row >= 0)
		{
			return (dataTableGetInt(MOVEMENT_TABLE, row, "affects_onfoot") == 1);
		}
		return false;
	}
	
	
	public static boolean canAffectVehicles(String name) throws InterruptedException
	{
		return canAffectVehicles(getStringCrc(name.toLowerCase()));
	}
	
	
	public static boolean canAffectVehicles(int nameCrc) throws InterruptedException
	{
		int row = dataTableSearchColumnForInt(nameCrc, "name", MOVEMENT_TABLE);
		if (row >= 0)
		{
			return (dataTableGetInt(MOVEMENT_TABLE, row, "affects_vehicle") == 1);
		}
		return false;
	}
	
	
	public static boolean canAffectMounts(String name) throws InterruptedException
	{
		return canAffectMounts(getStringCrc(name.toLowerCase()));
	}
	
	
	public static boolean canAffectMounts(int nameCrc) throws InterruptedException
	{
		int row = dataTableSearchColumnForInt(nameCrc, "name", MOVEMENT_TABLE);
		if (row >= 0)
		{
			return (dataTableGetInt(MOVEMENT_TABLE, row, "affects_mount") == 1);
		}
		return false;
	}
	
	
	public static boolean isStunEffect(String name) throws InterruptedException
	{
		return dataTableGetInt(MOVEMENT_TABLE, name, "type") == MT_STUN;
	}
	
	
	public static boolean hasStunEffect(obj_id target) throws InterruptedException
	{
		String[] movementMods = getAllModifiers(target);
		
		if (movementMods == null || movementMods.length == 0)
		{
			return false;
		}
		
		for (int i=0; i<movementMods.length; i++)
		{
			testAbortScript();
			if (isStunEffect(movementMods[i]))
			{
				
				return true;
			}
		}
		
		return false;
	}
	
	
	public static String[] getAllStunEffects(obj_id target) throws InterruptedException
	{
		String[] movementMods = getAllModifiers(target);
		
		if (movementMods == null || movementMods.length == 0)
		{
			return null;
		}
		
		Vector effectList = new Vector();
		effectList.setSize(0);
		
		for (int i=0; i<movementMods.length; i++)
		{
			testAbortScript();
			if (isStunEffect(movementMods[i]))
			{
				effectList.add(movementMods[i]);
			}
		}
		
		return utils.toStaticStringArray(effectList);
	}
	
	
	public static String[] getAllModifiers(obj_id target) throws InterruptedException
	{
		Vector mods = new Vector();
		
		deltadictionary dd = target.getScriptVars();
		java.util.Enumeration keys = dd.keys();
		while (keys.hasMoreElements())
		{
			testAbortScript();
			String key = (String)(keys.nextElement());
			if (key.startsWith(MOVEMENT_OBJVAR + ".") && key.endsWith(".time"))
			{
				String[] vars = split(key, '.');
				
				mods.add(vars[1]);
			}
		}
		
		return utils.toStaticStringArray(mods);
	}
	
	
	public static float _getMovementPercentageAdjustment(obj_id target, String name) throws InterruptedException
	{
		int type = getType(name);
		float pct = getCurrentStrength(target, name);
		pct /= 100.0f;
		
		if (type == MT_SNARE || type == MT_PERMASNARE)
		{
			pct *= -1.0f;
		}
		
		return pct;
	}
	
	
	public static float _recalculateMovementModifiers(obj_id target) throws InterruptedException
	{
		float boostMod = 0.0f;
		float snareMod = 0.0f;
		float permaBoost = 0.0f;
		float permaSnare = 1.0f;
		
		String[] mods = getAllModifiers(target);
		
		testStunEffects(target);
		
		if (mods == null || mods.length == 0)
		{
			return 1.0f;
		}
		
		obj_id mount = getMountId(target);
		
		for (int i = 0; i < mods.length; i++)
		{
			testAbortScript();
			if (isPlayer(target) && isIdValid(mount))
			{
				if (hasScript(mount,"systems.vehicle_system.vehicle_base"))
				{
					if (!canAffectVehicles(mods[i]))
					{
						continue;
					}
				}
				else
				{
					if (!canAffectMounts(mods[i]))
					{
						continue;
					}
				}
			}
			else
			{
				if (!canAffectOnFoot(mods[i]))
				{
					continue;
				}
			}
			
			if (isRoot(mods[i]))
			{
				return 0.0f;
			}
			
			float adj = _getMovementPercentageAdjustment(target, mods[i]);
			
			if (adj == Float.NEGATIVE_INFINITY)
			{
				continue;
			}
			
			int type = getType(mods[i]);
			switch(type)
			{
				
				case MT_BOOST:
				if (adj > boostMod)
				{
					boostMod = adj;
				}
				break;
				
				case MT_SNARE:
				if (adj < snareMod)
				{
					snareMod = adj;
				}
				break;
				
				case MT_PERMABOOST:
				permaBoost += adj;
				break;
				
				case MT_PERMASNARE:
				permaSnare *= (1.0f + adj);
				break;
			}
		}
		
		if (snareMod < 0.0)
		{
			boostMod = 0.0f;
		}
		
		float modifier = 1.0f + permaBoost + boostMod;
		
		modifier = modifier * permaSnare * (1.0f + snareMod);
		
		if (modifier < 0.0f)
		{
			modifier = 0.0f;
		}
		
		return modifier;
	}
	
	
	public static boolean checkForMovementImmunity(obj_id target, String name) throws InterruptedException
	{
		
		int modifierType = getType(name);
		
		if (isPlayer(target) && !buff.canBeDispelled(name))
		{
			return false;
		}
		
		boolean resisted = false;
		
		int stunResist = getEnhancedSkillStatisticModifierUncapped(target, "movement_resist_stun");
		int snareResist = getEnhancedSkillStatisticModifierUncapped(target, "movement_resist_snare");
		int rootResist = getEnhancedSkillStatisticModifierUncapped(target, "movement_resist_root");
		
		sendSystemMessageTestingOnly(target, "SNARE RESIST: "+snareResist);
		
		if (modifierType == MT_STUN)
		{
			if (rand(1, 99) < stunResist)
			{
				resisted = true;
			}
		}
		else if (modifierType == MT_SNARE)
		{
			if (rand(1, 99) < snareResist)
			{
				resisted = true;
			}
		}
		else if (modifierType == MT_ROOT)
		{
			if (rand(1, 99) < rootResist)
			{
				resisted = true;
			}
		}
		
		return resisted;
	}
	
	
	public static boolean testStunEffects(obj_id target) throws InterruptedException
	{
		boolean isStunned = hasStunEffect(target);
		
		setState(target, STATE_STUNNED, isStunned);
		
		float turnRate = isStunned ? 0.0f : 360.0f;
		
		return isStunned;
	}
	
	
	public static void updateMovementImmunity(obj_id target) throws InterruptedException
	{
		
		int stunResist = getEnhancedSkillStatisticModifierUncapped(target, "movement_resist_stun");
		int snareResist = getEnhancedSkillStatisticModifierUncapped(target, "movement_resist_snare");
		int rootResist = getEnhancedSkillStatisticModifierUncapped(target, "movement_resist_root");
		sendSystemMessageTestingOnly(target,"SNARE RESIST: "+snareResist);
		
		dictionary dict = new dictionary();
		dict.put("self", target);
		if (stunResist >= 100)
		{
			if (removeAllModifiersOfType(target, movement.MT_STUN))
			{
				string_id cureMovementMsg = new string_id ("spam", "cure_dot");
				dict.put("detriment", "stunned");
				dict.put("msgId", cureMovementMsg);
				messageTo(target, "messageDetrimentalRemoved", dict, 0.0f, false);
			}
		}
		
		if (snareResist >= 100)
		{
			if (removeAllModifiersOfType(target, movement.MT_SNARE))
			{
				string_id cureMovementMsg = new string_id ("spam", "cure_dot");
				dict.put("detriment", "snared");
				dict.put("msgId", cureMovementMsg);
				messageTo(target, "messageDetrimentalRemoved", dict, 0.0f, false);
			}
		}
		
		if (rootResist >= 100)
		{
			if (removeAllModifiersOfType(target, movement.MT_ROOT))
			{
				string_id cureMovementMsg = new string_id ("spam", "cure_dot");
				dict.put("detriment", "rooted");
				dict.put("msgId", cureMovementMsg);
				messageTo(target, "messageDetrimentalRemoved", dict, 0.0f, false);
			}
		}
		
	}
	
	
	public static void performWalk(obj_id actor) throws InterruptedException
	{
		removeObjVar(actor, "run");
		setMovementPercent(actor, 1.0f);
		setMovementWalk(actor);
	}
	
	
	public static void performRun(obj_id actor, String speed) throws InterruptedException
	{
		removeObjVar(actor, "run");
		
		float rate = utils.stringToFloat(speed);
		
		setObjVar(actor, "run", rate);
		
		setMovementPercent(actor, rate);
		setMovementRun(actor);
	}
	
	
	public static void performRun(obj_id actor, float rate) throws InterruptedException
	{
		removeObjVar(actor, "run");
		
		setObjVar(actor, "run", rate);
		
		setMovementPercent(actor, rate);
		setMovementRun(actor);
	}
}
