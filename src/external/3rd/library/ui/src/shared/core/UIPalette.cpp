#include "_precompile.h"

#include "UICanvas.h"
#include "UILowerString.h"
#include "UIManager.h"
#include "UIPage.h"
#include "UIPalette.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"
#include <cassert>
#include <map>
#include <list>
#include <vector>

const char * const UIPalette::TypeName = "Palette";

//----------------------------------------------------------------------

const UILowerString UIPalette::PropertyName::Reset         = UILowerString ("Reset");

const UILowerString UIPalette::PropertyName::AccentDark    = UILowerString ("AccentDark");
const UILowerString UIPalette::PropertyName::AccentLight   = UILowerString ("AccentLight");
const UILowerString UIPalette::PropertyName::BaseDark      = UILowerString ("BaseDark");
const UILowerString UIPalette::PropertyName::BaseLight     = UILowerString ("BaseLight");
const UILowerString UIPalette::PropertyName::Highlight     = UILowerString ("Highlight");
const UILowerString UIPalette::PropertyName::TextDark      = UILowerString ("TextDark");
const UILowerString UIPalette::PropertyName::TextLight     = UILowerString ("TextLight");
const UILowerString UIPalette::PropertyName::IsSpaceEnabled= UILowerString ("SpaceEnabled");

//----------------------------------------------------------------------

namespace
{

	typedef ui_stdmap<UILowerString, UILowerString>::fwd PalettePropertyMap;
	typedef ui_stdmap<UITypeID, PalettePropertyMap>::fwd PalettePropertyTypeMap;

	PalettePropertyTypeMap s_palettePropertyTypes;
}

//----------------------------------------------------------------------

UIPalette::UIPalette () :
UIBaseObject         (),
UINotificationServer (),
m_colorCache         (new ColorCache),
m_dirty              (true),
m_isSpaceEnabled(false)
{
}

//----------------------------------------------------------------------

UIPalette & UIPalette::operator= (const UIPalette & rhs)
{
	UIBaseObject::operator= (rhs);
	*m_colorCache = *(rhs.m_colorCache);
	m_dirty       = rhs.m_dirty;

	return *this;
}

//----------------------------------------------------------------------

UIPalette::UIPalette   (const UIPalette & rhs) : 
UIBaseObject         (rhs),
UINotificationServer (),
m_colorCache         (new ColorCache (*(rhs.m_colorCache))),
m_dirty              (rhs.m_dirty),
m_isSpaceEnabled(false)
{
}

//----------------------------------------------------------------------

UIPalette::~UIPalette ()
{
	delete m_colorCache;
	m_colorCache = 0;
}

//----------------------------------------------------------------------

const char *UIPalette::GetTypeName () const
{
	return TypeName;
}

//----------------------------------------------------------------------

bool UIPalette::IsA (const UITypeID Type) const
{
	return (Type == TUIPalette) || UIBaseObject::IsA( Type );
}

//----------------------------------------------------------------------

void UIPalette::GetPropertyNames      (UIPropertyNameVector & in, bool forCopy) const
{
	in.push_back (PropertyName::Reset      );
	in.push_back (PropertyName::AccentDark );
	in.push_back (PropertyName::AccentLight);
	in.push_back (PropertyName::BaseDark   );
	in.push_back (PropertyName::BaseLight  );
	in.push_back (PropertyName::Highlight  );
	in.push_back (PropertyName::TextDark   );
	in.push_back (PropertyName::TextLight  );
	in.push_back (PropertyName::IsSpaceEnabled);

	UIBaseObject::GetPropertyNames (in, forCopy);
}

//----------------------------------------------------------------------

bool UIPalette::SetProperty           (const UILowerString & Name, const UIString &Value )
{
	if (Name == PropertyName::Reset)
	{
		Reset ();
		return true;
	}
	else if (Name == PropertyName::IsSpaceEnabled)
	{
		return UIUtils::ParseBoolean(Value, m_isSpaceEnabled);
	}

	m_dirty = true;
	return  UIBaseObject::SetProperty (Name, Value);
}

