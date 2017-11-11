// ======================================================================
//
// VariableSetView.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstViewer.h"
#include "VariableSetView.h"

#include "afxcmn.h"
#include "clientSkeletalAnimation/CompositeMesh.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/Object.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/VoidBindSecond.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/RangedIntCustomizationVariable.h"
#include "viewer.h"
#include "viewerDoc.h"

#include <algorithm>
#include <set>
#include <string>
#include <vector>

// ======================================================================

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ======================================================================
// lint supression
// ======================================================================

//error 754: Info -- local structure member xxx not referenced
//lint -esym(754, VariableInfo::VariableInfo)
//lint -esym(754, VariableInfo::operator=)

namespace
{
	const int ms_leftViewBorder         = 10;
	const int ms_topViewBorder          = 10;

	const int ms_sliderHeight           = 50;
	const int ms_sliderWidth            = 400;

	const int ms_variableInfoSeparation = 20;
}

// ======================================================================

int  VariableSetView::ms_textHeight;
int  VariableSetView::ms_variableInfoHeight;

// ======================================================================
// embedded entity declarations
// ======================================================================

class VariableSetView::VariableInfo
{
public:

	VariableInfo(RangedIntCustomizationVariable &variable, const std::string &variableName, int controlId, CWnd *parentWindow, int left, int top, int sliderHeight, int sliderWidth);
	~VariableInfo();

	void  show();
	void  draw(CDC *pDC);

	void  saveControlToVariable();

private:

	// disabled
	VariableInfo();
	VariableInfo(const VariableInfo&);
	VariableInfo &operator =(const VariableInfo&);

private:

	CSliderCtrl                     m_control;

	RangedIntCustomizationVariable &m_variable;
	std::string                     m_variableName;

	int                             m_left;
	int                             m_top;

};

// ======================================================================
// class VariableSetView::VariableInfo
// ======================================================================

VariableSetView::VariableInfo::VariableInfo(RangedIntCustomizationVariable &variable, const std::string &variableName, int controlId, CWnd *parentWindow, int left, int top, int sliderHeight, int sliderWidth) :
	m_control(),
	m_variable(variable),
	m_variableName(variableName),
	m_left(left),
	m_top(top)
{
	//-- create the slider control
	RECT  rect;

	rect.left   = left;
	rect.top    = top + ms_textHeight;
	rect.right  = rect.left + sliderWidth;
	rect.bottom = rect.top + sliderHeight;

	const BOOL createResult = m_control.Create(TBS_HORZ | TBS_BOTH, rect, parentWindow, static_cast<UINT>(controlId));
	DEBUG_FATAL(!createResult, ("failed to create control\n"));
	UNREF(createResult);

	//-- set control range
	int minRangeInclusive = 0;
	int maxRangeExclusive = 100;

	variable.getRange(minRangeInclusive, maxRangeExclusive);
	m_control.SetRange(minRangeInclusive, maxRangeExclusive - 1);

	//-- set initial position
	m_control.SetPos(variable.getValue());
}

// ----------------------------------------------------------------------

VariableSetView::VariableInfo::~VariableInfo()
{
	IGNORE_RETURN(m_control.DestroyWindow());
}

// ----------------------------------------------------------------------

void VariableSetView::VariableInfo::show()
{
	IGNORE_RETURN(m_control.ShowWindow(SW_SHOW));
}

// ----------------------------------------------------------------------

void VariableSetView::VariableInfo::draw(CDC *pDC)
{
	NOT_NULL(pDC);

	//-- render text and value
	char text[256];

	sprintf(text, "%s [%d]", m_variableName.c_str(), m_control.GetPos());
	IGNORE_RETURN(pDC->TextOut(m_left, m_top, text, strlen(text)));
}

// ----------------------------------------------------------------------

void VariableSetView::VariableInfo::saveControlToVariable()
{
	m_variable.setValue(m_control.GetPos());
}

// ======================================================================
// class VariableSetView
// ======================================================================

IMPLEMENT_DYNCREATE(VariableSetView, CScrollView)

// ----------------------------------------------------------------------

