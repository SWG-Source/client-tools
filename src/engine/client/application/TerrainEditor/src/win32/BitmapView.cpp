//
// BitmapView.cpp
//
// copyright 2004, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "BitmapView.h"

#include "FindFrame.h"
#include "FormBitmapFamily.h"
#include "RecentDirectory.h"
#include "TerrainEditorDoc.h"
#include "TerrainGeneratorHelper.h"
#include "terraineditor.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(BitmapView, CTreeView)

//-------------------------------------------------------------------

BitmapView::BitmapView (void) :
	CTreeView (),
	bitmapGroup (0),
	deletingUnused (false)
{
}

BitmapView::~BitmapView()
{
	bitmapGroup = 0;
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(BitmapView, CTreeView)
	//{{AFX_MSG_MAP(BitmapView)
	ON_COMMAND(ID_NEWFAMILY, OnNewfamily)
	ON_UPDATE_COMMAND_UI(ID_NEWFAMILY, OnUpdateNewfamily)
	ON_COMMAND(ID_DELETEFAMILY, OnDeletefamily)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
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

void BitmapView::OnDraw(CDC* pDC)
{
	UNREF (pDC);
	CDocument* pDoc = GetDocument();
	UNREF (pDoc);
	// TODO: add draw code here
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void BitmapView::AssertValid() const
{
	CTreeView::AssertValid();
}

void BitmapView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

HTREEITEM BitmapView::getSelectedFamily (void) const
{
	return GetTreeCtrl ().GetSelectedItem ();
}

//-------------------------------------------------------------------

static int getUniqueFamilyId (const BitmapGroup* bitmapGroup)
{
	NOT_NULL (bitmapGroup);

	int familyId = 1;
	while (bitmapGroup->hasFamily (familyId))
		++familyId;

	return familyId;
}

//-------------------------------------------------------------------

void BitmapView::addFamily (const CString& familyName)
{
	//-- create new family
	const int familyId = getUniqueFamilyId (bitmapGroup);

	CString newFamilyName = createUniqueFamilyName (&familyName);
	NOT_NULL (bitmapGroup);
	bitmapGroup->addFamily (familyId, newFamilyName,0);

	//-- insert into tree
	const HTREEITEM family = GetTreeCtrl ().InsertItem (newFamilyName, 0, 1, TVI_ROOT, TVI_SORT);
	IGNORE_RETURN (GetTreeCtrl ().SetItemData (family, static_cast<DWORD> (familyId)));

	IGNORE_RETURN (GetTreeCtrl ().SelectItem (family));
	GetDocument ()->SetModifiedFlag ();

	Invalidate ();
}

//-------------------------------------------------------------------

void BitmapView::OnNewfamily() 
{
	//-- create new family
	const int familyId = getUniqueFamilyId (bitmapGroup);
	
	CString familyName = createUniqueFamilyName ();
	NOT_NULL (bitmapGroup);
	bitmapGroup->addFamily (familyId, familyName,0);

	//-- insert into tree
	const HTREEITEM family = GetTreeCtrl ().InsertItem (familyName, 0, 1, TVI_ROOT, TVI_SORT);
	IGNORE_RETURN (GetTreeCtrl ().SetItemData (family, static_cast<DWORD> (familyId)));

	IGNORE_RETURN (GetTreeCtrl ().SelectItem (family));

	OnFindBitmap();
	Invalidate ();
}

//-------------------------------------------------------------------

void BitmapView::OnUpdateNewfamily(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (true);
}  //lint !e1762  //-- pCmdUI could be made a const reference

//-------------------------------------------------------------------

bool BitmapView::familyExists (const char* familyName) const
{
	NOT_NULL (bitmapGroup);
	const int n = bitmapGroup->getNumberOfFamilies ();

	int i;
	for (i = 0; i < n; ++i)
	{
		const int familyId = bitmapGroup->getFamilyId (i);

		if (_stricmp (familyName, bitmapGroup->getFamilyName (familyId)) == 0)
			return true;
	}

	return false;
}

//-------------------------------------------------------------------

const CString BitmapView::createUniqueFamilyName (const CString* base) const
{
	CString familyName;
	int     familyIndex = 1;

	if (base)
	{
		familyName.Format ("%s", *base);
	}
	else
	{
		NOT_NULL (bitmapGroup);
		familyIndex = bitmapGroup->getNumberOfFamilies ();

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

void BitmapView::OnDeletefamily() 
{
	HTREEITEM family = getSelectedFamily ();

	if (family)
	{
		const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (family));
		NOT_NULL (bitmapGroup);
		const char* name   = bitmapGroup->getFamilyName (familyId);

		TerrainEditorDoc* const pDoc = safe_cast<TerrainEditorDoc*> (GetDocument());
		if (TerrainGeneratorHelper::usesBitmapFamily (pDoc->getTerrainGenerator (), familyId))
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
			
			//-- remove from bitmap group
			bitmapGroup->removeFamily (familyId);

			//-- delete from tree
			IGNORE_RETURN (GetTreeCtrl ().DeleteItem (family));

			GetDocument ()->SetModifiedFlag ();

			Invalidate ();
		}
	}
}

//-------------------------------------------------------------------

void BitmapView::OnUpdateDeletefamily(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (getSelectedFamily () != 0 ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void BitmapView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREF (pNMHDR);

	*pResult = 0;

	//-- same as find target
	OnFindBitmap ();
}
//-------------------------------------------------------------------

BOOL BitmapView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= (TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_EDITLABELS);
	
	return CTreeView::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

void BitmapView::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
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

void BitmapView::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
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
		NOT_NULL (bitmapGroup);
		bitmapGroup->setFamilyName (familyId, newName);

		//-- tell tree about new name
		IGNORE_RETURN (GetTreeCtrl ().SetItem (&pTVDispInfo->item));
		*pResult = 1;

		//-- tell properties window about new name.
		{
			TerrainEditorDoc* pDoc = safe_cast<TerrainEditorDoc*> (GetDocument());
			HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

			FormBitmapFamily::FormBitmapFamilyViewData vd;
			vd.familyId     = static_cast<int> (GetTreeCtrl ().GetItemData (selection));
			vd.bitmapGroup = bitmapGroup;

			pDoc->SetPropertyView (RUNTIME_CLASS(FormBitmapFamily), &vd);
		}

	}
	else 
	{
		IGNORE_RETURN (MessageBox("A family with this name already exists.  Please select another name.", "Bad Name"));
		*pResult = 0;
	}

}

//-------------------------------------------------------------------

void BitmapView::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult) 
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

