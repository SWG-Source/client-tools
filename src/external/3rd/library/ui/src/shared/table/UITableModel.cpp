//======================================================================
//
// UITableModel.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "_precompile.h"
#include "UITableModel.h"

#include "UIData.h"
#include "UIDataSource.h"
#include "UIWidget.h"
#include "UIUtils.h"

#if WIN32
#pragma warning (disable:4505)
#endif

#include <algorithm>
#include <cassert>
#include <list>
#include <vector>

//======================================================================

const char * const UITableModel::TypeName                    = "TableModel";

const UILowerString  UITableModel::DataProperties::WidgetValue = UILowerString ("WidgetValue");


namespace UITableModelNamespace
{
	namespace Properties
	{
		namespace ColumnDataSource
		{
			const UILowerString flags        = UILowerString ("Flags");
		}
	}
}

using namespace UITableModelNamespace;

//----------------------------------------------------------------------

UITableModel::UITableModel () :
UIBaseObject        (),
mColumnCellTypes    (new CellTypeVector),
mColumnCellFlags    (new CellFlagVector),
mSortStateList      (new SortStateList),
mRestoringSortState (false),
mSortStateVector    (new SortStateVector),
mSortCaseSensitive  (false)
{
}

//----------------------------------------------------------------------

UITableModel::~UITableModel ()
{
	delete mSortStateList;
	mSortStateList = 0;

	delete mSortStateVector;
	mSortStateVector = 0;

	delete mColumnCellTypes;
	mColumnCellTypes = 0;

	delete mColumnCellFlags;
	mColumnCellFlags = 0;
}

//-----------------------------------------------------------------

bool UITableModel::IsA( const UITypeID Type ) const
{
	return (Type == TUITableModel) || UIBaseObject::IsA (Type);
}

//----------------------------------------------------------------------

void UITableModel::cacheColumnCellTypes ()
{
	mColumnCellTypes->clear ();
	mColumnCellFlags->clear ();

	int columnCount = FindColumnCount ();

	mColumnCellTypes->reserve (columnCount);
	mColumnCellFlags->reserve (columnCount);

	for (int i = 0; i < columnCount; ++i)
	{
		const UITableTypes::CellType type = FindColumnCellType (i);
		mColumnCellTypes->push_back (type);

		int flags = FindColumnCellFlags (i);
		mColumnCellFlags->push_back (flags);
	}
}

//-----------------------------------------------------------------

bool UITableModel::GetValueAtText   (int , int , UIString     & ) const
{
	return false;
}

//-----------------------------------------------------------------

bool UITableModel::GetValueAtImage  (int , int , UIImageStyle *& ) const
{
	return false;
}

//-----------------------------------------------------------------

bool UITableModel::GetValueAtBool   (int , int , bool & ) const
{
	return false;
}

//-----------------------------------------------------------------

bool UITableModel::GetValueAtWidget (int , int , UIWidget *& ) const
{
	return false;
}

//----------------------------------------------------------------------

bool UITableModel::GetValueAtInteger(int , int , int & )     const
{
	return false;
}

//----------------------------------------------------------------------

bool UITableModel::GetValueAtFloat  (int , int , float & )     const
{
	return false;
}

//----------------------------------------------------------------------

bool UITableModel::SetValueAtText         (int, int, const UIString     &)
{
	return false;
}

//----------------------------------------------------------------------

bool UITableModel::SetValueAtImage        (int, int, const UIImageStyle *)
{
	return false;
}

//----------------------------------------------------------------------

bool UITableModel::SetValueAtBool         (int, int, bool)
{
	return false;
}

//----------------------------------------------------------------------

bool UITableModel::SetValueAtWidget       (int, int, const UIWidget *)
{
	return false;
}

//----------------------------------------------------------------------

bool UITableModel::SetValueAtInteger      (int, int, int)
{
	return false;
}

//----------------------------------------------------------------------

bool UITableModel::SetValueAtFloat        (int, int, float)
{
	return false;
}

//-----------------------------------------------------------------

bool UITableModel::verifyValueRequest (int row, int col, UITableTypes::CellType type)
{
	if (col < 0 || row < 0 || col >= GetColumnCount () || row >= GetRowCount ())
		return false;

	if ((*mColumnCellTypes) [col] != type)
		return false;

	return true;
}

//-----------------------------------------------------------------

