#ifndef __UIDROPDOWNBOXSTYLE_H__
#define __UIDROPDOWNBOXSTYLE_H__

#include "UIWidgetStyle.h"

class UIImageStyle;
class UIListboxStyle;

class UIDropdownboxStyle : public UIWidgetStyle
{
public:

	enum Image
	{
		NormalStartCap = 0,
		NormalBackground,
		NormalDownArrow,

		DisabledStartCap,
		DisabledBackground,
		DisabledDownArrow,

		SelectedStartCap,
		SelectedBackground,
		SelectedDownArrow,

		PressedStartCap,
		PressedBackground,
		PressedDownArrow,

		LastImage,

		NormalBase						= NormalStartCap,
		DisabledBase					= DisabledStartCap,
		SelectedBase					= SelectedStartCap,
		PressedBase						= PressedStartCap,

		StartCapOffset        = NormalStartCap   - NormalStartCap,
		BackgroundOffset      = NormalBackground - NormalStartCap,
		DownArrowOffset       = NormalDownArrow  - NormalStartCap,
	};

	static const char				 *TypeName;

	class PropertyName
	{
		public:

			static const UILowerString NormalStartCap;
			static const UILowerString NormalBackground;
			static const UILowerString NormalDownArrow;

			static const UILowerString DisabledStartCap;
			static const UILowerString DisabledBackground;
			static const UILowerString DisabledDownArrow;

			static const UILowerString SelectedStartCap;
			static const UILowerString SelectedBackground;
			static const UILowerString SelectedDownArrow;

			static const UILowerString PressedStartCap;
			static const UILowerString PressedBackground;
			static const UILowerString PressedDownArrow;

			static const UILowerString DropBy;
			static const UILowerString Padding;

//			static const UILowerString ScrollbarStyle;
			static const UILowerString ListboxStyle;

			static const UILowerString OpenSound;
			static const UILowerString CloseSound;
	};

														UIDropdownboxStyle();
	virtual									 ~UIDropdownboxStyle();

	virtual bool							IsA( const UITypeID ) const;
	virtual const char			 *GetTypeName( void ) const;
	virtual UIBaseObject		 *Clone( void ) const;

	virtual void                     GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void							GetPropertyNames( UIPropertyNameVector &, bool forCopy  ) const;
	virtual void                     GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool							SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool							GetProperty( const UILowerString & Name, UIString &Value ) const;

					void							SetImage( const Image, UIImageStyle * );
					UIImageStyle		 *GetImage( const Image ) const;

//					void							SetScrollbarStyle( UIScrollbarStyle * );
//					UIScrollbarStyle *GetScrollbarStyle( void ) const;

					void							SetListboxStyle( UIListboxStyle * );
					UIListboxStyle	 *GetListboxStyle( void ) const;

					void							SetDropBy( const long );
					long							GetDropBy( void ) const;

					void							SetPadding( const UIRect & );
					void							GetPadding( UIRect & ) const;

private:

	bool											LookupPropertyImage( const UILowerString & Name, Image &ImageID ) const;

	long											mDropBy;
	UIRect										mPadding;
	UIImageStyle						 *mImages[LastImage];
//	UIScrollbarStyle				 *mScrollbarStyle;
	UIListboxStyle					 *mListboxStyle;
};

#endif // __UIDROPDOWNBOXSTYLE_H__