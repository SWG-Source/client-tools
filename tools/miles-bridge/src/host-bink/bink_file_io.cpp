// Adapted from the repository's BinkTreeFileIO.cpp.
// Copyright 2001 - 2003 Sony Online Entertainment
// All Rights Reserved
// Original buffering and Bink suspension callbacks remain local to this x86
// host; only opaque reverse-file tokens cross the process boundary.
#include "bink_file_io.h"
#include "../host-runtime/host_file_runtime.h"
#include <cstring>
#include <type_traits>
#ifdef _WIN64
#error Original Bink IO adapter requires Win32
#endif
namespace MilesHostBink {
namespace {
struct Binding {
    MilesHostRuntime50::Runtime &runtime;
    TimerRead timer;
    Binding(MilesHostRuntime50::Runtime &r, TimerRead t):runtime(r),timer(t) {}
private:
    Binding(const Binding &);
    Binding &operator=(const Binding &);
};
Binding *volatile published = 0;
Binding &binding() {
    Binding *p = static_cast<Binding *>(InterlockedCompareExchangePointer(
        reinterpret_cast<PVOID volatile *>(&published), 0, 0));
    if (!p) MilesHostRuntime50::fatal();
    return *p;
}
U32 radopen(const char *name) {
    uint32_t token = 0;
    uint32_t status = binding().runtime.invoke(MilesWire::FileOpen,0,name,0,0,0,token);
    return status ? token : 0; // Open status is distinct from native/client handle bits.
}
void radseekbegin(U32 token, U32 offset) {
    // Reverse file seeks, like the engine AbstractFile API, use signed 32-bit offsets.
    // Do not wrap a >2GiB absolute position into a negative seek.
    if (offset > 0x7fffffffU) MilesHostRuntime50::fatal();
    uint32_t unused = 0;
    binding().runtime.invoke(MilesWire::FileSeek,token,0,static_cast<int32_t>(offset),0,0,unused);
}
void radread(U32 token, void *dest, U32 size, U32 *numRead) {
    U32 total = 0;
    // One logical Bink read may exceed a reverse frame. Preserve its byte order
    // and stop at the first short read; do not turn EOF into a retried full read.
    while (total < size) {
        U32 count = size - total;
        if (count > MilesWire::MaxFrameBytes - 128) count = MilesWire::MaxFrameBytes - 128;
        uint32_t unused = 0;
        U32 got = binding().runtime.invoke(MilesWire::FileRead,token,0,0,count,
            static_cast<unsigned char *>(dest)+total,unused);
        if (got > count) MilesHostRuntime50::fatal();
        total += got;
        if (got != count) break;
    }
    *numRead = total;
}
void radclose(U32 token) {
    uint32_t unused = 0;
    binding().runtime.invoke(MilesWire::FileClose,token,0,0,0,0,unused);
}
void LockedAddFunc(long *addend, long increment) {
    InterlockedExchangeAdd(reinterpret_cast<volatile LONG *>(addend),increment);
}
// defined variables that are accessed from the Bink IO structure
typedef struct BINKFILE
{
  U32 FileHandle;
  U32 FileIOPos;
  U32 FileBufPos;
  U8* BufPos;
  U32 BufEmpty;
  S32 InIdle;
  U8* Buffer;
  U8* BufEnd;
  U8* BufBack;
  U32 DontClose;
  U32 StartFile;
  U32 FileSize;
  U32 Simulate;
  S32 AdjustRate;
} BINKFILE;

#define BF ( ( BINKFILE PTR4 volatile * )bio->iodata )

#define BASEREADSIZE ( 64 * 1024 )

#define SUSPEND_CB( bio ) { if ( bio->suspend_callback ) bio->suspend_callback( bio );}
#define RESUME_CB( bio ) { if ( bio->resume_callback ) bio->resume_callback( bio );}
#define TRY_SUSPEND_CB( bio ) ( ( bio->try_suspend_callback == 0 ) ? 0 : bio->try_suspend_callback( bio ) )
#define IDLE_ON_CB( bio ) { if ( bio->idle_on_callback ) bio->idle_on_callback( bio );}

// ======================================================================
//reads from the header
static U32 RADLINK BinkFileReadHeader( BINKIO PTR4* bio, S32 offset, void* dest, U32 size )
{
  try {

  U32 amt, temp;

  SUSPEND_CB( bio );

  if ( offset != -1 )
  {
    if ( BF->FileIOPos != (U32) offset )
    {
      radseekbegin( BF->FileHandle, offset + BF->StartFile );
      BF->FileIOPos = offset;
    }
  }
  radread( BF->FileHandle, dest, size, &amt );
  if ( amt !=size )
    bio->ReadError=1;

  BF->FileIOPos += amt;
  BF->FileBufPos = BF->FileIOPos;

  temp = ( BF->FileSize - BF->FileBufPos );
  bio->CurBufSize = ( temp < bio->BufSize ) ? temp : bio->BufSize;

  RESUME_CB( bio );

  return( amt );

  } catch (...) { MilesHostRuntime50::fatal(); }
}
// ======================================================================


//reads a frame (BinkIdle might be running from another thread, so protect against it)
static U32  RADLINK BinkFileReadFrame( BINKIO PTR4* bio,
												  U32 framenum,
												  S32 offset,
												  void* dest,
												  U32 size )
{
  try {

	(void)framenum;

	S32 funcstart = 0;
	U32 amt, tamt = 0;
	U32 timer, timer2;
	U32 cpy;
	//void* odest = dest;

	if ( bio->ReadError )
		return( 0 );

	timer = binding().timer();

	if ( offset != -1 )
	{
		if ( BF->FileBufPos != (U32) offset )
		{

			funcstart = 1;
			SUSPEND_CB( bio );

			if ( ( (U32) offset > BF->FileBufPos ) && ( (U32) offset <= BF->FileIOPos ) )
			{

				amt = offset-BF->FileBufPos;

				BF->FileBufPos = offset;
				BF->BufEmpty += amt;
				bio->CurBufUsed -= amt;
				BF->BufPos += amt;
				if ( BF->BufPos > BF->BufEnd )
					BF->BufPos -= bio->BufSize;

			}
			else
			{

				radseekbegin( BF->FileHandle, offset + BF->StartFile );
				BF->FileIOPos = offset;
				BF->FileBufPos = offset;

				BF->BufEmpty = bio->BufSize;
				bio->CurBufUsed = 0;
				BF->BufPos = BF->Buffer;
				BF->BufBack = BF->Buffer;
			}
		}

	}

	// copy from background buffer
getrest:

	cpy = bio->CurBufUsed;

	if ( cpy )
	{
		U32 front;

		if ( cpy > size )
			cpy = size;

		size -= cpy;
		tamt += cpy;
		BF->FileBufPos += cpy;

		front = BF->BufEnd - BF->BufPos;
		if ( front <= cpy )
		{
			memcpy( dest, BF->BufPos, front );
			dest = ( (U8*) dest ) + front;
			BF->BufPos = BF->Buffer;
			cpy -= front;
			LockedAddFunc( (long*) &bio->CurBufUsed, -(S32)front );
			LockedAddFunc( (long*) &BF->BufEmpty, front );
			if ( cpy == 0 )
				goto skipwrap;
		}
		memcpy( dest, BF->BufPos, cpy );
		dest = ( (U8*) dest ) + cpy;
		BF->BufPos += cpy;
		LockedAddFunc( (long*) &bio->CurBufUsed, -(S32)cpy );
		LockedAddFunc( (long*) &BF->BufEmpty, cpy );
	}

skipwrap:

	if ( size )
	{
		if ( funcstart == 0 )
		{
			funcstart = 1;
			SUSPEND_CB( bio );

			goto getrest;
		}

		timer2 = binding().timer();
		radread( BF->FileHandle, dest, size, &amt );
		if ( amt < size )
			bio->ReadError = 1;

		BF->FileIOPos += amt;
		BF->FileBufPos += amt;
		bio->BytesRead += amt;
		tamt += amt;

		//if ( BF->Simulate )
		//  dosimulate( bio, amt, timer2 );

		amt = binding().timer();
		bio->TotalTime += ( amt - timer2 );
		bio->ForegroundTime += ( amt - timer );
	}
	else
	{
		bio->ForegroundTime += ( binding().timer() - timer );
	}

	amt = ( BF->FileSize - BF->FileBufPos );
	bio->CurBufSize = ( amt < bio->BufSize ) ? amt : bio->BufSize;
	if ( ( bio->CurBufUsed + BASEREADSIZE ) > bio->CurBufSize )
		bio->CurBufSize = bio->CurBufUsed;

	if ( funcstart )
		RESUME_CB( bio );

	return( tamt );

  } catch (...) { MilesHostRuntime50::fatal(); }
}


//returns the size of the recommended buffer
static U32 RADLINK BinkFileGetBufferSize( BINKIO PTR4* bio, U32 size )
{
	(void)bio;
	if (size > 0xffffffffU - (BASEREADSIZE - 1))
        MilesHostRuntime50::fatal();
	size = ( ( size + ( BASEREADSIZE - 1 ) ) / BASEREADSIZE ) * BASEREADSIZE;
	return( size );
}


//sets the address and size of the background buffer
static void RADLINK BinkFileSetInfo( BINKIO PTR4* bio,
												void PTR4* buf,
												U32 size,
												U32 filesize,
												U32 simulate )
{
  try {

	SUSPEND_CB( bio );

	size = ( size / BASEREADSIZE ) * BASEREADSIZE;
	BF->Buffer = (U8*) buf;
	BF->BufPos = (U8*) buf;
	BF->BufBack = (U8*) buf;
	BF->BufEnd =( (U8*) buf ) + size;
	bio->BufSize = size;
	BF->BufEmpty = size;
	bio->CurBufUsed = 0;
	BF->FileSize = filesize;
	BF->Simulate = simulate;

	RESUME_CB( bio );

  } catch (...) { MilesHostRuntime50::fatal(); }
}


//close the io structure
static void RADLINK BinkFileClose( BINKIO PTR4* bio )
{
  try {

	SUSPEND_CB( bio );

	if ( BF->FileHandle )
	{
		radclose( BF->FileHandle );
		BF->FileHandle = 0;
	}

	RESUME_CB( bio );

  } catch (...) { MilesHostRuntime50::fatal(); }
}


//tells the io system that idle time is occurring (can be called from another thread)
static U32 RADLINK BinkFileIdle(BINKIO PTR4* bio)
{
  try {

	U32 amt = 0;
	U32 temp, timer;
	S32 Working = bio->Working;

	if ( bio->ReadError )
		return( 0 );

	if ( bio->Suspended )
		return( 0 );

	if ( TRY_SUSPEND_CB( bio ) )
	{
		temp = ( BF->FileSize - BF->FileIOPos );

		if ( ( BF->BufEmpty >= BASEREADSIZE ) && ( temp >= BASEREADSIZE ) )
		{
			{
				timer = binding().timer();

				bio->DoingARead = 1;
				radread( BF->FileHandle, (void*) BF->BufBack, BASEREADSIZE, &amt );
				bio->DoingARead = 0;

				if ( amt < BASEREADSIZE )
					bio->ReadError = 1;

				bio->BytesRead += amt;
				BF->FileIOPos += amt;
				BF->BufBack += amt;
				if ( BF->BufBack >= BF->BufEnd )
					BF->BufBack = BF->Buffer;

				LockedAddFunc( (long*) &BF->BufEmpty, -(S32) amt );
				LockedAddFunc( (long*)&bio->CurBufUsed, amt );

				if ( bio->CurBufUsed > bio->BufHighUsed )
					bio->BufHighUsed = bio->CurBufUsed;

				//if ( BF->Simulate )
				//  dosimulate( bio, amt, timer );

				timer = binding().timer() - timer;
				bio->TotalTime += timer;
				if ( ( Working ) || ( bio->Working ) )
					bio->ThreadTime += timer;
				else
					bio->IdleTime += timer;
			}
		}
		else
		{
			// if we can't fill anymore, then set the max size to the current size
			bio->CurBufSize = bio->CurBufUsed;
		}

		RESUME_CB( bio );
	}
	else
	{
		// if we're in idle in the background thread, do a sleep to give it more time
		IDLE_ON_CB( bio ); // let the callback run
		amt = (U32)-1;
	}

	return( amt );

  } catch (...) { MilesHostRuntime50::fatal(); }
}


//close the io structure
static S32 RADLINK BinkFileBGControl( BINKIO PTR4* bio, U32 control )
{
  try {

  if ( control & BINKBGIOSUSPEND )
  {
    if ( bio->Suspended == 0 )
    {
      bio->Suspended = 1;
    }
    if ( control & BINKBGIOWAIT )
    {
      SUSPEND_CB( bio );
      RESUME_CB( bio );
    }
  }
  else if ( control & BINKBGIORESUME )
  {
    if ( bio->Suspended == 1 )
    {
      bio->Suspended = 0;
    }
    if ( control & BINKBGIOWAIT )
    {
      BinkFileIdle( bio );
    }
  }
  return( bio->Suspended );

  } catch (...) { MilesHostRuntime50::fatal(); }
}


//opens a normal filename into an io structure
static S32 RADLINK BinkFileOpen( BINKIO PTR4* bio, const char PTR4* name, U32 flags )
{
  try {

  memset( bio, 0, sizeof( BINKIO ) );

  // The real engine caller supplies filenames. Never interpret a remote
  // pointer or client handle as a native host file.
  if (!name || (flags & BINKFILEHANDLE)) return 0;
  BF->FileHandle = radopen(name);
  if (!BF->FileHandle) return 0;

  bio->ReadHeader = BinkFileReadHeader;
  bio->ReadFrame = BinkFileReadFrame;
  bio->GetBufferSize = BinkFileGetBufferSize;
  bio->SetInfo = BinkFileSetInfo;
  bio->Idle = BinkFileIdle;
  bio->Close = BinkFileClose;
  bio->BGControl = BinkFileBGControl;

  return( 1 );

  } catch (...) { MilesHostRuntime50::fatal(); }
}


static_assert(sizeof(BINKFILE) <= sizeof(((BINKIO *)0)->iodata), "Bink IO state extent");
static_assert(sizeof(void *) == 4 && sizeof(U32) == 4 && sizeof(long) == 4, "Win32 Bink IO ABI");
static_assert(std::is_same<decltype(&BinkFileOpen), BINKIOOPEN>::value, "Bink open callback ABI");
} // namespace
BINKIOOPEN bindFileIo(MilesHostRuntime50::Runtime &runtime, TimerRead timer) {
    if (!timer) MilesHostRuntime50::fatal();
    Binding *state = new Binding(runtime,timer);
    if (InterlockedCompareExchangePointer(reinterpret_cast<PVOID volatile *>(&published),state,0)) {
        delete state;
        MilesHostRuntime50::fatal();
    }
    return &BinkFileOpen;
}
} // namespace MilesHostBink
