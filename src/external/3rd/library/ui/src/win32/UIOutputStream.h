#ifndef __UIOUTPUTSTREAM_H__
#define __UIOUTPUTSTREAM_H__

#include "UITypes.h"

class UIOutputStream
{
private:
	FILE * fl;
public:
	UIOutputStream ();
	~UIOutputStream ();
	UIOutputStream & operator << (const char * s);
	UIOutputStream & operator << (const std::string &s);
	UIOutputStream & operator << (const Unicode::String &s);
	UIOutputStream & operator << (const char c);
	UIOutputStream & operator << (int i);
	UIOutputStream & operator << (size_t i);
	UIOutputStream & operator << (float f);
	void flush ();

	static void ExplicitDestroy ();
};

//----------------------------------------------------------------------

UIOutputStream *GetUIOutputStream( void );

#endif // __UIOUTPUTSTREAM_H__