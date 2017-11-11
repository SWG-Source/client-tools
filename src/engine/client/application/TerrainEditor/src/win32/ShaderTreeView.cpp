//
// ShaderTreeView.cpp
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "ShaderTreeView.h"

#include "FindFrame.h"
#include "FormShader.h"
#include "FormShaderFamily.h"
#include "RecentDirectory.h"
#include "ShaderGroupFrame.h"
#include "TerrainEditorDoc.h"
#include "TerrainGeneratorHelper.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "sharedUtility/FileName.h"
#include "terraineditor.h"

//-------------------------------------------------------------------

static inline bool keyDown (uint key)
{
	return (GetKeyState (key) & 0x8000) != 0;
}

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(ShaderTreeView, CTreeView)

//-------------------------------------------------------------------

ShaderTreeView::ShaderTreeView (void) :
	shaderGroup (0),
	imageListSet (false),
	deletingUnused (false),
	m_dragMode (DM_nothing)
{
	m_pDragImage      = 0;
	m_htiDrag         = m_htiDrop = m_htiOldDrop = 0;
	m_idTimer         = 0;

	m_moveCursor = LoadCursor (AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_CURSOR_MOVE));
	m_copyCursor = LoadCursor (AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_CURSOR_COPY));
}

ShaderTreeView::~ShaderTreeView()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(ShaderTreeView, CTreeView)
	//{{AFX_MSG_MAP(ShaderTreeView)
	ON_COMMAND(ID_NEWSHADERFAMILY, OnNewshaderfamily)
	ON_UPDATE_COMMAND_UI(ID_NEWSHADERFAMILY, OnUpdateNewshaderfamily)
	ON_COMMAND(ID_NEWSHADERCHILD, OnNewshaderchild)
	ON_UPDATE_COMMAND_UI(ID_NEWSHADERCHILD, OnUpdateNewshaderchild)
	ON_COMMAND(ID_PROMOTESHADER, OnPromoteshader)
	ON_UPDATE_COMMAND_UI(ID_PROMOTESHADER, OnUpdatePromoteshader)
	ON_COMMAND(ID_DEMOTESHADER, OnDemoteshader)
	ON_UPDATE_COMMAND_UI(ID_DEMOTESHADER, OnUpdateDemoteshader)
	ON_COMMAND(ID_DELETESHADER, OnDeleteshader)
	ON_UPDATE_COMMAND_UI(ID_DELETESHADER, OnUpdateDeleteshader)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_COMMAND(ID_FINDSHADER, OnFindshader)
	ON_UPDATE_COMMAND_UI(ID_FINDSHADER, OnUpdateFindshader)
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
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_COMMAND(ID_GROUP_FINDRULESUSINGTHISFAMILY, OnGroupFindrulesusingthisfamily)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

