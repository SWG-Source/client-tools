//
// TerrainGeneratorHelperFlora.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "TerrainGeneratorHelper.h"

#include "sharedTerrain/AffectorFloraStatic.h"
#include "FloraGroupFrame.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

static int findUniqueFamilyId (const FloraGroup& floraGroup)
{
	int i = 1;
	while (floraGroup.hasFamily (i))
		i++;

	return i;
}

//-------------------------------------------------------------------

bool TerrainGeneratorHelper::usesFloraFamily (const TerrainGenerator::Layer* layer, int familyId)
{
	{
		//-- check affectors
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
		{
			if (layer->getAffector (i)->getType () == TGAT_floraStaticCollidableConstant || layer->getAffector (i)->getType () == TGAT_floraStaticNonCollidableConstant)
			{
				const AffectorFloraStatic* const affector = safe_cast<const AffectorFloraStatic*> (layer->getAffector (i));

				if (familyId == affector->getFamilyId ())
					return true;
			}
		}
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			if (usesFloraFamily (layer->getLayer (i), familyId))
				return true;
	}

	return false;
}

//-------------------------------------------------------------------

bool TerrainGeneratorHelper::usesFloraFamily (const TerrainGenerator* generator, int familyId)
{
	//-- check sublayers
	int i;
	for (i = 0; i < generator->getNumberOfLayers (); i++)
		if (usesFloraFamily (generator->getLayer (i), familyId))
			return true;

	return false;
}

//-------------------------------------------------------------------

#if 0

static void buildFloraGroup (FloraGroup& trimFloraGroup, const FloraGroup& floraGroup, const TerrainGenerator::Affector* affector)
{
	if (affector->getType () == TGAT_floraConstant)
	{
		//-- find family used
		const AffectorFloraConstant* affectorFloraConstant = static_cast<const AffectorFloraConstant*> (affector);

		const int familyId = affectorFloraConstant->getFamilyId ();

		if (!trimFloraGroup.hasFamily (familyId))
		{
			trimFloraGroup.addFamily (familyId, floraGroup.getFamilyName (familyId), floraGroup.getFamilyColor (familyId));

			trimFloraGroup.setFamilyDensity (familyId, floraGroup.getFamilyDensity (familyId));

			const int n = floraGroup.getFamilyNumberOfChildren (familyId);
			int i;
			for (i = 0; i < n; i++)
				trimFloraGroup.addChild (floraGroup.getFamilyChild (familyId, i));
		}
	}
}

//-------------------------------------------------------------------

static void buildFloraGroup (FloraGroup& trimFloraGroup, const FloraGroup& floraGroup, const TerrainGenerator::Layer* layer)
{
	{
		//-- check affectors
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
			buildFloraGroup (trimFloraGroup, floraGroup, layer->getAffector (i));
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			buildFloraGroup (trimFloraGroup, floraGroup, layer->getLayer (i));
	}
}

#endif

//-------------------------------------------------------------------

void TerrainGeneratorHelper::saveFloraGroup  (Iff& iff, const FloraGroup&  floraGroup,  const TerrainGenerator::Affector* affector)
{
	UNREF (affector);

	//-- write out a flora group that only consists of families used by affector
//	FloraGroup trimFloraGroup;
//	buildFloraGroup (trimFloraGroup, floraGroup, affector);
//	trimFloraGroup.save (iff);

	floraGroup.save (iff);
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::saveFloraGroup  (Iff& iff, const FloraGroup&  floraGroup,  const TerrainGenerator::Filter* filter)
{
	UNREF (filter);

	//-- write out a flora group that only consists of families used by filter
//	FloraGroup trimFloraGroup;
//	trimFloraGroup.save (iff);

	floraGroup.save (iff);
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::saveFloraGroup  (Iff& iff, const FloraGroup&  floraGroup,  const TerrainGenerator::Layer* layer)
{
	UNREF (layer);

	//-- write out a flora group that only consists of families used by layer
//	FloraGroup trimFloraGroup;
//	buildFloraGroup (trimFloraGroup, floraGroup, layer);
//	trimFloraGroup.save (iff);

	floraGroup.save (iff);
}

//-------------------------------------------------------------------

FloraGroup*  TerrainGeneratorHelper::loadFloraGroup  (Iff& iff)
{
	FloraGroup* floraGroup = new FloraGroup;
	floraGroup->load (iff);

	return floraGroup;
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::fixupFlora (TerrainEditorDoc* document, FloraGroup& destinationFloraGroup, const FloraGroup& sourceFloraGroup, TerrainGenerator::Affector* affector)
{
	//-- does this affector need fixing?
	if (affector->getType () == TGAT_floraStaticCollidableConstant || affector->getType () == TGAT_floraStaticNonCollidableConstant)
	{
		AffectorFloraStatic* const affectorFloraStatic = safe_cast<AffectorFloraStatic*> (affector);

		//-- get the old id
		const int oldId = affectorFloraStatic->getFamilyId ();

		//-- get the old name
		const char* oldName = sourceFloraGroup.getFamilyName (oldId);
		NOT_NULL (oldName);

		//-- look up new name
		if (destinationFloraGroup.hasFamily (oldName))
		{
			//-- reset the id
			const int newId = destinationFloraGroup.getFamilyId (oldName);

			affectorFloraStatic->setFamilyId (newId);
		}
		else
		{
			//-- copy family over
			const int newId = findUniqueFamilyId (destinationFloraGroup);

			affectorFloraStatic->setFamilyId (newId);

			destinationFloraGroup.addFamily (newId, oldName, sourceFloraGroup.getFamilyColor (oldId));

			destinationFloraGroup.setFamilyDensity (newId, sourceFloraGroup.getFamilyDensity (oldId));

			const int n = sourceFloraGroup.getFamilyNumberOfChildren (oldId);
			int i;
			for (i = 0; i < n; i++)
			{
				FloraGroup::FamilyChildData fcd = sourceFloraGroup.getFamilyChild (oldId, i);
				fcd.familyId = newId;

				destinationFloraGroup.addChild (fcd);
			}

			if (document->getFloraGroupFrame ())
				static_cast<FloraGroupFrame*> (document->getFloraGroupFrame ())->reset ();
		}
	}
}

//-------------------------------------------------------------------

static void findFlora (const TerrainGenerator::Affector* affector, int familyId, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	if (affector->getType () == TGAT_floraStaticCollidableConstant || affector->getType () == TGAT_floraStaticNonCollidableConstant)
	{
		if (safe_cast<const AffectorFloraStatic*> (affector)->getFamilyId () == familyId)
		{
			CString* buffer = new CString;
			buffer->Format ("%s", affector->getName ());
			output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_find, TerrainGeneratorHelper::OutputData::T_info, buffer, affector));
		}
	}
}

//-------------------------------------------------------------------

static void findFlora (const TerrainGenerator::Layer* layer, int familyId, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	{
		//-- check affectors
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
			::findFlora (layer->getAffector (i), familyId, output);
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			::findFlora (layer->getLayer (i), familyId, output);
	}
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::findFlora (const TerrainGenerator* generator, int familyId, ArrayList<OutputData>& output)
{
	//-- query sublayers
	int i;
	for (i = 0; i < generator->getNumberOfLayers (); i++)
		::findFlora (generator->getLayer (i), familyId, output);
}

//-------------------------------------------------------------------

