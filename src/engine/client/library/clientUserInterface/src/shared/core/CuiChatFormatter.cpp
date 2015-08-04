// ======================================================================
//
// CuiChatFormatter.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiChatFormatter.h"

#include "LocalizationManager.h"
#include "LocalizedStringTable.h"
#include "UIManager.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientObject.h"
#include "clientGame/Game.h"
#include "clientGame/MoodManagerClient.h"
#include "clientGame/ShipObject.h"
#include "clientUserInterface/CuiSpatialChatManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "sharedGame/GameLanguageManager.h"
#include "sharedGame/SpatialChatManager.h"
#include "sharedNetworkMessages/MessageQueueSpatialChat.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedRandom/Random.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

namespace CuiChatFormatterNamespace
{
	namespace StringTable
	{
		const std::string chat_format_abbrevs = "chat_format_abbrevs";
		const std::string chat_prepositions   = "chat_prepositions";
	}

	const Unicode::String  s_msgSep  = Unicode::narrowToWide (": ");
	const Unicode::String  s_green   = Unicode::narrowToWide ("\\#11ff44");
	const Unicode::String  s_white   = Unicode::narrowToWide ("\\#ffffff");
	const Unicode::String  s_yellow  = Unicode::narrowToWide ("\\#ffff44");

	bool              s_installed;

	const LocalizedStringTable * s_abbrevsTable;

	const LocalizedStringTable * s_prepositionTable;
	Unicode::String              s_defaultPreposition;
}
using namespace CuiChatFormatterNamespace;

//-----------------------------------------------------------------

void CuiChatFormatter::install ()
{
	DEBUG_FATAL (s_installed, ("already installed.\n"));

	s_installed = true;

	s_abbrevsTable = LocalizationManager::getManager ().fetchStringTable (StringTable::chat_format_abbrevs);

	WARNING_STRICT_FATAL (!s_abbrevsTable, ("Unable to locate chat abbrevs table: %s", StringTable::chat_format_abbrevs.c_str ()));

	s_prepositionTable = LocalizationManager::getManager ().fetchStringTable (StringTable::chat_prepositions);

	if (s_prepositionTable)
	{
		const LocalizedString * const default_prep_locstr = s_prepositionTable->getLocalizedString ("default");

		if (default_prep_locstr)
			s_defaultPreposition = default_prep_locstr ? default_prep_locstr->getString () : Unicode::narrowToWide ("to");
		else
			WARNING_STRICT_FATAL (true, ("No default preposition"));
	}
	else
	{
		WARNING_STRICT_FATAL (true, ("Unable to locate chat prepositions table: %s", StringTable::chat_prepositions.c_str ()));
		s_defaultPreposition = Unicode::narrowToWide ("ERROR_PREPOSITION");
	}
}

//-----------------------------------------------------------------

void CuiChatFormatter::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	if (s_abbrevsTable)
	{
		LocalizationManager::getManager ().releaseStringTable (s_abbrevsTable);
		s_abbrevsTable = 0;
	}

	if (s_prepositionTable)
	{
		LocalizationManager::getManager ().releaseStringTable (s_prepositionTable);
		s_prepositionTable = 0;
	}

	s_installed = false;
}

//----------------------------------------------------------------------

