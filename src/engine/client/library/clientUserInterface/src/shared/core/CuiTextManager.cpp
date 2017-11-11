// ======================================================================
//
// CuiTextManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiTextManager.h"

#include "LocalizationManager.h"
#include "LocalizedString.h"
#include "LocalizedStringTable.h"
#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIPage.h"
#include "UIRectangleStyle.h"
#include "UIText.h"
#include "UITextStyle.h"
#include "UITextStyleManager.h"
#include "UITypes.h"
#include "UIWidgetRectangleStyles.h"
#include "UnicodeUtils.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientGraphics/Graphics.h"
#include "clientUserInterface/CuiLayer.h"
#include "clientUserInterface/CuiObjectTextManager.h"
#include "clientUserInterface/CuiPreferences.h"

#include <algorithm>
#include <map>

// ======================================================================

namespace
{
	typedef std::multimap<real, CuiTextManager::TextInfo> TextMap_t;
	
	TextMap_t *                           s_textMap;
	
	typedef std::map<int, UITextStyle *>  FontMap_t;
	
	FontMap_t *                           s_fontMaps [CuiTextManagerTextEnqueueInfo::TW_count];
	
	UISize                                s_screenSize;

	UIText *                              s_text;
	UIImage *                             s_image;
	UIImage *                             s_image2;

	static const float TEXT_DISTANCE_LIMIT_SOFT           = 192.0f;
	static const float TEXT_DISTANCE_LIMIT_SOFT_RECIP     = RECIP (TEXT_DISTANCE_LIMIT_SOFT);
	static const float TEXT_DISTANCE_LIMIT_HARD           = 256.0f;
	static const float TEXT_DISTANCE_LIMIT_RANGE          = TEXT_DISTANCE_LIMIT_HARD - TEXT_DISTANCE_LIMIT_SOFT;
	static const float TEXT_DISTANCE_LIMIT_RANGE_RECIP    = RECIP (TEXT_DISTANCE_LIMIT_RANGE);
}

//-----------------------------------------------------------------

bool                                      CuiTextManager::ms_installed;

//-----------------------------------------------------------------

void CuiTextManager::install (const UIBaseObject & rootPage)
{
	UNREF(rootPage);
	DEBUG_FATAL (ms_installed, ("already installed.\n"));

	s_textMap = new TextMap_t;

	const Unicode::String fontPrefixes [CuiTextManagerTextEnqueueInfo::TW_count] = 
	{
		Unicode::narrowToWide ("default_"),
		Unicode::narrowToWide ("bold_"),
		Unicode::narrowToWide ("starwars_")
	};

	for (int weight = 0; weight < CuiTextManagerTextEnqueueInfo::TW_count; ++weight)
	{		
		FontMap_t * const fontMap = s_fontMaps [weight] = new FontMap_t;

		for (int i = 12; i < 35; ++i)
		{
			char buf [32];
			IGNORE_RETURN (_itoa (i, buf, 10));
			
			const Unicode::String suffix (Unicode::narrowToWide (buf));

			UITextStyle * const textStyle = UITextStyleManager::GetInstance()->GetFontForLogicalFont (fontPrefixes [weight] + suffix);
			
			if (textStyle)
				fontMap->insert (std::make_pair (i, textStyle));
		}

		DEBUG_FATAL (fontMap->empty (), ("No fonts found for weight %d: %s.", weight, Unicode::wideToNarrow(fontPrefixes [weight]).c_str()));
	}
	

	s_text = new UIText;
	s_text->SetTextAlignment (UITextStyle::Center);
	s_text->SetPreLocalized  (true);
	s_text->Attach (0);
	s_image = new UIImage;
	s_image->Attach (0);
	s_image2 = new UIImage;
	s_image2->Attach (0);

	ms_installed = true;
}

//-----------------------------------------------------------------

void CuiTextManager::remove ()
{
	DEBUG_FATAL (!ms_installed, ("not installed.\n"));

	delete s_textMap;
	s_textMap = 0;

	for (int weight = 0; weight < CuiTextManagerTextEnqueueInfo::TW_count; ++weight)
	{		
		delete s_fontMaps [weight];
		s_fontMaps [weight] = 0;
	}

	s_image->Detach  (0);
	s_image2->Detach (0);
	s_text->Detach   (0);

	ms_installed = false;
}

//-----------------------------------------------------------------

void  CuiTextManager::resetQueue ()
{
	s_textMap->clear ();
}

//----------------------------------------------------------------------

