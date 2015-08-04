package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import java.util.Arrays;
import java.lang.System;
import java.lang.Math;

import java.util.Arrays;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Vector;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;

import script.script_entry;

import script.library.player_structure;
import script.library.healing;
import script.library.ai_lib;
import script.library.pclib;
import script.library.armor;
import script.library.chat;
import script.library.performance;
import script.library.trial;
import script.library.cloninglib;
import script.library.static_item;


public class utils extends script.base_script
{
	public utils()
	{
	}
	public static final String VERSION = "v0.01.00";
	
	public static final String VAR_OWNER = "owner";
	public static final String VAR_COOWNERS = "coowners";
	
	public static final String SLOT_INVENTORY = "inventory";
	public static final String SLOT_DATAPAD = "datapad";
	public static final String SLOT_HANGAR = "hangar";
	public static final String SLOT_BANK = "bank";
	public static final String SLOT_MISSION_BAG = "mission_bag";
	
	public static final String FREE_TRIAL = "free_trial";
	public static final String TIP = FREE_TRIAL+".tip";
	public static final String TIP_OUT_NUM = TIP + ".tip_out_num";
	public static final String TIP_OUT_AMMOUNT = TIP + ".tip_out_ammount";
	public static final String TIP_IN_NUM = TIP + ".tip_in_num";
	public static final String TIP_IN_AMMOUNT = TIP + ".tip_in_ammount";
	public static final String TIP_IN_THACK = TIP + ".tip_in_timeHack";
	public static final String TIP_OUT_THACK = TIP + ".tip_out_timeHack";
	
	public static final String TRIAL_STRUCTURE = FREE_TRIAL+".trial_structure";
	
	public static final int TIP_NUM_MAX = 25;
	public static final int TIP_AMT_MAX = 100000;
	
	public static final String NO_TRADE_SCRIPT = "item.special.nomove";
	
	public static final String CTS_OBJVAR_HISTORY = "ctsRetroHistory";
	
	public static final int BIT_LIST_SIZE = 32;
	
	public static final string_id SID_OBJECT_NOT_ACTIVE = new string_id("error_message", "object_not_active");
	
	public static final String[] WAYPOINT_COLORS =
	{
		waypoint_colors.blue,
		waypoint_colors.green,
		waypoint_colors.orange,
		waypoint_colors.yellow,
		waypoint_colors.purple,
		waypoint_colors.white
	};
	
	public static final string_id SID_SECONDS = new string_id("spam", "seconds");
	public static final string_id SID_MINUTES = new string_id("spam", "minutes");
	public static final string_id SID_HOURS = new string_id("spam", "hours");
	public static final string_id SID_DAYS = new string_id("spam", "days");
	public static final string_id SID_OUT_OF_RANGE = new string_id("spam", "out_of_range");
	
	public static final int COMMANDO = 1;
	public static final int SMUGGLER = 2;
	public static final int MEDIC = 3;
	public static final int OFFICER = 4;
	public static final int SPY = 5;
	public static final int BOUNTY_HUNTER = 6;
	public static final int FORCE_SENSITIVE = 7;
	public static final int TRADER = 8;
	public static final int ENTERTAINER = 9;
	
	public static final String LIFEDAY_OWNER = "lifeday.owner";
	public static final String XMAS_RECEIVED_V1 = "gift.xmas05";
	public static final String XMAS_RECEIVED_V2 = "gift.xmas05v2";
	
	public static final String XMAS_RECEIVED_VI1 = "gift.xmas06";
	public static final String XMAS_RECEIVED_VI2 = "gift.xmas06v2";
	
	public static final String XMAS_RECEIVED_VII1 = "gift.xmas07";
	public static final String XMAS_RECEIVED_VII2 = "gift.xmas07v2";
	public static final String XMAS_NOT_RECEIVED_TUTORIAL = "gift.xmas06_inTutorialFail";
	
	public static final String XMAS_RECEIVED_VIII1 = "gift.xmas08";
	public static final String XMAS_RECEIVED_VIII2 = "gift.xmas08v2";
	
	public static final String XMAS_RECEIVED_IX_01 = "gift.xmas09";
	
	public static final String EMPIRE_DAY_RECEIVED_VI = "gift.empire08";
	
	public static final string_id GIFT_GRANTED = new string_id("system_msg", "gift_granted");
	public static final string_id GIFT_GRANTED_SUB = new string_id("system_msg", "gift_granted_sub");
	
	public static final int HOUSE_CURRENT = 0;
	public static final int HOUSE_HISTORY = 1;
	public static final int HOUSE_MAX = 2;
	
	public static final obj_id OBJ_ID_BIO_LINK_PENDING = obj_id.getObjId(1);
	
