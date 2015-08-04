package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.ai.ai_combat;
import java.util.Arrays;
import java.util.Vector;
import script.library.ai_lib;
import script.library.armor;
import script.library.buff;
import script.library.callable;
import script.library.colors;
import script.library.corpse;
import script.library.group;
import script.library.hue;
import script.library.incubator;
import script.library.instance;
import script.library.pet_lib;
import script.library.prose;
import script.library.storyteller;
import script.library.utils;
import script.library.xp;


public class beast_lib extends script.base_script
{
	public beast_lib()
	{
	}
	public static final String BEASTS_TABLE = "datatables/beast/beasts.iff";
	public static final String BEASTS_STATS = "datatables/beast/beast_stats.iff";
	public static final String BEASTS_SPECIALS = "datatables/beast/beast_specials.iff";
	public static final String BEASTS_SPECIES_BONUS_TABLE = "datatables/beast/beast_class_attribute_grouping.iff";
	public static final String MOBILE_TEMPLATE_PREFIX = "object/mobile/beast_master/";
	public static final String INTANGIBLE_TEMPLATE_PREFIX = "object/intangible/beast/";
	public static final String BEAST_DISPLAY_TEMPLATE = "object/tangible/furniture/decorative/beast_stuffed.iff";
	
	public static final String OBJVAR_BEAST_NAME = "beast.name";
	public static final String OBJVAR_BEAST_TYPE = "beast.type";
	public static final String OBJVAR_BEAST_ENGINEER = "beast.creator";
	public static final String OBJVAR_SKILL_AGGRESSION = "beast.aggression";
	public static final String OBJVAR_SKILL_BEASTIAL_RESILIENCE = "beast.beastialResilience";
	public static final String OBJVAR_SKILL_CUNNING = "beast.cunning";
	public static final String OBJVAR_SKILL_HUNTERS_INSTINCT = "beast.huntersInstinct";
	public static final String OBJVAR_SKILL_INTELLIGENCE = "beast.intelligence";
	public static final String OBJVAR_SKILL_SURVIVAL = "beast.survival";
	public static final String OBJVAR_INCREASE_ARMOR = "beast.armor_increase";
	public static final String OBJVAR_INCREASE_DPS = "beast.dps_increase";
	
	public static final String OBJVAR_INCREASE_HIT_CHANCE = "beast.hit_chance_increase";
	public static final String OBJVAR_INCREASE_DODGE = "beast.dodge_increase";
	public static final String OBJVAR_INCREASE_HEALTH = "beast.health_increase";
	public static final String OBJVAR_INCREASE_PARRY = "beast.parry_increase";
	public static final String OBJVAR_INCREASE_GLANCING_BLOW = "beast.glancing_blow_increase";
	public static final String OBJVAR_INCREASE_BLOCK_CHANCE = "beast.block_chance_increase";
	public static final String OBJVAR_INCREASE_BLOCK_VALUE = "beast.block_value_increase";
	public static final String OBJVAR_INCREASE_CRITICAL_HIT = "beast.critical_hit_increase";
	public static final String OBJVAR_INCREASE_EVASION = "beast.evasion_increase";
	public static final String OBJVAR_INCREASE_EVASION_RATING = "beast.evasion_rating_increase";
	public static final String OBJVAR_INCREASE_STRIKETHROUGH = "beast.strikethrough_increase";
	public static final String OBJVAR_INCREASE_STRIKETHROUGH_RATING = "beast.strikethrough_rating_increase";
	
	public static final String OBJVAR_BEAST_BCD_OWNER = "beast.bcdOwner";
	
	public static final String OBJVAR_BEAST_APPEARANCE = "beast.appearance_template";
	public static final String OBJVAR_BEAST_PARENT = "beast.parent_creature_template";
	public static final String OBJVAR_BEAST_INCUBATION_BONUSES = "beast.incubation_bonuses";
	public static final String OBJVAR_OLD_PET_IDENTIFIER = "beast.converted_pet";
	public static final String OBJVAR_OLD_PET_RENAMED = "beast.old_pet_renamed";
	public static final String OBJVAR_OLD_PET_REHUED = "beast.old_pet_rehued";
	public static final String OBJVAR_BEAST_HUE = "beast.hue";
	public static final String OBJVAR_BEAST_HUE2 = "beast.hue2";
	public static final String OBJVAR_BEAST_HUE3 = "beast.hue3";
	public static final String OBJVAR_BEAST_PALETTE = "beast.palette";
	public static final String OBJVAR_BEAST_PALETTE2 = "beast.palette2";
	public static final String OBJVAR_BEAST_PALETTE3 = "beast.palette3";
	public static final String OBJVAR_BEAST_MOUNT_FLAG = "beast.isMount";
	public static final String OBJVAR_INJECTOR_MARK = "beast.injectorMark";
	
	public static final String[] DISPLAY_NAMES = 
	{
		"aggression_skill",
		"beastial_resilience_skill",
		"cunning_skill",
		"hunters_instinct_skill",
		"intelligence_skill",
		"survival_skill",
		"armor_bonus",
		"dps_bonus",
		"health_bonus",
		"hit_chance_bonus",
		"dodge_bonus",
		"parry_bonus",
		"glancing_blow_bonus",
		"block_chance_bonus",
		"block_value_bonus",
		"critical_hit_bonus",
		"evasion_bonus",
		"evasion_rating_bonus",
		"strikethrough_bonus",
		"strikethrough_rating_bonus"

	};
	
	public static final float[] DISPLAY_CONVERSION_RATES = 
	{
		1.0f, 
		1.0f, 
		1.0f, 
		1.0f, 
		1.0f, 
		1.0f, 
		0.1f, 
		0.1f, 
		0.2f, 
		0.1f, 
		1.0f, 
		1.0f, 
		0.1f, 
		1.0f, 
		1.0f, 
		1.0f, 
		1.0f, 
		1.0f, 
		1.0f, 
		1.0f  
	};
	
	public static final float[] DISPLAY_OBJVAR_CONVERSION_RATES = 
	{
		1.0f, 
		1.0f, 
		1.0f, 
		1.0f, 
		1.0f, 
		1.0f, 
		0.1f, 
		0.1f, 
		0.2f, 
		0.1f, 
		0.1f, 
		0.1f, 
		0.1f, 
		0.1f, 
		1.0f, 
		0.1f, 
		0.1f, 
		1.0f, 
		0.1f, 
		1.0f  
	};
	
	public static final String[] ARRAY_BEAST_INCUBATION_STATS = 
	{
		OBJVAR_SKILL_AGGRESSION,
		OBJVAR_SKILL_BEASTIAL_RESILIENCE,
		OBJVAR_SKILL_CUNNING,
		OBJVAR_SKILL_HUNTERS_INSTINCT,
		OBJVAR_SKILL_INTELLIGENCE,
		OBJVAR_SKILL_SURVIVAL,
		OBJVAR_INCREASE_ARMOR,
		OBJVAR_INCREASE_DPS,
		OBJVAR_INCREASE_HEALTH,
		OBJVAR_INCREASE_HIT_CHANCE,
		OBJVAR_INCREASE_DODGE,
		OBJVAR_INCREASE_PARRY,
		OBJVAR_INCREASE_GLANCING_BLOW,
		OBJVAR_INCREASE_BLOCK_CHANCE,
		OBJVAR_INCREASE_BLOCK_VALUE,
		OBJVAR_INCREASE_CRITICAL_HIT,
		OBJVAR_INCREASE_EVASION,
		OBJVAR_INCREASE_EVASION_RATING,
		OBJVAR_INCREASE_STRIKETHROUGH,
		OBJVAR_INCREASE_STRIKETHROUGH_RATING
	};
	
	public static final int MOD_HUNTERS_INSTINCT = 0;
	public static final int MOD_AGGRESSION = 1;
	public static final int MOD_INTELLIGENCE = 2;
	public static final int MOD_CUNNING = 3;
	public static final int MOD_RESILIENCE = 4;
	public static final int MOD_SURVIVAL = 5;
	
	public static final float BEAST_WEAPON_SPEED = 1.5f;
	public static final int BASE_ATTENTION_PENALTY = -25;
	public static final int BEASTS_STORED_MAXIMUM = 10;
	public static final int BEAST_LEVEL_MAX_DIFFERENCE = 10;
	public static final int STEROID_BUFF_LEVEL_CAP = 74;
	
	public static final boolean BEAST_DEBUG = true;
	
	public static final string_id SID_BEAST_TOO_HIGH_LEVEL = new string_id("beast", "cant_call_level");
	public static final string_id SID_MAXIMUM_BEASTS = new string_id("beast", "cant_call_maximum");
	public static final string_id SID_STORED = new string_id("beast", "stored");
	public static final string_id SID_NOT_BEAST_MASTER = new string_id("beast", "not_beast_master");
	public static final string_id SID_NO_BEAST_OUT = new string_id("beast", "no_beast");
	public static final string_id SID_MAXIMUM_COMBAT_CONTROL_DEVICES = new string_id("beast", "maximum_combat_control_devices");
	public static final string_id SID_BEAST_CANT_WHEN_DEAD = new string_id("beast", "beast_cant_when_dead");
	public static final string_id SID_BEAST_LEVEL_TOO_HIGH = new string_id("beast", "beast_buff_level_too_high");
	public static final string_id SID_BEAST_LEVEL_TOO_LOW = new string_id("beast", "beast_buff_level_too_low");
	
	public static final string_id SID_NO_TRICKS_WHILE_SWIMMING = new string_id("beast", "no_tricks_while_swimming");
	
	public static final String PET_TRAINED_SKILLS_LIST = "abilities.trained_skills";
	public static final String PET_AUTO_REPEAT_LIST = "abilities.auto_repeat";
	public static final String PLAYER_KNOWN_SKILLS_LIST = "beast_master.known_skills";
	public static final String ATTENTION_PENALTY_DEBUFF = "bm_attention_penalty_";
	
	public static final string_id SID_BEAST_WILD = new string_id("beast", "beast_wild");
	public static final int LOYALTY_LVL_TWO = 2001;
	public static final string_id SID_BEAST_DISOBEDIENT = new string_id("beast", "beast_disobedient");
	public static final int LOYALTY_LVL_THREE = 10001;
	public static final string_id SID_BEAST_TRAINED = new string_id("beast", "beast_trained");
	public static final int LOYALTY_LVL_FOUR = 50001;
	public static final string_id SID_BEAST_LOYAL = new string_id("beast", "beast_loyal");
	public static final int LOYALTY_LVL_FIVE = 250001;
	public static final string_id SID_BEAST_BFF = new string_id("beast", "beast_bff");
	public static final int LOYALTY_MAX = 500000;
	public static final String BEAST_LOYALTY_TITLE = "beast_loyalty_title";
	
	public static final String PET_LOYALTY_OBJVAR = "beastmood.beastLoyalty";
	public static final String PET_LOYALTY_LEVEL_OBJVAR = "beastmood.beastLoyaltyLevel";
	public static final float LOYALTY_DEATH_LOSS = 100;
	public static final float LOYALTY_FOOD_LOSS = 50;
	
	public static final string_id SID_LOYALTY_LEVEL_GAIN = new string_id("beast", "loyalty_gained");
	public static final string_id SID_LOYALTY_LEVEL_LOSS = new string_id("beast", "loyalty_lost");
	
	public static final String PET_HAPPINESS_SCRIPTVAR = "beastmood.beastHappiness";
	public static final String OBJVAR_BEAST_HAPPINESS = "beastmood.beastHappiness";
	public static final int PET_HAPPINESS_DEFAULT = 0;
	public static final int PET_HAPPINESS_LOYALTY_BONUS = 5;
	public static final int BEAST_HAPPINESS_MAX = 50;
	public static final int BEAST_HAPPINESS_MIN = -25;
	
	public static final String PET_LOYALTY_BONUS_SCRIPTVAR = "beastmood.beastLoyaltyBonus";
	
	public static final String BEAST_MOOD_TITLE = "beast_mood_title";
	public static final string_id SID_BEAST_FURIOUS = new string_id("beast", "beast_furious");
	public static final string_id SID_BEAST_ANGRY = new string_id("beast", "beast_angry");
	public static final string_id SID_BEAST_MISERABLE = new string_id("beast", "beast_miserable");
	public static final string_id SID_BEAST_UNHAPPY = new string_id("beast", "beast_unhappy");
	public static final string_id SID_BEAST_ANNOYED = new string_id("beast", "beast_annoyed");
	public static final string_id SID_BEAST_NORMAL = new string_id("beast", "beast_content");
	public static final string_id SID_BEAST_JOYOUS = new string_id("beast", "beast_joyous");
	public static final string_id SID_BEAST_HAPPY = new string_id("beast", "beast_happy");
	public static final string_id SID_BEAST_VERY_HAPPY = new string_id("beast", "beast_very_happy");
	public static final string_id SID_BEAST_ECSTATIC = new string_id("beast", "beast_ecstatic");
	public static final String DATATABLE_BEAST_FAVORITES = "datatables/beast/favorites.iff";
	public static final String DATATABLE_FOOD_COL = "beastFoodName";
	public static final String DATATABLE_LOCATION_COL = "beastLocationName";
	public static final String DATATABLE_ACTIVITY_COL = "beastActivityName";
	public static final int FOOD_FAVORITE = 0;
	public static final int FOOD_DISLIKE = 1;
	public static final int ACTIVITY_FAVORITE = 2;
	public static final int ACTIVITY_DISLIKE = 3;
	public static final int LOCATION_FAVORITE = 4;
	public static final int LOCATION_DISLIKE = 5;
	public static final int LIKE_DISLIKE_MAX = 6;
	public static final String PET_FAVORITES_OBJVAR = "beastmood.beastHappinessFactors";
	
	public static final string_id SID_NO_BEAST = new string_id("beast", "no_beast_to_feed");
	public static final string_id SID_NORMAL_FOOD = new string_id("beast", "fed_normal");
	public static final string_id SID_FAVORITE_FOOD = new string_id("beast", "fed_favorite");
	public static final string_id SID_DISLIKE_FOOD = new string_id("beast", "fed_dislike");
	public static final string_id SID_PET_HUNGRY = new string_id("beast", "pet_hungry");
	public static final String PET_HUNGRY_MESSAGE_SCRIPTVAR = "beastmood.ownerReceivedHungryMessage";
	public static final int PET_WHICH_FOOD = 0;
	public static final int PET_TIME_SINCE_FEEDING = 1;
	public static final int PET_FOOD_MAX_ARRAYS = 2;
	public static final int PET_NORMAL_FOOD = 5;
	public static final int PET_FAVORITE_FOOD = 10;
	public static final int PET_DISLIKE_FOOD = -5;
	public static final int PET_NO_FOOD = -10;
	public static final String PET_FED_SCRIPTVAR = "beastmood.beastLastFed";
	public static final String OBJVAR_BEAST_FOOD = "beastmood.beastLastFed";
	
	public static final string_id SID_FAVORITE_LOCATION = new string_id("beast", "location_favorite");
	public static final string_id SID_DISLIKE_LOCATION = new string_id("beast", "location_dislike");
	public static final int PET_NORMAL_LOCATION = 5;
	public static final int PET_FAVORITE_LOCATION = 10;
	public static final int PET_DISLIKE_LOCATION = -5;
	public static final String PET_LOCATION_SCRIPTVAR = "beastmood.beastLastLocation";
	
	public static final string_id SID_FAVORITE_ACTIVITY = new string_id("beast", "activity_favorite");
	public static final string_id SID_DISLIKE_ACTIVITY = new string_id("beast", "activity_dislike");
	public static final String PET_ACTIVITY_MESSAGE_SCRIPTVAR = "beastmood.ownerReceivedActivityMessage";
	public static final int PET_NORMAL_ACTIVITY = 0;
	public static final int PET_FAVORITE_ACTIVITY = 10;
	public static final int PET_DISLIKE_ACTIVITY = -10;
	public static final String PET_ACTIVITY_SCRIPTVAR = "beastmood.beastLastActivity";
	public static final String PET_KILL_MESSAGE_SCRIPTVAR = "beastmood.ownerReceivedKillMessage";
	
	public static final int EXTRACTION_LIMIT = 1;
	public static final String EXTRACTION_LIST = "enzyme.extraction_list";
	
	public static final string_id[] ENZYME_EXTRACTION_ERRORS = 
	{
		new string_id("beast", "enzyme_fail_invalid_target"),
		new string_id("beast", "enzyme_fail_too_many_extractions"),
		new string_id("beast", "enzyme_fail_beast_not_dead"),
		new string_id("beast", "enzyme_fail_target_is_pet"),
		new string_id("beast", "enzyme_fail_target_not_monster"),
		new string_id("beast", "enzyme_fail_no_loot_permission"),
		new string_id("beast", "enzyme_fail_already_harvested"),
		new string_id("beast", "beast_master_use_only")
	};
	
	public static final int ENZ_ERROR_INVALID_TARGET = 0;
	public static final int ENZ_ERROR_TOO_MANY_EXTRACTIONS = 1;
	public static final int ENZ_ERROR_NOT_DEAD = 2;
	public static final int ENZ_ERROR_TARGET_IS_PET = 3;
	public static final int ENZ_ERROR_TARGET_NOT_MONSTER = 4;
	public static final int ENZ_ERROR_NO_LOOT_PERMISSION = 5;
	public static final int ENZ_ERROR_ALREADY_HARVEST = 6;
	public static final int ENZ_ERROR_NOT_BEAST_MASTER = 7;
	
	public static final String BM_COMMAND_ATTACK = "bm_pet_attack_1";
	public static final String BM_COMMAND_FOLLOW = "bm_follow_1";
	public static final String BM_COMMAND_STAY = "bm_stay_1";
	public static final String BM_COMMAND_DISABLED = "disabled";
	public static final String BM_DISPOSITION_DEFENSIVE = "toggleBeastDefensive";
	public static final String BM_DISPOSITION_PASSIVE = "toggleBeastPassive";
	
	public static final String[] PET_BAR_DEFAULT_ARRAY = 
	{
		BM_COMMAND_DISABLED,
		BM_COMMAND_FOLLOW,
		BM_COMMAND_STAY,
		BM_COMMAND_DISABLED,
		BM_COMMAND_DISABLED,
		BM_COMMAND_DISABLED,
		BM_COMMAND_DISABLED,
		BM_DISPOSITION_DEFENSIVE,
		BM_DISPOSITION_PASSIVE
	};
	public static final string_id SID_TOGGLE_DEFENSIVE = new string_id("beast", "defensive_message");
	
	public static final int SKILL_CANNOT_COMPARE = -1;
	public static final int SKILL_DIFFERENT_LINE = 0;
	public static final int SKILL_SAME = 1;
	public static final int SKILL_HIGHER = 2;
	public static final int SKILL_LOWER = 3;
	
	public static final int ABILITY_INVALID = -1;
	public static final int ABILITY_TYPE_COMBAT = 0;
	public static final int ABILITY_TYPE_NONCOMBAT = 1;
	public static final int ABILITY_TYPE_INNATE = 2;
	
	public static final int TRAINING_VALID = 0;
	public static final int TRAINING_INVALID_PETTYPE = 1;
	public static final int TRAINING_SKILL_TOO_HIGH = 2;
	public static final int TRAINING_INSUFFICIENT_POINTS = 3;
	public static final int TRAINING_NON_IMPROVED_SKILL = 4;
	public static final int TRAINING_NO_AVAILABLE_SLOTS = 5;
	
	public static final int INJECTOR_RETURN_BAD_DATA = 0;
	public static final int INJECTOR_RETURN_WRONG_LEVEL = 1;
	public static final int INJECTOR_RETURN_HAS_BUFF = 2;
	public static final int INJECTOR_RETURN_WRONG_FAMILY = 3;
	public static final int INJECTOR_RETURN_SUCESS = 4;
	
	public static final String PET_HAPPINESS_BLOCK_SCRIPTVAR = "beastmood.beastHappinessBlock";
	
	public static final int HOLO_BEAST_TYPE2_QUALITY3 = 6;
	
	public static final int HOLO_BEAST_TYPE3_QUALITY3 = 10;
	
	public static final int HOLOGRAM_MAX_SETTING = 11;
	
