#include "_precompile.h"

#include "UIUtils.h"
#include "UIBaseObject.h"
#include "UIOutputStream.h"
#include "UIPalette.h"

#include <cassert>
#include <cstdio>
#include <vector>
#include <list>

//======================================================================================

namespace
{
	const Unicode::String s_layout_horizontal = UI_UNICODE_T ("Horizontal");
	const Unicode::String s_layout_vertical   = UI_UNICODE_T ("Vertical");
	const Unicode::String s_boolean_true      = Unicode::narrowToWide ("true");
	const Unicode::String s_boolean_false     = Unicode::narrowToWide ("false");

	const Unicode::String breakingChars = Unicode::narrowToWide (".\\/!@#$%^&*()-+=|\'\"?><{}[]");
}

namespace UIUtils
{
	char m_digitGroupingSymbol = '\0';
}
//----------------------------------------------------------------------

void UIUtils::RunTimeDiagnosticDump( const UIBaseObject *DumpingObject, const char *Message, const UIBaseObject *ContextObject )
{
	assert( DumpingObject );
	assert( Message );

	UINarrowString FullPath;
	DumpingObject->GetRoot()->GetPathTo( FullPath, DumpingObject );

	UIOutputStream &out = *GetUIOutputStream();
	
	out << "Error: '" << FullPath.c_str () << "':" << Message << "\n";

	if( ContextObject )
	{
		out << "Debug dump of the item:\n";

		UIBaseObject::UIPropertyNameVector PropertyNames;

		ContextObject->GetPropertyNames( PropertyNames, false );

		for( UIBaseObject::UIPropertyNameVector::const_iterator i = PropertyNames.begin(); i != PropertyNames.end(); ++i )
		{
			UIString PropertyValue;

			const UILowerString & lname = *i;
			ContextObject->GetProperty( lname, PropertyValue );

			out << "\t" << lname.get () << " = " << Unicode::wideToNarrow (PropertyValue) << '\n';
		}
	}
}


//======================================================================================
bool UIUtils::ParseBoolean( const UIString &s, bool &b )
{
	if(UIUnicode::icmp (s, s_boolean_true) == 0)
	{
		b = true;
		return true;
	}
	else if(UIUnicode::icmp (s, s_boolean_false) == 0 )
	{
		b = false;
		return true;
	}
	return false;
}

//======================================================================================

bool UIUtils::ParseLong( const UIString &s, long &out )
{
	UIString n;

	UIString::const_iterator	i = s.begin();
	for( ; (i != s.end()) && isspace(*i); ++i )
		;

	if( i == s.end() )
		return false;
	
	if( *i == '-' )
	{
		n += *(i++);

		if( i == s.end() )
			return false;
	}

	for( ; (i != s.end()) && isdigit(*i); ++i )
		n += *i;

	if( i != s.end() )
		return false;

	out = atoi( Unicode::wideToNarrow (n).c_str() );
	return true;
}

//======================================================================================

bool UIUtils::ParseFloat( const UIString &s, float &out )
{
	UIString n;

	UIString::const_iterator	i = s.begin();
	for( ; (i != s.end()) && isspace(*i); ++i )
		;

	if( i == s.end() )
		return false;
	
	if( *i == '-' )
	{
		n += *(i++);

		if( i == s.end() )
			return false;
	}

	for( ; (i != s.end()) && (isdigit(*i) || *i == '.'); ++i )
		n += *i;

	if( i != s.end() )
		return false;

	out = static_cast<float>( atof( Unicode::wideToNarrow (n).c_str() ) );
	return true;
}

//-----------------------------------------------------------------

bool UIUtils::ParseFloatPoint( const UIString &s, UIFloatPoint &p )
{
	const size_t comma = s.find (',', 0);

	if (comma == s.npos)
		return false;

	UIFloatPoint tmp;

	p.x = static_cast<float>(atof (Unicode::wideToNarrow (s.substr (0, comma)).c_str ()));
	p.y = static_cast<float>(atof (Unicode::wideToNarrow (s.substr (comma + 1)).c_str ()));

	return true;
}

//======================================================================================

