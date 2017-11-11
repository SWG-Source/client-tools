#ifndef __UIRadialMenu_H__
#define __UIRadialMenu_H__

#include "UIEventCallback.h"
#include "UIPage.h"

class UIRadialMenuStyle;
class UIButton;
class UIPage;
class UIDataSource;
class UIDataSourceContainer;
class UIData;
class UIPopupMenu;
class UINamespace;

//----------------------------------------------------------------------

class UIRadialMenu :
public UIPage,
public UIEventCallback,
public UINotification
{
public:

	static const char          *TypeName;

	struct PropertyName
	{
		static const UILowerString  DataSource;
		static const UILowerString  Style;
		static const UILowerString  PopupDataNamespace;
		static const UILowerString  RadialCenterPrototype;
		static const UILowerString  RadialCenterMargin;
	};

	struct DataPropertyName
	{
		static const UILowerString Text;
		static const UILowerString Icon;
	};

	UIRadialMenu();
	                               ~UIRadialMenu         ();

	bool                            IsA                  (const UITypeID ) const;
	const char                     *GetTypeName          () const;
	UIBaseObject                   *Clone                () const;

	virtual void                    GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	void                            GetPropertyNames     (UIPropertyNameVector &, bool forCopy ) const;
	virtual void                    GetLinkPropertyNames (UIPropertyNameVector &) const;


	bool                            SetProperty          (const UILowerString & Name, const UIString &Value );
	bool                            GetProperty          (const UILowerString & Name, UIString &Value ) const;

	void                            OnButtonPressed      (UIWidget * context);
	void                            OnShow               (UIWidget * context);
	void                            OnHide               (UIWidget * context);

	bool                            OnMessage            (UIWidget * context, const UIMessage & msg);
	void                            OnPopupMenuSelection (UIWidget * context);

	bool                            ProcessMessage       (const UIMessage & );

	void                            Render               (UICanvas & ) const;

	//-----------------------------------------------------------------

	void                            SetStyle             (UIRadialMenuStyle * );
	UIRadialMenuStyle              *GetRadialMenuStyle   () const;
	UIStyle                        *GetStyle             () const;

	//-----------------------------------------------------------------

	long                            GetItemCount         () const;
	bool                            AddItem              (const Unicode::NarrowString & name, const Unicode::NarrowString & globalLabel, bool enabled = true, float opacity = 1.0f);
	void                            ClearItems           ();
	const UINarrowString &          GetSelectedName      () const;

	//-----------------------------------------------------------------

	void                            Notify               (UINotificationServer *, UIBaseObject *, UINotification::Code );
	void                            RecreateButtons      ();

	const UIDataSource *            GetDataSource        () const;
	UIDataSource *                  GetDataSource        ();

	void                            Link                 ();

	void                            SetDataSource        (UIDataSource *NewDataSource );

	//-----------------------------------------------------------------

	const UIPopupMenu *             GetPopupMenu         () const;

	const UIButton *                GetRadialButton      (int index) const;

	UIDataSourceContainer *         GetPopupDataSourceContainer (int index, bool create = false);
	UIDataSourceContainer *         GetPopupDataSourceContainer (const UINarrowString & name, bool create = false);

	void                            SetRadialCenterPrototype (UIWidget * proto);
	UIWidget *                      GetRadialCenterWidget () const;

	static UIDataSource *           AddPopupDataSourceItem (UIDataSourceContainer * dsc, const std::string & name, const std::string & globalLabel, bool enabled = true, float opacity = 1.0f);
	static UIDataSource *           AddPopupDataSourceItem (UIDataSourceContainer * dsc, const std::string & name, const UIString & localLabel,     bool enabled = true, float opacity = 1.0f);

	//----------------------------------------------------------------------

private:

	                               UIRadialMenu( UIRadialMenu & );
	UIRadialMenu                   &operator = ( UIRadialMenu & );

	void                           LayoutButtons         ();
	void                           SetPopupMenu          (UIPopupMenu * menu);
	void                           SpawnPopupMenu        (int index);
	void                           PlacePopup            (int index, UIPopupMenu * pop);

	void                           SetPopupDataNamespace (UINamespace * ns);

	UIRadialMenuStyle *            mStyle;

	struct ButtonList;
	UIDataSource *                 mDataSource;
	UINarrowString                 mSelectedName;

	int                            mHoverButtonIndex;
	unsigned long                  mHoverButtonTime;

	unsigned long                  mLastNotifyTime;

	unsigned long                  mPopupMenuVanishTime;

	int                            mPopupButtonIndex;
	UIPopupMenu *                  mPopupMenu;

	UINamespace *                  mPopupDataNamespace;
	bool                           mRecursionGuard;

	UIWidget *                     mRadialCenterWidget;
	UIRect                         mRadialCenterMargin;
};

//-----------------------------------------------------------------

inline const UIDataSource * UIRadialMenu::GetDataSource () const
{
	return mDataSource;
}

//-----------------------------------------------------------------

inline UIDataSource * UIRadialMenu::GetDataSource ()
{
	return mDataSource;
}

//-----------------------------------------------------------------

inline const UINarrowString & UIRadialMenu::GetSelectedName () const
{
	return mSelectedName;
}

//----------------------------------------------------------------------

inline const UIPopupMenu * UIRadialMenu::GetPopupMenu () const
{
	return mPopupMenu;
}

//----------------------------------------------------------------------

inline UIWidget * UIRadialMenu::GetRadialCenterWidget () const
{
	return mRadialCenterWidget;
}

//-----------------------------------------------------------------
#endif // __UIRadialMenu_H__