	public static final String HOLO_BEAST_RARE_COLOR_OBJVAR = "rareHoloBeastColor";
	
	
	public static void blog(String text) throws InterruptedException
	{
		if (BEAST_DEBUG)
		{
			LOG("beast_lib", text);
		}
	}
	
	
	public static boolean isBeast(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast) || !exists(beast))
		{
			return false;
		}
		
		if (hasScript(beast, "ai.beast"))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isBeastMaster(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		int skill = getSkillStatisticModifier(player, "expertise_bm_base_mod");
		
		if (skill > 0)
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isValidBeast(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast) || !exists(beast))
		{
			return false;
		}
		
		if (!callable.hasCallableCD(beast))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean isValidBCD(obj_id bcd) throws InterruptedException
	{
		if (!isIdValid(bcd) || !exists(bcd))
		{
			return false;
		}
		
		if (!hasObjVar(bcd, OBJVAR_BEAST_TYPE))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean isValidPlayer(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static float getBeastScaleByLevel(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast))
		{
			return 1.0f;
		}
		
		String beastType = getBeastType(beast);
		dictionary beastDict = utils.dataTableGetRow(BEASTS_TABLE, beastType);
		
		float minScale = beastDict.getFloat("minScale");
		float maxScale = beastDict.getFloat("maxScale");
		
		int level = getLevel(beast);
		
		float scaleDifference = maxScale - minScale;
		
		if (scaleDifference <= 0)
		{
			return minScale;
		}
		
		return (float)(minScale + (scaleDifference * ((float)level / 90.0f)));
	}
	
	
	public static obj_id getBCDPlayer(obj_id bcd) throws InterruptedException
	{
		obj_id datapad = getContainedBy(bcd);
		obj_id player = getContainedBy(datapad);
		
		if (!isIdValid(player) || !isPlayer(player))
		{
			if (hasObjVar(bcd, OBJVAR_BEAST_BCD_OWNER))
			{
				obj_id owner = getObjIdObjVar(bcd, OBJVAR_BEAST_BCD_OWNER);
				if (isIdValid(owner))
				{
					return owner;
				}
				else
				{
					return null;
				}
			}
			else
			{
				return null;
			}
		}
		
		return player;
	}
	
	
	public static void setBCDBeastCalled(obj_id bcd, obj_id beast) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return;
		}
		
		setBCDBeastCalledTime(bcd, getGameTime());
		
		callable.setCDCallable(bcd, beast);
	}
	
	
	public static obj_id getBCDBeastCalled(obj_id bcd) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return null;
		}
		
		return callable.getCDCallable(bcd);
	}
	
	
	public static void setBeastOnPlayer(obj_id player, obj_id beast) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return;
		}
		
		callable.setCallable(player, beast, callable.CALLABLE_TYPE_COMBAT_PET);
	}
	
	
	public static obj_id getBeastOnPlayer(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return null;
		}
		
		obj_id beast = callable.getCallable(player, callable.CALLABLE_TYPE_COMBAT_PET);
		
		if (!isValidBeast(beast) || !beast_lib.isBeast(beast))
		{
			beast = null;
		}
		
		return beast;
	}
	
	
	public static void setBCDBeastTimeStored(obj_id bcd, int time) throws InterruptedException
	{
		if (!isIdValid(bcd))
		{
			return;
		}
		
		setObjVar(bcd, "beast.timeStored", time);
	}
	
	
	public static int getBCDBeastTimeStored(obj_id bcd) throws InterruptedException
	{
		if (!isIdValid(bcd))
		{
			return 0;
		}
		
		int time = 0;
		
		if (hasObjVar(bcd, "beast.timeStored"))
		{
			time = getIntObjVar(bcd, "beast.timeStored");
		}
		
		return time;
	}
	
	
	public static void setBCDBeastCalledTime(obj_id bcd, int time) throws InterruptedException
	{
		if (!isIdValid(bcd))
		{
			return;
		}
		
		setObjVar(bcd, "beast.timeCalled", time);
	}
	
	
	public static int getBCDBeastCalledTime(obj_id bcd) throws InterruptedException
	{
		if (!isIdValid(bcd))
		{
			return 0;
		}
		
		int time = 0;
		
		if (hasObjVar(bcd, "beast.timeStored"))
		{
			time = getIntObjVar(bcd, "beast.timeCalled");
		}
		
		return time;
	}
	
	
	public static String getBeastLocalizedName(obj_id bcd) throws InterruptedException
	{
		String beastType = getBCDBeastType(bcd);
		String beastDisplayType = stripBmFromType(beastType);
		string_id beastTypeSid = new string_id("monster_name", beastDisplayType);
		
		return localize(beastTypeSid);
	}
	
	
	public static void setBCDBeastName(obj_id bcd, String beastName) throws InterruptedException
	{
		if (!isIdValid(bcd) || beastName == null)
		{
			return;
		}
		
		setName(bcd, beastName);
		
		setObjVar(bcd, "beast.beastName", beastName);
	}
	
	
	public static boolean hasBCDBeastName(obj_id bcd) throws InterruptedException
	{
		if (hasObjVar(bcd, "beast.beastName"))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static String getBCDBeastName(obj_id bcd) throws InterruptedException
	{
		String beastName = getBeastLocalizedName(bcd);
		
		if (hasBCDBeastName(bcd))
		{
			beastName = getStringObjVar(bcd, "beast.beastName");
		}
		
		return beastName;
	}
	
	
	public static void setBeastName(obj_id beast, String beastName) throws InterruptedException
	{
		if (!isValidBeast(beast))
		{
			return;
		}
		
		setName(beast, beastName);
	}
	
	
	public static boolean hasBeastName(obj_id beast) throws InterruptedException
	{
		if (!isValidBeast(beast))
		{
			return false;
		}
		
		return hasBCDBeastName(getBeastBCD(beast));
	}
	
	
	public static String getBeastName(obj_id beast) throws InterruptedException
	{
		if (!isValidBeast(beast))
		{
			return null;
		}
		
		return getBCDBeastName(getBeastBCD(beast));
	}
	
	
	public static void setBCDBeastType(obj_id bcd, String beastType) throws InterruptedException
	{
		if (!isIdValid(bcd))
		{
			return;
		}
		
		int hashBeastType = incubator.getHashType(beastType);
		setObjVar(bcd, OBJVAR_BEAST_TYPE, hashBeastType);
	}
	
	
	public static String getBCDBeastType(obj_id bcd) throws InterruptedException
	{
		String beastType = "";
		
		if (hasObjVar(bcd, OBJVAR_BEAST_TYPE))
		{
			int hashBeastType = getIntObjVar(bcd, OBJVAR_BEAST_TYPE);
			beastType = incubator.convertHashTypeToString(hashBeastType);
		}
		
		return beastType;
	}
	
	
	public static void setBeastType(obj_id beast, String template) throws InterruptedException
	{
		if (!isIdValid(beast) || template == null || template.length() <= 0)
		{
			return;
		}
		
		int row = dataTableSearchColumnForString(template, "template", BEASTS_TABLE);
		
		if (row < 0)
		{
			return;
		}
		
		dictionary dict = dataTableGetRow(BEASTS_TABLE, row);
		
		if (dict == null)
		{
			return;
		}
		
		String type = dict.getString("beastType");
		
		obj_id bcd = getBeastBCD(beast);
		
		if (!isValidBCD(bcd))
		{
			return;
		}
		
		obj_id player = getBCDPlayer(bcd);
		
		if (!isValidPlayer(player))
		{
			return;
		}
		
		storeBeast(bcd);
		
		setBCDBeastType(bcd, type);
		
		createBeastFromBCD(player, bcd);
	}
	
	
	public static String getBeastType(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast))
		{
			return "";
		}
		
		String template = getTemplateName(beast);
		String[] templateSplit = split(template, '/');
		template = templateSplit[templateSplit.length - 1];
		int row = dataTableSearchColumnForString(template, "template", BEASTS_TABLE);
		
		dictionary dict = dataTableGetRow(BEASTS_TABLE, row);
		
		if (dict == null)
		{
			return "";
		}
		
		String type = dict.getString("beastType");
		
		return type;
	}
	
	
	public static void setBeastBCD(obj_id beast, obj_id bcd) throws InterruptedException
	{
		if (!isIdValid(beast))
		{
			return;
		}
		
		callable.setCallableCD(beast, bcd);
	}
	
	
	public static obj_id getBeastBCD(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast))
		{
			return null;
		}
		
		obj_id bcd = callable.getCallableCD(beast);
		
		if (!isValidBCD(bcd))
		{
			bcd = null;
		}
		
		return bcd;
	}
	
	
	public static void setBCDBeastLevel(obj_id bcd, int level) throws InterruptedException
	{
		if (!isIdValid(bcd) || level < 1 || level > 90)
		{
			return;
		}
		
		setObjVar(bcd, "beast.level", level);
	}
	
	
	public static int getBCDBeastLevel(obj_id bcd) throws InterruptedException
	{
		if (!isIdValid(bcd))
		{
			return -1;
		}
		
		int level = 0;
		
		if (hasObjVar(bcd, "beast.level"))
		{
			level = getIntObjVar(bcd, "beast.level");
		}
		
		return level;
	}
	
	
	public static void setBeastLevel(obj_id beast, int level) throws InterruptedException
	{
		if (!isIdValid(beast) || level < 1 || level > 90)
		{
			return;
		}
		
		setLevel(beast, level);
	}
	
	
	public static int getBeastLevel(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast))
		{
			return -1;
		}
		
		return getLevel(beast);
	}
	
	
	public static void setBCDBeastExperience(obj_id bcd, int exp) throws InterruptedException
	{
		if (!isIdValid(bcd) || exp < 0)
		{
			return;
		}
		
		setObjVar(bcd, "beast.experience", exp);
	}
	
	
	public static int getBCDBeastExperience(obj_id bcd) throws InterruptedException
	{
		if (!isIdValid(bcd))
		{
			return -1;
		}
		
		int exp = 0;
		
		if (hasObjVar(bcd, "beast.experience"))
		{
			exp = getIntObjVar(bcd, "beast.experience");
		}
		
		return exp;
	}
	
	
	public static void setBeastExperience(obj_id beast, int exp) throws InterruptedException
	{
		if (!isIdValid(beast) || exp < 0)
		{
			return;
		}
		
		setObjVar(beast, "beast.experience", exp);
	}
	
	
	public static int getBeastExperience(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast))
		{
			return -1;
		}
		
		int exp = 0;
		
		if (hasObjVar(beast, "beast.experience"))
		{
			exp = getIntObjVar(beast, "beast.experience");
		}
		
		return exp;
	}
	
	
	public static void setBCDBeastHealth(obj_id bcd, int health) throws InterruptedException
	{
		if (!isIdValid(bcd) || health < 0)
		{
			return;
		}
		
		setObjVar(bcd, "beast.health", health);
	}
	
	
	public static int getBCDBeastHealth(obj_id bcd) throws InterruptedException
	{
		if (!isIdValid(bcd))
		{
			return -1;
		}
		
		int health = 0;
		
		if (hasObjVar(bcd, "beast.health"))
		{
			health = getIntObjVar(bcd, "beast.health");
		}
		else
		{
			return -1;
		}
		
		return health;
	}
	
	
	public static void setBCDBeastCanLevel(obj_id bcd, boolean canLevel) throws InterruptedException
	{
		if (!isIdValid(bcd))
		{
			return;
		}
		
		setObjVar(bcd, "beast.canLevel", canLevel);
	}
	
	
	public static boolean getBCDBeastCanLevel(obj_id bcd) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return false;
		}
		
		boolean canLevel = false;
		
		if (hasObjVar(bcd, "beast.canLevel"))
		{
			canLevel = getBooleanObjVar(bcd, "beast.canLevel");
		}
		
		return canLevel;
	}
	
	
	public static void setBeastCanLevel(obj_id beast, boolean canLevel) throws InterruptedException
	{
		if (!isIdValid(beast))
		{
			return;
		}
		
		setObjVar(beast, "beast.canLevel", canLevel);
	}
	
	
	public static boolean getBeastCanLevel(obj_id beast) throws InterruptedException
	{
		if (!isValidBeast(beast))
		{
			return false;
		}
		
		boolean canLevel = false;
		
		if (hasObjVar(beast, "beast.canLevel"))
		{
			canLevel = getBooleanObjVar(beast, "beast.canLevel");
		}
		
		return canLevel;
	}
	
	
	public static void setBCDBeastIsDead(obj_id bcd, boolean beastIsDead) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return;
		}
		
		setObjVar(bcd, "beast.isDead", beastIsDead);
	}
	
	
	public static boolean getBCDBeastIsDead(obj_id bcd) throws InterruptedException
	{
		if (!isIdValid(bcd))
		{
			return true;
		}
		
		boolean beastIsDead = false;
		
		if (hasObjVar(bcd, "beast.isDead"))
		{
			beastIsDead = getBooleanObjVar(bcd, "beast.isDead");
		}
		
		return beastIsDead;
	}
	
	
	public static void setBCDBeastDefensive(obj_id bcd, obj_id player, boolean beastIsDefensive) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return;
		}
		
		obj_id beast = getBCDBeastCalled(bcd);
		
		if (!isValidBeast(beast))
		{
			return;
		}
		
		prose_package pp = new prose_package();
		
		if (beastIsDefensive)
		{
			utils.removeScriptVar(beast, "petIgnoreAttacks");
			pp = prose.setTT(pp, "DEFENSIVE");
		}
		else
		{
			utils.setScriptVar(beast, "petIgnoreAttacks", getGameTime());
			pp = prose.setTT(pp, "PASSIVE");
		}
		
		pp = prose.setStringId(pp, SID_TOGGLE_DEFENSIVE);
		sendSystemMessageProse(player, pp);
		
		setObjVar(bcd, "beast.isDefensive", beastIsDefensive);
		
		setToggledCommands(player, bcd);
	}
	
	
	public static boolean getBCDBeastDefensive(obj_id bcd) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return false;
		}
		
		if (hasObjVar(bcd, "beast.isDefensive"))
		{
			return getBooleanObjVar(bcd, "beast.isDefensive");
		}
		
		return false;
	}
	
	
	public static void setBeastDefensive(obj_id beast, obj_id player, boolean beastIsDefensive) throws InterruptedException
	{
		if (!isValidBeast(beast))
		{
			return;
		}
		
		setBCDBeastDefensive(getBeastBCD(beast), player, beastIsDefensive);
	}
	
	
	public static boolean getBeastDefensive(obj_id beast) throws InterruptedException
	{
		if (!isValidBeast(beast))
		{
			return false;
		}
		
		return getBCDBeastDefensive(getBeastBCD(beast));
	}
	
	
	public static obj_id[] getDatapadBCDs(obj_id player) throws InterruptedException
	{
		Vector datapadBCDs = new Vector();
		datapadBCDs.setSize(0);
		
		if (!isIdValid(player) || !exists(player))
		{
			return null;
		}
		
		obj_id datapad = utils.getPlayerDatapad(player);
		
		if (!isIdValid(datapad))
		{
			return null;
		}
		
		obj_id[] dataItems = getContents(datapad);
		
		for (int i = 0; i < dataItems.length; i++)
		{
			testAbortScript();
			if (beast_lib.isValidBCD(dataItems[i]))
			{
				datapadBCDs = utils.addElement(datapadBCDs, dataItems[i]);
			}
		}
		
		obj_id[] _datapadBCDs = new obj_id[0];
		if (datapadBCDs != null)
		{
			_datapadBCDs = new obj_id[datapadBCDs.size()];
			datapadBCDs.toArray(_datapadBCDs);
		}
		return _datapadBCDs;
	}
	
	
	public static int getTotalBeastControlDevices(obj_id player) throws InterruptedException
	{
		Vector datapadBCDs = new Vector();
		datapadBCDs.setSize(0);
		
		if (!isIdValid(player) || !exists(player))
		{
			return 0;
		}
		
		obj_id datapad = utils.getPlayerDatapad(player);
		
		if (!isIdValid(datapad))
		{
			return 0;
		}
		
		obj_id[] dataItems = getContents(datapad);
		if (dataItems == null || dataItems.length < 1)
		{
			return 0;
		}
		
		int numStored = 0;
		for (int i = 0; i < dataItems.length; i++)
		{
			testAbortScript();
			if (beast_lib.isValidBCD(dataItems[i]))
			{
				numStored++;
			}
		}
		return numStored;
	}
	
	
	public static int getTotalBeastsCalled(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return 0;
		}
		
		obj_id datapad = utils.getPlayerDatapad(player);
		
		if (!isIdValid(datapad))
		{
			return 0;
		}
		
		obj_id[] datapadContents = getContents(datapad);
		
		if (datapadContents == null)
		{
			return 0;
		}
		
		int beastTotal = 0;
		
		for (int i = 0; i < datapadContents.length; i++)
		{
			testAbortScript();
			if (hasObjVar(datapadContents[i], "beast.called") && isValidBeast(getBCDBeastCalled(datapadContents[i])))
			{
				beastTotal++;
			}
		}
		
		return beastTotal;
	}
	
	
	public static int getBCDBeastHappiness(obj_id bcd) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return 0;
		}
		
		if (!hasBCDBeastHappiness(bcd))
		{
			setBCDBeastHappiness(bcd, 0);
			
			return 0;
		}
		
		return getIntObjVar(bcd, OBJVAR_BEAST_HAPPINESS);
	}
	
	
	public static void setBCDBeastHappiness(obj_id bcd, int happiness) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return;
		}
		
		setObjVar(bcd, OBJVAR_BEAST_HAPPINESS, happiness);
	}
	
	
	public static boolean hasBCDBeastHappiness(obj_id bcd) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return false;
		}
		
		if (hasObjVar(bcd, OBJVAR_BEAST_HAPPINESS))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static int getBeastHappiness(obj_id beast) throws InterruptedException
	{
		if (!isValidBeast(beast))
		{
			return 0;
		}
		
		return getBCDBeastHappiness(getBeastBCD(beast));
	}
	
	
	public static void setBeastHappiness(obj_id beast, int happiness) throws InterruptedException
	{
		if (!isValidBeast(beast))
		{
			return;
		}
		
		setBCDBeastHappiness(getBeastBCD(beast), happiness);
	}
	
	
	public static float getBCDBeastLoyalty(obj_id bcd) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return 0;
		}
		
		if (!hasBCDBeastLoyalty(bcd))
		{
			setBCDBeastLoyalty(bcd, 1.0f);
			
			return 0;
		}
		
		return getFloatObjVar(bcd, PET_LOYALTY_OBJVAR);
	}
	
	
	public static void setBCDBeastLoyalty(obj_id bcd, float newLoyalty) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return;
		}
		
		setObjVar(bcd, PET_LOYALTY_OBJVAR, newLoyalty);
	}
	
	
	public static boolean hasBCDBeastLoyalty(obj_id bcd) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return false;
		}
		
		if (hasObjVar(bcd, PET_LOYALTY_OBJVAR))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static float getBeastLoyalty(obj_id beast) throws InterruptedException
	{
		if (!isValidBeast(beast))
		{
			return 0;
		}
		
		return getBCDBeastLoyalty(getBeastBCD(beast));
	}
	
	
	public static void setBeastLoyalty(obj_id beast, float newLoyalty) throws InterruptedException
	{
		if (!isValidBeast(beast))
		{
			return;
		}
		
		if (newLoyalty < 1.0f)
		{
			newLoyalty = 1.0f;
		}
		else if (newLoyalty > LOYALTY_MAX)
		{
			newLoyalty = LOYALTY_MAX;
		}
		
		setBCDBeastLoyalty(getBeastBCD(beast), newLoyalty);
	}
	
	
	public static void incrementBeastLoyalty(obj_id bcd) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return;
		}
		
		float loyalty = getBCDBeastLoyalty(bcd);
		
		if (loyalty >= LOYALTY_MAX)
		{
			return;
		}
		
		int happiness = getBCDBeastHappiness(bcd);
		
		if (happiness < 0)
		{
			happiness = 0;
		}
		
		float modifiedHappiness = 10 * (1.0f + ((float)happiness / 50.0f));
		
		float newLoyalty = loyalty + modifiedHappiness;
		
		if (loyalty < LOYALTY_LVL_TWO && newLoyalty >= LOYALTY_LVL_TWO)
		{
			incrementBeastLoyaltyLevel(bcd);
		}
		
		if (loyalty < LOYALTY_LVL_THREE && newLoyalty >= LOYALTY_LVL_THREE)
		{
			incrementBeastLoyaltyLevel(bcd);
		}
		
		if (loyalty < LOYALTY_LVL_FOUR && newLoyalty >= LOYALTY_LVL_FOUR)
		{
			incrementBeastLoyaltyLevel (bcd);
		}
		
		if (loyalty < LOYALTY_LVL_FIVE && newLoyalty >= LOYALTY_LVL_FIVE)
		{
			incrementBeastLoyaltyLevel(bcd);
		}
		
		setBCDBeastLoyalty(bcd, newLoyalty);
	}
	
	
	public static void decrementBeastLoyalty(obj_id bcd, float loyaltyDecrement) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return;
		}
		
		if (!hasObjVar(bcd, PET_LOYALTY_OBJVAR))
		{
			setObjVar(bcd, PET_LOYALTY_OBJVAR, 1.0f);
			return;
		}
		
		float loyalty = getBCDBeastLoyalty(bcd);
		
		if (loyalty <= 101)
		{
			return;
		}
		
		float newLoyalty = loyalty - loyaltyDecrement;
		
		if (loyalty >= LOYALTY_LVL_TWO && newLoyalty < LOYALTY_LVL_TWO)
		{
			decrementBeastLoyaltyLevel(bcd);
		}
		
		if (loyalty >= LOYALTY_LVL_THREE && newLoyalty < LOYALTY_LVL_THREE)
		{
			decrementBeastLoyaltyLevel (bcd);
		}
		
		if (loyalty >= LOYALTY_LVL_FOUR && newLoyalty < LOYALTY_LVL_FOUR)
		{
			decrementBeastLoyaltyLevel(bcd);
		}
		
		if (loyalty >= LOYALTY_LVL_FIVE && newLoyalty < LOYALTY_LVL_FIVE)
		{
			decrementBeastLoyaltyLevel(bcd);
		}
		
		setBCDBeastLoyalty(bcd, newLoyalty);
	}
	
	
	public static int getBCDBeastLoyaltyLevel(obj_id bcd) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return 0;
		}
		
		if (!hasBCDBeastLoyalty(bcd))
		{
			setBCDBeastLoyaltyLevel(bcd, 1);
			
			return 0;
		}
		
		return getIntObjVar(bcd, PET_LOYALTY_LEVEL_OBJVAR);
	}
	
	
	public static void setBCDBeastLoyaltyLevel(obj_id bcd, int newLevel) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return;
		}
		
		if (newLevel < 1)
		{
			newLevel = 1;
		}
		else if (newLevel > 5)
		{
			newLevel = 5;
		}
		
		setObjVar(bcd, PET_LOYALTY_LEVEL_OBJVAR, newLevel);
	}
	
	
	public static boolean hasBCDBeastLoyaltyLevel(obj_id bcd) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return false;
		}
		
		if (hasObjVar(bcd, PET_LOYALTY_LEVEL_OBJVAR))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static int getBeastLoyaltyLevel(obj_id beast) throws InterruptedException
	{
		if (!isValidBeast(beast))
		{
			return 0;
		}
		
		return getBCDBeastLoyaltyLevel(getBeastBCD(beast));
	}
	
	
	public static void setBeastLoyaltyLevel(obj_id beast, int newLevel) throws InterruptedException
	{
		if (!isValidBeast(beast))
		{
			return;
		}
		
		setBCDBeastLoyalty(getBeastBCD(beast), newLevel);
	}
	
	
	public static void incrementBeastLoyaltyLevel(obj_id bcd) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return;
		}
		
		obj_id beast = getBCDBeastCalled(bcd);
		obj_id player = getBCDPlayer(bcd);
		
		if (!isValidBeast(beast) || !isValidPlayer(player))
		{
			return;
		}
		
		int level = 1;
		
		if (hasBCDBeastLoyaltyLevel(bcd))
		{
			level = getBCDBeastLoyaltyLevel(bcd) + 1;
		}
		
		setBCDBeastLoyaltyLevel(bcd, level);
		
		playClientEffectObj(beast, "clienteffect/level_granted.cef", beast, null);
		sendSystemMessage(player, SID_LOYALTY_LEVEL_GAIN);
		
		updateBeastStats(bcd, beast);
	}
	
	
	public static void decrementBeastLoyaltyLevel(obj_id bcd) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return;
		}
		
		obj_id beast = getBCDBeastCalled(bcd);
		obj_id player = getBCDPlayer(bcd);
		
		if (!isValidBeast(beast) || !isValidPlayer(player))
		{
			return;
		}
		
		int level = 1;
		
		if (hasBCDBeastLoyaltyLevel(bcd))
		{
			level = getBCDBeastLoyaltyLevel(bcd) - 1;
		}
		
		setBCDBeastLoyaltyLevel(bcd, level);
		
		playClientEffectObj(beast, "clienteffect/medic_traumatize.cef", beast, null);
		sendSystemMessage(player, SID_LOYALTY_LEVEL_LOSS);
		
		updateBeastStats(bcd, beast);
	}
	
	
	public static int[] getBCDBeastFood(obj_id bcd) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return null;
		}
		
		if (!hasBCDBeastFood(bcd))
		{
			
			int[] newFood = new int[PET_FOOD_MAX_ARRAYS];
			
			newFood[PET_WHICH_FOOD] = PET_NORMAL_FOOD;
			newFood[PET_TIME_SINCE_FEEDING] = getGameTime();
			
			setBCDBeastFood(bcd, newFood);
			
			return newFood;
		}
		
		return getIntArrayObjVar(bcd, OBJVAR_BEAST_FOOD);
	}
	
	
	public static void setBCDBeastFood(obj_id bcd, int[] newFood) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return;
		}
		
		if (newFood == null || newFood.length < PET_FOOD_MAX_ARRAYS)
		{
			newFood = new int[PET_FOOD_MAX_ARRAYS];
			
			newFood[PET_WHICH_FOOD] = PET_NORMAL_FOOD;
			newFood[PET_TIME_SINCE_FEEDING] = getGameTime();
		}
		
		setObjVar(bcd, OBJVAR_BEAST_FOOD, newFood);
	}
	
	
	public static boolean hasBCDBeastFood(obj_id bcd) throws InterruptedException
	{
		if (!isValidBCD(bcd))
		{
			return false;
		}
		
		if (hasObjVar(bcd, OBJVAR_BEAST_FOOD))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static int[] getBeastFood(obj_id beast) throws InterruptedException
	{
		if (!isValidBeast(beast))
		{
			return null;
		}
		
		return getBCDBeastFood(getBeastBCD(beast));
	}
	
	
	public static void setBeastFood(obj_id beast, int[] newFood) throws InterruptedException
	{
		if (!isValidBeast(beast))
		{
			return;
		}
		
		setBCDBeastFood(getBeastBCD(beast), newFood);
	}
	
	
	public static int getBCDBeastLastFedTime(obj_id bcd) throws InterruptedException
	{
		
		int storedTime = getBCDBeastTimeStored(bcd);
		int calledTime = getBCDBeastCalledTime(bcd);
		int currentTime = getGameTime();
		int fedTimeDelta = 0;
		
		int[] beastFood = getBCDBeastFood(bcd);
		
		if (calledTime > beastFood[PET_TIME_SINCE_FEEDING])
		{
			
			fedTimeDelta = (storedTime - beastFood[PET_TIME_SINCE_FEEDING]) + (currentTime - calledTime);
		}
		else
		{
			
			if (storedTime > beastFood[PET_TIME_SINCE_FEEDING])
			{
				fedTimeDelta = currentTime - storedTime;
			}
			else
			{
				
				fedTimeDelta = currentTime - beastFood[PET_TIME_SINCE_FEEDING];
			}
		}
		
		return fedTimeDelta;
	}
	
	
	public static void updateBeastHappiness(obj_id bcd, obj_id beast) throws InterruptedException
	{
		if (!isValidBCD(bcd) || !isValidBeast(beast))
		{
			return;
		}
		
		obj_id player = getBCDPlayer(bcd);
		
		if (!isValidPlayer(player))
		{
			return;
		}
		
		int happiness = 0;
		float loyalty = getBCDBeastLoyalty(bcd);
		int loyaltyLevel = getBCDBeastLoyaltyLevel(bcd);
		
		int[] beastFood = getBCDBeastFood(bcd);
		
		if (getBCDBeastLastFedTime(bcd) > 3600)
		{
			dictionary params = new dictionary();
			params.put("beastId", beast);
			
			messageTo(bcd, "beastHungry", params, 1, false);
		}
		
		happiness += beastFood[PET_WHICH_FOOD];
		
		if (loyaltyLevel >= 3)
		{
			
			happiness += PET_HAPPINESS_LOYALTY_BONUS;
		}
		
		if (utils.hasScriptVar(bcd, PET_LOCATION_SCRIPTVAR))
		{
			
			happiness += utils.getIntScriptVar(bcd, PET_LOCATION_SCRIPTVAR);
		}
		
		if (utils.hasScriptVar(bcd, PET_ACTIVITY_SCRIPTVAR))
		{
			
			happiness += utils.getIntScriptVar(bcd, PET_ACTIVITY_SCRIPTVAR);
		}
		
		happiness += (int)getSkillStatisticModifier(player, "expertise_bm_pet_happiness");
		
		setBCDBeastHappiness(bcd, happiness);
		
		applyHappinessBuffIcon(beast);
	}
	
	
	public static void setBeastLinks(obj_id player, obj_id bcd, obj_id beast) throws InterruptedException
	{
		if (!isIdValid(player) || !isIdValid(bcd) || !isIdValid(beast))
		{
			return;
		}
		
		setBCDBeastCalled(bcd, beast);
		setBeastBCD(beast, bcd);
		setBeastOnPlayer(player, beast);
		
		setMaster(beast, player);
	}
	
	
	public static void removeBeastLinks(obj_id player, obj_id bcd, obj_id beast) throws InterruptedException
	{
		if (!isIdValid(player) || !isIdValid(bcd) || !isIdValid(beast))
		{
			return;
		}
		
		setBCDBeastCalled(bcd, null);
		setBeastBCD(beast, null);
		setBeastOnPlayer(player, null);
		setMaster(beast, null);
		
	}
	
	
	public static obj_id createBCDFromEgg(obj_id player, obj_id egg) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			blog("BCD could not be made because the player is invalid.");
			return null;
		}
		
		if (callable.hasMaxStoredCombatPets(player))
		{
			sendSystemMessage(player, SID_MAXIMUM_COMBAT_CONTROL_DEVICES);
			return null;
		}
		
		if (!isIdValid(egg))
		{
			blog("BCD could not be made because its egg is invalid.");
			return null;
		}
		
		int hashType = getIntObjVar(egg, OBJVAR_BEAST_TYPE);
		String beastType = incubator.convertHashTypeToString(hashType);
		
		if (beastType.equals("") || beastType == null)
		{
			blog("BCD could not be made because beastType is null.");
			return null;
		}
		
		dictionary beastDict = utils.dataTableGetRow(BEASTS_TABLE, beastType);
		
		if (beastDict == null)
		{
			blog("BCD could not be created because no dictionary exists in beast/beasts.tab. for the beastType: "+ beastType);
			return null;
		}
		
		String intangibleTemplateName = beastDict.getString("template");
		
		if (intangibleTemplateName.equals("") || intangibleTemplateName == null)
		{
			blog("BCD could not be made because intangibleTemplateName is null.");
			return null;
		}
		
		obj_id datapad = utils.getPlayerDatapad(player);
		
		if (!isIdValid(datapad))
		{
			blog("BCD could not be made because datapad is null.");
			return null;
		}
		
		blog("BCD attempting to create object for template: "+ INTANGIBLE_TEMPLATE_PREFIX + intangibleTemplateName + " in datapad: "+ datapad);
		
		obj_id beastControlDevice = createObject(INTANGIBLE_TEMPLATE_PREFIX + intangibleTemplateName, datapad, "");
		
		if (!isIdValid(beastControlDevice))
		{
			beastControlDevice = createObject(pet_lib.PET_CTRL_DEVICE_TEMPLATE, datapad, "");
			
			if (!isIdValid(beastControlDevice))
			{
				sendSystemMessage(player, pet_lib.SID_SYS_TOO_MANY_STORED_PETS);
				return null;
			}
		}
		
		if (!isIdValid(beastControlDevice))
		{
			blog("Beast control device not created for "+ beastType);
			return null;
		}
		
		setBCDBeastType(beastControlDevice, beastType);
		
		if (hasObjVar(egg, OBJVAR_OLD_PET_IDENTIFIER))
		{
			String oldName = getAssignedName(egg);
			if (oldName != null && !oldName.equals(""))
			{
				setBCDBeastName(beastControlDevice, oldName);
			}
			else
			{
				setName(beastControlDevice, getBeastLocalizedName(beastControlDevice));
			}
			setObjVar(beastControlDevice, OBJVAR_OLD_PET_IDENTIFIER, 1);
			if (hasObjVar(egg, OBJVAR_OLD_PET_REHUED))
			{
				setObjVar(beastControlDevice, OBJVAR_OLD_PET_REHUED, 1);
			}
		}
		else
		{
			setName(beastControlDevice, getBeastLocalizedName(beastControlDevice));
		}
		
		setBCDIncubationStats(beastControlDevice, egg);
		
		setBCDBeastHueFromEgg(beastControlDevice, egg);
		
		setBCDBeastLevel(beastControlDevice, 1);
		
		setBCDBeastExperience(beastControlDevice, 0);
		
		setBCDBeastCanLevel(beastControlDevice, true);
		
		setBCDBeastCalled(beastControlDevice, null);
		
		attachScript(beastControlDevice, "ai.beast_control_device");
		
		setupHappinessLoyalty(beastControlDevice);
		
		return beastControlDevice;
	}
	
	
	public static String stripBmFromType(String beastType) throws InterruptedException
	{
		if (beastType.indexOf("bm_") > -1)
		{
			String[] splitType = split(beastType, '_');
			beastType = splitType[1];
			
			for (int i = 2; i < splitType.length; ++i)
			{
				testAbortScript();
				beastType += "_"+ splitType[i];
			}
		}
		return beastType;
	}
	
	
	public static obj_id createBasicBeastFromObject(obj_id source) throws InterruptedException
	{
		location spawnLocation = getLocation(source);
		return createBasicBeastFromObject(source, spawnLocation);
	}
	
	
	public static obj_id createBasicBeastFromObject(obj_id source, location spawnLocation) throws InterruptedException
	{
		if (!isIdValid(source))
		{
			blog("createBeastFromBCD: Beast Statue could not be made because the source is invalid.");
			return null;
		}
		
		String beastType = getBCDBeastType(source);
		
		if (beastType.equals("") || beastType == null)
		{
			blog("createBeastFromBCD: Beast Statue could not be made because beastType is null.");
			return null;
		}
		
		dictionary beastDict = utils.dataTableGetRow(BEASTS_TABLE, beastType);
		
		if (beastDict == null)
		{
			blog("createBeastFromBCD: Beast could not be created because no dictionary exists in beast/beasts.tab.");
			return null;
		}
		
		String mobileTemplateName = beastDict.getString("template");
		
		if (mobileTemplateName.equals("") || mobileTemplateName == null)
		{
			blog("createBeastFromBCD: Beast could not be made because mobileTemplateName is null.");
			return null;
		}
		
		obj_id beast = createObject(MOBILE_TEMPLATE_PREFIX + mobileTemplateName, spawnLocation);
		
		setBeastBCD(beast, source);
		
		initializeBeastColor(beast);
		
		return beast;
	}
	
	
	public static obj_id createBasicBeastFromPlayer(obj_id player, obj_id bcd) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			blog("createBeastFromBCD: Beast could not be made because the player is invalid.");
			return null;
		}
		
		if (!isBeastMaster(player))
		{
			sendSystemMessage(player, SID_NOT_BEAST_MASTER);
			return null;
		}
		
		String beastType = getBCDBeastType(bcd);
		
		if (beastType.equals("") || beastType == null)
		{
			blog("createBeastFromBCD: Beast could not be made because beastType is null.");
			return null;
		}
		
		dictionary beastDict = utils.dataTableGetRow(BEASTS_TABLE, beastType);
		
		if (beastDict == null)
		{
			blog("createBeastFromBCD: Beast could not be created because no dictionary exists in beast/beasts.tab.");
			return null;
		}
		
		String mobileTemplateName = beastDict.getString("template");
		
		if (mobileTemplateName.equals("") || mobileTemplateName == null)
		{
			blog("createBeastFromBCD: Beast could not be made because mobileTemplateName is null.");
			return null;
		}
		
		location spawnLocation = getLocation(player);
		
		obj_id beast = createObject(MOBILE_TEMPLATE_PREFIX + mobileTemplateName, spawnLocation);
		
		setBeastLinks(player, bcd, beast);
		
		initializeBeastColor(beast);
		
		return beast;
	}
	
	
	public static obj_id createBeastFromBCD(obj_id player, obj_id bcd) throws InterruptedException
	{
		obj_id beast = createBasicBeastFromPlayer(player, bcd);
		
		if (!isIdValid(beast))
		{
			String beastType = getBCDBeastType(bcd);
			dictionary beastDict = utils.dataTableGetRow(BEASTS_TABLE, beastType);
			String mobileTemplateName = beastDict.getString("template");
			
			blog("createBeastFromBCD: Beast failed to be created by createObject: "+ (MOBILE_TEMPLATE_PREFIX + mobileTemplateName));
			return null;
		}
		
		utils.setScriptVar(beast, "beast.creationInProgress", true);
		
		initializeBeastScripts(beast);
		
		initializeBeastStats(bcd, beast);
		
		setYaw(beast, rand(0.0f, 360.0f));
		
		updatePetAbilityList(bcd, beast);
		
		setBeastmasterPet(player, beast);
		
		utils.removeScriptVar(beast, "beast.creationInProgress");
		
		if (BEAST_DEBUG)
		{
			String beastType = getBCDBeastType(bcd);
			location spawnLocation = getLocation(player);
			blog(getName(getSelf()) + " creating beastType: "+ beastType + " at "+ spawnLocation);
		}
		
		setAttributeAttained(beast, attrib.BEAST);
		
		return beast;
	}
	
	
	public static void initializeBeastScripts(obj_id beast) throws InterruptedException
	{
		attachScript(beast, "ai.ai");
		attachScript(beast, "ai.creature_combat");
		attachScript(beast, "systems.combat.combat_actions");
		attachScript(beast, "ai.beast");
	}
	
	
	public static void initializeBeastStats(obj_id bcd, obj_id beast) throws InterruptedException
	{
		if (!isValidBCD(bcd) || !isValidBeast(beast))
		{
			return;
		}
		
		int level = getBCDBeastLevel(bcd);
		
		setBeastLevel(beast, level);
		
		String beastType = getBCDBeastType(bcd);
		
		if (beastType == null || beastType.length() <= 0)
		{
			blog("initializeBeastStats() bad beast type.");
			return;
		}
		
		String beastName = getBCDBeastName(bcd);
		
		beastName = beastName.trim();
		
		if (beastName.length() >= 3)
		{
			setBeastName(beast, getBCDBeastName(bcd));
		}
		else
		{
			removeObjVar(bcd, "beast.beastName");
		}
		
		dictionary beastDict = utils.dataTableGetRow(BEASTS_TABLE, beastType);
		
		if (beastDict == null || beastDict.size() <= 0)
		{
			blog("initializeBeastStats() missing entry in the "+ BEASTS_TABLE + " table for beastType: "+ beastType + ".");
			return;
		}
		
		dictionary beastStatsDict = utils.dataTableGetRow(BEASTS_STATS, level - 1);
		
		if (beastStatsDict == null || beastStatsDict.size() <= 0)
		{
			blog("initializeBeastStats() missing entry in the "+ BEASTS_STATS + " table for beastType: "+ beastType + ".");
			return;
		}
		
		int[] incubationBonuses = new int[ARRAY_BEAST_INCUBATION_STATS.length];
		
		if (hasObjVar(bcd, OBJVAR_BEAST_INCUBATION_BONUSES))
		{
			incubationBonuses = getIntArrayObjVar(bcd, OBJVAR_BEAST_INCUBATION_BONUSES);
		}
		else
		{
			for (int i = 0; i < ARRAY_BEAST_INCUBATION_STATS.length; ++i)
			{
				testAbortScript();
				incubationBonuses[i] = 0;
			}
		}
		
		for (int i = 0; i < ARRAY_BEAST_INCUBATION_STATS.length; ++i)
		{
			testAbortScript();
			utils.setScriptVar(beast, ARRAY_BEAST_INCUBATION_STATS[i], (float)incubationBonuses[i]);
		}
		
		addInnateBeastBonuses(beast, beastType);
		
		setBeastSkillBonuses(beast);
		
		setScale(beast, getBeastScaleByLevel(beast));
		
		addSkillModModifier(beast, "slope_move", "slope_move", 50, -1, false, false);
		
		updateBeastHappiness(bcd, beast);
		
		int expertiseRegen = getEnhancedSkillStatisticModifierUncapped(getMaster(beast), "expertise_bm_pet_regen");
		int expertiseHealth = getEnhancedSkillStatisticModifierUncapped(getMaster(beast), "expertise_bm_pet_health");
		int expertiseAttackSpeed = getEnhancedSkillStatisticModifierUncapped(getMaster(beast), "expertise_bm_pet_attack_speed");
		int expertiseArmor = getEnhancedSkillStatisticModifierUncapped(getMaster(beast), "expertise_bm_pet_armor");
		int expertiseDamage = getEnhancedSkillStatisticModifierUncapped(getMaster(beast), "expertise_bm_pet_damage");
		
		float incubationArmorBonus = utils.getFloatScriptVar(beast, OBJVAR_INCREASE_ARMOR);
		float incubationDamageBonus = utils.getFloatScriptVar(beast, OBJVAR_INCREASE_DPS);
		float incubationHealthBonus = utils.getFloatScriptVar(beast, OBJVAR_INCREASE_HEALTH);
		
		int intMinDamage = (int)beastStatsDict.getInt("MinDmg");
		int intMaxDamage = (int)beastStatsDict.getInt("MaxDmg");
		intMinDamage = getExpertiseStat(intMinDamage, expertiseDamage, .5f);
		intMaxDamage = getExpertiseStat(intMaxDamage, expertiseDamage, .5f);
		
		float floatMinDamage = (float)intMinDamage * (1.0f + incubationDamageBonus / 100.0f);
		float floatMaxDamage = (float)intMaxDamage * (1.0f + incubationDamageBonus / 100.0f);
		intMinDamage = (int)floatMinDamage;
		intMaxDamage = (int)floatMaxDamage;
		
		int specialDamagePercent = getExpertiseStat(100, expertiseDamage, .5f) - 100;
		
		if (!hasSkillModModifier (beast, "expertise_damage_line_beast_only"))
		{
			addSkillModModifier(beast, "expertise_damage_line_beast_only", "expertise_damage_line_beast_only", specialDamagePercent, -1, false, false);
		}
		
		float primarySpeed = getExpertiseSpeed(BEAST_WEAPON_SPEED, expertiseAttackSpeed);
		
		obj_id beastWeapon = getCurrentWeapon(beast);
		
		if (isIdValid(beastWeapon))
		{
			setWeaponAttackSpeed(beastWeapon, primarySpeed);
			setWeaponMaxDamage(beastWeapon, intMaxDamage);
			setWeaponMinDamage(beastWeapon, intMinDamage);
			weapons.setWeaponData(beastWeapon);
			
			utils.setScriptVar(beastWeapon, "isCreatureWeapon", 1);
		}
		
		obj_id defaultWeapon = getDefaultWeapon(beast);
		
		if (isIdValid(defaultWeapon))
		{
			setWeaponAttackSpeed(defaultWeapon, primarySpeed);
			setWeaponMaxDamage(defaultWeapon, intMaxDamage);
			setWeaponMinDamage(defaultWeapon, intMinDamage);
			weapons.setWeaponData(defaultWeapon);
			
			utils.setScriptVar(defaultWeapon, "isCreatureWeapon", 1);
		}
		
		int beastHealth = (int)(getExpertiseStat(beastStatsDict.getInt("HP"), expertiseHealth, .5f) * (1.0f + (incubationHealthBonus * 0.2f) / 100.0f));
		
		int constitutionBonus = getEnhancedSkillStatisticModifierUncapped(beast, "constitution_modified");
		int staminaBonus = getEnhancedSkillStatisticModifierUncapped(beast, "stamina_modified");
		
		beastHealth += (constitutionBonus * 8) + (staminaBonus * 2);
		
		setMaxAttrib(beast, HEALTH, beastHealth);
		
		beastHealth = getBCDBeastHealth(bcd);
		
		if (beastHealth <= 0)
		{
			setAttrib(beast, HEALTH, getMaxAttrib(beast, HEALTH));
		}
		else
		{
			if (utils.hasScriptVar(beast, "beast.creationInProgress"))
			{
				setAttrib(beast, HEALTH, beastHealth);
				setAttrib(beast, ACTION, 100);
			}
			
			if (getBCDBeastIsDead(bcd))
			{
				kill(beast);
			}
		}
		
		setMaxAttrib(beast, ACTION, 100);
		
		if (!combat.isInCombat(beast))
		{
			int healthRegen = getExpertiseStat(beastStatsDict.getInt("HealthRegen"), expertiseRegen, .5f);
			int actionRegen = getExpertiseStat(beastStatsDict.getInt("ActionRegen"), expertiseRegen, .5f);
			
			setRegenRate(beast, HEALTH, healthRegen);
			setRegenRate(beast, ACTION, actionRegen);
		}
		
		float runSpeed = 12;
		
		setBaseRunSpeed(beast, runSpeed);
		
		if (getBaseWalkSpeed(beast) > runSpeed)
		{
			setBaseWalkSpeed(beast, runSpeed);
		}
		
		armor.removeAllArmorData(beast);
		
		int intArmor = (int)(getExpertiseStat(beastStatsDict.getInt("Armor"), expertiseArmor, .5f) * (1.0f + incubationArmorBonus / 100.0f));
		
		intArmor += (int)getSkillStatisticModifier(beast, "expertise_innate_protection_all");
		
		utils.setScriptVar(beast, "beast.display.armor", intArmor);
		
		if (intArmor >= 0)
		{
			
			String armorCategoryObjVar = armor.OBJVAR_ARMOR_BASE + "."+ armor.OBJVAR_ARMOR_CATEGORY;
			setObjVar(beast, armorCategoryObjVar, AC_battle);
			
			String armorLevelObjVar = armor.OBJVAR_ARMOR_BASE + "."+ armor.OBJVAR_ARMOR_LEVEL;
			setObjVar(beast, armorLevelObjVar, AL_standard);
			
			String genProtectionObjVar = armor.OBJVAR_ARMOR_BASE + "."+ armor.OBJVAR_GENERAL_PROTECTION;
			setObjVar(beast, genProtectionObjVar, (float)intArmor);
			utils.setScriptVar(beast, armor.SCRIPTVAR_CACHED_GENERAL_PROTECTION, intArmor);
		}
		
		setBeastExperience(beast, getBCDBeastExperience(bcd));
		
		setBeastCanLevel(beast, getBCDBeastCanLevel(bcd));
		
		int attentionPenaltyReduction = getEnhancedSkillStatisticModifierUncapped(getMaster(beast), "expertise_bm_self_debuff_reduction");
		int attentionPenalty = BASE_ATTENTION_PENALTY + attentionPenaltyReduction;
		int hackeyWorkAroundValue = (attentionPenalty / 5) * -1;
		
		if (hackeyWorkAroundValue > 5)
		{
			hackeyWorkAroundValue = 5;
		}
		
		if (attentionPenalty < 0 || hackeyWorkAroundValue > 0)
		{
			buff.applyBuff(getMaster(beast), ATTENTION_PENALTY_DEBUFF + hackeyWorkAroundValue);
		}
		else
		{
			removeAttentionPenaltyDebuff(getMaster(beast));
		}
	}
	
	
	public static void storeBeast(obj_id bcd) throws InterruptedException
	{
		if (!isIdValid(bcd))
		{
			return;
		}
		
		obj_id datapad = getContainedBy(bcd);
		obj_id player = getContainedBy(datapad);
		obj_id currentBeast = getBCDBeastCalled(bcd);
		
		if (!isIdValid(datapad) || !isIdValid(player) || !isIdValid(currentBeast))
		{
			return;
		}
		
		storeBeastStats(bcd, currentBeast);
		
		removeBeastLinks(player, bcd, currentBeast);
		
		if (currentBeast.isLoaded() && exists(currentBeast))
		{
			if (!destroyObject(currentBeast))
			{
				return;
			}
		}
		else
		{
			messageTo(currentBeast, "handlePackRequest", null, 1, false);
		}
		
		setBCDBeastTimeStored(bcd, getGameTime());
		
		setCount(bcd, 0);
		
		setBeastmasterPet(player, null);
		
		if (!utils.hasScriptVar(player, "beast.no_store_message"))
		{
			sendSystemMessage(player, beast_lib.SID_STORED);
		}
		else
		{
			utils.removeScriptVar(player, "beast.no_store_message");
		}
		
		removeAttentionPenaltyDebuff(player);
		
		int petBuff = buff.getBuffOnTargetFromGroup(player, "bm_player_buff");
		
		if (petBuff != 0)
		{
			buff.removeBuff(player, petBuff);
		}
	}
	
	
	public static void storeBeastStats(obj_id bcd, obj_id beast) throws InterruptedException
	{
		int health = getAttrib(beast, HEALTH);
		
		setBCDBeastHealth(bcd, health);
		
		boolean beastIsDead = isDead(beast);
		
		setBCDBeastIsDead(bcd, beastIsDead);
	}
	
	
	public static void storeBeasts(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return;
		}
		
		obj_id beast = getBeastOnPlayer(player);
		
		if (!isIdValid(beast))
		{
			return;
		}
		
		obj_id bcd = getBeastBCD(beast);
		
		if (!isIdValid(bcd))
		{
			destroyObject(beast);
			setBeastmasterPet(player, null);
			return;
		}
		
		storeBeast(bcd);
	}
	
	
	public static void killBeast(obj_id beast, obj_id killer) throws InterruptedException
	{
		if (!isValidBeast(beast))
		{
			return;
		}
		
		obj_id bcd = getBeastBCD(beast);
		
		if (!isValidBCD(bcd))
		{
			destroyObject(beast);
		}
		
		utils.setScriptVar(beast, "recapTimer", getGameTime());
		
		if (isIdValid(killer) && exists(killer))
		{
			if (pet_lib.isPet(killer) || beast_lib.isBeast(killer))
			{
				killer = getMaster(killer);
			}
			
			utils.setScriptVar(beast, "killer", killer);
		}
		
		blog("killBeast beast: "+ beast + " killer "+ killer);
		
		xp.cleanupCreditForKills();
		
		dictionary dictIncap = new dictionary();
		int incapTimer = getGameTime();
		
		utils.setScriptVar(beast, "incapTimer", incapTimer);
		dictIncap.put("incapTimer", incapTimer);
		
		messageTo(beast, "handleBeastIncappedDecay", dictIncap, 120, false);
		
		deltadictionary dict = beast.getScriptVars();
		dict.put("pet.regenMultiplier", 0);
		
		if (isIdValid(killer) && !isPlayer(killer))
		{
			obj_id master = getMaster(killer);
			
			if (!isIdValid(master) || !isPlayer(master))
			{
				decrementBeastLoyalty(bcd, LOYALTY_DEATH_LOSS);
			}
		}
		
		stop(beast);
		kill(beast);
		
		setBCDBeastHealth(bcd, 0);
		setBCDBeastIsDead(bcd, true);
	}
	
	
	public static void incrementBeastExperience(obj_id beast) throws InterruptedException
	{
		if (!isValidBeast(beast))
		{
			return;
		}
		
		int experience = getBeastExperience(beast);
		int previousExperience = experience;
		obj_id bcd = getBeastBCD(beast);
		
		if (!isValidBCD(bcd))
		{
			return;
		}
		
		float happinessAdjustment = ((float)getBCDBeastHappiness(bcd) / 100.0f) + 1.0f;
		
		if (happinessAdjustment > 1.5f)
		{
			happinessAdjustment = 1.5f;
		}
		else if (happinessAdjustment < 0.5f)
		{
			happinessAdjustment = 0.5f;
		}
		
		float percentageBonuses = happinessAdjustment;
		
		if (buff.hasBuff(beast, "bm_beast_steroid"))
		{
			percentageBonuses += 0.01f * utils.getIntScriptVar(beast, "beastBuff.beastXpBonusPercent");
		}
		
		obj_id master = getMaster(beast);
		
		if (isValidPlayer(master))
		{
			int experienceGain = Math.round(100 * percentageBonuses);
			
			experienceGain += experienceGain * 0.01f * getEnhancedSkillStatisticModifierUncapped(master, "bm_xp_mod_boost");
			
			experience += experienceGain;
			
			prose_package pp = prose.getPackage(xp.SID_FLYTEXT_XP, experience - previousExperience);
			
			showFlyTextPrivate(beast, master, pp, 2.5f, new color(180, 60, 240, 255));
		}
		
		setBeastExperience(beast, experience);
		setBCDBeastExperience(bcd, experience);
	}
	
	
	public static void incrementBeastLevel(obj_id beast) throws InterruptedException
	{
		if (!isValidBeast(beast))
		{
			return;
		}
		
		int level = getLevel(beast) + 1;
		
		if (level == 75 && buff.hasBuff(beast, "bm_beast_steroid"))
		{
			buff.removeBuff(beast, "bm_beast_steroid");
			obj_id master = getMaster(beast);
			sendSystemMessage(master, SID_BEAST_LEVEL_TOO_HIGH);
		}
		
		if (level > 90)
		{
			return;
		}
		
		obj_id bcd = getBeastBCD(beast);
		
		if (!isValidBCD(bcd))
		{
			return;
		}
		
		setBeastLevel(beast, level);
		setBCDBeastLevel(bcd, level);
		
		playClientEffectObj(beast, "clienteffect/level_granted.cef", beast, null);
		
		initializeBeastStats(bcd, beast);
	}
	
	
	public static boolean canBeastLevelUp(obj_id beast) throws InterruptedException
	{
		if (!isValidBeast(beast))
		{
			return false;
		}
		
		int level = getBeastLevel(beast);
		
		obj_id master = getMaster(beast);
		
		if (!isValidPlayer(master) || level >= getLevel(master))
		{
			return false;
		}
		
		dictionary beastStatsDict = utils.dataTableGetRow(BEASTS_STATS, level - 1);
		
		if (beastStatsDict == null || beastStatsDict.size() <= 0)
		{
			blog("initializeBeastStats() missing entry in the "+ BEASTS_STATS + " table for level: "+ level + ".");
			return false;
		}
		
		int experience = (int)beastStatsDict.getInt("XP");
		
		if (experience > 0 && getBeastExperience(beast) >= experience)
		{
			return true;
		}
		
		return false;
	}
	
	
	public static void grantBeastExperience(obj_id beast) throws InterruptedException
	{
		if (!isValidBeast(beast) || isDead(beast))
		{
			return;
		}
		
		if (!getBeastCanLevel(beast))
		{
			return;
		}
		
		obj_id bcd = getBeastBCD(beast);
		
		incrementBeastLoyalty(bcd);
		
		int level = getBeastLevel(beast);
		
		if (level < 90)
		{
			incrementBeastExperience(beast);
		}
		
		if (canBeastLevelUp(beast))
		{
			incrementBeastLevel(beast);
		}
	}
	
	
	public static float scaleDistanceByLevel(obj_id beast, float distance) throws InterruptedException
	{
		int level = getLevel(beast);
		
		distance += (level / 45);
		
		return distance;
	}
	
	
	public static void validateFollowTarget(obj_id beast, obj_id target) throws InterruptedException
	{
		if (!isIdValid(beast) || !isIdValid(target))
		{
			return;
		}
		
		obj_id master = getMaster(beast);
		
		if (!isValidPlayer(master))
		{
			return;
		}
		
		dictionary params = new dictionary();
		
		if (utils.getBooleanScriptVar(beast, "ai.pet.staying"))
		{
			return;
		}
		
		if (target != master && (stealth.hasServerCoverState(target) || stealth.hasInvisibleBuff(target)))
		{
			doStayCommand(beast, master);
			doConfusedEmote(beast);
			return;
		}
		
		if (target != master && !ai_lib.isInCombat(beast))
		{
			doFollowCommand(beast, master);
			return;
		}
		
		if (ai_lib.isInCombat(beast) && getBeastDefensive(beast))
		{
			if (getLocomotion(beast) != LOCOMOTION_RUNNING)
			{;
			}
			{
				setMovementRun(beast);
			}
			
			return;
		}
		
		if (ai_lib.isInCombat(beast) && !getBeastDefensive(beast) && !utils.hasScriptVar(beast, "petIgnoreAttacks"))
		{
			if (getLocomotion(beast) != LOCOMOTION_RUNNING)
			{;
			}
			{
				setMovementRun(beast);
			}
			
			return;
		}
		
		int currentTime = getGameTime();
		int lastFollowTest = utils.getIntScriptVar(beast, "beast.lastFollowTest");
		
		if (currentTime <= lastFollowTest)
		{
			return;
		}
		
		utils.setScriptVar(beast, "beast.lastFollowTest", currentTime);
		
		if (target == master)
		{
			
			params.put("target", master);
			
			if (getDistance(beast, master) >= 1.5f && getDistance(beast, master) < 5.0f)
			{
				if (getLocomotion(beast) == LOCOMOTION_RUNNING)
				{
					setMovementWalk(beast);
				}
			}
			
			else if (getDistance(beast, master) < 1.5f)
			{
				if (getLocomotion(beast) == LOCOMOTION_RUNNING)
				{
					setMovementWalk(beast);
				}
				
				follow(beast, target, scaleDistanceByLevel(beast, 0.1f), scaleDistanceByLevel(beast, 0.5f));
			}
			else
			{
				location offset = new location();
				
				offset.x = -1.5f;
				offset.z = -1;
				
				if (getLocomotion(beast) != LOCOMOTION_RUNNING && getDistance(beast, master) >= 5.0f)
				{;
				}
				{
					setMovementRun(beast);
				}
				
				if (getDistance(beast, master) >= 5.0f && !canSee(beast, master))
				{
					setLocation(beast, getLocation(master));
				}
				
				follow(beast, target, offset);
			}
			
			messageTo(beast, "checkMovementSpeed", params, 1, false);
			
			return;
		}
		
		return;
	}
	
	
	public static void doConfusedEmote(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast))
		{
			return;
		}
		
		showFlyText(beast, new string_id("npc_reaction/flytext", "alert"), 3.0f, colors.COPPER);
		doAnimationAction(beast, "emt_stand_confused");
	}
	
	
	public static void doStayCommand(obj_id beast, obj_id master) throws InterruptedException
	{
		if (!isIdValid(beast) || !isIdValid(master))
		{
			return;
		}
		
		setMovementWalk(beast);
		
		if (ai_lib.isInCombat(beast))
		{
			utils.removeScriptVar(beast, "ai.combat.target");
			utils.setScriptVar(beast, "petIgnoreAttacks", getGameTime());
			stopCombat(beast);
		}
		
		utils.setScriptVar(beast, "ai.pet.staying", true);
		ai_lib.aiStopFollowing(beast);
		
		location myLocation = getLocation(beast);
		
		setHomeLocation(beast, myLocation);
		
		setMovementWalk(beast);
		
		doAnimationAction(beast, "vocalize");
		
		ai_lib.setDefaultCalmBehavior(beast, ai_lib.BEHAVIOR_STOP);
	}
	
	
	public static void doFollowCommand(obj_id beast, obj_id master) throws InterruptedException
	{
		if (!isIdValid(beast) || !isIdValid(master))
		{
			return;
		}
		
		if (ai_lib.isInCombat(beast))
		{
			utils.removeScriptVar(beast, "ai.combat.target");
			utils.setScriptVar(beast, "petIgnoreAttacks", getGameTime());
			stopCombat(beast);
		}
		
		utils.removeScriptVar(beast, "ai.pet.staying");
		
		if (hasObjVar(beast, "ai.wounded"))
		{
			removeObjVar(beast, "ai.wounded");
		}
		
		int myPosture = getPosture(beast);
		
		if (myPosture != POSTURE_UPRIGHT && myPosture != POSTURE_SITTING)
		{
			stop(beast);
			removeObjVar(beast, "ai.combat.moveMode");
			queueCommand(beast, (-1465754503), beast, "", COMMAND_PRIORITY_FRONT);
		}
		
		beastFollowTarget(beast, master);
	}
	
	
	public static void beastFollowTarget(obj_id beast, obj_id target) throws InterruptedException
	{
		
		follow(beast, target, scaleDistanceByLevel(beast, 0.1f), scaleDistanceByLevel(beast, 0.2f));
		setMovementRun(beast);
	}
	
	
	public static void doAttackCommand(obj_id beast, obj_id master) throws InterruptedException
	{
		if (!isIdValid(master))
		{
			return;
		}
		
		utils.removeScriptVar(beast, "petIgnoreAttacks");
		
		obj_id target = getIntendedTarget(master);
		
		if (isIdValid(target) == false || target == beast)
		{
			doConfusedEmote(beast);
			return;
		}
		
		if (!pvpCanAttack(beast, target))
		{
			doConfusedEmote(beast);
			return;
		}
		
		if (!pvpCanAttack(master, target))
		{
			doConfusedEmote(beast);
			return;
		}
		
		if (master.isLoaded())
		{
			setHomeLocation(beast, getLocation(master));
		}
		else if (beast.isLoaded())
		{
			setHomeLocation(beast, getLocation(beast));
		}
		
		utils.removeScriptVar(beast, "ai.pet.staying");
		
		chat.setBadMood(beast);
		
		if (!ai_lib.isInCombat(beast))
		{
			startCombat(beast, target);
			
			setHate(beast, target, 0);
			
			utils.setScriptVar(beast, "ai.combat.target", target);
		}
		else
		{
			utils.setScriptVar(beast, "ai.combat.target", target);
			
			float maxHate = getMaxHate(beast);
			
			setHate(beast, target, maxHate + 5000);
		}
	}
	
	
	public static void doTrickCommand(obj_id beast, int trickNumber) throws InterruptedException
	{
		int myPosture = getPosture(beast);
		obj_id beastBCD = getBeastBCD (beast);
		
		obj_id master = getMaster(beast);
		int state = getState(beast, STATE_SWIMMING);
		
		if (state == 1)
		{
			sendSystemMessage(master, SID_NO_TRICKS_WHILE_SWIMMING);
			return;
		}
		
		if (isIdValid (beastBCD))
		{
			messageTo(beastBCD, "petDoingTrick", null, 1, false);
		}
		
		if (myPosture == POSTURE_SITTING)
		{
			doAnimationAction(beast, "sit_trick_"+ trickNumber);
		}
		else
		{
			doAnimationAction(beast, "trick_"+ + trickNumber);
		}
		
		return;
	}
	
	
	public static void setupHappinessLoyalty(obj_id beastBCD) throws InterruptedException
	{
		if (!isIdValid(beastBCD))
		{
			return;
		}
		
		if (!hasBCDBeastHappiness(beastBCD))
		{
			setBCDBeastHappiness(beastBCD, 0);
			
			obj_id beast = getBCDBeastCalled(beastBCD);
			
			updateBeastStats(beastBCD, beast);
		}
		
		if (!hasBCDBeastLoyalty(beastBCD))
		{
			setBCDBeastLoyalty(beastBCD, 1.0f);
		}
		
		if (!hasBCDBeastLoyaltyLevel(beastBCD))
		{
			setBCDBeastLoyaltyLevel(beastBCD, 1);
		}
		
		if (hasObjVar(beastBCD, PET_FAVORITES_OBJVAR))
		{
			return;
		}
		
		int[] beastHappiness = new int[LIKE_DISLIKE_MAX];
		
		String[] favoriteFood = dataTableGetStringColumnNoDefaults(DATATABLE_BEAST_FAVORITES, DATATABLE_FOOD_COL);
		String[] favoriteLocation = dataTableGetStringColumnNoDefaults(DATATABLE_BEAST_FAVORITES, DATATABLE_LOCATION_COL);
		String[] favoriteActivity = dataTableGetStringColumnNoDefaults(DATATABLE_BEAST_FAVORITES, DATATABLE_ACTIVITY_COL);
		
		int foodNumberGood = rand(1, favoriteFood.length);
		beastHappiness[0] = foodNumberGood;
		
		boolean goodNumberFood = false;
		
		while (!goodNumberFood)
		{
			testAbortScript();
			int foodNumberBad = rand(1, favoriteFood.length);
			
			if (beastHappiness[0] != foodNumberBad)
			{
				beastHappiness[1] = foodNumberBad;
				goodNumberFood = true;
			}
		}
		
		int activityNumberGood = rand(1, favoriteActivity.length);
		beastHappiness[2] = activityNumberGood;
		
		boolean goodNumberAct = false;
		
		while (!goodNumberAct)
		{
			testAbortScript();
			int activityNumberBad = rand(1, favoriteActivity.length);
			
			obj_id player = getBCDPlayer(beastBCD);
			
			if (hasObjVar(player, "qa_beast_hate"))
			{
				if (activityNumberGood == 1)
				{
					beastHappiness[2] = 2;
				}
				
				activityNumberBad = 1;
			}
			
			if (beastHappiness[2] != activityNumberBad)
			{
				beastHappiness[3] = activityNumberBad;
				goodNumberAct = true;
			}
		}
		
		int locationNumberGood = rand(1, favoriteLocation.length);
		beastHappiness[4] = locationNumberGood;
		
		boolean goodNumberLoc = false;
		
		while (!goodNumberLoc)
		{
			testAbortScript();
			int locationNumberBad = rand(1, favoriteLocation.length);
			
			if (beastHappiness[4] != locationNumberBad)
			{
				beastHappiness[5] = locationNumberBad;
				goodNumberLoc = true;
			}
		}
		
		setObjVar(beastBCD, PET_FAVORITES_OBJVAR, beastHappiness);
	}
	
	
	public static void applyFoodEffect(obj_id food, obj_id beast, obj_id player) throws InterruptedException
	{
		obj_id beastBCD = getBeastBCD(beast);
		String foodTemplate = utils.getTemplateFilenameNoPath(food);
		int currentTime = getGameTime();
		int[] petFood = getBCDBeastFood(beastBCD);
		
		if (!hasObjVar(beastBCD, PET_FAVORITES_OBJVAR))
		{
			LOG("beast", "Somehow this beast was created without having his favorites decided. Setting them now to avoid exceptions");
			setupHappinessLoyalty(beastBCD);
		}
		
		int[] beastHappiness = getIntArrayObjVar(beastBCD, PET_FAVORITES_OBJVAR);
		
		int oldFood = petFood[PET_WHICH_FOOD];
		
		dictionary params = new dictionary();
		params.put("beastId", beast);
		int foodType = PET_NORMAL_FOOD;
		
		String datatableFavorite = dataTableGetString(DATATABLE_BEAST_FAVORITES, (beastHappiness[0] - 1), DATATABLE_FOOD_COL);
		String datatableDislike = dataTableGetString(DATATABLE_BEAST_FAVORITES, (beastHappiness[1] - 1), DATATABLE_FOOD_COL);
		
		if (datatableFavorite.equals(foodTemplate))
		{
			foodType = PET_FAVORITE_FOOD;
			petFood[PET_WHICH_FOOD] = PET_FAVORITE_FOOD;
			petFood[PET_TIME_SINCE_FEEDING] = currentTime;
			sendSystemMessage(player, SID_FAVORITE_FOOD);
		}
		
		else if (datatableDislike.equals(foodTemplate))
		{
			foodType = PET_DISLIKE_FOOD;
			petFood[PET_WHICH_FOOD] = PET_DISLIKE_FOOD;
			petFood[PET_TIME_SINCE_FEEDING] = currentTime;
			sendSystemMessage(player, SID_DISLIKE_FOOD);
		}
		
		else
		{
			foodType = PET_NORMAL_FOOD;
			petFood[PET_WHICH_FOOD] = PET_NORMAL_FOOD;
			petFood[PET_TIME_SINCE_FEEDING] = currentTime;
			sendSystemMessage(player, SID_NORMAL_FOOD);
		}
		
		setBCDBeastFood(beastBCD, petFood);
		
		updateBeastHappiness(beastBCD, beast);
		
		updateBeastStats(beastBCD, beast);
		
		ai_lib.doAction(beast, "eat");
		
		messageTo(beastBCD, "beastHungry", params, 3600, false);
	}
	
	
	public static void checkForFavoriteLocation(obj_id beastBCD) throws InterruptedException
	{
		if (!isValidBCD(beastBCD))
		{
			return;
		}
		
		if (!hasObjVar(beastBCD, PET_FAVORITES_OBJVAR))
		{
			LOG("beast", "Somehow this beast was created without having his favorites decided. Setting them now to avoid exceptions");
			setupHappinessLoyalty(beastBCD);
		}
		
		obj_id player = getBCDPlayer(beastBCD);
		obj_id beast = getBCDBeastCalled (beastBCD);
		
		int[] beastHappiness = getIntArrayObjVar(beastBCD, PET_FAVORITES_OBJVAR);
		String favoriteLocation = dataTableGetString(DATATABLE_BEAST_FAVORITES, (beastHappiness[4] -1), DATATABLE_LOCATION_COL);
		String dislikeLocation = dataTableGetString(DATATABLE_BEAST_FAVORITES, (beastHappiness[5] -1), DATATABLE_LOCATION_COL);
		String currentLocation = getCurrentSceneName();
		int locationType = PET_NORMAL_LOCATION;
		
		if (isDead(beast))
		{
			return;
		}
		
		if (currentLocation.indexOf(favoriteLocation) > -1 || favoriteLocation.equals(currentLocation))
		{
			locationType = PET_FAVORITE_LOCATION;
			sendSystemMessage(player, SID_FAVORITE_LOCATION);
		}
		
		else if (instance.isInInstanceArea(player))
		{
			String alternatesColumn = favoriteLocation + "Alternates";
			
			if (dataTableHasColumn(DATATABLE_BEAST_FAVORITES, alternatesColumn))
			{
				String areaName = locations.getBuildoutAreaName(player);
				
				if (areaName != null && areaName.length() > 0)
				{
					String[] alternateLocs = dataTableGetStringColumn(DATATABLE_BEAST_FAVORITES, alternatesColumn);
					
					if (alternateLocs != null && alternateLocs.length > 0)
					{
						for (int i = 0; i < alternateLocs.length; i++)
						{
							testAbortScript();
							String alternateLocation = alternateLocs[i];
							
							if (alternateLocation.equals(areaName))
							{
								locationType = PET_FAVORITE_LOCATION;
								sendSystemMessage(player, SID_FAVORITE_LOCATION);
							}
						}
					}
				}
			}
		}
		
		else if (currentLocation.indexOf (dislikeLocation) > -1 || dislikeLocation.equals(currentLocation))
		{
			locationType = PET_DISLIKE_LOCATION;
			sendSystemMessage(player, SID_DISLIKE_LOCATION);
		}
		
		else
		{
			locationType = PET_NORMAL_LOCATION;
		}
		
		if (!utils.hasScriptVar(beastBCD, PET_LOCATION_SCRIPTVAR))
		{
			utils.setScriptVar(beastBCD, PET_LOCATION_SCRIPTVAR, locationType);
		}
		
		int currentTime = getGameTime();
		
		if (!utils.hasScriptVar(beastBCD, PET_HAPPINESS_BLOCK_SCRIPTVAR))
		{
			utils.setScriptVar(beastBCD, PET_HAPPINESS_BLOCK_SCRIPTVAR, currentTime);
		}
		
		int timeStamp = utils.getIntScriptVar(beastBCD, PET_HAPPINESS_BLOCK_SCRIPTVAR);
		
		if ((currentTime - timeStamp) > 5)
		{
			int oldLocation = utils.getIntScriptVar(beastBCD, PET_LOCATION_SCRIPTVAR);
			
			utils.setScriptVar(beastBCD, PET_HAPPINESS_BLOCK_SCRIPTVAR, currentTime);
			utils.setScriptVar(beastBCD, PET_LOCATION_SCRIPTVAR, locationType);
			
			updateBeastStats(beastBCD, beast);
		}
		
		updateBeastHappiness(beastBCD, beast);
	}
	
	
	public static int getSingleSkillMaxTrainingPoints(obj_id pet) throws InterruptedException
	{
		int loyaltyLevel = getBeastLoyaltyLevel(pet);
		int level = 0;
		
		if (isBeast(pet))
		{
			level = getBeastLevel(pet);
		}
		else
		{
			level = getBCDBeastLevel(pet);
		}
		
		return loyaltyLevel * level;
	}
	
	
	public static int getSingleSkillMaxTrainingPoints(obj_id bcd, int level) throws InterruptedException
	{
		int loyaltyLevel = getBCDBeastLoyaltyLevel(bcd);
		
		return loyaltyLevel * level;
	}
	
	
	public static int getMaxTrainingPoints(obj_id pet) throws InterruptedException
	{
		return getSingleSkillMaxTrainingPoints(pet) * 4;
	}
	
	
	public static int getAvailableTrainingPoints(obj_id pet) throws InterruptedException
	{
		return getAvailableTrainingPoints(pet, null);
	}
	
	
	public static int getAvailableTrainingPoints(obj_id pet, String passedAbility) throws InterruptedException
	{
		int spent = calculateTrainingPointsSpent(pet, passedAbility);
		int maximum = getMaxTrainingPoints(pet);
		
		return maximum - spent;
	}
	
	
	public static boolean hasSufficientTrainingPoints(obj_id pet, String ability) throws InterruptedException
	{
		int available = getAvailableTrainingPoints(pet, ability);
		return (getAbilityCost(ability) <= available);
	}
	
	
	public static boolean isValidForSingleSkillMaxCost(obj_id pet, String ability) throws InterruptedException
	{
		int singleSkillMax = getSingleSkillMaxTrainingPoints(pet);
		return getAbilityCost(ability) <=singleSkillMax;
	}
	
	
	public static int getAbilityCost(String ability) throws InterruptedException
	{
		return dataTableGetInt(BEASTS_SPECIALS, ability, "training_cost");
	}
	
	
	public static String[] getTrainedSkills(obj_id bcd) throws InterruptedException
	{
		if (!isIdValid(bcd) || !exists(bcd))
		{
			return null;
		}
		
		if (isBeast(bcd))
		{
			return getTrainedSkills(getBeastBCD(bcd));
		}
		
		String[] trainedSkills =
		{
			"empty","empty","empty","empty"
		};
		
		if (!hasObjVar(bcd, PET_TRAINED_SKILLS_LIST))
		{
			return trainedSkills;
		}
		else
		{
			trainedSkills = getStringArrayObjVar(bcd, PET_TRAINED_SKILLS_LIST);
		}
		
		trainedSkills = correctNullSkills(trainedSkills);
		
		return trainedSkills;
	}
	
	
	public static String[] getAutoRepeatAbilityList(obj_id bcd) throws InterruptedException
	{
		if (isBeast(bcd))
		{
			return getAutoRepeatAbilityList(getBeastBCD(bcd));
		}
		
		String[] barData = getBeastmasterPetBarData(getBCDBeastCalled(bcd));
		
		String[] abilityList =
		{
			barData[3], barData[4], barData[5], barData[6]
		};
		
		String[] repeatList =
		{
			"","","",""
		};
		
		if (hasObjVar(bcd, PET_AUTO_REPEAT_LIST))
		{
			repeatList = getStringArrayObjVar(bcd, PET_AUTO_REPEAT_LIST);
		}
		
		for (int i=0; i<repeatList.length; i++)
		{
			testAbortScript();
			boolean contains = false;
			for (int k=0; k<abilityList.length; k++)
			{
				testAbortScript();
				if (abilityList[k].equals(repeatList[i]))
				{
					contains = true;
				}
			}
			
			if (!contains)
			{
				repeatList[i].equals("");
			}
		}
		
		return repeatList;
	}
	
	
	public static void clearAutoRepeatAbiltiyList(obj_id bcd) throws InterruptedException
	{
		if (isBeast(bcd))
		{
			bcd = getBeastBCD(bcd);
		}
		
		String[] repeatList =
		{
			"", "", "", ""
		};
		
		setObjVar(bcd, PET_AUTO_REPEAT_LIST, repeatList);
	}
	
	
	public static boolean isAbilityAutoRepeat(obj_id bcd, String abilityName) throws InterruptedException
	{
		if (isBeast(bcd))
		{
			return isAbilityAutoRepeat(getBeastBCD(bcd), abilityName);
		}
		
		String[] repeatList = getAutoRepeatAbilityList(bcd);
		
		for (int i = 0; i < repeatList.length; i++)
		{
			testAbortScript();
			if (!repeatList[i].equals("") && repeatList[i].equals(abilityName))
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public static String[] setAbilityAutoRepeat(obj_id bcd, String abilityName) throws InterruptedException
	{
		if (isBeast(bcd))
		{
			return setAbilityAutoRepeat(getBeastBCD(bcd), abilityName);
		}
		
		String[] abilityList = getTrainedSkills(bcd);
		String[] repeatList = getAutoRepeatAbilityList(bcd);
		int row = dataTableSearchColumnForString(abilityName, "ability_name", BEASTS_SPECIALS);
		
		if (row < 0)
		{
			return null;
		}
		
		dictionary dict = dataTableGetRow(BEASTS_SPECIALS, row);
		
		if (dict == null || dict.isEmpty())
		{
			return null;
		}
		
		int specialType = dict.getInt("specialType");
		for (int i = 0; i < abilityList.length; i++)
		{
			testAbortScript();
			if (abilityName.equals(abilityList[i]))
			{
				if (specialType != 1)
				{
					repeatList[i] = abilityName;
				}
			}
		}
		
		setObjVar(bcd, PET_AUTO_REPEAT_LIST, repeatList);
		
		return repeatList;
	}
	
	
	public static String[] unsetAbilityAutoRepeat(obj_id bcd, String abilityName) throws InterruptedException
	{
		if (isBeast(bcd))
		{
			return unsetAbilityAutoRepeat(getBeastBCD(bcd), abilityName);
		}
		
		String[] repeatList = getAutoRepeatAbilityList(bcd);
		
		for (int i = 0; i < repeatList.length; i++)
		{
			testAbortScript();
			if (repeatList[i].equals(abilityName))
			{
				repeatList[i] = "";
			}
		}
		
		setObjVar(bcd, PET_AUTO_REPEAT_LIST, repeatList);
		
		return repeatList;
	}
	
	
	public static String getBestAutoRepeatAbility(obj_id beast) throws InterruptedException
	{
		String[] abilityList = getAutoRepeatAbilityList(beast);
		
		String bestAction = "";
		float highestCost = 0.0f;
		
		for (int i = 0; i < abilityList.length; i++)
		{
			testAbortScript();
			if (abilityList[i].equals(""))
			{
				continue;
			}
			
			combat_data abilityData = combat_engine.getCombatData(abilityList[i]);
			
			if (abilityData == null)
			{
				continue;
			}
			
			String cooldownGroup = abilityData.cooldownGroup;
			int groupCrc = getStringCrc(cooldownGroup);
			float coolDownLeft = getCooldownTimeLeft(getMaster(beast), groupCrc);
			
			if (coolDownLeft > 0.0f)
			{
				continue;
			}
			
			if (combat.canDrainCombatActionAttributes(beast, (int)abilityData.vigorCost) && abilityData.vigorCost > highestCost)
			{
				highestCost = abilityData.vigorCost;
				bestAction = abilityList[i];
			}
			
		}
		
		return bestAction;
	}
	
	
	public static void setTrainedPetSkills(obj_id pet, String[] abilityList) throws InterruptedException
	{
		obj_id bcd = getBeastBCD(pet);
		abilityList = correctNullSkills(abilityList);
		setObjVar(bcd, PET_TRAINED_SKILLS_LIST, abilityList);
		obj_id player = getMaster(pet);
		setBeastmasterToggledPetCommands(player, abilityList);
		updatePetAbilityList(bcd);
	}
	
	
	public static String[] correctNullSkills(String[] abilityList) throws InterruptedException
	{
		for (int i=0; i<abilityList.length; i++)
		{
			testAbortScript();
			if (abilityList[i].equals(""))
			{
				abilityList[i] = "empty";
			}
		}
		
		return abilityList;
	}
	
	
	public static void clearTrainedSkills(obj_id pet) throws InterruptedException
	{
		obj_id bcd = getBeastBCD(pet);
		String[] emptyList =
		{
			"empty","empty","empty","empty"
		};
		String[] nullList =
		{
			"","","",""
		};
		setObjVar(bcd, PET_TRAINED_SKILLS_LIST, emptyList);
		setObjVar(bcd, PET_AUTO_REPEAT_LIST, nullList);
		updatePetAbilityList(bcd);
	}
	
	
	public static boolean canPerformCommand(obj_id player, obj_id pet, String command) throws InterruptedException
	{
		int additionalAbilitySlot = getSkillStatisticModifier(player, "expertise_bm_add_pet_bar");
		
		String[] abilityList = getTrainedSkills(pet);
		for (int i=0; i<abilityList.length; i++)
		{
			testAbortScript();
			if (abilityList.equals(""))
			{
				continue;
			}
			
			if (abilityList[i].equals(command) && i <= additionalAbilitySlot)
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public static int getAvailableTrainingSlots(obj_id pet, String abilityName) throws InterruptedException
	{
		String[] knownSkills = getTrainedSkills(pet);
		int additionalAbilitySlot = getSkillStatisticModifier(getBCDPlayer(getBeastBCD(pet)), "expertise_bm_add_pet_bar");
		
		int openSlots = 0;
		
		if (!hasCommand(getMaster(pet), BM_COMMAND_ATTACK) && getBeastMasterSpecialType(abilityName) != ABILITY_TYPE_NONCOMBAT)
		{
			return 0;
		}
		
		for (int i=0; i < knownSkills.length; i++)
		{
			testAbortScript();
			if (i <= additionalAbilitySlot)
			{
				if (knownSkills[i].equals("empty") || isSkillUpgrade(pet, abilityName))
				{
					openSlots++;
				}
			}
		}
		
		return openSlots;
	}
	
	
	public static boolean isValidBeastSkillForCreature(obj_id pet, String skill) throws InterruptedException
	{
		obj_id bcd = getBeastBCD(pet);
		String beastName = getBeastType(bcd);
		
		dictionary beastDict = utils.dataTableGetRow(BEASTS_TABLE, beastName);
		
		String specialAttackFamily = beastDict.getString("special_attack_family");
		
		int isValidForType = dataTableGetInt(BEASTS_SPECIALS, skill, specialAttackFamily);
		
		return isValidForType == 1;
	}
	
	
	public static int calculateTrainingPointsSpent(obj_id pet) throws InterruptedException
	{
		return calculateTrainingPointsSpent(pet, null);
	}
	
	
	public static int calculateTrainingPointsSpent(obj_id pet, String passedAbility) throws InterruptedException
	{
		
		boolean compareAbility = passedAbility != null;
		
		String[] attackList = getTrainedSkills(pet);
		int trainedTotal = 0;
		
		for (int i = 0; i < attackList.length; i++)
		{
			testAbortScript();
			if (attackList.equals("empty"))
			{
				continue;
			}
			
			if (compareAbility && getSkillBeastmasterSkillComparison(passedAbility, attackList[i]) == SKILL_HIGHER)
			{
				int costOldAbility = getAbilityCost(attackList[i]);
				
				trainedTotal -= costOldAbility;
				continue;
			}
			
			int cost = getAbilityCost(attackList[i]);
			
			if (cost != -1)
			{
				trainedTotal += cost;
			}
			
		}
		
		return trainedTotal;
	}
	
	
	public static boolean isSkillAlreadyTrained(obj_id pet, String ability) throws InterruptedException
	{
		String[] knownSkills = getTrainedSkills(pet);
		
		for (int i = 0; i < knownSkills.length; i++)
		{
			testAbortScript();
			if (knownSkills[i].equals(ability) || getSkillBeastmasterSkillComparison(ability, knownSkills[i]) == SKILL_LOWER)
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public static boolean isSkillUpgrade(obj_id pet, String ability) throws InterruptedException
	{
		String[] knownSkills = getTrainedSkills(pet);
		
		if (knownSkills == null || knownSkills.length == 0)
		{
			return true;
		}
		
		for (int i=0; i<knownSkills.length; i++)
		{
			testAbortScript();
			if (getSkillBeastmasterSkillComparison(ability, knownSkills[i]) == SKILL_HIGHER)
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public static boolean canLearnSkill(obj_id pet, String abilityName) throws InterruptedException
	{
		int returnCode = getTrainingSuccessCode(pet, abilityName);
		boolean canLearn = true;
		
		switch (returnCode)
		{
			case TRAINING_INVALID_PETTYPE:
			sendSystemMessage(getMaster(pet), new string_id("beast", "train_fail_spam_bad_pet_type"));
			canLearn = false;
			break;
			
			case TRAINING_SKILL_TOO_HIGH:
			sendSystemMessage(getMaster(pet), new string_id("beast", "trail_fail_spam_single_skill_max"));
			canLearn = false;
			break;
			
			case TRAINING_INSUFFICIENT_POINTS:
			sendSystemMessage(getMaster(pet), new string_id("beast", "train_fail_spam_insufficient_points"));
			canLearn = false;
			break;
			
			case TRAINING_NON_IMPROVED_SKILL:
			sendSystemMessage(getMaster(pet), new string_id("beast", "train_fail_spam_already_known"));
			canLearn = false;
			break;
			
			case TRAINING_NO_AVAILABLE_SLOTS:
			sendSystemMessage(getMaster(pet), new string_id("beast", "train_fail_spam_no_slots"));
			canLearn = false;
			break;
		}
		
		return canLearn;
	}
	
	
	public static int getTrainingSuccessCode(obj_id pet, String abilityName) throws InterruptedException
	{
		if (!isValidBeastSkillForCreature(pet, abilityName))
		{
			return TRAINING_INVALID_PETTYPE;
		}
		
		if (isSkillAlreadyTrained(pet, abilityName))
		{
			return TRAINING_NON_IMPROVED_SKILL;
		}
		
		if (!isValidForSingleSkillMaxCost(pet, abilityName))
		{
			return TRAINING_SKILL_TOO_HIGH;
		}
		
		if (!hasSufficientTrainingPoints(pet, abilityName))
		{
			return TRAINING_INSUFFICIENT_POINTS;
		}
		
		if (getAvailableTrainingSlots(pet, abilityName) < 1)
		{
			return TRAINING_NO_AVAILABLE_SLOTS;
		}
		
		return TRAINING_VALID;
	}
	
	
	public static void updatePetAbilityList(obj_id bcd) throws InterruptedException
	{
		obj_id pet = getBCDBeastCalled(bcd);
		
		if (!isIdValid(pet) || !exists(pet))
		{
			return;
		}
		
		updatePetAbilityList(bcd, pet);
	}
	
	
	public static void updatePetAbilityList(obj_id bcd, obj_id pet) throws InterruptedException
	{
		String[] petAbilities = getTrainedSkills(bcd);
		setObjVar(pet, PET_TRAINED_SKILLS_LIST, petAbilities);
		setBeastmasterPetCommands(getBCDPlayer(bcd), getBeastmasterPetBarData(getBCDPlayer(bcd), pet));
		setToggledCommands(getBCDPlayer(bcd), bcd);
	}
	
	
	public static boolean canTrainPet(obj_id player, obj_id pet) throws InterruptedException
	{
		
		if (!beast_lib.isBeastMaster(player))
		{
			sendSystemMessage(player, new string_id("beast", "train_fail_beastmaster_only"));
			return false;
		}
		
		if (!isIdValid(pet) || !exists(pet))
		{
			sendSystemMessage(player, new string_id("beast", "train_fail_invalid_pet"));
			return false;
		}
		
		return true;
	}
	
	
	public static int getSkillCrcByName(String abilityName) throws InterruptedException
	{
		return dataTableGetInt(BEASTS_SPECIALS, abilityName, "abilityCrc");
	}
	
	
	public static String[] playerLearnBeastMasterSkill(obj_id player, String newSkill) throws InterruptedException
	{
		return playerLearnBeastMasterSkill(player, newSkill, true);
	}
	
	
	public static String[] playerLearnBeastMasterSkill(obj_id player, String newSkill, boolean notify) throws InterruptedException
	{
		Vector abilityCrcList = new Vector();
		abilityCrcList.setSize(0);
		
		if (!isLearnableBeastMasterSkill(newSkill))
		{
			return getKnownSkills(player);
		}
		
		if (getKnownSkillsCrc(player) != null)
		{
			abilityCrcList = getKnownSkillsCrc(player);
		}
		
		utils.removeBatchObjVar(player, PLAYER_KNOWN_SKILLS_LIST);
		
		int newSkillCrc = getSkillCrcByName(newSkill);
		int[] castToIntArray = new int[0];
		if (abilityCrcList != null)
		{
			castToIntArray = new int[abilityCrcList.size()];
			for (int _i = 0; _i < abilityCrcList.size(); ++_i)
			{
				castToIntArray[_i] = ((Integer)abilityCrcList.get(_i)).intValue();
			}
		}
		
		if (utils.getElementPositionInArray(castToIntArray, newSkillCrc) > -1)
		{
			
		}
		else
		{
			
			utils.addElement(abilityCrcList, newSkillCrc);
			
			if (notify)
			{
				sendCreatureAbilitySkillLearnSpam(player, newSkill);
			}
		}
		
		if (abilityCrcList != null)
		{
			castToIntArray = new int[abilityCrcList.size()];
			for (int _i = 0; _i < abilityCrcList.size(); ++_i)
			{
				castToIntArray[_i] = ((Integer)abilityCrcList.get(_i)).intValue();
			}
		}
		utils.setBatchObjVar(player, PLAYER_KNOWN_SKILLS_LIST, castToIntArray);
		
		return getKnownSkills(player);
	}
	
	
	public static boolean isLearnableBeastMasterSkill(String skillName) throws InterruptedException
	{
		int row = dataTableSearchColumnForString(skillName, "ability_name", BEASTS_SPECIALS);
		
		if (row == -1)
		{
			return false;
		}
		
		int specialType = getBeastMasterSpecialType(skillName);
		
		return (specialType != ABILITY_INVALID && specialType != ABILITY_TYPE_INNATE);
	}
	
	
	public static int getBeastMasterSpecialType(String skillName) throws InterruptedException
	{
		int row = dataTableSearchColumnForString(skillName, "ability_name", BEASTS_SPECIALS);
		
		if (row == -1)
		{
			return ABILITY_INVALID;
		}
		
		return dataTableGetInt(BEASTS_SPECIALS, row, "specialType");
	}
	
	
	public static boolean hasBeastMasterSkill(obj_id player, String beastAbility) throws InterruptedException
	{
		if (isIdValid(player))
		{
			if (beastAbility != null && !beastAbility.equals(""))
			{
				Vector abilityCrcList = getKnownSkillsCrc(player);
				int newSkillCrc = getSkillCrcByName(beastAbility);
				int[] castToIntArray = new int[0];
				if (abilityCrcList != null)
				{
					castToIntArray = new int[abilityCrcList.size()];
					for (int _i = 0; _i < abilityCrcList.size(); ++_i)
					{
						castToIntArray[_i] = ((Integer)abilityCrcList.get(_i)).intValue();
					}
				}
				
				if (utils.getElementPositionInArray(castToIntArray, newSkillCrc) > -1)
				{
					return true;
				}
			}
		}
		return false;
	}
	
	
	public static void sendCreatureAbilitySkillLearnSpam(obj_id player, String newSkill) throws InterruptedException
	{
		sendSystemMessage(player, new string_id("beast", "ability_learn_new_skill"));
		sendSystemMessage(player, new string_id("cmd_n", newSkill));
	}
	
	
	public static String[] getKnownSkills(obj_id player) throws InterruptedException
	{
		Vector abilityCrc = getKnownSkillsCrc(player);
		
		if (abilityCrc == null || abilityCrc.size() == 0)
		{
			return null;
		}
		
		String[] abilityString = new String[abilityCrc.size()];
		
		for (int i = 0; i < abilityCrc.size(); i++)
		{
			testAbortScript();
			int row = dataTableSearchColumnForInt(((Integer)(abilityCrc.get(i))).intValue(), "abilityCrc", BEASTS_SPECIALS);
			abilityString[i] = dataTableGetString(BEASTS_SPECIALS, row, "ability_name");
		}
		
		if (abilityString == null || abilityString.length == 0)
		{
			return null;
		}
		
		Arrays.sort(abilityString);
		
		return abilityString;
	}
	
	
	public static Vector getKnownSkillsCrc(obj_id player) throws InterruptedException
	{
		if (!utils.hasIntBatchObjVar(player, PLAYER_KNOWN_SKILLS_LIST))
		{
			return null;
		}
		
		return utils.getResizeableIntBatchObjVar(player, PLAYER_KNOWN_SKILLS_LIST);
	}
	
	
	public static void trainPetAbility(obj_id pet, String abilityName) throws InterruptedException
	{
		if (!canLearnSkill(pet, abilityName))
		{
			return;
		}
		
		String[] trainedSkills = getTrainedSkills(pet);
		
		for (int i = 0; i < trainedSkills.length; i++)
		{
			testAbortScript();
			if (getSkillBeastmasterSkillComparison(abilityName, trainedSkills[i]) == SKILL_HIGHER || trainedSkills[i].equals("empty"))
			{
				trainedSkills[i] = abilityName;
				break;
			}
		}
		
		setTrainedPetSkills(pet, trainedSkills);
		sendPetTrainNotification(pet, abilityName);
	}
	
	
	public static void sendPetTrainNotification(obj_id pet, String abilityName) throws InterruptedException
	{
		obj_id master = getMaster(pet);
		
		prose_package pp = new prose_package();
		pp.stringId = new string_id("beast", "train_success_new_ability");
		pp.actor.set(new string_id("cmd_n", abilityName));
		sendSystemMessageProse(master, pp);
	}
	
	
	public static int getExpertiseStat(int baseStat, int expertiseMod, float nerfPercent) throws InterruptedException
	{
		float expertisePercent = (float)expertiseMod/100;
		float baseStatFloat = (float)baseStat;
		float expertiseStatFloat = baseStatFloat - (baseStatFloat * (nerfPercent - (nerfPercent * expertisePercent) ) );
		int expertiseStat = (int)expertiseStatFloat;
		
		return expertiseStat;
	}
	
	
	public static float getExpertiseSpeed(float baseStat, int expertiseMod) throws InterruptedException
	{
		float expertisePercent = (float)expertiseMod/100;
		float expertiseSpeed = baseStat + (baseStat * (1.0f - expertisePercent) );
		
		return expertiseSpeed;
	}
	
	
	public static boolean isPreparedToLearnCreatureAbility(obj_id player) throws InterruptedException
	{
		return buff.hasBuff(player, "bm_creature_knowledge");
	}
	
	
	public static float getCreatureKnowledgeLevel(obj_id player) throws InterruptedException
	{
		int ck_skill = getEnhancedSkillStatisticModifier(player, "bm_creature_knowledge");
		int level = getLevel(player);
		
		float learningPotential = (((float)ck_skill + (float)level) / 10.0f);
		
		return learningPotential;
	}
	
	
	public static boolean makeAbilityLearnSkillCheck(obj_id player, String ability) throws InterruptedException
	{
		float learningPotential = getCreatureKnowledgeLevel(player);
		int challengeRating = getDifficultyToLearnAbility(ability);
		float beastLearnBonus = getBeastLearnBonus(player);
		
		boolean watchRoll = isGod(player) && hasObjVar(player, "viewLearnCreatureAbilityRoll");
		
		if (challengeRating < 1)
		{
			return false;
		}
		
		if (learningPotential < challengeRating)
		{
			if (watchRoll)
			{
				sendSystemMessageTestingOnly(player, "Ability("+ability+") challenge rating too high for user");
			}
			
			return false;
		}
		
		if (learningPotential > challengeRating)
		{
			learningPotential += ((1.0f + (learningPotential - (float)challengeRating)) * 100.0f) + (beastLearnBonus * 10.0f);
		}
		
		float roll_mod = (float)challengeRating * 1000.0f;
		float skillGoal = roll_mod - learningPotential;
		float learnRoll = rand(learningPotential, roll_mod) ;
		boolean skillLearnResult = learnRoll > skillGoal;
		
		if (watchRoll)
		{
			sendSystemMessageTestingOnly(player, "Abiltiy: "+ability+", Random("+learningPotential+", "+roll_mod+") > ("+skillGoal+")"+ " = "+learnRoll+"("+skillLearnResult+")"+", "+beastLearnBonus*10.0f+" chance gained from pet.");
		}
		
		return skillLearnResult;
	}
	
	
	public static int getDifficultyToLearnAbility(String ability) throws InterruptedException
	{
		return dataTableGetInt(BEASTS_SPECIALS, ability, "level_to_learn");
	}
	
	
	public static boolean checkForSkillAcquisition(obj_id attacker, obj_id defender, String actionName) throws InterruptedException
	{
		
		if (isBeast(defender))
		{
			defender = getMaster(defender);
		}
		
		if (!isIdValid(attacker) || !exists(attacker) || isPlayer(attacker) || isBeast(attacker) || pet_lib.isPet(attacker))
		{
			return false;
		}
		
		if (!isIdValid(defender) || !exists(defender) || !isPlayer(defender) || !isPreparedToLearnCreatureAbility(defender))
		{
			return false;
		}
		
		if (!isIdValid(getBeastOnPlayer(defender)))
		{
			return false;
		}
		
		if (!isLearnableCreatureAbility(defender, actionName))
		{
			return false;
		}
		
		if (makeAbilityLearnSkillCheck(defender, actionName))
		{
			playerLearnBeastMasterSkill(defender, actionName);
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isLearnableCreatureAbility(obj_id player, String actionName) throws InterruptedException
	{
		dictionary actionData = dataTableGetRow(BEASTS_SPECIALS, actionName);
		
		if (actionData == null)
		{
			return false;
		}
		
		if (actionData.getInt("level_to_learn") < 0)
		{
			return false;
		}
		
		if (!isValidBeastSkillForCreature(getBeastOnPlayer(player), actionName))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static void doBmProvokeCommand(obj_id self, obj_id target, int provokeLevel) throws InterruptedException
	{
		if (!isIdValid(target) || !exists(target) || isPlayer(target) || isBeast(target))
		{
			return;
		}
		
		obj_id[] enemies = getHateList(target);
		
		if (enemies == null || enemies.length == 0)
		{
			return;
		}
		
		obj_id topHateTarget = getHateTarget(target);
		
		if (topHateTarget == self)
		{
			return;
		}
		
		final int SKILL_LEVEL_CONST = 18;
		final float HATE_RANGE = 0.4f;
		final float HATE_FLOOR = 0.8f;
		
		float provokeResistance = getLevel(target) / SKILL_LEVEL_CONST;
		float provokeRating = provokeLevel / provokeResistance;
		
		float hateThreashold = getHate(target, topHateTarget) * HATE_RANGE;
		float hateFloor = hateThreashold * HATE_FLOOR;
		
		float distanceToFloor = hateFloor - getMaxHate(self);
		
		float modifiedHateRange = hateThreashold * provokeRating;
		
		float finalHateAdd = distanceToFloor + modifiedHateRange;
		
		finalHateAdd = finalHateAdd < 1 ? 1 : finalHateAdd;
		
		addHate(target, self, finalHateAdd);
	}
	
	
	public static void performingActivity(obj_id beastBCD, int activity) throws InterruptedException
	{
		obj_id player = getBCDPlayer(beastBCD);
		
		utils.setScriptVar(beastBCD, PET_ACTIVITY_SCRIPTVAR, activity);
		
		obj_id beast = getBCDBeastCalled(beastBCD);
		
		updateBeastHappiness(beastBCD, beast);
		updateBeastStats(beastBCD, beast);
	}
	
	
	public static void verifyAndUpdateCalledBeastStats(obj_id player) throws InterruptedException
	{
		obj_id beast = getBeastOnPlayer(player);
		
		if (!isIdValid(beast) || !exists(beast))
		{
			removeAttentionPenaltyDebuff(player);
			return;
		}
		
		obj_id BCD = getBeastBCD(beast);
		
		if (!isIdValid(BCD) || !exists(BCD))
		{
			removeAttentionPenaltyDebuff(player);
			return;
		}
		
		initializeBeastStats(BCD, beast);
		
		return;
	}
	
	
	public static void removeAttentionPenaltyDebuff(obj_id player) throws InterruptedException
	{
		for (int i = 1; i < 6; i++)
		{
			testAbortScript();
			if (buff.hasBuff(player, ATTENTION_PENALTY_DEBUFF + i))
			{
				buff.removeBuff(player, ATTENTION_PENALTY_DEBUFF + i);
			}
		}
		
		return;
	}
	
	
	public static int[] fillIncubationArray(obj_id egg, obj_id beastControlDevice) throws InterruptedException
	{
		int[] incubationBonuses = new int[ARRAY_BEAST_INCUBATION_STATS.length];
		
		for (int i = 0; i < ARRAY_BEAST_INCUBATION_STATS.length; ++i)
		{
			testAbortScript();
			if (hasObjVar(egg, ARRAY_BEAST_INCUBATION_STATS[i]))
			{
				incubationBonuses[i] = getIntObjVar(egg, ARRAY_BEAST_INCUBATION_STATS[i]);
			}
			else
			{
				incubationBonuses[i] = 0;
			}
		}
		
		return incubationBonuses;
	}
	
	
	public static string_id convertHappinessString(obj_id beastBCD) throws InterruptedException
	{
		int currentHappiness = getBCDBeastHappiness(beastBCD);
		string_id happiness = convertHappinessString (currentHappiness);
		if (happiness == null)
		{
			LOG("beast", "This beast control device ("+ beastBCD + ") is somehow missing a happiness value. This should not be possible. Ask customer to relog");
		}
		
		return happiness;
	}
	
	
	public static string_id convertHappinessString(int currentHappiness) throws InterruptedException
	{
		if (currentHappiness < -20)
		{
			return SID_BEAST_FURIOUS;
		}
		
		if (currentHappiness < -15)
		{
			return SID_BEAST_ANGRY;
		}
		
		if (currentHappiness < -10)
		{
			return SID_BEAST_MISERABLE;
		}
		
		if (currentHappiness < -5)
		{
			return SID_BEAST_UNHAPPY;
		}
		
		if (currentHappiness < 0)
		{
			return SID_BEAST_ANNOYED;
		}
		
		if (currentHappiness >= 20)
		{
			return SID_BEAST_ECSTATIC;
		}
		
		if (currentHappiness >= 15)
		{
			return SID_BEAST_VERY_HAPPY;
		}
		
		if (currentHappiness >= 10)
		{
			return SID_BEAST_HAPPY;
		}
		
		if (currentHappiness >= 5)
		{
			return SID_BEAST_JOYOUS;
		}
		
		if (currentHappiness >= 0)
		{
			return SID_BEAST_NORMAL;
		}
		else
		{
			return null;
		}
	}
	
	
	public static void applyHappinessBuffIcon(obj_id beast) throws InterruptedException
	{
		obj_id beastBCD = getBeastBCD(beast);
		
		if (!isValidBCD(beastBCD))
		{
			return;
		}
		
		int currentHappiness = getBCDBeastHappiness(beastBCD);
		
		int oldBuff = buff.getBuffOnTargetFromGroup(beast, "bm_happiness_icon");
		
		if (oldBuff != 0)
		{
			buff.removeBuff(beast, oldBuff);
		}
		
		if (currentHappiness <= -20)
		{
			buff.applyBuff(beast, "bm_happiness_furious");
			return;
		}
		
		if (currentHappiness <= -15)
		{
			buff.applyBuff(beast, "bm_happiness_angry");
			return;
		}
		
		if (currentHappiness <= -10)
		{
			buff.applyBuff(beast, "bm_happiness_miserable");
			return;
		}
		
		if (currentHappiness <= -5)
		{
			buff.applyBuff(beast, "bm_happiness_unhappy");
			return;
		}
		
		if (currentHappiness < 0)
		{
			buff.applyBuff(beast, "bm_happiness_annoyed");
			return;
		}
		
		if (currentHappiness >= 20)
		{
			buff.applyBuff(beast, "bm_happiness_ecstatic");
			return;
		}
		
		if (currentHappiness >= 15)
		{
			buff.applyBuff(beast, "bm_happiness_very_happy");
			return;
		}
		
		if (currentHappiness >= 10)
		{
			buff.applyBuff(beast, "bm_happiness_happy");
			return;
		}
		
		if (currentHappiness >= 5)
		{
			buff.applyBuff(beast, "bm_happiness_joyous");
			return;
		}
		
		if (currentHappiness >= 0)
		{
			buff.applyBuff(beast, "bm_happiness_content");
			return;
		}
	}
	
	
	public static string_id convertLoyaltyString(obj_id beastBCD) throws InterruptedException
	{
		int currentLoyalty = getBCDBeastLoyaltyLevel(beastBCD);
		string_id loyalty = convertLoyaltyString(currentLoyalty);
		if (loyalty == null)
		{
			LOG("beast", "This beast control device ("+ beastBCD + ") is somehow missing its Loyalty level or it's a different number than 1 to 5. This should not be possible. Check value in obj var beastmood.beastLoyaltyLevel on the Beast Control Device");
		}
		return loyalty;
	}
	
	
	public static string_id convertLoyaltyString(int currentLoyalty) throws InterruptedException
	{
		switch(currentLoyalty)
		{
			case 1:
			return SID_BEAST_WILD;
			case 2:
			return SID_BEAST_DISOBEDIENT;
			case 3:
			return SID_BEAST_TRAINED;
			case 4:
			return SID_BEAST_LOYAL;
			case 5:
			return SID_BEAST_BFF;
			default:
			return null;
		}
	}
	
	
	public static obj_id generateTypeThreeEnzyme(obj_id player, float enzymeValue) throws InterruptedException
	{
		return generateTypeThreeEnzyme(player, null, enzymeValue, enzymeValue, "none");
	}
	
	
	public static obj_id generateTypeThreeEnzyme(obj_id player, float enzymePurity, float enzymeMutagen) throws InterruptedException
	{
		return generateTypeThreeEnzyme(player, null, enzymePurity, enzymeMutagen, "none");
	}
	
	
	public static obj_id generateTypeThreeEnzyme(obj_id player, obj_id target, float enzymePurity, float enzymeMutagen, String trait) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			obj_id newEnzyme = createObjectInInventoryAllowOverload("object/tangible/loot/beast/enzyme_3.iff", player);
			setObjVar(newEnzyme, "enzyme.enzyme_purity", enzymePurity);
			setObjVar(newEnzyme, "enzyme.enzyme_mutagen", enzymeMutagen);
			setObjVar(newEnzyme, "enzyme.trait", trait);
			return newEnzyme;
		}
		
		int skillMod = getEnhancedSkillStatisticModifierUncapped(player, "expertise_bm_genetic_engineering");
		float ge_mod = 1.0f + (skillMod / 1000.0f);
		
		float levelDifference = (float)getLevel(target) / (float)getLevel(player);
		levelDifference = levelDifference > 1.0f ? 1.0f : levelDifference;
		
		float valuePerLevel = 0.02f;
		
		float levelAddedValue = (float)getLevel(target) * valuePerLevel;
		
		float randomFloor = levelAddedValue / 2.0f;
		float randomCeiling = levelAddedValue * 2.1f;
		
		float eliteValueAdd = levelAddedValue * 0.7f;
		float bossValueAdd = levelAddedValue * 1.3f;
		
		int difficultyClass = getIntObjVar(target, "difficultyClass");
		int seed = 0;
		
		switch (difficultyClass)
		{
			case 0:
			seed = 1;
			break;
			case 1: 
			seed = 3;
			randomCeiling += eliteValueAdd;
			break;
			case 2: 
			seed = 5;
			randomCeiling += bossValueAdd;
			break;
		}
		
		float basePurity = distributedRand(randomFloor, randomCeiling, seed);
		float baseMutagen = distributedRand(randomFloor, randomCeiling, seed);
		
		enzymePurity += basePurity;
		enzymeMutagen += baseMutagen;
		
		enzymePurity *= ge_mod;
		enzymeMutagen *= ge_mod;
		
		enzymePurity *= levelDifference;
		enzymeMutagen *= levelDifference;
		
		addToExtractionList(player, target);
		
		obj_id newEnzyme = createObjectInInventoryAllowOverload("object/tangible/loot/beast/enzyme_3.iff", player);
		setObjVar(newEnzyme, "enzyme.enzyme_purity", enzymePurity);
		setObjVar(newEnzyme, "enzyme.enzyme_mutagen", enzymeMutagen);
		setObjVar(newEnzyme, "enzyme.trait", trait);
		
		return newEnzyme;
	}
	
	
	public static int getEnzymeExtractionReturnCode(obj_id player, obj_id creatureCorpse) throws InterruptedException
	{
		if (!isBeastMaster(player))
		{
			return ENZ_ERROR_NOT_BEAST_MASTER;
		}
		if (!isIdValid(creatureCorpse))
		{
			return ENZ_ERROR_INVALID_TARGET;
		}
		
		if (!ai_lib.isAiDead(creatureCorpse))
		{
			return ENZ_ERROR_NOT_DEAD;
		}
		
		if (!ai_lib.isMonster(creatureCorpse) || storyteller.isAnyNpc(creatureCorpse))
		{
			return ENZ_ERROR_TARGET_NOT_MONSTER;
		}
		
		if (beast_lib.isBeast(creatureCorpse) || pet_lib.isPet(creatureCorpse))
		{
			return ENZ_ERROR_TARGET_IS_PET;
		}
		
		Vector extractions = getExtractionList(creatureCorpse);
		
		int numExtractions = 0;
		
		if (extractions != null && extractions.size() > 0)
		{
			numExtractions = extractions.size();
		}
		
		if (numExtractions > 0 && extractions.contains(player))
		{
			return ENZ_ERROR_ALREADY_HARVEST;
		}
		
		if (numExtractions >= EXTRACTION_LIMIT)
		{
			return ENZ_ERROR_TOO_MANY_EXTRACTIONS;
		}
		
		if (!corpse.hasLootPermissions(creatureCorpse, player))
		{
			return ENZ_ERROR_NO_LOOT_PERMISSION;
		}
		
		return -1;
	}
	
	
	public static Vector getExtractionList(obj_id corpse) throws InterruptedException
	{
		if (hasObjVar(corpse, EXTRACTION_LIST))
		{
			return getResizeableObjIdArrayObjVar(corpse, EXTRACTION_LIST);
		}
		
		Vector extractionList = new Vector();
		extractionList.setSize(0);
		
		return extractionList;
	}
	
	
	public static void addToExtractionList(obj_id player, obj_id corpse) throws InterruptedException
	{
		Vector extractionList = getExtractionList(corpse);
		utils.addElement(extractionList, player);
		
		setObjVar(corpse, EXTRACTION_LIST, extractionList, resizeableArrayTypeobj_id);
	}
	
	
	public static float distributedRand(float min, float max, int level) throws InterruptedException
	{
		final int levelMin = 0;
		final int levelMax = 10;
		
		boolean inverted = false;
		float _min = min;
		float _max = max;
		
		if (min > max)
		{
			inverted = true;
			min = _max;
			max = _min;
		}
		
		float rank = (float)(level - levelMin) / (float)(levelMax - levelMin);
		
		float mid = min + ((max - min) * rank);
		
		if (mid < min)
		{
			max += (mid-min);
			mid = min;
		}
		if (mid > max)
		{
			min += (mid-max);
			mid = max;
		}
		
		float minRand = rand(min, mid);
		float maxRand = rand(mid, max);
		
		float randNum = rand(minRand, maxRand);
		
		if (inverted)
		{
			randNum = _min + (_max - randNum);
		}
		
		return randNum;
	}
	
	
	public static String[] getBeastmasterPetBarData(obj_id pet) throws InterruptedException
	{
		return getBeastmasterPetBarData(getMaster(pet), pet);
	}
	
	
	public static String[] getBeastmasterPetBarData(obj_id player, obj_id pet) throws InterruptedException
	{
		
		String[] barData = (String[])PET_BAR_DEFAULT_ARRAY.clone();
		
		String[] knownSkills = getTrainedSkills(pet);
		
		int additionalAbilitySlot = 0;
		additionalAbilitySlot = getSkillStatisticModifier(player, "expertise_bm_add_pet_bar");
		
		if (hasCommand(player, BM_COMMAND_ATTACK))
		{
			barData[0] = BM_COMMAND_ATTACK;
		}
		
		if (hasCommand(player, BM_COMMAND_ATTACK) || getBeastMasterSpecialType(knownSkills[0]) == ABILITY_TYPE_NONCOMBAT)
		{
			barData[3] = knownSkills[0];
		}
		
		if (additionalAbilitySlot == 0)
		{
			return barData;
		}
		
		for (int i=0; i < additionalAbilitySlot; i++)
		{
			testAbortScript();
			
			if (additionalAbilitySlot > 3)
			{
				break;
			}
			barData[i + 4] = knownSkills[i + 1];
		}
		return barData;
	}
	
	
	public static boolean isChargeAttack(String actionName) throws InterruptedException
	{
		if (actionName.startsWith("bm_charge") || actionName.startsWith("bm_trample"))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean setBCDBeastHueFromEgg(obj_id bcd, obj_id egg) throws InterruptedException
	{
		if (!isIdValid(bcd) || !exists(bcd) || !isIdValid(egg) || !exists(egg))
		{
			return false;
		}
		
		if (hasObjVar(egg, OBJVAR_BEAST_HUE))
		{
			int beastHue = getIntObjVar(egg, OBJVAR_BEAST_HUE);
			
			setBCDBeastHuePrimary(bcd, "/private/index_color_1", beastHue);
			return true;
		}
		
		return false;
	}
	
	
	public static void setBCDBeastHuePrimary(obj_id bcd, String palette, int hueIndex) throws InterruptedException
	{
		LOG("barnStorage", "1 bcd: "+ bcd + " palette: "+ palette + " hueIndex: "+ hueIndex);
		
		if (!isIdValid(bcd) || !exists(bcd) || palette == null || palette.length() <= 0 || hueIndex < 0)
		{
			return;
		}
		
		LOG("barnStorage", "2 bcd: "+ bcd + " palette: "+ palette + " hueIndex: "+ hueIndex);
		
		hue.setColor(bcd, palette, hueIndex);
		setObjVar(bcd, OBJVAR_BEAST_HUE, hueIndex);
		setObjVar(bcd, OBJVAR_BEAST_PALETTE, palette);
	}
	
	
	public static void setBeastHuePrimary(obj_id beast, String palette, int hueIndex) throws InterruptedException
	{
		obj_id bcd = getBeastBCD(beast);
		
		LOG("barnStorage", " setBeastHuePrimary bcd: "+ bcd + "beast: "+ beast + " palette: "+ palette + " hueIndex: "+ hueIndex);
		
		if (!isIdValid(bcd) || !exists(bcd) || palette == null || palette.length() <= 0 || hueIndex < 0)
		{
			return;
		}
		
		LOG("barnStorage", "setBeastHuePrimary bcd: "+ bcd + " palette: "+ palette + " hueIndex: "+ hueIndex);
		
		setBCDBeastHuePrimary(bcd, palette, hueIndex);
		
		hue.setColor(beast, palette, hueIndex);
	}
	
	
	public static String getBCDPalettePrimary(obj_id bcd) throws InterruptedException
	{
		if (!isIdValid(bcd) || !exists(bcd) || !hasObjVar(bcd, OBJVAR_BEAST_PALETTE))
		{
			return "";
		}
		
		return getStringObjVar(bcd, OBJVAR_BEAST_PALETTE);
	}
	
	
	public static int getBCDHuePrimary(obj_id bcd) throws InterruptedException
	{
		if (!isIdValid(bcd) || !exists(bcd) || !hasObjVar(bcd, OBJVAR_BEAST_HUE))
		{
			return -1;
		}
		
		return getIntObjVar(bcd, OBJVAR_BEAST_HUE);
	}
	
	
	public static String getBeastPalettePrimary(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast) || !exists(beast))
		{
			return "";
		}
		
		obj_id bcd = getBeastBCD(beast);
		
		if (!isIdValid(bcd) || !exists(bcd))
		{
			return "";
		}
		
		return getBCDPalettePrimary(bcd);
	}
	
	
	public static int getBeastHuePrimary(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast) || !exists(beast))
		{
			return -1;
		}
		
		obj_id bcd = getBeastBCD(beast);
		
		if (!isIdValid(bcd) || !exists(bcd))
		{
			return -1;
		}
		
		return getBCDHuePrimary(bcd);
	}
	
	
	public static void setBCDBeastHueSecondary(obj_id bcd, String palette, int hueIndex) throws InterruptedException
	{
		
		if (!isIdValid(bcd) || !exists(bcd) || palette == null || palette.length() <= 0 || hueIndex < 0)
		{
			return;
		}
		
		hue.setColor(bcd, palette, hueIndex);
		setObjVar(bcd, OBJVAR_BEAST_HUE2, hueIndex);
		setObjVar(bcd, OBJVAR_BEAST_PALETTE2, palette);
	}
	
	
	public static void setBeastHueSecondary(obj_id beast, String palette, int hueIndex) throws InterruptedException
	{
		obj_id bcd = getBeastBCD(beast);
		
		LOG("barnStorage", "setBeastHueSecondary bcd: "+ bcd + "beast: "+ beast + " palette: "+ palette + " hueIndex: "+ hueIndex);
		
		if (!isIdValid(bcd) || !exists(bcd) || palette == null || palette.length() <= 0 || hueIndex < 0)
		{
			return;
		}
		
		LOG("barnStorage", "setBeastHueSecondary bcd: "+ bcd + " palette: "+ palette + " hueIndex: "+ hueIndex);
		
		setBCDBeastHueSecondary(bcd, palette, hueIndex);
		
		hue.setColor(beast, palette, hueIndex);
	}
	
	
	public static String getBCDPaletteSecondary(obj_id bcd) throws InterruptedException
	{
		if (!isIdValid(bcd) || !exists(bcd) || !hasObjVar(bcd, OBJVAR_BEAST_PALETTE2))
		{
			return "";
		}
		
		return getStringObjVar(bcd, OBJVAR_BEAST_PALETTE2);
	}
	
	
	public static int getBCDHueSecondary(obj_id bcd) throws InterruptedException
	{
		if (!isIdValid(bcd) || !exists(bcd) || !hasObjVar(bcd, OBJVAR_BEAST_HUE2))
		{
			return -1;
		}
		
		return getIntObjVar(bcd, OBJVAR_BEAST_HUE2);
	}
	
	
	public static String getBeastPaletteSecondary(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast) || !exists(beast))
		{
			return "";
		}
		
		obj_id bcd = getBeastBCD(beast);
		
		if (!isIdValid(bcd) || !exists(bcd))
		{
			return "";
		}
		
		return getBCDPaletteSecondary(bcd);
	}
	
	
	public static int getBeastHueSecondary(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast) || !exists(beast))
		{
			return -1;
		}
		
		obj_id bcd = getBeastBCD(beast);
		
		if (!isIdValid(bcd) || !exists(bcd))
		{
			return -1;
		}
		
		return getBCDHueSecondary(bcd);
	}
	
	
	public static void setBCDBeastHueThird(obj_id bcd, String palette, int hueIndex) throws InterruptedException
	{
		
		if (!isIdValid(bcd) || !exists(bcd) || palette == null || palette.length() <= 0 || hueIndex < 0)
		{
			return;
		}
		
		hue.setColor(bcd, palette, hueIndex);
		setObjVar(bcd, OBJVAR_BEAST_HUE3, hueIndex);
		setObjVar(bcd, OBJVAR_BEAST_PALETTE3, palette);
	}
	
	
	public static void setBeastHueThird(obj_id beast, String palette, int hueIndex) throws InterruptedException
	{
		obj_id bcd = getBeastBCD(beast);
		
		if (!isIdValid(bcd) || !exists(bcd) || palette == null || palette.length() <= 0 || hueIndex < 0)
		{
			return;
		}
		
		setBCDBeastHueThird(bcd, palette, hueIndex);
		
		hue.setColor(beast, palette, hueIndex);
	}
	
	
	public static String getBCDPaletteThird(obj_id bcd) throws InterruptedException
	{
		if (!isIdValid(bcd) || !exists(bcd) || !hasObjVar(bcd, OBJVAR_BEAST_PALETTE3))
		{
			return "";
		}
		
		return getStringObjVar(bcd, OBJVAR_BEAST_PALETTE3);
	}
	
	
	public static int getBCDHueThird(obj_id bcd) throws InterruptedException
	{
		if (!isIdValid(bcd) || !exists(bcd) || !hasObjVar(bcd, OBJVAR_BEAST_HUE3))
		{
			return -1;
		}
		
		return getIntObjVar(bcd, OBJVAR_BEAST_HUE3);
	}
	
	
	public static String getBeastPaletteThird(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast) || !exists(beast))
		{
			return "";
		}
		
		obj_id bcd = getBeastBCD(beast);
		
		if (!isIdValid(bcd) || !exists(bcd))
		{
			return "";
		}
		
		return getBCDPaletteThird(bcd);
	}
	
	
	public static int getBeastHueThird(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast) || !exists(beast))
		{
			return -1;
		}
		
		obj_id bcd = getBeastBCD(beast);
		
		if (!isIdValid(bcd) || !exists(bcd))
		{
			return -1;
		}
		
		return getBCDHueThird(bcd);
	}
	
	
	public static void clearBeastColor(obj_id beast) throws InterruptedException
	{
		obj_id bcd = getBeastBCD(beast);
		
		if (!isIdValid(bcd) || !exists(bcd))
		{
			return;
		}
		
		removeObjVar(bcd, OBJVAR_BEAST_HUE);
		removeObjVar(bcd, OBJVAR_BEAST_HUE2);
		removeObjVar(bcd, OBJVAR_BEAST_HUE3);
		
		removeObjVar(bcd, OBJVAR_BEAST_PALETTE);
		removeObjVar(bcd, OBJVAR_BEAST_PALETTE2);
		removeObjVar(bcd, OBJVAR_BEAST_PALETTE3);
		
	}
	
	
	public static void initializeBeastColor(obj_id beast) throws InterruptedException
	{
		obj_id bcd = getBeastBCD(beast);
		
		if (!isIdValid(bcd) || !exists(bcd))
		{
			return;
		}
		
		int primaryHue = getBCDHuePrimary(bcd);
		int secondaryHue = getBCDHueSecondary(bcd);
		int thirdHue = getBCDHueThird(bcd);
		
		String primaryPalette = getBCDPalettePrimary(bcd);
		String secondaryPalette = getBCDPaletteSecondary(bcd);
		String thirdPalette = getBCDPaletteThird(bcd);
		
		if (primaryHue >= 0 && (primaryPalette == null || primaryPalette.length() <= 0))
		{
			setBeastHuePrimary(beast, "/private/index_color_1", primaryHue);
			return;
		}
		
		setBeastHuePrimary(beast, primaryPalette, primaryHue);
		setBeastHueSecondary(beast, secondaryPalette, secondaryHue);
		setBeastHueThird(beast, thirdPalette, thirdHue);
	}
	
	
	public static boolean addInnateBeastBonuses(obj_id beast, String beastType) throws InterruptedException
	{
		int row = dataTableSearchColumnForString(beastType, "beastType", BEASTS_TABLE);
		
		if (row < 0)
		{
			return false;
		}
		
		int intBonusGroup = dataTableGetInt(BEASTS_TABLE, row, "attribute_bonus_class");
		
		row = dataTableSearchColumnForInt(intBonusGroup, "intNameClass", BEASTS_SPECIES_BONUS_TABLE);
		
		String strBonusGroup = dataTableGetString(BEASTS_SPECIES_BONUS_TABLE, row, "stringNameClass");
		
		int columnNumber = dataTableFindColumnNumber(BEASTS_SPECIES_BONUS_TABLE, strBonusGroup);
		
		int incrementColumnNumber = columnNumber + 1;
		
		String[] attributesAffected = dataTableGetStringColumnNoDefaults(BEASTS_SPECIES_BONUS_TABLE, strBonusGroup);
		
		int[] incrementAmounts = dataTableGetIntColumnNoDefaults(BEASTS_SPECIES_BONUS_TABLE, incrementColumnNumber);
		
		if (attributesAffected.length != incrementAmounts.length)
		{
			return false;
		}
		
		for (int i = 0; i < attributesAffected.length; ++i)
		{
			testAbortScript();
			String fullName = "beast."+attributesAffected[i];
			
			for (int j = 0; j < ARRAY_BEAST_INCUBATION_STATS.length; ++j)
			{
				testAbortScript();
				
				if (fullName.equals(ARRAY_BEAST_INCUBATION_STATS[j]))
				{
					float currentBonus = 0.0f;
					
					if (utils.hasScriptVar(beast, ARRAY_BEAST_INCUBATION_STATS[j]))
					{
						
						currentBonus = utils.getFloatScriptVar(beast, ARRAY_BEAST_INCUBATION_STATS[j]);
					}
					
					float newBonus = currentBonus + (float)incrementAmounts[i];
					utils.setScriptVar(beast, ARRAY_BEAST_INCUBATION_STATS[j], newBonus);
					break;
				}
			}
		}
		
		return true;
	}
	
	
	public static dictionary getBeastMasterExamineInfo(obj_id player, obj_id creature, String[] names, String[] attribs, int idx) throws InterruptedException
	{
		dictionary dict = new dictionary();
		dict.put("names", names);
		dict.put("attribs", attribs);
		dict.put("idx", idx);
		
		if (!buff.hasBuff(player, "bm_creature_knowledge") || !ai_lib.isMonster(creature) || !utils.hasScriptVar(player, "creature_knowledge.species"))
		{
			return dict;
		}
		
		if (hasObjVar(creature, "bm_doNotShowExamineInfo"))
		{
			return dict;
		}
		
		int species = utils.getIntScriptVar(player, "creature_knowledge.species");
		
		if (species != getSpecies(creature))
		{
			return dict;
		}
		
		int ck_skill = getEnhancedSkillStatisticModifierUncapped(player, "bm_creature_knowledge");
		
		String creatureName = getCreatureName(creature);
		dictionary creatureDict = dataTableGetRow("datatables/mob/creatures.iff", creatureName);
		
		if (ck_skill > -1)
		{
			if (creatureDict.getString("hideType") != "none")
			{
				names[idx] = "creature_resource_hide";
				attribs[idx] = utils.packStringId(new string_id("obj_attr_n", creatureDict.getString("hideType")));
				idx++;
			}
			if (creatureDict.getString("meatType") != "none")
			{
				names[idx] = "creature_resource_meat";
				attribs[idx] = utils.packStringId(new string_id("obj_attr_n", creatureDict.getString("meatType")));
				idx++;
			}
			if (creatureDict.getString("boneType") != "none")
			{
				names[idx] = "creature_resource_bone";
				attribs[idx] = utils.packStringId(new string_id("obj_attr_n", creatureDict.getString("boneType")));
				idx++;
			}
			
		}
		
		if (ck_skill > 0)
		{
			names[idx] = "aggro";
			attribs[idx] = ""+creatureDict.getFloat("aggressive");
			idx++;
			
			names[idx] = "assist_range";
			attribs[idx] = ""+creatureDict.getFloat("assist");
			idx++;
			
			names[idx] = "deathblow";
			boolean deathBlow = (creatureDict.getInt("death_blow") > 0);
			attribs[idx] = deathBlow ? "True" : "False";
			idx++;
			
		}
		
		dictionary statDict = dataTableGetRow("datatables/mob/stat_balance.iff", creatureDict.getInt("BaseLevel"));
		int difficultyClass = getIntObjVar(creature, "difficultyClass");
		String prefix = "";
		
		switch(difficultyClass)
		{
			case 1:
			prefix = "Elite_";
			break;
			case 2:
			prefix = "Boss_";
			break;
			default:
			break;
		}
		
		if (ck_skill > 3)
		{
			float dps = statDict.getFloat(prefix+"damagePerSecond");
			float attackSpeed = creatureDict.getFloat("primary_weapon_speed");
			int minDamage = Math.round((dps * attackSpeed) *0.5f);
			int maxDamage = Math.round((dps * attackSpeed) *1.5f);
			
			names[idx] = "damage_min";
			attribs[idx] = ""+minDamage;
			idx++;
			
			names[idx] = "damage_max";
			attribs[idx] = ""+maxDamage;
			idx++;
			
			names[idx] = "attackspeed";
			attribs[idx] = ""+attackSpeed;
			idx++;
			
			names[idx] = "damage_dps";
			attribs[idx] = ""+dps;
			idx++;
			
		}
		
		if (ck_skill > 5)
		{
			names[idx] = "armor_rating";
			attribs[idx] = ""+statDict.getInt(prefix+"Armor");
			idx++;
		}
		
		if (ck_skill > 7)
		{
			String special_list = creatureDict.getString("primary_weapon_specials");
			
			if (special_list != null && !special_list.equals(""))
			{
				Vector actionList = new Vector();
				actionList.setSize(0);
				
				for (int i = 1; i < 15; i++)
				{
					testAbortScript();
					String ability = dataTableGetString("datatables/ai/ai_combat_profiles.iff", special_list, "action"+i);
					
					if (ability != null && !ability.equals("") && actionList.indexOf(ability) == -1)
					{
						utils.addElement(actionList, ability);
					}
					
				}
				
				if (actionList.size() > 0)
				{
					for (int k = 0; k < actionList.size(); k++)
					{
						testAbortScript();
						names[idx] = "special_attack";
						attribs[idx] = utils.packStringId(new string_id("cmd_n", ((String)(actionList.get(k)))));
						idx++;
					}
				}
			}
			
		}
		
		dict.put("names", names);
		dict.put("attribs", attribs);
		dict.put("idx", idx);
		
		return dict;
	}
	
	
	public static String getBestBeastMasterSkill(obj_id player, String skillName) throws InterruptedException
	{
		String[] knownSkills = getKnownSkills(player);
		
		if (knownSkills == null || knownSkills.length == 0)
		{
			return skillName;
		}
		
		String passedSkillLine = getSkillLine(skillName);
		int passedSkillRank = getSkillRank(skillName);
		
		String bestSkill = skillName;
		
		for (int i=0; i<knownSkills.length; i++)
		{
			testAbortScript();
			String skillLine = getSkillLine(knownSkills[i]);
			int skillRank = getSkillRank(knownSkills[i]);
			
			if (!skillLine.equals(passedSkillLine))
			{
				continue;
			}
			
			if (skillRank <= passedSkillRank)
			{
				continue;
			}
			
			bestSkill = knownSkills[i];
		}
		
		return bestSkill;
	}
	
	
	public static String getNextSkillInLine(String skillName) throws InterruptedException
	{
		String[] allSkills = dataTableGetStringColumn(BEASTS_SPECIALS, "ability_name");
		
		String passedSkillLine = getSkillLine(skillName);
		int passedSkillRank = getSkillRank(skillName);
		int nextRank = passedSkillRank + 1;
		for (int i=0; i<allSkills.length; i++)
		{
			testAbortScript();
			String skillLine = getSkillLine(allSkills[i]);
			int skillRank = getSkillRank(allSkills[i]);
			if (!skillLine.equals(passedSkillLine))
			{
				continue;
			}
			
			if (skillRank == nextRank)
			{
				return allSkills[i];
			}
		}
		
		return skillName;
	}
	
	
	public static int getSkillBeastmasterSkillComparison(String querySkill, String skillToCompare) throws InterruptedException
	{
		
		String queryLine = getSkillLine(querySkill);
		String compareLine = getSkillLine(skillToCompare);
		
		if (!queryLine.equals(compareLine))
		{
			return SKILL_DIFFERENT_LINE;
		}
		
		int queryRank = getSkillRank(querySkill);
		int compareRank = getSkillRank(skillToCompare);
		
		if (queryRank < compareRank)
		{
			return SKILL_LOWER;
		}
		
		if (queryRank == compareRank)
		{
			return SKILL_SAME;
		}
		
		if (queryRank > compareRank)
		{
			return SKILL_HIGHER;
		}
		
		return SKILL_CANNOT_COMPARE;
	}
	
	
	public static String getSkillLine(String skillName) throws InterruptedException
	{
		return dataTableGetString(BEASTS_SPECIALS, skillName, "skill_line");
	}
	
	
	public static int getSkillRank(String skillName) throws InterruptedException
	{
		return dataTableGetInt(BEASTS_SPECIALS, skillName, "skill_rank");
	}
	
	
	public static void updateBeastStats(obj_id bcd, obj_id beast) throws InterruptedException
	{
		if (!isValidBCD(bcd) || !isValidBeast(beast))
		{
			return;
		}
		
		int level = getBCDBeastLevel(bcd);
		
		String beastType = getBCDBeastType(bcd);
		
		if (beastType == null || beastType.length() <= 0)
		{
			blog("updateBeastStats() bad beast type.");
			return;
		}
		
		dictionary beastDict = utils.dataTableGetRow(BEASTS_TABLE, beastType);
		
		if (beastDict == null || beastDict.size() <= 0)
		{
			blog("updateBeastStats() missing entry in the "+ BEASTS_TABLE + " table for beastType: "+ beastType + ".");
			return;
		}
		
		dictionary beastStatsDict = utils.dataTableGetRow(BEASTS_STATS, level - 1);
		
		if (beastStatsDict == null || beastStatsDict.size() <= 0)
		{
			blog("updateBeastStats() missing entry in the "+ BEASTS_STATS + " table for beastType: "+ beastType + ".");
			return;
		}
		
		int expertiseDamage = getEnhancedSkillStatisticModifierUncapped(getMaster(beast), "expertise_bm_pet_damage");
		
		float incubationDamageBonus = utils.getFloatScriptVar(beast, OBJVAR_INCREASE_DPS);
		
		int intMinDamage = (int)beastStatsDict.getInt("MinDmg");
		int intMaxDamage = (int)beastStatsDict.getInt("MaxDmg");
		intMinDamage = getExpertiseStat(intMinDamage, expertiseDamage, .5f);
		intMaxDamage = getExpertiseStat(intMaxDamage, expertiseDamage, .5f);
		
		float floatMinDamage = (float)intMinDamage * (1.0f + incubationDamageBonus / 100.0f);
		float floatMaxDamage = (float)intMaxDamage * (1.0f + incubationDamageBonus / 100.0f);
		intMinDamage = (int)floatMinDamage;
		intMaxDamage = (int)floatMaxDamage;
		
		obj_id beastWeapon = getCurrentWeapon(beast);
		
		if (isIdValid(beastWeapon))
		{
			setWeaponMaxDamage(beastWeapon, intMaxDamage);
			setWeaponMinDamage(beastWeapon, intMinDamage);
			weapons.setWeaponData(beastWeapon);
		}
		
		obj_id defaultWeapon = getDefaultWeapon(beast);
		
		if (isIdValid(defaultWeapon))
		{
			setWeaponMaxDamage(defaultWeapon, intMaxDamage);
			setWeaponMinDamage(defaultWeapon, intMinDamage);
			weapons.setWeaponData(defaultWeapon);
		}
		
		updateBeastHappiness(bcd, beast);
	}
	
	
	public static void setBCDIncubationStats(obj_id bcd, obj_id egg) throws InterruptedException
	{
		int[] incubationBonuses = fillIncubationArray(egg, bcd);
		setObjVar(bcd, OBJVAR_BEAST_INCUBATION_BONUSES, incubationBonuses);
		
		String parentTemplate = getStringObjVar(egg, OBJVAR_BEAST_PARENT);
		setObjVar(bcd, OBJVAR_BEAST_PARENT, parentTemplate);
		
		String creatorName = getStringObjVar(egg, OBJVAR_BEAST_ENGINEER);
		setObjVar(bcd, OBJVAR_BEAST_ENGINEER, creatorName);
	}
	
	
	public static void setBeastSkillBonuses(obj_id beast) throws InterruptedException
	{
		setBeastCritChance(beast);
		setBeastStrikethroughChance(beast);
		setBeastStrikethroughValue(beast);
		setBeastBlockChance(beast);
		setBeastBlockValue(beast);
		setBeastDodgeChance(beast);
		setBeastParryChance(beast);
		setBeastEvadeChance(beast);
		setBeastEvadeValue(beast);
	}
	
	
	public static boolean setBeastCritChance(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast) || !exists(beast))
		{
			return false;
		}
		
		if (!isBeast(beast))
		{
			return false;
		}
		
		float hi_mod = getBeastCustomMod(beast, MOD_HUNTERS_INSTINCT);
		
		float critChance = hi_mod * 0.8f;
		
		critChance += (utils.getFloatScriptVar(beast, OBJVAR_INCREASE_CRITICAL_HIT) / 10.0f);
		
		utils.setScriptVar(beast, OBJVAR_INCREASE_CRITICAL_HIT, critChance);
		return true;
	}
	
	
	public static float getBeastCritChance(obj_id attacker) throws InterruptedException
	{
		if (isPlayer(attacker))
		{
			obj_id beast = getBeastOnPlayer(attacker);
			
			if (!isIdValid(beast) || !exists(beast))
			{
				return 0.0f;
			}
			
			float cun_mod = getBeastCunningRating(beast);
			float crit_mod = getBeastCritChance(beast);
			
			cun_mod = (cun_mod / 100.0f);
			
			return crit_mod * cun_mod;
		}
		
		return utils.getFloatScriptVar(attacker, OBJVAR_INCREASE_CRITICAL_HIT);
	}
	
	
	public static boolean setBeastStrikethroughChance(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast) || !exists(beast))
		{
			return false;
		}
		
		if (!isBeast(beast))
		{
			return false;
		}
		
		float hi_mod = getBeastCustomMod(beast, MOD_HUNTERS_INSTINCT);
		
		float strikeChance = hi_mod * 0.8f;
		
		strikeChance += (utils.getFloatScriptVar(beast, OBJVAR_INCREASE_STRIKETHROUGH) / 10.0f);
		
		utils.setScriptVar(beast, OBJVAR_INCREASE_STRIKETHROUGH, strikeChance);
		
		return true;
	}
	
	
	public static float getBeastStrikethroughChance(obj_id attacker) throws InterruptedException
	{
		if (isPlayer(attacker))
		{
			obj_id beast = getBeastOnPlayer(attacker);
			
			if (!isIdValid(beast) || !exists(beast))
			{
				return 0.0f;
			}
			
			float cun_mod = getBeastCunningRating(beast);
			float strike_mod = getBeastStrikethroughChance(beast);
			
			cun_mod = (cun_mod / 100.0f);
			
			return strike_mod * cun_mod;
		}
		
		return utils.getFloatScriptVar(attacker, OBJVAR_INCREASE_STRIKETHROUGH);
	}
	
	
	public static boolean setBeastStrikethroughValue(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast) || !exists(beast))
		{
			return false;
		}
		
		if (!isBeast(beast))
		{
			return false;
		}
		
		float int_mod = getBeastCustomMod(beast, MOD_INTELLIGENCE);
		
		float strikeValue = int_mod * 2.0f;
		
		strikeValue += utils.getFloatScriptVar(beast, OBJVAR_INCREASE_STRIKETHROUGH_RATING);
		
		utils.setScriptVar(beast, OBJVAR_INCREASE_STRIKETHROUGH_RATING, strikeValue);
		return true;
	}
	
	
	public static float getBeastStrikethroughValue(obj_id attacker) throws InterruptedException
	{
		if (isPlayer(attacker))
		{
			obj_id beast = getBeastOnPlayer(attacker);
			
			if (!isIdValid(beast) || !exists(beast))
			{
				return 0.0f;
			}
			
			float cun_mod = getBeastCunningRating(beast);
			float strike_value = getBeastStrikethroughValue(beast);
			
			cun_mod = (cun_mod / 100.0f);
			
			return strike_value * cun_mod;
		}
		
		return utils.getFloatScriptVar(attacker, OBJVAR_INCREASE_STRIKETHROUGH_RATING);
	}
	
	
	public static float getBeastHitChance(obj_id attacker) throws InterruptedException
	{
		if (isPlayer(attacker))
		{
			obj_id beast = getBeastOnPlayer(attacker);
			
			if (!isIdValid(beast) || !exists(beast))
			{
				return 0.0f;
			}
			
			float cun_mod = getBeastCunningRating(beast);
			float hit_mod = getBeastHitChance(beast);
			
			cun_mod = (cun_mod / 100.0f);
			
			return hit_mod * cun_mod;
		}
		
		return (utils.getFloatScriptVar(attacker, OBJVAR_INCREASE_HIT_CHANCE) / 10.0f);
	}
	
	
	public static boolean setBeastBlockChance(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast) || !exists(beast))
		{
			return false;
		}
		
		if (!isBeast(beast))
		{
			return false;
		}
		
		float svl_mod = getBeastCustomMod(beast, MOD_SURVIVAL);
		
		float blockChance = svl_mod * 0.65f;
		
		blockChance += (utils.getFloatScriptVar(beast, OBJVAR_INCREASE_BLOCK_CHANCE) / 10.0f);
		
		utils.setScriptVar(beast, OBJVAR_INCREASE_BLOCK_CHANCE, blockChance);
		return true;
	}
	
	
	public static float getBeastBlockChance(obj_id attacker) throws InterruptedException
	{
		if (isPlayer(attacker))
		{
			obj_id beast = getBeastOnPlayer(attacker);
			
			if (!isIdValid(beast) || !exists(beast))
			{
				return 0.0f;
			}
			
			float cun_mod = getBeastCunningRating(beast);
			float block_mod = getBeastBlockChance(beast);
			
			cun_mod = (cun_mod / 100.0f);
			
			return block_mod * cun_mod;
		}
		
		return utils.getFloatScriptVar(attacker, OBJVAR_INCREASE_BLOCK_CHANCE);
	}
	
	
	public static boolean setBeastBlockValue(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast) || !exists(beast))
		{
			return false;
		}
		
		if (!isBeast(beast))
		{
			return false;
		}
		
		float int_mod = getBeastCustomMod(beast, MOD_INTELLIGENCE);
		
		float blockValue = int_mod * 25;
		
		blockValue += utils.getFloatScriptVar(beast, OBJVAR_INCREASE_BLOCK_VALUE);
		
		utils.setScriptVar(beast, OBJVAR_INCREASE_BLOCK_VALUE, blockValue);
		return true;
	}
	
	
	public static int getBeastBlockValue(obj_id attacker) throws InterruptedException
	{
		if (isPlayer(attacker))
		{
			obj_id beast = getBeastOnPlayer(attacker);
			
			if (!isIdValid(beast) || !exists(beast))
			{
				return 0;
			}
			
			float cun_mod = getBeastCunningRating(beast);
			float block_value = (float)getBeastBlockValue(beast);
			
			cun_mod = (cun_mod / 100.0f);
			
			return Math.round(block_value * cun_mod);
		}
		
		return (int)utils.getFloatScriptVar(attacker, OBJVAR_INCREASE_BLOCK_VALUE);
	}
	
	
	public static boolean setBeastDodgeChance(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast) || !exists(beast))
		{
			return false;
		}
		
		if (!isBeast(beast))
		{
			return false;
		}
		
		float svl_mod = getBeastCustomMod(beast, MOD_SURVIVAL);
		
		float dodgeChance = svl_mod * 0.65f;
		
		dodgeChance += (utils.getFloatScriptVar(beast, OBJVAR_INCREASE_DODGE) / 10.0f);
		
		utils.setScriptVar(beast, OBJVAR_INCREASE_DODGE, dodgeChance);
		return true;
	}
	
	
	public static float getBeastDodgeChance(obj_id attacker) throws InterruptedException
	{
		if (isPlayer(attacker))
		{
			obj_id beast = getBeastOnPlayer(attacker);
			
			if (!isIdValid(beast) || !exists(beast))
			{
				return 0.0f;
			}
			
			float cun_mod = getBeastCunningRating(beast);
			float dodge_mod = getBeastDodgeChance(beast);
			
			cun_mod = (cun_mod / 100.0f);
			
			return dodge_mod * cun_mod;
		}
		
		return utils.getFloatScriptVar(attacker, OBJVAR_INCREASE_DODGE);
	}
	
	
	public static boolean setBeastParryChance(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast) || !exists(beast))
		{
			return false;
		}
		
		if (!isBeast(beast))
		{
			return false;
		}
		
		float svl_mod = getBeastCustomMod(beast, MOD_SURVIVAL);
		
		float parryChance = svl_mod * 0.65f;
		
		parryChance += (utils.getFloatScriptVar(beast, OBJVAR_INCREASE_PARRY) / 10.0f);
		
		utils.setScriptVar(beast, OBJVAR_INCREASE_PARRY, parryChance);
		return true;
	}
	
	
	public static float getBeastParryChance(obj_id attacker) throws InterruptedException
	{
		if (isPlayer(attacker))
		{
			obj_id beast = getBeastOnPlayer(attacker);
			
			if (!isIdValid(beast) || !exists(beast))
			{
				return 0.0f;
			}
			
			float cun_mod = getBeastCunningRating(beast);
			float parry_mod = getBeastParryChance(beast);
			
			cun_mod = (cun_mod / 100.0f);
			
			return parry_mod * cun_mod;
		}
		
		return utils.getFloatScriptVar(attacker, OBJVAR_INCREASE_PARRY);
	}
	
	
	public static boolean setBeastEvadeChance(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast) || !exists(beast))
		{
			return false;
		}
		
		if (!isBeast(beast))
		{
			return false;
		}
		
		float svl_mod = getBeastCustomMod(beast, MOD_SURVIVAL);
		float evadeChance = svl_mod * 0.65f;
		evadeChance += (utils.getFloatScriptVar(beast, OBJVAR_INCREASE_EVASION) / 10.0f);
		utils.setScriptVar(beast, OBJVAR_INCREASE_EVASION, evadeChance);
		return true;
	}
	
	
	public static float getBeastEvadeChance(obj_id attacker) throws InterruptedException
	{
		if (isPlayer(attacker))
		{
			obj_id beast = getBeastOnPlayer(attacker);
			
			if (!isIdValid(beast) || !exists(beast))
			{
				return 0.0f;
			}
			
			float cun_mod = getBeastCunningRating(beast);
			float evade_mod = getBeastEvadeChance(beast);
			
			cun_mod = (cun_mod / 100.0f);
			
			return evade_mod * cun_mod;
		}
		
		return utils.getFloatScriptVar(attacker, OBJVAR_INCREASE_EVASION);
	}
	
	
	public static boolean setBeastEvadeValue(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast) || !exists(beast))
		{
			return false;
		}
		
		if (!isBeast(beast))
		{
			return false;
		}
		
		float int_mod = getBeastCustomMod(beast, MOD_INTELLIGENCE);
		
		float evadeValue = int_mod * 2.0f;
		
		evadeValue += utils.getFloatScriptVar(beast, OBJVAR_INCREASE_EVASION_RATING);
		
		utils.setScriptVar(beast, OBJVAR_INCREASE_EVASION_RATING, evadeValue);
		return true;
	}
	
	
	public static float getBeastEvadeValue(obj_id attacker) throws InterruptedException
	{
		if (isPlayer(attacker))
		{
			obj_id beast = getBeastOnPlayer(attacker);
			
			if (!isIdValid(beast) || !exists(beast))
			{
				return 0.0f;
			}
			
			float cun_mod = getBeastCunningRating(beast);
			float evade_mod = getBeastEvadeValue(beast);
			
			cun_mod = (cun_mod / 100.0f);
			
			return evade_mod * cun_mod;
		}
		
		return utils.getFloatScriptVar(attacker, OBJVAR_INCREASE_EVASION_RATING);
	}
	
	
	public static float getBeastLearnBonus(obj_id beast) throws InterruptedException
	{
		if (isPlayer(beast))
		{
			beast = getBeastOnPlayer(beast);
		}
		
		if (!isIdValid(beast) || !exists(beast))
		{
			return 0.0f;
		}
		
		float int_mod = getBeastCustomMod(beast, MOD_INTELLIGENCE);
		
		return (int_mod * 1.5f);
	}
	
	
	public static float getBeastCunningRating(obj_id beast) throws InterruptedException
	{
		if (!isIdValid(beast) || !exists(beast))
		{
			return 0.0f;
		}
		
		float cun_mod = getBeastCustomMod(beast, MOD_CUNNING);
		return (cun_mod * 3.0f);
	}
	
	
	public static float getBeastModDamageBonus(obj_id attacker) throws InterruptedException
	{
		if (isPlayer(attacker))
		{
			obj_id beast = getBeastOnPlayer(attacker);
			
			if (!isIdValid(beast) || !exists(beast))
			{
				return 0.0f;
			}
			
			float cun_mod = getBeastCunningRating(beast);
			float dam_mod = getBeastModDamageBonus(beast);
			
			cun_mod = (cun_mod / 100.0f);
			
			return dam_mod * cun_mod;
		}
		
		float agg_mod = getBeastCustomMod(attacker, MOD_AGGRESSION);
		return (float)agg_mod * 2.0f;
	}
	
	
	public static float getBeastModGlancingReduction(obj_id attacker) throws InterruptedException
	{
		if (isPlayer(attacker))
		{
			obj_id beast = getBeastOnPlayer(attacker);
			
			if (!isIdValid(beast) || !exists(beast))
			{
				return 0.0f;
			}
			
			float cun_mod = getBeastCunningRating(beast);
			float dam_mod = getBeastModGlancingReduction(beast);
			
			cun_mod = (cun_mod / 100.0f);
			
			return dam_mod * cun_mod;
		}
		
		float agg_mod = getBeastCustomMod(attacker, MOD_AGGRESSION);
		return agg_mod * 1.0f;
	}
	
	
	public static float getBeastModDamageReduction(obj_id defender) throws InterruptedException
	{
		if (isPlayer(defender))
		{
			obj_id beast = getBeastOnPlayer(defender);
			
			if (!isIdValid(beast) || !exists(beast))
			{
				return 0.0f;
			}
			
			float cun_mod = getBeastCunningRating(beast);
			float dam_mod = getBeastModDamageReduction(beast);
			
			cun_mod = (cun_mod / 100.0f);
			
			return dam_mod * cun_mod;
		}
		
		float res_mod = getBeastCustomMod(defender, MOD_RESILIENCE);
		return res_mod * 1.25f;
	}
	
	
	public static float getBeastModPunishingReduction(obj_id defender) throws InterruptedException
	{
		if (isPlayer(defender))
		{
			obj_id beast = getBeastOnPlayer(defender);
			
			if (!isIdValid(beast) || !exists(beast))
			{
				return 0.0f;
			}
			
			float cun_mod = getBeastCunningRating(beast);
			float dam_mod = getBeastModPunishingReduction(beast);
			
			cun_mod = (cun_mod / 100.0f);
			
			return dam_mod * cun_mod;
		}
		
		float res_mod = getBeastCustomMod(defender, MOD_RESILIENCE);
		
		return res_mod * 1.0f;
	}
	
	
	public static float getBeastCustomMod(obj_id beast, int type) throws InterruptedException
	{
		if (isPlayer(beast))
		{
			beast = getBeastOnPlayer(beast);
		}
		
		if (!isIdValid(beast) || !exists(beast) || !isBeast(beast))
		{
			return 0;
		}
		
		switch(type)
		{
			case MOD_HUNTERS_INSTINCT:
			return utils.getFloatScriptVar(beast, OBJVAR_SKILL_HUNTERS_INSTINCT);
			
			case MOD_AGGRESSION:
			return utils.getFloatScriptVar(beast, OBJVAR_SKILL_AGGRESSION);
			
			case MOD_INTELLIGENCE:
			return utils.getFloatScriptVar(beast, OBJVAR_SKILL_INTELLIGENCE);
			
			case MOD_CUNNING:
			return utils.getFloatScriptVar(beast, OBJVAR_SKILL_CUNNING);
			
			case MOD_RESILIENCE:
			return utils.getFloatScriptVar(beast, OBJVAR_SKILL_BEASTIAL_RESILIENCE);
			
			case MOD_SURVIVAL:
			return utils.getFloatScriptVar(beast, OBJVAR_SKILL_SURVIVAL);
		}
		
		return 0;
	}
	
	
	public static boolean setToggledCommands(obj_id player, obj_id bcd) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			
			return false;
		}
		
		if (!isIdValid(bcd))
		{
			
			return false;
		}
		String[] toggledAbilities =
		{
			"","","","",""
		};
		String[] toggledAutoRepeats = getAutoRepeatAbilityList(bcd);
		
		for (int i = 0; i < toggledAutoRepeats.length; ++i)
		{
			testAbortScript();
			
			toggledAbilities[i] = toggledAutoRepeats[i];
			
		}
		
		if (getBCDBeastDefensive(bcd))
		{
			toggledAbilities[4] = "toggleBeastDefensive";
		}
		else
		{
			toggledAbilities[4] = "toggleBeastPassive";
		}
		
		setBeastmasterToggledPetCommands(player, toggledAbilities);
		return true;
	}
	
	
	public static String getBeastTrait(obj_id player, obj_id target) throws InterruptedException
	{
		return "none";
	}
	
	
	public static boolean canBeastGetLevelBasedXP(obj_id beast, obj_id npc) throws InterruptedException
	{
		int beastLevel = getBeastLevel(beast);
		int mobLevel = getLevel(npc);
		int levelDiff = mobLevel - beastLevel;
		
		if (levelDiff > 0)
		{
			beastLevel += levelDiff;
		}
		
		int exp = xp.getLevelBasedXP(beastLevel);
		
		if (levelDiff < 0)
		{
			
			float maxLevelDiff = 10f;
			
			if (beastLevel > 20)
			{
				maxLevelDiff += (int) ((beastLevel - 20) / 6);
			}
			
			exp += (int) (exp * (levelDiff / maxLevelDiff));
			
			if (exp < 1)
			{
				exp = 1;
			}
		}
		
		if (exp > 1)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static int useBeastInjector(obj_id player, obj_id injector, obj_id beast, String beastFamily, String[] injectorFamilies, int mark) throws InterruptedException
	{
		if (!exists(player) || !exists(injector) || !exists(beast))
		{
			return INJECTOR_RETURN_BAD_DATA;
		}
		
		if (!isIdValid(player) || !isIdValid(injector) || !isIdValid(beast))
		{
			return INJECTOR_RETURN_BAD_DATA;
		}
		
		if (beastFamily == null || beastFamily.equals(""))
		{
			return INJECTOR_RETURN_BAD_DATA;
		}
		
		if (injectorFamilies == null || injectorFamilies.length <= 0)
		{
			return INJECTOR_RETURN_BAD_DATA;
		}
		
		int level = getBeastLevel(beast);
		
		if (level > STEROID_BUFF_LEVEL_CAP && mark <= 1)
		{
			sendSystemMessage(player, SID_BEAST_LEVEL_TOO_HIGH);
			return INJECTOR_RETURN_WRONG_LEVEL;
		}
		
		if (level <= STEROID_BUFF_LEVEL_CAP && mark == 2)
		{
			sendSystemMessage(player, SID_BEAST_LEVEL_TOO_LOW);
			return INJECTOR_RETURN_WRONG_LEVEL;
		}
		
		if (buff.hasBuff(beast, "bm_beast_steroid"))
		{
			return INJECTOR_RETURN_HAS_BUFF;
		}
		
		if (injectorFamilies.length > 0)
		{
			for (int i = 0; i < injectorFamilies.length; ++i)
			{
				testAbortScript();
				
				if (beastFamily.equals(injectorFamilies[i]))
				{
					
					return INJECTOR_RETURN_SUCESS;
				}
			}
			
			return INJECTOR_RETURN_WRONG_FAMILY;
		}
		
		return INJECTOR_RETURN_BAD_DATA;
	}
	
	
	public static boolean convertBcdIntoBeastItem(obj_id player, obj_id bcd) throws InterruptedException
	{
		obj_id pInv = utils.getInventoryContainer(player);
		
		obj_id beastDisplayBase = createObjectOverloaded(BEAST_DISPLAY_TEMPLATE, pInv);
		
		if (!isIdValid(beastDisplayBase))
		{
			debugSpeakMsg(player, "Stuffed beast base is null.");
			return false;
		}
		
		setObjVar(beastDisplayBase, "no_dna_core", 1);
		
		String allObjVars = getPackedObjvars(bcd);
		
		setPackedObjvars(beastDisplayBase, allObjVars);
		
		setName(beastDisplayBase, getBCDBeastName(bcd));
		
		attachScript(beastDisplayBase, "systems.beast.beast_display");
		
		destroyObject(bcd);
		
		if (isIdValid(bcd))
		{
			destroyObject(beastDisplayBase);
		}
		
		return true;
	}
	
	
	public static obj_id createHolopetCubeFromEgg(obj_id player, obj_id egg) throws InterruptedException
	{
		int beastHashType = getIntObjVar(egg, beast_lib.OBJVAR_BEAST_TYPE);
		String beastType = incubator.convertHashTypeToString(beastHashType);
		if (beastType != null && beastType.length() > 0)
		{
			obj_id holepetCube = createBeastHolopet(player, egg, beastType);
			if (isIdValid(holepetCube))
			{
				if (rand(0,99) == 10)
				{
					setObjVar(holepetCube, HOLO_BEAST_RARE_COLOR_OBJVAR, HOLO_BEAST_TYPE3_QUALITY3);
				}
				return holepetCube;
			}
		}
		
		return obj_id.NULL_ID;
	}
	
	
	public static obj_id createBeastHolopet(obj_id player, obj_id source, String beastType) throws InterruptedException
	{
		obj_id playerInv = utils.getInventoryContainer(player);
		if (isIdValid(playerInv))
		{
			String cubeTemplate = "object/tangible/item/data_cube_holobeast.iff";
			obj_id holepetCube = createObjectOverloaded(cubeTemplate, playerInv);
			if (isIdValid(holepetCube))
			{
				
				setObjVar(holepetCube, "holopet_type", beastType);
				
				if (hasObjVar(source, beast_lib.OBJVAR_BEAST_ENGINEER))
				{
					String creatorName = getStringObjVar(source, beast_lib.OBJVAR_BEAST_ENGINEER);
					if (creatorName != null && creatorName.length() > 0)
					{
						setObjVar(holepetCube, beast_lib.OBJVAR_BEAST_ENGINEER, creatorName);
					}
				}
				
				String holoBeastName = localizedHoloBeastBaseName( beastType);
				
				setName(holepetCube, holoBeastName + "Holo-beast Data Cube");
				attachScript(holepetCube, "item.publish_gift.holopet_data_cube");
				return holepetCube;
			}
		}
		
		return obj_id.NULL_ID;
	}
	
	
	public static String localizedHoloBeastBaseName(String beastType) throws InterruptedException
	{
		if (beastType.startsWith("bm_"))
		{
			beastType = beast_lib.stripBmFromType(beastType);
		}
		
		String finalName = "";
		
		string_id beastTypeSid = new string_id("monster_name", beastType);
		String lowercaseName = localize(beastTypeSid);
		if (lowercaseName != null && lowercaseName.length() > 0)
		{
			String[] splitName = split(lowercaseName, ' ');
			for (int i = 0; i < splitName.length; i++)
			{
				testAbortScript();
				finalName += toUpper(splitName[i], 0) + " ";
			}
		}
		return finalName;
	}
}