VariableSetView::VariableSetView()
:	CScrollView(),
	m_variableInfo(new VariableInfoVector()),
	m_viewWidth(150),
	m_viewHeight(100),
	m_eraseBrush(GetSysColorBrush(COLOR_BTNFACE)),
	m_customizationData(0),
	m_variableInfoTop(0),
	m_controlId(0),
	m_variableNameSet(0)
{
}

// ----------------------------------------------------------------------

VariableSetView::~VariableSetView()
{
	delete m_variableNameSet;

	m_eraseBrush = 0;

	if (m_customizationData)
	{
		m_customizationData->release();
		m_customizationData = 0;
	}

	//-- variableInfo elements are released in Destroy()
	delete m_variableInfo;
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(VariableSetView, CScrollView)
	//{{AFX_MSG_MAP(VariableSetView)
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

void VariableSetView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	//-- compute window-related dimensions
	{
		TEXTMETRIC textMetric;

		CDC *pDC = GetDC();
		DEBUG_FATAL(!pDC, ("failed to get DC for window"));

		const BOOL gtmSuccess = pDC->GetTextMetrics(&textMetric);
		DEBUG_FATAL(!gtmSuccess, ("failed to get text metrics"));
		UNREF(gtmSuccess);

		ms_textHeight         = textMetric.tmHeight + textMetric.tmExternalLeading;
		ms_variableInfoHeight = ms_textHeight + ms_sliderHeight;

		IGNORE_RETURN(ReleaseDC(pDC));
	}

	//-- get CustomizationData from viewer Object
	m_customizationData = 0;

	{
		Object *const object = getViewerDocument()->getObject();
		if (object)
		{
			//-- get CustomizationData for object
			CustomizationDataProperty *const property = safe_cast<CustomizationDataProperty*>(object->getProperty( CustomizationDataProperty::getClassPropertyId() ));

			if (property)
				m_customizationData = property->fetchCustomizationData();
		}
	}

	//-- build control information from document's variable set
	const int controlHeight = (m_customizationData ? buildVariableSetControlInfo(*m_customizationData) : 0);

	//-- setup the document scrolling size info
	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = ms_leftViewBorder + ms_sliderWidth;
	sizeTotal.cy = controlHeight;
	SetScrollSizes(MM_TEXT, sizeTotal);

	//-- show all the controls
	std::for_each(m_variableInfo->begin(), m_variableInfo->end(), VoidMemberFunction(&VariableInfo::show));
}

// ----------------------------------------------------------------------