void CuiChatFormatter::formatStandard  (const MessageQueueSpatialChat & spatialChat, Unicode::String & str, TextPosition position, bool randomMoodPhrase)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	const static bool useJapanese = UIManager::gUIManager().isLocaleJapanese();

	const ClientObject * source = safe_cast<const ClientObject *>(NetworkIdManager::getObjectById(spatialChat.getSourceId ()));
	const ClientObject * target = safe_cast<const ClientObject *>(NetworkIdManager::getObjectById(spatialChat.getTargetId ()));

	if (source == Game::getPlayerPilotedShip() && target == Game::getClientPlayer())
	{
		// Don't add "x says to y" to text that the ship you're piloting says to you, because it looks strange
		return;
	}

	NOT_NULL (source);

	const Unicode::String sourceName (source->getLocalizedName());

	Unicode::String phrase = sourceName;

	const uint32 chatType = spatialChat.getChatType () ? spatialChat.getChatType () : SpatialChatManager::getDefaultChatType ();
	const Unicode::String verbPhrase (CuiSpatialChatManager::getChatVerbPhrase (chatType));

	const Unicode::String * preposition = 0;

	if (target)
	{
		std::string canonicalChatName;
		if (!SpatialChatManager::getChatNameByType (chatType, canonicalChatName))
		{
			WARNING (true, ("Can't find chat type for %d", chatType));
			return;
		}

		const LocalizedString * const prep_locstr = s_prepositionTable ? s_prepositionTable->getLocalizedString (canonicalChatName) : 0;
		if (prep_locstr)
		{
			preposition = &prep_locstr->getString ();
			if (preposition->empty ())
				preposition = 0;
		}
		else
			preposition = &s_defaultPreposition;
	}

	const Unicode::String moodPhraseRaw (spatialChat.getMoodType () ? MoodManagerClient::getMoodAdverbPhrase (spatialChat.getMoodType (), randomMoodPhrase) : Unicode::String ());

	CuiStringVariablesData csvd;
	csvd.source = source;
	csvd.target = target;

	Unicode::String moodPhrase;
	CuiStringVariablesManager::process (moodPhraseRaw, csvd, moodPhrase);

	//-----------------------------------------------------------------
	//-- moodphrases that start with a comma are required to go after the verb and after the target

	if (!moodPhrase.empty () && moodPhrase [0] == ',')
	{
		IGNORE_RETURN (phrase.append (1, ' '));
		if (!useJapanese)
			phrase += verbPhrase;

		if (target)
		{
			if (preposition)
			{
				IGNORE_RETURN (phrase.append (1, ' '));
				phrase += *preposition;
			}

			IGNORE_RETURN (phrase.append (1, ' '));
			phrase += target->getLocalizedName ();
		}

		phrase += moodPhrase;
		if (useJapanese)
			phrase += verbPhrase;
	}

	//-----------------------------------------------------------------
	//-- targets with a null preposition require that the simplified mood phrase
	//-- go BEFORE the verb phrase:  e.g. Jack angrily commands Jill, 'gimme.'
	//--                                  Jack heatedly debates Jill, 'that's wrong.'

	else if (target && !preposition)
	{
		if (!moodPhrase.empty () && moodPhrase[0] != ' ')
		{
			IGNORE_RETURN (phrase.append (1, ' '));
			phrase += moodPhrase;
		}

		if (!useJapanese)
			IGNORE_RETURN (phrase.append (1, ' '));
		phrase += verbPhrase;
		if (!useJapanese)
			IGNORE_RETURN (phrase.append (1, ' '));
		phrase += target->getLocalizedName ();
	}

	//-----------------------------------------------------------------
	//-- all other normal cases handled here

	else
	{
		if (!useJapanese)
		{
			IGNORE_RETURN (phrase.append (1, ' '));
			phrase += verbPhrase;
		}

		if (!moodPhrase.empty ())
		{
			if (!useJapanese)
				IGNORE_RETURN (phrase.append (1, ' '));
			phrase += moodPhrase;
		}
		if (useJapanese)
			phrase += verbPhrase;

		if (target)
		{
			if (preposition)
			{
				if (!useJapanese)
					IGNORE_RETURN (phrase.append (1, ' '));
				phrase += *preposition;
			}

			if (!useJapanese)
				IGNORE_RETURN (phrase.append (1, ' '));
			phrase += target->getLocalizedName ();
		}
	}

	//-----------------------------------------------------------------
	//-- stick it all together.

	if (position == TP_First)
	{
		Unicode::String prefix;
		IGNORE_RETURN (str.insert (0U, 1U, '\"'));
		IGNORE_RETURN (str.append (1, '\"'));
		IGNORE_RETURN (str.append (1, ','));
		IGNORE_RETURN (str.append (1, ' '));
		str += s_green;
		str += phrase;

		if (GameLanguageManager::isLanguageValid(static_cast<int>(spatialChat.getLanguage())) &&
		    (static_cast<int>(spatialChat.getLanguage()) != GameLanguageManager::getBasicLanguageId()))
		{
			if (!useJapanese)
			{
				str += ' ';
				str += CuiStringIds::token_in.localize();
				str += ' ';
			}
			else
			{
				str += '[';
			}

			Unicode::String localizedLanguage;
			GameLanguageManager::getLocalizedLanguageName(static_cast<int>(spatialChat.getLanguage()), localizedLanguage);
			str += localizedLanguage;

			if (useJapanese)
				str += ']';
		}

		str += Unicode::narrowToWide(".\\#.");
	}
	else
	{
		phrase += Unicode::narrowToWide(", \\#.\"");
		str = s_green + phrase + str;

		IGNORE_RETURN (str.append (1, '\"'));

		if (GameLanguageManager::isLanguageValid(static_cast<int>(spatialChat.getLanguage())) &&
		    (static_cast<int>(spatialChat.getLanguage()) != GameLanguageManager::getBasicLanguageId()))
		{
			if (!useJapanese)
			{
				str += ' ';
				str += s_green;
				str += CuiStringIds::token_in.localize();
				str += ' ';
			}
			else
			{
				str += s_green;
				str += '[';
			}
			
			Unicode::String localizedLanguage;
			GameLanguageManager::getLocalizedLanguageName(static_cast<int>(spatialChat.getLanguage()), localizedLanguage);
			str += localizedLanguage;
			
			if (useJapanese)
				str += ']';

			str += Unicode::narrowToWide(".\\#.");
		}
	}
}

