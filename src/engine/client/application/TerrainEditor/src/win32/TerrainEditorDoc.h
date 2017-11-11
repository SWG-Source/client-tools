//
// TerrainEditorDoc.h
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#ifndef TERRAINEDITORDOC_H
#define TERRAINEDITORDOC_H

//-------------------------------------------------------------------

#include "sharedMath/Rectangle2d.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/ArrayList.h"
#include "PropertyView.h"
#include "sharedTerrain/TerrainGenerator.h"

//-------------------------------------------------------------------

class BakedTerrain;
class BitmapFrame;
class BitmapPreviewFrame;
class BlendGroupFrame;
class BookmarkFrame;
class BoundaryPolygon;
class ConsoleFrame;
class EnvironmentFrame;
class FindFrame;
class FloraGroupFrame;
class FractalFrame;
class FractalPreviewFrame;
class HelpFrame;
class LayerFrame;
class MapFrame;
class ProfileFrame;
class PropertyFrame;
class RadialGroupFrame;
class ShaderGroupFrame;
class View3dFrame;
class WarningFrame;
class PackedIntegerMap;
class PackedFixedPointMap;

//-------------------------------------------------------------------

namespace TerrainEditorParameters
{
	enum MapWidth
	{
		MW_32_32,
		MW_16_16,
		MW_8_8,
		MW_4_4
	};

	enum ChunkWidth
	{
		CW_32_32,
		CW_16_16,
		CW_8_8,
		CW_4_4
	};

	enum NumberOfTilesPerChunk
	{
		NOTPC_16,
		NOTPC_8,
		NOTPC_4,
		NOTPC_2,
		NOTPC_1
	};

	enum ShaderSize
	{
		SS_16,
		SS_8,
		SS_4,
		SS_2
	};
}

//-------------------------------------------------------------------

