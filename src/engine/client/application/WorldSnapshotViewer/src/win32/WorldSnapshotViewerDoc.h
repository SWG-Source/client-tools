//===================================================================
//
// WorldSnapshotViewerDoc.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_WorldSnapshotViewerDoc_H
#define INCLUDED_WorldSnapshotViewerDoc_H

//===================================================================

#include "sharedMath/Vector.h"

class WorldSnapshotReaderWriter;

//===================================================================

class WorldSnapshotViewerDoc : public CDocument
{
protected:

	WorldSnapshotViewerDoc (void);
	DECLARE_DYNCREATE(WorldSnapshotViewerDoc)

	//{{AFX_VIRTUAL(WorldSnapshotViewerDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

	const WorldSnapshotReaderWriter* getWorldSnapshotReaderWriter () const;

	void setSelectedPosition_p (const Vector& selectedPosition_p);
	const Vector& getSelectedPosition_p () const;
	void setSelectedPosition_w (const Vector& selectedPosition_w);
	const Vector& getSelectedPosition_w () const;

	void setNumberOfNodes (int numberOfNodes);
	int getNumberOfNodes () const;

	void setTotalNumberOfNodes (int totalNumberOfNodes);
	int getTotalNumberOfNodes () const;

public:

	virtual ~WorldSnapshotViewerDoc (void);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(WorldSnapshotViewerDoc)
	afx_msg void OnButtonExport();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	WorldSnapshotReaderWriter* m_worldSnapshotReader;
	Vector m_selectedPosition_p;
	Vector m_selectedPosition_w;
	int    m_numberOfNodes;
	int    m_totalNumberOfNodes;
};

//===================================================================

inline const WorldSnapshotReaderWriter* WorldSnapshotViewerDoc::getWorldSnapshotReaderWriter () const
{
	return m_worldSnapshotReader;
}

//===================================================================

//{{AFX_INSERT_LOCATION}}

//===================================================================

#endif
