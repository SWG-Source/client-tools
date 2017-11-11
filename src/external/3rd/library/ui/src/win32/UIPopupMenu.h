#ifndef __UIPopupMenu_H__
#define __UIPopupMenu_H__

#include "UIEventCallback.h"
#include "UINotification.h"
#include "UIPage.h"

class UIButton;
class UIData;
class UIDataSource;
class UIDataSourceContainer;
class UIPage;
class UIPopupMenuStyle;

//----------------------------------------------------------------------

class UIPopupMenu :
public UIPage,
public UIEventCallback,
public UINotification
{
public:

	static const char * const TypeName;

	struct PropertyName
	{
		static const UILowerString DataSource;
		static const UILowerString Style;
	};


	struct DataProperties
	{
		static const UILowerString Text;
		static const UILowerString LocalText;
		static const UILowerString SubMenu;
		static const UILowerString Opacity;
		static const UILowerString IsLabel;
	};

	                                UIPopupMenu       ();
	// UIPopupMenus now have a link to the owning page.  When that page is deactivated,
	// the UIPopupMenu will also be closed.  Below is the prefered constructor.  The
	// default constructor is kept around for the UIStandardLoader template
									UIPopupMenu       (UIPage * owningPage);
	virtual                        ~UIPopupMenu       ();

	virtual bool                    IsA               (const UITypeID) const;
	virtual const char             *GetTypeName       () const;
	virtual UIBaseObject           *Clone             () const;

	virtual void                    GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void                    GetPropertyNames  (UIPropertyNameVector &, bool forCopy  ) const;
	virtual void                    GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool                    SetProperty       (const UILowerString & Name, const UIString &Value );
	virtual bool                    GetProperty       (const UILowerString & Name, UIString &Value ) const;

	virtual void                    OnButtonPressed       (UIWidget * context);
	virtual bool                    OnMessage             (UIWidget * context, const UIMessage & msg);
	void                            OnPopupMenuSelection  (UIWidget * context);

	void                            OnHoverIn         (UIWidget * context);
	void                            OnHoverOut        (UIWidget * context);

	bool                            ProcessMessage    (const UIMessage & );

	//-----------------------------------------------------------------

	void                            SetStyle          (UIPopupMenuStyle * );
	UIPopupMenuStyle               *GetPopupMenuStyle () const;
	virtual UIStyle                *GetStyle          () const;

	void                            SetPopupLocation  (const UIPoint & popupLocation);
	const UIPoint &                 GetPopupLocation  () const;

	//-----------------------------------------------------------------

	long                            GetItemCount      () const;

	UIDataSource *                  AddItem           (const std::string & name, const std::string &     globalLabel);
	UIDataSource *                  AddItem           (const std::string & name, const Unicode::String & localLabel);

	void                            ClearItems        ();
	const UINarrowString &          GetSelectedName   () const;
	long                            GetSelectedIndex  () const;
	const UIDataSource *            GetSelectedData   () const;

	//-----------------------------------------------------------------

	virtual void                    Notify            (UINotificationServer *, UIBaseObject *, UINotification::Code );
	void                            RecreateButtons   ();

	const UIDataSourceContainer *   GetDataSourceContainer  () const;
	UIDataSourceContainer *         GetDataSourceContainer  ();

	virtual void                    Link              ();

	void                            SetDataSourceContainer (UIDataSourceContainer * NewDataSource);

	long                            GetMinimumDesiredWidth () const;
	void                            SetMinimumDesiredWidth (long width);

	//-----------------------------------------------------------------

	void                            SetPopupButtonBehavior (bool b);

	const UIPoint &                 GetOffset              () const;

	void                            SetOffsetIndex         (int offsetIndex);

	static UIPopupMenu *            CreatePopupMenu        (UIWidget & widget);

	UIPage * GetOwningPage() const;

	UIButton * GetHoverButton();
private:

	                               UIPopupMenu( UIPopupMenu const & );
	UIPopupMenu                   &operator = ( UIPopupMenu const & );

	void                           LayoutButtons  ();
	long                           GetButtonIndex (const UIButton * button) const;
	void                           SetSubMenu        (UIPopupMenu * pop, UIButton * button);
	void                           SetHoverButton    (UIButton * button);
	void                           spawnSubMenu      (UIButton & button);

	UIPage * const mOwningPage;
	UIPopupMenuStyle              *mStyle;

	struct ButtonList;
	UIDataSourceContainer *        mDataSourceContainer;
	UINarrowString                 mSelectedName;

	long                           mMinimumDesiredWidth;

	long                           mSelectedIndex;

	UIPoint                        mPopupLocation;

	UIPopupMenu *                  mSubMenu;
	UIButton *                     mSubMenuButton;
	UIButton *                     mHoverButton;
	long                           mHoverCountDown;

	bool                           mPopupButtonBehavior;

	UIPoint                        mOffset;

	bool                           mButtonWasPressed;
};

//-----------------------------------------------------------------

inline const UIDataSourceContainer * UIPopupMenu::GetDataSourceContainer () const
{
	return mDataSourceContainer;
}

//-----------------------------------------------------------------

inline UIDataSourceContainer * UIPopupMenu::GetDataSourceContainer ()
{
	return mDataSourceContainer;
}

//-----------------------------------------------------------------

inline const UINarrowString &          UIPopupMenu::GetSelectedName () const
{
	return mSelectedName;
}

//----------------------------------------------------------------------

inline long UIPopupMenu::GetMinimumDesiredWidth () const
{
	return mMinimumDesiredWidth;
}

//----------------------------------------------------------------------

inline long UIPopupMenu::GetSelectedIndex () const
{
	return mSelectedIndex;
}

//----------------------------------------------------------------------

inline const UIPoint & UIPopupMenu::GetPopupLocation  () const
{
	return mPopupLocation;
}

//----------------------------------------------------------------------

inline const UIPoint & UIPopupMenu::GetOffset              () const
{
	return mOffset;
}

//----------------------------------------------------------------------

inline UIButton * UIPopupMenu::GetHoverButton()
{
	return mHoverButton;
}


//-----------------------------------------------------------------
#endif // __UIPopupMenu_H__
