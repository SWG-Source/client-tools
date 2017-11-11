// SelectRegionDialog.cpp : implementation file
//

#include "FirstUiBuilder.h"
#include "SelectRegionDialog.h"

#include "UICanvas.h"
#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const float cMaxZoomLevel = 8.0f;

/////////////////////////////////////////////////////////////////////////////
// SelectRegionDialog dialog


SelectRegionDialog::SelectRegionDialog(CWnd* pParent /*=NULL*/)
:	CDialog(SelectRegionDialog::IDD, pParent),
	mOldSize(0,0),
	m_mouseWheelCounter(0)
{
	SelectRegionCanvas::registerClass();

	//{{AFX_DATA_INIT(SelectRegionDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void SelectRegionDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SelectRegionDialog)
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////

void SelectRegionDialog::_updateTextbox()
{
	UIString RectString;
	UIUtils::FormatRect(RectString, m_canvas.region());
	SetDlgItemText(IDC_VALUE, Unicode::wideToNarrow (RectString).c_str() );
}

// =========================================================================

void SelectRegionDialog::_updateSelectionRect()
{
	m_canvas.selectionRect().SetRect(m_canvas.region());
}

// =========================================================================

bool SelectRegionDialog::editProperty(UIImageStyle *theStyle, const UINarrowString &Value)
{
	UISmartPointer<UIImage> image(new UIImage);

	const UICanvas * const theCanvas = theStyle->GetSourceCanvas();
	if (!theCanvas)
	{
		return false;
	}

	image->SetCanvas(const_cast<UICanvas *>(theCanvas));

	UISize imageSize;
	theCanvas->GetSize(imageSize);

	// --------------------------------------------------------------

	m_canvas.construct(*image, imageSize, Value);

	int result = DoModal();
	if (result==IDOK)
	{
		UINarrowString newValue(static_cast<const char *>(m_canvas.regionText()));
		UIUtils::FormatRect(newValue, m_canvas.region());
		return true;
	}
	else
	{
		return false;
	}
}

// =========================================================================

bool SelectRegionDialog::editProperty(UIImage *theImage, const UINarrowString &Value)
{
	UISmartPointer<UIImage> image(UI_ASOBJECT(UIImage, theImage->DuplicateObject()));

	const UICanvas *theCanvas = image->GetCanvas();
	if (!theCanvas)
	{
		return false;
	}
	UISize imageSize;
	theCanvas->GetSize(imageSize);
	const UIRect bounds(imageSize);

	// --------------------------------------------------------------

	m_canvas.construct(*image, imageSize, Value);

	int result = DoModal();
	if (result==IDOK)
	{
		UINarrowString newValue(static_cast<const char *>(m_canvas.regionText()));
		UIUtils::FormatRect(newValue, m_canvas.region());
		return true;
	}
	else
	{
		return false;
	}
}

// =========================================================================

void SelectRegionDialog::_sizeToContent()
{
	RECT rcDialog;
	RECT rcImage;
	RECT rcDesktop;

	GetWindowRect(&rcDialog);
	m_canvas.GetWindowRect(&rcImage);

	// Note, I'm not using this rect as a rect, the right and bottom elements
	// are actually the width and height.
	rcDialog.right	 -= rcDialog.left; rcDialog.left = 0;
	rcDialog.bottom -= rcDialog.top;	 rcDialog.top  = 0;

	rcImage.right	-= rcImage.left;   rcImage.left = 0;
	rcImage.bottom -= rcImage.top;    rcImage.top  = 0;

	const UISize imageSize = m_canvas.imageSize();

	float zoomLevel = m_canvas.zoomLevel();
	UISize ZoomedImageSize( UIScalar( float(imageSize.x) * zoomLevel), UIScalar( float(imageSize.y) * zoomLevel));
	while( (ZoomedImageSize.x < rcImage.right) && (ZoomedImageSize.y < rcImage.bottom ) )
	{
		zoomLevel *= 2.0f;

		ZoomedImageSize.x = UIScalar( float(imageSize.x) * zoomLevel );
		ZoomedImageSize.y = UIScalar( float(imageSize.y) * zoomLevel );

		if (zoomLevel >= cMaxZoomLevel)
		{
			zoomLevel = cMaxZoomLevel;
			break;
		}
	}

	m_canvas.zoomLevel(zoomLevel);

	rcDialog.right  = rcDialog.right  - rcImage.right  + ZoomedImageSize.x + 1;
	rcDialog.bottom = rcDialog.bottom - rcImage.bottom + ZoomedImageSize.y + 1;
	
	SystemParametersInfo( SPI_GETWORKAREA, 0, &rcDesktop, 0 );

	// Center on desktop
	rcDialog.left = (rcDesktop.right - rcDesktop.left) / 2 + rcDesktop.left - rcDialog.right / 2;
	rcDialog.top  = (rcDesktop.bottom - rcDesktop.top) / 2 + rcDesktop.top - rcDialog.bottom / 2;

	MoveWindow(rcDialog.left, rcDialog.top, rcDialog.right, rcDialog.bottom, TRUE );
}

// =========================================================================

void SelectRegionDialog::_enableControls()
{
	const float zoomLevel = m_canvas.zoomLevel();
	if (zoomLevel <= 1.0f)
	{
		GetDlgItem(IDC_ZOOMIN)->EnableWindow(TRUE);
		GetDlgItem(IDC_ZOOMOUT)->EnableWindow(FALSE);
	}
	else if (zoomLevel >= cMaxZoomLevel)
	{
		GetDlgItem(IDC_ZOOMIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_ZOOMOUT)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_ZOOMIN)->EnableWindow(TRUE);
		GetDlgItem(IDC_ZOOMOUT)->EnableWindow(TRUE);
	}
}