int UITableModel::GetColumnCount () const
{
	return mColumnCellTypes->size ();
}

//----------------------------------------------------------------------

void UITableModel::fireDataChanged ()
{
	cacheColumnCellTypes ();
	resetSortStateVector ();
	restoreSortState ();
	SendNotification (UINotification::TableDataChanged, this);
}

//----------------------------------------------------------------------

void UITableModel::fireSortingChanged ()
{
	SendNotification (UINotification::TableSortingChanged, this);
}

//-----------------------------------------------------------------

void UITableModel::fireRowAdded ()
{
	SendNotification (UINotification::TableDataChanged, this);
}

//-----------------------------------------------------------------

void UITableModel::fireRowRemoved ()
{
	SendNotification (UINotification::TableDataChanged, this);
}

//-----------------------------------------------------------------

void UITableModel::fireColumnsChanged ()
{
	cacheColumnCellTypes ();
	SendNotification (UINotification::TableColumnsChanged, this);
}

//----------------------------------------------------------------------

UITableTypes::CellType  UITableModel::GetColumnCellType (int col) const
{
	if (col < 0 || col >= static_cast<int>(mColumnCellTypes->size ()))
		return UITableTypes::CT_none;

	return (*mColumnCellTypes) [col];
}

//----------------------------------------------------------------------

int UITableModel::GetColumnCellFlags (int col) const
{
	static_cast<void>(col);

	if (col < 0 || col >= static_cast<int>(mColumnCellFlags->size ()))
		return 0;

	return (*mColumnCellFlags) [col];

	return 0;
}

//----------------------------------------------------------------------

void UITableModel::clearSortingHistory    ()
{
	mSortStateList->clear ();
}

//----------------------------------------------------------------------

bool UITableModel::getSortOnColumnState(int & column, SortDirection & direction) const
{
	if (!mSortStateList->empty())
	{
		SortStateList::const_reverse_iterator ii = const_cast<SortStateList const *>(mSortStateList)->rbegin();
		column = ii->first;
		direction = ii->second;
		return true;
	}

	column = 0;
	direction = SD_down;
	return false;
}

//----------------------------------------------------------------------

void UITableModel::restoreSortState ()
{
	mRestoringSortState = true;

	if (mSortStateList->empty ())
		resetSortStateVector ();
	else
	{
		for (SortStateList::iterator it = mSortStateList->begin (); it != mSortStateList->end ();)
		{
			//-- column is no longer valid
			if ((*it).first >= static_cast<int>(mColumnCellTypes->size ()))
			{
				it = mSortStateList->erase (it);
			}
			else
			{
				sortOnColumn ((*it).first, (*it).second);
				++it;
			}
		}
	}

	mRestoringSortState = false;
}

//----------------------------------------------------------------------

/**
* Subclass implementations need to implement performSortOnColumn
*/
void UITableModel::sortOnColumn (int col, SortDirection sd)
{
	if (col < 0 || col >= static_cast<int>(mColumnCellTypes->size ()))
	{
		assert (false);
		return;
	}

	if (!mRestoringSortState)
	{
		SortDirection currentSd = SD_reverse;

		for (SortStateList::iterator it = mSortStateList->begin (); it != mSortStateList->end (); ++it)
		{
			if ((*it).first == col)
			{
				currentSd = (*it).second;
				mSortStateList->erase (it);
				break;
			}
		}

		if (sd == SD_reverse)
		{
			if (currentSd == SD_reverse || currentSd == SD_down)
				sd = SD_up;
			else
				sd = SD_down;
		}

		mSortStateList->push_back (ColumnSortState (col, sd));
	}

	performSortOnColumn (col, sd);
}

//----------------------------------------------------------------------

bool UITableModel::GetSortKeyAtString (int row, int col, UITableTypes::CellType type, UIString & strValue)  const
{
	switch (type)
	{
	case UITableTypes::CT_text:
		if (!GetValueAtText (row, col, strValue))
			strValue.clear ();
		return true;
	default:
		assert (false);
		break;
	}

	return false;
}

//----------------------------------------------------------------------

