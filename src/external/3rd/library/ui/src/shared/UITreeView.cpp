//======================================================================
//
// UITreeView.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "_precompile.h"
#include "UITreeView_DataNode.h"

#include "UICanvas.h"
#include "UIClock.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIDataSourceContainer.h"
#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIMessage.h"
#include "UIPropertyDescriptor.h"
#include "UIText.h"
#include "UITreeViewStyle.h"
#include "UIUtils.h"

#include <numeric>
#include <vector>

//----------------------------------------------------------------------

const char * const UITreeView::TypeName = "TreeView";

const UILowerString UITreeView::DataProperties::LocalText  = UILowerString ("LocalText");
const UILowerString UITreeView::DataProperties::Text       = UILowerString ("Text");
const UILowerString UITreeView::DataProperties::Underline  = UILowerString ("Underline");
const UILowerString UITreeView::DataProperties::Icon       = UILowerString ("Icon");
const UILowerString UITreeView::DataProperties::Expanded   = UILowerString ("Expanded");
const UILowerString UITreeView::DataProperties::ColorIndex = UILowerString ("ColorIndex");
const UILowerString UITreeView::DataProperties::Selectable = UILowerString ("Selectable");

const UILowerString UITreeView::PropertyName::DataSourceContainer      = UILowerString ("DataSourceContainer");
const UILowerString UITreeView::PropertyName::ScrollWheelSelection     = UILowerString ("ScrollWheelSelection");
const UILowerString UITreeView::PropertyName::SelectedRow              = UILowerString ("SelectedRow");
const UILowerString UITreeView::PropertyName::SelectedRowCount         = UILowerString ("SelectedRowCount");
const UILowerString UITreeView::PropertyName::SelectionAllowedMultiRow = UILowerString ("SelectionAllowedMultiRow");
const UILowerString UITreeView::PropertyName::ShowIcon                 = UILowerString ("ShowIcon");
const UILowerString UITreeView::PropertyName::ShowNavigationIcons      = UILowerString ("ShowNavigationIcons");
const UILowerString UITreeView::PropertyName::Style                    = UILowerString ("Style");

//======================================================================================
#define _TYPENAME UITreeView

namespace UITreeViewNamespace
{
	//----------------------------------------------------------------------

	void saveCacheState (UITreeView::DataNode * parent, const UIDataSourceContainer & dsc)
	{
		UI_UNREF (parent);
		UI_UNREF (dsc);
	}

	//----------------------------------------------------------------------

	void populateData (UITreeView::DataNode * parent, const UIDataSourceContainer & dsc)
	{
		const UIDataSourceContainer::DataSourceBaseList & dsbs = dsc.GetDataSourceBaseList();
		for (UIDataSourceContainer::DataSourceBaseList::const_iterator it = dsbs.begin (); it != dsbs.end (); ++it)
		{
			UIDataSourceBase * const dsb = *it;

			if (dsb->IsA (TUIDataSourceContainer))
			{
				UIDataSourceContainer * const dsc = static_cast<UIDataSourceContainer *>(dsb);

				UITreeView::DataNode * const node = new UITreeView::DataNode (dsc, parent);
				populateData (node, *dsc);
			}
		}
	}

	//-----------------------------------------------------------------

	UIDataSourceContainer * getNthChild (UIDataSourceContainer & parent, long n, long & count)
	{
		if (count == n)
			return &parent;

		const UIDataSourceContainer::DataSourceBaseList & dsbs = parent.GetDataSourceBaseList();
		for (UIDataSourceContainer::DataSourceBaseList::const_iterator it = dsbs.begin (); count < n && it != dsbs.end (); ++it)
		{
			UIDataSourceBase * const dsb = *it;

			if (dsb->IsA (TUIDataSourceContainer))
			{
				UIDataSourceContainer * const result = getNthChild (*static_cast<UIDataSourceContainer *>(dsb), n, ++count);
				if (result)
					return result;
			}
		}

		return 0;
	}

