// ObjectBrowserDialog.cpp : implementation file
//

#include "FirstUiBuilder.h"
#include "EditUtils.h"
#include "ObjectBrowserDialog.h"
#include "ObjectEditor.h"
#include "UIBuilder.h"
#include "UserWindowsMessages.h"

#include "UIBaseObject.h"
#include "UISliderbarStyle.h"
#include "UIScrollbar.h"
#include "UISliderbar.h"
#include "UIUtils.h"


#include <list>

// global dialog allocator.
//ObjectBrowserDialog *newObjectBrowserDialog(ObjectEditor &i_model) { return new ObjectBrowserDialog(i_model); }

extern CUiBuilderApp theApp;

/////////////////////////////////////////////////////////////////////////////
// ObjectBrowserDialog dialog

// ==========================================================

// Get a browser handle from a object.
void ObjectBrowserDialog::_setHandle(UIBaseObject &object, HTREEITEM h) 
{ 
	uiTreeItems[&object]=h;
}

// ==========================================================

// Get a object form a browser handle.
HTREEITEM ObjectBrowserDialog::_getHandle(UIBaseObject &object) 
{ 
	return uiTreeItems[&object];
}

// ==========================================================

// Associate a object and browser handle together.
void ObjectBrowserDialog::_associate(UIBaseObject &object, HTREEITEM h)
{
   _setHandle(object, h);
   tree.SetItemData(h, (DWORD)(&object));
}

// ==========================================================

// Disassociate a object and browser handle..
void ObjectBrowserDialog::_dissociate(UIBaseObject &object)
{
   HTREEITEM h=_getHandle(object);
   _setHandle(object, 0);
   if (h)
	{
		tree.SetItemData(h, 0);
	}
}

// ==========================================================

ObjectBrowserDialog::ObjectBrowserDialog(ObjectEditor &i_model, CWnd* pParent, CWnd* pOwner)
:	CDialog(ObjectBrowserDialog::IDD, pParent), model(i_model),
	accelerators(0),
	showActive(false)
{
	//{{AFX_DATA_INIT(ObjectBrowserDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
   root=0;
   dragitem=0;
   drop_target=0;
   context_target=0;
   prev_cursor=0;
   lock_ref=0;
   supress_sel_changes=false;
   Create(IDD, pParent);

	if (pParent)
	{
		SetParent(pParent);
	}

	if (pOwner)
	{
		SetOwner(pOwner);
	}
}

// ==========================================================

ObjectBrowserDialog::~ObjectBrowserDialog() 
{ 
}

// ==========================================================

void ObjectBrowserDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ObjectBrowserDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

// ==========================================================

void ObjectBrowserDialog::lock()
{
   if (!lock_ref)
	{
		tree.SetRedraw(false);
	}
   lock_ref++;
}

// ==========================================================

void ObjectBrowserDialog::unlock()
{
   assert(lock_ref>0);
   lock_ref--;

   // if lock ref has returned back to zero, re-enable redraws
   // and invalidate this window.
   if (!lock_ref)
   {
		tree.SetRedraw(true);
   }
}

// ==========================================================

void ObjectBrowserDialog::onEditReset()
{
	_removeTree();
}

// ==========================================================

void ObjectBrowserDialog::onEditInsertSubtree(UIBaseObject &subTree, UIBaseObject *previousSibling)
{
	UIBaseObject *parent = subTree.GetParent();
	if (parent)
	{
		_insertObject(subTree, previousSibling, true);
	}
	else
	{
		_setRoot(&subTree);
	}
}

// ==========================================================

void ObjectBrowserDialog::onEditRemoveSubtree(UIBaseObject &subTree)
{
	_removeObject(subTree, true);
}

// ==========================================================

void ObjectBrowserDialog::onEditMoveSubtree(UIBaseObject &subTree, UIBaseObject *previousSibling, UIBaseObject *oldParent)
{
   // heirarchy change - just remove and re-insert the object into the dialog.
	_removeObject(subTree, true);
	_insertObject(subTree, previousSibling, true);
}

// ==========================================================

