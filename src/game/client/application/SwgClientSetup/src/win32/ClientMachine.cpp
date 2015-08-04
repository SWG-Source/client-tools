// ======================================================================
//
// ClientMachine.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgClientSetup.h"
#include "ClientMachine.h"

#include "DxDiagnostic.h"
#include "Resource.h"
#include "CPUCount.h"

#include <atlbase.h>
#include <d3d9.h>
#include <ddraw.h>
#include <mss.h>
#include <NPClient.h>
#include <string>
#include <vector>
#include <windows.h>
#include <winnls.h>
#include <winioctl.h>

#undef  DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "NPClient.h"

// ======================================================================
// ClientMachineNamespace
// ======================================================================

#define DEBUGGING_OPTIONS 0

// ----------------------------------------------------------------------

namespace ClientMachineNamespace
{
#ifdef _DEBUG
	int const ms_debugPhysicalMemorySize = 128;
	int const ms_debugVideoMemorySize = 32;
	int const ms_debugCpuSpeed = 855;
	int const ms_debugVertexShaderMajorVersion = 1;
	int const ms_debugVertexShaderMinorVersion = 1;
	int const ms_debugPixelShaderMajorVersion = 1;
	int const ms_debugPixelShaderMinorVersion = 1;
#endif

	int                       ms_physicalMemorySize;
	int                       ms_numberOfPhysicalProcessors;
	int                       ms_numberOfLogicalProcessors;
	std::wstring              ms_cpuIdentifier;
	std::wstring              ms_cpuVendor;
	int                       ms_cpuSpeed;
	int                       ms_numberOfCdDrives;
	int                       ms_numberOfDvdDrives;
	std::wstring              ms_os;
	bool                      ms_directXSupported;
	bool                      ms_supportsHardwareMouseCursor;
	int                       ms_videoMemorySize;
	std::wstring              ms_deviceDescription;
	unsigned short            ms_deviceIdentifier;
	unsigned short            ms_vendorIdentifier;
	int                       ms_deviceDriverProduct;
	int                       ms_deviceDriverVersion;
	int                       ms_deviceDriverSubversion;
	int                       ms_deviceDriverBuild;
	std::wstring              ms_deviceDriverVersionText;
	int                       ms_vertexShaderMajorVersion;
	int                       ms_vertexShaderMinorVersion;
	int                       ms_pixelShaderMajorVersion;
	int                       ms_pixelShaderMinorVersion;
	std::wstring              ms_directXVersion;
	int                       ms_directXVersionMajor;
	int                       ms_directXVersionMinor;
	char                      ms_directXVersionLetter;

	typedef std::vector<D3DDISPLAYMODE> DisplayModeList;
	DisplayModeList           ms_displayModeList;
	std::wstring              ms_soundVersion;
	bool                      ms_soundHas2dProvider;
	typedef std::vector<std::wstring> ProviderList;
	ProviderList              ms_soundProviderList;
	TCHAR *                   ms_bitsStatus = _T("unknown");
	IDirectInput8 *           ms_directInput = NULL;
	int                       ms_numberOfJoysticks;
	int                       ms_numberOfFlightSticks;
	int                       ms_numberOfGamepads;
	std::vector<std::wstring> ms_joystickDescriptions;
	std::vector<int>          ms_joystickSupportsForceFeedback;	
	unsigned short ms_npClientDllVersion;

	// table of providers and the Miles open driver specs for them
	// Miles 7 lumps providers and speaker configurations together
	// we pull them apart to be more like previous versions
	struct ProviderInfo
	{
		int spec;
		std::wstring name;
	};

	int const ms_providerInfoCount = 9;

	ProviderInfo ms_providerInfo[ms_providerInfoCount] = 
	{

		{MSS_MC_USE_SYSTEM_CONFIG,_T("Windows Speaker Configuration")},
		{MSS_MC_HEADPHONES,       _T("Headphones")},
		{MSS_MC_STEREO,           _T("2 Speakers")},
		{MSS_MC_40_DISCRETE,      _T("4 Speakers")},
		{MSS_MC_51_DISCRETE,      _T("5.1 Speakers")},
		{MSS_MC_61_DISCRETE,      _T("6.1 Speakers")},
		{MSS_MC_71_DISCRETE,      _T("7.1 Speakers")},
		{MSS_MC_81_DISCRETE,      _T("8.1 Speakers")},
		{MSS_MC_DOLBY_SURROUND,   _T("Dolby Surround")}
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void query3dProviders(std::vector<std::wstring> &providerList)
	{
		// return the provider list
		for (int provider = 0; provider < ms_providerInfoCount; ++provider)
		{
			providerList.push_back(ms_providerInfo[provider].name);
		}
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	BOOL CALLBACK enumJoystick(LPCDIDEVICEINSTANCE lpddi, void *)
	{
		IDirectInputDevice8W * directInputDevice;
		HRESULT hresult = ms_directInput->CreateDevice(lpddi->guidInstance, &directInputDevice, 0);
		if (SUCCEEDED(hresult))
		{
			int numberOfAxis = 0;
			DWORD const fields[6] =
			{
				DIJOFS_X, 
				DIJOFS_Y, 
				DIJOFS_Z, 
				DIJOFS_RX, 
				DIJOFS_RY, 
				DIJOFS_RZ 
			};

			directInputDevice->SetDataFormat(&c_dfDIJoystick);
			for (int i = 0; i < 6; ++i)
			{
				DIPROPRANGE pr;
				memset(&pr, 0, sizeof(DIPROPRANGE));
				pr.diph.dwSize       = sizeof(DIPROPRANGE);
				pr.diph.dwHeaderSize = sizeof(DIPROPHEADER);
				pr.diph.dwHow        = DIPH_BYOFFSET;
				pr.diph.dwObj        = fields[i];
				hresult = directInputDevice->GetProperty(DIPROP_RANGE, &pr.diph);
				if (SUCCEEDED(hresult))
					++numberOfAxis;
			}

			if (numberOfAxis >= 2)
			{
				ms_joystickDescriptions.push_back(std::wstring(lpddi->tszInstanceName));

				// Detect force feedback support
				DIDEVCAPS deviceCaps;
				memset(&deviceCaps, 0, sizeof(deviceCaps));
				deviceCaps.dwSize = sizeof(deviceCaps);
				if (SUCCEEDED(directInputDevice->GetCapabilities(&deviceCaps)) && (deviceCaps.dwFlags & DIDC_FORCEFEEDBACK) != 0)
					ms_joystickSupportsForceFeedback.push_back(1);
				else
					ms_joystickSupportsForceFeedback.push_back(0);

				++ms_numberOfJoysticks;
			}

			directInputDevice->Release();

		}
		return DIENUM_CONTINUE;
	}	

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	// The Miles library functions are narrow.  This was copied the from the Miles API
	void UNICODE_AIL_MSS_version(TCHAR * const versionString, unsigned int const length)
	{
		HINSTANCE milesDll = LoadLibrary(_T("MSS32.DLL")); 
		if (reinterpret_cast<U32>(milesDll) <= 32)
			*versionString = 0;                           
		else 
		{                                
			LoadString(milesDll, 1, versionString, length);            
			FreeLibrary(milesDll);                     
		}
	}                                     

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace ClientMachineNamespace;

// ======================================================================
// STATIC PUBLIC ClientMachine
// ======================================================================

void ClientMachine::install ()
{
	//-- detect memory
	{
		MEMORYSTATUS memoryStatus;
		GlobalMemoryStatus (&memoryStatus);
		ms_physicalMemorySize = memoryStatus.dwTotalPhys / (1024 * 1024);

#if defined(_DEBUG) && DEBUGGING_OPTIONS
		ms_physicalMemorySize = ms_debugPhysicalMemorySize;
#endif
	}

	//-- detect cpu
	{
		unsigned char physicalProcessors = 0;
		unsigned char logicalProcessors = 0;
		if (CPUCount(&logicalProcessors, &physicalProcessors))
		{
			ms_numberOfPhysicalProcessors = physicalProcessors;
			ms_numberOfLogicalProcessors = logicalProcessors;
		}
		else
		{
			SYSTEM_INFO systemInfo;
			GetSystemInfo (&systemInfo);
			ms_numberOfPhysicalProcessors = systemInfo.dwNumberOfProcessors;
			ms_numberOfLogicalProcessors = systemInfo.dwNumberOfProcessors;
		}

		LONG	lresult;
		HKEY	NewKey;

		#define CPU_CONFIGURATION_KEY "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"
		lresult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,_T(CPU_CONFIGURATION_KEY),
			0,KEY_EXECUTE,&NewKey);

		if (ERROR_SUCCESS == lresult)
		{
			TCHAR szKeyValue[100];
			DWORD dwType;
			DWORD dwSize;

			dwType=REG_SZ; dwSize=100;
			memset(szKeyValue,0,100);
			lresult = RegQueryValueEx(NewKey,_T("Identifier"),NULL,&dwType,(LPBYTE)szKeyValue,&dwSize);
			if ((lresult==ERROR_SUCCESS) && (dwSize>0))
			{
				CString sCPUIdentifier = szKeyValue;
				sCPUIdentifier.TrimLeft ();
				sCPUIdentifier.TrimRight ();

				ms_cpuIdentifier = sCPUIdentifier;
			}

			CString sCPUVendorIdentifier;
			dwType=REG_SZ; dwSize=100;
			memset(szKeyValue,0,100);
			lresult=RegQueryValueEx(NewKey,_T("VendorIdentifier"),NULL,&dwType,(LPBYTE)szKeyValue,&dwSize);
			if ((lresult==ERROR_SUCCESS) && (dwSize>0))
			{
				CString sCPUVendorIdentifier = szKeyValue;
				sCPUVendorIdentifier.TrimLeft ();
				sCPUVendorIdentifier.TrimRight ();

				ms_cpuVendor = sCPUVendorIdentifier;
			}

			DWORD dwData;
			dwType=REG_DWORD; dwSize=sizeof(dwData);
			lresult=RegQueryValueEx(NewKey,_T("~MHz"),NULL,&dwType,(LPBYTE)(&dwData),&dwSize);
			if ((lresult==ERROR_SUCCESS) && (dwSize>0))
			{
				ms_cpuSpeed = dwData;
			}

			RegCloseKey(NewKey);
		}

#if defined(_DEBUG) && DEBUGGING_OPTIONS
		ms_cpuSpeed = ms_debugCpuSpeed;
#endif
	}

	//-- detect os
	{
		OSVERSIONINFO versionInfo;
		versionInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		GetVersionEx (&versionInfo);

		ms_os = _T("Unsupported");

		if (versionInfo.dwMajorVersion == 4 && versionInfo.dwMinorVersion == 10)
			ms_os = _T("Windows 98");
		else
			if (versionInfo.dwMajorVersion == 4 && versionInfo.dwMinorVersion == 90)
				ms_os = _T("Windows Me");
			else
				if (versionInfo.dwMajorVersion == 5 && versionInfo.dwMinorVersion == 0)
					ms_os = _T("Windows 2000");
				else
					if (versionInfo.dwMajorVersion == 5 && versionInfo.dwMinorVersion == 1)
						ms_os = _T("Windows XP");
					else
						if (versionInfo.dwMajorVersion == 5 && versionInfo.dwMinorVersion == 2)
							ms_os = _T("Windows Server 2003");
						else
							if (versionInfo.dwMajorVersion == 6 && versionInfo.dwMinorVersion == 0)
								ms_os = _T("Windows Vista");

		if (ms_os != _T("Unsupported"))
		{
			ms_os += _T(" ");
			ms_os += versionInfo.szCSDVersion;
		}
	}

	//-- detect directdraw
	{
		IDirectDraw* directDraw = 0;
		if (SUCCEEDED (DirectDrawCreate (NULL, &directDraw, NULL)) && directDraw)
		{
			DDCAPS caps;
			memset (&caps, 0, sizeof (DDCAPS));
			caps.dwSize = sizeof (DDCAPS);
			if (SUCCEEDED  (directDraw->GetCaps (&caps, 0)))
				ms_videoMemorySize = caps.dwVidMemTotal / (1024 * 1024);

			directDraw->Release ();
			directDraw = 0;
		}

#if defined(_DEBUG) && DEBUGGING_OPTIONS
		ms_videoMemorySize = ms_debugVideoMemorySize;
#endif
	}

	//-- detect direct3d
	{
		IDirect3D9* direct3d = Direct3DCreate9(D3D9b_SDK_VERSION);
		if (direct3d)
		{
			HRESULT hresult;
			D3DDISPLAYMODE displayMode;

			D3DCAPS9 caps;
			hresult = direct3d->GetDeviceCaps (D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
			ms_supportsHardwareMouseCursor = (caps.CursorCaps & D3DCURSORCAPS_COLOR) != 0;
			ms_vertexShaderMajorVersion = (caps.VertexShaderVersion & 0x0000ff00) >> 8;
			ms_vertexShaderMinorVersion = (caps.VertexShaderVersion & 0x000000ff);
			ms_pixelShaderMajorVersion = (caps.PixelShaderVersion & 0x0000ff00) >> 8;
			ms_pixelShaderMinorVersion = (caps.PixelShaderVersion & 0x000000ff);

#if defined(_DEBUG) && DEBUGGING_OPTIONS
			ms_vertexShaderMajorVersion = ms_debugVertexShaderMajorVersion;
			ms_vertexShaderMinorVersion = ms_debugVertexShaderMinorVersion;
			ms_pixelShaderMajorVersion = ms_debugPixelShaderMajorVersion;
			ms_pixelShaderMinorVersion = ms_debugPixelShaderMinorVersion;
#endif

			D3DADAPTER_IDENTIFIER9 identifier;
			direct3d->GetAdapterIdentifier (D3DADAPTER_DEFAULT, 0, &identifier);
			
			ms_deviceDescription = narrowToWide(identifier.Description);

			ms_deviceIdentifier = static_cast<unsigned short> (identifier.DeviceId);
			ms_vendorIdentifier = static_cast<unsigned short> (identifier.VendorId);

			ms_deviceDriverProduct    = HIWORD(identifier.DriverVersion.HighPart);
			ms_deviceDriverVersion    = LOWORD(identifier.DriverVersion.HighPart);
			ms_deviceDriverSubversion = HIWORD(identifier.DriverVersion.LowPart);
			ms_deviceDriverBuild      = LOWORD(identifier.DriverVersion.LowPart);

			CString temp;
			temp.Format (_T("%d.%d.%04d.%04d"), ms_deviceDriverProduct, ms_deviceDriverVersion, ms_deviceDriverSubversion, ms_deviceDriverBuild);
			ms_deviceDriverVersionText = temp;

			//-- enumerate all the available display modes
			int numberOfDisplayModes = direct3d->GetAdapterModeCount (D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8);
			if (numberOfDisplayModes > 0)
			{
				for (int i = 0; i < numberOfDisplayModes; ++i)
				{
					hresult = direct3d->EnumAdapterModes (D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, i, &displayMode);
#ifdef _DEBUG
					temp.Format (_T("D3DDISPLAYMODE: width=%d height=%d refresh=%d format=%d\n"), displayMode.Width, displayMode.Height, displayMode.RefreshRate, displayMode.Format);
					OutputDebugString (temp);
#endif

					//Disable modes above 1280x1024 with 64MB, and modes above 1024x768 with 32MB
					if ((ms_videoMemorySize < 32) && ((displayMode.Width > 1024) || (displayMode.Height > 768)))
						continue;

					if ((ms_videoMemorySize < 64) && ((displayMode.Width > 1280) || (displayMode.Height > 1024)))
						continue;

					if (displayMode.Width >= 1024 && displayMode.Height >= 720 && displayMode.RefreshRate >= 57 && displayMode.RefreshRate <= 75)
						ms_displayModeList.push_back (displayMode);
				}
			}

			if (ms_displayModeList.empty ())
			{
				displayMode.Width = 1024;
				displayMode.Height = 768;
				displayMode.RefreshRate = 60;

				ms_displayModeList.push_back (displayMode);
			}

			ms_directXSupported = true;

			direct3d->Release ();
			direct3d = 0;
		}
	}

	//-- detect audio version
	{
		TCHAR text [MAX_PATH];
		UNICODE_AIL_MSS_version(text, sizeof(text) / sizeof(TCHAR));
		ms_soundVersion = text;
		query3dProviders (ms_soundProviderList);
	}

	//-- detect cd / dvd
	{
		// get all the drive letters in the system
		wchar_t drives[4096];
		int drivesWritten = GetLogicalDriveStrings(sizeof(drives), drives);
		if (drivesWritten > 0 && drivesWritten <= sizeof(drives))
		{
			// iterate over all the drives in the system
			for (wchar_t const * currentDrive = drives; *currentDrive; currentDrive += wcslen(currentDrive) + 1)
			{
				// see if the drive is CD or DVD based
				if (GetDriveType(currentDrive) == DRIVE_CDROM)
				{
					// Get a handle to the device
					wchar_t createFileBuffer[256];
					_stprintf(createFileBuffer, _T("\\\\.\\%c:"), currentDrive[0]);
					HANDLE handle = CreateFile(createFileBuffer, 0, 0, NULL, OPEN_EXISTING, 0, 0);
					if (handle != INVALID_HANDLE_VALUE)
					{
						// Check its media types
						wchar_t buffer[32 * 1024];
						DWORD written = 0;
						if (DeviceIoControl(handle, IOCTL_STORAGE_GET_MEDIA_TYPES_EX, NULL, 0, buffer, sizeof(buffer), &written, 0) && written > 0 && written <= sizeof(buffer))
						{
							GET_MEDIA_TYPES const * const mediaTypes = reinterpret_cast<GET_MEDIA_TYPES const *>(buffer);

							if (mediaTypes->DeviceType == FILE_DEVICE_CD_ROM)
							{
								++ms_numberOfCdDrives;
							}
							else if (mediaTypes->DeviceType == FILE_DEVICE_DVD)
							{
								++ms_numberOfDvdDrives;
							}
						}

						CloseHandle(handle);
					}
				}
			}
		}
	}

	{
		CRegKey regKey;
		if (regKey.Open (HKEY_LOCAL_MACHINE, _T("Software\\microsoft\\windows\\currentversion\\app paths\\StarWarsGalaxies")) == ERROR_SUCCESS)
		{
			DWORD value = 0;
			if (regKey.QueryValue (value, _T("SoeBits")) == ERROR_SUCCESS)
			{
				switch (value)
				{
					case 0:
						ms_bitsStatus = _T("unavailable");
						break;
					case 1:
						ms_bitsStatus = _T("enabled");
						break;
					case 2:
						ms_bitsStatus = _T("disabled");
						break;
					default:
						ms_bitsStatus = _T("invalid");
						break;
						
				}
			}
		}
	}

	//-- detect .NET framework
	{
		HRESULT const hr = DirectInput8Create(AfxGetInstanceHandle(), DIRECTINPUT_VERSION, IID_IDirectInput8W, reinterpret_cast<void **>(&ms_directInput), 0);
		if (SUCCEEDED(hr) && ms_directInput)
		{
			static_cast<void>(ms_directInput->EnumDevices(DI8DEVCLASS_GAMECTRL, enumJoystick, 0, DIEDFL_ATTACHEDONLY));
			ms_directInput->Release();
		}
	}

	// get the Direct X version number
	{
		DxDiagnostic dxDiag;
		
		if (dxDiag.initialize(false))
		{
			if (dxDiag.changeToChildContainer(_T("DxDiag_SystemInfo")))
			{
				unsigned int VerNumMajor, VerNumMinor;
				char buffer[10];
				ms_directXVersion = _T("");

				dxDiag.getUnsignedIntegerProperty(_T("dwDirectXVersionMajor"), VerNumMajor);
				dxDiag.getUnsignedIntegerProperty(_T("dwDirectXVersionMinor"), VerNumMinor);
								
				_itoa(static_cast<int>(VerNumMajor), buffer, 10);
				ms_directXVersion = narrowToWide(buffer);
				ms_directXVersion += _T(".");
				_itoa(static_cast<int>(VerNumMinor), buffer, 10);	
				ms_directXVersion += narrowToWide(buffer);

				dxDiag.getStringProperty(_T("szDirectXVersionLetter"), buffer, 5);
				ms_directXVersion += narrowToWide(buffer);

				ms_directXVersionMajor = VerNumMajor;
				ms_directXVersionMinor = VerNumMinor;
				ms_directXVersionLetter = buffer[0];
			}

			dxDiag.destroy();
		}
	}

	//-- Query the NPClient dll
	{
		CRegKey regKey;
		if (regKey.Open(HKEY_CURRENT_USER, _T("Software\\NaturalPoint\\NATURALPOINT\\NPClient Location")) == ERROR_SUCCESS)
		{
			wchar_t libraryName[512];
			uint32 size = 512;
#if _MSC_VER < 1300
			if (regKey.QueryValue(libraryName, _T("Path"), &size) == ERROR_SUCCESS)
#else
			if (regKey.QueryStringValue(_T("Path"), libraryName, &size) == ERROR_SUCCESS)
#endif
			{
				wcscat(libraryName, _T("\\NPClient.dll"));
				HINSTANCE npClientDll = LoadLibrary(libraryName);
				if (npClientDll)
				{
					PF_NP_GETSIGNATURE getSignature = reinterpret_cast<PF_NP_GETSIGNATURE>(GetProcAddress(npClientDll, "NP_GetSignature"));
					if (getSignature)
					{
						SIGNATUREDATA signature;
						NPRESULT result = getSignature(&signature);
						if (result == NP_OK)
						{
							PF_NP_QUERYVERSION queryVersion = reinterpret_cast<PF_NP_QUERYVERSION>(GetProcAddress(npClientDll, "NP_QueryVersion"));
							if (queryVersion)
								queryVersion(&ms_npClientDllVersion);
						}
					}

					FreeLibrary(npClientDll);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

int ClientMachine::getPhysicalMemorySize ()
{
	return ms_physicalMemorySize;
}

// ----------------------------------------------------------------------

int ClientMachine::getNumberOfPhysicalProcessors ()
{
	return ms_numberOfPhysicalProcessors;
}

// ----------------------------------------------------------------------

int ClientMachine::getNumberOfLogicalProcessors ()
{
	return ms_numberOfLogicalProcessors;
}

// ----------------------------------------------------------------------

CONST TCHAR * ClientMachine::getCpuVendor ()
{
	return ms_cpuVendor.c_str ();
}

// ----------------------------------------------------------------------

int ClientMachine::getNumberOfCdDrives()
{
	return ms_numberOfCdDrives;
}

// ----------------------------------------------------------------------

int ClientMachine::getNumberOfDvdDrives()
{
	return ms_numberOfDvdDrives;
}

// ----------------------------------------------------------------------

const TCHAR* ClientMachine::getCpuIdentifier ()
{
	return ms_cpuIdentifier.c_str ();
}

// ----------------------------------------------------------------------

int ClientMachine::getCpuSpeed ()
{
	return ms_cpuSpeed;
}

// ----------------------------------------------------------------------

const TCHAR* ClientMachine::getOs ()
{
	return ms_os.c_str ();
}

// ----------------------------------------------------------------------

bool ClientMachine::getDirectXSupported ()
{
	return ms_directXSupported;
}

// ----------------------------------------------------------------------

bool ClientMachine::getSupportsHardwareMouseCursor ()
{
	return ms_supportsHardwareMouseCursor;
}

// ----------------------------------------------------------------------

const TCHAR* ClientMachine::getDeviceDescription ()
{
	return ms_deviceDescription.c_str ();
}

// ----------------------------------------------------------------------

unsigned short ClientMachine::getDeviceIdentifier ()
{
	return ms_deviceIdentifier;
}

// ----------------------------------------------------------------------

unsigned short ClientMachine::getVendorIdentifier ()
{
	return ms_vendorIdentifier;
}

// ----------------------------------------------------------------------

const TCHAR * ClientMachine::getDeviceDriverVersionText ()
{
	return ms_deviceDriverVersionText.c_str ();
}

// ----------------------------------------------------------------------

int ClientMachine::getDeviceDriverProduct()
{
	return ms_deviceDriverProduct;
}

// ----------------------------------------------------------------------

int ClientMachine::getDeviceDriverVersion ()
{
	return ms_deviceDriverVersion;
}

// ----------------------------------------------------------------------

int ClientMachine::getDeviceDriverSubversion ()
{
	return ms_deviceDriverSubversion;
}

// ----------------------------------------------------------------------

int ClientMachine::getDeviceDriverBuild()
{
	return ms_deviceDriverBuild;
}

// ----------------------------------------------------------------------

int ClientMachine::getVideoMemorySize ()
{
	return ms_videoMemorySize;
}

// ----------------------------------------------------------------------

int ClientMachine::getVertexShaderMajorVersion ()
{
	return ms_vertexShaderMajorVersion;
}

// ----------------------------------------------------------------------

int ClientMachine::getVertexShaderMinorVersion ()
{
	return ms_vertexShaderMinorVersion;
}

// ----------------------------------------------------------------------

int ClientMachine::getPixelShaderMajorVersion ()
{
	return ms_pixelShaderMajorVersion;
}

// ----------------------------------------------------------------------

int ClientMachine::getPixelShaderMinorVersion ()
{
	return ms_pixelShaderMinorVersion;
}

// ----------------------------------------------------------------------

const TCHAR* ClientMachine::getDirectXVersion ()
{
	return ms_directXVersion.c_str ();
}

// ----------------------------------------------------------------------

int ClientMachine::getDirectXVersionMajor ()
{
	return ms_directXVersionMajor;
}

// ----------------------------------------------------------------------

int ClientMachine::getDirectXVersionMinor ()
{
	return ms_directXVersionMinor;
}

// ----------------------------------------------------------------------

char ClientMachine::getDirectXVersionLetter ()
{
	return ms_directXVersionLetter;
}

// ----------------------------------------------------------------------

bool ClientMachine::supportsVertexShaders ()
{
	return getVertexShaderMajorVersion () != 0;
}

// ----------------------------------------------------------------------

bool ClientMachine::supportsPixelShaders ()
{
	return getPixelShaderMajorVersion () != 0;
}

// ----------------------------------------------------------------------

bool ClientMachine::supportsVertexAndPixelShaders ()
{
	return supportsVertexShaders () && supportsPixelShaders ();
}

// ----------------------------------------------------------------------

int ClientMachine::getNumberOfDisplayModes ()
{
	return static_cast<int> (ms_displayModeList.size ());
}

// ----------------------------------------------------------------------
int ClientMachine::getDisplayModeWidth (int const displayModeIndex)
{
	return static_cast<size_t> (ms_displayModeList [displayModeIndex].Width);
}

// ----------------------------------------------------------------------

int ClientMachine::getDisplayModeHeight (int const displayModeIndex)
{
	return static_cast<size_t> (ms_displayModeList [displayModeIndex].Height);
}

// ----------------------------------------------------------------------

int ClientMachine::getDisplayModeRefreshRate (int const displayModeIndex)
{
	return static_cast<size_t> (ms_displayModeList [displayModeIndex].RefreshRate);
}

// ----------------------------------------------------------------------

const TCHAR* ClientMachine::getSoundVersion ()
{
	return ms_soundVersion.c_str ();
}

// ----------------------------------------------------------------------

bool ClientMachine::getSoundHas2dProvider ()
{
	return ms_soundHas2dProvider;
}

// ----------------------------------------------------------------------

int ClientMachine::getNumberOfSoundProviders ()
{
#if DEBUGGING_OPTIONS
	return 0;
#else
	return static_cast<int> (ms_soundProviderList.size ());
#endif
}

// ----------------------------------------------------------------------

const TCHAR* ClientMachine::getSoundProvider (const int soundProviderIndex)
{
	return ms_soundProviderList [soundProviderIndex].c_str ();
}

// ----------------------------------------------------------------------

const TCHAR* ClientMachine::getBitsStatus()
{
	return ms_bitsStatus;
}

// ----------------------------------------------------------------------

int ClientMachine::getNumberOfJoysticks()
{
	return ms_numberOfJoysticks;
}

// ----------------------------------------------------------------------

const TCHAR * ClientMachine::getJoystickDescription(int joystickIndex)
{
	return ms_joystickDescriptions[joystickIndex].c_str();
}

// ----------------------------------------------------------------------

bool ClientMachine::getJoystickSupportsForceFeedback(int joystickIndex)
{
	return ms_joystickSupportsForceFeedback[joystickIndex] != 0;
}

// ----------------------------------------------------------------------

CString const ClientMachine::getTrackIRVersion()
{
	CString result;
	VERIFY(result.LoadString(IDS_NOT_SUPPORTED));

	if (ms_npClientDllVersion > 0)
		result.Format(_T("%d.%d"), ms_npClientDllVersion >> 8, ms_npClientDllVersion & 0x0FF);
	
	return result;
}

// ----------------------------------------------------------------------

CString const ClientMachine::getHardwareInformationString ()
{
	CString buffer;
	CString result;

	result = _T("Hardware Info Version: 5\n");

	buffer.Format (_T("Number of Processors: %i physical, %i logical"), getNumberOfPhysicalProcessors (), getNumberOfLogicalProcessors ());
	buffer += '\n';
	result += buffer;

	result += _T("Cpu Vendor: ");
	result += getCpuVendor ();
	result += '\n';

	result += _T("Cpu Identifier: ");
	result += getCpuIdentifier ();
	result += '\n';

	buffer.Format (_T("Cpu Speed: %i Mhz"), getCpuSpeed ());
	buffer += '\n';
	result += buffer;

	buffer.Format (_T("Physical Memory Size: %i MB"), getPhysicalMemorySize ());
	buffer += '\n';
	result += buffer;

	buffer.Format (_T("CD: %i"), getNumberOfCdDrives());
	buffer += '\n';
	result += buffer;

	buffer.Format (_T("DVD: %i"), getNumberOfDvdDrives());
	buffer += '\n';
	result += buffer;

	buffer.Format (_T("Joysticks: %i"), ms_numberOfJoysticks);
	buffer += '\n';
	result += buffer;

	buffer.Format (_T("Joysticks: %i"), ms_numberOfJoysticks);
	buffer += '\n';
	result += buffer;

	buffer.Format (_T("FlightSticks: %i"), ms_numberOfFlightSticks);
	buffer += '\n';
	result += buffer;

	buffer.Format (_T("GamePads: %i"), ms_numberOfGamepads);
	buffer += '\n';
	result += buffer;
	result += _T("Video Description: ");
	result += getDeviceDescription ();
	result += '\n';

	buffer.Format (_T("Video Identifier: 0x%04X[v], 0x%04X[d]"), getVendorIdentifier (), getDeviceIdentifier ());
	buffer += '\n';
	result += buffer;

	result += _T("Video Driver Version: ");
	result += getDeviceDriverVersionText ();
	result += '\n';

	buffer.Format (_T("Video Memory Size: %i MB"), getVideoMemorySize ());
	buffer += '\n';
	result += buffer;

	if (getVertexShaderMajorVersion () != 0)
		buffer.Format (_T("Vertex Shader Version: %i.%i"), getVertexShaderMajorVersion (), getVertexShaderMinorVersion ());
	else
		buffer = _T("Vertex Shader Version: Not supported");

	buffer += '\n';
	result += buffer;

	if (getPixelShaderMajorVersion () != 0)
		buffer.Format (_T("Pixel Shader Version: %i.%i"), getPixelShaderMajorVersion (), getPixelShaderMinorVersion ());
	else
		buffer = _T("Pixel Shader Version: Not supported");

	buffer += '\n';
	result += buffer;

	result += _T("DirectX Version: ");
	result += getDirectXVersion();
	result += '\n';

	result += _T("Operating System: ");
	result += getOs ();
	result += '\n';

	result += _T("BITS: ");
	result += getBitsStatus();
	result += '\n';

	result += _T("trackIR Version: ");
	result += getTrackIRVersion();
	result += '\n';

	return result;
}

// ======================================================================
