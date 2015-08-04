package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;



public class guild extends script.base_script
{
	public guild()
	{
	}
	public static final int GUILD_PERMISSION_MEMBER = (1<<0);
	public static final int GUILD_PERMISSION_SPONSOR = (1<<1);
	public static final int GUILD_PERMISSION_DISBAND = (1<<2);
	public static final int GUILD_PERMISSION_ACCEPT = (1<<3);
	public static final int GUILD_PERMISSION_KICK = (1<<4);
	public static final int GUILD_PERMISSION_MAIL = (1<<5);
	public static final int GUILD_PERMISSION_TITLE = (1<<6);
	public static final int GUILD_PERMISSION_NAMECHANGE = (1<<7);
	public static final int GUILD_PERMISSION_WAR = (1<<8);
	
	public static final int GUILD_PERMISSIONS_NONE = 0;
	public static final int GUILD_PERMISSIONS_ALL = -1;
	
	public static final String GUILD_MAIL_DISBAND_SUBJECT = "@guildmail:disband_subject";
	public static final string_id GUILD_MAIL_DISBAND_TEXT = new string_id("guildmail", "disband_text");
	public static final string_id GUILD_MAIL_DISBAND_NOT_ENOUGH_MEMBERS_TEXT = new string_id("guildmail", "disband_not_enough_members_text");
	public static final String GUILD_MAIL_SPONSOR_SUBJECT = "@guildmail:sponsor_subject";
	public static final string_id GUILD_MAIL_SPONSOR_TEXT = new string_id("guildmail", "sponsor_text");
	public static final String GUILD_MAIL_ACCEPT_SUBJECT = "@guildmail:accept_subject";
	public static final string_id GUILD_MAIL_ACCEPT_TEXT = new string_id("guildmail", "accept_text");
	public static final String GUILD_MAIL_ACCEPT_TARGET_SUBJECT = "@guildmail:accept_target_subject";
	public static final string_id GUILD_MAIL_ACCEPT_TARGET_TEXT = new string_id("guildmail", "accept_target_text");
	public static final String GUILD_MAIL_KICK_SUBJECT = "@guildmail:kick_subject";
	public static final string_id GUILD_MAIL_KICK_TEXT = new string_id("guildmail", "kick_text");
	public static final String GUILD_MAIL_DECLINE_SUBJECT = "@guildmail:decline_subject";
	public static final string_id GUILD_MAIL_DECLINE_TEXT = new string_id("guildmail", "decline_text");
	public static final String GUILD_MAIL_DECLINE_TARGET_SUBJECT = "@guildmail:decline_target_subject";
	public static final string_id GUILD_MAIL_DECLINE_TARGET_TEXT = new string_id("guildmail", "decline_target_text");
	public static final String GUILD_MAIL_NAMECHANGE_SUBJECT = "@guildmail:namechange_subject";
	public static final string_id GUILD_MAIL_NAMECHANGE_TEXT = new string_id("guildmail", "namechange_text");
	public static final String GUILD_MAIL_LEADERCHANGE_SUBJECT = "@guildmail:leaderchange_subject";
	public static final string_id GUILD_MAIL_LEADERCHANGE_TEXT = new string_id("guildmail", "leaderchange_text");
	public static final String GUILD_MAIL_LEAVE_SUBJECT = "@guildmail:leave_subject";
	public static final string_id GUILD_MAIL_LEAVE_TEXT = new string_id("guildmail", "leave_text");
	