void ObjectBrowserDialog::onEditSetObjectProperty(UIBaseObject &object, const char *i_propertyName)
{
	if (!_stricmp(i_propertyName, "name"))
	{
		// the object's name has changed.
		_retextObject(object);
	}
}

// ==========================================================

void ObjectBrowserDialog::onSelect(UIBaseObject &object, bool isSelected)
{
   const HTREEITEM item=_getHandle(object);
   if (item)
	{
      tree.selectItem(item, isSelected);
	}
}

// ==========================================================

void ObjectBrowserDialog::saveUserPreferences()
{
	if (IsWindowVisible())
	{
		CRect windowRect;
		GetWindowRect(windowRect);
		theApp.saveDialogPosition("ObjectBrowserDialog", windowRect);
	}
}

// ==========================================================

void ObjectBrowserDialog::setActiveAppearance(bool i_showActive)
{
	if (i_showActive!=showActive)
	{
		showActive=i_showActive;
		PostMessage(WM_NCACTIVATE, showActive);
	}
}

// ==========================================================

void ObjectBrowserDialog::_makeDisplayString(char *o_string, UIBaseObject &i_object)
{
	if (i_object.GetName().empty())
	{
		strcpy(o_string, "Unnamed Object");
	}
	else
	{
		strcpy(o_string, i_object.GetName().c_str());
	}
}

// ==========================================================

int ObjectBrowserDialog::_getImage(UIBaseObject &i_object)
{
	if (i_object.IsA(TUIScrollbar))
	{
		UISliderbarStyle * const pStyle = static_cast<UIScrollbar *>(&i_object)->GetSliderbarStyle();
		
		if (pStyle)
		{
			if (pStyle->GetLayout() == UIStyle::L_horizontal)
			{
				return 7;
			}
			else
			{
				return 8;
			}
		}
		else
		{
			return 7;
		}
	}
	else if (i_object.IsA(TUISliderbar))
	{
		UISliderbarStyle * const pStyle = static_cast<UISliderbar *>(&i_object)->GetSliderbarStyle();
		
		if (pStyle)
		{
			if (pStyle->GetLayout() == UIStyle::L_horizontal)
			{
				return 15;
			}
			else
			{
				return 16;
			}
		}
		else
		{
			return 15;
		}
	}
	else
	{
		static struct
		{
			UITypeID	Type;
			int       ImageID;
		}
		TypeToImageMap[] =
		{
			{  TUIDeformerHUD,       0 },
			{  TUIDeformerRotate,    0 },
			{  TUIDeformerWave,      0 },
			{  TUIText,              2 },
			{  TUITextbox,           3 },
			{  TUIAnimation,         4 },
			{  TUICheckbox,          5 },
			{  TUIEllipse,           4 },
			{  TUIImage,             6 },
			{  TUIButton,            9 },
			{  TUIProgressbar,      10 },
			{  TUIListbox,          11 },
			{  TUIList,             11 },
			{  TUIComboBox,         12 },
			{  TUIDropdownbox,      12 },
			{  TUINamespace,        13 },
			{  TUITooltipStyle,     17 },
			{  TUICursor,           18 },
			{  TUIDataSource,       19 },
			{  TUIData,             20 },
			{  TUITemplate,         21 },
			{  TUIImageFrame,       23 },    // Frame is a subclass of Fragment so check for it first
			{  TUIImageFragment,    22 },
			{  TUITextStyle,        24 },
			{  TUIFontCharacter,    25 },
			{  TUIClock,            26 },
			{  TUIGrid,             27 },
			{  TUIUnknown,          28 },
			{  TUISliderplane,      29 },
			{  TUITabSet,           30 },
			{  TUITabbedPane,       30 },
			{  TUIRectangleStyle,   31 },
			{  TUIPopupMenu,        32 },
			{  TUIVolumePage,       33 },
			{  TUIRadialMenu,       34 },
			{  TUICursorSet,        35 },
			{  TUITable,            36 },
			{  TUITableModelDefault,37 },
			{  TUIDataSourceContainer, 38 },
			{  TUITableHeader,       39 },
			{  TUIComposite,         40 },
			{  TUITreeView,          41 },
			{  TUIPie,               42 },
			{  TUIRunner,            43 },
			{  TUIPalette,           44 },
			{  TUIPage,              1 },    // some items are subclasses of pages, so check for it last
			{  TUIObject,            0 }     // Last because it's the fallback
		};

		for (int i = 0; i < sizeof( TypeToImageMap ) / sizeof( TypeToImageMap[0] ); ++i)
		{
			if (i_object.IsA( TypeToImageMap[i].Type))
			{
				return TypeToImageMap[i].ImageID;
			}
		}
	}

	return 0;
}

