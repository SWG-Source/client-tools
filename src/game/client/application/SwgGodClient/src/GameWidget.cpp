// ======================================================================
//
// GameWidget.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

//precompiled header includes
#include "SwgGodClient/FirstSwgGodClient.h"

//module includes
#include "GameWidget.h"
#include "GameWidget.moc"

//engine shared includes
#include "sharedCollision/CollisionWorld.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/ApplicationVersion.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedGame/SetupSharedGame.h"
#include "sharedGame/SharedBuildoutAreaManager.h"
#include "sharedImage/SetupSharedImage.h"
#include "sharedInputMap/InputMap.h"
#include "sharedIoWin/IoWinManager.h"
#include "sharedIoWin/SetupSharedIoWin.h"
#include "sharedMath/SetupSharedMath.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/SetupSharedObject.h"
#include "sharedPathfinding/SetupSharedPathfinding.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedTerrain/SetupSharedTerrain.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedUtility/SetupSharedUtility.h"
#include "../../../../../../engine/shared/library/sharedXml/src/shared/core/SetupSharedXml.h"

//engine client includes
#include "clientAnimation/SetupClientAnimation.h"
#include "clientAudio/SetupClientAudio.h"
#include "../../../../../../engine/client/library/clientBugReporting/src/win32/SetupClientBugReporting.h"
#include "../../../../../../engine/client/library/clientBugReporting/src/win32/ToolBugReporting.h"
#include "clientDirectInput/DirectInput.h"
#include "clientDirectInput/SetupClientDirectInput.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/FreeCamera.h"
#include "clientGame/FreeChaseCamera.h"
#include "clientGame/FormManagerClient.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/SetupClientGame.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/ScreenShotHelper.h"
#include "clientGraphics/SetupClientGraphics.h"
#include "clientObject/SetupClientObject.h"
#include "clientParticle/SetupClientParticle.h"
#include "clientSkeletalAnimation/SetupClientSkeletalAnimation.h"
#include "clientTextureRenderer/SetupClientTextureRenderer.h"
#include "clientTerrain/SetupClientTerrain.h"

//Cui includes
#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiPreferences.h"
#include "swgClientUserInterface/SwgCuiHud.h"
#include "swgClientUserInterface/SwgCuiHudFactory.h"
#include "swgClientUserInterface/SwgCuiManager.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgSharedNetworkMessages/SetupSwgSharedNetworkMessages.h"

//GodClient local includes
#include "ActionHack.h"
#include "ActionsEdit.h"
#include "ActionsGame.h"
#include "ActionsScript.h"
#include "ActionsView.h"
#include "ActionsWindow.h"
#include "BookmarkData.h"
#include "BrushData.h"
#include "BuildoutAreaSupport.h"
#include "BaseGotoDialog.h"
#include "ConfigGodClient.h"
#include "FavoritesListView.h"
#include "FavoritesWindow.h"
#include "FilesystemTree.h"
#include "FormWindow.h"
#include "GodClientData.h"
#include "MainFrame.h"
#include "ObjectTemplateData.h"
#include "RecentDirectory.h"
#include "ServerCommander.h"

//system includes
#include <ctime>
#include <dinput.h>

//QT includes
#include <qaction.h>
#include <qdatetime.h>
#include <qdragobject.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qtimer.h>

// ======================================================================

namespace GameWidgetNamespace
{
	//-- @todo: preferences manager should handle key mapping
	const int s_translateKey       = static_cast<int>(Qt::Key_T);
	const int s_groundTranslateKey = static_cast<int>(Qt::Key_G);
	const int s_scaleKey           = static_cast<int>(Qt::Key_S);
	const int s_rotateKey          = static_cast<int>(Qt::Key_R);
	const int s_avatarKey          = static_cast<int>(Qt::Key_A);
	const int s_shiftKey           = static_cast<int>(Qt::Key_Shift);
	const int s_gotoKey            = static_cast<int>(Qt::Key_Equal);

	const int s_gameLoopTimeOut    = 5;

	bool s_mustApplyTransforms = true;

	template <typename T> inline bool hasState(const T* event, Qt::ButtonState state)
	{
		return(static_cast<size_t>(event->state()) & static_cast<size_t>(state)) != 0;
	}
	template <typename T> inline bool hasStateAfter(const T* event, Qt::ButtonState state)
	{
		return(static_cast<size_t>(event->stateAfter()) & static_cast<size_t>(state)) != 0;
	}

	inline bool isMiddleButtonMove(const QMouseEvent* mouseEvent)
	{
		return(hasState(mouseEvent, QWidget::MidButton) ||
			(hasState(mouseEvent, QWidget::LeftButton) && hasState(mouseEvent, QWidget::RightButton)));
	}

	inline bool isMiddleButtonActive(const QMouseEvent* mouseEvent)
	{
		return(hasStateAfter(mouseEvent, QWidget::MidButton) ||
			(hasStateAfter(mouseEvent, QWidget::LeftButton) && hasStateAfter(mouseEvent, QWidget::RightButton)));
	}

	std::string s_lastBuildoutFileOpened;
	bool        s_askQuestionAboutBuildoutFiles = true;
	bool        s_openBuildoutFiles = false;
}

using namespace GameWidgetNamespace;

GameWidget *GameWidget::ms_globalGameWidget;

//-----------------------------------------------------------------

struct GameWidget::KeyStates
{
	typedef std::map<int,bool>     KeyContainer;
	KeyContainer container;

	inline bool isValid(int i) const
	{
		return container.find(i) != container.end();
	}

	inline bool state(int i) const
	{
		const KeyContainer::const_iterator it = container.find(i);
		if(it != container.end())
			return(*it).second;
		return false;
	}

	inline bool setIfValid(int i, bool b)
	{
		const KeyContainer::iterator it = container.find(i);
		if(it != container.end())
		{
			(*it).second = b;
			return true;
		}
		return false;
	}

	void reset()
	{
		container [s_translateKey]       = false;
		container [s_groundTranslateKey] = false;
		container [s_scaleKey]           = false;
		container [s_rotateKey]          = false;
		container [s_shiftKey]           = false;
	}

	//-- not 'any' state, really.  hasAnyState should be hasAnyStateThatOverridesContextMenusAndSelectionEvents
	inline bool hasAnyState() const
	{
		return state(s_groundTranslateKey) ||
			state(s_translateKey) ||
			state(s_rotateKey) ||
			state(s_scaleKey);
	}

	KeyStates() : container()
	{
		reset();
	}
};

//-----------------------------------------------------------------

