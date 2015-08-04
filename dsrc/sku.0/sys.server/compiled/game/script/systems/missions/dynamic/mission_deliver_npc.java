package script.systems.missions.dynamic;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.utils;
import script.library.ai_lib;
import script.library.chat;
import script.library.factions;
import script.library.anims;
import script.library.missions;



public class mission_deliver_npc extends script.systems.missions.base.mission_dynamic_base
{
	public mission_deliver_npc()
	{
	}
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		attachScript(self, "npc.converse.npc_converse_menu");
		debugServerConsoleMsg(self, "deliver dropoff script attached");
		factions.setFaction(self, "Unattackable");
		ai_lib.setDefaultCalmBehavior(self, ai_lib.BEHAVIOR_SENTINEL);
		clearCondition(self, CONDITION_HOLIDAY_INTERESTING);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		if (!hasScript (self, "systems.missions.base.mission_cleanup_tracker"))
		{
			attachScript (self, "systems.missions.base.mission_cleanup_tracker");
		}
		clearCondition(self, CONDITION_HOLIDAY_INTERESTING);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnStartNpcConversation(obj_id self, obj_id speaker) throws InterruptedException
	{
		boolean boolDebug = false;
		if (hasObjVar(speaker, "verboseDebug"))
		{
			boolDebug = true;
		}
		location here = getLocation (self);
		obj_id[] missionList = getMissionObjects (speaker);
		obj_id missionInfo = null;
		location missionLoc = new location();
		
		if (missionList == null)
		{
			if (boolDebug)
			{
				sendSystemMessageTestingOnly(speaker, "Error 1, Mission list is null");
			}
			doIncorrectPlayerBlurb(self);
			return SCRIPT_CONTINUE;
		}
		
		int numMissions = missionList.length;
		if (boolDebug)
		{
			sendSystemMessageTestingOnly(speaker, "NumMission is "+numMissions);
		}
		
		if (numMissions == 1)
		{
			obj_id onlyMission = missionList[0];
			if (boolDebug)
			{
				sendSystemMessageTestingOnly(speaker, "Since we're one, onlyMission is "+onlyMission);
			}
			
			int missionState = getIntObjVar(onlyMission, "intState");
			if (missionState == missions.STATE_DYNAMIC_PICKUP)
			{
				
				missionLoc = getMissionStartLocation (onlyMission);
				if (boolDebug)
				{
					sendSystemMessageTestingOnly(speaker, "State is pickup, using start location of value "+missionLoc);
				}
				
			}
			else if (missionState == missions.STATE_DYNAMIC_DROPOFF)
			{
				missionLoc = getMissionEndLocation (onlyMission);
				if (boolDebug)
				{
					sendSystemMessageTestingOnly(speaker, "State is dropoff, using end location of value "+missionLoc);
				}
			}
			else
			{
				if (boolDebug)
				{
					sendSystemMessageTestingOnly(speaker, "Fucked up missions, no nothing");
				}
				
				debugServerConsoleMsg (speaker, "IT's all messed up, the mission State was totally wrong");
				return SCRIPT_CONTINUE;
			}
			
			float dist = utils.getDistance2D (here, missionLoc);
			if (boolDebug)
			{
				sendSystemMessageTestingOnly(speaker, "using here as test value. Here is "+here);
			}
			
			if ((dist > 15.0f) || (dist < 0))
			{
				if (boolDebug)
				{
					sendSystemMessageTestingOnly(speaker, "Distance is "+dist+" and erroring out");
				}
				
				doIncorrectPlayerBlurb(self);
				return SCRIPT_CONTINUE;
			}
			missionInfo = onlyMission;
		}
		else if (numMissions == 2)
		{
			if (boolDebug)
			{
				sendSystemMessageTestingOnly(speaker, "numMissions is 2, doing a loop");
			}
			boolean rightSpot = false;
			for (int x = 0; x < numMissions; x ++)
			{
				testAbortScript();
				
				obj_id thisMission = missionList[x];
				if (boolDebug)
				{
					sendSystemMessageTestingOnly(speaker, "Iterating, x is "+x+ " and misison si "+thisMission);
				}
				
				int currentState = getIntObjVar (thisMission, "intState");
				
				if (currentState == missions.STATE_DYNAMIC_PICKUP)
				{
					missionLoc = getMissionStartLocation (thisMission);
					if (boolDebug)
					{
						sendSystemMessageTestingOnly(speaker, "State is pickup, using start location of value "+missionLoc);
					}
				}
				else if (currentState == missions.STATE_DYNAMIC_DROPOFF)
				{
					missionLoc = getMissionEndLocation (thisMission);
					if (boolDebug)
					{
						sendSystemMessageTestingOnly(speaker, "State is dropoff, using end location of value "+missionLoc);
					}
				}
				
				float dist2 = utils.getDistance2D (here, missionLoc);
				if (boolDebug)
				{
					sendSystemMessageTestingOnly(speaker, "dist2 is "+dist2);
					sendSystemMessageTestingOnly(speaker, "Location 1(here) is "+here);
					sendSystemMessageTestingOnly(speaker, "Location 2(missionLoc) is "+missionLoc);
				}
				
				if ((dist2 < 15.0f) && (dist2 >= 0))
				{
					if (boolDebug)
					{
						sendSystemMessageTestingOnly(speaker, "Distance 2 is "+dist2+" and setting rightspot to true");
					}
					
					rightSpot = true;
					setObjVar (self, "startLoc"+ x, missionLoc);
					missionInfo = thisMission;
				}
			}
			
			if (rightSpot != true)
			{
				if (boolDebug)
				{
					sendSystemMessageTestingOnly(speaker, "rightSpot is false after our loop, erroring out");
				}
				
				doIncorrectPlayerBlurb (self);
				return SCRIPT_CONTINUE;
			}
			
			if (hasObjVar (self, "startLoc1") && hasObjVar (self, "startLoc2"))
			{
				
				if (boolDebug)
				{
					sendSystemMessageTestingOnly(speaker, "sameSpot check");
				}
				
				location one = getLocationObjVar (self, "startLoc1");
				location two = getLocationObjVar (self, "startLoc2");
				if (one.equals(two))
				{
					if (boolDebug)
					{
						sendSystemMessageTestingOnly(speaker, "sameSpot is true");
					}
					boolean sameSpot = true;
				}
			}
		}
		
		int intState = getIntObjVar(missionInfo, "intState");
		
		if (intState == missions.STATE_DYNAMIC_DROPOFF)
		{
			String missionType = getMissionType (missionInfo);
			obj_id objMissionData = getMissionData(missionInfo);
			int intStringId = getIntObjVar(objMissionData, "intStringId");
			String strIdFileName = getStringObjVar(objMissionData, "strIdFileName");
			string_id strConvo = new string_id(strIdFileName, "m"+intStringId+"r");
			
			if (missionType.equals("deliver"))
			{
				
				setObjVar(objMissionData, "strIdFileName", strIdFileName);
				chat.chat(self, strConvo);
				
				dictionary dctParams = new dictionary();
				messageTo(missionInfo, "pickup_event", dctParams, 0, true);
				sendDeliverSuccess(missionInfo);
			}
			else if (missionType.equals("crafting"))
			{
				String strItemToMake = getStringObjVar(objMissionData, "strItemToMake");
				if (hasCraftingMissionItem(speaker, strItemToMake))
				{
					revokeMissionSchematic(speaker, missionInfo);
					setObjVar(objMissionData, "strIdFileName", strIdFileName);
					
					String strSpeakString = getString(strConvo);
					chat.chat(self, strSpeakString);
					
					dictionary dctParams = new dictionary();
					sendCraftingSuccess(missionInfo);
					return SCRIPT_OVERRIDE;
				}
				else
				{
					string_id strSpam = new string_id("mission/mission_generic", "wrong_item");
					chat.chat(self, strSpam);
					return SCRIPT_CONTINUE;
				}
			}
			else
			{
				string_id message = new string_id ("mission/mission_generic", "crafting_already_dropped_off");
				chat.chat(self, message);
				doAnimationAction(self, anims.PLAYER_WAVE_ON_DISMISSING);
			}
		}
		
		else if (intState == missions.STATE_DYNAMIC_PICKUP)
		{
			String missionType = getMissionType (missionInfo);
			if (missionType.equals("deliver"))
			{
				obj_id objMissionData = getMissionData(missionInfo);
				int intStringId = getIntObjVar(objMissionData, "intStringId");
				String strIdFileName = getStringObjVar(objMissionData, "strIdFileName");
				string_id strConvo = new string_id(strIdFileName, "m"+intStringId+"p");
				setObjVar(objMissionData, "strIdFileName", strIdFileName);
				chat.chat(self, strConvo);
				
				string_id strTitleId = new string_id("mission/mission_generic", "deliver_received_data");
				string_id strMissionTitleId = new string_id(strIdFileName, "m"+intStringId+"t");
				String strMissionTitle = getString(strMissionTitleId);
				String strSystemMessage = getString(strTitleId);
				strSystemMessage = strSystemMessage + " "+strMissionTitle;
				sendSystemMessageTestingOnly(speaker, strSystemMessage);
				
				dictionary dctParams = new dictionary();
				messageTo(missionInfo, "pickup_event", dctParams, 0, true);
				
				setObjVar(missionInfo, "intState", missions.STATE_DYNAMIC_DROPOFF);
				location locDropoffLocation = getMissionEndLocation(objMissionData);
				
				debugServerConsoleMsg(self, "objMissionData for npc pickup is "+objMissionData);
				
				if (hasObjVar(objMissionData, "objEndNPC"))
				{
					setObjVar(missionInfo, "intMissionComplete", 1);
				}
				else
				{
					setupSpawn(missionInfo, locDropoffLocation);
				}
				updateMissionWaypoint(missionInfo, locDropoffLocation);
			}
			if (missionType.equals("crafting"))
			{
				grantMissionSchematic(speaker, missionInfo);
				obj_id missionInfoData = getMissionData(missionInfo);
				String[] strComponents = getStringArrayObjVar(missionInfoData, "strComponents");
				int intI = 0;
				obj_id objInventory = utils.getInventoryContainer(speaker);
				while (intI < strComponents.length)
				{
					testAbortScript();
					obj_id objComponent = createObjectOverloaded(strComponents[intI], objInventory);
					setCraftedId(objComponent, objComponent);
					intI = intI+1;
					
				}
				setObjVar(missionInfo, "intState", missions.STATE_DYNAMIC_DROPOFF);
				int intStringId = getIntObjVar(missionInfoData, "intStringId");
				String strIdFileName = getStringObjVar(missionInfoData, "strIdFileName");
				string_id strConvo = new string_id(strIdFileName, "m"+intStringId+"p");
				setObjVar(missionInfoData, "strIdFileName", strIdFileName);
				String strSpeakString = getString(strConvo);
				chat.chat(self, strSpeakString);
				string_id strSpam = new string_id("mission/mission_generic", "crafting_components_received");
				sendSystemMessage(speaker, strSpam);
				dictionary dctParams = new dictionary();
				messageTo(missionInfo, "pickup_event", null, 0, true);
				location locDropoffLocation = getMissionEndLocation(missionInfoData);
				setupSpawn(missionInfo, locDropoffLocation);
				updateMissionWaypoint(missionInfo, locDropoffLocation);
			}
		}
		else
		{
			string_id message = new string_id ("mission/mission_generic", "deliver_already_dropped_off");
			chat.chat(self, message);
			doAnimationAction(self, anims.PLAYER_WAVE_ON_DISMISSING);
			return SCRIPT_CONTINUE;
		}
		return SCRIPT_CONTINUE;
	}
	
}
