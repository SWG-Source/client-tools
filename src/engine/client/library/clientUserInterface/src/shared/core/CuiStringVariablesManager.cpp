//======================================================================
//
// CuiStringVariablesManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiStringVariablesManager.h"

#include "LocalizationManager.h"
#include "LocalizedString.h"
#include "LocalizedStringTable.h"
#include "UIUtils.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiStringVariables.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedObject/NetworkIdManager.h"

//======================================================================

using namespace CuiStringVariables;

namespace
{
	//----------------------------------------------------------------------
	
	enum VariableGender
	{
		VG_neuter,
		VG_male,
		VG_female
	};
	
	struct VariableCharacter
	{
		Variable                   var;
		Unicode::unicode_char_t    c;
	};
	
	
	const VariableCharacter variableCharacter [CuiStringVariables::V_count] =
	{
		{ V_name,                      'T' },
		{ V_shortName,                 'N' },
		{ V_nameUnfiltered,            'U' },
		{ V_shortNameUnfiltered,       'A' },
		{ V_personalSubjectivePronoun, 'S' },
		{ V_personalObjectivePronoun,  'O' },
		{ V_possessivePronoun,         'P' },
		{ V_faction,                   'F' },
		{ V_species,                   'R' },
		{ V_honorific,                 'H' },
		{ V_militaryRank,              'M' },
		{ V_title,                     'K' },
		{ V_digit,                     'D' },
		{ V_networkId,                 'Z' },
	};
	
	Variable  findVariable (Unicode::unicode_char_t c)
	{
		for (size_t i = 0; i < V_count; ++i)
		{
			if (variableCharacter [i].c == c)
				return variableCharacter [i].var;
		}
			
		return V_none;
	}

	struct NamedStringEntry
	{
		const char *  name;
		uint32        id;
	};

	NamedStringEntry            s_objectivePronoun  = { "pronoun_objective",   0 };
	NamedStringEntry            s_subjectivePronoun = { "pronoun_subjective",  0 };
	NamedStringEntry            s_possessivePronoun = { "pronoun_possessive",  0 };
}

//----------------------------------------------------------------------

/**
* Transform the encoded string into the displayable resultStr string.  All
* encoded variables are properly expanded with fully localized text.
*
*/

