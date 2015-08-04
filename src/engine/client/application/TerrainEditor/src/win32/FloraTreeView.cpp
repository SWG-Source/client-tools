//
// FloraTreeView.cpp
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FloraTreeView.h"

#include "DialogResetFloraParameters.h"
#include "FindFrame.h"
#include "FloraGroupFrame.h"
#include "FormFlora.h"
#include "FormFloraFamily.h"
#include "RecentDirectory.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"
#include "TerrainGeneratorHelper.h"

//-------------------------------------------------------------------

static inline bool keyDown (int key)
{
	return (GetKeyState (key) & 0x8000) != 0;
}

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FloraTreeView, CTreeView)

//-------------------------------------------------------------------

FloraTreeView::FloraTreeView (void) :
	CTreeView (),
	floraGroup (0),
	imageListSet (false),
	imageList (),
	deletingUnused (false),
	m_dragMode (DM_nothing),
	m_pDragImage (0),
	m_htiDrag (0), 
	m_htiDrop (0), 
	m_htiOldDrop (0),
	m_bLDragging (false),
	m_idTimer (0),
	m_moveCursor (0),
	m_copyCursor (0)
{
}

//-------------------------------------------------------------------
	
FloraTreeView::~FloraTreeView (void)
{
	floraGroup   = 0;
	m_pDragImage = 0;
	m_htiDrag    = 0; 
	m_htiDrop    = 0; 
	m_htiOldDrop = 0;
	m_moveCursor = 0;
	m_copyCursor = 0;
}

//-------------------------------------------------------------------

//lint -save -e1924 -e648

BEGIN_MESSAGE_MAP(FloraTreeView, CTreeView)
	//{{AFX_MSG_MAP(FloraTreeView)
	ON_COMMAND(ID_BUTTON_FLORAGROUP_FINDTARGET, OnButtonFloragroupFindtarget)
	ON_COMMAND(ID_BUTTON_FLORAGROUP_NEWCHILD, OnButtonFloragroupNewchild)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_FLORAGROUP_NEWCHILD, OnUpdateButtonFloragroupNewchild)
	ON_COMMAND(ID_BUTTON_FLORAGROUP_NEWFAMILY, OnButtonFloragroupNewfamily)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_FLORAGROUP_NEWFAMILY, OnUpdateButtonFloragroupNewfamily)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_FLORAGROUP_FINDTARGET, OnUpdateButtonFloragroupFindtarget)
	ON_COMMAND(ID_BUTTON_FLORAGROUP_DELETEFAMILY, OnButtonFloragroupDelete)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_FLORAGROUP_DELETEFAMILY, OnUpdateButtonFloragroupDelete)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginlabeledit)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndlabeledit)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeydown)
	ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_GROUP_DELETE, OnGroupDelete)
	ON_COMMAND(ID_GROUP_RENAME, OnGroupRename)
	ON_COMMAND(ID_BUTTON_DELETEUNUSED, OnButtonDeleteunused)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBegindrag)
	ON_WM_DESTROY()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_COMMAND(ID_BUTTON_SWAYALL, OnButtonSwayall)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SWAYALL, OnUpdateButtonSwayall)
	ON_COMMAND(ID_BUTTON_SWAYFAMILY, OnButtonSwayfamily)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SWAYFAMILY, OnUpdateButtonSwayfamily)
	ON_COMMAND(ID_GROUP_FINDRULESUSINGTHISFAMILY, OnGroupFindrulesusingthisfamily)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

void FloraTreeView::OnDraw(CDC* pDC)
{
	UNREF (pDC);

	CDocument* pDoc = GetDocument();
	UNREF (pDoc);
	// TODO: add draw code here
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void FloraTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void FloraTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FloraTreeView::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();
	
	if (!imageListSet)
	{
		IGNORE_RETURN (imageList.Create (IDB_BITMAP_FAMILY, 16, 1, RGB (255,255,255)));
		IGNORE_RETURN (GetTreeCtrl ().SetImageList (&imageList, TVSIL_NORMAL));

		imageListSet = true;
	}
	
	//-- get the document's flora group
	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	floraGroup = &doc->getTerrainGenerator ()->getFloraGroup ();
	NOT_NULL (floraGroup);

	reset ();
}

//-------------------------------------------------------------------

void FloraTreeView::expandBranch (HTREEITEM item)
{
	//-- recursuvely expand each branch of the tree
	if (GetTreeCtrl ().ItemHasChildren (item))
	{
		IGNORE_RETURN (GetTreeCtrl ().Expand (item, TVE_EXPAND));
		item = GetTreeCtrl ().GetChildItem (item);

		do
		{
			expandBranch (item);
		}
		while ((item = GetTreeCtrl ().GetNextSiblingItem (item)) != 0);
	}
}

//-------------------------------------------------------------------

