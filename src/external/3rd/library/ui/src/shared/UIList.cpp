//======================================================================
//
// UIList.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "_precompile.h"
#include "UIList.h"

#include "UICanvas.h"
#include "UIClock.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIListStyle.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIRectangleStyle.h"
#include "UIText.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"
#include "UIWidgetRectangleStyles.h"

#include <list>
#include <numeric>
#include <vector>

//======================================================================

const char * const UIList::TypeName = "List";

//----------------------------------------------------------------------

const UILowerString  UIList::DataProperties::LOCALTEXT = UILowerString ("LocalText");
const UILowerString  UIList::DataProperties::TEXT      = UILowerString ("Text");

//----------------------------------------------------------------------

const UILowerString  UIList::PropertyName::DataSource               = UILowerString ("DataSource");
const UILowerString  UIList::PropertyName::RenderSeperatorLines     = UILowerString ("RenderSeperatorLines");
const UILowerString  UIList::PropertyName::SelectedRow              = UILowerString ("SelectedRow");
const UILowerString  UIList::PropertyName::SelectedRows             = UILowerString ("SelectedRows");
const UILowerString  UIList::PropertyName::SelectedRowCount         = UILowerString ("SelectedRowCount");
const UILowerString  UIList::PropertyName::SelectionAllowedMultiRow = UILowerString ("SelectionAllowedMultiRow");
const UILowerString  UIList::PropertyName::SelectionClickToggleMode = UILowerString ("SelectionClickToggleMode");
const UILowerString  UIList::PropertyName::SelectionAllowedDragSelection = UILowerString ("SelectionAllowedDragSelection");
const UILowerString  UIList::PropertyName::SelectionAllowedKeySelection = UILowerString ("SelectionAllowedKeySelection");
const UILowerString  UIList::PropertyName::Style                    = UILowerString ("Style");
const UILowerString  UIList::PropertyName::ScrollWheelSelection     = UILowerString ("ScrollWheelSelection");
const UILowerString  UIList::PropertyName::SelectedItem             = UILowerString ("SelectedItem");
const UILowerString  UIList::PropertyName::OnSelectionChanged       = UILowerString ("OnSelectionChanged");

//======================================================================================
#define _TYPENAME UIList

namespace UIListNamespace
{
	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(DataSource, "", T_object),
		_DESCRIPTOR(SelectedRow, "", T_int),
		_DESCRIPTOR(SelectedRows, "", T_string), // LONG VECTOR
		_DESCRIPTOR(Style, "", T_object),
	_GROUPEND(Basic, 2, 0);
	//================================================================

	//================================================================
	// Appearance category
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(RenderSeperatorLines, "", T_bool),
	_GROUPEND(Appearance, 2, 0);
	//================================================================

	//================================================================
	// Behavior category
	_GROUPBEGIN(Behavior)
		_DESCRIPTOR(ScrollWheelSelection, "", T_bool),
		_DESCRIPTOR(SelectionAllowedMultiRow, "", T_bool),
		_DESCRIPTOR(SelectionClickToggleMode, "", T_bool),
		_DESCRIPTOR(SelectionAllowedDragSelection, "", T_bool),
		_DESCRIPTOR(SelectionAllowedKeySelection, "", T_bool),
	_GROUPEND(Behavior, 2, 0);
	//================================================================

	//================================================================
	// AdvancedBehavior category
	_GROUPBEGIN(AdvancedBehavior)
		_DESCRIPTOR(OnSelectionChanged, "", T_string),
	_GROUPEND(AdvancedBehavior, 2, 0);
	//================================================================
}
using namespace UIListNamespace;
//======================================================================================

UIList::UIList () :
UIWidget                      (),
mDataSource                   (0),
mSelectedRows                 (new LongList),
mLastToggledRow               (0),
mSelectionAllowedRow          (true),
mSelectionAllowedRowMultiple  (true),
mSelectionClickToggleMode     (false),
mSelectionAllowedDragSelection(true),
mSelectionAllowedKeySelection (true),
mMouseDown                    (false),
mMouseDownRow                 (0L),
mMouseDragLastRow             (0L),
mMouseMoveLastPoint           (),
mListExtent                   (),
mStyle                        (0),
mSelectionMovesWithMouse      (false),
mSelectionMovingWithWheel     (false),
mSelectionFinalized           (false),
mScrollWheelSelection         (false),
mSuppressSelectionChangedCallback (false),
mRenderSeperatorLines             (true)
{
	SetBackgroundColor (UIColor (200, 200, 200));
	SetBackgroundOpacity (1.0f);

	AddCallback (this);

	SetSelectable (true);
}

