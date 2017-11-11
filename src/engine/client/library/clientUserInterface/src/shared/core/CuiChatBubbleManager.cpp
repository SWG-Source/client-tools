//======================================================================
//
// CuiChatBubbleManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiChatBubbleManager_BubbleStack.h"

#include "LocalizationManager.h"
#include "LocalizedStringTable.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIManager.h"
#include "UIPage.h"
#include "UIRectangleStyle.h"
#include "UIText.h"
#include "UITextStyle.h"
#include "UIWidgetRectangleStyles.h"
#include "UnicodeUtils.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroupObject.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiObjectTextManager.h"
#include "clientUserInterface/CuiTextManager.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/SpatialChatManager.h"
#include "sharedMath/Segment3d.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/CellProperty.h"
#include "sharedUtility/LocalMachineOptionManager.h"

//----------------------------------------------------------------------


const char * const KeyName = "ChatBubbles";
#define REGISTER_OPTION(a) (LocalMachineOptionManager::registerOption(ms_ ## a, KeyName, #a))

//lint -e655 //(Warning -- bit-wise operation uses (compatible) enum's)
//lint -esym(641, CollisionFlags) // convert enum to int
//lint -esym(534, UIBaseObject::Detach) // ignore return
//lint -esym(534, multimap<float,_STL::pair<const BubbleStack *,Vector>,_STL::less<float>,_STL::allocator<_STL::pair<const float,_STL::pair<const BubbleStack *,Vector>>>>::insert) // ignore return


//======================================================================

namespace
{
	namespace StringTable
	{
		const std::string chat_bubble_icon_types = "chat_bubble_icon_types";
	}

	typedef std::map<CachedNetworkId, CuiChatBubbleManager::BubbleStack> BubbleStackMap;
	BubbleStackMap s_bubbleStackMap;

	typedef std::multimap<float, std::pair <const CuiChatBubbleManager::BubbleStack *, Vector> > BubbleStackDepthMap;
	BubbleStackDepthMap s_bubbleStackDepthMap;

	namespace Properties
	{
		const UILowerString margin = UILowerString ("margin");
	}

	//-----------------------------------------------------------------

		//----------------------------------------------------------------------
	//-- these strings are ordered:
	//--
	//-- 0. name
	//-- 1. chat bubble outer icon style name (if null, use default)
	//-- 2. chat spout style name             (if null, use default)
	//-- 3. chat stack spout style name       (if null, use default)
	//-- 4. chat bubble rectangle style       (if null, use default)
	//--

	const char * const s_chatGraphicsFolder = "/styles.chatBubbles";

	const char * const s_chatStyleArray [][5] =
	{
		{ "default",     "Icon.outer.default",     "spout.default",     "stackspout.default",     "rect.default.rs" },
		{ "exclaim",     "Icon.outer.exclaim",     "spout.exclaim",     "stackspout.exclaim",     "rect.exclaim.rs" },
		{ "thought",     "Icon.outer.thought",     "spout.thought",     "stackspout.thought",     "rect.thought.rs" },
		{ "chatter",     0,                        "spout.default",     "stackspout.default",     "rect.chatter.rs" },
		{ "double_ring", 0,                        "spout.default",     "stackspout.default",     "rect.double_ring.rs" },
		{ "droid",       0,                        "spout.default",     "stackspout.default",     "rect.droid.rs" },
		{ "icy",         0,                        "spout.default",     "stackspout.default",     "rect.icy.rs" },
		{ "moan",        0,                        "spout.default",     "stackspout.default",     "rect.moan.rs" },
		{ "sarcastic",   0,                        "spout.default",     "stackspout.default",     "rect.sarcastic.rs" },
		{ "scream",      0,                        "spout.default",     "stackspout.default",     "rect.scream.rs" },
		{ "shout",       0,                        "spout.shout",       "stackspout.shout",       "rect.shout.rs" },
		{ "stutter",     0,                        "spout.default",     "stackspout.default",     "rect.stutter.rs" },
		{ "threaten",    0,                        "spout.default",     "stackspout.default",     "rect.threaten.rs" },
		{ "whisper",     0,                        "spout.default",     "stackspout.default",     "rect.whisper.rs" },
		{ "social",      0,                        "spout.social",      "stackspout.social",      "rect.double_ring.rs" },
		{ "evil",        0,                        "spout.default",     "stackspout.default",     "rect.evil.rs" },
		{ "evil2",       0,                        "spout.default",     "stackspout.default",     "rect.evil2.rs" },
		{ "poetry",      0,                        "spout.default",     "stackspout.default",     "rect.poetry.rs" },
		{ "sad",         0,                        "spout.default",     "stackspout.default",     "rect.sad.rs" },
		{ "sing2",       0,                        "spout.default",     "stackspout.default",     "rect.sing2.rs" },
		{ "sing3",       0,                        "spout.default",     "stackspout.default",     "rect.sing3.rs" }

	};

	const size_t s_numChatStyles  = sizeof (s_chatStyleArray) / sizeof (s_chatStyleArray[0]);

	uint32 s_droidChatType = 0;
	const UILowerString s_indentionProperty = UILowerString ("indention");

	inline int findChatStyleIndex (const char * name)
	{
		for (size_t i = 0; i < s_numChatStyles; ++i)
		{
			if (!_stricmp (s_chatStyleArray [i][0], name))
				return static_cast<int>(i);
		}

		return -1;
	}
	
	//----------------------------------------------------------------------
	//-- structure for grouping together the ui elements and data needed to render the chat bubbles
	//--

	struct ChatBubbleInfo
	{
		struct Icon
		{
			UIPage *   page;
			UIImage *  outer;
			UIImage *  ring;
			UIImage *  inner;
		};

