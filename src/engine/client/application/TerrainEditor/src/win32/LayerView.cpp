//
// LayerView.cpp
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "LayerView.h"

#include "FormLayerItem.h"
#include "MainFrame.h"
#include "MapFrame.h"
#include "ProfileFrame.h"
#include "PropertyFrame.h"
#include "RecentDirectory.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"
#include "TerrainGeneratorHelper.h"
#include "WarningFrame.h"
#include "sharedFile/Iff.h"
#include "sharedMath/Vector2d.h"
#include "sharedTerrain/Affector.h"
#include "sharedTerrain/Boundary.h"
#include "sharedTerrain/Filter.h"
#include "sharedTerrain/TerrainGeneratorLoader.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(LayerView, CTreeView)

//-------------------------------------------------------------------

LayerView::LayerView (void) :
	CTreeView (),
	imageListSet (false),
	imageList (),
	moveCursor (0),
	copyCursor (0),
	m_dragMode (DM_nothing),
	m_pDragImage (0),
	m_htiDrag (0),
	m_htiDrop (0),
	m_htiOldDrop (0),
	m_bLDragging (false),
	m_idTimer (0)
{
	moveCursor = LoadCursor (AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_CURSOR_MOVE));  //lint !1924  //-- c-style cast
	copyCursor = LoadCursor (AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_CURSOR_COPY));  //lint !1924  //-- c-style cast
}

//-------------------------------------------------------------------

LayerView::~LayerView (void)
{
	moveCursor   = 0;
	copyCursor   = 0;

	m_pDragImage = 0;
	m_htiDrag    = 0;
	m_htiDrop    = 0;
	m_htiOldDrop = 0;
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(LayerView, CTreeView)
	//{{AFX_MSG_MAP(LayerView)
	ON_COMMAND(ID_BUTTON_DELETE_LAYER, OnButtonDeleteLayer)
	ON_COMMAND(ID_BUTTON_DEMOTE_LAYER, OnButtonDemoteLayer)
	ON_COMMAND(ID_BUTTON_PROMOTE_LAYER, OnButtonPromoteLayer)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_DELETE_LAYER, OnUpdateButtonDeleteLayer)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_DEMOTE_LAYER, OnUpdateButtonDemoteLayer)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_PROMOTE_LAYER, OnUpdateButtonPromoteLayer)
	ON_NOTIFY_REFLECT(TVN_DELETEITEM, OnDeleteitem)
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeydown)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_COMMAND(ID_BUTTON_APPLY, OnButtonApply)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginlabeledit)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndlabeledit)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_EXPORT_LAYERITEM, OnExportLayeritem)
	ON_UPDATE_COMMAND_UI(ID_EXPORT_LAYERITEM, OnUpdateExportLayeritem)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
	ON_COMMAND(ID_INSERT_DELETE, OnInsertDelete)
	ON_COMMAND(ID_INSERT_RENAME, OnInsertRename)
	ON_UPDATE_COMMAND_UI(ID_INSERT_DELETE, OnUpdateInsertDelete)
	ON_UPDATE_COMMAND_UI(ID_INSERT_RENAME, OnUpdateInsertRename)
	ON_COMMAND(ID_BUTTON_NEWROOTLAYER, OnButtonNewrootlayer)
	ON_COMMAND(ID_BUTTON_NEWSUBLAYER, OnButtonNewsublayer)
	ON_COMMAND(ID_BUTTON_PUSHROOT, OnButtonPushroot)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_PUSHROOT, OnUpdateButtonPushroot)
	ON_COMMAND(ID_BUTTON_PUSHUP, OnButtonPushup)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_PUSHUP, OnUpdateButtonPushup)
	ON_COMMAND(ID_INSERT_IMPORT_AFFECTOR, OnInsertImportAffector)
	ON_COMMAND(ID_INSERT_IMPORT_BOUNDARY, OnInsertImportBoundary)
	ON_COMMAND(ID_INSERT_IMPORT_FILTER, OnInsertImportFilter)
	ON_COMMAND(ID_INSERT_IMPORT_LAYER, OnInsertImportLayer)
	ON_COMMAND(ID_INSERT_SCAN_ALL, OnInsertScanAll)
	ON_COMMAND(ID_INSERT_SCAN_LAYER, OnInsertScanLayer)
	ON_UPDATE_COMMAND_UI(ID_INSERT_SCAN_LAYER, OnUpdateInsertScanLayer)
	ON_COMMAND(ID_BUTTON_SHOW_PROFILE, OnButtonShowProfile)
	ON_COMMAND(ID_INSERT_SCAN_DUMP, OnInsertScanDump)
	ON_COMMAND(ID_INSERT_EXPANDALL, OnInsertExpandall)
	ON_COMMAND(ID_INSERT_COLLAPSEALL, OnInsertCollapseall)
	ON_COMMAND(ID_INSERT_COLLAPSEBRANCH, OnInsertCollapsebranch)
	ON_COMMAND(ID_INSERT_EXPANDBRANCH, OnInsertExpandbranch)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

#ifdef _DEBUG
void LayerView::AssertValid() const
{
	CTreeView::AssertValid();
}

void LayerView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

BOOL LayerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class
	cs.style |= (TVS_SHOWSELALWAYS | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_EDITLABELS);

	return CTreeView::PreCreateWindow(cs);
}

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

const LayerView::LayerViewIconType LayerView::getIcon (const TerrainGenerator::LayerItem* layerItem)
{
	const TerrainGenerator::Layer* layer = dynamic_cast<const TerrainGenerator::Layer*> (layerItem);
	if (layer)
	{
		if (layer->getInvertBoundaries () && layer->getInvertFilters ())
			return LVIT_layerInvertBoundariesFilters;

		if (layer->getInvertBoundaries ())
			return LVIT_layerInvertBoundaries;

		if (layer->getInvertFilters ())
			return LVIT_layerInvertFilters;

		return LVIT_layer;
	}

	const TerrainGenerator::Boundary* boundary = dynamic_cast<const TerrainGenerator::Boundary*> (layerItem);
	if (boundary)
	{
		switch (boundary->getType ())
		{
		case TGBT_circle:
			return LVIT_boundaryCircle;

		case TGBT_rectangle:
			{
				if (boundary->getType () == TGBT_rectangle)
				{
					const BoundaryRectangle* boundaryRectangle = safe_cast<const BoundaryRectangle*> (boundary);

					if (boundaryRectangle->isLocalWaterTable ())
						return LVIT_boundaryRectangleWaterTable;
				}

				return LVIT_boundaryRectangle;
			}

		case TGBT_polygon:
			{
				if (boundary->getType () == TGBT_polygon)
				{
					const BoundaryPolygon* boundaryPolygon = safe_cast<const BoundaryPolygon*> (boundary);

					if (boundaryPolygon->isLocalWaterTable ())
						return LVIT_boundaryPolygonWaterTable;
				}

				return LVIT_boundaryPolygon;
			}

		case TGBT_polyline:
			return LVIT_boundaryPolyline;
		}
	}

	const TerrainGenerator::Filter* filter = dynamic_cast<const TerrainGenerator::Filter*> (layerItem);
	if (filter)
	{
		switch (filter->getType ())
		{
		case TGFT_height:
			return LVIT_filterHeight;

		case TGFT_fractal:
			return LVIT_filterFractal;

		case TGFT_slope:
			return LVIT_filterSlope;

		case TGFT_direction:
			return LVIT_filterDirection;

		case TGFT_shader:
			return LVIT_filterShader;

		case TGFT_bitmap:
			return LVIT_filterBitmap;

		}
	}

	const TerrainGenerator::Affector* affector = dynamic_cast<const TerrainGenerator::Affector*> (layerItem);
	if (affector)
	{
		switch (affector->getType ())
		{
		case TGAT_heightTerrace:
		case TGAT_heightConstant:
		case TGAT_heightFractal:
			return LVIT_affectorHeight;

		case TGAT_colorConstant:
		case TGAT_colorRampHeight:
		case TGAT_colorRampFractal:
			return LVIT_affectorColor;

		case TGAT_shaderConstant:
		case TGAT_shaderReplace:
			return LVIT_affectorShader;

		case TGAT_floraStaticCollidableConstant:
		case TGAT_floraStaticNonCollidableConstant:
			return LVIT_affectorFloraStatic;

		case TGAT_floraDynamicNearConstant:
		case TGAT_floraDynamicFarConstant:
			return LVIT_affectorFloraDynamic;

		case TGAT_exclude:
			return LVIT_affectorExclude;

		case TGAT_passable:
			return LVIT_affectorPassable;

		case TGAT_environment:
			return LVIT_affectorEnvironment;

		case TGAT_road:
			return LVIT_affectorRoad;

		case TGAT_river:
			return LVIT_affectorRiver;

		case TGAT_ribbon:
			return LVIT_affectorRibbon;
		}
	}

	return LVIT_unknown;
}

//-------------------------------------------------------------------

static inline bool keyDown (int key)
{
	return (GetKeyState (key) & 0x8000) != 0;
}

//-------------------------------------------------------------------

bool LayerView::canAddBoundary (void) const
{
	//-- if anything is selected
	return GetTreeCtrl ().GetSelectedItem () != 0;
}

//-------------------------------------------------------------------

bool LayerView::canAddFilter (void) const
{
	return canAddBoundary ();
}

//-------------------------------------------------------------------

bool LayerView::canAddAffector (void) const
{
	//-- if anything is selected and it's not a layer
	return canAddBoundary ();
}

//-------------------------------------------------------------------

void LayerView::addLayer (TerrainGenerator::Layer* newLayer) const
{
	//-- get the currently selected item
	HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	//-- if it's not a layer
	if (!isLayer (selection))
	{
		//-- it must be a child so get its parent and its parent must be a layer
		selection = GetTreeCtrl ().GetParentItem (selection);

		FATAL (!isLayer (selection), ("selected item is not a layer"));
	}

	//-- get the layer from the item
	TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
	FATAL (item->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

	//-- add layer to selected layer
	TerrainGenerator::Layer* layer = safe_cast<TerrainGenerator::Layer*> (item->layerItem);
	layer->addLayer (newLayer);

	//-- add layer to tree
	HTREEITEM layerItem = addLayerToTree (selection, 0, newLayer);
	IGNORE_RETURN (GetTreeCtrl ().SelectItem (layerItem));

	//-- tell document to update views
	CDocument* doc = GetDocument ();
	doc->UpdateAllViews (0);
	doc->SetModifiedFlag ();
}

//-------------------------------------------------------------------

void LayerView::addBoundary (TerrainGenerator::Boundary* newBoundary) const
{
	//-- make sure the name is valid
	NOT_NULL (newBoundary->getName ());

	//-- get the currently selected item
	HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	//-- if it's not an layer
	if (!isLayer (selection))
	{
		//-- it must be a child so get its parent and its parent must be an layer
		selection = GetTreeCtrl ().GetParentItem (selection);

		FATAL (!isLayer (selection), ("selected item is not an layer nor anything below layer"));
	}

	//-- get the layer from the item
	TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
	FATAL (item->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));
	TerrainGenerator::Layer* layer = safe_cast<TerrainGenerator::Layer*> (item->layerItem);

	//-- add boundary to selected layer
	layer->addBoundary (newBoundary);

	//-- add boundary to tree
	HTREEITEM boundaryItem = addBoundaryToTree (selection, newBoundary);
	IGNORE_RETURN (GetTreeCtrl ().SelectItem (boundaryItem));

	//-- tell document to update views
	CDocument* doc = GetDocument ();
	doc->UpdateAllViews (0);
	doc->SetModifiedFlag ();
}

//-------------------------------------------------------------------

