//===================================================================
//
// WorldSnapshotTreeView.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "FirstWorldSnapshotViewer.h"
#include "WorldSnapshotTreeView.h"

#include "ChildFrm.h"
#include "DialogFindObject.h"
#include "DialogProgress.h"
#include "sharedMath/Transform.h"
#include "sharedUtility/WorldSnapshotReaderWriter.h"
#include "WorldSnapshotDataView.h"
#include "WorldSnapshotViewerDoc.h"

#include <map>

//===================================================================

IMPLEMENT_DYNCREATE(WorldSnapshotTreeView, CTreeView)

BEGIN_MESSAGE_MAP(WorldSnapshotTreeView, CTreeView)
	//{{AFX_MSG_MAP(WorldSnapshotTreeView)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_COMMAND(ID_BUTTON_FIND, OnButtonFind)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CTreeView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CTreeView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CTreeView::OnFilePrintPreview)
END_MESSAGE_MAP()

//===================================================================

WorldSnapshotTreeView::WorldSnapshotTreeView() :
	m_nameRoot (0),
	m_nodeRoot (0)
{
}

WorldSnapshotTreeView::~WorldSnapshotTreeView()
{
}

//-------------------------------------------------------------------

BOOL WorldSnapshotTreeView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= (TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_DISABLEDRAGDROP);

	return CTreeView::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

void WorldSnapshotTreeView::OnDraw(CDC* pDC)
{
//	WorldSnapshotViewerDoc* pDoc = GetDocument();
//	ASSERT_VALID(pDoc);

	UNREF (pDC);
}

//-------------------------------------------------------------------

BOOL WorldSnapshotTreeView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void WorldSnapshotTreeView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void WorldSnapshotTreeView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

//-------------------------------------------------------------------

static void addNode (CTreeCtrl& treeCtrl, HTREEITEM parent, const WorldSnapshotReaderWriter* const reader, const WorldSnapshotReaderWriter::Node* const node)
{
	const int networkIdInt = static_cast< int >( node->getNetworkIdInt () );
	const char* const objectTemplateName = reader->getObjectTemplateName (node->getObjectTemplateNameIndex ());

	char name [256];
	sprintf (name, "%i - %s", networkIdInt, objectTemplateName);

	HTREEITEM item = treeCtrl.InsertItem (name, parent);
	treeCtrl.SetItemData (item, reinterpret_cast<DWORD> (&node->getTransform_p ()));

	int i;
	for (i = 0; i < node->getNumberOfNodes (); ++i)
		addNode (treeCtrl, item, reader, node->getNode (i));
}

//-------------------------------------------------------------------

void WorldSnapshotTreeView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

	WorldSnapshotViewerDoc* const document = safe_cast<WorldSnapshotViewerDoc*> (GetDocument ());
	const WorldSnapshotReaderWriter* const reader = document->getWorldSnapshotReaderWriter ();

	const int numberOfNodes = reader->getNumberOfNodes ();
	if (!numberOfNodes)
		return;

	//-- add the object template names
	{
		char name [256];
		sprintf (name, "ObjectTemplate Names (%i)", reader->getNumberOfObjectTemplateNames ());
		m_nameRoot = GetTreeCtrl ().InsertItem (name, TVI_ROOT);

		int i;
		for (i = 0; i < reader->getNumberOfObjectTemplateNames (); ++i)
			GetTreeCtrl ().InsertItem (reader->getObjectTemplateName (i), m_nameRoot, TVI_SORT);
	}

	//-- add the nodes
	{
		char name [256];
		sprintf (name, "Objects (%i) [%i]", reader->getNumberOfNodes (), reader->getTotalNumberOfNodes ());
		m_nodeRoot = GetTreeCtrl ().InsertItem (name, TVI_ROOT);

		const int multiple = 10;

		DialogProgress* dlg = new DialogProgress ();
		IGNORE_RETURN (dlg->Create ());
		dlg->SetRange (0, numberOfNodes < multiple ? 1 : numberOfNodes / multiple);

		sprintf (name, "Loading %i root objects (%i total objects)...", reader->getNumberOfNodes (), reader->getTotalNumberOfNodes ());
		dlg->SetStatus (name);

		IGNORE_RETURN (dlg->SetStep (1));
		IGNORE_RETURN (dlg->SetPos (0));

		int i;
		for (i = 0; i < numberOfNodes; ++i)
		{
			if (i % multiple == 0)
			{
				IGNORE_RETURN (dlg->StepIt ());
				if (dlg->CheckCancelButton ())
					break;
			}

			addNode (GetTreeCtrl (), m_nodeRoot, reader, reader->getNode (i));
		}

		delete dlg;
		dlg = 0;
	}

	document->setTotalNumberOfNodes (reader->getTotalNumberOfNodes ());
	document->setNumberOfNodes (reader->getNumberOfNodes ());
}

