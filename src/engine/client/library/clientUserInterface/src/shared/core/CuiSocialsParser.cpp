// ======================================================================
//
// CuiSocialsParser.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiSocialsParser.h"

#include "LocalizationManager.h"
#include "LocalizedStringTable.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/NetworkScene.h"
#include "clientGame/PlayerObject.h"
//#include "clientUserInterface/CuiSocials.def"
#include "clientUserInterface/CuiSocialsManager.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedGame/SocialsManager.h"
#include "sharedNetworkMessages/MessageQueueSocial.h"
#include "sharedObject/Object.h"
#include "sharedObject/NetworkIdManager.h"
#include <algorithm>
#include <map>

// ======================================================================

namespace
{

	//-----------------------------------------------------------------

	bool                         s_installed;

	//----------------------------------------------------------------------

	/**
	* The command parser node that represents a social command.  It is a normal
	* command parser with an added uint32 social id piece of data.
	*/

	class SocialCommandParserNode : public CommandParser
	{
	public:
		SocialCommandParserNode (uint32 socialId, const String_t & cmd, size_t minArgs, const String_t & args, const String_t & help, CommandParser * delegate) :
		CommandParser (cmd, minArgs, args, help, delegate),
		m_socialId (socialId)
		{
		}

		uint32 getSocialId () const
		{
			return m_socialId;
		}

	private:
	                               SocialCommandParserNode ();
	                               SocialCommandParserNode (const SocialCommandParserNode & rhs);
		SocialCommandParserNode &  operator=     (const SocialCommandParserNode & rhs);
		uint32 m_socialId;
	};

	const char * const socialsListCommand = "socials";

	typedef std::map <Unicode::String, uint32> EmoticonSocialMap_t;

	EmoticonSocialMap_t * ms_emoticonSocialMap = 0;

	const char * const ms_emoticonSocialTableName = "emoticon_socials";

	typedef std::vector <Unicode::String> StringVector;

	void findCasingPermutations (const Unicode::String & str, size_t start, StringVector & sv)
	{
		const size_t len = str.size ();

		if (start < len)
		{
			const size_t i = start;
			Unicode::String tmp = str;

			const Unicode::unicode_char_t c = tmp [i];

			tmp [i] = static_cast<Unicode::unicode_char_t>(toupper (c));
//			sv.push_back (tmp);

			findCasingPermutations (tmp, start + 1, sv);

			tmp [i] = static_cast<Unicode::unicode_char_t>(tolower (c));
//			sv.push_back (tmp);

			findCasingPermutations (tmp, start + 1, sv);
		}
		else
			sv.push_back (str);
	}

	bool isAlphabeticOnly (const Unicode::String & str)
	{
		static Unicode::String valid;
		static bool init = false;

		if (!init)
		{
			valid.reserve (52);
			int c = 0;

			for (c = 'a'; c <= 'z'; ++c)
				IGNORE_RETURN(valid.append (1, c));
			for (c = 'A'; c <= 'Z'; ++c)
				IGNORE_RETURN(valid.append (1, c));
	
			init = true;
		}

		return str.find_first_not_of (valid) == str.npos;
	}
}

//-----------------------------------------------------------------

CuiSocialsParser::CuiSocialsParser () :
CommandParser ("socials", 0, "...", "all commands", 0)
{
	Unicode::String dummy;
	
	const LocalizedStringTable * const socialsTable = CuiSocialsManager::getSocialsTable ();
	
	if (socialsTable)
	{
		const LocalizedStringTable::Map_t & tableMap = socialsTable->getMap ();
		for ( LocalizedStringTable::Map_t::const_iterator it = tableMap.begin (); it != tableMap.end (); ++it)
		{
			const uint32 locStringId             = (*it).first;
			const LocalizedString * const locstr = NON_NULL ((*it).second);

			const std::string * const name = socialsTable->getStringNameByIdSlowly (locStringId);
			NOT_NULL (name);
			
			if (name)
			{
				const uint32 type = SocialsManager::getSocialTypeByName (*name);
				
				if (type == CuiSocialsManager::getWhisperSocialId ())
					continue;
			
				const Unicode::String & str = locstr->getStringLine (0);

				IGNORE_RETURN (addSubCommand (new SocialCommandParserNode (type, str, 0, dummy, dummy, this)));
			}
		}
	}
	else
		WARNING_STRICT_FATAL (true, ("Unable to initialize CuiSocialsParser without CuiSocialsManager::getSocialsTable ()."));

	IGNORE_RETURN (addSubCommand (new CommandParser (Unicode::narrowToWide (socialsListCommand), 0, dummy, dummy, this)));

}