void FloraTreeView::expandAll (void) 
{
	HTREEITEM item = GetTreeCtrl ().GetRootItem();

	if (item)
	{
		do
		{
			expandBranch (item);
		}
		while ((item = GetTreeCtrl ().GetNextSiblingItem (item)) != 0);
	}
}

//-------------------------------------------------------------------

BOOL FloraTreeView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class
	cs.style |= (TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_EDITLABELS);
	
	return CTreeView::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

bool FloraTreeView::isFamily (HTREEITEM selection) const
{
	//-- am i pointing to a family? (families have no root)
	const HTREEITEM parent = GetTreeCtrl ().GetParentItem (selection);

	return parent == 0;
}

//-------------------------------------------------------------------

HTREEITEM FloraTreeView::getSelectedFamily (void) const
{
	//-- am i pointing to anything?
	const HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	return isFamily (selection) ? selection : 0;
}

//-------------------------------------------------------------------

bool FloraTreeView::isFamilySelected (void) const
{
	return getSelectedFamily () != 0;
}

//-------------------------------------------------------------------

bool FloraTreeView::isChild (HTREEITEM selection) const
{
	//-- am i pointing to a family? (families have no root)
	const HTREEITEM parent        = GetTreeCtrl ().GetParentItem (selection);
	const HTREEITEM parentsParent = GetTreeCtrl ().GetParentItem (parent);

	return (parent && !parentsParent);
}

//-------------------------------------------------------------------

HTREEITEM FloraTreeView::getSelectedChild (void) const
{
	//-- am i pointing to anything?
	const HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	return isChild (selection) ? selection : 0;
}

//-------------------------------------------------------------------

bool FloraTreeView::isChildSelected (void) const
{
	return getSelectedChild () != 0;
}

//-------------------------------------------------------------------

void FloraTreeView::OnButtonFloragroupFindtarget() 
{
	//-- find mesh name
	if (isChildSelected ())
	{
		CFileDialog dlg (true, "*.*", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR));
		dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("Appearance_Flora");
		dlg.m_ofn.lpstrTitle      = "Select Appearance File";
    
		if (dlg.DoModal () == IDOK)
		{
			//-- get child
			const HTREEITEM child = getSelectedChild ();

			if (child)
			{
				IGNORE_RETURN (RecentDirectory::update ("Appearance_Flora", dlg.GetPathName ()));

				//-- get old name
				const CString oldName = GetTreeCtrl ().GetItemText (child);

				//-- get new name
				const CString temp = dlg.GetPathName ();

				//-- find where mesh starts and only copy from then on
				int offset = temp.Find ("appearance\\");

				char nameBuffer [1000];
				strcpy (nameBuffer, temp);

				const char* newName = nameBuffer + offset + istrlen ("appearance\\");

				//-- rename in flora group
				const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (GetTreeCtrl ().GetParentItem (child)));

				//-- refresh the property page because it sends changes back via child name.
				//-- Remove current flora form this BEFORE name changes so current form values can be set.
				TerrainEditorDoc* pDoc = safe_cast<TerrainEditorDoc*> (GetDocument());
				PropertyView::ViewData dummyData;
				pDoc->SetPropertyView (RUNTIME_CLASS(PropertyView), &dummyData);

				NOT_NULL (floraGroup);
				floraGroup->renameChild (familyId, oldName, newName);

				//-- add the new flora form now that the new child name has been installed.
				FormFlora::FormFloraViewData  vd;
				vd.familyId            = familyId;
				vd.floraGroup          = floraGroup;
				vd.childName           = newName;
				pDoc->SetPropertyView (RUNTIME_CLASS(FormFlora), &vd);

				//-- rename in tree
				IGNORE_RETURN (GetTreeCtrl ().SetItemText (child, newName));

				//-- tell the mesh view about it
				updateMeshView ();

				//-- prompt for save on exit
				pDoc->SetModifiedFlag ();

				Invalidate ();
			}
		}
	}
}

//-------------------------------------------------------------------

void FloraTreeView::OnUpdateButtonFloragroupFindtarget(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (isChildSelected ());
}

//-------------------------------------------------------------------

void FloraTreeView::OnButtonFloragroupNewchild() 
{
	//-- find HTREEITEM of family
	HTREEITEM family = getSelectedFamily ();

	if (!family)
	{
		//-- family is not selected, so see if child is
		family = getSelectedChild ();

		//-- get child's parent
		family = GetTreeCtrl ().GetParentItem (family);
	}

	if (family)
	{
		NOT_NULL (floraGroup);
		const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (family));

		CString childName;
		childName.Format ("Child_%i", floraGroup->getFamilyNumberOfChildren (familyId));

		//-- we now have the family
		FloraGroup::FamilyChildData fcd;
		fcd.appearanceTemplateName = childName;
		fcd.familyId               = familyId;
		fcd.weight                 = CONST_REAL (1);
		fcd.shouldSway             = false;
		fcd.displacement           = 0.01f;
		fcd.period                 = 0.1f;
		fcd.alignToTerrain         = false;

		floraGroup->addChild (fcd);

		const HTREEITEM child = GetTreeCtrl ().InsertItem (fcd.appearanceTemplateName, 2, 3, family);

		//-- set the child data to the family id for easy searching later
		IGNORE_RETURN (GetTreeCtrl ().SetItemData (child, static_cast<DWORD> (fcd.familyId)));

		IGNORE_RETURN (GetTreeCtrl ().SelectItem (child));

		GetDocument ()->SetModifiedFlag ();

		Invalidate ();
	}
}

