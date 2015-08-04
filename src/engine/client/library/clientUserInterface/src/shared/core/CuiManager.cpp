//
// CuiManager.cpp
// asommers 1-10-2001
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiManager.h"

#include "LocalizationManager.h"
#include "SetupUi.h"
#include "UIBaseObject.h"
#include "UIClipboard.h"
#include "UIClock.h"
#include "UIEffector.h"
#include "UILowerString.h"
#include "UIManager.h"
#include "UINullIMEManager.h"
#include "UIPage.h"
#include "UIScriptEngine.h"
#include "UIText.h"
#include "UITextStyle.h"
#include "UITextStyleManager.h"
#include "UnicodeUtils.h"
#include "clientAudio/Audio.h"
#include "clientDirectInput/DirectInput.h"
#include "clientGame/ClientRegionManager.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGraphics/Graphics.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiBackdrop.h"
#include "clientUserInterface/CuiChatBubbleManager.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiConsoleHelper.h"
#include "clientUserInterface/CuiDamageManager.h"
#include "clientUserInterface/CuiIMEManager.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiLayerRenderer.h"
#include "clientUserInterface/CuiLayer_CursorInterface.h"
#include "clientUserInterface/CuiLayer_EngineCanvas.h"
#include "clientUserInterface/CuiLayer_Loader.h"
#include "clientUserInterface/CuiLayer_TextureCanvas.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiManagerManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMediatorTypes.h"
#include "clientUserInterface/CuiMessageBox.h" // convert into manager
#include "clientUserInterface/CuiMessageQueueManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "clientUserInterface/CuiStringIdsServer.h"
#include "clientUserInterface/CuiTextManager.h"
#include "clientUserInterface/IMEManager.h"
#include "libEverQuestTCG/libEverQuestTCG.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/ConfigSharedGame.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedNetworkMessages/ErrorMessage.h"
#include "sharedNetworkMessages/NewbieTutorialRequest.h"
#include "sharedNetworkMessages/NewbieTutorialResponse.h"
#include "sharedObject/Object.h"
#include "unicodeArchive/UnicodeArchive.h"

#include <list>

//lint -esym(1774, UIBaseObject)
//lint -esym(534, UIBaseObject::Detach)

//-----------------------------------------------------------------
#define UI_BASE_OBJECT_USE_LEAK_FINDER 0
#if UI_BASE_OBJECT_USE_LEAK_FINDER
#include "sharedDebug/LeakFinder.h"
namespace UIBaseObjectNamespace
{
	extern LeakFinder s_leakFinder;
}
using UIBaseObjectNamespace::s_leakFinder;
#endif
//-----------------------------------------------------------------

namespace CuiManagerNamespace
{
	const std::string cms_newbieTutorialRequestChangeMouseMode ("changeMouseMode");
	const std::string cms_newbieTutorialRequestNpeContinuation ("showNpeUI");

	void changeMouseModeCallback (void* /*context*/)
	{
		const NewbieTutorialResponse response (cms_newbieTutorialRequestChangeMouseMode);
		GameNetwork::send (response, true);
	}

	void onExitGameMessageBoxClosed (const CuiMessageBox & box)
	{
		if (box.completedAffirmative ())
		{
			CuiManager::exitGame (true);
		}
	}

	typedef stdset<StringId>::fwd StringIdSet;
	StringIdSet s_badStringIds;

	StringId s_ignoreBadStringId;

	Unicode::String const s_effectTokenDelimiterString(Unicode::narrowToWide(" ,"));
	Unicode::String const s_effectTokenSeparatorString(Unicode::narrowToWide("=:+"));
	Unicode::String const s_booleanTrue(Unicode::narrowToWide("true"));

	bool getTokenValue(Unicode::UnicodeStringVector const & effectTokens, UIString const & token, UIString & value)
	{
		// Find the token.
		Unicode::UnicodeStringVector::const_iterator itToken;
		for (itToken = effectTokens.begin(); itToken != effectTokens.end(); ++itToken)
		{
			if (_wcsnicmp((*itToken).c_str(), token.c_str(), token.size()) == 0)
			{
				break;
			}
		}

		if (itToken == effectTokens.end() || (itToken + 1) == effectTokens.end() || (itToken + 2) == effectTokens.end())
		{
			return false;
		}

		// Ensure we have a properly formatted string.
		Unicode::String const & assignementString = *(itToken + 1);
		UIString const & assignmentToken = UIManager::gUIManager().getEffectToken(UIManager::EFTKN_Assignment);
		if (_wcsnicmp(assignementString.c_str(), assignmentToken.c_str(), assignmentToken.size()) != 0)
		{
			return false;
		}

		// Retrieve token.
		value = *(itToken + 2);

		// Success!
		return true;
	}

	void globalWarningCallback(char const * const msg)
	{
		UIText * const warningOutputText = safe_cast<UIText *>(UIManager::gUIManager().GetObjectFromPath("/Warnings.text", TUIText));
		if (NULL != warningOutputText)
		{
			warningOutputText->SetVisible(true);
			warningOutputText->AppendLocalText(Unicode::narrowToWide(msg));
			warningOutputText->ScrollToBottom();
			CuiSoundManager::restart(CuiSounds::negative);
		}
	}

#if _DEBUG
	struct LeakedObjectCompare
	{
		bool operator()(const UIBaseObject *const t, const UIBaseObject *const o) const
		{

			int c=strcmp(t->GetTypeName(), o->GetTypeName());
			if (c<0)
			{
				return true;
			}
			else if (c>0)
			{
				return false;
			}
			else
			{
				return o->GetRefCount()<t->GetRefCount();
			}
		}
	};
#endif
}

using namespace CuiManagerNamespace;

//-----------------------------------------------------------------

namespace
{
	UICanvas *                   ms_uiCanvas        = 0;
	UISoundCanvas *              ms_uiSoundCanvas   = 0;
	UIScriptEngine *             ms_uiScriptEngine  = 0;
	UIIMEManager *               ms_uiIMEManager    = 0;
	UICanvasFactory *            ms_uiCanvasFactory = 0;
	UILocalizedStringFactory *   ms_uiStringFactory = 0;
	UICursorInterface *          ms_cursorInterface = 0;

