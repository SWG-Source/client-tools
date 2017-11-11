//
// RadialTreeView.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "RadialTreeView.h"

#include "DialogResetFloraParameters.h"
#include "FindFrame.h"
#include "FormRadial.h"
#include "FormRadialFamily.h"
#include "RadialGroupFrame.h"
#include "RecentDirectory.h"
#include "TerrainEditorDoc.h"
#include "TerrainGeneratorHelper.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "sharedUtility/FileName.h"
#include "terraineditor.h"

//-------------------------------------------------------------------

static inline bool keyDown (int key)
{
	return (GetKeyState (key) & 0x8000) != 0;
}

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(RadialTreeView, CTreeView)

//-------------------------------------------------------------------

RadialTreeView::RadialTreeView (void) :
	CTreeView (),
	radialGroup (0),
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
	
RadialTreeView::~RadialTreeView()
{
	radialGroup  = 0;
	m_pDragImage = 0;
	m_htiDrag    = 0; 
	m_htiDrop    = 0; 
	m_htiOldDrop = 0;
	m_moveCursor = 0;
	m_copyCursor = 0;
}

//-------------------------------------------------------------------

//lint -save -e1924 -e648

BEGIN_MESSAGE_MAP(RadialTreeView, CTreeView)
	//{{AFX_MSG_MAP(RadialTreeView)
	ON_COMMAND(ID_BUTTON_RADIALGROUP_NEWFAMILY, OnNewradialfamily)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_RADIALGROUP_NEWFAMILY, OnUpdateNewradialfamily)
	ON_COMMAND(ID_BUTTON_RADIALGROUP_NEWCHILD, OnNewradialchild)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_RADIALGROUP_NEWCHILD, OnUpdateNewradialchild)
	ON_COMMAND(ID_BUTTON_RADIALGROUP_DELETEFAMILY, OnDeleteradial)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_RADIALGROUP_DELETEFAMILY, OnUpdateDeleteradial)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_COMMAND(ID_BUTTON_RADIALGROUP_FINDTARGET, OnFindshader)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_RADIALGROUP_FINDTARGET, OnUpdateFindshader)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginlabeledit)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndlabeledit)
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeydown)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_COMMAND(ID_BUTTON_CALCULATECOLOR, OnButtonCalculatecolor)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_CALCULATECOLOR, OnUpdateButtonCalculatecolor)
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

