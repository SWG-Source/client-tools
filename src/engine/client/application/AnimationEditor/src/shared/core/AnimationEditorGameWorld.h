// ======================================================================
//
// AnimationEditorGameWorld.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AnimationEditorGameWorld_H
#define INCLUDED_AnimationEditorGameWorld_H

// ======================================================================

class AnimationEnvironment;
class Object;

// ======================================================================

class AnimationEditorGameWorld
{
public:

	static Object               *getFocusObject();
	static AnimationEnvironment &getFocusObjectAnimationEnvironment();

};

// ======================================================================

#endif
