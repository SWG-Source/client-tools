//======================================================================
//
// UITable.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "_precompile.h"
#include "UITable.h"

#include "UICanvas.h"
#include "UIClock.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPalette.h"
#include "UITableModel.h"
#include "UIText.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"

#include <algorithm>
#include <cassert>
#include <list>
#include <math.h>
#include <numeric>
#include <vector>

//======================================================================

const char * const UITable::TypeName = "Table";

const UILowerString UITable::PropertyName::CellHeight                 = UILowerString ("CellHeight");
const UILowerString UITable::PropertyName::CellPadding                = UILowerString ("CellPadding");
const UILowerString UITable::PropertyName::ColumnResizePolicy         = UILowerString ("ColumnResizePolicy");
const UILowerString UITable::PropertyName::ColumnSizeDataSource       = UILowerString ("ColumnSizeDataSource");
const UILowerString UITable::PropertyName::DefaultTextColor           = UILowerString ("DefaultTextColor");
const UILowerString UITable::PropertyName::DefaultTextStyle           = UILowerString ("DefaultTextStyle");
const UILowerString UITable::PropertyName::DrawGridLines              = UILowerString ("DrawGridLines");
const UILowerString UITable::PropertyName::GridColorHorizontal        = UILowerString ("GridColorHorizontal");
const UILowerString UITable::PropertyName::GridColorVertical          = UILowerString ("GridColorVertical");
const UILowerString UITable::PropertyName::GridEdgeVertical           = UILowerString ("GridEdgeVertical");

const UILowerString UITable::PropertyName::ScrollWheelSelection       = UILowerString ("ScrollWheelSelection");
const UILowerString UITable::PropertyName::SelectedRow                = UILowerString ("SelectedRow");
const UILowerString UITable::PropertyName::SelectedRowCount           = UILowerString ("SelectedRowCount");
const UILowerString UITable::PropertyName::SelectionAllowedMultiRow   = UILowerString ("SelectionAllowedMultiRow");
const UILowerString UITable::PropertyName::SelectionAllowedRow        = UILowerString ("SelectionAllowedRow");
const UILowerString UITable::PropertyName::SelectionColorBackground   = UILowerString ("SelectionColorBackground");
const UILowerString UITable::PropertyName::SelectionColorRect         = UILowerString ("SelectionColorRect");
const UILowerString UITable::PropertyName::SelectionMultipleOnDrag    = UILowerString ("SelectionMultipleOnDrag");
const UILowerString UITable::PropertyName::SelectionTextColor         = UILowerString ("SelectionTextColor");
const UILowerString UITable::PropertyName::TableModel                 = UILowerString ("TableModel");
const UILowerString UITable::PropertyName::LogicalRowIndex            = UILowerString ("LogicalRowIndex");
const UILowerString UITable::PropertyName::DoubleClickOkayButton      = UILowerString ("DoubleClickOkayButton");

//----------------------------------------------------------------------

const UILowerString UITable::ColumnSizeInfo::PropertyName::Width       = UILowerString ("Width");
const UILowerString UITable::ColumnSizeInfo::PropertyName::Constant    = UILowerString ("Constant");
const UILowerString UITable::ColumnSizeInfo::PropertyName::Proportion  = UILowerString ("Proportion");

//======================================================================================
#define _TYPENAME UITable

namespace UITableNamespace
{
	const long MIN_COLUMN_SIZE = 24L;

	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(CellHeight, "", T_int),
		_DESCRIPTOR(CellPadding, "", T_point),
		_DESCRIPTOR(ColumnSizeDataSource, "", T_object),
		_DESCRIPTOR(SelectedRow, "", T_int),
		_DESCRIPTOR(TableModel, "", T_object),
	_GROUPEND(Basic, 2, 0);
	//================================================================

	//================================================================
	// Text category
	_GROUPBEGIN(Text)
		_DESCRIPTOR(DefaultTextStyle, "", T_object),
	_GROUPEND(Text, 2, 1);
	//================================================================

	//================================================================
	// Behavior category
	_GROUPBEGIN(Behavior)
		_DESCRIPTOR(SelectionAllowedMultiRow, "", T_bool),
		_DESCRIPTOR(SelectionAllowedRow, "", T_bool),
		_DESCRIPTOR(SelectionMultipleOnDrag, "", T_bool),
		_DESCRIPTOR(ScrollWheelSelection, "", T_bool),
	_GROUPEND(Behavior, 2, 2);
	//================================================================

	//================================================================
	// Appearance category
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(DefaultTextColor, "", T_color),
		_DESCRIPTOR(DrawGridLines, "", T_bool),
		_DESCRIPTOR(GridColorHorizontal, "", T_color),
		_DESCRIPTOR(GridColorVertical, "", T_color),
		_DESCRIPTOR(SelectionColorBackground, "", T_color),
		_DESCRIPTOR(SelectionColorRect, "", T_color),
		_DESCRIPTOR(SelectionTextColor, "", T_color),
		_DESCRIPTOR(GridEdgeVertical, "", T_bool),
	_GROUPEND(Appearance, 2, 3);
	//================================================================
}
using namespace UITableNamespace;
//======================================================================================

UITable::UITable () :
UIWidget                     (),
UINotification               (),
UINotificationServer         (),
UIEventCallback              (),
mTableModel                  (0),
mGridColorHorizontal         (UIColor::black),
mGridColorVertical           (UIColor::black),
mCellHeight                  (16),
mCellPadding                 (1,1),
mColumnSizes                 (new ColumnSizeInfoVector),
mCachedColumnCount           (0),
mCachedRowCount              (0),
mDefaultRendererText         (new UIText),
mDefaultRendererImage        (new UIImage),
mSelectedRows                (new LongVector),
mSelectedRowsInternal        (new LongVector),
mSelectionAllowedRow         (true),
mSelectionAllowedRowMultiple (true),
mSelectionColorBackground    (UIColor::cyan),
mSelectionColorRect          (UIColor::red),
mSelectionTextColor          (UIColor (0x88, 0x00, 0x00)),
mDefaultTextColor            (UIColor::white),
mMouseDown                   (false),
mMouseDownCell               (),
mMouseDragLastCell           (),
mMouseMoveLastPoint          (),
mSelectionChangeInProgress   (true),
mTableExtent                 (),
mSelectionMultipleOnDrag     (false),
mColumnSizeDataSource        (0),
mUpdatingColumnWidths        (false),
mScrollWheelSelection        (false),
mIgnoreDataChanges           (false),
mGridEdgeVertical            (false),
mIgnoreSortingChanges        (false),
mDrawGridLines               (true)
{
	SetBackgroundColor (UIColor (200, 200, 200));
	SetBackgroundOpacity (1.0f);

	mDefaultRendererText->Attach (this);
	mDefaultRendererImage->Attach (this);

	mDefaultRendererText->SetMargin       (UIRect (2, 1, 2, 1));
	mDefaultRendererText->SetMaxLines     (1);
	mDefaultRendererText->SetPreLocalized (true);

	AddCallback (this);
}

//----------------------------------------------------------------------

