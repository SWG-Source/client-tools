package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.collection;
import script.library.group;
import script.library.npe;
import script.library.prose;
import script.library.static_item;
import script.library.utils;


public class corpse extends script.base_script
{
	public corpse()
	{
	}
	public static final String SCRIPT_PLAYER_CORPSE = "corpse.player_corpse";
	public static final String SCRIPT_AI_CORPSE = "corpse.ai_corpse";
	public static final String SCRIPT_AI_CORPSE_INVENTORY = "corpse.ai_corpse_inventory";
	
	public static final String TEMPLATE_PLAYER_CORPSE = "object/tangible/container/corpse/player_corpse.iff";
	
	public static final String VAR_CORPSE_WAYPOINT = "waypoint";
	public static final String VAR_TIME_CREATED = "create_time";
	
	public static final String VAR_HAS_RESOURCE = "hasResource";
	public static final String VAR_BEEN_HARVESTED = "beenHarvested";
	
	public static final String GROUP_CONSENTED = "consented";
	
	public static final String DICT_PLAYER_ID = "playerId";
	public static final String DICT_CORPSE_ID = "corpseId";
	public static final String DICT_CORPSE_LOC = "corpseLoc";
	public static final String DICT_CORPSE_WAYPOINT = "waypoint";
	
	public static final String DICT_PLAYER_NAME = "playerName";
	
	public static final String DICT_COINS = "coins";
	
	public static final String DICT_LOOTERS = "looters";
	
	public static final int AI_CORPSE_EXPIRATION_TIME = 120;
	public static final int AI_CORPSE_EMPTY_TIME = 5;
	
	public static final int PLAYER_CORPSE_EXPIRATION_TIME = 604800;
	
	public static final String HANDLER_CORPSE_EXPIRE = "handleCorpseExpire";
	public static final String HANDLER_ADD_CONSENTED = "handleAddConsentedUser";
	public static final String HANDLER_REMOVE_CONSENTED = "handleRemoveConsentedUser";
	
	public static final String HANDLER_CORPSE_CLEANUP = "handleCorpseCleanup";
	
	public static final String HANDLER_CORPSE_WAYPOINT_UPDATE = "handleCorpseWaypointUpdate";
	
	public static final String HANDLER_REQUEST_COIN_LOOT = "handleRequestCoinLoot";
	
	public static final String HANDLER_COINS_LOOTED = "handleCorpseCoinLoot";
	public static final String HANDLER_COINLOOT_FAILED = "handleCorpseCoinLootFailed";
	
	public static final String HANDLER_CORPSE_EMPTY = "handleCorpseEmpty";
	
	public static final String HANDLER_PERMISSIONS_INITIALIZED = "handleCorpsePermInit";
	
	public static final String HANDLER_CORPSE_DEPOSIT_SUCCESS = "handleCorpseDepositSuccess";
	public static final String HANDLER_CORPSE_DEPOSIT_FAIL = "handleCorpseDepositFail";
	
	public static final String HANDLER_DESTROY_SELF = "handleDestroySelf";
	
	public static final string_id SID_LOOT_ALL = new string_id("sui", "loot_all");
	public static final string_id SID_HARVEST_CORPSE = new string_id("sui", "harvest_corpse");
	public static final string_id SID_HARVEST_MEAT = new string_id("sui", "harvest_meat");
	public static final string_id SID_HARVEST_BONE = new string_id("sui", "harvest_bone");
	public static final string_id SID_HARVEST_HIDE = new string_id("sui", "harvest_hide");
	
	public static final string_id SID_CORPSE_LOOTED = new string_id("base_player", "corpse_looted");
	
	public static final string_id PROSE_ITEM_LOOTED_SELF = new string_id("base_player", "prose_item_looted_self");
	public static final string_id PROSE_ITEM_LOOTED_OTHER = new string_id("base_player", "prose_item_looted_other");
	
	public static final string_id SID_CORPSE_EMPTY = new string_id("error_message","corpse_empty");
	
	public static final string_id SID_REMOVE_ONLY_CONTAINER = new string_id("error_message","remove_only");
	public static final string_id SID_REMOVE_ONLY_CORPSE = new string_id("error_message","remove_only_corpse");
	
	public static final string_id SID_CORPSE_DRAG_INTO = new string_id("error_message","corpse_drag_into");
	public static final string_id SID_CORPSE_DRAG_INSIDE = new string_id("error_message","corpse_drag_inside");
	public static final string_id SID_CORPSE_DRAG_RANGE = new string_id("error_message","corpse_drag_range");
	
	public static final string_id SID_CANNOT_LOOT_ALL = new string_id("error_message","cannot_loot_all");
	
	public static final string_id PROSE_INCOMPLETE_LOOT = new string_id("error_message","prose_incomplete_loot");
	public static final string_id SID_INV_FULL = new string_id("error_message","inv_full");
	
	public static final string_id SID_CLEANUP_AFTER_TRANSACTION = new string_id("base_player","corpse_transaction_pending");
	
	public static final string_id PROSE_HARVESTED_ON_CORPSE = new string_id("error_message","harvested_on_corpse");
	public static final string_id PROSE_RARE_HARVEST_ON_CORPSE = new string_id("error_message","rare_harvest_on_corpse");
	
	public static final string_id SID_CORPSE_HARVEST_SUCCESS = new string_id("skl_use","corpse_harvest_success");
	