	public static final String VENDOR_SCRIPT = "terminal.vendor";
	public static final String BAZAAR_SCRIPT = "terminal.bazaar";
	
	
	public static int clipRange(int iValue, int iClipMin, int iClipMax) throws InterruptedException
	{
		return (iValue < iClipMin) ? iClipMin : (iValue > iClipMax) ? iClipMax : iValue;
	}
	
	
	public static location getRandomAwayLocation(location pos, float fMinRadius, float fMaxRadius) throws InterruptedException
	{
		float fTheta = rand() * (2f * (float)Math.PI);
		float fRadius = Math.min(fMinRadius, fMaxRadius) + rand() * Math.abs(fMaxRadius - fMinRadius);
		
		pos.x += fRadius * Math.cos(fTheta);
		pos.z += fRadius * Math.sin(fTheta);
		return pos;
	}
	
	
	public static float getDistance2D(location locTarget1, location locTarget2) throws InterruptedException
	{
		if (locTarget1 == null || locTarget2 == null)
		{
			return -1.00f;
		}
		
		location loc1 = (location)locTarget1.clone();
		loc1.y = 0f;
		
		location loc2 = (location)locTarget2.clone();
		loc2.y = 0f;
		
		return getDistance(loc1, loc2);
	}
	
	
	public static float getDistance2D(obj_id hereTarget, obj_id thereTarget) throws InterruptedException
	{
		if (!isIdValid(hereTarget) || !isIdValid(thereTarget))
		{
			return -1.00f;
		}
		
		return getDistance2D(getWorldLocation(hereTarget), getWorldLocation(thereTarget));
	}
	
	
	public static attrib_mod[] addMindAttribToStim(int power) throws InterruptedException
	{
		attrib_mod[] am = new attrib_mod[2];
		
		for (int i = 0; i < 2; i++)
		{
			testAbortScript();
			am[i] = createHealDamageAttribMod(i * 2, power);
		}
		return am;
	}
	
	
	public static int randix(float[] fArray) throws InterruptedException
	{
		
		if (fArray.length > 1)
		{
			
			float fSum = 0.0f;
			for (int i = 0; i < fArray.length; i++)
			{
				testAbortScript();
				fSum += fArray[i];
			}
			
			float fRandom = rand() * fSum;
			
			fSum = 0.0f;
			for (int i = 0; i < fArray.length; i++)
			{
				testAbortScript();
				fSum += fArray[i];
				if (fRandom <= fSum)
				{
					return i;
				}
			}
		}
		
		return 0;
	}
	
	
	public static boolean isMando(obj_id armor) throws InterruptedException
	{
		
		if (isIdValid(armor))
		{
			String template = getTemplateName(armor);
			
			if ((template.endsWith("armor_mandalorian_belt.iff") || template.endsWith("armor_mandalorian_bicep_l.iff")|| template.endsWith("armor_mandalorian_bicep_r.iff")|| template.endsWith("armor_mandalorian_bracer_l.iff")|| template.endsWith("armor_mandalorian_bracer_r.iff")|| template.endsWith("armor_mandalorian_chest_plate.iff")|| template.endsWith("armor_mandalorian_helmet.iff")|| template.endsWith("armor_mandalorian_leggings.iff")|| template.endsWith("armor_mandalorian_shoes.iff")|| template.endsWith("armor_mandalorian_gloves.iff") ))
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public static boolean hasSpecialSkills(obj_id player) throws InterruptedException
	{
		boolean skillCheck = false;
		
		if (hasSkill (player, "class_commando_phase4_master"))
		{
			skillCheck = true;
		}
		
		if (hasSkill (player, "class_bountyhunter_phase4_master"))
		{
			skillCheck = true;
		}
		
		return skillCheck;
	}
	
	
	public static int unequipAndNotifyUncerted(obj_id player) throws InterruptedException
	{
		int totalUnequipped = 0;
		String[] armorSlots = new String[]
		{
			"hat",
			"chest2",
			"bicep_r",
			"bicep_l",
			"bracer_upper_r",
			"bracer_lower_r",
			"bracer_upper_l",
			"bracer_lower_l",
			"gloves",
			"pants2",
			"shoes",
			"chest1",
			"pants1",
			"utility_belt"
		};
		
		obj_id curArmor = null;
		obj_id inv = utils.getInventoryContainer(player);
		String classTemplate = getSkillTemplate(player);
		
		for (int i = 0; i < armorSlots.length; i++)
		{
			testAbortScript();
			curArmor = getObjectInSlot(player, armorSlots[i]);
			if (( (isIdValid(curArmor) && !armor.isArmorCertified(player, curArmor)) ||(isMando(curArmor) && !hasSpecialSkills(player)) ))
			{
				totalUnequipped++;
				prose_package pp = new prose_package();
				pp = prose.setStringId(pp, new string_id("spam", "armor_lost_cert"));
				pp = prose.setTT(pp, curArmor);
				sendSystemMessageProse(player, pp);
				putInOverloaded(curArmor, inv);
			}
			
			if (!isIdNull(curArmor) && hasObjVar(curArmor, "armor.fake_armor") && hasObjVar(curArmor, "dynamic_item.required_skill"))
			{
				String requiredSkill = getStringObjVar(curArmor, "dynamic_item.required_skill");
				if (classTemplate != null && !classTemplate.equals(""))
				{
					if (!classTemplate.startsWith(requiredSkill))
					{
						totalUnequipped++;
						prose_package pp = new prose_package();
						pp = prose.setStringId(pp, new string_id("spam", "armor_lost_cert"));
						pp = prose.setTT(pp, curArmor);
						sendSystemMessageProse(player, pp);
						putInOverloaded(curArmor, inv);
					}
				}
			}
			
		}
		
		obj_id[] equipSlots = metrics.getWornItems(player);
		if (equipSlots != null && equipSlots.length > 0)
		{
			for (int i = 0; i < equipSlots.length; i++)
			{
				testAbortScript();
				
				if (!static_item.validateLevelRequired(player,(equipSlots[i])))
				{
					totalUnequipped++;
					prose_package pp = new prose_package();
					pp = prose.setStringId(pp, new string_id("spam", "item_lost_cert"));
					pp = prose.setTT(pp, equipSlots[i]);
					sendSystemMessageProse(player, pp);
					putInOverloaded(equipSlots[i], inv);
				}
				
				else if (!static_item.validateLevelRequiredForWornEffect(player,(equipSlots[i])))
				{
					static_item.removeWornBuffs(equipSlots[i], player);
					prose_package pp = new prose_package();
					pp = prose.setStringId(pp, new string_id("spam", "item_lost_cert_effect"));
					pp = prose.setTT(pp, equipSlots[i]);
					sendSystemMessageProse(player, pp);
				}
			}
		}
		
		obj_id weapon = getCurrentWeapon(player);
		if (isIdValid(weapon) && !combat.hasCertification(player, weapon))
		{
			totalUnequipped++;
			prose_package pp = new prose_package();
			pp = prose.setStringId(pp, new string_id("spam", "weapon_lost_cert"));
			pp = prose.setTT(pp, weapon);
			sendSystemMessageProse(player, pp);
			putInOverloaded(weapon, inv);
		}
		
		obj_id hold_l = getObjectInSlot(player, "hold_l");
		obj_id hold_r = getObjectInSlot(player, "hold_r");
		
		if (isIdValid(hold_l) && !performance.isDancePropCertified(player, hold_l))
		{
			totalUnequipped++;
			prose_package pp = new prose_package();
			pp = prose.setStringId(pp, new string_id("spam", "prop_lost_cert"));
			pp = prose.setTT(pp, hold_l);
			sendSystemMessageProse(player, pp);
			putInOverloaded(hold_l, inv);
			hold_l = obj_id.NULL_ID;
		}
		
		if (isIdValid(hold_r) && !performance.isDancePropCertified(player, hold_r))
		{
			totalUnequipped++;
			prose_package pp = new prose_package();
			pp = prose.setStringId(pp, new string_id("spam", "prop_lost_cert"));
			pp = prose.setTT(pp, hold_r);
			sendSystemMessageProse(player, pp);
			putInOverloaded(hold_r, inv);
			hold_r = obj_id.NULL_ID;
		}
		
		if (performance.isValidDanceProp(hold_l) && performance.isValidDanceProp(hold_r) && !hasCommand(player, "prop_dual_wield"))
		{
			totalUnequipped++;
			prose_package pp = new prose_package();
			pp = prose.setStringId(pp, new string_id("spam", "dual_prop_lost_cert"));
			pp = prose.setTT(pp, hold_l);
			sendSystemMessageProse(player, pp);
			putInOverloaded(hold_l, inv);
			hold_l = obj_id.NULL_ID;
		}
		
		obj_id[] appearanceItems = getAllItemsFromAppearanceInventory(player);
		if (appearanceItems != null && appearanceItems.length > 0)
		{
			for (int i = 0; i < appearanceItems.length; ++i)
			{
				testAbortScript();
				curArmor = appearanceItems[i];
				
				if (( (isIdValid(curArmor) && !armor.isArmorCertified(player, curArmor)) ||(isMando(curArmor) && !hasSpecialSkills(player)) ))
				{
					totalUnequipped++;
					prose_package pp = new prose_package();
					pp = prose.setStringId(pp, new string_id("spam", "armor_lost_cert"));
					pp = prose.setTT(pp, curArmor);
					sendSystemMessageProse(player, pp);
					putInOverloaded(curArmor, inv);
				}
				if (!isIdNull(curArmor) && hasObjVar(curArmor, "armor.fake_armor") && hasObjVar(curArmor, "dynamic_item.required_skill"))
				{
					String requiredSkill = getStringObjVar(curArmor, "dynamic_item.required_skill");
					if (classTemplate != null && !classTemplate.equals(""))
					{
						if (!classTemplate.startsWith(requiredSkill))
						{
							totalUnequipped++;
							prose_package pp = new prose_package();
							pp = prose.setStringId(pp, new string_id("spam", "armor_lost_cert"));
							pp = prose.setTT(pp, curArmor);
							sendSystemMessageProse(player, pp);
							putInOverloaded(curArmor, inv);
						}
					}
				}
				
				if (!static_item.validateLevelRequired(player,curArmor))
				{
					totalUnequipped++;
					prose_package pp = new prose_package();
					pp = prose.setStringId(pp, new string_id("spam", "item_lost_cert"));
					pp = prose.setTT(pp, curArmor);
					sendSystemMessageProse(player, pp);
					putInOverloaded(curArmor, inv);
				}
				
				else if (!static_item.validateLevelRequiredForWornEffect(player,curArmor))
				{
					static_item.removeWornBuffs(curArmor, player);
					prose_package pp = new prose_package();
					pp = prose.setStringId(pp, new string_id("spam", "item_lost_cert_effect"));
					pp = prose.setTT(pp, curArmor);
					sendSystemMessageProse(player, pp);
				}
				
			}
		}
		
		return totalUnequipped;
	}
	
	
	public static int getIntObjVar(obj_id object, String name, int intDefault) throws InterruptedException
	{
		if (!hasObjVar( object, name ))
		{
			return intDefault;
		}
		return getIntObjVar( object, name );
	}
	
	
	public static int[] getIntArrayObjVar(obj_id object, String name, int[] intArrayDefault) throws InterruptedException
	{
		if (!hasObjVar( object, name ))
		{
			return intArrayDefault;
		}
		return getIntArrayObjVar( object, name );
	}
	
	
	public static float getFloatObjVar(obj_id object, String name, float fltDefault) throws InterruptedException
	{
		if (!hasObjVar( object, name ))
		{
			return fltDefault;
		}
		return getFloatObjVar( object, name );
	}
	
	
	public static float[] getFloatArrayObjVar(obj_id object, String name, float[] fltArrayDefault) throws InterruptedException
	{
		if (!hasObjVar( object, name ))
		{
			return fltArrayDefault;
		}
		return getFloatArrayObjVar( object, name );
	}
	
	
	public static String getStringObjVar(obj_id object, String name, String strDefault) throws InterruptedException
	{
		if (!hasObjVar( object, name ))
		{
			return strDefault;
		}
		return getStringObjVar( object, name );
	}
	
	
	public static String[] getStringArrayObjVar(obj_id object, String name, String[] strArrayDefault) throws InterruptedException
	{
		if (!hasObjVar( object, name ))
		{
			return strArrayDefault;
		}
		return getStringArrayObjVar( object, name );
	}
	
	
	public static obj_id getObjIdObjVar(obj_id object, String name, obj_id idDefault) throws InterruptedException
	{
		if (!hasObjVar( object, name ))
		{
			return idDefault;
		}
		return getObjIdObjVar( object, name );
	}
	
	
	public static obj_id[] getObjIdArrayObjVar(obj_id object, String name, obj_id[] idArrayDefault) throws InterruptedException
	{
		if (!hasObjVar( object, name ))
		{
			return idArrayDefault;
		}
		return getObjIdArrayObjVar( object, name );
	}
	
	
	public static location getLocationObjVar(obj_id object, String name, location locDefault) throws InterruptedException
	{
		if (!hasObjVar( object, name ))
		{
			return locDefault;
		}
		return getLocationObjVar( object, name );
	}
	
	
	public static location[] getLocationArrayObjVar(obj_id object, String name, location[] locArrayDefault) throws InterruptedException
	{
		if (!hasObjVar( object, name ))
		{
			return locArrayDefault;
		}
		return getLocationArrayObjVar( object, name );
	}
	
	
	public static boolean getBooleanObjVar(obj_id object, String name, boolean bDefault) throws InterruptedException
	{
		if (!hasObjVar( object, name ))
		{
			return bDefault;
		}
		
		return getBooleanObjVar( object, name );
	}
	
	
	public static boolean isObjIdInArray(obj_id[] objIdArray, obj_id objTarget) throws InterruptedException
	{
		if (objIdArray == null || objIdArray.length == 0)
		{
			return false;
		}
		
		for (int i = 0; i < objIdArray.length; i++)
		{
			testAbortScript();
			if (objIdArray[i ] == objTarget)
			{
				return true;
			}
			
		}
		return false;
	}
	
	
	public static boolean isElementInArray(Vector objIdArray, Object objTarget) throws InterruptedException
	{
		if (objIdArray == null || objIdArray.isEmpty())
		{
			return false;
		}
		
		return objIdArray.contains(objTarget);
	}
	
	
	public static int getElementPositionInArray(Vector array, Object element) throws InterruptedException
	{
		if (array == null)
		{
			return -1;
		}
		
		return array.indexOf(element);
	}
	
	
	public static int getElementPositionInArray(obj_id[] array, obj_id element) throws InterruptedException
	{
		if (array == null || array.length == 0)
		{
			return -1;
		}
		
		for (int i = 0; i < array.length; i++)
		{
			testAbortScript();
			if (array[i ] == element)
			{
				return i;
			}
			
		}
		return -1;
	}
	
	
	public static int getElementPositionInArray(int[] array, int element) throws InterruptedException
	{
		if (array == null || array.length == 0)
		{
			return -1;
		}
		
		for (int i = 0; i < array.length; i++)
		{
			testAbortScript();
			if (array[i ] == element)
			{
				return i;
			}
			
		}
		return -1;
	}
	
	
	public static int getElementPositionInArray(float[] array, float element) throws InterruptedException
	{
		if (array == null || array.length == 0)
		{
			return -1;
		}
		
		for (int i = 0; i < array.length; i++)
		{
			testAbortScript();
			if (array[i ] == element)
			{
				return i;
			}
			
		}
		return -1;
	}
	
	
	public static int getElementPositionInArray(boolean[] array, boolean element) throws InterruptedException
	{
		if (array == null || array.length == 0)
		{
			return -1;
		}
		
		for (int i = 0; i < array.length; i++)
		{
			testAbortScript();
			if (array[i ] == element)
			{
				return i;
			}
			
		}
		return -1;
	}
	
	
	public static int getElementPositionInArray(String[] array, String element) throws InterruptedException
	{
		if (array == null || array.length == 0)
		{
			return -1;
		}
		
		for (int i = 0; i < array.length; i++)
		{
			testAbortScript();
			if (array[i ].equals(element))
			{
				return i;
			}
			
		}
		return -1;
	}
	
	
	public static int getElementPositionInArray(region[] array, region element) throws InterruptedException
	{
		if (array == null || array.length == 0)
		{
			return -1;
		}
		
		for (int i = 0; i < array.length; i++)
		{
			testAbortScript();
			if (array[i ].equals(element))
			{
				return i;
			}
			
		}
		return -1;
	}
	
	
	public static int getElementPositionInArray(location[] array, location element) throws InterruptedException
	{
		if (array == null || array.length == 0)
		{
			return -1;
		}
		
		for (int i = 0; i < array.length; i++)
		{
			testAbortScript();
			if (array[i ].equals(element))
			{
				return i;
			}
			
		}
		return -1;
	}
	
	
	public static int getElementPositionInArray(string_id[] array, string_id element) throws InterruptedException
	{
		if (array == null || array.length == 0)
		{
			return -1;
		}
		
		for (int i = 0; i < array.length; i++)
		{
			testAbortScript();
			if (array[i ].equals(element))
			{
				return i;
			}
			
		}
		return -1;
	}
	
	
	public static int getElementPositionInArray(attrib_mod[] array, attrib_mod element) throws InterruptedException
	{
		if (array == null || array.length == 0)
		{
			return -1;
		}
		
		for (int i = 0; i < array.length; i++)
		{
			testAbortScript();
			if (array[i ].equals(element))
			{
				return i;
			}
			
		}
		return -1;
	}
	
	
	public static obj_id[] copyObjIdArray(obj_id[] objSourceArray, obj_id[] objDestinationArray) throws InterruptedException
	{
		
		if ((objSourceArray == null || objDestinationArray == null || objSourceArray.length == 0 || objDestinationArray.length == 0 || objSourceArray.length > objDestinationArray.length))
		{
			return null;
		}
		
		for (int i = 0; i < objSourceArray.length; i++)
		{
			testAbortScript();
			objDestinationArray[i ] = objSourceArray[i ];
		}
		
		return objDestinationArray;
	}
	
	
	public static obj_id[] copyArray(obj_id[] oldArray, obj_id[] newArray) throws InterruptedException
	{
		if ((oldArray == null || newArray == null || oldArray.length == 0 || newArray.length == 0 || oldArray.length > newArray.length))
		{
			return null;
		}
		
		for (int i = 0; i < oldArray.length; i++)
		{
			testAbortScript();
			newArray[i ] = oldArray[i ];
		}
		
		return newArray;
	}
	
	
	public static int[] copyArray(int[] oldArray, int[] newArray) throws InterruptedException
	{
		if ((oldArray == null || newArray == null || oldArray.length == 0 || newArray.length == 0 || oldArray.length > newArray.length))
		{
			return null;
		}
		
		for (int i = 0; i < oldArray.length; i++)
		{
			testAbortScript();
			newArray[i ] = oldArray[i ];
		}
		
		return newArray;
	}
	
	
	public static float[] copyArray(float[] oldArray, float[] newArray) throws InterruptedException
	{
		if ((oldArray == null || newArray == null || oldArray.length == 0 || newArray.length == 0 || oldArray.length > newArray.length))
		{
			return null;
		}
		
		for (int i = 0; i < oldArray.length; i++)
		{
			testAbortScript();
			newArray[i ] = oldArray[i ];
		}
		
		return newArray;
	}
	
	
	public static boolean[] copyArray(boolean[] oldArray, boolean[] newArray) throws InterruptedException
	{
		if ((oldArray == null || newArray == null || oldArray.length == 0 || newArray.length == 0 || oldArray.length > newArray.length))
		{
			return null;
		}
		
		for (int i = 0; i < oldArray.length; i++)
		{
			testAbortScript();
			newArray[i ] = oldArray[i ];
		}
		
		return newArray;
	}
	
	
	public static String[] copyArray(String[] oldArray, String[] newArray) throws InterruptedException
	{
		if ((oldArray == null || newArray == null || oldArray.length == 0 || newArray.length == 0 || oldArray.length > newArray.length))
		{
			return null;
		}
		
		for (int i = 0; i < oldArray.length; i++)
		{
			testAbortScript();
			newArray[i ] = oldArray[i ];
		}
		
		return newArray;
	}
	
	
	public static region[] copyArray(region[] oldArray, region[] newArray) throws InterruptedException
	{
		if ((oldArray == null || newArray == null || oldArray.length == 0 || newArray.length == 0 || oldArray.length > newArray.length))
		{
			return null;
		}
		
		for (int i = 0; i < oldArray.length; i++)
		{
			testAbortScript();
			newArray[i ] = oldArray[i ];
		}
		
		return newArray;
	}
	
	
	public static location[] copyArray(location[] oldArray, location[] newArray) throws InterruptedException
	{
		if ((oldArray == null || newArray == null || oldArray.length == 0 || newArray.length == 0 || oldArray.length > newArray.length))
		{
			return null;
		}
		
		for (int i = 0; i < oldArray.length; i++)
		{
			testAbortScript();
			newArray[i ] = oldArray[i ];
		}
		
		return newArray;
	}
	
	
	public static string_id[] copyArray(string_id[] oldArray, string_id[] newArray) throws InterruptedException
	{
		if ((oldArray == null || newArray == null || oldArray.length == 0 || newArray.length == 0 || oldArray.length > newArray.length))
		{
			return null;
		}
		
		for (int i = 0; i < oldArray.length; i++)
		{
			testAbortScript();
			newArray[i ] = oldArray[i ];
		}
		
		return newArray;
	}
	
	
	public static attrib_mod[] copyArray(attrib_mod[] oldArray, attrib_mod[] newArray) throws InterruptedException
	{
		if ((oldArray == null || newArray == null || oldArray.length == 0 || newArray.length == 0 || oldArray.length > newArray.length))
		{
			return null;
		}
		
		for (int i = 0; i < oldArray.length; i++)
		{
			testAbortScript();
			newArray[i ] = oldArray[i ];
		}
		
		return newArray;
	}
	
	
	public static String[] copyArrayOfRange(String[] oldArray, String[] newArray, int startIndex, int stopIndex) throws InterruptedException
	{
		if ((oldArray == null || newArray == null || oldArray.length == 0 || newArray.length == 0 || oldArray.length < stopIndex+1))
		{
			return null;
		}
		
		int j = 0;
		for (int i = startIndex; i <= stopIndex; i++)
		{
			testAbortScript();
			newArray[j ] = oldArray[i ];
			j++;
		}
		
		return newArray;
	}
	
	
	public static int setBit(int intBits, int intPos) throws InterruptedException
	{
		if (intPos < 0 || intPos > 31)
		{
			
			return 0;
		}
		int posVal = 1 << intPos;
		return ( intBits |= posVal );
	}
	
	
	public static int clearBit(int intBits, int intPos) throws InterruptedException
	{
		if (intPos < 0 || intPos > 31)
		{
			
			return ~0;
		}
		int posVal = 1 << intPos;
		return ( intBits &= ~posVal );
	}
	
	
	public static boolean checkBit(int intBits, int intPos) throws InterruptedException
	{
		if (intPos < 0 || intPos > 31)
		{
			
			return false;
		}
		int posVal = 1 << intPos;
		return ((intBits & posVal) != 0 );
	}
	
	
	public static obj_id stringToObjId(String text) throws InterruptedException
	{
		
		Long lngId;
		
		try
		{
			lngId = new Long(text);
		}
		catch (NumberFormatException err)
		{
			
			return null;
		}
		obj_id objObject = obj_id.getObjId(lngId.longValue());
		return objObject;
	}
	
	
	public static obj_id[] stringToObjId(String[] text) throws InterruptedException
	{
		if ((text == null) || (text.length == 0))
		{
			return null;
		}
		
		obj_id[] ids = new obj_id[text.length];
		
		for (int i = 0; i < text.length; i++)
		{
			testAbortScript();
			ids[i] = stringToObjId(text[i]);
		}
		
		if (ids.length == text.length)
		{
			return ids;
		}
		
		return null;
	}
	
	
	public static int stringToInt(String text) throws InterruptedException
	{
		if (text == null || text.equals(""))
		{
			return -1;
		}
		
		int amt;
		
		try
		{
			amt = Integer.parseInt(text);
		}
		catch (NumberFormatException err)
		{
			
			return -1;
		}
		
		return amt;
	}
	
	
	public static long stringToLong(String text) throws InterruptedException
	{
		if (text == null || text.equals(""))
		{
			return -1;
		}
		
		long amt;
		
		try
		{
			amt = Long.parseLong(text);
		}
		catch (NumberFormatException err)
		{
			
			return -1;
		}
		
		return amt;
	}
	
	
	public static float stringToFloat(String text) throws InterruptedException
	{
		if (text == null || text.equals(""))
		{
			return Float.NEGATIVE_INFINITY;
		}
		
		float amt;
		
		try
		{
			amt = Float.parseFloat(text);
		}
		catch (NumberFormatException err)
		{
			
			return Float.NEGATIVE_INFINITY;
		}
		
		return amt;
	}
	
	
	public static String objIdArrayToString(obj_id[] array) throws InterruptedException
	{
		String text = "[ ";
		
		for (int i = 0; i < array.length; i++)
		{
			testAbortScript();
			text += ""+array[i];
			
			if (i < array.length-1)
			{
				text += ", ";
			}
		}
		
		text += " ]";
		
		return text;
	}
	
	
	public static Vector removeElementAt(Vector array, int index) throws InterruptedException
	{
		if (array == null)
		{
			return null;
		}
		
		if (index < 0 || index >= array.size())
		{
			return array;
		}
		
		array.removeElementAt(index);
		return array;
	}
	
	
	public static Vector removeElement(Vector array, obj_id element) throws InterruptedException
	{
		if (array == null)
		{
			return null;
		}
		
		if (element == null)
		{
			return array;
		}
		
		array.removeElement(element);
		return array;
	}
	
	
	public static Vector removeElements(Vector array, obj_id[] elements) throws InterruptedException
	{
		if (array == null)
		{
			return null;
		}
		
		if (elements == null || elements.length == 0)
		{
			return array;
		}
		
		List collection = Arrays.asList(elements);
		array.removeAll(collection);
		return array;
	}
	
	
	public static Vector removeElements(obj_id[] array, obj_id[] elements) throws InterruptedException
	{
		if (array == null)
		{
			return null;
		}
		
		Vector ret = new Vector(Arrays.asList(array));
		if (ret == null)
		{
			return null;
		}
		
		if (elements == null || elements.length == 0)
		{
			return ret;
		}
		
		List collection = Arrays.asList(elements);
		ret.removeAll(collection);
		return ret;
	}
	
	
	public static Vector removeElements(Vector array, Vector elements) throws InterruptedException
	{
		if (array == null)
		{
			return null;
		}
		
		if (elements == null || elements.size() == 0)
		{
			return array;
		}
		
		array.removeAll(elements);
		return array;
	}
	
	
	public static Vector addElement(Vector array, Object element) throws InterruptedException
	{
		if (array == null)
		{
			return addElement(new Vector(), element);
		}
		
		if (element == null)
		{
			return array;
		}
		
		array.add(element);
		return array;
	}
	
	
	public static Vector addElement(Vector array, boolean element) throws InterruptedException
	{
		return addElement( array, new Boolean(element) );
	}
	
	
	public static Vector addElement(Vector array, int element) throws InterruptedException
	{
		return addElement( array, new Integer(element) );
	}
	
	
	public static Vector addElement(Vector array, float element) throws InterruptedException
	{
		return addElement( array, new Float(element) );
	}
	
	
	public static obj_id[] toStaticObjIdArray(Vector vector) throws InterruptedException
	{
		if (vector == null || vector.size() == 0)
		{
			return null;
		}
		
		obj_id[] ret = new obj_id[vector.size()];
		vector.toArray(ret);
		return ret;
	}
	
	
	public static String[] toStaticStringArray(Vector vector) throws InterruptedException
	{
		if (vector == null || vector.size() == 0)
		{
			return null;
		}
		
		String[] ret = new String[vector.size()];
		vector.toArray(ret);
		return ret;
	}
	
	
	public static location[] toStaticLocationArray(Vector vector) throws InterruptedException
	{
		if (vector == null || vector.size() == 0)
		{
			return null;
		}
		
		location[] ret = new location[vector.size()];
		vector.toArray(ret);
		return ret;
	}
	
	
	public static boolean[] messageTo(obj_id[] targets, String messageName, dictionary params, float time, boolean guaranteed) throws InterruptedException
	{
		if (targets != null && targets.length > 0)
		{
			boolean[] results = new boolean[targets.length ];
			for (int i = 0; i < targets.length; i++)
			{
				testAbortScript();
				results[i] = messageTo( targets[i], messageName, params, time, guaranteed );
			}
			return results;
		}
		return null;
	}
	
	
	public static boolean isLocSufficientDistanceFromObjects(location loc, obj_id[] items, float dist) throws InterruptedException
	{
		if ((items == null) || (dist < 0))
		{
			return false;
		}
		
		for (int i = 0; i < items.length; i++)
		{
			testAbortScript();
			if (getDistance2D(loc, getLocation(items[i])) < dist)
			{
				return false;
			}
		}
		
		return true;
	}
	
	
	public static obj_id[] concatArrays(obj_id[] array1, obj_id[] array2) throws InterruptedException
	{
		if (array1 == null)
		{
			return array2;
		}
		
		else if (array2 == null)
		{
			return array1;
		}
		
		obj_id[] toPass = new obj_id[array1.length + array2.length];
		
		for (int i = 0; i < array1.length; i++)
		{
			testAbortScript();
			toPass[i] = array1[i];
		}
		
		for (int i = 0; i < array2.length; i++)
		{
			testAbortScript();
			toPass[i + array1.length] = array2[i];
		}
		
		return toPass;
	}
	
	
	public static Vector concatArrays(Vector array1, Vector array2) throws InterruptedException
	{
		if (array1 == null)
		{
			return array2;
		}
		else if (array2 == null)
		{
			return array1;
		}
		
		array1.addAll(array2);
		return array1;
	}
	
	
	public static Vector concatArrays(Vector array1, Object[] array2) throws InterruptedException
	{
		if (array2 == null)
		{
			return array1;
		}
		
		if (array1 == null)
		{
			array1 = new Vector(array2.length + 10);
		}
		
		array1.addAll(Arrays.asList(array2));
		return array1;
	}
	
	
	public static Vector concatArrays(Vector array1, int[] array2) throws InterruptedException
	{
		if (array2 == null)
		{
			return array1;
		}
		
		Object[] toPass = new Object[array2.length];
		for (int i = 0; i < array2.length; i++)
		{
			testAbortScript();
			toPass[i] = new Integer(array2[i]);
		}
		
		return concatArrays(array1, toPass);
	}
	
	
	public static Vector concatArrays(Vector array1, float[] array2) throws InterruptedException
	{
		if (array2 == null)
		{
			return array1;
		}
		
		Object[] toPass = new Object[array2.length];
		for (int i = 0; i < array2.length; i++)
		{
			testAbortScript();
			toPass[i] = new Float(array2[i]);
		}
		
		return concatArrays(array1, toPass);
	}
	
	
	public static Vector concatArrays(Vector array1, boolean[] array2) throws InterruptedException
	{
		if (array2 == null)
		{
			return array1;
		}
		
		Object[] toPass = new Object[array2.length];
		for (int i = 0; i < array2.length; i++)
		{
			testAbortScript();
			toPass[i] = new Boolean(array2[i]);
		}
		
		return concatArrays(array1, toPass);
	}
	
	
	public static String[] concatArrays(String[] array1, String[] array2) throws InterruptedException
	{
		if (array1 == null)
		{
			return array2;
		}
		
		if (array2 == null)
		{
			return array1;
		}
		
		String[] toPass = new String[array1.length + array2.length];
		for (int i = 0; i < array1.length; i++)
		{
			testAbortScript();
			toPass[i] = array1[i];
		}
		
		for (int i = 0; i < array2.length; i++)
		{
			testAbortScript();
			toPass[i+array1.length] = array2[i];
		}
		
		return toPass;
	}
	
	
	public static boolean isSubset(obj_id[] array1, obj_id[] array2) throws InterruptedException
	{
		if ((array1 == null) || (array2 == null))
		{
			return false;
		}
		
		Vector v1 = new Vector( Arrays.asList(array1) );
		Vector v2 = new Vector( Arrays.asList(array2) );
		
		return v1.containsAll(v2);
	}
	
	
	public static boolean isSubset(String[] array1, String[] array2) throws InterruptedException
	{
		if ((array1 == null) || (array2 == null))
		{
			return false;
		}
		
		Vector v1 = new Vector( Arrays.asList(array1) );
		Vector v2 = new Vector( Arrays.asList(array2) );
		
		return v1.containsAll(v2);
	}
	
	
	public static boolean isSubset(string_id[] array1, string_id[] array2) throws InterruptedException
	{
		if ((array1 == null) || (array2 == null))
		{
			return false;
		}
		
		Vector v1 = new Vector( Arrays.asList(array1) );
		Vector v2 = new Vector( Arrays.asList(array2) );
		
		return v1.containsAll(v2);
	}
	
	
	public static boolean isSubset(region[] array1, region[] array2) throws InterruptedException
	{
		if ((array1 == null) || (array2 == null))
		{
			return false;
		}
		
		Vector v1 = new Vector( Arrays.asList(array1) );
		Vector v2 = new Vector( Arrays.asList(array2) );
		
		return v1.containsAll(v2);
	}
	
	
	public static boolean isSubset(location[] array1, location[] array2) throws InterruptedException
	{
		if ((array1 == null) || (array2 == null))
		{
			return false;
		}
		
		Vector v1 = new Vector( Arrays.asList(array1) );
		Vector v2 = new Vector( Arrays.asList(array2) );
		
		return v1.containsAll(v2);
	}
	
	
	public static boolean isSubset(attrib_mod[] array1, attrib_mod[] array2) throws InterruptedException
	{
		if ((array1 == null) || (array2 == null))
		{
			return false;
		}
		
		Vector v1 = new Vector( Arrays.asList(array1) );
		Vector v2 = new Vector( Arrays.asList(array2) );
		
		return v1.containsAll(v2);
	}
	
	
	public static String[] makeNameListFromPlayerObjIdList(obj_id[] players) throws InterruptedException
	{
		if (players == null)
		{
			return null;
		}
		
		Vector nameList = new Vector();
		nameList.setSize(0);
		
		for (int i = 0; i < players.length; i++)
		{
			testAbortScript();
			if (isPlayer(players[i]))
			{
				nameList = addElement(nameList, getName(players[i]));
			}
		}
		
		if (nameList.size() == players.length)
		{
			return (String[])nameList.toArray(new String[0]);
		}
		else
		{
			return null;
		}
	}
	
	
	public static String[] makeNameList(obj_id[] targets) throws InterruptedException
	{
		if (targets == null)
		{
			return null;
		}
		
		Vector nameList = new Vector();
		nameList.setSize(0);
		
		obj_id self = getSelf();
		
		for (int i = 0; i < targets.length; i++)
		{
			testAbortScript();
			if (isIdValid(targets[i]))
			{
				String itemName = getEncodedName(targets[i]);
				if (itemName != null)
				{
					nameList = addElement(nameList, itemName);
				}
			}
		}
		
		if (nameList.size() == targets.length)
		{
			return (String[])nameList.toArray(new String[0]);
		}
		else
		{
			return null;
		}
	}
	
	
	public static String[] makeNameList(Vector targets) throws InterruptedException
	{
		if (targets == null || targets.isEmpty())
		{
			return null;
		}
		
		obj_id[] targetsArray = new obj_id[targets.size()];
		targetsArray = (obj_id[])targets.toArray(targetsArray);
		return makeNameList(targetsArray);
	}
	
	
	public static boolean isOwner(obj_id target, obj_id player) throws InterruptedException
	{
		if ((!isIdValid(target)) || (!isIdValid(player)))
		{
			return false;
		}
		
		if (getOwner(target) == player)
		{
			return true;
		}
		
		if (getObjIdObjVar(target, VAR_OWNER) == player)
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isCoOwner(obj_id target, obj_id player) throws InterruptedException
	{
		if ((target == null) || (!isIdValid(player)))
		{
			return false;
		}
		
		obj_id[] coowners = getObjIdArrayObjVar(target, VAR_COOWNERS);
		if (coowners == null)
		{
			return false;
		}
		
		if (getElementPositionInArray(coowners, player) > -1)
		{
			return true;
		}
		
		return false;
	}
	
	
	public static obj_id[] getContents(obj_id container, boolean recurse, Vector excludedNodes) throws InterruptedException
	{
		Vector contents = getResizeableContents(container, recurse, excludedNodes);
		obj_id[] _contents = new obj_id[0];
		if (contents != null)
		{
			_contents = new obj_id[contents.size()];
			contents.toArray(_contents);
		}
		return _contents;
	}
	
	
	public static obj_id[] getContents(obj_id container, boolean recurse) throws InterruptedException
	{
		Vector contents = getResizeableContents(container, recurse, null);
		obj_id[] _contents = new obj_id[0];
		if (contents != null)
		{
			_contents = new obj_id[contents.size()];
			contents.toArray(_contents);
		}
		return _contents;
	}
	
	
	public static Vector getResizeableContents(obj_id container, boolean recurse, Vector excludedNodes) throws InterruptedException
	{
		if ((!isIdValid(container)) || (getContainerType(container) == 0))
		{
			return null;
		}
		
		if (!recurse)
		{
			return getResizeableContents(container);
		}
		else
		{
			Vector contents = getResizeableContents(container);
			if (contents == null)
			{
				return null;
			}
			
			boolean exclude = true;
			if ((excludedNodes == null) || excludedNodes.isEmpty())
			{
				exclude = false;
			}
			
			for (int i = 0; i < contents.size(); i++)
			{
				testAbortScript();
				obj_id item = ((obj_id)(contents.get(i)));
				if (isIdValid(item))
				{
					
					boolean keepItem = true;
					if (exclude)
					{
						int idx = excludedNodes.indexOf(item);
						if (idx > -1)
						{
							contents = removeElementAt(contents, i);
							i--;
							keepItem = false;
						}
					}
					
					if (keepItem)
					{
						
						int itemGameObjectType = getGameObjectType(item);
						if (itemGameObjectType != GOT_misc_factory_crate && itemGameObjectType != GOT_chronicles_quest_holocron)
						{
							int containerType = getContainerType(item);
							if (containerType != 0)
							{
								obj_id[] newContents = getContents(item);
								if (newContents != null)
								{
									contents = concatArrays(contents, newContents);
								}
							}
						}
					}
				}
			}
			return contents;
		}
	}
	
	
	public static obj_id[] getSharedContainerObjects(obj_id object) throws InterruptedException
	{
		if (!isIdValid(object) || !exists(object))
		{
			return null;
		}
		
		obj_id container = getContainedBy(object);
		if (!isIdValid(container) || !exists(container))
		{
			return null;
		}
		
		return getContents(container);
	}
	
	
	public static Vector getResizeableContents(obj_id container, boolean recurse) throws InterruptedException
	{
		return getResizeableContents(container, recurse, null);
	}
	
	
	public static obj_id[] getNonBankPlayerContents(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return null;
		}
		
		obj_id bank = getPlayerBank(player);
		if (!isIdValid(bank))
		{
			return null;
		}
		
		Vector toExclude = addElement(null, bank);
		obj_id[] contents = getContents(player, true, toExclude);
		return contents;
	}
	
	
	public static obj_id[] getFilteredPlayerContents(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return null;
		}
		
		Vector toExclude = null;
		
		obj_id bank = getPlayerBank(player);
		if (isIdValid(bank))
		{
			toExclude = addElement(toExclude, bank);
		}
		
		obj_id missionBag = getMissionBag(player);
		if (isIdValid(missionBag))
		{
			toExclude = addElement(toExclude, missionBag);
		}
		
		obj_id datapad = getPlayerDatapad(player);
		if (isIdValid(datapad))
		{
			toExclude = addElement(toExclude, datapad);
		}
		
		return getContents(player, true, toExclude);
	}
	
	
	public static obj_id[] getAllContentsOwnedByPlayer(obj_id container, obj_id player) throws InterruptedException
	{
		if ((!isIdValid(container)) || (container == obj_id.NULL_ID) || (!isIdValid(player)))
		{
			return null;
		}
		
		Vector ownedObjects = new Vector();
		ownedObjects.setSize(0);
		obj_id[] allObjects = getContents(container, true);
		
		if (allObjects == null)
		{
			return null;
		}
		
		for (int i = 0; i < allObjects.length; i++)
		{
			testAbortScript();
			if (getObjIdObjVar(allObjects[i], VAR_OWNER) == player)
			{
				ownedObjects = addElement(ownedObjects, allObjects[i]);
			}
		}
		
		if (ownedObjects.size() > 0)
		{
			obj_id[] _ownedObjects = new obj_id[0];
			if (ownedObjects != null)
			{
				_ownedObjects = new obj_id[ownedObjects.size()];
				ownedObjects.toArray(_ownedObjects);
			}
			return _ownedObjects;
		}
		
		return null;
	}
	
	
	public static boolean isNestedWithin(obj_id item, obj_id container) throws InterruptedException
	{
		if (!isIdValid(item) || !isIdValid(container))
		{
			return false;
		}
		
		obj_id containedBy = getContainedBy(item);
		if (!isIdValid(containedBy))
		{
			return false;
		}
		else if (containedBy == container)
		{
			return true;
		}
		
		return isNestedWithin(containedBy, container);
	}
	
	
	public static boolean isNestedWithinAPlayer(obj_id item) throws InterruptedException
	{
		return isNestedWithinAPlayer(item, true);
	}
	
	
	public static boolean isNestedWithinAPlayer(obj_id item, boolean searchBank) throws InterruptedException
	{
		if (!isIdValid(item))
		{
			return false;
		}
		
		if (isIdValid(getContainingPlayer(item)))
		{
			if (searchBank)
			{
				return true;
			}
			else
			{
				obj_id player = getContainingPlayer(item);
				obj_id bank = getPlayerBank(player);
				
				if (isIdValid(bank))
				{
					obj_id containedBy = getContainedBy(item);
					
					if (containedBy == bank)
					{
						return false;
					}
					else
					{
						return true;
					}
				}
				else
				{
					return true;
				}
			}
		}
		
		return false;
	}
	
	
	public static obj_id getContainingPlayer(obj_id item) throws InterruptedException
	{
		obj_id containedBy = getContainedBy(item);
		if (!isIdValid(containedBy))
		{
			return null;
		}
		else if (isPlayer(containedBy))
		{
			return containedBy;
		}
		return getContainingPlayer(containedBy);
	}
	
	
	public static boolean isNestedWithinANpcCreature(obj_id item) throws InterruptedException
	{
		if (!isIdValid(item))
		{
			return false;
		}
		
		if (isIdValid(getContainingNpcCreature(item)))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static obj_id getContainingNpcCreature(obj_id item) throws InterruptedException
	{
		obj_id containedBy = getContainedBy(item);
		if (!isIdValid(containedBy))
		{
			return null;
		}
		else if (isNpcCreature(containedBy))
		{
			return containedBy;
		}
		
		return getContainingNpcCreature(containedBy);
	}
	
	
	public static obj_id getContainingPlayerOrCreature(obj_id item) throws InterruptedException
	{
		obj_id containedBy = getContainedBy(item);
		if (!isIdValid(containedBy))
		{
			return null;
		}
		else if (isNpcCreature(containedBy) || isPlayer(containedBy))
		{
			return containedBy;
		}
		
		return getContainingPlayerOrCreature(containedBy);
	}
	
	
	public static boolean playerHasItemByTemplate(obj_id objPlayer, String strTemplate) throws InterruptedException
	{
		obj_id objInventory = getInventoryContainer(objPlayer);
		if (isIdValid(objInventory))
		{
			obj_id[] objContents = getContents(objInventory, true);
			
			if (objContents != null)
			{
				for (int intI = 0; intI<objContents.length; intI++)
				{
					testAbortScript();
					String strItemTemplate = getTemplateName(objContents[intI]);
					if (strItemTemplate.equals(strTemplate))
					{
						return true;
					}
				}
			}
		}
		return false;
	}
	
	
	public static boolean playerHasItemByTemplateInDataPad(obj_id objPlayer, String strTemplate) throws InterruptedException
	{
		obj_id pDataPad = getPlayerDatapad(objPlayer);
		if (isIdValid(pDataPad))
		{
			LOG("utils", "playerHasItemByTemplateInDataPad::pDataPad "+ pDataPad);
			obj_id[] objContents = getContents(pDataPad, true);
			LOG("utils", "playerHasItemByTemplateInDataPad::objContents.length "+ objContents.length);
			if (objContents != null)
			{
				for (int intI = 0; intI < objContents.length; intI++)
				{
					testAbortScript();
					String strItemTemplate = getTemplateName(objContents[intI]);
					LOG("utils", "playerHasItemByTemplateInDataPad::strItemTemplate "+ strItemTemplate);
					LOG("utils", "playerHasItemByTemplateInDataPad::strTemplate "+ strTemplate);
					if (strItemTemplate.equals(strTemplate))
					{
						return true;
					}
				}
			}
		}
		else
		{
			LOG("utils", "playerHasItemByTemplateInDataPad::Dpad was invalid");
		}
		return false;
	}
	
	
	public static int playerHasHowManyItemByTemplateInDataPad(obj_id objPlayer, String strTemplate, boolean recurse) throws InterruptedException
	{
		int count = 0;
		obj_id pDataPad = getPlayerDatapad(objPlayer);
		if (isIdValid(pDataPad))
		{
			obj_id[] objContents = getContents(pDataPad, recurse);
			if (objContents != null)
			{
				for (int intI = 0; intI < objContents.length; intI++)
				{
					testAbortScript();
					String strItemTemplate = getTemplateName(objContents[intI]);
					if (strItemTemplate.equals(strTemplate))
					{
						++count;
					}
				}
			}
		}
		
		return count;
	}
	
	
	public static boolean playerHasItemByTemplateInInventoryOrEquipped(obj_id player, String desiredTemplate) throws InterruptedException
	{
		obj_id[] contents = getInventoryAndEquipment(player);
		
		if (contents != null)
		{
			for (int i = 0; i<contents.length; i++)
			{
				testAbortScript();
				String inventoryObject = getTemplateName(contents[i]);
				if (inventoryObject.equals(desiredTemplate))
				{
					return true;
				}
			}
		}
		
		return false;
	}
	
	
	public static obj_id getItemByTemplateInInventoryOrEquipped(obj_id player, String desiredTemplate) throws InterruptedException
	{
		obj_id[] contents = getInventoryAndEquipment(player);
		if (contents != null)
		{
			
			for (int i = 0; i<contents.length; i++)
			{
				testAbortScript();
				String inventoryObject = getTemplateName(contents[i]);
				if (inventoryObject.equals(desiredTemplate))
				{
					return contents[i];
				}
			}
		}
		
		return null;
	}
	
	
	public static boolean playerHasItemWithObjVarInInventoryOrEquipped(obj_id player, String desiredObjVar) throws InterruptedException
	{
		obj_id[] contents = getInventoryAndEquipment(player);
		
		if (contents != null)
		{
			for (int i = 0; i<contents.length; i++)
			{
				testAbortScript();
				if (hasObjVar(contents[i], desiredObjVar))
				{
					return true;
				}
			}
		}
		return false;
	}
	
	
	public static boolean playerHasItemByTemplateWithObjVarInInventoryOrEquipped(obj_id player, String desiredTemplate, String desiredObjVar) throws InterruptedException
	{
		obj_id[] contents = getInventoryAndEquipment(player);
		
		if (contents != null)
		{
			for (int i = 0; i<contents.length; i++)
			{
				testAbortScript();
				String inventoryObject = getTemplateName(contents[i]);
				if (inventoryObject.equals(desiredTemplate))
				{
					if (hasObjVar(contents[i], desiredObjVar))
					{
						return true;
					}
				}
			}
		}
		return false;
	}
	
	
	public static boolean playerHasItemByTemplateInBank(obj_id objPlayer, String strTemplate) throws InterruptedException
	{
		obj_id objBank = getPlayerBank(objPlayer);
		if (isIdValid(objBank))
		{
			obj_id[] objContents = getContents(objBank, true);
			
			if (objContents != null)
			{
				for (int i = 0; i < objContents.length; i++)
				{
					testAbortScript();
					String strItemTemplate = getTemplateName(objContents[i]);
					if (strItemTemplate.equals(strTemplate))
					{
						return true;
					}
				}
			}
		}
		return false;
	}
	
	
	public static boolean playerHasItemByTemplateInBankOrInventory(obj_id objPlayer, String strTemplate) throws InterruptedException
	{
		if (playerHasItemByTemplate(objPlayer, strTemplate))
		{
			return true;
		}
		
		else if (playerHasItemByTemplateInBank( objPlayer, strTemplate))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean playerHasStaticItemInBankOrInventory(obj_id player, String staticItem) throws InterruptedException
	{
		return (isIdValid(getStaticItemInBankOrInventory(player, staticItem)));
	}
	
	
	public static obj_id getStaticItemInBankOrInventory(obj_id player, String staticItem) throws InterruptedException
	{
		obj_id[] contents = getInventoryAndEquipment(player);
		if (contents != null && contents.length > 0)
		{
			for (int i = 0; i < contents.length; i++)
			{
				testAbortScript();
				String itemName = getStaticItemName(contents[i]);
				if (itemName != null && staticItem != null && staticItem.equals(itemName))
				{
					return contents[i];
				}
			}
		}
		
		obj_id bank = getPlayerBank(player);
		if (isIdValid(bank))
		{
			contents = getContents(bank, true);
			
			if (contents != null && contents.length > 0)
			{
				for (int i = 0; i < contents.length; i++)
				{
					testAbortScript();
					String itemName = static_item.getStaticItemName(contents[i]);
					if (itemName != null && staticItem != null && staticItem.equals(itemName))
					{
						return contents[i];
					}
				}
			}
		}
		
		return null;
	}
	
	
	public static boolean playerHasStaticItemInAppearanceInventory(obj_id player, String staticItem) throws InterruptedException
	{
		return (isIdValid(getStaticItemInAppearanceInventory(player, staticItem)));
	}
	
	
	public static obj_id getStaticItemInAppearanceInventory(obj_id player, String staticItem) throws InterruptedException
	{
		obj_id[] contents = getAllItemsFromAppearanceInventory(player);
		{
			if (contents != null && contents.length > 0)
			{
				for (int i = 0; i < contents.length; i++)
				{
					testAbortScript();
					String itemName = getStaticItemName(contents[i]);
					if (itemName != null && staticItem != null && staticItem.equals(itemName))
					{
						return contents[i];
					}
				}
			}
		}
		
		return null;
	}
	
	
	public static obj_id[] getAllStaticItemsInPlayerInventory(obj_id player, String staticItem) throws InterruptedException
	{
		Vector objectList = new Vector ();
		
		obj_id inventory = getInventoryContainer(player);
		if (isIdValid(inventory))
		{
			obj_id[] contents = getInventoryAndEquipment(player);
			
			if (contents != null && contents.length > 0)
			{
				for (int i = 0; i < contents.length; i++)
				{
					testAbortScript();
					if (contents[i] != null)
					{
						if (static_item.isStaticItem(contents[i]))
						{
							String itemName = getStaticItemName(contents[i]);
							if (itemName.equals(staticItem))
							{
								objectList = addElement (objectList, contents[i]);
							}
						}
					}
				}
			}
		}
		
		if (objectList.size () > 0)
		{
			obj_id[] staticList = new obj_id[objectList.size()];
			objectList.toArray (staticList);
			return staticList;
		}
		
		return null;
	}
	
	
	public static obj_id getStaticItemInInventory(obj_id player, String staticItem) throws InterruptedException
	{
		obj_id[] contents = getInventoryAndEquipment(player);
		if (contents != null && contents.length > 0)
		{
			for (int i = 0; i < contents.length; i++)
			{
				testAbortScript();
				String itemName = getStaticItemName(contents[i]);
				if (itemName != null && staticItem != null && staticItem.equals(itemName))
				{
					return contents[i];
				}
			}
		}
		
		return null;
	}
	
	
	public static int countOfStackedItemsInArray(obj_id[] items) throws InterruptedException
	{
		int totalCount = 0;
		
		for (int i = 0; i < items.length; i++)
		{
			testAbortScript();
			int itemCount = getCount(items[i]);
			totalCount = itemCount + totalCount;
		}
		return totalCount;
	}
	
	
	public static obj_id getItemPlayerHasByTemplate(obj_id objPlayer, String strTemplate) throws InterruptedException
	{
		obj_id objInventory = getInventoryContainer(objPlayer);
		if (isIdValid(objInventory))
		{
			obj_id[] objContents = getContents(objInventory, true);
			
			if (objContents != null)
			{
				for (int i = 0; i < objContents.length; i++)
				{
					testAbortScript();
					String strItemTemplate = getTemplateName(objContents[i]);
					if (strItemTemplate.equals(strTemplate))
					{
						return objContents[i];
					}
				}
			}
		}
		return null;
	}
	
	
	public static obj_id getItemPlayerHasByTemplateInDatapad(obj_id objPlayer, String strTemplate) throws InterruptedException
	{
		obj_id pDataPad = getPlayerDatapad(objPlayer);
		if (isIdValid(pDataPad))
		{
			obj_id[] objContents = getContents(pDataPad, true);
			
			if (objContents != null)
			{
				for (int i = 0; i < objContents.length; i++)
				{
					testAbortScript();
					String strItemTemplate = getTemplateName(objContents[i]);
					if (strItemTemplate.equals(strTemplate))
					{
						return objContents[i];
					}
				}
			}
		}
		return null;
	}
	
	
	public static obj_id getItemPlayerHasByTemplateInBank(obj_id objPlayer, String strTemplate) throws InterruptedException
	{
		obj_id objBank = getPlayerBank(objPlayer);
		if (isIdValid(objBank))
		{
			obj_id[] objContents = getContents(objBank, true);
			
			if (objContents != null)
			{
				for (int i = 0; i < objContents.length; i++)
				{
					testAbortScript();
					String strItemTemplate = getTemplateName(objContents[i]);
					if (strItemTemplate.equals(strTemplate))
					{
						return objContents[i];
					}
				}
			}
		}
		return null;
	}
	
	
	public static obj_id getItemPlayerHasByTemplateInBankOrInventory(obj_id objPlayer, String strTemplate) throws InterruptedException
	{
		obj_id itemObj = getItemPlayerHasByTemplate(objPlayer, strTemplate);
		if (isIdValid( itemObj ))
		{
			return itemObj;
		}
		else
		{
			itemObj = getItemPlayerHasByTemplateInBank(objPlayer, strTemplate);
			if (isIdValid(itemObj))
			{
				return itemObj;
			}
		}
		return null;
	}
	
	
	public static obj_id[] getAllItemsPlayerHasByTemplate(obj_id objPlayer, String strTemplate) throws InterruptedException
	{
		obj_id objInventory = getInventoryContainer(objPlayer);
		if (isIdValid(objInventory))
		{
			return getAllItemsInContainerByTemplate( objInventory, strTemplate, true);
		}
		else
		{
			return null;
		}
	}
	
	
	public static obj_id[] getAllItemsPlayerHasByTemplateInBank(obj_id objPlayer, String strTemplate) throws InterruptedException
	{
		obj_id objBank = getPlayerBank(objPlayer);
		if (isIdValid(objBank))
		{
			return getAllItemsInContainerByTemplate( objBank, strTemplate, true );
		}
		else
		{
			return null;
		}
	}
	
	public static obj_id[] getAllItemsInContainerByTemplate(obj_id container, String template, boolean recurse) throws InterruptedException
	{
		return getAllItemsInContainerByTemplate(container, template, recurse, true);
	}
	
	
	public static obj_id[] getAllItemsInContainerByTemplate(obj_id container, String template, boolean recurse, boolean allowEquipped) throws InterruptedException
	{
		Vector objectList = new Vector ();
		if (isIdValid(container))
		{
			obj_id[] objContents = getContents(container, recurse);
			if (objContents != null)
			{
				for (int intI = 0; intI<objContents.length; intI++)
				{
					testAbortScript();
					String itemTemplate = getTemplateName(objContents[intI]);
					if (itemTemplate.equals(template))
					{
						if (!allowEquipped)
						{
							if (isEquipped(objContents[intI]))
							{
								continue;
							}
						}
						objectList = addElement (objectList, objContents[intI]);
					}
				}
			}
		}
		
		if (objectList.size () > 0)
		{
			obj_id[] staticList = new obj_id[objectList.size()];
			objectList.toArray (staticList);
			return staticList;
		}
		else
		{
			return null;
		}
	}
	
	
	public static obj_id[] getAllItemsPlayerHasByTemplateStartsWith(obj_id objPlayer, String strTemplate) throws InterruptedException
	{
		obj_id objInventory = getInventoryContainer(objPlayer);
		if (isIdValid(objInventory))
		{
			return getAllItemsInContainerByTemplateStartsWith( objInventory, strTemplate, true);
		}
		else
		{
			return null;
		}
	}
	
	
	public static obj_id[] getAllItemsPlayerHasByTemplateInBankStartsWith(obj_id objPlayer, String strTemplate) throws InterruptedException
	{
		obj_id objBank = getPlayerBank(objPlayer);
		if (isIdValid(objBank))
		{
			return getAllItemsInContainerByTemplateStartsWith( objBank, strTemplate, true );
		}
		else
		{
			return null;
		}
	}
	
	
	public static obj_id[] getAllItemsInContainerByTemplateStartsWith(obj_id container, String template, boolean recurse) throws InterruptedException
	{
		Vector objectList = new Vector ();
		if (isIdValid(container))
		{
			obj_id[] objContents = getContents(container, recurse);
			if (objContents != null)
			{
				for (int intI = 0; intI<objContents.length; intI++)
				{
					testAbortScript();
					String itemTemplate = getTemplateName(objContents[intI]);
					if (itemTemplate.startsWith (template))
					{
						objectList = addElement (objectList, objContents[intI]);
					}
				}
			}
		}
		
		if (objectList.size () > 0)
		{
			obj_id[] staticList = new obj_id[objectList.size()];
			objectList.toArray (staticList);
			return staticList;
		}
		else
		{
			return null;
		}
	}
	
	
	public static obj_id[] getAllItemsPlayerHasByTemplateInBankAndInventory(obj_id objPlayer, String strTemplate) throws InterruptedException
	{
		Vector objectList = new Vector ();
		
		obj_id objInventory = getInventoryContainer(objPlayer);
		if (isIdValid(objInventory))
		{
			obj_id[] objContents = getContents(objInventory, true);
			
			if (objContents != null)
			{
				for (int i = 0; i < objContents.length; i++)
				{
					testAbortScript();
					String strItemTemplate = getTemplateName(objContents[i]);
					if (strItemTemplate.equals(strTemplate))
					{
						objectList = addElement (objectList, objContents[i]);
					}
				}
			}
		}
		
		obj_id objBank = getPlayerBank(objPlayer);
		if (isIdValid(objBank))
		{
			obj_id[] objContents = getContents(objBank, true);
			
			if (objContents != null)
			{
				for (int j = 0; j < objContents.length; j++)
				{
					testAbortScript();
					String strItemTemplate = getTemplateName(objContents[j]);
					if (strItemTemplate.equals(strTemplate))
					{
						objectList = addElement (objectList, objContents[j]);
					}
				}
			}
		}
		if (objectList.size () > 0)
		{
			obj_id[] staticList = new obj_id[objectList.size()];
			objectList.toArray (staticList);
			return staticList;
		}
		
		return null;
	}
	
	
	public static obj_id[] getAllItemsInBankAndInventory(obj_id objPlayer) throws InterruptedException
	{
		Vector objectList = new Vector ();
		
		obj_id[] objContents = getInventoryAndEquipment(objPlayer);
		
		if (objContents != null)
		{
			for (int i = 0; i < objContents.length; i++)
			{
				testAbortScript();
				objectList = addElement (objectList, objContents[i]);
			}
		}
		
		obj_id objBank = getPlayerBank(objPlayer);
		if (isIdValid(objBank))
		{
			objContents = getContents(objBank, true);
			
			if (objContents != null)
			{
				for (int j = 0; j < objContents.length; j++)
				{
					testAbortScript();
					objectList = addElement (objectList, objContents[j]);
				}
			}
		}
		if (objectList.size () > 0)
		{
			obj_id[] staticList = new obj_id[objectList.size()];
			objectList.toArray (staticList);
			return staticList;
		}
		
		return null;
	}
	
	
	public static location getRandomLocationInRing(location locOrigin, float fltMinDistance, float fltMaxDistance) throws InterruptedException
	{
		
		float fltRand = rand();
		
		float fltDistance = (fltMaxDistance - fltMinDistance) * fltRand +fltMinDistance;
		obj_id objTest = null;
		debugServerConsoleMsg(objTest, "fltDistance is "+fltDistance);
		
		float fltAngle = rand(0, 360);
		locOrigin = rotatePointXZ(locOrigin, fltDistance, fltAngle);
		
		return locOrigin;
	}
	
	
	public static location rotatePointXZ(location locOrigin, float fltDistance, float fltAngle) throws InterruptedException
	{
		location locOffset = (location)locOrigin.clone();
		locOffset.x = locOrigin.x + fltDistance;
		locOffset.z = locOrigin.z;
		
		return rotatePointXZ(locOrigin, locOffset, fltAngle);
	}
	
	
	public static location rotatePointXZ(location locOrigin, location locPoint, float fltAngle) throws InterruptedException
	{
		float dx = locPoint.x - locOrigin.x;
		float dz = locPoint.z - locOrigin.z;
		
		float fltRadians = (float)Math.toRadians(fltAngle);
		float fltC = (float)Math.cos(fltRadians);
		float fltS = (float) Math.sin(fltRadians);
		
		location locNewOffset = (location)locOrigin.clone();
		locNewOffset.x += (dx * fltS) - (dz * fltC);
		locNewOffset.y = locPoint.y;
		locNewOffset.z += (dx * fltC) + (dz * fltS);
		
		return locNewOffset;
	}
	
	
	public static location rotatePointXZ(location locPoint, float fltAngle) throws InterruptedException
	{
		
		float fltRadians = (float) Math.toRadians(fltAngle);
		float fltC = (float) Math.cos(fltRadians);
		float fltS = (float) Math.sin(fltRadians);
		
		location locNewPoint = (location)locPoint.clone();
		locNewPoint.x += (locPoint.x * fltC) - (locPoint.z * fltS);
		locNewPoint.z += (locPoint.x * fltS) + (locPoint.z * fltC);
		
		return locNewPoint;
	}
	
	
	public static float getHeadingToLocation(location here, location there) throws InterruptedException
	{
		if (here == null || there == null)
		{
			return -1f;
		}
		
		float dx = there.x - here.x;
		float dz = there.z - here.z;
		
		double radHeading = Math.atan2(-dx, dz);
		double degreeHeading = Math.toDegrees(radHeading);
		return (float)(degreeHeading);
	}
	
	
	public static location findLocInFrontOfTarget(obj_id target, float distance) throws InterruptedException
	{
		if (target == null || !exists(target))
		{
			return null;
		}
		
		location origin = getLocation(target);
		
		if (origin == null)
		{
			return null;
		}
		
		float yaw = getYaw(target);
		
		location newLoc = rotatePointXZ(origin, distance, yaw);
		
		return newLoc;
	}
	
	
	public static attrib_mod createAttribMod(int attrib, int value, float duration, float attack, float decay) throws InterruptedException
	{
		return new attrib_mod(attrib, value, duration, attack, decay);
	}
	
	
	public static attrib_mod createAttribMod(int attrib, int value, float duration, float attack) throws InterruptedException
	{
		return new attrib_mod(attrib, value, duration, attack, 0.0f);
	}
	
	
	public static attrib_mod createAttribMod(int attrib, int value, float duration) throws InterruptedException
	{
		return new attrib_mod(attrib, value, duration, 0.0f, 0.0f);
	}
	
	
	public static attrib_mod createHealDamageAttribMod(int attrib, int value) throws InterruptedException
	{
		if ((value < 1) || (attrib % 3 != 0))
		{
			return null;
		}
		
		return new attrib_mod(attrib, value, 0.0f, 0.0f, MOD_POOL);
	}
	
	
	public static attrib_mod createHealWoundAttribMod(int attrib, int value) throws InterruptedException
	{
		if (value < 1)
		{
			return null;
		}
		
		return new attrib_mod(attrib, value, 0.0f, healing.AM_HEAL_WOUND, 0.0f);
	}
	
	
	public static attrib_mod createHealShockAttribMod(int value) throws InterruptedException
	{
		if (value < 1)
		{
			return null;
		}
		
		return new attrib_mod(0, value, 0.0f, healing.AM_HEAL_SHOCK, 0.0f);
	}
	
	
	public static attrib_mod createAddShockAttribMod(int value) throws InterruptedException
	{
		if (value < 1)
		{
			return null;
		}
		
		return new attrib_mod(0, value, 0.0f, healing.AM_ADD_SHOCK, 0.0f);
	}
	
	
	public static attrib_mod createWoundAttribMod(int attrib, int value) throws InterruptedException
	{
		if (value < 1)
		{
			return null;
		}
		
		return new attrib_mod(attrib, value, 0.0f, 0.0f, MOD_WOUND);
	}
	
	
	public static attrib_mod createAntidoteAttribMod(int attrib) throws InterruptedException
	{
		return new attrib_mod(attrib, 0, 0.0f, 0.0f, MOD_ANTIDOTE);
	}
	
	
	public static boolean addAttribMod(obj_id target, int attrib, int value, float duration) throws InterruptedException
	{
		return addAttribModifier(target, attrib, value, duration, 0.0f, 0.0f);
	}
	
	
	public static boolean addAttribMod(obj_id target, attrib_mod am) throws InterruptedException
	{
		if (target == null || am == null)
		{
			return false;
		}
		
		boolean litmus = true;
		
		int attrib = am.getAttribute();
		float duration = am.getDuration();
		
		int amt = am.getValue();
		int attack = (int)(am.getAttack());
		int decay = (int)(am.getDecay());
		
		if (attack < 0)
		{
			switch (attack)
			{
				case (int)healing.AM_HEAL_WOUND:
				if (amt < 0)
				{
					amt = 0;
				}
				break;
				
				case (int)healing.AM_HEAL_SHOCK:
				if (amt < 0)
				{
					amt = 0;
				}
				litmus &= healShockWound(target, amt);
				break;
				
				case (int)healing.AM_ADD_SHOCK:
				if (amt < 0)
				{
					amt = 0;
				}
				litmus &= addShockWound(target, amt);
				break;
			}
		}
		
		else
		{
			if (!addAttribModifier(target, attrib, amt, duration, am.getAttack(), am.getDecay()))
			{
				
				litmus = false;
			}
		}
		return litmus;
	}
	
	
	public static boolean addAttribMod(obj_id target, attrib_mod[] am) throws InterruptedException
	{
		if ((target == null) || (am == null) || (am.length == 0))
		{
			return false;
		}
		
		boolean ret = true;
		
		for (int i = 0; i < am.length; i++)
		{
			testAbortScript();
			ret &= addAttribMod(target, am[i]);
		}
		
		return ret;
	}
	
	
	public static boolean setHeading(obj_id target, float heading) throws InterruptedException
	{
		if (target == null)
		{
			return false;
		}
		
		return setYaw(target, heading);
	}
	
	
	public static dictionary parseKeyPairList(String keyPairList, char delim) throws InterruptedException
	{
		if (keyPairList.equals(""))
		{
			return null;
		}
		
		dictionary d = new dictionary();
		
		String[] pairs = split(keyPairList, delim);
		if ((pairs == null) || (pairs.length == 0))
		{
			return null;
		}
		
		for (int i = 0; i < pairs.length; i++)
		{
			testAbortScript();
			String[] tmp = split(pairs[i], '=');
			if ((tmp != null) && (tmp.length == 2))
			{
				int val = stringToInt(tmp[1]);
				if (val != -1)
				{
					d.put(tmp[0],val);
				}
			}
		}
		
		return d;
	}
	
	
	public static dictionary parseKeyPairList(String keyPairList) throws InterruptedException
	{
		return parseKeyPairList(keyPairList, ',');
	}
	
	
	public static boolean putInPlayerInventory(obj_id player, obj_id item) throws InterruptedException
	{
		if ((!isIdValid(player)) || (!isPlayer(player)) || (item == null))
		{
			return false;
		}
		
		obj_id inventory = getObjectInSlot(player, SLOT_INVENTORY);
		if (inventory == null)
		{
			return false;
		}
		
		return putIn(item, inventory);
	}
	
	
	public static boolean putInPlayerDatapad(obj_id player, obj_id item) throws InterruptedException
	{
		if ((!isIdValid(player)) || (!isPlayer(player)) || (item == null))
		{
			
			return false;
		}
		
		obj_id datapad = getObjectInSlot(player, SLOT_DATAPAD);
		if (datapad == null)
		{
			
			return false;
		}
		
		return putIn(item, datapad);
	}
	
	
	public static obj_id getInventoryContainer(obj_id target) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return null;
		}
		
		return getObjectInSlot(target, SLOT_INVENTORY);
	}
	
	
	public static obj_id getDroidInventoryContainer(obj_id droid) throws InterruptedException
	{
		if (!isIdValid(droid))
		{
			return null;
		}
		
		if (callable.hasCallableCD(droid))
		{
			return getObjectInSlot(callable.getCallableCD(droid), SLOT_INVENTORY);
		}
		else
		{
			return null;
		}
		
	}
	
	
	public static obj_id getMissionBag(obj_id objPlayer) throws InterruptedException
	{
		if (!isIdValid(objPlayer))
		{
			return null;
		}
		
		return getObjectInSlot(objPlayer, SLOT_MISSION_BAG);
		
	}
	
	
	public static obj_id getDatapad(obj_id target) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return null;
		}
		