	UICanvas **                  ms_canvasBuffer     = 0;
	int                          ms_canvasBufferSize = 0;

	const float CANVAS_TIMEOUT_SECS                  = 10.0f;
	const int CANVAS_TIMEOUT_FRAMES                  = 300;

	bool                         s_debugReportInstall        = false;
	bool                         s_debugReportInstallVerbose = false;
#if PRODUCTION == 0	
	bool                         s_debugRenderMetrics        = false;
	bool                         s_drawWidgetBorders         = false;
#endif
	bool                         s_textDropShadow            = true;

	namespace UnnamedMessages
	{
		const char * const ConnectionServerConnectionClosed = "ConnectionServerConnectionClosed";
	}
	//----------------------------------------------------------------------
	//----------------------------------------------------------------------
	//-- todo: move to sep file
	//----------------------------------------------------------------------

	class NetworkStatusManager : public MessageDispatch::Receiver
	{
	public:
		NetworkStatusManager ();
		void receiveMessage (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	private:

		CuiMessageBox * m_box;
		NetworkStatusManager & operator= (const NetworkStatusManager & rhs); //lint !e754
		NetworkStatusManager (const NetworkStatusManager & rhs); //lint !e754 //damn gay-ass lint
	};

	//----------------------------------------------------------------------
	NetworkStatusManager::NetworkStatusManager () :
	MessageDispatch::Receiver (),
		m_box (0)
	{
		connectToMessage ("ErrorMessage");
		connectToMessage (NewbieTutorialRequest::cms_name);
	};

	//----------------------------------------------------------------------
	void NetworkStatusManager::receiveMessage (const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message)
	{
		if (message.isType ("ErrorMessage"))
		{
			if (m_box)
				m_box->closeMessageBox ();
			Archive::ReadIterator ri (NON_NULL (safe_cast<const GameNetworkMessage *> (&message))->getByteStream ().begin ());
			ErrorMessage errorMsg (ri);

			m_box = CuiMessageBox::createInfoBox (Unicode::narrowToWide (errorMsg.getErrorName () + " : " + errorMsg.getDescription ()));
			m_box->connectToMessages (*this);
			return;
		}

		//----------------------------------------------------------------------

		if (message.isType (NewbieTutorialRequest::cms_name))
		{
			//-- what type of request is it?
			Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage*> (&message))->getByteStream ().begin ();
			const NewbieTutorialRequest newbieTutorialRequest (ri);

			if (newbieTutorialRequest.getRequest () == cms_newbieTutorialRequestChangeMouseMode)
				CuiManager::getIoWin ().setModeCallback (CuiManagerNamespace::changeMouseModeCallback, this);
			else if(newbieTutorialRequest.getRequest() == cms_newbieTutorialRequestNpeContinuation)
				CuiActionManager::performAction(CuiActions::npeContinuation, Unicode::emptyString);
		}

		//----------------------------------------------------------------------

		const CuiMessageBox::BoxMessage * const abm = dynamic_cast<const CuiMessageBox::BoxMessage *> (&message);

		if (abm && abm->getMessageBox () == m_box)
		{
			if (message.isType (CuiMessageBox::Messages::CLOSED))
				m_box = 0;
		}
	}

	//----------------------------------------------------------------------

	NetworkStatusManager * s_nsm;

	//----------------------------------------------------------------------

	SoundId s_musicId;

	//----------------------------------------------------------------------
}

//----------------------------------------------------------------------
//-----------------------------------------------------------------
//-- CuiManager
//-----------------------------------------------------------------

real                         CuiManager::ms_pixelOffset             = -0.5f;
bool                         CuiManager::ms_installed = false;
bool                         CuiManager::ms_theIoWinClosed = false;
bool                         CuiManager::ms_resetRequested = false;
//TODO remove this, SWG logic in the engine
const char * const           CuiManager::ms_musicName               = "sound/music_main_title.snd";
CuiIoWin *                   CuiManager::ms_theIoWin = NULL;

CuiManager::ImplementationInstallFunction  CuiManager::ms_implementationInstallFunction;
CuiManager::ImplementationInstallFunction  CuiManager::ms_implementationRemoveFunction;
CuiManager::ImplementationTestFunction     CuiManager::ms_implementationTestFunction;
CuiManager::ImplementationUpdateFunction   CuiManager::ms_implementationUpdateFunction;

bool                         CuiManager::ms_pointerMotionCapturedByUiX = false;
bool                         CuiManager::ms_pointerMotionCapturedByUiY = false;
float                        CuiManager::ms_cameraInertia = 0.0f;

//-----------------------------------------------------------------------

