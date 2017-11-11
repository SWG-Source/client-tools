//======================================================================
//
// CuiPopupHelp.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiPopupHelp.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIManager.h"
#include "UIPage.h"
#include "UIText.h"
#include "clientUserInterface/CuiPopupHelpData.h"

#include <vector>

//----------------------------------------------------------------------

namespace CuiPopupHelpNamespace
{
	int s_popupHelpCount = 0;

	//----------------------------------------------------------------------

	inline bool checkVisibility (const UIWidget & widgetToCheck)
	{
		const UIWidget * wid = & widgetToCheck;
		while (wid)
		{
			if (!wid->IsVisible ())
				return false;
			wid = wid->GetParentWidget ();
		}
		return true;
	}
}

using namespace CuiPopupHelpNamespace;

//======================================================================

CuiPopupHelp::CuiPopupHelp (UIPage & page) :
CuiMediator       ("CuiPopupHelp", page),
UIEventCallback(),
m_buttonNext      (0),
m_ptrSW           (0),
m_ptrNW           (0),
m_ptrNE           (0),
m_ptrSE           (0),
m_okNext          (false),
m_target          (0),
m_desiredPosition (CuiPopupHelpData::DP_nw),
m_parentMediator  (0),
m_text            (0),
m_forceClose      (false)
{
	char buf [128];
	const size_t buf_size = sizeof (buf);

	snprintf (buf, buf_size, "CuiPopupHelp_%d", ++s_popupHelpCount);
	setMediatorDebugName (buf);

	getCodeDataObject (TUIText,   m_text, "text");

	m_text->SetPreLocalized (true);

	getCodeDataObject (TUIButton, m_buttonNext, "buttonNext", true);

	getCodeDataObject (TUIWidget,   m_ptrSW, "ptrSW");
	getCodeDataObject (TUIWidget,   m_ptrSE, "ptrSE");
	getCodeDataObject (TUIWidget,   m_ptrNW, "ptrNW");
	getCodeDataObject (TUIWidget,   m_ptrNE, "ptrNE");

	if (m_buttonNext)
		registerMediatorObject (*m_buttonNext, true);

	setState (MS_closeable);
}

//----------------------------------------------------------------------

bool CuiPopupHelp::setData      (CuiMediator & parentMediator, const CuiPopupHelpData & data)
{
	m_parentMediator = &parentMediator;

	m_desiredPosition = static_cast<int>(data.desiredPosition);

	m_target = safe_cast<UIWidget *>(parentMediator.getPage ().GetObjectFromPath(data.widgetPath.c_str (), TUIWidget));
	if (m_target)
	{
		m_target->Attach (0);
	}
	else
	{
		WARNING (true, ("CuiPopupHelp bad target [%s] for [%s]", data.widgetPath.c_str (), parentMediator.getPage ().GetFullPath ().c_str ()));
	}


	if (!data.scriptPre.empty ())
		UIManager::gUIManager().ExecuteScript(data.scriptPre, &parentMediator.getPage ());

	m_scriptPost = data.scriptPost;

	m_text->SetLocalText    (data.localText);
	m_text->SetTextUnroll   (true);

	getPage ().ForcePackChildren ();

	UISize extent;
	m_text->GetScrollExtent (extent);

	if (extent.y > m_text->GetHeight ())
		getPage ().SetHeight (getPage ().GetHeight () + (extent.y - m_text->GetHeight ()));

	return true;
}

//----------------------------------------------------------------------

void CuiPopupHelp::forceNext    ()
{
	m_okNext = true;
	//-- must be active to goto next
	activate ();
	closeThroughWorkspace ();
}

//----------------------------------------------------------------------

CuiPopupHelp::~CuiPopupHelp ()
{
	if (m_target)
		m_target->Detach (0);

	m_target = NULL;
	m_parentMediator = NULL;
}

//----------------------------------------------------------------------

void CuiPopupHelp::performActivate   ()
{
	autoPosition ();
	m_forceClose = false;

	setIsUpdating (true);
}

