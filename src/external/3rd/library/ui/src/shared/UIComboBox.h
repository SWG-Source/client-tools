//======================================================================
//
// UIComboBox.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UIComboBox_H
#define INCLUDED_UIComboBox_H

//======================================================================

#include "UIPage.h"
#include "UIEventCallback.h"

class UIButtonStyle;
class UITextboxStyle;
class UISliderbarStyle;
class UIButton;
class UITextbox;
class UIScrollbar;
class UIDataSource;
class UIListStyle;
class UIList;
class UIData;

//----------------------------------------------------------------------

class UIComboBox :
public UIPage,
public UIEventCallback
{
public:
	static const char       * const TypeName;

	class PropertyName
	{ //lint !e578 // symbol hides symbol
	public:
		static const UILowerString ButtonMargin;
		static const UILowerString ButtonStyle;
		static const UILowerString ButtonWidth;
		static const UILowerString DataSource;
		static const UILowerString DropDownHeight;
		static const UILowerString Editable;
		static const UILowerString ListMargin;
		static const UILowerString ListStyle;
		static const UILowerString ScrollbarMargin;
		static const UILowerString ScrollbarStyle;
		static const UILowerString ScrollbarWidth;
		static const UILowerString SelectedIndex;
		static const UILowerString SelectedItem;
		static const UILowerString SelectedText;
		static const UILowerString TextboxMargin;
		static const UILowerString TextboxStyle;
	};

	UIComboBox ();
	~UIComboBox ();

	virtual bool            IsA (const UITypeID) const;
	virtual const char     *GetTypeName () const;
	virtual UIBaseObject   *Clone () const;

	virtual bool            SetProperty (const UILowerString & Name, const UIString &Value);
	virtual bool            GetProperty (const UILowerString & Name, UIString &Value) const;
	virtual void            GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const;
	virtual void            GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual void            GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool            OnMessage            (UIWidget *context, const UIMessage & msg);
	virtual void            OnHide               (UIWidget *context);
	virtual void            OnButtonPressed      (UIWidget *context);
	virtual void            OnGenericSelectionChanged (UIWidget * context);


	virtual bool            AddChild( UIBaseObject * );
	virtual bool            InsertChildBefore (UIBaseObject * childToAdd, const UIBaseObject * childToPrecede);
	virtual bool            InsertChildAfter  (UIBaseObject * childToAdd, const UIBaseObject * childTosucceed);

	void                    SetButtonStyle       (UIButtonStyle * style);
	void                    SetTextboxStyle      (UITextboxStyle * style);
	void                    SetListStyle         (UIListStyle * style);
	void                    SetScrollbarStyle    (UISliderbarStyle * style);
	void                    SetDataSource        (UIDataSource * ds);

	void                    SetButtonWidth       (long width);

	void                    Link ();

	void                    PerformPopup ();

	long                    GetSelectedIndex () const;
	void                    SetSelectedIndex (long index, bool updateValue = true);

	UIData *                GetDataAtIndex (long index);
	const UIData *          GetDataAtIndex (long index) const;

	UIDataSource *          GetDataSource ();
	const UIDataSource *    GetDataSource () const;

	virtual void            Pack();

	void                    SetTextboxMargin   (const UIRect & rect);
	void                    SetButtonMargin    (const UIRect & rect);
	void                    SetListMargin      (const UIRect & rect);
	void                    SetScrollbarMargin (const UIRect & rect);

	virtual void            Clear ();
	UIData *                AddItem (const Unicode::String &localizedString, const std::string &name);
	int                     GetItemCount() const;
	bool                    GetIndexLocalText(int const index, Unicode::String &localText) const;
	bool                    GetIndexName(int const index, std::string &name) const;
	void                    GetSelectedIndexLocalText(Unicode::String &localText) const;
	void                    GetSelectedIndexName(std::string &name) const;

	void                    SetScrollbarOpacity(float opacity);

private:

	UIComboBox (const UIComboBox &);
	UIComboBox & operator= (const UIComboBox &);

	void                    UpdateValue  ();
	void                    UpdateLayout ();

	UIScrollbar *           mScroll;
	UIButton *              mButton;
	UITextbox *             mTextbox;
	UIList *                mList;
	UIPage *                mPopupPage;

	UIButtonStyle *         mButtonStyle;
	UITextboxStyle *        mTextboxStyle;
	UIListStyle *           mListStyle;
	UISliderbarStyle *      mScrollbarStyle;

	UIDataSource *          mDataSource;

	bool                    mEditable;
	bool                    mMouseIsDown;

	long                    mDropDownHeight;
	long                    mSelectedIndex;

	UIRect                  mTextboxMargin;
	UIRect                  mButtonMargin;
	UIRect                  mListMargin;
	UIRect                  mScrollbarMargin;

	long                    mScrollbarWidth;

	float                   mScrollbarOpacity;
};

//-----------------------------------------------------------------

inline long UIComboBox::GetSelectedIndex () const
{
	return mSelectedIndex;
}

//-----------------------------------------------------------------

inline void UIComboBox::SetScrollbarOpacity(float opacity)
{
	mScrollbarOpacity = opacity;
}

//======================================================================

#endif
