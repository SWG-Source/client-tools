#ifndef __UITabbedPane_H__
#define __UITabbedPane_H__

#include "UIEventCallback.h"
#include "UINotification.h"
#include "UIPage.h"

class UITabbedPaneStyle;
class UIButton;
class UIPage;
class UIDataSource;
class UIData;

//----------------------------------------------------------------------

class UITabbedPane : public UIPage, public UIEventCallback, public UINotification
{
public:

	static const char          *TypeName;

	struct PropertyName
	{
		static const UILowerString TargetPage;
		static const UILowerString DataSource;
		static const UILowerString ActiveTab;
		static const UILowerString Style;
		static const UILowerString TabObject;
	};

	struct DataProperties
	{
		static const UILowerString DATA_CONTEXT_CAPABLE;
		static const UILowerString DATA_CONTEXT_TO_PARENT;
		static const UILowerString DATA_DRAGTYPE;
		static const UILowerString DATA_DROP_TO_PARENT;
		static const UILowerString DATA_EXTRA_PROPS;
		static const UILowerString DATA_ICON_COLOR;
		static const UILowerString DATA_ICON_PATH;
		static const UILowerString DATA_NAME;
		static const UILowerString DATA_TARGET;
		static const UILowerString Text;
		static const UILowerString LocalText;
	};

	UITabbedPane();
	virtual                        ~UITabbedPane         ();

	virtual bool                    IsA                  (const UITypeID ) const;
	virtual const char             *GetTypeName          () const;
	virtual UIBaseObject           *Clone                () const;

	virtual void                    GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void                    GetPropertyNames     (UIPropertyNameVector &, bool forCopy ) const;
	virtual void                    GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool                    SetProperty          (const UILowerString & Name, const UIString &Value);
	virtual bool                    GetProperty          (const UILowerString & Name, UIString &Value) const;

	virtual void                    OnButtonPressed      (UIWidget * context);
	virtual bool                    OnMessage            (UIWidget *Context, const UIMessage & msg );

	virtual bool                    ProcessMessage       (const UIMessage & );

	virtual void                    Render               (UICanvas &) const;

	void                            SetStyle             (UITabbedPaneStyle * );
	        UITabbedPaneStyle      *GetTabbedPaneStyle   () const;
	virtual UIStyle                *GetStyle             () const;

	virtual UIWidget               *GetWidgetFromPoint   (const UIPoint &, bool mustGetInput ) const;

	void                            GetScrollSizes       (UISize &PageSize, UISize &LineSize, bool rightLeft, bool downUp) const;

	//----------------------------------------------------------------------

	long                            GetTabCount          () const;
	long                            GetActiveTab         () const;
	void                            SetActiveTab         (long index);
	void                            SetActiveTab         (const std::string & name);
	UIData *                        GetTabData           (long index) const;
	bool                            GetTabName           (long index, UINarrowString & name) const;
	long                            GetTabFromPoint      (const UIPoint &) const;
	long                            FindTabIndex         (const UIButton & button);
	UIButton *                      GetTabButton         (long index);

	//----------------------------------------------------------------------

	UIWidget *                      GetActiveWidget      () const;
	UIButton *                      GetActivateTabButton () const;

	virtual void                    Notify               (UINotificationServer *, UIBaseObject *, UINotification::Code );
	virtual void                    SetSize              (const UISize &NewSize );
	void                            RecreateButtons      ();
	UIData *                        AppendTab            (const Unicode::String & localLabel, UIWidget * tabTarget);
	void                            Clear                ();

	const UIDataSource *            GetDataSource        () const;
	UIDataSource *                  GetDataSource        ();

	virtual void                    Link                 ();

	void                            SetDataSource        (UIDataSource *NewDataSource );
	void                            SetTargetPage        (UIPage * NewTargetPage);

	//-----------------------------------------------------------------

	void                            SetButtonIcon        (int index, UIImageStyle * icon);
	void                            SetButtonIconColor   (int index, const UIColor & color);
	void                            SetButtonText        (int index, const Unicode::String & str);

	virtual void                    SetScrollLocation    (const UIPoint & );

private:

	                               UITabbedPane          (UITabbedPane & );
	UITabbedPane                  &operator =            (UITabbedPane & );

	void                          LayoutButtons          ();

	UITabbedPaneStyle             *mStyle;
	long                          mActiveTab;
	UIDataSource                  *mDataSource;

	UIPage                        *mTargetPage;

	bool mCallbackForwardingRecursionGuard : 1;
	bool  mIgnoreDataChange : 1;
	mutable bool mDirtySemaphore : 1;

	UISmartPointer<UIWidget> mTabObject;
};

//-----------------------------------------------------------------

inline long UITabbedPane::GetActiveTab () const
{
	return mActiveTab;
};

//-----------------------------------------------------------------

inline const UIDataSource *            UITabbedPane::GetDataSource () const
{
	return mDataSource;
}

//-----------------------------------------------------------------

inline UIDataSource *                  UITabbedPane::GetDataSource ()
{
	return mDataSource;
}

//-----------------------------------------------------------------
#endif // __UITabbedPane_H__