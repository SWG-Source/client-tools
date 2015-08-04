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
import script.library.battlefield;
import script.library.locations;
import script.library.pclib;
import script.library.xp;
import script.library.player_structure;
import script.library.structure;


public class camping extends script.base_script
{
	public camping()
	{
	}
	public static final float CAMP_SIZE[] =
	{
		6.f, 8.f, 10.f, 13.f, 15.f, 20.f
	};
	public static final float MAX_SIZE = 14.f;
	
	public static final int NUM_MARKER = 6;
	
	public static final float HEARTBEAT_CREATION = 1.0f;
	public static final float HEARTBEAT_MAINTAIN = 30.0f;
	public static final float HEARTBEAT_RESTORE = 120.0f;
	
	public static final String MOD_CAMP = "camp";
	public static final String MOD_CAMP_ADVANCED = "camp_advanced";
	
	public static final String VAR_CAMP_BASE = "camp";
	
	public static final String VAR_OWNER = VAR_CAMP_BASE + ".owner";
	public static final String VAR_OWNER_NEAR = VAR_CAMP_BASE + ".owner_near";
	
	public static final String VAR_STATUS = VAR_CAMP_BASE + ".status";
	
	public static final String VAR_SIZE = VAR_CAMP_BASE + ".size";
	public static final String VAR_LIFETIME = VAR_CAMP_BASE + ".lifetime";
	public static final String VAR_CREATION_TIME = VAR_CAMP_BASE + ".creation_time";
	public static final String VAR_DESTRUCT_TIME = VAR_CAMP_BASE + ".destruction_time";
	
	public static final String VAR_BEEN_INITIALIZED = VAR_CAMP_BASE + ".beenInitialized";
	
	public static final String VAR_CAMP_XP = VAR_CAMP_BASE + ".xp";
	public static final String VAR_CAMP_REPEL = VAR_CAMP_BASE + ".repel";
	public static final String VAR_CAMP_POWER = VAR_CAMP_BASE + ".power";
	
	public static final String VAR_TRIGGER_VOLUME = VAR_CAMP_BASE + ".triggerVolume";
	
	public static final String VAR_CAMP_ITEM_BASE = VAR_CAMP_BASE + ".items";
	
	public static final String VAR_ITEM_IDX = VAR_CAMP_BASE + ".item_idx";
	public static final String VAR_ITEM_MAX = VAR_CAMP_BASE + ".item_max";
	
	public static final String VAR_MARKERS = VAR_CAMP_ITEM_BASE + ".markers";
	public static final String VAR_GENERAL_ITEMS = VAR_CAMP_ITEM_BASE + ".general";
	public static final String VAR_ADVANCED_ITEMS = VAR_CAMP_ITEM_BASE + ".advanced";
	
	public static final String VAR_PLAYER_CAMP = VAR_CAMP_BASE + ".id";
	
	public static final String VAR_CURRENT_CAMP = VAR_CAMP_BASE + ".current";
	
	public static final int STATUS_UNKNOWN = -1;
	public static final int STATUS_NEW = 0;
	public static final int STATUS_CREATION = 1;
	public static final int STATUS_MAINTAIN = 2;
	public static final int STATUS_ABANDONED = 3;
	
	public static final String SCRIPT_MASTER_OBJECT = "systems.camping.camp_master";
	
	public static final String COL_MASTER = "MASTER";
	public static final String COL_MARKER = "MARKER";
	public static final String COL_OTHER = "OTHER";
	
	public static final String COL_TEMPLATE = "TEMPLATE";
	public static final String COL_MIN_MOD = "MIN_MOD";
	
	public static final String HANDLER_SET_STATUS = "handleSetStatus";
	
	public static final String HANDLER_CAMP_CREATE = "handleCampCreationHeartbeat";
	public static final String HANDLER_CAMP_MAINTAIN = "handleCampMaintenanceHeartbeat";
	public static final String HANDLER_CAMP_RESTORE = "handleCampRestoreHeartbeat";
	
	public static final String HANDLER_CAMP_COMPLETE = "handleCampComplete";
	
