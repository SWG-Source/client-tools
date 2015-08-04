/**************************************************************************
 *
 * Java-side combat processing.
 *
 * copyright 2001 Sony Online Entertainment
 *
 * @todo: where do error messages from this class go? Right now we're just
 * printing to stderr, but do we want to hook in with our logging code?
 *
 *************************************************************************/

package script;

import script.location;
import script.obj_id;
import script.library.combat_consts;
import script.library.utils;
import script.base_class;
import java.util.Hashtable;

public class combat_engine
{
	/**
	 * Combat constants
	 */

	private static Hashtable m_CombatDataCache = new Hashtable();
	private static Hashtable m_BuffDataCache = new Hashtable();

	public static final int OUT_OF_RANGE = Integer.MIN_VALUE;
	private static float MAX_MELEE_TARGET_RANGE         = -1.0f;
	private static float DEFAULT_MAX_MELEE_TARGET_RANGE = 24.0f;

	private static float MAX_COMBAT_RANGE               = -1.0f;

	// get methods for config setting variables
	public static float getMaxMeleeTargetRange() throws InterruptedException
	{
		if (MAX_MELEE_TARGET_RANGE > 0)
			return MAX_MELEE_TARGET_RANGE;

		float error = utils.stringToFloat("");
		float tmp = utils.stringToFloat(base_class.getConfigSetting("GameServer", "maxMeleeTargetRange"));

		if (tmp == error)
			MAX_MELEE_TARGET_RANGE = DEFAULT_MAX_MELEE_TARGET_RANGE;
		else
			MAX_MELEE_TARGET_RANGE = tmp;

		return MAX_MELEE_TARGET_RANGE;
	}

	public static float getMaxCombatRange() throws InterruptedException
	{
		if (MAX_COMBAT_RANGE < 0)
		{
			MAX_COMBAT_RANGE = utils.stringToFloat(base_class.getConfigSetting("GameServer", "maxCombatRange"));
		}

		return MAX_COMBAT_RANGE;
	}

	/*********************************************************************/

		public static combat_data getCombatData(String actionName)
		{
			if(m_CombatDataCache.containsKey(actionName))
			{
				return combat_data.clone((combat_data)m_CombatDataCache.get(actionName));
			}

			combat_data dat = loadCombatData(actionName);
			if(dat != null)
			{
				m_CombatDataCache.put(actionName, dat);
			}
			else
			{
				return null;
			}

			return combat_data.clone(dat);
		}

		private static combat_data loadCombatData(String actionName)
		{
			combat_data dat = new combat_data();
			dictionary d = base_class.dataTableGetRow("datatables/combat/combat_data.iff", actionName);
			dictionary dc = base_class.dataTableGetRow("datatables/command/command_table.iff", actionName);
			if(d == null || dc == null)
			{
				return null;
			}

			dat.actionName 						= actionName;
			dat.minInvisLevelRequired 			= d.getInt("minInvisLevelRequired");
			dat.commandType 					= d.getInt("commandType");
			dat.validTarget 					= d.getInt("validTarget");
			dat.hitType 						= d.getInt("hitType");
			dat.effectOnTarget					= d.getString("effectOnTarget");
			dat.setCombatTarget 				= d.getInt("setCombatTarget");
			dat.delayAttackEggTemplate 			= d.getString("delayAttackEggTemplate");
			dat.delayAttackParticle 			= d.getString("delayAttackParticle");
			dat.initialDelayAttackInterval 		= d.getFloat("initialDelayAttackInterval");
			dat.delayAttackInterval 			= d.getFloat("delayAttackInterval");
			dat.delayAttackEggPosition 			= d.getInt("delayAttackEggPosition");
			dat.delayAttackLoops				= d.getInt("delayAttackLoops");
			dat.validEggTarget 					= d.getInt("validEggTarget");
			dat.attackType 						= d.getInt("attackType");
			dat.coneLength 						= d.getFloat("coneLength");
			dat.coneWidth 						= d.getFloat("coneWidth");
			dat.minRange 						= d.getFloat("minRange");
			dat.maxRange 						= d.getFloat("maxRange");
			dat.addedDamage 					= d.getInt("addedDamage");
			dat.percentAddFromWeapon 			= d.getFloat("percentAddFromWeapon");
			dat.hateDamageModifier 				= d.getFloat("hateDamageModifier");
			dat.maxHate 						= d.getInt("maxHate");
			dat.hateAdd 						= d.getInt("hateAdd");
			dat.hateAddTime 					= d.getInt("hateAddTime");
			dat.hateReduce						= d.getInt("hateReduce");
			dat.actionCost 						= d.getFloat("actionCost");
			dat.vigorCost 						= d.getFloat("vigorCost");
			dat.scaleByDefenderWeaponSpeed				= d.getFloat("scaleByDefenderWeaponSpeed");
			dat.dotType 						= d.getString("dotType");
			dat.dotIntensity 					= d.getInt("dotIntensity");
			dat.dotDuration 					= d.getInt("dotDuration");
			dat.buffNameTarget 					= d.getString("buffNameTarget");
			dat.buffStrengthTarget 				= d.getFloat("buffStrengthTarget");
			dat.buffDurationTarget 				= d.getFloat("buffDurationTarget");
			dat.buffNameSelf 					= d.getString("buffNameSelf");
			dat.buffStrengthSelf 				= d.getFloat("buffStrengthSelf");
			dat.buffDurationSelf 				= d.getFloat("buffDurationSelf");
			dat.canBePunishing					= d.getInt("canBePunishing");
			dat.increaseCritical				= d.getFloat("increaseCritical");
			dat.increaseStrikethrough			= d.getFloat("increaseStrikethrough");
			dat.reduceGlancing					= d.getFloat("reduceGlancing");
			dat.reduceParry						= d.getFloat("reduceParry");
			dat.reduceBlock						= d.getFloat("reduceBlock");
			dat.reduceDodge						= d.getFloat("reduceDodge");
			dat.overloadWeapon 					= d.getInt("overloadWeapon");
			dat.overloadWeaponMinDamage 		= d.getInt("minDamage");
			dat.overloadWeaponMaxDamage 		= d.getInt("maxDamage");
			dat.overloadWeaponMinRange 			= d.getFloat("minRange"); // this is shared with the command's min range
			dat.overloadWeaponMaxRange 			= d.getFloat("maxRangeOverloaded"); // this is shared with the command's max range
			dat.overloadWeaponType 				= d.getInt("weaponType");
			dat.overloadWeaponCategory 			= d.getInt("weaponCategory");
			dat.overloadWeaponDamageType 		= d.getInt("damageType");
			dat.overloadWeaponElementalType 	= d.getInt("elementalType");
			dat.overloadWeaponElementalValue 	= d.getInt("elementalValue");
			dat.overloadWeaponAttackSpeed 		= d.getFloat("attackSpeed");
			dat.overloadWeaponDamageRadius 		= d.getFloat("damageRadius");
			dat.specialLine 					= d.getString("specialLine");
			dat.animDefault						= d.getString("animDefault");
			dat.intRightFoot					= d.getInt("intRightFoot");
			dat.intLeftFoot						= d.getInt("intLeftFoot");
			dat.intRightHand					= d.getInt("intRightHand");
			dat.intLeftHand						= d.getInt("intLeftHand");
			dat.intWeapon						= d.getInt("intWeapon");
			dat.anim_unarmed					= d.getString("anim_unarmed");
			dat.anim_onehandmelee				= d.getString("anim_onehandmelee");
			dat.anim_twohandmelee				= d.getString("anim_twohandmelee");
			dat.anim_polearm					= d.getString("anim_polearm");
			dat.anim_pistol						= d.getString("anim_pistol");
			dat.anim_lightRifle					= d.getString("anim_lightRifle");
			dat.anim_carbine					= d.getString("anim_carbine");
			dat.anim_rifle						= d.getString("anim_rifle");
			dat.anim_heavyweapon				= d.getString("anim_heavyweapon");
			dat.anim_thrown						= d.getString("anim_thrown");
			dat.anim_onehandlightsaber			= d.getString("anim_onehandlightsaber");
			dat.anim_twohandlightsaber			= d.getString("anim_twohandlightsaber");
			dat.anim_polearmlightsaber			= d.getString("anim_polearmlightsaber");
			dat.validWeapon						= dc.getInt("validWeapon");
			dat.invalidWeapon					= dc.getInt("invalidWeapon");
			dat.cooldownGroup					= dc.getString("cooldownGroup");
			dat.warmupTime						= dc.getFloat("warmupTime");
			dat.executeTime						= dc.getFloat("executeTime");
			dat.cooldownTime					= dc.getFloat("cooldownTime");
			dat.attribute						= d.getInt("healAttrib");
			dat.bypassArmor						= d.getFloat("bypassArmor");
			dat.convertDamageToHealth			= d.getFloat("convertDamageToHealth");
			dat.ignore_distance					= d.getInt("ignore_distance");
			dat.pvp_only						= d.getInt("pvp_only");
			dat.attack_rolls					= d.getInt("attack_rolls");
			dat.flatActionDamage				= d.getInt("flatActionDamage");

			dat.performanceSpamStrId			= new string_id("cbt_spam", d.getString("performance_spam"));
			dat.hitSpam 						= d.getInt("hit_spam");

			return dat;
		}


