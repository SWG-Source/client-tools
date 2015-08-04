#include "FirstLagger.h"
#include <stdarg.h>
#include <stdio.h>
#include <memory>

#ifdef _DEBUG
void DebugLogger::write( const char *format, ... )
{
	va_list args;
	std::auto_ptr< char > buf( new char[ 1024 ] );

	va_start( args, format );
	_vsnprintf( buf.get(), 1024, format, args );
	va_end( args );
	OutputDebugString( buf.get() );
}
#else
void DebugLogger::write( const char *, ... )
{
}
#endif
