// ======================================================================
//
// DialogAddSkeleton.h
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#if !defined(AFX_DIALOGADDSKELETON_H__98CB1670_EBC5_40ED_8E12_5ECCAF933BD4__INCLUDED_)
#define AFX_DIALOGADDSKELETON_H__98CB1670_EBC5_40ED_8E12_5ECCAF933BD4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

// ======================================================================

#include "resource.h"

class BasicSkeletonTemplate;
class CrcLowerString;
class SkeletonTemplate;

// ======================================================================

class DialogAddSkeleton : public CDialog
{
public:

	typedef stdvector<const CrcLowerString*>::fwd         CrcLowerStringVector;
	typedef stdvector<const SkeletonTemplate*>::fwd       SkeletonTemplateVector;
	typedef stdvector<const BasicSkeletonTemplate*>::fwd  BasicSkeletonTemplateVector;

public:

	explicit DialogAddSkeleton(const SkeletonTemplateVector &skeletonTemplates, CWnd* pParent = NULL);
	~DialogAddSkeleton();

	const CrcLowerString &getAttachmentName() const;

	//lint -save -e1925 // public data members
	//{{AFX_DATA(DialogAddSkeleton)
	enum { IDD = IDD_ADD_SKELETON_TEMPLATE };
	CEdit	m_filePathEdit;
	CListBox	m_joints;
	CString	m_filePath;
	//}}AFX_DATA
	//lint -restore

	//{{AFX_VIRTUAL(DialogAddSkeleton)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogAddSkeleton)
	afx_msg void OnButtonBrowse();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	static void fetchLowestLodBasicSkeletonTemplates(const SkeletonTemplateVector &skeletonTemplates, BasicSkeletonTemplateVector &basicSkeletonTemplates);

private:

	// disabled
	DialogAddSkeleton();
	DialogAddSkeleton(const DialogAddSkeleton&);
	DialogAddSkeleton &operator =(const DialogAddSkeleton&);

private:

	CrcLowerStringVector *m_jointNames;
	CrcLowerString       *m_attachmentName;

};

// ======================================================================

inline const CrcLowerString &DialogAddSkeleton::getAttachmentName() const
{
	return *NON_NULL(m_attachmentName);
}

// ======================================================================

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
//}}AFX_INSERT_LOCATION

// ======================================================================

#endif
