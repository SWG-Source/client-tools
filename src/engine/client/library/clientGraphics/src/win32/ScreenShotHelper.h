// ======================================================================
//
// ScreenShotHelper.h
// Copyright 2003, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ScreenShotHelper_H
#define INCLUDED_ScreenShotHelper_H

// ======================================================================

class ScreenShotHelper
{
public:

	static void install();
	static bool screenShot();

private:
	ScreenShotHelper();
	ScreenShotHelper(ScreenShotHelper const &);
	ScreenShotHelper & operator =(ScreenShotHelper const &);
};

// ======================================================================

#endif
