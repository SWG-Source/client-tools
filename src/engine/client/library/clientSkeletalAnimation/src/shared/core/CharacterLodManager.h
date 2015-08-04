// ======================================================================
//
// CharacterLodManager.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CharacterLodManager_H
#define INCLUDED_CharacterLodManager_H

// ======================================================================

class Object;
class Vector;

// ======================================================================

class CharacterLodManager
{
public:

	typedef bool (*ManageLodCallback)(Object &object);

public:

	static void install();

	static void setManageLodCallback(ManageLodCallback callback);

	static void getConfiguration(bool &enabled, float &firstLodCount, int &everyOtherFrameSkinningCharacterCount, int &hardSkinningCharacterCount);
	static void setConfiguration(bool enabled, float firstLodCount, int everyOtherFrameSkinningCharacterCount, int hardSkinningCharacterCount);

	static bool isEnabled();

	static void addCharacter(Object *object);
	static void planNextFrame(Vector const &cameraPosition_w);

};

// ======================================================================

#endif
