//
// TerrainGeneratorHelperBitmap.cpp
//
// copyright 2004, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "TerrainGeneratorHelper.h"

#include "FractalFrame.h"
#include "TerrainEditorDoc.h"
#include "sharedTerrain/AffectorHeight.h"
#include "sharedTerrain/AffectorColor.h"
#include "sharedTerrain/Filter.h"


//-------------------------------------------------------------------

bool TerrainGeneratorHelper::usesBitmapFamily (const TerrainGenerator::Layer* layer, int familyId)
{
	{
		{
			//-- check filters
			int i;
			for (i = 0; i < layer->getNumberOfFilters (); i++)
			{
				if (layer->getFilter (i)->getType () == TGFT_bitmap)
				{
					const FilterBitmap* filter = static_cast<const FilterBitmap*> (layer->getFilter (i));

					if (familyId == filter->getFamilyId ())
						return true;
				}
			}
		}
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			if (usesBitmapFamily (layer->getLayer (i), familyId))
				return true;
	}

	return false;
}

//-------------------------------------------------------------------

bool TerrainGeneratorHelper::usesBitmapFamily (const TerrainGenerator* generator, int familyId)
{
	//-- check sublayers
	int i;
	for (i = 0; i < generator->getNumberOfLayers (); i++)
		if (usesBitmapFamily (generator->getLayer (i), familyId))
			return true;

	return false;
}

//-------------------------------------------------------------------

static void findBitmap (const TerrainGenerator::Filter* filter, int familyId, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	if (filter->getType () == TGFT_bitmap)
	{
		if (safe_cast<const FilterBitmap*> (filter)->getFamilyId () == familyId)
		{
			CString* buffer = new CString;
			buffer->Format ("%s", filter->getName ());
			output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_find, TerrainGeneratorHelper::OutputData::T_info, buffer, filter));
		}
	}
}


//-------------------------------------------------------------------

static void findBitmap (const TerrainGenerator::Layer* layer, int familyId, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	{
		//-- check filters
		int i;
		for (i = 0; i < layer->getNumberOfFilters (); i++)
			::findBitmap (layer->getFilter (i), familyId, output);
	}

	
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::findBitmap (const TerrainGenerator* generator, int familyId, ArrayList<OutputData>& output)
{
	//-- query sublayers
	int i;
	for (i = 0; i < generator->getNumberOfLayers (); i++)
		::findBitmap (generator->getLayer (i), familyId, output);
}


//-------------------------------------------------------------------

