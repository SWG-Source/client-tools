package script;

public class menu_info_types
{
	private static int index = 0;

	//----------------------------------------------------------------------
	//--
	//-- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//--
	//-- these MUST BE KEPT IN SYNC in the following files
	//--
	//-- menu_info_types.java
	//-- CuiMenuInfoTypes.h
	//-- game/datatables/player/radial_menu.tab
	//--
	//-- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	///--

	/// shared client-server menu items
	/// this must be kept in sync with the client source code
	/// do not put server-added items in this part of the list.

	public static final int UNKNOWN                    = index++;
	public static final int COMBAT_TARGET              = index++;
	public static final int COMBAT_UNTARGET            = index++;
	public static final int COMBAT_ATTACK              = index++;
	public static final int COMBAT_PEACE               = index++;
	public static final int COMBAT_DUEL                = index++;
	public static final int COMBAT_DEATH_BLOW          = index++;
	public static final int EXAMINE                    = index++;
	public static final int EXAMINE_CHARACTERSHEET     = index++;
	public static final int TRADE_START                = index++;
	public static final int TRADE_ACCEPT               = index++;
	public static final int ITEM_PICKUP                = index++;
	public static final int ITEM_EQUIP                 = index++;
	public static final int ITEM_UNEQUIP               = index++;
	public static final int ITEM_DROP                  = index++;
	public static final int ITEM_DESTROY               = index++;
	public static final int ITEM_TOKEN                 = index++;
	public static final int ITEM_OPEN                  = index++;
	public static final int ITEM_OPEN_NEW_WINDOW       = index++;
	public static final int ITEM_ACTIVATE              = index++;
	public static final int ITEM_DEACTIVATE            = index++;
	public static final int ITEM_USE                   = index++;
	public static final int ITEM_USE_SELF              = index++;
	public static final int ITEM_USE_OTHER             = index++;
	public static final int ITEM_SIT                   = index++;
	public static final int ITEM_MAIL                  = index++;
	public static final int CONVERSE_START             = index++;
	public static final int CONVERSE_RESPOND           = index++;
	public static final int CONVERSE_RESPONSE          = index++;
	public static final int CONVERSE_STOP              = index++;
	public static final int CRAFT_OPTIONS              = index++;
	public static final int CRAFT_START                = index++;
	public static final int CRAFT_HOPPER_INPUT         = index++;
	public static final int CRAFT_HOPPER_OUTPUT        = index++;
	public static final int MISSION_TERMINAL_LIST      = index++;
	public static final int MISSION_DETAILS            = index++;
	public static final int LOOT                       = index++;
	public static final int LOOT_ALL                   = index++;
	public static final int GROUP_INVITE               = index++;
	public static final int GROUP_JOIN                 = index++;
	public static final int GROUP_LEAVE                = index++;
	public static final int GROUP_KICK                 = index++;
	public static final int GROUP_DISBAND              = index++;
	public static final int GROUP_DECLINE              = index++;
	public static final int EXTRACT_OBJECT             = index++;
	public static final int PET_CALL                   = index++;
	public static final int TERMINAL_AUCTION_USE       = index++;
	public static final int CREATURE_FOLLOW            = index++;
	public static final int CREATURE_STOP_FOLLOW       = index++;
	public static final int SPLIT                      = index++;
	public static final int IMAGEDESIGN                = index++;
	public static final int SET_NAME                   = index++;
	public static final int ITEM_ROTATE                = index++;
	public static final int ITEM_ROTATE_RIGHT          = index++; // yaw
	public static final int ITEM_ROTATE_LEFT           = index++;
	public static final int ITEM_MOVE                  = index++;
	public static final int ITEM_MOVE_FORWARD          = index++;
	public static final int ITEM_MOVE_BACK             = index++;
	public static final int ITEM_MOVE_UP               = index++;
	public static final int ITEM_MOVE_DOWN             = index++;
	public static final int PET_STORE                  = index++;
	public static final int VEHICLE_GENERATE           = index++;
	public static final int VEHICLE_STORE              = index++;
	public static final int MISSION_ABORT              = index++;
	public static final int MISSION_END_DUTY           = index++;

	//space
	public static final int SHIP_MANAGE_COMPONENTS     = index++;
	public static final int WAYPOINT_AUTOPILOT         = index++;
	//endspace

