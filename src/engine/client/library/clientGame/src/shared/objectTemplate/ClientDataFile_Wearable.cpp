// ======================================================================
//
// ClientDataFile_Wearable.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientDataFile_Wearable.h"

#include "clientGame/ClientObject.h"
#include "clientGame/ContainerInterface.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/SkeletalAppearanceTemplate.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedGame/AssetCustomizationManager.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/MemoryBlockManagedObject.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"
#include "sharedObject/RangedIntCustomizationVariable.h"
#include "sharedObject/SlottedContainer.h"
#include <algorithm>
#include <string>

// ======================================================================

class ClientDataFile::Wearable::VariableSetting
{
public:

	VariableSetting(char const *name, int value);

	std::string const &getVariableName() const;
	int                getValue() const;

private:

	// Disabled.
	VariableSetting();
	VariableSetting(VariableSetting const &);             //lint -esym(754, VariableSetting::VariableSetting) // unreferenced // yes, defensive hiding.
	VariableSetting &operator =(VariableSetting const &);

private:

	std::string  m_variableName;
	int          m_value;

};

// ======================================================================
// private namespace
// ======================================================================

namespace ClientDataFile_WearableNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_MESH = TAG(M,E,S,H);
	Tag const TAG_WCSI = TAG(W,C,S,I);
	Tag const TAG_WEAR = TAG(W,E,A,R);
	Tag const TAG_OBJT = TAG(O,B,J,T);
	
}

using namespace ClientDataFile_WearableNamespace;

// ======================================================================
// class Wearable::VariableFactory
// ======================================================================

ClientDataFile::Wearable::VariableSetting::VariableSetting(char const *name, int value):
	m_variableName(name),
	m_value(value)
{
}

// ----------------------------------------------------------------------

inline std::string const &ClientDataFile::Wearable::VariableSetting::getVariableName() const
{
	return m_variableName;
}

// ----------------------------------------------------------------------

inline int ClientDataFile::Wearable::VariableSetting::getValue() const
{
	return m_value;
}

// ======================================================================
// class Wearable: PUBLIC STATIC
// ======================================================================

ClientDataFile::Wearable *ClientDataFile::Wearable::load_0000(Iff &iff)
{
	return new Wearable(iff);
}

// ----------------------------------------------------------------------

void ClientDataFile::Wearable::concludeApply(Object *wearerObject)
{
	//-- Lock wearables for the skeletal appearance.
	Appearance          *const baseAppearance   = wearerObject->getAppearance();
	SkeletalAppearance2 *const wearerAppearance = (baseAppearance ? baseAppearance->asSkeletalAppearance2() : 0);

	if (wearerAppearance)
		wearerAppearance->lockWearables();
}

// ======================================================================
// class Wearable: PUBLIC
// ======================================================================

ClientDataFile::Wearable::~Wearable()
{
	std::for_each(m_variableSettingVector.begin(), m_variableSettingVector.end(), PointerDeleter());

	if (NULL != m_objectTemplate)
	{
		m_objectTemplate->releaseReference();
	}

	if (NULL != m_appearanceTemplate)
	{
		AppearanceTemplateList::release(m_appearanceTemplate);
	}

	m_objectTemplate = 0;
	m_appearanceTemplate = 0;
}

// ----------------------------------------------------------------------

