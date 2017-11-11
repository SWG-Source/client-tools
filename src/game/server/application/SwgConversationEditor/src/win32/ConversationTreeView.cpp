// ======================================================================
//
// ConversationTreeView.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "ConversationTreeView.h"

#include "ConversationFrame.h"
#include "Conversation.h"
#include "DialogTestConversation.h"
#include "sharedFile/Iff.h"
#include "StringIdTracker.h"
#include "SwgConversationEditor.h"
#include "SwgConversationEditorDoc.h"

// ======================================================================

namespace ConversationTreeViewNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	DWORD const DRAG_DELAY = 80;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	inline bool keyDown (int key)
	{
		return (GetKeyState (key) & 0x8000) != 0;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void collapseBranch (CTreeCtrl & treeCtrl, HTREEITEM treeItem)
	{
		if (treeItem && treeCtrl.ItemHasChildren (treeItem))
		{
			IGNORE_RETURN (treeCtrl.Expand (treeItem, TVE_COLLAPSE));
			treeItem = treeCtrl.GetChildItem (treeItem);

			do
			{
				collapseBranch (treeCtrl, treeItem);
			}
			while ((treeItem = treeCtrl.GetNextSiblingItem (treeItem)) != 0);
		}
		
		IGNORE_RETURN (treeCtrl.EnsureVisible (treeCtrl.GetSelectedItem ()));
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void expandBranch (CTreeCtrl & treeCtrl, HTREEITEM treeItem)
	{
		if (treeItem && treeCtrl.ItemHasChildren (treeItem))
		{
			IGNORE_RETURN (treeCtrl.Expand (treeItem, TVE_EXPAND));
			treeItem = treeCtrl.GetChildItem (treeItem);

			do
			{
				expandBranch (treeCtrl, treeItem);
			}
			while ((treeItem = treeCtrl.GetNextSiblingItem (treeItem)) != 0);
		}
		
		IGNORE_RETURN (treeCtrl.EnsureVisible (treeCtrl.GetSelectedItem ()));
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
}

using namespace ConversationTreeViewNamespace;

// ======================================================================

IMPLEMENT_DYNCREATE(ConversationTreeView, CTreeView)

ConversationTreeView::ConversationTreeView () :
	m_imageListSet (false),
	m_imageList (),
	m_copyCursor (0),
	m_moveCursor (0),
	m_dragMode (DM_nothing),
	m_pDragImage (0),
	m_htiDrag (0),
	m_htiDrop (0),
	m_htiOldDrop (0),
	m_bLDragging (false),
	m_idTimer (0)
{
	m_copyCursor = LoadCursor (AfxGetApp ()->m_hInstance, MAKEINTRESOURCE (IDC_CURSOR_COPY));  //lint !1924  //-- c-style cast
	m_moveCursor = LoadCursor (AfxGetApp ()->m_hInstance, MAKEINTRESOURCE (IDC_CURSOR_MOVE));  //lint !1924  //-- c-style cast
}

ConversationTreeView::~ConversationTreeView()
{
	m_copyCursor = 0;
	m_moveCursor = 0;
	m_pDragImage = 0;
	m_htiDrag = 0;
	m_htiDrop = 0;
	m_htiOldDrop = 0;
}

// ======================================================================

BEGIN_MESSAGE_MAP(ConversationTreeView, CTreeView)
	//{{AFX_MSG_MAP(ConversationTreeView)
	ON_WM_RBUTTONDOWN()
	ON_NOTIFY_REFLECT(TVN_DELETEITEM, OnDeleteitem)
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeydown)
	ON_COMMAND(ID__BRANCH_ADDRESPONSE, OnBranchAddresponse)
	ON_COMMAND(ID__RESPONSE_ADDBRANCH, OnResponseAddbranch)
	ON_COMMAND(ID_BUTTON_DELETE, OnButtonDelete)
	ON_COMMAND(ID_BUTTON_DEMOTE, OnButtonDemote)
	ON_COMMAND(ID_BUTTON_PROMOTE, OnButtonPromote)
	ON_COMMAND(ID__COLLAPSEALL, OnCollapseall)
	ON_COMMAND(ID__EXPANDALL, OnExpandall)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(ID__BRANCH_TEST, OnBranchTest)
	ON_COMMAND(ID__BRANCH_DELETE, OnBranchDelete)
	ON_COMMAND(ID__RESPONSE_DELETE, OnResponseDelete)
	ON_COMMAND(ID__BRANCH_GROUPECHO_FLAGCHILDREN, OnBranchGroupEchoRecursiveOn)
	ON_COMMAND(ID__BRANCH_GROUPECHO_UNFLAGCHILDREN, OnBranchGroupEchoRecursiveOff)
	ON_COMMAND(ID__RESPONSE_GROUPECHO_FLAGCHILDREN, OnResponseGroupEchoRecursiveOn)
	ON_COMMAND(ID__RESPONSE_GROUPECHO_UNFLAGCHILDREN, OnResponseGroupEchoRecursiveOff)
	ON_UPDATE_COMMAND_UI(ID__BRANCH_ADDRESPONSE, OnUpdateBranchAddresponse)
	ON_UPDATE_COMMAND_UI(ID__RESPONSE_ADDBRANCH, OnUpdateResponseAddbranch)
	ON_UPDATE_COMMAND_UI(ID__BRANCH_TEST, OnUpdateBranchTest)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

#ifdef _DEBUG
void ConversationTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void ConversationTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

// ======================================================================

BOOL ConversationTreeView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= (TVS_SHOWSELALWAYS | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS);
	
	return CTreeView::PreCreateWindow(cs);
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnDraw(CDC * const pDC)
{
	pDC->MoveTo (0, 0);
	pDC->LineTo (100, 100);
}

// ----------------------------------------------------------------------

void ConversationTreeView::selectItem (ConversationItem const * const conversationItem)
{
	selectItem (find (GetTreeCtrl ().GetRootItem (), conversationItem));
}

// ----------------------------------------------------------------------

SwgConversationEditorDoc::ItemData * ConversationTreeView::getSelectedItemData ()
{
	return getItemData (GetTreeCtrl ().GetSelectedItem ());
}

// ----------------------------------------------------------------------

ConversationTreeView::TreeViewIconType ConversationTreeView::getIcon (SwgConversationEditorDoc::ItemData const * const itemData) const
{
	ConversationItem const * const conversationItem = itemData->m_conversationItem;
	int const conditionFamilyId = conversationItem->getConditionFamilyId ();

	int icon = 0;

	switch (itemData->m_type)
	{
	default:
	case SwgConversationEditorDoc::ItemData::T_unknown:
		return TVIT_root;

	case SwgConversationEditorDoc::ItemData::T_branch:
		{
			icon = TVIT_branch;
			if (conditionFamilyId != 0)
			{
				if (conversationItem->getNegateCondition ())
					icon = conversationItem->getDebug () ? TVIT_branchNegatedDebug : TVIT_branchNegated;
				else
					icon = conversationItem->getDebug () ? TVIT_branchDebug : TVIT_branch;
			}
			else
			{
				if (conversationItem->getNegateCondition ())
					icon = conversationItem->getDebug () ? TVIT_branchNegatedDebugDefault : TVIT_branchNegatedDefault;
				else
					icon = conversationItem->getDebug () ? TVIT_branchDebugDefault : TVIT_branchDefault;
			}
		}
		break;

	case SwgConversationEditorDoc::ItemData::T_response:
		{
			icon = TVIT_response;
			if (conditionFamilyId != 0)
			{
				if (conversationItem->getNegateCondition ())
					icon = conversationItem->getDebug () ? TVIT_responseNegatedDebug : TVIT_responseNegated;
				else
					icon = conversationItem->getDebug () ? TVIT_responseDebug : TVIT_response;
			}
			else
			{
				if (conversationItem->getNegateCondition ())
					icon = conversationItem->getDebug () ? TVIT_responseNegatedDebugDefault : TVIT_responseNegatedDefault;
				else
					icon = conversationItem->getDebug () ? TVIT_responseDebugDefault : TVIT_responseDefault;
			}
		}
		break;
	}

	if (conversationItem->getLabelFamilyId () != 0)
		icon += 16;
	else
		if (conversationItem->getLinkFamilyId () != 0)
			icon += 32;

	return static_cast<TreeViewIconType> (icon);
}

// ----------------------------------------------------------------------

void ConversationTreeView::refreshIcons (HTREEITEM treeItem)
{
	//-- refresh me
	SwgConversationEditorDoc::ItemData const * const itemData = getItemData (treeItem);
	TreeViewIconType icon = getIcon (itemData);
	bool const root = GetTreeCtrl ().GetParentItem (treeItem) == 0;

	IGNORE_RETURN (GetTreeCtrl ().SetItemImage (treeItem, root ? TVIT_root : icon, root ? TVIT_root : icon));

	//-- refresh the children
	HTREEITEM child = GetTreeCtrl ().GetChildItem (treeItem);

	if (child)
	{
		do
		{
			refreshIcons (child);

			child = GetTreeCtrl ().GetNextSiblingItem (child);
		} 
		while (child);
	}
}

// ----------------------------------------------------------------------

HTREEITEM ConversationTreeView::find (HTREEITEM hItem, ConversationItem const * const conversationItem) 
{
	HTREEITEM theItem = NULL;

	if (hItem == NULL) 
		return NULL;

	if ((theItem = GetTreeCtrl ().GetNextSiblingItem (hItem)) != NULL) 
	{
		theItem = find (theItem, conversationItem);

		if (theItem != NULL) 
			return theItem;
	}

	if ((theItem = GetTreeCtrl ().GetChildItem (hItem)) != NULL) 
	{		
		theItem = find (theItem, conversationItem);

		if (theItem != NULL) 
			return theItem;
	}

	SwgConversationEditorDoc::ItemData const * const itemData = getItemData (hItem);

	if (itemData->m_conversationItem == conversationItem)
		return hItem;

	return theItem;
}

//-------------------------------------------------------------------

void ConversationTreeView::selectItem (HTREEITEM const treeItem) 
{
	if (treeItem)
	{
		IGNORE_RETURN (GetTreeCtrl ().SelectItem (treeItem));
		IGNORE_RETURN (GetTreeCtrl ().EnsureVisible (treeItem));

		GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_conversationItemChanged, 0);
	}
}