void CuiManager::install ()
{
	DEBUG_FATAL (ms_installed, ("CuiManager Already installed.\n"));

	InstallTimer const installTimer("CuiManager");

	DebugFlags::registerFlag (s_debugReportInstallVerbose,   "ClientUserInterface", "installVerbose");
	DebugFlags::registerFlag (s_debugReportInstall,          "ClientUserInterface", "install");
#if PRODUCTION == 0	
	DebugFlags::registerFlag (s_debugRenderMetrics,          "ClientUserInterface", "renderMetrics");
	DebugFlags::registerFlag (s_drawWidgetBorders,           "ClientUserInterface", "drawWidgetBorders");
#endif
	DebugFlags::registerFlag (s_textDropShadow,              "ClientUserInterface", "textDropShadow");

	REPORT_LOG_PRINT (s_debugReportInstallVerbose, ("CuiManager::install start\n"));

	PerformanceTimer ptimer;
	ptimer.start ();
	
	//-- install the ui system	
	bool useJapanese          = (strcmp(ConfigSharedGame::getDefaultLocale(),"ja") == 0);
	IMEManager::install(useJapanese);

	SetupUi::Data setupUiData;
	setupUiData.m_reportSetFlagsFunction = Report::setFlags;
	setupUiData.m_reportPrintfFunction   = Report::vprintf;
	setupUiData.m_profilerEnterFunction  = Profiler::enter;
	setupUiData.m_profilerLeaveFunction  = Profiler::leave;
	setupUiData.m_memoryManagerVerifyFunction  = MemoryManager::verify;

	setupUiData.m_memoryBlockManagerDebugDumpOnRemove = ConfigSharedFoundation::getMemoryBlockManagerDebugDumpOnRemove ();
	SetupUi::install (setupUiData);

	//-- The CuiIMEManager needs to be installed after the UI library is installed
	CuiIMEManager::install();

	ConfigClientUserInterface::install ();

	DirectInput::setIgnoreLAlt(ConfigSharedGame::defaultLocaleIgnoresLAlt());

	UIManager * const uiManager = &UIManager::gUIManager ();

	uiManager->SetLocaleByString(ConfigSharedGame::getDefaultLocale());
	if(useJapanese)
	{
		uiManager->setUIIMEManager(CuiIMEManager::GetCuiIMEManager());
	}
	else
	{
		ms_uiIMEManager = new UINullIMEManager();
		uiManager->setUIIMEManager(ms_uiIMEManager);
	}	

	//-- setup the game ui system
	REPORT_LOG_PRINT (s_debugReportInstallVerbose, ("CuiManager::install CuiLayer::install start\n"));

	CuiLayer::install ();

	ms_uiCanvas         = new CuiLayer::EngineCanvas (UISize (0, 0));
	ms_uiCanvas->Attach (0);

	uiManager->SetTooltipDelaySecs (CuiPreferences::getTooltipDelaySecs ());
	uiManager->DrawCursor (false);

	ms_uiSoundCanvas    = new CuiLayer::SoundCanvas;
	uiManager->SetSoundCanvas (ms_uiSoundCanvas);

	ms_uiScriptEngine   = new UIScriptEngine;
	uiManager->SetScriptEngine (ms_uiScriptEngine);

	ms_cursorInterface  = new CuiLayer::CursorInterface;
	uiManager->SetCursorInterface (ms_cursorInterface);

	ms_uiCanvasFactory	= new CuiLayer::TextureCanvasFactory;
	uiManager->AddCanvasFactory (ms_uiCanvasFactory);

	CuiLayer::Loader theLoader;

	REPORT_LOG_PRINT (s_debugReportInstallVerbose, ("CuiManager::install LoadRootPage\n"));

	InstallTimer installTimerLoadRootPage("LoadRootPage");
	UIPage* const rootPage = theLoader.LoadRootPage (ConfigClientUserInterface::getUiRootName ());
	installTimerLoadRootPage.manualExit();

	ptimer.stop ();
	const float ptime_load = ptimer.getElapsedTime ();
	ptimer.start ();

	PerformanceTimer ptimer_secondary;
	ptimer_secondary.start ();

	NOT_NULL (rootPage);

	const UIBaseObject::UIObjectList & olist = rootPage->GetChildrenRef ();

	UIBaseObject * consolePage = 0;
	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		UIBaseObject * const obj = *it;
		if (obj->IsA (TUIWidget))
			static_cast<UIWidget *> (obj)->SetVisible (false);
		if (obj->IsName ("Console"))
			consolePage = obj;
	}

	//-- put the console on top
	if (consolePage)
		IGNORE_RETURN (rootPage->MoveChild (consolePage, UIBaseObject::Top));


	UITextStyleManager::GetInstance()->Initialize(rootPage, theLoader);

	InstallTimer installTimerLink("Link");
	rootPage->Link ();
	installTimerLink.manualExit();

	ptimer_secondary.stop ();
	const float ptime_link = ptimer_secondary.getElapsedTime ();
	ptimer_secondary.start ();

	ms_uiStringFactory	= new CuiLayer::StringFactory;
	uiManager->AddLocalizedStringFactory (ms_uiStringFactory);

	rootPage->SetSize (UISize (Graphics::getCurrentRenderTargetWidth (), Graphics::getCurrentRenderTargetHeight ()));
	rootPage->ForcePackChildren ();

	uiManager->SetRootPage (rootPage);

	//-- initialize warning display
#if PRODUCTION == 0
	bool const displayWarnings = ConfigFile::getKeyBool("ClientUserInterface", "displayWarnings", false);
	if (displayWarnings)
	{
		UIPage * const warningOutputPage = safe_cast<UIPage *>(UIManager::gUIManager().GetObjectFromPath("/Warnings", TUIPage));
		if (NULL != warningOutputPage)
		{
			warningOutputPage->SetVisible(true);
			UIText * const text = safe_cast<UIText *>(warningOutputPage->GetChild("Text"));
			if (NULL != text)
				text->Clear();

			SetWarningCallback(globalWarningCallback);
		}
	}
#endif

	REPORT_LOG_PRINT (s_debugReportInstallVerbose, ("CuiManager::install CuiManagerManager::install\n"));

	CuiManagerManager::install           (*rootPage);

	ptimer_secondary.stop ();
	const float ptime_manager = ptimer_secondary.getElapsedTime ();
	ptimer_secondary.start ();

	REPORT_LOG_PRINT (s_debugReportInstallVerbose, ("CuiManager::install ResetLocalizedStrings\n"));

	InstallTimer installTimerStrings("Strings");
	UIManager::gUIManager ().GetRootPage ()->ResetLocalizedStrings ();
	installTimerStrings.manualExit();

	ptimer_secondary.stop ();
	const float ptime_strings = ptimer_secondary.getElapsedTime ();
	ptimer_secondary.start();

	ms_theIoWin       = new CuiIoWin ();
	ms_theIoWinClosed = false;
	ms_theIoWin->open ();
	//--

	ptimer_secondary.stop ();
	const float ptime_iowin = ptimer_secondary.getElapsedTime ();
	ptimer.stop ();

	ms_installed = true;

	ms_theIoWin->setPointerToggledOn (CuiPreferences::getMouseModeDefault ());

	s_nsm = new NetworkStatusManager ();

	if (ms_implementationInstallFunction)
		 (ms_implementationInstallFunction) ();

	ptimer_secondary.stop ();
	const float ptime_impl = ptimer_secondary.getElapsedTime ();
	ptimer.stop ();

	const float ptime_setup   = ptimer.getElapsedTime ();
	const float ptime_total   = ptime_setup + ptime_load;

	REPORT_LOG_PRINT (s_debugReportInstall, (" :: CuiManager::install completed in %2.5fsec (%2.5f load, %2.5f setup [%2.5f link/%2.5f man/%2.5f strings/%2.5f iowin/%2.5f impl])\n",
		ptime_total, ptime_load, ptime_setup, ptime_link, ptime_manager, ptime_strings, ptime_iowin, ptime_impl));

	ms_cameraInertia = CuiPreferences::getCameraInertia ();

	GetUIOutputStream()->flush ();
}

