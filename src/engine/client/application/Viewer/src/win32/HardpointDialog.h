// ======================================================================
//
// HardpointDialog.h
//
// copyright 2005, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_HardpointDialog_H
#define INCLUDED_HardpointDialog_H

// ======================================================================

#if _MSC_VER > 1000
#pragma once
#endif

// ======================================================================

#include "resource.h"

#include <string>

class Object;
class CViewerDoc;

// ======================================================================

class HardpointDialog : public CDialog
{
public:
	HardpointDialog(CWnd* pParent = NULL);
	void updateHardpointTree(Object const * const object);
	void setViewerDoc(CViewerDoc * viewerDoc);
	CString getSelectedHardpoint() const;
	Object * getParentObject() const;

protected:
	void addObjectToTree(Object const * const object, HTREEITEM parent);

private:
	CViewerDoc * m_viewerDoc;
	CImageList m_imageList;

public:

	//{{AFX_DATA(HardpointDialog)
	enum { IDD = IDD_HARDPOINT_TREE };
	CTreeCtrl	m_hardpointTree;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(HardpointDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(HardpointDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnRclickHardpointTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif

// ======================================================================
