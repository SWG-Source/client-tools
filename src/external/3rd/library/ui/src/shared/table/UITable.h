//======================================================================
//
// UITable.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UITable_H
#define INCLUDED_UITable_H

//======================================================================

#include "UIWidget.h"
#include "UINotification.h"
#include "UIEventCallback.h"

//----------------------------------------------------------------------

class UITableModel;
class UIText;
class UIImage;
class UIDataSource;
class UIData;

//----------------------------------------------------------------------

class UITable :
public UIWidget,
public UINotification,
public UINotificationServer,
public UIEventCallback
{
public:
	UITable ();
	~UITable ();

	typedef ui_stdvector<float>::fwd FloatVector;
	typedef ui_stdvector<long>::fwd  LongVector;

	static const char * const TypeName;

	class PropertyName
	{ //lint !e578 // symbol hides symbol
	public:
		static const UILowerString TableModel;
		static const UILowerString ColumnResizePolicy;
		static const UILowerString SelectedRow;
		static const UILowerString SelectedRowCount;
		static const UILowerString SelectionAllowedRow;
		static const UILowerString SelectionAllowedMultiRow;
		static const UILowerString SelectionMultipleOnDrag;
		static const UILowerString CellPadding;
		static const UILowerString CellHeight;
		static const UILowerString ColumnSizeDataSource;
		static const UILowerString DrawGridLines;
		static const UILowerString GridColorHorizontal;
		static const UILowerString GridColorVertical;
		static const UILowerString GridEdgeVertical;
		static const UILowerString DefaultTextStyle;
		static const UILowerString DefaultTextColor;
		static const UILowerString SelectionColorBackground;
		static const UILowerString SelectionColorRect;
		static const UILowerString SelectionTextColor;
		static const UILowerString ScrollWheelSelection;
		static const UILowerString LogicalRowIndex;
		static const UILowerString DoubleClickOkayButton;
		// selection fg, bg colors
	};

	class ColumnSizeInfo
	{
	public:

		class PropertyName
		{
		public:
			static const UILowerString Constant;
			static const UILowerString Proportion;
			static const UILowerString Width;
		};

		bool  constant;
		float proportion;
		long  width;

		ColumnSizeInfo () : constant (false), proportion (0.0f), width (0) {}

		void updateFromData (const UIData & data);
		void updateData     (UIData & data) const;
	};

	typedef ui_stdvector<ColumnSizeInfo>::fwd ColumnSizeInfoVector;

	virtual bool                IsA                           (const UITypeID Type) const;
	virtual const char         *GetTypeName                   () const        { return TypeName; }
	virtual UIBaseObject       *Clone                         () const              { return new UITable; }

	virtual void                GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void                GetPropertyNames              (UIPropertyNameVector &, bool forCopy ) const;
	virtual void                GetLinkPropertyNames (UIPropertyNameVector &) const;

	virtual bool                SetProperty                   (const UILowerString & Name, const UIString &Value );
	virtual bool                GetProperty                   (const UILowerString & Name, UIString &Value ) const;

	virtual UIStyle            *GetStyle                      () const { return 0; }
	virtual void                Render                        (UICanvas &) const;

	virtual void                Notify                        (UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode );

	virtual void                SetSize                       (const UISize &);
	virtual bool                ProcessMessage                (const UIMessage &);

	virtual bool                OnMessage                     (UIWidget * context, const UIMessage & msg);

	void                        SetTableModel                 (UITableModel * model);
	UITableModel *              GetTableModel                 ();

	void                        SetGridColorHorizontal        (const UIColor & color);
	void                        SetGridColorVertical          (const UIColor & color);

	const UIColor &             GetGridColorHorizontal        () const;
	const UIColor &             GetGridColorVertical          () const;

	void                        SetCellPadding                (const UISize &);
	void                        SetCellHeight                 (long height);

	const UISize &              GetCellPadding                () const;
	long                        GetCellHeight                 () const;

	virtual void                Link                          ();

	bool                        IsRowSelected                 (long row) const;
	long                        GetLastSelectedRow            () const;
	const LongVector &          GetSelectedRows               () const;

	const UIColor &             GetSelectionColorBackground   () const;
	const UIColor &             GetSelectionColorRect         () const;

	bool                        GetCellFromPoint              (const UIPoint & pt, long * row, long * col) const;

	void                        SelectRow                     (long row);
	void                        AddRowSelection               (long row);
	void                        RemoveRowSelection            (long row);

	bool                        GetColumnWidth                (int col, long & width) const;
	bool                        GetTotalColumnExtents         (int col, long & x, long & width) const;

	bool                        ResizeColumns                 (int rightColumn, long dx);

	virtual void                GetScrollSizes                (UISize &PageSize, UISize &LineSize) const;
	void                        ScrollToRow                   (int targetRow);

	void                        GetColumnWidthProportions     (FloatVector & fv) const;
	bool                        RestoreColumnWidthProportions (const FloatVector & fv);

	UIWidget *                  GetCustomDragWidget           (const UIPoint & point, UIPoint & offset);

	void                        SetColumnSizeDataSource       (UIDataSource * ds);

	const ColumnSizeInfoVector & GetColumnSizeInfo            () const;
	void                         SetColumnSizeInfo            (const ColumnSizeInfoVector & csiv);

	long                         GetTotalColumnAvailableWidth () const;
	int                          GetRowCount                  ();

	void                         SetIgnoreDataChanges         (bool b);
	void                         SetIgnoreSortingChanges      (bool b);

	const Unicode::String &      GetLocalTooltip              (const UIPoint & pt) const;

private:

	void                        refreshCellData               ();
	void                        updateCellCounts              ();
	void                        updateExtent                  ();
	void                        updateColumnWidths            ();

	void                        addRangeSelection             (long one, long two);
	void                        removeRangeSelection          (long one, long two);

	bool                        GetTotalRowExtents            (int row, long & top, long & bottom);
	int                         GetLogicalDataRowIndex        (int row);

	bool                        MoveStuff                     (long targetRow, bool changesSelection, bool shifted);

	int                         updateColumnSizeInfo          ();
	void                        updateColumnSizeDataSource    ();

	void                        handleTextFlags               (int flags, UIText * text) const;

	UITable (const UITable &);
	UITable & operator= (const UITable &);

	UITableModel *              mTableModel;

	UIColor                     mGridColorHorizontal;
	UIColor                     mGridColorVertical;

	long                        mCellHeight;
	UISize                      mCellPadding;

private:

	ColumnSizeInfoVector *      mColumnSizes;

	long                        mCachedColumnCount;
	long                        mCachedRowCount;

	UIText  *                   mDefaultRendererText;
	UIImage *                   mDefaultRendererImage;

	LongVector *                mSelectedRows;
	LongVector *                mSelectedRowsInternal;

	bool                        mSelectionAllowedRow;
	bool                        mSelectionAllowedRowMultiple;

	UIColor                     mSelectionColorBackground;
	UIColor                     mSelectionColorRect;
	UIColor                     mSelectionTextColor;
	UIColor                     mDefaultTextColor;

	bool                        mMouseDown;
	UIPoint                     mMouseDownCell;
	UIPoint                     mMouseDragLastCell;
	UIPoint                     mMouseMoveLastPoint;

	bool                        mSelectionChangeInProgress;

	UIPoint                     mTableExtent;

	bool                        mSelectionMultipleOnDrag;

	UIDataSource *              mColumnSizeDataSource;

	bool                        mUpdatingColumnWidths;
	bool                        mScrollWheelSelection;

	bool                        mIgnoreDataChanges;

	bool                        mGridEdgeVertical;

	bool                        mIgnoreSortingChanges;

	bool                        mDrawGridLines;
};

//----------------------------------------------------------------------

inline UITableModel * UITable::GetTableModel ()
{
	return mTableModel;
}

//-----------------------------------------------------------------

inline const UIColor & UITable::GetGridColorHorizontal () const
{
	return mGridColorHorizontal;
}

//-----------------------------------------------------------------

inline const UIColor & UITable::GetGridColorVertical   () const
{
	return mGridColorVertical;
}

//-----------------------------------------------------------------

inline const UISize & UITable::GetCellPadding () const
{
	return mCellPadding;
}

//-----------------------------------------------------------------

inline long UITable::GetCellHeight () const
{
	return mCellHeight;
}

//----------------------------------------------------------------------

inline const UIColor & UITable::GetSelectionColorBackground () const
{
	return mSelectionColorBackground;
}

//----------------------------------------------------------------------

inline const UIColor & UITable::GetSelectionColorRect () const
{
	return mSelectionColorRect;
}

//----------------------------------------------------------------------

inline const UITable::LongVector & UITable::GetSelectedRows () const
{
	return *mSelectedRows;
}

//----------------------------------------------------------------------

inline const UITable::ColumnSizeInfoVector & UITable::GetColumnSizeInfo            () const
{
	return *mColumnSizes;
}

//======================================================================

#endif
