// UnicodeTesterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UnicodeTester.h"
#include "UnicodeTesterDlg.h"

#include <cstdio>

#include "Unicode.h"
#include "UnicodeCharData.h"
#include "UnicodeBlocks.h"
#include "UnicodeCharDataMap.h"

using namespace Unicode;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
static const TCHAR * s_dbFileKey = _T ("Software\\Sony Online Entertainment\\UnicodeTester\\db_filename");

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
// CUnicodeTesterDlg dialog

CUnicodeTesterDlg::CUnicodeTesterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUnicodeTesterDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUnicodeTesterDlg)
	m_curFilename = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUnicodeTesterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUnicodeTesterDlg)
	DDX_Control(pDX, IDC_LIST_CHARDATA, m_listCharData);
	DDX_Control(pDX, IDC_LIST_BLOCKS, m_listBlocks);
	DDX_Text(pDX, IDC_STATIC_FILENAME, m_curFilename);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CUnicodeTesterDlg, CDialog)
	//{{AFX_MSG_MAP(CUnicodeTesterDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_LOAD, OnButtonLoad)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_FILE, OnButtonChangeFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnicodeTesterDlg message handlers

BOOL CUnicodeTesterDlg::OnInitDialog()
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
	
	// TODO: Add extra initialization here
	
	CFont * font = new CFont ();

	m_listCharData.InsertColumn (0, _T("Code"),          LVCFMT_LEFT, 80);
	m_listCharData.InsertColumn (1, _T("Sample"),        LVCFMT_LEFT, 80);
	m_listCharData.InsertColumn (2, _T("Category"),      LVCFMT_LEFT, 80);
	m_listCharData.InsertColumn (3, _T("case"),          LVCFMT_LEFT, 80);
	m_listCharData.InsertColumn (4, _T("Reverse"),       LVCFMT_LEFT, 80);
	m_listCharData.InsertColumn (5, _T("ReverseSample"), LVCFMT_LEFT, 80);

	font->CreatePointFont (100, _T ("Code2000"));
	m_listCharData.SetFont (font);

	CFont * font2 = new CFont ();
	font2->CreatePointFont (80, _T ("Lucida Console"));
	m_listBlocks.SetFont (font2);

	m_listBlocks.InsertColumn (0, _T("BlockName"), LVCFMT_LEFT, 300);
	m_listBlocks.InsertColumn (1, _T("Start"),     LVCFMT_LEFT, 100);
	m_listBlocks.InsertColumn (2, _T("End"),       LVCFMT_LEFT, 100);

	const Blocks::Mapping::IdMap_t & idmap = Blocks::Mapping::getDefaultMapping ().getIdMap ();

	Unicode::String str;

	size_t itemNum = 0;
	for (Blocks::Mapping::IdMap_t::const_iterator iter = idmap.begin (); iter != idmap.end (); ++iter, ++itemNum)
	{
		m_listBlocks.InsertItem (itemNum, Unicode::narrowToWide ( (*iter).second.m_name, str).c_str ());

		TCHAR buf [8];

		_snwprintf (buf, 7, _T ("%04x"), (*iter).second.m_start);
		m_listBlocks.SetItemText (itemNum, 1, buf);
		_snwprintf (buf, 7, _T ("%04x"), (*iter).second.m_end);
		m_listBlocks.SetItemText (itemNum, 2, buf);
	}

	long len = _MAX_PATH + 1;
	TCHAR buf[_MAX_PATH];

	if (RegQueryValue (HKEY_CURRENT_USER, s_dbFileKey, buf, &len) == ERROR_SUCCESS)
	{
		m_curFilename = buf;
		UpdateData (false);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CUnicodeTesterDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CUnicodeTesterDlg::OnPaint() 
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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CUnicodeTesterDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CUnicodeTesterDlg::OnButtonLoad() 
{

	CharDataMap::getDefaultMap ().clearBlocks ();
	m_listCharData.DeleteAllItems ();

	long numblocks = m_listBlocks.GetSelectedCount ();

	POSITION pos = m_listBlocks.GetFirstSelectedItemPosition ();

	const Blocks::Mapping::IdMap_t & idmap = Blocks::Mapping::getDefaultMapping ().getIdMap ();

	if (pos)
	{
		while (pos)
		{
			int nItem = m_listBlocks.GetNextSelectedItem(pos);
			CharDataMap::getDefaultMap ().addBlock (Blocks::Mapping::ms_defaultBlockData [nItem].m_id);
		}
	}
	else
	{
		MessageBox ( _T ("No Blocks Selected, genius!\n"));
		return;
	}

	if (m_curFilename.IsEmpty ())
	{
		MessageBox ( _T ("Choose a db file first, smartguy.\n"));
		return;
	}

	CharDataMap::ErrorCode retval = CharDataMap::getDefaultMap ().generateMap (Blocks::Mapping::getDefaultMapping (), Unicode::wideToNarrow (LPCTSTR(m_curFilename)).c_str ());

	if (retval != CharDataMap::ERR_SUCCESS)
	{
		TCHAR buf [1024];

		_snwprintf (buf, 1023, _T ("Unable to generate map.\nError code: %d\n"), static_cast<int> (retval));
		MessageBox (buf);
	}
	else
	{
	
		typedef std::map <CharData::Category, Unicode::String> CategoryMap_t;
		//-- create the category map for use by the inline population function
		CategoryMap_t        catmap;
		
		{
			size_t i = 0;
			for (; CharData::ms_categoryNames [i].m_category != CharData::Cn; ++i)
			{
				catmap [CharData::ms_categoryNames [i].m_category] = Unicode::narrowToWide (CharData::ms_categoryNames [i].m_str);
			}
		}

		const CharDataMap::Map_t & cmap = CharDataMap::getDefaultMap ().getMap ();

		size_t itemNum = 0;

		for (CharDataMap::Map_t::const_iterator iter = cmap.begin (); iter != cmap.end (); ++iter, ++itemNum)
		{

			const CharData * const cdata = (*iter).second;

			TCHAR buf [1024];

			_snwprintf (buf, 1023, _T ("%04x"), cdata->m_code);
			m_listCharData.InsertItem (itemNum, buf);

			_snwprintf (buf, 1023, _T ("%c"), cdata->m_code);
			m_listCharData.SetItemText (itemNum, 1, buf);

			_snwprintf (buf, 1023, _T ("%2s (%02d)"), catmap [cdata->m_category].c_str (), static_cast<int>(cdata->m_category));
			m_listCharData.SetItemText (itemNum, 2, buf);

			_snwprintf (buf, 1023, _T ("%s"), cdata->isLowerCase () ? _T ("LOW") : cdata->isUpperCase () ? _T ("UPP") : _T (""));
			m_listCharData.SetItemText (itemNum, 3, buf);

			if (cdata->m_reverseCase)
			{

				_snwprintf (buf, 1023, _T ("%04x"), cdata->m_reverseCase);
				m_listCharData.SetItemText (itemNum, 4, buf);

				_snwprintf (buf, 1023, _T ("%c"), cdata->m_reverseCase);
				m_listCharData.SetItemText (itemNum, 5, buf);
			}

		}

	}
}

void CUnicodeTesterDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}


void CUnicodeTesterDlg::OnButtonChangeFile() 
{

	CFileDialog fileDialog (
		true,
		_T ("txt"),
		_T ("UnicodeData.txt"),
		OFN_READONLY | OFN_PATHMUSTEXIST  |OFN_FILEMUSTEXIST);
	
	int retval = fileDialog.DoModal ();

	if (retval == IDOK)
	{
		CString pathName = fileDialog.GetPathName ();
		m_curFilename = pathName;

		UpdateData (false);

		RegSetValue (HKEY_CURRENT_USER, s_dbFileKey, REG_SZ, m_curFilename, wcslen (m_curFilename));
	}

	// TODO: Add your control notification handler code here
	
}

