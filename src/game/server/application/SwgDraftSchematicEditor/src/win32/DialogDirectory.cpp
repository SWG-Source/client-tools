// ======================================================================
//
// DialogDirectory.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgDraftSchematicEditor.h"
#include "DialogDirectory.h"

#include "Configuration.h"
#include "DraftSchematic.h"
#include "MainFrame.h"
#include "SwgDraftSchematicEditor.h"

// ======================================================================

namespace DialogDirectoryNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum DirectoryDialogIconType
	{
		DDIT_closedFolder,
		DDIT_openFolder,
		DDIT_file
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool shouldAddFile (CString const & fileName)
	{
		return (fileName.Right (4) == ".tpf") && (fileName.Find ("base_") == -1) && (fileName.Find ("_base") == -1);
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool shouldAddDirectory (CString const & fileName)
	{
		return fileName != "base";
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HTREEITEM findChildTreeItem (CTreeCtrl & treeCtrl, HTREEITEM rootItem, CString const & itemName)
	{
		HTREEITEM treeItem = treeCtrl.GetChildItem (rootItem);
		while (treeItem)
		{
			if (itemName == treeCtrl.GetItemText (treeItem))
				return treeItem;

			treeItem = treeCtrl.GetNextSiblingItem (treeItem);
		}

		return 0;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void addDirectory (char const * const directory, CTreeCtrl & treeCtrl, HTREEITEM rootItem, bool const refresh)
	{
		int const n = strlen (directory);

		char searchMask [MAX_PATH];
		char subDirectory [MAX_PATH];

		sprintf (searchMask, "%s%s*.*", directory, directory [n-1] != '/' ? "/" : "");

		CFileFind finder;
		BOOL working = finder.FindFile (searchMask);
		while (working)
		{
			working = finder.FindNextFile();

			if (!finder.IsDots ())
			{
				if ((finder.IsDirectory () && shouldAddDirectory (finder.GetFileName ())) || (!finder.IsDirectory () && shouldAddFile (finder.GetFileName ())))
				{
					int const openIcon = finder.IsDirectory () ? DDIT_closedFolder : DDIT_file;
					int const closedIcon = finder.IsDirectory () ? DDIT_closedFolder : DDIT_file;

					HTREEITEM afterItem = TVI_SORT;
					if (finder.IsDirectory ())
					{
						HTREEITEM current = treeCtrl.GetChildItem (rootItem);

						if (current)
						{
							for (;;)
							{
								if (!treeCtrl.ItemHasChildren (current))
								{
									afterItem = treeCtrl.GetPrevSiblingItem (current);

									if (!afterItem)
										afterItem = TVI_FIRST;

									break;
								}

								current = treeCtrl.GetNextSiblingItem (current);
								if (!current)
									break;
							}
						}
					}

					HTREEITEM treeItem = 0;
					if (refresh)
						treeItem = findChildTreeItem (treeCtrl, rootItem, finder.GetFileName ());

					if (!treeItem)
						treeItem = treeCtrl.InsertItem (finder.GetFileName (), closedIcon, openIcon, rootItem, afterItem);

					if (finder.IsDirectory ())
					{
						sprintf (subDirectory, "%s%s%s", directory, directory [n-1] != '/' ? "/" : "", finder.GetFileName ());

						addDirectory (subDirectory, treeCtrl, treeItem, refresh);
					}
				}
			}
		}
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	CString getName (CTreeCtrl const & treeCtrl, HTREEITEM current, bool includeRoot = true)
	{
		CString name;

		if (current && !treeCtrl.ItemHasChildren (current))
		{
			while (current)
			{
				HTREEITEM parent = treeCtrl.GetParentItem (current);

				if (parent || (!parent && includeRoot))
				{
					CString const buffer = treeCtrl.GetItemText (current) + (name.GetLength () != 0 ? "/" : "") + name;
					name = buffer;
				}

				current = parent;
			}
		}

		return name;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	CDocument * GetDocument ()
	{
		//-- only display the new view if there is a document
		if (AfxGetMainWnd ())
		{
			MainFrame * const mainFrame = safe_cast<MainFrame *> (AfxGetMainWnd ());
			if (mainFrame)
			{
				CFrameWnd * const frameWnd = mainFrame->GetActiveFrame ();
				if (frameWnd)
					return frameWnd->GetActiveDocument ();
			}
		}

		return 0;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace DialogDirectoryNamespace;

// ======================================================================

DialogDirectory::DialogDirectory(CWnd* pParent /*=NULL*/) : 
	CDialog(DialogDirectory::IDD, pParent),
	m_initialized (false),
	m_imageListSet (false),
	m_imageList ()
{
	//{{AFX_DATA_INIT(DialogDirectory)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogDirectory::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogDirectory)
	DDX_Control(pDX, IDC_TREEVIEW, m_treeCtrl);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogDirectory, CDialog)
	//{{AFX_MSG_MAP(DialogDirectory)
	ON_WM_SIZE()
	ON_NOTIFY(NM_DBLCLK, IDC_TREEVIEW, OnDblclkTreeview)
	ON_NOTIFY(TVN_KEYDOWN, IDC_TREEVIEW, OnKeydownTreeview)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL DialogDirectory::OnInitDialog () 
{
	CDialog::OnInitDialog ();

	if (!m_imageListSet)
	{
		m_imageList.Create (IDB_BITMAP_DIRECTORY, 16, 1, RGB (255,255,255));
		GetTreeCtrl ().SetImageList (&m_imageList, TVSIL_NORMAL);

		m_imageListSet = true;
	}

	reset (false);

	m_initialized = true;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

void DialogDirectory::OnSize (UINT const nType, int const cx, int const cy) 
{
	CDialog::OnSize (nType, cx, cy);
	
	if (!m_initialized)
		return;

	CRect sz;
	GetClientRect (sz);
	sz.DeflateRect(5,5);

	GetTreeCtrl ().MoveWindow (sz);
}

// ----------------------------------------------------------------------

void DialogDirectory::OnOK () 
{
}

// ----------------------------------------------------------------------

void DialogDirectory::reset (bool const refresh)
{
	CString const root = Configuration::getServerObjectTemplatePath ();

	HTREEITEM treeItem = GetTreeCtrl ().GetRootItem ();

	if (!refresh)
	{
		GetTreeCtrl ().DeleteAllItems ();
		treeItem = GetTreeCtrl ().InsertItem (Configuration::getDraftSchematicDirectory ());
	}

	addDirectory (root + (root [root.GetLength () - 1] == '/' ? "" : "/") + Configuration::getDraftSchematicDirectory (), GetTreeCtrl (), treeItem, refresh);
	GetTreeCtrl ().Expand (treeItem, TVE_EXPAND);
}

// ----------------------------------------------------------------------

void DialogDirectory::OnDblclkTreeview (NMHDR * const /*pNMHDR*/, LRESULT * const pResult) 
{
	HTREEITEM treeItem = GetTreeCtrl ().GetSelectedItem ();

	CString name = getName (GetTreeCtrl (), treeItem);
	if (treeItem && GetTreeCtrl ().GetParentItem (treeItem) != 0 && !GetTreeCtrl ().ItemHasChildren (treeItem) && name.GetLength () != 0)
	{
		CString const rootPath = Configuration::getServerObjectTemplatePath ();
		if (!AfxGetApp ()->OpenDocumentFile (rootPath + (rootPath [rootPath.GetLength () - 1] == '/' ? "" : "/") + name))
		{
			CString buffer;
			buffer.Format ("%s could not be opened\n", rootPath + (rootPath [rootPath.GetLength () - 1] == '/' ? "" : "/") + name);
			CONSOLE_PRINT (buffer);
		}
		else
			SetFocus ();
	}
	
	*pResult = 0;
}

// ----------------------------------------------------------------------

void DialogDirectory::openItem (HTREEITEM treeItem, bool const closeDocument)
{
	if (!GetTreeCtrl ().ItemHasChildren (treeItem))
	{
		CString const assetName = getName (GetTreeCtrl (), treeItem);
		CString const rootPath = Configuration::getServerObjectTemplatePath ();
		CString const name = rootPath + (rootPath [rootPath.GetLength () - 1] == '/' ? "" : "/") + assetName;
		DraftSchematic draftSchematic;
		if (!draftSchematic.load (name))
			CONSOLE_PRINT ("Load failed for: " + name + "\n");
	}

	treeItem = m_treeCtrl.GetChildItem (treeItem);
	while (treeItem)
	{
		openItem (treeItem, closeDocument);
		treeItem = m_treeCtrl.GetNextSiblingItem (treeItem);
	}
}

// ----------------------------------------------------------------------

void DialogDirectory::openAll ()
{
	CONSOLE_PRINT ("----- START: OPEN ALL -----\r\n");

	HTREEITEM treeItem = GetTreeCtrl ().GetSelectedItem ();
	openItem (treeItem ? treeItem : TVI_ROOT, true);

	CONSOLE_PRINT ("----- STOP: OPEN ALL -----\r\n");
}

// ----------------------------------------------------------------------

void DialogDirectory::OnKeydownTreeview(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	if (pTVKeyDown->wVKey == 116)
		refresh ();

	*pResult = 0;
}

// ----------------------------------------------------------------------

void DialogDirectory::refresh ()
{
	reset (true);
}

// ======================================================================