// ==========================================================

void ObjectBrowserDialog::_insertObject(UIBaseObject &object, UIBaseObject *previousSibling, bool recurse)
{
   UIBaseObject *parent;
   HTREEITEM hParent, hNewItem;

	if (_getHandle(object))
	{
	   assert(!"Double insert.");
		return;
	}

	// If an object is not legal to browse, 
	// add that check and return from here if necessary.
	if (object.IsA(TUIWidget) && UI_ASOBJECT(UIWidget, &object)->IsTransient())
	{
		return;
	}

   // get parent coordinate system pointer and browser handle.
   parent=object.GetParent();
   if(!parent)
	{
      hParent=TVI_ROOT;
	}
   else
   {
      hParent=_getHandle(*parent);
      assert(hParent);
   }

	// ---------------------------------------------------

	HTREEITEM hInsertAfter;
	if (previousSibling==EM_INSERT_LAST)
	{
		hInsertAfter=TVI_LAST;
	}
	else if (previousSibling==EM_INSERT_FIRST)
	{
		hInsertAfter=TVI_FIRST;
	}
	else
	{
		hInsertAfter=_getHandle(*previousSibling);
		if (!hInsertAfter)
		{
			return;
		}
	}

	// ---------------------------------------------------

   // get our display string.
	char displayString[256];
	_makeDisplayString(displayString, object);

   // get our icon index.
   int i = _getImage(object);

   // insert the browser handle into the tree.
   hNewItem=tree.InsertItem(displayString, i, i, hParent, hInsertAfter);

   if (model.isSelected(object))
	{
		tree.selectItem(hNewItem, true);
	}

   // associate the new handle and object together.
   _associate(object, hNewItem);

	// -----------------------------------
	if (recurse)
	{
		UIBaseObject::UIObjectList children;
		EditUtils::getChildren(children, object);
		for (UIBaseObject::UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
		{
			UIBaseObject *c = *ci; assert(c);
			_insertObject(*c, (UIBaseObject *)EM_INSERT_LAST, true);
		}
	}
}

// ==========================================================

void ObjectBrowserDialog::_removeObject(UIBaseObject &object, bool recurse)
{
	if (recurse)
	{
		UIBaseObject::UIObjectList children;
		EditUtils::getChildren(children, object);
		for (UIBaseObject::UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
		{
			UIBaseObject *c = *ci; assert(c);
			_removeObject(*c, true);
		}
	}

   HTREEITEM hji;

   hji=_getHandle(object);
   if (hji)
   {
      supress_sel_changes=true;
      _dissociate(object);
      tree.DeleteItem(hji);
      supress_sel_changes=false;
   }
}

// ==========================================================

void ObjectBrowserDialog::_retextObject(UIBaseObject &object)
{
   HTREEITEM item;
	char displayString[256];

   // make a new display string and set it into the corresponding browser
   // item.
	_makeDisplayString(displayString, object);
   item=_getHandle(object);
   if (item)
	{
      tree.SetItemText(item, displayString);
	}
}

// ==========================================================

void ObjectBrowserDialog::_removeTree()
{
   if (_isAttached())
   {
      // clear entire dialog.
      ShowWindow(SW_HIDE); // hide window to avoid 50 million redraws.
      UIBaseObject *r;
      r=root;
      root=0; // so _isAttached() returns false from now on.
      _removeObject(*r, true);
      ShowWindow(SW_SHOWNORMAL); // show window again.
   }
}

// ==========================================================

// insert a new model object tree into this dialog.
void ObjectBrowserDialog::_setRoot(UIBaseObject *object)
{
   if (object)
   {
      _insertObject(*object, (UIBaseObject *)EM_INSERT_LAST, true);
      root=object;
   }
}

// ==========================================================

// Drag and drop re-heirarchying support.  
// This initiates a drag.
bool ObjectBrowserDialog::_beginDrag(HTREEITEM h, CPoint p)
{
   dragitem=h;  // remember our drag-item for later.
   if (dragitem)
   {
      SetCapture(); // capture the mouse.

      // load up the drag-n-drop cursor.
      prev_cursor=SetCursor(LoadCursor(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_MULTIDROP)));

      return true;
   }

   // failure case.
   dragitem=0;
   return false;
}

// ==========================================================

void ObjectBrowserDialog::_endDrag()
{ 
   // if a valid drop-targed was found, clear the drop state.
   if (drop_target)
   {
      tree.SetItemState(drop_target, 0, TVIS_DROPHILITED);
      drop_target=0;
   }
   dragitem=0; // clear our drag handle.
   SetCursor(prev_cursor); // restore cursor.
   ReleaseCapture();  // release mouse.
} 

// ==========================================================

// This function executes the actual drop of a dragged item.
void ObjectBrowserDialog::_performDragDrop()
{
   if (  _isDragging() 
		&& drop_target 
		&& !tree.isSelected(drop_target)
		)
   {
      UIBaseObject *const parent =_getObject(drop_target);
		if (parent)
		{
			std::vector<UIBaseObject *> sels;

			// get a list of the selected joints
			for (HTREEITEM iter=tree.getFirstSelectedItem();iter;iter=tree.getNextSelectedItem(iter))
			{
				if (iter!=drop_target)
				{
					sels.push_back(_getObject(iter));
				}
			}

			// clear the current browser selections.
			tree.clearSelections();

			std::vector<UIBaseObject *> userSels = sels;
			EditUtils::removeDescendants(sels);

			if (model.setObjectsParent(sels, *parent))
			{
				// select the parent item that the new items were dropped under.
				tree.SelectItem(drop_target);
			}
			/*
			else
			{
				// parent change failed - re-select original 
				for (unsigned i=0;i<userSels.size();i++)
				{
					model.select(userSels[i]);
				}
			}
			*/
		}
   }
}

// ==========================================================

// This function implements support for the right-click context menu.
void ObjectBrowserDialog::_contextMenu(HTREEITEM h, CPoint pt)
{
   if (!_isAttached()) return;

   UIBaseObject *obj;
   obj=_getObject(h);
   if (obj!=root)
   {
      CMenu menu, *popup;

      menu.LoadMenu(IDR_CONTEXTMENU);
      popup=menu.GetSubMenu(0);
      if (popup)
      {

         // If there is only one selection and it has context menu option overrides...
         if (tree.countSelections()<2)
         {
            context_target=h;

				/*
            popup->AppendMenu(MF_SEPARATOR);

            // query object for its type-specific menu options.
            for (i=0,stop=obj->NumButtons();i<stop;i++)
            {
               s=obj->GetButton(i);
               popup->AppendMenu(MF_STRING, IDC_TREEBUTTON1+i, s);
            }
				*/
         }
         else // general group operation
			{
            context_target=0;
			}

         popup->TrackPopupMenu(TPM_RIGHTALIGN|TPM_LEFTBUTTON, pt.x, pt.y, this);
      }
   }
}

// ==========================================================

// returns true if the item is a collapsable child:
// 1) collapsed
// 2) unselected
bool ObjectBrowserDialog::_r_collapseUnselected(const HTREEITEM item)
{
	if (tree.isExpanded(item) && tree.ItemHasChildren(item))
	{
		// we can only collapse if all of our children are collapsable.
		HTREEITEM iter;
		bool canCollapse=true;
		for (iter=tree.GetChildItem(item);iter;iter=tree.GetNextSiblingItem(iter))
		{
			if (!_r_collapseUnselected(iter))
			{
				canCollapse=false;
			}
		}

		if (!canCollapse)
		{
			return false;
		}

		if (!tree.Expand(item, TVE_COLLAPSE))
		{
			return false;
		}
	}

	return !tree.isSelected(item);
}

// ==========================================================

// _updateSelect ensures that a handle - object pair agree on the selection state.
void ObjectBrowserDialog::_updateSelect(HTREEITEM item, int state)
{
   UIBaseObject *obj;

   obj=_getObject(item);
   if (obj)
   {
      bool isel, osel;

      isel=0!=(state&TVIS_SELECTED); // dialog selection flag.
      osel=model.isSelected(*obj); // object selection flag.

      // try to change the object's selection to match the dialog's.
      if (isel!=osel /*&& obj!=root*/)
		{
         model.toggleSelect(*obj);
		}

      // if that failed, change the dialog's selection to match the object.
      osel=model.isSelected(*obj); // object selection flag.
      if (osel!=isel)
		{
         tree.selectItem(item, osel);
		}
   }
}

// ==========================================================

BEGIN_MESSAGE_MAP(ObjectBrowserDialog, CDialog)
	//{{AFX_MSG_MAP(ObjectBrowserDialog)
	ON_WM_DESTROY()
	ON_NOTIFY(TVN_BEGINDRAG, IDC_OBJECTSTRUCTURE, OnBegindragObjecttree)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_OBJECTSTRUCTURE, OnSelchangedObjecttree)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(TVN_SELCHANGING, IDC_OBJECTSTRUCTURE, OnSelchangingObjecttree)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_OBJECTSTRUCTURE, OnEndlabeledit)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_COMMAND(IDC_RENAMEOBJECT, OnRenameobject)
	ON_COMMAND(IDC_SHOWTOPLEVELPAGE, OnShowtoplevelpage)
	ON_COMMAND(IDC_SHOWOBJECT, OnShowobject)
	ON_COMMAND(IDC_HIDEOBJECT, OnHideobject)
	ON_COMMAND(IDC_COLLAPSEALL, OnCollapseAll)
	ON_COMMAND(IDC_CLEARCOLLAPSEALL, OnClearCollapseAll)
	ON_COMMAND(IDC_COLLAPSE, OnCollapse)
	ON_COMMAND(IDC_FULLYEXPAND, OnFullyExpand)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ObjectBrowserDialog message handlers

