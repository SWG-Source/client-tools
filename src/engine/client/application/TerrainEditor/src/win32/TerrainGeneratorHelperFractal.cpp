//
// TerrainGeneratorHelperFractal.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "TerrainGeneratorHelper.h"

#include "FractalFrame.h"
#include "TerrainEditorDoc.h"
#include "sharedTerrain/AffectorHeight.h"
#include "sharedTerrain/AffectorColor.h"
#include "sharedTerrain/Filter.h"
#include "sharedFractal/MultiFractal.h"

//-------------------------------------------------------------------

static int findUniqueFamilyId (const FractalGroup& fractalGroup)
{
	int i = 1;
	while (fractalGroup.hasFamily (i))
		i++;

	return i;
}

//-------------------------------------------------------------------

bool TerrainGeneratorHelper::usesFractalFamily (const TerrainGenerator::Layer* layer, int familyId)
{
	{
		{
			//-- check filters
			int i;
			for (i = 0; i < layer->getNumberOfFilters (); i++)
			{
				if (layer->getFilter (i)->getType () == TGFT_fractal)
				{
					const FilterFractal* filter = static_cast<const FilterFractal*> (layer->getFilter (i));

					if (familyId == filter->getFamilyId ())
						return true;
				}
			}
		}

		{
			int i;
			for (i = 0; i < layer->getNumberOfAffectors (); i++)
			{
				if (layer->getAffector (i)->getType () == TGAT_heightFractal)
				{
					const AffectorHeightFractal* affector = static_cast<const AffectorHeightFractal*> (layer->getAffector (i));

					if (familyId == affector->getFamilyId ())
						return true;
				}

				if (layer->getAffector (i)->getType () == TGAT_colorRampFractal)
				{
					const AffectorColorRampFractal* affector = static_cast<const AffectorColorRampFractal*> (layer->getAffector (i));

					if (familyId == affector->getFamilyId ())
						return true;
				}
			}
		}
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			if (usesFractalFamily (layer->getLayer (i), familyId))
				return true;
	}

	return false;
}

//-------------------------------------------------------------------

