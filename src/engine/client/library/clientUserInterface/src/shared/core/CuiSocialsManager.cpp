// ======================================================================
//
// CuiSocialsManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiSocialsManager.h"

#include "LocalizationManager.h"
#include "LocalizedStringTable.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureController.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientUserInterface/CuiAnimationManager.h"
#include "clientUserInterface/CuiChatBubbleManager.h"
#include "clientUserInterface/CuiDragInfo.h"
#include "clientUserInterface/CuiIconManager.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "clientUserInterface/CuiSocials.def"
#include "clientUserInterface/CuiSocialsParser.h"
#include "clientUserInterface/CuiStringGrammarManager.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/Crc.h"
#include "sharedGame/SocialsManager.h"
#include "sharedGame/SpatialChatManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueSocial.h"
#include "sharedObject/CachedNetworkId.h"

// ======================================================================

namespace
{

	bool                        s_installed;
	
	namespace StringTable
	{
		const std::string socials         = "socials";
	}

	namespace Transceivers
	{
		MessageDispatch::Transceiver<const CuiSocialsManager::Messages::SocialReceived::Payload &, CuiSocialsManager::Messages::SocialReceived > 
			socialReceived;
	}

	uint32 s_socialInternalCommandHash = 0;
	uint32 s_socialChatType            = 0;

	bool s_testSocialsData = false;
}

//-----------------------------------------------------------------

LocalizedStringTable * CuiSocialsManager::ms_socialsTable;
uint32                 CuiSocialsManager::ms_whisperSocialId;

//-----------------------------------------------------------------

void  CuiSocialsManager::install ()
{
	DEBUG_FATAL (s_installed, ("already installed\n"));
	
	DebugFlags::registerFlag(s_testSocialsData, "ClientUserInterface", "testSocialsData");

	s_socialChatType            = SpatialChatManager::getChatTypeByName ("social");

	ms_socialsTable             = LocalizationManager::getManager ().fetchStringTable (StringTable::socials);
	
	if (ms_socialsTable)
	{
		//-----------------------------------------------------------------
		//-- sanity-check the file
		
		const LocalizedStringTable::Map_t & theMap = ms_socialsTable->getMap ();
		for (LocalizedStringTable::Map_t::const_iterator it = theMap.begin (); it != theMap.end (); ++it)
		{
			size_t const id = (*it).first;
			
			std::string const * const name = ms_socialsTable->getStringNameByIdSlowly(id);
			
			if (NULL == name)
				DEBUG_FATAL(true, ("unexpected, name is invalid for id [%d].", id));
			else
			{
				const uint32 type = SocialsManager::getSocialTypeByName (*name);
				
				WARNING (type == 0, ("CuiSocialsManager:: Entry [%s] is not a valid social type.", name->c_str ()));
				
				const LocalizedString * locstr = (*it).second;
				if (locstr->getNumLines () < static_cast<int>(CuiSocials::LMF_Null))
				{
					
					WARNING (true, ("CuiSocialsManager:: Entry [%s] is malformed.  Not enough lines.  Has %d, needs %d.", name->c_str (),
						locstr->getNumLines (),
						static_cast<int>(CuiSocials::LMF_Null)));
				}
				
				//- do some testing for proper icons & localization
#if _DEBUG
				if (s_testSocialsData)
				{
					Unicode::String locTmpStr;
					CuiDragInfo cdi;
					cdi.type = CuiDragInfoTypes::CDIT_command;

					CuiSkillManager::localizeCmdName        (*name, locTmpStr);
					CuiSkillManager::localizeCmdDescription (*name, locTmpStr);
					
					cdi.str.clear ();
					cdi.str.push_back ('/');
					cdi.str += *name;
					
					CuiIconManager::findIconImageStyle (cdi);
				}
#endif				
			}
		}
			//----------------------------------------------------------------------
		//-- keep track of special socials, e.g. whisper
		
		ms_whisperSocialId = SocialsManager::getSocialTypeByName ("whisper");
	}
	else
		WARNING_STRICT_FATAL (true, ("Unable to locate socials table: %s", StringTable::socials.c_str ()));

	//-----------------------------------------------------------------

	CuiSocialsParser::install ();

	s_socialInternalCommandHash = Crc::normalizeAndCalculate("socialInternal");

	s_installed = true;
}

