package script.npc.faction_recruiter;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.factions;
import script.library.faction_perk;
import script.library.sui;
import script.library.utils;
import script.library.combat;


public class biolink_item extends script.base_script
{
	public biolink_item()
	{
	}
	public static final String VAR_TEMP_BASE = "biotemp";
	public static final String VAR_TEMP_EQUIP_FLAG = "biotemp.equip";
	
	public static final string_id SID_POINTS_NEEDED_FOR_BIO_LINK = new string_id("base_player", "faction_points_needed_for_bio_link");
	public static final string_id SID_ITEM_BIO_LINKED = new string_id("base_player", "item_bio_linked");
	public static final string_id SID_MUST_EQUIP_FROM_INVENTORY = new string_id("base_player", "must_equip_from_inventory");
	public static final string_id SID_MUST_BIO_LINK_FROM_INVENTORY = new string_id("base_player", "must_bio_link_from_inventory");
	public static final string_id SID_BIO_LINK_WRONG_OVERT_FACTION = new string_id("base_player", "bio_link_wrong_overt_faction");
	public static final string_id SID_BIO_LINK_WRONG_FACTION = new string_id("base_player", "bio_link_wrong_faction");
	public static final string_id SID_BIO_LINK = new string_id("ui_radial", "bio_link");
	public static final string_id SID_BIO_LINK_MSG_PROMPT = new string_id("sui", "bio_link_item_prompt");
	public static final string_id SID_SPEND_FACTION_POINTS_PROMPT = new string_id("sui", "spend_faction_points");
	public static final string_id SID_BIO_LINK_MSG_TITLE = new string_id("sui", "bio_link_item_title");
	public static final string_id SID_SPECIES_CANNOT_EQUIP = new string_id("base_player", "species_cannot_equip");
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		
		setBioLink(self, null);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		
		obj_id biolink = getBioLink(self);
		if (biolink == null)
		{
			setBioLink(self, null);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAboutToBeTransferred(obj_id self, obj_id destContainer, obj_id transferer) throws InterruptedException
	{
		CustomerServiceLog("Faction", "WARNING: factions::OnAboutToBeTransferred -- "+ self + " biolink_item script.");
		
		obj_id biolink = getBioLink(self);
		if (biolink == null)
		{
			setBioLink(self, null);
			biolink = utils.OBJ_ID_BIO_LINK_PENDING;
		}
		if (biolink == utils.OBJ_ID_BIO_LINK_PENDING && (isPlayer(destContainer) || isAPlayerAppearanceInventoryContainer(destContainer) ))
		{
			
			obj_id containerPlayer = utils.getContainingPlayer(self);
			obj_id appearanceInv = getAppearanceInventory(containerPlayer);
			
			if (containerPlayer != destContainer && destContainer != appearanceInv)
			{
				sendSystemMessage(containerPlayer, SID_MUST_EQUIP_FROM_INVENTORY);
				return SCRIPT_OVERRIDE;
			}
			
			obj_id player = null;
			
			if (containerPlayer == destContainer)
			{
				player = containerPlayer;
			}
			else
			{
				player = getContainedBy(destContainer);
			}
			
			utils.setScriptVar(self, "destContainer", destContainer);
			
			if (!factions.canUseFactionItem(player, self))
			{
				return SCRIPT_OVERRIDE;
			}
			
			int pid = sui.msgbox(self, player, "@"+ SID_BIO_LINK_MSG_PROMPT, sui.YES_NO, "@"+ SID_BIO_LINK_MSG_TITLE, "handleEquipBioLinkSui");
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info item) throws InterruptedException
	{
		
		obj_id biolink = getBioLink(self);
		if (biolink == utils.OBJ_ID_BIO_LINK_PENDING)
		{
			
			if (isIdValid(player) && utils.getContainingPlayer(self) == player)
			{
				
				if (!hasObjVar(self, factions.VAR_FACTION) || factions.canUseFactionItem(player, self))
				{
					item.addRootMenu(menu_info_types.BIO_LINK, SID_BIO_LINK);
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (item == menu_info_types.BIO_LINK)
		{
			if (!_bioLinkItem(self, player, false, true))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleEquipBioLinkSui(obj_id self, dictionary params) throws InterruptedException
	{
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		_bioLinkItem(self, player, true, true);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSpendFactionPointsSui(obj_id self, dictionary params) throws InterruptedException
	{
		int bp = sui.getIntButtonPressed(params);
		if (bp != sui.BP_CANCEL)
		{
			obj_id player = sui.getPlayerId(params);
			_bioLinkItem(self, player, false, false);
		}
		utils.removeScriptVar(self, VAR_TEMP_BASE);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean _bioLinkItem(obj_id self, obj_id player, boolean equipAfterLink, boolean queryPlayer) throws InterruptedException
	{
		if (!isIdValid(self) || !isIdValid(player))
		{
			CustomerServiceLog("Faction", "WARNING: biolink_item._bioLinkItem called with invalid player (%TU) or item ("+ self + ")", player);
			return false;
		}
		
		obj_id biolink = getBioLink(self);
		if (biolink != utils.OBJ_ID_BIO_LINK_PENDING)
		{
			CustomerServiceLog("Faction", "WARNING: Player %TU trying to bio-link item "+ self + " that has link id "+ biolink, player);
			return false;
		}
		
		if (utils.getContainingPlayer(self) != player)
		{
			sendSystemMessage(player, SID_MUST_BIO_LINK_FROM_INVENTORY);
			return false;
		}
		
		String template = getTemplateName(self);
		if ((template.startsWith("object/tangible/wearables/") ) && (!canEquipWearable(player, self) ))
		{
			sendSystemMessage(player, SID_SPECIES_CANNOT_EQUIP);
			return false;
		}
		
		if (isWeapon(self))
		{
			if (!combat.hasCertification(player, self))
			{
				sendSystemMessage(player, SID_SPECIES_CANNOT_EQUIP);
				return false;
			}
		}
		
		if (hasObjVar(self, factions.VAR_FACTION) && !factions.canUseFactionItem(player, self))
		{
			string_id errMsg = SID_BIO_LINK_WRONG_FACTION;
			prose_package pp = new prose_package();
			pp.stringId = errMsg;
			pp.target.nameId = factions.getLocalizedFactionName(getStringObjVar(self, factions.VAR_FACTION));
			sendSystemMessageProse(player, pp);
			return false;
		}
		
		setBioLink(self, player);
		prose_package pp = new prose_package();
		pp.stringId = SID_ITEM_BIO_LINKED;
		sendSystemMessageProse(player, pp);
		sendDirtyObjectMenuNotification(self);
		
		obj_id container = player;
		if (utils.hasScriptVar(self, "destContainer"))
		{
			container = utils.getObjIdScriptVar(self,"destContainer");
		}
		
		if (equipAfterLink)
		{
			equip(self, container);
		}
		
		return true;
	}
	
}
