// ======================================================================
//
// AnimationPriorityMap.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AnimationPriorityMap_H
#define INCLUDED_AnimationPriorityMap_H

// ======================================================================

class CrcLowerString;
class Iff;

// ======================================================================
/**
 * Maps animation priority group names into a numeric priority level.
 *
 * Animations specify priorities in terms of priority group names.
 * Priority group names are just CrcLowerString values.  Strings are
 * used for groups so that it is easy to adjust the numeric priority
 * of animations simply by adjusting the priority value associated
 * with the priority group name.
 *
 * Higher priority values take precendence over lower priority values.
 */

class AnimationPriorityMap
{
public:

	static void                  install(const char *priorityMapFileName);

	static int                   getPriorityValue(const CrcLowerString &priorityGroupName);

	static int                   getPriorityGroupCount();
	static const CrcLowerString &getPriorityGroupName(int index);
	static int                   getPriorityGroupPriority(int index);

private:

	typedef stdmap<CrcLowerString, int>::fwd  StringIntMap;

private:

	static void                  remove();
	static void                  load_0000(Iff &iff);

private:

	static const int     cms_defaultPriority;

	static bool          ms_installed;
	static StringIntMap *ms_priorityGroupMap;

};

// ======================================================================

#endif
