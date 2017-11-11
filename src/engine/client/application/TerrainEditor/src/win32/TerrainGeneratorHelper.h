//
// TerrainGeneratorHelper.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_TerrainGeneratorHelper_H
#define INCLUDED_TerrainGeneratorHelper_H

//-------------------------------------------------------------------

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/ArrayList.h"
#include "sharedTerrain/TerrainGenerator.h"

class AffectorBoundaryPoly;
class MultiFractal;
class TerrainEditorDoc;
class AffectorRibbon;

//-------------------------------------------------------------------

class TerrainGeneratorHelper
{
public:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum LayerItemQueryType
	{
		LIQT_isLayer,
		LIQT_isBoundary,
		LIQT_isFilter,
		LIQT_isAffector,
		LIQT_hasFractal,
		LIQT_hasBitmap,
		LIQT_hasShader,
		LIQT_hasFloraStatic,
		LIQT_hasFloraDynamic,
		LIQT_hasBoundaries,
		LIQT_hasFilters,
		LIQT_hasAffectors,
		LIQT_hasSublayers,
		LIQT_hasInvertedBoundaries,
		LIQT_hasInvertedFilters,
		LIQT_hasWater,
		LIQT_isBoundaryCircle,
		LIQT_isBoundaryRectangle,
		LIQT_isBoundaryPolygon,
		LIQT_isBoundaryPolyline,
		LIQT_isFilterFractal,
		LIQT_isFilterBitmap,
		LIQT_isFilterSlope,
		LIQT_isFilterDirection,
		LIQT_isFilterHeight,
		LIQT_isFilterShader,
		LIQT_isAffectorExclude,
		LIQT_isAffectorPassable,
		LIQT_isAffectorHeightConstant,
		LIQT_isAffectorHeightFractal,
		LIQT_isAffectorHeightTerrace,
		LIQT_isAffectorColorConstant,
		LIQT_isAffectorColorRampHeight,
		LIQT_isAffectorColorRampFractal,
		LIQT_isAffectorShaderConstant,
		LIQT_isAffectorShaderReplace,
		LIQT_isAffectorFloraStaticCollidableConstant,
		LIQT_isAffectorFloraStaticNonCollidableConstant,
		LIQT_isAffectorFloraDynamicNearConstant,
		LIQT_isAffectorFloraDynamicFarConstant,
		LIQT_isAffectorRiver,
		LIQT_isAffectorRoad,
		LIQT_isAffectorEnvironment,
		LIQT_isAffectorRibbon,

		LIQT_COUNT
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	static const char* const LayerItemQueries [LIQT_COUNT];

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct ScanData
	{
	public:

		int numberOfBoundaries;
		int numberOfFilters;
		int numberOfAffectors;
		int numberOfLayers;

		int numberOfFractals;

		int numberOfWarnings;
		int numberOfErrors;
		int numberOfMissingFiles;

		ArrayList<int>                 uniqueShaderFamilyList;
		ArrayList<int>                 uniqueFloraFamilyList;
		ArrayList<int>                 uniqueRadialFamilyList;

	public:

		ScanData (void);

		void addFractal (const MultiFractal* multiFractal);

		const ArrayList<const MultiFractal*>& getUniqueFractalList () const;

	private:

		ArrayList<const MultiFractal*> uniqueFractalList;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct ProfileData
	{
	public:

		real    minimumLayerTime;
		real    maximumLayerTime;
		CString maximumLayerName;

	public:

		ProfileData (void);
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct OutputData
	{
	public:

		enum Mode
		{
			M_warning,
			M_console,
			M_find
		};

		enum Type
		{
			T_error,
			T_warning,
			T_info
		};

	public:

		//-- 0 for warning view, 1 for console, 2 for find
		Mode                               mode;

		//-- 0 error, 1 warning, 2 info
		Type                               type;
		CString*                           message;
		const TerrainGenerator::LayerItem* layerItem;

	public:

		OutputData (void);
		OutputData (Mode newMode, Type newType, CString* newMessage, const TerrainGenerator::LayerItem* newLayerItem);
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	TerrainGeneratorHelper (void);
	~TerrainGeneratorHelper (void);
	TerrainGeneratorHelper (const TerrainGeneratorHelper& rhs);
	TerrainGeneratorHelper& operator= (const TerrainGeneratorHelper& rhs);

private:

	static bool usesShaderFamily (const TerrainGenerator::Layer* layer, int familyId);
	static bool usesFloraFamily (const TerrainGenerator::Layer* layer, int familyId);
	static bool usesRadialFamily (const TerrainGenerator::Layer* layer, int familyId);
	static bool usesEnvironmentFamily (const TerrainGenerator::Layer* layer, int familyId);
	static bool usesFractalFamily (const TerrainGenerator::Layer* layer, int familyId);
	static bool usesBitmapFamily (const TerrainGenerator::Layer* layer, int familyId);

	static void fixupShader (TerrainEditorDoc* document, ShaderGroup& destinationShaderGroup, const ShaderGroup& sourceShaderGroup, TerrainGenerator::Affector* affector);
	static void fixupShader (TerrainEditorDoc* document, ShaderGroup& destinationShaderGroup, const ShaderGroup& sourceShaderGroup, TerrainGenerator::Filter* filter);
	static void fixupRadial (TerrainEditorDoc* document, RadialGroup& destinationRadialGroup, const RadialGroup& sourceRadialGroup, TerrainGenerator::Affector* affector);
	static void fixupFlora (TerrainEditorDoc* document, FloraGroup& destinationFloraGroup, const FloraGroup& sourceFloraGroup, TerrainGenerator::Affector* affector);
	static void fixupEnvironment (TerrainEditorDoc* document, EnvironmentGroup& destinationEnvironmentGroup, const EnvironmentGroup& sourceEnvironmentGroup, TerrainGenerator::Affector* affector);
	static void fixupFractal (TerrainEditorDoc* document, FractalGroup& destinationFractalGroup, const FractalGroup& sourceFractalGroup, TerrainGenerator::Affector* affector);
	static void fixupFractal (TerrainEditorDoc* document, FractalGroup& destinationFractalGroup, const FractalGroup& sourceFractalGroup, TerrainGenerator::Filter* affector);

public:

	static bool usesShaderFamily (const TerrainGenerator* generator, int familyId);
	static bool usesFloraFamily (const TerrainGenerator* generator, int familyId);
	static bool usesRadialFamily (const TerrainGenerator* generator, int familyId);
	static bool usesEnvironmentFamily (const TerrainGenerator* generator, int familyId);
	static bool usesFractalFamily (const TerrainGenerator* generator, int familyId);
	static bool usesBitmapFamily (const TerrainGenerator* generator, int familyId);


	static void saveShaderGroup (Iff& iff, const ShaderGroup& shaderGroup, const TerrainGenerator::Affector* affector);
	static void saveFloraGroup  (Iff& iff, const FloraGroup&  floraGroup,  const TerrainGenerator::Affector* affector);
	static void saveRadialGroup (Iff& iff, const RadialGroup& radialGroup, const TerrainGenerator::Affector* affector);
	static void saveEnvironmentGroup (Iff& iff, const EnvironmentGroup& environmentGroup, const TerrainGenerator::Affector* affector);
	static void saveFractalGroup (Iff& iff, const FractalGroup& fractalGroup, const TerrainGenerator::Affector* affector);

	static void saveShaderGroup (Iff& iff, const ShaderGroup& shaderGroup, const TerrainGenerator::Filter* filter);
	static void saveFloraGroup  (Iff& iff, const FloraGroup&  floraGroup,  const TerrainGenerator::Filter* filter);
	static void saveRadialGroup (Iff& iff, const RadialGroup& radialGroup, const TerrainGenerator::Filter* filter);
	static void saveEnvironmentGroup (Iff& iff, const EnvironmentGroup& environmentGroup, const TerrainGenerator::Filter* filter);
	static void saveFractalGroup (Iff& iff, const FractalGroup& fractalGroup, const TerrainGenerator::Filter* filter);

	static void saveShaderGroup (Iff& iff, const ShaderGroup& shaderGroup, const TerrainGenerator::Layer* layer);
	static void saveFloraGroup  (Iff& iff, const FloraGroup&  floraGroup,  const TerrainGenerator::Layer* layer);
	static void saveRadialGroup (Iff& iff, const RadialGroup& radialGroup, const TerrainGenerator::Layer* layer);
	static void saveEnvironmentGroup (Iff& iff, const EnvironmentGroup& environmentGroup, const TerrainGenerator::Layer* layer);
	static void saveFractalGroup (Iff& iff, const FractalGroup& fractalGroup, const TerrainGenerator::Layer* layer);

	static ShaderGroup* loadShaderGroup (Iff& iff);
	static FloraGroup*  loadFloraGroup  (Iff& iff);
	static RadialGroup* loadRadialGroup (Iff& iff);
	static EnvironmentGroup* loadEnvironmentGroup (Iff& iff);
	static FractalGroup* loadFractalGroup (Iff& iff);

	static void fixup (TerrainEditorDoc* document, ShaderGroup& destinationShaderGroup, FloraGroup& destinationFloraGroup, RadialGroup& destinationRadialGroup, EnvironmentGroup& destinationEnvironmentGroup, FractalGroup& destinationFractalGroup, const ShaderGroup& sourceShaderGroup, const FloraGroup& sourceFloraGroup, const RadialGroup& sourceRadialGroup, const EnvironmentGroup& sourceEnvironmentGroup, const FractalGroup& sourceFractalGroup, TerrainGenerator::Affector* affector);
	static void fixup (TerrainEditorDoc* document, ShaderGroup& destinationShaderGroup, FloraGroup& destinationFloraGroup, RadialGroup& destinationRadialGroup, EnvironmentGroup& destinationEnvironmentGroup, FractalGroup& destinationFractalGroup, const ShaderGroup& sourceShaderGroup, const FloraGroup& sourceFloraGroup, const RadialGroup& sourceRadialGroup, const EnvironmentGroup& sourceEnvironmentGroup, const FractalGroup& sourceFractalGroup, TerrainGenerator::Filter* filter);
	static void fixup (TerrainEditorDoc* document, ShaderGroup& destinationShaderGroup, FloraGroup& destinationFloraGroup, RadialGroup& destinationRadialGroup, EnvironmentGroup& destinationEnvironmentGroup, FractalGroup& destinationFractalGroup, const ShaderGroup& sourceShaderGroup, const FloraGroup& sourceFloraGroup, const RadialGroup& sourceRadialGroup, const EnvironmentGroup& sourceEnvironmentGroup, const FractalGroup& sourceFractalGroup, TerrainGenerator::Layer* layer);

	static void buildLocalWaterTableList (const TerrainGenerator* generator, ArrayList<const TerrainGenerator::Boundary*>& localWaterTableList);
	static void buildAffectorRibbonList (const TerrainGenerator* generator, ArrayList<const AffectorRibbon*>& affectorRibbonList);
	static void verify (const TerrainGenerator* generator, ArrayList<OutputData>& output);
	static void scan (const TerrainGenerator* generator, ArrayList<OutputData>& output);
	static void scanLayer (const TerrainGenerator* generator, const TerrainGenerator::Layer* layer, ArrayList<OutputData>& output);
	static void profile (const TerrainGenerator* generator, real totalChunkGenerationTime, real averageChunkGenerationTime, real minimumChunkGenerationTime, real maximumChunkGenerationTime, ArrayList<OutputData>& output);
	static void dump (const TerrainGenerator* generator, const TerrainGenerator::LayerItem* layerItem, ArrayList<OutputData>& output);
	static void query (const TerrainGenerator* generator, const ArrayList<LayerItemQueryType>& query, ArrayList<OutputData>& output);
	static void query (const TerrainGenerator* generator, const CString& substring, ArrayList<OutputData>& output);

	static void findShader (const TerrainGenerator* generator, int familyId, ArrayList<OutputData>& output);
	static void findFlora (const TerrainGenerator* generator, int familyId, ArrayList<OutputData>& output);
	static void findRadial (const TerrainGenerator* generator, int familyId, ArrayList<OutputData>& output);
	static void findEnvironment (const TerrainGenerator* generator, int familyId, ArrayList<OutputData>& output);
	static void findFractal (const TerrainGenerator* generator, int familyId, ArrayList<OutputData>& output);
	static void findBitmap (const TerrainGenerator* generator, int familyId, ArrayList<OutputData>& output);

	static const TerrainGenerator::Boundary* findBoundary (const TerrainGenerator* generator, const Vector2d& position_w);
	static void findBoundary (const TerrainGenerator* generator, ArrayList<TerrainGenerator::Boundary*>& boundaryList, const Vector2d& position_w, bool bIncludeInactive = false);
	static void findAffectorBoundaryPoly (const TerrainGenerator* generator, ArrayList<AffectorBoundaryPoly*>& affectorList);
	static void findBoundaryAndAffectorBoundaryPoly (const TerrainGenerator* generator, ArrayList<TerrainGenerator::LayerItem*>& itemList, const Vector2d& position_w, bool bIncludeInactive = false);
	static const Vector2d computeCenter (const TerrainGenerator::Layer* layer);

	static bool isPointInStrip(const Vector2d& point, const ArrayList<Vector2d>& pointList, const float width);
};

//-------------------------------------------------------------------

#endif