	public static final int PROGRAM_DROID              = index++;
	public static final int VEHICLE_OFFER_RIDE         = index++;
	public static final int ITEM_PUBLIC_CONTAINER_USE1 = index++;
	public static final int COLLECTIONS                = index++;
	public static final int GROUP_MASTER_LOOTER        = index++;
	public static final int GROUP_MAKE_LEADER          = index++;
	public static final int GROUP_LOOT                 = index++;
	public static final int ITEM_ROTATE_FORWARD        = index++; // pitch
	public static final int ITEM_ROTATE_BACKWARD       = index++;
	public static final int ITEM_ROTATE_CLOCKWISE      = index++; // roll
	public static final int ITEM_ROTATE_COUNTERCLOCKWISE = index++;
	public static final int ITEM_ROTATE_RANDOM         = index++;
	public static final int ITEM_ROTATE_RANDOM_YAW     = index++;
	public static final int ITEM_ROTATE_RANDOM_PITCH   = index++;
	public static final int ITEM_ROTATE_RANDOM_ROLL    = index++;
	public static final int ITEM_ROTATE_RESET          = index++;
	public static final int ITEM_ROTATE_COPY           = index++;
	public static final int ITEM_MOVE_COPY_LOCATION    = index++;
	public static final int ITEM_MOVE_COPY_HEIGHT      = index++;
	public static final int GROUP_TELL                 = index++;
	public static final int ITEM_WP_SETCOLOR           = index++;
	public static final int ITEM_WP_SETCOLOR_BLUE      = index++;
	public static final int ITEM_WP_SETCOLOR_GREEN     = index++;
	public static final int ITEM_WP_SETCOLOR_ORANGE    = index++;
	public static final int ITEM_WP_SETCOLOR_YELLOW    = index++;
	public static final int ITEM_WP_SETCOLOR_PURPLE    = index++;
	public static final int ITEM_WP_SETCOLOR_WHITE     = index++;
	public static final int ITEM_MOVE_LEFT             = index++;
	public static final int ITEM_MOVE_RIGHT            = index++;
	public static final int ROTATE_APPLY               = index++;
	public static final int ROTATE_RESET               = index++;
	public static final int WINDOW_LOCK                = index++;
	public static final int WINDOW_UNLOCK              = index++;
	public static final int GROUP_CREATE_PICKUP_POINT  = index++;
	public static final int GROUP_USE_PICKUP_POINT     = index++;
	public static final int GROUP_USE_PICKUP_POINT_NOCAMP = index++;

	public static final int VOICE_SHORTLIST_REMOVE     = index++;
	public static final int VOICE_INVITE               = index++;
	public static final int VOICE_KICK                 = index++;
	public static final int ITEM_EQUIP_APPEARANCE      = index++;
	public static final int ITEM_UNEQUIP_APPEARANCE    = index++;
	public static final int OPEN_STORYTELLER_RECIPE    = index++;


	/// do not add any constants between here and SERVER_DIVIDER
	/// unless they also exist on the client

	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------

	/**
	* this is a dummy data element to delineate the client-server shared and server-only menu items
	*/
	public static final int SERVER_DIVIDER          = index++;

	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------

	//----------------------------------------------------------------------
	//--
	//-- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//--
	//-- these MUST BE KEPT IN SYNC in the following files
	//--
	//-- menu_info_types.java
	//-- CuiMenuInfoTypes.h
	//-- game/datatables/player/radial_menu.tab
	//--
	//-- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	///--

	public static final int SERVER_MENU1		  	                = index++;
	public static final int SERVER_MENU2		  	                = index++;
	public static final int SERVER_MENU3		  	                = index++;
	public static final int SERVER_MENU4		  	                = index++;
	public static final int SERVER_MENU5		  	                = index++;
	public static final int SERVER_MENU6		  	                = index++;
	public static final int SERVER_MENU7		  	                = index++;
	public static final int SERVER_MENU8		  	                = index++;
	public static final int SERVER_MENU9		  	                = index++;
	public static final int SERVER_MENU10		  	                = index++;

	public static final int SERVER_MENU11		  	                = index++;
	public static final int SERVER_MENU12		  	                = index++;
	public static final int SERVER_MENU13		  	                = index++;
	public static final int SERVER_MENU14		  	                = index++;
	public static final int SERVER_MENU15		  	                = index++;
	public static final int SERVER_MENU16		  	                = index++;
	public static final int SERVER_MENU17		  	                = index++;
	public static final int SERVER_MENU18		  	                = index++;
	public static final int SERVER_MENU19		  	                = index++;
	public static final int SERVER_MENU20		  	                = index++;

