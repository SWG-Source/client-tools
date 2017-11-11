//======================================================================
//
// CuiStringIdsCombatSpam.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsCombatSpam_H
#define INCLUDED_CuiStringIdsCombatSpam_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsCombatSpam
{
	MAKE_STRING_ID(cbt_spam, attack_result_template);
	MAKE_STRING_ID(cbt_spam, attack_result_attacker);
	MAKE_STRING_ID(cbt_spam, attack_result_attacker_brief);
	MAKE_STRING_ID(cbt_spam, attack_result_attacker_fullname);
	MAKE_STRING_ID(cbt_spam, attack_result_attacker_fullname_brief);
	MAKE_STRING_ID(cbt_spam, attack_result_defender);
	MAKE_STRING_ID(cbt_spam, attack_result_defender_brief);
	MAKE_STRING_ID(cbt_spam, attack_result_defender_fullname);
	MAKE_STRING_ID(cbt_spam, attack_result_defender_fullname_brief);
	MAKE_STRING_ID(cbt_spam, attack_result_attack_name);
	MAKE_STRING_ID(cbt_spam, attack_result_weapon);
	MAKE_STRING_ID(cbt_spam, attack_result_success);
	MAKE_STRING_ID(cbt_spam, attack_result_success_critical);
	MAKE_STRING_ID(cbt_spam, attack_result_success_glance);
	MAKE_STRING_ID(cbt_spam, attack_result_success_crush);
	MAKE_STRING_ID(cbt_spam, attack_result_success_strikethrough);
	MAKE_STRING_ID(cbt_spam, attack_result_success_evade);
	MAKE_STRING_ID(cbt_spam, attack_result_success_blocked);
	MAKE_STRING_ID(cbt_spam, attack_result_success_brief);
	MAKE_STRING_ID(cbt_spam, attack_result_success_brief_critical);
	MAKE_STRING_ID(cbt_spam, attack_result_success_brief_glance);
	MAKE_STRING_ID(cbt_spam, attack_result_success_brief_crush);
	MAKE_STRING_ID(cbt_spam, attack_result_success_brief_strikethrough);
	MAKE_STRING_ID(cbt_spam, attack_result_success_brief_evade);
	MAKE_STRING_ID(cbt_spam, attack_result_success_brief_blocked);
	MAKE_STRING_ID(cbt_spam, attack_result_fail);
	MAKE_STRING_ID(cbt_spam, attack_result_fail_dodge);
	MAKE_STRING_ID(cbt_spam, attack_result_fail_parry);
	MAKE_STRING_ID(cbt_spam, attack_result_fail_brief);
	MAKE_STRING_ID(cbt_spam, attack_result_fail_brief_dodge);
	MAKE_STRING_ID(cbt_spam, attack_result_fail_brief_parry);
	MAKE_STRING_ID(cbt_spam, attack_result_damage_detail_start);
	MAKE_STRING_ID(cbt_spam, attack_result_damage_detail_end);
	MAKE_STRING_ID(cbt_spam, attack_result_total_damage);
	MAKE_STRING_ID(cbt_spam, attack_result_total_damage_brief);
	MAKE_STRING_ID(cbt_spam, attack_result_damage);
	MAKE_STRING_ID(cbt_spam, attack_result_damage_energy);
	MAKE_STRING_ID(cbt_spam, attack_result_damage_kinetic);
	MAKE_STRING_ID(cbt_spam, attack_result_elemental_acid);
	MAKE_STRING_ID(cbt_spam, attack_result_elemental_cold);
	MAKE_STRING_ID(cbt_spam, attack_result_elemental_electrical);
	MAKE_STRING_ID(cbt_spam, attack_result_elemental_heat);
	MAKE_STRING_ID(cbt_spam, attack_result_environmental_heat);
	MAKE_STRING_ID(cbt_spam, attack_result_environmental_cold);
	MAKE_STRING_ID(cbt_spam, attack_result_environmental_acid);
	MAKE_STRING_ID(cbt_spam, attack_result_environmental_electrical);
	MAKE_STRING_ID(cbt_spam, attack_result_armor);
	MAKE_STRING_ID(cbt_spam, attack_result_armor_fullname);
	MAKE_STRING_ID(cbt_spam, attack_result_armor_generic);
	MAKE_STRING_ID(cbt_spam, attack_result_armor_generic_fullname);
	MAKE_STRING_ID(cbt_spam, attack_result_armor_protection);
	MAKE_STRING_ID(cbt_spam, attack_result_armor_unowned);
	MAKE_STRING_ID(cbt_spam, attack_result_raw_damage_connect);
	MAKE_STRING_ID(cbt_spam, attack_result_raw_damage);
	MAKE_STRING_ID(cbt_spam, attack_result_connect);
	MAKE_STRING_ID(cbt_spam, attack_result_end_sentence);

	MAKE_STRING_ID(cbt_spam, no_command_description);
};

#undef MAKE_STRING_ID

//======================================================================

#endif
