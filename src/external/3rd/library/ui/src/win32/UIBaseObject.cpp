#include "_precompile.h"

#include "UIBaseObject.h"
#include "UIData.h"
#include "UIManager.h"
#include "UIOutputStream.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"
#include "UIPalette.h"
#include "UIPropertyDescriptor.h"

#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

//-----------------------------------------------------------------
#define UI_BASE_OBJECT_USE_LEAK_FINDER 0
// NOTE: to use this you must temporarily hack the project with the
// sharedDebug include path.
//-----------------------------------------------------------------
#if UI_BASE_OBJECT_USE_LEAK_FINDER
	typedef unsigned long uint32; // this is to get us out of including sharedFoundation.
	#include "sharedDebug/LeakFinder.h"
#endif
//-----------------------------------------------------------------

UI_MEMORY_BLOCK_MANAGER_IMPLEMENTATION(UIBaseObject, 8);

//-----------------------------------------------------------------

const char * const UIBaseObject::TypeName                       = "BaseObject";
const UILowerString  UIBaseObject::PropertyName::Name           = UILowerString ("Name");
const UILowerString  UIBaseObject::PropertyName::SourceFile     = UILowerString ("ObjectLoadedFrom");

//-----------------------------------------------------------------

const UILowerString UIBaseObject::CategoryName::Base            = UILowerString ("Base");

//-----------------------------------------------------------------
#define _TYPENAME UIBaseObject

namespace UIBaseObjectNamespace
{
	typedef std::vector<UIBaseObject *> UIObjectVector;
	typedef std::set<UIBaseObject *> UIObjectSet;
	
	UIObjectSet * s_outstandingObjects = NULL;

	struct UIBaseObjectHash
	{
		size_t operator()(UIBaseObject const * x) const 
		{ 
			return reinterpret_cast<size_t>(x) >> 4; 
		}
	};

	typedef std::unordered_map<UIBaseObject const * /*child*/, UIBaseObject * /*root*/, UIBaseObjectHash> UIRootObjectMap;
	typedef std::unordered_set<UIBaseObject * /*child*/, UIBaseObjectHash> UIObjectHashSet;
	size_t const s_defaultWidgetObjects = 8192;
	UIRootObjectMap s_uiRootObjectMap(s_defaultWidgetObjects);
	UIObjectHashSet s_deletedObjects;

	unsigned short const s_maxObjectReferences = 11264;

	//-----------------------------------------------------------------

	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Name, "This object's name.", T_string),
		UIPropertyDescriptor(
			UIBaseObject::PropertyName::SourceFile.c_str(), // property name
			"This object's source file.", // description
			UIPropertyTypes::T_string, // type
			UIPropertyDescriptor::F_READ_ONLY // readOnly
		)
	_GROUPEND(Basic, 0, 0);
	//================================================================

#if UI_BASE_OBJECT_USE_LEAK_FINDER
	LeakFinder s_leakFinder;
#endif
} 

using namespace UIBaseObjectNamespace;

//----------------------------------------------------------------------

UIBaseObject::UIBaseObject() :
#if UI_USE_PROFILER
mFullPath   (0),
#endif
mReferences (0),
mName       (),
mParent     (0),
mProperties (0),
mDestroyed(DEBUG_NOT_DESTROYED_VALUE)
{
#if UI_BASE_OBJECT_USE_LEAK_FINDER
	s_leakFinder.onAllocate(this);
#endif
#if UI_USE_PROFILER
	mFullPath = new char [5];
	strcpy (mFullPath, "root");
#endif

#if _DEBUG
	if (!s_outstandingObjects)
		s_outstandingObjects = new UIObjectSet;

	s_outstandingObjects->insert(this);
#endif
}

//----------------------------------------------------------------------

UIBaseObject::UIBaseObject (const UIBaseObject & rhs) :
mReferences (0),
mName       (rhs.mName),
mParent     (0),
mProperties (0),
mDestroyed(DEBUG_NOT_DESTROYED_VALUE)
{
#if UI_BASE_OBJECT_USE_LEAK_FINDER
	s_leakFinder.onAllocate(this);
#endif
	CopyPropertiesFrom (rhs);
}

//----------------------------------------------------------------------