		Icon icon;

		struct Bubble
		{
			UIText *  text;
			UIImage * spout;
		};

		Bubble bubble;

	};

	ChatBubbleInfo s_info = { { 0, 0, 0, 0 }, { 0, 0 } };

	//----------------------------------------------------------------------

	UIImageStyle *                        s_bubbleIconOuterDefaultStyle;
	UIImageStyle *                        s_spoutDefaultStyle;
	UIImageStyle *                        s_stackSpoutDefaultStyle;
	UIRectangleStyle *                    s_rectangleDefaultStyle;

	//----------------------------------------------------------------------
	//-- the map of chatTypes to pointers of UIImageStyles
	//--

	struct ChatStyles
	{
		UIImageStyle     * iconOuter;
		UIImageStyle     * spout;
		UIImageStyle     * stackSpout;
		UIRectangleStyle * rectangle;

		ChatStyles (UIImageStyle * theIconOuter, UIImageStyle * theSpout, UIImageStyle * theStackSpout, UIRectangleStyle * theRectangle) :
			iconOuter (theIconOuter), spout (theSpout), stackSpout (theStackSpout), rectangle (theRectangle) {}
	private:
		ChatStyles ();
	};

	typedef std::map<uint32, ChatStyles> ChatStyleMap;
	ChatStyleMap * s_chatStyleMap;

	//-----------------------------------------------------------------

	float s_backgroundOpacity = 0.7f;

	UISize s_screenSize;
	UISize s_maxBubbleSize;

	//-----------------------------------------------------------------
	
	void autoSizeText (UIText & text, UITextStyle & style, const UIString & str)
	{
		//-- unlimit the max lines so the desired scroll sizes can be computed properly
		text.SetMaxLines (-1);

		UISize maxStrSize;
		
		style.MeasureText (str, maxStrSize);
		const float f_divisor = sqrt ((static_cast<float>(maxStrSize.x) / static_cast<float>(maxStrSize.y)) * 0.5f);
		const long  i_divisor = static_cast<long>(f_divisor);
		
		if (i_divisor > 1L)
			maxStrSize.x /= i_divisor;
		
		text.SetHeight (0L);
		text.SetStyle (&style);
		
		text.SetLocalText (str);
		
		const UIRect & textmargin = text.GetMargin ();

		maxStrSize.x += textmargin.left + textmargin.right + style.GetLeading ();
		maxStrSize.x = std::min (s_maxBubbleSize.x, maxStrSize.x);
		
		s_info.bubble.text->SetWidth (maxStrSize.x);
		
		UISize newSize;
		text.GetScrollExtent (newSize);
		newSize.y = std::min (s_maxBubbleSize.y, newSize.y);
		
		text.SetSize (newSize);

		const int numLines = (text.GetHeight () - textmargin.top - textmargin.bottom) / style.GetLeading ();
		text.SetMaxLines (numLines);
	}

	//-----------------------------------------------------------------


	void initBubbles ()
	{
		s_info.bubble.text->SetBackgroundOpacity (s_backgroundOpacity);
		s_info.bubble.text->SetTextColor         (UIColor::black);
	}

	//-----------------------------------------------------------------
	
