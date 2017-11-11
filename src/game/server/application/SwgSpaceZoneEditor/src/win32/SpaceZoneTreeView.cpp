// ======================================================================
//
// SpaceZoneTreeView.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceZoneEditor.h"
#include "SpaceZoneTreeView.h"

#include "ChildFrame.h"
#include "DialogObjectName.h"
#include "DialogProperties.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Transform.h"
#include "SpaceZoneMapView.h"
#include "SwgSpaceZoneEditorDoc.h"

// ======================================================================

IMPLEMENT_DYNCREATE(SpaceZoneTreeView, CTreeView)

BEGIN_MESSAGE_MAP(SpaceZoneTreeView, CTreeView)
	//{{AFX_MSG_MAP(SpaceZoneTreeView)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID__PROPERTIES, OnProperties)
	ON_COMMAND(ID__DELETE, OnDelete)
	ON_COMMAND(ID__RENAME, OnRename)
	ON_COMMAND(ID__DUPLICATE, OnDuplicate)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginlabeledit)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndlabeledit)
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeydown)
	ON_COMMAND(ID__SPAWNER_ADDPATROLPOINT, OnSpawnerAddpatrolpoint)
	ON_COMMAND(ID__SPAWNER_SELECTALLPATROLPOINTS, OnSpawnerSelectallpatrolpoints)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CTreeView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CTreeView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CTreeView::OnFilePrintPreview)
	ON_COMMAND(ID_SPAWNER_DUPLICATE, &SpaceZoneTreeView::OnSpawnerDuplicate)
END_MESSAGE_MAP()

// ======================================================================

SpaceZoneTreeView::SpaceZoneTreeView() :
	m_navPointRoot(0),
	m_spawnerRoot(0),
	m_miscRoot(0)
{
}

// ----------------------------------------------------------------------

SpaceZoneTreeView::~SpaceZoneTreeView()
{
}

// ----------------------------------------------------------------------

BOOL SpaceZoneTreeView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= (TVS_SHOWSELALWAYS | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_DISABLEDRAGDROP);

	return CTreeView::PreCreateWindow(cs);
}

// ----------------------------------------------------------------------

void SpaceZoneTreeView::OnDraw(CDC * const /*pDC*/)
{
}

// ----------------------------------------------------------------------

BOOL SpaceZoneTreeView::OnPreparePrinting(CPrintInfo* pInfo)
{
	return DoPreparePrinting(pInfo);
}

// ----------------------------------------------------------------------

void SpaceZoneTreeView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

// ----------------------------------------------------------------------

void SpaceZoneTreeView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

// ----------------------------------------------------------------------

void SpaceZoneTreeView::addNode(HTREEITEM parent, HTREEITEM after, SwgSpaceZoneEditorDoc::Object const * const object)
{
	NOT_NULL(object);
	HTREEITEM item = GetTreeCtrl().InsertItem(object->getName(), parent, after);
	GetTreeCtrl().SetItemData(item, reinterpret_cast<DWORD>(object));

	if (object->isPatrolPointSpawner())
	{
		SwgSpaceZoneEditorDoc const * const document = safe_cast<SwgSpaceZoneEditorDoc const *>(GetDocument());

		StringList patrolPointList;
		Configuration::unpackString(object->getPatrolPoints(), patrolPointList, ':');
		for (size_t i = 0; i < patrolPointList.size(); ++i)
		{
			SwgSpaceZoneEditorDoc::Object const * const patrolPoint = document->findPatrolPoint(patrolPointList[i]);
			if (patrolPoint)
				addNode(item, TVI_LAST, patrolPoint);
		}
	}
}

// ----------------------------------------------------------------------