	public static final String DICT_NEW_STATUS = "newStatus";
	
	public static final string_id SID_ABANDONED_CAMP = new string_id("camp", "abandoned_camp");
	public static final string_id SID_STARTING_CAMP = new string_id("camp", "starting_camp");
	public static final string_id SID_CAMP_COMPLETE = new string_id("camp", "camp_complete");
	public static final string_id SID_ASSUMING_OWNERSHIP = new string_id("camp", "assuming_ownership");
	
	public static final string_id SID_ERROR_CAMP_DISBAND = new string_id("camp", "error_camp_disband");
	
	public static final string_id SID_ERROR_DEFAULT = new string_id("camp", "error_default");
	public static final string_id SID_ERROR_INSIDE = new string_id("camp", "error_inside");
	public static final string_id SID_ERROR_CAMP_EXISTS = new string_id("camp", "error_camp_exists");
	public static final string_id SID_ERROR_CAMP_TOO_CLOSE = new string_id("camp", "error_camp_too_close");
	public static final string_id SID_ERROR_BUILDING_TOO_CLOSE = new string_id("camp", "error_building_too_close");
	public static final string_id SID_ERROR_LAIR_TOO_CLOSE = new string_id("camp", "error_lair_too_close");
	public static final string_id SID_ERROR_MUNI_TRUE = new string_id("camp", "error_muni_true");
	public static final string_id SID_ERROR_NOBUILD = new string_id("camp", "error_nobuild");
	public static final string_id SID_ERROR_BATTLEFIELD = new string_id("camp", "error_battlefield");
	public static final string_id SID_ERROR_IN_WATER = new string_id("camp", "error_in_water");
	public static final string_id SID_ERROR_TOO_CLOSE_TO_WATER = new string_id("camp", "error_too_close_to_water");
	
	public static final string_id SID_ERROR_CAMP_TOO_BIG = new string_id("camp", "error_too_big");
	public static final string_id SID_CAMP_MASTER_NAME = new string_id("camp", "camp_master");
	
	public static final string_id SID_GENERAL_ITEM_ADDED = new string_id("camp", "general_item_added");
	public static final string_id SID_ADVANCED_ITEMS_ADDED = new string_id("camp", "advanced_items_added");
	
	public static final string_id SID_CAMP_ENTER = new string_id("camp", "camp_enter");
	public static final string_id SID_CAMP_EXIT = new string_id("camp", "camp_exit");
	
	public static final string_id PROSE_CAMP_ENTER = new string_id("camp", "prose_camp_enter");
	public static final string_id PROSE_CAMP_EXIT = new string_id("camp", "prose_camp_exit");
	public static final string_id SID_RECRUITER_MODULE_ABORTED = new string_id("camp", "recruiter_aborted");
	
	public static final String CAMP_THEATERS[] =
	{
		"object/building/poi/scout_camp_s0.iff",
		"object/building/poi/scout_camp_s1.iff",
		"object/building/poi/scout_camp_s2.iff",
		"object/building/poi/scout_camp_s3.iff",
		"object/building/poi/scout_camp_s4.iff",
		"object/building/poi/scout_camp_s5.iff"
	};
	
