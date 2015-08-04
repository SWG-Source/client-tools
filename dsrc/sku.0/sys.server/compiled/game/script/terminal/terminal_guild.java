package script.terminal;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.guild;
import script.library.player_structure;
import script.library.sui;
import script.library.utils;
import script.library.prose;



public class terminal_guild extends script.terminal.base.base_terminal
{
	public terminal_guild()
	{
	}
	public static final string_id SID_GUILD_GUILD_MANAGEMENT = new string_id ("guild", "menu_guild_management");
	public static final string_id SID_GUILD_MEMBER_MANAGEMENT = new string_id ("guild", "menu_member_management");
	public static final string_id SID_GUILD_CREATE = new string_id ("guild", "menu_create");
	public static final string_id SID_GUILD_INFO = new string_id ("guild", "menu_info");
	public static final string_id SID_GUILD_MEMBERS = new string_id ("guild", "menu_members");
	public static final string_id SID_GUILD_SPONSORED = new string_id ("guild", "menu_sponsored");
	public static final string_id SID_GUILD_ENEMIES = new string_id ("guild", "menu_enemies");
	public static final string_id SID_GUILD_SPONSOR = new string_id ("guild", "menu_sponsor");
	public static final string_id SID_GUILD_DISBAND = new string_id ("guild", "menu_disband");
	public static final string_id SID_GUILD_NAMECHANGE = new string_id ("guild", "menu_namechange");
	public static final string_id SID_GUILD_SPONSOR_ACCEPT = new string_id ("guild", "sponsor_accept");
	public static final string_id SID_GUILD_SPONSOR_DECLINE = new string_id ("guild", "sponsor_decline");
	public static final string_id SID_GUILD_SPONSOR_NOT_FOUND = new string_id ("guild", "sponsor_not_found");
	public static final string_id SID_GUILD_SPONSOR_ALREADY_IN_GUILD = new string_id ("guild", "sponsor_already_in_guild");
	
	public static final String STR_GUILD_ALLEGIENCE = "@guild:allegiance";
	public static final String STR_GUILD_TITLE = "@guild:title";
	public static final String STR_GUILD_KICK = "@guild:kick";
	public static final String STR_GUILD_PERMISSIONS = "@guild:permissions";
	public static final String STR_GUILD_SPONSOR_PROMPT = "@guild:sponsor_prompt";
	public static final String STR_GUILD_SPONSOR_TITLE = "@guild:sponsor_title";
	public static final String STR_GUILD_ADD_ENEMY = "@guild:add_enemy";
	public static final String STR_GUILD_ENEMIES_PROMPT = "@guild:enemies_prompt";
	public static final String STR_GUILD_ENEMIES_TITLE = "@guild:enemies_title";
	public static final String STR_GUILD_SPONSORED_PROMPT = "@guild:sponsored_prompt";
	public static final String STR_GUILD_SPONSORED_TITLE = "@guild:sponsored_title";
	public static final String STR_GUILD_MEMBERS_PROMPT = "@guild:members_prompt";
	public static final String STR_GUILD_MEMBERS_TITLE = "@guild:members_title";
	public static final String STR_GUILD_DISBAND_TITLE = "@guild:disband_title";
	public static final String STR_GUILD_DISBAND_PROMPT = "@guild:disband_prompt";
	public static final String STR_GUILD_CREATE_NAME_PROMPT = "@guild:create_name_prompt";
	public static final String STR_GUILD_CREATE_NAME_TITLE = "@guild:create_name_title";
	public static final String STR_GUILD_CREATE_ABBREV_PROMPT = "@guild:create_abbrev_prompt";
	public static final String STR_GUILD_CREATE_ABBREV_TITLE = "@guild:create_abbrev_title";
	public static final String STR_GUILD_NAMECHANGE_NAME_PROMPT = "@guild:namechange_name_prompt";
	public static final String STR_GUILD_NAMECHANGE_NAME_TITLE = "@guild:namechange_name_title";
	public static final String STR_GUILD_NAMECHANGE_ABBREV_PROMPT = "@guild:namechange_abbrev_prompt";
	public static final String STR_GUILD_NAMECHANGE_ABBREV_TITLE = "@guild:namechange_abbrev_title";
	public static final String STR_GUILD_SPONSOR_VERIFY_PROMPT = "@guild:sponsor_verify_prompt";
	public static final String STR_GUILD_SPONSOR_VERIFY_TITLE = "@guild:sponsor_verify_title";
	public static final String STR_GUILD_MEMBER_OPTIONS_PROMPT = "@guild:member_options_prompt";
	public static final String STR_GUILD_MEMBER_OPTIONS_TITLE = "@guild:member_options_title";
	public static final String STR_GUILD_ACCEPT = "@guild:accept";
	public static final String STR_GUILD_DECLINE = "@guild:decline";
	public static final String STR_GUILD_SPONSORED_OPTIONS_PROMPT = "@guild:sponsored_options_prompt";
	public static final String STR_GUILD_SPONSORED_OPTIONS_TITLE = "@guild:sponsored_options_title";
	public static final String STR_GUILD_WAR_ENEMY_NAME_PROMPT = "@guild:war_enemy_name_prompt";
	public static final String STR_GUILD_WAR_ENEMY_NAME_TITLE = "@guild:war_enemy_name_title";
	public static final String STR_GUILD_TITLE_PROMPT = "@guild:title_prompt";
	public static final String STR_GUILD_TITLE_TITLE = "@guild:title_title";
	public static final String STR_GUILD_KICK_PROMPT = "@guild:kick_prompt";
	public static final String STR_GUILD_KICK_TITLE = "@guild:kick_title";
	public static final String STR_GUILD_PERMISSIONS_PROMPT = "@guild:permissions_prompt";
	public static final String STR_GUILD_PERMISSIONS_TITLE = "@guild:permissions_title";
	public static final String STR_GUILD_INFO_TITLE = "@guild:info_title";
	public static final String STR_GUILD_PERMISSION_MAIL_YES = "@guild:permission_mail_yes";
	public static final String STR_GUILD_PERMISSION_MAIL_NO = "@guild:permission_mail_no";
	public static final String STR_GUILD_PERMISSION_SPONSOR_YES = "@guild:permission_sponsor_yes";
	public static final String STR_GUILD_PERMISSION_SPONSOR_NO = "@guild:permission_sponsor_no";
	public static final String STR_GUILD_PERMISSION_TITLE_YES = "@guild:permission_title_yes";
	public static final String STR_GUILD_PERMISSION_TITLE_NO = "@guild:permission_title_no";
	public static final String STR_GUILD_PERMISSION_ACCEPT_YES = "@guild:permission_accept_yes";
	public static final String STR_GUILD_PERMISSION_ACCEPT_NO = "@guild:permission_accept_no";
	public static final String STR_GUILD_PERMISSION_KICK_YES = "@guild:permission_kick_yes";
	public static final String STR_GUILD_PERMISSION_KICK_NO = "@guild:permission_kick_no";
	public static final String STR_GUILD_PERMISSION_WAR_YES = "@guild:permission_war_yes";
	public static final String STR_GUILD_PERMISSION_WAR_NO = "@guild:permission_war_no";
	public static final String STR_GUILD_PERMISSION_NAMECHANGE_YES = "@guild:permission_namechange_yes";
	public static final String STR_GUILD_PERMISSION_NAMECHANGE_NO = "@guild:permission_namechange_no";
	public static final String STR_GUILD_PERMISSION_DISBAND_YES = "@guild:permission_disband_yes";
	public static final String STR_GUILD_PERMISSION_DISBAND_NO = "@guild:permission_disband_no";
	
	public static final string_id SID_GUILD_LEADER_CHANGE = new string_id ("guild", "menu_leader_change");
	public static final string_id SID_GUILD_ENABLE_ELECTIONS = new string_id ("guild", "menu_enable_elections");
	public static final string_id SID_GUILD_DISABLE_ELECTIONS = new string_id ("guild", "menu_disable_elections");
	
