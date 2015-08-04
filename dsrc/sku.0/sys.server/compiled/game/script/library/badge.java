package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.prose;
import script.library.respec;


public class badge extends script.base_script
{
	public badge()
	{
	}
	public static final String SFX_REVOKE = "sound/music_gloom_a.snd";
	public static final String STF_COLLECTION_N = "collection_n";
	public static final String BADGE_BOOK = "badge_book";
	
	public static final string_id SID_PROSE_GRANT = new string_id(STF_COLLECTION_N, "prose_grant");
	public static final string_id SID_PROSE_REVOKE = new string_id(STF_COLLECTION_N, "prose_revoke");
	public static final string_id SID_PROSE_HASBADGE = new string_id(STF_COLLECTION_N, "prose_hasbadge");
	public static final string_id SID_REWARD_BADGE = new string_id("collection","reward_badge");
	
	
	public static boolean grantBadge(obj_id player, String badge) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		if ((badge == null) || (badge.length() == 0))
		{
			return false;
		}
		
		String[] info = getCollectionSlotInfo(badge);
		if ((info == null) || (info.length != COLLECTION_INFO_ARRAY_SIZE) || (info[COLLECTION_INFO_INDEX_BOOK] == null) || (!info[COLLECTION_INFO_INDEX_BOOK].equals(BADGE_BOOK)))
		{
			return false;
		}
		
		if (hasCompletedCollectionSlot(player, badge))
		{
			return false;
		}
		
		if (!modifyCollectionSlotValue(player, badge, 1))
		{
			return false;
		}
		
		prose_package pp = new prose_package();
		prose.setStringId(pp, SID_REWARD_BADGE);
		prose.setTU(pp, new string_id("collection_n", badge));
		prose.setTO(pp, new string_id("collection_n", info[COLLECTION_INFO_INDEX_COLLECTION]));
		
		sendSystemMessageProse(player, pp);
		
		if ((info[COLLECTION_INFO_INDEX_MUSIC] != null) && (toUpper(info[COLLECTION_INFO_INDEX_MUSIC]) != "NONE"))
		{
			playMusic(player, info[COLLECTION_INFO_INDEX_MUSIC]);
		}
		
		boolean isAccumulationBadge = false;
		String[] slotInfo = getCollectionSlotInfo(badge);
		
		if ((slotInfo != null) && (slotInfo.length > 0))
		{
			if ((slotInfo[COLLECTION_INFO_INDEX_PAGE] != null) && (slotInfo[COLLECTION_INFO_INDEX_PAGE].equals("bdg_accumulation")))
			{
				isAccumulationBadge = true;
			}
		}
		
		if (!isAccumulationBadge)
		{
			checkBadgeCount(player);
		}
		
