//======================================================================
//
// UIListStyle.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UIListStyle_H
#define INCLUDED_UIListStyle_H

//======================================================================
#include "UIWidgetStyle.h"

class UITextStyle;
class UIImageStyle;
class UIText;
class UIImage;

//----------------------------------------------------------------------

class UIListStyle :
public UIWidgetStyle
{
public:

	static const char * const TypeName;

	class PropertyName
	{ //lint !e578 // symbol hides symbol
	public:
		static const UILowerString  BackgroundColor;
		static const UILowerString  BackgroundOpacity;
		static const UILowerString  CellHeight;
		static const UILowerString  CellPadding;
		static const UILowerString  DefaultTextColor;
		static const UILowerString  DefaultTextStyle;
		static const UILowerString  GridColor;
		static const UILowerString  Margin;
		static const UILowerString  SelectionColorBackground;
		static const UILowerString  SelectionColorRect;
		static const UILowerString  SelectionTextColor;
	};


	UIListStyle ();
	~UIListStyle ();

	virtual bool                IsA                          (const UITypeID type) const;
	virtual const char         *GetTypeName                  () const        { return TypeName; }
	virtual UIBaseObject       *Clone                        () const        { return new UIListStyle; }

	virtual void                GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void                GetPropertyNames             (UIPropertyNameVector &, bool forCopy ) const;
	virtual void                GetLinkPropertyNames (UIPropertyNameVector &) const;

	virtual bool                SetProperty                  (const UILowerString & Name, const UIString &Value );
	virtual bool                GetProperty                  (const UILowerString & Name, UIString &Value ) const;

	void                        Link                         ();

	void                        SetGridColor                 (const UIColor & color);
	const UIColor &             GetGridColor                 () const;

	void                        SetCellPadding               (const UISize &);
	void                        SetCellHeight                (long height);

	const UISize &              GetCellPadding               () const;
	long                        GetCellHeight                () const;

	const UIColor &             GetSelectionColorBackground  () const;
	const UIColor &             GetSelectionColorRect        () const;

	UIText &                    GetDefaultText               ();
	UIImage &                   GetDefaultImage              ();

	const UIRect &              GetMargin                    () const;
	void                        SetMargin                    (const UIRect & rect);

	const float                 GetBackgroundOpacity         () const;
	void                        SetBackgroundOpacity         (float f);

	const UIColor &             GetBackgroundColor           () const;
	void                        SetBackgroundColor           (const UIColor & color);

	const UIColor &             GetSelectionTextColor        () const;
	void                        SetSelectionTextColor        (const UIColor & color);

	const UIColor &             GetDefaultTextColor          () const;

private:

	UIColor                     mGridColor;
	long                        mCellHeight;
	UISize                      mCellPadding;
	UIText  *                   mDefaultRendererText;
	UIImage *                   mDefaultRendererImage;
	UIColor                     mSelectionColorBackground;
	UIColor                     mSelectionColorRect;
	UIColor                     mSelectionTextColor;
	UIColor                     mDefaultTextColor;

	UIRect                      mMargin;

	UIColor                     mBackgroundColor;
	float                       mBackgroundOpacity;
};

//-----------------------------------------------------------------

inline const UIColor & UIListStyle::GetGridColor () const
{
	return mGridColor;
}

//-----------------------------------------------------------------

inline const UISize & UIListStyle::GetCellPadding () const
{
	return mCellPadding;
}

//-----------------------------------------------------------------

inline long UIListStyle::GetCellHeight () const
{
	return mCellHeight;
}

//----------------------------------------------------------------------

inline const UIColor & UIListStyle::GetSelectionColorBackground () const
{
	return mSelectionColorBackground;
}

//----------------------------------------------------------------------

inline const UIColor & UIListStyle::GetSelectionColorRect () const
{
	return mSelectionColorRect;
}

//----------------------------------------------------------------------

inline UIText & UIListStyle::GetDefaultText ()
{
	return *mDefaultRendererText;
}

//----------------------------------------------------------------------

inline UIImage & UIListStyle::GetDefaultImage ()
{
	return *mDefaultRendererImage;
}

//----------------------------------------------------------------------

inline const UIRect & UIListStyle::GetMargin () const
{
	return mMargin;
}

//----------------------------------------------------------------------

inline const float UIListStyle::GetBackgroundOpacity () const
{
	return mBackgroundOpacity;
}

//----------------------------------------------------------------------

inline const UIColor & UIListStyle::GetBackgroundColor () const
{
	return mBackgroundColor;
}

//----------------------------------------------------------------------

inline const UIColor & UIListStyle::GetSelectionTextColor () const
{
	return mSelectionTextColor;
}

//----------------------------------------------------------------------

inline const UIColor & UIListStyle::GetDefaultTextColor          () const
{
	return mDefaultTextColor;
}

//======================================================================

#endif
