// ======================================================================
//
// CuiTextManager.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiTextManager_H
#define INCLUDED_CuiTextManager_H

// ======================================================================

#include "UITypes.h"
#include "UIString.h"
#include "sharedMath/Vector.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"

class UICanvas;
class UIBaseObject;
class UIImageStyle;
class UITextStyle;
class Vector;

//-----------------------------------------------------------------
/**
* The CuiTextManager provides an interface for drawing text in screenspace.
*
* Rendered text may have a depth value, in which case overlapping text gets
* rendered in a predictable order.
*
* Rendered text may also be displayed in a chat bubble if requested.
*
*/

//----------------------------------------------------------------------

//-- top-level class since c++ nested classes fwd decl is broken

struct CuiTextManagerTextEnqueueInfo
{
	enum TextWeight
	{
		TW_normal,
		TW_heavy,
		TW_starwars,
		TW_count
	};

	Vector         screenVect;
	real           worldDistance;
	real           opacity;
	real           backgroundOpacity;
	UIColor        textColor;
	TextWeight     textWeight;
	float          textSize;
	UIImageStyle * imageStyle;
	UIImageStyle * imageStyle2;
	NetworkId      id;
	bool           updateOffset;
	
	CuiTextManagerTextEnqueueInfo () :
		screenVect (),
		worldDistance (0.0f),
		opacity           (1.0f),
		backgroundOpacity (1.0f),
		textColor         (0, 0, 0),
		textWeight        (TW_normal),
		textSize          (1.0f),
		imageStyle        (0),
		imageStyle2       (0),
		id                (),
		updateOffset      (true)
		{}
};

//----------------------------------------------------------------------

//-- top-level class since c++ nested classes fwd decl is broken

struct CuiTextManagerTextChatInfo
{
	uint32  chatType;
	uint32  moodType;
	bool    isChatBubble;
	
	CuiTextManagerTextChatInfo () : 
	chatType (0),
		moodType (0),
		isChatBubble (false)
	{}
	
	CuiTextManagerTextChatInfo (uint32 theChatType, uint32 theMoodType) :
	chatType (theChatType),
		moodType (theMoodType),
		isChatBubble (true)
	{}
};

//----------------------------------------------------------------------

class CuiTextManager
{
public:

	typedef CuiTextManagerTextEnqueueInfo TextEnqueueInfo;
	typedef CuiTextManagerTextChatInfo    TextChatInfo;

public:
	static void                 resetQueue ();

	static void                 enqueueText (const Unicode::String & str, const TextEnqueueInfo & textEnqueueInfo);
	static void                 enqueueChatBubble (const Unicode::String & str, const TextChatInfo & textChatInfo, const TextEnqueueInfo & textEnqueueInfo);

	static void                 render (UICanvas & canvas);

	static void                 install (const UIBaseObject & rootPage);
	static void                 remove ();

	static UITextStyle *        getTextStyle (long height, const TextEnqueueInfo & info);
	static UITextStyle *        getTextStyle (long height);

	static void                 showSystemStatusString (const Unicode::String & str);

private:

	                            CuiTextManager ();
	                            CuiTextManager (const CuiTextManager & rhs);
	CuiTextManager &            operator=       (const CuiTextManager & rhs);

public:
	struct TextInfo
	{
		Unicode::String m_str;
		UIPoint         m_pt;
		TextEnqueueInfo m_info;
		TextChatInfo    m_chatInfo;

		TextInfo (const Unicode::String & str, real normalized, const TextChatInfo & textChatInfo, const TextEnqueueInfo & textEnqueueInfo) :
			m_str  (str),
			m_pt   (static_cast<long>(textEnqueueInfo.screenVect.x), static_cast<long>(textEnqueueInfo.screenVect.y)),
			m_info (textEnqueueInfo),
			m_chatInfo (textChatInfo)
		{
			//-- the world distance is normalized for enqueued text
			m_info.worldDistance = normalized;
		}
		
	private:
		TextInfo ();
	};

private:
	static void           renderPlainText (UICanvas & canvas, const TextInfo & ti, UITextStyle & style, const UISize & maxBubbleSize);
	static void           renderBubble    (UICanvas & canvas, const TextInfo & ti, UITextStyle & style, const UISize & maxBubbleSize);


private:
	static bool           ms_installed;
};


// ======================================================================

#endif
