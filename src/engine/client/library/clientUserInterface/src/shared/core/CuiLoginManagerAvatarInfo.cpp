//======================================================================
//
// CuiLoginManagerAvatarInfo.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiLoginManagerAvatarInfo.h"
#include "sharedNetworkMessages/ClientCentralMessages.h"

//======================================================================

CuiLoginManagerAvatarInfo::CuiLoginManagerAvatarInfo () :
name             (),
objectTemplate   (),
networkId        (),
clusterId        (0),
planetName       (),
characterType    (static_cast<int>(EnumerateCharacterId_Chardata::CT_normal)),
characterLevel   (0),
characterSkillTemplate()
{

}

//----------------------------------------------------------------------

CuiLoginManagerAvatarInfo::CuiLoginManagerAvatarInfo (const CuiLoginManagerAvatarInfo & tocopy) :
name             (tocopy.name),
objectTemplate   (tocopy.objectTemplate),
networkId        (tocopy.networkId),
clusterId        (tocopy.clusterId),
planetName       (tocopy.planetName),
characterType    (tocopy.characterType),
characterLevel   (tocopy.characterLevel),
characterSkillTemplate(tocopy.characterSkillTemplate)
{

}

//----------------------------------------------------------------------

CuiLoginManagerAvatarInfo::CuiLoginManagerAvatarInfo (uint32 _clusterId, const NetworkId & _networkId) :
name             (),
objectTemplate   (),
networkId        (_networkId),
clusterId        (_clusterId),
planetName       (),
characterType    (static_cast<int>(EnumerateCharacterId_Chardata::CT_normal)),
characterLevel   (0),
characterSkillTemplate()
{

}

//----------------------------------------------------------------------

void CuiLoginManagerAvatarInfo::clear ()
{
	name.clear ();
	objectTemplate.clear ();
	networkId = NetworkId::cms_invalid;
	clusterId = 0;
	planetName.clear ();
	characterType = static_cast<int>(EnumerateCharacterId_Chardata::CT_normal);
	characterLevel = 0;
	characterSkillTemplate.clear();
}

//======================================================================
