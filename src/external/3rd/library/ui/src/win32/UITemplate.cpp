#include "_precompile.h"

#include "UITemplate.h"
#include "UILoader.h"
#include "UIOutputStream.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"
#include "UIWidget.h"

#include <vector>

char  UITemplate::TokenDelimitor         = '$';
const char *UITemplate::TypeName					     = "Template";

//----------------------------------------------------------------------

const UILowerString UITemplate::PropertyName::Size		 = UILowerString ("Size");
const UILowerString UITemplate::PropertyName::Template = UILowerString ("Template");

//======================================================================================
#define _TYPENAME UITemplate

namespace UITemplateNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Size, "", T_point),
		_DESCRIPTOR(Template, "", T_object),
	_GROUPEND(Basic, 1, int(UIPropertyCategories::C_Basic));
	//================================================================
}
using namespace UITemplateNamespace;
//======================================================================================

UITemplate::UITemplate()
{
	mSize.x	= 0;
	mSize.y	= 0;
}

UITemplate::~UITemplate()
{
}

bool UITemplate::IsA( const UITypeID Type ) const
{
	return (Type == TUITemplate) || UIBaseObject::IsA( Type );
}

const char *UITemplate::GetTypeName( void ) const
{
	return TypeName;
}

UIBaseObject *UITemplate::Clone( void ) const
{
	return new UITemplate;
}

void UITemplate::AddToken( const UILoaderToken &t )
{
	mTemplateTokens.push_back( t );
}

void UITemplate::SetSize( const UISize &In )
{
	mSize = In;
}

void UITemplate::Instanciate( UILoader &LoaderContext, const UIBaseObject &ValueSource, UIWidgetVector *TopLevelObjects ) const
{
	for( UILoaderTokenList::const_iterator i = mTemplateTokens.begin(); i != mTemplateTokens.end(); ++i )
	{
		UILoaderToken CompletedToken;

		CompletedToken.IsContainer		= i->IsContainer;
		CompletedToken.IsData					= i->IsData;
		CompletedToken.IsEndContainer = i->IsEndContainer;

		UIString tmpString;
		FillOutTemplate( ValueSource, i->Header, tmpString );
		CompletedToken.Header = tmpString;

		for( UILoaderToken::UIStringMap::const_iterator j = i->Attributes.begin(); j != i->Attributes.end(); ++j )
		{
			UIString s;
			FillOutTemplate( ValueSource, j->second, s );
			CompletedToken.Attributes[j->first] = s;
		}

		UIBaseObject *o;
		LoaderContext.LoadFromToken( CompletedToken, o );

		if( o )
		{
			o->Attach(0);
			if( TopLevelObjects && o->IsA( TUIWidget ) )
			{
				TopLevelObjects->push_back( static_cast<UIWidget *>( o ) );
			}
			else
				o->Detach(0);
		}
	}
}

void UITemplate::FillOutTemplate( const UIBaseObject &ValueSource, const UIString &In, UIString &Out ) const
{
	for( UIString::const_iterator i = In.begin(); i != In.end(); ++i )
	{
		if( *i != TokenDelimitor )
			Out += *i;
		else
		{
			UIString Name;

			++i;

			for( ; (i != In.end()) && (*i != TokenDelimitor); ++i )
				Name += *i;

			UIString Value;
			ValueSource.GetProperty( UILowerString (Unicode::wideToNarrow (Name)), Value );
			Out += Value;

			if( i == In.end() )
				return;
		}
	}
}

//----------------------------------------------------------------------

void UITemplate::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::Template );

	UIBaseObject::GetLinkPropertyNames( In );
}

//----------------------------------------------------------------------

void UITemplate::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIBaseObject::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UITemplate::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	In.push_back( PropertyName::Size );
	In.push_back( PropertyName::Template );

	UIBaseObject::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

bool UITemplate::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::Size )
		return UIUtils::ParsePoint( Value, mSize );
	else if( Name == PropertyName::Template )
	{
		SetTemplate( Value );
		return true;
	}
	else
		return UIBaseObject::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UITemplate::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::Size )
		return UIUtils::FormatPoint( Value, mSize );
	else if( Name == PropertyName::Template )
	{
		GetTemplate( Value );
		return true;
	}
	return UIBaseObject::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

void UITemplate::GetTokens( UILoaderTokenList &Out ) const
{
	for( UILoaderTokenList::const_iterator i = mTemplateTokens.begin(); i != mTemplateTokens.end(); ++i )
		Out.push_back( *i );
}

void UITemplate::GetTemplate( UIString &Out ) const
{
	for( UILoaderTokenList::const_iterator CurrentToken = mTemplateTokens.begin(); CurrentToken != mTemplateTokens.end(); ++CurrentToken )
	{
		if( CurrentToken != mTemplateTokens.begin() )
		{
			if( Out.length() >= 2 )
			{
				UIString::value_type lastchar = Out[Out.length() - 1];

				if( lastchar != '\n' )
					Out.append( UI_UNICODE_T ("\r\n"));
			}
			else
				Out.append( UI_UNICODE_T ("\r\n"));
		}

		UINarrowString tmpStr;

		CurrentToken->WriteToString( tmpStr );

		Out += UIUnicode::narrowToWide (tmpStr);
	}
}

void UITemplate::SetTemplate( const UIString &NewTemplate )
{
	mTemplateTokens.clear();

	UINarrowString narrowNewTemplate = UIUnicode::wideToNarrow (NewTemplate);

	for( UINarrowString::const_iterator p = narrowNewTemplate.begin(); p != narrowNewTemplate.end();  )
	{
		UILoaderToken	NextToken;
		UINarrowString			ParseError;

		if( !NextToken.CreateFromXML( narrowNewTemplate, p, ParseError ) )
		{
			if( ParseError.empty() )
				continue;
			else
			{
				*GetUIOutputStream() << ParseError << '\n';
				return;
			}
		}
		else
			AddToken( NextToken );
	}

	SendNotification( UINotification::ObjectChanged, this );
}