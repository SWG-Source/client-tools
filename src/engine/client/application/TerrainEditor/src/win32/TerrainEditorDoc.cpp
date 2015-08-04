//
// TerrainEditorDoc.cpp
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "TerrainEditorDoc.h"

#include "ConsoleFrame.h"
#include "DialogEditorPreferences.h"
#include "DialogFloraParameters.h"
#include "DialogMapParameters.h"
#include "FindFrame.h"
#include "FormAffectorColorConstant.h"
#include "FormAffectorColorRampFractal.h"
#include "FormAffectorColorRampHeight.h"
#include "FormAffectorEnvironment.h"
#include "FormAffectorFloraDynamicConstant.h"
#include "FormAffectorFloraStaticConstant.h"
#include "FormAffectorRibbon.h"
#include "FormAffectorHeightConstant.h"
#include "FormAffectorHeightFractal.h"
#include "FormAffectorHeightTerrace.h"
#include "FormAffectorPassable.h"
#include "FormAffectorRiver.h"
#include "FormAffectorRoad.h"
#include "FormAffectorShaderConstant.h"
#include "FormAffectorShaderReplace.h"
#include "FormBoundaryCircle.h"
#include "FormBoundaryPolygon.h"
#include "FormBoundaryPolyline.h"
#include "FormBoundaryRect.h"
#include "FormFilterBitmap.h"
#include "FormFilterDirection.h"
#include "FormFilterFractal.h"
#include "FormFilterHeight.h"
#include "FormFilterShader.h"
#include "FormFilterSlope.h"
#include "FormLayer.h"
#include "LayerFrame.h"
#include "LayerView.h"
#include "MainFrame.h"
#include "MapFrame.h"
#include "MapView.h"
#include "PropertyFrame.h"
#include "ProgressDialog.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedRandom/Random.h"
#include "sharedTerrain/Affector.h"
#include "sharedTerrain/AffectorColor.h"
#include "sharedTerrain/AffectorEnvironment.h"
#include "sharedTerrain/AffectorFloraDynamic.h"
#include "sharedTerrain/AffectorFloraStatic.h"
#include "sharedTerrain/AffectorRibbon.h"
#include "sharedTerrain/AffectorHeight.h"
#include "sharedTerrain/AffectorPassable.h"
#include "sharedTerrain/AffectorRiver.h"
#include "sharedTerrain/AffectorRoad.h"
#include "sharedTerrain/AffectorShader.h"
#include "sharedTerrain/Boundary.h"
#include "sharedTerrain/Filter.h"
#include "sharedTerrain/ProceduralTerrainAppearanceTemplate.h"
#include "sharedTerrain/SamplerProceduralTerrainAppearanceTemplate.h"
#include "sharedTerrain/TerrainGeneratorLoader.h"
#include "sharedTerrain/TerrainGeneratorType.def"
#include "sharedUtility/BakedTerrain.h"
#include "sharedUtility/PackedIntegerMap.h"
#include "sharedUtility/PackedFixedPointMap.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/ProcessSpawner.h"
#include "TerrainEditor.h"

#include <algorithm>
#include <string>

#include <io.h>

//===================================================================

namespace
{
	static void _normalizePath(char *path, bool stripFile)
	{
		if (!path || !*path)
		{
			return;
		}

		char *iter, *lastDelim=0;
		for (iter=path;*iter;iter++)
		{
			if (*iter=='/')
			{
				*iter='\\';
			}

			if (*iter=='\\' || *iter==':')
			{
				lastDelim=iter;
			}
		}

		if (stripFile)
		{
			if (lastDelim)
			{
				lastDelim[1]=0;
			}
			else
			{
				path[0]=0;
			}
		}
		else if (iter!=lastDelim+1)
		{
			*iter++='\\';
			*iter++=0;
		}
	}
}

//===================================================================

TerrainEditorDoc::Item::Item (void) :
	type (T_unknown),
	layerItem (0)
{
}

//-------------------------------------------------------------------

TerrainEditorDoc::Item::~Item (void)
{
	type      = T_unknown;
	layerItem = 0;
}

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(TerrainEditorDoc, CDocument)

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(TerrainEditorDoc, CDocument)
	//{{AFX_MSG_MAP(TerrainEditorDoc)
	ON_COMMAND(ID_NEW_AFFECTOR_COLOR_CONSTANT, OnNewAffectorColorConstant)
	ON_COMMAND(ID_NEW_AFFECTOR_COLOR_RAMPFRACTAL, OnNewAffectorColorRampfractal)
	ON_COMMAND(ID_NEW_AFFECTOR_COLOR_RAMPHEIGHT, OnNewAffectorColorRampheight)
	ON_COMMAND(ID_NEW_AFFECTOR_EXCLUDE, OnNewAffectorExclude)
	ON_COMMAND(ID_NEW_AFFECTOR_PASSABLE, OnNewAffectorPassable)
	ON_COMMAND(ID_NEW_AFFECTOR_FLORA_COLLIDABLE_CONSTANT, OnNewAffectorFloraStaticCollidableConstant)
	ON_COMMAND(ID_NEW_AFFECTOR_FLORA_NONCOLLIDABLE_CONSTANT, OnNewAffectorFloraStaticNonCollidableConstant)
	ON_COMMAND(ID_NEW_AFFECTOR_FLORA_DYNAMICNEAR_CONSTANT, OnNewAffectorFloraDynamicNearConstant)
	ON_COMMAND(ID_NEW_AFFECTOR_FLORA_DYNAMICFAR_CONSTANT, OnNewAffectorFloraDynamicFarConstant)
	ON_COMMAND(ID_NEW_AFFECTOR_HEIGHT_CONSTANT, OnNewAffectorHeightConstant)
	ON_COMMAND(ID_NEW_AFFECTOR_HEIGHT_FRACTAL, OnNewAffectorHeightFractal)
	ON_COMMAND(ID_NEW_AFFECTOR_HEIGHT_TERRACE, OnNewAffectorHeightTerrace)
	ON_COMMAND(ID_NEW_AFFECTOR_SHADER_CONSTANT, OnNewAffectorShaderConstant)
	ON_COMMAND(ID_NEW_AFFECTOR_SHADER_REPLACE, OnNewAffectorShaderReplace)
	ON_COMMAND(ID_NEW_FILTER_BITMAP, OnNewFilterBitmap)
	ON_COMMAND(ID_NEW_FILTER_DIRECTION, OnNewFilterDirection)
	ON_COMMAND(ID_NEW_FILTER_FRACTAL, OnNewFilterFractal)
	ON_COMMAND(ID_NEW_FILTER_HEIGHT, OnNewFilterHeight)
	ON_COMMAND(ID_NEW_FILTER_SHADER, OnNewFilterShader)
	ON_COMMAND(ID_NEW_FILTER_SLOPE, OnNewFilterSlope)
	ON_COMMAND(ID_OPTIONS_FLORAPARAMETERS, OnOptionsFloraparameters)
	ON_COMMAND(ID_OPTIONS_MAPPARAMETERS, OnOptionsMapparameters)
	ON_UPDATE_COMMAND_UI(ID_NEW_AFFECTOR_FLORA_COLLIDABLE_CONSTANT, OnUpdateNewAffector)
	ON_UPDATE_COMMAND_UI(ID_NEW_FILTER_DIRECTION, OnUpdateNewFilter)
	ON_COMMAND(ID_VIEW_MAP_PREFERENCES, OnViewMapPreferences)
	ON_COMMAND(ID_NEW_AFFECTOR_ENVIRONMENT, OnNewAffectorEnvironment)
	ON_COMMAND(ID_DEBUG_ADDALLRULES, OnDebugAddallrules)
	ON_COMMAND(ID_TOOLS_BAKETERRAIN, OnToolsBaketerrain)
	ON_COMMAND(ID_TOOLS_BAKEFLORA, OnToolsBakeflora)
	ON_COMMAND(ID_TOOLS_BAKERIVERSROADS, OnToolsBakeriversroads)
	ON_COMMAND(ID_DEBUG_VIEWBAKEDTERRAIN, OnDebugViewbakedterrain)
	ON_UPDATE_COMMAND_UI(ID_NEW_AFFECTOR_FLORA_DYNAMICFAR_CONSTANT, OnUpdateNewAffector)
	ON_UPDATE_COMMAND_UI(ID_NEW_AFFECTOR_FLORA_DYNAMICNEAR_CONSTANT, OnUpdateNewAffector)
	ON_UPDATE_COMMAND_UI(ID_NEW_AFFECTOR_FLORA_NONCOLLIDABLE_CONSTANT, OnUpdateNewAffector)
	ON_UPDATE_COMMAND_UI(ID_NEW_AFFECTOR_COLOR_CONSTANT, OnUpdateNewAffector)
	ON_UPDATE_COMMAND_UI(ID_NEW_AFFECTOR_COLOR_RAMPFRACTAL, OnUpdateNewAffector)
	ON_UPDATE_COMMAND_UI(ID_NEW_AFFECTOR_COLOR_RAMPHEIGHT, OnUpdateNewAffector)
	ON_UPDATE_COMMAND_UI(ID_NEW_AFFECTOR_EXCLUDE, OnUpdateNewAffector)
	ON_UPDATE_COMMAND_UI(ID_NEW_AFFECTOR_PASSABLE, OnUpdateNewAffector)
	ON_UPDATE_COMMAND_UI(ID_NEW_AFFECTOR_HEIGHT_CONSTANT, OnUpdateNewAffector)
	ON_UPDATE_COMMAND_UI(ID_NEW_AFFECTOR_HEIGHT_FRACTAL, OnUpdateNewAffector)
	ON_UPDATE_COMMAND_UI(ID_NEW_AFFECTOR_HEIGHT_TERRACE, OnUpdateNewAffector)
	ON_UPDATE_COMMAND_UI(ID_NEW_AFFECTOR_SHADER_CONSTANT, OnUpdateNewAffector)
	ON_UPDATE_COMMAND_UI(ID_NEW_FILTER_BITMAP, OnUpdateNewFilter)
	ON_UPDATE_COMMAND_UI(ID_NEW_FILTER_FRACTAL, OnUpdateNewFilter)
	ON_UPDATE_COMMAND_UI(ID_NEW_FILTER_HEIGHT, OnUpdateNewFilter)
	ON_UPDATE_COMMAND_UI(ID_NEW_FILTER_SHADER, OnUpdateNewFilter)
	ON_UPDATE_COMMAND_UI(ID_NEW_FILTER_SLOPE, OnUpdateNewFilter)
	ON_UPDATE_COMMAND_UI(ID_NEW_AFFECTOR_ENVIRONMENT, OnUpdateNewAffector)
	ON_COMMAND(ID_EDIT_CLEARCONSOLE, OnEditClearconsole)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

TerrainEditorDoc::TerrainEditorDoc() :
	CDocument (),
	mapWidthInMeters (0.f),
	chunkWidthInMeters (0.f),
	numberOfTilesPerChunk (0),
	defaultShaderSize (CONST_REAL(2)),
	useGlobalWaterTable (false),
	globalWaterTableHeight (0),
	globalWaterTableShaderTemplateName (),
	globalWaterTableShaderSize (2.f),
	m_environmentCycleTime (60),
	m_collidableMinimumDistance (0.f),
	m_collidableMaximumDistance (128.f),
	m_collidableTileSize (8.f),
	m_collidableTileBorder (2.f),
	m_collidableSeed (0),
	m_nonCollidableMinimumDistance (0.f),
	m_nonCollidableMaximumDistance (64.f),
	m_nonCollidableTileSize (4.f),
	m_nonCollidableTileBorder (0.5f),
	m_nonCollidableSeed (0),
	m_radialMinimumDistance (0.f),
	m_radialMaximumDistance (20.f),
	m_radialTileSize (1.f),
	m_radialTileBorder (0.f),
	m_radialSeed (0),
	m_farRadialMinimumDistance (128.f),
	m_farRadialMaximumDistance (512.f),
	m_farRadialTileSize (32.f),
	m_farRadialTileBorder (8.f),
	m_farRadialSeed (0),
	m_legacyMap(false),
	terrainGenerator (0),
	radialGroupFrame (0),
	floraGroupFrame (0),
	layerFrame (0),
	bookmarkFrame (0),
	shaderGroupFrame (0),
	propertyFrame (0),
	consoleFrame (0),
	mapFrame (0),
	blendGroupFrame (0),
	fractalFrame (0),
	fractalPreviewFrame (0),
	bitmapFrame (0),
	bitmapPreviewFrame (0),
	view3dFrame (0),
	helpFrame (0),
	profileFrame (0),
	warningFrame (0),
	findFrame (0),
	environmentFrame (0),
	consoleMessage (),
	selectedItem (0),
	blackHeight (CONST_REAL (0)),
	whiteHeight (CONST_REAL (255)),
	lastTotalChunkGenerationTime (0),
	lastAverageChunkGenerationTime (0),
	lastMinimumChunkGenerationTime (0),
	lastMaximumChunkGenerationTime (0),
	m_bakedTerrain(0),
	m_staticCollidableFloraMap(0),
	m_staticCollidableFloraHeightMap(0)
{
	//-- create terrain generator
	terrainGenerator = new TerrainGenerator ();

	//-- create baked terrain
	m_bakedTerrain = new BakedTerrain ();
}

//-------------------------------------------------------------------

TerrainEditorDoc::~TerrainEditorDoc()
{
	if (terrainGenerator)
	{
		delete terrainGenerator;
		terrainGenerator = 0;
	}

	if (m_bakedTerrain)
	{
		delete m_bakedTerrain;
		m_bakedTerrain = 0;
	}

	if (m_staticCollidableFloraMap)
	{
		delete m_staticCollidableFloraMap;
		m_staticCollidableFloraMap=0;
	}

	if (m_staticCollidableFloraHeightMap)
	{
		delete m_staticCollidableFloraHeightMap;
		m_staticCollidableFloraHeightMap=0;
	}

	radialGroupFrame = 0;
	floraGroupFrame = 0;
	layerFrame = 0;
	bookmarkFrame = 0;
	shaderGroupFrame = 0;
	propertyFrame = 0;
	consoleFrame = 0;
	mapFrame = 0;
	blendGroupFrame = 0;
	fractalFrame = 0;
	fractalPreviewFrame = 0;
	bitmapFrame = 0;
	bitmapPreviewFrame = 0;
	view3dFrame = 0;
	helpFrame = 0;
	profileFrame = 0;
	warningFrame = 0;
	findFrame = 0;
	environmentFrame = 0;
	selectedItem = 0;
}

//-------------------------------------------------------------------

BOOL TerrainEditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	mapWidthInMeters           = CONST_REAL (4096);
	chunkWidthInMeters         = CONST_REAL (8);
	numberOfTilesPerChunk      = 4;
	defaultShaderSize          = CONST_REAL (2);

	//-- recreate generator
	NOT_NULL (terrainGenerator);
	terrainGenerator->reset ();

	//-- open default views
	safe_cast<TerrainEditorApp*> (AfxGetApp ())->onOpenDefaultViews (this);

	return TRUE;
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void TerrainEditorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void TerrainEditorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void TerrainEditorDoc::_load(Iff& iff, Tag version)
{
	iff.enterForm (version);

		iff.enterChunk (TAG_DATA);
		{
			m_legacyMap = true;

			//----------------------

			char* buffer = iff.read_string ();
			delete [] buffer;

			mapWidthInMeters               = iff.read_float ();
			chunkWidthInMeters             = iff.read_float ();
			numberOfTilesPerChunk          = iff.read_int32 ();
			useGlobalWaterTable            = iff.read_int32 () != 0;
			globalWaterTableHeight         = iff.read_float ();
			globalWaterTableShaderSize     = iff.read_float ();

			buffer = iff.read_string ();
			globalWaterTableShaderTemplateName = buffer;
			delete [] buffer;

			m_environmentCycleTime = static_cast<int> (iff.read_float ());

			if (version==TAG_0013)
			{
				std::string dummyString;
				iff.read_string (dummyString);
				iff.read_string (dummyString);
				IGNORE_RETURN (iff.read_float ());
				iff.read_string (dummyString);
				IGNORE_RETURN (iff.read_float ());
				iff.read_string (dummyString);
				IGNORE_RETURN (iff.read_float ());
				iff.read_string (dummyString);
				IGNORE_RETURN (iff.read_float ());
				IGNORE_RETURN (iff.read_int32 ());
				iff.read_string (dummyString);
			}

			m_collidableMinimumDistance    = iff.read_float ();
			m_collidableMaximumDistance    = iff.read_float ();
			m_collidableTileSize           = iff.read_float ();
			m_collidableTileBorder         = iff.read_float ();
			m_collidableSeed               = iff.read_uint32 ();
			m_nonCollidableMinimumDistance = iff.read_float ();
			m_nonCollidableMaximumDistance = iff.read_float ();
			m_nonCollidableTileSize        = iff.read_float ();
			m_nonCollidableTileBorder      = iff.read_float ();
			m_nonCollidableSeed            = iff.read_uint32 ();
			m_radialMinimumDistance        = iff.read_float ();
			m_radialMaximumDistance        = iff.read_float ();
			m_radialTileSize               = iff.read_float ();
			m_radialTileBorder             = iff.read_float ();
			m_radialSeed                   = iff.read_uint32 ();
			m_farRadialMinimumDistance     = iff.read_float ();
			m_farRadialMaximumDistance     = iff.read_float ();
			m_farRadialTileSize            = iff.read_float ();
			m_farRadialTileBorder          = iff.read_float ();
			m_farRadialSeed                = iff.read_uint32 ();

			// --------------------
			if (version>=TAG_0015)
			{
				m_legacyMap = iff.read_bool8();
			}
		}

		iff.exitChunk (TAG_DATA, true);

		NOT_NULL (terrainGenerator);
		terrainGenerator->load (iff);

		NOT_NULL (m_bakedTerrain);
		m_bakedTerrain->load (iff);

		if (version>=TAG_0015)
		{
			m_staticCollidableFloraMap       = new PackedIntegerMap(iff);
			m_staticCollidableFloraHeightMap = new PackedFixedPointMap(iff);
		}

	iff.exitForm (version);
}