		//Returns combat data in dictionary form. Need to to preserve data through delayed attacks.
		public static dictionary getCombatDataDictionary(String actionName)
		{
			return (getCombatDataDictionary(getCombatData(actionName)));
		}

		public static dictionary getCombatDataDictionary(combat_data actionData)
		{
			dictionary dict = new dictionary();

			dict.put("actionName", actionData.actionName);
			dict.put("minInvisLevelRequired", actionData.minInvisLevelRequired);
			dict.put("commandType", actionData.commandType);
			dict.put("validTarget", actionData.validTarget);
			dict.put("hitType", actionData.hitType);
			dict.put("effectOnTarget", actionData.effectOnTarget);
			dict.put("setCombatTarget", actionData.setCombatTarget);
			dict.put("delayAttackEggTemplate", actionData.delayAttackEggTemplate);
			dict.put("delayAttackParticle", actionData.delayAttackParticle);
			dict.put("initialDelayAttackInterval", actionData.initialDelayAttackInterval);
			dict.put("delayAttackInterval", actionData.delayAttackInterval);
			dict.put("delayAttackEggPosition", actionData.delayAttackEggPosition);
			dict.put("delayAttackLoops", actionData.delayAttackLoops);
			dict.put("validEggTarget", actionData.validEggTarget);
			dict.put("attackType", actionData.attackType);
			dict.put("coneLength", actionData.coneLength);
			dict.put("coneWidth", actionData.coneWidth);
			dict.put("minRange", actionData.minRange);
			dict.put("maxRange", actionData.maxRange);
			dict.put("addedDamage", actionData.addedDamage);
			dict.put("percentAddFromWeapon", actionData.percentAddFromWeapon);
			dict.put("hateDamageModifier", actionData.hateDamageModifier);
			dict.put("maxHate", actionData.maxHate);
			dict.put("hateAdd", actionData.hateAdd);
			dict.put("hateAddTime", actionData.hateAddTime);
			dict.put("hateReduce", actionData.hateReduce);
			dict.put("actionCost", actionData.actionCost);
			dict.put("vigorCost", actionData.vigorCost);
			dict.put("scaleByDefenderWeaponSpeed", actionData.scaleByDefenderWeaponSpeed);
			dict.put("dotType", actionData.dotType);
			dict.put("dotIntensity", actionData.dotIntensity);
			dict.put("dotDuration", actionData.dotDuration);
			dict.put("buffNameTarget", actionData.buffNameTarget);
			dict.put("buffStrengthTarget", actionData.buffStrengthTarget);
			dict.put("buffDurationTarget", actionData.buffDurationTarget);
			dict.put("buffNameSelf", actionData.buffNameSelf);
			dict.put("buffStrengthSelf", actionData.buffStrengthSelf);
			dict.put("buffDurationSelf", actionData.buffDurationSelf);
			dict.put("canBePunishing", actionData.canBePunishing);
			dict.put("increaseCritical", actionData.increaseCritical);
			dict.put("increaseStrikethrough", actionData.increaseStrikethrough);
			dict.put("reduceGlancing", actionData.reduceGlancing);
			dict.put("reduceParry", actionData.reduceParry);
			dict.put("reduceBlock", actionData.reduceBlock);
			dict.put("reduceDodge", actionData.reduceDodge);
			dict.put("overloadWeapon", actionData.overloadWeapon);
			dict.put("overloadWeaponMinDamage", actionData.overloadWeaponMinDamage);
			dict.put("overloadWeaponMaxDamage", actionData.overloadWeaponMaxDamage);
			dict.put("overloadWeaponMinRange", actionData.overloadWeaponMinRange);
			dict.put("overloadWeaponMaxRange", actionData.overloadWeaponMaxRange);
			dict.put("overloadWeaponType", actionData.overloadWeaponType);
			dict.put("overloadWeaponCategory", actionData.overloadWeaponCategory);
			dict.put("overloadWeaponDamageType", actionData.overloadWeaponDamageType);
			dict.put("overloadWeaponElementalType", actionData.overloadWeaponElementalType);
			dict.put("overloadWeaponElementalValue", actionData.overloadWeaponElementalValue);
			dict.put("overloadWeaponAttackSpeed", actionData.overloadWeaponAttackSpeed);
			dict.put("overloadWeaponDamageRadius", actionData.overloadWeaponDamageRadius);
			dict.put("specialLine", actionData.specialLine);
			dict.put("animDefault", actionData.animDefault);
			dict.put("intRightFoot", actionData.intRightFoot);
			dict.put("intLeftFoot", actionData.intLeftFoot);
			dict.put("intRightHand", actionData.intRightHand);
			dict.put("intLeftHand", actionData.intLeftHand);
			dict.put("intWeapon", actionData.intWeapon);
			dict.put("anim_unarmed", actionData.anim_unarmed);
			dict.put("anim_onehandmelee", actionData.anim_onehandmelee);
			dict.put("anim_twohandmelee", actionData.anim_twohandmelee);
			dict.put("anim_polearm", actionData.anim_polearm);
			dict.put("anim_pistol", actionData.anim_pistol);
			dict.put("anim_lightRifle", actionData.anim_lightRifle);
			dict.put("anim_carbine", actionData.anim_carbine);
			dict.put("anim_rifle", actionData.anim_rifle);
			dict.put("anim_heavyweapon", actionData.anim_heavyweapon);
			dict.put("anim_thrown", actionData.anim_thrown);
			dict.put("anim_onehandlightsaber", actionData.anim_onehandlightsaber);
			dict.put("anim_twohandlightsaber", actionData.anim_twohandlightsaber);
			dict.put("anim_polearmlightsaber", actionData.anim_polearmlightsaber);
			dict.put("validWeapon", actionData.validWeapon);
			dict.put("invalidWeapon", actionData.invalidWeapon);
			dict.put("cooldownGroup", actionData.cooldownGroup);
			dict.put("warmupTime", actionData.warmupTime);
			dict.put("executeTime", actionData.executeTime);
			dict.put("cooldownTime", actionData.cooldownTime);
			dict.put("attribute", actionData.attribute);
			dict.put("bypassArmor", actionData.bypassArmor);
			dict.put("convertDamageToHealth", actionData.convertDamageToHealth);
			dict.put("ignore_distance", actionData.ignore_distance);
			dict.put("pvp_only", actionData.pvp_only);
			dict.put("attack_rolls", actionData.attack_rolls);
			dict.put("flatActionDamage", actionData.flatActionDamage);
			dict.put("performanceSpamStrId", actionData.performanceSpamStrId);
			dict.put("hitSpam", actionData.hitSpam);

			return dict;
		}

