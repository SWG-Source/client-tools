//
// TerrainGeneratorHelperShader.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "TerrainGeneratorHelper.h"

#include "sharedTerrain/AffectorRibbon.h"
#include "sharedTerrain/AffectorShader.h"
#include "sharedTerrain/AffectorRoad.h"
#include "sharedTerrain/AffectorRiver.h"
#include "sharedTerrain/Filter.h"
#include "ShaderGroupFrame.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

static int findUniqueFamilyId (const ShaderGroup& shaderGroup)
{
	int i = 1;
	while (shaderGroup.hasFamily (i))
		i++;

	return i;
}

//-------------------------------------------------------------------

bool TerrainGeneratorHelper::usesShaderFamily (const TerrainGenerator::Layer* layer, int familyId)
{
	{
		//-- check filters
		int i;
		for (i = 0; i < layer->getNumberOfFilters (); i++)
			if (layer->getFilter (i)->getType () == TGFT_shader)
			{
				const FilterShader* const filter = safe_cast<const FilterShader*> (layer->getFilter (i));

				if (familyId == filter->getFamilyId ())
					return true;
			}
	}

	{
		//-- check affectors
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
		{
			if (layer->getAffector (i)->getType () == TGAT_shaderConstant)
			{
				const AffectorShaderConstant* affector = safe_cast<const AffectorShaderConstant*> (layer->getAffector (i));

				if (familyId == affector->getFamilyId ())
					return true;
			}

			if (layer->getAffector (i)->getType () == TGAT_shaderReplace)
			{
				const AffectorShaderReplace* affector = safe_cast<const AffectorShaderReplace*> (layer->getAffector (i));

				if (familyId == affector->getSourceFamilyId ())
					return true;

				if (familyId == affector->getDestinationFamilyId ())
					return true;
			}

			if (layer->getAffector (i)->getType () == TGAT_road)
			{
				const AffectorRoad* affector = safe_cast<const AffectorRoad*> (layer->getAffector (i));

				if (familyId == affector->getFamilyId ())
					return true;
			}

			if (layer->getAffector (i)->getType () == TGAT_river)
			{
				const AffectorRiver* affector = safe_cast<const AffectorRiver*> (layer->getAffector (i));

				if (familyId == affector->getBankFamilyId ())
					return true;

				if (familyId == affector->getBottomFamilyId ())
					return true;
			}
/*
			if (layer->getAffector (i)->getType () == TGAT_ribbon)
			{
				const AffectorRibbon* affector = safe_cast<const AffectorRibbon*> (layer->getAffector (i));

				if(familyId == affector->getFamilyId ())
				{
					return true;
				}
			}
*/
		}
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			if (usesShaderFamily (layer->getLayer (i), familyId))
				return true;
	}

	return false;
}

//-------------------------------------------------------------------

bool TerrainGeneratorHelper::usesShaderFamily (const TerrainGenerator* generator, int familyId)
{
	//-- check sublayers
	int i;
	for (i = 0; i < generator->getNumberOfLayers (); i++)
		if (usesShaderFamily (generator->getLayer (i), familyId))
			return true;

	return false;
}

//-------------------------------------------------------------------

#if 0

static void buildShaderGroup (ShaderGroup& trimShaderGroup, const ShaderGroup& shaderGroup, const TerrainGenerator::Affector* affector)
{
	if (affector->getType () == TGAT_shaderConstant)
	{
		//-- find family used
		const AffectorShaderConstant* affectorShaderConstant = static_cast<const AffectorShaderConstant*> (affector);

		const int familyId = affectorShaderConstant->getFamilyId ();

		if (!trimShaderGroup.hasFamily (familyId))
		{
			trimShaderGroup.addFamily (familyId, shaderGroup.getFamilyName (familyId), shaderGroup.getFamilyColor (familyId));

			trimShaderGroup.setFamilyFeatherClamp (familyId, shaderGroup.getFamilyFeatherClamp (familyId));
			trimShaderGroup.setFamilyShaderSize (familyId, shaderGroup.getFamilyShaderSize (familyId));

			const int n = shaderGroup.getFamilyNumberOfChildren (familyId);
			int i;
			for (i = 0; i < n; i++)
				trimShaderGroup.addChild (shaderGroup.getFamilyChild (familyId, i));
		}
	}
}

//-------------------------------------------------------------------

static void buildShaderGroup (ShaderGroup& trimShaderGroup, const ShaderGroup& shaderGroup, const TerrainGenerator::Filter* filter)
{
	if (filter->getType () == TGFT_shader)
	{
		//-- find family used
		const FilterShader* filterShader = static_cast<const FilterShader*> (filter);

		const int familyId = filterShader->getFamilyId ();

		if (!trimShaderGroup.hasFamily (familyId))
		{
			trimShaderGroup.addFamily (familyId, shaderGroup.getFamilyName (familyId), shaderGroup.getFamilyColor (familyId));

			trimShaderGroup.setFamilyFeatherClamp (familyId, shaderGroup.getFamilyFeatherClamp (familyId));
			trimShaderGroup.setFamilyShaderSize (familyId, shaderGroup.getFamilyShaderSize (familyId));

			const int n = shaderGroup.getFamilyNumberOfChildren (familyId);
			int i;
			for (i = 0; i < n; i++)
				trimShaderGroup.addChild (shaderGroup.getFamilyChild (familyId, i));
		}
	}
}

