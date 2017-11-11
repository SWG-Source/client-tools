// ======================================================================
//
// SwgSpaceQuestEditorView.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgSpaceQuestEditorView_H
#define INCLUDED_SwgSpaceQuestEditorView_H

// ======================================================================

#include "PropertyListCtrl.h"

// ======================================================================

class SwgSpaceQuestEditorView : public CView
{
protected: 

	SwgSpaceQuestEditorView();
	DECLARE_DYNCREATE(SwgSpaceQuestEditorView)

	void addStringId(CString const & stringId);
	void removeStringId(CString const & stringId);

public:

	//{{AFX_VIRTUAL(SwgSpaceQuestEditorView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

public:

	virtual ~SwgSpaceQuestEditorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	void onPropertyChanged(UINT hiControlId_loItemChanged, LONG lPropertyType) const;

	//{{AFX_MSG(SwgSpaceQuestEditorView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnEditUndo();
	afx_msg void OnEditStringidlist();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	class ParentInterface : public PropertyListCtrl::ParentInterface
	{
	public:

		ParentInterface(SwgSpaceQuestEditorView const * editorView);
		virtual void onPropertyChanged(UINT hiControlId_loItemChanged, LONG lPropertyType) const;

	private:

		SwgSpaceQuestEditorView const * const m_editorView;
	};

	friend class ParentInterface;

private:

	bool m_initialized;
	PropertyListCtrl m_properties;
	PropertyListCtrl m_strings;
	PropertyListCtrl m_questStrings;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif
