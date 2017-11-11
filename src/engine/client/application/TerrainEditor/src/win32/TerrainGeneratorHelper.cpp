 //
// TerrainGeneratorHelper.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "TerrainGeneratorHelper.h"

#include "sharedMath/Line2d.h"
#include "sharedTerrain/AffectorHeight.h"
#include "sharedTerrain/AffectorShader.h"
#include "sharedTerrain/AffectorFloraStatic.h"
#include "sharedTerrain/AffectorFloraDynamic.h"
#include "sharedTerrain/AffectorColor.h"
#include "sharedTerrain/AffectorRibbon.h"
#include "sharedTerrain/AffectorRiver.h"
#include "sharedTerrain/AffectorRoad.h"
#include "sharedTerrain/Boundary.h"
#include "sharedTerrain/Filter.h"
#include "sharedUtility/FileName.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "TerrainEditorDoc.h"

#include <string>

//-------------------------------------------------------------------

//lint -e788  //-- enum not used within switch

//-------------------------------------------------------------------

const char* const TerrainGeneratorHelper::LayerItemQueries [LIQT_COUNT] =
{
	"isLayer",
	"isBoundary",
	"isFilter",
	"isAffector",
	"hasFractal",
	"hasBitmap",
	"hasShader",
	"hasFloraStatic",
	"hasFloraDynamic",
	"hasBoundaries",
	"hasFilters",
	"hasAffectors",
	"hasSublayers",
	"hasInvertedBoundaries",
	"hasInvertedFilters",
	"hasWater",
	"isBoundaryCircle",
	"isBoundaryRectangle",
	"isBoundaryPolygon",
	"isBoundaryPolyline",
	"isFilterFractal",
	"isFilterBitmap",
	"isFilterSlope",
	"isFilterDirection",
	"isFilterHeight",
	"isFilterShader",
	"isAffectorExclude",
	"isAffectorPassable",
	"isAffectorHeightConstant",
	"isAffectorHeightFractal",
	"isAffectorHeightTerrace",
	"isAffectorColorConstant",
	"isAffectorColorRampHeight",
	"isAffectorColorRampFractal",
	"isAffectorShaderConstant",
	"isAffectorShaderReplace",
	"isAffectorFloraStaticCollidableConstant",
	"isAffectorFloraStaticNonCollidableConstant",
	"isAffectorDynamicNearConstant",
	"isAffectorDynamicFarConstant",
	"isAffectorRiver",
	"isAffectorRoad",
	"isAffectorEnvironment",
	"isAffectorRibbon"
};

//-------------------------------------------------------------------

TerrainGeneratorHelper::ScanData::ScanData (void) :
	numberOfBoundaries (0),
	numberOfFilters (0),
	numberOfAffectors (0),
	numberOfLayers (0),
	numberOfFractals (0),
	numberOfWarnings (0),
	numberOfErrors (0),
	numberOfMissingFiles (0),
	uniqueShaderFamilyList (),
	uniqueFloraFamilyList (),
	uniqueRadialFamilyList (),
	uniqueFractalList ()
{
}



//-------------------------------------------------------------------

void TerrainGeneratorHelper::ScanData::addFractal (const MultiFractal* multiFractal)
{
	int i;
	for (i = 0; i < uniqueFractalList.getNumberOfElements (); ++i)
		if (*(uniqueFractalList [i]) == *multiFractal)
			return;

	uniqueFractalList.add (multiFractal);
}

//-------------------------------------------------------------------

const ArrayList<const MultiFractal*>& TerrainGeneratorHelper::ScanData::getUniqueFractalList () const
{
	return uniqueFractalList;
}

//-------------------------------------------------------------------

TerrainGeneratorHelper::ProfileData::ProfileData (void) :
	minimumLayerTime (100.f),
	maximumLayerTime (0),
	maximumLayerName ()
{
}

//-------------------------------------------------------------------

TerrainGeneratorHelper::OutputData::OutputData (void) :
	mode (M_console),
	type (T_info),
	message (0),
	layerItem (0)
{
}

//-------------------------------------------------------------------

TerrainGeneratorHelper::OutputData::OutputData (OutputData::Mode newMode, OutputData::Type newType, CString* newMessage, const TerrainGenerator::LayerItem* newLayerItem) :
	mode (newMode),
	type (newType),
	message (newMessage),
	layerItem (newLayerItem)
{
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::fixup (TerrainEditorDoc* document, ShaderGroup& destinationShaderGroup, FloraGroup& destinationFloraGroup, RadialGroup& destinationRadialGroup, EnvironmentGroup& destinationEnvironmentGroup, FractalGroup& destinationFractalGroup, const ShaderGroup& sourceShaderGroup, const FloraGroup& sourceFloraGroup, const RadialGroup& sourceRadialGroup, const EnvironmentGroup& sourceEnvironmentGroup, const FractalGroup& sourceFractalGroup, TerrainGenerator::Affector* affector)
{
	fixupShader (document, destinationShaderGroup, sourceShaderGroup, affector);
	fixupFlora  (document, destinationFloraGroup,  sourceFloraGroup,  affector);
	fixupRadial (document, destinationRadialGroup, sourceRadialGroup, affector);
	fixupEnvironment (document, destinationEnvironmentGroup, sourceEnvironmentGroup, affector);
	fixupFractal (document, destinationFractalGroup, sourceFractalGroup, affector);
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::fixup (TerrainEditorDoc* document, ShaderGroup& destinationShaderGroup, FloraGroup& destinationFloraGroup, RadialGroup& destinationRadialGroup, EnvironmentGroup& destinationEnvironmentGroup, FractalGroup& destinationFractalGroup, const ShaderGroup& sourceShaderGroup, const FloraGroup& sourceFloraGroup, const RadialGroup& sourceRadialGroup, const EnvironmentGroup& sourceEnvironmentGroup, const FractalGroup& sourceFractalGroup, TerrainGenerator::Filter* filter)
{
	UNREF (destinationFloraGroup);
	UNREF (destinationRadialGroup);
	UNREF (destinationEnvironmentGroup);
	UNREF (destinationFractalGroup);
	UNREF (sourceFloraGroup);
	UNREF (sourceRadialGroup);
	UNREF (sourceEnvironmentGroup);
	UNREF (sourceFractalGroup);

	fixupShader (document, destinationShaderGroup, sourceShaderGroup, filter);
	fixupFractal (document, destinationFractalGroup, sourceFractalGroup, filter);
}  //lint !e1764  //-- unused parameters could be declared const references

//-------------------------------------------------------------------

void TerrainGeneratorHelper::fixup (TerrainEditorDoc* document, ShaderGroup& destinationShaderGroup, FloraGroup& destinationFloraGroup, RadialGroup& destinationRadialGroup, EnvironmentGroup& destinationEnvironmentGroup, FractalGroup& destinationFractalGroup, const ShaderGroup& sourceShaderGroup, const FloraGroup& sourceFloraGroup, const RadialGroup& sourceRadialGroup, const EnvironmentGroup& sourceEnvironmentGroup, const FractalGroup& sourceFractalGroup, TerrainGenerator::Layer* layer)
{
	{
		//-- check filters
		int i;
		for (i = 0; i < layer->getNumberOfFilters (); i++)
			fixup (document, destinationShaderGroup, destinationFloraGroup, destinationRadialGroup, destinationEnvironmentGroup, destinationFractalGroup, sourceShaderGroup, sourceFloraGroup, sourceRadialGroup, sourceEnvironmentGroup, sourceFractalGroup, layer->getFilter (i));
	}

	{
		//-- check affectors
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
			fixup (document, destinationShaderGroup, destinationFloraGroup, destinationRadialGroup, destinationEnvironmentGroup, destinationFractalGroup, sourceShaderGroup, sourceFloraGroup, sourceRadialGroup, sourceEnvironmentGroup, sourceFractalGroup, layer->getAffector (i));
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			fixup (document, destinationShaderGroup, destinationFloraGroup, destinationRadialGroup, destinationEnvironmentGroup, destinationFractalGroup, sourceShaderGroup, sourceFloraGroup, sourceRadialGroup, sourceEnvironmentGroup, sourceFractalGroup, layer->getLayer (i));
	}
}

//-------------------------------------------------------------------

static void verifyBoundary (const char* name, const TerrainGenerator::Boundary* boundary, TerrainGeneratorHelper::ScanData& scanData, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	switch (boundary->getType ())
	{
	case TGBT_circle:
		{
		}
		break;

	case TGBT_rectangle:
		{
		}
		break;

	case TGBT_polyline:
		{
		}
		break;

	case TGBT_polygon:
		{
			const BoundaryPolygon* const boundaryPolygon = safe_cast<const BoundaryPolygon*> (boundary);

			if (boundaryPolygon->getNumberOfPoints () > 50)
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Boundary '%s' - too many edges in your boundary (%i > 50)", name, boundary->getName (), boundaryPolygon->getNumberOfPoints ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, boundaryPolygon));
			}

			if (boundaryPolygon->isLocalWaterTable ())
			{
				FileName filename (FileName::P_shader, boundaryPolygon->getLocalWaterTableShaderTemplateName () ? boundaryPolygon->getLocalWaterTableShaderTemplateName () : "");

				if (!TreeFile::exists (filename))
				{
					++scanData.numberOfErrors;

					CString* buffer = new CString;
					buffer->Format ("ERROR: Layer '%s'/Boundary '%s' - missing local water shader template %s", name, boundary->getName (), boundaryPolygon->getLocalWaterTableShaderTemplateName ());

					output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, boundaryPolygon));
				}
			}
		}
		break;
	}
}

