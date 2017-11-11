// DialogPalette.cpp : implementation file
#include "FirstTerrainEditor.h"
#include "DialogPalette.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "sharedFile/Iff.h"

/////////////////////////////////////////////////////////////////////////////
// CDialogPalette dialog


CDialogPalette::CDialogPalette(CWnd* pParent, const char* windowName)
	: CDialog(CDialogPalette::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogPalette)
	m_StringBlue = _T("");
	m_StringGreen = _T("");
	m_StringRed = _T("");
	//}}AFX_DATA_INIT

	// Init nodes and default ramp
	Init();

	// Init geometry
	m_width = 20;
	m_height = 20;
	m_space = 2;

	// Init selection
	m_selected = 0;
	m_dragging = false;
	m_windowName = windowName;
}




//**************************************************
// Init
// Default : grey scale
//**************************************************
void CDialogPalette::Init()
{
	for(int i=0;i <256;i++)
		{
			PackedRgb color (static_cast<unsigned char> (i), static_cast<unsigned char> (i), static_cast<unsigned char> (i));
			colorRamp.setColor (i, color);
			colorRamp.setNode (i, false);
		}
	// Nodes 0 & 255 selected
	colorRamp.setNode (0, true);
	colorRamp.setNode (255, true);
}

void CDialogPalette::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogPalette)
	DDX_Text(pDX, IDC_EDIT_BLUE, m_StringBlue);
	DDX_Text(pDX, IDC_EDIT_GREEN, m_StringGreen);
	DDX_Text(pDX, IDC_EDIT_RED, m_StringRed);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogPalette, CDialog)
	//{{AFX_MSG_MAP(CDialogPalette)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_BUTTON_CHANGE, OnButtonChange)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_ALL, OnButtonDeleteAll)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, OnButtonLoad)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnButtonSave)
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogPalette message handlers

BOOL CDialogPalette::OnInitDialog() 
{
  CDialog::OnInitDialog();

  SetWindowText (m_windowName);
  // Bitmap and drawing...
  // MemDC and Bitmap
  CSize SizeImage(16*(m_width+m_space),16*(m_height+m_space)+30);
  CWindowDC dc(this);
  int NbPlane = dc.GetDeviceCaps(PLANES);
  int NbBit = dc.GetDeviceCaps(BITSPIXEL);
  m_MemDC.CreateCompatibleDC(&dc);//int nWidth, int nHeight, UINT nPlanes, UINT nBitcount, const void* lpBits );
  m_Bitmap.CreateBitmap(SizeImage.cx,SizeImage.cy,NbPlane,NbBit,NULL);
  m_MemDC.SelectObject(&m_Bitmap);
  m_MemDC.SetMapMode(MM_TEXT);
  CBrush brush;
  brush.CreateSolidBrush(::GetSysColor(COLOR_3DFACE));
  CPen pen(PS_SOLID,1,::GetSysColor(COLOR_3DFACE));
  m_MemDC.SelectObject(&pen);
  m_MemDC.SelectObject(&brush);
  m_MemDC.Rectangle(0,0,SizeImage.cx,SizeImage.cy);
	this->DrawPalette();	
  m_oldi = 0;
  m_oldj = 0;

	SetDlgItemInt(IDC_EDIT_RED,colorRamp.getColor (0).r);
	SetDlgItemInt(IDC_EDIT_GREEN,colorRamp.getColor (0).g);
	SetDlgItemInt(IDC_EDIT_BLUE,colorRamp.getColor (0).b);

	// Create the ToolTip control.
	m_tooltip.Create(this);
	m_tooltip.Activate(TRUE);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON_ADD), "Ramps are calculated from node to node");
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON_DELETE), "Delete selected node");
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON_DELETE_ALL), "Delete every node between first and last");
	m_tooltip.AddTool(GetDlgItem(IDC_EDIT_SELECTED), "Entry selected in the ramp");
	m_tooltip.AddTool(GetDlgItem(IDC_EDIT_RED), "Red");
	m_tooltip.AddTool(GetDlgItem(IDC_EDIT_GREEN), "Green");
	m_tooltip.AddTool(GetDlgItem(IDC_EDIT_BLUE), "Blue");

	return TRUE;  
}