GameWidget::GameWidget(QWidget* theParent, const char*theName)
: QWidget(theParent, theName), //lint !e578 //hides
  Receiver(),
  m_frameCounter(0),
  m_defaultCursor(static_cast<int>(ArrowCursor)),
  m_blankCursor(static_cast<int>(BlankCursor)),
  m_previousSecondframeCounter(0),
  m_gameHasFocus(false),
  m_lastMousePoint(),
  m_timer(0),
  m_secondTimer(0),
  m_keyStates(0),
  m_pop(0),
  m_templateMenu(0),
  m_lastOID(10000),
  m_rubberBandRect(),
  m_rubberBanding(false),
  m_rubberBandOk(false),
  m_mouseDownPoint(),
  m_flyButtonDown(false),
  m_interiorCameraOn(false),
  m_popupData(),
  m_gs(0),
  m_discardNextMouseRelease(false),
  m_warpMouse(false),
  m_autoDraggingObjects(false),
  m_autoDragNetworkId(),
  m_minFrameLength(50), //20 fps default if all else fails
  m_lastFrameTimestamp(),
  m_updateObjects(),
  m_callback (new MessageDispatch::Callback)
{
	ms_globalGameWidget = this;

	Zero(m_popupData);

	QWidget::setBackgroundMode(Qt::NoBackground);
	QWidget::setMouseTracking(true);

	// find the top level window
	HWND topLevelWindow = static_cast<HWND>(winId());
	HWND parentWindow;
	do
	{
		parentWindow = GetParent(topLevelWindow);
		if(parentWindow)
			topLevelWindow = parentWindow;
	} while(parentWindow);

	//-- thread
	SetupSharedThread::install();

	//-- debug
	SetupSharedDebug::install(4096);
	InstallTimer rootInstallTimer("root");

	//-- setup
	{
		//-- foundation
		SetupSharedFoundation::Data sharedFoundationData(SetupSharedFoundation::Data::D_mfc);
		sharedFoundationData.useWindowHandle    = true;
		sharedFoundationData.processMessagePump = true;
		sharedFoundationData.windowHandle       = topLevelWindow;
		sharedFoundationData.configFile         = "SwgGodClient.cfg";
		sharedFoundationData.clockUsesSleep     = true;
        sharedFoundationData.writeMiniDumps     = ApplicationVersion::isBootlegBuild();
	    SetupSharedFoundation::install(sharedFoundationData);

	    SetupClientBugReporting::install();

        if (ApplicationVersion::isBootlegBuild())
        {
	        ToolBugReporting::startCrashReporter();
        }

		//-- file
		SetupSharedFile::install(true);

		//-- math
		SetupSharedMath::install();

		//-- utility
		SetupSharedUtility::Data setupUtilityData;
		SetupSharedUtility::setupGameData (setupUtilityData);
		SetupSharedUtility::install (setupUtilityData);

		//-- random
		SetupSharedRandom::install(static_cast<uint32>(time(NULL)));

		//-- image
		SetupSharedImage::Data setupImageData;
		SetupSharedImage::setupDefaultData(setupImageData);
		SetupSharedImage::install(setupImageData);

		//-- network
		SetupSharedNetworkMessages::install();
		SetupSwgSharedNetworkMessages::install();

		//-- object
		SetupSharedObject::Data setupObjectData;
		SetupSharedObject::setupDefaultGameData(setupObjectData);
		// we want the SlotIdManager initialized, and we need the associated hardpoint names loaded.
		SetupSharedObject::addSlotIdManagerData(setupObjectData, true);
		// we want CustomizationData support on the client.
		SetupSharedObject::addCustomizationSupportData(setupObjectData);
		SetupSharedObject::install(setupObjectData);

		//-- game
		SetupSharedGame::Data setupSharedGameData;
		setupSharedGameData.setUseGameScheduler(true);
		SetupSharedGame::install (setupSharedGameData);

		//-- terrain
		SetupSharedTerrain::Data setupSharedTerrainData;
		SetupSharedTerrain::setupGameData(setupSharedTerrainData);
		SetupSharedTerrain::install(setupSharedTerrainData);

		//-- pathfinding
		SetupSharedPathfinding::install();

		//-- SharedXml
		SetupSharedXml::install();

		//-- audio
		SetupClientAudio::install();

		//-- graphics
		SetupClientGraphics::Data setupGraphicsData;
		SetupClientGraphics::setupDefaultGameData(setupGraphicsData);
		SetupClientGraphics::install(setupGraphicsData);
		Graphics::setTranslatePointFromGameToScreen(translatePointFromGameToScreen);

		//-- directinput
		SetupClientDirectInput::install(GetModuleHandle(NULL), Os::getWindow(), DIK_LCONTROL, Graphics::isWindowed);
		DirectInput::setScreenShotFunction(ScreenShotHelper::screenShot);
		DirectInput::suspendInput();

		//-- object
		SetupClientObject::Data setupClientObjectData;
		SetupClientObject::setupGameData (setupClientObjectData);
		SetupClientObject::install (setupClientObjectData);

		//-- animation and skeletal animation
		SetupClientAnimation::install();

		SetupClientSkeletalAnimation::Data  saData;
		SetupClientSkeletalAnimation::setupGameData(saData);
		SetupClientSkeletalAnimation::install (saData);

		//-- texture renderer
		SetupClientTextureRenderer::install();

		//-- terrain
		SetupClientTerrain::install();

		//-- particle system
		SetupClientParticle::install ();

		//-- game
		SetupClientGame::Data clientGameData;
		SetupClientGame::setupGameData(clientGameData);
		clientGameData.m_disableLazyInteriorLayoutCreation = true;
		SetupClientGame::install(clientGameData);

		//-- iowin (must be after clientgame due to order dependencies)
		SetupSharedIoWin::install();
	}

	GodClientApplication::setGameHWND(winId());

	//-- override the platform frame rate limiter with our own, if needed
	setFrameRateLimit(static_cast<int>(ConfigSharedFoundation::getFrameRateLimit()));
	Clock::noFrameRateLimit();

	ConfigGodClient::install();

	if(ConfigGodClient::getData().frameRateLimit)
		setFrameRateLimit(ConfigGodClient::getData().frameRateLimit);

	CuiManager::setImplementationInstallFunctions(SwgCuiManager::install, SwgCuiManager::remove, SwgCuiManager::update);

	// setup the game
	Game::install(Game::A_godClient);

	// turn off the mousemode being default
	CuiPreferences::setMouseModeDefault (false);

	// RecentDirectory
	RecentDirectory::install("Software\\Sony Online Entertainment\\SwgGodClient\\Recent");

	QWidget::setFixedSize(Graphics::getFrameBufferMaxWidth(), Graphics::getFrameBufferMaxHeight());
	QWidget::resize(Graphics::getFrameBufferMaxWidth(), Graphics::getFrameBufferMaxHeight());

	m_timer       = new QTimer(this, "timer");
	m_secondTimer = new QTimer(this, "secondTimer");

	// set up a timer to issue repaints to run the game loop as fast as possible
	// NOTE: this particular timer must be a zero-length timer to avoid a Qt bug
	IGNORE_RETURN(connect(m_timer, SIGNAL(timeout()), this, SLOT(runGameLoop())));
	IGNORE_RETURN(m_timer->start(s_gameLoopTimeOut, false));

	// set up a timer to update the frame rate once per second
	IGNORE_RETURN(connect(m_secondTimer, SIGNAL(timeout()), this, SLOT(secondElapsed())));
 	IGNORE_RETURN(m_secondTimer->start(1000, false));

	QWidget::setFocusPolicy(QWidget::StrongFocus);

	connectToEmitter(CuiManager::getIoWin().getEmitter(), CuiIoWin::Messages::MOUSE_HIT_EDGE);
	connectToMessage(Game::Messages::SCENE_CHANGED);

	m_pop = new QPopupMenu(this, "GameWidget_popup");
	IGNORE_RETURN(connect(m_pop, SIGNAL(activated (int)), this, SLOT(onPopupItemActivated(int))));

	m_templateMenu = new QPopupMenu(this, "GameWidget_templateMenu");
	IGNORE_RETURN(connect(m_templateMenu, SIGNAL(aboutToShow()), this, SLOT(onTemplateMenuShow())));

	m_keyStates = new KeyStates;

	updateSceneData();

	IGNORE_RETURN(connect(ActionsGame::getInstance().gameFocusAllowed, SIGNAL(toggled(bool)), this, SLOT(onGameFocusAllowedChanged(bool))));
//	IGNORE_RETURN(connect(ActionsGame::getInstance().interiorCameraAllowed, SIGNAL(toggled(bool)), this, SLOT(onInteriorCameraChanged(bool))));
	IGNORE_RETURN(connect(ActionsEdit::getInstance().createObjectFromSelectedTemplate, SIGNAL(activated()), this, SLOT(createObjectFromSelectedTemplate())));
	IGNORE_RETURN(connect(ActionsView::getInstance().addObjectBookmark, SIGNAL(activated()), this, SLOT(onAddObjectBookmark())));

	m_callback->connect (*this, &GameWidget::onEditFormData, static_cast<FormManagerClient::Messages::EditFormData *>(0));

	Graphics::setHardwareMouseCursorEnabled(false);
}

// ----------------------------------------------------------------------

GameWidget::~GameWidget()
{
	ms_globalGameWidget = NULL;

	delete m_timer;
	m_timer = NULL;
	delete m_secondTimer;
	m_secondTimer = NULL;

	disconnectFromMessage(CuiIoWin::Messages::MOUSE_HIT_EDGE);
	disconnectFromMessage(Game::Messages::SCENE_CHANGED);

	delete m_keyStates;
	m_keyStates = 0;

	m_callback->disconnect (*this, &GameWidget::onEditFormData, static_cast<FormManagerClient::Messages::EditFormData *>(0));
	delete m_callback;
	m_callback = 0;

	RecentDirectory::remove();
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	m_templateMenu = NULL; //lint !e423 creation of memory leak, it's owned by Qt, gets deleted eventually
	m_gs = NULL;
	m_autoDragNetworkId = NULL;
	m_pop = NULL;          //lint !e423 creation of memory leak, it's owned by Qt, gets deleted eventually
}

// ----------------------------------------------------------------------