	void renderBubble (UICanvas & canvas, UITextStyle & style, const CuiChatBubbleManager::BubbleStack::Element & element, const UIPoint & targetPt, bool stacked, UIRect & lastExtent)
	{
		if (element.m_lastFontSize != style.GetLeading ())
		{
		}
		
		long spoutIndention = 0L;

		//----------------------------------------------------------------------
		//-----------------------------------------------------------------
		//-- setup bubble styles and colors
		
		UIRect textmargin;
		UIRectangleStyle * rectangleStyle  = NON_NULL (s_rectangleDefaultStyle);
		UIImageStyle *     outerIconStyle  = NON_NULL (s_bubbleIconOuterDefaultStyle);
		UIImageStyle *     spoutStyle      = stacked ? NON_NULL (s_stackSpoutDefaultStyle) : NON_NULL (s_spoutDefaultStyle);
		
		{
			if (element.m_chatType)
			{
				const ChatStyleMap::const_iterator style_it = s_chatStyleMap->find (element.m_chatType);
				if (style_it != s_chatStyleMap->end ())
				{
					const ChatStyles & cgs = (*style_it).second;
					outerIconStyle = NON_NULL (cgs.iconOuter);
					spoutStyle     = stacked ? NON_NULL (cgs.stackSpout) : NON_NULL (cgs.spout);
					rectangleStyle = NON_NULL (cgs.rectangle);
				}
			}
			
			s_info.icon.outer->SetStyle   (outerIconStyle);
			IGNORE_RETURN (spoutStyle->GetPropertyLong (s_indentionProperty, spoutIndention));
			s_info.bubble.spout->SetStyle (spoutStyle);
			s_info.bubble.spout->SetSize  (spoutStyle->GetSize ());
			s_info.bubble.spout->SetOpacity (s_backgroundOpacity);
			
			s_info.bubble.text->GetWidgetRectangleStyles ()->SetRectangleStyle (UIWidgetRectangleStyles::RS_Default, rectangleStyle);
			
			if (!rectangleStyle->GetPropertyRect (Properties::margin, textmargin))
			{
				const UIImageStyle * const north = rectangleStyle->GetImage (UIRectangleStyle::I_North);
				if (north)
					textmargin.top = textmargin.bottom = north->GetHeight ();
				
				const UIImageStyle * const east  = rectangleStyle->GetImage (UIRectangleStyle::I_East);
				if (east)
					textmargin.left = textmargin.right = east->GetWidth ();
			}

			s_info.bubble.text->SetMargin (textmargin);
		}
		
		UISize bubbleSize (0L, 0L);
		bubbleSize.y += spoutStyle->GetHeight ();	
		bubbleSize.y -= spoutIndention;

		//----------------------------------------------------------------------
		//-- calculate bubble size
		if (!CuiChatBubbleManager::getBubblesMinimized ())
		{
			autoSizeText (*s_info.bubble.text, style, element.m_text);
			bubbleSize += s_info.bubble.text->GetSize ();
		}
		else
		{
			bubbleSize += s_info.icon.page->GetSize ();
		}
		
		UIPoint actualTextPoint (targetPt);

		actualTextPoint.x -= bubbleSize.x / 2;
		actualTextPoint.y -= bubbleSize.y;

		if (actualTextPoint.x + bubbleSize.x >= s_screenSize.x)
			actualTextPoint.x = s_screenSize.x - bubbleSize.x;
		if (actualTextPoint.x < 0)
			actualTextPoint.x = 0;

		if (!stacked)
		{
			if (actualTextPoint.y + bubbleSize.y >= s_screenSize.y)
				actualTextPoint.y = s_screenSize.y - bubbleSize.y;
			if (actualTextPoint.y < 0)
				actualTextPoint.y = 0;
		}
		
		//-----------------------------------------------------------------

		//-----------------------------------------------------------------
		
		canvas.PushState ();
		{
			const long bubbleOrIconHeight = CuiChatBubbleManager::getBubblesMinimized () ? s_info.icon.page->GetHeight () : s_info.bubble.text->GetHeight ();

			//-- draw the spout
			if (actualTextPoint.y + bubbleOrIconHeight <= targetPt.y)
			{				
				const long spaceUnderBubble = (targetPt.y - actualTextPoint.y) + (spoutIndention - bubbleOrIconHeight);
				if (spaceUnderBubble < s_info.bubble.spout->GetHeight ())
					s_info.bubble.spout->SetHeight (spaceUnderBubble + spoutIndention);
				
				UIPoint translate (targetPt);

				const long spoutHalfWidth = s_info.bubble.spout->GetWidth () / 2;
				if (stacked)
				{
					//-- center the stacking spout horizontally on the new bubble, or the last bubble
					if (bubbleSize.x < lastExtent.Width ())
						translate.x = actualTextPoint.x + bubbleSize.x / 2;
					else
						translate.x = (lastExtent.left + lastExtent.right) / 2;

					translate.x = std::max (lastExtent.left + spoutHalfWidth, std::min (translate.x, lastExtent.right - spoutHalfWidth));
				}

				translate.y -= s_info.bubble.spout->GetHeight ();
				translate.x -= spoutHalfWidth;
				
				UIFloatPoint shear;

				if (translate.x + s_info.bubble.spout->GetWidth () >= s_screenSize.x)
				{
					translate.x = s_screenSize.x - s_info.bubble.spout->GetWidth ();

					//-- if this is not a stacked bubble, shear or angle the spout to the correct location
					if (!stacked)
					{
						const long push = std::min (s_info.bubble.spout->GetHeight (), bubbleSize.x / 2L - s_info.bubble.spout->GetWidth ());
						translate.x -= push;
						shear.x = static_cast<float>(push) / s_info.bubble.spout->GetHeight ();
					}
				}

				if (translate.x < 0L)
				{
					translate.x = 0L;

					//-- if this is not a stacked bubble, shear or angle the spout to the correct location
					if (!stacked)
					{
						const long push = std::min (s_info.bubble.spout->GetHeight (), bubbleSize.x / 2L - s_info.bubble.spout->GetWidth ());
						translate.x += push;
						shear.x = - static_cast<float>(push) / s_info.bubble.spout->GetHeight ();
					}
				}
				
				canvas.Translate        (translate);
				canvas.Shear            (shear);

				canvas.ModifyOpacity    (s_info.bubble.spout->GetOpacity ());
				canvas.ModifyColor      (s_info.bubble.spout->GetColor ());
				s_info.bubble.spout->Render (canvas);
			}
			
			canvas.RestoreState ();
			
			canvas.Translate        (actualTextPoint);
			
			//-- only draw the icon when minimized
			if (!CuiChatBubbleManager::getBubblesMinimized ())
			{
				canvas.SetClip (UIRect (s_info.bubble.text->GetSize ()));
				s_info.bubble.text->Render (canvas);			
			}
			else
			{
				canvas.Translate (UISize (0L, bubbleOrIconHeight / 2L));
//				canvas.Translate (UISize (spoutStyle->GetWidth (), 0L) / 2L);
				s_info.icon.page->Render (canvas);
			}

			//-----------------------------------------------------------------		
		}
		canvas.PopState ();
			
		UNREF (stacked);

		lastExtent.Set (actualTextPoint, bubbleSize);
	}


	//----------------------------------------------------------------------

	bool ms_installed = false;
}

//-----------------------------------------------------------------

float CuiChatBubbleManager::ms_bubblesDurationScale;
float CuiChatBubbleManager::ms_bubblesDurationMin;
float CuiChatBubbleManager::ms_bubblesDurationMax;
float CuiChatBubbleManager::ms_bubbleRangeFactor;
float CuiChatBubbleManager::ms_bubbleFontSize;
int   CuiChatBubbleManager::ms_flags;
int   CuiChatBubbleManager::ms_flagsDefault = 0;

//-----------------------------------------------------------------

