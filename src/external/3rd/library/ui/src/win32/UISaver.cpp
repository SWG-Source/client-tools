#include "_precompile.h"

#include "UISaver.h"
#include "UITypes.h"
#include "UIWidget.h"

#include "UIPage.h"
#include "UINamespace.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UICheckbox.h"
#include "UIButton.h"
#include "UIDataSource.h"
#include "UIFontCharacter.h"
#include "UITextStyle.h"
#include "UIImageStyle.h"
#include "UIData.h"
#include "UITemplate.h"
#include "UIUnknown.h"

#include <cassert>
#include <vector>
#include <algorithm>

//-----------------------------------------------------------------
namespace UISaverNamespace
{
	namespace Unused
	{
		UILowerString PackSizeProp("PackSizeProp");
		UILowerString PackLocationProp("PackLocationProp");
		UILowerString Shear("Shear");
	}
}

using namespace UISaverNamespace;

//-----------------------------------------------------------------

UISaver::UISaver () :
m_defaultObjectMap ()
{
}

//-----------------------------------------------------------------

UISaver::~UISaver ()
{
	for (DefaultObjectMap::iterator it = m_defaultObjectMap.begin (); it != m_defaultObjectMap.end (); ++it)
	{
		(*it).second->Detach(0);
	}
}

//-----------------------------------------------------------------