void GameWidget::runGameLoop()
{
	static bool processingThisFunction = false;

	int actualMinFrameLength = m_minFrameLength;
	//-- if invisible, run at 1 fps, if simply inactive(may still be visible), run at half speed
	if(!isVisible())
		actualMinFrameLength = 1000;
	else if(!MainFrame::getInstance().isActive())
		actualMinFrameLength *= 2;

	const QTime curTime = QTime::currentTime();

	int const elapsedTime = m_lastFrameTimestamp.msecsTo(curTime);
	if(elapsedTime < actualMinFrameLength)
		return;

	m_lastFrameTimestamp = curTime;

	if (!processingThisFunction)
	{
		processingThisFunction = true;
	}
	else
	{
		return;
	}

	//-- for debugging - jww
	const bool pop_enabled = m_pop ? m_pop->isEnabled() : false;
	const bool pop_visible = m_pop ? m_pop->isVisible() : false;

	UNREF(pop_enabled);
	UNREF(pop_visible);

	if(m_gs && m_flyButtonDown && !m_gameHasFocus)
	{
		NOT_NULL(m_keyStates);

		FreeCamera* const cam  = NON_NULL(m_gs->getGodClientCamera());
		cam->setMode(FreeCamera::M_fly);

		InputMap* const imap   = NON_NULL(m_gs->getGodClientInputMap());
		MessageQueue* const q  = NON_NULL(imap->getMessageQueue());

		if(m_keyStates->state(s_shiftKey))
			q->appendMessage(static_cast<int>(CM_toggleRunOn), CONST_REAL(0));

		q->appendMessage(static_cast<int>(CM_up), CONST_REAL(0));
	}

	Game::runGameLoopOnce(true, static_cast<HWND>(winId()), width(), height());

	++m_frameCounter;

	//-- @todo: this drawing should be moved into the GodClientIoWin to prevent flicker

	if(m_rubberBanding)
	{
		QPainter painter(this);
		painter.setPen(Qt::SolidLine);
		painter.setPen(QColor(100, 255, 155));
		painter.drawRect(m_rubberBandRect);
	}

	processingThisFunction = false;

	IGNORE_RETURN(m_timer->start(s_gameLoopTimeOut, false));
}

// ----------------------------------------------------------------------

void GameWidget::keyPressEvent(QKeyEvent*keyEvent)
{
	keyEvent->accept();

	//cancal all rubber banding, selections, and ghosts on ESC
	if(keyEvent->key() == static_cast<int>(Qt::Key_Escape))
	{
		NOT_NULL(m_pop);

		if(m_pop->isVisible())
		{
			IGNORE_RETURN(m_pop->close());
		}

		if(m_rubberBanding)
		{
			m_rubberBanding = false;
			m_rubberBandOk = false;
		}
		else
		{
			if(GodClientData::getInstance().killGhosts() == 0)
				GodClientData::getInstance().clearSelection();
		}
	}

	if(!m_gameHasFocus)
	{
		if (keyEvent->key() == s_avatarKey && Game::getSinglePlayer())
		{
			Vector position;
			IGNORE_RETURN(getCursorPositionIntoWorld(position));

			GroundScene *groundScene = dynamic_cast<GroundScene *>(Game::getScene());
			Object* const player = groundScene ? groundScene->getPlayer() : 0;

			if (player)
			{
				if (player->getAttachedTo() != 0)
					player->setParentCell(CellProperty::getWorldCellProperty());

				CellProperty::setPortalTransitionsEnabled(false);
					player->setPosition_p(position);
				CellProperty::setPortalTransitionsEnabled(true);

				CollisionWorld::objectWarped(player);
			}
		}

		if (keyEvent->key() == s_gotoKey)
		{
			NOT_NULL(m_gs);
			FreeCamera* const camera = NON_NULL(m_gs->getGodClientCamera());

			Vector pivotPoint = camera->getPivotPoint();
			real pivotDistance = camera->getPivotDistance();

			BaseGotoDialog gotoDlg;
			gotoDlg.m_x->setText(QString().number(pivotPoint.x));
			gotoDlg.m_y->setText(QString().number(pivotPoint.y));
			gotoDlg.m_z->setText(QString().number(pivotPoint.z));
			gotoDlg.m_distance->setText(QString().number(pivotDistance));
			
			int result = gotoDlg.exec();

			if (result == QDialog::Accepted)
			{
				Vector v(gotoDlg.m_x->text().toFloat(), gotoDlg.m_y->text().toFloat(), gotoDlg.m_z->text().toFloat());

				camera->setPivotPoint(v);
				camera->setPivotDistance(gotoDlg.m_distance->text().toFloat());
			}
		}

		NOT_NULL(m_keyStates);
		if(m_keyStates->setIfValid(keyEvent->key(), true))
			return;

		if(hasState(keyEvent, Qt::ControlButton))
		{
			if(keyEvent->key() == static_cast<int>(Qt::Key_Control) || keyEvent->key() == static_cast<int>(Qt::Key_Alt))
				return;
		}
		else if(hasState(keyEvent, Qt::AltButton))
		{
			if(keyEvent->key() == static_cast<int>(Qt::Key_Control) || keyEvent->key() == static_cast<int>(Qt::Key_Alt))
				return;
		}
		return;
	}
	else if(keyEvent->key() == static_cast<int>(Key_Enter) ||
		(keyEvent->key() == static_cast<int>(Key_F8) && hasState(keyEvent, Qt::ControlButton)))
	{
		ActionsGame::getInstance().gameFocusAllowed->doActivate();
		return;
	}

	int k = keyEvent->key();
	int k2 = static_cast<int>(Key_F11);
	UNREF(k);
	UNREF(k2);
/*
	if(keyEvent->key() == static_cast<int>(Key_F11) && hasState(keyEvent, Qt::ControlButton))
	{
 		ActionsGame::getInstance().interiorCameraAllowed->doToggle(!m_interiorCameraOn);
		return;
	}
*/
//	else
//	{
		// send key press messages
		QString text = keyEvent->text();
		for(uint i = 0; i < text.length(); ++i)
			IoWinManager::queueCharacter(0, text.at(i).latin1());
//	}
}

// ----------------------------------------------------------------------

void GameWidget::keyReleaseEvent(QKeyEvent*keyEvent)
{
	if(!m_gameHasFocus)
	{
		keyEvent->accept();

		if(keyEvent->isAutoRepeat())
			return;

		NOT_NULL(m_keyStates);
		if(m_keyStates->setIfValid(keyEvent->key(), false))
			return;
	}
}

// ----------------------------------------------------------------------

void GameWidget::translatePointFromGameToScreen(int &x, int &y)
{
	if (ms_globalGameWidget)
	{
		QPoint p(x, y);
		QPoint q = ms_globalGameWidget->mapToGlobal(p);
		x = q.x();
		y = q.y();
	}
}

// ----------------------------------------------------------------------