void ObjectBrowserDialog::OnCancel() 
{ 
	if (tree.GetEditControl())
	{
		TreeView_EndEditLabelNow(tree, TRUE);
	}
}

void ObjectBrowserDialog::OnOK()
{
	if (tree.GetEditControl())
	{
		TreeView_EndEditLabelNow(tree, FALSE);
	}
}

void ObjectBrowserDialog::OnDestroy() 
{
   _removeTree();

	CDialog::OnDestroy();
}

void ObjectBrowserDialog::PostNcDestroy() 
{ 
	model.removeMonitor(*this);
	delete this;
}

BOOL ObjectBrowserDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

   // setup icon resources
   HINSTANCE hInst=AfxGetInstanceHandle();

	iconlist.Create(MAKEINTRESOURCE(IDR_IMAGELIST), 16, 1, CLR_DEFAULT);

   // sub-class our tree control.
   tree.SubclassDlgItem(IDC_OBJECTSTRUCTURE, this);

   tree.SetImageList(&iconlist, TVSIL_NORMAL);

	tree.SetOwner(this);
	
	model.addMonitor(*this);
	
	CRect savedRect;
	if (theApp.getDialogPosition(savedRect, "ObjectBrowserDialog"))
	{
		MoveWindow(savedRect.left, savedRect.top, savedRect.Width(), savedRect.Height());
	}

	return TRUE;  // return TRUE unless you set the focus to a control
}

