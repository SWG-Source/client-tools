/ UIFontBuilderDlg.cpp : implementation file
//

#include "stdafx.h"
#include <afxdlgs.h>

#include "UIFontBuilder.h"
#include "UIFontBuilderDlg.h"
#include "PreviewDialog.h"

#include "UnicodeUtils.h"
#include "UnicodeBlocks.h"
#include "UnicodeCharacterData.h"
#include "UnicodeCharacterDataMap.h"
#include "FontExporterPSUI.h"
#include "FontImageExporterBMP.h"
#include "FontImageExporterDDS.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int IMAGE_INCLUDE_DEFAULT = 3;
static const int IMAGE_INCLUDE         = 2;
static const int IMAGE_EXCLUDE         = 1;
static const int IMAGE_NONE            = 0;

static const int IMAGE_BUTTON_DOWN      = 0;
static const int IMAGE_BUTTON_UP        = 1;
static const int IMAGE_BUTTON_DOWN_FULL = 2;
static const int IMAGE_BUTTON_UP_FULL   = 3;

using Unicode::unicode_char_t;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUIFontBuilderDlg dialog

CUIFontBuilderDlg::CUIFontBuilderDlg(TCHAR * unicodeFilename, CWnd* pParent /*=NULL*/)
	: CDialog(CUIFontBuilderDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUIFontBuilderDlg)
	m_textFontSampleText = _T("");
	m_textFontSizeText = _T("");
	m_editImageNameText = _T("");
	m_editStyleNameText = _T("");
	m_checkAntialiasValue = FALSE;
	m_editPadBottomValue = _T("");
	m_editPadLeftValue = _T("");
	m_editPadRightValue = _T("");
	m_editPadTopValue = _T("");
	m_excludeControlValue = FALSE;
	m_checkOutlinesValue = FALSE;
	m_editStyleFileValue = _T("");
	m_editOutputDirImageValue = _T("");
	m_editTextPixelsValue = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_unicodeFilename = Unicode::wideToNarrow (unicodeFilename);
}

//-----------------------------------------------------------------

CUIFontBuilderDlg::~CUIFontBuilderDlg()
{
	delete m_nodeImageList;
	delete m_buttonImageList;
	delete m_unicodeFont;
	delete m_fixedFont;
	delete m_selectedFont;
}

//-----------------------------------------------------------------

void CUIFontBuilderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUIFontBuilderDlg)
	DDX_Control(pDX, IDC_EDIT_OUTPUTDIR_IMAGE, m_editOutputDirImage);
	DDX_Control(pDX, IDC_COMBO_TILESIZE, m_comboTileSize);
	DDX_Control(pDX, IDC_COMBO_FILEFORMAT, m_comboFileFormat);
	DDX_Control(pDX, IDC_BUTTON_NODE_UP_FULL, m_buttonNodeUpFull);
	DDX_Control(pDX, IDC_BUTTON_NODE_UP, m_buttonNodeUp);
	DDX_Control(pDX, IDC_BUTTON_NODE_DOWN_FULL, m_buttonNodeDownFull);
	DDX_Control(pDX, IDC_BUTTON_NODE_DOWN, m_buttonNodeDown);
	DDX_Control(pDX, IDC_EDIT_STYLE_NAME, m_editStyleName);
	DDX_Control(pDX, IDC_EDIT_STYLE_FILE, m_editStyleFile);
	DDX_Control(pDX, IDC_EDIT_IMAGE_NAME, m_editImageName);
	DDX_Control(pDX, IDC_TEXT_FONTSAMPLE, m_textFontSample);
	DDX_Control(pDX, IDC_TREE_UNICODE_SUBSETS, m_treeUnicodeSubsets);
	DDX_Text(pDX, IDC_TEXT_FONTSAMPLE, m_textFontSampleText);
	DDX_Text(pDX, IDC_TEXT_FONTSIZE, m_textFontSizeText);
	DDX_Text(pDX, IDC_EDIT_IMAGE_NAME, m_editImageNameText);
	DDX_Text(pDX, IDC_EDIT_STYLE_NAME, m_editStyleNameText);
	DDX_Check(pDX, IDC_CHECK_ANTIALIAS, m_checkAntialiasValue);
	DDX_Text(pDX, IDC_EDIT_PAD_BOTTOM, m_editPadBottomValue);
	DDX_Text(pDX, IDC_EDIT_PAD_LEFT, m_editPadLeftValue);
	DDX_Text(pDX, IDC_EDIT_PAD_RIGHT, m_editPadRightValue);
	DDX_Text(pDX, IDC_EDIT_PAD_TOP, m_editPadTopValue);
	DDX_Check(pDX, IDC_CHECK_EXCLUDE_CONTROL, m_excludeControlValue);
	DDX_Check(pDX, IDC_CHECK_OUTLINES, m_checkOutlinesValue);
	DDX_Text(pDX, IDC_EDIT_STYLE_FILE, m_editStyleFileValue);
	DDX_Text(pDX, IDC_EDIT_OUTPUTDIR_IMAGE, m_editOutputDirImageValue);
	DDX_Text(pDX, IDC_EDIT_TEXT_PIXELS, m_editTextPixelsValue);
	DDV_MinMaxInt(pDX, m_editTextPixelsValue, 0, 40);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CUIFontBuilderDlg, CDialog)
	//{{AFX_MSG_MAP(CUIFontBuilderDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SELECT_FONT, OnSelectFontButton)
	ON_BN_CLICKED(ID_QUIT, OnQuit)
	ON_BN_CLICKED(ID_BUTTON_COLLAPSE_ALL, OnButtonCollapseAll)
	ON_BN_CLICKED(ID_BUTTON_EXPAND_ALL, OnButtonExpandAll)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_TREE_UNICODE_SUBSETS, OnBeginlabeleditTreeUnicodeSubsets)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_TREE_UNICODE_SUBSETS, OnEndlabeleditTreeUnicodeSubsets)
	ON_BN_CLICKED(ID_BUTTON_NEW_INCLUSION, OnButtonNewInclusion)
	ON_BN_CLICKED(ID_BUTTON_NEW_EXCLUSION, OnButtonNewExclusion)
	ON_BN_CLICKED(ID_BUTTON_DELETE_RANGE, OnButtonDeleteRange)
	ON_BN_CLICKED(ID_BUTTON_EDIT_RANGE, OnButtonEditRange)
	ON_BN_CLICKED(IDC_BUTTON_RELOAD_BLOCKS, OnButtonReloadBlocks)
	ON_BN_CLICKED(ID_BUTTON_PREVIEW_RANGE, OnButtonPreviewRange)
	ON_BN_CLICKED(IDC_BUTTON_NODE_UP, OnButtonNodeUp)
	ON_BN_CLICKED(IDC_BUTTON_NODE_DOWN, OnButtonNodeDown)
	ON_BN_CLICKED(IDC_BUTTON_NODE_DOWN_FULL, OnButtonNodeDownFull)
	ON_BN_CLICKED(IDC_BUTTON_NODE_UP_FULL, OnButtonNodeUpFull)
	ON_BN_CLICKED(ID_BUILD_FONT, OnBuildFont)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_STYLE_FILE, OnButtonChangeStyleFile)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_OUTPUTDIR_IMAGE, OnButtonChangeOutputdirImage)
	ON_EN_CHANGE(IDC_EDIT_TEXT_PIXELS, OnChangeEditTextPixels)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------
inline int  CUIFontBuilderDlg::convertPointTenthsToPixels (int pointTenths)
{
	return MulDiv(pointTenths, GetDeviceCaps(CreateCompatibleDC (0), LOGPIXELSY), 72) / 10;
}
//-----------------------------------------------------------------

inline int CUIFontBuilderDlg::convertPixelsToPointTenths (int pixels)
{
	return MulDiv(720, pixels, GetDeviceCaps(CreateCompatibleDC (0), LOGPIXELSY));
}
//-----------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// CUIFontBuilderDlg message handlers

