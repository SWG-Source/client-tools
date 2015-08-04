package script.faction_perk.hq;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.gcw;
import script.library.hq;
import script.library.utils;
import script.library.factions;
import script.library.player_structure;


public class loader extends script.base_script
{
	public loader()
	{
	}
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		if (hasObjVar(self, factions.FACTION))
		{
			factions.setFaction(self, factions.getFaction(self));
		}
		
		String myFac = factions.getFaction(self);
		int intFac = myFac.equals("Rebel") ? gcw.FACTION_REBEL : gcw.FACTION_IMPERIAL;
		int lvl = getHqLevel(self);
		if (lvl > 0)
		{
			factions.changeFactionPoints(myFac, 10 * lvl);
		}
		
		gcw.incrementGCWScore (self);
		gcw.modifyPlanetaryBaseCount(self, intFac, 1);
		setObjVar(self, "lotsRefunded", 1);
		setObjVar(self, "lotsRefunded2", 1);
		setCWData(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		permissionsMakePublic(self);
		setObjVar(self, "playersWithAlignedEnemyFlagsAllowed", 1);
		
		messageTo(self, "handleDelayedRefundChecker", null, 10f, true);
		hq.removeCovertDetector(self);
		setCWData(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnReceivedItem(obj_id self, obj_id srcContainer, obj_id transferer, obj_id item) throws InterruptedException
	{
		if (!isIdValid(srcContainer))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(item))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.isNestedWithin(srcContainer, self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (isPlayer(item) && factions.isDeclared(item) && factions.getFaction(item).equals(factions.getFaction(self)))
		{
			utils.setScriptVar(item, "inAlignedStructure", self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAboutToReceiveItem(obj_id self, obj_id srcContainer, obj_id transferer, obj_id item) throws InterruptedException
	{
		if (utils.isNestedWithinAPlayer(item))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnLostItem(obj_id self, obj_id destContainer, obj_id transferer, obj_id item) throws InterruptedException
	{
		if (!isIdValid(destContainer))
		{
			utils.removeScriptVar(item, "inAlignedStructure");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDelayedRefundChecker(obj_id self, dictionary params) throws InterruptedException
	{
		hq.refundFactionBaseLots(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleHqDetonation(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = obj_id.NULL_ID;
		if (utils.hasScriptVar(self, "faction_hq.detonator"))
		{
			player = utils.getObjIdScriptVar(self, "faction_hq.detonator");
		}
		
		location[] tmplocs = params.getLocationArray("locs");
		
		if (tmplocs == null || tmplocs.length == 0)
		{
			gcw.grantBaseDestructionPoints(self);
			
			player_structure.destroyStructure(self, false, true);
			String hqName = getName(self);
			if (isIdValid(player))
			{
				CustomerServiceLog( "faction_hq", "Faction HQ ("+ self + "), destroyed by normal terminal overload. Player initiating the destruction is %TU", player, null);
			}
			else
			{
				CustomerServiceLog( "faction_hq", "Faction HQ ("+ self + "), destroyed by normal terminal overload.");
			}
			
			return SCRIPT_CONTINUE;
		}
		
		Vector locs = new Vector(Arrays.asList(tmplocs));
		if (locs == null || locs.size() == 0)
		{
			String myFac = factions.getFaction(self);
			int lvl = getHqLevel(self);
			if (lvl > 0)
			{
				factions.changeFactionPoints(myFac, -10 * lvl);
			}
			gcw.grantBaseDestructionPoints(self);
			
			player_structure.destroyStructure(self, false, true);
			String hqName = getName(self);
			if (isIdValid(player))
			{
				CustomerServiceLog( "faction_hq", "Faction HQ ("+ self + "), destroyed by normal terminal overload. Player initiating the destruction is %TU", player, null);
			}
			else
			{
				CustomerServiceLog( "faction_hq", "Faction HQ ("+ self + "), destroyed by normal terminal overload.");
			}
			
			return SCRIPT_CONTINUE;
		}
		
		location there = ((location)(locs.get(0)));
		locs.removeElementAt(0);
		if (locs == null || locs.size() == 0)
		{
			params.remove("locs");
		}
		else
		{
			params.put("locs", locs);
		}
		
		messageTo(self, "handleHqExplosions", params, rand(0.1f, 1.0f), false);
		
		messageTo(self, "handleHqDetonation", params, rand(0.1f, 1f), true);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleHqExplosions(obj_id self, dictionary params) throws InterruptedException
	{
		location[] locs = params.getLocationArray("locs");
		
		if (locs != null && locs.length > 0)
		{
			obj_id players[] = getPlayerCreaturesInRange(getLocation(self), 100.0f);
			if (players != null && players.length > 0)
			{
				
				playClientEffectLoc(players[0], "clienteffect/combat_explosion_lair_large.cef", locs[0], 0.0f);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int getHqLevel(obj_id self) throws InterruptedException
	{
		String template = getTemplateName(self);
		int idx = template.indexOf("_s0");
		if (idx > -1)
		{
			String sLvl = template.substring(idx+3, idx+4);
			int lvl = utils.stringToInt(sLvl);
			return lvl;
		}
		
		return -1;
	}
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		hq.cleanUpHackAlarms(self);
		String myFac = factions.getFaction(self);
		int intFac = myFac.equals("Rebel") ? gcw.FACTION_REBEL : gcw.FACTION_IMPERIAL;
		int lvl = getHqLevel(self);
		gcw.modifyPlanetaryBaseCount(self, intFac, -1);
		gcw.decrementGCWScore(self);
		hq.refundBaseUnit(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public void setCWData(obj_id self) throws InterruptedException
	{
		getClusterWideData("gcw_player_base", "base_cwdata_manager", true, self);
	}
	
	
	public int OnClusterWideDataResponse(obj_id self, String manage_name, String dungeon_name, int request_id, String[] element_name_list, dictionary[] dungeon_data, int lock_key) throws InterruptedException
	{
		
		String name = "base_cwdata_manager"+"-"+self;
		String scene = getCurrentSceneName();
		location loc = getLocation(self);
		String myFac = factions.getFaction(self);
		int intFac = myFac.equals("Rebel") ? gcw.FACTION_REBEL : gcw.FACTION_IMPERIAL;
		
		dictionary dungeon_info = new dictionary();
		dungeon_info.put("dungeon_id", self);
		dungeon_info.put("faction", intFac);
		dungeon_info.put("ownerId", getOwner(self));
		dungeon_info.put("scene", scene);
		dungeon_info.put("position_x", loc.x);
		dungeon_info.put("position_y", loc.y);
		dungeon_info.put("position_z", loc.z);
		
		replaceClusterWideData(manage_name, name, dungeon_info, true, lock_key);
		releaseClusterWideDataLock(manage_name, lock_key);
		return SCRIPT_CONTINUE;
	}
	
}