void VariableSetView::OnDraw(CDC* pDC)
{
	//-- set draw mode to transparent
	const int oldBackgroundMode = pDC->SetBkMode(TRANSPARENT);
	
	//-- tell variables to draw themselves
	std::for_each(m_variableInfo->begin(), m_variableInfo->end(), VoidBindSecond(VoidMemberFunction(&VariableInfo::draw), pDC));

	//-- cleanup
	IGNORE_RETURN(pDC->SetBkMode(oldBackgroundMode));
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

void VariableSetView::AssertValid() const
{
	CScrollView::AssertValid();
}

#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG

void VariableSetView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif

// ----------------------------------------------------------------------

CViewerDoc *VariableSetView::getViewerDocument()
{
	CViewerDoc *const pDoc = NON_NULL(dynamic_cast<CViewerDoc*>(GetDocument()));
	return pDoc;
}

// ----------------------------------------------------------------------

SkeletalAppearance2 *VariableSetView::getDocumentAppearance()
{
	//-- get the doc
	CViewerDoc *const pDoc = getViewerDocument();

	//-- get the viewer object
	Object *const object = pDoc->getObject();
	if (!object)
		return 0;

	//-- get the object appearance
	SkeletalAppearance2 *appearance = dynamic_cast<SkeletalAppearance2*>(object->getAppearance());
	if (!appearance)
		return 0;

	return appearance;
}

// ----------------------------------------------------------------------

int VariableSetView::buildVariableSetControlInfo(CustomizationData &customizationData)
{
	//-- Clear out the m_variableNameSet variable.
	if (!m_variableNameSet)
		m_variableNameSet = new StringSet();
	else
		m_variableNameSet->clear();

	//-- Add all variable names existing in the CustomizationData to the m_variableNameSet 
	customizationData.iterateOverVariables(variableIteratorCallback, this);

	//-- Retrieve and add the customization variables in alphabetical order.
	m_variableInfoTop = ms_topViewBorder;

	const StringSet::const_iterator endIt = m_variableNameSet->end();
	for (StringSet::const_iterator it = m_variableNameSet->begin(); it != endIt; ++it)
	{
		const std::string &fullVariablePathName = *it;

		// Get the variable.
		CustomizationVariable *const customizationVariable = customizationData.findVariable(fullVariablePathName);
		if (!customizationVariable)
		{
			WARNING(true, ("customization variable [%s] found during variable scan but findVariable() with same name returned NULL.", fullVariablePathName.c_str()));
			break;
		}

		// Add it to the view.
		addVariable(fullVariablePathName, customizationVariable);
	}

	//-- Return modified variable info top.
	return m_variableInfoTop;
}

// ----------------------------------------------------------------------

void VariableSetView::OnDestroy() 
{
	NOT_NULL(m_variableInfo);

	//-- destroy our controls
	std::for_each(m_variableInfo->begin(), m_variableInfo->end(), PointerDeleter());
	delete m_variableInfo;
	m_variableInfo = 0;

	//-- call base class functionality
	CScrollView::OnDestroy();
}

// ----------------------------------------------------------------------

void VariableSetView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	NOT_NULL(m_variableInfo);

	if (!pScrollBar || (pScrollBar->GetRuntimeClass() != RUNTIME_CLASS(CSliderCtrl)))
	{
		//-- not one of our slider bars, let the scroll view handle it
		CScrollView::OnHScroll(nSBCode, nPos, pScrollBar);
	}
	else
	{
		//-- this is one of our slider bars.
		const int controlId = pScrollBar->GetDlgCtrlID();
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, controlId, static_cast<int>(m_variableInfo->size()));

		//-- tell variable info to update the appropriate variable set
		VariableInfo &variableInfo = *NON_NULL((*m_variableInfo)[static_cast<size_t>(controlId)]);
		variableInfo.saveControlToVariable();
	}

	InvalidateRect(NULL);
}

// ----------------------------------------------------------------------

BOOL VariableSetView::OnEraseBkgnd(CDC* pDC) 
{
	//-- fill the view with the dialog box color
	CRect rect(0,0,m_viewWidth, m_viewHeight);
	FillRect(pDC->m_hDC, rect, m_eraseBrush);

	//-- indicate we erased
	return TRUE;
}

// ----------------------------------------------------------------------

void VariableSetView::OnSize(UINT nType, int cx, int cy) 
{
	m_viewWidth  = cx;
	m_viewHeight = cy;	

	CScrollView::OnSize(nType, cx, cy);
}

// ======================================================================

void VariableSetView::variableIteratorCallback(const std::string &fullVariablePathName, CustomizationVariable * /* customizationVariable */, void *context)
{
	//-- convert context into VariableSetView pointer
	NOT_NULL(context);
	VariableSetView *const view = static_cast<VariableSetView*>(context);

	view->m_variableNameSet->insert(fullVariablePathName);
}

// ======================================================================

void VariableSetView::addVariable(const std::string &fullVariablePathName, CustomizationVariable *customizationVariable)
{
	//-- convert customization variable into an RangedIntCustomizationVariable
	NOT_NULL(customizationVariable);

	RangedIntCustomizationVariable *const rangeVariable = dynamic_cast<RangedIntCustomizationVariable*>(customizationVariable);
	if (!rangeVariable)
	{
		WARNING(true, ("customization variable [%s]: skipped since not an int range.\n", fullVariablePathName.c_str()));
		return;
	}

	//-- add a VariableInfo for the variable
	m_variableInfo->push_back(new VariableInfo(*rangeVariable, fullVariablePathName, m_controlId, this, ms_leftViewBorder, m_variableInfoTop, ms_sliderHeight, ms_sliderWidth));

	//-- increment per-variable variables
	++m_controlId;
	m_variableInfoTop += ms_variableInfoHeight + ms_variableInfoSeparation;
}

// ======================================================================
