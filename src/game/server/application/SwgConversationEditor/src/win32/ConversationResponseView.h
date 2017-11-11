// ======================================================================
//
// ConversationResponseView.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ConversationResponseView_H
#define INCLUDED_ConversationResponseView_H

// ======================================================================

#include "Resource.h"
#include "SwgConversationEditorDoc.h"

// ======================================================================

class ConversationResponseView : public CFormView
{
protected:

	ConversationResponseView();           
	DECLARE_DYNCREATE(ConversationResponseView)

public:

	//{{AFX_DATA(ConversationResponseView)
	enum { IDD = IDD_RESPONSEVIEW };
	CComboBox	m_playerAnimationCtrl;
	CComboBox	m_npcAnimationCtrl;
	CButton	m_debugCtrl;
	CComboBox	m_linkCtrl;
	CComboBox	m_labelCtrl;
	CComboBox	m_conditionCtrl;
	CComboBox	m_actionCtrl;
	CEdit	m_textCtrl;
	CEdit	m_notesCtrl;
	BOOL	m_negateCondition;
	BOOL	m_debug;
	BOOL	m_groupEcho;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(ConversationResponseView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

protected:

	virtual ~ConversationResponseView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(ConversationResponseView)
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
