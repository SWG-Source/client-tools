package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.utils;
import script.library.healing;
import script.library.player_stomach;
import script.library.prose;
import script.library.food;
import script.library.metrics;
import script.library.doctor_bag;


public class consumable extends script.base_script
{
	public consumable()
	{
	}
	public static final float MAX_AFFECT_DISTANCE = 6.0f;
	
	public static final String[] STAT_NAME =
	{
		"HEALTH",
		"CONSTITUTION",
		"ACTION",
		"STAMINA",
		"MIND",
		"WILLPOWER",
		"POISON",
		"DISEASE"
	};
	
	public static final String VAR_CONSUMABLE_BASE = "consumable";
	public static final String VAR_CONSUMABLE_TARGET = "consumable.target";
	public static final String VAR_CONSUMABLE_MODS = "consumable.mods";
	public static final String VAR_CONSUMABLE_MEDICINE = "consumable.medicine";
	public static final String VAR_CONSUMABLE_PET_MED = "consumable.pet_med";
	public static final String VAR_CONSUMABLE_DROID_MED = "consumable.droid_med";
	
	public static final String VAR_CONSUMABLE_MED_TYPE = "consumable.med_type";
	
	public static final String VAR_CONSUMABLE_CHARGES = "consumable.charges";
	public static final String VAR_CONSUMABLE_STOMACH_VALUES = "consumable.stomachValues";
	
	public static final String VAR_BEEN_CONSUMED = "beenConsumed";
	
	public static final String VAR_SKILL_MOD_REQUIRED = "consumable.skillModRequired";
	public static final String VAR_SKILL_MOD_MIN = "consumable.skillModMin";
	
	public static final String MSG_INSUFFICIENT_SKILL = "@error_message:insufficient_skill";
	public static final String MSG_TARGET_OUT_OF_RANGE = "@error_message:target_out_of_range";
	public static final String MSG_CANNOT_TARGET = "@error_message:cannot_target";
	
	public static final string_id SID_ATTRIBMOD_APPLY = new string_id("base_player","attribmod_apply");
	
	public static final string_id SID_INSUFFICIENT_SKILL = new string_id("error_message","insufficient_skill");
	public static final string_id SID_TARGET_OUT_OF_RANGE = new string_id("error_message","target_out_of_range");
	public static final string_id SID_CANNOT_TARGET = new string_id("error_message","cannot_target");
	public static final string_id SID_TOO_MUCH_SHOCK = new string_id("error_message","too_much_shock");
	public static final string_id SID_YOU_HAVE_GREATER_ENHANCEMENT = new string_id("error_message","you_have_greater_enchancement");
	public static final string_id SID_THEY_HAVE_GREATER_ENHANCEMENT = new string_id("error_message","they_have_greater_enchancement");
	
	public static final string_id SID_SHOCK_EFFECT_LOW = new string_id("healing","shock_effect_low");
	public static final string_id SID_SHOCK_EFFECT_MEDIUM = new string_id("healing","shock_effect_medium");
	public static final string_id SID_SHOCK_EFFECT_HIGH = new string_id("healing","shock_effect_high");
	public static final string_id SID_SHOCK_EFFECT_LOW_TARGET = new string_id("healing","shock_effect_low_target");
	public static final string_id SID_SHOCK_EFFECT_MEDIUM_TARGET = new string_id("healing","shock_effect_medium_target");
	public static final string_id SID_SHOCK_EFFECT_HIGH_TARGET = new string_id("healing","shock_effect_high_target");
	public static final string_id SID_INSUFFICIENT_SKILL_HEAL = new string_id("healing","insufficient_skill_heal");
	
	public static final string_id PROSE_CONSUME_ITEM = new string_id("base_player", "prose_consume_item");
	
	public static final int MT_NONE = 0;
	public static final int MT_HEAL_DAMAGE = 1;
	public static final int MT_HEAL_WOUND = 2;
	public static final int MT_HEAL_STATE = 3;
	public static final int MT_HEAL_ENHANCE = 4;
	public static final int MT_REVIVE_PLAYER = 5;
	
