//
// TerrainGeneratorHelperEnvironment.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "TerrainGeneratorHelper.h"

#include "sharedTerrain/AffectorEnvironment.h"
#include "EnvironmentFrame.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

static int findUniqueFamilyId (const EnvironmentGroup& environmentGroup)
{
	int i = 1;
	while (environmentGroup.hasFamily (i))
		i++;

	return i;
}

//-------------------------------------------------------------------

bool TerrainGeneratorHelper::usesEnvironmentFamily (const TerrainGenerator::Layer* layer, int familyId)
{
	{
		//-- check affectors
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
		{
			if (layer->getAffector (i)->getType () == TGAT_environment)
			{
				const AffectorEnvironment* const affector = safe_cast<const AffectorEnvironment*> (layer->getAffector (i));

				if (familyId == affector->getFamilyId ())
					return true;
			}
		}
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			if (usesEnvironmentFamily (layer->getLayer (i), familyId))
				return true;
	}

	return false;
}

//-------------------------------------------------------------------

bool TerrainGeneratorHelper::usesEnvironmentFamily (const TerrainGenerator* generator, int familyId)
{
	//-- check sublayers
	int i;
	for (i = 0; i < generator->getNumberOfLayers (); i++)
		if (usesEnvironmentFamily (generator->getLayer (i), familyId))
			return true;

	return false;
}

//-------------------------------------------------------------------

#if 0

static void buildEnvironmentGroup (EnvironmentGroup& trimEnvironmentGroup, const EnvironmentGroup& environmentGroup, const TerrainGenerator::Affector* affector)
{
	if (affector->getType () == TGAT_environmentConstant)
	{
		//-- find family used
		const AffectorEnvironmentConstant* affectorEnvironmentConstant = static_cast<const AffectorEnvironmentConstant*> (affector);

		const int familyId = affectorEnvironmentConstant->getFamilyId ();

		if (!trimEnvironmentGroup.hasFamily (familyId))
		{
			trimEnvironmentGroup.addFamily (familyId, environmentGroup.getFamilyName (familyId), environmentGroup.getFamilyColor (familyId));

			trimEnvironmentGroup.setFamilyFeatherClamp (familyId, environmentGroup.getFamilyFeatherClamp (familyId));

			const int n = environmentGroup.getFamilyNumberOfChildren (familyId);
			int i;
			for (i = 0; i < n; i++)
				trimEnvironmentGroup.addChild (environmentGroup.getFamilyChild (familyId, i));
		}
	}
}

//-------------------------------------------------------------------

static void buildEnvironmentGroup (EnvironmentGroup& trimEnvironmentGroup, const EnvironmentGroup& environmentGroup, const TerrainGenerator::Layer* layer)
{
	{
		//-- check affectors
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
			buildEnvironmentGroup (trimEnvironmentGroup, environmentGroup, layer->getAffector (i));
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			buildEnvironmentGroup (trimEnvironmentGroup, environmentGroup, layer->getLayer (i));
	}
}

#endif

//-------------------------------------------------------------------

void TerrainGeneratorHelper::saveEnvironmentGroup (Iff& iff, const EnvironmentGroup& environmentGroup, const TerrainGenerator::Affector* affector)
{
	UNREF (affector);

	//-- write out a environment group that only consists of families used by affector
//	EnvironmentGroup trimEnvironmentGroup;
//	buildEnvironmentGroup (trimEnvironmentGroup, environmentGroup, affector);
//	trimEnvironmentGroup.save (iff);

	environmentGroup.save (iff);
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::saveEnvironmentGroup (Iff& iff, const EnvironmentGroup& environmentGroup, const TerrainGenerator::Filter* filter)
{
	UNREF (filter);

	//-- write out a environment group that only consists of families used by filter
//	EnvironmentGroup trimEnvironmentGroup;
//	trimEnvironmentGroup.save (iff);

	environmentGroup.save (iff);
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::saveEnvironmentGroup (Iff& iff, const EnvironmentGroup& environmentGroup, const TerrainGenerator::Layer* layer)
{
	UNREF (layer);

	//-- write out a environment group that only consists of families used by layer
//	EnvironmentGroup trimEnvironmentGroup;
//	buildEnvironmentGroup (trimEnvironmentGroup, environmentGroup, layer);
//	trimEnvironmentGroup.save (iff);

	environmentGroup.save (iff);
}

//-------------------------------------------------------------------

EnvironmentGroup* TerrainGeneratorHelper::loadEnvironmentGroup (Iff& iff)
{
	EnvironmentGroup* environmentGroup = new EnvironmentGroup;
	environmentGroup->load (iff);

	return environmentGroup;
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::fixupEnvironment (TerrainEditorDoc* document, EnvironmentGroup& destinationEnvironmentGroup, const EnvironmentGroup& sourceEnvironmentGroup, TerrainGenerator::Affector* affector)
{
	//-- does this affector need fixing?
	if (affector->getType () == TGAT_environment)
	{
		AffectorEnvironment* const affectorEnvironment = safe_cast<AffectorEnvironment*> (affector);

		//-- get the old id
		const int oldId = affectorEnvironment->getFamilyId ();

		//-- get the old name
		const char* oldName = sourceEnvironmentGroup.getFamilyName (oldId);
		NOT_NULL (oldName);

		//-- look up new name
		if (destinationEnvironmentGroup.hasFamily (oldName))
		{
			//-- reset the id
			const int newId = destinationEnvironmentGroup.getFamilyId (oldName);

			affectorEnvironment->setFamilyId (newId);
		}
		else
		{
			//-- copy family over
			const int newId = findUniqueFamilyId (destinationEnvironmentGroup);

			affectorEnvironment->setFamilyId (newId);

			destinationEnvironmentGroup.addFamily (newId, oldName, sourceEnvironmentGroup.getFamilyColor (oldId));

			destinationEnvironmentGroup.setFamilyFeatherClamp  (newId, sourceEnvironmentGroup.getFamilyFeatherClamp (oldId));

			if (document->getEnvironmentFrame ())
				static_cast<EnvironmentFrame*> (document->getEnvironmentFrame ())->reset ();
		}
	}
}

//-------------------------------------------------------------------

static void findEnvironment (const TerrainGenerator::Affector* affector, int familyId, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	if (affector->getType () == TGAT_environment)
	{
		if (safe_cast<const AffectorEnvironment*> (affector)->getFamilyId () == familyId)
		{
			CString* buffer = new CString;
			buffer->Format ("%s", affector->getName ());
			output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_find, TerrainGeneratorHelper::OutputData::T_info, buffer, affector));
		}
	}
}

//-------------------------------------------------------------------

static void findEnvironment (const TerrainGenerator::Layer* layer, int familyId, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	{
		//-- check affectors
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
			::findEnvironment (layer->getAffector (i), familyId, output);
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			::findEnvironment (layer->getLayer (i), familyId, output);
	}
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::findEnvironment (const TerrainGenerator* generator, int familyId, ArrayList<OutputData>& output)
{
	//-- query sublayers
	int i;
	for (i = 0; i < generator->getNumberOfLayers (); i++)
		::findEnvironment (generator->getLayer (i), familyId, output);
}

//-------------------------------------------------------------------

