// ======================================================================
//
// LightsaberCollisionManager.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_LightsaberCollisionManager_H
#define INCLUDED_LightsaberCollisionManager_H

// ======================================================================

class LightsaberAppearance;

// ======================================================================

class LightsaberCollisionManager
{
public:

	static void install();
	static void alter(float elapsedTime);

	static void addLightsaberBlade(LightsaberAppearance const &appearance, int bladeIndex);
	static void removeLightsaberBlade(LightsaberAppearance const &appearance, int bladeIndex);

};

// ======================================================================

#endif
