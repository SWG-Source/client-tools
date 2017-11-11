// ======================================================================
//
// CombatSpecialMoveVisualData.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_CombatSpecialMoveVisualData_H
#define INCLUDED_CombatSpecialMoveVisualData_H

// ======================================================================

class CrcString;

// ======================================================================

class CombatSpecialMoveVisualData
{
public:

	static void install(const char *filename);
	static bool lookupVisualData(int effectId, const CrcString *&appearanceFilename, float &timeToLive);

};

// ======================================================================

#endif