void BitmapView::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();

	//-- get the document's bitmap group
	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	bitmapGroup = &doc->getTerrainGenerator ()->getBitmapGroup ();
	NOT_NULL (bitmapGroup);

	reset ();
}

//-------------------------------------------------------------------

void BitmapView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREF (pNMHDR);

	if (deletingUnused)
		return;

	TerrainEditorDoc* pDoc = safe_cast<TerrainEditorDoc*> (GetDocument());
	HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	if (selection)
	{
		FormBitmapFamily::FormBitmapFamilyViewData vd;
		vd.familyId     = static_cast<int> (GetTreeCtrl ().GetItemData (selection));
		vd.bitmapGroup = bitmapGroup;

		pDoc->SetPropertyView (RUNTIME_CLASS (FormBitmapFamily), &vd);
	}

	*pResult = 0;
}

//-------------------------------------------------------------------

void BitmapView::OnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnSelchanged (pNMHDR, pResult);

	*pResult = 0;
}

//-------------------------------------------------------------------

void BitmapView::OnRButtonDown(UINT nFlags, CPoint point) 
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

void BitmapView::OnGroupDelete() 
{
	OnDeletefamily ();
}

//-------------------------------------------------------------------

void BitmapView::OnGroupRename() 
{
	IGNORE_RETURN (GetTreeCtrl ().EditLabel (GetTreeCtrl ().GetSelectedItem ()));
}

//-------------------------------------------------------------------