void CuiChatBubbleManager::install (const UIBaseObject & rootPage)
{
	DEBUG_FATAL (ms_installed, ("already installed.\n"));

	s_droidChatType = SpatialChatManager::getChatTypeByName ("jabber");

	bool const useJapanese = UIManager::gUIManager().isLocaleJapanese();
	if(useJapanese)
		ms_flagsDefault = BF_showAll | BF_showSelf | BF_enabledSocials | BF_enabledEmotes;
	else
		ms_flagsDefault = BF_showAll | BF_showSelf | BF_enabledSocials | BF_enabledEmotes;

	ms_flags = ms_flagsDefault;

	setFlag (BF_minimized,     ConfigClientUserInterface::getChatBubblesMinimized    ());
	setFlag (BF_enabled,       ConfigClientUserInterface::getChatBubblesEnabled      ());
	setFlag (BF_enabledEmotes, ConfigClientUserInterface::getChatBubbleEmotesEnabled ());

	ms_bubblesDurationScale  = ConfigClientUserInterface::getChatBubbleDurationScale ();
	ms_bubblesDurationMin    = ConfigClientUserInterface::getChatBubbleDurationMin ();
	ms_bubblesDurationMax    = ConfigClientUserInterface::getChatBubbleDurationMax ();

	ms_bubbleFontSize        = ConfigClientUserInterface::getChatBubbleFontSize    ();
	ms_bubbleRangeFactor     = ConfigClientUserInterface::getChatBubbleRangeFactor ();

	REGISTER_OPTION (flags);
	REGISTER_OPTION (bubblesDurationScale);
	REGISTER_OPTION (bubbleRangeFactor);
	REGISTER_OPTION (bubbleFontSize);

	s_info.icon.page = NON_NULL (safe_cast<UIPage *>(rootPage.GetObjectFromPath ("/MsgBox.ChatBubbleIcon", TUIPage)));
	s_info.icon.page->SetLocation (0, 0);
	s_info.icon.page->Attach (0);

	s_info.icon.outer = NON_NULL (safe_cast<UIImage *>(s_info.icon.page->GetObjectFromPath ("Outer", TUIImage)));
	s_info.icon.ring  = NON_NULL (safe_cast<UIImage *>(s_info.icon.page->GetObjectFromPath ("Ring",  TUIImage)));
	s_info.icon.inner = NON_NULL (safe_cast<UIImage *>(s_info.icon.page->GetObjectFromPath ("Inner", TUIImage)));

	s_info.bubble.text = NON_NULL (safe_cast<UIText *>(rootPage.GetObjectFromPath ("/MsgBox.ChatBubbleText", TUIText)));
	s_info.bubble.text->SetPreLocalized (true);
	// The Japanese client will default to Left-aligned text in text bubbles, where the US defaults to centered
	if (useJapanese)
		s_info.bubble.text->SetTextAlignment(UITextStyle::Left);
	s_info.bubble.text->Attach (0);

	s_info.bubble.spout = NON_NULL (safe_cast<UIImage *>(rootPage.GetObjectFromPath ("/MsgBox.ChatSpout", TUIImage)));
	s_info.bubble.spout->SetLocation (0, 0);

	//-----------------------------------------------------------------
	//-- load up the chat bubble icon style info

	UIImageStyle       * chatImageStyles [s_numChatStyles][4] = { {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}};
	UIRectangleStyle   * chatRectangleStyles      [s_numChatStyles]    = { 0 };

	UIBaseObject * const chatGraphicsFolder = NON_NULL (rootPage.GetObjectFromPath (s_chatGraphicsFolder));

	if (chatGraphicsFolder)
	{
		for (size_t i = 0; i < s_numChatStyles; ++i)
		{
			//-- setup the outer ring style pointer
			if (!s_chatStyleArray [i][1] ||
				0 == (chatImageStyles [i][0] = NON_NULL (safe_cast<UIImageStyle *>(chatGraphicsFolder->GetObjectFromPath (s_chatStyleArray [i][1], TUIImageStyle)))))
				chatImageStyles [i][0] = chatImageStyles [0][0];

			//-- setup the spout style pointer
			if (!s_chatStyleArray [i][2] ||
				0 == (chatImageStyles [i][1] = NON_NULL (safe_cast<UIImageStyle *>(chatGraphicsFolder->GetObjectFromPath (s_chatStyleArray [i][2], TUIImageStyle)))))
				chatImageStyles [i][1] = chatImageStyles [0][1];

			//-- setup the stack spout style pointer
			if (!s_chatStyleArray [i][3] ||
				0 == (chatImageStyles [i][2] = NON_NULL (safe_cast<UIImageStyle *>(chatGraphicsFolder->GetObjectFromPath (s_chatStyleArray [i][3], TUIImageStyle)))))
				chatImageStyles [i][2] = chatImageStyles [0][2];

			//-- setup the rectangle style pointer
			if (!s_chatStyleArray [i][4] ||
				0 == (chatRectangleStyles [i] = NON_NULL (safe_cast<UIRectangleStyle *>(chatGraphicsFolder->GetObjectFromPath (s_chatStyleArray [i][4], TUIRectangleStyle)))))
				chatRectangleStyles [i] = chatRectangleStyles [0];
		}
	}

	s_bubbleIconOuterDefaultStyle = NON_NULL (chatImageStyles [0][0]);
	s_spoutDefaultStyle           = NON_NULL (chatImageStyles [0][1]);
	s_stackSpoutDefaultStyle      = NON_NULL (chatImageStyles [0][2]);
	s_rectangleDefaultStyle       = NON_NULL (chatRectangleStyles [0]);	
	
	//-----------------------------------------------------------------
	
	s_chatStyleMap = new ChatStyleMap;
	
	const LocalizedStringTable * const table = LocalizationManager::getManager ().fetchStringTable (StringTable::chat_bubble_icon_types);
	
	if (table)
	{
		const LocalizedStringTable::NameMap_t & nameMap = table->getNameMap ();
		
		for (LocalizedStringTable::NameMap_t::const_iterator it = nameMap.begin (); it != nameMap.end (); ++it)
		{
			const std::string indexName = (*it).first;
			// Special case warning translators to not translate this file
			if (indexName.find("_do_not_translate_") != std::string::npos)
				continue;

			const int bubbleStyleIndex = findChatStyleIndex (indexName.c_str ());
			
			if (bubbleStyleIndex < 0)
			{
				DEBUG_WARNING (true, ("chat_bubble_icon_types.stf contains an unknown bubble style, '%s'", indexName.c_str ()));
				continue;
			}
			
			const LocalizedString * const locstr = table->getLocalizedString ((*it).second);
			NOT_NULL (locstr);
			
			const size_t num_lines = locstr->getNumLines ();
			
			for (size_t i = 0; i < num_lines; ++i)
			{
				const Unicode::NarrowString line (Unicode::wideToNarrow (locstr->getStringLine (i)));
				
				if (line.empty ())
					continue;
				
				const uint32 chatType = SpatialChatManager::getChatTypeByName (line);
				if (chatType == 0)
				{
					DEBUG_WARNING (true, ("chat_bubble_icon_types.stf, bubble style '%s', refers to unknown chat type '%s'", (*it).first.c_str (), line.c_str ()));
					continue;
				}
				
				const ChatStyles cgs (chatImageStyles [bubbleStyleIndex][0], chatImageStyles [bubbleStyleIndex][1], chatImageStyles [bubbleStyleIndex][2], chatRectangleStyles [bubbleStyleIndex]);
				
				const std::pair<ChatStyleMap::iterator, bool> retval = s_chatStyleMap->insert (std::make_pair (chatType, cgs));
				if (!retval.second)
				{
					DEBUG_WARNING (true, ("chat_bubble_icon_types.stf, bubble style '%s', attempted to insert duplicate chat type '%s'",  (*it).first.c_str (), line.c_str ()));
					continue;
				}
			}
		}
		
		LocalizationManager::getManager ().releaseStringTable (table); 
	}
	else
		WARNING_STRICT_FATAL (true, ("No chat bubble icon table %s", StringTable::chat_bubble_icon_types.c_str ()));
	
	ms_installed = true;
}