//-------------------------------------------------------------------

void FloraTreeView::OnUpdateButtonFloragroupNewchild(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ((isChildSelected () || isFamilySelected ()) ? TRUE : FALSE);
}

//-------------------------------------------------------------------

static int getUniqueFamilyId (const FloraGroup* floraGroup)
{
	int familyId = 1;

	int numberOfTries = 0;
	while (numberOfTries++ < 1000)
	{
		bool found = false;

		//-- search for id
		int familyIndex;
		for (familyIndex = 0; familyIndex < floraGroup->getNumberOfFamilies (); familyIndex++)
			if (floraGroup->getFamilyId (familyIndex) == familyId)
			{
				found = true;
				break;
			}

		//-- not found, so return it
		if (!found)
			return familyId;

		//-- bump up the family id since it's in use
		familyId++;
	}

	//-- just return the default
	FATAL (true, ("couldn't find any unique flora ids after 1000 tries"));

	return 0;  //lint !e527  //-- unreachable
}

//-------------------------------------------------------------------

bool FloraTreeView::childExistsInFamily (int familyId, const char* childName) const
{
	NOT_NULL (floraGroup);

	const int n = floraGroup->getFamilyNumberOfChildren (familyId);

	int i;
	for (i = 0; i < n; ++i)
	{
		const FloraGroup::FamilyChildData fcd = floraGroup->getFamilyChild (familyId, i);

		if (_stricmp (childName, fcd.appearanceTemplateName) == 0)
			return true;
	}

	return false;
}

//-------------------------------------------------------------------

bool FloraTreeView::familyExists (const char* familyName) const
{
	NOT_NULL (floraGroup);

	const int n = floraGroup->getNumberOfFamilies ();

	int i;
	for (i = 0; i < n; ++i)
	{
		const int familyId = floraGroup->getFamilyId (i);

		if (_stricmp (familyName, floraGroup->getFamilyName (familyId)) == 0)
			return true;
	}

	return false;
}

//-------------------------------------------------------------------

const CString FloraTreeView::CreateUniqueFamilyName (const CString* base) const
{
	CString familyName;
	int     familyIndex = 0;

	if (base)
	{
		familyName.Format ("%s", static_cast<const char*> (*base));
	}
	else
	{
		NOT_NULL (floraGroup);
		familyIndex = floraGroup->getNumberOfFamilies ();

		familyName.Format ("Family_%i", familyIndex);
	}

	while (familyExists (familyName))
	{
		if (base)
		{
			familyName.Format ("%s_%i", static_cast<const char*> (*base), familyIndex);
		}
		else
		{
			familyName.Format ("Family_%i", familyIndex);
		}

		familyIndex++;

		FATAL (familyIndex > 10000, ("Couldn't find any new unique family names after 10000 tries"));
	}

	return familyName;
}

//-------------------------------------------------------------------

void FloraTreeView::addFamily (const CString& familyName, const ArrayList<CString*>& children)
{
	NOT_NULL (floraGroup);

	//-- create new family
	const int familyId = getUniqueFamilyId (floraGroup);

	CString newFamilyName = CreateUniqueFamilyName (&familyName);
	floraGroup->addFamily (familyId, newFamilyName, PackedRgb::solidYellow);

	//-- insert into tree
	const HTREEITEM family = GetTreeCtrl ().InsertItem (newFamilyName, 0, 1);
	IGNORE_RETURN (GetTreeCtrl ().SetItemData (family, static_cast<DWORD> (familyId)));

	int i;
	for (i = 0; i < children.getNumberOfElements (); i++)
	{
		//-- we now have the family
		FloraGroup::FamilyChildData fcd;
		fcd.appearanceTemplateName = (LPCSTR) *children [i];  //lint !e1924  //-- c-style cast
		fcd.familyId               = familyId;
		fcd.weight                 = CONST_REAL (1);
		fcd.shouldSway             = false;
		fcd.displacement           = 0.01f;
		fcd.period                 = 0.1f;
		fcd.alignToTerrain         = false;

		floraGroup->addChild (fcd);

		const HTREEITEM child = GetTreeCtrl ().InsertItem (fcd.appearanceTemplateName, 2, 3, family);

		//-- set the child data to the family id for easy searching later
		IGNORE_RETURN (GetTreeCtrl ().SetItemData (child, static_cast<DWORD> (fcd.familyId)));
	}

	IGNORE_RETURN (GetTreeCtrl ().SelectItem (family));
	GetDocument ()->SetModifiedFlag ();

	Invalidate ();
}

