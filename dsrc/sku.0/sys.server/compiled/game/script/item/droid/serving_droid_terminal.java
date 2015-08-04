package script.item.droid;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.player_structure;
import script.library.utils;


public class serving_droid_terminal extends script.base_script
{
	public serving_droid_terminal()
	{
	}
	public static final String DROIDTEMPLATE = "object/creature/npc/droid/serving_droid.iff";
	
	public static final String MENU_FILE = "pet/pet_menu";
	public static final string_id TERMINAL_NAME = new string_id(MENU_FILE,"serving_droid_terminal_custom_name");
	public static final string_id ASTROMECH_NAME = new string_id(MENU_FILE,"serving_droid_custom_name");
	public static final string_id ASTROMECH_BIOLINK_NEEDED = new string_id(MENU_FILE,"serving_droid_biolink_needed");
	public static final string_id ASTROMECH_NOT_INVENTORY = new string_id(MENU_FILE,"serving_droid_not_inventory");
	public static final string_id ASTROMECH_NOT_OWNER = new string_id(MENU_FILE,"serving_droid_not_owner");
	public static final string_id ASTROMECH_NOT_IN_HOUSE = new string_id(MENU_FILE,"serving_droid_not_in_house");
	
	public static final string_id SUMMON = new string_id("sui", "summon_droid");
	public static final string_id DESTROY = new string_id("sui", "destroy_droid");
	
	public static final String OBJVAR_DROID_PATROL_POINTS = "droid.patrolPoints";
	public static final String OBJVAR_DROID_PATROL_LOOP = "droid.patrol_loop";
	public static final String OBJVAR_DROID_PATROL_ONCE = "droid.patrol_once";
	public static final String DROID_ID = "droid.id";
	public static final String DROID_NAMED = "droid.named";
	public static final String PLAYER = "droid.user";
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		obj_id terminal = self;
		obj_id droid = getObjIdObjVar(terminal, DROID_ID);
		