bool UIUtils::ParsePoint( const UIString &s, UIPoint &p )
{	
	const size_t comma = s.find (',', 0);

	if (comma == s.npos)
		return false;

	static std::string tmp;
	Unicode::wideToNarrow (s, tmp);

	p.x = atoi (tmp.c_str ());
	p.y = atoi (tmp.c_str () + comma + 1);

//	p.x = atoi (Unicode::wideToNarrow (s.substr (0, comma)).c_str ());
//	p.y = atoi (Unicode::wideToNarrow (s.substr (comma + 1)).c_str ());

	return true;
}

//======================================================================================
bool UIUtils::ParseRect( const UIString &ws, UIRect &out )
{
	size_t commas [3];
	size_t pos = 0;

	for (int i = 0; i < 3; ++i)
	{
		commas [i] = ws.find (',', pos);

		if (commas [i] == ws.npos)
			return false;

		pos = commas [i] + 1;
	}

	static std::string s;
	Unicode::wideToNarrow (ws, s);

	out.left   = atoi( s.c_str ());
	out.top    = atoi( s.c_str () + commas[0] + 1);
	out.right  = atoi( s.c_str () + commas[1] + 1);
	out.bottom = atoi( s.c_str () + commas[2] + 1);

	return true;
}

//----------------------------------------------------------------------

bool	UIUtils::ParseColorOrPalette ( const UIString & s, UIColor & out )
{
	if (UIUtils::ParseColor (s, out))
		return true;

	const UIPalette * const pal = UIPalette::GetInstance ();
	if (pal)
		return pal->FindColor (UILowerString (Unicode::wideToNarrow (s)), out);

	return false;
}

//======================================================================================

bool UIUtils::ParseColor( const UIString &s, UIColor &out )
{
	UIString c[3];

	UIString::const_iterator	i = s.begin();
	for( ; (i != s.end()) && isspace(*i); ++i )
		;

	if( i == s.end() )
		return false;

	if( *i != '#' )
		return false;

	for( int col = 0; col < 3; ++col )
	{
		if( ++i == s.end() || !isxdigit( *i ) )
			return false;

		c[col] = *i;

		if( ++i == s.end() || !isxdigit( *i ) )
			return false;

		c[col] += *i;
	}

	++i;

	for( ; isspace(*i) && (i != s.end()); ++i )
		;

	if( i != s.end() )	// gunk on the end of the string
		return false;
	
	out.r = static_cast< unsigned char >( UIUtils::hextoi<UIString::value_type>( c[0].c_str() ) );
	out.g = static_cast< unsigned char >( UIUtils::hextoi<UIString::value_type>( c[1].c_str() ) );
	out.b = static_cast< unsigned char >( UIUtils::hextoi<UIString::value_type>( c[2].c_str() ) );
	out.a = static_cast< unsigned char >( 0xFF );
	return true;
}

//======================================================================================
bool UIUtils::ParseColorARGB( const UIString &s, UIColor &out )
{
	UIString c[4];

	UIString::const_iterator	i = s.begin();
	for( ; (i != s.end()) && isspace(*i); ++i )
		;

	if( i == s.end() )
		return false;

	if( *i != '#' )
		return false;

	for( int col = 0; col < 4; ++col )
	{
		if( ++i == s.end() || !isxdigit( *i ) )
			return false;

		c[col] = *i;

		if( ++i == s.end() || !isxdigit( *i ) )
			return false;

		c[col] += *i;
	}

	++i;

	for( ; isspace(*i) && (i != s.end()); ++i )
		;

	if( i != s.end() )	// gunk on the end of the string
		return false;
	
	out.a = static_cast< unsigned char >( UIUtils::hextoi<UIString::value_type>( c[0].c_str() ) );
	out.r = static_cast< unsigned char >( UIUtils::hextoi<UIString::value_type>( c[1].c_str() ) );
	out.g = static_cast< unsigned char >( UIUtils::hextoi<UIString::value_type>( c[2].c_str() ) );
	out.b = static_cast< unsigned char >( UIUtils::hextoi<UIString::value_type>( c[3].c_str() ) );
	return true;
}

//======================================================================================

bool UIUtils::ParseLayout( const UIString &In, UIStyle::Layout &Out )
{
	if( UIUnicode::icmp (In, s_layout_horizontal) == 0)
		Out = UIStyle::L_horizontal;
	else if( UIUnicode::icmp (In, s_layout_vertical) == 0 )
		Out = UIStyle::L_vertical;
	else
		return false;

	return true;
}

//======================================================================================

