// CTreeCtrlEx.cpp : implementation file
//

#include "FirstUiBuilder.h"
#include "CTreeCtrlEx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTreeCtrlEx

CTreeCtrlEx::CTreeCtrlEx()
{
   clicked_item=0;
   initial_sel=0;
   click_handled=false;
   supress_sel_notifications=false;
}

CTreeCtrlEx::~CTreeCtrlEx()
{
}

int CTreeCtrlEx::countSelections()
{
   int ret_val=0;
   HTREEITEM iter;
   for (iter=getFirstSelectedItem();iter;iter=getNextSelectedItem(iter))
      ret_val++;
   return ret_val;
}

void CTreeCtrlEx::clearSelections(bool multi_only)
{
   HTREEITEM item;

   if (!multi_only)
      SelectItem(0);

   for (item=GetRootItem();item!=0;item=GetNextVisibleItem(item))
      if (isSelected(item))
      {
         SetItemState(item, 0, TVIS_SELECTED);
         _notifySelect(item, 0);
      }

   initial_sel=0;
}

void CTreeCtrlEx::selectItem(HTREEITEM item, bool select)
{
   bool is_sel=isSelected(item);
   if (is_sel!=select)
      _selectMultiple(item, MK_CONTROL);
   assert(isSelected(item)==select);
}

HTREEITEM CTreeCtrlEx::getFirstSelectedItem()
{
   HTREEITEM iter;

   for (iter=GetRootItem();iter;iter=GetNextVisibleItem(iter))
      if (isSelected(iter))
         return iter;

   return 0;
}

HTREEITEM CTreeCtrlEx::getNextSelectedItem(HTREEITEM x)
{
   HTREEITEM iter;
   for (iter=GetNextVisibleItem(x);iter;iter=GetNextVisibleItem(iter))
      if (isSelected(iter))
         return iter;

   return 0;
}

HTREEITEM CTreeCtrlEx::getLastSelectedItem()
{
   HTREEITEM ret_val, iter;

   ret_val=0;
   for (iter=GetRootItem();iter;iter=GetNextVisibleItem(iter))
      if (isSelected(iter))
         ret_val=iter;
   return ret_val;
}

HTREEITEM CTreeCtrlEx::getPrevSelectedItem(HTREEITEM x)
{
   HTREEITEM iter;
   for (iter=GetPrevVisibleItem(x);iter;iter=GetPrevVisibleItem(iter))
      if (isSelected(iter))
         return iter;

   return 0;
}

void CTreeCtrlEx::FullyExpand(HTREEITEM x)
{
	Expand(x, TVE_EXPAND);
	if (ItemHasChildren(x))
	{
		for (HTREEITEM iter=GetChildItem(x);iter;iter=GetNextSiblingItem(iter))
		{
			FullyExpand(iter);
		}
	}
}

void CTreeCtrlEx::_selectMultiple(HTREEITEM item, UINT flags)
{
   HTREEITEM old_item;

   old_item = GetSelectedItem();

   // Action depends on whether the user holds down the Shift or Ctrl key
   if (flags&MK_SHIFT)
   {
      // Select from first selected item to the clicked item
      if (!initial_sel)
         initial_sel=old_item;
      _selectItemRange(initial_sel, item);
   }
   else if (flags&MK_CONTROL) // toggle select
   {
      bool new_sel, old_sel;
      
      if (old_item==item) 
         old_item=0;
      new_sel=isSelected(item);
      old_sel=(old_item) ? isSelected(old_item) : false;
      if (new_sel) // de-select
      {
         SetItemState(item, 0, TVIS_SELECTED);
         if (old_sel)
         {
            if (initial_sel==item)
               initial_sel=old_item;
         }
         else
         {
            initial_sel=getFirstSelectedItem();
            SelectItem(initial_sel);
         }
      }
      else
      {
         supress_sel_notifications=true;
         SelectItem(item);
         supress_sel_notifications=false;
         if (old_sel)
            SetItemState(old_item, TVIS_SELECTED, TVIS_SELECTED);

         if (!initial_sel)
            initial_sel=item;
      }
      _notifySelect(item, old_item);
   }
   else
   {
      clearSelections();
      SelectItem(item);
      initial_sel=item;
   }
}

