//======================================================================
//
// CuiStringGrammarManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiStringGrammarManager.h"

#include "LocalizationManager.h"
#include "LocalizedString.h"
#include "LocalizedStringTable.h"
#include "clientGame/ClientObject.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiStringGrammar.h"

//======================================================================

using namespace CuiStringGrammar;

//----------------------------------------------------------------------

namespace
{
	//----------------------------------------------------------------------

	enum GrammarGenderId
	{

		G_None,
		G_Self,
		G_Neuter,
		G_Male,
		G_Female
	};

	const char * const GrammarGenderString [static_cast<int>(G_Female) + 1] =
	{
		"None",
		"Self",
		"Neuter",
		"Male",
		"Female"
	};

	enum PersonViewer
	{
		PV_First,
		PV_Second,
		PV_Third
	};

	//----------------------------------------------------------------------

	struct GrammarDescriptor
	{
		PersonViewer     person; // 1st, 2nd, 3rd
		GrammarGenderId  object;
		GrammarGenderId  subject;		
	};
	
	//----------------------------------------------------------------------

	bool operator== (const GrammarDescriptor & lhs, const GrammarDescriptor & rhs)
	{
		return rhs.person == lhs.person && rhs.object == lhs.object && rhs.subject == lhs.subject;
	}
	
	//----------------------------------------------------------------------

	struct SocialMessageMapping
	{
		GrammarDescriptor           id;
		CuiStringGrammar::Entries entry;
	};

	const SocialMessageMapping socialMapping [] =
	{
		{ { PV_First,  G_Neuter, G_None },     E_Self___N_Target_None },
		{ { PV_First,  G_Male,   G_None },     E_Self___M_Target_None },
		{ { PV_First,  G_Female, G_None },     E_Self___F_Target_None },
		{ { PV_Third,  G_Neuter, G_None },     E_Other__N_Target_None },
		{ { PV_Third,  G_Male,   G_None },     E_Other__M_Target_None },
		{ { PV_Third,  G_Female, G_None },     E_Other__F_Target_None },

		//----------------------------------------------------------------------
		//-- target self

		{ { PV_First,  G_Neuter, G_Self },     E_Self___N_Target_Self },
		{ { PV_First,  G_Male,   G_Self },     E_Self___M_Target_Self },
		{ { PV_First,  G_Female, G_Self },     E_Self___F_Target_Self },
		{ { PV_Third,  G_Neuter, G_Self },     E_Other__N_Target_Self },
		{ { PV_Third,  G_Male,   G_Self },     E_Other__M_Target_Self },
		{ { PV_Third,  G_Female, G_Self },     E_Other__F_Target_Self },

		//----------------------------------------------------------------------
		//-- target other character (messages to self)

		{ { PV_First,  G_Neuter, G_Neuter },     E_Self___N_Target_C_N },
		{ { PV_First,  G_Neuter, G_Male },       E_Self___N_Target_C_M },
		{ { PV_First,  G_Neuter, G_Female },     E_Self___N_Target_C_F },

		{ { PV_First,  G_Male,   G_Neuter },     E_Self___M_Target_C_N },
		{ { PV_First,  G_Male,   G_Male },       E_Self___M_Target_C_M },
		{ { PV_First,  G_Male,   G_Female },     E_Self___M_Target_C_F },

		{ { PV_First,  G_Male,   G_Neuter },     E_Self___F_Target_C_N },
		{ { PV_First,  G_Male,   G_Male },       E_Self___F_Target_C_M },
		{ { PV_First,  G_Male,   G_Female },     E_Self___F_Target_C_F },

		//----------------------------------------------------------------------
		//-- target other character (messages to others)

		{ { PV_Third, G_Neuter, G_Neuter },     E_Other__N_Target_C_N },
		{ { PV_Third, G_Neuter, G_Male },       E_Other__N_Target_C_M },
		{ { PV_Third, G_Neuter, G_Female },     E_Other__N_Target_C_F },

		{ { PV_Third, G_Neuter, G_Neuter },     E_Other__M_Target_C_N },
		{ { PV_Third, G_Neuter, G_Male },       E_Other__M_Target_C_M },
		{ { PV_Third, G_Neuter, G_Female },     E_Other__M_Target_C_F },

		{ { PV_Third, G_Neuter, G_Neuter },     E_Other__F_Target_C_N },
		{ { PV_Third, G_Neuter, G_Male },       E_Other__F_Target_C_M },
		{ { PV_Third, G_Neuter, G_Female },     E_Other__F_Target_C_F },

		//----------------------------------------------------------------------
		//-- target other character (messages to target)

		{ { PV_Second,  G_Neuter, G_Neuter },     E_Target_N_Target_C_N },
		{ { PV_Second,  G_Neuter, G_Male },       E_Target_N_Target_C_M },
		{ { PV_Second,  G_Neuter, G_Female },     E_Target_N_Target_C_F },

		{ { PV_Second,  G_Neuter, G_Neuter },     E_Target_M_Target_C_N },
		{ { PV_Second,  G_Neuter, G_Male },       E_Target_M_Target_C_M },
		{ { PV_Second,  G_Neuter, G_Female },     E_Target_M_Target_C_F },

		{ { PV_Second,  G_Neuter, G_Neuter },     E_Target_F_Target_C_N },
		{ { PV_Second,  G_Neuter, G_Male },       E_Target_F_Target_C_M },
		{ { PV_Second,  G_Neuter, G_Female },     E_Target_F_Target_C_F },
	};