void LayerView::addFilter (TerrainGenerator::Filter* newFilter) const
{
	//-- make sure the name is valid
	NOT_NULL (newFilter->getName ());

	//-- get the currently selected item
	HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	//-- if it's not an layer
	if (!isLayer (selection))
	{
		//-- it must be a child so get its parent and its parent must be an layer
		selection = GetTreeCtrl ().GetParentItem (selection);

		FATAL (!isLayer (selection), ("selected item is not an layer nor anything below layer"));
	}

	//-- get the layer from the item
	TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
	FATAL (item->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));
	TerrainGenerator::Layer* layer = safe_cast<TerrainGenerator::Layer*> (item->layerItem);

	//-- add filter to selected layer
	layer->addFilter (newFilter);

	//-- add filter to tree
	HTREEITEM filterItem = addFilterToTree (selection, newFilter);
	IGNORE_RETURN (GetTreeCtrl ().SelectItem (filterItem));

	//-- tell document to update views
	CDocument* doc = GetDocument ();
	doc->UpdateAllViews (0);
	doc->SetModifiedFlag ();
}

//-------------------------------------------------------------------

void LayerView::addAffector (TerrainGenerator::Affector* newAffector) const
{
	//-- make sure the name is valid
	NOT_NULL (newAffector->getName ());

	//-- get the currently selected item
	HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	//-- if it's not an layer
	if (!isLayer (selection))
	{
		//-- it must be a child so get its parent and its parent must be an layer
		selection = GetTreeCtrl ().GetParentItem (selection);

		FATAL (!isLayer (selection), ("selected item is not an layer nor anything below layer"));
	}

	//-- get the layer from the item
	TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
	FATAL (item->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));
	TerrainGenerator::Layer* layer = safe_cast<TerrainGenerator::Layer*> (item->layerItem);

	//-- add affector to selected layer
	layer->addAffector (newAffector);

	//-- add affector to tree
	HTREEITEM affectorItem = addAffectorToTree (selection, newAffector);
	IGNORE_RETURN (GetTreeCtrl ().SelectItem (affectorItem));

	//-- tell document to update views
	CDocument* doc = GetDocument ();
	doc->UpdateAllViews (0);
	doc->SetModifiedFlag ();
}

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

HTREEITEM LayerView::getRoot (HTREEITEM hitem) const
{
	HTREEITEM parent         = hitem;
	HTREEITEM possibleParent = hitem;

	do
	{
		possibleParent = GetTreeCtrl ().GetParentItem (possibleParent);

		if (possibleParent)
			parent = possibleParent;
	}
	while (possibleParent != 0);

	return parent;
}

//-------------------------------------------------------------------

bool LayerView::isLayer (HTREEITEM selection) const
{
	const TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));

	return item->type == TerrainEditorDoc::Item::T_layer;
}

//-------------------------------------------------------------------

HTREEITEM LayerView::getSelectedLayer (void) const
{
	//-- am i pointing to anything?
	const HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	if (selection)
		return isLayer (selection) ? selection : 0;
	else
		return 0;
}

//-------------------------------------------------------------------

bool LayerView::isLayerSelected (void) const
{
	return getSelectedLayer () != 0;
}

//-------------------------------------------------------------------

bool LayerView::isBoundary (HTREEITEM selection) const
{
	const TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));

	return item->type == TerrainEditorDoc::Item::T_boundary;
}

//-------------------------------------------------------------------

HTREEITEM LayerView::getSelectedBoundary (void) const
{
	//-- am i pointing to anything?
	const HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	if (selection)
		return isBoundary (selection) ? selection : 0;
	else
		return 0;
}

//-------------------------------------------------------------------

bool LayerView::isBoundarySelected (void) const
{
	return getSelectedBoundary () != 0;
}

//-------------------------------------------------------------------

bool LayerView::isFilter (HTREEITEM selection) const
{
	const TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));

	return item->type == TerrainEditorDoc::Item::T_filter;
}

//-------------------------------------------------------------------

HTREEITEM LayerView::getSelectedFilter (void) const
{
	//-- am i pointing to anything?
	const HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	if (selection)
		return isFilter (selection) ? selection : 0;
	else
		return 0;
}

//-------------------------------------------------------------------

bool LayerView::isFilterSelected (void) const
{
	return getSelectedFilter () != 0;
}

//-------------------------------------------------------------------

bool LayerView::isAffector (HTREEITEM selection) const
{
	const TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));

	return item->type == TerrainEditorDoc::Item::T_affector;
}

//-------------------------------------------------------------------

HTREEITEM LayerView::getSelectedAffector (void) const
{
	//-- am i pointing to anything?
	const HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	if (selection)
		return isAffector (selection) ? selection : 0;
	else
		return 0;
}

//-------------------------------------------------------------------

bool LayerView::isAffectorSelected (void) const
{
	return getSelectedAffector () != 0;
}

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void LayerView::deleteLayer (void) const
{
	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	TerrainGenerator* terrainGenerator = doc->getTerrainGenerator ();
	NOT_NULL (terrainGenerator);

	// set property view to default dialog before destroying data in dlg
	PropertyView::ViewData dummyData;
	doc->SetPropertyView (RUNTIME_CLASS (PropertyView), &dummyData);

	HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	//-- get layer item
	TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
	FATAL (item->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

	//--
	HTREEITEM parent = GetTreeCtrl ().GetParentItem (selection);
	if (parent)
	{
		TerrainEditorDoc::Item* parentItem = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (parent));
		FATAL (parentItem->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

		TerrainGenerator::Layer* parentLayer = safe_cast<TerrainGenerator::Layer*> (parentItem->layerItem);

		//-- remove layer from parent
		parentLayer->removeLayer (safe_cast<TerrainGenerator::Layer*> (item->layerItem), true);
	}
	else
	{
		//-- remove layer from generator
		terrainGenerator->removeLayer (safe_cast<TerrainGenerator::Layer*> (item->layerItem), true);
	}

	//-- tell doc this was deleted
	if (doc->getSelectedItem ()->layerItem == item->layerItem)
		doc->setSelectedItem (0);

	//-- remove layer from tree
	IGNORE_RETURN (GetTreeCtrl ().DeleteItem (selection));
}

//-------------------------------------------------------------------

void LayerView::deleteBoundary (void) const
{
	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	TerrainGenerator* terrainGenerator = doc->getTerrainGenerator ();
	NOT_NULL (terrainGenerator);

	// set property view to default dialog before destroying data in dlg
	PropertyView::ViewData dummyData;
	doc->SetPropertyView (RUNTIME_CLASS(PropertyView), &dummyData);

	HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	//-- get layer item
	TerrainEditorDoc::Item* boundaryItem = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
	FATAL (boundaryItem->type != TerrainEditorDoc::Item::T_boundary, ("call to wrong function"));

	//--
	HTREEITEM parent = GetTreeCtrl ().GetParentItem (selection);

	TerrainEditorDoc::Item* layerItem = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (parent));
	FATAL (layerItem->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

	TerrainGenerator::Layer* layer = safe_cast<TerrainGenerator::Layer*> (layerItem->layerItem);

	//-- remove boundary from generator
	layer->removeBoundary (safe_cast<TerrainGenerator::Boundary*> (boundaryItem->layerItem), true);

	//-- tell doc this was deleted
	if (doc->getSelectedItem ()->layerItem == boundaryItem->layerItem)
		doc->setSelectedItem (0);

	//-- remove boundary from tree
	IGNORE_RETURN (GetTreeCtrl ().DeleteItem (selection));
}

//-------------------------------------------------------------------

void LayerView::deleteFilter (void) const
{
	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	TerrainGenerator* terrainGenerator = doc->getTerrainGenerator ();
	NOT_NULL (terrainGenerator);

	// set property view to default dialog before destroying data in dlg
	PropertyView::ViewData dummyData;
	doc->SetPropertyView (RUNTIME_CLASS(PropertyView), &dummyData);

	HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	//-- get layer item
	TerrainEditorDoc::Item* filterItem = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
	FATAL (filterItem->type != TerrainEditorDoc::Item::T_filter, ("call to wrong function"));

	//--
	HTREEITEM parent = GetTreeCtrl ().GetParentItem (selection);

	TerrainEditorDoc::Item* layerItem = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (parent));
	FATAL (layerItem->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

	TerrainGenerator::Layer* layer = safe_cast<TerrainGenerator::Layer*> (layerItem->layerItem);

	//-- remove filter from generator
	layer->removeFilter (safe_cast<TerrainGenerator::Filter*> (filterItem->layerItem), true);

	//-- tell doc this was deleted
	if (doc->getSelectedItem ()->layerItem == filterItem->layerItem)
		doc->setSelectedItem (0);

	//-- remove filter from tree
	IGNORE_RETURN (GetTreeCtrl ().DeleteItem (selection));
}

//-------------------------------------------------------------------

void LayerView::deleteAffector (void) const
{
	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	TerrainGenerator* terrainGenerator = doc->getTerrainGenerator ();
	NOT_NULL (terrainGenerator);

	// set property view to default dialog before destroying data in dlg
	PropertyView::ViewData dummyData;
	doc->SetPropertyView (RUNTIME_CLASS(PropertyView), &dummyData);

	HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	//-- get layer item
	TerrainEditorDoc::Item* affectorItem = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
	FATAL (affectorItem->type != TerrainEditorDoc::Item::T_affector, ("call to wrong function"));

	//--
	HTREEITEM parent = GetTreeCtrl ().GetParentItem (selection);

	TerrainEditorDoc::Item* layerItem = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (parent));
	FATAL (layerItem->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

	TerrainGenerator::Layer* layer = safe_cast<TerrainGenerator::Layer*> (layerItem->layerItem);

	//-- remove affector from generator
	layer->removeAffector (safe_cast<TerrainGenerator::Affector*> (affectorItem->layerItem), true);

	//-- tell doc this was deleted
	if (doc->getSelectedItem ()->layerItem == affectorItem->layerItem)
		doc->setSelectedItem (0);

	//-- remove affector from tree
	IGNORE_RETURN (GetTreeCtrl ().DeleteItem (selection));
}

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void LayerView::OnButtonDeleteLayer()
{
	TerrainEditorDoc* doc  = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	//-- make sure we're not deleting the root layer
	{
		const TerrainGenerator* const terrainGenerator = doc->getTerrainGenerator ();
		NOT_NULL (terrainGenerator);

		if (!GetTreeCtrl ().GetParentItem (selection) && terrainGenerator->getNumberOfLayers () == 1)
		{
			MessageBox ("You can't delete the root layer!", 0, MB_OK);

			return;
		}
	}

	CString name = GetTreeCtrl ().GetItemText (selection);

	CString tmp;
	tmp.Format ("Are you sure you want to delete %s?", name);

	if (MessageBox (tmp, 0, MB_YESNO) == IDYES)
	{
		if (isLayerSelected ())
			deleteLayer ();
		else
			if (isBoundarySelected ())
				deleteBoundary ();
			else
				if (isFilterSelected ())
					deleteFilter ();
				else
					if (isAffectorSelected ())
						deleteAffector ();
					else
						FATAL (true, ("asommers has NO idea what this is!"));

		if (GetTreeCtrl ().GetCount () == 0)
			doc->SetPropertyView (0, 0);

		//-- tell document to update views
		doc->UpdateAllViews (0);
		doc->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

HTREEITEM LayerView::addBoundaryToTree (HTREEITEM parent, TerrainGenerator::Boundary* boundary) const
{
	//-- search for the first filter, it goes before that...
	HTREEITEM afterItem = 0;

	{
		HTREEITEM current = GetTreeCtrl ().GetChildItem (parent);

		if (current)
		{
			for (;;)
			{
				TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (current));

				if (item->type >= TerrainEditorDoc::Item::T_filter)
				{
					afterItem = GetTreeCtrl ().GetPrevSiblingItem (current);

					if (!afterItem)
						afterItem = TVI_FIRST;

					break;
				}

				current = GetTreeCtrl ().GetNextSiblingItem (current);
				if (!current)
					break;
			}
		}
	}

	//-- insert into tree
	const LayerViewIconType iconType = getIcon (boundary);

	const HTREEITEM boundaryItem = GetTreeCtrl ().InsertItem (boundary->getName (), iconType, iconType, parent, afterItem);
	IGNORE_RETURN (GetTreeCtrl ().SetCheck (boundaryItem, boundary->isActive ()));

	//-- set item
	TerrainEditorDoc::Item* item      = new TerrainEditorDoc::Item;
	item->type      = TerrainEditorDoc::Item::T_boundary;
	item->layerItem = boundary;
	IGNORE_RETURN (GetTreeCtrl ().SetItemData (boundaryItem, reinterpret_cast<DWORD> (item)));

	return boundaryItem;
}

//-------------------------------------------------------------------

HTREEITEM LayerView::addFilterToTree (HTREEITEM parent, TerrainGenerator::Filter* filter) const
{
	//-- search for the first affector, it goes before that...
	HTREEITEM afterItem = 0;

	{
		HTREEITEM current = GetTreeCtrl ().GetChildItem (parent);

		if (current)
		{
			for (;;)
			{
				TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (current));

				if (item->type >= TerrainEditorDoc::Item::T_affector)
				{
					afterItem = GetTreeCtrl ().GetPrevSiblingItem (current);

					if (!afterItem)
						afterItem = TVI_FIRST;

					break;
				}

				current = GetTreeCtrl ().GetNextSiblingItem (current);
				if (!current)
					break;
			}
		}
	}

	//-- insert into tree
	const LayerViewIconType iconType = getIcon (filter);

	const HTREEITEM filterItem = GetTreeCtrl ().InsertItem (filter->getName (), iconType, iconType, parent, afterItem);
	IGNORE_RETURN (GetTreeCtrl ().SetCheck (filterItem, filter->isActive ()));

	//-- set item
	TerrainEditorDoc::Item* item      = new TerrainEditorDoc::Item;
	item->type      = TerrainEditorDoc::Item::T_filter;
	item->layerItem = filter;
	IGNORE_RETURN (GetTreeCtrl ().SetItemData (filterItem, reinterpret_cast<DWORD> (item)));

	return filterItem;
}

//-------------------------------------------------------------------

HTREEITEM LayerView::addAffectorToTree (HTREEITEM parent, TerrainGenerator::Affector* affector) const
{
	//-- search for the first layer, it goes before that...
	HTREEITEM afterItem = 0;

	{
		HTREEITEM current = GetTreeCtrl ().GetChildItem (parent);

		if (current)
		{
			for (;;)
			{
				TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (current));

				if (item->type >= TerrainEditorDoc::Item::T_layer)
				{
					afterItem = GetTreeCtrl ().GetPrevSiblingItem (current);

					if (!afterItem)
						afterItem = TVI_FIRST;

					break;
				}

				current = GetTreeCtrl ().GetNextSiblingItem (current);
				if (!current)
					break;
			}
		}
	}

	//-- insert into tree
	const LayerViewIconType iconType = getIcon (affector);

	const HTREEITEM affectorItem = GetTreeCtrl ().InsertItem (affector->getName (), iconType, iconType, parent, afterItem);
	IGNORE_RETURN (GetTreeCtrl ().SetCheck (affectorItem, affector->isActive ()));

	//-- set item
	TerrainEditorDoc::Item* item      = new TerrainEditorDoc::Item;
	item->type      = TerrainEditorDoc::Item::T_affector;
	item->layerItem = affector;
	IGNORE_RETURN (GetTreeCtrl ().SetItemData (affectorItem, reinterpret_cast<DWORD> (item)));

	return affectorItem;
}

