// ======================================================================
//
// UITableHeader.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UITableHeader_H
#define INCLUDED_UITableHeader_H

// ======================================================================

#include "UIPage.h"
#include "UINotification.h"
#include "UIEventCallback.h"

class UITable;
class UITableModel;
class UIButtonStyle;
class UIImageStyle;
class UICursor;

//-----------------------------------------------------------------

class UITableHeader :
public UIPage,
public UINotification,
public UIEventCallback
{
public:

	UITableHeader ();
	~UITableHeader ();

	static const char * const TypeName;

	class PropertyName
	{ //lint !e578 // symbol hides symbol
	public:
		static const UILowerString Table;
		static const UILowerString ButtonStyle;
		static const UILowerString ImageSortUp;
		static const UILowerString ImageSortDown;
		static const UILowerString CursorResize;
		static const UILowerString PadFront;
		static const UILowerString PadBack;
		// selection fg, bg colors
	};

	virtual bool                IsA                 (const UITypeID Type ) const;
	virtual const char         *GetTypeName         () const        { return TypeName; }
	virtual UIBaseObject       *Clone               () const              { return new UITableHeader; }

	virtual void                GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void                GetPropertyNames    (UIPropertyNameVector &, bool forCopy ) const;
	virtual void                GetLinkPropertyNames (UIPropertyNameVector &) const;

	virtual bool                SetProperty         (const UILowerString & Name, const UIString &Value );
	virtual bool                GetProperty         (const UILowerString & Name, UIString &Value ) const;

	virtual UIStyle            *GetStyle            () const { return 0; }
	virtual void                Render              (UICanvas & ) const;

	virtual void                Notify              (UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode );

	virtual void                SetSize             (const UISize & );
	virtual bool                ProcessMessage      (const UIMessage & );

	void                        Link                ();
	void                        OnButtonPressed     (UIWidget * context);
	virtual bool                OnMessage           (UIWidget *Context, const UIMessage & msg );

	UITable *                   GetTable            ();

	void                        SetTable            (UITable * table);

	void                        SetButtonStyle      (UIButtonStyle * style);
	void                        SetImageSortUp      (UIImageStyle * style);
	void                        SetImageSortDown    (UIImageStyle * style);
	void                        SetCursorResize     (UICursor     * cursor);

	void                        ClearHeaderChildren ();

private:
	UITableHeader (const UITableHeader & rhs);
	UITableHeader & operator= (const UITableHeader & rhs);

	void                        RecreateButtons (bool force);
	void                        UpdateButtonLayout ();
	void                        UpdateButtonLabels ();

	UITable *                   mTable;
	UIButtonStyle *             mButtonStyle;
	UIImageStyle *              mImageSortUp;
	UIImageStyle *              mImageSortDown;

	UICursor *                  mCursorResize;

	long                        mResizerWidth;

	bool                        mDividerButtonDown;
	long                        mDividerColumnRight;

	UIPoint                     mLastDragPoint;

	bool                        mPadFront;
	bool                        mPadBack;

	bool                        mHasRecreated;
};

//-----------------------------------------------------------------

inline UITable * UITableHeader::GetTable ()
{
	return mTable;
}

// ======================================================================

#endif
