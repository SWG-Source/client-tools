// ======================================================================
//
// SkeletalAppearanceTemplateView.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstViewer.h"
#include "SkeletalAppearanceTemplateView.h"

#include "clientSkeletalAnimation/SkeletalAppearanceTemplate.h"
#include "sharedFoundation/CrcLowerString.h"
#include "viewer.h"
#include "viewerDoc.h"

#include <string>

// ======================================================================

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ======================================================================
// constants
// ======================================================================

const std::string SkeletalAppearanceTemplateView::mcs_animationStateGraphDir("appearance/animationgraph/");

const char *const ms_appearanceTemplateItemText                      = "Skeleton Appearance Template";
const char *const ms_appearanceTemplateMeshGeneratorGroupItemText    = "Mesh Generators";
const char *const ms_appearanceTemplateSkeletonTemplateGroupItemText = "Skeleton Templates";

const int         ms_meshGeneratorImageIndex         = 0;
const int         ms_meshGeneratorGroupImageIndex    = 1;
const int         ms_animationStateGraphImageIndex   = 2;
const int         ms_appearanceTemplateImageIndex    = 4;
const int         ms_workspaceImageIndex             = 5;
const int         ms_skeletonTemplateImageIndex      = 6;
const int         ms_skeletonTemplateGroupImageIndex = 7;

// ======================================================================

namespace
{
	std::string FormatAvailableSkeletonTemplateText(const char *skeletonTemplateName, const char *attachmentName)
	{
		std::string  result(skeletonTemplateName);
		result += " (attached to ";

		if (attachmentName && *attachmentName)
		{
			result += "joint \"";
			result += attachmentName;
			result += "\")";
		}
		else
			result += "<nothing>)";

		return result;
	}
}

// ----------------------------------------------------------------------

namespace
{
	inline std::string FormatSkeletonTemplateText(const char *skeletonTemplateName, const char *attachmentName)
	{
		// these are identical for now
		return FormatAvailableSkeletonTemplateText(skeletonTemplateName, attachmentName);
	}
}

// ======================================================================
// class SkeletalAppearanceTemplateView
// ======================================================================

IMPLEMENT_DYNCREATE(SkeletalAppearanceTemplateView, CTreeView)

// ----------------------------------------------------------------------

SkeletalAppearanceTemplateView::SkeletalAppearanceTemplateView()
:	CTreeView(),
	m_workspaceTreeItem(0),
	m_appearanceTemplateTreeItem(0),
	m_meshGeneratorGroupTreeItem(0),
	m_skeletonTemplateGroupTreeItem(0),
	m_animationStateGraphTreeItem(0),
	m_selectedItemType(IT_none),
	m_selectedIndex(-1),
	m_treeImageList(NON_NULL(new CImageList()))
{
}

// ----------------------------------------------------------------------

