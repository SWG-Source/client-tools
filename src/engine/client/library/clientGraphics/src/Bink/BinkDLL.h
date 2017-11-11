#include <d3d9.h>

#define BINK_CALL		__stdcall
#define BINK_STORAGE	extern

namespace Bink
{
	// ------------------------------------------

	bool bindBink(const char *i_dllname);
   void unbindBink();
	bool isBinkReady();

	// ------------------------------------------

	namespace RAD
	{
		#include "bink.h"
	}

	// ------------------------------------------

	typedef void *VOID_PTR;
	typedef U32 BINK_OPEN_FLAGS;
	typedef U32 BINK_COPY_FLAGS;
	typedef U32 BINK_GETRECTS_FLAGS;
	typedef U32 BINK_GOTO_FLAGS;
	typedef U32 BINK_GETKEY_FLAGS;
	typedef U32 BINK_BG_CONTROL_FLAGS;
	typedef U32 BINK_CONVERTER_FLAGS;
	typedef U32 *u32_ptr;
	typedef S32 *s32_ptr;

	using RAD::HBINK;
	using RAD::BINKFRAMEBUFFERS;
	using RAD::BINKSUMMARY;
	using RAD::BINKREALTIME;
	using RAD::HBINKTRACK;
	using RAD::BINKMEMALLOC;
	using RAD::BINKMEMFREE;

	// ------------------------------------------

	BINK_STORAGE HBINK (BINK_CALL *BinkOpen)(
	  const char *file_name,
	  BINK_OPEN_FLAGS open_flags
	);
	BINK_STORAGE void (BINK_CALL *BinkClose)(
	  HBINK bink
	);
	BINK_STORAGE S32 (BINK_CALL *BinkDoFrame)(
	  HBINK bink
	);
	BINK_STORAGE S32 (BINK_CALL *BinkCopyToBuffer)(
	  HBINK bink,
	  VOID_PTR dest_addr,
	  S32 dest_pitch,
	  U32 dest_height,
	  U32 dest_x,
	  U32 dest_y,
	  BINK_COPY_FLAGS copy_flags
	);
	BINK_STORAGE S32 (BINK_CALL *BinkCopyToBufferRect)(
	  HBINK bink,
	  VOID_PTR dest_addr,
	  S32 dest_pitch,
	  U32 dest_height,
	  U32 dest_x,
	  U32 dest_y,
	  U32 src_x,
	  U32 src_y,
	  U32 src_w,
	  U32 src_h,
	  BINK_COPY_FLAGS copy_flags
	);
	BINK_STORAGE void (BINK_CALL *BinkNextFrame)(
	  HBINK bink
	);
	BINK_STORAGE S32 (BINK_CALL *BinkWait)(
	  HBINK bink
	);
	BINK_STORAGE S32 (BINK_CALL *BinkPause)(
	  HBINK bink,
	  S32 pause
	);
	BINK_STORAGE S32 (BINK_CALL *BinkGetRects)(
	  HBINK bink,
	  BINK_GETRECTS_FLAGS getrects_flags
	);
	BINK_STORAGE void (BINK_CALL *BinkGoto)(
	  HBINK bink,
	  U32 frame_num,
	  BINK_GOTO_FLAGS goto_flags
	);
	BINK_STORAGE void (BINK_CALL *BinkGetFrameBuffersInfo)(
	  HBINK bink,
	  BINKFRAMEBUFFERS *set
	);
	BINK_STORAGE void (BINK_CALL *BinkRegisterFrameBuffers)(
	  HBINK bink,
	  BINKFRAMEBUFFERS const *set
	);
	BINK_STORAGE U32 (BINK_CALL *BinkGetKeyFrame)(
	  HBINK bink,
	  U32 frame_num,
	  BINK_GETKEY_FLAGS getkey_flags
	);
	BINK_STORAGE S32 (BINK_CALL *BinkShouldSkip)(
	  HBINK bink
	);
	BINK_STORAGE void (BINK_CALL *BinkService)(
	  HBINK bink
	);
	BINK_STORAGE void (BINK_CALL *BinkSetVolume)(
	  HBINK bink,
	  U32 trackID,
	  S32 volume
	);
	BINK_STORAGE void (BINK_CALL *BinkSetPan)(
	  HBINK bink,
	  U32 trackID,
	  S32 pan
	);
	BINK_STORAGE S32 (BINK_CALL *BinkSetVideoOnOff)(
	  HBINK bink,
	  S32 on_off
	);
	BINK_STORAGE S32 (BINK_CALL *BinkSetSoundOnOff)(
	  HBINK bink,
	  S32 on_off
	);
	BINK_STORAGE void (BINK_CALL *BinkGetSummary)(
	  HBINK bink,
	  BINKSUMMARY *summary
	);
	BINK_STORAGE void (BINK_CALL *BinkGetRealtime)(
	  HBINK bink,
	  BINKREALTIME *real_time,
	  U32 frame_window_length
	);
	BINK_STORAGE S32 (BINK_CALL *BinkControlBackgroundIO)(
	  HBINK bink,
	  BINK_BG_CONTROL_FLAGS control
	);
	BINK_STORAGE U32 (BINK_CALL *BinkGetTrackID)(
	  HBINK bink,
	  U32 track_index
	);
	BINK_STORAGE HBINKTRACK (BINK_CALL *BinkOpenTrack)(
	  HBINK bink,
	  U32 track_index
	);
	BINK_STORAGE void (BINK_CALL *BinkCloseTrack)(
	  HBINKTRACK bink_track
	);
	BINK_STORAGE U32 (BINK_CALL *BinkGetTrackData)(
	  HBINKTRACK bink_track,
	  VOID_PTR dest_addr
	);
	BINK_STORAGE void (BINK_CALL *BinkSetFrameRate)(
	  U32 frame_rate,
	  U32 frame_rate_div
	);
	BINK_STORAGE void (BINK_CALL *BinkSetSoundTrack)(
	  U32 track_count,
	  u32_ptr track_id
	);
	BINK_STORAGE void (BINK_CALL *BinkSetSimulate)(
	  U32 sim_speed
	);
	BINK_STORAGE void (BINK_CALL *BinkSetIOSize)(
	  U32 io_size
	);
	BINK_STORAGE void (BINK_CALL *BinkSetIO)(
	  VOID_PTR BinkOpenFileFunction
	);
	BINK_STORAGE void (BINK_CALL *BinkSetMemory)(
	  BINKMEMALLOC mem_alloc,
	  BINKMEMFREE mem_free
	);
	BINK_STORAGE const char *(BINK_CALL *BinkGetError)();
	BINK_STORAGE VOID_PTR (BINK_CALL *BinkLogoAddress)();
	BINK_STORAGE S32 (BINK_CALL *BinkSetSoundSystem)(
	  VOID_PTR BinkOpenSoundSystemFunction,
	  U32 param_value
	);

	typedef S32 (BINK_CALL PTR4* BINKSNDOPEN)(struct BINKSND PTR4* BnkSnd, U32 freq, S32 bits, S32 chans, U32 flags, HBINK bink);

	BINK_STORAGE BINKSNDOPEN (BINK_CALL *BinkOpenMiles)(U32 param); // don't call directly

	BINK_STORAGE BINK_COPY_FLAGS (BINK_CALL *BinkDX9SurfaceType)(
	  LPDIRECT3DSURFACE9 lpDirect3DSurface
	);

	BINK_STORAGE U32 (BINK_CALL *RADTimerRead)();
	// ------------------------------------------
};