	//----------------------------------------------------------------------

	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(DataSourceContainer, "", T_object),
		_DESCRIPTOR(SelectedRow, "", T_int),
		_DESCRIPTOR(SelectionAllowedMultiRow, "", T_bool),
		_DESCRIPTOR(Style, "", T_object),
		_DESCRIPTOR(ScrollWheelSelection, "", T_bool),
	_GROUPEND(Basic, 2, 0);
	//================================================================

	//================================================================
	// Appearance category
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(ShowIcon, "", T_bool),
		_DESCRIPTOR(ShowNavigationIcons, "", T_bool),
	_GROUPEND(Appearance, 2, 1);
	//================================================================
}
using namespace UITreeViewNamespace;
//======================================================================================

UITreeView::UITreeView () :
UIWidget                  (),
mDataSourceContainer      (0),
mSelectedRows             (new LongList),
mSelectionAllowedRow      (true),
mSelectionAllowedRowMultiple (true),
mMouseDown                (false),
mMouseDownRow             (0L),
mMouseDragLastRow         (0L),
mMouseMoveLastPoint       (),
mListExtent               (),
mStyle                    (0),
mSelectionMovesWithMouse  (false),
mSelectionMovingWithWheel (false),
mSelectionFinalized       (false),
mShowIcon                 (false),
mShowNavigationIcons      (true),
mDataCache                (0),
mScrollWheelSelection     (false),
mCachingInProgress        (false)
{
	SetBackgroundColor (UIColor (200, 200, 200));
	SetBackgroundOpacity (1.0f);

	AddCallback (this);
}

//----------------------------------------------------------------------

UITreeView::~UITreeView ()
{
	RemoveCallback (this);

	SetDataSourceContainer (0);
	SetStyle (0);

	delete mSelectedRows;
	mSelectedRows = 0;
}

//-----------------------------------------------------------------

bool UITreeView::IsA( const UITypeID Type ) const
{
	return (Type == TUITreeView) || UIWidget::IsA (Type);
}

//----------------------------------------------------------------------

void UITreeView::GetLinkPropertyNames( UIPropertyNameVector & in ) const
{
	in.push_back ( PropertyName::DataSourceContainer );
	in.push_back ( PropertyName::Style);

	UIWidget::GetLinkPropertyNames (in);
}

//----------------------------------------------------------------------

void UITreeView::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidget::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
}

//----------------------------------------------------------------------

void UITreeView::GetPropertyNames( UIPropertyNameVector & in, bool forCopy ) const
{
	in.push_back ( PropertyName::DataSourceContainer );
	in.push_back ( PropertyName::SelectedRow             );
	in.push_back ( PropertyName::SelectedRowCount        );
	in.push_back ( PropertyName::SelectionAllowedMultiRow);
	in.push_back ( PropertyName::Style);
	in.push_back ( PropertyName::ShowIcon);
	in.push_back ( PropertyName::ShowNavigationIcons);
	in.push_back ( PropertyName::ScrollWheelSelection);

	UIWidget::GetPropertyNames (in, forCopy);
}

//----------------------------------------------------------------------

