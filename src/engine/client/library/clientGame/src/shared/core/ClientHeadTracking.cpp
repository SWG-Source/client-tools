// ======================================================================
//
// ClientHeadTracking.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientHeadTracking.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/RegistryKey.h"

#include "NPClient.h"

// ======================================================================

namespace ClientHeadTrackingNamespace
{
	char const * const ms_npClientLocation = "Software\\NaturalPoint\\NATURALPOINT\\NPClient Location";

	unsigned short const c_developerId = 6001;

	bool ms_installed;
	bool ms_supported;
	bool ms_enabled;
	bool ms_debugReport;
	HINSTANCE ms_npClientDll;
	PF_NP_GETDATA ms_getData;
	uint16 ms_lastFrameSignature;

	void remove();
}

using namespace ClientHeadTrackingNamespace;

// ======================================================================

void ClientHeadTracking::install()
{
	InstallTimer const installTimer("ClientHeadTracking::install");

	DEBUG_FATAL(ms_installed, ("ClientHeadTracking::install: already installed"));
	ms_installed = true;

	char libraryName[Os::MAX_PATH_LENGTH];
	libraryName[0] = 0;

	uint16 npClientDllVersion = 0;

	RegistryKey * const currentUserRegistryKey = RegistryKey::getCurrentUserKey();
	if (currentUserRegistryKey && currentUserRegistryKey->subKeyExists(ms_npClientLocation))
	{
		RegistryKey * const npClientLocationRegistryKey = currentUserRegistryKey->openSubkey(ms_npClientLocation);
		if (npClientLocationRegistryKey && npClientLocationRegistryKey->getStringValue("Path", "", libraryName, sizeof(libraryName), true))
		{
			strcat(libraryName, "\\NPClient.dll");
			ms_npClientDll = LoadLibrary(libraryName);
			if (ms_npClientDll)
			{
				PF_NP_GETSIGNATURE getSignature = reinterpret_cast<PF_NP_GETSIGNATURE>(GetProcAddress(ms_npClientDll, "NP_GetSignature"));
				NOT_NULL(getSignature);

				SIGNATUREDATA verifySignature;
				strcpy(verifySignature.DllSignature, "precise head tracking\n put your head into the game\n now go look around\n\n Copyright EyeControl Technologies");
				strcpy(verifySignature.AppSignature, "hardware camera\n software processing data\n track user movement\n\n Copyright EyeControl Technologies");

				SIGNATUREDATA signature;
				NPRESULT result = getSignature(&signature);
				if (result == NP_OK &&
					strcmp(verifySignature.DllSignature, signature.DllSignature) == 0 &&
					strcmp(verifySignature.AppSignature, signature.AppSignature) == 0)
				{
					PF_NP_REGISTERWINDOWHANDLE registerWindowHandle = reinterpret_cast<PF_NP_REGISTERWINDOWHANDLE>(GetProcAddress(ms_npClientDll, "NP_RegisterWindowHandle"));
					NOT_NULL(registerWindowHandle);
					if (registerWindowHandle)
						registerWindowHandle(Os::getWindow());

					PF_NP_REGISTERPROGRAMPROFILEID registerProfileId = reinterpret_cast<PF_NP_REGISTERPROGRAMPROFILEID>(GetProcAddress(ms_npClientDll, "NP_RegisterProgramProfileID"));
					NOT_NULL(registerProfileId);
					if (registerProfileId)
						registerProfileId(c_developerId);

					PF_NP_QUERYVERSION queryVersion = reinterpret_cast<PF_NP_QUERYVERSION>(GetProcAddress(ms_npClientDll, "NP_QueryVersion"));
					NOT_NULL(queryVersion);
					if (queryVersion)
						queryVersion(&npClientDllVersion);

					PF_NP_REQUESTDATA requestData = reinterpret_cast<PF_NP_REQUESTDATA>(GetProcAddress(ms_npClientDll, "NP_RequestData"));
					NOT_NULL(requestData);
					if (requestData)
					{
						uint16 dataFields = 0;
						dataFields |= NPYaw;
						dataFields |= NPPitch;
						requestData(dataFields);
					}

					PF_NP_STOPCURSOR stopCursor = reinterpret_cast<PF_NP_STOPCURSOR>(GetProcAddress(ms_npClientDll, "NP_StopCursor"));
					NOT_NULL(stopCursor);
					if (stopCursor)
						stopCursor();

					PF_NP_STARTDATATRANSMISSION startDataTransmission = reinterpret_cast<PF_NP_STARTDATATRANSMISSION>(GetProcAddress(ms_npClientDll, "NP_StartDataTransmission"));
					NOT_NULL(startDataTransmission);
					if (startDataTransmission)
						startDataTransmission();

					ms_getData = reinterpret_cast<PF_NP_GETDATA>(GetProcAddress(ms_npClientDll, "NP_GetData"));
					NOT_NULL(ms_getData);

					ms_supported = ms_getData != 0;
				}
			}

			delete npClientLocationRegistryKey;
		}
	}

	if (ms_supported)
	{
		DEBUG_REPORT_LOG(true, ("ClientHeadTracking::install: loaded %s, version %d.%d\n", libraryName, npClientDllVersion >> 8, npClientDllVersion & 0x0FF));
		DebugFlags::registerFlag(ms_enabled, "ClientHeadTracking", "enable");
		DebugFlags::registerFlag(ms_debugReport, "ClientHeadTracking", "debugReport");
	}
	else
		DEBUG_REPORT_LOG(true, ("ClientHeadTracking::install: failed to load NPClient.DLL\n"));

	ExitChain::add(remove, "ClientHeadTrackingNamespace::remove");
}

// ----------------------------------------------------------------------

void ClientHeadTrackingNamespace::remove()
{
	DEBUG_FATAL(!ms_installed, ("ClientHeadTracking::remove: not installed"));
	ms_installed = false;

	if (ms_npClientDll)
	{
		FreeLibrary(ms_npClientDll);
		ms_npClientDll = 0;
	}
}

// ----------------------------------------------------------------------

bool ClientHeadTracking::isSupported()
{
	return ms_supported;
}

// ----------------------------------------------------------------------

bool ClientHeadTracking::getEnabled()
{
	return ms_supported && ms_enabled;
}

// ----------------------------------------------------------------------

void ClientHeadTracking::setEnabled(bool const enabled)
{
	ms_enabled = ms_supported && enabled;
}

// ----------------------------------------------------------------------

void ClientHeadTracking::getYawAndPitch(float & yaw, float & pitch)
{
	bool polled = false;

	if (getEnabled())
	{
		if (ms_getData)
		{
			TRACKIRDATA trackIrData;
			NPRESULT const result = ms_getData(&trackIrData);
			if (result == NP_OK && trackIrData.wNPStatus == NPSTATUS_REMOTEACTIVE && ms_lastFrameSignature != trackIrData.wPFrameSignature)
			{
				polled = true;

				// We take the negative of the Yaw value since it is backwards from what we expect
				yaw = clamp(-1.f, -trackIrData.fNPYaw / 16384.f, 1.f);
				pitch = clamp(-1.f, trackIrData.fNPPitch / 16384.f, 1.f);

				ms_lastFrameSignature = trackIrData.wPFrameSignature;

				DEBUG_REPORT_PRINT(ms_debugReport, ("y=%1.2f p=%1.2f gy=%1.2f gp=%1.2f\n", trackIrData.fNPYaw, trackIrData.fNPPitch, yaw, pitch));
			}
		}
	}

	if (!polled)
	{
		yaw = 0.f;
		pitch = 0.f;
	}
}

// ======================================================================