//-------------------------------------------------------------------

static void verifyFilter (const char* name, const TerrainGenerator::Filter* filter, const ShaderGroup& shaderGroup, bool layerAffectsHeight, TerrainGeneratorHelper::ScanData& scanData, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	switch (filter->getType ())
	{
	case TGFT_height:
		{
			const FilterHeight* const filterHeight = safe_cast<const FilterHeight*> (filter);

			if (filterHeight->getLowHeight () > filterHeight->getHighHeight ())
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Filter '%s' - low height > high height", name, filter->getName ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, filterHeight));
			}

			if (fabs (filterHeight->getLowHeight () - filterHeight->getHighHeight ()) < 0.01f)
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Filter '%s' - low height (%1.2f) too close to high height (%1.2f), minimum deviation is 0.01", name, filter->getName (), filterHeight->getLowHeight (), filterHeight->getHighHeight ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, filterHeight));
			}
		}
		break;

	case TGFT_fractal:
		{
			const FilterFractal* const filterFractal = safe_cast<const FilterFractal*> (filter);

			if (filterFractal->getScaleY () == CONST_REAL (0))
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Filter '%s' - y scale is set to 0", name, filter->getName ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, filterFractal));
			}
		}
		break;

	case TGFT_slope:
		{
			if (layerAffectsHeight)
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Filter '%s' - can't filter by slope and have affectors that affect height in this layer or sublayers", name, filter->getName ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, filter));
			}

			const FilterSlope* const filterSlope  = safe_cast<const FilterSlope*> (filter);
			const real               minimumAngle = convertRadiansToDegrees (filterSlope->getMinimumAngle ());
			const real               maximumAngle = convertRadiansToDegrees (filterSlope->getMaximumAngle ());

			if (minimumAngle > maximumAngle)
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Filter '%s' - minimum angle > maximum angle", name, filter->getName ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, filter));
			}

			if (minimumAngle <= 1.f && maximumAngle >= 89.f)
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Filter '%s' - low is set too close to 0 and high is set too close to 90", name, filter->getName ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, filter));
			}
		}
		break;

	case TGFT_direction:
		{
			if (layerAffectsHeight)
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Filter '%s' - can't filter by direction and have affectors that affect height in this layer or sublayers", name, filter->getName ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, filter));
			}

			const FilterDirection* const filterDirection = safe_cast<const FilterDirection*> (filter);
			const real                   minimumAngle    = convertRadiansToDegrees (filterDirection->getMinimumAngle ());
			const real                   maximumAngle    = convertRadiansToDegrees (filterDirection->getMaximumAngle ());

			if (minimumAngle > maximumAngle)
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Filter '%s' - minimum angle > maximum angle", name, filter->getName ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, filter));
			}

			if (minimumAngle <= -178.f && maximumAngle >= 178.f)
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Filter '%s' - low is set too close to -180 and high is set too close to 180", name, filter->getName ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, filter));
			}
		}
		break;

	case TGFT_shader:
		{
			if (layerAffectsHeight)
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Filter '%s' - can't filter by shader and have affectors that affect height in this layer or sublayers", name, filter->getName ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, filter));
			}

			const FilterShader* const filterShader = safe_cast<const FilterShader*> (filter);

			if (!shaderGroup.hasFamily (filterShader->getFamilyId ()))
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Filter '%s' - invalid family", name, filter->getName ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, filter));
			}
		}
		break;
	}
}

//-------------------------------------------------------------------

static void verifyAffector (const char* name, const TerrainGenerator::Affector* affector, const ShaderGroup& shaderGroup, const FloraGroup& floraGroup, const RadialGroup& radialGroup, TerrainGeneratorHelper::ScanData& scanData, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	switch (affector->getType ())
	{
	case TGAT_colorRampHeight:
		{
			const AffectorColorRampHeight* const affectorColorRampHeight = safe_cast<const AffectorColorRampHeight*> (affector);

			if (affectorColorRampHeight->getLowHeight () > affectorColorRampHeight->getHighHeight ())
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Affector '%s' - low height > high height", name, affector->getName ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, affectorColorRampHeight));
			}

			if (fabs (affectorColorRampHeight->getLowHeight () - affectorColorRampHeight->getHighHeight ()) < 0.01f)
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Affector '%s' - low height (%1.2f) too close to high height (%1.2f), minimum deviation is 0.01", name, affector->getName (), affectorColorRampHeight->getLowHeight (), affectorColorRampHeight->getHighHeight ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, affectorColorRampHeight));
			}

			if (!TreeFile::exists (affectorColorRampHeight->getImageName ().c_str ()))
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Affector '%s' - missing image reference file %s", name, affector->getName (), affectorColorRampHeight->getImageName ().c_str ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, affectorColorRampHeight));
			}

		}
		break;

	case TGAT_colorRampFractal:
		{
			const AffectorColorRampFractal* const affectorColorRampFractal = safe_cast<const AffectorColorRampFractal*> (affector);

			if (!TreeFile::exists (affectorColorRampFractal->getImageName ().c_str ()))
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Affector '%s' - missing image reference file %s", name, affector->getName (), affectorColorRampFractal->getImageName ().c_str ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, affectorColorRampFractal));
			}
		}
		break;

	case TGAT_shaderConstant:
		{
			const AffectorShaderConstant* const affectorShaderConstant = safe_cast<const AffectorShaderConstant*> (affector);

			if (!shaderGroup.hasFamily (affectorShaderConstant->getFamilyId ()))
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Affector '%s' - invalid family", name, affector->getName ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, affector));
			}
		}
		break;

	case TGAT_shaderReplace:
		{
			const AffectorShaderReplace* const affectorShaderReplace = safe_cast<const AffectorShaderReplace*> (affector);

			if (!shaderGroup.hasFamily (affectorShaderReplace->getSourceFamilyId ()))
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Affector '%s' - invalid family", name, affector->getName ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, affector));
			}

			if (!shaderGroup.hasFamily (affectorShaderReplace->getDestinationFamilyId ()))
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Affector '%s' - invalid family", name, affector->getName ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, affector));
			}

			if (affectorShaderReplace->getSourceFamilyId () == affectorShaderReplace->getDestinationFamilyId ())
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Affector '%s' - has matching source and destination families", name, affector->getName ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, affector));
			}
		}
		break;

	case TGAT_floraStaticCollidableConstant:
	case TGAT_floraStaticNonCollidableConstant:
		{
			const AffectorFloraStatic* const affectorFloraStatic = safe_cast<const AffectorFloraStatic*> (affector);

			if (!floraGroup.hasFamily (affectorFloraStatic->getFamilyId ()))
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Affector '%s' - has an invalid family", name, affector->getName ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, affector));
			}
		}
		break;

	case TGAT_floraDynamicNearConstant:
	case TGAT_floraDynamicFarConstant:
		{
			const AffectorFloraDynamic* const affectorFloraDynamic = safe_cast<const AffectorFloraDynamic*> (affector);

			if (!radialGroup.hasFamily (affectorFloraDynamic->getFamilyId ()))
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Affector '%s' - has an invalid family", name, affector->getName ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, affector));
			}
		}
		break;

	case TGAT_river:
		{
			const AffectorRiver* const affectorRiver = safe_cast<const AffectorRiver*> (affector);

			FileName filename (FileName::P_shader, affectorRiver->getLocalWaterTableShaderTemplateName () ? affectorRiver->getLocalWaterTableShaderTemplateName () : "");

			if (affectorRiver->getHasLocalWaterTable () && !TreeFile::exists (filename))
			{
				++scanData.numberOfErrors;

				CString* buffer = new CString;
				buffer->Format ("ERROR: Layer '%s'/Affector '%s' - missing local water shader template %s", name, affector->getName (), affectorRiver->getLocalWaterTableShaderTemplateName ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, affector));
			}
		}
		break;

	default:
		break;
	}
}

//-------------------------------------------------------------------

static bool verifyLayerAffectsHeight (const TerrainGenerator::Layer* layer)
{
	{
		//-- check affectors
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
			if (layer->getAffector (i)->isActive () && layer->getAffector (i)->affectsHeight ())
				return true;
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			if (layer->getLayer (i)->isActive () && verifyLayerAffectsHeight (layer->getLayer (i)))
				return true;
	}

	return false;
}

//-------------------------------------------------------------------

static void verifyNumberOfAffectorHeight (const TerrainGenerator::Layer* layer, TerrainGeneratorHelper::ScanData& scanData, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	int count = 0;

	int i;
	for (i = 0; i < layer->getNumberOfAffectors (); i++)
	{
		if (layer->getAffector (i)->isActive ())
		{
			switch (layer->getAffector (i)->getType ())
			{
			case TGAT_heightTerrace:
			case TGAT_heightConstant:
			case TGAT_heightFractal:
				++count;
				break;

			default:
				break;
			}
		}
	}

	if (count > 1)
	{
		++scanData.numberOfErrors;

		CString* buffer = new CString;
		buffer->Format ("ERROR: Layer '%s' - more than one height affector found", layer->getName ());

		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, layer));
	}
}

