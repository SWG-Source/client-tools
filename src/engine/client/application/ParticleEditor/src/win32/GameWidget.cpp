// ======================================================================
//
// GameWidget.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "FirstParticleEditor.h"

// module includes

#include "GameWidget.h"
#include "GameWidget.moc"

// engine shared includes

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/ApplicationVersion.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedGame/SetupSharedGame.h"
#include "sharedImage/SetupSharedImage.h"
#include "sharedInputMap/InputMap.h"
#include "sharedIoWin/IoWinManager.h"
#include "sharedIoWin/SetupSharedIoWin.h"
#include "sharedMath/SetupSharedMath.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/SetupSharedObject.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedRegex/SetupSharedRegex.h"
#include "sharedTerrain/SetupSharedTerrain.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedUtility/SetupSharedUtility.h"
#include "sharedXml/SetupSharedXml.h"

// engine client includes

#include "clientAnimation/SetupClientAnimation.h"
#include "clientAudio/SetupClientAudio.h"
#include "clientBugReporting/SetupClientBugReporting.h"
#include "clientBugReporting/ToolBugReporting.h"
#include "clientDirectInput/DirectInput.h"
#include "clientDirectInput/SetupClientDirectInput.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/FreeCamera.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/SetupClientGame.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/SetupClientGraphics.h"
#include "clientObject/SetupClientObject.h"
#include "clientParticle/SetupClientParticle.h"
#include "clientSkeletalAnimation/SetupClientSkeletalAnimation.h"
#include "clientTextureRenderer/SetupClientTextureRenderer.h"
#include "clientTerrain/SetupClientTerrain.h"

// Cui includes

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiPreferences.h"
#include "swgClientUserInterface/SwgCuiHud.h"
#include "swgClientUserInterface/SwgCuiHudFactory.h"
#include "swgClientUserInterface/SwgCuiInventory.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgClientUserInterface/SwgCuiManager.h"

#include <dinput.h>
#include <map>

// ======================================================================
namespace
{
	//-- @todo: preferences manager should handle key mapping
	const int s_translateKey       = static_cast<int>(Qt::Key_T);
	const int s_groundTranslateKey = static_cast<int>(Qt::Key_G);
	const int s_scaleKey           = static_cast<int>(Qt::Key_S);
	const int s_rotateKey          = static_cast<int>(Qt::Key_R);
	const int s_shiftKey           = static_cast<int>(Qt::Key_Shift);

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
}
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

	//-----------------------------------------------------------------
	//-- not 'any' state, really.  hasAnyState should be hasAnyStateThatOverridesContextMenusAndSelectionEvents

	inline bool hasAnyState()
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

GameWidget *GameWidget::ms_globalGameWidget;

//-----------------------------------------------------------------