void RadialTreeView::OnDraw(CDC* pDC)
{
	UNREF (pDC);
	CDocument* pDoc = GetDocument();
	UNREF (pDoc);
	// TODO: add draw code here
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void RadialTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void RadialTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

bool RadialTreeView::isFamily (HTREEITEM selection) const
{
	//-- am i pointing to a family? (families have no root)
	const HTREEITEM parent = GetTreeCtrl ().GetParentItem (selection);

	return parent == 0;
}

//-------------------------------------------------------------------

HTREEITEM RadialTreeView::getSelectedFamily (void) const
{
	//-- am i pointing to anything?
	const HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	return isFamily (selection) ? selection : 0;
}

//-------------------------------------------------------------------

bool RadialTreeView::isFamilySelected (void) const
{
	return getSelectedFamily () != 0;
}

//-------------------------------------------------------------------

bool RadialTreeView::isChild (HTREEITEM selection) const
{
	//-- am i pointing to a family? (families have no root)
	const HTREEITEM parent        = GetTreeCtrl ().GetParentItem (selection);
	const HTREEITEM parentsParent = GetTreeCtrl ().GetParentItem (parent);

	return (parent && !parentsParent);
}

//-------------------------------------------------------------------

HTREEITEM RadialTreeView::getSelectedChild (void) const
{
	//-- am i pointing to anything?
	const HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	return isChild (selection) ? selection : 0;
}

//-------------------------------------------------------------------

bool RadialTreeView::isChildSelected (void) const
{
	return getSelectedChild () != 0;
}

static int getUniqueFamilyId (RadialGroup* radialGroup)
{
	int familyId = 1;

	int numberOfTries = 0;
	while (numberOfTries++ < 1000)
	{
		bool found = false;

		//-- search for id
		int familyIndex;
		for (familyIndex = 0; familyIndex < radialGroup->getNumberOfFamilies (); familyIndex++)
			if (radialGroup->getFamilyId (familyIndex) == familyId)
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
	FATAL (true, ("couldn't find any unique ids after 1000 tries"));

	return 0;  //lint !e527  //-- unreachable
}

//-------------------------------------------------------------------

void RadialTreeView::addFamily (const CString& familyName, const ArrayList<CString*>& children)
{
	NOT_NULL (radialGroup);

	//-- create new family
	const int familyId = getUniqueFamilyId (radialGroup);

	CString newFamilyName = CreateUniqueFamilyName (&familyName);
	radialGroup->addFamily (familyId, newFamilyName, PackedRgb::solidYellow);

	//-- insert into tree
	const HTREEITEM family = GetTreeCtrl ().InsertItem (newFamilyName, 0, 1);
	IGNORE_RETURN (GetTreeCtrl ().SetItemData (family, static_cast<DWORD> (familyId)));

	int i;
	for (i = 0; i < children.getNumberOfElements (); i++)
	{
		//-- we now have the family
		RadialGroup::FamilyChildData fcd;
		fcd.shaderTemplateName     = (LPCSTR) *children [i];  //lint !e1924  //-- c-style cast
		fcd.familyId               = familyId;
		fcd.weight                 = CONST_REAL (1);
		fcd.distance               = CONST_REAL (10);
		fcd.minWidth               = CONST_REAL (1);
		fcd.maxWidth               = CONST_REAL (1);
		fcd.minHeight              = CONST_REAL (1);
		fcd.maxHeight              = CONST_REAL (1);
		fcd.shouldSway             = false;
		fcd.displacement           = 0.25f;
		fcd.period                 = 0.35f;
		fcd.maintainAspectRatio    = false;
		fcd.alignToTerrain         = false;
		fcd.createPlus             = false;

		radialGroup->addChild (fcd);

		const HTREEITEM child = GetTreeCtrl ().InsertItem (fcd.shaderTemplateName, 2, 3, family);

		//-- set the child data to the family id for easy searching later
		IGNORE_RETURN (GetTreeCtrl ().SetItemData (child, static_cast<DWORD> (fcd.familyId)));
	}

	IGNORE_RETURN (GetTreeCtrl ().SelectItem (family));
	GetDocument ()->SetModifiedFlag ();

	Invalidate ();
}

//-------------------------------------------------------------------

void RadialTreeView::OnNewradialfamily() 
{
	NOT_NULL (radialGroup);

	//-- create new family
	const int familyId = getUniqueFamilyId (radialGroup);
	
	CString familyName = CreateUniqueFamilyName ();
	radialGroup->addFamily (familyId, familyName, PackedRgb::solidYellow);

	//-- insert into tree
	const HTREEITEM family = GetTreeCtrl ().InsertItem (familyName, 0, 1);
	IGNORE_RETURN (GetTreeCtrl ().SetItemData (family, static_cast<DWORD> (familyId)));

	IGNORE_RETURN (GetTreeCtrl ().SelectItem (family));

	OnNewradialchild ();

	Invalidate ();
}

//-------------------------------------------------------------------

void RadialTreeView::OnUpdateNewradialfamily(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (true);
}

//-------------------------------------------------------------------

bool RadialTreeView::childExistsInFamily (int familyId, const char* childName) const
{
	NOT_NULL (radialGroup);

	const int n = radialGroup->getFamilyNumberOfChildren (familyId);

	int i;
	for (i = 0; i < n; ++i)
	{
		const RadialGroup::FamilyChildData fcd = radialGroup->getFamilyChild (familyId, i);

		if (_stricmp (childName, fcd.shaderTemplateName) == 0)
			return true;
	}

	return false;
}

//-------------------------------------------------------------------

bool RadialTreeView::familyExists (const char* familyName) const
{
	NOT_NULL (radialGroup);

	const int n = radialGroup->getNumberOfFamilies ();

	int i;
	for (i = 0; i < n; ++i)
	{
		const int familyId = radialGroup->getFamilyId (i);

		if (_stricmp (familyName, radialGroup->getFamilyName (familyId)) == 0)
			return true;
	}

	return false;
}

//-------------------------------------------------------------------

const CString RadialTreeView::CreateUniqueChildName (int familyId)
{
	NOT_NULL (radialGroup);

	CString childName;

	int lastChildIndex = radialGroup->getFamilyNumberOfChildren (familyId);

	childName.Format ("Child_%i", lastChildIndex);

	while (childExistsInFamily (familyId, childName))
	{
		lastChildIndex++;
		childName.Format ("Child_%i", lastChildIndex);
		FATAL (lastChildIndex > 1000, ("Couldn't find any new unique child names after 1000 tries"));
	}

	return childName;
}

//-------------------------------------------------------------------

const CString RadialTreeView::CreateUniqueFamilyName (const CString* base)
{
	NOT_NULL (radialGroup);

	CString familyName;
	int     familyIndex = 1;

	if (base)
	{
		familyName.Format ("%s", *base);
	}
	else
	{
		familyIndex = radialGroup->getNumberOfFamilies ();

		familyName.Format ("Family_%i", familyIndex);
	}

	while (familyExists (familyName))
	{
		if (base)
		{
			familyName.Format ("%s_%i", *base, familyIndex);
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

void RadialTreeView::OnNewradialchild() 
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
		NOT_NULL (radialGroup);

		const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (family));

		CString childName = CreateUniqueChildName (familyId);

		//-- we now have the family
		RadialGroup::FamilyChildData fcd;
		fcd.shaderTemplateName     = childName;
		fcd.familyId               = familyId;
		fcd.weight                 = CONST_REAL (1);
		fcd.distance               = CONST_REAL (10);
		fcd.minWidth               = CONST_REAL (1);
		fcd.maxWidth               = CONST_REAL (1);
		fcd.minHeight              = CONST_REAL (1);
		fcd.maxHeight              = CONST_REAL (1);
		fcd.shouldSway             = false;
		fcd.displacement           = 0.25f;
		fcd.period                 = 0.35f;
		fcd.alignToTerrain         = false;
		fcd.createPlus             = false;
		fcd.maintainAspectRatio    = false;

		radialGroup->addChild (fcd);

		const HTREEITEM child = GetTreeCtrl ().InsertItem (fcd.shaderTemplateName, 2, 3, family);

		//-- set the child data to the family id for easy searching later
		IGNORE_RETURN (GetTreeCtrl ().SetItemData (child, static_cast<DWORD> (fcd.familyId)));

		IGNORE_RETURN (GetTreeCtrl ().SelectItem (child));

		GetDocument ()->SetModifiedFlag ();

		Invalidate ();
	}
}

//-------------------------------------------------------------------

void RadialTreeView::OnUpdateNewradialchild(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ((isChildSelected () || isFamilySelected ()) ? TRUE : FALSE);
}

//-------------------------------------------------------------------

HTREEITEM RadialTreeView::insertFamily (int familyId, HTREEITEM& afterItem)
{
	NOT_NULL (radialGroup);

	const HTREEITEM familyRoot = GetTreeCtrl ().InsertItem (radialGroup->getFamilyName (familyId), 0, 1, TVI_ROOT, afterItem);

	//-- set the family data to the family index for easy searching later
	IGNORE_RETURN (GetTreeCtrl ().SetItemData (familyRoot, static_cast<DWORD> (familyId)));

	//-- insert children
	int j;
	for (j = 0; j < radialGroup->getFamilyNumberOfChildren (familyId); j++)
	{
		const RadialGroup::FamilyChildData fcd = radialGroup->getFamilyChild (familyId, j);
		DEBUG_FATAL (familyId != fcd.familyId, ("familyId != fcd.familyId (%i != %i)", familyId, fcd.familyId));

		//-- insert the child
		const HTREEITEM child = GetTreeCtrl ().InsertItem (fcd.shaderTemplateName, 2, 3, familyRoot);

		//-- set the child data to the family id for easy searching later
		IGNORE_RETURN (GetTreeCtrl ().SetItemData (child, static_cast<DWORD> (fcd.familyId)));
	}

	return familyRoot;
}

//-------------------------------------------------------------------

void RadialTreeView::OnDeleteradial() 
{
	HTREEITEM family = getSelectedFamily ();

	if (family)
	{
		NOT_NULL (radialGroup);

		const int familyId = GetTreeCtrl ().GetItemData (family);
		const char* name   = radialGroup->getFamilyName (familyId);

		TerrainEditorDoc* const pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
		if (TerrainGeneratorHelper::usesRadialFamily (pDoc->getTerrainGenerator (), familyId))
		{
			CString tmp;
			tmp.Format ("%s is still in use", name);
			MessageBox (tmp);

			return;
		}
		
		CString tmp;
		tmp.Format ("Are you sure you want to delete %s?", name);

		if (MessageBox (tmp, 0, MB_YESNO) == IDYES)
		{
			// set property view to default dialog before destroying data in dlg
			TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
			PropertyView::ViewData dummyData;
			pDoc->SetPropertyView (RUNTIME_CLASS(PropertyView), &dummyData);
			
			//-- remove from radial group
			radialGroup->removeFamily (familyId);

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
			NOT_NULL (radialGroup);

			//-- remove from radial group
			const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (GetTreeCtrl ().GetParentItem (child)));

			if (radialGroup->getFamilyNumberOfChildren (familyId) > 1)
			{
				const CString name = GetTreeCtrl ().GetItemText (child);

				CString tmp;
				tmp.Format ("Are you sure you want to delete %s?", name);

				if (MessageBox (tmp, 0, MB_YESNO) == IDYES)
				{
					RadialGroup::FamilyChildData fcd;
					fcd.shaderTemplateName     = name;
					fcd.familyId               = familyId;

					// set property view to default dialog before destroying data in dlg
					TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
					PropertyView::ViewData dummyData;
					pDoc->SetPropertyView (RUNTIME_CLASS(PropertyView), &dummyData);

					radialGroup->removeChild (fcd);

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

void RadialTreeView::OnUpdateDeleteradial(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ((isChildSelected () || isFamilySelected ()) ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void RadialTreeView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREF (pNMHDR);

	*pResult = 0;

	//-- same as find target
	OnFindshader ();
}

//-------------------------------------------------------------------

void RadialTreeView::OnFindshader() 
{
	//-- find shader template name
	if (isChildSelected ())
	{
		CFileDialog dlg (true, "*.sht", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR));
		dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("ShaderTemplate_Radial");
		dlg.m_ofn.lpstrTitle      = "Select Shader Template File";

		bool stillTrying = true;
		bool badName     = true;

		NOT_NULL (radialGroup);

		while (badName && stillTrying)
		{
			if (dlg.DoModal () == IDOK)
			{
				IGNORE_RETURN (RecentDirectory::update ("ShaderTemplate_Radial", dlg.GetPathName ()));

				//-- get new name
				const CString newName = dlg.GetFileTitle ();

				//-- get child
				const HTREEITEM child = getSelectedChild ();

				if (child)
				{
					const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (GetTreeCtrl ().GetParentItem (child)));

					if ( !childExistsInFamily (familyId, newName) )
					{
						badName = false;

						//-- get old name
						const CString oldName = GetTreeCtrl ().GetItemText (child);	

						//-- refresh the property page because it sends changes back via child name.
						//-- Remove current shader form this BEFORE name changes so current form values can be set.
						TerrainEditorDoc* const pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
						PropertyView::ViewData dummyData;
						pDoc->SetPropertyView (RUNTIME_CLASS(PropertyView), &dummyData);

						//-- rename in radial group
						radialGroup->renameChild (familyId, oldName, newName);

						//-- rename in tree
						IGNORE_RETURN (GetTreeCtrl ().SetItemText (child, newName));

						//-- tell the radial view about it
						updateRadialView ();

						GetDocument ()->SetModifiedFlag ();

						Invalidate ();

						//-- tell the properties view about it
						{
							HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();
							
							FormRadial::FormRadialViewData vd;
							vd.familyId = static_cast<int> (GetTreeCtrl ().GetItemData (selection));
							vd.radialGroup = radialGroup;
							vd.childName = GetTreeCtrl ().GetItemText (selection);

							pDoc->SetPropertyView (RUNTIME_CLASS(FormRadial), &vd);
						}

					}
					else
					{
						IGNORE_RETURN (MessageBox("A child with this name already exists.  Please select another name.", "Bad Name"));
					}
				}
			}
			else
				stillTrying = false;
		}
    
	}
}

//-------------------------------------------------------------------

void RadialTreeView::OnUpdateFindshader(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (isChildSelected ());
}

//-------------------------------------------------------------------

BOOL RadialTreeView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= (TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_EDITLABELS);
	
	return CTreeView::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

void RadialTreeView::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
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

void RadialTreeView::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
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

	if (!familyExists (newName))
	{
		NOT_NULL (radialGroup);

		//-- change name
		const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (pTVDispInfo->item.hItem));
		radialGroup->setFamilyName (familyId, newName);

		//-- tell tree about new name
		IGNORE_RETURN (GetTreeCtrl ().SetItem (&pTVDispInfo->item));
		*pResult = 1;

		//-- tell properties window about new name.
		{
			TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
			HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

			FormRadialFamily::FormRadialFamilyViewData vd;
			vd.familyId    = static_cast<int> (GetTreeCtrl ().GetItemData (selection));
			vd.radialGroup = radialGroup;

			pDoc->SetPropertyView (RUNTIME_CLASS(FormRadialFamily), &vd);
		}

	}
	else {
		IGNORE_RETURN (MessageBox("A family with this name already exists.  Please select another name.", "Bad Name"));
		*pResult = 0;
	}

}

//-------------------------------------------------------------------

void RadialTreeView::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult) 
{
	const TV_KEYDOWN* pTVKeyDown = reinterpret_cast<const TV_KEYDOWN*> (pNMHDR);

	switch (pTVKeyDown->wVKey)
	{
	case VK_INSERT:
		{
			//-- same as new child
			OnNewradialchild();
		}
		break;

	case VK_DELETE:
		{
			//-- same as delete
			OnDeleteradial();
		}
		break;

	default:
		break;
	}

	*pResult = 0;
}

//-------------------------------------------------------------------

void RadialTreeView::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();

	if (!imageListSet)
	{
		IGNORE_RETURN (imageList.Create (IDB_BITMAP_FAMILY, 16, 1, RGB (255,255,255)));
		IGNORE_RETURN (GetTreeCtrl ().SetImageList (&imageList, TVSIL_NORMAL));

		imageListSet = true;
	}
	
	//-- get the document's radial group
	TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	radialGroup = &doc->getTerrainGenerator ()->getRadialGroup ();
	NOT_NULL (radialGroup);

	reset ();
}

//-------------------------------------------------------------------

void RadialTreeView::expandBranch (HTREEITEM item)
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

void RadialTreeView::expandAll (void) 
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

void RadialTreeView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREF (pNMHDR);

	if (deletingUnused)
		return;

	if (isFamilySelected ())
	{
		TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
		HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

		FormRadialFamily::FormRadialFamilyViewData vd;
		vd.familyId    = static_cast<int> (GetTreeCtrl ().GetItemData (selection));
		vd.radialGroup = radialGroup;

		pDoc->SetPropertyView (RUNTIME_CLASS(FormRadialFamily), &vd);
	}
	else if (isChildSelected ())
	{
		TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
		HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();
		
		FormRadial::FormRadialViewData vd;
		vd.familyId = static_cast<int> (GetTreeCtrl ().GetItemData (selection));
		vd.radialGroup = radialGroup;
		vd.childName = GetTreeCtrl ().GetItemText (selection);

		pDoc->SetPropertyView (RUNTIME_CLASS(FormRadial), &vd);
	}

	*pResult = 0;

	//-- show selection
	updateRadialView ();
}

