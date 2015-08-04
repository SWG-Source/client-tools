#include "_precompile.h"

#include "UIOutputStream.h"
#include "UIReport.h"
#include <cassert>
#include <cstdio>

static UIOutputStream *o = 0;

#include <windows.h>

//----------------------------------------------------------------------

UIOutputStream *GetUIOutputStream( void )
{
	if( !o )
	{
		o = new UIOutputStream;
	}
	
	return o;
}

//----------------------------------------------------------------------

void UIOutputStream::ExplicitDestroy ()
{
	delete o;
	o = 0;
}


//----------------------------------------------------------------------

UIOutputStream::UIOutputStream ()
{
	fl = fopen ("ui.log", "w");
	if (!fl)
		UI_REPORT_LOG_PRINT (true, ("UIOutputStream: Unable to open file ui.log for writing."));
}

//----------------------------------------------------------------------

UIOutputStream & UIOutputStream::operator << (const char * s)
{
	if (fl)
		fputs (s, fl);
	return *this;
}

//----------------------------------------------------------------------

UIOutputStream & UIOutputStream::operator << (const std::string &s)
{ 
	return *this << s.c_str ();
}

//----------------------------------------------------------------------

UIOutputStream & UIOutputStream::operator << (const Unicode::String &s)
{
	return *this << Unicode::wideToNarrow (s).c_str ();
}

//----------------------------------------------------------------------

UIOutputStream & UIOutputStream::operator << (const char c)
{
	if (fl)
		fputc (c, fl);

	return *this;
}

//----------------------------------------------------------------------

UIOutputStream & UIOutputStream::operator << (const int i)
{
	if (fl)
		fprintf( fl, "%d", i);

	return *this;
}

//----------------------------------------------------------------------

UIOutputStream & UIOutputStream::operator << (const float f)
{
	if (fl)
		fprintf( fl, "%f", f);
	return *this;
}

//----------------------------------------------------------------------

UIOutputStream & UIOutputStream::operator << (const size_t i)
{
	if (fl)
		fprintf( fl, "%d", i);
	return *this;
}

//----------------------------------------------------------------------

void UIOutputStream::flush ()
{ 
	if (fl)
		fflush (fl);
}

//----------------------------------------------------------------------

UIOutputStream::~UIOutputStream ()
{
	if (fl)
	{
		fclose (fl);
		fl = 0;
	}
}

//----------------------------------------------------------------------
