//======================================================================
//
// CuiChatBubbleManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiChatBubbleManager_H
#define INCLUDED_CuiChatBubbleManager_H

//======================================================================

class Camera;
class CachedNetworkId;
class UICanvas;
class UIBaseObject;
class CreatureObject;
class NetworkId;

//-----------------------------------------------------------------

#define BINARY_BIT(a) (1 << a)

class CuiChatBubbleManager
{
public:

	enum BubbleFlags
	{
		BF_enabled         = BINARY_BIT (0x00),
		BF_enabledEmotes   = BINARY_BIT (0x01),
		BF_enabledSocials  = BINARY_BIT (0x02),
		BF_minimized       = BINARY_BIT (0x03),
		BF_showSelf        = BINARY_BIT (0x04),
		BF_showAll         = BINARY_BIT (0x05),
		BF_showGuild       = BINARY_BIT (0x06),
		BF_showGroup       = BINARY_BIT (0x07),
		BF_showNPC         = BINARY_BIT (0x08),
		BF_showPlayer      = BINARY_BIT (0x09),
		BF_prose           = BINARY_BIT (0x10)
	};

	class BubbleStack;

	static void update (float elapsedTime);
	static void render (UICanvas & canvas, const Camera & camera);

	static void enqueueChat                 (const CachedNetworkId & id, const Unicode::String & str, uint32 chatType, uint32 mood, uint16 volume);
	static bool objectHasChatBubblesVisible (const NetworkId & id);

	static void install (const UIBaseObject & rootPage);
	static void remove ();

	static void clearAll ();
	static void clearBubblesFor (const NetworkId & id);

	static void setBubblesMinimized (bool b);
	static bool getBubblesMinimized ();

	static void setBubblesEnabled             (bool b);
	static bool getBubblesEnabled             ();

	static void setBubbleEmotesEnabled        (bool b);
	static bool getBubbleEmotesEnabled        ();
	static bool getBubbleEmotesEnabledDefault ();

	static void setBubbleSocialsEnabled       (bool b);
	static bool getBubbleSocialsEnabled       (); 
	static bool getBubbleSocialsEnabledDefault(); 

	static void setBubbleProse                (bool b);
	static bool getBubbleProse                (); 
	static bool getBubbleProseDefault         (); 

	static void setBubbleShowSelf             (bool b);
	static bool getBubbleShowSelf             (); 
	static bool getBubbleShowSelfDefault      (); 

	static void setBubbleShowAll              (bool b);
	static bool getBubbleShowAll              (); 
	static bool getBubbleShowAllDefault       (); 

	static void setBubbleShowGuild            (bool b);
	static bool getBubbleShowGuild            (); 
	static bool getBubbleShowGuildDefault     (); 

	static void setBubbleShowGroup            (bool b);
	static bool getBubbleShowGroup            (); 
	static bool getBubbleShowGroupDefault     (); 

	static void setBubbleShowNPC              (bool b);
	static bool getBubbleShowNPC              (); 
	static bool getBubbleShowNPCDefault       (); 

	static void setBubbleShowPlayer           (bool b);
	static bool getBubbleShowPlayer           (); 
	static bool getBubbleShowPlayerDefault    (); 

	static void  setBubblesDurationScale (float f);
	static float getBubblesDurationScale ();

	static void  setBubblesDurationMin (float f);
	static float getBubblesDurationMin ();

	static void  setBubblesDurationMax (float f);
	static float getBubblesDurationMax ();

	static void   setChatBubbleRangeFactor     (float f);
	static float  getChatBubbleRangeFactor     ();

	static void   setBubbleFontSize        (float f);
	static float  getBubbleFontSize        ();

	static bool   hasFlag (BubbleFlags flag);
	static void   setFlag (BubbleFlags flag, bool b);

	static bool   chatBubbleOk (const CreatureObject * player, const CachedNetworkId & other);

private:

	static bool           ms_bubblesMinimized;
	static bool           ms_bubbleEmotesEnabled;
	static float          ms_bubblesDurationScale;
	static float          ms_bubblesDurationMin;
	static float          ms_bubblesDurationMax;
	static float          ms_bubbleRangeFactor;
	static float          ms_bubbleFontSize;

	static int            ms_flagsDefault;
	static int            ms_flags;
};

//----------------------------------------------------------------------

inline bool CuiChatBubbleManager::hasFlag (BubbleFlags flag)
{
	return (ms_flags & static_cast<int>(flag)) != 0;
}

//-----------------------------------------------------------------

inline bool   CuiChatBubbleManager::getBubblesMinimized ()
{
	return CuiChatBubbleManager::hasFlag (BF_minimized);
}

//----------------------------------------------------------------------

inline bool CuiChatBubbleManager::getBubblesEnabled ()
{
	return hasFlag (BF_enabled);
}

//----------------------------------------------------------------------

inline float CuiChatBubbleManager::getBubblesDurationScale ()
{
	return ms_bubblesDurationScale;
}

//----------------------------------------------------------------------

inline float CuiChatBubbleManager::getBubblesDurationMin ()
{
	return ms_bubblesDurationMin;
}

//----------------------------------------------------------------------

inline float CuiChatBubbleManager::getBubblesDurationMax ()
{
	return ms_bubblesDurationMax;
}

//----------------------------------------------------------------------

inline bool CuiChatBubbleManager::getBubbleEmotesEnabled ()
{
	return hasFlag (BF_enabledEmotes);
}

//======================================================================

#endif