UIBaseObject & UIBaseObject::operator=    (const UIBaseObject & rhs)
{
	DEBUG_DESTROYED();

	if (mProperties)
	{
		delete mProperties;
		mProperties = 0;
	}

	CopyPropertiesFrom (rhs);

	return *this;
}

//-----------------------------------------------------------------

UIBaseObject::~UIBaseObject()
{
	DEBUG_DESTROYED();

	mDestroyed = DEBUG_DESTROYED_VALUE;

#if UI_USE_PROFILER
	delete [] mFullPath;
	mFullPath = 0;
#endif

	assert( mReferences == 0 ); //lint !e1924 // C-style cast MSVC bug

	delete mProperties;
	mProperties = 0;

	UIManager::gUIManager().CancelEffectorsFor( this );

	mParent = 0;

#if _DEBUG
	assert(s_outstandingObjects); //lint !e1924 // C-style cast MSVC bug

	s_outstandingObjects->erase (this);

	if (s_outstandingObjects->empty())
	{
		delete s_outstandingObjects;
		s_outstandingObjects = NULL;
	}
#endif

	s_uiRootObjectMap.erase(this);

#if UI_BASE_OBJECT_USE_LEAK_FINDER
	s_leakFinder.onFree(this);
#endif
}

//-----------------------------------------------------------------

const char *UIBaseObject::GetTypeName( void ) const
{
	DEBUG_DESTROYED();
	return TypeName;
}

//-----------------------------------------------------------------

void UIBaseObject::Destroy( void )
{
	DEBUG_DESTROYED();
	s_deletedObjects.insert(this);
}

//-----------------------------------------------------------------

void UIBaseObject::Attach( const UIBaseObject *o )
{
	DEBUG_DESTROYED();

	UI_UNREF (o);

	// It is illegal to attach to ourselves
	assert( o != this ); //lint !e1924 // C-style cast MSVC bug

	++mReferences;

	// Watch out for objects with > 32k references.
	assert( mReferences < s_maxObjectReferences ); //lint !e1924 // C-style cast MSVC bug

	// Watch out for rollover, if this ever happens increase the size of mReferences to be 32 bits
	assert( mReferences != 0 ); //lint !e1924 // C-style cast MSVC bug
}

//-----------------------------------------------------------------

bool UIBaseObject::Detach( const UIBaseObject * )
{
	DEBUG_DESTROYED();

	// Watch out for objects with < 0 references.
	assert( mReferences > 0);

	if( !--mReferences )
	{
		Destroy();
		return true;
	}

	return false;
}

//-----------------------------------------------------------------

void UIBaseObject::SetName( const UINarrowString &In )
{
	DEBUG_DESTROYED();

	mName = In;
}

//-----------------------------------------------------------------

bool UIBaseObject::IsName( const char * const In ) const
{
	DEBUG_DESTROYED();

	return In && _stricmp (mName.c_str (), In) == 0;
}

//-----------------------------------------------------------------

bool UIBaseObject::IsName( const char * const In, const unsigned long l ) const
{
	DEBUG_DESTROYED();

	if (!In || l != mName.length() )
		return false;

	return _strnicmp (mName.c_str (), In, l) == 0;
}

//----------------------------------------------------------------------

void UIBaseObject::GetLinkPropertyNames  (UIPropertyNameVector & In) const
{
	DEBUG_DESTROYED();

	std::sort (In.begin (), In.end ());
	In.erase (std::unique (In.begin (), In.end ()), In.end ());
}

//----------------------------------------------------------------------

void UIBaseObject::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	DEBUG_DESTROYED();

	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UIBaseObject::GetPropertyNames (UIPropertyNameVector &In, bool forCopy) const
{
	DEBUG_DESTROYED();

	if (!forCopy)
		In.push_back (PropertyName::Name);
		
	if( mProperties )
	{
		for( UIPropertyNameMap::const_iterator i = mProperties->begin(); i != mProperties->end(); ++i )
		{
			const UILowerString & name = (*i).first;
			In.push_back (name);
		}
	}

	std::sort (In.begin (), In.end ());
	In.erase (std::unique (In.begin (), In.end ()), In.end ());
}

//----------------------------------------------------------------------

void UIBaseObject::GetCategories(UIPropertyCategories::CategoryMask &o_categories) const
{
	DEBUG_DESTROYED();

	o_categories.setHasBasic(true);
}

