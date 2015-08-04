//======================================================================
//
// CuiMenuInfoTypes.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
 
#include "UICursor.h"
#include "UIManager.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientObject.h"
#include "clientGame/Game.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientUserInterface/CuiRadialMenuManager.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/RadialMenuManager.h"

//======================================================================

namespace
{
	using namespace Cui;

	//----------------------------------------------------------------------
	
	struct TypeInfo
	{
		StringId   stringId;
		UICursor * cursor;		
		
		TypeInfo ();
		TypeInfo (const StringId & _stringId, const char * const cursorPath);
		~TypeInfo ();
		TypeInfo & operator= (const TypeInfo & rhs);
		TypeInfo             (const TypeInfo & rhs);
	};

	//----------------------------------------------------------------------

	TypeInfo::TypeInfo () :
	stringId (),
		cursor   (0)
	{
	}

	//----------------------------------------------------------------------

	TypeInfo::TypeInfo (const StringId & _stringId, const char * const cursorPath) :
	stringId (_stringId),
		cursor   (0)
	{
		if (cursorPath)
		{
			cursor = safe_cast<UICursor *>(UIManager::gUIManager ().GetObjectFromPath (cursorPath, TUICursor));
			if (cursor)
				cursor->Attach (0);
			else
				WARNING (true, ("Invalid CuiMenuInfoTypes cursor: [%s]", cursorPath));
		}
	}

	//----------------------------------------------------------------------

	TypeInfo::~TypeInfo ()
	{
		if (cursor)
			cursor->Detach (0);
		cursor = 0;
	}

	//----------------------------------------------------------------------

	TypeInfo & TypeInfo::operator= (const TypeInfo & rhs)
	{
		if (this != &rhs)
		{
			stringId = rhs.stringId;
			if (rhs.cursor)
				rhs.cursor->Attach (0);
			if (cursor)
				cursor->Detach (0);
			cursor   = rhs.cursor;
		}

		return *this;
	}

	//----------------------------------------------------------------------

	TypeInfo::TypeInfo (const TypeInfo & rhs) :
	stringId (rhs.stringId),
		cursor   (rhs.cursor)
	{
		if (cursor)
			cursor->Attach (0);
	}

	//----------------------------------------------------------------------

	typedef std::pair<Cui::MenuInfoTypes::Type, SharedObjectTemplate::GameObjectType> TypePair;

	typedef stdmap<TypePair, TypeInfo>::fwd        TypeStringIdMap;

	TypeStringIdMap      typeStringIds;

	StringId s_menuLabels [static_cast<size_t>(Cui::MenuInfoTypes::CLIENT_MENU_LAST)];

	bool installed = false;

	namespace CursorPaths
	{
		const char * const activate         = "/styles.cursors.activate";
		const char * const attack           = "/styles.cursors.attack";
		const char * const deactivate       = "/styles.cursors.deactivate";
		const char * const drop             = "/styles.cursors.drop";
		const char * const equip            = "/styles.cursors.equip";
		const char * const info             = "/styles.cursors.info";
		const char * const intended_attack  = "/styles.cursors.intended_attack";
		const char * const intended_attack_inactive = "/styles.cursors.intended_inactive";
		const char * const loot             = "/styles.cursors.loot";
		const char * const mission_details  = "/styles.cursors.mission_details";
		const char * const open             = "/styles.cursors.open";
		const char * const pickup           = "/styles.cursors.pickup";
		const char * const trade_accept     = "/styles.cursors.trade_accept";
		const char * const trade_start      = "/styles.cursors.trade_start";
		const char * const unequip          = "/styles.cursors.unequip";
		const char * const use              = "/styles.cursors.use";
		const char * const use_eat          = "/styles.cursors.use_eat";
		const char * const use_throw        = "/styles.cursors.use_throw";
		const char * const sit              = "/styles.cursors.activate";
		const char * const craft            = "/styles.cursors.crafting";
		const char * const converse         = "/styles.cursors.converse";
		const char * const converse_stop    = "/styles.cursors.converse_stop";
	};