void ShaderTreeView::OnDraw(CDC* pDC)
{
	UNREF (pDC);
	CDocument* pDoc = GetDocument();
	UNREF (pDoc);
	// TODO: add draw code here
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void ShaderTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void ShaderTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

bool ShaderTreeView::isFamily (HTREEITEM selection) const
{
	//-- am i pointing to a family? (families have no root)
	const HTREEITEM parent = GetTreeCtrl ().GetParentItem (selection);

	return parent == 0;
}

//-------------------------------------------------------------------

HTREEITEM ShaderTreeView::getSelectedFamily (void) const
{
	//-- am i pointing to anything?
	const HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	return isFamily (selection) ? selection : 0;
}

//-------------------------------------------------------------------

bool ShaderTreeView::isFamilySelected (void) const
{
	return getSelectedFamily () != 0;
}

//-------------------------------------------------------------------

bool ShaderTreeView::isChild (HTREEITEM selection) const
{
	//-- am i pointing to a family? (families have no root)
	const HTREEITEM parent        = GetTreeCtrl ().GetParentItem (selection);
	const HTREEITEM parentsParent = GetTreeCtrl ().GetParentItem (parent);

	return (parent && !parentsParent);
}

//-------------------------------------------------------------------

HTREEITEM ShaderTreeView::getSelectedChild (void) const
{
	//-- am i pointing to anything?
	const HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	return isChild (selection) ? selection : 0;
}

//-------------------------------------------------------------------

bool ShaderTreeView::isChildSelected (void) const
{
	return getSelectedChild () != 0;
}

//-------------------------------------------------------------------

static int getUniqueFamilyId (ShaderGroup* shaderGroup)
{
	int familyId = 1;

	int numberOfTries = 0;
	while (numberOfTries++ < 1000)
	{
		bool found = false;

		//-- search for id
		int familyIndex;
		for (familyIndex = 0; familyIndex < shaderGroup->getNumberOfFamilies (); familyIndex++)
			if (shaderGroup->getFamilyId (familyIndex) == familyId)
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
	FATAL (true, ("couldn't find any unique shaders ids after 1000 tries"));

	return 0;
}

//-------------------------------------------------------------------

void ShaderTreeView::addFamily (const CString& familyName, const ArrayList<CString*>& children)
{
	//-- create new family
	const int familyId = getUniqueFamilyId (shaderGroup);

	CString newFamilyName = CreateUniqueFamilyName (&familyName);
	shaderGroup->addFamily (familyId, newFamilyName, PackedRgb::solidYellow);

	TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
	shaderGroup->setFamilyShaderSize (familyId, pDoc->getDefaultShaderSize ());

	//-- insert into tree
	const HTREEITEM family = GetTreeCtrl ().InsertItem (newFamilyName, 0, 1);
	GetTreeCtrl ().SetItemData (family, familyId);

	int i;
	for (i = 0; i < children.getNumberOfElements (); i++)
	{
		//-- we now have the family
		ShaderGroup::FamilyChildData fcd;
		fcd.shaderTemplateName     = (LPCSTR) *children [i];
		fcd.familyId               = familyId;
		fcd.weight                 = CONST_REAL (1);

		shaderGroup->addChild (fcd);

		const HTREEITEM child = GetTreeCtrl ().InsertItem (fcd.shaderTemplateName, 2, 3, family);

		//-- set the child data to the family id for easy searching later
		GetTreeCtrl ().SetItemData (child, static_cast<DWORD> (fcd.familyId));
	}

	GetTreeCtrl ().SelectItem (family);
	GetDocument ()->SetModifiedFlag ();

	Invalidate ();
}

//-------------------------------------------------------------------

void ShaderTreeView::OnNewshaderfamily() 
{
	//-- create new family
	const int familyId = getUniqueFamilyId (shaderGroup);
	
	CString familyName = CreateUniqueFamilyName ();
	shaderGroup->addFamily (familyId, familyName, PackedRgb::solidYellow);

	TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
	shaderGroup->setFamilyShaderSize (familyId, pDoc->getDefaultShaderSize ());
	
	//-- insert into tree
	const HTREEITEM family = GetTreeCtrl ().InsertItem (familyName, 0, 1);
	GetTreeCtrl ().SetItemData (family, familyId);

	GetTreeCtrl ().SelectItem (family);

	OnNewshaderchild ();

	Invalidate ();
}

//-------------------------------------------------------------------

void ShaderTreeView::OnUpdateNewshaderfamily(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (true);
}

//-------------------------------------------------------------------

bool ShaderTreeView::childExistsInFamily (int familyId, const char* childName) const
{
	const int n = shaderGroup->getFamilyNumberOfChildren (familyId);

	int i;
	for (i = 0; i < n; ++i)
	{
		const ShaderGroup::FamilyChildData fcd = shaderGroup->getFamilyChild (familyId, i);

		if (_stricmp (childName, fcd.shaderTemplateName) == 0)
			return true;
	}

	return false;
}

//-------------------------------------------------------------------

bool ShaderTreeView::familyExists (const char* familyName) const
{
	const int n = shaderGroup->getNumberOfFamilies ();

	int i;
	for (i = 0; i < n; ++i)
	{
		const int familyId = shaderGroup->getFamilyId (i);

		if (_stricmp (familyName, shaderGroup->getFamilyName (familyId)) == 0)
			return true;
	}

	return false;
}

//-------------------------------------------------------------------

const CString ShaderTreeView::CreateUniqueChildName (int familyId)
{
	CString childName;

	int lastChildIndex = shaderGroup->getFamilyNumberOfChildren (familyId);

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

const CString ShaderTreeView::CreateUniqueFamilyName (const CString* base)
{
	CString familyName;
	int     familyIndex = 1;

	if (base)
	{
		familyName.Format ("%s", *base);
	}
	else
	{
		NOT_NULL (shaderGroup);
		familyIndex = shaderGroup->getNumberOfFamilies ();

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

void ShaderTreeView::OnNewshaderchild() 
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
		const int familyId    = GetTreeCtrl ().GetItemData (family);

		CString childName = CreateUniqueChildName (familyId);

		//-- we now have the family
		ShaderGroup::FamilyChildData fcd;
		fcd.shaderTemplateName     = childName;
		fcd.familyId               = familyId;
		fcd.weight                 = CONST_REAL (1);

		shaderGroup->addChild (fcd);

		const HTREEITEM child = GetTreeCtrl ().InsertItem (fcd.shaderTemplateName, 2, 3, family);

		//-- set the child data to the family id for easy searching later
		GetTreeCtrl ().SetItemData (child, static_cast<DWORD> (fcd.familyId));

		GetTreeCtrl ().SelectItem (child);

		GetDocument ()->SetModifiedFlag ();

		Invalidate ();
	}
}

//-------------------------------------------------------------------

void ShaderTreeView::OnUpdateNewshaderchild(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (isChildSelected () || isFamilySelected ());
}

//-------------------------------------------------------------------

HTREEITEM ShaderTreeView::insertFamily (int familyId, HTREEITEM& afterItem)
{
	const HTREEITEM familyRoot = GetTreeCtrl ().InsertItem (shaderGroup->getFamilyName (familyId), 0, 1, TVI_ROOT, afterItem);

	//-- set the family data to the family index for easy searching later
	GetTreeCtrl ().SetItemData (familyRoot, familyId);

	//-- insert children
	int j;
	for (j = 0; j < shaderGroup->getFamilyNumberOfChildren (familyId); j++)
	{
		const ShaderGroup::FamilyChildData fcd = shaderGroup->getFamilyChild (familyId, j);
		DEBUG_FATAL (familyId != fcd.familyId, ("familyId != fcd.familyId (%i != %i)", familyId, fcd.familyId));

		//-- insert the child
		const HTREEITEM child = GetTreeCtrl ().InsertItem (fcd.shaderTemplateName, 2, 3, familyRoot);

		//-- set the child data to the family id for easy searching later
		GetTreeCtrl ().SetItemData (child, static_cast<DWORD> (fcd.familyId));
	}
	return familyRoot;
}

//-------------------------------------------------------------------

void ShaderTreeView::OnPromoteshader() 
{
	HTREEITEM family = getSelectedFamily ();
	HTREEITEM prevFamily = GetTreeCtrl ().GetPrevSiblingItem (family);

	if (family && prevFamily)
	{
		const int familyId = GetTreeCtrl ().GetItemData (family);
		shaderGroup->promoteFamily (familyId);

		//-- reorder family entries.

		uint state = GetTreeCtrl ().GetItemState (prevFamily, TVIS_EXPANDED);
		bool expanded = ((state & TVIS_EXPANDED) == 0) ? false : true ;
		int prevFamilyId = GetTreeCtrl ().GetItemData (prevFamily);
		GetTreeCtrl ().DeleteItem (prevFamily);
		HTREEITEM newlyInserted = insertFamily (prevFamilyId, family);

		//-- expand if expanded before
		if (expanded)
			GetTreeCtrl ().SetItemState (newlyInserted, TVIS_EXPANDED, TVIS_EXPANDED);

		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void ShaderTreeView::OnUpdatePromoteshader(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (isFamilySelected ());
}

//-------------------------------------------------------------------

void ShaderTreeView::OnDemoteshader() 
{
	HTREEITEM family = getSelectedFamily ();
	HTREEITEM nextFamily = GetTreeCtrl ().GetNextSiblingItem (family);

	if (family && nextFamily)
	{
		const int familyId = GetTreeCtrl ().GetItemData (family);
		shaderGroup->demoteFamily (familyId);

		//-- reorder family entries.
		uint state = GetTreeCtrl ().GetItemState (family, TVIS_EXPANDED);
		bool expanded = ((state & TVIS_EXPANDED) != 0);
		GetTreeCtrl ().DeleteItem (family);
		HTREEITEM newlyInserted = insertFamily (familyId, nextFamily);
		GetTreeCtrl ().SelectItem (newlyInserted);

		//-- expand if expanded before
		if (expanded)
			GetTreeCtrl ().SetItemState (newlyInserted, TVIS_EXPANDED, TVIS_EXPANDED);

		Invalidate();

		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void ShaderTreeView::OnUpdateDemoteshader(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (isFamilySelected ());
}

//-------------------------------------------------------------------

void ShaderTreeView::OnDeleteshader() 
{
	HTREEITEM family = getSelectedFamily ();

	if (family)
	{
		const int familyId = GetTreeCtrl ().GetItemData (family);
		const char* name   = shaderGroup->getFamilyName (familyId);

		TerrainEditorDoc* const pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
		if (TerrainGeneratorHelper::usesShaderFamily (pDoc->getTerrainGenerator (), familyId))
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
			PropertyView::ViewData dummyData;
			pDoc->SetPropertyView (RUNTIME_CLASS(PropertyView), &dummyData);
			
			//-- remove from shader group
			shaderGroup->removeFamily (familyId);

			//-- delete from tree
			GetTreeCtrl ().DeleteItem (family);

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
			//-- remove from shader group
			const int familyId = GetTreeCtrl ().GetItemData (GetTreeCtrl ().GetParentItem (child));

			if (shaderGroup->getFamilyNumberOfChildren (familyId) > 1)
			{
				const CString name = GetTreeCtrl ().GetItemText (child);

				CString tmp;
				tmp.Format ("Are you sure you want to delete %s?", name);

				if (MessageBox (tmp, 0, MB_YESNO) == IDYES)
				{
					ShaderGroup::FamilyChildData fcd;
					fcd.shaderTemplateName     = name;
					fcd.familyId               = familyId;

					// set property view to default dialog before destroying data in dlg
					TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
					PropertyView::ViewData dummyData;
					pDoc->SetPropertyView (RUNTIME_CLASS(PropertyView), &dummyData);

					shaderGroup->removeChild (fcd);

					//-- delete from tree
					GetTreeCtrl ().DeleteItem (child);

					GetDocument ()->SetModifiedFlag ();

					Invalidate ();
				}
			}
			else
				MessageBox ("You can't delete the only child! Remove the family instead, or add another child and then remove this one...");				
		}
	}
}

//-------------------------------------------------------------------

void ShaderTreeView::OnUpdateDeleteshader(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (isChildSelected () || isFamilySelected ());
}


//-------------------------------------------------------------------

void ShaderTreeView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREF (pNMHDR);

	*pResult = 0;

	//-- same as find target
	OnFindshader ();
}

//-------------------------------------------------------------------

void ShaderTreeView::OnFindshader() 
{
	//-- find shader template name
	if (isChildSelected ())
	{
		CFileDialog dlg (true, "*.sht", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR));
		dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("ShaderTemplate_Shader");
		dlg.m_ofn.lpstrTitle      = "Select Shader Template File";

		bool stillTrying = true;
		bool badName     = true;

		while (badName && stillTrying)
		{
			if (dlg.DoModal () == IDOK)
			{
				RecentDirectory::update ("ShaderTemplate_Shader", dlg.GetPathName ());

				//-- get new name
				const CString newName = dlg.GetFileTitle ();

				//-- get child
				const HTREEITEM child = getSelectedChild ();

				if (child)
				{
					const int familyId = GetTreeCtrl ().GetItemData (GetTreeCtrl ().GetParentItem (child));

					if ( !childExistsInFamily (familyId, newName) )
					{
						badName = false;

						//-- get old name
						const CString oldName = GetTreeCtrl ().GetItemText (child);	

						//-- refresh the property page because it sends changes back via child name.
						//-- Remove current shader form this BEFORE name changes so current form values can be set.
						TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
						PropertyView::ViewData dummyData;
						pDoc->SetPropertyView (RUNTIME_CLASS(PropertyView), &dummyData);

						//-- rename in shader group
						shaderGroup->renameChild (familyId, oldName, newName);

						//-- rename in tree
						GetTreeCtrl ().SetItemText (child, newName);

						//-- tell the shader view about it
						updateShaderView ();

						GetDocument ()->SetModifiedFlag ();

						Invalidate ();

						//-- tell the properties view about it
						{
							TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
							HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();
							
							FormShader::FormShaderViewData vd;
							vd.familyId = GetTreeCtrl ().GetItemData (selection);
							vd.shaderGroup = shaderGroup;
							vd.childName = GetTreeCtrl ().GetItemText (selection);

							pDoc->SetPropertyView (RUNTIME_CLASS(FormShader), &vd);
						}

					}
					else
					{
						MessageBox("A child with this name already exists.  Please select another name.", "Bad Name");
					}
				}
			}
			else
				stillTrying = false;
		}
    
	}
}

//-------------------------------------------------------------------

void ShaderTreeView::OnUpdateFindshader(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (isChildSelected ());
}

//-------------------------------------------------------------------

BOOL ShaderTreeView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= (TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_EDITLABELS);
	
	return CTreeView::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

void ShaderTreeView::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
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

void ShaderTreeView::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
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
		//-- change name
		const int familyId = GetTreeCtrl ().GetItemData (pTVDispInfo->item.hItem);
		shaderGroup->setFamilyName (familyId, newName);

		//-- tell tree about new name
		GetTreeCtrl ().SetItem (&pTVDispInfo->item);
		*pResult = 1;

		//-- tell properties window about new name.
		{
			TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
			HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

			FormShaderFamily::FormShaderFamilyViewData vd;
			vd.familyId    = GetTreeCtrl ().GetItemData (selection);
			vd.shaderGroup = shaderGroup;

			pDoc->SetPropertyView (RUNTIME_CLASS(FormShaderFamily), &vd);
		}

	}
	else {
		MessageBox("A family with this name already exists.  Please select another name.", "Bad Name");
		*pResult = 0;
	}

}

//-------------------------------------------------------------------

void ShaderTreeView::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult) 
{
	const TV_KEYDOWN* pTVKeyDown = reinterpret_cast<const TV_KEYDOWN*> (pNMHDR);

	switch (pTVKeyDown->wVKey)
	{
	case VK_INSERT:
		{
			//-- same as new child
			OnNewshaderchild();
		}
		break;

	case VK_DELETE:
		{
			//-- same as delete
			OnDeleteshader();
		}
		break;
	}

	*pResult = 0;
}

//-------------------------------------------------------------------

void ShaderTreeView::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();

	if (!imageListSet)
	{
		imageList.Create (IDB_BITMAP_FAMILY, 16, 1, RGB (255,255,255));
		GetTreeCtrl ().SetImageList (&imageList, TVSIL_NORMAL);

		imageListSet = true;
	}
	
	//-- get the document's shader group
	TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	shaderGroup = &doc->getTerrainGenerator ()->getShaderGroup ();
	NOT_NULL (shaderGroup);

	reset ();
}