//-------------------------------------------------------------------

void FloraTreeView::OnButtonFloragroupNewfamily() 
{
	NOT_NULL (floraGroup);

	//-- create new family
	const int familyId = getUniqueFamilyId (floraGroup);
	
	CString familyName = CreateUniqueFamilyName ();
	floraGroup->addFamily (familyId, familyName, PackedRgb::solidGreen);

	//-- insert into tree
	const HTREEITEM family = GetTreeCtrl ().InsertItem (familyName, 0, 1);
	IGNORE_RETURN (GetTreeCtrl ().SetItemData (family, static_cast<DWORD> (familyId)));

	IGNORE_RETURN (GetTreeCtrl ().SelectItem (family));

	OnButtonFloragroupNewchild ();

	GetDocument ()->SetModifiedFlag ();

	Invalidate ();
}

//-------------------------------------------------------------------

void FloraTreeView::OnUpdateButtonFloragroupNewfamily(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (true);
}

//-------------------------------------------------------------------

void FloraTreeView::OnButtonFloragroupDelete() 
{
	NOT_NULL (floraGroup);

	HTREEITEM family = getSelectedFamily ();

	if (family)
	{
		const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (family));
		const char* name   = floraGroup->getFamilyName (familyId);

		TerrainEditorDoc* const pDoc = safe_cast<TerrainEditorDoc*> (GetDocument());
		if (TerrainGeneratorHelper::usesFloraFamily (pDoc->getTerrainGenerator (), familyId))
		{
			CString tmp;
			tmp.Format ("%s is still in use", name);
			IGNORE_RETURN (MessageBox (tmp));

			return;
		}
		
		CString tmp;
		tmp.Format ("Are you sure you want to delete %s?", name);

		if (MessageBox (tmp, 0, MB_YESNO) == IDYES)
		{
			// set property view to default dialog before destroying data in dlg
			PropertyView::ViewData dummyData;
			pDoc->SetPropertyView (RUNTIME_CLASS(PropertyView), &dummyData);

			//-- remove from flora group
			floraGroup->removeFamily (familyId);

			//-- delete from tree
			IGNORE_RETURN (GetTreeCtrl ().DeleteItem (family));

			GetDocument ()->SetModifiedFlag ();

			Invalidate ();
		}
	}
	else
	{
		//-- deleting a child
		const HTREEITEM child = getSelectedChild ();

		if (child)
		{
			//-- remove from flora group
			const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (GetTreeCtrl ().GetParentItem (child)));

			if (floraGroup->getFamilyNumberOfChildren (familyId) > 1)
			{
				const CString name = GetTreeCtrl ().GetItemText (child);

				CString tmp;
				tmp.Format ("Are you sure you want to delete %s?", static_cast<const char*> (name));

				if (MessageBox (tmp, 0, MB_YESNO) == IDYES)
				{
					FloraGroup::FamilyChildData fcd;
					fcd.appearanceTemplateName = name;
					fcd.familyId               = familyId;

					// set property view to default dialog before destroying data in dlg
					TerrainEditorDoc* pDoc = safe_cast<TerrainEditorDoc*> (GetDocument());
					PropertyView::ViewData dummyData;
					pDoc->SetPropertyView (RUNTIME_CLASS(PropertyView), &dummyData);

					floraGroup->removeChild (fcd);

					//-- delete from tree
					IGNORE_RETURN (GetTreeCtrl ().DeleteItem (child));

					GetDocument ()->SetModifiedFlag ();

					Invalidate ();
				}
			}
			else
				IGNORE_RETURN (MessageBox ("You can't delete the only child! Remove the family instead, or add another child and then remove this one..."));	
		}
	}
}

//-------------------------------------------------------------------

