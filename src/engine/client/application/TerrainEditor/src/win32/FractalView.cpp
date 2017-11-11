//
// FractalView.cpp
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FractalView.h"

#include "FindFrame.h"
#include "FormFractalFamily.h"
#include "TerrainEditorDoc.h"
#include "TerrainGeneratorHelper.h"
#include "terraineditor.h"
#include "sharedFractal/MultiFractal.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FractalView, CTreeView)

//-------------------------------------------------------------------

FractalView::FractalView (void) :
	CTreeView (),
	fractalGroup (0),
	deletingUnused (false)
{
}

FractalView::~FractalView()
{
	fractalGroup = 0;
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(FractalView, CTreeView)
	//{{AFX_MSG_MAP(FractalView)
	ON_COMMAND(ID_NEWFAMILY, OnNewfamily)
	ON_UPDATE_COMMAND_UI(ID_NEWFAMILY, OnUpdateNewfamily)
	ON_COMMAND(ID_DELETEFAMILY, OnDeletefamily)
	ON_UPDATE_COMMAND_UI(ID_DELETEFAMILY, OnUpdateDeletefamily)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginlabeledit)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndlabeledit)
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeydown)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_GROUP_DELETE, OnGroupDelete)
	ON_COMMAND(ID_GROUP_RENAME, OnGroupRename)
	ON_COMMAND(ID_BUTTON_DELETEUNUSED, OnButtonDeleteunused)
	ON_COMMAND(ID_GROUP_FINDRULESUSINGTHISFAMILY, OnGroupFindrulesusingthisfamily)
	ON_COMMAND(ID_BUTTON_COPY_CURRENT, OnButtonCopyCurrent)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_COPY_CURRENT, OnUpdateButtonCopyCurrent)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