// =========================================================================

void SelectRegionDialog::_showZoomLevel()
{
	char Buffer[256];

	const float zoomLevel = m_canvas.zoomLevel();
	if (zoomLevel != 1.0f)
	{
		sprintf( Buffer, "(%d%% Zoom)", int(zoomLevel * 100.0f) );
	}
	else
	{
		sprintf( Buffer, "(Actual Pixels)" );
	}

	SetDlgItemText(IDC_ZOOMLEVEL, Buffer);
}

// =========================================================================

void SelectRegionDialog::_readRectFromTextbox()
{
	CWnd *control = GetDlgItem(IDC_VALUE);
	if (control)
	{
		CString newValue;
		control->GetWindowText(newValue);
		m_canvas.updateValue(newValue);
	}
}

// =========================================================================

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(SelectRegionDialog, CDialog)
	ON_CONTROL(SelectRegionCanvas::NC_rectChanged, IDC_CANVAS, OnCanvasRectChanged)
	ON_CONTROL(SelectRegionCanvas::NC_zoomUp, IDC_CANVAS, OnCanvasZoomUp)
	ON_CONTROL(SelectRegionCanvas::NC_zoomDown, IDC_CANVAS, OnCanvasZoomDown)
	ON_WM_GETMINMAXINFO()
	//{{AFX_MSG_MAP(SelectRegionDialog)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_DRAWHIGHLIGHT, OnDrawhighlight)
	ON_BN_CLICKED(IDC_ZOOMIN, OnZoomin)
	ON_BN_CLICKED(IDC_ZOOMOUT, OnZoomout)
	ON_EN_CHANGE(IDC_VALUE, OnChangeValue)
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SelectRegionDialog message handlers
void SelectRegionDialog::OnGetMinMaxInfo(MINMAXINFO FAR* pmmi)
{
	CRect rc;

	rc.left   = 0;
	rc.top    = 0;
	rc.right  = mInitialSize.cx;
	rc.bottom = mInitialSize.cy;

	DWORD style = GetStyle();
	DWORD exStyle = GetExStyle();

	AdjustWindowRectEx(&rc, style, TRUE, exStyle);
	pmmi->ptMinTrackSize.x = rc.right - rc.left;
	pmmi->ptMinTrackSize.y = rc.bottom - rc.top;
}