		public static combat_data getCombatDataFromDictionary(dictionary dict)
		{
				combat_data dat = new combat_data();

				dat.actionName 						= dict.getString("actionName");
				dat.minInvisLevelRequired 			= dict.getInt("minInvisLevelRequired");
				dat.commandType 					= dict.getInt("commandType");
				dat.validTarget 					= dict.getInt("validTarget");
				dat.hitType 						= dict.getInt("hitType");
				dat.effectOnTarget					= dict.getString("effectOnTarget");
				dat.setCombatTarget 				= dict.getInt("setCombatTarget");
				dat.delayAttackEggTemplate 			= dict.getString("delayAttackEggTemplate");
				dat.delayAttackParticle 			= dict.getString("delayAttackParticle");
				dat.initialDelayAttackInterval 		= dict.getFloat("initialDelayAttackInterval");
				dat.delayAttackInterval 			= dict.getFloat("delayAttackInterval");
				dat.delayAttackEggPosition 			= dict.getInt("delayAttackEggPosition");
				dat.delayAttackLoops				= dict.getInt("delayAttackLoops");
				dat.validEggTarget 					= dict.getInt("validEggTarget");
				dat.attackType 						= dict.getInt("attackType");
				dat.coneLength 						= dict.getFloat("coneLength");
				dat.coneWidth 						= dict.getFloat("coneWidth");
				dat.minRange 						= dict.getFloat("minRange");
				dat.maxRange 						= dict.getFloat("maxRange");
				dat.addedDamage 					= dict.getInt("addedDamage");
				dat.percentAddFromWeapon 			= dict.getFloat("percentAddFromWeapon");
				dat.hateDamageModifier 				= dict.getFloat("hateDamageModifier");
				dat.maxHate 						= dict.getInt("maxHate");
				dat.hateAdd 						= dict.getInt("hateAdd");
				dat.hateAddTime 					= dict.getInt("hateAddTime");
				dat.hateReduce						= dict.getInt("hateReduce");
				dat.actionCost 						= dict.getFloat("actionCost");
				dat.vigorCost 						= dict.getFloat("vigorCost");
				dat.scaleByDefenderWeaponSpeed		= dict.getFloat("scaleByDefenderWeaponSpeed");
				dat.dotType 						= dict.getString("dotType");
				dat.dotIntensity 					= dict.getInt("dotIntensity");
				dat.dotDuration 					= dict.getInt("dotDuration");
				dat.buffNameTarget 					= dict.getString("buffNameTarget");
				dat.buffStrengthTarget 				= dict.getFloat("buffStrengthTarget");
				dat.buffDurationTarget 				= dict.getFloat("buffDurationTarget");
				dat.buffNameSelf 					= dict.getString("buffNameSelf");
				dat.buffStrengthSelf 				= dict.getFloat("buffStrengthSelf");
				dat.buffDurationSelf 				= dict.getFloat("buffDurationSelf");
				dat.canBePunishing					= dict.getInt("canBePunishing");
				dat.increaseCritical				= dict.getFloat("increaseCritical");
				dat.increaseStrikethrough			= dict.getFloat("increaseStrikethrough");
				dat.reduceGlancing					= dict.getFloat("reduceGlancing");
				dat.reduceParry						= dict.getFloat("reduceParry");
				dat.reduceBlock						= dict.getFloat("reduceBlock");
				dat.reduceDodge						= dict.getFloat("reduceDodge");
				dat.overloadWeapon 					= dict.getInt("overloadWeapon");
				dat.overloadWeaponMinDamage 		= dict.getInt("overloadWeaponMinDamage");
				dat.overloadWeaponMaxDamage 		= dict.getInt("overloadWeaponMaxDamage");
				dat.overloadWeaponMinRange 			= dict.getFloat("overloadWeaponMinRange"); // this is shared with the command's min range
				dat.overloadWeaponMaxRange 			= dict.getFloat("overloadWeaponMaxRange"); // this is shared with the command's max range
				dat.overloadWeaponType 				= dict.getInt("overloadWeaponType");
				dat.overloadWeaponCategory 			= dict.getInt("overloadWeaponCategory");
				dat.overloadWeaponDamageType 		= dict.getInt("overloadWeaponDamageType");
				dat.overloadWeaponElementalType 	= dict.getInt("overloadWeaponElementalType");
				dat.overloadWeaponElementalValue 	= dict.getInt("overloadWeaponElementalValue");
				dat.overloadWeaponAttackSpeed 		= dict.getFloat("overloadWeaponAttackSpeed");
				dat.overloadWeaponDamageRadius 		= dict.getFloat("overloadWeaponDamageRadius");
				dat.specialLine 					= dict.getString("specialLine");
				dat.animDefault						= dict.getString("animDefault");
				dat.intRightFoot					= dict.getInt("intRightFoot");
				dat.intLeftFoot						= dict.getInt("intLeftFoot");
				dat.intRightHand					= dict.getInt("intRightHand");
				dat.intLeftHand						= dict.getInt("intLeftHand");
				dat.intWeapon						= dict.getInt("intWeapon");
				dat.anim_unarmed					= dict.getString("anim_unarmed");
				dat.anim_onehandmelee				= dict.getString("anim_onehandmelee");
				dat.anim_twohandmelee				= dict.getString("anim_twohandmelee");
				dat.anim_polearm					= dict.getString("anim_polearm");
				dat.anim_pistol						= dict.getString("anim_pistol");
				dat.anim_lightRifle					= dict.getString("anim_lightRifle");
				dat.anim_carbine					= dict.getString("anim_carbine");
				dat.anim_rifle						= dict.getString("anim_rifle");
				dat.anim_heavyweapon				= dict.getString("anim_heavyweapon");
				dat.anim_thrown						= dict.getString("anim_thrown");
				dat.anim_onehandlightsaber			= dict.getString("anim_onehandlightsaber");
				dat.anim_twohandlightsaber			= dict.getString("anim_twohandlightsaber");
				dat.anim_polearmlightsaber			= dict.getString("anim_polearmlightsaber");
				dat.validWeapon						= dict.getInt("validWeapon");
				dat.invalidWeapon					= dict.getInt("invalidWeapon");
				dat.cooldownGroup					= dict.getString("cooldownGroup");
				dat.warmupTime						= dict.getFloat("warmupTime");
				dat.executeTime						= dict.getFloat("executeTime");
				dat.cooldownTime					= dict.getFloat("cooldownTime");
				dat.attribute						= dict.getInt("healAttrib");
				dat.bypassArmor						= dict.getFloat("bypassArmor");
				dat.convertDamageToHealth			= dict.getFloat("convertDamageToHealth");
				dat.ignore_distance					= dict.getInt("ignore_distance");
				dat.pvp_only						= dict.getInt("pvp_only");
				dat.attack_rolls					= dict.getInt("attack_rolls");
				dat.flatActionDamage				= dict.getInt("flatActionDamage");
				dat.performanceSpamStrId			= dict.getStringId("performanceSpamStrId");
				dat.hitSpam 						= dict.getInt("hitSpam");

				return dat;
		}