void FractalView::OnDraw(CDC* pDC)
{
	UNREF (pDC);
	CDocument* pDoc = GetDocument();
	UNREF (pDoc);
	// TODO: add draw code here
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void FractalView::AssertValid() const
{
	CTreeView::AssertValid();
}

void FractalView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

HTREEITEM FractalView::getSelectedFamily (void) const
{
	return GetTreeCtrl ().GetSelectedItem ();
}

//-------------------------------------------------------------------

static int getUniqueFamilyId (const FractalGroup* fractalGroup)
{
	NOT_NULL (fractalGroup);

	int familyId = 1;
	while (fractalGroup->hasFamily (familyId))
		++familyId;

	return familyId;
}

//-------------------------------------------------------------------

void FractalView::addFamily (const CString& familyName)
{
	//-- create new family
	const int familyId = getUniqueFamilyId (fractalGroup);

	CString newFamilyName = createUniqueFamilyName (&familyName);
	NOT_NULL (fractalGroup);
	fractalGroup->addFamily (familyId, newFamilyName);

	//-- insert into tree
	const HTREEITEM family = GetTreeCtrl ().InsertItem (newFamilyName, 0, 1, TVI_ROOT, TVI_SORT);
	IGNORE_RETURN (GetTreeCtrl ().SetItemData (family, static_cast<DWORD> (familyId)));

	IGNORE_RETURN (GetTreeCtrl ().SelectItem (family));
	GetDocument ()->SetModifiedFlag ();

	Invalidate ();
}

//-------------------------------------------------------------------

void FractalView::OnNewfamily() 
{
	//-- create new family
	const int familyId = getUniqueFamilyId (fractalGroup);
	
	CString familyName = createUniqueFamilyName ();
	NOT_NULL (fractalGroup);
	fractalGroup->addFamily (familyId, familyName);

	//-- insert into tree
	const HTREEITEM family = GetTreeCtrl ().InsertItem (familyName, 0, 1, TVI_ROOT, TVI_SORT);
	IGNORE_RETURN (GetTreeCtrl ().SetItemData (family, static_cast<DWORD> (familyId)));

	IGNORE_RETURN (GetTreeCtrl ().SelectItem (family));

	Invalidate ();
}

//-------------------------------------------------------------------

void FractalView::OnUpdateNewfamily(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (true);
}  //lint !e1762  //-- pCmdUI could be made a const reference

//-------------------------------------------------------------------

bool FractalView::familyExists (const char* familyName) const
{
	NOT_NULL (fractalGroup);
	const int n = fractalGroup->getNumberOfFamilies ();

	int i;
	for (i = 0; i < n; ++i)
	{
		const int familyId = fractalGroup->getFamilyId (i);

		if (_stricmp (familyName, fractalGroup->getFamilyName (familyId)) == 0)
			return true;
	}

	return false;
}

//-------------------------------------------------------------------

const CString FractalView::createUniqueFamilyName (const CString* base) const
{
	CString familyName;
	int     familyIndex = 1;

	if (base)
	{
		familyName.Format ("%s", *base);
	}
	else
	{
		NOT_NULL (fractalGroup);
		familyIndex = fractalGroup->getNumberOfFamilies ();

		familyName.Format ("Family_%i", familyIndex);
	}

	while (familyExists (familyName))
	{
		if (base)
		{
			familyName.Format ("%s_%i", (LPCTSTR) base, familyIndex);    //lint !e1924  //-- c-style cast
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

void FractalView::OnDeletefamily() 
{
	HTREEITEM family = getSelectedFamily ();

	if (family)
	{
		const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (family));
		NOT_NULL (fractalGroup);
		const char* name   = fractalGroup->getFamilyName (familyId);

		TerrainEditorDoc* const pDoc = safe_cast<TerrainEditorDoc*> (GetDocument());
		if (TerrainGeneratorHelper::usesFractalFamily (pDoc->getTerrainGenerator (), familyId))
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
			
			//-- remove from fractal group
			fractalGroup->removeFamily (familyId);

			//-- delete from tree
			IGNORE_RETURN (GetTreeCtrl ().DeleteItem (family));

			GetDocument ()->SetModifiedFlag ();

			Invalidate ();
		}
	}
}

//-------------------------------------------------------------------

void FractalView::OnUpdateDeletefamily(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (getSelectedFamily () != 0 ? TRUE : FALSE);
}

//-------------------------------------------------------------------

BOOL FractalView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= (TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_EDITLABELS);
	
	return CTreeView::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

void FractalView::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//-- get the tree view display info
	const NMTVDISPINFO* pTVDispInfo = reinterpret_cast<const NMTVDISPINFO*> (pNMHDR);

	//-- disallow editing by default
	*pResult = 1;

	const HTREEITEM selection = pTVDispInfo->item.hItem;

	if (selection)
	{
		//-- limit to 100 characters
		GetTreeCtrl ().GetEditControl()->LimitText (100);

		//-- don't edit
		*pResult = 0;
		return;
	}
}

//-------------------------------------------------------------------

void FractalView::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
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
		const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (pTVDispInfo->item.hItem));
		NOT_NULL (fractalGroup);
		fractalGroup->setFamilyName (familyId, newName);

		//-- tell tree about new name
		IGNORE_RETURN (GetTreeCtrl ().SetItem (&pTVDispInfo->item));
		*pResult = 1;

		//-- tell properties window about new name.
		{
			TerrainEditorDoc* pDoc = safe_cast<TerrainEditorDoc*> (GetDocument());
			HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

			FormFractalFamily::FormFractalFamilyViewData vd;
			vd.familyId     = static_cast<int> (GetTreeCtrl ().GetItemData (selection));
			vd.fractalGroup = fractalGroup;

			pDoc->SetPropertyView (RUNTIME_CLASS(FormFractalFamily), &vd);
		}

	}
	else 
	{
		IGNORE_RETURN (MessageBox("A family with this name already exists.  Please select another name.", "Bad Name"));
		*pResult = 0;
	}

}

//-------------------------------------------------------------------

void FractalView::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult) 
{
	const TV_KEYDOWN* pTVKeyDown = reinterpret_cast<const TV_KEYDOWN*> (pNMHDR);

	switch (pTVKeyDown->wVKey)
	{
	case VK_DELETE:
		{
			//-- same as delete
			OnDeletefamily();
		}
		break;
	}

	*pResult = 0;
}

//-------------------------------------------------------------------

void FractalView::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();

	//-- get the document's fractal group
	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	fractalGroup = &doc->getTerrainGenerator ()->getFractalGroup ();
	NOT_NULL (fractalGroup);

	reset ();
}

//-------------------------------------------------------------------

void FractalView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREF (pNMHDR);

	if (deletingUnused)
		return;

	TerrainEditorDoc* pDoc = safe_cast<TerrainEditorDoc*> (GetDocument());
	HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	if (selection)
	{
		FormFractalFamily::FormFractalFamilyViewData vd;
		vd.familyId     = static_cast<int> (GetTreeCtrl ().GetItemData (selection));
		vd.fractalGroup = fractalGroup;

		pDoc->SetPropertyView (RUNTIME_CLASS (FormFractalFamily), &vd);
	}

	*pResult = 0;
}