bool ClientDataFile::Wearable::apply(Object *wearerObject) const
{
	bool result = true;

	//-- Get wearer's appearance and customization data.
	Appearance          *const baseAppearance   = wearerObject->getAppearance();
	SkeletalAppearance2 *const wearerAppearance = (baseAppearance ? baseAppearance->asSkeletalAppearance2() : 0);
	NOT_NULL(wearerAppearance);

	CustomizationDataProperty *const wearerCdProperty = safe_cast<CustomizationDataProperty*>(wearerObject->getProperty(CustomizationDataProperty::getClassPropertyId()));
	CustomizationData *const wearerCustomizationData = (wearerCdProperty ? wearerCdProperty->fetchCustomizationData() : 0);

	//-- Loop through all the mesh generators in the wearable template and add all the customization
	//   variables for them.
	if (NULL != m_appearanceTemplate)
	{
		//-- Create the wearable object.
		Object *const wearableObject = new MemoryBlockManagedObject;
		
		//-- Add wearable object as child of wearer.  This child object will never have its appearance rendered.
		//   We add it as a child so it gets deleted when the parent is deleted.
		wearerObject->addChildObject_p(wearableObject);
		
		//-- Create the wearable customization data.
		CustomizationDataProperty *const wearableCdProperty = new CustomizationDataProperty(*wearableObject); //lint -esym(429, wearableCdProperty) // Custodial pointer is not freed // It's owned by wearableObject.
		wearableObject->addProperty(*wearableCdProperty);
		
		CustomizationData *wearableCustomizationData = wearableCdProperty->fetchCustomizationData();
		NOT_NULL(wearableCustomizationData);

		int const meshGeneratorCount = m_appearanceTemplate->getMeshGeneratorCount();
		for (int i = 0; i < meshGeneratorCount; ++i)
		{
			CrcString const &meshGeneratorName = m_appearanceTemplate->getMeshGeneratorName(i);
			
			int const addedVariableCount = AssetCustomizationManager::addCustomizationVariablesForAsset(meshGeneratorName, *wearableCustomizationData, true);
			UNREF(addedVariableCount);
		}
		
		//-- Hookup customization data to parent wearer object.
		if (wearerCustomizationData)
		{
			//-- make a link from wearable's /shared_owner directory to owner's /shared_owner/ customization variable directory
			IGNORE_RETURN(wearableCustomizationData->mountRemoteCustomizationData(*wearerCustomizationData, "/shared_owner/", "/shared_owner"));
		}
		else
			DEBUG_WARNING(true, ("Adding wearable to wearer object id [%s], template [%s], when wearer has no customization data. Stitched-in skin will not hue properly.", wearerObject->getNetworkId().getValueString().c_str(), wearerObject->getObjectTemplateName()));
		
		//-- Loop through all the variable settings specified for the client-baked wearable and set them.
		{
			VariableSettingVector::const_iterator const endIt = m_variableSettingVector.end();
			for (VariableSettingVector::const_iterator it = m_variableSettingVector.begin(); it != endIt; ++it)
			{
				VariableSetting const *const variableSetting = *it;
				NOT_NULL(variableSetting);
				
				// Get the CustomizationVariable associated with the specified variable name.
				std::string const &variableName = variableSetting->getVariableName();
				RangedIntCustomizationVariable *const variable = dynamic_cast<RangedIntCustomizationVariable*>(wearableCustomizationData->findVariable(variableName));
				if (variable)
				{
					int const newValue = variableSetting->getValue();
					
#ifdef _DEBUG
					int rangeMinInclusive;
					int rangeMaxExclusive;
					variable->getRange(rangeMinInclusive, rangeMaxExclusive);
					if ((newValue < rangeMinInclusive) || (newValue >= rangeMaxExclusive))
					{
						DEBUG_WARNING(true, ("client-baked wearable: variable [%s] applied to object template [%s] has out of range value: min/max/value=%d/%d/%d\n",
							variableName.c_str(), wearerObject->getObjectTemplateName(), rangeMinInclusive, rangeMaxExclusive - 1, newValue));
						result = false;
					}
#endif
					
					variable->setValue(newValue);
				}
				else
				{
					result = false;
					DEBUG_WARNING(true, ("client-baked wearable: variable [%s] is applied to object template [%s] but that variable does not exist.", variableName.c_str(), wearerObject->getObjectTemplateName()));
				}
			}
		}
		
		//-- Create the wearable appearance.
		Appearance *const wearableAppearance = m_appearanceTemplate->createAppearance();
		NOT_NULL(wearableAppearance);
		
		wearableObject->setAppearance(wearableAppearance);
		
		//-- Set wearable appearance's customization data.
		wearableAppearance->setCustomizationData(wearableCustomizationData);
		
		//-- Cause the wearer appearance to wear the wearable object.
		wearerAppearance->wear(wearableObject);
		
		//-- Release local references.
		wearableCustomizationData->release();
		if (wearerCustomizationData)
			wearerCustomizationData->release();
	}
	else if (NULL != m_objectTemplate)
	{
		ClientObject * const wearerClientObject = wearerObject->asClientObject();

		Object * const obj = m_objectTemplate->createObject();
		if (NULL != obj)
		{
			FormattedString<256> fstr;
			char const * const buf = fstr.sprintf("CdfObject_%s", m_objectTemplate->getName());
			obj->setDebugName(buf);

			ClientObject * const clientObject = obj->asClientObject();
			if (NULL != clientObject)
			{
				clientObject->endBaselines();

				int arrangement = m_objectWearableArrangement;
				//-- see if we need to compute an arrangement
				if (0 == arrangement)
				{
					SlottedContainer * const slotted = ContainerInterface::getSlottedContainer(*wearerClientObject);
					if (NULL != slotted)
					{
						Container::ContainerErrorCode tmp = Container::CEC_Success;
						if (!slotted->getFirstUnoccupiedArrangement(*clientObject, arrangement, tmp))
						{
							WARNING(true, ("ClientDataFile_Wearable unable to find unoccupied arrangement for [%s] on [%s]", clientObject->getDebugName(), wearerClientObject->getDebugName()));
							result = false;
						}
					}
				}

				ContainerInterface::transferItemToSlottedContainer(*wearerClientObject, *clientObject, arrangement);
				
			}

//			wearerObject->addChildObject_p(obj);
//			wearerAppearance->wear(obj);
		}
	}

	return result;
}

