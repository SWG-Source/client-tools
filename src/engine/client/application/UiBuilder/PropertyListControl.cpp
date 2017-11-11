// PropertyListControl.cpp : implementation file
//

#include "FirstUiBuilder.h"
#include "PropertyListControl.h"
#include "SelectRegionDialog.h"
#include "StringPropertyDialog.h"
#include "resource.h"

#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIPropertyDescriptor.h"
#include "UITypes.h"
#include "UIUtils.h"

/////////////////////////////////////////////////////////////////////////////

bool PropertyListControlProperty::setValue(const CString &i_newValue)
{
	m_narrowValue = i_newValue;
	refreshItemValue();
	return true;
}

UIString PropertyListControlProperty::wideValue() const
{
	return UIUnicode::narrowToWide(m_narrowValue);
}

void PropertyListControlProperty::onRelease(){}

void PropertyListControlProperty::onFollowLink() {}

void PropertyListControlProperty::refreshItemValue() const
{
	m_ownerControl.SetItem(m_listIndex, 1, LVIF_TEXT, m_narrowValue.c_str(), 0, 0, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////
// PropertyListControl

PropertyListControl::PropertyListControl()
:	m_inited(false),
	m_contextItem(-1),
	m_contextLoc(0,0),
	m_nHighlight(HIGHLIGHT_NORMAL)
{
}

// ========================================================================

PropertyListControl::~PropertyListControl()
{
}

// ========================================================================

int PropertyListControl::SetHighlightType(EHighlight hilite)
{
	int oldhilite = m_nHighlight;
	if (hilite <= HIGHLIGHT_ROW)
	{
		m_nHighlight = hilite;
		Invalidate();
	}
	return oldhilite;
}

// ========================================================================

void PropertyListControl::clear()
{
	DeleteAllItems();
}

// ========================================================================

void PropertyListControl::addProperty(const Property &i_property)
{
	if (!m_inited)
	{
		_initializeColumns();
	}

	// Create a new list-control item.
	// TODO!!! - this needs to be sorted by groupLevel / class-name.
	int item = InsertItem(GetItemCount(), i_property.m_descriptor.m_name);

	// Point Property to item.
	i_property.m_listIndex=item;

	// Point item to Property structure.
	SetItemData(item, DWORD(&i_property));

	// refresh value display.
	i_property.refreshItemValue();
}

// ========================================================================

void PropertyListControl::removeProperty(const Property &i_property)
{
	int item = i_property.m_listIndex;
	DeleteItem(item);
	_reindexProperties(item);
}

// ========================================================================

bool PropertyListControl::empty() const
{
	return size()==0;
}

// ========================================================================

int PropertyListControl::size() const
{
	return GetItemCount();
}

// ========================================================================

void PropertyListControl::_edit(int item, CPoint loc, bool forceText)
{
	int count = GetItemCount();
	if (item < 0 || item>=count)
	{
		return;
	}

	Property *p = (Property *)GetItemData(item);
	if (!p)
	{
		return;
	}

	if (p->m_descriptor.isReadOnly())
	{
		return;
	}

	UIBaseObject *const anchor = p->getAnchor();
	if (!p)
	{
		return;
	}

	UIPropertyTypes::Type propertyType = (forceText) ? UIPropertyTypes::T_string : p->m_descriptor.m_type;

	const CString itemString = GetItemText(item, 1);

	if (  propertyType != UIPropertyTypes::T_string
		&& !_stricmp(p->m_descriptor.m_name, "SourceRect")
		)
	{
		// Show graphical source rect editor
		bool UpdateProperty = false;
		
		SelectRegionDialog SelectRegion(this);
		
		if (anchor->IsA(TUIImageStyle))
		{
			propertyType = UIPropertyTypes::T_NUM_TYPES;
			UIImageStyle *imageStyle = UI_ASOBJECT(UIImageStyle, anchor);
			UpdateProperty = SelectRegion.editProperty(imageStyle, p->narrowValue());
		}
		else if (anchor->IsA(TUIImageFragment))
		{
			UIBaseObject *obj = anchor;
			
			while (obj && !obj->IsA(TUIImageStyle))
			{
				obj = obj->GetParent();
			}
			
			if (obj)
			{
				propertyType = UIPropertyTypes::T_NUM_TYPES;
				UpdateProperty = SelectRegion.editProperty(static_cast<UIImageStyle *>(obj), p->narrowValue());
			}
		}
		else if (anchor->IsA(TUIImage))
		{
			propertyType = UIPropertyTypes::T_NUM_TYPES;
			UpdateProperty = SelectRegion.editProperty(UI_ASOBJECT(UIImage, anchor), p->narrowValue());
		}
		
		if (UpdateProperty)
		{
			p->setValue(SelectRegion.regionText());
		}
	}

	switch (propertyType)
	{
	case UIPropertyTypes::T_int:
	{
		StringPropertyDialog sd(this, loc, itemString, StringPropertyDialog::FR_Integer);
		if (sd.DoModal()==IDOK)
		{
			CString value;
			sd.getStringProperty(value);
			p->setValue(value);
		}
	} break;

	case UIPropertyTypes::T_color:
	{
		UIColor col;
		UIUtils::ParseColor(p->narrowValue(), col);
		COLORREF col32 = RGB(col.r, col.g, col.b);
		CColorDialog cd(col32, CC_ANYCOLOR | CC_FULLOPEN);
		if (cd.DoModal()==IDOK)
		{
			COLORREF col32 = cd.GetColor();

			UIColor col;
			col.r = GetRValue(col32);
			col.g = GetGValue(col32);
			col.b = GetBValue(col32);
			col.a = 255;

			UINarrowString sColor;
			UIUtils::FormatColor(sColor, col);

			p->setValue(sColor.c_str());
		}
	} break;

	case UIPropertyTypes::T_string:
	case UIPropertyTypes::T_script:
	default:
	{
		StringPropertyDialog sd(this, loc, itemString, StringPropertyDialog::FR_AnyText);
		if (sd.DoModal()==IDOK)
		{
			CString value;
			sd.getStringProperty(value);
			p->setValue(value);
		}
	} break;

	case UIPropertyTypes::T_NUM_TYPES: break;
	}
}

// ========================================================================

bool PropertyListControl::_contextMenu(int item, CPoint loc)
{
	int count = GetItemCount();
	if (item < 0 || item>=count)
	{
		return false;
	}

	Property *p = (Property *)GetItemData(item);
	if (!p)
	{
		return false;
	}

	m_contextItem = item;
	m_contextLoc = loc;

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING | MF_ENABLED, ID_EDITPROPERTY, "Edit Property Text");

	const UIPropertyDescriptor &desc = p->m_descriptor;
	switch (desc.m_type)
	{
	case UIPropertyTypes::T_object:
	{
		menu.AppendMenu(MF_STRING | MF_ENABLED, ID_FOLLOWLINK, "Follow Link");
	}
	break;
	default:;
	}

	const bool result=menu.TrackPopupMenu(TPM_RIGHTALIGN|TPM_LEFTBUTTON, loc.x, loc.y, this)!=0;
	return result;
}

// ========================================================================

void PropertyListControl::_repaintSelectedItems()
{
    CRect rcBounds, rcLabel;
    
    // Invalidate focused item so it can repaint 
    int nItem = GetNextItem(-1, LVNI_FOCUSED);
    
    if(nItem != -1)
    {
        GetItemRect(nItem, rcBounds, LVIR_BOUNDS);
        GetItemRect(nItem, rcLabel, LVIR_LABEL);
        rcBounds.left = rcLabel.left;
        
        InvalidateRect(rcBounds, FALSE);
    }
    
    // Invalidate selected items depending on LVS_SHOWSELALWAYS
    if(!(GetStyle() & LVS_SHOWSELALWAYS))
    {
        for(nItem = GetNextItem(-1, LVNI_SELECTED);
        nItem != -1; nItem = GetNextItem(nItem, LVNI_SELECTED))
        {
            GetItemRect(nItem, rcBounds, LVIR_BOUNDS);
            GetItemRect(nItem, rcLabel, LVIR_LABEL);
            rcBounds.left = rcLabel.left;
            
            InvalidateRect(rcBounds, FALSE);
        }
    }
    
    UpdateWindow();
}

// ========================================================================

void PropertyListControl::_onDeleteItem(int item)
{
	Property *p = (Property *)GetItemData(item);
	if (p)
	{
		p->m_listIndex=-1;
		SetItemData(item, 0);
		p->onRelease();
	}
}

// ========================================================================

void PropertyListControl::_reindexProperties(int item)
{
	int count = GetItemCount();
	for (int i=item;i<count;i++)
	{
		Property *p = (Property *)GetItemData(item);
		if (p)
		{
			p->m_listIndex=i;
		}
	}
}

// ========================================================================

void PropertyListControl::_initializeColumns()
{
	CRect rect;
	GetClientRect(&rect);

	// TODO: Add your specialized code here and/or call the base class
	int width0 = rect.Width() / 2;
	int width1 = rect.Width() - width0;

	int column=-1;

	column = InsertColumn(column+1, "Name");
	SetColumnWidth(column, width0);

	column = InsertColumn(column + 1, "Value");
	SetColumnWidth(column, width1);

	m_inited=true;
}

// ========================================================================

int PropertyListControl::_itemHitTest(LVHITTESTINFO &results, CPoint *o_mouseScreen) // returns index of item hit or -1 if not item was hit.
{
	memset(&results, 0, sizeof(results));

	CPoint mouseScreen;
	if (GetCursorPos(&mouseScreen))
	{
		if (o_mouseScreen)
		{
			*o_mouseScreen=mouseScreen;
		}

		CRect windowRect;
		GetWindowRect(&windowRect);

		CPoint mouseLocal = mouseScreen - windowRect.TopLeft();

		results.pt = mouseLocal;
		return SubItemHitTest(&results);
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(PropertyListControl, CListCtrl)
	//{{AFX_MSG_MAP(PropertyListControl)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_NOTIFY_REFLECT(LVN_DELETEALLITEMS, OnDeleteallitems)
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnDeleteitem)
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRclick)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FOLLOWLINK, OnFollowLink)
	ON_COMMAND(ID_EDITPROPERTY, OnEditProperty)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PropertyListControl message handlers

