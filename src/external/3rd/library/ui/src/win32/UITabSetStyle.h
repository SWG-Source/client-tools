#ifndef __UITABSETSTYLE_H__
#define __UITABSETSTYLE_H__

#include "UIWidgetStyle.h"

class UIImageStyle;

class UITabSetStyle : public UIWidgetStyle
{
public:

	enum Image
	{
		ActiveTabStartCap,
		ActiveTabBackground,
		ActiveTabEndCap,

		InactiveTabStartCap,
		InactiveTabBackground,
		InactiveTabEndCap,

		BodyTopLeft,
		BodyTop,
		BodyTopRight,
		BodyRight,
		BodyBottomRight,
		BodyBottom,
		BodyBottomLeft,
		BodyLeft,
		BodyFill,

		LastImage,
	};

	static const char		  *TypeName;

	class PropertyName
	{
		public:

			static const UILowerString ActiveTabStartCap;
			static const UILowerString ActiveTabBackground;
			static const UILowerString ActiveTabEndCap;

			static const UILowerString InactiveTabStartCap;
			static const UILowerString InactiveTabBackground;
			static const UILowerString InactiveTabEndCap;

			static const UILowerString BodyTopLeft;
			static const UILowerString BodyTop;
			static const UILowerString BodyTopRight;
			static const UILowerString BodyRight;
			static const UILowerString BodyBottomRight;
			static const UILowerString BodyBottom;
			static const UILowerString BodyBottomLeft;
			static const UILowerString BodyLeft;
			static const UILowerString BodyFill;

			static const UILowerString Layout;
	};

											   UITabSetStyle();
	virtual							  ~UITabSetStyle();

	virtual bool				   IsA( const UITypeID ) const;
	virtual const char	  *GetTypeName( void ) const;
	virtual UIBaseObject	*Clone( void ) const;

	virtual void                GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void					 GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual void                GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool					 SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool					 GetProperty( const UILowerString & Name, UIString &Value ) const;

					void					 SetImage( const Image, UIImageStyle * );
					UIImageStyle  *GetImage( const Image ) const;

private:

					bool					 LookupImageByName( const UILowerString & Name, Image &theImage ) const;

	UIImageStyle				  *mImages[LastImage];
};

#endif // __UITABSETSTYLE_H__