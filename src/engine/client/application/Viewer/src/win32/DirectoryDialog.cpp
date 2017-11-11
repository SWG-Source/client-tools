// ======================================================================
//
// DirectoryDialog.cpp
// Copyright 2001-2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstViewer.h"
#include "DirectoryDialog.h"

#include "clientGraphics/ShaderPrimitiveSetTemplate.h"
#include "clientGraphics/ShaderTemplate.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientObject/MeshAppearanceTemplate.h"
#include "clientSkeletalAnimation/MeshGeneratorTemplate.h"
#include "clientSkeletalAnimation/MeshGeneratorTemplateList.h"
#include "clientSkeletalAnimation/SkeletalAppearanceTemplate.h"
#include "DialogProgress.h"
#include "MainFrm.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ArrayList.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedUtility/FileName.h"
#include "Viewer.h"
#include "ViewerDoc.h"
#include "ViewerView.h"

//-------------------------------------------------------------------

namespace DirectoryDialogNamespace
{
	Tag const TAG_MLOD = TAG(M,L,O,D);
}

using namespace DirectoryDialogNamespace;

//-------------------------------------------------------------------

DirectoryDialog::DirectoryDialog(CWnd* pParent /*=NULL*/)
	: CDialog(DirectoryDialog::IDD, pParent),
	imageListSet (false)
{
	//{{AFX_DATA_INIT(DirectoryDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	initialized = FALSE;
}

//-------------------------------------------------------------------

void DirectoryDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DirectoryDialog)
	DDX_Control(pDX, IDC_DIRECTORY_TREE, m_treeCtrl);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DirectoryDialog, CDialog)
	//{{AFX_MSG_MAP(DirectoryDialog)
	ON_WM_SIZE()
	ON_NOTIFY(NM_DBLCLK, IDC_DIRECTORY_TREE, OnDblclkDirectoryTree)
	ON_NOTIFY(TVN_KEYDOWN, IDC_DIRECTORY_TREE, OnKeydownDirectoryTree)
	ON_NOTIFY(NM_RCLICK, IDC_DIRECTORY_TREE, OnRclickDirectoryTree)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_DIRECTORY_TREE, OnItemexpandingDirectoryTree)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

static enum DirectoryIconList
{
	DIL_unknown,
	DIL_closedFolder,
	DIL_openFolder,
	DIL_apt,
	DIL_mesh,
	DIL_component,
	DIL_lod,
	DIL_sprite,
	DIL_particle,
	DIL_skeletal,
	DIL_shaderTemplate,
	DIL_dds,
	DIL_terrain,
	DIL_hardpointHierarchy
};

//-------------------------------------------------------------------

static bool shouldAddDirectory (const char* const directoryName)
{
	return (
		strcmp (directoryName, "appearance") == 0 ||
		strcmp (directoryName, "mesh") == 0 ||
		strcmp (directoryName, "sprite") == 0 ||
		strcmp (directoryName, "particle") == 0 ||
		strcmp (directoryName, "lod") == 0 ||
		strcmp (directoryName, "component") == 0 ||
		strcmp (directoryName, "shader") == 0 ||
		strcmp (directoryName, "shadertemplates") == 0 ||
		strcmp (directoryName, "texture") == 0 ||
		strcmp (directoryName, "animation") == 0
	);
}

//-------------------------------------------------------------------

static bool isFileExt (const char* const ext1, const char* ext2)
{
	return 
		ext1 [0] == ext2 [0] &&
		ext1 [1] == ext2 [1] &&
		ext1 [2] == ext2 [2] &&
		ext1 [3] == ext2 [3];
}

//-------------------------------------------------------------------

static bool shouldAddFile (const char* const fileName)
{
	const int length = strlen (fileName);

	//-- terrain layers
	if (isFileExt (&fileName [length - 4], ".lay"))
		return false;

	//-- terrain color ramps
	if (isFileExt (&fileName [length - 4], ".tga"))
		return false;

	//-- particle systems
	if (isFileExt (&fileName [length - 4], ".prt"))
		return false;

	return true;
}

//-------------------------------------------------------------------