//----------------------------------------------------------------------

void UIBaseObject::GetPropertiesInCategory (UIPropertyCategories::Category category, UIPropertyNameVector & In) const
{
	DEBUG_DESTROYED();

	if (category == UIPropertyCategories::C_Basic)
	{
		In.push_back(PropertyName::Name);
	}
}

//----------------------------------------------------------------------

bool UIBaseObject::SetPropertyBoolean ( const UILowerString & Name, bool Value )
{
	DEBUG_DESTROYED();

	UIString str;
	return UIUtils::FormatBoolean (str, Value) && SetProperty (Name, str);
}

//----------------------------------------------------------------------

bool UIBaseObject::SetPropertyColor   ( const UILowerString & Name, const UIColor &Value )
{
	DEBUG_DESTROYED();
	
	UIString str;
	return UIUtils::FormatColor (str, Value) && SetProperty (Name, str);
}

//----------------------------------------------------------------------

bool UIBaseObject::SetPropertyFloat   ( const UILowerString & Name, float Value )
{
	DEBUG_DESTROYED();
	
	UIString str;
	return UIUtils::FormatFloat (str, Value) && SetProperty (Name, str);
}

//----------------------------------------------------------------------

bool UIBaseObject::SetPropertyInteger ( const UILowerString & Name, int   Value )
{
	DEBUG_DESTROYED();
	
	UIString str;
	return UIUtils::FormatInteger (str, Value) && SetProperty (Name, str);
}

//----------------------------------------------------------------------

bool UIBaseObject::SetPropertyLong    ( const UILowerString & Name, long Value )
{
	DEBUG_DESTROYED();
	
	UIString str;
	return UIUtils::FormatLong (str, Value) && SetProperty (Name, str);
}

//----------------------------------------------------------------------

bool UIBaseObject::SetPropertyNarrow  ( const UILowerString & Name, const UINarrowString &Value)
{
	DEBUG_DESTROYED();
	
	return SetProperty (Name, Unicode::narrowToWide (Value));
}

//----------------------------------------------------------------------

bool UIBaseObject::SetPropertyPoint   ( const UILowerString & Name, const UIPoint & Value )
{
	DEBUG_DESTROYED();
	
	UIString str;
	return UIUtils::FormatPoint (str, Value) && SetProperty (Name, str);
}

//----------------------------------------------------------------------

bool UIBaseObject::SetPropertyRect    ( const UILowerString & Name, const UIRect &Value )
{
	DEBUG_DESTROYED();
	
	UIString str;
	return UIUtils::FormatRect (str, Value) && SetProperty (Name, str);
}

//-----------------------------------------------------------------

bool UIBaseObject::SetProperty( const UILowerString & Name, const UIString &Value )
{
	DEBUG_DESTROYED();
	
	if( Name.startsWith ('/'))
	{
		UIBaseObject *RootObject = GetRoot();

		if( RootObject )
			return RootObject->SetProperty( UILowerString (Name.c_str () + 1), Value );

		return SetProperty( UILowerString (Name.c_str () + 1), Value );
	}

	if( Name.equals ( "../", 3 ))
	{
		if( mParent )
			return mParent->SetProperty( UILowerString (Name.c_str () + 3), Value );

		return false;
	}

	if( Name.equals ( "parent.", 7 ))
	{
		if( mParent )
			return mParent->SetProperty( UILowerString (Name.c_str () + 7), Value );

		return false;
	}

	// TODO: should this be allowed?
	if( Name == PropertyName::Name )
	{
		SetName( UIUnicode::wideToNarrow (Value) );
		return true;
	}

	if( Value.empty() )
	{
		RemoveProperty( Name );
		return true;
	}

	const bool mightHaveProperty = mProperties != 0;
	if( !mProperties )
		mProperties = new UIPropertyNameMap;

	assert (mProperties); //lint !e1924 // C-style cast MSVC bug

#if 0

	if (Name.get ().find ('.') != std::string::npos)
		*GetUIOutputStream () << "Warning: suspicious property: [" << GetFullPath () << "].[" << Name.get () << "]\n";
#endif

	if (mightHaveProperty)
	{
		const UIPropertyNameMap::iterator it = mProperties->find( Name );
		if( it != mProperties->end() )
		{
			it->second = Value;
			return true;
		}
	}

//	char * const thePropertyName = new char[strlen(Name) + 1];
//	assert (thePropertyName); //lint !e1924 // C-style cast MSVC bug
//	strcpy( thePropertyName, Name );

	mProperties->insert (UIPropertyNameMap::value_type (Name, Value));

	return true;
}