//-------------------------------------------------------------------

void ShaderTreeView::expandBranch (HTREEITEM item)
{
	//-- recursuvely expand each branch of the tree
	if (GetTreeCtrl ().ItemHasChildren (item))
	{
		GetTreeCtrl ().Expand (item, TVE_EXPAND);
		item = GetTreeCtrl ().GetChildItem (item);

		do
		{
			expandBranch (item);
		}
		while ((item = GetTreeCtrl ().GetNextSiblingItem (item)) != 0);
	}
}

//-------------------------------------------------------------------

void ShaderTreeView::expandAll (void) 
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

void ShaderTreeView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREF (pNMHDR);

	if (deletingUnused)
		return;

	if (isFamilySelected ())
	{
		TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
		HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

		FormShaderFamily::FormShaderFamilyViewData vd;
		vd.familyId    = GetTreeCtrl ().GetItemData (selection);
		vd.shaderGroup = shaderGroup;

		pDoc->SetPropertyView (RUNTIME_CLASS(FormShaderFamily), &vd);
	}
	else if (isChildSelected ())
	{
		TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
		HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();
		
		FormShader::FormShaderViewData vd;
		vd.familyId = GetTreeCtrl ().GetItemData (selection);
		vd.shaderGroup = shaderGroup;
		vd.childName = GetTreeCtrl ().GetItemText (selection);

		pDoc->SetPropertyView (RUNTIME_CLASS(FormShader), &vd);
	}

	*pResult = 0;

	//-- show selection
	updateShaderView ();
}

