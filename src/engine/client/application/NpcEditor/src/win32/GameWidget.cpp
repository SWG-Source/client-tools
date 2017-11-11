// ============================================================================
//
// GameWidget.cpp
// Copyright 2004, Sony Online Entertainment
//
// ============================================================================

#include "FirstNpcEditor.h"

//-- module includes

#include "GameWidget.h"
#include "GameWidget.moc"
#include "NpcCuiMediatorTypes.h"
#include "NpcCuiManager.h"
#include "NpcCuiViewer.h"

//-- engine shared includes

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ApplicationVersion.h"
#include "sharedGame/SetupSharedGame.h"
#include "sharedGame/AssetCustomizationManager.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedImage/SetupSharedImage.h"
#include "sharedInputMap/InputMap.h"
#include "sharedIoWin/SetupSharedIoWin.h"
#include "sharedIoWin/IoWinManager.h"
#include "sharedMath/SetupSharedMath.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/SetupSharedObject.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/CustomizationVariable.h"
#include "sharedObject/RangedIntCustomizationVariable.h"
#include "sharedObject/BasicRangedIntCustomizationVariable.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/MemoryBlockManagedObject.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedRegex/SetupSharedRegex.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedUtility/SetupSharedUtility.h"
#include "sharedXml/SetupSharedXml.h"

//-- engine client includes

#include "clientAnimation/SetupClientAnimation.h"
#include "clientAudio/SetupClientAudio.h"
#include "clientDirectInput/DirectInput.h"
#include "clientDirectInput/SetupClientDirectInput.h"
#include "clientGame/Game.h"
#include "clientGame/SetupClientGame.h"
#include "clientGame/CreatureObject.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/GraphicsOptionTags.h"
#include "clientGraphics/ScreenShotHelper.h"
#include "clientGraphics/SetupClientGraphics.h"
#include "clientObject/SetupClientObject.h"
#include "clientParticle/SetupClientParticle.h"
#include "clientSkeletalAnimation/SetupClientSkeletalAnimation.h"
#include "clientTextureRenderer/SetupClientTextureRenderer.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/SkeletalAppearanceTemplate.h"
#include "clientBugReporting/SetupClientBugReporting.h"
#include "clientBugReporting/ToolBugReporting.h"

//-- Cui includes

#include "UIManager.h"
#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiBackdrop.h"

//-- Qt includes

#include <qcombobox.h>
#include <qdragobject.h>
#include <qdir.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qtimer.h>

//-- system includes

#include <dinput.h>
#include <time.h>
#include <map>

// ============================================================================

namespace
{
	//-- configuration parameters
	std::string wearableDirectory;
}

GameWidget *GameWidget::ms_globalGameWidget;

// ----------------------------------------------------------------------------

GameWidget::GameWidget(QWidget* theParent, const char*theName, WFlags const flags)
 : QWidget(theParent, theName, flags) //lint !e578 //hides
 , m_defaultCursor(static_cast<int>(ArrowCursor))
 , m_blankCursor(static_cast<int>(BlankCursor))
 , m_gameHasFocus(false)
 , m_timer(0)
 , m_minFrameLength(50)
 , m_npcViewer(0)
 , m_npcBackdrop(0)
 , m_isDirty(false)
{
	ms_globalGameWidget = this;

	QWidget::setBackgroundMode(Qt::NoBackground);
	QWidget::setMouseTracking(true);

	// @todo  Move all this installation code into SetupSystem.
	//        This widget should be platform independent.

	//-- get information about the GameWidget's HWND
	HWND topLevelWindow = static_cast<HWND>(winId());

	// AnimationEditor's GameWidget window is a child window.
#if 1
	HWND parentWindow;
	do
	{
		parentWindow = GetParent(topLevelWindow);
		if(parentWindow)
			topLevelWindow = parentWindow;
	} while(parentWindow);
#endif

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
		data.configFile         = "NpcEditor.cfg";
		data.clockUsesSleep     = true;
		data.writeMiniDumps	    = ApplicationVersion::isBootlegBuild();
		SetupSharedFoundation::install(data);

		//-- crash watcher support
		SetupClientBugReporting::install();

		if (ApplicationVersion::isBootlegBuild())
		{
			IGNORE_RETURN(ToolBugReporting::startCrashReporter());
		}

		SetupSharedCompression::install();

		//-- file
		SetupSharedFile::install(false);

		//-- Regex
		SetupSharedRegex::install();

		//-- utility
		SetupSharedUtility::Data setupUtilityData;
		SetupSharedUtility::setupGameData (setupUtilityData);
		SetupSharedUtility::install (setupUtilityData);

		//-- object
		SetupSharedObject::Data setupObjectData;
		SetupSharedObject::setupDefaultGameData(setupObjectData);
		// we want the SlotIdManager initialized, and we need the associated hardpoint names loaded.
		SetupSharedObject::addSlotIdManagerData(setupObjectData, true);
		// we want CustomizationData support on the client.
		SetupSharedObject::addCustomizationSupportData(setupObjectData);
		SetupSharedObject::install(setupObjectData);

		//-- math
		SetupSharedMath::install();

		//-- random
		SetupSharedRandom::install(static_cast<uint32>(time(NULL)));

		//-- image
		SetupSharedImage::Data setupImageData;
		SetupSharedImage::setupDefaultData(setupImageData);
		SetupSharedImage::install(setupImageData);

		//-- network
		SetupSharedNetworkMessages::install();

		//-- SharedXml
		SetupSharedXml::install();

		//-- game
		SetupSharedGame::Data setupSharedGameData;
		setupSharedGameData.setUseGameScheduler (true);
		SetupSharedGame::install (setupSharedGameData);

		//-- audio
		SetupClientAudio::install();

		//-- graphics
		SetupClientGraphics::Data setupGraphicsData;
		SetupClientGraphics::setupDefaultMFCData(setupGraphicsData);
		IGNORE_RETURN(SetupClientGraphics::install(setupGraphicsData));
		Graphics::setTranslatePointFromGameToScreen(translatePointFromGameToScreen);

		//-- directinput
		SetupClientDirectInput::install(GetModuleHandle(NULL), topLevelWindow, DIK_LMENU, Graphics::isWindowed);
		DirectInput::setScreenShotFunction(ScreenShotHelper::screenShot);
		DirectInput::setToggleWindowedModeFunction(Graphics::toggleWindowedMode);
		DirectInput::suspendInput();

		//-- object
		SetupClientObject::Data setupClientObjectData;
		SetupClientObject::setupToolData (setupClientObjectData);
		SetupClientObject::install (setupClientObjectData);

		//-- particles
		SetupClientParticle::install();

		//-- animation and skeletal animation
		SetupClientAnimation::install();

		SetupClientSkeletalAnimation::Data  saData;
		SetupClientSkeletalAnimation::setupToolData(saData);
		SetupClientSkeletalAnimation::install (saData);

		//-- texture renderer
		SetupClientTextureRenderer::install();

		//-- game
		SetupClientGame::Data setupClientData;
		SetupClientGame::setupGameData(setupClientData);
		SetupClientGame::install(setupClientData);

		//-- iowin (must come after SetupClientGame, even though "shared")
		SetupSharedIoWin::install();
	}

	//-- Need DOT3 for age customization
	DEBUG_WARNING(!GraphicsOptionTags::get(TAG(D,O,T,3)), ("GameWidget::GameWidget() - Need DOT3 for age customization."));

	//-- configuration parameters
	wearableDirectory = ConfigFile::getKeyString("NpcEditor", "wearableDirectory", 0, 0);

	//-- override the platform frame rate limiter with our own, if needed
	Clock::noFrameRateLimit();

	CuiManager::setImplementationInstallFunctions(NpcCuiManager::install, NpcCuiManager::remove, NpcCuiManager::update);

	Game::install(Game::A_npcEditor);

	m_npcBackdrop = dynamic_cast<CuiBackdrop *>(CuiMediatorFactory::activate (CuiMediatorTypes::Backdrop));
	
	m_npcViewer = dynamic_cast<NpcCuiViewer *>(CuiMediatorFactory::activate (CuiMediatorTypes::Viewer));
	NOT_NULL(m_npcViewer);

	m_timer = NON_NULL(new QTimer(this, "timer"));

	// set up a timer to issue repaints to run the game loop as fast as possible
	// NOTE: this particular timer must be a zero-length timer to avoid a Qt bug
	IGNORE_RETURN(connect(m_timer, SIGNAL(timeout()), this, SLOT(runGameLoop())));
	IGNORE_RETURN(m_timer->start(0));

	QWidget::setFocusPolicy(QWidget::StrongFocus);

	Graphics::setHardwareMouseCursorEnabled(false);

	releaseExclusiveFocus(true);
}

