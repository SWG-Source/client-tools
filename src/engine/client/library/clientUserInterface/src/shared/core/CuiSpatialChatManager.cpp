// ======================================================================
//
// CuiSpatialChatManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiSpatialChatManager.h"

#include "LocalizationManager.h"
#include "LocalizedStringTable.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientTextManager.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/MoodManagerClient.h"
#include "clientGame/NetworkScene.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/ProsePackageManagerClient.h"
#include "clientUserInterface/CuiChatBubbleManager.h"
#include "clientUserInterface/CuiChatFormatter.h"
#include "clientUserInterface/CuiChatManager.h"
#include "clientUserInterface/CuiChatParser.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSocialsManager.h"
#include "clientUserInterface/CuiStringIdsChat.h"
#include "clientUserInterface/CuiStringIdsGameLanguage.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedGame/GameLanguageManager.h"
#include "sharedGame/SpatialChatManager.h"
#include "sharedGame/TextManager.h"
#include "sharedMath/PackedRgb.h"
#include "sharedMessageDispatch/Emitter.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedNetworkMessages/MessageQueueSpatialChat.h"
#include "sharedObject/NetworkIdManager.h"
#include "unicodeArchive/UnicodeArchive.h"
#include <cstdio>

// ======================================================================

namespace
{
	namespace StringTable
	{
		const std::string chat_types = "chat_types";
	}

	typedef stdmap<Unicode::String,uint32>::fwd  LocalizedIntMap;
	typedef stdmap<uint32, Unicode::String>::fwd IntLocalizedMap;

	LocalizedIntMap s_localizedChatTypes;
	IntLocalizedMap s_localizedChatVerbs;

	MessageDispatch::Emitter * s_sender;

	bool s_installed = false;
	uint32 s_spatialChatInternalCommandHash = 0;

	struct CaseInsensitiveAbbrevPredicate
	{
		CaseInsensitiveAbbrevPredicate (const Unicode::String & abbrev) :
			m_abbrev (abbrev),
			m_size   (abbrev.size ())
			{}

		Unicode::String m_abbrev;
		size_t          m_size;

		bool operator () (const std::pair<Unicode::String, uint32> & str) const
		{
			return Unicode::caseInsensitiveCompare (str.first, m_abbrev, 0, m_size);
		}

	};
}

//----------------------------------------------------------------------

const char * const          CuiSpatialChatManager::Messages::CHAT_RECEIVED        = "CuiSpatialChatManager::Messages::CHAT_RECEIVED";

//-----------------------------------------------------------------

void CuiSpatialChatManager::install ()
{
	DEBUG_FATAL (s_installed, ("already installed.\n"));

	s_localizedChatTypes.clear ();
	s_localizedChatVerbs.clear ();

	const LocalizedStringTable * const chatTypesTable = LocalizationManager::getManager ().fetchStringTable (StringTable::chat_types);

	WARNING (!chatTypesTable, ("CuiSpatialChatManager needs stf table [%s] to function properly.", StringTable::chat_types.c_str ()));

	if (chatTypesTable)
	{
		const LocalizedStringTable::NameMap_t & nameMap = chatTypesTable->getNameMap ();

		for (LocalizedStringTable::NameMap_t::const_iterator it = nameMap.begin (); it != nameMap.end (); ++it)
		{
			const std::string & name = (*it).first;
			const uint32 stringId    = (*it).second;
			
			//-- special case
			if (name.find("_do_not_translate_") != std::string::npos)
				continue;

			const uint32 chatType = SpatialChatManager::getChatTypeByName (name);

			if (chatType)
			{
				const LocalizedString * const locStr = chatTypesTable->getLocalizedString (stringId);
				NOT_NULL (locStr);

				if (locStr)
				{
					const Unicode::String command = locStr->getStringLine (0);
					const Unicode::String verb    = locStr->getStringLine (1);
					const int numSecondaryCommands = locStr->getNumLines() - 2;

					s_localizedChatTypes.insert (std::make_pair (command, chatType));
					s_localizedChatVerbs.insert (std::make_pair (chatType, verb));
					for (int i = 0; i < numSecondaryCommands; i++)
						s_localizedChatTypes.insert (std::make_pair (locStr->getStringLine(i+2), chatType));
				}
			}
			else
				WARNING_STRICT_FATAL (true, ("CuiSpatialChatManager '%s' specifies invalid chat type '%s", StringTable::chat_types.c_str (), name.c_str ()));
		}

		LocalizationManager::getManager ().releaseStringTable (chatTypesTable);
	}

	CuiChatParser::install ();
	CuiChatFormatter::install ();

	s_sender = new MessageDispatch::Emitter;

	s_spatialChatInternalCommandHash = Crc::normalizeAndCalculate("spatialChatInternal");

	s_installed = true;
}

