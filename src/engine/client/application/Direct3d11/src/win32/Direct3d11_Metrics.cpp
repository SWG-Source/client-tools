// ======================================================================
//
// Direct3d11_Metrics.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_Metrics.h"

#include "Direct3d11_QueryPool.h"

#include "sharedDebug/DebugFlags.h"

// ======================================================================

int Direct3d11_Metrics::drawCalls;
int Direct3d11_Metrics::drawIndexedCalls;
int Direct3d11_Metrics::vertices;
int Direct3d11_Metrics::triangles;
int Direct3d11_Metrics::droppedDraws;
int Direct3d11_Metrics::unsatisfiableInputLayouts;

int Direct3d11_Metrics::inputLayoutBindCalls;
int Direct3d11_Metrics::inputLayoutBindMisses;
int Direct3d11_Metrics::vertexBufferBindCalls;
int Direct3d11_Metrics::vertexBufferBindMisses;
int Direct3d11_Metrics::indexBufferBindCalls;
int Direct3d11_Metrics::indexBufferBindMisses;
int Direct3d11_Metrics::vertexShaderBindCalls;
int Direct3d11_Metrics::vertexShaderBindMisses;
int Direct3d11_Metrics::pixelShaderBindCalls;
int Direct3d11_Metrics::pixelShaderBindMisses;
int Direct3d11_Metrics::blendStateBindCalls;
int Direct3d11_Metrics::blendStateBindMisses;
int Direct3d11_Metrics::depthStencilStateBindCalls;
int Direct3d11_Metrics::depthStencilStateBindMisses;
int Direct3d11_Metrics::rasterizerStateBindCalls;
int Direct3d11_Metrics::rasterizerStateBindMisses;
int Direct3d11_Metrics::samplerBindCalls;
int Direct3d11_Metrics::samplerBindMisses;
int Direct3d11_Metrics::shaderResourceBindCalls;
int Direct3d11_Metrics::shaderResourceBindMisses;
int Direct3d11_Metrics::viewportSetCalls;
int Direct3d11_Metrics::scissorSetCalls;
int Direct3d11_Metrics::setTransformCalls;

int Direct3d11_Metrics::constantBufferUpdates;
int Direct3d11_Metrics::constantBufferBytes;

int Direct3d11_Metrics::ringDiscards;
int Direct3d11_Metrics::ringNoOverwrites;
int Direct3d11_Metrics::ringBytes;

int Direct3d11_Metrics::renderTargetSwitches;
int Direct3d11_Metrics::textureBakeReadbacks;
int Direct3d11_Metrics::backBufferMaps;
int Direct3d11_Metrics::blockingStagingMaps;

int Direct3d11_Metrics::stateObjectCreations;
int Direct3d11_Metrics::inputLayoutCreations;
int Direct3d11_Metrics::constantBufferCreations;
int Direct3d11_Metrics::shaderCompiles;
int Direct3d11_Metrics::shaderCompileMicroseconds;
int Direct3d11_Metrics::backendShaderCompiles;
int Direct3d11_Metrics::vertexBufferCreations;
int Direct3d11_Metrics::indexBufferCreations;
int Direct3d11_Metrics::textureCreations;
int Direct3d11_Metrics::bufferCreateMicroseconds;
int Direct3d11_Metrics::textureCreateMicroseconds;
int Direct3d11_Metrics::textureUploadMicroseconds;
int Direct3d11_Metrics::drawPrepareMicroseconds;
int Direct3d11_Metrics::sceneMicroseconds;
int Direct3d11_Metrics::presentMicroseconds;
int Direct3d11_Metrics::maxLightsInList;
int Direct3d11_Metrics::lightBatches;
int Direct3d11_Metrics::lightBatchesWithLights;
int Direct3d11_Metrics::litBatches;
int Direct3d11_Metrics::maxAmbientMilli;
int Direct3d11_Metrics::shaderCacheHits;
int Direct3d11_Metrics::shaderCacheMisses;

int Direct3d11_Metrics::compositesCopied;
int Direct3d11_Metrics::compositesCorrected;

int Direct3d11_Metrics::presents;
int Direct3d11_Metrics::presentFailures;

// ======================================================================