SkeletalAppearanceTemplateView::~SkeletalAppearanceTemplateView()
{
	delete m_treeImageList;

	m_workspaceTreeItem             = 0;
	m_appearanceTemplateTreeItem    = 0;
	m_meshGeneratorGroupTreeItem    = 0;
	m_skeletonTemplateGroupTreeItem = 0;
	m_animationStateGraphTreeItem   = 0;
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(SkeletalAppearanceTemplateView, CTreeView)
	//{{AFX_MSG_MAP(SkeletalAppearanceTemplateView)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_SKELAPPEARWS_SAVE, OnSkeletalAppearanceWorkspaceSave)
	ON_COMMAND(ID_SKELAPPEARWS_SAVE_AS, OnSkeletalAppearanceWorkspaceSaveAs)
	ON_COMMAND(ID_SKELAPPEARWS_OPEN, OnSkeletalAppearanceWorkspaceOpen)
	ON_COMMAND(ID_SKELAPPEARTEMPLATE_SAVE, OnSkeletalAppearanceTemplateSave)
	ON_COMMAND(ID_SKELAPPEARTEMPLATE_SAVE_AS, OnSkeletonAppearanceTemplateSaveAs)
	ON_COMMAND(ID_SKELTEMP_DELETE, OnDeleteSkeletonTemplate)
	ON_COMMAND(ID_MESHGEN_DELETE, OnDeleteMeshGenerator)
	ON_COMMAND(ID_SKELAPPEARTEMPLATE_RENDER, OnRenderSkeletalAppearanceTemplate)
	ON_COMMAND(ID_ANIMSTATEGRAPH_BROWSE, OnAnimStateGraphBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplateView::OnDraw(CDC* pDC)
{
	//CDocument* pDoc = GetDocument();
	// TODO: add draw code here
	UNREF(pDC);
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void SkeletalAppearanceTemplateView::AssertValid() const
{
	CTreeView::AssertValid();
}
#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG
void SkeletalAppearanceTemplateView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplateView::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();

	CTreeCtrl &treeControl = GetTreeCtrl();

	//-- setup tree images

	// load image list
	NOT_NULL(m_treeImageList);
	m_treeImageList->Create(IDB_BITMAP_SKELAPPEARTEMPLATE, 13, 1, RGB(255,255,255));

	// assign to tree control
	treeControl.SetImageList(m_treeImageList, TVSIL_NORMAL);

	//-- setup tree control style
	LONG treeControlStyle = GetWindowLong(treeControl.m_hWnd, GWL_STYLE);

	// add lines
	treeControlStyle |= TVS_HASLINES;

	// add buttons
	treeControlStyle |= TVS_HASBUTTONS;

	// enable lines at root
	treeControlStyle |= TVS_LINESATROOT;

	// set the control
	IGNORE_RETURN(SetWindowLong(treeControl.m_hWnd, GWL_STYLE, treeControlStyle));

	//-- build the tree view control from existing data
	rebuildTreeControl();
}

// ----------------------------------------------------------------------

CViewerApp *SkeletalAppearanceTemplateView::getOurApp() const
{
	CViewerApp *app = dynamic_cast<CViewerApp*>(AfxGetApp());
	NOT_NULL(app);

	return app;
}

// ----------------------------------------------------------------------

CViewerDoc *SkeletalAppearanceTemplateView::getOurDocument() const
{
	CViewerDoc *doc = dynamic_cast<CViewerDoc*>(GetDocument());
	NOT_NULL(doc);

	return doc;
}

// ----------------------------------------------------------------------

SkeletalAppearanceTemplate *SkeletalAppearanceTemplateView::getSkeletalAppearanceTemplate() const
{
	CViewerDoc *viewerDoc = getOurDocument();
	return viewerDoc->getSkeletalAppearanceTemplate();
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplateView::rebuildTreeControl()
{
	CTreeCtrl &treeControl = GetTreeCtrl();

	//-- remove all elements
	IGNORE_RETURN(treeControl.DeleteAllItems());

	//-- get the skeletal appearance template
	SkeletalAppearanceTemplate *const appearanceTemplate = getSkeletalAppearanceTemplate();
	if (!appearanceTemplate)
	{
		IGNORE_RETURN(treeControl.InsertItem("Error: no skeletal appearance template available"));
		return;
	}

	//-- get our document
	CViewerDoc *viewerDoc = getOurDocument();
	if (!viewerDoc)
		return;

	//-- add workspace grouping
	const char *const workspaceFilename = viewerDoc->getSkeletalAppearanceWorkspaceFilename();
	const CString workspaceText         = formatSkeletalAppearanceWorkspaceText(workspaceFilename);
	m_workspaceTreeItem = treeControl.InsertItem(workspaceText, ms_workspaceImageIndex, ms_workspaceImageIndex);
	NOT_NULL(m_workspaceTreeItem);

	//-- add SkeletalAppearanceTemplate grouping
	const char *const skeletalAppearanceTemplateFilename = viewerDoc->getSkeletalAppearanceTemplateFilename();
	const CString     skeletalAppearanceTemplateText     = formatSkeletalAppearanceTemplateText(skeletalAppearanceTemplateFilename);
	m_appearanceTemplateTreeItem = treeControl.InsertItem(skeletalAppearanceTemplateText, ms_appearanceTemplateImageIndex, ms_appearanceTemplateImageIndex, m_workspaceTreeItem);
	NOT_NULL(m_appearanceTemplateTreeItem);

	{
		//-- add mesh generators grouping
		m_meshGeneratorGroupTreeItem = treeControl.InsertItem(ms_appearanceTemplateMeshGeneratorGroupItemText, ms_meshGeneratorGroupImageIndex, ms_meshGeneratorGroupImageIndex, m_appearanceTemplateTreeItem);
		NOT_NULL(m_meshGeneratorGroupTreeItem);

		{
			//-- add mesh generator names
			const int meshGeneratorCount = appearanceTemplate->getMeshGeneratorCount();
			for (int i = 0; i < meshGeneratorCount; ++i)
			{
				const CrcLowerString &crcName = appearanceTemplate->getMeshGeneratorName(i);
				HTREEITEM treeItem = treeControl.InsertItem(crcName.getString(), ms_meshGeneratorImageIndex, ms_meshGeneratorImageIndex, m_meshGeneratorGroupTreeItem);
				NOT_NULL(treeItem);

				IGNORE_RETURN(treeControl.SetItemData(treeItem, static_cast<DWORD>(i)));
			}
		}

		//-- add skeletal template grouping
		m_skeletonTemplateGroupTreeItem = treeControl.InsertItem(ms_appearanceTemplateSkeletonTemplateGroupItemText, ms_skeletonTemplateGroupImageIndex, ms_skeletonTemplateGroupImageIndex, m_appearanceTemplateTreeItem);
		NOT_NULL(m_skeletonTemplateGroupTreeItem);

		{
			//-- add skeleton template names
			const int skeletonTemplateCount = appearanceTemplate->getSkeletonTemplateCount();
			for (int i = 0; i < skeletonTemplateCount; ++i)
			{
				const SkeletalAppearanceTemplate::SkeletonTemplateInfo &skeletonTemplateInfo = appearanceTemplate->getSkeletonTemplateInfo(i);

				const CrcLowerString &crcSkeletonTemplateName = appearanceTemplate->getSkeletonTemplateName(skeletonTemplateInfo);
				const CrcLowerString *crcAttachmentName       = 0;
				if (appearanceTemplate->skeletonTemplateHasAttachmentTransform(skeletonTemplateInfo))
					crcAttachmentName = &(appearanceTemplate->getSkeletonTemplateAttachmentTransformName(skeletonTemplateInfo));

				const std::string itemText = FormatAvailableSkeletonTemplateText(crcSkeletonTemplateName.getString(), crcAttachmentName ? crcAttachmentName->getString() : "");

				HTREEITEM treeItem = treeControl.InsertItem(itemText.c_str(), ms_skeletonTemplateImageIndex, ms_skeletonTemplateImageIndex, m_skeletonTemplateGroupTreeItem);
				NOT_NULL(treeItem);

				IGNORE_RETURN(treeControl.SetItemData(treeItem, static_cast<DWORD>(i)));
			}
		}

		//-- add AnimationStateGraph item
		const CString asgText = formatAnimationStateGraphText(appearanceTemplate->getAnimationStateGraphTemplateName().getString());

		m_animationStateGraphTreeItem = treeControl.InsertItem(asgText, ms_animationStateGraphImageIndex, ms_animationStateGraphImageIndex, m_appearanceTemplateTreeItem);
		NOT_NULL(m_animationStateGraphTreeItem);
	}

	//-- open up everything we care to see
	IGNORE_RETURN(treeControl.Expand(m_workspaceTreeItem, TVE_EXPAND));
	IGNORE_RETURN(treeControl.Expand(m_appearanceTemplateTreeItem, TVE_EXPAND));
	IGNORE_RETURN(treeControl.Expand(m_meshGeneratorGroupTreeItem, TVE_EXPAND));
	IGNORE_RETURN(treeControl.Expand(m_skeletonTemplateGroupTreeItem, TVE_EXPAND));
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplateView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	UNREF(nFlags);

	//-- open up a popup menu for the selected tree item

	// get selected item
	CTreeCtrl &treeControl = GetTreeCtrl();
	HTREEITEM selectedItem = treeControl.GetSelectedItem();
	if (!selectedItem)
	{
		// no selected item, ignore it
		return;
	}

	// determine submenu
	int submenuItem = 0;

	const ItemType itemType = getItemType(selectedItem);
	switch (itemType)
	{
		case IT_appearanceTemplate:
			submenuItem = 0;
			break;

		case IT_availableMeshGeneratorGroup:
			submenuItem = 1;
			break;

		case IT_availableMeshGenerator:
			submenuItem = 2;
			break;

		case IT_availableSkeletonTemplateGroup:
			submenuItem = 3;
			break;

		case IT_availableSkeletonTemplate:
			submenuItem = 4;
			break;

		case IT_workspace:
			submenuItem = 5;
			break;

		case IT_animationStateGraph:
			submenuItem = 6;
			break;

		default:
			DEBUG_REPORT_LOG(true, ("unknown tree view item selected, ignoring right click\n"));
			return;
	}

	// load the menu
	CMenu  menus;
	const BOOL loadMenuSuccess = menus.LoadMenu(IDR_SKELETAL_APPEARANCE_TEMPLATE_TYPE);
	FATAL(!loadMenuSuccess, ("failed to load SkeletalAppearanceTemplate View's menu"));

	// get the popup submenu
	CMenu *const popupMenu = menus.GetSubMenu(submenuItem);
	NOT_NULL(popupMenu);

	// display it
	ClientToScreen(&point);
	IGNORE_RETURN(popupMenu->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this));
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplateView::addAvailableMeshGenerator(const char *meshGeneratorName, int index)
{
	UNREF(index);
	NOT_NULL(m_meshGeneratorGroupTreeItem);

	CTreeCtrl &treeControl = GetTreeCtrl();
	HTREEITEM treeItem = treeControl.InsertItem(meshGeneratorName, ms_meshGeneratorImageIndex, ms_meshGeneratorImageIndex, m_meshGeneratorGroupTreeItem);
	NOT_NULL(treeItem);

	IGNORE_RETURN(treeControl.SetItemData(treeItem, static_cast<DWORD>(index)));

	//-- redraw
	// getOurDocument()->rebuildAppearance();
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplateView::addAvailableSkeletonTemplate(const char *skeletonTemplateName, const char *attachmentTransformName, int index)
{
	UNREF(attachmentTransformName);
	UNREF(index);
	NOT_NULL(m_skeletonTemplateGroupTreeItem);

	CTreeCtrl         &treeControl = GetTreeCtrl();
	const std::string  itemText    = FormatAvailableSkeletonTemplateText(skeletonTemplateName, attachmentTransformName);

	HTREEITEM treeItem = treeControl.InsertItem(itemText.c_str(), ms_skeletonTemplateImageIndex, ms_skeletonTemplateImageIndex, m_skeletonTemplateGroupTreeItem);
	NOT_NULL(treeItem);

	IGNORE_RETURN(treeControl.SetItemData(treeItem, static_cast<DWORD>(index)));

	//-- redraw
	// getOurDocument()->rebuildAppearance();
}

// ----------------------------------------------------------------------

CString SkeletalAppearanceTemplateView::formatFilenameText(const char *pathName)
{
	const char *text;

	// lop of directory
	const char *const lastDirChar = strrchr(pathName, '\\');
	if (lastDirChar)
		text = lastDirChar + 1;
	else
		text = pathName;

	return CString(text);
}

// ----------------------------------------------------------------------

CString SkeletalAppearanceTemplateView::formatSkeletalAppearanceTemplateText(const char *pathName)
{
	return "Skeletal Appearance Template (" + formatFilenameText(pathName) + ")";
}

// ----------------------------------------------------------------------

CString SkeletalAppearanceTemplateView::formatSkeletalAppearanceWorkspaceText(const char *pathName)
{
	return "Workspace (" + formatFilenameText(pathName) + ")";
}

// ----------------------------------------------------------------------

CString SkeletalAppearanceTemplateView::formatAnimationStateGraphText(const char *pathName)
{
	CString  text("Animation State Graph File: ");

	if (pathName && *pathName)
		text += pathName;
	else
		text += "<none specified>";

	return text;
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplateView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// rebuild tree view
	rebuildTreeControl();

	CTreeView::OnUpdate(pSender, lHint, pHint);
}

// ----------------------------------------------------------------------

SkeletalAppearanceTemplateView::ItemType SkeletalAppearanceTemplateView::getItemType(HTREEITEM hItem) const
{
	if (!hItem)
		return IT_none;

	//-- check for items when we don't care about parent
	if (hItem == m_workspaceTreeItem)
		return IT_workspace;
	else if (hItem == m_appearanceTemplateTreeItem)
		return IT_appearanceTemplate;
	else if (hItem == m_meshGeneratorGroupTreeItem)
		return IT_availableMeshGeneratorGroup;
	else if (hItem == m_skeletonTemplateGroupTreeItem)
		return IT_availableSkeletonTemplateGroup;
	else if (hItem == m_animationStateGraphTreeItem)
		return IT_animationStateGraph;

	CTreeCtrl &treeControl = GetTreeCtrl();

	// figure out type of selected item
	HTREEITEM itemParent = treeControl.GetParentItem(hItem);
	if (!itemParent)	
	{
		// not an item we care about
		return IT_none;
	}

	if (itemParent == m_meshGeneratorGroupTreeItem)
	{
		// we've got an available mesh generator
		return IT_availableMeshGenerator;
	}
	else if (itemParent == m_skeletonTemplateGroupTreeItem)
	{
		// we've got a skeleton template
		return IT_availableSkeletonTemplate;
	}
	else
	{
		// not an item we care about
		return IT_none;
	}
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplateView::OnSkeletalAppearanceWorkspaceSave() 
{
	CTreeCtrl &treeControl = GetTreeCtrl();

	//-- make sure we've got an workspace selected
	HTREEITEM      selectedItem = treeControl.GetSelectedItem();
	const ItemType itemType     = getItemType(selectedItem);
	if (itemType != IT_workspace)
		return;

	//-- see if we have a filename for this
	CViewerDoc *doc = getOurDocument();
	const CString &workspaceName    = doc->getSkeletalAppearanceWorkspaceFilename();
	const bool     mustProvideName  = !workspaceName.GetLength();

	if (mustProvideName)
	{
		OnSkeletalAppearanceWorkspaceSaveAs();
		return;
	}

	//-- do the save
	const bool saveResult = doc->saveSkeletalAppearanceWorkspace(workspaceName);
	if (!saveResult)
	{
		char buffer[2*MAX_PATH];

		sprintf(buffer, "Failed to save skeletal appearance workspace [%s]", static_cast<const char*>(workspaceName));
		MessageBox(buffer, "Save Failure", MB_OK | MB_ICONSTOP);
		return;
	}
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplateView::OnSkeletalAppearanceWorkspaceSaveAs() 
{
	CTreeCtrl &treeControl = GetTreeCtrl();

	//-- make sure we've got the instance workspace selected
	HTREEITEM      selectedItem = treeControl.GetSelectedItem();
	const ItemType itemType     = getItemType(selectedItem);
	if (itemType != IT_workspace)
		return;

	CViewerApp *viewerApp = dynamic_cast<CViewerApp*>(AfxGetApp());
	if (!viewerApp)
		return;

	const char *const lastWorkspaceFilename = viewerApp->getLastSkeletalAppearanceWorkspaceFilename();

	CFileDialog dlg(FALSE, "saw", lastWorkspaceFilename, OFN_OVERWRITEPROMPT, "Skeletal Appearance Workspace files (*.saw)|*.saw||", this);
	if (dlg.DoModal() == IDOK)
	{
		const CString pathName = dlg.GetPathName();

		// set the instance filename
		viewerApp->setLastSkeletalAppearanceWorkspaceFilename(pathName);

		CViewerDoc *doc = getOurDocument();
		if (!doc)
			return;

		// save it
		const bool saveResult = doc->saveSkeletalAppearanceWorkspace(pathName);
		if (!saveResult)
		{
			char buffer[2*MAX_PATH];

			sprintf(buffer, "Failed to save skeletal appearance workspace [%s]", static_cast<const char*>(pathName));
			MessageBox(buffer, "Save Failure", MB_OK | MB_ICONSTOP);
			return;
		}

		// set workspace name from filename
		const CString text = formatSkeletalAppearanceWorkspaceText(pathName);
		IGNORE_RETURN(treeControl.SetItemText(selectedItem, text));
	}
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplateView::OnSkeletalAppearanceWorkspaceOpen() 
{
	CViewerApp *viewerApp = dynamic_cast<CViewerApp*>(AfxGetApp());
	if (!viewerApp)
		return;

	const char *const lastWorkspaceFilename = viewerApp->getLastSkeletalAppearanceWorkspaceFilename();

	CFileDialog dlg(TRUE, "saw", lastWorkspaceFilename, 0, "Skeletal Appearance Workspace files (*.saw)|*.saw||", this);
	if (dlg.DoModal() == IDOK)
	{
		const CString pathName = dlg.GetPathName();

		// set the workspace filename
		viewerApp->setLastSkeletalAppearanceWorkspaceFilename(pathName);

		CViewerDoc *doc = getOurDocument();
		if (!doc)
			return;

		// open it
		IGNORE_RETURN(doc->loadSkeletalAppearanceWorkspace(pathName));
	}
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplateView::OnSkeletalAppearanceTemplateSave() 
{
	CViewerDoc    *doc      = getOurDocument();
	const CString &filename = doc->getSkeletalAppearanceTemplateFilename();

	if (!filename.IsEmpty())
	{
		doc->saveSkeletalAppearanceTemplate(filename);
	}
	else
		OnSkeletonAppearanceTemplateSaveAs();
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplateView::OnSkeletonAppearanceTemplateSaveAs() 
{
	// get the app
	CViewerApp *const app = static_cast<CViewerApp*>(AfxGetApp());
	if (!app)
		return;

	CViewerDoc *doc = getOurDocument();

	// get the last skeleton appearance template file dealt with
	const char *const lastFilename = app->getLastSkeletalAppearanceTemplateFilename();
	CFileDialog dlg(FALSE, ".sat", lastFilename, 0, "Skeletal Appearance Templates (*.sat)|*.sat||");

	// open the dialog
	if (dlg.DoModal() == IDOK)
	{
		const CString pathName = dlg.GetPathName();

		// save it
		doc->saveSkeletalAppearanceTemplate(pathName);

		// remember the pathname
		app->setLastSkeletalAppearanceTemplateFilename(pathName);

		// fixup text for tree view
		CTreeCtrl &treeControl = GetTreeCtrl();

		const CString itemText = formatSkeletalAppearanceTemplateText(pathName);
		treeControl.SetItemText(m_appearanceTemplateTreeItem, itemText);
	}
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplateView::OnDeleteSkeletonTemplate() 
{
	//-- ensure we've got a skeleton template selected
	CTreeCtrl       &treeControl  = GetTreeCtrl();
	const HTREEITEM  selectedItem = treeControl.GetSelectedItem();
	
	if (!selectedItem)
	{
		WARNING(true, ("OnDeleteSkeletonTemplate called but nothing selected\n"));
		return;
	}

	const ItemType itemType = getItemType(selectedItem);
	if (itemType != IT_availableSkeletonTemplate)
	{
		WARNING(true, ("OnDeleteSkeletonTemplate called but selected item is not a skeleton template (%u)\n", static_cast<unsigned int>(itemType)));
		return;
	}

	//-- get the skeletal appearance template
	SkeletalAppearanceTemplate *const appearanceTemplate = getSkeletalAppearanceTemplate();
	NOT_NULL(appearanceTemplate);

	//-- get skeleton template index
	const int skeletonTemplateIndex = static_cast<int>(treeControl.GetItemData(selectedItem));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, skeletonTemplateIndex, appearanceTemplate->getSkeletonTemplateCount());

	//-- delete skeleton template
	appearanceTemplate->removeSkeletonTemplate(skeletonTemplateIndex);

	//-- rebuild tree since indices are now hosed
	rebuildTreeControl();

	//-- redraw
	getOurDocument()->rebuildAppearance();
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplateView::OnDeleteMeshGenerator() 
{
	//-- ensure we've got a mesh generator selected
	CTreeCtrl       &treeControl  = GetTreeCtrl();
	const HTREEITEM  selectedItem = treeControl.GetSelectedItem();
	
	if (!selectedItem)
	{
		WARNING(true, ("OnDeleteMeshGenerator called but nothing selected\n"));
		return;
	}

	const ItemType itemType = getItemType(selectedItem);
	if (itemType != IT_availableMeshGenerator)
	{
		WARNING(true, ("OnDeleteMeshGenerator called but selected item is not a skeleton template (%u)\n", static_cast<unsigned int>(itemType)));
		return;
	}

	//-- get the skeletal appearance template
	SkeletalAppearanceTemplate *const appearanceTemplate = getSkeletalAppearanceTemplate();
	NOT_NULL(appearanceTemplate);

	//-- get mesh generator index
	const int meshGeneratorIndex = static_cast<int>(treeControl.GetItemData(selectedItem));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, meshGeneratorIndex, appearanceTemplate->getMeshGeneratorCount());

	//-- delete skeleton template
	appearanceTemplate->removeMeshGenerator(meshGeneratorIndex);

	//-- rebuild tree since indices are now hosed
	rebuildTreeControl();

	//-- redraw
	getOurDocument()->rebuildAppearance();
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplateView::OnRenderSkeletalAppearanceTemplate() 
{
	getOurDocument()->rebuildAppearance();
}

// ----------------------------------------------------------------------

void SkeletalAppearanceTemplateView::OnAnimStateGraphBrowse() 
{
	//-- get the appearance template
	SkeletalAppearanceTemplate *const appearanceTemplate = getSkeletalAppearanceTemplate();
	NOT_NULL(appearanceTemplate);

	//-- setup an initial pathname for the dialog to use
	// @todo  add workspace registry save setting for this
	const char *const initialPathname = NULL;

	//-- get file from user
	CFileDialog dlg(TRUE, "asg", initialPathname, 0, "Animation State Graph files (*.asg)|*.asg||", this);

	if (dlg.DoModal() != IDOK)
	{
		// user canceled, abort
		return;
	}

	//-- convert full pathname to Treefile-relative pathname.
	//   NOTE: this code assumes these files live in
	//         appearance/animationgraph/*.asg.
	const std::string asgPathname = mcs_animationStateGraphDir + static_cast<const char*>(dlg.GetFileName());

	//-- set the appearance template's animation state graph
	appearanceTemplate->setAnimationStateGraphTemplateName(CrcLowerString(asgPathname.c_str()));

	//-- rebuild the tree view
	rebuildTreeControl();

	//-- reload the appearance so the animation system can make use of the new animation list
	// -TRF- @todo
}

// ======================================================================
