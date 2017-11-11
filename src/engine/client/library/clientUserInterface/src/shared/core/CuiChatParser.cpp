// ======================================================================
//
// CuiChatParser.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiChatParser.h"

#include "LocalizationManager.h"
#include "LocalizedStringTable.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/MoodManagerClient.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiSocialsManager.h"
#include "clientUserInterface/CuiSpatialChatManager.h"
#include "clientUserInterface/CuiStringIdsChatRoom.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedGame/GameLanguageManager.h"
#include "sharedGame/SocialsManager.h"
#include "sharedGame/SpatialChatManager.h"
#include "sharedObject/Object.h"

// ======================================================================

namespace
{
	bool                         s_installed;
	
	Unicode::unicode_char_t      s_quoteTickChar = '\'';
	Unicode::unicode_char_t      s_asteriskChar  = '*';
	Unicode::unicode_char_t      s_moodChar      = s_asteriskChar;
	Unicode::unicode_char_t      s_slashChar     = '/';
	Unicode::unicode_char_t      s_chatChar      = s_slashChar;
	Unicode::unicode_char_t      s_geChar        = '>';
	Unicode::unicode_char_t      s_targetChar    = s_geChar;
	Unicode::unicode_char_t      s_periodChar      = '.';
	Unicode::unicode_char_t      s_stopParsingChar = s_periodChar;

	Unicode::unicode_char_t      s_emoteChar     = ':';

	typedef std::map<Unicode::String, uint32> EmoticonSocialMap;
	EmoticonSocialMap s_emoticonSocials;

	std::string                  s_chat_emoticon_socials_filename = "chat_emoticon_socials";

	//-----------------------------------------------------------------
		
	const uint32 findEmoticonInfo (const Unicode::String & str)
	{		
		if (!CuiPreferences::getChatAutoEmote ())
			return 0;

		size_t pos      = 0;
		uint32 socialId = 0;
		size_t pos_end  = 0;
				
		//-- emoticons are searched in reverse order so as to check for the 'longer' emoticons first
		//-- this prevents >:) from being interpreted as simply :)
		
		const EmoticonSocialMap::reverse_iterator end = s_emoticonSocials.rend ();
		for (EmoticonSocialMap::reverse_iterator it = s_emoticonSocials.rbegin (); it != end; ++it)
		{			
			const Unicode::String & emoticon = (*it).first;
			
			const size_t epos = str.find (emoticon);
			
			if (epos != Unicode::String::npos && (pos_end == 0 || epos > pos_end))
			{
				//-- emoticons must be delimited by non alphanumeric characters
				if (epos > 0)
				{
					const Unicode::unicode_char_t prev_char = str [epos - 1];
					if (isalnum (prev_char))
						continue;
				}

				if ((epos + emoticon.size ()) < str.size ())
				{
					const Unicode::unicode_char_t next_char = str [epos + emoticon.size ()];
					if (isalnum (next_char))
						continue;
				}

				pos = epos;
				pos_end = pos + emoticon.size ();
				socialId = (*it).second;
			}
		}

		return socialId;
	}
}

//-----------------------------------------------------------------

Unicode::unicode_char_t            CuiChatParser::ms_cmdChar         = '/';
uint32                             CuiChatParser::ms_emoteChatType   = 0;

//-----------------------------------------------------------------

void CuiChatParser::install ()
{
	DEBUG_FATAL (s_installed, ("already installed.\n"));
	
	ms_emoteChatType   = SpatialChatManager::getChatTypeByName ("emote");
	
	s_installed = true;

	const LocalizedStringTable * const table = LocalizationManager::getManager ().fetchStringTable (s_chat_emoticon_socials_filename);
	if (!table)
		WARNING (true, ("CuiChatParser could not load [%s]", s_chat_emoticon_socials_filename.c_str ()));
	else
	{
		const LocalizedStringTable::NameMap_t & nameMap = table->getNameMap ();
		for (LocalizedStringTable::NameMap_t::const_iterator nit = nameMap.begin (); nit != nameMap.end (); ++nit)
		{
			const std::string & socialName  = (*nit).first;
			const uint32        stringIndex = (*nit).second;

			const uint32 socialId = SocialsManager::getSocialTypeByName (socialName);
			if (socialId == 0)
			{
				WARNING (true, ("CuiChatParser [%s] specifies invalid social [%s]", s_chat_emoticon_socials_filename.c_str (), socialName.c_str ()));
				continue;
			}

			const LocalizedString * const locstr = table->getLocalizedString (stringIndex);
			NOT_NULL (locstr);

			const size_t numLines = locstr->getNumLines ();
			for (size_t i = 0; i < numLines; ++i)
			{
				const Unicode::String & line = locstr->getStringLine (i);
				if (line.empty ())
				{
					WARNING (true, ("CuiChatParser [%s] social [%s] has blank line at line [%d]", s_chat_emoticon_socials_filename.c_str (), socialName.c_str (), i));
					continue;
				}

				s_emoticonSocials.insert (std::make_pair (line, socialId));
			}
		}

		LocalizationManager::getManager ().releaseStringTable (table);
	}
}