//-----------------------------------------------------------------

void CuiManager::remove ()
{
	//-- if not installed, don't remove
	if (!ms_installed)
		return;

	REPORT_LOG_PRINT (s_debugReportInstallVerbose, ("CuiManager::remove start\n"));

	if (ms_implementationRemoveFunction)
		 (ms_implementationRemoveFunction) ();

	delete s_nsm;
	s_nsm = 0;

	REPORT_LOG_PRINT (s_debugReportInstallVerbose, ("CuiManager::remove CuiManagerManager::remove start\n"));

	CuiManagerManager::remove();

	delete[] ms_canvasBuffer;
	ms_canvasBuffer = 0;
	ms_canvasBufferSize = 0;

	//-- shutdown the UI
	terminateIoWin ();

	delete ms_theIoWin;
	ms_theIoWin = 0;

	UIClock::ExplicitDestroy ();
	UIClipboard::ExplicitDestroy ();

	ms_uiCanvas->Detach (0);
	ms_uiCanvas = 0;

	UIManager * uiManager = &UIManager::gUIManager ();

	uiManager->SetSoundCanvas (0);
	delete ms_uiSoundCanvas;
	ms_uiSoundCanvas = 0;

	uiManager->SetScriptEngine (0);
	delete ms_uiScriptEngine;
	ms_uiScriptEngine =0;

	uiManager->SetCursorInterface (0);

	uiManager->setUIIMEManager( 0 );
	delete ms_uiIMEManager;
	ms_uiIMEManager = 0;

	// @TODO: safe to delete canvas factory without removing it first?
//	uiManager->RemoveCanvasFactory (uiCanvasFactory);
	delete ms_uiCanvasFactory;
	ms_uiCanvasFactory = 0;

	delete ms_uiStringFactory;
	ms_uiStringFactory = 0;

	delete ms_cursorInterface;
	ms_cursorInterface = 0;

	REPORT_LOG_PRINT (s_debugReportInstallVerbose, ("CuiManager::remove UIManager::ExplicitDestroy start\n"));

	SetWarningCallback(NULL);

	UIManager::ExplicitDestroy ();
	uiManager = 0;

	// hack to get around PS UI leak
	UIClock::ExplicitDestroy ();
	UIManager::ExplicitDestroy ();

	REPORT_LOG_PRINT (s_debugReportInstallVerbose, ("CuiManager::remove CuiLayer::remove start\n"));

	CuiLayer::remove ();

#if _DEBUG

#if UI_BASE_OBJECT_USE_LEAK_FINDER
   REPORT_PRINT(true, ("*************************************************\n"));
   REPORT_PRINT(true, ("UI Object leaks.\n"));
	LeakFinder::LiveObjectList leaks;
	s_leakFinder.getCurrentObjects(leaks);
	if (!leaks.empty())
	{
		LeakFinder::LiveObjectList::iterator li;
		for (li=leaks.begin();li!=leaks.end();++li)
		{
			const UIBaseObject * const obj = (const UIBaseObject *)li->object;
			if (obj->IsA (TUIFontCharacter) || obj->IsA (TUIImageStyle) || obj->IsA (TUIImageFrame))
				continue;

			const int refCount = obj->GetRefCount();
			const char *const typeName = obj->GetTypeName();
			UINarrowString pathName = obj->GetFullPath();

			REPORT_PRINT(true, ("leak: [ref:%03d] [%14s] %s", refCount, typeName, pathName.c_str()));
			li->callStack.debugPrint();
		}
	}
   REPORT_PRINT(true, ("*************************************************\n"));
#else
	REPORT_LOG_PRINT (s_debugReportInstallVerbose, ("CuiManager::remove leakcheck start\n"));

	UIBaseObject::UIObjectVector ov;
	UIBaseObject::GetOutstandingObjects (ov);

	if (!ov.empty ())
	{
		typedef std::map<UIBaseObject *, int, LeakedObjectCompare> LeakBuckets;

		LeakBuckets leakedObjects;
		for (UIBaseObject::UIObjectVector::const_iterator it = ov.begin (); it != ov.end (); ++it)
		{
			UIBaseObject *o=*it;
			leakedObjects[o]++;
		}

		LeakBuckets::iterator loi;
		for (loi=leakedObjects.begin();loi!=leakedObjects.end();++loi)
		{
			const UIBaseObject * const obj = loi->first;
			const int dupCount = loi->second;

			if (obj->IsA (TUIFontCharacter) || obj->IsA (TUIImageStyle) || obj->IsA (TUIImageFrame))
				continue;

			const int refCount = obj->GetRefCount();
			const char *const typeName = obj->GetTypeName();
			UINarrowString pathName = obj->GetFullPath();

			WARNING (true, ("leak: [ref:%03d] [%14s] %s (%d)", refCount, typeName, pathName.c_str(), dupCount));
		}

		DEBUG_WARNING (true, ("You leaked UI objects, genius.\n"));
	}
#endif
#endif

	UIOutputStream::ExplicitDestroy ();
	ms_installed = false;

	//-- remove the ui system
	SetupUi::remove ();
}

//-----------------------------------------------------------------

