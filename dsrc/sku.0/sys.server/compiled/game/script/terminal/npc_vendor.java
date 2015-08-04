package script.terminal;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.player_structure;
import script.library.chat;
import script.library.sui;
import script.library.utils;
import script.library.prose;
import script.library.slots;
import script.library.ai_lib;
import script.library.dressup;
import script.library.smuggler;
import script.library.vendor_lib;


public class npc_vendor extends script.base_script
{
	public npc_vendor()
	{
	}
	public static final String LOGGING_CATEGORY = "vendor";
	public static final boolean LOGGING_ON = true;
	
	public static final string_id SID_ITEM_DROP = new string_id("ui_radial", "item_drop");
	public static final string_id SID_DROP_NPC_VENDOR_BUILDING = new string_id("player_structure", "drop_npc_vendor_building");
	public static final string_id SID_DROP_NPC_VENDOR_PERM = new string_id("player_structure", "drop_npc_vendor_perm");
	public static final string_id SID_ITEM_DESTROY = new string_id("ui_radial", "item_destroy");
	public static final string_id SID_AREABARKS_ENABLED = new string_id("player_structure", "areabarks_enabled");
	public static final string_id SID_AREABARKS_DISABLED = new string_id("player_structure", "areabarks_disabled");
	public static final string_id SID_OBSCENE = new string_id("player_structure", "obscene");
	
	public static final String TBL_VENDOR_SUBCATEGORIES = "datatables/vendor/vendor_map_subcategories.iff";
	public static final String TBL_VENDOR_ANIMS = "datatables/vendor/vendor_areabark_anims.iff";
	public static final String TBL_VENDOR_MOODS = "datatables/vendor/vendor_areabark_moods.iff";
	public static final String TBL_VENDOR_STRCATS = "datatables/vendor/vendor_areabark_strcats.iff";
	
	public static final String ALERT_VOLUME_NAME = "vendorTriggerVolume";
	
	public static final String WKE_CAN_WEAR_PADAWAN_ROBE = "object/tangible/wearables/robe/robe_jedi_padawan_generic.iff";
	public static final String STANDARD_VENDOR_SHIRT = "object/tangible/wearables/shirt/shirt_s11.iff";
	public static final String STANDARD_VENDOR_PANTS = "object/tangible/wearables/pants/pants_s14.iff";
	public static final String STANDARD_VENDOR_SHOES = "object/tangible/wearables/boots/boots_s14.iff";
	public static final String ITH_VENDOR_SHIRT = "object/tangible/wearables/ithorian/ith_shirt_s05.iff";
	public static final String ITH_VENDOR_PANTS = "object/tangible/wearables/ithorian/ith_pants_s03.iff";
	
	public static final String NON_ENHANCEMENT_BUFF = "buff.non_enhancement";
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		removeTriggerVolume( "alertTriggerVolume");
		setInvulnerable( self, true );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		
		if (hasObjVar( self, "vendor.areaBarks" ))
		{
			createTriggerVolume( ALERT_VOLUME_NAME, 10, true );
		}
		