void PropertyListControl::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LVHITTESTINFO hitTest;
	CPoint mouseScreen;
	int item = _itemHitTest(hitTest, &mouseScreen);
	if (  item>=0
		&& (hitTest.flags & LVHT_ONITEMLABEL)!=0
		)
	{
		_edit(item, mouseScreen, false);
	}
	
	*pResult = 0;
}

void PropertyListControl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC *const  pDC    = CDC::FromHandle(lpDrawItemStruct->hDC);
	const int   nItem  = lpDrawItemStruct->itemID;
	const CRect rcItem(lpDrawItemStruct->rcItem);

	bool error=false;
	bool readOnly=false;
	bool isSubProperty=false;
	Property *p = (Property *)GetItemData(nItem);
	if (p)
	{
		error=p->m_descriptor.isError();
		readOnly=p->m_descriptor.isReadOnly();
		isSubProperty=p->m_descriptor.isSubProperty();
	}
	
	// Save dc state
	const int nSavedDC = pDC->SaveDC();
	
	// Get item image and state info
	LV_ITEM lvi;
	lvi.mask = LVIF_IMAGE | LVIF_STATE;
	lvi.iItem = nItem;
	lvi.iSubItem = 0;
	lvi.stateMask = 0xFFFF;     // get all state flags
	GetItem(&lvi);
	
	// Should the item be highlighted
	const BOOL bHighlight =
		(
			(lvi.state & LVIS_DROPHILITED)!=0
		||	(
				(lvi.state & LVIS_SELECTED)!=0
			&&	(
					(GetFocus() == this)
				||	(GetStyle()&LVS_SHOWSELALWAYS)!=0
				)
			)
		);
	
	// --------------------------------------------------------------
	// Get rectangles for drawing
	CRect rcBounds, rcLabel, rcIcon;
	GetItemRect(nItem, rcBounds, LVIR_BOUNDS);
	GetItemRect(nItem, rcLabel, LVIR_LABEL);
	GetItemRect(nItem, rcIcon, LVIR_ICON);
	CRect rcCol(rcBounds); 
	
	CString sLabel = GetItemText(nItem, 0);
	// --------------------------------------------------------------
	
	// --------------------------------------------------------------
	// Labels are offset by a certain amount  
	// This offset is related to the width of a space character
	CRect rcHighlight;
	const int spaceWidth = pDC->GetTextExtent(_T(" "), 1 ).cx;
	const int indent = (isSubProperty) ? spaceWidth * 8 : 0;
	const int offset = spaceWidth*2 + indent;
	{
		switch (m_nHighlight)
		{
		case 0: 
		{
			int nExt = pDC->GetOutputTextExtent(sLabel).cx + offset;
			rcHighlight = rcLabel;
			if (rcLabel.left + nExt < rcLabel.right)
			{
				rcHighlight.right = rcLabel.left + nExt;
			}
		} break;
		case 1:
		{
			rcHighlight = rcBounds;
			rcHighlight.left = rcLabel.left;
		} break;
		case 2:
		{
			CRect rcWnd;
			GetClientRect(&rcWnd);
			rcHighlight = rcBounds;
			rcHighlight.left = rcLabel.left;
			rcHighlight.right = rcWnd.right + 1;   // Add 1 to prevent trails
		} break;

		default:
			rcHighlight = rcLabel;
		}
	}
	// --------------------------------------------------------------
	
	// --------------------------------------------------------------
	// Draw the background color
	if (bHighlight)
	{
		pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
		pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
		
		DWORD highLight = ::GetSysColor(COLOR_HIGHLIGHT);
		BYTE r, g, b;
		r = GetRValue(highLight);
		g = GetGValue(highLight);
		b = GetBValue(highLight);
		/*
		if (readOnly)
		{
			r = BYTE((unsigned(r)*3) / 4);
			g = BYTE((unsigned(g)*3) / 4);
			b = BYTE((unsigned(b)*3) / 4);
		}
		*/
		pDC->FillRect(rcHighlight, &CBrush(RGB(r,g,b)));
	}
	else
	{
		DWORD highLight = ::GetSysColor(COLOR_WINDOW);
		BYTE r, g, b;
		r = GetRValue(highLight);
		g = GetGValue(highLight);
		b = GetBValue(highLight);
		/*
		if (readOnly)
		{
			r = BYTE((unsigned(r)*3) / 4);
			g = BYTE((unsigned(g)*3) / 4);
			b = BYTE((unsigned(b)*3) / 4);
		}
		*/
		pDC->FillRect(rcHighlight, &CBrush(RGB(r,g,b)));

		if (error)
		{
			pDC->SetTextColor(RGB(255,0,0));
		}
	}
	// --------------------------------------------------------------
	
	// --------------------------------------------------------------
	// Set clip region
	CRgn rgn;
	{
		rcCol.right = rcCol.left + GetColumnWidth(0);
		rgn.CreateRectRgnIndirect(&rcCol);
		pDC->SelectClipRgn(&rgn);
		rgn.DeleteObject();
	}
	// --------------------------------------------------------------
	
	// --------------------------------------------------------------
	// Draw state icon
	if (lvi.state & LVIS_STATEIMAGEMASK)
	{
		int nImage = ((lvi.state & LVIS_STATEIMAGEMASK)>>12) - 1;
		CImageList *pImageList = GetImageList(LVSIL_STATE);
		if (pImageList)
		{
			pImageList->Draw(
				pDC, 
				nImage,
				CPoint(rcCol.left, rcCol.top), ILD_TRANSPARENT
			);
		}
	}
	// --------------------------------------------------------------
	
	// --------------------------------------------------------------
	// Draw normal and overlay icon
	{
		CImageList *pImageList = GetImageList(LVSIL_SMALL);
		if (pImageList)
		{
			UINT nOvlImageMask=lvi.state & LVIS_OVERLAYMASK;
			pImageList->Draw(
				pDC, 
				lvi.iImage, 
				CPoint(rcIcon.left, rcIcon.top),
				(bHighlight ? ILD_BLEND50 : 0) | ILD_TRANSPARENT | nOvlImageMask 
			);
		}
	}
	// --------------------------------------------------------------
	
	// --------------------------------------------------------------
	// Draw item label - Column 0
	{
		rcLabel.left += offset/2;
		rcLabel.right -= offset;
		
		pDC->DrawText(
			sLabel,
			-1,
			rcLabel,
			DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP |DT_VCENTER | DT_END_ELLIPSIS
		);
	}
	// --------------------------------------------------------------
	
	// --------------------------------------------------------------
	// Draw labels for remaining columns
	{
		LV_COLUMN lvc;
		lvc.mask = LVCF_FMT | LVCF_WIDTH;

		rcBounds.right = rcHighlight.right > rcBounds.right ? rcHighlight.right :
		rcBounds.right;
		rgn.CreateRectRgnIndirect(&rcBounds);
		pDC->SelectClipRgn(&rgn);
		
		for (int nColumn = 1; GetColumn(nColumn, &lvc); nColumn++)
		{
			rcCol.left = rcCol.right;
			rcCol.right += lvc.cx;

			sLabel = GetItemText(nItem, nColumn);

			const bool colorField = nColumn==1 && p->m_descriptor.m_type==UIPropertyTypes::T_color;

			// -------------------------------------------------------------------
			DWORD fillColor=::GetSysColor(COLOR_WINDOW);

			if (colorField)
			{
				UIColor col;
				UIUtils::ParseColor(p->narrowValue(), col);

				// -----------------------------------------------

				CSize textSize = pDC->GetOutputTextExtent(sLabel);
				textSize.cx += offset + 2;
				if (textSize.cx>rcCol.Width())
				{
					textSize.cx=rcCol.Width();
				}
				if (textSize.cy>rcCol.Height())
				{
					textSize.cy=rcCol.Height();
				}

				CRect textRect;
				textRect.left   = rcCol.left;
				textRect.top    = rcCol.top;
				textRect.right  = textRect.left + textSize.cx;
				textRect.bottom = textRect.top + textSize.cy;
				pDC->FillRect(textRect, &CBrush(fillColor));

				// -----------------------------------------------

				CRect colorRect = rcCol;
				colorRect.left += textSize.cx;

				if (colorRect.Width()>0)
				{
					pDC->FillRect(colorRect, &CBrush(RGB(col.r, col.g, col.b)));
				}
			}
			else if (m_nHighlight == HIGHLIGHT_NORMAL) // Draw the background if needed
			{
				pDC->FillRect(rcCol, &CBrush(fillColor));
			}
			// -------------------------------------------------------------------

			if (sLabel.GetLength() == 0)
			{
				continue;
			}

			// -------------------------------------------------------------------
			if (m_nHighlight == 0)     // Highlight only first column
			{
				if (readOnly)
				{
					pDC->SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
				}
				else
				{
					pDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
				}
				pDC->SetBkColor(::GetSysColor(COLOR_WINDOW));
			}
			else
			{
				if (readOnly)
				{
					pDC->SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
				}
			}

			// -------------------------------------------------------------------
		
			
			// Get the text justification
			UINT nJustify = DT_LEFT;
			switch(lvc.fmt & LVCFMT_JUSTIFYMASK)
			{
			case LVCFMT_RIGHT:
				nJustify = DT_RIGHT;
				break;
			case LVCFMT_CENTER:
				nJustify = DT_CENTER;
				break;
			default:
				break;
			}
			
			rcLabel = rcCol;
			rcLabel.left += offset;
			rcLabel.right -= offset;
			
			pDC->DrawText(
				sLabel, 
				-1, 
				rcLabel, 
				nJustify | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS
			);
		}
	}
	// --------------------------------------------------------------
	
	// --------------------------------------------------------------
	// Draw focus rectangle if item has focus
	if (lvi.state & LVIS_FOCUSED && (GetFocus() == this))
	{
		pDC->DrawFocusRect(rcHighlight);
	}
	// --------------------------------------------------------------
	
	// --------------------------------------------------------------
	// Restore dc
	pDC->RestoreDC(nSavedDC);
}