int	UIUtils::ParseLongVector ( const UIString &s, std::vector<long> &Out )
{
	Out.clear();

	Unicode::UnicodeStringVector result;
	Unicode::tokenize(s, result);

	Out.reserve(result.size());
	int i;
	for(i = 0; i < static_cast<int>(result.size()); i++)
	{
		Out.push_back(atoi(Unicode::wideToNarrow(result[i]).c_str()));
	}
	return Out.size();
}

//======================================================================================
bool UIUtils::FormatBoolean( UIString &Out, const bool In )
{
	if( In )
		Out = s_boolean_true;
	else
		Out = s_boolean_false;

	return true;
}

//======================================================================================

bool UIUtils::FormatLong( UIString &Out, const long In )
{
	static char Buffer[128];

	_snprintf (Buffer, 128, "%d", In );
	Unicode::narrowToWide (Buffer, Out);
	return true;
}

//======================================================================================

bool UIUtils::FormatFloat( UIString &Out, const float In )
{
	static char buf[32];

	_snprintf (buf, 32, "%0.2f", In);
	Unicode::narrowToWide (buf, Out);
	return true;
}

//======================================================================================

bool UIUtils::FormatPoint( UIString &Out, const UIPoint &In )
{
	static char Buffer[128];

	_snprintf(Buffer, 128, "%d,%d", In.x, In.y );
	Unicode::narrowToWide (Buffer, Out);
	return true;
}

//======================================================================================

bool UIUtils::FormatFloatPoint( UIString &Out, const UIFloatPoint &In )
{
	static char Buffer[128];

	sprintf(Buffer, "%0.2f,%0.2f", In.x, In.y );
	Unicode::narrowToWide (Buffer, Out);
	return true;
}

//======================================================================================

bool UIUtils::FormatRect( UIString &Out, const UIRect &In )
{
	static char Buffer[128];

	_snprintf(Buffer, 128, "%d,%d,%d,%d", In.left, In.top, In.right, In.bottom );
	Unicode::narrowToWide (Buffer, Out);
	return true;
}

//======================================================================================

bool UIUtils::FormatColor( UIString &Out, const UIColor &In )
{
	static char Buffer[128];

	_snprintf(Buffer, 128, "#%02X%02X%02X", In.r, In.g, In.b );
	Unicode::narrowToWide (Buffer, Out);
	return true;
}

//======================================================================================
bool UIUtils::FormatColorARGB( UIString &Out, const UIColor &In )
{
	static char Buffer[128];

	_snprintf(Buffer, 128, "#%02x%02X%02X%02X", In.a, In.r, In.g, In.b );
	Unicode::narrowToWide (Buffer, Out);
	return true;
}

//----------------------------------------------------------------------

bool UIUtils::FormatLayout( UIString &Out, UIStyle::Layout In )
{
	switch( In )
	{
		default:
			assert( false );
			// Fall through

		case UIStyle::L_horizontal:
			Out = s_layout_horizontal;
			break;

		case UIStyle::L_vertical:
			Out = s_layout_vertical;
			break;
	}

	return true;

}

//-----------------------------------------------------------------

bool UIUtils::FormatLongList( UIString &Out, const LongList &In)
{
	static char Buffer[1024];
	static const size_t bufferSize = sizeof(Buffer) / sizeof(Buffer[0]);
	int pos = 0;
	LongList::const_iterator i;
	for( i = In.begin(); (i != In.end()) && (pos < bufferSize); ++i)
	{
		pos += _snprintf(&Buffer[pos], bufferSize - pos, "%d ", *i);
	}
	Buffer[bufferSize - 1] = '\0';
	Unicode::narrowToWide (Buffer, Out);
	return true;
}

//-----------------------------------------------------------------

int UIUtils::ParseTwoTokens (const UIString & str, UIString tokens[])
{
	assert (tokens);

	const size_t comma_pos = str.find_first_of (',');

	tokens [0] = str.substr (0, comma_pos);
	Unicode::trim (tokens [0]);

	if (comma_pos != UIString::npos)
	{
		tokens [1] = str.substr (comma_pos+1);
		Unicode::trim (tokens [1]);
		return 2;
	}

	return 1;
}

//----------------------------------------------------------------------