//-------------------------------------------------------------------

void ShaderTreeView::updateShaderView (void)
{
	HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	//-- if it's a child tell the frame about it to route the message to ShaderView
	if (isChild (selection))
	{
		const CString name = GetTreeCtrl ().GetItemText (selection);

		static_cast<ShaderGroupFrame*> (GetParentFrame ())->setSelectedShader (name);
	}
	else
	{
		static_cast<ShaderGroupFrame*> (GetParentFrame ())->setSelectedShader (0);
	}
}

//-------------------------------------------------------------------

void ShaderTreeView::OnButtonCalculatecolor() 
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

		//-- get shader
		const ShaderGroup::FamilyChildData fcd = shaderGroup->getFamilyChild (familyId, 0);
		const Shader * shader = ShaderTemplateList::fetchShader (FileName (FileName::P_shader, fcd.shaderTemplateName));

		//-- get texture 0
		const Texture* texture = 0;
		if (!dynamic_cast<const StaticShader*>(shader) || !static_cast<const StaticShader*>(shader)->getTexture(TAG(M,A,I,N), texture))
			return;

		bool valid = true;

		//-- check format
		Texture::LockData lockData(TF_RGB_888, 0, 0, 0, 1, 1, false);
		texture->lockReadOnly(lockData);
			const uint8* pixelData = reinterpret_cast<uint8*>(lockData.getPixelData ());
			color.r = pixelData [0];
			color.g = pixelData [1];
			color.b = pixelData [2];
		texture->unlock(lockData);

		if (valid)
		{
			shaderGroup->setFamilyColor (familyId, color);

			GetDocument ()->UpdateAllViews (0);
			GetDocument ()->SetModifiedFlag ();
		}
	}
}

