//======================================================================
//
// SwgCuiCredits.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCredits.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameMusicManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "sharedFile/Iff.h"
#include "sharedGame/PvpData.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "swgClientUserInterface/SwgCuiHudFactory.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

#include "StringId.h"
#include "UIComposite.h"
#include "UIData.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIText.h"

//======================================================================

namespace SwgCuiCreditsNamespace
{
	//-- pixels per second
	const float s_scrollSpeed = 60.0f;

	static const std::string datatable_filename = "datatables/player/credits.iff";

	enum EntryType
	{
		ET_main,
		ET_company,
		ET_header,
		ET_title,
		ET_names,
		ET_text,
		ET_numEntryTypes,
		ET_spacer
	};

	typedef std::pair<EntryType, Unicode::String> Entry;
	typedef stdvector<Entry>::fwd EntryVector;

	EntryVector s_creditEntries;

	//----------------------------------------------------------------------

	void unloadCredits ()
	{
		s_creditEntries.clear ();
	}

	//----------------------------------------------------------------------

	void loadCredits ()
	{
		unloadCredits ();

		Iff iff;
		
		if (!iff.open (datatable_filename.c_str (), true))
		{
			WARNING (true, ("Data file %s not available.", datatable_filename.c_str ()));
			return;
		}
		
		DataTable dt;
		dt.load (iff);
		iff.close ();

		const int numColumns = dt.getNumColumns ();
		if (numColumns != static_cast<int>(ET_numEntryTypes))
		{
			WARNING (true, ("SwgCuiCredits num columns %d, expected %d", numColumns, static_cast<int>(ET_numEntryTypes)));
			return;
		}

		const int numRows = dt.getNumRows ();

		for (int i = 0; i < numRows; ++i)
		{
			bool found = false;
			for (int j = 0; j < numColumns; ++j)
			{
				const std::string & str = dt.getStringValue (j, i);
				if (!str.empty ())
				{
					if (str [0] == '@')
					{
						const Entry e (static_cast<EntryType>(j), StringId::decodeString (Unicode::narrowToWide (str)));
						s_creditEntries.push_back (e);
					}
					else
					{
						//-- convert dot to space
						if (str.size() == 1 && str [0] == '.')
						{
							const Entry e (static_cast<EntryType>(j), Unicode::narrowToWide (" "));
							s_creditEntries.push_back (e);
						}
						else
						{
							const Entry e (static_cast<EntryType>(j), Unicode::narrowToWide (str));
							s_creditEntries.push_back (e);
						}
					}
					found = true;
					break;
				}
			}

			if (!found)
			{
				const Entry e (ET_spacer, Unicode::emptyString);
				s_creditEntries.push_back (e);
			}
		}
	}
}

using namespace SwgCuiCreditsNamespace;

//----------------------------------------------------------------------

SwgCuiCredits::SwgCuiCredits        (UIPage & page) :
CuiMediator          ("SwgCuiCredits", page),
UIEventCallback      (),
m_comp               (0),
m_textSampleHeader   (0),
m_textSampleNames    (0),
m_textSampleMain     (0),
m_textSampleCompany  (0),
m_textSampleTitle    (0),
m_textSampleText     (0),
m_pageSampleSpacer   (0),
m_elapsedTime        (0.0f)
{
	getCodeDataObject (TUIComposite, m_comp,              "comp");

	getCodeDataObject (TUIText,      m_textSampleHeader,  "sampleHeader");
	getCodeDataObject (TUIText,      m_textSampleNames,   "SampleNames");
	getCodeDataObject (TUIText,      m_textSampleMain,    "SampleMain");
	getCodeDataObject (TUIText,      m_textSampleCompany, "SampleCompany");
	getCodeDataObject (TUIText,      m_textSampleTitle,   "SampleTitle");
	getCodeDataObject (TUIText,      m_textSampleText,    "SampleText");

	getCodeDataObject (TUIPage,      m_pageSampleSpacer,  "sampleSpacer");

	m_textSampleCompany->Attach (0);
	m_textSampleHeader->Attach  (0);
	m_textSampleMain->Attach    (0);
	m_textSampleNames->Attach   (0);
	m_textSampleText->Attach    (0);
	m_textSampleTitle->Attach   (0);
	m_pageSampleSpacer->Attach  (0);

	m_textSampleCompany->SetVisible (false);
	m_textSampleHeader->SetVisible  (false);
	m_textSampleMain->SetVisible    (false);
	m_textSampleNames->SetVisible   (false);
	m_textSampleText->SetVisible    (false);
	m_textSampleTitle->SetVisible   (false);
	m_pageSampleSpacer->SetVisible  (false);

	m_textSampleCompany->SetPreLocalized (true);
	m_textSampleHeader->SetPreLocalized  (true);
	m_textSampleMain->SetPreLocalized    (true);
	m_textSampleNames->SetPreLocalized   (true);
	m_textSampleText->SetPreLocalized    (true);
	m_textSampleTitle->SetPreLocalized   (true);

	m_comp->Clear ();

	registerMediatorObject (getPage (), true);

	getPage ().AddChild (m_textSampleCompany);
	getPage ().AddChild (m_textSampleHeader);
	getPage ().AddChild (m_textSampleMain);
	getPage ().AddChild (m_textSampleNames);
	getPage ().AddChild (m_textSampleText);
	getPage ().AddChild (m_textSampleTitle);
	getPage ().AddChild (m_pageSampleSpacer);
}