void GameWidget::mouseMoveEvent(QMouseEvent*mouseEvent)
{
	mouseEvent->accept();
	if(m_gameHasFocus)
	{
		const int x = mouseEvent->x();
		const int y = mouseEvent->y();
		IoWinManager::queueSetSystemMouseCursorPosition(x, y);
		return;
	}

	if(ActionsGame::getInstance().gameFocusAllowed->isOn() && !m_interiorCameraOn)
	{
		CuiManager::getIoWin().warpCursor(mouseEvent->x(), mouseEvent->y());
		grabExclusiveFocus();
		mouseEvent->accept();
		return;
	}

	m_rubberBanding = false;

	if(!m_gs)
		return;

	const QPoint pt(mouseEvent->pos());
	GodClientData::getInstance().cursorScreenPositionChanged(pt.x(), pt.y());

	const real dx =(mouseEvent->x() - m_lastMousePoint.x())* CONST_REAL(0.01);
	const real dy =(mouseEvent->y() - m_lastMousePoint.y())* CONST_REAL(0.01);

	NOT_NULL(m_keyStates);

	m_gs->activateGodClientCamera();
	FreeCamera* const cam  = NON_NULL(m_gs->getGodClientCamera());

	//-- use maya movement controls
	if(hasState(mouseEvent, QWidget::AltButton))
	{
		InputMap* const imap   = NON_NULL(m_gs->getGodClientInputMap());
		MessageQueue* const q  = NON_NULL(imap->getMessageQueue());

		//-- zoom the camera(middle button or chorded middle), "Track"
		if(isMiddleButtonMove(mouseEvent))
		{
			//-- fly the camera
			if(hasState(mouseEvent, QWidget::ControlButton))
			{
				cam->setMode(FreeCamera::M_fly);
				q->appendMessage(static_cast<int>(CM_cameraYaw),   dx);
				q->appendMessage(static_cast<int>(CM_cameraPitch), dy);
			}

			//-- zoom the pivot
			else
			{
				q->appendMessage(static_cast<int>(CM_cameraPivotZoom), -dx);
			}
		}

		//-- camera turning, "Tumble"
		else if(hasState(mouseEvent, QWidget::LeftButton))
		{
			if(hasState(mouseEvent, QWidget::ControlButton))
			{
				cam->setMode(FreeCamera::M_fly);
				q->appendMessage(static_cast<int>(CM_cameraYaw),   dx);
				q->appendMessage(static_cast<int>(CM_cameraPitch), dy);
			}
			else
			{
				q->appendMessage(static_cast<int>(CM_cameraPivotYaw),   dx);
				q->appendMessage(static_cast<int>(CM_cameraPivotPitch), dy);
			}
		}

		//-- translate the camera
		else if(hasState(mouseEvent, QWidget::RightButton))
		{
			const TerrainObject* const terrain = TerrainObject::getConstInstance ();

			//-- translate pivot ALONG GROUND
			if(hasState(mouseEvent, QWidget::ControlButton))
			{
				cam->setMode(FreeCamera::M_pivot);
				FreeCamera::Info info(cam->getInfo());

				real oldHeight = CONST_REAL(10);
				const bool terrain_ok = terrain->getHeight(info.translate, oldHeight);
				if(terrain_ok)
					oldHeight = info.translate.y - oldHeight;

				const real mult = std::max(CONST_REAL(10), info.distance);

				const real my_dx = mult* dx;
				const real my_dy = -mult* dy;

				info.translate += cam->rotate_o2w(Vector(my_dx, CONST_REAL(0), CONST_REAL(0)));

				Vector forward(cam->getObjectFrameK_w());
				forward.y = CONST_REAL(0);
				IGNORE_RETURN(forward.normalize());
				info.translate += forward* my_dy;

				if(terrain_ok)
				{
					real newHeight;
					if(terrain->getHeight(info.translate, newHeight))
					{
						const real newDiff = info.translate.y - newHeight;
						if(newDiff < CONST_REAL(0))
							info.translate.y += oldHeight - newDiff;
					}
				}
				cam->setInfo(info);
				cam->setInterpolating(false);
			}

			//-- translate camera parallel to VIEW PLANE, "Dolly"
			else
			{
				if(dx)
					q->appendMessage(static_cast<int>(CM_cameraPivotTranslateX), -dx);
				if(dy)
					q->appendMessage(static_cast<int>(CM_cameraPivotTranslateY), dy);
			}
		}

		emit pivotDistanceChanged(m_gs->getGodClientCamera()->getPivotDistance());
	}

	else if(m_autoDraggingObjects && !m_keyStates->hasAnyState())
	{
		const QPoint pt(mouseEvent->pos());

		const CellProperty* cellProperty = CellProperty::getWorldCellProperty ();
		Vector result;
		if(GodClientData::getInstance().findIntersection_p(pt.x(), pt.y(), cellProperty, result))
		{
			IGNORE_RETURN(GodClientData::getInstance().moveGhostsFollowingObject(*m_autoDragNetworkId, result, true));
		}
	}

	else if(m_keyStates->state(s_translateKey))
	{
		if(isMiddleButtonMove(mouseEvent))
		{
			pullSelection(dx, false);
		}
		else if(hasState(mouseEvent, Qt::RightButton))
		{
			GodClientData::getInstance().translateSelectionY(-dy);
		}
		else if(hasState(mouseEvent, Qt::LeftButton))
		{
			GodClientData::getInstance().translateSelection(dx, dy, false);
		}
	}

	else if(m_keyStates->state(s_groundTranslateKey))
	{
		if(isMiddleButtonMove(mouseEvent))
		{
			pullSelection(dx, true);
		}
		else if(hasState(mouseEvent, Qt::LeftButton))
		{
			GodClientData::getInstance().translateSelection(dx, dy, true);
		}
		else if(hasState(mouseEvent, Qt::RightButton))
		{
			GodClientData::getInstance().translateSelectionY(-dy);
		}

		if(hasState(mouseEvent, Qt::ControlButton))
		{
			GodClientData::getInstance().alignGhostsToTerrain();
		}
	}

	else if(m_keyStates->state(s_rotateKey))
	{
		GodClientData::RotationPivotType pivotType = static_cast<GodClientData::RotationPivotType>(ActionsEdit::getInstance().getRotatePivotMode());

		GodClientData::RotationType type = GodClientData::Rotate_none;
		real delta = dx;

		if(isMiddleButtonMove(mouseEvent))
		{
			type = GodClientData::Rotate_pitch;
		}
		else if(hasState(mouseEvent, Qt::LeftButton))
		{
			type = GodClientData::Rotate_yaw;
		}
		else if(hasState(mouseEvent, Qt::RightButton))
		{
			type = GodClientData::Rotate_roll;
			delta = -delta;
		}

		//-- do it
		if(type != GodClientData::Rotate_none)
		{
			GodClientData::getInstance().rotateGhosts(delta, type, pivotType);
		}
	}

	else if(m_keyStates->state(s_scaleKey))
	{
		if(hasState(mouseEvent, Qt::LeftButton))
		{
			GodClientData::getInstance().scaleSelectionY(-dy);
		}
		else if(hasState(mouseEvent, Qt::RightButton))
		{
			GodClientData::getInstance().scaleSelection(dx, dy);
		}
	}

	//-- rubber banding
	else if(hasState(mouseEvent, Qt::LeftButton))
	{
		if(m_rubberBandOk)
		{
			m_rubberBanding = true;

			const QPoint mpos = mouseEvent->pos();

			m_rubberBandRect.setLeft (std::min(mpos.x(), m_mouseDownPoint.x()));
			m_rubberBandRect.setRight(std::max(mpos.x(), m_mouseDownPoint.x()));
			m_rubberBandRect.setTop  (std::min(mpos.y(), m_mouseDownPoint.y()));
			m_rubberBandRect.setBottom(std::max(mpos.y(), m_mouseDownPoint.y()));
		}
	}

	if(m_warpMouse)
	{
		m_lastMousePoint = rect().center();
		QCursor::setPos(mapToGlobal(m_lastMousePoint));
	}
	else
	{
		m_lastMousePoint = mouseEvent->pos();
	}

	if(!m_rubberBanding)
		m_rubberBandOk = false;

	//update the cursor position widget
	const CellProperty* cellProperty = CellProperty::getWorldCellProperty ();
	Vector result;
	bool collidedWithGround = GodClientData::getInstance().findIntersection_p(pt.x(), pt.y(), cellProperty, result);
	if(!collidedWithGround)
	{
		result.x = 0.0;
		result.y = 0.0;
		result.z = 0.0;
	}

	GodClientData::getInstance().setMouseCursorIntersection(result);

	emit cursorWorldPositionChanged(result.x, result.y, result.z);
}

//-----------------------------------------------------------------

void GameWidget::pullSelection(real dx, bool alongGround)
{
	if(m_gs == 0)
		return;

	FreeCamera* const cam = NON_NULL(m_gs->getGodClientCamera());
	Vector ghostCenter;

	if(GodClientData::getInstance().calculateSelectionCenter(ghostCenter, true))
	{
		//-- don't let us pull the selection behind us
		const real pull_dx = std::max(CONST_REAL(-1), dx);
		const Vector diff =(ghostCenter - cam->getPosition_p());

		Vector moveDiff = diff* pull_dx;

		if((diff + moveDiff).magnitudeSquared() > CONST_REAL(1.0))
		{
			GodClientData::getInstance().translateGhosts(moveDiff, alongGround);
		}
	}
}

//-----------------------------------------------------------------

void GameWidget::wheelEvent(QWheelEvent* evt)
{
	NOT_NULL(m_pop);

	//-- debugging
	const bool vis = m_pop->isVisible();
	const bool ena = m_pop->isEnabled();

	UNREF(vis);
	UNREF(ena);

	if(!m_gameHasFocus)
	{
		if(m_gs)
		{
			if(hasState(evt, Qt::AltButton) && hasState(evt, Qt::ControlButton))
			{
				FreeCamera* const cam  = NON_NULL(m_gs->getGodClientCamera());
				cam->setMode(FreeCamera::M_fly);
				FreeCamera::Info info(cam->getInfo());
				info.translate += cam->getObjectFrameK_w()*(static_cast<real>(evt->delta())* CONST_REAL(0.05));
				cam->setInfo(info);
				cam->setInterpolating(false);
			}
			else
				m_gs->pivotZoom(-evt->delta()* CONST_REAL(0.005));

			emit pivotDistanceChanged(m_gs->getGodClientCamera()->getPivotDistance());
		}
	}

	evt->accept();
}

// ----------------------------------------------------------------------

