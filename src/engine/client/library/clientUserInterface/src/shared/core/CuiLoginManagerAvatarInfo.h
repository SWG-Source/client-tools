//======================================================================
//
// CuiLoginManagerAvatarInfo.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiLoginManagerAvatarInfo_H
#define INCLUDED_CuiLoginManagerAvatarInfo_H

//======================================================================

#include "Unicode.h"
#include "sharedFoundation/NetworkId.h"

class CuiLoginManagerAvatarInfo
{

public:

	Unicode::String  name;
	std::string      objectTemplate;
	NetworkId        networkId;
	uint32           clusterId;
	std::string      planetName;
	int              characterType;
	int              characterLevel;
	std::string      characterSkillTemplate;

	                 CuiLoginManagerAvatarInfo ();
					 CuiLoginManagerAvatarInfo (const CuiLoginManagerAvatarInfo & tocopy);
	                 CuiLoginManagerAvatarInfo (uint32 _clusterId, const NetworkId & _networkID);

	void             clear ();
};

//======================================================================

#endif