void CuiManager::update (float elapsedTime)
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

	if (ms_resetRequested)
	{
		ms_resetRequested = false;
		remove ();
		install ();
		return;
	}

	//-- purge any outstanding message boxes
	{
		PROFILER_AUTO_BLOCK_DEFINE ("CuiMessageBox::update");
		CuiMessageBox::update ();
	}

	//-----------------------------------------------------------------
	//-- run any pending actions that the user has pumped into the game's messagequeue

	{
		PROFILER_AUTO_BLOCK_DEFINE ("scanMessageQueue");
		CuiMessageQueueManager::scanMessageQueue ();
	}

	//-----------------------------------------------------------------
	//-- texture canvas current time is used by the texture caanvases to monitor timeout of shaders

	const float curTime  = Game::getElapsedTime ();
	const int   curFrame = Graphics::getFrameNumber ();
	CuiLayer::TextureCanvas::setCurrentTime (curTime, curFrame);

	//----------------------------------------------------------------------

	{
		PROFILER_AUTO_BLOCK_DEFINE ("CuiLoginManager");
		CuiLoginManager::update (elapsedTime);
	}

	{
		PROFILER_AUTO_BLOCK_DEFINE ("UIManager");
		if (ms_theIoWin)
			UIManager::gUIManager ().SendHeartbeats ();
	}

	{
		PROFILER_AUTO_BLOCK_DEFINE ("LibEverQuest");
		if(libEverQuestTCG::isLaunched())
			libEverQuestTCG::update();
	}

	{
		PROFILER_AUTO_BLOCK_DEFINE ("CuiMediator");
		CuiMediator::updateAll (elapsedTime);
	}

	//-----------------------------------------------------------------
	//-- update the chat bubbles

	{
		PROFILER_AUTO_BLOCK_DEFINE ("CuiChatBubbleManager");
		CuiChatBubbleManager::update (elapsedTime);
	}

	//-----------------------------------------------------------------
	//-- update damage number display

	{
		PROFILER_AUTO_BLOCK_DEFINE ("CuiDamageManager");
		CuiDamageManager::update (elapsedTime);
	}

	//-----------------------------------------------------------------
	//-- update combat deferred text display

	{
		PROFILER_AUTO_BLOCK_DEFINE ("CuiCombatManager");
		CuiCombatManager::update (elapsedTime);
	}

	//-----------------------------------------------------------------
	//-- update console helper (increments time for macros)
	{	
		PROFILER_AUTO_BLOCK_DEFINE ("CuiConsoleHelper");
		CuiConsoleHelper::update (elapsedTime);
	}
		

	//----------------------------------------------------------------------
	//-- update all the other things that happen before the next render

	{
		PROFILER_AUTO_BLOCK_DEFINE ("CuiManagerManager");
		CuiManagerManager::update (elapsedTime);
	}

	//-----------------------------------------------------------------
	//-- see if we can cleanup any shaders

	{
		PROFILER_AUTO_BLOCK_DEFINE ("cleanup");

		typedef std::vector<UICanvas *> CanvasVector;
		static CanvasVector canvasVector;

		UIManager::gUIManager ().GetCanvases (canvasVector);

		for (CanvasVector::iterator it = canvasVector.begin (); it != canvasVector.end (); ++it)
		{
			CuiLayer::TextureCanvas * const texcanvas = safe_cast<CuiLayer::TextureCanvas *> (*it);

			if (texcanvas && texcanvas->isInitialized ())
			{
				if (curTime - texcanvas->getLastTimeUsed () > CANVAS_TIMEOUT_SECS)
				{
					if (curFrame - texcanvas->getLastFrameUsed () > CANVAS_TIMEOUT_FRAMES)
						texcanvas->unloadShader ();
				}
			}
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE ("ClientRegionManager");
			ClientRegionManager::update(elapsedTime);
		}

		//-----------------------------------------------------------------
		//-- cleanup any outstanding CuiMediators

		CuiMediator::garbageCollect (false);

		if (ms_implementationUpdateFunction)
			(ms_implementationUpdateFunction) (elapsedTime);
	}
}

//-------------------------------------------------------------------

void CuiManager::render ()
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

#if PRODUCTION == 0
	PerformanceTimer ptimer;
	ptimer.start ();
#endif

	Graphics::setViewport (0, 0, Graphics::getCurrentRenderTargetWidth (), Graphics::getCurrentRenderTargetHeight ());

	//-- ensure that the player mesh is rendered
	bool savedPlayerShowMeshState = false;
	Object * const player = Game::getPlayer ();
	Appearance * const playerAppearance = player ? player->getAppearance() : 0;
	SkeletalAppearance2 * const playerSkeletalAppearance = playerAppearance ? playerAppearance->asSkeletalAppearance2() : 0;
	if (playerSkeletalAppearance)
	{
		savedPlayerShowMeshState = playerSkeletalAppearance->getShowMesh();
		playerSkeletalAppearance->setShowMesh(true);
	}

#if PRODUCTION == 0
	if (s_debugRenderMetrics)
		CuiLayerRenderer::resetMetrics ();
	UIManager::gUIManager().enableDrawWidgetBorders(s_drawWidgetBorders);
#endif

	const GlFillMode oldFillMode = Graphics::getFillMode();
	Graphics::setFillMode (GFM_solid);

	bool wasDropShadowEnabled = UITextStyle::GetGlobalDropShadowEnabled();
	UITextStyle::SetGlobalDropShadowEnabled (s_textDropShadow);

	CuiTextManager::render (*ms_uiCanvas);
	CuiTextManager::resetQueue ();

	const Camera * const camera = Game::getCamera ();
	if (camera)
		CuiChatBubbleManager::render (*ms_uiCanvas, *camera);

	UITextStyle::SetGlobalDropShadowEnabled(wasDropShadowEnabled);

	UIManager::gUIManager ().Render (*ms_uiCanvas);
	CuiLayerRenderer::flushRenderQueue ();

	Graphics::setFillMode (oldFillMode);

#if PRODUCTION == 0
	ptimer.stop ();

	static const int numLastTimes = 100;
	static float lastTimes [numLastTimes] = {0};
	float totalTime = 0.0f;

	if (s_debugRenderMetrics)
	{
		for (int i = 0; i < numLastTimes - 1; ++i)
		{
			lastTimes [i] = lastTimes [i+1];
			totalTime += lastTimes [i];
		}
		
		lastTimes [numLastTimes - 1] = ptimer.getElapsedTime () * 1000.0f;;
		totalTime += lastTimes [numLastTimes - 1];
		totalTime /= numLastTimes;
		
		const CuiLayerRenderer::Metrics & metrics = CuiLayerRenderer::getMetrics ();
		REPORT_LOG_PRINT (true, (
			"UI rendered %4d quads in %3d draw calls, avg %3d quads/call.\n"
			"            %4d lines    %3d                 %3d lines/call.\n"
			"            completed in %5.2f ms (avg %5.2f)\n",
			metrics.quadCount, metrics.quadCallCount, metrics.quadCallCount ? metrics.quadCount / metrics.quadCallCount : 0,
			metrics.lineCount, metrics.lineCallCount, metrics.lineCallCount ? metrics.lineCount / metrics.lineCallCount : 0,
			lastTimes [numLastTimes - 1], totalTime));
	}