//-------------------------------------------------------------------

BOOL TerrainEditorDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
	{
		return FALSE;
	}

	FatalSetThrowExceptions (false);
	try
	{
		Iff iff;
		if (iff.open (lpszPathName, true))
		{
			if (iff.getCurrentName() == TAG (M,P,T,A) || iff.getCurrentName () == TAG (P,T,A,T))
			{
				iff.enterForm();
			}

			if (iff.getCurrentName() != TAG_0015)
			{
				MessageBox(
					AfxGetApp ()->m_pMainWnd->m_hWnd, 
					"The data file you are loading is an older version. Saving your\r\ndata file will automatically convert it to the new format.", 
					"Warning", 
					MB_ICONWARNING
					);
			}

			Tag version = iff.getCurrentName();
			if (version>=TAG_0013 && version<=TAG_0015)
			{
				_load(iff, version);
			}
			else
			{
				IGNORE_RETURN (MessageBox (0, "FORM loader not found", "Error", MB_ICONSTOP));
			}

			iff.exitForm ();
		}
	}
	catch (FatalException& e)
	{
		MessageBox(0, e.getMessage(), "Error loading terrain file.", MB_ICONSTOP);
	}
	FatalSetThrowExceptions (false);

	//-- open default views
	safe_cast<TerrainEditorApp*> (AfxGetApp ())->onOpenDefaultViews (this);

	return TRUE;
}

//-------------------------------------------------------------------

void TerrainEditorDoc::save (Iff& iff, const char* name) const
{
	ProceduralTerrainAppearanceTemplate::WriterData data;
	data.name                           = name ? name : "";
	data.mapWidthInMeters               = mapWidthInMeters;
	data.chunkWidthInMeters             = chunkWidthInMeters;
	data.numberOfTilesPerChunk          = numberOfTilesPerChunk;
	data.useGlobalWaterTable            = useGlobalWaterTable;
	data.globalWaterTableHeight         = globalWaterTableHeight;
	data.globalWaterTableShaderTemplateName = globalWaterTableShaderTemplateName;
	data.globalWaterTableShaderSize     = globalWaterTableShaderSize;
	data.environmentCycleTime           = static_cast<float> (m_environmentCycleTime);
	data.collidableMinimumDistance      = m_collidableMinimumDistance;
	data.collidableMaximumDistance      = m_collidableMaximumDistance;
	data.collidableTileSize             = m_collidableTileSize;
	data.collidableTileBorder           = m_collidableTileBorder;
	data.collidableSeed                 = m_collidableSeed;
	data.nonCollidableMinimumDistance   = m_nonCollidableMinimumDistance;
	data.nonCollidableMaximumDistance   = m_nonCollidableMaximumDistance;
	data.nonCollidableTileSize          = m_nonCollidableTileSize;
	data.nonCollidableTileBorder        = m_nonCollidableTileBorder;
	data.nonCollidableSeed              = m_nonCollidableSeed;
	data.radialMinimumDistance          = m_radialMinimumDistance;
	data.radialMaximumDistance          = m_radialMaximumDistance;
	data.radialTileSize                 = m_radialTileSize;
	data.radialTileBorder               = m_radialTileBorder;
	data.radialSeed                     = m_radialSeed;
	data.farRadialMinimumDistance       = m_farRadialMinimumDistance;
	data.farRadialMaximumDistance       = m_farRadialMaximumDistance;
	data.farRadialTileSize              = m_farRadialTileSize;
	data.farRadialTileBorder            = m_farRadialTileBorder;
	data.farRadialSeed                  = m_farRadialSeed;
	data.legacyMap                      = m_legacyMap;

	data.terrainGenerator               = terrainGenerator;
	data.bakedTerrain                   = m_bakedTerrain;

	const bool emptyFloraMaps = !m_staticCollidableFloraMap || m_staticCollidableFloraMap->getWidth()==0;
	if (emptyFloraMaps)
	{
		const float floraTileWidthInMeters=16.0f;
		int mapWidthInFlora = int(mapWidthInMeters / floraTileWidthInMeters);

		data.staticCollidableFloraMap = new PackedIntegerMap(mapWidthInFlora, mapWidthInFlora, 0);

		const float fixedPointResolution = 1.0f / 32.0f;
		data.staticCollidableFloraHeightMap = new PackedFixedPointMap(mapWidthInFlora, mapWidthInFlora, fixedPointResolution, 0);
	}
	else
	{
		data.staticCollidableFloraMap       = m_staticCollidableFloraMap;
		data.staticCollidableFloraHeightMap = m_staticCollidableFloraHeightMap;
	}

	ProceduralTerrainAppearanceTemplate::write (iff, data);

	if (emptyFloraMaps)
	{
		delete data.staticCollidableFloraMap;
		delete data.staticCollidableFloraHeightMap;
	}
}

//-------------------------------------------------------------------

BOOL TerrainEditorDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	ApplyPropertyViewChanges ();

	Iff iff (10000);
	save (iff, lpszPathName);

	FatalSetThrowExceptions (true);
	try
	{
		IGNORE_RETURN (iff.write (lpszPathName));
	}
	catch (FatalException& e)
	{
		MessageBox(0, e.getMessage(), "Error saving terrain file.", MB_ICONSTOP);
	}
	FatalSetThrowExceptions (false);

	return TRUE;
}

//===================================================================
//===================================================================
// Propery Frame