void FloraTreeView::OnUpdateButtonFloragroupDelete(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ((isChildSelected () || isFamilySelected ()) ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void FloraTreeView::OnBeginlabeledit (NMHDR* pNMHDR, LRESULT* pResult) 
{
	//-- get the tree view display info
	const NMTVDISPINFO* pTVDispInfo = reinterpret_cast<const NMTVDISPINFO*> (pNMHDR);

	//-- disallow editing by default
	*pResult = 1;

	const HTREEITEM selection = pTVDispInfo->item.hItem;

	if (isFamily (selection))
	{
		//-- limit to 100 characters
		GetTreeCtrl ().GetEditControl()->LimitText (100);

		//-- don't edit
		*pResult = 0;
		return;
	}
}

//-------------------------------------------------------------------

void FloraTreeView::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//-- get the tree view display info
	NMTVDISPINFO* pTVDispInfo = reinterpret_cast<NMTVDISPINFO*> (pNMHDR);

	//-- get the new text string
	const char* newName = pTVDispInfo->item.pszText;

	if (!newName || istrlen (newName) == 0)
	{
		*pResult = 0;
		return;
	}

	NOT_NULL (floraGroup);

	//-- change name
	const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (pTVDispInfo->item.hItem));
	floraGroup->setFamilyName (familyId, newName);

	//-- tell tree about new name
	IGNORE_RETURN (GetTreeCtrl ().SetItem (&pTVDispInfo->item));

	*pResult = 1;

	//-- tell properties window about new name.
	TerrainEditorDoc* pDoc = safe_cast<TerrainEditorDoc*> (GetDocument());

	FormFloraFamily::FormFloraFamilyViewData vd;
	vd.familyId    = familyId;
	vd.floraGroup = floraGroup;

	pDoc->SetPropertyView (RUNTIME_CLASS(FormFloraFamily), &vd);
}

//-------------------------------------------------------------------

void FloraTreeView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREF (pNMHDR);

	if (deletingUnused)
		return;

	if (isFamilySelected ())
	{
		TerrainEditorDoc* pDoc = safe_cast<TerrainEditorDoc*> (GetDocument());
		HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

		FormFloraFamily::FormFloraFamilyViewData vd;
		vd.familyId    = static_cast<int> (GetTreeCtrl ().GetItemData (selection));
		vd.floraGroup  = floraGroup;

		pDoc->SetPropertyView (RUNTIME_CLASS(FormFloraFamily), &vd);
	}
	else if (isChildSelected ())
	{
		TerrainEditorDoc* pDoc = safe_cast<TerrainEditorDoc*> (GetDocument());
		HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();
		
		FormFlora::FormFloraViewData vd;
		vd.familyId   = static_cast<int> (GetTreeCtrl ().GetItemData (selection));
		vd.floraGroup = floraGroup;
		vd.childName  = GetTreeCtrl ().GetItemText (selection);

		pDoc->SetPropertyView (RUNTIME_CLASS(FormFlora), &vd);
	}
	*pResult = 0;

	//-- show selection
	updateMeshView ();
}

//-------------------------------------------------------------------

void FloraTreeView::updateMeshView (void) const
{
	HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	//-- if it's a child tell the frame about it to route the message to FloraMeshView
	if (isChild (selection))
	{
		const CString name = GetTreeCtrl ().GetItemText (selection);

		safe_cast<FloraGroupFrame*> (GetParentFrame ())->setSelectedObject (name);
	}
	else
	{
		safe_cast<FloraGroupFrame*> (GetParentFrame ())->setSelectedObject (0);
	}
}

//-------------------------------------------------------------------


void FloraTreeView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREF (pNMHDR);

	*pResult = 0;

	//-- same as find target
	OnButtonFloragroupFindtarget ();
}

//-------------------------------------------------------------------

void FloraTreeView::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult) 
{
	const TV_KEYDOWN* pTVKeyDown = reinterpret_cast<const TV_KEYDOWN*> (pNMHDR);

	switch (pTVKeyDown->wVKey)
	{
	case VK_INSERT:
		{
			//-- same as new child
			OnButtonFloragroupNewchild ();
		}
		break;

	case VK_DELETE:
		{
			//-- same as delete
			OnButtonFloragroupDelete ();
		}
		break;

	default:
		break;
	}

	*pResult = 0;
}

//-------------------------------------------------------------------

void FloraTreeView::OnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnSelchanged (pNMHDR, pResult);

	*pResult = 0;
}

//-------------------------------------------------------------------

void FloraTreeView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if (isFamilySelected ())
	{
		//-- convert client coordinates to screen coordinates
		CPoint pt = point;
		ClientToScreen (&pt);

		CMenu menu;
		IGNORE_RETURN (menu.LoadMenu (IDR_GROUP_MENU));

		CMenu* subMenu = menu.GetSubMenu (0);
		IGNORE_RETURN (subMenu->TrackPopupMenu (TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, this));
	}
	
	CTreeView::OnRButtonDown(nFlags, point);
}  //lint !e1746  //-- point could have been a const reference

//-------------------------------------------------------------------

void FloraTreeView::OnGroupDelete() 
{
	OnButtonFloragroupDelete ();
}

//-------------------------------------------------------------------

void FloraTreeView::OnGroupRename() 
{
	IGNORE_RETURN (GetTreeCtrl ().EditLabel (GetTreeCtrl ().GetSelectedItem ()));
}

//-------------------------------------------------------------------

