//======================================================================
//
// CuiMenuInfoTypes.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiMenuInfoTypes_H
#define INCLUDED_CuiMenuInfoTypes_H

//======================================================================

class UICursor;
class ClientObject;
class NetworkId;

//----------------------------------------------------------------------

namespace Cui
{
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

	namespace MenuInfoTypes
	{
		enum Type
		{
			UNKNOWN, // 0
			COMBAT_TARGET,
			COMBAT_UNTARGET,
			COMBAT_ATTACK,
			COMBAT_PEACE,

			COMBAT_DUEL, // 5
			COMBAT_DEATH_BLOW,
			EXAMINE,
			EXAMINE_CHARACTERSHEET,
			TRADE_START,

			TRADE_ACCEPT, // 10
			ITEM_PICKUP, 
			ITEM_EQUIP,
			ITEM_UNEQUIP,
			ITEM_DROP,

			ITEM_DESTROY, // 15
			ITEM_TOKEN, 
			ITEM_OPEN,
			ITEM_OPEN_NEW_WINDOW,
			ITEM_ACTIVATE,

			ITEM_DEACTIVATE, // 20
			ITEM_USE, 
			ITEM_USE_SELF,
			ITEM_USE_OTHER,
			ITEM_SIT,

			ITEM_MAIL,  // 25
			CONVERSE_START, 
			CONVERSE_RESPOND,
			CONVERSE_RESPONSE,
			CONVERSE_STOP,

			CRAFT_OPTIONS, // 30
			CRAFT_START,  
			CRAFT_HOPPER_INPUT,
			CRAFT_HOPPER_OUTPUT,
			TERMINAL_MISSION_LIST,

			MISSION_DETAILS, // 35
			LOOT, 
			LOOT_ALL,
			GROUP_INVITE,
			GROUP_JOIN,

			GROUP_LEAVE, // 40
			GROUP_KICK, 
			GROUP_DISBAND,
			GROUP_DECLINE,
			EXTRACT_OBJECT,

			PET_CALL, // 45
			TERMINAL_AUCTION_USE, 
			CREATURE_FOLLOW,
			CREATURE_STOP_FOLLOW,
			SPLIT,

			IMAGEDESIGN, // 50
			SET_NAME, 
			ITEM_ROTATE,
			ITEM_ROTATE_RIGHT,
			ITEM_ROTATE_LEFT,

			ITEM_MOVE, // 55
			ITEM_MOVE_FORWARD, 
			ITEM_MOVE_BACK,
			ITEM_MOVE_UP,
			ITEM_MOVE_DOWN,

			PET_STORE, // 60
			VEHICLE_GENERATE, 
			VEHICLE_STORE,
			MISSION_ABORT,
			MISSION_END_DUTY,

			//space
			SHIP_MANAGE_COMPONENTS, // 65
			WAYPOINT_AUTOPILOT,
			//endspace

			PROGRAM_DROID, // 67
			VEHICLE_OFFER_RIDE,
			ITEM_PUBLIC_CONTAINER_USE1,

			COLLECTIONS, // 70
			GROUP_MASTER_LOOTER,
			GROUP_MAKE_LEADER,
			GROUP_LOOT,
			ITEM_ROTATE_FORWARD,

			ITEM_ROTATE_BACKWARD, // 75
			ITEM_ROTATE_CLOCKWISE,
			ITEM_ROTATE_COUNTERCLOCKWISE,
			ITEM_ROTATE_RANDOM,
			ITEM_ROTATE_RANDOM_YAW,

			ITEM_ROTATE_RANDOM_PITCH, // 80
			ITEM_ROTATE_RANDOM_ROLL,
			ITEM_ROTATE_RESET,
			ITEM_ROTATE_COPY,
			ITEM_MOVE_COPY_LOCATION,

			ITEM_MOVE_COPY_HEIGHT, // 85
			GROUP_TELL,
			ITEM_WP_SETCOLOR,
			ITEM_WP_SETCOLOR_BLUE,
			ITEM_WP_SETCOLOR_GREEN,

			ITEM_WP_SETCOLOR_ORANGE, // 90
			ITEM_WP_SETCOLOR_YELLOW,
			ITEM_WP_SETCOLOR_PURPLE,
			ITEM_WP_SETCOLOR_WHITE,
			ITEM_MOVE_LEFT,

			ITEM_MOVE_RIGHT, // 95
			ROTATE_APPLY,
			ROTATE_RESET,
			WINDOW_LOCK,
			WINDOW_UNLOCK,

			GROUP_CREATE_PICKUP_POINT, // 100
			GROUP_USE_PICKUP_POINT,
			GROUP_USE_PICKUP_POINT_NOCAMP,
			VOICE_SHORTLIST_REMOVE,
			VOICE_INVITE,

			VOICE_KICK, // 105
			ITEM_EQUIP_APPEARANCE,
			ITEM_UNEQUIP_APPEARANCE,
			OPEN_STORYTELLER_RECIPE,
			GOD_TELEPORT,

			CLIENT_MENU_LAST,

			// some server side menu items that we need to catch on the client side
			// *****NOTE***** these are referenced as *****OFFSETS***** from 
			// CLIENT_MENU_LAST, meaning that if you add a new value *****BEFORE*****
			// CLIENT_MENU_LAST, you *****DO NOT***** need to do anything to these values
			SERVER_MENU1 = CLIENT_MENU_LAST + 1,
			SERVER_MENU2 = CLIENT_MENU_LAST + 2,
			SERVER_PET_MOUNT = CLIENT_MENU_LAST + 178,
			SERVER_VEHICLE_ENTER_EXIT = CLIENT_MENU_LAST + 183,
		};

		void                  install ();
		void                  remove  ();

		const Unicode::String getLocalizedLabel (Type type, int gameObjectType);
		UICursor * const      findDefaultCursor (ClientObject & object);
		bool                  isAttackCursor(UICursor * cursor);
		UICursor * const      getIntendedAttackCursor ();
		UICursor * const      getIntendedAttackInactiveCursor ();

		bool                  executeCommandForMenu (Type, const NetworkId & target, uint32 clientObjectUniqueId);
	};
}

//======================================================================

#endif