void ObjectBrowserDialog::OnBegindragObjecttree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

   // user is initiating a drag-and-drop operation.
   if (_isDragging())
	{
      _endDrag();
	}

   CPoint pt=pNMTreeView->ptDrag;
   ClientToScreen(&pt);
   _beginDrag(pNMTreeView->itemNew.hItem, pt);

	*pResult = 0;
}

void ObjectBrowserDialog::OnMouseMove(UINT nFlags, CPoint point) 
{
   if (_isDragging())
   {
      HTREEITEM target;
      UINT hflags;
      CPoint spt, tpt;

      // translate point to tree-control relative coordinates.
      spt=point;
      ClientToScreen(&spt);
      tpt=spt;
      tree.ScreenToClient(&tpt);

      // do an item pick.
      target=tree.HitTest(tpt, &hflags);

      // scroll the tree if the user tries dragging their item above the top item
      // or below the bottom item.
      if (hflags==TVHT_ABOVE)
      {
         target=tree.GetFirstVisibleItem();
         if (target)
         {
            target=tree.GetPrevVisibleItem(target);
            if (target)
               if (tree.SelectSetFirstVisible(target) && tree.GetFirstVisibleItem()==target)
               {
                  tpt.y+=tree.GetItemHeight();
                  spt.y+=tree.GetItemHeight();
                  SetCursorPos(spt.x, spt.y);
                  target=tree.HitTest(tpt, &hflags);
               }
               else
                  target=0;
         }
      }
      else if (hflags==TVHT_BELOW)
      {
         target=tree.GetFirstVisibleItem();
         if (target)
         {
            target=tree.GetNextVisibleItem(target);
            if (target)
               if (tree.SelectSetFirstVisible(target) && tree.GetFirstVisibleItem()==target)
               {
                  tpt.y-=tree.GetItemHeight();
                  spt.y-=tree.GetItemHeight();
                  SetCursorPos(spt.x, spt.y);
                  target=tree.HitTest(tpt, &hflags);
               }
               else
                  target=0;
         }
      }

      // if there is a new drop target, update flags and current handle.
      if (target && target!=drop_target)
      {
         if (drop_target)
            tree.SetItemState(drop_target, 0, TVIS_DROPHILITED);
         tree.SetItemState(target, TVIS_DROPHILITED, TVIS_DROPHILITED);
         drop_target=target;
      }
   }
	CDialog::OnMouseMove(nFlags, point);
}

