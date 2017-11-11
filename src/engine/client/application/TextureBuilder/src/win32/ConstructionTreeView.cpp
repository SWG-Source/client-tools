// ======================================================================
//
// ConstructionTreeView.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "ConstructionTreeView.h"

#include "Element.h"
#include "resource.h"
#include "sharedFile/TreeFile.h"
#include "TextureBuilder.h"
#include "TextureBuilderDoc.h"

#include <string>
#include <vector>

// ======================================================================

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ======================================================================

namespace
{
	const char *const ms_textureFileFilter = "DDS Files (*.dds)|*.dds|Targa Files (*.tga)|*.tga|All Files (*.*)|*.*||";

	const int ms_elementTypeImageIndex[] =
		{
			 0, // ETI_unknown,
			 1, // ETI_textureGroup
			 2, // ETI_textureSingle,
			 3, // ETI_textureArray1d,
			 4, // ETI_textureArray2d
			 5, // ETI_commandGroup,
			 6, // ETI_commandClearScreen,
			 7, // ETI_commandDrawTexture
			 8, // ETI_imageSlot
			 9, // ETI_destinationTexture
			 9, // ETI_componentAttribute
			 9, // ETI_variableInt,
			 9, // ETI_textureSourceMode
			 9, // ETI_textureWriteMode,
			10, // ETI_variableDefinition
			 9, // ETI_textureArray1dElement
			11, // ETI_region
			 9, // ETI_bool
			 9, // ETI_hue
			 9, // ETI_hueMode
			 9  // ETI_path
		};

	//-- place a -1 when there is no context menu for an element type
	const int ms_elementTypeContextMenuIndex[] =
		{
			-1, // ETI_unknown,
			 0, // ETI_textureGroup
			-1, // ETI_textureSingle,
			 3, // ETI_textureArray1d,
			-1, // ETI_textureArray2d
			 1, // ETI_commandGroup,
			-1, // ETI_commandClearScreen
			-1, // ETI_commandDrawTexture
			-1, // ETI_imageSlot
			-1, // ETI_destinationTexture
			-1, // ETI_componentAttribute
			-1, // ETI_variableInt
			 2, // ETI_textureSourceMode
			 4, // ETI_textureWriteMode
			-1, // ETI_variableDefinition
			 7, // ETI_textureArray1dElement
			 5, // ETI_region
			-1,	// ETI_bool
			-1, // ETI_hue
			 6, // ETI_hueMode
			-1  // ETI_path
		};
}

// ======================================================================
// miscelleneous functions
// ======================================================================

namespace
{
	HTREEITEM FindTreeItemWithDataHelper(CTreeCtrl &treeControl, HTREEITEM item, DWORD dataValue)
	{
		//-- check if this item has the specified value
		if (treeControl.GetItemData(item) == dataValue)
			return item;

		//-- check children
		if (treeControl.ItemHasChildren(item))
		{
			HTREEITEM searchItem = treeControl.GetChildItem(item);
			while (searchItem)
			{
				HTREEITEM targetItem = FindTreeItemWithDataHelper(treeControl, searchItem, dataValue);
				if (targetItem)
					return targetItem;
				else
					searchItem = treeControl.GetNextSiblingItem(searchItem);
			}
		}

		//-- not found
		return NULL;
	}
}

// ----------------------------------------------------------------------

namespace
{
	HTREEITEM FindTreeItemWithData(CTreeCtrl &treeControl, DWORD dataValue)
	{
		//-- check root and all its siblings
		HTREEITEM searchItem = treeControl.GetRootItem();
		while (searchItem)
		{
			HTREEITEM targetItem = FindTreeItemWithDataHelper(treeControl, searchItem, dataValue);
			if (targetItem)
				return targetItem;
			else
				searchItem = treeControl.GetNextSiblingItem(searchItem);
		}

		//-- not found
		return NULL;
	}
}

// ======================================================================
// class ConstructionTreeView
// ======================================================================

IMPLEMENT_DYNCREATE(ConstructionTreeView, CTreeView)

BEGIN_MESSAGE_MAP(ConstructionTreeView, CTreeView)
	//{{AFX_MSG_MAP(ConstructionTreeView)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_CNSTR_TG_ADD_SINGLE, OnTextureGroupAddSingleTexture)
	ON_COMMAND(ID_CNSTR_CG_ADD_DRAWTEX, OnCommandGroupAddDrawTextureCommand)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBeginDrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