bool CTreeCtrlEx::_selectItemRange(HTREEITEM start, HTREEITEM end)
{
   HTREEITEM iter = GetRootItem();
   
   while (iter && iter!=start && iter!=end)
      iter=GetNextVisibleItem(iter);
   
   if (!iter)
      return false;

   bool reverse=iter==end;
   
   SelectItem(end);
   
   iter = GetRootItem();
   bool sel = false;
   while (iter)
   {
      if (iter==(reverse?end:start))
         sel=true;
      
      if (sel)
      {
         if (!isSelected(iter))
         {
            SetItemState(iter, TVIS_SELECTED, TVIS_SELECTED);
            _notifySelect(iter, 0);
         }
      }
      else
      {
         if (isSelected(iter))
         {
            SetItemState(iter, 0, TVIS_SELECTED);
            _notifySelect(iter, 0);
         }
      }
      
      if (iter==(reverse?start:end))
         sel=false;

      iter=GetNextVisibleItem(iter);
   }
   
   return TRUE;
}

void CTreeCtrlEx::_notifySelect(HTREEITEM item, HTREEITEM old_item)
{
   NM_TREEVIEW tv;

   tv.itemOld.hItem = old_item;
   if (old_item)
   {
      tv.itemOld.state=GetItemState(old_item, 0xffffffff);
      tv.itemOld.lParam=GetItemData(old_item);
   }
   else
   {
      tv.itemOld.state=0;
      tv.itemOld.lParam=0;
   }
   tv.itemOld.mask = TVIF_HANDLE|TVIF_STATE|TVIF_PARAM;

   CWnd* pWnd = GetParent();
   if (pWnd)
   {
      tv.hdr.hwndFrom = GetSafeHwnd();
      tv.hdr.idFrom = GetWindowLong(GetSafeHwnd(), GWL_ID);
      tv.hdr.code = TVN_SELCHANGED;
      tv.itemNew.hItem = item;
      tv.itemNew.state = GetItemState(item, 0xffffffff);
      tv.itemNew.lParam = GetItemData(item);
      tv.itemNew.mask = TVIF_HANDLE|TVIF_STATE|TVIF_PARAM;
      tv.action = TVC_UNKNOWN;
      pWnd->SendMessage(WM_NOTIFY, tv.hdr.idFrom, (LPARAM)&tv);
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CTreeCtrlEx, CTreeCtrl)
	//{{AFX_MSG_MAP(CTreeCtrlEx)
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_NOTIFY_REFLECT_EX(NM_SETFOCUS, OnSetfocus)
	ON_NOTIFY_REFLECT_EX(NM_KILLFOCUS, OnKillfocus)
	ON_NOTIFY_REFLECT_EX(TVN_ITEMEXPANDING, OnItemexpanding)
	ON_WM_CHAR()
	ON_NOTIFY_REFLECT_EX(TVN_SELCHANGING, OnSelchanging)
	ON_NOTIFY_REFLECT_EX(TVN_SELCHANGED, OnSelchanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTreeCtrlEx message handlers

void CTreeCtrlEx::OnLButtonDown(UINT nFlags, CPoint point) 
{
   UINT hit_flags;
   HTREEITEM hit_item;

   hit_flags=0;
   hit_item = HitTest(point, &hit_flags);
   if (hit_flags&TVHT_ONITEM)
   {
      SetFocus();

      if (!isSelected(hit_item))
      {
         _selectMultiple(hit_item, nFlags);
         if (isSelected(hit_item))
         {
            clicked_item=hit_item;
            click_pt=point;
            click_handled=true;
         }
      }
      else
      {
         clicked_item=hit_item;
         click_pt=point;
         click_handled=false;
      }
   }
   else
      CTreeCtrl::OnLButtonDown( nFlags, point );
}

void CTreeCtrlEx::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
   if (nChar==VK_UP || nChar==VK_DOWN)
   {
      if (!(GetKeyState(VK_SHIFT)&0x8000))
      {
         // Arrow key w/o shift - cancel multi-select.
         clearSelections(TRUE);
         CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
         return;
      }
      
      HTREEITEM sel_item, next_item;

      sel_item = GetSelectedItem();
      next_item=(nChar==VK_UP) ? GetPrevVisibleItem(sel_item) : GetNextVisibleItem(sel_item);
      if (next_item)
      {
         bool select;
         select=!(GetItemState(next_item, TVIS_SELECTED)&TVIS_SELECTED);
         SelectItem(next_item);
         if (select)
            SetItemState(sel_item, TVIS_SELECTED, TVIS_SELECTED);
      }
      
      // Provide TVN_KEYDOWN notification
      CWnd *pWnd = GetParent();
      if (pWnd)
      {
         NMTVKEYDOWN tvk;
         tvk.hdr.hwndFrom = GetSafeHwnd();
         tvk.hdr.idFrom = GetWindowLong( GetSafeHwnd(), GWL_ID );
         tvk.hdr.code = TVN_KEYDOWN;
         tvk.wVKey = (WORD)nChar;
         tvk.flags = 0;
         pWnd->SendMessage(WM_NOTIFY, tvk.hdr.idFrom, (LPARAM)&tvk);
      }
   }
   else
      CTreeCtrl::OnKeyDown( nChar, nRepCnt, nFlags );
}

void CTreeCtrlEx::OnMouseMove(UINT nFlags, CPoint point) 
{
   if (clicked_item)
   {
      CSize sizeMoved = click_pt-point;
      if (abs(sizeMoved.cx) > GetSystemMetrics(SM_CXDRAG) || abs(sizeMoved.cy) > GetSystemMetrics(SM_CYDRAG))
      {
         CWnd* pWnd = GetParent();
         if (pWnd)
         {
            NM_TREEVIEW tv;
            tv.hdr.hwndFrom = GetSafeHwnd();
            tv.hdr.idFrom = GetWindowLong( GetSafeHwnd(), GWL_ID );
            tv.hdr.code = TVN_BEGINDRAG;
            tv.itemNew.hItem = clicked_item;
            tv.itemNew.state = GetItemState(clicked_item, 0xffffffff);
            tv.itemNew.lParam = GetItemData(clicked_item);
            tv.ptDrag.x = point.x;
            tv.ptDrag.y = point.y;
            pWnd->SendMessage(WM_NOTIFY, tv.hdr.idFrom, (LPARAM)&tv);
         }
         clicked_item=0;
      }
   }
   CTreeCtrl::OnMouseMove( nFlags, point );
}

void CTreeCtrlEx::OnLButtonUp(UINT nFlags, CPoint point) 
{
   if (clicked_item)
   {
      if (!click_handled)
         _selectMultiple(clicked_item, nFlags);
      clicked_item=0;
   }
   CTreeCtrl::OnLButtonUp( nFlags, point );
}

BOOL CTreeCtrlEx::OnSetfocus(NMHDR* pNMHDR, LRESULT* pResult) 
{
   Invalidate();
   *pResult = 0;
   return FALSE;
}

BOOL CTreeCtrlEx::OnKillfocus(NMHDR* pNMHDR, LRESULT* pResult) 
{
   Invalidate();
   *pResult = 0;
   return FALSE;
}

BOOL CTreeCtrlEx::OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult) 
{
   NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
   if (pNMTreeView->action==TVE_COLLAPSE)
   {
      HTREEITEM hItem = GetChildItem(pNMTreeView->itemNew.hItem);
      while (hItem)
      {
         if (GetItemState( hItem, TVIS_SELECTED)&TVIS_SELECTED)
         {
            SetItemState(hItem, 0, TVIS_SELECTED);
            _notifySelect(hItem, 0);
         }
         
         HTREEITEM next_item = GetChildItem( hItem );
         if (!next_item)
         {
            if (!(next_item=GetNextSiblingItem(hItem)))
            {
               HTREEITEM parent_item = hItem;
               while (!next_item)
               {
                  if (!(parent_item=GetParentItem(parent_item)))
                     break;
                  
                  if (parent_item==pNMTreeView->itemNew.hItem)
                     break;
                  
                  next_item = GetNextSiblingItem( parent_item );
               }
               if ( parent_item == pNMTreeView->itemNew.hItem )
                  break;
            }
         }
         hItem=next_item;
      }
   }
   *pResult = 0;
   return TRUE;
}

void CTreeCtrlEx::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
   // Eat WM_CHAR message.
   // Here to prevent item lookup when a alpha key is pressed.
}

BOOL CTreeCtrlEx::OnSelchanging(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	*pResult = 0;
   return supress_sel_notifications;
}

BOOL CTreeCtrlEx::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	*pResult = 0;
   return supress_sel_notifications;
}
