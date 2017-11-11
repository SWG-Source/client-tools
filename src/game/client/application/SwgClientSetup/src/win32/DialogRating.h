// ======================================================================
//
// DialogRating.cpp
// asommers
// rsitton
//
// copyright 2005, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogRating_H
#define INCLUDED_DialogRating_H

// ======================================================================

#include "TabPoll.h"

// ======================================================================

class DialogRating : public CDialog
{
public:
	DialogRating(CStdioFile & file, CString const & stationId, CString const & fromAddress);

	void sendInformationToServer();
	bool isValid() const;
	float getFrequency() const;

	//{{AFX_DATA(DialogRating)
	enum { IDD = IDD_DIALOG_RATING };
	CButton	m_buttonFinished;
	CButton	m_buttonNotInterested;
	CStatic	m_description;
	TabPoll	m_tabPoll;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogRating)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogRating)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonPrev();
	afx_msg void OnButtonNext();
	afx_msg void OnButtonNotInterested();
	afx_msg void OnButtonComplete();
	afx_msg void OnSelchangeTabPoll(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	DialogRating();
	
	void updateExitButton();

	CString m_stationId;
	CString m_fromEmailAddress;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
