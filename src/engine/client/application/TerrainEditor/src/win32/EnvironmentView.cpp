//
// EnvironmentView.cpp
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "EnvironmentView.h"

#include "EnvironmentFrame.h"
#include "FindFrame.h"
#include "FormEnvironmentFamily.h"
#include "TerrainEditorDoc.h"
#include "TerrainGeneratorHelper.h"
#include "terraineditor.h"

#include "fileInterface/StdioFile.h"
#include "sharedUtility/FileName.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(EnvironmentView, CTreeView)

//-------------------------------------------------------------------

EnvironmentView::EnvironmentView (void) :
	CTreeView (),
	environmentGroup (0),
	deletingUnused (false)
{
}

EnvironmentView::~EnvironmentView()
{
	environmentGroup = 0;
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(EnvironmentView, CTreeView)
	//{{AFX_MSG_MAP(EnvironmentView)
	ON_COMMAND(ID_NEWENVIRONMENTFAMILY, OnNewenvironmentfamily)
	ON_UPDATE_COMMAND_UI(ID_NEWENVIRONMENTFAMILY, OnUpdateNewenvironmentfamily)
	ON_COMMAND(ID_DELETEENVIRONMENT, OnDeleteenvironment)
	ON_UPDATE_COMMAND_UI(ID_DELETEENVIRONMENT, OnUpdateDeleteenvironment)
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

void EnvironmentView::OnDraw(CDC* pDC)
{
	UNREF (pDC);
	CDocument* pDoc = GetDocument();
	UNREF (pDoc);
	// TODO: add draw code here
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void EnvironmentView::AssertValid() const
{
	CTreeView::AssertValid();
}

void EnvironmentView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

HTREEITEM EnvironmentView::getSelectedFamily (void) const
{
	return GetTreeCtrl ().GetSelectedItem ();
}

//-------------------------------------------------------------------

static int getUniqueFamilyId (const EnvironmentGroup* environmentGroup)
{
	NOT_NULL (environmentGroup);

	int familyId = 1;

	while (environmentGroup->hasFamily (familyId))
		++familyId;

	return familyId;
}

//-------------------------------------------------------------------

void EnvironmentView::addFamily (const CString& familyName)
{
	//-- create new family
	const int familyId = getUniqueFamilyId (environmentGroup);

	CString newFamilyName = CreateUniqueFamilyName (&familyName);
	NOT_NULL (environmentGroup);
	environmentGroup->addFamily (familyId, newFamilyName, PackedRgb::solidYellow);

	//-- insert into tree
	const HTREEITEM family = GetTreeCtrl ().InsertItem (newFamilyName, 0, 1, TVI_ROOT, TVI_SORT);
	IGNORE_RETURN (GetTreeCtrl ().SetItemData (family, static_cast<DWORD> (familyId)));
	IGNORE_RETURN (GetTreeCtrl ().SelectItem (family));
	GetDocument ()->SetModifiedFlag ();

	Invalidate ();
}

//-------------------------------------------------------------------

void EnvironmentView::OnNewenvironmentfamily() 
{
	//-- create new family
	const int familyId = getUniqueFamilyId (environmentGroup);
	
	CString familyName = CreateUniqueFamilyName ();
	NOT_NULL (environmentGroup);
	environmentGroup->addFamily (familyId, familyName, PackedRgb::solidYellow);

	//-- insert into tree
	const HTREEITEM family = GetTreeCtrl ().InsertItem (familyName, 0, 1, TVI_ROOT, TVI_SORT);
	IGNORE_RETURN (GetTreeCtrl ().SetItemData (family, static_cast<DWORD> (familyId)));
	IGNORE_RETURN (GetTreeCtrl ().SelectItem (family));
	GetDocument ()->SetModifiedFlag ();

	Invalidate ();
}

//-------------------------------------------------------------------

void EnvironmentView::OnUpdateNewenvironmentfamily(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (true);
}

//-------------------------------------------------------------------

bool EnvironmentView::familyExists (const char* familyName) const
{
	NOT_NULL (environmentGroup);

	const int n = environmentGroup->getNumberOfFamilies ();

	int i;
	for (i = 0; i < n; ++i)
	{
		const int familyId = environmentGroup->getFamilyId (i);

		if (_stricmp (familyName, environmentGroup->getFamilyName (familyId)) == 0)
			return true;
	}

	return false;
}

//-------------------------------------------------------------------

const CString EnvironmentView::CreateUniqueFamilyName (const CString* base) const
{
	NOT_NULL (environmentGroup);

	CString familyName;
	int     familyIndex = 1;

	if (base)
	{
		familyName.Format ("%s", *base);
	}
	else
	{
		familyIndex = environmentGroup->getNumberOfFamilies ();

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

void EnvironmentView::OnDeleteenvironment() 
{
	HTREEITEM family = getSelectedFamily ();

	if (family)
	{
		NOT_NULL (environmentGroup);

		const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (family));
		const char* name   = environmentGroup->getFamilyName (familyId);

		TerrainEditorDoc* const pDoc = safe_cast<TerrainEditorDoc*> (GetDocument());
		if (TerrainGeneratorHelper::usesEnvironmentFamily (pDoc->getTerrainGenerator (), familyId))
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
			
			//-- remove from environment group
			environmentGroup->removeFamily (familyId);

			//-- delete from tree
			IGNORE_RETURN (GetTreeCtrl ().DeleteItem (family));

			GetDocument ()->SetModifiedFlag ();

			Invalidate ();
		}
	}
}

//-------------------------------------------------------------------

void EnvironmentView::OnUpdateDeleteenvironment(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (getSelectedFamily () != 0 ? TRUE : FALSE);
}

//-------------------------------------------------------------------

BOOL EnvironmentView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= (TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_EDITLABELS);
	
	return CTreeView::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

void EnvironmentView::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
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

void EnvironmentView::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//-- get the tree view display info
	NMTVDISPINFO* pTVDispInfo = reinterpret_cast<NMTVDISPINFO*> (pNMHDR);

	//-- get the new text string
	CString newName (pTVDispInfo->item.pszText ? pTVDispInfo->item.pszText : "");

	if (newName.GetLength () == 0)
	{
		*pResult = 0;
		return;
	}

	//-- change name
	const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (pTVDispInfo->item.hItem));
	const CString oldFamilyName (environmentGroup->getFamilyName (familyId));

	if (newName.CompareNoCase (oldFamilyName) == 0 || !familyExists (newName))
	{
		NOT_NULL (environmentGroup);

		//-- change name
		const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (pTVDispInfo->item.hItem));
		environmentGroup->setFamilyName (familyId, newName);

		//-- tell tree about new name
		IGNORE_RETURN (GetTreeCtrl ().SetItem (&pTVDispInfo->item));
		*pResult = 1;

		//-- tell properties window about new name.
		{
			TerrainEditorDoc* pDoc = safe_cast<TerrainEditorDoc*> (GetDocument());
			HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

			FormEnvironmentFamily::FormEnvironmentFamilyViewData vd;
			vd.familyId         = static_cast<int> (GetTreeCtrl ().GetItemData (selection));
			vd.environmentGroup = environmentGroup;

			pDoc->SetPropertyView (RUNTIME_CLASS(FormEnvironmentFamily), &vd);
		}
	}
	else
	{
		IGNORE_RETURN (MessageBox ("A family with this name already exists.  Please select another name.", "Bad Name"));
		*pResult = 0;
	}
}

