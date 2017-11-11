// ======================================================================
//
// SwgContentBuilderDlg.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgContentBuilderDlg_H
#define INCLUDED_SwgContentBuilderDlg_H

// ======================================================================

#include "Conversation.h"
#include "Resource.h"

class Quest;

#include <vector>

// ======================================================================

class SwgContentBuilderDlg : public CDialog
{
public:

	SwgContentBuilderDlg(CWnd* pParent = NULL);

	//{{AFX_DATA(SwgContentBuilderDlg)
	enum { IDD = IDD_SWGCONTENTBUILDER_DIALOG };
	CEdit	m_editCtrlQuestGiverName;
	CButton	m_buttonBrowseQuestGiver;
	CComboBox	m_comboGatingFaction;
	CComboBox	m_comboQuestType;
	CComboBox	m_comboQuestGiverConversation;
	CComboBox	m_comboQuestNpcConversation;
	CComboBox	m_comboLocationPlanet;
	CString	m_questId;
	float	m_locationX;
	float	m_locationY;
	float	m_locationZ;
	CString	m_overallObjvar;
	CString	m_tempObjvar;
	CString	m_retrieveObjectTemplate;
	CString	m_deliverObjectTemplate;
	CString	m_playerScript;
	int		m_questLeg;
	CString	m_questGiverEntry;
	CString	m_questNpcEntry;
	CString	m_questNpcName;
	CString	m_questNpcScript;
	CString	m_spawnerObjectTemplateName;
	CString	m_dataTableName;
	CString	m_conversationName;
	int		m_gatingFactionAmount;
	CString	m_gatingObjectTemplate;
	CString	m_gatingObjvar;
	CString	m_questGiverName;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(SwgContentBuilderDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	HICON m_hIcon;

	//{{AFX_MSG(SwgContentBuilderDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonBrowseobjectdeliver();
	afx_msg void OnButtonBrowseobjectquestgiver();
	afx_msg void OnButtonBrowseobjectquestnpc();
	afx_msg void OnButtonBrowseobjectretrieve();
	afx_msg void OnButtonDelete();
	afx_msg void OnButtonEditconvoquestgiver();
	afx_msg void OnButtonEditconvoquestnpc();
	afx_msg void OnButtonEditextranpcs();
	afx_msg void OnButtonEditrewards();
	afx_msg void OnButtonLoad();
	afx_msg void OnButtonNew();
	afx_msg void OnButtonNext();
	afx_msg void OnButtonPrevious();
	afx_msg void OnButtonSave();
	afx_msg void OnButtonInsert();
	afx_msg void OnButtonAppend();
	afx_msg void OnButtonSaveas();
	afx_msg void OnButtonBrowsegatingobject();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	void updateQuestId ();
	void updateCurrentQuest (bool updateData);
	void save (bool verify);
	void createNames (CString const & pathName);

private:

	int m_currentQuest;

	typedef std::vector<Quest*> QuestList;
	QuestList m_questList;

	Conversation m_conversation;

	CString m_baseName;
	CString m_spawnerObjectTemplateFullName;
	CString m_dataTableFullName;
	CString m_conversationFullName;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 

