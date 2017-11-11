//======================================================================
//
// UITreeView.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UITreeView_H
#define INCLUDED_UITreeView_H

//======================================================================

#include "UIWidget.h"
#include "UINotification.h"
#include "UIEventCallback.h"

//----------------------------------------------------------------------

class UITreeViewModel;
class UIText;
class UIImage;
class UIDataSourceContainer;
class UITreeViewStyle;

//----------------------------------------------------------------------

class UITreeView :
public UIWidget,
public UINotification,
public UINotificationServer,
public UIEventCallback
{
public:

	class                      DataNode;

	static const char * const TypeName;

	struct DataProperties
	{
		static const UILowerString LocalText;
		static const UILowerString Text;
		static const UILowerString Underline;
		static const UILowerString Icon;
		static const UILowerString Expanded;
		static const UILowerString ColorIndex;
		static const UILowerString Selectable;
	};

	struct PropertyName
	{ //lint !e578 // symbol hides symbol
		static const UILowerString DataSourceContainer;
		static const UILowerString SelectedRow;
		static const UILowerString SelectedRowCount;
		static const UILowerString SelectionAllowedMultiRow;
		static const UILowerString Style;
		static const UILowerString ShowIcon;
		static const UILowerString ShowNavigationIcons;
		static const UILowerString ScrollWheelSelection;
		// selection fg, bg colors
	};

	UITreeView ();
	~UITreeView ();

	virtual bool                IsA                          (const UITypeID Type) const;
	virtual const char         *GetTypeName                  () const              { return TypeName; }
	virtual UIBaseObject       *Clone                        () const              { return new UITreeView; }

	virtual void                GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void                GetPropertyNames             (UIPropertyNameVector &, bool forCopy ) const;
	virtual void                GetLinkPropertyNames         (UIPropertyNameVector &) const;

	virtual bool                SetProperty                  (const UILowerString & Name, const UIString &Value );
	virtual bool                GetProperty                  (const UILowerString & Name, UIString &Value ) const;

	virtual UIStyle            *GetStyle                     () const;
	virtual void                Render                       (UICanvas &) const;

	virtual void                Notify                       (UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode );

	virtual void                SetSize                      (const UISize &);
	virtual bool                ProcessMessage               (const UIMessage & );
	virtual bool                OnMessage                    (UIWidget * context, const UIMessage & msg);

	void                        SetDataSourceContainer       (UIDataSourceContainer * model, bool cache = true);
	UIDataSourceContainer *     GetDataSourceContainer       ();

	virtual void                Link                         ();

	bool                        IsRowSelected                (long row) const;
	long                        GetLastSelectedRow           () const;

	long                        GetRowCount                  ();

	UIDataSourceContainer *     GetDataSourceContainerAtRow  (long row);

	bool                        GetRowFromPoint              (const UIPoint & pt, long & row, DataNode ** node) const;
	DataNode *                  GetDataNodeAtRow             (long row);

	void                        SelectRow                    (long row);
	void                        AddRowSelection              (long row);
	void                        RemoveRowSelection           (long row);

	virtual void                GetScrollSizes               (UISize &PageSize, UISize &LineSize ) const;
	void                        ScrollToRow                  (int targetRow);

	void                        SetStyle                     (UITreeViewStyle * style);

	void                        SetSelectionAllowedMultiRow  (bool b);

	void                        SetSelectionMovesWithMouse   (bool b);

	void                        SetSelectionFinalized        (bool b);
	bool                        GetSelectionFinalized        ();

	void                        SetRowExpanded               (long row, bool b);
	void                        SetAllRowsExpanded           (bool b);
	bool                        GetRowExpanded               (long row);

	bool                        SetNodeExpanded              (DataNode & node, long row, bool b, bool applyToChildren = false);

	DataNode *                  FindDataNodeByDataSource     (const UIBaseObject & dataSource, int & row) const;
	void                        ExpandParentNodes            (DataNode & node);

	void                        ClearData                    ();

private:

	void                        updateExtent                 ();

	void                        addRangeSelection            (long one, long two);
	void                        removeRangeSelection         (long one, long two);

	bool                        GetTotalRowExtents           (int row, long & top, long & bottom);

	bool                        MoveStuff                    (long targetRow, bool changesSelection, bool shifted);

	void                        CacheData                    ();

	void                        ToggleExpanded               (int row, DataNode & node);

	UITreeView (const UITreeView &);
	UITreeView & operator= (const UITreeView &);

	UIDataSourceContainer *     mDataSourceContainer;

	typedef ui_stdlist<long>::fwd LongList;
	LongList *                  mSelectedRows;

	bool                        mSelectionAllowedRow;
	bool                        mSelectionAllowedRowMultiple;

	bool                        mMouseDown;
	long                        mMouseDownRow;
	long                        mMouseDragLastRow;
	UIPoint                     mMouseMoveLastPoint;

	UISize                      mListExtent;

	UITreeViewStyle *           mStyle;

	bool                        mSelectionMovesWithMouse;
	bool                        mSelectionMovingWithWheel;

	bool                        mSelectionFinalized;

	bool                        mShowIcon;
	bool                        mShowNavigationIcons;

	DataNode *                  mDataCache;

	bool                        mScrollWheelSelection;
	bool                        mCachingInProgress;

};

//----------------------------------------------------------------------

inline UIDataSourceContainer * UITreeView::GetDataSourceContainer ()
{
	return mDataSourceContainer;
}

//----------------------------------------------------------------------

/**
* A selection is 'finalized' by releasing the left mouse button over the selection.
*/
inline bool UITreeView::GetSelectionFinalized ()
{
	return mSelectionFinalized;
}

//======================================================================

#endif