//-----------------------------------------------------------------

void CuiChatParser::remove  ()
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));
	
	s_installed = false;
}

//-----------------------------------------------------------------

bool CuiChatParser::parse (const Unicode::String & rawCmd, Unicode::String & result, uint32 chatRoomId, bool useChatRoom)
{
	Unicode::String cmd (Unicode::getTrim (rawCmd));
	static const Unicode::unicode_char_t token_sep [] = { '.', '>', ' ', '\n', '\t', 0x3000, 0 };

	if (cmd.empty ())
		return true;

	size_t curPos = 0;
	size_t endPos = 0;		

	Unicode::String token;
	uint32 mood     = 0;
	uint32 chatType = 0;
	uint32 language = 0;

	const CreatureObject * const player = dynamic_cast<CreatureObject *>(Game::getPlayer ());

	//-----------------------------------------------------------------
	//-- no cmd prefix, simply send the message as raw text

	// See if the first token is a language type, only the first token can
	// be a language

	if (cmd [curPos] == ms_cmdChar)
	{
		if (Unicode::getFirstToken(cmd, 1, endPos, token, token_sep) &&
		    GameLanguageManager::isLanguageAbbreviationValid(token, language))
		{
			PlayerObject *playerObject = Game::getPlayerObject();

			if ((playerObject != NULL) &&
				!playerObject->speaksLanguage(language))
			{
				std::string languageName;
				GameLanguageManager::getLanguageName(language, languageName);

				result +=  Unicode::narrowToWide("You do not speak " + languageName + ".");
				return false;
			}

			// Chew the language token

			curPos = Unicode::skipWhitespace(cmd, endPos);

			if (!Unicode::getFirstToken(cmd, curPos, endPos, token, token_sep))
			{
				result +=  Unicode::narrowToWide("No text after the language command.");
				return false;
			}
		}
	}

	if (cmd [curPos] == ms_cmdChar)
		++curPos;
	else if (cmd [curPos] != s_targetChar)
	{
		if (cmd [curPos] == s_stopParsingChar)
			++curPos;
		else if (cmd [curPos] == s_emoteChar)
		{
			++curPos;
			chatType = ms_emoteChatType;
		}
		
		if (curPos < cmd.length ())
		{
			if (curPos == curPos && useChatRoom)
			{
				if (chatRoomId)
					CuiChatRoomManager::sendToRoom (chatRoomId, cmd, Unicode::String ());
				else
					result += CuiStringIdsChatRoom::channel_unavailable.localize ();
			}
			else
			{
				const bool isPrivate = SpatialChatManager::isPrivate (chatType);
				const Unicode::String text (Unicode::getTrim (cmd.substr (curPos)));

				PlayerObject *playerObject = Game::getPlayerObject();

				if ((language == 0) &&
				    (playerObject != NULL))
				{
					language = playerObject->getSpokenLanguage();
				}

				CuiSpatialChatManager::sendMessage (NetworkId::cms_invalid, chatType, player ? player->getMood () : 0, text, isPrivate, false, language);

				const uint32 emoticon = findEmoticonInfo (text);
				if (emoticon)
					CuiSocialsManager::sendMessage (NetworkId::cms_invalid, emoticon, true, false);
			}
			return true;
		}
		else
		{
			result +=  Unicode::narrowToWide ("all command text got stripped at the beginning.");
			return false;
		}
	}

	//-----------------------------------------------------------------
	//-- are there any normal tokens?

	if ((cmd[curPos] != s_targetChar) && (cmd[curPos] != s_stopParsingChar))
	{
		//-----------------------------------------------------------------
		//-- parse first token

		if (!Unicode::getFirstToken(cmd, curPos, endPos, token, token_sep))
		{
			result +=  Unicode::narrowToWide("No tokens in the command.");
			return false;
		}

		//-----------------------------------------------------------------
		//-- are there any normal tokens?

		if (cmd [curPos] != s_targetChar && cmd [curPos] != s_stopParsingChar)
		{
			//-----------------------------------------------------------------
			//-- parse first token

			if (!Unicode::getFirstToken (cmd, curPos, endPos, token, token_sep))
			{
				result +=  Unicode::narrowToWide ("No tokens in the command.");
				return false;
			}

			Unicode::NarrowString chatCmdStr;
			
			//-----------------------------------------------------------------
			//-- see if it matches a mood or chattype

			chatType = CuiSpatialChatManager::findChatTypeByLocalizedAbbrev (token);

			//-----------------------------------------------------------------
			//-- this token could be either a mood or a command
			//-- always select the chatType command if found
			
			if (chatType)
			{
				mood = player ? player->getMood () : 0;
				curPos = Unicode::skipWhitespace (cmd, endPos);
			}
			
			else
			{
				mood  = MoodManagerClient::getMoodByAbbrevNoCase (token);
				
				//-----------------------------------------------------------------
				//-- this is a mood, look for a chattype following
				
				if (mood)
				{
					curPos = Unicode::skipWhitespace (cmd, endPos);
					
					if (cmd [curPos] == ms_cmdChar)
					{
						++curPos;
						if (!Unicode::getFirstToken (cmd, curPos, endPos, token, token_sep))
						{
							result +=  Unicode::narrowToWide ("No chat type specified after the '/' character.");
							return false;
						}
						
						chatType = CuiSpatialChatManager::findChatTypeByLocalizedAbbrev (token);
						
						if (chatType)
							curPos = Unicode::skipWhitespace (cmd, endPos);
						else
						{
							result += Unicode::narrowToWide ("No such chat type: ") + token;
							return false;
						}
					}
				}
				//-----------------------------------------------------------------
				//-- the token didn't match any command, USER ERROR! :)
				
				else
				{
					result += Unicode::narrowToWide ("No such command, mood, chat type: ") + token;
					return false;
				}
			}
		}
	}

	NetworkId targetId;

	//-----------------------------------------------------------------
	//-- finally see if there is a target

	if (cmd [curPos] == s_targetChar)
	{
	
		++curPos;

		if (!Unicode::getFirstToken (cmd, curPos, endPos, token, token_sep))
		{
			result +=  Unicode::narrowToWide ("No target in the command.");
			return false;
		}

		const Object * target = 0;
		const Unicode::NarrowString narrowLowerToken (Unicode::toLower (Unicode::wideToNarrow (token)));

		// @todo: localize this
		if (narrowLowerToken == "self" || narrowLowerToken == "me" || narrowLowerToken == "this")
			target = player;
		else if (player)
		{
			target = ClientWorld::findObjectByLocalizedAbbrev (player->getPosition_w (), token);
			ClientObject const * const clientObject = target ? target->asClientObject () : 0;
			CreatureObject const * const creatureObject = clientObject ? clientObject->asCreatureObject () : 0;
			if (creatureObject && !creatureObject->getCoverVisibility() && !PlayerObject::isAdmin() && !creatureObject->isPassiveRevealPlayerCharacter(Game::getPlayerNetworkId()))
				target = 0;
		}

		if (target == 0)
		{
			result +=  Unicode::narrowToWide ("no such target.");
			return false;
		}

		targetId = target->getNetworkId ();

		curPos = Unicode::skipWhitespace (cmd, endPos);
	}

	//----------------------------------------------------------------------

	if (cmd [curPos] == s_stopParsingChar)
	{
		curPos = Unicode::skipWhitespace (cmd, curPos+1);
	}
	
	if (curPos < cmd.length ())
	{
		const Unicode::String & text = cmd.substr (curPos);
		const bool isPrivate = SpatialChatManager::isPrivate (chatType);
		CuiSpatialChatManager::sendMessage (targetId, chatType, mood, text, isPrivate, false, language);

		const uint32 emoticon = findEmoticonInfo (text);
		if (emoticon)
			CuiSocialsManager::sendMessage (targetId, emoticon, true, false);

		return true;
	}

	result +=  Unicode::narrowToWide ("No message text specified.");
	return false;

}

// ======================================================================