	public static final String CAMP_DATATABLE = "datatables/item/camp.iff";
	
	
	public static obj_id createCamp(obj_id creator) throws InterruptedException
	{
		return createCamp( creator, 1 );
	}
	
	
	public static obj_id createCamp(obj_id creator, int campPower) throws InterruptedException
	{
		if (!isIdValid(creator))
		{
			return null;
		}
		
		location here = utils.findLocInFrontOfTarget(creator, 1.0f);
		if (here == null)
		{
			sendSystemMessage(creator, new string_id("camp", "bad_location"));
			return null;
		}
		
		if (campPower == 0)
		{
			campPower = 1;
		}
		
		if (canPlaceCampHere( creator, here, campPower ))
		{
			obj_id master = createCampTheater( here, campPower );
			if (!isIdValid(master))
			{
				sendSystemMessage(creator, new string_id("camp", "bad_creation"));
				return null;
			}
			else
			{
				setCampOwner( master, creator );
				
				setCampMasterName(master);
				
				if (campPower > 4 && factions.isDeclared(creator))
				{
					String creatorFac = factions.getFaction(creator);
					if (creatorFac != null)
					{
						factions.setFaction(master, creatorFac);
						attachScript(master, "planet_map.map_loc_attach");
					}
				}
				
				int mod_camp = getSkillStatMod( creator, MOD_CAMP );
				int repel = campPower;
				if (campPower == 1)
				{
					repel = 0;
				}
				setObjVar(master, VAR_CAMP_REPEL, repel);
				setObjVar(master, VAR_CAMP_POWER, campPower);
				attachScript( master, SCRIPT_MASTER_OBJECT );
				setStatus( master, STATUS_NEW );
				setObjVar(master, VAR_ITEM_IDX, 0);
				int maxItem = (int)(mod_camp / 20) + 2;
				setObjVar(master, VAR_ITEM_MAX, maxItem);
				sendCampCreationHeartbeat( master );
				
				int createTime = getGameTime();
				setObjVar( master, "camp.createTime", createTime );
				
				return master;
			}
		}
		else
		{
			LOG("camping", "Cannot place camp here...");
		}
		
		return null;
	}
	
	
	public static obj_id createCampTheater(location here, int campPower) throws InterruptedException
	{
		if (here == null)
		{
			return null;
		}
		return createObject( CAMP_THEATERS[campPower-1], here );
	}
	
	
	public static boolean canPlaceCampHere(obj_id creator, location here, int campPower) throws InterruptedException
	{
		if (!isIdValid(creator) || (here == null))
		{
			return false;
		}
		
		if (isIdValid(here.cell) || here.area.startsWith("space_"))
		{
			sendSystemMessage(creator, SID_ERROR_INSIDE);
			return false;
		}
		
		obj_id currentCamp = getCurrentCamp(creator);
		if (currentCamp != null)
		{
			
			sendSystemMessage(creator, SID_ERROR_CAMP_EXISTS);
			
			return false;
		}
		
		if (isTooCloseToAnotherCamp(here,creator))
		{
			sendSystemMessage(creator, new string_id("camp", "camp_too_close"));
			return false;
		}
		
		if (here.area.equals("mustafar"))
		{
			if (here.x > 1120 || here.z < -1024)
			{
				return false;
			}
		}
		
		if (instance.isInInstanceArea(creator))
		{
			return false;
		}
		
		region bf = battlefield.getBattlefield(here);
		if (bf != null)
		{
			sendSystemMessage(creator, SID_ERROR_BATTLEFIELD);
			return false;
		}
		
		if (locations.isInMissionCity( here ))
		{
			sendSystemMessage(creator, SID_ERROR_MUNI_TRUE);
			return false;
		}
		
		if (isBelowWater( here ))
		{
			sendSystemMessage(creator, SID_ERROR_IN_WATER);
			return false;
		}
		
		float size = CAMP_SIZE[campPower] + 1.0f;
		if (isWaterInRect (here.x - size, here.z - size, here.x + size, here.z + size))
		{
			sendSystemMessage(creator, SID_ERROR_TOO_CLOSE_TO_WATER);
			return false;
		}
		
		return true;
	}
	
	
	public static boolean nukeCamp(obj_id master) throws InterruptedException
	{
		if (!isIdValid(master) || !exists( master ))
		{
			return false;
		}
		
		removeTriggerVolume("camp_"+ master);
		
		obj_id[] players = getPlayerCreaturesInRange(getLocation(master), getCampSize(master));
		if ((players == null) || (players.length == 0))
		{
			
		}
		else
		{
			for (int i = 0; i < players.length; i++)
			{
				testAbortScript();
				clearCurrentCamp(players[i]);
			}
		}
		
		int xpGranted = getIntObjVar(master, VAR_CAMP_XP);
		obj_id owner = getObjIdObjVar(master, VAR_OWNER);
		if ((owner != null) && (owner != obj_id.NULL_ID))
		{
			pclib.msgRemoveObjVar(owner, VAR_CAMP_BASE);
			if ((owner.isLoaded()) && (isInWorld(owner)))
			{
				
				sendSystemMessage(owner, SID_ERROR_CAMP_DISBAND);
			}
		}
		
		if (!destroyObject(master))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static obj_id getCurrentCamp(obj_id target) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return null;
		}
		