//-----------------------------------------------------------------

bool UIBaseObject::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	DEBUG_DESTROYED();
	
	{
		const size_t pSeparator = Name.get ().find ('.');

		if( pSeparator != std::string::npos)
		{
			UIObjectList olist;
			GetChildren (olist);

			const int len = pSeparator;

			for( UIObjectList::const_iterator o = olist.begin(); o != olist.end(); ++o )
			{
				UIBaseObject * const obj = *o;

				if( obj->IsName( Name.c_str (), len ) )
					return obj->GetProperty( UILowerString (Name.get ().substr (pSeparator + 1)), Value );
			}
		}
	}

	//----------------------------------------------------------------------

	if (Name.startsWith ('/'))
	{
		UIBaseObject *RootObject = GetRoot();

		if( RootObject )
			return RootObject->GetProperty( UILowerString(Name.c_str () + 1), Value );
		else
			return GetProperty( UILowerString (Name.c_str () + 1), Value );
	}

	if( Name.equals ("../", 3 ) )
	{
		if( mParent )
			return mParent->GetProperty( UILowerString(Name.c_str () + 3), Value );
		else
			return false;
	}

	if( Name.equals ( "parent.", 7 ) )
	{
		if( mParent )
			return mParent->GetProperty( UILowerString(Name.c_str () + 7), Value );
		else
			return false;
	}

	if( Name == PropertyName::Name )
	{
		Value = UIUnicode::narrowToWide(mName);
		return true;
	}

	if( mProperties )
	{
		UIPropertyNameMap::const_iterator i = mProperties->find(Name);

		if( i == mProperties->end() )
			return false;

		Value = i->second;
		return true;
	}

	return false;
}

//-----------------------------------------------------------------

bool UIBaseObject::GetPropertyBoolean( const UILowerString & Name, bool &Value ) const
{
	DEBUG_DESTROYED();
	
	UIString StringValue;

	if( GetProperty( Name, StringValue ) )
		return UIUtils::ParseBoolean( StringValue, Value );

	return false;
}

//-----------------------------------------------------------------

bool UIBaseObject::GetPropertyLong( const UILowerString & Name, long &Value ) const
{
	DEBUG_DESTROYED();
	
	UIString StringValue;

	if( GetProperty( Name, StringValue ) )
		return UIUtils::ParseLong( StringValue, Value );

	return false;
}

//-----------------------------------------------------------------

bool UIBaseObject::GetPropertyInteger( const UILowerString & Name, int &Value ) const
{
	DEBUG_DESTROYED();
	
	UIString StringValue;
	return GetProperty( Name, StringValue ) && UIUtils::ParseInteger( StringValue, Value );
}

//-----------------------------------------------------------------

bool UIBaseObject::GetPropertyFloat( const UILowerString & Name, float &Value ) const
{
	DEBUG_DESTROYED();
	
	UIString StringValue;

	if( GetProperty( Name, StringValue ) )
		return UIUtils::ParseFloat( StringValue, Value );

	return false;
}

//-----------------------------------------------------------------

bool UIBaseObject::GetPropertyPoint( const UILowerString & Name, UIPoint &Value ) const
{
	DEBUG_DESTROYED();
	
	UIString StringValue;

	if( GetProperty( Name, StringValue ) )
		return UIUtils::ParsePoint( StringValue, Value );

	return false;
}

//-----------------------------------------------------------------

bool UIBaseObject::GetPropertyRect( const UILowerString & Name, UIRect &Value ) const
{
	DEBUG_DESTROYED();
	
	UIString StringValue;

	if( GetProperty( Name, StringValue ) )
		return UIUtils::ParseRect( StringValue, Value );

	return false;
}

//----------------------------------------------------------------------

bool  UIBaseObject::GetPropertyColorOrPalette  (const UILowerString & Name, UIColor &Value ) const
{
	DEBUG_DESTROYED();
	
	if (GetPropertyColor (Name, Value))
		return true;

	const UIPalette * const pal = UIPalette::GetInstance ();
	if (pal)
		return pal->FindColor (Name, Value);

	return false;
}

