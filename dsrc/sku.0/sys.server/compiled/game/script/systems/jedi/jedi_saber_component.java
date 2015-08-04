package script.systems.jedi;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.ai_lib;
import script.library.jedi;
import script.library.jedi_trials;
import script.library.colors_hex;
import script.library.sui;
import script.library.utils;
import script.library.static_item;


public class jedi_saber_component extends script.base_script
{
	public jedi_saber_component()
	{
	}
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		
		detachScript(self, "systems.crafting.weapon.component.crafting_weapon_component_attribute");
		if (!static_item.isStaticItem(self))
		{
			jedi.clickyCombatPearlFix(self);
		}
		
		String template = getTemplateName(self);
		if (template.endsWith("lance_module_force_crystal.iff"))
		{
			removeObjVar(self, jedi.VAR_CRYSTAL_STATS + "."+ jedi.VAR_MIN_DMG);
			removeObjVar(self, jedi.VAR_CRYSTAL_STATS + "."+ jedi.VAR_MAX_DMG);
		}
		
		removeObjVar(self, jedi.VAR_CRYSTAL_STATS + "."+ jedi.VAR_SPEED);
		removeObjVar(self, jedi.VAR_CRYSTAL_STATS + "."+ jedi.VAR_WOUND);
		removeObjVar(self, jedi.VAR_CRYSTAL_STATS + "."+ jedi.VAR_ATTACK_COST);
		removeObjVar(self, jedi.VAR_CRYSTAL_STATS + "."+ jedi.VAR_FORCE);
		removeObjVar(self, jedi.VAR_CRYSTAL_STATS + "."+ jedi.VAR_ACCURACY);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		if (!static_item.isStaticItem(self))
		{
			
			int level = -1;
			obj_id inv = getContainedBy(self);
			LOG("jedi", "Crystal created in - "+inv);
			if (inv != null)
			{
				if (isIdValid(inv))
				{
					obj_id target = getContainedBy(inv);
					LOG("jedi", " Inventory contained by - "+target);
					if (isIdValid(target))
					{
						final String mobType = getCreatureName(target);
						LOG("jedi", " Mob type = "+mobType);
						
						level = getLevel(target);
						LOG("jedi", " level = "+level);
					}
				}
				if (level == -1)
				{
					level = rand (1, 50);
				}
				
				jedi.initializeCrystal(self, level);
			}
			else
			{
				dictionary data = new dictionary();
				data.put("attempts", 0);
				messageTo(self, "setCrystalLevel", data, 0.5f, false);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int setCrystalLevel(obj_id self, dictionary params) throws InterruptedException
	{
		int attempts = params.getInt("attempts");
		
		int level = -1;
		obj_id inv = getContainedBy(self);
		LOG("jedi", "Crystal created in - "+inv);
		if (inv != null)
		{
			if (isIdValid(inv))
			{
				obj_id target = getContainedBy(inv);
				LOG("jedi", " Inventory contained by - "+target);
				if (isIdValid(target))
				{
					final String mobType = getCreatureName(target);
					LOG("jedi", " Mob type = "+mobType);
					
					level = getLevel(target);
					LOG("jedi", " level = "+level);
				}
			}
			if (level == -1)
			{
				level = rand (1, 50);
			}
			
			jedi.initializeCrystal(self, level);
		}
		else
		{
			attempts++;
			
			if (attempts < 5)
			{
				params.put("attempts", attempts);
				messageTo(self, "setCrystalLevel", params, 0.5f, false);
			}
			else
			{
				jedi.initializeCrystal(self, rand (1, 50));
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		if (isDisabled(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		csLogCrystal(self, "destroyed");
		
		obj_id inv = getContainedBy(self);
		obj_id saber = getContainedBy(inv);
		
		if (!jedi.isLightsaber(saber))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] contents = getContents(inv);
		
		if (contents == null || contents.length == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		for (int i = 0; i < contents.length; i++)
		{
			testAbortScript();
			if (contents[i] == self)
			{
				jedi.removeCrystalStats(saber, self);
				break;
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectDisabled(obj_id self, obj_id killer) throws InterruptedException
	{
		obj_id inv = getContainedBy(self);
		obj_id saber = getContainedBy(inv);
		
		csLogCrystal(self, "decayed");
		
		LOG ("saber_test", "Crystal ("+self+") disabled.");
		
		if (jedi.isLightsaber(saber))
		{
			jedi.removeCrystalStats(saber, self);
		}
		
		String name = getEncodedName(self);
		if (name.startsWith("@"))
		{
			name = localize(getNameStringId(self));
		}
		name = "\\"+colors_hex.RED+""+name + " (broken)\\#.";
		
		LOG ("saber_test", "Setting Crystal name to: "+name);
		
		setName(self, name);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (!jedi.isForceSensitiveLevelRequired(player, jedi.MIN_CRYSTAL_TUNE_PLAYER_LEVEL))
		{
			return SCRIPT_CONTINUE;
		}
		if (!utils.isNestedWithin(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (canManipulate(player, self, false, true, 15, true))
		{
			if (!jedi.isCrystalTuned(self))
			{
				menu_info_data mid = mi.getMenuItemByType(menu_info_types.SERVER_PET_OPEN);
				if (mid != null)
				{
					mid.setServerNotify(true);
				}
				else
				{
					mi.addRootMenu (menu_info_types.SERVER_PET_OPEN, new string_id("jedi_spam","tune_crystal"));
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (!jedi.isForceSensitiveLevelRequired(player, jedi.MIN_CRYSTAL_TUNE_PLAYER_LEVEL))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (canManipulate(player, self, false, true, 15, true))
		{
			if (item == menu_info_types.SERVER_PET_OPEN)
			{
				verifyTune(player);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void verifyTune(obj_id player) throws InterruptedException
	{
		obj_id self = getSelf();
		
		int pid = sui.createSUIPage( sui.SUI_MSGBOX, self, player, "handleVerifyTune");
		
		setSUIProperty(pid, "", "Size", "250,175");
		setSUIProperty(pid, sui.MSGBOX_TITLE, sui.PROP_TEXT, utils.packStringId(new string_id("jedi_spam", "confirm_tune_title")));
		setSUIProperty(pid, sui.MSGBOX_PROMPT, sui.PROP_TEXT, utils.packStringId(new string_id("jedi_spam", "confirm_tune_prompt")));
		
		sui.msgboxButtonSetup(pid, sui.OK_CANCEL);
		
		sui.showSUIPage( pid );
	}
	
	
	public int handleVerifyTune(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		int bp = sui.getIntButtonPressed(params);
		switch (bp)
		{
			case sui.BP_CANCEL:
			return SCRIPT_CONTINUE;
			
			case sui.BP_OK:
			tuneCrystal(player);
			return SCRIPT_CONTINUE;
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void tuneCrystal(obj_id player) throws InterruptedException
	{
		obj_id self = getSelf();
		
		setObjVar(self, jedi.VAR_CRYSTAL_OWNER_ID, player);
		setObjVar(self, jedi.VAR_CRYSTAL_OWNER_NAME, getName(player));
		
		String name = getEncodedName(self);
		if (name.startsWith("@"))
		{
			name = localize(getNameStringId(self));
		}
		name = "\\"+colors_hex.GREEN+""+name + " (tuned)\\#.";
		setName(self, name);
		
		sendSystemMessage(player, new string_id("jedi_spam", "crystal_tune_success"));
		sendDirtyObjectMenuNotification(self);
	}
	
	
	public int distributedRand(int min, int max, int level) throws InterruptedException
	{
		final int levelMin = 60;
		final int levelMax = 280;
		
		boolean inverted = false;
		int _min = min;
		int _max = max;
		
		if (min > max)
		{
			inverted = true;
			int temp = min;
			min = max;
			max = temp;
			
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
		
		int minRand = rand(min, (int)(mid+0.5f));
		int maxRand = rand((int)(mid+0.5f), max);
		
		int randNum = rand(minRand, maxRand);
		
		if (inverted)
		{
			randNum = _min + (_max - randNum);
		}
		
		return randNum;
	}
	
	
	public float distributedRand(float min, float max, int level) throws InterruptedException
	{
		final int levelMin = 60;
		final int levelMax = 280;
		
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
	
	
	public void csLogCrystal(obj_id crystal, String state) throws InterruptedException
	{
		int damage = getIntObjVar(crystal, jedi.VAR_CRYSTAL_STATS + "."+ jedi.VAR_MIN_DMG);
		float wound = getFloatObjVar(crystal, jedi.VAR_CRYSTAL_STATS + "."+ jedi.VAR_WOUND);
		float force = getFloatObjVar(crystal, jedi.VAR_CRYSTAL_STATS + "."+ jedi.VAR_FORCE);
		float speed = getFloatObjVar(crystal, jedi.VAR_CRYSTAL_STATS + "."+ jedi.VAR_SPEED);
		
		obj_id player = getContainedBy(crystal);
		
		while (!isPlayer(player))
		{
			testAbortScript();
			obj_id container = getContainedBy(player);
			
			if (player != container)
			{
				player = container;
			}
			else
			{
				break;
			}
		}
		
		CustomerServiceLog("jedi_saber", "A crystal("+crystal+" - "+getTemplateName(crystal)+") owned by %TU was "+state+". (Dam:"+damage+" Spd:"+speed+" Wnd:"+wound+" Frc:"+force+")", player);
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		int idx = utils.getValidAttributeIndex(names);
		if (idx == -1)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		if (jedi.isCrystalTuned(self))
		{
			if (hasObjVar(self, jedi.VAR_CRYSTAL_STATS + "." + jedi.VAR_MIN_DMG))
			{
				names[idx] = "wpn_comp_damage_min";
				int attrib = getIntObjVar(self, jedi.VAR_CRYSTAL_STATS + "."+ jedi.VAR_MIN_DMG);
				attribs[idx] = Integer.toString(attrib);
				idx++;
				if (idx >= names.length)
				{
					return super.OnGetAttributes(self, player, names, attribs);
				}
			}
			if (hasObjVar(self, jedi.VAR_CRYSTAL_STATS + "." + jedi.VAR_MAX_DMG))
			{
				names[idx] = "wpn_comp_damage_max";
				int attrib = getIntObjVar(self, jedi.VAR_CRYSTAL_STATS + "."+ jedi.VAR_MAX_DMG);
				attribs[idx] = Integer.toString(attrib);
				idx++;
				if (idx >= names.length)
				{
					return super.OnGetAttributes(self, player, names, attribs);
				}
			}
			
			names[idx] = "crystal_owner";
			attribs[idx] = getStringObjVar(self, jedi.VAR_CRYSTAL_OWNER_NAME);
			idx++;
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
		}
		else
		{
			names[idx] = "crystal_owner";
			attribs[idx] = "\\#pcontrast2 "+localize(new string_id("jedi_spam", "crystal_untuned"))+"\\#.";
			idx++;
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
			
			if (hasObjVar(self, jedi.VAR_CRYSTAL_STATS + "." + jedi.VAR_QUALITY))
			{
				int qualityLevel = getIntObjVar(self, jedi.VAR_CRYSTAL_STATS + "."+ jedi.VAR_QUALITY);
				names[idx] = "crystal_quality";
				int attrib = jedi.getStaticCrystalQuality(qualityLevel);
				attribs[idx] = utils.packStringId(new string_id("jedi_spam", "crystal_quality_"+attrib));
				idx++;
				if (idx >= names.length)
				{
					return super.OnGetAttributes(self, player, names, attribs);
				}
			}
		}
		
		if (hasObjVar(self, jedi.VAR_CRYSTAL_STATS + "." + jedi.VAR_COLOR))
		{
			names[idx] = "color";
			int attrib = getIntObjVar(self, jedi.VAR_CRYSTAL_STATS + "."+ jedi.VAR_COLOR);
			String strAsciiId = "saber_color_"+attrib;
			
			if (getIntObjVar(self, jedi.VAR_CRYSTAL_STATS + "." + jedi.VAR_SHADER) == 1)
			{
				strAsciiId = "saber_color_lava";
			}
			string_id strSpam = new string_id("jedi_spam", strAsciiId);
			String strPackedId = utils.packStringId(strSpam);
			attribs[idx] = strPackedId;
			idx++;
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
		}
		
		return super.OnGetAttributes(self, player, names, attribs);
	}
}