bool TerrainGeneratorHelper::usesFractalFamily (const TerrainGenerator* generator, int familyId)
{
	//-- check sublayers
	int i;
	for (i = 0; i < generator->getNumberOfLayers (); i++)
		if (usesFractalFamily (generator->getLayer (i), familyId))
			return true;

	return false;
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::saveFractalGroup (Iff& iff, const FractalGroup& fractalGroup, const TerrainGenerator::Affector* affector)
{
	UNREF (affector);

	//-- write out a fractal group that only consists of families used by affector
//	FractalGroup trimFractalGroup;
//	buildFractalGroup (trimFractalGroup, fractalGroup, affector);
//	trimFractalGroup.save (iff);

	fractalGroup.save (iff);
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::saveFractalGroup (Iff& iff, const FractalGroup& fractalGroup, const TerrainGenerator::Filter* filter)
{
	UNREF (filter);

	//-- write out a fractal group that only consists of families used by filter
//	FractalGroup trimFractalGroup;
//	trimFractalGroup.save (iff);

	fractalGroup.save (iff);
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::saveFractalGroup (Iff& iff, const FractalGroup& fractalGroup, const TerrainGenerator::Layer* layer)
{
	UNREF (layer);

	//-- write out a fractal group that only consists of families used by layer
//	FractalGroup trimFractalGroup;
//	buildFractalGroup (trimFractalGroup, fractalGroup, layer);
//	trimFractalGroup.save (iff);

	fractalGroup.save (iff);
}

//-------------------------------------------------------------------

FractalGroup* TerrainGeneratorHelper::loadFractalGroup (Iff& iff)
{
	FractalGroup* fractalGroup = new FractalGroup;
	fractalGroup->load (iff);

	return fractalGroup;
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::fixupFractal (TerrainEditorDoc* document, FractalGroup& destinationFractalGroup, const FractalGroup& sourceFractalGroup, TerrainGenerator::Affector* affector)
{
	//-- does this affector need fixing?
	if (affector->getType () == TGAT_heightFractal)
	{
		AffectorHeightFractal* const affectorHeightFractal = safe_cast<AffectorHeightFractal*> (affector);

		//-- get the old id
		const int oldId = affectorHeightFractal->getFamilyId ();

		//-- get the old name
		const char* oldName = sourceFractalGroup.getFamilyName (oldId);
		NOT_NULL (oldName);

		//-- look up new name
		int newId;
		if (destinationFractalGroup.findFamily (oldName, newId))
		{
			//-- reset the id
			affectorHeightFractal->setFamilyId (newId);
		}
		else
		{
			//-- copy family over
			newId = findUniqueFamilyId (destinationFractalGroup);

			affectorHeightFractal->setFamilyId (newId);

			destinationFractalGroup.addFamily (newId, oldName);

			const MultiFractal* sourceMultiFractal = sourceFractalGroup.getFamilyMultiFractal (oldId);
			NOT_NULL (sourceMultiFractal);

			MultiFractal* destinationMultiFractal = destinationFractalGroup.getFamilyMultiFractal (newId);
			NOT_NULL (destinationMultiFractal);

			*destinationMultiFractal = *sourceMultiFractal;

			if (document->getFractalFrame ())
				static_cast<FractalFrame*> (document->getFractalFrame ())->reset ();
		}
	}

	//-- does this affector need fixing?
	if (affector->getType () == TGAT_colorRampFractal)
	{
		AffectorColorRampFractal* const affectorColorRampFractal = safe_cast<AffectorColorRampFractal*> (affector);

		//-- get the old id
		const int oldId = affectorColorRampFractal->getFamilyId ();

		//-- get the old name
		const char* oldName = sourceFractalGroup.getFamilyName (oldId);
		NOT_NULL (oldName);

		//-- look up new name
		int newId;
		if (destinationFractalGroup.findFamily (oldName, newId))
		{
			//-- reset the id
			affectorColorRampFractal->setFamilyId (newId);
		}
		else
		{
			//-- copy family over
			newId = findUniqueFamilyId (destinationFractalGroup);

			affectorColorRampFractal->setFamilyId (newId);

			destinationFractalGroup.addFamily (newId, oldName);

			const MultiFractal* sourceMultiFractal = sourceFractalGroup.getFamilyMultiFractal (oldId);
			NOT_NULL (sourceMultiFractal);

			MultiFractal* destinationMultiFractal = destinationFractalGroup.getFamilyMultiFractal (newId);
			NOT_NULL (destinationMultiFractal);

			*destinationMultiFractal = *sourceMultiFractal;

			if (document->getFractalFrame ())
				static_cast<FractalFrame*> (document->getFractalFrame ())->reset ();
		}
	}
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::fixupFractal (TerrainEditorDoc* document, FractalGroup& destinationFractalGroup, const FractalGroup& sourceFractalGroup, TerrainGenerator::Filter* filter)
{
	//-- does this affector need fixing?
	if (filter->getType () == TGFT_fractal)
	{
		FilterFractal* const filterFractal = safe_cast<FilterFractal*> (filter);

		//-- get the old id
		const int oldId = filterFractal->getFamilyId ();

		//-- get the old name
		const char* oldName = sourceFractalGroup.getFamilyName (oldId);
		NOT_NULL (oldName);

		//-- look up new name
		int newId;
		if (destinationFractalGroup.findFamily (oldName, newId))
		{
			//-- reset the id
			filterFractal->setFamilyId (newId);
		}
		else
		{
			//-- copy family over
			newId = findUniqueFamilyId (destinationFractalGroup);

			filterFractal->setFamilyId (newId);

			destinationFractalGroup.addFamily (newId, oldName);

			const MultiFractal* sourceMultiFractal = sourceFractalGroup.getFamilyMultiFractal (oldId);
			NOT_NULL (sourceMultiFractal);

			MultiFractal* destinationMultiFractal = destinationFractalGroup.getFamilyMultiFractal (newId);
			NOT_NULL (destinationMultiFractal);

			*destinationMultiFractal = *sourceMultiFractal;

			if (document->getFractalFrame ())
				static_cast<FractalFrame*> (document->getFractalFrame ())->reset ();
		}
	}
}

//-------------------------------------------------------------------

static void findFractal (const TerrainGenerator::Filter* filter, int familyId, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	if (filter->getType () == TGFT_fractal)
	{
		if (safe_cast<const FilterFractal*> (filter)->getFamilyId () == familyId)
		{
			CString* buffer = new CString;
			buffer->Format ("%s", filter->getName ());
			output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_find, TerrainGeneratorHelper::OutputData::T_info, buffer, filter));
		}
	}
}

//-------------------------------------------------------------------

static void findFractal (const TerrainGenerator::Affector* affector, int familyId, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	if (affector->getType () == TGAT_heightFractal)
	{
		if (safe_cast<const AffectorHeightFractal*> (affector)->getFamilyId () == familyId)
		{
			CString* buffer = new CString;
			buffer->Format ("%s", affector->getName ());
			output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_find, TerrainGeneratorHelper::OutputData::T_info, buffer, affector));
		}
	}

	if (affector->getType () == TGAT_colorRampFractal)
	{
		if (safe_cast<const AffectorColorRampFractal*> (affector)->getFamilyId () == familyId)
		{
			CString* buffer = new CString;
			buffer->Format ("%s", affector->getName ());
			output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_find, TerrainGeneratorHelper::OutputData::T_info, buffer, affector));
		}
	}
}

//-------------------------------------------------------------------

static void findFractal (const TerrainGenerator::Layer* layer, int familyId, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	{
		//-- check filters
		int i;
		for (i = 0; i < layer->getNumberOfFilters (); i++)
			::findFractal (layer->getFilter (i), familyId, output);
	}

	{
		//-- check affectors
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
			::findFractal (layer->getAffector (i), familyId, output);
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			::findFractal (layer->getLayer (i), familyId, output);
	}
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::findFractal (const TerrainGenerator* generator, int familyId, ArrayList<OutputData>& output)
{
	//-- query sublayers
	int i;
	for (i = 0; i < generator->getNumberOfLayers (); i++)
		::findFractal (generator->getLayer (i), familyId, output);
}

//-------------------------------------------------------------------

