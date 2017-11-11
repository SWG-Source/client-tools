// NewHeaderCtrl.cpp : implementation file
//

#include "FirstTerrainEditor.h"
#include "TreeListHeaderCtrl.h"
#include "TreeListFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewHeaderCtrl

CNewHeaderCtrl::CNewHeaderCtrl()
{
	m_pImageList = NULL;
	m_RTL = FALSE;
}

CNewHeaderCtrl::~CNewHeaderCtrl()
{
}


BEGIN_MESSAGE_MAP(CNewHeaderCtrl, CHeaderCtrl)
	//{{AFX_MSG_MAP(CNewHeaderCtrl)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewHeaderCtrl message handlers

void CNewHeaderCtrl::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	CDC dc;

	dc.Attach( lpDrawItemStruct->hDC );

	// Save DC
	int nSavedDC = dc.SaveDC();

/*	if(m_RTL)
	{
		dc.SetViewportOrg(0, 0);
		dc.SetMapMode(MM_TEXT);
	}
*/
	// Get the column rect
	CRect rcLabel( lpDrawItemStruct->rcItem );

/*	CTLFrame *pFrame = (CTLFrame *)GetParent();

	CRect m_clientRect;
	pFrame->m_tree.GetClientRect(&m_clientRect);

	if(m_RTL)
	{
		int left = rcLabel.left;
		int right = rcLabel.right;

		rcLabel.left = m_clientRect.Width() - right;
		rcLabel.right = m_clientRect.Width() - left;
	}
*/
	// Set clipping region to limit drawing within column
	CRgn rgn;
	rgn.CreateRectRgnIndirect( &rcLabel );
	dc.SelectObject( &rgn );
	rgn.DeleteObject();

	// Labels are offset by a certain amount  
	// This offset is related to the width of a space character
	int offset = dc.GetTextExtent(_T(" "), 1 ).cx*2;


	// Draw image from image list

	// Get the column text and format
	TCHAR buf[256];
	HD_ITEM hditem;
	
	hditem.mask = HDI_TEXT | HDI_FORMAT;
	hditem.pszText = buf;
	hditem.cchTextMax = 255;

	GetItem( lpDrawItemStruct->itemID, &hditem );

	// Determine format for drawing column label
	UINT uFormat = DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP 
						| DT_VCENTER | DT_END_ELLIPSIS ;

	if( hditem.fmt & HDF_CENTER)
		uFormat |= DT_CENTER;
	else if( hditem.fmt & HDF_RIGHT)
		uFormat |= DT_RIGHT;
	else
		uFormat |= DT_LEFT;

	if(!(uFormat & DT_RIGHT))
	{
		// Adjust the rect if the mouse button is pressed on it
		if( lpDrawItemStruct->itemState == ODS_SELECTED )
		{
			rcLabel.left++;
			rcLabel.top += 2;
			rcLabel.right++;
		}

		rcLabel.left += offset;
		rcLabel.right -= offset;

		// Draw column label
		if( rcLabel.left < rcLabel.right )
			dc.DrawText(buf,-1,rcLabel, uFormat);
	}


	int imageIndex = 0;
	if (m_pImageList && GetItemImage(lpDrawItemStruct->itemID))
	{
		if( imageIndex != -1 )
		{
			if(uFormat & DT_RIGHT)
			// draw to the left of the label
				m_pImageList->Draw(&dc, imageIndex, 
						CPoint( rcLabel.left + offset,offset/3 ),
						ILD_TRANSPARENT );
			else
			// draw to the right
				m_pImageList->Draw(&dc, imageIndex, 
						CPoint( rcLabel.right - dc.GetTextExtent(_T(buf), 1 ).cx*2,offset/3 ),
						ILD_TRANSPARENT );


			// Now adjust the label rectangle
			IMAGEINFO imageinfo;
			if( m_pImageList->GetImageInfo( imageIndex, &imageinfo ) )
			{
				rcLabel.left += offset/2 + 
					imageinfo.rcImage.right - imageinfo.rcImage.left;
			}
		}
	}

	if(uFormat & DT_RIGHT)
	{
		// Adjust the rect if the mouse button is pressed on it
		if( lpDrawItemStruct->itemState == ODS_SELECTED )
		{
			rcLabel.left++;
			rcLabel.top += 2;
			rcLabel.right++;
		}

		rcLabel.left += offset;
		rcLabel.right -= offset;

		// Draw column label
		if( rcLabel.left < rcLabel.right )
			dc.DrawText(buf,-1,rcLabel, uFormat);
	}