ConstructionTreeView::ConstructionTreeView()
:	CTreeView(),
	m_treeImageList(new CImageList()),
	m_dragImageList(0),
	m_leftDragging(false),
	m_dragElement(0),
	m_dropTargetElement(0),
	m_dropCursor(0),
	m_noDropCursor(0)
{
	//-- load cursors
	CWinApp *app   = NON_NULL(AfxGetApp());
	m_dropCursor   = app->LoadCursor(IDC_STANDARD);
	m_noDropCursor = app->LoadCursor(IDC_DROP_NO);
}

// ----------------------------------------------------------------------

ConstructionTreeView::~ConstructionTreeView()
{
	m_noDropCursor      = 0;
	m_dropCursor        = 0;

	m_dropTargetElement = 0;
	m_dragElement       = 0;

	delete m_dragImageList;
	delete m_treeImageList;
}

// ----------------------------------------------------------------------

BOOL ConstructionTreeView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CTreeView::PreCreateWindow(cs);
}

// ----------------------------------------------------------------------

void ConstructionTreeView::OnDraw(CDC* pDC)
{
	TextureBuilderDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
	UNREF(pDC);
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void ConstructionTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}
#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG
void ConstructionTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG
TextureBuilderDoc* ConstructionTreeView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(TextureBuilderDoc)));
	return static_cast<TextureBuilderDoc*>(m_pDocument);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

void ConstructionTreeView::appendTreeItem(HTREEITEM parentTreeItem, const Element &element, bool ensureVisible, HTREEITEM insertAfter)
{
	CTreeCtrl &treeControl = GetTreeCtrl();

	//-- construct tree item for this 
	const unsigned int elementTypeIndex = element.getTypeIndex();
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(static_cast<unsigned int>(0), elementTypeIndex, sizeof(ms_elementTypeImageIndex)/sizeof(ms_elementTypeImageIndex[0]));

	const int normalImageIndex   = ms_elementTypeImageIndex[elementTypeIndex];
	const int selectedImageIndex = normalImageIndex;

	HTREEITEM newTreeItem = treeControl.InsertItem(element.getLabel().c_str(), normalImageIndex, selectedImageIndex, parentTreeItem, insertAfter);

	//-- save pointer to element so we can work with it later
	IGNORE_RETURN(treeControl.SetItemData(newTreeItem, reinterpret_cast<DWORD>(&element)));

	//-- make us visible
	if (ensureVisible)
		IGNORE_RETURN(treeControl.EnsureVisible(newTreeItem));

	//-- add children
	const ElementVector &elementVector  = element.getChildren();
	ElementVector::const_iterator itEnd = elementVector.end();
	for (ElementVector::const_iterator it = elementVector.begin(); it != itEnd; ++it)
		appendTreeItem(newTreeItem, *NON_NULL(*it), ensureVisible, insertAfter);
}

// ----------------------------------------------------------------------

void ConstructionTreeView::buildTree()
{
	IGNORE_RETURN(GetTreeCtrl().DeleteAllItems());

	const ElementVector &elementVector  = GetDocument()->getElements();
	ElementVector::const_iterator itEnd = elementVector.end();
	for (ElementVector::const_iterator it = elementVector.begin(); it != itEnd; ++it)
		appendTreeItem(TVI_ROOT, *NON_NULL(*it));
}

// ----------------------------------------------------------------------

void ConstructionTreeView::appendElementToParent(const Element *parent, Element *element, bool ensureVisible)
{
	NOT_NULL(element);

	HTREEITEM parentItem = FindTreeItemWithData(GetTreeCtrl(), reinterpret_cast<DWORD>(parent));

	if (parentItem)
		appendTreeItem(parentItem, *element, ensureVisible);
	else
		DEBUG_FATAL(true, ("failed to find parent in tree"));
}

// ----------------------------------------------------------------------

void ConstructionTreeView::updateElement(const Element *element, bool ensureVisible)
{
	NOT_NULL(element);

	CTreeCtrl &treeControl = GetTreeCtrl();

	HTREEITEM oldItem = FindTreeItemWithData(treeControl, reinterpret_cast<DWORD>(element));
	if (oldItem)
	{
		//-- get the parent of this item
		HTREEITEM parentItem = treeControl.GetParentItem(oldItem);
		FATAL(!parentItem, ("tree item has no parent item"));

		//-- insert a new version of this item after the old one
		appendTreeItem(parentItem, *element, ensureVisible, oldItem);

		//-- delete the old version
		IGNORE_RETURN(treeControl.DeleteItem(oldItem));
	}
}

