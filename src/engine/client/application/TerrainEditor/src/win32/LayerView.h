//
// LayerView.h
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#ifndef LAYERVIEW_H
#define LAYERVIEW_H

//-------------------------------------------------------------------

#include "sharedTerrain/TerrainGenerator.h"

//-------------------------------------------------------------------

class LayerView : public CTreeView
{
public:

	enum LayerViewIconType
	{
		LVIT_unknown,
		LVIT_layer,
		LVIT_boundaryRectangle,
		LVIT_boundaryCircle,
		LVIT_boundaryPolygon,
		LVIT_boundaryPolyline,
		LVIT_filterSlope,
		LVIT_filterDirection,
		LVIT_filterHeight,
		LVIT_filterFractal,
		LVIT_filterShader,
		LVIT_affectorHeight,
		LVIT_affectorColor,
		LVIT_affectorShader,
		LVIT_affectorFloraStatic,
		LVIT_affectorFloraDynamic,
		LVIT_affectorExclude,
		LVIT_affectorRoad,
		LVIT_affectorRiver,
		LVIT_affectorEnvironment,
		LVIT_layerInvertBoundaries,
		LVIT_layerInvertFilters,
		LVIT_layerInvertBoundariesFilters,
		LVIT_boundaryPolygonWaterTable,
		LVIT_boundaryRectangleWaterTable,
		LVIT_affectorPassable,
		LVIT_filterBitmap,
		LVIT_affectorRibbon
	};

public:

	static const LayerViewIconType getIcon (const TerrainGenerator::LayerItem* layerItem);

private:

	//-- drag and drop
	enum DragMode
	{
		DM_nothing,
		DM_move,
		DM_copy
	};

private:

	bool        imageListSet;
	CImageList  imageList;

	HCURSOR     moveCursor;
	HCURSOR     copyCursor;

	DragMode        m_dragMode;
	CImageList*     m_pDragImage;
	HTREEITEM       m_htiDrag, m_htiDrop, m_htiOldDrop;
	bool            m_bLDragging;
	UINT            m_idTimer;

	void            OnLButtonUpForMove ();
	void            OnLButtonUpForCopy ();
	void			ExportBoundary ();
	void			ExportFilter ();
	void			ExportAffector ();
	void			ExportLayer ();

	void            refreshIcons (HTREEITEM item);
	void            refreshIcons (void);

private:

	void      expandBranch (HTREEITEM item);
	void      expandAll (void);

	HTREEITEM getRoot (HTREEITEM hitem) const;

	bool      verify (void);

public:

	HTREEITEM addLayerToTree    (HTREEITEM parent, HTREEITEM afterItem, TerrainGenerator::Layer* layer) const;

private:

	HTREEITEM addBoundaryToTree (HTREEITEM parent, TerrainGenerator::Boundary* boundary) const;
	HTREEITEM addFilterToTree   (HTREEITEM parent, TerrainGenerator::Filter* filter) const;
	HTREEITEM addAffectorToTree (HTREEITEM parent, TerrainGenerator::Affector* affector) const;

	bool      isLayer (HTREEITEM selection) const;
	HTREEITEM getSelectedLayer (void) const;
	bool      isLayerSelected (void) const;
	bool      isBoundary (HTREEITEM selection) const;
	HTREEITEM getSelectedBoundary (void) const;
	bool      isBoundarySelected (void) const;
	bool      isFilter (HTREEITEM selection) const;
	HTREEITEM getSelectedFilter (void) const;
	bool      isFilterSelected (void) const;
	bool      isAffector (HTREEITEM selection) const;
	HTREEITEM getSelectedAffector (void) const;
	bool      isAffectorSelected (void) const;

	void      deleteLayer (void) const;
	void      deleteBoundary (void) const;
	void      deleteFilter (void) const;
	void      deleteAffector (void) const;

	void      updateTree (HTREEITEM item);

	HTREEITEM find (HTREEITEM hItem, const TerrainGenerator::LayerItem* layerItem);

	void      saveExpandedState (HTREEITEM hti);
	void      saveExpandedState (void);

public:

	bool      canAddBoundary (void) const;
	bool      canAddFilter (void) const;
	bool      canAddAffector (void) const;

	void      addBoundary (TerrainGenerator::Boundary* newBoundary) const;
	void      addAffector (TerrainGenerator::Affector* newAffector) const;
	void      addFilter   (TerrainGenerator::Filter*   newFilter) const;
	void      addLayer    (TerrainGenerator::Layer*    newLayer) const;

	void      selectLayerItem (const TerrainGenerator::LayerItem* layerItem);

protected:

	LayerView (void);           // protected constructor used by dynamic creation
	virtual ~LayerView (void);

	DECLARE_DYNCREATE(LayerView)

public:

	//{{AFX_VIRTUAL(LayerView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

protected:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//{{AFX_MSG(LayerView)
	afx_msg void OnButtonDeleteLayer();
	afx_msg void OnButtonDemoteLayer();
	afx_msg void OnButtonPromoteLayer();
	afx_msg void OnUpdateButtonDeleteLayer(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonDemoteLayer(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonPromoteLayer(CCmdUI* pCmdUI);
	afx_msg void OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonApply();
	afx_msg void OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnExportLayeritem();
	afx_msg void OnUpdateExportLayeritem(CCmdUI* pCmdUI);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnInsertDelete();
	afx_msg void OnInsertRename();
	afx_msg void OnUpdateInsertDelete(CCmdUI* pCmdUI);
	afx_msg void OnUpdateInsertRename(CCmdUI* pCmdUI);
	afx_msg void OnButtonNewrootlayer();
	afx_msg void OnButtonNewsublayer();
	afx_msg void OnButtonPushroot();
	afx_msg void OnUpdateButtonPushroot(CCmdUI* pCmdUI);
	afx_msg void OnButtonPushup();
	afx_msg void OnUpdateButtonPushup(CCmdUI* pCmdUI);
	afx_msg void OnInsertImportAffector();
	afx_msg void OnInsertImportBoundary();
	afx_msg void OnInsertImportFilter();
	afx_msg void OnInsertImportLayer();
	afx_msg void OnButtonScanLayers();
	afx_msg void OnInsertScanAll();
	afx_msg void OnInsertScanLayer();
	afx_msg void OnUpdateInsertScanLayer(CCmdUI* pCmdUI);
	afx_msg void OnButtonShowProfile();
	afx_msg void OnInsertScanDump();
	afx_msg void OnInsertExpandall();
	afx_msg void OnInsertCollapseall();
	afx_msg void OnInsertCollapsebranch();
	afx_msg void OnInsertExpandbranch();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