// ----------------------------------------------------------------------------

GameWidget::~GameWidget()
{
	delete m_timer;
	m_timer = 0;

	ms_globalGameWidget = NULL;

	m_npcViewer = 0;
	m_npcBackdrop = 0;

	SetupSharedFoundation::remove();
	SetupSharedThread::remove();
}

// ----------------------------------------------------------------------------

void GameWidget::setModel(const char * templateName)
{
	m_npcViewer->loadModel(templateName);

	m_npcViewer->updateCurrentWearables();

	setDirty();
}

// ----------------------------------------------------------------------------

void GameWidget::setAppearance(const char * filename)
{
	m_npcViewer->loadSAT(filename);

	m_npcViewer->updateCurrentWearables();

	setDirty();
}

// ----------------------------------------------------------------------------

void GameWidget::setAvatar(const char * filename)
{
	setModel("object/base_model.iff");
	setAppearance(filename);
}

// ----------------------------------------------------------------------------

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

	Game::runGameLoopOnce(true, static_cast<HWND>(winId()), width(), height());

	if(m_npcViewer->isDirty())
	{
		m_npcViewer->setDirty(false);
		emit viewerDirty();
	}
	
	Sleep(0);

	processingThisFunction = false;
}

// ----------------------------------------------------------------------------

void GameWidget::keyPressEvent(QKeyEvent*keyEvent)
{
	keyEvent->accept();
}

// ----------------------------------------------------------------------------

void GameWidget::keyReleaseEvent(QKeyEvent*keyEvent)
{
	keyEvent->accept();
}

// ----------------------------------------------------------------------------

void GameWidget::translatePointFromGameToScreen(int &ix, int &iy)
{
	if (ms_globalGameWidget)
	{
		QPoint p(ix, iy);
		QPoint q = ms_globalGameWidget->mapToGlobal(p);
		ix = q.x();
		iy = q.y();
	}
}

// ----------------------------------------------------------------------------

void GameWidget::mouseMoveEvent(QMouseEvent*mouseEvent)
{
	mouseEvent->accept();

	DEBUG_REPORT_LOG(false, ("%d,%d\n", mouseEvent->x(), mouseEvent->y()) );

	if(!m_gameHasFocus)
		return;

	const int ix = mouseEvent->x();
	const int iy = mouseEvent->y();
	IoWinManager::queueSetSystemMouseCursorPosition(ix, iy);

	return;
}

// ----------------------------------------------------------------------------

void GameWidget::wheelEvent(QWheelEvent* evt)
{
	int delta = evt->delta();

	DEBUG_REPORT_LOG(false, ("GameWidget::wheelEvent() - delta = %d\n", delta));

	float factor = m_npcViewer->getFitDistanceFactor() - delta * 0.005f;

	m_npcViewer->setFitDistanceFactor(factor);

	DEBUG_REPORT_LOG(false, ("GameWidget::wheelEvent() - factor = %f\n", factor));

	evt->accept();
}
	
// ----------------------------------------------------------------------------

void GameWidget::enterEvent(QEvent * e)
{
	UNREF(e);

	DEBUG_REPORT_LOG(false, ("GameWidget::enterEvent()\n"));

	onGameFocusAllowedChanged(true);
}

// ----------------------------------------------------------------------------

void GameWidget::leaveEvent(QEvent * e)
{
	UNREF(e);

	DEBUG_REPORT_LOG(false, ("GameWidget::leaveEvent()\n"));

	onGameFocusAllowedChanged(false);
}

// ----------------------------------------------------------------------------

void GameWidget::mousePressEvent(QMouseEvent * mouseEvent)
{
	mouseEvent->accept();

	DEBUG_REPORT_LOG(false, ("GameWidget::mousePressEvent()\n"));

	setDirty();

	Graphics::setAllowMouseCursorConstrained(true);
}

// ----------------------------------------------------------------------------

void GameWidget::mouseReleaseEvent(QMouseEvent  * mouseEvent)
{			
	mouseEvent->accept();
		
	DEBUG_REPORT_LOG(false, ("GameWidget::mouseReleaseEvent()\n"));

	setDirty();

	Graphics::setAllowMouseCursorConstrained(false);
}

// ----------------------------------------------------------------------------

void GameWidget::mouseDoubleClickEvent(QMouseEvent * mouseEvent)
{
	mouseEvent->accept();
}