// ----------------------------------------------------------------------

void ConstructionTreeView::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();
	
	CTreeCtrl &treeControl = GetTreeCtrl();

	//-- setup tree image list
	const BOOL createSuccess = m_treeImageList->Create(IDB_CONSTRUCTION_TREEVIEW, 15, 1, RGB(255,255,255));
	DEBUG_FATAL(!createSuccess, ("failed to create treeview image list"));
	UNREF(createSuccess);

	IGNORE_RETURN(treeControl.SetImageList(m_treeImageList, TVSIL_NORMAL));
	
	//-- setup tree control style
	LONG treeControlStyle = GetWindowLong(treeControl.m_hWnd, GWL_STYLE);

	// add lines, buttons and lines at root
	treeControlStyle |= TVS_HASLINES;
	treeControlStyle |= TVS_HASBUTTONS;
	treeControlStyle |= TVS_LINESATROOT;

	// set the control
	IGNORE_RETURN(SetWindowLong(treeControl.m_hWnd, GWL_STYLE, treeControlStyle));

	//-- build the tree
	buildTree();
}

// ----------------------------------------------------------------------

void ConstructionTreeView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	UNREF(nFlags);

	// present the submenu for the selected item (if one is present)

	//-- work with the item currently under the cursor
	UINT      flags;

	CTreeCtrl &treeControl = GetTreeCtrl();
	HTREEITEM selectedItem = treeControl.HitTest(point, &flags);
	UNREF(flags);

	if (!selectedItem)
		return;

	// select this item
	IGNORE_RETURN(treeControl.SelectItem(selectedItem));

	Element &element = *NON_NULL(reinterpret_cast<Element*>(treeControl.GetItemData(selectedItem)));

	//-- determine submenu
	const unsigned int elementTypeIndex = element.getTypeIndex();
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(static_cast<unsigned int>(0), elementTypeIndex, sizeof(ms_elementTypeContextMenuIndex)/sizeof(ms_elementTypeContextMenuIndex[0]));

	const int submenuIndex = ms_elementTypeContextMenuIndex[elementTypeIndex];
	if (submenuIndex < 0)
	{
		// no submenu
		return;
	}

	// load the menu
	CMenu      menus;
	const BOOL loadMenuSuccess = menus.LoadMenu(IDR_ELEMENT_CONSTRUCTION_CONTEXT_MENUS);
	FATAL(!loadMenuSuccess, ("failed to load element element construction's context menus"));

	// get the popup submenu
	CMenu *const popupMenu = menus.GetSubMenu(submenuIndex);
	NOT_NULL(popupMenu);

	// display it
	ClientToScreen(&point);
	IGNORE_RETURN(popupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this));

	// CTreeView::OnRButtonDown(nFlags, point);
}

// ----------------------------------------------------------------------

void ConstructionTreeView::OnTextureGroupAddSingleTexture() 
{
	//-- pop up a dialog to retrieve texture name
	TextureBuilderApp &app                 = TextureBuilderApp::getApp();
	const std::string &lastTexturePathname = app.getLastTexturePathname();

	CFileDialog dlg(TRUE, "dds", lastTexturePathname.c_str(), 0, ms_textureFileFilter, this);
	if (dlg.DoModal() == IDOK)
	{
		//-- Get the document.
		TextureBuilderDoc *const doc = GetDocument();

		//-- Get the full pathname to the file.
		const CString fullPathName = dlg.GetPathName();

		//-- Convert full pathname to TreeFile-relative pathname.
		std::string  treeFilePathName;

		const bool fsToTreeSuccess = TreeFile::stripTreeFileSearchPathFromFile(std::string(fullPathName), treeFilePathName);
		if (fsToTreeSuccess)
		{
			//-- Add a new single texture with the given TreeFile-relative pathname.
			doc->addSingleTexture(treeFilePathName);

			//-- Keep track of the last filesystem texture pathname used.
			app.setLastTexturePathname(fullPathName);
		}
		else
		{
			//-- Indicate the specified filename was not in the TreeFile path.
			char buffer[1024];

			sprintf(buffer, "The specified file [%s] is not within the TreeFile search path, ignoring.", static_cast<const char*>(fullPathName));
			MessageBox(buffer, "File Not in TreeFile Path", MB_OK | MB_ICONSTOP);
		}
	}
}

