#include "_precompile.h"

#include "UIFontCharacter.h"

#include "UICanvas.h"
#include "UIManager.h"
#include "UINamespace.h"
#include "UIPage.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"

#include <cassert>
#include <vector>

const char * const UIFontCharacter::TypeName                    = "FontCharacter";

//----------------------------------------------------------------------

const UILowerString UIFontCharacter::PropertyName::Code          = UILowerString ("Code");
const UILowerString UIFontCharacter::PropertyName::Advance       = UILowerString ("Advance");
const UILowerString UIFontCharacter::PropertyName::AdvancePre    = UILowerString ("AdvancePre");
const UILowerString UIFontCharacter::PropertyName::SourceRect    = UILowerString ("SourceRect");
const UILowerString UIFontCharacter::PropertyName::SourceFile    = UILowerString ("SourceFile");

//======================================================================================
#define _TYPENAME UIFontCharacter

namespace UIFontCharacterNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Code, "", T_int),
		_DESCRIPTOR(Advance, "", T_int),
		_DESCRIPTOR(AdvancePre, "", T_int),
		_DESCRIPTOR(SourceFile, "", T_string),
		_DESCRIPTOR(SourceRect, "", T_rect),
	_GROUPEND(Basic, 1, int(UIPropertyCategories::C_Basic));
	//================================================================
}
using namespace UIFontCharacterNamespace;
//======================================================================================

UIFontCharacter::UIFontCharacter() :
	UIBaseObject        (),
	mCanvas             (0),
	mLocation           (),
	mSize               (),
	mAdvance            (0),
	mAdvancePre         (0),
	mCode               (0)
{
}

//----------------------------------------------------------------------

UIFontCharacter::~UIFontCharacter()
{
	SetCanvas( 0 );
}

//----------------------------------------------------------------------

bool UIFontCharacter::IsA( const UITypeID Type ) const
{
	return (Type == TUIFontCharacter) || UIBaseObject::IsA( Type );
}

//----------------------------------------------------------------------

UIBaseObject *UIFontCharacter::Clone( void ) const
{
	return new UIFontCharacter;
}

//----------------------------------------------------------------------

void UIFontCharacter::SetCanvas( UICanvas *NewCanvas )
{
	if( NewCanvas )
		NewCanvas->Attach( this );

	if( mCanvas )
		mCanvas->Detach( this );

	mCanvas = NewCanvas;
}

//----------------------------------------------------------------------

void UIFontCharacter::SetLocation( const UIPoint &NewLocation )
{
	mLocation = NewLocation;
}

//----------------------------------------------------------------------

void UIFontCharacter::SetSize( const UISize &NewSize )
{
	mSize = NewSize;
}

//----------------------------------------------------------------------

void UIFontCharacter::SetAdvance( const long NewAdvance )
{
	assert (false);
	mAdvance = NewAdvance;
}

//----------------------------------------------------------------------

void UIFontCharacter::SetCharacterCode( const Unicode::unicode_char_t NewCode )
{
	mCode = NewCode;
}

//----------------------------------------------------------------------

void UIFontCharacter::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::SourceFile );

	UIBaseObject::GetLinkPropertyNames( In );
}

//----------------------------------------------------------------------

void UIFontCharacter::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIBaseObject::GetPropertyGroups(o_groups, category);
}

//----------------------------------------------------------------------

void UIFontCharacter::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	In.push_back( PropertyName::Code );
	In.push_back( PropertyName::Advance );
	In.push_back( PropertyName::AdvancePre );
	In.push_back( PropertyName::SourceFile );
	In.push_back( PropertyName::SourceRect );

	UIBaseObject::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

bool UIFontCharacter::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::Code )
	{
		long i;
		if (UIUtils::ParseLong( Value, i ))
		{
			mCode = static_cast<Unicode::unicode_char_t> (i);
			return true;
		}
		return false;
	}
	else if( Name == PropertyName::Advance )
		return UIUtils::ParseLong( Value, mAdvance );
	else if( Name == PropertyName::AdvancePre )
		return UIUtils::ParseLong( Value, mAdvancePre );
	else if( Name == PropertyName::SourceFile )
	{
		UICanvas *theCanvas = UIManager::gUIManager().GetCanvas( Value);

		if( theCanvas )
		{
			SetCanvas( theCanvas );
			return true;
		}

		return false;
	}
	else if( Name == PropertyName::SourceRect )
	{
		UIRect rect;

		if( !UIUtils::ParseRect( Value, rect ) )
			return false;

		SetLocation (rect.Location ());
		SetSize     (rect.Size ());
		return true;
	}
	else
		return UIBaseObject::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UIFontCharacter::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::Code )
		return UIUtils::FormatLong( Value, mCode );
	else if( Name == PropertyName::Advance )
		return UIUtils::FormatLong( Value, mAdvance );
	else if( Name == PropertyName::AdvancePre )
		return UIUtils::FormatLong( Value, mAdvancePre );
	else if( Name == PropertyName::SourceFile )
	{
		GetPathTo( Value, mCanvas );
		return true;
	}
	else if( Name == PropertyName::SourceRect )
		return UIUtils::FormatRect( Value, UIRect( mLocation, mSize));

	return UIBaseObject::GetProperty( Name, Value );
}

//----------------------------------------------------------------------