static void verifyNumberOfAffectorColor (const TerrainGenerator::Layer* layer, TerrainGeneratorHelper::ScanData& scanData, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	int count = 0;

	int i;
	for (i = 0; i < layer->getNumberOfAffectors (); i++)
	{
		if (layer->getAffector (i)->isActive ())
		{
			switch (layer->getAffector (i)->getType ())
			{
			case TGAT_colorConstant:
				++count;
				break;

			default:
				break;
			}
		}
	}

	if (count > 1)
	{
		++scanData.numberOfErrors;

		CString* buffer = new CString;
		buffer->Format ("ERROR: Layer '%s' - more than one color affector found", layer->getName ());

		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, layer));
	}
}

static void verifyNumberOfAffectorShader (const TerrainGenerator::Layer* layer, TerrainGeneratorHelper::ScanData& scanData, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	bool hasShaderConstant      = false;
	int  shaderConstantFamilyId = 0;

	int count = 0;

	{
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
		{
			const TerrainGenerator::Affector* const affector = layer->getAffector (i);

			if (affector->isActive ())
			{
				switch (affector->getType ())
				{
				case TGAT_shaderConstant:
					hasShaderConstant      = true;
					shaderConstantFamilyId = safe_cast<const AffectorShaderConstant*> (affector)->getFamilyId ();
					++count;
					break;

				case TGAT_shaderReplace:
					//-- does not contribute to shader count per affector
					break;

				default:
					break;
				}
			}
		}
	}

	if (count > 1)
	{
		++scanData.numberOfErrors;

		CString* buffer = new CString;
		buffer->Format ("ERROR: Layer '%s' - more than one shader affector found", layer->getName ());

		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, layer));
	}
	else
	{
		if (hasShaderConstant)
		{
			int i;
			for (i = 0; i < layer->getNumberOfAffectors (); i++)
			{
				const TerrainGenerator::Affector* const affector = layer->getAffector (i);

				if (affector->isActive () && affector->getType () == TGAT_shaderReplace && safe_cast<const AffectorShaderReplace*> (affector)->getSourceFamilyId () == shaderConstantFamilyId)
				{
					++scanData.numberOfErrors;

					CString* buffer = new CString;
					buffer->Format ("ERROR: Layer '%s' - shader being set and then immediately replaced", layer->getName ());

					output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, layer));

					break;
				}
			}
		}
	}
}

static void verifyNumberOfAffectorFloraStaticCollidable (const TerrainGenerator::Layer* layer, TerrainGeneratorHelper::ScanData& scanData, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	int count = 0;

	int i;
	for (i = 0; i < layer->getNumberOfAffectors (); i++)
	{
		if (layer->getAffector (i)->isActive ())
		{
			switch (layer->getAffector (i)->getType ())
			{
			case TGAT_floraStaticCollidableConstant:
				++count;
				break;

			default:
				break;
			}
		}
	}

	if (count > 1)
	{
		++scanData.numberOfErrors;

		CString* buffer = new CString;
		buffer->Format ("ERROR: Layer '%s' - more than one flora affector found", layer->getName ());

		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, layer));
	}
}

static void verifyNumberOfAffectorFloraStaticNonCollidable (const TerrainGenerator::Layer* layer, TerrainGeneratorHelper::ScanData& scanData, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	int count = 0;

	int i;
	for (i = 0; i < layer->getNumberOfAffectors (); i++)
	{
		if (layer->getAffector (i)->isActive ())
		{
			switch (layer->getAffector (i)->getType ())
			{
			case TGAT_floraStaticNonCollidableConstant:
				++count;
				break;

			default:
				break;
			}
		}
	}

	if (count > 1)
	{
		++scanData.numberOfErrors;

		CString* buffer = new CString;
		buffer->Format ("ERROR: Layer '%s' - more than one flora affector found", layer->getName ());

		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, layer));
	}
}

static void verifyNumberOfAffectorFloraDynamicNear (const TerrainGenerator::Layer* layer, TerrainGeneratorHelper::ScanData& scanData, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	int count = 0;

	int i;
	for (i = 0; i < layer->getNumberOfAffectors (); i++)
	{
		if (layer->getAffector (i)->isActive ())
		{
			switch (layer->getAffector (i)->getType ())
			{
			case TGAT_floraDynamicNearConstant:
				++count;
				break;

			default:
				break;
			}
		}
	}

	if (count > 1)
	{
		++scanData.numberOfErrors;

		CString* buffer = new CString;
		buffer->Format ("ERROR: Layer '%s' - more than one radial affector found", layer->getName ());

		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, layer));
	}
}

static void verifyNumberOfAffectorFloraDynamicFar (const TerrainGenerator::Layer* layer, TerrainGeneratorHelper::ScanData& scanData, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	int count = 0;

	int i;
	for (i = 0; i < layer->getNumberOfAffectors (); i++)
	{
		if (layer->getAffector (i)->isActive ())
		{
			switch (layer->getAffector (i)->getType ())
			{
			case TGAT_floraDynamicFarConstant:
				++count;
				break;

			default:
				break;
			}
		}
	}

	if (count > 1)
	{
		++scanData.numberOfErrors;

		CString* buffer = new CString;
		buffer->Format ("ERROR: Layer '%s' - more than one radial affector found", layer->getName ());

		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_error, buffer, layer));
	}
}

//-------------------------------------------------------------------

static void verifyLayer (const TerrainGenerator::Layer* layer, const ShaderGroup& shaderGroup, const FloraGroup& floraGroup, const RadialGroup& radialGroup, TerrainGeneratorHelper::ScanData& scanData, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	{
		//-- check boundaries
		int i;
		for (i = 0; i < layer->getNumberOfBoundaries (); i++)
			if (layer->getBoundary (i)->isActive ())
				verifyBoundary (layer->getName (), layer->getBoundary (i), scanData, output);
	}

	{
		//-- check filters
		int i;
		for (i = 0; i < layer->getNumberOfFilters (); i++)
			if (layer->getFilter (i)->isActive ())
				verifyFilter (layer->getName (), layer->getFilter (i), shaderGroup, verifyLayerAffectsHeight (layer), scanData, output);
	}

	{
		//-- check affectors
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
			if (layer->getAffector (i)->isActive ())
				verifyAffector (layer->getName (), layer->getAffector (i), shaderGroup, floraGroup, radialGroup, scanData, output);

		verifyNumberOfAffectorHeight (layer, scanData, output);
		verifyNumberOfAffectorColor  (layer, scanData, output);
		verifyNumberOfAffectorShader (layer, scanData, output);
		verifyNumberOfAffectorFloraStaticCollidable  (layer, scanData, output);
		verifyNumberOfAffectorFloraStaticNonCollidable  (layer, scanData, output);
		verifyNumberOfAffectorFloraDynamicNear (layer, scanData, output);
		verifyNumberOfAffectorFloraDynamicFar (layer, scanData, output);
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			if (layer->getLayer (i)->isActive ())
				verifyLayer (layer->getLayer (i), shaderGroup, floraGroup, radialGroup, scanData, output);
	}
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::verify (const TerrainGenerator* generator, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	ScanData scanData;

	{
		//-- check sublayers
		int i;
		for (i = 0; i < generator->getNumberOfLayers (); i++)
			if (generator->getLayer (i)->isActive ())
				verifyLayer (generator->getLayer (i), generator->getShaderGroup (), generator->getFloraGroup (), generator->getRadialGroup (), scanData, output);
	}
}

//-------------------------------------------------------------------

static void buildLocalWaterTableList_s (const TerrainGenerator::Layer* layer, ArrayList<const TerrainGenerator::Boundary*>& localWaterTableList)
{
	{
		//-- check boundaries
		int i;
		for (i = 0; i < layer->getNumberOfBoundaries (); i++)
		{
			if (layer->getBoundary (i)->isActive () && layer->getBoundary (i)->getType () == TGBT_polygon)
			{
				const BoundaryPolygon* const boundaryPolygon = safe_cast<const BoundaryPolygon*> (layer->getBoundary (i));

				if (boundaryPolygon->isLocalWaterTable ())
					localWaterTableList.add (boundaryPolygon);
			}

			if (layer->getBoundary (i)->isActive () && layer->getBoundary (i)->getType () == TGBT_rectangle)
			{
				const BoundaryRectangle* const boundaryRectangle = safe_cast<const BoundaryRectangle*> (layer->getBoundary (i));

				if (boundaryRectangle->isLocalWaterTable ())
					localWaterTableList.add (boundaryRectangle);
			}
		}
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			if (layer->getLayer (i)->isActive ())
				buildLocalWaterTableList_s (layer->getLayer (i), localWaterTableList);
	}
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::buildLocalWaterTableList (const TerrainGenerator* generator, ArrayList<const TerrainGenerator::Boundary*>& localWaterTableList)
{
	{
		localWaterTableList.clear ();

		//-- check sublayers
		int i;
		for (i = 0; i < generator->getNumberOfLayers (); i++)
			if (generator->getLayer (i)->isActive ())
				buildLocalWaterTableList_s (generator->getLayer (i), localWaterTableList);
	}
}

//-------------------------------------------------------------------

static void buildAffectorRibbonList_s (const TerrainGenerator::Layer* layer, ArrayList<const AffectorRibbon*>& affectorRibbonList)
{
	{
		//-- check boundaries
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
		{
			if (layer->getAffector (i)->isActive () && layer->getAffector (i)->getType () == TGAT_ribbon)
			{
				const AffectorRibbon* const affectorRibbon = safe_cast<const AffectorRibbon*> (layer->getAffector (i));
				affectorRibbonList.add (affectorRibbon);
			}
		}
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			if (layer->getLayer (i)->isActive ())
				buildAffectorRibbonList_s (layer->getLayer (i), affectorRibbonList);
	}
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::buildAffectorRibbonList (const TerrainGenerator* generator, ArrayList<const AffectorRibbon*>& affectorRibbonList)
{
	{
		affectorRibbonList.clear ();

		//-- check sublayers
		int i;
		for (i = 0; i < generator->getNumberOfLayers (); i++)
			if (generator->getLayer (i)->isActive ())
				buildAffectorRibbonList_s (generator->getLayer (i), affectorRibbonList);
	}
}

//-------------------------------------------------------------------

static void scanAffector (const char* name, const TerrainGenerator::Affector* affector, const ShaderGroup& shaderGroup, const FloraGroup& floraGroup, const RadialGroup& radialGroup, TerrainGeneratorHelper::ScanData& scanData, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	UNREF (name);
	UNREF (output);

	++scanData.numberOfAffectors;

	verifyAffector (name, affector, shaderGroup, floraGroup, radialGroup, scanData, output);

	switch (affector->getType ())
	{
	case TGAT_heightTerrace:
		{
		}
		break;

	case TGAT_heightConstant:
		{
		}
		break;

	case TGAT_heightFractal:
		{
			++scanData.numberOfFractals;

#if 0
			{
				const AffectorHeightFractal* affectorHeightFractal = static_cast<const AffectorHeightFractal*> (affector);
				if (affectorHeightFractal->getNumberOfOctaves () > 2)
				{
					++scanData.numberOfWarnings;

					CString* buffer = new CString;
					buffer->Format ("Warning: Layer '%s'/Affector '%s' - %i octaves found", name, affector->getName (), affectorHeightFractal->getNumberOfOctaves ());

					output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_warning, buffer, affector));
				}

				if (affectorHeightFractal->getUseBias () || affectorHeightFractal->getUseGain ())
				{
					++scanData.numberOfWarnings;

					CString* buffer = new CString;
					buffer->Format ("Warning: Layer '%s'/Affector '%s' - using bias or gain", name, affector->getName ());

					output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_warning, buffer, affector));
				}

				scanData.addFractal (affectorHeightFractal->getMultiFractal ());
			}
#endif

		}
		break;

	case TGAT_colorConstant:
		{
		}
		break;

	case TGAT_colorRampFractal:
		{
			++scanData.numberOfFractals;

#if 0
			{
				const AffectorColorRampFractal* affectorColorRampFractal = static_cast<const AffectorColorRampFractal*> (affector);
				if (affectorColorRampFractal->getNumberOfOctaves () > 2)
				{
					++scanData.numberOfWarnings;

					CString* buffer = new CString;
					buffer->Format ("Warning: Layer '%s'/Affector '%s' - %i octaves found", name, affector->getName (), affectorColorRampFractal->getNumberOfOctaves ());

					output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_warning, buffer, affector));
				}

				if (affectorColorRampFractal->getUseBias () || affectorColorRampFractal->getUseGain ())
				{
					++scanData.numberOfWarnings;

					CString* buffer = new CString;
					buffer->Format ("Warning: Layer '%s'/Affector '%s' - using bias or gain", name, affector->getName ());

					output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_warning, buffer, affector));
				}

				scanData.addFractal (affectorColorRampFractal->getMultiFractal ());
			}