//-------------------------------------------------------------------

void RadialTreeView::updateRadialView (void)
{
	HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	//-- if it's a child tell the frame about it to route the message to RadialView
	if (isChild (selection))
	{
		const CString name = GetTreeCtrl ().GetItemText (selection);

		static_cast<RadialGroupFrame*> (GetParentFrame ())->setSelectedShader (name);
	}
	else
	{
		static_cast<RadialGroupFrame*> (GetParentFrame ())->setSelectedShader (0);
	}
}

//-------------------------------------------------------------------

void RadialTreeView::OnButtonCalculatecolor() 
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
		NOT_NULL (radialGroup);
	
		PackedRgb color;

		//-- get familyId
		const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (family));

		//-- get shader
		const RadialGroup::FamilyChildData fcd = radialGroup->getFamilyChild (familyId, 0);
		const Shader * shader = ShaderTemplateList::fetchShader (FileName (FileName::P_shader, fcd.shaderTemplateName));

		//-- get texture 0
		const Texture* texture = 0;
		if (!dynamic_cast<const StaticShader*>(shader) || !static_cast<const StaticShader*>(shader)->getTexture(TAG(M,A,I,N), texture))
			return;

		//-- check format
		Texture::LockData lockData(TF_RGB_888, 0, 0, 0, 1, 1, false);
		texture->lockReadOnly(lockData);
			const uint8* pixelData = reinterpret_cast<uint8*>(lockData.getPixelData ());
			color.r = pixelData [0];
			color.g = pixelData [1];
			color.b = pixelData [2];
		texture->unlock(lockData);

		radialGroup->setFamilyColor (familyId, color);

		GetDocument ()->UpdateAllViews (0);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void RadialTreeView::OnUpdateButtonCalculatecolor(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ((isFamilySelected () || isChildSelected ()) ? TRUE : FALSE);	
}