//-----------------------------------------------------------------

void CuiChatBubbleManager::remove ()
{
	DEBUG_FATAL (!ms_installed, ("not installed.\n"));

	delete s_chatStyleMap;
	s_chatStyleMap = 0;

	s_info.bubble.text->Detach (0);
	s_info.bubble.text = 0;

	s_info.icon.page->Detach (0);
	s_info.icon.page  = 0;
	s_info.icon.outer = 0;
	s_info.icon.ring  = 0;
	s_info.icon.inner = 0;

	ms_installed = false;
}

//-----------------------------------------------------------------

void CuiChatBubbleManager::update (float elapsedTime)
{
	DEBUG_FATAL (!ms_installed, ("not installed.\n"));

	for (BubbleStackMap::iterator it = s_bubbleStackMap.begin (); it != s_bubbleStackMap.end (); )
	{
		const CachedNetworkId & id = (*it).first;
		BubbleStack & bubbleStack  = (*it).second;

		if (!id.getObject () || !bubbleStack.update (elapsedTime))
		{
			s_bubbleStackMap.erase (it++);
			continue;
		}

		++it;
	}
}

//-----------------------------------------------------------------

namespace
{

	Vector s_screenVector;
	Vector s_screenCenter;
	float  s_screenRadius;

	void calculateScreenVector (const Camera & camera, const Vector & headPoint, long offset, Vector & screenVect)
	{
		if (!camera.projectInCameraSpace (headPoint, &screenVect.x, &screenVect.y, &screenVect.z, true))
		{
			screenVect = headPoint;

			float len = sqrt (sqr (screenVect.x) + sqr (screenVect.y));

			if (len < 0.0001)
			{
				len = 1.0f;
				screenVect.x = 0.0f;
				screenVect.y = 1.0f;
			}

			len = RECIP (len) * s_screenRadius;
			screenVect.x *= len;
			screenVect.y *= len;
			screenVect.y -= static_cast<float>(offset);

			//-- right edge
			if (screenVect.x > s_screenCenter.x)
			{
				screenVect.y = s_screenCenter.x * screenVect.y / screenVect.x;
				screenVect.x = s_screenCenter.x;
			}
			//-- left edge
			else if (screenVect.x < -s_screenCenter.x)
			{
				screenVect.y = -s_screenCenter.x * screenVect.y / screenVect.x;
				screenVect.x = -s_screenCenter.x;
			}
			//-- top edge
			else if (screenVect.y > s_screenCenter.y)
			{
				screenVect.x = s_screenCenter.y * screenVect.x / screenVect.y;
				screenVect.y = s_screenCenter.y;
			}
			//-- bottom edge
			else if (screenVect.y < -s_screenCenter.y)
			{
				screenVect.x = -s_screenCenter.y * screenVect.x / screenVect.y;
				screenVect.y = -s_screenCenter.y;
			}
			
			screenVect.x += s_screenCenter.x;
			screenVect.y = s_screenCenter.y - screenVect.y;
		}
		else
			screenVect.y += static_cast<float>(offset);
	}
}

//-----------------------------------------------------------------