#endif

		}
		break;

	case TGAT_shaderConstant:
		{
			const AffectorShaderConstant* const affectorShaderConstant = safe_cast<const AffectorShaderConstant*> (affector);

			scanData.uniqueShaderFamilyList.addIfNotExist (affectorShaderConstant->getFamilyId ());
		}
		break;

	case TGAT_shaderReplace:
		{
			const AffectorShaderReplace* const affectorShaderReplace = safe_cast<const AffectorShaderReplace*> (affector);

			scanData.uniqueShaderFamilyList.addIfNotExist (affectorShaderReplace->getSourceFamilyId ());
			scanData.uniqueShaderFamilyList.addIfNotExist (affectorShaderReplace->getDestinationFamilyId ());
		}
		break;

	case TGAT_floraStaticCollidableConstant:
	case TGAT_floraStaticNonCollidableConstant:
		{
			const AffectorFloraStatic* const affectorFloraStatic = safe_cast<const AffectorFloraStatic*> (affector);

			scanData.uniqueFloraFamilyList.addIfNotExist (affectorFloraStatic->getFamilyId ());
		}
		break;

	case TGAT_floraDynamicNearConstant:
	case TGAT_floraDynamicFarConstant:
		{
			const AffectorFloraDynamic* const affectorFloraDynamic = safe_cast<const AffectorFloraDynamic*> (affector);

			scanData.uniqueRadialFamilyList.addIfNotExist (affectorFloraDynamic->getFamilyId ());
		}
		break;

	case TGAT_river:
		{
			const AffectorRiver* const affectorRiver = safe_cast<const AffectorRiver*> (affector);

			scanData.uniqueShaderFamilyList.addIfNotExist (affectorRiver->getBankFamilyId ());
			scanData.uniqueShaderFamilyList.addIfNotExist (affectorRiver->getBottomFamilyId ());
		}
		break;

	case TGAT_road:
		{
			const AffectorRoad* const affectorRoad = safe_cast<const AffectorRoad*> (affector);

			scanData.uniqueShaderFamilyList.addIfNotExist (affectorRoad->getFamilyId ());
		}
		break;

	case TGAT_ribbon:
		{
			//const AffectorRibbon* const affectorRibbon = safe_cast<const AffectorRibbon*> (affector);

			//scanData.uniqueShaderFamilyList.addIfNotExist (affectorRibbon->getFamilyId ());
		}

	default:
		break;
	}
}

static void scanFilter (const char* name, const TerrainGenerator::Filter* filter, const ShaderGroup& shaderGroup, bool layerAffectsHeight, TerrainGeneratorHelper::ScanData& scanData, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	UNREF (name);
	UNREF (output);

	++scanData.numberOfFilters;

	verifyFilter (name, filter, shaderGroup, layerAffectsHeight, scanData, output);

	switch (filter->getType ())
	{
	case TGFT_height:
		{
		}
		break;

	case TGFT_fractal:
		{
			++scanData.numberOfFractals;

			const FilterFractal* const filterFractal = safe_cast<const FilterFractal*> (filter);

			if (fabs (filterFractal->getLowFractalLimit () - filterFractal->getHighFractalLimit ()) < 0.05f)
			{
				++scanData.numberOfWarnings;

				CString* buffer = new CString;
				buffer->Format ("Warning: Layer '%s'/Filter '%s' - minimum threshold (%1.2f) too close to maximum threshold (%1.2f), minimum deviation is 0.05", name, filter->getName (), filterFractal->getLowFractalLimit (), filterFractal->getHighFractalLimit ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_warning, buffer, filter));
			}

			if (filterFractal->getLowFractalLimit () <= 0.01f && filterFractal->getHighFractalLimit () >= 0.99f)
			{
				++scanData.numberOfWarnings;

				CString* buffer = new CString;
				buffer->Format ("Warning: Layer '%s'/Filter '%s' - low is set to 0 and high is set to 1", name, filter->getName ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_warning, buffer, filter));
			}

#if 0
			{
				if (filterFractal->getNumberOfOctaves () > 2)
				{
					++scanData.numberOfWarnings;

					CString* buffer = new CString;
					buffer->Format ("Warning: Layer '%s'/Filter '%s' - %i octaves found", name, filter->getName (), filterFractal->getNumberOfOctaves ());

					output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_warning, buffer, filter));
				}

				if (filterFractal->getUseBias () || filterFractal->getUseGain ())
				{
					++scanData.numberOfWarnings;

					CString* buffer = new CString;
					buffer->Format ("Warning: Layer '%s'/Filter '%s' - using bias or gain", name, filter->getName ());

					output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_warning, buffer, filter));
				}

				scanData.addFractal (filterFractal->getMultiFractal ());
			}
#endif

		}
		break;

	case TGFT_slope:
		{
			const FilterSlope* const filterSlope  = safe_cast<const FilterSlope*> (filter);
			const real               minimumAngle = convertRadiansToDegrees (filterSlope->getMinimumAngle ());
			const real               maximumAngle = convertRadiansToDegrees (filterSlope->getMaximumAngle ());

			if (fabs (minimumAngle - maximumAngle) < 5.f)
			{
				++scanData.numberOfWarnings;

				CString* buffer = new CString;
				buffer->Format ("Warning: Layer '%s'/Filter '%s' - minimum angle (%1.2f) too close to maximum angle (%1.2f), minimum deviation is 5", name, filter->getName (), minimumAngle, maximumAngle);

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_warning, buffer, filter));
			}
		}
		break;

	case TGFT_direction:
		{
			const FilterDirection* const filterDirection = safe_cast<const FilterDirection*> (filter);
			const real                   minimumAngle    = convertRadiansToDegrees (filterDirection->getMinimumAngle ());
			const real                   maximumAngle    = convertRadiansToDegrees (filterDirection->getMaximumAngle ());

			if (fabs (maximumAngle - minimumAngle) < 5.f)
			{
				++scanData.numberOfWarnings;

				CString* buffer = new CString;
				buffer->Format ("Warning: Layer '%s'/Filter '%s' - minimum angle (%1.2f) too close to maximum angle (%1.2f), minimum deviation is 5", name, filter->getName (), minimumAngle, maximumAngle);

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_warning, buffer, filter));
			}
		}
		break;

	case TGFT_shader:
		{
			const FilterShader* const filterShader = safe_cast<const FilterShader*> (filter);

			scanData.uniqueShaderFamilyList.addIfNotExist (filterShader->getFamilyId ());
		}
		break;
	}
}