// ----------------------------------------------------------------------

bool ConversationTreeView::isBranch (HTREEITEM const treeItem) const
{
	SwgConversationEditorDoc::ItemData const * const itemData = getItemData (treeItem);

	return itemData->m_type == SwgConversationEditorDoc::ItemData::T_branch;
}

// ----------------------------------------------------------------------

HTREEITEM ConversationTreeView::getSelectedBranch () const
{
	//-- am i pointing to anything?
	HTREEITEM const treeItem = GetTreeCtrl ().GetSelectedItem ();

	if (treeItem)
		return isBranch (treeItem) ? treeItem : 0;
	else
		return 0;
}

// ----------------------------------------------------------------------

bool ConversationTreeView::isBranchSelected () const
{
	return getSelectedBranch () != 0;
}

// ----------------------------------------------------------------------

bool ConversationTreeView::isResponse (HTREEITEM const treeItem) const
{
	SwgConversationEditorDoc::ItemData const * const itemData = getItemData (treeItem);

	return itemData->m_type == SwgConversationEditorDoc::ItemData::T_response;
}

// ----------------------------------------------------------------------

HTREEITEM ConversationTreeView::getSelectedResponse () const
{
	//-- am i pointing to anything?
	HTREEITEM const treeItem = GetTreeCtrl ().GetSelectedItem ();

	if (treeItem)
		return isResponse (treeItem) ? treeItem : 0;
	else
		return 0;
}

// ----------------------------------------------------------------------

bool ConversationTreeView::isResponseSelected () const
{
	return getSelectedResponse () != 0;
}

// ----------------------------------------------------------------------