//-----------------------------------------------------------------

void  CuiSocialsManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));

	CuiSocialsParser::remove ();

	if (ms_socialsTable)
	{
		LocalizationManager::getManager ().releaseStringTable (ms_socialsTable);
		ms_socialsTable = 0;
	}

	s_installed = false;
}

//-----------------------------------------------------------------

void  CuiSocialsManager::processMessage (const MessageQueueSocial & socialMsg)
{
	const bool printMessage  = socialMsg.isTextOk ();
	const bool playAnimation = socialMsg.isAnimationOk ();

	Unicode::String str (Unicode::narrowToWide ("empty social"));

	const CachedNetworkId sourceId (socialMsg.getSourceId ());

	ClientObject * const source = dynamic_cast<ClientObject *>(sourceId.getObject ());

	if (!source)
	{
		WARNING (true, ("Received social message for non-existant source object %s", sourceId.getValueString ().c_str ()));
		return;
	}

	if (!ms_socialsTable)
	{
		WARNING (true, ("Cannot process message qith no socials table."));
		return;
	}

	const CachedNetworkId targetId (socialMsg.getTargetId ());
	ClientObject * const target =  dynamic_cast<ClientObject *>(targetId.getObject ());
	
	const uint32 socialType = socialMsg.getSocialType ();
	std::string socialName;

	if (!SocialsManager::getSocialNameByType (socialType, socialName))
	{
		WARNING (true, ("Received invalid social type %d in CuiSocialsManager::processMessage"));
		return;
	}

	const LocalizedString * const social_locstr = ms_socialsTable->getLocalizedString (socialName);
	
	if (!social_locstr)
	{
		str = Unicode::narrowToWide ("UNABLE TO LOAD SOCIAL !!");
		Transceivers::socialReceived.emitMessage (str);
	}
	else
	{
		if (printMessage)
		{
			Unicode::String rawSocialMessage;
			
			if (!CuiStringGrammarManager::getRawMessage (*social_locstr, source,  target, rawSocialMessage))
				str = Unicode::narrowToWide ("AN ERROR OCCURED PROCESSING THE SOCIAL TEXT!");
			else
			{
				CuiStringVariablesData data;
				data.source = source;
				data.target = target;
				CuiStringVariablesManager::process (rawSocialMessage, data, str);
			}

			Transceivers::socialReceived.emitMessage (str);

			const Object * const player = Game::getPlayer ();

			if (source != player && CuiChatBubbleManager::hasFlag (CuiChatBubbleManager::BF_enabledSocials))
			{
				CuiChatBubbleManager::enqueueChat (sourceId, str, s_socialChatType, 0, 32);
			}
		}
		
		bool animationWasPlayed = false;

		CreatureObject * const sourceAsCreature = dynamic_cast<CreatureObject *>(sourceId.getObject ());
		const bool sourceRidingMount = sourceAsCreature && sourceAsCreature->isRidingMount();

		if (playAnimation && !sourceRidingMount)
		{
			//-----------------------------------------------------------------
			//-- try to load and play the animations
			
			Unicode::String w_animationString;
			
			if (socialMsg.getTargetId () == socialMsg.getSourceId ())
				w_animationString = social_locstr->getStringLine (static_cast<size_t>(CuiSocials::LMF_Animation_Target_Self));		
			else if (socialMsg.getTargetId () != NetworkId::cms_invalid)
				w_animationString = social_locstr->getStringLine (static_cast<size_t>(CuiSocials::LMF_Animation_Target_Other));
			else
				w_animationString = social_locstr->getStringLine (static_cast<size_t>(CuiSocials::LMF_Animation_Target_None));
			
			if (!w_animationString.empty () && w_animationString [0] != '~')
			{
				const Unicode::NarrowString animationString = Unicode::wideToNarrow (w_animationString);
				size_t endpos = 0;
				Unicode::NarrowString animationName;
				
				if (!Unicode::getFirstToken (animationString, 0, endpos, animationName))
					WARNING (true, ("Bad animation line for social %s. No first token.", socialName.c_str ()));
				else
				{
					int animationCount = 1;
					Unicode::NarrowString animationCountString;
					if (endpos != Unicode::NarrowString::npos && Unicode::getFirstToken (animationString, endpos + 1, endpos, animationCountString))
					{
						animationCount = atoi (animationCountString.c_str ());
					}
					
					CuiAnimationManager::attemptPlayEmote (*source, target, animationName.c_str ());
					animationWasPlayed = true;
				}
			}
		}

		//-- have the source face the target if the source velocity is zero and if the target is non-null.
		if (!animationWasPlayed && target)
		{
			CreatureController * const creatureController = dynamic_cast<CreatureController*> (source->getController ());
			if (creatureController)
				creatureController->face (target->getPosition_w ());
		}
	}
}