static DirectoryIconList getIconType (const char* name)
{
	if (!name)
		return DIL_unknown;

	const int length = istrlen (name);

	if (length < 5)
		return DIL_unknown;

	const char* extension = &name [length - 4];

	if (isFileExt (extension, ".ans"))
		return DIL_skeletal;

	if (isFileExt (extension, ".apt"))
		return DIL_apt;

	if (isFileExt (extension, ".msh"))
		return DIL_mesh;

	if (isFileExt (extension, ".cmp"))
		return DIL_component;

	if (isFileExt (extension, ".lod"))
		return DIL_lod;

	if (isFileExt (extension, ".spr"))
		return DIL_sprite;

	if (isFileExt (extension, ".prt"))
		return DIL_particle;

	if (isFileExt (extension, ".sat"))
		return DIL_skeletal;

	if (isFileExt (extension, ".sht"))
		return DIL_shaderTemplate;

	if (isFileExt (extension, ".dds"))
		return DIL_dds;

	if (isFileExt (extension, ".trn"))
		return DIL_terrain;

	if (isFileExt (extension, ".hard"))
		return DIL_hardpointHierarchy;

	return DIL_unknown;
}

//-------------------------------------------------------------------

static void addDirectory (const char* directory, CTreeCtrl& tree, HTREEITEM root)
{
	int n = istrlen (directory);

	char searchMask [Os::MAX_PATH_LENGTH];
	char subDirectory [Os::MAX_PATH_LENGTH];

	sprintf (searchMask, "%s%s*.*", directory, directory [n-1] != '/' ? "/" : "");

	CFileFind finder;
	BOOL working = finder.FindFile (searchMask);
	while (working)
	{
		working = finder.FindNextFile();

		if (!finder.IsDots ())
		{
			if (!finder.IsDirectory () && shouldAddFile (finder.GetFileName ()) || (finder.IsDirectory () && shouldAddDirectory (finder.GetFileName ())))
			{
				const int openIcon   = finder.IsDirectory () ? DIL_openFolder   : getIconType (finder.GetFileName ());
				const int closedIcon = finder.IsDirectory () ? DIL_closedFolder : getIconType (finder.GetFileName ());

				HTREEITEM item = tree.InsertItem (finder.GetFileName (), closedIcon, openIcon, root);

				if (finder.IsDirectory ())
				{
					sprintf (subDirectory, "%s%s%s", directory, directory [n-1] != '/' ? "/" : "", finder.GetFileName ());

					addDirectory (subDirectory, tree, item);

					if (!tree.ItemHasChildren (item))
						tree.DeleteItem (item);
				}
			}
		}
	}

	tree.SortChildren (root);
}

//-------------------------------------------------------------------

static void fixupName (char* name)
{
	while (*name)
	{
		if (*name == '\\')
			*name = '/';

		name++;
	}
}

//-------------------------------------------------------------------
static void getViewerDirectories(std::vector<std::string>& viewerDirectories)
{
	viewerDirectories.clear();

	if(ConfigFile::isInstalled())
	{
		ConfigFile::Section* configFileSection = ConfigFile::getSection("ViewerDirectories");
		if(configFileSection)
		{
			ConfigFile::Key* key = configFileSection->findKey("viewerDir");
			if(key)
			{
				int numValues = key->getCount();
				for(int i = 0; i < numValues; ++i)
				{
					char buffer[Os::MAX_PATH_LENGTH];
					Os::getAbsolutePath(key->getAsString(i,""),buffer,sizeof(buffer));
					std::string currentValue(buffer);
					viewerDirectories.push_back(currentValue);
				}
			}
		}
	}
}
//-------------------------------------------------------------------
void DirectoryDialog::reset (void)
{	
	std::vector<std::string> viewerDirectories;
	getViewerDirectories(viewerDirectories);
	bool useViewerDirectoriesFromConfig = false;
	int n;
	if(viewerDirectories.size())
	{
		n = viewerDirectories.size();
		useViewerDirectoriesFromConfig = true;
	}
	else
	{
		n = TreeFile::getNumberOfSearchPaths();
	}

	DialogProgress* dlg = 0;

	if (n)
	{
		dlg = new DialogProgress ();
		IGNORE_RETURN (dlg->Create ());
		dlg->SetRange (0, n);
		IGNORE_RETURN (dlg->SetStep (1));
		IGNORE_RETURN (dlg->SetPos (0));
	}

	m_treeCtrl.DeleteAllItems ();

	bool quit = false;

	char name [Os::MAX_PATH_LENGTH];

	int i;
	for (i = 0; !quit && i < n; i++)
	{
		//-- populate tree

		if(useViewerDirectoriesFromConfig)
		{
			strcpy(name, viewerDirectories[i].c_str());
		}
		else
		{
			strcpy (name, TreeFile::getSearchPath (i));
		}

		if (dlg)
		{
			dlg->SetStatus (name);
			IGNORE_RETURN (dlg->StepIt ());

			if (dlg->CheckCancelButton ())
				quit = true;
		}

		fixupName (name);

		HTREEITEM item = m_treeCtrl.InsertItem (name, DIL_closedFolder, DIL_openFolder);

		//--
		//m_treeCtrl.InsertItem ("__empty", DIL_closedFolder, DIL_openFolder, item);
		addDirectory (name, m_treeCtrl, item);
		//--

		if (!m_treeCtrl.ItemHasChildren (item))
			m_treeCtrl.DeleteItem (item);
	}

	if (dlg)
		delete dlg;
}