UITable::~UITable ()
{
	RemoveCallback (this);

	SetColumnSizeDataSource (0);

	mDefaultRendererText->Detach (this);
	mDefaultRendererImage->Detach (this);
	SetTableModel (0);

	delete mColumnSizes;
	mColumnSizes = 0;

	delete mSelectedRows;
	mSelectedRows = 0;

	delete mSelectedRowsInternal;
	mSelectedRowsInternal = 0;
}

//-----------------------------------------------------------------

bool UITable::IsA( const UITypeID Type ) const
{
	return (Type == TUITable) || UIWidget::IsA (Type);
}

//----------------------------------------------------------------------

void UITable::GetLinkPropertyNames( UIPropertyNameVector & in ) const
{
	in.push_back ( PropertyName::ColumnSizeDataSource);
	in.push_back ( PropertyName::DefaultTextStyle);
	in.push_back ( PropertyName::TableModel);

	UIWidget::GetLinkPropertyNames (in);
}

//----------------------------------------------------------------------

void UITable::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidget::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Text, category, o_groups);
	GROUP_TEST_AND_PUSH(Behavior, category, o_groups);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
}

//----------------------------------------------------------------------

void UITable::GetPropertyNames( UIPropertyNameVector & in, bool forCopy ) const
{
	UIPalette::GetPropertyNamesForType (TUITable, in);

	in.push_back ( PropertyName::CellHeight);
	in.push_back ( PropertyName::CellPadding);
	in.push_back ( PropertyName::ColumnResizePolicy);
	in.push_back ( PropertyName::ColumnSizeDataSource);
	in.push_back ( PropertyName::DefaultTextColor);
	in.push_back ( PropertyName::DefaultTextStyle);
	in.push_back ( PropertyName::DrawGridLines);
	in.push_back ( PropertyName::GridColorHorizontal);
	in.push_back ( PropertyName::GridColorVertical);
	in.push_back ( PropertyName::SelectedRow);
	in.push_back ( PropertyName::SelectedRowCount);
	in.push_back ( PropertyName::SelectionAllowedMultiRow);
	in.push_back ( PropertyName::SelectionAllowedRow);
	in.push_back ( PropertyName::SelectionColorBackground);
	in.push_back ( PropertyName::SelectionColorRect);
	in.push_back ( PropertyName::SelectionMultipleOnDrag);
	in.push_back ( PropertyName::SelectionTextColor);
	in.push_back ( PropertyName::TableModel);
	in.push_back (PropertyName::ScrollWheelSelection);
	in.push_back (PropertyName::GridEdgeVertical);

	UIWidget::GetPropertyNames (in, forCopy);
}

//----------------------------------------------------------------------

bool UITable::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if (Name == PropertyName::TableModel    )
	{
		UITableModel * const object = static_cast<UITableModel *>(GetObjectFromPath( Value, TUITableModel ));

		if( object || Value.empty() )
		{
			SetTableModel (object);
			return true;
		}
	}

	else if (Name == PropertyName::ColumnSizeDataSource)
	{
		UIDataSource * const object = static_cast<UIDataSource *>(GetObjectFromPath( Value, TUIDataSource ));

		if( object || Value.empty() )
		{
			SetColumnSizeDataSource (object);
			return true;
		}
	}

	else if (Name == PropertyName::ColumnResizePolicy       )
	{
	}

	else if (Name == PropertyName::SelectedRow              )
	{
		long index = -1;
		if (UIUtils::ParseLong (Value, index))
		{
			SelectRow (index);
			return true;
		}

		return false;
	}

	else if (Name == PropertyName::SelectedRowCount         )
	{
		return false;
	}

	else if (Name == PropertyName::SelectionAllowedRow      )
	{
		return UIUtils::ParseBoolean (Value, mSelectionAllowedRow);
	}

	else if (Name == PropertyName::SelectionAllowedMultiRow )
	{
		return UIUtils::ParseBoolean (Value, mSelectionAllowedRowMultiple);
	}

	else if (Name == PropertyName::CellPadding              )
	{
		UIPoint pt;
		if (UIUtils::ParsePoint (Value, pt))
		{
			SetCellPadding (pt);
			return true;
		}
		return false;
	}

	else if (Name == PropertyName::CellHeight               )
	{
		long height = -1;
		if (UIUtils::ParseLong (Value, height))
		{
			SetCellHeight (height);
			return true;
		}
		return false;
	}

	else if (Name == PropertyName::GridColorHorizontal      )
	{
		return UIUtils::ParseColor (Value, mGridColorHorizontal);
	}

	else if (Name == PropertyName::GridColorVertical        )
	{
		return UIUtils::ParseColor (Value, mGridColorVertical);
	}
	else if (Name == PropertyName::GridEdgeVertical)
	{
		return UIUtils::ParseBoolean (Value, mGridEdgeVertical);
	}
	else if (Name == PropertyName::DefaultTextStyle)
	{
		mDefaultRendererText->SetParent (this);
		mDefaultRendererText->SetProperty (UIText::PropertyName::Style, Value);
		mDefaultRendererText->SetParent (0);
		// fall through
	}
	else if (Name == PropertyName::DrawGridLines)
	{
		return UIUtils::ParseBoolean (Value, mDrawGridLines);
	}
	else if (Name == PropertyName::DefaultTextColor)
	{
		return UIUtils::ParseColor (Value, mDefaultTextColor);
	}
	else if (Name == PropertyName::SelectionTextColor)
	{
		return UIUtils::ParseColor (Value, mSelectionTextColor);
	}
	else if ( Name == PropertyName::SelectionColorBackground )
	{
		return UIUtils::ParseColor (Value, mSelectionColorBackground);
	}
	else if ( Name == PropertyName::SelectionColorRect )
	{
		return UIUtils::ParseColor (Value, mSelectionColorRect);
	}
	else if ( Name == PropertyName::SelectionMultipleOnDrag)
	{
		return UIUtils::ParseBoolean (Value, mSelectionMultipleOnDrag);
	}
	else if (Name == PropertyName::ScrollWheelSelection)
	{
		return UIUtils::ParseBoolean (Value, mScrollWheelSelection);
	}
	else
	{
		UIPalette::SetPropertyForObject (*this, Name, Value);
	}

	return UIWidget::SetProperty (Name, Value);
}

//----------------------------------------------------------------------