//-------------------------------------------------------------------

void RadialTreeView::OnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnSelchanged (pNMHDR, pResult);

	*pResult = 0;
}

//-------------------------------------------------------------------

void RadialTreeView::OnRButtonDown(UINT nFlags, CPoint point) 
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

void RadialTreeView::OnGroupDelete() 
{
	OnDeleteradial ();
}

//-------------------------------------------------------------------

void RadialTreeView::OnGroupRename() 
{
	IGNORE_RETURN (GetTreeCtrl ().EditLabel (GetTreeCtrl ().GetSelectedItem ()));
}

//-------------------------------------------------------------------

void RadialTreeView::reset ()
{
	IGNORE_RETURN (GetTreeCtrl ().DeleteAllItems ());

	NOT_NULL (radialGroup);

	//-- populate tree
	int i;
	for (i = 0; i < radialGroup->getNumberOfFamilies (); i++)
	{
		const int familyId = radialGroup->getFamilyId (i);

		//-- get family name
		const HTREEITEM familyRoot = GetTreeCtrl ().InsertItem (radialGroup->getFamilyName (familyId),  0, 1);

		//-- set the family data to the family index for easy searching later
		IGNORE_RETURN (GetTreeCtrl ().SetItemData (familyRoot, static_cast<DWORD> (familyId)));

		//-- insert children
		int j;
		for (j = 0; j < radialGroup->getNumberOfChildren (i); j++)
		{
			const RadialGroup::FamilyChildData fcd = radialGroup->getChild (i, j);
			DEBUG_FATAL (familyId != fcd.familyId, ("familyId != fcd.familyId (%i != %i)", familyId, fcd.familyId));

			//-- insert the child
			const HTREEITEM child = GetTreeCtrl ().InsertItem (fcd.shaderTemplateName, 2, 3, familyRoot);

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

void RadialTreeView::OnButtonDeleteunused() 
{
	//--
	CString tmp;
	tmp.Format ("Are you sure you want to delete all unused families?");

	if (MessageBox (tmp, 0, MB_YESNO) == IDNO)
		return;

	TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	// set property view to default dialog before destroying data in dlg
	PropertyView::ViewData dummyData;
	doc->SetPropertyView (RUNTIME_CLASS(PropertyView), &dummyData);

	TerrainGenerator* generator = doc->getTerrainGenerator ();
	NOT_NULL (generator);

	NOT_NULL (radialGroup);

	const int originalCount = radialGroup->getNumberOfFamilies ();
	int n = originalCount;
	int i = 0;

	while (i < n)
	{
		if (TerrainGeneratorHelper::usesRadialFamily (generator, radialGroup->getFamilyId (i)))
			i++;
		else
		{
			radialGroup->removeFamily (radialGroup->getFamilyId (i));
			n--;
		}
	}

	deletingUnused = true;
	reset ();
	deletingUnused = false;
}

//-------------------------------------------------------------------

void RadialTreeView::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
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

void RadialTreeView::OnLButtonUp (UINT nFlags, CPoint point)
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
}  //lint !e1746  //-- point could have been made a const reference

//-------------------------------------------------------------------

HTREEITEM RadialTreeView::insertChild (HTREEITEM familyItem, int familyId, const CString& childName)
{
	HTREEITEM child = GetTreeCtrl ().InsertItem (childName, 2, 3, familyItem);
	IGNORE_RETURN (GetTreeCtrl ().SetItemData (child, static_cast<DWORD> (familyId)));

	return child;
}

//-------------------------------------------------------------------

void RadialTreeView::OnLButtonUpForDrag (bool move)
{
	NOT_NULL (radialGroup);

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
	TerrainEditorDoc* const doc = static_cast<TerrainEditorDoc*> (GetDocument ());
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

	if (move && radialGroup->getFamilyNumberOfChildren (sourceFamilyId) == 1)
	{
		IGNORE_RETURN (MessageBox ("You can't move the last child, but you can copy it!"));
		return;
	}

	RadialGroup::FamilyChildData fcd = radialGroup->getFamilyChild (sourceFamilyId, sourceFamilyName);

	fcd.familyId = destinationFamilyId;
	radialGroup->addChild (fcd);

	const HTREEITEM child = insertChild (findFamily (GetTreeCtrl ().GetRootItem (), fcd.familyId), fcd.familyId, fcd.shaderTemplateName);
	IGNORE_RETURN (GetTreeCtrl ().SelectItem (child));
	
	if (move)
	{
		IGNORE_RETURN (GetTreeCtrl ().DeleteItem (m_htiDrag));

		fcd.familyId = sourceFamilyId;
		radialGroup->removeChild (fcd);
	}

	doc->UpdateAllViews (0);
	doc->SetModifiedFlag ();
}

//-------------------------------------------------------------------

void RadialTreeView::OnLButtonUpForCopy ()
{
	OnLButtonUpForDrag (false);
}

//-------------------------------------------------------------------

void RadialTreeView::OnLButtonUpForMove () 
{
	OnLButtonUpForDrag (true);
}

//-------------------------------------------------------------------

void RadialTreeView::OnMouseMove(UINT nFlags, CPoint point) 
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
				IGNORE_RETURN (KillTimer( static_cast<int> (m_idTimer) ));
				m_idTimer = 0;
			}
			
			if( !m_idTimer )
				m_idTimer = SetTimer( 1000, 2000, NULL );

			IGNORE_RETURN (CImageList::DragShowNolock(TRUE));
		}
	}
}  //lint !e1746  //-- point could have been mase a const reference