		if (!isValidId(droid))
		{
			removeObjVar(terminal, DROID_ID);
		}
		else if (!exists(droid))
		{
			boolean created = recreateDroid(terminal);
			if (!created)
			{
				removeObjVar(terminal, DROID_ID);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		obj_id terminal = self;
		obj_id droid = getObjIdObjVar(terminal, DROID_ID);
		if (!isValidId(droid))
		{
			return SCRIPT_CONTINUE;
		}
		
		destroyObject(droid);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAboutToBeTransferred(obj_id self, obj_id destContainer, obj_id transferer) throws InterruptedException
	{
		obj_id terminal = self;
		if (!isValidId(terminal))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(terminal, DROID_ID) && exists(terminal))
		{
			messageTo(terminal, "destroyDroid", null, 1, false);
		}
		
		if (hasObjVar(terminal, OBJVAR_DROID_PATROL_POINTS) && exists(terminal))
		{
			removeObjVar(terminal, OBJVAR_DROID_PATROL_POINTS);
		}
		if (hasObjVar(terminal, OBJVAR_DROID_PATROL_LOOP) && exists(terminal))
		{
			removeObjVar(terminal, OBJVAR_DROID_PATROL_LOOP);
		}
		if (hasObjVar(terminal, OBJVAR_DROID_PATROL_ONCE) && exists(terminal))
		{
			removeObjVar(terminal, OBJVAR_DROID_PATROL_ONCE);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		obj_id terminal = self;
		if (!exists(terminal))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.isInVendor(terminal) || utils.isInBazaar(terminal))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id ownerContainer = getTopMostContainer(terminal);
		if (!isValidId(ownerContainer) || !exists(ownerContainer))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.isBuilding(ownerContainer) && !isPlayer(ownerContainer))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(terminal, DROID_ID))
		{
			mi.addRootMenu(menu_info_types.ITEM_USE, DESTROY);
		}
		else
		{
			mi.addRootMenu(menu_info_types.ITEM_USE, SUMMON);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		obj_id terminal = self;
		obj_id biolink = getBioLink(terminal);
		
		if (!isValidId(terminal))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isValidId(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		sendDirtyObjectMenuNotification(terminal);
		
		if (item != menu_info_types.ITEM_USE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isValidId(biolink) || biolink == utils.OBJ_ID_BIO_LINK_PENDING)
		{
			sendSystemMessage(player, ASTROMECH_BIOLINK_NEEDED);
			return SCRIPT_CONTINUE;
		}
		
		if (utils.isNestedWithinAPlayer(terminal))
		{
			sendSystemMessage(player, ASTROMECH_NOT_INVENTORY);
			return SCRIPT_CONTINUE;
		}
		
		if (player != getObjIdObjVar(terminal, "biolink.id"))
		{
			sendSystemMessage(player, ASTROMECH_NOT_OWNER);
			return SCRIPT_CONTINUE;
		}
		if (!utils.isInHouseCellSpace(terminal))
		{
			sendSystemMessage(player, ASTROMECH_NOT_IN_HOUSE);
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(terminal, DROID_ID))
		{
			utils.setScriptVar(terminal, "terminal.summon", true);
			boolean droidCreated = createDroid(terminal, player);
			if (!droidCreated)
			{
				removeObjVar(terminal, DROID_ID);
			}
		}
		else
		{
			messageTo(terminal, "destroyDroid", null, 1, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int destroyDroid(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id terminal = self;
		if (!isValidId(terminal))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = getObjIdObjVar(terminal, "biolink.id");
		if (!isValidId(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id droid = getObjIdObjVar(terminal, DROID_ID);
		if (isValidId(droid) && exists(droid))
		{
			obj_id house = getObjIdObjVar(droid, "house");
			if (!isValidId(house) || !exists(house))
			{
				return SCRIPT_CONTINUE;
			}
			
			removeObjVar(house, "serving_droid.droid");
			if (hasScript(house, "structure.serving_droid_control"))
			{
				detachScript(house, "structure.serving_droid_control");
			}
			
			destroyObject(droid);
		}
		removeObjVar(terminal, DROID_ID);
		sendConsoleMessage(player, "Droid retired.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean nameTerminal(obj_id terminal) throws InterruptedException
	{
		if (!isValidId(terminal))
		{
			return false;
		}
		obj_id user = getObjIdObjVar(terminal, "biolink.id");
		String text = getName(user) + localize(TERMINAL_NAME);
		setName(terminal, text);
		setObjVar(terminal, DROID_NAMED, true);
		return true;
	}
	
	
	public boolean setDroidVars(obj_id terminal, obj_id droid) throws InterruptedException
	{
		if (!isValidId(terminal) || !exists(terminal))
		{
			return false;
		}
		
		if (!isValidId(droid) || !exists(droid))
		{
			return false;
		}
		
		obj_id user = getObjIdObjVar(terminal, "biolink.id");
		setInvulnerable(droid, true);
		
		setObjVar(terminal, DROID_ID, droid);
		setObjVar(droid, "terminal", terminal);
		setObjVar(droid, PLAYER, user);
		messageTo(droid, "reColorDroid", null, 0, false);
		setName(droid, getName(user) + localize(ASTROMECH_NAME));
		return true;
	}
	
	
	public boolean createDroid(obj_id terminal, obj_id player) throws InterruptedException
	{
		
		if (!isValidId(terminal) || !exists(terminal))
		{
			return false;
		}
		
		obj_id anotherDroid = getObjIdObjVar(terminal, DROID_ID);
		
		if (isValidId(anotherDroid) && exists(anotherDroid))
		{
			return false;
		}
		
		obj_id house = getTopMostContainer(terminal);
		if (!isIdValid(house) && !exists(house))
		{
			return false;
		}
		
		if (!player_structure.isBuilding(house))
		{
			return false;
		}
		
		if (!hasScript(house, "structure.serving_droid_control"))
		{
			attachScript(house, "structure.serving_droid_control");
			listenToMessage(house, "handleActivateDroid");
		}
		
		location startLocation = new location();
		location[] patrolLoc = getLocationArrayObjVar(terminal, OBJVAR_DROID_PATROL_POINTS);
		if (patrolLoc == null)
		{
			
			location playerLoc = getLocation(player);
			if (!isValidId(playerLoc.cell))
			{
				startLocation = getLocation(terminal);
			}
			else
			{
				startLocation = getLocation(player);
			}
		}
		else
		{
			
			boolean validList = validLocationList(terminal, patrolLoc);
			if (!validList)
			{
				return false;
			}
			
			startLocation = patrolLoc[0];
		}
		
		obj_id droid = createObject(DROIDTEMPLATE, startLocation);
		if (patrolLoc != null)
		{
			messageTo(droid, "handleActivateDroid", null, 1, false);
		}
		
		if (!isIdValid(droid) && !exists(droid))
		{
			return false;
		}
		
		setDroidVars(terminal, droid);
		
		if (utils.hasScriptVar(terminal, "terminal.summon"))
		{
			sendConsoleMessage(player, "Droid summoned.");
			utils.removeScriptVar(terminal, "terminal.summon");
		}
		if (!hasObjVar(terminal, DROID_NAMED))
		{
			nameTerminal(terminal);
		}
		
		return true;
	}
	
	
	public boolean recreateDroid(obj_id terminal) throws InterruptedException
	{
		obj_id anotherDroid = getObjIdObjVar(terminal, DROID_ID);
		
		if (isValidId(anotherDroid) && exists(anotherDroid))
		{
			return false;
		}
		
		obj_id house = getTopMostContainer(terminal);
		if (!isIdValid(house) && !exists(house))
		{
			return false;
		}
		
		if (!player_structure.isBuilding(house))
		{
			return false;
		}
		
		if (!hasScript(house, "structure.serving_droid_control"))
		{
			attachScript(house, "structure.serving_droid_control");
			listenToMessage(house, "handleActivateDroid");
		}
		
		location startLocation = new location();
		location[] patrolLoc = getLocationArrayObjVar(terminal, OBJVAR_DROID_PATROL_POINTS);
		if (patrolLoc == null)
		{
			return false;
		}
		
		boolean validList = validLocationList(terminal, patrolLoc);
		if (!validList)
		{
			return false;
		}
		
		startLocation = patrolLoc[0];
		
		obj_id droid = createObject(DROIDTEMPLATE, startLocation);
		messageTo(droid, "handleActivateDroid", null, 1, false);
		
		if (!isIdValid(droid) && !exists(droid))
		{
			return false;
		}
		
		setDroidVars(terminal, droid);
		
		if (utils.hasScriptVar(terminal, "terminal.summon"))
		{
			utils.removeScriptVar(terminal, "terminal.summon");
		}
		if (!hasObjVar(terminal, DROID_NAMED))
		{
			nameTerminal(terminal);
		}
		
		return true;
	}
	
	
	public boolean validLocationList(obj_id terminal, location[] patrolLocations) throws InterruptedException
	{
		if (patrolLocations == null)
		{
			return false;
		}
		
		for (int i = 0; i < patrolLocations.length; ++i)
		{
			testAbortScript();
			if (!isValidId(patrolLocations[i].cell))
			{
				removeObjVar(terminal, OBJVAR_DROID_PATROL_POINTS);
				return false;
			}
		}
		return true;
	}
}