bool UITable::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if (Name == PropertyName::TableModel       )
	{
		if( mTableModel )
		{
			GetPathTo( Value, mTableModel );
			return true;
		}
	}

	//----------------------------------------------------------------------

	else if (Name == PropertyName::ColumnSizeDataSource)
	{
		if( mColumnSizeDataSource )
		{
			GetPathTo( Value, mColumnSizeDataSource );
			return true;
		}
	}

	//----------------------------------------------------------------------

	else if (Name == PropertyName::ColumnResizePolicy       )
	{
	}

	else if (Name == PropertyName::SelectedRow              )
	{
		return UIUtils::FormatLong (Value, GetLastSelectedRow ());
	}

	else if (Name == PropertyName::SelectedRowCount         )
	{
		return UIUtils::FormatLong (Value, static_cast<long>(mSelectedRows->size ()));
	}

	else if (Name == PropertyName::SelectionAllowedRow      )
	{
		return UIUtils::FormatBoolean (Value, mSelectionAllowedRow);
	}

	else if (Name == PropertyName::SelectionAllowedMultiRow )
	{
		return UIUtils::FormatBoolean (Value, mSelectionAllowedRowMultiple);
	}

	else if (Name == PropertyName::CellPadding              )
	{
		return UIUtils::FormatPoint (Value, mCellPadding);
	}

	else if (Name == PropertyName::CellHeight               )
	{
		return UIUtils::FormatLong (Value, mCellHeight);
	}

	else if (Name == PropertyName::GridColorHorizontal      )
	{
		return UIUtils::FormatColor (Value, mGridColorHorizontal);
	}

	else if (Name == PropertyName::GridColorVertical        )
	{
		return UIUtils::FormatColor (Value, mGridColorVertical);
	}

	else if (Name == PropertyName::GridEdgeVertical)
	{
		return UIUtils::FormatBoolean (Value, mGridEdgeVertical);
	}
	else if (Name == PropertyName::DefaultTextStyle)
	{
		mDefaultRendererText->SetParent (const_cast<UITable *>(this));
		const bool retval = mDefaultRendererText->GetProperty (UIText::PropertyName::Style, Value);
		mDefaultRendererText->SetParent (0);

		return retval;
	}
	else if (Name == PropertyName::DrawGridLines)
	{
		return UIUtils::FormatBoolean (Value, mDrawGridLines);
	}
	else if (Name == PropertyName::DefaultTextColor)
	{
		return UIUtils::FormatColor (Value, mDefaultTextColor);
	}
	else if (Name == PropertyName::SelectionTextColor)
	{
		return UIUtils::FormatColor (Value, mSelectionTextColor);
	}
	else if ( Name == PropertyName::SelectionColorBackground )
	{
		return UIUtils::FormatColor (Value, mSelectionColorBackground);
	}
	else if ( Name == PropertyName::SelectionColorRect )
	{
		return UIUtils::FormatColor (Value, mSelectionColorRect);
	}
	else if ( Name == PropertyName::SelectionMultipleOnDrag)
	{
		return UIUtils::FormatBoolean (Value, mSelectionMultipleOnDrag);
	}
	else if (Name == PropertyName::ScrollWheelSelection)
	{
		return UIUtils::FormatBoolean (Value, mScrollWheelSelection);
	}

	return UIWidget::GetProperty (Name, Value);
}

//----------------------------------------------------------------------

void UITable::Render( UICanvas & canvas ) const
{
	UIWidget::Render (canvas);

	if (!mTableModel)
		return;

	const UIPoint & scrollLocation = GetScrollLocation ();
	const UIPoint & size = GetSize ();

	const UIColor oldColor = canvas.GetColor ();
	const UISize halfCellPadding (mCellPadding / 2L);

	const long totalCellHeight = mCellHeight + mCellPadding.y;
	const float totalCachedRowCellHeight = static_cast<float>(mCachedRowCount * totalCellHeight);
	int rowStart = 0;

	if (totalCachedRowCellHeight > 0.0f)
	{
		const float scrollPercent = static_cast<float>(scrollLocation.y) / totalCachedRowCellHeight;
		rowStart = static_cast<int>(floorf(static_cast<float>(mCachedRowCount) * scrollPercent));
	}

	int rowEnd = rowStart;
	
	if (totalCellHeight > 0)
	{
		rowEnd = std::min(rowStart + static_cast<long>(ceilf(static_cast<float>(size.y) / static_cast<float>(totalCellHeight))), mCachedRowCount);
	}

	if(mDrawGridLines)
	{
		long x = 0;

		UILine line;

		if (mCellPadding.y > 0)
		{
			line.p1.x = static_cast<float>(scrollLocation.x);
			line.p2.x = line.p1.x + static_cast<float>(size.x);

			bool lastRowSelected = false;
			long y = totalCellHeight * rowStart;

			for (int row = rowStart; row <= rowEnd; ++row, y += totalCellHeight)
			{
				canvas.SetColor (oldColor);

				if (IsRowSelected (row))
				{
					if (y + mCellHeight + mCellPadding.y >= scrollLocation.y && y < scrollLocation.y + size.y)
					{
						canvas.ModifyColor (mSelectionColorBackground);
						canvas.BltFrom (0, UIPoint::zero, UIPoint (scrollLocation.x, y + halfCellPadding.y ), UISize (size.x, mCellHeight));
						canvas.SetColor (oldColor);
					}
				}

				if (y < scrollLocation.y)
					continue;

				if (y >= scrollLocation.y + size.y)
					break;

				line.p1.y = line.p2.y = static_cast<float>(y);

				const bool isRowSelected = IsRowSelected (row);

				if (isRowSelected || lastRowSelected)
					canvas.ModifyColor (mSelectionColorRect);
				else
					canvas.ModifyColor (mGridColorHorizontal);

				lastRowSelected = isRowSelected;
				canvas.RenderLine (line);
			}
		}

		canvas.SetColor (oldColor);

		if (mCellPadding.x > 0)
		{
			line.p1.y = static_cast<float>(scrollLocation.y);
			line.p2.y = std::min (static_cast<float>(mTableExtent.y), line.p1.y + static_cast<float>(size.y));

			canvas.ModifyColor (mGridColorVertical);

			assert (mCachedColumnCount <= static_cast<long>(mColumnSizes->size ()));

			long lastColumnWidth = 0L;

			//- we must draw mCachedColumnCount + 1 columns
			//- x is at the left edge of the vertical padding lines

			for (int col = 0; col <= mCachedColumnCount; ++col, x += lastColumnWidth + mCellPadding.x)
			{
				lastColumnWidth = col < mCachedColumnCount ? (*mColumnSizes) [col].width : 0;

				if (!mGridEdgeVertical && (col == 0 || col == mCachedColumnCount))
					continue;

				if (x < scrollLocation.x || x >= scrollLocation.x + size.x)
					continue;

				line.p1.x = line.p2.x = static_cast<float>(x);
				canvas.RenderLine (line);
			}
		}
	}

	//----------------------------------------------------------------------
	//----------------------------------------------------------------------
	//----------------------------------------------------------------------

	//- cache this

	UIPoint pt;
	pt.x = halfCellPadding.x;
	long lastColumnWidth = 0;

	Unicode::String tmpStr;

	for (int col = 0; col < mCachedColumnCount; ++col, pt.x += lastColumnWidth + mCellPadding.x)
	{
		lastColumnWidth = (*mColumnSizes) [col].width;

		if (pt.x >= scrollLocation.x + mTableExtent.x || pt.x + lastColumnWidth < scrollLocation.x)
			continue;

		pt.y = halfCellPadding.y + (totalCellHeight * rowStart);

		const UITableTypes::CellType columnType = mTableModel->GetColumnCellType (col);
		const int flags                         = mTableModel->GetColumnCellFlags(col);

		if (columnType == UITableTypes::CT_none)
			continue;

		for (int row = rowStart; row < rowEnd; ++row, pt.y += totalCellHeight)
		{
			canvas.SetColor (oldColor);
			/*
			if (IsRowSelected (row))
			{
				canvas.ModifyColor (mSelectionColorBackground);
				canvas.BltFrom (0, UIPoint::zero, pt, UISize (lastColumnWidth, mCellHeight));
				canvas.SetColor (oldColor);
			}
	*/

			const bool isRowSelected = IsRowSelected (row);

			UIWidget * widgetToRender = 0;

			switch (columnType)
			{

			//----------------------------------------------------------------------

			case UITableTypes::CT_image:
				{
					UIImageStyle * imageStyle = 0;
					if (!mTableModel->GetValueAtImage (row, col, imageStyle) || !imageStyle || imageStyle->GetHeight () == 0L)
						break;

					UIImage * const image = mDefaultRendererImage;
					image->SetHeight (mCellHeight);
					image->SetWidth  (std::min (lastColumnWidth, mCellHeight * imageStyle->GetWidth () / imageStyle->GetHeight ()));
					image->SetStyle  (imageStyle);
					widgetToRender = image;
				}
				break;

			//----------------------------------------------------------------------
			case UITableTypes::CT_bool:

				{
					bool b;
					if (mTableModel->GetValueAtBool (row, col, b))
						UIUtils::FormatBoolean (tmpStr, b);
					else
						tmpStr.clear ();

					UIText * const text = mDefaultRendererText;

					if (isRowSelected)
						text->SetTextColor (mSelectionTextColor);
					else
						text->SetTextColor (mDefaultTextColor);

					text->SetTextAlignment (UITextStyle::Center);
					text->SetLocalText     (tmpStr);
					text->SetSize          (UISize (lastColumnWidth, mCellHeight));
					widgetToRender = text;

					handleTextFlags(flags, text);
				}
				break;

			//----------------------------------------------------------------------

			case UITableTypes::CT_integer:
			case UITableTypes::CT_percent:
			case UITableTypes::CT_delimitedInteger:
				{
					int i;
					if (mTableModel->GetValueAtInteger (row, col, i))
						UIUtils::FormatLong (tmpStr, i);
					else
						tmpStr.clear ();

					if (columnType == UITableTypes::CT_percent)
						tmpStr.append (1, '%');

					if (columnType == UITableTypes::CT_delimitedInteger)
						tmpStr = UIUtils::FormatDelimitedInteger(tmpStr);

					UIText * const text = mDefaultRendererText;

					if (isRowSelected)
						text->SetTextColor (mSelectionTextColor);
					else
						text->SetTextColor (mDefaultTextColor);

					text->SetTextAlignment (UITextStyle::Right);
					text->SetLocalText     (tmpStr);
					text->SetSize          (UISize (lastColumnWidth, mCellHeight));
					widgetToRender = text;

					handleTextFlags(flags, text);
				}
				break;

			//----------------------------------------------------------------------

			case UITableTypes::CT_float:
				{
					float f;
					if (mTableModel->GetValueAtFloat (row, col, f))
						UIUtils::FormatFloat (tmpStr, f);
					else
						tmpStr.clear ();

					UIText * const text = mDefaultRendererText;

					if (isRowSelected)
						text->SetTextColor (mSelectionTextColor);
					else
						text->SetTextColor (mDefaultTextColor);

					text->SetTextAlignment (UITextStyle::Right);
					text->SetLocalText     (tmpStr);
					text->SetSize          (UISize (lastColumnWidth, mCellHeight));
					widgetToRender = text;

					handleTextFlags(flags, text);
				}
				break;

			//----------------------------------------------------------------------

			case UITableTypes::CT_widget:
				{
					widgetToRender = 0;
					UI_IGNORE_RETURN (mTableModel->GetValueAtWidget (row, col, widgetToRender));

					if (widgetToRender)
					{
						widgetToRender->SetSize (UISize (lastColumnWidth, mCellHeight));
					}
				}
				break;

			//----------------------------------------------------------------------

			case UITableTypes::CT_text:
			default:
				{
					if (!mTableModel->GetValueAtText (row, col, tmpStr))
						tmpStr.clear ();

					UIText * const text = mDefaultRendererText;

					if (isRowSelected)
						text->SetTextColor (mSelectionTextColor);
					else
						text->SetTextColor (mDefaultTextColor);

					text->SetTextAlignment (UITextStyle::Left);
					text->SetLocalText     (tmpStr);
					text->SetSize          (UISize (lastColumnWidth, mCellHeight));
					widgetToRender = text;

					handleTextFlags(flags, text);
				}
				break;
			}

			//----------------------------------------------------------------------

			if (widgetToRender)
			{
				if (widgetToRender == mDefaultRendererText)
				{
					mDefaultRendererText->SetTruncateElipsis (true);
					const UITextStyle * const textStyle = mDefaultRendererText->GetTextStyle ();
					if (textStyle)
					{
						const int lines = std::max (1L, mDefaultRendererText->GetHeight () / textStyle->GetLeading ());
						mDefaultRendererText->SetMaxLines (lines);
					}
					else
						mDefaultRendererText->SetMaxLines (1);
				}

				canvas.PushState ();
				canvas.Translate (pt);
				canvas.Clip (widgetToRender->GetRect ());
				widgetToRender->Render (canvas);
				if(widgetToRender->IsA(TUI3DObjectListViewer))
				{
					widgetToRender->RenderText(canvas);
				}
				canvas.PopState ();
			}
		}
	}
}

//----------------------------------------------------------------------

void UITable::handleTextFlags(int flags, UIText * text) const
{
	int mask = UITableFlags::RightHorizontalJustified;
	if((flags & mask) != 0 )
		text->SetTextAlignment (UITextStyle::Right);
	mask = UITableFlags::CenterHorizontalJustified;
	if((flags & mask) != 0 )
		text->SetTextAlignment (UITextStyle::Center);
	mask = UITableFlags::LeftHorizontalJustified;
	if((flags & mask) != 0 )
		text->SetTextAlignment (UITextStyle::Left);
	mask = UITableFlags::TopVerticalAligned;
	if((flags & mask) != 0 )
		text->SetTextAlignmentVertical (UIText::TAV_top);
	mask = UITableFlags::CenterVerticalAligned;
	if((flags & mask) != 0 )
		text->SetTextAlignmentVertical (UIText::TAV_center);
	mask = UITableFlags::BottomVerticalAligned;
	if((flags & mask) != 0 )
		text->SetTextAlignmentVertical (UIText::TAV_bottom);
}

//----------------------------------------------------------------------

void UITable::SetTableModel (UITableModel * model)
{
	RemoveProperty (PropertyName::TableModel);

	if (mTableModel)
		mTableModel->StopListening (this);

	bool retval = AttachMember (mTableModel, model);

	if (mTableModel)
		mTableModel->Listen (this);

	if (retval)
	{
		mColumnSizes->clear ();
		mCachedColumnCount = 0L;

		mSelectedRows->clear ();
		mSelectedRowsInternal->clear ();

		updateCellCounts ();
		refreshCellData ();
		SendNotification (UINotification::TableDataChanged,    mTableModel);
		SendNotification (UINotification::TableColumnsChanged, this);
	}
}

//----------------------------------------------------------------------

