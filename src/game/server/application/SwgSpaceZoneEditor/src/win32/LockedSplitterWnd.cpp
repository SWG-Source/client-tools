// ======================================================================
//
// LockedSplitterWnd.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceZoneEditor.h"
#include "LockedSplitterWnd.h"

// ======================================================================

LockedSplitterWnd::LockedSplitterWnd()
{
	m_cxSplitter = 3;
	m_cySplitter = 3;
	m_cxBorderShare = 0;
	m_cyBorderShare = 0;
	m_cxSplitterGap = 3;
	m_cySplitterGap = 3;
}

// ----------------------------------------------------------------------

LockedSplitterWnd::~LockedSplitterWnd()
{
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(LockedSplitterWnd, CSplitterWnd)
	//{{AFX_MSG_MAP(LockedSplitterWnd)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

int LockedSplitterWnd::HitTest(CPoint const /*pt*/) const
{
	return 0;
}

// ======================================================================