//-------------------------------------------------------------------

void EnvironmentView::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult) 
{
	const TV_KEYDOWN* pTVKeyDown = reinterpret_cast<const TV_KEYDOWN*> (pNMHDR);

	switch (pTVKeyDown->wVKey)
	{
	case VK_DELETE:
		{
			//-- same as delete
			OnDeleteenvironment();
		}
		break;
	}

	*pResult = 0;
}

//-------------------------------------------------------------------

void EnvironmentView::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();

	//-- get the document's environment group
	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	environmentGroup = &doc->getTerrainGenerator ()->getEnvironmentGroup ();
	NOT_NULL (environmentGroup);

	reset ();
}

//-------------------------------------------------------------------

void EnvironmentView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREF (pNMHDR);

	if (deletingUnused)
		return;

	TerrainEditorDoc* pDoc = safe_cast<TerrainEditorDoc*> (GetDocument());
	HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	if (selection)
	{
		FormEnvironmentFamily::FormEnvironmentFamilyViewData vd;
		vd.familyId         = static_cast<int> (GetTreeCtrl ().GetItemData (selection));
		vd.environmentGroup = environmentGroup;

		pDoc->SetPropertyView (RUNTIME_CLASS (FormEnvironmentFamily), &vd);
	}

	*pResult = 0;
}

//-------------------------------------------------------------------

void EnvironmentView::OnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnSelchanged (pNMHDR, pResult);

	*pResult = 0;
}