//----------------------------------------------------------------------

UIBaseObject  * UIPalette::Clone () const
{
	return new UIPalette;
}

//----------------------------------------------------------------------

bool UIPalette::GetProperty           (const UILowerString & Name, UIString &Value ) const
{
	if (Name == PropertyName::IsSpaceEnabled)
	{
		return UIUtils::FormatBoolean(Value, m_isSpaceEnabled);
	}

	return UIBaseObject::GetProperty (Name, Value);
}

//----------------------------------------------------------------------

bool UIPalette::FindColor (UILowerString name, UIColor & color) const
{
	if (m_dirty)
		CacheColors ();

	const ColorCache::const_iterator it = m_colorCache->find (name);
	if (it != m_colorCache->end ())
	{
		color = (*it).second;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

void UIPalette::CacheColors () const
{
	m_dirty = false;

	m_colorCache->clear ();
	const UIPropertyNameMap  * const props = const_cast<UIPalette *>(this)->GetPropertyMap ();

	if (!props)
		return;

	for (UIPropertyNameMap::const_iterator it = props->begin (); it != props->end (); ++it)
	{
		const UILowerString &   name  = (*it).first;
		const Unicode::String & value = (*it).second;

		UIColor color;

		bool found = false;

		if (UIUtils::ParseColor (value, color))
			found = true;
		else
		{
			UILowerString lowerValue (Unicode::wideToNarrow (value));

			if (FindColor (lowerValue, color))
				found = true;
			else
			{
				const UIPropertyNameMap::const_iterator fit	= props->find (lowerValue);

				if (fit != props->end ())
				{
					const Unicode::String & referentColorName = (*it).second;
					found = UIUtils::ParseColor (referentColorName, color);
				}
			}
		}

		if (found)
		{
			m_colorCache->insert (std::make_pair (name, color));
		}
		else if ((name.c_str() != 0) && (strstr(name.c_str(), "notcolor_") == 0))
		{
			UI_REPORT_LOG_PRINT (true, ("Palette [%s] Unable to find or parse referent color [%s] for [%s]\n", GetName ().c_str (), Unicode::wideToNarrow (value).c_str (), name.c_str ()));
		}
	}
}

//----------------------------------------------------------------------

UIPalette * UIPalette::GetInstance           ()
{
	if (!UIManager::isUIReady()) 
	{
		return 0;
	}
	
	const UIPage * const root = UIManager::gUIManager ().GetRootPage ();
	if (!root)
		return 0;

	std::string palettePath;
	const static UILowerString paletteProp ("palette");
	if (root->GetPropertyNarrow (paletteProp, palettePath))
		return static_cast<UIPalette *>(root->GetObjectFromPath (palettePath.c_str (), TUIPalette));

	return 0;
}

//----------------------------------------------------------------------

void UIPalette::Reset                 ()
{
	if (m_dirty)
		CacheColors ();

	if (this != GetInstance ())
		return;

//	SendNotification (UINotification::ObjectChanged, this);

	UIPage * const root = UIManager::gUIManager ().GetRootPage ();
	assert (root);

	typedef ui_stdvector<UIBaseObject *>::fwd ObjectVector;

	static ObjectVector objectStack;

	objectStack.clear ();
	objectStack.push_back (root);

	UIBaseObject::UIObjectList olist;

	while (!objectStack.empty ())
	{
		UIBaseObject * const obj = objectStack.back ();
		objectStack.pop_back ();

		olist.clear ();

		obj->GetChildren (olist);

		objectStack.reserve (objectStack.size () + olist.size ());
		objectStack.insert (objectStack.end (), olist.begin (), olist.end ());
		
		ApplyPalette (*obj);
	}
}

//----------------------------------------------------------------------

void UIPalette::RegisterPaletteEntry  (UITypeID type, const UILowerString & name, const UILowerString & targetProp)
{
	PalettePropertyMap & palProps = s_palettePropertyTypes [type];
	palProps.insert (std::make_pair (name, targetProp));
}

//----------------------------------------------------------------------

void UIPalette::ApplyPalette (UIBaseObject & obj) const
{
	UIColor	    color;
	std::string propPaletteEntry;

	for (PalettePropertyTypeMap::const_iterator it = s_palettePropertyTypes.begin (); it != s_palettePropertyTypes.end (); ++it)
	{
		const UITypeID id = (*it).first;

		if (obj.IsA (id))
		{
			const PalettePropertyMap & palProps = (*it).second;

			for (PalettePropertyMap::const_iterator pit = palProps.begin (); pit != palProps.end (); ++pit)
			{
				const UILowerString & name = (*pit).first;

				if (obj.GetPropertyNarrow (name, propPaletteEntry))
				{
					const UILowerString lowerPropPaletteEntry (propPaletteEntry);
					if (FindColor (lowerPropPaletteEntry, color))
					{
						const UILowerString & targetProp = (*pit).second;
						obj.SetPropertyColor (targetProp, color);
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void UIPalette::CopyPaletteEntriesForObject(UITypeID typeId, UIBaseObject & lhs, UIBaseObject const & rhs)
{
	assert(lhs.IsA(typeId) && rhs.IsA(typeId));

	PalettePropertyTypeMap::const_iterator const it = s_palettePropertyTypes.find (typeId);
	
	Unicode::String value;

	if (it != s_palettePropertyTypes.end ())
	{
		const PalettePropertyMap & palProps = (*it).second;

		for (PalettePropertyMap::const_iterator pit = palProps.begin (); pit != palProps.end (); ++pit)
		{
			const UILowerString & name = (*pit).first;
			value.clear();
			rhs.GetProperty(name, value);
			lhs.SetProperty(name, value);
		}
	}
}

//----------------------------------------------------------------------

void UIPalette::GetPropertyNamesForType (UITypeID id, UIPropertyNameVector & in)
{
	PalettePropertyTypeMap::const_iterator it = s_palettePropertyTypes.find (id);
	
	if (it != s_palettePropertyTypes.end ())
	{
		const PalettePropertyMap & palProps = (*it).second;

		for (PalettePropertyMap::const_iterator pit = palProps.begin (); pit != palProps.end (); ++pit)
		{
			const UILowerString & name = (*pit).first;
			in.push_back (name);
		}
	}
}

//----------------------------------------------------------------------

bool UIPalette::SetPropertyForObject  (UIBaseObject & obj, const UILowerString & name, const Unicode::String & Value)
{
	if (Value.empty ())
		return false;

	UIPalette * const palette = GetInstance ();

	if (!palette)
		return false;

	UIColor	    color;
	const UILowerString lowerPropPaletteEntry (Unicode::wideToNarrow (Value));
	
	for (PalettePropertyTypeMap::const_iterator it = s_palettePropertyTypes.begin (); it != s_palettePropertyTypes.end (); ++it)
	{
		const UITypeID id = (*it).first;
		
		if (obj.IsA (id))
		{
			const PalettePropertyMap & palProps = (*it).second;
			
			PalettePropertyMap::const_iterator pit = palProps.find (name);
			if (pit != palProps.end ())
			{
				if (palette->FindColor (lowerPropPaletteEntry, color))
				{
					const UILowerString & targetProp = (*pit).second;
					obj.SetPropertyColor (targetProp, color);
					return true;
				}
				else
					UI_REPORT_LOG_PRINT (true, ("Palette [%s] Unable to find palette color [%s] for [%s].[%s]\n", palette->GetName ().c_str (), lowerPropPaletteEntry.c_str (), obj.GetFullPath ().c_str (), name.c_str ()));
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------