static void scanBoundary (const char* name, const TerrainGenerator::Boundary* boundary, TerrainGeneratorHelper::ScanData& scanData, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	UNREF (output);

	++scanData.numberOfBoundaries;

	verifyBoundary (name, boundary, scanData, output);

	switch (boundary->getType ())
	{
	case TGBT_circle:
		{
		}
		break;

	case TGBT_rectangle:
		{
		}
		break;

	case TGBT_polyline:
		{
		}
		break;

	case TGBT_polygon:
		{
			const BoundaryPolygon* const boundaryPolygon = safe_cast<const BoundaryPolygon*> (boundary);

			if (boundaryPolygon->getNumberOfPoints () > 10)
			{
				++scanData.numberOfWarnings;

				CString* buffer = new CString;
				buffer->Format ("Warning: Layer '%s'/Boundary '%s' - boundary has more than 10 edges (%i)", name, boundary->getName (), boundaryPolygon->getNumberOfPoints ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_warning, buffer, boundary));
			}
		}
		break;
	}
}

//-------------------------------------------------------------------

static void scanLayer (const TerrainGenerator::Layer* layer, const ShaderGroup& shaderGroup, const FloraGroup& floraGroup, const RadialGroup& radialGroup, TerrainGeneratorHelper::ScanData& scanData, ArrayList<TerrainGeneratorHelper::OutputData>& output, bool root)
{
	++scanData.numberOfLayers;

	int activeBoundaries = 0;
	{
		//-- check boundaries
		int i;
		for (i = 0; i < layer->getNumberOfBoundaries (); i++)
			if (layer->getBoundary (i)->isActive ())
			{
				++activeBoundaries;

				scanBoundary (layer->getName (), layer->getBoundary (i), scanData, output);
			}
	}

	int activeFilters = 0;
	{
		//-- check filters
		int i;
		for (i = 0; i < layer->getNumberOfFilters (); i++)
			if (layer->getFilter (i)->isActive ())
			{
				++activeFilters;

				scanFilter (layer->getName (), layer->getFilter (i), shaderGroup, verifyLayerAffectsHeight (layer), scanData, output);
			}
	}

	int activeAffectors = 0;
	{
		//-- check affectors
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
			if (layer->getAffector (i)->isActive ())
			{
				++activeAffectors;

				scanAffector (layer->getName (), layer->getAffector (i), shaderGroup, floraGroup, radialGroup, scanData, output);
			}
	}

	int activeLayers = 0;
	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			if (layer->getLayer (i)->isActive ())
			{
				++activeLayers;

				scanLayer (layer->getLayer (i), shaderGroup, floraGroup, radialGroup, scanData, output, false);
			}
	}

	{
		if (activeBoundaries == 0 && activeFilters == 0 && activeAffectors == 0 && activeLayers == 0)
		{
			++scanData.numberOfWarnings;

			CString* buffer = new CString;
			buffer->Format ("Warning: Layer '%s' - no active boundaries, filters, affectors, or layers found", layer->getName ());

			output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_warning, buffer, layer));
		}
		else
			if (activeBoundaries == 0 && activeFilters == 0 && activeAffectors == 0 && activeLayers != 0 && !root)
			{
				++scanData.numberOfWarnings;

				CString* buffer = new CString;
				buffer->Format ("Comment: Layer '%s' - layer only has sublayers, you should consider pushing them to the parent", layer->getName ());

				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_info, buffer, layer));
			}
			else
				if (activeBoundaries == 0 && activeFilters == 0 && activeAffectors == 1 && activeLayers == 0)
				{
					++scanData.numberOfWarnings;

					CString* buffer = new CString;
					buffer->Format ("Comment: Layer '%s' - layer has only one affector", layer->getName ());

					output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_warning, TerrainGeneratorHelper::OutputData::T_info, buffer, layer));
				}
	}
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::scanLayer (const TerrainGenerator* generator, const TerrainGenerator::Layer* layer, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	ScanData scanData;

	::scanLayer (layer, generator->getShaderGroup (), generator->getFloraGroup (), generator->getRadialGroup (), scanData, output, true);\

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			if (layer->getLayer (i)->isActive ())
				::scanLayer (layer->getLayer (i), generator->getShaderGroup (), generator->getFloraGroup (), generator->getRadialGroup (), scanData, output, true);
	}

	//-- print family data
	{
		CString* buffer = new CString;
		buffer->Format ("number of shader families used = %i", generator->getShaderGroup ().getNumberOfFamilies ());
		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));

		buffer = new CString;
		buffer->Format ("number of flora families used = %i", generator->getFloraGroup ().getNumberOfFamilies ());
		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));

		buffer = new CString;
		buffer->Format ("number of radial families used = %i", generator->getRadialGroup ().getNumberOfFamilies ());
		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
	}

	//-- add anything from scan data
	{
		CString* buffer = new CString;
		buffer->Format ("layers = %i, boundaries = %i, filters = %i, affectors = %i, fractals = %i, unique fractals = %i", scanData.numberOfLayers, scanData.numberOfBoundaries, scanData.numberOfFilters, scanData.numberOfAffectors, scanData.numberOfFractals, scanData.getUniqueFractalList ().getNumberOfElements ());
		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
	}

	//-- print warnings and errors
	{
		CString* buffer = new CString;
		buffer->Format ("%i errors, %i warnings", scanData.numberOfErrors, scanData.numberOfWarnings);
		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
	}
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::scan (const TerrainGenerator* generator, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	ScanData scanData;

	int unusedShaderFamilies = 0;

	{
		//-- go throught the shader group
		const ShaderGroup& shaderGroup = generator->getShaderGroup ();

		int i;
		for (i = 0; i < shaderGroup.getNumberOfFamilies (); i++)
		{
			const int familyId = shaderGroup.getFamilyId (i);

			if (usesShaderFamily (generator, familyId))
			{
				const int numberOfChildren = shaderGroup.getNumberOfChildren (i);

				if (numberOfChildren > 3)
				{
					++scanData.numberOfWarnings;

					CString* buffer = new CString;
					buffer->Format ("Warning: shader family '%s' has more than 3 children", shaderGroup.getFamilyName (shaderGroup.getFamilyId (i)));

					output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_warning, buffer, 0));
				}

				int j;
				for (j = 0; j < numberOfChildren; j++)
				{
					const ShaderGroup::FamilyChildData fcd = shaderGroup.getChild (i, j);

					FileName assetName (FileName::P_shader, fcd.shaderTemplateName);

					if (!TreeFile::exists (assetName))
					{
						++scanData.numberOfWarnings;

						CString* buffer = new CString;
						buffer->Format ("Warning: shader family '%s' has missing child '%s' (%s)", shaderGroup.getFamilyName (shaderGroup.getFamilyId (i)), fcd.shaderTemplateName, assetName.getString ());

						output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_warning, buffer, 0));
					}
				}
			}
			else
			{
				++scanData.numberOfWarnings;

				CString* buffer = new CString;
				buffer->Format ("Warning: shader family '%s' is not used", shaderGroup.getFamilyName (familyId));
				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_warning, buffer, 0));

				++unusedShaderFamilies;
			}
		}
	}

	int unusedFloraFamilies = 0;

	{
		//-- check flora group
		const FloraGroup& floraGroup = generator->getFloraGroup ();

		int i;
		for (i = 0; i < floraGroup.getNumberOfFamilies (); i++)
		{
			const int familyId = floraGroup.getFamilyId (i);

			if (usesFloraFamily (generator, familyId))
			{
				const int numberOfChildren = floraGroup.getNumberOfChildren (i);

				int j;
				for (j = 0; j < numberOfChildren; j++)
				{
					const FloraGroup::FamilyChildData fcd = floraGroup.getChild (i, j);

					FileName assetName (FileName::P_appearance, fcd.appearanceTemplateName, "");

					if (!TreeFile::exists (assetName))
					{
						++scanData.numberOfWarnings;

						CString* buffer = new CString;
						buffer->Format ("Warning: flora family '%s' has missing child '%s' (%s)", floraGroup.getFamilyName (floraGroup.getFamilyId (i)), fcd.appearanceTemplateName, assetName.getString ());

						output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_warning, buffer, 0));
					}

					if (fcd.shouldScale && fcd.minimumScale > fcd.maximumScale)
					{
						++scanData.numberOfWarnings;

						CString* buffer = new CString;
						buffer->Format ("Warning: flora family '%s' has child '%s' (%s) with minimumScale > maximumScale (%1.2f > %1.2f)", floraGroup.getFamilyName (floraGroup.getFamilyId (i)), fcd.appearanceTemplateName, assetName.getString (), fcd.minimumScale, fcd.maximumScale);

						output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_warning, buffer, 0));
					}
				}
			}
			else
			{
				++scanData.numberOfWarnings;

				CString* buffer = new CString;
				buffer->Format ("Warning: flora family '%s' is not used", floraGroup.getFamilyName (familyId));
				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_warning, buffer, 0));

				++unusedFloraFamilies;
			}
		}
	}

	int unusedRadialFamilies = 0;

	{
		//-- check radial group
		const RadialGroup& radialGroup = generator->getRadialGroup ();

		int i;
		for (i = 0; i < radialGroup.getNumberOfFamilies (); i++)
		{
			const int familyId = radialGroup.getFamilyId (i);

			if (usesRadialFamily (generator, familyId))
			{
				const int numberOfChildren = radialGroup.getNumberOfChildren (i);

				int j;
				for (j = 0; j < numberOfChildren; j++)
				{
					const RadialGroup::FamilyChildData fcd = radialGroup.getChild (i, j);

					FileName assetName (FileName::P_shader, fcd.shaderTemplateName);

					if (!TreeFile::exists (assetName))
					{
						++scanData.numberOfWarnings;

						CString* buffer = new CString;
						buffer->Format ("Warning: radial family '%s' has missing child '%s' (%s)", radialGroup.getFamilyName (radialGroup.getFamilyId (i)), fcd.shaderTemplateName, assetName.getString ());

						output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_warning, buffer, 0));
					}
				}
			}
			else
			{
				++scanData.numberOfWarnings;

				CString* buffer = new CString;
				buffer->Format ("Warning: radial family '%s' is not used", radialGroup.getFamilyName (familyId));
				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_warning, buffer, 0));

				++unusedRadialFamilies;
			}
		}
	}

	int unusedFractalFamilies = 0;

	{
		//-- check radial group
		const FractalGroup& fractalGroup = generator->getFractalGroup ();

		int i;
		for (i = 0; i < fractalGroup.getNumberOfFamilies (); i++)
		{
			const int familyId = fractalGroup.getFamilyId (i);

			if (!usesFractalFamily (generator, familyId))
			{
				++scanData.numberOfWarnings;

				CString* buffer = new CString;
				buffer->Format ("Warning: fractal family '%s' is not used", fractalGroup.getFamilyName (familyId));
				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_warning, buffer, 0));

				++unusedFractalFamilies;
			}
		}
	}

	int unusedEnvironmentFamilies = 0;

	{
		//-- check environment group
		const EnvironmentGroup& environmentGroup = generator->getEnvironmentGroup ();

		int i;
		for (i = 0; i < environmentGroup.getNumberOfFamilies (); i++)
		{
			const int familyId = environmentGroup.getFamilyId (i);

			if (!usesEnvironmentFamily (generator, familyId))
			{
				++scanData.numberOfWarnings;

				CString* buffer = new CString;
				buffer->Format ("Warning: environment family '%s' is not used", environmentGroup.getFamilyName (familyId));
				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_warning, buffer, 0));

				++unusedEnvironmentFamilies;
			}
		}
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < generator->getNumberOfLayers (); i++)
			if (generator->getLayer (i)->isActive ())
				::scanLayer (generator->getLayer (i), generator->getShaderGroup (), generator->getFloraGroup (), generator->getRadialGroup (), scanData, output, true);
	}

	//-- print family data
	{
		CString* buffer = new CString;
		buffer->Format ("number of shader families = %i, unused = %i", generator->getShaderGroup ().getNumberOfFamilies (), unusedShaderFamilies);
		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));

		buffer = new CString;
		buffer->Format ("number of flora families = %i, unused = %i", generator->getFloraGroup ().getNumberOfFamilies (), unusedFloraFamilies);
		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));

		buffer = new CString;
		buffer->Format ("number of radial families = %i, unused = %i", generator->getRadialGroup ().getNumberOfFamilies (), unusedRadialFamilies);
		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));

		buffer = new CString;
		buffer->Format ("number of fractal families = %i, unused = %i", generator->getFractalGroup ().getNumberOfFamilies (), unusedFractalFamilies);
		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));

		buffer = new CString;
		buffer->Format ("number of environment families = %i, unused = %i", generator->getEnvironmentGroup ().getNumberOfFamilies (), unusedEnvironmentFamilies);
		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
	}

	//-- add anything from scan data
	{
		CString* buffer = new CString;
		buffer->Format ("layers = %i, boundaries = %i, filters = %i, affectors = %i, fractals = %i, unique fractals = %i", scanData.numberOfLayers, scanData.numberOfBoundaries, scanData.numberOfFilters, scanData.numberOfAffectors, scanData.numberOfFractals, scanData.getUniqueFractalList ().getNumberOfElements ());
		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
	}

	//-- print warnings and errors
	{
		CString* buffer = new CString;
		buffer->Format ("%i errors, %i warnings", scanData.numberOfErrors, scanData.numberOfWarnings);
		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
	}
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::profile (const TerrainGenerator* generator, real totalChunkGenerationTime, real averageChunkGenerationTime, real minimumChunkGenerationTime, real maximumChunkGenerationTime, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	UNREF (generator);

	{
		CString* buffer = new CString;
		buffer->Format ("total chunk time = %1.3f seconds (for all visible chunks)", totalChunkGenerationTime);
		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
	}

	{
		CString* buffer = new CString;
		buffer->Format ("average chunk time = %1.3f seconds", averageChunkGenerationTime);
		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
	}

	{
		CString* buffer = new CString;
		buffer->Format ("minimum chunk time = %1.3f seconds", minimumChunkGenerationTime);
		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
	}

	{
		CString* buffer = new CString;
		buffer->Format ("maximum chunk time = %1.3f seconds", maximumChunkGenerationTime);
		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
	}
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::dump (const TerrainGenerator* generator, const TerrainGenerator::LayerItem* layerItem, ArrayList<OutputData>& output)
{
	{
		const ShaderGroup& shaderGroup = generator->getShaderGroup ();

		{
			CString* buffer = new CString;
			buffer->Format ("Shader families: %i", shaderGroup.getNumberOfFamilies ());
			output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
		}

		int i;
		for (i = 0; i < shaderGroup.getNumberOfFamilies (); i++)
		{
			{
				CString* buffer = new CString;
				buffer->Format ("  Family '%s' [id=%i]", shaderGroup.getFamilyName (shaderGroup.getFamilyId (i)), shaderGroup.getFamilyId (i));
				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
			}

			int j;
			for (j = 0; j < shaderGroup.getNumberOfChildren (i); j++)
			{
				const ShaderGroup::FamilyChildData fcd = shaderGroup.getChild (i, j);

				{
					CString* buffer = new CString;
					buffer->Format ("    Child '%s'", fcd.shaderTemplateName);
					output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
				}
			}
		}
	}

	{
		const FloraGroup& floraGroup = generator->getFloraGroup ();

		{
			CString* buffer = new CString;
			buffer->Format ("Flora families: %i", floraGroup.getNumberOfFamilies ());
			output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
		}

		int i;
		for (i = 0; i < floraGroup.getNumberOfFamilies (); i++)
		{
			{
				CString* buffer = new CString;
				buffer->Format ("  Family '%s' [id=%i]", floraGroup.getFamilyName (floraGroup.getFamilyId (i)), floraGroup.getFamilyId (i));
				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
			}

			int j;
			for (j = 0; j < floraGroup.getNumberOfChildren (i); j++)
			{
				const FloraGroup::FamilyChildData fcd = floraGroup.getChild (i, j);

				{
					CString* buffer = new CString;
					buffer->Format ("    Child '%s'", fcd.appearanceTemplateName);
					output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
				}
			}
		}
	}

	{
		const RadialGroup& radialGroup = generator->getRadialGroup ();

		{
			CString* buffer = new CString;
			buffer->Format ("Radial families: %i", radialGroup.getNumberOfFamilies ());
			output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
		}

		int i;
		for (i = 0; i < radialGroup.getNumberOfFamilies (); i++)
		{
			{
				CString* buffer = new CString;
				buffer->Format ("  Family '%s' [id=%i]", radialGroup.getFamilyName (radialGroup.getFamilyId (i)), radialGroup.getFamilyId (i));
				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
			}

			int j;
			for (j = 0; j < radialGroup.getNumberOfChildren (i); j++)
			{
				const RadialGroup::FamilyChildData fcd = radialGroup.getChild (i, j);

				{
					CString* buffer = new CString;
					buffer->Format ("    Child '%s'", fcd.shaderTemplateName);
					output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
				}
			}
		}
	}

	{
		const FractalGroup& fractalGroup = generator->getFractalGroup ();

		{
			CString* buffer = new CString;
			buffer->Format ("Fractal families: %i", fractalGroup.getNumberOfFamilies ());
			output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
		}

		int i;
		for (i = 0; i < fractalGroup.getNumberOfFamilies (); i++)
		{
			CString* buffer = new CString;
			buffer->Format ("  Family '%s' [id=%i]", fractalGroup.getFamilyName (fractalGroup.getFamilyId (i)), fractalGroup.getFamilyId (i));
			output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
		}
	}

	{
		const EnvironmentGroup& environmentGroup = generator->getEnvironmentGroup ();

		{
			CString* buffer = new CString;
			buffer->Format ("Environment families: %i", environmentGroup.getNumberOfFamilies ());
			output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
		}

		int i;
		for (i = 0; i < environmentGroup.getNumberOfFamilies (); i++)
		{
			CString* buffer = new CString;
			buffer->Format ("  Family '%s' [id=%i]", environmentGroup.getFamilyName (environmentGroup.getFamilyId (i)), environmentGroup.getFamilyId (i));
			output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
		}
	}

	switch (layerItem->getTag ())
	{
	case TAG_BPOL:
		{
			const BoundaryPolygon* const boundary = safe_cast<const BoundaryPolygon*> (layerItem);

			int i;
			for (i = 0; i < boundary->getNumberOfPoints (); ++i)
			{
				CString* buffer = new CString;
				buffer->Format ("%3i  %10.2f %10.2f", i, boundary->getPoint (i).x, boundary->getPoint (i).y);
				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
			}
		}
		break;

	case TAG_BPLN:
		{
			const BoundaryPolyline* const boundary = safe_cast<const BoundaryPolyline*> (layerItem);

			int i;
			for (i = 0; i < boundary->getNumberOfPoints (); ++i)
			{
				CString* buffer = new CString;
				buffer->Format ("%3i  %10.2f %10.2f", i, boundary->getPoint (i).x, boundary->getPoint (i).y);
				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
			}
		}
		break;

	case TAG_ARIV:
	case TAG_AROA:
		{
			const AffectorBoundaryPoly* const affector = safe_cast<const AffectorBoundaryPoly*> (layerItem);

			const HeightData& heightData = affector->getHeightData ();

			int i;
			for (i = 0; i < heightData.getNumberOfSegments (); ++i)
			{
				{
					const Vector2d start = affector->getPoint (i);
					const Vector2d end   = affector->getPoint (i + 1);

					CString* buffer = new CString;
					buffer->Format ("%3i:  <%1.2f, %1.2f>  <%1.2f, %1.2f>", i, start.x, start.y, end.x, end.y);
					output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
				}

				int j;
				for (j = 0; j < heightData.getNumberOfPoints (i); ++j)
				{
					const Vector point = heightData.getPoint (i, j);

					CString* buffer = new CString;
					buffer->Format ("    %3i %10.2f %10.2f %10.2f", j, point.x, point.y, point.z);
					output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_console, TerrainGeneratorHelper::OutputData::T_info, buffer, 0));
				}
			}
		}
		break;
	case TAG_ARIB:
		{
		}
		break;

	default:
		break;
	}
}