//-------------------------------------------------------------------

HTREEITEM LayerView::addLayerToTree (HTREEITEM parent, HTREEITEM afterItem, TerrainGenerator::Layer* layer) const
{
	//-- insert into tree
	const LayerViewIconType iconType = getIcon (layer);

	const HTREEITEM sublayerItem = GetTreeCtrl ().InsertItem (layer->getName (), iconType, iconType, parent, afterItem);
	IGNORE_RETURN (GetTreeCtrl ().SetCheck (sublayerItem, layer->isActive ()));

	//-- set item
	TerrainEditorDoc::Item* item      = new TerrainEditorDoc::Item;
	item->type      = TerrainEditorDoc::Item::T_layer;
	item->layerItem = layer;
	IGNORE_RETURN (GetTreeCtrl ().SetItemData (sublayerItem, reinterpret_cast<DWORD> (item)));

	//-- walk boundaries, filters and affectors
	int i;
	for (i = 0; i < layer->getNumberOfBoundaries (); i++)
		IGNORE_RETURN (addBoundaryToTree (sublayerItem, layer->getBoundary (i)));

	for (i = 0; i < layer->getNumberOfFilters (); i++)
		IGNORE_RETURN (addFilterToTree (sublayerItem, layer->getFilter (i)));

	for (i = 0; i < layer->getNumberOfAffectors (); i++)
		IGNORE_RETURN (addAffectorToTree (sublayerItem, layer->getAffector (i)));

	for (i = 0; i < layer->getNumberOfLayers (); i++)
		IGNORE_RETURN (addLayerToTree (sublayerItem, 0, layer->getLayer (i)));

	//-- set its expanded state
	if (layer->getExpanded ())
		IGNORE_RETURN (GetTreeCtrl ().SetItemState (sublayerItem, TVIS_EXPANDED, TVIS_EXPANDED));

	return sublayerItem;
}

//-------------------------------------------------------------------

void LayerView::OnButtonPromoteLayer()
{
	//-- root layers
	if (isLayerSelected () && GetTreeCtrl ().GetParentItem (getSelectedLayer ()) == 0)
	{
		HTREEITEM selection = getSelectedLayer ();
		HTREEITEM next      = GetTreeCtrl ().GetNextSiblingItem (selection);

		if (next)
		{
			TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
			NOT_NULL (doc);

			TerrainGenerator* terrainGenerator = doc->getTerrainGenerator ();
			NOT_NULL (terrainGenerator);

			//-- get layer item
			TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
			FATAL (item->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

			TerrainGenerator::Layer* layer = safe_cast<TerrainGenerator::Layer*> (item->layerItem);

			//-- promote layer
			terrainGenerator->promoteLayer (layer);

			//-- remove layer from tree
			saveExpandedState (selection);
			IGNORE_RETURN (GetTreeCtrl ().DeleteItem (selection));

			//-- reinsert layer after next
			HTREEITEM inserted = addLayerToTree (TVI_ROOT, next, layer);
			IGNORE_RETURN (GetTreeCtrl ().SelectItem (inserted));

			//-- tell document to update views
			doc->UpdateAllViews (0);
			doc->SetModifiedFlag ();
		}
	}
	else
		//-- sub layers
		if (isLayerSelected () && GetTreeCtrl ().GetParentItem (getSelectedLayer ()) != 0)
		{
			HTREEITEM selection = getSelectedLayer ();
			HTREEITEM next      = GetTreeCtrl ().GetNextSiblingItem (selection);

			if (next)
			{
				TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
				NOT_NULL (doc);

				TerrainGenerator* terrainGenerator = doc->getTerrainGenerator ();
				NOT_NULL (terrainGenerator);

				//-- get layer item
				TerrainEditorDoc::Item* layerItem = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
				FATAL (layerItem->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

				TerrainGenerator::Layer* layer = safe_cast<TerrainGenerator::Layer*> (layerItem->layerItem);

				//-- get parent item
				HTREEITEM parent = GetTreeCtrl ().GetParentItem (selection);

				TerrainEditorDoc::Item* parentItem = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (parent));
				FATAL (parentItem->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

				TerrainGenerator::Layer* parentLayer = safe_cast<TerrainGenerator::Layer*> (parentItem->layerItem);

				//-- promote layer
				parentLayer->promoteLayer (layer);

				//-- remove layer from tree
				saveExpandedState (selection);
				IGNORE_RETURN (GetTreeCtrl ().DeleteItem (selection));

				//-- reinsert layer after next
				HTREEITEM inserted = addLayerToTree (GetTreeCtrl ().GetParentItem (next), next, layer);
				IGNORE_RETURN (GetTreeCtrl ().SelectItem (inserted));

				//-- tell document to update views
				doc->UpdateAllViews (0);
				doc->SetModifiedFlag ();
			}
		}
}

//-------------------------------------------------------------------

void LayerView::OnButtonDemoteLayer()
{
	if (isLayerSelected () && GetTreeCtrl ().GetParentItem (getSelectedLayer ()) == 0)
	{
		HTREEITEM selection = getSelectedLayer ();
		HTREEITEM prev      = GetTreeCtrl ().GetPrevSiblingItem (selection);

		if (prev && reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (prev))->type != TerrainEditorDoc::Item::T_layer)
			prev = 0;

		if (prev)
		{
			TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
			NOT_NULL (doc);

			TerrainGenerator* terrainGenerator = doc->getTerrainGenerator ();
			NOT_NULL (terrainGenerator);

			//-- get layer item
			TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
			FATAL (item->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

			TerrainGenerator::Layer* layer = safe_cast<TerrainGenerator::Layer*> (item->layerItem);

			//-- get previous layer item
			TerrainEditorDoc::Item* prevItem = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (prev));
			FATAL (prevItem->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

			TerrainGenerator::Layer* prevLayer = safe_cast<TerrainGenerator::Layer*> (prevItem->layerItem);

			//-- demote layer
			terrainGenerator->demoteLayer (layer);

			//-- remove previous layer from tree
			saveExpandedState (prev);
			IGNORE_RETURN (GetTreeCtrl ().DeleteItem (prev));

			//-- reinsert layer after next
			IGNORE_RETURN (addLayerToTree (TVI_ROOT, selection, prevLayer));
			IGNORE_RETURN (GetTreeCtrl ().SelectItem (selection));

			//-- tell document to update views
			doc->UpdateAllViews (0);
			doc->SetModifiedFlag ();
		}
	}
	else
		if (isLayerSelected () && GetTreeCtrl ().GetParentItem (getSelectedLayer ()) != 0)
		{
			HTREEITEM selection = getSelectedLayer ();
			HTREEITEM prev      = GetTreeCtrl ().GetPrevSiblingItem (selection);

			if (prev && reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (prev))->type != TerrainEditorDoc::Item::T_layer)
				prev = 0;

			if (prev)
			{
				TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
				NOT_NULL (doc);

				TerrainGenerator* terrainGenerator = doc->getTerrainGenerator ();
				NOT_NULL (terrainGenerator);

				//-- get layer item
				TerrainEditorDoc::Item* layerItem = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
				FATAL (layerItem->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

				TerrainGenerator::Layer* layer = safe_cast<TerrainGenerator::Layer*> (layerItem->layerItem);

				//-- get previous layer item
				TerrainEditorDoc::Item* prevLayerItem = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (prev));
				FATAL (prevLayerItem->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

				TerrainGenerator::Layer* prevLayer = safe_cast<TerrainGenerator::Layer*> (prevLayerItem->layerItem);

				//-- get parent item
				HTREEITEM parent = GetTreeCtrl ().GetParentItem (selection);

				TerrainEditorDoc::Item* parentItem = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (parent));
				FATAL (parentItem->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

				TerrainGenerator::Layer* parentLayer = safe_cast<TerrainGenerator::Layer*> (parentItem->layerItem);

				//-- promote layer
				parentLayer->demoteLayer (layer);

				//-- remove previous layer from tree
				saveExpandedState (prev);
				IGNORE_RETURN (GetTreeCtrl ().DeleteItem (prev));

				//-- reinsert layer after next
				IGNORE_RETURN (addLayerToTree (GetTreeCtrl ().GetParentItem (selection), selection, prevLayer));
				IGNORE_RETURN (GetTreeCtrl ().SelectItem (selection));

				//-- tell document to update views
				doc->UpdateAllViews (0);
				doc->SetModifiedFlag ();
			}
		}
}

//-------------------------------------------------------------------

void LayerView::OnUpdateButtonPromoteLayer(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ((GetTreeCtrl ().GetCount () != 0 && (isLayerSelected () || isLayerSelected () || isAffectorSelected ())) ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void LayerView::OnUpdateButtonDeleteLayer(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ((GetTreeCtrl ().GetCount () != 0) ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void LayerView::OnUpdateButtonDemoteLayer(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ((GetTreeCtrl ().GetCount () != 0 && (isLayerSelected () || isLayerSelected () || isAffectorSelected ())) ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void LayerView::OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = reinterpret_cast<NM_TREEVIEW*> (pNMHDR);

	//-- get the item to delete
	HTREEITEM selection = pNMTreeView->itemOld.hItem;

	//-- delete the item data associated with the item
	TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));

	if (item)
		delete item;

	//-- set the data to 0
	IGNORE_RETURN (GetTreeCtrl ().SetItemData (selection, 0));

	*pResult = 0;
}

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void LayerView::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult)
{
	const TV_KEYDOWN* pTVKeyDown = reinterpret_cast<const TV_KEYDOWN*> (pNMHDR);

	switch (pTVKeyDown->wVKey)
	{
	case VK_DELETE:
		{
			//-- same as delete
			OnButtonDeleteLayer ();
		}
		break;
	}

	*pResult = 0;
}

//-------------------------------------------------------------------

void LayerView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	UNREF (pNMTreeView);

	//-- get the selection
	HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	if (selection)
	{
		TerrainEditorDoc* const doc  = safe_cast<TerrainEditorDoc*> (GetDocument ());

//		if (doc->getSelectedItem ())
//			GetTreeCtrl ().SetItemState (find (GetTreeCtrl ().GetRootItem (), doc->getSelectedItem ()->layerItem), 0, TVIS_BOLD);

//		GetTreeCtrl ().SetItemState (selection, TVIS_BOLD, TVIS_BOLD);

		//-- get the item data
		TerrainEditorDoc::Item* const item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
		doc->setSelectedItem (item);

		//-- show item properties
		FormLayerItem::FormLayerItemViewData bld;
		bld.item = item;
		doc->SetPropertyView (doc->getLayerRuntimeClass (item), &bld);
		doc->UpdateAllViews (0);

		GetParentFrame ()->ActivateFrame ();
	}

	*pResult = 0;
}

//-------------------------------------------------------------------

void LayerView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

	if (!imageListSet)
	{
		IGNORE_RETURN (imageList.Create (IDB_BITMAP_LAYER, 16, 1, RGB (255,255,255)));
		IGNORE_RETURN (GetTreeCtrl ().SetImageList (&imageList, TVSIL_NORMAL));

		imageListSet = true;
	}

	//-- according to the docs:
	//	If you want to use this style, you must set the TVS_CHECKBOXES style with
	//	SetWindowLong after you create the treeview control, and before you populate
	//	the tree. Otherwise, the checkboxes might appear unchecked, depending on
	//	timing issues
	IGNORE_RETURN (SetWindowLong (m_hWnd, GWL_STYLE, static_cast<long> (GetStyle () | TVS_CHECKBOXES)));

	//-- fill the tree with the documents layer data
	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	TerrainGenerator* terrainGenerator = doc->getTerrainGenerator ();
	NOT_NULL (terrainGenerator);

	if (terrainGenerator->getNumberOfLayers () == 0)
	{
		//-- create a new layer
		TerrainGenerator::Layer* layer = new TerrainGenerator::Layer ();

		//-- find a unique name
		layer->setName ("Layer");

		//-- add layer to generator
		terrainGenerator->addLayer (layer);
	}

	int i;
	for (i = 0; i < terrainGenerator->getNumberOfLayers (); i++)
		IGNORE_RETURN (addLayerToTree (TVI_ROOT, 0, terrainGenerator->getLayer (i)));

	//-- expand the tree
	expandAll ();

	IGNORE_RETURN (GetTreeCtrl ().SelectItem (GetTreeCtrl ().GetRootItem ()));
	IGNORE_RETURN (GetTreeCtrl ().EnsureVisible (GetTreeCtrl ().GetRootItem()));
}

//-------------------------------------------------------------------

void LayerView::expandBranch (HTREEITEM item)
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

void LayerView::expandAll (void)
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

void LayerView::updateTree (HTREEITEM selection)
{
	//
	//-- update the item
	//

	//-- get the item data
	TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));

	//-- get the current check state
	bool checked = (GetTreeCtrl ().GetCheck (selection) != 0);

	//-- if they differ, the user has changed the state, so update accordingly
	item->layerItem->setActive (checked);

	//
	//-- update the items children
	//
	HTREEITEM child = GetTreeCtrl ().GetChildItem (selection);

	if (child)
	{
		do
		{
			updateTree (child);

			child = GetTreeCtrl ().GetNextSiblingItem (child);
		}
		while (child);
	}
}

