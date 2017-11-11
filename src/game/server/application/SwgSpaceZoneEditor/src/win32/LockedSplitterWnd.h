// ======================================================================
//
// LockedSplitterWnd.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_LockedSplitterWnd_H
#define INCLUDED_LockedSplitterWnd_H

// ======================================================================

class SpaceZoneMapView;

// ======================================================================

class LockedSplitterWnd : public CSplitterWnd
{
public:

	LockedSplitterWnd();

	//{{AFX_VIRTUAL(LockedSplitterWnd)
	//}}AFX_VIRTUAL

public:

	virtual ~LockedSplitterWnd();

	int HitTest(CPoint pt) const;

protected:

	//{{AFX_MSG(LockedSplitterWnd)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

#endif 