		deltadictionary dd = target.getScriptVars();
		obj_id camp = dd.getObjId(VAR_CURRENT_CAMP);
		
		if (!isIdValid( camp ) || !exists( camp ) || (getStatus( camp ) == STATUS_ABANDONED))
		{
			
			clearCurrentCamp( target );
			return null;
		}
		
		return camp;
	}
	
	
	public static boolean setCurrentCamp(obj_id target, obj_id master) throws InterruptedException
	{
		if (!isIdValid(target) || !isIdValid(master))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.put(VAR_CURRENT_CAMP, master);
		return true;
	}
	
	
	public static boolean clearCurrentCamp(obj_id target) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		
		deltadictionary dd = target.getScriptVars();
		dd.remove(VAR_CURRENT_CAMP);
		return true;
	}
	
	
	public static float getCampSize(obj_id master) throws InterruptedException
	{
		int campPower = getIntObjVar( master, VAR_CAMP_POWER ) - 1;
		if (campPower < 0)
		{
			campPower = 0;
		}
		return CAMP_SIZE[campPower];
	}
	
	
	public static boolean isTooCloseToAnotherCamp(location here, obj_id creator) throws InterruptedException
	{
		if (here == null || !isIdValid(creator))
		{
			return true;
		}
		
		float range = 2 * MAX_SIZE + 1;
		obj_id[] objs = getNonCreaturesInRange(here, range);
		if ((objs == null) || (objs.length == 0))
		{
			return false;
		}
		else
		{
			for (int i = 0; i < objs.length; i++)
			{
				testAbortScript();
				obj_id item = objs[i];
				
				if (hasObjVar(item, VAR_OWNER))
				{
					sendSystemMessage(creator, SID_ERROR_CAMP_TOO_CLOSE);
					return true;
				}
				
				if (player_structure.isBuilding( item ))
				{
					sendSystemMessage(creator, SID_ERROR_BUILDING_TOO_CLOSE);
					return true;
				}
			}
		}
		
		return false;
	}
	
	
	public static obj_id getCampOwner(obj_id master) throws InterruptedException
	{
		if (!isIdValid(master))
		{
			return null;
		}
		
		return getObjIdObjVar(master, VAR_OWNER);
	}
	
	
	public static boolean setCampOwner(obj_id master, obj_id newOwner) throws InterruptedException
	{
		if (!isIdValid(master) || !isIdValid(newOwner))
		{
			return false;
		}
		
		setObjVar(master, VAR_OWNER, newOwner);
		setObjVar(newOwner, VAR_PLAYER_CAMP, master);
		return true;
	}
	
	
	public static boolean clearCampOwner(obj_id master) throws InterruptedException
	{
		if (!isIdValid(master))
		{
			return false;
		}
		
		obj_id owner = getCampOwner(master);
		if (isIdValid(owner))
		{
			pclib.msgRemoveObjVar(owner, VAR_PLAYER_CAMP);
		}
		
		setObjVar(master, VAR_OWNER, obj_id.NULL_ID);
		return true;
	}
	
	
	public static boolean updateCampOwner(obj_id master, obj_id newOwner) throws InterruptedException
	{
		setCampOwner(master, newOwner);
		
		return setStatus(master, STATUS_MAINTAIN);
	}
	
	
	public static boolean campAbandoned(obj_id master) throws InterruptedException
	{
		
		obj_id owner = getCampOwner( master );
		if (isIdValid( owner ))
		{
			
			setObjVar(master, camping.VAR_CAMP_XP, 0);
		}
		
		setName (master, SID_ABANDONED_CAMP);
		
		obj_id[] children = getObjIdArrayObjVar (master, "theater.children");
		if (children == null)
		{
			return true;
		}
		
		int numKids = children.length;
		
		for (int x = 0; x < numKids; x ++)
		{
			testAbortScript();
			obj_id thisChild = children[x];
			if (hasScript (thisChild, "systems.camping.camp_controlpanel"))
			{
				setName (thisChild, SID_ABANDONED_CAMP);
			}
		}
		clearCampOwner(master);
		
		messageTo(master, "handleNuke", null, 300f, false);
		
		return setStatus(master, STATUS_ABANDONED);
	}
	
	
	public static int getStatus(obj_id master) throws InterruptedException
	{
		if (!isIdValid(master))
		{
			return STATUS_UNKNOWN;
		}
		
		return getIntObjVar(master, VAR_STATUS);
	}
	
	
	public static boolean setStatus(obj_id master, int newStatus) throws InterruptedException
	{
		if (!isIdValid(master))
		{
			return false;
		}
		
		dictionary d = new dictionary();
		d.put(DICT_NEW_STATUS, newStatus);
		messageTo(master, HANDLER_SET_STATUS, d, 0, false);
		
		return setObjVar(master, VAR_STATUS, newStatus);
	}
	
	
	public static boolean isOwnerInVicinity(obj_id master) throws InterruptedException
	{
		return getBooleanObjVar( master, VAR_OWNER_NEAR );
	}
	
	
	public static void sendCampCreationHeartbeat(obj_id master) throws InterruptedException
	{
		messageTo(master, HANDLER_CAMP_CREATE, null, HEARTBEAT_CREATION, false);
	}
	
	
	public static void sendCampCreationComplete(obj_id master) throws InterruptedException
	{
		messageTo(master, HANDLER_CAMP_COMPLETE, null, HEARTBEAT_CREATION, false);
	}
	
	
	public static void sendCampMaintenanceHeartbeat(obj_id master) throws InterruptedException
	{
		messageTo(master, HANDLER_CAMP_MAINTAIN, null, HEARTBEAT_MAINTAIN, false);
	}
	
	
	public static void sendCampRestoreHeartbeat(obj_id master) throws InterruptedException
	{
		messageTo(master, HANDLER_CAMP_RESTORE, null, HEARTBEAT_RESTORE, false);
	}
	
	
	public static int campingSkillCheck(obj_id owner) throws InterruptedException
	{
		int roll = rand(0, 100);
		
		int chanceSuccess = getSkillStatMod(owner, MOD_CAMP);
		int delta = chanceSuccess - roll;
		
		return delta;
	}
	
	
	public static void setCampMasterName(obj_id master) throws InterruptedException
	{
		if (isIdValid(master))
		{
			setName( master, "");
			setName(master, SID_CAMP_MASTER_NAME);
		}
	}
	
	
	public static int getCampRepelValue(obj_id target) throws InterruptedException
	{
		if (isIdValid(target))
		{
			return -1;
		}
		
		obj_id master = getCurrentCamp(target);
		if (!isIdValid(master))
		{
			return 0;
		}
		else
		{
			int val = getIntObjVar(master, VAR_CAMP_REPEL);
			return val;
		}
	}
	
	
	public static boolean repelledByCamp(obj_id npc, obj_id player) throws InterruptedException
	{
		
		if (!isIdValid(npc))
		{
			return false;
		}
		if (!isIdValid(player))
		{
			return false;
		}
		
		obj_id camp = getCurrentCamp( player );
		if (!isIdValid(camp))
		{
			return false;
		}
		
		int repel = getIntObjVar( camp, VAR_CAMP_REPEL );
		int maxlevel = (repel-2)*15 + 20;
		if (ai_lib.getLevel( npc ) <= maxlevel)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static float getCampHealModifier(obj_id master) throws InterruptedException
	{
		int campPower = getIntObjVar( master, VAR_CAMP_POWER );
		
		switch ( campPower )
		{
			case 1:
			return 0.65f;
			case 2:
			return 0.65f;
			case 3:
			return 0.75f;
			case 4:
			return 0.85f;
			case 5:
			return 0.95f;
			case 6:
			return 1.0f;
			default:
			return 0.65f;
		}
	}
	
	
	public static boolean isRecruiterModule(String module_template) throws InterruptedException
	{
		if (( (module_template.equals("object/building/poi/player_camp_imperial_recruiter.iff")) || (module_template.equals("object/building/poi/player_camp_rebel_recruiter.iff")) ))
		{
			return true;
		}
		return false;
	}
	
	
	public static boolean validateRecruiter(obj_id player, String module_template, int recruitersPlaced) throws InterruptedException
	{
		if (!isRecruiterModule(module_template))
		{
			return true;
		}
		
		String playerFaction = factions.getFaction(player);
		if (playerFaction == null || playerFaction.equals(""))
		{
			if (recruitersPlaced == 0)
			{
				return true;
			}
			return false;
			
		}
		
		if (playerFaction.equals(factions.FACTION_IMPERIAL))
		{
			if (module_template.equals("object/building/poi/player_camp_imperial_recruiter.iff"))
			{
				if (recruitersPlaced == 0)
				{
					return true;
				}
			}
		}
		
		if (playerFaction.equals(factions.FACTION_REBEL))
		{
			if (module_template.equals("object/building/poi/player_camp_rebel_recruiter.iff"))
			{
				if (recruitersPlaced == 0)
				{
					return true;
				}
			}
		}
		
		return false;
	}
	
	
	public static void initializeAdvancedCamp(obj_id deed, obj_id camp, obj_id player, location loc) throws InterruptedException
	{
		setObjVar(camp, VAR_OWNER, player);
		attachScript(camp, "item.camp.camp_advanced");
		
		String name = getFirstName(player);
		name += "'s Camp";
		
		setName(camp, name);
		
		int numNodes = getAdvancedCampNumberOfNodes(camp);
		if (numNodes > 6)
		{
			numNodes = 6;
		}
		
		obj_id[] nodeObjects = new obj_id[numNodes];
		int arrayIdx = 0;
		
		obj_var_list ovl = getObjVarList(deed, "modules");
		
		float lifetime = 0f;
		lifetime += ovl.getFloatObjVar("lifetime");
		
		int recruitersPlaced = 0;
		if (ovl != null)
		{
			String[] modules = ovl.getAllObjVarNames();
			
			if (modules != null && modules.length > 0)
			{
				int nodeCount = 1;
				int i = 0;
				
				while (nodeCount <= numNodes && i < modules.length)
				{
					testAbortScript();
					if (modules[i] != null && !modules[i].equals(""))
					{
						if (ovl.getFloatObjVar(modules[i]) != 0.0f)
						{
							location tempLoc = (location)loc.clone();
							location nodeLoc = getAdvancedCampNodeLocation(camp, tempLoc, nodeCount);
							int nodeYaw = getAdvancedCampNodeYaw(camp, nodeCount);
							
							String module_template = "";
							
							if (modules[i].equals("shuttle_beacon"))
							{
								module_template = "object/building/poi/player_camp_shuttle_beacon.iff";
							}
							else if (modules[i].equals("cloning_tube"))
							{
								module_template = "object/building/poi/player_camp_clone_tube.iff";
							}
							else if (modules[i].equals("entertainer"))
							{
								module_template = "object/building/poi/player_camp_entertainment_area.iff";
							}
							else if (modules[i].equals("junk_dealer"))
							{
								module_template = "object/building/poi/player_camp_junk_dealer.iff";
							}
							else if (modules[i].equals("clothing_station"))
							{
								module_template = "object/building/poi/player_camp_crafting_clothing.iff";
							}
							else if (modules[i].equals("food_station"))
							{
								module_template = "object/building/poi/player_camp_crafting_food.iff";
							}
							else if (modules[i].equals("ship_station"))
							{
								module_template = "object/building/poi/player_camp_crafting_space.iff";
							}
							else if (modules[i].equals("structure_station"))
							{
								module_template = "object/building/poi/player_camp_crafting_structure.iff";
							}
							else if (modules[i].equals("weapon_station"))
							{
								module_template = "object/building/poi/player_camp_crafting_weapon.iff";
							}
							else if (modules[i].equals("imperial"))
							{
								module_template = "object/building/poi/player_camp_imperial_recruiter.iff";
							}
							else if (modules[i].equals("rebel"))
							{
								module_template = "object/building/poi/player_camp_rebel_recruiter.iff";
							}
							else if (modules[i].equals("extra_life"))
							{
								module_template = "object/static/structure/corellia/corl_power_transformer_s01.iff";
								lifetime += ovl.getFloatObjVar("extra_life");
								
							}
							
							if (!module_template.equals(""))
							{
								if (validateRecruiter(player, module_template, recruitersPlaced))
								{
									obj_id module_id = create.object(module_template, nodeLoc);
									if (isRecruiterModule(module_template))
									{
										recruitersPlaced = recruitersPlaced +1;
									}
									if (isIdValid(module_id))
									{
										setYaw(module_id, nodeYaw);
										setName(module_id, name);
										copyObjVar(deed, module_id, "modules."+ modules[i]);
										
										nodeObjects[arrayIdx] = module_id;
										arrayIdx++;
										
										nodeCount++;
									}
								}
								else
								{
									sendSystemMessage(player, SID_RECRUITER_MODULE_ABORTED);
								}
							}
						}
					}
					
					i++;
				}
				
				copyObjVar(deed, camp, "modules");
			}
		}
		setObjVar(camp, "modules.ids", nodeObjects);
		setObjVar(camp, VAR_LIFETIME, lifetime);
		setObjVar(camp, VAR_CREATION_TIME, getCalendarTime());
		
		messageTo(camp, "handleControlTerminalLocation", null, 10f, false);
		messageTo(camp, "handleCampDecay", null, lifetime, false);
	}
	
	
	public static float getAdvancedCampRadius(obj_id camp) throws InterruptedException
	{
		String filename = utils.getTemplateFilenameNoPath(camp);
		return dataTableGetFloat(CAMP_DATATABLE, filename, "radius");
	}
	
	
	public static int getAdvancedCampNumberOfNodes(obj_id camp) throws InterruptedException
	{
		String filename = utils.getTemplateFilenameNoPath(camp);
		return dataTableGetInt(CAMP_DATATABLE, filename, "nodes");
	}
	
	
	public static int getAdvancedCampNodeYaw(obj_id camp, int node) throws InterruptedException
	{
		String filename = utils.getTemplateFilenameNoPath(camp);
		String yawCol = "yaw"+ node;
		
		return dataTableGetInt(CAMP_DATATABLE, filename, yawCol);
	}
	
	
	public static location getAdvancedCampNodeLocation(obj_id camp, location loc, int node) throws InterruptedException
	{
		String filename = utils.getTemplateFilenameNoPath(camp);
		
		String xCol = "x"+ node;
		String zCol = "z"+ node;
		
		float x = dataTableGetFloat(CAMP_DATATABLE, filename, xCol);
		float z = dataTableGetFloat(CAMP_DATATABLE, filename, zCol);
		
		loc.x += x;
		loc.z += z;
		
		return loc;
	}
	
	
	public static obj_id getCurrentAdvancedCamp(obj_id player) throws InterruptedException
	{
		obj_id[] objects = getNonCreaturesInRange(player, 25.0f);
		
		if (objects == null || objects.length == 0)
		{
			return null;
		}
		
		for (int i = 0; i < objects.length; i++)
		{
			testAbortScript();
			if (hasScript(objects[i], "item.camp.camp_advanced"))
			{
				if (isInTriggerVolume(objects[i], "campsite", player))
				{
					return objects[i];
				}
			}
		}
		
		return null;
	}
	
	
	public static boolean isInEntertainmentCamp(obj_id player) throws InterruptedException
	{
		return isInEntertainmentCamp(player, obj_id.NULL_ID);
	}
	
	
	public static boolean isInEntertainmentCamp(obj_id player, obj_id camp) throws InterruptedException
	{
		if (!isIdValid(camp))
		{
			camp = getCurrentAdvancedCamp(player);
		}
		
		if (!isIdValid(camp))
		{
			return false;
		}
		
		if (hasObjVar(camp, "modules.entertainer"))
		{
			return true;
		}
		
		return false;
	}
}
