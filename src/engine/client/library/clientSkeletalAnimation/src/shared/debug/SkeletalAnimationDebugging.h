// ======================================================================
//
// SkeletalAnimationDebugging.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SkeletalAnimationDebugging_H
#define INCLUDED_SkeletalAnimationDebugging_H

// ======================================================================

class Appearance;
class Object;

// ======================================================================

class SkeletalAnimationDebugging
{
public:

	static void          install();

	static void          setFocusObject(Object const *focusObject);
	static Object const *getFocusObject();
};

// ======================================================================

#endif
