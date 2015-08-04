// ======================================================================
//
// DialogStringFileMerge.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstStringFileTool.h"
#include "DialogStringFileMerge.h"

#include "DialogAbout.h"
#include "DialogMerge.h"
#include "sharedFoundation/PointerDeleter.h"
#include "StringFileTool.h"
#include "StringTable.h"
#include "Unicode.h"
#include "UnicodeUtils.h"

#include <algorithm>
#include <map>

// ======================================================================

class DialogStringFileMerge::Node
{
public:

	Node() :
		m_original(),
		m_theirs(),
		m_yours(),
		m_merged(),
		m_iconType(StringFileTool::IT_conflict)
	{
	}

	//-- Merge rules
	// original	theirs	yours	resolution
	//     -       -      -      theirs
	//     -       x      -      theirs
	//     -       -      x      yours
	//     -       x      x      theirs
	//     -       x      y      CONFLICT
	void merge()
	{
		if ((m_original == m_yours) && (m_original != m_theirs) && !m_theirs.empty())
		{
			m_merged = m_theirs;
			m_iconType = StringFileTool::IT_merged;
		}
		else
			if ((m_original == m_theirs) && (m_original != m_yours) && !m_yours.empty())
			{
				m_merged = m_yours;
				m_iconType = StringFileTool::IT_merged;
			}
			else
				if (m_theirs == m_yours)
				{
					m_merged = m_theirs;
					m_iconType = StringFileTool::IT_merged;
				}
	}

	StringFileTool::IconType getIconType() const
	{
		return m_iconType;
	}

public:

	Unicode::String m_original;
	Unicode::String m_theirs;
	Unicode::String m_yours;
	Unicode::String m_merged;

private:

	StringFileTool::IconType m_iconType;
};

// ======================================================================

DialogStringFileMerge::DialogStringFileMerge(StringTable const & originalStringTable, StringTable const & theirsStringTable, StringTable const & yoursStringTable, StringTable & mergedStringTable, CWnd* pParent /*=NULL*/) : 
	CDialog(DialogStringFileMerge::IDD, pParent),
	m_hIcon(0),
	m_imageList(),
	m_originalStringTable(originalStringTable),
	m_theirsStringTable(theirsStringTable),
	m_yoursStringTable(yoursStringTable),
	m_mergedStringTable(mergedStringTable)
{
	//{{AFX_DATA_INIT(DialogStringFileMerge)
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

// ----------------------------------------------------------------------

void DialogStringFileMerge::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogStringFileMerge)
	DDX_Control(pDX, IDCANCEL, m_cancelButton);
	DDX_Control(pDX, IDOK, m_okButton);
	DDX_Control(pDX, IDC_LISTCTRL, m_listCtrl);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogStringFileMerge, CDialog)
	//{{AFX_MSG_MAP(DialogStringFileMerge)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_NOTIFY(NM_DBLCLK, IDC_LISTCTRL, OnDblclkListctrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL DialogStringFileMerge::OnInitDialog()
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
	IGNORE_RETURN(GetListCtrl().InsertColumn(0, Unicode::narrowToWide("StringId").c_str(), LVCFMT_LEFT, 256, 0));

	//-- Populate list ctrl
	populate();

	m_okButton.EnableWindow(GetListCtrl().GetItemCount() == 0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// ----------------------------------------------------------------------

void DialogStringFileMerge::OnSysCommand(UINT nID, LPARAM lParam)
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

void DialogStringFileMerge::OnPaint() 
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

HCURSOR DialogStringFileMerge::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// ----------------------------------------------------------------------

void DialogStringFileMerge::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	if (IsWindow(m_listCtrl.m_hWnd))
		m_listCtrl.MoveWindow(10, 10, cx - 20, cy - 50);

	if (IsWindow(m_okButton.m_hWnd))
	{
		CRect cancelRect;
		m_cancelButton.GetWindowRect(cancelRect);

		CRect okRect;
		m_okButton.GetWindowRect(okRect);
		m_okButton.MoveWindow(cx - cancelRect.Width() - okRect.Width() - 20, cy - 30, okRect.Width(), okRect.Height());
	}

	if (IsWindow(m_cancelButton.m_hWnd))
	{
		CRect rect;
		m_cancelButton.GetWindowRect(rect);
		m_cancelButton.MoveWindow(cx - rect.Width() - 10, cy - 30, rect.Width(), rect.Height());
	}
}