		/*********************************************************************/

		/**
		 * Support classes
		 */

		public static class combat_data implements java.lang.Cloneable
		{
			public static combat_data clone(combat_data template)
			{
				try
				{
					combat_data cd = (combat_data)template.clone();
					return cd;
				}
				catch(java.lang.CloneNotSupportedException exc)
				{
					return null;
				}
			}

			public String 	actionName 						= "";
			public int 		minInvisLevelRequired 			= -1;
			public int 		commandType 					= 2; // special attack
			public int 		validTarget 					= 0; // standard
			public int 		hitType 						= -1; // ATTACK
			public String 	effectOnTarget					= "";
			public int 		setCombatTarget 				= 1;
			public String 	delayAttackEggTemplate 			= "object/tangible/space/content_infrastructure/generic_egg_small.iff";
			public String 	delayAttackParticle 			= "";
			public float 	initialDelayAttackInterval 		= -1.0f;
			public float 	delayAttackInterval 			= 3.0f;
			public int 		delayAttackLoops 				= 1;
			public int 		delayAttackEggPosition 			= 2; // TargetedObject
			public int 		validEggTarget 					= 0; // standard
			public int 		attackType 						= 1; // single target
			public float 	coneLength 						= 0;
			public float 	coneWidth 						= 0;
			public float 	minRange 						= 0;
			public float 	maxRange 						= 0;
			public int 		addedDamage 					= 0;
			public float 	percentAddFromWeapon 			= 1.0f;
			public float 	hateDamageModifier 				= 1.0f;
			public int 		maxHate 						= 0;
			public int 		hateAdd 						= 0;
			public int 		hateAddTime 					= 0;
			public int		hateReduce						= 0;
			public float	actionCost 						= 0;
			public float 	vigorCost						= 0;
			public float	scaleByDefenderWeaponSpeed				= 0;
			public String 	dotType 						= "";
			public int 		dotIntensity 					= 0;
			public int 		dotDuration 					= 0;
			public String 	buffNameTarget 					= "";
			public float 	buffStrengthTarget 				= 0;
			public float 	buffDurationTarget 				= 0;
			public String 	buffNameSelf 					= "";
			public float 	buffStrengthSelf 				= 0;
			public float 	buffDurationSelf 				= 0;
			public int 		canBePunishing					= 1;
			public float	increaseCritical				= 0.0f;
			public float	increaseStrikethrough			= 0.0f;
			public float	reduceGlancing					= 0.0f;
			public float	reduceParry						= 0.0f;
			public float	reduceBlock						= 0.0f;
			public float	reduceDodge						= 0.0f;
			public int 		overloadWeapon 					= 0;
			public int 		overloadWeaponMinDamage 		= 0;
			public int 		overloadWeaponMaxDamage 		= 0;
			public float	overloadWeaponMinRange 			= 0;
			public float	overloadWeaponMaxRange 			= 0;
			public int 		overloadWeaponType 				= 2;
			public int 		overloadWeaponCategory 			= -1; // ranged
			public int 		overloadWeaponDamageType 		= 1; // kinetic
			public int 		overloadWeaponElementalType 	= 1; // kinetic?
			public int 		overloadWeaponElementalValue 	= 0;
			public float 	overloadWeaponAttackSpeed 		= 1.0f;
			public float 	overloadWeaponDamageRadius 		= 0;
			public String 	specialLine 					= "";
			public location targetLoc						= new location();
			public float	bypassArmor						= 0;
			public int		intLeftFoot						= 0;
			public int		intRightFoot					= 0;
			public int 		intRightHand					= 0;
			public int		intLeftHand						= 0;
			public int		intWeapon						= 0;
			public String	animDefault						= "";
			public String	anim_unarmed					= "";
			public String	anim_onehandmelee				= "";
			public String	anim_twohandmelee				= "";
			public String	anim_polearm					= "";
			public String	anim_pistol						= "";
			public String	anim_lightRifle					= "";
			public String	anim_carbine					= "";
			public String	anim_rifle						= "";
			public String	anim_heavyweapon				= "";
			public String	anim_thrown						= "";
			public String	anim_onehandlightsaber			= "";
			public String	anim_twohandlightsaber			= "";
			public String	anim_polearmlightsaber			= "";
			public int		delayAttackLoopsDone			= 0;
			public int		validWeapon						= 30; // ALL=30
			public int		invalidWeapon					= 0;
			public String	cooldownGroup					= "";
			public float	warmupTime						= 0;
			public float	executeTime						= 0;
			public float 	cooldownTime					= 0;
			public int		attribute						= 0;
			public String	params							= "";
			public string_id performanceSpamStrId			= new string_id("", "");
			public int      hitSpam							= 0;
			public float	convertDamageToHealth			= 0.0f;
			public int		ignore_distance					= 0;
			public int		pvp_only						= 0;
			public int 		attack_rolls					= 1;
			public int		flatActionDamage				= 0;
	}