BOOL SelectRegionDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_canvas.SubclassDlgItem(IDC_CANVAS, this);

	_updateTextbox();
	_updateSelectionRect();
	
	m_canvas.SendMessage(WM_APP, 0, 0);
	
	// TODO AddTooltipFromControlID(gTooltip, m_hWnd, IDC_ZOOMIN, "Zoom In" );
	// TODO AddTooltipFromControlID(gTooltip, m_hWnd, IDC_ZOOMOUT, "Zoom Out" );
	
	SendDlgItemMessage(
		IDC_ZOOMIN, 
		BM_SETIMAGE, 
		IMAGE_ICON,
		(LPARAM)LoadImage(
			GetModuleHandle(0),
			MAKEINTRESOURCE(IDI_ZOOMIN),
			IMAGE_ICON,
			16,
			16,
			0
		) 
	);

	SendDlgItemMessage(
		IDC_ZOOMOUT, 
		BM_SETIMAGE, 
		IMAGE_ICON,
		(LPARAM)LoadImage(
			GetModuleHandle(0), 
			MAKEINTRESOURCE(IDI_ZOOMOUT), 
			IMAGE_ICON, 
			16, 
			16, 
			0 
		)
	);
	
	SendDlgItemMessage(
		IDC_DRAWHIGHLIGHT,
		BM_SETIMAGE,
		IMAGE_ICON,
		(LPARAM)LoadImage(
			GetModuleHandle(0),
			MAKEINTRESOURCE(IDI_DRAWHIGHLIGHT),
			IMAGE_ICON,
			16,
			16,
			0
		)
	);

	CheckDlgButton(IDC_DRAWHIGHLIGHT, BST_CHECKED);
	
	RECT rcClient;
	GetClientRect(&rcClient);
	SendMessage(WM_SIZE, 0, MAKELPARAM( rcClient.right, rcClient.bottom ) );
	
	_sizeToContent();
	_enableControls();
	_showZoomLevel();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void SelectRegionDialog::OnClose() 
{
	EndDialog(0);
}

void SelectRegionDialog::_positionControl(int id, int x, int y, int cx, int cy)
{
	CRect r;
	r.left = x;
	r.top = y;
	r.right = x + cx;
	r.bottom = y + cy;

	MapDialogRect(&r);
	GetDlgItem(id)->MoveWindow(r.left, r.top, r.Width(), r.Height(), FALSE);
}