//-----------------------------------------------------------------

void  CuiSocialsManager::sendMessage (const NetworkId & targetId, uint32 socialType, bool animationOk, bool textOk)
{
	const Object * const player = Game::getPlayer ();

	if (!player)
		return;

	const NetworkId & id = player->getNetworkId();

	sendMessage (id, targetId, socialType, animationOk, textOk);
} 

//----------------------------------------------------------------------

void CuiSocialsManager::sendMessage (const NetworkId & sourceId, const NetworkId & targetId, uint32 socialType, bool animationOk, bool textOk)
{
	if (Game::getSinglePlayer ())
	{
		const MessageQueueSocial msg (CachedNetworkId (sourceId), CachedNetworkId (targetId), socialType, animationOk, textOk);
		processMessage (msg);
	}
	else
	{
		const Object * const player = Game::getPlayer ();
		
		if (!player)
			return;
		
		if (sourceId != player->getNetworkId ())
		{
			WARNING (true, ("Trying to send non-player social in network mode"));
			return;
		}

		// a permanently squelched player cannot do any emote
		if (Game::isPlayerPermanentlySquelched())
			return;

		// build params to include chatType, volume, mood, flags, str
		char buf[64];

		//-- volume is no longer used for client->server transmission
		snprintf (buf, sizeof (buf), "%s %d %d %d", targetId.getValueString ().c_str (), socialType, animationOk ? 1 : 0, textOk ? 1 : 0);
		Unicode::String params(Unicode::narrowToWide(buf));
		ClientCommandQueue::enqueueCommand(s_socialInternalCommandHash, NetworkId::cms_invalid, params);
	}
}

//----------------------------------------------------------------------
/**
 * whisper socials should only be called when there is both a source and a target, and neither is the player
 *
*/

void CuiSocialsManager::processWhisperMessage (const NetworkId &sourceId, const NetworkId & targetId, bool printMessage)
{
	const MessageQueueSocial msg (CachedNetworkId (sourceId), CachedNetworkId (targetId), ms_whisperSocialId, true, printMessage);
	CuiSocialsManager::processMessage (msg);
}

//----------------------------------------------------------------------

uint32 CuiSocialsManager::getSocialIdForCommandName (const std::string & name)
{
	if (!ms_socialsTable)
		return 0;

	const std::string & lowerName = Unicode::toLower(name);
	
	LocalizedString::id_type nameIdResult = ms_socialsTable->getIdByName(lowerName);
	if (nameIdResult)
	{
		const std::string * lookupName = ms_socialsTable->getStringNameByIdSlowly (nameIdResult);
		if (lookupName)
			return SocialsManager::getSocialTypeByName(*lookupName);
	}
	return 0;
}

//----------------------------------------------------------------------