	public static class attack_roll_result
	{
		public int baseRoll = 0;
		public int finalRoll = 0;
	}

	public static class hit_result
	{
		public boolean success = false;
		public int     baseRoll = 0;
		public int     finalRoll = 0;
		public float   attackVal = 0.0f;
		public boolean canSee = true;
		public int     hitLocation = 0;
		public int     damage = 0;
		public int     damageType = 0;	    // base damage type, set to 0 to use the attacker's weapon's damage type
		public int     rawDamage = 0;		// base amount of actual damage (not counting bleed or crit, does include blocked damage)
		public int     elementalDamage = 0;	// Amount of secondary damage done
		public int     elementalDamageType = 0;	// Type of elemental damage, set to 0 to use the attacker's weapon's damage type
		public int     bleedDamage = 0;		// bleed damage amount
		public int     critDamage = 0;		// critical damage bonus
		public int     blockedDamage = 0;	// damage amount blocked by armor
		public obj_id  blockingArmor = null;// id of the armor piece that blocked the damage
		public int     bleedingChance = 0;
		public boolean proc = false;
		public boolean critical = false;	// does extra damage
		public boolean strikethrough = false;// bypasses armor
		public float strikethroughAmmount = 0.0f; // Ammount of armor to bypass
		public boolean glancing = false;	// Damage reduction by skill comparison
		public boolean dodge = false;		// Negates damage
		public boolean parry = false;		// Negates melee damage
		public boolean crushing = false;	// Damage increase by skill comparison
		public boolean miss = false;		// Failed attack roll
		public boolean evadeResult = false; // Chance to avoid AE damage
		public float evadeAmmount = 0.0f;	// % of damage avoided
		public boolean blockResult = false; // Chance to soak damage
		public int block = 0;				// Ammount of damage mitigated on a block
	}

	public static class combatant_data
	{
		public obj_id   id;                 // the combatant's id
		public location pos;				// where the combatant is in local coordinates
		public location worldPos;			// where the combatant is in world-space coordinates
		public float    radius;			    // the bounding-radius of the combatant
		public boolean  isCreature;			// flag that the combatant is a creature
		public int      posture;			// the combatant's posture (if it is a creature)
		public int      locomotion;         // the combatant's locomotion (if it is a creature)
		public int      scriptMod = 0;		// generic param that will be added to the combatant's roll
	}

	public static class attacker_data extends combatant_data
	{
		public int weaponSkillMod;		// the skill mod for the weapon being used by the combatant
		public int aims;				// number of aims the attacker is taking
		public float critChance				= 0.0f;
		public float pvpCrit				= 0.0f;
		public float npcCrit				= 0.0f;
		public float droidCrit				= 0.0f;
		public float creatureCrit			= 0.0f;
		public float hitChance				= 0.0f;
		public float strikethroughChance	= 0.0f;
		public float reduceParry			= 0.0f;
		public float reduceBlock			= 0.0f;
		public float reduceDodge			= 0.0f;
		public float reduceGlancing			= 0.0f;
		public float reduceEvade			= 0.0f;
	}

	public static class defender_data extends combatant_data
	{
		public int combatSkeleton;			// combat skeleton id of the defender
		public int[] hitLocationChances;	// chances to hit a particular area
		public int cover;					// the amount of cover the defender has
		public float reduceCritical			= 0.0f;
		public float increaseMiss			= 0.0f;
		public float reduceStrikethrough	= 0.0f;
		public float reducePunishing		= 0.0f;
		public float glancingChance			= 0.0f;
		public float parryChance			= 0.0f;
		public float dodgeChance			= 0.0f;
		public float evadeChance			= 0.0f;
		public float blockChance			= 0.0f;
	}

	public static class weapon_data
	{
		public obj_id      id;					// weapon's network id
		public string_id   weaponName = null;	// weapon's name
		public int         minDamage;			// minimum damage done
		public int         maxDamage;			// maximum damage done
		public int         weaponType;          // weapon type (rifle, pistol, etc)
		public int 	   weaponCategory;	// category
		public int         attackType;			// attack type (melee, ranged, etc)
		public int         damageType;			// base damage type (kinetic or energy)
		public int         elementalType;		// bonus elemental damage type (acid, cold, etc.)
		public int         elementalValue;		// bonus elemental damage amount
		public float       attackSpeed;         // the weapon's intrisic firing rate
        public float       woundChance;         // base % chance for a wound to occur on a successful attack
     	public int         accuracy;         	// accuracy modifier for the weapon
		public float       minRange;            // range where the min range mod is set
	  	public float       maxRange;            // range past which the weapon cannot hit (always read from the template)
		public float       damageRadius;        // damage radius for area-effect weapons
		public int         attackCost;    		// weapon special move cost
		public int         hvyWeaponType;		// stores an integer (as listed in heavyweapons.scriptlib::TYPE_*) denoting which heavy weapon
		public float       hvyWeaponAeDamageMod;// a value determining the amount of damage AOE targets take based on how much damage the combat target took
		public boolean     isDisabled;			// flag that the weapon is disabled due to damage
	}