//-------------------------------------------------------------------

BOOL DirectoryDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CRect sz;
	GetClientRect(sz);
	sz.DeflateRect(5,5);
	
	initialized = TRUE;

	reset ();

	if (!imageListSet)
	{
		imageList.Create (IDB_BITMAP_DIRECTORY, 16, 1, RGB (255,255,255));
		m_treeCtrl.SetImageList (&imageList, TVSIL_NORMAL);

		imageListSet = true;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//-------------------------------------------------------------------

void DirectoryDialog::OnSize(UINT nType, int cx, int cy) 
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

static CString getName (const CTreeCtrl& tree, HTREEITEM current, bool includeRoot=true)
{
	CString name;

	if (current && !tree.ItemHasChildren (current))
	{
		while (current)
		{
			HTREEITEM parent = tree.GetParentItem (current);

			if (parent || (!parent && includeRoot))
			{
				CString tmp = tree.GetItemText (current) + (name.GetLength () != 0 ? "/" : "") + name;

				name = tmp;
			}

			current = parent;
		}
	}

	return name;
}

void DirectoryDialog::OnDblclkDirectoryTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREF (pNMHDR);
	*pResult = 0;

	HTREEITEM selection = m_treeCtrl.GetSelectedItem ();

	CString name = getName (m_treeCtrl, selection);

	// first check to see if it's an animation
	if(name && (name.Find (".ans") != -1))
	{
		CViewerDoc* pDoc = GetViewerApp()->getDocument();
		if(pDoc)
		{
			pDoc->playAnim(name.GetString(),false,true);
		}
	}
	else if (selection && m_treeCtrl.GetParentItem (selection) != 0 && m_treeCtrl.ItemHasChildren (selection) == false && name.GetLength () != 0)
	{
		AfxGetApp ()->OpenDocumentFile (name);
		SetFocus ();
	}
}

//-------------------------------------------------------------------

CString DirectoryDialog::getSelectedFilename() const
{
	HTREEITEM selection = m_treeCtrl.GetSelectedItem ();

	CString name = getName (m_treeCtrl, selection);

	if (selection && m_treeCtrl.GetParentItem (selection) != 0 && m_treeCtrl.ItemHasChildren (selection) == false && name.GetLength () != 0)
	{
		return name;
	}

	return "";
}

//-------------------------------------------------------------------

void DirectoryDialog::OnOK() 
{
	// TODO: Add extra validation here
	
//	CDialog::OnOK();
}

//-------------------------------------------------------------------

void DirectoryDialog::OnKeydownDirectoryTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;

	*pResult              = 0;
	

	if (pTVKeyDown->wVKey == VK_DELETE)
	{
		HTREEITEM   selection = m_treeCtrl.GetSelectedItem ();

		if (!selection)
			return;

		//-- key down operation that affect root items
		if (m_treeCtrl.GetParentItem (selection) == 0)
		{
			CString tmp;
			tmp.Format ("Are you sure you wish to remove %s?", m_treeCtrl.GetItemText (selection));
			if (MessageBox (tmp, 0, MB_YESNO) == IDYES)
				m_treeCtrl.DeleteItem (selection);
		}
	}
	else if(pTVKeyDown->wVKey == VK_SPACE)
	{
		if(m_treeCtrl.GetSelectedCount() > 0)
		{
			CTreeItemList treeItemList;
			m_treeCtrl.GetSelectedList(treeItemList);
			uint32 numItems = treeItemList.GetCount();
			bool queueAnimation = false;
			for(uint32 i = 0; i < numItems; ++i)
			{
				POSITION pos = treeItemList.FindIndex(i);
				HTREEITEM currentItem = treeItemList.GetAt(pos);
				if(m_treeCtrl.GetParentItem(currentItem)) // skip roots
				{
					CString name = getName (m_treeCtrl, currentItem);
					if(name && (name.Find (".ans") != -1))
					{
						CViewerDoc* pDoc = GetViewerApp()->getDocument();
						if(pDoc)
						{
							pDoc->playAnim(name.GetString(),queueAnimation,true);
							queueAnimation = true;
						}
					}
					else if(m_treeCtrl.GetParentItem (currentItem) != 0 && m_treeCtrl.ItemHasChildren (currentItem) == false && name.GetLength () != 0)
					{
						AfxGetApp ()->OpenDocumentFile (name);
					}	
				}
			}
			SetFocus ();
		}
	}
}