	public static final string_id SID_GUILD_CREATE_FAIL_IN_GUILD = new string_id ("guild", "create_fail_in_guild");
	public static final string_id SID_GUILD_CREATE_FAIL_NAME_BAD_LENGTH = new string_id ("guild", "create_fail_name_bad_length");
	public static final string_id SID_GUILD_CREATE_FAIL_ABBREV_BAD_LENGTH = new string_id ("guild", "create_fail_abbrev_bad_length");
	public static final string_id SID_GUILD_CREATE_FAIL_NAME_IN_USE = new string_id ("guild", "create_fail_name_in_use");
	public static final string_id SID_GUILD_CREATE_FAIL_ABBREV_IN_USE = new string_id ("guild", "create_fail_abbrev_in_use");
	public static final string_id SID_GUILD_CREATE_FAIL_NAME_NOT_ALLOWED = new string_id ("guild", "create_fail_name_not_allowed");
	public static final string_id SID_GUILD_CREATE_FAIL_ABBREV_NOT_ALLOWED = new string_id ("guild", "create_fail_abbrev_not_allowed");
	public static final string_id SID_GUILD_SPONSOR_FAIL_FULL = new string_id ("guild", "sponsor_fail_full");
	public static final string_id SID_GUILD_SPONSOR_ALREADY_IN_GUILD = new string_id ("guild", "sponsor_already_in_guild");
	public static final string_id SID_GUILD_TITLE_FAIL_BAD_LENGTH = new string_id ("guild", "title_fail_bad_length");
	public static final string_id SID_GUILD_TITLE_FAIL_NOT_ALLOWED = new string_id ("guild", "title_fail_not_allowed");
	public static final string_id SID_GUILD_GENERIC_FAIL_NO_PERMISSION = new string_id ("guild", "generic_fail_no_permission");
	public static final string_id SID_GUILD_LEAVE_FAIL_LEADER_TRIED_TO_LEAVE = new string_id ("guild", "leave_fail_leader_tried_to_leave");
	public static final string_id SID_GUILD_WAR_FAIL_NO_SUCH_GUILD = new string_id ("guild", "war_fail_no_such_guild");
	public static final string_id SID_GUILD_KICK_SELF = new string_id ("guild", "kick_self");
	public static final string_id SID_GUILD_KICK_TARGET = new string_id ("guild", "kick_target");
	public static final string_id SID_GUILD_DECLINE_SELF = new string_id ("guild", "decline_self");
	public static final string_id SID_GUILD_DECLINE_TARGET = new string_id ("guild", "decline_target");
	public static final string_id SID_GUILD_SPONSOR_SELF = new string_id ("guild", "sponsor_self");
	public static final string_id SID_GUILD_SPONSOR_TARGET = new string_id ("guild", "sponsor_target");
	public static final string_id SID_GUILD_ACCEPT_SELF = new string_id ("guild", "accept_self");
	public static final string_id SID_GUILD_ACCEPT_TARGET = new string_id ("guild", "accept_target");
	public static final string_id SID_GUILD_LEAVE_SELF = new string_id ("guild", "leave_self");
	public static final string_id SID_GUILD_TITLE_SELF = new string_id ("guild", "title_self");
	public static final string_id SID_GUILD_TITLE_TARGET = new string_id ("guild", "title_target");
	public static final string_id SID_GUILD_NAMECHANGE_FAIL_ABBREV_BAD_LENGTH = new string_id ("guild", "namechange_fail_abbrev_bad_length");
	public static final string_id SID_GUILD_NAMECHANGE_FAIL_NAME_BAD_LENGTH = new string_id ("guild", "namechange_fail_name_bad_length");
	public static final string_id SID_GUILD_NAMECHANGE_FAIL_ABBREV_NOT_ALLOWED = new string_id ("guild", "namechange_fail_abbrev_not_allowed");
	public static final string_id SID_GUILD_NAMECHANGE_FAIL_NAME_NOT_ALLOWED = new string_id ("guild", "namechange_fail_name_not_allowed");
	public static final string_id SID_GUILD_NAMECHANGE_SELF = new string_id ("guild", "namechange_self");
	public static final string_id SID_GUILD_ALLEGIANCE_SELF = new string_id ("guild", "allegiance_self");
	public static final string_id SID_GUILD_ALLEGIANCE_UNCHANGED_SELF = new string_id ("guild", "allegiance_unchanged_self");
	