	//----------------------------------------------------------------------

	bool getRawSocialMessage (const LocalizedString & locstr, const GrammarDescriptor & id,  Unicode::String & result)
	{
		Entries entry = E_Null;

		bool found = false;
 
		const size_t num_mappings = sizeof (socialMapping) / sizeof (socialMapping [0]);
		// @todo: map this into stl map
		for (size_t i = 0; i < num_mappings; ++i)
		{
			if (socialMapping [i].id == id)
			{
				entry = socialMapping [i].entry;
				found = true;
				break;
			}
		}

		if (!found)
			return false;

		result = locstr.getStringLine (static_cast<size_t>(entry));
		if (result[0] == '~' || result [0] == '\n')
			return false;

		return true;
	}
}

//----------------------------------------------------------------------

bool CuiStringGrammarManager::getRawMessage (const StringId & id, const ClientObject * source, const ClientObject * target, Unicode::String & str)
{
	const LocalizedStringTable * const table = LocalizationManager::getManager ().fetchStringTable   (id.getTable ());

	if (table)
	{
		bool retval = false;

		const LocalizedString * const locStr = table->getLocalizedString (id.getText ());

		if (locStr)
		{
			retval = getRawMessage (*locStr, source, target, str);
			WARNING (!retval, ("CuiStringGrammarManager:: raw message retrieval failed for %s.", id.getDebugString ().c_str ()));
		}
		else
			WARNING (true, ("CuiStringGrammarManager:: No string entry in table for grammar entry %s.", id.getDebugString ().c_str ()));

		LocalizationManager::getManager ().releaseStringTable (table);
		return retval;
	}
	else
		WARNING (true, ("CuiStringGrammarManager:: No string table for grammar entry %s.", id.getDebugString ().c_str ()));

	return false;
}

//----------------------------------------------------------------------

bool CuiStringGrammarManager::getRawMessage (const LocalizedString & locstr, const ClientObject * source, const ClientObject * target, Unicode::String & result)
{
	const ClientObject * const player = Game::getClientPlayer ();
	
	if (!player)
		return false;
	
	GrammarDescriptor id =
	{
		source == player ? PV_First : (target == player ? PV_Second : PV_Third),
		G_Male,
		(target) ? (target == source ? G_Self : G_Male) : G_None
	};
	
	GrammarDescriptor originalId = id;

	for (;;)
	{
		if (getRawSocialMessage (locstr, id, result))
			return true;
		
		if (id.subject == G_Female || id.subject == G_Male)
			id.subject = G_Neuter;
		else if (id.object == G_Female || id.object == G_Male)
			id.object = G_Neuter;
		else
		{
			WARNING (true, ("CuiStringGrammarManager:: No string or fallbacks for grammar entry %s. for %s POV object=%s, subject=%s, original object=%s, subject=%s", 
				Unicode::wideToNarrow (locstr.getStringLine (0)).c_str (),
				id.person  == PV_First ? "first" : (id.person == PV_Second ? "second" : "third"),
				GrammarGenderString [static_cast<int>(id.object)],
				GrammarGenderString [static_cast<int>(id.subject)],
				GrammarGenderString [static_cast<int>(originalId.object)],
				GrammarGenderString [static_cast<int>(originalId.subject)]
				));
			
			break; //lint !e527
		}
	}
	
	return false; //lint !e527
}

//======================================================================