//-----------------------------------------------------------------

/**
* @param str should already be loaded with the value of the message string
*/
void CuiChatFormatter::formatDiku    (const MessageQueueSpatialChat & spatialChat, Unicode::String & str)
{
	const static bool useJapanese = UIManager::gUIManager().isLocaleJapanese();
	formatStandard (spatialChat, str, (useJapanese ? TP_First : TP_Last), false);
}

//-----------------------------------------------------------------

/**
* @param str should already be loaded with the value of the message string
* username [verb,mood]>target: message
*/
void CuiChatFormatter::formatRaw (const MessageQueueSpatialChat & spatialChat, Unicode::String & str)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	const ClientObject * const source = safe_cast<const ClientObject *>(NetworkIdManager::getObjectById(spatialChat.getSourceId ()));
	const ClientObject * const target = safe_cast<const ClientObject *>(NetworkIdManager::getObjectById(spatialChat.getTargetId ()));

	NOT_NULL (source);

	const Unicode::String & verbPhrase  = CuiSpatialChatManager::getChatVerbPhrase (spatialChat.getChatType ());
	const Unicode::String & moodPhrase  = spatialChat.getMoodType () ? MoodManagerClient::getMoodAdverbPhrase (spatialChat.getMoodType (), false) : Unicode::String ();
	const static bool useJapanese = UIManager::gUIManager().isLocaleJapanese();

	Unicode::String str_prefix (s_green + source->getLocalizedName());

	if (!verbPhrase.empty () || !moodPhrase.empty ())
	{
		IGNORE_RETURN (str_prefix.append (1, ' '));
		str_prefix += s_yellow;
		IGNORE_RETURN (str_prefix.append (1, '['));
		str_prefix += s_green;

		bool needsComma = false;

		if (!useJapanese && !verbPhrase.empty ())
		{
			str_prefix += verbPhrase;
			needsComma = true;
		}

		if (!moodPhrase.empty ())
		{
			if (needsComma)
			{
				str_prefix += s_yellow;
				IGNORE_RETURN (str_prefix.append (1, ','));
				str_prefix += s_green;
			}

			str_prefix += moodPhrase;
		}

		if (useJapanese && !verbPhrase.empty ())
			str_prefix += verbPhrase;

		str_prefix += s_yellow;
		IGNORE_RETURN (str_prefix.append (1, ']'));
	}

	if (target)
	{
		// @todo the targetname should be "himself/herself/itself" if it == source

		IGNORE_RETURN (str_prefix.append (1, '>'));
		str_prefix += s_green;
		str_prefix += target->getLocalizedName ();
	}

	str = str_prefix + s_white + s_msgSep + str;
}

