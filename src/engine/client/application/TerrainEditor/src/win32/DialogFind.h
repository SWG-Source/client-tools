//
// DialogFind.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_DialogFind_H
#define INCLUDED_DialogFind_H

//-------------------------------------------------------------------

#include "resource.h"
#include "TerrainGeneratorHelper.h"

//-------------------------------------------------------------------

class DialogFind : public CDialog
{
private:

	ArrayList<TerrainGeneratorHelper::LayerItemQueryType>& query;

private:

	//{{AFX_DATA(DialogFind)
	enum { IDD = IDD_DIALOG_FIND };
	CListBox	m_source;
	CListBox	m_destination;
	BOOL	m_clear;
	CString	m_name;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogFind)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	DialogFind (void);

protected:

	//{{AFX_MSG(DialogFind)
	afx_msg void OnDblclkListDestination();
	afx_msg void OnDblclkListSource();
	afx_msg void OnTodestination();
	afx_msg void OnTosource();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnCheckClear();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:

	explicit DialogFind(ArrayList<TerrainGeneratorHelper::LayerItemQueryType>& newQuery, CWnd* pParent = NULL);   

	bool getClear (void) const;
	const CString& getName () const;
};

//-------------------------------------------------------------------

inline bool DialogFind::getClear (void) const
{
	return m_clear == TRUE;
}

//-------------------------------------------------------------------

inline const CString& DialogFind::getName () const
{
	return m_name;
}

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