class TerrainEditorDoc : public CDocument
{
public:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum Hints
	{
		H_layerViewApply = 1000
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct Item
	{
	public:

		enum Type
		{
			T_unknown,
			T_boundary,
			T_filter,
			T_affector,
			T_layer
		};

	public:

		Type                         type;
		TerrainGenerator::LayerItem* layerItem;

	public:

		Item (void);
		~Item (void);
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	real                    mapWidthInMeters;
	real                    chunkWidthInMeters;
	int                     numberOfTilesPerChunk;
	real                    defaultShaderSize;

	bool                    useGlobalWaterTable;
	real                    globalWaterTableHeight;
	CString                 globalWaterTableShaderTemplateName;
	real                    globalWaterTableShaderSize;

	int                     m_environmentCycleTime;

	float                   m_collidableMinimumDistance;
	float                   m_collidableMaximumDistance;
	float                   m_collidableTileSize;
	float                   m_collidableTileBorder;
	uint32                  m_collidableSeed;
	float                   m_nonCollidableMinimumDistance;
	float                   m_nonCollidableMaximumDistance;
	float                   m_nonCollidableTileSize;
	float                   m_nonCollidableTileBorder;
	uint32                  m_nonCollidableSeed;
	float                   m_radialMinimumDistance;
	float                   m_radialMaximumDistance;
	float                   m_radialTileSize;
	float                   m_radialTileBorder;
	uint32                  m_radialSeed;
	float                   m_farRadialMinimumDistance;
	float                   m_farRadialMaximumDistance;
	float                   m_farRadialTileSize;
	float                   m_farRadialTileBorder;
	uint32                  m_farRadialSeed;
	bool                    m_legacyMap;

	TerrainGenerator*       terrainGenerator;

	RadialGroupFrame*       radialGroupFrame;
	FloraGroupFrame*        floraGroupFrame;
	LayerFrame*             layerFrame;
	BookmarkFrame*          bookmarkFrame;
	ShaderGroupFrame*       shaderGroupFrame;
	PropertyFrame*          propertyFrame;
	ConsoleFrame*           consoleFrame;
	MapFrame*               mapFrame;
	BlendGroupFrame*        blendGroupFrame;
	FractalFrame*           fractalFrame;
	FractalPreviewFrame*    fractalPreviewFrame;
	BitmapFrame*            bitmapFrame;
	BitmapPreviewFrame*     bitmapPreviewFrame;
	View3dFrame*            view3dFrame;
	HelpFrame*              helpFrame;
	ProfileFrame*           profileFrame;
	WarningFrame*           warningFrame;
	FindFrame*              findFrame;
	EnvironmentFrame*       environmentFrame;

	CString                 consoleMessage;

	Item*                   selectedItem;

	real                    blackHeight;
	real                    whiteHeight;

	real                    lastTotalChunkGenerationTime;
	real                    lastAverageChunkGenerationTime;
	real                    lastMinimumChunkGenerationTime;
	real                    lastMaximumChunkGenerationTime;

	BakedTerrain*           m_bakedTerrain;

	PackedIntegerMap       *m_staticCollidableFloraMap;
	PackedFixedPointMap    *m_staticCollidableFloraHeightMap;

private:

	void load_0013 (Iff& iff);
	void _load(Iff& iff, Tag version);

	bool _bakeFlora(const char *terrainFile);
	bool _importFloraSampleFile(const char *sampleFileName);

protected:

	TerrainEditorDoc();
	TerrainEditorDoc (const TerrainEditorDoc&);
	TerrainEditorDoc& operator= (const TerrainEditorDoc&);
	DECLARE_DYNCREATE(TerrainEditorDoc)

private:

	//{{AFX_VIRTUAL(TerrainEditorDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(TerrainEditorDoc)
	afx_msg void OnNewAffectorColorConstant();
	afx_msg void OnNewAffectorColorRampfractal();
	afx_msg void OnNewAffectorColorRampheight();
	afx_msg void OnNewAffectorExclude();
	afx_msg void OnNewAffectorPassable();
	afx_msg void OnNewAffectorFloraStaticCollidableConstant();
	afx_msg void OnNewAffectorFloraStaticNonCollidableConstant();
	afx_msg void OnNewAffectorFloraDynamicNearConstant();
	afx_msg void OnNewAffectorFloraDynamicFarConstant();
	afx_msg void OnNewAffectorHeightConstant();
	afx_msg void OnNewAffectorHeightFractal();
	afx_msg void OnNewAffectorHeightTerrace();
	afx_msg void OnNewAffectorShaderConstant();
	afx_msg void OnNewAffectorShaderReplace();
	afx_msg void OnNewFilterBitmap();
	afx_msg void OnNewFilterDirection();
	afx_msg void OnNewFilterFractal();
	afx_msg void OnNewFilterHeight();
	afx_msg void OnNewFilterShader();
	afx_msg void OnNewFilterSlope();
	afx_msg void OnOptionsFloraparameters();
	afx_msg void OnOptionsMapparameters();
	afx_msg void OnUpdateNewAffector(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNewFilter(CCmdUI* pCmdUI);
	afx_msg void OnViewMapPreferences();
	afx_msg void OnNewAffectorEnvironment();
	afx_msg void OnDebugAddallrules();
	afx_msg void OnToolsBaketerrain();
	afx_msg void OnToolsBakeriversroads();
	afx_msg void OnToolsBakeflora();
	afx_msg void OnDebugViewbakedterrain();
	afx_msg void OnEditClearconsole();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual ~TerrainEditorDoc();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void                    setRadialGroupFrame (RadialGroupFrame* newRadialGroupFrame);
	RadialGroupFrame*       getRadialGroupFrame (void);

	void                    setFloraGroupFrame (FloraGroupFrame* newFloraGroupFrame);
	FloraGroupFrame*        getFloraGroupFrame (void);

	void                    setLayerFrame (LayerFrame* newLayerFrame);
	LayerFrame*             getLayerFrame (void);

	void                    setBookmarkFrame (BookmarkFrame* newBookmarkFrame);
	BookmarkFrame*          getBookmarkFrame (void);

	void                    setShaderGroupFrame (ShaderGroupFrame* newShaderGroupFrame);
	ShaderGroupFrame*       getShaderGroupFrame (void);

	void                    setPropertyFrame (PropertyFrame* newPropertyFrame);
	PropertyFrame*          getPropertyFrame (void);

	void                    SetPropertyView (CRuntimeClass* cls, PropertyView::ViewData* vd);
	void                    ApplyPropertyViewChanges ();
	void                    RefreshPropertyView ();

	void                    setConsoleFrame (ConsoleFrame* newConsoleFrame);
	ConsoleFrame*           getConsoleFrame (void);

	void                    setMapFrame (MapFrame* newMapFrame);
	MapFrame*               getMapFrame (void);
	const MapFrame*         getMapFrame (void) const;

	void                    setBlendGroupFrame (BlendGroupFrame* newBlendGroupFrame);
	BlendGroupFrame*        getBlendGroupFrame (void);

	void                    setFractalFrame (FractalFrame* newFractalFrame);
	FractalFrame*           getFractalFrame (void);

	void                    setFractalPreviewFrame (FractalPreviewFrame* newFractalPreviewFrame);
	FractalPreviewFrame*    getFractalPreviewFrame (void);

	void                    setBitmapFrame (BitmapFrame* newBitmapFrame);
	BitmapFrame*            getBitmapFrame (void);

	void                    setBitmapPreviewFrame (BitmapPreviewFrame* newBitmapPreviewFrame);
	BitmapPreviewFrame*     getBitmapPreviewFrame (void);

	void                    setProfileFrame (ProfileFrame* newProfileFrame);
	ProfileFrame*           getProfileFrame (void);

	void                    setWarningFrame (WarningFrame* newWarningFrame);
	WarningFrame*           getWarningFrame (void);

	void                    setFindFrame (FindFrame* newFindFrame);
	FindFrame*              getFindFrame (void);

	void                    setEnvironmentFrame (EnvironmentFrame* newEnvironmentFrame);
	EnvironmentFrame*       getEnvironmentFrame (void);

	void                    setView3dFrame (View3dFrame* newView3dFrame);
	View3dFrame*            getView3dFrame (void);

	void                    setHelpFrame (HelpFrame* newHelpFrame);
	HelpFrame*              getHelpFrame (void);

	void                    clearConsoleMessage (void);
	void                    addConsoleMessage (const CString& newMessage);

	void                    setSelectedItem (Item* newSelectedLayerItem);
	Item*                   getSelectedItem (void);
	const Item*             getSelectedItem (void) const;

	void                    addBoundaryRectangle (const Rectangle2d& newRectangle);
	void                    addBoundaryCircle    (const Vector2d& newCenter, real newRadius);
	void                    addBoundaryPolygon   (const ArrayList<Vector2d>& newPointList);
	void                    addBoundaryPolyline  (const ArrayList<Vector2d>& newPointList);
	void                    addAffectorRoad      (const ArrayList<Vector2d>& newPointList);
	void                    addAffectorRiver     (const ArrayList<Vector2d>& newPointList);
	void                    addAffectorRibbon    (const ArrayList<Vector2d>& newPointList);

	CRuntimeClass*          getLayerRuntimeClass (Item* item) const;

	bool                    canAddBoundary (void) const;

	real                    getMapWidthInMeters (void) const;
	real                    getChunkWidthInMeters (void) const;
	real                    getTileWidthInMeters (void) const;
	int                     getNumberOfTilesPerChunk (void) const;
	real                    getDistanceBetweenHeightPoles (void) const;
	int                     getNumberOfHeightPolesPerChunk (void) const;

	bool                    getUseGlobalWaterTable (void) const;
	real                    getGlobalWaterTableHeight (void) const;

	TerrainGenerator*       getTerrainGenerator (void);
	const TerrainGenerator* getTerrainGenerator (void) const;

	BakedTerrain*           getBakedTerrain ();
	const BakedTerrain*     getBakedTerrain () const;

	real                    getDefaultShaderSize (void) const;

	real                    getBlackHeight (void) const;
	real                    getWhiteHeight (void) const;

	void                    BuildFloraFamilyDropList (CComboBox& cbox, int hiliteFamilyId) const;
	void                    BuildShaderFamilyDropList (CComboBox& cbox, int hiliteFamilyId) const;
	void                    BuildRadialFamilyDropList (CComboBox& cbox, int hiliteFamilyId) const;
	void                    BuildEnvironmentFamilyDropList (CComboBox& cbox, int hiliteFamilyId) const;
	void                    BuildFractalFamilyDropList (CComboBox& cbox, int hiliteFamilyId) const;
	void                    BuildBitmapFamilyDropList (CComboBox& cbox, int hiliteFamilyId) const;

	void                    setLastChunkGenerationTimes (real total, real average, real minimum, real maximum);
	real                    getLastTotalChunkGenerationTime (void) const;
	real                    getLastAverageChunkGenerationTime (void) const;
	real                    getLastMinimumChunkGenerationTime (void) const;
	real                    getLastMaximumChunkGenerationTime (void) const;

	void                    save (Iff& iff, const char* name=0) const;
};

//-------------------------------------------------------------------

extern void CONSOLE_PRINT (const CString& newMessage);

//-------------------------------------------------------------------

inline void TerrainEditorDoc::setRadialGroupFrame (RadialGroupFrame* newRadialGroupFrame)
{
	radialGroupFrame = newRadialGroupFrame;
}

//-------------------------------------------------------------------

inline RadialGroupFrame* TerrainEditorDoc::getRadialGroupFrame (void)
{
	return radialGroupFrame;
}

//-------------------------------------------------------------------

inline void TerrainEditorDoc::setFloraGroupFrame (FloraGroupFrame* newFloraGroupFrame)
{
	floraGroupFrame = newFloraGroupFrame;
}

//-------------------------------------------------------------------

inline FloraGroupFrame* TerrainEditorDoc::getFloraGroupFrame (void)
{
	return floraGroupFrame;
}

//-------------------------------------------------------------------

inline void TerrainEditorDoc::setLayerFrame (LayerFrame* newLayerFrame)
{
	layerFrame = newLayerFrame;
}

//-------------------------------------------------------------------

inline LayerFrame* TerrainEditorDoc::getLayerFrame (void)
{
	return layerFrame;
}

//-------------------------------------------------------------------

inline void TerrainEditorDoc::setShaderGroupFrame (ShaderGroupFrame* newShaderGroupFrame)
{
	shaderGroupFrame = newShaderGroupFrame;
}

//-------------------------------------------------------------------

inline ShaderGroupFrame* TerrainEditorDoc::getShaderGroupFrame (void)
{
	return shaderGroupFrame;
}

//-------------------------------------------------------------------

inline void TerrainEditorDoc::setBookmarkFrame (BookmarkFrame* newBookmarkFrame)
{
	bookmarkFrame = newBookmarkFrame;
}

//-------------------------------------------------------------------

inline BookmarkFrame* TerrainEditorDoc::getBookmarkFrame (void)
{
	return bookmarkFrame;
}

//-------------------------------------------------------------------

inline void TerrainEditorDoc::setHelpFrame (HelpFrame* newHelpFrame)
{
	helpFrame = newHelpFrame;
}

//-------------------------------------------------------------------

inline HelpFrame* TerrainEditorDoc::getHelpFrame (void)
{
	return helpFrame;
}

//-------------------------------------------------------------------

inline void TerrainEditorDoc::setPropertyFrame (PropertyFrame* newPropertyFrame)
{
	propertyFrame = newPropertyFrame;
}

//-------------------------------------------------------------------

inline PropertyFrame* TerrainEditorDoc::getPropertyFrame (void)
{
	return propertyFrame;
}

//-------------------------------------------------------------------

inline void TerrainEditorDoc::setConsoleFrame (ConsoleFrame* newConsoleFrame)
{
	consoleFrame = newConsoleFrame;
}

//-------------------------------------------------------------------

inline ConsoleFrame* TerrainEditorDoc::getConsoleFrame (void)
{
	return consoleFrame;
}

//-------------------------------------------------------------------

inline void TerrainEditorDoc::setMapFrame (MapFrame* newMapFrame)
{
	mapFrame = newMapFrame;
}

//-------------------------------------------------------------------

inline MapFrame* TerrainEditorDoc::getMapFrame (void)
{
	return mapFrame;
}

//-------------------------------------------------------------------

inline const MapFrame* TerrainEditorDoc::getMapFrame (void) const
{
	return mapFrame;
}

//-------------------------------------------------------------------

inline void TerrainEditorDoc::setBlendGroupFrame (BlendGroupFrame* newBlendGroupFrame)
{
	blendGroupFrame = newBlendGroupFrame;
}

//-------------------------------------------------------------------

inline BlendGroupFrame* TerrainEditorDoc::getBlendGroupFrame (void)
{
	return blendGroupFrame;
}

//-------------------------------------------------------------------

inline void TerrainEditorDoc::setFractalFrame (FractalFrame* newFractalFrame)
{
	fractalFrame = newFractalFrame;
}

//-------------------------------------------------------------------

inline FractalFrame* TerrainEditorDoc::getFractalFrame (void)
{
	return fractalFrame;
}

//-------------------------------------------------------------------

inline void TerrainEditorDoc::setBitmapFrame (BitmapFrame* newBitmapFrame)
{
	bitmapFrame = newBitmapFrame;
}

//-------------------------------------------------------------------

inline BitmapFrame* TerrainEditorDoc::getBitmapFrame (void)
{
	return bitmapFrame;
}

//-------------------------------------------------------------------

inline void TerrainEditorDoc::setView3dFrame (View3dFrame* newView3dFrame)
{
	view3dFrame = newView3dFrame;
}

//-------------------------------------------------------------------

inline View3dFrame* TerrainEditorDoc::getView3dFrame (void)
{
	return view3dFrame;
}

//-------------------------------------------------------------------

inline void TerrainEditorDoc::setFractalPreviewFrame (FractalPreviewFrame* newFractalPreviewFrame)
{
	fractalPreviewFrame = newFractalPreviewFrame;
}

//-------------------------------------------------------------------

inline FractalPreviewFrame* TerrainEditorDoc::getFractalPreviewFrame (void)
{
	return fractalPreviewFrame;
}


//-------------------------------------------------------------------

inline void TerrainEditorDoc::setBitmapPreviewFrame (BitmapPreviewFrame* newBitmapPreviewFrame)
{
	bitmapPreviewFrame = newBitmapPreviewFrame;
}

//-------------------------------------------------------------------

inline BitmapPreviewFrame* TerrainEditorDoc::getBitmapPreviewFrame (void)
{
	return bitmapPreviewFrame;
}

//-------------------------------------------------------------------

inline void TerrainEditorDoc::setProfileFrame (ProfileFrame* newProfileFrame)
{
	profileFrame = newProfileFrame;
}

//-------------------------------------------------------------------

inline ProfileFrame* TerrainEditorDoc::getProfileFrame (void)
{
	return profileFrame;
}

//-------------------------------------------------------------------

inline void TerrainEditorDoc::setWarningFrame (WarningFrame* newWarningFrame)
{
	warningFrame = newWarningFrame;
}

//-------------------------------------------------------------------

inline WarningFrame* TerrainEditorDoc::getWarningFrame (void)
{
	return warningFrame;
}

//-------------------------------------------------------------------

inline void TerrainEditorDoc::setFindFrame (FindFrame* newFindFrame)
{
	findFrame = newFindFrame;
}

//-------------------------------------------------------------------

inline FindFrame* TerrainEditorDoc::getFindFrame (void)
{
	return findFrame;
}

//-------------------------------------------------------------------

inline void TerrainEditorDoc::setEnvironmentFrame (EnvironmentFrame* newEnvironmentFrame)
{
	environmentFrame = newEnvironmentFrame;
}

//-------------------------------------------------------------------

inline EnvironmentFrame* TerrainEditorDoc::getEnvironmentFrame (void)
{
	return environmentFrame;
}

//-------------------------------------------------------------------

inline TerrainGenerator* TerrainEditorDoc::getTerrainGenerator (void)
{
	return terrainGenerator;
}

//-------------------------------------------------------------------

inline const TerrainGenerator* TerrainEditorDoc::getTerrainGenerator (void) const
{
	return terrainGenerator;
}

//-------------------------------------------------------------------

inline void TerrainEditorDoc::setSelectedItem (TerrainEditorDoc::Item* newSelectedItem)
{
	selectedItem = newSelectedItem;
}

//-------------------------------------------------------------------

inline TerrainEditorDoc::Item* TerrainEditorDoc::getSelectedItem (void)
{
	return selectedItem;
}

//-------------------------------------------------------------------

inline const TerrainEditorDoc::Item* TerrainEditorDoc::getSelectedItem (void) const
{
	return selectedItem;
}

//-------------------------------------------------------------------

inline real TerrainEditorDoc::getMapWidthInMeters () const
{
	return mapWidthInMeters;
}

//-------------------------------------------------------------------

inline real TerrainEditorDoc::getChunkWidthInMeters () const
{
	return chunkWidthInMeters;
}

//-------------------------------------------------------------------

inline real TerrainEditorDoc::getTileWidthInMeters () const
{
	return chunkWidthInMeters / numberOfTilesPerChunk;
}

//-------------------------------------------------------------------

inline int TerrainEditorDoc::getNumberOfTilesPerChunk () const
{
	return numberOfTilesPerChunk;
}

//-------------------------------------------------------------------

inline real TerrainEditorDoc::getDistanceBetweenHeightPoles () const
{
	return chunkWidthInMeters / (2.f * static_cast<real> (numberOfTilesPerChunk));
}

//-------------------------------------------------------------------

inline int TerrainEditorDoc::getNumberOfHeightPolesPerChunk () const
{
	return numberOfTilesPerChunk * 2 + 3;
}

//-------------------------------------------------------------------

inline real TerrainEditorDoc::getDefaultShaderSize (void) const
{
	return defaultShaderSize;
}

//-------------------------------------------------------------------

inline real TerrainEditorDoc::getBlackHeight (void) const
{
	return blackHeight;
}

//-------------------------------------------------------------------

inline real TerrainEditorDoc::getWhiteHeight (void) const
{
	return whiteHeight;
}

//-------------------------------------------------------------------

inline bool TerrainEditorDoc::getUseGlobalWaterTable (void) const
{
	return useGlobalWaterTable;
}

//-------------------------------------------------------------------

inline real TerrainEditorDoc::getGlobalWaterTableHeight (void) const
{
	return globalWaterTableHeight;
}

//-------------------------------------------------------------------

inline void TerrainEditorDoc::setLastChunkGenerationTimes (real total, real average, real minimum, real maximum)
{
	lastTotalChunkGenerationTime   = total;
	lastAverageChunkGenerationTime = average;
	lastMinimumChunkGenerationTime = minimum;
	lastMaximumChunkGenerationTime = maximum;
}

//-------------------------------------------------------------------

inline real TerrainEditorDoc::getLastTotalChunkGenerationTime (void) const
{
	return lastTotalChunkGenerationTime;
}

//-------------------------------------------------------------------

inline real TerrainEditorDoc::getLastAverageChunkGenerationTime (void) const
{
	return lastAverageChunkGenerationTime;
}

//-------------------------------------------------------------------

inline real TerrainEditorDoc::getLastMinimumChunkGenerationTime (void) const
{
	return lastMinimumChunkGenerationTime;
}

//-------------------------------------------------------------------

inline real TerrainEditorDoc::getLastMaximumChunkGenerationTime (void) const
{
	return lastMaximumChunkGenerationTime;
}

//-------------------------------------------------------------------

inline BakedTerrain* TerrainEditorDoc::getBakedTerrain ()
{
	return m_bakedTerrain;
}

//-------------------------------------------------------------------

inline const BakedTerrain* TerrainEditorDoc::getBakedTerrain () const
{
	return m_bakedTerrain;
}

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif
