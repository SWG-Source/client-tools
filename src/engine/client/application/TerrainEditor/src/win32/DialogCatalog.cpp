//
// DialogCatalog.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "DialogCatalog.h"

#include "FloraGroupFrame.h"
#include "RadialGroupFrame.h"
#include "ShaderGroupFrame.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

DialogCatalog::DialogCatalog(CWnd* pParent /*=NULL*/) : 
	CDialog(DialogCatalog::IDD, pParent),
	initialized (FALSE),
	imageListSet (false),
	imageList (),
	m_shaders (0),
	m_flora (0),
	m_radial (0),
	m_blends (0),

	//-- widgets
	m_treeCtrl ()
{
	//{{AFX_DATA_INIT(DialogCatalog)
	//}}AFX_DATA_INIT
}

//-------------------------------------------------------------------

DialogCatalog::~DialogCatalog (void)
{
	m_shaders = 0;
	m_flora   = 0;
	m_radial  = 0;
	m_blends  = 0;
}

//-------------------------------------------------------------------

void DialogCatalog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogCatalog)
	DDX_Control(pDX, IDC_CATALOG_TREE, m_treeCtrl);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(DialogCatalog, CDialog)
	//{{AFX_MSG_MAP(DialogCatalog)
	ON_WM_SIZE()
	ON_COMMAND(ID__INSERTINTOSHADERGROUP, OnInsertintoshadergroup)
	ON_NOTIFY(NM_RCLICK, IDC_CATALOG_TREE, OnRclickCatalogTree)
	ON_COMMAND(ID__INSERTINTOFLORAGROUP, OnInsertintofloragroup)
	ON_COMMAND(ID__INSERTINTORADIALGROUP, OnInsertintoradialgroup)
	ON_UPDATE_COMMAND_UI(ID__INSERTINTOFLORAGROUP, OnUpdateInsertintofloragroup)
	ON_UPDATE_COMMAND_UI(ID__INSERTINTORADIALGROUP, OnUpdateInsertintoradialgroup)
	ON_UPDATE_COMMAND_UI(ID__INSERTINTOSHADERGROUP, OnUpdateInsertintoshadergroup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

enum CatalogIconList
{
	CIL_unknown,  
	CIL_closedFolder,
	CIL_openFolder,
	CIL_shaderTemplate,
	CIL_appearance,
	CIL_blend     
};

//-------------------------------------------------------------------

#if 0

static CatalogIconList getIconType (const char* name)
{
	if (!name)
		return CIL_unknown;

	const int length = istrlen (name);

	if (length < 5)
		return CIL_unknown;

	const char* extension = &name [length - 4];

	if (_stricmp (extension, ".msh") == 0)
		return CIL_appearance;

	if (_stricmp (extension, ".cmp") == 0)
		return CIL_appearance;

	if (_stricmp (extension, ".lod") == 0)
		return CIL_appearance;

	if (_stricmp (extension, ".spr") == 0)
		return CIL_appearance;

	if (_stricmp (extension, ".sht") == 0)
		return CIL_shaderTemplate;

	if (_stricmp (extension, ".dds") == 0)
		return CIL_blend;

	return CIL_unknown;
}

#endif

//-------------------------------------------------------------------

static HTREEITEM findParent (const HTREEITEM parent, const CTreeCtrl& tree, const CString& item)
{
	HTREEITEM current = tree.GetChildItem (parent);
	while (current)
	{
		if (tree.GetItemText (current) == item)
			break;

		current = tree.GetNextSiblingItem (current);
	}

	return current;
}

//-------------------------------------------------------------------

static HTREEITEM getRoot (const CTreeCtrl& tree, HTREEITEM hitem)
{
	HTREEITEM parent         = hitem;
	HTREEITEM possibleParent = hitem;

	do
	{
		possibleParent = tree.GetParentItem (possibleParent);

		if (possibleParent)
			parent = possibleParent;
	}
	while (possibleParent != 0);

	return parent;
}

//-------------------------------------------------------------------

void DialogCatalog::add (HTREEITEM parent, const CString& family, const CString& child)
{
	//-- did we find a '/'?
	const int index = family.Find ('/');
	if (index != -1)
	{
		CString directory = family.Left (index);

		const int length = family.GetLength ();
		CString newFamily = family.Right (length - index - 1);

		//-- find the parent in the tree
		const HTREEITEM current   = findParent (parent, m_treeCtrl, directory);
		const HTREEITEM newParent = current ? current : m_treeCtrl.InsertItem (directory, CIL_closedFolder, CIL_openFolder, parent);

		add (newParent, newFamily, child);
	}
	else
	{
		const HTREEITEM current   = findParent (parent, m_treeCtrl, family);
		const HTREEITEM newParent = current ? current : m_treeCtrl.InsertItem (family, CIL_closedFolder, CIL_openFolder, parent);

		const CatalogIconList icon = getRoot (m_treeCtrl, newParent) == m_shaders ? CIL_shaderTemplate : CIL_appearance;

		IGNORE_RETURN (m_treeCtrl.InsertItem (child, icon, icon, newParent));
	}
}

//-------------------------------------------------------------------

void DialogCatalog::readIniFile (const char* filename)
{
	//-- open the config file
	CStdioFile infile;
	if (infile.Open (filename, CFile::modeRead | CFile::typeText))
	{
		//-- read each line...
		CString line;
		while (infile.ReadString (line))
		{
			//-- see if the line is empty
			line.TrimLeft ();
			line.TrimRight ();
			if (line.GetLength () == 0)
				continue;

			//-- see if the first character is a comment
			int index = line.Find (";");
			if (index == 0)
				continue;

			//-- find the =
			index = line.Find ("=");
			if (index == -1)
				break;

			//-- left half goes in family, right half goes in child
			CString family;
			CString child;

			int length = line.GetLength ();
			family     = line.Left (index);
			child      = line.Right (length - index - 1);

			switch (line [0])
			{
			case 'S':
				{
					//-- find /
					index = line.Find ("/");

					length = family.GetLength ();
					family = family.Right (length - index - 1);

					add (m_shaders, family, child);
				}
				break;

			case 'F':
				{
					//-- find /
					index = line.Find ("/");

					length = family.GetLength ();
					family = family.Right (length - index - 1);

					add (m_flora, family, child);
				}
				break;

			case 'R':
				{
					//-- find /
					index = line.Find ("/");

					length = family.GetLength ();
					family = family.Right (length - index - 1);

					add (m_radial, family, child);
				}
				break;

			default:
				break;
			}
		}
	}
}

//-------------------------------------------------------------------

void DialogCatalog::reset (void)
{
	IGNORE_RETURN (m_treeCtrl.DeleteAllItems ());

	m_shaders = m_treeCtrl.InsertItem ("Shader Families", CIL_closedFolder, CIL_openFolder);
	m_flora   = m_treeCtrl.InsertItem ("Flora Families",  CIL_closedFolder, CIL_openFolder);
	m_radial  = m_treeCtrl.InsertItem ("Radial Families", CIL_closedFolder, CIL_openFolder);
//	m_blends  = m_treeCtrl.InsertItem ("Blend Families", CIL_closedFolder, CIL_openFolder);

	//-- read the ini file
	readIniFile ("terrainEditor.ini");
}

//-------------------------------------------------------------------

BOOL DialogCatalog::OnInitDialog() 
{
	//-- lint, shut up!
	UNREF (CIL_unknown);
	UNREF (CIL_blend);

	CDialog::OnInitDialog();

	initialized = TRUE;

	reset ();

	if (!imageListSet)
	{
		IGNORE_RETURN (imageList.Create (IDB_BITMAP_CATALOG, 16, 1, RGB (255,255,255)));
		IGNORE_RETURN (m_treeCtrl.SetImageList (&imageList, TVSIL_NORMAL));

		imageListSet = true;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//-------------------------------------------------------------------

void DialogCatalog::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	if(!initialized)
		return;

	CRect sz;
	GetClientRect(sz);
	sz.DeflateRect(5,5);

	m_treeCtrl.MoveWindow(sz);
}

//-------------------------------------------------------------------

void DialogCatalog::OnOK() 
{
}

//-------------------------------------------------------------------

void DialogCatalog::OnCancel() 
{
}

//-------------------------------------------------------------------

void DialogCatalog::OnRclickCatalogTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREF (pNMHDR);

	HTREEITEM selection = m_treeCtrl.GetSelectedItem ();

	if (selection && getRoot (m_treeCtrl, selection) == m_shaders)
	{
		HTREEITEM child = m_treeCtrl.GetChildItem (selection);

		if (child)
		{
			child = m_treeCtrl.GetChildItem (child);

			if (!child)
			{
				POINT pntPos; 
				IGNORE_RETURN (::GetCursorPos( &pntPos ));

				CMenu menu;
				IGNORE_RETURN (menu.LoadMenu (IDR_MENU_CATALOG));

				CMenu* subMenu = menu.GetSubMenu (0);
				IGNORE_RETURN (subMenu->TrackPopupMenu (TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pntPos.x, pntPos.y, this));
			}
		}
	}
	else
		if (selection && getRoot (m_treeCtrl, selection) == m_flora)
		{
			HTREEITEM child = m_treeCtrl.GetChildItem (selection);

			if (child)
			{
				child = m_treeCtrl.GetChildItem (child);

				if (!child)
				{
					POINT pntPos; 
					IGNORE_RETURN (::GetCursorPos( &pntPos ));

					CMenu menu;
					IGNORE_RETURN (menu.LoadMenu (IDR_MENU_CATALOG));

					CMenu* subMenu = menu.GetSubMenu (1);
					IGNORE_RETURN (subMenu->TrackPopupMenu (TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pntPos.x, pntPos.y, this));
				}
			}
		}
		else
			if (selection && getRoot (m_treeCtrl, selection) == m_radial)
			{
				HTREEITEM child = m_treeCtrl.GetChildItem (selection);

				if (child)
				{
					child = m_treeCtrl.GetChildItem (child);

					if (!child)
					{
						POINT pntPos; 
						IGNORE_RETURN (::GetCursorPos( &pntPos ));

						CMenu menu;
						IGNORE_RETURN (menu.LoadMenu (IDR_MENU_CATALOG));

						CMenu* subMenu = menu.GetSubMenu (2);
						IGNORE_RETURN (subMenu->TrackPopupMenu (TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pntPos.x, pntPos.y, this));
					}
				}
			}

	*pResult = 0;
}

//-------------------------------------------------------------------

void DialogCatalog::OnInsertintoshadergroup() 
{
	TerrainEditorDoc* doc = GetApp ()->getDocument ();
	if (doc)
	{
		if (doc->getShaderGroupFrame ())
		{
			ArrayList<CString*> children;

			HTREEITEM selection = m_treeCtrl.GetSelectedItem ();
			CString familyName = m_treeCtrl.GetItemText (selection);

			HTREEITEM current = m_treeCtrl.GetChildItem (selection);
			while (current)
			{
				children.add (new CString (m_treeCtrl.GetItemText (current)));

				current = m_treeCtrl.GetNextSiblingItem (current);
			}

			static_cast<ShaderGroupFrame*> (doc->getShaderGroupFrame ())->addFamily (familyName, children);

			int i;
			for (i = 0; i < children.getNumberOfElements (); i++)
			{
				delete children [i];
			}
		}
	}
}

//-------------------------------------------------------------------

void DialogCatalog::OnUpdateInsertintoshadergroup(CCmdUI* pCmdUI) 
{
	bool enable = false;

	HTREEITEM selection = m_treeCtrl.GetSelectedItem ();

	if (selection && getRoot (m_treeCtrl, selection) == m_shaders)
	{
		HTREEITEM child = m_treeCtrl.GetChildItem (selection);

		if (child)
		{
			child = m_treeCtrl.GetChildItem (child);

			if (!child)
				enable = true;
		}
	}

	pCmdUI->Enable (enable ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void DialogCatalog::OnInsertintofloragroup() 
{
	TerrainEditorDoc* doc = GetApp ()->getDocument ();
	if (doc)
	{
		if (doc->getFloraGroupFrame ())
		{
			ArrayList<CString*> children;

			HTREEITEM selection = m_treeCtrl.GetSelectedItem ();
			CString familyName = m_treeCtrl.GetItemText (selection);

			HTREEITEM current = m_treeCtrl.GetChildItem (selection);
			while (current)
			{
				children.add (new CString (m_treeCtrl.GetItemText (current)));

				current = m_treeCtrl.GetNextSiblingItem (current);
			}

			static_cast<FloraGroupFrame*> (doc->getFloraGroupFrame ())->addFamily (familyName, children);

			int i;
			for (i = 0; i < children.getNumberOfElements (); i++)
			{
				delete children [i];
			}
		}
	}
}

//-------------------------------------------------------------------

void DialogCatalog::OnUpdateInsertintofloragroup(CCmdUI* pCmdUI) 
{
	bool enable = false;

	HTREEITEM selection = m_treeCtrl.GetSelectedItem ();

	if (selection && getRoot (m_treeCtrl, selection) == m_flora)
	{
		HTREEITEM child = m_treeCtrl.GetChildItem (selection);

		if (child)
		{
			child = m_treeCtrl.GetChildItem (child);

			if (!child)
				enable = true;
		}
	}

	pCmdUI->Enable (enable ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void DialogCatalog::OnInsertintoradialgroup() 
{
	TerrainEditorDoc* doc = GetApp ()->getDocument ();
	if (doc)
	{
		if (doc->getRadialGroupFrame ())
		{
			ArrayList<CString*> children;

			HTREEITEM selection = m_treeCtrl.GetSelectedItem ();
			CString familyName = m_treeCtrl.GetItemText (selection);

			HTREEITEM current = m_treeCtrl.GetChildItem (selection);
			while (current)
			{
				children.add (new CString (m_treeCtrl.GetItemText (current)));

				current = m_treeCtrl.GetNextSiblingItem (current);
			}

			static_cast<RadialGroupFrame*> (doc->getRadialGroupFrame ())->addFamily (familyName, children);

			int i;
			for (i = 0; i < children.getNumberOfElements (); i++)
			{
				delete children [i];
			}
		}
	}
}

//-------------------------------------------------------------------

void DialogCatalog::OnUpdateInsertintoradialgroup(CCmdUI* pCmdUI) 
{
	bool enable = false;

	HTREEITEM selection = m_treeCtrl.GetSelectedItem ();

	if (selection && getRoot (m_treeCtrl, selection) == m_radial)
	{
		HTREEITEM child = m_treeCtrl.GetChildItem (selection);

		if (child)
		{
			child = m_treeCtrl.GetChildItem (child);

			if (!child)
				enable = true;
		}
	}

	pCmdUI->Enable (enable ? TRUE : FALSE);
}

//-------------------------------------------------------------------
