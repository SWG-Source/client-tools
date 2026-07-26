// ======================================================================
//
// Direct3d11_Device.h
// copyright (c) 2026 Galaxies Reborn
//
// The D3D11 device, its DXGI factory and adapter, and everything the engine
// can ask about the hardware.
//
// Deliberately created WITHOUT D3D11_CREATE_DEVICE_SINGLETHREADED. Resource
// creation is not confined to the main thread: ShaderImplementation::load runs
// on whichever thread loaded the .eft, and ShaderTemplateList registers the
// 'sht' extension with the AsynchronousLoader, so createShaderImplementation-
// GraphicsData can arrive from the loader thread. D3D9 survived that because
// device creation is internally serialised; SINGLETHREADED would turn it into
// intermittent heap corruption. ID3D11Device::Create* is therefore treated as
// free-threaded, while the immediate context stays main-thread-only and says so
// with an assert on entry.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_Device_H
#define INCLUDED_Direct3d11_Device_H

// ======================================================================

#include <d3d11_1.h>
#include <dxgi1_5.h>

// ======================================================================

class Direct3d11_Device
{
public:

	typedef void (*CallbackFunction)();

public:

	static bool                    install();
	static void                    remove();
	static bool                    isInstalled();

	static ID3D11Device1          *getDevice();
	static ID3D11DeviceContext1   *getContext();
	static IDXGIFactory2          *getFactory();
	static IDXGIAdapter1          *getAdapter();
	static D3D_FEATURE_LEVEL       getFeatureLevel();
	static bool                    supportsTearing();

	// Two separate capabilities, both needed for a constant buffer to be used as
	// a ring rather than renamed per draw, and both optional on feature level 11_0
	// hardware even though the interface that exposes them is 11_1:
	//
	//   offsetting   bind a sub-range of one buffer with VSSetConstantBuffers1,
	//                so a per-draw slice costs an offset instead of an upload
	//   noOverwrite  Map a dynamic CONSTANT buffer WRITE_NO_OVERWRITE, so
	//                appending to the ring does not rename it
	//
	// Without both, per-draw constants have to fall back to rotating buffers with
	// WRITE_DISCARD. That is measurably worse, so it is reported rather than
	// silently adopted.
	static bool                    supportsConstantBufferOffsetting();
	static bool                    supportsConstantBufferNoOverwrite();

	// Gl_api surface this class answers directly.
	static int                     getShaderCapability();
	static int                     getVideoMemoryInMegabytes();
	static void                    getOtherAdapterRects(stdvector<RECT>::fwd &otherAdapterRects);
	static bool                    supportsAntialias();
	static bool                    isSampleCountSupported(int sampleCount);

	// The engine's device lost/restored registries. DX11 has no lost device, so
	// these fire only around a swap-chain resize, which is the one case the two
	// real subscribers -- BinkVideo and PostProcessingEffectsManager -- actually
	// need: their render targets are sized to the back buffer.
	static void                    addDeviceLostCallback(CallbackFunction callbackFunction);
	static void                    removeDeviceLostCallback(CallbackFunction callbackFunction);
	static void                    addDeviceRestoredCallback(CallbackFunction callbackFunction);
	static void                    removeDeviceRestoredCallback(CallbackFunction callbackFunction);
	static void                    fireDeviceLost();
	static void                    fireDeviceRestored();

	static char const             *describeHresult(HRESULT hresult);

	// Move whatever the D3D11 debug layer has queued into the warning log. No-op unless the
	// debugLayer config key is on.
	//
	// Storing messages and never reading them is the same as not enabling the layer: a break on
	// error in a build with no debugger attached gives a breakpoint exception and no text. Draining
	// them is what makes the layer answer questions.
	static void                    drainDebugMessages();
	static void                    checkForDeviceRemoved(HRESULT hresult, char const *what);

#ifdef _DEBUG
	static void                    assertMainThread(char const *what);
#endif

private:

	Direct3d11_Device();
	Direct3d11_Device(Direct3d11_Device const &);
	Direct3d11_Device &operator =(Direct3d11_Device const &);
};

// ======================================================================

// The immediate context is not thread safe and the engine does reach the
// backend from the AsynchronousLoader thread, so every entry point that touches
// it says which one it was when it is reached from elsewhere.
#ifdef _DEBUG
	#define DX11_ASSERT_MAIN_THREAD() Direct3d11_Device::assertMainThread(__FUNCTION__)
#else
	#define DX11_ASSERT_MAIN_THREAD() NOP
#endif

// ======================================================================

#endif
