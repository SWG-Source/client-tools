//--
//-- CuiManager.h - static class serves as manager for SW UI system
//-- jwatson 2001-02-11
//-- copyright 2001 Sony Online Entertainment
//--


//-------------------------------------------------------------------

#ifndef CuiMANAGER_H
#define CuiMANAGER_H

//-----------------------------------------------------------------

class CuiIoWin;
class CuiMediator;
class CuiMessageBox;
class Object;
class StringId;
class UICanvas;
class UIManager;
class UIPage;

//-------------------------------------------------------------------

class CuiManager
{
public:

	typedef void (*ImplementationInstallFunction) (void);
	typedef bool (*ImplementationTestFunction)    (std::string &);
	typedef void (*ImplementationUpdateFunction)  (float);
	
	/**
	* This is the only method in this class which may be called before install ()
	*/
	static void                         setImplementationInstallFunctions (ImplementationInstallFunction  installFunc, ImplementationInstallFunction removeFunc, ImplementationUpdateFunction updateFunc);
	static void                         setImplementationTestFunction     (ImplementationTestFunction     testFunc);

	class InputManager
	{
	private:
		friend class CuiMediator;
		friend class CuiIoWin;
		friend class CuiMessageBox;

		static void  setPointerInputActive         (bool b);
		static void  setKeyboardInputActive        (bool b);
		static void  setInputToggleActive          (bool b);
		static void  setPointerMotionCapturedByUiX (bool b);
		static void  setPointerMotionCapturedByUiY (bool b);
	};

	friend class InputManager;

	// static initializer/destructor
	static void                         install      ();
	static void                         remove       ();

	/**
	* Request that the UI reset as soon as possible.  All assets will be reloaded, and the background will be activated.
	*/
	static void                         requestReset ();

	/**
	* Get the global destination canvas for the UI system.
	*/
	static UICanvas *                   getUICanvas       ();

	/**
	* Pump the UI system.
	*/
	static void                         update           (float elapsedTime);

	/**
	* Raise the CuiIoWin to the top of the IoWinManager stack.
	*/

	static void                         raiseToTop              ();

	/**
	* Get the active state of the pointer input system.
	*/
	static bool                         getPointerInputActive   ();
	static bool                         getKeyboardInputActive  ();
	static bool                         getInputToggleActive    ();

	static void                         render                  ();

	static void                         terminateIoWin          ();

	static real                         getPixelOffset          ();
	static void                         setPixelOffset          (real offset);

	static void                         resetLocalizedStrings   ();

	static void                         restartMusic            (bool onlyIfNotPlaying = false);
	static void                         stopMusic               (float fadeout);
	
	static CuiIoWin &                   getIoWin                ();
	static bool                         getLocalizedString      (const std::string & name, Unicode::String & dest);
	static const Unicode::String        getLocalizedString      (const std::string & name);

	static bool                         getInstalled            ();

	static bool                         getPointerToggledOn     ();
	static void                         setPointerToggledOn     (bool b);
	static void                         requestKeyboard         (bool b);
	static void                         requestPointer          (bool b);
    
	/**
	* It is used by such things as the game's camera controller to determine if mouse motion events
	* should be processed by such game cameras.  The pointer input may be disabled, but pointer motion
	* may not be captured.
	*
	* This is to implement the 'dead zone' for the reticle at the center of the screen.  The UI captures
	* mouse motion events inside the 'dead zone', but otherwise ignores them.
	*
	* This method just returns the value 
	*/
	static bool                         getPointerMotionCapturedByUiX ();
	static bool                         getPointerMotionCapturedByUiY ();

	static float                        getCameraInertia         ();
	static void                         setCameraInertia         (float f);

	static bool                         test                     (std::string & result);

	static void                         setSize                  (int width, int height);

	static void                         generateStringList       ();

	static void                         exitGame                 (bool confirmed);

	static void                         forceRender              ();

	static void                         debugBadStringIdsFunc    (const StringId & id, bool isTableBad);

	static void                         setIgnoreBadStringId     (const StringId & id);

	static void playUiEffect(std::string const & effect, Object * target = NULL);

private:
	static void                         setPointerInputActive    (bool b);
	static void                         setKeyboardInputActive   (bool b);
	static void                         setInputToggleActive     (bool b);
	static void                         setPointerMotionCapturedByUiX (bool b);
	static void                         setPointerMotionCapturedByUiY (bool b);

	static real                         ms_pixelOffset;
	static bool                         ms_installed;
	static bool                         ms_theIoWinClosed;	
	static bool                         ms_resetRequested;
	static const char * const           ms_musicName;
	static CuiIoWin *                   ms_theIoWin;

	static bool                         ms_pointerMotionCapturedByUiX;
	static bool                         ms_pointerMotionCapturedByUiY;

	static float                        ms_cameraInertia;

	static ImplementationInstallFunction  ms_implementationInstallFunction;
	static ImplementationInstallFunction  ms_implementationRemoveFunction;
	static ImplementationUpdateFunction   ms_implementationUpdateFunction;
	static ImplementationTestFunction     ms_implementationTestFunction;
};

//-------------------------------------------------------------------

inline void CuiManager::requestReset (void)
{
	ms_resetRequested = true;
}

//-----------------------------------------------------------------

inline real  CuiManager::getPixelOffset ()
{
	return ms_pixelOffset;
}

//----------------------------------------------------------------

inline void  CuiManager::setPixelOffset (real offset)
{
	ms_pixelOffset = offset;
}

//-----------------------------------------------------------------
/**
 * Determine if the CuiManager is installed.
 *
 * @return true if the CuiManager is installed, false otherwise.
 */

inline bool CuiManager::getInstalled ()
{
	return ms_installed;
}

//-----------------------------------------------------------------

inline bool CuiManager::getPointerMotionCapturedByUiX ()
{
	return ms_pointerMotionCapturedByUiX;
}

//-----------------------------------------------------------------

inline bool CuiManager::getPointerMotionCapturedByUiY ()
{
	return ms_pointerMotionCapturedByUiY;
}

//-----------------------------------------------------------------

#endif
