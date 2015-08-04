//======================================================================
//
// UITableModel.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UITableModel_H
#define INCLUDED_UITableModel_H

//======================================================================

#include "UIBaseObject.h"
#include "UITableFlags.def"
#include "UITableTypes.def"
#include "UINotification.h"

class UIData;
class UIImageStyle;
class UIWidget;

//----------------------------------------------------------------------

class UITableModel :
public UIBaseObject,
public UINotificationServer
{
public:
	static const char * const TypeName;

	class DataProperties
	{
	public:
		static const UILowerString  WidgetValue;
	};

	UITableModel ();
	~UITableModel () = 0;

	virtual bool                  IsA                    (const UITypeID Type) const;

	virtual const char *          GetTypeName            () const        { return TypeName; }

	virtual bool                  GetValueAtText         (int row, int col, UIString     & value)  const;
	virtual bool                  GetValueAtImage        (int row, int col, UIImageStyle *& value) const;
	virtual bool                  GetValueAtBool         (int row, int col, bool & value)          const;
	virtual bool                  GetValueAtWidget       (int row, int col, UIWidget *& value)     const;
	virtual bool                  GetValueAtInteger      (int row, int col, int & value)     const;
	virtual bool                  GetValueAtFloat        (int row, int col, float & value)     const;

	virtual bool                  SetValueAtText         (int row, int col, const UIString     & value);
	virtual bool                  SetValueAtImage        (int row, int col, const UIImageStyle * value);
	virtual bool                  SetValueAtBool         (int row, int col, bool value);
	virtual bool                  SetValueAtWidget       (int row, int col, const UIWidget * value);
	virtual bool                  SetValueAtInteger      (int row, int col, int value);
	virtual bool                  SetValueAtFloat        (int row, int col, float value);

	virtual bool                  GetSortKeyAtString     (int row, int col, UITableTypes::CellType type, UIString & strValue)  const;
	virtual bool                  GetSortKeyAtInteger    (int row, int col, UITableTypes::CellType type, int & integerValue)  const;

	virtual bool                  GetLocalizedColumnName    (int col, UIString & str) const = 0;
	virtual bool                  GetLocalizedColumnTooltip (int col, UIString & str) const;

	virtual bool                  GetTooltipAt              (int row, int col, UIString & tooltip) const;
	virtual bool                  SetTooltipAt              (int row, int col, const UIString & tooltip);

	virtual UIWidget *            GetDragWidgetAt        (int row, int col);

	        int                   GetRowCount            () const;

	int                           GetColumnCount         () const;
	UITableTypes::CellType        GetColumnCellType      (int col) const;
	virtual int                   GetColumnCellFlags     (int col) const;

	void                          fireDataChanged        ();
	void                          fireSortingChanged     ();
	void                          fireRowAdded           ();
	void                          fireRowRemoved         ();
	void                          fireColumnsChanged     ();

	void                          restoreSortState       ();

	enum SortDirection
	{
		SD_up,
		SD_down,
		SD_reverse
	};

	void                          sortOnColumn           (int col, SortDirection sd);

	int                           GetLogicalDataRowIndex (int visualRow) const;
	int                           GetVisualDataRowIndex  (int logicalRow) const;

	int                           FindInColumnText       (const Unicode::String & text, int col);

	bool                          GetSortCaseSensitive   () const;
	void                          SetSortCaseSensitive   (bool b);

	void                          clearSortingHistory    ();
	bool                          getSortOnColumnState(int & column, SortDirection & direction) const;

	typedef ui_stdvector<int>::fwd SortStateVector;

	const SortStateVector &       getSortStateVector     () const;
	void                          setSortStateVector     (const SortStateVector & ssv);

protected:

	bool                           verifyValueRequest    (int row, int col, UITableTypes::CellType type);

	virtual void                   performSortOnColumn   (int col, SortDirection sd);

	virtual UITableTypes::CellType FindColumnCellType    (int col) const = 0;
	virtual int                    FindColumnCellFlags   (int col) const;
	virtual int                    FindColumnCount       () const = 0;
	virtual int                    FindRowCount          () const = 0;

private:

	void                          cacheColumnCellTypes ();

	void                          resetSortStateVector ();

	UITableModel             (const UITableModel &);
	UITableModel & operator= (const UITableModel &);

	typedef std::pair <int, SortDirection>            ColumnSortState;
	typedef ui_stdvector<UITableTypes::CellType>::fwd CellTypeVector;
	typedef ui_stdvector<int>::fwd                    CellFlagVector;
	typedef ui_stdlist<ColumnSortState>::fwd          SortStateList;

	CellTypeVector *                    mColumnCellTypes;
	CellFlagVector *                    mColumnCellFlags;

	SortStateList *                     mSortStateList;

	bool                                mRestoringSortState;

	SortStateVector *                   mSortStateVector;

	bool                                mSortCaseSensitive;
};

//----------------------------------------------------------------------

inline bool UITableModel::GetSortCaseSensitive   () const
{
	return mSortCaseSensitive;
}

//----------------------------------------------------------------------

inline const UITableModel::SortStateVector & UITableModel::getSortStateVector() const
{
	return *mSortStateVector;
}

//======================================================================

#endif