//-------------------------------------------------------------------

void RadialTreeView::OnDestroy() 
{
	if( m_idTimer )
	{
		IGNORE_RETURN (KillTimer( static_cast<int> (m_idTimer) ));
		m_idTimer = 0;
	}

	CTreeView::OnDestroy();
}

//-------------------------------------------------------------------

void RadialTreeView::OnTimer(UINT nIDEvent) 
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

HTREEITEM RadialTreeView::findFamily (HTREEITEM hItem, int familyId) const
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

void RadialTreeView::OnButtonSwayall() 
{
	DialogResetFloraParameters dlg (true);

	if (dlg.DoModal () == IDOK)
	{
		int i;
		for (i = 0; i < radialGroup->getNumberOfFamilies (); ++i)
		{
			int j;
			for (j = 0; j < radialGroup->getNumberOfChildren (i); ++j)
			{
				RadialGroup::FamilyChildData fcd = radialGroup->getChild (i, j);

				if (dlg.m_checkDisplacement)
					fcd.displacement = dlg.m_finalDisplacement;

				if (dlg.m_checkPeriod)
					fcd.period = dlg.m_finalPeriod;

				if (dlg.m_checkSway)
					fcd.shouldSway = dlg.m_shouldSway == TRUE;

				if (dlg.m_checkAlign)
					fcd.alignToTerrain = dlg.m_alignToTerrain == TRUE;

				radialGroup->setFamilyChild (fcd.familyId, fcd);
			}
		}

		GetDocument ()->UpdateAllViews (this);
	}
}