//-------------------------------------------------------------------

void ShaderTreeView::OnUpdateButtonCalculatecolor(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (isFamilySelected () || isChildSelected ());	
}

//-------------------------------------------------------------------

void ShaderTreeView::OnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnSelchanged (pNMHDR, pResult);

	*pResult = 0;
}

//-------------------------------------------------------------------

void ShaderTreeView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if (isFamilySelected ())
	{
		//-- convert client coordinates to screen coordinates
		CPoint pt = point;
		ClientToScreen (&pt);

		CMenu menu;
		menu.LoadMenu (IDR_GROUP_MENU);

		CMenu* subMenu = menu.GetSubMenu (0);
		subMenu->TrackPopupMenu (TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	}

	CTreeView::OnRButtonDown(nFlags, point);
}

//-------------------------------------------------------------------

void ShaderTreeView::OnGroupDelete() 
{
	OnDeleteshader ();
}

//-------------------------------------------------------------------

void ShaderTreeView::OnGroupRename() 
{
	GetTreeCtrl ().EditLabel (GetTreeCtrl ().GetSelectedItem ());
}

//-------------------------------------------------------------------

void ShaderTreeView::reset ()
{
	GetTreeCtrl ().DeleteAllItems ();

	//-- populate tree
	int i;
	for (i = 0; i < shaderGroup->getNumberOfFamilies (); i++)
	{
		const int familyId = shaderGroup->getFamilyId (i);

		//-- get family name
		const HTREEITEM familyRoot = GetTreeCtrl ().InsertItem (shaderGroup->getFamilyName (familyId),  0, 1);

		//-- set the family data to the family index for easy searching later
		GetTreeCtrl ().SetItemData (familyRoot, familyId);

		//-- insert children
		int j;
		for (j = 0; j < shaderGroup->getNumberOfChildren (i); j++)
		{
			const ShaderGroup::FamilyChildData fcd = shaderGroup->getChild (i, j);
			DEBUG_FATAL (familyId != fcd.familyId, ("familyId != fcd.familyId (%i != %i)", familyId, fcd.familyId));

			//-- insert the child
			const HTREEITEM child = GetTreeCtrl ().InsertItem (fcd.shaderTemplateName, 2, 3, familyRoot);

			//-- set the child data to the family id for easy searching later
			GetTreeCtrl ().SetItemData (child, static_cast<DWORD> (fcd.familyId));
		}
	}

	//-- expand the tree
	expandAll ();

	GetTreeCtrl ().SelectItem (GetTreeCtrl ().GetRootItem ());
	GetTreeCtrl ().EnsureVisible (GetTreeCtrl ().GetRootItem());
}