GameWidget::GameWidget(QWidget* theParent, const char*theName, WFlags const flags)
 : QWidget(theParent, theName, flags) //lint !e578 //hides
 , Receiver()
 , m_frameCounter(0)
 , m_defaultCursor(static_cast<int>(ArrowCursor))
 , m_blankCursor(static_cast<int>(BlankCursor))
 , m_previousSecondframeCounter(0)
 , m_gameHasFocus(false)
 , m_lastMousePoint()
 , m_timer(0)
 , m_secondTimer(0)
 , m_keyStates(0)
 , m_pop(0)
 , m_templateMenu(0)
 , m_lastOID(10000)
 , m_rubberBandRect()
 , m_rubberBanding(false)
 , m_rubberBandOk(false)
 , m_mouseDownPoint()
 , m_flyButtonDown(false)
 , m_popupData()
 , m_gs(0)
 , m_discardNextMouseRelease(false)
 , m_warpMouse(false)
 , m_autoDraggingObjects(false)
 , m_autoDragObjectId(0)
 , m_minFrameLength(50)
 , m_lastFrameTimestamp()
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
		SetupSharedFoundation::Data data(SetupSharedFoundation::Data::D_mfc);
		data.useWindowHandle    = true;
		data.processMessagePump = false;
		data.windowHandle       = topLevelWindow;
		data.configFile         = "ParticleEditor.cfg";
		data.clockUsesSleep     = true;
        data.writeMiniDumps		= ApplicationVersion::isBootlegBuild();
	    SetupSharedFoundation::install(data);

	    SetupClientBugReporting::install();

        if (ApplicationVersion::isBootlegBuild())
        {
	        ToolBugReporting::startCrashReporter();
        }

		SetupSharedCompression::install();

		//-- regex
		SetupSharedRegex::install();

		//-- file
		SetupSharedFile::install(false);

		//-- utility
		SetupSharedUtility::Data setupUtilityData;
		SetupSharedUtility::setupGameData (setupUtilityData);
		SetupSharedUtility::install (setupUtilityData);

		//-- math
		SetupSharedMath::install();

		//-- random
		SetupSharedRandom::install(static_cast<uint32>(time(NULL)));

		//-- image
		SetupSharedImage::Data setupImageData;
		SetupSharedImage::setupDefaultData(setupImageData);
		SetupSharedImage::install(setupImageData);

		//-- object
		SetupSharedObject::Data setupObjectData;
		SetupSharedObject::setupDefaultGameData(setupObjectData);
		// we want the SlotIdManager initialized, and we need the associated hardpoint names loaded.
		SetupSharedObject::addSlotIdManagerData(setupObjectData, true);
		// we want CustomizationData support on the client.
		SetupSharedObject::addCustomizationSupportData(setupObjectData);
		SetupSharedObject::install(setupObjectData);

		//-- game
		SetupSharedGame::Data  sharedGameData;
		sharedGameData.setUseGameScheduler(true);
		SetupSharedGame::install(sharedGameData);

		//-- terrain
		SetupSharedTerrain::Data setupSharedTerrainData;
		SetupSharedTerrain::setupGameData(setupSharedTerrainData);
		SetupSharedTerrain::install(setupSharedTerrainData);

		//-- SharedXml
		SetupSharedXml::install();

		//-- audio
		SetupClientAudio::install();

		//-- graphicsscreenshot
		SetupClientGraphics::Data setupGraphicsData;
		SetupClientGraphics::setupDefaultGameData(setupGraphicsData);
		SetupClientGraphics::install(setupGraphicsData);
		Graphics::setTranslatePointFromGameToScreen(translatePointFromGameToScreen);

		//-- directinput
		SetupClientDirectInput::install(GetModuleHandle(NULL), topLevelWindow, DIK_LMENU, Graphics::isWindowed);
		DirectInput::suspendInput();

		//-- object
		SetupClientObject::Data setupClientObjectData;
		SetupClientObject::setupToolData (setupClientObjectData);
		SetupClientObject::install (setupClientObjectData);

		//-- animation and skeletal animation
		SetupClientAnimation::install();

		SetupClientSkeletalAnimation::Data  saData;
		SetupClientSkeletalAnimation::setupToolData(saData);
		SetupClientSkeletalAnimation::install (saData);

		//-- texture renderer
		SetupClientTextureRenderer::install();

		//-- terrain
		SetupClientTerrain::install();

		//-- particles
		SetupClientParticle::install();

		//-- game
		SetupClientGame::Data clientGameData;
		SetupClientGame::setupGameData(clientGameData);
		SetupClientGame::install(clientGameData);
	
		//-- iowin (must come after SetupClientGame, even though "shared")
		SetupSharedIoWin::install();
	}

	//----------------------------------------------------------------------
	//-- override the platform frame rate limiter with our own, if needed

	Clock::noFrameRateLimit();

	CuiManager::setImplementationInstallFunctions(SwgCuiManager::install, SwgCuiManager::remove, SwgCuiManager::update);

	Game::install(Game::A_particleEditor);

	// turn off the mousemode being default
	CuiPreferences::setMouseModeDefault (false);

	QWidget::setFixedSize(Graphics::getFrameBufferMaxWidth(), Graphics::getFrameBufferMaxHeight());
	QWidget::resize(Graphics::getFrameBufferMaxWidth(), Graphics::getFrameBufferMaxHeight());

	m_timer       = NON_NULL(new QTimer(this, "timer"));
	m_secondTimer = NON_NULL(new QTimer(this, "secondTimer"));

	// set up a timer to issue repaints to run the game loop as fast as possible
	// NOTE: this particular timer must be a zero-length timer to avoid a Qt bug
	IGNORE_RETURN(connect(m_timer, SIGNAL(timeout()), this, SLOT(runGameLoop())));

	int requestedFrameRate = atoi(ConfigFile::getKeyString("ParticleEditor", "frameRateLimit", "20"));

	if (requestedFrameRate <= 0)
	{
		requestedFrameRate = 300;
	}

	//IGNORE_RETURN(m_timer->start(1000 / requestedFrameRate));
	IGNORE_RETURN(m_timer->start(0));

	// set up a timer to update the frame rate once per second
	IGNORE_RETURN(connect(m_secondTimer, SIGNAL(timeout()), this, SLOT(secondElapsed())));
	IGNORE_RETURN(m_secondTimer->start(1000, false));

	QWidget::setFocusPolicy(QWidget::StrongFocus);

	connectToMessage(Game::Messages::SCENE_CHANGED);
	
	m_pop = new QPopupMenu(this, "GameWidget_popup");
	IGNORE_RETURN(connect(m_pop, SIGNAL(activated (int)), this, SLOT(onPopupItemActivated(int))));

	m_templateMenu = new QPopupMenu(this, "GameWidget_templateMenu");
	IGNORE_RETURN(connect(m_templateMenu, SIGNAL(aboutToShow()), this, SLOT(onTemplateMenuShow())));

	m_keyStates = new KeyStates;

	updateSceneData();

	Graphics::setHardwareMouseCursorEnabled(false);
}