void FloraTreeView::reset (void)
{
	NOT_NULL (floraGroup);

	IGNORE_RETURN (GetTreeCtrl ().DeleteAllItems ());

	//-- populate tree
	int i;
	for (i = 0; i < floraGroup->getNumberOfFamilies (); i++)
	{
		const int familyId = floraGroup->getFamilyId (i);

		//-- get family name
		const HTREEITEM familyRoot = GetTreeCtrl ().InsertItem (floraGroup->getFamilyName (familyId),  0, 1);

		//-- set the family data to the family index for easy seatching later
		IGNORE_RETURN (GetTreeCtrl ().SetItemData (familyRoot, static_cast<DWORD> (familyId)));

		//-- insert children
		int j;
		for (j = 0; j < floraGroup->getNumberOfChildren (i); j++)
		{
			const FloraGroup::FamilyChildData fcd = floraGroup->getChild (i, j);
			DEBUG_FATAL (familyId != fcd.familyId, ("familyId != fcd.familyId (%i != %i)", familyId, fcd.familyId));

			//-- insert the child
			const HTREEITEM child = GetTreeCtrl ().InsertItem (fcd.appearanceTemplateName, 2, 3, familyRoot);

			//-- set the child data to the family id for easy searching later
			IGNORE_RETURN (GetTreeCtrl ().SetItemData (child, static_cast<DWORD> (fcd.familyId)));
		}
	}

	//-- expand the tree
	expandAll ();

	IGNORE_RETURN (GetTreeCtrl ().SelectItem (GetTreeCtrl ().GetRootItem ()));
	IGNORE_RETURN (GetTreeCtrl ().EnsureVisible (GetTreeCtrl ().GetRootItem()));
}

//-------------------------------------------------------------------

void FloraTreeView::OnButtonDeleteunused() 
{
	NOT_NULL (floraGroup);

	//--
	CString tmp;
	tmp.Format ("Are you sure you want to delete all unused families?");

	if (MessageBox (tmp, 0, MB_YESNO) == IDNO)
		return;

	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	// set property view to default dialog before destroying data in dlg
	PropertyView::ViewData dummyData;
	doc->SetPropertyView (RUNTIME_CLASS(PropertyView), &dummyData);

	TerrainGenerator* generator = doc->getTerrainGenerator ();
	NOT_NULL (generator);

	const int originalCount = floraGroup->getNumberOfFamilies ();
	int n = originalCount;
	int i = 0;

	while (i < n)
	{
		if (TerrainGeneratorHelper::usesFloraFamily (generator, floraGroup->getFamilyId (i)))
			i++;
		else
		{
			floraGroup->removeFamily (floraGroup->getFamilyId (i));
			n--;
		}
	}

	deletingUnused = true;
	reset ();
	deletingUnused = false;
}

//-------------------------------------------------------------------

void FloraTreeView::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = reinterpret_cast<NM_TREEVIEW*> (pNMHDR);

	m_htiDrag = pNMTreeView->itemNew.hItem;
	m_htiDrop = NULL;

	m_pDragImage = GetTreeCtrl().CreateDragImage( m_htiDrag );
	if( !m_pDragImage )
		return;

	if (keyDown (VK_CONTROL))
		m_dragMode = DM_copy;
	else
		m_dragMode = DM_move;

	CPoint pt(0,0);
	
	IMAGEINFO ii;
	IGNORE_RETURN (m_pDragImage->GetImageInfo( 0, &ii ));
	pt.x = (ii.rcImage.right - ii.rcImage.left) / 2;
	pt.y = (ii.rcImage.bottom - ii.rcImage.top) / 2;

	IGNORE_RETURN (m_pDragImage->BeginDrag( 0, pt ));
	pt = pNMTreeView->ptDrag;
	ClientToScreen( &pt );
	IGNORE_RETURN (CImageList::DragEnter(NULL,pt));
	
	SetCapture();

	*pResult = 0;
}

//-------------------------------------------------------------------

void FloraTreeView::OnLButtonUp (UINT nFlags, CPoint point)
{
	CTreeView::OnLButtonUp(nFlags, point);

	switch (m_dragMode)
	{
	case DM_move:
		OnLButtonUpForMove ();
		break;

	case DM_copy:
		OnLButtonUpForCopy ();
		break;

	case DM_nothing:
	default:
		break;
	}
}  //lint !e1746  //-- point could have been a const reference

//-------------------------------------------------------------------

HTREEITEM FloraTreeView::insertChild (HTREEITEM familyItem, int familyId, const CString& childName) const
{
	HTREEITEM child = GetTreeCtrl ().InsertItem (childName, 2, 3, familyItem);
	IGNORE_RETURN (GetTreeCtrl ().SetItemData (child, static_cast<DWORD> (familyId)));

	return child;
}

//-------------------------------------------------------------------