//-------------------------------------------------------------------

static void queryBoundary (const TerrainGenerator::Boundary* boundary, const ArrayList<TerrainGeneratorHelper::LayerItemQueryType>& query, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	bool add = false;

	if (query.existsInList (TerrainGeneratorHelper::LIQT_isBoundary))
		add = true;

	switch (boundary->getType ())
	{
	case TGBT_circle:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isBoundaryCircle))
				add = true;
		}
		break;

	case TGBT_rectangle:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isBoundaryRectangle))
				add = true;

			const BoundaryRectangle* const boundaryRectangle = safe_cast<const BoundaryRectangle*> (boundary);

			if (query.existsInList (TerrainGeneratorHelper::LIQT_hasWater) && boundaryRectangle->isLocalWaterTable ())
				add = true;
		}
		break;

	case TGBT_polygon:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isBoundaryPolygon))
				add = true;

			const BoundaryPolygon* const boundaryPolygon = safe_cast<const BoundaryPolygon*> (boundary);

			if (query.existsInList (TerrainGeneratorHelper::LIQT_hasWater) && boundaryPolygon->isLocalWaterTable ())
				add = true;
		}
		break;

	case TGBT_polyline:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isBoundaryPolyline))
				add = true;
		}
		break;
	}

	if (add)
	{
		CString* buffer = new CString;
		buffer->Format ("%s", boundary->getName ());
		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_find, TerrainGeneratorHelper::OutputData::T_info, buffer, boundary));
	}
}