void CuiTextManager::enqueueText (const Unicode::String & str, const TextEnqueueInfo & textEnqueueInfo)
{
	IGNORE_RETURN (s_textMap->insert (std::make_pair (textEnqueueInfo.screenVect.z, TextInfo (str, textEnqueueInfo.worldDistance, TextChatInfo (), textEnqueueInfo))));
}

//----------------------------------------------------------------------

void CuiTextManager::enqueueChatBubble (const Unicode::String & str, const TextChatInfo & textChatInfo, const TextEnqueueInfo & textEnqueueInfo)
{
	IGNORE_RETURN (s_textMap->insert (std::make_pair (textEnqueueInfo.screenVect.z, TextInfo  (str, textEnqueueInfo.worldDistance, textChatInfo, textEnqueueInfo))));
}

//----------------------------------------------------------------------

void CuiTextManager::showSystemStatusString (const Unicode::String & str)
{
	TextEnqueueInfo sysinfo_tq;
	sysinfo_tq.screenVect.y      = 180.0f;
	sysinfo_tq.screenVect.x      = static_cast<float>(s_screenSize.x / 2);
	sysinfo_tq.worldDistance     = 0.0f;
	sysinfo_tq.opacity           = 1.0f;
	sysinfo_tq.backgroundOpacity = 0.0f;
	sysinfo_tq.textColor         = UIColor::red;
	sysinfo_tq.textWeight        = CuiTextManagerTextEnqueueInfo::TW_starwars;
	sysinfo_tq.textSize          = 3.0f;
		 
	TextChatInfo syschatinfo;
	TextInfo sysinfo (str, 1.0f, syschatinfo, sysinfo_tq);

	sysinfo.m_pt.x = s_screenSize.x / 2;
	sysinfo.m_info.screenVect.x = static_cast<float>(sysinfo.m_pt.x);

	IGNORE_RETURN (s_textMap->insert (std::make_pair (sysinfo_tq.screenVect.z, TextInfo  (str, sysinfo_tq.worldDistance, syschatinfo, sysinfo_tq))));
}

//-----------------------------------------------------------------

void CuiTextManager::renderPlainText (UICanvas & canvas, const TextInfo & ti, UITextStyle & style, const UISize & maxBubbleSize)
{
	UNREF (canvas);
	UNREF (ti);
	UNREF (maxBubbleSize);
	UNREF (style); 

	s_text->SetTextColor (ti.m_info.textColor);
	s_text->SetBackgroundOpacity (ti.m_info.backgroundOpacity);
	s_text->SetMargin (UIRect (4,4,4,4));
	s_text->SetStyle (&style);
	s_text->SetLocalText (ti.m_str);
	s_text->SizeToContent ();

	if (ti.m_info.imageStyle)
	{
		s_image->SetStyle(ti.m_info.imageStyle);
		s_image->SetSize(ti.m_info.imageStyle->GetSize());
	}
	else
		s_image->SetStyle (0);

	if (ti.m_info.imageStyle2)
	{
		s_image2->SetStyle(ti.m_info.imageStyle2);
		s_image2->SetSize(ti.m_info.imageStyle2->GetSize());
	}
	else
		s_image2->SetStyle (0);

	const int headOffset = CuiObjectTextManager::getObjectHeadPointOffset (ti.m_info.id, 0);

	const UISize & newSize = s_text->GetSize();
	const UISize & imgSize = s_image->GetSize();
	const UIPoint upperLeft       (ti.m_pt.x   - (newSize.x + imgSize.x) / 2L, ti.m_pt.y - newSize.y + headOffset);
	const UIPoint actualTextPoint (upperLeft.x + imgSize.x, upperLeft.y);
	const UIPoint upperRight      (actualTextPoint + UIPoint (newSize.x, 0L));

	if (ti.m_info.updateOffset)
		CuiObjectTextManager::setObjectHeadPointOffset (ti.m_info.id, actualTextPoint.y - static_cast<int>(ti.m_pt.y));
		
	//-----------------------------------------------------------------
	//- coordinates are relative to bubble origin  (actualTextPoint)
	
	canvas.PushState ();
	canvas.ModifyOpacity (ti.m_info.opacity);

	if (ti.m_info.imageStyle)
	{
		canvas.PushState ();
		canvas.Translate (upperLeft);
		s_image->Render (canvas);
		canvas.PopState ();
	}

	if (ti.m_info.imageStyle2)
	{
		canvas.PushState ();
		canvas.Translate (upperRight);
		s_image2->Render (canvas);
		canvas.PopState ();
	}

	canvas.Translate    (actualTextPoint);
	s_text->SetLocation (actualTextPoint);

	s_text->Render (canvas);

	//-----------------------------------------------------------------
	
	canvas.PopState ();
}

//-----------------------------------------------------------------

