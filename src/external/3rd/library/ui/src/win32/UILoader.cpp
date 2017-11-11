#include "_precompile.h"
#include "UILoader.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UILoaderSetup.h"
#include "UILoaderToken.h"
#include "UIPage.h"
#include "UISystemDependancies.h"
#include "UITemplate.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UIUnknown.h"
#include <cassert>
#include <cstdio>
#include <algorithm>

#define UI_LOADER_PROFILE 1
#define UI_LOADER_LINT    0

//----------------------------------------------------------------------

namespace
{
	
	const char * const UnknownTag1 = "Encountered unknown tag '";
	const char * const UnknownTag2 = "'";
	const char * const UnwrappedData = "Encountered data outside of container tags";
	const char * const BadDataInTag1 = "Encountered data in tag '";
	const char * const BadDataInTag2 = "', tags of that type cannot contain data";
	const char * const BadTagInTag1 = "Encountered tag in tag '";
	const char * const BadTagInTag2 = "', tags of that type cannot contain other tags";
	
	const char * const TagInFailedContainer1 = "Warning: Encountered tag '";
	const char * const TagInFailedContainer2 = "' in container '";
	const char * const TagInFailedContainer3 = "', the container failed to create so the contained object will be ignored.";
	
	const char * const UnmatchedEndContainer = "Encountered unmatched end of container";
	const char * const UnmatchedEndContainerExpected1 = "Encountered </";
	const char * const UnmatchedEndContainerExpected2 = "> which does not close open container '";
	const char * const UnmatchedEndContainerExpected3 = "' of type ";
	const char * const UnclosedContainer1 = "End of file encountered, container '";
	const char * const UnclosedContainer2 = "' was not closed";
	
	const char * const ObjectCreationFailed1 = "Creation of object having type '";
	const char * const ObjectCreationFailed2 = "' failed";
	
	const char * const ObjectRedefinition1 = "Warning: Redefinition of object '";
	const char * const ObjectRedefinition2 = "', the redefinition will be ignored";

	struct LoadInfo
	{
		long tickTotal;
		int  count;
	};

	typedef ui_stdmap<UILowerString, LoadInfo>::fwd LoadMap;
	LoadMap s_loadMap;
}

//----------------------------------------------------------------------

UILoader::UILoader()
{
	UILoaderSetup::performSetup (*this);	
}

//----------------------------------------------------------------------

UILoader::~UILoader()
{
}

//----------------------------------------------------------------------

void UILoader::AddToken( const char * const name, const UILoaderExtension *Constructor )
{
	assert (mConstructorMap.find (name) == mConstructorMap.end ());

	mConstructorMap[name] = Constructor;
}

//----------------------------------------------------------------------

UIBaseObject * UILoader::CreateObject (const char * const name)
{
	assert (name);

	const UIConstructorMap::iterator it = mConstructorMap.find (name);

	if (it != mConstructorMap.end ())
		return (*it).second->Create ();

	return 0;
}

//----------------------------------------------------------------------

bool UILoader::LoadStringFromResource( const UINarrowString &ResourceName, UINarrowString &Out )
{		
	FILE *fp = fopen( ResourceName.c_str(), "rb" );

	if( !fp )
		return false;
	
	fseek( fp, 0, SEEK_END );
	long len = ftell( fp );
	fseek( fp, 0, SEEK_SET );
	
	char *rawdata = new char[len+1];
	fread( rawdata, len, 1, fp );
	rawdata[len] = 0;
	fclose( fp );

	Out = rawdata;
	delete rawdata;

	return true;
}

//----------------------------------------------------------------------

bool UILoader::LoadFromResource( const UINarrowString &ResourceName, UIObjectList &TopLevelObjects, bool SetSourceFileProperty )
{
	const long tickStart = UISystemDependancies::Get ().GetTickCount ();

	for( UINarrowStringVector::iterator i = mIncludes.begin(); i != mIncludes.end(); ++i )
	{
		if( *i == ResourceName )
		{
			*GetUIOutputStream() << " :: Circular include of resource '" << ResourceName << "'.  Include stack was:\n";

			for( UINarrowStringVector::iterator j = mIncludes.begin(); j != mIncludes.end(); ++j )
				*GetUIOutputStream() << "   " << *j << '\n';

			return false;
		}
	}

	{
		if (mIncludeHistory.find(ResourceName) != mIncludeHistory.end())
			return true;
	}

	mIncludes.push_back( ResourceName );
	mIncludeHistory.insert( ResourceName );

	std::string data;
	if( !LoadStringFromResource( ResourceName, data ) )
	{
		*GetUIOutputStream() << " :: Could not access resource " << ResourceName << '\n';
		return false;
	}

	const bool rc = LoadFromString( data, TopLevelObjects, SetSourceFileProperty );
	mIncludes.pop_back();

	if (!rc)
	{
		*GetUIOutputStream() << " :: Failed to load data from resource: " << ResourceName << "\n";
	}

	const long tickEnd = UISystemDependancies::Get ().GetTickCount ();

	UI_UNREF (tickEnd);
	UI_UNREF (tickStart);

#if UI_LOADER_PROFILE
	*GetUIOutputStream() << " :: UI_LOAD_TIME " << ResourceName << " " << (tickEnd - tickStart) << "\n";
#endif

	return rc;
}

//----------------------------------------------------------------------

bool UILoader::LoadFromString( const UINarrowString &data, UIObjectList &TopLevelObjects, bool SetSourceFileProperty )
{		
	bool ProcessingInclude = false;

	UILoaderToken   NextToken;

	for( UINarrowString::const_iterator p = data.begin(); p != data.end();  )
	{
		std::string     ParseError;

		if( !NextToken.CreateFromXML( data, p, ParseError ) )
		{
			if( ParseError.empty() )
				continue;
			else
			{
				*GetUIOutputStream() << " :: Bad data at position " << std::distance (data.begin (), p) << "\n";
				*GetUIOutputStream() << " :: " << ParseError << '\n';
				return false;
			}
		}

		const UINarrowString & narrowHeader = UIUnicode::wideToNarrow(NextToken.Header);

		if( !mContainerStack.empty() && mContainerStack.back () && mContainerStack.back ()->IsA (TUITemplate) )
		{
			if( NextToken.IsEndContainer && !_stricmp( narrowHeader.c_str(), "template" ) )
			{
				mContainerNameStack.pop();
				mContainerTypeStack.pop();
				mContainerStack.pop_back();
			}
			else	
				static_cast<UITemplate *>(mContainerStack.back ())->AddToken (NextToken);
		}
		else
		{
			if( NextToken.IsContainer && !_stricmp( narrowHeader.c_str(), "include" ) )
				ProcessingInclude = true;
			else if( NextToken.IsEndContainer && !_stricmp( narrowHeader.c_str(), "include" ) )
				ProcessingInclude = false;
			else if( NextToken.IsData && ProcessingInclude )
			{
				if( !LoadFromResource( narrowHeader, TopLevelObjects, SetSourceFileProperty ) )
				{
					*GetUIOutputStream() << " :: Failed to load data from included resource: " << narrowHeader << "\n";
					return false;
				}
			}
			else
			{
				UIBaseObject *o;

				if( !LoadFromToken( NextToken, o, SetSourceFileProperty ) )
				{
					*GetUIOutputStream() << " :: Failed to load from token:\n" << NextToken.Header << "\n";
					return false;
				}

				if( o )
					TopLevelObjects.push_back( o );
			}
		}
	}

	if( mIncludes.size() <= 1 )
	{
		while( !mContainerStack.empty() )
		{
			*GetUIOutputStream() << " :: " << UnclosedContainer1 << mContainerNameStack.top() << UnclosedContainer2 << '\n';

			mContainerNameStack.pop();
			mContainerTypeStack.pop();
			mContainerStack.pop_back();
		}

#if UI_LOADER_LINT
		for( UIObjectList::iterator i = TopLevelObjects.begin(); i != TopLevelObjects.end(); ++i )
			(*i)->Link();
#endif

	}
	return true;
}

//----------------------------------------------------------------------

