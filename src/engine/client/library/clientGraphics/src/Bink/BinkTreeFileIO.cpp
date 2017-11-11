// ======================================================================
//
// BinkTreeFileIO.cpp
//
// Copyright 2001 - 2003 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/BinkTreeFileIO.h"

#include "clientGraphics/BinkDLL.h"

#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/PerThreadData.h"
#include "sharedSynchronization/RecursiveMutex.h"

// ======================================================================

namespace BinkTreeFileIONamespace
{
	static const U32 s_u32neg1 = U32(-1);

	// ----------------------------------------------------------------------


	U32 radopen(const char *name)
	{
		AbstractFile *result = TreeFile::open(name, AbstractFile::PriorityAudioVideo, true);
		return (result) ? U32(result) : U32(-1);
	}

	void radseekbegin(U32 fileHandle, int offset)
	{
		AbstractFile *f = (AbstractFile *)fileHandle;
		f->seek(AbstractFile::SeekBegin, offset);
	}

	U32 radseekcur(U32 fileHandle, int offset)
	{
		AbstractFile *f = (AbstractFile *)fileHandle;
		if (f->seek(AbstractFile::SeekCurrent, offset))
		{
			return U32(f->tell());
		}
		else
		{
			return U32(-1);
		}
	}

	void radread(U32 fileHandle, void *dest, size_t size, unsigned *numRead)
	{
		AbstractFile *f = (AbstractFile *)fileHandle;
		int count = f->read(dest, size);
		if (numRead)
		{
			*numRead=count;
		}
	}

	void radclose(U32 fileHandle)
	{
		AbstractFile *f = (AbstractFile *)fileHandle;
		f->close();
		delete f;
	}

	// ----------------------------------------------------------------------
	static void _lameLockedAddFunc(long *addend, long increment)
	{
		static RecursiveMutex lameInterlockedAdd;
		lameInterlockedAdd.enter();
		{
			*addend+=increment;
		}
		lameInterlockedAdd.leave();
	}

	inline static void LockedAddFunc(long *addend, long increment)
	{
		_lameLockedAddFunc(addend, increment);
	}

	// ----------------------------------------------------------------------

}
using namespace BinkTreeFileIONamespace;

using namespace Bink;
using namespace Bink::RAD;

// ======================================================================

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
}
// ======================================================================
/*
static void dosimulate( BINKIO PTR4* bio, U32 amt, U32 timer )
{
  S32 rt, wait = mult64anddiv( amt, 1000, BF->Simulate );

  rt = RADTimerRead();

  BF->AdjustRate += ( (S32) wait ) - ( (S32) ( rt - timer ) );
  while ( BF->AdjustRate > 0 )
  {
    do
    {
      #ifdef __RADNT__
      Sleep( 0 );
      #endif
      wait = RADTimerRead();
    } while ( ( wait - rt ) < BF->AdjustRate );
    BF->AdjustRate -= ( wait - rt );
    rt = wait;
  }
}
*/

//reads a frame (BinkIdle might be running from another thread, so protect against it)
static U32  RADLINK BinkFileReadFrame( BINKIO PTR4* bio, 
												  U32 framenum, 
												  S32 offset,
												  void* dest,
												  U32 size )
{
	// ---------------------------------------------------

	if (!Os::isMainThread())
	{
		if (!PerThreadData::isThreadInstalled())
		{
			PerThreadData::threadInstall(false);
		}
	}

	// ---------------------------------------------------

	UNREF(framenum);

	S32 funcstart = 0;
	U32 amt, tamt = 0;
	U32 timer, timer2;
	U32 cpy;
	//void* odest = dest;
	
	if ( bio->ReadError )
		return( 0 );
	
	timer = RADTimerRead();
	
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
		
		timer2 = RADTimerRead();
		radread( BF->FileHandle, dest, size, &amt );
		if ( amt < size )
			bio->ReadError = 1;
		
		BF->FileIOPos += amt;
		BF->FileBufPos += amt;
		bio->BytesRead += amt;
		tamt += amt;
		
		//if ( BF->Simulate )
		//  dosimulate( bio, amt, timer2 );
		
		amt = RADTimerRead();
		bio->TotalTime += ( amt - timer2 );
		bio->ForegroundTime += ( amt - timer );
	}
	else
	{
		bio->ForegroundTime += ( RADTimerRead() - timer );
	}
	
	amt = ( BF->FileSize - BF->FileBufPos );
	bio->CurBufSize = ( amt < bio->BufSize ) ? amt : bio->BufSize;
	if ( ( bio->CurBufUsed + BASEREADSIZE ) > bio->CurBufSize )
		bio->CurBufSize = bio->CurBufUsed;
	
	if ( funcstart )
		RESUME_CB( bio );
	
	return( tamt );
}


//returns the size of the recommended buffer
static U32 RADLINK BinkFileGetBufferSize( BINKIO PTR4* bio, U32 size )
{
	UNREF(bio);
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
}


//close the io structure
static void RADLINK BinkFileClose( BINKIO PTR4* bio )
{
	SUSPEND_CB( bio );
	
	if ( BF->DontClose == 0 )
		radclose( BF->FileHandle );
	
	RESUME_CB( bio );
}


//tells the io system that idle time is occurring (can be called from another thread)
static U32 RADLINK BinkFileIdle(BINKIO PTR4* bio)
{
	// ---------------------------------------------------

	if (!Os::isMainThread())
	{
		if (!PerThreadData::isThreadInstalled())
		{
			PerThreadData::threadInstall(false);
		}
	}

	// ---------------------------------------------------

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
				timer = RADTimerRead();
				
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
				
				timer = RADTimerRead() - timer;
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
}


//close the io structure
static S32 RADLINK BinkFileBGControl( BINKIO PTR4* bio, U32 control )
{
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
}


//opens a normal filename into an io structure
RADDEFFUNC S32 RADLINK BinkFileOpen( BINKIO PTR4* bio, const char PTR4* name, U32 flags );
RADDEFFUNC S32 RADLINK BinkFileOpen( BINKIO PTR4* bio, const char PTR4* name, U32 flags )
{
  memset( bio, 0, sizeof( BINKIO ) );

  if ( flags & BINKFILEHANDLE )
  {
    BF->FileHandle = (U32) name;
    BF->DontClose = 1;
    BF->StartFile = radseekcur( (U32) name, 0 );
	 if (BF->StartFile == U32(-1))
	 {
		 return 0;
	 }
  }
  else
  {
    BF->FileHandle = radopen(name);

    //if ( BF->FileHandle == s_u32neg1 )
    //  BF->FileHandle = radopenwithwrite( name, RADREAD );

    if ( BF->FileHandle == s_u32neg1 )
      return( 0 );
  }

  bio->ReadHeader = BinkFileReadHeader;
  bio->ReadFrame = BinkFileReadFrame;
  bio->GetBufferSize = BinkFileGetBufferSize;
  bio->SetInfo = BinkFileSetInfo;
  bio->Idle = BinkFileIdle;
  bio->Close = BinkFileClose;
  bio->BGControl = BinkFileBGControl;

  return( 1 );
}

void *BinkTreeFileIO::getBinkOpenFileFunction()
{
	return (void *)BinkFileOpen;
}
