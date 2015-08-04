//======================================================================
//
// SwgCuiAvatarCustomizationBase_CustomizationGroup.h
// copyright (c) 2002-2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAvatarCustomizationBase_CustomizationGroup_H
#define INCLUDED_SwgCuiAvatarCustomizationBase_CustomizationGroup_H

//======================================================================

#include "swgClientUserInterface/SwgCuiAvatarCustomizationBase.h"

#include "sharedGame/CustomizationManager.h"

//#include <vector>

class UIDataSource;
class CustomizationData;
class UIData;
class Object;
class SharedObjectTemplate;
class UIDataSource;

#include <vector>

//----------------------------------------------------------------------

struct SwgCuiAvatarCustomizationBase::CustomizationGroup
{
	//----------------------------------------------------------------------

	typedef stdvector<CustomizationManager::MorphParameter>::fwd MorphParameterVector;

	//-- @todo group name should be narrow, and we need some localization
	Unicode::String      name;
	MorphParameterVector horizontal;
	MorphParameterVector vertical;
	
	Unicode::String      varColorNames   [2];
	std::string          varColors       [2];

	std::string          varColorLinkedToHair      [2];
	std::string          varColorLinkedToSelf      [2][2];
	bool                 isVarColorHair            [2];
	bool                 isColorRandomizable       [2];
	bool                 isColorRandomizableGroup  [2];

	bool                 isHair;
	bool                 isMarking;
	bool                 isColorLinkedToHair;
	bool                 isColorLinkedToBody;

	std::string          pictureName;
	std::string          cameraLookAtBone;
	float                cameraYaw;
	bool                 cameraYawValid;

	float                cameraZoomFactor;

	CustomizationGroup   (const Object & obj, CustomizationData & customizationData, std::string const & species_gender, std::string const & groupName, CustomizationManager::CustomizationVector const & data);

	void                 populateMorphParams  (CustomizationData & cdata, CustomizationManager::CustomizationVector const & ds, MorphParameterVector & v) const;
	CustomizationManager::MorphParameter *     findMorphParam       (const std::string & paramName, MorphParameterVector & v) const;

	void                 randomize            (bool isGroup);
	void                 randomize            (MorphParameterVector & v, bool isGroup) const;

	int                  incrementTarget      (Object & obj, float increment);

	void                 computeDefaults      (const Object & obj, SharedObjectTemplate const * tmpl);

	void                 resetDefaults        (Object & obj, bool interpolate);

	void                 debugPrint           (std::string & result) const;
};

//======================================================================

#endif