bool UILoader::LoadFromToken( const UILoaderToken &NextToken, UIBaseObject *&TopLevelObjectOut, bool SetSourceFileProperty )
{
	TopLevelObjectOut = 0;

	const long tickStart = UISystemDependancies::Get ().GetTickCount ();

	const UINarrowString & narrowHeader = UIUnicode::wideToNarrow(NextToken.Header);

	if( NextToken.IsData )
	{
		if( mContainerStack.empty() )
			*GetUIOutputStream() << UnwrappedData << '\n';
		else
		{
			UIBaseObject *o = mContainerStack.back ();

			if( !o )
			{
				if( mContainerNameStack.top().empty() )
					*GetUIOutputStream() << "Warning: Encountered data '" << narrowHeader << "' in unnamed container of type '" << mContainerTypeStack.top() << "', the container failed to create so the contained data will be ignored.\n";
				else
					*GetUIOutputStream() << "Warning: Encountered data '" << narrowHeader << "' in container '" << mContainerNameStack.top() << "', the container failed to create so the contained data will be ignored.\n";
			}
			else if( o->IsA( TUIText ) )
			{
				static_cast<UIText *>(o)->SetText(NextToken.Header);
			}
			else if( o->IsA( TUITextbox ) )
			{
				static_cast<UITextbox *>(o)->SetText(NextToken.Header);
			}
			else if( o->IsA( TUIButton ) )
			{
				static_cast<UIButton *>(o)->SetText(NextToken.Header);
			}
			else if( o->IsA( TUICheckbox ) )
			{
				static_cast<UICheckbox *>(o)->SetText(NextToken.Header);
			}
			else
				*GetUIOutputStream() << BadDataInTag1 << mContainerNameStack.top() << BadDataInTag2 << '\n';
		}
		return true;
	}
	else if( NextToken.IsEndContainer )
	{
		if( mContainerStack.empty() )
			*GetUIOutputStream() << UnmatchedEndContainer << '\n';
		else
		{
			if( mContainerTypeStack.top() == narrowHeader )
			{
				mContainerNameStack.pop();
				mContainerTypeStack.pop();
				mContainerStack.pop_back();
			}
			else
			{
				*GetUIOutputStream() << UnmatchedEndContainerExpected1 << narrowHeader;
				*GetUIOutputStream() << UnmatchedEndContainerExpected2 << mContainerNameStack.top();
				*GetUIOutputStream() << UnmatchedEndContainerExpected3 << mContainerTypeStack.top() << '\n';
			}
		}
		return true;
	}
	 
	//-- else
	
	UIBaseObject								*NewObject;
	
//	static const std::string nameProperty = "name";

	UILoaderToken::UIStringMap::const_iterator  NameAttribute = NextToken.Attributes.find (UIBaseObject::PropertyName::Name);
	UINarrowString										 ObjectName;
	
	if( NameAttribute != NextToken.Attributes.end() )
		ObjectName = UIUnicode::wideToNarrow (NameAttribute->second);
	
	const UIConstructorMap::iterator i = mConstructorMap.find (narrowHeader.c_str ());
	
	if( i == mConstructorMap.end() )
	{
		NewObject = new UIUnknown;
		
		if( NewObject )
			static_cast<UIUnknown *>( NewObject )->SetTypeName( narrowHeader.c_str() );
	}
	else
	{
		NewObject	= i->second->Create();
	}
	
	if( !NewObject )
	{
		*GetUIOutputStream() << ObjectCreationFailed1 << narrowHeader << ObjectCreationFailed2 << '\n';
		
		if( NextToken.IsContainer )
		{
			mContainerNameStack.push( ObjectName );
			mContainerTypeStack.push( narrowHeader );
			mContainerStack.push_back ( 0 );
		}
		return true;
	}
	
	NewObject->SetName( ObjectName );
	
	if( SetSourceFileProperty )
	{				
		if( mIncludes.empty() )
			NewObject->SetProperty( UIBaseObject::PropertyName::SourceFile, UIString () );
		else
			NewObject->SetProperty( UIBaseObject::PropertyName::SourceFile, UIUnicode::narrowToWide (mIncludes.back()) );
	}
	
	if( mContainerNameStack.empty() )
		TopLevelObjectOut = NewObject;
	
	if( !mContainerStack.empty() )
		NewObject->SetParent( mContainerStack.back () );
	
	const UILoaderToken::UIStringMap::const_iterator end = NextToken.Attributes.end();
	for( UILoaderToken::UIStringMap::const_iterator Attrib = NextToken.Attributes.begin(); Attrib != end; ++Attrib )
		NewObject->SetProperty(Attrib->first, Attrib->second );
	
	AddToCurrentContainer( NextToken, NewObject );
	
	if( NextToken.IsContainer )
	{
		mContainerNameStack.push( ObjectName );
		mContainerTypeStack.push( narrowHeader );
		mContainerStack.push_back (NewObject);
	}

	const long tickEnd = UISystemDependancies::Get ().GetTickCount ();

	UI_UNREF (tickEnd);
	UI_UNREF (tickStart);

#if UI_LOADER_PROFILE

	const UILowerString lowerTypename (narrowHeader);

	const LoadMap::iterator it = s_loadMap.find (lowerTypename);

	if (it != s_loadMap.end ())
	{
		LoadInfo & li = (*it).second;
		li.tickTotal += (tickEnd - tickStart);
		++li.count;
	}
	else
	{
		LoadInfo li;
		li.tickTotal = (tickEnd - tickStart);
		li.count = 1;
		s_loadMap.insert (std::make_pair (lowerTypename, li));
	}
#endif

	return true;
}