//----------------------------------------------------------------------

CuiSocialsParser::~CuiSocialsParser ()
{

}

//-----------------------------------------------------------------

void CuiSocialsParser::install ()
{
	DEBUG_FATAL (s_installed, ("already installed\n"));
 
	LocalizedStringTable * const table = LocalizationManager::getManager ().fetchStringTable (ms_emoticonSocialTableName);

	ms_emoticonSocialMap = new EmoticonSocialMap_t;

	if (table)
	{
		const LocalizedStringTable * const socialsTable = CuiSocialsManager::getSocialsTable ();

		if (socialsTable)
		{
			const LocalizedStringTable::NameMap_t & nameMap = table->getNameMap ();
			
			for (LocalizedStringTable::NameMap_t::const_iterator iter = nameMap.begin (); iter != nameMap.end (); ++iter)
			{
				const LocalizedString * const locstr     = NON_NULL (table->getLocalizedString ((*iter).second));
				const Unicode::NarrowString & socialName = (*iter).first;
				
				const uint32 social_type = SocialsManager::getSocialTypeByName (socialName);
				
				if (social_type == 0)
				{
					WARNING (true, ("Invalid social type specified in %s: '%s'", ms_emoticonSocialTableName, socialName.c_str ()));
					continue;
				}
				
				const size_t num_lines = locstr->getNumLines ();
				
				for (size_t i = 0; i < num_lines; ++i)
				{
					const Unicode::String emoticon (Unicode::getTrim (locstr->getStringLine (i)));
					
					if (!emoticon.empty ())
					{
						StringVector sv;
						
						if (isAlphabeticOnly (emoticon))
							findCasingPermutations (emoticon, 0, sv);
						else
						{
							sv.reserve (1);
							sv.push_back (emoticon);
						}
						
						for (StringVector::const_iterator svit = sv.begin (); svit != sv.end (); ++svit)
						{
							const Unicode::String & emoticon_permutation = *svit;
							
							const std::pair<EmoticonSocialMap_t::const_iterator, bool> retval = ms_emoticonSocialMap->insert (std::make_pair (emoticon_permutation, social_type));
							
							if (!retval.second)
							{
								const uint32 old_social = (retval.first)->second;						
								const std::string * const old_social_name = socialsTable->getStringNameByIdSlowly (old_social);
								
								WARNING (true, ("Unable to insert duplicate emoticon '%s' into map.  Already exists for social '%s'",
									Unicode::wideToNarrow (emoticon_permutation).c_str (), old_social_name ? old_social_name->c_str () : "???"));
							}
						}
						
					}
					else
						WARNING (true, ("Invalid empty emoticon for '%s', line %d", socialName.c_str (), i));
					
				}
			}

		}
		else
		{
			WARNING_STRICT_FATAL (true, ("CuiSocialsParser needs CuiSocialsManager::getSocialsTable ()"));
		}
		
		LocalizationManager::getManager ().releaseStringTable (table);
	}
	else
	{
		WARNING_STRICT_FATAL (true, ("No emoticon socials table %s", ms_emoticonSocialTableName));
	}

	s_installed = true;
}

//-----------------------------------------------------------------

void CuiSocialsParser::remove  ()
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));

	delete ms_emoticonSocialMap;
	ms_emoticonSocialMap = 0;

	s_installed = false;
}

//----------------------------------------------------------------------

bool CuiSocialsParser::performParsing (const NetworkId &, const StringVector_t & argv, const String_t & , String_t & result, const CommandParser * node)
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));

	const Unicode::String & social_type_token = argv [0];

	if (isCommand (social_type_token, socialsListCommand))
	{
		Unicode::String abbrev;
		if (argv.size () > 1)
			abbrev = argv [1];

		CuiSocialsManager::listSocials (abbrev, result);
		return true;
	}
	
	const SocialCommandParserNode * const socialNode = NON_NULL (dynamic_cast <const SocialCommandParserNode *> (node));
	const uint32 social_id = socialNode->getSocialId ();

	if (social_id == 0)
	{
		result +=  Unicode::narrowToWide ("Data error: that social was not found in the string file.");
		return true;
	}

	static const Unicode::String empty;

	performSocial (argv.size () > 1 ? argv [1] : empty, social_id, result);
	
	return true;
}