namespace Direct3d11_MetricsNamespace
{
	// Run totals, accumulated at each beginFrame from the per-frame counters.
	int  ms_runFrames;
	int  ms_runDrawCalls;
	int  ms_runTriangles;
	int  ms_runStateObjectCreations;
	int  ms_runInputLayoutCreations;
	int  ms_runConstantBufferCreations;
	int  ms_runShaderCompiles;
	int  ms_runDroppedDraws;
	int  ms_runUnsatisfiableInputLayouts;
	int  ms_runBackendShaderCompiles;
	int  ms_runShaderCacheHits;
	int  ms_runShaderCacheMisses;
	int  ms_runTextureBakeReadbacks;
	int  ms_runBlockingStagingMaps;
	int  ms_runPresentFailures;

	// Peak in-frame creation count seen after the first few frames. Warm-up is
	// expected to create things; a steady-state frame is not.
	int  ms_peakInFrameCreations;

	// The breakdown of the peak frame, and which frame it was. Kept alongside the total because
	// the total on its own does not say whether the answer is a warm-up pass or a broken cache.
	int  ms_peakInFrameFrame;
	int  ms_peakInFrameStateObjects;
	int  ms_peakInFrameInputLayouts;
	int  ms_peakInFrameConstantBuffers;
	int  ms_peakInFrameShaderCompiles;
	int  ms_peakInFrameCompileMicroseconds;
	int  ms_runCompileMicroseconds;
	int  const cms_warmUpFrames = 60;

	bool ms_reportRequested;

	// Start of the current beginScene..endScene interval, in QueryPerformanceCounter ticks.
	long long ms_sceneStart;

	void reportRoutine();
}
using namespace Direct3d11_MetricsNamespace;

// ======================================================================

void Direct3d11_Metrics::install()
{
	// A DebugFlags lever so a report can be asked for from the debug menu.
	// Registered with a report routine, which is the DLLEXPORT overload -- but
	// note the report routines are only driven by DebugFlags::callReportRoutines,
	// whose one call site in the game loop is inside a #if 0. Until that is
	// re-enabled this lever does nothing on its own, which is why the shutdown
	// summary below does not depend on it.
	DebugFlags::registerFlag(ms_reportRequested, "Direct3d11", "reportMetrics", reportRoutine);
}

// ----------------------------------------------------------------------

void Direct3d11_Metrics::remove()
{
	// Unconditional, and through WARNING rather than a debug-only channel: the
	// zero-invariant counters are worthless if they are only visible in a build
	// nobody measures.
	report();

	DebugFlags::unregisterFlag(ms_reportRequested);
}

// ----------------------------------------------------------------------

void Direct3d11_MetricsNamespace::reportRoutine()
{
	Direct3d11_Metrics::report();
}

// ----------------------------------------------------------------------

Direct3d11_Metrics::ScopedTimer::ScopedTimer(int &microseconds)
:
	m_microseconds(microseconds),
	m_start(0)
{
	LARGE_INTEGER now;
	if (QueryPerformanceCounter(&now))
		m_start = now.QuadPart;
}

// ----------------------------------------------------------------------

Direct3d11_Metrics::ScopedTimer::~ScopedTimer()
{
	LARGE_INTEGER now;
	LARGE_INTEGER frequency;
	if (m_start && QueryPerformanceCounter(&now) && QueryPerformanceFrequency(&frequency) && frequency.QuadPart)
		m_microseconds += static_cast<int>(((now.QuadPart - m_start) * 1000000) / frequency.QuadPart);
}

// ----------------------------------------------------------------------

void Direct3d11_Metrics::markSceneBegin()
{
	LARGE_INTEGER now;
	ms_sceneStart = QueryPerformanceCounter(&now) ? now.QuadPart : 0;
}

// ----------------------------------------------------------------------

void Direct3d11_Metrics::markSceneEnd()
{
	LARGE_INTEGER now;
	LARGE_INTEGER frequency;
	if (ms_sceneStart && QueryPerformanceCounter(&now) && QueryPerformanceFrequency(&frequency) && frequency.QuadPart)
		sceneMicroseconds += static_cast<int>(((now.QuadPart - ms_sceneStart) * 1000000) / frequency.QuadPart);

	ms_sceneStart = 0;
}

// ----------------------------------------------------------------------

