// ======================================================================
//
// ConversationBranchView.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ConversationBranchView_H
#define INCLUDED_ConversationBranchView_H

// ======================================================================

#include "Resource.h"
#include "SwgConversationEditorDoc.h"

// ======================================================================

class ConversationBranchView : public CFormView
{
protected:

	ConversationBranchView();           
	DECLARE_DYNCREATE(ConversationBranchView)

public:

	//{{AFX_DATA(ConversationBranchView)
	enum { IDD = IDD_BRANCHVIEW };
	CComboBox	m_tokenTOCtrl;
	CComboBox	m_tokenDICtrl;
	CComboBox	m_tokenDFCtrl;
	CComboBox	m_playerAnimationCtrl;
	CComboBox	m_npcAnimationCtrl;
	CButton	m_debugCtrl;
	CComboBox	m_linkCtrl;
	CComboBox	m_labelCtrl;
	CComboBox	m_conditionCtrl;
	CComboBox	m_actionCtrl;
	CEdit	m_notesCtrl;
	CEdit	m_textCtrl;
	BOOL	m_negateCondition;
	BOOL	m_debug;
	BOOL	m_useProsePackage;
	BOOL	m_groupEcho;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(ConversationBranchView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

protected:

	virtual ~ConversationBranchView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(ConversationBranchView)
	afx_msg void OnChangeEditNotes();
	afx_msg void OnChangeEditText();
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnEditUndo();
	afx_msg void OnSelchangeComboAction();
	afx_msg void OnSelchangeComboCondition();
	afx_msg void OnCheckNegatecondition();
	afx_msg void OnCheckDebug();
	afx_msg void OnCheckGroupEcho();
	afx_msg void OnSelchangeComboLabel();
	afx_msg void OnSelchangeComboLink();
	afx_msg void OnSelchangeComboNpcanimation();
	afx_msg void OnSelchangeComboPlayeranimation();
	afx_msg void OnCheckUseprosepackage();
	afx_msg void OnSelchangeComboTokendf();
	afx_msg void OnSelchangeComboTokendi();
	afx_msg void OnSelchangeComboTokento();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void setItemData (SwgConversationEditorDoc::ItemData * itemData);

private:

	SwgConversationEditorDoc::ItemData * m_itemData;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
