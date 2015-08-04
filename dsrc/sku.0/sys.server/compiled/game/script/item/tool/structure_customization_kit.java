package script.item.tool;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.hue;
import script.library.player_structure;
import script.library.sui;
import script.library.static_item;
import script.library.utils;


public class structure_customization_kit extends script.base_script
{
	public structure_customization_kit()
	{
	}
	public static final boolean BLOGGING_ON = true;
	public static final String BLOGGING_CATEGORY = "structure_recolor";
	
	public static final int OBJECT_COLOR_MAX = 4;
	
	public static final String VAR_PREFIX = "structure_colorize";
	public static final String PID_NAME = VAR_PREFIX + ".pid";
	public static final String PLAYER_ID = VAR_PREFIX + ".player_oid";
	public static final String TOOL_ID = VAR_PREFIX + ".tool_oid";
	
	public static final String CUSTOMER_SVC_CATEGORY = "playerStructure";
	public static final string_id SID_MUST_BE_INSIDE_STRUCTURE = new string_id("tool/customizer", "inside_structure_that_you_own");
	public static final string_id SID_MUST_BE_INSIDE_INVENTORY = new string_id("tool/customizer", "must_be_in_inventory");
	public static final string_id SID_STRUCTURE_PAINTING_FAILED = new string_id("tool/customizer", "there_was_a_problem_with_structure_kit");
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		int idx = utils.getValidAttributeIndex(names);
		if (idx == -1)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		if (exists(self) && hasObjVar(self, "crafting_components.charges"))
		{
			names[idx] = "count";
			int attrib = getIntObjVar(self, "crafting_components.charges");
			attribs[idx] = Integer.toString(attrib);
			idx++;
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
		}
		
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		blog("OnObjectMenuRequest - functions");
		if (!isValidId(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.isNestedWithin(self, player))
		{
			sendSystemMessage(player, SID_MUST_BE_INSIDE_INVENTORY);
			return SCRIPT_CONTINUE;
		}
		
		menu_info_data mid = mi.getMenuItemByType (menu_info_types.ITEM_USE);
		if (mid != null)
		{
			mid.setServerNotify (true);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		blog("OnObjectMenuSelect - functions too");
		
		if (!isValidId(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.isNestedWithin(self, player))
		{
			sendSystemMessage(player, SID_MUST_BE_INSIDE_INVENTORY);
			return SCRIPT_CONTINUE;
		}
		
		if (item != menu_info_types.ITEM_USE)
		{
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog(CUSTOMER_SVC_CATEGORY, "A player "+ player + " is using crafted structure customization kit "+self+". [ OnObjectMenuSelect() ]");
		blog("OnObjectMenuSelect - player attempting to use kit not in inventory");
		if (!validateStructureOrCell(player))
		{
			
			sendSystemMessage(player, SID_MUST_BE_INSIDE_STRUCTURE);
			return SCRIPT_CONTINUE;
		}
		if (!beginStructureColorization(self, player))
		{
			sendSystemMessage(player, SID_STRUCTURE_PAINTING_FAILED);
			CustomerServiceLog(CUSTOMER_SVC_CATEGORY, "A player "+ player + " used the crafted structure customization kit "+self+" and it failed to color or find a building. [ OnObjectMenuSelect() ]");
			blog("OnObjectMenuSelect - player failed trying to use beginStructureColorization");
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean validateStructureOrCell(obj_id player) throws InterruptedException
	{
		blog("validateStructureOrCell - init");
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		
		blog("validateStructureOrCell - initial validation done");
		
		if (isInWorldCell(player))
		{
			return false;
		}
		
		obj_id house = getTopMostContainer(player);
		if (!isIdValid(house) || !player_structure.isBuilding(house))
		{
			return false;
		}
		
		if (!player_structure.isOwner(house, player))
		{
			return false;
		}
		
		blog("validateStructureOrCell - looks good, returning");
		
		return true;
	}
	
	
	public boolean beginStructureColorization(obj_id tool, obj_id player) throws InterruptedException
	{
		blog("beginStructureColorization - init");
		if (!isValidId(tool) || !exists(tool))
		{
			return false;
		}
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		
		blog("beginStructureColorization - validation completed");
		
		if (!utils.isNestedWithin(tool, player))
		{
			sendSystemMessage(player, SID_MUST_BE_INSIDE_INVENTORY);
			return false;
		}
		
		blog("beginStructureColorization - about to validate structure.");
		
		if (!validateStructureOrCell(player))
		{
			
			CustomerServiceLog(CUSTOMER_SVC_CATEGORY, "ERROR: Player somehow got outside or in some invalid building while using the structure customization kit: "+tool+". [ beginStructureColorization() ]");
			sendSystemMessage(player, SID_MUST_BE_INSIDE_STRUCTURE);
			return false;
		}
		
		closeOldWindow(player);
		blog("beginStructureColorization: validateStructureOrCell was ok");
		
		obj_id house = getTopMostContainer(player);
		if (!isIdValid(house) || !player_structure.isBuilding(house))
		{
			
			CustomerServiceLog(CUSTOMER_SVC_CATEGORY, "ERROR: Structure Customization Tool failed to find a valid structure to color. Kit: "+tool+". [ beginStructureColorization() ]");
			sendSystemMessage(player, SID_MUST_BE_INSIDE_STRUCTURE);
			blog("beginStructureColorization: not in house");
			return false;
		}
		
		String houseTemplate = getTemplateName(house);
		if (houseTemplate == null || houseTemplate.equals(""))
		{
			
			CustomerServiceLog(CUSTOMER_SVC_CATEGORY, "ERROR: Structure Customization Tool failed because kit could not get the structure template. Kit: "+tool+". [ beginStructureColorization() ]");
			blog("beginStructureColorization: no house template");
			return false;
		}
		
		blog("beginStructureColorization - getting house pallete");
		ranged_int_custom_var[] palColors = hue.getPalcolorVars(house);
		if ((palColors == null) || (palColors.length == 0))
		{
			return false;
		}
		
		blog("beginStructureColorization - palColors.length: "+palColors.length);
		
		int palColorsLength = palColors.length;
		blog("beginStructureColorization - palColors.length: "+palColorsLength);
		
		String[] indexName = new String[OBJECT_COLOR_MAX];
		
		blog("beginStructureColorization - indexName: "+indexName.length);
		
		int loop = OBJECT_COLOR_MAX;
		if (palColorsLength < OBJECT_COLOR_MAX)
		{
			for (int i = 0; i < OBJECT_COLOR_MAX; i++)
			{
				testAbortScript();
				indexName[i] = "";
			}
			
			loop = palColorsLength;
		}
		
		for (int i = 0; i < loop; i++)
		{
			testAbortScript();
			ranged_int_custom_var ri = palColors[i];
			if (ri != null)
			{
				blog("beginStructureColorization - indexName["+i+"]: "+indexName[i]);
				String customizationVar = ri.getVarName();
				if (customizationVar.startsWith("/"))
				{
					customizationVar = customizationVar.substring(1);
				}
				indexName[i] = customizationVar;
			}
		}
		if (indexName[0].equals(""))
		{
			CustomerServiceLog(CUSTOMER_SVC_CATEGORY, "Coloring Instantiating Color Customization UI on Object: "+ house +" for player "+player+ " has failed because the 1st color was null. Kit OID: "+tool+". [ beginStructureColorization() ]");
			return false;
		}
		CustomerServiceLog(CUSTOMER_SVC_CATEGORY, "Coloring Instantiating Color Customization UI on Object: "+ house +" for player "+player+ " using colorization using crafted armor customization kit "+tool+". [ beginStructureColorization() ]");
		blog("beginStructureColorization - Armor Piece getting color UI: "+ house);
		
		utils.setScriptVar(player, TOOL_ID, tool);
		utils.setScriptVar(tool, PLAYER_ID, player);
		openCustomizationWindow(player, house, indexName[0], -1, -1, indexName[1], -1, -1, indexName[2], -1, -1, indexName[3], -1, -1);
		
		return true;
	}
	
	
	public int decrementTool(obj_id self, dictionary params) throws InterruptedException
	{
		blog("decrementTool - init");
		
		obj_id player = utils.getObjIdScriptVar(self, PLAYER_ID);
		if (!isValidId(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("decrementTool - validation completed");
		
		static_item.decrementStaticItem(self);
		int count = getIntObjVar(self, "crafting_components.charges");
		
		if (!exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		count--;
		if (count > 0)
		{
			setObjVar(self, "crafting_components.charges", count);
		}
		else
		{
			destroyObject(self);
		}
		
		removePlayerVars(player);
		return SCRIPT_CONTINUE;
	}
	
	
	public int cancelTool(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = utils.getObjIdScriptVar(self, PLAYER_ID);
		if (!isValidId(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		removePlayerVars(player);
		return SCRIPT_CONTINUE;
	}
	
	
	public void closeOldWindow(obj_id player) throws InterruptedException
	{
		blog("closeOldWindow - init");
		
		int pid = sui.getPid(player, PID_NAME);
		blog("closeOldWindow - pid: "+pid);
		if (pid > -1)
		{
			blog("closeOldWindow - force closing: "+pid);
			forceCloseSUIPage(pid);
			sui.removePid(player, PID_NAME);
		}
	}
	
	
	public void removePlayerVars(obj_id player) throws InterruptedException
	{
		obj_id self = getSelf();
		if (!isValidId(self) || !exists(self) || !isValidId(player) || !exists(player))
		{
			return;
		}
		
		utils.removeScriptVarTree(player, VAR_PREFIX);
		utils.removeScriptVarTree(self, VAR_PREFIX);
		utils.removeObjVar(player, VAR_PREFIX);
	}
	
	
	public boolean blog(String msg) throws InterruptedException
	{
		if (!BLOGGING_ON)
		{
			return false;
		}
		else if (msg == null || msg.equals(""))
		{
			return false;
		}
		
		LOG(BLOGGING_CATEGORY, msg);
		
		return true;
	}
}
