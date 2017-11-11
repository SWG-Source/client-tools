// ======================================================================
//
// AnimationTreeDialog.cpp
// Copyright 2006 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstViewer.h"
#include "AnimationTreeDialog.h"
#include "MainFrm.h"
#include "Viewer.h"
#include "ViewerDoc.h"
#include "ViewerView.h"
#include <vector>
#include <string>

//-------------------------------------------------------------------

namespace AnimationTreeDialogNamespace
{
	const char* ms_primarySkeletonName = "Primary Skeleton";
	const char* ms_secondarySkeletonName = "Secondary Skeleton";
	const char* ms_animPath = "appearance/animation/";
}
using namespace AnimationTreeDialogNamespace;

//-------------------------------------------------------------------

AnimationTreeDialog::AnimationTreeDialog(CWnd* pParent /*=NULL*/)
	: CDialog(AnimationTreeDialog::IDD, pParent),
	m_imageListSet (false),
	m_activeDocument (NULL)
{
	//{{AFX_DATA_INIT(AnimationTreeDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_initialized = FALSE;
}

//-------------------------------------------------------------------

void AnimationTreeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AnimationTreeDialog)
	DDX_Control(pDX, IDC_ANIMATION_TREE, m_treeCtrl);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(AnimationTreeDialog, CDialog)
	//{{AFX_MSG_MAP(AnimationTreeDialog)
	ON_WM_SIZE()
	ON_NOTIFY(NM_DBLCLK, IDC_ANIMATION_TREE, OnDblclkDirectoryTree)
	ON_NOTIFY(TVN_KEYDOWN, IDC_ANIMATION_TREE, OnKeydownDirectoryTree)
	ON_NOTIFY(NM_RCLICK, IDC_ANIMATION_TREE, OnRclickDirectoryTree)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_ANIMATION_TREE, OnItemexpandingDirectoryTree)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

static enum DirectoryIconList
{
	DIL_unknown,
	DIL_closedFolder,
	DIL_openFolder,
	DIL_apt,
	DIL_mesh,
	DIL_component,
	DIL_lod,
	DIL_sprite,
	DIL_particle,
	DIL_skeletal,
	DIL_shaderTemplate,
	DIL_dds,
	DIL_terrain,
	DIL_hardpointHierarchy
};

//-------------------------------------------------------------------

void AnimationTreeDialog::clear()
{
	m_treeCtrl.DeleteAllItems();
	m_activeDocument = static_cast<CViewerDoc*> (GetMainFrame ()->GetActiveFrame ()->GetActiveDocument ());
}

void AnimationTreeDialog::initTreeCtrl(CViewerDoc::AnimationFileList* animFileListPrimarySkel, CViewerDoc::AnimationFileList* animFileListSecondarySkel)
{
	CViewerDoc* currentDoc = static_cast<CViewerDoc*> (GetMainFrame ()->GetActiveFrame ()->GetActiveDocument ());
	if(currentDoc == m_activeDocument)
		return;

	clear();
	HTREEITEM primarySkeletonItem = m_treeCtrl.InsertItem(ms_primarySkeletonName,DIL_closedFolder, DIL_openFolder);
	HTREEITEM secondarySkeletonItem = m_treeCtrl.InsertItem(ms_secondarySkeletonName,DIL_closedFolder, DIL_openFolder);

	if(animFileListPrimarySkel && animFileListPrimarySkel->size())
	{
		for(uint32 i = 0; i < animFileListPrimarySkel->size(); ++i)
		{
			m_treeCtrl.InsertItem((*animFileListPrimarySkel)[i].c_str(),DIL_skeletal,DIL_skeletal,primarySkeletonItem);
		}
		m_treeCtrl.Expand(primarySkeletonItem,TVE_EXPAND);
	}

	if(animFileListSecondarySkel && animFileListSecondarySkel->size())
	{
		for(uint32 i = 0; i < animFileListSecondarySkel->size(); ++i)
		{
			m_treeCtrl.InsertItem((*animFileListSecondarySkel)[i].c_str(),DIL_skeletal,DIL_skeletal,secondarySkeletonItem);
		}
		//m_treeCtrl.Expand(secondarySkeletonItem,TVE_EXPAND);
	}
}

//-------------------------------------------------------------------