//-------------------------------------------------------------------

void ShaderTreeView::OnButtonDeleteunused() 
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

	const int originalCount = shaderGroup->getNumberOfFamilies ();
	int n = originalCount;
	int i = 0;

	while (i < n)
	{
		if (TerrainGeneratorHelper::usesShaderFamily (generator, shaderGroup->getFamilyId (i)))
			i++;
		else
		{
			shaderGroup->removeFamily (shaderGroup->getFamilyId (i));
			n--;
		}
	}

	deletingUnused = true;
	reset ();
	deletingUnused = false;
}

//-------------------------------------------------------------------

void ShaderTreeView::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

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
	m_pDragImage->GetImageInfo( 0, &ii );
	pt.x = (ii.rcImage.right - ii.rcImage.left) / 2;
	pt.y = (ii.rcImage.bottom - ii.rcImage.top) / 2;

	m_pDragImage->BeginDrag( 0, pt );
	pt = pNMTreeView->ptDrag;
	ClientToScreen( &pt );
	m_pDragImage->DragEnter(NULL,pt);
	
	SetCapture();

	*pResult = 0;
}

//-------------------------------------------------------------------

void ShaderTreeView::OnLButtonUp (UINT nFlags, CPoint point)
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
}

//-------------------------------------------------------------------