void CuiStringVariablesManager::process (const Unicode::String & encoded, const CuiStringVariablesData & data, Unicode::String & resultStr)
{
	const LocalizedStringTable * const ms_grammarTable  = LocalizationManager::getManager ().fetchStringTable ("grammar");

	if (NULL == ms_grammarTable)
	{
		resultStr = encoded;
		return;
	}
	
	if (ms_grammarTable)
	{
		if (s_objectivePronoun.id == 0)
			s_objectivePronoun.id = ms_grammarTable->getIdByName  (s_objectivePronoun.name);
		if (s_subjectivePronoun.id == 0)
			s_subjectivePronoun.id = ms_grammarTable->getIdByName (s_subjectivePronoun.name);
		if (s_possessivePronoun.id == 0)
			s_possessivePronoun.id = ms_grammarTable->getIdByName (s_possessivePronoun.name);
	}
	else
		WARNING (true, ("No grammar table"));
	
	resultStr.clear ();

	static const Unicode::unicode_char_t specialChar = '%';

	const size_t encodedLength = encoded.size ();

	size_t pos = 0;
	size_t next = 0;

	//----------------------------------------------------------------------
	//-- cut the line up, expanding each encoded variable.  Any % special char found is
	//-- stripped and replaced, along with the 2 subsequent characters

	while ((next = encoded.find (specialChar, pos)) != encoded.npos) //lint !e737
	{
		if ((encodedLength - next) < 3)
			break;

		if (next > 0)
			resultStr += encoded.substr (pos, next - pos);

		const Unicode::String         code3 (encoded.substr (next, 3));

		const Unicode::unicode_char_t VariableCode = encoded [next + 1];
		const Variable ge                          = findVariable (VariableCode);

		if (ge == V_none)
		{
			WARNING (true, ("CuiStringVariablesManager: Invalid grammar element code '%c' (in '%s') for message [%s]", VariableCode, Unicode::wideToNarrow (code3).c_str (), Unicode::wideToNarrow (encoded).c_str ()));
			resultStr.append (code3);
			pos = next + 3;
			continue;
		}

		const Unicode::unicode_char_t participantCode    = encoded [next + 2];
		const ClientObject * participant           = 0;
		Unicode::String participantNameOverride;

		if (participantCode == 'U')
		{
			participant             = data.source;
			participantNameOverride = StringId::decodeString (data.sourceName);
		}
		else if (participantCode == 'T')
		{
			participant             = data.target;
			participantNameOverride = StringId::decodeString (data.targetName);
		}
		else if (participantCode == 'O')
		{
			participant             = data.other;
			participantNameOverride = StringId::decodeString (data.otherName);
		}
		else if (ge != V_digit)
			WARNING (true, ("CuiStringVariablesManager: Invalid participant code '%c' (in '%s') for message [%s]", participantCode, Unicode::wideToNarrow (code3).c_str (), Unicode::wideToNarrow (encoded).c_str ()));

		if (!participant && participantNameOverride.empty () && ge != V_digit)
		{
			resultStr.append (code3);
			pos = next + 3;
			continue;
		}

		const CreatureObject * const creature = dynamic_cast<const CreatureObject *> (participant);

		//----------------------------------------------------------------------
		//-- expand the encoded variable variable

		const LocalizedString * locstr = 0;                // case statements set this if they want it to be used at the end of the switch
		bool  findStringLineByGender   = false;
		size_t locstr_index            = 0;

		// @todo: implement the display of faction, species, honorific, military rank, and title

		switch (ge)
		{
		case V_none:
			DEBUG_FATAL (true, ("should not be here.\n"));
			break;
		case V_nameUnfiltered:
		case V_name:
			if (ge == V_nameUnfiltered && creature)
				resultStr += creature->getLocalizedNameUnfiltered();
			else if (participant)
				resultStr += participant->getLocalizedName ();
			else
				resultStr += participantNameOverride;

			break;

		case V_shortName:
		case V_shortNameUnfiltered:
			{
				Unicode::String longName;

				if (ge == V_shortNameUnfiltered && creature)
					longName = creature->getLocalizedNameUnfiltered();
				else if (participant)
					longName = participant->getLocalizedName ();
				else
					longName = participantNameOverride;

				Unicode::String firstPart;
				size_t endpos = 0;
				if (Unicode::getFirstToken (longName, 0, endpos, firstPart))
					resultStr += firstPart;
				else
				{
					WARNING (true, ("bad tokenizing name %s", Unicode::wideToNarrow (longName).c_str ()));
				}
			}
			break;
		case V_personalSubjectivePronoun:
			if (ms_grammarTable)
			{
				locstr = NON_NULL (ms_grammarTable->getLocalizedString (s_subjectivePronoun.id));
				findStringLineByGender = true;
			}
			else
				resultStr.append (code3);
			break;
		case V_personalObjectivePronoun:
			if (ms_grammarTable)
			{
				locstr = NON_NULL (ms_grammarTable->getLocalizedString (s_objectivePronoun.id));
				findStringLineByGender = true;
			}
			else
				resultStr.append (code3);
			break;
		case V_possessivePronoun:
			if (ms_grammarTable)
			{
				locstr = NON_NULL (ms_grammarTable->getLocalizedString (s_possessivePronoun.id));
				findStringLineByGender = true;
			}
			else
				resultStr.append (code3);
			break;
		case V_faction:
			break;
		case V_species:
			if (creature)
			{
				Unicode::String tmp;
				creature->getSpeciesString (tmp);
				resultStr += tmp;
			}
			else if (participant)
			{
				resultStr += GameObjectTypes::getLocalizedName (participant->getGameObjectType ());
			}
			break;
		case V_honorific:
			break;
		case V_militaryRank:
			break;
		case V_title:
			//-- @todo: what do we print if the creature has no title?
			if (creature)
			{
				const PlayerObject * const playerObject = creature->getPlayerObject ();
				if (playerObject)
				{
					resultStr += playerObject->getLocalizedTitle ();
				}
			}
			break;
		case V_digit:
			{
				Unicode::String field;

				if (participantCode == 'I')
					UIUtils::FormatInteger (field, data.digit_i);
				else if (participantCode == 'F')
					UIUtils::FormatFloat   (field, data.digit_f);
				else
				{
					WARNING (true, ("Unknown digit code: '%c'", static_cast<char>(participantCode)));
					resultStr.append (code3);
				}

				resultStr.append (field);
				break;
			}
		case V_networkId:
			{
			const CreatureObject* const playerCreature = dynamic_cast<const CreatureObject*> (data.source);
				if (PlayerObject::isAdmin()) {
					if (participantCode == 'U') {
						resultStr.append(Unicode::narrowToWide(playerCreature->getNetworkId().getValueString().c_str()));
					}
					else if (participantCode == 'T') {
						resultStr.append(Unicode::narrowToWide(playerCreature->getIntendedTarget().getValueString().c_str()));
					}
				}
			}
		}

		//----------------------------------------------------------------------
		//-- we found a localized string to pull a line from
		
		if (locstr)
		{
			if (findStringLineByGender)
			{
				locstr_index = static_cast<size_t>(VG_neuter);
				
				if (creature)
				{
					if (creature->getGender () == SharedCreatureObjectTemplate::GE_male)
						locstr_index = static_cast<size_t>(VG_male);
					else if (creature->getGender () == SharedCreatureObjectTemplate::GE_female)
						locstr_index = static_cast<size_t>(VG_female);
				}
			}

			resultStr += locstr->getStringLine (locstr_index);
		}

		pos = next + 3;
	}

	//-- tack on anything left at the end.
	if (static_cast<int>(pos) != static_cast<int>(encoded.npos))
		resultStr += encoded.substr (pos);

	if (ms_grammarTable)
		LocalizationManager::getManager ().releaseStringTable (ms_grammarTable);
}