#endif

	ClientRegionManager::drawRegions();

	//-- restore the player mesh rendering state
	if (playerSkeletalAppearance)
		playerSkeletalAppearance->setShowMesh(savedPlayerShowMeshState);
}

//-----------------------------------------------------------------

void CuiManager::setPointerInputActive (const bool )
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

	//-- these null checks are necessary because these methods will be called
	//-- during the CuiMaanager::remove () execution

	if (!ms_theIoWin)
		return;

	const bool b = CuiMediator::getCountPointerInputActive () > 0 || ms_theIoWin->getPointerToggledOn () || CuiPreferences::getUseModelessInterface();

	ms_theIoWin->setPointerInputActive (b);
	UIManager::gUIManager ().DrawCursor (ms_theIoWin->getPointerInputActive ());
}

//-----------------------------------------------------------------

void CuiManager::setKeyboardInputActive (const bool )
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

	//-- these null checks are necessary because these methods will be called
	//-- during the CuiMaanager::remove () execution

	if (!ms_theIoWin)
		return;

	ms_theIoWin->setKeyboardInputActive (CuiMediator::getCountKeyboardInputActive () > 0);
}

//-----------------------------------------------------------------

void CuiManager::setInputToggleActive (const bool)
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

	//-- these null checks are necessary because these methods will be called
	//-- during the CuiMaanager::remove () execution

	if (!ms_theIoWin)
		return;

	ms_theIoWin->setInputToggleActive (CuiMediator::getCountInputToggleActive () > 0);
}

//----------------------------------------------------------------------

void CuiManager::setPointerMotionCapturedByUiX (bool b)
{
	ms_pointerMotionCapturedByUiX = b;
}

//----------------------------------------------------------------------

void CuiManager::setPointerMotionCapturedByUiY (bool b)
{
	ms_pointerMotionCapturedByUiY = b;
}

//-----------------------------------------------------------------

bool CuiManager::getPointerInputActive ()
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

	if (CuiMessageBox::getModalMessageBoxCount () > 0)
		return true;

	return ms_theIoWin ? ms_theIoWin->getPointerInputActive () : false;
}

//-----------------------------------------------------------------

bool CuiManager::getKeyboardInputActive ()
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

	if (CuiMessageBox::getModalMessageBoxCount () > 0)
		return true;

	return ms_theIoWin ?ms_theIoWin->getKeyboardInputActive () : false;
}

//-----------------------------------------------------------------

bool CuiManager::getInputToggleActive ()
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

	if (CuiMessageBox::getModalMessageBoxCount () > 0)
		return false;

	return ms_theIoWin ? ms_theIoWin->getInputToggleActive () : false;
}

//-----------------------------------------------------------------

void CuiManager::raiseToTop ()
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

	if (!ms_theIoWin)
		return;

	if (!ms_theIoWinClosed)
		ms_theIoWin->close ();
	else
		ms_theIoWinClosed = false;

	ms_theIoWin->open ();
}

//-----------------------------------------------------------------

void CuiManager::terminateIoWin ()
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

	if (!ms_theIoWin)
		return;

	if (!ms_theIoWinClosed)
	{
		ms_theIoWinClosed = true;

		if (ms_theIoWin)
			ms_theIoWin->close ();

		Audio::stopAllSounds ();
		Audio::stopSound (s_musicId, 0.0f);
	}
}

//----------------------------------------------------------------------

void CuiManager::exitGame                 (bool confirmed)
{
	if (confirmed)
	{
		CuiMediatorFactory::deactivateAll  ();
		terminateIoWin ();
		return;
	}

	CuiMessageBox::createYesNoBox (CuiStringIds::confirm_exit_game.localize (), onExitGameMessageBoxClosed);
}

//-----------------------------------------------------------------

void CuiManager::resetLocalizedStrings ()
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

	LocalizationManager::getManager ().purgeUnusedStringTables ();

	//-- do this to keep the ui string table around long enough to load the ui

	NOT_NULL (UIManager::gUIManager ().GetRootPage ());
	UIManager::gUIManager ().GetRootPage ()->ResetLocalizedStrings ();
}

//-----------------------------------------------------------------

void  CuiManager::restartMusic (bool onlyIfNotPlaying)
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

	if (!onlyIfNotPlaying || !Audio::isSoundPlaying (s_musicId))
	{
		Audio::stopSound (s_musicId, 1.0f);
		s_musicId = Audio::restartSound (ms_musicName);
	}
}

//----------------------------------------------------------------------

void CuiManager::stopMusic (float fadeout)
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));
	Audio::stopSound (s_musicId, fadeout);
}

//-----------------------------------------------------------------

UICanvas * CuiManager::getUICanvas ()
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

	return ms_uiCanvas;
}

//-----------------------------------------------------------------

CuiIoWin & CuiManager::getIoWin ()
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

	return *ms_theIoWin;
}

//-----------------------------------------------------------------

bool CuiManager::getLocalizedString (const Unicode::NarrowString & name, Unicode::String & dest)
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

	if (name.empty ())
		return false;

	if (name [0] == '@')
		return UIManager::gUIManager ().CreateLocalizedString (Unicode::narrowToWide (name), dest);
	else
		return ms_uiStringFactory->GetLocalizedString (name, dest);
}

//----------------------------------------------------------------------

const Unicode::String CuiManager::getLocalizedString (const Unicode::NarrowString & name)
{
	Unicode::String result;
	IGNORE_RETURN (getLocalizedString (name, result));
	return result;
}

//-----------------------------------------------------------------
// this function was causing a crash in release mode when it was inlined, so it was moved here

void	CuiManager::InputManager::setKeyboardInputActive (bool b)
{
	CuiManager::setKeyboardInputActive (b);
}

