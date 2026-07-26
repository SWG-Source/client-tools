// ======================================================================
//
// Direct3d11_Device.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_Device.h"

#include "ConfigDirect3d11.h"

#include "clientGraphics/ShaderCapability.h"
#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedFoundation/CrashReportInformation.h"
#include "sharedFoundation/Os.h"

#include <vector>

// ======================================================================

namespace Direct3d11_DeviceNamespace
{
	bool                        ms_installed;

	IDXGIFactory2              *ms_factory;
	IDXGIAdapter1              *ms_adapter;
	ID3D11Device1              *ms_device;
	ID3D11DeviceContext1       *ms_context;
	D3D11_FEATURE_DATA_THREADING ms_threadingSupport;
	D3D11_FEATURE_DATA_D3D11_OPTIONS ms_options;
	D3D_FEATURE_LEVEL           ms_featureLevel = D3D_FEATURE_LEVEL_11_0;
	bool                        ms_supportsTearing;

	DXGI_ADAPTER_DESC1          ms_adapterDescription;
	int                         ms_videoMemoryInMegabytes;
	int                         ms_shaderCapability;

	typedef std::vector<Direct3d11_Device::CallbackFunction> CallbackList;
	CallbackList                ms_deviceLostCallbacks;
	CallbackList                ms_deviceRestoredCallbacks;

	void                        selectAdapter();
	void                        createDevice();
	void                        queryCapabilities();
	void                        configureDebugLayer();
	void                        removeCallback(CallbackList &callbackList, Direct3d11_Device::CallbackFunction callbackFunction);
}
using namespace Direct3d11_DeviceNamespace;

// ======================================================================
/**
 * Turn an HRESULT into something a log reader can act on.
 *
 * Replaces DXGetErrorString9 from the DirectX 9 SDK, which this backend must
 * not link. Only the results this code can actually produce are named; anything
 * else is reported as a hex code, which is still better than a bare "failed".
 */

char const *Direct3d11_Device::describeHresult(HRESULT hresult)
{
	switch (hresult)
	{
		case S_OK:                              return "S_OK";
		case E_FAIL:                            return "E_FAIL";
		case E_INVALIDARG:                      return "E_INVALIDARG";
		case E_OUTOFMEMORY:                     return "E_OUTOFMEMORY";
		case E_NOTIMPL:                         return "E_NOTIMPL";
		case DXGI_ERROR_DEVICE_HUNG:            return "DXGI_ERROR_DEVICE_HUNG";
		case DXGI_ERROR_DEVICE_REMOVED:         return "DXGI_ERROR_DEVICE_REMOVED";
		case DXGI_ERROR_DEVICE_RESET:           return "DXGI_ERROR_DEVICE_RESET";
		case DXGI_ERROR_DRIVER_INTERNAL_ERROR:  return "DXGI_ERROR_DRIVER_INTERNAL_ERROR";
		case DXGI_ERROR_INVALID_CALL:           return "DXGI_ERROR_INVALID_CALL";
		case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:return "DXGI_ERROR_NOT_CURRENTLY_AVAILABLE";
		case DXGI_ERROR_NOT_FOUND:              return "DXGI_ERROR_NOT_FOUND";
		case DXGI_ERROR_UNSUPPORTED:            return "DXGI_ERROR_UNSUPPORTED";
		case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS: return "D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS";
		case D3D11_ERROR_FILE_NOT_FOUND:        return "D3D11_ERROR_FILE_NOT_FOUND";
		default:                                break;
	}

	static char buffer[32];
	sprintf(buffer, "0x%08lx", static_cast<unsigned long>(hresult));
	return buffer;
}

// ----------------------------------------------------------------------
/**
 * Treat a lost device as fatal, because it is.
 *
 * DX11 has no Reset() and no lost-device state to recover from: a removed
 * device invalidates every resource this backend owns and every handle the
 * engine is holding. Continuing produces a cascade of failures whose first
 * cause is long gone by the time anything is reported.
 */