	UICursor *s_attackCursor;
	UICursor *s_intendedAttackCursor;
	UICursor *s_intendedAttackInactiveCursor;
}

//----------------------------------------------------------------------

namespace Cui
{
	namespace MenuInfoTypes
	{
		void                  install ()
		{
			DEBUG_FATAL (installed, ("already installed"));
			
			installed = true;
			
#define MAKE_ID(a,b,c,d) typeStringIds [TypePair (MenuInfoTypes::##a, SharedObjectTemplate::GOT_##b)] = TypeInfo (StringId ("ui_radial", #c), d);
			
			MAKE_ID(UNKNOWN,               none,                      unknown,                              0);
			MAKE_ID(COMBAT_TARGET,         none,                      combat_target,                        0);
			MAKE_ID(COMBAT_UNTARGET,       none,                      combat_untarget,                      0);
			MAKE_ID(COMBAT_ATTACK,         none,                      combat_attack,                        CursorPaths::attack);
			MAKE_ID(COMBAT_PEACE,          none,                      combat_peace,                         0);
			MAKE_ID(COMBAT_DUEL,           none,                      combat_duel,                          0);
			MAKE_ID(COMBAT_DEATH_BLOW,     none,                      combat_death_blow,                    CursorPaths::attack);

			MAKE_ID(EXAMINE,               none,                      examine,                              0);
			MAKE_ID(EXAMINE_CHARACTERSHEET,none,                      examine_charactersheet,               0);
			MAKE_ID(TRADE_START,           none,                      trade_start,                          CursorPaths::trade_start);
			MAKE_ID(TRADE_ACCEPT,          none,                      trade_accept,                         CursorPaths::trade_accept);
			MAKE_ID(ITEM_PICKUP,           none,                      item_pickup,                          CursorPaths::pickup);
			MAKE_ID(ITEM_EQUIP,            none,                      item_equip,                           CursorPaths::equip);
			MAKE_ID(ITEM_UNEQUIP,          none,                      item_unequip,                         CursorPaths::unequip);
			MAKE_ID(ITEM_DROP,             none,                      item_drop,                            CursorPaths::drop);
			MAKE_ID(ITEM_DESTROY,          none,                      item_destroy,                         0);
			MAKE_ID(ITEM_TOKEN,            none,                      item_token,                           0);
			MAKE_ID(ITEM_OPEN,             none,                      item_open,                            0);
			MAKE_ID(ITEM_OPEN_NEW_WINDOW,  none,                      item_open_new_window,                 0);
			MAKE_ID(ITEM_ACTIVATE,         none,                      item_activate,                        CursorPaths::use);
			MAKE_ID(ITEM_DEACTIVATE,       none,                      item_deactivate,                      CursorPaths::use);
			
			MAKE_ID(ITEM_USE,              none,                      item_use,                             CursorPaths::use);
			MAKE_ID(ITEM_USE_SELF,         none,                      item_use_self,                        CursorPaths::use);
			MAKE_ID(ITEM_USE_OTHER,        none,                      item_use_other,                       CursorPaths::use);
			MAKE_ID(ITEM_SIT,              none,                      item_sit,                             CursorPaths::sit);
			MAKE_ID(ITEM_MAIL,             none,                      item_mail,                            0);
			MAKE_ID(ITEM_USE,              deed,                      item_use_deed,                        CursorPaths::use);
			MAKE_ID(ITEM_USE,              deed_installation,         item_use_deed,                        CursorPaths::use);
			MAKE_ID(ITEM_USE,              deed_building,             item_use_deed,                        CursorPaths::use);
			MAKE_ID(ITEM_USE,              misc_food,                 item_use_food,                        CursorPaths::use_eat);
			MAKE_ID(ITEM_USE,              misc_food_pet,             item_feed_pet,                        CursorPaths::use);
			MAKE_ID(ITEM_USE,              misc_drink,                item_use_drink,                       CursorPaths::use_eat);
			MAKE_ID(ITEM_USE,              misc_pharmaceutical,       item_use_pharmaceutical,              CursorPaths::use);
			MAKE_ID(ITEM_USE_SELF,         misc_pharmaceutical,       item_use_pharmaceutical_self,         CursorPaths::use);
			MAKE_ID(ITEM_USE_OTHER,        misc_pharmaceutical,       item_use_pharmaceutical_other,        CursorPaths::use);
			MAKE_ID(ITEM_USE,              misc_sign,                 item_use_sign,                        CursorPaths::use);
			MAKE_ID(ITEM_USE,              misc_ticket_travel,        item_use_ticket_travel,               CursorPaths::use);
			MAKE_ID(ITEM_USE,              weapon_ranged_thrown,      item_use_weapon_thrown,               CursorPaths::use_throw);
			MAKE_ID(ITEM_USE,              misc_trap,                 item_use_weapon_thrown,               CursorPaths::use_throw);
			MAKE_ID(ITEM_USE,              weapon_heavy_misc,         item_use_weapon_heavy,                CursorPaths::use);
			MAKE_ID(ITEM_USE,              creature_droid_probe,      probe_droid_use,                      CursorPaths::use);
			MAKE_ID(ITEM_USE,              misc_firework,             firework_launch,                      CursorPaths::use);
			
			MAKE_ID(ITEM_USE,              terminal_bank,             item_use_terminal_bank,               CursorPaths::use);
			MAKE_ID(ITEM_USE,              terminal_cloning,          item_use_terminal_cloning,            CursorPaths::use);
			MAKE_ID(ITEM_USE,              terminal_insurance,        item_use_terminal_insurance,          CursorPaths::use);
			MAKE_ID(ITEM_USE,              terminal_mission,          item_use_terminal_mission,            CursorPaths::use);
			MAKE_ID(ITEM_USE,              terminal_player_structure, item_use_terminal_player_structure,   CursorPaths::use);
			MAKE_ID(ITEM_USE,              terminal_shipping,         item_use_terminal_shipping,           CursorPaths::use);
			MAKE_ID(ITEM_USE,              terminal_travel,           item_use_terminal_travel,             CursorPaths::use);
			MAKE_ID(ITEM_USE,              terminal_permissions,      item_use_terminal_permissions,        CursorPaths::use);
			MAKE_ID(ITEM_USE,              terminal_manage,           item_use_terminal_manage,             CursorPaths::use);
			MAKE_ID(ITEM_USE,              terminal_space_npe,        item_use_terminal_space_npe,          CursorPaths::use);
			MAKE_ID(ITEM_USE,              misc_force_crystal,        item_use_force_crystal,               CursorPaths::use);

			MAKE_ID(CONVERSE_START,        none,                      converse_start,                       CursorPaths::converse);
			MAKE_ID(CONVERSE_RESPOND,      none,                      converse_respond,                     0);
			MAKE_ID(CONVERSE_STOP,         none,                      converse_stop,                        CursorPaths::converse_stop);
			MAKE_ID(CRAFT_OPTIONS,         none,                      craft_options,                        CursorPaths::craft);
			MAKE_ID(CRAFT_START,           none,                      craft_start,                          CursorPaths::craft);
			MAKE_ID(CRAFT_HOPPER_INPUT,    none,                      craft_hopper_input,                   0);
			MAKE_ID(CRAFT_HOPPER_OUTPUT,   none,                      craft_hopper_output,                  0);

			MAKE_ID(TERMINAL_MISSION_LIST, none,                      terminal_mission_list,                CursorPaths::use);
			MAKE_ID(MISSION_DETAILS,       none,                      mission_details,                      CursorPaths::mission_details);
			
			MAKE_ID(LOOT,                  none,                      loot,                                 0);
			MAKE_ID(LOOT_ALL,              none,                      loot_all,                             0);
			MAKE_ID(GROUP_INVITE,          none,                      group_invite,                         0);
			MAKE_ID(GROUP_JOIN,            none,                      group_join,                           0);
			MAKE_ID(GROUP_LEAVE,           none,                      group_leave,                          0);
			MAKE_ID(GROUP_KICK,            none,                      group_kick,                           0);
			MAKE_ID(GROUP_DISBAND,         none,                      group_disband,                        0);
			MAKE_ID(GROUP_DECLINE,         none,                      group_decline,                        0);

			MAKE_ID(EXTRACT_OBJECT,        none,                      extract_object,                       0);
			MAKE_ID(TERMINAL_AUCTION_USE,  terminal_bazaar,           terminal_auction_use,                 CursorPaths::use);
			MAKE_ID(TERMINAL_AUCTION_USE,  vendor,                    vendor_use,                           CursorPaths::use);
			MAKE_ID(TERMINAL_AUCTION_USE,  creature,                  vendor_use,                           CursorPaths::use);

			MAKE_ID(CREATURE_FOLLOW,       none,                      creature_follow,                      0);
			MAKE_ID(CREATURE_STOP_FOLLOW,  none,                      creature_stop_follow,                 0);
			MAKE_ID(SPLIT,                 resource_container,        split,                                0);
			MAKE_ID(SPLIT,                 misc_factory_crate,        split,                                0);
			MAKE_ID(IMAGEDESIGN,           none,                      imagedesign,                          0);
			MAKE_ID(SET_NAME,              none,                      set_name,                             0);

			MAKE_ID(ITEM_ROTATE,           none,                      item_rotate,                          0);
			MAKE_ID(ITEM_ROTATE_RIGHT,     none,                      item_rotate_right,                    0);
			MAKE_ID(ITEM_ROTATE_LEFT,      none,                      item_rotate_left,                     0);
			MAKE_ID(ITEM_MOVE,             none,                      item_move,                            0);
			MAKE_ID(ITEM_MOVE_FORWARD,     none,                      item_move_forward,                    0);
			MAKE_ID(ITEM_MOVE_BACK,        none,                      item_move_back,                       0);
			MAKE_ID(ITEM_MOVE_UP,          none,                      item_move_up,                         0);
			MAKE_ID(ITEM_MOVE_DOWN,        none,                      item_move_down,                       0);

			MAKE_ID(VEHICLE_GENERATE,      none,                      control_call,                         CursorPaths::use);
			MAKE_ID(VEHICLE_STORE,         none,                      control_store,                        CursorPaths::use);
			MAKE_ID(PET_CALL,              none,                      pet_call,                             CursorPaths::use);
			MAKE_ID(PET_STORE,             none,                      control_store,                        CursorPaths::use);
			MAKE_ID(MISSION_ABORT,         none,                      mission_abort,                        CursorPaths::use);
			MAKE_ID(MISSION_END_DUTY,      none,                      mission_end_duty,                     CursorPaths::use);
				
			//space
			MAKE_ID(SHIP_MANAGE_COMPONENTS, none,                     ship_manage_components,               CursorPaths::use);
			MAKE_ID(WAYPOINT_AUTOPILOT,     none,                     waypoint_autopilot,                   CursorPaths::use);
			//endspace

			MAKE_ID(PROGRAM_DROID,          none,                     program_droid,                        CursorPaths::use);
			MAKE_ID(VEHICLE_OFFER_RIDE,     none,                     vehicle_offer_ride,                   0);
			MAKE_ID(ITEM_PUBLIC_CONTAINER_USE1, none,                 item_public_container_use1,           0);
			MAKE_ID(COLLECTIONS,            none,                     collections,                          0);
			MAKE_ID(GROUP_MASTER_LOOTER,    none,                     group_master_looter,                  0);
			MAKE_ID(GROUP_MAKE_LEADER,		none,					  group_make_leader,                    0);
			MAKE_ID(GROUP_LOOT,				none,					  group_loot,                           0);

			MAKE_ID(ITEM_ROTATE_FORWARD,   none,                      item_rotate_forward,                  0);
			MAKE_ID(ITEM_ROTATE_BACKWARD,  none,                      item_rotate_backward,                 0);
			MAKE_ID(ITEM_ROTATE_CLOCKWISE, none,                      item_rotate_clockwise,                0);
			MAKE_ID(ITEM_ROTATE_COUNTERCLOCKWISE, none,               item_rotate_counterclockwise,         0);
			MAKE_ID(ITEM_ROTATE_RANDOM,    none,                      item_rotate_random,                   0);
			MAKE_ID(ITEM_ROTATE_RANDOM_YAW, none,                     item_rotate_random_yaw,               0);
			MAKE_ID(ITEM_ROTATE_RANDOM_PITCH, none,                   item_rotate_random_pitch,             0);
			MAKE_ID(ITEM_ROTATE_RANDOM_ROLL, none,                    item_rotate_random_roll,              0);
			MAKE_ID(ITEM_ROTATE_RESET,     none,                      item_rotate_reset,                    0);
			MAKE_ID(ITEM_ROTATE_COPY,      none,                      item_rotate_copy,                     0);
			MAKE_ID(ITEM_MOVE_COPY_LOCATION, none,                    item_move_copy_location,              0);
			MAKE_ID(ITEM_MOVE_COPY_HEIGHT, none,                      item_move_copy_height,                0);
			MAKE_ID(GROUP_TELL,            none,                      group_tell,                           0);
			MAKE_ID(ITEM_WP_SETCOLOR,      none,                      item_wp_setcolor,                     0);
			MAKE_ID(ITEM_WP_SETCOLOR_BLUE, none,                      item_wp_setcolor_blue,                0);
			MAKE_ID(ITEM_WP_SETCOLOR_GREEN,none,                      item_wp_setcolor_green,               0);
			MAKE_ID(ITEM_WP_SETCOLOR_ORANGE, none,                    item_wp_setcolor_orange,              0);
			MAKE_ID(ITEM_WP_SETCOLOR_YELLOW, none,                    item_wp_setcolor_yellow,              0);
			MAKE_ID(ITEM_WP_SETCOLOR_PURPLE, none,                    item_wp_setcolor_purple,              0);
			MAKE_ID(ITEM_WP_SETCOLOR_WHITE, none,                     item_wp_setcolor_white,               0);
			MAKE_ID(ITEM_MOVE_LEFT,        none,                      item_move_left,                       0);
			MAKE_ID(ITEM_MOVE_RIGHT,       none,                      item_move_right,                      0);
			MAKE_ID(ROTATE_APPLY,          none,                      rotate_apply,                         0);
			MAKE_ID(ROTATE_RESET,          none,                      rotate_reset,                         0);
			MAKE_ID(WINDOW_LOCK,           none,                      window_lock,                          0);
			MAKE_ID(WINDOW_UNLOCK,         none,                      window_unlock,                        0);
			MAKE_ID(GROUP_CREATE_PICKUP_POINT, none,                  group_create_pickup_point,            0);
			MAKE_ID(GROUP_USE_PICKUP_POINT, none,                     group_use_pickup_point,               0);
			MAKE_ID(GROUP_USE_PICKUP_POINT_NOCAMP, none,              group_use_pickup_point_nocamp,        0);

			MAKE_ID(VOICE_SHORTLIST_REMOVE, none,                     voice_shortlist_remove,               0);
			MAKE_ID(VOICE_INVITE,           none,                     voice_invite,                         0);
			MAKE_ID(VOICE_KICK,             none,                     voice_kick,                           0);
			MAKE_ID(ITEM_EQUIP_APPEARANCE,  none,                     item_equip_appearance,                CursorPaths::equip);
			MAKE_ID(ITEM_UNEQUIP_APPEARANCE,none,                     item_unequip_appearance,              CursorPaths::unequip);
			MAKE_ID(OPEN_STORYTELLER_RECIPE,none,					  open_storyteller_recipe,              0);

#undef MAKE_ID
			
			s_attackCursor = safe_cast<UICursor *>(UIManager::gUIManager ().GetObjectFromPath (CursorPaths::attack, TUICursor));
			if(s_attackCursor)
				s_attackCursor->Attach (0);
			s_intendedAttackCursor = safe_cast<UICursor *>(UIManager::gUIManager ().GetObjectFromPath (CursorPaths::intended_attack, TUICursor));
			if (s_intendedAttackCursor)
				s_intendedAttackCursor->Attach (0); 
			s_intendedAttackInactiveCursor = safe_cast<UICursor *>(UIManager::gUIManager ().GetObjectFromPath (CursorPaths::intended_attack_inactive, TUICursor));
			if (s_intendedAttackInactiveCursor)
				s_intendedAttackInactiveCursor->Attach (0); 
		}
		