void GameWidget::mousePressEvent(QMouseEvent*mouseEvent)
{
	mouseEvent->accept();
	if(isMiddleButtonActive(mouseEvent) &&
		(mouseEvent->state() &(Qt::AltButton | Qt::ControlButton)) ==(Qt::AltButton | Qt::ControlButton))
		m_flyButtonDown = true;

	if(!m_gameHasFocus)
	{
		if(CuiManager::getPointerInputActive())
		{
			ActionsGame::getInstance().gameFocusAllowed->doToggle(true);
			CuiManager::getIoWin().warpCursor(mouseEvent->x(), mouseEvent->y());
			return;
		}

		m_mouseDownPoint = m_lastMousePoint = mouseEvent->pos();

		//-- rubberbanding can only performed with the left mouse button, and
		//-- only with no modifiers or either/both shift/control
		NOT_NULL(m_keyStates);

		if(mouseEvent->button() == Qt::LeftButton && !m_keyStates->hasAnyState())
		{
			if(!m_gs)
				return;

			if(static_cast<int>(mouseEvent->state()) == 0)
			{
				const Object* const obj = m_gs->findObject(mouseEvent->x(), mouseEvent->y());
				if(obj)
				{
					bool isSelection = false;
					bool isGhost     = false;

					bool b = GodClientData::getInstance().getIsSelectionOrGhost(*obj, isSelection, isGhost);

					if(b)
					{
						if(isSelection)
							IGNORE_RETURN(GodClientData::getInstance().killGhosts());

						m_autoDraggingObjects = true;
						m_autoDragNetworkId = &obj->getNetworkId();
						m_discardNextMouseRelease = true;
						return;
					}
				}
			}

			if((mouseEvent->state() & ~(Qt::LeftButton | Qt::ControlButton | Qt::ShiftButton)) == 0) //lint !e641 //damn enums
			{
				m_rubberBanding = false;
				m_rubberBandOk = true;
			}
		}
	}
}

// ----------------------------------------------------------------------

void GameWidget::mouseReleaseEvent(QMouseEvent*mouseEvent)
{
	mouseEvent->accept();

	if(!isMiddleButtonActive(mouseEvent))
		m_flyButtonDown = false;

	if(mouseEvent->button() == Qt::LeftButton)
	{
		m_autoDraggingObjects = false;
	}

	if(m_discardNextMouseRelease)
	{
		m_discardNextMouseRelease = false;
		return;
	}

	if(!m_gameHasFocus)
	{

		if(hasState(mouseEvent, Qt::AltButton))
		{
			return;
		}

		//-- handle rubber-band selection
		if(m_gs && m_rubberBanding)
		{
			m_rubberBanding = false;
			m_rubberBandOk  = false;

			ClientObject** result;

			const int num = m_gs->findObjects(static_cast<real>(m_rubberBandRect.left()),
				static_cast<real>(m_rubberBandRect.top()),
				static_cast<real>(m_rubberBandRect.right()),
				static_cast<real>(m_rubberBandRect.bottom()),
				result);

			if(num == 0)
				return;

			//-- add all objects to selection
			if(hasState(mouseEvent, Qt::ShiftButton))
			{
				for(int i = 0; i < num; ++i)
				{
					GodClientData::getInstance().addSelection(result[i]);
				}
			}

			//-- toggle all objects rubberbanded
			else if(hasState(mouseEvent, Qt::ControlButton))
			{
				for(int i = 0; i < num; ++i)
				{
					GodClientData::getInstance().toggleSelection(result[i]);
				}
			}

			//-- simply replace the selection
			else
			{
				GodClientData::getInstance().clearSelection();
				for(int i = 0; i < num; ++i)
				{
				GodClientData::getInstance().addSelection(result[i]);
				}
			}

			delete[] result;
			return;
		}

		//-- handle click-to-select
		if(m_gs)
		{

			if(!s_mustApplyTransforms)
			{
				GodClientData::getInstance().synchronizeSelectionWithGhosts();
			}

			NOT_NULL(m_keyStates);
			if(m_keyStates->hasAnyState())
				return;

			if(mouseEvent->button() == QMouseEvent::LeftButton)
			{
				if(hasState(mouseEvent, Qt::ControlButton))
				{
					ClientObject* const obj = dynamic_cast<ClientObject*>(m_gs->findObject(mouseEvent->x(), mouseEvent->y()));
					if(obj)
						GodClientData::getInstance().toggleSelection(obj);
				}
				else if(hasState(mouseEvent, QWidget::ShiftButton))
				{
					ClientObject* const obj = dynamic_cast<ClientObject*>(m_gs->findObject(mouseEvent->x(), mouseEvent->y()));
					if(obj)
						GodClientData::getInstance().addSelection(obj);
				}
				else
				{
					ClientObject* const obj = dynamic_cast<ClientObject*>(m_gs->findObject(mouseEvent->x(), mouseEvent->y()));
					if(obj)
					{
						GodClientData::getInstance().setSelection(obj);
					}
				}
			}
		}
	}
}
//-----------------------------------------------------------------

void GameWidget::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{
	mouseEvent->accept();

	if(mouseEvent->button() != QMouseEvent::LeftButton)
	{
		return;
	}

	if(!m_gameHasFocus)
	{
		if(m_gs)
		{
			//don't allow click-moves inside other objects
			if(m_gs->getPlayer()->getParentCell() != CellProperty::getWorldCellProperty())
				return;

			Object* const obj = m_gs->findObject(mouseEvent->x(), mouseEvent->y());

			if(obj)
			{
				ActionsView::getInstance().centerCameraOnObject(*obj, true, static_cast<int>(ActionsView::Fit_dontCare));
				QCursor::setPos(mapToGlobal(rect().center()));
				m_rubberBandOk = false;
				m_rubberBanding = false;
			}
			else
			{
				const CellProperty* cellProperty = CellProperty::getWorldCellProperty ();
				Vector result;
				if(GodClientData::getInstance().findIntersection_p(mouseEvent->x(), mouseEvent->y(), cellProperty, result))
				{
					ActionsView::getInstance().centerCameraOnPoint(result, true, static_cast<int>(ActionsView::Fit_point));
					QCursor::setPos(mapToGlobal(rect().center()));
					m_rubberBandOk = false;
					m_rubberBanding = false;
				}
			}

			m_discardNextMouseRelease = true;
		}
	}
}

//-----------------------------------------------------------------

namespace
{
	void populateMenu(QPopupMenu* menu, const FilesystemTree::Node* node, int& num, QObject* const receiver, const char* const member)
	{
		for(FilesystemTree::Node::ConstIterator it = node->begin(); it != node->end(); ++it)
		{
			std::string name =(*it)->name;

			if((*it)->type == FilesystemTree::Node::File)
			{
				const size_t dotpos = name.find_last_of('.');

				if(dotpos != name.npos) //lint !e737 implicit promotion, bug in STL with size_t and std::string::npos being of different signage
					name = name.substr(0, dotpos);
			}

			if((*it)->type == FilesystemTree::Node::Folder)
			{
				QPopupMenu* subMenu = new QPopupMenu(menu, name.c_str());
				IGNORE_RETURN(menu->insertItem(name.c_str(), subMenu));
				populateMenu(subMenu,*it, num, receiver, member);
			}
			else
			{
				IGNORE_RETURN(menu->insertItem(name.c_str(), receiver, member, 0, num++));
			}
		}
	}
}

//-----------------------------------------------------------------

