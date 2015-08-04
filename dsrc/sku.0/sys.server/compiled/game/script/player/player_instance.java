package script.player;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.badge;
import script.library.buff;
import script.library.callable;
import script.library.factions;
import script.library.group;
import script.library.instance;
import script.library.locations;
import script.library.prose;
import script.library.skill;
import script.library.static_item;
import script.library.sui;
import script.library.trial;
import script.library.utils;


public class player_instance extends script.base_script
{
	public player_instance()
	{
	}
	
	public int OnHearSpeech(obj_id self, obj_id speaker, String text) throws InterruptedException
	{
		if (!isGod(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] parse = split(text, ' ');
		
		if (parse.length < 2)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (parse[0].equals("instanceRequest") && speaker == self)
		{
			if (parse.length == 3)
			{
				instance.requestInstanceMovement(self, parse[1], parse[2]);
			}
			else if (parse.length == 4)
			{
				instance.requestInstanceMovement(self, parse[1], utils.stringToInt(parse[2]), parse[3]);
			}
			else
			{
				instance.requestInstanceMovement(self, parse[1]);
			}
		}
		
		if (parse[0].equals("closeInstance") && speaker == self)
		{
			instance.closeOwnedInstance(self, parse[1]);
		}
		
		if (parse[0].equals("exitInstance") && speaker == self)
		{
			instance.requestExitPlayer(parse[1], self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnUnsticking(obj_id self) throws InterruptedException
	{
		obj_id instanceController = instance.getAreaInstanceController(self);
		if (hasScript(self, "npe.trigger_journal") && hasObjVar(self, "npe"))
		{
			location myLoc = getLocation(self);
			if (myLoc.area.equals("dungeon1"))
			{
				String room = getCellName(myLoc.cell);
				if (room.equals("cave_entrance"))
				{
					warpPlayer(self, "dungeon1", 0.0f, 0.0f, 0.0f, myLoc.cell, 7.10f, 9.5f, 6.5f);
					return SCRIPT_OVERRIDE;
				}
			}
		}
		
		if (isIdValid(instanceController))
		{
			String instanceName = instance.getInstanceName(instanceController);
			instance.requestExitPlayer(instanceName, self);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDuelRequest(obj_id self, obj_id actor, obj_id target) throws InterruptedException
	{
		if (instance.isInInstanceArea(actor) || instance.isInInstanceArea(target))
		{
			sendConsoleMessage(actor, "Dueling is not permitted here");
			sendConsoleMessage(target, "Dueling is not permitted here");
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnLogin(obj_id self) throws InterruptedException
	{
		String area_name = locations.getBuildoutAreaName(self);
		int area_row = locations.getBuildoutAreaRow(self);
		
		obj_id planet = getPlanetByName(getLocation(self).area);
		String instance_controller = ""+area_name+"_"+area_row;
		
		obj_id areaInstanceController = utils.hasScriptVar(planet, instance_controller) ? utils.getObjIdScriptVar(planet, instance_controller) : null;
		
		if (!instance.isInInstanceArea(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(areaInstanceController))
		{
			doLogging("OnLogin", "Could not locate instance controller");
			return SCRIPT_CONTINUE;
		}
		
		dictionary dict = new dictionary();
		dict.put("player", self);
		
		messageTo(areaInstanceController, "validatePlayer", dict, 1.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnClusterWideDataResponse(obj_id self, String manage_name, String name, int request_id, String[] element_name_list, dictionary[] data, int lock_key) throws InterruptedException
	{
		CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-player "+self+"("+getPlayerName(self)+")recieved trigger");
		int requestType = instance.getInstanceRequestType(self);
		CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-player "+self+"("+getPlayerName(self)+")'s requestType is "+ requestType);
		int team = instance.getInstanceRequestTeam(self);
		CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-player "+self+"("+getPlayerName(self)+")'s team is "+ team);
		
		if (requestType == instance.REQUEST_MOVETO && team != instance.REQUEST_INVALID)
		{
			CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-player "+self+"("+getPlayerName(self)+")'s requestType was "+instance.REQUEST_MOVETO+" and team was not "+instance.REQUEST_INVALID);
			if (data == null || data.length == 0 || !manage_name.startsWith("instance_manager"))
			{
				CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-player "+self+"("+getPlayerName(self)+")'s dictionary 'data' was null, or manage name didnt start with 'instance_manager'. Releasing lock and dieing here.");
				CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-player "+self+"("+getPlayerName(self)+")'s manage_name was "+manage_name+" and instance name was "+name+".");
				doLogging("Insufficient Data. Manage name = "+manage_name +", instance name = "+name);
				releaseClusterWideDataLock(manage_name, lock_key);
				instance.setRequestResolved(self);
				return SCRIPT_CONTINUE;
			}
			int emptyInstance = -1;
			String instance_name = "";
			
			CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-about to start looping thru player "+self+"("+getPlayerName(self)+")'s dictionary 'data' passed from the trigger.");
			for (int i=0; i<data.length; i++)
			{
				testAbortScript();
				dictionary dict = data[i];
				obj_id instance_id = dict.getObjId("instance_id");
				CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-Dictionary "+i+"'s instance_id for player "+self+"("+getPlayerName(self)+") is "+instance_id+".");
				instance_name = dict.getString("instance_name");
				CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-Dictionary "+i+"'s instance_name for player "+self+"("+getPlayerName(self)+") is "+instance_name+".");
				obj_id owner = dict.getObjId("owner");
				CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-Dictionary "+i+"'s owner for player "+self+"("+getPlayerName(self)+") is "+owner+".");
				int instance_team = dict.getInt("team");
				CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-Dictionary "+i+"'s team for player "+self+"("+getPlayerName(self)+") is "+team+".");
				
				String groupList = "none";
				if (team == 1)
				{
					groupList = dict.getString("groupOne");
					CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-Dictionary "+i+"'s groupList for player "+self+"("+getPlayerName(self)+") is "+groupList+".");
				}
				
				if (team == 2)
				{
					groupList = dict.getString("groupTwo");
					CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-Dictionary "+i+"'s groupList for player "+self+"("+getPlayerName(self)+") is "+groupList+".");
				}
				
				boolean isInstanceGroupValidated = instance.isPlayerOwnerGroupList(self, owner, instance_id, groupList, team, instance_team);
				CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-Dictionary "+i+"'s isInstanceGroupValidated for player "+self+"("+getPlayerName(self)+") is "+isInstanceGroupValidated+".");
				
				if (isInstanceGroupValidated)
				{
					doLogging("Found instance, initializing the move");
					CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-Dictionary "+i+"'s isInstanceGroupValidated for player "+self+"("+getPlayerName(self)+") was true, so we are going to stop here.");
					if (instance.isValidatedByLockoutData(self, instance_id, owner, instance_name))
					{
						CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-calling movePlayerToInstance("+self+", "+instance_id+", "+instance_name+", "+owner+", false, "+team+") for player "+self+"("+getPlayerName(self)+").");
						instance.movePlayerToInstance(self, instance_id, instance_name, owner, false, team);
						CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-player "+self+"("+getPlayerName(self)+") is being sent to instance. We now need to add them to the player list, if they are not in it already.");
						if (!instance.isPlayerInPlayerList(self, groupList))
						{
							CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-player "+self+"("+getPlayerName(self)+") was not in group list. Adding now.");
							
							String newList = "";
							
							if (groupList.equals("none"))
							{
								newList = ""+self;
							}
							else
							{
								newList = groupList.concat("_"+self);
							}
							
							CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-player "+self+"("+getPlayerName(self)+")'s new groupList is "+ newList);
							if (team == 1)
							{
								CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-player "+self+"("+getPlayerName(self)+") assigning new list to group1");
								dict.put("groupOne", newList);
							}
							if (team == 2)
							{
								CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-player "+self+"("+getPlayerName(self)+") assigning new list to group2");
								dict.put("groupTwo", newList);
							}
						}
						CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-player "+self+"("+getPlayerName(self)+") replacing clusterwide data");
						name = instance_name+"_"+instance_id;
						
						replaceClusterWideData(manage_name, name, dict, true, lock_key);
						CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-player "+self+"("+getPlayerName(self)+") releasing clusterwide data");
						releaseClusterWideDataLock(manage_name, lock_key);
						instance.setRequestResolved(self);
						return SCRIPT_CONTINUE;
						
					}
					else
					{
						CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "movePlayerToInstance-isValidatedByLockoutData returned false for player "+getFirstName(self)+"("+self+"). Sending messageTo 'instanceFailureMessage'");
						dict.put("fail_reason", instance.FAIL_INVALID_LOCKOUT);
						messageTo(self, "instanceFailureMessage", dict, 0.0f, false);
						releaseClusterWideDataLock(manage_name, lock_key);
						instance.setRequestResolved(self);
						return SCRIPT_CONTINUE;
					}
				}
				
				if ((instance_id == owner) && emptyInstance == -1)
				{
					emptyInstance = i;
				}
				
			}
			
			dictionary lockoutData = instance.getLockoutData(self, instance_name);
			
			if (lockoutData != null)
			{
				String strName = "@"+new string_id("instance", instance_name);
				prose_package pp = new prose_package();
				pp.other.set(strName);
				pp.stringId = new string_id("instance", "lockout_notification");
				sendSystemMessageProse(self, pp);
			}
			else if (emptyInstance > -1)
			{
				CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-we had an empty instance ("+data[emptyInstance]+") for player "+self+"("+getPlayerName(self)+").");
				dictionary newDict = data[emptyInstance];
				CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-calling movePlayerToInstance("+self+", "+newDict.getObjId("instance_id")+", "+newDict.getString("instance_name")+", "+newDict.getObjId("owner")+", true, "+team+") for player "+self+"("+getPlayerName(self)+").");
				instance.movePlayerToInstance(self, newDict.getObjId("instance_id"), newDict.getString("instance_name"), newDict.getObjId("owner"), true, team);
				newDict.put("owner", self);
				newDict.put("team", team);
				String strName = newDict.getString("instance_name");
				obj_id objName = newDict.getObjId("instance_id");
				name = strName+"_"+objName;
				String groupList = ""+self;
				
				CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-player "+self+"("+getPlayerName(self)+")'s new groupList is "+ groupList);
				if (team == 1)
				{
					CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-player "+self+"("+getPlayerName(self)+") assigning new list to group1");
					newDict.put("groupOne", groupList);
				}
				if (team == 2)
				{
					CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-player "+self+"("+getPlayerName(self)+") assigning new list to group2");
					newDict.put("groupTwo", groupList);
				}
				CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-calling replaceClusterWideData("+manage_name+", "+name+", "+newDict+", true, "+lock_key+") for player "+self+"("+getPlayerName(self)+").");
				replaceClusterWideData(manage_name, name, newDict, true, lock_key);
				releaseClusterWideDataLock(manage_name, lock_key);
				instance.setRequestResolved(self);
				return SCRIPT_CONTINUE;
			}
			else
			{
				sendSystemMessage(self, new string_id("instance", "all_full"));
			}
		}
		
		if (requestType == instance.REQUEST_CLOSE)
		{
			CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-player "+self+"("+getPlayerName(self)+")'s requestType was "+instance.REQUEST_CLOSE+".");
			if (data == null || data.length == 0 || !manage_name.startsWith("instance_manager"))
			{
				CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-player "+self+"("+getPlayerName(self)+")'s dictionary 'data' was null, or manage name didnt start with 'instance_manager'. Releasing lock and dieing here.");
				doLogging("Insufficient Data. Manage name = "+manage_name +", instance name = "+name);
				releaseClusterWideDataLock(manage_name, lock_key);
				instance.setRequestResolved(self);
				return SCRIPT_CONTINUE;
			}
			
			CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-about to start looping thru player "+self+"("+getPlayerName(self)+")'s dictionary 'data' passed from the trigger.");
			for (int i=0; i<data.length; i++)
			{
				testAbortScript();
				dictionary dict = data[i];
				obj_id instance_id = dict.getObjId("instance_id");
				CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-Dictionary "+i+"'s instance_id for player "+self+"("+getPlayerName(self)+") is "+instance_id+".");
				String instance_name = dict.getString("instance_name");
				CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-Dictionary "+i+"'s instance_name for player "+self+"("+getPlayerName(self)+") is "+instance_name+".");
				obj_id owner = dict.getObjId("owner");
				CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-Dictionary "+i+"'s owner for player "+self+"("+getPlayerName(self)+") is "+owner+".");
				String groupOne = dict.getString("groupOne");
				CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-Dictionary "+i+"'s groupOne for player "+self+"("+getPlayerName(self)+") is "+groupOne+".");
				String groupTwo = dict.getString("groupTwo");
				CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-Dictionary "+i+"'s groupTwo for player "+self+"("+getPlayerName(self)+") is "+groupTwo+".");
				
				if (isIdValid(owner) && owner == self)
				{
					CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "OnClusterWideDataResponse-Dictionary "+i+"'s found owned instance for player "+self+"("+getPlayerName(self)+"). Closing Instance");
					doLogging("Found owned instance: Player list = "+groupOne);
					instance.closeInstance(self, dict);
					releaseClusterWideDataLock(manage_name, lock_key);
					instance.setRequestResolved(self);
					return SCRIPT_CONTINUE;
				}
			}
		}
		
		releaseClusterWideDataLock(manage_name, lock_key);
		instance.setRequestResolved(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int movePlayerToInstance(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "movePlayerToInstance-Params was empty, we are bailing out to avoid exceptions.");
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "movePlayerToInstance-MessageHandler starts here for player "+self+"("+getPlayerName(self)+").");
		
		location instanceLoc = params.getLocation("location");
		obj_id instance_id = params.getObjId("instance_id");
		String instance_name = params.getString("instance_name");
		obj_id owner = params.getObjId("owner");
		int start_time = params.getInt("start_time");
		int join_code = params.getInt("join_code");
		int team = params.getInt("team");
		
		utils.removeScriptVar(self, instance.REQUEST_TYPE);
		utils.removeScriptVar(self, instance.REQUEST_TEAM);
		utils.removeScriptVar(self, instance.REQUEST_TRIGGER);
		
		if (!dataTableOpen(instance.INSTANCE_DATATABLE))
		{
			doLogging("Could not open table");
			CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "movePlayerToInstance-datatable "+instance.INSTANCE_DATATABLE+" couldnt be opened exiting scripts");
			return SCRIPT_CONTINUE;
		}
		dictionary data = dataTableGetRow(instance.INSTANCE_DATATABLE, instance_name);
		
		if (data == null)
		{
			doLogging("null data in table");
			CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "movePlayerToInstance-dataTableGetRow("+instance.INSTANCE_DATATABLE+", "+instance_name+") returned null, exiting.");
			return SCRIPT_CONTINUE;
		}
		
		instance.attachInstanceScriptsOnPlayer(self, data);
		
		if (instance.isExclusiveInstance(instance_name))
		{
			CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "movePlayerToInstance-isExclusiveInstance returned true, calling setResetDataOnPlayer for player "+self+"("+getPlayerName(self)+").");
			instance.setResetDataOnPlayer(self, instance_id, owner, instance_name, start_time);
		}
		
		callable.storeCallables(self);
		utils.dismountRiderJetpackCheck(self);
		
		CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "movePlayerToInstance-Team for player "+self+"("+getPlayerName(self)+") is "+team+".");
		
		if (team == 1)
		{
			CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "movePlayerToInstance-team equaled 1 for player "+self+"("+getPlayerName(self)+"). calling sendToEnterOne");
			instance.sendToEnterOne(self, instance_id, instanceLoc, data);
		}
		if (team == 2)
		{
			CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "movePlayerToInstance-team equaled 1 for player "+self+"("+getPlayerName(self)+"). calling sendToEnterTwo");
			instance.sendToEnterTwo(self, instance_id, instanceLoc, data);
		}
		
		int calendarTime = getCalendarTime();
		String realTime = getCalendarTimeStringLocal(calendarTime);
		int duration = data.getInt("time_limit");
		String strProfession = skill.getProfessionName(getSkillTemplate(self));
		String playerName = getPlayerName(self);
		int playerLevel = getLevel(self);
		obj_id groupId = getGroupObject(self);
		String playerFaction = factions.getFaction(self);
		int remaining = (start_time + duration) - getGameTime();
		
		CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "movePlayerToInstance-join_code for player "+self+"("+getPlayerName(self)+") was "+join_code+".");
		
		if (join_code == 0)
		{
			CustomerServiceLog("instance-"+instance_name, "Player("+playerName+" STARTED instance_id("+instance_id+")");
			CustomerServiceLog("instance-"+instance_name, "Start Time: "+realTime+", Duration: "+duration);
			CustomerServiceLog("instance-"+instance_name, "Instance Physical Location: "+instanceLoc);
			CustomerServiceLog("instance-"+instance_name, "Side "+team);
			CustomerServiceLog("instance-"+instance_name, "Players Is: "+strProfession+"/"+playerLevel+"/"+playerFaction);
			
			if (isIdValid(groupId))
			{
				obj_id[] players = getGroupMemberIds(groupId);
				CustomerServiceLog("instance-"+instance_name, "GROUP LIST");
				for (int i=0; i<players.length; i++)
				{
					testAbortScript();
					if (players[i] == self)
					{
						continue;
					}
					
					CustomerServiceLog("instance-"+instance_name, "Member: "+getPlayerName(players[i]));
				}
			}
		}
		
		if (join_code == 1)
		{
			CustomerServiceLog("instance-"+instance_name, "Player("+playerName+" JOINED instance_id("+instance_id+")");
			CustomerServiceLog("instance-"+instance_name, "Instance owned by "+getPlayerName(owner));
			CustomerServiceLog("instance-"+instance_name, "Player Joined at: "+realTime+", time remaining: "+remaining);
			CustomerServiceLog("instance-"+instance_name, "Instance Physical Location: "+instanceLoc);
			CustomerServiceLog("instance-"+instance_name, "Side "+team);
			CustomerServiceLog("instance-"+instance_name, "Players Is: "+strProfession+"/"+playerLevel+"/"+playerFaction);
			
			if (isIdValid(groupId))
			{
				obj_id[] players = getGroupMemberIds(groupId);
				CustomerServiceLog("instance-"+instance_name, "GROUP LIST");
				for (int i=0; i<players.length; i++)
				{
					testAbortScript();
					if (players[i] == self)
					{
						continue;
					}
					
					CustomerServiceLog("instance-"+instance_name, "Member: "+getPlayerName(players[i]));
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdShowInstanceInformation(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (utils.hasScriptVar(self, "showInstanceSui"))
		{
			int oldPid = utils.getIntScriptVar(self, "showInstanceSui");
			sui.forceCloseSUIPage(oldPid);
		}
		
		String title = utils.packStringId (new string_id("instance", "instance_sui_title"));
		String prompt = getShowInstanceInformationPrompt(self);
		
		int pid = createSUIPage(sui.SUI_LISTBOX, self, self, "noHandler");
		
		setSUIProperty(pid, "", "Size", "650,375");
		setSUIProperty(pid, sui.LISTBOX_TITLE, sui.PROP_TEXT, title);
		setSUIProperty(pid, sui.LISTBOX_PROMPT, sui.PROP_TEXT, prompt);
		
		sui.listboxButtonSetup(pid, sui.OK_ONLY);
		setSUIProperty(pid, sui.LISTBOX_BTN_OK, sui.PROP_TEXT, "@cancel");
		String playerRaidId = "";
		
		clearSUIDataSource(pid, sui.LISTBOX_DATASOURCE);
		
		dictionary[] instance_data = instance.getAllLockoutData(self);
		int idx = 0;
		Vector listId = new Vector();
		listId.setSize(0);
		
		String header = utils.packStringId (new string_id("instance", "instance_sui_heading"));
		addSUIDataItem(pid, sui.LISTBOX_DATASOURCE, ""+ idx);
		setSUIProperty(pid, sui.LISTBOX_DATASOURCE + "."+ idx, sui.PROP_TEXT, header);
		idx++;
		
		if (instance_data == null || instance_data.length == 0)
		{
			addSUIDataItem(pid, sui.LISTBOX_DATASOURCE, ""+ idx);
			setSUIProperty(pid, sui.LISTBOX_DATASOURCE + "."+ idx, sui.PROP_TEXT, "No Instance Data");
			idx++;
		}
		else
		{
			for (int i = 0; i < instance_data.length; i++)
			{
				testAbortScript();
				
				if (instance_data[i] == null)
				{
					listId.set(i, "");
					continue;
				}
				
				int time = instance_data[i].getInt("time");
				obj_id instance_id = instance_data[i].getObjId("instance_id");
				obj_id owner = instance_data[i].getObjId("owner");
				String instance_name = instance_data[i].getString("instance_name");
				int startTime = instance_data[i].getInt("start_time");
				
				int timeToClose = startTime + instance.getInstanceDuration(instance_name) - getGameTime();
				String xxTimeToClose = getCalendarTimeStringLocal(getCalendarTime() + timeToClose);
				if (timeToClose < 0)
				{
					timeToClose = 0;
				}
				
				playerRaidId = getRaidId(""+startTime, ""+instance_id, ""+owner);
				listId.add(instance_name+"-"+playerRaidId);
				
				int timeDifference = time - getCalendarTime();
				String timeString = utils.formatTimeVerbose(timeDifference);
				String xxTimeString = getCalendarTimeStringLocal(getCalendarTime() + timeDifference);
				String closeString = "";
				boolean isClosed = false;
				
				prose_package statusProse = new prose_package();
				if (timeToClose == 0)
				{
					isClosed = true;
					closeString = "Instance Closed";
				}
				else
				{
					isClosed = false;
					closeString = utils.formatTimeVerbose(timeToClose);
				}
				
				prose_package pp = new prose_package();
				
				if (isClosed)
				{
					pp.stringId = new string_id("instance", "player_closed");
				}
				else
				{
					pp.stringId = new string_id("instance", "player_open");
				}
				
				String instanceName = "@"+new string_id("instance", instance_name);
				pp.actor.set(playerRaidId);
				pp.other.set(timeString);
				pp.target.set(closeString);
				
				String dataPoint = " \0"+packOutOfBandProsePackage(null, pp);
				
				dataPoint = ""+instanceName+dataPoint;
				
				addSUIDataItem(pid, sui.LISTBOX_DATASOURCE, ""+ idx);
				setSUIProperty(pid, sui.LISTBOX_DATASOURCE + "."+ idx, sui.PROP_TEXT, dataPoint);
				idx++;
				
			}
		}
		
		obj_id groupId = getGroupObject(self);
		
		if (isIdValid(groupId))
		{
			obj_id[] members = getGroupMemberIds(groupId);
			
			if (members != null || members.length > 0)
			{
				addSUIDataItem(pid, sui.LISTBOX_DATASOURCE, ""+ idx);
				setSUIProperty(pid, sui.LISTBOX_DATASOURCE + "."+ idx, sui.PROP_TEXT, "Group Members In Range");
				idx++;
				
				for (int i=0; i<members.length; i++)
				{
					testAbortScript();
					if (!isIdValid(members[i]) || !exists(members[i]))
					{
						continue;
					}
					
					if (!isPlayer(members[i]) || members[i] == self)
					{
						continue;
					}
					
					String playerName = getPlayerFullName(members[i]);
					prose_package namePackage = new prose_package();
					namePackage.stringId = new string_id("instance", "gold_text");
					namePackage.actor.set(playerName);
					playerName = "\0"+packOutOfBandProsePackage(null, namePackage);
					
					addSUIDataItem(pid, sui.LISTBOX_DATASOURCE, ""+ idx);
					setSUIProperty(pid, sui.LISTBOX_DATASOURCE + "."+ idx, sui.PROP_TEXT, playerName);
					idx++;
					
					dictionary[] groupMemberInstanceData = instance.getAllLockoutData(members[i]);
					
					if (groupMemberInstanceData == null || groupMemberInstanceData.length == 0)
					{
						addSUIDataItem(pid, sui.LISTBOX_DATASOURCE, ""+ idx);
						setSUIProperty(pid, sui.LISTBOX_DATASOURCE + "."+ idx, sui.PROP_TEXT, "No Instance Data");
						idx++;
					}
					else
					{
						for (int q = 0; q < groupMemberInstanceData.length; q++)
						{
							testAbortScript();
							
							if (groupMemberInstanceData[q] == null)
							{
								continue;
							}
							
							int time = groupMemberInstanceData[q].getInt("time");
							obj_id instance_id = groupMemberInstanceData[q].getObjId("instance_id");
							obj_id owner = groupMemberInstanceData[q].getObjId("owner");
							String instance_name = groupMemberInstanceData[q].getString("instance_name");
							int startTime = groupMemberInstanceData[q].getInt("start_time");
							
							int timeToClose = startTime + instance.getInstanceDuration(instance_name) - getGameTime();
							String xxTimeToClose = getCalendarTimeStringLocal(getCalendarTime() + timeToClose);
							if (timeToClose < 0)
							{
								timeToClose = 0;
							}
							
							String raidId = getRaidId(""+startTime, ""+instance_id, ""+owner);
							
							prose_package raidProse = new prose_package();
							boolean bClose = false;
							boolean bMatch = false;
							
							String passedId = getPassedId(listId, instance_name);
							if (!passedId.equals("") && raidId.equals(passedId))
							{
								bMatch = true;
							}
							else
							{
								bMatch = false;
							}
							
							int timeDifference = time - getCalendarTime();
							String timeString = utils.formatTimeVerbose(timeDifference);
							String xxTimeString = getCalendarTimeStringLocal(getCalendarTime() + timeDifference);
							
							prose_package statusProse = new prose_package();
							String closeString = "";
							
							if (timeToClose == 0)
							{
								bClose = true;
								closeString = "Instance Closed";
								
							}
							else
							{
								bClose = false;
								closeString = utils.formatTimeVerbose(timeToClose);
							}
							
							prose_package pp = new prose_package();
							
							if (bClose && bMatch)
							{
								pp.stringId = new string_id("instance", "close_same");
							}
							
							if (bClose && !bMatch)
							{
								pp.stringId = new string_id("instance", "close_different");
							}
							
							if (!bClose && bMatch)
							{
								pp.stringId = new string_id("instance", "open_same");
							}
							
							if (!bClose && !bMatch)
							{
								pp.stringId = new string_id("instance", "open_different");
							}
							
							String instanceName = "@"+new string_id("instance", instance_name);
							
							pp.other.set(timeString);
							pp.target.set(closeString);
							pp.actor.set(raidId);
							
							String dataPoint = " \0"+packOutOfBandProsePackage(null, pp);
							
							dataPoint = ""+instanceName+dataPoint;
							
							addSUIDataItem(pid, sui.LISTBOX_DATASOURCE, ""+ idx);
							setSUIProperty(pid, sui.LISTBOX_DATASOURCE + "."+ idx, sui.PROP_TEXT, dataPoint);
							idx++;
						}
					}
				}
			}
			
		}
		
		subscribeToSUIEvent(pid, sui_event_type.SET_onGenericSelection, sui.LISTBOX_LIST, "noHandler");
		subscribeToSUIPropertyForEvent(pid, sui_event_type.SET_onGenericSelection, sui.LISTBOX_LIST, sui.LISTBOX_LIST, sui.PROP_SELECTEDROW);
		
		subscribeToSUIProperty(pid, sui.LISTBOX_LIST, sui.PROP_SELECTEDROW);
		subscribeToSUIProperty(pid, sui.LISTBOX_TITLE, sui.PROP_TEXT);
		
		showSUIPage(pid);
		flushSUIPage(pid);
		
		utils.setScriptVar(self, "showInstanceSui", pid);
		return SCRIPT_CONTINUE;
	}
	
	
	public String getShowInstanceInformationPrompt(obj_id self) throws InterruptedException
	{
		
		String concatenatedAtrocity = getString(new string_id("instance", "flagging_header"));
		concatenatedAtrocity += "\n";
		
		concatenatedAtrocity += getUnflaggedInstancesString(self);
		concatenatedAtrocity += "\n";
		
		obj_id groupId = getGroupObject(self);
		
		if (isIdValid(groupId))
		{
			obj_id[] members = getGroupMemberIds(groupId);
			
			if (members != null || members.length > 0)
			{
				for (int i = 0; i < members.length; i++)
				{
					testAbortScript();
					if (!isIdValid(members[i]) || !exists(members[i]))
					{
						continue;
					}
					
					if (!isPlayer(members[i]) || members[i] == self)
					{
						continue;
					}
					
					concatenatedAtrocity += getUnflaggedInstancesString(members[i]);
					concatenatedAtrocity += "\n";
				}
			}
		}
		
		return concatenatedAtrocity;
	}
	
	
	public String getUnflaggedInstancesString(obj_id player) throws InterruptedException
	{
		String[] instanceList = new String[dataTableGetNumRows(instance.INSTANCE_DATATABLE)];
		int numNotFlagged = 0;
		String unflaggedInstancesString = getString(new string_id("instance", "gold"));
		unflaggedInstancesString += getFirstName(player) + ": ";
		
		String instancesString = "";
		String preambleString = "";
		
		for (int i = 0; i < instanceList.length; i++)
		{
			testAbortScript();
			String thisInstance = dataTableGetString(instance.INSTANCE_DATATABLE, i, "key_required");
			
			if (thisInstance.equals("none"))
			{
				continue;
			}
			if (!hasObjVar(player, "instance_player_protected_data." + thisInstance))
			{
				
				if (numNotFlagged > 0)
				{
					instancesString += ", ";
				}
				
				instancesString += getString(new string_id("instance", thisInstance));
				numNotFlagged++;
			}
		}
		
		if (numNotFlagged == 0)
		{
			preambleString += getString(new string_id ("instance", "flagging_all_flags"));
		}
		else
		{
			preambleString += getString(new string_id("instance", "flagging_not_flagged_for"));
		}
		
		unflaggedInstancesString += preambleString + instancesString;
		
		return unflaggedInstancesString;
	}
	
	
	public String getRaidId(String time, String instance, String owner) throws InterruptedException
	{
		String subTime = time.substring(time.length() -3, time.length());
		String subInstance = instance.substring(instance.length() - 3, instance.length());
		String subOwner = owner.substring(owner.length() -3,owner.length());
		return subTime + subInstance + subOwner;
	}
	
	
	public String getPassedId(Vector listId, String instance_name) throws InterruptedException
	{
		if (listId == null || listId.size() == 0)
		{
			return "";
		}
		
		for (int i=0; i<listId.size(); i++)
		{
			testAbortScript();
			if (((String)(listId.get(i))) == "")
			{
				continue;
			}
			
			String[] parse = split(((String)(listId.get(i))), '-');
			String passedName = parse[0];
			String passedId = parse[1];
			
			if (passedName.equals(instance_name))
			{
				return passedId;
			}
		}
		
		return "";
	}
	
	
	public void doLogging(String message) throws InterruptedException
	{
		LOG("doLogging/player_instance", message);
	}
	
	
	public void doLogging(String section, String message) throws InterruptedException
	{
		LOG("doLogging/player_instance/"+section, message);
	}
	
	
	public int handleAwardtoken(obj_id self, dictionary params) throws InterruptedException
	{
		int tokenIndex = params.getInt("tokenIndex");
		int count = 1;
		
		if (params.containsKey("tokenCount"))
		{
			count = params.getInt("tokenCount");
		}
		
		if (tokenIndex < 0 || tokenIndex >= trial.HEROIC_TOKENS.length)
		{
			sendSystemMessageTestingOnly(self, "DEBUG: handleAwardtoken sent with out of range tokenIndex.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id inventory = utils.getInventoryContainer(self);
		obj_id token = static_item.createNewItemFunction(trial.HEROIC_TOKENS[tokenIndex], inventory, count);
		
		prose_package pp = new prose_package();
		if (count > 1)
		{
			pp = prose.setStringId(pp, new string_id("set_bonus", "recieve_heroic_token_multi"));
		}
		else
		{
			pp = prose.setStringId(pp, new string_id("set_bonus", "recieve_heroic_token"));
		}
		
		pp = prose.setDI(pp, count);
		pp = prose.setTT(pp, new string_id("static_item_n", trial.HEROIC_TOKENS[tokenIndex]));
		sendSystemMessageProse(self, pp);
		
		if (tokenIndex == 5 && count == 9)
		{
			badge.grantBadge(self, "champion_of_hoth");
			obj_id[] rewards = new obj_id[1];
			
			rewards[0] = static_item.createNewItemFunction("item_hoth_flawless_painting", inventory);
			showLootBox(self, rewards);
			setObjVar(self, "hoth.flawless_reward", 1);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int instanceFailureMessage(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "instanceFailureMessage-has a null params exiting handler.");
			return SCRIPT_CONTINUE;
		}
		
		int reason = params.getInt("fail_reason");
		
		CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "instanceFailureMessage-params.fail_reason was "+ reason + ".");
		
		switch(reason)
		{
			case instance.FAIL_INSTANCE_FULL:
			sendSystemMessage(self, new string_id("instance", "fail_instance_full"));
			break;
			
			case instance.FAIL_NOT_MEMBER:
			sendSystemMessage(self, new string_id("instance", "fail_not_member"));
			break;
			case instance.INSTANCE_TIMEOUT:
			sendSystemMessage(self, new string_id("instance", "instance_time_out"));
			break;
			case instance.FAIL_INVALID_LOCKOUT:
			sendSystemMessage(self, new string_id("instance", "fail_invalid_lockout"));
			break;
			case instance.FAIL_INSTANCE_CLOSED:
			sendSystemMessageTestingOnly(self, "The instance you are attempting to enter is currently inactive");
			break;
			case instance.FAIL_INSTANCE_FEW_PLAYERS:
			sendSystemMessage(self, new string_id("instance", "fail_under_player_limit"));
			break;
			case instance.FAIL_WARN_TOO_FEW:
			int limit = params.getInt("player_low_limit");
			prose_package pp = new prose_package();
			pp = prose.setStringId(pp, new string_id("instance", "fail_under_player_limit_warning"));
			pp = prose.setDI(pp, limit);
			sendSystemMessageProse(self, pp);
			break;
			default:
			CustomerServiceLog(instance.INSTANCE_DEBUG_LOG, "instanceFailureMessage-switch statement reached the default case this means no system message to player and we died.");
			break;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int requestInstance(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id enterObject = getFirstObjectWithObjVar(getLocation(trial.getTop(self)), 1000.0f, "instance_name");
		
		if (!isIdValid(enterObject))
		{
			sendSystemMessageTestingOnly(self, "No object in range");
			return SCRIPT_CONTINUE;
		}
		
		String instance_name = getStringObjVar(enterObject, "instance_name");
		instance.requestInstanceMovement(self, instance_name);
		return SCRIPT_CONTINUE;
	}
	
	
	public int delayInstanceRequest(obj_id self, dictionary params) throws InterruptedException
	{
		String instance_name = params.getString("instance_name");
		instance.requestInstanceMovement(self, instance_name);
		return SCRIPT_CONTINUE;
	}
	
	
	public int clearInstanceScriptVar(obj_id self, dictionary params) throws InterruptedException
	{
		utils.removeScriptVarTree(self, "instance");
		
		return SCRIPT_CONTINUE;
	}
}