//----------------------------------------------------------------------

void CuiStringVariablesManager::process (const StringId & stringId,       const CuiStringVariablesData & data, Unicode::String & resultStr)
{
	const Unicode::String & str = stringId.localize ();
	process (str, data, resultStr);
}

//----------------------------------------------------------------------

void CuiStringVariablesManager::process (const StringId & stringId,       const std::string & source, const std::string & target, Unicode::String & resultStr)
{
	process (stringId, Unicode::narrowToWide (source), Unicode::narrowToWide (target), Unicode::String (), resultStr);
}

//----------------------------------------------------------------------

void CuiStringVariablesManager::process (const StringId & stringId,       const Unicode::String & source, const Unicode::String & target, const Unicode::String & other, Unicode::String & resultStr)
{
	process (stringId, source, target, other, 0, 0.0f, resultStr);
}

//----------------------------------------------------------------------

void  CuiStringVariablesManager::process (const StringId & stringId,       const Unicode::String & source, const Unicode::String & target, const Unicode::String & other, int digitInteger, float digitFloat, Unicode::String & resultStr)
{
	CuiStringVariablesData data;
	data.sourceName = source;
	data.targetName = target;
	data.otherName  = other;
	data.digit_i    = digitInteger;
	data.digit_f    = digitFloat;

	process (stringId, data, resultStr);
}

//----------------------------------------------------------------------

bool CuiStringVariablesManager::stringHasCodeForObject (const Unicode::String & encoded, VariableObject variableObject)
{
	static const Unicode::unicode_char_t specialChar = '%';

	const size_t encodedLength = encoded.size ();

	Unicode::unicode_char_t participantCode;

	switch(variableObject)
	{
	case VO_target:
		participantCode = 'T';
		break;
	case VO_user:
		participantCode = 'U';
		break;
	case VO_other:
		participantCode = 'O';
		break;
	default:
		return false;
	}
	
	size_t pos = 0;
	size_t next = 0;

	// find all of the % codes and see if any of them refer to the object we are interested in
	while ((next = encoded.find (specialChar, pos)) != encoded.npos) //lint !e737
	{
		if ((encodedLength - next) < 3)
			break;

		const Unicode::unicode_char_t VariableCode = encoded [next + 1];
		const Variable ge                          = findVariable (VariableCode);

		if (ge != V_none)
		{
			const Unicode::unicode_char_t thisParticipantCode    = encoded [next + 2];
			if (participantCode == thisParticipantCode)
			{
				return true;
			}
		}
		pos = next + 3;
	}
	return false;
}

//======================================================================

