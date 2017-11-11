#ifndef __UISLIDERBARSTYLE_H__
#define __UISLIDERBARSTYLE_H__

#include "UIWidgetStyle.h"

class UIImageStyle;
class UIRectangleStyle;
class UIButtonStyle;

//----------------------------------------------------------------------

class UISliderbarStyle :
public UIWidgetStyle
{
public:

	static const char      * const TypeName;

	class PropertyName
	{
	public:

		static const UILowerString  Layout;

		static const UILowerString  ButtonStyleStart;
		static const UILowerString  ButtonStyleEnd;
		static const UILowerString  ButtonStyleThumb;
		static const UILowerString  ButtonStyleTrack;
		static const UILowerString  ButtonStyleTrackDisabled;
		static const UILowerString  ButtonStyleOverlay;

		static const UILowerString  ButtonLengthStart;
		static const UILowerString  ButtonLengthEnd;
		static const UILowerString  ButtonLengthThumb;

		static const UILowerString  ButtonMarginStart;
		static const UILowerString  ButtonMarginEnd;
		static const UILowerString  ButtonMarginThumb;
		static const UILowerString  ButtonMarginTrack;
		static const UILowerString  ButtonMarginTrackDisabled;
		static const UILowerString  ButtonMarginOverlay;

	};

	                           UISliderbarStyle ();
	virtual                   ~UISliderbarStyle ();

	virtual bool               IsA              (const UITypeID) const;
	virtual const char *       GetTypeName      () const;
	virtual UIBaseObject *     Clone            () const;

	virtual void               GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void               GetPropertyNames (UIPropertyNameVector &, bool forCopy) const;
	virtual void               GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool               SetProperty      (const UILowerString & Name, const UIString &Value);
	virtual bool               GetProperty      (const UILowerString & Name, UIString &Value) const;

	Layout                     GetLayout        () const;
	void                       SetLayout        (Layout);


	enum ButtonType
	{
		BT_start,
		BT_end,
		BT_thumb,
		BT_track,
		BT_trackDisabled,
		BT_overlay,
		BT_numButtonTypes
	};

	enum PropertyType
	{
		PT_style,
		PT_length,
		PT_margin,
		PT_numPropertyTypes,
	};

	static bool                FindPropertyInfo (const char * const str, PropertyType & ptype, ButtonType & btype);

	UIButtonStyle *            GetButtonStyle   (const ButtonType);
	const UIRect &             GetButtonMargin  (const ButtonType type) const;
	long                       GetButtonLength  (const ButtonType type);

	void                       SetButtonStyle   (const ButtonType, UIButtonStyle * style);
	void                       SetButtonMargin  (const ButtonType type, const UIRect & rect);
	void                       SetButtonLength  (const ButtonType type, const long length);

private:

	Layout                 mLayout;

	UIButtonStyle *        mButtonStyles  [BT_numButtonTypes];
	UIRect                 mButtonMargins [BT_numButtonTypes];
	long                   mButtonLengths [BT_numButtonTypes];
};

//----------------------------------------------------------------------

inline UIButtonStyle * UISliderbarStyle::GetButtonStyle   (const ButtonType type)
{
	return mButtonStyles [type];
}

//----------------------------------------------------------------------

inline const UIRect & UISliderbarStyle::GetButtonMargin  (const ButtonType type) const
{
	return mButtonMargins [type];
}

//----------------------------------------------------------------------

inline long UISliderbarStyle::GetButtonLength  (const ButtonType type)
{
	return mButtonLengths [type];
}

//----------------------------------------------------------------------

#endif // __UISLIDERBARSTYLE_H__