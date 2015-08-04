package script.theme_park.nym;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.groundquests;
import script.library.locations;
import script.library.utils;


public class stronghold_ladder extends script.base_script
{
	public stronghold_ladder()
	{
	}
	public static final String STF_FILE = "theme_park/outbreak/outbreak";
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if ((!groundquests.isTaskActive(player, "outbreak_quest_facility_05_imperial", "leaveFacility") && !groundquests.isTaskActive(player, "outbreak_quest_facility_05_rebel", "leaveFacility") && !groundquests.isTaskActive(player, "outbreak_quest_facility_05_neutral", "leaveFacility") && !groundquests.isTaskActive(player, "outbreak_quest_facility_05_imperial", "coverSurvivors") && !groundquests.isTaskActive(player, "outbreak_quest_facility_05_rebel", "coverSurvivors") && !groundquests.isTaskActive(player, "outbreak_quest_facility_05_neutral", "coverSurvivors") && !groundquests.isTaskActive(player, "outbreak_quest_facility_05_imperial", "seeHX138") && !groundquests.isTaskActive(player, "outbreak_quest_facility_05_rebel", "seeHX138") && !groundquests.isTaskActive(player, "outbreak_quest_facility_05_neutral", "seeHX138") && !groundquests.hasCompletedQuest(player, "outbreak_quest_final_imperial") && !groundquests.hasCompletedQuest(player, "outbreak_quest_final_rebel") && !groundquests.hasCompletedQuest(player, "outbreak_quest_final_neutral")))
		{
			CustomerServiceLog("outbreak_themepark", "exit_ladder.OnObjectMenuRequest: Player did not have the appropriate quest task and has not finished the quest. player: "+player);
			return SCRIPT_CONTINUE;
		}
		int mnu = mi.addRootMenu (menu_info_types.ITEM_USE, new string_id(STF_FILE,"open_hatch"));
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if ((!groundquests.isTaskActive(player, "outbreak_quest_facility_05_imperial", "leaveFacility") && !groundquests.isTaskActive(player, "outbreak_quest_facility_05_rebel", "leaveFacility") && !groundquests.isTaskActive(player, "outbreak_quest_facility_05_neutral", "leaveFacility") && !groundquests.isTaskActive(player, "outbreak_quest_facility_05_imperial", "coverSurvivors") && !groundquests.isTaskActive(player, "outbreak_quest_facility_05_rebel", "coverSurvivors") && !groundquests.isTaskActive(player, "outbreak_quest_facility_05_neutral", "coverSurvivors") && !groundquests.isTaskActive(player, "outbreak_quest_facility_05_imperial", "seeHX138") && !groundquests.isTaskActive(player, "outbreak_quest_facility_05_rebel", "seeHX138") && !groundquests.isTaskActive(player, "outbreak_quest_facility_05_neutral", "seeHX138") && !groundquests.hasCompletedQuest(player, "outbreak_quest_final_imperial") && !groundquests.hasCompletedQuest(player, "outbreak_quest_final_rebel") && !groundquests.hasCompletedQuest(player, "outbreak_quest_final_neutral")))
		{
			CustomerServiceLog("outbreak_themepark", "exit_ladder.OnObjectMenuSelect: Player did not have the appropriate quest task and has not finished the quest. player: "+player);
			return SCRIPT_CONTINUE;
		}
		
		if ((groundquests.isTaskActive(player, "outbreak_quest_facility_05_imperial", "leaveFacility") || groundquests.isTaskActive(player, "outbreak_quest_facility_05_rebel", "leaveFacility") || groundquests.isTaskActive(player, "outbreak_quest_facility_05_neutral", "leaveFacility")))
		{
			groundquests.sendSignal(player, "hasLeftFacility");
		}
		
		warpPlayer(player, "dathomir", -7390, 572, -7384, null, -7390, 572, -7384, null, true);
		return SCRIPT_CONTINUE;
	}
}