bool UITreeView::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if (Name == PropertyName::Style       )
	{
		UIBaseObject * const object = GetObjectFromPath( Value, TUITreeViewStyle );

		if( object || Value.empty() )
		{
			SetStyle( static_cast<UITreeViewStyle *>(object) );
			return true;
		}
	}

	else if (Name == PropertyName::DataSourceContainer       )
	{
		UIBaseObject * const object = GetObjectFromPath( Value, TUIDataSourceContainer );

		if( object || Value.empty() )
		{
			SetDataSourceContainer( static_cast<UIDataSourceContainer *>(object) );
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

	else if (Name == PropertyName::SelectedRowCount         )
	{
		return false;
	}

	else if (Name == PropertyName::SelectionAllowedMultiRow )
	{
		return UIUtils::ParseBoolean (Value, mSelectionAllowedRowMultiple);
	}

	else if (Name == PropertyName::ShowIcon )
	{
		return UIUtils::ParseBoolean (Value, mShowIcon);
	}

	else if (Name == PropertyName::ShowNavigationIcons )
	{
		return UIUtils::ParseBoolean (Value, mShowNavigationIcons );
	}

	else if (Name == PropertyName::ScrollWheelSelection )
	{
		return UIUtils::ParseBoolean (Value, mScrollWheelSelection);
	}
	return UIWidget::SetProperty (Name, Value);
}

//----------------------------------------------------------------------

bool UITreeView::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if (Name == PropertyName::Style       )
	{
		if( mStyle )
		{
			GetPathTo( Value, mStyle );
			return true;
		}
	}

	if (Name == PropertyName::DataSourceContainer       )
	{
		if( mDataSourceContainer )
		{
			GetPathTo( Value, mDataSourceContainer );
			return true;
		}
	}

	else if (Name == PropertyName::SelectedRow              )
	{
		return UIUtils::FormatLong (Value, GetLastSelectedRow ());
	}

	else if (Name == PropertyName::SelectedRowCount         )
	{
		return UIUtils::FormatLong (Value, static_cast<long>(mSelectedRows->size ()));
	}

	else if (Name == PropertyName::SelectionAllowedMultiRow )
	{
		return UIUtils::FormatBoolean (Value, mSelectionAllowedRowMultiple);
	}

	else if (Name == PropertyName::ShowIcon )
	{
		return UIUtils::FormatBoolean (Value, mShowIcon);
	}

	else if (Name == PropertyName::ShowNavigationIcons )
	{
		return UIUtils::FormatBoolean (Value, mShowNavigationIcons);
	}

	else if (Name == PropertyName::ScrollWheelSelection )
	{
		return UIUtils::FormatBoolean (Value, mScrollWheelSelection);
	}

	return UIWidget::GetProperty (Name, Value);
}

//----------------------------------------------------------------------