//-------------------------------------------------------------------

static void buildShaderGroup (ShaderGroup& trimShaderGroup, const ShaderGroup& shaderGroup, const TerrainGenerator::Layer* layer)
{
	{
		//-- check filters
		int i;
		for (i = 0; i < layer->getNumberOfFilters (); i++)
			buildShaderGroup (trimShaderGroup, shaderGroup, layer->getFilter (i));
	}

	{
		//-- check affectors
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
			buildShaderGroup (trimShaderGroup, shaderGroup, layer->getAffector (i));
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			buildShaderGroup (trimShaderGroup, shaderGroup, layer->getLayer (i));
	}
}

#endif

//-------------------------------------------------------------------

void TerrainGeneratorHelper::saveShaderGroup (Iff& iff, const ShaderGroup& shaderGroup, const TerrainGenerator::Affector* affector)
{
	UNREF (affector);

	//-- write out a shader group that only consists of families used by affector
//	ShaderGroup trimShaderGroup;
//	buildShaderGroup (trimShaderGroup, shaderGroup, affector);
//	trimShaderGroup.save (iff);

	shaderGroup.save (iff);
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::saveShaderGroup (Iff& iff, const ShaderGroup& shaderGroup, const TerrainGenerator::Filter* filter)
{
	UNREF (filter);

	//-- write out a shader group that only consists of families used by filter
//	ShaderGroup trimShaderGroup;
//	buildShaderGroup (trimShaderGroup, shaderGroup, filter);
//	trimShaderGroup.save (iff);

	shaderGroup.save (iff);
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::saveShaderGroup (Iff& iff, const ShaderGroup& shaderGroup, const TerrainGenerator::Layer* layer)
{
	UNREF (layer);

	//-- write out a shader group that only consists of families used by layer
//	ShaderGroup trimShaderGroup;
//	buildShaderGroup (trimShaderGroup, shaderGroup, layer);
//	trimShaderGroup.save (iff);

	shaderGroup.save (iff);
}

//-------------------------------------------------------------------

ShaderGroup* TerrainGeneratorHelper::loadShaderGroup (Iff& iff)
{
	ShaderGroup* shaderGroup = new ShaderGroup;
	shaderGroup->load (iff);

	return shaderGroup;
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::fixupShader (TerrainEditorDoc* document, ShaderGroup& destinationShaderGroup, const ShaderGroup& sourceShaderGroup, TerrainGenerator::Affector* affector)
{
	//-- does this affector need fixing?
	if (affector->getType () == TGAT_shaderConstant)
	{
		AffectorShaderConstant* const affectorShaderConstant = safe_cast<AffectorShaderConstant*> (affector);

		//-- get the old id
		const int oldId = affectorShaderConstant->getFamilyId ();

		//-- get the old name
		const char* oldName = sourceShaderGroup.getFamilyName (oldId);
		NOT_NULL (oldName);

		//-- look up new name
		if (destinationShaderGroup.hasFamily (oldName))
		{
			//-- reset the id
			const int newId = destinationShaderGroup.getFamilyId (oldName);

			affectorShaderConstant->setFamilyId (newId);
		}
		else
		{
			//-- copy family over
			const int newId = findUniqueFamilyId (destinationShaderGroup);

			affectorShaderConstant->setFamilyId (newId);

			destinationShaderGroup.addFamily (newId, oldName, sourceShaderGroup.getFamilyColor (oldId));

			destinationShaderGroup.setFamilyFeatherClamp (newId, sourceShaderGroup.getFamilyFeatherClamp (oldId));
			destinationShaderGroup.setFamilyShaderSize (newId, sourceShaderGroup.getFamilyShaderSize (oldId));

			const int n = sourceShaderGroup.getFamilyNumberOfChildren (oldId);
			int i;
			for (i = 0; i < n; i++)
			{
				ShaderGroup::FamilyChildData fcd = sourceShaderGroup.getFamilyChild (oldId, i);
				fcd.familyId = newId;

				destinationShaderGroup.addChild (fcd);

				static_cast<ShaderGroupFrame*> (document->getShaderGroupFrame ())->reset ();
			}
		}
	}
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::fixupShader (TerrainEditorDoc* document, ShaderGroup& destinationShaderGroup, const ShaderGroup& sourceShaderGroup, TerrainGenerator::Filter* filter)
{
	//-- does this filter need fixing?
	if (filter->getType () == TGFT_shader)
	{
		FilterShader* const filterShader = safe_cast<FilterShader*> (filter);

		//-- get the old id
		const int oldId = filterShader->getFamilyId ();

		//-- get the old name
		const char* oldName = sourceShaderGroup.getFamilyName (oldId);
		NOT_NULL (oldName);

		//-- look up new name
		if (destinationShaderGroup.hasFamily (oldName))
		{
			//-- reset the id
			const int newId = destinationShaderGroup.getFamilyId (oldName);

			filterShader->setFamilyId (newId);
		}
		else
		{
			//-- copy family over
			const int newId = findUniqueFamilyId (destinationShaderGroup);

			filterShader->setFamilyId (newId);

			destinationShaderGroup.addFamily (newId, oldName, sourceShaderGroup.getFamilyColor (oldId));

			const int n = sourceShaderGroup.getFamilyNumberOfChildren (oldId);
			int i;
			for (i = 0; i < n; i++)
			{
				ShaderGroup::FamilyChildData fcd = sourceShaderGroup.getFamilyChild (oldId, i);
				fcd.familyId = newId;

				destinationShaderGroup.addChild (fcd);
			}

			if (document->getShaderGroupFrame ())
				static_cast<ShaderGroupFrame*> (document->getShaderGroupFrame ())->reset ();
		}
	}
}

//-------------------------------------------------------------------

static void findShader (const TerrainGenerator::Filter* filter, int familyId, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	if (filter->getType () == TGFT_shader)
	{
		if (safe_cast<const FilterShader*> (filter)->getFamilyId () == familyId)
		{
			CString* buffer = new CString;
			buffer->Format ("%s", filter->getName ());
			output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_find, TerrainGeneratorHelper::OutputData::T_info, buffer, filter));
		}
	}
}

//-------------------------------------------------------------------

static void findShader (const TerrainGenerator::Affector* affector, int familyId, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	{
		if (affector->getType () == TGAT_shaderConstant)
		{
			if (safe_cast<const AffectorShaderConstant*> (affector)->getFamilyId () == familyId)
			{
				CString* buffer = new CString;
				buffer->Format ("%s", affector->getName ());
				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_find, TerrainGeneratorHelper::OutputData::T_info, buffer, affector));
			}
		}
	}

	{
		if (affector->getType () == TGAT_shaderReplace)
		{
			if (safe_cast<const AffectorShaderReplace*> (affector)->getSourceFamilyId () == familyId)
			{
				CString* buffer = new CString;
				buffer->Format ("%s", affector->getName ());
				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_find, TerrainGeneratorHelper::OutputData::T_info, buffer, affector));
			}

			if (safe_cast<const AffectorShaderReplace*> (affector)->getDestinationFamilyId () == familyId)
			{
				CString* buffer = new CString;
				buffer->Format ("%s", affector->getName ());
				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_find, TerrainGeneratorHelper::OutputData::T_info, buffer, affector));
			}
		}
	}

	{
		if (affector->getType () == TGAT_road)
		{
			if (safe_cast<const AffectorRoad*> (affector)->getFamilyId () == familyId)
			{
				CString* buffer = new CString;
				buffer->Format ("%s", affector->getName ());
				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_find, TerrainGeneratorHelper::OutputData::T_info, buffer, affector));
			}
		}
	}

	{
		if (affector->getType () == TGAT_river)
		{
			if (safe_cast<const AffectorRiver*> (affector)->getBankFamilyId () == familyId || safe_cast<const AffectorRiver*> (affector)->getBottomFamilyId () == familyId)
			{
				CString* buffer = new CString;
				buffer->Format ("%s", affector->getName ());
				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_find, TerrainGeneratorHelper::OutputData::T_info, buffer, affector));
			}
		}
	}