void FloraTreeView::OnLButtonUpForDrag (bool move)
{
	NOT_NULL (floraGroup);

	m_dragMode = DM_nothing;

	IGNORE_RETURN (CImageList::DragLeave (this));
	CImageList::EndDrag ();

	ReleaseCapture();

	delete m_pDragImage;

	IGNORE_RETURN (GetTreeCtrl ().SelectDropTarget (NULL));
	m_htiOldDrop = NULL;

	if (m_idTimer)
	{
		IGNORE_RETURN (KillTimer (static_cast<int> (m_idTimer)));
		m_idTimer = 0;
	}

	//-- exit out if no target was selected to drop on
	if (m_htiDrop == NULL)
		return;

	if (m_htiDrag == m_htiDrop)
		return;

	//-- if the drag item is not a child
	if (isFamily (m_htiDrag))
		return;

	//-- get the document
	TerrainEditorDoc* const doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	//-- get the source item
	const int sourceFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (m_htiDrag));

	//-- get the destination item
	const int destinationFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (m_htiDrop));

	//-- are the families equal?
	if (destinationFamilyId == sourceFamilyId)
		return;

	const CString sourceFamilyName = GetTreeCtrl ().GetItemText (m_htiDrag);

	//-- does the child already exist in the family?
	if (childExistsInFamily (destinationFamilyId, sourceFamilyName))
	{
		IGNORE_RETURN (MessageBox ("Selected child already exists in the family!"));
		return;
	}

	if (move && floraGroup->getFamilyNumberOfChildren (sourceFamilyId) == 1)
	{
		IGNORE_RETURN (MessageBox ("You can't move the last child, but you can copy it!"));
		return;
	}

	FloraGroup::FamilyChildData fcd = floraGroup->getFamilyChild (sourceFamilyId, sourceFamilyName);

	fcd.familyId = destinationFamilyId;
	floraGroup->addChild (fcd);

	const HTREEITEM child = insertChild (findFamily (GetTreeCtrl ().GetRootItem (), fcd.familyId), fcd.familyId, fcd.appearanceTemplateName);
	IGNORE_RETURN (GetTreeCtrl ().SelectItem (child));
	
	if (move)
	{
		IGNORE_RETURN (GetTreeCtrl ().DeleteItem (m_htiDrag));

		fcd.familyId = sourceFamilyId;
		floraGroup->removeChild (fcd);
	}

	doc->UpdateAllViews (0);
	doc->SetModifiedFlag ();
}

//-------------------------------------------------------------------

void FloraTreeView::OnLButtonUpForCopy ()
{
	OnLButtonUpForDrag (false);
}

//-------------------------------------------------------------------

void FloraTreeView::OnLButtonUpForMove () 
{
	OnLButtonUpForDrag (true);
}

//-------------------------------------------------------------------

void FloraTreeView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CTreeView::OnMouseMove(nFlags, point);

	HTREEITEM hti;
	UINT      flags;

	if (m_dragMode == DM_copy)
		SetCursor (m_copyCursor);

	if (m_dragMode == DM_move || m_dragMode == DM_copy)
	{
		CTreeCtrl& theTree = GetTreeCtrl();
		POINT pt = point;
		ClientToScreen( &pt );
		IGNORE_RETURN (CImageList::DragMove(pt));
		
		hti = theTree.HitTest(point,&flags);
		if( hti != NULL )
		{
			IGNORE_RETURN (CImageList::DragShowNolock(FALSE));

			if( m_htiOldDrop == NULL )
				m_htiOldDrop = theTree.GetDropHilightItem();

			IGNORE_RETURN (theTree.SelectDropTarget(hti));
			
			m_htiDrop = hti;
			
			if( m_idTimer && hti == m_htiOldDrop )
			{
				IGNORE_RETURN (KillTimer (static_cast<int> (m_idTimer)));
				m_idTimer = 0;
			}
			
			if( !m_idTimer )
				m_idTimer = SetTimer( 1000, 2000, NULL );

			IGNORE_RETURN (CImageList::DragShowNolock(TRUE));
		}
	}
}  //lint !e1746  //-- point could have been make a const reference

//-------------------------------------------------------------------

void FloraTreeView::OnDestroy() 
{
	if( m_idTimer )
	{
		IGNORE_RETURN (KillTimer (static_cast<int> (m_idTimer)));
		m_idTimer = 0;
	}

	CTreeView::OnDestroy();
}

//-------------------------------------------------------------------

void FloraTreeView::OnTimer(UINT nIDEvent) 
{
    if( nIDEvent == m_idTimer )
    {
        CTreeCtrl& theTree = GetTreeCtrl();
        HTREEITEM htiFloat = theTree.GetDropHilightItem();
        if( htiFloat && htiFloat == m_htiDrop )
        {
            if( theTree.ItemHasChildren( htiFloat ) )
                IGNORE_RETURN (theTree.Expand( htiFloat, TVE_EXPAND ));
        }
    }

    CTreeView::OnTimer(nIDEvent);
}

