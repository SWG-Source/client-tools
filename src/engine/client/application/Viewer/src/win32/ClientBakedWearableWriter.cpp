// ======================================================================
//
// ClientBakedWearableWriter.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstViewer.h"
#include "ClientBakedWearableWriter.h"

#include "clientSkeletalAnimation/SkeletalAppearanceTemplate.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedGame/AssetCustomizationManager.h"
#include "sharedMath/PaletteArgb.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/Object.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"

#include <stdio.h>
#include <string>

// ======================================================================

namespace ClientBakedWearableWriterNamespace
{
	void WriteVariableData(const std::string &fullVariablePathName, const CustomizationVariable *customizationVariable, void *context);

	CustomizationData *s_defaultCustomizationData;
}

using namespace ClientBakedWearableWriterNamespace;

// ======================================================================

void ClientBakedWearableWriterNamespace::WriteVariableData(const std::string &fullVariablePathName, const CustomizationVariable *customizationVariable, void *context)
{
	NOT_NULL(customizationVariable);
	NOT_NULL(context);
	NOT_NULL(s_defaultCustomizationData);
	FATAL(fullVariablePathName.empty(), ("fullVariablePathName is zero-length."));

	//-- Ignore shared_owner variables.
	if (static_cast<int>(fullVariablePathName.find("/shared_owner/")) >= 0)
	{
		// Skip shared_owner variables.  The baked wearables will attach to the shared_owner
		// variables of the wearer like normal wearables do.
		return;
	}

	//-- Ignore variables that are set to the same value as the default.
	RangedIntCustomizationVariable const *const setRangedIntVariable     = dynamic_cast<RangedIntCustomizationVariable const*>(customizationVariable);
	RangedIntCustomizationVariable const *const defaultRangedIntVariable = dynamic_cast<RangedIntCustomizationVariable const*>(s_defaultCustomizationData->findConstVariable(fullVariablePathName));

	FATAL(!setRangedIntVariable || !defaultRangedIntVariable, ("variable [%s] not a ranged int-compatible variable, programmer error.", fullVariablePathName.c_str()));
	int const setValue = setRangedIntVariable->getValue();
	if (setValue == defaultRangedIntVariable->getValue())
	{
		//-- Skip it --- nothing to do if we're already the default.
		return;
	}

	FILE *const outputFile = static_cast<FILE*>(context);

	//-- Write the wearable customization int setting override.	
	fprintf(outputFile, "\t\t\tWearableCustomizationSetInt(\"%s\", %d)\n", fullVariablePathName.c_str(), setValue);
}

// ======================================================================

bool ClientBakedWearableWriter::write(Object const &object, char const *outputFileName)
{
	//-- Get the skeletal appearance template.
	Appearance const *const appearance = object.getAppearance();
	if (!appearance)
	{
		DEBUG_WARNING(true, ("Object has a NULL appearance."));
		return false;
	}

	SkeletalAppearanceTemplate const *const appearanceTemplate = dynamic_cast<SkeletalAppearanceTemplate const*>(appearance->getAppearanceTemplate());
	if (!appearanceTemplate)
	{
		DEBUG_WARNING(true, ("Object's appearance template is NULL or not a skeletal appearance template."));
		return false;
	}

	//-- Open the output file for writing.
	FILE *outputFile = fopen(outputFileName, "w");
	if (!outputFile)
	{
		DEBUG_WARNING(true, ("Failed to open file [%s] for writing.", outputFileName));
		return false;
	}


	//-- Begin output.
	CustomizationData *defaultCustomizationData = new CustomizationData(const_cast<Object&>(object));
	defaultCustomizationData->fetch();

	s_defaultCustomizationData = defaultCustomizationData;

	fprintf(outputFile, "\t\t\tBeginWearable\n");
	{
		//-- Add mesh generators.
		int const count = appearanceTemplate->getMeshGeneratorCount();
		for (int i = 0; i < count; ++i)
		{
			char const *const mgnName = appearanceTemplate->getMeshGeneratorName(i).getString();
			if (mgnName && (strlen(mgnName) > 0))
			{
				fprintf(outputFile, "\t\t\tUseMeshGenerator(\"%s\")\n", mgnName);
				
				// Collect default customization variable settings for this asset.
				AssetCustomizationManager::addCustomizationVariablesForAsset(appearanceTemplate->getMeshGeneratorName(i), *defaultCustomizationData, true);
			}
		}

		//-- Add customization variables.
		CustomizationDataProperty const *const cdProperty = dynamic_cast<CustomizationDataProperty const*>(object.getProperty(CustomizationDataProperty::getClassPropertyId()));
		if (cdProperty)
		{
			CustomizationData const *const customizationData = cdProperty->fetchCustomizationData();
			if (customizationData)
			{
				customizationData->iterateOverConstVariables(WriteVariableData, outputFile);
			}
		}
	}

	//-- Release local references.
	defaultCustomizationData->release();
	s_defaultCustomizationData = NULL;

	//-- End output.
	fprintf(outputFile, "\t\t\tEndWearable\n");

	//-- Close output file.
	fclose(outputFile);

	// Success.
	return true;
}

// ======================================================================