//-----------------------------------------------------------------

void CuiSpatialChatManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	delete s_sender;
	s_sender = 0;

	s_localizedChatTypes.clear ();
	s_localizedChatVerbs.clear ();

	CuiChatFormatter::remove ();
	CuiChatParser::remove ();

	s_installed = false;
}

//----------------------------------------------------------------------

void CuiSpatialChatManager::processMessage (const MessageQueueSpatialChat & spatialChat, bool chatBoxOk, bool chatBubbleOk)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	UNREF (spatialChat);	

	Unicode::String str;

	ClientObject * const source = safe_cast<ClientObject *>(NetworkIdManager::getObjectById(spatialChat.getSourceId ()));

	if (!CuiPreferences::getShowAFKSpeech() && source)
	{
		CreatureObject * sourceCreatureObject = source->asCreatureObject();
		if (sourceCreatureObject)
		{
			PlayerObject * sourcePlayerObject = sourceCreatureObject->getPlayerObject();
			if (sourcePlayerObject && sourcePlayerObject->isAwayFromKeyBoard())
			{
				return;
			}
		}
	}

	if (!CuiPreferences::getShowNPCSpeech() && source)
	{
		CreatureObject * sourceCreatureObject = source->asCreatureObject();
		if (sourceCreatureObject)
		{
			PlayerObject * sourcePlayerObject = sourceCreatureObject->getPlayerObject();
			if (!sourcePlayerObject)
			{
				return;
			}
		}
		else
		{
			return;
		}
	}

	if ((spatialChat.getLanguage() == 0) || (spatialChat.getChatType() == CuiChatParser::getEmoteChatType()))
	{
		// Common language or emote, automatically no translation

		str = spatialChat.getText();
	}
	else
	{
		PlayerObject *playerObject = Game::getPlayerObject();

		if (playerObject != NULL)
		{
			if (!playerObject->comprehendsLanguage(spatialChat.getLanguage()))
			{
				const int languageId = static_cast<int>(spatialChat.getLanguage());

				if (!GameLanguageManager::isLanguageAudible(languageId))
				{
					if (source != NULL)
					{
						CuiStringVariablesData data;
						data.targetName = source->getLocalizedName();

						Unicode::String lekkuNoComprehendString;
						CuiStringVariablesManager::process(CuiStringIdsGameLanguage::lekku_no_comprehend, data, lekkuNoComprehendString);

						str = lekkuNoComprehendString;

						s_sender->emitMessage (ChatReceivedMsg (Messages::CHAT_RECEIVED, str));

						return;
					}
					else
					{
						DEBUG_WARNING(true, ("Message source is NULL"));
					}
				}
				else
				{
					GameLanguageManager::convertToLanguage(spatialChat.getText(), languageId, str);
				}
			}
			else
			{
				str = spatialChat.getText();
			}
		}
		else
		{
			str = spatialChat.getText();
		}
	}

	Unicode::trim (str);

	if (!spatialChat.getOutOfBand ().empty ())
		ProsePackageManagerClient::appendAllProsePackages (spatialChat.getOutOfBand (), str);

	Unicode::trim (str);

	// Possibly profanity filter the text

	if (Game::isProfanityFiltered())
	{
		str = TextManager::filterText(str);
	}

	if (!source)
	{
		//-- this is not always an error, it would be nice to be able to detect the error case tho
//		DEBUG_WARNING (true, ("Received chat message for non existant object"));
		return;
	}

	ClientObject * const target = safe_cast<ClientObject *>(NetworkIdManager::getObjectById(spatialChat.getTargetId ()));
	Object * const player = Game::getPlayer ();

	if ((spatialChat.getFlags () & MessageQueueSpatialChat::F_isPrivate) != 0)
	{
		if (player && target)
		{
			if (target != player && source != player)
			{
				CuiSocialsManager::processWhisperMessage (spatialChat.getSourceId (), spatialChat.getTargetId (), true);

				if (!spatialChat.getText ().empty ())
				{
					WARNING (true, ("Received a private chat message with private text in it!\n"));
				}

				return;
			}
			else
				CuiSocialsManager::processWhisperMessage (spatialChat.getSourceId (), spatialChat.getTargetId (), false);
		}
	}

	Object * const obj = source;

	Unicode::String displayStr;

	// @todo should mood display in chat bubbles be configurable?

	if (spatialChat.getMoodType ())
	{
		Unicode::String moodName;
		if (MoodManagerClient::getMoodName (spatialChat.getMoodType (), moodName))
		{
			IGNORE_RETURN (displayStr.append (1, '('));
			displayStr += moodName;
			IGNORE_RETURN (displayStr.append (1, ')'));
			IGNORE_RETURN (displayStr.append (1, ' '));

			CreatureObject * const creature = dynamic_cast<CreatureObject *>(source);

			if (creature)
			{
				//-- the mood manager listens for changes to creatures' moods.
				//-- these changes will cause the mood manager to enqueue the appropriate
				//-- transient mood emote, then return to the ambient mood emote

				if (creature->getMood () != spatialChat.getMoodType ())
					creature->clientSetMoodTemporary (spatialChat.getMoodType (), 3.0f);
			}
		}
	}

	if (spatialChat.getChatType () == CuiChatParser::getEmoteChatType ())
	{
		Unicode::String prefix = source->getLocalizedName ();
		IGNORE_RETURN (prefix.append (1, ' '));
		str = prefix + str;
		
		str [0] = static_cast<Unicode::unicode_char_t>(toupper (str [0]));
      const Unicode::unicode_char_t lastChar = str [str.size () - 1];
		if (isalnum (lastChar) || lastChar == ' ')
			str.append (1, '.');
	}
	
	static const uint32 emoteChatType = SpatialChatManager::getChatTypeByName ("emote");
	
	if (chatBubbleOk)
	{
		if (CuiChatBubbleManager::getBubbleEmotesEnabled () || spatialChat.getChatType () != emoteChatType)
		{
			Unicode::String chatBubbleStr;
			
			chatBubbleStr = str;
			
			if (CuiChatBubbleManager::hasFlag (CuiChatBubbleManager::BF_prose))
			{
				if (!chatBubbleStr.empty ())
				{
					chatBubbleStr [0] = static_cast<Unicode::unicode_char_t>(toupper (chatBubbleStr [0]));
               const Unicode::unicode_char_t lastChar = chatBubbleStr [chatBubbleStr.size () - 1];
					if (isalnum (lastChar) || lastChar == ' ')
						chatBubbleStr.append (1, '.');
				}
				chatBubbleStr = displayStr + chatBubbleStr;
			}
			
			CuiChatBubbleManager::enqueueChat (CachedNetworkId (*obj), chatBubbleStr, spatialChat.getChatType (), spatialChat.getMoodType (), spatialChat.getVolume ());
		}
	}
	
	// Colorize the text, since it is already filtered above, we don't need to do it again here

	if (spatialChat.getChatType () == CuiChatParser::getEmoteChatType ())
	{
		// Emote

		str = ClientTextManager::colorAndFilterText(str, ClientTextManager::TT_emote, false);
	}
	else
	{
		// Spatial

		str = ClientTextManager::colorAndFilterText(str, ClientTextManager::TT_spatial, false);
	}

	if (chatBoxOk)
	{
		displayStr += str;
		if (spatialChat.getChatType () == CuiChatParser::getEmoteChatType ())
		{
			s_sender->emitMessage (ChatReceivedMsg (Messages::CHAT_RECEIVED, displayStr));
			return;
		}
		
		const int cs = CuiChatManager::getChatStyle ();
		
		if (spatialChat.getFlags() & MessageQueueSpatialChat::F_ship)
			CuiChatFormatter::formatSpaceComm(spatialChat, str);
		else if (cs == CuiChatManager::CS_Brief)
		{
			//- normal, utilitiarian chat style
			if (GameLanguageManager::isLanguageValid(static_cast<int>(spatialChat.getLanguage())) &&
			    (static_cast<int>(spatialChat.getLanguage()) != GameLanguageManager::getBasicLanguageId()))
			{
				// Prefix the language type on the string

				Unicode::String languageName;
				languageName += ClientTextManager::getColorCode(PackedRgb::solidYellow);
				languageName += Unicode::narrowToWide("[");
				languageName += ClientTextManager::getColorCode(PackedRgb::solidGreen);

				Unicode::String localizedLanguage;
				GameLanguageManager::getLocalizedLanguageName(static_cast<int>(spatialChat.getLanguage()), localizedLanguage);

				languageName += localizedLanguage;
				languageName += ClientTextManager::getColorCode(PackedRgb::solidYellow);
				languageName += Unicode::narrowToWide("]");
				languageName += ' ';

				str.insert(0, languageName);
			}

			CuiChatFormatter::formatRaw (spatialChat, str);
		}
		else if (cs == CuiChatManager::CS_Prose)
		{
			//-- english prose
			CuiChatFormatter::formatProse (spatialChat, str);
		}
		else if (cs == CuiChatManager::CS_Diku)
		{
			//-- Diku-mud style
			CuiChatFormatter::formatDiku (spatialChat, str);
		}
		else
		{
			WARNING (true, ("CuiSpatialChatManager unknown chat style %d", cs));
		}
		
		IGNORE_RETURN (str.append (1, '\n'));
		
		s_sender->emitMessage (ChatReceivedMsg (Messages::CHAT_RECEIVED, str));
	}
}