//-----------------------------------------------------------------

bool UIBaseObject::GetPropertyColor( const UILowerString & Name, UIColor &Value ) const
{
	DEBUG_DESTROYED();
	
	UIString StringValue;

	if( GetProperty( Name, StringValue ) )
		return UIUtils::ParseColor( StringValue, Value );

	return false;
}

//-----------------------------------------------------------------

bool UIBaseObject::GetPropertyNarrow (const UILowerString & Name,  UINarrowString &Value) const
{
	DEBUG_DESTROYED();
	
	UIString wideValue;

	if (GetProperty (Name, wideValue))
	{
		Value = Unicode::wideToUTF8 (wideValue);
		return true;
	}

	return false;
}

//-----------------------------------------------------------------

void UIBaseObject::CopyPropertiesFrom( const UIBaseObject & rhs )
{
	DEBUG_DESTROYED();
	
	SetName(rhs.GetName());

	UIPropertyNameVector SourceProperties;
	rhs.GetPropertyNames( SourceProperties, true );

	UIString PropertyValue;

	for( UIPropertyNameVector::const_iterator i = SourceProperties.begin(); i != SourceProperties.end(); ++i )
	{
		const UILowerString & thePropertyName = *i;
		PropertyValue.clear();
		rhs.GetProperty( thePropertyName, PropertyValue );
		SetProperty( thePropertyName, PropertyValue );
	}
}

//-----------------------------------------------------------------

bool UIBaseObject::RemoveProperty( const UILowerString & Name )
{
	DEBUG_DESTROYED();
	
	if( !mProperties )
		return false;

	const UIPropertyNameMap::iterator i = mProperties->find( Name );

	if( i != mProperties->end() )
	{
		mProperties->erase(i);
		return true;
	}

	return false;
}

//-----------------------------------------------------------------

void UIBaseObject::PurgeProperty( const UILowerString & Name )
{
	DEBUG_DESTROYED();
	
	RemoveProperty( Name );

	UIObjectList myChildren;
	GetChildren( myChildren );

	for( UIObjectList::iterator i = myChildren.begin(); i != myChildren.end(); ++i )
	{
		(*i)->PurgeProperty( Name );
	}
}

//-----------------------------------------------------------------

bool UIBaseObject::IsPropertyRemovable( const UILowerString & Name ) const
{
	DEBUG_DESTROYED();
	
	if( !mProperties )
		return false;

	return mProperties->find(Name) != mProperties->end();
}

//-----------------------------------------------------------------

bool UIBaseObject::HasProperty( const UILowerString & Name ) const
{
	DEBUG_DESTROYED();
	
	UIString Value;

	return GetProperty( Name, Value );
}

//-----------------------------------------------------------------

void UIBaseObject::SetParent( UIBaseObject *NewParent )
{
	DEBUG_DESTROYED();
	
	if (mParent != NewParent)
	{
		s_uiRootObjectMap.erase(this);

		mParent = NewParent;

#if UI_USE_PROFILER
		if (mFullPath)
			delete [] mFullPath;
		
		const std::string & fullPath = GetFullPath ();
		mFullPath = new char [fullPath.size () + 1];
		strcpy (mFullPath, GetFullPath ().c_str ());
#endif

	}
}

//-----------------------------------------------------------------

UIBaseObject *UIBaseObject::GetRoot( void ) const
{
	DEBUG_DESTROYED();
	
	UIRootObjectMap::const_iterator const itCurrentRoot = s_uiRootObjectMap.find(this);
	UIBaseObject * Root = NULL;

	if (itCurrentRoot != s_uiRootObjectMap.end())
	{
		Root = itCurrentRoot->second;
	}
	else
	{
		Root = const_cast<UIBaseObject *>( this );
		
		while (Root && Root->GetParent())
			Root = Root->GetParent();

		s_uiRootObjectMap[this] = Root;
	}

	return Root;
}


//-----------------------------------------------------------------

UIBaseObject *UIBaseObject::GetChild( const char * ) const
{
	DEBUG_DESTROYED();
	
	// Base object does not know about its children, this is just a hook
	return 0;
}

//-----------------------------------------------------------------

