// ======================================================================
//
// BinkDLL.cpp
//
// Copyright 2001 - 2003 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/BinkDLL.h"

// ======================================================================

namespace Bink
{
	// ======================================================================

	static int       s_bindCount;
   static HINSTANCE s_hModule;

	// ======================================================================

	HBINK           (BINK_CALL *BinkOpen)                 (const char *file_name,BINK_OPEN_FLAGS open_flags);
	void            (BINK_CALL *BinkClose)                (HBINK bink);
	S32             (BINK_CALL *BinkDoFrame)              (HBINK bink);
	S32             (BINK_CALL *BinkCopyToBuffer)         (HBINK bink,VOID_PTR dest_addr,S32 dest_pitch,U32 dest_height,U32 dest_x,U32 dest_y,BINK_COPY_FLAGS copy_flags);
	S32             (BINK_CALL *BinkCopyToBufferRect)     (HBINK bink,VOID_PTR dest_addr,S32 dest_pitch,U32 dest_height,U32 dest_x,U32 dest_y,U32 src_x,U32 src_y,U32 src_w,U32 src_h,BINK_COPY_FLAGS copy_flags);
	void            (BINK_CALL *BinkNextFrame)            (HBINK bink);
	S32             (BINK_CALL *BinkWait)                 (HBINK bink);
	S32             (BINK_CALL *BinkPause)                (HBINK bink,S32 pause);
	S32             (BINK_CALL *BinkGetRects)             (HBINK bink,BINK_GETRECTS_FLAGS getrects_flags);
	void            (BINK_CALL *BinkGoto)                 (HBINK bink,U32 frame_num,BINK_GOTO_FLAGS goto_flags);
	void            (BINK_CALL *BinkGetFrameBuffersInfo)  (HBINK bink,BINKFRAMEBUFFERS *set);
	void            (BINK_CALL *BinkRegisterFrameBuffers) (HBINK bink,BINKFRAMEBUFFERS const *set);
	U32             (BINK_CALL *BinkGetKeyFrame)          (HBINK bink,U32 frame_num,BINK_GETKEY_FLAGS getkey_flags);
	S32             (BINK_CALL *BinkShouldSkip)           (HBINK bink);
	void            (BINK_CALL *BinkService)              (HBINK bink);
	void            (BINK_CALL *BinkSetVolume)            (HBINK bink,U32 trackID,S32 volume);
	void            (BINK_CALL *BinkSetPan)               (HBINK bink,U32 trackID,S32 pan);
	S32             (BINK_CALL *BinkSetVideoOnOff)        (HBINK bink,S32 on_off);
	S32             (BINK_CALL *BinkSetSoundOnOff)        (HBINK bink,S32 on_off);
	void            (BINK_CALL *BinkGetSummary)           (HBINK bink,BINKSUMMARY *summary);
	void            (BINK_CALL *BinkGetRealtime)          (HBINK bink,BINKREALTIME *real_time,U32 frame_window_length);
	S32             (BINK_CALL *BinkControlBackgroundIO)  (HBINK bink,BINK_BG_CONTROL_FLAGS control);
	U32             (BINK_CALL *BinkGetTrackID)           (HBINK bink,U32 track_index);
	HBINKTRACK      (BINK_CALL *BinkOpenTrack)            (HBINK bink,U32 track_index);
	void            (BINK_CALL *BinkCloseTrack)           (HBINKTRACK bink_track);
	U32             (BINK_CALL *BinkGetTrackData)         (HBINKTRACK bink_track,VOID_PTR dest_addr);
	void            (BINK_CALL *BinkSetFrameRate)         (U32 frame_rate,U32 frame_rate_div);
	void            (BINK_CALL *BinkSetSoundTrack)        (U32 track_count,u32_ptr track_id);
	void            (BINK_CALL *BinkSetSimulate)          (U32 sim_speed);
	void            (BINK_CALL *BinkSetIOSize)            (U32 io_size);
	void            (BINK_CALL *BinkSetIO)                (VOID_PTR BinkOpenFileFunction);
	void            (BINK_CALL *BinkSetMemory)            (BINKMEMALLOC mem_alloc,BINKMEMFREE mem_free);
	const char *    (BINK_CALL *BinkGetError)             ();
	VOID_PTR        (BINK_CALL *BinkLogoAddress)          ();
	S32             (BINK_CALL *BinkSetSoundSystem)       (VOID_PTR BinkOpenSoundSystemFunction,U32 param_value);
	BINKSNDOPEN     (BINK_CALL *BinkOpenMiles)            (U32);
	BINK_COPY_FLAGS (BINK_CALL *BinkDX9SurfaceType)       (LPDIRECT3DSURFACE9);
	U32             (BINK_CALL *RADTimerRead)             ();