	public static final int SERVER_MENU21		  	                = index++;
	public static final int SERVER_MENU22		  	                = index++;
	public static final int SERVER_MENU23		  	                = index++;
	public static final int SERVER_MENU24		  	                = index++;
	public static final int SERVER_MENU25		  	                = index++;
	public static final int SERVER_MENU26		  	                = index++;
	public static final int SERVER_MENU27		  	                = index++;
	public static final int SERVER_MENU28		  	                = index++;
	public static final int SERVER_MENU29		  	                = index++;
	public static final int SERVER_MENU30		  	                = index++;

	public static final int SERVER_MENU31		  	                = index++;
	public static final int SERVER_MENU32		  	                = index++;
	public static final int SERVER_MENU33		  	                = index++;
	public static final int SERVER_MENU34		  	                = index++;
	public static final int SERVER_MENU35		  	                = index++;
	public static final int SERVER_MENU36		  	                = index++;
	public static final int SERVER_MENU37		  	                = index++;
	public static final int SERVER_MENU38		  	                = index++;
	public static final int SERVER_MENU39		  	                = index++;
	public static final int SERVER_MENU40		  	                = index++;

	public static final int SERVER_MENU41		  	                = index++;
	public static final int SERVER_MENU42		  	                = index++;
	public static final int SERVER_MENU43		  	                = index++;
	public static final int SERVER_MENU44		  	                = index++;
	public static final int SERVER_MENU45		  	                = index++;
	public static final int SERVER_MENU46		  	                = index++;
	public static final int SERVER_MENU47		  	                = index++;
	public static final int SERVER_MENU48		  	                = index++;
	public static final int SERVER_MENU49		  	                = index++;
	public static final int SERVER_MENU50		  	                = index++;

	public static final int SERVER_HARVESTER_MANAGE                 = index++;
	public static final int SERVER_HOUSE_MANAGE                     = index++;
	public static final int SERVER_FACTION_HALL_MANAGE              = index++;

	public static final int SERVER_HUE                              = index++;

	public static final int SERVER_OBSERVE                          = index++;
	public static final int SERVER_STOP_OBSERVING                   = index++;

	//TERMINAL MENUS
	public static final int SERVER_TRAVEL_OPTIONS                   = index++;
	public static final int SERVER_BAZAAR_OPTIONS                   = index++;
	public static final int SERVER_SHIPPING_OPTIONS                 = index++;

	// Healing Menus
	public static final int SERVER_HEAL_WOUND                       = index++;
	public static final int SERVER_HEAL_WOUND_HEALTH                = index++;
	public static final int SERVER_HEAL_WOUND_ACTION                = index++;
	public static final int SERVER_HEAL_WOUND_STRENGTH              = index++;
	public static final int SERVER_HEAL_WOUND_CONSTITUTION          = index++;
	public static final int SERVER_HEAL_WOUND_QUICKNESS             = index++;
	public static final int SERVER_HEAL_WOUND_STAMINA               = index++;
	public static final int SERVER_HEAL_DAMAGE                      = index++;
	public static final int SERVER_HEAL_STATE                       = index++;
	public static final int SERVER_HEAL_STATE_STUNNED               = index++;
	public static final int SERVER_HEAL_STATE_BLINDED               = index++;
	public static final int SERVER_HEAL_STATE_DIZZY                 = index++;
	public static final int SERVER_HEAL_STATE_INTIMIDATED           = index++;
	public static final int SERVER_HEAL_ENHANCE                     = index++;
	public static final int SERVER_HEAL_ENHANCE_HEALTH              = index++;
	public static final int SERVER_HEAL_ENHANCE_ACTION              = index++;
	public static final int SERVER_HEAL_ENHANCE_STRENGTH            = index++;
	public static final int SERVER_HEAL_ENHANCE_CONSTITUTION        = index++;
	public static final int SERVER_HEAL_ENHANCE_QUICKNESS           = index++;
	public static final int SERVER_HEAL_ENHANCE_STAMINA             = index++;
	public static final int SERVER_HEAL_FIRSTAID			= index++;
	public static final int SERVER_HEAL_CURE_POISON     		= index++;
	public static final int SERVER_HEAL_CURE_DISEASE   	        = index++;
	public static final int SERVER_HEAL_APPLY_POISON                = index++;
	public static final int SERVER_HEAL_APPLY_DISEASE               = index++;


	public static final int SERVER_HARVEST_CORPSE                   = index++;

	public static final int SERVER_PERFORMANCE_LISTEN               = index++;
	public static final int SERVER_PERFORMANCE_WATCH                = index++;
	public static final int SERVER_PERFORMANCE_LISTEN_STOP          = index++;
	public static final int SERVER_PERFORMANCE_WATCH_STOP           = index++;

