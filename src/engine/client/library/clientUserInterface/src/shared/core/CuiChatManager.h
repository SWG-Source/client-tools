// ======================================================================
//
// CuiChatManager.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiChatManager_H
#define INCLUDED_CuiChatManager_H

// ======================================================================

struct ChatAvatarId;
class StringId;
class Callback;

//----------------------------------------------------------------------

class CuiChatManager
{
public:
	enum ChatStyle
	{
		CS_Brief,
		CS_Diku,
		CS_Prose
	};

	static void                          install               ();
	static void                          remove                ();

	static int                           getChatStyle          ();
	static void                          setChatStyle          (ChatStyle cs);

	static bool                          getChatBoxKeyClick     ();
	static void                          setChatBoxKeyClick     (bool b);

	static bool                          getChatBoxTimestamp    ();
	static void                          setChatBoxTimestamp    (bool b);

	static int                           getChatWindowFontSizeDefaultIndex ();
	static int                           getChatWindowFontSizeDefaultSize  ();
	static void                          setChatWindowFontSizeDefaultIndex (int index);

	static bool                          isChatBrief                       ();
	static bool                          isChatDiku                        ();
	static bool                          isChatProse                       ();

	static bool                          isChatBriefDefault                ();
	static bool                          isChatDikuDefault                 ();
	static bool                          isChatProseDefault                ();

	static void                          setChatBrief                      (bool b);
	static void                          setChatDiku                       (bool b);
	static void                          setChatProse                      (bool b);

	static void                          getShortName            (const ChatAvatarId & avatarId, Unicode::String & str);
	static const Unicode::String         getShortName            (const ChatAvatarId & avatarId);

	static const ChatAvatarId            getSelfAvatarId         ();
	static const bool                    constructChatAvatarId   (const std::string & name, ChatAvatarId & avatarId);

	static const Unicode::String         prosify                 (const ChatAvatarId & actor, const ChatAvatarId & target, const ChatAvatarId & other, const StringId & stringId);
	static const Unicode::String         prosify                 (const ChatAvatarId & actor, const ChatAvatarId & target, const StringId & stringId);
	static const Unicode::String         prosify                 (const ChatAvatarId & actor, const StringId & stringId);
	static const Unicode::String         prosify                 (const ChatAvatarId & actor, const Unicode::String & otherTxt, const StringId & stringId);

	static Callback &                    getChatFontSizeCallback    ();
	static Callback &                    getChatBoxKeyClickCallback ();

private:

	static int         ms_chatStyle;
	static int         ms_chatWindowFontSizeDefaultIndex;
	static bool        ms_chatBoxKeyClick;
};

//----------------------------------------------------------------------

inline int CuiChatManager::getChatStyle          ()
{
	return ms_chatStyle;
}

// ======================================================================

#endif