SwgConversationEditorDoc::ItemData * ConversationTreeView::getItemData (HTREEITEM const treeItem, VerifyType const verifyType)
{
	SwgConversationEditorDoc::ItemData * const itemData = reinterpret_cast<SwgConversationEditorDoc::ItemData *> (GetTreeCtrl ().GetItemData (treeItem));
	switch (verifyType)
	{
	case VT_branch:
		FATAL (itemData->m_type != SwgConversationEditorDoc::ItemData::T_branch, ("call to wrong function branch"));
		break;

	case VT_response:
		FATAL (itemData->m_type != SwgConversationEditorDoc::ItemData::T_response, ("call to wrong function response"));
		break;

	case VT_none:
	default:
		break;
	}

	return itemData;
}

// ----------------------------------------------------------------------

SwgConversationEditorDoc::ItemData const * ConversationTreeView::getItemData (HTREEITEM const treeItem, VerifyType const verifyType) const
{
	SwgConversationEditorDoc::ItemData * const itemData = reinterpret_cast<SwgConversationEditorDoc::ItemData *> (GetTreeCtrl ().GetItemData (treeItem));
	switch (verifyType)
	{
	case VT_branch:
		FATAL (itemData->m_type != SwgConversationEditorDoc::ItemData::T_branch, ("call to wrong function branch"));
		break;

	case VT_response:
		FATAL (itemData->m_type != SwgConversationEditorDoc::ItemData::T_response, ("call to wrong function response"));
		break;

	case VT_none:
	default:
		break;
	}

	return itemData;
}

// ----------------------------------------------------------------------

void ConversationTreeView::setItemData (HTREEITEM const treeItem, SwgConversationEditorDoc::ItemData * const itemData) const
{
	if (itemData)
	{
		itemData->m_treeItem = treeItem;
		IGNORE_RETURN (GetTreeCtrl ().SetItemData (treeItem, reinterpret_cast<DWORD> (itemData)));
	}
}

// ----------------------------------------------------------------------

HTREEITEM ConversationTreeView::addResponseToTree (HTREEITEM const parentItem, HTREEITEM const afterItem, ConversationResponse * const response) const
{
	SwgConversationEditorDoc::ItemData * const itemData = new SwgConversationEditorDoc::ItemData;
	itemData->m_type = SwgConversationEditorDoc::ItemData::T_response;
	itemData->m_conversationItem = response;

	bool const root = parentItem == TVI_ROOT;
	TreeViewIconType const icon = getIcon (itemData);
	HTREEITEM const treeItem = GetTreeCtrl ().InsertItem (root ? "Root" : itemData->getText (), root ? TVIT_root : icon, root ? TVIT_root : icon, parentItem, afterItem);
	setItemData (treeItem, itemData);

	//-- add branches
	for (int i = 0; i < response->getNumberOfBranches (); ++i)
		addBranchToTree (treeItem, TVI_LAST, response->getBranch (i));

	//-- set its expanded state
	if (response->getExpanded ())
		IGNORE_RETURN (GetTreeCtrl ().Expand (treeItem, TVE_EXPAND));

	return treeItem;
}

// ----------------------------------------------------------------------

void ConversationTreeView::addConversationToTree ()
{
	//-- add the conversation to the tree
	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	Conversation * const conversation = document->getConversation ();
	addResponseToTree (TVI_ROOT, 0, conversation);

	//-- expand the tree
	expandBranch (GetTreeCtrl (), GetTreeCtrl ().GetRootItem ());

	selectItem (GetTreeCtrl ().GetRootItem ());
}

// ----------------------------------------------------------------------

HTREEITEM ConversationTreeView::addBranchToTree (HTREEITEM const parentItem, HTREEITEM const afterItem, ConversationBranch * const branch) const
{
	SwgConversationEditorDoc::ItemData * const itemData = new SwgConversationEditorDoc::ItemData;
	itemData->m_type = SwgConversationEditorDoc::ItemData::T_branch;
	itemData->m_conversationItem = branch;

	TreeViewIconType icon = getIcon (itemData);
	HTREEITEM const treeItem = GetTreeCtrl ().InsertItem (itemData->getText (), icon, icon, parentItem, afterItem);
	setItemData (treeItem, itemData);

	//-- add responses
	for (int i = 0; i < branch->getNumberOfResponses (); ++i)
		addResponseToTree (treeItem, TVI_LAST, branch->getResponse (i));

	//-- set its expanded state
	if (branch->getExpanded ())
		IGNORE_RETURN (GetTreeCtrl ().Expand (treeItem, TVE_EXPAND));

	return treeItem;
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();

	//-- set the tree's image list
	if (!m_imageListSet)
	{
		IGNORE_RETURN (m_imageList.Create (IDB_BITMAP_CONVERSATIONTREEVIEW, 16, 1, RGB (255,255,255)));
		IGNORE_RETURN (GetTreeCtrl ().SetImageList (&m_imageList, TVSIL_NORMAL));

		m_imageListSet = true;
	}

	addConversationToTree ();
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CTreeView::OnRButtonDown(nFlags, point);

	UINT flags = 0;
	HTREEITEM treeItem = GetTreeCtrl ().HitTest (point, &flags);
	if (treeItem)
	{
		selectItem (treeItem);

		SwgConversationEditorDoc::ItemData const * const itemData = getItemData (treeItem);
		DWORD menuBranch = 0;
		switch (itemData->m_type)
		{
		case SwgConversationEditorDoc::ItemData::T_branch:
			menuBranch = 0;
			break;

		case SwgConversationEditorDoc::ItemData::T_response:
			menuBranch = 1;
			break;
		}

		CPoint pt = point;
		ClientToScreen (&pt);

		CMenu menu;
		IGNORE_RETURN (menu.LoadMenu (IDR_MENU_TREEVIEW));

		CMenu* rootMenu = menu.GetSubMenu (0);
		CMenu* subMenu = rootMenu->GetSubMenu (menuBranch);
		IGNORE_RETURN (subMenu->TrackPopupMenu (TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, this));
	}
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = reinterpret_cast<NM_TREEVIEW*> (pNMHDR);

	//-- get the treeitem to delete
	HTREEITEM const treeItem = pNMTreeView->itemOld.hItem;

	//-- delete the itemdata associated with the treeitem
	SwgConversationEditorDoc::ItemData * const itemData = getItemData (treeItem);

	if (itemData)
		delete itemData;

	//-- set the data to 0
	setItemData (treeItem, 0);

	*pResult = 0;
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnKeydown (NMHDR * const pNMHDR, LRESULT * const pResult) 
{
	TV_KEYDOWN const * const pTVKeyDown = reinterpret_cast<TV_KEYDOWN const *> (pNMHDR);

	switch (pTVKeyDown->wVKey)
	{
	case VK_DELETE:
		OnButtonDelete ();
		break;
	}

	*pResult = 0;
}

// ----------------------------------------------------------------------

void ConversationTreeView::deleteBranch (HTREEITEM treeItem)
{
	SwgConversationEditorDoc::ItemData * const itemData = getItemData (treeItem, VT_branch);

	HTREEITEM parentTreeItem = GetTreeCtrl ().GetParentItem (treeItem);
	SwgConversationEditorDoc::ItemData * const parentItemData = getItemData (parentTreeItem, VT_response);

	ConversationBranch * const branch = safe_cast<ConversationBranch *> (itemData->m_conversationItem);
	ConversationResponse * const response = safe_cast<ConversationResponse *> (parentItemData->m_conversationItem);
	response->removeBranch (branch, true);

	HTREEITEM siblingItem = GetTreeCtrl ().GetPrevSiblingItem (treeItem);
	selectItem (siblingItem ? siblingItem : parentTreeItem);
	IGNORE_RETURN (GetTreeCtrl ().DeleteItem (treeItem));

	GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_conversationItemChanged, 0);
	GetDocument ()->SetModifiedFlag ();
	Invalidate ();
}

