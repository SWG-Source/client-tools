//======================================================================
//
// SwgCuiAvatarCustomizationBase_CustomizationGroup.cpp
// copyright (c) 2002-2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAvatarCustomizationBase_CustomizationGroup.h"

#include "sharedGame/CustomizationManager.h"
#include "sharedGame/CustomizationManager_MorphParameter.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "clientGraphics/GraphicsOptionTags.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedMath/PaletteArgb.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"
#include "sharedObject/Object.h"
#include "sharedRandom/Random.h"
#include "sharedFile/Iff.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include "UnicodeUtils.h"
#include <vector>

//======================================================================

namespace SwgCuiAvatarCustomizationBaseNamespace
{
	Unicode::String const cms_ageName = Unicode::narrowToWide ("@ui_cust:age");
	int const cms_maxColors = 2;
	
	std::string s_randomCustomizationRangesDatatable = "datatables/player/random_customization_ranges.iff";
}

using namespace SwgCuiAvatarCustomizationBaseNamespace;

//======================================================================

SwgCuiAvatarCustomizationBase::CustomizationGroup::CustomizationGroup   (const Object & obj, CustomizationData & cdata, std::string const & species_gender, std::string const & groupName, CustomizationManager::CustomizationVector const & data) :
name                (Unicode::narrowToWide (groupName)),
horizontal          (),
vertical            (),
isHair              (false),
isMarking           (false),
isColorLinkedToHair (false),
isColorLinkedToBody (false),
pictureName         (),
cameraLookAtBone    (),
cameraYaw           (0.0f),
cameraYawValid      (false),
cameraZoomFactor    (1.0f)
{
	UNREF(cdata);

	CustomizationManager::CustomizationGroupSharedData const & groupSharedData = CustomizationManager::getCustomizationGroupSharedData(groupName);
	CustomizationManager::CustomizationGroupInstanceData const & groupInstanceData = CustomizationManager::getCustomizationGroupInstanceData(species_gender, groupName);
	isHair = groupSharedData.isHair;
	isMarking = groupSharedData.isMarking;
	isColorLinkedToHair = groupInstanceData.isColorLinkedToHair;
	isColorLinkedToBody = groupInstanceData.isColorLinkedToBody;
	cameraLookAtBone = groupInstanceData.lookAtBone;
	cameraZoomFactor = groupInstanceData.zoomFactor;

	CustomizationManager::CustomizationVector horizontalSliderVector;
	CustomizationManager::CustomizationVector verticalSliderVector;
	CustomizationManager::CustomizationVector colorVector;

	for(CustomizationManager::CustomizationVector::const_iterator i = data.begin(); i != data.end(); ++i)
	{
		if(i->type == "hslider")
		{
			horizontalSliderVector.push_back(*i);
		}
		else if(i->type == "vslider")
		{
			verticalSliderVector.push_back(*i);
		}
		else if(i->type == "color")
		{
			colorVector.push_back(*i);
		}
		else
		{
			DEBUG_FATAL(true, ("Unknown customization type [%s] found for species_gender [%s] and group [%s]", i->type.c_str(), species_gender.c_str(), groupName.c_str()));
		}
	}

	DEBUG_FATAL(horizontalSliderVector.size() + verticalSliderVector.size() + colorVector.size() != data.size(), ("Wrong number of customization types found"));


	populateMorphParams (cdata, horizontalSliderVector, horizontal);
	populateMorphParams (cdata, verticalSliderVector, horizontal);

	DEBUG_FATAL(colorVector.size() > cms_maxColors, ("Too many colors found [%d], max is [%d]", colorVector.size(), cms_maxColors));

	if(colorVector.size() != 0)
	{
		for (int j = 0; j < cms_maxColors; ++j)
		{
			isVarColorHair           [j] = false;
			isColorRandomizable      [j] = true;
			isColorRandomizableGroup [j] = false;

			if(j < static_cast<int>(colorVector.size()))
			{
				CustomizationManager::Customization colorCustomization = colorVector[j];
				varColors[j] = colorCustomization.variables;
				varColorNames[j] = Unicode::narrowToWide(colorCustomization.name);
	

				//TODO do I need to worry about these?
//				data->GetPropertyNarrow   (prop_colorLinked,              varColorLinkedToHair [j]);
//				data->GetPropertyNarrow   (prop_colorLinkedToSelf0,       varColorLinkedToSelf [j][0]);
//				data->GetPropertyNarrow   (prop_colorLinkedToSelf1,       varColorLinkedToSelf [j][1]);

				isVarColorHair[j] = colorCustomization.isVarHairColor;
				isColorRandomizable[j] = colorCustomization.randomizable;
				isColorRandomizableGroup[j] = colorCustomization.randomizableGroup;

				//-- setup linked color defaults 
				//-- these are used to link the human male facial hair to his head hair color
				if (varColorLinkedToHair [j].empty () && isVarColorHair [j])
					varColorLinkedToHair [j] = varColors [j];
			}
		}
	}


//	cameraYawValid = group_ns.GetPropertyFloat  (prop_cameraYaw,        cameraYaw);

	//-- convert cameraYaw from degrees to radians
//	if (cameraYawValid)
//		cameraYaw = cameraYaw * PI / 180.0f;
	
	ObjectTemplate const * const tmp = obj.getObjectTemplate();
	SharedObjectTemplate const * const tmpl = dynamic_cast<SharedObjectTemplate const *>(tmp);
	if(tmpl)
	{
		computeDefaults (obj, tmpl);
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::CustomizationGroup::debugPrint (std::string & result) const
{
	{
		for (MorphParameterVector::const_iterator it = horizontal.begin (); it != horizontal.end (); ++it)
		{
			const CustomizationManager::MorphParameter & mp = *it;
			mp.debugPrint (result);
		}
	}

	for (MorphParameterVector::const_iterator it = vertical.begin (); it != vertical.end (); ++it)
	{
		const CustomizationManager::MorphParameter & mp = *it;
		mp.debugPrint (result);
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::CustomizationGroup::populateMorphParams (CustomizationData & cdata, CustomizationManager::CustomizationVector const & ds, MorphParameterVector & v) const
{
	DEBUG_FATAL (&v != &horizontal && &v != &vertical, ("bad vector"));
	
	for(CustomizationManager::CustomizationVector::const_iterator i = ds.begin(); i != ds.end(); ++i)
	{
		const CustomizationManager::MorphParameter morphParam (cdata, *i);

		if (morphParam.name == cms_ageName)
		{
			if (!GraphicsOptionTags::get (TAG (D,O,T,3)))
				continue;
		}

		v.push_back (morphParam);
	}
}

//----------------------------------------------------------------------

CustomizationManager::MorphParameter * SwgCuiAvatarCustomizationBase::CustomizationGroup::findMorphParam (const std::string & paramName, MorphParameterVector & v) const
{
	for (MorphParameterVector::iterator it = v.begin (); it != v.end (); ++it)
	{
		CustomizationManager::MorphParameter & morphParam = *it;

		if (Unicode::caseInsensitiveCompare (morphParam.name, paramName))
			return &morphParam;
	}
	
	return 0;
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::CustomizationGroup::randomize (bool isGroup)
{
	randomize (horizontal, isGroup);
	randomize (vertical,   isGroup);
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::CustomizationGroup::randomize (MorphParameterVector & v, bool isGroup) const
{
	DataTable * rangesDatatable = DataTableManager::getTable(s_randomCustomizationRangesDatatable, true);

	for (MorphParameterVector::iterator it = v.begin (); it != v.end (); ++it)
	{
		CustomizationManager::MorphParameter & morphParam = *it;
		if (morphParam.randomizable || (isGroup && morphParam.randomizableGroup))
		{
			real targetRandom = 0.0;
			real min = 0.0;
			real max = 1.0;

			if (rangesDatatable && rangesDatatable->searchColumnString(0, Unicode::wideToNarrow(morphParam.name)) != -1)
			{
				int row = rangesDatatable->searchColumnString(0, Unicode::wideToNarrow(morphParam.name));
				min = rangesDatatable->getFloatValue(1, row);
				max = rangesDatatable->getFloatValue(2, row);
			}
			
			targetRandom = Random::randomReal();
			// set the targetRandom in the desired range
			targetRandom = min + (targetRandom * (max - min));
			// since morphParam.setTarget does a 1.0 - the arg, pass it in as the reverse 
			targetRandom = 1.0f - targetRandom;

			morphParam.setTarget(targetRandom);
		}
	}
}

//----------------------------------------------------------------------

int SwgCuiAvatarCustomizationBase::CustomizationGroup::incrementTarget (Object & obj, float increment)
{
	int count = 0;

	{
		for (MorphParameterVector::iterator it = horizontal.begin (); it != horizontal.end (); ++it)
		{
			CustomizationManager::MorphParameter & morphParam = *it;

			if (morphParam.isDiscrete)
			{
				if (morphParam.current != morphParam.target)
				{
					morphParam.update (obj, morphParam.target);
					++count;
				}
			}
			else  if ((*it).incrementTarget (obj, increment))
				++count;
		}
	}
	
	for (MorphParameterVector::iterator it = vertical.begin (); it != vertical.end (); ++it)
	{
		CustomizationManager::MorphParameter & morphParam = *it;

		if (morphParam.isDiscrete)
		{
			if (morphParam.current != morphParam.target)
			{
				morphParam.update (obj, morphParam.target);
				++count;
			}
		}
		else  if ((*it).incrementTarget (obj, increment))
			++count;
	}

	return count;
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::CustomizationGroup::computeDefaults (Object const & obj, SharedObjectTemplate const * const tmpl)
{
	{
		for (MorphParameterVector::iterator it = horizontal.begin (); it != horizontal.end (); ++it)
			(*it).computeDefaults (obj, tmpl);
	}

	for (MorphParameterVector::iterator it = vertical.begin (); it != vertical.end (); ++it)
		(*it).computeDefaults (obj, tmpl);
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::CustomizationGroup::resetDefaults (Object & obj, bool interpolate)
{
	{
		for (MorphParameterVector::iterator it = horizontal.begin (); it != horizontal.end (); ++it)
			(*it).resetDefaults (obj, interpolate);
	}

	for (MorphParameterVector::iterator it = vertical.begin (); it != vertical.end (); ++it)
		(*it).resetDefaults (obj, interpolate);
}

//======================================================================
