// ======================================================================
//
// SwgDraftSchematicEditorDoc.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgDraftSchematicEditorDoc_H
#define INCLUDED_SwgDraftSchematicEditorDoc_H

// ======================================================================

#include "DraftSchematic.h"

// ======================================================================

class SwgDraftSchematicEditorDoc : public CDocument
{
protected: 

	SwgDraftSchematicEditorDoc();
	DECLARE_DYNCREATE(SwgDraftSchematicEditorDoc)

public:

	//{{AFX_VIRTUAL(SwgDraftSchematicEditorDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void SetModifiedFlag(BOOL bModified);
	//}}AFX_VIRTUAL

public:

	virtual ~SwgDraftSchematicEditorDoc();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(SwgDraftSchematicEditorDoc)
	afx_msg void OnButtonCompile();
	afx_msg void OnButtonP4edit();
	afx_msg void OnButtonScan();
	afx_msg void OnButtonSavetest();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	enum Hint
	{
		H_nothing,
		H_nameChanged,
		H_slotChanged
	};

public:

	struct ItemData
	{
	public:

		enum Type
		{
			T_unknown,
			T_attribute,
			T_slot
		};

	public:

		Type m_type;

		union
		{
			DraftSchematic::Attribute * m_attribute;
			DraftSchematic::Slot * m_slot;
			void * m_void;
		};

		HTREEITEM m_treeItem;

	public:

		ItemData ();
		~ItemData ();

		CString getName () const;
	};

public:

	DraftSchematic & getDraftSchematic ();
	DraftSchematic const & getDraftSchematic () const;

	void updateTpfTabs (char const * pathName);

private:

	DraftSchematic m_draftSchematic;
};

// ======================================================================

inline DraftSchematic & SwgDraftSchematicEditorDoc::getDraftSchematic ()
{
	return m_draftSchematic;
}

// ----------------------------------------------------------------------

inline DraftSchematic const & SwgDraftSchematicEditorDoc::getDraftSchematic () const
{
	return m_draftSchematic;
}

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
