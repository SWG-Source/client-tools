//
// ProfileView.h
// asommers 
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_ProfileView_H
#define INCLUDED_ProfileView_H

//-------------------------------------------------------------------

#include "resource.h"
#include "sharedTerrain/TerrainGenerator.h"
#include "TreeListFrame.h"

//-------------------------------------------------------------------

class ProfileView : public CFormView
{
private:

	CTLFrame   m_wndTreeList;
	CImageList m_cImageList;
	BOOL       m_TLInitialized;

	//{{AFX_DATA(CMainView)
	enum { IDD = IDD_TREELIST_VIEW };
	//}}AFX_DATA

	real      slowestTotalTime;
	real      slowestLayerTime;
	real      slowestOverheadTime;
	real      slowestBoundaryTime;
	real      slowestFilterTime;
	real      slowestAffectorTime;
	real      slowestSubLayerTime;

	HTREEITEM slowestTotal;
	HTREEITEM slowestLayer;
	HTREEITEM slowestOverhead;
	HTREEITEM slowestBoundary;
	HTREEITEM slowestFilter;
	HTREEITEM slowestAffector;
	HTREEITEM slowestSubLayer;

private:

	void updateTimes (const TerrainGenerator::Layer::ProfileData& profileData, HTREEITEM item);

	void update (const TerrainGenerator::Layer* layer, HTREEITEM root);

protected:

	ProfileView();
	DECLARE_DYNCREATE(ProfileView)

public:

	virtual ~ProfileView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void update (const TerrainGenerator* generator);

	//{{AFX_VIRTUAL(ProfileView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(ProfileView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