//----------------------------------------------------------------------

bool CuiSocialsParser::preparseEmoticons (const Unicode::String & str, Unicode::String & result)
{
	Unicode::String cmd;
	size_t endpos = 0;

	if (Unicode::getFirstToken (str, 0, endpos, cmd))
	{
		const EmoticonSocialMap_t::const_iterator it = ms_emoticonSocialMap->find (cmd);

		if (it != ms_emoticonSocialMap->end ())
		{
			Unicode::String targetName;

			if (endpos != str.npos)
				IGNORE_RETURN (Unicode::getFirstToken (str, endpos + 1, endpos, targetName));

			CuiSocialsParser::performSocial (targetName, (*it).second, result);
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------

void CuiSocialsParser::performSocialClient (const Object & actor, const Unicode::String & targetName, const std::string & socialName, Unicode::String & result)
{
	const NetworkId targetId = findTarget (actor, targetName, result);
	if (!targetName.empty () && !targetId.isValid ())
		return;

	const uint32 social      = SocialsManager::getSocialTypeByName (socialName);
	const MessageQueueSocial mqs (actor.getNetworkId (), targetId, social, true, true);
	CuiSocialsManager::processMessage (mqs);
}

//----------------------------------------------------------------------

void CuiSocialsParser::performSocial (const Unicode::String & targetName, uint32 social_id, Unicode::String & result)
{
	const Object * const player = Game::getPlayer ();
	if (!player)
		return;

	const NetworkId targetId = findTarget (*player, targetName, result);
	if (!targetName.empty () && !targetId.isValid ())
		return;

	CuiSocialsManager::sendMessage (targetId, social_id);
}

//----------------------------------------------------------------------

NetworkId CuiSocialsParser::findTarget (const Object & actor, const Unicode::String & targetName, Unicode::String & result)
{
	NetworkId targetId;

	CreatureObject * const clientPlayer = NON_NULL(dynamic_cast<CreatureObject *>(Game::getClientPlayer()));

	if (!targetName.empty ())
	{
		//run text through the socials parser to convert "%TT" to "luke", etc.
		Unicode::String resultText;
		CuiStringVariablesData csvd;
		csvd.source = clientPlayer;
		
		if(CuiPreferences::getAutoAimToggle())
			csvd.target = dynamic_cast<ClientObject*>(clientPlayer->getIntendedTarget().getObject());
		else
			csvd.target = dynamic_cast<ClientObject*>(clientPlayer->getLookAtTarget().getObject());

		CuiStringVariablesManager::process(targetName, csvd, resultText);

		const Unicode::String & lowerName = Unicode::toLower (resultText);
		const Object * target = 0;

		static const Unicode::String str_self (Unicode::narrowToWide ("self"));
		static const Unicode::String str_me   (Unicode::narrowToWide ("me"));
		static const Unicode::String str_this (Unicode::narrowToWide ("this"));

		// @todo: localize this
		if (lowerName == str_self || lowerName == str_me || lowerName == str_this)
		{
			target = &actor;
		}
		else
		{
			static const float social_range = 50.0f;

			// @todo: this is no longer valid for objects in portalized cells
			target = ClientWorld::findObjectByLocalizedAbbrev (actor.getPosition_w (), resultText, social_range);
			
			ClientObject const * const clientObject = target ? target->asClientObject () : 0;
			CreatureObject const * const creatureObject = clientObject ? clientObject->asCreatureObject () : 0;			
			if (creatureObject && !creatureObject->getCoverVisibility() && !PlayerObject::isAdmin() && !creatureObject->isPassiveRevealPlayerCharacter(Game::getPlayerNetworkId()))
				target = 0;

			if (target == 0)
			{
				// @todo: localize this
				result +=  Unicode::narrowToWide ("Nothing or no-one by that name could be found in range.");
				return NetworkId::cms_invalid;
			}
		}

		targetId = target->getNetworkId();
	}
	else
	{
		if(CuiPreferences::getAutoAimToggle())
		{	
			ClientObject* obj = dynamic_cast<ClientObject*>(clientPlayer->getIntendedTarget().getObject());
			if(obj)
				targetId = obj->getNetworkId();
		}
		else
		{
			ClientObject* obj = dynamic_cast<ClientObject*>(clientPlayer->getLookAtTarget().getObject());
			if(obj)
				targetId = obj->getNetworkId();
		}
	}

	return targetId;
}

// ======================================================================