// ----------------------------------------------------------------------

void ConstructionTreeView::OnCommandGroupAddDrawTextureCommand() 
{
	GetDocument()->addDrawTextureCommand();
}

// ----------------------------------------------------------------------

void ConstructionTreeView::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = reinterpret_cast<NM_TREEVIEW*>(pNMHDR);
	*pResult = 0;

	//-- find out what is being dragged
	HTREEITEM dragItem  = pNMTreeView->itemNew.hItem;
	m_dropTargetElement = 0;

	CTreeCtrl &treeControl = GetTreeCtrl();
	IGNORE_RETURN(treeControl.SelectItem(dragItem));

	//-- check if it is a drag source
	m_dragElement = reinterpret_cast<Element*>(treeControl.GetItemData(dragItem));
	if (!m_dragElement || !m_dragElement->isDragSource())
		return;

	//-- get the image list for dragging
	m_dragImageList = treeControl.CreateDragImage(dragItem);
	if(!m_dragImageList)
		return;

	//-- set up drag image rendering
	IGNORE_RETURN(m_dragImageList->BeginDrag(0, CPoint(15, 15)));
	POINT pt = pNMTreeView->ptDrag;
	ClientToScreen(&pt);
	IGNORE_RETURN(CImageList::DragEnter(NULL, pt));
	SetCapture();

	// we are now officially dragging
	m_leftDragging = true;
}

// ----------------------------------------------------------------------

void ConstructionTreeView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CTreeCtrl &treeControl = GetTreeCtrl();

	if (m_leftDragging)
	{
		POINT pt = point;
		ClientToScreen(&pt);
		IGNORE_RETURN(CImageList::DragMove(pt));

		UINT      flags;
		HTREEITEM hitem = treeControl.HitTest(point, &flags);

		if (!hitem)
			m_dropTargetElement = 0;
		else
		{
			IGNORE_RETURN(CImageList::DragShowNolock(FALSE));

			//-- get the element for the tree item
			m_dropTargetElement = reinterpret_cast<Element*>(treeControl.GetItemData(hitem));

			//-- we can't drop on top of the same element
			if (m_dropTargetElement == m_dragElement)
			{
				m_dropTargetElement = 0;
				hitem               = 0;
			}

			//-- check if this item is eligible to be a drag-n-drop target given the
			//   item we're currently dragging.
			if (m_dropTargetElement && !m_dropTargetElement->isDragTarget(*NON_NULL(m_dragElement)))
			{
				m_dropTargetElement = 0;
				hitem               = 0;
			}

			IGNORE_RETURN(treeControl.SelectDropTarget(hitem));
			IGNORE_RETURN(CImageList::DragShowNolock(TRUE));
		}

		if (m_dropTargetElement)
			SetCursor(m_dropCursor);
		else
			SetCursor(m_noDropCursor);
	}

	CTreeView::OnMouseMove(nFlags, point);
} //lint !e1746 // parameter 'point' could be made const // can't change that

// ----------------------------------------------------------------------

void ConstructionTreeView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CTreeView::OnLButtonUp(nFlags, point);

	if (m_leftDragging)
	{
		//-- turn off dragging
		m_leftDragging = false;

		IGNORE_RETURN(CImageList::DragLeave(this));
		CImageList::EndDrag();

		ReleaseCapture();
		delete m_dragImageList;
		m_dragImageList = 0;

		//-- tell the doc about the drag
		if (m_dragElement && m_dropTargetElement)
			GetDocument()->notifyElementDragAndDrop(*NON_NULL(m_dragElement), *NON_NULL(m_dropTargetElement));
	}
} //lint !e1746 // parameter 'point' could be made const // can't change that

// ----------------------------------------------------------------------

void ConstructionTreeView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	UNREF(nFlags);

	UINT       hitFlags;
	CTreeCtrl &treeControl = GetTreeCtrl();
	HTREEITEM  item        = treeControl.HitTest(point, &hitFlags);

	if (!item)
		return;

	Element *const element = reinterpret_cast<Element*>(treeControl.GetItemData(item));
	if (!element)
		return;

	GetDocument()->notifyElementLeftDoubleClick(*element);

	// CTreeView::OnLButtonDblClk(nFlags, point);
} //lint !e1746 // parameter 'point' could be made const // can't change that

