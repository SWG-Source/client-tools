//======================================================================
//
// UITreeViewStyle.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UITreeViewStyle_H
#define INCLUDED_UITreeViewStyle_H

//======================================================================
#include "UIWidgetStyle.h"

class UITextStyle;
class UIImageStyle;
class UIText;
class UIImage;
class UIImageStyle;

//----------------------------------------------------------------------

class UITreeViewStyle :
public UIWidgetStyle
{
public:

	static const char * const TypeName;

	class PropertyName
	{ //lint !e578 // symbol hides symbol
	public:
		static const UILowerString CellHeight;
		static const UILowerString CellPadding;
		static const UILowerString DefaultTextColor;
		static const UILowerString DefaultTextColorHighlight;
		static const UILowerString DefaultTextColorLowlight;
		static const UILowerString DefaultTextStyle;
		static const UILowerString GridColor;
		static const UILowerString ImageStyleCollapsed;
		static const UILowerString ImageStyleExpanded;
		static const UILowerString Indentation;
		static const UILowerString SelectionColorBackground;
		static const UILowerString SelectionColorRect;
		static const UILowerString SelectionTextColor;
		static const UILowerString SelectionTextColorHighlight;
		static const UILowerString SelectionTextColorLowlight;
	};

	enum ImageStyleType
	{
		IST_collapsed,
		IST_expanded,
		IST_count
	};

	enum DefaultTextColors
	{
		DTC_normal,
		DTC_highlight,
		DTC_lowlight,
		DTC_count
	};

	UITreeViewStyle ();
	~UITreeViewStyle ();

	virtual bool                IsA         (const UITypeID type) const;
	virtual const char         *GetTypeName () const        { return TypeName; }
	virtual UIBaseObject       *Clone       () const        { return new UITreeViewStyle; }

	virtual void                GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void                GetPropertyNames ( UIPropertyNameVector &, bool forCopy ) const;
	virtual void                GetLinkPropertyNames (UIPropertyNameVector &) const;

	virtual bool                SetProperty      ( const UILowerString & Name, const UIString &Value );
	virtual bool                GetProperty      ( const UILowerString & Name, UIString &Value ) const;

	void                        Link ();

	void                        SetGridColor (const UIColor & color);
	const UIColor &             GetGridColor () const;

	void                        SetCellPadding (const UISize &);
	void                        SetCellHeight (long height);

	const UISize &              GetCellPadding () const;
	long                        GetCellHeight () const;

	const UIColor &             GetSelectionColorBackground () const;
	const UIColor &             GetSelectionColorRect () const;

	UIText &                    GetDefaultText  (DefaultTextColors index = DTC_normal, bool selected = false);
	UIImage &                   GetDefaultImage (ImageStyleType type);
	UIImage &                   GetDefaultImage (UIImageStyle * style);

	void                        SetImageStyle (ImageStyleType type, UIImageStyle * style);
	UIImageStyle *              GetImageStyle (ImageStyleType type);

	long                        GetIndentation () const;

	const UIColor &             GetDefaultTextColor (DefaultTextColors index, bool selected = false);

private:

	UIColor                     mGridColor;
	long                        mCellHeight;
	UISize                      mCellPadding;
	UIText  *                   mDefaultRendererText;
	UIImage *                   mDefaultRendererImage;
	UIColor                     mSelectionColorBackground;
	UIColor                     mSelectionColorRect;
	UIImageStyle *              mImageStyles [IST_count];
	long                        mIndentation;

	UIColor                     mDefaultTextColors   [DTC_count];
	UIColor                     mSelectionTextColors [DTC_count];
};

//-----------------------------------------------------------------

inline const UIColor & UITreeViewStyle::GetGridColor () const
{
	return mGridColor;
}

//-----------------------------------------------------------------

inline const UISize & UITreeViewStyle::GetCellPadding () const
{
	return mCellPadding;
}

//-----------------------------------------------------------------

inline long UITreeViewStyle::GetCellHeight () const
{
	return mCellHeight;
}

//----------------------------------------------------------------------

inline const UIColor & UITreeViewStyle::GetSelectionColorBackground () const
{
	return mSelectionColorBackground;
}

//----------------------------------------------------------------------

inline const UIColor & UITreeViewStyle::GetSelectionColorRect () const
{
	return mSelectionColorRect;
}

//----------------------------------------------------------------------

inline long UITreeViewStyle::GetIndentation () const
{
	return mIndentation;
}

//======================================================================

#endif