void GameWidget::contextMenuEvent(QContextMenuEvent* evt)
{
	if(m_gameHasFocus || !m_gs)
		return;

	NOT_NULL(m_keyStates);
	if((evt->state() & ~Qt::RightButton) != 0 || m_keyStates->hasAnyState()) //lint !e641 //enums
		return;

	evt->accept();

	NOT_NULL(m_pop);

	m_pop->clear();

	IGNORE_RETURN(ActionsGame::getInstance().gameFocusAllowed->addTo(m_pop));
	IGNORE_RETURN(ActionsView::getInstance().addCameraBookmark->addTo(m_pop));
	IGNORE_RETURN(ActionsView::getInstance().centerSelection->addTo (m_pop));
	IGNORE_RETURN(ActionsView::getInstance().centerGhosts->addTo    (m_pop));

	IGNORE_RETURN(m_pop->insertSeparator());

	const QPoint pt = mapFromGlobal(QCursor::pos());
	const ClientObject* const obj = dynamic_cast<ClientObject*>(m_gs->findObject(pt.x(), pt.y())); //use dynamic_cast because failure to cast here is acceptable
	m_menuIds.objNetworkId = obj ? obj->getNetworkId() : NetworkId::cms_invalid;

	ActionsEdit& ea = ActionsEdit::getInstance();

	if(m_menuIds.objNetworkId != NetworkId::cms_invalid)
	{
		Object const * const obj = NetworkIdManager::getObjectById(m_menuIds.objNetworkId);
		ClientObject const * const clientObj = obj ? obj->asClientObject() : NULL;

		IGNORE_RETURN(m_pop->insertItem("Object Editing..."));

		IGNORE_RETURN(ActionsView::getInstance().addObjectBookmark->addTo  (m_pop));
		IGNORE_RETURN(ActionsView::getInstance().removeObjectBookmark->addTo(m_pop));
		IGNORE_RETURN(m_pop->insertSeparator());

		//only display the form editing option if this tempate type has a form bound to it
		if(clientObj)
		{
			char const * const templateName = clientObj->getTemplateName();
			if(templateName)
			{
				FormManager::Form const * const form = FormManager::getFormForSharedObjectTemplate(templateName);
				if(form)
				{
					IGNORE_RETURN(ea.editFormData->addTo(m_pop));
					IGNORE_RETURN(m_pop->insertSeparator());
				}
			}
		}
	}

	IGNORE_RETURN(ea.copy->addTo  (m_pop));
	IGNORE_RETURN(ea.cut->addTo   (m_pop));
	IGNORE_RETURN(ea.paste->addTo (m_pop));
	IGNORE_RETURN(ea.del->addTo   (m_pop));

	IGNORE_RETURN(m_pop->insertSeparator());

	if(!GodClientData::getInstance().getSelectionEmpty())
	{
		IGNORE_RETURN(ea.applyTransform->addTo(m_pop));
		IGNORE_RETURN(ea.dropToTerrain-> addTo(m_pop));
		IGNORE_RETURN(ea.randomRotate->  addTo(m_pop));
		IGNORE_RETURN(ea.setTransform->  addTo(m_pop));
		IGNORE_RETURN(ea.editTriggers->  addTo(m_pop));
		IGNORE_RETURN(ea.showObjectAxes->addTo(m_pop));
		IGNORE_RETURN(ActionsGame::getInstance().watchTargetPath->addTo(m_pop));
		IGNORE_RETURN(ActionsGame::getInstance().ignoreTargetPath->addTo(m_pop));

		//additional options if a particle system is selected
		if(GodClientData::getInstance().isParticleSystemSelected())
		{
			IGNORE_RETURN(m_pop->insertSeparator());
			IGNORE_RETURN(ea.togglePauseParticleSystems->addTo(m_pop));
		}

		if (GodClientData::getInstance ().isBuildingSelected ())
			IGNORE_RETURN(ActionsGame::getInstance().m_saveInteriorLayout->addTo(m_pop));

		IGNORE_RETURN(m_pop->insertSeparator());
		IGNORE_RETURN(ea.unlockSelected->addTo(m_pop));
	}

	IGNORE_RETURN(ea.unlockAll->addTo(m_pop));
	IGNORE_RETURN(ea.unlockNonStructures->addTo(m_pop));
	IGNORE_RETURN(ea.unlockServerOnly->addTo(m_pop));

	IGNORE_RETURN(m_pop->insertSeparator());
	IGNORE_RETURN(ea.m_toggleDropToTerrain-> addTo(m_pop));
	IGNORE_RETURN(ea.m_toggleAlignToTerrain-> addTo(m_pop));

	IGNORE_RETURN(m_pop->insertSeparator());
	IGNORE_RETURN(ea.createObjectFromSelectedTemplate->addTo(m_pop));

	{
		IGNORE_RETURN(m_pop->insertSeparator());
		IGNORE_RETURN(m_pop->insertItem("Create Object...", m_templateMenu));
	}

	m_popupData.point = QCursor::pos();
	m_pop->popup(QCursor::pos());

}

//-----------------------------------------------------------------

void GameWidget::onTemplateMenuShow()
{
	NOT_NULL(m_templateMenu);
	m_templateMenu->clear();
	int num = 0;

	const AbstractFilesystemTree* const tree = ObjectTemplateData::getInstance().getTree();

	if(tree)
		populateMenu(m_templateMenu, tree->getRootNode(), num, this, SLOT(onCreateTemplateFromMenu(int)));
}

// ----------------------------------------------------------------------

void GameWidget::grabExclusiveFocus()
{
	if(!m_gameHasFocus)
	{
		setFocus();
		grabMouse();
		grabKeyboard();
		m_gameHasFocus = true;
		setCursor(m_blankCursor);
		DirectInput::resumeInput();

		emit gameExclusiveFocusChanged(false);

		Graphics::setAllowMouseCursorConstrained(true);
	}
}

// ----------------------------------------------------------------------

void GameWidget::releaseExclusiveFocus()
{
	if(m_gameHasFocus)
	{
		Graphics::setAllowMouseCursorConstrained(false);
		releaseMouse();
		releaseKeyboard();
		m_gameHasFocus = false;
		setCursor(m_defaultCursor);
		DirectInput::suspendInput();

		if (m_gs)
		{
			InputMap * const im = m_gs->getGodClientInputMap ();
			if (im)
			{
				im->handleInputReset ();
			}

			// preserve camera position and orientation when switching from freeCamera and freeChaseCamera
			FreeCamera* freeCamera = m_gs->getGodClientCamera ();
			GroundScene * const groundScene = safe_cast<GroundScene * const>(Game::getScene());
			if(freeCamera && groundScene)
			{
				switch(groundScene->getCurrentView())
				{
				case GroundScene::CI_freeChase:
					freeCamera->setInitializeFromFreeChaseCamera(true);
					break;
				case GroundScene::CI_free:
					freeCamera->setInitializeFromFreeCamera(true);
					break;
				}
			}
		}

		if(CuiManager::getPointerInputActive())
		{
			UIPoint pt;
			CuiManager::getIoWin().getCursorLocation(pt);
			QCursor::setPos(mapToGlobal(QPoint(pt.x, pt.y)));
		}
		else
		{
			QCursor::setPos(mapToGlobal(rect().center()));
		}

		emit gameExclusiveFocusChanged(true);
	}
}
// ----------------------------------------------------------------------

/**
 * Run this function once every second(a QTimer is set in the constructor to handle this).
 * Right now all we want to do is update the FPU widget
 */
void GameWidget::secondElapsed()
{
	//recalculate and update FPU counter on screen
	emit framesPerSecond(m_frameCounter - m_previousSecondframeCounter);
	m_previousSecondframeCounter = m_frameCounter;

	if(Game::getPlayer())
	{
		std::string const & buildoutName = SharedBuildoutAreaManager::getBuildoutNameForPosition(Game::getSceneId().c_str(), Game::getPlayer()->getPosition_w(), false);
		std::pair<std::string, std::string> const & s = SharedBuildoutAreaManager::parseEncodedBuildoutName(buildoutName.c_str());
		emit buildoutRegionChanged(s.first.c_str(), s.second.c_str());
	}
}

//-----------------------------------------------------------------

void GameWidget::updateSceneData()
{
	m_gs = dynamic_cast<GroundScene*>(Game::getScene());
	if(m_gs)
	{
		m_gs->activateGodClientCamera();
		FreeCamera* const cam  = NON_NULL(m_gs->getGodClientCamera());
		cam->setMode(FreeCamera::M_pivot);
		FreeCamera::Info fci = cam->getInfo();
		fci.distance = CONST_REAL(100);
		cam->setInfo(fci);

		SwgCuiHud * const hud = SwgCuiHudFactory::findMediatorForCurrentHud ();
		if (hud)
			hud->setHudEnabled(false);
	}

	//send the setgodmode command to the server
	ServerCommander::getInstance().setGodMode();
}

//-----------------------------------------------------------------

void GameWidget::receiveMessage(const MessageDispatch::Emitter& , const MessageDispatch::MessageBase& message)
{
#if 0
	if(message.isType(CuiIoWin::Messages::MOUSE_HIT_EDGE)  && !m_interiorCameraOn)
	{
		UIPoint pt;
		CuiManager::getIoWin().getCursorLocation(pt);
		QCursor::setPos(mapToGlobal(QPoint(pt.x, pt.y)));

		releaseExclusiveFocus();
	}
	else
#endif
	if(message.isType(Game::Messages::SCENE_CHANGED))
	{
		updateSceneData();
	}
}

//-----------------------------------------------------------------

void GameWidget::onGameFocusAllowedChanged(bool b)
{
	if(m_interiorCameraOn == true)
		return;

	if(m_gameHasFocus && !b)
		releaseExclusiveFocus();
	else if(!m_gameHasFocus && b)
		grabExclusiveFocus();
}

