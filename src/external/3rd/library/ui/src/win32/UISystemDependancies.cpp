#include "_precompile.h"

#include "UISystemDependancies.h"

static UISystemDependancies gUISystemDependancies;

UISystemDependancies &UISystemDependancies::Get( void )
{
	return gUISystemDependancies;
}

#if _WIN32

#include <windows.h>
#include <mmsystem.h>

UISystemDependancies::UISystemDependancies( void )
{
	timeBeginPeriod( 10 );
}

UISystemDependancies::~UISystemDependancies( void )
{
	timeEndPeriod( 10 );
}

long UISystemDependancies::GetCaratBlinkTickCount( void ) const
{
	static long CaratBlinkTime = GetCaretBlinkTime() * 60 / 1000;
	return CaratBlinkTime;
}

long UISystemDependancies::GetTickCount( void ) const
{
	return timeGetTime();
}

#else

	#error Functions in class UISystemDependancy are not implemented for target platform

#endif