// ----------------------------------------------------------------------------

void GameWidget::grabExclusiveFocus()
{
	if(!m_gameHasFocus)
	{
		m_gameHasFocus = true;

		setCursor(m_blankCursor);
		UIManager::gUIManager().DrawCursor(true);

		DirectInput::resumeInput();

		emit gameExclusiveFocusChanged(false);
	}
}

// ----------------------------------------------------------------------------

void GameWidget::releaseExclusiveFocus(bool force)
{
	if(m_gameHasFocus || force)
	{
		m_gameHasFocus = false;

		setCursor(m_defaultCursor);
		UIManager::gUIManager().DrawCursor(false);

		DirectInput::suspendInput();

		emit gameExclusiveFocusChanged(true);
	}
}

// ----------------------------------------------------------------------------

void GameWidget::onGameFocusAllowedChanged(bool b)
{
	if(m_gameHasFocus && !b)
		releaseExclusiveFocus();
	else if(!m_gameHasFocus && b)
		grabExclusiveFocus();
}

// ----------------------------------------------------------------------------

void GameWidget::resizeEvent(QResizeEvent *resizeData)
{
	if (!resizeData)
		return;

	//-- retrieve new widget size
	const QSize &newWindowSize = resizeData->size();
	const int    newWidth      = newWindowSize.width();
	const int    newHeight     = newWindowSize.height();

	if (newHeight == 0)
		return;

	//-- set UI window size.
	CuiManager::setSize(newWidth, newHeight);
}

// ----------------------------------------------------------------------------

void GameWidget::focusInEvent( QFocusEvent* )
{
}

// ----------------------------------------------------------------------------

void GameWidget::setFrameRateLimit(int fps)
{
	if(fps)
		m_minFrameLength = std::max(1, 1000 / fps);
}

// ----------------------------------------------------------------------------

int GameWidget::getFrameRateLimit() const
{
	if(m_minFrameLength)
		return 1000 / m_minFrameLength;
	else
		return 0;
}

// ----------------------------------------------------------------------------

void GameWidget::callbackRandomize(const std::string &fullVariablePathName, CustomizationVariable *cv, void *context)
{
	int minRange, maxRange, delta;

	NOT_NULL(cv);
	UNREF(context);
	UNREF(fullVariablePathName);
	
	//-- ranged int variables
	BasicRangedIntCustomizationVariable * ri = dynamic_cast<BasicRangedIntCustomizationVariable *>(cv);
	if(ri)
	{
		ri->getRange(minRange, maxRange);
		delta = abs(minRange - maxRange);

		ri->setValue(minRange + (rand() % delta));
		return;
	}

	PaletteColorCustomizationVariable * pv = dynamic_cast<PaletteColorCustomizationVariable *>(cv);
	if(pv)
	{
		pv->getRange(minRange, maxRange);
		delta = abs(minRange - maxRange);

		pv->setValue(minRange + (rand() % delta));
		return;
	}

	return;
}

// ----------------------------------------------------------------------------

void GameWidget::testCustomization()
{
	CreatureObject * creature = m_npcViewer->getCreature();
	if(!creature) return;

	CustomizationData * cdata = getAvatarCustomizationData();
	if(!cdata) return;

	//-- randomize the scale/height
	{
		ObjectTemplate const * const tmp = creature->getObjectTemplate();

		if(tmp)
		{
			SharedObjectTemplate const * const tmpl = dynamic_cast<SharedObjectTemplate const *>(tmp);

			if(tmpl)
			{
				float const scaleMax   = tmpl->getScaleMax ();
				float const scaleMin   = tmpl->getScaleMin ();
				float const scaleRange = scaleMax - scaleMin;
				float const normalizedValue = static_cast<float>(rand() % 100) / 100.0f;
				float const denormalized = scaleMin + (normalizedValue * scaleRange);

				UNREF(denormalized);
				/* Do not randomize the scale. This values is not part of custom variables.
				creature->setScaleFactor (denormalized);
				*/
			}
		}
	}

	//-- randomize everything ...
	cdata->iterateOverVariables(callbackRandomize, 0);

	Appearance * const app = creature->getAppearance ();
	if (app)
	{
		SkeletalAppearance2 * const skelApp = app->asSkeletalAppearance2 ();
		if (skelApp)
		{
			for(int i = 0; i < skelApp->getWearableCount(); ++i)
			{
				SkeletalAppearance2 * s = skelApp->getWearableAppearance(i);
				if(!s) continue;

				CustomizationData * cdata2 = s->fetchCustomizationData();
				if(!cdata2) continue;

				cdata2->iterateOverVariables(callbackRandomize, 0);

				cdata2->release();

				Object * o = s->getOwner();
				if(!o) continue;

				const SkeletalAppearanceTemplate * a = dynamic_cast<const SkeletalAppearanceTemplate *>(s->getAppearanceTemplate());
				if(!a) continue;

				for(int j=0; j < a->getMeshGeneratorCount(); ++j)
				{
					DEBUG_REPORT_LOG(false, ("MeshGen = %s\n", a->getMeshGeneratorName(j).getString()));
				}
			}
		}
	}

#ifdef _DEBUG
	cdata->debugDump();
#endif

	cdata->release();
}