//-------------------------------------------------------------------

HTREEITEM FloraTreeView::findFamily (HTREEITEM hItem, int familyId) const
{
	HTREEITEM theItem = NULL;

	if (hItem == NULL) 
		return NULL;

	if ((theItem = GetTreeCtrl ().GetNextSiblingItem (hItem)) != NULL) 
	{
		theItem = findFamily (theItem, familyId);

		if (theItem != NULL) 
			return theItem;
	}

	if (static_cast<int> (GetTreeCtrl ().GetItemData (hItem)) == familyId)
		return hItem;

	return theItem;
}

//-------------------------------------------------------------------

void FloraTreeView::OnButtonSwayall() 
{
	DialogResetFloraParameters dlg (false);

	if (dlg.DoModal () == IDOK)
	{
		NOT_NULL (floraGroup);

		int i;
		for (i = 0; i < floraGroup->getNumberOfFamilies (); ++i)
		{
			int j;
			for (j = 0; j < floraGroup->getNumberOfChildren (i); ++j)
			{
				FloraGroup::FamilyChildData fcd = floraGroup->getChild (i, j);

				if (dlg.m_checkDisplacement)
					fcd.displacement = dlg.m_finalDisplacement;

				if (dlg.m_checkPeriod)
					fcd.period = dlg.m_finalPeriod;

				if (dlg.m_checkSway)
					fcd.shouldSway = dlg.m_shouldSway == TRUE;

				if (dlg.m_checkAlign)
					fcd.alignToTerrain = dlg.m_alignToTerrain == TRUE;

				floraGroup->setFamilyChild (fcd.familyId, fcd);
			}
		}

		GetDocument ()->UpdateAllViews (this);
	}
}

//-------------------------------------------------------------------

void FloraTreeView::OnUpdateButtonSwayall(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (isFamilySelected ());
}

//-------------------------------------------------------------------

void FloraTreeView::OnButtonSwayfamily() 
{
	DialogResetFloraParameters dlg (false);

	if (dlg.DoModal () == IDOK)
	{
		HTREEITEM family = getSelectedFamily ();

		if (family)
		{
			NOT_NULL (floraGroup);

			const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (family));

			int j;
			for (j = 0; j < floraGroup->getFamilyNumberOfChildren (familyId); ++j)
			{
				FloraGroup::FamilyChildData fcd = floraGroup->getFamilyChild (familyId, j);

				if (dlg.m_checkDisplacement)
					fcd.displacement = dlg.m_finalDisplacement;

				if (dlg.m_checkPeriod)
					fcd.period = dlg.m_finalPeriod;

				if (dlg.m_checkSway)
					fcd.shouldSway = dlg.m_shouldSway == TRUE;

				if (dlg.m_checkAlign)
					fcd.alignToTerrain = dlg.m_alignToTerrain == TRUE;

				floraGroup->setFamilyChild (familyId, fcd);
			}
		}

		GetDocument ()->UpdateAllViews (this);
	}
}

//-------------------------------------------------------------------

void FloraTreeView::OnUpdateButtonSwayfamily(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (isFamilySelected ());
}

//-------------------------------------------------------------------

void FloraTreeView::OnGroupFindrulesusingthisfamily() 
{
	//-- get family
	HTREEITEM family = getSelectedFamily ();

	if (!family)
	{
		//-- get family
		const HTREEITEM child = getSelectedChild ();

		family = GetTreeCtrl ().GetParentItem (child);
	}

	if (family)
	{
		NOT_NULL (floraGroup);

		PackedRgb color;

		//-- get familyId
		const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (family));

		TerrainEditorDoc* const doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
		NOT_NULL (doc);

		ArrayList<TerrainGeneratorHelper::OutputData> output;

		TerrainGeneratorHelper::findFlora (doc->getTerrainGenerator (), familyId, output);

		if (output.empty ())
		{
			CString string;
			string.Format ("Family %s not used", floraGroup->getFamilyName (familyId));

			IGNORE_RETURN (MessageBox (string));
		}
		else
		{
			GetApp ()->showFind (true);

			//--
			int i;
			for (i = 0; i < output.getNumberOfElements (); i++)
			{
				switch (output [i].mode)
				{
				case TerrainGeneratorHelper::OutputData::M_find:
					{
						if (doc->getFindFrame ())
							doc->getFindFrame ()->update (output [i]);
					}
					break;

				case TerrainGeneratorHelper::OutputData::M_console:
					{
						CONSOLE_PRINT (*output [i].message);
						CONSOLE_PRINT ("\r\n");
					}
					break;

				case TerrainGeneratorHelper::OutputData::M_warning:
				default:
					break;
				}

				delete output [i].message;
			}
		}
	}
}

//-------------------------------------------------------------------