//-------------------------------------------------------------------

BOOL DirectoryDialog::PreTranslateMessage(MSG* pMsg) 
{
	return CDialog::PreTranslateMessage(pMsg);
}


//-------------------------------------------------------------------

class Item
{
public:

	CString path;
	CString asset;
};

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

static void checkDupes (ArrayList<Item*>& itemList, int& duplicateCount, const CTreeCtrl& tree, HTREEITEM root) 
{
	while (root)
	{
		if (tree.ItemHasChildren (root))
			checkDupes (itemList, duplicateCount, tree, tree.GetChildItem (root));
		else
		{
			CString path  = tree.GetItemText (getRoot (tree, root));
			CString asset = getName (tree, root, false);

			//-- 
			int i;
			for (i = 0; i < itemList.getNumberOfElements (); i++)
				if (asset.GetLength () == itemList [i]->asset.GetLength () && asset.CompareNoCase (itemList [i]->asset) == 0)
					break;

			if (i < itemList.getNumberOfElements ())
			{
				CONSOLE_PRINT ("Duplicate detected! ");
				CONSOLE_PRINT (asset);
				CONSOLE_PRINT (": in ");
				CONSOLE_PRINT (itemList [i]->path);
				CONSOLE_PRINT (" and in ");
				CONSOLE_PRINT (path);
				CONSOLE_PRINT ("\r\n");

				duplicateCount++;
			}
			else
			{
				Item* item = new Item;
				item->asset = asset;
				item->path  = path;
				itemList.add (item);
			}
		}

		root = tree.GetNextSiblingItem (root);
	}
}

//-------------------------------------------------------------------

void DirectoryDialog::OnButtonCheckDupes() 
{
	CONSOLE_PRINT ("-------------------------------------------------------------------\r\n");

	ArrayList<Item*> itemList (2000);
	int         duplicateCount = 0;
	HTREEITEM   root           = m_treeCtrl.GetRootItem ();

	checkDupes (itemList, duplicateCount, m_treeCtrl, root);

	int i;
	for (i = 0; i < itemList.getNumberOfElements (); i++)
	{
		delete itemList [i];
		itemList [i] = 0;
	}

	itemList.clear ();

	char buffer [1000];
	sprintf (buffer, "%i duplicates found...", duplicateCount);
	CONSOLE_PRINT (buffer);
	CONSOLE_PRINT ("\r\n");
}

//-------------------------------------------------------------------

void DirectoryDialog::OnRclickDirectoryTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREF (pNMHDR);

	// TODO: Add your control notification handler code here
//	HTREEITEM selection = m_treeCtrl.GetSelectedItem ();

//	if (selection && !m_treeCtrl.ItemHasChildren (selection))
//		MessageBox (getName (m_treeCtrl, selection, false));

	*pResult = 0;
}

//-------------------------------------------------------------------

static HTREEITEM findItem (CTreeCtrl& tree, HTREEITEM root, const CString& name)
{
	HTREEITEM item = 0;

	while (root)
	{
		if (tree.ItemHasChildren (root))
			item = ::findItem (tree, tree.GetChildItem (root), name);

		if (item)
			return item;

		CString asset = tree.GetItemText (root);
		if (asset.Find (name) != -1)
			item = root;

		if (item)
			return item;

		root = tree.GetNextSiblingItem (root);
	}

	return item;
}

//-------------------------------------------------------------------

void DirectoryDialog::OnButtonFind(const CString& name)
{
	HTREEITEM selection = findItem (m_treeCtrl, m_treeCtrl.GetRootItem (), name);

	if (selection)
	{
		m_treeCtrl.EnsureVisible (selection);
		m_treeCtrl.SelectItem (selection);
	}
	else
	{
		MessageBox (name + " not found");
	}
}

