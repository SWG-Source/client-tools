// ======================================================================
//
// GameAnimationMessageCallback.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_GameAnimationMessageCallback_H
#define INCLUDED_GameAnimationMessageCallback_H

// ======================================================================

class CrcLowerString;
class Object;
class TransformAnimationController;

// ======================================================================

class GameAnimationMessageCallback
{
public:

	static void install();

	static void addCallback(Object &object);
	static void animationMessageToClientEventCallback(void *context, const CrcLowerString &animationMessageName, const TransformAnimationController *controller);

};

// ======================================================================

#endif
