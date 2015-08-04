//======================================================================
//
// CuiConversationManager.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiConversationManager_H
#define INCLUDED_CuiConversationManager_H

//======================================================================

class CachedNetworkId;
class NetworkId;
class StringId;
class ProsePackage;

//----------------------------------------------------------------------

namespace MessageDispatch
{
	template <typename MessageType, typename IdentifierType> class Transceiver;
}

//----------------------------------------------------------------------

#define END_CHAT_IN_DIALOG 1
#define END_CHAT_WITH_RESPONSE 0

//----------------------------------------------------------------------

class CuiConversationManager
{
public:

	struct Messages
	{
		static const char * const RESPONSES_CHANGED;
		static const char * const TARGET_CHANGED;
		
		struct ConsoleOutput
		{
			typedef Unicode::String Payload;
		};

		struct ResponsesChanged
		{
			typedef bool Payload;
		};

		struct TargetChanged
		{
			typedef bool Payload;
		};

		struct ConversationEnded
		{
			typedef bool Payload;
		};
	};

	struct TransceiverTypes
	{
		typedef MessageDispatch::Transceiver<const Messages::ConsoleOutput::Payload &,    Messages::ConsoleOutput>    ConsoleOutput;
		typedef MessageDispatch::Transceiver<const Messages::ResponsesChanged::Payload &, Messages::ResponsesChanged> ResponsesChanged;
		typedef MessageDispatch::Transceiver<const Messages::TargetChanged::Payload &,    Messages::TargetChanged>    TargetChanged;
		typedef MessageDispatch::Transceiver<const Messages::ConversationEnded::Payload &, Messages::ConversationEnded> ConversationEnded;
	};

	static void                              install ();
	static void                              remove ();

	static bool                              respond (const NetworkId & target, int response);

	typedef stdvector<Unicode::String>::fwd  StringVector;

	static void                              getResponses (StringVector & result);
	static const StringVector &              getResponses ();

	static void                              setResponses (const StringVector & responses, bool useDefaultResponse = true);
	static void                              issueMessage (const Unicode::String & msg);

	static const CachedNetworkId &           getTarget ();
	static void                              setTarget (NetworkId const & id, uint32 appearanceOverrideSharedTemplateCrc, std::string const & soundEffect);
	static uint32                            getAppearanceOverrideTemplateCrc ();
	static void                              onServerStopConversing(NetworkId const & id, StringId const & finalText, Unicode::String const & finalProse, Unicode::String const & finalResponse);

	static void                              start (const NetworkId & id);
	static bool                              stop ();

	static const Unicode::String &           getLastMessage ();

	static void startClientOnlyConversation(NetworkId const & target, StringId const & message, Unicode::String const & messageoob, Unicode::String const & response);
	static void stopClientOnlyConversation();
	static bool getUseDefaultResponse();
	static bool isClientOnlyMode();
	static const std::string & getSoundEffect();
	static float getClientOnlyWindowCloseTime();

private:

	static void                              makeSinglePlayerResponses ();
	class CuiConversationManagerAction;

	static CuiConversationManagerAction      ms_action;
	static StringVector                      ms_responses;
	static Unicode::String                   ms_lastMessage;
	static CachedNetworkId                   ms_targetId;
	static uint32                            ms_appearanceOverrideSharedTemplateCrc;
};

//----------------------------------------------------------------------

inline const CuiConversationManager::StringVector &                   CuiConversationManager::getResponses ()
{
	return ms_responses;
}

//----------------------------------------------------------------------

inline const Unicode::String & CuiConversationManager::getLastMessage ()
{
	return ms_lastMessage;
}

//======================================================================

#endif


