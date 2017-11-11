// ======================================================================
//
// AnimationDialog.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#if !defined(AFX_ANIMATIONDIALOG_H__48A2B067_049E_4DAC_8299_DCF70993A219__INCLUDED_)
#define AFX_ANIMATIONDIALOG_H__48A2B067_049E_4DAC_8299_DCF70993A219__INCLUDED_

// ======================================================================

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ======================================================================

#include "SkeletalAnimationKey.h"

class Appearance;
class CrcLowerString;

// ======================================================================

class AnimationDialog : public CDialog
{
public:

	explicit AnimationDialog(CWnd* pParent = NULL);

	void updateAnimationView (const Appearance *appearance);
	void updateAnimationView (const Appearance *appearance, const SkeletalAnimationKey::Map& skeletalAnimationKeyMap);

	CrcLowerString  getSelectedTransformName () const;

public:

// Dialog Data
	//{{AFX_DATA(AnimationDialog)
	enum { IDD = IDD_ANIMATION };
	CTreeCtrl	m_treeCtrl;
	CListCtrl	m_listCtrl;
	//}}AFX_DATA
protected:

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AnimationDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	//{{AFX_MSG(AnimationDialog)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnKeydownListAnimations(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkListAnimations(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangedTreeSkeleton(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	void populateAnimationListData (const SkeletalAnimationKey::Map& skeletalAnimationKeyMap);
	void populateSkeletonTransformData (const Appearance *appearance);

private:

	BOOL initialized;

};

//{{AFX_INSERT_LOCATION}}
//}}AFX_INSERT_LOCATION

// ======================================================================

#endif
