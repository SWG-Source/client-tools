// ======================================================================
//
// DraftSchematicTreeView.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgDraftSchematicEditor.h"
#include "DraftSchematicTreeView.h"

#include "ChildFrame.h"
#include "Resource.h"
#include "SwgDraftSchematicEditorDoc.h"

// ======================================================================

namespace DraftSchematicTreeViewNamespace
{
	enum DraftSchematicTreeViewIconType
	{
		DSTVIT_property,
		DSTVIT_attributes,
		DSTVIT_attribute,
		DSTVIT_slots,
		DSTVIT_requiredSlot,
		DSTVIT_optionalSlot,
	};
}

using namespace DraftSchematicTreeViewNamespace;

// ======================================================================

IMPLEMENT_DYNCREATE(DraftSchematicTreeView, CTreeView)

// ----------------------------------------------------------------------

DraftSchematicTreeView::DraftSchematicTreeView() :
	CTreeView (),
	m_imageListSet (false),
	m_imageList (),
	m_rootProperty (0),
	m_rootSlots (0),
	m_rootAttributes (0)
{
}

// ----------------------------------------------------------------------

DraftSchematicTreeView::~DraftSchematicTreeView()
{
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DraftSchematicTreeView, CTreeView)
	//{{AFX_MSG_MAP(DraftSchematicTreeView)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_TREE_ADDATTRIBUTE, OnTreeAddattribute)
	ON_COMMAND(ID_TREE_ADDSLOT, OnTreeAddslot)
	ON_COMMAND(ID_TREE_DELETE, OnTreeDelete)
	ON_NOTIFY_REFLECT(TVN_DELETEITEM, OnDeleteitem)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

#ifdef _DEBUG
void DraftSchematicTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void DraftSchematicTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

void DraftSchematicTreeView::addAttributeToTree (DraftSchematic::Attribute * const attribute)
{
	SwgDraftSchematicEditorDoc::ItemData * const itemData = new SwgDraftSchematicEditorDoc::ItemData;
	itemData->m_type = SwgDraftSchematicEditorDoc::ItemData::T_attribute;
	itemData->m_attribute = attribute;
	HTREEITEM treeItem = GetTreeCtrl ().InsertItem (itemData->getName (), DSTVIT_attribute, DSTVIT_attribute, m_rootAttributes);
	itemData->m_treeItem = treeItem;
	GetTreeCtrl ().SetItemData (treeItem, reinterpret_cast<DWORD> (itemData));
}

// ----------------------------------------------------------------------

void DraftSchematicTreeView::addSlotToTree (DraftSchematic::Slot * const slot)
{
	SwgDraftSchematicEditorDoc::ItemData * const itemData = new SwgDraftSchematicEditorDoc::ItemData;
	itemData->m_type = SwgDraftSchematicEditorDoc::ItemData::T_slot;
	itemData->m_slot = slot;
	HTREEITEM treeItem = GetTreeCtrl ().InsertItem (itemData->getName (), slot->m_optional ? DSTVIT_optionalSlot : DSTVIT_requiredSlot, slot->m_optional ? DSTVIT_optionalSlot : DSTVIT_requiredSlot, m_rootSlots);
	itemData->m_treeItem = treeItem;
	GetTreeCtrl ().SetItemData (treeItem, reinterpret_cast<DWORD> (itemData));
}

// ----------------------------------------------------------------------

void DraftSchematicTreeView::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();

	if (!m_imageListSet)
	{
		m_imageList.Create (IDB_BITMAP_DRAFTSCHEMATIC, 16, 1, RGB (255,255,255));
		GetTreeCtrl ().SetImageList (&m_imageList, TVSIL_NORMAL);

		m_imageListSet = true;
	}

	SwgDraftSchematicEditorDoc * const document = safe_cast<SwgDraftSchematicEditorDoc *> (GetDocument ());
	DraftSchematic & draftSchematic = document->getDraftSchematic ();

	m_rootProperty = GetTreeCtrl ().InsertItem ("Properties", DSTVIT_property, DSTVIT_property);
	GetTreeCtrl ().Expand (m_rootProperty, TVE_EXPAND);

	m_rootSlots = GetTreeCtrl ().InsertItem ("Slots", DSTVIT_slots, DSTVIT_slots);
	{
		for (int i = 0; i < draftSchematic.getNumberOfSlots (); ++i)
			addSlotToTree (draftSchematic.getSlot (i));
	}
	GetTreeCtrl ().Expand (m_rootSlots, TVE_EXPAND);

	m_rootAttributes = GetTreeCtrl ().InsertItem ("Attributes", DSTVIT_attributes, DSTVIT_attributes);
	{
		for (int i = 0; i < draftSchematic.getNumberOfAttributes (); ++i)
			addAttributeToTree (draftSchematic.getAttribute (i));
	}
	GetTreeCtrl ().Expand (m_rootAttributes, TVE_EXPAND);

	selectItem (m_rootProperty);
}