void CuiSocialsManager::listSocials (const Unicode::String & abbrev, Unicode::String & result)
{	
	ClientObject * const player = Game::getClientPlayer ();

	if (!ms_socialsTable)
		return;

	//-- push social list into info map to alphabetize it
	typedef stdmap<Unicode::String, std::string>::fwd SocialInfoMap;
	SocialInfoMap socialInfoMap;

	{
		const LocalizedStringTable::NameMap_t & nameMap = ms_socialsTable->getNameMap ();
		for ( LocalizedStringTable::NameMap_t::const_iterator it = nameMap.begin (); it != nameMap.end (); ++it)
		{
			const std::string & canonicalName = (*it).first;
			const size_t        stringTableId = (*it).second;

			const uint32 socialType = SocialsManager::getSocialTypeByName (canonicalName);

			if (!socialType)
				continue;

			if (socialType == CuiSocialsManager::getWhisperSocialId ())
				continue;
			
			const LocalizedString * const locStr = ms_socialsTable->getLocalizedString (stringTableId);
			if (!locStr)
				return;

			const Unicode::String & str = locStr->getStringLine (0);
			
			const Unicode::String & w_animationName = locStr->getStringLine (static_cast<size_t>(CuiSocials::LMF_Animation_Target_None));
			
			if (!abbrev.empty ())
			{
				if (abbrev [0] == '*')
				{
					if (w_animationName.empty () || w_animationName [0] == '~')
						continue;
				}
				else if (!CommandParser::isAbbrev (abbrev, str))
					continue;
			}
			
			socialInfoMap.insert (std::make_pair (str, Unicode::wideToNarrow (w_animationName)));
		}
	}

	// @todo: this message should be localized
	result += Unicode::narrowToWide ("\\#ffffff");
	result += Unicode::narrowToWide ("Available socials:");
	const Unicode::String color (Unicode::narrowToWide ("\\#ffccff"));
	
	static const int NUM_COLS = 4;
	int col = 0;

	static const Unicode::String colTabstops [NUM_COLS] = 
	{
		Unicode::String (),
		Unicode::narrowToWide ("\\@2"),
		Unicode::narrowToWide ("\\@4"),
		Unicode::narrowToWide ("\\@6")
	};

	for (SocialInfoMap::const_iterator it = socialInfoMap.begin (); it != socialInfoMap.end (); ++it)
	{
		const Unicode::String & str      = (*it).first;
		const std::string & animation    = (*it).second;

		if (col >= NUM_COLS)
			col = 0;

		if (col == 0)
			result.push_back ('\n');
		
		result.append (colTabstops [col]);

		if (animation.empty () || animation [0] == '~')
			result.append (8, ' ');
		else
		{
			result.append (4, ' ');
			if (player && !CuiAnimationManager::isValidEmote (*player, 0, animation))
			{
				WARNING (true, ("Unable to find animation [%s] for social [%s]", animation.c_str (), Unicode::wideToNarrow (str).c_str ()));
				result.append (1, '-');
			}
			else
				result.append (1, '*');

			result.append (3, ' ');
		}
		
		result += color + str;

		++col;
	}

	result += Unicode::narrowToWide ("\\#ffffff");
	result.push_back ('\n');
}

//----------------------------------------------------------------------

void CuiSocialsManager::testSocials (const NetworkId & target, Unicode::String & result)
{
	ClientObject * const player = Game::getClientPlayer ();

	if (!player)
	{
		result += Unicode::narrowToWide ("Unable to test socials without a player.");
		return;
	}

	if (!ms_socialsTable)
		return;

	const LocalizedStringTable::Map_t & tableMap = ms_socialsTable->getMap ();
	for ( LocalizedStringTable::Map_t::const_iterator it = tableMap.begin (); it != tableMap.end (); ++it)
	{
		const uint32 id = (*it).second->getId ();
		if (id == CuiSocialsManager::getWhisperSocialId ())
			continue;
		
		sendMessage (player->getNetworkId (), NetworkId::cms_invalid, id);
		sendMessage (player->getNetworkId (), player->getNetworkId (), id);

		if (target != NetworkId::cms_invalid)
			sendMessage (player->getNetworkId (), NetworkId::cms_invalid, id);
	}
}

// ======================================================================