// ----------------------------------------------------------------------------

void setCustomVariable(CustomizationData *wearableCustomizationData, std::string const &variableName, int newValue)
{
	//-- get the CustomizationVariable associated with the specified variable name.
	RangedIntCustomizationVariable *const variable = dynamic_cast<RangedIntCustomizationVariable*>(wearableCustomizationData->findVariable(variableName));
	if (variable)
	{
#ifdef _DEBUG
		int rangeMinInclusive;
		int rangeMaxExclusive;
		variable->getRange(rangeMinInclusive, rangeMaxExclusive);
		DEBUG_WARNING((newValue < rangeMinInclusive) || (newValue >= rangeMaxExclusive),
			("client-baked wearable: variable [%s] applied to object template [_s] has out of range value: min/max/value=%d/%d/%d\n",
			variableName.c_str(), rangeMinInclusive, rangeMaxExclusive - 1, newValue));
#endif

		variable->setValue(newValue);
	}
	else
	{
		DEBUG_WARNING(true,
			("client-baked wearable: variable [%s] is applied to object template [_s] but that variable does not exist.\n",
			variableName.c_str()));
	}
}

// ----------------------------------------------------------------------------

void GameWidget::testClear()
{
	CreatureObject * creature = m_npcViewer->getCreature();

	Appearance * const app = creature->getAppearance ();
	if (app)
	{
		SkeletalAppearance2 * const skelApp = app->asSkeletalAppearance2 ();
		if (skelApp)
		{
			int i = skelApp->getWearableCount() - 1;

			if(i >= 0)
			{
				SkeletalAppearance2 * s = skelApp->getWearableAppearance(i);

				Object * o = s->getOwner();

				skelApp->unlockWearables();
				skelApp->stopWearing(o);
				skelApp->lockWearables();

				delete o;
			}
		}
	}
}

// ----------------------------------------------------------------------------

void GameWidget::fillVariableComboBox(QComboBox* comboBoxVariables)
{
	comboBoxVariables->clear();

	CustomizationData * cdata = getAvatarCustomizationData();
	if(!cdata) return;

	cdata->iterateOverVariables(callbackFillComboBox, comboBoxVariables);

	cdata->release();
}

// ----------------------------------------------------------------------------

void GameWidget::callbackFillComboBox(const std::string &fullVariablePathName, CustomizationVariable *cv, void *context)
{
	NOT_NULL(context);
	NOT_NULL(cv);

	QComboBox *comboBoxVariables = static_cast<QComboBox*>(context);

	//-- ranged int variables
	BasicRangedIntCustomizationVariable * ri = dynamic_cast<BasicRangedIntCustomizationVariable *>(cv);
	if(ri)
	{
		comboBoxVariables->insertItem(fullVariablePathName.c_str());
		return;
	}

	PaletteColorCustomizationVariable * pv = dynamic_cast<PaletteColorCustomizationVariable *>(cv);
	if(pv)
	{
		comboBoxVariables->insertItem(fullVariablePathName.c_str());
		return;
	}

	return;
}


// ----------------------------------------------------------------------------

void GameWidget::getIntRange(const char *currentText, int &minValue, int &maxValue)
{
	minValue = 0;
	maxValue = 0;

	CustomizationData * cdata = getAvatarCustomizationData();
	if(!cdata) return;

	CustomizationVariable * cv = cdata->findVariable(currentText);
	if(!cv) return;

	BasicRangedIntCustomizationVariable * ri = dynamic_cast<BasicRangedIntCustomizationVariable *>(cv);
	if(ri)
	{
		ri->getRange(minValue, maxValue);
		cdata->release();
		return;
	}

	PaletteColorCustomizationVariable * pv = dynamic_cast<PaletteColorCustomizationVariable *>(cv);
	if(pv)
	{
		pv->getRange(minValue, maxValue);
		cdata->release();
		return;
	}

	cdata->release();
	return;
}