//-----------------------------------------------------------------
// this function was causing a crash in release mode when it was inlined, so it was moved here

void	CuiManager::InputManager::setPointerInputActive (bool b)
{
	CuiManager::setPointerInputActive (b);
}

//-----------------------------------------------------------------
// this function was causing a crash in release mode when it was inlined, so it was moved here

void	CuiManager::InputManager::setInputToggleActive (bool b)
{
	CuiManager::setInputToggleActive (b);
}

//-----------------------------------------------------------------

void  CuiManager::InputManager::setPointerMotionCapturedByUiX (bool b)
{
	CuiManager::setPointerMotionCapturedByUiX (b);
}

//-----------------------------------------------------------------

void  CuiManager::InputManager::setPointerMotionCapturedByUiY (bool b)
{
	CuiManager::setPointerMotionCapturedByUiY (b);
}

//-----------------------------------------------------------------

void CuiManager::setImplementationInstallFunctions (ImplementationInstallFunction installFunc, ImplementationInstallFunction removeFunc, ImplementationUpdateFunction updateFunc)
{
	ms_implementationInstallFunction = installFunc;
	ms_implementationRemoveFunction  = removeFunc;
	ms_implementationUpdateFunction  = updateFunc;
}

//-----------------------------------------------------------------

void CuiManager::setImplementationTestFunction     (ImplementationTestFunction testFunc)
{
	ms_implementationTestFunction = testFunc;
}


//----------------------------------------------------------------------

bool CuiManager::getPointerToggledOn ()
{
	NOT_NULL (ms_theIoWin);
	return ms_theIoWin->getPointerToggledOn ();
}

//----------------------------------------------------------------------

void CuiManager::setPointerToggledOn (bool b)
{
	NOT_NULL (ms_theIoWin);
	ms_theIoWin->setPointerToggledOn (b);
}

//----------------------------------------------------------------------

void CuiManager::requestPointer (bool b)
{
	NOT_NULL (ms_theIoWin);
	ms_theIoWin->requestPointer (b);
}

//----------------------------------------------------------------------

void CuiManager::requestKeyboard (bool b)
{
	NOT_NULL (ms_theIoWin);
	ms_theIoWin->requestKeyboard (b);
}

//-----------------------------------------------------------------

void CuiManager::setCameraInertia (float f)
{
	if (ms_cameraInertia != f)
	{
		ms_cameraInertia = f;
		if (ms_theIoWin)
			ms_theIoWin->flashDeadZoneInertia ();
	}
}

//----------------------------------------------------------------------

/**
* Return false if test was successful, true otherwise
*/

bool CuiManager::test (std::string & result)
{
	UNREF (result);

	bool error = false;

	if (ms_installed)
		remove ();

	install ();

	update (100.0f);
	render ();

	setPointerInputActive         (true);
	setKeyboardInputActive        (true);
	setInputToggleActive          (true);
	setPointerMotionCapturedByUiX (true);
	setPointerMotionCapturedByUiY (true);

	update (100.0f);
	render ();

	setPointerInputActive         (false);
	setKeyboardInputActive        (false);
	setInputToggleActive          (false);
	setPointerMotionCapturedByUiX (false);
	setPointerMotionCapturedByUiY (false);

	requestReset ();

	update (100.0f);
	render ();

	resetLocalizedStrings ();
	restartMusic ();

	if (CuiMediatorFactory::test (result))
		return error;

	update (100.0f);
	render ();

	CuiMediatorFactory::deactivateAll ();

	std::vector<std::string> names;
	CuiMediatorFactory::getMediatorNames (names);
	for (std::vector<std::string>::const_iterator it = names.begin (); it != names.end (); ++it)
	{
		CuiMediator * const mediator = CuiMediatorFactory::get ((*it).c_str ());
		NOT_NULL (mediator);
		mediator->activate ();
		update (100.0f);
		render ();
		mediator->deactivate ();
	}

	if (ms_implementationTestFunction && ((ms_implementationTestFunction) (result)))
	{
		result += "Implementation test function failed\n";
		return true;
	}

	remove ();

	return error;
}

//----------------------------------------------------------------------

void CuiManager::generateStringList ()
{
	CuiLayer::StringFactory::generateStringList ();
	install ();

	typedef CuiLayer::StringFactory::StringIdVector StringIdVector;

	const StringIdVector & siv = CuiLayer::StringFactory::getStringList ();

	typedef stdmap<StringId, Unicode::String>::fwd StringIdStatusMap;
	StringIdStatusMap sism;

	static const Unicode::String empty;

	{
		Unicode::String str;
		for (StringIdVector::const_iterator it = siv.begin (); it != siv.end (); ++it)
		{
			const StringId & id = *it;
			if (id.localize (str))
				sism.insert (std::make_pair (id, str));
			else
				sism.insert (std::make_pair (id, empty));
		}
	}

	Unicode::String result;

	for (StringIdStatusMap::const_iterator it = sism.begin (); it != sism.end (); ++it)
	{
		const StringId & id         = (*it).first;
		const Unicode::String str   = (*it).second;

		result.clear ();

		if (str.empty ())
		{
			result.append (1, '*');
			result.append (2, ' ');
		}
		else
			result.append (3, ' ');

		Unicode::appendStringField (result, id.getTable (), 16);

		result.append (3, ' ');

		Unicode::appendStringField (result, id.getText (),  32);

		Unicode::String firstLine;
		size_t endpos = 0;

		if (!Unicode::getFirstToken (str, 0, endpos, firstLine, Unicode::endlines))
			firstLine = str;

		if (firstLine.size () > 30)
			firstLine.erase (size_t(30));

		result.append (firstLine);
		if (firstLine.size () != str.size ())
			result.append (3, '.');

		result.append (1, '\n');

		REPORT_LOG_PRINT (true, ("%s", Unicode::wideToNarrow (result).c_str ()));
	}

	remove ();
}

//-----------------------------------------------------------------

void CuiManager::setSize (int width, int height)
{
	UIPage * const rootPage = UIManager::gUIManager ().GetRootPage ();

	NOT_NULL (rootPage);

	rootPage->SetSize (UISize (width, height));
	rootPage->ForcePackChildren ();

	if (ms_theIoWin)
		ms_theIoWin->resetDeadZone ();
}

