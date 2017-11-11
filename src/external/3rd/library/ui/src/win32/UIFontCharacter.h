#ifndef __UIFONTCHARACTER_H__
#define __UIFONTCHARACTER_H__

#include "UIBaseObject.h"
#include "UITypes.h"

class UICanvas;

class UIFontCharacter : public UIBaseObject
{
public:

	static const char * const TypeName; //lint !e1516 // data member hides inherited member

	class PropertyName
	{//lint !e578 // symbol hides symbol
	public:
		static const UILowerString Code;
		static const UILowerString AdvancePre;
		static const UILowerString Advance;
		static const UILowerString SourceRect;
		static const UILowerString SourceFile;
	};

	UIFontCharacter();
	virtual                ~UIFontCharacter();

	virtual bool           IsA( const UITypeID ) const;
	virtual const char    *GetTypeName () const;
	virtual UIBaseObject  *Clone () const;

	virtual void           GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void           GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual void           GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool           SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool           GetProperty( const UILowerString & Name, UIString &Value ) const;

	void           SetCanvas( UICanvas * );
	UICanvas      *GetCanvas () const { return mCanvas; };

	void           SetLocation( const UIPoint & );
	const UIPoint &GetLocation () const { return mLocation; };

	void           SetSize( const UISize & );
	long           GetWidth () const { return mSize.x; };

	void           SetAdvance (const long);
	long           GetAdvance () const;
	long           GetAdvancePre () const;

	void           SetCharacterCode( Unicode::unicode_char_t );
	long           GetCharacterCode () const { return mCode; };

private:

	UICanvas                            *mCanvas;
	UIPoint                              mLocation;
	UISize                               mSize;
	long                                 mAdvance;
	long                                 mAdvancePre;
	Unicode::unicode_char_t              mCode;
};

//----------------------------------------------------------------------

inline long UIFontCharacter::GetAdvance () const
{
	return mAdvance;
}

//----------------------------------------------------------------------

inline long UIFontCharacter::GetAdvancePre () const
{
	return mAdvancePre;
}

//----------------------------------------------------------------------

inline const char *UIFontCharacter::GetTypeName( void ) const
{
	return TypeName;
}

//----------------------------------------------------------------------

#endif // __UIFONTCHARACTER_H__