BOOL PropertyListControl::PreCreateWindow(CREATESTRUCT& cs) 
{
	// default is report view and full row selection
	cs.style &= ~LVS_TYPEMASK;
	cs.style &= ~LVS_SHOWSELALWAYS;
	cs.style |= LVS_REPORT | LVS_OWNERDRAWFIXED;
	cs.style |= LVS_NOSORTHEADER;
	cs.style |= LVS_NOCOLUMNHEADER;
	cs.style |= LVS_NOSCROLL;
	return CListCtrl::PreCreateWindow(cs);
}

void PropertyListControl::OnPaint() 
{
	// in full row select mode, we need to extend the clipping region
	// so we can paint a selection all the way to the right
	if (m_nHighlight == HIGHLIGHT_ROW
		&& (GetStyle() & LVS_TYPEMASK) == LVS_REPORT 
		)
	{
		CRect rcBounds;
		GetItemRect(0, rcBounds, LVIR_BOUNDS);
		
		CRect rcClient;
		GetClientRect(&rcClient);
		if(rcBounds.right < rcClient.right)
		{
			CPaintDC dc(this);
			
			CRect rcClip;
			dc.GetClipBox(rcClip);
			
			rcClip.left = min(rcBounds.right-1, rcClip.left);
			rcClip.right = rcClient.right;
			
			InvalidateRect(rcClip, FALSE);
		}
	}

	CListCtrl::OnPaint();
}

