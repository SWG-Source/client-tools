//
// LayerFrame.h
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#ifndef LAYERFRAME_H
#define LAYERFRAME_H

//-------------------------------------------------------------------

#include "sharedTerrain/TerrainGenerator.h"

//-------------------------------------------------------------------

class LayerFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(LayerFrame)

protected:

	LayerFrame (void);           // protected constructor used by dynamic creation
	virtual ~LayerFrame (void);

private:

	CString     m_windowName;
	CToolBar    m_wndToolBar;

public:

	void selectLayerItem (const TerrainGenerator::LayerItem* layerItem);

	//{{AFX_VIRTUAL(LayerFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(LayerFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif

