// ======================================================================
//
// DirectInput.cpp
// copyright 2001, Sony Online Entertainment
//
// ======================================================================

#include "clientDirectInput/FirstClientDirectInput.h"
#include "clientDirectInput/DirectInput.h"

#include "clientDirectInput/ConfigClientDirectInput.h"
#include "clientDirectInput/ForceFeedbackEffectTemplate.h"

#include "sharedDebug/DebugKey.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/Production.h"
#include "sharedIoWin/IoWinManager.h"
#include "sharedFile/Iff.h"
#include "sharedUtility/DataTable.h"


#include <cstdio>
#include <map>
#include <vector>

// ensure we create a DirectInput8 device
#undef  DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

// ======================================================================

namespace DirectInputNamespace
{
	Tag const TAG_KBDT = TAG(K,B,D,T);

	class Device
	{
	public:

		virtual ~Device();

		int          getRemainingEventCount();
		uint32       getNextEventSequenceNumber();

		virtual bool update();
		virtual void submitEvent() = 0;
		virtual void unacquire();

		GUID		 getGuid() { return m_GUID; }

	private:

		// disable default constructor, copy constructor, assignment operator
		Device();
		Device(const Device&);
		Device &operator =(const Device&);

	protected:

		// called by derived classes
		explicit Device(const GUID &newDeviceGuid, int newMaxBufferedEventCount);

	protected:

		int                    m_eventCount;
		int                    m_nextEventIndex;
		int                    m_maxBufferedEventCount;
		DIDEVICEOBJECTDATA *   m_eventData;
		IDirectInputDevice8A * m_directInputDevice;
		GUID				   m_GUID;
	};

	// ----------------------------------------------------------------------

	class KeyboardDevice: public Device
	{
	public:

		KeyboardDevice(const GUID &newDeviceGuid, int newMaxBufferedEventCount, DWORD menuKey);
		virtual ~KeyboardDevice();

		virtual bool update();
		virtual void submitEvent();

		void loadTranslationTable();

		bool getFullscreenWindowsKeyEnabled() const;
		void setFullscreenWindowsKeyEnabled(bool enabled);
		bool getWindowedWindowsKeyEnabled() const;
		void setWindowedWindowsKeyEnabled(bool enabled);

		void  discardNextKeyboardInput();
		void  setScreenShotFunction(DirectInput::ScreenShotFunction screenShotFunction);
		void  setToggleWindowedModeFunction(DirectInput::ToggleWindowedModeFunction toggleWindowedModeFunction);
		void  setRequestDebugMenuFunction(DirectInput::RequestDebugMenuFunction requestDebugMenuFunction);

	private:

		// disabled
		KeyboardDevice();
		KeyboardDevice(const KeyboardDevice&);
		KeyboardDevice &operator =(const KeyboardDevice&);

		void setCooperativeLevel();

	private:

		DWORD m_menuKey;
		bool  m_mayHaveMenuTap;
		bool  m_mayHaveAltEnter;
		bool  m_shouldDiscardNextKeyboardInput;
		bool  m_windowsKeyEnabledWhenFullscreen;
		bool  m_windowsKeyEnabledWhenWindowed;

		DirectInput::ScreenShotFunction         m_screenShotFunction;
		DirectInput::ToggleWindowedModeFunction m_toggleWindowedModeFunction;
		DirectInput::RequestDebugMenuFunction   m_requestDebugMenuFunction;

		uint8 m_translationTable[256];
	};

	// ----------------------------------------------------------------------

	class MouseDevice: public Device
	{
	public:

		MouseDevice(const GUID &newDeviceGuid, int newMaxBufferedEventCount);
		virtual ~MouseDevice();
		
		virtual bool		   update();
		virtual void           submitEvent();
		void                   setMouseScale(float xMickeysPerUnit, bool xFlip, float yMickeysPerUnit, bool yFlip, float zMickeysPerUnit, bool zFlip);

	private:

		// disabled
		MouseDevice();
		MouseDevice(const MouseDevice&);
		MouseDevice &operator =(const MouseDevice&);

	private:

		bool  m_leftHanded;

		float m_ooxMickeysPerUnit;
		float m_ooyMickeysPerUnit;
		float m_oozMickeysPerUnit;

		float m_xRemainder;
		float m_yRemainder;
		float m_zRemainder;
	};

	// ----------------------------------------------------------------------

	int const cms_numberOfJoystickAxis = 6;
	int const cms_numberOfJoystickSliders = 2;

	class JoystickDevice: public Device
	{
	public:

		JoystickDevice(const GUID &newDeviceGuid, int newMaxBufferedEventCount, int newJoystickNumber);
		virtual ~JoystickDevice();

		virtual bool           update();
		virtual void           submitEvent();

		int getNumberOfAxis() const;

		void setJoystickTranslationRange(float xCenterVal, float xHalfRange, bool xFlip, float yCenterVal, float yHalfRange, bool yFlip, float zCenterVal, float zHalfRange, bool zFlip);
		void setJoystickTranslationDeadZone(float xPercentAroundCenter, float yPercentAroundCenter, float zPercentAroundCenter);
		void setJoystickTranslationSaturation(float xSaturationPercent, float ySaturationPercent, float zSaturationPercent);

		void setJoystickRotationRange(float xCenterVal, float xHalfRange, bool xFlip, float yCenterVal, float yHalfRange, bool yFlip, float zCenterVal, float zHalfRange, bool zFlip);
		void setJoystickRotationDeadZone(float xPercentAroundCenter, float yPercentAroundCenter, float zPercentAroundCenter);
		void setJoystickRotationSaturation(float xSaturationPercent, float ySaturationPercent, float zSaturationPercent);

		void setJoystickSliderRange(int sliderIndex, float centerVal, float halfRange, bool flip);
		void setJoystickSliderDeadZone(int sliderIndex, float percentAroundCenter);
		void setJoystickSliderSaturation(int sliderIndex, float saturationPercent);
		bool sliderExists(int sliderIndex);

		void loadForceFeedbackEffectIntoTemplate(std::string const & file, ForceFeedbackEffectTemplate * effectTemplate);

		bool createEffect(REFGUID guidEffect, LPCDIEFFECT idEffect, LPDIRECTINPUTEFFECT * const effect, LPUNKNOWN punkOuterlpdife);

		int getJoystickNumber() const { return joystickNumber; }

	private:

		// disabled
		JoystickDevice();
		JoystickDevice(const JoystickDevice&);
		JoystickDevice &operator =(const JoystickDevice&);

		void getRangeData();

	private:

		int   joystickNumber;
		bool  pollIsNeeded;
		bool  axisExists[cms_numberOfJoystickAxis];
		DWORD axisData[cms_numberOfJoystickAxis];

		// translation factors for device coordinate -> user coordinate transformation
		float xDeviceCenter;
		float ooxDeviceHalfRange;
		float yDeviceCenter;
		float ooyDeviceHalfRange;
		float zDeviceCenter;
		float oozDeviceHalfRange;

		float xClientCenter;
		float xClientHalfRange;
		float yClientCenter;
		float yClientHalfRange;
		float zClientCenter;
		float zClientHalfRange;

		float rxDeviceCenter;
		float oorxDeviceHalfRange;
		float ryDeviceCenter;
		float ooryDeviceHalfRange;
		float rzDeviceCenter;
		float oorzDeviceHalfRange;

		float rxClientCenter;
		float rxClientHalfRange;
		float ryClientCenter;
		float ryClientHalfRange;
		float rzClientCenter;
		float rzClientHalfRange;

		struct Slider
		{
			bool exists;
			float deviceCenter;
			float deviceHalfRange;
			float clientCenter;
			float clientHalfRange;
		};

		Slider slider[cms_numberOfJoystickSliders];
	};

	// ----------------------------------------------------------------------

	class InternalForceFeedbackEffect : public DirectInput::ForceFeedbackEffect
	{
	public:
		InternalForceFeedbackEffect(IDirectInputEffect * effect);
		virtual ~InternalForceFeedbackEffect();

		virtual bool play(int const iterations) const;
		virtual bool stop() const;
		virtual bool isPlaying() const;

	private:
		InternalForceFeedbackEffect();

	private:
		IDirectInputEffect * m_effect;
	};

	// ----------------------------------------------------------------------

	void          addDevice(Device *newDevice);
	BOOL CALLBACK enumJoystickDevice(const DIDEVICEINSTANCEA *lpddi, LPVOID pvRef);
	BOOL CALLBACK EnumEffectsInFileIntoTemplateProc(LPCDIFILEEFFECT lpdife, LPVOID pvRef);
	void          installKeyboardDevice(DWORD menuKey);
	void          installMouseDevice();
	void          installJoystickDevices();

	int const   cms_maxBufferedEventCount = 512;
	float const cms_DirectInputAnglesToRadians = 1.74532925199E-4f;
	int const   cms_numberOfJoysticks = 8;
	DWORD const cms_axisLookup[cms_numberOfJoystickAxis] =
	{
		DIJOFS_X,
		DIJOFS_Y,
		DIJOFS_Z,
		DIJOFS_RX,
		DIJOFS_RY,
		DIJOFS_RZ
	};

	typedef std::vector<Device *>     Devices;

	bool                              ms_installed;
	bool                              ms_suspended;
	HWND                              ms_window;
	IDirectInput8 *                   ms_directInput;
	Devices                           ms_devices;
	KeyboardDevice *                  ms_keyboardDevice;
	MouseDevice *                     ms_mouseDevice;
	JoystickDevice *                  ms_joystickDevice[cms_numberOfJoysticks];
	bool                              ms_sendInputReset;
	DirectInput::IsWindowedFunction   ms_isWindowedFunction;
	typedef std::map<uint8, std::string> ScanCodeToKeyNameMap;
	ScanCodeToKeyNameMap s_scanCodeToKeyNameMap;
	ForceFeedbackEffectTemplate * ms_loadingTemplate = NULL;
	int ms_numberOfJoysticksAvailable = 0;

	bool                              ms_ignoreLAlt = false;
}
using namespace DirectInputNamespace;

// ======================================================================
// construct a DirectInput::Device, allocating space for the buffered
// event list

Device::Device(const GUID &newDeviceGuid, int newMaxBufferedEventCount)
:
	m_eventCount(0),
	m_nextEventIndex(0),
	m_maxBufferedEventCount(newMaxBufferedEventCount),
	m_eventData(NULL),
	m_directInputDevice(NULL),
	m_GUID()
{
	DEBUG_FATAL(newMaxBufferedEventCount < 1, ("device must buffer at least one event, client specified %d\n", newMaxBufferedEventCount));
	m_eventData = new DIDEVICEOBJECTDATA[static_cast<size_t>(newMaxBufferedEventCount)];

	HRESULT     hr;
	DIPROPDWORD dipdw;

	// create the device
	NOT_NULL(ms_directInput);
	hr = ms_directInput->CreateDevice(newDeviceGuid, &m_directInputDevice, 0);
	FATAL_HR("failed to create DirectInput device (%d)", hr);

	// set the device's event buffer size property (code lifted from DX8 manual)
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = static_cast<DWORD>(newMaxBufferedEventCount);
	hr = m_directInputDevice->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
	DEBUG_FATAL(FAILED(hr), ("failed to set input device event buffer size to %d (error=%d)", newMaxBufferedEventCount, hr & 0xffffL));

	m_GUID = newDeviceGuid;
}

// ----------------------------------------------------------------------

// release resources acquired by DirectInput::Device

Device::~Device()
{
	if (m_directInputDevice)
	{
		static_cast<void>(m_directInputDevice->Unacquire());
		static_cast<void>(m_directInputDevice->Release());
		m_directInputDevice = 0;
	}

	delete [] m_eventData;
	m_eventData = 0;
}

// ----------------------------------------------------------------------
// return the number of events not yet submitted via submitEvent()
//
// Remarks:
//
//   Every time Device::update() is called, the device's buffer
//   is filled with DirectInput event data.  Every call to
//   Device::submitEvent() dequeues the first event in the device's
//   event list.  This function returns the number of events
//   not yet dequeued from the Device() since the last call to
//   Device::update().

int Device::getRemainingEventCount()
{
	return m_nextEventIndex - m_eventCount;
}

// ----------------------------------------------------------------------

// return the sequence number for the next event to be retrieved
// via Device::submitEvent()

inline uint32 Device::getNextEventSequenceNumber()
{
	DEBUG_FATAL(m_nextEventIndex >= m_eventCount, ("no more unprocessed events\n"));
	NOT_NULL(m_eventData);
	return m_eventData[m_nextEventIndex].dwSequence;
}

// ----------------------------------------------------------------------
// retrieve all events that have occurred since the last call to update
//
// Remarks:
//   Devices are created with a maximum number of events that can be
//   buffered.  If more events than the maximum have occurred since
//   the last call to update(), all overflow events are completely
//   lost.

bool Device::update()
{
	NOT_NULL(m_directInputDevice);
	
	HRESULT hr;
	DWORD dwInOut;

	dwInOut = static_cast<DWORD>(m_maxBufferedEventCount);
	hr = m_directInputDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), m_eventData, &dwInOut, 0);

	if (FAILED(hr))
	{
		if (hr != DIERR_INPUTLOST && hr != DIERR_NOTACQUIRED)
		{
			FATAL_HR("failed to get device data (%d)", hr);
			return false;
		}
		else
		{
			// attempt to reacquire the device
			hr = m_directInputDevice->Acquire();
			if (!FAILED(hr))
			{
				ms_sendInputReset = true;

				// throw away the data that comes in immediately after an acquire
				dwInOut = static_cast<DWORD>(m_maxBufferedEventCount);
				IGNORE_RETURN(m_directInputDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), m_eventData, &dwInOut, 0));
			}

			dwInOut = 0;
		}
	}
	else
	{

#ifdef _DEBUG
		DEBUG_REPORT_LOG(hr == DI_BUFFEROVERFLOW, ("DirectInput::update buffer overflow\n"));
#endif

	}

	// update device's event information
	m_eventCount     = static_cast<int>(dwInOut);
	m_nextEventIndex = 0;

	return true;
}

// ----------------------------------------------------------------------
/**
 * Unacquire the DirectInput device.
 */

void Device::unacquire()
{
	IGNORE_RETURN(m_directInputDevice->Unacquire());
}

// ======================================================================

KeyboardDevice::KeyboardDevice(const GUID &newDeviceGuid, int newMaxBufferedEventCount, DWORD newMenuKey)
:
	Device(newDeviceGuid, newMaxBufferedEventCount),
	m_menuKey(newMenuKey),
	m_mayHaveMenuTap(false),
	m_mayHaveAltEnter(false),
	m_windowsKeyEnabledWhenFullscreen(false),
	m_windowsKeyEnabledWhenWindowed(false),
	m_screenShotFunction(NULL),
	m_toggleWindowedModeFunction(NULL),
	m_requestDebugMenuFunction(NULL)
{
	HRESULT hr;

	// device is now constructed but not yet acquired
	NOT_NULL(m_directInputDevice);

	// perform any keyboard-specific property setting here
	hr = m_directInputDevice->SetDataFormat(&c_dfDIKeyboard);
	FATAL_HR("failed to set data format for keyboard (%d)", hr);

	// set the cooperative level
	m_windowsKeyEnabledWhenFullscreen = ConfigClientDirectInput::getWindowsKeyEnabledWhenFullscreen();
	m_windowsKeyEnabledWhenWindowed = ConfigClientDirectInput::getWindowsKeyEnabledWhenWindowed();
	setCooperativeLevel();

	// acquire the device
	IGNORE_RETURN(m_directInputDevice->Acquire());

	// load the translation table

	loadTranslationTable();
}

// ----------------------------------------------------------------------

KeyboardDevice::~KeyboardDevice()
{
	// nothing to do yet
}

// ----------------------------------------------------------------------

void KeyboardDevice::setCooperativeLevel()
{
	bool const isWindowKeyEnabled = ms_isWindowedFunction() ? m_windowsKeyEnabledWhenWindowed : m_windowsKeyEnabledWhenFullscreen;
	HRESULT const hresult = m_directInputDevice->SetCooperativeLevel(ms_window, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | (isWindowKeyEnabled ? 0 : DISCL_NOWINKEY));
	FATAL_HR("failed to set cooperative level (%d)", hresult);
}

// ----------------------------------------------------------------------

void KeyboardDevice::loadTranslationTable()
{
	char keyboardLayoutName[KL_NAMELENGTH];

	if (GetKeyboardLayoutName(keyboardLayoutName))
	{
		DEBUG_REPORT_LOG(true, ("Loading keyboard layout: %s\n", keyboardLayoutName));

		// Reset the translation table

		for (int i = 0; i < 256; ++i)
			m_translationTable[i] = static_cast<uint8>(i);

		// Load all the keyboard scan code mappings

		Iff iff("datatables/scancode_map/scancode_map.iff");
		DataTable dataTable;
		dataTable.load(iff);

		s_scanCodeToKeyNameMap.clear();

		for (int row = 0; row < dataTable.getNumRows(); ++row)
		{
			std::string layoutId(dataTable.getStringValue("LayoutId", row));

			if (_stricmp(keyboardLayoutName, layoutId.c_str()) == 0)
			{
				int const fromScanCode = dataTable.getIntValue("FromScanCode", row);
				int const toScanCode = dataTable.getIntValue("ToScanCode", row);
				std::string const &fromKeyName = dataTable.getStringValue("FromKeyName", row);

				m_translationTable[fromScanCode] = static_cast<uint8>(toScanCode);
				s_scanCodeToKeyNameMap.insert(std::make_pair(toScanCode, fromKeyName));

				DEBUG_REPORT_LOG(true, ("fromScanCode: %d toScanCode: %d fromKeyName: %s\n", fromScanCode, toScanCode, fromKeyName.c_str()));
			}
		}
	}
	else
	{
		DEBUG_WARNING(true, ("Unable to detect keyboard layout."));
	}
}

// ----------------------------------------------------------------------

void KeyboardDevice::setScreenShotFunction(DirectInput::ScreenShotFunction newScreenShotFunction)
{
	m_screenShotFunction = newScreenShotFunction;
}

// ----------------------------------------------------------------------

void KeyboardDevice::setToggleWindowedModeFunction(DirectInput::ToggleWindowedModeFunction newToggleWindowedModeFunction)
{
	m_toggleWindowedModeFunction = newToggleWindowedModeFunction;
}

// ----------------------------------------------------------------------

void KeyboardDevice::setRequestDebugMenuFunction(DirectInput::RequestDebugMenuFunction requestDebugMenuFunction)
{
	m_requestDebugMenuFunction = requestDebugMenuFunction;
}

// ----------------------------------------------------------------------

bool KeyboardDevice::update()
{
	DebugKey::newFrame();
	Device::update();

	return true;
}

// ----------------------------------------------------------------------

void KeyboardDevice::submitEvent()
{
	// ensure we have another event---did you forget to check getRemainingEventCount()?
	DEBUG_FATAL(m_nextEventIndex >= m_eventCount, ("device contains no more events\n"));

	// setup event based on keydown or keyup event
	const DIDEVICEOBJECTDATA &od1 = m_eventData[m_nextEventIndex];
	DWORD key = od1.dwOfs;
	if (key < 256)
		key = m_translationTable[key];
	bool const pressed = (od1.dwData & 0x80) != 0 ? true : false;;

	// move to next event
	++m_nextEventIndex;


	// watch for a left-alt tap to popup the debug menu
	if (key == m_menuKey)
	{
		if (pressed)
			m_mayHaveMenuTap  = true;
		else
		{
			if (m_mayHaveMenuTap && m_requestDebugMenuFunction)
				m_requestDebugMenuFunction();
			m_mayHaveMenuTap = false;
		}
	}
	else
		m_mayHaveMenuTap = false;

	// must be alt-down followed immediately by return-down
	if (m_toggleWindowedModeFunction && pressed)
	{
		if (key == DIK_LMENU || key == DIK_RMENU)
			m_mayHaveAltEnter = true;
		else
		{
			if (m_mayHaveAltEnter && key == DIK_RETURN)
				m_toggleWindowedModeFunction();

			m_mayHaveAltEnter = false;
		}
	}
	else
		m_mayHaveAltEnter = false;

	if (m_screenShotFunction && pressed && key == DIK_SYSRQ)
		m_screenShotFunction();

#if PRODUCTION == 0

	// handle the debug keyboard characters
	if (DebugKey::isActive() && (key >= DIK_1 && key <= DIK_0))
	{
		// 0 is above 9 on the keyboard, not under 1.
		if (key == DIK_0)
		{
			if (pressed)
				DebugKey::pressKey(0);
			else
				DebugKey::releaseKey(0);
		}
		else
		{
			const int k = static_cast<int>(key - DIK_1 + 1);

			if (pressed)
				DebugKey::pressKey(k);
			else
				DebugKey::releaseKey(k);
		}

		return;
	}

#endif

	// send this event to the IoWinManager
	if (pressed)
	{
		IoWinManager::queueKeyDown(0, static_cast<int>(key));
	}
	else
		IoWinManager::queueKeyUp(0, static_cast<int>(key));
}

// ----------------------------------------------------------------------

bool KeyboardDevice::getFullscreenWindowsKeyEnabled() const
{
	return m_windowsKeyEnabledWhenFullscreen;
}

// ----------------------------------------------------------------------

void KeyboardDevice::setFullscreenWindowsKeyEnabled(bool enabled)
{
	unacquire();

	m_windowsKeyEnabledWhenFullscreen = enabled;
	setCooperativeLevel();

	// acquire the device
	IGNORE_RETURN(m_directInputDevice->Acquire());
}

// ----------------------------------------------------------------------

bool KeyboardDevice::getWindowedWindowsKeyEnabled() const
{
	return m_windowsKeyEnabledWhenWindowed;
}

// ----------------------------------------------------------------------