void CuiChatBubbleManager::render (UICanvas & canvas, const Camera & camera)
{
	DEBUG_FATAL (!ms_installed, ("not installed.\n"));

	if (!hasFlag (BF_enabled))
		return;

	if (s_bubbleStackMap.empty ())
		return;

	s_screenSize.x    = Graphics::getCurrentRenderTargetWidth ();
	s_screenSize.y    = Graphics::getCurrentRenderTargetHeight ();
	s_maxBubbleSize.x = s_screenSize.x / 2;
	s_maxBubbleSize.y = s_screenSize.y / 4;
	s_screenVector.x  = static_cast<float>(s_screenSize.x);
	s_screenVector.y  = static_cast<float>(s_screenSize.y);
	s_screenCenter    = s_screenVector * 0.5f;
	s_screenRadius    = sqrt (sqr(s_screenCenter.x) + sqr (s_screenCenter.y));

	initBubbles ();
	
	//----------------------------------------------------------------------
	//-- sort the stack map
	
	s_bubbleStackDepthMap.clear ();
	
	const CreatureObject * const player = Game::getPlayerCreature ();

	{
		const CellProperty * const selfCell      = camera.getParentCell ();
		const Vector &             selfPos       = camera.getPosition_w ();

		for (BubbleStackMap::iterator it = s_bubbleStackMap.begin (); it != s_bubbleStackMap.end (); ++it)
		{
			CachedNetworkId const & id      = (*it).first;
			BubbleStack & bubbleStack = (*it).second;
			
			if (!chatBubbleOk (player, id))
				continue;

			const Object * const obj = id.getObject ();
			
			if (!obj)
				continue;

			if (obj == Game::getPlayer ())
			{
				if (Game::isViewFirstPerson ())
					continue;
			}			

			bubbleStack.setId(id);

			float timeFactor = 1.0f;
			if (CuiObjectTextManager::canSee (selfPos, selfCell, *obj, timeFactor))
			{
				Vector screenVect;

				// in the same portalized object	
				const int offset = CuiObjectTextManager::getObjectHeadPointOffset (obj->getNetworkId (), 0);
				
				const Vector & headPoint_o = CuiObjectTextManager::getCurrentObjectHeadPoint_o (*obj);
				const Vector & headPoint_w = obj->rotateTranslate_o2w   (headPoint_o);

				UIPoint parentHeadPt;
				if (CuiObjectTextManager::getObjectHeadPoint     (*obj, camera, 0.0f, parentHeadPt))
				{
					screenVect.x = static_cast<float>(parentHeadPt.x);
					screenVect.y = static_cast<float>(parentHeadPt.y + offset);
				}
				else
				{
					const Vector & headPoint_c = camera.rotateTranslate_w2o (headPoint_w);
				
					int lastOffset = 0;
					calculateScreenVector (camera, headPoint_c, lastOffset, screenVect);
				}


				float worldDistance     = headPoint_o.magnitude ();				
				worldDistance += worldDistance * (1.0f - timeFactor);

				if (worldDistance > bubbleStack.getMaxVolume ())
					continue;

				worldDistance *= ms_bubbleRangeFactor;

				if (worldDistance > bubbleStack.getMaxVolume () * ms_bubbleRangeFactor)
					continue;

				const Vector & frameK     = camera.getObjectFrameK_w ();
				Vector vectTarget         = headPoint_w - camera.getPosition_w ();
				vectTarget.approximateNormalize ();

				//-- scale worlddistance based onthe dot product of the view vector and the vector to the object
				//-- scale by 1.0f if looking directly at it, 4.0f if looking perp to it, 6.0f if looking directly away
				const float dotProduct = frameK.dot (vectTarget);
				worldDistance *= 1.0f + (1.0f - dotProduct) * 2.0f;

				if (worldDistance > bubbleStack.getMaxVolume () * ms_bubbleRangeFactor)
					continue;

				s_bubbleStackDepthMap.insert (std::make_pair(worldDistance, std::make_pair (&bubbleStack, screenVect)));
			}
		}
	}
	
	const float chatBubbleFontSize = CuiChatBubbleManager::getBubbleFontSize ();

	//----------------------------------------------------------------------
	
	for (BubbleStackDepthMap::reverse_iterator it = s_bubbleStackDepthMap.rbegin (); it != s_bubbleStackDepthMap.rend (); ++it)
	{
		const float worldDistance       = (*it).first;
		const BubbleStack & bubbleStack = *NON_NULL ((*it).second.first);
		const Vector & screenVect       = (*it).second.second;
		
		bool stacked = false;
		
		const BubbleStack::ElementList & elementList = bubbleStack.getElements ();

		UIScalar nameOffset = CuiObjectTextManager::getNameWidgetHeight(bubbleStack.getId());

		UIPoint screenPt(static_cast<long>(screenVect.x), static_cast<long>(screenVect.y) - nameOffset);
		
		const float oldOpacity = canvas.GetOpacity ();
		
		UIRect lastExtent;
		
		for (BubbleStack::ElementList::const_iterator bit = elementList.begin (); bit != elementList.end (); ++bit)
		{
			const BubbleStack::Element & elem = *bit;
			
			const float modifiedVolume = elem.m_volume * ms_bubbleRangeFactor;
			if (worldDistance > modifiedVolume)
				continue;
			
			const float CHATBUBBLE_DISTANCE_LIMIT_SOFT           = static_cast<float>(modifiedVolume) * 0.75f;
			const float CHATBUBBLE_DISTANCE_LIMIT_SOFT_RECIP     = RECIP (CHATBUBBLE_DISTANCE_LIMIT_SOFT);
			const float CHATBUBBLE_DISTANCE_LIMIT_HARD           = static_cast<float>(modifiedVolume);
			const float CHATBUBBLE_DISTANCE_LIMIT_RANGE          = CHATBUBBLE_DISTANCE_LIMIT_HARD - CHATBUBBLE_DISTANCE_LIMIT_SOFT;
			const float CHATBUBBLE_DISTANCE_LIMIT_RANGE_RECIP    = RECIP (CHATBUBBLE_DISTANCE_LIMIT_RANGE);
			
			const float textScale = std::min (1.0f, std::max (0.0f, - (worldDistance - CHATBUBBLE_DISTANCE_LIMIT_SOFT) * CHATBUBBLE_DISTANCE_LIMIT_SOFT_RECIP));
			// this sets text styles between sizes 19 and 13

			UITextStyle * const style = CuiTextManager::getTextStyle (static_cast<int>(textScale * chatBubbleFontSize * 4.5f + 12.0f));
			
			if (!style)
				continue;
			
			const float opacity = (&elem == &elementList.back ()) ? bubbleStack.getTopBubbleOpacity () : 1.0f; 			
			
			float distanceOpacity = oldOpacity;
			
			if (worldDistance > CHATBUBBLE_DISTANCE_LIMIT_SOFT)
				distanceOpacity *= (CHATBUBBLE_DISTANCE_LIMIT_RANGE - (worldDistance - CHATBUBBLE_DISTANCE_LIMIT_SOFT)) * CHATBUBBLE_DISTANCE_LIMIT_RANGE_RECIP;			

			canvas.SetOpacity (opacity * distanceOpacity);
			
			renderBubble (canvas, *style, elem, screenPt, stacked, lastExtent);

			//-- only draw one icon when minimized
			if (CuiChatBubbleManager::getBubblesMinimized ())
				break;

			screenPt.y = lastExtent.top;
			stacked = true;
			
			if (screenPt.y < 0)
				break;
		}
		
		canvas.SetOpacity (oldOpacity);
	}
}