HTREEITEM ShaderTreeView::insertChild (HTREEITEM familyItem, int familyId, const CString& childName)
{
	HTREEITEM child = GetTreeCtrl ().InsertItem (childName, 2, 3, familyItem);
	GetTreeCtrl ().SetItemData (child, static_cast<DWORD> (familyId));

	return child;
}

//-------------------------------------------------------------------

void ShaderTreeView::OnLButtonUpForDrag (bool move)
{
	m_dragMode = DM_nothing;

	CImageList::DragLeave (this);
	CImageList::EndDrag ();

	ReleaseCapture();

	delete m_pDragImage;

	GetTreeCtrl ().SelectDropTarget (NULL);
	m_htiOldDrop = NULL;

	if (m_idTimer)
	{
		KillTimer (m_idTimer);
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
	const int sourceFamilyId = GetTreeCtrl ().GetItemData (m_htiDrag);

	//-- get the destination item
	const int destinationFamilyId = GetTreeCtrl ().GetItemData (m_htiDrop);

	//-- are the families equal?
	if (destinationFamilyId == sourceFamilyId)
		return;

	const CString sourceFamilyName = GetTreeCtrl ().GetItemText (m_htiDrag);

	//-- does the child already exist in the family?
	if (childExistsInFamily (destinationFamilyId, sourceFamilyName))
	{
		MessageBox ("Selected child already exists in the family!");
		return;
	}

	if (move && shaderGroup->getFamilyNumberOfChildren (sourceFamilyId) == 1)
	{
		MessageBox ("You can't move the last child, but you can copy it!");
		return;
	}

	ShaderGroup::FamilyChildData fcd = shaderGroup->getFamilyChild (sourceFamilyId, sourceFamilyName);

	fcd.familyId = destinationFamilyId;
	shaderGroup->addChild (fcd);

	const HTREEITEM child = insertChild (findFamily (GetTreeCtrl ().GetRootItem (), fcd.familyId), fcd.familyId, fcd.shaderTemplateName);
	GetTreeCtrl ().SelectItem (child);
	
	if (move)
	{
		GetTreeCtrl ().DeleteItem (m_htiDrag);

		fcd.familyId = sourceFamilyId;
		shaderGroup->removeChild (fcd);
	}

	doc->UpdateAllViews (0);
	doc->SetModifiedFlag ();
}

//-------------------------------------------------------------------

void ShaderTreeView::OnLButtonUpForCopy ()
{
	OnLButtonUpForDrag (false);
}

//-------------------------------------------------------------------

void ShaderTreeView::OnLButtonUpForMove () 
{
	OnLButtonUpForDrag (true);
}

//-------------------------------------------------------------------

void ShaderTreeView::OnMouseMove(UINT nFlags, CPoint point) 
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
		CImageList::DragMove(pt);
		
		hti = theTree.HitTest(point,&flags);
		if( hti != NULL )
		{
			CImageList::DragShowNolock(FALSE);

			if( m_htiOldDrop == NULL )
				m_htiOldDrop = theTree.GetDropHilightItem();

			theTree.SelectDropTarget(hti);
			
			m_htiDrop = hti;
			
			if( m_idTimer && hti == m_htiOldDrop )
			{
				KillTimer( m_idTimer );
				m_idTimer = 0;
			}
			
			if( !m_idTimer )
				m_idTimer = SetTimer( 1000, 2000, NULL );

			CImageList::DragShowNolock(TRUE);
		}
	}
}