	public static final int MAX_MEMBERS_PER_GUILD = 500;
	
	
	public static String resolveGuildName(int guildId) throws InterruptedException
	{
		if (guildId != 0)
		{
			String guildName = guildGetName(guildId);
			if (guildName != null && !guildName.equals(""))
			{
				return guildName;
			}
			return "unknown";
		}
		return "none";
	}
	
	
	public static String resolveMemberName(int guildId, obj_id who) throws InterruptedException
	{
		if (isIdValid(who))
		{
			
			{
				String s = getName(who);
				if (s != null && !s.equals(""))
				{
					return s;
				}
			}
			
			{
				String s = guildGetMemberName(guildId, who);
				if (s != null && !s.equals(""))
				{
					return s;
				}
			}
			return "unknown";
		}
		return "none";
	}
	
	
	public static void GuildLog(int guildId, String command, obj_id actor, obj_id target, String description) throws InterruptedException
	{
		String guildDesc = resolveGuildName(guildId);
		String actorDesc = resolveMemberName(guildId, actor);
		
		CustomerServiceLog("Guild", "[command="+command+"] [guild="+guildId+"("+guildDesc+")] [actor="+actor+"("+resolveMemberName(guildId, actor)+")] [target="+target+"("+resolveMemberName(guildId, target)+")] - "+ description);
	}
	
	
	public static String buildFakeLocalizedProse(String rawText, String substitute1, String substitute2) throws InterruptedException
	{
		if (rawText.startsWith("@"))
		{
			String cookedText = new String(rawText);
			
			string_id id = new string_id(rawText.substring(1, rawText.indexOf(":")), rawText.substring(rawText.indexOf(":")+1, rawText.length()));
			cookedText = localize(id);
			
			int pos1 = cookedText.indexOf("%TU");
			if (pos1 != -1)
			{
				cookedText = cookedText.substring(0, pos1) + substitute1 + cookedText.substring(pos1+3, cookedText.length());
			}
			
			int pos2 = cookedText.indexOf("%TT");
			if (pos2 != -1)
			{
				cookedText = cookedText.substring(0, pos2) + substitute2 + cookedText.substring(pos2+3, cookedText.length());
			}
			return cookedText;
		}
		else
		{
			return rawText;
		}
	}
	
	
	public static boolean hasGuildPermission(int guildId, obj_id who, int permission) throws InterruptedException
	{
		if (guildId == 0)
		{
			return false;
		}
		if (isGod(who))
		{
			return true;
		}
		if ((guildGetMemberPermissions(guildId, who) & permission) == permission)
		{
			return true;
		}
		return false;
	}
	
	
	public static boolean hasDeclaredWarAgainst(int guildId, int enemyId) throws InterruptedException
	{
		int[] enemies = guildGetEnemies(guildId);
		if (enemies != null)
		{
			for (int i = 0; i < enemies.length; ++i)
			{
				testAbortScript();
				if (enemies[i] == enemyId)
				{
					return true;
				}
			}
		}
		return false;
	}
	
	
	public static void create(obj_id actor, String guildName, String guildAbbrev) throws InterruptedException
	{
		if (getGuildId(actor) != 0)
		{
			sendSystemMessage(actor, SID_GUILD_CREATE_FAIL_IN_GUILD);
		}
		else if (guildAbbrev.length() < 1 || guildAbbrev.length() > 5)
		{
			sendSystemMessage(actor, SID_GUILD_CREATE_FAIL_ABBREV_BAD_LENGTH);
		}
		else if (guildName.length() < 1 || guildName.length() > 25)
		{
			sendSystemMessage(actor, SID_GUILD_CREATE_FAIL_NAME_BAD_LENGTH);
		}
		else if (findGuild(guildAbbrev) != 0)
		{
			sendSystemMessage(actor, SID_GUILD_CREATE_FAIL_ABBREV_IN_USE);
		}
		else if (findGuild(guildName) != 0)
		{
			sendSystemMessage(actor, SID_GUILD_CREATE_FAIL_NAME_IN_USE);
		}
		else if (isNameReserved(guildAbbrev))
		{
			sendSystemMessage(actor, SID_GUILD_CREATE_FAIL_ABBREV_NOT_ALLOWED);
		}
		else if (isNameReserved(guildName))
		{
			sendSystemMessage(actor, SID_GUILD_CREATE_FAIL_NAME_NOT_ALLOWED);
		}
		else
		{
			int guildId = createGuild(guildName, guildAbbrev);
			if (guildId != 0)
			{
				GuildLog(guildId, "create", actor, null, "guildName='"+guildName+"' guildAbbrev='"+guildAbbrev+"'");
				guildSetMemberInfo(guildId, actor, GUILD_PERMISSIONS_ALL, "", actor);
				guildSetLeader(guildId, actor);
			}
		}
	}
	
	
	public static void changeLeader(int guildId, obj_id newLeader) throws InterruptedException
	{
		obj_id oldLeader = guildGetLeader(guildId);
		if (oldLeader != newLeader && isIdValid(newLeader))
		{
			GuildLog(guildId, "changeLeader", null, newLeader, "");
			mailToGuild(guildId, GUILD_MAIL_LEADERCHANGE_SUBJECT, GUILD_MAIL_LEADERCHANGE_TEXT, guildGetMemberName(guildId, newLeader));
			obj_id[] memberIds = getMemberIds(guildId, false, true);
			
			if (findObjIdTableOffset(memberIds, oldLeader) != -1)
			{
				guildSetMemberInfo(
				guildId,
				oldLeader,
				GUILD_PERMISSION_MEMBER,
				guildGetMemberTitle(guildId, oldLeader),
				guildGetMemberAllegiance(guildId, oldLeader));
			}
			
			guildSetLeader(guildId, newLeader);
			
			if (findObjIdTableOffset(memberIds, newLeader) != -1)
			{
				guildSetMemberInfo(
				guildId,
				newLeader,
				GUILD_PERMISSIONS_ALL,
				guildGetMemberTitle(guildId, newLeader),
				guildGetMemberAllegiance(guildId, newLeader));
			}
		}
	}
	
	
	public static void disbandForNotEnoughMembers(int guildId) throws InterruptedException
	{
		GuildLog(guildId, "disbandForNotEnoughMembers", null, null, "");
		mailToGuild(guildId, GUILD_MAIL_DISBAND_SUBJECT, GUILD_MAIL_DISBAND_NOT_ENOUGH_MEMBERS_TEXT);
		disbandGuild(guildId);
	}
	
	
	public static void disband(int guildId, obj_id actor) throws InterruptedException
	{
		if (!hasGuildPermission(guildId, actor, GUILD_PERMISSION_DISBAND))
		{
			sendSystemMessage(actor, SID_GUILD_GENERIC_FAIL_NO_PERMISSION);
		}
		else
		{
			GuildLog(guildId, "disband", actor, null, "");
			mailToGuild(guildId, GUILD_MAIL_DISBAND_SUBJECT, GUILD_MAIL_DISBAND_TEXT, getName(actor));
			disbandGuild(guildId);
		}
	}
	
	
	public static void sponsor(int guildId, obj_id actor, obj_id who) throws InterruptedException
	{
		if (isIdValid(who))
		{
			if (getGuildId(who) != 0)
			{
				prose_package pp = new prose_package();
				pp.actor.set(getName(who));
				pp.stringId = SID_GUILD_SPONSOR_ALREADY_IN_GUILD;
				sendSystemMessageProse(getChatName(actor), pp);
			}
			else if (!hasGuildPermission(guildId, actor, GUILD_PERMISSION_SPONSOR))
			{
				sendSystemMessage(actor, SID_GUILD_GENERIC_FAIL_NO_PERMISSION);
			}
			else
			{
				obj_id[] members = guildGetMemberIds(guildId);
				if (members.length >= MAX_MEMBERS_PER_GUILD)
				{
					sendSystemMessage(actor, SID_GUILD_SPONSOR_FAIL_FULL);
				}
				else
				{
					GuildLog(guildId, "sponsor", actor, who, "");
					prose_package pp = new prose_package();
					pp.target.set(guildGetName(guildId));
					pp.actor.set(getName(who));
					pp.stringId = SID_GUILD_SPONSOR_SELF;
					sendSystemMessageProse(getChatName(actor), pp);
					pp.actor.set(getName(actor));
					pp.stringId = SID_GUILD_SPONSOR_TARGET;
					sendSystemMessageProse(getChatName(who), pp);
					obj_id leader = guildGetLeader(guildId);
					if (isIdValid(leader))
					{
						mailToGuildMember(guildId, leader, GUILD_MAIL_SPONSOR_SUBJECT, GUILD_MAIL_SPONSOR_TEXT, getName(actor), getName(who));
					}
					guildSetMemberInfo(guildId, who, GUILD_PERMISSIONS_NONE, "", obj_id.NULL_ID);
				}
			}
		}
	}
	
	
	public static void accept(int guildId, obj_id actor, String who) throws InterruptedException
	{
		if (!hasGuildPermission(guildId, actor, GUILD_PERMISSION_ACCEPT))
		{
			sendSystemMessage(actor, SID_GUILD_GENERIC_FAIL_NO_PERMISSION);
		}
		else
		{
			obj_id memberId = findMemberIdByName(guildId, who, true, false);
			if (isIdValid(memberId))
			{
				GuildLog(guildId, "accept", actor, memberId, "");
				String memberName = guildGetMemberName(guildId, memberId);
				prose_package pp = new prose_package();
				pp.target.set(guildGetName(guildId));
				pp.actor.set(memberName);
				pp.stringId = SID_GUILD_ACCEPT_SELF;
				sendSystemMessageProse(getChatName(actor), pp);
				pp.actor.set(getName(actor));
				pp.stringId = SID_GUILD_ACCEPT_TARGET;
				sendSystemMessageProse(memberName, pp);
				
				mailToGuild(guildId, GUILD_MAIL_ACCEPT_SUBJECT, GUILD_MAIL_ACCEPT_TEXT, getName(actor), memberName);
				
				guildSetMemberInfo(guildId, memberId, GUILD_PERMISSION_MEMBER, "", guildGetLeader(guildId));
				
				mailToPerson(guildId, memberName, GUILD_MAIL_ACCEPT_TARGET_SUBJECT, GUILD_MAIL_ACCEPT_TARGET_TEXT, getName(actor), guildGetName(guildId));
			}
		}
	}
	
	
	public static void leave(obj_id actor) throws InterruptedException
	{
		int guildId = getGuildId(actor);
		if (guildId != 0)
		{
			
			if (actor == guildGetLeader(guildId))
			{
				sendSystemMessage(actor, SID_GUILD_LEAVE_FAIL_LEADER_TRIED_TO_LEAVE);
			}
			else
			{
				GuildLog(guildId, "leave", actor, null, "");
				prose_package pp = new prose_package();
				pp.actor.set(guildGetName(guildId));
				pp.stringId = SID_GUILD_LEAVE_SELF;
				sendSystemMessageProse(getChatName(actor), pp);
				mailToGuild(guildId, GUILD_MAIL_LEAVE_SUBJECT, GUILD_MAIL_LEAVE_TEXT, getName(actor));
				guildRemoveMember(guildId, actor);
			}
		}
	}
	
	
	public static void kick(int guildId, obj_id actor, String who) throws InterruptedException
	{
		if (guildId == 0)
		{
			return;
		}
		
		obj_id memberId = findMemberIdByName(guildId, who, false, true);
		if (isIdValid(memberId))
		{
			
			if (memberId == actor)
			{
				leave(actor);
			}
			
			else if (!hasGuildPermission(guildId, actor, GUILD_PERMISSION_KICK) || (!isGod(actor) && memberId == guildGetLeader(guildId)))
			{
				sendSystemMessage(actor, SID_GUILD_GENERIC_FAIL_NO_PERMISSION);
			}
			else
			{
				GuildLog(guildId, "kick", actor, memberId, "removing member");
				String memberName = guildGetMemberName(guildId, memberId);
				prose_package pp = new prose_package();
				pp.target.set(guildGetName(guildId));
				pp.actor.set(memberName);
				pp.stringId = SID_GUILD_KICK_SELF;
				sendSystemMessageProse(getChatName(actor), pp);
				pp.actor.set(getName(actor));
				pp.stringId = SID_GUILD_KICK_TARGET;
				sendSystemMessageProse(memberName, pp);
				mailToGuild(guildId, GUILD_MAIL_KICK_SUBJECT, GUILD_MAIL_KICK_TEXT, getName(actor), memberName);
				guildRemoveMember(guildId, memberId);
			}
		}
		
		else
		{
			obj_id sponsoredId = findMemberIdByName(guildId, who, true, false);
			if (isIdValid(sponsoredId))
			{
				if (!hasGuildPermission(guildId, actor, GUILD_PERMISSION_ACCEPT))
				{
					sendSystemMessage(actor, SID_GUILD_GENERIC_FAIL_NO_PERMISSION);
				}
				else
				{
					GuildLog(guildId, "kick", actor, memberId, "removing sponsored");
					String memberName = guildGetMemberName(guildId, sponsoredId);
					prose_package pp = new prose_package();
					pp.target.set(guildGetName(guildId));
					pp.actor.set(memberName);
					pp.stringId = SID_GUILD_DECLINE_SELF;
					sendSystemMessageProse(getChatName(actor), pp);
					pp.actor.set(getName(actor));
					pp.stringId = SID_GUILD_DECLINE_TARGET;
					sendSystemMessageProse(memberName, pp);
					
					mailToGuild(guildId, GUILD_MAIL_DECLINE_SUBJECT, GUILD_MAIL_DECLINE_TEXT, getName(actor), memberName);
					
					guildRemoveMember(guildId, sponsoredId);
					
					mailToPerson(guildId, memberName, GUILD_MAIL_DECLINE_TARGET_SUBJECT, GUILD_MAIL_DECLINE_TARGET_TEXT, getName(actor), guildGetName(guildId));
				}
			}
		}
	}
	
	
	public static void title(int guildId, obj_id actor, String who, String title) throws InterruptedException
	{
		if (!hasGuildPermission(guildId, actor, GUILD_PERMISSION_TITLE))
		{
			sendSystemMessage(actor, SID_GUILD_GENERIC_FAIL_NO_PERMISSION);
		}
		else
		{
			obj_id memberId = findMemberIdByName(guildId, who, false, true);
			if (isIdValid(memberId))
			{
				if (isNameReserved(title))
				{
					sendSystemMessage(actor, SID_GUILD_TITLE_FAIL_NOT_ALLOWED);
				}
				else if (title.length() > 25)
				{
					sendSystemMessage(actor, SID_GUILD_TITLE_FAIL_BAD_LENGTH);
				}
				else
				{
					GuildLog(guildId, "title", actor, memberId, "setting title to '"+title+"'");
					String memberName = guildGetMemberName(guildId, memberId);
					prose_package pp = new prose_package();
					pp.target.set(title);
					pp.actor.set(memberName);
					pp.stringId = SID_GUILD_TITLE_SELF;
					sendSystemMessageProse(getChatName(actor), pp);
					pp.actor.set(getName(actor));
					pp.stringId = SID_GUILD_TITLE_TARGET;
					sendSystemMessageProse(memberName, pp);
					guildSetMemberInfo(
					guildId,
					memberId,
					guildGetMemberPermissions(guildId, memberId),
					title,
					guildGetMemberAllegiance(guildId, memberId));
				}
			}
		}
	}
	
	
	public static void handleGuildNameChange(int guildId, String newName, String newAbbrev, obj_id changerId) throws InterruptedException
	{
		
		if (isIdValid(changerId) && !hasGuildPermission(guildId, changerId, GUILD_PERMISSION_NAMECHANGE))
		{
			return;
		}
		
		if (findGuild(newName) != 0 || findGuild(newAbbrev) != 0)
		{
			return;
		}
		
		GuildLog(guildId, "handleGuildNameChange", changerId, null, "name='"+newName+"' abbrev='"+newAbbrev+"'");
		
		guildSetName(guildId, newName);
		guildSetAbbrev(guildId, newAbbrev);
		
		String memberName = null;
		
		if (isIdValid(changerId))
		{
			memberName = guildGetMemberName(guildId, changerId);
		}
		if (memberName != null)
		{
			mailToGuild(guildId, GUILD_MAIL_NAMECHANGE_SUBJECT, GUILD_MAIL_NAMECHANGE_TEXT, newName, newAbbrev, memberName);
		}
		else
		{
			mailToGuild(guildId, GUILD_MAIL_NAMECHANGE_SUBJECT, GUILD_MAIL_NAMECHANGE_TEXT, newName, newAbbrev, "System");
		}
	}
	
	
	public static void nameChange(int guildId, obj_id actor, String newName, String newAbbrev) throws InterruptedException
	{
		if (!hasGuildPermission(guildId, actor, GUILD_PERMISSION_NAMECHANGE))
		{
			sendSystemMessage(actor, SID_GUILD_GENERIC_FAIL_NO_PERMISSION);
		}
		else
		{
			if (newAbbrev.length() < 1 || newAbbrev.length() > 5)
			{
				sendSystemMessage(actor, SID_GUILD_NAMECHANGE_FAIL_ABBREV_BAD_LENGTH);
			}
			else if (newName.length() < 1 || newName.length() > 25)
			{
				sendSystemMessage(actor, SID_GUILD_NAMECHANGE_FAIL_NAME_BAD_LENGTH);
			}
			else if (isNameReserved(newAbbrev))
			{
				sendSystemMessage(actor, SID_GUILD_NAMECHANGE_FAIL_ABBREV_NOT_ALLOWED);
			}
			else if (isNameReserved(newName))
			{
				sendSystemMessage(actor, SID_GUILD_NAMECHANGE_FAIL_NAME_NOT_ALLOWED);
			}
			else
			{
				GuildLog(guildId, "nameChange", actor, null, "queueing change to name='"+newName+"' abbrev='"+newAbbrev+"'");
				prose_package pp = new prose_package();
				pp.actor.set(newName);
				pp.target.set(newAbbrev);
				pp.stringId = SID_GUILD_NAMECHANGE_SELF;
				sendSystemMessageProse(getChatName(actor), pp);
				obj_id masterGuildObj = getMasterGuildObject();
				dictionary params = new dictionary();
				params.put("guildId", guildId);
				params.put("newName", newName);
				params.put("newAbbrev", newAbbrev);
				if (getGuildId(actor) == guildId)
				{
					params.put("changerId", actor);
				}
				else
				{
					params.put("changerId", obj_id.NULL_ID);
				}
				messageTo(masterGuildObj, "initiateGuildNameChange", params, 0, false);
			}
		}
	}
	
	
	public static void allegiance(obj_id actor, String who) throws InterruptedException
	{
		int guildId = getGuildId(actor);
		if (guildId != 0)
		{
			obj_id allegianceId = findMemberIdByName(guildId, who, false, true);
			if (isIdValid(allegianceId))
			{
				GuildLog(guildId, "allegiance", actor, allegianceId, "");
				prose_package pp = new prose_package();
				pp.actor.set(guildGetMemberName(guildId, allegianceId));
				pp.target.set(guildGetName(guildId));
				pp.stringId = SID_GUILD_ALLEGIANCE_SELF;
				sendSystemMessageProse(getChatName(actor), pp);
				guildSetMemberInfo(
				guildId,
				actor,
				guildGetMemberPermissions(guildId, actor),
				guildGetMemberTitle(guildId, actor),
				allegianceId);
			}
			else
			{
				sendSystemMessage(actor, SID_GUILD_ALLEGIANCE_UNCHANGED_SELF);
			}
		}
	}
	
	
	public static void war(int guildId, obj_id actor, String who) throws InterruptedException
	{
		int enemyId = findGuild(who);
		if (enemyId == 0)
		{
			sendSystemMessage(actor, SID_GUILD_WAR_FAIL_NO_SUCH_GUILD);
		}
		else
		{
			war(guildId, actor, enemyId);
		}
	}
	
	
	public static void war(int guildId, obj_id actor, int enemyId) throws InterruptedException
	{
		if (enemyId != 0 && enemyId != guildId)
		{
			if (!hasGuildPermission(guildId, actor, GUILD_PERMISSION_WAR))
			{
				sendSystemMessage(actor, SID_GUILD_GENERIC_FAIL_NO_PERMISSION);
			}
			else
			{
				GuildLog(guildId, "war", actor, null, "Declaring war with "+enemyId+"("+guildGetName(enemyId)+")");
				guildSetEnemy(guildId, enemyId);
			}
		}
	}
	
	
	public static void peace(int guildId, obj_id actor, String who) throws InterruptedException
	{
		peace(guildId, actor, findGuild(who));
	}
	
	
	public static void peace(int guildId, obj_id actor, int enemyId) throws InterruptedException
	{
		if (enemyId != 0 && enemyId != guildId)
		{
			if (!hasGuildPermission(guildId, actor, GUILD_PERMISSION_WAR))
			{
				sendSystemMessage(actor, SID_GUILD_GENERIC_FAIL_NO_PERMISSION);
			}
			else
			{
				GuildLog(guildId, "war", actor, null, "Declaring peace with "+enemyId+"("+guildGetName(enemyId)+")");
				guildRemoveEnemy(guildId, enemyId);
			}
		}
	}
	
	
	public static void togglePermission(int guildId, obj_id actor, String who, int permBit) throws InterruptedException
	{
		if (guildGetLeader(guildId) == actor || isGod(actor))
		{
			obj_id memberId = findMemberIdByName(guildId, who, false, true);
			if (isIdValid(memberId))
			{
				GuildLog(guildId, "togglePermission", actor, memberId, "bit "+permBit);
				guildSetMemberInfo(
				guildId,
				memberId,
				guildGetMemberPermissions(guildId, memberId) ^ permBit,
				guildGetMemberTitle(guildId, memberId),
				guildGetMemberAllegiance(guildId, memberId));
			}
		}
	}
	
	
	public static void dumpInfo(int guildId, obj_id actor) throws InterruptedException
	{
		debugConsoleMsg(actor, "Guild Id: "+ guildId);
		if (guildId == 0)
		{
			return;
		}
		
		debugConsoleMsg(actor, "Guild Name: "+ guildGetName(guildId));
		debugConsoleMsg(actor, "Guild Abbrev: "+ guildGetAbbrev(guildId));
		debugConsoleMsg(actor, "Guild Leader: "+ guildGetLeader(guildId));
		
		obj_id[] members = guildGetMemberIds(guildId);
		int memberCount = 0;
		if (members != null)
		{
			memberCount = members.length;
		}
		debugConsoleMsg(actor, "Guild members ("+ memberCount + "):");
		for (int i = 0; i < memberCount; ++i)
		{
			testAbortScript();
			debugConsoleMsg(actor, (i+1) + ")" + " id = "+ members[i]
			+ " name = "+ guildGetMemberName(guildId, members[i])
			+ " perms = "+ guildGetMemberPermissions(guildId, members[i])
			+ " title = "+ guildGetMemberTitle(guildId, members[i])
			+ " allegiance = "+ guildGetMemberAllegiance(guildId, members[i]));
		}
		
		int[] enemies = guildGetEnemies(guildId);
		int enemyCount = 0;
		if (enemies != null)
		{
			enemyCount = enemies.length;
		}
		debugConsoleMsg(actor, "Guild enemies ("+ enemyCount + "):");
		for (int i = 0; i < enemyCount; ++i)
		{
			testAbortScript();
			debugConsoleMsg(actor, (i+1) + ") "+ enemies[i]);
		}
	}
	
	
	public static obj_id findMemberIdByName(int guildId, String name, boolean fromSponsored, boolean fromMembers) throws InterruptedException
	{
		obj_id[] members = getMemberIds(guildId, fromSponsored, fromMembers);
		if (members != null)
		{
			
			java.util.StringTokenizer st = new java.util.StringTokenizer(name);
			String compareName = toLower(st.nextToken());
			
			for (int i = 0; i < members.length; ++i)
			{
				testAbortScript();
				
				java.util.StringTokenizer st2 = new java.util.StringTokenizer(guildGetMemberName(guildId, members[i]));
				String memberName = toLower(st2.nextToken());
				if (compareName.equals(memberName))
				{
					return members[i];
				}
			}
		}
		return obj_id.NULL_ID;
	}
	
	
	public static boolean hasSponsoredMembers(int guildId) throws InterruptedException
	{
		obj_id[] members = guildGetMemberIds(guildId);
		if (members != null)
		{
			for (int i = 0; i < members.length; ++i)
			{
				testAbortScript();
				if (guildGetMemberPermissions(guildId, members[i]) == GUILD_PERMISSIONS_NONE)
				{
					return true;
				}
			}
		}
		return false;
	}
	
	
	public static obj_id[] getMemberIds(int guildId, boolean fromSponsored, boolean fromMembers) throws InterruptedException
	{
		obj_id[] members = guildGetMemberIds(guildId);
		int count = 0;
		if (members != null)
		{
			if (fromSponsored && fromMembers)
			{
				count = members.length;
			}
			else
			{
				for (int i = 0; i < members.length; ++i)
				{
					testAbortScript();
					if (guildGetMemberPermissions(guildId, members[i]) == GUILD_PERMISSIONS_NONE)
					{
						if (fromSponsored)
						{
							++count;
						}
					}
					else if (fromMembers)
					{
						++count;
					}
				}
			}
		}
		obj_id[] ret = new obj_id[count];
		int index = 0;
		for (int i = 0; i < members.length; ++i)
		{
			testAbortScript();
			if (guildGetMemberPermissions(guildId, members[i]) == GUILD_PERMISSIONS_NONE)
			{
				if (fromSponsored)
				{
					ret[index++] = members[i];
				}
			}
			else if (fromMembers)
			{
				ret[index++] = members[i];
			}
		}
		return ret;
	}
	
	
	public static String[] getMemberNames(int guildId, boolean fromSponsored, boolean fromMembers) throws InterruptedException
	{
		obj_id[] members = getMemberIds(guildId, fromSponsored, fromMembers);
		int count = 0;
		if (members != null)
		{
			count = members.length;
		}
		String[] ret = new String[count];
		for (int i = 0; i < count; ++i)
		{
			testAbortScript();
			ret[i] = guildGetMemberName(guildId, members[i]);
		}
		
		Arrays.sort(ret);
		return ret;
	}
	
	
	public static String[] getMemberNamesAndTitles(int guildId) throws InterruptedException
	{
		obj_id[] members = getMemberIds(guildId, false, true);
		int count = 0;
		if (members != null)
		{
			count = members.length;
		}
		String[] ret = new String[count];
		for (int i = 0; i < count; ++i)
		{
			testAbortScript();
			String name = guildGetMemberName(guildId, members[i]);
			String title = guildGetMemberTitle(guildId, members[i]);
			if (title.equals(""))
			{
				ret[i] = name;
			}
			else
			{
				ret[i] = name + " [" + title + "]";
			}
		}
		Arrays.sort(ret);
		return ret;
	}
	
	
	public static String[] getEnemyNamesAndAbbrevs(int guildId) throws InterruptedException
	{
		int[] enemies_A_to_B = guildGetEnemies(guildId);
		int[] enemies_B_to_A = getGuildsAtWarWith(guildId);
		int count = 0;
		if (enemies_A_to_B != null)
		{
			count = enemies_A_to_B.length;
		}
		if (enemies_B_to_A != null)
		{
			for (int i = 0; i < enemies_B_to_A.length; ++i)
			{
				testAbortScript();
				if (findIntTableOffset(enemies_A_to_B, enemies_B_to_A[i]) == -1)
				{
					++count;
				}
			}
		}
		String[] ret = new String[count];
		int pos = 0;
		if (enemies_A_to_B != null)
		{
			for (int i = 0; i < enemies_A_to_B.length; ++i)
			{
				testAbortScript();
				StringBuffer sb = new StringBuffer();
				if (findIntTableOffset(enemies_B_to_A, enemies_A_to_B[i]) == -1)
				{
					sb.append("> ");
				}
				else
				{
					sb.append("= ");
				}
				sb.append(guildGetName(enemies_A_to_B[i]));
				sb.append(" <");
				sb.append(guildGetAbbrev(enemies_A_to_B[i]));
				sb.append(">");
				ret[pos] = sb.toString();
				++pos;
			}
		}
		if (enemies_B_to_A != null)
		{
			for (int i = 0; i < enemies_B_to_A.length; ++i)
			{
				testAbortScript();
				if (findIntTableOffset(enemies_A_to_B, enemies_B_to_A[i]) == -1)
				{
					StringBuffer sb = new StringBuffer();
					sb.append("< ");
					sb.append(guildGetName(enemies_B_to_A[i]));
					sb.append(" <");
					sb.append(guildGetAbbrev(enemies_B_to_A[i]));
					sb.append(">");
					ret[pos] = sb.toString();
					++pos;
				}
			}
		}
		Arrays.sort(ret);
		return ret;
	}
	
	
	public static int[] getEnemyIds(int guildId) throws InterruptedException
	{
		int[] enemies_A_to_B = guildGetEnemies(guildId);
		int[] enemies_B_to_A = getGuildsAtWarWith(guildId);
		int count = 0;
		if (enemies_A_to_B != null)
		{
			count = enemies_A_to_B.length;
		}
		if (enemies_B_to_A != null)
		{
			for (int i = 0; i < enemies_B_to_A.length; ++i)
			{
				testAbortScript();
				if (findIntTableOffset(enemies_A_to_B, enemies_B_to_A[i]) == -1)
				{
					++count;
				}
			}
		}
		int[] ret = new int[count];
		int pos = 0;
		if (enemies_A_to_B != null)
		{
			for (int i = 0; i < enemies_A_to_B.length; ++i)
			{
				testAbortScript();
				ret[pos++] = enemies_A_to_B[i];
			}
		}
		if (enemies_B_to_A != null)
		{
			for (int i = 0; i < enemies_B_to_A.length; ++i)
			{
				testAbortScript();
				if (findIntTableOffset(enemies_A_to_B, enemies_B_to_A[i]) == -1)
				{
					ret[pos++] = enemies_B_to_A[i];
				}
			}
		}
		return ret;
	}
	
	
	public static void mailToGuild(int guildId, String subject, string_id textId, String substitute1, String substitute2, String substitute3) throws InterruptedException
	{
		obj_id[] members = getMemberIds(guildId, false, true);
		if (members != null)
		{
			for (int i = 0; i < members.length; ++i)
			{
				testAbortScript();
				mailToGuildMember(guildId, members[i], subject, textId, substitute1, substitute2, substitute3);
			}
		}
	}
	
	
	public static void mailToGuild(int guildId, String subject, string_id textId, String substitute1, String substitute2) throws InterruptedException
	{
		mailToGuild(guildId, subject, textId, substitute1, substitute2, "");
	}
	
	
	public static void mailToGuild(int guildId, String subject, string_id textId, String substitute1) throws InterruptedException
	{
		mailToGuild(guildId, subject, textId, substitute1, "", "");
	}
	
	
	public static void mailToGuild(int guildId, String subject, string_id textId) throws InterruptedException
	{
		mailToGuild(guildId, subject, textId, "", "", "");
	}
	
	
	public static void mailToPerson(int guildId, String toName, String subject, string_id textId, String substitute1, String substitute2, String substitute3) throws InterruptedException
	{
		prose_package pp = new prose_package();
		pp.stringId = textId;
		pp.actor.set(substitute1);
		pp.target.set(substitute2);
		pp.other.set(substitute3);
		chatSendPersistentMessage(
		guildGetName(guildId),
		toName,
		subject,
		null,
		chatMakePersistentMessageOutOfBandBody(null, pp));
	}
	
	
	public static void mailToPerson(int guildId, String toName, String subject, string_id textId, String substitute1, String substitute2) throws InterruptedException
	{
		mailToPerson(guildId, toName, subject, textId, substitute1, substitute2, "");
	}
	
	
	public static void mailToPerson(int guildId, String toName, String subject, string_id textId, String substitute1) throws InterruptedException
	{
		mailToPerson(guildId, toName, subject, textId, substitute1, "", "");
	}
	
	
	public static void mailToPerson(int guildId, String toName, String subject, string_id textId) throws InterruptedException
	{
		mailToPerson(guildId, toName, subject, textId, "", "", "");
	}
	
	
	public static void mailToGuildMember(int guildId, obj_id memberId, String subject, string_id textId, String substitute1, String substitute2, String substitute3) throws InterruptedException
	{
		mailToPerson(guildId, guildGetMemberName(guildId, memberId), subject, textId, substitute1, substitute2, substitute3);
	}
	
	
	public static void mailToGuildMember(int guildId, obj_id memberId, String subject, string_id textId, String substitute1, String substitute2) throws InterruptedException
	{
		mailToGuildMember(guildId, memberId, subject, textId, substitute1, substitute2, "");
	}
	
	
	public static void mailToGuildMember(int guildId, obj_id memberId, String subject, string_id textId, String substitute1) throws InterruptedException
	{
		mailToGuildMember(guildId, memberId, subject, textId, substitute1, "", "");
	}
	
	
	public static void mailToGuildMember(int guildId, obj_id memberId, String subject, string_id textId) throws InterruptedException
	{
		mailToGuildMember(guildId, memberId, subject, textId, "", "", "");
	}
	
	
	public static int findIntTableOffset(int[] from, int find) throws InterruptedException
	{
		if (from != null)
		{
			for (int i = 0; i < from.length; ++i)
			{
				testAbortScript();
				if (from[i] == find)
				{
					return i;
				}
			}
		}
		return -1;
	}
	
	
	public static int findObjIdTableOffset(obj_id[] from, obj_id find) throws InterruptedException
	{
		if (from != null)
		{
			for (int i = 0; i < from.length; ++i)
			{
				testAbortScript();
				if (from[i] == find)
				{
					return i;
				}
			}
		}
		return -1;
	}
	
	
	public static obj_id determineLeaderByAllegiance(int guildId) throws InterruptedException
	{
		
		obj_id[] memberIds = getMemberIds(guildId, false, true);
		if (memberIds.length > 0)
		{
			int[] allegianceCounts = new int[memberIds.length];
			
			for (int i = 0; i < memberIds.length; ++i)
			{
				testAbortScript();
				int offset = findObjIdTableOffset(memberIds, guildGetMemberAllegiance(guildId, memberIds[i]));
				if (offset != -1)
				{
					allegianceCounts[offset]++;
				}
			}
			
			obj_id oldLeader = guildGetLeader(guildId);
			obj_id newLeader = memberIds[0];
			int newLeaderAllegianceCount = allegianceCounts[0];
			for (int i = 1; i < memberIds.length; ++i)
			{
				testAbortScript();
				if ((allegianceCounts[i] > newLeaderAllegianceCount || (allegianceCounts[i] == newLeaderAllegianceCount && memberIds[i] == oldLeader)))
				{
					newLeader = memberIds[i];
					newLeaderAllegianceCount = allegianceCounts[i];
				}
			}
			return newLeader;
		}
		
		return obj_id.NULL_ID;
	}
	
}