//----------------------------------------------------------------------

void CuiManager::forceRender ()
{
	Graphics::beginScene ();
	CuiManager::render ();
	Graphics::endScene ();
	Graphics::present();
}

//----------------------------------------------------------------------

float CuiManager::getCameraInertia ()
{
	if (CuiManager::getPointerInputActive ())
		return ms_cameraInertia * 2.0f;

	//-- don't put inertia on a non-existant deadzone
	if (CuiIoWin::getDeadZoneSize () == 0)
		return 0.0f;

	//-- if view is first person and pointer input is not active, no inertia
	if (Game::isViewFirstPerson ())
		return CuiPreferences::getFirstPersonCameraInertia ();

	return ms_cameraInertia;
}

//----------------------------------------------------------------------

void CuiManager::setIgnoreBadStringId     (const StringId & id)
{
	s_ignoreBadStringId = id;
}

//----------------------------------------------------------------------

void CuiManager::debugBadStringIdsFunc    (const StringId & id, bool isTableBad)
{
	if (id == s_ignoreBadStringId)
		return;

	const std::pair<StringIdSet::iterator, bool> retval = s_badStringIds.insert (id);

	if (retval.second)
	{
		if (isTableBad)
			WARNING (true, ("Invalid StringId TABLE [%s]", id.getCanonicalRepresentation ().c_str ()));
		else
			WARNING (true, ("Invalid StringId NAME  [%s]", id.getCanonicalRepresentation ().c_str ()));
	}
}

//----------------------------------------------------------------------

void CuiManager::playUiEffect(std::string const & effect, Object * /*target*/)
{
	//-- Get a wide string.
	UIString effectString(Unicode::narrowToWide(effect));

	//-- Get them tokens!
	Unicode::UnicodeStringVector effectTokens;
	Unicode::tokenize(effectString, effectTokens, &s_effectTokenDelimiterString, &s_effectTokenSeparatorString);

	//-- Need some tokens to continue.
	if (effectTokens.empty())
	{
		WARNING(true, ("playUiEffect - empty effect string."));
		return;
	}

	//-- Get the target page & root page.
	UIPage * targetPage = NON_NULL(UIManager::gUIManager().GetRootPage());

	UIString showMediator;
	UIString hideMediator;

	if (getTokenValue(effectTokens, UIManager::gUIManager().getEffectToken(UIManager::EFTKN_ShowMediator), showMediator))
	{
		CuiMediatorFactory::activateInWorkspace(Unicode::wideToNarrow(showMediator).c_str());
	}
	else if (getTokenValue(effectTokens, UIManager::gUIManager().getEffectToken(UIManager::EFTKN_HideMediator), hideMediator))
	{
		CuiMediatorFactory::deactivateInWorkspace(Unicode::wideToNarrow(hideMediator).c_str());
	}
	else
	{
		//-- Get the target page.
		UIString targetPageName;
		if(getTokenValue(effectTokens, UIManager::gUIManager().getEffectToken(UIManager::EFTKN_Page), targetPageName))
		{
			// The server script passes down a ui mediator page.
			CuiMediator * const cuiMediator = CuiMediatorFactory::get(Unicode::wideToNarrow(targetPageName).c_str(), false);

			WARNING(cuiMediator == NULL, ("playUiEffect - Mediator (%s) not found.", Unicode::wideToNarrow(targetPageName).c_str()));

			//-- Fallback if the target object does not exist.
			targetPage = cuiMediator ? &(cuiMediator->getPage()) : targetPage;
		}

		//-- Reset.
		UIString resetToken;
		bool resetEffectors = false;
		if(getTokenValue(effectTokens, UIManager::gUIManager().getEffectToken(UIManager::EFTKN_Reset), resetToken))
		{
			resetEffectors = _wcsnicmp(resetToken.c_str(), s_booleanTrue.c_str(), s_booleanTrue.size()) == 0;
		}

		//-- Cancel.
		UIString cancelToken;
		bool cancelEffector = false;
		if(getTokenValue(effectTokens, UIManager::gUIManager().getEffectToken(UIManager::EFTKN_Cancel), cancelToken))
		{
			cancelEffector = _wcsnicmp(cancelToken.c_str(), s_booleanTrue.c_str(), s_booleanTrue.size()) == 0;
		}

		//-- Set active.
		UIString activatePage;
		bool setVisible = true;
		if(getTokenValue(effectTokens, UIManager::gUIManager().getEffectToken(UIManager::EFTKN_Active), activatePage))
		{
			setVisible = _wcsnicmp(activatePage.c_str(), s_booleanTrue.c_str(), s_booleanTrue.size()) == 0;
			targetPage->SetVisible(setVisible);
		}

		//-- Get the effector and set properties on it.
		if (resetEffectors)
		{
			UIManager::gUIManager().CancelEffectorsFor(targetPage, true);
		}
		else
		{
			UIString effectName;
			if(getTokenValue(effectTokens, UIManager::gUIManager().getEffectToken(UIManager::EFTKN_Effect), effectName))
			{
				UIEffector * targetEffector = NULL;
				IGNORE_RETURN(targetPage->GetCodeDataObject(TUIEffector, targetEffector, Unicode::wideToNarrow(effectName).c_str(), true));
				if (targetEffector)
				{
					if (cancelEffector)
					{
						UIManager::gUIManager().CancelEffector(targetEffector, targetPage);
					}
					else
					{
						//-- Set the properties.
						UIBaseObject::UIPropertyNameVector propertyNames;
						targetEffector->GetPropertyNames(propertyNames, false);
						if (!propertyNames.empty())
						{
							for(UIBaseObject::UIPropertyNameVector::const_iterator itProperty = propertyNames.begin(); itProperty != propertyNames.end(); ++itProperty)
							{
								UIString propertyValue;
								if (getTokenValue(effectTokens, UIUnicode::narrowToWide((*itProperty).c_str()), propertyValue))
								{
									targetEffector->SetProperty(*itProperty, propertyValue);
								}
							}
						}
						
						//-- Trigger the effector.
						targetPage->ExecuteEffector(*targetEffector);
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------

