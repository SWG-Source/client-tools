// DefaultObjectPropertiesDialog.cpp : implementation file
//

#include "FirstUiBuilder.h"
#include "DefaultObjectPropertiesDialog.h"
#include "AddPropertyDialogBox.h"

/////////////////////////////////////////////////////////////////////////////
// DefaultObjectPropertiesDialog dialog


DefaultObjectPropertiesDialog::DefaultObjectPropertiesDialog(DefaultObjectPropertiesManager &defaults, CWnd* pParent /*=NULL*/)
	: CDialog(DefaultObjectPropertiesDialog::IDD, pParent),
	m_defaults(defaults)
{
	//{{AFX_DATA_INIT(DefaultObjectPropertiesDialog)
	//}}AFX_DATA_INIT
}

/////////////////////////////////////////////////////////////////////////////

void DefaultObjectPropertiesDialog::_loadDataToDialog()
{
	m_workingObjectProperties = m_defaults.getDefaultProperties();

	for (DefaultObjectPropertiesList::const_iterator i = m_workingObjectProperties.begin(); i != m_workingObjectProperties.end(); ++i )
	{
		const UINarrowString &tname = (*i).TypeName;
		long InsertedAt = m_objectType.AddString(tname.c_str());
		m_objectType.SetItemData(InsertedAt, (DWORD)i->TypeID);
	}

	m_objectType.SetCurSel(0);
	_loadDefaultsToListbox();
}

void DefaultObjectPropertiesDialog::_loadDefaultsToListbox()
{
	m_propertyList.ResetContent();
	long CurrentSelection = m_objectType.GetCurSel();

	if (CurrentSelection >= 0)
	{
		UITypeID TypeID = (UITypeID)m_objectType.GetItemData(CurrentSelection);
		m_defaults.setSelectedPropertyMap(0);

		for (DefaultObjectPropertiesList::iterator i = m_workingObjectProperties.begin(); i != m_workingObjectProperties.end(); ++i )
		{
			if (i->TypeID == TypeID)
			{
				m_defaults.setSelectedPropertyMap(&(i->Properties));

				for (StringMap::iterator j = i->Properties.begin(); j != i->Properties.end(); ++j )
				{
					const UINarrowString & name = (*j).first;
					m_propertyList.AddString(name.c_str());
				}
			}
		}
	}
}

void DefaultObjectPropertiesDialog::_addNewProperty()
{
	AddPropertyDialogBox AddProp;

	if (AddProp.DoModal()==IDOK)
	{
		UINarrowString       NewName = static_cast<const char *>(AddProp.m_propertyName);
		UIString             NewValue = UIUnicode::narrowToWide(static_cast<const char *>(AddProp.m_propertyValue));

		StringMap *selectedPropertyMap = m_defaults.getSelectedPropertyMap();
		(*selectedPropertyMap)[NewName] = NewValue;
		_loadDefaultsToListbox();
	}

	m_propertyList.SetFocus();
	m_addProperty.ModifyStyle(BS_DEFPUSHBUTTON, 0);
}

void DefaultObjectPropertiesDialog::_removeSelectedProperty()
{
	const long CurrentSelection = m_propertyList.GetCurSel();

	if (CurrentSelection >= 0)
	{
		CString itemText;
		m_propertyList.GetText(CurrentSelection, itemText);

		StringMap *selectedPropertyMap = m_defaults.getSelectedPropertyMap();
		
		StringMap::iterator i = selectedPropertyMap->find(static_cast<const char *>(itemText));

		if (i != selectedPropertyMap->end())
		{
			selectedPropertyMap->erase(i);
		}

		_loadDefaultsToListbox();
	}

	m_propertyList.SetFocus();
	m_removeProperty.ModifyStyle(BS_DEFPUSHBUTTON, 0);
}

/////////////////////////////////////////////////////////////////////////////

void DefaultObjectPropertiesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DefaultObjectPropertiesDialog)
	DDX_Control(pDX, IDC_REMOVEPROPERTY, m_removeProperty);
	DDX_Control(pDX, IDC_ADDPROPERTY, m_addProperty);
	DDX_Control(pDX, IDC_PROPERTYLIST, m_propertyList);
	DDX_Control(pDX, IDC_OBJECTTYPE, m_objectType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DefaultObjectPropertiesDialog, CDialog)
	//{{AFX_MSG_MAP(DefaultObjectPropertiesDialog)
	ON_WM_CLOSE()
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_ADDPROPERTY, OnAddproperty)
	ON_BN_CLICKED(IDC_REMOVEPROPERTY, OnRemoveproperty)
	ON_CBN_SELCHANGE(IDC_OBJECTTYPE, OnSelchangeObjecttype)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DefaultObjectPropertiesDialog message handlers

BOOL DefaultObjectPropertiesDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// TODO AddTooltipFromControlID( gTooltip, gMainWindow, IDC_ADDPROPERTY, "Add Default Property" );
	// TODO AddTooltipFromControlID( gTooltip, gMainWindow, IDC_REMOVEPROPERTY, "Remove Default Property" );

	HICON icon;

	icon = (HICON)LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ADDPROPERTY), IMAGE_ICON, 16, 16, 0);
	m_addProperty.SetIcon(icon);

	icon = (HICON)LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_REMOVEPROPERTY), IMAGE_ICON, 16, 16, 0);
	m_removeProperty.SetIcon(icon);

	_loadDataToDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void DefaultObjectPropertiesDialog::OnClose() 
{
	// Equivalent to pressing cancel
	// TODO EndDialog( hwndDlg, 0 );
	
	CDialog::OnClose();
}

void DefaultObjectPropertiesDialog::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	HBITMAP   DoubleBufferBitmap, OldBitmap;
	HDC       DoubleBufferDC;		
	HFONT     OldFont;
	POINT     ItemSize;
	RECT      rcFill;
	RECT      rcClip;

	ItemSize.x = lpDrawItemStruct->rcItem.right - lpDrawItemStruct->rcItem.left;
	ItemSize.y = lpDrawItemStruct->rcItem.bottom - lpDrawItemStruct->rcItem.top;
	
	SetRect(&rcFill, 0, 0, ItemSize.x, ItemSize.y);
	SetRect(&rcClip, 2, 0, ItemSize.x / 2, ItemSize.y);
	
	DoubleBufferDC		 = CreateCompatibleDC( lpDrawItemStruct->hDC );
	DoubleBufferBitmap = CreateCompatibleBitmap( lpDrawItemStruct->hDC, ItemSize.x, ItemSize.y);

	OldBitmap = (HBITMAP)SelectObject( DoubleBufferDC, DoubleBufferBitmap );
	OldFont   = (HFONT)  SelectObject( DoubleBufferDC, GetCurrentObject( lpDrawItemStruct->hDC, OBJ_FONT));
	
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		CWnd *focus = GetFocus();

		if (focus == &m_propertyList)
		{
			FillRect(DoubleBufferDC, &rcFill, GetSysColorBrush( COLOR_HIGHLIGHT));
			SetTextColor(DoubleBufferDC, GetSysColor( COLOR_HIGHLIGHTTEXT));
		}
		else
		{
			FillRect(DoubleBufferDC, &rcFill, GetSysColorBrush( COLOR_BTNFACE));
			SetTextColor(DoubleBufferDC, GetSysColor( COLOR_BTNTEXT));
		}
	}
	else
	{
		FillRect( DoubleBufferDC, &rcFill, GetSysColorBrush( COLOR_WINDOW ) );
		SetTextColor( DoubleBufferDC, GetSysColor( COLOR_WINDOWTEXT ) );
	}

	SetBkMode( DoubleBufferDC, TRANSPARENT );

	if (lpDrawItemStruct->itemID != -1)
	{
		CString propertyName;
		m_propertyList.GetText(lpDrawItemStruct->itemID, propertyName);

		DrawText(DoubleBufferDC, propertyName, -1, &rcClip, DT_SINGLELINE );

		rcClip.left  = rcClip.right;
		rcClip.right = ItemSize.x;

		UIString Value;

		const StringMap *selectedPropertyMap = m_defaults.getSelectedPropertyMap();
		StringMap::const_iterator i = selectedPropertyMap->find(static_cast<const char *>(propertyName));
		
		if (i != selectedPropertyMap->end())
		{
			Value = (*i).second;
		}
		
		DrawText(DoubleBufferDC, UIUnicode::wideToNarrow(Value).c_str(), -1, &rcClip, DT_SINGLELINE );
	}
	
	BitBlt( lpDrawItemStruct->hDC, lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top, ItemSize.x, ItemSize.y, DoubleBufferDC, 0, 0, SRCCOPY );

	SelectObject(DoubleBufferDC, OldFont);
	SelectObject(DoubleBufferDC, OldBitmap);
	DeleteObject(DoubleBufferBitmap);
	DeleteDC(DoubleBufferDC);
}

void DefaultObjectPropertiesDialog::OnOK() 
{
	// TODO mDefaultObjectProperties = mWorkingObjectProperties;
	// TODO EndDialog( hwndDlg, 0 );
	
	CDialog::OnOK();
}

void DefaultObjectPropertiesDialog::OnCancel() 
{
	// TODO EndDialog( hwndDlg, 0 );
	
	CDialog::OnCancel();
}

void DefaultObjectPropertiesDialog::OnAddproperty() 
{
	_addNewProperty();
}

void DefaultObjectPropertiesDialog::OnRemoveproperty() 
{
	_removeSelectedProperty();
}

void DefaultObjectPropertiesDialog::OnSelchangeObjecttype() 
{
	_loadDefaultsToListbox();
}