//-------------------------------------------------------------------

#ifdef _DEBUG

void WorldSnapshotTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void WorldSnapshotTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

#endif //_DEBUG

//-------------------------------------------------------------------

void WorldSnapshotTreeView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	UNREF (pSender);
	UNREF (lHint);
	UNREF (pHint);
}

//-------------------------------------------------------------------

static HTREEITEM getRoot (CTreeCtrl& m_treeCtrl, HTREEITEM hitem)
{
	HTREEITEM parent         = hitem;
	HTREEITEM possibleParent = hitem;

	do
	{
		possibleParent = m_treeCtrl.GetParentItem (possibleParent);

		if (possibleParent)
			parent = possibleParent;
	}
	while (possibleParent != 0);

	return parent;
}

//-------------------------------------------------------------------

const Vector WorldSnapshotTreeView::getPosition_p (HTREEITEM item) const
{
	if (item == m_nodeRoot || getRoot (GetTreeCtrl (), item) != m_nodeRoot)
		return Vector::zero;

	const Transform& transform = *reinterpret_cast<const Transform*> (GetTreeCtrl ().GetItemData (item));
	return transform.getPosition_p ();
}

//-------------------------------------------------------------------

const Vector WorldSnapshotTreeView::getPosition_w (HTREEITEM item) const
{
	if (item == m_nodeRoot || getRoot (GetTreeCtrl (), item) != m_nodeRoot)
		return Vector::zero;

	Vector result;
	do
	{
		const Transform& transform = *reinterpret_cast<const Transform*> (GetTreeCtrl ().GetItemData (item));
		result = transform.rotateTranslate_l2p (result);

		item = GetTreeCtrl ().GetParentItem (item);
	}
	while (item != m_nodeRoot);

	return result;
}

//-------------------------------------------------------------------

void WorldSnapshotTreeView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREF (pNMHDR);

	safe_cast<WorldSnapshotViewerDoc*> (GetDocument ())->setSelectedPosition_p (getPosition_p (GetTreeCtrl ().GetSelectedItem ()));
	safe_cast<WorldSnapshotViewerDoc*> (GetDocument ())->setSelectedPosition_w (getPosition_w (GetTreeCtrl ().GetSelectedItem ()));
	GetDocument ()->UpdateAllViews (this);

	*pResult = 0;
}

//-------------------------------------------------------------------

static HTREEITEM findItem(CTreeCtrl& tree, HTREEITEM root, const CString& objectId)
{
	HTREEITEM item = 0;

	while (root)
	{
		if (tree.ItemHasChildren(root))
			item = ::findItem(tree, tree.GetChildItem(root), objectId);

		if (item)
			return item;

		CString object = tree.GetItemText(root);
		object = object.Left(object.Find(' '));
		if (object == objectId)
			item = root;

		if (item)
			return item;

		root = tree.GetNextSiblingItem(root);
	}

	return item;
}

//-------------------------------------------------------------------

void WorldSnapshotTreeView::OnButtonFind() 
{
	DialogFindObject dlg;
	if (dlg.DoModal() == IDOK)
	{
		HTREEITEM item = findItem(GetTreeCtrl(), GetTreeCtrl().GetRootItem(), dlg.m_objectId);
		if (item)
			GetTreeCtrl().SelectItem(item);
		else
			MessageBox(dlg.m_objectId + " was not found");
	}
}

//===================================================================