//-------------------------------------------------------------------

void LayerView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	UNREF (pSender);
	UNREF (lHint);
	UNREF (pHint);

	//-- recurse tree and update children
	HTREEITEM item = GetTreeCtrl ().GetRootItem ();

	if (item)
	{
		do
		{
			updateTree (item);

			item = GetTreeCtrl ().GetNextSiblingItem (item);
		}
		while (item);
	}

	saveExpandedState ();

	Invalidate ();
}

//-------------------------------------------------------------------

bool LayerView::verify (void)
{
	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	//--
	ArrayList<TerrainGeneratorHelper::OutputData> output;

	TerrainGeneratorHelper::verify (doc->getTerrainGenerator (), output);

	if (output.getNumberOfElements () > 0)
	{
		GetApp ()->showWarning (true);
		GetApp ()->showConsole ();

		//--
		CString tmp;
		tmp = "-----------------\r\n";
		CONSOLE_PRINT (tmp);
	}

	//--
	int i;
	for (i = 0; i < output.getNumberOfElements (); i++)
	{
		switch (output [i].mode)
		{
		case TerrainGeneratorHelper::OutputData::M_warning:
			{
				if (doc->getWarningFrame ())
					doc->getWarningFrame ()->update (output [i]);
			}
			break;

		case TerrainGeneratorHelper::OutputData::M_console:
			{
				CONSOLE_PRINT (*output [i].message);
				CONSOLE_PRINT ("\r\n");
			}
			break;
		}

		delete output [i].message;
	}

	if (output.getNumberOfElements () > 0)
	{
		CString buffer;
		buffer.Format ("%i errors found, fix before continuing...", output.getNumberOfElements ());

		IGNORE_RETURN (MessageBox (buffer));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------

void LayerView::OnButtonApply()
{
	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	//-- clear times
	doc->getTerrainGenerator ()->resetProfileData ();

	//-- update the tree to reflect active/inactive changes
 	OnUpdate (0, 0, 0);

	//-- fill the tree with the documents layer data
	doc->getPropertyFrame ()->ApplyChanges ();

	//-- reload all the bitmaps
	doc->getTerrainGenerator()->getBitmapGroup().reloadAllFamilyBitmaps();

	//-- update the generator's internal data for optimizations since we may have changed things
	doc->getTerrainGenerator ()->prepare ();
	refreshIcons ();

	if (!verify ())
		return;

	//-- tell document to update views
	doc->UpdateAllViews (this, TerrainEditorDoc::H_layerViewApply);
	doc->SetModifiedFlag ();
}

//-------------------------------------------------------------------

void LayerView::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREF (pNMHDR);

	//-- limit to 100 characters
	GetTreeCtrl ().GetEditControl()->LimitText (100);

	//-- don't edit
	*pResult = 0;
}

//-------------------------------------------------------------------

void LayerView::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult)
{
	//-- get the tree view display info
	NMTVDISPINFO* pTVDispInfo = reinterpret_cast<NMTVDISPINFO*> (pNMHDR);

	//-- don't edit by default
	*pResult = 0;

	//-- get the new text string
	const char* newName = pTVDispInfo->item.pszText;

	if (!newName || istrlen (newName) == 0)
		return;

	//-- get the item data
	TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (pTVDispInfo->item.hItem));
	item->layerItem->setName (newName);

	//-- tell tree about new name
	IGNORE_RETURN (GetTreeCtrl ().SetItem (&pTVDispInfo->item));

	*pResult = 1;

	//-- tell document to update views
	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	doc->UpdateAllViews (0);
	doc->SetModifiedFlag ();
}

//-------------------------------------------------------------------

void LayerView::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult)
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
	IGNORE_RETURN (m_pDragImage->GetImageInfo( 0, &ii ));
	pt.x = (ii.rcImage.right - ii.rcImage.left) / 2;
	pt.y = (ii.rcImage.bottom - ii.rcImage.top) / 2;

	IGNORE_RETURN (m_pDragImage->BeginDrag( 0, pt ));
	pt = pNMTreeView->ptDrag;
	ClientToScreen( &pt );
	IGNORE_RETURN (m_pDragImage->DragEnter(NULL,pt));

	SetCapture();

	*pResult = 0;
}

//-------------------------------------------------------------------

void LayerView::OnLButtonUp (UINT nFlags, CPoint point)
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