//-----------------------------------------------------------------

namespace
{
	size_t countOccurances (const Unicode::String & str, Unicode::unicode_char_t c)
	{
		size_t num = 0;
		size_t start = 0;
		while (start != str.npos && (start = str.find (c, start)) != str.npos)  //lint !e737 //loss of sign, stlport bug
		{
			++num;
			++start;
		}
		return num;
	}

	size_t countNestedDepth (const Unicode::String & str, Unicode::unicode_char_t open, Unicode::unicode_char_t close)
	{
		const size_t num_open = countOccurances (str, open);

		if (open == close)
			return num_open % 2;
		else
		{
			const size_t num_close = countOccurances (str, close);
			return num_open - num_close;
		}
	}
}

//----------------------------------------------------------------------

/**
* @param str should already be loaded with the value of the message string
*/
void CuiChatFormatter::formatProse   (const MessageQueueSpatialChat & spatialChat, Unicode::String & str)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	const ClientObject * source = safe_cast<const ClientObject *>(NetworkIdManager::getObjectById(spatialChat.getSourceId ()));

	NOT_NULL (source);

	const static bool useJapanese = UIManager::gUIManager().isLocaleJapanese();

	//----------------------------------------------------------------------
	//-- first handle breaking up the str into 2 parts if possible

	static const Unicode::unicode_char_t breakingPunctuation [] =
	{
		'!', '?', '.', 0
	};

	size_t first_end    = 0;
	size_t second_start = 0;
	size_t first_start = Unicode::skipWhitespace (str, 0);

	if (first_start == str.npos)  //lint !e737 //loss of sign, stlport bug
	{
		str = Unicode::narrowToWide ("empty string in prose formatter.\n");
		return;
	}

	for (size_t pos = 0; pos != str.npos; )  //lint !e737 //loss of sign, stlport bug
	{
		const size_t punc_start = str.find_first_of (breakingPunctuation, pos);
		if (punc_start == str.npos)  //lint !e737 //loss of sign, stlport bug
			break;

		const size_t punc_end   = str.find_first_not_of (breakingPunctuation, punc_start + 1);
		if (punc_end == str.npos)   //lint !e737 //loss of sign, stlport bug
			break;

		//-- all breaks must be followed with a space
		if (str [punc_end] != ' ')
		{
			pos = punc_end;
			continue;
		}

		//-----------------------------------------------------------------
		//-- check for abbrevs to skip
		//-- only look for abbrevs if there was a single '.' as punctuation, followed by a space

		if ((punc_end - punc_start) == 1 && str [punc_end - 1] == '.')
		{
			bool ok = true;

			if (s_abbrevsTable)
			{
				const LocalizedString * const loc_str = NON_NULL (s_abbrevsTable->getLocalizedString ("default"));
				const size_t num_abbrevs = loc_str->getNumLines ();

				// @todo this could be optimized by finding the max abbrev len in install ()
				// peel off the '.' character
				const Unicode::String lesserString (Unicode::toLower (str.substr (first_start, (punc_end - first_start) - 1)));

				for (size_t i = 0; i < num_abbrevs; ++i)
				{
					const Unicode::String abbrev (loc_str->getStringLine (i));

					if (abbrev.empty ())
						continue;

					const size_t abbrev_len = abbrev.size ();

					if ((punc_end - first_start) > abbrev_len)
					{
						if (lesserString.size () < abbrev_len)
							continue;

						//-- the text abbrev must start with a space character to be valid.  Otherwise we would
						//-- refuse to break things like "My name is Ronmr.  I am here to kill you."
						if (lesserString.size () >= (abbrev_len + 1) && lesserString [(lesserString.size () - abbrev_len) - 1] != ' ')
							continue;

						if (abbrev.compare (0, abbrev_len, lesserString, lesserString.size () - abbrev_len, abbrev_len) == 0)
						{
							ok = false;
							break;
						}
					}
				}
			}

			if (!ok)
			{
				pos = punc_end;
				continue;
			}
		}

		//-----------------------------------------------------------------
		//-- check for nested enclosures like (), [], "", etc...

		static const size_t num_enclosing_pairs = 4;
		static const Unicode::unicode_char_t enclosing [num_enclosing_pairs][2] =
		{
			{ '(',  ')' },
			{ '{',  '}' },
			{ '[',  ']' },
			{ '\"', '\"' }
		};

		//-- check backwards for nesting
		const Unicode::String strSegment (str.substr (first_start, punc_end - first_start));

		bool ok = true;
		for (size_t i = 0; i < num_enclosing_pairs; ++i)
		{
			const size_t nested_depth = countNestedDepth (strSegment, enclosing [i][0], enclosing [i][1]);
			if (nested_depth > 0)
			{
				ok = false;
				break;
			}
		}

		if (!ok)
		{
			pos = punc_end;
			continue;
		}

		second_start = str.find_first_not_of (' ', punc_end);
		first_end = punc_end;
		break;
	}

	//----------------------------------------------------------------------
	//-- add end period if needed
	{
		const unsigned short last_char = str [str.size () - 1];
		bool has_end_punctuation = false;
		for (int i = 0; breakingPunctuation [i] != 0; ++i)
		{
			if (breakingPunctuation [i] == last_char)
			{
				has_end_punctuation = true;
				break;
			}
		}

		if (!has_end_punctuation)
			IGNORE_RETURN (str.append (1, '.'));

	}

	//-----------------------------------------------------------------
	//- str has been split into 2 sections

	if (first_end != first_start && first_end != str.npos && second_start != str.npos)   //lint !e737 //loss of sign, stlport bug
	{
		Unicode::String first (str.substr (first_start, first_end - first_start));
		first [0] = static_cast<unsigned short>(toupper (first [0]));

		Unicode::String second (str.substr (second_start));
		second [0] = static_cast<unsigned short>(toupper (second [0]));

		//-- print the split text, split
		if (!Random::random (0, 2))
		{
			formatStandard (spatialChat, first, TP_First, true);

			str = first;
			IGNORE_RETURN (str.append (2, ' '));
			IGNORE_RETURN (str.append (s_white));
			IGNORE_RETURN (str.append (1, '\"'));
			str += second;
			IGNORE_RETURN (str.append (1, '\"'));
			return;
		}

		//-- put the capitalized sentances together, please.
		str = first;
		IGNORE_RETURN (str.append (2, ' '));
		str += second;
	}

	//-----------------------------------------------------------------
	//- there was no split, one section only.

	str = str.substr (first_start);
	str [0] = static_cast<unsigned short>(toupper (str [0]));
	formatStandard (spatialChat, str, (useJapanese ? TP_First : (Random::random (0, 1) ? TP_First : TP_Last)), true);
}

//----------------------------------------------------------------------

void CuiChatFormatter::formatSpaceComm(MessageQueueSpatialChat const &spatialChat, Unicode::String &str)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	const ClientObject * source = safe_cast<const ClientObject *>(NetworkIdManager::getObjectById(spatialChat.getSourceId ()));

	NOT_NULL (source);

	// ShipName - SpeakerName<StationName> : text

	uint32 const flags = spatialChat.getFlags();

	Unicode::String phrase (s_green);
	phrase += source->getLocalizedName();
	phrase += ' ';
	phrase += '-';
	phrase += ' ';
	phrase += spatialChat.getSourceName();
	phrase += '<';
	if (flags & MessageQueueSpatialChat::F_shipPilot)
		phrase += CuiStringIds::spatial_station_pilot.localize();
	if (flags & MessageQueueSpatialChat::F_shipOperations)
		phrase += CuiStringIds::spatial_station_operations.localize();
	if (flags & MessageQueueSpatialChat::F_shipGunner)
		phrase += CuiStringIds::spatial_station_gunner.localize();
	phrase += '>';
	phrase += ' ';
	phrase += ':';
	phrase += ' ';
	phrase += 
	str = phrase + s_white + str;
}

// ======================================================================