//-----------------------------------------------------------------

void GameWidget::onInteriorCameraChanged(bool b)
{
	if(m_interiorCameraOn == b)
		return;
	if(m_gameHasFocus == !b)
		return;
	if(!m_gameHasFocus == b)
		return;

	onGameFocusAllowedChanged(!b);

	m_interiorCameraOn = b;

	onGameFocusAllowedChanged(!b);

	if(m_gs)
		if(b)
			m_gs->activateGodClientInteriorCamera();
		else
			m_gs->deactivateGodClientInteriorCamera();

	emit interiorCameraFocusChanged(b);
}

//-----------------------------------------------------------------

void GameWidget::onPopupItemActivated(int id) const
{
	UNREF(id);
	QCursor::setPos(m_popupData.point);
}

//-----------------------------------------------------------------

void GameWidget::onPopupMenuShow() const
{
}

//-----------------------------------------------------------------

void GameWidget::onPopupMenuHide() const
{
}

//-----------------------------------------------------------------

void GameWidget::createObjectFromSelectedTemplate()
{
	GodClientData::getInstance().setObjectCreationPending(true);
	ClientObject* obj = dynamic_cast<ClientObject *>(createObjectFromTemplate(ActionsEdit::getInstance().getSelectedServerTemplate()));
	if(obj)
	{
		GodClientData::getInstance().setSelection(obj);
		GodClientData::getInstance().synchronizeSelectionWithGhosts();
	}
}

//-----------------------------------------------------------------

/**
 * @pre templateName must be fully qualified as object/<templateName>.iff already
 */
Object* GameWidget::createObjectFromTemplateAtPoint(const std::string& templateName, const QPoint& pt)
{
	if(templateName.empty())
		return NULL;

	const CellProperty* cellProperty = CellProperty::getWorldCellProperty ();
	Vector result;
	if(!GodClientData::getInstance().findIntersection_p(pt.x(), pt.y(), cellProperty, result))
	{
		//if there's no terrain, put the object at the camera's pivot point (the crossbars)
		result = GodClientData::getInstance().getCameraPivotPoint();
	}

	if(result == Vector::zero)
	{
		const std::string msg = "Trying to place object at (0, 0, 0), aborting.";
		IGNORE_RETURN(QMessageBox::warning(this, "Warning", msg.c_str()));
		return NULL;
	}

	// determine the name of the buildout file for this object

	std::string buildoutName( SharedBuildoutAreaManager::getBuildoutNameForPosition( Game::getSceneId().c_str(), result, false ) );
	std::pair<std::string, std::string> const & s = SharedBuildoutAreaManager::parseEncodedBuildoutName(buildoutName.c_str());


	bool okToPlaceObj = true;

	if ( s_askQuestionAboutBuildoutFiles )
	{
		s_askQuestionAboutBuildoutFiles = false; // <- make sure we don't ask again for this session

		int messageBoxResult = QMessageBox::question(
			0,
			"Buildout Files",
			"Do you want to automatically open buildout files?",
			QMessageBox::Yes , QMessageBox::No );

		if ( messageBoxResult == QMessageBox::Yes )
		{
			s_openBuildoutFiles = true;
		}

	}
	if ( s_openBuildoutFiles && s.second != s_lastBuildoutFileOpened )
	{
		okToPlaceObj = BuildoutAreaSupport::openBuildoutFilesForEditing( s.second );
	}
	
	Object *obj = 0;

	// get the names of the data files
	if ( okToPlaceObj == true )
	{
		// save the name of the buildout area so that we don't try and open it every time we place an object
		s_lastBuildoutFileOpened = s.second; 

		// try and create the object
		Transform transform;
		transform.setPosition_p(result);
		obj = ServerCommander::getInstance().createObject("Newb", templateName, cellProperty, transform);
	}

	return obj;
	
}

//-----------------------------------------------------------------

NetworkId GameWidget::getCursorPositionIntoWorld (Vector & result/*OUT*/)
{
	CellProperty const * resultCell = CellProperty::getWorldCellProperty();
	QPoint const p(mapFromGlobal(QCursor::pos()));
	resultCell = CellProperty::getWorldCellProperty ();
	if(!GodClientData::getInstance().findIntersection_p(p.x(), p.y(), resultCell, result))
	{
		//if there's no terrain, put the object at the camera's pivot point (the crossbars)
		result = GodClientData::getInstance().getCameraPivotPoint();
		resultCell = CellProperty::getWorldCellProperty();
	}
	if(resultCell)
		return resultCell->getOwner().getNetworkId();
	else
		return NetworkId::cms_invalid;
}

//-----------------------------------------------------------------

/**
 * @pre templateName must *not* be fully qualified (i.e. it must be apron, *not* object/apron.iff)
 */
Object* GameWidget::createObjectFromTemplate(const std::string& templateName)
{
	if(templateName.empty())
		return NULL;

	//the template name  needs to be of the form (object/<templateName>.iff)
	std::string fullTemplateName = std::string("object/") + templateName + std::string(".iff");
	return createObjectFromTemplateAtPoint(fullTemplateName, QPoint(mapFromGlobal(QCursor::pos())));
}

//-----------------------------------------------------------------

void GameWidget::dragEnterEvent(QDragEnterEvent* evt)
{
	QString text;
	if(QTextDrag::decode(evt, text))
	{
		if(text == ObjectTemplateData::DragMessages::SERVER_TEMPLATE_DRAGGED)
			evt->accept();

		if(text == ObjectTemplateData::DragMessages::CLIENT_TEMPLATE_DRAGGED)
			evt->accept();

		if(text == ActionsEdit::DragMessages::PALETTE_DRAGGED_INTO_GAME)
			evt->accept();

		if(text == BrushData::Messages::BRUSH_DRAGGED)
			evt->accept();

		if(text == ActionsScript::DragMessages::SCRIPT_DRAGGED)
			evt->accept();

		if(text == FavoritesListView::DragMessages::FAVORITES_SERVER_TEMPLATE_DRAGGED)
			evt->accept();

		if(text == FavoritesListView::DragMessages::FAVORITES_CLIENT_TEMPLATE_DRAGGED)
			evt->accept();
	}
}

//-----------------------------------------------------------------