void SpaceZoneTreeView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

	SwgSpaceZoneEditorDoc * const document = safe_cast<SwgSpaceZoneEditorDoc *>(GetDocument());

	m_navPointRoot = GetTreeCtrl().InsertItem("Nav Points", TVI_ROOT);
	m_spawnerRoot = GetTreeCtrl().InsertItem("Spawners", TVI_ROOT);
	m_miscRoot = GetTreeCtrl().InsertItem("Miscellaneous", TVI_ROOT);

	int const numberOfObjects = document->getNumberOfObjects();
	if (!numberOfObjects)
		return;

	{
		int i;
		for (i = 0; i < numberOfObjects; ++i)
		{
			SwgSpaceZoneEditorDoc::Object const * const object = document->getObject(i);

			switch (object->getType())
			{
			case SwgSpaceZoneEditorDoc::Object::T_navPoint:
				addNode(m_navPointRoot, TVI_SORT, object);
				break;

			case SwgSpaceZoneEditorDoc::Object::T_spawner:
				addNode(m_spawnerRoot, TVI_SORT, object);
				break;

			case SwgSpaceZoneEditorDoc::Object::T_misc:
				addNode(m_miscRoot, TVI_SORT, object);
				break;
			}
		}
	}
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

void SpaceZoneTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void SpaceZoneTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

#endif //_DEBUG

// ----------------------------------------------------------------------

void SpaceZoneTreeView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (pSender != this)
	{
		SwgSpaceZoneEditorDoc::Object const * const object = reinterpret_cast<SwgSpaceZoneEditorDoc::Object const *>(pHint);

		if (lHint == SwgSpaceZoneEditorDoc::H_selectObject)
			selectObject(object);


		if (lHint == SwgSpaceZoneEditorDoc::H_deleteObject)
		{
			HTREEITEM item = find(GetTreeCtrl().GetRootItem(), object);

			HTREEITEM itemToSelect = GetTreeCtrl().GetPrevSiblingItem(item);
			if (!itemToSelect)
				itemToSelect = GetTreeCtrl().GetNextSiblingItem(item);

			if (!itemToSelect)
				itemToSelect = GetTreeCtrl().GetParentItem(item);

			GetTreeCtrl ().DeleteItem(item);
			selectItem(itemToSelect);
		}

		if (lHint == SwgSpaceZoneEditorDoc::H_addPatrolPoint)
		{
			SwgSpaceZoneEditorDoc::Object const * const spawner = reinterpret_cast<SwgSpaceZoneEditorDoc::Object const *>(object->getParent());

			HTREEITEM item = find(GetTreeCtrl().GetRootItem(), spawner);
			addNode(item, TVI_LAST, object);
		}

		if (lHint == SwgSpaceZoneEditorDoc::H_addNavPoint)
		{
			addNode(m_navPointRoot, TVI_SORT, object);			
		}

		if (lHint == SwgSpaceZoneEditorDoc::H_addSpawner)
		{
			addNode(m_spawnerRoot, TVI_SORT, object);			
		}

		if (lHint == SwgSpaceZoneEditorDoc::H_addMisc)
		{
			addNode(m_miscRoot, TVI_SORT, object);			
		}
	}
}

// ----------------------------------------------------------------------

#if 0
static HTREEITEM getRoot(CTreeCtrl& m_treeCtrl, HTREEITEM hitem)
{
	HTREEITEM parent         = hitem;
	HTREEITEM possibleParent = hitem;

	do
	{
		possibleParent = m_treeCtrl.GetParentItem(possibleParent);

		if (possibleParent)
			parent = possibleParent;
	}
	while (possibleParent != 0);

	return parent;
}
#endif

// ----------------------------------------------------------------------