void CuiTextManager::renderBubble (UICanvas & canvas, const TextInfo & ti, UITextStyle & style, const UISize & maxBubbleSize)
{
	UNREF (canvas);
	UNREF (ti);
	UNREF (style);
	UNREF (maxBubbleSize);
}

//-----------------------------------------------------------------

void  CuiTextManager::render (UICanvas & canvas)
{
	const float oldOpacity = canvas.GetOpacity ();

	s_screenSize.x = Graphics::getCurrentRenderTargetWidth  ();
	s_screenSize.y = Graphics::getCurrentRenderTargetHeight ();

	//-- display "GOD MODE" text if necessary
	if (PlayerObject::isAdmin ())
	{
		TextEnqueueInfo godinfo_tq;
		static bool init = false;
		if (!init)
		{
			init = true;			
			godinfo_tq.worldDistance     = 0.0f;
			godinfo_tq.opacity           = 1.0f;
			godinfo_tq.backgroundOpacity = 0.0f;
			godinfo_tq.textColor         = UIColor::yellow;
			godinfo_tq.textWeight        = CuiTextManagerTextEnqueueInfo::TW_starwars;
			godinfo_tq.textSize          = 3.0f;
		}
			
		static TextInfo godinfo (Unicode::narrowToWide ("GOD MODE"), 1.0f, TextChatInfo (), godinfo_tq);

		godinfo.m_pt.x = s_screenSize.x / 2;
		godinfo.m_info.screenVect.x = static_cast<float>(godinfo.m_pt.x);

		godinfo.m_pt.y = Game::isSpace() ? 200 : 222;
		godinfo.m_info.screenVect.y = static_cast<float>(godinfo.m_pt.y);

		UITextStyle * const style = getTextStyle (20, godinfo.m_info);
		
		if (style)
		{
			canvas.SetOpacity (1.0f);
			renderPlainText (canvas, godinfo, *style, UISize (1000, 1000));
		}
	}

	//----------------------------------------------------------------------

	if (s_textMap->size ())
	{
		canvas.SetClip(UIRect (0, (s_screenSize.y / 10), s_screenSize.x, s_screenSize.y));

		const UISize maxBubbleSize (s_screenSize.x / 2, s_screenSize.y / 4);

		const float preferenceFontSizeFactor = CuiPreferences::getObjectNameFontSizeFactor ();
		
		for (TextMap_t::reverse_iterator rit = s_textMap->rbegin (); rit != s_textMap->rend (); ++rit) //lint !e55 !e81 //bad type
		{
			const TextInfo & ti = (*rit).second;

			if (ti.m_str.empty ())
				continue;

			//-- world distance on textinfo is prenormalized
			const float worldDistance = ti.m_info.worldDistance;
			const float textScale     = std::min (1.0f, std::max (0.0f, - (worldDistance - TEXT_DISTANCE_LIMIT_SOFT) * TEXT_DISTANCE_LIMIT_SOFT_RECIP));
			const int textSize        = static_cast<int>((textScale * 11.0f + 9.0f) * ti.m_info.textSize * preferenceFontSizeFactor);

			UITextStyle * const style = getTextStyle (textSize, ti.m_info);
			
			if (style == 0)
				continue;
			
			float distanceOpacity = oldOpacity;
		
			if (worldDistance > TEXT_DISTANCE_LIMIT_SOFT)
				distanceOpacity *= (TEXT_DISTANCE_LIMIT_RANGE - (worldDistance - TEXT_DISTANCE_LIMIT_SOFT)) * TEXT_DISTANCE_LIMIT_RANGE_RECIP;

			canvas.SetOpacity (distanceOpacity);

			if (ti.m_chatInfo.isChatBubble)
				renderBubble (canvas, ti, *style, maxBubbleSize);
			else
				renderPlainText (canvas, ti, *style, maxBubbleSize);
		}

		canvas.SetClip(UIRect (0, 0, s_screenSize.x, s_screenSize.y));
	}

	canvas.SetOpacity (oldOpacity); 
}

//-----------------------------------------------------------------
	
UITextStyle *  CuiTextManager::getTextStyle (long height, const TextEnqueueInfo & info)
{	
	const FontMap_t * fontMap = 0;

	fontMap = s_fontMaps [info.textWeight];

	FontMap_t::const_iterator fit = fontMap->lower_bound (height);
	
	if (fit == fontMap->end ())
		fit = (++(fontMap->rbegin ())).base (); //lint !e1058
	
	return (*fit).second;
}

//----------------------------------------------------------------------

UITextStyle * CuiTextManager::getTextStyle (long height)
{
	return getTextStyle (height, TextEnqueueInfo ());
}

// ======================================================================