		void                  remove  ()
		{
			DEBUG_FATAL (!installed, ("not installed"));
			typeStringIds.clear ();
			if (s_attackCursor)
				s_attackCursor->Detach (0);
			if (s_intendedAttackCursor)
				s_intendedAttackCursor->Detach (0);
			s_intendedAttackCursor = 0;
			if (s_intendedAttackInactiveCursor)
				s_intendedAttackInactiveCursor->Detach (0);
			s_intendedAttackInactiveCursor = 0;
			installed = false;
		}

		//----------------------------------------------------------------------

		const TypeInfo * findTypeInfo (Type type, int gameObjectType)
		{
			const SharedObjectTemplate::GameObjectType got = static_cast<SharedObjectTemplate::GameObjectType>(gameObjectType);

			TypePair tp (type, got);

			TypeStringIdMap::iterator it = typeStringIds.find (tp);

			if (it != typeStringIds.end ())
			{
				const TypeInfo & info = (*it).second;
				return &info;
			}

			if (tp.second != 0)
			{
				//-- check for the generic base type
				const int maskedType = GameObjectTypes::getMaskedType (tp.second);
				
				if (maskedType)
				{
					tp.second = static_cast<SharedObjectTemplate::GameObjectType>(maskedType);
					it = typeStringIds.find (tp);
					
					if (it != typeStringIds.end ())
					{
						const TypeInfo & info = (*it).second;
						return &info;
					}
				}

				//-- check for the global 'none' type
				tp.second = SharedObjectTemplate::GOT_none;

				it = typeStringIds.find (tp);

				if (it != typeStringIds.end ())
				{
					const TypeInfo & info = (*it).second;
					return &info;
				}
			}

			return 0;
		}