// ----------------------------------------------------------------------

GameWidget::~GameWidget()
{
	ms_globalGameWidget = NULL;

	disconnectFromMessage(CuiIoWin::Messages::MOUSE_HIT_EDGE);
	
	delete m_keyStates;
	m_keyStates = 0;

	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	delete m_timer;
	delete m_secondTimer;
	m_timer = 0;
	m_secondTimer = 0;

}

// ----------------------------------------------------------------------
void GameWidget::runGameLoop()
{
	static bool processingThisFunction = false;

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

	// Try to close the ground hud for the first 100 frames

	SwgCuiHud * const hud = SwgCuiHudFactory::findMediatorForCurrentHud();
	if (hud && hud->getHudEnabled() && m_frameCounter < 100)
		hud->setHudEnabled(false);

	static bool inventoryInitialized = false;
	
	if (   (inventoryInitialized == false)
	    && (Game::getClientPlayer() != NULL))
	{
		inventoryInitialized = true;
		SwgCuiInventory::setupPlayerInventory(Game::getClientPlayer());
	}

	Sleep(20.0f);

	processingThisFunction = false;
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
			//if(GodClientData::getInstance().killGhosts() == 0)
			//	GodClientData::getInstance().clearSelection();
		}
	}

	if(keyEvent->key() == static_cast<int>(Key_Enter) ||
		(keyEvent->key() == static_cast<int>(Key_F8) && hasState(keyEvent, Qt::ControlButton)))
	{
		onGameFocusAllowedChanged(!m_gameHasFocus);
		//ActionsGame::getInstance().gameFocusAllowed->doActivate();
		return;
	}

	if(!m_gameHasFocus)
	{
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
	
	else
	{
		// send key press messages
		QString text = keyEvent->text();
		for(uint i = 0; i < text.length(); ++i)
			IoWinManager::queueCharacter(0, text.at(i).latin1());
	}
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

	keyEvent->ignore();
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
		
	//if(ActionsGame::getInstance().gameFocusAllowed->isOn())
	//if (m_gameHasFocus)
	//{
	//	CuiManager::getIoWin().warpCursor(mouseEvent->x(), mouseEvent->y());
	//	grabExclusiveFocus();
	//	mouseEvent->accept();
	//	return;
	//}

	m_rubberBanding = false;
	
	if(!m_gs)
		return;
	
	const QPoint pt(mouseEvent->pos());
	//GodClientData::getInstance().cursorScreenPositionChanged(pt.x(), pt.y());

	const real dx =(mouseEvent->x() - m_lastMousePoint.x())* CONST_REAL(0.01);
	const real dy =(mouseEvent->y() - m_lastMousePoint.y())* CONST_REAL(0.01);
	
	NOT_NULL(m_keyStates);

	m_gs->activateGodClientCamera();
	FreeCamera* const cam  = NON_NULL(m_gs->getGodClientCamera());

	//-----------------------------------------------------------------
	//-- use maya movement controls
	
	if(hasState(mouseEvent, QWidget::AltButton))
	{
		InputMap* const imap   = NON_NULL(m_gs->getGodClientInputMap());
		MessageQueue* const q  = NON_NULL(imap->getMessageQueue());

		//-----------------------------------------------------------------
		//-- zoom the camera(middle button or chorded middle), "Track"
		
		if(isMiddleButtonMove(mouseEvent))
		{
			//-----------------------------------------------------------------
			//-- fly the camera

			if(hasState(mouseEvent, QWidget::ControlButton))
			{
				cam->setMode(FreeCamera::M_fly);
				q->appendMessage(static_cast<int>(CM_cameraYaw),   dx);
				q->appendMessage(static_cast<int>(CM_cameraPitch), dy);
			}

			//-----------------------------------------------------------------
			//-- zoom the pivot

			else
			{
				q->appendMessage(static_cast<int>(CM_cameraPivotZoom), -dx);
			}
		}
		
		//-----------------------------------------------------------------
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
		
		//-----------------------------------------------------------------
		//-- translate the camera
		
		else if(hasState(mouseEvent, QWidget::RightButton))
		{
			const TerrainObject* const terrain = NON_NULL(TerrainObject::getInstance());

			//-----------------------------------------------------------------
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

			//-----------------------------------------------------------------
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
	
	
	//-----------------------------------------------------------------
	else if(m_autoDraggingObjects && !m_keyStates->hasAnyState())
	{
		const QPoint pt(mouseEvent->pos());

		Vector result;
		//if(GodClientData::getInstance().getTerrainIntersection(pt.x(), pt.y(), result))
		//{
		//	IGNORE_RETURN(GodClientData::getInstance().moveGhostsFollowingObject(m_autoDragObjectId, result, true));
		//}
	}
	else if(m_keyStates->state(s_translateKey))
	{
		if(isMiddleButtonMove(mouseEvent))
		{
			//pullSelection(dx, false);
		}
		else if(hasState(mouseEvent, Qt::RightButton))
		{
			//GodClientData::getInstance().translateSelectionY(-dy);					
		}
		else if(hasState(mouseEvent, Qt::LeftButton))
		{
			//GodClientData::getInstance().translateSelection(dx, dy, false);
		}
	}
	//-----------------------------------------------------------------
	else if(m_keyStates->state(s_groundTranslateKey))
	{
		if(isMiddleButtonMove(mouseEvent))
		{
			//pullSelection(dx, true);
		}
		else if(hasState(mouseEvent, Qt::LeftButton))
		{
			//GodClientData::getInstance().translateSelection(dx, dy, true);
		}
		else if(hasState(mouseEvent, Qt::RightButton))
		{
			//GodClientData::getInstance().translateSelectionY(-dy);					
		}
		
		if(hasState(mouseEvent, Qt::ControlButton))
		{
			//GodClientData::getInstance().alignGhostsToTerrain();
		}
	}
	//-----------------------------------------------------------------
	
	else if(m_keyStates->state(s_rotateKey))
	{
		//GodClientData::RotationPivotType pivotType = static_cast<GodClientData::RotationPivotType>(ActionsEdit::getInstance().getRotatePivotMode());
		
		//GodClientData::RotationType type = GodClientData::Rotate_none;
		real delta = dx;
		
		if(isMiddleButtonMove(mouseEvent))
		{
		//	type = GodClientData::Rotate_pitch;
		}
		else if(hasState(mouseEvent, Qt::LeftButton))
		{
		//	type = GodClientData::Rotate_yaw;
		}
		else if(hasState(mouseEvent, Qt::RightButton))
		{
		//	type = GodClientData::Rotate_roll;
			delta = -delta;
		}
		
		//-----------------------------------------------------------------
		//-- do it
		
		//if(type != GodClientData::Rotate_none)
		//{
		//	GodClientData::getInstance().rotateGhosts(delta, type, pivotType);
		//}
	}
	
	//-----------------------------------------------------------------
	
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

	Vector result;

	emit cursorWorldPositionChanged(result.x, result.y, result.z);
}

//-----------------------------------------------------------------
void GameWidget::wheelEvent(QWheelEvent* evt)
{
	NOT_NULL(m_pop);

	//----------------------------------------------------------------------
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
			onGameFocusAllowedChanged(true);
			CuiManager::getIoWin().warpCursor(mouseEvent->x(), mouseEvent->y());
			return;
		}

		m_mouseDownPoint = m_lastMousePoint = mouseEvent->pos();
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

		//-----------------------------------------------------------------
		//-- handle click-to-select

		if(m_gs)
		{	
			
			if(!s_mustApplyTransforms)
			{
				//GodClientData::getInstance().synchronizeSelectionWithGhosts();
			}

			NOT_NULL(m_keyStates);
			if(m_keyStates->hasAnyState())
				return;
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
			m_discardNextMouseRelease = true;
		}
	}	
}