//-------------------------------------------------------------------

void DirectoryDialog::openItem (HTREEITEM root, const bool writeCustomizationData, const bool closeDocument, const bool debugDump) 
{
	while (root)
	{
//		if (m_treeCtrl.ItemHasChildren (root))
//			openItem (m_treeCtrl.GetChildItem (root));

		const CString string = m_treeCtrl.GetItemText (root);

		if (writeCustomizationData)
		{
			CONSOLE_PRINT ("processing customization data for ");
			CONSOLE_PRINT (string);
			CONSOLE_PRINT ("\r\n");
		}
		else
		{
			CONSOLE_PRINT ("opening ");
			CONSOLE_PRINT (string);
			CONSOLE_PRINT ("\r\n");
		}

		m_treeCtrl.SelectItem (root);
		long result;
		OnDblclkDirectoryTree(0, &result);

		if (GetViewerApp ()->getDocument ())
		{
			if (writeCustomizationData)
			{
				GetViewerApp ()->getDocument ()->writeObjectTemplateCustomizationData (false);
				GetViewerApp ()->getDocument ()->OnCloseDocument ();
			}
			else
				if (debugDump)
				{
					GetViewerApp ()->getDocument ()->debugDump ();
					GetViewerApp ()->getDocument ()->OnCloseDocument ();
				}
				else 
					if (closeDocument)
					{
						POSITION position = GetViewerApp ()->getDocument ()->GetFirstViewPosition ();
						CView* const view = GetViewerApp ()->getDocument ()->GetNextView (position);
						if (view)
						{
							AfxWndProc (view->m_hWnd, WM_PAINT, 0, 0);
							GetViewerApp ()->getDocument ()->OnCloseDocument ();
						}
					}
		}

		root = m_treeCtrl.GetNextSiblingItem (root);
	}
}

// ----------------------------------------------------------------------

void DirectoryDialog::openSingleItem (HTREEITEM root, const bool writeCustomizationData, const bool closeDocument, const bool debugDump) 
{
	const CString string = m_treeCtrl.GetItemText (root);

	if (writeCustomizationData)
	{
		CONSOLE_PRINT ("processing customization data for ");
		CONSOLE_PRINT (string);
		CONSOLE_PRINT ("\r\n");
	}
	else
	{
		CONSOLE_PRINT ("opening ");
		CONSOLE_PRINT (string);
		CONSOLE_PRINT ("\r\n");
	}

	m_treeCtrl.SelectItem (root);
	long result;
	OnDblclkDirectoryTree(0, &result);

	if (GetViewerApp ()->getDocument ())
	{
		if (writeCustomizationData)
		{
			GetViewerApp ()->getDocument ()->writeObjectTemplateCustomizationData (false);
			GetViewerApp ()->getDocument ()->OnCloseDocument ();
		}
		else
			if (debugDump)
			{
				GetViewerApp ()->getDocument ()->debugDump ();
				GetViewerApp ()->getDocument ()->OnCloseDocument ();
			}
			else 
				if (closeDocument)
				{
					POSITION position = GetViewerApp ()->getDocument ()->GetFirstViewPosition ();
					CView* const view = GetViewerApp ()->getDocument ()->GetNextView (position);
					if (view)
					{
						AfxWndProc (view->m_hWnd, WM_PAINT, 0, 0);
						GetViewerApp ()->getDocument ()->OnCloseDocument ();
					}
				}
	}
}

//-------------------------------------------------------------------

void DirectoryDialog::OnButtonOpenAll (const bool closeDocument)
{
	CONSOLE_PRINT ("----- START: OPEN ALL -----\r\n");

	if (m_treeCtrl.GetSelectedItem () && m_treeCtrl.ItemHasChildren (m_treeCtrl.GetSelectedItem ()))
	{
		HTREEITEM selection = m_treeCtrl.GetSelectedItem ();

		openItem (m_treeCtrl.GetChildItem (m_treeCtrl.GetSelectedItem ()), false, closeDocument, false);

		m_treeCtrl.SelectItem (selection);
		m_treeCtrl.EnsureVisible (selection);
	}

	CONSOLE_PRINT ("----- STOP: OPEN ALL -----\r\n");
}

// ----------------------------------------------------------------------