BOOL AnimationTreeDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CRect sz;
	GetClientRect(sz);
	sz.DeflateRect(5,5);
	
	m_initialized = TRUE;

	if (!m_imageListSet)
	{
		m_imageList.Create (IDB_BITMAP_DIRECTORY, 16, 1, RGB (255,255,255));
		m_treeCtrl.SetImageList (&m_imageList, TVSIL_NORMAL);

		m_imageListSet = true;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//-------------------------------------------------------------------

void AnimationTreeDialog::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	if(!m_initialized)
		return;

	CRect sz;
	GetClientRect(sz);
	sz.DeflateRect(5,5);

	m_treeCtrl.MoveWindow(sz);
}

//-------------------------------------------------------------------

static CString getName (const CTreeCtrl& tree, HTREEITEM current, bool includeRoot=true)
{
	CString name;

	if (current && !tree.ItemHasChildren (current))
	{
		while (current)
		{
			HTREEITEM parent = tree.GetParentItem (current);

			if (parent || (!parent && includeRoot))
			{
				CString tmp = tree.GetItemText (current) + (name.GetLength () != 0 ? "/" : "") + name;

				name = tmp;
			}

			current = parent;
		}
	}

	return name;
}

void AnimationTreeDialog::OnDblclkDirectoryTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREF (pNMHDR);
	*pResult = 0;

	HTREEITEM selection = m_treeCtrl.GetSelectedItem ();

	CString fullName = getName (m_treeCtrl, selection); 
	bool usePrimarySkeleton = (fullName.Find(ms_primarySkeletonName) != -1);
	CString name = getName (m_treeCtrl, selection,false);

	if (selection && m_treeCtrl.GetParentItem (selection) != 0 && m_treeCtrl.ItemHasChildren (selection) == false && name.GetLength () != 0)
	{
		if (GetViewerApp ()->getDocument ())
		{
			std::string filename = ms_animPath;
			filename += name.GetString();
			GetViewerApp()->getDocument()->playAnim(filename.c_str(),false,usePrimarySkeleton);
		}
		SetFocus ();
	}
}

//-------------------------------------------------------------------

CString AnimationTreeDialog::getSelectedFilename() const
{
	HTREEITEM selection = m_treeCtrl.GetSelectedItem ();

	CString name = getName (m_treeCtrl, selection);

	if (selection && m_treeCtrl.GetParentItem (selection) != 0 && m_treeCtrl.ItemHasChildren (selection) == false && name.GetLength () != 0)
	{
		return name;
	}

	return "";
}

//-------------------------------------------------------------------

void AnimationTreeDialog::OnOK() 
{
	// TODO: Add extra validation here
	
//	CDialog::OnOK();
}

//-------------------------------------------------------------------

void AnimationTreeDialog::OnKeydownDirectoryTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREF(pResult);

	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;
	if(m_treeCtrl.GetSelectedCount() > 0 && pTVKeyDown->wVKey == VK_SPACE)
	{
		CTreeItemList treeItemList;
		m_treeCtrl.GetSelectedList(treeItemList);
		uint32 numItems = treeItemList.GetCount();

		bool queueAnimation = false;
		for(uint32 i = 0; i < numItems; ++i)
		{
			POSITION pos = treeItemList.FindIndex(i);
			HTREEITEM currentItem = treeItemList.GetAt(pos);
			if(m_treeCtrl.GetParentItem(currentItem)) // skip roots
			{
				CString fullName = getName (m_treeCtrl, currentItem);
				bool usePrimarySkeleton = (fullName.Find(ms_primarySkeletonName) != -1);
				CString name = getName (m_treeCtrl, currentItem, false);
				
				if(GetViewerApp()->getDocument())
				{
					std::string filename = ms_animPath;
					filename += name.GetString();
					GetViewerApp()->getDocument()->playAnim(filename.c_str(),queueAnimation,usePrimarySkeleton);
					queueAnimation = true;
				}
				SetFocus();
			}
		}
	}	
}

//-------------------------------------------------------------------

BOOL AnimationTreeDialog::PreTranslateMessage(MSG* pMsg) 
{
	if(pMsg->message == WM_KEYDOWN) 
	{ 
		if(pMsg->wParam == VK_RETURN) 
		{
			long result;
			OnDblclkDirectoryTree(0, &result);
		}
	} 
	
	return CDialog::PreTranslateMessage(pMsg);
}

//-------------------------------------------------------------------

class Item
{
public:

	CString path;
	CString asset;
};

//-------------------------------------------------------------------

