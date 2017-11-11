#include "_precompile.h"

#include "UITextStyle.h"
#include "UITextStyleWrappedText.h"

#include "UICanvas.h"
#include "UIFontCharacter.h"
#include "UIManager.h"
#include "UIPalette.h"
#include "UIPropertyDescriptor.h"
#include "UIRectangleStyle.h"
#include "UITextStyleManager.h"
#include "UIUtils.h"

#include <unordered_map>
#include <list>
#include <vector>

// ==============================================================

namespace
{
	const Unicode::unicode_char_t elipsis_char = '.';
	const int TabSize = 21;
}

//----------------------------------------------------------------------

const char *UITextStyle::TypeName              = "TextStyle";
bool UITextStyle::ms_enabled                   = true;
float UITextStyle::ms_opacityRelativeMin       = 0.0f;

const UILowerString UITextStyle::PropertyName::Leading = UILowerString ("Leading");
const UILowerString UITextStyle::PropertyName::DropShadowDepth = UILowerString ("DropShadowDepth");
const UILowerString UITextStyle::PropertyName::DropShadowsEnabled = UILowerString ("DropShadowsEnabled");

const std::string UITextStyle::AlignmentNames::Left    = "Left";
const std::string UITextStyle::AlignmentNames::Center  = "Center";
const std::string UITextStyle::AlignmentNames::Right   = "Right";

//======================================================================================
#define _TYPENAME UITextStyle

namespace UITextStyleNamespace
{
	const long JapaneseAddedLeading = 1;

	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Leading, "", T_int),
	_GROUPEND(Basic, 2, int(UIPropertyCategories::C_Basic));
	//================================================================
	//================================================================
	// Appearance category.
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(DropShadowsEnabled, "", T_bool),
		_DESCRIPTOR(DropShadowDepth, "", T_point),
	_GROUPEND(Appearance, 2, int(UIPropertyCategories::C_Appearance));
	//================================================================
}
using namespace UITextStyleNamespace;
//======================================================================================

bool UITextStyle::ms_dropShadowEnabled = true;
UIPoint UITextStyle::ms_dropShadowDepth = UIPoint::one;
bool UITextStyle::ms_autoDropShadow = false;

//http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wcefonts5/html/wce50conenablingasianlinebreaking.asp

// Kinsoku-Shori Japanese line break rule says that a line cannot end with any of the following characters
const unsigned short UITextStyle::KinsokuDontBreakChars[]	=
{
	0x0024,	0x0028,	0x005B,	0x005C, 0x007B,	0x00A2,	0x8165,	0x8167,
	0x8169,	0x816B,	0x816D,	0x816F,	0x8171,	0x8173,	0x8175,	0x8177,
	0x8179,	0x818F,	0x8190,	0x8192,
	0xff08, 0xff3b, 0xff5b, 0xff1c, 0x300c, 0xff62, 0x3014, 0x300a, 0x300e, 0x3010
};

// Kinsoku-Shori Japanese line break rule says that a line cannot begin with any of the follow characters
const unsigned short UITextStyle::KinsokuDontLetStart[] =
{
	0x0021, 0x0025,	0x0029,	0x002C,	0x002E,	0x003F,	0x005D,	0x007D,
	0x00A1,	0x00A3,	0x00A4,	0x00A5,	0x00A7,	0x00A8,	0x00A9,	0x00AA,
	0x00AB,	0x00AC,	0x00AD,	0x00AE,	0x00AF,	0x00B0,	0x00DE,	0x00DF,
	0x3001, 0x3002, 0x300b, 0x300d, 0x300f, 0x3011, 0x3015, 0x309b, 0x309c, 0x30fc, 
	0x8141,	0x8142,	0x8143,	0x8144,	0x8145,	0x8146,	0x8147,	0x8148,
	0x8149,	0x814A,	0x814B,	0x8152,	0x8153,	0x8154,	0x8155,	0x8158,
	0x815B,	0x8166,	0x8168,	0x816A,	0x816C,	0x816E,	0x8170,	0x8172,
	0x8174,	0x8176,	0x8178,	0x817A,	0x818B,	0x818C,	0x818D,	0x818E,
	0x8191,	0x8193,	0x81F1,	0x829F,	0x82A1,	0x82A3,	0x82A5,	0x82A7,
	0x82C1,	0x82E1,	0x82E3,	0x82E5,	0x82EC,	0x8340,	0x8342,	0x8344,
	0x8346,	0x8348,	0x8362,	0x8383,	0x8385,	0x8387,	0x838E,	0x8395,
	0x8396,
	0xff0c, 0xff0e, 0xff63, 0xff64, 0xff61, 0xff1f, 0xff01, 0xff09, 0xff3d,
	0xff5d, 0xff70, 0xff1e, 0xff9e, 0xff9f
};

using namespace UITextStyleNamespace;

// ==============================================================

UITextStyle::UITextStyle() :
UIStyle (),
mGlyphMap (new GlyphMap_t),
mLeading (0),
mOpacity (1.0f),
mDropShadowEnabled(false),
mDropShadowDepth(NULL)
{
	for( long i = 0; i < GLYPH_ARRAY_SIZE; ++i )
		mGlyphArray[i] = 0;
}

// ==============================================================

UITextStyle::~UITextStyle()
{	
	for( long i = 0; i < GLYPH_ARRAY_SIZE; ++i )
	{
		if( mGlyphArray[i] )
			mGlyphArray[i]->Detach( this );
	}
	
	for (GlyphMap_t::iterator iter = mGlyphMap->begin (); iter != mGlyphMap->end (); ++iter)
	{
		(*iter).second->Detach (this);
	}
	
	mGlyphMap->clear();
	delete mGlyphMap;
	mGlyphMap = 0;

	delete mDropShadowDepth;
	mDropShadowDepth = NULL;
}

// ==============================================================

bool UITextStyle::IsA( const UITypeID Type ) const
{
	return (Type == TUITextStyle) || UIStyle::IsA( Type );
}

// ==============================================================

const char *UITextStyle::GetTypeName( void ) const
{
	return TypeName;
}

// ==============================================================

UIBaseObject *UITextStyle::Clone( void ) const
{
	return new UITextStyle;
}

// ==============================================================

void UITextStyle::MeasureCharacter( long c, long &width, long &height ) const
{
	const UIFontCharacter * const glyph = GetCharacter (c);
	
	if( glyph )
	{
		width  = glyph->GetWidth();
		height = mLeading;
	}
	else
	{
		width  = 0;
		height = 0;
	}
}

// ==============================================================

void UITextStyle::MeasureCharacter( long c, UISize &theSize ) const
{
	MeasureCharacter( c, theSize.x, theSize.y );	
}

// ==============================================================

void UITextStyle::SetLeading( const long NewLeading )
{
	mLeading = NewLeading + JapaneseAddedLeading;
}

// ==============================================================

long UITextStyle::GetLeading( void ) const
{
	return mLeading;
}

// ==============================================================

void UITextStyle::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIStyle::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
}

//----------------------------------------------------------------------

void UITextStyle::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	In.push_back ( PropertyName::Leading );
	In.push_back( PropertyName::DropShadowsEnabled);
	In.push_back( PropertyName::DropShadowDepth);


	UIStyle::GetPropertyNames( In, forCopy );
}

// ==============================================================

bool UITextStyle::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::Leading )
	{
		bool ret = UIUtils::ParseLong( Value, mLeading );
		mLeading += JapaneseAddedLeading;
		return ret;
	}
	else if(Name == PropertyName::DropShadowsEnabled)
	{
		bool bDropShadowsEnabled = false;
		
		if(!UIUtils::ParseBoolean(Value, bDropShadowsEnabled))
			return false;
		
		SetDropShadowsEnabled(bDropShadowsEnabled);
		return true;
	}
	else if(Name == PropertyName::DropShadowDepth)
	{
		UIPoint offset;
		
		if(!UIUtils::ParsePoint(Value, offset))
			return false;
		
		SetDropShadowDepth(offset);
		return true;
	}
	
	return UIStyle::SetProperty( Name, Value );
}

// ==============================================================

bool UITextStyle::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::Leading )
	{
		return UIUtils::FormatLong( Value, mLeading );
	}
	else if(Name == PropertyName::DropShadowsEnabled)
	{
		return UIUtils::FormatBoolean(Value, GetDropShadowsEnabled());
	}
	else if(Name == PropertyName::DropShadowDepth)
	{
		return UIUtils::FormatPoint(Value, GetDropShadowDepth());
	}

	return UIStyle::GetProperty( Name, Value );
}

// ==============================================================

UIBaseObject *UITextStyle::GetChild( const char *ChildName ) const
{
	for( int i = 0; i< GLYPH_ARRAY_SIZE; ++i )
	{
		if( mGlyphArray[i] && mGlyphArray[i]->IsName( ChildName ) )
			return mGlyphArray[i];
	}
	
	for (GlyphMap_t::const_iterator iter = mGlyphMap->begin (); iter != mGlyphMap->end (); ++iter)
	{
		UIFontCharacter * const glyph = (*iter).second;
		
		if (glyph->IsName (ChildName))
			return glyph;
	}
	
	return 0;
}

// ==============================================================

bool UITextStyle::AddChild( UIBaseObject *ChildToAdd )
{
	if( ChildToAdd && ChildToAdd->IsA( TUIFontCharacter ) )
	{
		UIFontCharacter *NewCharacter = static_cast<UIFontCharacter *>( ChildToAdd );
		unsigned long code            = NewCharacter->GetCharacterCode();
		
		NewCharacter->Attach( this );
		
		if (code >= 0 && code < GLYPH_ARRAY_SIZE)
		{
			if( mGlyphArray[code] )
			{
				mGlyphArray[code]->Detach( this );
			}
			
			mGlyphArray[code] = NewCharacter;
			
		}
		else
		{
			GlyphMap_t::iterator find_iter = mGlyphMap->find (code);
			
			if (find_iter != mGlyphMap->end ())
			{
				(*find_iter).second->Detach (this);
				(*find_iter).second = NewCharacter;
			}
			else
			{
				(*mGlyphMap) [code] = NewCharacter;
			}
		}
		
		NewCharacter->SetParent( this );
		
		return true;
	}
	else
		return false;
}

// ==============================================================

bool UITextStyle::RemoveChild( UIBaseObject *o )
{
	if( o->IsA( TUIFontCharacter ) )
	{
		UIFontCharacter *NewCharacter = static_cast<UIFontCharacter *>( o );
		unsigned long code            = NewCharacter->GetCharacterCode();
		
		if (code >= 0 && code < GLYPH_ARRAY_SIZE)
		{
			if( mGlyphArray[code] == NewCharacter )
			{
				mGlyphArray[code]->Detach( this );
				mGlyphArray[code] = 0;
				return true;
				
			}
		}
		else
		{
			GlyphMap_t::iterator find_iter = mGlyphMap->find (code);
			
			if (find_iter != mGlyphMap->end ())
			{
				(*find_iter).second->Detach (this);
				(*find_iter).second = 0;
				return true;
			}
		}		
	}
	
	return false;	
}

// ==============================================================

void UITextStyle::GetChildren( UIObjectList &Out ) const
{
	for( int i = 0; i< GLYPH_ARRAY_SIZE; ++i )
	{
		if( mGlyphArray[i] )
			Out.push_back( mGlyphArray[i] );
	}
	
	for (GlyphMap_t::const_iterator iter = mGlyphMap->begin (); iter != mGlyphMap->end (); ++iter)
	{
		Out.push_back ((*iter).second);
	}
}

// ==============================================================

unsigned long UITextStyle::GetChildCount( void ) const
{
	unsigned long nChildren = 0;
	
	for( int i = 0; i < GLYPH_ARRAY_SIZE; ++i )
	{
		if( mGlyphArray[i] )
			++nChildren;
	}
	
	return nChildren + mGlyphMap->size ();
}

// ==============================================================

void UITextStyle::MeasureText( const UIString &s, long &width, long &height, MeasureMethod method, bool ProcessEscapeCharacters ) const
{
	width  = 0;
	height = mLeading;
	
	long maxWidth = 0;

	bool ignoreNextEscape = false;
	
	long indentation = 0L;

	for( UIString::const_iterator i = s.begin(); i != s.end(); ++i )
	{
		UIString::value_type    theChar  = *i;
		
		if( theChar == '\n' )
		{
			height += mLeading;
			maxWidth = std::max(width,maxWidth);
			width = 0;
			continue;
		}
		else if( theChar == '\t' )
		{
			width += TabSize - (width % TabSize);
		}
		else if( ProcessEscapeCharacters && !ignoreNextEscape && theChar == '\\' )
		{
			UIString::const_iterator next = EatEscapeSequence( i, s.end(), ignoreNextEscape, width, indentation);

			width = std::max (indentation, width);

			if (next == s.end ())
				break;
			
			// next is already pointing to the next character to
			// analyse, so compensate for the loop increment.
			if (next != i)
			{
				i = --next;
				continue;
			}
		}		

		ignoreNextEscape = false;
		
		const UIFontCharacter * const glyph = GetCharacter (theChar);
		
		if( glyph )
		{
			if( ((i + 1) == s.end()) && (method == UseLastCharWidth) )
				width += glyph->GetWidth();
			else
				width += glyph->GetAdvance();
		}
	}

	width = std::max(width,maxWidth);
}

// ==============================================================

void UITextStyle::MeasureText( const UIString &s, UISize &theSize, MeasureMethod method, bool ProcessEscapeCharacters ) const
{
	MeasureText( s, theSize.x, theSize.y, method, ProcessEscapeCharacters );	
}

// ==============================================================

void UITextStyle::MeasureWrappedText( const UIString &s, const long WrapWidth, long &width, long &height, MeasureMethod method, bool ProcessEscapeCharacters ) const
{
	GetWrappedTextInfo (s, -1, WrapWidth, width, height, 0, 0, method, ProcessEscapeCharacters);
}

// ==============================================================

void UITextStyle::MeasureWrappedText( const UIString &s, const long WrapWidth, UISize &theSize, MeasureMethod method, bool ProcessEscapeCharacters ) const
{
	MeasureWrappedText( s, WrapWidth, theSize.x, theSize.y, method, ProcessEscapeCharacters );
}

// ==============================================================

/**
* @param WrapWidth zero indicates no wrapping to be performed
*/