void UITreeView::Render( UICanvas & canvas ) const
{
	UIWidget::Render (canvas);

	if (!mDataCache || !mStyle)
		return;

	const long numRows = static_cast<long>(mDataCache->countAllChildren ());

	if (numRows <= 0)
		return;

	const UIPoint & scrollLocation      = GetScrollLocation ();
	const UIPoint & size                = GetSize ();

	const UISize & cellPadding          = mStyle->GetCellPadding ();
	const long     cellHeight           = mStyle->GetCellHeight ();

	const UIColor oldColor              = canvas.GetColor ();
	const UISize halfCellPadding (cellPadding / 2L);

	const UIColor & selectionBackground = mStyle->GetSelectionColorBackground ();
	const UIColor & selectionRect       = mStyle->GetSelectionColorRect ();
	const UIColor & gridColor           = mStyle->GetGridColor ();

	{
		UILine line;

		if (cellPadding.y > 0)
		{
			long y = 0;

			line.p1.x = static_cast<float>(scrollLocation.x);
			line.p2.x = line.p1.x + static_cast<float>(size.x);

			bool lastRowSelected = false;
			bool lastRowUnderline = false;

			DataNode::Iterator it (mDataCache, true);
			it.skip ();

			for (DataNode * cur = it.next () ;; cur = it.next (), y += cellHeight + cellPadding.y)
			{
				canvas.SetColor (oldColor);

				if (cur && IsRowSelected (it.getIndex ()))
				{
					if (y + cellHeight + cellPadding.y >= scrollLocation.y && y < scrollLocation.y + size.y)
					{
						canvas.ModifyColor (selectionBackground);
						canvas.BltFrom (0, UIPoint::zero, UIPoint (scrollLocation.x, y + halfCellPadding.y ), UISize (size.x, cellHeight));
						canvas.SetColor (oldColor);
					}
				}

				if (y >= scrollLocation.y + size.y)
					break;

				const bool isRowSelected = IsRowSelected (it.getIndex ());

				if (y >= scrollLocation.y)
				{
					line.p1.y = line.p2.y = static_cast<float>(y);

					if (isRowSelected || lastRowSelected)
					{
						canvas.ModifyColor (selectionRect);
						canvas.RenderLine (line);
					}
					else if (lastRowUnderline)
					{
						canvas.ModifyColor (gridColor);
						canvas.RenderLine (line);
					}
				}

				if (!cur)
					break;

				lastRowUnderline = cur->underline;
				lastRowSelected = isRowSelected;
			}
		}
	}

	//----------------------------------------------------------------------
	//----------------------------------------------------------------------
	//----------------------------------------------------------------------

	//- cache this

	UIPoint pt (halfCellPadding);
	long columnWidth = GetWidth () - halfCellPadding.x;

	canvas.SetColor (oldColor);
	canvas.PushState ();

	DataNode::Iterator it (mDataCache, true);
	it.skip ();

	pt.x = 0L;

	for (DataNode * cur = it.next (); cur; cur = it.next (), pt.y += cellHeight + cellPadding.y, pt.x = 0L)
	{
		canvas.RestoreState ();

		canvas.Translate (pt);

		const bool isRowSelected = IsRowSelected (it.getIndex ());

		if (mShowIcon)
		{
			pt.x += halfCellPadding.x;
			canvas.Translate (halfCellPadding.x, 0L);

			if (cur->icon)
			{
				UIImage & image = mStyle->GetDefaultImage (cur->icon);
				canvas.Translate (image.GetLocation ());
				image.Render (canvas);
				canvas.Translate (-image.GetLocation ());
			}

			pt.x += cellHeight + halfCellPadding.x;
			canvas.Translate (cellHeight + halfCellPadding.x, 0L);
		}

		const int indentationLevel = cur->countDepth () - 2;
		const long translate_diff_x = indentationLevel * mStyle->GetIndentation ();
		pt.x += translate_diff_x;

		canvas.Translate (translate_diff_x, 0L);
		
		if (mShowNavigationIcons)
		{
			if (!cur->children.empty ())
			{
				UIImage & image = mStyle->GetDefaultImage (cur->expanded ? UITreeViewStyle::IST_expanded : UITreeViewStyle::IST_collapsed);
				canvas.Translate (image.GetLocation ());
				image.Render (canvas);
				canvas.Translate (-image.GetLocation ());
			}
			
			pt.x += mStyle->GetIndentation ();
			pt.x += halfCellPadding.x;
			
			canvas.Translate (mStyle->GetIndentation () + halfCellPadding.x, 0L);
		}

		UIText & text = mStyle->GetDefaultText (static_cast<UITreeViewStyle::DefaultTextColors>(cur->colorIndex), isRowSelected);
		text.SetPreLocalized    (true);
		text.SetMaxLines        (1);
		text.SetTruncateElipsis (true);
		text.SetLocalText       (cur->text);
		text.SetSize            (UISize (columnWidth - pt.x, cellHeight));

		canvas.Clip (text.GetRect ());
		text.Render (canvas);
	}

	canvas.PopState ();
}

//----------------------------------------------------------------------

void UITreeView::SetDataSourceContainer (UIDataSourceContainer * ds, bool cache)
{
	RemoveProperty (PropertyName::DataSourceContainer);

	if (mDataSourceContainer)
		mDataSourceContainer->StopListening (this);

	AttachMember (mDataSourceContainer, ds);

	if (mDataSourceContainer)
		mDataSourceContainer->Listen (this);

	if (cache)
		CacheData ();
}

//----------------------------------------------------------------------