void LayerView::OnLButtonUpForCopy ()
{
	m_dragMode = DM_nothing;

	IGNORE_RETURN (CImageList::DragLeave (this));
	CImageList::EndDrag ();

	ReleaseCapture();

	delete m_pDragImage;

	IGNORE_RETURN (GetTreeCtrl ().SelectDropTarget (NULL));
	m_htiOldDrop = NULL;

	//-- exit out if no target was selected to drop on
	if (m_htiDrop == NULL)
		return;

	if (m_idTimer)
	{
		IGNORE_RETURN (KillTimer (static_cast<int> (m_idTimer)));
		m_idTimer = 0;
	}

	//-- get the document
	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	//-- get the destination item
	TerrainEditorDoc::Item*  dstItem   = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (m_htiDrop));
	HTREEITEM                dstParent = 0;
	TerrainGenerator::Layer* dstLayer  = 0;

	//-- find the destination layer
	switch (dstItem->type)
	{
	case TerrainEditorDoc::Item::T_layer:
		{
			dstParent = m_htiDrop;
			dstLayer  = safe_cast<TerrainGenerator::Layer*> (dstItem->layerItem);
		}
		break;

	case TerrainEditorDoc::Item::T_boundary:
	case TerrainEditorDoc::Item::T_filter:
	case TerrainEditorDoc::Item::T_affector:
		{
			//-- get the parent destination item
			dstParent = GetTreeCtrl ().GetParentItem (m_htiDrop);

			//-- get the item data
			TerrainEditorDoc::Item* dstParentItem = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (dstParent));
			FATAL (dstParentItem->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

			//-- get the destination layer
			dstLayer = safe_cast<TerrainGenerator::Layer*> (dstParentItem->layerItem);
		}
		break;

	default:
		DEBUG_FATAL (true, ("unknown layer item type"));
		break;
	}

	NOT_NULL (dstLayer);

	//-- get the source item
	TerrainEditorDoc::Item* srcItem   = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (m_htiDrag));

	//-- determine what can be moved to what
	switch (srcItem->type)
	{
	case TerrainEditorDoc::Item::T_layer:
		{
			//-- get the source layer
			TerrainGenerator::Layer* srcLayer = safe_cast<TerrainGenerator::Layer*> (srcItem->layerItem);

			Iff iff (1024);
			iff.allowNonlinearFunctions ();
			srcLayer->save (iff);
			iff.goToTopOfForm ();

			//-- create boundary
			TerrainGenerator::Layer* newLayer = new TerrainGenerator::Layer ();
			newLayer->load (iff, doc->getTerrainGenerator ());

			dstLayer->addLayer (newLayer);

			//-- find where to add
			HTREEITEM selection = addLayerToTree (dstParent, 0, newLayer);
			IGNORE_RETURN (GetTreeCtrl ().SelectItem (selection));
		}
		break;

	case TerrainEditorDoc::Item::T_boundary:
		{
			//-- get the source layer
			TerrainGenerator::Boundary* srcBoundary = safe_cast<TerrainGenerator::Boundary*> (srcItem->layerItem);

			//-- save boundary
			Iff iff (1024);
			iff.allowNonlinearFunctions ();
			iff.insertForm (srcBoundary->getTag ());
				srcBoundary->save (iff);
			iff.exitForm ();
			iff.goToTopOfForm ();

			//-- create boundary
			TerrainGenerator::Boundary* newBoundary = TerrainGeneratorLoader::createBoundary (iff);

			if (newBoundary)
			{
				//-- add the boundary to the dstLayer
				dstLayer->addBoundary (newBoundary);

				//-- add boundary to the tree
				HTREEITEM selection = addBoundaryToTree (dstParent, newBoundary);
				IGNORE_RETURN (GetTreeCtrl ().SelectItem (selection));
			}
		}
		break;

	case TerrainEditorDoc::Item::T_filter:
		{
			//-- get the source layer
			TerrainGenerator::Filter* srcFilter = safe_cast<TerrainGenerator::Filter*> (srcItem->layerItem);

			//-- save filter
			Iff iff (1024);
			iff.allowNonlinearFunctions ();
			iff.insertForm (srcFilter->getTag ());
				srcFilter->save (iff);
			iff.exitForm ();
			iff.goToTopOfForm ();

			//-- create filter
			TerrainGenerator::Filter* newFilter = TerrainGeneratorLoader::createFilter (iff, doc->getTerrainGenerator ());

			if (newFilter)
			{
				//-- add the filter to the dstLayer
				dstLayer->addFilter (newFilter);

				//-- add filter to the tree
				HTREEITEM selection = addFilterToTree (dstParent, newFilter);
				IGNORE_RETURN (GetTreeCtrl ().SelectItem (selection));
			}
		}
		break;

	case TerrainEditorDoc::Item::T_affector:
		{
			//-- get the source layer
			TerrainGenerator::Affector* srcAffector = safe_cast<TerrainGenerator::Affector*> (srcItem->layerItem);

			//-- save affector
			Iff iff(10000);
			iff.allowNonlinearFunctions ();
			iff.insertForm (srcAffector->getTag ());
				srcAffector->save (iff);
			iff.exitForm ();
			iff.goToTopOfForm ();

			//-- create affector
			TerrainGenerator::Affector* newAffector = TerrainGeneratorLoader::createAffector (iff, doc->getTerrainGenerator ());

			if (newAffector)
			{
				//-- add the affector to the dstLayer
				dstLayer->addAffector (newAffector);

				//-- add affector to the tree
				HTREEITEM selection = addAffectorToTree (dstParent, newAffector);
				IGNORE_RETURN (GetTreeCtrl ().SelectItem (selection));
			}
		}
		break;
	}

	doc->UpdateAllViews (0);
	doc->SetModifiedFlag ();
}

//-------------------------------------------------------------------

void LayerView::OnLButtonUpForMove ()
{
	m_dragMode = DM_nothing;

	IGNORE_RETURN (CImageList::DragLeave (this));
	CImageList::EndDrag ();

	ReleaseCapture();

	delete m_pDragImage;

	IGNORE_RETURN (GetTreeCtrl ().SelectDropTarget (NULL));
	m_htiOldDrop = NULL;

	//-- exit out if no target was selected to drop on
	if (m_htiDrop == NULL)
		return;

	if (m_htiDrag == m_htiDrop)
		return;

	if (m_idTimer)
	{
		IGNORE_RETURN (KillTimer (static_cast<int> (m_idTimer)));
		m_idTimer = 0;
	}

	//-- is the source's parent the drop target?
	if (GetTreeCtrl ().GetParentItem (m_htiDrag) == m_htiDrop)
		return;

	//-- don't allow drags of parent layers onto sublayers
	{
		HTREEITEM possibleParent = m_htiDrop;

		do
		{
			possibleParent = GetTreeCtrl ().GetParentItem (possibleParent);

			if (possibleParent == m_htiDrag)
				return;
		}
		while (possibleParent != 0);
	}

	//-- get the destination item
	TerrainEditorDoc::Item*  dstItem   = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (m_htiDrop));
	HTREEITEM                dstParent = 0;
	TerrainGenerator::Layer* dstLayer  = 0;

	//-- do the source and dest have the same parent? and is the destination NOT a layer?
	if (GetTreeCtrl ().GetParentItem (m_htiDrag) && GetTreeCtrl ().GetParentItem (m_htiDrag) == GetTreeCtrl ().GetParentItem (m_htiDrop) && dstItem->type != TerrainEditorDoc::Item::T_layer)
		return;

	//-- find the destination layer
	switch (dstItem->type)
	{
	case TerrainEditorDoc::Item::T_layer:
		{
			dstParent = m_htiDrop;
			dstLayer  = safe_cast<TerrainGenerator::Layer*> (dstItem->layerItem);
		}
		break;

	case TerrainEditorDoc::Item::T_boundary:
	case TerrainEditorDoc::Item::T_filter:
	case TerrainEditorDoc::Item::T_affector:
		{
			//-- get the parent destination item
			dstParent = GetTreeCtrl ().GetParentItem (m_htiDrop);

			//-- get the item data
			TerrainEditorDoc::Item* dstParentItem = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (dstParent));
			FATAL (dstParentItem->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

			//-- get the destination layer
			dstLayer = safe_cast<TerrainGenerator::Layer*> (dstParentItem->layerItem);
		}
		break;
	}

	NOT_NULL (dstLayer);

	//-- get the source item
	TerrainEditorDoc::Item* srcItem   = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (m_htiDrag));

	//-- determine what can be moved to what
	switch (srcItem->type)
	{
	case TerrainEditorDoc::Item::T_layer:
		{
			//-- get the parent
			HTREEITEM srcParent = GetTreeCtrl ().GetParentItem (m_htiDrag);

			//-- get the source layer
			TerrainGenerator::Layer* srcLayer = safe_cast<TerrainGenerator::Layer*> (srcItem->layerItem);

			if (!srcParent)
			{
				//-- source item is being moved from the generator
				TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
				NOT_NULL (doc);

				TerrainGenerator* terrainGenerator = doc->getTerrainGenerator ();
				NOT_NULL (terrainGenerator);

				terrainGenerator->removeLayer (srcLayer, false);

				//-- add layer to destination layer
				dstLayer->addLayer (srcLayer);

				//-- remove layer from tree
				saveExpandedState (m_htiDrag);
				IGNORE_RETURN (GetTreeCtrl ().DeleteItem (m_htiDrag));

				//-- add layer to the tree
				HTREEITEM selection = addLayerToTree (dstParent, 0, srcLayer);
				IGNORE_RETURN (GetTreeCtrl ().SelectItem (selection));
			}
			else
			{
				//-- source item is being moved from a layer
				TerrainEditorDoc::Item* srcParentItem = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (srcParent));
				FATAL (srcParentItem->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

				//-- remove src item from parent
				TerrainGenerator::Layer* srcParentLayer = safe_cast<TerrainGenerator::Layer*> (srcParentItem->layerItem);
				srcParentLayer->removeLayer (srcLayer, false);

				//-- remove layer from tree
				saveExpandedState (m_htiDrag);
				IGNORE_RETURN (GetTreeCtrl ().DeleteItem (m_htiDrag));

				//-- add layer to destination layer
				dstLayer->addLayer (srcLayer);

				//-- add layer to the tree
				HTREEITEM selection = addLayerToTree (dstParent, 0, srcLayer);
				IGNORE_RETURN (GetTreeCtrl ().SelectItem (selection));
			}
		}
		break;

	case TerrainEditorDoc::Item::T_boundary:
	case TerrainEditorDoc::Item::T_filter:
	case TerrainEditorDoc::Item::T_affector:
		{
			//-- complete operation
			switch (srcItem->type)
			{
			case TerrainEditorDoc::Item::T_boundary:
				{
					//-- get the source item
					TerrainGenerator::Boundary* boundary = safe_cast<TerrainGenerator::Boundary*> (srcItem->layerItem);

					//-- get the source layer
					HTREEITEM srcParent = GetTreeCtrl ().GetParentItem (m_htiDrag);

					//-- get the item data
					TerrainEditorDoc::Item* srcParentItem = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (srcParent));
					FATAL (srcParentItem->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

					TerrainGenerator::Layer* srcLayer = safe_cast<TerrainGenerator::Layer*> (srcParentItem->layerItem);

					//-- remove src item from parent
					srcLayer->removeBoundary (boundary, false);

					//-- remove item from tree
					saveExpandedState (m_htiDrag);
					IGNORE_RETURN (GetTreeCtrl ().DeleteItem (m_htiDrag));

					//-- add layer to destination layer
					dstLayer->addBoundary (boundary);

					//-- add boundary to the tree
					HTREEITEM selection = addBoundaryToTree (dstParent, boundary);
					IGNORE_RETURN (GetTreeCtrl ().SelectItem (selection));
				}
				break;

			case TerrainEditorDoc::Item::T_filter:
				{
					//-- get the source item
					TerrainGenerator::Filter* filter = safe_cast<TerrainGenerator::Filter*> (srcItem->layerItem);

					//-- get the source layer
					HTREEITEM srcParent = GetTreeCtrl ().GetParentItem (m_htiDrag);

					//-- get the item data
					TerrainEditorDoc::Item* srcParentItem = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (srcParent));
					FATAL (srcParentItem->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

					TerrainGenerator::Layer* srcLayer = safe_cast<TerrainGenerator::Layer*> (srcParentItem->layerItem);

					//-- remove src item from parent
					srcLayer->removeFilter (filter, false);

					//-- remove item from tree
					saveExpandedState (m_htiDrag);
					IGNORE_RETURN (GetTreeCtrl ().DeleteItem (m_htiDrag));

					//-- add layer to destination layer
					dstLayer->addFilter (filter);

					//-- add filter to the tree
					HTREEITEM selection = addFilterToTree (dstParent, filter);
					IGNORE_RETURN (GetTreeCtrl ().SelectItem (selection));
				}
				break;

			case TerrainEditorDoc::Item::T_affector:
				{
					//-- get the source item
					TerrainGenerator::Affector* affector = safe_cast<TerrainGenerator::Affector*> (srcItem->layerItem);

					//-- get the source layer
					HTREEITEM srcParent = GetTreeCtrl ().GetParentItem (m_htiDrag);

					//-- get the item data
					TerrainEditorDoc::Item* srcParentItem = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (srcParent));
					FATAL (srcParentItem->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

					TerrainGenerator::Layer* srcLayer = safe_cast<TerrainGenerator::Layer*> (srcParentItem->layerItem);

					//-- remove src item from parent
					srcLayer->removeAffector (affector, false);

					//-- remove item from tree
					saveExpandedState (m_htiDrag);
					IGNORE_RETURN (GetTreeCtrl ().DeleteItem (m_htiDrag));

					//-- add layer to destination layer
					dstLayer->addAffector (affector);

					//-- add affector to the tree
					HTREEITEM selection = addAffectorToTree (dstParent, affector);
					IGNORE_RETURN (GetTreeCtrl ().SelectItem (selection));
				}
				break;
			}
		}
		break;
	}

	GetDocument ()->UpdateAllViews (0);
	GetDocument ()->SetModifiedFlag ();
}

//-------------------------------------------------------------------

void LayerView::OnMouseMove(UINT nFlags, CPoint point)
{
	CTreeView::OnMouseMove(nFlags, point);

	HTREEITEM hti;
	UINT      flags;

	if (m_dragMode == DM_copy)
		SetCursor (copyCursor);

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
}

//-------------------------------------------------------------------

void LayerView::OnDestroy()
{
	if( m_idTimer )
	{
		IGNORE_RETURN (KillTimer (static_cast<int> (m_idTimer)));
		m_idTimer = 0;
	}

	CTreeView::OnDestroy();
}