//-----------------------------------------------------------------

void CuiSpatialChatManager::sendMessage (const NetworkId & targetId, uint32 chatType, uint32 mood, const Unicode::String & str, bool isPrivate, bool skipSource, uint32 const language)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	NetworkScene * const ns      = dynamic_cast<NetworkScene *>(Game::getScene ());

	if (ns == 0)
	{
		REPORT_LOG_PRINT (true, ("Can't chat when not in scene.\n"));
		return;
	}

	// squelched
	if (Game::isPlayerSquelched())
		return;

	CreatureObject * const player = NON_NULL (Game::getPlayerCreature ());

	//-- @todo: emotes based on mood?

	if (mood)
	{
	}

	CachedNetworkId id (*player);

	uint32 chatFlags = 0;

	if (isPrivate)
		chatFlags |= static_cast<size_t>(MessageQueueSpatialChat::F_isPrivate);

	if (skipSource)
		chatFlags |= static_cast<size_t>(MessageQueueSpatialChat::F_skipSource);

	if (mood > 16384 || chatType > 16384)
	{
		WARNING (true, ("mood or chatType out of 16 bit range: %d, %d", mood, chatType));
		return;
	}

	//run text through the socials parser to convert "hi %TT" to "hi luke", etc.
	Unicode::String resultText;
	CuiStringVariablesData csvd;
	csvd.source = player;
	ClientObject* const t = dynamic_cast<ClientObject*>(NetworkIdManager::getObjectById(targetId));
	csvd.target = NULL;

	if(CuiPreferences::getAutoAimToggle())
		csvd.target = dynamic_cast<ClientObject *>(player->getIntendedTarget().getObject());
	else
		csvd.target = dynamic_cast<ClientObject *>(player->getLookAtTarget().getObject());
	
	csvd.other  = t;
	CuiStringVariablesManager::process(str, csvd, resultText);

	Unicode::String actualText = resultText;

	const int MAX_SPATIAL_CHAT_LENGTH = 256;
	if (actualText.size () > MAX_SPATIAL_CHAT_LENGTH)
	{
		CuiSystemMessageManager::sendFakeSystemMessage (CuiStringIdsChat::msg_truncated_spatial.localize ());
		actualText = actualText.substr (0, MAX_SPATIAL_CHAT_LENGTH);
	}

	if (Game::getSinglePlayer ())
	{
		//-- volume will be over-ridden on the server
		const uint16 volume = SpatialChatManager::getVolume (chatType);
		const MessageQueueSpatialChat spatialChat(id, CachedNetworkId(targetId), actualText, volume, static_cast<uint16>(chatType), static_cast<uint16>(mood), chatFlags, language, Unicode::emptyString);
		processMessage(spatialChat);
	}
	else
	{
		// build params to include chatType, volume, mood, flags, str
		char buf[64];

		uint32 languageId = language;
		
		if (languageId == 0)
		{
			// Since the player is not trying to just send a single message in a non-common
			// language, send the message in the language which is their current spoken
			// language.

			PlayerObject *playerObject = Game::getPlayerObject();

			if (playerObject != NULL)
			{
				languageId = static_cast<uint32>(playerObject->getSpokenLanguage());
			}
		}

		//-- volume is no longer used for client->server transmission
		snprintf(buf, sizeof (buf), "%s %d %d %d %d ", targetId.getValueString ().c_str (), chatType, mood, chatFlags, languageId);
		Unicode::String params(Unicode::narrowToWide(buf));
		params += actualText;
		ClientCommandQueue::enqueueCommand(s_spatialChatInternalCommandHash, NetworkId::cms_invalid, params);
	}
}