	public static final string_id SID_LEADER_RACE = new string_id ("guild", "menu_leader_race");
	public static final string_id SID_LEADER_STANDINGS = new string_id ("guild", "menu_leader_standings");
	public static final string_id SID_LEADER_VOTE = new string_id ("guild", "menu_leader_vote");
	public static final string_id SID_LEADER_REGISTER = new string_id ("guild", "menu_leader_register");
	public static final string_id SID_LEADER_UNREGISTER = new string_id ("guild", "menu_leader_unregister");
	public static final string_id SID_RESET_VOTING = new string_id ("guild", "menu_leader_reset_vote");
	public static final string_id SID_NO_CANDIDATES = new string_id ("guild", "vote_no_candidates");
	public static final string_id SID_VOTE_PLACED = new string_id ("guild", "vote_placed");
	public static final string_id SID_VOTE_ABSTAIN = new string_id ("guild", "vote_abstain");
	public static final string_id SID_REGISTER_DUPE = new string_id ("guild", "vote_register_dupe");
	public static final string_id SID_NOT_REGISTERED = new string_id ("guild", "vote_not_registered");
	public static final string_id SID_UNREGISTERED = new string_id ("guild", "vote_unregistered");
	public static final string_id SID_REGISTER_CONGRATS = new string_id ("guild", "vote_register_congrats");
	public static final string_id SID_ELECTIONS_OPEN = new string_id ("guild", "vote_elections_open");
	public static final string_id SID_ELECTIONS_CLOSED = new string_id ("guild", "vote_elections_closed");
	
	public static final string_id SID_ALREADY_LEADER = new string_id ("guild", "already_leader");
	
	public static final string_id OPEN_ELECTIONS_EMAIL_BODY = new string_id ("guild", "open_elections_email_body");
	public static final string_id OPEN_ELECTIONS_EMAIL_SUBJECT = new string_id ("guild", "open_elections_email_subject");
	public static final string_id CLOSED_ELECTIONS_EMAIL_BODY = new string_id ("guild", "closed_elections_email_body");
	public static final string_id CLOSED_ELECTIONS_EMAIL_SUBJECT = new string_id ("guild", "closed_elections_email_subject");
	public static final string_id OPEN_ELECTIONS_ABSENT_EMAIL_BODY = new string_id ("guild", "open_elections_absent_email_body");
	public static final string_id OPEN_ELECTIONS_ABSENT_EMAIL_SUBJECT = new string_id ("guild", "open_elections_absent_email_subject");
	
	public static final string_id SID_NEED_ACCEPT_HALL = new string_id ("guild", "need_accept_hall");
	public static final string_id SID_ACCEPT_HALL = new string_id ("guild", "accept_hall");
	public static final string_id SID_TERMINAL_LOCKED = new string_id ("guild", "terminal_locked");
	public static final string_id SID_NO_LOTS = new string_id ("guild", "no_lots");
	
	public static final string_id SID_ML_NOT_LOADED = new string_id ("guild", "ml_not_loaded");
	public static final string_id SID_ML_NO_LOTS_FREE = new string_id ("guild", "ml_no_lots_free");
	public static final string_id SID_ML_TRIAL = new string_id ("guild", "ml_trial");
	public static final string_id SID_ML_SUCCESS = new string_id ("guild", "ml_success");
	public static final string_id SID_ML_REJECTED = new string_id ("guild", "ml_rejected");
	public static final string_id SID_ML_YOU_ARE_LEADER = new string_id ("guild", "ml_you_are_leader");
	public static final string_id SID_ML_FAIL = new string_id ("guild", "ml_fail");
	
	public static final string_id SID_PA_OWNER_NOW = new string_id ("guild", "pa_owner_now");
	
	public static final int DISBAND_CHECK_TIME = 60*60*2;
	public static final float TERMINAL_USE_DISTANCE = 8.0f;
	
	public static final int LEADER_ABSENT_TIME = 60*60*24*28;
	public static final int ELECTIONS_COMPLETE = 60*60*24*14;
	