//********************************************
// MakePalette
//********************************************
void CDialogPalette::MakePalette() 
{
 int i, j;
 m_ArrayNode.RemoveAll();
 for(i=0;i<=255;i++)
   {
   if(colorRamp.getNode (i))
     m_ArrayNode.Add(i);
   }
 
 int size = m_ArrayNode.GetSize();
 TRACE("%d nodes\n",size);
 int x1,y1,x2,y2,b;
 float a;
 for(i=0;i<size-1;i++)
   {
    x1=m_ArrayNode[i];
    x2=m_ArrayNode[i+1];
    VERIFY(x1<x2);
    y1=colorRamp.getRed (x1);
    y2=colorRamp.getRed (x2);
    a = (float)(y2-y1)/(float)(x2-x1);
    b = y1 - (int)(a*(float)x1);
    for(j=x1;j<x2;j++)
      colorRamp.setRed (j, (BYTE)(a*(float)j+b));
   }
 for(i=0;i<size-1;i++)
   {
    x1=m_ArrayNode[i];
    x2=m_ArrayNode[i+1];
    VERIFY(x1<x2);
    y1=colorRamp.getGreen (x1);
    y2=colorRamp.getGreen (x2);
    a = (float)(y2-y1)/(float)(x2-x1);
    b = y1 - (int)(a*(float)x1);
    for(j=x1;j<x2;j++)
      colorRamp.setGreen (j, (BYTE)(a*(float)j+b));
   }
 for(i=0;i<size-1;i++)
   {
    x1=m_ArrayNode[i];
    x2=m_ArrayNode[i+1];
    VERIFY(x1<x2);
    y1=colorRamp.getBlue (x1);
    y2=colorRamp.getBlue (x2);
    a = (float)(y2-y1)/(float)(x2-x1);
    b = y1 - (int)(a*(float)x1);
    for(j=x1;j<x2;j++)
      colorRamp.setBlue (j, (BYTE)(a*(float)j+b));
   }
}

//********************************************
// DrawItem
//********************************************
void CDialogPalette::DrawItem(CDC *pDC,
                              COLORREF LineColor,
                              COLORREF FillColor,
                              CRect &rect) 
{
	CBrush brush;
  brush.CreateSolidBrush(FillColor);
	CPen pen(PS_SOLID,1,LineColor);
  CPen *pOldPen = m_MemDC.SelectObject(&pen);
  CBrush *pOldBrush = m_MemDC.SelectObject(&brush);
  pDC->RoundRect(rect.left,rect.top,rect.right+1,rect.bottom+1,0,0);
  m_MemDC.SelectObject(pOldBrush);      
  m_MemDC.SelectObject(pOldPen);
}