//----------------------------------------------------------------------

UIList::~UIList ()
{
	//-- don't re-create the clock accidentally
	UIClock * const clock = UIClock::GetSingleton ();
	if (clock)
		clock->StopListening (this);

	RemoveCallback (this);

	SetDataSource (0);
	SetStyle (0);

	delete mSelectedRows;
	mSelectedRows = 0;
}

//-----------------------------------------------------------------

bool UIList::IsA( const UITypeID Type ) const
{
	return (Type == TUIList) || UIWidget::IsA (Type);
}

//----------------------------------------------------------------------

const char * UIList::GetTypeName  () const
{
	return TypeName;
}

//----------------------------------------------------------------------

UIBaseObject * UIList::Clone () const
{
	return new UIList;
}

//----------------------------------------------------------------------

void UIList::GetLinkPropertyNames( UIPropertyNameVector & in ) const
{
	in.push_back ( PropertyName::DataSource );
	in.push_back ( PropertyName::Style);

	UIWidget::GetLinkPropertyNames (in);
}

//----------------------------------------------------------------------

void UIList::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidget::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Behavior, category, o_groups);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
	GROUP_TEST_AND_PUSH(AdvancedBehavior, category, o_groups);
}

//----------------------------------------------------------------------

void UIList::GetPropertyNames( UIPropertyNameVector & in, bool forCopy ) const
{
	// PropertyName::SelectedItem is a special property
	// and is not returned by this function

	in.push_back ( PropertyName::DataSource );
	in.push_back ( PropertyName::SelectedRow             );
	in.push_back ( PropertyName::SelectedRows            );
	in.push_back ( PropertyName::SelectedRowCount        );
	in.push_back ( PropertyName::SelectionAllowedMultiRow);
	in.push_back ( PropertyName::SelectionClickToggleMode);
	in.push_back ( PropertyName::SelectionAllowedDragSelection);
	in.push_back ( PropertyName::SelectionAllowedKeySelection);
	in.push_back ( PropertyName::Style);
	in.push_back (PropertyName::ScrollWheelSelection);
	in.push_back (PropertyName::OnSelectionChanged);
	in.push_back (PropertyName::RenderSeperatorLines);

	UIWidget::GetPropertyNames (in, forCopy);
}

//----------------------------------------------------------------------