//-----------------------------------------------------------------

void GameWidget::contextMenuEvent(QContextMenuEvent* evt)
{
	UNREF(evt);
}
//-----------------------------------------------------------------
void GameWidget::onTemplateMenuShow()
{
	NOT_NULL(m_templateMenu);
	m_templateMenu->clear();
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
		Graphics::setAllowMouseCursorConstrained(true);

		emit gameExclusiveFocusChanged(false);
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

		SwgCuiHud * const hud = SwgCuiHudFactory::findMediatorForCurrentHud();
		if (hud)
			hud->setHudEnabled(false);
	}
}
//-----------------------------------------------------------------

void GameWidget::receiveMessage(const MessageDispatch::Emitter& , const MessageDispatch::MessageBase& message)
{
	if(message.isType(Game::Messages::SCENE_CHANGED))
	{
		updateSceneData();
	}
}
//-----------------------------------------------------------------

void GameWidget::onGameFocusAllowedChanged(bool b)
{
	if(m_gameHasFocus && !b)
		releaseExclusiveFocus();
	else if(!m_gameHasFocus && b)
		grabExclusiveFocus();

}
//-----------------------------------------------------------------

void GameWidget::onPopupItemActivated(int id)
{
	UNREF(id);
	QCursor::setPos(m_popupData.point);
}
//-----------------------------------------------------------------
void GameWidget::onPopupMenuShow()
{
}
//-----------------------------------------------------------------
void GameWidget::onPopupMenuHide()
{
}

//-----------------------------------------------------------------
void GameWidget::createObjectFromSelectedTemplate()
{
	//createObjectFromTemplate(ActionsEdit::getInstance().getSelectedTemplate());
}
//-----------------------------------------------------------------

void GameWidget::createObjectFromTemplateAtPoint(const std::string& templateName, const QPoint& pt)
{
	UNREF(pt);

	if(templateName.empty())
		return;

	if(m_gs == 0)
		return;

	Vector result;

	Transform transform;
	transform.setPosition_p(result);
}
//-----------------------------------------------------------------

void GameWidget::createObjectFromTemplate(const std::string& templateName)
{
	if(templateName.empty())
		return;
	
	createObjectFromTemplateAtPoint(templateName, QPoint(mapFromGlobal(QCursor::pos())));
}
//-----------------------------------------------------------------
void GameWidget::dragEnterEvent(QDragEnterEvent *)
{
}

//-----------------------------------------------------------------

void GameWidget::dropEvent(QDropEvent *)
{
}

//-----------------------------------------------------------------

void GameWidget::onCreateTemplateFromMenu(int)
{
	QCursor::setPos(m_popupData.point);
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

// ======================================================================
