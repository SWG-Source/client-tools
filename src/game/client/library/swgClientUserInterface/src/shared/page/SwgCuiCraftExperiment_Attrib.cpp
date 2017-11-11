// ======================================================================
//
// SwgCuiCraftExperiment_Attrib.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCraftExperiment_Attrib.h"

#include "StringId.h"
#include "UIButton.h"
#include "UIData.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIOpacityEffector.h"
#include "UIPage.h"
#include "UIText.h"
#include "UIUtils.h"
#include "clientGame/DraftSchematicInfo.h"
#include "clientGame/DraftSchematicManager.h"
#include "clientUserInterface/CuiCraftManager.h"
#include <list>

// ======================================================================

namespace
{
	static const char * const inner_button_name = "b";
}

//-----------------------------------------------------------------

SwgCuiCraftExperiment::Attrib::Attrib (UIPage & page, SwgCuiCraftExperiment & mediatorExperiment) :
CuiMediator           ("SwgCuiCraftExperiment::Attrib", page),
m_pageCur             (0),
m_pageExp             (0),
m_pageMax             (0),
m_pageResourceMax     (0),
m_textLabel           (0),
m_textValue           (0),
m_min                 (0),
m_max                 (0),
m_cur                 (0),
m_resourceMax         (0),
m_exp                 (0),
m_name                (new StringId),
m_mediatorExperiment  (mediatorExperiment),
m_expPointsAvailable  (0),
m_mouseDown           (false)
{
	{
		for (int i = 0; i < 10; ++i)
		{
			m_percentPages        [i] = 0;
			m_percentButtons      [i] = 0;
		}
	}

	getCodeDataObject (TUIPage,            m_pageCur,              "pageCur");
	getCodeDataObject (TUIPage,            m_pageMax,              "pageMax");
	getCodeDataObject (TUIPage,            m_pageResourceMax,      "pageResourceMax");
	getCodeDataObject (TUIPage,            m_pageExp,              "pageExp");
	getCodeDataObject (TUIPage,            m_pageInner,            "pageInner");

	getCodeDataObject (TUIText,            m_textLabel,            "textName");
	getCodeDataObject (TUIText,            m_textValue,            "textValue");

	m_textLabel->SetPreLocalized (true);
	m_textValue->SetPreLocalized (true);

	registerMediatorObject (*m_pageMax,           true);
	registerMediatorObject (*m_pageInner,         true);
	
	{
		int index = 0;
		const UIBaseObject::UIObjectList & olist = m_pageExp->GetChildrenRef ();
		for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
		{
			UIBaseObject * const obj = NON_NULL (*it);
			
			if (obj->IsA (TUIPage))
			{
				m_percentPages   [index] = NON_NULL (safe_cast<UIPage *>(obj));
				m_percentButtons [index] = NON_NULL (safe_cast<UIButton *>(obj->GetChild (inner_button_name)));
				registerMediatorObject (*m_percentPages [index],  true);
				++index;
			}
		}
	}
}

//----------------------------------------------------------------------

SwgCuiCraftExperiment::Attrib::~Attrib ()
{
	delete m_name;
	m_name = 0;
}

//-----------------------------------------------------------------

void SwgCuiCraftExperiment::Attrib::performActivate   ()
{
	setExp (0);
	m_mouseDown = false;
}

//-----------------------------------------------------------------

void SwgCuiCraftExperiment::Attrib::performDeactivate ()
{
}

//-----------------------------------------------------------------

bool SwgCuiCraftExperiment::Attrib::OnMessage( UIWidget *context, const UIMessage & msg )
{
	//-- an exp button
	if (context->GetParent () == m_pageExp)
	{
		if (msg.Type == UIMessage::LeftMouseDown)
		{
			const int expIndex = findExpIndex (context);
			if (expIndex < 0 || expIndex >= NumPercents)
			{
				WARNING (true, ("bad exp widget"));
				return true;
			}
	
			setExp (expIndex + 1 - m_expBegin);
			return false;
		}
	}
	else if (context == m_pageInner)
	{
		if (msg.Type == UIMessage::LeftMouseDown || (msg.Type == UIMessage::MouseMove && m_mouseDown))
		{
			const UIWidget * const valueWidget = NON_NULL (m_pageCur->GetParentWidget ());

			UIPoint pt = valueWidget->GetLocation ();

			const UIWidget * parent = NON_NULL (valueWidget->GetParentWidget ());

			while (parent && parent != m_pageInner)
			{
				pt += parent->GetLocation ();
				parent = parent->GetParentWidget ();
			}

			const long relpos = msg.MouseCoords.x - pt.x;
			const long end    = valueWidget->GetWidth ();

			if (relpos < 0)
				setExp (0);
			else if (relpos > end)
				setExp (m_expEnd - m_expBegin);
			else
			{
				setExp ((10 * relpos / end) + 1 - m_expBegin);
			}

			m_mouseDown = true;
			return false;
		}
		else if (msg.Type == UIMessage::LeftMouseUp)
		{
			m_mouseDown = false;
		}
	}

	return true;
}

//----------------------------------------------------------------------

