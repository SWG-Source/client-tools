// ======================================================================
//
// GameWindow.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "GameWindow.h"
#include "GameWindow.moc"

#include "ActionHack.h"
#include "ActionsGame.h"
#include "GameWidget.h"
#include "clientGraphics/Graphics.h"
#include "QLayout.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlcdnumber.h>

// ======================================================================

GameWindow::GameWindow(QWidget *theParent, const char *theName)
: BaseGameWindow(theParent, theName),
  m_interiorCameraOn(false)
{
	m_gameWidget->setFixedSize(Graphics::getFrameBufferMaxWidth(), Graphics::getFrameBufferMaxHeight());
	m_gameWidget->resize(Graphics::getFrameBufferMaxWidth(), Graphics::getFrameBufferMaxHeight());

	QLayout * const lay = layout();

	if(lay)
	{
		resize(lay->minimumSize());
	}

	ActionsGame & ga = ActionsGame::getInstance();

	onGameExclusiveFocusChanged(true);

	IGNORE_RETURN(connect(m_gameWidget, SIGNAL(gameExclusiveFocusChanged   (bool)),  this,                 SLOT(onGameExclusiveFocusChanged(bool))));
	IGNORE_RETURN(connect(m_gameWidget, SIGNAL(interiorCameraFocusChanged  (bool)),  this,                 SLOT(onInteriorCameraFocusChanged(bool))));
	IGNORE_RETURN(connect(m_gameWidget, SIGNAL(pivotDistanceChanged        (float)), this,                 SLOT(onPivotDistanceChanged     (float))));
	IGNORE_RETURN(connect(m_gameButton, SIGNAL(toggled                     (bool)),  ga.gameFocusAllowed,  SLOT(doToggle (bool))));
	IGNORE_RETURN(connect(m_gameWidget, SIGNAL(framesPerSecond             (int)),   m_fpsLCD,             SLOT(display                    (int))));
	IGNORE_RETURN(connect(ga.gameFocusAllowed, SIGNAL(toggled              (bool)),  this,                 SLOT(onGameFocusAllowedChanged(bool))));
	IGNORE_RETURN(connect(m_gameWidget, SIGNAL(cursorWorldPositionChanged  (float, float, float)),         SLOT(onCursorWorldPositionChanged(float, float, float))));
	IGNORE_RETURN(connect(m_gameWidget, SIGNAL(buildoutRegionChanged       (const char *, const char *)),  SLOT(onBuildoutRegionChanged(const char *, const char *))));
}

//-----------------------------------------------------------------

GameWindow::~GameWindow()
{
}

//-----------------------------------------------------------------

void GameWindow::onGameExclusiveFocusChanged(bool b)
{
	std::string msg;

	if(!b)
		msg = "Game Focus";
	else
		msg = "God Client Focus";

	m_focusLabel->setText(msg.c_str());
	setCaption((std::string("[") + msg + "] Game Window").c_str());
}

//-----------------------------------------------------------------

void GameWindow::onPivotDistanceChanged(float distance)
{
	char buf [128];
	sprintf(buf, "Distance: %4.2f", distance);
	m_distanceLabel->setText(buf);
}

//-----------------------------------------------------------------

void GameWindow::onGameFocusAllowedChanged(bool b)
{
	//leave button down if we're in the interior camera "mini-mode"
	if(m_interiorCameraOn == true)
		return;
	if((b && m_gameButton->state() == QButton::Off) ||
		(!b && m_gameButton->state() == QButton::On))
		m_gameButton->toggle();
}

//-----------------------------------------------------------------

void GameWindow::onInteriorCameraFocusChanged (bool b)
{
	std::string msg;

	if(!b)
	{
		msg = "Game Focus";
		m_interiorCameraOn = false;
	}
	else
	{
		msg = "Interior Camera";
		m_interiorCameraOn = true;
	}
	m_focusLabel->setText(msg.c_str());
	setCaption((std::string("[") + msg + "] Game Window").c_str());
}

//-----------------------------------------------------------------

void GameWindow::onCursorWorldPositionChanged(float positionX, float positionY, float positionZ)
{
	char buf [128];
	sprintf(buf, "Position:(%4.2f, %4.2f, %4.2f)", positionX, positionY, positionZ);
	m_positionLabel->setText(buf);
}

//-----------------------------------------------------------------

void GameWindow::onBuildoutRegionChanged(const char * planet, const char * buildout)
{
	UNREF(planet);
	char buf [128];
	sprintf(buf, "Buildout Region: %s", buildout);
	m_buildoutRegionLabel->setText(buf);
}

// ======================================================================