void UITable::Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode )
{
	if (NotifyingObject == mTableModel)
	{
		if (NotificationCode == UINotification::TableSortingChanged)
		{
			refreshCellData ();
			mSelectedRows->clear ();

			if (mTableModel)
			{
				for (LongVector::const_iterator it = mSelectedRowsInternal->begin (); it != mSelectedRowsInternal->end (); ++it)
					mSelectedRows->push_back (mTableModel->GetVisualDataRowIndex (*it));
			}

			if (!mSelectedRows->empty ())
			{
				if (!mIgnoreSortingChanges)
					ScrollToRow (mSelectedRows->back ());
			}
		}

		else if (NotificationCode == UINotification::TableColumnsChanged)
		{
			if (!mIgnoreDataChanges)
			{
				refreshCellData ();
				updateCellCounts ();
				refreshCellData ();
			}
		}
		else if (NotificationCode == UINotification::TableDataChanged)
		{
			if (!mIgnoreDataChanges)
				refreshCellData ();
		}

		UI_UNREF (ContextObject);
		SendNotification (NotificationCode, mTableModel);
	}
	else if (NotifyingObject == &UIClock::gUIClock ())
	{
		if (mMouseDown)
		{
			UIPoint scrollLocation (GetScrollLocation ());

			if (mMouseMoveLastPoint.y < 0)
				scrollLocation.y += mMouseMoveLastPoint.y;
			else if (mMouseMoveLastPoint.y > GetHeight ())
				scrollLocation.y += mMouseMoveLastPoint.y - GetHeight ();

			if (mMouseMoveLastPoint.x < 0)
				scrollLocation.x += mMouseMoveLastPoint.x;
			else if (mMouseMoveLastPoint.x > GetWidth ())
				scrollLocation.x += mMouseMoveLastPoint.x - GetWidth ();

			ScrollToPoint (scrollLocation);

		}
	}
	else if (NotifyingObject == mColumnSizeDataSource)
	{
		if (mTableModel)
		{
			if (!mUpdatingColumnWidths)
			{
				updateColumnSizeInfo ();
				updateColumnWidths   ();
			}
		}
	}
}

//----------------------------------------------------------------------

//-- no-op right now, cache data later
void UITable::refreshCellData ()
{
}

//----------------------------------------------------------------------

void UITable::updateCellCounts ()
{
	const long oldColumnCount = mCachedColumnCount;
	mCachedColumnCount = mTableModel ? mTableModel->GetColumnCount () : 0;

	const long oldRowCount = mCachedRowCount;
	mCachedRowCount    = mTableModel ? mTableModel->GetRowCount () : 0;

	if (oldRowCount != mCachedRowCount)
		updateExtent ();

	if (oldColumnCount != mCachedColumnCount)
		updateColumnWidths ();
}

//----------------------------------------------------------------------

void UITable::updateExtent ()
{
	SetMinimumScrollExtent (UIPoint::zero);
	UISize scrollExtent;
	GetScrollExtent (scrollExtent);

	scrollExtent.x = GetWidth ();
	scrollExtent.y = mCachedRowCount * (mCellHeight + mCellPadding.y);

	mTableExtent = scrollExtent;

	SetScrollExtent (scrollExtent);
	SetMinimumScrollExtent (scrollExtent);
}

//-----------------------------------------------------------------

void UITable::ScrollToRow (int targetRow)
{
	long top = 0L;
	long bottom = 0L;

	if (GetTotalRowExtents (targetRow, top, bottom))
	{
		const UIPoint & scrollLocation = GetScrollLocation ();
		const UIPoint & size = GetSize ();

		if (bottom >= (scrollLocation.y + size.y))
			ScrollToPoint (UIPoint (0, bottom - size.y));
		else if (top < scrollLocation.y)
			ScrollToPoint (UIPoint (0, top));
	}
}

//-----------------------------------------------------------------

void  UITable::GetScrollSizes( UISize &PageSize, UISize &LineSize ) const
{
	PageSize = GetSize () / 2L;
	LineSize.y = LineSize.y = mCellHeight + mCellPadding.y;
}

//----------------------------------------------------------------------

long UITable::GetTotalColumnAvailableWidth () const
{
	const long totalPadding   = (mCachedColumnCount * mCellPadding.x) + mCellPadding.x / 2L;

	long minimumWidthToDivide = 4L * mCachedColumnCount;
	return std::max (minimumWidthToDivide, GetWidth () - totalPadding);
}

//-----------------------------------------------------------------

void UITable::updateColumnWidths ()
{
	if (mUpdatingColumnWidths)
		return;

	mUpdatingColumnWidths = true;

	const long oldColumnCount = mColumnSizes->size ();

	long widthToDivide = GetTotalColumnAvailableWidth ();

	if (mCachedColumnCount != oldColumnCount)
	{
		if (mColumnSizeDataSource)
			mColumnSizes->clear  ();

		//-- even resizing
		if (mCachedColumnCount)
		{
			const int numColumnsUpdated = updateColumnSizeInfo ();

			long newWidthToDivide = widthToDivide;
			int numDividers = mCachedColumnCount - numColumnsUpdated;

         int i;
			for (i = 0; i < numColumnsUpdated; ++i)
			{
				const ColumnSizeInfo & csi = (*mColumnSizes) [i];
				if (csi.constant)
					newWidthToDivide -= csi.width;
			}

			if (numDividers)
			{
				long columnSize           = newWidthToDivide / numDividers;
				long widthRemainder       = newWidthToDivide - (columnSize * numDividers);

				for (; i < mCachedColumnCount; ++i)
				{
					ColumnSizeInfo & csi = (*mColumnSizes) [i];
					csi.width = columnSize;

					if (widthRemainder > 0)
					{
						++csi.width;
						--widthRemainder;
					}
				}
			}

			{
				for (ColumnSizeInfoVector::iterator it = mColumnSizes->begin (); it != mColumnSizes->end (); ++it)
				{
					const ColumnSizeInfo & csi = *it;
					if (csi.constant)
						widthToDivide -= csi.width;
				}
			}

			{
				for (ColumnSizeInfoVector::iterator it = mColumnSizes->begin (); it != mColumnSizes->end (); ++it)
				{
					ColumnSizeInfo & csi = *it;

					if (widthToDivide)
					{
						if (!csi.constant)
						{
							csi.proportion = static_cast<float>(csi.width) / static_cast<float>(widthToDivide);
						}
					}
				}
			}
		}
	}

	else
	{
		int numDividers = 0;

		{
			int i = 0;

			for (ColumnSizeInfoVector::iterator it = mColumnSizes->begin (); it != mColumnSizes->end (); ++it, ++i)
			{
				ColumnSizeInfo & csi = *it;

//				csi.constant = (i % 2) != 0;

				if (csi.constant)
				{
					widthToDivide -= csi.width;
				}
				else
				{
					++numDividers;
				}
			}
		}

		long total = 0L;
		int i = 0;
		{
			for (ColumnSizeInfoVector::iterator it = mColumnSizes->begin (); it != mColumnSizes->end (); ++it, ++i)
			{
				ColumnSizeInfo & csi = *it;
				if (!csi.constant)
				{
					long width = std::max (4L, static_cast<long>(widthToDivide * csi.proportion));
					total += width;
					csi.width = width;
				}
			}
		}

		if (widthToDivide > total)
		{
			long widthRemainder = widthToDivide - total;

			for (ColumnSizeInfoVector::reverse_iterator rit = mColumnSizes->rbegin (); rit != mColumnSizes->rend () && widthRemainder; ++rit)
			{
				ColumnSizeInfo & csi = *rit;

				if (!csi.constant)
				{
					++csi.width;
					--widthRemainder;
				}
			}
		}

		updateColumnSizeDataSource ();
	}

	SendNotification (UINotification::TableColumnsChanged, this);

	mUpdatingColumnWidths = false;
}