bool UIList::SetProperty( const UILowerString & Name, const UIString &Value )
{
	const size_t pSeparator = Name.get ().find ('.');

	if( pSeparator != std::string::npos)
	{
		const int len = pSeparator;

		if( Name.equals (PropertyName::SelectedItem.c_str (), len))
		{
			UIData * const selectedDataObject = GetDataAtRow(GetLastSelectedRow());

			if (selectedDataObject)
				selectedDataObject->SetProperty( UILowerString (Name.get ().substr (pSeparator + 1)), Value );

			return true;
		}
	}

	if (Name == PropertyName::Style       )
	{
		UIBaseObject * const object = GetObjectFromPath( Value, TUIListStyle );

		if( object || Value.empty() )
		{
			SetStyle( static_cast<UIListStyle *>(object) );
			return true;
		}
	}

	else if (Name == PropertyName::DataSource       )
	{
		UIBaseObject * const object = GetObjectFromPath( Value, TUIDataSource );

		if( object || Value.empty() )
		{
			SetDataSource( static_cast<UIDataSource *>(object) );
			return true;
		}
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
	else if (Name == PropertyName::SelectedRows)
	{
		mSuppressSelectionChangedCallback = true;
		//Clear selected rows
		long row = 0;		
		while(row != -1)
		{
			row = GetLastSelectedRow();
			if(row != -1)
				RemoveRowSelection(row);
		}
		std::vector<long> rowsToSelect;

		//Set new ones
		if(UIUtils::ParseLongVector(Value, rowsToSelect))
		{
			for(int i = 0; i < static_cast<int>(rowsToSelect.size()); i++)
			{
				AddRowSelection(rowsToSelect[i]);
			}
		}
		mSuppressSelectionChangedCallback = false;		
		SendCallback( &UIEventCallback::OnGenericSelectionChanged, PropertyName::OnSelectionChanged);
	}

	else if (Name == PropertyName::SelectedRowCount         )
	{
		return false;
	}

	else if (Name == PropertyName::SelectionAllowedMultiRow )
	{
		return UIUtils::ParseBoolean (Value, mSelectionAllowedRowMultiple);
	}
	else if (Name == PropertyName::SelectionClickToggleMode )
	{
		return UIUtils::ParseBoolean (Value, mSelectionClickToggleMode);
	}
	else if (Name == PropertyName::SelectionAllowedDragSelection )
	{
		return UIUtils::ParseBoolean (Value, mSelectionAllowedDragSelection);
	}
	else if (Name == PropertyName::SelectionAllowedKeySelection )
	{
		return UIUtils::ParseBoolean (Value, mSelectionAllowedKeySelection);
	}
	else if (Name == PropertyName::ScrollWheelSelection)
	{
		return UIUtils::ParseBoolean (Value, mScrollWheelSelection);
	}
	else if (Name == PropertyName::RenderSeperatorLines)
	{
		return UIUtils::ParseBoolean (Value, mRenderSeperatorLines);
	}

	return UIWidget::SetProperty (Name, Value);
}

//----------------------------------------------------------------------

bool UIList::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	const size_t pSeparator = Name.get ().find ('.');

	if( pSeparator != std::string::npos)
	{
		const int len = pSeparator;

		if( Name.equals (PropertyName::SelectedItem.c_str (), len))
		{
			UIData const * const selectedDataObject = GetDataAtRow(GetLastSelectedRow());

			if (selectedDataObject)
				selectedDataObject->GetProperty( UILowerString (Name.get ().substr (pSeparator + 1)), Value );

			return true;
		}
	}

	if (Name == PropertyName::Style       )
	{
		if( mStyle )
		{
			GetPathTo( Value, mStyle );
			return true;
		}
	}

	if (Name == PropertyName::DataSource       )
	{
		if( mDataSource )
		{
			GetPathTo( Value, mDataSource );
			return true;
		}
	}

	else if (Name == PropertyName::SelectedRow              )
	{
		return UIUtils::FormatLong (Value, GetLastSelectedRow ());
	}
	else if (Name == PropertyName::SelectedRows             )
	{
		return UIUtils::FormatLongList (Value, *mSelectedRows);
	}

	else if (Name == PropertyName::SelectedRowCount         )
	{
		return UIUtils::FormatLong (Value, static_cast<long>(mSelectedRows->size ()));
	}

	else if (Name == PropertyName::SelectionAllowedMultiRow )
	{
		return UIUtils::FormatBoolean (Value, mSelectionAllowedRowMultiple);
	}
	else if (Name == PropertyName::SelectionClickToggleMode )
	{
		return UIUtils::FormatBoolean (Value, mSelectionClickToggleMode);
	}
	else if (Name == PropertyName::SelectionAllowedDragSelection )
	{
		return UIUtils::FormatBoolean (Value, mSelectionAllowedDragSelection);
	}
	else if (Name == PropertyName::SelectionAllowedKeySelection )
	{
		return UIUtils::FormatBoolean (Value, mSelectionAllowedKeySelection);
	}
	else if (Name == PropertyName::ScrollWheelSelection)
	{
		return UIUtils::FormatBoolean (Value, mScrollWheelSelection);
	}
	else if (Name == PropertyName::RenderSeperatorLines)
	{
		return UIUtils::FormatBoolean (Value, mRenderSeperatorLines);
	}

	return UIWidget::GetProperty (Name, Value);
}

//----------------------------------------------------------------------