//-------------------------------------------------------------------

void LayerView::OnTimer(UINT nIDEvent)
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

void LayerView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (isLayerSelected () || isBoundarySelected () || isFilterSelected () || isAffectorSelected ())
	{
		//-- convert client coordinates to screen coordinates
		CPoint pt = point;
		ClientToScreen (&pt);

		CMenu menu;
		IGNORE_RETURN (menu.LoadMenu (IDR_LAYERVIEW_MENU));

		CMenu* subMenu = menu.GetSubMenu (0);
		IGNORE_RETURN (subMenu->TrackPopupMenu (TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, this));
	}

	CTreeView::OnRButtonDown(nFlags, point);
}

//-------------------------------------------------------------------

void LayerView::ExportAffector ()
{
	const HTREEITEM                   treeItem = getSelectedAffector ();
	const TerrainEditorDoc::Item*     item     = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (treeItem));
	const TerrainGenerator::Affector* affector = dynamic_cast<TerrainGenerator::Affector*> (item->layerItem);
	if (affector)
	{
		CFileDialog dlg (false, "*.aff", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR), "Affector Files *.aff|*.aff||");
		dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("Affector");
		dlg.m_ofn.lpstrDefExt     = "aff";
		dlg.m_ofn.lpstrTitle      = "Exporting Affector";
		if (dlg.DoModal()==IDOK)
		{
			//-- save affector
			Iff iff(10000);

			const TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
			NOT_NULL (doc);

			const TerrainGenerator* terrainGenerator = doc->getTerrainGenerator ();
			NOT_NULL (terrainGenerator);

			TerrainGeneratorHelper::saveShaderGroup (iff, terrainGenerator->getShaderGroup (), affector);
			TerrainGeneratorHelper::saveFloraGroup  (iff, terrainGenerator->getFloraGroup (),  affector);
			TerrainGeneratorHelper::saveRadialGroup (iff, terrainGenerator->getRadialGroup (), affector);
			TerrainGeneratorHelper::saveEnvironmentGroup (iff, terrainGenerator->getEnvironmentGroup (), affector);
			TerrainGeneratorHelper::saveFractalGroup (iff, terrainGenerator->getFractalGroup (), affector);

			iff.insertForm (affector->getTag());
				affector->save (iff);
			iff.exitForm ();
			if (!iff.write (dlg.GetPathName ()))
				IGNORE_RETURN (MessageBox ("Could not write file"));

			IGNORE_RETURN (RecentDirectory::update ("Affector", dlg.GetPathName ()));
		}
	}
	else
	{
		DEBUG_FATAL((true), ("Wrong data extracted from layer tree view."));
	}
}

//-------------------------------------------------------------------

void LayerView::ExportFilter ()
{
	const HTREEITEM                   treeItem = getSelectedFilter ();
	const TerrainEditorDoc::Item*     item     = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (treeItem));
	const TerrainGenerator::Filter*   filter   = dynamic_cast<TerrainGenerator::Filter*> (item->layerItem);
	if (filter)
	{
		CFileDialog dlg (false, "*.fil", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR), "Filter Files *.fil|*.fil||");
		dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("Filter");
		dlg.m_ofn.lpstrDefExt     = "fil";
		dlg.m_ofn.lpstrTitle      = "Exporting Filter";
		if (dlg.DoModal()==IDOK)
		{
			//-- save filter
			Iff iff(10000);

			const TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
			NOT_NULL (doc);

			const TerrainGenerator* terrainGenerator = doc->getTerrainGenerator ();
			NOT_NULL (terrainGenerator);

			TerrainGeneratorHelper::saveShaderGroup (iff, terrainGenerator->getShaderGroup (), filter);
			TerrainGeneratorHelper::saveFloraGroup  (iff, terrainGenerator->getFloraGroup (),  filter);
			TerrainGeneratorHelper::saveRadialGroup (iff, terrainGenerator->getRadialGroup (), filter);
			TerrainGeneratorHelper::saveEnvironmentGroup (iff, terrainGenerator->getEnvironmentGroup (), filter);
			TerrainGeneratorHelper::saveFractalGroup (iff, terrainGenerator->getFractalGroup (), filter);

			iff.insertForm (filter->getTag());
				filter->save (iff);
			iff.exitForm ();
			if (!iff.write (dlg.GetPathName ()))
				IGNORE_RETURN (MessageBox ("Could not write file"));

			IGNORE_RETURN (RecentDirectory::update ("Filter", dlg.GetPathName ()));
		}
	}
	else
	{
		DEBUG_FATAL((true), ("Wrong data extracted from layer tree view."));
	}
}

//-------------------------------------------------------------------

void LayerView::ExportBoundary ()
{
	const HTREEITEM                   treeItem = getSelectedBoundary ();
	const TerrainEditorDoc::Item*     item     = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (treeItem));
	const TerrainGenerator::Boundary* boundary = dynamic_cast<TerrainGenerator::Boundary*> (item->layerItem);
	if (boundary)
	{
		CFileDialog dlg (false, "*.bnd", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR), "Boundary Files *.bnd|*.bnd||");
		dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("Boundary");
		dlg.m_ofn.lpstrDefExt     = "bnd";
		dlg.m_ofn.lpstrTitle      = "Exporting Boundary";
		if (dlg.DoModal()==IDOK)
		{
			//-- save boundary
			Iff iff(10000);

			const TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
			NOT_NULL (doc);

			const TerrainGenerator* terrainGenerator = doc->getTerrainGenerator ();
			NOT_NULL (terrainGenerator);

			iff.insertForm (boundary->getTag());
				boundary->save (iff);
			iff.exitForm ();
			if (!iff.write (dlg.GetPathName ()))
				IGNORE_RETURN (MessageBox ("Could not write file"));

			IGNORE_RETURN (RecentDirectory::update ("Boundary", dlg.GetPathName ()));
		}
	}
	else
	{
		DEBUG_FATAL((true), ("Wrong data extracted from layer tree view."));
	}
}

//-------------------------------------------------------------------

void LayerView::ExportLayer ()
{
	const HTREEITEM                   treeItem = getSelectedLayer ();
	const TerrainEditorDoc::Item*     item     = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (treeItem));
	const TerrainGenerator::Layer*    layer    = dynamic_cast<TerrainGenerator::Layer*> (item->layerItem);
	if (layer)
	{
		CFileDialog dlg (false, "*.lay", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR), "Layer Files *.lay|*.lay||");
		dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("Layer");
		dlg.m_ofn.lpstrDefExt     = "lay";
		dlg.m_ofn.lpstrTitle      = "Exporting Layer";
		if (dlg.DoModal()==IDOK)
		{
			//-- save layer
			Iff iff(10000);

			const TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
			NOT_NULL (doc);

			const TerrainGenerator* terrainGenerator = doc->getTerrainGenerator ();
			NOT_NULL (terrainGenerator);

			TerrainGeneratorHelper::saveShaderGroup (iff, terrainGenerator->getShaderGroup (), layer);
			TerrainGeneratorHelper::saveFloraGroup  (iff, terrainGenerator->getFloraGroup (),  layer);
			TerrainGeneratorHelper::saveRadialGroup (iff, terrainGenerator->getRadialGroup (), layer);
			TerrainGeneratorHelper::saveEnvironmentGroup (iff, terrainGenerator->getEnvironmentGroup (), layer);
			TerrainGeneratorHelper::saveFractalGroup (iff, terrainGenerator->getFractalGroup (), layer);

			layer->save (iff);
			if (!iff.write (dlg.GetPathName ()))
				IGNORE_RETURN (MessageBox ("Could not write file"));

			IGNORE_RETURN (RecentDirectory::update ("Layer", dlg.GetPathName ()));
		}
	}
	else
	{
		DEBUG_FATAL((true), ("Wrong data extracted from layer tree view."));
	}
}

//-------------------------------------------------------------------

void LayerView::OnExportLayeritem()
{
	if (!verify ())
		return;

	if (isAffectorSelected())
	{
		ExportAffector ();
	}
	else if (isFilterSelected())
	{
		ExportFilter ();
	}
	else if (isBoundarySelected())
	{
		ExportBoundary ();
	}
	else if (isLayerSelected())
	{
		ExportLayer ();
	}
}

//-------------------------------------------------------------------

void LayerView::OnUpdateExportLayeritem(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (true);
}

//-------------------------------------------------------------------

void LayerView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREF (pNMHDR);

	const HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	if (selection)
	{
		TerrainEditorDoc* doc  = safe_cast<TerrainEditorDoc*> (GetDocument ());
		UNREF (doc);

		//-- get the item data
		TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));

		//--
		switch (item->type)
		{
		case TerrainEditorDoc::Item::T_boundary:
			{
				const TerrainGenerator::Boundary* boundary = safe_cast<const TerrainGenerator::Boundary*> (item->layerItem);

				if (doc->getMapFrame ())
					doc->getMapFrame ()->recenter (boundary->getCenter ());
			}
			break;

		default:
			break;
		}
	}

	*pResult = 0;
}

//-------------------------------------------------------------------

void LayerView::OnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnSelchanged (pNMHDR, pResult);

	*pResult = 0;
}

//-------------------------------------------------------------------