void DirectoryDialog::writeSatCustomizationData(HTREEITEM treeItem)
{
	while (treeItem)
	{
		//-- Process children.
		if (m_treeCtrl.ItemHasChildren (treeItem))
			writeSatCustomizationData (m_treeCtrl.GetChildItem (treeItem));

		//-- Check if this is an MGN.
		CString const fileName = m_treeCtrl.GetItemText (treeItem);
		if (fileName.Find (".sat") != -1)
		{
			openSingleItem (treeItem, true, true, false);
		}

		m_treeCtrl.SelectItem (treeItem);
		m_treeCtrl.EnsureVisible (treeItem);

		treeItem = m_treeCtrl.GetNextSiblingItem (treeItem);
	}
}

//-------------------------------------------------------------------

void DirectoryDialog::OnButtonWriteCustomizationData ()
{
	CONSOLE_PRINT ("----- START: WRITE CUSTOMIZATION DATA -----\r\n");

	HTREEITEM treeItem = m_treeCtrl.GetRootItem ();

	while (treeItem)
	{
		writeSatCustomizationData(m_treeCtrl.GetChildItem (treeItem));
		treeItem = m_treeCtrl.GetNextSiblingItem (treeItem);
	}

	CONSOLE_PRINT ("----- STOP: WRITE CUSTOMIZATION DATA -----\r\n");
}

//-------------------------------------------------------------------

void DirectoryDialog::OnButtonDebugDumpAll ()
{
	CONSOLE_PRINT ("----- START: DEBUG DUMP -----\r\n");

	if (m_treeCtrl.GetSelectedItem () && m_treeCtrl.ItemHasChildren (m_treeCtrl.GetSelectedItem ()))
	{
		HTREEITEM selection = m_treeCtrl.GetSelectedItem ();

		openItem (m_treeCtrl.GetChildItem (m_treeCtrl.GetSelectedItem ()), false, false, true);

		m_treeCtrl.SelectItem (selection);
		m_treeCtrl.EnsureVisible (selection);
	}

	CONSOLE_PRINT ("----- STOP: END DEBUG DUMP -----\r\n");
}

//-------------------------------------------------------------------

void DirectoryDialog::OnItemexpandingDirectoryTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	const HTREEITEM item       = pNMTreeView->itemNew.hItem;
	const HTREEITEM parentItem = m_treeCtrl.GetParentItem (item);
	const HTREEITEM childItem  = m_treeCtrl.GetChildItem (item);

	if (parentItem == 0)
	{
		const CString childItemText = m_treeCtrl.GetItemText (childItem);
		if (childItemText == "__empty")
		{
			m_treeCtrl.DeleteItem (childItem);

			const CString itemText = m_treeCtrl.GetItemText (item);
			addDirectory (itemText, m_treeCtrl, item);

			//--
			if (!m_treeCtrl.ItemHasChildren (item))
				m_treeCtrl.DeleteItem (item);
		}

		Invalidate ();
	}

	*pResult = 0;
}

//-------------------------------------------------------------------

bool DirectoryDialog::openMeshItem(DialogProgress * const dialog, int & current, int const maximum, CString const & parentPath, HTREEITEM root, bool const detectNonCollidableShaderTemplates) const
{
	while (root)
	{
		//-- Build our path.
		const CString string = m_treeCtrl.GetItemText (root);
		CString const path = parentPath + '/' + string;

		if (m_treeCtrl.ItemHasChildren (root))
		{
			if (!openMeshItem(dialog, current, maximum, path, m_treeCtrl.GetChildItem(root), detectNonCollidableShaderTemplates))
				return false;
		}

		if (string.Find (".msh") != -1)
		{
			if (!detectNonCollidableShaderTemplates)
				CONSOLE_PRINT (CString ("opening ") + string + CString ("\r\n"));

			AppearanceTemplate const * const appearanceTemplate = AppearanceTemplateList::fetch (path);
			if (appearanceTemplate)
			{
#ifdef _DEBUG
				if (detectNonCollidableShaderTemplates)
				{
					MeshAppearanceTemplate const * const meshAppearanceTemplate = dynamic_cast<MeshAppearanceTemplate const *> (appearanceTemplate);
					DEBUG_WARNING (meshAppearanceTemplate && meshAppearanceTemplate->hasOnlyNonCollidableShaderTemplates (), ("%s has only noncollidable shader templates", string));
				}
#endif

				AppearanceTemplateList::release (appearanceTemplate);
			}
		}
		else if (string.Find (".mgn") != -1)
		{
			if (!detectNonCollidableShaderTemplates)
				CONSOLE_PRINT (CString ("opening ") + string + CString ("\r\n"));

			MeshGeneratorTemplate const *const mgTemplate = MeshGeneratorTemplateList::fetch(TemporaryCrcString(path, true));
			if (mgTemplate)
			{
				DEBUG_WARNING (detectNonCollidableShaderTemplates && mgTemplate->hasOnlyNonCollidableShaderTemplates (), ("%s has only noncollidable shader templates", string));
				mgTemplate->release();
			}
		}

		if (dialog)
		{
			if (++current > maximum)
			{
				current = 0;
				dialog->StepIt();
			}
			
			if (dialog->CheckCancelButton())
				return false;
		}

		root = m_treeCtrl.GetNextSiblingItem (root);
	}

	return true;
}