	public static final String SID_VOTING_ENABLED = "@guild:voting_enabled";
	public static final String SID_VOTING_DISABLED = "@guild:voting_disabled";
	
	
	public int getStructureGuildId(obj_id self) throws InterruptedException
	{
		
		int guildId = 0;
		obj_id structure = player_structure.getStructure(self);
		if (isIdValid(structure) && hasObjVar(structure, "player_structure.owner") && hasObjVar(structure, "guildId"))
		{
			guildId = getIntObjVar(structure, "guildId");
			if (guildId != 0 && !guildExists(guildId))
			{
				removeObjVar(structure, "guildId");
				guildId = 0;
			}
		}
		
		if (guildId == 0 && hasObjVar(self, "guildId"))
		{
			guildId = getIntObjVar(self, "guildId");
			if (guildId != 0 && !guildExists(guildId))
			{
				removeObjVar(self, "guildId");
				guildId = 0;
			}
		}
		return guildId;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		messageTo(self, "checkDisband", null, DISBAND_CHECK_TIME, false);
		
		obj_id structure = player_structure.getStructure(self);
		if (isIdValid(structure))
		{
			if (!hasObjVar(structure, player_structure.VAR_IS_GUILD_HALL))
			{
				setObjVar(structure, player_structure.VAR_IS_GUILD_HALL, 1);
			}
			setObjVar( self, "pa_hall", structure );
			if (hasObjVar( structure, "elections" ))
			{
				setObjVar( self, "elections", getIntObjVar( structure, "elections") );
			}
		}
		
		obj_id pa_hall = player_structure.getStructure( self );
		if (!hasScript( pa_hall, "guild.pa_hall_vote" ))
		{
			attachScript( pa_hall, "guild.pa_hall_vote");
		}
		setObjVar( pa_hall, "vote_terminal", self );
		
		return super.OnInitialize(self);
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		int idx = utils.getValidAttributeIndex(names);
		if (idx == -1)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (exists(self))
		{
			
			obj_id pa_hall = getTopMostContainer(self);
			if (isValidId(pa_hall) && exists(pa_hall))
			{
				int guildId = getStructureGuildId(self);
				if (guildId > -1)
				{
					String guildName = guildGetName(guildId);
					if (!guildName.equals(""))
					{
						names[idx] = "guild_name";
						attribs[idx] = guildName;
						idx++;
					}
					String guildAbbrev = guildGetAbbrev(guildId);
					if (!guildName.equals(""))
					{
						names[idx] = "guild_abbrev";
						attribs[idx] = guildAbbrev;
						idx++;
					}
					
					obj_id guildLeader = guildGetLeader(guildId);
					if (isValidId(guildLeader))
					{
						names[idx] = "guild_leader";
						attribs[idx] = guildGetMemberName(guildId, guildLeader);
						idx++;
					}
					
					names[idx] = "elections";
					if (hasObjVar( self, "elections" ))
					{
						attribs[idx] = SID_VOTING_ENABLED;
					}
					else
					{
						attribs[idx] = SID_VOTING_DISABLED;
					}
					idx++;
					
					if (hasObjVar( self, "elections"))
					{
						names[idx] = "elections_countdown";
						int electionCompleted = (getIntObjVar(self, "elections") + ELECTIONS_COMPLETE) - getGameTime();
						attribs[idx] = utils.assembleTimeRemainToUse(electionCompleted);
						idx++;
						
						obj_id[] candidates = getObjIdArrayObjVar( pa_hall, "candidate_list");
						if (candidates != null)
						{
							for (int i=0; i < candidates.length; i++)
							{
								testAbortScript();
								if (candidates[i] == guildLeader)
								{
									names[idx] = "incumbent";
									attribs[idx] = guildGetMemberName(guildId, guildLeader);
									idx++;
								}
								else
								{
									names[idx] = "candidate";
									attribs[idx] = guildGetMemberName(guildId, candidates[i]);
									idx++;
								}
							}
						}
					}
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (outOfRange(self, player))
		{
			return super.OnObjectMenuRequest(self, player, mi);
		}
		
		int guildId = getStructureGuildId(self);
		
		obj_id structure = player_structure.getStructure(self);
		String pa_owner = getStringObjVar( structure, "player_structure.owner");
		
		if (guildId == 0)
		{
			if (isIdValid(structure) && hasObjVar(structure, "player_structure.owner"))
			{
				String ownerName = getStringObjVar(structure, "player_structure.owner");
				String firstName = getFirstName(player);
				if (ownerName.toUpperCase().equals(firstName.toUpperCase()))
				{
					
					guildId = getGuildId(player);
					if (guildId != 0)
					{
						setObjVar(structure, "guildId", guildId);
						permissionsAddAllowed(structure, "Guild:"+guildGetAbbrev(guildId));
					}
					else
					{
						mi.addRootMenu(menu_info_types.SERVER_GUILD_CREATE, SID_GUILD_CREATE);
					}
				}
			}
		}
		
		if (guildId != 0 && (getGuildId(player) == guildId || isGod(player)))
		{
			int guildManagementMenu = mi.addRootMenu(menu_info_types.SERVER_GUILD_GUILD_MANAGEMENT, SID_GUILD_GUILD_MANAGEMENT);
			mi.addSubMenu(guildManagementMenu, menu_info_types.SERVER_GUILD_INFO, SID_GUILD_INFO);
			mi.addSubMenu(guildManagementMenu, menu_info_types.SERVER_GUILD_ENEMIES, SID_GUILD_ENEMIES);
			if (guild.hasGuildPermission(guildId, player, guild.GUILD_PERMISSION_DISBAND))
			{
				mi.addSubMenu(guildManagementMenu, menu_info_types.SERVER_GUILD_DISBAND, SID_GUILD_DISBAND);
			}
			if (guild.hasGuildPermission(guildId, player, guild.GUILD_PERMISSION_NAMECHANGE))
			{
				mi.addSubMenu(guildManagementMenu, menu_info_types.SERVER_GUILD_NAMECHANGE, SID_GUILD_NAMECHANGE);
			}
			if (player == guildGetLeader(guildId))
			{
				if (hasObjVar( self, "elections" ))
				{
					mi.addSubMenu(guildManagementMenu, menu_info_types.SERVER_MENU2, SID_GUILD_DISABLE_ELECTIONS);
				}
				else
				{
					mi.addSubMenu(guildManagementMenu, menu_info_types.SERVER_MENU2, SID_GUILD_ENABLE_ELECTIONS);
				}
				
				String player_whole_name = guildGetMemberName( guildId, player );
				java.util.StringTokenizer tok = new java.util.StringTokenizer( player_whole_name );
				String player_name = tok.nextToken();
				if (!player_name.equalsIgnoreCase( pa_owner ))
				{
					
					obj_id pa_owner_id = getPlayerIdFromFirstName( pa_owner );
					int login_date = getPlayerLastLoginTime( pa_owner_id );
					long current_date = System.currentTimeMillis() / 1000;
					if (current_date - login_date > LEADER_ABSENT_TIME)
					{
						
						mi.addRootMenu( menu_info_types.SERVER_MENU9, SID_ACCEPT_HALL );
					}
				}
			}
			
			int memberManagementMenu = mi.addRootMenu(menu_info_types.SERVER_GUILD_MEMBER_MANAGEMENT, SID_GUILD_MEMBER_MANAGEMENT);
			mi.addSubMenu(memberManagementMenu, menu_info_types.SERVER_GUILD_MEMBERS, SID_GUILD_MEMBERS);
			if (guild.hasSponsoredMembers(guildId))
			{
				mi.addSubMenu(memberManagementMenu, menu_info_types.SERVER_GUILD_SPONSORED, SID_GUILD_SPONSORED);
			}
			if (guild.hasGuildPermission(guildId, player, guild.GUILD_PERMISSION_SPONSOR))
			{
				mi.addSubMenu(memberManagementMenu, menu_info_types.SERVER_GUILD_SPONSOR, SID_GUILD_SPONSOR);
			}
			if (player == guildGetLeader(guildId))
			{
				mi.addSubMenu(memberManagementMenu, menu_info_types.SERVER_MENU1, SID_GUILD_LEADER_CHANGE);
			}
			
			if (hasObjVar( self, "elections" ))
			{
				
				int menu = mi.addRootMenu( menu_info_types.SERVER_MENU3, SID_LEADER_RACE );
				
				mi.addSubMenu( menu, menu_info_types.SERVER_MENU4, SID_LEADER_STANDINGS );
				mi.addSubMenu( menu, menu_info_types.SERVER_MENU5, SID_LEADER_VOTE );
				
				if (!isRegisteredToRun( player, self ))
				{
					mi.addSubMenu( menu, menu_info_types.SERVER_MENU6, SID_LEADER_REGISTER );
				}
				else
				{
					mi.addSubMenu( menu, menu_info_types.SERVER_MENU7, SID_LEADER_UNREGISTER );
				}
				
			}
		}
		
		return super.OnObjectMenuRequest(self, player, mi);
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (outOfRange(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int guildId = getStructureGuildId(self);
		obj_id structure = player_structure.getStructure(self);
		String pa_owner = getStringObjVar( structure, "player_structure.owner");
		
		String leader_name = null;
		if (guildId != 0)
		{
			String leader_whole_name = guildGetMemberName( guildId, guildGetLeader(guildId) );
			if (leader_whole_name != null)
			{
				java.util.StringTokenizer tok = new java.util.StringTokenizer( leader_whole_name );
				leader_name = tok.nextToken();
			}
		}
		
		if (guildId == 0)
		{
			
			if (getGuildId(player) == 0)
			{
				if (isIdValid(structure))
				{
					String ownerName = getStringObjVar(structure, "player_structure.owner");
					String firstName = getFirstName(player);
					if (ownerName.toUpperCase().equals(firstName.toUpperCase()))
					{
						sui.inputbox(
						self,
						player,
						STR_GUILD_CREATE_NAME_PROMPT,
						sui.OK_CANCEL,
						STR_GUILD_CREATE_NAME_TITLE,
						sui.INPUT_NORMAL,
						null,
						"onGuildCreateNameResponse");
					}
				}
			}
		}
		else if (getGuildId(player) == guildId || isGod(player))
		{
			if (item == menu_info_types.ITEM_USE || item == menu_info_types.SERVER_GUILD_INFO)
			{
				showGuildInfo(self, player);
			}
			else if (item == menu_info_types.SERVER_GUILD_ENEMIES)
			{
				showGuildEnemies(self, player);
			}
			else if (item == menu_info_types.SERVER_GUILD_SPONSORED)
			{
				showGuildSponsored(self, player);
			}
			else if (item == menu_info_types.SERVER_GUILD_MEMBERS)
			{
				showGuildMembers(self, player);
			}
			else if (item == menu_info_types.SERVER_GUILD_SPONSOR)
			{
				sui.inputbox(
				self,
				player,
				STR_GUILD_SPONSOR_PROMPT,
				sui.OK_CANCEL,
				STR_GUILD_SPONSOR_TITLE,
				sui.INPUT_NORMAL,
				null,
				"onGuildSponsorResponse");
			}
			else if (item == menu_info_types.SERVER_GUILD_DISBAND)
			{
				sui.msgbox(
				self,
				player,
				STR_GUILD_DISBAND_PROMPT,
				sui.YES_NO,
				STR_GUILD_DISBAND_TITLE,
				sui.MSG_NORMAL,
				"onGuildDisbandResponse");
			}
			else if (item == menu_info_types.SERVER_GUILD_NAMECHANGE)
			{
				sui.inputbox(
				self,
				player,
				STR_GUILD_NAMECHANGE_NAME_PROMPT,
				sui.OK_CANCEL,
				STR_GUILD_NAMECHANGE_NAME_TITLE,
				sui.INPUT_NORMAL,
				null,
				"onGuildNamechangeNameResponse");
			}
			else if (item == menu_info_types.SERVER_MENU1)
			{
				if (player == guildGetLeader(guildId))
				{
					sui.inputbox( self, player, "@guild:make_leader_d", sui.OK_CANCEL, "@guild:make_leader_t", sui.INPUT_NORMAL, null, "handleMakeLeader", null );
				}
			}
			else if (item == menu_info_types.SERVER_MENU2)
			{
				toggleElections( self, player );
				sendDirtyObjectMenuNotification(self);
			}
			
			if (item == menu_info_types.SERVER_MENU3 || item == menu_info_types.SERVER_MENU4)
			{
				
				showStandings( self, player );
			}
			else if (item == menu_info_types.SERVER_MENU5)
			{
				
				placeVote( self, player );
			}
			else if (item == menu_info_types.SERVER_MENU6)
			{
				
				registerToRun( self, player );
			}
			else if (item == menu_info_types.SERVER_MENU7)
			{
				
				unregisterFromRace( self, player );
			}
			else if (item == menu_info_types.SERVER_MENU8)
			{
				
			}
			else if ((leader_name == null) || !leader_name.equalsIgnoreCase( pa_owner ))
			{
				
				if (player == guildGetLeader(guildId) && item == menu_info_types.SERVER_MENU9)
				{
					obj_id pa_owner_id = getPlayerIdFromFirstName( pa_owner );
					int login_date = getPlayerLastLoginTime( pa_owner_id );
					long current_date = System.currentTimeMillis() / 1000;
					if (current_date - login_date > LEADER_ABSENT_TIME)
					{
						acceptPAHall( self, player );
					}
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int selectPermissionsMessage(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		String name = params.getString("name");
		
		if (outOfRange(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int guildId = getStructureGuildId(self);
		
		obj_id target = guild.findMemberIdByName(guildId, name, false, true);
		setMenuContextString(self, player, "guildPermissionsTarget", name);
		
		String[] perms = new String[8];
		
		if (guild.hasGuildPermission(guildId, target, guild.GUILD_PERMISSION_MAIL))
		{
			perms[0] = STR_GUILD_PERMISSION_MAIL_YES;
		}
		else
		{
			perms[0] = STR_GUILD_PERMISSION_MAIL_NO;
		}
		
		if (guild.hasGuildPermission(guildId, target, guild.GUILD_PERMISSION_SPONSOR))
		{
			perms[1] = STR_GUILD_PERMISSION_SPONSOR_YES;
		}
		else
		{
			perms[1] = STR_GUILD_PERMISSION_SPONSOR_NO;
		}
		
		if (guild.hasGuildPermission(guildId, target, guild.GUILD_PERMISSION_TITLE))
		{
			perms[2] = STR_GUILD_PERMISSION_TITLE_YES;
		}
		else
		{
			perms[2] = STR_GUILD_PERMISSION_TITLE_NO;
		}
		
		if (guild.hasGuildPermission(guildId, target, guild.GUILD_PERMISSION_ACCEPT))
		{
			perms[3] = STR_GUILD_PERMISSION_ACCEPT_YES;
		}
		else
		{
			perms[3] = STR_GUILD_PERMISSION_ACCEPT_NO;
		}
		
		if (guild.hasGuildPermission(guildId, target, guild.GUILD_PERMISSION_KICK))
		{
			perms[4] = STR_GUILD_PERMISSION_KICK_YES;
		}
		else
		{
			perms[4] = STR_GUILD_PERMISSION_KICK_NO;
		}
		
		if (guild.hasGuildPermission(guildId, target, guild.GUILD_PERMISSION_WAR))
		{
			perms[5] = STR_GUILD_PERMISSION_WAR_YES;
		}
		else
		{
			perms[5] = STR_GUILD_PERMISSION_WAR_NO;
		}
		
		if (guild.hasGuildPermission(guildId, target, guild.GUILD_PERMISSION_NAMECHANGE))
		{
			perms[6] = STR_GUILD_PERMISSION_NAMECHANGE_YES;
		}
		else
		{
			perms[6] = STR_GUILD_PERMISSION_NAMECHANGE_NO;
		}
		
		if (guild.hasGuildPermission(guildId, target, guild.GUILD_PERMISSION_DISBAND))
		{
			perms[7] = STR_GUILD_PERMISSION_DISBAND_YES;
		}
		else
		{
			perms[7] = STR_GUILD_PERMISSION_DISBAND_NO;
		}
		
		sui.listbox(
		self,
		player,
		guild.buildFakeLocalizedProse(STR_GUILD_PERMISSIONS_PROMPT, name, ""),
		sui.OK_CANCEL,
		STR_GUILD_PERMISSIONS_TITLE,
		perms,
		"onGuildPermissionsResponse");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int showGuildEnemiesMessage(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		
		if (outOfRange(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int guildId = getStructureGuildId(self);
		
		int[] enemyIds = guild.getEnemyIds(guildId);
		setMenuContextIntArray(self, player, "guildEnemyIds", enemyIds);
		String[] enemies = guild.getEnemyNamesAndAbbrevs(guildId);
		int numEnemies = 0;
		if (enemies != null)
		{
			numEnemies = enemies.length;
		}
		boolean warPerms = guild.hasGuildPermission(guildId, player, guild.GUILD_PERMISSION_WAR);
		int numOptions = numEnemies;
		if (warPerms)
		{
			++numOptions;
		}
		String[] options = new String[numOptions];
		int index = 0;
		if (warPerms)
		{
			options[index++] = STR_GUILD_ADD_ENEMY;
		}
		for (int i = 0; i < numEnemies; ++i)
		{
			testAbortScript();
			options[index++] = enemies[i];
		}
		sui.listbox(
		self,
		player,
		STR_GUILD_ENEMIES_PROMPT,
		sui.OK_CANCEL,
		STR_GUILD_ENEMIES_TITLE,
		options,
		"onGuildEnemiesResponse",
		true,
		false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int showGuildSponsoredMessage(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		
		if (outOfRange(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int guildId = getStructureGuildId(self);
		
		sui.listbox(
		self,
		player,
		STR_GUILD_SPONSORED_PROMPT,
		sui.OK_CANCEL,
		STR_GUILD_SPONSORED_TITLE,
		guild.getMemberNames(guildId, true, false),
		"onGuildSponsoredResponse",
		true,
		false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int onGuildNamechangeNameResponse(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		
		if (outOfRange(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (sui.getIntButtonPressed(params) == sui.BP_OK)
		{
			String newGuildName = sui.getInputBoxText(params);
			setMenuContextString(self, player, "guildNamechangeName", newGuildName);
			sui.inputbox(
			self,
			player,
			STR_GUILD_NAMECHANGE_ABBREV_PROMPT,
			sui.OK_CANCEL,
			STR_GUILD_NAMECHANGE_ABBREV_TITLE,
			sui.INPUT_NORMAL,
			null,
			"onGuildNamechangeAbbrevResponse");
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int onGuildNamechangeAbbrevResponse(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		String newGuildName = getMenuContextString(self, player, "guildNamechangeName");
		removeMenuContextVar(self, player, "guildNamechangeName");
		
		if (outOfRange(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (sui.getIntButtonPressed(params) == sui.BP_OK)
		{
			int guildId = getStructureGuildId(self);
			String newGuildAbbrev = sui.getInputBoxText(params);
			guild.nameChange(guildId, player, newGuildName, newGuildAbbrev);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int onGuildCreateNameResponse(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		
		if (outOfRange(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (sui.getIntButtonPressed(params) == sui.BP_OK)
		{
			String newGuildName = sui.getInputBoxText(params);
			setMenuContextString(self, player, "guildCreateName", newGuildName);
			sui.inputbox(
			self,
			player,
			STR_GUILD_CREATE_ABBREV_PROMPT,
			sui.OK_CANCEL,
			STR_GUILD_CREATE_ABBREV_TITLE,
			sui.INPUT_NORMAL,
			null,
			"onGuildCreateAbbrevResponse");
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int onGuildCreateAbbrevResponse(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		String newGuildName = getMenuContextString(self, player, "guildCreateName");
		removeMenuContextVar(self, player, "guildCreateName");
		
		if (outOfRange(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (sui.getIntButtonPressed(params) == sui.BP_OK)
		{
			
			messageTo( self, "endVote", null, 0.f, false );
			
			String newGuildAbbrev = sui.getInputBoxText(params);
			guild.create(player, newGuildName, newGuildAbbrev);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int onGuildDisbandResponse(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		
		if (outOfRange(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int guildId = getStructureGuildId(self);
		
		if (sui.getIntButtonPressed(params) == sui.BP_OK)
		{
			guild.disband(guildId, player);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int onGuildSponsorResponse(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		
		if (outOfRange(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (sui.getIntButtonPressed(params) == sui.BP_OK)
		{
			String sponsorName = sui.getInputBoxText(params);
			obj_id target = utils.getNearbyPlayerByName(player, sponsorName);
			if (isIdValid(target))
			{
				if (getGuildId(target) != 0)
				{
					prose_package pp = new prose_package();
					pp.actor.set(getName(target));
					pp.stringId = SID_GUILD_SPONSOR_ALREADY_IN_GUILD;
					sendSystemMessageProse(getChatName(player), pp);
				}
				else
				{
					obj_id targetPlayerObj = getPlayerObject(target);
					if (isIdValid(targetPlayerObj) && !isIgnoring(targetPlayerObj, getFirstName(player)))
					{
						setMenuContextObjId(self, target, "sponsoring", player);
						int guildId = getStructureGuildId(self);
						
						deltadictionary dd = target.getScriptVars();
						dd.put("sponsorTerminal", self);
						
						dictionary d = new dictionary();
						d.put("prompt", guild.buildFakeLocalizedProse(STR_GUILD_SPONSOR_VERIFY_PROMPT, getName(player), guildGetName(guildId)));
						d.put("title", STR_GUILD_SPONSOR_VERIFY_TITLE);
						messageTo(target, "startGuildSponsorSui", d, 0, false);
					}
				}
			}
			else
			{
				prose_package pp = new prose_package();
				pp.actor.set(sponsorName);
				pp.stringId = SID_GUILD_SPONSOR_NOT_FOUND;
				sendSystemMessageProse(getChatName(player), pp);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int onGuildSponsorVerifyResponse(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id target = params.getObjId("player");
		obj_id player = getMenuContextObjId(self, target, "sponsoring");
		removeMenuContextVar(self, target, "sponsoring");
		
		int guildId = getStructureGuildId(self);
		
		prose_package pp = new prose_package();
		pp.actor.set(getName(target));
		if (sui.getIntButtonPressed(params) == sui.BP_OK)
		{
			guild.sponsor(guildId, player, target);
			pp.stringId = SID_GUILD_SPONSOR_ACCEPT;
		}
		else
		{
			pp.stringId = SID_GUILD_SPONSOR_DECLINE;
		}
		sendSystemMessageProse(getChatName(player), pp);
		return SCRIPT_CONTINUE;
	}
	
	
	public int onGuildMembersResponse(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		
		if (outOfRange(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int guildId = getStructureGuildId(self);
		
		if (sui.getIntButtonPressed(params) == sui.BP_OK)
		{
			int row = sui.getListboxSelectedRow(params);
			obj_id[] members = guild.getMemberIds(guildId, false, true);
			if (row >= 0 && row < members.length)
			{
				String name = guildGetMemberName(guildId, members[row]);
				setMenuContextString(self, player, "guildMemberName", name);
				sui.listbox(
				self,
				player,
				guild.buildFakeLocalizedProse(STR_GUILD_MEMBER_OPTIONS_PROMPT, name, ""),
				sui.OK_CANCEL,
				STR_GUILD_MEMBER_OPTIONS_TITLE,
				getAvailableMemberOptions(self, player),
				"onGuildMemberOptionsResponse");
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int onGuildMemberOptionsResponse(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		
		if (outOfRange(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (sui.getIntButtonPressed(params) == sui.BP_OK)
		{
			int row = sui.getListboxSelectedRow(params);
			String name = getMenuContextString(self, player, "guildMemberName");
			String[] entries = getAvailableMemberOptions(self, player);
			if (row >= 0 && row < entries.length)
			{
				if (entries[row].equals(STR_GUILD_TITLE))
				{
					chooseTitle(self, player, name);
				}
				else if (entries[row].equals(STR_GUILD_KICK))
				{
					confirmKick(self, player, name);
				}
				else if (entries[row].equals(STR_GUILD_PERMISSIONS))
				{
					selectPermissions(self, player, name);
				}
			}
		}
		removeMenuContextVar(self, player, "guildMemberName");
		return SCRIPT_CONTINUE;
	}
	
	
	public int onGuildSponsoredResponse(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		
		if (outOfRange(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int guildId = getStructureGuildId(self);
		
		if (sui.getIntButtonPressed(params) == sui.BP_OK)
		{
			int row = sui.getListboxSelectedRow(params);
			obj_id[] sponsored = guild.getMemberIds(guildId, true, false);
			if (row >= 0 && row < sponsored.length)
			{
				String[] entries = new String[2];
				entries[0] = STR_GUILD_ACCEPT;
				entries[1] = STR_GUILD_DECLINE;
				String name = guildGetMemberName(guildId, sponsored[row]);
				setMenuContextString(self, player, "guildSponsoredName", name);
				sui.listbox(
				self,
				player,
				guild.buildFakeLocalizedProse(STR_GUILD_SPONSORED_OPTIONS_PROMPT, name, ""),
				sui.OK_CANCEL,
				STR_GUILD_SPONSORED_OPTIONS_TITLE,
				entries,
				"onGuildSponsoredOptionsResponse");
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int onGuildSponsoredOptionsResponse(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		String name = getMenuContextString(self, player, "guildSponsoredName");
		removeMenuContextVar(self, player, "guildSponsoredName");
		
		if (name == null || !isIdValid(player) || outOfRange(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (sui.getIntButtonPressed(params) == sui.BP_OK)
		{
			int row = sui.getListboxSelectedRow(params);
			if (row >= 0 && row <= 1)
			{
				int guildId = getStructureGuildId(self);
				if (row == 0)
				{
					guild.accept(guildId, player, name);
				}
				else if (row == 1)
				{
					guild.kick(guildId, player, name);
				}
				showGuildSponsored(self, player);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int onGuildEnemiesResponse(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		int[] enemyIds = getMenuContextIntArray(self, player, "guildEnemyIds");
		removeMenuContextVar(self, player, "guildEnemyIds");
		
		if (outOfRange(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (sui.getIntButtonPressed(params) == sui.BP_OK)
		{
			int guildId = getStructureGuildId(self);
			if (guild.hasGuildPermission(guildId, player, guild.GUILD_PERMISSION_WAR))
			{
				int row = sui.getListboxSelectedRow(params);
				if (row == 0)
				{
					
					sui.inputbox(
					self,
					player,
					STR_GUILD_WAR_ENEMY_NAME_PROMPT,
					sui.OK_CANCEL,
					STR_GUILD_WAR_ENEMY_NAME_TITLE,
					sui.INPUT_NORMAL,
					null,
					"onGuildWarEnemyNameResponse");
				}
				else
				{
					
					if (row > 0 && row <= enemyIds.length)
					{
						int enemyId = enemyIds[row-1];
						if (guild.hasDeclaredWarAgainst(guildId, enemyId))
						{
							guild.peace(guildId, player, enemyId);
						}
						else
						{
							guild.war(guildId, player, enemyId);
						}
						showGuildEnemies(self, player);
					}
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int onGuildWarEnemyNameResponse(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		
		if (outOfRange(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (sui.getIntButtonPressed(params) == sui.BP_OK)
		{
			int guildId = getStructureGuildId(self);
			String enemyName = sui.getInputBoxText(params);
			guild.war(guildId, player, enemyName);
		}
		showGuildEnemies(self, player);
		return SCRIPT_CONTINUE;
	}
	
	
	public int onGuildKickResponse(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		String name = getMenuContextString(self, player, "guildKickName");
		removeMenuContextVar(self, player, "guildKickName");
		
		if (outOfRange(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int guildId = getStructureGuildId(self);
		
		if (sui.getIntButtonPressed(params) == sui.BP_OK)
		{
			guild.kick(guildId, player, name);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int onGuildTitleResponse(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		String name = getMenuContextString(self, player, "guildTitlePlayerName");
		removeMenuContextVar(self, player, "guildTitlePlayerName");
		
		if (outOfRange(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int guildId = getStructureGuildId(self);
		
		if (sui.getIntButtonPressed(params) == sui.BP_OK)
		{
			String newTitle = sui.getInputBoxText(params);
			guild.title(guildId, player, name, newTitle);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int onGuildInfoResponse(obj_id self, dictionary params) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int onGuildPermissionsResponse(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		String target = getMenuContextString(self, player, "guildPermissionsTarget");
		removeMenuContextVar(self, player, "guildPermissionsTarget");
		
		if (outOfRange(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int guildId = getStructureGuildId(self);
		
		if (sui.getIntButtonPressed(params) == sui.BP_OK)
		{
			int row = sui.getListboxSelectedRow(params);
			if (row >= 0 && row <= 7)
			{
				if (row == 0)
				{
					guild.togglePermission(guildId, player, target, guild.GUILD_PERMISSION_MAIL);
				}
				else if (row == 1)
				{
					guild.togglePermission(guildId, player, target, guild.GUILD_PERMISSION_SPONSOR);
				}
				else if (row == 2)
				{
					guild.togglePermission(guildId, player, target, guild.GUILD_PERMISSION_TITLE);
				}
				else if (row == 3)
				{
					guild.togglePermission(guildId, player, target, guild.GUILD_PERMISSION_ACCEPT);
				}
				else if (row == 4)
				{
					guild.togglePermission(guildId, player, target, guild.GUILD_PERMISSION_KICK);
				}
				else if (row == 5)
				{
					guild.togglePermission(guildId, player, target, guild.GUILD_PERMISSION_WAR);
				}
				else if (row == 6)
				{
					guild.togglePermission(guildId, player, target, guild.GUILD_PERMISSION_NAMECHANGE);
				}
				else if (row == 7)
				{
					guild.togglePermission(guildId, player, target, guild.GUILD_PERMISSION_DISBAND);
				}
				selectPermissions(self, player, target);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int checkDisband(obj_id self, dictionary params) throws InterruptedException
	{
		int guildId = getStructureGuildId(self);
		
		if (guildId != 0 && !guildExists(guildId))
		{
			obj_id structure = player_structure.getStructure(self);
			if (isIdValid(structure))
			{
				player_structure.destroyStructure(structure, false);
			}
		}
		else
		{
			messageTo(self, "checkDisband", null, DISBAND_CHECK_TIME, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean outOfRange(obj_id self, obj_id player) throws InterruptedException
	{
		location a = getLocation(self);
		location b = getLocation(player);
		if (a.cell == b.cell && a.distance(b) < TERMINAL_USE_DISTANCE)
		{
			return false;
		}
		return true;
	}
	
	
	public String[] getAvailableMemberOptions(obj_id self, obj_id player) throws InterruptedException
	{
		int guildId = getStructureGuildId(self);
		
		int count = 0;
		if (guild.hasGuildPermission(guildId, player, guild.GUILD_PERMISSION_TITLE))
		{
			++count;
		}
		if (guild.hasGuildPermission(guildId, player, guild.GUILD_PERMISSION_KICK))
		{
			++count;
		}
		if (player == guildGetLeader(guildId) || isGod(player))
		{
			++count;
		}
		String[] ret = new String[count];
		count = 0;
		if (guild.hasGuildPermission(guildId, player, guild.GUILD_PERMISSION_TITLE))
		{
			ret[count++] = STR_GUILD_TITLE;
		}
		if (guild.hasGuildPermission(guildId, player, guild.GUILD_PERMISSION_KICK))
		{
			ret[count++] = STR_GUILD_KICK;
		}
		if (player == guildGetLeader(guildId) || isGod(player))
		{
			ret[count++] = STR_GUILD_PERMISSIONS;
		}
		return ret;
	}
	
	
	public void chooseTitle(obj_id self, obj_id player, String name) throws InterruptedException
	{
		setMenuContextString(self, player, "guildTitlePlayerName", name);
		sui.inputbox(
		self,
		player,
		guild.buildFakeLocalizedProse(STR_GUILD_TITLE_PROMPT, name, ""),
		sui.OK_CANCEL,
		STR_GUILD_TITLE_TITLE,
		sui.INPUT_NORMAL,
		null,
		"onGuildTitleResponse");
	}
	
	
	public void confirmKick(obj_id self, obj_id player, String name) throws InterruptedException
	{
		setMenuContextString(self, player, "guildKickName", name);
		sui.msgbox(
		self,
		player,
		guild.buildFakeLocalizedProse(STR_GUILD_KICK_PROMPT, name, ""),
		sui.YES_NO,
		STR_GUILD_KICK_TITLE,
		sui.MSG_NORMAL,
		"onGuildKickResponse");
	}
	
	
	public void selectPermissions(obj_id self, obj_id player, String name) throws InterruptedException
	{
		dictionary params = new dictionary();
		params.put("player", player);
		params.put("name", name);
		messageTo(self, "selectPermissionsMessage", params, 2, false);
	}
	
	
	public void showGuildInfo(obj_id self, obj_id player) throws InterruptedException
	{
		
		int guildId = getStructureGuildId(self);
		obj_id[] memberIds = guild.getMemberIds(guildId, false, true);
		String guildInfoStr = new String (
		"Guild Name: "+ guildGetName(guildId) + "\n"+
		"Guild Abbreviation: "+ guildGetAbbrev(guildId) + "\n"+
		"Guild Leader: "+ guildGetMemberName(guildId, guildGetLeader(guildId)) + "\n"+
		"Members: "+ memberIds.length);
		
		sui.msgbox(
		self,
		player,
		guildInfoStr,
		sui.OK_ONLY,
		STR_GUILD_INFO_TITLE,
		sui.MSG_NORMAL,
		"onGuildInfoResponse");
	}
	
	
	public void showGuildMembers(obj_id self, obj_id player) throws InterruptedException
	{
		int guildId = getStructureGuildId(self);
		sui.listbox(
		self,
		player,
		STR_GUILD_MEMBERS_PROMPT,
		sui.OK_CANCEL,
		STR_GUILD_MEMBERS_TITLE,
		guild.getMemberNamesAndTitles(guildId),
		"onGuildMembersResponse",
		true,
		false);
	}
	
	
	public void showGuildEnemies(obj_id self, obj_id player) throws InterruptedException
	{
		dictionary params = new dictionary();
		params.put("player", player);
		messageTo(self, "showGuildEnemiesMessage", params, 2, false);
	}
	
	
	public void showGuildSponsored(obj_id self, obj_id player) throws InterruptedException
	{
		dictionary params = new dictionary();
		params.put("player", player);
		messageTo(self, "showGuildSponsoredMessage", params, 2, false);
	}
	
	
	public String getMenuContextString(obj_id self, obj_id player, String varName) throws InterruptedException
	{
		deltadictionary dd = self.getScriptVars();
		return dd.getString("guildMenu." + player + "." + varName);
	}
	
	
	public void setMenuContextString(obj_id self, obj_id player, String varName, String value) throws InterruptedException
	{
		deltadictionary dd = self.getScriptVars();
		dd.put("guildMenu."+ player + "."+ varName, value);
	}
	
	
	public obj_id getMenuContextObjId(obj_id self, obj_id player, String varName) throws InterruptedException
	{
		deltadictionary dd = self.getScriptVars();
		return dd.getObjId("guildMenu." + player + "." + varName);
	}
	
	
	public void setMenuContextObjId(obj_id self, obj_id player, String varName, obj_id value) throws InterruptedException
	{
		deltadictionary dd = self.getScriptVars();
		dd.put("guildMenu."+ player + "."+ varName, value);
	}
	
	
	public int[] getMenuContextIntArray(obj_id self, obj_id player, String varName) throws InterruptedException
	{
		deltadictionary dd = self.getScriptVars();
		return dd.getIntArray("guildMenu." + player + "." + varName);
	}
	
	
	public void setMenuContextIntArray(obj_id self, obj_id player, String varName, int[] value) throws InterruptedException
	{
		deltadictionary dd = self.getScriptVars();
		dd.put("guildMenu."+ player + "."+ varName, value);
	}
	
	
	public void removeMenuContextVar(obj_id self, obj_id player, String varName) throws InterruptedException
	{
		deltadictionary dd = self.getScriptVars();
		dd.remove("guildMenu."+ player + "."+ varName);
	}
	
	
	public int handleMakeLeader(obj_id self, dictionary params) throws InterruptedException
	{
		if (sui.getIntButtonPressed(params) != sui.BP_OK)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		String input_name = sui.getInputBoxText( params );
		int guildId = getStructureGuildId(self);
		
		obj_id[] members = guild.getMemberIds( guildId, false, true );
		for (int i=0; i<members.length; i++)
		{
			testAbortScript();
			String name = guildGetMemberName( guildId, members[i] );
			if (name.equals( input_name ))
			{
				if (members[i] == player)
				{
					sendSystemMessage( player, SID_ALREADY_LEADER );
					return SCRIPT_CONTINUE;
				}
				
				float dist = getDistance( self, members[i] );
				if (dist < 0 || dist > 20)
				{
					
					sendSystemMessage( player, SID_ML_NOT_LOADED );
					return SCRIPT_CONTINUE;
				}
				
				obj_id structure = player_structure.getStructure( self );
				if (!player_structure.canOwnStructure( structure, members[i] ))
				{
					
					sendSystemMessage( player, SID_ML_NO_LOTS_FREE );
					return SCRIPT_CONTINUE;
				}
				
				if (utils.isFreeTrial(player, members[i]))
				{
					
					sendSystemMessage( player, SID_ML_TRIAL );
					return SCRIPT_CONTINUE;
				}
				
				utils.setScriptVar( self, "temp_new_leader", members[i] );
				sui.msgbox( self, members[i], "@guild:make_leader_p", sui.YES_NO, "@guild:make_leader_t", sui.MSG_NORMAL, "handleAcceptLeadership");
				
				return SCRIPT_CONTINUE;
			}
		}
		
		sendSystemMessage( player, SID_ML_FAIL );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleAcceptLeadership(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id pa_hall = getTopMostContainer( self );
		int guildId = getStructureGuildId( self );
		obj_id oldLeader = guildGetLeader( guildId );
		obj_id newLeader = utils.getObjIdScriptVar( self, "temp_new_leader");
		if (sui.getIntButtonPressed(params) == sui.BP_OK)
		{
			obj_id[] members = guild.getMemberIds( guildId, false, true );
			boolean done = false;
			for (int i=0; i<members.length && !done; i++)
			{
				testAbortScript();
				if (members[i] == newLeader)
				{
					
					guild.changeLeader( guildId, newLeader );
					sendSystemMessage( oldLeader, SID_ML_SUCCESS );
					sendSystemMessage( newLeader, SID_ML_YOU_ARE_LEADER );
					done = true;
				}
			}
			if (!done)
			{
				sendSystemMessage( oldLeader, SID_ML_FAIL );
			}
		}
		else
		{
			sendSystemMessage( oldLeader, SID_ML_REJECTED );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void toggleElections(obj_id self, obj_id player) throws InterruptedException
	{
		obj_id pa_hall = getTopMostContainer( self );
		int guildId = getStructureGuildId( self );
		
		if (player == guildGetLeader(guildId))
		{
			if (hasObjVar( self, "elections" ))
			{
				
				removeObjVar( self, "elections");
				removeObjVar( pa_hall, "elections");
				
				messageTo( self, "resetVotes", null, 0.f, false );
				
				sendSystemMessage( player, SID_ELECTIONS_CLOSED );
				
				obj_id[] members = guild.getMemberIds( guildId, false, true );
				String name = guildGetMemberName( guildId, player );
				for (int i=0; i<members.length; i++)
				{
					testAbortScript();
					String cname = guildGetMemberName( guildId, members[i] );
					prose_package bodypp = prose.getPackage( CLOSED_ELECTIONS_EMAIL_BODY, name );
					utils.sendMail( CLOSED_ELECTIONS_EMAIL_SUBJECT, bodypp, cname, "Guild Management");
				}
				
			}
			else
			{
				
				int curTime = getGameTime();
				setObjVar( self, "elections", curTime );
				setObjVar( pa_hall, "elections", curTime );
				
				messageTo( self, "resetVotes", null, 0.f, false );
				
				sendSystemMessage( player, SID_ELECTIONS_OPEN );
				
				obj_id[] members = guild.getMemberIds( guildId, false, true );
				String name = guildGetMemberName( guildId, player );
				for (int i=0; i<members.length; i++)
				{
					testAbortScript();
					String cname = guildGetMemberName( guildId, members[i] );
					prose_package bodypp = prose.getPackage( OPEN_ELECTIONS_EMAIL_BODY, name );
					utils.sendMail( OPEN_ELECTIONS_EMAIL_SUBJECT, bodypp, cname, "Guild Management");
				}
			}
		}
	}
	
	
	public void showStandings(obj_id self, obj_id player) throws InterruptedException
	{
		
		obj_id pa_hall = getTopMostContainer( self );
		int guildId = getStructureGuildId( self );
		obj_id leader = guildGetLeader( guildId );
		
		Vector vote_ids = new Vector();
		vote_ids.setSize(0);
		Vector vote_counts = new Vector();
		vote_counts.setSize(0);
		
		obj_id[] members = guild.getMemberIds( guildId, false, true );
		String name = guildGetMemberName( guildId, player );
		for (int i=0; i<members.length; i++)
		{
			testAbortScript();
			obj_id vote = guildGetMemberAllegiance( guildId, members[i] );
			int found = 0;
			for (int j=0; (j < vote_ids.size()) && (found == 0); j++)
			{
				testAbortScript();
				
				if (((obj_id)(vote_ids.get(j))) == vote)
				{
					found = 1;
					vote_counts.set(j, new Integer(((Integer)(vote_counts.get(j))).intValue() + 1));
				}
			}
			if (found == 0)
			{
				
				utils.addElement( vote_ids, vote );
				utils.addElement( vote_counts, 1 );
			}
		}
		
		cleanCandidates( self, player );
		
		obj_id[] candidates = getObjIdArrayObjVar( pa_hall, "candidate_list");
		if (candidates == null)
		{
			sendSystemMessage( player, SID_NO_CANDIDATES );
			return;
		}
		
		String[] candidate_names = new String[candidates.length];
		for (int i=0; i<candidates.length; i++)
		{
			testAbortScript();
			if (candidates[i] == leader)
			{
				candidate_names[i] = "Current Leader: " + guildGetMemberName( guildId, leader ) + " -- Votes: " + getNumVotes( leader, vote_ids, vote_counts );
			}
			else
			{
				candidate_names[i] = guildGetMemberName( guildId, candidates[i] ) + " -- Votes: " + getNumVotes( candidates[i], vote_ids, vote_counts );
			}
		}
		
		sui.listbox( self, player, "@guild:leader_standings_d", sui.OK_CANCEL, "@guild:leader_standings_t", candidate_names, "handleNone", true );
	}
	
	
	public void placeVote(obj_id self, obj_id player) throws InterruptedException
	{
		
		obj_id pa_hall = getTopMostContainer( self );
		int guildId = getStructureGuildId( self );
		obj_id leader = guildGetLeader( guildId );
		
		if (!guild.hasGuildPermission( guildId, player, guild.GUILD_PERMISSION_MEMBER ))
		{
			
			return;
		}
		
		cleanCandidates( self, player );
		
		obj_id[] candidates = getObjIdArrayObjVar( pa_hall, "candidate_list");
		if (candidates == null)
		{
			candidates = new obj_id[0];
		}
		
		String[] candidate_names = new String[candidates.length+1];
		candidate_names[0] = "Abstain";
		for (int i=0; i<candidates.length; i++)
		{
			testAbortScript();
			candidate_names[i+1] = guildGetMemberName( guildId, candidates[i] );
		}
		
		sui.listbox( self, player, "@guild:leader_vote_d", sui.OK_CANCEL, "@guild:leader_vote_t", candidate_names, "handlePlaceVote", true );
	}
	
	
	public int handlePlaceVote(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id player = sui.getPlayerId( params );
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
		
		obj_id pa_hall = getTopMostContainer( self );
		int guildId = getStructureGuildId( self );
		obj_id leader = guildGetLeader( guildId );
		
		if (!guild.hasGuildPermission( guildId, player, guild.GUILD_PERMISSION_MEMBER ))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] candidates = getObjIdArrayObjVar( pa_hall, "candidate_list");
		if (candidates == null)
		{
			candidates = new obj_id[0];
		}
		
		obj_id vote = null;
		if (idx == 0)
		{
			vote = null;
		}
		else
		{
			vote = candidates[idx-1];
		}
		guildSetMemberInfo( guildId, player, guildGetMemberPermissions(guildId, player),
		guildGetMemberTitle(guildId, player), vote );
		
		prose_package pp = null;
		if (vote != null)
		{
			pp = prose.getPackage( SID_VOTE_PLACED, guildGetMemberName( guildId, vote ) );
		}
		else
		{
			pp = prose.getPackage( SID_VOTE_ABSTAIN );
		}
		sendSystemMessageProse( player, pp );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void registerToRun(obj_id self, obj_id player) throws InterruptedException
	{
		
		obj_id pa_hall = getTopMostContainer( self );
		int guildId = getStructureGuildId( self );
		obj_id leader = guildGetLeader( guildId );
		
		if (!guild.hasGuildPermission( guildId, player, guild.GUILD_PERMISSION_MEMBER ))
		{
			
			return;
		}
		
		if (utils.isFreeTrial( player ))
		{
			return;
		}
		
		cleanCandidates( self, player );
		
		obj_id[] candidates = getObjIdArrayObjVar( pa_hall, "candidate_list");
		if (candidates == null)
		{
			candidates = new obj_id[0];
		}
		
		if (isRegisteredToRun( player, self ))
		{
			sendSystemMessage( player, SID_REGISTER_DUPE );
			return;
		}
		
		obj_id[] new_candidates = new obj_id[candidates.length+1];
		for (int i=0; i<candidates.length; i++)
		{
			testAbortScript();
			new_candidates[i] = candidates[i];
		}
		new_candidates[new_candidates.length-1] = player;
		setObjVar( pa_hall, "candidate_list", new_candidates );
		guildSetMemberInfo( guildId, player, guildGetMemberPermissions(guildId, player),
		guildGetMemberTitle(guildId, player), player );
		sendSystemMessage( player, SID_REGISTER_CONGRATS );
		
	}
	
	
	public boolean isRegisteredToRun(obj_id player, obj_id self) throws InterruptedException
	{
		
		obj_id pa_hall = getTopMostContainer( self );
		obj_id[] candidates = getObjIdArrayObjVar( pa_hall, "candidate_list");
		if (candidates == null)
		{
			candidates = new obj_id[0];
		}
		
		for (int i=0; i<candidates.length; i++)
		{
			testAbortScript();
			if (candidates[i] == player)
			{
				return true;
			}
		}
		return false;
	}
	
	
	public void unregisterFromRace(obj_id self, obj_id player) throws InterruptedException
	{
		
		obj_id pa_hall = getTopMostContainer( self );
		int guildId = getStructureGuildId( self );
		obj_id leader = guildGetLeader( guildId );
		
		cleanCandidates( self, player );
		
		obj_id[] candidates = getObjIdArrayObjVar( pa_hall, "candidate_list");
		if (candidates == null)
		{
			candidates = new obj_id[0];
		}
		
		if (!isRegisteredToRun( player, self ))
		{
			sendSystemMessage( player, SID_NOT_REGISTERED );
			return;
		}
		
		if (candidates.length == 1)
		{
			removeObjVar( pa_hall, "candidate_list");
		}
		else
		{
			int j = 0;
			obj_id[] new_candidates = new obj_id[candidates.length-1];
			for (int i=0; i<candidates.length; i++)
			{
				testAbortScript();
				if (candidates[i] != player)
				{
					new_candidates[j++] = candidates[i];
				}
			}
			setObjVar( pa_hall, "candidate_list", new_candidates );
		}
		
		obj_id[] members = guild.getMemberIds( guildId, false, true );
		String name = guildGetMemberName( guildId, player );
		for (int i=0; i<members.length; i++)
		{
			testAbortScript();
			
			obj_id vote = guildGetMemberAllegiance( guildId, members[i] );
			if (vote == player)
			{
				
				guildSetMemberInfo( guildId, members[i],
				guildGetMemberPermissions(guildId, members[i]), guildGetMemberTitle(guildId, members[i]), null );
			}
		}
		
		sendSystemMessage( player, SID_UNREGISTERED );
	}
	
	
	public int resetVotes(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id pa_hall = getTopMostContainer( self );
		removeObjVar( pa_hall, "candidate_list");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int startVote(obj_id self, dictionary params) throws InterruptedException
	{
		
		int guildId = getStructureGuildId( self );
		obj_id pa_hall = getTopMostContainer( self );
		int curTime = getGameTime();
		setObjVar( self, "elections", curTime );
		setObjVar( pa_hall, "elections", curTime );
		
		messageTo( self, "resetVotes", null, 0.f, false );
		
		obj_id[] members = guild.getMemberIds( guildId, false, true );
		for (int i=0; i<members.length; i++)
		{
			testAbortScript();
			String cname = guildGetMemberName( guildId, members[i] );
			prose_package bodypp = prose.getPackage( OPEN_ELECTIONS_ABSENT_EMAIL_BODY, cname );
			utils.sendMail( OPEN_ELECTIONS_ABSENT_EMAIL_SUBJECT, bodypp, cname, "Guild Management");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int endVote(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id pa_hall = getTopMostContainer( self );
		removeObjVar( self, "elections");
		removeObjVar( pa_hall, "elections");
		
		messageTo( self, "resetVotes", null, 0.f, false );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void cleanCandidates(obj_id self, obj_id player) throws InterruptedException
	{
		
		obj_id pa_hall = getTopMostContainer( self );
		int guildId = getStructureGuildId( self );
		
		obj_id[] candidates = getObjIdArrayObjVar( pa_hall, "candidate_list");
		if (candidates == null)
		{
			return;
		}
		
		int[] bad_array = new int[candidates.length];
		int bad_entries = 0;
		for (int i=0; i<candidates.length; i++)
		{
			testAbortScript();
			if (guildGetMemberName( guildId, candidates[i] ) == null)
			{
				bad_array[i] = 1;
				bad_entries++;
			}
			else
			{
				bad_array[i] = 0;
			}
		}
		
		if (candidates.length-bad_entries == 0)
		{
			removeObjVar( pa_hall, "candidate_list");
			return;
		}
		
		int j = 0;
		obj_id[] new_candidates = new obj_id[candidates.length-bad_entries];
		for (int i=0; i<candidates.length; i++)
		{
			testAbortScript();
			if (bad_array[i] == 0)
			{
				new_candidates[j++] = candidates[i];
			}
		}
		
		if (new_candidates.length > 0)
		{
			setObjVar( pa_hall, "candidate_list", new_candidates );
		}
		else
		{
			if (hasObjVar( pa_hall, "candidate_list" ))
			{
				removeObjVar( pa_hall, "candidate_list");
			}
		}
	}
	
	
	public String getNumVotes(obj_id candidate, Vector vote_ids, Vector vote_counts) throws InterruptedException
	{
		for (int j=0; j<vote_ids.size(); j++)
		{
			testAbortScript();
			if (candidate == (obj_id) vote_ids.elementAt(j))
			{
				Integer votes = (Integer) vote_counts.elementAt(j);
				return "" + votes;
			}
		}
		return "0";
	}
	
	
	public void acceptPAHall(obj_id self, obj_id player) throws InterruptedException
	{
		sui.msgbox( self, player, "@guild:accept_pa_hall_p", sui.YES_NO, "@guild:accept_pa_hall_t", sui.MSG_NORMAL, "handleAcceptPAHall");
	}
	
	
	public int handleAcceptPAHall(obj_id self, dictionary params) throws InterruptedException
	{
		if (sui.getIntButtonPressed(params) == sui.BP_OK)
		{
			obj_id pa_hall = getTopMostContainer( self );
			int guildId = getStructureGuildId( self );
			obj_id player = guildGetLeader( guildId );
			
			obj_id structure = player_structure.getStructure( self );
			if (!player_structure.canOwnStructure( structure, player ))
			{
				
				sendSystemMessage( player, SID_NO_LOTS );
				return SCRIPT_CONTINUE;
			}
			
			obj_id hall_owner = getOwner( structure );
			player_structure.removeStructure( structure, hall_owner );
			player_structure.addStructure( structure, player );
			
			sendSystemMessage( player, SID_PA_OWNER_NOW );
		}
		
		return SCRIPT_CONTINUE;
	}
}