void UISaver::SaveToStringSet( UINarrowStringMap &Files, UINarrowStringMap::iterator CurrentFile, UIBaseObject &Source, int indent ) const
{
	if( Source.IsA( TUIWidget ) && static_cast<UIWidget *>( &Source )->IsTransient() )
		return;

	if( Source.IsA( TUITextStyle))
		return;

	if( Source.IsA( TUINamespace) && (Source.GetName() == "Fonts"))
		return;

	UINarrowStringMap::iterator Dest = CurrentFile;
	UIString                           PropertyValue;
	UIBaseObject::UIPropertyNameVector    PropertyNames;
		
	if( Source.GetProperty( UIBaseObject::PropertyName::SourceFile, PropertyValue ) )
	{
		const UINarrowString & nstr = UIUnicode::wideToNarrow (PropertyValue);

		Dest = Files.find( nstr );

		if( Dest == Files.end() )
			Files[nstr] = UINarrowString();

		Dest = Files.find( nstr );

		if( Dest != CurrentFile )
		{
			if( CurrentFile != Files.end() )
			{
				CurrentFile->second.append (indent, '\t');
				CurrentFile->second += "<include>";
				CurrentFile->second += UIUnicode::wideToNarrow (MakeSafeDataValue( PropertyValue ));
				CurrentFile->second += "</include>\r\n";
			}
		}
	}
	else
	{
		if( Files.empty() )
		{
			Files[UINarrowString()] = UINarrowString();
			Dest = Files.begin();
		}
		else
			Dest = CurrentFile;
	}

	if( Dest->second.capacity() - Dest->second.size() < 1024 )
		Dest->second.reserve( Dest->second.capacity() * 2 );

	Dest->second.append (indent, '\t');
	Dest->second += "<";
	Dest->second += Source.GetTypeName();

	Source.GetPropertyNames( PropertyNames, false );

	if( Source.IsA( TUIUnknown ) )
	{
		// Special case for unknown objects, they don't write out their
		// typename as a property, it's written as the object type
		PropertyNames.erase( std::remove (PropertyNames.begin (), PropertyNames.end (), UIUnknown::PropertyName::TypeName), PropertyNames.end () );
	}

	{	
		UIBaseObject * Clone = 0;

		const DefaultObjectMap::const_iterator it = m_defaultObjectMap.find (Source.GetTypeName ());
		if (it != m_defaultObjectMap.end ())
		{
			Clone = (*it).second;
		}
		else
		{
			Clone = Source.Clone();
			if (Clone) 
			{
				Clone->Attach(0);
				m_defaultObjectMap.insert (std::make_pair (Source.GetTypeName (), Clone));
			}
		}
		
		for( UIBaseObject::UIPropertyNameVector::const_iterator i = PropertyNames.begin(); i != PropertyNames.end(); ++i )
		{
			UILowerString const & thePropertyName = *i;
			char const * const thePropertyNameStr = thePropertyName.c_str();

			if( Source.GetProperty( thePropertyName, PropertyValue ) )
			{
				if( PropertyValue.empty() )
					continue;
				else if( thePropertyName ==  UIBaseObject::PropertyName::SourceFile )
					continue;
				else if( Source.IsA( TUIButton ) && thePropertyName == UIButton::PropertyName::Text )
					continue;
				else if( Source.IsA( TUICheckbox ) && thePropertyName == UICheckbox::PropertyName::Text )
					continue;
				else if( Source.IsA( TUIText ) && thePropertyName == UIText::PropertyName::Text )
					continue;
				else if( Source.IsA( TUITextbox ) && thePropertyName == UITextbox::PropertyName::Text )
					continue;
				else if( Source.IsA( TUITemplate) && thePropertyName == UITemplate::PropertyName::Template )
					continue;
				else if(thePropertyName == UISaverNamespace::Unused::PackSizeProp)
					continue;
				else if(thePropertyName == UISaverNamespace::Unused::PackLocationProp)
					continue;
				else if(thePropertyName == UISaverNamespace::Unused::Shear)
					continue;
				else if(*thePropertyNameStr == '!')
					continue;

				
				UIString ClonePropertyValue;

				// Check that the value of the property is not the default value
				if( Clone && Clone->GetProperty( thePropertyName, ClonePropertyValue ) && 
					UIUnicode::icmp (PropertyValue, ClonePropertyValue) == 0)
				{
					continue;
				}

				Dest->second += "\r\n";
				Dest->second.append (indent + 1, '\t');
				Dest->second += thePropertyName.get ();
				Dest->second += "=\'";
				Dest->second += UIUnicode::wideToNarrow (MakeSafeDataValue( PropertyValue ));
				Dest->second.push_back ('\'');
			}
		}
	}

	Dest->second += "\r\n";
	Dest->second.append (indent, '\t');

	if( Source.IsA( TUIText ) || Source.IsA( TUITextbox ) || Source.IsA( TUIButton ) || Source.IsA( TUICheckbox ) || Source.IsA( TUITemplate ) )
	{
		UIString Contents;
		Dest->second += ">";

		if( Source.IsA( TUIText )	)
		{
			// TODO: tokens should be unicode? - jww
			UIString wstr;
			static_cast<UIText *>(&Source)->GetText( wstr );
			Contents.erase ();
			Contents.append (wstr.begin (), wstr.end ());
		}
		else if( Source.IsA( TUITextbox )	)
		{
			// TODO: tokens should be unicode? - jww
			UIString wstr;
			static_cast<UITextbox *>(&Source)->GetText( wstr );
			Contents.erase ();
			Contents.append (wstr.begin (), wstr.end ());
		}
		else if( Source.IsA( TUIButton ) )
		{
			// TODO: tokens should be unicode? - jww
			UIString wstr;
			static_cast<UIButton *>(&Source)->GetText( wstr );
			Contents.erase ();
			Contents.append (wstr.begin (), wstr.end ());
		}
		else if( Source.IsA( TUICheckbox ) )
		{
			// TODO: tokens should be unicode? - jww
			UIString wstr;
			static_cast<UICheckbox *>(&Source)->GetText( wstr );
			Contents.erase ();
			Contents.append (wstr.begin (), wstr.end ());
		}
		else if( Source.IsA( TUITemplate ) )
			static_cast<UITemplate *>(&Source)->GetTemplate( Contents );

		Dest->second += UIUnicode::wideToNarrow (MakeSafeDataValue( Contents ));
		Dest->second += "</";
		Dest->second += Source.GetTypeName();
		Dest->second += ">\r\n";
	}	
	else if( Source.IsA( TUITemplate ) )
	{
		UITemplate::UILoaderTokenList TemplateTokens;

		Dest->second += ">\r\n";

		static_cast<UITemplate *>(&Source)->GetTokens( TemplateTokens );

		for( UITemplate::UILoaderTokenList::iterator CurrentToken = TemplateTokens.begin(); CurrentToken != TemplateTokens.end(); ++CurrentToken )
		{
			const UINarrowString & narrowHeader = UIUnicode::wideToNarrow(CurrentToken->Header);

			if( CurrentToken->IsData )
				Dest->second += narrowHeader;
			else if( CurrentToken->IsEndContainer )
			{
				Dest->second += "</";
				Dest->second += narrowHeader;
				Dest->second += ">";
			}
			else
			{
				Dest->second += "<";
				Dest->second += narrowHeader;

				for( UILoaderToken::UIStringMap::iterator CurrentProperty = CurrentToken->Attributes.begin();
						 CurrentProperty != CurrentToken->Attributes.end(); 
						 ++CurrentProperty )
				{
					Dest->second += " ";
					Dest->second += CurrentProperty->first.get ();
					Dest->second += "='";
					Dest->second += UIUnicode::wideToNarrow (CurrentProperty->second);
					Dest->second += "'";
				}

				if( CurrentToken->IsContainer )
				{
					Dest->second += ">\n";
				}
				else
				{
					Dest->second += "/>\r\n";
				}
			}
		}

		Dest->second += "</";
		Dest->second += Source.GetTypeName();
		Dest->second += ">\r\n\r\n";
	}
	else if( Source.GetChildCount() > 0 )
	{
		Dest->second += ">\r\n";

		UIBaseObject::UIObjectList ol;
		Source.GetChildren(ol);

		for( UIBaseObject::UIObjectList::const_iterator ChildObject = ol.begin(); ChildObject != ol.end(); ++ChildObject )
			SaveToStringSet( Files, Dest, **ChildObject, indent + 1 );

		Dest->second.append (indent, '\t');
		Dest->second += "</";
		Dest->second += Source.GetTypeName();
		Dest->second += ">\r\n";
	}
	else
		Dest->second += "/>\r\n";
}

//-----------------------------------------------------------------

UIString UISaver::MakeSafeDataValue( const UIString &RawString ) const
{
	UIString CookedString;
	CookedString.reserve (RawString.size () * 3L / 2L);

	static const UIString s_open        = Unicode::narrowToWide ("\\<");
	static const UIString s_close       = Unicode::narrowToWide ("\\>");
	static const UIString s_backslash   = Unicode::narrowToWide ("\\\\");
	static const UIString s_quote       = Unicode::narrowToWide ("\\\'");
	static const UIString s_doublequote = Unicode::narrowToWide ("\\\"");

	for( UIString::const_iterator i = RawString.begin(); i != RawString.end(); ++i )
	{
		switch( *i )
		{
			case '<':
				CookedString += s_open;
				break;

			case '>':
				CookedString += s_close;
				break;

			case '\\':
				CookedString += s_backslash;
				break;

			case '\'':
				CookedString += s_quote;
				break;
		
			case '\"':
				CookedString += s_doublequote;
				break;

			default:
				CookedString += *i;
		}
	}

	return CookedString;
}

//-----------------------------------------------------------------