// ----------------------------------------------------------------------

Element *ConstructionTreeView::getSelectedElement()
{
	//-- Work with the item currently under the cursor.
	CTreeCtrl &treeControl = GetTreeCtrl();
	HTREEITEM selectedItem = treeControl.GetSelectedItem();

	if (!selectedItem)
		return 0;

	return reinterpret_cast<Element*>(treeControl.GetItemData(selectedItem));
}

// ----------------------------------------------------------------------

Element *ConstructionTreeView::getSelectedElementParent()
{
	//-- Work with the item currently under the cursor.
	CTreeCtrl &treeControl = GetTreeCtrl();
	HTREEITEM selectedItem = treeControl.GetSelectedItem();
	HTREEITEM parentItem   = (selectedItem ? treeControl.GetParentItem(selectedItem) : 0);

	if (!parentItem)
		return 0;

	return reinterpret_cast<Element*>(treeControl.GetItemData(parentItem));
}

// ----------------------------------------------------------------------
/**
 * Retrieve the 0-based index of the selected element relative to the
 * parent's child list.
 *
 * @return  the 0-based index that indicates where the selected item fits
 *          within the ordered list of the parent's direct children.
 */

int ConstructionTreeView::getSelectedElementChildIndex() const
{
	int index = 0;

	//-- Work with the item currently under the cursor.
	CTreeCtrl &treeControl = GetTreeCtrl();
	HTREEITEM item         = treeControl.GetSelectedItem();

	//-- Add one to index for each sibling in the list.
	while (item)
	{
		// Get the previous sibling under the same parent.
		item = treeControl.GetPrevSiblingItem(item);
		if (item)
			++index;
	}

	return index;
}

// ----------------------------------------------------------------------

void ConstructionTreeView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CTreeCtrl &treeControl = GetTreeCtrl();

	//-- find the item under point and select it
	UINT      flags;
	HTREEITEM hitem = treeControl.HitTest(point, &flags);
	UNREF(flags);
	
	if (hitem)
	{
		// -TRF- don't really think I need to do this, default handler should do it
		// GetTreeCtrl().SelectItem(hitem);

		Element *element = reinterpret_cast<Element*>(treeControl.GetItemData(hitem));
		if (element)
		{
			// notify doc
			GetDocument()->notifyElementSelected(*element);
		}
	}

	CTreeView::OnLButtonDown(nFlags, point);
} //lint !e1746 // parameter 'point' could be made const // can't change that

// ----------------------------------------------------------------------
/**
 * Delete the selected Element in the construction tree view if allowable.
 */

void ConstructionTreeView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	//-- Look for a delete keypress.
	DEBUG_REPORT_LOG(true, ("char value [%d]\n", nChar));

	if (nChar == 46) // delete button --- what's the constant for this?
	{
		//-- Get the selected element.
		CTreeCtrl &treeControl = GetTreeCtrl();
		HTREEITEM selectedItem = treeControl.GetSelectedItem();

		if (!selectedItem)
			return;

		Element *const element = reinterpret_cast<Element*>(treeControl.GetItemData(selectedItem));
		if (!element || !element->userCanDelete())
			return;

		Element *const elementParent = getSelectedElementParent();

		//-- Give element a chance to do any special processing required to delete it.
		element->doPreDeleteTasks();

		//-- Handle element deletion.

		// Remove from the tree view.
		BOOL const result = treeControl.DeleteItem(selectedItem);
		DEBUG_WARNING(!result, ("treeControl.DeleteItem failed."));

		// Try to remove from the parent's element list.  The doPreDeleteTasks() may have done this for
		// us for complicated parent-child relationships.
		if (elementParent)
		{
			Element::ElementVector &children = elementParent->getChildren();
			Element::ElementVector::iterator findIt = std::find(children.begin(), children.end(), element);
			if (findIt != children.end())
				children.erase(findIt);
			else
				DEBUG_REPORT_LOG(true, ("note: element to be deleted does not exist in parent's child list.\n"));
		}

		// Delete the element.
		delete element;

		// Tell construction view to redraw itself.
		buildTree();
		InvalidateRect(NULL);
	}
	
	CTreeView::OnKeyUp(nChar, nRepCnt, nFlags);
}

// ======================================================================
