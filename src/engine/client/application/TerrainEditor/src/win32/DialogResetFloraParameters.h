//===================================================================
//
// DialogResetFloraParameters.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_DialogResetFloraParameters_H
#define INCLUDED_DialogResetFloraParameters_H

//===================================================================

#include "NumberEdit.h"
#include "Resource.h"

//===================================================================

class DialogResetFloraParameters : public CDialog
{
private:

	bool m_radial;

public:

	explicit DialogResetFloraParameters(bool radial, CWnd* pParent = NULL);   

	//{{AFX_DATA(DialogResetFloraParameters)
	enum { IDD = IDD_DIALOG_FLORAGLOBALSWAY };
	NumberEdit	m_period;
	NumberEdit	m_displacement;
	BOOL	m_shouldSway;
	BOOL	m_alignToTerrain;
	BOOL	m_checkSway;
	BOOL	m_checkDisplacement;
	BOOL	m_checkPeriod;
	BOOL	m_checkAlign;
	//}}AFX_DATA

	float m_finalPeriod;
	float m_finalDisplacement;

	//{{AFX_VIRTUAL(DialogResetFloraParameters)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogResetFloraParameters)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//===================================================================

//{{AFX_INSERT_LOCATION}}

//===================================================================

#endif 