void Direct3d11_Device::checkForDeviceRemoved(HRESULT hresult, char const *what)
{
	if (hresult != DXGI_ERROR_DEVICE_REMOVED && hresult != DXGI_ERROR_DEVICE_RESET)
		return;

	HRESULT const reason = ms_device ? ms_device->GetDeviceRemovedReason() : hresult;
	FATAL(true, ("Direct3d11: the graphics device was removed during %s (%s, reason %s). This is usually a driver reset, a TDR, or the adapter being disabled.", what, describeHresult(hresult), describeHresult(reason)));
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void Direct3d11_Device::assertMainThread(char const *what)
{
	DEBUG_FATAL(!Os::isMainThread(), ("Direct3d11: %s touched the immediate context from a thread other than the main thread. The context is not thread safe.", what));
}
#endif

// ======================================================================
/**
 * Pick the adapter to render on.
 *
 * [Direct3d11] adapter selects one by index; -1 takes the one DXGI enumerates
 * first, which is the one Windows considers primary.
 */

void Direct3d11_DeviceNamespace::selectAdapter()
{
	int const requested = ConfigDirect3d11::getAdapter();
	UINT const index = (requested < 0) ? 0 : static_cast<UINT>(requested);

	HRESULT hresult = ms_factory->EnumAdapters1(index, &ms_adapter);
	if (FAILED(hresult) && requested >= 0)
	{
		WARNING(true, ("Direct3d11: [Direct3d11] adapter %d does not exist (%s), falling back to adapter 0.", requested, Direct3d11_Device::describeHresult(hresult)));
		hresult = ms_factory->EnumAdapters1(0, &ms_adapter);
	}
	FATAL(FAILED(hresult) || !ms_adapter, ("Direct3d11: no usable display adapter (%s).", Direct3d11_Device::describeHresult(hresult)));

	Zero(ms_adapterDescription);
	hresult = ms_adapter->GetDesc1(&ms_adapterDescription);
	FATAL(FAILED(hresult), ("Direct3d11: IDXGIAdapter1::GetDesc1 failed (%s).", Direct3d11_Device::describeHresult(hresult)));

	// The description is wide; the log and the crash report are not.
	char description[128];
	int const converted = WideCharToMultiByte(CP_ACP, 0, ms_adapterDescription.Description, -1, description, sizeof(description), NULL, NULL);
	if (converted <= 0)
		strcpy(description, "<unnameable adapter>");

	bool const verbose = ConfigSharedFoundation::getVerboseHardwareLogging();
	REPORT_LOG(verbose, ("Direct3d11 adapter %u: %s (vendor 0x%04x device 0x%04x)\n", index, description, ms_adapterDescription.VendorId, ms_adapterDescription.DeviceId));
	CrashReportInformation::addStaticText("D3d11 adapter: %s (vendor 0x%04x device 0x%04x)\n", description, ms_adapterDescription.VendorId, ms_adapterDescription.DeviceId);
}

// ----------------------------------------------------------------------

void Direct3d11_DeviceNamespace::createDevice()
{
	// 11_1 first for VSSetConstantBuffers1, which lets a constant-buffer ring be
	// bound at an offset instead of renamed per draw. 11_0 is the floor: below it
	// the shader model the assets need is not available, and there is no useful
	// fallback to offer.
	D3D_FEATURE_LEVEL levels[2] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
	UINT levelCount = 2;

	switch (ConfigDirect3d11::getFeatureLevelCap())
	{
		case 110:
			levels[0] = D3D_FEATURE_LEVEL_11_0;
			levelCount = 1;
			WARNING(true, ("Direct3d11: [Direct3d11] featureLevelCap pins this run to feature level 11_0."));
			break;

		case 111:
			levelCount = 1;
			break;

		default:
			break;
	}

	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_UNKNOWN;
	switch (ConfigDirect3d11::getDriverType())
	{
		case ConfigDirect3d11::DT_warp:      driverType = D3D_DRIVER_TYPE_WARP;      break;
		case ConfigDirect3d11::DT_reference: driverType = D3D_DRIVER_TYPE_REFERENCE; break;
		default:                             driverType = D3D_DRIVER_TYPE_UNKNOWN;   break;
	}

	// A driver type other than UNKNOWN cannot be combined with an adapter.
	IDXGIAdapter1 * const adapter = (driverType == D3D_DRIVER_TYPE_UNKNOWN) ? ms_adapter : NULL;

	// No SINGLETHREADED: see the header. BGRA_SUPPORT because the UI and the
	// swap chain both want the B8G8R8A8 order D3D9 used.
	UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	if (ConfigDirect3d11::getDebugLayer())
		flags |= D3D11_CREATE_DEVICE_DEBUG;

	ID3D11Device        *device = NULL;
	ID3D11DeviceContext *context = NULL;

	HRESULT hresult = D3D11CreateDevice(adapter, driverType, NULL, flags, levels, levelCount, D3D11_SDK_VERSION, &device, &ms_featureLevel, &context);

	if (FAILED(hresult) && (flags & D3D11_CREATE_DEVICE_DEBUG))
	{
		// The debug layer needs the Graphics Tools optional feature installed. Its
		// absence must not stop a developer build from running, but it must be
		// said out loud, because a run without it does not validate anything.
		WARNING(true, ("Direct3d11: device creation with the debug layer failed (%s). The Graphics Tools feature is probably not installed. Retrying without validation -- this run checks nothing.", Direct3d11_Device::describeHresult(hresult)));
		flags &= ~D3D11_CREATE_DEVICE_DEBUG;
		hresult = D3D11CreateDevice(adapter, driverType, NULL, flags, levels, levelCount, D3D11_SDK_VERSION, &device, &ms_featureLevel, &context);
	}

	FATAL(FAILED(hresult), ("Direct3d11: D3D11CreateDevice failed (%s). This client requires a Direct3D 11 feature level 11_0 adapter.", Direct3d11_Device::describeHresult(hresult)));
	FATAL(ms_featureLevel < D3D_FEATURE_LEVEL_11_0, ("Direct3d11: the adapter reported feature level 0x%04x, below the required 11_0.", static_cast<unsigned>(ms_featureLevel)));

	hresult = device->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void **>(&ms_device));
	FATAL(FAILED(hresult) || !ms_device, ("Direct3d11: ID3D11Device1 is unavailable (%s).", Direct3d11_Device::describeHresult(hresult)));

	hresult = context->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void **>(&ms_context));
	FATAL(FAILED(hresult) || !ms_context, ("Direct3d11: ID3D11DeviceContext1 is unavailable (%s).", Direct3d11_Device::describeHresult(hresult)));

	device->Release();
	context->Release();

	bool const verbose = ConfigSharedFoundation::getVerboseHardwareLogging();
	REPORT_LOG(verbose, ("Direct3d11 feature level 0x%04x\n", static_cast<unsigned>(ms_featureLevel)));
	CrashReportInformation::addStaticText("D3d11 featureLevel: 0x%04x\n", static_cast<unsigned>(ms_featureLevel));
}