void Direct3d11_Metrics::reportHitches()
{
	// Wall time between successive calls, which is a frame. QueryPerformanceCounter rather than the
	// engine clock so this measures the same thing a person perceives.
	static LARGE_INTEGER previous = { 0 };
	static LARGE_INTEGER frequency = { 0 };
	// Bounded, but generously: a hitch report is one line and the whole point is to catch the ones
	// that happen minutes into a session, not only at zone-in.
	static int reportsRemaining = 200;
	static int frameNumber = 0;

	if (frequency.QuadPart == 0 && !QueryPerformanceFrequency(&frequency))
		return;

	LARGE_INTEGER now;
	if (!QueryPerformanceCounter(&now))
		return;

	++frameNumber;

	if (previous.QuadPart != 0 && frameNumber > cms_warmUpFrames)
	{
		double const milliseconds = static_cast<double>(now.QuadPart - previous.QuadPart) * 1000.0 / static_cast<double>(frequency.QuadPart);

		// A frame-time distribution, because "stutter" and "low frame rate" feel similar and the
		// 40 ms gate below cannot tell them apart: a room that renders at a steady 33 ms trips it
		// never, and a room that alternates 12 ms and 45 ms trips it every other frame. One line per
		// window says which, and carries the average split so the answer is not just "slow".
		{
			int const cms_windowFrames = 600;

			static int windowFrames;
			static int windowUnder17;
			static int windowUnder25;
			static int windowUnder34;
			static int windowUnder50;
			static int windowOver50;
			static double windowMilliseconds;
			static double windowWorst;
			static double windowScene;
			static double windowPresent;

			++windowFrames;
			windowMilliseconds += milliseconds;
			if (milliseconds > windowWorst)
				windowWorst = milliseconds;
			windowScene += static_cast<double>(sceneMicroseconds) / 1000.0;
			windowPresent += static_cast<double>(presentMicroseconds) / 1000.0;

			if (milliseconds < 17.0)      ++windowUnder17;
			else if (milliseconds < 25.0) ++windowUnder25;
			else if (milliseconds < 34.0) ++windowUnder34;
			else if (milliseconds < 50.0) ++windowUnder50;
			else                          ++windowOver50;

			if (windowFrames >= cms_windowFrames)
			{
				double const window = static_cast<double>(windowFrames);
				double const averageMilliseconds = windowMilliseconds / window;
				double const averageScene = windowScene / window;
				double const averagePresent = windowPresent / window;

				WARNING(true, ("Direct3d11 FRAMES: last %d frames averaged %.1f ms (%.0f fps), worst %.1f ms; distribution <17 ms %d, <25 ms %d, <34 ms %d, <50 ms %d, >=50 ms %d; average %.1f ms in scene, %.1f ms in Present, %.1f ms outside this backend; GPU recently %.1f ms; %d draw(s) last frame; last frame had %d lit batch(es), highest scene ambient %.3f",
					windowFrames, averageMilliseconds, (averageMilliseconds > 0.0) ? (1000.0 / averageMilliseconds) : 0.0, windowWorst,
					windowUnder17, windowUnder25, windowUnder34, windowUnder50, windowOver50,
					averageScene, averagePresent, averageMilliseconds - averageScene - averagePresent,
					static_cast<double>(Direct3d11_QueryPool::getGpuFrameTimeMilliseconds()),
					drawCalls + drawIndexedCalls,
					litBatches, static_cast<double>(maxAmbientMilli) / 1000.0));

				windowFrames = 0;
				windowUnder17 = 0;
				windowUnder25 = 0;
				windowUnder34 = 0;
				windowUnder50 = 0;
				windowOver50 = 0;
				windowMilliseconds = 0.0;
				windowWorst = 0.0;
				windowScene = 0.0;
				windowPresent = 0.0;
			}
		}

		// 40 ms is a hitch anyone notices at 60 Hz and is well clear of ordinary variance.
		if (milliseconds > 40.0 && reportsRemaining > 0)
		{
			--reportsRemaining;
			// The split first, because it is what decides whether this backend is even the subject.
			// The GPU figure comes from the query ring and is therefore a few frames old; it says
			// what the GPU has recently been costing, not what this particular frame cost it.
			double const sceneMilliseconds = static_cast<double>(sceneMicroseconds) / 1000.0;
			double const presentMilliseconds = static_cast<double>(presentMicroseconds) / 1000.0;
			double const outsideMilliseconds = milliseconds - sceneMilliseconds - presentMilliseconds;

			WARNING(true, ("Direct3d11 HITCH: frame %d took %.1f ms -- %.1f ms in scene, %.1f ms in Present, %.1f ms outside this backend; GPU recently %.1f ms; at most %d light(s) in any batch, %d of %d batch(es) lit; %d draw(s) spent %.1f ms in prepareToDraw; %d constant upload(s) of %d byte(s), %d ring discard(s); bind misses layout %d vb %d ib %d vs %d ps %d blend %d depth %d rast %d samp %d srv %d; streamed in %d vertex buffer(s), %d index buffer(s), %d texture(s) taking %.1f ms (%.1f creating buffers, %.1f creating textures, %.1f uploading mips); created %d state object(s), %d input layout(s), %d constant buffer(s); compiled %d shader(s) in %.1f ms; %d blocking staging map(s), %d bake readback(s), %d render target switch(es), %d draw(s)",
				frameNumber, milliseconds,
				sceneMilliseconds, presentMilliseconds, outsideMilliseconds,
				static_cast<double>(Direct3d11_QueryPool::getGpuFrameTimeMilliseconds()),
				maxLightsInList, lightBatchesWithLights, lightBatches,
				drawCalls + drawIndexedCalls, static_cast<double>(drawPrepareMicroseconds) / 1000.0,
				constantBufferUpdates, constantBufferBytes, ringDiscards,
				inputLayoutBindMisses, vertexBufferBindMisses, indexBufferBindMisses,
				vertexShaderBindMisses, pixelShaderBindMisses, blendStateBindMisses,
				depthStencilStateBindMisses, rasterizerStateBindMisses,
				samplerBindMisses, shaderResourceBindMisses,
				vertexBufferCreations, indexBufferCreations, textureCreations,
				static_cast<double>(bufferCreateMicroseconds + textureCreateMicroseconds + textureUploadMicroseconds) / 1000.0,
				static_cast<double>(bufferCreateMicroseconds) / 1000.0,
				static_cast<double>(textureCreateMicroseconds) / 1000.0,
				static_cast<double>(textureUploadMicroseconds) / 1000.0,
				stateObjectCreations, inputLayoutCreations, constantBufferCreations,
				shaderCompiles, static_cast<double>(shaderCompileMicroseconds) / 1000.0,
				blockingStagingMaps, textureBakeReadbacks, renderTargetSwitches,
				drawCalls + drawIndexedCalls));
		}
	}

	previous = now;
}