		return true;
	}
	
	
	public static boolean[] grantBadge(obj_id[] players, String badge) throws InterruptedException
	{
		if (players == null || players.length == 0)
		{
			boolean[] emptyReturnList = new boolean[1];
			emptyReturnList[0] = false;
			return emptyReturnList;
		}
		
		boolean[] returnList = new boolean[players.length];
		
		for (int i=0; i<players.length; i++)
		{
			testAbortScript();
			returnList[i] = grantBadge(players[i], badge);
		}
		
		return returnList;
	}
	
	
	public static boolean revokeBadge(obj_id player, String badge, boolean tellPlayer) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		if ((badge == null) || (badge.equals("")))
		{
			return false;
		}
		
		String[] info = getCollectionSlotInfo(badge);
		if ((info == null) || (info.length != COLLECTION_INFO_ARRAY_SIZE) || (info[COLLECTION_INFO_INDEX_BOOK] == null) || (!info[COLLECTION_INFO_INDEX_BOOK].equals(BADGE_BOOK)))
		{
			return false;
		}
		
		if (!hasCompletedCollectionSlot(player, badge))
		{
			return false;
		}
		
		if (!modifyCollectionSlotValue(player, badge, -1))
		{
			return false;
		}
		
		if (tellPlayer)
		{
			prose_package pp = new prose_package();
			pp.stringId = SID_PROSE_REVOKE;
			pp.other.set(new string_id(STF_COLLECTION_N, badge));
			
			playMusic(player, SFX_REVOKE);
			sendSystemMessageProse(player, pp);
		}
		
		boolean isAccumulationBadge = false;
		String[] slotInfo = getCollectionSlotInfo(badge);
		if ((slotInfo != null) && (slotInfo.length > 0))
		{
			if ((slotInfo[COLLECTION_INFO_INDEX_PAGE] != null) && (slotInfo[COLLECTION_INFO_INDEX_PAGE].equals("bdg_accumulation")))
			{
				isAccumulationBadge = true;
			}
		}
		
		if (!isAccumulationBadge)
		{
			checkBadgeCount(player);
		}
		
		return true;
	}
	
	
	public static boolean hasBadge(obj_id player, String badge) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		if ((badge == null) || (badge.equals("")))
		{
			return false;
		}
		
		String[] info = getCollectionSlotInfo(badge);
		if ((info == null) || (info.length != COLLECTION_INFO_ARRAY_SIZE) || (info[COLLECTION_INFO_INDEX_BOOK] == null) || (!info[COLLECTION_INFO_INDEX_BOOK].equals(BADGE_BOOK)))
		{
			return false;
		}
		
		return hasCompletedCollectionSlot(player, badge);
	}
	
	
	public static boolean notifyHasBadge(obj_id player, String badge) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		if ((badge == null) || (badge.equals("")))
		{
			return false;
		}
		
		String[] info = getCollectionSlotInfo(badge);
		if ((info == null) || (info.length != COLLECTION_INFO_ARRAY_SIZE) || (info[COLLECTION_INFO_INDEX_BOOK] == null) || (!info[COLLECTION_INFO_INDEX_BOOK].equals(BADGE_BOOK)))
		{
			return false;
		}
		
		if (hasCompletedCollectionSlot(player, badge))
		{
			prose_package pp = new prose_package();
			pp.stringId = SID_PROSE_HASBADGE;
			pp.other.set(new string_id(STF_COLLECTION_N, badge));
			sendSystemMessageProse(player, pp);
			return true;
		}
		
		return false;
	}
	
	
	public static boolean checkBadgeCount(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		String[] badges = getCompletedCollectionSlotsInBook(player, BADGE_BOOK);
		if (badges != null)
		{
			if (badges.length >= 500)
			{
				grantBadge(player, "count_500");
			}
			else
			{
				revokeBadge(player, "count_500", true);
			}
			
			if (badges.length >= 475)
			{
				grantBadge(player, "count_475");
			}
			else
			{
				revokeBadge(player, "count_475", true);
			}
			
			if (badges.length >= 450)
			{
				grantBadge(player, "count_450");
			}
			else
			{
				revokeBadge(player, "count_450", true);
			}
			
			if (badges.length >= 425)
			{
				grantBadge(player, "count_425");
			}
			else
			{
				revokeBadge(player, "count_425", true);
			}
			
			if (badges.length >= 400)
			{
				grantBadge(player, "count_400");
			}
			else
			{
				revokeBadge(player, "count_400", true);
			}
			
			if (badges.length >= 375)
			{
				grantBadge(player, "count_375");
			}
			else
			{
				revokeBadge(player, "count_375", true);
			}
			
			if (badges.length >= 350)
			{
				grantBadge(player, "count_350");
			}
			else
			{
				revokeBadge(player, "count_350", true);
			}
			
			if (badges.length >= 325)
			{
				grantBadge(player, "count_325");
			}
			else
			{
				revokeBadge(player, "count_325", true);
			}
			
			if (badges.length >= 300)
			{
				grantBadge(player, "count_300");
			}
			else
			{
				revokeBadge(player, "count_300", true);
			}
			
			if (badges.length >= 275)
			{
				grantBadge(player, "count_275");
			}
			else
			{
				revokeBadge(player, "count_275", true);
			}
			
			if (badges.length >= 250)
			{
				grantBadge(player, "count_250");
			}
			else
			{
				revokeBadge(player, "count_250", true);
			}
			
			if (badges.length >= 225)
			{
				grantBadge(player, "count_225");
			}
			else
			{
				revokeBadge(player, "count_225", true);
			}
			
			if (badges.length >= 200)
			{
				grantBadge(player, "count_200");
			}
			else
			{
				revokeBadge(player, "count_200", true);
			}
			
			if (badges.length >= 175)
			{
				grantBadge(player, "count_175");
			}
			else
			{
				revokeBadge(player, "count_175", true);
			}
			
			if (badges.length >= 150)
			{
				grantBadge(player, "count_150");
			}
			else
			{
				revokeBadge(player, "count_150", true);
			}
			
			if (badges.length >= 125)
			{
				grantBadge(player, "count_125");
			}
			else
			{
				revokeBadge(player, "count_125", true);
			}
			
			if (badges.length >= 100)
			{
				grantBadge(player, "count_100");
			}
			else
			{
				revokeBadge(player, "count_100", true);
			}
			
			if (badges.length >= 75)
			{
				grantBadge(player, "count_75");
			}
			else
			{
				revokeBadge(player, "count_75", true);
			}
			
			if (badges.length >= 50)
			{
				grantBadge(player, "count_50");
			}
			else
			{
				revokeBadge(player, "count_50", true);
			}
			
			if (badges.length >= 25)
			{
				grantBadge(player, "count_25");
			}
			else
			{
				revokeBadge(player, "count_25", true);
			}
			
			if (badges.length >= 10)
			{
				grantBadge(player, "count_10");
			}
			else
			{
				revokeBadge(player, "count_10", true);
			}
			
			if (badges.length >= 5)
			{
				grantBadge(player, "count_5");
			}
			else
			{
				revokeBadge(player, "count_5", true);
			}
		}
		
		return true;
	}
	
	
	public static void grantMasterSkillBadge(obj_id player, String skill) throws InterruptedException
	{
		if (!isIdValid( player ) || !isPlayer( player ))
		{
			return;
		}
		
		if (hasObjVar( player, respec.OBJVAR_RESPEC_ROOT))
		{
			return;
		}
		
		if (!skill.endsWith("_master"))
		{
			return;
		}
		
		String badgeId = "";
		if (skill.equals("class_forcesensitive_phase4_master"))
		{
			badgeId = "new_prof_jedi_master";
		}
		else if (skill.equals("class_smuggler_phase4_master"))
		{
			badgeId = "new_prof_smuggler_master";
		}
		else if (skill.equals("class_bountyhunter_phase4_master"))
		{
			badgeId = "new_prof_bountyhunter_master";
		}
		else if (skill.equals("class_commando_phase4_master"))
		{
			badgeId = "new_prof_commando_master";
		}
		else if (skill.equals("class_officer_phase4_master"))
		{
			badgeId = "new_prof_officer_master";
		}
		else if (skill.equals("class_medic_phase4_master"))
		{
			badgeId = "new_prof_medic_master";
		}
		else if (skill.equals("class_spy_phase4_master"))
		{
			badgeId = "new_prof_spy_master";
		}
		else if (skill.equals("class_entertainer_phase4_master"))
		{
			badgeId = "new_prof_entertainer_master";
		}
		else if (skill.equals("class_trader_phase4_master"))
		{
			badgeId = "new_prof_trader_master";
		}
		else if (skill.equals("combat_1hsword_master"))
		{
			badgeId = "combat_1hsword_master";
		}
		else if (skill.equals("combat_2hsword_master"))
		{
			badgeId = "combat_2hsword_master";
		}
		else if (skill.equals("combat_bountyhunter_master"))
		{
			badgeId = "combat_bountyhunter_master";
		}
		else if (skill.equals("combat_brawler_master"))
		{
			badgeId = "combat_brawler_master";
		}
		else if (skill.equals("combat_carbine_master"))
		{
			badgeId = "combat_carbine_master";
		}
		else if (skill.equals("combat_commando_master"))
		{
			badgeId = "combat_commando_master";
		}
		else if (skill.equals("combat_marksman_master"))
		{
			badgeId = "combat_marksman_master";
		}
		else if (skill.equals("combat_pistol_master"))
		{
			badgeId = "combat_pistol_master";
		}
		else if (skill.equals("combat_polearm_master"))
		{
			badgeId = "combat_polearm_master";
		}
		else if (skill.equals("combat_rifleman_master"))
		{
			badgeId = "combat_rifleman_master";
		}
		else if (skill.equals("combat_smuggler_master"))
		{
			badgeId = "combat_smuggler_master";
		}
		else if (skill.equals("combat_unarmed_master"))
		{
			badgeId = "combat_unarmed_master";
		}
		
		else if (skill.equals("class_structures_phase4_master"))
		{
			badgeId = "new_prof_crafting_architect_master";
			
			if (!hasBadge(player, badgeId))
			{
				grantBadge( player, badgeId );
			}
			
			badgeId = "new_prof_crafting_merchant_master";
			
			if (!hasBadge(player, badgeId))
			{
				grantBadge( player, badgeId );
			}
			
			badgeId = "new_prof_crafting_shipwright";
		}
		
		else if (skill.equals("class_munitions_phase4_master"))
		{
			badgeId = "new_prof_crafting_armorsmith_master";
			
			if (!hasBadge(player, badgeId))
			{
				grantBadge( player, badgeId );
			}
			
			badgeId = "new_prof_crafting_merchant_master";
			
			if (!hasBadge(player, badgeId))
			{
				grantBadge( player, badgeId );
			}
			
			badgeId = "new_prof_crafting_weaponsmith_master";
		}
		
		else if (skill.equals("class_domestics_phase4_master"))
		{
			badgeId = "new_prof_crafting_chef_master";
			
			if (!hasBadge(player, badgeId))
			{
				grantBadge( player, badgeId );
			}
			
			badgeId = "new_prof_crafting_merchant_master";
			
			if (!hasBadge(player, badgeId))
			{
				grantBadge( player, badgeId );
			}
			
			badgeId = "new_prof_crafting_tailor_master";
		}
		
		else if (skill.equals("class_engineering_phase4_master"))
		{
			badgeId = "new_prof_crafting_droidengineer_master";
			
			if (!hasBadge(player, badgeId))
			{
				grantBadge( player, badgeId );
			}
			
			badgeId = "new_prof_crafting_merchant_master";
		}
		
		else if (skill.equals("class_engineering_phase1_master"))
		{
			badgeId = "new_prof_crafting_artisan_master";
		}
		else if (skill.equals("outdoors_bio_engineer_master"))
		{
			badgeId = "new_prof_outdoors_bio_engineer_master";
		}
		else if (skill.equals("outdoors_creaturehandler_master"))
		{
			badgeId = "outdoors_creaturehandler_master";
		}
		else if (skill.equals("outdoors_ranger_master"))
		{
			badgeId = "outdoors_ranger_master";
		}
		else if (skill.equals("outdoors_scout_master"))
		{
			badgeId = "outdoors_scout_master";
		}
		else if (skill.equals("outdoors_squadleader_master"))
		{
			badgeId = "outdoors_squadleader_master";
		}
		else if (skill.equals("science_combatmedic_master"))
		{
			badgeId = "science_combatmedic_master";
		}
		else if (skill.equals("science_doctor_master"))
		{
			badgeId = "science_doctor_master";
		}
		else if (skill.equals("science_medic_master"))
		{
			badgeId = "science_medic_master";
		}
		else if (skill.equals("social_dancer_master"))
		{
			badgeId = "new_prof_social_dancer_master";
		}
		else if (skill.equals("social_entertainer_master"))
		{
			badgeId = "new_prof_social_entertainer_master";
		}
		else if (skill.equals("social_imagedesigner_master"))
		{
			badgeId = "new_prof_social_imagedesigner_master";
		}
		else if (skill.equals("social_musician_master"))
		{
			badgeId = "new_prof_social_musician_master";
		}
		
		else if (skill.equals("pilot_neutral_master"))
		{
			if (space_flags.isSpaceTrack(player, space_flags.PRIVATEER_TATOOINE))
			{
				badgeId = "pilot_neutral_tatooine";
			}
			else if (space_flags.isSpaceTrack(player, space_flags.PRIVATEER_NABOO))
			{
				badgeId = "pilot_neutral_naboo";
			}
			else if (space_flags.isSpaceTrack(player, space_flags.PRIVATEER_CORELLIA))
			{
				badgeId = "pilot_neutral_corellia";
			}
		}
		else if (skill.equals("pilot_rebel_navy_master"))
		{
			if (space_flags.isSpaceTrack(player, space_flags.REBEL_NABOO))
			{
				badgeId = "pilot_rebel_navy_naboo";
			}
			else if (space_flags.isSpaceTrack(player, space_flags.REBEL_TATOOINE))
			{
				badgeId = "pilot_rebel_navy_tatooine";
			}
			else if (space_flags.isSpaceTrack(player, space_flags.REBEL_CORELLIA))
			{
				badgeId = "pilot_rebel_navy_corellia";
			}
		}
		else if (skill.equals("pilot_imperial_navy_master"))
		{
			if (space_flags.isSpaceTrack(player, space_flags.IMPERIAL_NABOO))
			{
				badgeId = "pilot_imperial_navy_naboo";
			}
			else if (space_flags.isSpaceTrack(player, space_flags.IMPERIAL_TATOOINE))
			{
				badgeId = "pilot_imperial_navy_tatooine";
			}
			else if (space_flags.isSpaceTrack(player, space_flags.IMPERIAL_CORELLIA))
			{
				badgeId = "pilot_imperial_navy_corellia";
			}
		}
		else
		{
			return;
		}
		
		if (!hasBadge(player, badgeId))
		{
			grantBadge( player, badgeId );
		}
		
	}
	
	
	public static void grantThemeParkBadges(obj_id player) throws InterruptedException
	{
		
		if (getIntObjVar(player, "theme_park_imperial") >= 39)
		{
			
			if (!hasBadge(player, "bdg_thm_park_imperial_badge"))
			{
				
				grantBadge(player, "bdg_thm_park_imperial_badge");
			}
		}
		
		if (getIntObjVar(player, "theme_park_rebel") >= 49)
		{
			
			if (!hasBadge(player, "bdg_thm_park_rebel_badge"))
			{
				
				grantBadge(player, "bdg_thm_park_rebel_badge");
			}
		}
		
		if (getIntObjVar(player, "theme_park_jabba") >= 28)
		{
			
			if (!hasBadge(player, "bdg_thm_park_jabba_badge"))
			{
				
				grantBadge(player, "bdg_thm_park_jabba_badge");
			}
		}
		
		return;
	}
	
	
	public static void checkForCombatTargetBadge(obj_id player, String creatureType) throws InterruptedException
	{
		String datatable = "datatables/badge/combat_target_badges.iff";
		int row = dataTableSearchColumnForString(creatureType, "creatureName", datatable);
		
		if (row > -1)
		{
			int badgeNumber = dataTableGetInt(datatable, row, "badgeNumber");
			String badgeName = getCollectionSlotName(badgeNumber);
			
			if ((badgeName != null) && (badgeName.length() > 0) && !hasBadge(player, badgeName))
			{
				grantBadge(player, badgeName);
			}
		}
		
		return;
	}
}
