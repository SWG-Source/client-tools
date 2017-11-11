// DialogProgress.h : header file
// CG: This file was added by the Progress Dialog component

/////////////////////////////////////////////////////////////////////////////
// DialogProgress dialog

#ifndef __DIALOGPROGRESS_H__
#define __DIALOGPROGRESS_H__

#include "resource.h"

class DialogProgress : public CDialog
{
// Construction / Destruction
public:
    DialogProgress(UINT nCaptionID = 0);   // standard constructor
    ~DialogProgress();

    BOOL Create(CWnd *pParent=NULL);

    // Checking for Cancel button
    BOOL CheckCancelButton();
    // Progress Dialog manipulation
    void SetStatus(LPCTSTR lpszMessage);
    void SetRange(int nLower,int nUpper);
    int  SetStep(int nStep);
    int  SetPos(int nPos);
    int  OffsetPos(int nPos);
    int  StepIt();
        
// Dialog Data
    //{{AFX_DATA(DialogProgress)
    enum { IDD = CG_IDD_PROGRESS };
    CProgressCtrl	m_Progress;
    //}}AFX_DATA

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(DialogProgress)
    public:
    virtual BOOL DestroyWindow();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:
	UINT m_nCaptionID;
    int m_nLower;
    int m_nUpper;
    int m_nStep;
    
    BOOL m_bCancel;
    BOOL m_bParentDisabled;

	CButton m_cancelButton;

    void ReEnableParent();

    virtual void OnCancel();
    virtual void OnOK() {}; 
    void UpdatePercent(int nCurrent);
    void PumpMessages();

    // Generated message map functions
    //{{AFX_MSG(DialogProgress)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif // __DIALOGPROGRESS_H__
