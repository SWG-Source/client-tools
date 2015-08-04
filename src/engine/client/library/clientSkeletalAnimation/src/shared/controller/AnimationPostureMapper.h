// ======================================================================
//
// AnimationPostureMapper.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AnimationPostureMapper_H
#define INCLUDED_AnimationPostureMapper_H

// ======================================================================

class AnimationStateNameId;
class Iff;

// ======================================================================

class AnimationPostureMapper
{
public:

	static void install(const char *postureMapperFile);
	static bool mapPosture(int postureEnumValue, AnimationStateNameId &postureStateId);

private:

	class MapEntry;
	typedef stdvector<MapEntry>::fwd  MapEntryVector;

private:

	static void remove();

	static void load(Iff &iff);
	static void load_0000(Iff &iff);

private:

	static bool            ms_installed;
	static MapEntryVector  ms_mapEntryVector;

};

// ======================================================================

#endif