bool UITableModel::GetSortKeyAtInteger (int row, int col, UITableTypes::CellType type, int & integerValue)  const
{
	switch (type)
	{
	case UITableTypes::CT_image:
		{
			UIImageStyle * style = 0;
			if (GetValueAtImage (row, col, style))
				integerValue = reinterpret_cast<int>(style);
			else
				integerValue = 0;
			return true;
		}
		break;
	case UITableTypes::CT_bool:
		{
			bool b;
			if (GetValueAtBool (row, col, b))
				integerValue = b ? 1 : 0;
			else
				integerValue = -1;

			return true;
		}
		break;
	case UITableTypes::CT_widget:
		{
			UIWidget * widget = 0;
			if (GetValueAtWidget (row, col, widget) && widget)
			{
				if (!widget->GetPropertyInteger (DataProperties::WidgetValue, integerValue))
					integerValue = reinterpret_cast<int>(widget);
			}
			else
				integerValue = -1;

			return true;
		}
	case UITableTypes::CT_integer:
	case UITableTypes::CT_percent:
	case UITableTypes::CT_delimitedInteger:
		{
			return GetValueAtInteger (row, col, integerValue);
		}
	case UITableTypes::CT_float:
		{
			float f;
			if (GetValueAtFloat (row, col, f))
			{
				//@todo floats will be sorted at a resolution on ten thousandths.  this may be a problem one day
				integerValue = static_cast<int>(f * 10000.0f);
				return true;
			}

			return false;
		}

		break;
	default:
		assert (false);
		break;
	}

	return false;
}

//----------------------------------------------------------------------

namespace
{
	struct ComparePairFirstUp
	{
		template <typename T> bool operator() (const std::pair<T,int> & first, const std::pair<T,int> & second)
		{
			return first.first < second.first;
		}
	};

	struct ComparePairFirstDown
	{
		template <typename T> bool operator() (const std::pair<T,int> & first, const std::pair<T,int> & second)
		{
			return second.first < first.first;
		}
	};
}

//----------------------------------------------------------------------

/**
* Subclass implementations need to implement this
*/

void UITableModel::performSortOnColumn (int col, SortDirection sd)
{
	const int rowCount = GetRowCount ();

	const UITableTypes::CellType type = GetColumnCellType (col);

	switch (type)
	{
	case UITableTypes::CT_text:
		{
			typedef std::pair<UIString, int> SortingVectorElement;
			typedef std::vector<SortingVectorElement> SortingVector;

			SortingVector sv;
			sv.reserve (rowCount);

			for (int i = 0; i < rowCount; ++i)
			{
				UIString str;
				GetSortKeyAtString (i, col, type, str);

				//-- lowerize string
				if (!mSortCaseSensitive)
					str = Unicode::toLower (str);

				//-- just push back an empty string even if call failed
				sv.push_back (std::make_pair(str, (*mSortStateVector)[i]));
			}

			if (sd == SD_up)
				std::stable_sort (sv.begin (), sv.end (), ComparePairFirstUp ());
			else
				std::stable_sort (sv.begin (), sv.end (), ComparePairFirstDown ());

			mSortStateVector->clear ();
			mSortStateVector->reserve (rowCount);

			for (SortingVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
				mSortStateVector->push_back ((*it).second);
		}
		break;
	case UITableTypes::CT_image:
	case UITableTypes::CT_bool:
	case UITableTypes::CT_widget:
	case UITableTypes::CT_integer:
	case UITableTypes::CT_float:
	case UITableTypes::CT_percent:
	case UITableTypes::CT_delimitedInteger:
		{
			typedef std::pair<int, int> SortingVectorElement;
			typedef std::vector<SortingVectorElement> SortingVector;

			SortingVector sv;
			sv.reserve (rowCount);

			for (int i = 0; i < rowCount; ++i)
			{
				int value = 0;
				GetSortKeyAtInteger (i, col, type, value);

				//-- just push back an empty string even if call failed
				sv.push_back (std::make_pair(value, (*mSortStateVector)[i]));
			}

			if (sd == SD_up)
				std::stable_sort (sv.begin (), sv.end (), ComparePairFirstUp ());
			else
				std::stable_sort (sv.begin (), sv.end (), ComparePairFirstDown ());

			mSortStateVector->clear ();
			mSortStateVector->reserve (rowCount);

			for (SortingVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
				mSortStateVector->push_back ((*it).second);
		}
		break;
	default:
		assert(false);
		break;
	}

	fireSortingChanged ();
}

//----------------------------------------------------------------------

void UITableModel::resetSortStateVector ()
{
	mSortStateVector->clear ();
	const int rowCount = GetRowCount ();

	mSortStateVector->reserve (rowCount);
	for (int i = 0; i < rowCount; ++i)
		mSortStateVector->push_back (i);
}

//----------------------------------------------------------------------

int UITableModel::GetLogicalDataRowIndex (int row) const
{
	if (row < 0 || row >= static_cast<int>(mSortStateVector->size ()))
		return -1;

	return (*mSortStateVector) [row];
}

//-----------------------------------------------------------------

/**
*
*/
int UITableModel::GetVisualDataRowIndex (int logicalRow) const
{
	int i = 0;
	for (SortStateVector::const_iterator it = mSortStateVector->begin (); it != mSortStateVector->end (); ++it, ++i)
	{
		if ((*it) == logicalRow)
			return i;
	}

	return -1;
}

//----------------------------------------------------------------------

int UITableModel::GetRowCount () const
{
	return FindRowCount ();
}

//----------------------------------------------------------------------

int UITableModel::FindInColumnText       (const Unicode::String & text, int col)
{

	if (col < 0 || col >= GetColumnCount ())
		return -1;

	if (GetColumnCellType (col) != UITableTypes::CT_text)
		return -1;

	Unicode::String str;

	const int rowCount = GetRowCount ();
	for (int i = 0; i < rowCount; ++i)
	{
		if (GetValueAtText (i, col, str))
		{
			if (str == text)
				return i;
		}
	}

	return -1;
}

//----------------------------------------------------------------------

UIWidget * UITableModel::GetDragWidgetAt        (int , int )
{
	return 0;
}

//----------------------------------------------------------------------

bool UITableModel::GetLocalizedColumnTooltip (int col, UIString & str) const
{
	return GetLocalizedColumnName (col, str);
}

//----------------------------------------------------------------------

void UITableModel::SetSortCaseSensitive   (bool b)
{
	mSortCaseSensitive = b;
}

//----------------------------------------------------------------------

bool UITableModel::GetTooltipAt              (int row, int col, UIString & tooltip) const
{
	const UITableTypes::CellType ct = GetColumnCellType (col);

	switch (ct)
	{
	case UITableTypes::CT_text:
		return GetValueAtText (row, col, tooltip);
	case UITableTypes::CT_bool:
		{
			bool b;
			if (GetValueAtBool (row, col, b))
				return UIUtils::FormatBoolean (tooltip, b);
		}
		return false;

		//----------------------------------------------------------------------

	case UITableTypes::CT_integer:
	case UITableTypes::CT_percent:
	case UITableTypes::CT_delimitedInteger:
		{
			int i;
			if (GetValueAtInteger (row, col, i))
			{
				if (UIUtils::FormatLong (tooltip, i))
				{
					if (ct == UITableTypes::CT_percent)
						tooltip.append (1, '%');

					if (ct == UITableTypes::CT_delimitedInteger)
						tooltip = UIUtils::FormatDelimitedInteger(tooltip);

					return true;
				}
			}
		}
		return false;

		//----------------------------------------------------------------------

	case UITableTypes::CT_float:
		{
			float f;
			if (GetValueAtFloat (row, col, f))
				return UIUtils::FormatFloat (tooltip, f);
		}
		return false;

	case UITableTypes::CT_widget:
		{
			UIWidget * widget = 0;
			if (GetValueAtWidget (row, col, widget) && widget)
			{
				tooltip = widget->GetLocalTooltip (UIPoint::zero);
				return true;
			}
		}

	default:
		return false;
	}
}

//----------------------------------------------------------------------

bool UITableModel::SetTooltipAt              (int row, int col, const UIString & tooltip)
{
	UI_UNREF (row);
	UI_UNREF (col);
	UI_UNREF (tooltip);

	return false;
}

//----------------------------------------------------------------------

void UITableModel::setSortStateVector     (const SortStateVector & ssv)
{
	if (ssv.empty ())
		resetSortStateVector ();
	else if (mSortStateVector->size () == ssv.size ())
	{
		*mSortStateVector = ssv;
		fireSortingChanged ();
	}
}
//----------------------------------------------------------------------

int UITableModel::FindColumnCellFlags (int col) const
{
	static_cast<void>(col);
	return 0;
}

//======================================================================
