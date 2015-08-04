// ======================================================================
//
// GamePlaybackScript.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_GamePlaybackScript_H
#define INCLUDED_GamePlaybackScript_H

// ======================================================================

class CrcString;
class Object;

// ======================================================================

class GamePlaybackScript
{
public:

	static void install();

	static void sitCreatureOnChair(Object &creatureObject, Object &chairObject, int positionIndex);
	static void standFromChair(Object &creatureObject);

	static void sitOnSaddle(Object &riderObject, Object &mountObject, CrcString const &riderHardpointName);
	static void standFromSaddle(Object &riderObject);

private:

	static void remove();

};

// ======================================================================

#endif
