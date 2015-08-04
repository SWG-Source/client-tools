package script.terminal;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.city;
import script.library.prose;
import script.library.sui;
import script.library.utils;
import java.util.Vector;



public class city_vote extends script.terminal.base.base_terminal
{
	public city_vote()
	{
	}
	public static final string_id SID_MAYORAL_RACE = new string_id("city/city", "mayoral_race");
	public static final string_id SID_MAYORAL_STANDINGS = new string_id("city/city", "mayoral_standings");
	public static final string_id SID_MAYORAL_VOTE = new string_id("city/city", "mayoral_vote");
	public static final string_id SID_MAYORAL_REGISTER = new string_id("city/city", "mayoral_register");
	public static final string_id SID_MAYORAL_UNREGISTER = new string_id("city/city", "mayoral_unregister");
	
	public static final string_id SID_RESET_VOTING = new string_id("city/city", "reset_voting");
	
	public static final string_id SID_REGISTER_INCUMBENT = new string_id("city/city", "register_incumbent");
	public static final string_id SID_REGISTER_NONCITIZEN = new string_id("city/city", "register_noncitizen");
	public static final string_id SID_REGISTER_NONPOLITICIAN = new string_id("city/city", "register_nonpolitician");
	public static final string_id SID_REGISTER_DUPE = new string_id("city/city", "register_dupe");
	public static final string_id SID_REGISTER_CONGRATS = new string_id("city/city", "register_congrats");
	public static final string_id SID_REGISTER_TIMESTAMP = new string_id("city/city", "register_timestamp");
	
	public static final string_id SID_REGISTRATION_LOCKED = new string_id("city/city", "registration_locked");
	
	public static final string_id REGISTERED_CITIZEN_EMAIL_BODY = new string_id("city/city", "rceb");
	public static final string_id REGISTERED_CITIZEN_EMAIL_SUBJECT = new string_id("city/city", "registered_citizen_email_subject");
	public static final string_id UNREGISTERED_CITIZEN_EMAIL_BODY = new string_id("city/city", "unregistered_citizen_email_body");
	public static final string_id UNREGISTERED_CITIZEN_EMAIL_SUBJECT = new string_id("city/city", "unregistered_citizen_email_subject");
	
	public static final string_id SID_NOT_REGISTERED = new string_id("city/city", "not_registered");
	public static final string_id SID_UNREGISTERED = new string_id("city/city", "unregistered_race");
	
	public static final string_id SID_NO_CANDIDATES = new string_id("city/city", "no_candidates");
	
	public static final string_id SID_VOTE_NONCITIZEN = new string_id("city/city", "vote_noncitizen");
	public static final string_id SID_VOTE_PLACED = new string_id("city/city", "vote_placed");
	public static final string_id SID_VOTE_ABSTAIN = new string_id("city/city", "vote_abstain");
	
	public static final string_id SID_ALREADY_MAYOR = new string_id("city/city", "already_mayor");
	
	public static final String STF_FILE = "city/city";
	
	public static final string_id SID_NOT_OLD_ENOUGH = new string_id("city/city", "not_old_enough");
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		
		dictionary outparams = new dictionary();
		outparams.put( "terminal", self );
		obj_id city_hall = getTopMostContainer( self );
		messageTo( city_hall, "registerVoteTerminal", outparams, 0.f, true );
		
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
			