// ----------------------------------------------------------------------

void ConversationTreeView::deleteBranch ()
{
	deleteBranch (GetTreeCtrl ().GetSelectedItem ());
}

// ----------------------------------------------------------------------

void ConversationTreeView::deleteResponse (HTREEITEM treeItem)
{
	SwgConversationEditorDoc::ItemData * const itemData = getItemData (treeItem, VT_response);

	HTREEITEM parentTreeItem = GetTreeCtrl ().GetParentItem (treeItem);
	SwgConversationEditorDoc::ItemData * const parentItemData = getItemData (parentTreeItem, VT_branch);

	ConversationResponse * const response = safe_cast<ConversationResponse *> (itemData->m_conversationItem);
	ConversationBranch * const branch = safe_cast<ConversationBranch *> (parentItemData->m_conversationItem);
	branch->removeResponse (response, true);

	HTREEITEM siblingItem = GetTreeCtrl ().GetPrevSiblingItem (treeItem);
	selectItem (siblingItem ? siblingItem : parentTreeItem);
	IGNORE_RETURN (GetTreeCtrl ().DeleteItem (treeItem));

	GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_conversationItemChanged, 0);
	GetDocument ()->SetModifiedFlag ();
	Invalidate ();
}

// ----------------------------------------------------------------------

void ConversationTreeView::deleteResponse ()
{
	deleteResponse (GetTreeCtrl ().GetSelectedItem ());
}

// ----------------------------------------------------------------------