void BitmapView::reset ()
{
	IGNORE_RETURN (GetTreeCtrl ().DeleteAllItems ());

	//-- populate tree
	NOT_NULL (bitmapGroup);
	int i;
	for (i = 0; i < bitmapGroup->getNumberOfFamilies (); i++)
	{
		const int familyId = bitmapGroup->getFamilyId (i);

		//-- get family name
		const HTREEITEM familyRoot = GetTreeCtrl ().InsertItem (bitmapGroup->getFamilyName (familyId),  0, 1, TVI_ROOT, TVI_SORT);

		//-- set the family data to the family index for easy searching later
		IGNORE_RETURN (GetTreeCtrl ().SetItemData (familyRoot, static_cast<DWORD> (familyId)));
	}

	IGNORE_RETURN (GetTreeCtrl ().SelectItem (GetTreeCtrl ().GetRootItem ()));
	IGNORE_RETURN (GetTreeCtrl ().EnsureVisible (GetTreeCtrl ().GetRootItem()));
}

//-------------------------------------------------------------------

void BitmapView::OnButtonDeleteunused() 
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

	NOT_NULL (bitmapGroup);
	const int originalCount = bitmapGroup->getNumberOfFamilies ();
	int n = originalCount;
	int i = 0;

	while (i < n)
	{
		if (TerrainGeneratorHelper::usesBitmapFamily (generator, bitmapGroup->getFamilyId (i)))
			i++;
		else
		{
			bitmapGroup->removeFamily (bitmapGroup->getFamilyId (i));
			n--;
		}
	}

	deletingUnused = true;
	reset ();
	deletingUnused = false;
}

//-------------------------------------------------------------------

void BitmapView::OnGroupFindrulesusingthisfamily() 
{
	//-- get family
	HTREEITEM family = getSelectedFamily ();

	if (family)
	{
		NOT_NULL (bitmapGroup);

		PackedRgb color;

		//-- get familyId
		const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (family));

		TerrainEditorDoc* const doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
		NOT_NULL (doc);

		ArrayList<TerrainGeneratorHelper::OutputData> output;

		TerrainGeneratorHelper::findBitmap (doc->getTerrainGenerator (), familyId, output);

		if (output.empty ())
		{
			CString string;
			string.Format ("Family %s not used", bitmapGroup->getFamilyName (familyId));

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

void BitmapView::OnButtonCopyCurrent() 
{

}

//-------------------------------------------------------------------

void BitmapView::OnUpdateButtonCopyCurrent(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (getSelectedFamily () != 0);
}

//-------------------------------------------------------------------

void BitmapView::OnFindBitmap() 
{
	CFileDialog dlg (true, "*.tga", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR));
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("Targa");
	dlg.m_ofn.lpstrTitle      = "Select Bitmap File";

	bool stillTrying = true;
	bool badName     = true;

	while (badName && stillTrying)
	{
		if (dlg.DoModal () == IDOK)
		{
			RecentDirectory::update ("Targa", dlg.GetPathName ());

			//-- get new name
			const CString newName = dlg.GetFileTitle ();

			//-- get child
			const HTREEITEM selectedFamily = getSelectedFamily ();

			if (selectedFamily)
			{
				const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (selectedFamily));
					
				badName = false;

				//-- get old name
				const CString oldName = GetTreeCtrl ().GetItemText (selectedFamily);	

				//-- refresh the property page because it sends changes back via child name.
				//-- Remove current shader form this BEFORE name changes so current form values can be set.
				TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
				PropertyView::ViewData dummyData;
				pDoc->SetPropertyView (RUNTIME_CLASS(PropertyView), &dummyData);

				//-- rename in bitmap group
				bitmapGroup->setFamilyName (familyId, newName);
				bitmapGroup->loadFamilyBitmap(familyId, newName);

				//-- rename in tree
				GetTreeCtrl ().SetItemText (selectedFamily, newName);

				GetDocument ()->SetModifiedFlag ();

				Invalidate ();

				//-- tell the properties view about it
				{
					TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
							
					FormBitmapFamily::FormBitmapFamilyViewData vd;
					vd.familyId = GetTreeCtrl ().GetItemData (selectedFamily);
					vd.bitmapGroup = bitmapGroup;

					pDoc->SetPropertyView (RUNTIME_CLASS(FormBitmapFamily), &vd);
				}
					
			}

		}
		else
		{
			stillTrying = false;
		}
	}
}

