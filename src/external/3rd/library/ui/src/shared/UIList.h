//======================================================================
//
// UIList.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UIList_H
#define INCLUDED_UIList_H

//======================================================================

#include "UIWidget.h"
#include "UINotification.h"
#include "UIEventCallback.h"

//----------------------------------------------------------------------

class UIListModel;
class UIText;
class UIImage;
class UIDataSource;
class UIListStyle;
class UIData;

//----------------------------------------------------------------------

class UIList :
public UIWidget,
public UINotification,
public UINotificationServer,
public UIEventCallback
{
public:
	UIList ();
	~UIList ();

	static const char * const TypeName;

	struct DataProperties
	{
		static const UILowerString  LOCALTEXT;
		static const UILowerString  TEXT;
	};

	struct PropertyName
	{ //lint !e578 // symbol hides symbol
		static const UILowerString  DataSource;
		static const UILowerString  SelectedRow;
		static const UILowerString  SelectedRows;
		static const UILowerString  SelectedRowCount;
		static const UILowerString  SelectedItem;
		static const UILowerString  SelectionAllowedMultiRow;
		static const UILowerString  SelectionClickToggleMode;
		static const UILowerString  SelectionAllowedDragSelection;
		static const UILowerString  SelectionAllowedKeySelection;
		static const UILowerString  Style;
		static const UILowerString  ScrollWheelSelection;
		static const UILowerString  OnSelectionChanged;
		static const UILowerString  RenderSeperatorLines;
	};

	virtual bool                IsA                         (const UITypeID Type ) const;
	virtual const char         *GetTypeName                 () const;
	virtual UIBaseObject       *Clone                       () const;

	virtual void                GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void                GetPropertyNames            (UIPropertyNameVector &, bool forCopy ) const;
	virtual void                GetLinkPropertyNames (UIPropertyNameVector &) const;

	virtual bool                SetProperty                 (const UILowerString & Name, const UIString &Value );
	virtual bool                GetProperty                 (const UILowerString & Name, UIString &Value ) const;

	virtual UIStyle            *GetStyle                    () const;
	virtual void                Render                      (UICanvas & ) const;

	virtual void                Notify                      (UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode );

	virtual void                SetSize                     (const UISize & );
	virtual bool                ProcessMessage              (const UIMessage & );
	virtual bool                OnMessage                   (UIWidget * context, const UIMessage & msg);

	void                        SetDataSource               (UIDataSource * model);
	UIDataSource *              GetDataSource               ();

	virtual void                Link                        ();

	bool                        IsRowSelected               (long row) const;
	long                        GetLastSelectedRow          () const;
	long                        GetLastToggledRow           () const;

	UIData *                    GetDataAtRow                (long row);
	UIData const *              GetDataAtRow                (long row) const;

	bool                        GetRowFromPoint             (const UIPoint & pt, long & row) const;

	void                        SelectRow                   (long row, bool const sendCallback = true);
	void                        AddRowSelection             (long row, bool const sendCallback = true);
	void                        RemoveRowSelection          (long row, bool const sendCallback = true);
	void                        ToggleRowSelection          (long row, bool const sendCallback = true);

	virtual void                GetScrollSizes              (UISize &PageSize, UISize &LineSize ) const;
	void                        ScrollToRow                 (int targetRow);

	void                        SetStyle                    (UIListStyle * style);

	void                        SetSelectionAllowedMultiRow (bool b);
	void                        SetSelectionClickToggleMode (bool b);
	void                        SetSelectionAllowedDragSelection (bool b);
	void                        SetSelectionAllowedKeySelection (bool b);

	void                        SetSelectionMovesWithMouse  (bool b);

	void                        SetSelectionFinalized       (bool b);
	bool                        GetSelectionFinalized       ();

	void                        SetRenderSeperatorLines     (bool b);
	bool                        GetRenderSeperatorLines     ();

	int                         GetRowCount                 () const;

	void                        UpdateFromStyle             ();

	void                        SetScrollWheelSelection     (bool b);

	void                        Clear                       ();
	void                        AddRow                      (Unicode::String const &localizedString, std::string const &name);
	bool                        RemoveRow                   (long row);
	bool                        GetText                     (int row, std::string &text);
	bool                        GetLocalText                (int row, Unicode::String &text);
	bool                        SetLocalText                (int row, Unicode::String const &text);

private:

	void                        updateExtent                ();

	void                        addRangeSelection           (long one, long two);
	void                        removeRangeSelection        (long one, long two);

	bool                        GetTotalRowExtents          (int row, long & top, long & bottom);

	bool                        MoveStuff                   (long targetRow, bool changesSelection, bool shifted);

	UIList (const UIList &);
	UIList & operator= (const UIList &);

	UIDataSource *              mDataSource;

private:

	typedef ui_stdlist<long>::fwd LongList;
	LongList *                  mSelectedRows;
	long                        mLastToggledRow;

	bool                        mSelectionAllowedRow;
	bool                        mSelectionAllowedRowMultiple;
	bool                        mSelectionClickToggleMode;
	bool                        mSelectionAllowedDragSelection;
	bool                        mSelectionAllowedKeySelection;

	bool                        mMouseDown;
	long                        mMouseDownRow;
	long                        mMouseDragLastRow;
	UIPoint                     mMouseMoveLastPoint;

	UISize                      mListExtent;

	UIListStyle *               mStyle;

	bool                        mSelectionMovesWithMouse;
	bool                        mSelectionMovingWithWheel;

	bool                        mSelectionFinalized;
	bool                        mScrollWheelSelection;
	mutable bool                mSuppressSelectionChangedCallback;
	bool                        mRenderSeperatorLines;
};

//----------------------------------------------------------------------

inline UIDataSource * UIList::GetDataSource ()
{
	return mDataSource;
}

//----------------------------------------------------------------------

/**
* A selection is 'finalized' by releasing the left mouse button over the selection.
*/
inline bool UIList::GetSelectionFinalized ()
{
	return mSelectionFinalized;
}

//======================================================================

#endif