bool UIUtils::MovePointLeftOneWord (const Unicode::String & str, int startpos, int & newpos)
{
	newpos = startpos;
	if (startpos <= 0)
		return false;

	const size_t last_white_pos = str.find_last_of (Unicode::whitespace, startpos - 1);
	const size_t last_break_pos = str.find_last_of (breakingChars,       startpos - 1);

	size_t pos = static_cast<size_t>(std::string::npos);

	if (last_white_pos != std::string::npos && (last_white_pos > last_break_pos || last_break_pos == std::string::npos))
		pos = last_white_pos;
	else
		pos = last_break_pos;

	if (pos == static_cast<size_t>(startpos - 1))
	{
		if (last_break_pos == pos)
			pos = str.find_last_not_of (breakingChars,       startpos - 1);
		else
			pos = str.find_last_not_of (Unicode::whitespace, startpos - 1);
	}

	if (pos != str.npos)
		newpos = pos + 1;
	else
		newpos = 0;

	return true;
}

//----------------------------------------------------------------------

bool UIUtils::MovePointRightOneWord (const Unicode::String & str, int startpos, int & newpos)
{
	newpos = startpos;
	if (startpos < 0)
		return false;

	const size_t first_white_pos = str.find_first_of (Unicode::whitespace, startpos);
	const size_t first_break_pos = str.find_first_of (breakingChars, startpos);

	size_t pos = std::min (first_white_pos, first_break_pos);

	if (pos == static_cast<size_t>(startpos))
	{
		if (first_break_pos == pos)
			pos = str.find_first_not_of (breakingChars, startpos);
		else
			pos = str.find_first_not_of (Unicode::whitespace, startpos);
	}

	if (pos != str.npos)
		newpos = pos;
	else
		newpos = str.size ();

	return true;
}

//----------------------------------------------------------------------


bool UIUtils::FormatBoolean( UINarrowString & result, const bool b  )
{
	static UIString wide_result;
	if (FormatBoolean (wide_result, b))
	{
		Unicode::wideToNarrow (wide_result, result);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------
bool UIUtils::FormatLong( UINarrowString & result, const long   val   )
{
	static UIString wide_result;
	if (FormatLong (wide_result, val))
	{
		Unicode::wideToNarrow (wide_result, result);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------

bool UIUtils::FormatFloat  ( UINarrowString & result, const float    f )
{
	static UIString wide_result;
	if (FormatFloat (wide_result, f))
	{
		Unicode::wideToNarrow (wide_result, result);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------

bool UIUtils::FormatPoint  ( UINarrowString & result, const UIPoint & pt)
{
	static UIString wide_result;
	if (FormatPoint (wide_result, pt))
	{
		Unicode::wideToNarrow (wide_result, result);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------

bool UIUtils::FormatRect   ( UINarrowString & result, const UIRect &  rect )
{
	static UIString wide_result;
	if (FormatRect (wide_result, rect))
	{
		Unicode::wideToNarrow (wide_result, result);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------

bool UIUtils::FormatColor  ( UINarrowString & result, const UIColor & color )
{
	UIString wide_result;
	if (FormatColor (wide_result, color))
	{
		Unicode::wideToNarrow (wide_result, result);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------

bool UIUtils::FormatColorARGB  ( UINarrowString & result, const UIColor & color )
{
	UIString wide_result;
	if (FormatColorARGB (wide_result, color))
	{
		Unicode::wideToNarrow (wide_result, result);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------

bool UIUtils::FormatLayout ( UINarrowString & result, const UIStyle::Layout  lay  )
{
	UIString wide_result;
	if (FormatLayout (wide_result, lay))
	{
		Unicode::wideToNarrow (wide_result, result);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------

Unicode::String UIUtils::FormatDelimitedInteger ( Unicode::String const & in)
{

	Unicode::String newString = in;

	if(UIUtils::m_digitGroupingSymbol == '\0')
		return newString;

	Unicode::String::iterator iter = newString.end();

	// Total length of the original string.
	unsigned int total = (in.length()-1)/3;
	char token = UIUtils::m_digitGroupingSymbol;

	for(unsigned int i = 0; i < total; ++i)
	{
		iter -= 3;
		
		// We don't want to add delimits to any white space that might be there for padding reasons.
		if((*iter) == ' ')
			continue;
		
		iter = newString.insert(iter, token);
	}

	return newString;

}

//-----------------------------------------------------------------

void UIUtils::SetDigitGroupingSymbol(const char newSymbol )
{
	UIUtils::m_digitGroupingSymbol = newSymbol;
}
//======================================================================================