// ----------------------------------------------------------------------

void Direct3d11_Metrics::beginFrame()
{
	reportHitches();

	++ms_runFrames;

	ms_runDrawCalls               += drawCalls + drawIndexedCalls;
	ms_runTriangles               += triangles;
	ms_runStateObjectCreations    += stateObjectCreations;
	ms_runInputLayoutCreations    += inputLayoutCreations;
	ms_runConstantBufferCreations += constantBufferCreations;
	ms_runShaderCompiles          += shaderCompiles;
	ms_runCompileMicroseconds     += shaderCompileMicroseconds;
	ms_runBackendShaderCompiles   += backendShaderCompiles;
	ms_runShaderCacheHits         += shaderCacheHits;
	ms_runShaderCacheMisses       += shaderCacheMisses;
	ms_runDroppedDraws            += droppedDraws;
	ms_runUnsatisfiableInputLayouts += unsatisfiableInputLayouts;
	ms_runTextureBakeReadbacks    += textureBakeReadbacks;
	ms_runBlockingStagingMaps     += blockingStagingMaps;
	ms_runPresentFailures         += presentFailures;

	int const inFrameCreations = getInFrameCreationCount();
	if (ms_runFrames > cms_warmUpFrames && inFrameCreations > ms_peakInFrameCreations)
	{
		ms_peakInFrameCreations = inFrameCreations;
		ms_peakInFrameFrame = ms_runFrames;
		ms_peakInFrameStateObjects = stateObjectCreations;
		ms_peakInFrameInputLayouts = inputLayoutCreations;
		ms_peakInFrameConstantBuffers = constantBufferCreations;
		ms_peakInFrameShaderCompiles = shaderCompiles;
		ms_peakInFrameCompileMicroseconds = shaderCompileMicroseconds;
	}

	drawCalls = 0;
	drawIndexedCalls = 0;
	vertices = 0;
	triangles = 0;
	droppedDraws = 0;
	unsatisfiableInputLayouts = 0;

	inputLayoutBindCalls = 0;
	inputLayoutBindMisses = 0;
	vertexBufferBindCalls = 0;
	vertexBufferBindMisses = 0;
	indexBufferBindCalls = 0;
	indexBufferBindMisses = 0;
	vertexShaderBindCalls = 0;
	vertexShaderBindMisses = 0;
	pixelShaderBindCalls = 0;
	pixelShaderBindMisses = 0;
	blendStateBindCalls = 0;
	blendStateBindMisses = 0;
	depthStencilStateBindCalls = 0;
	depthStencilStateBindMisses = 0;
	rasterizerStateBindCalls = 0;
	rasterizerStateBindMisses = 0;
	samplerBindCalls = 0;
	samplerBindMisses = 0;
	shaderResourceBindCalls = 0;
	shaderResourceBindMisses = 0;
	viewportSetCalls = 0;
	scissorSetCalls = 0;
	setTransformCalls = 0;

	constantBufferUpdates = 0;
	constantBufferBytes = 0;

	ringDiscards = 0;
	ringNoOverwrites = 0;
	ringBytes = 0;

	renderTargetSwitches = 0;
	textureBakeReadbacks = 0;
	backBufferMaps = 0;
	blockingStagingMaps = 0;

	stateObjectCreations = 0;
	inputLayoutCreations = 0;
	constantBufferCreations = 0;
	shaderCompiles = 0;
	shaderCompileMicroseconds = 0;
	backendShaderCompiles = 0;
	vertexBufferCreations = 0;
	indexBufferCreations = 0;
	textureCreations = 0;
	bufferCreateMicroseconds = 0;
	textureCreateMicroseconds = 0;
	textureUploadMicroseconds = 0;
	drawPrepareMicroseconds = 0;
	sceneMicroseconds = 0;
	presentMicroseconds = 0;
	maxLightsInList = 0;
	lightBatches = 0;
	lightBatchesWithLights = 0;
	litBatches = 0;
	maxAmbientMilli = 0;
	shaderCacheHits = 0;
	shaderCacheMisses = 0;

	compositesCopied = 0;
	compositesCorrected = 0;

	presentFailures = 0;
}

