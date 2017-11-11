// ======================================================================
//
// SwgSpaceQuestEditorDoc.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgSpaceQuestEditorDoc_H
#define INCLUDED_SwgSpaceQuestEditorDoc_H

// ======================================================================

#include "SpaceQuest.h"

// ======================================================================

class SwgSpaceQuestEditorDoc : public CDocument
{
public:

	enum Hint
	{
		H_unknown,
		H_aboutToSave
	};

public:

	virtual ~SwgSpaceQuestEditorDoc();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_VIRTUAL(SwgSpaceQuestEditorDoc)
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void SetModifiedFlag(BOOL bModified);
	//}}AFX_VIRTUAL

	CString const & getMissionTemplateType() const;

	void scan(bool showHeader);
	void save();
	void edit(bool interactive) const;

protected:

	SwgSpaceQuestEditorDoc();
	DECLARE_DYNCREATE(SwgSpaceQuestEditorDoc)

	//{{AFX_MSG(SwgSpaceQuestEditorDoc)
	afx_msg void OnButtonP4edit();
	afx_msg void OnButtonScan();
	afx_msg void OnButtonInformation();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	SpaceQuest & getSpaceQuest();
	SpaceQuest const & getSpaceQuest() const;

private:

	CString m_missionTemplateType;
	SpaceQuest m_spaceQuest;
};

// ======================================================================

inline SpaceQuest & SwgSpaceQuestEditorDoc::getSpaceQuest()
{
	return m_spaceQuest;
}

// ----------------------------------------------------------------------

inline SpaceQuest const & SwgSpaceQuestEditorDoc::getSpaceQuest() const
{
	return m_spaceQuest;
}

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