static HTREEITEM getRoot (const CTreeCtrl& tree, HTREEITEM hitem)
{
	HTREEITEM parent         = hitem;
	HTREEITEM possibleParent = hitem;

	do
	{
		possibleParent = tree.GetParentItem (possibleParent);

		if (possibleParent)
			parent = possibleParent;
	}
	while (possibleParent != 0);

	return parent;
}

//-------------------------------------------------------------------

void AnimationTreeDialog::OnRclickDirectoryTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREF (pNMHDR);

	// TODO: Add your control notification handler code here

	*pResult = 0;
}

//-------------------------------------------------------------------

void AnimationTreeDialog::openItem (HTREEITEM root, const bool writeCustomizationData, const bool closeDocument, const bool debugDump) 
{
	// JU_TODO: remove this
	while (root)
	{
		const CString string = m_treeCtrl.GetItemText (root);

		if (writeCustomizationData)
		{
			CONSOLE_PRINT ("processing customization data for ");
			CONSOLE_PRINT (string);
			CONSOLE_PRINT ("\r\n");
		}
		else
		{
			CONSOLE_PRINT ("opening ");
			CONSOLE_PRINT (string);
			CONSOLE_PRINT ("\r\n");
		}

		m_treeCtrl.SelectItem (root);
		long result;
		OnDblclkDirectoryTree(0, &result);

		if (GetViewerApp ()->getDocument ())
		{
			if (writeCustomizationData)
			{
				GetViewerApp ()->getDocument ()->writeObjectTemplateCustomizationData (false);
				GetViewerApp ()->getDocument ()->OnCloseDocument ();
			}
			else
				if (debugDump)
				{
					GetViewerApp ()->getDocument ()->debugDump ();
					GetViewerApp ()->getDocument ()->OnCloseDocument ();
				}
				else 
					if (closeDocument)
					{
						POSITION position = GetViewerApp ()->getDocument ()->GetFirstViewPosition ();
						CView* const view = GetViewerApp ()->getDocument ()->GetNextView (position);
						if (view)
						{
							AfxWndProc (view->m_hWnd, WM_PAINT, 0, 0);
							GetViewerApp ()->getDocument ()->OnCloseDocument ();
						}
					}
		}

		root = m_treeCtrl.GetNextSiblingItem (root);
	}
}

// ----------------------------------------------------------------------

void AnimationTreeDialog::openSingleItem (HTREEITEM root, const bool writeCustomizationData, const bool closeDocument, const bool debugDump) 
{
	// JU_TODO: remove this
	const CString string = m_treeCtrl.GetItemText (root);

	if (writeCustomizationData)
	{
		CONSOLE_PRINT ("processing customization data for ");
		CONSOLE_PRINT (string);
		CONSOLE_PRINT ("\r\n");
	}
	else
	{
		CONSOLE_PRINT ("opening ");
		CONSOLE_PRINT (string);
		CONSOLE_PRINT ("\r\n");
	}

	m_treeCtrl.SelectItem (root);
	long result;
	OnDblclkDirectoryTree(0, &result);

	if (GetViewerApp ()->getDocument ())
	{
		if (writeCustomizationData)
		{
			GetViewerApp ()->getDocument ()->writeObjectTemplateCustomizationData (false);
			GetViewerApp ()->getDocument ()->OnCloseDocument ();
		}
		else
			if (debugDump)
			{
				GetViewerApp ()->getDocument ()->debugDump ();
				GetViewerApp ()->getDocument ()->OnCloseDocument ();
			}
			else 
				if (closeDocument)
				{
					POSITION position = GetViewerApp ()->getDocument ()->GetFirstViewPosition ();
					CView* const view = GetViewerApp ()->getDocument ()->GetNextView (position);
					if (view)
					{
						AfxWndProc (view->m_hWnd, WM_PAINT, 0, 0);
						GetViewerApp ()->getDocument ()->OnCloseDocument ();
					}
				}
	}
}

//-------------------------------------------------------------------

void AnimationTreeDialog::OnItemexpandingDirectoryTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	const HTREEITEM item       = pNMTreeView->itemNew.hItem;
	const HTREEITEM parentItem = m_treeCtrl.GetParentItem (item);
	const HTREEITEM childItem  = m_treeCtrl.GetChildItem (item);

	UNREF(item);
	UNREF(parentItem);
	UNREF(childItem);

	*pResult = 0;
}

// ==================================================================
