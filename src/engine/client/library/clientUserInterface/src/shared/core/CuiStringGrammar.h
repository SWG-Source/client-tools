//======================================================================
//
// CuiStringGrammar.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringGrammar_H
#define INCLUDED_CuiStringGrammar_H

//======================================================================
namespace CuiStringGrammar
{

	/**
	* These constants represent the lines in each grammar file entry.
	* - The 0th line is the localized or display name for the social.  It is used as the command name.
	* - The 1st-3rd lines are animation names.  Use a ~ if there is no animation.  If the Self or Other
	*   animation is not found, then the system attempts to fallback to the LMF_Animation_Target_None.
	* - E_Error_No_Target is the error message displayed if the social requires a target and
	*   the user did not specify one.  Put a ~ here if it is not applicable.
	* - All the rest of the lines are the string messages output by the social.
	*   The names of the lines are structured like this:
	*   E_<Message Output Recepient>_<Social User Gender>_Target_<Target Identifier>
	*   <Target Identifier> can be None, Self, or C_<Target Gender>.
	*  (The C_ stands for "character", although we use the same messages for inanimate items)
	*/

	enum Entries
	{
		E_Name,                  // 0
		E_Animation_Target_None,
		E_Animation_Target_Self,
		E_Animation_Target_Other,
		E_Error_No_Target,

		//----------------------------------------------------------------------
		//-- no target

		E_Self___N_Target_None,  // 5
		E_Self___M_Target_None,
		E_Self___F_Target_None,
		E_Other__N_Target_None,
		E_Other__M_Target_None,
		E_Other__F_Target_None,  // 10

		//----------------------------------------------------------------------
		//-- target self

		E_Self___N_Target_Self,
		E_Self___M_Target_Self,
		E_Self___F_Target_Self,
		E_Other__N_Target_Self,
		E_Other__M_Target_Self,  // 15
		E_Other__F_Target_Self,

		//----------------------------------------------------------------------
		//-- target other character (messages to self)

		E_Self___N_Target_C_N,
		E_Self___N_Target_C_M,
		E_Self___N_Target_C_F,

		E_Self___M_Target_C_N,  // 20
		E_Self___M_Target_C_M,
		E_Self___M_Target_C_F,

		E_Self___F_Target_C_N,
		E_Self___F_Target_C_M,
		E_Self___F_Target_C_F, // 25

		//----------------------------------------------------------------------
		//-- target other character (messages to others)

		E_Other__N_Target_C_N,
		E_Other__N_Target_C_M,
		E_Other__N_Target_C_F,

		E_Other__M_Target_C_N,
		E_Other__M_Target_C_M, // 30
		E_Other__M_Target_C_F,

		E_Other__F_Target_C_N,
		E_Other__F_Target_C_M,
		E_Other__F_Target_C_F,

		//----------------------------------------------------------------------
		//-- target other character (messages to target)

		E_Target_N_Target_C_N,  // 35
		E_Target_N_Target_C_M,
		E_Target_N_Target_C_F,

		E_Target_M_Target_C_N,
		E_Target_M_Target_C_M,
		E_Target_M_Target_C_F,  // 40

		E_Target_F_Target_C_N,
		E_Target_F_Target_C_M,
		E_Target_F_Target_C_F,

		E_Null
	};
}

//======================================================================

#endif