// ----------------------------------------------------------------------------

int GameWidget::getIntValue(const char *currentText)
{
	int retVal = 0;

	CustomizationData * cdata = getAvatarCustomizationData();
	if(!cdata) return retVal;

	CustomizationVariable * cv = cdata->findVariable(currentText);
	if(!cv) return retVal;

	BasicRangedIntCustomizationVariable * ri = dynamic_cast<BasicRangedIntCustomizationVariable *>(cv);
	if(ri)
	{
		retVal = ri->getValue();
		cdata->release();
		return retVal;
	}

	PaletteColorCustomizationVariable * pv = dynamic_cast<PaletteColorCustomizationVariable *>(cv);
	if(pv)
	{
		retVal = pv->getValue();
		cdata->release();
		return retVal;
	}

	cdata->release();
	return retVal;
}

// ----------------------------------------------------------------------------

void GameWidget::setIntValue(const char *currentText, int val)
{
	CustomizationData * cdata = getAvatarCustomizationData();
	if(!cdata) return;

	CustomizationVariable * cv = cdata->findVariable(currentText);
	if(!cv)
	{
		cdata->release();
		return;
	}

	RangedIntCustomizationVariable * ri = dynamic_cast<RangedIntCustomizationVariable *>(cv);
	if(ri)
		ri->setValue(val);

	cdata->release();
	return;
}

// ----------------------------------------------------------------------------

float GameWidget::getHeight()
{
	CreatureObject * creature = m_npcViewer->getCreature();
	if(!creature) return 0.f;

	ObjectTemplate const * const tmp = creature->getObjectTemplate();
	if(!tmp) return 0.f;

	SharedObjectTemplate const * const tmpl = dynamic_cast<SharedObjectTemplate const *>(tmp);
	if(!tmpl) return 0.f;

	float const scaleMax   = tmpl->getScaleMax ();
	float const scaleMin   = tmpl->getScaleMin ();
	float const scaleFact  = creature->getScaleFactor ();
	float const scaleRange = scaleMax - scaleMin;
	float const scaleRet = scaleRange <= 0 ? 0 : (scaleFact - scaleMin) / scaleRange;

	return scaleRet;
}

// ----------------------------------------------------------------------------

void GameWidget::setHeight(float h)
{
	CreatureObject * creature = m_npcViewer->getCreature();
	if(!creature) return;

	ObjectTemplate const * const tmp = creature->getObjectTemplate();
	if(!tmp) return;

	SharedObjectTemplate const * const tmpl = dynamic_cast<SharedObjectTemplate const *>(tmp);
	if(!tmpl) return;

	float const scaleMax   = tmpl->getScaleMax ();
	float const scaleMin   = tmpl->getScaleMin ();
	float const scaleRange = scaleMax - scaleMin;
	float const denormalized = scaleMin + (h * scaleRange);

	creature->setScaleFactor (denormalized);
}

// ----------------------------------------------------------------------------

void GameWidget::wear(const char * mgName)
{
	const char * relativeName = strstr(mgName, "appearance");
	NOT_NULL(relativeName);

	m_npcViewer->wear(relativeName);

	m_npcViewer->updateCurrentWearables();

	setDirty();
}

// ----------------------------------------------------------------------------