			obj_id city_hall = getTopMostContainer( self );
			if (isValidId(city_hall) && exists(city_hall))
			{
				int city_id = findCityByCityHall( city_hall );
				if (city_id > -1)
				{
					names[idx] = "city_name";
					attribs[idx] = cityGetName( city_id );
					idx++;
					
					obj_id mayor = cityGetLeader(city_id);
					if (isValidId(mayor))
					{
						names[idx] = "current_mayor";
						attribs[idx] = cityGetCitizenName(city_id, mayor);
						idx++;
					}
					
					int currentInterval = getIntObjVar(city_hall, "cityVoteInterval");
					if (currentInterval > -1)
					{
						String intervalName = convertInterval(currentInterval);
						if (!intervalName.equals("error"))
						{
							names[idx] = "vote_interval";
							attribs[idx] = intervalName;
							idx++;
						}
					}
					
					int nextUpdate = (getIntObjVar(city_hall, "lastUpdateTime") + getIntObjVar(city_hall, "currentInterval")) - getGameTime();
					if (nextUpdate > -1)
					{
						names[idx] = "next_interval";
						attribs[idx] = utils.assembleTimeRemainToUse(nextUpdate);
						idx++;
					}
					
					obj_id[] candidates = getObjIdArrayObjVar(city_hall, "candidate_list");
					if (candidates != null)
					{
						for (int i=0; i < candidates.length; i++)
						{
							testAbortScript();
							if (candidates[i] == mayor)
							{
								names[idx] = "incumbent";
								attribs[idx] = cityGetCitizenName(city_id, mayor);
								idx++;
							}
							else
							{
								names[idx] = "candidate";
								attribs[idx] = cityGetCitizenName(city_id, candidates[i]);
								idx++;
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
		obj_id city_hall = getTopMostContainer( self );
		int city_id = findCityByCityHall( city_hall );
		obj_id mayor = cityGetLeader( city_id );
		
		int menu = mi.addRootMenu( menu_info_types.SERVER_MENU1, SID_MAYORAL_RACE );
		
		mi.addSubMenu( menu, menu_info_types.SERVER_MENU2, SID_MAYORAL_STANDINGS );
		mi.addSubMenu( menu, menu_info_types.SERVER_MENU3, SID_MAYORAL_VOTE );
		
		if (!isRegisteredToRun( player, self ))
		{
			mi.addSubMenu( menu, menu_info_types.SERVER_MENU4, SID_MAYORAL_REGISTER );
		}
		else
		{
			mi.addSubMenu( menu, menu_info_types.SERVER_MENU5, SID_MAYORAL_UNREGISTER );
		}
		
		if (isGod( player ))
		{
			mi.addSubMenu( menu, menu_info_types.SERVER_MENU6, SID_RESET_VOTING );
		}
		
		return super.OnObjectMenuRequest(self, player, mi);
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (item == menu_info_types.SERVER_MENU1 || item == menu_info_types.SERVER_MENU2)
		{
			
			showStandings( self, player );
		}
		else if (item == menu_info_types.SERVER_MENU3)
		{
			
			placeVote( self, player );
		}
		else if (item == menu_info_types.SERVER_MENU4)
		{
			
			registerToRun( self, player );
		}
		else if (item == menu_info_types.SERVER_MENU5)
		{
			
			unregisterFromRace( self, player );
		}
		else if (item == menu_info_types.SERVER_MENU6)
		{
			
			if (isGod( player ))
			{
				obj_id city_hall = getTopMostContainer( self );
				int city_id = findCityByCityHall( city_hall );
				obj_id[] citizens = cityGetCitizenIds( city_id );
				for (int i=0; i<citizens.length; i++)
				{
					testAbortScript();
					city.setCitizenAllegiance( city_id, citizens[i], null );
				}
				removeObjVar( city_hall, "candidate_list");
				sendSystemMessage(self, new string_id(STF_FILE, "voting_reset_request"));
				
				CustomerServiceLog( "player_city", "City voting has been reset by request. Hall: "+ city_hall + " GM: "+ player );
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void showStandings(obj_id self, obj_id player) throws InterruptedException
	{
		
		obj_id city_hall = getTopMostContainer( self );
		int city_id = findCityByCityHall( city_hall );
		obj_id mayor = cityGetLeader( city_id );
		
		Vector vote_ids = new Vector();
		vote_ids.setSize(0);
		Vector vote_counts = new Vector();
		vote_counts.setSize(0);
		obj_id[] citizens = cityGetCitizenIds( city_id );
		for (int i=0; i<citizens.length; i++)
		{
			testAbortScript();
			obj_id vote = cityGetCitizenAllegiance( city_id, citizens[i] );
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
		
		obj_id[] candidates = getObjIdArrayObjVar( city_hall, "candidate_list");
		if (candidates == null)
		{
			sendSystemMessage( player, SID_NO_CANDIDATES );
			return;
		}
		
		String[] candidate_names = new String[candidates.length];
		for (int i=0; i<candidates.length; i++)
		{
			testAbortScript();
			if (candidates[i] == mayor)
			{
				candidate_names[i] = "Incumbent: " + cityGetCitizenName( city_id, mayor ) + " -- Votes: " + getNumVotes( mayor, vote_ids, vote_counts );
			}
			else
			{
				candidate_names[i] = cityGetCitizenName( city_id, candidates[i] ) + " -- Votes: " + getNumVotes( candidates[i], vote_ids, vote_counts );
			}
		}
		
		sui.listbox( self, player, "@city/city:mayoral_standings_d", sui.OK_CANCEL, "@city/city:mayoral_standings_t", candidate_names, "handleNone", true );
	}
	
	
	public void placeVote(obj_id self, obj_id player) throws InterruptedException
	{
		
		obj_id city_hall = getTopMostContainer( self );
		int city_id = findCityByCityHall( city_hall );
		obj_id mayor = cityGetLeader( city_id );
		
		if (!city.isCitizenOfCity( player, city_id ))
		{
			sendSystemMessage( player, SID_VOTE_NONCITIZEN );
			return;
		}
		
		if (!isOldEnough(player) && !isGod(player))
		{
			sendSystemMessage( player, SID_NOT_OLD_ENOUGH);
			return;
		}
		
		cleanCandidates( self, player );
		
		obj_id[] candidates = getObjIdArrayObjVar( city_hall, "candidate_list");
		if (candidates == null)
		{
			candidates = new obj_id[0];
		}
		
		String[] candidate_names = new String[candidates.length+1];
		
		candidate_names[0] = "Abstain";
		for (int i=0; i<candidates.length; i++)
		{
			testAbortScript();
			candidate_names[i+1] = cityGetCitizenName( city_id, candidates[i] );
		}
		
		sui.listbox( self, player, "@city/city:mayoral_vote_d", sui.OK_CANCEL, "@city/city:mayoral_vote_t", candidate_names, "handlePlaceVote", true );
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
		
		obj_id city_hall = getTopMostContainer( self );
		int city_id = findCityByCityHall( city_hall );
		obj_id mayor = cityGetLeader( city_id );
		
		if (!city.isCitizenOfCity( player, city_id ))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] candidates = getObjIdArrayObjVar( city_hall, "candidate_list");
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
		city.setCitizenAllegiance( city_id, player, vote );
		
		prose_package pp = null;
		if (vote != null)
		{
			pp = prose.getPackage( SID_VOTE_PLACED, cityGetCitizenName( city_id, vote ) );
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
		
		obj_id city_hall = getTopMostContainer( self );
		int city_id = findCityByCityHall( city_hall );
		
		obj_id mayor = cityGetLeader( city_id );
		
		if (city.isAMayor( player ) && (player != mayor))
		{
			sendSystemMessage( player, SID_ALREADY_MAYOR );
			return;
		}
		
		if (!city.isCitizenOfCity( player, city_id ))
		{
			sendSystemMessage( player, SID_REGISTER_NONCITIZEN );
			return;
		}
		
		if (!hasSkill( player, "social_politician_novice" ))
		{
			sendSystemMessage( player, SID_REGISTER_NONPOLITICIAN );
			return;
		}
		
		int cityVoteInterval = getIntObjVar( city_hall, "cityVoteInterval");
		if (cityVoteInterval == 2)
		{
			sendSystemMessage( player, SID_REGISTRATION_LOCKED );
			return;
		}
		
		int lastCityVoteReg = getIntObjVar( player, "lastCityVoteReg");
		if (!isGod( player ) && (getGameTime() - lastCityVoteReg < (60*60*24)))
		{
			sendSystemMessage( player, SID_REGISTER_TIMESTAMP );
			return;
		}
		
		cleanCandidates( self, player );
		
		obj_id[] candidates = getObjIdArrayObjVar( city_hall, "candidate_list");
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
		setObjVar( city_hall, "candidate_list", new_candidates );
		city.setCitizenAllegiance( city_id, player, player );
		sendSystemMessage( player, SID_REGISTER_CONGRATS );
		
		setObjVar( player, "lastCityVoteReg", getGameTime() );
		
		obj_id[] citizens = cityGetCitizenIds( city_id );
		if (citizens != null)
		{
			String pname = cityGetCitizenName( city_id, player );
			for (int i=0; i<citizens.length; i++)
			{
				testAbortScript();
				String cname = cityGetCitizenName( city_id, citizens[i] );
				prose_package bodypp = prose.getPackage( REGISTERED_CITIZEN_EMAIL_BODY, pname );
				utils.sendMail( REGISTERED_CITIZEN_EMAIL_SUBJECT, bodypp, cname, "Planetary Civic Authority");
			}
		}
	}
	
	
	public boolean isRegisteredToRun(obj_id player, obj_id self) throws InterruptedException
	{
		
		obj_id city_hall = getTopMostContainer( self );
		obj_id[] candidates = getObjIdArrayObjVar( city_hall, "candidate_list");
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
		
		obj_id city_hall = getTopMostContainer( self );
		int city_id = findCityByCityHall( city_hall );
		
		cleanCandidates( self, player );
		
		obj_id[] candidates = getObjIdArrayObjVar( city_hall, "candidate_list");
		if (candidates == null)
		{
			candidates = new obj_id[0];
		}
		
		int cityVoteInterval = getIntObjVar( city_hall, "cityVoteInterval");
		if (cityVoteInterval == 2)
		{
			sendSystemMessage( player, SID_REGISTRATION_LOCKED );
			return;
		}
		
		if (!isRegisteredToRun( player, self ))
		{
			sendSystemMessage( player, SID_NOT_REGISTERED );
			return;
		}
		
		if (candidates.length == 1)
		{
			removeObjVar( city_hall, "candidate_list");
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
			setObjVar( city_hall, "candidate_list", new_candidates );
		}
		
		obj_id[] citizens = cityGetCitizenIds( city_id );
		if (citizens != null)
		{
			String pname = cityGetCitizenName( city_id, player );
			for (int i=0; i<citizens.length; i++)
			{
				testAbortScript();
				
				obj_id vote = cityGetCitizenAllegiance( city_id, citizens[i] );
				if (vote == player)
				{
					city.setCitizenAllegiance( city_id, citizens[i], null );
				}
				
				String cname = cityGetCitizenName( city_id, citizens[i] );
				prose_package bodypp = prose.getPackage( UNREGISTERED_CITIZEN_EMAIL_BODY, pname );
				utils.sendMail( UNREGISTERED_CITIZEN_EMAIL_SUBJECT, bodypp, cname, "Planetary Civic Authority");
			}
		}
		
		sendSystemMessage( player, SID_UNREGISTERED );
	}
	
	
	public int resetVoteTerminal(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id city_hall = getTopMostContainer( self );
		removeObjVar( city_hall, "candidate_list");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void cleanCandidates(obj_id self, obj_id player) throws InterruptedException
	{
		
		obj_id city_hall = getTopMostContainer( self );
		int city_id = findCityByCityHall( city_hall );
		
		obj_id[] candidates = getObjIdArrayObjVar( city_hall, "candidate_list");
		if (candidates == null)
		{
			return;
		}
		
		int[] bad_array = new int[candidates.length];
		int bad_entries = 0;
		for (int i=0; i<candidates.length; i++)
		{
			testAbortScript();
			if (cityGetCitizenName( city_id, candidates[i] ) == null)
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
			removeObjVar( city_hall, "candidate_list");
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
			setObjVar( city_hall, "candidate_list", new_candidates );
		}
		else
		{
			if (hasObjVar( city_hall, "candidate_list" ))
			{
				removeObjVar( city_hall, "candidate_list");
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
	
	
	public boolean isOldEnough(obj_id player) throws InterruptedException
	{
		boolean isOldEnough = false;
		
		int timeData = getPlayerBirthDate(player);
		int rightNow = getCurrentBirthDate();
		int delta = rightNow - timeData;
		
		if (delta > 7 || isGod(player))
		{
			isOldEnough = true;
		}
		
		return isOldEnough;
	}
	
	
	public String convertInterval(int currentInterval) throws InterruptedException
	{
		if (currentInterval == 0)
		{
			return "Voting Week 1";
		}
		if (currentInterval == 1)
		{
			return "Voting Week 2";
		}
		if (currentInterval == 2)
		{
			return "Election Week";
		}
		return "error";
	}
}