//-----------------------------------------------------------------

void UITable::SetSize( const UISize & size)
{
	if (size != GetSize ())
	{
		UIWidget::SetSize (size);

		updateExtent ();
		updateColumnWidths ();

		SendNotification (UINotification::TableSizeChanged, this);
	}
}

//-----------------------------------------------------------------

void UITable::SetGridColorHorizontal (const UIColor & color)
{
	mGridColorHorizontal = color;
}

//-----------------------------------------------------------------

void UITable::SetGridColorVertical   (const UIColor & color)
{
	mGridColorVertical = color;
}

//-----------------------------------------------------------------

void UITable::SetCellPadding (const UISize & size)
{
	const long oldX = mCellPadding.x;

	mCellPadding = size;

	updateExtent ();

	if (mCellPadding.x != oldX)
	{
		updateColumnWidths ();
	}
}

//-----------------------------------------------------------------

void UITable::SetCellHeight (long height)
{
	mCellHeight = height;
	updateExtent ();
}

//----------------------------------------------------------------------

void UITable::Link()
{
	UIWidget::Link ();

	mDefaultRendererText->SetParent (this);
	mDefaultRendererText->Link ();
	mDefaultRendererText->SetParent (0);

	mDefaultRendererImage->SetParent (this);
	mDefaultRendererImage->Link ();
	mDefaultRendererImage->SetParent (0);

	updateCellCounts ();
}

//----------------------------------------------------------------------

void UITable::SelectRow (long row)
{
	const bool empty = mSelectedRows->empty ();

	//-- this row is already selected
	if (mSelectedRows->size () == 1 && mSelectedRows->front () == row)
		return;

	mSelectedRows->clear ();
	mSelectedRowsInternal->clear ();
	
	if (row >= 0)
	{
		if (mSelectionAllowedRow)
		{
			if (row < mCachedRowCount)
			{
				mSelectedRows->push_back (row);
				mSelectedRowsInternal->push_back (GetLogicalDataRowIndex (row));
				UIString newLogicalIndex;
				
				if(UIUtils::FormatInteger(newLogicalIndex, GetLogicalDataRowIndex(row)))
					SetProperty(UITable::PropertyName::LogicalRowIndex, newLogicalIndex);

				SendCallback( &UIEventCallback::OnGenericSelectionChanged, UILowerString::null );
			}
		}
	}
	else if (!empty)
		SendCallback( &UIEventCallback::OnGenericSelectionChanged, UILowerString::null );
	mMouseDownCell.y = row;
	mMouseDragLastCell.y = row;
}

//----------------------------------------------------------------------

void UITable::AddRowSelection (long row)
{
	if (row < 0)
		return;
	
	if (mSelectionAllowedRow)
	{
		if (mSelectionAllowedRowMultiple)
		{
			const int logicalRowInternal = GetLogicalDataRowIndex (row);
			
			mSelectedRows->erase (std::remove (mSelectedRows->begin (), mSelectedRows->end (), row), mSelectedRows->end ());
			mSelectedRowsInternal->erase (std::remove (mSelectedRowsInternal->begin (), mSelectedRowsInternal->end (), logicalRowInternal), mSelectedRowsInternal->end ());
			
			mSelectedRows->push_back (row);
			mSelectedRowsInternal->push_back (logicalRowInternal);
			
			SendCallback( &UIEventCallback::OnGenericSelectionChanged, UILowerString::null );
			
		}
		else
			SelectRow (row);
	}
}

//----------------------------------------------------------------------

void UITable::RemoveRowSelection (long row)
{
	if (row < 0)
		return;

	if (mSelectionAllowedRowMultiple)
	{
		const int logicalRowInternal = GetLogicalDataRowIndex (row);

		mSelectedRows->erase (std::remove (mSelectedRows->begin (), mSelectedRows->end (), row), mSelectedRows->end ());
		mSelectedRowsInternal->erase (std::remove (mSelectedRowsInternal->begin (), mSelectedRowsInternal->end (), logicalRowInternal), mSelectedRowsInternal->end ());

		SendCallback( &UIEventCallback::OnGenericSelectionChanged, UILowerString::null );
	}
	else
	{
		SelectRow (-1);
	}
}

//----------------------------------------------------------------------

bool UITable::IsRowSelected (long row) const
{
	return std::find (mSelectedRows->begin (), mSelectedRows->end (), row) != mSelectedRows->end ();
}

//----------------------------------------------------------------------

bool UITable::GetCellFromPoint (const UIPoint & pt, long * row, long * col) const
{
	const UIPoint & scrollLocation = GetScrollLocation ();

	const UIPoint & size = GetSize ();

	const UIPoint translatedPt (pt + scrollLocation);

	bool inside = true;

	if (row)
	{
		*row = std::min (mCachedRowCount - 1L, (translatedPt.y) / (mCellHeight + mCellPadding.y));
	}

	if (pt.y < 0 || pt.y >= size.y || translatedPt.y < 0 || translatedPt.y >= mTableExtent.y)
		inside = false;

	if (col)
	{
		*col = -1;
		int curX = 0;
		int index = 0;
		for (ColumnSizeInfoVector::const_iterator it = mColumnSizes->begin (); it != mColumnSizes->end (); ++it, ++index)
		{
			const ColumnSizeInfo & csi = *it;
			curX += csi.width;

			if (curX > pt.x)
			{
				*col = index;
				break;
			}
		}
	}

	return inside;
}

//----------------------------------------------------------------------

void UITable::addRangeSelection (long one, long two)
{
	if (one <= two)
	{
		for (long i = one; i <= two; ++i)
			AddRowSelection (i);
	}
	else if (two < one)
	{
		for (long i = one; i >= two; --i)
			AddRowSelection (i);
	}
}

//----------------------------------------------------------------------

void UITable::removeRangeSelection (long one, long two)
{
	if (one <= two)
	{
		for (long i = one; i <= two; ++i)
			RemoveRowSelection (i);
	}
	else if (two < one)
	{
		for (long i = one; i >= two; --i)
			RemoveRowSelection (i);
	}
}

//----------------------------------------------------------------------

