// ======================================================================
//
// SetupClientAnimation.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_SetupClientAnimation_H
#define INCLUDED_SetupClientAnimation_H

// ======================================================================

class SetupClientAnimation
{
public:

	static void install();

private:
	// disabled
	SetupClientAnimation();
	SetupClientAnimation(const SetupClientAnimation&);
	SetupClientAnimation &operator =(const SetupClientAnimation&);
};

// ======================================================================

#endif