//-------------------------------------------------------------------

void FractalView::OnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnSelchanged (pNMHDR, pResult);

	*pResult = 0;
}

//-------------------------------------------------------------------

void FractalView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if (getSelectedFamily () != 0)
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
}

//-------------------------------------------------------------------

void FractalView::OnGroupDelete() 
{
	OnDeletefamily ();
}

//-------------------------------------------------------------------

void FractalView::OnGroupRename() 
{
	IGNORE_RETURN (GetTreeCtrl ().EditLabel (GetTreeCtrl ().GetSelectedItem ()));
}

//-------------------------------------------------------------------

void FractalView::reset ()
{
	IGNORE_RETURN (GetTreeCtrl ().DeleteAllItems ());

	//-- populate tree
	NOT_NULL (fractalGroup);
	int i;
	for (i = 0; i < fractalGroup->getNumberOfFamilies (); i++)
	{
		const int familyId = fractalGroup->getFamilyId (i);

		//-- get family name
		const HTREEITEM familyRoot = GetTreeCtrl ().InsertItem (fractalGroup->getFamilyName (familyId),  0, 1, TVI_ROOT, TVI_SORT);

		//-- set the family data to the family index for easy searching later
		IGNORE_RETURN (GetTreeCtrl ().SetItemData (familyRoot, static_cast<DWORD> (familyId)));
	}

	IGNORE_RETURN (GetTreeCtrl ().SelectItem (GetTreeCtrl ().GetRootItem ()));
	IGNORE_RETURN (GetTreeCtrl ().EnsureVisible (GetTreeCtrl ().GetRootItem()));
}

//-------------------------------------------------------------------

void FractalView::OnButtonDeleteunused() 
{
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

	NOT_NULL (fractalGroup);
	const int originalCount = fractalGroup->getNumberOfFamilies ();
	int n = originalCount;
	int i = 0;

	while (i < n)
	{
		if (TerrainGeneratorHelper::usesFractalFamily (generator, fractalGroup->getFamilyId (i)))
			i++;
		else
		{
			fractalGroup->removeFamily (fractalGroup->getFamilyId (i));
			n--;
		}
	}

	deletingUnused = true;
	reset ();
	deletingUnused = false;
}

//-------------------------------------------------------------------

void FractalView::OnGroupFindrulesusingthisfamily() 
{
	//-- get family
	HTREEITEM family = getSelectedFamily ();

	if (family)
	{
		NOT_NULL (fractalGroup);

		PackedRgb color;

		//-- get familyId
		const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (family));

		TerrainEditorDoc* const doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
		NOT_NULL (doc);

		ArrayList<TerrainGeneratorHelper::OutputData> output;

		TerrainGeneratorHelper::findFractal (doc->getTerrainGenerator (), familyId, output);

		if (output.empty ())
		{
			CString string;
			string.Format ("Family %s not used", fractalGroup->getFamilyName (familyId));

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

void FractalView::OnButtonCopyCurrent() 
{
	//-- get family
	HTREEITEM sourceFamily = getSelectedFamily ();

	if (sourceFamily)
	{
		NOT_NULL (fractalGroup);

		//-- get familyId
		const int     sourceFamilyId   = static_cast<int> (GetTreeCtrl ().GetItemData (sourceFamily));
		const CString sourceFamilyName = fractalGroup->getFamilyName (sourceFamilyId);
	
		//-- create new family
		const int     familyId   = getUniqueFamilyId (fractalGroup);
		const CString baseName   = sourceFamilyName + "_copy";
		const CString familyName = createUniqueFamilyName (&baseName);

		fractalGroup->addFamily (familyId, familyName);
		*fractalGroup->getFamilyMultiFractal (familyId) = *fractalGroup->getFamilyMultiFractal (sourceFamilyId);

		//-- insert into tree
		const HTREEITEM family = GetTreeCtrl ().InsertItem (familyName, 0, 1, TVI_ROOT, TVI_SORT);
		IGNORE_RETURN (GetTreeCtrl ().SetItemData (family, static_cast<DWORD> (familyId)));
		IGNORE_RETURN (GetTreeCtrl ().SelectItem (family));
		GetDocument ()->SetModifiedFlag ();

		Invalidate ();
	}
}

//-------------------------------------------------------------------

void FractalView::OnUpdateButtonCopyCurrent(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (getSelectedFamily () != 0);
}

//-------------------------------------------------------------------