		return getObjectInSlot(target, SLOT_DATAPAD);
	}
	
	
	public static obj_id getPlayerDatapad(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player) || !isPlayer(player))
		{
			return null;
		}
		
		return getObjectInSlot(player, SLOT_DATAPAD);
	}
	
	
	public static obj_id getPlayerHangar(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player) || !isPlayer(player))
		{
			return null;
		}
		
		return getObjectInSlot(player, SLOT_HANGAR);
	}
	
	
	public static obj_id getPlayerBank(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player) || !isPlayer(player))
		{
			return null;
		}
		
		return getObjectInSlot(player, SLOT_BANK);
	}
	
	
	public static boolean itemNotActive(obj_id player) throws InterruptedException
	{
		if ((!isIdValid(player)) || (!isPlayer(player)))
		{
			return false;
		}
		
		sendSystemMessage(player, SID_OBJECT_NOT_ACTIVE);
		return true;
	}
	
	
	public static boolean requestContainerOpen(obj_id player, obj_id container) throws InterruptedException
	{
		if ((!isIdValid(player)) || (!isPlayer(player)) || (!isIdValid(container)) || (getContainerType(container) == 0))
		{
			return false;
		}
		
		queueCommand(player, (1880585606), container, "", COMMAND_PRIORITY_DEFAULT);
		return true;
	}
	
	
	public static String packStringId(string_id strId) throws InterruptedException
	{
		if (strId == null)
		{
			
			return null;
		}
		
		if (!strId.isValid())
		{
			return null;
		}
		
		String strPackedId = "@"+strId.getTable()+":"+strId.getAsciiId();
		return strPackedId;
		
	}
	
	public static string_id unpackString(String strId) throws InterruptedException
	{
		if (strId == null || strId.equals(""))
		{
			return null;
		}
		
		if (strId.startsWith("@"))
		{
			strId = strId.substring(1);
		}
		
		String[] s = split(strId, ':');
		if (s != null && s.length == 2)
		{
			return new string_id(s[0], s[1]);
		}
		
		return null;
	}
	
	
	public static obj_id getNearbyPlayerByName(obj_id actor, String name) throws InterruptedException
	{
		java.util.StringTokenizer st = new java.util.StringTokenizer(name);
		if (st.hasMoreTokens())
		{
			String compareName = toLower(st.nextToken());
			
			obj_id[] players = getPlayerCreaturesInRange(actor, 128.0f);
			if (players != null)
			{
				for (int i = 0; i < players.length; ++i)
				{
					testAbortScript();
					java.util.StringTokenizer st2 = new java.util.StringTokenizer(getName(players[i]));
					String playerName = toLower(st2.nextToken());
					if (compareName.equals(playerName))
					{
						return players[i];
					}
				}
			}
		}
		return obj_id.NULL_ID;
	}
	
	
	public static void sendPostureChange(obj_id objCreature, int intPosture) throws InterruptedException
	{
		attacker_results cbtAttackerResults = new attacker_results();
		
		setPosture(objCreature, intPosture);
		String strPlayback = "change_posture";
		cbtAttackerResults.id = objCreature;
		cbtAttackerResults.endPosture = intPosture;
		cbtAttackerResults.weapon = null;
		doCombatResults(strPlayback, cbtAttackerResults, null);
		return;
	}
	
	
	public static obj_id[] getBuildingsInObjIdList(obj_id[] items) throws InterruptedException
	{
		if ((items == null) || (items.length == 0))
		{
			return null;
		}
		
		Vector buildings = new Vector();
		buildings.setSize(0);
		
		for (int i = 0; i < items.length; i++)
		{
			testAbortScript();
			if (getCellNames(items[i]) == null)
			{
			}
			else
			{
				buildings = addElement(buildings, items[i]);
			}
		}
		
		if ((buildings == null) || (buildings.size() == 0))
		{
			return null;
		}
		else
		{
			obj_id[] _buildings = new obj_id[0];
			if (buildings != null)
			{
				_buildings = new obj_id[buildings.size()];
				buildings.toArray(_buildings);
			}
			return _buildings;
		}
	}
	
	
	public static obj_id[] getContainedGOTObjects(obj_id container, int got, boolean recurse, boolean allowDerived) throws InterruptedException
	{
		if (!isIdValid(container) || (got < 0))
		{
			return null;
		}
		
		if (getContainerType(container) == 0)
		{
			
			return null;
		}
		
		obj_id[] contents = getContents(container, recurse);
		if ((contents == null) || (contents.length == 0))
		{
			return null;
		}
		
		Vector ret = new Vector();
		ret.setSize(0);
		for (int i = 0; i < contents.length; i++)
		{
			testAbortScript();
			int myType = getGameObjectType(contents[i]);
			if (!allowDerived)
			{
				if (myType == got)
				{
					ret = addElement(ret, contents[i]);
				}
			}
			else
			{
				if (isGameObjectTypeOf(myType, got))
				{
					ret = addElement(ret, contents[i]);
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
	
	
	public static obj_id[] getContainedGOTObjects(obj_id container, int got, boolean recurse) throws InterruptedException
	{
		return getContainedGOTObjects(container, got, recurse, false);
	}
	
	
	public static obj_id[] getContainedGOTObjects(obj_id container, int got) throws InterruptedException
	{
		return getContainedGOTObjects(container, got, false);
	}
	
	
	public static obj_id[] getContainedObjectsWithObjVar(obj_id container, String var, boolean recurse) throws InterruptedException
	{
		if (!isIdValid(container))
		{
			return null;
		}
		
		if (getContainerType(container) == 0)
		{
			return null;
		}
		
		if (var == null || var.equals(""))
		{
			return null;
		}
		
		obj_id[] contents = getContents(container, recurse);
		if (contents == null || contents.length == 0)
		{
			return null;
		}
		
		Vector ret = new Vector();
		ret.setSize(0);
		for (int i = 0; i < contents.length; i++)
		{
			testAbortScript();
			if (hasObjVar(contents[i], var))
			{
				ret = addElement(ret, contents[i]);
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
	
	
	public static obj_id[] getContainedObjectsWithObjVar(obj_id container, String var) throws InterruptedException
	{
		return getContainedObjectsWithObjVar(container, var, false);
	}
	
	
	public static obj_id[] getContainedObjectsWithScript(obj_id container, String script, boolean recurse) throws InterruptedException
	{
		if (!isIdValid(container))
		{
			return null;
		}
		
		if (getContainerType(container) == 0)
		{
			return null;
		}
		
		if (script == null || script.equals(""))
		{
			return null;
		}
		
		obj_id[] contents = getContents(container, recurse);
		if (contents == null || contents.length == 0)
		{
			return null;
		}
		
		Vector ret = new Vector();
		ret.setSize(0);
		for (int i = 0; i < contents.length; i++)
		{
			testAbortScript();
			if (hasScript(contents[i], script))
			{
				ret = addElement(ret, contents[i]);
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
	
	
	public static obj_id[] getContainedObjectsWithScript(obj_id container, String script) throws InterruptedException
	{
		return getContainedObjectsWithScript(container, script, false);
	}
	
	
	public static void sendMail(string_id subject, string_id body, String to, String from) throws InterruptedException
	{
		String body_oob = chatMakePersistentMessageOutOfBandBody(null, body);
		String subject_str = "@"+ subject.toString ();
		
		chatSendPersistentMessage( from, to, subject_str, null, body_oob );
		
		return;
	}
	
	
	public static void sendMail(string_id subject, string_id body, obj_id to, String from) throws InterruptedException
	{
		String body_oob = chatMakePersistentMessageOutOfBandBody(null, body);
		String subject_str = "@"+ subject.toString ();
		
		chatSendPersistentMessage( from, to, subject_str, null, body_oob );
	}
	
	
	public static void sendMail(string_id subject, prose_package body, obj_id to, String from) throws InterruptedException
	{
		String body_oob = chatMakePersistentMessageOutOfBandBody(null, body);
		String subject_str = "@"+ subject.toString ();
		
		chatSendPersistentMessage( from, to, subject_str, null, body_oob );
	}
	
	
	public static void sendMail(string_id subject, prose_package body, String to, String from) throws InterruptedException
	{
		String body_oob = chatMakePersistentMessageOutOfBandBody(null, body);
		String subject_str = "@"+ subject.toString ();
		
		chatSendPersistentMessage( from, to, subject_str, null, body_oob );
	}
	
	
	public static boolean isNightTime() throws InterruptedException
	{
		if (getLocalTime () < getLocalDayLength ())
		{
			
			return false;
		}
		else
		{
			return true;
			
		}
	}
	
	
	public static void sendSystemMessageTestingOnly(obj_id[] players, String message) throws InterruptedException
	{
		if (players == null || players.length == 0)
		{
			return;
		}
		
		for (int i=0; i< players.length; i++)
		{
			testAbortScript();
			if (isIdValid(players[i]) && exists(players[i]))
			{
				sendSystemMessageTestingOnly(players[i], message);
			}
		}
	}
	
	
	public static void sendSystemMessage(obj_id[] players, string_id message) throws InterruptedException
	{
		if (players == null || players.length == 0)
		{
			return;
		}
		
		for (int i=0; i< players.length; i++)
		{
			testAbortScript();
			if (isIdValid(players[i]) && exists(players[i]))
			{
				sendSystemMessage(players[i], message);
			}
		}
	}
	
	
	public static void sendSystemMessageProse(obj_id[] players, prose_package message) throws InterruptedException
	{
		if (players == null || players.length == 0)
		{
			return;
		}
		
		for (int i=0; i< players.length; i++)
		{
			testAbortScript();
			if (isIdValid(players[i]) && exists(players[i]))
			{
				sendSystemMessageProse(players[i], message);
			}
		}
	}
	
	
	public static void sendSystemMessagePob(obj_id pob, string_id message) throws InterruptedException
	{
		obj_id[] players = trial.getPlayersInDungeon(pob);
		if (players != null && players.length > 0)
		{
			utils.sendSystemMessage(players, message);
		}
	}
	
	
	public static void sendSystemMessageProsePob(obj_id pob, prose_package message) throws InterruptedException
	{
		obj_id[] players = trial.getPlayersInDungeon(pob);
		if (players != null && players.length > 0)
		{
			utils.sendSystemMessageProse(players, message);
		}
	}
	
	
	public static void messagePlayer(obj_id source, obj_id player, string_id message, String templateOverride) throws InterruptedException
	{
		prose_package pp = new prose_package();
		pp.stringId = message;
		
		if (templateOverride.equals("none"))
		{
			commPlayer(source, player, pp);
		}
		else
		{
			commPlayer(source, player, pp, templateOverride);
		}
	}
	
	
	public static void messagePlayer(obj_id source, obj_id player, string_id message) throws InterruptedException
	{
		messagePlayer(source, player, message, "none");
	}
	
	
	public static void messagePlayer(obj_id source, obj_id[] players, string_id message, String templateOverride) throws InterruptedException
	{
		prose_package pp = new prose_package();
		pp.stringId = message;
		
		if (templateOverride.equals("none"))
		{
			commPlayers(source, players, pp);
		}
		else
		{
			commPlayers(source, players, pp, templateOverride);
		}
	}
	
	
	public static void messagePlayer(obj_id source, obj_id[] players, string_id message, String templateOverride, float duration) throws InterruptedException
	{
		prose_package pp = new prose_package();
		pp.stringId = message;
		
		if (templateOverride.equals("none"))
		{
			commPlayers(source, players, pp);
			return;
		}
		
		if (duration <= 0.0f)
		{
			commPlayers(source, players, pp, templateOverride);
		}
		else
		{
			commPlayers(source, templateOverride, null, duration, players, pp);
		}
	}
	
	
	public static void messagePlayer(obj_id source, obj_id[] players, string_id message) throws InterruptedException
	{
		messagePlayer(source, players, message, "none");
	}
	
	
	public static boolean setLocalVar(obj_id target, String path, Vector val) throws InterruptedException
	{
		
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		if (val == null || val.isEmpty())
		{
			return false;
		}
		
		Object sample = val.get(0);
		if (sample == null)
		{
			return false;
		}
		
		boolean result = false;
		if (sample instanceof Integer)
		{
			int count = val.size();
			int[] tempVal = new int[count];
			for (int i = 0; i < count; ++i)
			{
				testAbortScript();
				tempVal[i] = ((Integer)val.get(i)).intValue();
			}
			result = setLocalVar(target, path, tempVal);
		}
		else if (sample instanceof Boolean)
		{
			int count = val.size();
			boolean[] tempVal = new boolean[count];
			for (int i = 0; i < count; ++i)
			{
				testAbortScript();
				tempVal[i] = ((Boolean)val.get(i)).booleanValue();
			}
			result = setLocalVar(target, path, tempVal);
		}
		else if (sample instanceof Float)
		{
			int count = val.size();
			float[] tempVal = new float[count];
			for (int i = 0; i < count; ++i)
			{
				testAbortScript();
				tempVal[i] = ((Float)val.get(i)).intValue();
			}
			result = setLocalVar(target, path, tempVal);
		}
		else if (sample instanceof String)
		{
			String[] tempVal = new String[val.size()];
			tempVal = (String[])val.toArray(tempVal);
			result = setLocalVar(target, path, tempVal);
		}
		else if (sample instanceof obj_id)
		{
			obj_id[] tempVal = new obj_id[val.size()];
			tempVal = (obj_id[])val.toArray(tempVal);
			result = setLocalVar(target, path, tempVal);
		}
		else if (sample instanceof location)
		{
			location[] tempVal = new location[val.size()];
			tempVal = (location[])val.toArray(tempVal);
			result = setLocalVar(target, path, tempVal);
		}
		else if (sample instanceof transform)
		{
			transform[] tempVal = new transform[val.size()];
			tempVal = (transform[])val.toArray(tempVal);
			result = setLocalVar(target, path, tempVal);
		}
		else if (sample instanceof vector)
		{
			vector[] tempVal = new vector[val.size()];
			tempVal = (vector[])val.toArray(tempVal);
			result = setLocalVar(target, path, tempVal);
		}
		
		return result;
	}
	
	
	public static boolean setLocalVar(obj_id target, String path, location val) throws InterruptedException
	{
		
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		dictionary dd = target.getScriptDictionary();
		dd.put(path, val );
		return true;
		
	}
	
	public static boolean setLocalVar(obj_id target, String path, location[] val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		dictionary dd = target.getScriptDictionary();
		dd.put(path, val );
		return true;
		
	}
	
	
	public static boolean setLocalVar(obj_id target, String path, String val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		dictionary dd = target.getScriptDictionary();
		dd.put(path, val );
		return true;
		
	}
	
	public static boolean setLocalVar(obj_id target, String path, String[] val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		dictionary dd = target.getScriptDictionary();
		dd.put(path, val );
		return true;
		
	}
	
	
	public static boolean setLocalVar(obj_id target, String path, obj_id val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		dictionary dd = target.getScriptDictionary();
		dd.put(path, val );
		return true;
		
	}
	
	public static boolean setLocalVar(obj_id target, String path, obj_id[] val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		dictionary dd = target.getScriptDictionary();
		dd.put(path, val );
		return true;
		
	}
	
	
	public static boolean setLocalVar(obj_id target, String path, int val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		dictionary dd = target.getScriptDictionary();
		dd.put(path, val );
		return true;
		
	}
	
	public static boolean setLocalVar(obj_id target, String path, int[] val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		dictionary dd = target.getScriptDictionary();
		dd.put(path, val );
		return true;
		
	}
	
	
	public static boolean setLocalVar(obj_id target, String path, float val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		dictionary dd = target.getScriptDictionary();
		dd.put(path, val );
		return true;
		
	}
	
	public static boolean setLocalVar(obj_id target, String path, float[] val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		dictionary dd = target.getScriptDictionary();
		dd.put(path, val );
		return true;
		
	}
	
	
	public static boolean setLocalVar(obj_id target, String path, boolean val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		dictionary dd = target.getScriptDictionary();
		dd.put(path, val );
		return true;
		
	}
	
	public static boolean setLocalVar(obj_id target, String path, transform val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		dictionary dd = target.getScriptDictionary();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setLocalVar(obj_id target, String path, transform[] val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		dictionary dd = target.getScriptDictionary();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setLocalVar(obj_id target, String path, vector val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		dictionary dd = target.getScriptDictionary();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setLocalVar(obj_id target, String path, vector[] val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		dictionary dd = target.getScriptDictionary();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setLocalVar(obj_id target, String path, boolean[] val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		dictionary dd = target.getScriptDictionary();
		dd.put(path, val );
		return true;
		
	}
	
	
	public static boolean setLocalVar(obj_id target, String path, dictionary val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		dictionary dd = target.getScriptDictionary();
		dd.put(path, val);
		return true;
	}
	
	
	public static boolean hasLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		return dd.containsKey(path);
	}
	
	
	public static boolean removeLocalVar(obj_id target, String path) throws InterruptedException
	{
		if ((target == null) || (target == obj_id.NULL_ID))
		{
			return false;
		}
		
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		dictionary dd = target.getScriptDictionary();
		dd.remove(path);
		
		return true;
	}
	
	
	public static int getIntLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		return dd.getInt(path);
	}
	
	
	public static int[] getIntArrayLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		return dd.getIntArray(path);
	}
	
	
	public static float getFloatLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		return dd.getFloat(path);
	}
	
	
	public static float[] getFloatArrayLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		return dd.getFloatArray(path);
	}
	
	
	public static String getStringLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		return dd.getString(path);
	}
	
	
	public static String[] getStringArrayLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		return dd.getStringArray(path);
	}
	
	
	public static transform[] getTransformArrayLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		return dd.getTransformArray(path);
	}
	
	
	public static boolean getBooleanLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		return dd.getBoolean(path);
	}
	
	
	public static boolean[] getBooleanArrayLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		return dd.getBooleanArray(path);
	}
	
	
	public static location getLocationLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		return dd.getLocation(path);
	}
	
	
	public static location[] getLocationArrayLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		return dd.getLocationArray(path);
	}
	
	
	public static obj_id getObjIdLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		return dd.getObjId(path);
	}
	
	
	public static obj_id[] getObjIdArrayLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		return dd.getObjIdArray(path);
	}
	
	
	public static Vector getResizeableObjIdArrayLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		obj_id[] objArray = dd.getObjIdArray(path);
		return new Vector(Arrays.asList(objArray));
	}
	
	
	public static Vector getResizeableLocationArrayLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		location[] locArray = dd.getLocationArray(path);
		return new Vector(Arrays.asList(locArray));
	}
	
	
	public static Vector getResizeableIntArrayLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		int[] intArray = dd.getIntArray(path);
		Vector rszArray = new Vector(intArray.length + 1);
		for (int i = 0; i < intArray.length; ++i)
		{
			testAbortScript();
			rszArray.add(new Integer(intArray[i]));
		}
		return rszArray;
	}
	
	
	public static Vector getResizeableFloatArrayLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		float[] fltArray = dd.getFloatArray(path);
		Vector rszArray = new Vector(fltArray.length + 10);
		for (int i = 0; i < fltArray.length; ++i)
		{
			testAbortScript();
			rszArray.add(new Float(fltArray[i]));
		}
		return rszArray;
	}
	
	
	public static Vector getResizeableStringArrayLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		String[] strArray = dd.getStringArray(path);
		return new Vector(Arrays.asList((String[])strArray));
	}
	
	
	public static Vector getResizeableTransformArrayLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		transform[] trArray = dd.getTransformArray(path);
		Vector v1 = new Vector( Arrays.asList(trArray));
		
		return v1;
	}
	
	
	public static Vector getResizeableVectorArrayLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		vector[] vctArray = dd.getVectorArray(path);
		Vector v1 = new Vector( Arrays.asList(vctArray));
		return v1;
	}
	
	
	public static string_id getStringIdLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		return dd.getStringId(path);
	}
	
	
	public static string_id[] getStringIdArrayLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		return dd.getStringIdArray(path);
	}
	
	
	public static dictionary getDictionaryLocalVar(obj_id target, String path) throws InterruptedException
	{
		dictionary dd = target.getScriptDictionary();
		return dd.getDictionary(path);
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, location val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, location[] val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, String val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, String[] val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, String[][] val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val);
		return true;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, obj_id val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, obj_id[] val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, int val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, int[] val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, long val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, long[] val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, double val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, double[] val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, float val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, float[] val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, boolean val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, transform val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, transform[] val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, vector val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, vector[] val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, boolean[] val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val );
		return true;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, Vector val) throws InterruptedException
	{
		
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		if (val == null || val.isEmpty())
		{
			return false;
		}
		
		Object sample = val.get(0);
		if (sample == null)
		{
			return false;
		}
		
		boolean result = false;
		if (sample instanceof Integer)
		{
			int count = val.size();
			int[] tempVal = new int[count];
			for (int i = 0; i < count; ++i)
			{
				testAbortScript();
				tempVal[i] = ((Integer)val.get(i)).intValue();
			}
			result = setScriptVar(target, path, tempVal);
		}
		else if (sample instanceof Boolean)
		{
			int count = val.size();
			boolean[] tempVal = new boolean[count];
			for (int i = 0; i < count; ++i)
			{
				testAbortScript();
				tempVal[i] = ((Boolean)val.get(i)).booleanValue();
			}
			result = setScriptVar(target, path, tempVal);
		}
		else if (sample instanceof Float)
		{
			int count = val.size();
			float[] tempVal = new float[count];
			for (int i = 0; i < count; ++i)
			{
				testAbortScript();
				tempVal[i] = ((Float)val.get(i)).intValue();
			}
			result = setScriptVar(target, path, tempVal);
		}
		else if (sample instanceof String)
		{
			String[] tempVal = new String[val.size()];
			tempVal = (String[])val.toArray(tempVal);
			result = setScriptVar(target, path, tempVal);
		}
		else if (sample instanceof obj_id)
		{
			obj_id[] tempVal = new obj_id[val.size()];
			tempVal = (obj_id[])val.toArray(tempVal);
			result = setScriptVar(target, path, tempVal);
		}
		else if (sample instanceof location)
		{
			location[] tempVal = new location[val.size()];
			tempVal = (location[])val.toArray(tempVal);
			result = setScriptVar(target, path, tempVal);
		}
		else if (sample instanceof transform)
		{
			transform[] tempVal = new transform[val.size()];
			tempVal = (transform[])val.toArray(tempVal);
			result = setScriptVar(target, path, tempVal);
		}
		else if (sample instanceof vector)
		{
			vector[] tempVal = new vector[val.size()];
			tempVal = (vector[])val.toArray(tempVal);
			result = setScriptVar(target, path, tempVal);
		}
		
		return result;
	}
	
	
	public static boolean setScriptVar(obj_id target, String path, dictionary val) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(path, val);
		return true;
	}
	
	
	public static boolean hasScriptVar(obj_id target, String path) throws InterruptedException
	{
		return target.hasScriptVar( path );
	}
	
	
	public static boolean hasScriptVarTree(obj_id target, String path) throws InterruptedException
	{
		if ((target == null) || (target == obj_id.NULL_ID))
		{
			return false;
		}
		
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		Enumeration keys = dd.keys();
		while (keys.hasMoreElements())
		{
			testAbortScript();
			String key = (String)(keys.nextElement());
			if (key.equals(path) || key.startsWith(path + "."))
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public static boolean removeScriptVar(obj_id target, String path) throws InterruptedException
	{
		if ((target == null) || (target == obj_id.NULL_ID))
		{
			return false;
		}
		
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.remove(path);
		
		return true;
	}
	
	
	public static boolean removeScriptVarTree(obj_id target, String path) throws InterruptedException
	{
		if ((target == null) || (target == obj_id.NULL_ID))
		{
			return false;
		}
		
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		Enumeration keys = dd.keys();
		while (keys.hasMoreElements())
		{
			testAbortScript();
			String key = (String)(keys.nextElement());
			if (key.equals(path) || key.startsWith(path + "."))
			{
				dd.remove(key);
			}
		}
		
		return true;
	}
	
	
	public static int getIntScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getInt(path);
	}
	
	
	public static int[] getIntArrayScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getIntArray(path);
	}
	
	
	public static float getFloatScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getFloat(path);
	}
	
	
	public static float[] getFloatArrayScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getFloatArray(path);
	}
	
	
	public static long getLongScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getLong(path);
	}
	
	
	public static long[] getLongArrayScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getLongArray(path);
	}
	
	
	public static double getDoubleScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getDouble(path);
	}
	
	
	public static double[] getDoubleArrayScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getDoubleArray(path);
	}
	
	
	public static String getStringScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getString(path);
	}
	
	
	public static String[] getStringArrayScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getStringArray(path);
	}
	
	
	public static String[][] getStringArrayArrayScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getStringArrayArray(path);
	}
	
	
	public static transform[] getTransformArrayScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getTransformArray(path);
	}
	
	
	public static transform getTransformScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getTransform(path);
	}
	
	public static boolean getBooleanScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getBoolean(path);
	}
	
	
	public static boolean[] getBooleanArrayScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getBooleanArray(path);
	}
	
	
	public static location getLocationScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getLocation(path);
	}
	
	
	public static location[] getLocationArrayScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getLocationArray(path);
	}
	
	
	public static obj_id getObjIdScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getObjId(path);
	}
	
	
	public static obj_id[] getObjIdArrayScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getObjIdArray(path);
	}
	
	
	public static Vector getResizeableObjIdArrayScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getResizeableObjIdArray(path);
	}
	
	
	public static Vector getResizeableLocationArrayScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getResizeableLocationArray(path);
	}
	
	
	public static Vector getResizeableIntArrayScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getResizeableIntArray(path);
	}
	
	
	public static Vector getResizeableFloatArrayScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getResizeableFloatArray(path);
	}
	
	
	public static Vector getResizeableStringArrayScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getResizeableStringArray(path);
	}
	
	
	public static Vector getResizeableTransformArrayScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getResizeableTransformArray(path);
	}
	
	
	public static Vector getResizeableVectorArrayScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getResizeableVectorArray(path);
	}
	
	
	public static string_id getStringIdScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getStringId(path);
	}
	
	
	public static string_id[] getStringIdArrayScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getStringIdArray(path);
	}
	
	
	public static dictionary getDictionaryScriptVar(obj_id target, String path) throws InterruptedException
	{
		deltadictionary dd = target.getScriptVars();
		return dd.getDictionary(path);
	}
	
	
	public static boolean emptyContainer(obj_id target) throws InterruptedException
	{
		if ((target == null) || (target == obj_id.NULL_ID))
		{
			return false;
		}
		
		obj_id[] contents = getContents(target);
		if ((contents == null) || (contents.length == 0))
		{
			return false;
		}
		
		for (int i = 0; i < contents.length; i++)
		{
			testAbortScript();
			destroyObject(contents[i]);
		}
		
		return true;
	}
	
	
	public static boolean emptyContainerExceptStorytellerLoot(obj_id target) throws InterruptedException
	{
		if ((target == null) || (target == obj_id.NULL_ID))
		{
			return false;
		}
		
		obj_id[] contents = getContents(target);
		if ((contents == null) || (contents.length == 0))
		{
			return false;
		}
		
		for (int i = 0; i < contents.length; i++)
		{
			testAbortScript();
			obj_id contentsItem = contents[i];
			if (!utils.hasScriptVar(contentsItem, "storytellerLoot"))
			{
				destroyObject(contentsItem);
			}
		}
		
		return true;
	}
	
	
	public static location getLocationInArc(obj_id objPlayer, float fltStartAngle, float fltArcSize, float fltDistance) throws InterruptedException
	{
		
		location locHeading = getHeading(objPlayer);
		location locCenter = getLocation(objPlayer);
		
		locHeading.x = locHeading.x * fltDistance;
		locHeading.z = locHeading.z * fltDistance;
		
		locHeading.y = locCenter.y;
		locHeading.cell = locCenter.cell;
		locHeading.area = locCenter.area;
		
		float fltAngle = rand(fltStartAngle - fltArcSize, fltStartAngle + fltArcSize);
		
		location locNewHeading = rotatePointXZ(locHeading, fltAngle);
		
		locNewHeading.x = locCenter.x + locNewHeading.x;
		locNewHeading.z = locCenter.z + locNewHeading.z;
		
		return locNewHeading;
		
	}
	
	
	public static float getHeadingDegrees(obj_id objTarget) throws InterruptedException
	{
		location locStart = getLocation(objTarget);
		location locHeading = getHeading(objTarget);
		locHeading.x = locStart.x + locHeading.x;
		locHeading.z = locStart.z + locHeading.z;
		locHeading.y = locStart.y + locHeading.y;
		
		float fltAngle = thetaDegrees(locStart, locHeading);
		return fltAngle;
		
	}
	
	
	public static float thetaDegrees(location direction) throws InterruptedException
	{
		return (float) Math.toDegrees (Math.atan2 (direction.x, direction.z));
	}
	
	
	public static float thetaDegrees(location start, location end) throws InterruptedException
	{
		location direction = new location ();
		direction.x = end.x - start.x;
		direction.y = end.y - start.y;
		direction.z = end.z - start.z;
		
		return thetaDegrees (direction);
	}
	
	
	public static int countSubStringObjVars(obj_id[] objObjects, String strObjVar, String strSubString) throws InterruptedException
	{
		if (objObjects == null)
		{
			
			return 0;
		}
		int intCount = 0;
		for (int intI = 0; intI<objObjects.length; intI++)
		{
			testAbortScript();
			String strString = getStringObjVar(objObjects[intI], strObjVar);
			if (strString != null)
			{
				int intIndex = strString.indexOf(strSubString);
				if (intIndex>-1)
				{
					intCount = intCount +1;
				}
			}
		}
		return intCount;
	}
	
	
	public static dictionary addObjVarToDictionary(obj_var ov, dictionary d, String basePath) throws InterruptedException
	{
		if (ov == null || d == null)
		{
			return d;
		}
		
		if (basePath == null)
		{
			basePath = "";
		}
		
		Object dta = ov.getData();
		if (dta == null)
		{
			return d;
		}
		
		String name = ov.getName();
		String path = basePath + name;
		
		if (dta instanceof Integer)
		{
			int iVal = ov.getIntData();
			int iCur = d.getInt(path);
			
			iVal += iCur;
			
			d.put(path, iVal);
		}
		else if (dta instanceof Float)
		{
			float fVal = ov.getFloatData();
			float fCur = d.getFloat(path);
			
			fVal += fCur;
			
			d.put(path, fVal);
		}
		else if (dta instanceof int[])
		{
			int[] iaVal = ov.getIntArrayData();
			int[] iaCur = d.getIntArray(path);
			if ((iaCur == null) || (iaCur.length == 0))
			{
				d.put(path, iaVal);
			}
			else
			{
				if (iaVal.length == iaCur.length)
				{
					for (int z = 0; z < iaVal.length; z++)
					{
						testAbortScript();
						iaVal[z] += iaCur[z];
					}
					
					d.put(path, iaVal);
				}
			}
		}
		else if (dta instanceof float[])
		{
			float[] faVal = ov.getFloatArrayData();
			float[] faCur = d.getFloatArray(path);
			if ((faCur == null) || (faCur.length == 0))
			{
				d.put(path, faVal);
			}
			else
			{
				if (faVal.length == faCur.length)
				{
					for (int z = 0; z < faVal.length; z++)
					{
						testAbortScript();
						faVal[z] += faCur[z];
					}
					
					d.put(path, faVal);
				}
			}
		}
		
		return d;
	}
	
	
	public static dictionary addObjVarToDictionary(obj_var ov, dictionary d) throws InterruptedException
	{
		return addObjVarToDictionary(ov, d, null);
	}
	
	
	public static dictionary addObjVarListToDictionary(obj_var_list ovl, dictionary d, String basePath) throws InterruptedException
	{
		if (ovl == null || d == null)
		{
			return d;
		}
		
		if (basePath == null)
		{
			basePath = "";
		}
		
		String name = ovl.getName();
		String path = basePath + name;
		
		boolean litmus = true;
		
		int numItems = ovl.getNumItems();
		for (int i = 0; i < numItems; i++)
		{
			testAbortScript();
			obj_var ov = ovl.getObjVar(i);
			if (ov != null)
			{
				if (ov instanceof obj_var_list)
				{
					d = addObjVarListToDictionary((obj_var_list)(ov), d, path + ".");
				}
				else
				{
					d = addObjVarToDictionary(ov, d, path + ".");
				}
			}
		}
		
		return d;
	}
	
	
	public static dictionary addObjVarListToDictionary(obj_var_list ovl, dictionary d) throws InterruptedException
	{
		return addObjVarListToDictionary(ovl, d, null);
	}
	
	
	public static obj_id cloneObject(obj_id template, obj_id container) throws InterruptedException
	{
		String templateName = getTemplateName(template);
		obj_id clone = createObject(templateName, container, "");
		if (!isIdValid(clone))
		{
			return null;
		}
		
		copyObjectData(template, clone);
		return clone;
	}
	
	
	public static obj_id cloneObject(obj_id template, location loc) throws InterruptedException
	{
		String templateName = getTemplateName(template);
		obj_id clone = createObject(templateName, loc);
		if (!isIdValid(clone))
		{
			return null;
		}
		
		copyObjectData(template, clone);
		return clone;
	}
	
	
	public static void copyObjectData(obj_id template, obj_id clone) throws InterruptedException
	{
		
		obj_var_list ovl = getObjVarList(clone, "");
		
		if (ovl == null)
		{
			return;
		}
		
		int numItems = ovl.getNumItems();
		for (int x = 0; x < numItems; x++)
		{
			testAbortScript();
			obj_var ov = ovl.getObjVar(x);
			if (ov != null)
			{
				copyObjVar(template, clone, ov.getName());
			}
		}
	}
	
	
	public static boolean copyObjVar(obj_id target, String basePath, obj_var ov) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		
		if (basePath == null)
		{
			basePath = "";
		}
		
		if (ov == null)
		{
			return false;
		}
		
		String name = ov.getName();
		String path = basePath + name;
		
		Object dta = ov.getData();
		if (dta instanceof Integer)
		{
			int val = ov.getIntData();
			return setObjVar(target, path, val);
		}
		else if (dta instanceof int[])
		{
			int[] val = ov.getIntArrayData();
			return setObjVar(target, path, val);
		}
		else if (dta instanceof Float)
		{
			float val = ov.getFloatData();
			return setObjVar(target, path, val);
		}
		else if (dta instanceof float[])
		{
			float[] val = ov.getFloatArrayData();
			return setObjVar(target, path, val);
		}
		else if (dta instanceof Boolean)
		{
			boolean val = ov.getBooleanData();
			return setObjVar(target, path, val);
		}
		else if (dta instanceof Boolean[])
		{
			boolean[] val = ov.getBooleanArrayData();
			return setObjVar(target, path, val);
		}
		else if (dta instanceof String)
		{
			String val = ov.getStringData();
			return setObjVar(target, path, val);
		}
		else if (dta instanceof String[])
		{
			String[] val = ov.getStringArrayData();
			return setObjVar(target, path, val);
		}
		else if (dta instanceof obj_id)
		{
			obj_id val = ov.getObjIdData();
			return setObjVar(target, path, val);
		}
		else if (dta instanceof obj_id[])
		{
			obj_id[] val = ov.getObjIdArrayData();
			return setObjVar(target, path, val);
		}
		else if (dta instanceof location)
		{
			location val = ov.getLocationData();
			return setObjVar(target, path, val);
		}
		else if (dta instanceof location[])
		{
			location[] val = ov.getLocationArrayData();
			return setObjVar(target, path, val);
		}
		else if (dta instanceof string_id)
		{
			string_id val = ov.getStringIdData();
			return setObjVar(target, path, val);
		}
		else if (dta instanceof string_id[])
		{
			string_id[] val = ov.getStringIdArrayData();
			return setObjVar(target, path, val);
		}
		else if (dta instanceof attrib_mod)
		{
			attrib_mod val = ov.getAttribModData();
			return setObjVar(target, path, val);
		}
		else if (dta instanceof attrib_mod[])
		{
			attrib_mod[] val = ov.getAttribModArrayData();
			return setObjVar(target, path, val);
		}
		
		return false;
	}
	
	
	public static boolean[] removeObjVarList(obj_id object, String[] objVarList) throws InterruptedException
	{
		boolean[] result = new boolean[objVarList.length];
		
		for (int i=0; i<objVarList.length; i++)
		{
			testAbortScript();
			if (!hasObjVar(object, objVarList[i]))
			{
				result[i] = false;
			}
			else
			{
				removeObjVar(object, objVarList[i]);
				result[i] = true;
			}
		}
		
		if (result == null || result.length == 0)
		{
			result[0] = false;
		}
		
		return result;
	}
	
	
	public static boolean saveDictionaryAsObjVar(obj_id object, String rootName, dictionary dict) throws InterruptedException
	{
		if (!isIdValid(object) || dict == null)
		{
			return false;
		}
		
		if (rootName == null)
		{
			rootName = "";
		}
		else if (rootName != null && rootName.length() > 0)
		{
			if (!Character.isLetterOrDigit(rootName.charAt(0)))
			{
				return false;
			}
			if (rootName.charAt(rootName.length() - 1) != '.')
			{
				rootName = rootName + ".";
			}
		}
		
		Enumeration keys = dict.keys();
		while (keys.hasMoreElements())
		{
			testAbortScript();
			String key = null;
			try
			{
				key = (String)keys.nextElement();
			}
			catch ( ClassCastException err )
			{
				return false;
			}
			Object data = dict.get(key);
			if (data instanceof Integer)
			{
				
				setObjVar(object, rootName + key, ((Integer)data).intValue());
			}
			else if (data instanceof Float)
			{
				
				setObjVar(object, rootName + key, ((Float)data).floatValue());
			}
			else if (data instanceof String)
			{
				
				setObjVar(object, rootName + key, (String)data);
			}
			else if (data instanceof int[])
			{
				
				setObjVar(object, rootName + key, (int[])data);
			}
			else if (data instanceof float[])
			{
				
				setObjVar(object, rootName + key, (float[])data);
			}
			else if (data instanceof dictionary)
			{
				
				saveDictionaryAsObjVar(object, rootName + key, (dictionary)data);
			}
			else
			{
				
			}
		}
		return true;
	}
	
	
	public static String getFactionSubString(String strSearchString) throws InterruptedException
	{
		if (strSearchString == null)
		{
			return null;
		}
		String strTestString = toLower(strSearchString);
		final String[] FACTION_SEARCH_STRINGS =
		{
			factions.FACTION_IMPERIAL, factions.FACTION_REBEL
		};
		for (int intI = 0; intI<FACTION_SEARCH_STRINGS.length; intI++)
		{
			testAbortScript();
			
			int intIndex = strTestString.indexOf(toLower(FACTION_SEARCH_STRINGS[intI]));
			if (intIndex>-1)
			{
				return FACTION_SEARCH_STRINGS[intI];
			}
			
		}
		return null;
	}
	
	
	public static boolean setBatchObjVar(obj_id target, String base_path, Object[] array) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		
		if ((base_path == null) || (base_path.equals("")))
		{
			return false;
		}
		
		if ((array == null) || (array.length == 0))
		{
			return false;
		}
		
		if (hasObjVar(target, base_path))
		{
			removeObjVar(target, base_path);
		}
		
		int BatchSize = 10;
		
		boolean litmus = true;
		int n = 0;
		Vector toSet = new Vector();
		for (int i = 0; i < array.length; i++)
		{
			testAbortScript();
			toSet.add(array[i]);
			if (toSet.size() >= BatchSize)
			{
				litmus &= setObjectArrayObjVar(target, base_path + "."+ n, toSet);
				toSet.clear();
				n++;
			}
		}
		
		litmus &= setObjectArrayObjVar(target, base_path + "."+ n, toSet);
		
		return litmus;
	}
	
	
	public static void removeBatchObjVar(obj_id target, String base_path) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return;
		}
		
		if ((base_path == null) || (base_path.equals("")))
		{
			return;
		}
		
		if (!hasObjVar(target, base_path))
		{
			return;
		}
		
		if (hasObjVar(target, base_path))
		{
			removeObjVar(target, base_path);
		}
		
		obj_var_list list = getObjVarList(target, null);
		
		if (list == null)
		{
			return;
		}
		
		int listLength = list.getNumItems();
		int intQ = 0;
		
		for (int i=0; i<listLength; i++)
		{
			testAbortScript();
			if (hasObjVar(target, base_path+"."+intQ))
			{
				removeObjVar(target, base_path+"."+intQ);
				intQ++;
			}
			
		}
		
	}
	
	
	public static boolean setBatchObjVar(obj_id target, String base_path, int[] array) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		
		if ((base_path == null) || (base_path.equals("")))
		{
			return false;
		}
		
		if ((array == null) || (array.length == 0))
		{
			return false;
		}
		
		if (hasObjVar(target, base_path))
		{
			removeObjVar(target, base_path);
		}
		
		int BatchSize = 10;
		
		boolean litmus = true;
		int n = 0;
		Vector toSet = new Vector();
		for (int i = 0; i < array.length; i++)
		{
			testAbortScript();
			toSet.add(new Integer(array[i]));
			if (toSet.size() >= BatchSize)
			{
				litmus &= setObjectArrayObjVar(target, base_path + "."+ n, toSet);
				toSet.clear();
				n++;
			}
		}
		
		litmus &= setObjectArrayObjVar(target, base_path + "."+ n, toSet);
		
		return litmus;
	}
	
	
	public static boolean setResizeableBatchObjVar(obj_id target, String base_path, Vector vector) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		
		if ((base_path == null) || (base_path.equals("")))
		{
			
			return false;
		}
		
		if ((vector == null) || (vector.size() == 0))
		{
			
			return false;
		}
		
		if (hasObjVar(target, base_path))
		{
			removeObjVar(target, base_path);
		}
		
		int BatchSize = 10;
		
		boolean litmus = true;
		int n = 0;
		Vector toSet = new Vector();
		for (int i = 0; i < vector.size(); i++)
		{
			testAbortScript();
			toSet.add(vector.get(i));
			if (toSet.size() >= BatchSize)
			{
				litmus &= setObjectArrayObjVar(target, base_path + "."+ n, toSet);
				toSet.clear();
				n++;
			}
		}
		
		litmus &= setObjectArrayObjVar(target, base_path + "."+ n, toSet);
		
		return litmus;
	}
	
	
	public static boolean setObjectArrayObjVar(obj_id target, String path, Vector vec) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		
		if ((path == null) || (path.equals("")))
		{
			return false;
		}
		
		if ((vec == null) || (vec.size() == 0))
		{
			return false;
		}
		
		int size = vec.size();
		
		Object test = vec.elementAt(0);
		
		if (test instanceof String)
		{
			String[] tmp = new String[size];
			vec.toArray(tmp);
			return setObjVar(target, path, tmp);
		}
		else if (test instanceof string_id)
		{
			string_id[] tmp = new string_id[size];
			vec.toArray(tmp);
			return setObjVar(target, path, tmp);
		}
		else if (test instanceof obj_id)
		{
			obj_id[] tmp = new obj_id[size];
			vec.toArray(tmp);
			return setObjVar(target, path, tmp);
		}
		else if (test instanceof attrib_mod)
		{
			attrib_mod[] tmp = new attrib_mod[size];
			vec.toArray(tmp);
			return setObjVar(target, path, tmp);
		}
		else if (test instanceof location)
		{
			location[] tmp = new location[size];
			vec.toArray(tmp);
			return setObjVar(target, path, tmp);
		}
		else if (test instanceof Integer)
		{
			int[] tmp = new int[size];
			for (int i = 0; i < vec.size(); i ++)
			{
				testAbortScript();
				tmp[i] = ((Integer)(vec.get(i))).intValue();
			}
			return setObjVar(target, path, tmp);
		}
		
		return false;
	}
	
	
	public static String[] getStringBatchObjVar(obj_id target, String base_path) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return null;
		}
		
		if ((base_path == null) || (base_path.equals("")))
		{
			return null;
		}
		
		if (!hasObjVar(target, base_path))
		{
			return null;
		}
		
		Vector ret = new Vector();
		ret.setSize(0);
		obj_var_list ovl = getObjVarList(target, base_path);
		if ((ovl == null) || (ovl.getNumItems() == 0))
		{
			return null;
		}
		
		int numItems = ovl.getNumItems();
		for (int i = 0; i < numItems; i++)
		{
			testAbortScript();
			String[] tmp = getStringArrayObjVar(target, base_path + "."+ i);
			if ((tmp != null) && (tmp.length > 0))
			{
				ret = concatArrays(ret, tmp);
			}
		}
		
		if ((ret != null) && (ret.size() > 0))
		{
			return (String[])ret.toArray(new String[0]);
		}
		
		return null;
	}
	
	
	public static Vector getResizeableStringBatchObjVar(obj_id target, String base_path) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return null;
		}
		
		if ((base_path == null) || (base_path.equals("")))
		{
			return null;
		}
		
		if (!hasObjVar(target, base_path))
		{
			return null;
		}
		
		Vector ret = new Vector();
		ret.setSize(0);
		obj_var_list ovl = getObjVarList(target, base_path);
		if ((ovl == null) || (ovl.getNumItems() == 0))
		{
			return null;
		}
		
		int numItems = ovl.getNumItems();
		for (int i = 0; i < numItems; i++)
		{
			testAbortScript();
			String[] tmp = getStringArrayObjVar(target, base_path + "."+ i);
			if ((tmp != null) && (tmp.length > 0))
			{
				ret = concatArrays(ret, tmp);
			}
		}
		
		if ((ret != null) && (ret.size() > 0))
		{
			return ret;
		}
		
		return null;
	}
	
	
	public static string_id[] getStringIdBatchObjVar(obj_id target, String base_path) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return null;
		}
		
		if ((base_path == null) || (base_path.equals("")))
		{
			return null;
		}
		
		if (!hasObjVar(target, base_path))
		{
			return null;
		}
		
		Vector ret = new Vector();
		ret.setSize(0);
		obj_var_list ovl = getObjVarList(target, base_path);
		if ((ovl == null) || (ovl.getNumItems() == 0))
		{
			return null;
		}
		
		int numItems = ovl.getNumItems();
		for (int i = 0; i < numItems; i++)
		{
			testAbortScript();
			string_id[] tmp = getStringIdArrayObjVar(target, base_path + "."+ i);
			if ((tmp != null) && (tmp.length > 0))
			{
				ret = concatArrays(ret, tmp);
			}
		}
		
		if ((ret != null) && (ret.size() > 0))
		{
			return (string_id[])ret.toArray(new string_id[0]);
		}
		
		return null;
	}
	
	
	public static obj_id[] getObjIdBatchObjVar(obj_id target, String base_path) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return null;
		}
		
		if ((base_path == null) || (base_path.equals("")))
		{
			return null;
		}
		
		if (!hasObjVar(target, base_path))
		{
			return null;
		}
		
		Vector ret = new Vector();
		ret.setSize(0);
		obj_var_list ovl = getObjVarList(target, base_path);
		if ((ovl == null) || (ovl.getNumItems() == 0))
		{
			return null;
		}
		
		int numItems = ovl.getNumItems();
		for (int i = 0; i < numItems; i++)
		{
			testAbortScript();
			obj_id[] tmp = getObjIdArrayObjVar(target, base_path + "."+ i);
			if ((tmp != null) && (tmp.length > 0))
			{
				ret = concatArrays(ret, tmp);
			}
		}
		
		if ((ret != null) && (ret.size() > 0))
		{
			obj_id[] _ret = new obj_id[0];
			if (ret != null)
			{
				_ret = new obj_id[ret.size()];
				ret.toArray(_ret);
			}
			return _ret;
		}
		
		return null;
	}
	
	
	public static Vector getResizeableObjIdBatchObjVar(obj_id target, String base_path) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return null;
		}
		
		if ((base_path == null) || (base_path.equals("")))
		{
			return null;
		}
		
		if (!hasObjVar(target, base_path))
		{
			return null;
		}
		
		Vector ret = new Vector();
		ret.setSize(0);
		obj_var_list ovl = getObjVarList(target, base_path);
		if ((ovl == null) || (ovl.getNumItems() == 0))
		{
			return null;
		}
		
		int numItems = ovl.getNumItems();
		for (int i = 0; i < numItems; i++)
		{
			testAbortScript();
			obj_id[] tmp = getObjIdArrayObjVar(target, base_path + "."+ i);
			if ((tmp != null) && (tmp.length > 0))
			{
				ret = concatArrays(ret, tmp);
			}
		}
		
		if ((ret != null) && (ret.size() > 0))
		{
			return ret;
		}
		
		return null;
	}
	
	
	public static int[] getIntBatchObjVar(obj_id target, String base_path) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return null;
		}
		
		if ((base_path == null) || (base_path.equals("")))
		{
			return null;
		}
		
		if (!hasObjVar(target, base_path))
		{
			return null;
		}
		
		Vector ret = new Vector();
		ret.setSize(0);
		obj_var_list ovl = getObjVarList(target, base_path);
		if ((ovl == null) || (ovl.getNumItems() == 0))
		{
			return null;
		}
		
		int numItems = ovl.getNumItems();
		for (int i = 0; i < numItems; i++)
		{
			testAbortScript();
			int[] tmp = getIntArrayObjVar(target, base_path + "."+ i);
			if ((tmp != null) && (tmp.length > 0))
			{
				ret = concatArrays(ret, tmp);
			}
		}
		
		if ((ret != null) && (ret.size() > 0))
		{
			int[] _ret = new int[0];
			if (ret != null)
			{
				_ret = new int[ret.size()];
				for (int _i = 0; _i < ret.size(); ++_i)
				{
					_ret[_i] = ((Integer)ret.get(_i)).intValue();
				}
			}
			return _ret;
		}
		
		return null;
	}
	
	
	public static Vector getResizeableIntBatchObjVar(obj_id target, String base_path) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return null;
		}
		
		if ((base_path == null) || (base_path.equals("")))
		{
			return null;
		}
		
		if (!hasObjVar(target, base_path))
		{
			return null;
		}
		
		Vector ret = new Vector();
		ret.setSize(0);
		obj_var_list ovl = getObjVarList(target, base_path);
		if ((ovl == null) || (ovl.getNumItems() == 0))
		{
			return null;
		}
		
		int numItems = ovl.getNumItems();
		for (int i = 0; i < numItems; i++)
		{
			testAbortScript();
			int[] tmp = getIntArrayObjVar(target, base_path + "."+ i);
			if ((tmp != null) && (tmp.length > 0))
			{
				ret = concatArrays(ret, tmp);
			}
		}
		
		if ((ret != null) && (ret.size() > 0))
		{
			return ret;
		}
		
		return null;
	}
	
	
	public static attrib_mod[] getAttribModBatchObjVar(obj_id target, String base_path) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return null;
		}
		
		if ((base_path == null) || (base_path.equals("")))
		{
			return null;
		}
		
		if (!hasObjVar(target, base_path))
		{
			return null;
		}
		
		obj_var_list ovl = getObjVarList(target, base_path);
		if ((ovl == null) || (ovl.getNumItems() == 0))
		{
			return null;
		}
		
		Vector ret = new Vector();
		ret.setSize(0);
		int numItems = ovl.getNumItems();
		for (int i = 0; i < numItems; i++)
		{
			testAbortScript();
			attrib_mod[] tmp = getAttribModArrayObjVar(target, base_path + "."+ i);
			if ((tmp != null) && (tmp.length > 0))
			{
				ret = concatArrays(ret, tmp);
			}
		}
		
		if ((ret != null) && (ret.size() > 0))
		{
			return (attrib_mod[])ret.toArray(new attrib_mod[0]);
		}
		
		return null;
	}
	
	
	public static location[] getLocationBatchObjVar(obj_id target, String base_path) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return null;
		}
		
		if ((base_path == null) || (base_path.equals("")))
		{
			return null;
		}
		
		if (!hasObjVar(target, base_path))
		{
			return null;
		}
		
		Vector ret = new Vector();
		ret.setSize(0);
		obj_var_list ovl = getObjVarList(target, base_path);
		if ((ovl == null) || (ovl.getNumItems() == 0))
		{
			return null;
		}
		
		int numItems = ovl.getNumItems();
		for (int i = 0; i < numItems; i++)
		{
			testAbortScript();
			location[] tmp = getLocationArrayObjVar(target, base_path + "."+ i);
			if ((tmp != null) && (tmp.length > 0))
			{
				ret = concatArrays(ret, tmp);
			}
		}
		
		if ((ret != null) && (ret.size() > 0))
		{
			return (location[])ret.toArray(new location[0]);
		}
		
		return null;
	}
	
	
	public static boolean hasStringBatchObjVar(obj_id target, String base_path) throws InterruptedException
	{
		return getStringBatchObjVar(target, base_path) != null;
	}
	
	
	public static boolean hasResizeableStringBatchObjVar(obj_id target, String base_path) throws InterruptedException
	{
		return getResizeableStringBatchObjVar(target, base_path) != null;
	}
	
	
	public static boolean hasStringIdBatchObjVar(obj_id target, String base_path) throws InterruptedException
	{
		return getStringIdBatchObjVar(target, base_path) != null;
	}
	
	
	public static boolean hasObjIdBatchObjVar(obj_id target, String base_path) throws InterruptedException
	{
		return getObjIdBatchObjVar(target, base_path) != null;
	}
	
	
	public static boolean hasResizeableObjIdBatchObjVar(obj_id target, String base_path) throws InterruptedException
	{
		return getResizeableObjIdBatchObjVar(target, base_path) != null;
	}
	
	
	public static boolean hasIntBatchObjVar(obj_id target, String base_path) throws InterruptedException
	{
		return getIntBatchObjVar(target, base_path) != null;
	}
	
	
	public static boolean hasResizeableIntBatchObjVar(obj_id target, String base_path) throws InterruptedException
	{
		return getResizeableIntBatchObjVar(target, base_path) != null;
	}
	
	
	public static boolean hasAttribModBatchObjVar(obj_id target, String base_path) throws InterruptedException
	{
		return getAttribModBatchObjVar(target, base_path) != null;
	}
	
	
	public static boolean hasLocationBatchObjVar(obj_id target, String base_path) throws InterruptedException
	{
		return getLocationBatchObjVar(target, base_path) != null;
	}
	
	
	public static boolean setBatchScriptVar(obj_id target, String base_path, Vector array) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		
		if ((base_path == null) || (base_path.equals("")))
		{
			return false;
		}
		
		if ((array == null) || (array.size() == 0))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		
		int BatchSize = 10;
		
		int n = 0;
		Vector toSet = new Vector();
		for (int i = 0; i < array.size(); i++)
		{
			testAbortScript();
			toSet.add(array.elementAt(i));
			if (toSet.size() >= BatchSize)
			{
				dd.put(base_path + "."+ n, toSet);
				toSet.clear();
				n++;
			}
		}
		
		if (toSet.size() > 0)
		{
			dd.put(base_path + "."+ n, toSet);
			n++;
		}
		
		dd.put(base_path, n);
		return true;
	}
	
	
	public static boolean setBatchScriptVar(obj_id target, String base_path, obj_id[] array) throws InterruptedException
	{
		if ((array == null) || (array.length == 0))
		{
			return false;
		}
		
		Vector vArray = new Vector(Arrays.asList(array));
		return setBatchScriptVar(target, base_path, vArray);
	}
	
	
	public static boolean setBatchScriptVar(obj_id target, String base_path, String[] array) throws InterruptedException
	{
		if ((array == null) || (array.length == 0))
		{
			return false;
		}
		
		Vector vArray = new Vector(Arrays.asList(array));
		return setBatchScriptVar(target, base_path, vArray);
	}
	
	
	public static boolean setBatchScriptVar(obj_id target, String base_path, string_id[] array) throws InterruptedException
	{
		if ((array == null) || (array.length == 0))
		{
			return false;
		}
		
		Vector vArray = new Vector(Arrays.asList(array));
		return setBatchScriptVar(target, base_path, vArray);
	}
	
	
	public static boolean setBatchScriptVar(obj_id target, String base_path, location[] array) throws InterruptedException
	{
		if ((array == null) || (array.length == 0))
		{
			return false;
		}
		
		Vector vArray = new Vector(Arrays.asList(array));
		return setBatchScriptVar(target, base_path, vArray);
	}
	
	
	public static Vector getResizeableObjIdBatchScriptVar(obj_id target, String base_path) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return null;
		}
		
		if ((base_path == null) || (base_path.equals("")))
		{
			return null;
		}
		
		if (!hasScriptVar(target, base_path))
		{
			return null;
		}
		
		int cnt = getIntScriptVar(target, base_path);
		deltadictionary dd = target.getScriptVars();
		
		Vector ret = new Vector();
		for (int i = 0; i < cnt; i++)
		{
			testAbortScript();
			Vector tmp = dd.getResizeableObjIdArray(base_path + "."+ i);
			if ((tmp != null) && (tmp.size() > 0))
			{
				ret.addAll(tmp);
			}
		}
		
		if ((ret != null) && (ret.size() > 0))
		{
			return ret;
		}
		
		return null;
	}
	
	
	public static obj_id[] getObjIdBatchScriptVar(obj_id target, String base_path) throws InterruptedException
	{
		Vector ret = getResizeableObjIdBatchScriptVar(target, base_path);
		if ((ret != null) && (ret.size() > 0))
		{
			return toStaticObjIdArray(ret);
		}
		
		return null;
	}
	
	
	public static Vector getResizeableStringBatchScriptVar(obj_id target, String base_path) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return null;
		}
		
		if ((base_path == null) || (base_path.equals("")))
		{
			return null;
		}
		
		if (!hasScriptVar(target, base_path))
		{
			return null;
		}
		
		int cnt = getIntScriptVar(target, base_path);
		deltadictionary dd = target.getScriptVars();
		
		Vector ret = new Vector();
		for (int i = 0; i < cnt; i++)
		{
			testAbortScript();
			Vector tmp = dd.getResizeableStringArray(base_path + "."+ i);
			if ((tmp != null) && (tmp.size() > 0))
			{
				ret.addAll(tmp);
			}
		}
		
		if ((ret != null) && (ret.size() > 0))
		{
			return ret;
		}
		
		return null;
	}
	
	
	public static String[] getStringBatchScriptVar(obj_id target, String base_path) throws InterruptedException
	{
		Vector ret = getResizeableStringBatchScriptVar(target, base_path);
		if ((ret != null) && (ret.size() > 0))
		{
			return toStaticStringArray(ret);
		}
		
		return null;
	}
	
	
	public static Vector getResizeableLocationBatchScriptVar(obj_id target, String base_path) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return null;
		}
		
		if ((base_path == null) || (base_path.equals("")))
		{
			return null;
		}
		
		if (!hasScriptVar(target, base_path))
		{
			return null;
		}
		
		int cnt = getIntScriptVar(target, base_path);
		deltadictionary dd = target.getScriptVars();
		
		Vector ret = new Vector();
		for (int i = 0; i < cnt; i++)
		{
			testAbortScript();
			Vector tmp = dd.getResizeableLocationArray(base_path + "."+ i);
			if ((tmp != null) && (tmp.size() > 0))
			{
				ret.addAll(tmp);
			}
		}
		
		if ((ret != null) && (ret.size() > 0))
		{
			return ret;
		}
		
		return null;
	}
	
	
	public static Vector getResizeableIntBatchScriptVar(obj_id target, String base_path) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return null;
		}
		
		if ((base_path == null) || (base_path.equals("")))
		{
			return null;
		}
		
		if (!hasScriptVar(target, base_path))
		{
			return null;
		}
		
		int cnt = getIntScriptVar(target, base_path);
		deltadictionary dd = target.getScriptVars();
		
		Vector ret = new Vector();
		for (int i = 0; i < cnt; i++)
		{
			testAbortScript();
			Vector tmp = dd.getResizeableIntArray(base_path + "."+ i);
			if ((tmp != null) && (tmp.size() > 0))
			{
				ret.addAll(tmp);
			}
		}
		
		if ((ret != null) && (ret.size() > 0))
		{
			return ret;
		}
		
		return null;
	}
	
	
	public static location[] getLocationBatchScriptVar(obj_id target, String base_path) throws InterruptedException
	{
		Vector ret = getResizeableLocationBatchScriptVar(target, base_path);
		if ((ret != null) && (ret.size() > 0))
		{
			return toStaticLocationArray(ret);
		}
		
		return null;
	}
	
	
	public static int[] getIntBatchScriptVar(obj_id target, String base_path) throws InterruptedException
	{
		Vector ret = getResizeableIntBatchScriptVar(target, base_path);
		if ((ret != null) && (ret.size() > 0))
		{
			int[] _ret = new int[0];
			if (ret != null)
			{
				_ret = new int[ret.size()];
				for (int _i = 0; _i < ret.size(); ++_i)
				{
					_ret[_i] = ((Integer)ret.get(_i)).intValue();
				}
			}
			return _ret;
		}
		
		return null;
	}
	
	
	public static void removeBatchScriptVar(obj_id target, String base_path) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return;
		}
		
		if ((base_path == null) || (base_path.equals("")))
		{
			return;
		}
		
		if (!hasScriptVar(target, base_path))
		{
			return;
		}
		
		int cnt = getIntScriptVar(target, base_path);
		deltadictionary dd = target.getScriptVars();
		dd.remove(base_path);
		
		for (int i = 0; i < cnt; i++)
		{
			testAbortScript();
			dd.remove(base_path + "."+ i);
		}
	}
	
	
	public static boolean hasResizeableObjIdBatchScriptVar(obj_id target, String base_path) throws InterruptedException
	{
		return getResizeableObjIdBatchScriptVar(target, base_path) != null;
	}
	
	
	public static boolean hasObjIdBatchScriptVar(obj_id target, String base_path) throws InterruptedException
	{
		return getObjIdBatchScriptVar(target, base_path) != null;
	}
	
	
	public static boolean hasResizeableStringBatchScriptVar(obj_id target, String base_path) throws InterruptedException
	{
		return getResizeableStringBatchScriptVar(target, base_path) != null;
	}
	
	
	public static boolean hasStringBatchScriptVar(obj_id target, String base_path) throws InterruptedException
	{
		return getStringBatchScriptVar(target, base_path) != null;
	}
	
	
	public static boolean hasResizeableLocationBatchScriptVar(obj_id target, String base_path) throws InterruptedException
	{
		return getResizeableLocationBatchScriptVar(target, base_path) != null;
	}
	
	
	public static boolean hasResizeableIntBatchScriptVar(obj_id target, String base_path) throws InterruptedException
	{
		return getResizeableIntBatchScriptVar(target, base_path) != null;
	}
	
	
	public static boolean hasLocationBatchScriptVar(obj_id target, String base_path) throws InterruptedException
	{
		return getLocationBatchScriptVar(target, base_path) != null;
	}
	
	
	public static boolean hasIntBatchScriptVar(obj_id target, String base_path) throws InterruptedException
	{
		return getIntBatchScriptVar(target, base_path) != null;
	}
	
	
	public static int getIntConfigSetting(String section, String key) throws InterruptedException
	{
		String setting = getConfigSetting(section, key);
		if (setting == null || setting.length() == 0)
		{
			return 0;
		}
		
		try
		{
			return Integer.parseInt(setting);
		}
		catch ( NumberFormatException err)
		{
		}
		return 0;
	}
	
	
	public static float getFloatConfigSetting(String section, String key) throws InterruptedException
	{
		String setting = getConfigSetting(section, key);
		if (setting == null || setting.length() == 0)
		{
			return 0;
		}
		
		try
		{
			return Float.parseFloat(setting);
		}
		catch ( NumberFormatException err)
		{
		}
		return 0;
	}
	
	
	public static boolean checkConfigFlag(String strSection, String strConfigSetting) throws InterruptedException
	{
		
		String strTest = getConfigSetting(strSection, strConfigSetting);
		if (strTest != null)
		{
			strTest = toLower(strTest);
			if (strTest.equals("true"))
			{
				return true;
				
			}
			if (strTest.equals("1"))
			{
				return true;
				
			}
			return false;
			
		}
		else
		{
			return false;
		}
		
	}
	
	public static boolean checkServerSpawnLimits() throws InterruptedException
	{
		final int intServerSpawnLimit = getServerSpawnLimit();
		final int intNumCreatures = utils.getNumCreaturesForSpawnLimit();
		final int intNumPlayers = getNumPlayers();
		if (intNumPlayers > 0)
		{
			
			if (intServerSpawnLimit > 0)
			{
				
				if (intNumCreatures > intServerSpawnLimit)
				{
					return false;
					
				}
			}
			else
			{
				if (intNumPlayers < 200000)
				{
					
					if (intNumCreatures > 5000)
					{
						return false;
						
					}
					
				}
				else
				{
					float fltRatio = (float)(intNumCreatures/intNumPlayers);
					
					if (fltRatio > 10)
					{
						return false;
						
					}
				}
			}
		}
		return true;
	}
	
	
	public static String formatTimeVerbose(int seconds) throws InterruptedException
	{
		if (seconds < 1)
		{
			return "";
		}
		
		int[] convert_time = player_structure.convertSecondsTime(seconds);
		
		int idx = 0;
		for (int i = 0; i < convert_time.length; i++)
		{
			testAbortScript();
			if (convert_time[i] > 0)
			{
				idx = i;
				break;
			}
		}
		
		if (idx == convert_time.length - 1)
		{
			idx--;
		}
		
		String time_str = "";
		String[] verboseIndex = 
		{
			" days ",
			" hours ",
			" minutes ",
			" seconds "
		};
		
		for (int n = idx; n < convert_time.length; n++)
		{
			testAbortScript();
			
			time_str += convert_time[n]+verboseIndex[n];
			
			if (n < convert_time.length - 1)
			{
				time_str += ", ";
			}
		}
		
		if (time_str != null && !time_str.equals(""))
		{
			return time_str;
		}
		
		return "";
	}
	
	
	public static String formatTimeVerboseNoSpaces(int seconds) throws InterruptedException
	{
		if (seconds < 1)
		{
			return "";
		}
		
		int[] convert_time = player_structure.convertSecondsTime(seconds);
		
		int idx = 0;
		for (int i = 0; i < convert_time.length; i++)
		{
			testAbortScript();
			if (convert_time[i] > 0)
			{
				idx = i;
				break;
			}
		}
		
		if (idx == convert_time.length - 1)
		{
			idx--;
		}
		
		String time_str = "";
		String[] verboseIndex = 
		{
			" days",
			" hours",
			" minutes",
			" seconds"
		};
		
		for (int n = idx; n < convert_time.length; n++)
		{
			testAbortScript();
			
			time_str += convert_time[n]+verboseIndex[n];
			
			if (n < convert_time.length - 1)
			{
				time_str += ", ";
			}
		}
		
		if (time_str != null && !time_str.equals(""))
		{
			return time_str;
		}
		
		return "";
	}
	
	
	public static String formatTime(int seconds) throws InterruptedException
	{
		if (seconds < 1)
		{
			return "";
		}
		
		int[] convert_time = player_structure.convertSecondsTime(seconds);
		
		int idx = 0;
		for (int i = 0; i < convert_time.length; i++)
		{
			testAbortScript();
			if (convert_time[i] > 0)
			{
				idx = i;
				break;
			}
		}
		
		if (idx == convert_time.length - 1)
		{
			idx--;
		}
		
		String time_str = "";
		for (int n = idx; n < convert_time.length; n++)
		{
			testAbortScript();
			
			time_str += convert_time[n];
			
			if (n < convert_time.length - 1)
			{
				time_str += ":";
			}
		}
		
		if (time_str != null && !time_str.equals(""))
		{
			return time_str;
		}
		
		return "";
	}
	
	
	public static String formatTime(float fTime) throws InterruptedException
	{
		int iTime = (int)fTime;
		
		float diff = fTime - iTime;
		int decimals = (int)(diff*100);
		
		if (iTime > 0)
		{
			String sTime = formatTime(iTime);
			sTime += "."+ decimals;
			return sTime;
		}
		else
		{
			return "0." + decimals;
		}
	}
	
	
	public static String padTimeDHMS(int seconds) throws InterruptedException
	{
		if (seconds < 0)
		{
			return null;
		}
		
		int days = seconds / (3600 * 24);
		
		String daysText = ""+ days;
		
		if (daysText.length() <= 4)
		{
			daysText = "0000".substring(0, 4 - daysText.length()) + daysText;
		}
		
		seconds = seconds % (3600 * 24);
		
		int hours = seconds / 3600;
		
		String hoursText = ""+ hours;
		
		if (hoursText.length() <= 2)
		{
			hoursText = "00".substring(0,2 - hoursText.length()) + hoursText;
		}
		
		seconds = seconds % 3600;
		
		int minutes = seconds / 60;
		
		String minutesText = ""+ minutes;
		
		if (minutesText.length() <= 2)
		{
			minutesText = "00".substring(0,2 - minutesText.length()) + minutesText;
		}
		
		seconds = seconds % 60;
		
		String secondsText = ""+ seconds;
		
		if (secondsText.length() <= 2)
		{
			secondsText = "00".substring(0,2 - secondsText.length()) + secondsText;
		}
		
		return daysText + "d:" + hoursText + "h:" + minutesText + "m:" + secondsText + "s";
	}
	
	
	public static String padTimeHM(int seconds) throws InterruptedException
	{
		seconds = seconds % (3600 * 24);
		
		int hours = seconds / 3600;
		
		String hoursText = ""+ hours;
		
		if (hoursText.length() <= 2)
		{
			hoursText = "00".substring(0,2 - hoursText.length()) + hoursText;
		}
		
		seconds = seconds % 3600;
		
		int minutes = seconds / 60;
		
		String minutesText = ""+ minutes;
		
		if (minutesText.length() <= 2)
		{
			minutesText = "00".substring(0,2 - minutesText.length()) + minutesText;
		}
		
		return hoursText + "h:" + minutesText + "m";
	}
	
	
	public static void sendDelayedSystemMessage(obj_id target, string_id sid, float delay) throws InterruptedException
	{
		if (!isIdValid(target) || (sid == null) || (delay < 0f))
		{
			return;
		}
		
		dictionary msg = new dictionary();
		msg.put("sidMsg", sid);
		
		messageTo(target, "handleDelayedSystemMessage", msg, delay, false);
	}
	
	
	public static void sendDelayedSystemMessage(obj_id target, String sid, float delay) throws InterruptedException
	{
		if (!isIdValid(target) || (sid == null) || sid.equals("") || (delay < 0f))
		{
			return;
		}
		
		dictionary msg = new dictionary();
		msg.put("stringMsg", sid);
		
		messageTo(target, "handleDelayedSystemMessage", msg, delay, false);
	}
	
	
	public static void sendDelayedProseMessage(obj_id msgTarget, string_id sid, obj_id actor, String actorString, string_id actorStringId, obj_id target, String targetString, string_id targetStringId, obj_id other, String otherString, string_id otherStringId, int di, float df, float delay) throws InterruptedException
	{
		if (!isIdValid(msgTarget) || (sid == null) || (delay < 0f))
		{
			return;
		}
		
		dictionary msg = new dictionary();
		msg.put("sid", sid);
		
		msg.put("actor", actor);
		msg.put("actorString", actorString);
		msg.put("actorStringId", actorStringId);
		
		msg.put("target", target);
		msg.put("targetString", targetString);
		msg.put("targetStringId", targetStringId);
		
		msg.put("other", other);
		msg.put("otherString", otherString);
		msg.put("otherStringId", otherStringId);
		
		msg.put("di", di);
		msg.put("df", df);
		
		messageTo(msgTarget, "handleDelayedProseMessage", msg, delay, false);
	}
	
	
	public static Vector alphabetizeStringArray(String[] array) throws InterruptedException
	{
		if ((array == null) || (array.length == 0))
		{
			return null;
		}
		
		Vector tmp = new Vector();
		java.text.Collator myCol = java.text.Collator.getInstance();
		for (int i = 0; i < array.length; i++)
		{
			testAbortScript();
			boolean inserted = false;
			for (int n = 0; n < tmp.size(); n++)
			{
				testAbortScript();
				if (myCol.compare(array[i], tmp.elementAt(n)) < 0)
				{
					tmp.add(n, array[i]);
					inserted = true;
					break;
				}
			}
			
			if (!inserted)
			{
				tmp.add(array[i]);
			}
		}
		
		if ((tmp == null) || (tmp.size() == 0))
		{
			return null;
		}
		
		return tmp;
	}
	
	
	public static Vector alphabetizeStringArray(Vector array) throws InterruptedException
	{
		if ((array == null) || (array.size() == 0))
		{
			return null;
		}
		
		String[] toPass = toStaticStringArray(array);
		if ((toPass == null) || (toPass.length == 0))
		{
			return null;
		}
		
		return alphabetizeStringArray(toPass);
	}
	
	
	public static String getPlayerSpeciesName(int species) throws InterruptedException
	{
		switch (species)
		{
			case SPECIES_HUMAN:
			return "human";
			
			case SPECIES_BOTHAN:
			return "bothan";
			
			case SPECIES_RODIAN:
			return "rodian";
			
			case SPECIES_TWILEK:
			return "twilek";
			
			case SPECIES_TRANDOSHAN:
			return "trandoshan";
			
			case SPECIES_MON_CALAMARI:
			return "moncalamari";
			
			case SPECIES_WOOKIEE:
			return "wookiee";
			
			case SPECIES_ZABRAK:
			return "zabrak";
			case SPECIES_ITHORIAN:
			return "ithorian";
			case SPECIES_SULLUSTAN:
			return "sullustan";
			default:
			return "unknown";
		}
	}
	
	
	public static void addListener(String strObjVar, obj_id objListener, obj_id objTarget) throws InterruptedException
	{
		
		if (strObjVar == null)
		{
			LOG("DESIGNER_FATAL", "Null objVar paseed into addListener");
			return;
			
		}
		if (!isIdValid(objTarget))
		{
			LOG("DESIGNER_FATAL", "Null owner paseed into addListener");
			return;
		}
		if (!isIdValid(objListener))
		{
			
			LOG("DESIGNER_FATAL", "Null listener paseed into addListener");
		}
		
		dictionary dctParams = new dictionary();
		dctParams.put("objListener", objListener);
		dctParams.put("strObjVar", strObjVar);
		messageTo(objTarget, "addListener", dctParams, 0, true);
		return;
		
	}
	
	
	public static void removeListener(String strObjVar, obj_id objListener, obj_id objTarget) throws InterruptedException
	{
		if (strObjVar == null)
		{
			LOG("DESIGNER_FATAL", "Null objVar paseed into removeListener");
			return;
			
		}
		if (!isIdValid(objTarget))
		{
			LOG("DESIGNER_FATAL", "Null target paseed into removeListener");
			return;
		}
		if (!isIdValid(objListener))
		{
			
			LOG("DESIGNER_FATAL", "Null listener paseed into removeListeer");
		}
		
		dictionary dctParams = new dictionary();
		dctParams.put("objListener", objListener);
		dctParams.put("strObjVar", strObjVar);
		messageTo(objTarget, "removeListener", dctParams, 0, true);
		return;
		
	}
	
	
	public static void messageListeners(String strObjVar, obj_id objOwner, String strMessageName, dictionary dctParams) throws InterruptedException
	{
		
		if (strObjVar == null)
		{
			LOG("DESIGNER_FATAL", "Null objVar paseed into messageListeners");
			return;
			
		}
		if (!isIdValid(objOwner))
		{
			LOG("DESIGNER_FATAL", "Null owner paseed into messageListeners");
			return;
		}
		if (strMessageName == null)
		{
			LOG("DESIGNER_FATAL", "Null messageName paseed into messageListeners");
			
		}
		
		if (hasObjVar(objOwner, strObjVar))
		{
			
			int intI = 0;
			obj_id objListeners[] = getObjIdArrayObjVar(objOwner, "mission.objListeners");
			while (intI < objListeners.length)
			{
				testAbortScript();
				
				messageTo(objListeners[intI], strMessageName, dctParams, 0, true);
				intI = intI+1;
			}
		}
		
		return;
		
	}
	
	
	public static int getTheaterSize(String strLairType) throws InterruptedException
	{
		if (isTheater(strLairType))
		{
			int intIndex = strLairType.indexOf("medium");
			if (intIndex > 0)
			{
				return 40;
				
			}
			intIndex = strLairType.indexOf("small");
			if (intIndex > 0)
			{
				return 24;
				
			}
			intIndex = strLairType.indexOf("large");
			if (intIndex > 0)
			{
				return 72;
				
			}
			
			return 24;
		}
		return -1;
		
	}
	
	
	public static boolean isTheater(String strLairType) throws InterruptedException
	{
		
		if (strLairType == null)
		{
			return false;
		}
		int intIndex = strLairType.indexOf("theater");
		if (intIndex > 0)
		{
			return true;
		}
		return false;
	}
	
	
	public static boolean isAppropriateName(String name) throws InterruptedException
	{
		if (name == null || name.equals(""))
		{
			return false;
		}
		
		java.util.StringTokenizer st = new java.util.StringTokenizer(name);
		while (st.hasMoreTokens())
		{
			testAbortScript();
			String tkn = st.nextToken();
			if (stringToFloat(tkn) == Float.NEGATIVE_INFINITY)
			{
				if (isObscene(tkn) || !isAppropriateText(tkn))
				{
					return false;
				}
			}
		}
		
		return true;
	}
	
	
	public static boolean setNonProfaneName(obj_id target, String name) throws InterruptedException
	{
		if (!isIdValid(target) || name == null || name.equals(""))
		{
			return false;
		}
		
		if (!isAppropriateName(name))
		{
			return false;
		}
		
		return setName(target, name);
	}
	
	
	public static boolean setNonReservedName(obj_id target, String name) throws InterruptedException
	{
		if (!isIdValid(target) || name == null || name.equals(""))
		{
			return false;
		}
		
		if (!isNameReserved(name))
		{
			return setName(target, name);
		}
		
		return false;
	}
	
	
	public static boolean setFilteredName(obj_id target, String name) throws InterruptedException
	{
		if (!isIdValid(target) || name == null || name.equals(""))
		{
			return false;
		}
		
		if (!isNameReserved(name))
		{
			return setNonProfaneName(target, name);
		}
		
		return false;
	}
	
	
	public static void destroyObjects(obj_id[] objects) throws InterruptedException
	{
		if (objects == null || objects.length == 0)
		{
			return;
		}
		
		for (int i = 0; i < objects.length; i++)
		{
			testAbortScript();
			if (isIdValid(objects[i]))
			{
				destroyObject(objects[i]);
			}
		}
	}
	
	
	public static String getTemplateFilenameNoPath(obj_id target) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return null;
		}
		
		String filename = null;
		String fullPath = getTemplateName(target);
		if (fullPath != null && !fullPath.equals(""))
		{
			String[] tmp = split(fullPath, '/');
			if (tmp != null && tmp.length > 0)
			{
				filename = tmp[tmp.length - 1];
			}
		}
		
		return filename;
	}
	
	
	public static int getFirstNonValidIdIndex(obj_id[] ids) throws InterruptedException
	{
		if (ids == null || ids.length == 0)
		{
			return -1;
		}
		
		for (int i = 0; i < ids.length; i++)
		{
			testAbortScript();
			if (!isIdValid(ids[i]))
			{
				return i;
			}
		}
		
		return -1;
	}
	
	
	public static int getFirstValidIdIndex(obj_id[] ids) throws InterruptedException
	{
		if (ids == null || ids.length == 0)
		{
			return -1;
		}
		
		for (int i = 0; i < ids.length; i++)
		{
			testAbortScript();
			if (isIdValid(ids[i]))
			{
				return i;
			}
		}
		
		return -1;
	}
	
	
	public static void moneyInMetric(obj_id objTransferer, String strAccount, int intAmount) throws InterruptedException
	{
		
		int intTime = getGameTime();
		String strSpam = "moneyIn;"+intTime+";"+objTransferer+";"+strAccount+";"+intAmount;
		logBalance(strSpam);
		return;
	}
	
	
	public static void moneyOutMetric(obj_id objTransferer, String strAccount, int intAmount) throws InterruptedException
	{
		int intTime = getGameTime();
		String strSpam = "moneyOut;"+intTime+";"+objTransferer+";"+strAccount+";"+intAmount;
		logBalance(strSpam);
		return;
	}
	
	
	public static int getValidAttributeIndex(String[] array) throws InterruptedException
	{
		for (int i = 0; i < array.length; i++)
		{
			testAbortScript();
			if (array[i] == null || array[i].equals(""))
			{
				return i;
			}
		}
		return -1;
	}
	
	
	public static String getStringName(obj_id target) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return null;
		}
		
		String name = getEncodedName(target);
		string_id sid_name = unpackString(name);
		if (sid_name != null)
		{
			name = getString(sid_name);
		}
		
		return name;
	}
	
	
	public static void warpPlayer(obj_id player, location dest) throws InterruptedException
	{
		if (isIdValid(dest.cell))
		{
			warpPlayer (player, dest.area, 0.0f, 0.0f, 0.0f, dest.cell, dest.x, dest.y, dest.z);
		}
		else
		{
			warpPlayer (player, dest.area, dest.x, dest.y, dest.z, null, 0.0f, 0.0f, 0.0f);
		}
	}
	
	
	public static void warpPlayer(obj_id player, String planet, location dest) throws InterruptedException
	{
		warpPlayer (player, planet, dest.x, dest.y, dest.z, null, 0.0f, 0.0f, 0.0f);
	}
	
	
	public static boolean copyObjVarList(obj_id from, obj_id to, String listpath) throws InterruptedException
	{
		if (!isIdValid(from) || !isIdValid(to) || listpath == null || listpath.equals(""))
		{
			return false;
		}
		
		obj_var_list ovl = getObjVarList(from, listpath);
		if (ovl == null)
		{
			return false;
		}
		
		boolean litmus = true;
		
		int numItems = ovl.getNumItems();
		for (int i = 0; i < numItems; i++)
		{
			testAbortScript();
			obj_var ov = ovl.getObjVar(i);
			if (ov != null)
			{
				litmus &= copyObjVar(from, to, listpath + "."+ ov.getName());
			}
		}
		
		return litmus;
	}
	
	
	public static string_id getCardinalDirectionForPoints(location locTest1, location locTest2) throws InterruptedException
	{
		String cardinalString = getStringCardinalDirection(locTest1, locTest2);
		if (cardinalString == null || cardinalString.equals(""))
		{
			return null;
		}
		
		return new string_id("mission/mission_generic", cardinalString);
	}
	
	
	public static String getStringCardinalDirection(location locTest1, location locTest2, boolean abbreviated) throws InterruptedException
	{
		if (locTest1 == null || locTest2 == null)
		{
			return null;
		}
		
		float fltAngle = getHeadingToLocation(locTest1, locTest2);
		
		if ((fltAngle>=-22.5)&&(fltAngle <= 22.5))
		{
			if (!abbreviated)
			{
				return "north";
			}
			else
			{
				return "N";
			}
		}
		else if ((fltAngle>=-67.5)&&(fltAngle<=-22.5))
		{
			if (!abbreviated)
			{
				return "northeast";
			}
			else
			{
				return "NE";
			}
		}
		else if ((fltAngle>=-112.5)&&(fltAngle<=-67.5))
		{
			if (!abbreviated)
			{
				return "east";
			}
			else
			{
				return "E";
			}
		}
		else if ((fltAngle>=-157.5)&&(fltAngle<=-112.5))
		{
			if (!abbreviated)
			{
				return "southeast";
			}
			else
			{
				return "SE";
			}
		}
		
		else if ((fltAngle >= 22.5)&&(fltAngle <= 67.5))
		{
			if (!abbreviated)
			{
				return "northwest";
			}
			else
			{
				return "NW";
			}
		}
		else if ((fltAngle >= 67.5)&&(fltAngle <= 112.5))
		{
			if (!abbreviated)
			{
				return "west";
			}
			else
			{
				return "W";
			}
		}
		else if ((fltAngle >= 112.5)&&(fltAngle <= 157.5))
		{
			if (!abbreviated)
			{
				return "southwest";
			}
			else
			{
				return "SW";
			}
		}
		else
		{
			if (!abbreviated)
			{
				return "south";
			}
			else
			{
				return "S";
			}
		}
	}
	
	
	public static String getStringCardinalDirection(location locTest1, location locTest2) throws InterruptedException
	{
		return getStringCardinalDirection(locTest1, locTest2, false);
	}
	
	
	public static boolean isContainer(obj_id target) throws InterruptedException
	{
		if (isIdValid(target))
		{
			return (getContainerType(target)!=0);
		}
		
		return false;
	}
	
	
	public static boolean noIncapDrainAttributes(obj_id target, int actionCost, int mindCost) throws InterruptedException
	{
		if (!isIdValid(target) || actionCost < 0)
		{
			return false;
		}
		
		if (getAttrib(target, ACTION) < actionCost)
		{
			
			return false;
		}
		
		return drainAttributes(target, actionCost, 0);
	}
	
	
	public static int getUnbuffedWoundedMaxAttrib(obj_id target, int attrib) throws InterruptedException
	{
		if (!isIdValid(target) || attrib < HEALTH || attrib > WILLPOWER)
		{
			return -1;
		}
		
		int unmodmax = getUnmodifiedMaxAttrib(target, attrib);
		return unmodmax;
	}
	
	
	public static boolean validatePlayerHairStyle(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		String hair_table = "datatables/customization/hair_assets_skill_mods.iff";
		
		obj_id hair = getObjectInSlot(player, "hair");
		
		if (!isIdValid(hair))
		{
			return false;
		}
		
		String hair_template = getTemplateName(hair);
		
		int idx = dataTableSearchColumnForString(hair_template, "SERVER_TEMPLATE", hair_table);
		
		if (idx < 0)
		{
			return false;
		}
		
		String required_template = dataTableGetString(hair_table, idx, "SERVER_PLAYER_TEMPLATE");
		
		if (required_template == null)
		{
			return false;
		}
		
		String player_template = getTemplateName(player);
		
		if (!player_template.equals(required_template))
		{
			
			int new_hair_idx = dataTableSearchColumnForString(player_template, "SERVER_PLAYER_TEMPLATE", hair_table);
			
			String new_hair_template = dataTableGetString(hair_table, new_hair_idx, "SERVER_TEMPLATE");
			
			if (new_hair_template == null)
			{
				CustomerServiceLog("imageDesigner", getFirstName(player) + "("+ player + ") has an invalid hairstyle, but a replacement cannot be found!");
				return false;
			}
			
			destroyObject(hair);
			createObject(new_hair_template, player, "hair");
			CustomerServiceLog("imageDesigner", getFirstName(player) + "("+ player + ") has an invalid hairstyle. It has been replaced with a default style.");
			sendSystemMessageTestingOnly(player, "An illegal hairstyle has been detected on your character. This has been corrected.");
		}
		
		return true;
	}
	
	
	public static String getPackedScripts(obj_id objObject) throws InterruptedException
	{
		String strTest = "";
		String[] strScripts = getScriptList(objObject);
		if (strScripts.length > 0)
		{
			for (int intI = 0; intI<strScripts.length; intI++)
			{
				testAbortScript();
				strTest = strTest + strScripts[intI];
				if (intI<strScripts.length-1)
				{
					strTest = strTest + ",";
				}
			}
		}
		return strTest;
	}
	
	public static String[] getUsableScriptList(obj_id objObject) throws InterruptedException
	{
		String strTest = "";
		String[] strScripts = getScriptList(objObject);
		debugSpeakMsg(objObject, "script list is "+strScripts.length);
		if (strScripts.length > 0)
		{
			debugSpeakMsg(objObject, "list length is "+strScripts.length);
			String[] strCorrectArray = new String[strScripts.length];
			for (int intI = 0; intI<strScripts.length; intI++)
			{
				testAbortScript();
				String script = strScripts[intI];
				if (script.indexOf( "script." ) > -1)
				{
					script = script.substring( 7 );
					debugSpeakMsg(objObject, "setting array to "+script);
					strCorrectArray[intI] = script;
				}
			}
			return strCorrectArray;
		}
		return null;
		
	}
	
	
	public static String[] unpackScriptString(String strScripts) throws InterruptedException
	{
		String[] strTest = split(strScripts, ',');
		return strTest;
	}
	
	
	public static boolean isEquipped(obj_id item) throws InterruptedException
	{
		if (!isIdValid(item))
		{
			return false;
		}
		
		obj_id containedBy = getContainedBy(item);
		return ( isIdValid(containedBy) && getContainerType(containedBy) == 1 );
	}
	
	
	public static obj_id unequipWeaponHand(obj_id player) throws InterruptedException
	{
		return unequipSlot(player, "hold_r");
	}
	
	
	public static obj_id unequipOffHand(obj_id player) throws InterruptedException
	{
		return unequipSlot(player, "hold_l");
	}
	
	
	public static obj_id unequipSlot(obj_id player, String slot) throws InterruptedException
	{
		obj_id equipment = getObjectInSlot(player, slot);
		if (isIdValid(equipment))
		{
			obj_id playerInv = utils.getInventoryContainer(player);
			if (isIdValid(playerInv))
			{
				putInOverloaded(equipment, playerInv);
				return equipment;
			}
		}
		return obj_id.NULL_ID;
	}
	
	
	public static obj_id[] getAttackableTargetsInRadius(obj_id attacker, int radius, boolean player_targets) throws InterruptedException
	{
		
		location loc = getLocation(attacker);
		if (loc == null)
		{
			return null;
		}
		
		obj_id[] objects = getObjectsInRange(loc, radius);
		Vector attackable_targets = new Vector();
		attackable_targets.setSize(0);
		for (int i = 0; i < objects.length; i++)
		{
			testAbortScript();
			if (isMob(objects[i]))
			{
				if (pvpCanAttack(attacker, objects[i]))
				{
					if (!isIncapacitated(objects[i]) && !isDead(objects[i]))
					{
						if (isPlayer(objects[i]))
						{
							if (!player_targets)
							{
								continue;
							}
						}
						
						if (canSee(attacker, objects[i]))
						{
							attackable_targets = utils.addElement(attackable_targets, objects[i]);
						}
					}
				}
			}
			else if (ai_lib.isTurret(objects[i]))
			{
				if (pvpCanAttack(attacker, objects[i]))
				{
					
					if (canSee(attacker, objects[i]))
					{
						attackable_targets = utils.addElement(attackable_targets, objects[i]);
					}
				}
			}
		}
		
		if (attackable_targets.size() < 1)
		{
			return null;
		}
		else
		{
			obj_id[] _attackable_targets = new obj_id[0];
			if (attackable_targets != null)
			{
				_attackable_targets = new obj_id[attackable_targets.size()];
				attackable_targets.toArray(_attackable_targets);
			}
			return _attackable_targets;
		}
	}
	
	
	public static obj_id getTrapDroidId(obj_id player) throws InterruptedException
	{
		obj_id dataPad = getPlayerDatapad(player);
		Vector data = getResizeableContents(dataPad);
		
		for (int i = 0; i < data.size(); i++)
		{
			testAbortScript();
			if (callable.hasCDCallable(((obj_id)(data.get(i)))) && hasObjVar(((obj_id)(data.get(i))), "module_data.trap_bonus"))
			{
				return ((obj_id)(data.get(i)));
			}
		}
		
		return null;
		
	}
	
	
	public static int getTimeLeft(obj_id player, String toCheckFor, int timePenalty) throws InterruptedException
	{
		
		int currentTime = getGameTime();
		int timeCalled = utils.getIntScriptVar(player, toCheckFor);
		if (timeCalled < 1)
		{
			return -1;
		}
		
		int timeDone = timeCalled + timePenalty;
		int timeLeft = timeDone - currentTime;
		
		return timeLeft;
		
	}
	
	
	public static int getTimeLeft(obj_id player, String toCheckFor, String modifiedTime) throws InterruptedException
	{
		
		int currentTime = getGameTime();
		int timeCalled = utils.getIntScriptVar(player, toCheckFor);
		if (timeCalled < 1)
		{
			return -1;
		}
		
		int timePenalty = utils.getIntScriptVar(player, modifiedTime);
		int timeDone = timeCalled + timePenalty;
		int timeLeft = timeDone - currentTime;
		
		return timeLeft;
		
	}
	
	
	public static boolean isFreeTrial(obj_id player) throws InterruptedException
	{
		return isFreeTrialAccount(player);
	}
	
	
	public static boolean isFreeTrial(obj_id player, obj_id target) throws InterruptedException
	{
		return isFreeTrialAccount(player);
		
	}
	
	
	public static void notifyObject(obj_id objTarget, String strNotificationName, dictionary dctParams) throws InterruptedException
	{
		if (!isIdValid(objTarget))
		{
			debugServerConsoleMsg(null, "Null object id passed into notifyObject, exceptioning");
			Thread.dumpStack();
			throw new InterruptedException();
		}
		if (strNotificationName == null)
		{
			debugServerConsoleMsg(null, "null notification name passed into notifyObject, exceptioning");
			Thread.dumpStack();
			throw new InterruptedException();
		}
		if (strNotificationName.equals(""))
		{
			debugServerConsoleMsg(null, "Empty Notification name passed into notifyObject, exceptioning");
			Thread.dumpStack();
			throw new InterruptedException();
		}
		
		try
		{
			int intReturn = script_entry.callMessageHandlers(strNotificationName, objTarget, dctParams);
			return;
		}
		catch (Throwable err)
		{
			debugServerConsoleMsg(null, "Unable to call into callMessageHandlers ");
			Thread.dumpStack();
			throw new InterruptedException();
		}
	}
	
	
	public static void callTrigger(String strTrigger, Object[] params) throws InterruptedException
	{
		try
		{
			script_entry.runScripts(strTrigger, params);
			return;
		}
		catch (Throwable err)
		{
			debugServerConsoleMsg(null, "Unable to call into callMessageHandlers ");
			Thread.dumpStack();
			throw new InterruptedException();
		}
	}
	
	
	public static String getCellName(obj_id building, obj_id cell) throws InterruptedException
	{
		
		String[] cellNames = getCellNames(building);
		
		for (int i = 0; i < cellNames.length; i++)
		{
			testAbortScript();
			
			String cellName = cellNames[i];
			obj_id thisCell = getCellId(building, cellName);
			
			if (thisCell == cell)
			{
				
				return cellName;
			}
		}
		
		return "";
	}
	
	
	public static String getRealPlayerFirstName(obj_id player) throws InterruptedException
	{
		String firstName = getPlayerName(player);
		
		if (firstName == null)
		{
			return null;
		}
		
		int idx = firstName.toLowerCase().indexOf("corpse of");
		if (idx >= 0)
		{
			firstName = firstName.substring(idx+1);
		}
		
		if (firstName.length() > 0)
		{
			char first = firstName.charAt(0);
			first = Character.toUpperCase(first);
			firstName = first + firstName.substring(1);
		}
		
		return firstName;
	}
	
	
	public static void dismountRiderJetpackCheck(obj_id rider) throws InterruptedException
	{
		if (!isIdValid(rider))
		{
			return;
		}
		
		obj_id mount = getMountId (rider);
		
		if (!exists(mount))
		{
			return;
		}
		
		if (isIdValid(mount))
		{
			pet_lib.doDismountNow(rider, false);
			String name = getTemplateName (mount);
			
			if (vehicle.isJetPackVehicle(mount))
			{
				string_id jetDismount = new string_id ("pet/pet_menu", "jetpack_dismount");
				sendSystemMessage(rider, jetDismount);
				
				obj_id petControlDevice = callable.getCallableCD(mount);
				vehicle.storeVehicle(petControlDevice, rider);
			}
		}
		
		return;
	}
	
	
	public static location getLocationFromTransform(transform trTest) throws InterruptedException
	{
		location locTest = new location();
		vector vctTest = trTest.getPosition_p();
		locTest.x = vctTest.x;
		locTest.y = vctTest.y;
		locTest.z = vctTest.z;
		return locTest;
	}
	
	
	public static int addClassRequirementAttributes(obj_id player, obj_id item, String[] names, String[] attribs, int firstFree, String prefix) throws InterruptedException
	{
		
		if (hasObjVar(item, prefix + "classRequired"))
		{
			
			String requiredClasses = getStringObjVar(item, prefix + "classRequired");
			java.util.StringTokenizer st = new java.util.StringTokenizer(requiredClasses, ",");
			String requiredText = "";
			boolean qualifies = false;
			
			while (st.hasMoreTokens())
			{
				testAbortScript();
				String classId = st.nextToken();
				String tmp = "@skl_n:class_"+ classId + "\0";
				if (st.hasMoreTokens())
				{
					tmp += "\n\\>117\0";
				}
				
				requiredText += tmp;
				
				if (isProfession(player, stringToInt(classId)))
				{
					qualifies = true;
					break;
				}
			}
			
			if (!qualifies)
			{
				names[firstFree] = "class_required";
				attribs[firstFree++] = requiredText;
			}
		}
		
		if (hasObjVar(item, prefix + "levelRequired"))
		{
			int minLevel = getIntObjVar(item, prefix + "levelRequired");
			if (minLevel > getLevel(player))
			{
				names[firstFree] = "levelrequired";
				attribs[firstFree++] = "" + minLevel;
			}
		}
		
		return firstFree;
	}
	
	
	public static boolean testItemClassRequirements(obj_id player, String requiredClasses, boolean silent) throws InterruptedException
	{
		java.util.StringTokenizer st = new java.util.StringTokenizer(requiredClasses, ",");
		String requiredText = "";
		
		while (st.hasMoreTokens())
		{
			testAbortScript();
			String classId = st.nextToken();
			if (isProfession(player, stringToInt(classId)))
			{
				return true;
			}
		}
		
		if (!silent)
		{
			sendSystemMessage(player, new string_id("spam", "classrequired"));
		}
		
		return false;
	}
	
	
	public static boolean testItemClassRequirements(obj_id player, obj_id thing, boolean silent, String prefix) throws InterruptedException
	{
		if (!hasObjVar(thing, prefix + "classRequired"))
		{
			return true;
		}
		
		String requiredClasses = getStringObjVar(thing, prefix + "classRequired");
		return testItemClassRequirements(player, requiredClasses, silent);
	}
	
	
	public static boolean testItemLevelRequirements(obj_id player, obj_id thing, boolean silent, String prefix) throws InterruptedException
	{
		if (!hasObjVar(thing, prefix + "levelRequired"))
		{
			return true;
		}
		
		int minLevel = getIntObjVar(thing, prefix + "levelRequired");
		if (minLevel > getLevel(player))
		{
			if (!silent)
			{
				prose_package pp = prose.getPackage(new string_id("spam", "levelrequired"), ""+ minLevel);
				sendSystemMessageProse(player, pp);
			}
			return false;
		}
		
		return true;
	}
	
	
	public static boolean testItemAbilityRequirements(obj_id player, obj_id thing, boolean silent, String prefix) throws InterruptedException
	{
		if (!hasObjVar(thing, prefix + "abilityRequired"))
		{
			return true;
		}
		
		String commandRequired = getStringObjVar(thing, prefix + "abilityRequired");
		if (!hasCommand(player, commandRequired))
		{
			if (!silent)
			{
				prose_package pp = prose.getPackage(new string_id("spam", "abilityrequired"), "@cmd_n:"+ toLower(commandRequired));
				sendSystemMessageProse(player, pp);
			}
			return false;
		}
		
		return true;
	}
	
	
	public static boolean testItemSkillRequirements(obj_id player, obj_id thing, boolean silent, String prefix) throws InterruptedException
	{
		if (!hasObjVar(thing, prefix + "skillRequired"))
		{
			return true;
		}
		
		String skillRequired = getStringObjVar(thing, prefix + "skillRequired");
		if (!hasSkill(player, skillRequired))
		{
			if (!silent)
			{
				prose_package pp = prose.getPackage(new string_id("spam", "skill_required"), "@skl_n:"+ toLower(skillRequired));
				sendSystemMessageProse(player, pp);
			}
			return false;
		}
		
		return true;
	}
	
	
	public static void makeItemNoDrop(obj_id item) throws InterruptedException
	{
		attachScript(item, "item.special.nomove");
		setObjVar(item, "noTrade", 1);
	}
	
	
	public static boolean isItemNoDrop(obj_id item) throws InterruptedException
	{
		return hasScript(item, "item.special.nomove") || hasObjVar(item, "noTrade");
	}
	
	
	public static void clearNoDropFromItem(obj_id item) throws InterruptedException
	{
		detachScript(item, "item.special.nomove");
		removeObjVar(item, "noTrade");
	}
	
	
	public static obj_id findNoTradeItem(obj_id[] items, boolean testPlayers) throws InterruptedException
	{
		return findNoTradeItem( items, testPlayers, false );
	}
	
	
	public static obj_id findNoTradeItem(obj_id[] items, boolean testPlayers, boolean novendor) throws InterruptedException
	{
		if (items != null)
		{
			for (int i = 0; i < items.length; ++i)
			{
				testAbortScript();
				if (isIdValid(items[i]) && (testPlayers || !isPlayer(items[i])))
				{
					if (novendor && hasScript( items[i], "terminal.vendor" ))
					{
						continue;
					}
					if (!canTrade(items[i]))
					{
						return items[i];
					}
					else if (utils.isContainer(items[i]))
					{
						obj_id result = findNoTradeItem(getContents(items[i]), testPlayers);
						if (isIdValid(result))
						{
							return result;
						}
					}
				}
			}
		}
		return null;
	}
	
	
	public static obj_id findNoTradeItemNotVendor(obj_id[] items, boolean testPlayers) throws InterruptedException
	{
		return findNoTradeItem( items, testPlayers, true );
	}
	
	
	public static obj_id hasWaypoint(obj_id player, String name) throws InterruptedException
	{
		if (!isIdValid(player) || name == null)
		{
			return null;
		}
		
		obj_id[] waypoints = getWaypointsInDatapad(player);
		if (waypoints == null)
		{
			return null;
		}
		
		for (int i = 0; i < waypoints.length; ++i)
		{
			testAbortScript();
			if (isIdValid(waypoints[i]))
			{
				String waypointName = getWaypointName(waypoints[i]);
				if (waypointName != null && waypointName.equals(name))
				{
					return waypoints[i];
				}
			}
		}
		return null;
	}
	
	
	public static boolean waypointExists(obj_id player, obj_id waypoint) throws InterruptedException
	{
		if (!isIdValid(player) || !isIdValid(waypoint))
		{
			return false;
		}
		
		obj_id[] waypoints = getWaypointsInDatapad(player);
		if (waypoints == null || waypoints.length <= 0)
		{
			return false;
		}
		
		for (int i = 0; i < waypoints.length; ++i)
		{
			testAbortScript();
			if (isIdValid(waypoints[i]))
			{
				if (waypoints[i] == waypoint)
				{
					return true;
				}
			}
		}
		return false;
	}
	
	
	public static boolean isExtendedASCII(String inString) throws InterruptedException
	{
		
		if (inString == null)
		{
			return false;
		}
		
		for (int i = 0; i < inString.length(); i++)
		{
			testAbortScript();
			int ASCIIValue = inString.charAt(i);
			if (ASCIIValue < 0 || ASCIIValue > 255)
			{
				return true;
			}
		}
		return false;
	}
	
	
	public static boolean isAntiDecay(obj_id item) throws InterruptedException
	{
		if (isIdValid(item) && hasObjVar(item, "antidecay"))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean makeAntiDecay(obj_id item) throws InterruptedException
	{
		if (isIdValid(item))
		{
			setObjVar(item, "antidecay", 1);
			attachScript(item, "systems.veteran_reward.antidecay_examine");
			
			return true;
		}
		
		else
		{
			
			return false;
		}
	}
	
	
	public static boolean removeAntiDecay(obj_id item) throws InterruptedException
	{
		if (isIdValid(item))
		{
			if (hasObjVar(item, "antidecay"))
			{
				removeObjVar(item, "antidecay");
			}
			
			return true;
		}
		
		else
		{
			return false;
		}
	}
	
	
	public static boolean validItemForAntiDecay(obj_id item) throws InterruptedException
	{
		if (isIdValid(item))
		{
			
			if (jedi.isLightsaber(item))
			{
				return true;
			}
			
			if (isGameObjectTypeOf(item, GOT_armor_psg))
			{
				return false;
			}
			
			if (cloninglib.isDamagedOnCloneGOT(getGameObjectType(item)) || isGameObjectTypeOf(item, GOT_weapon))
			{
				return true;
			}
		}
		return false;
	}
	
	
	public static obj_id[] getLocalGroupMemberIds(obj_id group) throws InterruptedException
	{
		if (!isIdValid(group))
		{
			return null;
		}
		
		obj_id[] groupMemberIds = getGroupMemberIds(group);
		if (groupMemberIds == null || groupMemberIds.length == 0)
		{
			return null;
		}
		
		Vector localMemberIdsVector = new Vector();
		for (int i =0; i < groupMemberIds.length; ++i)
		{
			testAbortScript();
			if (isIdValid(groupMemberIds[i]) && exists(groupMemberIds[i]) && isPlayer(groupMemberIds[i]))
			{
				localMemberIdsVector.addElement(groupMemberIds[i]);
			}
		}
		
		if (localMemberIdsVector == null || localMemberIdsVector.size() < 1)
		{
			return null;
		}
		
		return ( utils.toStaticObjIdArray(localMemberIdsVector));
	}
	
	
	public static boolean canSpeakWookiee(obj_id player, obj_id npc) throws InterruptedException
	{
		if (hasSkill(player, "class_smuggler_phase1_novice"))
		{
			return false;
		}
		
		if (hasSkill(player, "social_language_wookiee_comprehend"))
		{
			return false;
		}
		
		else
		{
			return true;
		}
	}
	
	
	public static void emoteWookieeConfusion(obj_id player, obj_id npc) throws InterruptedException
	{
		playClientEffectObj(npc, "clienteffect/voc_wookiee_med_4sec.cef", player, "");
		chat.thinkTo(player, player, new string_id("ep3/sidequests", "wke_convo_failure"));
	}
	
	
	public static void setObjVarsList(obj_id object, String objVarList) throws InterruptedException
	{
		if (objVarList == null || objVarList.equals(""))
		{
			return;
		}
		
		String[] pairs = split(objVarList, ',');
		for (int i = 0; i < pairs.length; i++)
		{
			testAbortScript();
			
			String[] objVarToSet = split(pairs[i], '=');
			String objVarValue = objVarToSet[1];
			
			String[] objVarNameAndType = split(objVarToSet[0], ':');
			String objVarType = objVarNameAndType[0];
			String objVarName = objVarNameAndType[1];
			
			if (objVarType.equals("string"))
			{
				setObjVar(object, objVarName, objVarValue);
			}
			else if (objVarType.equals("int"))
			{
				setObjVar(object, objVarName, utils.stringToInt( objVarValue));
			}
			else if (objVarType.equals("float"))
			{
				setObjVar(object, objVarName, utils.stringToFloat( objVarValue));
			}
			else if (objVarType.equals("boolean") || objVarType.equals("bool"))
			{
				setObjVar(object, objVarName, utils.stringToInt( objVarValue));
			}
			else
			{
				setObjVar(object, objVarName, objVarValue);
			}
			
		}
	}
	
	
	public static void setObjVarsListUsingSemiColon(obj_id object, String objVarList) throws InterruptedException
	{
		if (objVarList == null || objVarList.equals(""))
		{
			return;
		}
		
		String[] pairs = split(objVarList, ',');
		for (int i = 0; i < pairs.length; i++)
		{
			testAbortScript();
			
			String[] objVarToSet = split(pairs[i], '=');
			String objVarValue = objVarToSet[1];
			
			String[] objVarNameAndType = split(objVarToSet[0], ';');
			String objVarType = objVarNameAndType[0];
			String objVarName = objVarNameAndType[1];
			
			if (objVarType.equals("string"))
			{
				setObjVar(object, objVarName, objVarValue);
			}
			else if (objVarType.equals("int"))
			{
				setObjVar(object, objVarName, utils.stringToInt( objVarValue));
			}
			else if (objVarType.equals("float"))
			{
				setObjVar(object, objVarName, utils.stringToFloat( objVarValue));
			}
			else if (objVarType.equals("boolean") || objVarType.equals("bool"))
			{
				setObjVar(object, objVarName, utils.stringToInt( objVarValue));
			}
			else
			{
				setObjVar(object, objVarName, objVarValue);
			}
			
		}
	}
	
	
	public static boolean verifyLocationBasedDestructionAnchor(obj_id subject, float distance) throws InterruptedException
	{
		if (!hasObjVar(subject, "recordLoc"))
		{
			location recordLoc = getLocation(subject);
			setObjVar(subject, "recordLoc", recordLoc);
			return true;
		}
		
		location verifyLoc = getLocationObjVar(subject, "recordLoc");
		location here = getLocation(getTopMostContainer(subject));
		float distanceDifference = getDistance(verifyLoc, here);
		if (distanceDifference > distance || distanceDifference == -1f)
		{
			destroyObject(subject);
			return false;
		}
		
		return true;
	}
	
	
	public static boolean verifyLocationBasedDestructionAnchor(obj_id subject, location recordLoc, float distance) throws InterruptedException
	{
		if (!hasObjVar(subject, "recordLoc"))
		{
			setObjVar(subject, "recordLoc", recordLoc);
			return true;
		}
		
		location verifyLoc = getLocationObjVar(subject, "recordLoc");
		location here = getLocation(getTopMostContainer(subject));
		float distanceDifference = getDistance(verifyLoc, here);
		if (distanceDifference > distance || distanceDifference == -1f)
		{
			destroyObject(subject);
			return false;
		}
		
		return true;
	}
	
	
	public static String assembleTimeRemainToUse(int time) throws InterruptedException
	{
		return assembleTimeRemainToUse(time, true);
	}
	
	
	public static String assembleTimeRemainToUse(int time, boolean localized) throws InterruptedException
	{
		
		if (time <= 60)
		{
			String attrib = "";
			if (localized)
			{
				attrib = time + (time != 1 ? " $@spam:seconds$": " $@spam:second$");
			}
			else
			{
				attrib = time + (time != 1 ? " seconds": " second");
			}
			
			return attrib;
		}
		
		if (time > 60 && time <= 3600)
		{
			
			int minutes = (int)Math.floor(time / 60);
			
			int remainder_time = time % 60;
			
			String attrib = "";
			if (localized)
			{
				attrib = minutes + (minutes != 1 ? " $@spam:minutes$, ": " $@spam:minute$, ") + remainder_time + (remainder_time != 1 ? " $@spam:seconds$": " $@spam:second$");
			}
			else
			{
				attrib = minutes + (minutes != 1 ? " minutes, ": " minute, ") + remainder_time + (remainder_time != 1 ? " seconds": " second");
			}
			return attrib;
		}
		
		if (time > 3600 && time <= 86400)
		{
			
			int hours = (int)Math.floor(time / 3600);
			
			int remainder_time = (time % 3600) / 60;
			
			String attrib = "";
			if (localized)
			{
				attrib = hours + (hours != 1 ? " $@spam:hours$, ": " $@spam:hour$, ") + remainder_time + (remainder_time != 1 ? " $@spam:minutes$": " $@spam:minute$");
			}
			else
			{
				attrib = hours + (hours != 1 ? " hours, ": " hour, ") + remainder_time + (remainder_time != 1 ? " minutes": " minute");
			}
			return attrib;
		}
		
		if (time > 86400 && time < Integer.MAX_VALUE)
		{
			
			int days = (int)Math.floor(time / 86400);
			
			int remainder_hours = (time % 86400) / 3600;
			int remainder_minutes = (time % 3600) / 60;
			
			String attrib = "";
			if (localized)
			{
				attrib = days + (days != 1 ? " $@spam:days$, ": " $@spam:day$, ") + remainder_hours + (remainder_hours != 1 ? " $@spam:hours$, ": " $@spam:hour$, ") +
				+ remainder_minutes + (remainder_minutes != 1 ? " $@spam:minutes$": " $@spam:minute$");
			}
			else
			{
				attrib = days + (days != 1 ? " days, ": " day, ") + remainder_hours + (remainder_hours != 1 ? " hours, ": " hour, ") +
				+ remainder_minutes + (remainder_minutes != 1 ? " minutes": " minute");
			}
			
			return attrib;
		}
		
		return null;
	}
	
	
	public static boolean isProfession(obj_id player, int profession) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		if (!isPlayer(player))
		{
			return false;
		}
		
		String classTemplate = getSkillTemplate(player);
		String professionName = "";
		
		switch(profession)
		{
			case COMMANDO:
			professionName = "commando";
			break;
			case SMUGGLER:
			professionName = "smuggler";
			break;
			case MEDIC:
			professionName = "medic";
			break;
			case OFFICER:
			professionName = "officer";
			break;
			case SPY:
			professionName = "spy";
			break;
			case BOUNTY_HUNTER:
			professionName = "bounty";
			break;
			case FORCE_SENSITIVE:
			professionName = "force";
			break;
			case TRADER:
			professionName = "trader";
			break;
			case ENTERTAINER:
			professionName = "entertainer";
			break;
			default:
			break;
		}
		
		if (classTemplate != null && classTemplate.startsWith(professionName))
		{
			return true;
		}
		return false;
	}
	
	
	public static int getPlayerProfession(obj_id player) throws InterruptedException
	{
		String[] noviceSkillList = 
		{
			"class_forcesensitive_phase1_novice",
			"class_bountyhunter_phase1_novice",
			"class_smuggler_phase1_novice",
			"class_commando_phase1_novice",
			"class_officer_phase1_novice",
			"class_spy_phase1_novice",
			"class_medic_phase1_novice",
			"class_entertainer_phase1_novice"
		};
		
		int[] professionList = 
		{
			FORCE_SENSITIVE,
			BOUNTY_HUNTER,
			SMUGGLER,
			COMMANDO,
			OFFICER,
			SPY,
			MEDIC,
			ENTERTAINER
		};
		
		for (int i=0; i<noviceSkillList.length; i++)
		{
			testAbortScript();
			if (hasSkill(player, noviceSkillList[i]))
			{
				return professionList[i];
			}
		}
		
		return TRADER;
	}
	
	
	public static byte[] packObject(Object o) throws InterruptedException
	{
		byte[] ret = null;
		
		try
		{
			ByteArrayOutputStream byteOutput = new ByteArrayOutputStream();
			ObjectOutputStream objectOutput = new ObjectOutputStream(byteOutput);
			
			try
			{
				objectOutput.writeObject(o);
				ret = byteOutput.toByteArray();
			}
			catch ( IOException err )
			{
				LOG("utils", "ERROR in Java utils.packObject(): "+ err.getMessage());
			}
		}
		catch (java.io.IOException e)
		{
			LOG("utils", "ERROR in initialization of utils.packObject: "+ e.getMessage());
		}
		
		return ret;
	}
	
	
	public static Object unpackObject(byte[] source) throws InterruptedException
	{
		try
		{
			ByteArrayInputStream byteInput = new ByteArrayInputStream(source);
			ObjectInputStream ois = new ObjectInputStream(byteInput);
			Object result = ois.readObject();
			
			return result;
		}
		catch (ClassNotFoundException x)
		{
			LOG("utils", "ClassNotFoundException: "+x.toString());
		}
		catch (IOException x)
		{
			LOG("utils", "IOException: "+x.toString());
		}
		
		return null;
	}
	
	
	public static boolean replaceSnowflakeItem(obj_id oldItem, String newTemplateName) throws InterruptedException
	{
		obj_id ownerContainer = getContainedBy(oldItem);
		
		if (!isIdValid(ownerContainer))
		{
			return false;
		}
		
		boolean inVendor = isInVendor(oldItem);
		boolean inBazaar = isInBazaar(oldItem);
		if (inVendor || inBazaar)
		{
			
			messageTo(oldItem, "handlerReInitialize", null, 5.0f, false);
			return false;
		}
		
		obj_id newItem = null;
		
		if (isPlayer(ownerContainer))
		{
			obj_id inventory = utils.getInventoryContainer(ownerContainer);
			
			if (utils.isEquipped(oldItem))
			{
				putInOverloaded(oldItem, inventory);
			}
			
			newItem = createObjectOverloaded(newTemplateName, inventory);
			CustomerServiceLog("replaceSnowflakeItem: ", "Old Item ("+ oldItem + ")"+ " is contained in player's "+ getFirstName(getContainingPlayer(oldItem)) + "("+ getContainingPlayer(oldItem) + ") inventory.");
			CustomerServiceLog("replaceSnowflakeItem: ", "New Item ("+ newItem + ")"+ " was created in "+ getFirstName(getContainingPlayer(newItem)) + "("+ getContainingPlayer(newItem) + ") inventory.");
		}
		
		else if (getCellName(ownerContainer) != null)
		{
			if (player_structure.isBuilding(getTopMostContainer(ownerContainer)))
			{
				location loc = getLocation(oldItem);
				newItem = createObject(newTemplateName, loc);
				
				if (isIdValid(newItem))
				{
					if (isObjectPersisted(oldItem) && !isObjectPersisted(newItem))
					{
						persistObject(newItem);
					}
				}
				
				CustomerServiceLog("replaceSnowflakeItem: ", "Old Item ("+ oldItem + ") is contained in a building ("+ getTopMostContainer(ownerContainer) + ")");
				CustomerServiceLog("replaceSnowflakeItem: ", "New Item ("+ newItem + ") was created in building ("+ getTopMostContainer(ownerContainer) + ")");
				CustomerServiceLog("replaceSnowflakeItem: ", "New Item ("+ newItem + ")s location is in a building ("+ getTopMostContainer(ownerContainer) + ") is x = "+ loc.x + " y = "+ loc.y + " z = "+ loc.z + "and cell = "+ getCellName(getTopMostContainer(ownerContainer), loc.cell));
			}
		}
		else
		{
			newItem = createObjectOverloaded(newTemplateName, ownerContainer);
			CustomerServiceLog("replaceSnowflakeItem: ", "New Item ("+ newItem + ")"+ " was created in container ("+ ownerContainer + ")");
		}
		
		if (isIdValid(newItem))
		{
			CustomerServiceLog("replaceSnowflakeItem: ", "New Item ("+ newItem + ")"+ " has a valid ID. So Old Item ("+ oldItem + ") is to be destroyed");
			destroyObject(oldItem);
		}
		
		return true;
	}
	
	
	public static void checkInventoryForSnowflakeItemSwaps(obj_id self) throws InterruptedException
	{
		String[] oldTemplates = dataTableGetStringColumn("datatables/item/snowflake_item_swaps.iff", "OLD_TEMPLATE");
		obj_id objInventory = getInventoryContainer(self);
		
		if (isIdValid(objInventory))
		{
			obj_id[] objContents = getContents(objInventory, true);
			
			if (objContents != null)
			{
				for (int i = 0; i < objContents.length; i++)
				{
					testAbortScript();
					for (int j = 0; j < oldTemplates.length; j++)
					{
						testAbortScript();
						String strItemTemplate = getTemplateName(objContents[i]);
						if (strItemTemplate.equals(oldTemplates[j]))
						{
							String newTemplate = dataTableGetString("datatables/item/snowflake_item_swaps.iff", j, "NEW_TEMPLATE");
							
							if (newTemplate != null || !newTemplate.equals(""))
							{
								replaceSnowflakeItem(objContents[i], newTemplate);
							}
						}
					}
				}
			}
		}
		
		return;
	}
	
	
	public static boolean isVendorObject(obj_id object) throws InterruptedException
	{
		if (hasScript(object, VENDOR_SCRIPT))
		{
			return true;
		}
		return false;
	}
	
	
	public static boolean isBazaarObject(obj_id object) throws InterruptedException
	{
		if (hasScript(object, BAZAAR_SCRIPT))
		{
			return true;
		}
		return false;
	}
	
	
	public static boolean isInVendor(obj_id item) throws InterruptedException
	{
		obj_id ownerContainer = getContainedBy(item);
		if (!isIdValid(ownerContainer))
		{
			return false;
		}
		
		obj_id parentContainer = getContainedBy(ownerContainer);
		
		if (hasObjVar(parentContainer, "vendor"))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isInBazaar(obj_id item) throws InterruptedException
	{
		obj_id ownerContainer = getContainedBy(item);
		if (!isIdValid(ownerContainer))
		{
			return false;
		}
		
		if (hasScript(ownerContainer, "terminal.bazaar"))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean outOfRange(obj_id self, obj_id player, float distance, boolean message) throws InterruptedException
	{
		
		if (isGod(player))
		{
			return false;
		}
		
		location a = getLocation(self);
		location b = getLocation(player);
		if (a.cell == b.cell && a.distance(b) < distance)
		{
			return false;
		}
		
		if (message)
		{
			sendSystemMessage(player, SID_OUT_OF_RANGE);
		}
		
		return true;
	}
	
	
	public static float roundFloatByDecimal(float number) throws InterruptedException
	{
		float tempNumber = number *100;
		int result = (int)Math.round(tempNumber);
		float finalResult = (float)result/100;
		return finalResult;
	}
	
	
	public static void housePackingDecreaseIncrease(obj_id objPlayer, int change) throws InterruptedException
	{
		
		if (change > 0)
		{
			
			int[] housePacking = new int[utils.HOUSE_MAX];
			
			if (!hasObjVar (objPlayer, "housePackup"))
			{
				setObjVar (objPlayer, "housePackup", housePacking);
			}
			
			housePacking = getIntArrayObjVar (objPlayer, "housePackup");
			
			if (housePacking.length != utils.HOUSE_MAX)
			{
				return;
			}
			
			housePacking[0]++;
			housePacking[1]++;
			setObjVar (objPlayer, "housePackup", housePacking);
			
			if (!hasObjVar (objPlayer, "dailyHousePackup"))
			{
				setObjVar (objPlayer, "dailyHousePackup", 1);
				int resetTime = getGameTime() + player_structure.TIME_TO_NEXT_PACKUP;
				setObjVar (objPlayer, "maxHousePackupTimer", resetTime);
			}
			else
			{
				int dailyHousePacking = getIntObjVar (objPlayer, "dailyHousePackup");
				dailyHousePacking++;
				setObjVar (objPlayer, "dailyHousePackup", dailyHousePacking);
			}
			
			if (!badge.hasBadge (objPlayer, "house_packup_badge"))
			{
				int[] packupObjVar = getIntArrayObjVar (objPlayer, "housePackup");
				
				if (packupObjVar != null && packupObjVar.length > 0)
				{
					if (packupObjVar[1] >= player_structure.NUM_NEEDED_PACKUP_FIRST_BADGE)
					{
						badge.grantBadge (objPlayer, "house_packup_badge");
					}
				}
			}
			if (badge.hasBadge (objPlayer, "house_packup_badge"))
			{
				if (!badge.hasBadge (objPlayer, "house_packup_badge_master"))
				{
					int[] packupObjVar = getIntArrayObjVar (objPlayer, "housePackup");
					
					if (packupObjVar != null && packupObjVar.length > 0)
					{
						if (packupObjVar[1] >= player_structure.NUM_NEEDED_PACKUP_SECOND_BADGE)
						{
							badge.grantBadge (objPlayer, "house_packup_badge_master");
						}
					}
				}
			}
		}
		
		else
		{
			
			int[] housePacking = new int[utils.HOUSE_MAX];
			
			housePacking = getIntArrayObjVar (objPlayer, "housePackup");
			
			if (housePacking.length != utils.HOUSE_MAX)
			{
				return;
			}
			
			housePacking[0]--;
			setObjVar (objPlayer, "housePackup", housePacking);
		}
	}
	
	
	public static boolean isInHouseCellSpace(obj_id object) throws InterruptedException
	{
		if (isIdValid(object))
		{
			
			if (isNestedWithinAPlayer(object))
			{
				return false;
			}
			
			obj_id house = getTopMostContainer(object);
			if (isIdValid(house) && !player_structure.isBuilding(house))
			{
				return false;
			}
			
			obj_id container = getContainedBy(object);
			if (isIdValid(container) && (getContainedBy(container) != house))
			{
				return false;
			}
			
			if (!hasObjVar(house, "player_structure.admin.adminList"))
			{
				return false;
			}
			
			return true;
		}
		
		return false;
	}
	
	
	public static boolean hasResourceInInventory(obj_id player, String resource) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		if (resource == null || resource.equals(""))
		{
			return false;
		}
		
		obj_id[] contents = getFilteredPlayerContents(player);
		if ((contents == null) || (contents.length == 0))
		{
			return false;
		}
		
		for (int n = 0; n < contents.length; n++)
		{
			testAbortScript();
			int got = getGameObjectType(contents[n]);
			if (isGameObjectTypeOf(got, GOT_resource_container))
			{
				obj_id resourceCrate = contents[n];
				if (isIdValid(resourceCrate))
				{
					obj_id resourceId = getResourceContainerResourceType(resourceCrate);
					if (isIdValid(resourceId))
					{
						if (isResourceDerivedFrom(resourceId, resource))
						{
							return true;
						}
						
					}
				}
			}
		}
		return false;
	}
	
	
	public static obj_id[] validateObjIdArray(obj_id[] source) throws InterruptedException
	{
		Vector target = new Vector();
		target.setSize(0);
		
		for (int i = 0; i < source.length; i++)
		{
			testAbortScript();
			if (isIdValid(source[i]) && exists(source[i]))
			{
				addElement(target, source[i]);
			}
		}
		
		if (target != null)
		{
			source = new obj_id[target.size()];
			target.toArray(source);
		}
		
		return source;
	}
	
	
	public static obj_id[] getPlayersInBuildoutRow(obj_id player) throws InterruptedException
	{
		location loc = getLocation(player);
		return getPlayersInBuildoutRow(loc.area, locations.getBuildoutAreaRow(player));
	}
	
	
	public static obj_id[] getPlayersInBuildoutDimensions(String scene, float x1, float x2, float z1, float z2) throws InterruptedException
	{
		float centX = Math.abs((x2 - x1) / 2.0f) + x1;
		float centZ = Math.abs((z2 - z1) / 2.0f) + z1;
		
		location centerLoc = new location(centX, 0.0f, centZ, scene);
		location lowerLeft = new location(x1, 0.0f, z1, scene);
		
		float hypotenuse = getDistance(centerLoc, lowerLeft);
		
		obj_id[] allPlayers = getAllPlayers(centerLoc, hypotenuse * 3.0f);
		
		Vector playersInArea = new Vector();
		playersInArea.setSize(0);
		
		if (allPlayers == null || allPlayers.length == 0)
		{
			return null;
		}
		
		for (int i = 0; i < allPlayers.length; i++)
		{
			testAbortScript();
			location loc = getLocation(trial.getTop(allPlayers[i]));
			
			if (loc.x < x1 || loc.x > x2 || loc.z < z1 || loc.z > z2)
			{
				LOG("doLogging", ""+loc.x+" vs: "+x1+", "+x2+" and "+loc.z+" vs: "+z1+", "+z2);
				continue;
			}
			
			utils.addElement(playersInArea, allPlayers[i]);
		}
		
		if (playersInArea == null || playersInArea.size() == 0)
		{
			return null;
		}
		
		obj_id[] _playersInArea = new obj_id[0];
		if (playersInArea != null)
		{
			_playersInArea = new obj_id[playersInArea.size()];
			playersInArea.toArray(_playersInArea);
		}
		return _playersInArea;
	}
	
	
	public static dictionary getCoordinatesInBuildoutRow(String scene, int buildout_row) throws InterruptedException
	{
		String datatable = "datatables/buildout/areas_"+scene+".iff";
		dictionary data = dataTableGetRow(datatable, buildout_row);
		
		return data;
	}
	
	
	public static obj_id[] getPlayersInBuildoutRow(String scene, int buildout_row) throws InterruptedException
	{
		String datatable = "datatables/buildout/areas_"+scene+".iff";
		dictionary data = dataTableGetRow(datatable, buildout_row);
		
		return getPlayersInBuildoutDimensions(scene, data.getFloat("x1"), data.getFloat("x2"), data.getFloat("z1"), data.getFloat("z2"));
	}
	
	
	public static obj_id[] getPlayersInBuildoutArea(String scene, String buildout_area) throws InterruptedException
	{
		String datatable = "datatables/buildout/areas_"+scene+".iff";
		int rowNum = dataTableSearchColumnForString(buildout_area, 0, datatable);
		
		if (rowNum == -1)
		{
			return null;
		}
		
		return getPlayersInBuildoutRow(scene, rowNum);
	}
	
	
	public static obj_id[] getAllNpcInBuildoutArea(obj_id baseObject) throws InterruptedException
	{
		location loc = getLocation(baseObject);
		int buildout_area = locations.getBuildoutAreaRow(baseObject);
		
		obj_id[] allObjects = getAllObjectsInBuildoutArea(loc.area, buildout_area);
		
		if (allObjects == null || allObjects.length == 0)
		{
			return null;
		}
		
		Vector allNpc = new Vector();
		allNpc.setSize(0);
		
		for (int i=0; i<allObjects.length; i++)
		{
			testAbortScript();
			if (!isMob(allObjects[i]))
			{
				continue;
			}
			
			allNpc.add(allObjects[i]);
		}
		
		if (allNpc == null || allNpc.size() == 0)
		{
			return null;
		}
		
		if (allNpc != null)
		{
			allObjects = new obj_id[allNpc.size()];
			allNpc.toArray(allObjects);
		}
		return allObjects;
	}
	
	
	public static obj_id[] getAllObjectsInBuildoutArea(obj_id baseObject) throws InterruptedException
	{
		location loc = getLocation(baseObject);
		int buildout_row = locations.getBuildoutAreaRow(baseObject);
		
		return getAllObjectsInBuildoutArea(loc.area, buildout_row);
	}
	
	
	public static obj_id[] getAllObjectsInBuildoutArea(String scene, int buildout_row) throws InterruptedException
	{
		String datatable = "datatables/buildout/areas_"+scene+".iff";
		dictionary data = dataTableGetRow(datatable, buildout_row);
		
		float x1 = data.getFloat("x1");
		float x2 = data.getFloat("x2");
		float z1 = data.getFloat("z1");
		float z2 = data.getFloat("z2");
		
		float centX = Math.abs((data.getFloat("x2") - data.getFloat("x1")) / 2.0f) + data.getFloat("x1");
		float centZ = Math.abs((data.getFloat("z2") - data.getFloat("z1")) / 2.0f) + data.getFloat("z1");
		
		location centerLoc = new location(centX, 0.0f, centZ, scene);
		location lowerLeft = new location(data.getFloat("x1"), 0.0f, data.getFloat("z1"), scene);
		
		float hypotenuse = getDistance(centerLoc, lowerLeft);
		
		obj_id[] allObjects = getObjectsInRange(centerLoc, hypotenuse * 3.0f);
		
		Vector objectsInArea = new Vector();
		objectsInArea.setSize(0);
		
		if (allObjects == null || allObjects.length == 0)
		{
			return null;
		}
		
		for (int i=0; i<allObjects.length; i++)
		{
			testAbortScript();
			location testLoc = getLocation(trial.getTop(allObjects[i]));
			if (testLoc.x < x1 || testLoc.x > x2 || testLoc.z < z1 || testLoc.z > z2)
			{
				continue;
			}
			
			objectsInArea.add(allObjects[i]);
			
		}
		
		if (objectsInArea == null || objectsInArea.size() == 0)
		{
			return null;
		}
		
		obj_id[] _objectsInArea = new obj_id[0];
		if (objectsInArea != null)
		{
			_objectsInArea = new obj_id[objectsInArea.size()];
			objectsInArea.toArray(_objectsInArea);
		}
		return _objectsInArea;
		
	}
	
	
	public static Vector shuffleArray(Vector list) throws InterruptedException
	{
		Vector newList = new Vector();
		
		for (int size = list.size(); size > 0; size--)
		{
			testAbortScript();
			
			int i = rand(0, (size - 1));
			
			newList = utils.addElement(newList, list.get(i));
			list = utils.removeElementAt(list, i);
		}
		
		if (newList == null || newList.size() == 0)
		{
			return null;
		}
		
		return newList;
		
	}
	
	
	public static boolean grantGift(obj_id player) throws InterruptedException
	{
		
		if ((getCurrentBirthDate() - getPlayerBirthDate(player)) < 10)
		{
			return false;
		}
		
		if (hasObjVar(player, XMAS_RECEIVED_VI1))
		{
			return false;
		}
		
		if (!isIdValid(player))
		{
			return false;
		}
		
		String config = getConfigSetting("GameServer", "grantGift");
		if (config != null)
		{
			if (config.equals("false"))
			{
				return false;
			}
		}
		
		if (isInTutorialArea(player))
		{
			setObjVar(player, XMAS_NOT_RECEIVED_TUTORIAL, 1);
			CustomerServiceLog("grantGift", getFirstName(player) + "("+ player + ") did not receive Christmas '06 gift because they logged in on the tutorial planet.");
			return false;
		}
		
		removeObjVar(player, XMAS_NOT_RECEIVED_TUTORIAL);
		
		obj_id inv = utils.getInventoryContainer(player);
		if (!isIdValid(inv))
		{
			CustomerServiceLog("grantGift", getFirstName(player) + "("+ player + ") did not have an inventory container. Didn't receive Christmas '06 gift, but not blocked from future attempt.");
			return false;
		}
		
		obj_id giftSelf = static_item.createNewItemFunction( "item_lifeday_gift_self_01_02", inv);
		
		obj_id giftOther = static_item.createNewItemFunction( "item_lifeday_gift_other_01_02", inv);
		setObjVar(giftOther, LIFEDAY_OWNER, player);
		
		utils.sendMail(GIFT_GRANTED_SUB, GIFT_GRANTED, player, "System");
		setObjVar(player, XMAS_RECEIVED_VI1, 1);
		CustomerServiceLog("grantGift", getFirstName(player) + "("+ player + ") has received his Christmas '06 gift.");
		
		return true;
	}
	
	
	public static void fullExpertiseReset(obj_id player, boolean storeBeast) throws InterruptedException
	{
		
		if (storeBeast)
		{
			if (beast_lib.isBeastMaster(player))
			{
				beast_lib.storeBeasts(player);
			}
		}
		
		if (utils.isProfession(player, utils.FORCE_SENSITIVE))
		{
			if (buff.isInStance(player))
			{
				buff.removeBuff(player, jedi.JEDI_STANCE);
			}
			
			if (buff.isInFocus(player))
			{
				buff.removeBuff(player, jedi.JEDI_FOCUS);
			}
		}
		
		respec.setRespecVersion(player);
		
		buff.removeAllBuffs(player, false, true);
		
		resetExpertises(player);
	}
	
	
	public static obj_id[] getAllRidersInVehicle(obj_id player, obj_id vehicle) throws InterruptedException
	{
		
		if (!isIdValid(player) || !isIdValid(vehicle))
		{
			return null;
		}
		if (!exists(player) || !exists(vehicle))
		{
			return null;
		}
		
		Vector resizeList = new Vector();
		resizeList.setSize(0);
		
		for (int i = 1; i <= 25; ++i)
		{
			testAbortScript();
			
			obj_id tempRider = getObjectInSlot(vehicle, "rider"+i);
			
			if (!isIdValid(tempRider) || !exists(tempRider))
			{
				continue;
			}
			
			resizeList = addElement(resizeList, tempRider);
		}
		
		obj_id[] _resizeList = new obj_id[0];
		if (resizeList != null)
		{
			_resizeList = new obj_id[resizeList.size()];
			resizeList.toArray(_resizeList);
		}
		return _resizeList;
	}
	
	
	public static void updateCTSObjVars(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return;
		}
		
		boolean retrievedCtsOjbvars = false;
		dictionary[] ctsOjbvars = null;
		
		String updateObjvar = utils.CTS_OBJVAR_HISTORY + ".1";
		if (!hasObjVar(player, updateObjvar))
		{
			if ((ctsOjbvars == null) && !retrievedCtsOjbvars)
			{
				ctsOjbvars = getCharacterRetroactiveCtsObjvars(player);
				retrievedCtsOjbvars = true;
			}
			
			if ((ctsOjbvars != null) && (ctsOjbvars.length > 0))
			{
				utils.updateRespecCTSObjvars(player, ctsOjbvars);
				utils.updateBeastMasterCTSObjvars(player, ctsOjbvars);
			}
			
			setObjVar(player, updateObjvar, 1);
		}
		
		updateObjvar = utils.CTS_OBJVAR_HISTORY + ".2";
		if (!hasObjVar(player, updateObjvar))
		{
			if ((ctsOjbvars == null) && !retrievedCtsOjbvars)
			{
				ctsOjbvars = getCharacterRetroactiveCtsObjvars(player);
				retrievedCtsOjbvars = true;
			}
			
			if ((ctsOjbvars != null) && (ctsOjbvars.length > 0))
			{
				utils.updateHousePackupCTSObjvars(player, ctsOjbvars);
			}
			
			setObjVar(player, updateObjvar, 1);
		}
		
	}
	
	
	public static void updateRespecCTSObjvars(obj_id player, dictionary[] ctsOjbvars) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return;
		}
		
		if ((ctsOjbvars == null) || (ctsOjbvars.length <= 0))
		{
			return;
		}
		
		final int indexMostRecentTransfer = ctsOjbvars.length - 1;
		if ((ctsOjbvars[indexMostRecentTransfer] != null) && ctsOjbvars[indexMostRecentTransfer].containsKey("respecsBought") && ctsOjbvars[indexMostRecentTransfer].isInt("respecsBought"))
		{
			final int respecsBoughtFromTransfer = ctsOjbvars[indexMostRecentTransfer].getInt("respecsBought");
			int respecsBoughtCurrentValue = 0;
			if (hasObjVar(player, "respecsBought"))
			{
				respecsBoughtCurrentValue = getIntObjVar(player, "respecsBought");
			}
			
			if (respecsBoughtFromTransfer > respecsBoughtCurrentValue)
			{
				CustomerServiceLog("CharacterTransferRetroactiveHistory", "changing respecsBought objvar for "+ player + " from "+ respecsBoughtCurrentValue + " to "+ respecsBoughtFromTransfer);
				setObjVar(player, "respecsBought", respecsBoughtFromTransfer);
			}
		}
		
		int[] recpecLevelsFromTransfer =
		{
			-1, -1, -1
		};
		for (int i = 0; i < ctsOjbvars.length; ++i)
		{
			testAbortScript();
			if ((ctsOjbvars[i] != null) && ctsOjbvars[i].containsKey(respec.PROF_LEVEL_ARRAY) && ctsOjbvars[i].isIntArray(respec.PROF_LEVEL_ARRAY))
			{
				int[] levelArray = ctsOjbvars[i].getIntArray(respec.PROF_LEVEL_ARRAY);
				if ((levelArray != null) && (levelArray.length == 3))
				{
					if (levelArray[0] > recpecLevelsFromTransfer[0])
					{
						recpecLevelsFromTransfer[0] = levelArray[0];
					}
					
					if (levelArray[1] > recpecLevelsFromTransfer[1])
					{
						recpecLevelsFromTransfer[1] = levelArray[1];
					}
					
					if (levelArray[2] > recpecLevelsFromTransfer[2])
					{
						recpecLevelsFromTransfer[2] = levelArray[2];
					}
				}
			}
		}
		
		if ((recpecLevelsFromTransfer[0] > -1) || (recpecLevelsFromTransfer[1] > -1) || (recpecLevelsFromTransfer[2] > -1))
		{
			int[] recpecLevelsCurrentValue =
			{
				-1, -1, -1
			};
			if (hasObjVar(player, respec.PROF_LEVEL_ARRAY))
			{
				recpecLevelsCurrentValue = getIntArrayObjVar(player, respec.PROF_LEVEL_ARRAY);
			}
			
			if ((recpecLevelsCurrentValue != null) && (recpecLevelsCurrentValue.length == 3))
			{
				int[] recpecLevelsNewValue =
				{
					-1, -1, -1
				};
				recpecLevelsNewValue[0] = Math.max(recpecLevelsCurrentValue[0], recpecLevelsFromTransfer[0]);
				recpecLevelsNewValue[1] = Math.max(recpecLevelsCurrentValue[1], recpecLevelsFromTransfer[1]);
				recpecLevelsNewValue[2] = Math.max(recpecLevelsCurrentValue[2], recpecLevelsFromTransfer[2]);
				
				if ((recpecLevelsNewValue[0] != recpecLevelsCurrentValue[0]) || (recpecLevelsNewValue[1] != recpecLevelsCurrentValue[1]) || (recpecLevelsNewValue[2] != recpecLevelsCurrentValue[2]))
				{
					CustomerServiceLog("CharacterTransferRetroactiveHistory", "changing "+ respec.PROF_LEVEL_ARRAY + " objvar for "+ player + " from ("+ recpecLevelsCurrentValue[0] + ","+ recpecLevelsCurrentValue[1] + ","+ recpecLevelsCurrentValue[2] + ") to ("+ recpecLevelsNewValue[0] + ","+ recpecLevelsNewValue[1] + ","+ recpecLevelsNewValue[2] + ")");
					setObjVar(player, respec.PROF_LEVEL_ARRAY, recpecLevelsNewValue);
				}
			}
		}
		
		if (hasObjVar(player, respec.PROF_LEVEL_ARRAY))
		{
			
			String skillTemplate = getSkillTemplate(player);
			
			int[] recpecLevelsValue = getIntArrayObjVar(player, respec.PROF_LEVEL_ARRAY);
			
			if ((skillTemplate != null) && (skillTemplate.length() > 0) && (recpecLevelsValue != null) && (recpecLevelsValue.length == 3))
			{
				
				int currentLevel = getLevel(player);
				
				if (!skillTemplate.startsWith("entertainer") && !skillTemplate.startsWith("trader"))
				{
					
					int combatLevel = recpecLevelsValue[respec.PROF_LEVEL_COMBAT];
					
					if (currentLevel < combatLevel)
					{
						respec.autoLevelPlayer(player, combatLevel, true);
					}
				}
				
				else if (skillTemplate.startsWith("entertainer"))
				{
					
					int entLevel = recpecLevelsValue[respec.PROF_LEVEL_ENT];
					
					if (currentLevel < entLevel)
					{
						respec.autoLevelPlayer(player, entLevel, true);
					}
				}
				
				else if (skillTemplate.startsWith("trader"))
				{
					
					int traderLevel = recpecLevelsValue[respec.PROF_LEVEL_TRADER];
					
					if (currentLevel < traderLevel)
					{
						respec.autoLevelPlayer(player, traderLevel, true);
					}
				}
			}
		}
	}
	
	
	public static void updateBeastMasterCTSObjvars(obj_id player, dictionary[] ctsOjbvars) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return;
		}
		
		if ((ctsOjbvars == null) || (ctsOjbvars.length <= 0))
		{
			return;
		}
		
		HashSet abilities = new HashSet();
		
		for (int i = 0; i < ctsOjbvars.length; ++i)
		{
			testAbortScript();
			if (ctsOjbvars[i] != null)
			{
				int j = 0;
				while (true)
				{
					testAbortScript();
					final String objVarName = beast_lib.PLAYER_KNOWN_SKILLS_LIST + "."+ j;
					if (ctsOjbvars[i].containsKey(objVarName) && ctsOjbvars[i].isIntArray(objVarName))
					{
						final int[] objVarValue = ctsOjbvars[i].getIntArray(objVarName);
						if ((objVarValue != null) && (objVarValue.length > 0))
						{
							for (int k = 0; k < objVarValue.length; ++k)
							{
								testAbortScript();
								abilities.add(new Integer(objVarValue[k]));
							}
						}
						
						++j;
					}
					else
					{
						break;
					}
				}
			}
		}
		
		if (!abilities.isEmpty())
		{
			
			Vector abilitiesCurrent = utils.getResizeableIntBatchObjVar(player, beast_lib.PLAYER_KNOWN_SKILLS_LIST);
			
			boolean updateRequired = false;
			if ((abilitiesCurrent == null) || (abilitiesCurrent.size() <= 0))
			{
				updateRequired = true;
			}
			else
			{
				
				HashSet abilitiesCurrentNoDupes = new HashSet();
				for (int i = 0; i < abilitiesCurrent.size(); ++i)
				{
					testAbortScript();
					abilitiesCurrentNoDupes.add(new Integer(((Integer)(abilitiesCurrent.get(i))).intValue()));
				}
				
				Iterator abilitiesIterator = abilitiesCurrentNoDupes.iterator();
				while (abilitiesIterator.hasNext())
				{
					testAbortScript();
					Integer ability = (Integer)abilitiesIterator.next();
					abilities.add(ability);
				}
				
				if (abilities.size() != abilitiesCurrentNoDupes.size())
				{
					updateRequired = true;
				}
			}
			
			if (updateRequired)
			{
				int[] abilitiesNew = new int[abilities.size()];
				Iterator abilitiesIterator = abilities.iterator();
				String strAbilitiesNew = new String();
				int i = 0;
				while (abilitiesIterator.hasNext() && (i < abilitiesNew.length))
				{
					testAbortScript();
					Integer ability = (Integer)abilitiesIterator.next();
					abilitiesNew[i] = ability.intValue();
					strAbilitiesNew += ""+ abilitiesNew[i] + ", ";
					
					++i;
				}
				
				String strAbilitiesCurrent = new String();
				if ((abilitiesCurrent != null) && (abilitiesCurrent.size() > 0))
				{
					for (int j = 0; j < abilitiesCurrent.size(); ++j)
					{
						testAbortScript();
						strAbilitiesCurrent += ""+ ((Integer)(abilitiesCurrent.get(j))).intValue() + ", ";
					}
				}
				
				CustomerServiceLog("CharacterTransferRetroactiveHistory", "changing "+ beast_lib.PLAYER_KNOWN_SKILLS_LIST + " objvar for "+ player + " from ("+ strAbilitiesCurrent + ") to ("+ strAbilitiesNew + ")");
				
				utils.setBatchObjVar(player, beast_lib.PLAYER_KNOWN_SKILLS_LIST, abilitiesNew);
			}
		}
	}
	
	
	public static void updateHousePackupCTSObjvars(obj_id player, dictionary[] ctsOjbvars) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return;
		}
		
		if ((ctsOjbvars == null) || (ctsOjbvars.length <= 0))
		{
			return;
		}
		
		int[] housePackupNewValues =
		{
			0, 0
		};
		for (int i = 0; i < ctsOjbvars.length; ++i)
		{
			testAbortScript();
			if ((ctsOjbvars[i] != null) && ctsOjbvars[i].containsKey(player_structure.HOUSE_PACKUP_ARRAY_OBJVAR) && ctsOjbvars[i].isIntArray(player_structure.HOUSE_PACKUP_ARRAY_OBJVAR))
			{
				int[] housePackupCTSValues = ctsOjbvars[i].getIntArray(player_structure.HOUSE_PACKUP_ARRAY_OBJVAR);
				if ((housePackupCTSValues != null) && (housePackupCTSValues.length == 2))
				{
					if (housePackupCTSValues[0] > 0)
					{
						housePackupNewValues[0] += housePackupCTSValues[0];
					}
					
					if (housePackupCTSValues[1] > 0)
					{
						housePackupNewValues[1] += housePackupCTSValues[1];
					}
				}
			}
		}
		
		if ((housePackupNewValues[0] > 0) || (housePackupNewValues[1] > 0))
		{
			int[] housePackupCurrentValues =
			{
				0, 0
			};
			if (hasObjVar(player, player_structure.HOUSE_PACKUP_ARRAY_OBJVAR))
			{
				housePackupCurrentValues = getIntArrayObjVar(player, player_structure.HOUSE_PACKUP_ARRAY_OBJVAR);
			}
			
			if ((housePackupCurrentValues != null) && (housePackupCurrentValues.length == 2))
			{
				housePackupNewValues[0] += housePackupCurrentValues[0];
				housePackupNewValues[1] += housePackupCurrentValues[1];
				
				CustomerServiceLog("CharacterTransferRetroactiveHistory", "changing "+ player_structure.HOUSE_PACKUP_ARRAY_OBJVAR + " objvar for "+ player + " from ("+ housePackupCurrentValues[0] + ", "+ housePackupCurrentValues[1] + ") to ("+ housePackupNewValues[0] + ", "+ housePackupNewValues[1] + ")");
				setObjVar(player, player_structure.HOUSE_PACKUP_ARRAY_OBJVAR, housePackupNewValues);
			}
		}
	}
	
	
	public static String getOnlineOfflineStatus(obj_id player) throws InterruptedException
	{
		String returnData = "";
		
		if (!isPlayer(player))
		{
			returnData = "Deleted";
		}
		else if (!isPlayerConnected(player))
		{
			int lastLoginTime = getPlayerLastLoginTime(player);
			
			if (lastLoginTime > 0)
			{
				int timeDifference = getCalendarTime() - lastLoginTime;
				
				if (timeDifference > 0)
				{
					returnData = "Offline "+ utils.padTimeDHMS(timeDifference);
				}
				else
				{
					returnData = "Offline ????d:??h:??m:??s";
				}
			}
			else
			{
				returnData = "Unknown";
			}
		}
		else
		{
			String locText = "Unknown";
			dictionary playerLoc = getConnectedPlayerLocation(player);
			
			if (playerLoc == null)
			{
				return locText;
			}
			
			String planet = playerLoc.getString("planet");
			
			if (planet != null && planet.length() > 0)
			{
				locText = localize(new string_id("planet_n", planet));
			}
			
			String region = playerLoc.getString("region");
			
			if (region != null && region.length() > 0)
			{
				locText += ": "+ localize(new string_id(region.substring(1, region.indexOf(":")), region.substring(region.indexOf(":") + 1, region.length())));
			}
			
			String city = playerLoc.getString("playerCity");
			
			if (city != null && city.length() > 0)
			{
				locText += ", "+ city;
			}
			
			returnData = "Online "+ locText;
		}
		return returnData;
	}
	
	
	public static String localizeSIDString(String text) throws InterruptedException
	{
		if (text == null || text.length() < 1)
		{
			return null;
		}
		
		int left = 0;
		
		if (text.startsWith("@"))
		{
			left = 1;
		}
		
		if (text.indexOf(":") < 0)
		{
			return text;
		}
		
		return localize(new string_id(text.substring(left, text.indexOf(":")), text.substring(text.indexOf(":") + 1, text.length())));
	}
	
	
	public static boolean colorizeItemFromWidget(obj_id player, obj_id item, String params) throws InterruptedException
	{
		
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		if (!isValidId(item) || !exists(item))
		{
			utils.removeScriptVar(player, "veteranRewardItemColor.color_setting");
			return false;
		}
		
		if (params == null || params.equals(""))
		{
			utils.removeScriptVar(player, "veteranRewardItemColor.color_setting");
			return false;
		}
		
		String[] colorArray = split(params, ' ');
		if (colorArray == null || colorArray.length <= 0)
		{
			utils.removeScriptVar(player, "veteranRewardItemColor.color_setting");
			return false;
		}
		
		for (int i = 0; i < colorArray.length; i+=2)
		{
			testAbortScript();
			if (colorArray[i] == null || colorArray[i].equals(""))
			{
				break;
			}
			hue.setColor(item, colorArray[i], utils.stringToInt(colorArray[i+1]));
		}
		utils.removeScriptVar(player, "veteranRewardItemColor.color_setting");
		return true;
	}
	
	
	public static boolean validateSkillModsAttached(obj_id item) throws InterruptedException
	{
		return true;
	}
	
	
	public static int getNumCreaturesForSpawnLimit() throws InterruptedException
	{
		
		final int intNumCreatures = getNumAI() - (getNumHibernatingAI() / 2);
		if (intNumCreatures <= 0)
		{
			return 0;
		}
		
		return intNumCreatures;
	}
	
}