void SpaceZoneTreeView::showInformation()
{
	HTREEITEM selection = GetTreeCtrl().GetSelectedItem();
	if (selection)
	{
		SwgSpaceZoneEditorDoc::Object * const object = reinterpret_cast<SwgSpaceZoneEditorDoc::Object *>(GetTreeCtrl().GetItemData(selection));
		if (object)
		{
			CString objVars;
			Configuration::packObjVars(object->getObjVarList(), objVars);
			DialogProperties dlg(object->getOriginalObjVars(), objVars, object->getScripts());
			if (dlg.DoModal() == IDOK)
			{
				if (dlg.m_scripts != object->getScripts())
				{
					object->setScripts(dlg.m_scripts);
					GetDocument()->SetModifiedFlag();
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

bool SpaceZoneTreeView::hasInformation() const
{
	HTREEITEM selection = GetTreeCtrl().GetSelectedItem();
	return GetTreeCtrl().GetParentItem(selection) != 0;
}

// ----------------------------------------------------------------------

HTREEITEM SpaceZoneTreeView::find(HTREEITEM hItem, SwgSpaceZoneEditorDoc::Object const * const object) const
{
	HTREEITEM theItem = NULL;

	if (hItem == NULL) 
		return NULL;

	if ((theItem = GetTreeCtrl().GetNextSiblingItem(hItem)) != NULL) 
	{
		theItem = find(theItem, object);

		if(theItem != NULL) 
			return theItem;
	}

	if ((theItem = GetTreeCtrl().GetChildItem(hItem)) != NULL) 
	{		
		theItem = find(theItem, object);

		if(theItem != NULL) 
			return theItem;
	}

	SwgSpaceZoneEditorDoc::Object const * const item = reinterpret_cast<SwgSpaceZoneEditorDoc::Object const *>(GetTreeCtrl().GetItemData(hItem));

	if (item == object)
		return hItem;

	return theItem;
}

//-------------------------------------------------------------------

void SpaceZoneTreeView::selectObject(SwgSpaceZoneEditorDoc::Object const * const object)
{
	//-- find htreeitem accociated with layer
	HTREEITEM item;

	if (!object)
		item = GetTreeCtrl().GetParentItem(GetTreeCtrl().GetSelectedItem());
	else
		item = find(GetTreeCtrl().GetRootItem(), object);

	if (item )
		selectItem(item);
}

// ----------------------------------------------------------------------

void SpaceZoneTreeView::selectItem(HTREEITEM item)
{
	if (item)
		GetTreeCtrl().SelectItem(item);
}

// ----------------------------------------------------------------------

void SpaceZoneTreeView::validate()
{
	SwgSpaceZoneEditorDoc const * const document = safe_cast<SwgSpaceZoneEditorDoc const *>(GetDocument());
	int const numberOfObjects = document->getNumberOfObjects();
	if (!numberOfObjects)
	{
		MessageBox("No objects to validate");
		return;
	}

	CString warning;

	bool first = true;

	AxialBox const zoneLimit(Vector::xyz111 * (-8192.f + 512.f), Vector::xyz111 * (8192.f - 512.f));
	for (int i = 0; i < numberOfObjects; ++i)
	{
		SwgSpaceZoneEditorDoc::Object const * const object = document->getObject(i);
		Vector const & position = object->getTransform_o2w().getPosition_p();
		if (!zoneLimit.contains(position))
		{
			if (first)
			{
				selectObject(object);
				centerSelectedItem();
				first = false;
			}

			CString buffer;
			buffer.Format("%s is out of bounds at <%1.2f, %1.2f, %1.2f>\r\n", object->getName(), position.x, position.y, position.z);

			warning += buffer;
		}
	}

	if (!warning.IsEmpty())
		MessageBox(warning, "Objects out of zone boundary");
	else
		MessageBox("All objects are within the zone boundary");
}

// ----------------------------------------------------------------------
void SpaceZoneTreeView::perforceEdit()
{
	SwgSpaceZoneEditorDoc * const document = safe_cast<SwgSpaceZoneEditorDoc *>(GetDocument());
	
	// Make sure we have a file opened.
	if(document->getCurrentFile().IsEmpty())
	{
		MessageBox("No file is currently opened. You must open the file before checking it out.", "Warning", MB_ICONWARNING | MB_OK);
		return;
	}

	// Check out the tab file
	CString editCommand("p4 edit ");
	editCommand += document->getCurrentFile();

	// Grab the file name again so we can replace certain sections and get our .IFF counterpart.
	CString secondCommand = document->getCurrentFile();

	//-- Replace dsrc w/ data
	secondCommand.Replace("dsrc", "data");

	//-- Replace .tab w/ .iff
	secondCommand.Replace(".tab", ".iff");

	// Full command string
	editCommand += " " + secondCommand;

	// Execute the p4 edit command
	system(editCommand.GetString());

}

// ----------------------------------------------------------------------

void SpaceZoneTreeView::OnSelchanged(NMHDR * const /*pNMHDR*/, LRESULT * const pResult) 
{
	HTREEITEM selection = GetTreeCtrl().GetSelectedItem();
	if (selection)
	{
		SwgSpaceZoneEditorDoc::Object const * object = reinterpret_cast<SwgSpaceZoneEditorDoc::Object const *>(GetTreeCtrl().GetItemData(selection));
		safe_cast<SwgSpaceZoneEditorDoc *>(GetDocument())->selectObject(object);
	}
	
	*pResult = 0;
}

// ----------------------------------------------------------------------

void SpaceZoneTreeView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CTreeView::OnLButtonDown(nFlags, point);

	HTREEITEM selection = GetTreeCtrl().HitTest(point);
	if (selection)
	{
		if (selection != GetTreeCtrl().GetSelectedItem())
			GetTreeCtrl().SelectItem(selection);
	}
}

// ----------------------------------------------------------------------

void SpaceZoneTreeView::OnRButtonDown(UINT const nFlags, CPoint const point) 
{
	CTreeView::OnRButtonDown(nFlags, point);

	HTREEITEM selection = GetTreeCtrl().HitTest(point);
	if (selection)
	{
		if (selection != GetTreeCtrl().GetSelectedItem())
			GetTreeCtrl().SelectItem(selection);

		if (GetTreeCtrl().GetParentItem(selection))
		{
			SwgSpaceZoneEditorDoc::Object const * object = reinterpret_cast<SwgSpaceZoneEditorDoc::Object const *>(GetTreeCtrl().GetItemData(selection));

			DWORD menuBranch = 0;
			if (object->isPatrolPoint() || object->isPatrolPointSpawner())
				menuBranch = 1;

			CPoint pt = point;
			ClientToScreen(&pt);

			CMenu menu;
			menu.LoadMenu(IDR_MENU_TREE);

			CMenu * const rootMenu = menu.GetSubMenu(0);
			CMenu * const subMenu = rootMenu->GetSubMenu(menuBranch);
			subMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, this);
		}	
	}
}

// ----------------------------------------------------------------------

void SpaceZoneTreeView::OnProperties() 
{
	showInformation();
}

// ----------------------------------------------------------------------

void SpaceZoneTreeView::centerSelectedItem() 
{
	HTREEITEM selection = GetTreeCtrl().GetSelectedItem();
	if (selection)
	{
		SwgSpaceZoneEditorDoc::Object const * const object = reinterpret_cast<SwgSpaceZoneEditorDoc::Object const *>(GetTreeCtrl().GetItemData(selection));
		if (object)
			GetDocument()->UpdateAllViews(this, SwgSpaceZoneEditorDoc::H_selectObject, reinterpret_cast<CObject *>(const_cast<SwgSpaceZoneEditorDoc::Object *>(object)));
	}
}

// ----------------------------------------------------------------------

void SpaceZoneTreeView::OnDelete() 
{
	HTREEITEM selection = GetTreeCtrl().GetSelectedItem();
	if (selection)
	{
		SwgSpaceZoneEditorDoc * const document = safe_cast<SwgSpaceZoneEditorDoc *>(GetDocument());
		SwgSpaceZoneEditorDoc::Object * const object = reinterpret_cast<SwgSpaceZoneEditorDoc::Object *>(GetTreeCtrl().GetItemData(selection));
		if (object && MessageBox(CString("Delete ") + object->getName(), 0, MB_YESNO) == IDYES)
			document->deleteObject(object);
	}
}

// ----------------------------------------------------------------------

void SpaceZoneTreeView::OnRename() 
{
	// TODO: Add your command handler code here
	
}

// ----------------------------------------------------------------------

void SpaceZoneTreeView::OnDuplicate()
{
	// Just make sure we have a selection
	HTREEITEM selection = GetTreeCtrl().GetSelectedItem();
	if (selection)
	{
		// Bring up the ObjectName Dialog
		DialogObjectName nameDlg;

		// If we hit okay and we passed in something for the name
		if (nameDlg.DoModal() == IDOK && !nameDlg.GetNameField().IsEmpty())
		{
			SwgSpaceZoneEditorDoc * const document = safe_cast<SwgSpaceZoneEditorDoc *>(GetDocument());
			SwgSpaceZoneEditorDoc::Object * const object = reinterpret_cast<SwgSpaceZoneEditorDoc::Object *>(GetTreeCtrl().GetItemData(selection));
			// Duplicate
			document->duplicateObject(object, nameDlg.GetNameField());
		}
		else
		{
			// We had some error. Either the user hit CANCEL, or didn't supply a name.
			MessageBox(CString("Duplicate failed."));
		}


	}
}

/*
void ScriptTreeView::deleteLibrary (HTREEITEM treeItem)
{
	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	Conversation::LibrarySet & librarySet = document->getLibrarySet ();

	std::string libraryName (GetTreeCtrl ().GetItemText (treeItem));
	Conversation::LibrarySet::iterator iterator = librarySet.find (libraryName);
	if (iterator != librarySet.end ())
		document->getLibrarySet ().erase (iterator);

	HTREEITEM siblingItem = GetTreeCtrl ().GetPrevSiblingItem (treeItem);
	if (!siblingItem)
		siblingItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);

	IGNORE_RETURN (GetTreeCtrl ().DeleteItem (treeItem));
	selectItem (siblingItem);

	GetDocument ()->SetModifiedFlag ();
	Invalidate ();
}

// ----------------------------------------------------------------------

void ScriptTreeView::OnBeginlabeledit(NMHDR * const pNMHDR, LRESULT * const pResult) 
{
	//-- get the tree view display info
	NMTVDISPINFO const * const pTVDispInfo = reinterpret_cast<NMTVDISPINFO const *> (pNMHDR);

	//-- disallow editing by default
	*pResult = 1;

	HTREEITEM const treeItem = pTVDispInfo->item.hItem;

	if ((isCondition (treeItem) || isAction (treeItem) || isLabel (treeItem) || isTokenTO (treeItem) || isTokenDI (treeItem) || isTokenDF (treeItem)) && GetTreeCtrl ().GetItemData (treeItem) != 0)
	{
		//-- limit to 100 characters
		GetTreeCtrl ().GetEditControl()->LimitText (100);

		//-- allow edit
		*pResult = 0;
		return;
	}
}

// ----------------------------------------------------------------------

bool isValidName (char const * name)
{
	while (*name != 0)
	{
		if (!(isalpha (*name) || isdigit (*name) || *name == '_'))
			return false;

		++name;
	}

	return true;
}

// ----------------------------------------------------------------------

void ScriptTreeView::OnEndlabeledit(NMHDR * const pNMHDR, LRESULT * const pResult) 
{
	//-- get the tree view display info
	NMTVDISPINFO * const pTVDispInfo = reinterpret_cast<NMTVDISPINFO *> (pNMHDR);

	HTREEITEM treeItem = pTVDispInfo->item.hItem;

	//-- get the new text string
	char const * const newName = pTVDispInfo->item.pszText;

	if (!newName || istrlen (newName) == 0)
	{
		*pResult = 0;
		return;
	}

	if (isdigit (*newName))
	{
		MessageBox ("Names cannot start with a number.  Please select another name.", "Bad Name");
		*pResult = 0;
		return;
	}

	if (*newName == '_')
	{
		MessageBox ("Names starting with underscores are reserved.  Please select another name.", "Bad Name");
		*pResult = 0;
		return;
	}

	if (!isValidName (newName))
	{
		MessageBox ("Names can only contain alpha-numberic characters with first character not a digit.  Please select another name.", "Bad Name");
		*pResult = 0;
		return;
	}

	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	ScriptGroup * scriptGroup = 0;
	if (isCondition (treeItem))
		scriptGroup = document->getConditionGroup ();
	else
		if (isAction (treeItem))
			scriptGroup = document->getActionGroup ();
		else
			if (isTokenTO (treeItem))
				scriptGroup = document->getTokenTOGroup ();
			else
				if (isTokenDI (treeItem))
					scriptGroup = document->getTokenDIGroup ();
				else
					if (isTokenDF (treeItem))
						scriptGroup = document->getTokenDFGroup ();
					else
						if (isLabel (treeItem))
							scriptGroup = document->getLabelGroup ();

	if (scriptGroup && !familyExists (scriptGroup, newName))
	{
		//-- change name
		int const familyId = GetTreeCtrl ().GetItemData (treeItem);
		scriptGroup->setFamilyName (familyId, newName);

		//-- tell tree about new name
		GetTreeCtrl ().SetItem (&pTVDispInfo->item);

		selectItem (treeItem);

		document->UpdateAllViews (this, SwgConversationEditorDoc::H_scriptChanged);

		*pResult = 1;
	}
	else 
	{
		MessageBox ("A script with this name already exists.  Please select another name.", "Bad Name");
		*pResult = 0;
	}
}

// ----------------------------------------------------------------------

void ScriptTreeView::OnScriptRename() 
{
	if (isActionSelected () || isConditionSelected () || isLabelSelected () || isTokenTOSelected () || isTokenDISelected () || isTokenDFSelected ())
		GetTreeCtrl ().EditLabel (GetTreeCtrl ().GetSelectedItem ());
}

*/

void SpaceZoneTreeView::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	UNREF(pTVDispInfo);
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

// ----------------------------------------------------------------------

void SpaceZoneTreeView::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	UNREF(pTVDispInfo);
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

// ----------------------------------------------------------------------

void SpaceZoneTreeView::OnKeydown(NMHDR * const pNMHDR, LRESULT * const pResult) 
{
	TV_KEYDOWN const * const pTVKeyDown = reinterpret_cast<TV_KEYDOWN const *>(pNMHDR);

	switch (pTVKeyDown->wVKey)
	{
	case VK_DELETE:
		OnDelete();
		break;
	}

	*pResult = 0;
}

// ----------------------------------------------------------------------

void SpaceZoneTreeView::OnSpawnerAddpatrolpoint() 
{
	HTREEITEM selection = GetTreeCtrl().GetSelectedItem();
	if (selection)
	{
		SwgSpaceZoneEditorDoc * const document = safe_cast<SwgSpaceZoneEditorDoc *>(GetDocument());
		SwgSpaceZoneEditorDoc::Object * const object = reinterpret_cast<SwgSpaceZoneEditorDoc::Object *>(GetTreeCtrl().GetItemData(selection));
		if (object)
			document->addPatrolPoint(object);
	}
}

// ----------------------------------------------------------------------

void SpaceZoneTreeView::OnSpawnerSelectallpatrolpoints() 
{
	HTREEITEM selection = GetTreeCtrl().GetSelectedItem();
	if (selection)
	{
		SwgSpaceZoneEditorDoc::Object * const object = reinterpret_cast<SwgSpaceZoneEditorDoc::Object *>(GetTreeCtrl().GetItemData(selection));
		if (object && (object->isPatrolPoint() || object->isPatrolPointSpawner()))
			GetDocument()->UpdateAllViews(this, SwgSpaceZoneEditorDoc::H_selectAllPatrolPoints, reinterpret_cast<CObject *>(object));
	}
}

// ======================================================================


void SpaceZoneTreeView::OnSpawnerDuplicate()
{
	// TODO: Add your command handler code here
	OnDuplicate();
}