	public static class effect_data
	{
		public int     posture = base_class.POSTURE_NONE;    // End posture after attack
		public int[]   states;                               // Array of states to be applied with attack
		public float[] durations;                            // Array of duration of states, must match states length
		public int     stateChance = 0;                      // Chance for each state to be applied
	}

	// Buff data

	public static buff_data getBuffData(int buffNameCrc)
	{
		if(m_BuffDataCache.containsKey("" + buffNameCrc))
		{
			return buff_data.clone((buff_data)m_BuffDataCache.get("" + buffNameCrc));
		}

		buff_data dat = loadBuffData(buffNameCrc);

		if(dat != null)
		{
			m_BuffDataCache.put("" + buffNameCrc, dat);
		}
		else
		{
			return null;
		}

		return buff_data.clone(dat);
	}

	private static buff_data loadBuffData(int buffNameCrc)
	{
		buff_data dat = new buff_data();

		int row = base_class.dataTableSearchColumnForInt(buffNameCrc, "NAME", "datatables/buff/buff.iff");

		if(row == -1)
		{
			return null;
		}

		dictionary d = base_class.dataTableGetRow("datatables/buff/buff.iff", row);

		if(d == null)
		{
			return null;
		}

		dat.buffName 						= d.getString("NAME");
		dat.buffGroup1						= d.getString("GROUP1");
		dat.buffGroup1Crc					= base_class.getStringCrc(dat.buffGroup1);
		dat.buffGroup2						= d.getString("GROUP2");
		dat.buffGroup2Crc					= base_class.getStringCrc(dat.buffGroup2);
		dat.blockGroup						= d.getString("BLOCK");
		dat.blockGroupCrc					= base_class.getStringCrc(dat.blockGroup);
		dat.priority			 			= d.getInt("PRIORITY");
		dat.icon							= d.getString("ICON");
		dat.duration				 		= d.getFloat("DURATION");
		dat.effect1Param					= d.getString("EFFECT1_PARAM");
		dat.effect1Value			 		= d.getFloat("EFFECT1_VALUE");
		dat.effect2Param					= d.getString("EFFECT2_PARAM");
		dat.effect2Value			 		= d.getFloat("EFFECT2_VALUE");
		dat.effect3Param					= d.getString("EFFECT3_PARAM");
		dat.effect3Value			 		= d.getFloat("EFFECT3_VALUE");
		dat.effect4Param					= d.getString("EFFECT4_PARAM");
		dat.effect4Value			 		= d.getFloat("EFFECT4_VALUE");
		dat.effect5Param					= d.getString("EFFECT5_PARAM");
		dat.effect5Value			 		= d.getFloat("EFFECT5_VALUE");
		dat.buffState			 			= d.getInt("STATE");
		dat.callback						= d.getString("CALLBACK");
		dat.particle						= d.getString("PARTICLE");
		dat.particleHardpoint				= d.getString("PARTICLE_HARDPOINT");
		dat.visible				 			= d.getInt("VISIBLE");
		dat.debuff							= d.getInt("DEBUFF");
		dat.stanceParticle					= d.getString("STANCE_PARTICLE");
		dat.dispellPlayer		 			= d.getInt("DISPELL_PLAYER");
		dat.removeOnDeath		 			= d.getInt("REMOVE_ON_DEATH");
		dat.playerRemovable		 			= d.getInt("PLAYER_REMOVABLE");
		dat.isCelestial			 			= d.getInt("IS_CELESTIAL");
		dat.maxStacks			 			= d.getInt("MAX_STACKS");
		dat.isPersistant		 			= d.getInt("IS_PERSISTENT");
		dat.displayOrder		 			= d.getInt("DISPLAY_ORDER");
		dat.removeOnRespec		 			= d.getInt("REMOVE_ON_RESPEC");
		dat.aiRemoveOnCombatEnd	 			= d.getInt("AI_REMOVE_ON_COMBAT_END");
		dat.decayOnPvpDeath					= d.getInt("DECAY_ON_PVP_DEATH");

		return dat;
	}

	public static class buff_data implements java.lang.Cloneable
	{
		public static buff_data clone(buff_data template)
		{
			try
			{
				buff_data cd = (buff_data)template.clone();
				return cd;
			}
			catch(java.lang.CloneNotSupportedException exc)
			{
				return null;
			}
		}

		public String 	buffName 						= "";
		public String 	buffGroup1						= "";
		public int	 	buffGroup1Crc					= 0;
		public String 	buffGroup2						= "";
		public int	 	buffGroup2Crc					= 0;
		public String 	blockGroup						= "";
		public int	 	blockGroupCrc					= 0;
		public int 		priority			 			= 0;
		public String 	icon							= "";
		public float 	duration				 		= 0.0f;
		public String 	effect1Param					= "";
		public float 	effect1Value			 		= 0.0f;
		public String 	effect2Param					= "";
		public float 	effect2Value			 		= 0.0f;
		public String 	effect3Param					= "";
		public float 	effect3Value			 		= 0.0f;
		public String 	effect4Param					= "";
		public float 	effect4Value			 		= 0.0f;
		public String 	effect5Param					= "";
		public float 	effect5Value			 		= 0.0f;
		public int 		buffState				 		= 0;
		public String 	callback						= "";
		public String 	particle						= "";
		public String 	particleHardpoint				= "";
		public int 		visible				 			= 1;
		public int		debuff							= 0;
		public String	stanceParticle					= "";
		public int 		dispellPlayer		 			= 1;
		public int 		removeOnDeath		 			= 1;
		public int 		playerRemovable		 			= 1;
		public int 		isCelestial			 			= 0;
		public int 		maxStacks			 			= 1;
		public int 		isPersistant		 			= 1;
		public int 		displayOrder		 			= 0;
		public int 		removeOnRespec		 			= 1;
		public int 		aiRemoveOnCombatEnd	 			= 1;
		public int		decayOnPvpDeath					= 1;
	}


	/*********************************************************************/

	/**
 	 * Main combat functions.
	 */

