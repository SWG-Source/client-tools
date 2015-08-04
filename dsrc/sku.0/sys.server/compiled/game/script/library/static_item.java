package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.jedi;
import java.lang.Math;
import script.library.loot;
import script.library.storyteller;


public class static_item extends script.base_script
{
	public static_item()
	{
	}
	public static final int NUM_DYNAMIC_MODIFIERS = 3;
	
	public static final String MASTER_ITEM_TABLE = "datatables/item/master_item/master_item.iff";
	public static final String WEAPON_STAT_BALANCE_TABLE = "datatables/item/master_item/weapon_stats.iff";
	public static final String ARMOR_STAT_BALANCE_TABLE = "datatables/item/master_item/armor_stats.iff";
	public static final String ITEM_STAT_BALANCE_TABLE = "datatables/item/master_item/item_stats.iff";
	public static final String WEAPON_EXAMINE_SCRIPT = "systems.combat.combat_weapon";
	public static final string_id SID_NOT_LINKED = new string_id("base_player", "not_linked");
	public static final string_id SID_NOT_LINKED_TO_HOLDER = new string_id("base_player", "not_linked_to_holder");
	public static final string_id ALREADY_HAVE_SIMILAR_BUFF = new string_id("base_player", "already_have_similar_buff");
	public static final string_id BUFF_APPLIED = new string_id("base_player", "buff_applied");
	public static final string_id SID_ITEM_LEVEL_TOO_LOW = new string_id("base_player", "level_too_low_for_effect");
	public static final string_id SID_UNIQUE_NO_CREATE = new string_id("base_player", "unique_failed_create");
	public static final string_id SID_ITEM_NOT_ENOUGH_SKILL = new string_id("base_player", "not_correct_skill");
	public static final String STATIC_ITEM_NAME = "static_item_n";
	public static final String ORIG_OWNER = "origOwner";
	
	public static final int IT_WEAPON = 1;
	public static final int IT_ARMOR = 2;
	public static final int IT_ITEM = 3;
	
	public static final java.text.NumberFormat noDecimalFormat = new java.text.DecimalFormat("###");
	