//-----------------------------------------------------------------

Unicode::String CuiSpatialChatManager::getChatVerbPhrase   (uint32 chatType)
{
	if (chatType == 0)
		return Unicode::String ();

	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	typedef stdmap<uint32, Unicode::String>::fwd IntLocalizedMap;

	const IntLocalizedMap::const_iterator it = s_localizedChatVerbs.find (chatType);

	if (it != s_localizedChatVerbs.end ())
		return (*it).second;
	else
	{
		WARNING (true, ("Request for verb chat phrase for unknown chat type: %d in %d verbs", chatType, s_localizedChatVerbs.size ()));
		return Unicode::narrowToWide ("ERROR");
	}
}

//----------------------------------------------------------------------

uint32 CuiSpatialChatManager::findChatTypeByLocalizedAbbrev (const Unicode::String & abbrev)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	const LocalizedIntMap::const_iterator it = std::find_if (s_localizedChatTypes.begin (), s_localizedChatTypes.end (), CaseInsensitiveAbbrevPredicate (abbrev));

	if (it != s_localizedChatTypes.end ())
		return (*it).second;

	return 0;
}

//----------------------------------------------------------------------

void CuiSpatialChatManager::getLocalizedChatTypeNames     (StringVector & sv)
{
	for (LocalizedIntMap::const_iterator it = s_localizedChatTypes.begin (); it != s_localizedChatTypes.end (); ++it)
		sv.push_back ((*it).first);
}

// ======================================================================
