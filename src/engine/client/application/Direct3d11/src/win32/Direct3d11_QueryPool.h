// ======================================================================
//
// Direct3d11_QueryPool.h
// copyright (c) 2026 Galaxies Reborn
//
// GPU timing and pipeline statistics.
//
// There is not a single CreateQuery call anywhere in the first-party tree today,
// which means no GPU-side number has ever been measured for this renderer. Frame
// time alone cannot distinguish a CPU submission cost from a GPU fill cost, and
// two of the largest semantic changes in this port -- per-pixel fog and alpha
// test as a discard -- are GPU cost that CPU-side percentiles cannot see.
//
// Results are read three frames late, never waited on. A GetData that blocks the
// main thread would make the instrumentation itself the slowest thing in the
// frame, which is the classic way a measurement changes what it measures.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_QueryPool_H
#define INCLUDED_Direct3d11_QueryPool_H

// ======================================================================

class Direct3d11_QueryPool
{
public:

	static bool   install();
	static void   remove();

	static void   beginFrame();
	static void   endFrame();

	// Milliseconds of GPU time for the most recent frame whose results have
	// arrived, or -1 when none has yet. Never blocks.
	static float  getGpuFrameTimeMilliseconds();

	// True when the last completed frame's timestamps were disjoint, meaning the
	// GPU changed clock rate during it and the timing is not usable.
	static bool   wasLastFrameDisjoint();

private:

	Direct3d11_QueryPool();
	Direct3d11_QueryPool(Direct3d11_QueryPool const &);
	Direct3d11_QueryPool &operator =(Direct3d11_QueryPool const &);
};

// ======================================================================

#endif
