#ifndef __UITOOLTIPSTYLE_H__
#define __UITOOLTIPSTYLE_H__

#include "UITemplateCache.h"
#include "UIWidgetStyle.h"

class UIImageStyle;
class UIGridStyle;
class UIDataSource;
class UITemplate;
class UITextStyle;

class UITooltipStyle : public UIWidgetStyle
{
public:

	static const char	    * const TypeName; //lint !e1516 // data member hides inherited member

	class PropertyName
	{	 //lint !e578 // symbol hides symbol
		public:

			static const UILowerString DataSource;
			static const UILowerString GridStyle;
			static const UILowerString MaxWidth;
			static const UILowerString Template;
			static const UILowerString TextColor;
			static const UILowerString TextPadding;
			static const UILowerString TextStyle;
			static const UILowerString BackgroundOpacity;
	};

												 UITooltipStyle ();
	virtual								~UITooltipStyle ();

	virtual bool					 IsA( const UITypeID ) const;
	virtual const char		*GetTypeName () const;
	virtual UIBaseObject  *Clone () const;

	virtual void                GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void					 GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual void                GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool					 SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool					 GetProperty( const UILowerString & Name, UIString &Value ) const;

					void					 SetGridStyle( UIGridStyle * );
					void					 SetDataSource( UIDataSource * );
					void					 SetTemplate( UITemplate * );
					void           SetTextStyle( UITextStyle * );

					void					 SetItemInDataSource( long In );
					long					 GetItemInDataSource () const { return mItemInDataSource; };

					// Render using internal template & data source
					void					 Render( UICanvas &, UITime ) const;

					// Render using caller's string
					void					 Render( UICanvas &, const UIString &, UITime ) const;

					// Measure using internal template & data source
	const UIPoint                            Measure () const;

					// Measure using caller's string
					UIPoint				 Measure( const UIString &) const;

					void SetCharacterWidth(long characters);

private:

	UIGridStyle						*mGridStyle;

	UIDataSource				  *mDataSource;
	long									 mItemInDataSource;

	UITemplate						*mTemplate;
	UITemplateCache				 mCache;

	long									 mMaxWidth;
	UIRect								 mTextPadding;
	UIColor								 mTextColor;
	UITextStyle						*mTextStyle;
	float                            mBackgroundOpacity;


};

#endif // __UITOOLTIPSTYLE_H__