//********************************************
// DrawPalette
//********************************************
void CDialogPalette::DrawPalette() 
{
  CRect rect;
  CRect item;
  int i,j;
  for(j=0;j<16;j++)
    for(i=0;i<16;i++)
     {
      item.SetRect(i*(m_width+m_space),j*(m_width+m_space),
                   i*(m_width+m_space)+m_width,j*(m_width+m_space)+m_height);

      // Selected item
      if(colorRamp.getNode (j*16+i))
        { 
         rect.SetRect(item.left,item.top,item.right,item.bottom);
         this->DrawItem(&m_MemDC,RGB(255,0,0),RGB(0,0,0),rect);
        }
	  else
        { 
         rect.SetRect(item.left,item.top,item.right,item.bottom);
         this->DrawItem(&m_MemDC,::GetSysColor(COLOR_3DFACE),RGB(0,0,0),rect);
        }
      if(m_selected == (j*16+i))
        {
         rect.SetRect(item.left+1,item.top+1,item.right-1,item.bottom-1);
         this->DrawItem(&m_MemDC,RGB(255,255,255),RGB(colorRamp.getRed (j*16+i), colorRamp.getGreen (j*16+i), colorRamp.getBlue (j*16+i)),rect);
        }
      else
        {
         rect.SetRect(item.left+1,item.top+1,item.right-1,item.bottom-1);
         this->DrawItem(&m_MemDC,::GetSysColor(COLOR_3DFACE),RGB(colorRamp.getRed (j*16+i), colorRamp.getGreen (j*16+i), colorRamp.getBlue (j*16+i)),rect);
        }
      
     }

	int left = 0;
	int top = 16*(m_height+m_space)+10;
	for(i=0;i<256;i++)
	   this->DrawLine(&m_MemDC,left+i,top,left+i,top+20,RGB(colorRamp.getRed (i), colorRamp.getGreen (i), colorRamp.getBlue (i)));
}

//********************************************
// DrawPalette
//********************************************
void CDialogPalette::DrawPaletteItem(int i,int j) 
{
  CRect rect;
  CRect item;
  item.SetRect(i*(m_width+m_space),j*(m_width+m_space),
               i*(m_width+m_space)+m_width,j*(m_width+m_space)+m_height);

  // Selected item
  if(colorRamp.getNode (j*16+i))
    { 
     rect.SetRect(item.left,item.top,item.right,item.bottom);
     this->DrawItem(&m_MemDC,RGB(255,0,0),RGB(0,0,0),rect);
    }
	else
    { 
     rect.SetRect(item.left,item.top,item.right,item.bottom);
     this->DrawItem(&m_MemDC,::GetSysColor(COLOR_3DFACE),RGB(0,0,0),rect);
    }
  if(m_selected == (j*16+i))
    {
     rect.SetRect(item.left+1,item.top+1,item.right-1,item.bottom-1);
	 this->DrawItem(&m_MemDC,RGB(255,255,255),RGB(colorRamp.getRed (j*16+i), colorRamp.getGreen (j*16+i), colorRamp.getBlue (j*16+i)),rect);
    }
  else
    {
     rect.SetRect(item.left+1,item.top+1,item.right-1,item.bottom-1);
     this->DrawItem(&m_MemDC,::GetSysColor(COLOR_3DFACE),RGB(colorRamp.getRed (j*16+i), colorRamp.getGreen (j*16+i), colorRamp.getBlue (j*16+i)),rect);
    }
      
}



void CDialogPalette::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	dc.BitBlt(10,10,16*(m_width+m_space),16*(m_height+m_space)+30,&m_MemDC,0,0,SRCCOPY);

}

void CDialogPalette::OnClose() 
{
 m_MemDC.DeleteDC();
 m_Bitmap.DeleteObject();
 CDialog::OnClose();
}

void CDialogPalette::OnMouseMove (UINT nFlags, CPoint point) 
{
	if (m_dragging)
	{
		CRect rect; 
		int i,j; 
		for (j = 0; j < 16; j++)
		{
			for (i = 0; i < 16; i++)
			{
				rect.SetRect (10 + i * (m_width + m_space),10 + j * (m_width + m_space), 10 + i * (m_width + m_space) + m_width,10 + j * (m_height + m_space) + m_height); 
				if (rect.PtInRect (point))
				{
					int m_new = j * 16 + i; 
					if (m_new != m_dragOldIndex)
					{
						colorRamp.setColor (m_dragOldIndex, m_dragOldColor);
						colorRamp.setNode  (m_dragOldIndex, m_dragOldNode);

						m_dragOldIndex = m_new;
						m_dragOldColor = colorRamp.getColor (m_dragOldIndex);
						m_dragOldNode  = colorRamp.getNode  (m_dragOldIndex);
						
						colorRamp.setColor (m_dragOldIndex, m_dragNewColor);
						colorRamp.setNode  (m_dragOldIndex, true);

						m_selected = m_new;

						this->MakePalette (); 
						this->DrawPalette (); 

						RedrawWindow (NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE );  
					}
				}
			}
		}
	}

	CDialog::OnMouseMove (nFlags, point); 
}