void UITextStyle::GetWrappedTextInfo( const UIString &s, 
									 int  maxLines,
									 long WrapWidth, 
									 long &width, 
									 long &height, 
									 UIStringConstIteratorVector *LinePointers, 
									 std::vector<long> *LineWidths, 
									 MeasureMethod method, 
									 bool ProcessEscapeCharacters,
									 bool ignoreLocaleForWrapping) const
{	
	width  = 0;
	height = 0;
	
	const long oldWrapWidth = WrapWidth;

	long currentIndentation = 0L;

	if( !s.empty() )
	{
		// This is the place we can safely break a line
		UIString::const_iterator last_breaker = s.end ();
		UIString::const_iterator line_start   = s.begin ();
		long lineWidth            = 0;
		bool startNewLine         = false;
		long breaker_width        = 0;
		bool ignoreNextEscape     = false;
		long lastCharDiff         = 0L;
		bool lastCharStartNewLine = false;
		int  curLine              = 1;
		bool useJapanese          = !ignoreLocaleForWrapping && UIManager::gUIManager().isLocaleJapanese();
		
		if (maxLines == 1)
			WrapWidth = 16384L;

		for( UIString::const_iterator i = s.begin(); ; ++i )
		{
			//-- delyay breaking out of the loop until startNewLine is processed

			if (startNewLine)
			{
				lastCharStartNewLine = true;

				if (method == UseLastCharWidth && lastCharDiff)
					lineWidth -= lastCharDiff;
				
				if (LinePointers)
					LinePointers->push_back( line_start );
				if (LineWidths)
					LineWidths->push_back (lineWidth);
				
				width = std::max (width, lineWidth);
				height += mLeading;
				
				startNewLine  = false;
				last_breaker  = s.end ();
				breaker_width = 0;
				lineWidth     = currentIndentation;
				line_start    = i;
				lastCharDiff  = 0;

				++curLine;

				if (curLine == maxLines)
					WrapWidth = 16384L;
				else
					WrapWidth = oldWrapWidth;
			}

			//----------------------------------------------------------------------
			
			if (i == s.end())
			{
				if (lastCharStartNewLine)
					lineWidth = -1;
				break;
			}

			lastCharStartNewLine = false;

			Unicode::unicode_char_t c = *i;
			
			if (c == '\r')
				continue;

			//----------------------------------------------------------------------
			
			if (c == '\n')
			{
				startNewLine = true;
				continue;
			}
			
			//----------------------------------------------------------------------
			
			else if( ProcessEscapeCharacters && !ignoreNextEscape && c == '\\')
			{
				UIString::const_iterator next = EatEscapeSequence( i, s.end(), ignoreNextEscape, lineWidth, currentIndentation);
				lineWidth = std::max (currentIndentation, lineWidth);
				
				if (next == s.end ())
					break;
				
				if (next != i)
				{
					// next is already pointing to the next character to
					// analyse, so compensate for the loop increment.
					i = --next;
					continue;
				}
			}
			
			ignoreNextEscape = false;
			
			//----------------------------------------------------------------------
			
			const UIFontCharacter * const glyph = GetCharacter (c);
			
			if (!glyph)
				continue;
			
			const long glyphAdvance = glyph->GetAdvance ();
			const long glyphWidth = glyph->GetAdvance ();
			
			long glyphMeasureWidth = 0L;
			
			if (method == UseLastCharWidth)
			{
				glyphMeasureWidth = glyphWidth;
				lastCharDiff = glyphAdvance - glyphWidth;
			}
			else
				glyphMeasureWidth = glyphAdvance;
			
			//----------------------------------------------------------------------
			//-- time to break it.  Don't break if there's nothing on the line, that could cause an infinite loop of nastiness
			
			if (WrapWidth && (lineWidth > currentIndentation) && (glyphMeasureWidth + lineWidth > WrapWidth))
			{
				if (last_breaker != s.end ())
				{					
					i = last_breaker;
					lineWidth = breaker_width;
				}
				else if (i != s.begin ())
					--i;
				
				startNewLine = true;
			}
			
			//----------------------------------------------------------------------
			//-- no break here (yet...)
			
			else
			{
				lineWidth += glyphAdvance;
				
				if (c == ' ' || c == '-' || c == ',' || c == ';' || c == 0x30FB) // 0x30FB is the japanese "dot-space"
				{
					last_breaker  = i;
					breaker_width = lineWidth;
					
					//-- remove the trailing space from the width
					if (c == ' ')
						breaker_width -= glyphAdvance;
					else if (method == UseLastCharWidth)
						breaker_width -= lastCharDiff;
				}

				// If Locale is Japanese, check to see if the current character
				// Is allowed to be the last character of a line.  If it is,
				// then set possible_breaker and update possible_breaker_width
				if (useJapanese)
				{
					int numChars = sizeof(KinsokuDontBreakChars)/sizeof(KinsokuDontBreakChars[0]);
					bool canBreak = true;
					int j;
					for (j = 0; j < numChars; j++)
					{
						if (KinsokuDontBreakChars[j] == c)
							canBreak = false;
					}
					if(((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')))
						canBreak = false;

					// If We are splitting on a Japanese character, then we have to check the current
					// character to make sure it's not in the set of Kinsoku characters which cannot
					// start a new line.  If it is in the set, then break on last_breaker, otherwise
					// break on kinsoku_possible_last_breaker				
					UIString::const_iterator ip = i + 1;
					if(ip != s.end())
					{
						Unicode::unicode_char_t cp = *ip;
						const int numStartChars = sizeof(KinsokuDontLetStart)/sizeof(KinsokuDontLetStart[0]); 
						for (j = 0; j < numStartChars; j++)
						{
							if (cp == KinsokuDontLetStart[j])
							{
								canBreak = false;
							}
						}
					}

					
					// If its not in the set of prohibited end characters, then
					// it is a possible breaker
					if (canBreak)
					{
						last_breaker = i;
						breaker_width = lineWidth;
					}
				}
			}
		}
		
		if (lineWidth)
		{
			if (lineWidth < 0)
				lineWidth = currentIndentation;

			if (method == UseLastCharWidth && lastCharDiff)
				lineWidth -= lastCharDiff;
			
			if (LinePointers)
				LinePointers->push_back( line_start );
			if (LineWidths)
				LineWidths->push_back (lineWidth);
			
			width = std::max (width, lineWidth);
			height += mLeading;
		}
		
	}
	
	// Tail pointer	
	if (LineWidths)
		LineWidths->push_back( 0 );
	if (LinePointers)
		LinePointers->push_back( s.end() );
}

// ==============================================================

void UITextStyle::GetWrappedTextInfo( const UIString &s, 
									 int maxLines,
									 const long WrapWidth, 
									 UISize &theSize, 
									 UIStringConstIteratorVector * LinePointers, 
									 std::vector<long> * LineWidths, 
									 MeasureMethod method,
									 bool ProcessEscapeCharacters,
									 bool ignoreLocaleForWrapping) const
{
	GetWrappedTextInfo( s, maxLines, WrapWidth, theSize.x, theSize.y, LinePointers, LineWidths, method, ProcessEscapeCharacters, ignoreLocaleForWrapping );
}

// ==============================================================

void UITextStyle::RenderText( Alignment align, const UIString &s, UICanvas & DestinationCanvas, const UIPoint &loc, const long * const WrapWidth, long maxLines, bool truncateElipsis, bool ignoreLocaleForWrapping) const
{
	if (!ms_enabled)
		return;

	UISize textExtent;
	UIStringConstIteratorVector LinePointers;
	std::vector<long> LineWidths;

	long theWrapWidth = 0L;
	if  (WrapWidth)
	{
		if (maxLines != 1 || !truncateElipsis)
			theWrapWidth = *WrapWidth;
	}

	GetWrappedTextInfo (s, maxLines, theWrapWidth, textExtent, &LinePointers, &LineWidths, UseLastCharAdvance, true, ignoreLocaleForWrapping);
	
	UIRect canvasRect;
	DestinationCanvas.GetClip (canvasRect);
	
	canvasRect.left = std::max (loc.x, canvasRect.left);
	if (WrapWidth)
		canvasRect.right = std::min (loc.x + *WrapWidth, canvasRect.right);
	
	RenderPrewrappedText (align, LinePointers, LineWidths, DestinationCanvas, loc, canvasRect.Size (), maxLines, truncateElipsis);
}

//----------------------------------------------------------------------

long UITextStyle::RenderPrewrappedLine(RenderLineData const & rld, UIRectangleStyle const * const textMaskStyle) const
{
	if (!ms_enabled && !rld.selection && !rld.findCol)
		return 0;

	UISize textDropSize = GetDropShadowDepth();

	if (ms_dropShadowEnabled && !rld.ignoreColors)
	{
		//-- if the text is bright, render a dark shadow
		bool autoDropAvailable = mDropShadowEnabled || (ms_autoDropShadow && ((rld.defaultColor.r + rld.defaultColor.g + rld.defaultColor.b) >= 200));
		if (autoDropAvailable || GetDropShadowsEnabled())
		{
			RenderLineData drld = rld;
			UIPoint dropTextOffset = (autoDropAvailable ? ms_dropShadowDepth : textDropSize);
			drld.loc            += dropTextOffset;
			drld.width          -= dropTextOffset.x;
			drld.lineWidth      -= dropTextOffset.x;

			drld.defaultColor   = UIColor::black;
			drld.ignoreColors   = true;

			const UIColor color = rld.canvas ? rld.canvas->GetColor () : UIColor::white;
			RenderPrewrappedLine (drld);
			if (rld.canvas)
				rld.canvas->SetColor (color);
		}
	}

	float oldOpacity = 0.0f;
	if (rld.canvas)
	{
		oldOpacity = rld.canvas->GetOpacity ();
		const float opacityRelativeMax = 1.0f;
		const float opacityRelativeRange = opacityRelativeMax - ms_opacityRelativeMin;
		const float threshold = 0.20f;
		
		float actualOpacity = (oldOpacity * opacityRelativeRange) + ms_opacityRelativeMin;
		
		if (threshold && oldOpacity < ms_opacityRelativeMin && oldOpacity < threshold)
			actualOpacity *= oldOpacity / threshold;
		
		rld.canvas->SetOpacity  (actualOpacity);

		if (rld.ignoreColors)
			rld.canvas->SetColor (rld.defaultColor);
	}

	UIPoint outdest (rld.loc);
	outdest.x = std::max (outdest.x, rld.currentIndentation);
	
	if (rld.lineWidth > 0)
	{
		if( rld.align == Right )
			outdest.x += rld.width - rld.lineWidth;
		else if( rld.align == Center )
		{
			if (rld.lineWidth < rld.width)
				outdest.x += (rld.width - rld.lineWidth) / 2L;
		}
	}
	
	const UIFontCharacter * const elipsisCh = rld.truncateElipsis ? GetCharacter ('.') : 0;
	
	const long elipsisCharWidth = elipsisCh ? elipsisCh->GetAdvance () : 0L;
	const long elipsisWidth     = elipsisCharWidth * 3L;
	long truncateX = 0L;
	
	bool needsToTruncate = false;

	if (rld.truncateElipsis)
	{
		outdest.x = std::max (0L, outdest.x);

		truncateX = std::min (rld.width + rld.loc.x, rld.lineWidth + outdest.x);
		if (rld.width < rld.lineWidth)
		{
			truncateX -= (elipsisWidth);
			needsToTruncate = true;
		}
	}
	
	bool foundCol = false;
		
	const UIFontCharacter * lastCh = 0;
	
	int index = 0;

	UIString::const_iterator begin    = rld.begin;
	UIString::const_iterator nextLine = rld.nextLine;

	for (; begin != nextLine; ++begin, ++index)
	{
//		++index;
		const Unicode::unicode_char_t c = *begin;

		// jww - This is here because template UIText prewrapped data can end up having \r's and \n's in it
		if ( c == '\r')
			continue;
		
		if (c == '\n' && !rld.selection)
			break;
		
		if( c == '\t' )
		{
			outdest.x += TabSize - (outdest.x % TabSize);
			continue;
		}
		else if( c == '\\' )
		{
			rld.inEscapeSequence = !rld.inEscapeSequence;
			
			if (rld.inEscapeSequence)
				continue;
		}
		else if (rld.inEscapeSequence)
		{			
			rld.inEscapeSequence = false;
			
			if( c == '#' )
			{
				UIColor color = rld.canvas ? rld.canvas->GetColor () : UIColor::white;
				begin = ParseColorEscapeSequence( begin, nextLine, rld.defaultColor, color);

				if (!rld.selection && rld.canvas && !rld.ignoreColors)
					rld.canvas->SetColor( color );
				
				// i is already pointing to the next character to
				// analyse, so compensate for the loop increment.
				--begin;
				continue;				
			}
			else if (c == '@')
			{
				bool b = false;
				long dummy = 0L;
				begin = EatEscapeSequence (--begin, nextLine, b, outdest.x, dummy);
				--begin;
				continue;
			}
			else if (c == '>')
			{
				bool b = false;
				long dummy = 0L;
				begin = EatEscapeSequence (--begin, nextLine, b, dummy, rld.currentIndentation);
				outdest.x = std::max (rld.currentIndentation, outdest.x);
				--begin;
				continue;
			}
			else if (c == '^')
			{
				bool b = false;
				long dummy = 0L;
				begin = EatEscapeSequence (--begin, nextLine, b, dummy, rld.currentIndentation);
				outdest.x = rld.currentIndentation;
				--begin;
				continue;
			}			
			else if (c == '%')
			{
				bool b = false;
				long dummy = 0L;
				begin = EatEscapeSequence (--begin, nextLine, b, dummy, rld.currentIndentation);
				long tmp = (static_cast<long>(rld.width * rld.currentIndentation * 0.01f));
				outdest.x = std::max (tmp, outdest.x);
				--begin;
				continue;
			}

		}
		
		const UIFontCharacter * const ch = c == '\n' ? GetCharacter (' ') : GetCharacter (c);
		
		if( ch )
		{
			const int lastAdvance = lastCh ? lastCh->GetAdvance () : 0;
			const int advance = ch->GetAdvance ();
			
			if (rld.findCol)
			{				
				if (!foundCol)
				{
					if ((outdest.x > rld.point) ||
						((outdest.x - (lastAdvance / 2)) <= rld.point && (outdest.x + (advance / 2) > rld.point)))
					{
						foundCol     = true;
						*rld.findCol = index;
					}
				}
			}
			else
			{

				if (rld.numCharsToRender > 0 && rld.numCharsRendered >= rld.numCharsToRender)
					break;

				if (rld.truncateElipsis && elipsisCh && needsToTruncate && (ch->GetAdvance () + outdest.x) > truncateX)
				{
					const UISize elipsisSize (elipsisCh->GetWidth (), mLeading);
					
					for (int e = 0; e < 3; ++e)
					{
						if (rld.canvas)
						{
							rld.canvas->BltFromNoScaleOrRotate (elipsisCh->GetCanvas(), elipsisCh->GetLocation(), outdest, elipsisSize );
						}

						outdest.x += elipsisCh->GetAdvance();
					}
					++rld.numCharsRendered;
					break;
				}
				
				if (rld.canvas)
				{
					if (rld.selection)
					{
						if (index >= rld.mark && index < rld.point)
						{
							UISize const textSize(ch->GetAdvance(), mLeading);
							rld.canvas->BltFromNoScaleOrRotate (0, ch->GetLocation(), outdest, textSize);
						}
					}
					else
					{
						UISize const textSize(ch->GetWidth(), mLeading);

						outdest.x += ch->GetAdvancePre();

						//-- Draw the background rs_text style.  
						if (textMaskStyle)
						{
							UIPoint const & newDest = outdest - textDropSize;
							UIPoint const & newSize = textSize + (textDropSize * static_cast<UIScalar>(2));
							
							rld.canvas->Translate(newDest);
							textMaskStyle->Render(GetAnimationState(), *(rld.canvas), newSize);
							rld.canvas->Translate(-newDest);
						}

						rld.canvas->BltFromNoScaleOrRotate (ch->GetCanvas(), ch->GetLocation(), outdest, textSize);
						outdest.x -= ch->GetAdvancePre();
					}

					++rld.numCharsRendered;
				}
			}
			
			lastCh = ch;

			outdest.x += advance;
		}
	}
	
	if (rld.findCol && !foundCol)
		*rld.findCol = std::max (0, index);

	if (rld.canvas)
		rld.canvas->SetOpacity (oldOpacity);

	return outdest.x;
}
										
										
//----------------------------------------------------------------------
/**
* mark must be <= point
*/

void UITextStyle::RenderPrewrappedSelection  ( Alignment align, const UIStringConstIteratorVector & LinePointers,const std::vector<long> & LineWidths,
											  UICanvas & DestinationCanvas, const UIPoint & loc, const UISize & size, long maxLines,int mark, int point) const
{
	UIRect	rect;
	UIRect	ClippingRect;
	
	DestinationCanvas.GetClip( ClippingRect );
	
	rect.left   = loc.x;
	rect.top    = loc.y;
	rect.right  = ClippingRect.right;
	rect.bottom = loc.y + mLeading * (LinePointers.size() - 1);
	
	if( !UIUtils::ClipRect( rect, ClippingRect ) )
		return;
	
	// LastLineIndex - 1 can be negative, so these are signed
	long FirstLineIndex = (rect.top - loc.y) / mLeading;
	long LastLineIndex  = (rect.bottom - loc.y) / mLeading + 2;
	
	if( LastLineIndex > static_cast<long> (LinePointers.size()) )
		LastLineIndex = LinePointers.size();
			
	int linesDrawn = 0;
	
	int ch = 0;

	RenderLineData rld (&DestinationCanvas, align, loc, size.x, mark, point, UIColor::white);
	rld.truncateElipsis = false;
	rld.selection       = true;

	for( long i = 0; i < (LastLineIndex - 1) && (maxLines <= 0 || linesDrawn < maxLines); ++i )
	{	
		if (ch >= point)
			break;
		
		if (rld.loc.y > rect.bottom)
			break;
				
		UIString::const_iterator CurrentChar = LinePointers[i];
		UIString::const_iterator NextLine    = LinePointers[i+1];
		
		const int numChars = std::distance (CurrentChar, NextLine);
		int rangeCh = ch + numChars;
		
		if (rld.loc.y + mLeading >= rect.top)	
		{
			if (i >= FirstLineIndex)
			{
				if (ch >= mark || rangeCh >= mark)
				{
					rld.begin      = CurrentChar;
					rld.nextLine   = NextLine;
					rld.lineWidth  = LineWidths [i];
					rld.point      = point - ch;
					rld.mark       = mark  - ch;

//					RenderPrewrappedLine (align, CurrentChar, NextLine, LineWidths [i], &DestinationCanvas, outdest, size.x, inEscapeSequence, false, tmark, tpoint, true, 0, rld);
					RenderPrewrappedLine (rld);
				}
				
				++linesDrawn;
			}
		}

		rld.loc.y += mLeading;
		ch = rangeCh;
	}
}

//----------------------------------------------------------------------

const UIPoint        UITextStyle::FindLocationFromCharacter ( Alignment align, const UIStringConstIteratorVector & LinePointers,
	                                                    const std::vector<long> & LineWidths, const UIPoint & loc, const UISize & size, int indexToFind) const
{	
	long FirstLineIndex = 0;
	long LastLineIndex  = LinePointers.size () + 1;
	
	int ch_index = 0;

	RenderLineData rld (0, align, loc, size.x, 0, 0, UIColor::white);
	rld.truncateElipsis = false;
	rld.selection       = false;

	for (long i = FirstLineIndex; i < (LastLineIndex - 1) && ch_index < indexToFind; ++i)
	{
		UIString::const_iterator CurrentChar = LinePointers[i];
		UIString::const_iterator NextLine    = LinePointers[i+1];

		const int numCharsThisLine = static_cast<int>(std::distance (CurrentChar, NextLine));

		if ((numCharsThisLine + ch_index) >= indexToFind)
		{
			UIString::const_iterator actualEnd    = CurrentChar;

			Unicode::unicode_char_t last_char = 0;

			if (indexToFind - ch_index > 0)
			{
				std::advance (actualEnd, indexToFind - ch_index - 1);
				last_char = *actualEnd;
				++actualEnd;
			}
			else
				std::advance (actualEnd, indexToFind - ch_index);

			if (last_char != '\n')
			{
				rld.begin     = CurrentChar;
				rld.nextLine  = actualEnd;
				rld.lineWidth = LineWidths [i];
				rld.loc.x = RenderPrewrappedLine (rld);
				return rld.loc;
			}
		}
		
		ch_index += numCharsThisLine;
		rld.loc.y += mLeading;
	}

	return rld.loc;
}

//----------------------------------------------------------------------

int UITextStyle::FindCharacterFromLocation ( Alignment align, const UIStringConstIteratorVector & LinePointers,
	                                                    const std::vector<long> & LineWidths, const UIPoint & loc, const UISize & size, long maxLines, const UIPoint & pt) const
{
	if (LinePointers.empty ())
		return 0;
		
	int FirstLineIndex = 0;
	int LastLineIndex  = static_cast<int>(LinePointers.size ()) - 1;
	
	int linesDrawn = 0;

	RenderLineData rld (0, align, loc, size.x, 0, pt.x, UIColor::white);

	if (rld.loc.y > pt.y)
		return 0;

	for (int i = FirstLineIndex; i < LastLineIndex  && (maxLines <= 0 || linesDrawn < maxLines); ++i, ++linesDrawn)
	{
		if ((rld.loc.y + mLeading) > pt.y || i == (LastLineIndex - 1))
		{
			UIString::const_iterator CurrentChar = LinePointers [i];
			UIString::const_iterator NextLine    = LinePointers [i + 1];
				
			int col = 0;
			rld.begin     = CurrentChar;
			rld.nextLine  = NextLine;
			rld.lineWidth = LineWidths [i];
			rld.findCol   = &col;
			RenderPrewrappedLine (rld);
//			RenderPrewrappedLine (align, CurrentChar, NextLine, LineWidths [i], 0, outdest, size.x, inEscapeSequence, false, 0, pt.x, false, &col);
			return std::distance (LinePointers [0], CurrentChar) + col;
		}
		
		rld.loc.y += mLeading;
	}


	return std::distance (LinePointers.front (), LinePointers.back ());
}

//----------------------------------------------------------------------

void UITextStyle::RenderPrewrappedText( Alignment align,
									   const UIStringConstIteratorVector &LinePointers,
									   const std::vector<long> &LineWidth, UICanvas & DestinationCanvas, 
									   const UIPoint & loc, const UISize & size, long maxLines, 
									   bool truncateElipsis, int numCharsToRender, 
									   UIRectangleStyle const * const textMaskStyle) const
{
	if (!ms_enabled)
		return;

	UIRect	rect;
	UIRect	ClippingRect;
	
	DestinationCanvas.GetClip( ClippingRect );
	
	rect.left   = loc.x;
	rect.top    = loc.y;
	rect.right  = ClippingRect.right;
	rect.bottom = loc.y + mLeading * (LinePointers.size() - 1);
	
	if( !UIUtils::ClipRect( rect, ClippingRect ) )
		return;
	
	// LastLineIndex - 1 can be negative, so these are signed
	long FirstLineIndex = (mLeading != 0) ? (rect.top - loc.y) / mLeading : 0;
	long LastLineIndex  = (mLeading != 0) ? (rect.bottom - loc.y) / mLeading + 2 : 2;

	UI_REPORT_LOG_PRINT (mLeading == 0, ("TextStyle [%s] has an mLeading of 0, which is BAD.\n", Unicode::wideToNarrow(mLogicalName).c_str()));

	if( LastLineIndex > static_cast<long> (LinePointers.size()) )
		LastLineIndex = LinePointers.size();
	
	UIPoint	theOutdest( loc.x, loc.y + FirstLineIndex * mLeading );
	
	RenderLineData rld(&DestinationCanvas, align, theOutdest, size.x, 0, 0, DestinationCanvas.GetColor ());
	rld.truncateElipsis  = false;
	rld.numCharsToRender = numCharsToRender;

	//-- only truncate the last line
	
	int linesDrawn = 0;
	
	for( long i = FirstLineIndex; i < (LastLineIndex - 1) && (maxLines <= 0 || linesDrawn < maxLines); ++i )
	{	
		if (rld.loc.y + mLeading < rect.top)
			continue;
		
		if (rld.loc.y > rect.bottom)
			break;
		
		rld.begin     = LinePointers [i];
		rld.nextLine  = LinePointers [i+1];
		rld.lineWidth = LineWidth    [i];		

		//increment lines drawn now before the elipsis check
		++linesDrawn;
		
		if (truncateElipsis && (linesDrawn == maxLines) && (maxLines != 0))
		{
			rld.nextLine = LinePointers.back ();
			rld.truncateElipsis = true;
		}

		RenderPrewrappedLine(rld, textMaskStyle);
		rld.loc.y += mLeading;
	}
}

//-----------------------------------------------------------------

UIString::const_iterator UITextStyle::EatEscapeSequence( const UIString::const_iterator & begin, const UIString::const_iterator & end, bool & ignoreNextEscape, long & width, long & indentation) const
{
	ignoreNextEscape = false;
	
	if( begin == end )
		return end;
	
	if( *begin != '\\' )
		return begin;
	
	UIString::const_iterator iter = begin;
	
	++iter;
	
	if( iter == end )
		return iter;
	
	if (*iter == '\\')
	{
		ignoreNextEscape = true;
		return iter;
	}
	
	if( *iter == '#' )
	{
		++iter;
		if (iter == end)
			return iter;
		
		const Unicode::unicode_char_t first_char = *iter;
		
		switch (first_char)
		{
			// Color escape sequence: \#. set to default
		case '.':
			++iter;
			break;

			// Color escape sequence: \#p<paletteentry>
		case 'p':
			
			
			for (++iter;iter != end; ++iter)
			{
				const Unicode::unicode_char_t c = *iter;
				if (c == ' ')
					break;
			}
			break;

			// Color escape sequence: \#XXXXXX
		default:
			{
				int	i;
				for( i = 0; (i < 6) && (iter != end); ++i )
					++iter;			
			}
			break;
		}
	}
	
	else if (*iter == '@')
	{
		// tab setting escape sequence \@xx

		char numbuf [3] = {0,0,0};
		
		++iter;

		if (iter != end)
		{
			numbuf [0] = static_cast<char>(*(iter++));

			if (iter != end)
			{
				numbuf [1] = static_cast<char>(*(iter++));
				numbuf [2] = 0;

				width = std::max (static_cast<long>(atoi (numbuf) * TabSize), width);
			}
		}
	}

	else if (*iter == '>')
	{
		// indentation setting escape sequence \>xxx

		char numbuf [4] = {0,0,0,0};
		
		int i = 0;
		for (++iter; iter != end && i < 3; ++iter, ++i)
			numbuf [i] = static_cast<char>(*iter);
				
		//-- indentation
		indentation = static_cast<long>(atoi (numbuf));
	}
	
	else if (*iter == '^')
	{
		// indentation setting escape sequence \^xxx

		char numbuf [4] = {0,0,0,0};
		
		int i = 0;
		for (++iter; iter != end && i < 3; ++iter, ++i)
			numbuf [i] = static_cast<char>(*iter);
				
		//-- indentation
		indentation = static_cast<long>(atoi (numbuf));
	}
	else if (*iter == '%')
	{
		// indentation setting escape sequence \%xxx

		char numbuf [4] = {0,0,0,0};
		
		int i = 0;
		for (++iter; iter != end && i < 3; ++iter, ++i)
			numbuf [i] = static_cast<char>(*iter);
				
		//-- indentation
		indentation = static_cast<long>(atoi (numbuf));
	}
	return iter;
}

// ==============================================================

UIString::const_iterator UITextStyle::ParseColorEscapeSequence( const UIString::const_iterator & start, const UIString::const_iterator & end, const UIColor & defaultColor, UIColor &CurrentColor ) const
{
	if (start == end || *start != '#')
		return start;

	// Format: #000000
	static UIString ColorString;
	ColorString.clear ();
	
	UIString::const_iterator iter = start;
	ColorString.push_back (*iter);
	++iter;
	if (iter == end)
		return iter;
	
	const Unicode::unicode_char_t first_char = *iter;
	
	switch (first_char)
	{
		// Color escape sequence: \#. set to default
	case '.':
		CurrentColor = defaultColor;
		++iter;
		break; 

		// Color escape sequence: \#p<paletteentry>
	case 'p':
		{
		ColorString.clear ();
		
		for (++iter;iter != end; ++iter)
		{
			const Unicode::unicode_char_t c = *iter;
			if (c == ' ')
			{
				++iter;
				break;
			}
			
			ColorString.push_back (c);
		}
		
		const UIPalette * const pal = UIPalette::GetInstance ();
		if (pal)
			pal->FindColor (UILowerString (Unicode::wideToNarrow (ColorString)), CurrentColor);
		}
		break;

		// Color escape sequence: \#XXXXXX
	default:
		{
			int	i;
			for( i = 0; (i < 6) && (iter != end); ++i )
			{
				const Unicode::unicode_char_t c = *iter;
				ColorString.push_back (c);
				++iter;
			}
			
			if( i == 6 )
				UIUtils::ParseColor( ColorString, CurrentColor );
		}
		break;
	}
	
	return iter;
}

//----------------------------------------------------------------------

bool UITextStyle::ParseAlignment (const Unicode::String & Value, Alignment & align)
{
	if (Unicode::caseInsensitiveCompare      (Value, AlignmentNames::Left, 0, Value.size ()))
		align = Left;
	else if (Unicode::caseInsensitiveCompare (Value, AlignmentNames::Center, 0, Value.size ()))
		align = Center;
	else if (Unicode::caseInsensitiveCompare (Value, AlignmentNames::Right, 0, Value.size ()))
		align = Right;
	else
		return false;
	
	return true;
}

//----------------------------------------------------------------------

void UITextStyle::FormatAlignment (Unicode::String & Value, const Alignment align)
{
	if (align == Left)
		Value = Unicode::narrowToWide (AlignmentNames::Left);
	else if (align == Center)
		Value = Unicode::narrowToWide (AlignmentNames::Center);
	else
		Value = Unicode::narrowToWide (AlignmentNames::Right);
}

//-----------------------------------------------------------------

UIFontCharacter *UITextStyle::GetCharacter( long code ) const
{
	if (code >= 0 && code < GLYPH_ARRAY_SIZE)
	{
		// zero is the default empty box
		if (mGlyphArray [code])
			return mGlyphArray [code];
		else
			return mGlyphArray [0];
	}
	else
	{
		GlyphMap_t::const_iterator find_iter = mGlyphMap->find (code);
		if (find_iter != mGlyphMap->end ())
			return (*find_iter).second;
		else
			return mGlyphArray [0];
	}
}

//----------------------------------------------------------------------

void UITextStyle::setEnabled (bool b)
{
	ms_enabled = b;
}

//----------------------------------------------------------------------

void  UITextStyle::setOpacityRelativeMin (float f)
{
	ms_opacityRelativeMin = f;
}

//----------------------------------------------------------------------

void UITextStyle::CopyPropertiesFrom(const UIBaseObject & rhs)
{
	UIStyle::CopyPropertiesFrom(rhs);

	if (rhs.IsA (TUITextStyle))
	{
		UITextStyle const & rhs_textStyle = static_cast<UITextStyle const &>(rhs);
		SetDropShadowDepth(rhs_textStyle.GetDropShadowDepth());
		SetDropShadowsEnabled(rhs_textStyle.GetDropShadowsEnabled());
	}
}


//----------------------------------------------------------------------

void UITextStyle::SetDropShadowDepth(const UIPoint & offset)
{
	if (mDropShadowDepth == NULL && offset != UIPoint::one)
	{
		mDropShadowDepth = new UIPoint;
	}
	
	if (mDropShadowDepth)
	{
		*mDropShadowDepth = offset;
	}
}

//----------------------------------------------------------------------

UIPoint const & UITextStyle::GetDropShadowDepth() const
{
	return mDropShadowDepth ? *mDropShadowDepth : UIPoint::one;
}

//----------------------------------------------------------------------