//----------------------------------------------------------------------

SwgCuiCredits::~SwgCuiCredits            ()
{
	m_textSampleCompany->Detach (0);
	m_textSampleHeader->Detach  (0);
	m_textSampleMain->Detach    (0);
	m_textSampleNames->Detach   (0);
	m_textSampleText->Detach    (0);
	m_textSampleTitle->Detach   (0);
	m_pageSampleSpacer->Detach  (0);
}

//----------------------------------------------------------------------

void SwgCuiCredits::performActivate      ()
{
	setPointerInputActive  (true);
	setKeyboardInputActive (true);
	setInputToggleActive   (false);

	loadCredits ();
	populate ();
	setIsUpdating (true);

	CreatureObject const * const player = Game::getPlayerCreature();

	uint32 const pvpFaction = player ? player->getPvpFaction() : PvpData::getNeutralFactionId();

	// we're always on your side
	if (PvpData::isImperialFactionId(pvpFaction))
		GameMusicManager::setMusicTrackOverride("sound/music_credits_imperial.snd");
	else if (PvpData::isRebelFactionId(pvpFaction))
		GameMusicManager::setMusicTrackOverride("sound/music_credits_rebel.snd");
	else
		GameMusicManager::setMusicTrackOverride("sound/music_credits_neutral.snd");
}

//----------------------------------------------------------------------

void SwgCuiCredits::performDeactivate    ()
{
	setPointerInputActive  (false);
	setKeyboardInputActive (true);
	setInputToggleActive   (true);

	m_comp->Clear ();
	setIsUpdating (false);
	unloadCredits ();

	GameMusicManager::cancelMusicTrackOverride();
}

//----------------------------------------------------------------------