void UITreeView::Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode )
{
	if (NotifyingObject == mDataSourceContainer)
	{
		if (mCachingInProgress)
			return;

		UI_UNREF (ContextObject);
		UI_UNREF (NotificationCode);

		CacheData ();
//		SelectRow (-1);
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

void UITreeView::updateExtent ()
{
	if (!mStyle)
		return;

	UISize scrollExtent;
	GetScrollExtent (scrollExtent);

	UISize const & minimumExtent = GetMinimumScrollExtent();

	scrollExtent.x = minimumExtent.x > GetWidth () ? minimumExtent.x : GetWidth();

	const long cellHeight      = mStyle->GetCellHeight ();
	const long rowCount        = mDataCache ? static_cast<long>(mDataCache->countAllVisibleChildren ()) : 0;
	const UISize & cellPadding = mStyle->GetCellPadding ();

	scrollExtent.y = rowCount * (cellHeight + cellPadding.y);

	mListExtent = scrollExtent;

	SetMinimumScrollExtent (scrollExtent);
	SetScrollExtent (scrollExtent);
}

//-----------------------------------------------------------------

void UITreeView::ScrollToRow (int targetRow)
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

void  UITreeView::GetScrollSizes( UISize &PageSize, UISize &LineSize ) const
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

void UITreeView::SetSize( const UISize & size)
{
	if (size != GetSize ())
	{
		UIWidget::SetSize (size);
		updateExtent ();
	}
}

//----------------------------------------------------------------------

void UITreeView::Link()
{
	UIWidget::Link ();

	if (mStyle)
		mStyle->Link ();
}

//----------------------------------------------------------------------

void UITreeView::SelectRow (long row)
{
	mSelectedRows->clear ();

	if (row >= 0)
	{
		mSelectedRows->push_back (row);
	}

	SendCallback( &UIEventCallback::OnGenericSelectionChanged, UILowerString::null);
}

//----------------------------------------------------------------------

void UITreeView::AddRowSelection (long row)
{
	if (row < 0)
		return;

	if (mSelectionAllowedRowMultiple)
	{
		mSelectedRows->remove (row);
		mSelectedRows->push_back (row);
	}
	else
		SelectRow (row);

	SendCallback( &UIEventCallback::OnGenericSelectionChanged, UILowerString::null );
}

//----------------------------------------------------------------------

void UITreeView::RemoveRowSelection (long row)
{
	if (row < 0)
		return;

	if (mSelectionAllowedRowMultiple)
	{
		mSelectedRows->remove (row);
	}
	else
	{
		SelectRow (-1);
	}

	SendCallback( &UIEventCallback::OnGenericSelectionChanged, UILowerString::null );
}

//----------------------------------------------------------------------

bool UITreeView::IsRowSelected (long row) const
{
	return std::find (mSelectedRows->begin (), mSelectedRows->end (), row) != mSelectedRows->end ();
}

//----------------------------------------------------------------------

bool UITreeView::GetRowFromPoint (const UIPoint & pt, long & row, DataNode ** node) const
{
	if (!mStyle || !mDataCache)
	{
		row = -1;
		return false;
	}

	const UIPoint & scrollLocation = GetScrollLocation ();
	const UIPoint & size           = GetSize ();
	const UIPoint translatedPt (pt + scrollLocation);
	const long     cellHeight      = mStyle->GetCellHeight ();
	const UISize & cellPadding     = mStyle->GetCellPadding ();

	row = -1;
	long curY = cellPadding.y / 2L;

	DataNode::Iterator it (mDataCache, true);
	it.skip ();

	for (DataNode * cur = it.next () ;cur; cur = it.next (), curY += cellHeight + cellPadding.y)
	{
		if (translatedPt.y >= curY && translatedPt.y < (curY + cellHeight + cellPadding.y))
		{
			row = it.getIndex ();
			if (node)
				*node = cur;

			if (pt.y < 0 || pt.y >= size.y || translatedPt.y < 0 || translatedPt.y >= mListExtent.y)
				return false;

			break;
		}
	}

	return true;
}

//----------------------------------------------------------------------

void UITreeView::addRangeSelection (long one, long two)
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

void UITreeView::removeRangeSelection (long one, long two)
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

bool UITreeView::ProcessMessage( const UIMessage & msg)
{
	// postpone the ContextRequest until we have picked the
	// tree slot where the context click occured
	if (msg.Type != UIMessage::ContextRequest)
	{
		if (UIWidget::ProcessMessage (msg))
		{
			return true;
		}
	}

	if (!mStyle || !mDataCache)
		return false;

	const long rowCount        = static_cast<long>(mDataCache->countAllChildren ());
	const long cellHeight      = mStyle->GetCellHeight ();
	const UISize & cellPadding = mStyle->GetCellPadding ();

	//----------------------------------------------------------------------

	// we now select the widget even if there is a ContextRequest 
	if ((msg.Type == UIMessage::LeftMouseDown) || (msg.Type == UIMessage::ContextRequest))
	{
		mSelectionFinalized = false;

		DataNode * node = 0;
		long row = -1;
		if (!GetRowFromPoint (msg.MouseCoords, row, &node))
			return false;

		if (node)
		{
			int depth = node->countDepth () - 1;
			if (!mShowNavigationIcons)
				depth = std::max (0, depth - 1);
			
			long maxX = depth * mStyle->GetIndentation ();
			long minX = 0L;
			if (mShowIcon)
			{
				minX = cellHeight + cellPadding.x;
				maxX += minX;
			}
			
			if (msg.MouseCoords.x < maxX && msg.MouseCoords.x > minX)
			{
				ToggleExpanded (row, *node);
				return true;
			}
		}
		
		if (msg.Modifiers.LeftControl || msg.Modifiers.LeftControl)
		{
			if (node && node->selectable)
			{
				if (IsRowSelected (row))
					RemoveRowSelection (row);
				else if (mSelectionAllowedRowMultiple)
					AddRowSelection (row);
				else
					SelectRow (row);
			}
		}
		else if (mSelectionAllowedRowMultiple && (msg.Modifiers.LeftShift || msg.Modifiers.RightShift))
		{
			if (node && node->selectable)
			{
				const long lastSelectedRow = GetLastSelectedRow ();
				addRangeSelection (std::max (0L, lastSelectedRow), row);
			}
		}
		else if (node && node->selectable)
			SelectRow (row);

		mMouseDown = true;
		mMouseDownRow = row;
		mMouseDragLastRow = row;

		UIClock::gUIClock ().ListenPerFrame (this);

		mSelectionMovingWithWheel = false;

		// in the case of this being a ContextRequest we now want
		// to process the message exactly as above
		if (msg.Type == UIMessage::ContextRequest)
		{
			// now process the context click
			if (!UIWidget::ProcessMessage (msg))
			{
				return false;
			}
		}

		return true;
	}

	//----------------------------------------------------------------------

	if (msg.Type == UIMessage::LeftMouseDoubleClick)
	{
		mSelectionFinalized = false;

		DataNode * node = 0;
		long row = -1;
		if (!GetRowFromPoint (msg.MouseCoords, row, &node))
			return false;

		if (node)
		{
			ToggleExpanded (row, *node);
			return true;
		}

		return false;
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::LeftMouseUp || msg.Type == UIMessage::MouseExit)
	{
		const bool mouseWasDown = mMouseDown;

		mMouseDown = false;
		UIClock::gUIClock ().StopListening (this);

		if (msg.Type == UIMessage::LeftMouseUp)
		{
			if (HitTest (msg.MouseCoords))
			{
				mSelectionFinalized = true;
				SendCallback( &UIEventCallback::OnGenericSelectionChanged, UILowerString::null );
			}
			else if (mouseWasDown)
				return true;
		}

		return false;
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::MouseMove)
	{
		if (mMouseDown || (mSelectionMovesWithMouse && !mSelectionMovingWithWheel))
		{
			long row = -1;
			DataNode * node = 0;
			GetRowFromPoint (msg.MouseCoords, row, &node);

			if (row >= 0)
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
					if (node && node->selectable)
						SelectRow (row);
				}
			}

			mMouseDragLastRow = row;
			mMouseMoveLastPoint = msg.MouseCoords;
			return true;
		}
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::KeyDown || msg.Type == UIMessage::KeyRepeat)
	{

		UISize pageSize;
		UISize lineSize;

		GetScrollSizes( pageSize, lineSize );

		long targetRow = mMouseDragLastRow;

		bool changesSelection = false;
		if ((msg.IsKeyMessage () && msg.Keystroke == UIMessage::DownArrow) ||
			(msg.Type == UIMessage::MouseWheel && msg.Data < 0))
		{
			changesSelection = true;
			++targetRow;
		}
		else if ((msg.IsKeyMessage () && msg.Keystroke == UIMessage::UpArrow) ||
			(msg.Type == UIMessage::MouseWheel && msg.Data > 0))
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

		return MoveStuff (targetRow, changesSelection, (msg.Modifiers.LeftShift || msg.Modifiers.RightShift));
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::Character)
	{
		if (msg.Keystroke == UIMessage::Space)
		{
			DataNode::Iterator it (mDataCache, false);
			it.skip ();
			DataNode * const node = it.advance (mMouseDragLastRow + 1);

			if (node)
			{
				ToggleExpanded (mMouseDragLastRow, *node);
				return true;
			}
		}
		return false;
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::MouseWheel)
		return true;

	return false;
}

