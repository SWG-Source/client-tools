// ======================================================================
//
// main.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"

#include "MainFrame.h"
#include "SplashScreen.h"

#include <qlabel.h>

// ======================================================================

int main(int argc, char ** argv)
{
	GodClientApplication application(argc, argv);

	//show a splash screen until we get the main window ready
	const QRect screen = QApplication::desktop()->screenGeometry();
	SplashScreen* splashScreen = new SplashScreen(0, "splashscreen");
	splashScreen->move(screen.center() - QPoint(splashScreen->width() / 2, splashScreen->height() / 2));
	splashScreen->raise();
	splashScreen->show();
	QApplication::flush();
 
	//get the main window ready
	MainFrame mainFrame (0, "SWG GOD CLIENT");
	application.setMainWidget(&mainFrame);
	mainFrame.setCaption ("SWG God Client");
	mainFrame.resize (screen.width () - 64, screen.height () - 64);
	mainFrame.move (0,0);
	mainFrame.showMaximized();

	//the app is ready, chuck the splashscreen
	splashScreen->hide ();
	delete splashScreen;

	//run the app
	return application.exec();
}

// ======================================================================
