package script.creature;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.utils;
import script.library.dot;
import script.library.buff;


public class sarlacc extends script.base_script
{
	public sarlacc()
	{
	}
	public static final string_id SID_EXTRACT_BILE = new string_id("mob/sarlacc", "extract_bile");
	public static final string_id SID_BILE_FAIL = new string_id("mob/sarlacc", "bile_fail");
	public static final string_id SID_BILE_SUCCESS = new string_id("mob/sarlacc", "bile_success");
	public static final string_id SID_BILE_ALREADY = new string_id("mob/sarlacc", "bile_already");
	public static final string_id SID_SARLACC_ERUPT = new string_id("mob/sarlacc", "sarlacc_erupt");
	public static final string_id SID_SARLACC_POISON = new string_id("mob/sarlacc", "sarlacc_poison");
	public static final string_id SID_SARLACC_DOT = new string_id("mob/sarlacc", "sarlacc_dot");
	
	public static final String BILE_OBJECT = "object/tangible/loot/quest/quest_item_sarlacc_bile_jar.iff";
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		if (!hasTriggerVolume( self, "sarlaccBreach" ))
		{
			createTriggerVolume( "sarlaccBreach", 30.f, true );
		}
		if (!hasTriggerVolume( self, "sarlaccPreBreach" ))
		{
			createTriggerVolume( "sarlaccPreBreach", 60.f, true );
		}
		
		messageTo( self, "poisonArea", null, 300, false );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTriggerVolumeEntered(obj_id self, String volumeName, obj_id whoTriggeredMe) throws InterruptedException
	{
		
		sendSystemMessageTestingOnly(whoTriggeredMe, "triggerend");
		
		if (volumeName.equals("sarlaccBreach"))
		{
			
			if (!isPlayer( whoTriggeredMe ))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (!utils.hasScriptVar( self, "bile.eject" ))
			{
				playClientEffectLoc( whoTriggeredMe, "clienteffect/cr_sarlacc_erupt.cef", getLocation(self), 3f );
				
				obj_id[] contents = getTriggerVolumeContents( self, "sarlaccBreach");
				if (contents != null)
				{
					for (int i=0; i<contents.length; i++)
					{
						testAbortScript();
						if (isPlayer( contents[i] ))
						{
							sendSystemMessage( contents[i], SID_SARLACC_ERUPT );
						}
					}
				}
			}
			
			utils.setScriptVar( self, "bile.eject", 1 );
			messageTo( self, "resetBileEject", null, 10.f, false );
			
			int diseaseResist = getEnhancedSkillStatisticModifierUncapped(whoTriggeredMe, "resistance_disease");
			
			if (diseaseResist < 24)
			{
				if (buff.applyBuff(whoTriggeredMe, "sarlaccSnare", 300))
				{;
				}
				{
					sendSystemMessage( whoTriggeredMe, SID_SARLACC_DOT );
				}
			}
		}
		else if (volumeName.equals("sarlaccPreBreach"))
		{
			
			if (!isPlayer( whoTriggeredMe ))
			{
				return SCRIPT_CONTINUE;
			}
			
			sendSystemMessage( whoTriggeredMe, SID_SARLACC_POISON );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int resetBileEject(obj_id self, dictionary params) throws InterruptedException
	{
		utils.removeScriptVar( self, "bile.eject");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int poisonArea(obj_id self, dictionary params) throws InterruptedException
	{
		
		int playerHint = 0;
		if (!utils.hasScriptVar( self, "bile.eject" ))
		{
			int bile = 0;
			obj_id[] contents = getTriggerVolumeContents( self, "sarlaccBreach");
			if (contents != null)
			{
				for (int i=0; i<contents.length; i++)
				{
					testAbortScript();
					if (isPlayer( contents[i] ))
					{
						playerHint = 1;
						if (bile == 0)
						{
							bile = 1;
							playClientEffectLoc( contents[i], "clienteffect/cr_sarlacc_erupt.cef", getLocation(self), 3f );
						}
						
						int diseaseResist = getEnhancedSkillStatisticModifierUncapped(contents[i], "resistance_disease");
						
						if (diseaseResist < 24)
						{
							if (buff.applyBuff(contents[i], "sarlaccSnare", 300))
							{;
							}
							{
								sendSystemMessage( contents[i], SID_SARLACC_DOT );
							}
						}
						sendSystemMessage( contents[i], SID_SARLACC_ERUPT );
					}
				}
			}
		}
		
		if (playerHint == 1)
		{
			messageTo( self, "poisonArea", null, 60+rand(60), false );
		}
		else
		{
			messageTo( self, "poisonArea", null, 120, false );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		mi.addRootMenu( menu_info_types.SERVER_MENU1, SID_EXTRACT_BILE );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		
		if (item == menu_info_types.SERVER_MENU1)
		{
			
			obj_id[] items = getInventoryAndEquipment( player );
			if (items != null)
			{
				for (int i=0; i<items.length; i++)
				{
					testAbortScript();
					if (getTemplateName( items[i] ).compareTo( BILE_OBJECT ) == 0)
					{
						sendSystemMessage( player, SID_BILE_ALREADY );
						return SCRIPT_CONTINUE;
					}
				}
			}
			
			if (utils.hasScriptVar( self, "bile.taken" ))
			{
				
				sendSystemMessage( player, SID_BILE_FAIL );
				return SCRIPT_CONTINUE;
			}
			
			sendSystemMessage( player, SID_BILE_SUCCESS );
			obj_id pInv = utils.getInventoryContainer( player );
			if (isIdValid( pInv ))
			{
				createObject( BILE_OBJECT, pInv, "");
			}
			
			utils.setScriptVar( self, "bile.taken", 1 );
			messageTo( self, "resetBile", null, 300, false );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int resetBile(obj_id self, dictionary params) throws InterruptedException
	{
		utils.removeScriptVar( self, "bile.taken");
		
		return SCRIPT_CONTINUE;
	}
}
