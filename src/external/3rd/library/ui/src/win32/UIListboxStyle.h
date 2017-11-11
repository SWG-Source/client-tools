#ifndef __UILISTBOXSTYLE_H__
#define __UILISTBOXSTYLE_H__

#include "UIWidgetStyle.h"

class UIGridStyle;

class UIListboxStyle : public UIWidgetStyle
{
public:

	static const char		  *TypeName;

	class PropertyName
	{
		public:
			static const UILowerString GridStyle;
			static const UILowerString Layout;
	};

											   UIListboxStyle();
	virtual							  ~UIListboxStyle();

	virtual bool				   IsA( const UITypeID ) const;
	virtual const char	  *GetTypeName( void ) const;
	virtual UIBaseObject	*Clone( void ) const;

	virtual void                GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void					 GetPropertyNames( UIPropertyNameVector &, bool forCopy  ) const;
	virtual void                GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool					 SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool					 GetProperty( const UILowerString & Name, UIString &Value ) const;

					void					 SetGridStyle( UIGridStyle * );
					UIGridStyle   *GetGridStyle( void ) const;

					Layout         GetLayout( void ) const;
					void           SetLayout( Layout );

private:

	Layout								 mLayout;
	UIGridStyle					  *mGridStyle;
};

#endif // __UILISTBOXSTYLE_H__