	/**
	 * Determines if a given attacker hits his opponent.
	 *
	 * @param attacker		the attacker's data
	 * @param defender		the defender's data
	 * @param weapon		the attacker's weapon's data
	 *
	 * @return result of attack
	 */
	public static hit_result calculateHit(attacker_data attacker, defender_data defender, weapon_data weapon)
	{
		hit_result result = new hit_result();

		// determine the attacker's and defender's rolls
		attack_roll_result attackRoll = getAttackRoll(attacker, defender, weapon);
		if (attackRoll.baseRoll == OUT_OF_RANGE)
		{
//			System.err.println("OUT OF RANGE");
			return result;
		}

		int defenseRoll = getDefenseRoll(attacker, defender, weapon);

		// determine if the attack was successful
		float attackVal = attackRoll.baseRoll - defenseRoll;
		result.attackVal = attackVal;
		attackVal /= combat_consts.toHitScale;

		float stepDir = 0.0f;
		if (attackVal > stepDir) stepDir = 1.0f;
		else if (attackVal < stepDir) stepDir = -1.0f;

		float toHitChance = combat_consts.baseToHit;
		int maxStep = (int)Math.ceil((combat_consts.baseToHit - combat_consts.minToHit)/combat_consts.toHitStep);
		for (int i = 1; i < maxStep; i++)
		{
			if ((attackVal * stepDir) > i)
			{
				toHitChance += stepDir * combat_consts.toHitStep;
				attackVal -= stepDir * i;
			}
			else
			{
				toHitChance += (attackVal/i) * combat_consts.toHitStep;
				break;
			}
		}

		if (toHitChance > combat_consts.maxToHit) toHitChance = combat_consts.maxToHit;
		if (toHitChance < combat_consts.minToHit) toHitChance = combat_consts.minToHit;

		if (random.rand(1.0f, 100.0f) < toHitChance)
		{
			// Shouldn't need LOS check here as checks are done at defender list creation
			//// verify that the attacker can see the defender
			//if (!base_class.canSee(attacker.id, defender.id))
			//{
			//	result.canSee = false;
			//}
			//else
			{
				result.success = true;
				// determine hit location and base damage
				result.hitLocation = getHitLocation(defender);
				result.damage = getDamage(weapon, result.attackVal);
			}
		}

		result.baseRoll = attackRoll.baseRoll;
		result.finalRoll = attackRoll.finalRoll;
		return result;
	}	// calculateHit()

	/*********************************************************************/

	/**
	 * Support functions.
	 */

	/**
 	 * Determines the attack roll.
	 *
	 * @param attacker		the attacker's data
	 * @param defender		the defender's data
	 * @param weapon		the attacker's weapon's data
	 *
	 * @return the attack roll, along with the unmodified base attack roll
	 */
	public static attack_roll_result getAttackRoll(attacker_data attacker, defender_data defender, weapon_data weapon)
	{
		if (attacker == null || defender == null || weapon == null)
		{
			System.err.println("WARNING: null data passed to getAttackRoll: " + attacker + "; " + defender + "; " + weapon);
			return null;
		}
		if (attacker.id == null || defender.id == null || weapon.id == null)
		{
			System.err.println("WARNING: null id passed to getAttackRoll: " + attacker.id + "; " + defender.id + "; " + weapon.id);
			return null;
		}
		if (attacker.pos == null || attacker.worldPos == null || defender.pos == null || defender.worldPos == null)
		{
			System.err.println("WARNING: null pos passed to getAttackRoll: " + attacker.pos + "; " + attacker.worldPos +
				"; " + defender.pos + "; " + defender.worldPos);
			return null;
		}

		attack_roll_result result = new attack_roll_result();

		int skillMod = 0;
		int aimsMod = 0;
		int attitudeMod = 0;
		int locomotionMod = 0;

		// adjust for distance between attacker and defender, and auto fail if the
		// attacker is out of range
		float dist = attacker.worldPos.distance(defender.worldPos) - (attacker.radius + defender.radius);
		// check for object interpenetration
		if ( dist < 0 )
			dist = 0;
		// TODO: COMBAT_UPGRADE: Add Accuracy
//		System.out.println("***Java combat_engine: attacker_rad = " + attacker.radius + ", defender_rad = " + defender.radius);
//		System.out.println("***Java combat_engine: base dist = " + attacker.worldPos.distance(defender.worldPos) + ", adjusted dist = " + dist + ", rangeMod = " + rangeMod);

		// adjust for weapon skills
		skillMod = attacker.weaponSkillMod;

		// adjust for aims
		if (attacker.aims > 0) // modified by dan to work with new aim skill mods
		{
			if(attacker.aims>3)
			{
				attacker.aims=3;
			}
			String strAimMod = getWeaponStringType(weapon.weaponType);
			strAimMod.concat("_aim");
			int intBaseAimMod = base_class.getSkillStatisticModifier(attacker.id, "aim");
			aimsMod = attacker.aims * (base_class.getSkillStatisticModifier(attacker.id, strAimMod)+intBaseAimMod);

		/*
			if (aimIndex < combat_consts.aimAttackMods.length)
				aimsMod = combat_consts.aimAttackMods[attacker.aims - 1];
			else
				System.err.println("ERROR: Java combat_engine getAttackRoll - too many aims (" + attacker.aims + ")");
		*/ // Potentiall depracated functions above. we're using  straight multiply now
		}

		// adjust for attacker locomotion
		if (attacker.isCreature)
		{
			if ( weapon.attackType >= 0 && weapon.attackType < combat_consts.locomotionAttackMod.length )
			{
				if (attacker.locomotion >= 0 && attacker.locomotion < combat_consts.locomotionAttackMod[weapon.attackType].length)
				{
					locomotionMod = combat_consts.locomotionAttackMod[weapon.attackType][attacker.locomotion];
					// adjust locomotion mod for weapon type
					if (weapon.weaponType >= 0 && weapon.weaponType < combat_consts.weaponTypeAttackLocomotionMultipliers.length)
					{
						locomotionMod = (int)(locomotionMod * combat_consts.weaponTypeAttackLocomotionMultipliers[weapon.weaponType]);
						locomotionMod = locomotionMod + getPostureModForWeapon(attacker.id, attacker.posture,weapon.weaponType);
					}


				}
				else
					System.err.println("ERROR: Java combat_engine getAttackRoll - invalid locomotion " + attacker.locomotion);
			}
			else
				System.err.println("ERROR: Java combat_engine getAttackRoll - invalid weapon attack type " + weapon.attackType);
		}
		if(isLocomotionMoving(attacker.locomotion)) // added by dan to support new locomotion skill modss
		{
			String strLocomotionAttackMod = getWeaponStringType(weapon.weaponType);
			strLocomotionAttackMod.concat("_hit_while_moving");
			locomotionMod = locomotionMod - base_class.getSkillStatisticModifier(attacker.id, strLocomotionAttackMod); // bonus to attacker if they have the skill against moving targets
			if(locomotionMod<0)
			{
				locomotionMod = 0;
			}
		}

		// roll the attack number
		//result.baseRoll = random.rand(1, 250);
		//result.finalRoll = result.baseRoll + skillMod + aimsMod + attitudeMod + locomotionMod + rangeMod + attacker.scriptMod;
		result.baseRoll = skillMod + attacker.scriptMod;
		result.finalRoll = result.baseRoll + aimsMod + attitudeMod + locomotionMod;

		return result;
	}	// getAttackRoll()