// ----------------------------------------------------------------------

void ClientDataFile::Wearable::preloadAssets() const
{
	if (NULL != m_objectTemplate)
	{
		m_objectTemplate->preloadAssets();
	}

	if (NULL != m_appearanceTemplate)
	{
		m_appearanceTemplate->preloadAssets();
	}
}

// ======================================================================
// class Wearable: PRIVATE
// ======================================================================

ClientDataFile::Wearable::Wearable(Iff &iff) :
m_objectTemplate(NULL),
m_objectWearableArrangement(0),
m_appearanceTemplate(NULL),
m_variableSettingVector()
{	
	//-- Handle load.
	iff.enterForm(TAG_WEAR);
	
	while (iff.getNumberOfBlocksLeft())
	{
		switch (iff.getCurrentName())
		{
		case TAG_MESH:
			{
				if (NULL == m_appearanceTemplate)
				{
					m_appearanceTemplate = new SkeletalAppearanceTemplate;
					//-- Fetch reference to newly-created non-shared skeletal appearance template.
					IGNORE_RETURN(AppearanceTemplateList::fetchNew(m_appearanceTemplate));
				}
					
				iff.enterChunk(TAG_MESH);
				{				
					char meshPathName[MAX_PATH * 2];
					
					iff.read_string(meshPathName, sizeof(meshPathName) - 1);
					IGNORE_RETURN(m_appearanceTemplate->addMeshGenerator(meshPathName));
				}
				iff.exitChunk(TAG_MESH);
			}
			break;

		case TAG_OBJT:
			{
				iff.enterChunk(TAG_OBJT);
				{
					char meshPathName[MAX_PATH * 2];
					iff.read_string(meshPathName, sizeof(meshPathName) - 1);
					m_objectTemplate = safe_cast<SharedObjectTemplate const *>(ObjectTemplateList::fetch(meshPathName));
					if (NULL == m_objectTemplate)
					{
						WARNING(true, ("ClientDataFile_Wearable failed to find object template [%s]", meshPathName));
					}
					m_objectWearableArrangement = iff.read_int32();
				}
				iff.exitChunk(TAG_OBJT);
			}
			break;
			
		case TAG_WCSI:
			{
				iff.enterChunk(TAG_WCSI);
				{
					char variableName[256];
					
					iff.read_string(variableName, sizeof(variableName) - 1);
					variableName[sizeof(variableName) - 1] = '\0';
					int const value = static_cast<int>(iff.read_int32());
					
					m_variableSettingVector.push_back(new VariableSetting(variableName, value));
				}
				iff.exitChunk(TAG_WCSI);
			}
			break;
			
		default:
			{
				char buffer[5];
				ConvertTagToString(iff.getCurrentName(), buffer);
				DEBUG_WARNING(true, ("ClientDataFile::Wearable: unsupported block type [%s].", buffer));
				iff.allowNonlinearFunctions();
				iff.goForward();
			}
		}
	}
	
	iff.exitForm(TAG_WEAR);
}

// ======================================================================