//-----------------------------------------------------------------

void CuiChatBubbleManager::enqueueChat (const CachedNetworkId & id, const Unicode::String & str, uint32 chatType, uint32 mood, uint16 volume)
{
	if (!chatBubbleOk (0, id))
		return;

	DEBUG_FATAL (!ms_installed, ("not installed.\n"));

	if (chatType == 0)
	{
		const ClientObject * const source = safe_cast<const ClientObject *>(id.getObject ());
		if (source && 
			(source->getGameObjectType () == SharedObjectTemplate::GOT_creature_droid ||
			source->getGameObjectType () == SharedObjectTemplate::GOT_creature_droid_probe))
		{
			chatType = s_droidChatType;
		}
	}

	BubbleStack & bubbleStack = s_bubbleStackMap [id];
	bubbleStack.addElement (BubbleStack::Element (str, chatType, mood, volume));
}

//----------------------------------------------------------------------

void CuiChatBubbleManager::clearAll ()
{
	s_bubbleStackMap.clear ();
}

//----------------------------------------------------------------------

void CuiChatBubbleManager::clearBubblesFor (const NetworkId & id)
{
	const CachedNetworkId cid (id);
	s_bubbleStackMap.erase (cid);
}

//-----------------------------------------------------------------

void CuiChatBubbleManager::setBubblesMinimized (bool b)
{
	setFlag (BF_minimized, b);
}

//----------------------------------------------------------------------

void CuiChatBubbleManager::setBubblesEnabled (bool b)
{
	setFlag (BF_enabled, b);
}

//----------------------------------------------------------------------

void  CuiChatBubbleManager::setBubblesDurationScale (float f)
{
	ms_bubblesDurationScale = f;
}

//----------------------------------------------------------------------

void  CuiChatBubbleManager::setBubblesDurationMin (float f)
{
	ms_bubblesDurationMin = f;
}

//----------------------------------------------------------------------

void  CuiChatBubbleManager::setBubblesDurationMax (float f)
{
	ms_bubblesDurationMax = f;
}

//----------------------------------------------------------------------

void CuiChatBubbleManager::setBubbleEmotesEnabled (bool b)
{
	setFlag (BF_enabledEmotes, b);
}

//----------------------------------------------------------------------

float CuiChatBubbleManager::getChatBubbleRangeFactor ()
{
	return RECIP (ms_bubbleRangeFactor);
}

//----------------------------------------------------------------------

void CuiChatBubbleManager::setChatBubbleRangeFactor   (float f)
{
	if (f > 0.0f)
		ms_bubbleRangeFactor = RECIP (f);
}

//----------------------------------------------------------------------

void   CuiChatBubbleManager::setBubbleFontSize        (float f)
{
	ms_bubbleFontSize = f;
}

//----------------------------------------------------------------------

float  CuiChatBubbleManager::getBubbleFontSize        ()
{
	return ms_bubbleFontSize;
}

//----------------------------------------------------------------------

void CuiChatBubbleManager::setFlag (BubbleFlags flag, bool b)
{
	if (b)
		ms_flags |= static_cast<int>(flag);
	else
		ms_flags &= ~static_cast<int>(flag);
}

//----------------------------------------------------------------------