	public static final String SET_BONUS_TABLE = "datatables/item/item_sets.iff";
	
	
	public static obj_id createNewItemFunction(String itemName, obj_id container) throws InterruptedException
	{
		return createNewItemFunction (itemName, container, null);
	}
	
	
	public static obj_id createNewItemFunction(String itemName, obj_id container, location pos) throws InterruptedException
	{
		
		if (itemName == null || itemName.equals(""))
		{
			LOG( "loot", itemName + " could not be made because itemName is null");
			return null;
		}
		if (isIdNull(container))
		{
			LOG( "loot", itemName + " could not be made because inital container is bad "+container );
			return null;
		}
		
		if (isUniqueStaticItem(itemName))
		{
			if (!canCreateUniqueStaticItem(container, itemName))
			{
				sendSystemMessage(container, SID_UNIQUE_NO_CREATE);
				return null;
			}
		}
		
		dictionary itemData = new dictionary();
		itemData = dataTableGetRow(MASTER_ITEM_TABLE, itemName);
		if (itemData == null)
		{
			LOG( "loot", itemName + " could not be made because row in datatable is bad");
			return null;
		}
		
		if (getContainerType(container) != 2 && pos == null)
		{
			container = utils.getInventoryContainer(container);
			
			if (getContainerType(container) != 2)
			{
				LOG( "loot", itemName + " could not be made because we failed the inventory container check");
				return null;
			}
		}
		
		obj_id newItem = null;
		
		if (pos != null)
		{
			if (isValidInteriorLocation(pos))
			{
				newItem = createObject (itemData.getString ("template_name"), pos);
			}
		}
		else
		{
			
			if (utils.isNestedWithinAPlayer(container))
			{
				newItem = createObjectOverloaded(itemData.getString ("template_name"), container);
			}
			else
			{
				newItem = createObject(itemData.getString ("template_name"), container, "");
			}
		}
		
		if (!isIdValid(newItem))
		{
			LOG( "loot", itemName + " could not be made because item is not valid");
			return null;
		}
		
		setStaticItemName(newItem, itemName );
		String objVarList = itemData.getString("creation_objvars");
		
		if (objVarList != null && !objVarList.equals(""))
		{
			utils.setObjVarsList(newItem, objVarList);
		}
		
		setStaticItemVersion(newItem, itemData.getInt("version"));
		
		int chargeList = itemData.getInt("charges");
		
		if (chargeList > 0 && chargeList <= 500)
		{
			setCount(newItem, chargeList);
		}
		
		initializeObject(newItem, itemData);
		
		return newItem;
	}
	
	
	public static void initializeObject(obj_id object, dictionary itemData) throws InterruptedException
	{
		
		String scriptList = itemData.getString("scripts");
		String itemName = itemData.getString("name");
		
		if (itemName == null || itemName.equals(""))
		{
			LOG( "create", object + " InitializeObject FAILED: No NAME field in datatable or bad dictionary passed in");
			return;
		}
		
		if (scriptList != null && !scriptList.equals(""))
		{
			String[] scriptArray = split(scriptList, ',');
			for (int i = 0; i < scriptArray.length; i++)
			{
				testAbortScript();
				if (!hasScript(object,scriptArray[i]))
				{
					attachScript( object, scriptArray[i] );
				}
			}
		}
		
		attachScript(object, "item.static_item_base");
		
		if (!jedi.isCrystalTuned(object))
		{
			setName(object, "");
		}
		
		if (exists(object) && !hasObjVar (object, "playerQuest"))
		{
			setName(object, new string_id("static_item_n", itemName));
		}
		
		setDescriptionStringId(object, new string_id("static_item_d", itemName));
		LOG("npe", "MAKING THING");
		switch (itemData.getInt("type"))
		{
			case 1:
			initializeWeapon(object,itemName);
			break;
			case 2:
			initializeArmor(object,itemName);
			break;
			case 3:
			initializeItem(object,itemName);
			break;
			case 4:
			initializeStorytellerObject(object, itemName);
			break;
			case 5:
			break;
		}
		
		return;
	}
	
	
	public static boolean initializeArmor(obj_id object, String itemName) throws InterruptedException
	{
		
		dictionary itemData = new dictionary();
		
		int row = dataTableSearchColumnForString( itemName , 0, ARMOR_STAT_BALANCE_TABLE);
		if (row == -1)
		{
			return false;
		}
		itemData = dataTableGetRow(ARMOR_STAT_BALANCE_TABLE, row);
		
		float condition = itemData.getFloat("condition_multiplier");
		float generalProtection = itemData.getFloat("protection");
		int armorLevel = itemData.getInt("armor_level");
		int armorCategory = itemData.getInt("armor_category");
		int sockets = itemData.getInt("sockets");
		int hitPoints = itemData.getInt("hit_points");
		String skillMods = itemData.getString("skill_mods");
		String attributeBonus = itemData.getString("attribute_bonus");
		String objVarList = itemData.getString("objvars");
		String colorMods = itemData.getString("color");
		
		armor.setArmorDataPercent(object, armorLevel, armorCategory, generalProtection, condition);
		
		if (armorCategory == 0)
		{
			armor.setArmorSpecialProtectionPercent(object, armor.DATATABLE_RECON_LAYER, 1.0f);
		}
		if (armorCategory == 2)
		{
			armor.setArmorSpecialProtectionPercent(object, armor.DATATABLE_ASSAULT_LAYER, 1.0f);
		}
		
		setMaxHitpoints(object, hitPoints);
		setHitpoints(object, hitPoints);
		
		if (sockets > 0)
		{
			setSkillModSockets(object, 1);
		}
		else
		{
			setSkillModSockets(object, 0);
		}
		
		if (attributeBonus != null && !attributeBonus.equals(""))
		{
			
			String[] stringArray = split(attributeBonus, ',');
			for (int i = 0; i < stringArray.length; i++)
			{
				testAbortScript();
				
				String[] attributeArray = split(stringArray[i], '=');
				for (int x = 0; x < attributeArray.length; x++)
				{
					testAbortScript();
					int attribute = 0;
					if (attributeArray[0].equalsIgnoreCase ("health"))
					{
						attribute = HEALTH;
					}
					else if (attributeArray[0].equalsIgnoreCase ("action"))
					{
						attribute = ACTION;
					}
					else if (attributeArray[0].equalsIgnoreCase ("mind"))
					{
						attribute = MIND;
					}
					else if (attributeArray[0].equalsIgnoreCase ("constitution"))
					{
						attribute = CONSTITUTION;
					}
					else if (attributeArray[0].equalsIgnoreCase ("stamina"))
					{
						attribute = STAMINA;
					}
					else if (attributeArray[0].equalsIgnoreCase ("willpower"))
					{
						attribute = WILLPOWER;
					}
					
					setAttributeBonus(object, attribute, utils.stringToInt(attributeArray[1]));
				}
			}
			
		}
		
		dictionary dict = parseSkillModifiers(null, skillMods);
		if (dict != null)
		{
			java.util.Enumeration keys = dict.keys();
			while (keys.hasMoreElements())
			{
				testAbortScript();
				String skillModName = (String)keys.nextElement();
				int skillModValue = dict.getInt(skillModName);
				setSkillModBonus(object, skillModName, skillModValue);
			}
		}
		
		if (objVarList != null && !objVarList.equals(""))
		{
			utils.setObjVarsList(object, objVarList);
		}
		
		if (colorMods != null && !colorMods.equals(""))
		{
			String[] colorArray = split(colorMods, ',');
			for (int i = 0; i < colorArray.length; i++)
			{
				testAbortScript();
				
				String[] colorIndex = split(colorArray[i], '=');
				for (int x = 0; x < colorIndex.length; x++)
				{
					testAbortScript();
					hue.setColor(object, "/private/"+colorIndex[0], Integer.parseInt(colorIndex[1]));
					
				}
			}
		}
		
		return true;
	}
	
	
	public static boolean initializeWeapon(obj_id object, String itemName) throws InterruptedException
	{
		
		dictionary itemData = new dictionary();
		
		int row = dataTableSearchColumnForString( itemName , 0, WEAPON_STAT_BALANCE_TABLE);
		if (row == -1)
		{
			return false;
		}
		itemData = dataTableGetRow(WEAPON_STAT_BALANCE_TABLE, row);
		
		int minDamage = itemData.getInt("min_damage");
		int maxDamage = itemData.getInt("max_damage");
		int attackSpeed = itemData.getInt("attack_speed");
		int woundChance = itemData.getInt("wound_chance");
		int hp = itemData.getInt("hit_points");
		int accuracy = itemData.getInt("accuracy");
		int minRange = itemData.getInt("min_range_distance");
		int maxRange = itemData.getInt("max_range_distance");
		int attackCost = itemData.getInt("special_attack_cost");
		int elementalType = itemData.getInt("elemental_type");
		int elementalDamage = itemData.getInt("elemental_damage");
		int damageType = itemData.getInt("damage_type");
		String skillMods = itemData.getString("skill_mods");
		String objVarList = itemData.getString("objvars");
		
		if (damageType == 1)
		{
			damageType = DAMAGE_KINETIC;
		}
		else
		{
			damageType = DAMAGE_ENERGY;
		}
		
		if (!hasScript(object, WEAPON_EXAMINE_SCRIPT))
		{
			attachScript(object, WEAPON_EXAMINE_SCRIPT);
		}
		
		setObjVar(object, "weapon.original_max_range", (float)maxRange);
		
		setWeaponMinDamage(object, minDamage);
		setWeaponMaxDamage(object, maxDamage);
		setWeaponAttackSpeed(object, (float)attackSpeed/100.0f);
		setWeaponWoundChance(object, (float)woundChance/10.0f);
		setMaxHitpoints(object,(hp));
		setHitpoints(object,(hp));
		setWeaponAccuracy(object, accuracy);
		setWeaponDamageType(object, damageType);
		setWeaponRangeInfo(object, (float)minRange, (float) maxRange);
		setWeaponAttackCost(object, attackCost);
		setWeaponElementalDamage(object, elementalType, elementalDamage);
		weapons.setHeavyWeaponAoeSplashPercent(object);
		
		if (objVarList != null && !objVarList.equals(""))
		{
			utils.setObjVarsList(object, objVarList);
		}
		
		dictionary dict = parseSkillModifiers(null, skillMods);
		if (dict != null)
		{
			java.util.Enumeration keys = dict.keys();
			while (keys.hasMoreElements())
			{
				testAbortScript();
				String skillModName = (String)keys.nextElement();
				int skillModValue = dict.getInt(skillModName);
				setSkillModBonus(object, skillModName, skillModValue);
			}
		}
		weapons.setWeaponData(object);
		
		return true;
	}
	
	
	public static boolean initializeItem(obj_id object, String itemName) throws InterruptedException
	{
		
		dictionary itemData = new dictionary();
		
		int row = dataTableSearchColumnForString( itemName , 0, ITEM_STAT_BALANCE_TABLE);
		if (row == -1)
		{
			return false;
		}
		itemData = dataTableGetRow(ITEM_STAT_BALANCE_TABLE, row);
		
		String skillMods = itemData.getString("skill_mods");
		String attributeBonus = itemData.getString("attribute_bonus");
		String objVarList = itemData.getString("objvars");
		String colorMods = itemData.getString("color");
		
		if (attributeBonus != null && !attributeBonus.equals(""))
		{
			
			String[] stringArray = split(attributeBonus, ',');
			for (int i = 0; i < stringArray.length; i++)
			{
				testAbortScript();
				
				String[] attributeArray = split(stringArray[i], '=');
				for (int x = 0; x < attributeArray.length; x++)
				{
					testAbortScript();
					int attribute = 0;
					if (attributeArray[0].equalsIgnoreCase ("health"))
					{
						attribute = HEALTH;
					}
					else if (attributeArray[0].equalsIgnoreCase ("action"))
					{
						attribute = ACTION;
					}
					else if (attributeArray[0].equalsIgnoreCase ("constitution"))
					{
						attribute = CONSTITUTION;
					}
					else if (attributeArray[0].equalsIgnoreCase ("stamina"))
					{
						attribute = STAMINA;
					}
					
					setAttributeBonus(object, attribute, utils.stringToInt(attributeArray[1]));
				}
			}
			
		}
		
		dictionary dict = parseSkillModifiers(null, skillMods);
		if (dict != null)
		{
			java.util.Enumeration keys = dict.keys();
			while (keys.hasMoreElements())
			{
				testAbortScript();
				String skillModName = (String)keys.nextElement();
				int skillModValue = dict.getInt(skillModName);
				setSkillModBonus(object, skillModName, skillModValue);
			}
		}
		
		if (objVarList != null && !objVarList.equals(""))
		{
			utils.setObjVarsList(object, objVarList);
		}
		
		if (colorMods != null && !colorMods.equals(""))
		{
			String[] colorArray = split(colorMods, ',');
			for (int i = 0; i < colorArray.length; i++)
			{
				testAbortScript();
				
				String[] colorIndex = split(colorArray[i], '=');
				for (int x = 0; x < colorIndex.length; x++)
				{
					testAbortScript();
					hue.setColor(object, "/private/"+colorIndex[0], Integer.parseInt(colorIndex[1]));
					
				}
			}
		}
		
		return true;
	}
	
	
	public static boolean initializeStorytellerObject(obj_id newObject, String itemName) throws InterruptedException
	{
		
		dictionary itemData = new dictionary();
		if (!dataTableOpen(storyteller.STORYTELLER_DATATABLE))
		{
			return false;
		}
		
		int row = dataTableSearchColumnForString( itemName , "name", storyteller.STORYTELLER_DATATABLE);
		if (row == -1)
		{
			return false;
		}
		
		itemData = dataTableGetRow(storyteller.STORYTELLER_DATATABLE, row);
		
		String template = itemData.getString("template_name");
		int type = itemData.getInt("type");
		String objvarString = itemData.getString("objvar");
		String scriptString = itemData.getString("scripts");
		
		setObjVarString(newObject, objvarString, type);
		setScriptString(newObject, scriptString, type);
		
		setName(newObject, getString(new string_id("static_item_n", itemName)) + getString(new string_id("storyteller", "token_label")));
		
		return true;
	}
	
	
	public static void setObjVarString(obj_id object, String objVarString) throws InterruptedException
	{
		setObjVarString(object, objVarString, -1);
	}
	
	
	public static void setObjVarString(obj_id object, String objVarString, int type) throws InterruptedException
	{
		
		switch (type)
		{
			case storyteller.PROP:
			break;
			case storyteller.STATIC_EFFECT:
			break;
			case storyteller.IMMEDIATE_EFFECT:
			break;
			case storyteller.COMBAT_NPC:
			break;
			case storyteller.FLAVOR_NPC:
			break;
			case storyteller.OTHER:
			break;
			case storyteller.THEATER:
			break;
		}
		
		if (objVarString == null || objVarString.equals("") || objVarString.equals("none"))
		{
			return;
		}
		
		String[] parse = split(objVarString, ',');
		
		if (parse == null || parse.length == 0)
		{
			return;
		}
		
		for (int i=0; i<parse.length; i++)
		{
			testAbortScript();
			String[] typeDataSplit = split(parse[i], ':');
			String dataType = typeDataSplit[0];
			String data = typeDataSplit[1];
			
			String[] nameValueSplit = split(data, '=');
			String name = nameValueSplit[0];
			String value = nameValueSplit[1];
			
			if (dataType.equals("int"))
			{
				setObjVar(object, name, utils.stringToInt(value));
			}
			if (dataType.equals("float"))
			{
				setObjVar(object, name, utils.stringToFloat(value));
			}
			if (dataType.equals("string"))
			{
				setObjVar(object, name, value);
			}
			if (dataType.equals("boolean") && (value.equals("true")|| value.equals("1")))
			{
				setObjVar(object, name, true);
			}
			if (dataType.equals("boolean") && (value.equals("false") || value.equals("0")))
			{
				setObjVar(object, name, false);
			}
		}
		
	}
	
	
	public static void setScriptString(obj_id object, String scriptString) throws InterruptedException
	{
		setScriptString(object, scriptString, -1);
	}
	
	
	public static void setScriptString(obj_id object, String scriptString, int type) throws InterruptedException
	{
		
		switch (type)
		{
			case storyteller.PROP:
			attachScript(object, "systems.storyteller.prop_token");
			break;
			case storyteller.STATIC_EFFECT:
			attachScript(object, "systems.storyteller.effect_token");
			break;
			case storyteller.IMMEDIATE_EFFECT:
			attachScript(object, "systems.storyteller.effect_token");
			break;
			case storyteller.COMBAT_NPC:
			attachScript(object, "systems.storyteller.npc_token");
			break;
			case storyteller.FLAVOR_NPC:
			attachScript(object, "systems.storyteller.npc_token");
			break;
			case storyteller.OTHER:
			break;
			case storyteller.THEATER:
			attachScript(object, "systems.storyteller.theater_token");
			break;
		}
		
		if (scriptString == null || scriptString.equals("") || scriptString.equals("none"))
		{
			return;
		}
		
		String[] parse = split(scriptString, ',');
		
		if (parse == null || parse.length == 0)
		{
			return;
		}
		
		for (int i=0; i<parse.length; i++)
		{
			testAbortScript();
			attachScript(object, parse[i]);
		}
		
	}
	
	
	public static boolean validateLevelRequired(obj_id player, int requiredLevel) throws InterruptedException
	{
		int playerLevel = getLevel(player);
		
		if (playerLevel < requiredLevel)
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean validateLevelRequired(obj_id player, obj_id item) throws InterruptedException
	{
		int playerLevel = getLevel(player);
		dictionary itemData = getMasterItemDictionary(item);
		
		if (itemData != null)
		{
			int requiredLevel = itemData.getInt("required_level");
			if (requiredLevel > 0)
			{
				
				if (playerLevel < requiredLevel)
				{
					return false;
				}
			}
		}
		
		return true;
	}
	
	
	public static boolean validateLevelRequiredForWornEffect(obj_id player, obj_id item) throws InterruptedException
	{
		int playerLevel = getLevel(player);
		String itemName = getStaticItemName(item);
		dictionary itemData = getStaticObjectTypeDictionary(itemName);
		
		if (itemData != null)
		{
			int requiredLevel = itemData.getInt("required_level_for_effect");
			if (requiredLevel > 0)
			{
				
				if (playerLevel < requiredLevel)
				{
					return false;
				}
			}
		}
		
		return true;
	}
	
	
	public static void decrementStaticItem(obj_id item) throws InterruptedException
	{
		if (!isIdValid(item))
		{
			return;
		}
		
		int count = getCount(item);
		count--;
		
		if (count <= 0)
		{
			destroyObject(item);
		}
		else
		{
			setCount(item, count);
		}
	}
	
	
	public static void decrementStaticItemAmount(obj_id item, int amount) throws InterruptedException
	{
		if (!isIdValid(item))
		{
			return;
		}
		
		int count = getCount(item);
		int newCount = count - amount;
		
		if (count < amount)
		{
			return;
		}
		else if (newCount <= 0)
		{
			destroyObject(item);
		}
		else
		{
			setCount(item, newCount);
		}
	}
	
	
	public static void decrementUnstackedStaticItemAmount(obj_id[] items, int amount) throws InterruptedException
	{
		int totalCount = amount;
		
		for (int i = 0; i < items.length; i++)
		{
			testAbortScript();
			int itemCount = getCount(items[i]);
			
			if (itemCount <= totalCount)
			{
				destroyObject(items[i]);
			}
			else if (itemCount > totalCount)
			{
				totalCount = itemCount - totalCount;
				setCount(items[i], totalCount);
				return;
			}
			
			totalCount = totalCount - itemCount;
		}
	}
	
	
	public static dictionary getMasterItemDictionary(String itemName) throws InterruptedException
	{
		if (itemName == null || itemName.equals(""))
		{
			LOG( "create", " Bad itemName passed into the getMasterItemDictionary "+itemName );
			return null;
		}
		
		dictionary itemData = new dictionary();
		itemData = dataTableGetRow(MASTER_ITEM_TABLE, itemName);
		if (itemData == null)
		{
			LOG( "create", itemName + " Initalize from getMasterItemDictionary could not happen because row in datatable is bad");
			return null;
		}
		
		return itemData;
	}
	
	
	public static dictionary getMasterItemDictionary(obj_id item) throws InterruptedException
	{
		if (isIdValid(item))
		{
			String itemName = getStaticItemName(item);
			if (itemName == null || itemName.equals(""))
			{
				CustomerServiceLog( "static_item", item + " BAD ITEM:No Static Item Name this item needs to be fixed by adding a value to objvar staticItem");
				return null;
			}
			return getMasterItemDictionary (itemName);
		}
		else
		{
			LOG( "create","Bad item passed into the getMasterItemDictionary, bailing out");
			return null;
		}
	}
	
	
	public static boolean isStaticItem(obj_id item) throws InterruptedException
	{
		String itemName = getStaticItemName(item);
		if (itemName == null || itemName.equals(""))
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	
	
	public static boolean isStaticItem(String itemName) throws InterruptedException
	{
		int rowNum = dataTableSearchColumnForString(itemName, 0, MASTER_ITEM_TABLE);
		
		return rowNum != -1;
	}
	
	
	public static int getStaticObjectType(String itemName) throws InterruptedException
	{
		return dataTableGetInt(MASTER_ITEM_TABLE, itemName, "type");
	}
	
	
	public static int getStaticObjectValue(String itemName) throws InterruptedException
	{
		return dataTableGetInt(MASTER_ITEM_TABLE, itemName, "value");
	}
	
	
	public static dictionary getStaticObjectTypeDictionary(String itemName) throws InterruptedException
	{
		if (itemName == null || itemName.equals(""))
		{
			LOG( "create", " Itemname is Bad "+itemName );
			return null;
		}
		
		dictionary itemData = null;
		switch (getStaticObjectType(itemName))
		{
			case 1:
			itemData = getStaticItemWeaponDictionary(itemName);
			break;
			case 2:
			itemData = getStaticArmorDictionary(itemName);
			break;
			case 3:
			itemData = getStaticItemDictionary(itemName);
			break;
		}
		
		return itemData;
		
	}
	
	
	public static dictionary getStaticItemWeaponDictionary(String itemName) throws InterruptedException
	{
		if (itemName == null || itemName.equals(""))
		{
			LOG( "create", " Itemname is Bad "+itemName );
			return null;
		}
		
		dictionary itemData = new dictionary();
		itemData = dataTableGetRow(WEAPON_STAT_BALANCE_TABLE, itemName);
		if (itemData == null)
		{
			LOG( "create", itemName + " Initalize from getStaticItemWeaponDictionary could not happen because row in datatable is bad");
		}
		
		return itemData;
	}
	
	
	public static dictionary getStaticItemWeaponDictionary(obj_id item) throws InterruptedException
	{
		if (isIdValid(item))
		{
			String itemName = getStaticItemName(item);
			return getStaticItemWeaponDictionary (itemName);
		}
		else
		{
			LOG( "create","Bad item passed into the getWeaponDictionary, bailing out");
			return null;
		}
	}
	
	
	public static dictionary getStaticItemDictionary(String itemName) throws InterruptedException
	{
		if (itemName == null || itemName.equals(""))
		{
			LOG( "create", " Itemname is Bad "+itemName );
			return null;
		}
		
		dictionary itemData = new dictionary();
		itemData = dataTableGetRow(ITEM_STAT_BALANCE_TABLE, itemName);
		if (itemData == null)
		{
			LOG( "create", itemName + " Initalize from getStaticItemDictionary could not happen because row in datatable is bad");
		}
		
		return itemData;
	}
	
	
	public static dictionary getStaticItemDictionary(obj_id item) throws InterruptedException
	{
		if (isIdValid(item))
		{
			String itemName = getStaticItemName(item);
			return getStaticItemDictionary (itemName);
		}
		else
		{
			LOG( "create","Bad item passed into the getStaticItemDictionary, bailing out");
			return null;
		}
	}
	
	
	public static dictionary getStaticArmorDictionary(String itemName) throws InterruptedException
	{
		if (itemName == null || itemName.equals(""))
		{
			LOG( "create", " Itemname is Bad "+itemName );
			return null;
		}
		
		dictionary itemData = new dictionary();
		itemData = dataTableGetRow(ARMOR_STAT_BALANCE_TABLE, itemName);
		if (itemData == null)
		{
			LOG( "create", itemName + " Initalize from getStaticArmorDictionary could not happen because row in datatable is bad");
		}
		
		return itemData;
	}
	
	
	public static dictionary getStaticArmorDictionary(obj_id item) throws InterruptedException
	{
		if (isIdValid(item))
		{
			String itemName = getStaticItemName(item);
			return getStaticArmorDictionary (itemName);
		}
		else
		{
			LOG( "create","Bad armor passed into the getStaticArmorDictionary, bailing out");
			return null;
		}
	}
	
	
	public static void versionUpdate(obj_id self, dictionary itemData) throws InterruptedException
	{
		
		String itemName = itemData.getString("name");
		obj_id player = utils.getContainingPlayer(self);
		obj_id bioLink = getBioLink(self);
		
		String currentTemplate = getTemplateName(self);
		String staticName = getStaticItemName(self);
		if (staticName.equals("") || staticName == null)
		{
			return;
		}
		
		int itemRow = dataTableSearchColumnForString(staticName, "name", MASTER_ITEM_TABLE);
		
		String tableTemplate = dataTableGetString(MASTER_ITEM_TABLE, itemRow, "template_name");
		
		if (!currentTemplate.equals(tableTemplate))
		{
			CustomerServiceLog("versionUpdate: ", "Old Item's ("+ self + ") object template has changed. A new one must be created.");
			
			obj_id container = getContainedBy(self);
			if (isIdValid(container))
			{
				
				obj_id newItem = createNewItemFunction(staticName, getContainedBy(self));
				if (isIdValid(newItem))
				{
					
					if (isIdValid(player))
					{
						CustomerServiceLog("versionUpdate: ", "New Item ("+ newItem + ") has been created in "+ getPlayerName(player) + "("+player+")'s inventory.");
					}
					else
					{
						obj_id newItemsContainer = getContainedBy(newItem);
						CustomerServiceLog("versionUpdate: ", "New Item ("+ newItem + ") has been created in container ("+ newItemsContainer + ").");
					}
					CustomerServiceLog("versionUpdate: ", "Now deleting the old object ("+ self + ").");
					
					if (bioLink != null && bioLink != utils.OBJ_ID_BIO_LINK_PENDING)
					{
						setBioLink(newItem, bioLink);
					}
					
					destroyObject(self);
				}
				else
				{
					CustomerServiceLog("versionUpdate: ", "A new version was NOT created, so we will NOT destroy old item ("+self+")");
				}
			}
			else
			{
				CustomerServiceLog("versionUpdate: ", "Old Item ("+ self + ") returned an invalid container, we will NOT destroy it.");
			}
			
			return;
		}
		
		detachAllScripts(self);
		removeWornBuffs(self, player);
		checkForRemoveSetBonus(self, player);
		removeAllObjVars(self);
		
		if (bioLink != null && bioLink != utils.OBJ_ID_BIO_LINK_PENDING)
		{
			setBioLink(self, bioLink);
		}
		
		setStaticItemName(self, itemName );
		String objVarList = itemData.getString("creation_objvars");
		
		if (objVarList != null && !objVarList.equals(""))
		{
			utils.setObjVarsList(self, objVarList);
		}
		
		setStaticItemVersion(self, itemData.getInt("version"));
		
		int chargeList = itemData.getInt("charges");
		
		if (chargeList > 0 && chargeList <= 500)
		{
			setCount(self, chargeList);
		}
		
		initializeObject(self, itemData);
		
		return;
	}
	
	
	public static void validateWornEffects(obj_id player) throws InterruptedException
	{
		obj_id[] equipSlots = metrics.getWornItems(player);
		if (equipSlots != null && equipSlots.length > 0)
		{
			for (int i = 0; i < equipSlots.length; i++)
			{
				testAbortScript();
				String itemName = getStaticItemName(equipSlots[i]);
				if (itemName != null && !itemName.equals(""))
				{
					dictionary itemData = getStaticObjectTypeDictionary(itemName);
					if (itemData != null)
					{
						String buffName = itemData.getString("buff_name");
						if (buffName != null && !buffName.equals(""))
						{
							if (validateLevelRequiredForWornEffect(player,(equipSlots[i])))
							{
								applyWornBuffs(equipSlots[i], getContainedBy(equipSlots[i]));
								checkForAddSetBonus(equipSlots[i], getContainedBy(equipSlots[i]));
							}
						}
					}
				}
			}
		}
	}
	
	
	public static void applyWornBuffs(obj_id self, obj_id player) throws InterruptedException
	{
		
		if (hasScript(self, "item.armor.biolink_item_non_faction"))
		{
			
			obj_id bioLinked = getBioLink(self);
			if (bioLinked == null || bioLinked == utils.OBJ_ID_BIO_LINK_PENDING)
			{
				sendSystemMessage(player, SID_NOT_LINKED);
				return;
				
			}
			if (bioLinked != player)
			{
				sendSystemMessage(player, SID_NOT_LINKED_TO_HOLDER);
				return;
			}
		}
		
		String itemName = getStaticItemName(self);
		if (itemName == null || itemName.equals(""))
		{
			LOG( "create", "Worn Buff Itemname bad, name is "+itemName );
			return;
		}
		
		dictionary itemData = getStaticObjectTypeDictionary(itemName);
		
		if (itemData == null)
		{
			LOG( "create", itemName + "Worn Buff Apply could not happen because row in datatable is bad");
			return;
		}
		
		String buffName = itemData.getString("buff_name");
		String clientEffect = itemData.getString("client_effect");
		int requiredLevel = itemData.getInt("required_level_for_effect");
		int playerLevel = getLevel(player);
		
		if (buffName == null || buffName.equals(""))
		{
			LOG( "create","Worn Buff Name is bad");
			return;
		}
		
		if (static_item.validateLevelRequired(player,requiredLevel))
		{
			if (buff.canApplyBuff(player, buffName))
			{
				buff.applyBuff(player, buffName);
				
				playClientEffectObj(player, clientEffect, player, "");
			}
			else
			{
				utils.setScriptVar(self,"buffNotApplied", true);
				sendSystemMessage(player, ALREADY_HAVE_SIMILAR_BUFF);
				return;
			}
		}
		else
		{
			utils.setScriptVar(self,"buffNotApplied", true);
			sendSystemMessage(player, SID_ITEM_LEVEL_TOO_LOW);
			return;
		}
		
		return;
	}
	
	
	public static void removeWornBuffs(obj_id self, obj_id player) throws InterruptedException
	{
		
		String itemName = getStaticItemName(self);
		if (itemName == null || itemName.equals(""))
		{
			LOG( "create", "Worn Buff Itemname bad, name is "+itemName );
			return;
		}
		
		dictionary itemData = getStaticObjectTypeDictionary(itemName);
		
		if (itemData == null)
		{
			LOG( "create", itemName + "Worn Buff Remove could not happen because row in datatable is bad");
			return;
		}
		
		String buffName = itemData.getString("buff_name");
		
		if (buffName == null || buffName.equals(""))
		{
			LOG( "create","Remove Buff Name is bad");
			return;
		}
		
		if (!utils.hasScriptVar(self, "buffNotApplied"))
		{
			buff.removeBuff(player, buffName);
		}
		else
		{
			utils.removeScriptVar(self, "buffNotApplied");
		}
		
		return;
	}
	
	
	public static void checkForAddSetBonus(obj_id item, obj_id player) throws InterruptedException
	{
		if (!isSetBonusItem(item))
		{
			return;
		}
		
		int setBonusIndex = getSetBonusIndex(item);
		int numberOfSetItems = getNumSetItems(player, item, setBonusIndex);
		String setBonusBuffName = getSetBonusBuffName(item, setBonusIndex, numberOfSetItems);
		
		if (!setBonusBuffName.equals(null) && !setBonusBuffName.equals("none"))
		{
			if (!buff.hasBuff(player, setBonusBuffName))
			{
				buff.applyBuff(player, setBonusBuffName);
				sendSystemMessage(player, new string_id("set_bonus", setBonusBuffName + "_sys"));
			}
		}
		
		return;
	}
	
	
	public static void checkForRemoveSetBonus(obj_id item, obj_id player) throws InterruptedException
	{
		if (!isSetBonusItem(item))
		{
			return;
		}
		
		int setBonusIndex = getSetBonusIndex(item);
		int numberOfSetItems = getNumSetItems(player, item, setBonusIndex);
		String setBonusBuffName = getSetBonusBuffName(item, setBonusIndex, numberOfSetItems + 1);
		
		if (!setBonusBuffName.equals(null) && !setBonusBuffName.equals("none"))
		{
			if (buff.hasBuff(player, setBonusBuffName))
			{
				buff.removeBuff(player, setBonusBuffName);
			}
			
			String lowerSetBonusBuffName = getSetBonusBuffName(item, setBonusIndex, numberOfSetItems);
			
			if (!lowerSetBonusBuffName.equals(null) && !setBonusBuffName.equals("none"))
			{
				if (!buff.hasBuff(player, lowerSetBonusBuffName))
				{
					buff.applyBuff(player, lowerSetBonusBuffName);
				}
			}
		}
		
		return;
	}
	
	
	public static boolean isSetBonusItem(obj_id item) throws InterruptedException
	{
		return hasScript(item, "item.set_bonus_item");
	}
	
	
	public static int getSetBonusIndex(obj_id item) throws InterruptedException
	{
		int setBonusIndex = -1;
		
		if (!isIdValid(item))
		{
			return setBonusIndex;
		}
		
		if (hasObjVar(item, "item.set.set_id"))
		{
			setBonusIndex = getIntObjVar(item, "item.set.set_id");
		}
		else
		{
			sendSystemMessageTestingOnly(getContainedBy(item), "getSetBonusIndex called on object missing set bonus index objvar.");
		}
		
		return setBonusIndex;
	}
	
	
	public static int getNumSetItems(obj_id player, obj_id item, int setBonusIndex) throws InterruptedException
	{
		int numSetItems = 0;
		
		if (setBonusIndex == -1)
		{
			return numSetItems;
		}
		
		obj_id[] equippedItems = metrics.getWornItems(player);
		
		if (equippedItems != null && equippedItems.length > 0)
		{
			for (int i = 0; i < equippedItems.length; i++)
			{
				testAbortScript();
				if (isSetBonusItem(equippedItems[i]))
				{
					if (setBonusIndex == getSetBonusIndex(equippedItems[i]))
					{
						numSetItems++;
					}
				}
			}
		}
		
		return numSetItems;
	}
	
	
	public static String getSetBonusBuffName(obj_id item, int setBonusIndex, int numberOfSetItems) throws InterruptedException
	{
		String setBonusBuffName = "none";
		
		int numTableRows = dataTableGetNumRows(SET_BONUS_TABLE);
		
		for (int i = 0; i < numTableRows; i++)
		{
			testAbortScript();
			int thisRowSetBonusIndex = dataTableGetInt(SET_BONUS_TABLE, i, "SETID");
			int thisRowNumberOfSetItems = dataTableGetInt(SET_BONUS_TABLE, i, "NUM_ITEMS");
			
			if (setBonusIndex == thisRowSetBonusIndex && numberOfSetItems == thisRowNumberOfSetItems)
			{
				setBonusBuffName = dataTableGetString(SET_BONUS_TABLE, i, "EFFECT");
				return setBonusBuffName;
			}
			
		}
		
		return setBonusBuffName;
	}
	
	
	public static String getItemType(String itemName) throws InterruptedException
	{
		dictionary itemData = new dictionary();
		itemData = dataTableGetRow(MASTER_ITEM_TABLE, itemName);
		if (itemData == null)
		{
			return "";
		}
		
		switch (itemData.getInt("type"))
		{
			case 1:
			return "weapon";
			case 2:
			return "armor";
			case 3:
			return "item";
		}
		
		return "";
	}
	
	
	public static dictionary getMergedItemDictionary(String itemName) throws InterruptedException
	{
		dictionary masterItemData = getMasterItemDictionary(itemName);
		String itemType = static_item.getItemType(itemName);
		dictionary specificItemData = null;
		if (itemType.equals("weapon"))
		{
			specificItemData = getStaticItemWeaponDictionary (itemName);
		}
		else if (itemType.equals("armor"))
		{
			specificItemData = getStaticArmorDictionary (itemName);
		}
		else if (itemType.equals("item"))
		{
			specificItemData = getStaticItemDictionary (itemName);
		}
		
		if (specificItemData != null)
		{
			java.util.Enumeration e = specificItemData.keys();
			while (e.hasMoreElements())
			{
				testAbortScript();
				String s = e.nextElement().toString();
				masterItemData.put(s, specificItemData.get(s));
			}
		}
		return masterItemData;
	}
	
	
	public static void getAttributes(obj_id player, String staticItemName, String[] names, String[] attribs) throws InterruptedException
	{
		String itemType = static_item.getItemType(staticItemName);
		if (itemType.equals("weapon"))
		{
			getStaticWeaponObjectAttributes(player, staticItemName, names, attribs);
		}
		else if (itemType.equals("armor"))
		{
			getStaticArmorObjectAttributes(player, staticItemName, names, attribs);
		}
		else if (itemType.equals("item"))
		{
			getStaticItemObjectAttributes(player, staticItemName, names, attribs);
		}
	}
	
	
	public static void getStaticWeaponObjectAttributes(obj_id player, String staticItemName, String[] names, String[] attribs) throws InterruptedException
	{
		String at = "@obj_attr_n:";
		int free = 0;
		
		dictionary itemData = static_item.getMergedItemDictionary(staticItemName);
		
		int levelRequired = itemData.getInt("required_level");
		names[free] = "healing_combat_level_required";
		attribs[free++] = "" + levelRequired;
		
		names[free] = "tooltip.healing_combat_level_required";
		attribs[free++] = "" + levelRequired;
		
		String skillRequired = itemData.getString("required_skill");
		String skillRequiredAttribute;
		if (skillRequired != null && !skillRequired.equals(""))
		{
			skillRequiredAttribute = "@skl_n:"+ skillRequired;
		}
		else
		{
			skillRequiredAttribute = "None";
		}
		
		names[free] = "skillmodmin";
		attribs[free++] = skillRequiredAttribute;
		
		names[free] = "tooltip.skillmodmin";
		attribs[free++] = skillRequiredAttribute;
		
		int damageType = itemData.getInt("damage_type");
		String wpn_damage_type = "cat_wpn_damage.wpn_damage_type";
		names[free] = wpn_damage_type;
		String damageTypeStr = "kinetic";
		if (damageType == 1)
		{
			damageTypeStr = "kinetic";
		}
		else if (damageType == 2)
		{
			damageTypeStr = "energy";
		}
		else
		{
			damageTypeStr = "unknown";
		}
		attribs[free++] = at + "armor_eff_" + damageTypeStr;
		
		names[free] = "tooltip.wpn_damage_type";
		attribs[free++] = at + "armor_eff_" + damageTypeStr;
		
		int minDamage = itemData.getInt("min_damage");
		int maxDamage = itemData.getInt("max_damage");
		names[free] = "cat_wpn_damage.damage";
		String weaponDamage = Integer.toString(minDamage) + " - "+ Integer.toString(maxDamage);
		attribs[free++] = weaponDamage;
		
		names[free] = "tooltip.damage";
		attribs[free++] = weaponDamage;
		
		names[free] = "cat_wpn_other.wpn_range";
		int minRange = itemData.getInt("min_range_distance");
		int maxRange = itemData.getInt("max_range_distance");
		String weaponRange = noDecimalFormat.format(minRange) + "-"+ noDecimalFormat.format(maxRange) + "m";
		attribs[free++] = weaponRange;
		
		names[free] = "tooltip.wpn_range";
		attribs[free++] = weaponRange;
		
		int tier = itemData.getInt("tier");
		names[free] = "tier";
		attribs[free++] = "" + tier;
		
		names[free] = "tooltip.tier";
		attribs[free++] = "" + tier;
		
		String skillMods = itemData.getString("skill_mods");
		dictionary dict = parseSkillModifiers(player, skillMods);
		String statstf = "@stat_n:";
		
		if (dict != null)
		{
			java.util.Enumeration keys = dict.keys();
			
			while (keys.hasMoreElements())
			{
				testAbortScript();
				String skillModName = (String)keys.nextElement();
				int skillModValue = dict.getInt(skillModName);
				
				names[free] = statstf + skillModName;
				attribs[free++] = Integer.toString(skillModValue);
				
				names[free] = "tooltip." + statstf + skillModName;
				attribs[free++] = Integer.toString(skillModValue);
			}
		}
	}
	
	
	public static void getStaticArmorObjectAttributes(obj_id player, String staticItemName, String[] names, String[] attribs) throws InterruptedException
	{
		String at = "@obj_attr_n:";
		int free = 0;
		
		dictionary itemData = static_item.getMergedItemDictionary(staticItemName);
		
		int levelRequired = itemData.getInt("required_level");
		names[free] = "healing_combat_level_required";
		attribs[free++] = "" + levelRequired;
		
		names[free] = "tooltip.healing_combat_level_required";
		attribs[free++] = "" + levelRequired;
		
		String skillRequired = itemData.getString("required_skill");
		String skillRequiredAttribute;
		if (skillRequired != null && !skillRequired.equals(""))
		{
			skillRequiredAttribute = "@skl_n:"+ skillRequired;
		}
		else
		{
			skillRequiredAttribute = "None";
		}
		
		names[free] = "skillmodmin";
		attribs[free++] = skillRequiredAttribute;
		
		names[free] = "tooltip.skillmodmin";
		attribs[free++] = skillRequiredAttribute;
		
		int damageType = itemData.getInt("damage_type");
		String wpn_damage_type = "cat_wpn_damage.wpn_damage_type";
		names[free] = wpn_damage_type;
		String damageTypeStr = "kinetic";
		if (damageType == 1)
		{
			damageTypeStr = "kinetic";
		}
		else if (damageType == 2)
		{
			damageTypeStr = "energy";
		}
		else
		{
			damageTypeStr = "unknown";
		}
		attribs[free++] = at + "armor_eff_" + damageTypeStr;
		
		int tier = itemData.getInt("tier");
		names[free] = "tier";
		attribs[free++] = "" + tier;
		
		names[free] = "tooltip.tier";
		attribs[free++] = "" + tier;
		
		String skillMods = itemData.getString("skill_mods");
		dictionary dict = parseSkillModifiers(player, skillMods);
		String statstf = "@stat_n:";
		
		if (dict != null)
		{
			java.util.Enumeration keys = dict.keys();
			
			while (keys.hasMoreElements())
			{
				testAbortScript();
				String skillModName = (String)keys.nextElement();
				int skillModValue = dict.getInt(skillModName);
				
				names[free] = statstf + skillModName;
				attribs[free++] = Integer.toString(skillModValue);
				
				names[free] = "tooltip." + statstf + skillModName;
				attribs[free++] = Integer.toString(skillModValue);
			}
		}
		
	}
	
	
	public static void getStaticItemObjectAttributes(obj_id player, String item_name, String[] names, String[] attribs) throws InterruptedException
	{
		String at = "@obj_attr_n:";
		int free = 0;
		
		dictionary itemData = static_item.getMergedItemDictionary(item_name);
		
		int requiredLevelToEquip = itemData.getInt("required_level");
		if (requiredLevelToEquip != 0)
		{
			names[free] = utils.packStringId(new string_id ("proc/proc", "required_combat_level"));
			attribs[free++] = "" + requiredLevelToEquip;
			names[free] = utils.packStringId(new string_id ("proc/proc", "tooltip.required_combat_level"));
			attribs[free++] = "" + requiredLevelToEquip;
		}
		
		String requiredSkillToEquip = itemData.getString("required_skill");
		if (requiredSkillToEquip != null && !requiredSkillToEquip.equals(""))
		{
			names[free] = utils.packStringId(new string_id ("proc/proc", "required_skill"));
			attribs[free++] = utils.packStringId(new string_id ("ui_roadmap", requiredSkillToEquip));
			names[free] = utils.packStringId(new string_id ("proc/proc", "tooltip.required_skill"));
			attribs[free++] = utils.packStringId(new string_id ("ui_roadmap", requiredSkillToEquip));
		}
		
		int reuseTime = itemData.getInt("reuse_time");
		if (reuseTime != 0)
		{
			names[free] = utils.packStringId(new string_id ("proc/proc", "reuse_time"));
			attribs[free++] = "" + reuseTime + " / sec";
			names[free] = utils.packStringId(new string_id ("proc/proc", "tooltip.reuse_time"));
			attribs[free++] = "" + reuseTime + " / sec";
		}
		
		if (reuseTime > 30)
		{
			names[free] = utils.packStringId(new string_id ("spam", "reuse_time_counted"));
			
			String coolDownGroup = itemData.getString("cool_down_group");
			String varName = "clickItem."+ coolDownGroup;
			int current_time = getGameTime();
			int timestamp = -1;
			
			if (hasObjVar(player,varName))
			{
				timestamp = getIntObjVar(player,varName);
			}
			
			int time_remaining = timestamp - current_time;
			
			if (timestamp == -1 || time_remaining <= 0)
			{
				attribs[free] = "0";
			}
			else
			{
				attribs[free] = utils.assembleTimeRemainToUse(time_remaining);
			}
			free++;
		}
		
		int requiredLevelForEffect = itemData.getInt("required_level_for_effect");
		if (requiredLevelForEffect != 0)
		{
			names[free] = utils.packStringId(new string_id ("proc/proc", "effect_level"));
			attribs[free++] = "" + requiredLevelForEffect;
			names[free] = utils.packStringId(new string_id ("proc/proc", "tooltip.effect_level"));
			attribs[free++] = "" + requiredLevelForEffect;
		}
		
		String buffName = itemData.getString("buff_name");
		if (buffName != null && !buffName.equals(""))
		{
			names[free] = utils.packStringId(new string_id ("proc/proc", "proc_name"));
			attribs[free++] = utils.packStringId(new string_id ("ui_buff", buffName));
			names[free] = utils.packStringId(new string_id ("proc/proc", "tooltip.proc_name"));
			attribs[free++] = utils.packStringId(new string_id ("ui_buff", buffName));
		}
		
		String skillMods = itemData.getString("skill_mods");
		dictionary dict = parseSkillModifiers(player, skillMods);
		String statstf = "@stat_n:";
		
		if (dict != null)
		{
			java.util.Enumeration keys = dict.keys();
			
			while (keys.hasMoreElements())
			{
				testAbortScript();
				String skillModName = (String)keys.nextElement();
				int skillModValue = dict.getInt(skillModName);
				
				names[free] = statstf + skillModName;
				attribs[free++] = Integer.toString(skillModValue);
				
				names[free] = "tooltip." + statstf + skillModName;
				attribs[free++] = Integer.toString(skillModValue);
			}
		}
		
		int tier = itemData.getInt("tier");
		names[free] = "tier";
		attribs[free++] = "" + tier;
		
		names[free] = "tooltip.tier";
		attribs[free++] = "" + tier;
	}
	
	
	public static dictionary parseSkillModifiers(obj_id player, String skillMods) throws InterruptedException
	{
		dictionary dict = new dictionary();
		if (skillMods != null && !skillMods.equals(""))
		{
			String[] stringArray = split(skillMods, ',');
			for (int i = 0; i < stringArray.length; i++)
			{
				testAbortScript();
				String[] modsArray = split(stringArray[i], '=');
				for (int x = 0; x < modsArray.length; x+=2)
				{
					testAbortScript();
					dict.put(modsArray[0], utils.stringToInt(modsArray[1]));
				}
			}
		}
		return dict;
	}
	
	
	public static obj_id makeDynamicObject(String strName, obj_id objContainer, int intLevel) throws InterruptedException
	{
		int whatRow = -1;
		if (intLevel > 90)
		{
			intLevel = 90;
		}
		if (intLevel < 1)
		{
			intLevel = 1;
		}
		String strBaseTable = "datatables/item/dynamic_item/types/";
		boolean boolIsWeapon = false;
		if (strName.startsWith("dynamic_weapon"))
		{
			strBaseTable = strBaseTable+"weapons.iff";
			boolIsWeapon = true;
		}
		
		else if (strName.startsWith("dynamic_armor") && intLevel > 21)
		{
			strBaseTable = strBaseTable + "armor.iff";
		}
		else if (strName.startsWith("dynamic_clothing"))
		{
			strBaseTable = strBaseTable + "clothing.iff";
		}
		else
		{
			return null;
		}
		
		dictionary dctItemInfo = dataTableGetRow(strBaseTable, strName);
		if (dctItemInfo == null)
		{
			return null;
		}
		String strAppearance = dctItemInfo.getString("strBaseAppearance");
		if (strAppearance.endsWith(".iff"))
		{
			
		}
		else
		{
			String[] strTemplates = dataTableGetStringColumnNoDefaults("datatables/item/dynamic_item/appearances/"+strAppearance+".iff", "strTemplate");
			if ((strTemplates == null)||(strTemplates.length < 1))
			{
				return null;
			}
			
			whatRow = rand(0, strTemplates.length-1);
			strAppearance = strTemplates[whatRow];
		}
		obj_id objObject = createObject(strAppearance, objContainer, "");
		
		if (utils.hasScriptVar(objContainer, "theft_in_progress"))
		{
			loot.notifyThiefOfItemStolen(objContainer, objObject);
		}
		
		if (!isIdValid(objObject))
		{
			return null;
		}
		if (boolIsWeapon)
		{
			dictionary dctWeaponStats = dataTableGetRow("datatables/item/dynamic_item/root_balance_data/weapon_data.iff",intLevel);
			return setupDynamicWeapon(objObject, strName, intLevel, dctWeaponStats, dctItemInfo, true);
		}
		
		if (strName.startsWith("dynamic_armor"))
		{
			if (strName.startsWith("dynamic_armor_jedi"))
			{
				setObjVar(objObject, "dynamic_item.required_skill", "force_sensitive");
			}
			
			if (whatRow > -1)
			{
				String strAppearanceForReals = dctItemInfo.getString("strBaseAppearance");
				int indexColor = dataTableGetInt("datatables/item/dynamic_item/appearances/"+strAppearanceForReals+".iff", whatRow, "indexColor");
				int maxHue = dataTableGetInt("datatables/item/dynamic_item/appearances/"+strAppearanceForReals+".iff", whatRow, "maxHue");
				int datatableArmorType = dataTableGetInt("datatables/item/dynamic_item/appearances/"+strAppearanceForReals+".iff", whatRow, "armorType");
				
				if (indexColor >= 0 && maxHue >= 0)
				{
					int thisHue = rand(0, maxHue);
					hue.setColor(objObject, "/private/index_color_"+ indexColor, thisHue);
				}
				
				if (datatableArmorType > -1)
				{
					setObjVar(objObject, "armor.armorCategory", datatableArmorType);
				}
				
			}
			
			return setupDynamicArmor(objObject, intLevel, dctItemInfo, true);
		}
		
		if (strName.startsWith("dynamic_clothing"))
		{
			
			if (whatRow > -1)
			{
				String strAppearanceForReals = dctItemInfo.getString("strBaseAppearance");
				int indexColor = dataTableGetInt("datatables/item/dynamic_item/appearances/"+strAppearanceForReals+".iff", whatRow, "indexColor");
				int maxHue = dataTableGetInt("datatables/item/dynamic_item/appearances/"+strAppearanceForReals+".iff", whatRow, "maxHue");
				
				if (indexColor >= 0 && maxHue >= 0)
				{
					int thisHue = rand(0, maxHue);
					hue.setColor(objObject, "/private/index_color_"+ indexColor, thisHue);
				}
			}
			setObjVar(objObject, "dynamic_item.intLevelRequired", intLevel);
			generateItemStatBonuses(objObject, intLevel, dctItemInfo);
			String strSuffix = getArmorNameSuffix(objObject);
			dctItemInfo.put("strSuffix", strSuffix);
			setupDynamicItemName(objObject, dctItemInfo);
			attachScript(objObject, "item.armor.dynamic_armor");
			
			int basePrice = (int)(Math.pow( (intLevel*12.0f), 1.3f) );
			
			if (basePrice > 10200)
			{
				basePrice = 10200;
			}
			
			setObjVar(objObject, "junkDealer.intPrice", basePrice);
			int minPriceValue = 0;
			int maxPriceValue = intLevel * 10;
			setupJunkDealerPrice(objObject, minPriceValue, maxPriceValue, 0);
		}
		
		return objObject;
	}
	
	
	public static obj_id setupDynamicArmor(obj_id objArmor, int intLevel) throws InterruptedException
	{
		
		return setupDynamicArmor(objArmor, intLevel, null, false);
	}
	
	
	public static obj_id setupDynamicArmor(obj_id objArmor, int intLevel, dictionary dctItemInfo, boolean boolSetupData) throws InterruptedException
	{
		String armorBalanceTable = "datatables/item/dynamic_item/root_balance_data/armor_data.iff";
		String modifiersTable = "datatables/item/dynamic_item/modifiers/armor_mods.iff";
		String armorTypeTable = "datatables/item/dynamic_item/types/armor.iff";
		
		int minLevel = dataTableGetInt(armorBalanceTable, 0, "minLevel");
		int protectIncreasePerLevel = dataTableGetInt(armorBalanceTable, 0, "protectIncreasePerLevel");
		int minRawProtection = dataTableGetInt(armorBalanceTable, 0, "minRawProtection");
		int maxRawProtection = dataTableGetInt(armorBalanceTable, 0, "maxRawProtection");
		float balanceMod = dataTableGetFloat(armorBalanceTable, 0, "balanceMod");
		float minBaseMod = dataTableGetFloat(armorBalanceTable, 0, "minBaseMod");
		float maxBaseMod = dataTableGetFloat(armorBalanceTable, 0, "maxBaseMod");
		float minPrimaryMod = dataTableGetFloat(armorBalanceTable, 0, "minPrimaryMod");
		float maxPrimaryMod = dataTableGetFloat(armorBalanceTable, 0, "maxPrimaryMod");
		float minSpecialMod = dataTableGetFloat(armorBalanceTable, 0, "minSpecialMod");
		float maxSpecialMod = dataTableGetFloat(armorBalanceTable, 0, "maxSpecialMod");
		float baseSpecialMod = dataTableGetFloat(armorBalanceTable, 0, "baseSpecialMod");
		
		float baseMod = minBaseMod;
		float primaryMod = minPrimaryMod;
		float specialMod = minSpecialMod;
		int armorTypeModRow = 0;
		
		if (boolSetupData)
		{
			
			baseMod = rand(minBaseMod, maxBaseMod);
			primaryMod = rand(minPrimaryMod, maxPrimaryMod);
			specialMod = rand(minSpecialMod, maxSpecialMod);
			armorTypeModRow = rand(0, dataTableGetNumRows(modifiersTable) - 1 );
			
			setObjVar(objArmor, "dynamic_item.intLevelRequired", intLevel);
			setObjVar(objArmor, "dynamic_item.baseMod", baseMod);
			setObjVar(objArmor, "dynamic_item.primaryMod", primaryMod);
			setObjVar(objArmor, "dynamic_item.specialMod", specialMod);
			setObjVar(objArmor, "dynamic_item.armorTypeModRow", armorTypeModRow);
		}
		
		else
		{
			baseMod = getFloatObjVar(objArmor, "dynamic_item.baseMod");
			primaryMod = getFloatObjVar(objArmor, "dynamic_item.primaryMod");
			specialMod = getFloatObjVar(objArmor, "dynamic_item.specialMod");
			armorTypeModRow = getIntObjVar(objArmor, "dynamic_item.armorTypeModRow");
			intLevel = getIntObjVar(objArmor, "dynamic_item.intLevelRequired");
			
			if (baseMod < minBaseMod)
			{
				baseMod = minBaseMod;
				setObjVar(objArmor, "dynamic_item.baseMod", baseMod);
			}
			
			if (baseMod > maxBaseMod)
			{
				baseMod = maxBaseMod;
				setObjVar(objArmor, "dynamic_item.baseMod", baseMod);
			}
			
			if (primaryMod < minPrimaryMod)
			{
				primaryMod = minPrimaryMod;
				setObjVar(objArmor, "dynamic_item.primaryMod", primaryMod);
			}
			if (primaryMod > maxPrimaryMod)
			{
				primaryMod = maxPrimaryMod;
				setObjVar(objArmor, "dynamic_item.primaryMod", primaryMod);
			}
			
			if (specialMod < minSpecialMod)
			{
				specialMod = minSpecialMod;
				setObjVar(objArmor, "dynamic_item.specialMod", specialMod);
			}
			
			if (specialMod > maxSpecialMod)
			{
				specialMod = maxSpecialMod;
				setObjVar(objArmor, "dynamic_item.specialMod", specialMod);
			}
			
			if (armorTypeModRow > dataTableGetNumRows(modifiersTable) - 1)
			{
				armorTypeModRow = dataTableGetNumRows(modifiersTable) - 1;
				setObjVar(objArmor, "dynamic_item.armorTypeModRow", armorTypeModRow);
			}
			
			if (armorTypeModRow < 0)
			{
				armorTypeModRow = 0;
				setObjVar(objArmor, "dynamic_item.armorTypeModRow", armorTypeModRow);
			}
			
			if (intLevel > 90)
			{
				intLevel = 90;
				setObjVar(objArmor, "dynamic_item.intLevelRequired", intLevel);
			}
			
			if (intLevel < 1)
			{
				intLevel = 1;
				setObjVar(objArmor, "dynamic_item.intLevelRequired", intLevel);
			}
		}
		
		int rawProtection = ( (intLevel - minLevel) * protectIncreasePerLevel ) + minRawProtection;
		rawProtection *= balanceMod;
		
		if (rawProtection > maxRawProtection)
		{
			rawProtection = maxRawProtection;
		}
		
		dictionary armorTypeMods = dataTableGetRow(modifiersTable, armorTypeModRow);
		float baseProtection = baseMod * rawProtection;
		
		float kinMod = armorTypeMods.getFloat("kinMod");
		float engMod = armorTypeMods.getFloat("engMod");
		
		float kinProtFloat = (1 + (kinMod * primaryMod) ) * baseProtection;
		float engProtFloat = (1 + (engMod * primaryMod) ) * baseProtection;
		
		float heatMod = armorTypeMods.getFloat("heatMod");
		float coldMod = armorTypeMods.getFloat("coldMod");
		float acidMod = armorTypeMods.getFloat("acidMod");
		float electMod = armorTypeMods.getFloat("electMod");
		
		float heatProtFloat = (1 + (heatMod * specialMod) ) * baseProtection * baseSpecialMod;
		float coldProtFloat = (1 + (coldMod * specialMod) ) * baseProtection * baseSpecialMod;
		float acidProtFloat = (1 + (acidMod * specialMod) ) * baseProtection * baseSpecialMod;
		float electProtFloat = (1 + (electMod * specialMod) ) * baseProtection * baseSpecialMod;
		
		setObjVar(objArmor, "armor.fake_armor.kinetic", (int)kinProtFloat);
		setObjVar(objArmor, "armor.fake_armor.energy", (int)engProtFloat);
		setObjVar(objArmor, "armor.fake_armor.heat", (int)heatProtFloat);
		setObjVar(objArmor, "armor.fake_armor.cold", (int)coldProtFloat);
		setObjVar(objArmor, "armor.fake_armor.acid", (int)acidProtFloat);
		setObjVar(objArmor, "armor.fake_armor.electricity", (int)electProtFloat);
		
		if (boolSetupData)
		{
			
			int basePrice = (int)(Math.pow( (intLevel*12.0f), 1.3f) );
			
			if (basePrice > 10200)
			{
				basePrice = 10200;
			}
			
			setObjVar(objArmor, "junkDealer.intPrice", basePrice);
			
			int minPriceValue = 0;
			int maxPriceValue = intLevel * 10;
			
			attachScript(objArmor, "item.armor.dynamic_armor");
			setupJunkDealerPrice(objArmor, minPriceValue, maxPriceValue, 0);
			generateItemStatBonuses(objArmor, intLevel, dctItemInfo);
			
			String strSuffix = getArmorNameSuffix(objArmor);
			armorTypeMods.put("strSuffix", strSuffix);
			setupDynamicItemName(objArmor, armorTypeMods);
			
			if (!hasScript(objArmor, "item.armor.new_armor"))
			{
				attachScript(objArmor, "item.armor.new_armor");
			}
		}
		
		return objArmor;
	}
	
	
	public static float getDynamicWeaponRange(obj_id weapon) throws InterruptedException
	{
		int level = getIntObjVar(weapon, "dynamic_item.intLevelRequired");
		dictionary dctWeaponStats = dataTableGetRow("datatables/item/dynamic_item/root_balance_data/weapon_data.iff", level);
		int intWeaponType = getWeaponType(weapon);
		String strWeaponString = combat.getWeaponStringType(intWeaponType);
		return dctWeaponStats.getFloat(strWeaponString + "_max_range");
	}
	
	
	public static obj_id setupDynamicWeapon(obj_id objWeapon, String itemName, int intLevel, dictionary dctWeaponStats, dictionary dctItemInfo, boolean boolSetupData) throws InterruptedException
	{
		obj_id self = getSelf();
		
		dictionary dctItemNames = new dictionary();
		
		if (boolSetupData)
		{
			String strBaseName = dctItemInfo.getString("strRootName");
			if (!strBaseName.equals(""))
			{
				dctItemNames.put("strBaseName", strBaseName);
			}
		}
		
		float fltMinDamageVal = 0;
		float fltMaxDamageVal = 0;
		
		if (boolSetupData)
		{
			fltMinDamageVal = rand(0f, 1f);
			fltMaxDamageVal = rand(0f, 1f);
			setObjVar(objWeapon, "dynamic_item.minDamageVal", fltMinDamageVal);
			setObjVar(objWeapon, "dynamic_item.maxDamageVal", fltMaxDamageVal);
		}
		else
		{
			fltMinDamageVal = getFloatObjVar(objWeapon, "dynamic_item.minDamageVal");
			fltMaxDamageVal = getFloatObjVar(objWeapon, "dynamic_item.maxDamageVal");
			
		}
		
		int intWeaponType = getWeaponType(objWeapon);
		String strWeaponString = combat.getWeaponStringType(intWeaponType);
		
		int intMinDamageMin = dctWeaponStats.getInt(strWeaponString + "_min_damage_min");
		int intMinDamageMax = dctWeaponStats.getInt(strWeaponString + "_min_damage_max");
		
		int intMaxDamageMin = dctWeaponStats.getInt(strWeaponString + "_max_damage_min");
		int intMaxDamageMax = dctWeaponStats.getInt(strWeaponString + "_max_damage_max");
		
		float fltMin = (float) (intMinDamageMax - intMinDamageMin);
		float fltMax = (float) (intMaxDamageMax - intMaxDamageMin);
		
		fltMin = fltMin * fltMinDamageVal;
		fltMax = fltMax * fltMaxDamageVal;
		
		int intMinDamage = intMinDamageMin + (int)fltMin;
		int intMaxDamage = intMaxDamageMin + (int)fltMax;
		
		float fltAttackSpeed = dctWeaponStats.getFloat(strWeaponString + "_attack_speed");
		float fltMaxRange = dctWeaponStats.getFloat(strWeaponString + "_max_range");
		
		int intDamageType = DAMAGE_NONE;
		
		if (boolSetupData)
		{
			intDamageType = dctItemInfo.getInt("baseDamageType");
			
			if (intDamageType == -1)
			{
				int intRoll = rand(1,2);
				if (intRoll == 1)
				{
					intDamageType = DAMAGE_KINETIC;
				}
				else
				{
					intDamageType = DAMAGE_ENERGY;
				}
			}
		}
		else
		{
			intDamageType = getWeaponDamageType(objWeapon);
		}
		
		setWeaponMinDamage(objWeapon, intMinDamage);
		setWeaponMaxDamage(objWeapon, intMaxDamage);
		setWeaponAttackSpeed(objWeapon, fltAttackSpeed);
		setWeaponWoundChance(objWeapon, 0);
		setMaxHitpoints(objWeapon,(1000));
		setHitpoints(objWeapon,(1000));
		setWeaponAccuracy(objWeapon, 0);
		setWeaponDamageType(objWeapon, intDamageType);
		setWeaponRangeInfo(objWeapon, 0, fltMaxRange);
		setWeaponAttackCost(objWeapon, 0);
		
		setupJunkDealerPrice(objWeapon, dctWeaponStats.getInt("intMinValue"), dctWeaponStats.getInt("intMaxValue"), (fltMinDamageVal + fltMaxDamageVal) /2);
		
		setObjVar(objWeapon, "dynamic_item.intLevelRequired", intLevel);
		setObjVar(objWeapon, "weapon.original_max_range", fltMaxRange);
		
		if (boolSetupData)
		{
			setupDynamicItemName(objWeapon, dctItemNames);
		}
		
		weapons.setWeaponData(objWeapon);
		setConversionId(objWeapon, weapons.CONVERSION_VERSION);
		
		return objWeapon;
	}
	
	
	public static void generateItemStatBonuses(obj_id item, int level, dictionary dctItemInfo) throws InterruptedException
	{
		
		float[] numStatChances =
		{
			101.0f, 101.0f, 101.0f, 101.0f, 101.0f, 101.0f, 101.0f
		};
		int[] statBonuses =
		{
			0, 0, 0, 0, 0, 0, 0
		};
		String[] statBonusNames =
		{
			"precision_modified", "strength_modified", "stamina_modified",
			"constitution_modified", "agility_modified", "luck_modified", "camouflage"
		};
		
		for (int i = 0; i < numStatChances.length; i ++)
		{
			testAbortScript();
			float thisChance = dctItemInfo.getFloat( i + "statChance");
			
			if (thisChance < 0)
			{
				thisChance = 0;
			}
			
			numStatChances[i] = thisChance;
		}
		
		float numberOfStatsRoll = rand(1.0f, 100.0f);
		int numberOfStats = 0;
		
		for (int i = 6; i > 0; i --)
		{
			testAbortScript();
			if (numberOfStatsRoll > numStatChances[i])
			{
				numberOfStats = i;
				break;
			}
		}
		
		if (numberOfStats > 0)
		{
			for (int i = 0; i < numberOfStats; i ++)
			{
				testAbortScript();
				int statBonus = generateStatMod(level);
				
				for (int j = 0; j < 12; j ++)
				{
					testAbortScript();
					int isCamo = rand(1, 100);
					int thisStat = rand(0, 5);
					if (isCamo <= 5)
					{
						thisStat = 6;
					}
					
					if (statBonuses[thisStat] == 0)
					{
						
						statBonuses[thisStat] = statBonus;
						break;
					}
				}
			}
		}
		
		for (int i = 0; i < statBonuses.length; i++)
		{
			testAbortScript();
			if (statBonuses[i] > 0)
			{
				setObjVar(item, "skillmod.bonus."+ statBonusNames[i], statBonuses[i]);
			}
		}
		
		return;
	}
	
	
	public static int generateStatMod(int level) throws InterruptedException
	{
		final float VARIANCE = 0.125f;
		final float BASE_MOD = 0.2f;
		final float MAX_MOD = 0.312f;
		final float FLOOR_CHANCE = 60.0f;
		
		float varianceIncrements = level*MAX_MOD-level*BASE_MOD;
		float exponentIncrement = 18/varianceIncrements;
		float baseMod = level* BASE_MOD;
		float maxMod = level*MAX_MOD;
		float currentIncrement = 0.0f;
		
		float finalModFloat = baseMod;
		
		if (varianceIncrements < 1)
		{
			varianceIncrements = 1.0f;
		}
		
		if (exponentIncrement < 1)
		{
			exponentIncrement = 1.0f;
		}
		
		if (baseMod < 1)
		{
			baseMod = 1.0f;
		}
		
		if (maxMod < 1)
		{
			maxMod = 1.0f;
		}
		
		float roll = rand(1.0f, 100.0f);
		
		for (float f = 0.0f; f <= 18; f += exponentIncrement)
		{
			testAbortScript();
			currentIncrement = (float)Math.pow(FLOOR_CHANCE, 1+(f/100) );
			
			if (roll < currentIncrement)
			{
				finalModFloat = baseMod;
				break;
			}
			baseMod++;
		}
		
		if (baseMod > maxMod)
		{
			baseMod = maxMod;
		}
		
		int statMod = (int)baseMod;
		
		return statMod;
	}
	
	
	public static String getArmorNameSuffix(obj_id item) throws InterruptedException
	{
		String strSuffix = "@dynamic_items:armor_none";
		int numStats = 0;
		int highestStatNum = 0;
		int highestStatValue = 0;
		
		String[] statBonusNames =
		{
			"precision_modified", "strength_modified", "stamina_modified",
			"constitution_modified", "agility_modified", "luck_modified", "camouflage"
		};
		
		if (hasObjVar(item, "skillmod.bonus"))
		{
			for (int i = 0; i < statBonusNames.length; i++)
			{
				testAbortScript();
				int thisStat = getIntObjVar(item, "skillmod.bonus."+ statBonusNames[i]);
				
				if (thisStat > 0)
				{
					numStats++;
					
					if (thisStat > highestStatValue)
					{
						highestStatNum = i;
						highestStatValue = thisStat;
					}
				}
			}
			
			if (numStats >= 6)
			{
				strSuffix = "@dynamic_items:armor_six_stat";
			}
			
			if (numStats == 5)
			{
				strSuffix = "@dynamic_items:armor_five_stat";
			}
			
			if (numStats >= 1 && numStats < 5)
			{
				strSuffix = "@dynamic_items:armor_"+ statBonusNames[highestStatNum] + "_"+ numStats;
			}
		}
		
		return strSuffix;
	}
	
	
	public static void setupJunkDealerPrice(obj_id objObject, int intMinValue, int intMaxValue, float fltRange) throws InterruptedException
	{
		
		if (fltRange != 0)
		{
			
			int intPrice = getIntObjVar(objObject, "junkDealer.intPrice");
			float fltDiff = (float)(intMaxValue - intMinValue);
			
			fltDiff = fltDiff * fltRange;
			
			intPrice = intMinValue + (int) fltDiff;
			
			setObjVar(objObject, "junkDealer.intPrice", intPrice);
		}
		else
		{
			int intPrice = getIntObjVar(objObject, "junkDealer.intPrice");
			intPrice = intPrice + rand(intMinValue, intMaxValue);
			
			setObjVar(objObject, "junkDealer.intPrice", intPrice);
		}
		return;
	}
	
	
	public static dictionary applyWeaponItemModifers(obj_id objWeapon, String strModifier, int intMinDamage, int intMaxDamage, dictionary dctItemNames) throws InterruptedException
	{
		
		dictionary dctModifierInfo = null;
		if (strModifier.startsWith("elemental_damage"))
		{
			
			String strTable = "datatables/item/dynamic_item/modifiers/elemental_damage_modifiers.iff";
			dctModifierInfo = dataTableGetRow(strTable, strModifier);
			
			if (dctModifierInfo == null)
			{
				return dctItemNames;
			}
			
			float fltMedianDamage = (float)(intMinDamage + intMaxDamage)/2;
			if (fltMedianDamage < 1)
			{
				return dctItemNames ;
			}
			
			float fltDamageAdded = dctModifierInfo.getFloat("fltDamageAdded");
			int intDamageType = dctModifierInfo.getInt("intDamageType");
			int intDamage = (int)(fltDamageAdded * fltMedianDamage);
			if (intDamage < 1)
			{
				return dctItemNames;
			}
			setWeaponElementalDamage(objWeapon, intDamageType, intDamage);
			
		}
		if (dctModifierInfo == null)
		{
			return dctItemNames;
		}
		setupJunkDealerPrice(objWeapon, dctModifierInfo.getInt("intMinValueAdded"), dctModifierInfo.getInt("intMaxValueAdded"), 0);
		
		String strPrefix = dctModifierInfo.getString("strDisplayPrefix");
		String strSuffix = dctModifierInfo.getString("strDisplaySuffix");
		if (!strPrefix.equals(""))
		{
			
			dctItemNames.put("strPrefix", strPrefix);
		}
		if (!strSuffix.equals(""))
		{
			
			dctItemNames.put("strSuffix", strSuffix);
		}
		
		return dctItemNames;
		
	}
	
	
	public static void setupDynamicItemName(obj_id objItem, dictionary dctNames) throws InterruptedException
	{
		
		String strPrefix = dctNames.getString("strPrefix");
		String strBaseName = dctNames.getString("strBaseName");
		String strSuffix = dctNames.getString("strSuffix");
		
		if (strPrefix != null)
		{
			int intIndex = strPrefix.indexOf('@');
			if (intIndex>-1)
			{
				strPrefix = getString(utils.unpackString(strPrefix));
			}
		}
		else
		{
			strPrefix = "";
		}
		if (strBaseName != null)
		{
			int intIndex = strBaseName.indexOf('@');
			if (intIndex>-1)
			{
				strBaseName = getString(utils.unpackString(strPrefix));
			}
			
		}
		else
		{
			strBaseName = "";
		}
		if (strSuffix != null)
		{
			int intIndex = strSuffix.indexOf('@');
			if (intIndex>-1)
			{
				strSuffix= getString(utils.unpackString(strSuffix));
			}
			
		}
		else
		{
			strSuffix = "";
		}
		
		if ((!strPrefix.equals(""))||(!strSuffix.equals(""))||(!strBaseName.equals("")))
		{
			
			String strName = "";
			if (!strPrefix.equals(""))
			{
				strName +=strPrefix + " ";
			}
			
			if (!strBaseName.equals(""))
			{
				strName+=strBaseName+" ";
			}
			else
			{
				
				strName+=getString(utils.unpackString(getEncodedName(objItem)))+" ";
			}
			if (!strSuffix.equals(""))
			{
				strName += strSuffix;
			}
			
			if (!strName.equals(""))
			{
				setName(objItem, "");
				setName(objItem, strName);
			}
		}
		
		return;
	}
	
	
	public static boolean isDynamicItem(obj_id objItem) throws InterruptedException
	{
		if (hasObjVar(objItem, "dynamic_item"))
		{
			return true;
		}
		return false;
	}
	
	
	public static boolean isUniqueStaticItem(obj_id item) throws InterruptedException
	{
		String item_name = getStaticItemName(item);
		dictionary dict = getMasterItemDictionary(item_name);
		if (dict != null)
		{
			return dict.getInt("unique") == 1;
		}
		else
		{
			LOG( "create", " Could not check for unique, dictionary returned null "+item );
			return false;
		}
	}
	
	
	public static boolean isUniqueStaticItem(String item) throws InterruptedException
	{
		dictionary dict = getMasterItemDictionary(item);
		if (dict != null)
		{
			return dict.getInt("unique") == 1;
		}
		else
		{
			LOG( "create", " Could not check for unique, dictionary returned null "+item );
			return false;
		}
		
	}
	
	
	public static boolean canCreateUniqueStaticItem(obj_id container, String itemName) throws InterruptedException
	{
		if (!isPlayer(getOwner(container)))
		{
			return true;
		}
		
		return (!utils.playerHasStaticItemInBankOrInventory(getOwner(container), itemName));
	}
	
	
	public static boolean canEquip(obj_id player, obj_id item) throws InterruptedException
	{
		dictionary itemData = static_item.getMasterItemDictionary(item);
		
		int requiredLevel = itemData.getInt("required_level");
		String requiredSkill = itemData.getString("required_skill");
		
		boolean canEquip = true;
		
		if (!static_item.validateLevelRequired(player,requiredLevel))
		{
			canEquip = false;
		}
		if (requiredSkill != null && !requiredSkill.equals(""))
		{
			String classTemplate = getSkillTemplate(player);
			if (classTemplate != null && !classTemplate.equals(""))
			{
				if (!classTemplate.startsWith(requiredSkill))
				{
					canEquip = false;
				}
			}
		}
		
		return canEquip;
	}
	
	
	public static void origOwnerCheckStamp(obj_id object, obj_id player) throws InterruptedException
	{
		if (hasObjVar(object, ORIG_OWNER))
		{
			return;
		}
		
		if (!utils.isNestedWithinAPlayer(object))
		{
			return;
		}
		
		if (isIdValid(player))
		{
			setObjVar(object, ORIG_OWNER, player);
		}
		
		return;
	}
	
	
	public static boolean userIsOrigOwner(obj_id object, obj_id player) throws InterruptedException
	{
		if (!isIdValid(object) || !isIdValid(player))
		{
			return false;
		}
		
		obj_id origOwner = utils.getObjIdObjVar(object, ORIG_OWNER);
		obj_id currentOwner = utils.getContainingPlayer(object);
		
		if (!isIdValid(origOwner) || !isIdValid(currentOwner))
		{
			return false;
		}
		
		if (origOwner == currentOwner)
		{
			return true;
		}
		
		return false;
	}
	
	
	public static string_id getStaticItemStringIdName(obj_id object) throws InterruptedException
	{
		if (!isIdValid(object) || !exists(object))
		{
			return null;
		}
		
		if (!isStaticItem(object))
		{
			return null;
		}
		
		String staticItemName = getStaticItemName(object);
		
		return new string_id(STATIC_ITEM_NAME, staticItemName);
	}
}