void SelectRegionDialog::_zoomUp()
{
	GetDlgItem(IDC_VALUE)->SetFocus();
	GetDlgItem(IDC_ZOOMIN)->ModifyStyle(BS_DEFPUSHBUTTON, 0);

	float zoomLevel = m_canvas.zoomLevel();
	zoomLevel *= 2.0f;
	if (zoomLevel > cMaxZoomLevel)
	{
		zoomLevel = cMaxZoomLevel;
	}
	m_canvas.zoomLevel(zoomLevel);

	_enableControls();
	_showZoomLevel();

	const UIRect & rect = m_canvas.selectionRect().GetRect();
	const UIPoint center ((rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2);

	m_canvas.updateScrollbars(&center);
	m_canvas.InvalidateRect(0, FALSE); 
}

void SelectRegionDialog::_zoomDown()
{
	GetDlgItem(IDC_VALUE)->SetFocus();
	GetDlgItem(IDC_ZOOMOUT)->ModifyStyle(BS_DEFPUSHBUTTON, 0);

	float zoomLevel = m_canvas.zoomLevel();
	zoomLevel /= 2.0f;
	if( zoomLevel < 1.0f )
		zoomLevel = 1.0f;
	m_canvas.zoomLevel(zoomLevel);

	_enableControls();
	_showZoomLevel();

	const UIRect & rect = m_canvas.selectionRect().GetRect();
	const UIPoint center ((rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2);

	m_canvas.updateScrollbars(&center);
	m_canvas.InvalidateRect(0, FALSE); 
}

void SelectRegionDialog::OnSize(UINT nType, int cx, int cy) 
{
	if (cx>0 && cy>0)
	{
		CSize NewSize;
		
		NewSize.cx = cx;
		NewSize.cy = cy;

		// 246, 190
		// 
		// UR-corner, track to right edge.
		CWnd *okButton = GetDlgItem(IDOK);
		if (okButton)
		{
			CRect dialogMapping(0, 0, 256, 256);
			MapDialogRect(dialogMapping);

			const int dx = cx * 256 / dialogMapping.Width();
			const int dy = cy * 256 / dialogMapping.Height();

			_positionControl(IDOK,               dx-56,   5,  50,  14);
			_positionControl(IDCANCEL,           dx-56,  24,  50,  14);

			// LL-corner, track to bottom edge
			_positionControl(IDC_DRAWHIGHLIGHT,  136, dy-15,  15,  14);
			_positionControl(IDC_ZOOMIN,         156, dy-15,  15,  14);
			_positionControl(IDC_ZOOMOUT,        170, dy-15,  15,  14);
			_positionControl(IDC_ZOOMLEVEL,      190, dy-13,  50,  10);
			_positionControl(IDC_VALUELABEL,       5, dy-13,  26,   8);
			_positionControl(IDC_VALUE,           35, dy-15,  95,  12);

			// UL-corner, scale to fill available area.
			_positionControl(IDC_CANVAS,           5,     5, dx-65, dy-25);

			RedrawWindow();
		}

		if (mOldSize.cx==0)
		{
			mInitialSize = NewSize;
		}

		mOldSize = NewSize;
	}
	CDialog::OnSize(nType, cx, cy);
}

void SelectRegionDialog::OnOK() 
{
	_readRectFromTextbox();				
	EndDialog(1);
}

void SelectRegionDialog::OnCancel() 
{
	EndDialog(0);
}

void SelectRegionDialog::OnDrawhighlight() 
{
	bool drawHighlight = IsDlgButtonChecked(IDC_DRAWHIGHLIGHT) != 0;
	m_canvas.drawHighlight(drawHighlight);
	GetDlgItem(IDC_VALUE)->SetFocus();
	m_canvas.InvalidateRect(0, FALSE); 
}

void SelectRegionDialog::OnZoomin() 
{
	_zoomUp();
}

void SelectRegionDialog::OnZoomout() 
{
	_zoomDown();
}

void SelectRegionDialog::OnChangeValue() 
{
	_readRectFromTextbox();
	_updateSelectionRect();
	m_canvas.InvalidateRect(0, FALSE);
}

BOOL SelectRegionDialog::PreCreateWindow(CREATESTRUCT& cs) 
{
	return CDialog::PreCreateWindow(cs);
}

void SelectRegionDialog::OnCanvasRectChanged()
{
	_updateTextbox();
}

void SelectRegionDialog::OnCanvasZoomUp()
{
	_zoomUp();
}

void SelectRegionDialog::OnCanvasZoomDown()
{
	_zoomDown();
}

BOOL SelectRegionDialog::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	CRect canvasRect;
	m_canvas.GetWindowRect(&canvasRect);

	if (canvasRect.PtInRect(pt))
	{
		m_mouseWheelCounter+=int(zDelta);
		if (m_mouseWheelCounter>0)
		{
			while (m_mouseWheelCounter>=WHEEL_DELTA)
			{
				m_mouseWheelCounter-=WHEEL_DELTA;
				_zoomUp();
			}
		}
		else if (m_mouseWheelCounter<0)
		{
			while (m_mouseWheelCounter<=-WHEEL_DELTA)
			{
				m_mouseWheelCounter+=WHEEL_DELTA;
				_zoomDown();
			}
		}
	}

	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}
