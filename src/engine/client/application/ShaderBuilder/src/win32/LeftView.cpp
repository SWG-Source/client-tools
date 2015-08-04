// ======================================================================
// LeftView.cpp : implementation file
// ======================================================================

#include "FirstShaderBuilder.h"
#include "ShaderBuilder.h"
#include "LeftView.h"
#include "MainFrm.h"
#include "sharedFile/Iff.h"
#include "RecentDirectory.h"
#include "ShaderBuilderDoc.h"
#include "Node.h"
#include "RenderView.h"
#include "sharedFile/TreeFile.h"

#include <afxdlgs.h>
#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ======================================================================

IMPLEMENT_DYNCREATE(CLeftView, CTreeView)

BEGIN_MESSAGE_MAP(CLeftView, CTreeView)
	//{{AFX_MSG_MAP(CLeftView)
	ON_WM_RBUTTONDOWN()
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_COMMAND(IDE_LINT, OnEffectLint)
	ON_COMMAND(IDE_ADDLOCALIMPLEMENTATION, OnEffectAddlocalimplementation)
	ON_COMMAND(IDI_ADDLOCALPASS, OnImplementationAddlocalpass)
	ON_COMMAND(IDI_DELETE, OnImplementationDelete)
	ON_COMMAND(IDP_ADDLOCALSTAGE, OnPassAddlocalstage)
	ON_COMMAND(IDP_DELETE, OnPassDelete)
	ON_COMMAND(IDS_DELETE, OnStageDelete)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_COMMAND(IDE_SAVEEFFECT, OnSaveeffect)
	ON_COMMAND(IDE_SAVEEFFECTAS, OnSaveeffectas)
	ON_COMMAND(IDI_SAVE, OnSaveImplementation)
	ON_COMMAND(IDE_ADDNAMEDIMPLEMENTATION, OnAddnamedimplementation)
	ON_COMMAND(IDE_LOCALIZEEFFECT, OnLocalizeeffect)
	ON_COMMAND(IDI_LOCALIZEIMPLEMENTATION, OnLocalizeimplementation)
	ON_COMMAND(IDE_LOADEFFECT, OnLoadeffect)
	ON_COMMAND(IDT_LOAD, OnTemplateLoad)
	ON_COMMAND(IDT_NEW, OnTemplateNew)
	ON_COMMAND(IDT_SAVE, OnTemplateSave)
	ON_COMMAND(IDT_SAVEAS, OnTemplateSaveas)
	ON_UPDATE_COMMAND_UI(IDP_ADDLOCALSTAGE, OnUpdateAddlocalstage)
	ON_COMMAND(IDX_DELETE, OnPixelShaderDelete)
	ON_COMMAND(IDP_ADDFIXEDFUNCTIONPIPELINE, OnAddfixedfunctionpipeline)
	ON_UPDATE_COMMAND_UI(IDP_ADDFIXEDFUNCTIONPIPELINE, OnUpdateAddfixedfunctionpipeline)
	ON_COMMAND(IDP_ADDVERTEXSHADER, OnAddvertexshader)
	ON_UPDATE_COMMAND_UI(IDP_ADDVERTEXSHADER, OnUpdateAddvertexshader)
	ON_COMMAND(IDF_DELETE, OnDeleteFixedfunctionpipeline)
	ON_COMMAND(IDV_DELETE, OnDeleteVertexShader)
	ON_COMMAND(IDVPT_COMPILEALL, OnCompileAllVertexShaderPrograms)
	ON_COMMAND(IDVPT_OPEN, OnOpenVertexShader)
	ON_COMMAND(IDPPT_COMPILEALL, OnCompileallPixelShaders)
	ON_COMMAND(IDPPT_OPEN, OnOpenPixelShader)
	ON_COMMAND(IDIT_OPEN, OnIncludeOpen)
	ON_COMMAND(IDIF_CLOSE, OnIncludeClose)
	ON_COMMAND(IDVF_CLOSE, OnVertexShaderProgramClose)
	ON_COMMAND(IDPF_CLOSE, OnPixelShaderProgramClose)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

CLeftView::CLeftView()
:
	templateTreeNode(NULL),
	renderNode(NULL),
	renderItem(NULL),
	dragging(false),
	dragImageList(NULL),
	timerId(0),
	renderView(NULL)
{
	for (int i = 0; i < CNode::Count; ++i)
		contextMenu[i] = -1;
	contextMenu[CNode::Template]                = 0;
	contextMenu[CNode::Effect]                  = 1;
	contextMenu[CNode::Implementation]          = 2;
	contextMenu[CNode::Pass]                    = 3;
	contextMenu[CNode::Stage]                   = 4;
	contextMenu[CNode::PixelShader]             = 6;
	contextMenu[CNode::FixedFunctionPipeline]   = 7;
	contextMenu[CNode::VertexShader]            = 8;
	contextMenu[CNode::VertexShaderProgramTree] = 9;
	contextMenu[CNode::PixelShaderProgramTree]  = 10;
	contextMenu[CNode::IncludeTree]             = 11;
	contextMenu[CNode::Include]                 = 12;
	contextMenu[CNode::VertexShaderProgram]     = 13;
	contextMenu[CNode::PixelShaderProgram]      = 14;

	CShaderBuilderDoc::leftView = this;
}

// ----------------------------------------------------------------------