bool CuiChatBubbleManager::chatBubbleOk (const CreatureObject * player, const CachedNetworkId & other)
{
	if (!player)
	{
		player = Game::getPlayerCreature ();
		if (!player)
			return false;
	}

	Object * const object = other.getObject ();
	ClientObject * const clientObject = object ? object->asClientObject () : 0;

	if (clientObject && clientObject->asShipObject())
		return false;

	if (player->getNetworkId () == other)
		return hasFlag (BF_showSelf);

	if (hasFlag (BF_showAll))
		return true;

	if (!clientObject)
		return false;

	TangibleObject * const tangible = clientObject->asTangibleObject ();
	
	if (tangible)
	{
		if (tangible->isPlayer ())
		{
			if (hasFlag (BF_showPlayer))
				return true;
		}
		else
		{
			if (hasFlag (BF_showNPC))
				return true;
		}
	}

	if (hasFlag (BF_showGroup))
	{
		GroupObject * const group = safe_cast<GroupObject *>(player->getGroup ().getObject ());
		if (group)
		{
			bool leader = false;
			if (group->findMember (other, leader))
				return true;
		}
	}

	if (hasFlag (BF_showGuild))
	{
		const int playerGuild = player->getGuildId ();

		if (playerGuild)
		{
			CreatureObject * const creature = clientObject->asCreatureObject ();

			if (creature && creature->getGuildId () == playerGuild)
				return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------

void CuiChatBubbleManager::setBubbleSocialsEnabled (bool b)
{
	setFlag (BF_enabledSocials, b);
}

//----------------------------------------------------------------------

bool CuiChatBubbleManager::getBubbleSocialsEnabled ()
{
	return hasFlag (BF_enabledSocials);
}

//----------------------------------------------------------------------

bool CuiChatBubbleManager::getBubbleSocialsEnabledDefault ()
{
	return (ms_flagsDefault & static_cast<int>(BF_enabledSocials)) != 0;
}

//----------------------------------------------------------------------

void CuiChatBubbleManager::setBubbleProse          (bool b)
{
	setFlag (BF_prose, b);
}

//----------------------------------------------------------------------

bool CuiChatBubbleManager::getBubbleProse          () 
{
	return hasFlag (BF_prose);
}

//----------------------------------------------------------------------

bool CuiChatBubbleManager::getBubbleProseDefault          () 
{
	return (ms_flagsDefault & static_cast<int>(BF_prose)) != 0;
}

//----------------------------------------------------------------------

void CuiChatBubbleManager::setBubbleShowSelf       (bool b)
{
	setFlag (BF_showSelf, b);
}

//----------------------------------------------------------------------

bool CuiChatBubbleManager::getBubbleShowSelf       () 
{
	return hasFlag (BF_showSelf);
}

//----------------------------------------------------------------------

bool CuiChatBubbleManager::getBubbleShowSelfDefault       () 
{
	return (ms_flagsDefault & static_cast<int>(BF_showSelf)) != 0;
}

//----------------------------------------------------------------------

void CuiChatBubbleManager::setBubbleShowAll        (bool b)
{
	setFlag (BF_showAll, b);
}

//----------------------------------------------------------------------

bool CuiChatBubbleManager::getBubbleShowAll        () 
{
	return hasFlag (BF_showAll);
}

//----------------------------------------------------------------------

bool CuiChatBubbleManager::getBubbleShowAllDefault        () 
{
	return (ms_flagsDefault & static_cast<int>(BF_showAll)) != 0;
}

//----------------------------------------------------------------------

void CuiChatBubbleManager::setBubbleShowGuild      (bool b)
{
	setFlag (BF_showGuild, b);
}

//----------------------------------------------------------------------

bool CuiChatBubbleManager::getBubbleShowGuild      () 
{
	return hasFlag (BF_showGuild);
}

//----------------------------------------------------------------------

bool CuiChatBubbleManager::getBubbleShowGuildDefault      () 
{
	return (ms_flagsDefault & static_cast<int>(BF_showGuild)) != 0;
}

//----------------------------------------------------------------------

void CuiChatBubbleManager::setBubbleShowGroup      (bool b)
{
	setFlag (BF_showGroup, b);
}

//----------------------------------------------------------------------

bool CuiChatBubbleManager::getBubbleShowGroup      () 
{
	return hasFlag (BF_showGroup);
}

//----------------------------------------------------------------------

bool CuiChatBubbleManager::getBubbleShowGroupDefault      () 
{
	return (ms_flagsDefault & static_cast<int>(BF_showGroup)) != 0;
}

//----------------------------------------------------------------------

void CuiChatBubbleManager::setBubbleShowNPC        (bool b)
{
	setFlag (BF_showNPC, b);
}

//----------------------------------------------------------------------

bool CuiChatBubbleManager::getBubbleShowNPC        () 
{
	return hasFlag (BF_showNPC);
}

//----------------------------------------------------------------------

bool CuiChatBubbleManager::getBubbleShowNPCDefault        () 
{
	return (ms_flagsDefault & static_cast<int>(BF_showNPC)) != 0;
}

//----------------------------------------------------------------------

void CuiChatBubbleManager::setBubbleShowPlayer     (bool b)
{
	setFlag (BF_showPlayer, b);
}

//----------------------------------------------------------------------

bool CuiChatBubbleManager::getBubbleShowPlayer     () 
{
	return hasFlag (BF_showPlayer);
}
//----------------------------------------------------------------------

bool CuiChatBubbleManager::getBubbleShowPlayerDefault     () 
{
	return (ms_flagsDefault & static_cast<int>(BF_showPlayer)) != 0;
}

//----------------------------------------------------------------------

bool CuiChatBubbleManager::objectHasChatBubblesVisible (const NetworkId & id)
{
	const CachedNetworkId cid (id);
	
	const BubbleStackMap::const_iterator it = s_bubbleStackMap.find (cid);
	if (it != s_bubbleStackMap.end ())
	{
		const CachedNetworkId & theKey = (*it).first;
		return CuiChatBubbleManager::getBubblesEnabled () && CuiChatBubbleManager::chatBubbleOk (0, theKey);
	}

	return false;
}
			
//----------------------------------------------------------------------

//======================================================================	