void UIList::Render (UICanvas & canvas) const
{
	if (!mDataSource)
		return;

	bool renderDefault = true;

	const UIPoint & scrollLocation = GetScrollLocation ();

	if (mStyle)
	{
		const UIRectangleStyle * const bgstyle = mStyle->GetRectangleStyles ().GetAppropriateStyle (*this, 0);

		if (bgstyle)
		{
			canvas.PushState ();

			canvas.ModifyOpacity (GetBackgroundOpacity ());
			canvas.Translate  (scrollLocation);

			bgstyle->Render   (GetAnimationState (), canvas, GetSize ());

			canvas.PopState  ();

			renderDefault = false;
		}
	}

	if (renderDefault)
		UIWidget::Render (canvas);

	if (!mStyle)
		return;

	const long numRows = static_cast<long>(mDataSource->GetChildCount ());

	if (numRows <= 0)
		return;

	const UIRect & margin          = mStyle->GetMargin ();
	const UIPoint & size           = GetSize ();
	UISize scrollExtent;
	GetScrollExtent (scrollExtent);

	const UISize & cellPadding = mStyle->GetCellPadding ();
	const long     cellHeight  = mStyle->GetCellHeight ();

	const UIColor oldColor = canvas.GetColor ();
	const UISize halfCellPadding (cellPadding / 2L);

	const UIColor & selectionBackground = mStyle->GetSelectionColorBackground ();
	const UIColor & selectionRect       = mStyle->GetSelectionColorRect ();
	const UIColor & gridColor           = mStyle->GetGridColor ();

	canvas.PushState ();

	canvas.Clip (scrollLocation.x + margin.left, scrollLocation.y + margin.top, scrollLocation.x + size.x - margin.right, scrollLocation.y + size.y - margin.bottom);

	{
		UILine line;

		if (cellPadding.y > 0)
		{
			long y = mStyle->GetMargin ().top;

			line.p1.x = static_cast<float>(std::max (margin.left,                   scrollLocation.x));
			line.p2.x = static_cast<float>(std::min (scrollExtent.x - margin.right, static_cast<long>(line.p1.x) + size.x));

			bool lastRowSelected = false;

			//- we must draw numRows + 1 rows
			for (int row = 0; row <= numRows; ++row, y += cellHeight + cellPadding.y)
			{
				canvas.SetColor (oldColor);

				if (IsRowSelected (row))
				{
					if ((y + cellHeight + cellPadding.y) >= (scrollLocation.y + margin.top) && y < (scrollLocation.y + size.y - margin.bottom))
					{
						canvas.ModifyColor (selectionBackground);
						const long lineWidth = static_cast<long>(line.p2.x - line.p1.x);
						canvas.BltFrom (0, UIPoint::zero, UIPoint (static_cast<long>(line.p1.x), y + halfCellPadding.y ), UISize (lineWidth, cellHeight));
						canvas.SetColor (oldColor);
					}
				}

				if (y < (scrollLocation.y + margin.top))
					continue;

				if (y >= (scrollLocation.y + size.y - margin.bottom))
					break;

				line.p1.y = line.p2.y = static_cast<float>(y);

				const bool isRowSelected = IsRowSelected (row);

				if (isRowSelected || lastRowSelected)
					canvas.ModifyColor (selectionRect);
				else
					canvas.ModifyColor (gridColor);

				lastRowSelected = isRowSelected;
				if(mRenderSeperatorLines)
					canvas.RenderLine (line);
			}
		}
	}

	//----------------------------------------------------------------------
	//----------------------------------------------------------------------
	//----------------------------------------------------------------------

	//- cache this

	UIPoint pt (halfCellPadding + margin.Location ());
	long columnWidth = GetWidth () - (cellPadding.x * 2L) - margin.left - margin.right;

	for (int row = 0; row < numRows; ++row, pt.y += cellHeight + cellPadding.y)
	{
		canvas.SetColor (oldColor);

		const UIData * const data = mDataSource->GetChildByPosition (row);

		if (!data)
			break;

		UIWidget * widgetToRender = 0;

		{
			UIString str;
			if (!data->GetProperty (DataProperties::LOCALTEXT, str) || str.empty ())
				data->GetProperty (DataProperties::TEXT, str);

			UIText & text = mStyle->GetDefaultText ();
			const bool isRowSelected = IsRowSelected (row);

			if (isRowSelected)
				text.SetTextColor (mStyle->GetSelectionTextColor ());
			else
				text.SetTextColor (mStyle->GetDefaultTextColor   ());

			text.SetPreLocalized    (true);
			text.SetMaxLines        (1);
			text.SetTruncateElipsis (true);
			text.SetLocalText       (str);
			text.SetSize            (UISize (columnWidth, cellHeight));
			widgetToRender = &text;
		}

		if (widgetToRender)
		{
			canvas.PushState       ();
			canvas.Translate       (pt);
			canvas.Clip            (widgetToRender->GetRect ());
			widgetToRender->Render (canvas);
			if(widgetToRender->IsA(TUI3DObjectListViewer))
			{
				widgetToRender->RenderText(canvas);
			}
			canvas.PopState        ();
		}
	}

	canvas.PopState ();
}