void GameWidget::addFilteredWearables(const char * filter, QComboBox * page)
{
	const unsigned maxCount = 8*3;
	std::string filterString;
	char numberString[8];

	if(filter[0] == 0)
		return;

	//-- get ready for a potentially long operation ...
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	filterString = filter;
	IGNORE_RETURN(filterString.append(".lmg"));

	QDir dir(wearableDirectory.c_str(), filterString.c_str());

	m_npcViewer->clearWearables();

	unsigned count = dir.count();
	unsigned start = 0;

	if(count > maxCount)
	{
		unsigned selectedPage = 0;
		unsigned numPages = count / maxCount + 1;

		if(page->isEnabled() && numPages == static_cast<unsigned>(page->count()))
		{
			selectedPage = static_cast<unsigned>(page->currentItem());
		}
		else
		{
			page->clear();
			page->setEnabled(true);

			for(unsigned i=0; i<numPages; ++i)
			{
				sprintf(numberString, "%d", i);
				page->insertItem(numberString);
			}
		}

		start = selectedPage * maxCount;
		count = count - start >= maxCount ? maxCount : count - start;
	}
	else
	{
		// clear page control
		page->clear();
		page->setEnabled(false);
	}

	DEBUG_REPORT_LOG(false, ("GameWidget::addFilteredWearables() - start = %d, count = %d\n", start, count));

	for ( unsigned i = start; i < start + count; ++i )
	{
		int n = static_cast<int>(i);

		DEBUG_REPORT_LOG(false, ( "%s\n", dir[n].ascii() ));
		std::string wearableName = "appearance/mesh/";
		IGNORE_RETURN(wearableName.append(dir[n].ascii()));

		m_npcViewer->addWearable(wearableName.c_str());
	}

	m_npcViewer->doneAddingWearables();

    QApplication::restoreOverrideCursor();
}

// ----------------------------------------------------------------------------

void GameWidget::fillWearableVariableComboBox(QComboBox* comboBoxVariables)
{
	comboBoxVariables->clear();

	CustomizationData * cdata = getWearableCustomizationData();
	if(!cdata)
		return;

	// Last parameter indicates that this function should only
	//   iterator over local variables (otherwise all the character's
	//     customization variables will show up)
	cdata->iterateOverVariables(callbackFillComboBox, comboBoxVariables, false);

	cdata->release();
}

// ----------------------------------------------------------------------------

void GameWidget::getWearableIntRange(const char *currentText, int &minValue, int &maxValue)
{
	if(currentText == 0 || currentText[0] == 0)
		return;

	CustomizationData * cdata = getWearableCustomizationData();
	if(!cdata)
		return;

	CustomizationVariable * cv = cdata->findVariable(currentText);
	if(!cv)
		return;

	BasicRangedIntCustomizationVariable * ri = dynamic_cast<BasicRangedIntCustomizationVariable *>(cv);
	if(ri)
	{
		ri->getRange(minValue, maxValue);
		cdata->release();
		return;
	}

	PaletteColorCustomizationVariable * pv = dynamic_cast<PaletteColorCustomizationVariable *>(cv);
	if(pv)
	{
		pv->getRange(minValue, maxValue);
		cdata->release();
		return;
	}

	cdata->release();
	return;
}

// ----------------------------------------------------------------------------

CustomizationData * GameWidget::getWearableCustomizationData()
{
	int wearableNum;

	CreatureObject * creature = m_npcViewer->getCreature();
	if(!creature)
		return 0;

	Appearance * const app = creature->getAppearance();
	if(!app)
		return 0;

	SkeletalAppearance2 * const skelApp = app->asSkeletalAppearance2();
	if(!skelApp)
		return 0;

	if(skelApp->getWearableCount() <= 0)
		return 0;

	m_npcViewer->getSelectedWearableNum(wearableNum);
	if(wearableNum < 0)
		return 0;

	SkeletalAppearance2 * s = skelApp->getWearableAppearance(wearableNum);
	if(!s)
		return 0;

	return s->fetchCustomizationData();
}

// ----------------------------------------------------------------------------

CustomizationData * GameWidget::getAvatarCustomizationData()
{
	CreatureObject * creature = m_npcViewer->getCreature();
	if(!creature) return 0;

	return creature->fetchCustomizationData();
}

// ----------------------------------------------------------------------------

int GameWidget::getWearableIntValue(const char *currentText)
{
	int retVal = 0;

	if(currentText == 0 || currentText[0] == 0)
		return 0;

	CustomizationData * cdata = getWearableCustomizationData();
	if(!cdata) return retVal;

	CustomizationVariable * cv = cdata->findVariable(currentText);
	if(!cv) return retVal;

	BasicRangedIntCustomizationVariable * ri = dynamic_cast<BasicRangedIntCustomizationVariable *>(cv);
	if(ri)
	{
		retVal = ri->getValue();
		cdata->release();
		return retVal;
	}

	PaletteColorCustomizationVariable * pv = dynamic_cast<PaletteColorCustomizationVariable *>(cv);
	if(pv)
	{
		retVal = pv->getValue();
		cdata->release();
		return retVal;
	}

	cdata->release();
	return retVal;
}