		//----------------------------------------------------------------------

		const Unicode::String getLocalizedLabel (Type type, int gameObjectType)
		{
			const TypeInfo * const tinfo = findTypeInfo (type, gameObjectType);

			if (tinfo)
				return tinfo->stringId.localize ();

			static const Unicode::String err (Unicode::narrowToWide ("err"));
			return err;
		}


		//----------------------------------------------------------------------
		
		UICursor * const findDefaultCursor (ClientObject & object)
		{
			const int type = CuiRadialMenuManager::findDefaultAction (object);
			if (type)
			{
				const TypeInfo * const tinfo = findTypeInfo (static_cast<Type>(type), object.getGameObjectType ());

				if (tinfo)
					return tinfo->cursor;
			}
			return 0;
		}

		//----------------------------------------------------------------------

		bool isAttackCursor(UICursor * cursor)
		{
			return (cursor == s_attackCursor);
		}
		
		//----------------------------------------------------------------------

		UICursor * const getIntendedAttackCursor ()
		{
			return s_intendedAttackCursor;
		}

		//----------------------------------------------------------------------
		
		UICursor * const getIntendedAttackInactiveCursor ()
		{
			return s_intendedAttackInactiveCursor;
		}

		//----------------------------------------------------------------------

		bool executeCommandForMenu (Type type, const NetworkId & target, uint32 clientObjectUniqueId)
		{
			std::string commandName;
			bool useRadialTarget = false;
			if (RadialMenuManager::getCommandForMenuType (type, commandName, useRadialTarget) && !commandName.empty ())
			{
				if (!useRadialTarget)
					ClientCommandQueue::enqueueCommand (commandName, NetworkId::cms_invalid, Unicode::emptyString);
				else
				{
					if (!target.isValid () && clientObjectUniqueId)
					{
						char buf [64];
						const size_t buf_size = sizeof (buf);
						snprintf (buf, buf_size, "@%ld", clientObjectUniqueId);
						ClientCommandQueue::enqueueCommand (commandName, NetworkId::cms_invalid, Unicode::narrowToWide (buf));
					}
					else
						ClientCommandQueue::enqueueCommand (commandName, target, Unicode::emptyString);
				}
				return true;
			}
			return false;
		}

		//----------------------------------------------------------------------		
	}
}

//======================================================================