/*	if(m_RTL)
	{
		dc.SetViewportOrg(m_clientRect.Width(), 0);
		CSize ext = dc.GetViewportExt();
		ext.cx = ext.cx > 0 ? -ext.cx : ext.cx;

		dc.SetMapMode(MM_ANISOTROPIC);
		dc.SetViewportExt(ext);
	}
*/
	// Restore dc
	dc.RestoreDC( nSavedDC );

	// Detach the dc before returning
	dc.Detach();
}

CImageList* CNewHeaderCtrl::SetImageList( CImageList* pImageList )
{
	CImageList *pPrevList = m_pImageList;
	m_pImageList = pImageList;
	return pPrevList;
}

int CNewHeaderCtrl::GetItemImage( int nItem )
{
	std::map<int, int>::iterator iter = m_mapImageIndex.find(nItem);
	return (iter != m_mapImageIndex.end()) ? iter->second : -1;
}

void CNewHeaderCtrl::SetItemImage( int nItem, int nImage )
{
	// Save the image index
	m_mapImageIndex[nItem] = nImage;

	// Change the item to ownder drawn
	HD_ITEM hditem;

	hditem.mask = HDI_FORMAT;
	GetItem( nItem, &hditem );
	hditem.fmt |= HDF_OWNERDRAW;
	SetItem( nItem, &hditem );

	// Invalidate header control so that it gets redrawn
	Invalidate();
}

void CNewHeaderCtrl::Autofit(int nOverrideItemData /*= -1*/, int nOverrideWidth /*= 0*/)
{
	int nItemCount = GetItemCount();
	int nTotalWidthOfColumns = 0;
	int nDifferenceInWidht;
	int nItem;
	HD_ITEM hi;
	CRect rClient;

	if (!m_bAutofit)
		return;

	SetRedraw(FALSE);

	GetParent()->GetClientRect(&rClient);
	if (-1 != nOverrideItemData)
		rClient.right -= nOverrideWidth;

	// Get total width of all columns
	for (nItem = 0; nItem < nItemCount; nItem++)
	{
		if (nItem == nOverrideItemData)	// Don't mess with the item being resized by the user
			continue;

		hi.mask = HDI_WIDTH;
		GetItem(nItem, &hi);

		nTotalWidthOfColumns += hi.cxy;
	}

	if (nTotalWidthOfColumns != rClient.Width())
	{
		nDifferenceInWidht = abs(nTotalWidthOfColumns-rClient.Width());	// We need to shrink/expand all columns!
		
		// Shrink/expand all columns proportionally based on their current size
		for (nItem = 0; nItem < nItemCount; nItem++)
		{
			if (nItem == nOverrideItemData)	// Skip the overrride column if there is one!
				continue;
			
			hi.mask = HDI_WIDTH;
			GetItem(nItem, &hi);

			hi.mask = HDI_WIDTH;
			hi.cxy = (hi.cxy * rClient.Width()) / nTotalWidthOfColumns;

			SetItem(nItem, &hi);
		}
	}

	SetRedraw(TRUE);
	Invalidate();
}

void CNewHeaderCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

/*	CTLFrame *pFrame = (CTLFrame *)GetParent();

	CRect m_clientRect;
	pFrame->m_tree.GetClientRect(&m_clientRect);

	if(m_RTL)
	{
		dc.SetViewportOrg(m_clientRect.Width(), 0);
		CSize ext = dc.GetViewportExt();
		ext.cx = ext.cx > 0 ? -ext.cx : ext.cx;

		dc.SetMapMode(MM_ANISOTROPIC);
		dc.SetViewportExt(ext);
	}
*/
	CWnd::DefWindowProc( WM_PAINT, (WPARAM)dc.m_hDC, 0 );

/*	if(m_RTL)
	{
		dc.SetViewportOrg(0, 0);
		dc.SetMapMode(MM_TEXT);
	}*/
	// Do not call CHeaderCtrl::OnPaint() for painting messages
}