//----------------------------------------------------------------------

bool UILoader::AddToCurrentContainer( const UILoaderToken &T, UIBaseObject *NewObject )
{
	if( !mContainerStack.empty() )
	{
		UIBaseObject * const o = mContainerStack.back ();

		if( !o )
			*GetUIOutputStream() << TagInFailedContainer1 << T.Header << TagInFailedContainer2 << mContainerNameStack.top() << TagInFailedContainer3 << '\n';					
		else if( !o->AddChild( NewObject ) )
		{
			*GetUIOutputStream() << BadTagInTag1 << mContainerNameStack.top() << BadTagInTag2 << '\n';
			return false;
		}
		return true;
	}
	else
	{
		return false;
	}
}

//----------------------------------------------------------------------

UIPage *UILoader::LoadRootPage( const UINarrowString &ResourceName )
{
	s_loadMap.clear ();
	UIObjectList TopLevelObjects;

	LoadFromResource( ResourceName, TopLevelObjects );

#if UI_LOADER_PROFILE
	char buf [256];

	for (LoadMap::const_iterator it = s_loadMap.begin (); it != s_loadMap.end (); ++it)
	{
		const UILowerString & lstr = (*it).first;
		const LoadInfo & li = (*it).second;
		_snprintf (buf, 256, ":: UI_LOAD_TYPE: %30s %5d %5d %2.5f\n", lstr.get ().c_str (), li.count, li.tickTotal, (static_cast<float>(li.tickTotal) / static_cast<float>(li.count)));
		*GetUIOutputStream() << buf;
	}
#endif

	Lint ();

	int numDestroyed = 0;

	for( UIObjectList::iterator i = TopLevelObjects.begin(); i != TopLevelObjects.end(); ++i )
	{
		UIBaseObject *o = *i;

		if( o->IsA( TUIPage ) )
			return static_cast<UIPage *>( o );
		else
		{
			if( o->GetRefCount() == 0 )
			{
				o->Destroy();
				++numDestroyed;
			}
		}
	}

	*GetUIOutputStream() << " :: UI_LOAD_CLEANUP_DESTROY: " << numDestroyed << "\n";

	return 0;
}

//----------------------------------------------------------------------