//-------------------------------------------------------------------

void ShaderTreeView::OnDestroy() 
{
	if( m_idTimer )
	{
		KillTimer( m_idTimer );
		m_idTimer = 0;
	}

	CTreeView::OnDestroy();
}

//-------------------------------------------------------------------

void ShaderTreeView::OnTimer(UINT nIDEvent) 
{
    if( nIDEvent == m_idTimer )
    {
        CTreeCtrl& theTree = GetTreeCtrl();
        HTREEITEM htiFloat = theTree.GetDropHilightItem();
        if( htiFloat && htiFloat == m_htiDrop )
        {
            if( theTree.ItemHasChildren( htiFloat ) )
                theTree.Expand( htiFloat, TVE_EXPAND );
        }
    }

    CTreeView::OnTimer(nIDEvent);
}

//-------------------------------------------------------------------

HTREEITEM ShaderTreeView::findFamily (HTREEITEM hItem, int familyId) const
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

	if (theItem == NULL) 
		return NULL;

	return theItem;
}

//-------------------------------------------------------------------

void ShaderTreeView::OnGroupFindrulesusingthisfamily() 
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

		TerrainGeneratorHelper::findShader (doc->getTerrainGenerator (), familyId, output);

		if (output.empty ())
		{
			CString string;
			string.Format ("Family %s not used", shaderGroup->getFamilyName (familyId));

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