// ----------------------------------------------------------------------

CListCtrl & DialogStringFileMerge::GetListCtrl()
{
	return m_listCtrl;
}

// ----------------------------------------------------------------------

void DialogStringFileMerge::populate()
{
	GetListCtrl().DeleteAllItems();

	typedef std::map<std::string, Node *> NodeMap;
	NodeMap nodeMap;

	//-- Insert original string table
	{
		for (int i = 0; i < m_originalStringTable.getNumberOfStrings (); ++i)
		{
			Node * const node = new Node;
			node->m_original = m_originalStringTable.getValue(i);
			nodeMap.insert(std::make_pair(m_originalStringTable.getKey(i), node));
		}
	}

	//-- Insert theirs string table
	{
		for (int i = 0; i < m_theirsStringTable.getNumberOfStrings(); ++i)
		{
			Node * node = 0;
			std::string const & key = m_theirsStringTable.getKey(i);
			NodeMap::iterator iter = nodeMap.find(key);
			if (iter == nodeMap.end ())
			{
				node = new Node;
				nodeMap.insert(std::make_pair(key, node));
			}
			else
				node = iter->second;

			node->m_theirs = m_theirsStringTable.getValue(i);
		}
	}

	//-- Insert yours string table
	{
		for (int i = 0; i < m_yoursStringTable.getNumberOfStrings(); ++i)
		{
			Node * node = 0;
			std::string const & key = m_yoursStringTable.getKey(i);
			NodeMap::iterator iter = nodeMap.find(key);
			if (iter == nodeMap.end ())
			{
				node = new Node;
				nodeMap.insert(std::make_pair(key, node));
			}
			else
				node = iter->second;

			node->m_yours = m_yoursStringTable.getValue(i);
		}
	}

	//-- Merge and only fill out the list ctrl with items that need to be resolved
	{
		for (NodeMap::const_iterator iter = nodeMap.begin (); iter != nodeMap.end (); ++iter)
		{
			iter->second->merge();

			if (iter->second->getIconType() == StringFileTool::IT_merged)
				m_mergedStringTable.setString(iter->first, iter->second->m_merged);
			else
				GetListCtrl().InsertItem(GetListCtrl().GetItemCount(), Unicode::narrowToWide(iter->first).c_str(), iter->second->getIconType()); 

			delete iter->second;
		}
	}
}

// ----------------------------------------------------------------------

void DialogStringFileMerge::OnDblclkListctrl(NMHDR * /*pNMHDR*/, LRESULT * const pResult) 
{
	//-- Double-clicking any row performs the merge
	POSITION position = GetListCtrl().GetFirstSelectedItemPosition();

	if (position)
	{
		int const item = GetListCtrl().GetNextSelectedItem(position);
		CString const stringId = GetListCtrl().GetItemText(item, 0);
		Unicode::String stringIdUnicode;
		StringFileTool::convertToUnicodeString(stringId, stringIdUnicode);
		std::string stringIdStd = Unicode::wideToNarrow(stringIdUnicode);
		const char * stringIdChars = stringIdStd.c_str();

		DialogMerge dlg(stringIdUnicode.c_str(), m_originalStringTable.getValue(stringIdChars).c_str(), m_theirsStringTable.getValue(stringIdChars).c_str(), m_yoursStringTable.getValue(stringIdChars).c_str());
		if (dlg.DoModal() == IDOK)
		{
			Unicode::String merged;
			StringFileTool::convertToUnicodeString(dlg.m_merged, merged);

			m_mergedStringTable.setString(stringIdStd, merged);
			GetListCtrl().DeleteItem(item);

			m_okButton.EnableWindow(GetListCtrl().GetItemCount() == 0);
		}
	}

	*pResult = 0;
}

// ======================================================================