	/**
 	 * Determines the defense roll.
	 *
	 * @param attacker		the attacker's data
	 * @param defender		the defender's data
	 * @param weapon		the attacker's weapon's data
	 *
	 * @return the defense roll
	 */
	public static int getDefenseRoll(attacker_data attacker, defender_data defender, weapon_data weapon)
	{
		//int baseRoll = combat_consts.toHitNumber;
		int locomotionMod = 0;
		int coverMod = 0;

		// adjust for defender locomotion
		if (defender.isCreature)
		{
			if ( weapon.attackType >= 0 && weapon.attackType < combat_consts.locomotionDefenseMod.length )
			{
				if (defender.locomotion >= 0 && defender.locomotion < combat_consts.locomotionDefenseMod[weapon.attackType].length)
					locomotionMod = combat_consts.locomotionDefenseMod[weapon.attackType][defender.locomotion];
				else
					System.err.println("ERROR: Java combat_engine getDefenseRoll - invalid locomotion " + defender.locomotion);
			}
			else
				System.err.println("ERROR: Java combat_engine getDefenseRoll - invalid weapon attack type " + weapon.attackType);
		}

		// adjust for defender cover
		coverMod = defender.cover;

		//int baseRoll = random.rand(25, 175);
		//int defenseRoll = baseRoll + locomotionMod + coverMod + defender.scriptMod;
		int baseRoll = defender.scriptMod;
		int defenseRoll = baseRoll + locomotionMod + coverMod;

		return baseRoll;
	}	// getDefenseRoll()

	/**
	 * Determines where a defender got hit.
	 *
	 * @param defender		the object that was being attacked
	 *
	 * @return the hit location
	 */
	public static int getHitLocation(defender_data defender)
	{
		if (defender.combatSkeleton == 0 || defender.hitLocationChances == null)
			return 0;

		int randNum = random.rand(1, 100);

		for (int i = 0; i < defender.hitLocationChances.length; ++i)
		{
			if (randNum <= defender.hitLocationChances[i])
				return i;
			randNum -= defender.hitLocationChances[i];
		}
		// we should never get here
		System.err.println("WARNING Java combat_engine.getHitLocation, defender hit location chances != 100");
		return 0;
	}	// getHitLocation()

	/**
	 * Determines the base amount of damage a weapon does.
	 *
	 * @param weapon		the weapon
	 *
	 * @return the amount of damage
	 */
	public static int getDamage(weapon_data weapon)
	{
		return getDamage(weapon, 0);
	}

	public static int getDamage(weapon_data weapon, float attackVal)
	{
		int damage = 0;
		switch ( weapon.damageType )
		{
			case base_class.DAMAGE_KINETIC :
			case base_class.DAMAGE_ENERGY :
			case base_class.DAMAGE_BLAST :
			case base_class.DAMAGE_STUN :
			case base_class.DAMAGE_RESTRAINT :
			case base_class.DAMAGE_ELEMENTAL_HEAT :
			case base_class.DAMAGE_ELEMENTAL_COLD :
			case base_class.DAMAGE_ELEMENTAL_ACID :
			case base_class.DAMAGE_ELEMENTAL_ELECTRICAL :
			case base_class.DAMAGE_ENVIRONMENTAL_HEAT :
			case base_class.DAMAGE_ENVIRONMENTAL_COLD :
			case base_class.DAMAGE_ENVIRONMENTAL_ACID :
			case base_class.DAMAGE_ENVIRONMENTAL_ELECTRICAL :
				{
					float dist = 0.5f + (attackVal / combat_consts.damageScale);
					//damage = random.distributedRand(weapon.minDamage, weapon.maxDamage, dist);
					// TEMP HACK - Test with fixed damage results based on attackval
					if (dist < 0f) dist = 0f;
					if (dist > 1f) dist = 1f;
					damage = (int)(weapon.minDamage + ((weapon.maxDamage - weapon.minDamage) * dist));
				}
				break;
			default:
				System.err.println("ERROR Java combat_engine.getDamage weapon has unknown damage type " + weapon.damageType);
				break;
		}
		return damage;
	}	// getDamage()

	private static String getWeaponStringType(int intWeaponType)
	{// written by Dan R. Do not blame steve :)
		switch(intWeaponType)
		{
			case base_class.WEAPON_TYPE_RIFLE : return "rifle";
			case base_class.WEAPON_TYPE_LIGHT_RIFLE : return "carbine";
			case base_class.WEAPON_TYPE_PISTOL : return "pistol";
			case base_class.WEAPON_TYPE_HEAVY : return "heavyWeapon";
			case base_class.WEAPON_TYPE_1HAND_MELEE : return "1handMelee";
			case base_class.WEAPON_TYPE_2HAND_MELEE: return "2handeMelee";
			case base_class.WEAPON_TYPE_UNARMED: return "unarmed";
			case base_class.WEAPON_TYPE_POLEARM: return "polearm";
			case base_class.WEAPON_TYPE_THROWN: return "thrown";
		}
		return "";
	}
	private static boolean isLocomotionMoving(int intLocomotion)
	{	// written by Dan R.
		switch(intLocomotion)
		{
			case base_class.LOCOMOTION_SNEAKING		: return true;
			case base_class.LOCOMOTION_WALKING  		: return true;
			case base_class.LOCOMOTION_RUNNING  		: return true;
			case base_class.LOCOMOTION_CRAWLING		: return true;
			case base_class.LOCOMOTION_CLIMBING		: return true;
			case base_class.LOCOMOTION_FLYING		: return true;
			case base_class.LOCOMOTION_DRIVING_VEHICLE	: return true;
			case base_class.LOCOMOTION_RIDING_CREATURE	: return true;
		}
		return false;
	}
	private static int getPostureModForWeapon(obj_id objPlayer, int intPosture, int intWeaponType)
	{// dans function again, don't blame steve.
		switch (intPosture)
		{
			case base_class.POSTURE_CROUCHED: return base_class.getSkillStatisticModifier(objPlayer, getWeaponStringType(intWeaponType)+"_kneeling");
			case base_class.POSTURE_PRONE: return base_class.getSkillStatisticModifier(objPlayer, getWeaponStringType(intWeaponType)+"_prone");
			case base_class.POSTURE_UPRIGHT: return base_class.getSkillStatisticModifier(objPlayer, getWeaponStringType(intWeaponType)+"_standing");

		}
		return 0;
	}
}	// class combat_engine