//----------------------------------------------------------------------

void CuiPopupHelp::performDeactivate ()
{
	setIsUpdating (false);
}

//----------------------------------------------------------------------

bool CuiPopupHelp::close ()
{
	if (m_forceClose)
	{
		m_forceClose = false;
		removeState (MS_openNextFrame);
		return false;
	}

	if (isActive ())
	{
		NOT_NULL (m_parentMediator);
		if (!m_scriptPost.empty())
			UIManager::gUIManager ().ExecuteScript (m_scriptPost, &m_parentMediator->getPage ());
		
		m_parentMediator->onPopupHelpClose (*this, m_okNext);
		removeState (MS_openNextFrame);
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

void CuiPopupHelp::OnButtonPressed (UIWidget * context)
{
	if (context == m_buttonNext)
	{
		m_okNext = true;
		closeThroughWorkspace ();
	}
}

//----------------------------------------------------------------------

void CuiPopupHelp::autoPosition ()
{
	if (!getPage ().GetParentWidget ())
		return;

	if (!m_target)
		return;

	if (m_parentMediator)
		m_parentMediator->raisePopupHelp ();

	typedef std::vector <int> IntVector;
	static IntVector iv;
	iv.clear ();
	iv.reserve (4);

	for (int i = 0; i < static_cast<int>(CuiPopupHelpData::DP_count); ++i)
	{
		if (i != m_desiredPosition)
			iv.push_back (i);
	}

	iv.push_back (m_desiredPosition);

	while (!iv.empty ())
	{
		const int dp = iv.back ();
		iv.pop_back ();

		std::pair<bool, int> retval = autoPosition (dp, iv.empty ());

		if (retval.first == true || iv.empty ())
			return;

		if (iv.back () == retval.second)
			continue;

		IntVector::iterator it = std::find (iv.begin (), iv.end (), retval.second);

		if (it != iv.end ())
		{
			const int nextVal = *it;
			iv.erase (it);
			iv.push_back (nextVal);
		}
	}
}

//----------------------------------------------------------------------

std::pair<bool, int> CuiPopupHelp::autoPosition      (int _desiredPosition, bool force)
{
	std::pair<bool, int> retval = std::make_pair (false, 0);

	if (!m_target)
		return retval;

	if (!getPage ().GetParentWidget ())
	{
		WARNING (true, ("CuiPopupHelp attempt to autoPosition with no parent widget"));
		return retval;
	}

	const UISize & parentSize = getPage ().GetParentWidget ()->GetSize ();
	const UISize & size       = getPage ().GetSize ();
	//-- this assumes that the parent page is the size of the screen
	UIRect targetRect         = m_target->GetWorldRect ();	
	UIPoint targetRectCenter  = UIPoint (targetRect.right + targetRect.left, targetRect.bottom + targetRect.top) / 2L;
	UISize  targetMargin      = targetRect.Size () / 4L;
	targetMargin.x = std::min (32L, targetMargin.x);
	targetMargin.y = std::min (32L, targetMargin.y);

	UIRect rect (size);

	CuiPopupHelpData::DesiredPosition desiredPosition = static_cast<CuiPopupHelpData::DesiredPosition>(_desiredPosition);

	switch (desiredPosition)
	{
	case CuiPopupHelpData::DP_nw:
		{
			rect += targetRect.Location () - size;
			rect += UIPoint (targetMargin.x, targetMargin.y);
			
			if (rect.left < 0L)
			{
				rect.right -= rect.left;
				rect.left   = 0L;
			}
			
			if (rect.top < 0L)
			{
				rect.bottom -= rect.top;
				rect.top     = 0;
			}
			
			if (rect.right > targetRectCenter.x)
			{
				if (rect.bottom > targetRectCenter.y)
					retval = std::make_pair (false, static_cast<int>(CuiPopupHelpData::DP_se));
				else
					retval = std::make_pair (false, static_cast<int>(CuiPopupHelpData::DP_ne));
			}
			else if (rect.bottom > targetRectCenter.y)
				retval = std::make_pair (false, static_cast<int>(CuiPopupHelpData::DP_sw));
			else
				retval = std::make_pair (true, _desiredPosition);
		}
		break;
	case CuiPopupHelpData::DP_ne:
		{
			rect += targetRect.Location () + UISize (targetRect.Size ().x, -size.y);
			rect += UIPoint (-targetMargin.x, targetMargin.y);

			if (rect.right > parentSize.x)
			{
				rect.left -= (rect.right - parentSize.x);
				rect.right  = parentSize.x;
			}
			
			if (rect.top < 0L)
			{
				rect.bottom -= rect.top;
				rect.top     = 0;
			}
			
			if (rect.left < targetRectCenter.x)
			{
				if (rect.bottom > targetRectCenter.y)
					retval.second = static_cast<int>(CuiPopupHelpData::DP_sw);
				else
					retval.second = static_cast<int>(CuiPopupHelpData::DP_nw);
			}
			else if (rect.bottom > targetRectCenter.y)
				retval.second = static_cast<int>(CuiPopupHelpData::DP_se);
			else
				retval = std::make_pair (true, _desiredPosition);
		}
		break;
	case CuiPopupHelpData::DP_sw:
		{
			rect += targetRect.Location () + UIPoint (-size.x, targetRect.Size ().y);
			rect += UIPoint (targetMargin.x, -targetMargin.y);
			
			if (rect.left < 0)
			{
				rect.right -= rect.left;
				rect.left   = 0L;
			}
			
			if (rect.bottom > parentSize.y)
			{
				rect.top    -= (rect.bottom - parentSize.y);
				rect.bottom  = parentSize.y;
			}
			
			if (rect.right > targetRectCenter.x)
			{
				if (rect.top > targetRectCenter.y)
					retval = std::make_pair (false, static_cast<int>(CuiPopupHelpData::DP_se));
				else
					retval = std::make_pair (false, static_cast<int>(CuiPopupHelpData::DP_ne));
			}
			else if (rect.top < targetRectCenter.y)
				retval = std::make_pair (false, static_cast<int>(CuiPopupHelpData::DP_nw));
			else
				retval = std::make_pair (true, _desiredPosition);
		}

		break;
	case CuiPopupHelpData::DP_se:
		{
			rect += targetRect.Location () + UISize (targetRect.Size ().x, targetRect.Size ().y);
			rect += UIPoint (-targetMargin.x, -targetMargin.y);
			
			if (rect.right > parentSize.x)
			{
				rect.left -= (rect.right - parentSize.x);
				rect.right  = parentSize.x;
			}
			
			if (rect.bottom > parentSize.y)
			{
				rect.top    -= (rect.bottom - parentSize.y);
				rect.bottom  = parentSize.y;
			}
			
			if (rect.left < targetRectCenter.x)
			{
				if (rect.top > targetRectCenter.y)
					retval.second = static_cast<int>(CuiPopupHelpData::DP_sw);
				else
					retval.second = static_cast<int>(CuiPopupHelpData::DP_nw);
			}
			else if (rect.top < targetRectCenter.y)
				retval.second = static_cast<int>(CuiPopupHelpData::DP_ne);
			else
				retval = std::make_pair (true, _desiredPosition);
		}
		break;
	}

	if (retval.first || force)
	{
		getPage ().SetRect (rect);
		m_ptrSW->SetVisible (retval.second == CuiPopupHelpData::DP_ne);
		m_ptrSE->SetVisible (retval.second == CuiPopupHelpData::DP_nw);
		m_ptrNW->SetVisible (retval.second == CuiPopupHelpData::DP_se);
		m_ptrNE->SetVisible (retval.second == CuiPopupHelpData::DP_sw);
	}

	return retval;
}

//----------------------------------------------------------------------

void CuiPopupHelp::setForceClose (bool b)
{
	m_forceClose = b;
}

//----------------------------------------------------------------------

void CuiPopupHelp::update (float deltaTimeSecs)
{
	UNREF (deltaTimeSecs);
}

//======================================================================