BOOL CUIFontBuilderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}


	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// check for default font size  from registry
	long len = _MAX_PATH - 1;
	TCHAR buf[_MAX_PATH];
	
	if (RegQueryValue (HKEY_CURRENT_USER, CUIFontBuilderApp::ms_lastFontSizeKey, buf, &len) != ERROR_SUCCESS)
	{
		m_selectedFontSizePointTenths = 200;
	}
	else
	{
		m_selectedFontSizePointTenths = _wtoi (buf);
	}

	// check for default font face from registry
	len = _MAX_PATH - 1;
	if (RegQueryValue (HKEY_CURRENT_USER, CUIFontBuilderApp::ms_lastFontFaceKey, buf, &len) != ERROR_SUCCESS)
	{
		wcscpy (buf, _T ("Lucida Sans Unicode"));
	}

	m_selectedFont = new CFont ();
	m_selectedFont->CreatePointFont (m_selectedFontSizePointTenths, buf);
	m_selectedFont->GetLogFont (&m_selectedLogFont);

	//-- setup default output dirs
	len = _MAX_PATH - 1;
	if (RegQueryValue (HKEY_CURRENT_USER, CUIFontBuilderApp::ms_lastStyleFileKey, buf, &len) != ERROR_SUCCESS)
	{
		*buf = 0;
	}

	m_editStyleFileValue = buf;

	//-- setup default output dirs
	len = _MAX_PATH - 1;
	if (RegQueryValue (HKEY_CURRENT_USER, CUIFontBuilderApp::ms_lastImageDirKey, buf, &len) != ERROR_SUCCESS)
	{
		*buf = 0;
	}

	m_editOutputDirImageValue = buf;

	// setup blocks
	m_fixedFont = new CFont      ();
	m_fixedFont->CreatePointFont (80, _T("lucida console"));
	m_treeUnicodeSubsets.SetFont (m_fixedFont);
	m_editImageName.SetFont      (m_fixedFont);
	m_editStyleFile.SetFont      (m_fixedFont);
	m_editStyleName.SetFont      (m_fixedFont);
	m_editOutputDirImage.SetFont (m_fixedFont);
	m_comboTileSize.SetCurSel (4);
	m_checkAntialiasValue = true;
	m_excludeControlValue = true;
	
	UpdateData (false);

	m_unicodeFont = new CFont      ();
	m_unicodeFont->CreatePointFont (120, _T("Lucida Sans Unicode"));
//	m_editPreview.SetFont        (m_unicodeFont);

	m_editPadLeftValue = m_editPadTopValue = _T("0");
	m_editPadRightValue = m_editPadBottomValue = _T("2");
	
	m_buttonImageList = new CImageList ();
	m_buttonImageList->Create (IDB_BITMAP_ARROWS, 16, 1, RGB (255,255,255));

	m_buttonNodeUp.SetIcon       (m_buttonImageList->ExtractIcon (IMAGE_BUTTON_UP));
	m_buttonNodeDown.SetIcon     (m_buttonImageList->ExtractIcon (IMAGE_BUTTON_DOWN));
	m_buttonNodeUpFull.SetIcon   (m_buttonImageList->ExtractIcon (IMAGE_BUTTON_UP_FULL));
	m_buttonNodeDownFull.SetIcon (m_buttonImageList->ExtractIcon (IMAGE_BUTTON_DOWN_FULL));

	m_nodeImageList = new CImageList ();
	m_nodeImageList->Create (IDB_BITMAP_TREE, 16, 1, RGB (255,255,255));
	m_treeUnicodeSubsets.SetImageList (m_nodeImageList, TVSIL_NORMAL);

	OnButtonReloadBlocks ();

	m_comboFileFormat.SetCurSel (0);

	setupFontControls ();

	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CUIFontBuilderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CUIFontBuilderDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CUIFontBuilderDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------

