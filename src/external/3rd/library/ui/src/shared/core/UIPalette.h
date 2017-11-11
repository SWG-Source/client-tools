//======================================================================
//
// UIPalette.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UIPalette_H
#define INCLUDED_UIPalette_H

//======================================================================

#include "UIBaseObject.h"
#include "UINotification.h"

class  UICanvas;
struct UIColor;

//lint -esym(1516,UIPalette::TypeName) // hides
//lint -esym(578,UIPalette::PropertyName) // hides

//======================================================================================

class UIPalette :
public UIBaseObject,
public UINotificationServer
{
public:

	struct PropertyName
	{
		static const UILowerString Reset;

		static const UILowerString AccentDark;
		static const UILowerString AccentLight;
		static const UILowerString BaseDark;
		static const UILowerString BaseLight;
		static const UILowerString Highlight;
		static const UILowerString TextDark;
		static const UILowerString TextLight;
		static const UILowerString IsSpaceEnabled;
	};

	static const char * const   TypeName;

	                            UIPalette               ();
	virtual                    ~UIPalette               ();

	virtual bool                IsA                     (const UITypeID) const;
	virtual const char         *GetTypeName             () const;
	virtual UIBaseObject       *Clone                   () const;

	virtual void                GetPropertyNames        (UIPropertyNameVector &, bool forCopy ) const;
	virtual bool                SetProperty             (const UILowerString & Name, const UIString &Value );
	virtual bool                GetProperty             (const UILowerString & Name, UIString &Value ) const;

	bool                        FindColor               (UILowerString name, UIColor & color) const;

	static UIPalette *          GetInstance             ();

	void                        Reset                   ();

	void                        ApplyPalette            (UIBaseObject & obj) const;

	static void                 RegisterPaletteEntry    (UITypeID type, const UILowerString & name, const UILowerString & prop);
	static void                 GetPropertyNamesForType (UITypeID id, UIPropertyNameVector & );

	static bool                 SetPropertyForObject    (UIBaseObject & obj, const UILowerString & name, const Unicode::String & Value);

	static void                 CopyPaletteEntriesForObject(UITypeID typeId, UIBaseObject & lhs, UIBaseObject const & rhs);

	bool IsSpaceEnabled() const;
	bool SetIsSpaceEnabled(bool const yesno);


protected:

	UIPalette & operator= (const UIPalette &);
	          UIPalette   (const UIPalette &);
	void                        CacheColors           () const;

	typedef ui_stdmap<UILowerString, UIColor>::fwd ColorCache;
	mutable ColorCache *               m_colorCache;

	mutable bool                       m_dirty;

	bool m_isSpaceEnabled;
};

//======================================================================================

inline bool UIPalette::IsSpaceEnabled() const
{
	return m_isSpaceEnabled;
}

//======================================================================================

inline bool UIPalette::SetIsSpaceEnabled(bool const yesno)
{
	m_isSpaceEnabled = yesno;
}


//======================================================================

#endif

