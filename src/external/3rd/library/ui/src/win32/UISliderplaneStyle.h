#ifndef __UISliderplaneSTYLE_H__
#define __UISliderplaneSTYLE_H__

#include "UIWidgetStyle.h"

class UIImageStyle;

class UISliderplaneStyle : public UIWidgetStyle
{
public:

	enum Image
	{
		StartCap,
		Background,
		EndCap,
		Slider,
		LastImage,
	};

	static const char      *TypeName;

	class PropertyName
	{
	public:

		static const UILowerString StepSize;
		static const UILowerString StartCap;
		static const UILowerString Background;
		static const UILowerString EndCap;
		static const UILowerString Slider;
		static const UILowerString Layout;
	};

	                        UISliderplaneStyle();
	virtual                ~UISliderplaneStyle();

	virtual bool             IsA( const UITypeID ) const;
	virtual const char      *GetTypeName( void ) const;
	virtual UIBaseObject    *Clone( void ) const;

	virtual void             GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void             GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual void             GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool             SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool             GetProperty( const UILowerString & Name, UIString &Value ) const;

	void                     SetImage( const Image, UIImageStyle * );
	UIImageStyle            *GetImage( const Image ) const;

	Layout                   GetLayout( void ) const;
	void                     SetLayout( Layout );

private:

	Layout                   mLayout;
	Image                    LookupImageIDByName( const UILowerString & Name ) const;
	UIImageStyle            *mImages[LastImage];
};

#endif // __UISliderplaneSTYLE_H__