// ----------------------------------------------------------------------

void DraftSchematicTreeView::OnSelchanged (NMHDR * const /*pNMHDR*/, LRESULT * const pResult) 
{
	HTREEITEM treeItem = GetTreeCtrl ().GetSelectedItem ();
	if (treeItem == m_rootProperty)
		safe_cast<ChildFrame *> (GetParentFrame ())->setViewProperty ();
	else
		if (treeItem == m_rootSlots)
			safe_cast<ChildFrame *> (GetParentFrame ())->setViewEmpty ();
		else
			if (treeItem == m_rootAttributes)
				safe_cast<ChildFrame *> (GetParentFrame ())->setViewEmpty ();
			else
				if (GetTreeCtrl ().GetParentItem (treeItem) == m_rootSlots)
					safe_cast<ChildFrame *> (GetParentFrame ())->setViewSlots ();
				else
					if (GetTreeCtrl ().GetParentItem (treeItem) == m_rootAttributes)
						safe_cast<ChildFrame *> (GetParentFrame ())->setViewAttributes ();
					else
						safe_cast<ChildFrame *> (GetParentFrame ())->setViewEmpty ();

	*pResult = 0;
}

// ----------------------------------------------------------------------

BOOL DraftSchematicTreeView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= (TVS_SHOWSELALWAYS | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS);
	
	return CTreeView::PreCreateWindow(cs);
}

// ----------------------------------------------------------------------

SwgDraftSchematicEditorDoc::ItemData * DraftSchematicTreeView::getItemData (HTREEITEM treeItem)
{
	return reinterpret_cast<SwgDraftSchematicEditorDoc::ItemData *> (GetTreeCtrl ().GetItemData (treeItem));
}

// ----------------------------------------------------------------------

SwgDraftSchematicEditorDoc::ItemData * DraftSchematicTreeView::getSelectedItemData ()
{
	return getItemData (GetTreeCtrl ().GetSelectedItem ());
}

// ----------------------------------------------------------------------

void DraftSchematicTreeView::selectItem (HTREEITEM const treeItem) 
{
	if (treeItem)
	{
		GetTreeCtrl ().SelectItem (treeItem);
		GetTreeCtrl ().EnsureVisible (treeItem);

		GetDocument ()->UpdateAllViews (this);
	}
}

// ----------------------------------------------------------------------

void DraftSchematicTreeView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CTreeView::OnRButtonDown(nFlags, point);

	UINT flags = 0;
	HTREEITEM treeItem = GetTreeCtrl ().HitTest (point, &flags);
	if (treeItem)
	{
		selectItem (treeItem);

		CPoint pt = point;
		ClientToScreen (&pt);

		CMenu menu;
		menu.LoadMenu (IDR_MENU_CONTEXT);

		CMenu* rootMenu = menu.GetSubMenu (0);
		CMenu* subMenu = rootMenu->GetSubMenu (0);
		subMenu->TrackPopupMenu (TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	}
}

// ----------------------------------------------------------------------

void DraftSchematicTreeView::OnUpdate(CView * const pSender, LPARAM const lHint, CObject * const pHint) 
{
	if (pSender != this)
	{
		switch (lHint)
		{
		case SwgDraftSchematicEditorDoc::H_nothing:
		default:
			break;

		case SwgDraftSchematicEditorDoc::H_nameChanged:
			{
				HTREEITEM treeItem = reinterpret_cast<HTREEITEM> (pHint);
				if (treeItem)
				{
					SwgDraftSchematicEditorDoc::ItemData const * const itemData = getItemData (treeItem);
					GetTreeCtrl ().SetItemText (treeItem, itemData->getName ());
				}
			}
			break;

		case SwgDraftSchematicEditorDoc::H_slotChanged:
			refreshIcons ();
			break;
		}
	}
}

// ----------------------------------------------------------------------