//----------------------------------------------------------------------

void UIList::SetDataSource (UIDataSource * ds)
{
	RemoveProperty (PropertyName::DataSource);

	if (mDataSource)
		mDataSource->StopListening (this);

	if (AttachMember (mDataSource, ds))
		updateExtent ();

	if (mDataSource)
		mDataSource->Listen (this);
}

//----------------------------------------------------------------------

void UIList::Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode )
{
	if (NotifyingObject == mDataSource)
	{
		UI_UNREF (ContextObject);

		if (NotificationCode != UINotification::ChildChanged &&
			NotificationCode != UINotification::ObjectChanged)
		{
			SelectRow (-1);
			updateExtent ();
		}
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
}

//----------------------------------------------------------------------

void UIList::updateExtent ()
{
	if (!mStyle)
		return;

	UISize scrollExtent;
	GetScrollExtent (scrollExtent);

	scrollExtent.x = GetWidth ();

	const long cellHeight      = mStyle->GetCellHeight ();
	const long rowCount        = mDataSource ? static_cast<long>(mDataSource->GetChildCount ()) : 0;
	const UISize & cellPadding = mStyle->GetCellPadding ();

	const UIRect & margin = mStyle->GetMargin ();

	scrollExtent.y = (rowCount * (cellHeight + cellPadding.y)) + margin.top + margin.bottom;

	mListExtent = scrollExtent;

	SetMinimumScrollExtent (scrollExtent);
	SetScrollExtent (scrollExtent);
}

//-----------------------------------------------------------------

void UIList::ScrollToRow (int targetRow)
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

void  UIList::GetScrollSizes( UISize &PageSize, UISize &LineSize ) const
{
	if (!mStyle)
	{
		UIWidget::GetScrollSizes (PageSize, LineSize);
		return;
	}

	const long cellHeight      = mStyle->GetCellHeight ();
	const UISize & cellPadding = mStyle->GetCellPadding ();

	PageSize = GetSize () / 2L;
	LineSize.y = LineSize.y = cellHeight + cellPadding.y;
}

//-----------------------------------------------------------------

void UIList::SetSize( const UISize & size)
{
	if (size != GetSize ())
	{
		UIWidget::SetSize (size);
		updateExtent ();
	}
}

//----------------------------------------------------------------------

void UIList::Link()
{
	UIWidget::Link ();

	if (mStyle)
		mStyle->Link ();
}

//----------------------------------------------------------------------

void UIList::SelectRow (long row, bool const sendCallback)
{
	if (row < 0 && mSelectedRows->empty ())
		return;

	if (mSelectedRows->size () != 1 || mSelectedRows->front () != row)
	{
		mSelectedRows->clear ();

		row = std::min (row, GetRowCount () - 1L);
		if (row >= 0)
		{
			mSelectedRows->push_back (row);
		}

		mLastToggledRow = row;

		if(sendCallback)
		{
			SendCallback( &UIEventCallback::OnGenericSelectionChanged, PropertyName::OnSelectionChanged);
		}
	}

	mMouseDragLastRow = row;
}

//----------------------------------------------------------------------

void UIList::AddRowSelection (long row, bool const sendCallback)
{
	if (row < 0)
		return;

	if (mSelectionAllowedRowMultiple)
	{
		mSelectedRows->remove (row);
		mSelectedRows->push_back (row);
	}
	else
		SelectRow (row, sendCallback);

	mLastToggledRow = row;

	if(!mSuppressSelectionChangedCallback && sendCallback)
		SendCallback( &UIEventCallback::OnGenericSelectionChanged, PropertyName::OnSelectionChanged);
}

//----------------------------------------------------------------------

void UIList::RemoveRowSelection (long row, bool const sendCallback)
{
	if (row < 0)
		return;

	if (mSelectionAllowedRowMultiple)
	{
		mSelectedRows->remove (row);
	}
	else
	{
		SelectRow (-1, sendCallback);
	}

	mLastToggledRow = row;

	if(!mSuppressSelectionChangedCallback && sendCallback)
		SendCallback( &UIEventCallback::OnGenericSelectionChanged, PropertyName::OnSelectionChanged);
}

//----------------------------------------------------------------------

void UIList::ToggleRowSelection (long row, bool const sendCallback)
{
	if( IsRowSelected (row))
	{
		RemoveRowSelection (row, sendCallback);
	}
	else
	{
		AddRowSelection (row, sendCallback);
	}
}

//----------------------------------------------------------------------

bool UIList::IsRowSelected (long row) const
{
	return std::find (mSelectedRows->begin (), mSelectedRows->end (), row) != mSelectedRows->end ();
}

//----------------------------------------------------------------------

bool UIList::GetRowFromPoint (const UIPoint & pt, long & row) const
{
	if (!mStyle || !mDataSource)
	{
		row = -1;
		return false;
	}

	const UIPoint & scrollLocation = GetScrollLocation ();
	const UIPoint & size           = GetSize ();
	const UIPoint translatedPt (pt + scrollLocation - mStyle->GetMargin ().Location ());
	const long     cellHeight      = mStyle->GetCellHeight ();
	const UISize & cellPadding     = mStyle->GetCellPadding ();

	row = std::min (static_cast<long>(mDataSource->GetChildCount ()) -1L, (translatedPt.y) / (cellHeight + cellPadding.y));

	if (pt.y < 0 || pt.y >= size.y || translatedPt.y < 0 || translatedPt.y >= mListExtent.y)
		return false;

	return true;
}

//----------------------------------------------------------------------

void UIList::addRangeSelection (long one, long two)
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

void UIList::removeRangeSelection (long one, long two)
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

bool UIList::ProcessMessage( const UIMessage & msg)
{
	if (UIWidget::ProcessMessage (msg))
		return true;

	if (!mStyle || !mDataSource)
		return false;

	const long rowCount        = static_cast<long>(mDataSource ? mDataSource->GetChildCount () : 0);
	const long cellHeight      = mStyle->GetCellHeight ();
	const UISize & cellPadding = mStyle->GetCellPadding ();

	//----------------------------------------------------------------------

	if (msg.Type == UIMessage::LeftMouseDown)
	{
		mSelectionFinalized = false;

		long row = -1;
		if (!GetRowFromPoint (msg.MouseCoords, row))
			return false;

		if (mSelectionClickToggleMode)
		{
			ToggleRowSelection (row);
		}
		else if (msg.Modifiers.isControlDown ())
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
			SelectRow (row);

		mMouseDown = true;
		mMouseDownRow = row;
		mMouseDragLastRow = row;

		UIClock::gUIClock ().ListenPerFrame (this);

		mSelectionMovingWithWheel = false;

		return true;
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::LeftMouseUp || msg.Type == UIMessage::MouseExit)
	{
		const bool mouseWasDown = mMouseDown;

		mMouseDown = false;
		UIClock::gUIClock ().StopListening (this);

		if (mSelectionMovesWithMouse && msg.Type == UIMessage::LeftMouseUp)
		{
			if (HitTest (msg.MouseCoords))
			{
				mSelectionFinalized = true;
				SendCallback( &UIEventCallback::OnGenericSelectionChanged, PropertyName::OnSelectionChanged);
			}
			else if (mouseWasDown)
				return true;
		}

		return false;
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::LeftMouseDoubleClick)
	{
		mMouseDown = false;
		long row = -1;
		if (!GetRowFromPoint (msg.MouseCoords, row))
			return false;
		if(mSelectionClickToggleMode)
		{
			ToggleRowSelection (row);
		}
		else
		{
			SelectRow (row);
		}
		UIMessage fakeEnterMessage;
		fakeEnterMessage.Type = UIMessage::KeyDown;
		fakeEnterMessage.Keystroke = UIMessage::Enter;

		if (UIManager::isUIReady()) 
		{
			UIManager::gUIManager().ProcessMessage(fakeEnterMessage);
		}
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::MouseMove)
	{
		if (mSelectionAllowedDragSelection && (mMouseDown || (mSelectionMovesWithMouse && !mSelectionMovingWithWheel)))
		{
			long row = -1;
			GetRowFromPoint (msg.MouseCoords, row);

			if (row >= 0 && row != mMouseDragLastRow)
			{
				if (mSelectionAllowedRowMultiple)
				{
					if (row >= 0 && row != mMouseDragLastRow)
					{
						removeRangeSelection (row, mMouseDragLastRow);
						addRangeSelection (mMouseDownRow, row);
					}
				}
				else
				{
					SelectRow (row);
				}
			}

			mMouseDragLastRow = row;
			mMouseMoveLastPoint = msg.MouseCoords;
			return true;
		}
	}

	else if (msg.Type == UIMessage::KeyDown || msg.Type == UIMessage::KeyRepeat)
	{
		UISize pageSize;
		UISize lineSize;

		GetScrollSizes( pageSize, lineSize );

		long targetRow = mMouseDragLastRow;

		bool changesSelection = false;
		if (msg.IsKeyMessage () && msg.Keystroke == UIMessage::DownArrow)
		{
			changesSelection = true;
			++targetRow;
		}
		else if (msg.IsKeyMessage () && msg.Keystroke == UIMessage::UpArrow)
		{
			changesSelection = true;
			--targetRow;
		}
		else if (msg.Keystroke == UIMessage::PageDown)
			targetRow += (pageSize.y) / (cellHeight + cellPadding.y);
		else if (msg.Keystroke == UIMessage::PageUp)
			targetRow -= (pageSize.y) / (cellHeight + cellPadding.y);
		else if (msg.Keystroke == UIMessage::Home)
			targetRow = 0;
		else if (msg.Keystroke == UIMessage::End)
			targetRow = rowCount - 1;

		targetRow = std::max (0L, std::min (targetRow, rowCount - 1L));

		if (mSelectionAllowedKeySelection)
		{
			if (!(msg.Modifiers.LeftShift || msg.Modifiers.RightShift))
			{
				mMouseDownRow = targetRow;
				if (changesSelection)
					SelectRow (targetRow);
			}
			else if (targetRow != mMouseDragLastRow && mSelectionAllowedRowMultiple)
			{
				removeRangeSelection (targetRow, mMouseDragLastRow);
				addRangeSelection (mMouseDownRow, targetRow);
			}
			else
				return true;
		}

		if (mMouseDragLastRow != targetRow || (msg.IsKeyMessage () && changesSelection))
		{
			mMouseDragLastRow = targetRow;
			ScrollToRow (targetRow);
			return true;
		}

		return changesSelection;
	}
	else if (msg.Type == UIMessage::MouseWheel)
		return true;

	return false;
}

//-----------------------------------------------------------------

bool UIList::MoveStuff (long targetRow, bool changesSelection, bool shifted)
{
	if (!mDataSource)
		return false;

	const long rowCount        = static_cast<long>(mDataSource->GetChildCount ());

	targetRow = std::max (0L, std::min (targetRow, rowCount - 1L));

	if (!shifted)
	{
		mMouseDownRow = targetRow;
		if (changesSelection)
			SelectRow (targetRow);
	}
	else if (targetRow != mMouseDragLastRow && mSelectionAllowedRowMultiple)
	{
		removeRangeSelection (targetRow, mMouseDragLastRow);
		addRangeSelection (mMouseDownRow, targetRow);
	}
	else
		return false;

	mMouseDragLastRow = targetRow;
	ScrollToRow (targetRow);

	return true;
}

//-----------------------------------------------------------------

bool UIList::OnMessage (UIWidget * context, const UIMessage & msg)
{
	if (context == this && msg.Type == UIMessage::MouseWheel)
	{
		if (mScrollWheelSelection)
		{
			long targetRow = mMouseDragLastRow;

			if (msg.Data < 0)
				++targetRow;
			else if (msg.Data > 0)
				--targetRow;
			else
				return true;

			if (MoveStuff (targetRow, true, false))
			{
				mSelectionMovingWithWheel = true;
				return false;
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------

bool UIList::GetTotalRowExtents (int row, long & top, long & bottom)
{
	const long rowCount = static_cast<long>(mDataSource ? mDataSource->GetChildCount () : 0);

	if (row < 0 || row >= rowCount)
		return false;

	const long cellHeight  = mStyle->GetCellHeight ();
	const UISize & cellPadding = mStyle->GetCellPadding ();

	top    = row * (cellHeight + (row == 0 ? 0L : cellPadding.y));
	bottom = top + cellHeight + cellPadding.y;
	return true;
}

//-----------------------------------------------------------------

long UIList::GetLastSelectedRow () const
{
	return mSelectedRows->empty () ? -1 : mSelectedRows->back ();
}

//-----------------------------------------------------------------

long UIList::GetLastToggledRow () const
{
	return mLastToggledRow;
}

//----------------------------------------------------------------------

void UIList::SetStyle (UIListStyle * style)
{
	RemoveProperty (PropertyName::Style);

	AttachMember (mStyle, style);
	updateExtent ();
}

//----------------------------------------------------------------------

UIStyle * UIList::GetStyle( void ) const
{
	return mStyle;
}

//----------------------------------------------------------------------

void UIList::SetSelectionAllowedMultiRow (bool b)
{
	mSelectionAllowedRowMultiple = b;
}

//----------------------------------------------------------------------

void UIList::SetSelectionClickToggleMode (bool b)
{
	mSelectionClickToggleMode = b;
}

//----------------------------------------------------------------------

void UIList::SetSelectionAllowedDragSelection (bool b)
{
	mSelectionAllowedDragSelection = b;
}

//----------------------------------------------------------------------

void UIList::SetSelectionAllowedKeySelection (bool b)
{
	mSelectionAllowedKeySelection = b;
}

//----------------------------------------------------------------------

void UIList::SetSelectionMovesWithMouse (bool b)
{
	mSelectionMovesWithMouse = b;
	mSelectionMovingWithWheel = false;
}

//----------------------------------------------------------------------

void UIList::SetSelectionFinalized (bool b)
{
	mSelectionFinalized = b;
}

//----------------------------------------------------------------------

void UIList::SetRenderSeperatorLines (bool b)
{
	mRenderSeperatorLines = b;
}

//----------------------------------------------------------------------

bool UIList::GetRenderSeperatorLines ()
{
	return mRenderSeperatorLines;
}

//----------------------------------------------------------------------

UIData * UIList::GetDataAtRow (long row)
{
	if (!mDataSource || row < 0 || row >= static_cast<long>(mDataSource->GetChildCount ()))
		return 0;

	return mDataSource->GetChildByPosition (row);
}

//----------------------------------------------------------------------

UIData const * UIList::GetDataAtRow(long row) const
{
	return const_cast<UIList *>(this)->GetDataAtRow(row);
}

//----------------------------------------------------------------------

int UIList::GetRowCount           () const
{
	if (mDataSource)
		return static_cast<int>(mDataSource->GetChildCount ());

	return 0;
}

//----------------------------------------------------------------------

void UIList::UpdateFromStyle            ()
{
	if (mStyle)
	{
		SetBackgroundColor   (mStyle->GetBackgroundColor ());
		SetBackgroundOpacity (mStyle->GetBackgroundOpacity ());
	}
}

//----------------------------------------------------------------------

void UIList::SetScrollWheelSelection    (bool b)
{
	mScrollWheelSelection = b;
}

//----------------------------------------------------------------------

void UIList::Clear ()
{
	if (mDataSource)
		mDataSource->Clear();
}

//----------------------------------------------------------------------

void UIList::AddRow(Unicode::String const &localizedString, std::string const &name)
{
	UIDataSource *dataSource = GetDataSource();

	if (dataSource != NULL)
	{
		UIData *data = new UIData;
		data->SetProperty(UIList::DataProperties::LOCALTEXT, localizedString);
		data->SetName(name);
		dataSource->AddChild(data);
	}
}

//----------------------------------------------------------------------

bool UIList::RemoveRow(long row)
{
	bool result = false;;
	UIData *data = GetDataAtRow(row);

	if (data != NULL)
	{
		UIDataSource *dataSource = GetDataSource();

		if (dataSource != NULL)
		{
			result = dataSource->RemoveChild(data);
		}
	}

	return result;
}

//----------------------------------------------------------------------

bool UIList::GetLocalText(int const row, Unicode::String &localText)
{
	bool result = false;
	UIData *data = GetDataAtRow(row);

	if (data != NULL)
	{
		data->GetProperty(UIList::DataProperties::LOCALTEXT, localText);
		result = true;
	}

	return result;
}

//----------------------------------------------------------------------

bool UIList::SetLocalText(int row, Unicode::String const &text)
{
	bool result = false;
	UIData *data = GetDataAtRow(row);

	if (data != NULL)
	{
		data->SetProperty(UIList::DataProperties::LOCALTEXT, text);
		result = true;
	}

	return result;
}

//----------------------------------------------------------------------

bool UIList::GetText(int const index, std::string &text)
{
	bool result = false;
	UIData *data = GetDataAtRow(index);

	if (data != NULL)
	{
		text = data->GetName();
		result = true;
	}

	return result;
}

//======================================================================