//-------------------------------------------------------------------

static void queryFilter (const TerrainGenerator::Filter* filter, const ArrayList<TerrainGeneratorHelper::LayerItemQueryType>& query, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	bool add = false;

	if (query.existsInList (TerrainGeneratorHelper::LIQT_isFilter))
		add = true;

	switch (filter->getType ())
	{
	case TGFT_height:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isFilterHeight))
				add = true;
		}
		break;

	case TGFT_fractal:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isFilterFractal))
				add = true;

			if (query.existsInList (TerrainGeneratorHelper::LIQT_hasFractal))
				add = true;
		}
		break;

	case TGFT_slope:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isFilterSlope))
				add = true;
		}
		break;

	case TGFT_direction:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isFilterDirection))
				add = true;
		}
		break;

	case TGFT_shader:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isFilterShader))
				add = true;

			if (query.existsInList (TerrainGeneratorHelper::LIQT_hasShader))
				add = true;
		}
		break;
	}

	if (add)
	{
		CString* buffer = new CString;
		buffer->Format ("%s", filter->getName ());
		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_find, TerrainGeneratorHelper::OutputData::T_info, buffer, filter));
	}
}

//-------------------------------------------------------------------

static void queryAffector (const TerrainGenerator::Affector* affector, const ArrayList<TerrainGeneratorHelper::LayerItemQueryType>& query, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	bool add = false;

	if (query.existsInList (TerrainGeneratorHelper::LIQT_isAffector))
		add = true;

	switch (affector->getType ())
	{
	case TGAT_heightTerrace:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isAffectorHeightTerrace))
				add = true;
		}
		break;

	case TGAT_heightConstant:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isAffectorHeightConstant))
				add = true;
		}
		break;

	case TGAT_heightFractal:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isAffectorHeightFractal))
				add = true;

			if (query.existsInList (TerrainGeneratorHelper::LIQT_hasFractal))
				add = true;
		}
		break;

	case TGAT_colorConstant:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isAffectorColorConstant))
				add = true;
		}
		break;

	case TGAT_colorRampHeight:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isAffectorColorRampHeight))
				add = true;
		}
		break;

	case TGAT_colorRampFractal:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isAffectorColorRampFractal))
				add = true;

			if (query.existsInList (TerrainGeneratorHelper::LIQT_hasFractal))
				add = true;
		}
		break;

	case TGAT_shaderConstant:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isAffectorShaderConstant))
				add = true;

			if (query.existsInList (TerrainGeneratorHelper::LIQT_hasShader))
				add = true;
		}
		break;

	case TGAT_shaderReplace:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isAffectorShaderReplace))
				add = true;

			if (query.existsInList (TerrainGeneratorHelper::LIQT_hasShader))
				add = true;
		}
		break;

	case TGAT_floraStaticCollidableConstant:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isAffectorFloraStaticCollidableConstant))
				add = true;

			if (query.existsInList (TerrainGeneratorHelper::LIQT_hasFloraStatic))
				add = true;
		}
		break;

	case TGAT_floraStaticNonCollidableConstant:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isAffectorFloraStaticNonCollidableConstant))
				add = true;

			if (query.existsInList (TerrainGeneratorHelper::LIQT_hasFloraStatic))
				add = true;
		}
		break;

	case TGAT_floraDynamicNearConstant:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isAffectorFloraDynamicNearConstant))
				add = true;

			if (query.existsInList (TerrainGeneratorHelper::LIQT_hasFloraDynamic))
				add = true;
		}
		break;

	case TGAT_floraDynamicFarConstant:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isAffectorFloraDynamicFarConstant))
				add = true;

			if (query.existsInList (TerrainGeneratorHelper::LIQT_hasFloraDynamic))
				add = true;
		}
		break;

	case TGAT_exclude:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isAffectorExclude))
				add = true;
		}
		break;

	case TGAT_passable:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isAffectorPassable))
				add = true;
		}
		break;

	case TGAT_environment:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isAffectorEnvironment))
				add = true;
		}
		break;

	case TGAT_road:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isAffectorRoad))
				add = true;
		}
		break;

	case TGAT_river:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isAffectorRiver))
				add = true;
		}
		break;

	case TGAT_ribbon:
		{
			if (query.existsInList (TerrainGeneratorHelper::LIQT_isAffectorRibbon))
			{
				add = true;
			}
		}
	}

	if (add)
	{
		CString* buffer = new CString;
		buffer->Format ("%s", affector->getName ());
		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_find, TerrainGeneratorHelper::OutputData::T_info, buffer, affector));
	}
}

//-------------------------------------------------------------------

static void queryLayer (const TerrainGenerator::Layer* layer, const ArrayList<TerrainGeneratorHelper::LayerItemQueryType>& query, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	//-- check me!
	{
		bool add = false;

		if (query.existsInList (TerrainGeneratorHelper::LIQT_isLayer))
			add = true;

		if (query.existsInList (TerrainGeneratorHelper::LIQT_hasBoundaries) && layer->getNumberOfBoundaries () > 0)
			add = true;

		if (query.existsInList (TerrainGeneratorHelper::LIQT_hasFilters) && layer->getNumberOfFilters () > 0)
			add = true;

		if (query.existsInList (TerrainGeneratorHelper::LIQT_hasAffectors) && layer->getNumberOfAffectors () > 0)
			add = true;

		if (query.existsInList (TerrainGeneratorHelper::LIQT_hasSublayers) && layer->getNumberOfLayers () > 0)
			add = true;

		if (query.existsInList (TerrainGeneratorHelper::LIQT_hasInvertedBoundaries) && layer->getInvertBoundaries ())
			add = true;

		if (query.existsInList (TerrainGeneratorHelper::LIQT_hasInvertedFilters) && layer->getInvertFilters ())
			add = true;

		if (add)
		{
			CString* buffer = new CString;
			buffer->Format ("%s", layer->getName ());
			output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_find, TerrainGeneratorHelper::OutputData::T_info, buffer, layer));
		}
	}

	{
		//-- check boundaries
		int i;
		for (i = 0; i < layer->getNumberOfBoundaries (); i++)
			::queryBoundary (layer->getBoundary (i), query, output);
	}

	{
		//-- check filters
		int i;
		for (i = 0; i < layer->getNumberOfFilters (); i++)
			::queryFilter (layer->getFilter (i), query, output);
	}

	{
		//-- check affectors
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
			::queryAffector (layer->getAffector (i), query, output);
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			::queryLayer (layer->getLayer (i), query, output);
	}
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::query (const TerrainGenerator* generator, const ArrayList<TerrainGeneratorHelper::LayerItemQueryType>& queryList, ArrayList<OutputData>& output)
{
	{
		//-- query sublayers
		int i;
		for (i = 0; i < generator->getNumberOfLayers (); i++)
			::queryLayer (generator->getLayer (i), queryList, output);
	}
}

