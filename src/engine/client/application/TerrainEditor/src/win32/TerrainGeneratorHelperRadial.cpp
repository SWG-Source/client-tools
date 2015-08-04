//
// TerrainGeneratorHelperRadial.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "TerrainGeneratorHelper.h"

#include "sharedTerrain/AffectorFloraDynamic.h"
#include "RadialGroupFrame.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

static int findUniqueFamilyId (const RadialGroup& radialGroup)
{
	int i = 1;
	while (radialGroup.hasFamily (i))
		i++;

	return i;
}

//-------------------------------------------------------------------

bool TerrainGeneratorHelper::usesRadialFamily (const TerrainGenerator::Layer* layer, int familyId)
{
	{
		//-- check affectors
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
		{
			if (layer->getAffector (i)->getType () == TGAT_floraDynamicNearConstant || layer->getAffector (i)->getType () == TGAT_floraDynamicFarConstant)
			{
				const AffectorFloraDynamic* const affector = safe_cast<const AffectorFloraDynamic*> (layer->getAffector (i));

				if (familyId == affector->getFamilyId ())
					return true;
			}
		}
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			if (usesRadialFamily (layer->getLayer (i), familyId))
				return true;
	}

	return false;
}

//-------------------------------------------------------------------

bool TerrainGeneratorHelper::usesRadialFamily (const TerrainGenerator* generator, int familyId)
{
	//-- check sublayers
	int i;
	for (i = 0; i < generator->getNumberOfLayers (); i++)
		if (usesRadialFamily (generator->getLayer (i), familyId))
			return true;

	return false;
}

//-------------------------------------------------------------------

#if 0

static void buildRadialGroup (RadialGroup& trimRadialGroup, const RadialGroup& radialGroup, const TerrainGenerator::Affector* affector)
{
	if (affector->getType () == TGAT_radialConstant)
	{
		//-- find family used
		const AffectorRadialConstant* affectorRadialConstant = static_cast<const AffectorRadialConstant*> (affector);

		const int familyId = affectorRadialConstant->getFamilyId ();

		if (!trimRadialGroup.hasFamily (familyId))
		{
			trimRadialGroup.addFamily (familyId, radialGroup.getFamilyName (familyId), radialGroup.getFamilyColor (familyId));

			trimRadialGroup.setFamilyDensity (familyId, radialGroup.getFamilyDensity (familyId));

			const int n = radialGroup.getFamilyNumberOfChildren (familyId);
			int i;
			for (i = 0; i < n; i++)
				trimRadialGroup.addChild (radialGroup.getFamilyChild (familyId, i));
		}
	}
}

//-------------------------------------------------------------------

static void buildRadialGroup (RadialGroup& trimRadialGroup, const RadialGroup& radialGroup, const TerrainGenerator::Layer* layer)
{
	{
		//-- check affectors
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
			buildRadialGroup (trimRadialGroup, radialGroup, layer->getAffector (i));
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			buildRadialGroup (trimRadialGroup, radialGroup, layer->getLayer (i));
	}
}

#endif

//-------------------------------------------------------------------

void TerrainGeneratorHelper::saveRadialGroup (Iff& iff, const RadialGroup& radialGroup, const TerrainGenerator::Affector* affector)
{
	UNREF (affector);

	//-- write out a radial group that only consists of families used by affector
//	RadialGroup trimRadialGroup;
//	buildRadialGroup (trimRadialGroup, radialGroup, affector);
//	trimRadialGroup.save (iff);

	radialGroup.save (iff);
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::saveRadialGroup (Iff& iff, const RadialGroup& radialGroup, const TerrainGenerator::Filter* filter)
{
	UNREF (filter);

	//-- write out a radial group that only consists of families used by filter
//	RadialGroup trimRadialGroup;
//	trimRadialGroup.save (iff);

	radialGroup.save (iff);
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::saveRadialGroup (Iff& iff, const RadialGroup& radialGroup, const TerrainGenerator::Layer* layer)
{
	UNREF (layer);

	//-- write out a radial group that only consists of families used by layer
//	RadialGroup trimRadialGroup;
//	buildRadialGroup (trimRadialGroup, radialGroup, layer);
//	trimRadialGroup.save (iff);

	radialGroup.save (iff);
}

//-------------------------------------------------------------------

RadialGroup* TerrainGeneratorHelper::loadRadialGroup (Iff& iff)
{
	RadialGroup* radialGroup = new RadialGroup;
	radialGroup->load (iff);

	return radialGroup;
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::fixupRadial (TerrainEditorDoc* document, RadialGroup& destinationRadialGroup, const RadialGroup& sourceRadialGroup, TerrainGenerator::Affector* affector)
{
	//-- does this affector need fixing?
	if (affector->getType () == TGAT_floraDynamicNearConstant || affector->getType () == TGAT_floraDynamicFarConstant)
	{
		AffectorFloraDynamic* const affectorFloraDynamic = safe_cast<AffectorFloraDynamic*> (affector);

		//-- get the old id
		const int oldId = affectorFloraDynamic->getFamilyId ();

		//-- get the old name
		const char* oldName = sourceRadialGroup.getFamilyName (oldId);
		NOT_NULL (oldName);

		//-- look up new name
		if (destinationRadialGroup.hasFamily (oldName))
		{
			//-- reset the id
			const int newId = destinationRadialGroup.getFamilyId (oldName);

			affectorFloraDynamic->setFamilyId (newId);
		}
		else
		{
			//-- copy family over
			const int newId = findUniqueFamilyId (destinationRadialGroup);

			affectorFloraDynamic->setFamilyId (newId);

			destinationRadialGroup.addFamily (newId, oldName, sourceRadialGroup.getFamilyColor (oldId));

			destinationRadialGroup.setFamilyDensity (newId, sourceRadialGroup.getFamilyDensity (oldId));

			const int n = sourceRadialGroup.getFamilyNumberOfChildren (oldId);
			int i;
			for (i = 0; i < n; i++)
			{
				RadialGroup::FamilyChildData fcd = sourceRadialGroup.getFamilyChild (oldId, i);
				fcd.familyId = newId;

				destinationRadialGroup.addChild (fcd);
			}

			if (document->getRadialGroupFrame ())
				static_cast<RadialGroupFrame*> (document->getRadialGroupFrame ())->reset ();
		}
	}
}

//-------------------------------------------------------------------

static void findRadial (const TerrainGenerator::Affector* affector, int familyId, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	if (affector->getType () == TGAT_floraDynamicNearConstant || affector->getType () == TGAT_floraDynamicFarConstant)
	{
		if (safe_cast<const AffectorFloraDynamic*> (affector)->getFamilyId () == familyId)
		{
			CString* buffer = new CString;
			buffer->Format ("%s", affector->getName ());
			output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_find, TerrainGeneratorHelper::OutputData::T_info, buffer, affector));
		}
	}
}

//-------------------------------------------------------------------

static void findRadial (const TerrainGenerator::Layer* layer, int familyId, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	{
		//-- check affectors
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
			::findRadial (layer->getAffector (i), familyId, output);
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			::findRadial (layer->getLayer (i), familyId, output);
	}
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::findRadial (const TerrainGenerator* generator, int familyId, ArrayList<OutputData>& output)
{
	//-- query sublayers
	int i;
	for (i = 0; i < generator->getNumberOfLayers (); i++)
		::findRadial (generator->getLayer (i), familyId, output);
}

//-------------------------------------------------------------------

