// ======================================================================
//
// AnimationDialog.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "FirstViewer.h"
#include "viewer.h"

#include "AnimationDialog.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "MainFrm.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/Object.h"
#include "ViewerDoc.h"

// ======================================================================

static void AddItem (CListCtrl& m_listCtrl, char key, bool isPlaying, const char* name, real weight, real timescale, bool isLooping)
{
	int handle;

	char buffer [100];

	//-- add key
	buffer [0] = key;
	buffer [1] = 0;

	handle = m_listCtrl.InsertItem (0, buffer);

	//-- add isplaying
	m_listCtrl.SetItem (handle, 1, LVIF_TEXT, isPlaying ? "yes" : "no", 0, 0, 0, 0);

	//-- add name
	m_listCtrl.SetItem (handle, 2, LVIF_TEXT, name, 0, 0, 0, 0);

	//-- 
	sprintf (buffer, "%1.2f", weight);
	m_listCtrl.SetItem (handle, 3, LVIF_TEXT, buffer, 0, 0, 0, 0);

	//-- add timescale
	sprintf (buffer, "%1.2f", timescale);
	m_listCtrl.SetItem (handle, 4, LVIF_TEXT, buffer, 0, 0, 0, 0);

	//-- add looping
	m_listCtrl.SetItem (handle, 5, LVIF_TEXT, isLooping ? "yes" : "no", 0, 0, 0, 0);
}

// ----------------------------------------------------------------------

static bool Find (const CTreeCtrl& m_treeCtrl, HTREEITEM current, const char* text, const int value, HTREEITEM* result)
{
	if (!result || !current)
		return false;

	//-- is it me
	CString currentText = m_treeCtrl.GetItemText (current);
	int     currentData = static_cast<int> (m_treeCtrl.GetItemData (current));
	if (currentText.Compare (text) == 0 && currentData == value)
	{
		*result = current;
		return true;
	}

	//-- search my children
	HTREEITEM item = m_treeCtrl.GetChildItem (current);

	if (item)
	{
		do
		{
			if (Find (m_treeCtrl, item, text, value, result))
				return true;

			item = m_treeCtrl.GetNextSiblingItem (item);
		}
		while (item);
	}

	return false;
}

// ======================================================================

BEGIN_MESSAGE_MAP(AnimationDialog, CDialog)
	//{{AFX_MSG_MAP(AnimationDialog)
	ON_WM_SIZE()
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST_ANIMATIONS, OnKeydownListAnimations)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ANIMATIONS, OnDblclkListAnimations)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_SKELETON, OnSelchangedTreeSkeleton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================
// class AnimationDialog
// ======================================================================

AnimationDialog::AnimationDialog(CWnd* pParent /*=NULL*/)
	: CDialog(AnimationDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(AnimationDialog)
	//}}AFX_DATA_INIT

	initialized = FALSE;
}

// ----------------------------------------------------------------------

void AnimationDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AnimationDialog)
	DDX_Control(pDX, IDC_TREE_SKELETON, m_treeCtrl);
	DDX_Control(pDX, IDC_LIST_ANIMATIONS, m_listCtrl);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

void AnimationDialog::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
	if(!initialized)
		return;

	//-- place treectrl
	CRect sz;
	GetClientRect(sz);
	sz.DeflateRect(5,5);
	sz.right /= 2;
	sz.right -= 2;
	m_treeCtrl.MoveWindow(sz);

	//-- place listctrl
	GetClientRect(sz);
	sz.DeflateRect(5,5);

	int left = sz.right;
	left /= 2;
	left += 2;
	sz.left = left;
	m_listCtrl.MoveWindow(sz);
}

// ----------------------------------------------------------------------

BOOL AnimationDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CRect sz;
	GetClientRect(sz);
	sz.DeflateRect(5,5);
	
	initialized = TRUE;

	// TODO: Add extra initialization here
	LV_COLUMN lvc;
	lvc.mask    = LVCF_TEXT | LVCF_WIDTH;

	lvc.cx      = 75;
	lvc.pszText = "Key";
	m_listCtrl.InsertColumn (0, &lvc);

	lvc.cx      = 75;
	lvc.pszText = "Used";
	m_listCtrl.InsertColumn (1, &lvc);

	lvc.cx      = 300;
	lvc.pszText = "Animation Filename";
	m_listCtrl.InsertColumn (2, &lvc);

	lvc.cx      = 75;
	lvc.pszText = "Weight";
	m_listCtrl.InsertColumn (3, &lvc);

	lvc.cx      = 75;
	lvc.pszText = "Timescale";
	m_listCtrl.InsertColumn (4, &lvc);

	lvc.cx      = 75;
	lvc.pszText = "Looping";
	m_listCtrl.InsertColumn (5, &lvc);

	//-- get bone structure

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

void AnimationDialog::updateAnimationView (const Appearance *appearance)
{
	UNREF(appearance);

	//-- update skeleton tree view
	populateSkeletonTransformData (appearance);

	//-- update animation list view
	m_listCtrl.DeleteAllItems ();

	UpdateData (false);
	Invalidate ();
}

// ----------------------------------------------------------------------

void AnimationDialog::updateAnimationView (const Appearance *appearance, const SkeletalAnimationKey::Map& skeletalAnimationKeyMap)
{
	//-- update skeleton tree view
	populateSkeletonTransformData (appearance);

	//-- update animation list view
	populateAnimationListData (skeletalAnimationKeyMap);

	UpdateData (false);
	Invalidate ();
}