//-------------------------------------------------------------------

static void queryLayerItem (const TerrainGenerator::LayerItem* layerItem, const CString& substring, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	const CString string (layerItem->getName ());
	if (string.Find (substring) != -1)
	{
		CString* buffer = new CString;
		buffer->Format ("%s", layerItem->getName ());
		output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_find, TerrainGeneratorHelper::OutputData::T_info, buffer, layerItem));
	}
}

//-------------------------------------------------------------------

static void queryLayer (const TerrainGenerator::Layer* layer, const CString& substring, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	//-- check me!
	{
		queryLayerItem (layer, substring, output);
	}

	{
		//-- check boundaries
		int i;
		for (i = 0; i < layer->getNumberOfBoundaries (); i++)
			::queryLayerItem (layer->getBoundary (i), substring, output);
	}

	{
		//-- check filters
		int i;
		for (i = 0; i < layer->getNumberOfFilters (); i++)
			::queryLayerItem (layer->getFilter (i), substring, output);
	}

	{
		//-- check affectors
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
			::queryLayerItem (layer->getAffector (i), substring, output);
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			::queryLayer (layer->getLayer (i), substring, output);
	}
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::query (const TerrainGenerator* generator, const CString& substring, ArrayList<OutputData>& output)
{
	{
		//-- query sublayers
		int i;
		for (i = 0; i < generator->getNumberOfLayers (); i++)
			::queryLayer (generator->getLayer (i), substring, output);
	}
}

//-------------------------------------------------------------------

static const TerrainGenerator::Boundary* findBoundary (const TerrainGenerator::Layer* layer, const Vector2d& position_w)
{
	{
		//-- check boundaries
		int i;
		for (i = 0; i < layer->getNumberOfBoundaries (); i++)
		{
			if (layer->getBoundary (i)->isActive () && layer->getBoundary (i)->isWithin (position_w.x, position_w.y))
				return layer->getBoundary (i);
		}
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
		{
			if (layer->getLayer (i)->isActive ())
			{
				const TerrainGenerator::Boundary* boundary = ::findBoundary (layer->getLayer (i), position_w);

				if (boundary)
					return boundary;
			}
		}
	}

	return 0;
}

//-------------------------------------------------------------------

const TerrainGenerator::Boundary* TerrainGeneratorHelper::findBoundary (const TerrainGenerator* generator, const Vector2d& position_w)
{
	{
		//-- query sublayers
		int i;
		for (i = 0; i < generator->getNumberOfLayers (); i++)
		{
			if (generator->getLayer (i)->isActive ())
			{
				const TerrainGenerator::Boundary* boundary = ::findBoundary (generator->getLayer (i), position_w);

				if (boundary)
					return boundary;
			}
		}
	}

	return 0;
}

// ---------------------------------------------------------------------
static void findBoundary (const TerrainGenerator::Layer* layer, ArrayList<TerrainGenerator::Boundary*>& boundaryList, const Vector2d& position_w, bool bIncludeInactive)
{
	{
		//-- check boundaries
		int i;
		for(i = 0; i < layer->getNumberOfBoundaries(); ++i)
		{
			if( ( bIncludeInactive || layer->getBoundary(i)->isActive() )
				&& layer->getBoundary (i)->isWithin (position_w.x, position_w.y)
			)
			{
				boundaryList.add(const_cast<TerrainGenerator::Boundary*> (safe_cast<const TerrainGenerator::Boundary*> (layer->getBoundary(i))));
			}
		}
	}

	{
		//-- check sublayers
		int i;
		for(i = 0; i < layer->getNumberOfLayers(); ++i)
		{
			if(bIncludeInactive || layer->isActive())
			{
				::findBoundary(layer->getLayer(i), boundaryList,position_w, bIncludeInactive);
			}
		}
	}
}
// ---------------------------------------------------------------------
void TerrainGeneratorHelper::findBoundary (const TerrainGenerator* generator, ArrayList<TerrainGenerator::Boundary*>& boundaryList, const Vector2d& position_w, bool bIncludeInactive)
{
	{
		//-- query sublayers
		int i;
		for(i = 0; i < generator->getNumberOfLayers(); ++i)
		{
			if(bIncludeInactive || generator->getLayer(i)->isActive())
			{
				::findBoundary(generator->getLayer(i),boundaryList,position_w,bIncludeInactive);
			}
		}
	}
}


//-------------------------------------------------------------------

static void findAffectorBoundaryPoly (const TerrainGenerator::Layer* layer, ArrayList<AffectorBoundaryPoly*>& affectorList)
{
	{
		//-- check affectors
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
		{
			if (layer->getAffector (i)->isActive ()
				&& (layer->getAffector (i)->getType () == TGAT_river || layer->getAffector (i)->getType () == TGAT_road || layer->getAffector (i)->getType () == TGAT_ribbon)
			)
			{
				affectorList.add (const_cast<AffectorBoundaryPoly*> (safe_cast<const AffectorBoundaryPoly*> (layer->getAffector (i))));
			}
		}
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
		{
			if (layer->isActive ())
			{
				::findAffectorBoundaryPoly (layer->getLayer (i), affectorList);
			}
		}
	}
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::findAffectorBoundaryPoly (const TerrainGenerator* generator, ArrayList<AffectorBoundaryPoly*>& affectorList)
{
	{
		//-- query sublayers
		int i;
		for (i = 0; i < generator->getNumberOfLayers (); i++)
			if (generator->getLayer (i)->isActive ())
				::findAffectorBoundaryPoly (generator->getLayer (i), affectorList);
	}
}

//-------------------------------------------------------------------
void TerrainGeneratorHelper::findBoundaryAndAffectorBoundaryPoly (const TerrainGenerator* generator, ArrayList<TerrainGenerator::LayerItem*>& itemList, const Vector2d& position_w, bool bIncludeInactive)
{
	ArrayList<TerrainGenerator::Boundary*> boundaryList;
	TerrainGeneratorHelper::findBoundary (generator, boundaryList,position_w,bIncludeInactive);

	ArrayList<AffectorBoundaryPoly*> affectorList;
	TerrainGeneratorHelper::findAffectorBoundaryPoly(generator, affectorList);

	for(int boundaryIndex = 0; boundaryIndex < boundaryList.getNumberOfElements(); ++boundaryIndex)
	{
		itemList.add(boundaryList[boundaryIndex]);
	}

	for(int affectorIndex = 0; affectorIndex < affectorList.getNumberOfElements(); ++affectorIndex)
	{
		if(isPointInStrip(position_w,affectorList[affectorIndex]->getPointList(),affectorList[affectorIndex]->getWidth()))
		{
			itemList.add(affectorList[affectorIndex]);
		}
	}
}


//-------------------------------------------------------------------

static void expandExtents (const TerrainGenerator::Layer* layer, Rectangle2d& extent)
{
	{
		//-- check boundaries
		int i;
		for (i = 0; i < layer->getNumberOfBoundaries (); i++)
		{
			if (layer->getBoundary (i)->isActive ())
			{
				layer->getBoundary (i)->expand (extent);
			}
		}
	}

	{
		//-- check affectors
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
		{
			if (layer->getAffector (i)->isActive ()
				&& (layer->getAffector (i)->getType () == TGAT_river || layer->getAffector (i)->getType () == TGAT_road || layer->getAffector (i)->getType () == TGAT_ribbon)
			)
			{
				safe_cast<const AffectorBoundaryPoly*> (layer->getAffector (i))->expand (extent);
			}
		}
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
		{
			if (layer->getLayer (i)->isActive ())
			{
				::expandExtents (layer->getLayer (i), extent);
			}
		}
	}
}

//-------------------------------------------------------------------

const Vector2d TerrainGeneratorHelper::computeCenter (const TerrainGenerator::Layer* layer)
{
	Rectangle2d extent;
	extent.x0 = REAL_MAX;
	extent.y0 = REAL_MAX;
	extent.x1 = -REAL_MAX;
	extent.y1 = -REAL_MAX;

	expandExtents (layer, extent);

	return extent.getCenter ();
}

bool TerrainGeneratorHelper::isPointInStrip(const Vector2d& point, const ArrayList<Vector2d>& pointList, const float width)
{
	float halfWidth = width/2.0f;
	if(halfWidth <= 0.0f)
	{
		return false;
	}

	if (pointList.getNumberOfElements () == 0)
	{
		return false;
	}

	if (halfWidth > 0.f)
	{
		int i;
		for (i = 0; i < pointList.getNumberOfElements () - 1; ++i)
		{
			const Vector2d& segmentStart          = pointList [i];
			const Vector2d& segmentEnd            = pointList [i + 1];
			Line2d line(segmentStart.x,segmentStart.y,segmentEnd.x,segmentEnd.y);
			float dist = line.computeDistanceTo(point);

			if(fabs(dist) <= halfWidth)
			{
				Vector2d segment = segmentEnd - segmentStart;
				float segmentMag = segment.magnitude();
				Vector2d point2SegmentStart = segmentStart - point;
				Vector2d point2SegmentEnd = segmentEnd - point;
				if(point2SegmentStart.magnitude() < segmentMag && point2SegmentEnd.magnitude() < segmentMag)
				{
					return true;
				}
			}
		}
	}

	return false;
}

//===================================================================