SwgConversationEditorDoc * ConversationTreeView::getConversationEditorDoc()
{
	return safe_cast<SwgConversationEditorDoc *> (GetDocument ());
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnBranchAddresponse() 
{
	HTREEITEM treeItem = GetTreeCtrl ().GetSelectedItem ();
	SwgConversationEditorDoc::ItemData * const itemData = getItemData (treeItem, VT_branch);

	ConversationBranch * const branch = safe_cast<ConversationBranch *> (itemData->m_conversationItem);

	if (branch->getLinkFamilyId () != 0)
	{
		MessageBox ("Conversation item has a link.  You can only add children to the original conversation item.");
		return;
	}

	ConversationResponse * const response = new ConversationResponse(getConversationEditorDoc()->getStringIdTracker());
	response->setExpanded (true);
	response->setDebug (branch->getDebug ());
	response->setGroupEcho (branch->getGroupEcho ());
	branch->addResponse (response);

	addResponseToTree (treeItem, TVI_LAST, response);

	IGNORE_RETURN (GetTreeCtrl ().Expand (treeItem, TVE_EXPAND));

	GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_conversationItemChanged);
	GetDocument ()->SetModifiedFlag ();
	Invalidate ();
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnResponseAddbranch() 
{
	HTREEITEM treeItem = GetTreeCtrl ().GetSelectedItem ();
	SwgConversationEditorDoc::ItemData * const itemData = getItemData (treeItem, VT_response);

	ConversationResponse * const response = safe_cast<ConversationResponse *> (itemData->m_conversationItem);

	if (response->getLinkFamilyId () != 0)
	{
		MessageBox ("Conversation item has a link.  You can only add children to the original conversation item.");
		return;
	}

	ConversationBranch * const branch = new ConversationBranch(getConversationEditorDoc()->getStringIdTracker());
	branch->setDebug (response->getDebug ());
	branch->setGroupEcho (response->getGroupEcho ());
	response->addBranch (branch);

	addBranchToTree (treeItem, TVI_LAST, branch);

	IGNORE_RETURN (GetTreeCtrl ().Expand (treeItem, TVE_EXPAND));

	GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_conversationItemChanged);
	GetDocument ()->SetModifiedFlag ();
	Invalidate ();
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnButtonDelete() 
{
	HTREEITEM treeItem = GetTreeCtrl ().GetSelectedItem ();
	if (treeItem)
	{
		HTREEITEM parentTreeItem = GetTreeCtrl ().GetParentItem (treeItem);
		if (parentTreeItem)
		{
			CString const name = GetTreeCtrl ().GetItemText (treeItem);

			CString message;
			message.Format ("Are you sure you want to delete %s?", name);

			if (MessageBox (message, 0, MB_YESNO) == IDYES)
			{
				if (isBranchSelected ())
					deleteBranch ();
				else
					if (isResponseSelected ())
						deleteResponse ();
			}
		}
		else
			MessageBox ("You can't delete the root.  Delete the children of the root or start a new conversation.");
	}
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnButtonDemote() 
{
	if (isBranchSelected ())
		demoteBranch ();
	else
		if (isResponseSelected ())
			demoteResponse ();
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnButtonPromote() 
{
	if (isBranchSelected ())
		promoteBranch ();
	else
		if (isResponseSelected ())
			promoteResponse ();
}

// ----------------------------------------------------------------------

void ConversationTreeView::promoteBranch ()
{
	HTREEITEM treeItem = getSelectedBranch ();
	HTREEITEM nextItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);

	if (nextItem)
	{
		//-- get branch
		SwgConversationEditorDoc::ItemData * const itemData = getItemData (treeItem, VT_branch);
		ConversationBranch * const branch = safe_cast<ConversationBranch *> (itemData->m_conversationItem);

		//-- get response
		HTREEITEM parentItem = GetTreeCtrl ().GetParentItem (treeItem);
		SwgConversationEditorDoc::ItemData * const parentItemData = getItemData (parentItem, VT_response);
		ConversationResponse * const response = safe_cast<ConversationResponse *> (parentItemData->m_conversationItem);

		//-- promote branch
		response->promoteBranch (branch);

		//-- delete branch and reinsert
		saveExpandedState (treeItem);
		IGNORE_RETURN (GetTreeCtrl ().DeleteItem (treeItem));
		HTREEITEM insertedItem = addBranchToTree (GetTreeCtrl ().GetParentItem (nextItem), nextItem, branch);
		selectItem (insertedItem);

		Invalidate ();
		GetDocument ()->SetModifiedFlag ();
	}
}

// ----------------------------------------------------------------------

void ConversationTreeView::demoteBranch ()
{
	HTREEITEM treeItem = getSelectedBranch ();
	HTREEITEM previousTreeItem = GetTreeCtrl ().GetPrevSiblingItem (treeItem);

	if (previousTreeItem)
	{
		//-- get branch
		SwgConversationEditorDoc::ItemData * const itemData = getItemData (treeItem, VT_branch);
		ConversationBranch * const branch = safe_cast<ConversationBranch *> (itemData->m_conversationItem);

		//-- get previous branch
		SwgConversationEditorDoc::ItemData * const previousItemData = getItemData (previousTreeItem, VT_branch);
		ConversationBranch * const previousBranch = safe_cast<ConversationBranch *> (previousItemData->m_conversationItem);

		//-- get parent item
		HTREEITEM parentItem = GetTreeCtrl ().GetParentItem (treeItem);
		SwgConversationEditorDoc::ItemData * const parentItemData = getItemData (parentItem, VT_response);
		ConversationResponse * const response = safe_cast<ConversationResponse *> (parentItemData->m_conversationItem);

		//-- demote branch
		response->demoteBranch (branch);

		//-- remove previous branch from tree
		saveExpandedState (previousTreeItem);
		IGNORE_RETURN (GetTreeCtrl ().DeleteItem (previousTreeItem));

		//-- reinsert branch after next
		IGNORE_RETURN (addBranchToTree (GetTreeCtrl ().GetParentItem (treeItem), treeItem, previousBranch));
		selectItem (treeItem);

		Invalidate ();
		GetDocument ()->SetModifiedFlag ();
	}
}

// ----------------------------------------------------------------------

void ConversationTreeView::promoteResponse ()
{
	HTREEITEM treeItem = getSelectedResponse ();
	HTREEITEM nextItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);

	if (nextItem)
	{
		//-- get response
		SwgConversationEditorDoc::ItemData * const itemData = getItemData (treeItem, VT_response);
		ConversationResponse * const response = safe_cast<ConversationResponse *> (itemData->m_conversationItem);

		//-- get branch
		HTREEITEM parentItem = GetTreeCtrl ().GetParentItem (treeItem);
		SwgConversationEditorDoc::ItemData * const parentItemData = getItemData (parentItem, VT_branch);
		ConversationBranch * const branch = safe_cast<ConversationBranch *> (parentItemData->m_conversationItem);

		//-- promote branch
		branch->promoteResponse (response);

		//-- delete branch and reinsert
		saveExpandedState (treeItem);
		IGNORE_RETURN (GetTreeCtrl ().DeleteItem (treeItem));
		HTREEITEM insertedItem = addResponseToTree (GetTreeCtrl ().GetParentItem (nextItem), nextItem, response);
		selectItem (insertedItem);

		Invalidate ();
		GetDocument ()->SetModifiedFlag ();
	}
}

// ----------------------------------------------------------------------

void ConversationTreeView::demoteResponse ()
{
	HTREEITEM treeItem = getSelectedResponse ();
	HTREEITEM previousTreeItem = GetTreeCtrl ().GetPrevSiblingItem (treeItem);

	if (previousTreeItem)
	{
		//-- get response
		SwgConversationEditorDoc::ItemData * const itemData = getItemData (treeItem, VT_response);
		ConversationResponse * const response = safe_cast<ConversationResponse *> (itemData->m_conversationItem);

		//-- get previous response
		SwgConversationEditorDoc::ItemData * const previousItemData = getItemData (previousTreeItem, VT_response);
		ConversationResponse * const previousResponse = safe_cast<ConversationResponse *> (previousItemData->m_conversationItem);

		//-- get parent item
		HTREEITEM parentItem = GetTreeCtrl ().GetParentItem (treeItem);
		SwgConversationEditorDoc::ItemData * const parentItemData = getItemData (parentItem, VT_branch);
		ConversationBranch * const branch = safe_cast<ConversationBranch *> (parentItemData->m_conversationItem);

		//-- demote branch
		branch->demoteResponse (response);

		//-- remove previous response from tree
		saveExpandedState (previousTreeItem);
		IGNORE_RETURN (GetTreeCtrl ().DeleteItem (previousTreeItem));

		//-- reinsert response after next
		IGNORE_RETURN (addResponseToTree (GetTreeCtrl ().GetParentItem (treeItem), treeItem, previousResponse));
		selectItem (treeItem);

		//-- tell document to update views
		Invalidate ();
		GetDocument ()->SetModifiedFlag ();
	}
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnCollapseall() 
{
	collapseBranch (GetTreeCtrl (), GetTreeCtrl ().GetSelectedItem ());
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnExpandall() 
{
	expandBranch (GetTreeCtrl (), GetTreeCtrl ().GetSelectedItem ());
}

// ----------------------------------------------------------------------

void ConversationTreeView::saveExpandedState (HTREEITEM treeItem)
{
	if (treeItem && GetTreeCtrl ().ItemHasChildren (treeItem))
	{
		SwgConversationEditorDoc::ItemData * const itemData = getItemData (treeItem);
		itemData->m_conversationItem->setExpanded ((GetTreeCtrl ().GetItemState (treeItem, TVIS_EXPANDED) & TVIS_EXPANDED) != 0);

		treeItem = GetTreeCtrl ().GetChildItem (treeItem);

		do
		{
			saveExpandedState (treeItem);
		}
		while ((treeItem = GetTreeCtrl ().GetNextSiblingItem (treeItem)) != 0);
	}
}

// ----------------------------------------------------------------------

void ConversationTreeView::saveExpandedState ()
{
    HTREEITEM treeItem = GetTreeCtrl ().GetRootItem ();

	if (treeItem)
	{
		do
		{
			saveExpandedState (treeItem);
		}
		while ((treeItem = GetTreeCtrl ().GetNextSiblingItem (treeItem)) != 0);
	}
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnSelchanged(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
//	HTREEITEM treeItem = GetTreeCtrl ().GetSelectedItem ();
//	SwgConversationEditorDoc::ItemData * const itemData = getItemData (treeItem);
	safe_cast<ConversationFrame *> (GetParentFrame ())->updateView ();

	*pResult = 0;
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (this != pSender)
	{
		switch (lHint)
		{
		case SwgConversationEditorDoc::H_textChanged:
			{
				HTREEITEM treeItem = reinterpret_cast<HTREEITEM> (pHint);
				if (treeItem)
				{
					SwgConversationEditorDoc::ItemData const * const itemData = getItemData (treeItem);
					GetTreeCtrl ().SetItemText (treeItem, itemData->getText ());
				}
			}
			break;

		case SwgConversationEditorDoc::H_fileImported:
			{
				GetTreeCtrl ().DeleteAllItems ();
				addConversationToTree ();
			}
			break;

		case SwgConversationEditorDoc::H_debugChanged:
		case SwgConversationEditorDoc::H_conditionChanged:
		case SwgConversationEditorDoc::H_negateConditionChanged:
		case SwgConversationEditorDoc::H_groupEchoChanged:
		case SwgConversationEditorDoc::H_actionChanged:
		case SwgConversationEditorDoc::H_labelChanged:
		case SwgConversationEditorDoc::H_linkChanged:
			{
				refreshIcons (GetTreeCtrl ().GetRootItem ());
			}
			break;
		}
	}
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//-- prevent accidental drags.
    if ((GetTickCount () - m_dwDragStart) < DRAG_DELAY)
		return;

	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	m_dragMode = DM_move;
	m_htiDrag = pNMTreeView->itemNew.hItem;
	m_htiDrop = NULL;

	m_pDragImage = GetTreeCtrl().CreateDragImage( m_htiDrag );
	if( !m_pDragImage )
		return;

	CPoint pt(0,0);
	
	IMAGEINFO ii;
	IGNORE_RETURN (m_pDragImage->GetImageInfo( 0, &ii ));
	pt.x = (ii.rcImage.right - ii.rcImage.left) / 2;
	pt.y = (ii.rcImage.bottom - ii.rcImage.top) / 2;

	IGNORE_RETURN (m_pDragImage->BeginDrag( 0, pt ));
	pt = pNMTreeView->ptDrag;
	ClientToScreen( &pt );
	IGNORE_RETURN (m_pDragImage->DragEnter(NULL,pt));
	
	SetCapture();

	*pResult = 0;
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CTreeView::OnLButtonUp(nFlags, point);

	switch (m_dragMode)
	{
	case DM_move:
		OnLButtonUpForMove ();
		break;

	case DM_copy:
		OnLButtonUpForCopy ();
		break;

	case DM_nothing:
	default:
		break;
	}
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CTreeView::OnMouseMove(nFlags, point);

	HTREEITEM hti;
	UINT      flags;

	if (m_dragMode == DM_move || m_dragMode == DM_copy)
	{
		if (keyDown (VK_CONTROL))
			m_dragMode = DM_copy;
		else
			m_dragMode = DM_move;

		if (m_dragMode == DM_copy)
			SetCursor (m_copyCursor);
		else
			SetCursor (m_moveCursor);

		CTreeCtrl& theTree = GetTreeCtrl();
		POINT pt = point;
		ClientToScreen( &pt );
		IGNORE_RETURN (CImageList::DragMove(pt));
		
		hti = theTree.HitTest(point,&flags);
		if( hti != NULL )
		{
			IGNORE_RETURN (CImageList::DragShowNolock(FALSE));

			if( m_htiOldDrop == NULL )
				m_htiOldDrop = theTree.GetDropHilightItem();

			IGNORE_RETURN (theTree.SelectDropTarget(hti));
			
			m_htiDrop = hti;
			
			if( m_idTimer && hti == m_htiOldDrop )
			{
				IGNORE_RETURN (KillTimer( static_cast<int> (m_idTimer) ));
				m_idTimer = 0;
			}
			
			if( !m_idTimer )
				m_idTimer = SetTimer (1000, 250, NULL);

			IGNORE_RETURN (CImageList::DragShowNolock(TRUE));
		}
	}
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnDestroy() 
{
	if( m_idTimer )
	{
		IGNORE_RETURN (KillTimer (static_cast<int> (m_idTimer)));
		m_idTimer = 0;
	}

	CTreeView::OnDestroy();
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnTimer(UINT nIDEvent) 
{
    if( nIDEvent == m_idTimer )
    {
		if (m_dragMode != DM_nothing)
		{
			if (keyDown (VK_CONTROL))
				m_dragMode = DM_copy;
			else
				m_dragMode = DM_move;

			if (m_dragMode == DM_copy)
				SetCursor (m_copyCursor);
			else
				SetCursor (m_moveCursor);
		}

        CTreeCtrl& theTree = GetTreeCtrl();
        HTREEITEM htiFloat = theTree.GetDropHilightItem();
        if( htiFloat && htiFloat == m_htiDrop )
        {
            if( theTree.ItemHasChildren( htiFloat ) )
                IGNORE_RETURN (theTree.Expand( htiFloat, TVE_EXPAND ));
        }
    }

    CTreeView::OnTimer(nIDEvent);
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnLButtonUpForCopy ()
{
	m_dragMode = DM_nothing;

	IGNORE_RETURN (CImageList::DragLeave (this));
	CImageList::EndDrag ();

	ReleaseCapture();

	delete m_pDragImage;

	IGNORE_RETURN (GetTreeCtrl ().SelectDropTarget (NULL));
	m_htiOldDrop = NULL;

	if (m_idTimer)
	{
		IGNORE_RETURN (KillTimer (static_cast<int> (m_idTimer)));
		m_idTimer = 0;
	}

	//-- exit out if no target was selected to drop on
	if (m_htiDrop == NULL)
		return;

	//-- cancel if we're dragging the root
	if (m_htiDrag == GetTreeCtrl ().GetRootItem ())
		return;
		
	copyBranch (m_htiDrag, m_htiDrop);
}

// ----------------------------------------------------------------------

void ConversationTreeView::copyBranch (HTREEITEM sourceTreeItem, HTREEITEM destinationTreeItem)
{
	//-- get the source item
	SwgConversationEditorDoc::ItemData * const srcItemData = getItemData (sourceTreeItem);
	saveExpandedState (sourceTreeItem);
	
	SwgConversationEditorDoc::ItemData * const dstItemData = getItemData (destinationTreeItem);

	Iff iff (1024);
	iff.allowNonlinearFunctions ();
	srcItemData->m_conversationItem->save (iff);
	iff.goToTopOfForm ();

	switch (srcItemData->m_type)
	{
	case SwgConversationEditorDoc::ItemData::T_branch:
		{
			HTREEITEM dstParentItem = 0;
			ConversationResponse * dstResponse = 0;
			switch (dstItemData->m_type)
			{
			case SwgConversationEditorDoc::ItemData::T_branch:
				{
					//-- get the parent
					dstParentItem = GetTreeCtrl ().GetParentItem (destinationTreeItem);
					SwgConversationEditorDoc::ItemData * const dstParentItemData = getItemData (dstParentItem);
					dstResponse = safe_cast<ConversationResponse *> (dstParentItemData->m_conversationItem);
				}
				break;
			
			case SwgConversationEditorDoc::ItemData::T_response:
				{
					dstParentItem = destinationTreeItem;
					dstResponse = safe_cast<ConversationResponse *> (dstItemData->m_conversationItem);
				}
				break;
			}

			ConversationBranch * const branch = new ConversationBranch(getConversationEditorDoc()->getStringIdTracker());
			branch->load (iff);
			dstResponse->addBranch (branch);
			HTREEITEM treeItem = addBranchToTree (dstParentItem, TVI_LAST, branch);
			selectItem (treeItem);
		}
		break;

	case SwgConversationEditorDoc::ItemData::T_response:
		{
			HTREEITEM dstParentItem = 0;
			ConversationBranch * dstBranch = 0;
			switch (dstItemData->m_type)
			{
			case SwgConversationEditorDoc::ItemData::T_branch:
				{
					dstParentItem = destinationTreeItem;
					dstBranch = safe_cast<ConversationBranch *> (dstItemData->m_conversationItem);
				}
				break;
			
			case SwgConversationEditorDoc::ItemData::T_response:
				{
					//-- get the parent
					dstParentItem = GetTreeCtrl ().GetParentItem (destinationTreeItem);
					SwgConversationEditorDoc::ItemData * const dstParentItemData = getItemData (dstParentItem);
					dstBranch = safe_cast<ConversationBranch *> (dstParentItemData->m_conversationItem);
				}
				break;
			}

			ConversationResponse * const response = new ConversationResponse(getConversationEditorDoc()->getStringIdTracker());
			response->load (iff);
			dstBranch->addResponse (response);
			HTREEITEM treeItem = addResponseToTree (dstParentItem, TVI_LAST, response);
			selectItem (treeItem);
		}
		break;
	}

	Invalidate ();
	GetDocument ()->UpdateAllViews (this);
	GetDocument ()->SetModifiedFlag (true);
}

//-------------------------------------------------------------------

void ConversationTreeView::OnLButtonUpForMove () 
{
	m_dragMode = DM_nothing;

	IGNORE_RETURN (CImageList::DragLeave (this));
	CImageList::EndDrag ();

	ReleaseCapture();

	delete m_pDragImage;

	IGNORE_RETURN (GetTreeCtrl ().SelectDropTarget (NULL));
	m_htiOldDrop = NULL;

	if (m_idTimer)
	{
		IGNORE_RETURN (KillTimer (static_cast<int> (m_idTimer)));
		m_idTimer = 0;
	}

	//-- exit out if no target was selected to drop on
	if (m_htiDrop == NULL)
		return;

	//-- cancel if we're dragging the root
	if (m_htiDrag == GetTreeCtrl ().GetRootItem ())
		return;
		
	//-- don't allow drags of parent layers onto sublayers
	{
		HTREEITEM possibleParent = m_htiDrop;

		do
		{
			possibleParent = GetTreeCtrl ().GetParentItem (possibleParent);

			if (possibleParent == m_htiDrag)
				return;
		}
		while (possibleParent != 0);
	}

	//-- do the source and dest have the same parent?
	if (GetTreeCtrl ().GetParentItem (m_htiDrag) == GetTreeCtrl ().GetParentItem (m_htiDrop))
		return;

	//-- are we dropping onto our existing parent?
	if (GetTreeCtrl ().GetParentItem (m_htiDrag) == m_htiDrop)
		return;

	moveBranch (m_htiDrag, m_htiDrop);
}

void ConversationTreeView::moveBranch (HTREEITEM sourceTreeItem, HTREEITEM destinationTreeItem)
{
	//-- get the source item
	SwgConversationEditorDoc::ItemData * const srcItemData = getItemData (sourceTreeItem);
	saveExpandedState (sourceTreeItem);

	SwgConversationEditorDoc::ItemData * const dstItemData = getItemData (destinationTreeItem);
	
	Iff iff (1024);
	iff.allowNonlinearFunctions ();
	srcItemData->m_conversationItem->save (iff);
	iff.goToTopOfForm ();

	switch (srcItemData->m_type)
	{
	case SwgConversationEditorDoc::ItemData::T_branch:
		{
			HTREEITEM dstParentItem = 0;
			ConversationResponse * dstResponse = 0;
			switch (dstItemData->m_type)
			{
			case SwgConversationEditorDoc::ItemData::T_branch:
				{
					//-- get the parent
					dstParentItem = GetTreeCtrl ().GetParentItem (destinationTreeItem);
					SwgConversationEditorDoc::ItemData * const dstParentItemData = getItemData (dstParentItem);
					dstResponse = safe_cast<ConversationResponse *> (dstParentItemData->m_conversationItem);
				}
				break;
			
			case SwgConversationEditorDoc::ItemData::T_response:
				{
					dstParentItem = destinationTreeItem;
					dstResponse = safe_cast<ConversationResponse *> (dstItemData->m_conversationItem);
				}
				break;
			}

			deleteBranch (sourceTreeItem);
			ConversationBranch * const branch = new ConversationBranch(getConversationEditorDoc()->getStringIdTracker());
			branch->load (iff);
			dstResponse->addBranch (branch);
			HTREEITEM treeItem = addBranchToTree (dstParentItem, TVI_LAST, branch);
			selectItem (treeItem);
		}
		break;

	case SwgConversationEditorDoc::ItemData::T_response:
		{
			HTREEITEM dstParentItem = 0;
			ConversationBranch * dstBranch = 0;
			switch (dstItemData->m_type)
			{
			case SwgConversationEditorDoc::ItemData::T_branch:
				{
					dstParentItem = destinationTreeItem;
					dstBranch = safe_cast<ConversationBranch *> (dstItemData->m_conversationItem);
				}
				break;
			
			case SwgConversationEditorDoc::ItemData::T_response:
				{
					//-- get the parent
					dstParentItem = GetTreeCtrl ().GetParentItem (destinationTreeItem);
					if (!dstParentItem)
						return;

					SwgConversationEditorDoc::ItemData * const dstParentItemData = getItemData (dstParentItem);
					dstBranch = safe_cast<ConversationBranch *> (dstParentItemData->m_conversationItem);
				}
				break;
			}

			deleteResponse (sourceTreeItem);
			ConversationResponse * const response = new ConversationResponse(getConversationEditorDoc()->getStringIdTracker());
			response->load (iff);
			dstBranch->addResponse (response);
			HTREEITEM treeItem = addResponseToTree (dstParentItem, TVI_LAST, response);
			selectItem (treeItem);
		}
		break;
	}

	Invalidate ();
	GetDocument ()->UpdateAllViews (this);
	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_dwDragStart = GetTickCount ();

	UINT flags = 0;
	HTREEITEM treeItem = GetTreeCtrl ().HitTest (point, &flags);
	if (treeItem)
		selectItem (treeItem);
	
	CTreeView::OnLButtonDown(nFlags, point);
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnBranchTest() 
{
	SwgConversationEditorDoc::ItemData * const itemData = getSelectedItemData ();
	if (itemData)
	{
		SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
		NOT_NULL (document);

		Conversation * const conversation = document->getConversation ();
		std::pair <bool, bool> result = document->compile (false, true, false, true, true);
		if (result.first)
		{
			DialogTestConversation dialog (conversation, itemData->m_conversationItem, this);
			dialog.DoModal ();
		}
		else
			if (!result.second)
				MessageBox ("Please correct the errors before testing the conversation.");
	}
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnBranchDelete() 
{
	OnButtonDelete ();
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnResponseDelete() 
{
	OnButtonDelete ();
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnUpdateBranchAddresponse(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (isBranchSelected ());
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnUpdateResponseAddbranch(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (isResponseSelected ());
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnUpdateBranchTest(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (isBranchSelected () || isResponseSelected ());
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnBranchGroupEchoRecursiveOn()
{
	HTREEITEM treeItem = GetTreeCtrl ().GetSelectedItem ();
	SwgConversationEditorDoc::ItemData * const itemData = getItemData (treeItem, VT_branch);

	if(itemData)
	{
		ConversationBranch * const branch = safe_cast<ConversationBranch *> (itemData->m_conversationItem);

		if(branch)
		{
			branch->setGroupEchoRecursive (true);

			GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_groupEchoChanged);
			GetDocument ()->SetModifiedFlag ();
			Invalidate ();
		}
	}
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnBranchGroupEchoRecursiveOff()
{
	HTREEITEM treeItem = GetTreeCtrl ().GetSelectedItem ();
	SwgConversationEditorDoc::ItemData * const itemData = getItemData (treeItem, VT_branch);

	if(itemData)
	{
		ConversationBranch * const branch = safe_cast<ConversationBranch *> (itemData->m_conversationItem);

		if(branch)
		{
			branch->setGroupEchoRecursive (false);

			GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_groupEchoChanged);
			GetDocument ()->SetModifiedFlag ();
			Invalidate ();
		}
	}
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnResponseGroupEchoRecursiveOn()
{
	HTREEITEM treeItem = GetTreeCtrl ().GetSelectedItem ();
	SwgConversationEditorDoc::ItemData * const itemData = getItemData (treeItem, VT_response);

	if(itemData)
	{
		ConversationResponse * const response = safe_cast<ConversationResponse *> (itemData->m_conversationItem);

		if(response)
		{
			response->setGroupEchoRecursive (true);

			GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_groupEchoChanged);
			GetDocument ()->SetModifiedFlag ();
			Invalidate ();
		}
	}
}

// ----------------------------------------------------------------------

void ConversationTreeView::OnResponseGroupEchoRecursiveOff()
{
	HTREEITEM treeItem = GetTreeCtrl ().GetSelectedItem ();
	SwgConversationEditorDoc::ItemData * const itemData = getItemData (treeItem, VT_response);

	if(itemData)
	{
		ConversationResponse * const response = safe_cast<ConversationResponse *> (itemData->m_conversationItem);

		if(response)
		{
			response->setGroupEchoRecursive (false);

			GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_groupEchoChanged);
			GetDocument ()->SetModifiedFlag ();
			Invalidate ();
		}
	}
}

// ======================================================================