/*
	{
		if (affector->getType () == TGAT_ribbon)
		{
			if(safe_cast<const AffectorRibbon*> (affector)->getFamilyId () == familyId)
			{
				CString* buffer = new CString;
				buffer->Format ("%s", affector->getName ());
				output.add (TerrainGeneratorHelper::OutputData (TerrainGeneratorHelper::OutputData::M_find, TerrainGeneratorHelper::OutputData::T_info, buffer, affector));
			}
		}
	}
*/
}

//-------------------------------------------------------------------

static void findShader (const TerrainGenerator::Layer* layer, int familyId, ArrayList<TerrainGeneratorHelper::OutputData>& output)
{
	{
		//-- check filters
		int i;
		for (i = 0; i < layer->getNumberOfFilters (); i++)
			::findShader (layer->getFilter (i), familyId, output);
	}

	{
		//-- check affectors
		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
			::findShader (layer->getAffector (i), familyId, output);
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			::findShader (layer->getLayer (i), familyId, output);
	}
}

//-------------------------------------------------------------------

void TerrainGeneratorHelper::findShader (const TerrainGenerator* generator, int familyId, ArrayList<OutputData>& output)
{
	//-- query sublayers
	int i;
	for (i = 0; i < generator->getNumberOfLayers (); i++)
		::findShader (generator->getLayer (i), familyId, output);
}

//-------------------------------------------------------------------