CLeftView::~CLeftView()
{
	delete dragImageList;
	delete templateTreeNode;
	renderNode = NULL;
	renderItem = NULL;
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void CLeftView::AssertValid() const
{
	CTreeView::AssertValid();
}

// ----------------------------------------------------------------------

void CLeftView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

void CLeftView::ResetTree()
{
	CTreeCtrl &treeCtrl = GetTreeCtrl();
	HTREEITEM root = treeCtrl.GetRootItem();
	treeCtrl.Select(root, TVGN_CARET);

	delete templateTreeNode;

	templateTreeNode = new CTemplateTreeNode(*this, treeCtrl);
	renderNode = NULL;
	renderItem = NULL;
}

// ----------------------------------------------------------------------

void CLeftView::ExpandSelectedItem()
{
	CTreeCtrl &treeCtrl = GetTreeCtrl();
	HTREEITEM item = treeCtrl.GetSelectedItem();
	if (item)
		treeCtrl.Expand(item, TVE_EXPAND);
}

// ----------------------------------------------------------------------

void CLeftView::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();

	static bool once = false;
	if (!once)
	{
		cursorPointer      = LoadCursor (AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_POINTER));
		cursorPointerCopy  = LoadCursor (AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_POINTER_COPY));
		cursorNoDrag       = LoadCursor (AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_POINTER_NODRAG));
		cursorNotTarget    = LoadCursor (AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_POINTER_NOTTARGET));

		imageList.Create(IDB_TREEIMAGES, 16, 8, 0x00ff00ff);
		GetTreeCtrl().SetImageList(&imageList, TVSIL_NORMAL);
		rootMenu.LoadMenu(IDR_CONTEXT);
		HWND window = GetSafeHwnd();
		LONG style = GetWindowLong(window, GWL_STYLE );
		SetWindowLong(window, GWL_STYLE, style | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS);
		once = true;
	}
}

// ----------------------------------------------------------------------

void CLeftView::OnDraw(CDC* pDC)
{
	// TODO: add draw code here
	CDocument* pDoc = GetDocument();
	UNREF(pDoc);
	CTreeView::OnDraw(pDC);
}

// ----------------------------------------------------------------------

