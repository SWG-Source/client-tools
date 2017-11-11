// ======================================================================
//
// AnimationStateNameIdManager.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_AnimationStateNameIdManager_H
#define INCLUDED_AnimationStateNameIdManager_H

// ======================================================================

class AnimationStateNameId;
class CrcLowerString;
class UniqueNameList;

// ======================================================================

class AnimationStateNameIdManager
{
public:

	static void                        install();

	static AnimationStateNameId        createId(const CrcLowerString &stateName);
	static const AnimationStateNameId &getRootId();

	static const CrcLowerString       &getNameString(const AnimationStateNameId &id);

private:

	static void                        remove();

private:

	static bool                  ms_installed;
	static UniqueNameList        ms_stateNameList;
	static AnimationStateNameId  ms_rootId;

};

// ======================================================================

#endif