	public static final int SERVER_TERMINAL_PERMISSIONS             = index++;
	public static final int SERVER_TERMINAL_MANAGEMENT              = index++;
	public static final int SERVER_TERMINAL_PERMISSIONS_ENTER       = index++;
	public static final int SERVER_TERMINAL_PERMISSIONS_BANNED      = index++;
	public static final int SERVER_TERMINAL_PERMISSIONS_ADMIN       = index++;
	public static final int SERVER_TERMINAL_PERMISSIONS_VENDOR      = index++;
	public static final int SERVER_TERMINAL_PERMISSIONS_HOPPER      = index++;
	public static final int SERVER_TERMINAL_MANAGEMENT_STATUS       = index++;
	public static final int SERVER_TERMINAL_MANAGEMENT_PRIVACY      = index++;
	public static final int SERVER_TERMINAL_MANAGEMENT_TRANSFER     = index++;
	public static final int SERVER_TERMINAL_MANAGEMENT_RESIDENCE    = index++;
	public static final int SERVER_TERMINAL_MANAGEMENT_DESTROY      = index++;
	public static final int SERVER_TERMINAL_MANAGEMENT_PAY          = index++;
	public static final int SERVER_TERMINAL_CREATE_VENDOR           = index++;
	public static final int SERVER_GIVE_VENDOR_MAINTENANCE          = index++;

	public static final int SERVER_ITEM_OPTIONS                     = index++;

	public static final int SERVER_SURVEY_TOOL_RANGE                = index++;
	public static final int SERVER_SURVEY_TOOL_RESOLUTION           = index++;
	public static final int SERVER_SURVEY_TOOL_CLASS                = index++;

	// probe droids
	public static final int SERVER_PROBE_DROID_TRACK_TARGET         = index++;
	public static final int SERVER_PROBE_DROID_FIND_TARGET          = index++;
	public static final int SERVER_PROBE_DROID_ACTIVATE             = index++;
	public static final int SERVER_PROBE_DROID_BUY                  = index++;

	public static final int SERVER_TEACH                            = index++;

	//pets
	public static final int PET_COMMAND                             = index++;
	public static final int PET_FOLLOW                              = index++;
	public static final int PET_STAY                                = index++;
	public static final int PET_GUARD                               = index++;
	public static final int PET_FRIEND                              = index++;
	public static final int PET_ATTACK                              = index++;
	public static final int PET_PATROL                              = index++;
	public static final int PET_GET_PATROL_POINT                    = index++;
	public static final int PET_CLEAR_PATROL_POINTS                 = index++;
	public static final int PET_ASSUME_FORMATION_1                  = index++;
	public static final int PET_ASSUME_FORMATION_2                  = index++;
	public static final int PET_TRANSFER                            = index++;
	public static final int PET_RELEASE                             = index++;
	public static final int PET_TRICK_1                             = index++;
	public static final int PET_TRICK_2                             = index++;
	public static final int PET_TRICK_3                             = index++;
	public static final int PET_TRICK_4                             = index++;
	public static final int PET_GROUP                               = index++;
	public static final int PET_TAME                                = index++;
	public static final int PET_FEED                                = index++;
	public static final int PET_SPECIAL_ATTACK_ONE					= index++;
	public static final int PET_SPECIAL_ATTACK_TWO					= index++;
	public static final int PET_RANGED_ATTACK						= index++;

	//dice
	public static final int DICE_ROLL                               = index++;
	public static final int DICE_TWO_FACE                           = index++;
	public static final int DICE_THREE_FACE                         = index++;
	public static final int DICE_FOUR_FACE                          = index++;
	public static final int DICE_FIVE_FACE                          = index++;
	public static final int DICE_SIX_FACE                           = index++;
	public static final int DICE_SEVEN_FACE                         = index++;
	public static final int DICE_EIGHT_FACE                         = index++;
	public static final int DICE_COUNT_ONE                          = index++;
	public static final int DICE_COUNT_TWO                          = index++;
	public static final int DICE_COUNT_THREE                        = index++;
	public static final int DICE_COUNT_FOUR                         = index++;

	//ballot box terminal
	public static final int CREATE_BALLOT                           = index++;
	public static final int VOTE                                    = index++;

	// Bombing Runs
	public static final int BOMBING_RUN                             = index++;
	public static final int SELF_DESTRUCT                           = index++;
	public static final int THIRTY_SEC                              = index++;
	public static final int FIFTEEN_SEC                             = index++;

	//camping
	public static final int SERVER_CAMP_DISBAND                     = index++;
	public static final int SERVER_CAMP_ASSUME_OWNERSHIP            = index++;
	public static final int SERVER_PROBE_DROID_PROGRAM              = index++;

