// ======================================================================
//
// Direct3d11_QueryPool.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_QueryPool.h"

#include "Direct3d11_Device.h"

// ======================================================================

namespace Direct3d11_QueryPoolNamespace
{
	// Three frames deep. Two is enough for the GPU to be a frame behind; three
	// leaves room for a driver that buffers more without ever making the CPU wait.
	int const cms_frameCount = 3;

	struct FrameQueries
	{
		ID3D11Query *disjoint;
		ID3D11Query *startTimestamp;
		ID3D11Query *endTimestamp;
		bool         inFlight;
	};

	FrameQueries  ms_frames[cms_frameCount];
	int           ms_currentFrame;
	bool          ms_installed;

	float         ms_gpuFrameTimeMilliseconds = -1.0f;
	bool          ms_lastFrameDisjoint;

	void          collect(FrameQueries &frame);
}
using namespace Direct3d11_QueryPoolNamespace;

// ======================================================================

bool Direct3d11_QueryPool::install()
{
	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	NOT_NULL(device);

	for (int i = 0; i < cms_frameCount; ++i)
	{
		D3D11_QUERY_DESC description;
		Zero(description);

		description.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
		if (FAILED(device->CreateQuery(&description, &ms_frames[i].disjoint)))
		{
			WARNING(true, ("Direct3d11: GPU timing is unavailable -- a disjoint query could not be created."));
			return false;
		}

		description.Query = D3D11_QUERY_TIMESTAMP;
		if (FAILED(device->CreateQuery(&description, &ms_frames[i].startTimestamp)) ||
		    FAILED(device->CreateQuery(&description, &ms_frames[i].endTimestamp)))
		{
			WARNING(true, ("Direct3d11: GPU timing is unavailable -- a timestamp query could not be created."));
			return false;
		}
	}

	ms_installed = true;
	return true;
}

// ----------------------------------------------------------------------

void Direct3d11_QueryPool::remove()
{
	for (int i = 0; i < cms_frameCount; ++i)
	{
		if (ms_frames[i].endTimestamp)   { ms_frames[i].endTimestamp->Release();   ms_frames[i].endTimestamp = NULL; }
		if (ms_frames[i].startTimestamp) { ms_frames[i].startTimestamp->Release(); ms_frames[i].startTimestamp = NULL; }
		if (ms_frames[i].disjoint)       { ms_frames[i].disjoint->Release();       ms_frames[i].disjoint = NULL; }
		ms_frames[i].inFlight = false;
	}

	ms_installed = false;
}

// ----------------------------------------------------------------------

void Direct3d11_QueryPool::beginFrame()
{
	if (!ms_installed)
		return;

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	// Advance first, then read whatever the slot we are about to reuse was
	// holding. That slot is cms_frameCount frames old, so its results are ready
	// and nothing has to wait for them.
	ms_currentFrame = (ms_currentFrame + 1) % cms_frameCount;
	FrameQueries &frame = ms_frames[ms_currentFrame];

	if (frame.inFlight)
		collect(frame);

	context->Begin(frame.disjoint);
	context->End(frame.startTimestamp);
}

// ----------------------------------------------------------------------

void Direct3d11_QueryPool::endFrame()
{
	if (!ms_installed)
		return;

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	FrameQueries &frame = ms_frames[ms_currentFrame];

	context->End(frame.endTimestamp);
	context->End(frame.disjoint);
	frame.inFlight = true;
}

// ----------------------------------------------------------------------
/**
 * Read a frame's results, if they are there.
 *
 * D3D11_ASYNC_GETDATA_DONOTFLUSH on every call: without it, GetData can flush
 * the command buffer, which changes the submission pattern being measured.
 */

void Direct3d11_QueryPoolNamespace::collect(FrameQueries &frame)
{
	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
	Zero(disjointData);
	if (context->GetData(frame.disjoint, &disjointData, sizeof(disjointData), D3D11_ASYNC_GETDATA_DONOTFLUSH) != S_OK)
		return;

	UINT64 startTime = 0;
	UINT64 endTime = 0;
	if (context->GetData(frame.startTimestamp, &startTime, sizeof(startTime), D3D11_ASYNC_GETDATA_DONOTFLUSH) != S_OK)
		return;
	if (context->GetData(frame.endTimestamp, &endTime, sizeof(endTime), D3D11_ASYNC_GETDATA_DONOTFLUSH) != S_OK)
		return;

	frame.inFlight = false;

	ms_lastFrameDisjoint = (disjointData.Disjoint != FALSE);

	// A disjoint frame spans a clock rate change, so its elapsed count does not
	// convert to time. Dropping it is correct; averaging it in is not.
	if (ms_lastFrameDisjoint || disjointData.Frequency == 0 || endTime <= startTime)
		return;

	ms_gpuFrameTimeMilliseconds = static_cast<float>(static_cast<double>(endTime - startTime) * 1000.0 / static_cast<double>(disjointData.Frequency));
}

// ----------------------------------------------------------------------

float Direct3d11_QueryPool::getGpuFrameTimeMilliseconds()
{
	return ms_gpuFrameTimeMilliseconds;
}

// ----------------------------------------------------------------------

bool Direct3d11_QueryPool::wasLastFrameDisjoint()
{
	return ms_lastFrameDisjoint;
}

// ======================================================================