	// ======================================================================

	static const void *_bind(const char * i_funcname)
	{
		if (!s_hModule) return 0;
		const void *const returnValue = GetProcAddress(s_hModule, i_funcname);
		DEBUG_FATAL(!returnValue, ("Failed to get %s procedure address from Bink DLL.", i_funcname));
		return returnValue;
	}

	// ======================================================================

	static void _destroy()
	{
		if (!s_hModule)
		{
			return;
		}

		// ---------------------------

		BinkOpen = 0;
		BinkClose = 0;
		BinkDoFrame = 0;
		BinkCopyToBuffer = 0;
		BinkCopyToBufferRect = 0;
		BinkNextFrame = 0;
		BinkWait = 0;
		BinkPause = 0;
		BinkGetRects = 0;
		BinkGoto = 0;
		BinkGetFrameBuffersInfo = 0;
		BinkRegisterFrameBuffers = 0;
		BinkGetKeyFrame = 0;
		BinkShouldSkip = 0;
		BinkService = 0;
		BinkSetVolume = 0;
		BinkSetPan = 0;
		BinkSetVideoOnOff = 0;
		BinkSetSoundOnOff = 0;
		BinkGetSummary = 0;
		BinkGetRealtime = 0;
		BinkControlBackgroundIO = 0;
		BinkGetTrackID = 0;
		BinkOpenTrack = 0;
		BinkCloseTrack = 0;
		BinkGetTrackData = 0;
		BinkSetFrameRate = 0;
		BinkSetSoundTrack = 0;
		BinkSetSimulate = 0;
		BinkSetIOSize = 0;
		BinkSetIO = 0;
		BinkSetMemory = 0;
		BinkGetError = 0;
		BinkLogoAddress = 0;
		BinkSetSoundSystem = 0;
		BinkOpenMiles = 0;
		BinkDX9SurfaceType = 0;
		RADTimerRead = 0;

		// ---------------------------

		FreeLibrary(s_hModule);
		s_hModule=0;
	}

// ======================================================================