void CDialogPalette::OnLButtonDown(UINT nFlags, CPoint point) 
{
 CRect rect;
 int i,j;
 for(j=0;j<16;j++)
   for(i=0;i<16;i++)
   {
    rect.SetRect(10+i*(m_width+m_space),10+j*(m_width+m_space),
                 10+i*(m_width+m_space)+m_width,10+j*(m_height+m_space)+m_height);
    if(rect.PtInRect(point))
      {
      m_selected = j*16+i;
      this->DrawPaletteItem(m_oldi,m_oldj);
      this->DrawPaletteItem(i,j);
      CRect rect(10,10,10+16*(m_width+m_space)+m_width,10+16*(m_height+m_space)+m_width+30);
      RedrawWindow(rect,NULL,RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE ); 
      SetDlgItemInt(IDC_EDIT_SELECTED,m_selected);
      SetDlgItemInt(IDC_EDIT_RED,colorRamp.getRed (m_selected));
      SetDlgItemInt(IDC_EDIT_GREEN,colorRamp.getGreen (m_selected));
      SetDlgItemInt(IDC_EDIT_BLUE,colorRamp.getBlue (m_selected));
      m_oldi = i;
      m_oldj = j;
	  m_dragging = colorRamp.getNode (m_selected) && m_selected != 0 && m_selected != 255;
		if (m_dragging)
		{
			m_dragOldIndex = m_selected;
			m_dragOldColor = colorRamp.getColor (m_dragOldIndex);
			m_dragOldNode  = false;
			m_dragNewColor = m_dragOldColor;
		}
      }
   }


  CDialog::OnLButtonDown(nFlags, point);
}

void CDialogPalette::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_dragging = false;
	
	CDialog::OnLButtonUp(nFlags, point);
}

void CDialogPalette::OnRButtonDown(UINT nFlags, CPoint point) 
{
 CRect rect;
 int i,j;
 for(j=0;j<16;j++)
   for(i=0;i<16;i++)
   {
    rect.SetRect(10+i*(m_width+m_space),10+j*(m_width+m_space),
                 10+i*(m_width+m_space)+m_width,10+j*(m_height+m_space)+m_height);
    if(rect.PtInRect(point))
      {
		  int m_new = j*16+i;
		  if (colorRamp.getNode (m_selected))
		  {
			  colorRamp.setColor (m_new, colorRamp.getColor (m_selected));
			  colorRamp.setNode  (m_new, true);

			  this->MakePalette ();
			  this->DrawPalette ();

			   RedrawWindow(NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE ); 
		  }
      }
   }
	
	CDialog::OnRButtonDown(nFlags, point);
}

//**************************************************
// OnButtonChange
//**************************************************
void CDialogPalette::OnButtonChange() 
{
	if (colorRamp.getNode (m_selected))
	{
		CColorDialog dlg (RGB(colorRamp.getRed (m_selected), colorRamp.getGreen (m_selected), colorRamp.getBlue (m_selected)), CC_FULLOPEN, this);

		if(dlg.DoModal()==IDOK)
		{
			COLORREF color = dlg.GetColor();
			SetDlgItemInt(IDC_EDIT_RED,GetRValue(color));
			SetDlgItemInt(IDC_EDIT_GREEN,GetGValue(color));
			SetDlgItemInt(IDC_EDIT_BLUE,GetBValue(color));
			colorRamp.setRed (m_selected, GetRValue(color));
			colorRamp.setGreen (m_selected, GetGValue(color));
			colorRamp.setBlue (m_selected, GetBValue(color));
			if(colorRamp.getNode (m_selected))
				this->MakePalette();
			this->DrawPalette();
			RedrawWindow(NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE ); 
		}
	}
}