	// guilds
	public static final int SERVER_GUILD_CREATE                     = index++;
	public static final int SERVER_GUILD_INFO                       = index++;
	public static final int SERVER_GUILD_MEMBERS                    = index++;
	public static final int SERVER_GUILD_SPONSORED                  = index++;
	public static final int SERVER_GUILD_ENEMIES                    = index++;
	public static final int SERVER_GUILD_SPONSOR                    = index++;
	public static final int SERVER_GUILD_DISBAND                    = index++;
	public static final int SERVER_GUILD_NAMECHANGE                 = index++;
	public static final int SERVER_GUILD_GUILD_MANAGEMENT           = index++;
	public static final int SERVER_GUILD_MEMBER_MANAGEMENT          = index++;

	// manufacturing
	public static final int SERVER_MANF_HOPPER_INPUT                = index++;
	public static final int SERVER_MANF_HOPPER_OUTPUT               = index++;
	public static final int SERVER_MANF_STATION_SCHEMATIC           = index++;

	// elevators

	public static final int ELEVATOR_UP                             = index++;
	public static final int ELEVATOR_DOWN                           = index++;


	public static final int SERVER_PET_OPEN                         = index++;
	public static final int SERVER_PET_DPAD                         = index++;

	public static final int SERVER_MED_TOOL_DIAGNOSE                = index++;
	public static final int SERVER_MED_TOOL_TENDWOUND               = index++;
	public static final int SERVER_MED_TOOL_TENDDAMAGE              = index++;

	public static final int SERVER_PET_MOUNT              			= index++; // *DO NOT* use for anything else other than for a pet mount pet radial menu action
	public static final int SERVER_PET_DISMOUNT              		= index++;
	public static final int SERVER_PET_TRAIN_MOUNT         			= index++;

	public static final int SERVER_VEHICLE_ENTER           			= index++;
	public static final int SERVER_VEHICLE_EXIT              		= index++;
	public static final int SERVER_VEHICLE_ENTER_EXIT       		= index++; // *DO NOT* use for anything else other than for a vehicle enter/exit radial menu action

	public static final int OPEN_NAVICOMP_DPAD	            		= index++;
	public static final int INIT_NAVICOMP_DPAD	            		= index++;




	// player cities

	public static final int CITY_STATUS								= index++;
	public static final int CITY_CITIZENS							= index++;
	public static final int CITY_STRUCTURES							= index++;
	public static final int CITY_TREASURY							= index++;
	public static final int CITY_MANAGEMENT							= index++;
	public static final int CITY_NAME								= index++;
	public static final int CITY_MILITIA							= index++;
	public static final int CITY_TAXES								= index++;
	public static final int CITY_TREASURY_DEPOSIT					= index++;
	public static final int CITY_TREASURY_WITHDRAW					= index++;
	public static final int CITY_REGISTER							= index++;
	public static final int CITY_RANK								= index++;
	public static final int CITY_ADMIN_1							= index++;
	public static final int CITY_ADMIN_2							= index++;
	public static final int CITY_ADMIN_3							= index++;
	public static final int CITY_ADMIN_4							= index++;
	public static final int CITY_ADMIN_5							= index++;
	public static final int CITY_ADMIN_6							= index++;
	public static final int MEMORY_CHIP_PROGRAM	       = index++;
	public static final int MEMORY_CHIP_TRANSFER	   = index++;
	public static final int MEMORY_CHIP_ANALYZE        = index++;
	public static final int EQUIP_DROID_ON_SHIP 	   = index++;

	public static final int BIO_LINK                   = index++;

	public static final int LANDMINE_DISARM						= index++;
	public static final int LANDMINE_REVERSE_TRIGGER				= index++;
	public static final int REWARD_TRADE_IN							= index++;

// VERY END
	//----------------------------------------------------------------------
	//--
	//-- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//--
	//-- these MUST BE KEPT IN SYNC in the following files
	//--
	//-- menu_info_types.java
	//-- CuiMenuInfoTypes.h
	//-- game/datatables/player/radial_menu.tab
	//--
	//-- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	///--

	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------

	public static final int MENU_LAST                  = index++;

	//-----------------------------------------------------------------------------

	static
	{
		if (MENU_LAST >= 65535)
		{
			//-- the network packets bearing the menu information stores
			//-- the menu type as unsigned 16 bits.
			//-- see sharedNetworkMessages/ObjectMenuInfoRequestData.h for details

			System.out.println ("ERROR!!! number of menu types, " + MENU_LAST + " has reached or exceeded 65535!\nSee menu_info_types.java for details.");
			System.exit (1);
		}
	}
}