// ----------------------------------------------------------------------

void Direct3d11_DeviceNamespace::queryCapabilities()
{
	// Resource creation off the main thread is a requirement, not an
	// optimisation, so a driver that cannot do it needs to say so here rather
	// than corrupt its heap later.
	Zero(ms_threadingSupport);
	HRESULT const hresult = ms_device->CheckFeatureSupport(D3D11_FEATURE_THREADING, &ms_threadingSupport, sizeof(ms_threadingSupport));
	if (SUCCEEDED(hresult))
		WARNING(!ms_threadingSupport.DriverConcurrentCreates, ("Direct3d11: this driver reports it cannot create resources concurrently. Asset loading happens on the AsynchronousLoader thread, so expect trouble."));

	// Constant buffer offsetting and no-overwrite mapping. Both are exposed
	// through the 11_1 interface but are optional on 11_0 hardware, and the
	// per-draw constant design needs both: offsetting to bind a slice of a ring,
	// no-overwrite to append to it without renaming the buffer. Queried here so
	// the constant buffers can pick their strategy once, at install, and report
	// which one they got.
	Zero(ms_options);
	IGNORE_RETURN(ms_device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS, &ms_options, sizeof(ms_options)));
	WARNING(!ms_options.ConstantBufferOffsetting || !ms_options.MapNoOverwriteOnDynamicConstantBuffer,
		("Direct3d11: this device lacks constant buffer offsetting (%d) or no-overwrite constant mapping (%d), so per-draw constants must use rotating buffers with DISCARD. Expect higher per-draw constant cost than a device with both.",
		ms_options.ConstantBufferOffsetting ? 1 : 0, ms_options.MapNoOverwriteOnDynamicConstantBuffer ? 1 : 0));

	// Tearing lets a benchmark run present without waiting for vblank. Without
	// it every frame-time percentile is pinned to the refresh rate.
	ms_supportsTearing = false;
	IDXGIFactory5 *factory5 = NULL;
	if (SUCCEEDED(ms_factory->QueryInterface(__uuidof(IDXGIFactory5), reinterpret_cast<void **>(&factory5))) && factory5)
	{
		BOOL allowTearing = FALSE;
		if (SUCCEEDED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
			ms_supportsTearing = (allowTearing != FALSE);
		factory5->Release();
	}
	WARNING(ConfigDirect3d11::getAllowTearing() && !ms_supportsTearing, ("Direct3d11: [Direct3d11] allowTearing is set but this system does not support it. Presents will wait for vblank, so frame times are quantised to the refresh rate."));

	// Video memory. DX9 does not ask Direct3D for this at all -- it creates a
	// DirectDraw object and reads DDCAPS.dwVidMemTotal, defaulting to 32 MB
	// (Direct3d9.cpp:1651-1657). That number gates real features in
	// SetupClientGraphics: DOT3 needs 40 MB, POST and HEAT need 100 MB. DXGI's
	// DedicatedVideoMemory is the honest answer, and on modern hardware it is
	// very likely LARGER than what DirectDraw reports, which means DX11 can
	// enable effects the DX9 build has been quietly running without. That is a
	// visible difference between the two backends and it is not a DX11 defect --
	// it has to be measured against gl05 on the same machine, not assumed.
	SIZE_T const dedicated = ms_adapterDescription.DedicatedVideoMemory;
	SIZE_T const shared    = ms_adapterDescription.SharedSystemMemory;
	SIZE_T const usable    = dedicated ? dedicated : shared;
	ms_videoMemoryInMegabytes = static_cast<int>(usable / (1024 * 1024));

	WARNING(ms_videoMemoryInMegabytes < 100, ("Direct3d11: only %d MB of video memory is reported, so SetupClientGraphics will disable POST and HEAT (and DOT3 below 40 MB).", ms_videoMemoryInMegabytes));

	bool const verbose = ConfigSharedFoundation::getVerboseHardwareLogging();
	REPORT_LOG(verbose, ("Direct3d11 video memory: %d MB dedicated, %d MB shared\n", static_cast<int>(dedicated / (1024 * 1024)), static_cast<int>(shared / (1024 * 1024))));
	CrashReportInformation::addStaticText("D3d11 videoMemory: %d MB\n", ms_videoMemoryInMegabytes);

	// Shader capability is not a description of the hardware, it is a key the
	// engine compares for EXACT equality against a value stored in every shader
	// effect asset, and 2.0 is the highest tier any shipped asset was authored
	// for. Reporting anything else does not degrade the image; it stops shader
	// implementations being selected at all.
	int const override = ConfigDirect3d11::getShaderCapabilityOverride();
	ms_shaderCapability = override ? override : ShaderCapability(2, 0);

	REPORT_LOG(verbose, ("Direct3d11 shader capability %d.%d\n", GetShaderCapabilityMajor(ms_shaderCapability), GetShaderCapabilityMinor(ms_shaderCapability)));
	CrashReportInformation::addStaticText("ShaderCapability: %d.%d\n", GetShaderCapabilityMajor(ms_shaderCapability), GetShaderCapabilityMinor(ms_shaderCapability));
}