// ----------------------------------------------------------------------

int Direct3d11_Metrics::getInFrameCreationCount()
{
	return stateObjectCreations + inputLayoutCreations + constantBufferCreations + shaderCompiles + backendShaderCompiles;
}

// ----------------------------------------------------------------------

void Direct3d11_Metrics::report()
{
	WARNING(true, ("Direct3d11 metrics: %d frames, %d presents, %d draw calls, %d triangles",
		ms_runFrames, presents, ms_runDrawCalls, ms_runTriangles));

	WARNING(true, ("Direct3d11 metrics: creations over the run -- state objects %d, input layouts %d, constant buffers %d, shader compiles %d (plus %d this backend compiles itself at install)",
		ms_runStateObjectCreations, ms_runInputLayoutCreations, ms_runConstantBufferCreations, ms_runShaderCompiles, ms_runBackendShaderCompiles));

	// The gate. Anything nonzero here after warm-up means work is being created
	// inside the frame loop.
	WARNING(true, ("Direct3d11 metrics: peak creations in a single frame after warm-up: %d (required: 0) -- frame %d, %d state object(s), %d input layout(s), %d constant buffer(s), %d shader compile(s)",
		ms_peakInFrameCreations, ms_peakInFrameFrame,
		ms_peakInFrameStateObjects, ms_peakInFrameInputLayouts,
		ms_peakInFrameConstantBuffers, ms_peakInFrameShaderCompiles));

	WARNING((ms_runShaderCacheHits + ms_runShaderCacheMisses) != 0, ("Direct3d11 metrics: precompiled shader cache -- %d hit(s), %d miss(es).",
		ms_runShaderCacheHits, ms_runShaderCacheMisses));

	WARNING(ms_runShaderCompiles != 0, ("Direct3d11 metrics: shader compilation cost %.1f ms over the run for %d program(s), %.1f ms average; the peak frame spent %.1f ms in D3DCompile.",
		static_cast<double>(ms_runCompileMicroseconds) / 1000.0,
		ms_runShaderCompiles,
		static_cast<double>(ms_runCompileMicroseconds) / (1000.0 * static_cast<double>(ms_runShaderCompiles)),
		static_cast<double>(ms_peakInFrameCompileMicroseconds) / 1000.0));

	if (ms_runDroppedDraws || ms_runTextureBakeReadbacks || ms_runBlockingStagingMaps || ms_runPresentFailures)
		WARNING(true, ("Direct3d11 metrics: NONZERO INVARIANTS -- dropped draws %d, bake readbacks %d, blocking staging maps %d, present failures %d",
			ms_runDroppedDraws, ms_runTextureBakeReadbacks, ms_runBlockingStagingMaps, ms_runPresentFailures));

	WARNING(ms_runUnsatisfiableInputLayouts != 0, ("Direct3d11 metrics: %d draw(s) asked a vertex shader to read an input its vertex buffer does not carry. Shipping DX9 does not draw these either -- see Direct3d11_Metrics.h.", ms_runUnsatisfiableInputLayouts));

	WARNING(true, ("Direct3d11 metrics: composite -- %d copied, %d colour corrected", compositesCopied, compositesCorrected));
}

// ======================================================================
