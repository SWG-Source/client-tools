// ======================================================================
//
// DialogStringFileDiff.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstStringFileTool.h"
#include "DialogStringFileDiff.h"

#include "DialogAbout.h"
#include "DialogDiff.h"
#include "StringFileTool.h"
#include "StringTable.h"

#include "UnicodeUtils.h"

#include <map>

// ======================================================================

class DialogStringFileDiff::Node
{
public:

	Node() :
		m_string1(),
		m_string2(),
		m_iconType(StringFileTool::IT_equal)
	{
	}

	void diff()
	{
		if (m_string1 == m_string2)
			m_iconType = StringFileTool::IT_equal;
		else
			if (m_string1 != m_string2 && m_string2.empty())
				m_iconType = StringFileTool::IT_left;
			else
				if (m_string1 != m_string2 && m_string1.empty())
					m_iconType = StringFileTool::IT_right;
				else
					m_iconType = StringFileTool::IT_notEqual;
	}

	StringFileTool::IconType getIconType() const
	{
		return m_iconType;
	}

public:

	Unicode::String m_string1;
	Unicode::String m_string2;

private:

	StringFileTool::IconType m_iconType;
};

// ======================================================================

DialogStringFileDiff::DialogStringFileDiff(StringTable const & stringTable1, StringTable const & stringTable2, CWnd* pParent /*=NULL*/) : 
	CDialog(DialogStringFileDiff::IDD, pParent),
	m_hIcon(0),
	m_imageList(),
	m_stringTable1(stringTable1),
	m_stringTable2(stringTable2)
{
	//{{AFX_DATA_INIT(DialogStringFileDiff)
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

// ----------------------------------------------------------------------

void DialogStringFileDiff::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogStringFileDiff)
	DDX_Control(pDX, IDOK, m_okButton);
	DDX_Control(pDX, IDC_COMBO_SHOW, m_comboBox);
	DDX_Control(pDX, IDC_LISTCTRL, m_listCtrl);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogStringFileDiff, CDialog)
	//{{AFX_MSG_MAP(DialogStringFileDiff)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE(IDC_COMBO_SHOW, OnSelchangeComboShow)
	ON_WM_SIZE()
	ON_NOTIFY(NM_DBLCLK, IDC_LISTCTRL, OnDblclkListctrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL DialogStringFileDiff::OnInitDialog()
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

	//-- Add columns to list ctrl
	IGNORE_RETURN(m_imageList.Create(IDB_BITMAP_ICONS, 16, 5, RGB(255,255,255)));
	IGNORE_RETURN(GetListCtrl().SetImageList(&m_imageList, LVSIL_SMALL));
	IGNORE_RETURN(GetListCtrl().InsertColumn(0, Unicode::narrowToWide("StringId").c_str(), LVCFMT_LEFT, 128, 0));
	IGNORE_RETURN(GetListCtrl().InsertColumn(1, Unicode::narrowToWide(m_stringTable1.getFileName()).c_str(), LVCFMT_LEFT, 256, 0));
	IGNORE_RETURN(GetListCtrl().InsertColumn(2, Unicode::narrowToWide(m_stringTable2.getFileName()).c_str(), LVCFMT_LEFT, 256, 0));

	//-- Add sort types to combo box
	m_comboBox.ResetContent ();
	for (int i = 0; i < StringFileTool::ST_COUNT; ++i)
		m_comboBox.AddString(Unicode::narrowToWide(StringFileTool::getShowNames()[i]).c_str());
	m_comboBox.SetCurSel(1);

	//-- Populate list ctrl
	populate();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// ----------------------------------------------------------------------

void DialogStringFileDiff::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		DialogAbout dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// ----------------------------------------------------------------------

void DialogStringFileDiff::OnPaint() 
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

// ----------------------------------------------------------------------

HCURSOR DialogStringFileDiff::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// ----------------------------------------------------------------------

void DialogStringFileDiff::OnSelchangeComboShow() 
{
	UpdateData(true);

	populate();
}

// ----------------------------------------------------------------------

void DialogStringFileDiff::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	if (IsWindow(m_listCtrl.m_hWnd))
		m_listCtrl.MoveWindow(10, 10, cx - 20, cy - 50);

	if (IsWindow(m_comboBox.m_hWnd))
	{
		CRect rect;
		m_comboBox.GetWindowRect(rect);
		m_comboBox.MoveWindow(10, cy - 30, rect.Width(), rect.Height());
	}

	if (IsWindow(m_okButton.m_hWnd))
	{
		CRect rect;
		m_okButton.GetWindowRect(rect);
		m_okButton.MoveWindow(cx - rect.Width() - 10, cy - 30, rect.Width(), rect.Height());
	}
}