//-------------------------------------------------------------------

void DirectoryDialog::OnButtonBuildAsynchronousLoaderData () 
{
	if (ConfigFile::getKeyBool ("Viewer", "detectNonCollidableShaderTemplates", false))
	{
		CONSOLE_PRINT ("----- START: DETECTING ALPHA ONLY -----\r\n");

		bool const verboseWarnings = ConfigSharedFoundation::getVerboseWarnings ();
		ConfigSharedFoundation::setVerboseWarnings (false);
		CViewerView::setNoRender (true);

		HTREEITEM root = m_treeCtrl.GetRootItem ();

		while (root)
		{
			const CString string = m_treeCtrl.GetItemText (root);
			CONSOLE_PRINT ("searchPath: ");
			CONSOLE_PRINT (string);
			CONSOLE_PRINT ("\r\n");

			int current = 0;
			openMeshItem (0, current, 0, string, m_treeCtrl.GetChildItem (root), true);

			root = m_treeCtrl.GetNextSiblingItem (root);
		}

		CViewerView::setNoRender (false);
		ConfigSharedFoundation::setVerboseWarnings (verboseWarnings);

		CONSOLE_PRINT ("----- STOP: DETECTING ALPHA ONLY -----\r\n");
	}
	else
	{
		CONSOLE_PRINT ("----- START: BUILD ASYNCHRONOUS LOADER DATA -----\r\n");

#ifdef _DEBUG
		const bool logTreeFileOpens = TreeFile::isLoggingFiles ();
		TreeFile::setLogTreeFileOpens (true);
#endif

		bool const verboseWarnings = ConfigSharedFoundation::getVerboseWarnings ();
		ConfigSharedFoundation::setVerboseWarnings (false);
		CViewerDoc::setBuildingAsynchronousLoaderData(true);
		CViewerView::setNoRender (true);
		MeshAppearanceTemplate::setBuildingAsynchronousLoaderData(true);
		ShaderPrimitiveSetTemplate::setBuildingAsynchronousLoaderData(true);
		TextureList::setBuildingAsynchronousLoaderData (true);

		DialogProgress * const dialog = new DialogProgress();
		if (m_treeCtrl.GetCount())
		{
			dialog->Create();
			dialog->SetRange(0, 100);
			dialog->SetStep(1);
			dialog->SetPos(0);
			dialog->OffsetPos(0);
		}

		int current = 0;
		int const maximum = m_treeCtrl.GetCount() / 100;

		HTREEITEM root = m_treeCtrl.GetRootItem ();

		while (root)
		{
			const CString string = m_treeCtrl.GetItemText (root);
			CONSOLE_PRINT ("searchPath: ");
			CONSOLE_PRINT (string);
			CONSOLE_PRINT ("\r\n");

			if (!openMeshItem(dialog, current, maximum, string, m_treeCtrl.GetChildItem(root), false))
				break;

			if (++current > maximum)
			{
				current = 0;
				dialog->StepIt();
			}
				
			if (dialog->CheckCancelButton())
				break;

			root = m_treeCtrl.GetNextSiblingItem (root);
		}

		delete dialog;

		CViewerDoc::setBuildingAsynchronousLoaderData(false);
		CViewerView::setNoRender (false);
		ConfigSharedFoundation::setVerboseWarnings (verboseWarnings);
		MeshAppearanceTemplate::setBuildingAsynchronousLoaderData(false);
		ShaderPrimitiveSetTemplate::setBuildingAsynchronousLoaderData(false);
		TextureList::setBuildingAsynchronousLoaderData (false);

#ifdef _DEBUG
		TreeFile::setLogTreeFileOpens (logTreeFileOpens);
#endif

		CONSOLE_PRINT ("----- STOP: BUILD ASYNCHRONOUS LOADER DATA -----\r\n");
	}
}