	public static final String FORAGED_DATA = "datatables/foraging/forage_global.iff";
	
	
	public static boolean consumeItem(obj_id player, obj_id target, obj_id item) throws InterruptedException
	{
		return consumeItem (player, target, item, true);
	}
	
	public static boolean consumeItem(obj_id player, obj_id target, obj_id item, boolean checkPvpStatus) throws InterruptedException
	{
		if (!isIdValid(player) || !isIdValid(target) || !isIdValid(item))
		{
			return false;
		}
		
		if (!isMob(player) || !isMob(target))
		{
			return false;
		}
		
		if (!player.isLoaded() || !target.isLoaded() || !item.isLoaded())
		{
			sendSystemMessage(player, SID_TARGET_OUT_OF_RANGE);
			return false;
		}
		
		obj_id owner = utils.getContainingPlayer(item);
		
		if (player == owner)
		{
			
		}
		else
		{
			
			if (isIdValid(owner))
			{
				
				CustomerServiceLog("VendorCreditDupeExploit", "%TU tried to consume an item ("+ item + "), but is no longer in possession of it! %TT currently has the item.", player, owner);
				
			}
			else
			{
				
				CustomerServiceLog("VendorCreditDupeExploit", "%TU tried to consume an item ("+ item + "), but is no longer in possession of it!", player);
			}
			
			return false;
		}
		
		String medicinebuff = healing.SCRIPT_VAR_MEDICINE_BUFF;
		String medicinerebuff = healing.SCRIPT_VAR_MEDICINE_REBUFF;
		
		if (healing.isRangedMedicine(item))
		{
			
			float healing_range = (float)healing.getHealingRange(item);
			float healing_range_mod = (float)getSkillStatMod(player, "healing_range");
			healing_range = healing_range + ((16 * healing_range_mod) / 100.0f);
			
			if (getDistance(player, target) > healing_range)
			{
				sendSystemMessage(player, SID_TARGET_OUT_OF_RANGE);
				return false;
			}
		}
		else
		{
			if (getDistance(player, target) > MAX_AFFECT_DISTANCE)
			{
				sendSystemMessage(player, SID_TARGET_OUT_OF_RANGE);
				return false;
			}
		}
		
		boolean isConsumable = false;
		int[] vol =
		{
			60, 0
		};
		attrib_mod[] am = null;
		String[] skillReq = null;
		int[] skillMin = null;
		
		if (hasScript( item, "item.comestible.foraged" ))
		{
			isConsumable = true;
			am = getForagedFoodMods( item );
			if (am == null)
			{
				isConsumable = false;
			}
			else
			{
				isConsumable = true;
			}
		}
		else
		{
			
			isConsumable = hasObjVar( item, VAR_CONSUMABLE_BASE );
			if (isConsumable)
			{
				vol = getIntArrayObjVar( item, VAR_CONSUMABLE_STOMACH_VALUES );
				am = getAttribModArrayObjVar( item, VAR_CONSUMABLE_MODS );
				skillReq = getStringArrayObjVar(item, VAR_SKILL_MOD_REQUIRED);
				skillMin = getIntArrayObjVar(item, VAR_SKILL_MOD_MIN);
			}
			else
			{
				
				if (hasScript( item, "item.comestible.crafted" ))
				{
					isConsumable = true;
				}
			}
		}
		
		if (!canEatFood( target, item ))
		{
			return false;
		}
		
		if (isConsumable)
		{
			boolean canTarget = false;
			if (player == target)
			{
				canTarget = true;
			}
			else
			{
				if (checkPvpStatus)
				{
					if (factions.pvpDoAllowedHelpCheck(player, target))
					{
						canTarget = true;
					}
				}
				else
				{
					canTarget = true;
				}
			}
			
			if (!canTarget)
			{
				sendSystemMessage(player, SID_CANNOT_TARGET);
				return false;
			}
			
			if (hasObjVar( item, "race_restriction" ))
			{
				int race = getIntObjVar( item, "race_restriction");
				int target_species = getSpecies( target );
				if (target_species == -2)
				{
					sendSystemMessage( target, new string_id( "error_message", "pets_only") );
					return false;
				}
				else if (race != target_species)
				{
					sendSystemMessage( target, new string_id( "error_message", "race_restriction") );
					return false;
				}
			}
			
			if ((vol == null) || (vol.length == 0))
			{
				return false;
			}
			
			float dcy = am[0].getDecay();
			if (am[0].getDuration() > 0)
			{
				attrib_mod[] modifyBuffValue = modifyBuffMods(player, target, item, am);
				if (modifyBuffValue != null && modifyBuffValue.length > 0)
				{
					am = modifyBuffValue;
				}
			}
			else if ((am.length == 2) && ( (int)dcy == (int)MOD_POOL ))
			{
				
				LOG("buffs", "Roger Dodger");
				am = utils.addMindAttribToStim(am[0].getValue());
				setObjVar(getSelf(), consumable.VAR_CONSUMABLE_MODS, am);
			}
			
			if ((skillReq != null) && (skillReq.length != 0) && (skillMin != null) && (skillMin.length != 0))
			{
				if (skillReq.length == skillMin.length)
				{
					boolean canUse = true;
					for (int i = 0; i < skillReq.length; i++)
					{
						testAbortScript();
						int skillMod = getSkillStatMod(player, skillReq[i]);
						if (skillMod < skillMin[i])
						{
							canUse = false;
						}
					}
					
					if (!canUse)
					{
						String[] reqs = new String[skillReq.length];
						
						for (int n = 0; n < skillReq.length; n++)
						{
							testAbortScript();
							reqs[n] = skillReq[n] + "[" + getSkillStatMod(player, skillReq[n]) + "/" + skillMin[n] + "]";
						}
						
						if (skillReq.length > 1)
						{
							sui.listbox(player, player, MSG_INSUFFICIENT_SKILL, reqs);
							sendSystemMessage(player, SID_INSUFFICIENT_SKILL);
						}
						else
						{
							String skill_mod_name = "@stat_n:"+ skillReq[0];
							prose_package pp = prose.getPackage(SID_INSUFFICIENT_SKILL_HEAL, skill_mod_name, skillMin[0]);
							sendSystemMessageProse(player, pp);
						}
						return false;
					}
					
					if (healing.isMedicine(item))
					{
						
						float multiplier = healing.getHealingMultiplier(player, item);
						float final_multiplier = healing.applyShockWoundModifier(multiplier, target);
						
						if (final_multiplier <= 0.0f)
						{
							sendSystemMessage(player, SID_TOO_MUCH_SHOCK);
							return false;
						}
						
						float shock_effect = final_multiplier / multiplier;
						if (shock_effect < .3f)
						{
							sendSystemMessage(player, SID_SHOCK_EFFECT_HIGH);
							if (player != target)
							{
								sendSystemMessage(target, SID_SHOCK_EFFECT_HIGH_TARGET);
							}
						}
						else if (shock_effect < .5f)
						{
							sendSystemMessage(player, SID_SHOCK_EFFECT_MEDIUM);
							if (player != target)
							{
								sendSystemMessage(target, SID_SHOCK_EFFECT_MEDIUM_TARGET);
							}
						}
						else if (shock_effect < .8f)
						{
							sendSystemMessage(player, SID_SHOCK_EFFECT_LOW);
							if (player != target)
							{
								sendSystemMessage(target, SID_SHOCK_EFFECT_LOW_TARGET);
							}
						}
						
						if (am[0].getAttribute() > 8)
						{
							final_multiplier = 1f;
						}
						
						attrib_mod[] am_new = healing.modifyMedicineAttributes(am, final_multiplier);
						am = (attrib_mod[]) am_new.clone();
						
						if (healing.isBuffMedicine(item))
						{
							String buff_name = "";
							int current_value = 0;
							int failed = 0;
							for (int x = 0; x < am.length; x++)
							{
								testAbortScript();
								if (healing.hasEnhancement(target, am[x].getAttribute()))
								{
									
									buff_name = "medical_enhance_"+ healing.attributeToString(am[x].getAttribute()).toLowerCase();
									current_value = healing.getHealEnhanceValue(target, am[x].getAttribute());
									if (am[x].getValue() >= current_value)
									{
										
										removeAttribOrSkillModModifier(target, buff_name);
										
										utils.setScriptVar(target, medicinerebuff +"."+x, current_value);
									}
									else
									{
										failed ++;
										if (player == target)
										{
											sendSystemMessage(player, SID_YOU_HAVE_GREATER_ENHANCEMENT);
										}
										else
										{
											prose_package ppGreaterEnhancement = prose.getPackage(SID_THEY_HAVE_GREATER_ENHANCEMENT, target);
											sendSystemMessageProse(player, ppGreaterEnhancement);
										}
									}
									
								}
							}
							if (failed == am.length)
							{
								return false;
							}
						}
					}
				}
				else
				{
					debugSpeakMsg(player, "consumable::consumeItem(): skill mod arrays are not synchronized");
				}
			}
			else if ((skillReq == null) && (skillMin == null))
			{
				
				if (hasObjVar(item, consumable.VAR_CONSUMABLE_DROID_MED))
				{
					
					if (hasObjVar(item, "consumable.energy"))
					{
						
						int totalEnergy = getIntObjVar(item, "consumable.energy");
						
						int toHeal[] = healing.distributeHAMDamageHealingPoints(target, totalEnergy);
						attrib_mod[] tempAm = new attrib_mod[3];
						
						if (toHeal.length != 3)
						{
							return false;
						}
						
						for (int y = 0; y < 3; y++)
						{
							testAbortScript();
							
							tempAm[y] = new attrib_mod(y*3, toHeal[y], 0.0f, 0.0f, MOD_POOL);
						}
						
						am = tempAm;
						
					}
					else
					{
						
						int totalPower = am[0].getValue();
						
						int toHeal[] = healing.distributeHAMWoundHealingPoints(target, totalPower);
						attrib_mod[] tempAm = new attrib_mod[3];
						
						if (toHeal.length != 3)
						{
							return false;
						}
						
						for (int z = 0; z < 3; z++)
						{
							testAbortScript();
							
							tempAm[z] = new attrib_mod(z*3, toHeal[z], 0.0f, healing.AM_HEAL_WOUND, 0.0f);
						}
						
						am = tempAm;
					}
				}
			}
			else
			{
				debugSpeakMsg(player, "consumable::consumeItem(): skill mod arrays are fubar!");
			}
			
			boolean wasConsumed = player_stomach.addToStomach( player, target, vol );
			
			if (wasConsumed)
			{
				if (checkPvpStatus)
				{
					if (target != player)
					{
						pvpHelpPerformed( player, target );
					}
				}
				
				if (healing.isRevivePack(item))
				{
					pclib.clearEffectsForDeath(target);
					
					setAttrib(target, HEALTH, 1);
				}
				
				attrib_mod[] modifiedAm = modifyConsumableMods(player, target, item, am);
				if (modifiedAm != null && modifiedAm.length > 0)
				{
					am = modifiedAm;
				}
				
				if (healing.isBuffMedicine(item) && (am != null))
				{
					for (int x = 0; x < am.length; x++)
					{
						testAbortScript();
						String buff_name = "medical_enhance_"+ healing.attributeToString(am[x].getAttribute()).toLowerCase();
						utils.setScriptVar( target, medicinebuff +"."+x, am[x].getValue() );
						utils.setScriptVar( target, "healing."+ buff_name, am[x].getValue() );
						int buff_type = am[x].getAttribute();
						if (buff_type <= 5)
						{
							addAttribModifier(target, buff_name, buff_type, am[x].getValue(), am[x].getDuration(), am[x].getAttack(), am[x].getDecay(), true, false, true);
							
						}
						else if (buff_type == 6)
						{
							addSkillModModifier(target, buff_name, "resistance_poison", am[x].getValue(), am[x].getDuration(), false, true);
							
							if (hasObjVar(item, healing.VAR_HEALING_ABSORPTION))
							{
								addSkillModModifier(target, buff_name+".absorption", "absorption_poison", getIntObjVar(item, healing.VAR_HEALING_ABSORPTION), am[x].getDuration(), false, false);
							}
						}
						else if (buff_type == 7)
						{
							addSkillModModifier(target, buff_name, "resistance_disease", am[x].getValue(), am[x].getDuration(), false, true);
							
							if (hasObjVar(item, healing.VAR_HEALING_ABSORPTION))
							{
								addSkillModModifier(target, buff_name+".absorption", "absorption_disease", getIntObjVar(item, healing.VAR_HEALING_ABSORPTION), am[x].getDuration(), false, false);
							}
						}
					}
					
					metrics.logBuffStatus(target);
				}
				else
				{
					
					if (am != null)
					{
						
						boolean isBuff = false;
						for (int i = 0; i < am.length; i++)
						{
							testAbortScript();
							debugServerConsoleMsg(target, ">>>>>>>>>>>> PREPARING TO APPLY MOD TO ATTRIBUTE: "+ am[i].getAttribute());
							
							if (am[i].getAttribute() == HEALTH)
							{
								debugServerConsoleMsg(target, ">>>>>>>>>>>> ATTEMPTING TO APPLY MOD -- "+ am[i].getAttribute() + " LOOKS LIKE HEALTH TO ME!");
								utils.addAttribMod(target, am[i]);
							}
							
							if (am[i].getDuration() > 0)
							{
								isBuff = true;
							}
						}
						
						if (isBuff)
						{
							metrics.logBuffStatus(target);
						}
					}
				}
				
				applyFoodEffects( target, item, am );
				
				if (!healing.isMedicine(item))
				{
					
					String snd = "clienteffect/";
					switch ( getSpecies(player) )
					{
						case SPECIES_HUMAN:
						case SPECIES_BOTHAN:
						case SPECIES_TWILEK:
						case SPECIES_ZABRAK:
						snd += "human_";
						break;
						
						case SPECIES_MON_CALAMARI:
						case SPECIES_RODIAN:
						case SPECIES_TRANDOSHAN:
						snd += "reptile_";
						break;
						
						case SPECIES_WOOKIEE:
						snd += "wookiee_";
						break;
					}
					
					int gender = getGender(player);
					switch ( gender )
					{
						case GENDER_FEMALE:
						snd += "female_eat.cef";
						break;
						
						case GENDER_MALE:
						default:
						snd += "male_eat.cef";
						break;
					}
					
					if (!snd.equals(""))
					{
						playClientEffectLoc( player, snd, getLocation(player), getScale(player) );
					}
					
					prose_package pp = prose.getPackage(PROSE_CONSUME_ITEM, player, item);
					sendSystemMessageProse(target, pp);
				}
				
				removeObjVar( item, VAR_BEEN_CONSUMED );
				return decrementCharges(item, player);
			}
			else
			{
				removeObjVar( item, VAR_BEEN_CONSUMED );
			}
			return false;
		}
		
		return false;
	}
	
	
	public static boolean consumeItem(obj_id player, obj_id item) throws InterruptedException
	{
		return consumeItem(player, player, item);
	}
	
	
	public static attrib_mod[] modifyConsumableMods(obj_id player, obj_id target, obj_id item, attrib_mod[] am) throws InterruptedException
	{
		if (!isIdValid(player) || !isIdValid(target) || !isIdValid(item))
		{
			return null;
		}
		
		if (am == null || am.length == 0)
		{
			return null;
		}
		
		float modval = 1f;
		float moddur = 1f;
		
		int species = getSpecies(target);
		if ((species == SPECIES_TWILEK) && (am != null))
		{
			moddur *= 1.1f;
		}
		
		if (utils.hasScriptVar(player, "registerWithLocation") && factions.isDeclared(player) && healing.isMedicine(item))
		{
			obj_id regLoc = utils.getObjIdScriptVar(player, "registerWithLocation");
			if (isIdValid(regLoc))
			{
				String lFac = factions.getFaction(regLoc);
				String pFac = factions.getFaction(player);
				if (lFac != null && pFac != null && lFac.equals(pFac))
				{
					modval *= 1.25f;
				}
			}
		}
		
		if (modval != 1f || moddur != 1f)
		{
			for (int x = 0; x < am.length; x++)
			{
				testAbortScript();
				float atk = am[x].getAttack();
				float decay = am[x].getDecay();
				if (moddur != 1f && atk >= 0f && decay >= 0f)
				{
					attrib_mod tmp = new attrib_mod(am[x].getAttribute(), am[x].getValue(), am[x].getDuration() * moddur, atk, decay);
					if (tmp != null)
					{
						am[x] = tmp;
					}
				}
				
				if (modval != 1f && (atk == healing.AM_HEAL_WOUND || atk == healing.AM_HEAL_SHOCK || decay == MOD_ANTIDOTE))
				{
					
					attrib_mod tmp = new attrib_mod(am[x].getAttribute(), (int)(am[x].getValue() * modval), am[x].getDuration(), atk, decay);
					if (tmp != null)
					{
						am[x] = tmp;
					}
				}
			}
		}
		
		return am;
	}
	
	
	public static attrib_mod[] modifyBuffMods(obj_id player, obj_id target, obj_id item, attrib_mod[] am) throws InterruptedException
	{
		if (!isIdValid(player) || !isIdValid(target) || !isIdValid(item))
		{
			return null;
		}
		if (am == null || am.length == 0)
		{
			return null;
		}
		
		for (int x = 0; x < am.length; x++)
		{
			testAbortScript();
			float atk = am[x].getAttack();
			float decay = am[x].getDecay();
			int buffValue = 0;
			
			attrib_mod tmp = new attrib_mod(am[x].getAttribute(), buffValue, am[x].getDuration(), atk, decay);
			if (tmp != null)
			{
				am[x] = tmp;
			}
		}
		
		return am;
	}
	
	
	public static boolean createConsumable(obj_id item, attrib_mod[] am, int[] stomach, dictionary skillReq) throws InterruptedException
	{
		if ((item == null) || (am == null) || (stomach == null) || (stomach.length != player_stomach.STOMACH_MAX))
		{
			return false;
		}
		
		boolean litmus = true;
		
		if (am.length == 0)
		{
			
			return false;
		}
		
		litmus &= setObjVar(item, VAR_CONSUMABLE_MODS, am);
		litmus &= setObjVar(item, VAR_CONSUMABLE_STOMACH_VALUES, stomach);
		
		if ((skillReq != null) && (!skillReq.isEmpty()))
		{
			java.util.Enumeration keys = skillReq.keys();
			
			String[] skillModReq = new String[skillReq.size()];
			int[] skillModMin = new int[skillReq.size()];
			
			for (int i = 0; i < skillReq.size(); i++)
			{
				testAbortScript();
				skillModReq[i] = (String)keys.nextElement();
				skillModMin[i] = skillReq.getInt(skillModReq[i]);
			}
			
			if (skillModReq.length == 0 || skillModMin.length == 0)
			{
				
				return false;
			}
			
			litmus &= setObjVar(item, VAR_SKILL_MOD_REQUIRED, skillModReq);
			litmus &= setObjVar(item, VAR_SKILL_MOD_MIN, skillModMin);
		}
		return litmus;
	}
	
	
	public static boolean decrementCharges(obj_id item, obj_id player) throws InterruptedException
	{
		if (item == null)
		{
			return false;
		}
		
		if (doctor_bag.isDoctorBag( item ))
		{
			doctor_bag.decrementSurrogateCharge( item );
		}
		
		int charges = getCount(item);
		if (charges > 1000)
		{
			setCount( item, 1000 );
		}
		else if (charges > 1)
		{
			incrementCount(item, -1);
		}
		else
		{
			if (!doctor_bag.isDoctorBag( item ))
			{
				destroyObject(item);
			}
		}
		
		return true;
	}
	
	
	public static boolean decrementSpecificObjectRecursive(obj_id player, String template, String objvar) throws InterruptedException
	{
		obj_id inventory = utils.getInventoryContainer(player);
		obj_id[] device = utils.getAllItemsInContainerByTemplate(inventory, template, true);
		if (device == null)
		{
			return false;
		}
		for (int i = 0; i < device.length; i++)
		{
			testAbortScript();
			if (getTemplateName(device[i]).equals(template) && hasObjVar(device[i], objvar))
			{
				decrementCount(device[i]);
				return true;
			}
		}
		return false;
	}
	
	
	public static boolean decrementObjectInventoryOrEquipped(obj_id player, String template, String objvar) throws InterruptedException
	{
		obj_id inventory = utils.getInventoryContainer(player);
		obj_id[] inventoryContents = getInventoryAndEquipment(player);
		
		if (inventoryContents == null)
		{
			return false;
		}
		
		for (int i = 0; i < inventoryContents.length; i++)
		{
			testAbortScript();
			if (getTemplateName(inventoryContents[i]).equals(template) && hasObjVar(inventoryContents[i], objvar))
			{
				decrementCount(inventoryContents[i]);
				return true;
			}
		}
		return false;
	}
	
	
	public static boolean decrementSpecificObject(obj_id player, String template, String objvar) throws InterruptedException
	{
		obj_id inventory = utils.getInventoryContainer(player);
		obj_id[] device = utils.getContainedObjectsWithObjVar(inventory, objvar);
		
		if (device == null)
		{
			return false;
		}
		for (int i = 0; i < device.length; i++)
		{
			testAbortScript();
			if (getTemplateName(device[i]).equals(template))
			{
				decrementCount(device[i]);
				return true;
			}
		}
		return false;
	}
	
	
	public static attrib_mod[] getForagedFoodMods(obj_id item) throws InterruptedException
	{
		int mod_time = 0;
		int health_mod = 0;
		int action_mod = 0;
		int mind_mod = 0;
		
		if (item == null || !isIdValid(item))
		{
			return null;
		}
		
		String mytemplate = getTemplateName(item);
		String prefix = "object/tangible/";
		mytemplate = mytemplate.substring( prefix.length() );
		
		if (mytemplate == null || mytemplate.equals(""))
		{
			
			detachScript( item, "item.comestible.foraged");
			return null;
		}
		
		int index = -1;
		String tangibles[] = dataTableGetStringColumn(FORAGED_DATA, "ITEM_TANGIBLE");
		
		if (tangibles == null || tangibles.length == 0)
		{
			
			detachScript( item, "item.comestible.foraged");
			return null;
		}
		
		for (int i = 0; i < tangibles.length; i++)
		{
			testAbortScript();
			if (mytemplate.equals(tangibles[i]))
			{
				
				index = i;
			}
		}
		
		if (index == -1)
		{
			
			detachScript( item, "item.comestible.foraged");
			return null;
		}
		
		mod_time = dataTableGetInt(FORAGED_DATA, index, "MOD_TIME");
		health_mod = dataTableGetInt(FORAGED_DATA, index, "MOD_HEALTH");
		action_mod = dataTableGetInt(FORAGED_DATA, index, "MOD_ACTION");
		mind_mod = dataTableGetInt(FORAGED_DATA, index, "MOD_MIND");
		
		int numItems = 0;
		
		if (health_mod > 0)
		{
			numItems++;
		}
		
		if (action_mod > 0)
		{
			numItems++;
		}
		
		if (mind_mod > 0)
		{
			numItems++;
		}
		
		attrib_mod[] am = new attrib_mod[numItems];
		
		numItems = 0;
		
		if (health_mod > 0)
		{
			am[numItems++] = new attrib_mod( HEALTH, health_mod, mod_time, 0.0f, 0.0f );
		}
		
		if (action_mod > 0)
		{
			am[numItems++] = new attrib_mod( ACTION, action_mod, mod_time, 0.0f, 0.0f );
		}
		
		if (mind_mod > 0)
		{
			am[numItems++] = new attrib_mod( MIND, mind_mod, mod_time, 0.0f, 0.0f );
		}
		
		return am;
	}
	
	
	public static boolean canEatFood(obj_id target, obj_id item) throws InterruptedException
	{
		
		if (hasObjVar( item, "duration" ))
		{
			String type = getStringObjVar( item, "duration.type");
			
			if (utils.hasScriptVar( target, "food."+type+".dur" ))
			{
				sendSystemMessage( target, new string_id( "combat_effects", "already_affected") );
				return false;
			}
		}
		
		if (hasObjVar( item, "delayed" ))
		{
			String type = getStringObjVar( item, "delayed.type");
			
			if (utils.hasScriptVar( target, "food."+type+".dur" ))
			{
				sendSystemMessage( target, new string_id( "combat_effects", "already_affected") );
				return false;
			}
		}
		
		if (hasObjVar( item, "skill_mod" ))
		{
			
			String mod_name = getStringObjVar( item, "skill_mod.name");
			String mod_id = "food_"+ mod_name;
			
			if (utils.hasScriptVar( target, mod_id ))
			{
				sendSystemMessage( target, new string_id( "combat_effects", "already_affected") );
				return false;
			}
		}
		
		return true;
	}
	
	
	public static void applyFoodEffects(obj_id target, obj_id item, attrib_mod[] am) throws InterruptedException
	{
		
		String templateName = getTemplateName(item);
		int lastslash = templateName.lastIndexOf( '/');
		int lastdot = templateName.lastIndexOf( '.');
		templateName = templateName.substring( lastslash+1, lastdot );
		
		if (hasObjVar( item, "stat_mod" ))
		{
			if (am == null)
			{
				return;
			}
			
			float dur = am[0].getDuration();
			addBuffIcon( target, "food."+templateName, dur );
		}
		
		else if (hasObjVar( item, "delayed" ))
		{
			
			String type = getStringObjVar( item, "delayed.type");
			utils.setScriptVar( target, "food."+type+".dur", getIntObjVar( item, "delayed.dur") );
			utils.setScriptVar( target, "food."+type+".eff", getIntObjVar( item, "delayed.eff") );
			utils.setScriptVar( target, "food."+type+".time", getGameTime() );
			
			addBuffIcon( target, "food."+type, 3600 );
			
			dictionary outparams = new dictionary();
			outparams.put( "type", type );
			messageTo( target, "removeDelayedFoodEffect", outparams, 3600f, false );
		}
		
		else if (hasObjVar( item, "duration" ))
		{
			String type = getStringObjVar( item, "duration.type");
			
			if (utils.hasScriptVar( target, "food."+type+".dur" ))
			{
				sendSystemMessage( target, new string_id( "combat_effects", "no_additional_effect") );
				return;
			}
			
			int duration = getIntObjVar( item, "duration.dur");
			utils.setScriptVar( target, "food."+type+".dur", duration );
			utils.setScriptVar( target, "food."+type+".eff", getIntObjVar( item, "duration.eff") );
			
			addBuffIcon( target, "food."+type, duration );
			
			dictionary outparams = new dictionary();
			outparams.put( "type", type );
			messageTo( target, "removeDurationFoodEffect", outparams, duration, false );
		}
		
		else if (hasObjVar( item, "instant" ))
		{
			
			food.applyInstantEffect( target, item );
		}
		
		else if (hasObjVar( item, "skill_mod" ))
		{
			
			String mod_name = getStringObjVar( item, "skill_mod.name");
			int amount = (int) getFloatObjVar( item, "skill_mod.amount");
			int duration = (int) getFloatObjVar( item, "skill_mod.dur");
			String mod_id = "food_"+ mod_name;
			
			if (utils.hasScriptVar( target, mod_id ))
			{
				
				prose_package pp = prose.getPackage( new string_id( "combat_effects", "dupe_skill_mod"), localize( new string_id( "stat_n", mod_name ) ) );
				sendSystemMessageProse( target, pp );
				return;
			}
			
			utils.setScriptVar( target, mod_id, getGameTime() );
			addSkillModModifier( target, mod_id, mod_name, amount, duration, true, true );
			
			addBuffIcon( target, "food."+templateName, duration );
			
			prose_package pp = prose.getPackage( new string_id( "combat_effects", "skill_mod_buffed"), localize( new string_id( "stat_n", mod_name ) ) );
			sendSystemMessageProse( target, pp );
		}
		
		else if (hasObjVar( item, "mind_heal" ))
		{
			
			int mind_heal = getIntObjVar( item, "mind_heal");
			int mind = getAttrib( target, MIND );
			int wounds = getAttribWound( target, MIND );
			int max_mind = getMaxAttrib( target, MIND ) - wounds;
			
			int damage = max_mind - mind;
			if (mind_heal > damage)
			{
				mind_heal = damage;
			}
			
			setAttrib( target, MIND, getUnmodifiedAttrib( target, MIND ) + mind_heal );
			
			prose_package pp = prose.getPackage( new string_id( "combat_effects", "food_mind_heal"), mind_heal );
			sendSystemMessageProse( target, pp );
		}
	}
}