// ----------------------------------------------------------------------

CListCtrl & DialogStringFileDiff::GetListCtrl()
{
	return m_listCtrl;
}

// ----------------------------------------------------------------------

void DialogStringFileDiff::populate()
{
	GetListCtrl().DeleteAllItems();

	typedef std::map<std::string, Node *> NodeMap;
	NodeMap nodeMap;

	//-- Insert stringTable1
	{
		for (int i = 0; i < m_stringTable1.getNumberOfStrings (); ++i)
		{
			Node * const node = new Node;
			node->m_string1 = m_stringTable1.getValue(i);
			nodeMap.insert(std::make_pair(m_stringTable1.getKey(i), node));
		}
	}

	//-- Insert stringTable2
	{
		for (int i = 0; i < m_stringTable2.getNumberOfStrings(); ++i)
		{
			std::string const & key = m_stringTable2.getKey(i);
			NodeMap::iterator iter = nodeMap.find(key);
			if (iter == nodeMap.end ())
			{
				Node * const node = new Node;
				node->m_string2 = m_stringTable2.getValue(i);
				nodeMap.insert(std::make_pair(key, node));
			}
			else
				iter->second->m_string2 = m_stringTable2.getValue(i);
		}
	}

	//-- Diff and show according to show type
	{
		for (NodeMap::const_iterator iter = nodeMap.begin (); iter != nodeMap.end (); ++iter)
		{
			iter->second->diff();

			bool add = false;
			switch(m_comboBox.GetCurSel())
			{
			case StringFileTool::ST_all:
				add = true;
				break;

			case StringFileTool::ST_differences:
				add = iter->second->getIconType() != StringFileTool::IT_equal;
				break;

			case StringFileTool::ST_notEqual:
				add = iter->second->getIconType() == StringFileTool::IT_notEqual;
				break;

			case StringFileTool::ST_left:
				add = iter->second->getIconType() == StringFileTool::IT_left;
				break;

			case StringFileTool::ST_right:
				add = iter->second->getIconType() == StringFileTool::IT_right;
				break;
			}
			
			if (add)
			{
				int const item = GetListCtrl().InsertItem(GetListCtrl().GetItemCount(), Unicode::narrowToWide(iter->first).c_str(), iter->second->getIconType()); 

				GetListCtrl().SetItemText(item, 1, iter->second->m_string1.c_str());
				GetListCtrl().SetItemText(item, 2, iter->second->m_string2.c_str());
			}

			delete iter->second;
		}
	}
}

// ----------------------------------------------------------------------

void DialogStringFileDiff::OnDblclkListctrl(NMHDR * /*pNMHDR*/, LRESULT * const pResult) 
{
	//-- Double-clicking any row shows the string in greater detail
	POSITION position = GetListCtrl().GetFirstSelectedItemPosition();

	if (position)
	{
		int const item = GetListCtrl().GetNextSelectedItem(position);
		CString const stringId = GetListCtrl().GetItemText(item, 0);
		CString const string1 = GetListCtrl().GetItemText(item, 1);
		CString const string2 = GetListCtrl().GetItemText(item, 2);
		DialogDiff dlg(stringId, m_stringTable1.getFileName().c_str(), string1, m_stringTable2.getFileName().c_str(), string2);
		dlg.DoModal();
	}

	*pResult = 0;
}

// ======================================================================