// ----------------------------------------------------------------------
/**
 * Break on anything the debug layer considers our fault.
 *
 * Corruption and errors stop the process where they happen rather than being
 * discovered as a wrong image later. Warnings are logged, not broken on: the
 * flip model produces a benign one about the back buffer being unbound at
 * Present, and breaking on that would make the debug layer unusable.
 */

void Direct3d11_DeviceNamespace::configureDebugLayer()
{
	if (!ConfigDirect3d11::getDebugLayer())
		return;

	ID3D11InfoQueue *infoQueue = NULL;
	if (FAILED(ms_device->QueryInterface(__uuidof(ID3D11InfoQueue), reinterpret_cast<void **>(&infoQueue))) || !infoQueue)
		return;

	// Corruption still breaks -- there is nothing useful to do after it. Errors are logged rather
	// than broken on, which is a change from how this was first written.
	//
	// The original intent was that an error should stop the process where it happens instead of
	// being discovered later as a wrong image. That is right in a debugger and useless without one:
	// the break arrives as a bare breakpoint exception and the message explaining it stays in the
	// queue, unread. drainDebugMessages puts the text in the log, which is what actually made the
	// black-screen investigation tractable.
	IGNORE_RETURN(infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE));
	IGNORE_RETURN(infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, FALSE));

	D3D11_MESSAGE_ID denied[] =
	{
		D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET,
	};

	D3D11_INFO_QUEUE_FILTER filter;
	Zero(filter);
	filter.DenyList.NumIDs = sizeof(denied) / sizeof(denied[0]);
	filter.DenyList.pIDList = denied;
	IGNORE_RETURN(infoQueue->AddStorageFilterEntries(&filter));

	infoQueue->Release();

	WARNING(true, ("Direct3d11: the D3D11 debug layer is enabled. It costs frame time, so do not measure performance against this run."));
}

// ----------------------------------------------------------------------

void Direct3d11_Device::drainDebugMessages()
{
	if (!ConfigDirect3d11::getDebugLayer() || !ms_device)
		return;

	ID3D11InfoQueue *infoQueue = NULL;
	if (FAILED(ms_device->QueryInterface(__uuidof(ID3D11InfoQueue), reinterpret_cast<void **>(&infoQueue))) || !infoQueue)
		return;

	// Bounded per call and over the run. The layer will happily repeat the same complaint once per
	// draw, and a log with a hundred thousand identical lines answers nothing.
	static int totalReported = 0;
	int const cms_perCall = 24;
	int const cms_perRun  = 400;

	UINT64 const available = infoQueue->GetNumStoredMessages();

	for (UINT64 i = 0; i < available && i < static_cast<UINT64>(cms_perCall); ++i)
	{
		SIZE_T length = 0;
		if (FAILED(infoQueue->GetMessage(i, NULL, &length)) || !length)
			continue;

		D3D11_MESSAGE * const message = static_cast<D3D11_MESSAGE *>(operator new(length));
		if (!message)
			continue;

		if (SUCCEEDED(infoQueue->GetMessage(i, message, &length)) && totalReported < cms_perRun)
		{
			++totalReported;
			WARNING(true, ("Direct3d11 debug layer [%d/%d]: %s",
				static_cast<int>(message->Severity),
				static_cast<int>(message->ID),
				message->pDescription ? message->pDescription : "<no description>"));
		}

		operator delete(message);
	}

	infoQueue->ClearStoredMessages();
	infoQueue->Release();

	if (totalReported >= cms_perRun)
	{
		static bool reportedCap = false;
		if (!reportedCap)
		{
			reportedCap = true;
			WARNING(true, ("Direct3d11: %d debug layer messages reported; further ones are dropped.", cms_perRun));
		}
	}
}

// ======================================================================

bool Direct3d11_Device::install()
{
	DEBUG_FATAL(ms_installed, ("Direct3d11_Device::install called twice"));

	UINT factoryFlags = 0;
	if (ConfigDirect3d11::getDebugLayer())
		factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

	HRESULT hresult = CreateDXGIFactory2(factoryFlags, __uuidof(IDXGIFactory2), reinterpret_cast<void **>(&ms_factory));
	if (FAILED(hresult) && factoryFlags)
		hresult = CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), reinterpret_cast<void **>(&ms_factory));
	FATAL(FAILED(hresult) || !ms_factory, ("Direct3d11: CreateDXGIFactory2 failed (%s).", describeHresult(hresult)));

	selectAdapter();
	createDevice();
	queryCapabilities();
	configureDebugLayer();

	ms_installed = true;
	return true;
}

// ----------------------------------------------------------------------
/**
 * Release everything, tolerating any state including never installed.
 *
 * This runs from the ExitChain entry clientGraphics registers as critical
 * before the DLL is even loaded, which means it also runs while the process is
 * fataling -- and on that path the engine's own non-critical Texture and vertex
 * buffer teardown is skipped, so engine objects may still be holding resources
 * created here. Releasing our references anyway is correct; what is not
 * acceptable is failing, because a FATAL inside a FATAL loses the original
 * diagnostic.
 */

void Direct3d11_Device::remove()
{
	ms_deviceLostCallbacks.clear();
	ms_deviceRestoredCallbacks.clear();

	if (ms_context)
	{
		ms_context->ClearState();
		ms_context->Flush();
		ms_context->Release();
		ms_context = NULL;
	}

	if (ms_device)
	{
		ms_device->Release();
		ms_device = NULL;
	}

	if (ms_adapter)
	{
		ms_adapter->Release();
		ms_adapter = NULL;
	}

	if (ms_factory)
	{
		ms_factory->Release();
		ms_factory = NULL;
	}

	ms_installed = false;
}

// ----------------------------------------------------------------------

bool Direct3d11_Device::isInstalled()
{
	return ms_installed;
}

// ----------------------------------------------------------------------

ID3D11Device1 *Direct3d11_Device::getDevice()
{
	return ms_device;
}

// ----------------------------------------------------------------------

ID3D11DeviceContext1 *Direct3d11_Device::getContext()
{
	return ms_context;
}

// ----------------------------------------------------------------------

IDXGIFactory2 *Direct3d11_Device::getFactory()
{
	return ms_factory;
}

// ----------------------------------------------------------------------

IDXGIAdapter1 *Direct3d11_Device::getAdapter()
{
	return ms_adapter;
}

// ----------------------------------------------------------------------

D3D_FEATURE_LEVEL Direct3d11_Device::getFeatureLevel()
{
	return ms_featureLevel;
}

// ----------------------------------------------------------------------

bool Direct3d11_Device::supportsTearing()
{
	return ms_supportsTearing;
}

// ----------------------------------------------------------------------

bool Direct3d11_Device::supportsConstantBufferOffsetting()
{
	return ms_options.ConstantBufferOffsetting != FALSE;
}

// ----------------------------------------------------------------------

bool Direct3d11_Device::supportsConstantBufferNoOverwrite()
{
	return ms_options.MapNoOverwriteOnDynamicConstantBuffer != FALSE;
}

// ----------------------------------------------------------------------

int Direct3d11_Device::getShaderCapability()
{
	return ms_shaderCapability;
}

// ----------------------------------------------------------------------

int Direct3d11_Device::getVideoMemoryInMegabytes()
{
	return ms_videoMemoryInMegabytes;
}

// ----------------------------------------------------------------------
/**
 * Report the desktop rectangles of every monitor except this adapter's.
 *
 * Os installs this as a raw hook for the life of the process and calls it on
 * every WM_ENTERSIZEMOVE, so it must not depend on device state and must not
 * become expensive. DX9 skips whole adapters rather than individual monitors
 * (Direct3d9.cpp:4461-4479); the same shape is kept here, walking each other
 * adapter's outputs, since DXGI models several outputs per adapter where D3D9
 * modelled one monitor per adapter.
 */