//-----------------------------------------------------------------

bool UITreeView::MoveStuff (long targetRow, bool changesSelection, bool shifted)
{
	if (!mDataSourceContainer || !mDataCache)
		return false;

	const long rowCount        = static_cast<long>(mDataCache->countAllChildren ());

	targetRow = std::max (0L, std::min (targetRow, rowCount - 1L));

	DataNode::Iterator it (mDataCache, true);
	it.skip ();

	const bool down = (targetRow > mMouseDragLastRow);
	int lastRow = -1;
	bool found = false;

	for (DataNode * cur = it.next () ;cur; cur = it.next ())
	{
		const int index = it.getIndex ();

		if (down)
		{
			if (index >= targetRow)
			{
				targetRow = index;
				found = true;
				break;
			}
		}
		else
		{
			if (index == targetRow)
			{
				targetRow = index;
				found = true;
				break;
			}
			else if (index > targetRow)
				break;
		}

		lastRow = index;
	}

	if (!found)
		targetRow = lastRow;

	if (!shifted)
	{
		if (mMouseDownRow != targetRow)
		{
			mMouseDownRow = targetRow;
			if (changesSelection)
			{
				SelectRow (targetRow);
				mMouseDragLastRow = targetRow;
				ScrollToRow (targetRow);

				return true;
			}
		}

		return false;
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

bool UITreeView::OnMessage (UIWidget * context, const UIMessage & msg)
{
	if (context == this)
	{
		if (mScrollWheelSelection && msg.Type == UIMessage::MouseWheel)
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

bool UITreeView::GetTotalRowExtents (int row, long & top, long & bottom)
{
	if (!mDataCache)
		return false;

	const long rowCount = static_cast<long>(mDataCache->countAllChildren ());

	if (row < 0 || row >= rowCount)
		return false;

	const long cellHeight  = mStyle->GetCellHeight ();
	const UISize & cellPadding = mStyle->GetCellPadding ();

	DataNode::Iterator it (mDataCache, true);
	it.skip ();

	long tmpTop = 0L;

	for (DataNode * cur = it.next () ;cur; cur = it.next ())
	{
		if (it.getIndex () > row)
			return false;
		else if (it.getIndex () == row)
		{
			top = tmpTop;
			bottom = top + cellHeight + cellPadding.y;
			return true;
		}
		tmpTop += cellHeight + cellPadding.y;
	}

	return false;
}

//-----------------------------------------------------------------

long UITreeView::GetLastSelectedRow () const
{
	return mSelectedRows->empty () ? -1 : mSelectedRows->back ();
}

//----------------------------------------------------------------------

void UITreeView::SetStyle (UITreeViewStyle * style)
{
	RemoveProperty (PropertyName::Style);
	AttachMember (mStyle, style);
	updateExtent ();
}

//----------------------------------------------------------------------

UIStyle * UITreeView::GetStyle( void ) const
{
	return mStyle;
}

//----------------------------------------------------------------------

void UITreeView::SetSelectionAllowedMultiRow (bool b)
{
	mSelectionAllowedRowMultiple = b;
}

//----------------------------------------------------------------------

void UITreeView::SetSelectionMovesWithMouse (bool b)
{
	mSelectionMovesWithMouse = b;
	mSelectionMovingWithWheel = false;
}

//----------------------------------------------------------------------

void UITreeView::SetSelectionFinalized (bool b)
{
	mSelectionFinalized = b;
}

//----------------------------------------------------------------------

void UITreeView::CacheData ()
{
	if (mCachingInProgress)
		return;

	mCachingInProgress = true;

	if (mDataCache)
	{
		if (mDataSourceContainer)
		{
			mDataSourceContainer->StopListening (this);
			mDataCache->saveStateRecursive      ();
			mDataSourceContainer->Listen (this);
		}
	}

	delete mDataCache;
	mDataCache = 0;

	if (mDataSourceContainer)
	{
		mDataCache = new DataNode (0, 0);
		mDataCache->expanded = true;
		populateData (mDataCache, *mDataSourceContainer);
	}

	updateExtent ();

	mCachingInProgress = false;
}

//----------------------------------------------------------------------

void UITreeView::ToggleExpanded (int row, DataNode & node)
{
	SetNodeExpanded (node, row, !node.expanded);
	updateExtent ();

	//see if expansion/collapse caused a different row to now be selected, check for selectable
	long r = GetLastSelectedRow();
	DataNode * n = GetDataNodeAtRow(r);
	if(n && !n->selectable)
	{
		RemoveRowSelection(r);
	}
	SendTreeRowExpansionToggledCallback(row);
}

//----------------------------------------------------------------------

bool UITreeView::SetNodeExpanded (DataNode & node, long row, bool b, bool applyToChildren)
{
	node.expanded = b;

	//----------------------------------------------------------------------
	//-- deselect children
	if (!node.expanded || applyToChildren)
	{
		DataNode::Iterator it (&node, false);
		it.next ();

		bool found = false;

		for (DataNode * cur = it.next () ;cur; cur = it.next ())
		{
			const int index = row + it.getIndex ();
			if (!node.expanded && IsRowSelected (index))
			{
				found = true;
				RemoveRowSelection (index);
			}

			if (applyToChildren)
				cur->expanded = b;
		}

		if (found)
			AddRowSelection (row);

		return found;
	}

	return false;
}

//-----------------------------------------------------------------

UIDataSourceContainer * UITreeView::GetDataSourceContainerAtRow (long row)
{
	if (row < 0)
		return 0;

	long count = 0;
	return mDataSourceContainer ? getNthChild (*mDataSourceContainer, row + 1, count) : 0;
}

//----------------------------------------------------------------------

UITreeView::DataNode * UITreeView::GetDataNodeAtRow  (long row)
{
	if (!mStyle || !mDataCache)
	{
		row = -1;
		return 0;
	}

	long theRow = 0;

	DataNode::Iterator it (mDataCache, true);
	it.skip ();

	for (DataNode * cur = it.next () ;cur; cur = it.next (), ++theRow)
	{
		if (row == theRow)
			return cur;
	}

	return 0;
}

//----------------------------------------------------------------------

void UITreeView::SetRowExpanded (long row, bool b)
{
	DataNode * const node = GetDataNodeAtRow (row);
	if (node)
	{
		SetNodeExpanded (*node, row, b);
		updateExtent ();
	}
}

//----------------------------------------------------------------------

void UITreeView::SetAllRowsExpanded (bool b)
{
	if (mDataCache)
	{
		SetNodeExpanded (*mDataCache, 0, b, true);
		updateExtent ();
	}
}

//----------------------------------------------------------------------

bool UITreeView::GetRowExpanded(long row)
{
	DataNode * const node = GetDataNodeAtRow (row);
	if (node)
	{
		return node->expanded;
	}
	return false;
}

//----------------------------------------------------------------------

UITreeView::DataNode * UITreeView::FindDataNodeByDataSource      (const UIBaseObject & dataSource, int & row) const
{
	DataNode::Iterator it (mDataCache, false);
	it.skip ();
	
	row = 0;
	for (DataNode * cur = it.next () ;cur; cur = it.next ())
	{
		if (cur->getDataObject () == &dataSource)
			return cur;
		++row;
	}

	return 0;
}

//----------------------------------------------------------------------

void  UITreeView::ExpandParentNodes         (DataNode & node)
{
	DataNode * applyToNode = node.parent;

	while (applyToNode)
	{
		SetNodeExpanded (*applyToNode, 0, true, false);
		applyToNode = applyToNode->parent;
	}

	updateExtent ();
}

//----------------------------------------------------------------------

void UITreeView::ClearData                    ()
{
	if (mDataSourceContainer)
		mDataSourceContainer->Clear ();

	delete mDataCache;
	mDataCache = 0;
}

//----------------------------------------------------------------------

long UITreeView::GetRowCount                    ()
{
	return static_cast<long>(mDataCache->countAllChildren ());
}

//======================================================================