bool UIBaseObject::AddChild( UIBaseObject * )
{
	DEBUG_DESTROYED();
	
	// Base object does not support children, this is just a hook
	return false;
}

//-----------------------------------------------------------------

bool UIBaseObject::RemoveChild( UIBaseObject * )
{
	DEBUG_DESTROYED();
	
	// Base object does not support children, this is just a hook
	return false;
}

//-----------------------------------------------------------------

void UIBaseObject::SelectChild( UIBaseObject * )
{
	DEBUG_DESTROYED();
	
	// Base object does not support children, this is just a hook
}

//-----------------------------------------------------------------

void UIBaseObject::GetChildren( UIObjectList & ) const
{
	DEBUG_DESTROYED();
	
	// Base object does not support children, this is just a hook
}

//-----------------------------------------------------------------

unsigned long UIBaseObject::GetChildCount( void ) const
{
	DEBUG_DESTROYED();
	
	// Base object does not support children, this is just a hook
	return 0;
}

//-----------------------------------------------------------------

UIBaseObject *UIBaseObject::GetObjectFromPath( const char * const ObjectName ) const
{
	DEBUG_DESTROYED();
	
	if (ObjectName == 0 || *ObjectName == 0)
		return 0;

	UIBaseObject const * ObjectToSearch = this;

	if( *ObjectName == '/' )
	{
		UIBaseObject const * const root = GetRoot();
		UIBaseObject * const result = root ? root->GetObjectFromPath( ObjectName + 1 ) : NULL;
		return result;
	}

	while( ObjectToSearch )
	{
		if (!_stricmp (ObjectName, "this") || !_stricmp (ObjectName, ObjectToSearch->mName.c_str ()))
			return const_cast<UIBaseObject *>(ObjectToSearch);

		UIBaseObject *o = ObjectToSearch->GetChild( ObjectName );

		if( o )
			return o;

		ObjectToSearch = ObjectToSearch->GetParent();
	}

	return 0;
}

//-----------------------------------------------------------------

UIBaseObject *UIBaseObject::GetObjectFromPath( const char * const ObjectName, UITypeID Type ) const
{
	DEBUG_DESTROYED();
	
	UIBaseObject *FoundObject = GetObjectFromPath( ObjectName );

	if( FoundObject && FoundObject->IsA( Type ) )
		return FoundObject;
	else
		return 0;
}

//-----------------------------------------------------------------

bool UIBaseObject::CanChildMove( UIBaseObject *, ChildMovementDirection )
{
	DEBUG_DESTROYED();
	
	return false;
}

//-----------------------------------------------------------------

bool UIBaseObject::MoveChild( UIBaseObject *, ChildMovementDirection )
{
	DEBUG_DESTROYED();
	
	return false;
}
//-----------------------------------------------------------------
const UINarrowString  UIBaseObject::GetFullPath () const
{
	DEBUG_DESTROYED();
	
	UINarrowString str = GetName ();

	const UIBaseObject * obj = this;
	while ((obj = obj->GetParent ()) != 0)
	{
		if (obj->GetParent ())
			str = obj->GetName () + "." + str;
	}

	return "/" + str;
}
//-----------------------------------------------------------------

void UIBaseObject::GetPathTo( UINarrowString &Path, const UIBaseObject *OtherObject ) const
{
	DEBUG_DESTROYED();
	
	if( !OtherObject )
	{
		Path.erase();
		return;
	}

	Path = OtherObject->GetName();
	UIBaseObject const * CurrentSearchPath = this;

	while( CurrentSearchPath )
	{
		const UIBaseObject *PathJunction = OtherObject->GetParent();

		while( PathJunction )
		{
			if( PathJunction == CurrentSearchPath )
			{
				const UIBaseObject *PathSection = OtherObject->GetParent();

				while( PathSection != PathJunction )
				{
					Path.insert(Path.begin (), 1, '.');
					Path.insert(0, PathSection->GetName());

					PathSection = PathSection->GetParent();
				}

				if( !PathJunction->GetParent() )
				{
					Path.insert(Path.begin (), 1, '/');
				}

				return;
			}
			PathJunction = PathJunction->GetParent();
		}
		CurrentSearchPath = CurrentSearchPath->GetParent();
	}
}

//-----------------------------------------------------------------