void SwgCuiCraftExperiment::Attrib::setTextLabel (const StringId & name)
{
	*m_name = name;
	UIString nameStr;
	//-- hack to make attribs work if they containe upper case shite
	const StringId lowerName (m_name->getTable (), Unicode::toLower (m_name->getText ()));
	lowerName.localize (nameStr);
	m_textLabel->SetLocalText (nameStr);
}

//----------------------------------------------------------------------

void  SwgCuiCraftExperiment::Attrib::setAttrib (int index, const StringId & name, float min, float max, float cur, float resourceMax)
{
	m_exp   = 0;
	m_index = index;
	m_min   = min;
	m_max   = max;
	m_cur   = cur;
	m_resourceMax = resourceMax;

	setTextLabel  (name);

	Unicode::String tooltip;

	const std::pair<uint32, uint32> & draftCrc = CuiCraftManager::getCurrentDraftSchematicCrc ();
	const DraftSchematicInfo * const dsi = DraftSchematicManager::findDraftSchematic (draftCrc);
	if (dsi)
		dsi->formatDraftAttribWeights (tooltip, name.localize (), false);

	getPage ().SetLocalTooltip (tooltip);

	//-- convert to percent range
	const float relativeMax         = (max - min);
	const float relativeCur         = (cur - min);
	const float relativeResourceMax = (resourceMax - min);

	if (relativeMax == 0)
		return;

	const int resourceMaxPercent = static_cast<int>(relativeResourceMax * 100.0f);
	const int curPercent         = static_cast<int>(relativeCur         * 100.0f);

	const long maxParentLength = NON_NULL (m_pageMax->GetParentWidget ())->GetWidth ();
	const long usableLength    = NON_NULL (m_pageCur->GetParentWidget ())->GetWidth ();

	m_pageMax->SetWidth         (static_cast<long>(relativeMax         * maxParentLength));
	m_pageCur->SetWidth         (static_cast<long>(relativeCur         * usableLength));
	m_pageResourceMax->SetWidth (static_cast<long>(relativeResourceMax * usableLength));

	Unicode::String percentStr;
	UIUtils::FormatInteger (percentStr, curPercent);
	percentStr.append (1, '%');
	m_textValue->SetLocalText (percentStr);

	m_expBegin                   = curPercent          / 10;
	m_expEnd                     = resourceMaxPercent  / 10;
	if (m_expEnd * 10 < resourceMaxPercent)
		++m_expEnd;

	if (curPercent == resourceMaxPercent)
		m_expBegin = m_expEnd;

	m_expEnd                     = std::min (static_cast<int>(NumPercents), m_expEnd);

	updateExpBoxes   ();
}

//----------------------------------------------------------------------

void SwgCuiCraftExperiment::Attrib::setExp (int exp)
{
	const int maxExp    = m_expEnd - m_expBegin;
	const int curMaxExp = std::min (maxExp, m_exp + m_expPointsAvailable);

	exp = std::min (exp, curMaxExp);

	if (exp < 0)
		return;

	int diff = exp - m_exp;

	if (diff > 0)
		diff = m_mediatorExperiment.fetchPoints (diff);
	else
		diff = -m_mediatorExperiment.releasePoints (-diff);

	m_exp += diff;

	setExpPointsAvailable (m_expPointsAvailable - diff);
}

//----------------------------------------------------------------------

void SwgCuiCraftExperiment::Attrib::updateExpBoxes ()
{
	const int maxExp    = m_expEnd - m_expBegin;
	const int curMaxExp = std::min (maxExp, m_exp + m_expPointsAvailable);

	for (int i = 0; i < NumPercents; ++i)
	{
		UIPage * const page     = NON_NULL (m_percentPages   [i]);
		UIButton * const button = NON_NULL (m_percentButtons [i]);

		page->SetVisible (true);
		
		if (i < m_expBegin)
		{
			button->SetVisible (true);
			button->SetEnabled (false);
			button->SetIsToggleDown (false);
		}
		else if (i < m_expEnd)
		{
			button->SetVisible (true);
			button->SetEnabled (true);

			if (i < m_expBegin + m_exp)
				button->SetIsToggleDown (true);
			else
				button->SetIsToggleDown (false);
		}
		else
		{
			button->SetVisible (false);
			button->SetEnabled (false);
		}

		if (i < (m_expBegin + curMaxExp))
			page->SetEnabled (true);
		else
			page->SetEnabled (false);
	}

	m_pageExp->SetPackDirty (true);
}	


//----------------------------------------------------------------------

int SwgCuiCraftExperiment::Attrib::findExpIndex      (UIWidget * widget)
{
	if (widget->IsA (TUIButton))
	{
		for (int i = 0; i < NumPercents; ++i)
		{
			if (m_percentButtons [i] == widget)
				return i;
		}
	}
	else
	{
		for (int i = 0; i < NumPercents; ++i)
		{
			if (m_percentPages [i] == widget)
				return i;
		}
	}

	return -1;
}

//----------------------------------------------------------------------

const StringId & SwgCuiCraftExperiment::Attrib::getName           () const
{
	return *NON_NULL (m_name);
}

//----------------------------------------------------------------------

void SwgCuiCraftExperiment::Attrib::setExpPointsAvailable (int points)
{
	m_expPointsAvailable = points;
	updateExpBoxes ();
}

// ======================================================================