void CUIFontBuilderDlg::OnSelectFontButton() 
{

	CFontDialog fontDialog (&m_selectedLogFont);

	if (fontDialog.DoModal () == IDCANCEL)
		return;

	fontDialog.GetCurrentFont (&m_selectedLogFont);

	m_selectedFontSizePointTenths = fontDialog.GetSize ();

	setupFontControls ();	
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------

void CUIFontBuilderDlg::OnQuit() 
{
	const long len = _MAX_PATH - 1;
	TCHAR buf[_MAX_PATH];

	_snwprintf (buf, len, _T("%d"), m_selectedFontSizePointTenths);

	if (RegSetValue (HKEY_CURRENT_USER,CUIFontBuilderApp::ms_lastFontSizeKey, REG_SZ, buf, wcslen (buf)) != ERROR_SUCCESS)
	{
		MessageBox (_T("Error saving last font size to registry."));
	}

	_snwprintf (buf, len, _T("%s"), (LPCTSTR)m_selectedLogFont.lfFaceName);

	if (RegSetValue (HKEY_CURRENT_USER,CUIFontBuilderApp::ms_lastFontFaceKey, REG_SZ, buf, wcslen (buf)) != ERROR_SUCCESS)
	{
		MessageBox (_T("Error saving last font face to registry."));
	}

	_snwprintf (buf, len, _T("%s"), (LPCTSTR)m_editStyleFileValue);

	if (RegSetValue (HKEY_CURRENT_USER,CUIFontBuilderApp::ms_lastStyleFileKey, REG_SZ, buf, wcslen (buf)) != ERROR_SUCCESS)
	{
		MessageBox (_T("Error saving last style output dir to registry."));
	}

	_snwprintf (buf, len, _T("%s"), (LPCTSTR)m_editOutputDirImageValue);

	if (RegSetValue (HKEY_CURRENT_USER,CUIFontBuilderApp::ms_lastImageDirKey, REG_SZ, buf, wcslen (buf)) != ERROR_SUCCESS)
	{
		MessageBox (_T("Error saving last image output dir to registry."));
	}

	CDialog::OnOK ();
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------

void CUIFontBuilderDlg::OnButtonCollapseAll() 
{
	HTREEITEM item = m_treeUnicodeSubsets.GetRootItem ();

	while (item)
	{
		m_treeUnicodeSubsets.Expand ( item, TVE_COLLAPSE);	

		item = m_treeUnicodeSubsets.GetNextSiblingItem (item);
	}
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------

void CUIFontBuilderDlg::OnButtonExpandAll() 
{
	HTREEITEM item = m_treeUnicodeSubsets.GetRootItem ();

	while (item)
	{
		m_treeUnicodeSubsets.Expand ( item, TVE_EXPAND);	

		item = m_treeUnicodeSubsets.GetNextSiblingItem (item);
	}
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------

void CUIFontBuilderDlg::OnBeginlabeleditTreeUnicodeSubsets(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	TVITEM item = pTVDispInfo->item; 
	
	// don't edit the root entries
	if (m_treeUnicodeSubsets.ItemHasChildren (item.hItem))
	{
		*pResult = 1;
		return;
	}	

	// don't edit the default entry

	int img;
	int selimg;

	m_treeUnicodeSubsets.GetItemImage (item.hItem, img, selimg);

	if (img == IMAGE_INCLUDE_DEFAULT)
	{
		*pResult = 1;
		return;
	}
	
	*pResult = 0;
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------

void CUIFontBuilderDlg::OnEndlabeleditTreeUnicodeSubsets(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	TVITEM item = pTVDispInfo->item; 

	LPCTSTR str = item.pszText;

	m_treeUnicodeSubsets.SetItemText (item.hItem, str);

	CString text = m_treeUnicodeSubsets.GetItemText (item.hItem);

	text.IsEmpty ();

	LPCTSTR str2 = (LPCTSTR)text;

	str2;
	
	*pResult = 0;
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------

void   CUIFontBuilderDlg::setupFontControls ()
{
	
	CFont * newFont = new CFont ();
	newFont->CreateFontIndirect ( &m_selectedLogFont);
	m_textFontSample.SetFont (newFont);
	
	delete m_selectedFont;
	m_selectedFont = newFont;
	
	m_textFontSampleText = m_selectedLogFont.lfFaceName;
	TCHAR buf [1024];
	_snwprintf (buf, 1023, _T("%2.1f"), static_cast<float>(m_selectedFontSizePointTenths) / 10);
	m_textFontSizeText = buf;
	
	m_editTextPixelsValue = convertPointTenthsToPixels (m_selectedFontSizePointTenths);

	const bool isBold = m_selectedLogFont.lfWeight >= FW_SEMIBOLD;

	_snwprintf (buf, 1023, _T("%s_%s%d"), m_textFontSampleText, isBold ? _T("bold_") : _T(""), m_editTextPixelsValue);

	const size_t len = wcslen (buf);

	for (size_t i = 0; i < len; ++i)
	{
		if (buf[i] == ' ')
			buf[i] = '_';
		else
			buf[i] = towlower (buf[i]);
	}


	m_editImageNameText = buf;
	m_editStyleNameText = buf;

	wcscat (buf, _T(".inc"));

	Unicode::String oldstr ((LPCTSTR)m_editStyleFileValue);

	const size_t last_sep = oldstr.rfind ('\\');

	if (last_sep == Unicode::String::npos)
		m_editStyleFileValue = buf;
	else
	{
		oldstr.erase (last_sep + 1);
		oldstr.append (buf);
		m_editStyleFileValue = oldstr.c_str ();
	}

	UpdateData (false);
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------

void CUIFontBuilderDlg::createNewRange (bool inclusive)
{
	HTREEITEM htitem = m_treeUnicodeSubsets.GetSelectedItem ();

	if (htitem == 0)
	{
		MessageBox (_T("No Unicode Block selected!\n"));
		return;
	}

	HTREEITEM parent = m_treeUnicodeSubsets.GetParentItem (htitem);

	HTREEITEM insertAfter = 0;

	// this is a root node, insert the new node at the end of its children
	if (parent != 0)
	{
		if ((insertAfter = m_treeUnicodeSubsets.GetPrevSiblingItem (htitem)) == 0)
			insertAfter = TVI_FIRST;
	}
	else
	{
		parent = htitem;
	}

	TCHAR buf [1024];
	static int ls_num = 0;

	_snwprintf (buf, 1023, _T("%d"), ++ls_num);

	const int img_code = inclusive ? IMAGE_INCLUDE : IMAGE_EXCLUDE;

	HTREEITEM subitem = m_treeUnicodeSubsets.InsertItem ( buf, img_code, img_code, parent, insertAfter);
	
	long appdata = 0x0;
	
	m_treeUnicodeSubsets.SetItemData (subitem, appdata);
	
	m_treeUnicodeSubsets.SetCheck (subitem, true);

	m_treeUnicodeSubsets.Expand ( parent, TVE_EXPAND);	
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------

void CUIFontBuilderDlg::OnButtonNewInclusion() 
{
	createNewRange (true);
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------

void CUIFontBuilderDlg::OnButtonNewExclusion() 
{
	createNewRange (false);

}

//-----------------------------------------------------------------
//-----------------------------------------------------------------

void CUIFontBuilderDlg::OnButtonDeleteRange() 
{
	HTREEITEM htitem = getRangeSelection (true);
	
	if (htitem == 0)
		return;

	int img;
	int selimg;

	m_treeUnicodeSubsets.GetItemImage (htitem, img, selimg);

	if (img == IMAGE_INCLUDE_DEFAULT)
	{
		MessageBox (_T("You can't delete the default inclusion, only disable it."));
		return;
	}

	HTREEITEM parent;
	parent = m_treeUnicodeSubsets.GetParentItem (htitem);

	if (parent == 0)
	{
		MessageBox (_T("An error occured.  The selected item has no parent.\n"));
		return;
	}

	m_treeUnicodeSubsets.Expand ( parent, TVE_EXPAND);
	m_treeUnicodeSubsets.DeleteItem (htitem);
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------

void CUIFontBuilderDlg::OnButtonEditRange() 
{
	// TODO: Add your control notification handler code here
	
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------

void CUIFontBuilderDlg::OnButtonReloadBlocks() 
{
	m_treeUnicodeSubsets.DeleteAllItems ();

	const Unicode::Blocks::Mapping::IdMap_t &  blockMap = Unicode::Blocks::Mapping::getDefaultMapping ().getIdMap ();

	for (Unicode::Blocks::Mapping::IdMap_t::const_iterator iter = blockMap.begin (); iter != blockMap.end (); ++iter)
	{
		const Unicode::Blocks::Data & data = (*iter).second;

		// data representing the range of this block
		const long appdata = (data.m_start << 16) | data.m_end;
		TCHAR buf [1024];
		_snwprintf (buf, 1023, _T("[%04x-%04x] %s"), data.m_start, data.m_end, Unicode::narrowToWide (data.m_name).c_str ());
		HTREEITEM item = m_treeUnicodeSubsets.InsertItem ( buf, IMAGE_NONE, IMAGE_NONE);
		m_treeUnicodeSubsets.SetItemData (item, appdata);


		_snwprintf (buf, 1023, _T("%04x-%04x"), data.m_start, data.m_end);
		HTREEITEM subitem = m_treeUnicodeSubsets.InsertItem ( buf, IMAGE_INCLUDE_DEFAULT, IMAGE_INCLUDE_DEFAULT, item);

		m_treeUnicodeSubsets.SetItemData (subitem, appdata);

		m_treeUnicodeSubsets.SetCheck (subitem, true);
	}
}

//-----------------------------------------------------------------

CUIFontBuilderDlg::IdSet_t & CUIFontBuilderDlg::getIdSet (IdSet_t & idSet)
{
	// update the data members from the controls
	UpdateData (true);

	// DO ALL CHECKED

	HTREEITEM htitem = m_treeUnicodeSubsets.GetRootItem ();

	if (htitem == 0)
	{
		return idSet;
	}

	typedef std::vector<Unicode::Blocks::RawData *> RawDataVec_t;
	RawDataVec_t rawDataVec;

	size_t index = 0;

	Unicode::CharDataMap cdmap;

	while (htitem)
	{
		// is checked
		if (m_treeUnicodeSubsets.GetCheck (htitem))
		{

			Unicode::Blocks::Data data (Unicode::Blocks::Mapping::ms_defaultBlockData [index]);
			cdmap.addBlock (data.m_id);

			HTREEITEM subItem = m_treeUnicodeSubsets.GetChildItem (htitem);

			//-- if the first item is the default inclusion, we can skip it
			{
				int im;
				int selim;
					
				m_treeUnicodeSubsets.GetItemImage (subItem, im, selim);

				if (im == IMAGE_INCLUDE_DEFAULT)
					subItem = m_treeUnicodeSubsets.GetNextSiblingItem (subItem);
			}

			while (subItem)
			{

				if (m_treeUnicodeSubsets.GetCheck (subItem))
				{
					int im;
					int selim;
					
					m_treeUnicodeSubsets.GetItemImage (subItem, im, selim);
					
					CString text = m_treeUnicodeSubsets.GetItemText (subItem);
					
					data.addRangeGroup (im != IMAGE_EXCLUDE, Unicode::wideToNarrow ((LPCTSTR)text));
				}
				
				subItem = m_treeUnicodeSubsets.GetNextSiblingItem (subItem);
			}

			data.generateFilteredIdSet (idSet);
		}

		htitem = m_treeUnicodeSubsets.GetNextSiblingItem (htitem);

		++index;
	}

	// now use the chardata map to remove control characters if needed

	if (m_excludeControlValue)
	{
		cdmap.generateMap (Unicode::Blocks::Mapping::getDefaultMapping (), m_unicodeFilename.c_str ());

		for (std::set<unicode_char_t>::iterator iter = idSet.begin (); iter != idSet.end ();)
		{
			const Unicode::CharData * cdata = cdmap.findCharData (*iter);

			if (cdata &&
				(cdata->m_category == Unicode::CharData::Cc ||
				cdata->m_category == Unicode::CharData::Co))
			{
				idSet.erase (iter++);
				continue;
			}

			++iter;
		}
	}

	return idSet;

}
//-----------------------------------------------------------------
//-----------------------------------------------------------------

void CUIFontBuilderDlg::OnButtonPreviewRange() 
{

	std::set<unicode_char_t> idSet;

	
	if (getIdSet (idSet).size () == 0)
	{
		MessageBox (_T("No Items!\n"));
		return;
	}

	CPreviewDialog cpd (this, idSet, m_selectedFont, m_editTextPixelsValue);

	cpd.DoModal ();

	using CPreviewDialog::ExclusionVector_t;

	const ExclusionVector_t & vec = cpd.getExclusionVector ();

	if (vec.size ())
	{

		TCHAR hexbuf[16];

		size_t blockIndex = 0;
		
		HTREEITEM item = m_treeUnicodeSubsets.GetRootItem ();

		// TODO: exclude
		for (ExclusionVector_t::const_iterator iter = vec.begin (); iter != vec.end (); ++iter)
		{
			Unicode::unicode_char_t cur_char = *iter;
			const Unicode::Blocks::RawData * rdata = &Unicode::Blocks::Mapping::ms_defaultBlockData [blockIndex];

			while (cur_char < rdata->m_start || cur_char > rdata->m_end)
			{
				rdata = &Unicode::Blocks::Mapping::ms_defaultBlockData [++blockIndex];

				if (rdata->m_id == Unicode::Blocks::End_Block_Ids)
				{
					rdata = 0;
					break;
				}

				if ((item = m_treeUnicodeSubsets.GetNextSiblingItem (item)) == 0)
				{
					MessageBox ( _T("List and data to not match."));
					rdata = 0;
					break;
				}
			}

			if (rdata == 0)
			{
				MessageBox (_T ("Error doing exclusion."));
				break;
			}

			_snwprintf (hexbuf, 7, _T("%04x"), cur_char);

			HTREEITEM subitem = m_treeUnicodeSubsets.InsertItem ( hexbuf, IMAGE_EXCLUDE, IMAGE_EXCLUDE, item);
			m_treeUnicodeSubsets.SetCheck (subitem, true);
			m_treeUnicodeSubsets.Expand ( item, TVE_EXPAND);

		}

	}


	// show preview dialog
//	m_editPreviewValue = buf;

	UpdateData (false);
}

HTREEITEM CUIFontBuilderDlg::getRangeSelection (bool showmsg)
{
	HTREEITEM htitem = m_treeUnicodeSubsets.GetSelectedItem ();

	if (htitem == 0 || m_treeUnicodeSubsets.GetParentItem (htitem) == 0)
	{
		if (showmsg)
			MessageBox (_T("No Unicode Range selected!\n"));
		return 0;
	}

	return htitem;
}
//-----------------------------------------------------------------
//-----------------------------------------------------------------

void CUIFontBuilderDlg::moveItem (HTREEITEM htitem, HTREEITEM insertAfter)
{
	HTREEITEM parent = m_treeUnicodeSubsets.GetParentItem (htitem);

	if (parent == 0)
	{
		MessageBox (_T("Specified Unicode Range has no parent!\n"));
		return;
	}

	m_treeUnicodeSubsets.Expand ( parent, TVE_EXPAND);

	CString oldText = m_treeUnicodeSubsets.GetItemText (htitem);
	int     oldImage;
	int     oldSelectedImage;
	int    checked = m_treeUnicodeSubsets.GetCheck (htitem);

	m_treeUnicodeSubsets.GetItemImage (htitem, oldImage, oldSelectedImage);

	m_treeUnicodeSubsets.DeleteItem (htitem);
	HTREEITEM newItem = m_treeUnicodeSubsets.InsertItem (oldText, oldImage, oldSelectedImage, parent, insertAfter);
	m_treeUnicodeSubsets.SetCheck   (newItem, checked);
	m_treeUnicodeSubsets.SelectItem (newItem);

}
//-----------------------------------------------------------------
//-----------------------------------------------------------------

void CUIFontBuilderDlg::OnButtonNodeUp() 
{
	HTREEITEM htitem = getRangeSelection (true);
	
	if (htitem == 0)
		return;

	HTREEITEM insertAfter = 0;

	// item is already first.
	if ((insertAfter = m_treeUnicodeSubsets.GetPrevSiblingItem (htitem)) == 0)
		return;

	if ((insertAfter = m_treeUnicodeSubsets.GetPrevSiblingItem (insertAfter)) == 0)
		insertAfter = TVI_FIRST;

	moveItem (htitem, insertAfter);
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------

void CUIFontBuilderDlg::OnButtonNodeDown() 
{
	HTREEITEM htitem = getRangeSelection (true);
	
	if (htitem == 0)
		return;

	HTREEITEM insertAfter = 0;

	// item is already last
	if ((insertAfter = m_treeUnicodeSubsets.GetNextSiblingItem (htitem)) == 0)
		return;

	moveItem (htitem, insertAfter);
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------

void CUIFontBuilderDlg::OnButtonNodeDownFull() 
{
	HTREEITEM htitem = getRangeSelection (true);
	
	if (htitem == 0)
		return;

	// item is already last
	if (m_treeUnicodeSubsets.GetNextSiblingItem (htitem) == 0)
		return;

	moveItem (htitem, TVI_LAST);
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------

void CUIFontBuilderDlg::OnButtonNodeUpFull() 
{

	HTREEITEM htitem = getRangeSelection (true);
	
	if (htitem == 0)
		return;

	// item is already last
	if (m_treeUnicodeSubsets.GetPrevSiblingItem (htitem) == 0)
		return;

	moveItem (htitem, TVI_FIRST);	
}
//-----------------------------------------------------------------
//-----------------------------------------------------------------

void CUIFontBuilderDlg::OnBuildFont() 
{

	UpdateData (true);

	std::set<unicode_char_t> idSet;

	if (getIdSet (idSet).size () == 0)
	{
		MessageBox (_T("No Items!\n"));
		return;
	}

	Unicode::NarrowString styleStr     = Unicode::wideToNarrow ((LPCTSTR)m_editStyleNameText);
	Unicode::NarrowString styleFileStr = Unicode::wideToNarrow ((LPCTSTR)m_editStyleFileValue);
	Unicode::NarrowString imageStr     = Unicode::wideToNarrow ((LPCTSTR)m_editImageNameText);
	Unicode::NarrowString imageDirStr  = Unicode::wideToNarrow ((LPCTSTR)m_editOutputDirImageValue);

	int tilesize = m_comboTileSize.GetCurSel ();
	tilesize = 1 << (tilesize + 4);

	FontExporter::ExporterInfo info = 
	{
		m_selectedFont,
			m_editTextPixelsValue,
			tilesize,
			tilesize,
			m_checkOutlinesValue != 0,
			m_checkAntialiasValue != 0,
			styleStr.c_str (),
			styleFileStr.c_str (),
			imageStr.c_str (),
			imageDirStr.c_str (),
		{
			_wtoi (m_editPadLeftValue),
			_wtoi (m_editPadRightValue),
			_wtoi (m_editPadTopValue),
			_wtoi (m_editPadBottomValue)
		}
	};


	int filetype = m_comboFileFormat.GetCurSel ();

	FontImageExporter * imageExporter = 0;

	// dds
	if (filetype == 0)
	{
		imageExporter = new FontImageExporterDDS ();
	}

	// bmp
	else
	{
		imageExporter = new FontImageExporterBMP ();
	}

	FontExporterPSUI exporter (*imageExporter, info);

	// character zero is always required by the UI font rendering system
	idSet.insert (0);

	HCURSOR oldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
	bool retval = exporter.export (idSet);
	SetCursor(oldCursor);

	if (retval == false)
	{
		MessageBox (_T("Export Failed!\n"));
	}
	else
	{
		MessageBox (_T("Export complete.\n"));
	}
}

//-----------------------------------------------------------------

void CUIFontBuilderDlg::OnButtonChangeStyleFile() 
{
	Unicode::String fileStr = (LPCTSTR) m_editStyleFileValue;

	const size_t slashpos = fileStr.rfind ( '\\');

	if (slashpos != Unicode::String::npos)
	{
		fileStr = fileStr.substr (slashpos + 1);
	}
	
	CFileDialog fileDialog (
		true,
		0,
		fileStr.c_str (),
		OFN_PATHMUSTEXIST );
	
	int retval = fileDialog.DoModal ();
	
	if (retval == IDOK)
	{
		m_editStyleFileValue = fileDialog.GetPathName ();
	}

	UpdateData (false);
}
//-----------------------------------------------------------------

void CUIFontBuilderDlg::OnButtonChangeOutputdirImage() 
{
	
}
//-----------------------------------------------------------------

void CUIFontBuilderDlg::OnChangeEditTextPixels() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here

	UpdateData (true);
	m_selectedFontSizePointTenths = convertPixelsToPointTenths (m_editTextPixelsValue);
	m_selectedLogFont.lfHeight    = m_editTextPixelsValue;
	setupFontControls ();
	
}