void ObjectBrowserDialog::OnLButtonUp(UINT nFlags, CPoint point) 
{
   if (_isDragging()) 
   {
      _performDragDrop();
      _endDrag();
   }
	CDialog::OnLButtonUp(nFlags, point);
}

void ObjectBrowserDialog::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
   if (tree.m_hWnd)
	{
      tree.SetWindowPos(0, 0, 0, cx, cy, SWP_NOZORDER);
	}
}

void ObjectBrowserDialog::OnSelchangedObjecttree(NMHDR* pNMHDR, LRESULT* pResult) 
{
   if (_isAttached())
   {
	   NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

      // user has clicked to change a selection in the tree - update the editor.
      if (pNMTreeView->itemNew.hItem)
		{
         _updateSelect(pNMTreeView->itemNew.hItem, pNMTreeView->itemNew.state);
		}
      if (pNMTreeView->itemOld.hItem)
		{
         _updateSelect(pNMTreeView->itemOld.hItem, pNMTreeView->itemOld.state);
		}
   }
	*pResult = 0;
}

void ObjectBrowserDialog::OnContextMenu(CWnd* pWnd, CPoint point) 
{
   CPoint cpt;
   HTREEITEM item;
   UINT htflags=0;
   
   cpt=point;
   tree.ScreenToClient(&cpt);
   item=tree.HitTest(cpt, &htflags);
   if (item)
	{
      _contextMenu(item, point);
	}
}

void ObjectBrowserDialog::OnRenameobject()
{
   if (context_target) // single selection
	{
		tree.SetFocus();

		CEdit *editControl = tree.EditLabel(context_target);
		if (editControl)
		{
			editControl->ModifyStyle(0, ES_WANTRETURN);
		}
	}
}

void ObjectBrowserDialog::OnShowtoplevelpage()
{
   if (context_target) // single selection
	{
		UIBaseObject *o = _getObject(context_target);
		if (o)
		{
			model.setCurrentlyVisiblePage(o);
		}
	}
}

void ObjectBrowserDialog::OnShowobject()
{
	UIBaseObject::UIObjectVector objects;

   HTREEITEM iter;
   for (iter=tree.getFirstSelectedItem();iter;iter=tree.getNextSelectedItem(iter))
	{
		UIBaseObject *o = _getObject(iter);
		if (o)
		{
			if (model.isTopLevelPage(*o))
			{
				objects.clear();
				model.setCurrentlyVisiblePage(o);
				break;
			}
			else
			{
				objects.push_back(o);
			}
		}
	}

	if (!objects.empty())
	{
		UILowerString propName("Visible");
		UIString propValue(Unicode::narrowToWide("true"));
		model.setObjectProperty(objects, propName, propValue);
	}
}