void LayerView::OnUpdateInsertDelete(CCmdUI* pCmdUI)
{
	pCmdUI->Enable ((GetTreeCtrl ().GetSelectedItem () != 0) ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void LayerView::OnInsertDelete()
{
	OnButtonDeleteLayer ();
}

//-------------------------------------------------------------------

void LayerView::OnUpdateInsertRename(CCmdUI* pCmdUI)
{
	pCmdUI->Enable ((GetTreeCtrl ().GetSelectedItem () != 0) ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void LayerView::OnInsertRename()
{
	IGNORE_RETURN (GetTreeCtrl ().EditLabel (GetTreeCtrl ().GetSelectedItem ()));
}

//-------------------------------------------------------------------

void LayerView::OnButtonNewrootlayer()
{
	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	TerrainGenerator* terrainGenerator = doc->getTerrainGenerator ();
	NOT_NULL (terrainGenerator);

	//-- create a new layer
	TerrainGenerator::Layer* layer = new TerrainGenerator::Layer ();
	layer->setName ("Layer");

	//-- add layer to generator
	terrainGenerator->addLayer (layer);

	//-- add new layer to tree
	HTREEITEM layerItem = addLayerToTree (TVI_ROOT, 0, layer);
	IGNORE_RETURN (GetTreeCtrl ().SelectItem (layerItem));
}

//-------------------------------------------------------------------

void LayerView::OnButtonNewsublayer()
{
	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	TerrainGenerator* terrainGenerator = doc->getTerrainGenerator ();
	NOT_NULL (terrainGenerator);

	if (terrainGenerator->getNumberOfLayers () == 0)
		OnButtonNewrootlayer ();

	//-- get the currently selected item
	HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	//-- if it's not a layer
	if (!isLayer (selection))
	{
		//-- it must be a child so get its parent and its parent must be a layer
		selection = GetTreeCtrl ().GetParentItem (selection);

		FATAL (!isLayer (selection), ("selected item is not a layer"));
	}

	//-- get the layer from the item
	TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
	FATAL (item->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

	//-- add layer to selected layer
	TerrainGenerator::Layer* layer = safe_cast<TerrainGenerator::Layer*> (item->layerItem);

	//-- create a new layer
	TerrainGenerator::Layer* newLayer = new TerrainGenerator::Layer ();
	newLayer->setName ("Layer");
	layer->addLayer (newLayer);

	//-- add layer to tree
	HTREEITEM layerItem = addLayerToTree (selection, 0, newLayer);
	IGNORE_RETURN (GetTreeCtrl ().SelectItem (layerItem));

	//-- tell document to update views
	doc->UpdateAllViews (0);
	doc->SetModifiedFlag ();
}

//-------------------------------------------------------------------

void LayerView::OnButtonPushroot()
{
	//-- get the currently selected item
	HTREEITEM selection = getSelectedLayer ();

	HTREEITEM parent = GetTreeCtrl ().GetParentItem (selection);

	if (!parent)
		return;

	//-- extract the Item
	TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
	FATAL (item->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

	TerrainEditorDoc::Item* parentItem = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (parent));
	FATAL (parentItem->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

	//-- get the layers
	TerrainGenerator::Layer* srcLayer = safe_cast<TerrainGenerator::Layer*> (item->layerItem);

	TerrainGenerator::Layer* parentLayer = safe_cast<TerrainGenerator::Layer*> (parentItem->layerItem);

	//-- remove the layer
	parentLayer->removeLayer (srcLayer, false);

	//-- add it to the generator
	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	TerrainGenerator* terrainGenerator = doc->getTerrainGenerator ();
	NOT_NULL (terrainGenerator);

	terrainGenerator->addLayer (srcLayer);

	//-- remove it from the tree
	saveExpandedState (selection);
	IGNORE_RETURN (GetTreeCtrl ().DeleteItem (selection));

	//-- add layer to tree
	HTREEITEM layerItem = addLayerToTree (TVI_ROOT, 0, srcLayer);
	IGNORE_RETURN (GetTreeCtrl ().SelectItem (layerItem));

	//-- tell document to update views
	doc->UpdateAllViews (0);
	doc->SetModifiedFlag ();
}

//-------------------------------------------------------------------

void LayerView::OnUpdateButtonPushroot(CCmdUI* pCmdUI)
{
	pCmdUI->Enable ((isLayerSelected () && GetTreeCtrl ().GetParentItem (getSelectedLayer ()) != 0) ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void LayerView::OnButtonPushup()
{
/*
	//-- get the currently selected item
	HTREEITEM selection = getSelectedLayer ();

	HTREEITEM parent = GetTreeCtrl ().GetParentItem (selection);

	if (!parent)
		return;

	//-- extract the Item
	TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
	FATAL (item->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

	TerrainEditorDoc::Item* parentItem = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (parent));
	FATAL (parentItem->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

	//-- get the layers
	TerrainGenerator::Layer* srcLayer = safe_cast<TerrainGenerator::Layer*> (item->layerItem);

	TerrainGenerator::Layer* parentLayer = safe_cast<TerrainGenerator::Layer*> (parentItem->layerItem);

	//-- remove the layer
	parentLayer->removeLayer (srcLayer, false);

	//-- add it to the generator
	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	TerrainGenerator* terrainGenerator = doc->getTerrainGenerator ();
	NOT_NULL (terrainGenerator);

	terrainGenerator->addLayer (srcLayer);

	//-- remove it from the tree
	saveExpandedState (selection);
	IGNORE_RETURN (GetTreeCtrl ().DeleteItem (selection));

	//-- add layer to tree
	HTREEITEM layerItem = addLayerToTree (TVI_ROOT, 0, srcLayer);
	IGNORE_RETURN (GetTreeCtrl ().SelectItem (layerItem));

	//-- tell document to update views
	doc->UpdateAllViews (0);
	doc->SetModifiedFlag ();
*/
}

//-------------------------------------------------------------------

void LayerView::OnUpdateButtonPushup(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (false);
//	pCmdUI->Enable (isLayerSelected () && GetTreeCtrl ().GetParentItem (getSelectedLayer ()) != 0);
}

//-------------------------------------------------------------------

void LayerView::OnInsertImportAffector()
{
	// get filename
	CFileDialog dlg (true, "*.aff", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR), "Affector Files *.aff|*.aff||");
	dlg.m_ofn.lpstrDefExt     = "aff";
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("Affector");
	dlg.m_ofn.lpstrTitle      = "Importing Affector";

	if (dlg.DoModal () == IDOK)
	{
		Iff iff;
		if (iff.open (dlg.GetPathName ()))
		{
			IGNORE_RETURN (RecentDirectory::update ("Affector", dlg.GetPathName ()));

			//-- add it to the generator
			TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
			NOT_NULL (doc);

			ShaderGroup* sg = TerrainGeneratorHelper::loadShaderGroup (iff);
			NOT_NULL (sg);

			FloraGroup*  fg = TerrainGeneratorHelper::loadFloraGroup (iff);
			NOT_NULL (fg);

			RadialGroup* rg = TerrainGeneratorHelper::loadRadialGroup (iff);
			NOT_NULL (rg);

			EnvironmentGroup* eg = TerrainGeneratorHelper::loadEnvironmentGroup (iff);
			NOT_NULL (eg);

			FractalGroup* mg = TerrainGeneratorHelper::loadFractalGroup (iff);
			NOT_NULL (mg);

			TerrainGenerator::Affector* affector = TerrainGeneratorLoader::createAffector (iff, doc->getTerrainGenerator ());

			//--
			TerrainGeneratorHelper::fixup (
				doc,
				doc->getTerrainGenerator ()->getShaderGroup (),
				doc->getTerrainGenerator ()->getFloraGroup (),
				doc->getTerrainGenerator ()->getRadialGroup (),
				doc->getTerrainGenerator ()->getEnvironmentGroup (),
				doc->getTerrainGenerator ()->getFractalGroup (),
				*sg,
				*fg,
				*rg,
				*eg,
				*mg,
				affector);

			delete sg;
			delete fg;
			delete rg;
			delete eg;
			delete mg;

			//-- get the currently selected item
			HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

			//-- if it's not a layer
			if (!isLayer (selection))
			{
				//-- it must be a child so get its parent and its parent must be a layer
				selection = GetTreeCtrl ().GetParentItem (selection);

				FATAL (!isLayer (selection), ("selected item is not a layer"));
			}

			//-- get the layer from the item
			TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
			FATAL (item->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

			//-- add layer to selected layer
			TerrainGenerator::Layer* layer = safe_cast<TerrainGenerator::Layer*> (item->layerItem);

			layer->addAffector (affector);

			selection = addAffectorToTree (selection, affector);

			GetDocument ()->UpdateAllViews (0);
			GetDocument ()->SetModifiedFlag ();
		}
	}
}

//-------------------------------------------------------------------

void LayerView::OnInsertImportBoundary()
{
	// get filename
	CFileDialog dlg (true, "*.bnd", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR), "Boundary Files *.bnd|*.bnd||");
	dlg.m_ofn.lpstrDefExt     = "bnd";
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("Boundary");
	dlg.m_ofn.lpstrTitle      = "Importing Boundary";

	if (dlg.DoModal()==IDOK)
	{
		Iff iff;
		if (iff.open (dlg.GetPathName ()))
		{
			IGNORE_RETURN (RecentDirectory::update ("Boundary", dlg.GetPathName ()));

			TerrainGenerator::Boundary* boundary = TerrainGeneratorLoader::createBoundary (iff);

			//-- get the currently selected item
			HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

			//-- if it's not a layer
			if (!isLayer (selection))
			{
				//-- it must be a child so get its parent and its parent must be a layer
				selection = GetTreeCtrl ().GetParentItem (selection);

				FATAL (!isLayer (selection), ("selected item is not a layer"));
			}

			//-- get the layer from the item
			TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
			FATAL (item->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

			//-- add layer to selected layer
			TerrainGenerator::Layer* layer = safe_cast<TerrainGenerator::Layer*> (item->layerItem);

			layer->addBoundary (boundary);

			selection = addBoundaryToTree (selection, boundary);

			GetDocument ()->UpdateAllViews (0);
			GetDocument ()->SetModifiedFlag ();
		}
	}
}

//-------------------------------------------------------------------

void LayerView::OnInsertImportFilter()
{
	// get filename
	CFileDialog dlg (true, "*.fil", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR), "Filter Files *.fil|*.fil||");
	dlg.m_ofn.lpstrDefExt     = "fil";
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("Filter");
	dlg.m_ofn.lpstrTitle      = "Importing Filter";

	if (dlg.DoModal()==IDOK)
	{
		Iff iff;
		if (iff.open (dlg.GetPathName ()))
		{
			IGNORE_RETURN (RecentDirectory::update ("Filter", dlg.GetPathName ()));

			//-- add it to the generator
			TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
			NOT_NULL (doc);

			ShaderGroup* sg = TerrainGeneratorHelper::loadShaderGroup (iff);
			NOT_NULL (sg);

			FloraGroup*  fg = TerrainGeneratorHelper::loadFloraGroup (iff);
			NOT_NULL (fg);

			RadialGroup* rg = TerrainGeneratorHelper::loadRadialGroup (iff);
			NOT_NULL (rg);

			EnvironmentGroup* eg = TerrainGeneratorHelper::loadEnvironmentGroup (iff);
			NOT_NULL (eg);

			FractalGroup* mg = TerrainGeneratorHelper::loadFractalGroup (iff);
			NOT_NULL (mg);

			TerrainGenerator::Filter* filter = TerrainGeneratorLoader::createFilter (iff, doc->getTerrainGenerator ());

			//--
			TerrainGeneratorHelper::fixup (
				doc,
				doc->getTerrainGenerator ()->getShaderGroup (),
				doc->getTerrainGenerator ()->getFloraGroup (),
				doc->getTerrainGenerator ()->getRadialGroup (),
				doc->getTerrainGenerator ()->getEnvironmentGroup (),
				doc->getTerrainGenerator ()->getFractalGroup (),
				*sg,
				*fg,
				*rg,
				*eg,
				*mg,
				filter);

			delete sg;
			delete fg;
			delete rg;
			delete eg;
			delete mg;

			//-- get the currently selected item
			HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

			//-- if it's not a layer
			if (!isLayer (selection))
			{
				//-- it must be a child so get its parent and its parent must be a layer
				selection = GetTreeCtrl ().GetParentItem (selection);

				FATAL (!isLayer (selection), ("selected item is not a layer"));
			}

			//-- get the layer from the item
			TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
			FATAL (item->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

			//-- add layer to selected layer
			TerrainGenerator::Layer* layer = safe_cast<TerrainGenerator::Layer*> (item->layerItem);

			layer->addFilter (filter);

			selection = addFilterToTree (selection, filter);

			GetDocument ()->UpdateAllViews (0);
			GetDocument ()->SetModifiedFlag ();
		}
	}
}

//-------------------------------------------------------------------

void LayerView::OnInsertImportLayer()
{
	// get filename
	CFileDialog dlg (true, "*.lay", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR), "Layer Files *.lay|*.lay||");
	dlg.m_ofn.lpstrDefExt     = "lay";
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("Layer");
	dlg.m_ofn.lpstrTitle      = "Importing Layer";

	if (dlg.DoModal()==IDOK)
	{
		Iff iff;
		if (iff.open (dlg.GetPathName () ) )
		{
			IGNORE_RETURN (RecentDirectory::update ("Layer", dlg.GetPathName ()));

			//-- add it to the generator
			TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
			NOT_NULL (doc);

			ShaderGroup* sg = TerrainGeneratorHelper::loadShaderGroup (iff);
			NOT_NULL (sg);

			FloraGroup*  fg = TerrainGeneratorHelper::loadFloraGroup (iff);
			NOT_NULL (fg);

			RadialGroup* rg = TerrainGeneratorHelper::loadRadialGroup (iff);
			NOT_NULL (rg);

			EnvironmentGroup* eg = TerrainGeneratorHelper::loadEnvironmentGroup (iff);
			NOT_NULL (eg);

			FractalGroup* mg = TerrainGeneratorHelper::loadFractalGroup (iff);
			NOT_NULL (mg);

			TerrainGenerator::Layer* newLayer = new TerrainGenerator::Layer;
			newLayer->load (iff, doc->getTerrainGenerator ());

			//--
			TerrainGeneratorHelper::fixup (
				doc,
				doc->getTerrainGenerator ()->getShaderGroup (),
				doc->getTerrainGenerator ()->getFloraGroup (),
				doc->getTerrainGenerator ()->getRadialGroup (),
				doc->getTerrainGenerator ()->getEnvironmentGroup (),
				doc->getTerrainGenerator ()->getFractalGroup (),
				*sg,
				*fg,
				*rg,
				*eg,
				*mg,
				newLayer);

			delete sg;
			delete fg;
			delete rg;
			delete eg;
			delete mg;

			//-- get the currently selected item
			HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

			//-- if it's not a layer
			if (!isLayer (selection))
			{
				//-- it must be a child so get its parent and its parent must be a layer
				selection = GetTreeCtrl ().GetParentItem (selection);

				FATAL (!isLayer (selection), ("selected item is not a layer"));
			}

			//-- get the layer from the item
			TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
			FATAL (item->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

			//-- add layer to selected layer
			TerrainGenerator::Layer* layer = safe_cast<TerrainGenerator::Layer*> (item->layerItem);

			layer->addLayer (newLayer);

			selection = addLayerToTree (selection, 0, newLayer);

			GetDocument ()->UpdateAllViews (0);
			GetDocument ()->SetModifiedFlag ();
		}
	}
}

//-------------------------------------------------------------------

void LayerView::OnButtonScanLayers()
{
	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	//--
	CString tmp;
	tmp = "-----------------\r\n";
	CONSOLE_PRINT (tmp);

	//--
	ArrayList<TerrainGeneratorHelper::OutputData> output;

	TerrainGeneratorHelper::scan (doc->getTerrainGenerator (), output);

	if (output.getNumberOfElements ())
	{
		GetApp ()->showWarning (true);
		GetApp ()->showConsole ();
	}

	//--
	int i;
	for (i = 0; i < output.getNumberOfElements (); i++)
	{
		switch (output [i].mode)
		{
		case TerrainGeneratorHelper::OutputData::M_warning:
			{
				if (doc->getWarningFrame ())
					doc->getWarningFrame ()->update (output [i]);
			}
			break;

		case TerrainGeneratorHelper::OutputData::M_console:
			{
				CONSOLE_PRINT (*output [i].message);
				CONSOLE_PRINT ("\r\n");
			}
			break;
		}

		delete output [i].message;
	}
}

//-------------------------------------------------------------------

void LayerView::refreshIcons (HTREEITEM selection)
{
	//-- refresh me
	const TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
	const LayerViewIconType iconType   = getIcon (item->layerItem);

	IGNORE_RETURN (GetTreeCtrl ().SetItemImage (selection, iconType, iconType));

	//-- refresh the children
	HTREEITEM child = GetTreeCtrl ().GetChildItem (selection);

	if (child)
	{
		do
		{
			refreshIcons (child);

			child = GetTreeCtrl ().GetNextSiblingItem (child);
		}
		while (child);
	}
}

//-------------------------------------------------------------------

void LayerView::refreshIcons (void)
{
	HTREEITEM item = GetTreeCtrl ().GetRootItem ();

	if (item)
	{
		do
		{
			refreshIcons (item);

			item = GetTreeCtrl ().GetNextSiblingItem (item);
		}
		while (item);
	}
}

//-------------------------------------------------------------------

void LayerView::OnInsertScanAll()
{
	OnButtonScanLayers ();
}

//-------------------------------------------------------------------

void LayerView::OnInsertScanLayer()
{
	//-- get the currently selected item
	HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	//-- if it's not a layer
	if (!isLayer (selection))
	{
		//-- it must be a child so get its parent and its parent must be a layer
		selection = GetTreeCtrl ().GetParentItem (selection);

		FATAL (!isLayer (selection), ("selected item is not a layer"));
	}

	//-- get the layer from the item
	TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));
	FATAL (item->type != TerrainEditorDoc::Item::T_layer, ("call to wrong function"));

	//-- add layer to selected layer
	TerrainGenerator::Layer* layer = safe_cast<TerrainGenerator::Layer*> (item->layerItem);

	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	//--
	CString tmp;
	tmp = "-----------------\r\n";
	CONSOLE_PRINT (tmp);

	//--
	ArrayList<TerrainGeneratorHelper::OutputData> output;

	TerrainGeneratorHelper::scanLayer (doc->getTerrainGenerator (), layer, output);

	if (output.getNumberOfElements ())
	{
		GetApp ()->showWarning (true);
		GetApp ()->showConsole ();
	}

	//--
	int i;
	for (i = 0; i < output.getNumberOfElements (); i++)
	{
		switch (output [i].mode)
		{
		case TerrainGeneratorHelper::OutputData::M_warning:
			{
				if (doc->getWarningFrame ())
					doc->getWarningFrame ()->update (output [i]);
			}
			break;

		case TerrainGeneratorHelper::OutputData::M_console:
			{
				CONSOLE_PRINT (*output [i].message);
				CONSOLE_PRINT ("\r\n");
			}
			break;
		}

		delete output [i].message;
	}
}

//-------------------------------------------------------------------

void LayerView::OnUpdateInsertScanLayer(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (isLayerSelected ());
}

//-------------------------------------------------------------------

void LayerView::OnButtonShowProfile()
{
	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	//-- update profile
	{
		GetApp ()->showProfile ();

		if (doc->getProfileFrame ())
			doc->getProfileFrame ()->update (doc->getTerrainGenerator ());
	}

	//-- update console
	{
		CString tmp;
		tmp = "-----------------\r\n";
		CONSOLE_PRINT (tmp);

		//--
		ArrayList<TerrainGeneratorHelper::OutputData> output;

		TerrainGeneratorHelper::profile (doc->getTerrainGenerator (), doc->getLastTotalChunkGenerationTime (), doc->getLastAverageChunkGenerationTime (), doc->getLastMinimumChunkGenerationTime (), doc->getLastMaximumChunkGenerationTime (), output);

		//--
		int i;
		for (i = 0; i < output.getNumberOfElements (); i++)
		{
			switch (output [i].mode)
			{
			case TerrainGeneratorHelper::OutputData::M_warning:
				{
				}
				break;

			case TerrainGeneratorHelper::OutputData::M_console:
				{
					CONSOLE_PRINT (*output [i].message);
					CONSOLE_PRINT ("\r\n");
				}
				break;
			}

			delete output [i].message;
		}
	}

	{
		GetApp ()->showConsole ();
	}
}

//-------------------------------------------------------------------

HTREEITEM LayerView::find (HTREEITEM hItem, const TerrainGenerator::LayerItem* layerItem)
{
	HTREEITEM theItem = NULL;

	if (hItem == NULL)
		return NULL;

	if ((theItem = GetTreeCtrl ().GetNextSiblingItem (hItem)) != NULL)
	{
		theItem = find (theItem, layerItem);

		if (theItem != NULL)
			return theItem;
	}

	if ((theItem = GetTreeCtrl ().GetChildItem (hItem)) != NULL)
	{
		theItem = find (theItem, layerItem);

		if (theItem != NULL)
			return theItem;
	}

	const TerrainEditorDoc::Item* item = reinterpret_cast<const TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (hItem));

	if (item->layerItem == layerItem)
		return hItem;

	return theItem;
}

//-------------------------------------------------------------------

void LayerView::selectLayerItem (const TerrainGenerator::LayerItem* layerItem)
{
	//-- find htreeitem accociated with layer
	HTREEITEM selection = find (GetTreeCtrl ().GetRootItem (), layerItem);

	if (selection)
	{
		IGNORE_RETURN (GetTreeCtrl ().Select (selection, TVGN_FIRSTVISIBLE));
		IGNORE_RETURN (GetTreeCtrl ().SelectItem (selection));
	}
}

//-------------------------------------------------------------------

void LayerView::OnInsertScanDump()
{
	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	//-- update console
	{
		GetApp ()->showConsole ();

		CString tmp;
		tmp = "-----------------\r\n";
		CONSOLE_PRINT (tmp);

		//--
		ArrayList<TerrainGeneratorHelper::OutputData> output;

		const HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();
		const TerrainEditorDoc::Item* const item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (selection));

		TerrainGeneratorHelper::dump (doc->getTerrainGenerator (), item->layerItem, output);

		//--
		int i;
		for (i = 0; i < output.getNumberOfElements (); i++)
		{
			switch (output [i].mode)
			{
			case TerrainGeneratorHelper::OutputData::M_warning:
				{
				}
				break;

			case TerrainGeneratorHelper::OutputData::M_console:
				{
					CONSOLE_PRINT (*output [i].message);
					CONSOLE_PRINT ("\r\n");
				}
				break;
			}

			delete output [i].message;
		}
	}
}

//-------------------------------------------------------------------

static void ExpandBranch (CTreeCtrl& tree, HTREEITEM hti)
{
	if (hti && tree.ItemHasChildren (hti))
	{
		IGNORE_RETURN (tree.Expand (hti, TVE_EXPAND));
		hti = tree.GetChildItem (hti);

		do
		{
			ExpandBranch (tree, hti);
		}
		while ((hti = tree.GetNextSiblingItem (hti)) != 0);
	}

	IGNORE_RETURN (tree.EnsureVisible (tree.GetSelectedItem ()));
}

//-------------------------------------------------------------------

void LayerView::OnInsertExpandall()
{
    HTREEITEM hti = GetTreeCtrl ().GetRootItem ();

    do
	{
		ExpandBranch (GetTreeCtrl (), hti);
	}
	while ((hti = GetTreeCtrl ().GetNextSiblingItem (hti)) != 0);
}

//-------------------------------------------------------------------

void LayerView::OnInsertExpandbranch()
{
	ExpandBranch (GetTreeCtrl (), GetTreeCtrl ().GetSelectedItem ());
}

//-------------------------------------------------------------------

static void CollapseBranch (CTreeCtrl& tree, HTREEITEM hti)
{
	if (tree.ItemHasChildren (hti))
	{
		IGNORE_RETURN (tree.Expand (hti, TVE_COLLAPSE));
		hti = tree.GetChildItem (hti);

		do
		{
			CollapseBranch (tree, hti);
		}
		while ((hti = tree.GetNextSiblingItem (hti)) != 0);
	}
}

//-------------------------------------------------------------------

void LayerView::OnInsertCollapseall()
{
    HTREEITEM hti = GetTreeCtrl ().GetRootItem ();

    do
	{
		CollapseBranch (GetTreeCtrl (), hti);
	}
	while ((hti = GetTreeCtrl ().GetNextSiblingItem (hti)) != 0);
}

//-------------------------------------------------------------------

void LayerView::OnInsertCollapsebranch()
{
	CollapseBranch (GetTreeCtrl (), GetTreeCtrl ().GetSelectedItem ());
}

//-------------------------------------------------------------------

void LayerView::saveExpandedState (HTREEITEM hti)
{
	NOT_NULL (hti);

	if (GetTreeCtrl ().ItemHasChildren (hti))
	{
		TerrainEditorDoc::Item* item = reinterpret_cast<TerrainEditorDoc::Item*> (GetTreeCtrl ().GetItemData (hti));
		FATAL (item->type != TerrainEditorDoc::Item::T_layer, ("item is not a layer"));

		TerrainGenerator::Layer* layer = safe_cast<TerrainGenerator::Layer*> (item->layerItem);
		layer->setExpanded ((GetTreeCtrl ().GetItemState (hti, TVIS_EXPANDED) & TVIS_EXPANDED) != 0);

		hti = GetTreeCtrl ().GetChildItem (hti);

		do
		{
			saveExpandedState (hti);
		}
		while ((hti = GetTreeCtrl ().GetNextSiblingItem (hti)) != 0);
	}
}

//-------------------------------------------------------------------

void LayerView::saveExpandedState (void)
{
    HTREEITEM hti = GetTreeCtrl ().GetRootItem ();

	if (hti)
	{
		do
		{
			saveExpandedState (hti);
		}
		while ((hti = GetTreeCtrl ().GetNextSiblingItem (hti)) != 0);
	}
}

//-------------------------------------------------------------------