void KeyboardDevice::setWindowedWindowsKeyEnabled(bool enabled)
{
	unacquire();

	m_windowsKeyEnabledWhenWindowed = enabled;
	setCooperativeLevel();

	// acquire the device
	IGNORE_RETURN(m_directInputDevice->Acquire());
}

// ======================================================================

MouseDevice::MouseDevice(const GUID &newDeviceGuid, int newMaxBufferedEventCount)
:
	Device(newDeviceGuid, newMaxBufferedEventCount),
	m_leftHanded(GetSystemMetrics(SM_SWAPBUTTON) == TRUE),
	m_ooxMickeysPerUnit(1.0f),
	m_ooyMickeysPerUnit(1.0f),
	m_oozMickeysPerUnit(1.0f),
	m_xRemainder(0.0f),
	m_yRemainder(0.0f),
	m_zRemainder(0.0f)
{
	HRESULT hr;

	// device is now constructed but not yet acquired
	NOT_NULL(m_directInputDevice);

	// perform any mouse-specific property setting here
	hr = m_directInputDevice->SetDataFormat(&c_dfDIMouse2);
	FATAL_HR("failed to set data format for mouse (%d)", hr);

	// set cooperative level
	hr = m_directInputDevice->SetCooperativeLevel(ms_window, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	FATAL_HR("failed to set cooperative level (%d)", hr);

	// acquire the device
	IGNORE_RETURN(m_directInputDevice->Acquire());
}

// ----------------------------------------------------------------------

MouseDevice::~MouseDevice()
{
	// nothing to do yet
}

// ----------------------------------------------------------------------
bool MouseDevice::update()
{
	Device::update();
	return true;
}

// ----------------------------------------------------------------------

void MouseDevice::submitEvent()
{
	int  button;
	float rawValue;
	float newValue;

	// ensure we have another event---did you forget to check getRemainingEventCount()?
	DEBUG_FATAL(m_nextEventIndex >= m_eventCount, ("device contains no more events\n"));

	// setup event based on keydown or keyup event
	const DIDEVICEOBJECTDATA &od = m_eventData[m_nextEventIndex];

	// move to next event
	++m_nextEventIndex;

//lint -save -e30 // Error -- Expected a constant
	switch(od.dwOfs)
	{
		case DIMOFS_BUTTON0:
		case DIMOFS_BUTTON1:
		case DIMOFS_BUTTON2:
		case DIMOFS_BUTTON3:
		case DIMOFS_BUTTON4:
		case DIMOFS_BUTTON5:
		case DIMOFS_BUTTON6:
		case DIMOFS_BUTTON7:

			button = static_cast<int>(od.dwOfs - static_cast<DWORD>(DIMOFS_BUTTON0));

			// handle left-handed freaks like Frank Roan
			if (m_leftHanded && button < 2)
				button = 1 - button;

			if (od.dwData & 0x80)
				IoWinManager::queueMouseButtonDown(0, button);
			else
				IoWinManager::queueMouseButtonUp(0, button);
			break;

		case DIMOFS_X:
			rawValue   = static_cast<float>(static_cast<LONG>(od.dwData)) * m_ooxMickeysPerUnit + m_xRemainder;
			newValue   = static_cast<float>(static_cast<int32>(rawValue));
			m_xRemainder = rawValue - newValue;

			if (newValue != 0.0f)
				IoWinManager::queueMouseTranslateX(0, newValue);
			break;

		case DIMOFS_Y:
			rawValue   = static_cast<float>(static_cast<LONG>(od.dwData)) * m_ooyMickeysPerUnit + m_yRemainder;
			newValue   = static_cast<float>(static_cast<int32>(rawValue));
			m_yRemainder = rawValue - newValue;

			if (newValue != 0.0f)
				IoWinManager::queueMouseTranslateY(0, newValue);
			break;

		case DIMOFS_Z:
			rawValue   = static_cast<float>(static_cast<LONG>(od.dwData)) * m_oozMickeysPerUnit + m_zRemainder;
			newValue   = static_cast<float>(static_cast<int32>(rawValue));
			m_zRemainder = rawValue - newValue;

			if (newValue != 0.0f)
				IoWinManager::queueMouseTranslateZ(0, newValue);
			break;

		default:
			FATAL(true, ("unknown mouse input, (dwOfs=%d, dwData=0x%08X)\n", od.dwOfs, od.dwData));
	}
//lint -restore // Error -- Expected a constant
}

// ----------------------------------------------------------------------

void MouseDevice::setMouseScale(
	float xMickeysPerUnit,
	bool  xFlip,
	float yMickeysPerUnit,
	bool  yFlip,
	float zMickeysPerUnit,
	bool  zFlip
	)
{
	FATAL(xMickeysPerUnit == 0.0f, ("invalid, xMickeysPerUnit arg set to zero"));
	FATAL(yMickeysPerUnit == 0.0f, ("invalid, yMickeysPerUnit arg set to zero"));
	FATAL(zMickeysPerUnit == 0.0f, ("invalid, zMickeysPerUnit arg set to zero"));

	m_ooxMickeysPerUnit = 1.0f / xMickeysPerUnit;
	m_ooyMickeysPerUnit = 1.0f / yMickeysPerUnit;
	m_oozMickeysPerUnit = 1.0f / zMickeysPerUnit;

	if (xFlip)
		m_ooxMickeysPerUnit *= -1.0f;
	if (yFlip)
		m_ooyMickeysPerUnit *= -1.0f;
	if (zFlip)
		m_oozMickeysPerUnit *= -1.0f;
}

// ======================================================================

void JoystickDevice::getRangeData()
{
	HRESULT     hr;
	DIPROPRANGE pr;

	NOT_NULL(m_directInputDevice);

	memset(&pr, 0, sizeof(DIPROPRANGE));
	pr.diph.dwSize       = sizeof(DIPROPRANGE);
	pr.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	pr.diph.dwHow        = DIPH_BYOFFSET;

	// get x translation range
	pr.diph.dwObj = static_cast<DWORD>(DIJOFS_X); //lint !e413
	hr = m_directInputDevice->GetProperty(DIPROP_RANGE, &pr.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
	{
		FATAL_HR("failed to get joystick range for x (%d)", hr);
		xDeviceCenter = static_cast<float>(((pr.lMin + pr.lMax) / 2) + 1); //lint !e653 // possible loss of fraction
		FATAL((pr.lMax - pr.lMin) == 0, ("zero joystick range for x"));
		ooxDeviceHalfRange = 2.0f / static_cast<float>(pr.lMax - pr.lMin + 1);
		axisExists[0] = true;
		axisData[0] = static_cast<DWORD>(xDeviceCenter);
	}

	// get y translation range
	pr.diph.dwObj = static_cast<DWORD>(DIJOFS_Y); //lint !e413
	hr = m_directInputDevice->GetProperty(DIPROP_RANGE, &pr.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
	{
		FATAL_HR("failed to get joystick range for y (%d)", hr);
		yDeviceCenter = static_cast<float>(((pr.lMin + pr.lMax) / 2) + 1); //lint !e653 // possible loss of fraction
		FATAL((pr.lMax - pr.lMin) == 0, ("zero joystick range for y"));
		ooyDeviceHalfRange = 2.0f / static_cast<float>(pr.lMax - pr.lMin + 1);
		axisExists[1] = true;
		axisData[1] = static_cast<DWORD>(yDeviceCenter);
	}

	// get z translation range
	pr.diph.dwObj = static_cast<DWORD>(DIJOFS_Z); //lint !e413
	hr = m_directInputDevice->GetProperty(DIPROP_RANGE, &pr.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
	{
		FATAL_HR("failed to get joystick range for z (%d)", hr);
		zDeviceCenter = static_cast<float>(((pr.lMin + pr.lMax) / 2) + 1); //lint !e653 // possible loss of fraction
		FATAL((pr.lMax - pr.lMin) == 0, ("zero joystick range for z"));
		oozDeviceHalfRange = 2.0f / static_cast<float>(pr.lMax - pr.lMin + 1);
		axisExists[2] = true;
		axisData[2] = static_cast<DWORD>(yDeviceCenter);
	}

	// get rx rotation range
	pr.diph.dwObj = static_cast<DWORD>(DIJOFS_RX); //lint !e413
	hr = m_directInputDevice->GetProperty(DIPROP_RANGE, &pr.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
	{
		FATAL_HR("failed to get joystick range for rx (%d)", hr);
		rxDeviceCenter = static_cast<float>(((pr.lMin + pr.lMax) / 2) + 1); //lint !e653 // possible loss of fraction
		FATAL((pr.lMax - pr.lMin) == 0, ("zero joystick range for rx"));
		oorxDeviceHalfRange = 2.0f / static_cast<float>(pr.lMax - pr.lMin + 1);
		axisExists[3] = true;
		axisData[3] = static_cast<DWORD>(rxDeviceCenter);
	}

	// get ry rotation range
	pr.diph.dwObj = static_cast<DWORD>(DIJOFS_RY); //lint !e413
	hr = m_directInputDevice->GetProperty(DIPROP_RANGE, &pr.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
	{
		FATAL_HR("failed to get joystick range for ry (%d)", hr);
		rxDeviceCenter = static_cast<float>(((pr.lMin + pr.lMax) / 2) + 1); //lint !e653 // possible loss of fraction
		FATAL((pr.lMax - pr.lMin) == 0, ("zero joystick range for ry"));
		ooryDeviceHalfRange = 2.0f / static_cast<float>(pr.lMax - pr.lMin + 1);
		axisExists[4] = true;
		axisData[4] = static_cast<DWORD>(ryDeviceCenter);
	}

	// get rz rotation range
	pr.diph.dwObj = static_cast<DWORD>(DIJOFS_RZ); //lint !e413
	hr = m_directInputDevice->GetProperty(DIPROP_RANGE, &pr.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
	{
		FATAL_HR("failed to get joystick range for rz (%d)", hr);
		rzDeviceCenter = static_cast<float>(((pr.lMin + pr.lMax) / 2) + 1); //lint !e653 // possible loss of fraction
		FATAL((pr.lMax - pr.lMin) == 0, ("zero joystick range for rz"));
		oorzDeviceHalfRange = 2.0f / static_cast<float>(pr.lMax - pr.lMin + 1);
		axisExists[5] = true;
		axisData[5] = static_cast<DWORD>(rzDeviceCenter);
	}

	pr.diph.dwObj = static_cast<DWORD>(DIJOFS_SLIDER(0)); //lint !e413
	hr = m_directInputDevice->GetProperty(DIPROP_RANGE, &pr.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
	{
		FATAL_HR("failed to get joystick slider0 (%d)", hr);
		slider[0].deviceCenter = static_cast<float>(((pr.lMin + pr.lMax) / 2) + 1); //lint !e653 // possible loss of fraction
		FATAL((pr.lMax - pr.lMin) == 0, ("zero joystick range for slider0"));
		slider[0].deviceHalfRange = 2.0f / static_cast<float>(pr.lMax - pr.lMin + 1);
		slider[0].exists = true;
	}

	pr.diph.dwObj = static_cast<DWORD>(DIJOFS_SLIDER(1)); //lint !e413
	hr = m_directInputDevice->GetProperty(DIPROP_RANGE, &pr.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
	{
		FATAL_HR("failed to get joystick slider1 (%d)", hr);
		slider[1].deviceCenter = static_cast<float>(((pr.lMin + pr.lMax) / 2) + 1); //lint !e653 // possible loss of fraction
		FATAL((pr.lMax - pr.lMin) == 0, ("zero joystick range for slider0"));
		slider[1].deviceHalfRange = 2.0f / static_cast<float>(pr.lMax - pr.lMin + 1);
		slider[1].exists = true;
	}
}

// ----------------------------------------------------------------------

JoystickDevice::JoystickDevice(const GUID &newDeviceGuid, int newMaxBufferedEventCount, int newJoystickNumber)
:
	Device(newDeviceGuid, newMaxBufferedEventCount),
	joystickNumber(newJoystickNumber),
	pollIsNeeded(true),
	xDeviceCenter(0.0f),
	ooxDeviceHalfRange(1.0f/5000.0f),
	yDeviceCenter(0.0f),
	ooyDeviceHalfRange(1.0f/5000.0f),
	zDeviceCenter(0.0f),
	oozDeviceHalfRange(1.0f/5000.0f),
	xClientCenter(0.0f),
	xClientHalfRange(1.0f),
	yClientCenter(0.0f),
	yClientHalfRange(1.0f),
	zClientCenter(0.0f),
	zClientHalfRange(1.0f),
	rxDeviceCenter(0.0f),
	oorxDeviceHalfRange(1.0f/5000.0f),
	ryDeviceCenter(0.0f),
	ooryDeviceHalfRange(1.0f/5000.0f),
	rzDeviceCenter(0.0f),
	oorzDeviceHalfRange(1.0f/5000.0f),
	rxClientCenter(0.0f),
	rxClientHalfRange(1.0f),
	ryClientCenter(0.0f),
	ryClientHalfRange(1.0f),
	rzClientCenter(0.0f),
	rzClientHalfRange(1.0f)
{
	HRESULT   hr;
	DWORD     flags;
	DIDEVCAPS devCaps;

	memset(axisExists, 0, sizeof(axisExists));
	memset(axisData, 0, sizeof(axisData));

	// initialize array data that couldn't be done in the constructor initializer list
	for (int i = 0; i < cms_numberOfJoystickAxis; ++i)
	{
		axisExists[i] = false;
		axisData[i]   = 0;
	}
	for (int j = 0; j < cms_numberOfJoystickSliders; ++j)
	{
		slider[j].exists = false;
		slider[j].deviceCenter = 0.0f;
		slider[j].deviceHalfRange = 0.0f;
		slider[j].clientCenter = 0.5f;
		slider[j].clientHalfRange = 0.5f;
	}

	// device is now constructed but not yet acquired
	NOT_NULL(m_directInputDevice);

	// perform any joystick-specific property setting here
	hr = m_directInputDevice->SetDataFormat(&c_dfDIJoystick);
	FATAL_HR("failed to set data format for mouse (%d)", hr);

	// check if polling is needed
	memset(&devCaps, 0, sizeof(DIDEVCAPS));
	devCaps.dwSize = sizeof(DIDEVCAPS);
	hr = m_directInputDevice->GetCapabilities(&devCaps);
	FATAL_HR("failed to get joystick device caps (%d)", hr);
	pollIsNeeded = (devCaps.dwFlags & DIDC_POLLEDDATAFORMAT) ? true : false;

	// retrieve range for axes
	getRangeData();

	// set cooperative level
	flags = DISCL_FOREGROUND | DISCL_EXCLUSIVE;

	hr = m_directInputDevice->SetCooperativeLevel(ms_window, flags);
	FATAL_HR("failed to set cooperative level (%d)", hr);

	// acquire the device
	IGNORE_RETURN(m_directInputDevice->Acquire());

	DEBUG_REPORT_LOG(true,("Created New Joystick device.\n"));
}

// ----------------------------------------------------------------------

bool JoystickDevice::update()
{
	HRESULT hr;

	if (pollIsNeeded)
	{
		// perform polling
		NOT_NULL(m_directInputDevice);
		hr = m_directInputDevice->Poll();

		// check if we lost the device
		if (FAILED(hr))
		{

			// make sure this is recoverable
			if (hr != DIERR_INPUTLOST && hr != DIERR_NOTACQUIRED)
				return true;

			// attempt to reacquire the device
			hr = m_directInputDevice->Acquire();
			if (FAILED(hr))
				return true;

			// attempt to re-poll the device
			hr = m_directInputDevice->Poll();
			if (FAILED(hr))
				return true;

			ms_sendInputReset = true;
		}

	}

	// retrieve data from buffer
	Device::update();

	// check if we have all the axis data
	bool axisDataPresent[cms_numberOfJoystickAxis];
	memset(axisDataPresent, 0, sizeof(axisDataPresent));
	for (int i = 0; i < m_eventCount; ++i)
	{
		switch (m_eventData[i].dwOfs)
		{
			case DIJOFS_X:
				axisDataPresent[0] = true;
				axisData[0]        = m_eventData[i].dwData;
				break;

			case DIJOFS_Y:
				axisDataPresent[1] = true;
				axisData[1]        = m_eventData[i].dwData;
				break;

			case DIJOFS_Z:
				axisDataPresent[2] = true;
				axisData[2]        = m_eventData[i].dwData;
				break;

			case DIJOFS_RX:
				axisDataPresent[3] = true;
				axisData[3]        = m_eventData[i].dwData;
				break;

			case DIJOFS_RY:
				axisDataPresent[4] = true;
				axisData[4]        = m_eventData[i].dwData;
				break;

			case DIJOFS_RZ:
				axisDataPresent[5] = true;
				axisData[5]        = m_eventData[i].dwData;
				break;

			break;
		}
	}

	// see how many slots we need to fill in
	int inserts = 0;
	for (int j = 0; j < cms_numberOfJoystickAxis; ++j)
		if (axisExists[j] && !axisDataPresent[j])
			++inserts;

	if (inserts && m_eventCount + inserts <= m_maxBufferedEventCount)
	{
		// make room for the new events
		memmove(m_eventData+inserts, m_eventData, m_eventCount*sizeof(*m_eventData));
		m_eventCount += inserts;

		for (int k = 0, slot = 0; k < cms_numberOfJoystickAxis; ++k)
			if (axisExists[k] && !axisDataPresent[k])
			{
				m_eventData[slot].dwData      = axisData[k];
				m_eventData[slot].dwOfs       = cms_axisLookup[k];
				m_eventData[slot].dwSequence  = 0;
				m_eventData[slot].dwTimeStamp = 0;
				++slot;
			}
	}
	
	return true;
}

// ----------------------------------------------------------------------

JoystickDevice::~JoystickDevice()
{
	// nothing to do yet
}

// ----------------------------------------------------------------------

int JoystickDevice::getNumberOfAxis() const
{
	int numberOfAxis = 0;

	for (int i = 0; i < cms_numberOfJoystickAxis; ++i)
		if (axisExists[i])
			++numberOfAxis;
	
	return numberOfAxis;
}

// ----------------------------------------------------------------------

void JoystickDevice::submitEvent()
{
	float devicePercent;
	int  hat, button;

	// ensure we have another event---did you forget to check getRemainingEventCount()?
	DEBUG_FATAL(m_nextEventIndex >= m_eventCount, ("device contains no more events\n"));

	// setup event based on keydown or keyup event
	const DIDEVICEOBJECTDATA &od = m_eventData[m_nextEventIndex];

	// move to next event
	++m_nextEventIndex;

//lint -save -e30 // Error -- Expected a constant

	if (od.dwOfs == DIJOFS_X)
	{
		devicePercent = (static_cast<float>(static_cast<LONG>(od.dwData)) - xDeviceCenter) * ooxDeviceHalfRange;
		IoWinManager::queueJoystickTranslateX(joystickNumber, xClientCenter + devicePercent * xClientHalfRange);
	}
	else if (od.dwOfs == DIJOFS_Y)
	{
		devicePercent = (static_cast<float>(static_cast<LONG>(od.dwData)) - yDeviceCenter) * ooyDeviceHalfRange;
		IoWinManager::queueJoystickTranslateY(joystickNumber, yClientCenter + devicePercent * yClientHalfRange);
	}
	else if (od.dwOfs == DIJOFS_Z)
	{
		devicePercent = (static_cast<float>(static_cast<LONG>(od.dwData)) - zDeviceCenter) * oozDeviceHalfRange;
		IoWinManager::queueJoystickTranslateZ(joystickNumber, zClientCenter + devicePercent * zClientHalfRange);
	}
	else if (od.dwOfs == DIJOFS_RX)
	{
		devicePercent = (static_cast<float>(static_cast<LONG>(od.dwData)) - rxDeviceCenter) * oorxDeviceHalfRange;
		IoWinManager::queueJoystickRotateX(joystickNumber, rxClientCenter + devicePercent * rxClientHalfRange);
	}
	else if (od.dwOfs == DIJOFS_RY)
	{
		devicePercent = (static_cast<float>(static_cast<LONG>(od.dwData)) - ryDeviceCenter) * ooryDeviceHalfRange;
		IoWinManager::queueJoystickRotateY(joystickNumber, ryClientCenter + devicePercent * ryClientHalfRange);
	}
	else if (od.dwOfs == DIJOFS_RZ)
	{
		devicePercent = (static_cast<float>(static_cast<LONG>(od.dwData)) - rzDeviceCenter) * oorzDeviceHalfRange;
		IoWinManager::queueJoystickRotateZ(joystickNumber, rzClientCenter + devicePercent * rzClientHalfRange);
	}
	else if (od.dwOfs >= DIJOFS_POV(0) && od.dwOfs <= DIJOFS_POV(3))
	{
		// pov hat
		hat = (static_cast<int>(od.dwOfs) - static_cast<int>(DIJOFS_POV(0))) / isizeof(DWORD);
		if ((od.dwData & 0xffff) == 0xffff)
			IoWinManager::queueJoystickPOVHatCentered(joystickNumber, hat);
		else
			IoWinManager::queueJoystickPOVHatOffset(joystickNumber, hat, od.dwData * cms_DirectInputAnglesToRadians);
		return;
	}
	else if (od.dwOfs >= DIJOFS_BUTTON(0) && od.dwOfs <= DIJOFS_BUTTON(31))
	{
		// button press or release
		button = static_cast<int>(od.dwOfs - static_cast<DWORD>(DIJOFS_BUTTON0));
		if (od.dwData & 0x80)
			IoWinManager::queueJoystickButtonDown(joystickNumber, button);
		else
			IoWinManager::queueJoystickButtonUp(joystickNumber, button);
	}
	else if (od.dwOfs == DIJOFS_SLIDER(0))
	{
		Slider const & s = slider[0];
		devicePercent = (static_cast<float>(static_cast<LONG>(od.dwData)) - s.deviceCenter) * s.deviceHalfRange;
		float value = s.clientCenter + devicePercent * s.clientHalfRange;
		IoWinManager::queueJoystickSlider(joystickNumber, 0, value);
	}
	else if (od.dwOfs == DIJOFS_SLIDER(1))
	{
		Slider const & s = slider[1];
		devicePercent = (static_cast<float>(static_cast<LONG>(od.dwData)) - s.deviceCenter) * s.deviceHalfRange;
		float value = s.clientCenter + devicePercent * s.clientHalfRange;
		IoWinManager::queueJoystickSlider(joystickNumber, 1, value);
	}
	else
	{
		// signal that the event is not valid
		DEBUG_REPORT_LOG(true, ("unknown joystick event %d\n", od.dwOfs));
	}

//lint -restore // Error -- Expected a constant
}

// ----------------------------------------------------------------------

void JoystickDevice::setJoystickTranslationRange(
	float xCenterVal,
	float xHalfRange,
	bool xFlip,
	float yCenterVal,
	float yHalfRange,
	bool yFlip,
	float zCenterVal,
	float zHalfRange,
	bool zFlip
	)
{
	xClientCenter = xCenterVal;
	if (xFlip)
		xClientHalfRange = -xHalfRange;
	else
		xClientHalfRange = xHalfRange;

	yClientCenter = yCenterVal;
	if (yFlip)
		yClientHalfRange = -yHalfRange;
	else
		yClientHalfRange = yHalfRange;

	zClientCenter = zCenterVal;
	if (zFlip)
		zClientHalfRange = -zHalfRange;
	else
		zClientHalfRange = zHalfRange;
}

// ----------------------------------------------------------------------

void JoystickDevice::setJoystickTranslationDeadZone(
	float xPercentAroundCenter,
	float yPercentAroundCenter,
	float zPercentAroundCenter
	)
{
	DIPROPDWORD prop;
	HRESULT     hr;
	const float  FACTOR = 10000.0f/100.0f;

	NOT_NULL(m_directInputDevice);

	memset(&prop, 0, sizeof(DIPROPDWORD));
	prop.diph.dwSize       = sizeof(DIPROPDWORD);
	prop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	prop.diph.dwHow        = DIPH_BYOFFSET;

	prop.diph.dwObj = static_cast<DWORD>(DIJOFS_X); //lint !e413
	prop.dwData     = static_cast<DWORD>(xPercentAroundCenter * FACTOR);
	hr = m_directInputDevice->SetProperty(DIPROP_DEADZONE, &prop.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
		FATAL_HR("failed to set joystick dead zone for x (%d)", hr);

	prop.diph.dwObj = static_cast<DWORD>(DIJOFS_Y); //lint !e413
	prop.dwData     = static_cast<DWORD>(yPercentAroundCenter * FACTOR);
	hr = m_directInputDevice->SetProperty(DIPROP_DEADZONE, &prop.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
		FATAL_HR("failed to set joystick dead zone for y (%d)", hr);

	prop.diph.dwObj = static_cast<DWORD>(DIJOFS_Z); //lint !e413
	prop.dwData     = static_cast<DWORD>(zPercentAroundCenter * FACTOR);
	hr = m_directInputDevice->SetProperty(DIPROP_DEADZONE, &prop.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
		FATAL_HR("failed to set joystick dead zone for z (%d)", hr);
}

// ----------------------------------------------------------------------

void JoystickDevice::setJoystickTranslationSaturation(
	float xSaturationPercent,
	float ySaturationPercent,
	float zSaturationPercent
	)
{
	DIPROPDWORD prop;
	HRESULT     hr;
	const float  FACTOR = 10000.0f/100.0f;

	NOT_NULL(m_directInputDevice);

	memset(&prop, 0, sizeof(DIPROPDWORD));
	prop.diph.dwSize       = sizeof(DIPROPDWORD);
	prop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	prop.diph.dwHow        = DIPH_BYOFFSET;

	prop.diph.dwObj = static_cast<DWORD>(DIJOFS_X); //lint !e413
	prop.dwData     = static_cast<DWORD>(xSaturationPercent * FACTOR);
	hr = m_directInputDevice->SetProperty(DIPROP_SATURATION, &prop.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
		FATAL_HR("failed to set joystick saturation for x (%d)", hr);

	prop.diph.dwObj = static_cast<DWORD>(DIJOFS_Y); //lint !e413
	prop.dwData     = static_cast<DWORD>(ySaturationPercent * FACTOR);
	hr = m_directInputDevice->SetProperty(DIPROP_SATURATION, &prop.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
		FATAL_HR("failed to set joystick saturation for y (%d)", hr);

	prop.diph.dwObj = static_cast<DWORD>(DIJOFS_Z); //lint !e413
	prop.dwData     = static_cast<DWORD>(zSaturationPercent * FACTOR);
	hr = m_directInputDevice->SetProperty(DIPROP_SATURATION, &prop.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
		FATAL_HR("failed to set joystick saturation for z (%d)", hr);
}

// ----------------------------------------------------------------------

void JoystickDevice::setJoystickRotationRange(
	float xCenterVal,
	float xHalfRange,
	bool xFlip,
	float yCenterVal,
	float yHalfRange,
	bool yFlip,
	float zCenterVal,
	float zHalfRange,
	bool zFlip
	)
{
	rxClientCenter = xCenterVal;
	if (xFlip)
		rxClientHalfRange = -xHalfRange;
	else
		rxClientHalfRange = xHalfRange;

	ryClientCenter = yCenterVal;
	if (yFlip)
		ryClientHalfRange = -yHalfRange;
	else
		ryClientHalfRange = yHalfRange;

	rzClientCenter = zCenterVal;
	if (zFlip)
		rzClientHalfRange = -zHalfRange;
	else
		rzClientHalfRange = zHalfRange;
}

// ----------------------------------------------------------------------

void JoystickDevice::setJoystickRotationDeadZone(
	float xPercentAroundCenter,
	float yPercentAroundCenter,
	float zPercentAroundCenter
	)
{
	DIPROPDWORD prop;
	HRESULT     hr;
	const float  FACTOR = 10000.0f / 100.0f;

	NOT_NULL(m_directInputDevice);

	memset(&prop, 0, sizeof(DIPROPDWORD));
	prop.diph.dwSize       = sizeof(DIPROPDWORD);
	prop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	prop.diph.dwHow        = DIPH_BYOFFSET;

	prop.diph.dwObj = static_cast<DWORD>(DIJOFS_RX); //lint !e413
	prop.dwData     = static_cast<DWORD>(xPercentAroundCenter * FACTOR);
	hr = m_directInputDevice->SetProperty(DIPROP_DEADZONE, &prop.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
		FATAL_HR("failed to set joystick dead zone for rx (%d)", hr);

	prop.diph.dwObj = static_cast<DWORD>(DIJOFS_RY); //lint !e413
	prop.dwData     = static_cast<DWORD>(yPercentAroundCenter * FACTOR);
	hr = m_directInputDevice->SetProperty(DIPROP_DEADZONE, &prop.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
		FATAL_HR("failed to set joystick dead zone for ry (%d)", hr);

	prop.diph.dwObj = static_cast<DWORD>(DIJOFS_RZ); //lint !e413
	prop.dwData     = static_cast<DWORD>(zPercentAroundCenter * FACTOR);
	hr = m_directInputDevice->SetProperty(DIPROP_DEADZONE, &prop.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
		FATAL_HR("failed to set joystick dead zone for rz (%d)", hr);
}

// ----------------------------------------------------------------------

void JoystickDevice::setJoystickRotationSaturation(
	float xSaturationPercent,
	float ySaturationPercent,
	float zSaturationPercent
	)
{
	DIPROPDWORD prop;
	HRESULT     hr;
	const float  FACTOR = 10000.0f / 100.0f;

	NOT_NULL(m_directInputDevice);

	memset(&prop, 0, sizeof(DIPROPDWORD));
	prop.diph.dwSize       = sizeof(DIPROPDWORD);
	prop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	prop.diph.dwHow        = DIPH_BYOFFSET;

	prop.diph.dwObj = static_cast<DWORD>(DIJOFS_RX); //lint !e413
	prop.dwData     = static_cast<DWORD>(xSaturationPercent * FACTOR);
	hr = m_directInputDevice->SetProperty(DIPROP_SATURATION, &prop.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
		FATAL_HR("failed to set joystick saturation for rx (%d)", hr);

	prop.diph.dwObj = static_cast<DWORD>(DIJOFS_RY); //lint !e413
	prop.dwData     = static_cast<DWORD>(ySaturationPercent * FACTOR);
	hr = m_directInputDevice->SetProperty(DIPROP_SATURATION, &prop.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
		FATAL_HR("failed to set joystick saturation for ry (%d)", hr);

	prop.diph.dwObj = static_cast<DWORD>(DIJOFS_RZ); //lint !e413
	prop.dwData     = static_cast<DWORD>(zSaturationPercent * FACTOR);
	hr = m_directInputDevice->SetProperty(DIPROP_SATURATION, &prop.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
		FATAL_HR("failed to set joystick saturation for rz (%d)", hr);
}

// ----------------------------------------------------------------------

void JoystickDevice::setJoystickSliderRange(int sliderIndex, float centerVal, float halfRange, bool flip)
{
	DEBUG_FATAL(sliderIndex < 0 || sliderIndex >= cms_numberOfJoystickSliders, ("joystick slider %d out of range", sliderIndex));
	DEBUG_FATAL(!slider[sliderIndex].exists, ("joystick slider %d does not exist", sliderIndex));

	Slider & s = slider[sliderIndex];
	s.clientCenter = centerVal;
	if (flip)
		s.clientHalfRange = -halfRange;
	else
		s.clientHalfRange = halfRange;
}

// ----------------------------------------------------------------------

void JoystickDevice::setJoystickSliderDeadZone(int sliderIndex, float percentAroundCenter)
{
	DEBUG_FATAL(sliderIndex < 0 || sliderIndex >= cms_numberOfJoystickSliders, ("joystci slider %d out of range", sliderIndex));
	DEBUG_FATAL(!slider[sliderIndex].exists, ("joystick slider %d does not exist", sliderIndex));

	DIPROPDWORD prop;
	HRESULT     hr;
	const float  FACTOR = 10000.0f / 100.0f;

	NOT_NULL(m_directInputDevice);

	memset(&prop, 0, sizeof(DIPROPDWORD));
	prop.diph.dwSize       = sizeof(DIPROPDWORD);
	prop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	prop.diph.dwHow        = DIPH_BYOFFSET;

	prop.diph.dwObj = static_cast<DWORD>(DIJOFS_SLIDER(sliderIndex));
	prop.dwData     = static_cast<DWORD>(percentAroundCenter * FACTOR);
	hr = m_directInputDevice->SetProperty(DIPROP_DEADZONE, &prop.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
		FATAL_HR("failed to set joystick dead zone for slider (%d)", hr);
}

// ----------------------------------------------------------------------

void JoystickDevice::setJoystickSliderSaturation(int sliderIndex, float saturationPercent)
{
	DEBUG_FATAL(sliderIndex < 0 || sliderIndex >= cms_numberOfJoystickSliders, ("joystci slider %d out of range", sliderIndex));
	DEBUG_FATAL(!slider[sliderIndex].exists, ("joystick slider %d does not exist", sliderIndex));

	DIPROPDWORD prop;
	HRESULT     hr;
	const float  FACTOR = 10000.0f / 100.0f;

	NOT_NULL(m_directInputDevice);

	memset(&prop, 0, sizeof(DIPROPDWORD));
	prop.diph.dwSize       = sizeof(DIPROPDWORD);
	prop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	prop.diph.dwHow        = DIPH_BYOFFSET;

	prop.diph.dwObj = static_cast<DWORD>(DIJOFS_SLIDER(sliderIndex));
	prop.dwData     = static_cast<DWORD>(saturationPercent * FACTOR);
	hr = m_directInputDevice->SetProperty(DIPROP_SATURATION, &prop.diph);
	if (hr != DIERR_OBJECTNOTFOUND)
		FATAL_HR("failed to set joystick saturation for slider (%d)", hr);
}

// ----------------------------------------------------------------------

bool JoystickDevice::sliderExists(int const sliderIndex)
{
	if(sliderIndex < 0 || sliderIndex >= cms_numberOfJoystickSliders)
		return false;
	return slider[sliderIndex].exists;
}

// ----------------------------------------------------------------------

void JoystickDevice::loadForceFeedbackEffectIntoTemplate(std::string const & file, ForceFeedbackEffectTemplate * const effectTemplate)
{
	ms_loadingTemplate = effectTemplate;
	m_directInputDevice->EnumEffectsInFile(file.c_str(), EnumEffectsInFileIntoTemplateProc, &joystickNumber, DIFEF_MODIFYIFNEEDED);
	ms_loadingTemplate = NULL;
}

// ----------------------------------------------------------------------

bool JoystickDevice::createEffect(REFGUID guidEffect, LPCDIEFFECT const idEffect, LPDIRECTINPUTEFFECT * const effect, LPUNKNOWN const punkOuterlpdife)
{
	HRESULT const hr = m_directInputDevice->CreateEffect(guidEffect, idEffect, effect, punkOuterlpdife);
	return (!FAILED(hr)) ? true : false;
}

// ======================================================================
// add a device to the list of devices processed during update handling
//
// Remarks:
//   The device list also represents the list of devices that will be
//   destroyed during application cleanup.

void DirectInputNamespace::addDevice(Device *newDevice)
{
	ms_devices.push_back(newDevice);
}

// ----------------------------------------------------------------------

void DirectInputNamespace::installKeyboardDevice(DWORD menuKey)
{
	InstallTimer const installTimer("DirectInputNamespace::installKeyboardDevice");

	NOT_NULL(ms_directInput);
	DEBUG_FATAL(ms_keyboardDevice, ("a keyboard device already exists\n"));
	ms_keyboardDevice = new KeyboardDevice(GUID_SysKeyboard, cms_maxBufferedEventCount, menuKey);
	addDevice(ms_keyboardDevice);
}

// ----------------------------------------------------------------------

void DirectInputNamespace::installMouseDevice()
{
	InstallTimer const installTimer("DirectInputNamespace::installMouseDevice");
	if (ConfigClientDirectInput::getUseMouse())
	{
		NOT_NULL(ms_directInput);
		DEBUG_FATAL(ms_mouseDevice, ("a mouse device already exists\n"));
		ms_mouseDevice = new MouseDevice(GUID_SysMouse, cms_maxBufferedEventCount);
		addDevice(ms_mouseDevice);
	}
}

// ----------------------------------------------------------------------

// DirectInput device enumeration callback function
//
// Remarks:
//   Handles searching for the client's desired joystick device and
//   sets the device up if/when found.

BOOL CALLBACK DirectInputNamespace::enumJoystickDevice(LPCDIDEVICEINSTANCE lpddi, LPVOID)
{
	NOT_NULL(lpddi);

	if (ms_numberOfJoysticksAvailable < cms_numberOfJoysticks)
	{
		for(Devices::iterator iter = ms_devices.begin(); iter != ms_devices.end(); ++iter)
		{
			if(lpddi->guidInstance == (*iter)->getGuid())
				return DIENUM_CONTINUE;
		}

		// create the joystick device
		JoystickDevice * joystickDevice = new JoystickDevice(lpddi->guidInstance, cms_maxBufferedEventCount, ms_numberOfJoysticksAvailable);

		// make sure it reall is a joystick (has multiple input axis)
		if (joystickDevice->getNumberOfAxis() >= 2)
		{
			ms_joystickDevice[ms_numberOfJoysticksAvailable] = joystickDevice;
			addDevice(joystickDevice);
			++ms_numberOfJoysticksAvailable;
		}
		else
		{
			delete joystickDevice;
		}
	}

	return DIENUM_CONTINUE;
}

// ----------------------------------------------------------------------

BOOL CALLBACK DirectInputNamespace::EnumEffectsInFileIntoTemplateProc(LPCDIFILEEFFECT lpdife, LPVOID pvRef)
{
	if(!ms_loadingTemplate)
		return DIENUM_CONTINUE;

	int * const joystickIndex = reinterpret_cast<int *>(pvRef);

	// ignore non joysticks
	if(DirectInput::isJoystickAvailable(*joystickIndex))
	{
		JoystickDevice * const joystick = ms_joystickDevice[*joystickIndex];
		LPDIRECTINPUTEFFECT newEffect = NULL;
		BOOL const result = joystick->createEffect(lpdife->GuidEffect, lpdife->lpDiEffect, &newEffect, NULL);
		if(result && newEffect)
		{
			InternalForceFeedbackEffect * const newInternalEffect = new InternalForceFeedbackEffect(newEffect);
			ms_loadingTemplate->addEffect(newInternalEffect);
		}
	}
	return DIENUM_CONTINUE;
}

// ----------------------------------------------------------------------

void DirectInputNamespace::installJoystickDevices()
{
	InstallTimer const installTimer("DirectInputNamespace::installJoystickDevices");
	if (ConfigClientDirectInput::getUseJoysticks())
	{
		NOT_NULL(ms_directInput);
		HRESULT hr = ms_directInput->EnumDevices(DI8DEVCLASS_GAMECTRL, enumJoystickDevice, NULL, DIEDFL_ATTACHEDONLY);
		FATAL_HR("failed to enumerate gamectrl devices (%d)", hr);
	}
}

//----------------------------------------------------------------------

int DirectInput::getNumberOfJoysticksAvailable()
{
	return ms_numberOfJoysticksAvailable;
}

// ----------------------------------------------------------------------

DirectInput::ForceFeedbackEffect::ForceFeedbackEffect()
{}

// ----------------------------------------------------------------------

DirectInput::ForceFeedbackEffect::~ForceFeedbackEffect()
{}

// ----------------------------------------------------------------------

InternalForceFeedbackEffect::InternalForceFeedbackEffect(IDirectInputEffect * const effect)
: m_effect(effect)
{
	DEBUG_WARNING(NULL == m_effect, ("Creating a InternalForceFeedbackEffect with a NULL IDirectInputEffect, this is probably an error"));
}

// ----------------------------------------------------------------------

InternalForceFeedbackEffect::~InternalForceFeedbackEffect()
{
	if(m_effect)
	{
		m_effect->Release();
		m_effect = NULL;
	}
}

// ----------------------------------------------------------------------

bool InternalForceFeedbackEffect::play(int const iterations) const
{
	if(!m_effect)
		return false;

	HRESULT const startResult = m_effect->Start(static_cast<unsigned int>(iterations), 0);
	return (!FAILED(startResult));
}

// ----------------------------------------------------------------------

bool InternalForceFeedbackEffect::stop() const
{
	if(!m_effect)
		return false;

	HRESULT const stopResult = m_effect->Stop();
	return (!FAILED(stopResult));
}

// ----------------------------------------------------------------------

bool InternalForceFeedbackEffect::isPlaying() const
{
	if(!m_effect)
		return false;

	DWORD flags = 0;
	m_effect->GetEffectStatus(&flags);
	return (flags == DIEGES_PLAYING);
}


// ----------------------------------------------------------------------
/**
 * install the DirectInput system.
 *
 * @param instanceHandle  [IN]  handle of EXE/DLL making use of DirectInput
 */

void DirectInput::install(HINSTANCE instanceHandle, HWND window, DWORD menuKey, IsWindowedFunction isWindowedFunction)
{
	InstallTimer const installTimer("DirectInput::install");

	DEBUG_FATAL(ms_installed, ("DirectInput already installed"));
	DEBUG_FATAL(!instanceHandle, ("null instanceHandle arg"));

	ms_window = window;
	ms_isWindowedFunction= isWindowedFunction;

	HRESULT hr;

	// retrieve IDirectInput
	hr = DirectInput8Create(instanceHandle, DIRECTINPUT_VERSION, IID_IDirectInput8A, reinterpret_cast<void **>(&ms_directInput), 0);
	FATAL_HR("DirectInput8 device could not be created %d", hr);

	// we can install ourselves now that we've successfully retrieved the DX8 DirectInput
	ms_installed = true;
	ExitChain::add(DirectInput::remove, "DirectInput::remove", 0, true);

	// setup DirectInput::Device for each device the client wants handled
	installKeyboardDevice(menuKey);
	installMouseDevice();
	installJoystickDevices();

	// -qq- read mouse and joystick configuration information from
	// some configuration file here.  for now, just setting to
	// reasonable defaults
	// set initial mouse conditions
	if (isMouseAvailable())
		setMouseScale(1.0f, false, 1.0f, false, 1.0f, false);

	// set initial joystick conditions
	for (int i = 0; i < cms_numberOfJoysticks; ++i)
		if (isJoystickAvailable(i))
		{
			setJoystickTranslationRange(i, 0.0f, 1.0f, false, 	0.0f, 1.0f, false, 	0.0f, 1.0f, false);
			setJoystickTranslationDeadZone(i, 10.0f, 10.0f, 10.0f);
			setJoystickTranslationSaturation(i, 90.0f, 90.0f, 90.0f);

			setJoystickRotationRange(i, 0.0f, 1.0f, false, 0.0f, 1.0f, false, 0.0f, 1.0f, false);
			setJoystickRotationDeadZone(i, 10.0f, 10.0f, 10.0f);
			setJoystickRotationSaturation(i, 90.0f, 90.0f,	90.0f);

			setJoystickSliderRange(i, 0, 0.5, 0.5, true);
		}

}

// ----------------------------------------------------------------------
/**
 * remove the DirectInput system.
 *
 * After this call, no more events will be handled by the DirectInput
 * system.  In addition, all allocated resources will be released.
 */

void DirectInput::remove()
{
	DEBUG_FATAL(!ms_installed, ("attempted to remove DirectInput when not installed\n"));

	// release each handled device
	while (!ms_devices.empty())
	{
		Device *deadDevice = ms_devices.back();
		ms_devices.pop_back();
		delete deadDevice;
	}

	// release IDirectInput
	if (ms_directInput)
	{
		static_cast<void>(ms_directInput->Release());
		ms_directInput = 0;
	}

	ms_installed = false;
}

// ----------------------------------------------------------------------
/**
 * Check whether Direct Input is installed.
 * @return True if Direct Input is already installed, otherwise false.
 */

bool DirectInput::isInstalled()
{
	return ms_installed;
}

// ----------------------------------------------------------------------
/*
 * Suspend input from devices.
 *
 * Calling this routine causes DirectInput to unacquire all its devices and
 * prevent update() from attempting to reacquire those devices until resumeInput()
 * has been called.
 */
void DirectInput::suspendInput()
{
	unacquireAllDevices();
	ms_suspended = true;
}

// ----------------------------------------------------------------------
/*
 * Resume input from devices.
 *
 * Calling this routine allows DirectInput to reacquire all its devices next
 * update and resume sending device input.
 */

void DirectInput::resumeInput()
{
	ms_suspended = false;
}

// ----------------------------------------------------------------------

bool DirectInput::getFullscreenWindowsKeyEnabled()
{
	if (ms_keyboardDevice)
		return ms_keyboardDevice->getFullscreenWindowsKeyEnabled();
	return false;
}

// ----------------------------------------------------------------------

void DirectInput::setFullscreenWindowsKeyEnabled(bool enabled)
{
	if (ms_keyboardDevice)
		ms_keyboardDevice->setFullscreenWindowsKeyEnabled(enabled);
}

// ----------------------------------------------------------------------

bool DirectInput::getWindowedWindowsKeyEnabled()
{
	if (ms_keyboardDevice)
		return ms_keyboardDevice->getWindowedWindowsKeyEnabled();
	return false;
}

// ----------------------------------------------------------------------

void DirectInput::setWindowedWindowsKeyEnabled(bool enabled)
{
	if (ms_keyboardDevice)
		ms_keyboardDevice->setWindowedWindowsKeyEnabled(enabled);
}

// ----------------------------------------------------------------------
/**
 * Unacquire all the DirectInput devices.
 */

void DirectInput::unacquireAllDevices()
{
	if (ms_installed)
	{
		Devices::iterator const iEnd = ms_devices.end();
		for (Devices::iterator i = ms_devices.begin(); i != iEnd; ++i)
			(*i)->unacquire();
		ms_sendInputReset = true;
	}
}

// ----------------------------------------------------------------------
/**
 * process all events received by configured DirectInput devices,
 * propagating the events through the IoWinManager.
 *
 * It is only valid to call this function between calls to
 * DirectInput::install() and DirectInput::remove().  This
 * function should be called every time the client wishes to
 * process events queued by DirectInput.  Events are always
 * passed to the IoWinManager chronologically (i.e. if event
 * b is passed to IoWinManager after event a, it must be true
 * that event b occurred exactly at the same time or after
 * event a occurred).
 */

void DirectInput::update()
{
	DEBUG_FATAL(!ms_installed, ("DirectInput not installed"));

	if (ms_suspended)
		return;

	// update each device
	{
		Devices::iterator const iEnd = ms_devices.end();
		for (Devices::iterator i = ms_devices.begin(); i != iEnd; )
		{
			if(!(*i)->update())
			{
				// Some device died. Right now only Joysticks can "die" so let's kill it!
				Devices::iterator removeIter = i;
				Device* device = *i;
				++i;
				ms_devices.erase(removeIter);
				--ms_numberOfJoysticksAvailable;
				ms_joystickDevice[safe_cast<DirectInputNamespace::JoystickDevice*>(device)->getJoystickNumber()] = 0;

				delete device;
				continue;

			}

			++i;
		}
	}

	// check if we send an input reset first
	if (ms_sendInputReset)
	{
		// inform the client that we lost the input device(s), and that
		// the client should assume all buttons and such are in their
		// default state.

		DebugKey::lostFocus();
		IoWinManager::queueInputReset();
		ms_sendInputReset = false;
	}

	// send each buffered event down the pipe in the order they were received
	for (;;)
	{
		Device * dataDevice = NULL;
		uint32 dataSequenceNumber = 0;

		Devices::iterator const iEnd = ms_devices.end();
		for (Devices::iterator i = ms_devices.begin(); i != iEnd; ++i)
		{
			Device *device = *i;

			// check if device has any remaining events
			if (device->getRemainingEventCount())
			{
				// check if this event came first
				uint32 testSequenceNumber = device->getNextEventSequenceNumber();
				if (!dataDevice || DISEQUENCE_COMPARE(dataSequenceNumber, >, testSequenceNumber))
				{
					dataDevice         = device;
					dataSequenceNumber = testSequenceNumber;
				}
			}
		}

		// check if we have an event
		if (!dataDevice)
			break;

		dataDevice->submitEvent();
	}

	ForceFeedbackEffectTemplate::update();
}

// ----------------------------------------------------------------------
/**
 * Allow the DirectInput system take screen shots when Shift-PrintScreen is pressed.
 *
 * Screen shots will be written out as a file named screenShot####.FMT, where the
 * #### is a 4-digit increasing number, and FMT indicates the format of the image.
 * The number is reset to 0 for the start of every run.
 */

void DirectInput::setScreenShotFunction(ScreenShotFunction screenShotFunction)
{
	DEBUG_FATAL(!ms_installed, ("DirectInput not installed"));
	if (ms_keyboardDevice)
		ms_keyboardDevice->setScreenShotFunction(screenShotFunction);
}

// ----------------------------------------------------------------------
/**
 * Allow the DirectInput to toggle windowed mode when Alt-Enter is pressed.
 */

void DirectInput::setToggleWindowedModeFunction(ToggleWindowedModeFunction toggleWindowedModeFunction)
{
	DEBUG_FATAL(!ms_installed, ("DirectInput not installed"));
	if (ms_keyboardDevice)
		ms_keyboardDevice->setToggleWindowedModeFunction(toggleWindowedModeFunction);
}

// ----------------------------------------------------------------------

void DirectInput::setRequestDebugMenuFunction(RequestDebugMenuFunction requestDebugMenuFunction)
{
	DEBUG_FATAL(!ms_installed, ("DirectInput not installed"));
	if (ms_keyboardDevice)
		ms_keyboardDevice->setRequestDebugMenuFunction(requestDebugMenuFunction);
}

// ----------------------------------------------------------------------

void DirectInput::inputLanguageChanged()
{
	DEBUG_FATAL(!ms_installed, ("DirectInput not installed"));
	if (ms_keyboardDevice)
		ms_keyboardDevice->loadTranslationTable();
}

// ----------------------------------------------------------------------

int DirectInput::getMaximumNumberOfJoysticks()
{
	return cms_numberOfJoysticks;
}

// ----------------------------------------------------------------------

int DirectInput:: getMaximumNumberOfSlidersPerJoystick()
{
	return cms_numberOfJoystickSliders;
}

// ----------------------------------------------------------------------
/**
 * retrieve whether a client-specified joystick is available for input.
 *
 * This function must return true before the client calls any of the
 * DirectInput::setJoystick* functions.
 */

bool DirectInput::isJoystickAvailable(int joystickIndex)
{
	DEBUG_FATAL(!ms_installed, ("DirectInput not installed"));
	return (ms_joystickDevice[joystickIndex] != 0);
}

// ----------------------------------------------------------------------
/**
 * specify the joystick coordinate range for the translational axes.
 *
 * The client specifies the center value, the value added when
 * the joystick is moved to the max position, and whether the
 * orientation of the axis is reversed for each of the three
 * translational joystick axes.
 *
 * For standard joysticks, the following is typical behavior
 * for the following axes:
 * x-axis: minimal value = left
 * y-axis: minimal value = up
 * z-axis: minimal value = farthest position away from user
 *
 * e.g.
 * xCenterVal = 10.0f
 * xHalfRange = 1.0f
 * xFlip      = false
 * will yield a value of 10 when the joystick is centered,
 * a value of 9 when the joystick is all the way to the left,
 * and a value of 11 when the joystick is all the way to the right
 * for the x direction.
 *
 * The default value is (centerVal = 0, halfRange = 1, flip = false)
 * for each axis.
 *
 * This function is only valid to call when getIsJoystickAvailable()
 * returns true.
 *
 * @see setJoystickTranslationDeadZone(), setJoystickTranslationSaturation(),
 * setJoystickRotationRange()
 */

void DirectInput::setJoystickTranslationRange(int joystickIndex, float xCenterVal, float xHalfRange, bool xFlip, float yCenterVal, float yHalfRange, bool yFlip, float zCenterVal, float zHalfRange, bool zFlip)
{
	DEBUG_FATAL(!ms_installed, ("DirectInput not installed"));
	DEBUG_FATAL(joystickIndex < 0 || joystickIndex >= cms_numberOfJoysticks, ("joystick index out of range %d/%d", joystickIndex, cms_numberOfJoysticks));
	DEBUG_FATAL(!ms_joystickDevice[joystickIndex], ("attempted to set joystick translation range on uninstalled joystick\n"));
	ms_joystickDevice[joystickIndex]->setJoystickTranslationRange(xCenterVal, xHalfRange, xFlip, yCenterVal, yHalfRange, yFlip, zCenterVal, zHalfRange, zFlip);
}

// ----------------------------------------------------------------------
/**
 * set the percentage of total range reported as the center-value for
 * the translational joystick axes.
 *
 * The arguments should be specified in percents, such that a value
 * of 10.0f = 10% of the total range.  In this case, 5%
 * of the total range to either side of the center point will report
 * the center position for that axis.
 *
 * The default is 10% for each axis.
 *
 * This function is only valid to call when getIsJoystickAvailable()
 * returns true.
 */

void DirectInput::setJoystickTranslationDeadZone(int joystickIndex, float xPercentAroundCenter, float yPercentAroundCenter, float zPercentAroundCenter)
{
	DEBUG_FATAL(!ms_installed, ("DirectInput not installed"));
	DEBUG_FATAL(joystickIndex < 0 || joystickIndex >= cms_numberOfJoysticks, ("joystick index out of range %d/%d", joystickIndex, cms_numberOfJoysticks));
	DEBUG_FATAL(!ms_joystickDevice[joystickIndex], ("attempted to set joystick translation dead zone on uninstalled joystick\n"));
	ms_joystickDevice[joystickIndex]->setJoystickTranslationDeadZone(xPercentAroundCenter, yPercentAroundCenter, zPercentAroundCenter);
}

// ----------------------------------------------------------------------
/**
 * set the saturation values for the joystick translational axes.
 *
 * Each argument represents the percentage of total movement in one
 * direction at which the value for the max movement will be reported.
 *
 * e.g.
 * Assume the client has setup the x axis to return values centered
 * at 0 and extending 1 in either direction, for a range of -1 to 1
 * for x values.  If the client assigns xSaturationPercent = 90,
 * the joystick will report a value of -1 when the client moves 90%
 * of the way between the joystick's center position and the maximum
 * left position.
 *
 * The default is 90% for each axis.
 *
 * This function is only valid to call when getIsJoystickAvailable()
 * returns true.
 */

void DirectInput::setJoystickTranslationSaturation(int joystickIndex, float xSaturationPercent, float ySaturationPercent, float zSaturationPercent)
{
	DEBUG_FATAL(!ms_installed, ("DirectInput not installed"));
	DEBUG_FATAL(joystickIndex < 0 || joystickIndex >= cms_numberOfJoysticks, ("joystick index out of range %d/%d", joystickIndex, cms_numberOfJoysticks));
	DEBUG_FATAL(!ms_joystickDevice[joystickIndex], ("attempted to set joystick translation saturation on uninstalled joystick\n"));
	ms_joystickDevice[joystickIndex]->setJoystickTranslationSaturation(xSaturationPercent, ySaturationPercent, zSaturationPercent);
}

// ----------------------------------------------------------------------
/**
 * specify the joystick coordinate range for the rotational axes.
 *
 * The client specifies the center value, the value added when
 * the joystick is rotated to the max position, and whether the
 * orientation of the axis is reversed for each of the three
 * rotational joystick axes.
 *
 * For standard joysticks, the following is typical behavior
 * for the following axes:
 * z-axis: minimal value = rotated CCW
 *
 * e.g.
 * zCenterVal = 25.0f
 * zHalfRange = 5.0f
 * zFlip      = false
 * will yield a value of 25 when the joystick is centered,
 * a value of 20 when the joystick is rotated completely CCW,
 * and a value of 30 when the joystick is rotated completely CW
 * for the rotational z direction.
 *
 * The default value is (centerVal = 0, halfRange = 1, flip = false)
 * for each axis.
 *
 * This function is only valid to call when getIsJoystickAvailable()
 * returns true.
 *
 * @see setJoystickRotationDeadZone, setJoystickRotationSaturation,
 * setJoystickTranslationRange
 */

void DirectInput::setJoystickRotationRange(int joystickIndex, float xCenterVal, float xHalfRange, bool xFlip, float yCenterVal, float yHalfRange, bool yFlip, float zCenterVal, float zHalfRange, bool zFlip)
{
	DEBUG_FATAL(!ms_installed, ("DirectInput not installed"));
	DEBUG_FATAL(joystickIndex < 0 || joystickIndex >= cms_numberOfJoysticks, ("joystick index out of range %d/%d", joystickIndex, cms_numberOfJoysticks));
	DEBUG_FATAL(!ms_joystickDevice[joystickIndex], ("attempted to set joystick rotation range on uninstalled joystick\n"));
	ms_joystickDevice[joystickIndex]->setJoystickRotationRange(xCenterVal, xHalfRange, xFlip, yCenterVal, yHalfRange, yFlip, zCenterVal, zHalfRange, zFlip);
}

// ----------------------------------------------------------------------
/**
 * set the percentage of total range reported as the center-value for
 * the rotational joystick axes.
 *
 * The arguments should be specified in percents, such that a value
 * of 15.0f = 15% of the total range.  In this case, 7.5%
 * of the total range to either side of the center point will report
 * the center position for that axis.
 *
 * The default is 10% for each axis.
 *
 * This function is only valid to call when getIsJoystickAvailable()
 * returns true.
 */

void DirectInput::setJoystickRotationDeadZone(int joystickIndex, float xPercentAroundCenter, float yPercentAroundCenter, float zPercentAroundCenter)
{
	DEBUG_FATAL(!ms_installed, ("DirectInput not installed"));
	DEBUG_FATAL(joystickIndex < 0 || joystickIndex >= cms_numberOfJoysticks, ("joystick index out of range %d/%d", joystickIndex, cms_numberOfJoysticks));
	DEBUG_FATAL(!ms_joystickDevice[joystickIndex], ("attempted to set joystick rotation dead zone on uninstalled joystick\n"));
	ms_joystickDevice[joystickIndex]->setJoystickRotationDeadZone(xPercentAroundCenter, yPercentAroundCenter, zPercentAroundCenter);
}

// ----------------------------------------------------------------------
/**
 * set the saturation values for the joystick rotational axes.
 *
 * Each argument represents the percentage of total movement in one
 * direction at which the value for the max movement will be reported.
 *
 * e.g.
 * Assume the client has setup the rotational z axis to return values centered
 * at 50 and extending 10 in either direction, for a range of 40 to 60
 * for z values.  If the client assigns zSaturationPercent = 75,
 * the joystick will report a value of 60 when the client rotates 75%
 * of the way between the joystick's center position and the maximum
 * CW rotational z-axis position.
 *
 * The default is 90% for each axis.
 *
 * This function is only valid to call when getIsJoystickAvailable()
 * returns true.
 */

void DirectInput::setJoystickRotationSaturation(int joystickIndex, float xSaturationPercent, float ySaturationPercent, float zSaturationPercent)
{
	DEBUG_FATAL(!ms_installed, ("DirectInput not installed"));
	DEBUG_FATAL(joystickIndex < 0 || joystickIndex >= cms_numberOfJoysticks, ("joystick index out of range %d/%d", joystickIndex, cms_numberOfJoysticks));
	DEBUG_FATAL(!ms_joystickDevice[joystickIndex], ("attempted to set joystick rotation saturation on uninstalled joystick\n"));
	ms_joystickDevice[joystickIndex]->setJoystickRotationSaturation(xSaturationPercent, ySaturationPercent, zSaturationPercent);
}

// ----------------------------------------------------------------------

void DirectInput::setJoystickSliderRange(int joystickIndex, int sliderIndex, float centerVal, float halfRange, bool flip)
{
	DEBUG_FATAL(!ms_installed, ("DirectInput not installed"));
	DEBUG_FATAL(joystickIndex < 0 || joystickIndex >= cms_numberOfJoysticks, ("joystick index out of range %d/%d", joystickIndex, cms_numberOfJoysticks));
	DEBUG_FATAL(!ms_joystickDevice[joystickIndex], ("attempted to set slider range on uninstalled joystick\n"));
	if(ms_joystickDevice[joystickIndex]->sliderExists(sliderIndex))
		ms_joystickDevice[joystickIndex]->setJoystickSliderRange(sliderIndex, centerVal, halfRange, flip);
}

// ----------------------------------------------------------------------

void DirectInput::setJoystickSliderDeadZone(int joystickIndex, int sliderIndex, float percentAroundCenter)
{
	DEBUG_FATAL(!ms_installed, ("DirectInput not installed"));
	DEBUG_FATAL(joystickIndex < 0 || joystickIndex >= cms_numberOfJoysticks, ("joystick index out of range %d/%d", joystickIndex, cms_numberOfJoysticks));
	DEBUG_FATAL(!ms_joystickDevice[joystickIndex], ("attempted to set slider range on uninstalled joystick\n"));
	ms_joystickDevice[joystickIndex]->setJoystickSliderDeadZone(sliderIndex, percentAroundCenter);
}

// ----------------------------------------------------------------------

void DirectInput::setJoystickSliderSaturation(int joystickIndex, int sliderIndex, float saturationPercent)
{
	DEBUG_FATAL(!ms_installed, ("DirectInput not installed"));
	DEBUG_FATAL(joystickIndex < 0 || joystickIndex >= cms_numberOfJoysticks, ("joystick index out of range %d/%d", joystickIndex, cms_numberOfJoysticks));
	DEBUG_FATAL(!ms_joystickDevice[joystickIndex], ("attempted to set slider range on uninstalled joystick\n"));
	ms_joystickDevice[joystickIndex]->setJoystickSliderSaturation(sliderIndex, saturationPercent);
}

// ----------------------------------------------------------------------

void DirectInput::reaquireJoystick()
{
	DirectInputNamespace::installJoystickDevices();
}
// ----------------------------------------------------------------------
/**
 * retrieve whether a mouse is available for input.
 *
 * This function must return true before the client calls any of the
 * DirectInput::setMouse* functions.
 */

bool DirectInput::isMouseAvailable()
{
	DEBUG_FATAL(!ms_installed, ("DirectInput not installed"));
	return (ms_mouseDevice != 0);
}

// ----------------------------------------------------------------------
/**
 * specify the number of mouse device units for each client unit
 * reported for each of the translational axes.
 *
 * The client specifies the number of mickeys that must be
 * moved through before a client mouse movement unit is reported.
 * An internal remainder factor is kept such that small mouse
 * movements in a given direction, each of which would result in
 * less than one client unit, will eventually cause a client unit
 * to be reported in that direction.
 *
 * The default value is 20 Mickeys per unit for each axis.
 *
 * It is only valid to call this function if getIsMouseAvailable()
 * returned true.
 */

void DirectInput::setMouseScale(float xMickeysPerUnit, bool xFlip, float yMickeysPerUnit, bool yFlip, float zMickeysPerUnit, bool zFlip)
{
	DEBUG_FATAL(!ms_installed, ("DirectInput not installed"));
	DEBUG_FATAL(!ms_mouseDevice, ("attempted to set mouse scale with no mouse available\n"));
	ms_mouseDevice->setMouseScale(xMickeysPerUnit, xFlip, yMickeysPerUnit, yFlip, zMickeysPerUnit, zFlip);
}

// ----------------------------------------------------------------------

bool DirectInput::getScanCodeKeyName(uint8 scanCode, std::string &keyName)
{
	bool result = false;
	ScanCodeToKeyNameMap::const_iterator iterScanCodeToKeyNameMap = s_scanCodeToKeyNameMap.find(scanCode);

	if (iterScanCodeToKeyNameMap != s_scanCodeToKeyNameMap.end())
	{
		keyName = iterScanCodeToKeyNameMap->second;
		result = true;
	}

	return result;
}

// ----------------------------------------------------------------------

bool DirectInput::enumForceFeedbackEffectIntoTemplate(std::string const & file, ForceFeedbackEffectTemplate * const effectTemplate)
{
	DEBUG_FATAL(!ms_installed, ("DirectInput not installed"));

	bool success = false;

	if(!ms_loadingTemplate)
	{
		int const joystickIndex = ConfigClientDirectInput::getUseJoystick();
		if(isJoystickAvailable(joystickIndex))
		{
			JoystickDevice * const joystick = ms_joystickDevice[joystickIndex];
			if(joystick)
			{
				joystick->loadForceFeedbackEffectIntoTemplate(file, effectTemplate);
				success = true;
			}
		}
	}
	return success;
}

// ----------------------------------------------------------------------

bool DirectInput::destroyForceFeedbackEffect(ForceFeedbackEffect * const effect)
{
	DEBUG_FATAL(!ms_installed, ("DirectInput not installed"));

	if(!effect)
	{
		return false;
	}

	delete effect;
	return true;
}

// ----------------------------------------------------------------------

void DirectInput::setIgnoreLAlt(bool ignoreLAlt)
{
	ms_ignoreLAlt = ignoreLAlt;
}

// ======================================================================