// ----------------------------------------------------------------------------

void GameWidget::setWearableIntValue(const char *currentText, int val)
{
	if(currentText == 0 || currentText[0] == 0)
		return;

	CustomizationData * cdata = getWearableCustomizationData();
	if(!cdata) return;

	CustomizationVariable * cv = cdata->findVariable(currentText);
	if(!cv) return;

	RangedIntCustomizationVariable * ri = dynamic_cast<RangedIntCustomizationVariable *>(cv);
	if(ri)
		ri->setValue(val);

	cdata->release();
	return;
}

// ----------------------------------------------------------------------------

void GameWidget::removeSelectedWearable()
{
	int wearableNum = -1;

	CreatureObject * creature = m_npcViewer->getCreature();

	m_npcViewer->getSelectedWearableNum(wearableNum);
	if(wearableNum < 0)
		return;

	Appearance * const app = creature->getAppearance ();
	if (app)
	{
		SkeletalAppearance2 * const skelApp = app->asSkeletalAppearance2 ();
		if (skelApp)
		{
			int i = skelApp->getWearableCount() - 1;

			if(i >= wearableNum)
			{
				SkeletalAppearance2 * s = skelApp->getWearableAppearance(wearableNum);

				Object * o = s->getOwner();

				skelApp->unlockWearables();
				skelApp->stopWearing(o);
				skelApp->lockWearables();

				delete o;
			}
		}
	}

	m_npcViewer->updateCurrentWearables();
}

// ----------------------------------------------------------------------------

void GameWidget::setupWearableFilterPresets(QComboBox * presets) const
{
	const char * wearableFilterName = 0;
	const int maxPresets = 64;
	bool done = false;
	int i = 0;

	//-- keep looping until no more names are found
	while(!done && i < maxPresets)
	{
		wearableFilterName = ConfigFile::getKeyString("NpcEditor", "wearableFilterName", i++, 0);

		if(wearableFilterName)
			presets->insertItem(wearableFilterName);
		else
			done = true;
	}

	presets->setEnabled(presets->count() > 0);
}

// ----------------------------------------------------------------------------

const char * GameWidget::getWearableFilterPreset(int i) const
{
	const char * defaultFilterExpr = "Not available";
	const char * wearableFilterExpr = 0;
	
	wearableFilterExpr = ConfigFile::getKeyString("NpcEditor", "wearableFilterExpr", i, 0);
	
	return wearableFilterExpr ? wearableFilterExpr : defaultFilterExpr;
}

// ----------------------------------------------------------------------------

bool GameWidget::isPaletteVariable(const char *currentText, bool wearable)
{
	bool ret = false;
	CustomizationData * cdata = 0;

	if(currentText == 0 || currentText[0] == 0)
		return false;

	if(wearable)
	{
		cdata = getWearableCustomizationData();
		if(!cdata) return false;
	}
	else
	{
		cdata = getAvatarCustomizationData();
		if(!cdata) return false;
	}

	CustomizationVariable * cv = cdata->findVariable(currentText);
	if(!cv)
	{
		cdata->release();
		return false;
	}

	PaletteColorCustomizationVariable * pi = dynamic_cast<PaletteColorCustomizationVariable *>(cv);

	ret = pi != 0;

	cdata->release();

	return ret;
}

// ----------------------------------------------------------------------------

PaletteArgb const * GameWidget::getPalette(const char *currentText, bool wearable)
{
	PaletteArgb const * colorPalette = 0;
	CustomizationData * cdata = 0;

	if(wearable)
	{
		cdata = getWearableCustomizationData();
		if(!cdata) return 0;
	}
	else
	{
		cdata = getAvatarCustomizationData();
		if(!cdata) return 0;

	}

	CustomizationVariable * cv = cdata->findVariable(currentText);
	if(!cv)
	{
		cdata->release();
		return 0;
	}

	PaletteColorCustomizationVariable * pi = dynamic_cast<PaletteColorCustomizationVariable *>(cv);

	if(!pi)
	{
		cdata->release();
		return 0;
	}

	colorPalette = pi->fetchPalette();

	cdata->release();

	return colorPalette;
}

// ----------------------------------------------------------------------------

void GameWidget::setBackdrop(int backdrop)
{
	if(m_npcBackdrop)
		m_npcBackdrop->setBackdrop(backdrop);
}

// ============================================================================