bool SwgCuiCredits::OnMessage            (UIWidget *, const UIMessage & msg)
{
	if (msg.Type == UIMessage::KeyDown)
	{
		deactivate ();
		SwgCuiHudFactory::setHudActive (true);
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

void SwgCuiCredits::populate             ()
{
	getPage ().ForcePackChildren ();

	m_elapsedTime = -1.0f;
	m_comp->Clear ();
	
	m_textSampleCompany->SetVisible (true);
	m_textSampleHeader->SetVisible  (true);
	m_textSampleMain->SetVisible    (true);
	m_textSampleNames->SetVisible   (true);
	m_textSampleText->SetVisible    (true);
	m_textSampleTitle->SetVisible   (true);
	m_pageSampleSpacer->SetVisible  (true);
	
	//-- top spacer
	{
		UIPage * const spacer = safe_cast<UIPage *>(m_pageSampleSpacer->DuplicateObject ());
		spacer->SetMaximumSize (UISize (16384L, m_comp->GetHeight ()));
		spacer->SetMinimumSize (UISize (0L, spacer->GetMaximumSize ().y));
		m_comp->AddChild (spacer);
	}
	
	UIText * lastNamesText = 0;
	for (EntryVector::const_iterator it = s_creditEntries.begin (); it != s_creditEntries.end (); ++it)
	{
		const Entry & e = *it;
		
		const EntryType type        = e.first;
		const Unicode::String & str = e.second;
		
		if (type == ET_spacer)
		{
			UIPage * const spacer = safe_cast<UIPage *>(m_pageSampleSpacer->DuplicateObject ());
			m_comp->AddChild (spacer);
			lastNamesText = 0;
			continue;
		}
		
		UIText * text = 0;
		UIText * newLastNamesText = 0;

		switch (type)
		{
		case ET_main:
			text = safe_cast<UIText *>(m_textSampleMain->DuplicateObject ());
			break;
		case ET_company:
			text = safe_cast<UIText *>(m_textSampleCompany->DuplicateObject ());
			break;
		case ET_header:
			text = safe_cast<UIText *>(m_textSampleHeader->DuplicateObject ());
			break;
		case ET_title:
			text = safe_cast<UIText *>(m_textSampleTitle->DuplicateObject ());
			break;
		case ET_names:
			if (lastNamesText)
			{
				Unicode::String oldStr = lastNamesText->GetLocalText ();
				oldStr.push_back ('\n');
				oldStr += str;
				lastNamesText->SetLocalText (oldStr);
			}
			else
			{
				text = safe_cast<UIText *>(m_textSampleNames->DuplicateObject ());
				newLastNamesText = text;
			}
			break;
		case ET_text:
			text = safe_cast<UIText *>(m_textSampleText->DuplicateObject ());
			break;
		default:
			WARNING (true, ("SwgCuiCredits invalid type [%d]", type));
			break;
		}
		
		if (!text)
			continue;

		lastNamesText = newLastNamesText;
		
		text->SetLocalText (str);
		m_comp->AddChild (text);
		
		text->Link ();
		if (type != ET_names && type != ET_text)
		{
			if (str.size () != 1 || str [0] != ' ')
			{
				const long maxWidth = text->GetWidth ();
				text->SetWidth  (16384L);
				const UIRect & textMargin = text->GetMargin ();
				const UISize & textExtent = text->GetTextExtent ();
				text->SetWidth (textExtent.x + textMargin.left + textMargin.right);
				text->SetWidth (std::min (maxWidth, text->GetWidth ()));
				text->SetLocation (UIPoint ((m_comp->GetWidth () - text->GetWidth ()) / 2L, 0L));
			}
		}
	}
	
	//-- bottom spacer
	{
		UIPage * const spacer = safe_cast<UIPage *>(m_pageSampleSpacer->DuplicateObject ());
		spacer->SetMaximumSize (UISize (16384L, m_comp->GetHeight ()));
		spacer->SetMinimumSize (UISize (0L, spacer->GetMaximumSize ().y));
		m_comp->AddChild (spacer);
	}

	m_comp->Pack ();

	m_textSampleCompany->SetVisible (false);
	m_textSampleHeader->SetVisible  (false);
	m_textSampleMain->SetVisible    (false);
	m_textSampleNames->SetVisible   (false);
	m_textSampleText->SetVisible    (false);
	m_textSampleTitle->SetVisible   (false);
	m_pageSampleSpacer->SetVisible  (false);
}

//----------------------------------------------------------------------

void SwgCuiCredits::update               (float deltaTimeSecs)
{
	if (m_comp->IsUserModifying ())
		return;

	if (m_elapsedTime < 0)
	{
		m_elapsedTime = 0.0f;
		return;
	}

	m_elapsedTime += deltaTimeSecs;

	const long pixelsToScroll = static_cast<long>(s_scrollSpeed * m_elapsedTime);

	if (!pixelsToScroll)
		return;

	//-- keep the remainder for next update
	m_elapsedTime -= static_cast<float>(pixelsToScroll / s_scrollSpeed);

	UIPoint scrollLocation   = m_comp->GetScrollLocation ();
	UISize scrollExtent;
	m_comp->GetScrollExtent (scrollExtent);

	scrollLocation.y += pixelsToScroll;

	if (scrollLocation.y >= scrollExtent.y - m_comp->GetHeight ())
		scrollLocation.y = 0;

	m_comp->SetScrollLocation (scrollLocation);

/*
	const UIBaseObject::UIObjectList & olist = m_comp->GetChildrenRef ();
	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		UIWidget * const widget = safe_cast<UIWidget *>(*it);
		NOT_NULL (widget);

		const UIRect & rect = widget->GetRect ();

		const long comp_bottom = scrollLocation.y + m_comp->GetHeight ();
		if ((rect.top < comp_bottom) &&
			rect.bottom > scrollLocation.y)
		{
			const long height = widget->GetHeight ();

			if (rect.top < scrollLocation.y)
			{
				widget->SetOpacity (static_cast<float>(height - (scrollLocation.y - rect.top)) / static_cast<float>(height));
			}
			else if (rect.bottom > comp_bottom)
			{
				widget->SetOpacity (static_cast<float>(height - (rect.bottom - comp_bottom)) / static_cast<float>(height));
			}
			else
				widget->SetOpacity (1.0f);


		}
	}

  */
}

//======================================================================