void TerrainEditorDoc::SetPropertyView (CRuntimeClass* cls, PropertyView::ViewData* vd)
{
	if (propertyFrame)
		propertyFrame->ChangeView (cls, vd);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::ApplyPropertyViewChanges ()
{
	if (propertyFrame)
		propertyFrame->ApplyChanges ();
}

//-------------------------------------------------------------------

void TerrainEditorDoc::RefreshPropertyView ()
{
	if (propertyFrame)
		propertyFrame->RefreshView ();
}

//===================================================================
//===================================================================

//===================================================================
//===================================================================
// Affectors

void TerrainEditorDoc::OnNewAffectorColorConstant()
{
	// TODO: Add your command handler code here
	AffectorColorConstant* affector = new AffectorColorConstant ();
	affector->setName ("AffectorColorConstant");

	PackedRgb color;
	color.r = static_cast<uint8> (Random::random (0, 255));
	color.g = static_cast<uint8> (Random::random (0, 255));
	color.b = static_cast<uint8> (Random::random (0, 255));

	affector->setColor (color);

	//-- tell layer view about it
	NOT_NULL (layerFrame);
	safe_cast<LayerView*> (layerFrame->GetActiveView ())->addAffector (affector);

	UpdateAllViews (0);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnUpdateNewAffector(CCmdUI* pCmdUI)
{
	//-- can I add an affector?
	pCmdUI->Enable ((layerFrame && safe_cast<const LayerView*> (layerFrame->GetActiveView ())->canAddAffector ()) ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnNewAffectorHeightConstant()
{
	AffectorHeightConstant* affector = new AffectorHeightConstant ();
	affector->setName ("AffectorHeightConstant");

	//-- tell layer view about it
	NOT_NULL (layerFrame);
	safe_cast<LayerView*> (layerFrame->GetActiveView ())->addAffector (affector);

	UpdateAllViews (0);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnNewAffectorHeightFractal()
{
	if (terrainGenerator->getFractalGroup ().getNumberOfFamilies () == 0)
	{
		AfxMessageBox ("Please first define at least one fractal family.");
	}
	else
	{
		AffectorHeightFractal* affector = new AffectorHeightFractal ();
		affector->setName ("AffectorHeightFractal");
		affector->setFamilyId (terrainGenerator->getFractalGroup().getFamilyId (0));
		affector->setOperation (TGO_replace);
		affector->setScaleY (50.f);

		//-- tell layer view about it
		NOT_NULL (layerFrame);
		safe_cast<LayerView*> (layerFrame->GetActiveView ())->addAffector (affector);

		UpdateAllViews (0);
	}
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnNewAffectorHeightTerrace()
{
	AffectorHeightTerrace* affector = new AffectorHeightTerrace ();
	affector->setName ("AffectorHeightTerrace");

	//-- tell layer view about it
	NOT_NULL (layerFrame);
	safe_cast<LayerView*> (layerFrame->GetActiveView ())->addAffector (affector);

	UpdateAllViews (0);
}
//-------------------------------------------------------------------

void TerrainEditorDoc::OnNewAffectorShaderConstant()
{
	if (terrainGenerator->getShaderGroup ().getNumberOfFamilies () == 0)
	{
		AfxMessageBox ("Please first define at least one shader family.");
	}
	else
	{
		AffectorShaderConstant* affector = new AffectorShaderConstant ();
		affector->setName ("AffectorShaderConstant");
		affector->setFamilyId (terrainGenerator->getShaderGroup().getFamilyId (0));

		//-- tell layer view about it
		NOT_NULL (layerFrame);
		safe_cast<LayerView*> (layerFrame->GetActiveView ())->addAffector (affector);

		UpdateAllViews (0);
	}
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnNewAffectorShaderReplace()
{
	if (terrainGenerator->getShaderGroup ().getNumberOfFamilies () == 0)
	{
		AfxMessageBox ("Please first define at least one shader family.");
	}
	else
	{
		AffectorShaderReplace* affector = new AffectorShaderReplace ();
		affector->setName ("AffectorShaderReplace");
		affector->setSourceFamilyId (terrainGenerator->getShaderGroup().getFamilyId (0));
		affector->setDestinationFamilyId (terrainGenerator->getShaderGroup().getFamilyId (0));

		//-- tell layer view about it
		NOT_NULL (layerFrame);
		safe_cast<LayerView*> (layerFrame->GetActiveView ())->addAffector (affector);

		UpdateAllViews (0);
	}
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnNewAffectorExclude()
{
	AffectorExclude* affector = new AffectorExclude ();
	affector->setName ("AffectorExclude");

	//-- tell layer view about it
	NOT_NULL (layerFrame);
	safe_cast<LayerView*> (layerFrame->GetActiveView ())->addAffector (affector);

	UpdateAllViews (0);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnNewAffectorPassable()
{
	AffectorPassable* const affector = new AffectorPassable ();
	affector->setName ("AffectorPassable");

	//-- tell layer view about it
	NOT_NULL (layerFrame);
	safe_cast<LayerView*> (layerFrame->GetActiveView ())->addAffector (affector);

	UpdateAllViews (0);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::addAffectorRoad (const ArrayList<Vector2d>& newPointList)
{
	AffectorRoad* affector = new AffectorRoad ();
	affector->setName ("AffectorRoad");
	affector->copyPointList (newPointList);
	affector->setWidth (4.f);
	affector->setFamilyId (terrainGenerator->getShaderGroup().getFamilyId (0));

	//-- tell layer view about it
	NOT_NULL (layerFrame);
	safe_cast<LayerView*> (layerFrame->GetActiveView ())->addAffector (affector);

	UpdateAllViews (0);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::addAffectorRiver (const ArrayList<Vector2d>& newPointList)
{
	AffectorRiver* affector = new AffectorRiver ();
	affector->setName ("AffectorRiver");
	affector->copyPointList (newPointList);
	affector->setWidth (4.f);
	affector->setBankFamilyId (terrainGenerator->getShaderGroup().getFamilyId (0));
	affector->setBottomFamilyId (terrainGenerator->getShaderGroup().getFamilyId (0));

	//-- tell layer view about it
	NOT_NULL (layerFrame);
	safe_cast<LayerView*> (layerFrame->GetActiveView ())->addAffector (affector);

	UpdateAllViews (0);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::addAffectorRibbon (const ArrayList<Vector2d>& newPointList)
{
	AffectorRibbon* affector = new AffectorRibbon ();
	affector->setName ("AffectorRibbon");
	affector->copyPointList (newPointList);
	affector->generateEndCapPointList();
	affector->setWidth (4.f);
	affector->setTerrainShaderFamilyId (terrainGenerator->getShaderGroup().getFamilyId (0));

	//-- tell layer view about it
	NOT_NULL (layerFrame);
	safe_cast<LayerView*> (layerFrame->GetActiveView ())->addAffector (affector);

	UpdateAllViews (0);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnNewAffectorColorRampheight()
{
	AffectorColorRampHeight* affector = new AffectorColorRampHeight ();
	affector->setName ("AffectorColorRampHeight");
	affector->setLowHeight (0.f);
	affector->setHighHeight (20.f);

	//-- tell layer view about it
	NOT_NULL (layerFrame);
	safe_cast<LayerView*> (layerFrame->GetActiveView ())->addAffector (affector);

	UpdateAllViews (0);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnNewAffectorColorRampfractal()
{
	if (terrainGenerator->getFractalGroup ().getNumberOfFamilies () == 0)
	{
		AfxMessageBox ("Please first define at least one fractal family.");
	}
	else
	{
		AffectorColorRampFractal* affector = new AffectorColorRampFractal ();
		affector->setName ("AffectorColorRampFractal");
		affector->setOperation (TGO_replace);
		affector->setFamilyId (terrainGenerator->getFractalGroup().getFamilyId (0));

		//-- tell layer view about it
		NOT_NULL (layerFrame);
		safe_cast<LayerView*> (layerFrame->GetActiveView ())->addAffector (affector);

		UpdateAllViews (0);
	}
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnNewAffectorFloraStaticCollidableConstant()
{
	if (terrainGenerator->getFloraGroup ().getNumberOfFamilies () == 0)
	{
		AfxMessageBox ("Please first define at least one flora family.");
	}
	else
	{
		AffectorFloraStaticCollidableConstant* affector = new AffectorFloraStaticCollidableConstant ();
		affector->setName ("AffectorFloraCollidableConstant");

		affector->setFamilyId (terrainGenerator->getFloraGroup().getFamilyId (0));

		//-- tell layer view about it
		NOT_NULL (layerFrame);
		safe_cast<LayerView*> (layerFrame->GetActiveView ())->addAffector (affector);

		UpdateAllViews (0);
	}
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnNewAffectorFloraStaticNonCollidableConstant()
{
	if (terrainGenerator->getFloraGroup ().getNumberOfFamilies () == 0)
	{
		AfxMessageBox ("Please first define at least one flora family.");
	}
	else
	{
		AffectorFloraStaticNonCollidableConstant* affector = new AffectorFloraStaticNonCollidableConstant ();
		affector->setName ("AffectorFloraNonCollidableConstant");

		affector->setFamilyId (terrainGenerator->getFloraGroup().getFamilyId (0));

		//-- tell layer view about it
		NOT_NULL (layerFrame);
		safe_cast<LayerView*> (layerFrame->GetActiveView ())->addAffector (affector);

		UpdateAllViews (0);
	}
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnNewAffectorFloraDynamicNearConstant()
{
	if (terrainGenerator->getRadialGroup ().getNumberOfFamilies () == 0)
	{
		AfxMessageBox ("Please first define at least one radial family.");
	}
	else
	{
		AffectorFloraDynamicNearConstant* affector = new AffectorFloraDynamicNearConstant ();
		affector->setName ("AffectorRadialNearConstant");
		affector->setFamilyId (terrainGenerator->getRadialGroup ().getFamilyId (0));

		//-- tell layer view about it
		NOT_NULL (layerFrame);
		safe_cast<LayerView*> (layerFrame->GetActiveView ())->addAffector (affector);

		UpdateAllViews (0);
	}
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnNewAffectorFloraDynamicFarConstant()
{
	if (terrainGenerator->getRadialGroup ().getNumberOfFamilies () == 0)
	{
		AfxMessageBox ("Please first define at least one radial family.");
	}
	else
	{
		AffectorFloraDynamicFarConstant* affector = new AffectorFloraDynamicFarConstant ();
		affector->setName ("AffectorRadialFarConstant");
		affector->setFamilyId (terrainGenerator->getRadialGroup ().getFamilyId (0));

		//-- tell layer view about it
		NOT_NULL (layerFrame);
		safe_cast<LayerView*> (layerFrame->GetActiveView ())->addAffector (affector);

		UpdateAllViews (0);
	}
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnNewAffectorEnvironment()
{
	if (terrainGenerator->getEnvironmentGroup ().getNumberOfFamilies () == 0)
	{
		AfxMessageBox ("Please first define at least one environment family.");
	}
	else
	{
		AffectorEnvironment* affector = new AffectorEnvironment ();
		affector->setName ("AffectorEnvironment");
		affector->setFamilyId (terrainGenerator->getEnvironmentGroup ().getFamilyId (0));

		//-- tell layer view about it
		NOT_NULL (layerFrame);
		safe_cast<LayerView*> (layerFrame->GetActiveView ())->addAffector (affector);

		UpdateAllViews (0);
	}
}

//===================================================================
//===================================================================

//===================================================================
//===================================================================
// Boundaries

void TerrainEditorDoc::addBoundaryRectangle (const Rectangle2d& newRectangle)
{
	BoundaryRectangle* boundary = new BoundaryRectangle ();
	boundary->setName ("BoundaryRectangle");
	boundary->setRectangle (newRectangle);

	//-- tell layer view about it
	NOT_NULL (layerFrame);
	safe_cast<LayerView*> (layerFrame->GetActiveView ())->addBoundary (boundary);

	UpdateAllViews (0);

//	AfxMessageBox (m_bakedTerrain->hasWater (newRectangle) ? "water: yup" : "water: nope");
//	AfxMessageBox (m_bakedTerrain->hasSteepSlope (newRectangle) ? "slope: yup" : "slope: nope");
}

//-------------------------------------------------------------------

void TerrainEditorDoc::addBoundaryCircle (const Vector2d& newCenter, real newRadius)
{
	BoundaryCircle* boundary = new BoundaryCircle ();
	boundary->setName ("BoundaryCircle");
	boundary->setCircle (newCenter.x, newCenter.y, newRadius);

	//-- tell layer view about it
	NOT_NULL (layerFrame);
	safe_cast<LayerView*> (layerFrame->GetActiveView ())->addBoundary (boundary);

	UpdateAllViews (0);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::addBoundaryPolygon (const ArrayList<Vector2d>& newPointList)
{
	BoundaryPolygon* boundary = new BoundaryPolygon ();
	boundary->setName ("BoundaryPolygon");
	boundary->copyPointList (newPointList);

	//-- tell layer view about it
	NOT_NULL (layerFrame);
	safe_cast<LayerView*> (layerFrame->GetActiveView ())->addBoundary (boundary);

	UpdateAllViews (0);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::addBoundaryPolyline (const ArrayList<Vector2d>& newPointList)
{
	BoundaryPolyline* boundary = new BoundaryPolyline ();
	boundary->setName ("BoundaryPolyline");
	boundary->copyPointList (newPointList);

	//-- tell layer view about it
	NOT_NULL (layerFrame);
	safe_cast<LayerView*> (layerFrame->GetActiveView ())->addBoundary (boundary);

	UpdateAllViews (0);
}

//===================================================================
//===================================================================

//===================================================================
//===================================================================
// Filters

void TerrainEditorDoc::OnNewFilterFractal()
{
	if (terrainGenerator->getFractalGroup ().getNumberOfFamilies () == 0)
	{
		AfxMessageBox ("Please first define at least one fractal family.");
	}
	else
	{
		FilterFractal* filter = new FilterFractal ();
		filter->setName ("FilterFractal");
		filter->setLowFractalLimit (CONST_REAL (0.45));
		filter->setHighFractalLimit (CONST_REAL (0.55));
		filter->setFamilyId (terrainGenerator->getFractalGroup().getFamilyId (0));

		//-- tell layer view about it
		NOT_NULL (layerFrame);
		safe_cast<LayerView*> (layerFrame->GetActiveView ())->addFilter (filter);

		UpdateAllViews (0);
	}
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnNewFilterShader()
{
	if (terrainGenerator->getShaderGroup ().getNumberOfFamilies()>0)
	{
		FilterShader* filter = new FilterShader ();
		filter->setName ("FilterShader");
		filter->setFamilyId (terrainGenerator->getShaderGroup ().getFamilyId (0));

		//-- tell layer view about it
		NOT_NULL (layerFrame);
		safe_cast<LayerView*> (layerFrame->GetActiveView ())->addFilter (filter);

		UpdateAllViews (0);
	}
	else
	{
		AfxMessageBox("Please create a shader family before using this filter.");
	}
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnNewFilterBitmap()
{
	if (terrainGenerator->getBitmapGroup ().getNumberOfFamilies () == 0)
	{
		AfxMessageBox ("Please first define at least one bitmap family.");
	}
	else if(!terrainGenerator->getBitmapGroup().hasFamily(terrainGenerator->getBitmapGroup().getFamilyId (0)))
	{
		AfxMessageBox ("First Family not defined yet");
	}
	else if(terrainGenerator->getBitmapGroup().getFamilyBitmap(terrainGenerator->getBitmapGroup().getFamilyId (0)) == NULL)
	{
		AfxMessageBox ("bitmap not assigned to first family yet");
	}
	else
	{
		FilterBitmap* filter = new FilterBitmap ();
		filter->setName ("FilterBitmap");
		filter->setLowBitmapLimit (CONST_REAL (0.45));
		filter->setHighBitmapLimit (CONST_REAL (0.55));
		filter->setFamilyId (terrainGenerator->getBitmapGroup().getFamilyId (0));
		filter->setGain(CONST_REAL(0.0));

		//-- tell layer view about it
		NOT_NULL (layerFrame);
		safe_cast<LayerView*> (layerFrame->GetActiveView ())->addFilter (filter);

		UpdateAllViews (0);
	}
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnUpdateNewFilter(CCmdUI* pCmdUI)
{
	//-- can I add a filter
	pCmdUI->Enable ((layerFrame && safe_cast<const LayerView*> (layerFrame->GetActiveView ())->canAddFilter ()) ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnNewFilterHeight()
{
	FilterHeight* filter = new FilterHeight ();
	filter->setName ("FilterHeight");
	filter->setLowHeight (CONST_REAL (10));
	filter->setHighHeight (CONST_REAL (20));

	//-- tell layer view about it
	NOT_NULL (layerFrame);
	safe_cast<LayerView*> (layerFrame->GetActiveView ())->addFilter (filter);

	UpdateAllViews (0);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnNewFilterSlope()
{
	FilterSlope* filter = new FilterSlope ();
	filter->setName ("FilterSlope");
	filter->setMinimumAngle (PI_OVER_4);
	filter->setMaximumAngle (PI_OVER_2);

	//-- tell layer view about it
	NOT_NULL (layerFrame);
	safe_cast<LayerView*> (layerFrame->GetActiveView ())->addFilter (filter);

	UpdateAllViews (0);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnNewFilterDirection()
{
	FilterDirection* filter = new FilterDirection ();
	filter->setName ("FilterDirection");
	filter->setMinimumAngle (-PI_OVER_2);
	filter->setMaximumAngle (PI_OVER_2);

	//-- tell layer view about it
	NOT_NULL (layerFrame);
	safe_cast<LayerView*> (layerFrame->GetActiveView ())->addFilter (filter);

	UpdateAllViews (0);
}

//===================================================================
//===================================================================

//-------------------------------------------------------------------

CRuntimeClass* TerrainEditorDoc::getLayerRuntimeClass (TerrainEditorDoc::Item* item) const
{
	switch (item->type)
	{
	case Item::T_layer:
		return RUNTIME_CLASS (FormLayer);

	case Item::T_boundary:
		{
			const TerrainGenerator::Boundary* boundary = safe_cast<const TerrainGenerator::Boundary*> (item->layerItem);
			switch (boundary->getType ())
			{
			case TGBT_circle:
				return RUNTIME_CLASS (FormBoundaryCircle);

			case TGBT_rectangle:
				return RUNTIME_CLASS (FormBoundaryRect);

			case TGBT_polygon:
				return RUNTIME_CLASS (FormBoundaryPolygon);

			case TGBT_polyline:
				return RUNTIME_CLASS (FormBoundaryPolyline);
			}
		}
		break;

	case Item::T_filter:
		{
			const TerrainGenerator::Filter* filter = safe_cast<const TerrainGenerator::Filter*> (item->layerItem);
			switch (filter->getType ())
			{
			case TGFT_slope:
				return RUNTIME_CLASS (FormFilterSlope);

			case TGFT_direction:
				return RUNTIME_CLASS (FormFilterDirection);

			case TGFT_height:
				return RUNTIME_CLASS (FormFilterHeight);

			case TGFT_fractal:
				return RUNTIME_CLASS (FormFilterFractal);

			case TGFT_shader:
				return RUNTIME_CLASS (FormFilterShader);

			case TGFT_bitmap:
				return RUNTIME_CLASS (FormFilterBitmap);
			}
		}
		break;

	case Item::T_affector:
		{
			const TerrainGenerator::Affector* affector = safe_cast<const TerrainGenerator::Affector*> (item->layerItem);
			switch (affector->getType ())
			{
			case TGAT_heightTerrace:
				return RUNTIME_CLASS (FormAffectorHeightTerrace);

			case TGAT_heightConstant:
				return RUNTIME_CLASS (FormAffectorHeightConstant);

			case TGAT_heightFractal:
				return RUNTIME_CLASS (FormAffectorHeightFractal);

			case TGAT_colorConstant:
				return RUNTIME_CLASS (FormAffectorColorConstant);

			case TGAT_colorRampHeight:
				return RUNTIME_CLASS (FormAffectorColorRampHeight);

			case TGAT_colorRampFractal:
				return RUNTIME_CLASS (FormAffectorColorRampFractal);

			case TGAT_shaderConstant:
				return RUNTIME_CLASS (FormAffectorShaderConstant);

			case TGAT_shaderReplace:
				return RUNTIME_CLASS (FormAffectorShaderReplace);

			case TGAT_floraStaticCollidableConstant:
			case TGAT_floraStaticNonCollidableConstant:
				return RUNTIME_CLASS (FormAffectorFloraStaticConstant);

			case TGAT_floraDynamicNearConstant:
			case TGAT_floraDynamicFarConstant:
				return RUNTIME_CLASS (FormAffectorFloraDynamicConstant);

			case TGAT_exclude:
				break;

			case TGAT_passable:
				return RUNTIME_CLASS (FormAffectorPassable);

			case TGAT_environment:
				return RUNTIME_CLASS (FormAffectorEnvironment);

			case TGAT_road:
				return RUNTIME_CLASS (FormAffectorRoad);

			case TGAT_river:
				return RUNTIME_CLASS (FormAffectorRiver);

			case TGAT_ribbon:
				return RUNTIME_CLASS (FormAffectorRibbon);
			}
		}
		break;

	case Item::T_unknown:
		break;
	}

	return RUNTIME_CLASS (PropertyView);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::BuildFloraFamilyDropList (CComboBox& comboBox, int selectFamilyId) const
{
	comboBox.Clear ();
	CDC* dc = comboBox.GetDC ();

	int dropWidth = 0;

	int i;
	for (i = 0; i < terrainGenerator->getFloraGroup ().getNumberOfFamilies (); i++)
	{
		const int id = terrainGenerator->getFloraGroup ().getFamilyId (i);

		const CString string = terrainGenerator->getFloraGroup ().getFamilyName (id);
		IGNORE_RETURN (comboBox.InsertString (i, string));
		IGNORE_RETURN (comboBox.SetItemData (i, static_cast<DWORD> (id)));

		if (id == selectFamilyId)
			IGNORE_RETURN (comboBox.SetCurSel (i));

		const CSize size = dc->GetTextExtent (string);

		if (size.cx > dropWidth)
			dropWidth = size.cx;
	}

	comboBox.ReleaseDC (dc);

	comboBox.SetDroppedWidth (dropWidth);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::BuildShaderFamilyDropList (CComboBox& comboBox, int selectFamilyId) const
{
	comboBox.Clear ();
	CDC* dc = comboBox.GetDC ();

	int dropWidth = 0;

	int i;
	for (i = 0; i < terrainGenerator->getShaderGroup ().getNumberOfFamilies (); i++)
	{
		const int id = terrainGenerator->getShaderGroup ().getFamilyId (i);

		const CString string = terrainGenerator->getShaderGroup ().getFamilyName (id);
		IGNORE_RETURN (comboBox.InsertString (i, string));
		IGNORE_RETURN (comboBox.SetItemData (i, static_cast<DWORD> (id)));

		if (id == selectFamilyId)
			IGNORE_RETURN (comboBox.SetCurSel (i));

		const CSize size = dc->GetTextExtent (string);

		if (size.cx > dropWidth)
			dropWidth = size.cx;
	}

	comboBox.ReleaseDC (dc);

	comboBox.SetDroppedWidth (dropWidth);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::BuildRadialFamilyDropList (CComboBox& comboBox, int selectFamilyId) const
{
	comboBox.Clear ();
	CDC* dc = comboBox.GetDC ();

	int dropWidth = 0;

	int i;
	for (i = 0; i < terrainGenerator->getRadialGroup ().getNumberOfFamilies (); i++)
	{
		const int id = terrainGenerator->getRadialGroup ().getFamilyId (i);

		const CString string = terrainGenerator->getRadialGroup ().getFamilyName (id);
		IGNORE_RETURN (comboBox.InsertString (i, string));
		IGNORE_RETURN (comboBox.SetItemData (i, static_cast<DWORD> (id)));

		if (id == selectFamilyId)
			IGNORE_RETURN (comboBox.SetCurSel (i));

		const CSize size = dc->GetTextExtent (string);

		if (size.cx > dropWidth)
			dropWidth = size.cx;
	}

	comboBox.ReleaseDC (dc);

	comboBox.SetDroppedWidth (dropWidth);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::BuildEnvironmentFamilyDropList (CComboBox& comboBox, int selectFamilyId) const
{
	comboBox.Clear ();
	CDC* dc = comboBox.GetDC ();

	int dropWidth = 0;

	int i;
	for (i = 0; i < terrainGenerator->getEnvironmentGroup ().getNumberOfFamilies (); i++)
	{
		const int id = terrainGenerator->getEnvironmentGroup ().getFamilyId (i);

		const CString string = terrainGenerator->getEnvironmentGroup ().getFamilyName (id);
		IGNORE_RETURN (comboBox.InsertString (i, string));
		IGNORE_RETURN (comboBox.SetItemData (i, static_cast<DWORD> (id)));

		if (id == selectFamilyId)
			IGNORE_RETURN (comboBox.SetCurSel (i));

		const CSize size = dc->GetTextExtent (string);

		if (size.cx > dropWidth)
			dropWidth = size.cx;
	}

	comboBox.ReleaseDC (dc);

	comboBox.SetDroppedWidth (dropWidth);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::BuildFractalFamilyDropList (CComboBox& comboBox, int selectFamilyId) const
{
	comboBox.Clear ();
	CDC* dc = comboBox.GetDC ();

	int dropWidth = 0;

	int i;
	for (i = 0; i < terrainGenerator->getFractalGroup ().getNumberOfFamilies (); i++)
	{
		const int id = terrainGenerator->getFractalGroup ().getFamilyId (i);

		const CString string = terrainGenerator->getFractalGroup ().getFamilyName (id);
		IGNORE_RETURN (comboBox.InsertString (i, string));
		IGNORE_RETURN (comboBox.SetItemData (i, static_cast<DWORD> (id)));

		if (id == selectFamilyId)
			IGNORE_RETURN (comboBox.SetCurSel (i));

		const CSize size = dc->GetTextExtent (string);

		if (size.cx > dropWidth)
			dropWidth = size.cx;
	}

	comboBox.ReleaseDC (dc);

	comboBox.SetDroppedWidth (dropWidth);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::BuildBitmapFamilyDropList (CComboBox& comboBox, int selectFamilyId) const
{
	comboBox.Clear ();
	CDC* dc = comboBox.GetDC ();

	int dropWidth = 0;

	int i;
	for (i = 0; i < terrainGenerator->getBitmapGroup ().getNumberOfFamilies (); i++)
	{
		const int id = terrainGenerator->getBitmapGroup ().getFamilyId (i);

		const CString string = terrainGenerator->getBitmapGroup ().getFamilyName (id);
		IGNORE_RETURN (comboBox.InsertString (i, string));
		IGNORE_RETURN (comboBox.SetItemData (i, static_cast<DWORD> (id)));

		if (id == selectFamilyId)
			IGNORE_RETURN (comboBox.SetCurSel (i));

		const CSize size = dc->GetTextExtent (string);

		if (size.cx > dropWidth)
			dropWidth = size.cx;
	}

	comboBox.ReleaseDC (dc);

	comboBox.SetDroppedWidth (dropWidth);
}

//-------------------------------------------------------------------

void TerrainEditorDoc::clearConsoleMessage (void)
{
	consoleMessage = "";

	addConsoleMessage ("");
}

//-------------------------------------------------------------------

void TerrainEditorDoc::addConsoleMessage (const CString& newMessage)
{
	//-- update message
	consoleMessage += newMessage;

	if (consoleFrame)
		safe_cast<ConsoleFrame*> (consoleFrame)->setConsoleMessage (consoleMessage);
}

//-------------------------------------------------------------------

bool TerrainEditorDoc::canAddBoundary (void) const
{
	return layerFrame && safe_cast<const LayerView*> (layerFrame->GetActiveView ())->canAddBoundary ();
}

//-------------------------------------------------------------------

using namespace TerrainEditorParameters;

void TerrainEditorDoc::OnOptionsMapparameters()
{
	DialogMapParameters dlg;

	switch (static_cast<int> (mapWidthInMeters))
	{
	case 32768:  dlg.mapWidth = MW_32_32;  break;
	case 16384:  dlg.mapWidth = MW_16_16;  break;
	default:
	case 8192:   dlg.mapWidth = MW_8_8;    break;
	case 4096:   dlg.mapWidth = MW_4_4;    break;
	}

	switch (static_cast<int> (chunkWidthInMeters))
	{
	case 32:     dlg.chunkWidth = CW_32_32;  break;
	default:
	case 16:     dlg.chunkWidth = CW_16_16;  break;
	case 8:	     dlg.chunkWidth = CW_8_8;    break;
	case 4:	     dlg.chunkWidth = CW_4_4;    break;
	}

	switch (static_cast<int> (numberOfTilesPerChunk))
	{
	case 16:     dlg.tilesPerChunk = NOTPC_16;  break;
	case 8:      dlg.tilesPerChunk = NOTPC_8;   break;
	default:
	case 4:	     dlg.tilesPerChunk = NOTPC_4;   break;
	case 2:	     dlg.tilesPerChunk = NOTPC_2;   break;
	case 1:	     dlg.tilesPerChunk = NOTPC_1;   break;
	}

	switch (static_cast<int> (defaultShaderSize))
	{
	case 16:     dlg.shaderSize = SS_16;  break;
	case 8:      dlg.shaderSize = SS_8;  break;
	case 4:      dlg.shaderSize = SS_4;  break;
	default:
	case 2:      dlg.shaderSize = SS_2;  break;
	}

	dlg.m_globalWaterHeight = globalWaterTableHeight;
	dlg.m_globalWaterShader = globalWaterTableShaderTemplateName;
	dlg.m_legacyMap         = m_legacyMap;
	dlg.m_useGlobalWater    = useGlobalWaterTable;
	dlg.m_globalWaterShaderSize = globalWaterTableShaderSize;
	dlg.m_hours             = m_environmentCycleTime / 3600;
	dlg.m_minutes           = (m_environmentCycleTime % 3600) / 60;

	if (dlg.DoModal () == IDOK)
	{
		switch (dlg.mapWidth)
		{
		case MW_32_32:  mapWidthInMeters = 32768.f;  break;
		case MW_16_16:  mapWidthInMeters = 16384.f;  break;
		case MW_8_8:    mapWidthInMeters = 8192.f;   break;
		default:
		case MW_4_4:    mapWidthInMeters = 4096.f;   break;
		}

		switch (dlg.chunkWidth)
		{
		case CW_32_32:  chunkWidthInMeters = 32.f;  break;
		case CW_16_16:  chunkWidthInMeters = 16.f;  break;
		default:
		case CW_8_8:    chunkWidthInMeters = 8.f;  break;
		case CW_4_4:    chunkWidthInMeters = 4.f;  break;
		}

		switch (dlg.tilesPerChunk)
		{
		case NOTPC_16: numberOfTilesPerChunk = 16; break;
		case NOTPC_8:  numberOfTilesPerChunk = 8;  break;
		default:
		case NOTPC_4:  numberOfTilesPerChunk = 4;  break;
		case NOTPC_2:  numberOfTilesPerChunk = 2;  break;
		case NOTPC_1:  numberOfTilesPerChunk = 1;  break;
		}

		switch (dlg.shaderSize)
		{
		case SS_16: defaultShaderSize = 16.f;  break;
		case SS_8:  defaultShaderSize = 8.f;   break;
		case SS_4:  defaultShaderSize = 4.f;   break;
		default:
		case SS_2:  defaultShaderSize = 2.f;   break;
		}

		globalWaterTableHeight             = dlg.m_globalWaterHeight;
		globalWaterTableShaderTemplateName = dlg.m_globalWaterShader;
		globalWaterTableShaderSize         = dlg.m_globalWaterShaderSize;
		m_legacyMap                        = dlg.m_legacyMap!=0;
		useGlobalWaterTable                = dlg.m_useGlobalWater != 0;
		m_environmentCycleTime             = dlg.m_hours * 3600 + dlg.m_minutes * 60;
		if (m_environmentCycleTime == 0)
			m_environmentCycleTime = 60;
	}
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnViewMapPreferences()
{
	DialogEditorPreferences dlg;
	dlg.m_minHeight = blackHeight;
	dlg.m_maxHeight = whiteHeight;
	if (dlg.DoModal () == IDOK)
	{
		blackHeight = dlg.m_minHeight;
		whiteHeight = dlg.m_maxHeight;

		if (whiteHeight < blackHeight)
			std::swap (whiteHeight, blackHeight);
	}
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnOptionsFloraparameters()
{
	DialogFloraParameters dlg;
	dlg.m_collidableMinimumDistance      = m_collidableMinimumDistance;
	dlg.m_collidableMaximumDistance      = m_collidableMaximumDistance;
	dlg.m_collidableTileSize             = m_collidableTileSize;
	dlg.m_collidableTileBorder           = m_collidableTileBorder;
	dlg.m_collidableSeed                 = m_collidableSeed;
	dlg.m_nonCollidableMinimumDistance   = m_nonCollidableMinimumDistance;
	dlg.m_nonCollidableMaximumDistance   = m_nonCollidableMaximumDistance;
	dlg.m_nonCollidableTileSize          = m_nonCollidableTileSize;
	dlg.m_nonCollidableTileBorder        = m_nonCollidableTileBorder;
	dlg.m_nonCollidableSeed              = m_nonCollidableSeed;
	dlg.m_radialMinimumDistance          = m_radialMinimumDistance;
	dlg.m_radialMaximumDistance          = m_radialMaximumDistance;
	dlg.m_radialTileSize                 = m_radialTileSize;
	dlg.m_radialTileBorder               = m_radialTileBorder;
	dlg.m_radialSeed                     = m_radialSeed;
	dlg.m_farRadialMinimumDistance       = m_farRadialMinimumDistance;
	dlg.m_farRadialMaximumDistance       = m_farRadialMaximumDistance;
	dlg.m_farRadialTileSize              = m_farRadialTileSize;
	dlg.m_farRadialTileBorder            = m_farRadialTileBorder;
	dlg.m_farRadialSeed                  = m_farRadialSeed;

	if (dlg.DoModal () == IDOK)
	{
		m_collidableMinimumDistance      = dlg.m_collidableMinimumDistance;
		m_collidableMaximumDistance      = dlg.m_collidableMaximumDistance;
		m_collidableTileSize             = dlg.m_collidableTileSize;
		m_collidableTileBorder           = dlg.m_collidableTileBorder;
		m_collidableSeed                 = dlg.m_collidableSeed;
		m_nonCollidableMinimumDistance   = dlg.m_nonCollidableMinimumDistance;
		m_nonCollidableMaximumDistance   = dlg.m_nonCollidableMaximumDistance;
		m_nonCollidableTileSize          = dlg.m_nonCollidableTileSize;
		m_nonCollidableTileBorder        = dlg.m_nonCollidableTileBorder;
		m_nonCollidableSeed              = dlg.m_nonCollidableSeed;
		m_radialMinimumDistance          = dlg.m_radialMinimumDistance;
		m_radialMaximumDistance          = dlg.m_radialMaximumDistance;
		m_radialTileSize                 = dlg.m_radialTileSize;
		m_radialTileBorder               = dlg.m_radialTileBorder;
		m_radialSeed                     = dlg.m_radialSeed;
		m_farRadialMinimumDistance       = dlg.m_farRadialMinimumDistance;
		m_farRadialMaximumDistance       = dlg.m_farRadialMaximumDistance;
		m_farRadialTileSize              = dlg.m_farRadialTileSize;
		m_farRadialTileBorder            = dlg.m_farRadialTileBorder;
		m_farRadialSeed                  = dlg.m_farRadialSeed;
	}
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnDebugAddallrules()
{
	if (terrainGenerator->getShaderGroup ().getNumberOfFamilies () == 0 ||
		terrainGenerator->getFloraGroup ().getNumberOfFamilies () == 0 ||
		terrainGenerator->getRadialGroup ().getNumberOfFamilies () == 0 ||
		terrainGenerator->getEnvironmentGroup ().getNumberOfFamilies () == 0 ||
		terrainGenerator->getFractalGroup ().getNumberOfFamilies () == 0)
	{
		AfxMessageBox ("Create families first!");
	}
	else
	{
		OnNewFilterDirection();
		OnNewFilterFractal();
		OnNewFilterHeight();
		OnNewFilterShader();
		OnNewFilterSlope();
		OnNewAffectorShaderConstant();
		OnNewAffectorShaderReplace();
		OnNewAffectorHeightConstant();
		OnNewAffectorHeightFractal();
		OnNewAffectorHeightTerrace();
		OnNewAffectorColorConstant();
		OnNewAffectorColorRampfractal();
		OnNewAffectorColorRampheight();
		OnNewAffectorFloraStaticCollidableConstant();
		OnNewAffectorFloraStaticNonCollidableConstant();
		OnNewAffectorFloraDynamicNearConstant();
		OnNewAffectorFloraDynamicFarConstant();
		OnNewAffectorExclude();
		OnNewAffectorPassable();
	}
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnToolsBaketerrain()
{
	mapFrame->bakeTerrain();
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnToolsBakeriversroads()
{
	mapFrame->updateRiversAndRoads();
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnToolsBakeflora()
{
	_bakeFlora(m_strPathName);
}

//-------------------------------------------------------------------
#pragma warning (disable: 4189)

bool TerrainEditorDoc::_bakeFlora(const char *terrainFile)
{
	char pathName[2048];

	const char *startupDirectory = Os::getProgramStartupDirectory();

	strcpy(pathName, terrainFile);
	char *dot = strrchr(pathName, '.');
	if (!dot)
	{
		return false;
	}
	strcpy(dot, ".tcf");

	char commandLine[2048];

	GetModuleFileName(0, commandLine, sizeof(commandLine));
	_normalizePath(commandLine, true);
	strcat(commandLine, "Turf_r.exe");

	if (0!=_access(commandLine, 0))
	{
		strcpy(commandLine, startupDirectory);
		_normalizePath(commandLine, false);
		strcat(commandLine, "Turf_r.exe");
		if (0!=_access(commandLine, 0))
		{
			MessageBox(mapFrame->getMapView()->m_hWnd, "Could not find Turf_r.exe!", "Error", MB_ICONERROR);
			return false;
		}
	}

	strcat(commandLine, " ");
	strcat(commandLine, pathName);
	strcat(commandLine, " /f");

	// -----------------------------------------------------

	bool returnValue=false;

	ProcessSpawner spawner;
	if (spawner.create(commandLine, startupDirectory, true))
	{
		CProgressDlg* const dlg = new CProgressDlg (CG_IDS_PROGRESS_CAPTION_FLORABAKE);
		dlg->Create();
		dlg->SetRange (0, 100);
		dlg->SetStep(1);
		dlg->SetPos(0);

		// ---------------------------------------------------------

		bool quit = false;

		while (!spawner.isFinished(200))
		{
			char buffer[2048];
			while (spawner.getOutputString(buffer, sizeof(buffer)))
			{
				buffer[sizeof(buffer)-1]=0;
				REPORT_PRINT(true, ("%s\n", buffer));

				int complete, total;
				if (sscanf(buffer, "%i/%i", &complete, &total)==2)
				{
					int progress = complete * 100 / total;
					dlg->SetPos(progress);
				}
			}

			if (dlg->CheckCancelButton())
			{
				quit = true;
				break;
			}
		}

		delete dlg;

		if (quit)
		{
			spawner.terminate(unsigned(-1));
			MessageBox(mapFrame->getMapView()->m_hWnd, "You cancelled flora baking.  Your flora is not fully baked!", "Warning", MB_ICONWARNING);
		}
		else
		{
			returnValue=_importFloraSampleFile(pathName);
		}
	}
	else
	{
		MessageBox(mapFrame->getMapView()->m_hWnd, "Could not launch Turf_r.exe!", "Error", MB_ICONERROR);
	}

	// -----------------------------------------------------

	return returnValue;
}

//-------------------------------------------------------------------

bool TerrainEditorDoc::_importFloraSampleFile(const char *sampleFileName)
{
	AppearanceTemplate *appearanceTemplate=0;

	{
		Iff iff (10000);
		save(iff);
		iff.allowNonlinearFunctions ();
		iff.goToTopOfForm ();

		appearanceTemplate = SamplerProceduralTerrainAppearanceTemplate::create(m_strPathName, &iff);
		if (!appearanceTemplate)
		{
			return false;
		}
	}

	bool returnValue=false;

	SamplerProceduralTerrainAppearanceTemplate *const samplerAppearanceTemplate = static_cast<SamplerProceduralTerrainAppearanceTemplate *>(appearanceTemplate);

	if (samplerAppearanceTemplate->loadStaticCollidableFloraFile(sampleFileName))
	{
		samplerAppearanceTemplate->commitStaticCollidableFloraSamples(0);


		const PackedIntegerMap *newStaticCollidableFloraMap = samplerAppearanceTemplate->getStaticCollidableFloraMap();
		const PackedFixedPointMap *newStaticCollidableFloraHeightMap = samplerAppearanceTemplate->getStaticCollidableFloraHeightMap();

		if (newStaticCollidableFloraMap && newStaticCollidableFloraHeightMap)
		{
			if (m_staticCollidableFloraMap)
			{
				delete m_staticCollidableFloraMap;
				m_staticCollidableFloraMap=0;
			}
			m_staticCollidableFloraMap = new PackedIntegerMap(*newStaticCollidableFloraMap);

			if (m_staticCollidableFloraHeightMap)
			{
				delete m_staticCollidableFloraHeightMap;
				m_staticCollidableFloraHeightMap=0;
			}
			m_staticCollidableFloraHeightMap = new PackedFixedPointMap(*newStaticCollidableFloraHeightMap);

			returnValue=true;
		}
	}

	delete samplerAppearanceTemplate;

	return returnValue;
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnDebugViewbakedterrain()
{
	CONSOLE_PRINT ("---- START: BakedTerrain water -----\r\n");

	const float mapWidthInMeters_2 = mapWidthInMeters * 0.5f;
	const float delta = 32.f;
	const int bufferSize = static_cast<int> (mapWidthInMeters / delta);
	char* const buffer = new char [bufferSize + 4];

	{
		float x;
		float z;
		for (z = mapWidthInMeters_2 - delta; z > -mapWidthInMeters_2; z -= delta)
		{
			int offset = 0;
			for (x = -mapWidthInMeters_2 + delta; x < mapWidthInMeters_2; x += delta)
				buffer [offset++] = m_bakedTerrain->getWater (Vector2d (x, z)) ? 'X' : '.';

			buffer [offset++] = '\r';
			buffer [offset++] = '\n';
			buffer [offset++] = 0;

			CONSOLE_PRINT (buffer);
		}
	}

	CONSOLE_PRINT ("---- STOP: BakedTerrain water -----\r\n");

	CONSOLE_PRINT ("---- START: BakedTerrain slope -----\r\n");

	{
		float x;
		float z;
		for (z = mapWidthInMeters_2 - delta; z > -mapWidthInMeters_2; z -= delta)
		{
			int offset = 0;
			for (x = -mapWidthInMeters_2 + delta; x < mapWidthInMeters_2; x += delta)
				buffer [offset++] = m_bakedTerrain->getSlope (Vector2d (x, z)) ? 'X' : '.';

			buffer [offset++] = '\r';
			buffer [offset++] = '\n';
			buffer [offset++] = 0;

			CONSOLE_PRINT (buffer);
		}
	}

	delete [] buffer;

	CONSOLE_PRINT ("---- STOP: BakedTerrain slope -----\r\n");
}

//-------------------------------------------------------------------

void TerrainEditorDoc::OnEditClearconsole()
{
	clearConsoleMessage ();
}

//-------------------------------------------------------------------