	public static final string_id SID_CREATURE_QUALITY_FAT = new string_id("skl_use","creature_quality_fat");
	public static final string_id SID_CREATURE_QUALITY_MEDIUM = new string_id("skl_use","creature_quality_medium");
	public static final string_id SID_CREATURE_QUALITY_SKINNY = new string_id("skl_use","creature_quality_skinny");
	public static final string_id SID_CREATURE_QUALITY_SCRAWNY = new string_id("skl_use","creature_quality_scrawny");
	public static final string_id SID_GROUP_CONSERVE = new string_id("skl_use","group_conserve");
	public static final string_id SID_GROUP_HARVEST_BONUS = new string_id("skl_use","group_harvest_bonus");
	public static final string_id SID_GROUP_HARVEST_BONUS_SCOUT = new string_id("skl_use","group_harvest_bonus_scout");
	public static final string_id SID_GROUP_HARVEST_BONUS_MASTERSCOUT = new string_id("skl_use","group_harvest_bonus_masterscout");
	
	public static final string_id SID_NOTHING_TO_HARVEST = new string_id("skl_use","nothing_to_harvest");
	
	public static final String DATATABLE_SPECIES = "datatables/mob/creatures.iff";
	
	public static final String DATATABLE_COL_HAS_RESOURCE = "hasResources";
	
	public static final String DATATABLE_COL_MEAT_AMT = "meat";
	public static final String DATATABLE_COL_MEAT_TYPE = "meatType";
	
	public static final String DATATABLE_COL_HIDE_AMT = "hide";
	public static final String DATATABLE_COL_HIDE_TYPE = "hideType";
	
	public static final String DATATABLE_COL_BONE_AMT = "bone";
	public static final String DATATABLE_COL_BONE_TYPE = "boneType";
	
	public static final String DATATABLE_COL_RARE_AMT = "rare";
	public static final String DATATABLE_COL_RARE_TYPE = "rareType";
	
	public static final String[] DATATABLE_COL =
	{
		DATATABLE_COL_MEAT_AMT,
		DATATABLE_COL_HIDE_AMT,
		DATATABLE_COL_BONE_AMT,
		DATATABLE_COL_RARE_AMT
	};
	
	public static final int CCR_MEAT = 0;
	public static final int CCR_HIDE = 1;
	public static final int CCR_BONE = 2;
	public static final int CCR_RARE = 3;
	
	public static final int CCR_MAX = 4;
	
	public static final float GROUP_BONUS_NONE = 1.0f;
	public static final float GROUP_BONUS = 1.2f;
	public static final float GROUP_BONUS_SCOUT = 1.3f;
	public static final float GROUP_BONUS_MASTERSCOUT = 1.4f;
	
	
	public static obj_id spawnPlayerCorpse(obj_id player) throws InterruptedException
	{
		if ((player == null) || (!isPlayer(player)))
		{
			return null;
		}
		
		location loc = getLocation(player);
		obj_id corpse = createObject(TEMPLATE_PLAYER_CORPSE, loc);
		if ((corpse == null) || (corpse == obj_id.NULL_ID))
		{
			sendSystemMessageTestingOnly(player, "spawnPlayerCorpse: unable to spawn a corpse at your location");
			sendSystemMessageTestingOnly(player, "spawnPlayerCorpse: loc = "+ loc.toString());
			return null;
		}
		else
		{
			persistObject(corpse);
			setYaw(corpse, getYaw(player));
			
			String pname = getAssignedName(player);
			if (pname == null || pname.equals(""))
			{
				setName(corpse, "Corpse of Unknown Player");
			}
			else
			{
				setName(corpse, "Corpse of "+ pname);
			}
			
			setOwner(corpse, player);
			setObjVar(corpse, utils.VAR_OWNER, player);
			setObjVar(corpse, VAR_TIME_CREATED, getGameTime());
			initializeCorpsePermissions(corpse, null);
			attachScript(corpse, SCRIPT_PLAYER_CORPSE);
			grantCorpseConsent(corpse, player);
			
			if (hasObjVar(player, pclib.VAR_CONSENT_TO_ID))
			{
				obj_id[] consented = getObjIdArrayObjVar(player, pclib.VAR_CONSENT_TO_ID);
				if ((consented != null) && (consented.length > 0))
				{
					for (int i = 0; i < consented.length; i++)
					{
						testAbortScript();
						
						if (!grantCorpseConsent(corpse, consented[i]))
						{
							
						}
					}
				}
			}
			
			messageTo(corpse, HANDLER_CORPSE_EXPIRE, null, PLAYER_CORPSE_EXPIRATION_TIME, true);
		}
		
		return corpse;
	}
	
	
	public static boolean initializeCorpsePermissions(obj_id corpse, dictionary params) throws InterruptedException
	{
		if (!isIdValid(corpse))
		{
			return false;
		}
		
		boolean litmus = true;
		litmus &= permissions.initializePermissions(corpse);
		litmus &= permissions.createPermissionsGroup(corpse, GROUP_CONSENTED);
		litmus &= permissions.grantGroupPermission(corpse, GROUP_CONSENTED, permissions.canOpen);
		
		messageTo(corpse, HANDLER_PERMISSIONS_INITIALIZED, params, 1f, isObjectPersisted(corpse));
		
		return litmus;
	}
	
	
	public static boolean grantCorpseConsent(obj_id corpse, obj_id player) throws InterruptedException
	{
		
		if (!isIdValid(corpse) || !isIdValid(player))
		{
			return false;
		}
		
		dictionary d = new dictionary();
		d.put(DICT_PLAYER_ID, player);
		
		return messageTo(corpse, HANDLER_ADD_CONSENTED, d, 1f, isObjectPersisted(corpse));
	}
	
	
	public static boolean grantCorpseConsent(obj_id[] corpses, obj_id player) throws InterruptedException
	{
		if ((corpses == null) || (corpses.length == 0) || !isIdValid(player))
		{
			return false;
		}
		
		boolean litmus = true;
		for (int i =0; i < corpses.length; i++)
		{
			testAbortScript();
			litmus &= grantCorpseConsent(corpses[i], player);
		}
		return litmus;
	}
	
	
	public static boolean revokeCorpseConsent(obj_id corpse, obj_id player) throws InterruptedException
	{
		if (!isIdValid(corpse) || !isIdValid(player))
		{
			return false;
		}
		
		dictionary d = new dictionary();
		d.put(DICT_PLAYER_ID, player);
		
		return messageTo(corpse, HANDLER_REMOVE_CONSENTED, d, 1f, isObjectPersisted(corpse));
	}
	
	
	public static boolean revokeCorpseConsent(obj_id[] corpses, obj_id player) throws InterruptedException
	{
		if ((corpses == null) || (corpses.length == 0) || !isIdValid(player))
		{
			return false;
		}
		
		boolean litmus = true;
		for (int i =0; i < corpses.length; i++)
		{
			testAbortScript();
			litmus &= revokeCorpseConsent(corpses[i], player);
		}
		return litmus;
	}
	
	
	public static boolean revokeAllCorpseConsent(obj_id owner) throws InterruptedException
	{
		if (!isIdValid(owner))
		{
			return false;
		}
		
		obj_id[] corpses = utils.getObjIdBatchObjVar(owner, pclib.VAR_CORPSE_ID);
		obj_id[] consented = getObjIdArrayObjVar(owner, pclib.VAR_CONSENT_TO_ID);
		if ((corpses == null) || (consented == null))
		{
			return false;
		}
		
		boolean litmus = true;
		for (int i = 0; i < corpses.length; i++)
		{
			testAbortScript();
			for (int n = 0; n < consented.length; n++)
			{
				testAbortScript();
				litmus &= revokeCorpseConsent(corpses[i], consented[n]);
			}
		}
		return litmus;
	}
	
	
	public static boolean cleanUpPlayerCorpse(obj_id corpse) throws InterruptedException
	{
		return cleanUpPlayerCorpse( corpse, true );
	}
	
	
	public static boolean cleanUpPlayerCorpse(obj_id corpse, boolean withDestroyMessage) throws InterruptedException
	{
		obj_id owner = getObjIdObjVar(corpse, utils.VAR_OWNER);
		
		obj_id waypoint = getObjIdObjVar(corpse, VAR_CORPSE_WAYPOINT);
		
		dictionary d = new dictionary();
		d.put(DICT_CORPSE_ID, corpse);
		d.put(DICT_CORPSE_WAYPOINT, waypoint);
		
		messageTo(owner, HANDLER_CORPSE_CLEANUP, d, 1, true);
		
		int cash = getCashBalance(corpse);
		if (cash > 0)
		{
			dictionary dictCoin = new dictionary();
			dictCoin.put(DICT_COINS, cash);
			
			depositCashToBank(corpse, cash, HANDLER_CORPSE_DEPOSIT_SUCCESS, HANDLER_CORPSE_DEPOSIT_FAIL, dictCoin);
			return true;
		}
		
		if (withDestroyMessage)
		{
			destroyObject( corpse );
		}
		return true;
	}
	
	
	public static boolean dragPlayerCorpse(obj_id player, obj_id corpse, boolean giveFeedback) throws InterruptedException
	{
		if ((player == null) || (corpse == null))
		{
			return false;
		}
		
		if (canDragPlayerCorpse(player, corpse, giveFeedback))
		{
			location playerLoc = getLocation(player);
			obj_id[] stuff = getObjectsInRange(playerLoc, 20.0f);
			
			for (int i = 0; i < stuff.length; i++)
			{
				testAbortScript();
				if (isPlayer(stuff[i]))
				{
					playClientEffectLoc(stuff[i], "clienteffect/medic_drag.cef", getLocation(corpse), 0.0f);
				}
			}
			
			setLocation(corpse, getLocation(player));
			updateCorpseOwnerWaypoint(player, corpse);
			return true;
		}
		
		return false;
	}
	
	
	public static boolean dragPlayerCorpse(obj_id player, obj_id corpse) throws InterruptedException
	{
		return dragPlayerCorpse(player, corpse, true);
	}
	
	
	public static obj_id[] dragPlayerCorpse(obj_id player, obj_id[] corpses) throws InterruptedException
	{
		if (!isIdValid(player) || (corpses == null) || (corpses.length == 0))
		{
			return null;
		}
		
		Vector moved = new Vector();
		moved.setSize(0);
		
		for (int i = 0; i < corpses.length; i++)
		{
			testAbortScript();
			if (dragPlayerCorpse(player, corpses[i], false))
			{
				moved = utils.addElement(moved, corpses[i]);
			}
		}
		
		if ((moved == null) || (moved.size() == 0))
		{
			return null;
		}
		
		obj_id[] _moved = new obj_id[0];
		if (moved != null)
		{
			_moved = new obj_id[moved.size()];
			moved.toArray(_moved);
		}
		return _moved;
	}
	
	
	public static boolean isPlayerCorpse(obj_id objectInQuestion) throws InterruptedException
	{
		if (isPlayer(objectInQuestion) && getPosture(objectInQuestion) == POSTURE_DEAD)
		{
			return true;
		}
		
		return false;
	}
	
	
	public static obj_id[] getDraggableCorpsesInRange(obj_id player, boolean feedback) throws InterruptedException
	{
		Vector corpses = new Vector();
		corpses.setSize(0);
		obj_id[] stuff = getObjectsInRange(player, 100.0f);
		
		for (int i = 0; i < stuff.length; i++)
		{
			testAbortScript();
			if (isPlayerCorpse(stuff[i]))
			{
				if (canDragPlayerCorpse(player, stuff[i], true))
				{
					corpses = utils.addElement(corpses, stuff[i]);
				}
			}
		}
		
		if (corpses == null || corpses.size() == 0)
		{
			sendSystemMessage(player, new string_id("spam", "drag_none_in_range") );
		}
		
		obj_id[] _corpses = new obj_id[0];
		if (corpses != null)
		{
			_corpses = new obj_id[corpses.size()];
			corpses.toArray(_corpses);
		}
		return _corpses;
	}
	
	
	public static boolean canDragPlayerCorpse(obj_id player, obj_id corpse, boolean giveFeedback) throws InterruptedException
	{
		if ((player == null) || (corpse == null))
		{
			return false;
		}
		
		location pLoc = getLocation(player);
		location cLoc = getLocation(corpse);
		if (cLoc == null)
		{
			return false;
		}
		
		obj_id owner = getOwner(corpse);
		if (!isIdValid(owner))
		{
			return false;
		}
		
		if (player != owner && !group.inSameGroup(player, owner) && !isGod(player))
		{
			sendSystemMessage(player, permissions.SID_NO_CORPSE_PERMISSION);
			return false;
		}
		
		if (cLoc.area.equals(pLoc.area))
		{
			if (cLoc.cell == pLoc.cell)
			{
				float dist = utils.getDistance2D(pLoc, cLoc);
				
				if (dist <= pclib.RANGE_CORPSE_DRAG_NORMAL)
				{
					return true;
				}
				else
				{
					if (giveFeedback)
					{
						sendSystemMessage(player, SID_CORPSE_DRAG_RANGE);
					}
					
					return false;
				}
			}
			else
			{
				
				obj_id corpseContainer = structure.getContainingBuilding(corpse);
				if (corpseContainer == null)
				{
					
					if (giveFeedback)
					{
						sendSystemMessage(player, SID_CORPSE_DRAG_INTO);
					}
					
					return false;
				}
				else
				{
					
					obj_id playerContainer = structure.getContainingBuilding(player);
					if (playerContainer == null)
					{
						
						location bLoc = getLocation(corpseContainer);
						if (utils.getDistance2D(pLoc, bLoc) <= pclib.RANGE_CORPSE_DRAG_INTERIOR)
						{
							return true;
						}
					}
					else
					{
						
						if (corpseContainer == playerContainer)
						{
							if (giveFeedback)
							{
								sendSystemMessage(player, SID_CORPSE_DRAG_INSIDE);
							}
							
							return false;
						}
					}
				}
			}
		}
		
		return false;
	}
	
	
	public static boolean updateCorpseOwnerWaypoint(obj_id player, obj_id corpse) throws InterruptedException
	{
		if ((player == null) || (corpse == null))
		{
			return false;
		}
		
		obj_id waypoint = getObjIdObjVar(corpse, VAR_CORPSE_WAYPOINT);
		obj_id owner = getObjIdObjVar(corpse, utils.VAR_OWNER);
		
		if ((waypoint == null) || (owner == null))
		{
			return false;
		}
		
		location corpseLoc = getLocation(corpse);
		if (corpseLoc == null)
		{
			return false;
		}
		
		dictionary d = new dictionary();
		d.put(DICT_CORPSE_LOC,corpseLoc);
		d.put(DICT_CORPSE_WAYPOINT, waypoint);
		
		return messageTo(owner, HANDLER_CORPSE_WAYPOINT_UPDATE, d, 0, false);
	}
	
	
	public static boolean lootCorpseCoins(obj_id player, obj_id corpse) throws InterruptedException
	{
		if (!isIdValid(player) || !isIdValid(corpse))
		{
			return false;
		}
		
		int cash = getCashBalance(corpse);
		LOG("grouping","lootCorpseCoins: corpse cash = "+ cash);
		if (cash > 0)
		{
			dictionary d = new dictionary();
			d.put(DICT_CORPSE_ID, corpse);
			d.put(DICT_PLAYER_ID, player);
			d.put(DICT_COINS, cash);
			
			messageTo(player, HANDLER_REQUEST_COIN_LOOT, d, 1f, isObjectPersisted(corpse));
			return true;
		}
		
		return false;
	}
	
	
	public static boolean openPlayerCorpse(obj_id player, obj_id corpse) throws InterruptedException
	{
		boolean lootedCoins = lootCorpseCoins(player, corpse);
		
		boolean canLootItems = true;
		obj_id[] corpseContents = getContents(corpse);
		if ((corpseContents == null) || (corpseContents.length == 0))
		{
			canLootItems = false;
		}
		
		if (!lootedCoins && !canLootItems)
		{
			sendSystemMessage(player, SID_CORPSE_EMPTY);
			return false;
		}
		
		return true;
	}
	
	
	public static boolean lootPlayerCorpse(obj_id player, obj_id corpse) throws InterruptedException
	{
		if (!isIdValid(player) || !isIdValid(corpse))
		{
			return false;
		}
		
		if (getOwner(corpse) != player && !isGod(player))
		{
			return false;
		}
		
		if (openPlayerCorpse(player, corpse))
		{
			obj_id[] corpseContents = getContents(corpse);
			if ((corpseContents == null) || (corpseContents.length == 0))
			{
				return false;
			}
			
			obj_id pInv = utils.getInventoryContainer(player);
			if (!isIdValid(pInv))
			{
				return false;
			}
			
			int moved = moveObjects(corpseContents, pInv);
			if (moved < corpseContents.length)
			{
				obj_id[] leftContents = getContents(corpse);
				if ((leftContents == null) || (leftContents.length == 0))
				{
				}
				else
				{
					int itemSize = getVolume(leftContents[0]);
					int freeVol = getVolumeFree(pInv);
					
					if (itemSize > freeVol)
					{
						sendSystemMessage(player, SID_INV_FULL);
					}
				}
				
				int itemsLeft = corpseContents.length - moved;
				prose_package pp = prose.getPackage(PROSE_INCOMPLETE_LOOT, null, corpse, itemsLeft);
				sendSystemMessageProse(player, pp);
				return false;
			}
			
			sendSystemMessage(player, SID_CORPSE_LOOTED);
			
			if (getCashBalance(corpse) == 0)
			{
				cleanUpPlayerCorpse(corpse);
			}
			else
			{
				
				if (!hasObjVar(corpse, "cleanupStamp"))
				{
					messageTo(corpse, "handleAttemptCorpseCleanup", null, 300, isObjectPersisted(corpse));
				}
			}
			
			return true;
		}
		
		return false;
	}
	
	
	public static boolean openAICorpse(obj_id player, obj_id corpse) throws InterruptedException
	{
		obj_id[] corpseContents = getContents(utils.getInventoryContainer(corpse));
		if ((corpseContents == null) || (corpseContents.length == 0))
		{
			LOG("grouping","openAICorpse: no corpse contents!");
			
			sendSystemMessage(player, SID_CORPSE_EMPTY);
			return false;
		}
		
		return true;
	}
	
	
	public static boolean lootAICorpse(obj_id player, obj_id corpse) throws InterruptedException
	{
		
		if (!isIdValid(player) || !isIdValid(corpse))
		{
			return false;
		}
		
		LOG("loot","lootAICorpse: entered...");
		if (openAICorpse(player, corpse))
		{
			LOG("loot","lootAICorpse: successfully opened corpse!!!!!!!!!!!!!!!!!!!!!");
			if (lootAICorpseContents(player, corpse))
			{
				LOG("loot","lootAICorpse: successfully looted contents");
				messageTo(corpse, HANDLER_CORPSE_EMPTY, null, 1f, isObjectPersisted(corpse));
			}
			else
			{
				LOG("loot","lootAICorpse: FAILED TO LOOT ALL CONTENTS...hahaha");
			}
			
		}
		
		LOG("loot","lootAICorpse: returning false");
		return false;
	}
	
	
	public static boolean lootAICorpseContents(obj_id player, obj_id corpse) throws InterruptedException
	{
		boolean grouped = false;
		boolean movedAll = true;
		
		if (group.isGrouped (player))
		{
			grouped = true;
		}
		
		if (!isIdValid(player) || !isIdValid(corpse))
		{
			return false;
		}
		
		obj_id cInv = utils.getInventoryContainer(corpse);
		if (!isIdValid(cInv))
		{
			return false;
		}
		
		obj_id[] corpseContents = getValidLootContents(getContents(cInv));
		LOG("loot", "corpseContents.length: "+corpseContents.length);
		
		if ((corpseContents == null) || (corpseContents.length == 0))
		{
			LOG("loot", "I AM DESTOYING CORPSE because corpseContents.length = "+corpseContents.length);
			messageTo(corpse, "handleCorpseExpire", null, 5, true);
			return false;
		}
		
		obj_id pInv = utils.getInventoryContainer(player);
		if (!isIdValid(pInv))
		{
			return false;
		}
		
		LOG("loot", "Inventory received: "+pInv);
		
		int moved = moveObjects(corpseContents, pInv);
		LOG("loot", "moved: "+moved);
		
		obj_id[] leftContents = getContents(cInv);
		LOG("loot", "leftContents: "+leftContents.length);
		if (grouped)
		{
			obj_id gid = getGroupObject(player);
			if (isIdValid(gid))
			{
				Vector movedContents = utils.removeElements(corpseContents, leftContents);
				if (movedContents != null && movedContents.size() > 0)
				{
					for (int z = 0; z < movedContents.size(); z ++)
					{
						testAbortScript();
						group.notifyItemLoot(gid, player, corpse, ((obj_id)(movedContents.get(z))));
					}
				}
			}
		}
		
		if (leftContents.length > 0)
		{
			if (grouped)
			{
				loot.sendGroupLootSystemMessage (leftContents[0], player, group.GROUP_STF, "full_inventory");
				
			}
			
			prose_package pp = prose.getPackage(PROSE_INCOMPLETE_LOOT, null, corpse, leftContents.length);
			sendSystemMessageProse(player, pp);
			
			return false;
		}
		
		sendSystemMessage(player, SID_CORPSE_LOOTED);
		return true;
	}
	
	
	public static obj_id[] getValidLootContents(obj_id[] corpseContents) throws InterruptedException
	{
		if (corpseContents == null || corpseContents.length == 0)
		{
			return null;
		}
		
		Vector validContents = new Vector();
		validContents.setSize(0);
		
		for (int i=0; i<corpseContents.length; i++)
		{
			testAbortScript();
			if (!utils.hasScriptVar(corpseContents[i], "isCreatureWeapon"))
			{
				utils.addElement(validContents, corpseContents[i]);
			}
			else
			{
				trial.cleanupObject(corpseContents[i]);
			}
		}
		
		obj_id[] _validContents = new obj_id[0];
		if (validContents != null)
		{
			_validContents = new obj_id[validContents.size()];
			validContents.toArray(_validContents);
		}
		return _validContents;
	}
	
	
	public static int[] hasResource(String mobType) throws InterruptedException
	{
		if ((mobType == null) || (mobType.equals("")))
		{
			return null;
		}
		
		int val = dataTableGetInt(DATATABLE_SPECIES, mobType, DATATABLE_COL_HAS_RESOURCE);
		if (val == 1)
		{
			int[] ret = new int[CCR_MAX];
			
			int badCnt = 0;
			for (int i = 0; i < CCR_MAX; i++)
			{
				testAbortScript();
				String type = dataTableGetString( DATATABLE_SPECIES, mobType, DATATABLE_COL[i]+"Type");
				if (!type.equals("none") && type != null)
				{
					ret[i] = 1;
				}
				else
				{
					ret[i] = 0;
				}
				if (ret[i] < 1)
				{
					badCnt++;
				}
			}
			
			if (badCnt == ret.length)
			{
				return null;
			}
			else if ((badCnt == ret.length - 1) && (ret[CCR_RARE] > 0))
			{
				return null;
			}
			
			return ret;
		}
		
		return null;
	}
	
	
	public static boolean hasResource(obj_id corpse) throws InterruptedException
	{
		if (corpse == null)
		{
			return false;
		}
		
		return ( hasResource(ai_lib.getCreatureName(corpse)) != null );
	}
	
	
	public static boolean harvestCreatureCorpse(obj_id player, obj_id corpse, String restype) throws InterruptedException
	{
		
		obj_id pInv = utils.getInventoryContainer(player);
		obj_id cInv = utils.getInventoryContainer(corpse);
		if ((pInv == null) || (cInv == null))
		{
			return false;
		}
		
		int freeVol = getVolumeFree(pInv);
		
		if (1 > freeVol)
		{
			sendSystemMessage(player, SID_INV_FULL);
			return false;
		}
		
		float inGroupBonusMultiplier = GROUP_BONUS_NONE;
		
		if ((player == null) || (corpse == null))
		{
			return false;
		}
		
		if (!hasObjVar(corpse, VAR_HAS_RESOURCE))
		{
			return false;
		}
		
		if (utils.hasScriptVar( corpse, "harvestedBy." + player ))
		{
			return false;
		}
		
		if (getDistance(getLocation(player), getLocation(corpse)) > 15.0f)
		{
			return false;
		}
		utils.setScriptVar( corpse, "harvestedBy."+ player, 1 );
		
		String mobType = ai_lib.getCreatureName(corpse);
		dictionary params = null;
		if (restype.equals(""))
		{
			params = getRandomHarvestCorpseResources( player, corpse, 1 );
		}
		else
		{
			params = getHarvestCorpseResources( player, corpse, restype );
		}
		if ((params == null) || (params.isEmpty()))
		{
			return false;
		}
		
		int sklMod = getSkillStatMod( player, "creature_harvesting");
		
		float bonusHarvest = sklMod / 100.0f;
		float skillEfficiency = ((sklMod * 10f) + 500f) / 2000f;
		
		boolean litmus = true;
		int harvestXP = 0;
		int successCount = 0;
		java.util.Enumeration keys = params.keys();
		int amt = 0;
		String type = null;
		int actualAmount = 0;
		
		while (keys.hasMoreElements())
		{
			testAbortScript();
			
			String resourceType = (String)(keys.nextElement());
			amt = params.getInt(resourceType);
			amt += amt * bonusHarvest;
			
			amt = (int)(amt * skillEfficiency);
			
			if (group.isGrouped(player))
			{
				obj_id[] groupMembers = getGroupMemberIds(getGroupObject(player));
				for (int i=0; i < groupMembers.length; i++)
				{
					testAbortScript();
					
					if (isIdValid(groupMembers[i]) && exists(groupMembers[i]) && isPlayer(groupMembers[i]) && groupMembers[i] != player)
					{
						
						float dist = getDistance(player, groupMembers[i]);
						if (dist != -1 && dist <= 64)
						{
							
							if (inGroupBonusMultiplier == GROUP_BONUS_NONE)
							{
								inGroupBonusMultiplier = GROUP_BONUS;
							}
							
							if (hasSkill(groupMembers[i], "outdoors_scout_novice") && inGroupBonusMultiplier < GROUP_BONUS_SCOUT)
							{
								inGroupBonusMultiplier = GROUP_BONUS_SCOUT;
							}
							
							if (hasSkill(groupMembers[i], "outdoors_scout_master"))
							{
								inGroupBonusMultiplier = GROUP_BONUS_MASTERSCOUT;
							}
						}
					}
				}
				
				amt *= inGroupBonusMultiplier;
			}
			
			if (amt <= 0)
			{
				amt = 1;
			}
			
			String sceneName = getCurrentSceneName();
			String rsrcMapTable = "datatables/creature_resource/resource_scene_map.iff";
			String correctedPlanetName = dataTableGetString(rsrcMapTable, sceneName, 1);
			
			if (correctedPlanetName == null || correctedPlanetName.equals(""))
			{
				correctedPlanetName = "tatooine";
			}
			
			type = resourceType + "_"+ correctedPlanetName;
			
			if (amt > 0)
			{
				
				if (hasObjVar(corpse, "npe.harvestable"))
				{
					
					actualAmount = npe.harvestNpeResourceStack(player, type, amt);
					
				}
				
				else
				{
					
					actualAmount = extractCorpseResource( type, amt, getLocation(getTopMostContainer(corpse)), pInv, cInv, 1 );
				}
				
				if ((actualAmount <= 0) && hasObjVar(player, "newbie_handoff.mission.harvest"))
				{
					actualAmount = 1;
				}
				
				if (actualAmount > 0)
				{
					params.put( resourceType, actualAmount );
					
					successCount++;
					
					if (hasScript(player, "theme_park.new_player.new_player"))
					{
						dictionary webster = new dictionary();
						webster.put("resourceType", resourceType);
						webster.put("amount", actualAmount);
						messageTo(player, "handleNewPlayerScoutAction", webster, 1, false);
					}
				}
				else
				{
					litmus &= false;
				}
			}
			else
			{
				litmus &= false;
			}
		}
		
		obj_id groupId = getGroupObject(player);
		
		if (litmus && actualAmount > 0)
		{
			
			type = resource.getResourceName( type );
			prose_package pp = null;
			pp = prose.getPackage( new string_id("skl_use", "harvest_success"), null, type, null, null, null, null, null, null, null, actualAmount, 0.f );
			sendSystemMessageProse( player, pp );
			
			if (inGroupBonusMultiplier == GROUP_BONUS_MASTERSCOUT)
			{
				sendSystemMessage(player, SID_GROUP_HARVEST_BONUS_MASTERSCOUT);
			}
			if (inGroupBonusMultiplier == GROUP_BONUS_SCOUT)
			{
				sendSystemMessage(player, SID_GROUP_HARVEST_BONUS_SCOUT);
			}
			if (inGroupBonusMultiplier == GROUP_BONUS)
			{
				sendSystemMessage(player, SID_GROUP_HARVEST_BONUS);
			}
			
			if (isIdValid(groupId))
			{
				group.notifyHarvest( groupId, player, corpse, type, actualAmount );
			}
		}
		
		if (successCount == 0)
		{
			sendSystemMessage( player, SID_NOTHING_TO_HARVEST );
		}
		else
		{
			if (hasObjVar(player, "newbie_handoff.mission.harvest"))
			{
				messageTo( player, "missionHarvestComplete", null, 0, true );
			}
		}
		
		messageTo(corpse, HANDLER_CORPSE_EMPTY, null, 1f, isObjectPersisted(corpse));
		return litmus;
	}
	
	
	public static void setCreatureHarvestXp(obj_id corpse, obj_id player) throws InterruptedException
	{
		
	}
	
	
	public static dictionary getHarvestCorpseResources(obj_id player, obj_id corpse, String restype) throws InterruptedException
	{
		if ((player == null) || (corpse == null))
		{
			return null;
		}
		
		dictionary d = new dictionary();
		String mobType = ai_lib.getCreatureName( corpse );
		if (mobType == null)
		{
			return null;
		}
		
		int amt = (int) (Math.pow( ai_lib.getLevel( corpse ), 1.65f ) * 0.4f + 2.f);
		amt += 25;
		if (amt < 1)
		{
			amt = 1;
		}
		if (amt > 700)
		{
			amt = 700;
		}
		if (!restype.equals(""))
		{
			String typeName = utils.dataTableGetString( DATATABLE_SPECIES, mobType, restype + "Type");
			if ((typeName == null) || (typeName.equals("")) || (typeName.equals("none")))
			{
			}
			else
			{
				d.put( typeName, amt );
			}
		}
		else
		{
			for (int i = 0; i <= CCR_BONE; i++)
			{
				testAbortScript();
				String typeName = utils.dataTableGetString( DATATABLE_SPECIES, mobType, DATATABLE_COL[i] + "Type");
				if ((typeName == null) || (typeName.equals("")) || (typeName.equals("none")))
				{
				}
				else
				{
					d.put( typeName, amt );
					
				}
			}
		}
		
		return d;
	}
	
	
	public static dictionary getRandomHarvestCorpseResources(obj_id player, obj_id corpse, int maxTypes) throws InterruptedException
	{
		if ((player == null) || (corpse == null) || (maxTypes < 1))
		{
			return null;
		}
		
		dictionary params = getHarvestCorpseResources(player, corpse, "");
		if ((params == null) || (params.isEmpty()))
		{
			LOG("harvestCorpse","getRandomHarvestCorpseResources -> no corpse resources!");
			return null;
		}
		
		dictionary ret = new dictionary();
		
		for (int n = 0; n < maxTypes; n++)
		{
			testAbortScript();
			if (!params.isEmpty())
			{
				java.util.Enumeration keys = params.keys();
				
				int size = params.size();
				int idx = rand(0, size - 1);
				
				for (int i = 0; i < idx; i++)
				{
					testAbortScript();
					String temp = (String)keys.nextElement();
				}
				
				String type = (String)keys.nextElement();
				int amt = params.getInt(type);
				
				ret.put(type, amt);
				
				params.remove(type);
			}
		}
		
		if ((ret == null) || (ret.isEmpty()))
		{
			LOG("harvestCorpse","getRandomHarvestCorpseResources -> no returnable resources!");
			return null;
		}
		
		return ret;
	}
	
	
	public static dictionary getRandomHarvestCorpseResources(obj_id player, obj_id corpse) throws InterruptedException
	{
		return getRandomHarvestCorpseResources(player, corpse, 1);
	}
	
	
	public static int extractCorpseResource(String resourceClass, int amt, location loc, obj_id pInv, obj_id cInv, int min) throws InterruptedException
	{
		
		if ((resourceClass.equals("")) || (amt < 1) || (loc == null) || (pInv == null) || (cInv == null))
		{
			LOG("harvestCorpse","ERROR: extractCorpseResource -> invalid parameters!!");
			return -1;
		}
		
		obj_id playerId = obj_id.NULL_ID;
		obj_id corpseId = getContainedBy( cInv );
		
		obj_id[] crates = resource.createRandom( resourceClass, amt, loc, cInv, playerId, min);
		
		obj_id player = utils.getContainingPlayer(pInv);
		collection.collectionResource(player, resourceClass);
		
		if ((crates == null) || (crates.length == 0))
		{
			LOG("harvestCorpse","ERROR: extractCorpseResource -> unable to create random resources!! "+ resourceClass);
			return -1;
		}
		
		int total = 0;
		for (int i = 0; i < crates.length; i++)
		{
			testAbortScript();
			total += getResourceContainerQuantity( crates[i] );
		}
		
		boolean movefail = false;
		for (int i = 0; i < crates.length; i++)
		{
			testAbortScript();
			if (!putIn( crates[i], pInv, playerId ))
			{
				movefail = true;
			}
		}
		
		if (movefail)
		{
			prose_package pp = prose.getPackage( PROSE_HARVESTED_ON_CORPSE, corpseId );
			sendSystemMessageProse( playerId, pp );
		}
		
		return total;
	}
	
	
	public static boolean isCorpseEmpty(obj_id objCorpse) throws InterruptedException
	{
		
		obj_id inv = utils.getInventoryContainer(objCorpse);
		if (isIdValid(objCorpse))
		{
			obj_id[] contents = getContents(inv);
			LOG("corpse_debug", "contents are "+ contents);
			if ((contents == null) || (contents.length == 0))
			{
				int cash = getCashBalance(objCorpse);
				if (cash == 0)
				{
					LOG("corpse_debug", "cash is 0");
					if (hasObjVar(objCorpse, "hasResource"))
					{
						LOG("corpse_debug", "resources is very yes");
						return false;
					}
					else
					{
						LOG("corpse_debug", "no resources here");
						return true;
					}
				}
			}
		}
		return false;
	}
	
	
	public static boolean hasLootPermissions(obj_id corpse, obj_id player) throws InterruptedException
	{
		obj_id killCredit = getObjIdObjVar(corpse, xp.VAR_TOP_GROUP);
		
		if (exists(corpse) && hasObjVar(corpse, "storytellerid"))
		{
			obj_id playerstorytellerId = utils.getObjIdScriptVar(player, "storytellerid");
			obj_id corpsestorytellerId = getObjIdObjVar(corpse, "storytellerid");
			if (isIdValid(playerstorytellerId) && playerstorytellerId == corpsestorytellerId)
			{
				return true;
			}
			else if (isGod(player))
			{
				sendSystemMessageTestingOnly(player, "GOD MODE: Granting access to storyteller loot container that you do not have permission for.");
				return true;
			}
			return false;
		}
		
		if (player == killCredit)
		{
			return true;
		}
		
		if (group.isGroupObject(killCredit))
		{
			obj_id[] killList = getGroupMemberIds(killCredit);
			for (int i = 0; i < killList.length; i++)
			{
				testAbortScript();
				if (player == killList[i])
				{
					return true;
				}
			}
		}
		
		if (isGod(player))
		{
			sendSystemMessageTestingOnly(player, "GOD MODE: Granting access to loot container that you do not have permission for.");
			return true;
		}
		
		return false;
	}
	
	
	public static void showLootMeParticle(obj_id corpse) throws InterruptedException
	{
		obj_id killCredit = getObjIdObjVar(corpse, xp.VAR_TOP_GROUP);
		obj_id[] killList;
		if (group.isGroupObject(killCredit))
		{
			killList = getGroupMemberIds(killCredit);
		}
		else
		{
			killList = new obj_id[1];
			killList[0] = killCredit;
		}
		
		transform offset = transform.identity.setPosition_p(0f, getObjectCollisionRadius(corpse), 0);
		playClientEffectObj(killList, "appearance/pt_loot_disc.prt", corpse, "", offset, "lootMe");
		
	}
	
	
	public static void clearLootMeParticle(obj_id corpse) throws InterruptedException
	{
		obj_id killCredit = getObjIdObjVar(corpse, xp.VAR_TOP_GROUP);
		obj_id[] killList;
		if (group.isGroupObject(killCredit))
		{
			killList = getGroupMemberIds(killCredit);
		}
		else
		{
			killList = new obj_id[1];
			killList[0] = killCredit;
		}
		
		stopClientEffectObjByLabel(killList, corpse, "lootMe", false);
	}
}
