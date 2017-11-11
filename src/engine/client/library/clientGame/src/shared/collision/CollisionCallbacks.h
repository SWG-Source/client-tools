// ======================================================================
//
// CollisionCallbacks.h
// Copyright 2004, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CollisionCallbacks_H
#define INCLUDED_CollisionCallbacks_H

// ======================================================================

class Object;


class CollisionCallbacks
{
public:
	static void install();
	static void setIgnoreCollision(bool const value);
private:
	CollisionCallbacks();
	CollisionCallbacks(CollisionCallbacks const & copy);
	CollisionCallbacks & operator = (CollisionCallbacks const & copy);
};

// ======================================================================

#endif

