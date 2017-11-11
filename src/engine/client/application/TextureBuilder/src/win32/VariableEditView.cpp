// ======================================================================
//
// VariableEditView.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "VariableEditView.h"

#include "IntSliderVariableControl.h"
#include "IntVariableElement.h"
#include "sharedFoundation/PointerDeleter.h"
#include "TextureBuilderDoc.h"
#include "VariableControl.h"
#include "sharedFoundation/VoidMemberFunction.h"

#include <algorithm>
#include <string>
#include <vector>

// ======================================================================

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ======================================================================

namespace
{
	const int ms_borderWidth  = 10;
	const int ms_borderHeight = 10;
}

// ======================================================================

VariableEditView::VariableEditView()
:	CScrollView(),
	m_variableControls(new VariableControlVector()),
	m_viewWidth(0),
	m_viewHeight(0),
	m_eraseBrush(GetSysColorBrush(COLOR_BTNFACE))
{
}

// ----------------------------------------------------------------------

VariableEditView::~VariableEditView()
{
	m_eraseBrush = 0;

	std::for_each(m_variableControls->begin(), m_variableControls->end(), PointerDeleter());
	delete m_variableControls;
}

// ----------------------------------------------------------------------

IMPLEMENT_DYNCREATE(VariableEditView, CScrollView)

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(VariableEditView, CScrollView)
	//{{AFX_MSG_MAP(VariableEditView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

void VariableEditView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);
}

// ----------------------------------------------------------------------

void VariableEditView::OnDraw(CDC* pDC)
{
	// CDocument* pDoc = GetDocument();
	UNREF(pDC);
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void VariableEditView::AssertValid() const
{
	CScrollView::AssertValid();
}
#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG
void VariableEditView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG
TextureBuilderDoc *VariableEditView::GetDocument()
{
	return NON_NULL(dynamic_cast<TextureBuilderDoc*>(CScrollView::GetDocument()));
}
#endif

// ----------------------------------------------------------------------

void VariableEditView::setupControlDimensions()
{
	const int startX   = ms_borderWidth;
	const int endX     = m_viewWidth - ms_borderWidth + 1;

	int       currentY = ms_borderHeight;

	//-- tell each window to resize itself, returning the end y value for the control
	const VariableControlVector::iterator itEnd = m_variableControls->end();
	for (VariableControlVector::iterator it = m_variableControls->begin(); it != itEnd; ++it)
		currentY = (*it)->setupDimensions(startX, currentY, endX);

	//-- set scroll view size
	CSize  sizeTotal;

	sizeTotal.cx = m_viewWidth;
	sizeTotal.cy = currentY + ms_borderHeight;

	SetScrollSizes(MM_TEXT, sizeTotal);
}

// ----------------------------------------------------------------------

void VariableEditView::replaceVariableElements(VariableElementVector &variableElements)
{
	//-- delete existing controls
	std::for_each(m_variableControls->begin(), m_variableControls->end(), PointerDeleter());
	m_variableControls->clear();
	m_variableControls->reserve(variableElements.size());

	//-- create a control for each element
	{
		const VariableElementVector::iterator itEnd = variableElements.end();
		for (VariableElementVector::iterator it = variableElements.begin(); it != itEnd; ++it)
		{
			// -TRF- this part is the linkage between the VariableControl hierarchy and the VariableElement hierachy.
			//       I'm not thrilled with this, but I really don't like having a VariableElement know which kind of
			//       VariableControl to create.  That would link the VariableElement hierarchy to the Control hierarchy.
			IntVariableElement *const intVariableElement = dynamic_cast<IntVariableElement*>(*it);
			if (intVariableElement)
			{
				// this element is an IntVariableElement object
				m_variableControls->push_back(new IntSliderVariableControl(this, *intVariableElement));
				continue;
			}

			FATAL(true, ("unsupported variable element type with label [%s]", (*it)->getLabel().c_str()));
		}
	}

	setupControlDimensions();

	//-- update the control values
	std::for_each(m_variableControls->begin(), m_variableControls->end(), VoidMemberFunction(&VariableControl::loadControlFromVariable));

	//-- redraw
	Invalidate();
}

// ----------------------------------------------------------------------

void VariableEditView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);

	if (cx && cy)
	{
		m_viewWidth  = cx;
		m_viewHeight = cy;

		setupControlDimensions();
		Invalidate();
	}	
}

// ----------------------------------------------------------------------

BOOL VariableEditView::OnEraseBkgnd(CDC* pDC) 
{
	//-- fill the view with the dialog box color
	CRect rect(0,0,m_viewWidth, m_viewHeight);
	FillRect(pDC->m_hDC, rect, m_eraseBrush);

	//-- indicate we erased
	return TRUE;
}

// ----------------------------------------------------------------------

void VariableEditView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (!pScrollBar || (pScrollBar->GetRuntimeClass() != RUNTIME_CLASS(CSliderCtrl)))
	{
		//-- not one of our slider bars, let the scroll view handle it
		CScrollView::OnHScroll(nSBCode, nPos, pScrollBar);
	}
	else
	{
		//-- this is one of our slider bars
		VariableControl *const variableControl = reinterpret_cast<VariableControl*>(pScrollBar->GetDlgCtrlID());

		//-- make sure the variable control returned is valid
		VariableControlVector::iterator it = std::find(m_variableControls->begin(), m_variableControls->end(), variableControl);
		DEBUG_FATAL(it == m_variableControls->end(), ("slider bar control is not managed by a variable control"));

		const bool notifyDocument = variableControl->saveControlToVariable(pScrollBar);
		if (notifyDocument)
			GetDocument()->notifyVariableElementModified(variableControl->getVariableElement());
	}
}

// ----------------------------------------------------------------------

BOOL VariableEditView::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (HIWORD(wParam) != EN_CHANGE)
		return CScrollView::OnCommand(wParam, lParam);
	else
	{
		CWnd *const childWindow = CWnd::FromHandle(reinterpret_cast<HWND>(lParam));
		if (!childWindow)
			return CScrollView::OnCommand(wParam, lParam);
		else		
		{
			VariableControl *const variableControl = reinterpret_cast<VariableControl*>(childWindow->GetDlgCtrlID());

			//-- check if the control is one of our variable  controls
			VariableControlVector::iterator it = std::find(m_variableControls->begin(), m_variableControls->end(), variableControl);
			if (it == m_variableControls->end())
			{
				// not a control we're tracking
				return CScrollView::OnCommand(wParam, lParam);
			}
			else
			{
				//-- save variable control value to the variable element
				const bool notifyDocument = variableControl->saveControlToVariable(childWindow);

				//-- update the doc
				if (notifyDocument)
					GetDocument()->notifyVariableElementModified(variableControl->getVariableElement());

				//-- indicate we handled this command
				return TRUE;
			}
		}
	}
}

// ======================================================================
