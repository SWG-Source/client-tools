//
// IffDoc.h
// asommers 6-12-99
//
// copyright 1999, bootprint entertainment
//

//-------------------------------------------------------------------

#ifndef IFFDOC_H
#define IFFDOC_H

//-------------------------------------------------------------------

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/ArrayList.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class Iff;

//-------------------------------------------------------------------

class IffDoc : public CDocument
{
public:

	class IffNode
	{
	public:

		enum Type
		{
			T_form,
			T_chunk,
		};

	public:

		Type      type;

		//--
		Tag       nameTag;
		char      name [5];

		//-- only valid for chunks
		void*     data;
		int       length;

		//-- only valid for forms
		IffNode** list;
		int       listCount;

		//-- 
		HTREEITEM handle;

	public:

		IffNode (void);
		~IffNode (void);

		void fill (Iff& iff);
		void dump (int indent, FILE * file) const;
	};

private:

	ArrayList<IffNode*> iffTreeList;

protected:

	IffDoc (void);
	DECLARE_DYNCREATE(IffDoc)

	//{{AFX_VIRTUAL(IffDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

	void           realFillTreeCtrl (CTreeCtrl& treeCtrl, IffDoc::IffNode* node, int currentLevel, HTREEITEM currentHandle);
	const IffNode* findNode (const IffNode* node, HTREEITEM handle) const;

public:

	virtual ~IffDoc (void);

	int            getCount (void) const;
	const IffNode* getIffTree (int index) const;
	void           fillTreeCtrl (CTreeCtrl& treeCtrl);
	const IffNode* findNode (HTREEITEM handle) const;

	void           dump (char const * fileName) const;

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(IffDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

inline int IffDoc::getCount (void) const
{
	return iffTreeList.getNumberOfElements ();
}

inline const IffDoc::IffNode* IffDoc::getIffTree (int index) const
{
	return iffTreeList [index];
}

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

#endif
