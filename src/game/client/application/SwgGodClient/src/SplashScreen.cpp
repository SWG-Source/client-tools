// ======================================================================
//
// SplashScreen.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "SplashScreen.h"
#include "SplashScreen.moc"

#include "IconLoader.h"

#include <qlabel.h>
#include <qpixmap.h>

#include <cstdlib>
#include <ctime>

// ======================================================================

/*
  add in the flags needed make the window frameless and always on top
*/
SplashScreen::SplashScreen(QWidget *theParent, const char *theName)
: BaseSplashScreen (theParent, theName, false, Qt::WDestructiveClose | Qt::WStyle_Customize | Qt::WStyle_NoBorder | Qt::WX11BypassWM)
{

	//we want to randomly show a different splash screen
	srand(static_cast<unsigned int>(time(0)));
	int randomValue = static_cast<int>(rand() % 100);
	//every 1 in 100 time, do the cool thing
	if(randomValue == 42)
	{
		//change the background to use the spaceballs pixmap
		const QPixmap spaceBallsPixmap = IconLoader::fetchPixmap("sb_splash");
		m_splashPixmap->setPaletteBackgroundPixmap(spaceBallsPixmap);
	}
}

// ======================================================================