//-------------------------------------------------------------------

void RadialTreeView::OnUpdateButtonSwayall(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (isFamilySelected ());
}

//-------------------------------------------------------------------

void RadialTreeView::OnButtonSwayfamily() 
{
	DialogResetFloraParameters dlg (true);

	if (dlg.DoModal () == IDOK)
	{
		HTREEITEM family = getSelectedFamily ();

		if (family)
		{
			NOT_NULL (radialGroup);

			const int familyId = GetTreeCtrl ().GetItemData (family);

			int j;
			for (j = 0; j < radialGroup->getFamilyNumberOfChildren (familyId); ++j)
			{
				RadialGroup::FamilyChildData fcd = radialGroup->getFamilyChild (familyId, j);

				if (dlg.m_checkDisplacement)
					fcd.displacement = dlg.m_finalDisplacement;

				if (dlg.m_checkPeriod)
					fcd.period = dlg.m_finalPeriod;

				if (dlg.m_checkSway)
					fcd.shouldSway = dlg.m_shouldSway == TRUE;

				if (dlg.m_checkAlign)
					fcd.alignToTerrain = dlg.m_alignToTerrain == TRUE;

				radialGroup->setFamilyChild (familyId, fcd);
			}
		}

		GetDocument ()->UpdateAllViews (this);
	}
}

//-------------------------------------------------------------------

void RadialTreeView::OnUpdateButtonSwayfamily(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (isFamilySelected ());
}

//-------------------------------------------------------------------

void RadialTreeView::OnGroupFindrulesusingthisfamily() 
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
		PackedRgb color;

		//-- get familyId
		const int familyId = GetTreeCtrl ().GetItemData (family);

		TerrainEditorDoc* const doc  = static_cast<TerrainEditorDoc*> (GetDocument ());
		NOT_NULL (doc);

		ArrayList<TerrainGeneratorHelper::OutputData> output;

		TerrainGeneratorHelper::findRadial (doc->getTerrainGenerator (), familyId, output);

		if (output.empty ())
		{
			CString string;
			string.Format ("Family %s not used", radialGroup->getFamilyName (familyId));

			MessageBox (string);
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