//-------------------------------------------------------------------

void EnvironmentView::OnRButtonDown(UINT nFlags, CPoint point) 
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

void EnvironmentView::OnGroupDelete() 
{
	OnDeleteenvironment ();
}

//-------------------------------------------------------------------

void EnvironmentView::OnGroupRename() 
{
	IGNORE_RETURN (GetTreeCtrl ().EditLabel (GetTreeCtrl ().GetSelectedItem ()));
}

//-------------------------------------------------------------------

void EnvironmentView::reset () const 
{
	IGNORE_RETURN (GetTreeCtrl ().DeleteAllItems ());

	//-- populate tree
	NOT_NULL (environmentGroup);

	int i;
	for (i = 0; i < environmentGroup->getNumberOfFamilies (); i++)
	{
		const int familyId = environmentGroup->getFamilyId (i);

		//-- get family name
		const HTREEITEM familyRoot = GetTreeCtrl ().InsertItem (environmentGroup->getFamilyName (familyId),  0, 1, TVI_ROOT, TVI_SORT);

		//-- set the family data to the family index for easy searching later
		IGNORE_RETURN (GetTreeCtrl ().SetItemData (familyRoot, static_cast<DWORD> (familyId)));
	}

	IGNORE_RETURN (GetTreeCtrl ().SelectItem (GetTreeCtrl ().GetRootItem ()));
	IGNORE_RETURN (GetTreeCtrl ().EnsureVisible (GetTreeCtrl ().GetRootItem()));
}

//-------------------------------------------------------------------

void EnvironmentView::OnButtonDeleteunused() 
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
	NOT_NULL (environmentGroup);

	const int originalCount = environmentGroup->getNumberOfFamilies ();
	int n = originalCount;
	int i = 0;

	while (i < n)
	{
		if (TerrainGeneratorHelper::usesEnvironmentFamily (generator, environmentGroup->getFamilyId (i)))
			i++;
		else
		{
			environmentGroup->removeFamily (environmentGroup->getFamilyId (i));
			n--;
		}
	}

	deletingUnused = true;
	reset ();
	deletingUnused = false;
}

//-------------------------------------------------------------------

void EnvironmentView::OnGroupFindrulesusingthisfamily() 
{
	//-- get family
	HTREEITEM family = getSelectedFamily ();

	if (family)
	{
		NOT_NULL (environmentGroup);

		PackedRgb color;

		//-- get familyId
		const int familyId = static_cast<int> (GetTreeCtrl ().GetItemData (family));

		TerrainEditorDoc* const doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
		NOT_NULL (doc);

		ArrayList<TerrainGeneratorHelper::OutputData> output;

		TerrainGeneratorHelper::findEnvironment (doc->getTerrainGenerator (), familyId, output);

		if (output.empty ())
		{
			CString string;
			string.Format ("Family %s not used", environmentGroup->getFamilyName (familyId));

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

void EnvironmentView::OnButtonCopyCurrent() 
{
	//-- get family
	HTREEITEM sourceFamily = getSelectedFamily ();

	if (sourceFamily)
	{
		NOT_NULL (environmentGroup);

		PackedRgb color;

		//-- get familyId
		const int     sourceFamilyId   = static_cast<int> (GetTreeCtrl ().GetItemData (sourceFamily));
		const CString sourceFamilyName = environmentGroup->getFamilyName (sourceFamilyId);
	
		//-- create new family
		const int     familyId   = getUniqueFamilyId (environmentGroup);
		const CString baseName   = sourceFamilyName + "_copy";
		const CString familyName = CreateUniqueFamilyName (&baseName);

		environmentGroup->addFamily (familyId, familyName, environmentGroup->getFamilyColor (sourceFamilyId));
		environmentGroup->copyFamily (familyId, sourceFamilyId);

		//-- insert into tree
		const HTREEITEM family = GetTreeCtrl ().InsertItem (familyName, 0, 1, TVI_ROOT, TVI_SORT);
		IGNORE_RETURN (GetTreeCtrl ().SetItemData (family, static_cast<DWORD> (familyId)));
		IGNORE_RETURN (GetTreeCtrl ().SelectItem (family));
		GetDocument ()->SetModifiedFlag ();

		Invalidate ();
	}
}

//-------------------------------------------------------------------

void EnvironmentView::OnUpdateButtonCopyCurrent(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (getSelectedFamily () != 0);
}

//-------------------------------------------------------------------