bool UITable::ProcessMessage( const UIMessage & msg)
{
	if (UIWidget::ProcessMessage (msg))
		return true;

	//----------------------------------------------------------------------

	if (msg.Type == UIMessage::LeftMouseDown || msg.Type == UIMessage::RightMouseDown)
	{
		long row = -1;
		if (!GetCellFromPoint (msg.MouseCoords, &row, 0))
			return false;

		if (msg.Modifiers.LeftControl || msg.Modifiers.LeftControl)
		{
			if (IsRowSelected (row))
				RemoveRowSelection (row);
			else if (mSelectionAllowedRowMultiple)
				AddRowSelection (row);
			else
				SelectRow (row);
		}
		else if (mSelectionAllowedRowMultiple && (msg.Modifiers.LeftShift || msg.Modifiers.RightShift))
		{
			const long lastSelectedRow = GetLastSelectedRow ();
			addRangeSelection (std::max (0L, lastSelectedRow), row);
		}
		else
		{
			if (mSelectedRows->size () != 1 || mSelectedRows->front () != row)
			{
				//-- right button never overrides the selection when clicking within the selection
				//-- this is necessary to make popup menu work right
				if (msg.Type == UIMessage::LeftMouseDown || !IsRowSelected (row))
					SelectRow (row);
			}
		}

		if (msg.Type == UIMessage::LeftMouseDown)
		{
			mMouseDown = true;
			mMouseDownCell.y = row;
			mMouseDragLastCell.y = row;
			UIClock::gUIClock ().ListenPerFrame (this);
		}
		return true;
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::LeftMouseDoubleClick && HasProperty(PropertyName::DoubleClickOkayButton))
	{
		long row = -1;
		if (!GetCellFromPoint (msg.MouseCoords, &row, 0))
			return false;

		if (mSelectedRows->size () != 1 || mSelectedRows->front () != row)
		{
			if (!IsRowSelected (row))
				SelectRow (row);
		}

		UIMessage fakeEnterMessage;
		fakeEnterMessage.Type = UIMessage::KeyDown;
		fakeEnterMessage.Keystroke = UIMessage::Enter;

		if (UIManager::isUIReady()) 
		{
			UIManager::gUIManager().ProcessMessage(fakeEnterMessage);
		}

		UIClock::gUIClock().StopListening(this);

	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::LeftMouseUp || msg.Type == UIMessage::MouseExit)
	{
		mMouseDown = false;
		UIClock::gUIClock ().StopListening (this);
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::MouseMove && mMouseDown)
	{
		long row = -1;
		GetCellFromPoint (msg.MouseCoords, &row, 0);
		
		if (row >= 0)
		{
			if (row >= 0 && row != mMouseDragLastCell.y)
			{
				if (mSelectionAllowedRowMultiple && mSelectionMultipleOnDrag)
				{
					removeRangeSelection (row, mMouseDragLastCell.y);
					addRangeSelection (mMouseDownCell.y, row);
				}
				else
				{
					SelectRow (row);
				}
				
				mMouseDragLastCell.y = row;
			}
		}
		
		mMouseMoveLastPoint = msg.MouseCoords;
		return true;
	}

	else if (msg.Type == UIMessage::KeyDown || msg.Type == UIMessage::KeyRepeat)
	{
		UISize pageSize;
		UISize lineSize;

		GetScrollSizes( pageSize, lineSize );

		long targetRow = mMouseDownCell.y;

		bool changesSelection = false;
		if (msg.Keystroke == UIMessage::DownArrow)
		{
			if (targetRow >= mCachedRowCount)
				return true;

			changesSelection = true;
			++targetRow;
		}
		else if (msg.Keystroke == UIMessage::UpArrow)
		{
			if (targetRow == 0)
				return true;

			changesSelection = true;
			--targetRow;
		}
		else if (msg.Keystroke == UIMessage::PageDown)
		{
			targetRow += (pageSize.y) / (mCellHeight + mCellPadding.y);
			changesSelection = true;
		}
		else if (msg.Keystroke == UIMessage::PageUp)
		{
			targetRow -= (pageSize.y) / (mCellHeight + mCellPadding.y);
			changesSelection = true;
		}
		else if (msg.Keystroke == UIMessage::Home)
		{
			targetRow = 0;
			changesSelection = true;
		}
		else if (msg.Keystroke == UIMessage::End)
		{
			targetRow = mCachedRowCount - 1;
			changesSelection = true;
		}
		else
			return false;

		MoveStuff (targetRow, changesSelection, (msg.Modifiers.LeftShift || msg.Modifiers.RightShift));
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

bool UITable::MoveStuff (long targetRow, bool changesSelection, bool shifted)
{
	targetRow = std::max (0L, std::min (targetRow, mCachedRowCount - 1L));

	if (!shifted)
	{
		mMouseDownCell.y = targetRow;
		if (changesSelection)
			SelectRow (targetRow);
	}
	else if (targetRow != mMouseDragLastCell.y && mSelectionAllowedRowMultiple)
	{
		removeRangeSelection (targetRow, mMouseDragLastCell.y);
		addRangeSelection (mMouseDownCell.y, targetRow);
	}
	else
		return false;

	mMouseDragLastCell.y = targetRow;

	ScrollToRow (targetRow);

	return true;
}

//----------------------------------------------------------------------

bool UITable::OnMessage (UIWidget * context, const UIMessage & msg)
{
	if (context == this && msg.Type == UIMessage::MouseWheel)
	{
		if (mScrollWheelSelection)
		{
			long targetRow = mMouseDragLastCell.y;

			if (msg.Data < 0)
				++targetRow;
			else if (msg.Data > 0)
				--targetRow;
			else
				return true;

			if (MoveStuff (targetRow, true, false))
			{
				return false;
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------

bool UITable::GetTotalColumnExtents (int col, long & x, long & width) const
{
	if (!GetColumnWidth (col, width))
		return false;

	width += mCellPadding.x;

	x = col * mCellPadding.x;

	int i = 0;
	for (ColumnSizeInfoVector::const_iterator it = mColumnSizes->begin (); it != mColumnSizes->end () && i < col; ++it, ++i)
	{
		const ColumnSizeInfo & csi = *it;
		x += csi.width;
	}

	return true;
}

//-----------------------------------------------------------------

bool UITable::GetColumnWidth (int col, long & width) const
{
	if (col < 0 || col >= static_cast<int>(mColumnSizes->size ()))
		return false;

	width = (*mColumnSizes) [col].width;
	return true;
}

//----------------------------------------------------------------------

bool UITable::GetTotalRowExtents (int row, long & top, long & bottom)
{
	if (row < 0 || row >= mCachedRowCount)
		return false;

	top    = row * (mCellHeight + (row == 0 ? 0L : mCellPadding.y));
	bottom = top + mCellHeight + mCellPadding.y;
	return true;
}

//-----------------------------------------------------------------

long UITable::GetLastSelectedRow () const
{
	return mSelectedRows->empty () ? -1 : mSelectedRows->back ();
}

//-----------------------------------------------------------------

int UITable::GetLogicalDataRowIndex (int row)
{
	return mTableModel ? mTableModel->GetLogicalDataRowIndex (row) : -1;
}

//----------------------------------------------------------------------

bool UITable::ResizeColumns (int rightColumn, long dx)
{
	if (dx == 0 || rightColumn < 1 || rightColumn >= static_cast<int>(mColumnSizes->size ()))
		return false;

	long & leftWidth  = (*mColumnSizes) [rightColumn - 1].width;
	long & rightWidth = (*mColumnSizes) [rightColumn].width;

	const long totalWidth = leftWidth + rightWidth;

	leftWidth = std::min (std::max (leftWidth + dx, std::min (leftWidth, MIN_COLUMN_SIZE)), totalWidth - MIN_COLUMN_SIZE);
	rightWidth = totalWidth - leftWidth;

	long widthToDivide = GetTotalColumnAvailableWidth ();

	{
		for (ColumnSizeInfoVector::iterator it = mColumnSizes->begin (); it != mColumnSizes->end (); ++it)
		{
			const ColumnSizeInfo & csi = *it;

			if (csi.constant)
				widthToDivide -= csi.width;
		}
	}

	if (widthToDivide)
	{
		for (ColumnSizeInfoVector::iterator it = mColumnSizes->begin (); it != mColumnSizes->end (); ++it)
		{
			ColumnSizeInfo & csi = *it;

			if (!csi.constant)
				csi.proportion = static_cast<float>(csi.width) / static_cast<float>(widthToDivide);
		}
	}

	updateColumnSizeDataSource ();

	SendNotification (UINotification::TableSizeChanged, this);

	return true;
}

//----------------------------------------------------------------------

void UITable::GetColumnWidthProportions     (FloatVector & fv) const
{
	fv.clear ();
	fv.reserve (mColumnSizes->size ());

	for (ColumnSizeInfoVector::const_iterator it = mColumnSizes->begin (); it != mColumnSizes->end (); ++it)
	{
		const ColumnSizeInfo & csi = *it;
		fv.push_back (csi.proportion);
	}
}

//----------------------------------------------------------------------

bool UITable::RestoreColumnWidthProportions (const FloatVector & fv)
{
	if (mColumnSizes->size () != fv.size ())
		return false;

	int i = 0;
	for (FloatVector::const_iterator it = fv.begin (); it != fv.end (); ++it, ++i)
	{
		const float prop = *it;
		ColumnSizeInfo & csi = (*mColumnSizes) [i];
		csi.proportion = prop;
	}

	updateColumnWidths ();

	return true;
}

//----------------------------------------------------------------------

UIWidget * UITable::GetCustomDragWidget  (const UIPoint & point, UIPoint & offset)
{
	long row = -1;
	long col = -1;
	if (GetCellFromPoint (point, &row, &col))
	{
		if (mTableModel)
		{
			UIWidget * dragWidget = 0;
			dragWidget = mTableModel->GetDragWidgetAt (row, col);

			if (dragWidget)
			{
				if (dragWidget->GetParent () == 0)
					offset -= (dragWidget->GetSize () / 2L);
				else
					offset -= (dragWidget->GetWorldLocation () + (dragWidget->GetSize () / 2L)) - GetWorldLocation ();

				//-- offsets for custom widgets must be negative
				offset.x = std::min (0L, offset.x);
				offset.y = std::min (0L, offset.y);

				offset.x = -std::min (dragWidget->GetWidth (),  -offset.x);
				offset.y = -std::min (dragWidget->GetHeight (), -offset.y);

				return dragWidget;
			}
		}
	}

	return 0;
}

//----------------------------------------------------------------------

void UITable::SetColumnSizeDataSource (UIDataSource * ds)
{
	RemoveProperty (PropertyName::ColumnSizeDataSource);

	if (mColumnSizeDataSource)
		mColumnSizeDataSource->StopListening (this);

	if (AttachMember (mColumnSizeDataSource, ds))
	{
//		updateColumnSizeInfo ();
		updateColumnWidths   ();
	}

	if (mColumnSizeDataSource)
		mColumnSizeDataSource->Listen (this);
}

//----------------------------------------------------------------------

int UITable::updateColumnSizeInfo ()
{
	mColumnSizes->resize (mCachedColumnCount, ColumnSizeInfo ());

	if (!mColumnSizeDataSource)
		return 0;

	const UIDataList & oldData = mColumnSizeDataSource->GetData ();

	int i = 0;
	bool oldDataValid = true;
	UIDataList::const_iterator dit = oldData.begin ();
	for (ColumnSizeInfoVector::iterator it = mColumnSizes->begin (); it != mColumnSizes->end (); ++it)
	{
		if (oldDataValid)
		{
			if (dit == oldData.end ())
				break;
			else
			{
				++i;
				UIData * const data = *dit;
				ColumnSizeInfo & csi = *it;
				csi.updateFromData (*data);
				++dit;
			}
		}
	}

	return i;
}

//----------------------------------------------------------------------

void UITable::updateColumnSizeDataSource ()
{
	if (!mColumnSizeDataSource)
		return;

	mColumnSizeDataSource->StopListening (this);

	const UIDataList & oldData = mColumnSizeDataSource->GetData ();

	bool oldDataValid = true;
	UIDataList::const_iterator dit = oldData.begin ();
	for (ColumnSizeInfoVector::const_iterator it = mColumnSizes->begin (); it != mColumnSizes->end (); ++it)
	{
		UIData * data = 0;

		if (oldDataValid)
		{
			if (dit == oldData.end ())
				oldDataValid = false;
			else
			{
				data = *dit;
				++dit;
			}
		}

		if (!data)
		{
			data = new UIData;
			mColumnSizeDataSource->AddChild (data);
		}

		const ColumnSizeInfo & csi = *it;

		csi.updateData (*data);
	}

	mColumnSizeDataSource->Listen (this);
}

//----------------------------------------------------------------------

void UITable::ColumnSizeInfo::updateFromData (const UIData & data)
{
	data.GetPropertyBoolean (PropertyName::Constant,   constant);
	data.GetPropertyFloat   (PropertyName::Proportion, proportion);
	data.GetPropertyLong    (PropertyName::Width,      width);
}

//----------------------------------------------------------------------

void UITable::ColumnSizeInfo::updateData (UIData & data) const
{
	data.SetPropertyBoolean (PropertyName::Constant,   constant);
	data.SetPropertyFloat   (PropertyName::Proportion, proportion);
	data.SetPropertyLong    (PropertyName::Width,      width);
}

//----------------------------------------------------------------------

void UITable::SetColumnSizeInfo (const ColumnSizeInfoVector & csiv)
{
	*mColumnSizes = csiv;
	mCachedColumnCount = csiv.size ();
	updateColumnWidths ();
}

//----------------------------------------------------------------------

int UITable::GetRowCount()
{
	int result = 0;

	UITableModel * tableModel = GetTableModel();

	if (tableModel != NULL)
	{
		result = tableModel->GetRowCount();
	}

	return result;
}

//----------------------------------------------------------------------

void UITable::SetIgnoreDataChanges         (bool b)
{
	mIgnoreDataChanges = b;
}

//----------------------------------------------------------------------

void UITable::SetIgnoreSortingChanges (bool b)
{
	mIgnoreSortingChanges = b;
}

//----------------------------------------------------------------------

const Unicode::String & UITable::GetLocalTooltip              (const UIPoint & pt) const
{
	if (mTableModel)
	{
		long row = 0;
		long col = 0;
		
		if (GetCellFromPoint (pt, &row, &col))
		{
			static Unicode::String value;
			if (mTableModel->GetTooltipAt (row, col, value))
				return value;
		}
	}

	return UIWidget::GetLocalTooltip (pt);
}

//======================================================================
