#ifndef __UIGRIDSTYLE_H__
#define __UIGRIDSTYLE_H__

#include "UIWidgetStyle.h"

class UIImageStyle;
class UICanvas;

class UIGridStyle : public UIWidgetStyle
{
public:

	enum Image
	{
		NormalBackground = 0,
		NormalJunction,
		NormalVStartCap,
		NormalVBackground,
		NormalVEndCap,
		NormalHStartCap,
		NormalHBackground,
		NormalHEndCap,

		SelectedBackground,
		SelectedJunction,
		SelectedVStartCap,
		SelectedVBackground,
		SelectedVEndCap,
		SelectedHStartCap,
		SelectedHBackground,
		SelectedHEndCap,

		LastImage,

		NormalBase	 = NormalBackground,
		SelectedBase = SelectedBackground,

		BackgroundOffset  = NormalBackground  - NormalBackground,
		JunctionOffset    = NormalJunction    - NormalBackground,
		VStartCapOffset   = NormalVStartCap   - NormalBackground,
		VBackgroundOffset	= NormalVBackground - NormalBackground,
		VEndCapOffset			= NormalVEndCap     - NormalBackground,
		HStartCapOffset   = NormalHStartCap   - NormalBackground,
		HBackgroundOffset	= NormalHBackground - NormalBackground,
		HEndCapOffset			= NormalHEndCap     - NormalBackground,
	};

	static const char		 *TypeName;

	class PropertyName
	{
	public:

		static const UILowerString NormalBackground;
		static const UILowerString NormalJunction;
		static const UILowerString NormalVStartCap;
		static const UILowerString NormalVBackground;
		static const UILowerString NormalVEndCap;
		static const UILowerString NormalHStartCap;
		static const UILowerString NormalHBackground;
		static const UILowerString NormalHEndCap;

		static const UILowerString SelectedBackground;
		static const UILowerString SelectedJunction;
		static const UILowerString SelectedVStartCap;
		static const UILowerString SelectedVBackground;
		static const UILowerString SelectedVEndCap;
		static const UILowerString SelectedHStartCap;
		static const UILowerString SelectedHBackground;
		static const UILowerString SelectedHEndCap;
	};
												 UIGridStyle();
	virtual							  ~UIGridStyle();

	virtual bool					 IsA( const UITypeID ) const;
	virtual const char	  *GetTypeName( void ) const;
	virtual UIBaseObject	*Clone( void ) const;

	virtual void                GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void					 GetPropertyNames( UIPropertyNameVector &, bool forCopy  ) const;
	virtual void                GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool					 SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool					 GetProperty( const UILowerString & Name, UIString &Value ) const;

					void					 SetImage( const Image, UIImageStyle * );
					UIImageStyle  *GetImage( const Image ) const;

					void					 Render( UITime, UICanvas &, const UISize &CellSize, const UISize &CellCount, const bool *CellSelectionState ) const;

private:

	UIGridStyle::Image		 LookupImageIDByName( const UILowerString & Name ) const;
	UIImageStyle				  *mImages[LastImage];
};

#endif // __UIGRIDSTYLE_H__