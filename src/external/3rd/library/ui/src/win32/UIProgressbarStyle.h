#ifndef __UIPROGRESSBARSTYLE_H__
#define __UIPROGRESSBARSTYLE_H__

#include "UIWidgetStyle.h"

class UIImageStyle;

class UIProgressbarStyle : public UIWidgetStyle
{
public:

	enum Image
	{
		StartCap,
		Background,
		EndCap,

		BarStartCap,
		BarBackground,
		BarEndCap,

		LastImage,
	};

	static const char		  *TypeName;

	class PropertyName
	{
	public:

		static const UILowerString Background;
		static const UILowerString BarBackground;
		static const UILowerString BarEndCap;
		static const UILowerString BarStartCap;
		static const UILowerString EndCap;
		static const UILowerString Inverted;
		static const UILowerString Layout;
		static const UILowerString StartCap;
		static const UILowerString StepSize;
	};

											   UIProgressbarStyle();
	virtual							  ~UIProgressbarStyle();

	virtual bool				   IsA( const UITypeID ) const;
	virtual const char	  *GetTypeName( void ) const;
	virtual UIBaseObject	*Clone( void ) const;

	virtual void                GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void					 GetPropertyNames( UIPropertyNameVector &, bool forCopy  ) const;
	virtual void                GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool					 SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool					 GetProperty( const UILowerString & Name, UIString &Value ) const;

					void					 SetImage( const Image, UIImageStyle * );
					UIImageStyle  *GetImage( const Image ) const;

					void					 SetStepSize( const long );
					long					 GetStepSize( void ) const;

					Layout				 GetLayout( void ) const { return mLayout; };
					void 					 SetLayout( const Layout NewLayout ){ mLayout = NewLayout; };

					bool           IsInverted( void ) const { return mInverted; };

private:

	Image									 LookupImageIDByName( const UILowerString & Name ) const;

	Layout                 mLayout;
	long								   mStepSize;
	UIImageStyle				  *mImages[LastImage];
	bool									 mInverted;
};

#endif // __UIPROGRESSBARSTYLE_H__