void UIBaseObject::Link ()
{
	DEBUG_DESTROYED();
	
	size_t max_size = 80;
	UIPropertyNameVector pnv;
	pnv.reserve (max_size);

	GetLinkPropertyNames (pnv);

	max_size = std::max (pnv.size (), max_size);

	UIString PropertyValue;

	for( UIPropertyNameVector::const_iterator i = pnv.begin(); i != pnv.end(); ++i )
	{
		const UILowerString & thePropertyName = *i;

		PropertyValue.clear ();

		if( GetProperty (thePropertyName, PropertyValue))
		{
			if (mProperties)
			{
				const UIPropertyNameMap::iterator theProperty = mProperties->find(PropertyName::Name);

				if( theProperty != mProperties->end() )
				{
					mProperties->erase( theProperty );
					SetProperty( thePropertyName, PropertyValue);
				}
				else
					SetProperty( thePropertyName, PropertyValue );
			}
			else
				SetProperty( thePropertyName, PropertyValue );
		}
	}

	if( mProperties && mProperties->empty() )
	{
		delete mProperties;
		mProperties = 0;
	}
}

//-----------------------------------------------------------------

void UIBaseObject::GetOutstandingObjects (UIObjectVector & olist)
{
	if (s_outstandingObjects)
	{
		olist.reserve(s_outstandingObjects->size());
		olist.insert(olist.begin(), s_outstandingObjects->begin(), s_outstandingObjects->end());
	}
}

//-----------------------------------------------------------------

UIBaseObject * UIBaseObject::DuplicateObject () const
{
	DEBUG_DESTROYED();
	
	UIBaseObject *NewObject = this->Clone();

	if(!NewObject)
		return NULL;
	
	NewObject->CopyPropertiesFrom( *this );

	UIBaseObject::UIObjectList Children;
	this->GetChildren( Children );

	for( UIBaseObject::UIObjectList::const_iterator it = Children.begin(); it != Children.end(); ++it )
	{
		UIBaseObject * const dupe = (*it)->DuplicateObject ();
		if (dupe)
			NewObject->AddChild( dupe);
	}

	return NewObject;
}

//----------------------------------------------------------------------

bool UIBaseObject::NotifyActionListener  () const
{
	DEBUG_DESTROYED();
	
	return UIManager::gUIManager ().NotifyActionListener (*this);
}

//----------------------------------------------------------------------

bool UIBaseObject::RemoveProperty (const char * )
{
	DEBUG_DESTROYED();
	
	assert (false);
	return false;
}

//----------------------------------------------------------------------

bool UIBaseObject::SetProperty (const char * , const UIString &)
{
	DEBUG_DESTROYED();
	
	assert (false);
	return false;
}

//----------------------------------------------------------------------

bool UIBaseObject::GetProperty (const char * , UIString & )
{
	DEBUG_DESTROYED();
	
	assert (false);
	return false;
}

//----------------------------------------------------------------------

UIBaseObject * UIBaseObject::GetParent (UITypeID type)
{
	DEBUG_DESTROYED();
	
	if (mParent != NULL && mParent->IsA(type))
	{
		return mParent;
	}
	
	return NULL;
};

//----------------------------------------------------------------------

const UIBaseObject * UIBaseObject::GetParent  (UITypeID type) const
{
	DEBUG_DESTROYED();
	
	if (mParent != NULL && mParent->IsA(type))
	{
		return mParent;
	}

	return NULL;
};

//----------------------------------------------------------------------

void UIBaseObject::RemoveFromParent()
{
	DEBUG_DESTROYED();
	
	if (mParent)
	{
		if (mParent->RemoveChild(this) == false)
			mParent = 0;
	}
}

//----------------------------------------------------------------------

void UIBaseObject::garbageCollect()
{
	while (!s_deletedObjects.empty()) 
	{
		UIObjectHashSet deleteMe(s_deletedObjects);
		s_deletedObjects.clear();

		for (UIObjectHashSet::iterator itObj = deleteMe.begin(); deleteMe.end() != itObj; ++itObj) 
		{
			UIBaseObject * const obj = *itObj;
			delete obj;
		}
	}
}

//-----------------------------------------------------------------

bool UIBaseObject::isAncestor( const UIBaseObject *widg ) const
{
	const UIBaseObject *parent = GetParent();
	while (parent && widg != parent)
		parent = parent->GetParent();

	return (widg == parent);
}