void CLeftView::OnRButtonDown(UINT nFlags, CPoint point)
{
	UNREF(nFlags);
	// see if we hit an item in the tree
	UINT flags = 0;
	HTREEITEM hit = GetTreeCtrl().HitTest(point, &flags);
	if (hit)
	{
		// select the item
		GetTreeCtrl().SelectItem(hit);

		// find the node for the tree item
		CNode *node = templateTreeNode->Find(hit);
		ASSERT(node);
		ASSERT(node->GetType() < CNode::Count);

		// see if this node has a context menu
		const int menuNumber = contextMenu[node->GetType()];
		if (menuNumber >= 0)
		{
			// get the appropriate context menu
			CMenu *subMenu = rootMenu.GetSubMenu(0);
			ASSERT(subMenu);
			CMenu *popUpMenu = subMenu->GetSubMenu(menuNumber);
			ASSERT(popUpMenu);

			switch (node->GetType())
			{
				case CNode::Effect:
					if (node->IsLocked(CNode::Self))
					{
						popUpMenu->EnableMenuItem(IDE_LOCALIZEEFFECT, MF_BYCOMMAND | MF_ENABLED);
						popUpMenu->EnableMenuItem(IDE_ADDLOCALIMPLEMENTATION, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
						popUpMenu->EnableMenuItem(IDE_ADDNAMEDIMPLEMENTATION, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
					}
					else
					{
						popUpMenu->EnableMenuItem(IDE_LOCALIZEEFFECT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
						popUpMenu->EnableMenuItem(IDE_ADDLOCALIMPLEMENTATION, MF_BYCOMMAND | MF_ENABLED);
						popUpMenu->EnableMenuItem(IDE_ADDNAMEDIMPLEMENTATION, MF_BYCOMMAND | MF_ENABLED);
					}
					if (static_cast<CEffectNode*>(node)->GetFileName().IsEmpty())
						popUpMenu->EnableMenuItem(IDE_SAVEEFFECT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
					else
						popUpMenu->EnableMenuItem(IDE_SAVEEFFECT, MF_BYCOMMAND | MF_ENABLED);
					break;

				case CNode::Implementation:
					if (node->IsLocked(CNode::Parent))
						popUpMenu->EnableMenuItem(IDI_DELETE,                 MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
					else
						popUpMenu->EnableMenuItem(IDI_DELETE,                 MF_BYCOMMAND | MF_ENABLED);

					if (node->IsLocked(CNode::Self) && !node->IsLocked(CNode::Parent))
						popUpMenu->EnableMenuItem(IDI_LOCALIZEIMPLEMENTATION, MF_BYCOMMAND | MF_ENABLED);
					else
						popUpMenu->EnableMenuItem(IDI_LOCALIZEIMPLEMENTATION, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

					if (node->IsLocked(CNode::Either))
						popUpMenu->EnableMenuItem(IDI_ADDLOCALPASS,           MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
					else
						popUpMenu->EnableMenuItem(IDI_ADDLOCALPASS,           MF_BYCOMMAND | MF_ENABLED);
					break;

				case CNode::Pass:
					if (node->IsLocked(CNode::Either))
						popUpMenu->EnableMenuItem(IDP_DELETE,   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
					else
						popUpMenu->EnableMenuItem(IDP_DELETE,   MF_BYCOMMAND | MF_ENABLED);

					if (node->IsLocked(CNode::Either) || static_cast<CPassNode*>(node)->GetFixedFunctionPipeline() || static_cast<CPassNode*>(node)->GetVertexShader() || static_cast<CPassNode*>(node)->GetPixelShader())
						popUpMenu->EnableMenuItem(IDP_ADDFIXEDFUNCTIONPIPELINE,   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
					else
						popUpMenu->EnableMenuItem(IDP_ADDFIXEDFUNCTIONPIPELINE,   MF_BYCOMMAND | MF_ENABLED);

					if (node->IsLocked(CNode::Either) || static_cast<CPassNode*>(node)->GetFixedFunctionPipeline() || static_cast<CPassNode*>(node)->GetVertexShader() || static_cast<CPassNode*>(node)->GetFirstStage())
						popUpMenu->EnableMenuItem(IDP_ADDVERTEXSHADER,            MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
					else
						popUpMenu->EnableMenuItem(IDP_ADDVERTEXSHADER,            MF_BYCOMMAND | MF_ENABLED);

					if (node->IsLocked(CNode::Either) || static_cast<CPassNode*>(node)->GetPixelShader() || static_cast<CPassNode*>(node)->GetVertexShader())
						popUpMenu->EnableMenuItem(IDP_ADDLOCALSTAGE,           MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
					else
						popUpMenu->EnableMenuItem(IDP_ADDLOCALSTAGE,           MF_BYCOMMAND | MF_ENABLED);
					break;

				case CNode::PixelShader:
					if (node->IsLocked(CNode::Either))
						popUpMenu->EnableMenuItem(IDX_DELETE,   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
					else
						popUpMenu->EnableMenuItem(IDX_DELETE,       MF_BYCOMMAND | MF_ENABLED);
					break;

				case CNode::FixedFunctionPipeline:
					if (node->IsLocked(CNode::Either))
						popUpMenu->EnableMenuItem(IDF_DELETE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
					else
						popUpMenu->EnableMenuItem(IDF_DELETE, MF_BYCOMMAND | MF_ENABLED);
					break;

				case CNode::VertexShader:
					if (node->IsLocked(CNode::Either))
						popUpMenu->EnableMenuItem(IDV_DELETE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
					else
						popUpMenu->EnableMenuItem(IDV_DELETE, MF_BYCOMMAND | MF_ENABLED);
					break;

				case CNode::VertexShaderProgram:
					if (static_cast<CVertexShaderProgramNode*>(node)->m_manuallyLoaded)
						popUpMenu->EnableMenuItem(IDVF_CLOSE, MF_BYCOMMAND | MF_ENABLED);
					else
						popUpMenu->EnableMenuItem(IDVF_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
					break;

				case CNode::PixelShaderProgram:
					if (static_cast<CPixelShaderProgramNode*>(node)->m_manuallyLoaded)
						popUpMenu->EnableMenuItem(IDVF_CLOSE, MF_BYCOMMAND | MF_ENABLED);
					else
						popUpMenu->EnableMenuItem(IDVF_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
					break;

			};

			// display it where the mouse was clicked
			ClientToScreen(&point);
			popUpMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, this);
		}
	}
}

// ----------------------------------------------------------------------

void CLeftView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	UNREF(pNMTreeView);
	*pResult = 0;

	HTREEITEM selectedItem = GetTreeCtrl().GetSelectedItem();
	if (selectedItem)
	{
		CNode *node = templateTreeNode->Find(selectedItem);
		ASSERT(node);
		mainFrame->ShowNode(*node);
	}
}

// ----------------------------------------------------------------------

CNode *CLeftView::GetSelectedNode()
{
	CTreeCtrl &treeCtrl = GetTreeCtrl();
	HTREEITEM selectedItem = treeCtrl.GetSelectedItem();
	if (!selectedItem)
		return NULL;

	CNode *node = templateTreeNode->Find(selectedItem);
	ASSERT(node);
	return node;
}

// ----------------------------------------------------------------------

const CNode *CLeftView::GetSelectedNode() const
{
	CTreeCtrl &treeCtrl = GetTreeCtrl();
	HTREEITEM selectedItem = treeCtrl.GetSelectedItem();
	if (!selectedItem)
		return NULL;

	CNode *node = templateTreeNode->Find(selectedItem);
	ASSERT(node);
	return node;
}

// ----------------------------------------------------------------------

const CNode *CLeftView::GetRenderNode() const
{
	CTreeCtrl &treeCtrl = GetTreeCtrl();
	HTREEITEM selectedItem = treeCtrl.GetSelectedItem();
	if (!selectedItem)
		return NULL;

	CNode *selectedNode = templateTreeNode->Find(selectedItem);
	if (!selectedNode)
		return NULL;

	CNode::Type type = selectedNode->GetType();
	if (type == CNode::Texture)
	{
		// unmark the previous rendering node
		if (renderNode)
			renderNode->SetRendering(false);

		// remember the new rendering node and item
		renderNode = selectedNode;
		renderItem = selectedItem;

		// mark the new rendering node
		renderNode->SetRendering(true);
	}

	return renderNode;
}

// ----------------------------------------------------------------------

void CLeftView::DestroyNode(const CNode *node)
{
	if (node == renderNode)
	{
		renderNode = NULL;
		renderItem = NULL;
	}
}

// ----------------------------------------------------------------------

void CLeftView::ShowNode(CNode* node)
{
	ASSERT(node);
	mainFrame->ShowNode(*node);
}

// ----------------------------------------------------------------------

CNode &CLeftView::GetSelectedNode(CNode::Type type)
{
	UNREF(type);
	CTreeCtrl &treeCtrl = GetTreeCtrl();
	HTREEITEM selectedItem = treeCtrl.GetSelectedItem();
	ASSERT(selectedItem);
	CNode *node = templateTreeNode->Find(selectedItem);
	ASSERT(node);
	ASSERT(node->GetType() == type);
	return *node;
}

// ----------------------------------------------------------------------

void CLeftView::UpdateAllItemImages(HTREEITEM item)
{
	for ( ; item; item = GetTreeCtrl().GetNextSiblingItem(item))
	{
		UpdateAllItemImages(GetTreeCtrl().GetChildItem(item));
		CNode *node = templateTreeNode->Find(item);
		if (node)
			node->SetItemImages();
	}
}


// ----------------------------------------------------------------------

void CLeftView::UpdateAllItemImages()
{
	UpdateAllItemImages(GetTreeCtrl().GetRootItem());
}

// ----------------------------------------------------------------------

void CLeftView::OnTemplateLoad() 
{
	CFileDialog fileDialog(TRUE, "*.sht", 0, OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR, "Shader Templates *.sht|*.sht||");
	fileDialog.m_ofn.lpstrInitialDir = RecentDirectory::find("Template");
	fileDialog.m_ofn.lpstrDefExt     = "sht";
	fileDialog.m_ofn.lpstrTitle      = "Load Shader Template";
	if (fileDialog.DoModal() == IDOK)
		if (LoadTemplate(fileDialog.GetPathName()))
			GetDocument()->SetPathName(fileDialog.GetPathName());
}

// ----------------------------------------------------------------------

BOOL CLeftView::LoadTemplate(const char *fileName)
{
	ResetTree();
	Iff iff;
	if (!iff.open(fileName, true))
		return FALSE;

	if (!templateTreeNode->Load(iff))
	{
		ResetTree();
		return FALSE;
	}

	UpdateAllItemImages();
	RecentDirectory::update("Template", fileName);

	CEffectNode &effect = templateTreeNode->GetEffect();
	const CImplementationNode *imp = effect.GetFirstImplementation();
	if (imp)
		GetTreeCtrl().Select(imp->GetTreeItem(), TVGN_CARET);

	return TRUE;
}

// ----------------------------------------------------------------------

void CLeftView::OnTemplateNew() 
{
	ResetTree();
	GetDocument()->GetDocTemplate()->SetDefaultTitle(GetDocument());
	GetDocument()->OnNewDocument();
}

// ----------------------------------------------------------------------

void CLeftView::OnTemplateSave() 
{
	if (!templateTreeNode->Validate())
		return;

	if (GetDocument()->GetPathName()  == "")
	{
		OnTemplateSaveas();
		return;
	}

	SaveTemplate(GetDocument()->GetPathName());
}

// ----------------------------------------------------------------------

void CLeftView::OnTemplateSaveas() 
{
	if (!templateTreeNode->Validate())
		return;

	CFileDialog fileDialog(FALSE, "*.sht", 0, OFN_NOCHANGEDIR, "Shader Templates *.sht|*.sht||");
	fileDialog.m_ofn.lpstrInitialDir = RecentDirectory::find("Template");
	fileDialog.m_ofn.lpstrTitle      = "Save Shader Template As";
	if (fileDialog.DoModal() == IDOK)
	{
		CString fileName = fileDialog.GetPathName();
		if (SaveTemplate(fileName))
			GetDocument()->SetPathName(fileDialog.GetPathName());
	}
}

// ----------------------------------------------------------------------

BOOL CLeftView::SaveTemplate(const char *fileName)
{
	Iff iff(32 * 1024);

	if (!templateTreeNode->Save(iff))
		return FALSE;

	if (!iff.write(fileName, true))
	{
		MessageBox("Could not save shader template [is the file read only?]", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	RecentDirectory::update("Template", fileName);
	return TRUE;
}

// ----------------------------------------------------------------------

void CLeftView::OnEffectLint() 
{
	CEffectNode &effectNode = static_cast<CEffectNode &>(GetSelectedNode(CNode::Effect));
	effectNode.Lint();
	ExpandSelectedItem();
}

// ----------------------------------------------------------------------

void CLeftView::OnEffectAddlocalimplementation() 
{
	CEffectNode &effectNode = static_cast<CEffectNode &>(GetSelectedNode(CNode::Effect));
	effectNode.AddLocalImplementation();
	ExpandSelectedItem();
}

// ----------------------------------------------------------------------

void CLeftView::OnImplementationAddlocalpass() 
{
	CImplementationNode &implementationNode = static_cast<CImplementationNode &>(GetSelectedNode(CNode::Implementation));
	implementationNode .AddLocalPass();
	ExpandSelectedItem();
}

// ----------------------------------------------------------------------

void CLeftView::OnImplementationDelete() 
{
	CImplementationNode &implementationNode = static_cast<CImplementationNode &>(GetSelectedNode(CNode::Implementation));
	implementationNode.Delete();
	Invalidate();
}

// ----------------------------------------------------------------------

void CLeftView::OnPassAddlocalstage() 
{
	CPassNode &passNode = static_cast<CPassNode &>(GetSelectedNode(CNode::Pass));
	passNode.AddLocalStage();
	ExpandSelectedItem();
}


// ----------------------------------------------------------------------

void CLeftView::OnUpdateAddlocalstage(CCmdUI* pCmdUI) 
{
	UNREF(pCmdUI);
}

// ----------------------------------------------------------------------

void CLeftView::OnPassDelete() 
{
	CPassNode &passNode = static_cast<CPassNode &>(GetSelectedNode(CNode::Pass));
	passNode.Delete();
	Invalidate();
}

// ----------------------------------------------------------------------

void CLeftView::OnStageDelete() 
{
	CStageNode &stageNode = static_cast<CStageNode &>(GetSelectedNode(CNode::Stage));
	stageNode.Delete();
	Invalidate();
}

// ----------------------------------------------------------------------

void CLeftView::OnPixelShaderDelete() 
{
	CPixelShaderNode &node = static_cast<CPixelShaderNode &>(GetSelectedNode(CNode::PixelShader));
	node.Delete();
	Invalidate();
}

// ----------------------------------------------------------------------

void CLeftView::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	dragItem = pNMTreeView->itemNew.hItem;

	GetTreeCtrl().Select(dragItem, TVGN_CARET);

	// have to be able to find the node to drag it
	dragNode = templateTreeNode->Find(dragItem);
	if (!dragNode)
		return;

	// can only drag implementations, passes, stages, pixelShaders.  Can't drag when the parent is locked either.
	CNode::Type type = dragNode->GetType();
	if ((type != CNode::Implementation && type != CNode::Pass && type != CNode::Stage && type != CNode::PixelShader) || dragNode->IsLocked(CNode::Parent))
	{
		SetCapture();
		SetCursor(cursorNoDrag);
		return;
	}

	// create the drag image
	dragImageList = GetTreeCtrl().CreateDragImage(dragItem);
	if (!dragImageList)
		return;

	// figure out if this is a copy or move
	const bool copy = (GetKeyState (VK_CONTROL) & 0x8000) != 0;
	if (copy)
		SetCursor(cursorPointerCopy);
	else
		SetCursor(cursorPointer);
	
	// drag from the bottom left of the image
	IMAGEINFO ii;
	dragImageList->GetImageInfo(0, &ii);
	CPoint pt(0, ii.rcImage.bottom);
	dragImageList->BeginDrag(0, pt);
	pt = pNMTreeView->ptDrag;
	ClientToScreen( &pt );
	dragImageList->DragEnter(NULL, pt);

	// all input events should come here	
	SetCapture();

	dragging = true;
	*pResult = 0;
}

// ----------------------------------------------------------------------

void CLeftView::CheckTarget()
{
	// look up the target node
	targetValid = true;

	HTREEITEM targetItem = GetTreeCtrl().GetDropHilightItem();
	const CNode *targetNode = templateTreeNode->Find(targetItem);
	if (!targetNode)
		targetValid = false;
	else
	{
		// can't drag to a locked node
		if (targetNode->IsLocked(CNode::Either))
			targetValid = false;
		else
		{
			// get the types of the drag and target nodes
			CNode::Type dropType   = dragNode->GetType();
			CNode::Type targetType = targetNode->GetType();

			// see if the types make sense for a drag
			if (dropType == CNode::Implementation)
			{
				if (targetType != CNode::Effect &&targetType != CNode::Implementation)
					targetValid = false;
			}
			else
				if (dropType == CNode::Pass)
				{
					if (targetType != CNode::Implementation && targetType != CNode::Pass)
						targetValid = false;
				}
				else
					if (dropType == CNode::Stage)
					{
						if (targetType != CNode::Pass && targetType != CNode::Stage)
							targetValid = false;
					}
					else
						if (dropType == CNode::PixelShader)
						{
							if (targetType != CNode::Pass)
								targetValid = false;
							else
							{
								// can't drop a pixel shader onto a pass with a pixel shader or a texture stage cascade
								const CPassNode *passNode = static_cast<const CPassNode *>(targetNode);
								if (passNode->GetPixelShader() || passNode->GetFirstStage())
									targetValid = false;
							}
						}
		}
	}
}

// ----------------------------------------------------------------------

void CLeftView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CTreeView::OnMouseMove(nFlags, point);

	if (dragging)
	{
		CTreeCtrl &treeCtrl = GetTreeCtrl();
		POINT pt = point;
		ClientToScreen(&pt);
		CImageList::DragMove(pt);
		
		UINT       flags;
		HTREEITEM  targetItem = treeCtrl.HitTest(point, &flags);
		if (targetItem != NULL)
		{
			CImageList::DragShowNolock(FALSE);

			HTREEITEM oldTargetItem = treeCtrl.GetDropHilightItem();
			treeCtrl.SelectDropTarget(targetItem);
		
			// set a timer for auto-open on items
			if (timerId && targetItem == oldTargetItem)
			{
				KillTimer(timerId);
				timerId = 0;
			}
			if (!timerId)
				timerId = SetTimer(1000, 2000, NULL);

			CImageList::DragShowNolock(TRUE);
		}

		// set the cursor to indicate to the user what the drag state is
		CheckTarget();
		if (!targetValid)
			SetCursor(cursorNotTarget);
		else
		{
			const bool copy = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
			if (copy)
				SetCursor(cursorPointerCopy);
			else
				SetCursor(cursorPointer);
		}
	}
}

// ----------------------------------------------------------------------

void CLeftView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (dragging)
	{
		const bool copy = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

		CTreeCtrl &treeCtrl = GetTreeCtrl();
		CImageList::DragLeave(this);
		CImageList::EndDrag();

		UINT      flags      = 0;
		HTREEITEM targetItem = treeCtrl.HitTest(point, &flags);
		if (targetItem)
		{
			// set the current target
			treeCtrl.SelectDropTarget(targetItem);

			// make sure this is a valid target for the source
			CheckTarget();

			if (targetValid)
			{
				// figure out the new parent item, and location to be inserted after
				CNode *targetNode = templateTreeNode->Find(targetItem);
				HTREEITEM parentItem;
				HTREEITEM afterItem;
				if (targetNode->GetType() == dragNode->GetType())
				{
					parentItem = GetTreeCtrl().GetParentItem(targetItem);
					afterItem  = targetItem;
				}
				else
				{
					parentItem = targetItem;
					afterItem  = TVI_FIRST;
				}

				// get access to the parent item
				CNode *parentNode = templateTreeNode->Find(parentItem);
				switch (dragNode->GetType())
				{
					case CNode::Implementation:
						{
							CEffectNode         *effectNode         = static_cast<CEffectNode *>(parentNode);
							CImplementationNode *implementationNode = static_cast<CImplementationNode *>(dragNode);
							effectNode->AddLocalImplementation(*implementationNode, afterItem);
							if (!copy)
								implementationNode->Delete();
						}
						break;

					case CNode::Pass:
						{
							CImplementationNode *implementationNode  = static_cast<CImplementationNode *>(parentNode);
							CPassNode *passNode = static_cast<CPassNode *>(dragNode);
							implementationNode->AddLocalPass(*passNode, afterItem);
							if (!copy)
								passNode->Delete();
						}
						break;

					case CNode::Stage:
						{
							CPassNode  *passNode  = static_cast<CPassNode *>(parentNode);
							CStageNode *stageNode = static_cast<CStageNode *>(dragNode);
							passNode->AddLocalStage(*stageNode, afterItem);
							if (!copy)
								stageNode->Delete();
						}
						break;

					case CNode::PixelShader:
						{
							CPassNode        *passNode        = static_cast<CPassNode *>(parentNode);
							CPixelShaderNode *pixelShaderNode = static_cast<CPixelShaderNode *>(dragNode);
							passNode->AddLocalPixelShader(*pixelShaderNode);
							if (!copy)
								pixelShaderNode->Delete();
						}
						break;

					default:
						assert(false);
						break;
				}

				GetTreeCtrl().Expand(parentItem, TVE_EXPAND);
				UpdateAllItemImages();
				Invalidate();
			}
		}

		// clean up after our attempted drag operation
		treeCtrl.SelectDropTarget(NULL);
		treeCtrl.Select(targetItem, TVGN_CARET);
		if (timerId)
		{
			KillTimer(timerId);
			timerId;
		}
		delete dragImageList;
		dragImageList = NULL;
		dragging = false;
	}	

	ReleaseCapture();
	SetCursor(cursorPointer);	
	CTreeView::OnLButtonUp(nFlags, point);
}

// ----------------------------------------------------------------------

void CLeftView::OnTimer(UINT nIDEvent) 
{
	if (dragging && nIDEvent == timerId)
	{
		// handle auto-expand of closed tree
		CTreeCtrl &treeCtrl = GetTreeCtrl();
		HTREEITEM item = treeCtrl.GetDropHilightItem();
		if (item && treeCtrl.ItemHasChildren(item))
			treeCtrl.Expand(item, TVE_EXPAND);
	}

	CTreeView::OnTimer(nIDEvent);
}

// ----------------------------------------------------------------------

void CLeftView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (dragging)
	{
		if (nChar == VK_ESCAPE)
		{
			// abort tthe drag
			CImageList::DragLeave(this);
			CImageList::EndDrag();

			CTreeCtrl &treeCtrl = GetTreeCtrl();
			treeCtrl.SelectDropTarget(NULL);
			treeCtrl.Select(dragItem, TVGN_CARET);
			if (timerId)
			{
				KillTimer(timerId);
				timerId;
			}
			delete dragImageList;
			dragImageList = NULL;

			ReleaseCapture();
			SetCursor(cursorPointer);	

			dragging = false;
		}
		else
			if (!targetValid)
				SetCursor(cursorNotTarget);
			else
			{
				const bool copy = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
				if (copy)
					SetCursor(cursorPointerCopy);
				else
					SetCursor(cursorPointer);
			}
	}
	else
	{
		if (nChar == VK_DELETE && nRepCnt == 1)
		{
			CNode *node = GetSelectedNode();
			if (node && !node->IsLocked(CNode::Either))
			{
				switch (node->GetType())
				{
					case CNode::Implementation:
						OnImplementationDelete();
						break;

					case CNode::Pass:
						OnPassDelete();
						break;

					case CNode::Stage:
						OnStageDelete();
						break;

					case CNode::PixelShader:
						OnPixelShaderDelete();
						break;

					default:
						break;
				}
			}	
		}
	}
			
	CTreeView::OnKeyDown(nChar, nRepCnt, nFlags);
}

// ----------------------------------------------------------------------

void CLeftView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (dragging)
	{
		if (!targetValid)
			SetCursor(cursorNotTarget);
		else
		{
			const bool copy = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
			if (copy)
				SetCursor(cursorPointerCopy);
			else
				SetCursor(cursorPointer);
		}
	}
	else
	{
		if (nChar == VK_INSERT && nRepCnt == 1)
		{
			CNode *node = GetSelectedNode();
			if (node && !node->IsLocked(CNode::Either))
			{
				switch (node->GetType())
				{
					case CNode::Effect:
						OnEffectAddlocalimplementation();
						break;

					case CNode::Implementation:
						OnImplementationAddlocalpass();
						break;

					case CNode::Pass:
						if (static_cast<const CPassNode *>(node)->GetFirstStage())
							OnPassAddlocalstage();
						break;

					default:
						break;
				}
			}	
		}
	}
	
	CTreeView::OnKeyUp(nChar, nRepCnt, nFlags);
}

// ----------------------------------------------------------------------

void CLeftView::OnSaveeffect() 
{
	CEffectNode &effect = templateTreeNode->GetEffect();
	
	if (!effect.Validate())
		return;

	char pathName[512];
	if (!TreeFile::getPathName(effect.GetFileName(), pathName, sizeof(pathName)))
	{
		MessageBox("Could not find old effect to save over", "Error", MB_ICONERROR | MB_OK);
		return;
	}

	Iff iff(32 * 1024);
	CString effectFileName = effect.GetFileName();
	if (!effect.Save(iff, true))
		return;

	if (!iff.write(pathName, true))
		MessageBox("Could not save shader effect", "Error", MB_ICONERROR | MB_OK);
}

// ----------------------------------------------------------------------

void CLeftView::OnSaveeffectas() 
{
	CEffectNode &effect = templateTreeNode->GetEffect();

	if(!effect.Validate())
		return;

	CFileDialog fileDialog(FALSE, "*.eft", 0, OFN_NOCHANGEDIR, "Shader Effects *.eft|*.eft||");
	fileDialog.m_ofn.lpstrInitialDir = RecentDirectory::find("Effect");
	fileDialog.m_ofn.lpstrTitle      = "Save Shader Effect As";
	char buffer[MAX_PATH];
	if (!effect.GetFileName().IsEmpty())
	{
		const char *fileName = effect.GetFileName();
		if (strrchr(fileName, '\\'))
			fileName = strrchr(fileName, '\\') + 1;
		if (strrchr(fileName, '/'))
			fileName = strrchr(fileName, '/') + 1;
		strcpy(buffer, fileName);
		fileDialog.m_ofn.lpstrFile = buffer;
		fileDialog.m_ofn.nMaxFile = sizeof(buffer);
	}

	// Make sure the user selected OK
	if (fileDialog.DoModal() == IDOK)
	{
		Iff iff(32 * 1024);
		CString effectFileName = fileDialog.GetPathName();
		if (!effect.Save(iff, true))
			return;

		if (!iff.write(effectFileName, true))
		{
			MessageBox("Could not save shader effect", "Error", MB_ICONERROR | MB_OK);
			return;
		}

		RecentDirectory::update("Effect", effectFileName);

		if (MessageBox("Would you like to use the effect by name in the template?", "Use named effect", MB_YESNO) == IDYES)
		{
			std::string localEffectFileName = static_cast<CShaderBuilderApp*>(AfxGetApp())->stripTreefileSearchPathFromFile(static_cast<LPCSTR>(effectFileName));
			effect.SetFileName(localEffectFileName.c_str());
			mainFrame->ShowNode(effect);
			UpdateAllItemImages();
		}
	}
}

// ----------------------------------------------------------------------

void CLeftView::OnSaveImplementation() 
{
	CImplementationNode &implementation = static_cast<CImplementationNode&>(GetSelectedNode(CNode::Implementation));

	if(!implementation.Validate())
		return;

	CFileDialog fileDialog(FALSE, "*.imp", 0, OFN_NOCHANGEDIR, "Shader Implementations *.imp|*.imp||");
	fileDialog.m_ofn.lpstrInitialDir = RecentDirectory::find("Implementation");
	fileDialog.m_ofn.lpstrTitle      = "Save Implementation As";
	if (!implementation.GetFileName().IsEmpty())
	{
		char buffer[MAX_PATH];
		strcpy(buffer, implementation.GetFileName());
		fileDialog.m_ofn.lpstrFile = buffer;
	}

	// Make sure the user selected OK
	if (fileDialog.DoModal() == IDOK)
	{
		Iff iff(32 * 1024);
		CString implementationFileName = fileDialog.GetPathName();
		if (!implementation.Save(iff, true))
			return;

		if (!iff.write(implementationFileName, true))
		{
			MessageBox("Could not save shader implementation", "Error", MB_ICONERROR | MB_OK);
			return;
		}

		RecentDirectory::update("Implementation", implementationFileName);

		if (!implementation.IsLocked(CNode::Parent) && MessageBox("Would you like to use the implementation by name in the effect?", "Use named implementation", MB_YESNO) == IDYES)
		{
			std::string localImplementationFileName = static_cast<CShaderBuilderApp*>(AfxGetApp())->stripTreefileSearchPathFromFile(static_cast<LPCSTR>(implementationFileName));
			implementation.SetFileName(localImplementationFileName.c_str());
			mainFrame->ShowNode(implementation);
			UpdateAllItemImages();
		}
	}
}

// ----------------------------------------------------------------------

void CLeftView::OnAddnamedimplementation() 
{
	CFileDialog fileDialog(TRUE, "*.imp", 0, OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR, "Shader Implementations *.imp|*.imp||");
	fileDialog.m_ofn.lpstrInitialDir = RecentDirectory::find("Implementation");
	fileDialog.m_ofn.lpstrTitle      = "Add Named Implementation";
	if (fileDialog.DoModal() == IDOK)
	{
		CString implementationFileName = fileDialog.GetPathName();
		Iff iff;
		if (!iff.open(implementationFileName, true))
			return;

		RecentDirectory::update("Implementation", implementationFileName);

		CEffectNode &effectNode = static_cast<CEffectNode &>(GetSelectedNode(CNode::Effect));
		CImplementationNode &implementationNode = effectNode.AddLocalImplementation();

		implementationNode.Load(iff);

		std::string localImplementationFileName = static_cast<CShaderBuilderApp*>(AfxGetApp())->stripTreefileSearchPathFromFile(static_cast<LPCSTR>(implementationFileName));
		implementationNode.SetFileName(localImplementationFileName.c_str());

		//if the final filename has a : in it, warn about possible absolute path
		if(localImplementationFileName.find(":", 0) != std::string::npos)
		{
			MessageBox("Absolute pathed implementation detected.  File might not be in TreeFile search path.", "Warning", MB_OK);
		}

		UpdateAllItemImages();
		ExpandSelectedItem();
	}
}

// ----------------------------------------------------------------------

void CLeftView::OnLocalizeeffect() 
{
	CEffectNode &effect = templateTreeNode->GetEffect();
	effect.Localize();
	mainFrame->ShowNode(effect);
	UpdateAllItemImages();
}

// ----------------------------------------------------------------------

void CLeftView::OnLocalizeimplementation() 
{
	CImplementationNode &implementation = static_cast<CImplementationNode &>(GetSelectedNode(CNode::Implementation));
	implementation.SetFileName("");
	mainFrame->ShowNode(implementation);
	UpdateAllItemImages();
}

// ----------------------------------------------------------------------

template <class T>
void AddReferenceCount(CTaggedTreeNode<T> &node, std::set<CString> &tags)
{
	node.GetChildTags(tags);
	for (std::set<CString>::iterator i = tags.begin(); i != tags.end(); ++i)
		node.Add(*i);
}

// ----------------------------------------------------------------------

template <class T>
void RemoveReferenceCount(CTaggedTreeNode<T> &node, std::set<CString> &tags)
{
	for (std::set<CString>::iterator i = tags.begin(); i != tags.end(); ++i)
		node.Remove(*i);
}

// ----------------------------------------------------------------------

void CLeftView::OnLoadeffect() 
{
	CFileDialog fileDialog(TRUE, "*.eft", 0, OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR, "Shader Effects *.eft|*.eft||");
	fileDialog.m_ofn.lpstrInitialDir = RecentDirectory::find("Effect");
	fileDialog.m_ofn.lpstrTitle      = "Load Shader Effect";
	if (fileDialog.DoModal() == IDOK)
	{
		CString effectFileName = fileDialog.GetPathName();
		Iff iff;
		if (!iff.open(effectFileName, true))
			return;

		RecentDirectory::update("Effect", effectFileName);

		// add a reference to all the current tagged elements in use (so they'll be preserved across the effect reset and load)
		std::set<CString> materialTags;
		std::set<CString> textureTags;
		std::set<CString> textureCoordinateSetTags;
		std::set<CString> textureFactorTags;
		std::set<CString> alphaReferenceValueTags;
		std::set<CString> stencilReferenceValueTags;
		AddReferenceCount(templateTreeNode->GetMaterialTree(),              materialTags);
		AddReferenceCount(templateTreeNode->GetTextureTree(),               textureTags);
		AddReferenceCount(templateTreeNode->GetTextureCoordinateSetTree(),  textureCoordinateSetTags);
		AddReferenceCount(templateTreeNode->GetTextureFactorTree(),         textureFactorTags);
		AddReferenceCount(templateTreeNode->GetAlphaReferenceValueTree(),   alphaReferenceValueTags);
		AddReferenceCount(templateTreeNode->GetStencilReferenceValueTree(), stencilReferenceValueTags);

		// load the new effect
		CEffectNode &effectNode = templateTreeNode->GetEffect();
		effectNode.Reset();
		effectNode.Load(iff);

		// remove a reference from all the previous tagged elements
		RemoveReferenceCount(templateTreeNode->GetMaterialTree(),              materialTags);
		RemoveReferenceCount(templateTreeNode->GetTextureTree(),               textureTags);
		RemoveReferenceCount(templateTreeNode->GetTextureCoordinateSetTree(),  textureCoordinateSetTags);
		RemoveReferenceCount(templateTreeNode->GetTextureFactorTree(),         textureFactorTags);
		RemoveReferenceCount(templateTreeNode->GetAlphaReferenceValueTree(),   alphaReferenceValueTags);
		RemoveReferenceCount(templateTreeNode->GetStencilReferenceValueTree(), stencilReferenceValueTags);


		std::string localEffectFileName = static_cast<CShaderBuilderApp*>(AfxGetApp())->stripTreefileSearchPathFromFile(static_cast<LPCSTR>(effectFileName));
		effectNode.SetFileName(localEffectFileName.c_str());
		//if the final filename has a : in it, warn about possible absolute path
		if(localEffectFileName.find(":", 0) != std::string::npos)
		{
			MessageBox("Absolute pathed effect detected.  File might not be in TreeFile search path.", "Warning", MB_OK);
		}

		UpdateAllItemImages();
		ExpandSelectedItem();
		mainFrame->ShowNode(effectNode);
	}
}

// ======================================================================

void CLeftView::OnAddfixedfunctionpipeline() 
{
	CPassNode &passNode = static_cast<CPassNode &>(GetSelectedNode(CNode::Pass));
	passNode.AddFixedFunctionPipeline();
	ExpandSelectedItem();
}

void CLeftView::OnUpdateAddfixedfunctionpipeline(CCmdUI* pCmdUI) 
{
	UNREF(pCmdUI);
}

void CLeftView::OnAddvertexshader() 
{
	CPassNode &passNode = static_cast<CPassNode &>(GetSelectedNode(CNode::Pass));
	passNode.AddVertexShader();
	passNode.AddLocalPixelShader();
	ExpandSelectedItem();
}

void CLeftView::OnUpdateAddvertexshader(CCmdUI* pCmdUI) 
{
	UNREF(pCmdUI);
}

// ======================================================================

void CLeftView::OnDeleteFixedfunctionpipeline() 
{
	CFixedFunctionPipelineNode &fixedFunctionPipelineNode = static_cast<CFixedFunctionPipelineNode &>(GetSelectedNode(CNode::FixedFunctionPipeline));
	fixedFunctionPipelineNode.Delete();
	Invalidate();
}

// ======================================================================

void CLeftView::OnDeleteVertexShader() 
{
	CVertexShaderNode &vertexShaderNode = static_cast<CVertexShaderNode &>(GetSelectedNode(CNode::VertexShader));
	vertexShaderNode.Delete();
	Invalidate();
}

// ======================================================================

void CLeftView::OnCompileAllVertexShaderPrograms() 
{
	CTemplateTreeNode::GetInstance()->GetVertexShaderProgramTree().CompileAll();
}

// ----------------------------------------------------------------------

void CLeftView::OnOpenVertexShader() 
{
	char temp[1024 * 32] = {0};
	CFileDialog fileDialog(TRUE, "*.vsh", 0, OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_NOCHANGEDIR, "Vertex Shaders *.vsh|*.vsh||");
	fileDialog.m_ofn.lpstrInitialDir = RecentDirectory::find("VertexShaderProgram");
	fileDialog.m_ofn.lpstrFile = temp;
	fileDialog.m_ofn.nMaxFile = sizeof(temp);
	fileDialog.m_ofn.lpstrTitle      = "Open Vertex Shader";
	DWORD result = fileDialog.DoModal();
	if (result == IDOK)
	{
		POSITION p = fileDialog.GetStartPosition();
		while (p)
		{
			CString name = fileDialog.GetNextPathName(p);
			RecentDirectory::update("VertexShaderProgram", name);
			std::string fileName = static_cast<CShaderBuilderApp*>(AfxGetApp())->stripTreefileSearchPathFromFile(static_cast<LPCSTR>(name));
			CTemplateTreeNode::GetInstance()->GetVertexShaderProgramTree().Add(fileName.c_str(), true);
		}
	}
}

// ======================================================================

void CLeftView::OnCompileallPixelShaders() 
{
	CTemplateTreeNode::GetInstance()->GetPixelShaderProgramTree().CompileAll();
}

void CLeftView::OnOpenPixelShader() 
{
	char temp[1024 * 32] = {0};
	CFileDialog fileDialog(TRUE, "*.psh", 0, OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_NOCHANGEDIR, "Pixel Shaders *.psh|*.psh||");
	fileDialog.m_ofn.lpstrInitialDir = RecentDirectory::find("PixelShaderProgram");
	fileDialog.m_ofn.lpstrFile = temp;
	fileDialog.m_ofn.nMaxFile = sizeof(temp);
	fileDialog.m_ofn.lpstrTitle      = "Open Pixel Shader";
	if (fileDialog.DoModal() == IDOK)
	{
		POSITION p = fileDialog.GetStartPosition();
		while (p)
		{
			CString name = fileDialog.GetNextPathName(p);
			RecentDirectory::update("PixelShaderProgram", name);
			std::string fileName = static_cast<CShaderBuilderApp*>(AfxGetApp())->stripTreefileSearchPathFromFile(static_cast<LPCSTR>(name));
			CTemplateTreeNode::GetInstance()->GetPixelShaderProgramTree().Add(fileName.c_str(), true);
		}
	}
}

// ======================================================================

void CLeftView::OnIncludeOpen() 
{
	CFileDialog fileDialog(TRUE, "*.inc", 0, OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_NOCHANGEDIR, "Include files *.inc|*.inc|All Files *.*|*.*||");
	fileDialog.m_ofn.lpstrInitialDir = RecentDirectory::find("Include");
	char temp[1024 * 32] = {0};
	fileDialog.m_ofn.lpstrFile = temp;
	fileDialog.m_ofn.nMaxFile = sizeof(temp);
	fileDialog.m_ofn.lpstrTitle      = "Open Include File";

	DWORD result = fileDialog.DoModal();
	if (result == IDOK)
	{
		POSITION p = fileDialog.GetStartPosition();
		while (p)
		{
			CString name = fileDialog.GetNextPathName(p);
			RecentDirectory::update("Include", name);
			std::string fileName = static_cast<CShaderBuilderApp*>(AfxGetApp())->stripTreefileSearchPathFromFile(static_cast<LPCSTR>(name));
			CTemplateTreeNode::GetInstance()->GetIncludeTree().Add(fileName.c_str());
		}
	}
}

void CLeftView::OnIncludeClose() 
{
	CTreeCtrl &treeCtrl = GetTreeCtrl();
	HTREEITEM selectedItem = treeCtrl.GetSelectedItem();
	if (!selectedItem)
		return;
	CTemplateTreeNode::GetInstance()->GetIncludeTree().Remove(treeCtrl.GetItemText(selectedItem));
}

void CLeftView::OnVertexShaderProgramClose() 
{
	CTreeCtrl &treeCtrl = GetTreeCtrl();
	HTREEITEM selectedItem = treeCtrl.GetSelectedItem();
	if (!selectedItem)
		return;
	CTemplateTreeNode::GetInstance()->GetVertexShaderProgramTree().Remove(treeCtrl.GetItemText(selectedItem), true);
}

void CLeftView::OnPixelShaderProgramClose() 
{
	CTreeCtrl &treeCtrl = GetTreeCtrl();
	HTREEITEM selectedItem = treeCtrl.GetSelectedItem();
	if (!selectedItem)
		return;
	CTemplateTreeNode::GetInstance()->GetPixelShaderProgramTree().Remove(treeCtrl.GetItemText(selectedItem), true);
}
