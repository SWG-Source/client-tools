//===================================================================
//
// DialogControlPointEditor.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_DialogControlPointEditor_H
#define INCLUDED_DialogControlPointEditor_H

//===================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/ArrayList.h"
#include "Resource.h"
#include "sharedMath/Vector2d.h"

//===================================================================

class DialogControlPointEditor : public CDialog
{
public:

	explicit DialogControlPointEditor (const ArrayList<Vector2d>& pointList);
	explicit DialogControlPointEditor (const ArrayList<Vector2d>& pointList, const ArrayList<float>& heightList);

	const ArrayList<Vector2d>& getPointList () const;
	const ArrayList<float>& getHeightList () const;

	//{{AFX_DATA(DialogControlPointEditor)
	enum { IDD = IDD_DIALOG_CONTROL_POINT_EDITOR };
	CListCtrl	m_listCtrl;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogControlPointEditor)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogControlPointEditor)
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonDelete();
	afx_msg void OnButtonDown();
	afx_msg void OnButtonImport();
	afx_msg void OnButtonUp();
	afx_msg void OnDblclkListControlPoints(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonImportAppend();
	afx_msg void OnButtonImportClear();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	ArrayList<Vector2d> m_pointList;
	ArrayList<float> m_heightList;
	bool m_hasHeights;
};

//===================================================================

inline const ArrayList<Vector2d>& DialogControlPointEditor::getPointList () const
{
	return m_pointList;
}

//-------------------------------------------------------------------

inline const ArrayList<float>& DialogControlPointEditor::getHeightList () const
{
	return m_heightList;
}

//===================================================================


//{{AFX_INSERT_LOCATION}}

//===================================================================

#endif 
