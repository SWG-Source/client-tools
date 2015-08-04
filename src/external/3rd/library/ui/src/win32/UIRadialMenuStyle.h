#ifndef __UIRadialMenuStyle_H__
#define __UIRadialMenuStyle_H__

#include "UIWidgetStyle.h"

class UIImageStyle;
class UIButtonStyle;
class UIPopupMenuStyle;

//----------------------------------------------------------------------

class UIRadialMenuStyle : public UIWidgetStyle
{
public:

	static const char     * const TypeName;

	enum ButtonStyleType
	{
		BST_NW,
		BST_N,
		BST_NE,
		BST_E,
		BST_SE,
		BST_S,
		BST_SW,
		BST_W,
		BST_NumButtonStyles
	};

	struct PropertyName
	{
		static const UILowerString  ButtonStyle_NW;
		static const UILowerString  ButtonStyle_N;
		static const UILowerString  ButtonStyle_NE;
		static const UILowerString  ButtonStyle_E;
		static const UILowerString  ButtonStyle_SE;
		static const UILowerString  ButtonStyle_S;
		static const UILowerString  ButtonStyle_SW;
		static const UILowerString  ButtonStyle_W;

		static const UILowerString  ItemHeight;
		static const UILowerString  ItemMargin;

		static const UILowerString  ItemRadius;

		static const UILowerString  ItemPopupTimeout;

		static const UILowerString  PopupStyle_NW;
		static const UILowerString  PopupStyle_N;
		static const UILowerString  PopupStyle_NE;
		static const UILowerString  PopupStyle_E;
		static const UILowerString  PopupStyle_SE;
		static const UILowerString  PopupStyle_S;
		static const UILowerString  PopupStyle_SW;
		static const UILowerString  PopupStyle_W;
	};

	                        UIRadialMenuStyle     ();
	virtual                ~UIRadialMenuStyle     ();

	virtual bool            IsA                   (const UITypeID) const;
	virtual const char     *GetTypeName           () const;
	virtual UIBaseObject   *Clone                 () const;

	virtual void            GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void            GetPropertyNames      (UIPropertyNameVector &, bool forCopy  ) const;
	virtual void            GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool            SetProperty           (const UILowerString & Name, const UIString &Value );
	virtual bool            GetProperty           (const UILowerString & Name, UIString &Value ) const;

	UIButtonStyle *         GetButtonStyle        (ButtonStyleType type) const;
	void                    SetButtonStyle        (ButtonStyleType type, UIButtonStyle *);

	UIPopupMenuStyle *      GetPopupStyle         (ButtonStyleType type) const;
	void                    SetPopupStyle         (ButtonStyleType type, UIPopupMenuStyle * style);

	long                    GetItemHeight         () const;

	virtual void            Link                  ();

	ButtonStyleType         LookupButtonStyleType (const UILowerString & name, bool & isButton) const;

	const UISize &          GetItemMargin         () const;

	unsigned long           GetItemPopupTimeout   () const;
	void                    SetItemPopupTimeout   (unsigned long);

	long                    GetItemRadius         () const;

private:

	UIButtonStyle          *mButtonStyles [BST_NumButtonStyles];

	long                    mItemHeight;
	UISize                  mItemMargin;

	unsigned long           mItemPopupTimeout;

	UIPopupMenuStyle *      mPopupStyles [BST_NumButtonStyles];

	long                    mItemRadius;
};

//----------------------------------------------------------------------

inline UIButtonStyle * UIRadialMenuStyle::GetButtonStyle (ButtonStyleType type) const
{
	return mButtonStyles [type];
}

//----------------------------------------------------------------------

inline long UIRadialMenuStyle::GetItemHeight () const
{
	return mItemHeight;
}

//----------------------------------------------------------------------

inline const UISize & UIRadialMenuStyle::GetItemMargin () const
{
	return mItemMargin;
}

//----------------------------------------------------------------------

inline unsigned long UIRadialMenuStyle::GetItemPopupTimeout () const
{
	return mItemPopupTimeout;
}

//----------------------------------------------------------------------

inline void  UIRadialMenuStyle::SetItemPopupTimeout (unsigned long time)
{
	mItemPopupTimeout = time;
}

//----------------------------------------------------------------------

inline UIPopupMenuStyle * UIRadialMenuStyle::GetPopupStyle (ButtonStyleType type) const
{
	return mPopupStyles [type];
}

//----------------------------------------------------------------------

inline long UIRadialMenuStyle::GetItemRadius         () const
{
	return mItemRadius;
}

//-----------------------------------------------------------------

#endif // __UIRadialMenuStyle_H__