namespace
{
#if UI_LOADER_LINT
	static const std::string skips [] = 
	{
		"/styles.icon.state",
			"/styles.icon.posture",
			"/styles.icon.command",
			"/styles.chatBubbles",
			"/styles.icon.fallback",
			"/Styles.New.active.rs_default",
			"/styles.cursors.activate",
			"/styles.cursors.attack",
			"/styles.cursors.deactivate",
			"/styles.cursors.drop",
			"/styles.cursors.equip",
			"/styles.cursors.info",
			"/styles.cursors.loot",
			"/styles.cursors.mission_details",
			"/styles.cursors.open",
			"/styles.cursors.pickup",
			"/styles.cursors.trade_accept",
			"/styles.cursors.trade_start",
			"/styles.cursors.unequip",
			"/styles.cursors.use",
			"/styles.cursors.use_eat",
			"/styles.cursors.use_throw",
			"/styles.cursors.activate",
			"/styles.radialmenu.default.style",
			"/styles.cursors.equip",
			"/styles.cursors.reticle_default",
			"/styles.cursors.equip"
	};
	
	const int numSkips = static_cast<int>(sizeof (skips) / sizeof (skips [0]));
	
	bool init = false;
	typedef ui_stdvector<std::string>::fwd StringVector;
	StringVector sv;
	
	void doInit ()
	{
		if (!init)
		{
			for (int i = 0; i < numSkips; ++i)
			{
				sv.push_back (Unicode::toLower (skips [i]));
			}

			std::sort (sv.begin (), sv.end ());
			init = true;
		}

	}
	
	bool isSkipLower (const std::string & str)
	{
		if (!init)
			doInit ();

		if (!strncmp (str.c_str (), "/styles.icon", 12))
			return true;

		if (!strncmp (str.c_str (), "/styles.chatbubbles", 19))
			return true;

		return std::binary_search (sv.begin (), sv.end (), str);
	}

	bool addSkip (const std::string & str)
	{
		if (!init)
			doInit ();

		if (!isSkipLower (str))
		{
			sv.push_back (str);
			std::sort (sv.begin (), sv.end ());
			return true;
		}
		return false;
	}
#endif
}

//----------------------------------------------------------------------

void UILoader::Lint () const
{

#if UI_LOADER_LINT
	//-- button styles
	UIBaseObject::UIObjectVector ov;
	UIBaseObject::GetOutstandingObjects (ov);

	/*
	typedef ui_stdvector<const UIComboBox *>::fwd    ComboBoxVector;
	typedef ui_stdvector<const UIButton *>::fwd      ButtonVector;
	typedef ui_stdvector<const UIButtonStyle *>::fwd ButtonStyleVector;
	typedef ui_stdvector<const UIImageStyle *>::fwd  ImageStyleVector;
	typedef ui_stdvector<const UITableHeader *>::fwd TableHeaderVector;
	typedef ui_stdvector<const UIStyle *>::fwd       StyleVector;
*/

	char buf [256];

	{
		for (UIBaseObject::UIObjectVector::const_iterator it = ov.begin (); it != ov.end (); ++it)
		{
			UIBaseObject * const obj = *it;

			const UIBaseObject::UIPropertyNameMap  * const pmap = obj->GetPropertyMap ();

			if (pmap)
			{
				for (UIBaseObject::UIPropertyNameMap::const_iterator pit = pmap->begin (); pit != pmap->end (); ++pit)
				{
					const Unicode::String & value = (*pit).second;

					if (!value.empty () && value [0] == '/')
					{
						const std::string & narrow = Unicode::toLower (Unicode::wideToNarrow (value));

						if (strncmp (narrow.c_str (), "/styles.", 8))
							continue;

						addSkip (narrow);
					}
				}
			}

			if (obj->IsA (TUIStyle))
			{
				const UIStyle * const style = static_cast<const UIStyle *>(obj);

				if (style->IsA (TUITextStyle))
					continue;

				if (style->GetRefCount () == 1)
				{
					const std::string & fullPath = style->GetFullPath ();

					if (isSkipLower (Unicode::toLower (fullPath)))
						continue;

					_snprintf (buf, 256, ":: UI_LINT: %-20s %-60s\n", style->GetTypeName (), style->GetFullPath ().c_str ());
					*GetUIOutputStream() << buf;
				}
			}
		}
	}
#endif
}

//----------------------------------------------------------------------