void PropertyListControl::OnSetFocus(CWnd* pOldWnd) 
{
	CListCtrl::OnSetFocus(pOldWnd);
	
	// check if we are getting focus from label edit box
	if(pOldWnd!=NULL && pOldWnd->GetParent()==this)
		return;
	
	// repaint items that should change appearance
	if((GetStyle() & LVS_TYPEMASK)==LVS_REPORT)
		_repaintSelectedItems();
}

void PropertyListControl::OnKillFocus(CWnd* pNewWnd) 
{
	CListCtrl::OnKillFocus(pNewWnd);
	
	// check if we are losing focus to label edit box
	if (pNewWnd != NULL && pNewWnd->GetParent() == this)
	{
		return;
	}
	
	// repaint items that should change appearance
	if ((GetStyle() & LVS_TYPEMASK) == LVS_REPORT)
	{
		_repaintSelectedItems();
	}
}

void PropertyListControl::OnDeleteallitems(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	int count = GetItemCount();
	for (int i=0;i<count;i++)
	{
		_onDeleteItem(i);
	}
	
	*pResult = TRUE; // prevent LVN_DELETITEM notifications
}

void PropertyListControl::OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here

	_onDeleteItem(pNMListView->iItem);
	
	*pResult = 0;
}

void PropertyListControl::OnRclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LVHITTESTINFO hitTest;
	CPoint mouseScreen;
	int item = _itemHitTest(hitTest, &mouseScreen);
	if (  item>=0
		&& (hitTest.flags & LVHT_ONITEMLABEL)!=0
		)
	{
		_contextMenu(item, mouseScreen);
	}
	
	*pResult = 0;
}

void PropertyListControl::OnFollowLink()
{
	int count = GetItemCount();
	if (m_contextItem < 0 || m_contextItem>=count)
	{
		return;
	}

	Property *p = (Property *)GetItemData(m_contextItem);
	if (!p)
	{
		return;
	}

	const UIPropertyDescriptor &desc = p->m_descriptor;
	if (desc.m_type!=UIPropertyTypes::T_object)
	{
		return;
	}

	p->onFollowLink();
}

void PropertyListControl::OnEditProperty()
{
	_edit(m_contextItem, m_contextLoc, true);
}