// ----------------------------------------------------------------------

void DirectoryDialog::checkForLmg (CString const &parentPath, HTREEITEM root) 
{
	while (root)
	{
		//-- Build our path.
		CString const path = parentPath + '/' + m_treeCtrl.GetItemText (root);

		//-- Process children.
		if (m_treeCtrl.ItemHasChildren (root))
			checkForLmg (path, m_treeCtrl.GetChildItem (root));

		//-- Check if this is an MGN.
		if (path.Find (".mgn") != -1)
		{
			//-- Load and check first form entry.
			Iff  iff;

			if (iff.open(path, true))
			{
				if (iff.getCurrentName() == TAG_MLOD)
				{
					// Found one.
					CONSOLE_PRINT (path);
				}
			}
		}

		root = m_treeCtrl.GetNextSiblingItem (root);
	}
}


//-------------------------------------------------------------------
/**
 * Find all MGN files that are LodMeshGeneratorTemplate files.
 */

void DirectoryDialog::OnButtonFindLmgs() 
{
	CONSOLE_PRINT ("----- START: FIND LMG FILES -----\r\n");

	HTREEITEM root = m_treeCtrl.GetRootItem ();

	while (root)
	{
		const CString string = m_treeCtrl.GetItemText (root);
		checkForLmg (string, m_treeCtrl.GetChildItem (root));

		root = m_treeCtrl.GetNextSiblingItem (root);
	}

	CONSOLE_PRINT ("----- STOP: FIND LMG FILES -----\r\n");
}

// ----------------------------------------------------------------------

static CString const cs_fixBaseDir("c:\\exportedSatFix\\");

void DirectoryDialog::fixExportedSatFiles (CString const &parentPath, HTREEITEM root) 
{
	while (root)
	{
		//-- Build our path.
		CString const path = parentPath + '/' + m_treeCtrl.GetItemText (root);

		//-- Process children.
		if (m_treeCtrl.ItemHasChildren (root))
			fixExportedSatFiles (path, m_treeCtrl.GetChildItem (root));

		//-- Check if this is an MGN.
		if ((path.Find (".sat") != -1) && (path.Find ("exported") != -1))
		{
			AppearanceTemplate const *const baseAppearanceTemplate = AppearanceTemplateList::fetch(path);
			if (baseAppearanceTemplate)
			{
				SkeletalAppearanceTemplate const *const skeletalAppearanceTemplate = dynamic_cast<SkeletalAppearanceTemplate const*>(baseAppearanceTemplate);
				if (skeletalAppearanceTemplate)
				{
#ifdef _DEBUG
					if (const_cast<SkeletalAppearanceTemplate*>(skeletalAppearanceTemplate)->renameMgnToLmg())
					{
						// Build destination path.
						CString fixPath = cs_fixBaseDir + m_treeCtrl.GetItemText (root);

						// Construct the IFF
						Iff iff(128 * 1024);
						skeletalAppearanceTemplate->write(iff);

						// Write the IFF
						if (iff.write(fixPath, true))
						{
							char consoleOutput[4 * MAX_PATH];

							sprintf(consoleOutput, "%s,%s\n", static_cast<char const*>(path), static_cast<char const*>(fixPath));
							CONSOLE_PRINT (consoleOutput);
						}
						else
						{
							REPORT_LOG(true, ("FIXUP WRITE FAILED: [%s].\n", static_cast<char const*>(fixPath)));
						}
					}
#endif
				}

				AppearanceTemplateList::release(baseAppearanceTemplate);
			}
		}

		root = m_treeCtrl.GetNextSiblingItem (root);
	}
}

// ----------------------------------------------------------------------

void DirectoryDialog::OnButtonFixExportedSatFiles()
{
	CONSOLE_PRINT ("----- START: FIX EXPORTED SAT FILES -----\r\n");

	HTREEITEM root = m_treeCtrl.GetRootItem ();

	while (root)
	{
		const CString string = m_treeCtrl.GetItemText (root);
		fixExportedSatFiles (string, m_treeCtrl.GetChildItem (root));

		root = m_treeCtrl.GetNextSiblingItem (root);
	}

	CONSOLE_PRINT ("----- STOP: FIX EXPORTED SAT FILES -----\r\n");
}

// ==================================================================