void Direct3d11_Device::getOtherAdapterRects(stdvector<RECT>::fwd &otherAdapterRects)
{
	otherAdapterRects.clear();

	if (!ms_factory)
		return;

	LUID ours;
	Zero(ours);
	if (ms_adapter)
		ours = ms_adapterDescription.AdapterLuid;

	for (UINT adapterIndex = 0; ; ++adapterIndex)
	{
		IDXGIAdapter1 *adapter = NULL;
		if (FAILED(ms_factory->EnumAdapters1(adapterIndex, &adapter)) || !adapter)
			break;

		DXGI_ADAPTER_DESC1 description;
		Zero(description);
		bool skip = false;
		if (SUCCEEDED(adapter->GetDesc1(&description)))
			skip = (description.AdapterLuid.LowPart == ours.LowPart && description.AdapterLuid.HighPart == ours.HighPart);

		if (!skip)
			for (UINT outputIndex = 0; ; ++outputIndex)
			{
				IDXGIOutput *output = NULL;
				if (FAILED(adapter->EnumOutputs(outputIndex, &output)) || !output)
					break;

				DXGI_OUTPUT_DESC outputDescription;
				Zero(outputDescription);
				if (SUCCEEDED(output->GetDesc(&outputDescription)))
					otherAdapterRects.push_back(outputDescription.DesktopCoordinates);

				output->Release();
			}

		adapter->Release();
	}
}

// ----------------------------------------------------------------------

bool Direct3d11_Device::isSampleCountSupported(int sampleCount)
{
	if (sampleCount <= 1)
		return true;

	if (!ms_device)
		return false;

	UINT qualityLevels = 0;
	if (FAILED(ms_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, static_cast<UINT>(sampleCount), &qualityLevels)))
		return false;

	return qualityLevels > 0;
}

// ----------------------------------------------------------------------

bool Direct3d11_Device::supportsAntialias()
{
	// Answered from the hardware rather than from the config, because the engine
	// asks whether the option can be offered, not whether it is on.
	return isSampleCountSupported(2);
}

// ======================================================================

void Direct3d11_Device::addDeviceLostCallback(CallbackFunction callbackFunction)
{
	NOT_NULL(callbackFunction);
	ms_deviceLostCallbacks.push_back(callbackFunction);
}

// ----------------------------------------------------------------------

void Direct3d11_Device::removeDeviceLostCallback(CallbackFunction callbackFunction)
{
	removeCallback(ms_deviceLostCallbacks, callbackFunction);
}

// ----------------------------------------------------------------------

void Direct3d11_Device::addDeviceRestoredCallback(CallbackFunction callbackFunction)
{
	NOT_NULL(callbackFunction);
	ms_deviceRestoredCallbacks.push_back(callbackFunction);
}

// ----------------------------------------------------------------------

void Direct3d11_Device::removeDeviceRestoredCallback(CallbackFunction callbackFunction)
{
	removeCallback(ms_deviceRestoredCallbacks, callbackFunction);
}

// ----------------------------------------------------------------------

void Direct3d11_DeviceNamespace::removeCallback(CallbackList &callbackList, Direct3d11_Device::CallbackFunction callbackFunction)
{
	for (CallbackList::iterator i = callbackList.begin(); i != callbackList.end(); ++i)
		if (*i == callbackFunction)
		{
			IGNORE_RETURN(callbackList.erase(i));
			return;
		}

	DEBUG_WARNING(true, ("Direct3d11: a device callback was removed that was never added."));
}

// ----------------------------------------------------------------------

void Direct3d11_Device::fireDeviceLost()
{
	for (CallbackList::const_iterator i = ms_deviceLostCallbacks.begin(); i != ms_deviceLostCallbacks.end(); ++i)
		(*i)();
}

// ----------------------------------------------------------------------

void Direct3d11_Device::fireDeviceRestored()
{
	for (CallbackList::const_iterator i = ms_deviceRestoredCallbacks.begin(); i != ms_deviceRestoredCallbacks.end(); ++i)
		(*i)();
}

// ======================================================================