void GameWidget::dropEvent(QDropEvent* evt)
{
	QString text;

	const QPoint pt = evt->pos();

	if(QTextDrag::decode(evt, text))
	{
		if(text == ObjectTemplateData::DragMessages::SERVER_TEMPLATE_DRAGGED)
		{
			if(Game::getSinglePlayer())
			{
				createObjectFromSelectedTemplate();
			}
			else
			{
				//if this object template has a form associated with it, launch that process now
				std::string const notFullyQualifiedTemplateName = ActionsEdit::getInstance().getSelectedServerTemplate();
				std::string const fullyQualifiedTemplateName = std::string("object/") + notFullyQualifiedTemplateName + std::string(".iff");

				FormManager::Form const * const form = FormManager::getFormForServerObjectTemplate(fullyQualifiedTemplateName);
				if(form)
				{
					FormWindow* formWindow = new FormWindow(&MainFrame::getInstance(), form->getName().c_str());
					formWindow->setDisplayReason(FormManager::CREATE_OBJECT);
					formWindow->setCurrentForm(*form);
					formWindow->setServerObjectTemplate(fullyQualifiedTemplateName);
					Vector pos;
					NetworkId const cellId = getCursorPositionIntoWorld(pos);
					formWindow->setCreatePosition(pos, cellId);
					formWindow->populatePage();
					formWindow->show();
				}
				else
				{
					GodClientData::getInstance().setObjectCreationPending(true);
					IGNORE_RETURN(createObjectFromTemplate(notFullyQualifiedTemplateName));
				}
			}
			evt->accept();
		}

		else if(text == ObjectTemplateData::DragMessages::CLIENT_TEMPLATE_DRAGGED)
		{
			if(!Game::getSinglePlayer())
			{
				IGNORE_RETURN(QMessageBox::warning(this, "Don't use client templates in multi player", "You cannot instanciate shared templates in multi player, use the corresponding server template"));
			}
			else
			{
				ClientObject* obj = dynamic_cast<ClientObject *>(createObjectFromTemplate(ActionsEdit::getInstance().getSelectedClientTemplate()));
				if(obj)
				{
					GodClientData::getInstance().setSelection(obj);
					GodClientData::getInstance().synchronizeSelectionWithGhosts();
				}

			}
			evt->accept();
		}

		else if(text == BrushData::Messages::BRUSH_DRAGGED)
		{
			//set the clipboard with the current brush
			GodClientData::getInstance().setCurrentBrush(*BrushData::getInstance().getSelectedBrush());

			//update the cursor before we paste, so we paste at cursor
			GodClientData::getInstance().cursorScreenPositionChanged(pt.x(), pt.y());

			//now paste that brush into the world as usual
			ActionsEdit::getInstance().pasteBrush->doActivate();
			evt->accept();
		}

		else if(text == ActionsEdit::DragMessages::PALETTE_DRAGGED_INTO_GAME)
		{
			IGNORE_RETURN(createObjectFromTemplate(GodClientData::getInstance().getSelectedPalette()));
			evt->accept();

		}
		else if(text == ActionsScript::DragMessages::SCRIPT_DRAGGED)
		{
			NOT_NULL(m_gs);
			ClientObject* const obj = dynamic_cast<ClientObject *>(m_gs->findObject(pt.x(), pt.y()));
			if(obj)
			{
				std::string scriptName;
				if(ActionsScript::getInstance().getSelectedScript(scriptName))
				{
					IGNORE_RETURN(ServerCommander::getInstance().scriptAttach(*obj, scriptName));
					m_updateObjects.emitMessage(UpdateObjects());
				}
			}
			evt->accept();
		}
		else if(text == FavoritesListView::DragMessages::FAVORITES_SERVER_TEMPLATE_DRAGGED)
		{
			
			if(Game::getSinglePlayer())
			{
				GodClientData::getInstance().setObjectCreationPending(true);
				ClientObject* obj = dynamic_cast<ClientObject *>(createObjectFromTemplate(MainFrame::getInstance().getFavoritesWindow()->getSelectedItemTemplateName()));
				if(obj)
				{
					GodClientData::getInstance().setSelection(obj);
					GodClientData::getInstance().synchronizeSelectionWithGhosts();
				}
			}
			else
			{
				//if this object template has a form associated with it, launch that process now
				std::string const notFullyQualifiedTemplateName = MainFrame::getInstance().getFavoritesWindow()->getSelectedItemTemplateName();
				std::string const fullyQualifiedTemplateName = std::string("object/") + notFullyQualifiedTemplateName + std::string(".iff");

				FormManager::Form const * const form = FormManager::getFormForServerObjectTemplate(fullyQualifiedTemplateName);
				if(form)
				{
					FormWindow* formWindow = new FormWindow(&MainFrame::getInstance(), form->getName().c_str());
					formWindow->setDisplayReason(FormManager::CREATE_OBJECT);
					formWindow->setCurrentForm(*form);
					formWindow->setServerObjectTemplate(fullyQualifiedTemplateName);
					Vector pos;
					NetworkId const cellId = getCursorPositionIntoWorld(pos);
					formWindow->setCreatePosition(pos, cellId);
					formWindow->populatePage();
					formWindow->show();
				}
				else
				{
					GodClientData::getInstance().setObjectCreationPending(true);
					IGNORE_RETURN(createObjectFromTemplate(notFullyQualifiedTemplateName));
				}
			}
			
			evt->accept();
		}
		else if(text == FavoritesListView::DragMessages::FAVORITES_CLIENT_TEMPLATE_DRAGGED)
		{
			
			if(!Game::getSinglePlayer())
			{
				IGNORE_RETURN(QMessageBox::warning(this, "Don't use client templates in multi player", "You cannot instanciate shared templates in multi player, use the corresponding server template"));
			}
			else
			{
				ClientObject* obj = dynamic_cast<ClientObject *>(createObjectFromTemplate(MainFrame::getInstance().getFavoritesWindow()->getSelectedItemTemplateName()));
				if(obj)
				{
					GodClientData::getInstance().setSelection(obj);
					GodClientData::getInstance().synchronizeSelectionWithGhosts();
				}

			}
			
			evt->accept();
		}
	}
}

//-----------------------------------------------------------------

namespace
{
	const FilesystemTree::Node* findLeafNode(const FilesystemTree::Node* node, int id, int& counter, std::string& path)
	{
		for(FilesystemTree::Node::ConstIterator it = node->begin(); it != node->end(); ++it)
		{
			if((*it)->type == FilesystemTree::Node::File)
			{
				if(id == counter++)
				{
					path += "/" +(*it)->name;
					return*it;
				}
			}
			else
			{
				std::string subPath = path + "/" +(*it)->name;
				const FilesystemTree::Node* const result = findLeafNode(*it, id, counter, subPath);

				if(result)
				{
					path = subPath;
					return result;
				}
			}
		}
		return 0;
	}
}

//-----------------------------------------------------------------

void GameWidget::onCreateTemplateFromMenu(int id)
{
	QCursor::setPos(m_popupData.point);

	const AbstractFilesystemTree* fst = ObjectTemplateData::getInstance().getTree();

	if(!fst)
		return;

	int counter = 0;
	std::string path;
	const FilesystemTree::Node* const node = findLeafNode(fst->getRootNode(), id, counter, path);

	if(node)
	{
		if(!path.empty() && path[0] == '/')
			path = path.substr(1);

		if(path.size() > 3 && !_stricmp(path.substr(path.size() - 4).c_str(), ".iff"))
			path = path.substr(0, path.size() - 4);

		IGNORE_RETURN(createObjectFromTemplate(path));
	}
}

//-----------------------------------------------------------------

void GameWidget::onAddObjectBookmark()
{
	if(m_gs == 0)
		return;

	const Object* obj = 0;
	m_menuIds.objNetworkId = obj ? obj->getNetworkId() : NetworkId::cms_invalid;

	if(m_menuIds.objNetworkId != NetworkId::cms_invalid)
	{
		obj = NetworkIdManager::getObjectById (m_menuIds.objNetworkId);

		if(obj == 0)
		{
			char buf [128];
			IGNORE_RETURN(_snprintf(buf, 128, "Object with Network ID %d not found.\n", m_menuIds.objNetworkId));
			IGNORE_RETURN(QMessageBox::warning(this, "Warning", buf));
			return;
		}
	}
	else
	{
		const QPoint pt = mapFromGlobal(QCursor::pos());
		obj = dynamic_cast<ClientObject*>(m_gs->findObject(pt.x(), pt.y())); //use dynamic_cast because failure to cast here is acceptable
	}
	if(obj == 0)
	{
		char buf [128];
		IGNORE_RETURN(_snprintf(buf, 128, "No Object Indicated\n"));
		IGNORE_RETURN(QMessageBox::warning(this, "Warning", buf));
		return;
	}

	FreeCamera* const camera = NON_NULL(m_gs->getGodClientCamera());
	const std::string theName = obj->getDebugName() ? obj->getDebugName() : "Unnamed";
	FreeCamera::Info fci = camera->getInfo();
	fci.yaw -= obj->getObjectFrameK_p().theta();
	fci.distance = obj->getPosition_w().magnitudeBetween(camera->getPosition_w());
	BookmarkData::getInstance().addObjectBookmark(obj->getNetworkId(), theName, fci);
}

//-----------------------------------------------------------------

void GameWidget::focusInEvent( QFocusEvent* )
{
	NOT_NULL(m_keyStates);
	m_keyStates->reset();
}

//----------------------------------------------------------------------

void GameWidget::setFrameRateLimit(int fps)
{
	if(fps)
		m_minFrameLength = std::max(1, 1000 / fps);
}

//----------------------------------------------------------------------

int GameWidget::getFrameRateLimit() const
{
	if(m_minFrameLength)
		return 1000 / m_minFrameLength;
	else
		return 0;
}

//----------------------------------------------------------------------

void GameWidget::onEditFormData(FormManagerClient::Messages::EditFormData::Payload const & payload)
{
	NetworkId const & objectToEdit = payload.first;
	Unicode::String const & packedData = payload.second;
	FormManager::UnpackedFormData const unpackedData = FormManager::unpackFormData(packedData);

	Object const * const obj = NetworkIdManager::getObjectById(objectToEdit);
	ClientObject const * const clientObj = obj ? obj->asClientObject() : NULL;

	if(clientObj)
	{
		char const * const sharedTemplateName = clientObj->getTemplateName();
		if(sharedTemplateName)
		{
			FormManager::Form const * const form = sharedTemplateName ? FormManager::getFormForSharedObjectTemplate(sharedTemplateName) : NULL;
			if(form)
			{
				FormWindow* formWindow = new FormWindow(&MainFrame::getInstance(), form->getName().c_str());
				formWindow->setDisplayReason(FormManager::EDIT_OBJECT);
				formWindow->setCurrentForm(*form);
				formWindow->setObject(clientObj);
				formWindow->populatePage(unpackedData);
				formWindow->show();
			}
		}
	}
}

// ======================================================================