	static bool _construct(const char * i_name)
	{
		if (s_hModule) {
			_destroy();
		}

		// ---------------------------

		s_hModule=LoadLibrary(i_name);
		if (s_hModule==0)
		{
			return false;
		}

		// ---------------------------

		BinkOpen = (HBINK (BINK_CALL *)(const char *,BINK_OPEN_FLAGS))_bind("_BinkOpen@8");
		BinkClose = (void (BINK_CALL *)(HBINK))_bind("_BinkClose@4");
		BinkDoFrame = (S32 (BINK_CALL *)(HBINK))_bind("_BinkDoFrame@4");
		BinkCopyToBuffer = (S32 (BINK_CALL *)(HBINK,VOID_PTR,S32,U32,U32,U32,BINK_COPY_FLAGS))_bind("_BinkCopyToBuffer@28");
		BinkCopyToBufferRect = (S32 (BINK_CALL *)(HBINK,VOID_PTR,S32,U32,U32,U32,U32,U32,U32,U32,BINK_COPY_FLAGS))_bind("_BinkCopyToBufferRect@44");
		BinkNextFrame = (void (BINK_CALL *)(HBINK))_bind("_BinkNextFrame@4");
		BinkWait = (S32 (BINK_CALL *)(HBINK))_bind("_BinkWait@4");
		BinkPause = (S32 (BINK_CALL *)(HBINK,S32))_bind("_BinkPause@8");
		BinkGetRects = (S32 (BINK_CALL *)(HBINK,BINK_GETRECTS_FLAGS))_bind("_BinkGetRects@8");
		BinkGoto = (void (BINK_CALL *)(HBINK,U32,BINK_GOTO_FLAGS))_bind("_BinkGoto@12");
		BinkGetFrameBuffersInfo = (void (BINK_CALL *)(HBINK,BINKFRAMEBUFFERS *))_bind("_BinkGetFrameBuffersInfo@8");
		BinkRegisterFrameBuffers = (void (BINK_CALL *)(HBINK,BINKFRAMEBUFFERS const *))_bind("_BinkRegisterFrameBuffers@8");
		BinkGetKeyFrame = (U32 (BINK_CALL *)(HBINK,U32,BINK_GETKEY_FLAGS))_bind("_BinkGetKeyFrame@12");
		BinkShouldSkip = (S32 (BINK_CALL *)(HBINK))_bind("_BinkShouldSkip@4");
		BinkService = (void (BINK_CALL *)(HBINK))_bind("_BinkService@4");
		BinkSetVolume = (void (BINK_CALL *)(HBINK,U32,S32))_bind("_BinkSetVolume@12");
		BinkSetPan = (void (BINK_CALL *)(HBINK,U32,S32))_bind("_BinkSetPan@12");
		BinkSetVideoOnOff = (S32 (BINK_CALL *)(HBINK,S32))_bind("_BinkSetVideoOnOff@8");
		BinkSetSoundOnOff = (S32 (BINK_CALL *)(HBINK,S32))_bind("_BinkSetSoundOnOff@8");
		BinkGetSummary = (void (BINK_CALL *)(HBINK,BINKSUMMARY *))_bind("_BinkGetSummary@8");
		BinkGetRealtime = (void (BINK_CALL *)(HBINK,BINKREALTIME *,U32))_bind("_BinkGetRealtime@12");
		BinkControlBackgroundIO = (S32 (BINK_CALL *)(HBINK,BINK_BG_CONTROL_FLAGS))_bind("_BinkControlBackgroundIO@8");
		BinkGetTrackID = (U32 (BINK_CALL *)(HBINK,U32))_bind("_BinkGetTrackID@8");
		BinkOpenTrack = (HBINKTRACK (BINK_CALL *)(HBINK,U32))_bind("_BinkOpenTrack@8");
		BinkCloseTrack = (void (BINK_CALL *)(HBINKTRACK))_bind("_BinkCloseTrack@4");
		BinkGetTrackData = (U32 (BINK_CALL *)(HBINKTRACK,VOID_PTR))_bind("_BinkGetTrackData@8");
		BinkSetFrameRate = (void (BINK_CALL *)(U32,U32))_bind("_BinkSetFrameRate@8");
		BinkSetSoundTrack = (void (BINK_CALL *)(U32,u32_ptr))_bind("_BinkSetSoundTrack@8");
		BinkSetSimulate = (void (BINK_CALL *)(U32))_bind("_BinkSetSimulate@4");
		BinkSetIOSize = (void (BINK_CALL *)(U32))_bind("_BinkSetIOSize@4");
		BinkSetIO = (void (BINK_CALL *)(VOID_PTR))_bind("_BinkSetIO@4");
		BinkSetMemory = (void (BINK_CALL *)(BINKMEMALLOC,BINKMEMFREE))_bind("_BinkSetMemory@8");
		BinkGetError = (const char *(BINK_CALL *)())_bind("_BinkGetError@0");
		BinkLogoAddress = (VOID_PTR (BINK_CALL *)())_bind("_BinkLogoAddress@0");
		BinkSetSoundSystem = (S32 (BINK_CALL *)(VOID_PTR,U32))_bind("_BinkSetSoundSystem@8");
		BinkOpenMiles = (BINKSNDOPEN (BINK_CALL *)(U32))_bind("_BinkOpenMiles@4");
		BinkDX9SurfaceType = (BINK_COPY_FLAGS (BINK_CALL *)(LPDIRECT3DSURFACE9))_bind("_BinkDX9SurfaceType@4");
		RADTimerRead = (U32 (BINK_CALL *)())_bind("_RADTimerRead@0");

		// ---------------------------

		return true;
	}

	// ==========================================================

	bool bindBink(const char *i_dllname)
	{
		if (!s_bindCount)
		{
			if (!_construct(i_dllname))
			{
				return false;
			}
		}

		s_bindCount++;
		return true;
	}

	// ==========================================================

   void unbindBink()
	{
		if (s_bindCount<=0 || --s_bindCount>0)
		{
			return;
		}
		_destroy();
		s_bindCount--;
	}

	// ==========================================================

	bool isBinkReady()
	{ 
		return s_hModule!=0; 
	}

	// ==========================================================
}

// ======================================================================
