//======================================================================
//
// NebulaLightningData.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_NebulaLightningData_H
#define INCLUDED_NebulaLightningData_H

//======================================================================

#include "sharedMath/Vector.h"

//----------------------------------------------------------------------

class NebulaLightningData
{
public:

	NebulaLightningData();

	uint16 lightningId;
	int nebulaId;
	uint32 syncStampStart;
	uint32 syncStampEnd;
	Vector endpoint0;
	Vector endpoint1;
};

//======================================================================

#endif
