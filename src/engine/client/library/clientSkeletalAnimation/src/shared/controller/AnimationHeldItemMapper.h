// ======================================================================
//
// AnimationHeldItemMapper.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AnimationHeldItemMapper_H
#define INCLUDED_AnimationHeldItemMapper_H

// ======================================================================

class AnimationStateNameId;
class Iff;

// ======================================================================

class AnimationHeldItemMapper
{
public:

	static void install(const char *heldItemMapperFileName);

	static bool mapHeldItem(const std::string &heldItemAppearanceTemplateName, AnimationStateNameId &heldItemStateId);

private:

	struct MapEntry;
	typedef stdvector<MapEntry*>::fwd  MapEntryVector;

private:

	static void remove();

private:

	static void load(Iff &iff);
	static void load_0000(Iff &iff);

private:

	static bool           ms_installed;
	static MapEntryVector ms_mapEntryVector;

};

// ======================================================================

#endif