void CDialogPalette::OnButtonAdd() 
{
colorRamp.setNode (m_selected, true);
this->MakePalette();
this->DrawPalette();
CRect rect(10,10,10+16*(m_width+m_space)+m_width,10+16*(m_height+m_space)+m_width+30);
RedrawWindow(NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE ); 
}

//**************************************************
// OnButtonDelete
//**************************************************
void CDialogPalette::OnButtonDelete() 
{
	if(m_selected == 0 || m_selected == 255)
		return; 
	colorRamp.setNode (m_selected, false);
	MakePalette();
	DrawPalette();
	RedrawWindow(NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE ); 
}


//*********************************************************
//  DrawLine
//*********************************************************
void CDialogPalette::DrawLine(CDC *pDC,
                           int x1,
                           int y1,
                           int x2,
                           int y2,
                           COLORREF color /* = RGB(0,0,0) */,
                           int PenStyle   /* = PS_SOLID     */)
{
 if(!pDC)
   return;
 CPen pen(PenStyle,1,color);
 CPen *pOldPen; 
 pOldPen = pDC->SelectObject(&pen);
 int OldBkMode = pDC->SetBkMode(TRANSPARENT);
 pDC->MoveTo(x1,y1);
 pDC->LineTo(x2,y2);
 pDC->SelectObject(pOldPen);
 pDC->SetBkMode(OldBkMode);
}


//**************************************************
// OnButtonDeleteAll
//**************************************************
void CDialogPalette::OnButtonDeleteAll() 
{
	int i;
	for(i=1;i<=254;i++)
		colorRamp.setNode (i, false);
	MakePalette();
	DrawPalette();
	RedrawWindow(NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE ); 
}

BOOL CDialogPalette::PreTranslateMessage(MSG* pMsg)
{
	// CG: The following block was added by the ToolTips component.
	{
		// Let the ToolTip process this message.
		m_tooltip.RelayEvent(pMsg);

		return CDialog::PreTranslateMessage(pMsg);
	}
}

//**************************************************
// OnOK
//**************************************************
void CDialogPalette::OnOK() 
{
	m_MemDC.DeleteDC();
	m_Bitmap.DeleteObject();

	CDialog::OnOK();
}

void CDialogPalette::OnCancel() 
{
	m_MemDC.DeleteDC();
	m_Bitmap.DeleteObject();
	CDialog::OnCancel();
}

//**************************************************
// OnButtonLoad
//**************************************************
void CDialogPalette::OnButtonLoad() 
{
	CFileDialog dlg(TRUE,
                  _T("PAL"),
                  _T("*.PAL"),
                  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                  _T("Palette files (*.PAL)|*.PAL|") );
  if(dlg.DoModal()==IDOK)
    {
		CString FileName = dlg.GetPathName();
		Iff iff (FileName);
		colorRamp.load (iff);

    MakePalette();
    DrawPalette();
		RedrawWindow(NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE ); 
  }
	
}

//**************************************************
// OnButtonSave
//**************************************************
void CDialogPalette::OnButtonSave() 
{
	CFileDialog dlg(FALSE,
                  _T("PAL"),
                  _T("*.PAL"),
                  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                  _T("Palette files (*.PAL)|*.PAL|") );
  if(dlg.DoModal()==IDOK)
    {
    CString FileName = dlg.GetPathName();

		Iff iff (1024);
		colorRamp.save (iff);

		if (!iff.write (FileName, true))
		{
			AfxMessageBox("Unable to open file for writing");
		}
  }
}

const ColorRamp256& CDialogPalette::getColorRamp () const
{
	return colorRamp;
}

void CDialogPalette::setColorRamp (const ColorRamp256& newColorRamp)
{
	colorRamp = newColorRamp;
}