void DraftSchematicTreeView::OnTreeAddattribute() 
{
	SwgDraftSchematicEditorDoc * const document = safe_cast<SwgDraftSchematicEditorDoc *> (GetDocument ());
	DraftSchematic & draftSchematic = document->getDraftSchematic ();
	DraftSchematic::Attribute * const attribute = new DraftSchematic::Attribute;
	DraftSchematic::createDefaultAttribute (*attribute);
	draftSchematic.addAttribute (attribute);
	addAttributeToTree (attribute);

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void DraftSchematicTreeView::OnTreeAddslot() 
{
	SwgDraftSchematicEditorDoc * const document = safe_cast<SwgDraftSchematicEditorDoc *> (GetDocument ());
	DraftSchematic & draftSchematic = document->getDraftSchematic ();
	DraftSchematic::Slot * const slot = new DraftSchematic::Slot;
	DraftSchematic::createDefaultSlot (*slot);
	draftSchematic.addSlot (slot);
	addSlotToTree (slot);

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void DraftSchematicTreeView::OnTreeDelete() 
{
	HTREEITEM treeItem = GetTreeCtrl ().GetSelectedItem ();
	if (treeItem)
	{
		HTREEITEM parentTreeItem = GetTreeCtrl ().GetParentItem (treeItem);
		if (parentTreeItem)
		{
			SwgDraftSchematicEditorDoc::ItemData * const itemData = getItemData (treeItem);
			if (itemData)
			{
				if ((itemData->m_type == SwgDraftSchematicEditorDoc::ItemData::T_attribute) && ((itemData->m_attribute->m_nameStringTable == "crafting" && itemData->m_attribute->m_nameStringId == "complexity") || (itemData->m_attribute->m_nameStringTable == "crafting" && itemData->m_attribute->m_nameStringId == "xp")))
					MessageBox ("You cannot delete this item.");
				else
				{
					CString message;
					message.Format ("Are you sure you want to delete %s?", itemData->getName ());

					if (MessageBox (message, 0, MB_YESNO) == IDYES)
					{
						SwgDraftSchematicEditorDoc * const document = safe_cast<SwgDraftSchematicEditorDoc *> (GetDocument ());
						DraftSchematic & draftSchematic = document->getDraftSchematic ();

						switch (itemData->m_type)
						{
						case SwgDraftSchematicEditorDoc::ItemData::T_attribute:
							{
								draftSchematic.removeAttribute (itemData->m_attribute);
								delete itemData->m_attribute;

								HTREEITEM siblingItem = GetTreeCtrl ().GetPrevSiblingItem (treeItem);
								selectItem (siblingItem ? siblingItem : parentTreeItem);
								GetTreeCtrl ().DeleteItem (treeItem);

								GetDocument ()->SetModifiedFlag ();
								Invalidate ();
							}
							break;

						case SwgDraftSchematicEditorDoc::ItemData::T_slot:
							{
								draftSchematic.removeSlot (itemData->m_slot);
								delete itemData->m_slot;

								HTREEITEM siblingItem = GetTreeCtrl ().GetPrevSiblingItem (treeItem);
								selectItem (siblingItem ? siblingItem : parentTreeItem);
								GetTreeCtrl ().DeleteItem (treeItem);

								GetDocument ()->SetModifiedFlag ();
								Invalidate ();
							}
							break;
						}
					}
				}
			}
			else
				MessageBox ("You cannot delete this item.");
		}
		else
			MessageBox ("You cannot delete this item.");
	}
}

// ----------------------------------------------------------------------

void DraftSchematicTreeView::OnDeleteitem(NMHDR * const pNMHDR, LRESULT * const pResult) 
{
	NM_TREEVIEW * const pNMTreeView = reinterpret_cast<NM_TREEVIEW *> (pNMHDR);

	//-- get the treeitem to delete
	HTREEITEM const treeItem = pNMTreeView->itemOld.hItem;

	//-- delete the itemdata associated with the treeitem
	SwgDraftSchematicEditorDoc::ItemData * const itemData = getItemData (treeItem);

	if (itemData)
		delete itemData;

	//-- set the data to 0
	GetTreeCtrl ().SetItemData (treeItem, 0);

	*pResult = 0;
}

// ----------------------------------------------------------------------

void DraftSchematicTreeView::refreshIcons ()
{
	HTREEITEM current = GetTreeCtrl ().GetRootItem ();

	if (current)
	{
		do
		{
			refreshIcons (current);

			current = GetTreeCtrl ().GetNextSiblingItem (current);
		} 
		while (current);
	}
}

// ----------------------------------------------------------------------

void DraftSchematicTreeView::refreshIcons (HTREEITEM treeItem)
{
	//-- refresh me
	DraftSchematicTreeViewIconType icon = DSTVIT_property;

	SwgDraftSchematicEditorDoc::ItemData const * const itemData = getItemData (treeItem);
	if (itemData)
	{
		if (itemData->m_type == SwgDraftSchematicEditorDoc::ItemData::T_attribute)
			icon = DSTVIT_attribute;
		else
			if (itemData->m_type == SwgDraftSchematicEditorDoc::ItemData::T_slot)
				icon = itemData->m_slot->m_optional ? DSTVIT_optionalSlot : DSTVIT_requiredSlot;
	}
	else
	{
		if (treeItem == m_rootProperty)
			icon = DSTVIT_property;
		else
			if (treeItem == m_rootAttributes)
				icon = DSTVIT_attributes;
			else
				if (treeItem == m_rootSlots)
					icon = DSTVIT_slots;
	}

	GetTreeCtrl ().SetItemImage (treeItem, icon, icon);

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

// ======================================================================