// ----------------------------------------------------------------------

void AnimationDialog::OnKeydownListAnimations(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN* pLVKeyDown = (LV_KEYDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here

	if (pLVKeyDown->wVKey == VK_INSERT)
	{
		//-- get the document
		CViewerDoc* doc = static_cast<CViewerDoc*> (GetMainFrame ()->GetActiveFrame ()->GetActiveDocument ());

		//-- insert new item
		doc->newAnimation ();
	}
	else
	{
		if (pLVKeyDown->wVKey == VK_DELETE)
		{		
			POSITION pos = m_listCtrl.GetFirstSelectedItemPosition();
			if (pos != NULL)
			{
				int nItem = m_listCtrl.GetNextSelectedItem(pos);
				CString item = m_listCtrl.GetItemText (nItem, 0);

				//-- delete selected item
				CViewerDoc* doc = static_cast<CViewerDoc*> (GetMainFrame ()->GetActiveFrame ()->GetActiveDocument ());
				doc->removeAnimation (item [0]);
			}
		}
	}

	*pResult = 0;
}

// ----------------------------------------------------------------------

void AnimationDialog::OnDblclkListAnimations(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREF (pNMHDR);

	// TODO: Add your control notification handler code here
	POSITION pos = m_listCtrl.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_listCtrl.GetNextSelectedItem(pos);
		CString item = m_listCtrl.GetItemText (nItem, 0);

		//-- edit selected item
		if (GetMainFrame () && GetMainFrame ()->GetActiveFrame ())
		{
			CViewerDoc* doc = static_cast<CViewerDoc*> (GetMainFrame ()->GetActiveFrame ()->GetActiveDocument ());
			doc->editAnimation (item [0]);
		}
	}
	
	*pResult = 0;
}

// ----------------------------------------------------------------------

CrcLowerString AnimationDialog::getSelectedTransformName () const
{
	HTREEITEM selection = m_treeCtrl.GetSelectedItem ();
	if (selection)
		return CrcLowerString(m_treeCtrl.GetItemText (selection));
	else
		return CrcLowerString("");
}

// ----------------------------------------------------------------------

void AnimationDialog::OnSelchangedTreeSkeleton(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREF (pNMHDR);

	//-- -TRF- fix, removed granny
#if 0
	if (GetMainFrame () && GetMainFrame ()->GetActiveFrame () && GetMainFrame ()->GetActiveFrame ()->GetActiveDocument ())
	{
		CViewerDoc* doc = static_cast<CViewerDoc*> (GetMainFrame ()->GetActiveFrame ()->GetActiveDocument ());
		if (doc->getObject ())
		{
			const SkeletalAppearance* sa = dynamic_cast<const SkeletalAppearance*> (doc->getObject ()->getAppearance ());

			if (sa)
				doc->setSelectedBoneIndex (sa->getRootSequence ().getBoneIndex (m_treeCtrl.GetItemText (m_treeCtrl.GetSelectedItem ())));
		}
	}
#endif

	*pResult = 0;
}

// ======================================================================

void AnimationDialog::populateAnimationListData (const SkeletalAnimationKey::Map& skeletalAnimationKeyMap)
{
	IGNORE_RETURN (m_listCtrl.DeleteAllItems ());

	for (SkeletalAnimationKey::Map::const_iterator iter = skeletalAnimationKeyMap.begin (); iter != skeletalAnimationKeyMap.end(); ++iter)
	{
		const SkeletalAnimationKey* const mak = (*iter).second;

		AddItem (m_listCtrl, mak->key, false, mak->filename, mak->weight, mak->timeScale, mak->isLooping);
	}
}

// ----------------------------------------------------------------------

void AnimationDialog::populateSkeletonTransformData (const Appearance *appearance)
{
	IGNORE_RETURN (m_treeCtrl.DeleteAllItems ());

	m_treeCtrl.InsertItem ("Skeleton Transforms");
	m_treeCtrl.Expand (m_treeCtrl.GetRootItem (), TVE_EXPAND);

	if (!appearance)
		return;

	const SkeletalAppearance2 *const skeletalAppearance = dynamic_cast<const SkeletalAppearance2*>(appearance);
	if (!skeletalAppearance)
		return;

	//-- grab current LOD skeleton
	const Skeleton *skeleton = skeletalAppearance->getDisplayLodSkeleton();
	if (skeleton)
	{
		const int transformCount = skeleton->getTransformCount();
		for (int i = 0; i < transformCount; ++i)
		{
			HTREEITEM root = m_treeCtrl.GetRootItem ();
			
			int parentTransformIndex = skeleton->getParentTransformIndex (i);
			if (parentTransformIndex >= 0)
			{
				const CrcString& parentTransformName = skeleton->getTransformName (parentTransformIndex);
				HTREEITEM        result              = 0;

				if (Find (m_treeCtrl, root, parentTransformName.getString (), parentTransformIndex, &result))
					root = result;
			}

			HTREEITEM item = m_treeCtrl.InsertItem (skeleton->getTransformName(i).getString(), root);
			m_treeCtrl.SetItemData (item, static_cast<DWORD> (i));
			m_treeCtrl.Expand (root, TVE_EXPAND);
		}
	}

	m_treeCtrl.SelectItem (m_treeCtrl.GetRootItem ());
	m_treeCtrl.EnsureVisible (m_treeCtrl.GetRootItem ());
	m_treeCtrl.Expand (m_treeCtrl.GetRootItem (), TVE_EXPAND);
}

// ======================================================================