void ObjectBrowserDialog::OnHideobject()
{
	UIBaseObject::UIObjectVector objects;

   HTREEITEM iter;
   for (iter=tree.getFirstSelectedItem();iter;iter=tree.getNextSelectedItem(iter))
	{
		UIBaseObject *o = _getObject(iter);
		if (o)
		{
			if (model.isTopLevelPage(*o))
			{
				objects.clear();
				model.setCurrentlyVisiblePage(0);
			}
			else
			{
				objects.push_back(o);
			}
		}
	}

	if (!objects.empty())
	{
		UILowerString propName("Visible");
		UIString propValue(Unicode::narrowToWide("false"));
		model.setObjectProperty(objects, propName, propValue);
	}
}

void ObjectBrowserDialog::OnCollapseAll()
{ 
	const HTREEITEM root = tree.GetRootItem();
	if (root && tree.ItemHasChildren(root))
	{
		lock();
		// we can only collapse if all of our children are collapsable.
		for (HTREEITEM iter=tree.GetChildItem(root);iter;iter=tree.GetNextSiblingItem(iter))
		{
			_r_collapseUnselected(iter);
		}
		unlock();
	}
}

void ObjectBrowserDialog::OnClearCollapseAll()
{
	model.clearSelections();
	OnCollapseAll();
}

void ObjectBrowserDialog::OnCollapse()
{
	lock();
   HTREEITEM iter;
   for (iter=tree.getFirstSelectedItem();iter;iter=tree.getNextSelectedItem(iter))
	{
		_r_collapseUnselected(iter);
	}
	unlock();
}

void ObjectBrowserDialog::OnFullyExpand()
{
	lock();
   HTREEITEM iter;
   for (iter=tree.getFirstSelectedItem();iter;iter=tree.getNextSelectedItem(iter))
	{
		tree.FullyExpand(iter);
	}
	unlock();
}

void ObjectBrowserDialog::OnSelchangingObjecttree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
   // The action>2 test prevents auto-selection of the topmost object in 
   // the tree dialog that occurs when the dialog is activated with no selections.
   // This auto-selection is some feature of the tree control that automatically selects
   // an item when it gains focus and it has no item already selected.  When this occurs
   // pNMTreeView->action is 4096.  I decided to reject anything that wasn't the 3 documented values.
	*pResult = supress_sel_changes || pNMTreeView->action>2;
}

void ObjectBrowserDialog::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult)
{
	TVITEM &itemDisplayInfo = ((NMTVDISPINFO*)pNMHDR)->item;
	if (itemDisplayInfo.mask & TVIF_TEXT)
	{
		HTREEITEM renamedItem = itemDisplayInfo.hItem;
		UIBaseObject *obj = _getObject(renamedItem);
		if (obj)
		{
			const char *newName = itemDisplayInfo.pszText;
			if (model.renameObject(obj, newName))
			{
			   tree.SetItemText(renamedItem, newName);
			}
		}
	}
}

BOOL ObjectBrowserDialog::PreTranslateMessage(MSG* pMsg) 
{
	CWnd *pMainWnd = GetOwner();

	if	( !(  pMainWnd
			&& accelerators
			&& ::TranslateAccelerator(pMainWnd->m_hWnd, accelerators, pMsg)
			)
		)
	{
		return CDialog::PreTranslateMessage(pMsg);
	}
	else
	{
		return TRUE;
	}
}

LRESULT ObjectBrowserDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
		case WM_NCACTIVATE:
			wParam=showActive;
			break;
	}
	return CDialog::WindowProc(message, wParam, lParam);
}

void ObjectBrowserDialog::OnClose() 
{
	CWnd *owner = GetOwner();
	if (owner)
	{
		owner->SendMessage(WM_closeObjectBrowserDialog, 0, 0);
	}
}
