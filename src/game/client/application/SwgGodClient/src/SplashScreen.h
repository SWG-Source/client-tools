// ======================================================================
//
// SplashScreen.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SplashScreen_H
#define INCLUDED_SplashScreen_H

// ======================================================================

#include "BaseSplashScreen.h"

// ======================================================================

class SplashScreen : public BaseSplashScreen
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	explicit SplashScreen(QWidget *theParent=0, const char *theName=0);

private:
	//disabled
	SplashScreen (const SplashScreen & rhs);
	SplashScreen & operator= (const SplashScreen & rhs);
};

// ======================================================================

#endif
