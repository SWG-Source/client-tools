// ======================================================================
//
// DialogObjectName_H.h
// mboudreaux
//
// copyright 2007, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogObjectName_H
#define INCLUDED_DialogObjectName_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class DialogObjectName : public CDialog
{
	DECLARE_DYNAMIC(DialogObjectName)

public:
	DialogObjectName(CWnd* pParent = NULL);   // standard constructor
	virtual ~DialogObjectName();
	
	CString const &	GetNameField() { return mNameText; }
// Dialog Data
	enum { IDD = IDD_DIALOG_OBJECTNAME };
private:
	CString mNameText;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonNameok();
	afx_msg void OnBnClickedButtonNamecancel();
};

#endif