		String templateName = getTemplateName( self );
		if (!hasObjVar( self, "dressed" ) && (templateName.indexOf( "ithorian" ) > 0))
		{
			
			dressup.dressNpc( self, "random_ithorian", true );
			setObjVar( self, "dressed", 1 );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		obj_id ownerId = getObjIdObjVar( self, "vendor_owner");
		if (player == ownerId)
		{
			
			obj_id inventory = getObjectInSlot( player, "inventory");
			if (contains( inventory, self ))
			{
				mi.addRootMenu( menu_info_types.ITEM_DESTROY, SID_ITEM_DESTROY );
				return SCRIPT_CONTINUE;
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		obj_id ownerId = getObjIdObjVar( self, "vendor_owner");
		if (player != ownerId)
		{
			return SCRIPT_CONTINUE;
		}
		int hiringSkillMod = getSkillStatisticModifier(player, "hiring");
		
		if ((item == menu_info_types.SERVER_MENU4) && utils.isProfession(player, utils.TRADER))
		{
			
			if (hasObjVar( self, "vendor.areaBarks" ))
			{
				
				removeObjVar( self, "vendor.areaBarks");
				removeTriggerVolume( ALERT_VOLUME_NAME );
				
				sendSystemMessage( player, SID_AREABARKS_DISABLED );
			}
			else
			{
				
				String[] rawAnims = dataTableGetStringColumnNoDefaults( TBL_VENDOR_ANIMS, 0 );
				String[] anims = new String[rawAnims.length];
				for (int i=0; i<rawAnims.length; i++)
				{
					testAbortScript();
					anims[i] = "@player_structure:"+rawAnims[i];
				}
				sui.listbox( self, player, "@player_structure:vendor_anim_d", sui.OK_CANCEL, "@player_structure:vendor_anim_t", anims, "handleVendorAnimSelect", true );
			}
			
			sendDirtyObjectMenuNotification( self );
		}
		
		else if ((item == menu_info_types.SERVER_MENU8) && hiringSkillMod >= 90)
		{
			
			chat._chat( self, null, chat.CHAT_SAY, chat.MOOD_CHEERFUL, null, new string_id( "player_structure", "wear_how"), null );
			doAnimationAction( self, "slow_down");
			
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAboutToBeTransferred(obj_id self, obj_id destContainer, obj_id transferer) throws InterruptedException
	{
		if (hasObjVar( self, "vendor.areaBarks" ))
		{
			removeObjVar( self, "vendor.areaBarks");
			removeTriggerVolume( ALERT_VOLUME_NAME );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGiveItem(obj_id self, obj_id item, obj_id player) throws InterruptedException
	{
		blog("OnGiveItem init");
		
		obj_id ownerId = getObjIdObjVar( self, "vendor_owner");
		if (player != ownerId)
		{
			return SCRIPT_CONTINUE;
		}
		
		String templateName = getTemplateName( self );
		String itemName = getTemplateName( item );
		if (templateName == null || templateName.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		else if (itemName == null || itemName.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(item, NON_ENHANCEMENT_BUFF))
		{
			
			doAnimationAction(self, "shake_head_no");
			return SCRIPT_CONTINUE;
		}
		
		if (vendor_lib.isSpecialVendor(self) && getIntObjVar(self, "vendor.special_vendor_clothing") == 0)
		{
			
			if (getIntObjVar(self, "vendor.special_vendor_basic") == 1)
			{
				chat._chat( self, null, chat.CHAT_SAY, chat.MOOD_STUBBORN, null, new string_id("player_structure", "wear_not_special_vendor"), null );
				doAnimationAction( self, "wave_finger_warning");
				return SCRIPT_CONTINUE;
			}
			
			doAnimationAction(self, "shake_head_no");
			return SCRIPT_CONTINUE;
		}
		
		if (templateName.indexOf( "ithorian" ) > -1)
		{
			blog("I am an Ithorian");
			if (itemName.indexOf( "ith_" ) > -1)
			{
				blog("I have received an Ithorian wearable!");
				
			}
			else if (itemName.equals(WKE_CAN_WEAR_PADAWAN_ROBE))
			{
				
			}
			else if (!(itemName.indexOf("cybernetic_") > -1))
			{
				chat._chat( self, null, chat.CHAT_SAY, chat.MOOD_STUBBORN, null, new string_id( "player_structure", "wear_not_ithorian"), null );
				doAnimationAction( self, "wave_on_dismissing");
				return SCRIPT_CONTINUE;
			}
		}
		
		if (templateName.indexOf( "wookiee" ) > -1)
		{
			if ((itemName.indexOf( "wke_" ) > -1) || (itemName.indexOf( "armor_kashyyykian_" ) > -1))
			{
				
			}
			else if (itemName.equals(WKE_CAN_WEAR_PADAWAN_ROBE))
			{
				
			}
			else if (!(itemName.indexOf("cybernetic_") > -1))
			{
				chat._chat( self, null, chat.CHAT_SAY, chat.MOOD_STUBBORN, null, new string_id( "player_structure", "wear_not_wookiee"), null );
				doAnimationAction( self, "wave_on_dismissing");
				return SCRIPT_CONTINUE;
			}
		}
		else if (templateName.indexOf( "ithorian" ) > -1)
		{
			blog("itemName: "+itemName);
			if ((itemName.indexOf( "ith_" ) > -1))
			{
				
			}
			else if (itemName.equals(WKE_CAN_WEAR_PADAWAN_ROBE))
			{
				
			}
			else if (!(itemName.indexOf("cybernetic_") > -1))
			{
				chat._chat( self, null, chat.CHAT_SAY, chat.MOOD_STUBBORN, null, new string_id( "player_structure", "wear_not_wookiee"), null );
				doAnimationAction( self, "wave_on_dismissing");
				return SCRIPT_CONTINUE;
			}
		}
		else
		{
			if (itemName.indexOf( "wke_" ) > -1)
			{
				chat._chat( self, null, chat.CHAT_SAY, chat.MOOD_STUBBORN, null, new string_id( "player_structure", "wear_no_wookiee"), null );
				doAnimationAction( self, "wave_on_dismissing");
				return SCRIPT_CONTINUE;
			}
			else if (itemName.indexOf( "ith_" ) > -1)
			{
				chat._chat( self, null, chat.CHAT_SAY, chat.MOOD_STUBBORN, null, new string_id( "player_structure", "wear_no_ithorian"), null );
				doAnimationAction( self, "wave_on_dismissing");
				return SCRIPT_CONTINUE;
			}
		}
		
		if (templateName.indexOf( "ishi_tib" ) > -1 || templateName.indexOf( "aqualish" ) > -1 || templateName.indexOf( "devaronian" ) > -1)
		{
			if (itemName.startsWith("object/tangible/wearables/hat"))
			{
				chat._chat( self, null, chat.CHAT_SAY, chat.MOOD_STUBBORN, null, new string_id( "player_structure", "wear_not_ishitib"), null );
				doAnimationAction( self, "wave_on_dismissing");
				return SCRIPT_CONTINUE;
			}
		}
		
		if (utils.isContainer(item) && smuggler.hasItemsInContainer(item))
		{
			chat._chat( self, null, chat.CHAT_SAY, chat.MOOD_STUBBORN, null, new string_id( "player_structure", "wear_no_bag_full"), null );
			doAnimationAction( self, "wave_on_dismissing");
			return SCRIPT_CONTINUE;
		}
		
		blog("OnGiveItem ABOUT TO FORCE CLOTHING");
		
		messageTo(self, "checkNakedSlots", null, 1, false);
		
		int hiringSkillMod = getSkillStatisticModifier(player, "hiring");
		if (hiringSkillMod < 90)
		{
			chat._chat( self, null, chat.CHAT_SAY, chat.MOOD_OFFENDED, null, new string_id( "player_structure", "wear_noway"), null );
			doAnimationAction( self, "wave_on_dismissing");
			return SCRIPT_CONTINUE;
		}
		
		if (equipOverride( item, self ))
		{
			
			if (isGameObjectTypeOf( item, GOT_weapon ))
			{
				chat._chat( self, null, chat.CHAT_SAY, chat.MOOD_HAPPY, null, new string_id( "player_structure", "wear_yes_weapon"), null );
			}
			else
			{
				chat._chat( self, null, chat.CHAT_SAY, chat.MOOD_HAPPY, null, new string_id( "player_structure", "wear_yes"), null );
			}
			doAnimationAction( self, "pose_proudly");
			return SCRIPT_CONTINUE;
		}
		else
		{
			
			chat._chat( self, null, chat.CHAT_SAY, chat.MOOD_CHEERFUL, null, new string_id( "player_structure", "wear_no"), null );
			doAnimationAction( self, "slow_down");
			return SCRIPT_CONTINUE;
		}
	}
	
	
	public int handleVendorAnimSelect(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id player = sui.getPlayerId(params);
		int idx = sui.getListboxSelectedRow( params );
		if (idx < 0)
		{
			idx = 0;
		}
		int btn = sui.getIntButtonPressed( params );
		if (btn == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] rawAnims = dataTableGetStringColumnNoDefaults( TBL_VENDOR_ANIMS, 0 );
		setObjVar( self, "vendor.barkAnim", rawAnims[idx] );
		
		String[] rawMoods = dataTableGetStringColumnNoDefaults( TBL_VENDOR_MOODS, 0 );
		String[] moods = new String[rawMoods.length];
		for (int i=0; i<rawMoods.length; i++)
		{
			testAbortScript();
			moods[i] = "@player_structure:"+rawMoods[i];
		}
		sui.listbox( self, player, "@player_structure:vendor_moods_d", sui.OK_CANCEL, "@player_structure:vendor_moods_t", moods, "handleVendorMoodSelect", true );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleVendorMoodSelect(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id player = sui.getPlayerId(params);
		int idx = sui.getListboxSelectedRow( params );
		if (idx < 0)
		{
			idx = 0;
		}
		int btn = sui.getIntButtonPressed( params );
		if (btn == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] rawMoods = dataTableGetStringColumnNoDefaults( TBL_VENDOR_MOODS, 0 );
		setObjVar( self, "vendor.barkMood", rawMoods[idx] );
		
		String[] rawStrCats = dataTableGetStringColumnNoDefaults( TBL_VENDOR_STRCATS, 0 );
		Vector strcats = null;
		for (int i=0; i<rawStrCats.length; i++)
		{
			testAbortScript();
			strcats = utils.addElement( strcats, "@player_structure:subcat_"+rawStrCats[i] );
		}
		
		if (utils.isProfession( player, utils.TRADER ))
		{
			strcats = utils.addElement( strcats, "@player_structure:custom");
		}
		sui.listbox( self, player, "@player_structure:vendor_strcats_d", sui.OK_CANCEL, "@player_structure:vendor_strcats_t", strcats, "handleVendorStrCatSelect", true );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleVendorStrCatSelect(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id player = sui.getPlayerId(params);
		int idx = sui.getListboxSelectedRow( params );
		if (idx < 0)
		{
			idx = 0;
		}
		int btn = sui.getIntButtonPressed( params );
		if (btn == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] rawStrCatsArray = dataTableGetStringColumnNoDefaults( TBL_VENDOR_STRCATS, 0 );
		Vector rawStrCats = new Vector(Arrays.asList(rawStrCatsArray));if (utils.isProfession(player, utils.TRADER))
		{
			rawStrCats = utils.addElement( rawStrCats, "custom");
		}
		setObjVar( self, "vendor.barkStrCat", ((String)(rawStrCats.get(idx))) );
		
		if (((String)(rawStrCats.get(idx))) == "custom")
		{
			
			sui.inputbox( self, player, "@player_structure:cust_d", sui.OK_CANCEL, "@player_structure:cust_t", sui.INPUT_NORMAL, null, "handleSetCustomBark", null );
			return SCRIPT_CONTINUE;
		}
		
		removeObjVar( self, "vendor.customBark");
		setObjVar( self, "vendor.areaBarks", 1 );
		createTriggerVolume( ALERT_VOLUME_NAME, 10, true );
		
		sendSystemMessage( player, SID_AREABARKS_ENABLED );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSetCustomBark(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id player = sui.getPlayerId(params);
		
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		String customBark = sui.getInputBoxText( params );
		int btn = sui.getIntButtonPressed( params );
		if (btn == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		if ((customBark.equals("")) || !isAppropriateText(customBark))
		{
			
			sendSystemMessage( player, SID_OBSCENE );
			sui.inputbox( self, player, "@player_structure:cust_d", sui.OK_CANCEL, "@player_structure:cust_t", sui.INPUT_NORMAL, null, "handleSetCustomBark", null );
			return SCRIPT_CONTINUE;
		}
		
		if (customBark.length() > 140)
		{
			customBark = customBark.substring( 0, 139 );
		}
		setObjVar( self, "vendor.customBark", customBark );
		
		setObjVar( self, "vendor.areaBarks", 1 );
		createTriggerVolume( ALERT_VOLUME_NAME, 10, true );
		
		sendSystemMessage( player, SID_AREABARKS_ENABLED );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTriggerVolumeEntered(obj_id self, String volumeName, obj_id breacher) throws InterruptedException
	{
		if (hasObjVar( breacher, "gm"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (breacher == self)
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (isIncapacitated(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!isMob( breacher ))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (ai_lib.isMonster( breacher ))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!hasObjVar( self, "vendor.areaBarks" ))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (utils.hasScriptVar( self, "bark.alreadyBarked" ))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (rand( 1, 3 ) == 1)
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (volumeName.equals(ALERT_VOLUME_NAME))
		{
			
			String anim = getStringObjVar( self, "vendor.barkAnim");
			String mood = getStringObjVar( self, "vendor.barkMood");
			String strcat = getStringObjVar( self, "vendor.barkStrCat");
			String custbark = getStringObjVar( self, "vendor.customBark");
			
			faceTo( self, breacher );
			if (custbark != null)
			{
				chat._chat( self, breacher, chat.CHAT_SAY, mood, custbark, null, null );
			}
			else
			{
				prose_package pp = prose.getPackage( new string_id( "player_structure", "areabark_"+strcat ), breacher );
				chat.publicChat( self, breacher, chat.CHAT_SAY, mood, pp );
			}
			doAnimationAction( self, anim );
			
			utils.setScriptVar( self, "bark.alreadyBarked", 1 );
			messageTo( self, "resetBarkTimer", null, 60, false );
			
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int resetBarkTimer(obj_id self, dictionary params) throws InterruptedException
	{
		utils.removeScriptVar( self, "bark.alreadyBarked");
		return SCRIPT_CONTINUE;
	}
	
	
	public int checkNakedSlots(obj_id self, dictionary params) throws InterruptedException
	{
		String templateName = getTemplateName(self);
		if (templateName == null || templateName.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (templateName.indexOf("wookiee") > -1)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, "vendor.special_vendor_clothing") && getIntObjVar(self, "vendor.special_vendor_clothing") == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		boolean ithorian = false;
		
		if (templateName.indexOf("ithorian") > -1)
		{
			blog("I am an Ithorian");
			ithorian = true;
		}
		
		blog("OnGiveItem FORCING CLOTHING");
		
		obj_id currentChest = getObjectInSlot(self, slots.CHEST1);
		obj_id currentPants = getObjectInSlot(self, slots.PANTS1);
		obj_id currentShoes = getObjectInSlot(self, slots.SHOES);
		
		if (!isValidId(currentChest))
		{
			blog("OnGiveItem NO CHEST");
			
			if (ithorian)
			{
				createObject(ITH_VENDOR_SHIRT, self, slots.CHEST1);
			}
			else
			{
				createObject(STANDARD_VENDOR_SHIRT, self, slots.CHEST1);
			}
			
		}
		if (!isValidId(currentPants))
		{
			blog("OnGiveItem NO PANTS");
			
			if (ithorian)
			{
				createObject(ITH_VENDOR_PANTS, self, slots.PANTS1);
			}
			else
			{
				createObject(STANDARD_VENDOR_PANTS, self, slots.PANTS1);
			}
			
		}
		
		if (!isValidId(currentShoes) && !ithorian)
		{
			blog("OnGiveItem NO SHOES");
			
			createObject(STANDARD_VENDOR_SHOES, self, slots.SHOES);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean blog(String msg) throws InterruptedException
	{
		if (LOGGING_ON && msg != null && !msg.equals(""))
		{
			LOG(LOGGING_CATEGORY, msg);
		}
		return true;
	}
	
}
